#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define SOCKET_PATH "my_socket"
#define BUFFER_SIZE 40960
#define MAX_CLIENTS 10

typedef struct {
    int fd;
    char buffer[BUFFER_SIZE];
    size_t buf_len;
    int id;           // номер клиента для логов
} Client;

Client clients[MAX_CLIENTS];

void print_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", localtime(&ts.tv_sec));
    printf("[%s.%03ld] ", time_buf, ts.tv_nsec / 1000000);
}

int main() {
    int server_fd, max_fd;
    struct sockaddr_un addr;
    fd_set readfds;
    char temp_buf[BUFFER_SIZE];
    ssize_t num_read;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].buf_len = 0;
        clients[i].id = i + 1;
    }

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket"); exit(1); }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) { perror("bind"); exit(1); }
    if (listen(server_fd, 5) == -1) { perror("listen"); exit(1); }

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    printf("Task 31 server (select + line buffering) running...\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd != -1) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > max_fd) max_fd = clients[i].fd;
            }
        }

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) continue;
            perror("select"); exit(1);
        }

        // Новое соединение
        if (FD_ISSET(server_fd, &readfds)) {
            int new_fd = accept(server_fd, NULL, NULL);
            if (new_fd != -1) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == -1) {
                        clients[i].fd = new_fd;
                        clients[i].buf_len = 0;
                        fcntl(new_fd, F_SETFL, O_NONBLOCK);
                        break;
                    }
                }
            }
        }

        // Обработка клиентов
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd == -1 || !FD_ISSET(clients[i].fd, &readfds)) continue;

            num_read = read(clients[i].fd, temp_buf, BUFFER_SIZE);
            if (num_read > 0) {
                memcpy(clients[i].buffer + clients[i].buf_len, temp_buf, num_read);
                clients[i].buf_len += num_read;

                char *line_start = clients[i].buffer;
                char *newline;
                while ((newline = memchr(line_start, '\n', clients[i].buf_len - (line_start - clients[i].buffer))) != NULL) {
                    size_t line_len = newline - line_start + 1;

                    print_time();
                    printf("[Client %d] ", clients[i].id);
                    for (size_t j = 0; j < line_len; j++) {
                        char c = toupper((unsigned char)line_start[j]);
                        putchar(c);
                    }

                    memmove(line_start, newline + 1, clients[i].buf_len - (newline + 1 - clients[i].buffer));
                    clients[i].buf_len -= line_len;
                    line_start = clients[i].buffer;
                }
            } else if (num_read == 0) {
                if (clients[i].buf_len > 0) {
                    print_time();
                    printf("[Client %d] ", clients[i].id);
                    for (size_t j = 0; j < clients[i].buf_len; j++) {
                        putchar(toupper((unsigned char)clients[i].buffer[j]));
                    }
                    putchar('\n');
                }
                close(clients[i].fd);
                clients[i].fd = -1;
            }
        }
    }
    return 0;
}