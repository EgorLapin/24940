#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

typedef struct {
    off_t offset;
    size_t length;
} LineInfo;

typedef struct {
    LineInfo *lines;
    size_t count;
    size_t capacity;
} LineTable;

int append_line(LineTable *table, off_t offset, size_t length) {
    if (table->count == table->capacity) {
        size_t new_capacity = table->capacity == 0 ? 64 : table->capacity * 2;
        LineInfo *new_lines = realloc(table->lines, new_capacity * sizeof(LineInfo));
        if (!new_lines) return -1;
        table->lines = new_lines;
        table->capacity = new_capacity;
    }
    table->lines[table->count].offset = offset;
    table->lines[table->count].length = length;
    table->count++;
    return 0;
}

void free_line_table(LineTable *table) {
    free(table->lines);
    table->lines = NULL;
    table->capacity = 0;
    table->count = 0;
}

int build_line_table(int fd, LineTable *table) {
    size_t bufsize = 4096;
    char *buffer = malloc(bufsize);
    if (!buffer) return -1;
    off_t current_offset = 0;
    ssize_t bytes_read;
    off_t line_start = 0;

    while ((bytes_read = read(fd, buffer, bufsize)) > 0) {
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n') {
                size_t line_len = (current_offset + i + 1) - line_start;
                if (append_line(table, line_start, line_len) != 0) {
                    free(buffer);
                    return -1;
                }
                line_start = current_offset + i + 1;
            }
        }
        current_offset += bytes_read;
    }

    if (bytes_read < 0) {
        free(buffer);
        return -1;
    }

    if (line_start < current_offset) {
        size_t line_len = current_offset - line_start;
        if (append_line(table, line_start, line_len) != 0) {
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 0;
}

void print_line_table(LineTable *table) {
    printf("Line offsets and lengths:\n");
    for (size_t i = 0; i < table->count; i++) {
        printf("Line %zu: offset = %ld, length = %zu\n",
               i + 1, (long)table->lines[i].offset, table->lines[i].length);
    }
}

void print_full_file(int fd) {
    const size_t bufsize = 4096;
    char *buffer = malloc(bufsize);
    if (!buffer) {
        perror("malloc");
        return;
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        free(buffer);
        return;
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, bufsize)) > 0) {
        fwrite(buffer, 1, bytes_read, stdout);
    }

    if (bytes_read < 0) {
        perror("read");
    }

    free(buffer);
}

int fd_global = -1;
LineTable table_global = {0};

void alarm_handler() {
    printf("\nTimeout reached! Printing entire file and exiting.\n");
    if (fd_global >= 0) {
        print_full_file(fd_global);
    }
    free_line_table(&table_global);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    fd_global = open(argv[1], O_RDONLY);
    if (fd_global < 0) {
        perror("open");
        return 1;
    }

    if (build_line_table(fd_global, &table_global) != 0) {
        perror("build_line_table");
        close(fd_global);
        free_line_table(&table_global);
        return 1;
    }

    print_line_table(&table_global);

    struct sigaction sa = {0};
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        close(fd_global);
        free_line_table(&table_global);
        return 1;
    }

    int first_input = 1;
    while (1) {

        printf("\nEnter line number (0 to quit): ");

        if (first_input) {
            alarm(5);  
        } else {
            alarm(0);  
        }

        int num;
        int ret = scanf("%d", &num);

        alarm(0);  

        if (first_input) {
            first_input = 0;  
        }

        if (ret != 1) {
            fprintf(stderr, "Invalid input, please enter a number\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {} 
            continue;  
        }


        if (num == 0)
            break;

        if ((size_t)num < 1 || (size_t)num > table_global.count) {
            printf("Line number out of range (1-%zu)\n", table_global.count);
            int c;
            while ((c = getchar()) != '\n' && c != EOF){}
            continue;
        }

        off_t offset = table_global.lines[num - 1].offset;
        size_t length = table_global.lines[num - 1].length;

        if (lseek(fd_global, offset, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            continue;
        }

        char *line_buf = malloc(length + 1);
        if (!line_buf) {
            perror("malloc");
            break;
        }

        ssize_t r = read(fd_global, line_buf, length);
        if (r < 0) {
            perror("read");
            free(line_buf);
            break;
        }

        line_buf[r] = '\0';
        printf("Line %d: %s", num, line_buf);
        if (line_buf[r - 1] != '\n') {
            printf("\n");
        }

        free(line_buf);
    }

    close(fd_global);
    free_line_table(&table_global);
    return 0;
}
