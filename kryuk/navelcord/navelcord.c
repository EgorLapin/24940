#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main(void) {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    ssize_t nbytes;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        close(pipefd[1]);
        
        while ((nbytes = read(pipefd[0], buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[nbytes] = '\0';
            
            for (ssize_t i = 0; i < nbytes; i++) {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }
            
            printf("%s", buffer);
            fflush(stdout);
        }
        
        if (nbytes == -1) {
            perror("read");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }
        
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    } else {
        close(pipefd[0]);
        
        const char *text = "Hello World!\nThis is a Test String.\n"
                          "Solaris OS Programming Example.\n"
                          "Mixed Case: AbCdEfGhIjKlMnOpQrStUvWxYz\n";
        
        ssize_t text_len = strlen(text);
        if (write(pipefd[1], text, text_len) != text_len) {
            perror("write");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }
        
        close(pipefd[1]);
        
        wait(NULL);
        
        exit(EXIT_SUCCESS);
    }
}

