#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/server_socket"
#define BUFFER_SIZE 1024

int main(void) {
    int sock_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;
    
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
    
    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(sock_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            perror("write");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Partial write: wrote %zd of %zd bytes\n", bytes_written, bytes_read);
        }
    }
    
    if (bytes_read == -1) {
        perror("read");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    close(sock_fd);
    
    exit(EXIT_SUCCESS);
}

