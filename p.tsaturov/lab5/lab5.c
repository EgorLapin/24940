#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LINES 100
#define BUFFER 100

typedef struct Line {
    off_t off;
    size_t len;
} Line;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Use: %s <filename>\n", argv[0]);
        return 1;
    }

    int file = open(argv[1], O_RDONLY);
    if (file < 0) {
        perror("OpenFileError");
        return 1;
    }

    Line lines_info[MAX_LINES];
    int line_count = 0;

    char buffer[BUFFER];
    ssize_t bytes;
    off_t cur_off = 0;
    off_t line_start = 0;
    int line_length = 0; 

    while ((bytes = read(file, buffer, BUFFER)) > 0) {
        for (ssize_t i = 0; i < bytes; i++) {
            if (buffer[i] == '\n') {
                if (line_count < MAX_LINES) {
                    lines_info[line_count].off = line_start;
                    lines_info[line_count].len = line_length;
                    line_count++;
                }
                line_start = cur_off + i + 1;
                line_length = 0;
            } else {
                line_length++;
            }
        }
        cur_off += bytes;
    }

    if (cur_off > line_start && line_count < MAX_LINES) {
        lines_info[line_count].off = line_start;
        lines_info[line_count].len = cur_off - line_start;
        line_count++;
    }

    printf("Table:\n");
    for (int i = 0; i < line_count; i++) {
        printf("line %2d: offset=%6ld, length=%3zu\n", i + 1, (long)lines_info[i].off, lines_info[i].len);
    }

    while (1) {
        int line_num;
        if (scanf("%d", &line_num) != 1) {
            printf("InputError.\n");
            while (getchar() != '\n');
            continue;
        }

        if (line_num == 0) break;

        if (line_num < 1 || line_num > line_count) {
            printf("The line is not exist. Max line - %d.\n", line_count);
            continue;
        }

        Line *curr_line = &lines_info[line_num - 1];

        if (lseek(file, curr_line->off, SEEK_SET) == -1) {
            perror("lseekError");
            break;
        }

        char line_buffer[curr_line->len + 1];
        ssize_t read_bytes = read(file, line_buffer, curr_line->len);
        if (read_bytes < 0) {
            perror("ReadError");
            break;
        }

        line_buffer[read_bytes] = '\0';
        printf("%s\n", line_buffer);
    }

    close(file);
    return 0;
}
