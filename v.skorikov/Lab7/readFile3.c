#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_LINES 1000
#define BUFFER_SIZE 1024
#define TIMEOUT_SECONDS 5

typedef struct {
    off_t offset;  // смещение начала строки в файле
    size_t length; // длина строки (без символа новой строки)
} LineInfo;

// Функция для вывода всего содержимого файла
void print_entire_file(const char* mapped_file, off_t file_size) {
    printf("\nTime's out: Вывод всего содержимого файла:\n");
    printf("============================================\n");
    write(STDOUT_FILENO, mapped_file, file_size);
    printf("\n============================================\n");
}

// Функция для проверки ввода с таймаутом
int input_with_timeout(const char* prompt, int* value) {
    fd_set readfds;
    struct timeval timeout;
    int ret;

    printf("%s", prompt);
    fflush(stdout);  // Важно: сбрасываем буфер вывода

    // Настраиваем набор файловых дескрипторов
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    // Устанавливаем таймаут 5 секунд
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    // Ждем ввода с таймаутом
    ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if (ret == -1) {
        perror("Ошибка select");
        return -1;
    } else if (ret == 0) {
        // Таймаут - время вышло
        printf("\nВремя на ввод истекло (%d секунд)\n", TIMEOUT_SECONDS);
        return 0;
    } else {
        // Есть ввод - читаем число
        if (scanf("%d", value) != 1) {
            // Очищаем некорректный ввод
            while (getchar() != '\n');
            return -1;
        }
        return 1;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <текстовый_файл>\n", argv[0]);
        return 1;
    }

    // Открываем файл для чтения
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка при открытии файла");
        return 1;
    }

    // Получаем размер файла
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Ошибка получения размера файла");
        close(fd);
        return 1;
    }
    off_t file_size = st.st_size;

    // Отображаем файл в память
    char* mapped_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Ошибка отображения файла в память");
        close(fd);
        return 1;
    }

    LineInfo lines[MAX_LINES];
    int line_count = 0;
    off_t current_offset = 0;
    size_t chars_in_line = 0;

    printf("Построение таблицы строк...\n");

    // Строим таблицу строк, анализируя отображенный буфер
    for (off_t i = 0; i < file_size; i++) {
        chars_in_line++;
        if (mapped_file[i] == '\n' || i == file_size - 1) {
            if (line_count < MAX_LINES) {
                lines[line_count].offset = current_offset;
                lines[line_count].length = (i == file_size - 1 && mapped_file[i] != '\n') ? chars_in_line : chars_in_line - 1;
                line_count++;
            }
            current_offset += chars_in_line;
            chars_in_line = 0;
        }
    }

    // Выводим таблицу для отладки
    printf("\n=== ТАБЛИЦА СТРОК (для отладки) ===\n");
    printf("Номер | Смещение | Длина\n");
    printf("------+----------+-------\n");
    for (int i = 0; i < line_count; i++) {
        printf("%5d | %8ld | %6zu\n", i + 1, lines[i].offset, lines[i].length);
    }
    printf("Всего строк: %d\n\n", line_count);

    printf("У вас есть %d секунд чтобы ввести номер строки...\n", TIMEOUT_SECONDS);

    // Основной цикл запросов
    int line_number;
    char line_buffer[BUFFER_SIZE];
    int input_result;

    while (1) {
        // Запрос ввода с таймаутом
        input_result = input_with_timeout("Введите номер строки (0 для выхода): ", &line_number);

        if (input_result == 0) {
            // Таймаут - выводим весь файл и завершаем
            print_entire_file(mapped_file, file_size);
            break;
        } else if (input_result == -1) {
            // Ошибка ввода
            printf("Ошибка ввода. Пожалуйста, введите число.\n");
            continue;
        }

        // Корректный ввод - обрабатываем
        if (line_number == 0) {
            printf("Завершение работы программы.\n");
            break;
        }

        if (line_number < 1 || line_number > line_count) {
            printf("Ошибка: строка %d не существует. Доступные строки: 1-%d\n",
                   line_number, line_count);
            continue;
        }

        // Получаем информацию о запрошенной строке
        LineInfo *line = &lines[line_number - 1];

        // Копируем строку из отображенного буфера
        memcpy(line_buffer, mapped_file + line->offset, line->length);
        line_buffer[line->length] = '\0';

        // Выводим строку
        printf("Строка %d: %s\n", line_number, line_buffer);

        // Сбрасываем таймаут для следующей итерации
        printf("У вас %d секунд на след.запрос\n", TIMEOUT_SECONDS);
    }

    // Освобождаем отображение и закрываем файл
    munmap(mapped_file, file_size);
    close(fd);
    printf("Программа завершена.\n");
    return 0;
}