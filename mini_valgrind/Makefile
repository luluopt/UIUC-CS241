OBJS_DIR = .objs

# define all of student executables
EXE_SHARED = mini_valgrind.so
EXE_BINARY = mini_valgrind
EXE_TEST   = test

EXES_RELEASE = $(EXE_SHARED) $(EXE_BINARY) $(EXE_TEST)
EXES_DEBUG   = $(EXE_SHARED:%.so=%-debug.so) $(EXE_BINARY)-debug $(EXE_TEST)

# list object file dependencies for each
OBJS_SHARED = mini_valgrind.o mini_hacks.o
OBJS_BINARY = mini_main.o
OBJS_TEST   = test.o

# targets for make clean
CLEAN = $(OBJS_DIR) $(EXES_RELEASE) $(EXES_DEBUG)

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Wno-unused-parameter
CFLAGS_DEBUG   = -O0 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O2 $(WARNINGS)    -std=c99 -c -MMD -MP -D_GNU_SOURCE
CFLAGS_SHARED  = -fPIC -fvisibility=hidden

# set up linker
LD = clang
LDFLAGS =
LDFLAGS_SHARED = -ldl -lpthread -shared -fPIC

# the string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
IS_VM=$(shell hostname | grep "cs241")

ifeq ($(IS_VM),)
$(error This assignment must be compiled on the CS241 VMs)
endif

.PHONY: all
all: release

# build types
.PHONY: release
.PHONY: debug

release: $(EXES_RELEASE)
debug:   $(EXES_DEBUG)

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $@

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.pic.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $(CFLAGS_SHARED) $< -o $@

$(OBJS_DIR)/%-debug.pic.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $(CFLAGS_SHARED) $< -o $@

# exes
# you will need a pair of exe and exe-debug targets for each exe
$(EXE_SHARED): $(OBJS_SHARED:%.o=$(OBJS_DIR)/%-release.pic.o)
	$(LD) $^ $(LDFLAGS) $(LDFLAGS_SHARED) -o $@
	@chmod 0644 $@

$(EXE_SHARED:%.so=%-debug.so): $(OBJS_SHARED:%.o=$(OBJS_DIR)/%-debug.pic.o)
	$(LD) $^ $(LDFLAGS) $(LDFLAGS_SHARED) -o $@
	@chmod 0644 $@

$(EXE_BINARY): $(OBJS_BINARY:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_BINARY)-debug: $(OBJS_BINARY:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_TEST): $(OBJS_TEST:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf $(CLEAN)
