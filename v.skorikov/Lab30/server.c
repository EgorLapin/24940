#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#define SOCKET_PATH "my_socket"
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];
    ssize_t num_read;

    // Создание сокета
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    // Привязка к пути
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);  // Удалить если уже есть файл сокета с привязкой
    
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Прослушивание
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    // Принятие клиента
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept");
        exit(1);
    }

    // Чтение данных в цикле (неограниченный размер)
    while ((num_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t i = 0; i < num_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        write(STDOUT_FILENO, buffer, num_read);  // Вывод в stdout
    }

    if (num_read == -1) {
        perror("read");
    }

    // Завершение
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}