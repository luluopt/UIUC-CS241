/**
 * Chatroom Lab
 * CS 241 - Fall 2017
 */
 
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "utils.h"

#define MAX_CLIENTS 8

void *process_client(void *p);

static volatile int endSession;

static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int serverSocket = 0;

/**
 * Signal handler for SIGINT.
 * Used to set flag to end server.
 */
void close_server() {
    endSession = 1;
    // add any additional flags here you want.
}

/**
 * Cleanup function called in main after `run_server` exits.
 * Server ending clean up (such as shutting down clients) should be handled
 * here.
 */
void cleanup() {
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown(): ");
            }
            close(clients[i]);
        }
    }
}

// Look through the client_fds and find one that has not been taken yet
int find_free_client()
{
    // a client_fd == -1 means it has not been taken
    for (int i=0; i<MAX_CLIENTS; i++) {
        if (clients[i] == -1) {
            return i;
        }
    }

    return -1;
}

/**
 * Sets up a server connection.
 * Does not accept more than MAX_CLIENTS connections.  If more than MAX_CLIENTS
 * clients attempts to connects, simply shuts down
 * the new client and continues accepting.
 * Per client, a thread should be created and 'process_client' should handle
 * that client.
 * Makes use of 'endSession', 'clientsCount', 'client', and 'mutex'.
 *
 * port - port server will run on.
 *
 * If any networking call fails, the appropriate error is printed and the
 * function calls exit(1):
 *    - fprtinf to stderr for getaddrinfo
 *    - perror() for any other call
 */
void run_server(char *port)
{
    int rc;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    // Get the network address for the server socket
    rc = getaddrinfo(NULL, port, &hints, &result);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        exit(1);
    }

    // Set socket option so port can be reused immediately
    int optval;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    // Bind the socket with a its local address
    if (bind(serverSocket, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }

    // Mark the server socket as a passive socket
    if (listen(serverSocket, MAX_CLIENTS + 2)) {
        perror("listen()");
        exit(1);
    }

    // Initialize client count, all client fd are initialized as -1
    clientsCount = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }

    // Free the addrinfo data structure since we dont need it anymore
    freeaddrinfo(result);
    
    // Init mutex
    endSession = 0;
    pthread_mutex_init(&mutex, NULL);

    pthread_t client_threads[MAX_CLIENTS];

    // Waiting for client connections in an infinite loop, the server is terminated with SIGINT
    while (true) {
        int client_fd       = accept(serverSocket, NULL, NULL);
        // Lock the clients fd data structure because other threads might access it
        pthread_mutex_lock(&mutex);
        if (clientsCount > MAX_CLIENTS) {
            // if already more than MAX_CLIENTS, close client_fd and go back to accept()
            close(client_fd);
            pthread_mutex_unlock(&mutex);
            continue;
        }

        // if less than MAX_CLIENTS
        intptr_t client_idx     = find_free_client();
        clients[client_idx]     = client_fd;
        clientsCount           += 1;
        pthread_mutex_unlock(&mutex);
        
        // Spawn off the pthread for the connected client
        pthread_create(&(client_threads[client_idx]), NULL, process_client, (void*)client_idx);

        if (endSession == 1) {
            break;
        }
    }
}

/**
 * Broadcasts the message to all connected clients.
 *
 * message  - the message to send to all clients.
 * size     - length in bytes of message to send.
 */
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0) {
                retval = write_all_to_socket(clients[i], message, size);
            }
            if (retval == -1) {
                perror("write(): ");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * Handles the reading to and writing from clients.
 *
 * p  - (void*)intptr_t index where clients[(intptr_t)p] is the file descriptor
 * for this client.
 *
 * Return value not used.
 */
void *process_client(void *p) {
    pthread_detach(pthread_self());
    intptr_t clientId = (intptr_t)p;
    ssize_t retval = 1;
    char *buffer = NULL;

    while (retval > 0 && endSession == 0) {
        retval = get_message_size(clients[clientId]);
        if (retval > 0) {
            buffer = calloc(1, retval);
            retval = read_all_from_socket(clients[clientId], buffer, retval);
        }
        if (retval > 0)
            write_to_clients(buffer, retval);

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);

    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {

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

    // signal(SIGINT, close_server);
    run_server(argv[1]);
    cleanup();
    pthread_exit(NULL);
}
