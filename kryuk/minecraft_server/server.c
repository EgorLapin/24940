#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <aio.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define SOCKET_PATH "/tmp/server_socket"
#define MAX_CLIENTS 4

typedef struct {
    int fd;
    struct aiocb aio;
    char buffer[BUFFER_SIZE];
    int active;
    int pending;
} client_info_t;

int main(void) {
    int server_fd;
    struct sockaddr_un server_addr;
    fd_set read_fds, master_fds;
    client_info_t clients[MAX_CLIENTS];
    int max_fd;
    int i;
    
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    unlink(SOCKET_PATH);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }
    
    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    
    int had_clients = 0;
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].active = 0;
        clients[i].pending = 0;
        memset(&clients[i].aio, 0, sizeof(struct aiocb));
    }
    
    while (1) {
        read_fds = master_fds;
        
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        
        if (select(server_fd + 1, &read_fds, NULL, NULL, &timeout) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            break;
        }
        
        if (FD_ISSET(server_fd, &read_fds)) {
            struct sockaddr_un client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_client = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            
            if (new_client == -1) {
                perror("accept");
            } else {
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == -1) {
                        clients[i].fd = new_client;
                        clients[i].active = 1;
                        clients[i].pending = 0;
                        
                        memset(&clients[i].aio, 0, sizeof(struct aiocb));
                        clients[i].aio.aio_fildes = new_client;
                        clients[i].aio.aio_buf = clients[i].buffer;
                        clients[i].aio.aio_nbytes = BUFFER_SIZE - 1;
                        clients[i].aio.aio_offset = 0;
                        
                        if (aio_read(&clients[i].aio) == -1) {
                            perror("aio_read");
                            close(new_client);
                            clients[i].fd = -1;
                            clients[i].active = 0;
                        } else {
                            clients[i].pending = 1;
                            had_clients = 1;
                        }
                        break;
                    }
                }
                if (i == MAX_CLIENTS) {
                    close(new_client);
                }
            }
        }
        
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd != -1 && clients[i].pending) {
                int error = aio_error(&clients[i].aio);
                
                if (error == EINPROGRESS) {
                    continue;
                }
                
                clients[i].pending = 0;
                ssize_t nbytes = aio_return(&clients[i].aio);
                
                if (nbytes <= 0) {
                    if (nbytes == -1) {
                        perror("aio_return");
                    }
                    close(clients[i].fd);
                    clients[i].fd = -1;
                    clients[i].active = 0;
                } else {
                    clients[i].buffer[nbytes] = '\0';
                    
                    time_t now;
                    struct tm *timeinfo;
                    char timestamp[64];
                    
                    time(&now);
                    timeinfo = localtime(&now);
                    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", timeinfo);
                    
                    for (int j = 0; j < nbytes; j++) {
                        clients[i].buffer[j] = toupper((unsigned char)clients[i].buffer[j]);
                    }
                    
                    printf("%s%s", timestamp, clients[i].buffer);
                    fflush(stdout);
                    
                    memset(&clients[i].aio, 0, sizeof(struct aiocb));
                    clients[i].aio.aio_fildes = clients[i].fd;
                    clients[i].aio.aio_buf = clients[i].buffer;
                    clients[i].aio.aio_nbytes = BUFFER_SIZE - 1;
                    clients[i].aio.aio_offset = 0;
                    
                    if (aio_read(&clients[i].aio) == -1) {
                        if (errno != EINPROGRESS) {
                            perror("aio_read");
                            close(clients[i].fd);
                            clients[i].fd = -1;
                            clients[i].active = 0;
                        } else {
                            clients[i].pending = 1;
                        }
                    } else {
                        clients[i].pending = 1;
                    }
                }
            }
        }
        
        int active_clients = 0;
        int pending_operations = 0;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd != -1) {
                active_clients++;
            }
            if (clients[i].pending) {
                pending_operations++;
            }
        }
        
        if (had_clients && active_clients == 0 && pending_operations == 0) {
            break;
        }
    }
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd != -1) {
            aio_cancel(clients[i].fd, NULL);
            close(clients[i].fd);
        }
    }
    
    close(server_fd);
    unlink(SOCKET_PATH);
    
    exit(EXIT_SUCCESS);
}

