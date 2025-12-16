/* -*- coding: koi8-r -*-
 * pipe_upper.c — Связь через программный канал (лаб. №25)
 * Родитель вводит текст → канал → потомок переводит в ВЕРХНИЙ РЕГИСТР → вывод
 * Компиляция: cc -o pipe pipe.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>        /* для toupper() */
#include <sys/wait.h>     /* для wait() */

#define BUFFER_SIZE 1024

int main() { 
    int pipefd[2];                /* pipefd[0] — чтение, pipefd[1] — запись */
    pid_t pid;
    char buffer[BUFFER_SIZE];

    /* Создаём программный канал */
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* Создаём дочерний процесс */
    pid = fork();

    if (pid == -1) { // Ошибка при создании процесса
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { //потомок
        close(pipefd[1]);         /* Закрываем неиспользуемый конец для записи */

        printf("Потомок запущен. Ожидаю текст от родителя...\n\n");

        /* Читаем из канала по частям */
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';  /* Добавляем нулевой символ */

            /* Преобразуем все символы в верхний регистр */
            for (int i = 0; buffer[i]; i++) {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }

            /* Выводим результат */
            printf("ПОЛУЧЕНО И ПРЕОБРАЗОВАНО: %s", buffer);
            fflush(stdout);
        }

        if (bytes_read == -1) {
            perror("read в потомке");
        }

        printf("\nКанал закрыт. Потомок завершает работу.\n");
        close(pipefd[0]);
        exit(EXIT_SUCCESS);

    } else { //родитель
        close(pipefd[0]);  /* Закрываем неиспользуемый конец для чтения */

        printf("Связь через программный канал установлена!\n");
        printf("Введите текст (пустая строка или Ctrl+D — выход):\n");
        printf("Работает только с буквами английского алфавита!\n\n");

        /* Читаем строки с клавиатуры и отправляем в канал */
        while (1) {
            printf("> ");
            fflush(stdout);

            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                /* Ctrl+D — конец ввода */
                printf("\nВвод завершён (Ctrl+D). Закрываю канал...\n");
                break;
            }

            /* Убираем символ новой строки */
            buffer[strcspn(buffer, "\n")] = '\0';

            /* Если ввели пустую строку — выходим */
            if (buffer[0] == '\0') {
                printf("Пустая строка — завершаю ввод.\n");
                break;
            }

            /* Отправляем в канал */
            if (write(pipefd[1], buffer, strlen(buffer)) == -1) {
                perror("write в родителе");
                break;
            }

            /* Добавляем перевод строки для красоты */
            if (write(pipefd[1], "\n", 1) == -1) {
                perror("write \\n");
                break;
            }
        }

        /* Закрываем конец записи — потомок получит EOF */
        close(pipefd[1]);

        /* Ждём завершения потомка */
        wait(NULL);

        printf("Родитель завершил работу.\n");
    }

    return 0;
}