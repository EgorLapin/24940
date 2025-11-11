#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Ошибка fork\n");
        return 1;
    } 
    else if (pid == 0) {
        execvp(argv[1], &argv[1]);
        fprintf(stderr, "Ошибка выполнения команды %s\n", argv[1]);
        exit(1);
    } 
    else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        } else {
            printf("Дочерний процесс не вернул код\n");
        }
    }

    return 0;
}