#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

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

const char *filedata_global = NULL;
size_t filesize_global = 0;
LineTable table_global = {0};

void print_full_file() {
    fwrite(filedata_global, 1, filesize_global, stdout);
}

void alarm_handler() {
    printf("\nTimeout reached! Printing entire file and exiting.\n\n");
    print_full_file();
    free_line_table(&table_global);
    exit(0);
}

int build_line_table(const char *data, size_t filesize, LineTable *table) {
    off_t line_start = 0;
    for (off_t i = 0; i < (off_t)filesize; i++) {
        if (data[i] == '\n') {
            size_t line_len = i + 1 - line_start;
            if (append_line(table, line_start, line_len) != 0) {
                return -1;
            }
            line_start = i + 1;
        }
    }
    if (line_start < (off_t)filesize) {
        size_t line_len = filesize - line_start;
        if (append_line(table, line_start, line_len) != 0) {
            return -1;
        }
    }
    return 0;
}

void print_line_table(LineTable *table) {
    printf("Line offsets and lengths:\n");
    for (size_t i = 0; i < table->count; i++) {
        printf("Line %zu: offset = %ld, length = %zu\n",
               i + 1, (long)table->lines[i].offset, table->lines[i].length);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    filesize_global = st.st_size;
    if (filesize_global == 0) {
        printf("Empty file\n");
        close(fd);
        return 0;
    }

    filedata_global = mmap(NULL, filesize_global, PROT_READ, MAP_PRIVATE, fd, 0);
    if (filedata_global == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    if (build_line_table(filedata_global, filesize_global, &table_global) != 0) {
        fprintf(stderr, "Failed to build line table\n");
        munmap((void*)filedata_global, filesize_global);
        close(fd);
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
        munmap((void*)filedata_global, filesize_global);
        close(fd);
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
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        off_t offset = table_global.lines[num - 1].offset;
        size_t length = table_global.lines[num - 1].length;

        printf("Line %d: ", num);
        fwrite(filedata_global + offset, 1, length, stdout);
        if (length == 0 || filedata_global[offset + length - 1] != '\n') {
            printf("\n");
        }
    }

    munmap((void*)filedata_global, filesize_global);
    close(fd);
    free_line_table(&table_global);

    return 0;
}
