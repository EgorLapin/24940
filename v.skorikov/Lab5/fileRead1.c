#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX_LINES 1000
#define BUFFER_SIZE 1024

typedef struct {
    off_t offset;  // смещение начала строки в файле
    size_t length; // длина строки (без символа новой строки)
} LineInfo;

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

    LineInfo lines[MAX_LINES];
    int line_count = 0;
    off_t current_offset = 0;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("Построение таблицы строк...\n");

    // Читаем файл и строим таблицу строк
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                // Записываем информацию о строке
                if (line_count < MAX_LINES) {
                    lines[line_count].offset = current_offset;
                    lines[line_count].length = lseek(fd, 0L, SEEK_CUR) - current_offset - (bytes_read - i);
                    line_count++;
                }
                current_offset = lseek(fd, 0L, SEEK_CUR) - (bytes_read - i - 1);
            }   
        }
    }

    // Проверяем, есть ли последняя строка без символа новой строки
    if (lseek(fd, 0L, SEEK_END) > current_offset && line_count < MAX_LINES) {
        lines[line_count].offset = current_offset;
        lines[line_count].length = lseek(fd, 0L, SEEK_END) - current_offset;
        line_count++;
    }

    // Отладочная печать таблицы
    printf("\n=== ТАБЛИЦА СТРОК (для отладки) ===\n");
    printf("Номер | Смещение | Длина\n");
    printf("------+----------+-------\n");
    for (int i = 0; i < line_count; i++) {
        printf("%5d | %8ld | %6zu\n", i + 1, lines[i].offset, lines[i].length);
    }
    printf("Всего строк: %d\n\n", line_count);

    // Основной цикл запросов
    int line_number;
    char line_buffer[BUFFER_SIZE];

    while (1) {
        printf("Введите номер строки (0 для выхода): ");
        if (scanf("%d", &line_number) != 1) {
            printf("Ошибка ввода. Пожалуйста, введите число.\n");
            while (getchar() != '\n'); // Очистка буфера ввода
            continue;
        }

        if (line_number == 0) {
            break;
        }

        if (line_number < 1 || line_number > line_count) {
            printf("Ошибка: строка %d не существует. Доступные строки: 1-%d\n", 
                   line_number, line_count);
            continue;
        }

        // Получаем информацию о запрошенной строке
        LineInfo *line = &lines[line_number - 1];
        
        // Перемещаемся к началу строки
        if (lseek(fd, line->offset, SEEK_SET) == -1) {
            perror("Ошибка позиционирования в файле");
            continue;
        }

        // Читаем строку
        ssize_t read_bytes = read(fd, line_buffer, line->length);
        if (read_bytes == -1) {
            perror("Ошибка чтения строки");
            continue;
        }

        // Добавляем нулевой терминатор
        line_buffer[read_bytes] = '\0';

        // Выводим строку
        printf("Строка %d: %s\n", line_number, line_buffer);
    }

    close(fd);
    printf("Программа завершена.\n");
    return 0;
}