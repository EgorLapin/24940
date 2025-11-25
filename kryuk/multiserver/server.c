#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define SOCKET_PATH "/tmp/server_socket"
#define MAX_CLIENTS 4

int main(void) {
    int server_fd;
    struct sockaddr_un server_addr;
    fd_set read_fds, master_fds;
    int client_fds[MAX_CLIENTS];
    int max_fd;
    int i, nbytes;
    char buffer[BUFFER_SIZE];
    
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
    max_fd = server_fd;
    
    int had_clients = 0;
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_fds[i] = -1;
    }
    
    while (1) {
        read_fds = master_fds;
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
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
                    if (client_fds[i] == -1) {
                        client_fds[i] = new_client;
                        FD_SET(new_client, &master_fds);
                        if (new_client > max_fd) {
                            max_fd = new_client;
                        }
                        had_clients = 1;
                        break;
                    }
                }
                if (i == MAX_CLIENTS) {
                    close(new_client);
                }
            }
        }
        
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] != -1 && FD_ISSET(client_fds[i], &read_fds)) {
                nbytes = read(client_fds[i], buffer, BUFFER_SIZE - 1);
                
                if (nbytes <= 0) {
                    if (nbytes == -1) {
                        perror("read");
                    }
                    close(client_fds[i]);
                    FD_CLR(client_fds[i], &master_fds);
                    client_fds[i] = -1;
                } else {
                    // Capture processing time (when server processes the message, not when it was sent)
                    time_t now;
                    struct tm *timeinfo;
                    char timestamp[64];
                    
                    time(&now);
                    timeinfo = localtime(&now);
                    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", timeinfo);
                    
                    buffer[nbytes] = '\0';
                    
                    for (int j = 0; j < nbytes; j++) {
                        buffer[j] = toupper((unsigned char)buffer[j]);
                    }
                    
                    printf("%s%s", timestamp, buffer);
                    fflush(stdout);
                }
            }
        }
        
        int active_clients = 0;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] != -1) {
                active_clients++;
            }
        }
        
        if (had_clients && active_clients == 0) {
            break;
        }
    }
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_fds[i] != -1) {
            close(client_fds[i]);
        }
    }
    
    close(server_fd);
    unlink(SOCKET_PATH);
    
    exit(EXIT_SUCCESS);
}