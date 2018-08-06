/**
 * Mini Valgrind Lab
 * CS 241 - Fall 2017
 */

#include <assert.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mini_valgrind.h"

/*
 * WARNING: Here be dragons! (And not the kind from Pointers Gone Wild.)
 *
 * This file implements supporting code for mini_valgrind.
 *
 * You shouldn't modify any of it, nor are you expected to understand all of
 * it, but you are welcome to study it to figure out how mini_valgrind works.
 *
 * ----------------------------------------------------------------------------
 */

/*
 * This macro ensures that only certain symbols are visible to the users of
 * this shared library.
 */
#define EXPORT_SYMBOL __attribute__((__visibility__("default")))

/*
 * These function pointers refer to the original, libc implementations of the
 * malloc family.
 */
static void *(*real_malloc)(size_t) = NULL;
static void *(*real_calloc)(size_t, size_t) = NULL;
static void *(*real_realloc)(void *, size_t) = NULL;
static void (*real_free)(void *) = NULL;

/*
 * Thread-local flag indicating whether our special malloc/realloc hooks are
 * currently enabled. We disable the hooks inside of the malloc wrapper and
 * some other places to allow internal functions to call the real malloc
 * instead of getting stuck recursing indefinitely.
 */
static __thread bool hooks_active = true;

/*
 * Load the real versions of malloc and friends and store them in our special
 * function pointers.
 *
 * This function has a reentrancy guard to prevent it from getting stuck in an
 * infinite loop, since dlsym can actually end up calling malloc. In this
 * unusual case, we act like malloc failed by returning false. Otherwise, this
 * function returns true.
 */
static bool load_real_malloc() {
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread bool reentrant_guard = false;

    if (reentrant_guard) {
        return false;
    }
    reentrant_guard = true;
    pthread_mutex_lock(&lock);

    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_calloc = dlsym(RTLD_NEXT, "calloc");
    real_realloc = dlsym(RTLD_NEXT, "realloc");
    real_free = dlsym(RTLD_NEXT, "free");

    if (!real_malloc || !real_calloc || !real_realloc || !real_free) {
        // Can't get the real functions for some reason.
        // Use write instead of printf because printf requires malloc.
        const char *err = "FATAL: couldn't resolve libc malloc!\n";
        write(STDERR_FILENO, err, strlen(err));
        exit(3);
    }

    pthread_mutex_unlock(&lock);
    reentrant_guard = false;
    return true;
}

/*
 * Store info about the function caller in the provided arguments.
 */
static void fetch_caller_info(const char **filename, void **instruction) {
    static const size_t stacksize = 8;
    static const char *internal = "mini_valgrind";

    void *callstack[stacksize];
    Dl_info info;

    int frames = backtrace(callstack, stacksize);

    for (int frame = 0; frame < frames; frame++) {
        void *addr = callstack[frame];
        if (!dladdr(addr, &info)) {
            // Couldn't resolve the calling object
            return;
        }

        assert(info.dli_fname != NULL);
        if (strstr(info.dli_fname, internal) == NULL) {
            // Found it!
            *filename = info.dli_fname;
            *instruction = addr;
            break;
        }
    }
}

/*
 * Macros to wrap a mini_*alloc function call.
 */
#define ALLOC_FUNC_SETUP                                                       \
    hooks_active = false;                                                      \
    const char *filename = "<unknown>";                                        \
    void *instruction = NULL;                                                  \
    fetch_caller_info(&filename, &instruction);

#define ALLOC_FUNC_TEARDOWN hooks_active = true;

#define ALLOC_FUNC_PASSTHRU(real_func, ...)                                    \
    if (real_func == NULL) {                                                   \
        if (!load_real_malloc()) {                                             \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    return real_func(__VA_ARGS__);

/*
 * Replacement functions to override the builtin libc malloc family. These wrap
 * mini_malloc and friends and call them with info about the caller.
 */
EXPORT_SYMBOL
void *malloc(size_t request_size) {
    if (!hooks_active) {
        ALLOC_FUNC_PASSTHRU(real_malloc, request_size)
    }

    ALLOC_FUNC_SETUP
    void *ptr = mini_malloc(request_size, filename, instruction);
    ALLOC_FUNC_TEARDOWN
    return ptr;
}

EXPORT_SYMBOL
void *calloc(size_t num_elements, size_t element_size) {
    if (!hooks_active) {
        ALLOC_FUNC_PASSTHRU(real_calloc, num_elements, element_size)
    }

    ALLOC_FUNC_SETUP
    void *ptr = mini_calloc(num_elements, element_size, filename, instruction);
    ALLOC_FUNC_TEARDOWN
    return ptr;
}

EXPORT_SYMBOL
void *realloc(void *ptr, size_t request_size) {
    if (!hooks_active) {
        ALLOC_FUNC_PASSTHRU(real_realloc, ptr, request_size)
    }

    ALLOC_FUNC_SETUP
    ptr = mini_realloc(ptr, request_size, filename, instruction);
    ALLOC_FUNC_TEARDOWN
    return ptr;
}

EXPORT_SYMBOL
void free(void *ptr) {
    if (!hooks_active) {
        if (real_free != NULL || load_real_malloc()) {
            real_free(ptr);
        }
        return;
    }

    hooks_active = false;
    mini_free(ptr);
    hooks_active = true;
}

/*
 * Print out a message to stderr, prepended with our current process ID.
 */
static void printmsg(const char *format, ...) {
    bool orig_status = hooks_active;
    hooks_active = false;

    fprintf(stderr, "==%d== ", getpid());

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    hooks_active = orig_status;
}

/*
 * Try to decode a filename and instruction into a symbol name and source file
 * name/line number.
 *
 * This is kind of an ugly hack that uses the addr2line utility, because it
 * seems to be the best way to actually resolve symbols without pulling in a
 * whole library like libunwind or implementing a bunch of symbol table parsing
 * code ourselves.
 */
static void resolve_addr2line(char *symbol, char *source, const char *filename,
                              void *instruction) {
    int fds[2];
    if (pipe2(fds, O_CLOEXEC) < 0) {
        return;
    }

    pid_t child = fork();
    if (child < 0) {
        // Fork failed; act like we couldn't resolve the symbol
        return;
    }

    if (!child) {
        // In the child:
        // Call "addr2line -f -s -e <filename> <instruction>"
        // and send the stdout to our parent via the pipe.
        char inst_str[128];
        snprintf(inst_str, sizeof(inst_str), "%p", instruction);

        char fn_buf[1024];
        strncpy(fn_buf, filename, sizeof(fn_buf));
        fn_buf[sizeof(fn_buf) - 1] = '\0';

        char *command = "addr2line";
        char *args[] = {command, "-f", "-s", "-e", fn_buf, inst_str, NULL};
        char *env[] = {"LD_PRELOAD=", NULL};

        dup2(fds[1], STDOUT_FILENO);
        close(STDERR_FILENO);
        execvpe(command, args, env);
        exit(1);
    }

    else {
        // In the parent:
        // Wait on our child, then read the output of addr2line from the pipe,
        // then extract the symbol and source info.
        int status;
        close(fds[1]);
        if (waitpid(child, &status, 0) < 0) {
            return;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            return;
        }

        char util_buffer[1024];
        ssize_t len = read(fds[0], util_buffer, sizeof(util_buffer));
        util_buffer[len] = '\0';
        close(fds[0]);

        char *source_info = strchr(util_buffer, '\n');
        if (!source_info) {
            return;
        }

        *source_info++ = '\0';
        if (source_info[strlen(source_info) - 1] == '\n') {
            source_info[strlen(source_info) - 1] = '\0';
        }
        if (strncmp(source_info, "??:", 3)) {
            strcpy(source, source_info);
        }

        if (strcmp(util_buffer, "??")) {
            strcpy(symbol, util_buffer);
        }
    }
}

/*
 * Decode a filename and instruction to a nice output string.
 */
static void resolve(char *output, const char *filename, void *instruction,
                    size_t bufsize) {
    char symbol[1024] = "??";
    char source[1024] = "??:0";

    Dl_info info;
    if (dladdr(instruction, &info) && info.dli_sname != NULL) {
        strncpy(symbol, info.dli_sname, sizeof(symbol));
        symbol[sizeof(symbol) - 1] = '\0';
    }

    snprintf(source, sizeof(source), "%s:%p", filename, instruction);

    resolve_addr2line(symbol, source, filename, instruction);

    snprintf(output, bufsize, "%s (%s)", symbol, source);
}

/*
 * Do some cleanup right before printing the final report.
 */
static void finalize() {
    // Calling printf() malloc()s a buffer inside glibc, but by default it
    // doesn't get free()d until later in the cleanup process than here.
    // We force it to flush its buffer to prevent this from being reported as
    // a leak:
    setvbuf(stdout, NULL, _IONBF, 0);

    // Free anything else allocated internally by glibc. We need to store the
    // original value for invalid_addresses because it can be clobbered when
    // glibc frees its internal structures:
    extern void __libc_freeres(void);
    size_t invalid_addresses_temp = invalid_addresses;
    __libc_freeres();
    invalid_addresses = invalid_addresses_temp;

    // Disable our custom malloc hooks so free() works correctly:
    hooks_active = false;

    // Some standard programs like /bin/ls close stderr right before exiting.
    // I'm not sure why they do this, but it's mean because our leak report
    // gets lost. Let's work around that:
    if (fcntl(STDERR_FILENO, F_GETFD) < 0) {
        freopen("/dev/tty", "w", stderr);
    }
}

/*
 * On shared library load, indicate that we're up and running.
 */
static void __attribute__((constructor)) print_greeting() {
    printmsg("Mini-Valgrind\n");
}

/*
 * On shared library unload, print the mini_valgrind leak report.
 */
static void __attribute__((destructor)) print_leak_info() {
    meta_data *leak_info, *garbage_collector;
    size_t total_leak = 0;
    char buffer[512] = "";

    finalize();

    printmsg("\n");
    leak_info = head;
    if (leak_info != NULL) {
        printmsg("LEAK REPORT:\n");
    }
    while (leak_info != NULL) {
        total_leak += leak_info->request_size;
        resolve(buffer, leak_info->filename, leak_info->instruction,
                sizeof(buffer));
        printmsg("   Leak origin: %s\n", buffer);
        printmsg("   Leak size: %zu bytes\n", leak_info->request_size);
        printmsg("   Leak memory address: %p\n", leak_info + 1);
        printmsg("\n");
        garbage_collector = leak_info;
        leak_info = leak_info->next;
        free(garbage_collector);
    }

    printmsg("Program made %zu bad call(s) to free or realloc.\n",
             invalid_addresses);
    printmsg("\n");
    printmsg("HEAP SUMMARY:\n");
    printmsg("   Total memory requested: %zu bytes\n", total_memory_requested);
    printmsg("   Total memory freed: %zu bytes\n", total_memory_freed);
    if (total_leak != 0) {
        printmsg("   Total leak: %zu bytes\n", total_leak);
    } else {
        printmsg("   No leaks, all memory freed. Congratulations!\n");
    }

    // Catch logical errors in the leak detector
    assert(total_leak == total_memory_requested - total_memory_freed);
}
