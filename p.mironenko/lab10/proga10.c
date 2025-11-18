/* -*- coding: koi8-r -*-
 * Программа: запуск команды с аргументами и вывод кода завершения
 * Задача 10: Код завершения команды
 * Компиляция: cc -o proga10 proga10.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        fprintf(stderr, "Пример: %s ls -l /tmp\n", argv[0]);
        fprintf(stderr, "        %s echo \"Привет, мир!\"\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Создаём дочерний процесс */
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* === ДОЧЕРНИЙ ПРОЦЕСС === */
        /* argv[1] — имя команды, argv[1], argv[2], ..., argv[argc-1], NULL */
        execvp(argv[1], &argv[1]);

        /* Если execvp вернул управление — значит, произошла ошибка */
        perror("Ошибка execvp");
        fprintf(stderr, "Не удалось выполнить команду: %s\n", argv[1]);
        exit(127);  /* Стандартный код ошибки для "команда не найдена" */
    } else {
        /* === РОДИТЕЛЬСКИЙ ПРОЦЕСС === */
        int status;
        pid_t waited_pid;

        printf("Запущена команда: %s (PID дочернего процесса: %d)\n", argv[1], pid);

        /* Ждём завершения именно нашего дочернего процесса */
        waited_pid = waitpid(pid, &status, 0);
        if (waited_pid == -1) {
            perror("Ошибка waitpid");
            return EXIT_FAILURE;
        }

        /* Анализируем код завершения */
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Команда завершилась нормально с кодом: %d\n", exit_code);
        } else if (WIFSIGNALED(status)) {
            int term_sig = WTERMSIG(status);
            printf("Команда завершена по сигналу: %d (%s)%s\n",
                   term_sig,
                   strsignal(term_sig),
                   WCOREDUMP(status) ? " (создан core-файл)" : "");
        } else {
            printf("Команда завершена необычным образом (статус: 0x%x)\n", status);
        }
    }

    return EXIT_SUCCESS;
}