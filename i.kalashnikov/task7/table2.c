#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

char *file_map;
size_t file_size;
int fd;

void print_file() {
    write(STDOUT_FILENO, file_map, file_size);
    munmap(file_map, file_size);
    close(fd);
    exit(0);
}

LineInfo *build_table(char *data, size_t size, int *count) {
    int cap = 10;
    LineInfo *table = malloc(cap * sizeof(LineInfo));
    *count = 0;

    long start = 0;
    for (long pos = 0; pos < (long)size; pos++) {
        if (data[pos] == '\n') {
            if (*count >= cap) {
                cap *= 2;
                table = realloc(table, cap * sizeof(LineInfo));
            }
            table[*count].offset = start;
            table[*count].length = pos - start;
            (*count)++;
            start = pos + 1;
        }
    }

    if (start < (long)size) {
        table[*count].offset = start;
        table[*count].length = size - start;
        (*count)++;
    }

    return table;
}

int main(void) {
    fd = open("../file.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    fstat(fd, &st);
    file_size = st.st_size;

    file_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    signal(SIGALRM, print_file);

    int lines;
    LineInfo *table = build_table(file_map, file_size, &lines);

    printf("№\tОтступ\tДлина\n");
    for (int i = 0; i < lines; i++)
        printf("%d\t%ld\t%d\n", i + 1, table[i].offset, table[i].length);

    char er[100];

    int n;
    alarm(5);
    printf("\nВведите номер строки (0 для выхода, у вас 5 секунд): ");
    while (1) {

        if (scanf("%d", &n) != 1) {
            printf("Ошибка ввода\n");
            fgets(er, 100, stdin);
            alarm(0);
            printf("\nВведите номер строки (0 для выхода): ");
            continue;
        }
        alarm(0);
        if (n == 0) {
            break;
        }
        if (n < 1 || n > lines) {
            printf("Неверный номер (1–%d)\n", lines);
            printf("\nВведите номер строки (0 для выхода): ");
            continue;
        }

        printf("Строка %d: %.*s\n", n, table[n - 1].length, file_map + table[n - 1].offset);
        printf("\nВведите номер строки (0 для выхода): ");
    }

    free(table);
    munmap(file_map, file_size);
    close(fd);
    return 0;
}