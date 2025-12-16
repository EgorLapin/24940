#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Ошибка fork\n");
        return 1;
    } 
    else if (pid == 0) {
        execlp("cat", "cat", "gggg.txt", NULL);
        fprintf(stderr, "Ошибка exec\n");
        return 1;
    } 
    else {
        printf("PID родительского процесса: %d\n\n", getpid());

        int status;
        waitpid(pid, &status, 0);

        printf("\nДочерний процесс (PID: %d) завершился.\n", pid);
    }

    return 0;
}