#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;
    
    if (argc < 2) {
        fprintf(stderr, "Using: %s <command> [arguments...]\n", argv[0]);
        exit(1);
    }
    
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        execvp(argv[1], &argv[1]);
        
        perror("execvp");
        exit(1);
    }
    else {
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(1);
        }

        printf("\n");
        
        if (WIFEXITED(status)) {
            printf("Code of exit: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("Process terminated by signal: %d\n", WTERMSIG(status));
        }
        else {
            printf("Process terminated with unknown status: %d\n", status);
        }
    }
    
    return 0;
}
