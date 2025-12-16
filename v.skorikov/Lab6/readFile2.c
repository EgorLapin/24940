#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define MAX_LINES 1000
#define BUFFER_SIZE 1024
#define TIMEOUT_SECONDS 5

typedef struct {
    off_t offset;  // смещение начала строки в файле
    size_t length; // длина строки (без символа новой строки)
} LineInfo;

// Глобальная переменная для файлового дескриптора
static int global_fd = -1;

// Обработчик сигнала SIGALRM
void alarm_handler(int sig) {
    printf("\nВремя на ввод истекло (%d секунд)\n", TIMEOUT_SECONDS);
    print_entire_file(global_fd);
    close(global_fd);
    exit(0);
}

// Функция для вывода всего содержимого файла
void print_entire_file(int fd) {
    printf("\nTime's out: Вывод всего содержимого файла:\n");
    printf("============================================\n");
    
    // Перемещаемся в начало файла
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Ошибка позиционирования в файле");
        return;
    }
    
    // Читаем и выводим весь файл
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    
    printf("\n============================================\n");
}

// Функция для ввода с таймаутом (только для первого запроса)
int input_with_timeout(int fd, const char* prompt, int* value) {
    printf("%s", prompt);
    fflush(stdout); 
    
    // Устанавливаем обработчик сигнала SIGALRM
    signal(SIGALRM, alarm_handler);
    
    // Устанавливаем таймер на 5 секунд
    alarm(TIMEOUT_SECONDS);
    
    // Читаем ввод
    if (scanf("%d", value) != 1) {
        // Очищаем некорректный ввод
        alarm(0); // Отключаем таймер
        while (getchar() != '\n');
        return -1;
    }
    
    // Отключаем таймер после успешного ввода
    alarm(0);
    return 1;
}

// Функция для ввода без таймаута (для последующих запросов)
int input_without_timeout(const char* prompt, int* value) {
    printf("%s", prompt);
    fflush(stdout); 
    
    // Читаем ввод
    if (scanf("%d", value) != 1) {
        // Очищаем некорректный ввод
        while (getchar() != '\n');
        return -1;
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <текстовый_файл>\n", argv[0]);
        return 1;
    }

    global_fd = open(argv[1], O_RDONLY);
    if (global_fd == -1) {
        perror("Ошибка при открытии файла");
        return 1;
    }

    LineInfo lines[MAX_LINES];
    int line_count = 0;
    off_t current_offset = 0;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("Построение таблицы строк\n");

    while ((bytes_read = read(global_fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                // Записываем информацию о строке
                if (line_count < MAX_LINES) {
                    lines[line_count].offset = current_offset;
                    lines[line_count].length = lseek(global_fd, 0L, SEEK_CUR) - current_offset - (bytes_read - i);
                    line_count++;
                }
                current_offset = lseek(global_fd, 0L, SEEK_CUR) - (bytes_read - i - 1);
            }
        }
    }
    
    // Проверяем, есть ли последняя строка без символа новой строки
    if (lseek(global_fd, 0L, SEEK_END) > current_offset && line_count < MAX_LINES) {
        lines[line_count].offset = current_offset;
        lines[line_count].length = lseek(global_fd, 0L, SEEK_END) - current_offset;
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

    printf("У вас есть %d секунд чтобы ввести номер строки...\n", TIMEOUT_SECONDS);

    // Основной цикл запросов
    int line_number;
    char line_buffer[BUFFER_SIZE];
    int input_result;
    int first_input = 1; // Флаг для первого запроса

    while (1) {
        // Запрос ввода
        if (first_input) {
            input_result = input_with_timeout(global_fd, "Введите номер строки (0 для выхода): ", &line_number);
            first_input = 0; // Отключаем таймер для последующих запросов
        } else {
            input_result = input_without_timeout("Введите номер строки (0 для выхода): ", &line_number);
        }
        
        if (input_result == -1) {
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
        
        // Перемещаемся к началу строки
        if (lseek(global_fd, line->offset, SEEK_SET) == -1) {
            perror("Ошибка позиционирования в файле");
            continue;
        }

        // Читаем строку
        ssize_t read_bytes = read(global_fd, line_buffer, line->length);
        if (read_bytes == -1) {
            perror("Ошибка чтения строки");
            continue;
        }

        // Добавляем нулевой терминатор
        line_buffer[read_bytes] = '\0';

        // Выводим строку
        printf("Строка %d: %s\n", line_number, line_buffer);
    }

    close(global_fd);
    printf("Программа завершена.\n");
    return 0;
}