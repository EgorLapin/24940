#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    if (argc < 2) {
        fprintf(stderr, "not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    int wait_status;
    pid_t pid = fork();

    switch (pid)
    {
    case -1:
        perror("fork failed");
        exit(EXIT_FAILURE);

    case 0:
        printf("PID=%ld (child)\n", (long)getpid());
        fflush(stdout);
        execlp("cat", "cat", argv[1], (char *)NULL);
        perror("execlp failed");
        _exit(EXIT_FAILURE);

    default:
        printf("PID=%ld (parent)\n", (long)getpid());
        fflush(stdout);

        printf("success\n");

        if (waitpid(pid, &wait_status, 0) == -1) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }

        printf("\nfork exited\n");
        exit(EXIT_SUCCESS);
    }
}
