#include <syslog.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h> // https://man7.org/linux/man-pages/man7/ip.7.html
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#define GNU_SOURCE // needed for accept4
// #include <poll.h>

#define BIND_PORT 9000
#define MAX_CONNECTIONS 1
#define BUFFER_SIZE 1024
#define SOCKET_DATA_FILE "/var/tmp/aesdsocketdata"

struct file_params{
    int sockfd;
    int accepted_sockfd;
    int socket_file_fd;
};

struct file_params glob_params;

bool caught_sigint = false;
bool caught_sigterm = false;

char buffer[BUFFER_SIZE] = {0};

void signal_handler(int signum); // https://man7.org/linux/man-pages/man2/sigaction.2.html
void cleanup(struct file_params* fp);

int main(int argc, char* argv[]){
    ssize_t nbytes_socket;
    ssize_t nbytes_file;
    ssize_t buf_end_ptr;

    /*
     * NULL -> ident is NULL -> default id string is argv[0] (program name)
     * 0 -> no options flags set (such as LOG_PID, LOG_NDELAY, LOG_PERROR or LOG_CONS)
     */
    openlog(NULL, 0, LOG_USER);

    // Signal handler setup:
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = signal_handler;

    if (sigaction(SIGINT, &act, NULL) == -1){
        syslog(LOG_ERR, "Could not register signal handler for SIGINT.");
        return -1;
    }

    if (sigaction(SIGTERM, &act, NULL) == -1){
        syslog(LOG_ERR, "Could not register signal handler for SIGKILL.");
        return -1;
    }

    // Socket: https://linux.die.net/man/2/socket
    glob_params.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (glob_params.sockfd == -1){
        syslog(LOG_ERR, "Could not get socket file descriptor.");
        return -1;
    }

    // https://linux.die.net/man/2/setsockopt
    int enable = 1;
    if (setsockopt(glob_params.sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1){
        syslog(LOG_ERR, "Could not set socket option to reuse address.");
        return -1;
    }

    // Bind: https://man7.org/linux/man-pages/man2/bind.2.html
    struct sockaddr_in socket_address = {.sin_family = AF_INET,
                                         .sin_addr.s_addr = htonl(INADDR_ANY),
                                         .sin_port = htons(BIND_PORT)};
    if (bind(glob_params.sockfd, (struct sockaddr*) &socket_address, sizeof(socket_address)) == -1){
        syslog(LOG_ERR, "Could not bind socket.");
        return -1;
    }

    // Listen: https://man7.org/linux/man-pages/man2/listen.2.html
    if (listen(glob_params.sockfd, MAX_CONNECTIONS) == -1){
        syslog(LOG_ERR, "Error listening to socket.");
        return -1;
    }

    glob_params.socket_file_fd = open(SOCKET_DATA_FILE, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (glob_params.socket_file_fd == -1){
        syslog(LOG_ERR, "Could not open data file at %s\n", SOCKET_DATA_FILE);
        return -1;
    }

    int conn_count = 0;
    while (1){
        // Poll: https://man7.org/linux/man-pages/man2/poll.2.html
        // struct pollfd pfd = {.fd = sockfd, .events = POLLIN}

        // Accept: https://linux.die.net/man/2/accept
        // Behavior now is as blocking.
        struct sockaddr_in connect_address;
        socklen_t addrlen = sizeof(connect_address);
        glob_params.accepted_sockfd = accept(glob_params.sockfd, (struct sockaddr*) &connect_address, &addrlen);
        if (glob_params.accepted_sockfd == -1){
            syslog(LOG_ERR, "Error accepting from socket.");
            return -1;
        }

        conn_count++;
    
        // inet: https://man7.org/linux/man-pages/man3/inet.3.html
        syslog(LOG_INFO, "Accepted connection from %s\n", inet_ntoa(connect_address.sin_addr));
        printf("[%d] Accepted connection from %s\n", conn_count, inet_ntoa(connect_address.sin_addr));
        printf("[%d] Identifier for accepted socket = %d\n", conn_count, glob_params.accepted_sockfd);

        
        // Recv/Send: https://man7.org/linux/man-pages/man2/recv.2.html
        nbytes_socket = recv(glob_params.accepted_sockfd, buffer, BUFFER_SIZE, 0);
        if (nbytes_socket == -1){
            syslog(LOG_ERR, "Error receiving from socket %d. Exiting!\n", glob_params.accepted_sockfd);
            return -1;
        }

        if (nbytes_socket > 0){
            buf_end_ptr = nbytes_socket - 1;
        }

        printf("[%d] Received %li bytes from socket %d\n",
                conn_count, nbytes_socket, glob_params.accepted_sockfd);

        nbytes_file = write(glob_params.socket_file_fd, buffer, nbytes_socket);
        if (nbytes_file == -1){
            syslog(LOG_ERR, "Error writing %li bytes to file %s\n", nbytes_socket, SOCKET_DATA_FILE);
            // close(sockfd);
            // close(accepted_sockfd);
            // close(socket_file_fd);
            // // remove(SOCKET_DATA_FILE);
        }

        int rc_sync = fsync(glob_params.socket_file_fd);
        if (rc_sync == -1){
            // Add error handling here
            // syslog
        }

        if (buffer[buf_end_ptr] == '\n'){
            printf("[%d] Found end of data symbol, closing connection.\n", conn_count);

            // Read in all of file:

            lseek(glob_params.socket_file_fd, 0, SEEK_SET); // Go to beginning of file
            do{
                nbytes_file = read(glob_params.socket_file_fd, buffer, BUFFER_SIZE); // Read in up to BUFFER_SIZE bytes
                if (nbytes_file == -1){
                    // Add error handling
                }
                printf("[%d] Got %li bytes from file\n", conn_count, nbytes_file);
                send(glob_params.accepted_sockfd, buffer, nbytes_file, 0); // Send back nbytes_file to socket
            } while (nbytes_file > 0);

            close(glob_params.accepted_sockfd); // need to gracefully close from all the errors!
            syslog(LOG_INFO, "Closed connection from %s\n", inet_ntoa(connect_address.sin_addr));
            printf("[%d] Closed connection from %s\n", conn_count, inet_ntoa(connect_address.sin_addr));
        }

        // if (caught_sigint || caught_sigterm){
        //     syslog(LOG_INFO, "Caught signal, exiting");
        //     printf("Caught signal, exiting");
        //     cleanup(&glob_params);
        //     // if (caught_sigint) printf("Caught SIGINT. EXIT.\n");
        //     // if (caught_sigterm) printf("Caught SIGTERM. EXIT.\n");
        //     return 0;
        // }
    }

    cleanup(&glob_params);
    return 0;
}

void signal_handler(int signal_number){
    syslog(LOG_INFO, "Caught signal, exiting"); // this is probably not safe, fix
    if (signal_number == SIGINT){
        caught_sigint = true;
    }
    if (signal_number == SIGTERM){
        caught_sigterm = true;
    }
    printf("Caught signal, exiting\n");
    cleanup(&glob_params);
    exit(0);
}

void cleanup(struct file_params* fp){
    close(fp->sockfd);
    close(fp->accepted_sockfd);
    close(fp->socket_file_fd);
    remove(SOCKET_DATA_FILE);
}
