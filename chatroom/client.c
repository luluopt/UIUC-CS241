/**
 * Networking
 * CS 241 - Fall 2017
 */
#include "common.h"
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#define HOSTNAME_IDX   0
#define PORT_IDX       1
#define VERB_IDX       2
#define REMOTEFILE_IDX 3
#define LOCALFILE_IDX  4


char **parse_args(int argc, char **argv);
verb check_args(char **args);

/**
 * Sets up a connection to a chatroom server and returns
 * the file descriptor associated with the connection.
 *
 * host - Server to connect to.
 * port - Port to connect to server on.
 *
 * Returns integer of valid file descriptor, or exit(1) on failure.
 */
// Disclaimer: this code is taken from the Chatroom Lab
int connect_to_server(const char *host, const char *port)
{
    // Get a fd number for the client socket
    int rc;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        fprintf(stderr, "ERROR: failed to create client socket\n");
        exit(1);
    }

    // Get address info of the host and port we want to connect to
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;  // TCP

    rc = getaddrinfo(host, port, &hints, &result);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        exit(1);
    }

    // Connect the client socket to the server
    if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("connect()");
        exit(1);
    }
    
    // No longer needs address info
    freeaddrinfo(result);
     
    // If we get to here, connection is successful
    return sock_fd;
}

// Allocate space for the protocol header and contruct it
char *construct_protocol_header(verb cmd, char**args)
{
    char *header = NULL;
    if (cmd == LIST) {
        size_t len = strlen(args[VERB_IDX]) + 1 + 1;  // \n and null termination
        header     = malloc(len);
        memset(header, 0, len); 
        sprintf(header, "%s\n", args[VERB_IDX]);
    } else {
        size_t len = strlen(args[VERB_IDX]) + 1 + strlen(args[REMOTEFILE_IDX]) + 1 + 1;
        header = malloc(len);
        memset(header, 0, len);
        sprintf(header, "%s %s\n", args[VERB_IDX], args[REMOTEFILE_IDX]);
    }
    
    return header;
}

// Send the protocol header through the socket
int send_protocol_header(int sock, verb cmd, char **args)
{
    char *header = construct_protocol_header(cmd, args);
    size_t  len  = strlen(header); 
    ssize_t rc   = write_all_to_socket(sock, header, len); 
    
    free(header);
    
    // Check if write is successful
    if (rc > 0) {
        return 0;
    } else {
        fprintf(stderr, "ERROR: write header failed\n");
        exit(1);
    }
}

// Send the protocol body
int send_protocol_body(int sock, verb cmd, char **args)
{
    if (cmd == PUT) {
        FILE   *localfile = fopen(args[LOCALFILE_IDX], "r");
        fseek(localfile, 0, SEEK_END);
        size_t  size      = ftell(localfile);
        fseek(localfile, 0, SEEK_SET);

        write_payload_size(sock, size);

        size_t written_size = 0;
        size_t rem_size     = size;
        size_t curr_size    = 0;
        char   buffer[4097];
        memset(buffer, 0, 4097);
       
        while (written_size < size) {
            curr_size = rem_size > 4096 ? 4096 : rem_size;

            fread(buffer, 1, curr_size, localfile);

            int rc = write_all_to_socket(sock, buffer, curr_size);
            handle_rc(rc, written_size, size);
            
            rem_size     -= rc;
            written_size += rc;
            printf("fiel is at %ld\n", ftell(localfile));
        }

        fclose(localfile);

        return 0;
    } else {
        // LIST, GET, DELETE doesn't have a payload
        return 0;
    }
}

// Send entire protocol transaction through the socket
int send_cmd(int sock, verb cmd, char **args)
{
    // Write the header to the socket
    send_protocol_header(sock, cmd, args);
    // Write the body to the socket (if needed)
    send_protocol_body(sock, cmd, args);

    return 0;
}

// Read from the socket for the response status
int read_response_status(int sock)
{
    char buffer[1025];
    memset(buffer, 0, 1025);

    // Read from the socket until newline
    read_from_socket_eol(sock, buffer, 1024);

    // Check the status
    if (strcmp(buffer, "OK") == 0) {
        return 0;
    } else if (strcmp(buffer, "ERROR") == 0) {
        return 1; 
    } else {
        print_invalid_response();
        exit(1);
        return -1;
    }
}

// Read from the socket for the error message
void read_error_message(int sock)
{
    // Only need to read error messages if status is ERROR
    char buffer[1025];
    memset(buffer, 0, 1025);

    // Read from socket until newline
    read_from_socket_eol(sock, buffer, 1024);
    
    print_error_message(buffer); 
}

// Read the response body, also decides what to do with it based on the CMD
void read_resp_body(int sock, verb cmd, char **args)
{
    if (cmd == DELETE || cmd == PUT) {
        print_success();
        return;
    }

    size_t size = get_payload_size(sock);
    size_t read_size = 0;
    size_t rem_size = size;
    size_t curr_size = 0;

    ssize_t rc = 0;
    
    // Read 4KB from socket everytime
    char buffer[4097];

    if (cmd == LIST) {
        while (read_size < size) {
            memset(buffer, 0, 4097);
            curr_size = rem_size > 4096 ? 4096 : rem_size;
            rc = read_all_from_socket(sock, buffer, curr_size);
            handle_rc(rc, read_size, size);
            
            rem_size  -= rc;
            read_size += rc;
            printf("%s", buffer);
        }
        printf("\n");
    } else if (cmd == GET) {
        FILE *localfile = fopen(args[LOCALFILE_IDX], "w");
        
        while (read_size < size) {
            memset(buffer, 0, 4097);
            curr_size = rem_size > 4096 ? 4096 : rem_size;
            rc = read_all_from_socket(sock, buffer, curr_size);
            handle_rc(rc, read_size, size);
            
            rem_size  -= rc;
            read_size += rc;
            fwrite(buffer, 1, rc, localfile);
        }
        fclose(localfile);
    }
}

// Read the server response from the socket
int read_response(int sock, verb cmd, char **args)
{
    int status = read_response_status(sock); 

    if (status == 0) {
        read_resp_body(sock, cmd, args);
    } else {
        read_error_message(sock);
    }

    return 0;
}

int main(int argc, char **argv)
{
    char **args = parse_args(argc, argv);
    verb   cmd  = check_args(args);

    int server_sock = connect_to_server(args[HOSTNAME_IDX], args[PORT_IDX]);

    send_cmd(server_sock, cmd, args);

    // Shutdown the write connection after we sent our command
    if (shutdown(server_sock, SHUT_WR) != 0) {
        perror("shutdown()");
    }

    read_response(server_sock, cmd, args);

    // Close the socket fd after everything is done
    close(server_sock);

    // Free memories
    free(args);
    return 0;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
