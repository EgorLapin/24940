#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>

#define SOCKET_PATH "/tmp/server_socket"

int main(void) {
    int sock_fd;
    struct sockaddr_un server_addr;
    ssize_t bytes_written;
    
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    
    const char *message = "hello\n";
    size_t message_len = strlen(message);
    struct timespec start_time, current_time;
    
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        
        // Check if 2 seconds have elapsed
        long elapsed_ms = (current_time.tv_sec - start_time.tv_sec) * 1000L +
                         (current_time.tv_nsec - start_time.tv_nsec) / 1000000L;
        if (elapsed_ms >= 2000) {
            break;
        }
        
        bytes_written = write(sock_fd, message, message_len);
        if (bytes_written == -1) {
            perror("write");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        if (bytes_written != (ssize_t)message_len) {
            fprintf(stderr, "Partial write: wrote %zd of %zu bytes\n", bytes_written, message_len);
        }
        
        usleep(10000); // Sleep for 10 ms
    }

    close(sock_fd);
    
    exit(EXIT_SUCCESS);
}

