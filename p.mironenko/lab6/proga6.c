/* -*- coding: koi8-r -*-
 * Программа для поиска строк в текстовом файле с таймером (alarm)
 * Кодировка: KOI8-R
 * Компиляция: cc -o line_search_alarm line_search_alarm.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

/* Структура для хранения смещения и длины строки */
typedef struct {
    off_t offset; /* Смещение начала строки в файле */
    size_t length; /* Длина строки (включая '\n', если есть) */
} LineInfo;

/* Глобальная переменная для имени файла */
static const char *global_filename = NULL;

/* Обработчик сигнала SIGALRM */
void alarm_handler(int signum) {
    int fd = open(global_filename, O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла в обработчике SIGALRM");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytes_read;
    printf("\nВремя истекло! Вывод всего файла:\n");
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    if (bytes_read == -1) {
        perror("Ошибка чтения файла в обработчике SIGALRM");
    }
    close(fd);
    exit(EXIT_SUCCESS);
}

/* Функция для построения таблицы смещений и длин строк */
LineInfo *build_line_table(const char *filename, int *line_count) {
    int fd; /* Дескриптор файла */
    char buffer[1]; /* Читаем по одному байту для точного подсчёта */
    ssize_t bytes_read; /* Количество прочитанных байт */
    off_t current_offset = 0;
    int lines_capacity = 100;
    int lines = 0;
    LineInfo *table = malloc(lines_capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("Ошибка выделения памяти для таблицы");
        return NULL;
    }

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        free(table);
        return NULL;
    }

    table[lines].offset = 0; /* Начало первой строки */
    size_t current_length = 0;

    /* Читаем файл по одному байту */
    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        current_length++;
        if (buffer[0] == '\n') {
            table[lines].length = current_length; /* Включаем '\n' */
            lines++;
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
            table[lines].offset = current_offset + 1; /* Следующая строка начинается после '\n' */
            current_length = 0;
        }
        current_offset++;
    }

    if (bytes_read == -1) {
        perror("Ошибка чтения файла");
        close(fd);
        free(table);
        return NULL;
    }

    /* Если есть остаток (строка без '\n') */
    if (current_length > 0) {
        table[lines].length = current_length;
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

    if (lseek(fd, table[line_number - 1].offset, SEEK_SET) == -1) {
        perror("Ошибка lseek");
        close(fd);
        return;
    }

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

    buffer[bytes_read] = '\0';
    printf("Строка %d: %s", line_number, buffer);
    free(buffer);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    global_filename = argv[1];
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

    /* Настройка обработчика SIGALRM */
    if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
        perror("Ошибка установки обработчика SIGALRM");
        free(table);
        return EXIT_FAILURE;
    }

    /* Запрос номера строки */
    int line_number;
    while (1) {
        printf("\nВведите номер строки (0 для выхода, 5 секунд на ввод): ");
        alarm(5); /* Устанавливаем таймер на 5 секунд */

        if (scanf("%d", &line_number) != 1) {
            alarm(0);
            while (getchar() != '\n');
            printf("Ошибка ввода, попробуйте снова (доступны только числа)\n");
            continue;
        }

        alarm(0);

        if (line_number == 0) {
            break;
        }

        print_line(argv[1], table, line_number, line_count);
    }

    free(table);
    return EXIT_SUCCESS;
}