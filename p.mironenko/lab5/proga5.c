/* -*- coding: koi8-r -*-
 * Программа для поиска строк в текстовом файле с использованием таблицы смещений
 * Кодировка: KOI8-R
 * Компиляция: cc -o line_search line_search.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* Структура для хранения смещения и длины строки */
typedef struct {
    off_t offset; /* Смещение начала строки в файле */
    size_t length; /* Длина строки (без '\n') */
} LineInfo;

/* Функция для построения таблицы смещений и длин строк */
LineInfo *build_line_table(const char *filename, int *line_count) {
    int fd;
    char buffer[1024];
    ssize_t bytes_read;
    off_t current_offset = 0;
    int lines_capacity = 100; /* Начальная емкость массива */
    int lines = 0;
    LineInfo *table = malloc(lines_capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("Ошибка выделения памяти для таблицы");
        return NULL;
    }

    /* Открываем файл */
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        free(table);
        return NULL;
    }

    /* Читаем файл поблочно */
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                /* Увеличиваем массив, если нужно */
                if (lines >= lines_capacity) {
                    lines_capacity *= 2;
                    LineInfo *new_table = realloc(table, lines_capacity * sizeof(LineInfo));
                    if (new_table == NULL) {
                        perror("Ошибка перевыделения памяти");
                        close(fd);
                        free(table);
                        return NULL;
                    }
                    table = new_table;
                }

                /* Записываем информацию о строке */
                table[lines].offset = current_offset;
                table[lines].length = i + 1; /* Длина до '\n' включительно */
                current_offset += i + 1;
                lines++;
                /* Перемещаем указатель чтения в буфере */
                if (i + 1 < bytes_read) {
                    memmove(buffer, buffer + i + 1, bytes_read - i - 1);
                    bytes_read -= i + 1;
                    i = -1; /* Начать цикл заново с начала нового буфера */
                } else {
                    bytes_read = 0;
                }
            }
        }
        current_offset += bytes_read;
    }

    if (bytes_read == -1) {
        perror("Ошибка чтения файла");
        close(fd);
        free(table);
        return NULL;
    }

    /* Если последняя строка не заканчивается '\n', добавляем её */
    if (current_offset > table[lines - 1].offset + table[lines - 1].length) {
        if (lines >= lines_capacity) {
            lines_capacity++;
            LineInfo *new_table = realloc(table, lines_capacity * sizeof(LineInfo));
            if (new_table == NULL) {
                perror("Ошибка перевыделения памяти");
                close(fd);
                free(table);
                return NULL;
            }
            table = new_table;
        }
        table[lines].offset = table[lines - 1].offset + table[lines - 1].length;
        table[lines].length = current_offset - table[lines].offset;
        lines++;
    }

    close(fd);
    *line_count = lines;
    return table;
}

/* Функция для вывода строки по номеру */
void print_line(const char *filename, LineInfo *table, int line_number, int line_count) {
    if (line_number < 1 || line_number > line_count) {
        printf("Неверный номер строки: %d (доступно 1-%d)\n", line_number, line_count);
        return;
    }

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        return;
    }

    /* Позиционируемся на начало строки */
    if (lseek(fd, table[line_number - 1].offset, SEEK_SET) == -1) {
        perror("Ошибка lseek");
        close(fd);
        return;
    }

    /* Читаем строку */
    char *buffer = malloc(table[line_number - 1].length + 1);
    if (buffer == NULL) {
        perror("Ошибка выделения памяти");
        close(fd);
        return;
    }

    ssize_t bytes_read = read(fd, buffer, table[line_number - 1].length);
    if (bytes_read == -1) {
        perror("Ошибка чтения строки");
        free(buffer);
        close(fd);
        return;
    }

    buffer[bytes_read] = '\0'; /* Добавляем нулевой терминатор */
    printf("Строка %d: %s", line_number, buffer);
    free(buffer);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int line_count;
    LineInfo *table = build_line_table(argv[1], &line_count);
    if (table == NULL) {
        return EXIT_FAILURE;
    }

    /* Отладочный вывод таблицы */
    printf("Таблица смещений и длин строк:\n");
    for (int i = 0; i < line_count; i++) {
        printf("Строка %d: смещение=%lld, длина=%zu\n", i + 1, table[i].offset, table[i].length);
    }

    /* Запрос номера строки */
    int line_number;
    while (1) {
        printf("\nВведите номер строки (0 для выхода): ");
        if (scanf("%d", &line_number) != 1) {
            while (getchar() != '\n'); /* Очищаем ввод */
            printf("Ошибка ввода, попробуйте снова (доступны только числа)\n");
            continue;
        }

        if (line_number == 0) {
            break;
        }

        print_line(argv[1], table, line_number, line_count);
    }

    free(table);
    return EXIT_SUCCESS;
}