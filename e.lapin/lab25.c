#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

int main() {
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();

    if (pid == 0) {
        close(pipefd[1]);

        char buf[128];
        ssize_t n;

        while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
            for (ssize_t i = 0; i < n; i++) {
                buf[i] = toupper((unsigned char)buf[i]);
            }
            write(STDOUT_FILENO, buf, n);
        }

        close(pipefd[0]);
        exit(0);

    } else {
        close(pipefd[0]);

        const char *messages[] = {
            "Testing test\n",
            "Hello world\n",
            "Bottom text",
            NULL
        };

        for (int i = 0; messages[i] != NULL; i++) {
            write(pipefd[1], messages[i], strlen(messages[i]));
            usleep(1000000);
        }

        close(pipefd[1]);
    }

    return 0;
}