#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 4096

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];
    ssize_t num_read;

    unlink(SOCKET_PATH);

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен, ожидает подключения на %s...\n", SOCKET_PATH);

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    printf("Клиент подключился\n");

    while ((num_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t i = 0; i < num_read; i++) {
            buffer[i] = toupper((unsigned char)buffer[i]);
        }
        write(STDOUT_FILENO, buffer, num_read);
    }

    if (num_read == -1) {
        perror("read");
    }

    printf("Клиент отключился\n");

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}