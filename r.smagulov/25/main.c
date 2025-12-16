#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    char msg[] = "dshjdaJJsadjdjas dhsadjakJDKJkS iosidoIDOIODIODIdjs KJJJ jjjj";

    if (pid == 0) {
        close(fd[0]);
        write(fd[1], msg, sizeof(msg));
        close(fd[1]);
    } else {
        close(fd[1]);
        char received[256];
        ssize_t n = read(fd[0], received, sizeof(received));
        close(fd[0]);

        if (n > 0) {
            for (int i = 0; i < n; i++) {
                received[i] = toupper(received[i]);
            }
            printf("%s\n", received);
        }
    }
    return 0;
}