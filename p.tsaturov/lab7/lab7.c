#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>

#define MAX_LINES 100

typedef struct Line {
    off_t off;
    size_t len;
} Line;

int file_fd;
char *file_data;
size_t file_size;

void timeout_handler(int signum) {
    (void)signum;
    printf("\n\nTIMEOUT\n\n");

    write(STDOUT_FILENO, file_data, file_size);

    munmap(file_data, file_size);
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


    struct stat sb;
    if (fstat(file_fd, &sb) == -1) {
        perror("fstat");
        close(file_fd);
        return 1;
    }
    file_size = sb.st_size;


    file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap");
        close(file_fd);
        return 1;
    }

    Line lines_info[MAX_LINES];
    int line_count = 0;
    off_t line_start = 0;
    size_t line_length = 0;


    for (off_t i = 0; i < file_size; i++) {
        if (file_data[i] == '\n') {
            if (line_count < MAX_LINES) {
                lines_info[line_count].off = line_start;
                lines_info[line_count].len = line_length;
                line_count++;
            }
            line_start = i + 1;
            line_length = 0;
        } else {
            line_length++;
        }
    }

    if (line_length > 0 && line_count < MAX_LINES) {
        lines_info[line_count].off = line_start;
        lines_info[line_count].len = line_length;
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

        fwrite(file_data + curr_line->off, 1, curr_line->len, stdout);
        printf("\n");
    }


    munmap(file_data, file_size);
    close(file_fd);
    
    return 0;
}