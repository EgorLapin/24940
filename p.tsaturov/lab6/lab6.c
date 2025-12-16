#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINES 100
#define BUFFER 100

typedef struct Line {
    off_t off;
    size_t len;
} Line;

int file_fd;

void timeout_handler(int signum) {
    (void)signum;
    printf("\n\nTIMEOUT\n\n");

    if (lseek(file_fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        exit(1);
    }

    char buf[4096];
    ssize_t r;
    while ((r = read(file_fd, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, r);
    }

    close(file_fd);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Use: %s <filename>\n", argv[0]);
        return 1;
    }

    file_fd = open(argv[1], O_RDONLY);
    if (file_fd < 0) {
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


    while ((bytes = read(file_fd, buffer, BUFFER)) > 0) {
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


    signal(SIGALRM, timeout_handler);
    int first_input = 1;

    while (1) {
        int line_num;

        fflush(stdout);


        if (first_input) {
        alarm(5);
        }

        if (scanf("%d", &line_num) != 1) {

            if (first_input) {
                alarm(0);
                signal(SIGALRM, SIG_DFL);
                first_input = 0;
            } else {
                alarm(0);
            }

            printf("InputError.\n");
            while (getchar() != '\n');
            continue;
        }

        if (first_input) {
            alarm(0);
            signal(SIGALRM, SIG_DFL);
            first_input = 0;
        } else {
            alarm(0);
        }

        if (line_num == 0)
            break;

        if (line_num < 1 || line_num > line_count) {
            printf("The line is not exist. Max line - %d.\n", line_count);
            continue;
        }

        Line *curr_line = &lines_info[line_num - 1];

        if (lseek(file_fd, curr_line->off, SEEK_SET) == -1) {
            perror("lseekError");
            break;
        }

        char line_buffer[curr_line->len + 1];
        ssize_t read_bytes = read(file_fd, line_buffer, curr_line->len);
        if (read_bytes < 0) {
            perror("ReadError");
            break;
        }

        line_buffer[read_bytes] = '\0';
        printf("%s\n", line_buffer);
    }

    close(file_fd);
    return 0;
}
