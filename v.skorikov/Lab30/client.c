#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define SOCKET_PATH "my_socket"
#define BUFFER_SIZE 1024

int main() {
    int client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];
    ssize_t num_read;

    // Создание сокета
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        exit(1);
    }

    // Подключение
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(1);
    }

    // Чтение из stdin и отправка (неограниченный размер)
    while ((num_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        write(client_fd, buffer, num_read);
    }

    if (num_read == -1) {
        perror("read");
    }

    // Завершение
    close(client_fd);
    return 0;
}