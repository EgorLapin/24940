#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

static int server_fd = -1;
static int client_fds[MAX_CLIENTS];
static char buffer[BUFFER_SIZE];

void add_client(int client_fd)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_fds[i] == -1)
        {
            client_fds[i] = client_fd;
            return;
        }
    }

    fprintf(stderr, "Too many clients, closing %d\n", client_fd);
    close(client_fd);
}

void remove_client(int index)
{
    if (client_fds[index] != -1)
    {
        close(client_fds[index]);
        client_fds[index] = -1;
    }
}

void sigio_handler(int signo)
{
    if (signo != SIGIO)
    {
        return;
    }

    while (1)
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            perror("Accept failed");
            break;
        }

        printf("Client %d connected\n", client_fd);

        int flags = fcntl(client_fd, F_GETFL);
        if (flags == -1)
        {
            perror("fcntl F_GETFL failed");
            close(client_fd);
            continue;
        }

        if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC) == -1)
        {
            perror("fcntl F_SETFL failed");
            close(client_fd);
            continue;
        }

        if (fcntl(client_fd, F_SETOWN, getpid()) == -1)
        {
            perror("fcntl F_SETOWN failed for client");
            close(client_fd);
            continue;
        }

        add_client(client_fd);
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int fd = client_fds[i];
        if (fd == -1)
        {
            continue;
        }

        while (1)
        {
            ssize_t read_bytes = read(fd, buffer, BUFFER_SIZE - 1);
            if (read_bytes > 0)
            {
                buffer[read_bytes] = '\0';
                for (int j = 0; buffer[j] != '\0'; j++)
                {
                    buffer[j] = toupper((unsigned char)buffer[j]);
                }
                printf("Client %d input in upper case: %s", fd, buffer);
            }
            else if (read_bytes == 0)
            {
                printf("Client %d disconnected\n", fd);
                remove_client(i);
                break;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break;
                }
                perror("Read error");
                remove_client(i);
                break;
            }
        }
    }
}

int main()
{
    struct sockaddr_un address;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_fds[i] = -1;
    }

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_PATH);
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) == -1)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(server_fd, F_GETFL);
    if (flags == -1)
    {
        perror("fcntl F_GETFL failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC) == -1)
    {
        perror("fcntl F_SETFL failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (fcntl(server_fd, F_SETOWN, getpid()) == -1)
    {
        perror("fcntl F_SETOWN failed for server");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigio_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGIO, &sa, NULL) == -1)
    {
        perror("sigaction failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        pause();
    }

    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
