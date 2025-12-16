/* -*- coding: koi8-r -*-
 * Программа для поиска строк в текстовом файле с использованием mmap и таймера
 * Кодировка: KOI8-R
 * Компиляция: cc -o line_search_mmap line_search_mmap.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

/* Структура для хранения смещения и длины строки */
typedef struct {
    off_t offset; /* Смещение начала строки в файле */
    size_t length; /* Длина строки (включая '\n', если есть) */
} LineInfo;

/* Глобальные переменные для имени файла и отображённой памяти */
static const char *global_filename = NULL;
static char *file_map = NULL;
static size_t file_size = 0;

/* Обработчик сигнала SIGALRM */
void alarm_handler(int signum) {
    printf("\nВремя истекло! Вывод всего файла:\n");
    /* Выводим всё содержимое отображённой памяти */
    if (write(STDOUT_FILENO, file_map, file_size) == -1) {
        perror("Ошибка вывода файла в обработчике SIGALRM");
    }
    exit(EXIT_SUCCESS);
}

/* Функция для построения таблицы смещений и длин строк */
LineInfo *build_line_table(int *line_count) {
    int lines_capacity = 100;
    int lines = 0;
    LineInfo *table = malloc(lines_capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("Ошибка выделения памяти для таблицы");
        return NULL;
    }

    table[lines].offset = 0;
    size_t current_length = 0;

    /* Анализируем отображённую память */
    for (size_t i = 0; i < file_size; i++) {
        current_length++;
        if (file_map[i] == '\n') {
            table[lines].length = current_length;
            lines++;
            if (lines >= lines_capacity) {
                lines_capacity *= 2;
                LineInfo *new_table = realloc(table, lines_capacity * sizeof(LineInfo));
                if (new_table == NULL) {
                    perror("Ошибка перевыделения памяти");
                    free(table);
                    return NULL;
                }
                table = new_table;
            }
            table[lines].offset = i + 1;
            current_length = 0;
        }
    }

    /* Если есть остаток (строка без '\n') */
    if (current_length > 0) {
        table[lines].length = current_length;
        lines++;
    }

    *line_count = lines;
    return table;
}

/* Функция для вывода строки по номеру */
void print_line(LineInfo *table, int line_number, int line_count) {
    if (line_number < 1 || line_number > line_count) {
        printf("Неверный номер строки: %d (доступно 1-%d)\n", line_number, line_count);
        return;
    }

    /* Выводим строку из отображённой памяти */
    off_t offset = table[line_number - 1].offset;
    size_t length = table[line_number - 1].length;
    char *buffer = malloc(length + 1);
    if (buffer == NULL) {
        perror("Ошибка выделения памяти");
        return;
    }

    memcpy(buffer, file_map + offset, length);
    buffer[length] = '\0';
    printf("Строка %d: %s", line_number, buffer);
    free(buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    global_filename = argv[1];

    /* Открываем файл */
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    /* Получаем размер файла */
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Ошибка получения размера файла");
        close(fd);
        return EXIT_FAILURE;
    }
    file_size = st.st_size;

    /* Отображаем файл в память */
    if (file_size > 0) {
        file_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (file_map == MAP_FAILED) {
            perror("Ошибка отображения файла в память");
            close(fd);
            return EXIT_FAILURE;
        }
    } else {
        file_map = NULL; /* Пустой файл */
    }

    /* Закрываем дескриптор, так как mmap сохраняет доступ */
    close(fd);

    /* Строим таблицу смещений */
    int line_count;
    LineInfo *table = build_line_table(&line_count);
    if (table == NULL) {
        if (file_size > 0) {
            munmap(file_map, file_size);
        }
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
        if (file_size > 0) {
            munmap(file_map, file_size);
        }
        return EXIT_FAILURE;
    }

    /* Запрос номера строки */
    int line_number;
    while (1) {
        printf("\nВведите номер строки (0 для выхода, 5 секунд на ввод): ");
        alarm(5);

        if (scanf("%d", &line_number) != 1) {
            alarm(0);
            while (getchar() != '\n');
            printf("Ошибка ввода, попробуйте снова\n");
            continue;
        }

        alarm(0);

        if (line_number == 0) {
            break;
        }

        print_line(table, line_number, line_count);
    }

    /* Очистка */
    free(table);
    if (file_size > 0) {
        munmap(file_map, file_size);
    }

    return EXIT_SUCCESS;
}