#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    
    raw.c_cc[VMIN] = 1; 
    raw.c_cc[VTIME] = 0;  
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void refresh_line(char *buffer, int len) {
    printf("\r\033[K");
    
    int line_start = 0;
    int current_line = 0;
    
    while (line_start < len) {
        int line_end = line_start + 40;
        if (line_end < len && !isspace(buffer[line_end - 1]) && !isspace(buffer[line_end])) {
            int word_start = line_end;
            while (word_start > line_start && !isspace(buffer[word_start - 1])) {
                word_start--;
            }
            if (word_start > line_start) {
                line_end = word_start;
            }
        }

        for (int i = line_start; i < line_end && i < len; i++) {
            putchar(buffer[i]);
        }
        
        line_start = line_end;
        if (line_start < len) {
            printf("\n");
            current_line++;
        }
    }
    if (len > 0) {
        int total_lines = 0;
        int pos = 0;
        
        line_start = 0;
        while (line_start < len) {
            int line_end = line_start + 40;
            if (line_end < len && !isspace(buffer[line_end - 1]) && !isspace(buffer[line_end])) {
                int word_start = line_end;
                while (word_start > line_start && !isspace(buffer[word_start - 1])) {
                    word_start--;
                }
                if (word_start > line_start) {
                    line_end = word_start;
                }
            }
            
            if (line_start <= len && len <= line_end) {
                pos = len - line_start;
                break;
            }
            line_start = line_end;
            total_lines++;
        }
        
        if (total_lines > 0) {
            printf("\033[%dA", total_lines);
        }
        printf("\r");
        if (pos > 0) {
            printf("\033[%dC", pos);
        }
    }
    fflush(stdout);
}

int main() {
    enableRawMode();
    char buffer[401] = {0}; 
    int length = 0;
    while (1) {
        char c = getchar();
        if (c == 4) {
            if (length == 0) {
                break;
            } else {
                putchar(7);
            }
        }
        else if (c == 127) {
            if (length > 0) {
                length--;
                refresh_line(buffer, length);
            } else {
                putchar(7);
            }
        }
        else if (c == 21) {
            length = 0;
            refresh_line(buffer, length);
        }
        else if (c == 23) {
            int i = length - 1;
            while (i >= 0 && isspace(buffer[i])) {
                i--;
            }
            while (i >= 0 && !isspace(buffer[i])) {
                i--;
            }
            length = i + 1;
            refresh_line(buffer, length);
        } else if (isprint(c)) {
            if (length < (int)(sizeof(buffer) - 1)) {
                buffer[length++] = c;
                refresh_line(buffer, length);
            } else {
                putchar(7);
            }
        }
        else {
            putchar(7);
        }
        fflush(stdout);
    }
    printf("\n");
    return 0;
}
