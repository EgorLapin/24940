#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IOLBF, 0);

    if (argc < 2) {
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    int status;

    switch (pid) {
    case -1:
        perror("fail - fork");
        exit(EXIT_FAILURE);

    case 0:
        printf("Child PID: %ld\n", (long)getpid());
        execlp("cat", "cat", argv[1], NULL);
        perror("fail - execlp");
        _exit(EXIT_FAILURE);

    default:
        printf("Parent PID: %ld\n", (long)getpid());
        waitpid(pid, &status, 0);
        printf("Parent: Child process finished\n");
        exit(EXIT_SUCCESS);
    }
}