/**
 * Networking
 * CS 241 - Fall 2017
 */
#include "includes/dictionary.h"
#include "common.h"
#include "format.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <sys/epoll.h>

#define HEADER_SIZE 1024
#define MAX_CLIENTS 64
#define MAX_EVENTS  MAX_CLIENTS + 1

#define READING_HEADER   0
#define READING_SIZE     1
#define PROCESSING_GET   2
#define PROCESSING_PUT   3
#define WRITING_OK       4
#define WRITING_ERR      5
#define WRITING_MSG      6
#define WRITING_SIZE     7
#define WRITING_LIST     8
#define WRITING_FILE     9

typedef struct client_status {
    int         state;
    verb        cmd;
    char       *filename;
    char        header[HEADER_SIZE];
    int         buffer_offset;
    const char *err_msg;
    FILE       *fh;
    size_t      file_size;
    char       *ls;
} client_status;

static dictionary  *client_states;
static vector      *g_fn_vec;
static char        *g_dir_name;
static int          g_epfd;

void *dict_key_copy_constructor(void *key)
{
    int *dst = malloc(sizeof(int));
    int *src = (int*)key;
    *dst = *src;

    return dst;
}

void dict_key_destructor(void *key)
{
    free(key);
}

void *dict_value_copy_constructor(void *value)
{
    client_status *src = (client_status*)value;
    client_status *dst = malloc(sizeof(client_status));
    
    memcpy(dst, src, sizeof(client_status));
    return dst;
}

void dict_value_destructor(void *value)
{
    client_status *dst = (client_status*)value;
    free(dst->filename);
    free(dst->ls);
    free(dst);
}

void *vec_entry_copy_constructor(void *obj)
{
    char *src = (char*)obj;
    int   len = strlen(src);

    char *dst = malloc(len + 1);
    memset(dst, 0, len+1);
    memcpy(dst, src, len);

    return dst;
}

void vec_entry_destructor(void *obj)
{
    free(obj);
}


void init_clients(void)
{
    client_states = dictionary_create(NULL, int_compare, 
                                      dict_key_copy_constructor,
                                      dict_key_destructor,
                                      dict_value_copy_constructor,
                                      dict_value_destructor);
}

void init_file_list(void)
{
    g_fn_vec = vector_create(vec_entry_copy_constructor,
                             vec_entry_destructor,
                             NULL);
}

int delete_file(char *fn)
{
    int len = strlen(g_dir_name) + 1 + strlen(fn) + 1;
    char temp[len];
    memset(temp, 0, len);
    sprintf(temp, "%s/%s", g_dir_name, fn);

   return unlink(temp); 
}

FILE *open_file(char *fn, char *op)
{
    int len = strlen(g_dir_name) + 1 + strlen(fn) + 1;
    char temp[len];
    memset(temp, 0, len);
    sprintf(temp, "%s/%s", g_dir_name, fn);

    return fopen(temp, op);
}

void destroy_clients(void)
{
    dictionary_destroy(client_states);
}

void change_to_epoll_write(int client_sock)
{
    // Change the epoll event to OUTPUT
    struct epoll_event ev;
    ev.events  = EPOLLOUT;
    ev.data.fd = client_sock;
    epoll_ctl(g_epfd, EPOLL_CTL_MOD, client_sock, &ev); 
}

void go_to_err_state(client_status *tracker, int client_sock, const char *err_msg)
{
    tracker->state   = WRITING_ERR;
    tracker->buffer_offset = 0;
    tracker->err_msg = err_msg;
    
    change_to_epoll_write(client_sock);
}

void go_to_write_ok(client_status *tracker, int client_sock)
{
    tracker->state = WRITING_OK;
    tracker->buffer_offset = 0;
    
    change_to_epoll_write(client_sock);
}

void command_done(int client_sock)
{
    // delete socket from epoll
    epoll_ctl(g_epfd, EPOLL_CTL_DEL, client_sock, NULL); 
    
    // close the socket
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);

    // remove the state tracker
    dictionary_remove(client_states, &client_sock);
}

void write_ok(int client_sock, client_status *tracker)
{
    char *buffer  = "OK\n";
          buffer += tracker->buffer_offset;
    int   max     = 3 - tracker->buffer_offset; 
    int   count   = server_write_all_to_socket(client_sock, buffer, max);

    if (count == -1) {
        command_done(client_sock);
        return;
    }

    if (count == max) {
        if (tracker->cmd == PUT || tracker->cmd == DELETE) {
            command_done(client_sock);
            return;
        } else if (tracker->cmd == LIST || tracker->cmd == GET) {
            tracker->state = WRITING_SIZE;
            tracker->buffer_offset = 0;
            return;
        }
    }

    tracker->buffer_offset += count;
}

void write_size(int client_sock, client_status *tracker)
{
    char *buffer  = (char*)(&(tracker->file_size));
          buffer += tracker->buffer_offset;
    int   max     = sizeof(size_t) - tracker->buffer_offset;
    int   count   = server_write_all_to_socket(client_sock, buffer, max);

    if (count == -1) {
        command_done(client_sock);
        return;
    }

    if (count == max) {
        if (tracker->cmd == LIST) {
            tracker->state = WRITING_LIST;
            tracker->buffer_offset = 0;
            return;
        } else if (tracker->cmd == GET) {
            tracker->state = WRITING_FILE;
            tracker->buffer_offset = 0;
            return;
        }
    }

    tracker->buffer_offset += count;
}

void write_list(int client_sock, client_status *tracker)
{
    char *buffer  = tracker->ls;
          buffer += tracker->buffer_offset;
    int   max     = strlen(buffer);
    int   count   = server_write_all_to_socket(client_sock, buffer, max);

    if (count == -1) {
        command_done(client_sock);
        return;
    }

    if (count == max) {
        command_done(client_sock);
        return;
    }

    tracker->buffer_offset += count;
}

void write_file(int client_sock, client_status *tracker)
{
    char buffer[4096];

    size_t written_size = ftell(tracker->fh);
    int    rem_size     = 0;
    int    curr_size    = 0;   
    int    count;

    while (written_size < tracker->file_size) {
        rem_size  = tracker->file_size - written_size; 
        curr_size = rem_size > 4096 ? 4096 : rem_size;

        // Read a part of the file into memory
        fread(buffer, 1, curr_size, tracker->fh);
        count = server_write_all_to_socket(client_sock, buffer, curr_size); 
        
        if (count == -1) {
            fclose(tracker->fh);
            command_done(client_sock);
            return;
        }
        
        // Check if there's more data left in buffer
        if (count < curr_size) {
            // need to rewind the file reader position
            fseek(tracker->fh, count - curr_size, SEEK_CUR);
            break; 
        }

        written_size = ftell(tracker->fh);
    }

    // If we wrote the proposed size, close the file
    if (written_size == tracker->file_size) {
        fclose(tracker->fh);
        command_done(client_sock);
        return;
    }
}

void write_err_status(int client_sock, client_status *tracker)
{
    char *buffer  = "ERROR\n";
          buffer += tracker->buffer_offset;
    int   max     = strlen(buffer); 
    int   count   = server_write_all_to_socket(client_sock, buffer, max);

    if (count == -1) {
        command_done(client_sock);
        return;
    }

    if (count == max) {
        tracker->state = WRITING_MSG;
        tracker->buffer_offset = 0;
        return;
    }

    tracker->buffer_offset += count;
}

void write_err_msg(int client_sock, client_status *tracker)
{
    const char *buffer  = tracker->err_msg;
                buffer += tracker->buffer_offset;
    int   max     = strlen(buffer);
    int   count   = server_write_all_to_socket(client_sock, buffer, max);

    if (count == -1) {
        command_done(client_sock);
        return;
    }

    if (count == max) {
        command_done(client_sock);
        return;
    }

    tracker->buffer_offset += count;
}

int parse_header(client_status *tracker)
{
    char verb_str[10];
    memset(verb_str, 0, 10);
    
    int len = strlen(tracker->header);
    tracker->filename = malloc(len);
    memset(tracker->filename, 0, len);
    
    int rc = sscanf(tracker->header, "%s %s", verb_str, tracker->filename);
    
    if (rc == 0) {
        return -1;
    }

    // Determine the VERB
    if (strcmp(verb_str, "LIST") == 0) {
        tracker->cmd = LIST;
    } else if (strcmp(verb_str, "GET") == 0) {
        tracker->cmd = GET;
        if (rc != 2) {
            return -1;
        }
    } else if (strcmp(verb_str, "DELETE") == 0) {
        tracker->cmd = DELETE;
        if (rc != 2) {
            return -1;
        }
    } else if (strcmp(verb_str, "PUT") == 0) {
        tracker->cmd = PUT;
        if (rc != 2) {
            return -1;
        }
        tracker->state = READING_SIZE;
        tracker->buffer_offset = 0;
        memset(tracker->header, 0, HEADER_SIZE);
    } else {
        return -1;
    }

    return 0;
}

void process_list(int client_sock, client_status *tracker)
{
    // This is a bit inefficient, proper implementation is probably to create a tmp file
    int len = 0;
    VECTOR_FOR_EACH(g_fn_vec, node, {
        char *fn = (char*)node;
        len     += strlen(fn) + 1;
    }); 

    tracker->file_size = len - 1; // remove the last \n

    tracker->ls = malloc(len);
    memset(tracker->ls, 0, len);

    // Copy the filenames to this buffer
    char *buffer  = tracker->ls;
    int   size    = vector_size(g_fn_vec);
    int   counter = 0;

    VECTOR_FOR_EACH(g_fn_vec, node, {
        char *fn = (char*)node;
        memcpy(buffer, fn, strlen(fn));
        buffer += strlen(fn);
        counter++;
        if (counter != size) {
            buffer[0] = '\n';
            buffer++;
        } else {
            buffer[0] = 0x0;
        }
    }); 

     
    go_to_write_ok(tracker, client_sock);
}

void process_delete(int client_sock, client_status *tracker)
{
    bool file_exist = false;
    int  index      = 0;
    VECTOR_FOR_EACH(g_fn_vec, node, {
        char *fn = (char*)node;
        if (strcmp(fn, tracker->filename) == 0) {
            file_exist = true;
            break;
        }
        index++;
    }); 

    
    if (!file_exist) {
        go_to_err_state(tracker, client_sock, err_no_such_file);
        return;
    } else {
        // delete the file, and also remove it from the file list
        delete_file(tracker->filename);
        vector_erase(g_fn_vec, index);

        go_to_write_ok(tracker, client_sock);
    }
}

// Open the file in tmp directory for GET verb
void process_file(int client_sock, client_status *tracker)
{
    bool file_exist = false;
    VECTOR_FOR_EACH(g_fn_vec, node, {
        char *fn = (char*)node;
        if (strcmp(fn, tracker->filename) == 0) {
            file_exist = true;
            break;
        }
    }); 

    if (!file_exist) {
        go_to_err_state(tracker, client_sock, err_no_such_file);
        return;
    } else {
        tracker->fh = open_file(tracker->filename, "r");
        if (!(tracker->fh)) {
            go_to_err_state(tracker, client_sock, err_no_such_file);
            return;
        }

        fseek(tracker->fh, 0, SEEK_END);
        tracker->file_size = ftell(tracker->fh);
        fseek(tracker->fh, 0, SEEK_SET);

        go_to_write_ok(tracker, client_sock);
    }
}

// Read the header and parse the information contained in the header
// Also do what can immediately be done within the server without getting blocked by
// IO
void read_header(int client_sock, client_status *tracker)
{
    char *buffer  = tracker->header;
          buffer += tracker->buffer_offset;
    int   max     = HEADER_SIZE - tracker->buffer_offset; 
    int   status  = S_READING;
    int   count   = server_read_from_socket_eol(client_sock, buffer, max, &status);

    if (count == -1) {
        go_to_err_state(tracker, client_sock, err_bad_request);
        return;
    }

    if (status == S_READING) {
        tracker->buffer_offset += count;  
    } else if (status == S_HEADER_COMPLETE) {
        if (parse_header(tracker) == -1) {
            go_to_err_state(tracker, client_sock, err_bad_request);
            return;
        }

        if (tracker->cmd == DELETE) {
            process_delete(client_sock, tracker);
        } else if (tracker->cmd == LIST) {
            process_list(client_sock, tracker);
        } else if (tracker->cmd == GET) {
            process_file(client_sock, tracker);
        }
    }
}

void read_payload_size(int client_sock, client_status *tracker)
{
    int  conn_closed = 0;

    char *buffer  = (char*)(&(tracker->file_size));
          buffer += tracker->buffer_offset;
    int   max     = sizeof(size_t) - tracker->buffer_offset;
    int   count   = server_read_all_from_socket(client_sock, buffer, max, &conn_closed);

    if (count == -1) {
        go_to_err_state(tracker, client_sock, err_bad_file_size);
        return;
    }

    if (count == max) {
        tracker->state = PROCESSING_PUT;     
        tracker->fh    = open_file(tracker->filename, "w"); 
        if (!(tracker->fh)) {
            go_to_err_state(tracker, client_sock, err_no_such_file);
            return;
        }
        
        // Need to drain the socket
    } else {
        tracker->buffer_offset += count;
    }
}

// Read as much from the socket as possible
void read_payload(int client_sock, client_status *tracker)
{
    char buffer[4096];
    size_t read_size = ftell(tracker->fh);
    int  rem_size  = tracker->file_size - read_size;
    int  curr_size = 0;   
    int  count;
    int  conn_closed = 0;

    while (read_size < tracker->file_size) {
        memset(buffer, 0, 4096);
        rem_size  = tracker->file_size  - read_size;
        curr_size = rem_size > 4096 ? 4096 : rem_size;
        conn_closed = 0;
        count = server_read_all_from_socket(client_sock, buffer, curr_size, &conn_closed); 
        
        if (count == -1 || conn_closed == 1) {
            // PUT operation failed, delete the half-processed file
            fclose(tracker->fh);
            delete_file(tracker->filename);
            go_to_err_state(tracker, client_sock, err_bad_file_size);
            return;
        }
        
        // write to file all the bytes that we've read so far
        fwrite(buffer, 1, count, tracker->fh);

        // Check if there's more data left in buffer
        if (count < curr_size) {
            break; 
        }

        read_size = ftell(tracker->fh);
    }

    // If we read the proposed size, close the file and check if there is more to read
    if (read_size == tracker->file_size) {
        fclose(tracker->fh);

        count = server_read_all_from_socket(client_sock, buffer, 1024, &conn_closed);
        // Received too much data
        if (count != 0) {
            delete_file(tracker->filename);
            go_to_err_state(tracker, client_sock, err_bad_file_size);
            return;
        }
        
        // File successfully written
        vector_push_back(g_fn_vec, tracker->filename);
        go_to_write_ok(tracker, client_sock);
    }
}

void process_client(int client_sock)
{
    if (!dictionary_contains(client_states, &client_sock)) {
        fprintf(stderr, "ERROR: client_state does not exist in dictionary\n");
        exit(1);
    }

    client_status *tracker = (client_status*)dictionary_get(client_states, &client_sock);

    if (tracker->state == READING_HEADER) {
        read_header(client_sock, tracker);
    } else if (tracker->state == READING_SIZE) {
        read_payload_size(client_sock, tracker);
    } else if (tracker->state == PROCESSING_PUT) {
        read_payload(client_sock, tracker);
    } else if (tracker->state == WRITING_OK) {
        write_ok(client_sock, tracker);
    } else if (tracker->state == WRITING_ERR) {
        write_err_status(client_sock, tracker);
    } else if (tracker->state == WRITING_MSG) {
        write_err_msg(client_sock, tracker);
    } else if (tracker->state == WRITING_SIZE) {
        write_size(client_sock, tracker);
    } else if (tracker->state == WRITING_LIST) {
        write_list(client_sock, tracker);
    } else if (tracker->state == WRITING_FILE) {
        write_file(client_sock, tracker);
    }
}

void run_server(char *port)
{
    // socket()
    int rc;
    int listen_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    
    // bind()
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    rc = getaddrinfo(NULL, port, &hints, &result);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        exit(1);
    }

    int optval;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    if (bind(listen_sock, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }

    freeaddrinfo(result);

    // listen()
    if (listen(listen_sock, MAX_CLIENTS)) {
        perror("listen()");
        exit(1);
    }

    // create epoll related data structures
    g_epfd = epoll_create1(0);
    if (g_epfd == -1) {
        perror("epoll_create1()");
        exit(1);
    }

    // create epoll_event data structures
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];

    // Some of the epoll code is referenced from:
    //      http://man7.org/linux/man-pages/man7/epoll.7.html
    ev.events  = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(g_epfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(1);
    }
    
    int nfds;

    client_status temp_st;

    // initialize the client state data structure
    init_clients();

    while (true) {
        // wait for a trigger
        nfds = epoll_wait(g_epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(1);
        }

        // process all triggered events
        for (int n=0; n<nfds; n++) {
            if (events[n].data.fd == listen_sock) {
                int client_sock = accept(listen_sock, NULL, NULL);
                if (client_sock == -1) {
                    perror("accept");
                    exit(1);
                }

                // set the client file descriptor to non-blocking
                int flags = fcntl(client_sock, F_GETFL, 0);
                fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);

                // add the new client connection to epoll_fd
                ev.events  = EPOLLIN;
                ev.data.fd = client_sock; 
                if (epoll_ctl(g_epfd, EPOLL_CTL_ADD, client_sock, &ev) == -1) {
                    perror("epoll_ctl: client_sock");
                    exit(1);
                }

                // add an entry to the client state
                memset(&temp_st, 0, sizeof(client_status));
                temp_st.state = READING_HEADER;
                temp_st.buffer_offset = 0;
                dictionary_set(client_states, &client_sock, &temp_st);
            } else {
                process_client(events[n].data.fd);
            }
        }
    }
}

void close_server()
{
    if (vector_size(g_fn_vec) > 0) {
        VECTOR_FOR_EACH(g_fn_vec, node, {
            char *fn = (char*)node;
            delete_file(fn);
        }); 
    }

    rmdir(g_dir_name);
    
    vector_destroy(g_fn_vec);
    destroy_clients();
    
    close(g_epfd);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "./server <port>\n"); 
        return -1;
    }

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    char template[] = "XXXXXX";
    g_dir_name = mkdtemp(template);
    print_temp_directory(g_dir_name);
    init_file_list();

    run_server(argv[1]);
    return 0;
}
