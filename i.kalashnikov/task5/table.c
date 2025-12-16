#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

LineInfo *build_table(int fd, int *count) {
    int cap = 10;
    LineInfo *table = malloc(cap * sizeof(LineInfo));
    char c;
    long pos = 0, start = 0;
    *count = 0;

    lseek(fd, 0, SEEK_SET);
    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            if (*count >= cap) {
                cap *= 2;
                table = realloc(table, cap * sizeof(LineInfo));
            }
            table[*count].offset = start;
            table[*count].length = pos - start;
            (*count)++;
            start = pos + 1;
        }
        pos++;
    }

    if (pos > start) {
        table[*count].offset = start;
        table[*count].length = pos - start;
        (*count)++;
    }
    return table;
}

int main(void) {
    int fd = open("../file.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int lines;
    LineInfo *table = build_table(fd, &lines);

    printf("№\tОтступ\tДлина\n");
    for (int i = 0; i < lines; i++)
        printf("%d\t%ld\t%d\n", i + 1, table[i].offset, table[i].length);

    int n;
    char buf[1024];
    char er[100];

    while (1) {
        printf("\nВведите номер строки (0 для выхода): ");
        if (scanf("%d", &n) != 1) {
            printf("Ошибка ввода\n");
            fgets(er, 100, stdin);
            continue;
        }
        if (n == 0)
            break;
        if (n < 1 || n > lines) {
            printf("Неверный номер (1–%d)\n", lines);
            continue;
        }

        lseek(fd, table[n - 1].offset, SEEK_SET);
        int len = read(fd, buf, table[n - 1].length);
        buf[len] = '\0';
        printf("Строка %d: %s\n", n, buf);
    }

    free(table);
    close(fd);
    return 0;
}