/* -*- coding: koi8-r -*-
 * Программа для создания двух процессов с ожиданием завершения дочернего
 * Кодировка: KOI8-R
 * Компиляция: cc -o proga9_2 proga9_2.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* Дочерний процесс: выполняет cat */
        execlp("cat", "cat", argv[1], NULL);
        perror("Ошибка execlp");
        exit(EXIT_FAILURE);
    } else {
        /* Родительский процесс */
        printf("Родитель: PID=%d, создал дочерний процесс PID=%d\n", getpid(), pid);

        /* Ожидание завершения дочернего процесса */
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("Ошибка waitpid");
            return EXIT_FAILURE;
        }

        /* Проверка статуса завершения дочернего процесса */
        if (WIFEXITED(status)) {
            printf("Родитель: дочерний процесс завершился с кодом %d\n", WEXITSTATUS(status));
        } else {
            printf("Родитель: дочерний процесс завершился ненормально\n");
        }

        /* Последняя строка после завершения дочернего процесса */
        printf("Родитель: дочерний процесс завершён, это последняя строка\n");
    }

    return EXIT_SUCCESS;
}