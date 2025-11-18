/* -*- coding: koi8-r -*-
 * Программа для создания двух процессов: дочерний выполняет cat, родитель печатает текст
 * Кодировка: KOI8-R
 * Компиляция: cc -o proga proga9.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork(); /* Создание дочернего процесса */
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
        /* Родительский процесс: печатает текст */
        printf("Родитель: PID=%d, создал дочерний процесс PID=%d\n", getpid(), pid);
        printf("Родитель: вывод текста сразу после fork\n");
    }

    return EXIT_SUCCESS;
}