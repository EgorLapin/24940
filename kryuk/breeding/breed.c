#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;
    
    if (argc < 2) {
        fprintf(stderr, "Using: %s <file_for_cat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    printf("Parent process (PID: %d) started\n", getpid());
    printf("Creating a subprocess to execute cat...\n\n");
    
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        printf("Child process (PID: %d) started\n\n", getpid());
        
        execlp("cat", "cat", argv[1], (char *)NULL);
        
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        printf("Parent process is waiting for the child process to finish (PID: %d)\n\n", pid);
        
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        
        printf("\n=== LAST PARENT WORD: Child process finished ===\n");
    }
    
    return EXIT_SUCCESS;
}
