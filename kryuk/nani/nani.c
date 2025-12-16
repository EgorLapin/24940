#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>

#define MAX_LINE_LENGTH 40
#define BELL_CHAR '\007'

static struct termios original_termios;
static int terminal_configured = 0;

void restore_terminal(void) {
    if (terminal_configured) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        terminal_configured = 0;
    }
}

void signal_handler(int sig) {
    restore_terminal();
    exit(0);
}

int configure_terminal(void) {
    struct termios new_termios;
    
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        perror("tcgetattr");
        return -1;
    }
    
    new_termios = original_termios;
    
    new_termios.c_lflag &= ~ICANON;
    new_termios.c_lflag &= ~ECHO;
    new_termios.c_lflag &= ~ISIG;
    
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;
    
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
        perror("tcsetattr");
        return -1;
    }
    
    terminal_configured = 1;
    return 0;
}

int is_printable(char c) {
    return (c >= 32 && c <= 126);
}

int is_space(char c) {
    return (c == ' ' || c == '\t');
}

void erase_last_char(char *line, int *pos) {
    if (*pos > 0) {
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
    }
}

void kill_line(char *line, int *pos) {
    while (*pos > 0) {
        erase_last_char(line, pos);
    }
}

void erase_last_word(char *line, int *pos) {
    int start_pos = *pos;
    
    while (*pos > 0 && is_space(line[*pos - 1])) {
        (*pos)--;
    }
    
    while (*pos > 0 && !is_space(line[*pos - 1])) {
        (*pos)--;
    }
    
    int chars_to_erase = start_pos - *pos;
    for (int i = 0; i < chars_to_erase; i++) {
        printf("\b \b");
    }
    fflush(stdout);
}

void handle_line_wrap(char *line, int *pos) {
    if (*pos >= MAX_LINE_LENGTH) {
        int word_start = *pos;
        while (word_start > 0 && !is_space(line[word_start - 1])) {
            word_start--;
        }
        
        if (word_start == 0 || word_start >= MAX_LINE_LENGTH) {
            printf("\n");
            fflush(stdout);
            *pos = 0;
        } else {
            printf("\n");
            fflush(stdout);
            
            int word_len = *pos - word_start;
            memmove(line, line + word_start, word_len);
            *pos = word_len;
            
            for (int i = 0; i < word_len; i++) {
                printf("%c", line[i]);
            }
            fflush(stdout);
        }
    }
}

void process_input(void) {
    char line[MAX_LINE_LENGTH + 1];
    int pos = 0;
    char c;
    
    printf("Program started. Enter text (CTRL+D to exit):\n");
    fflush(stdout);
    
    while (1) {
        if (read(STDIN_FILENO, &c, 1) <= 0) {
            break;
        }
        
        if (c == 4 && pos == 0) {
            printf("\nProgram terminated.\n");
            break;
        }
        
        if (c == 4) {
            continue;
        }
        
        if (c == 127 || c == 8) {  // DEL or Backspace
            erase_last_char(line, &pos);
            continue;
        }
        
        if (c == 21) {  // CTRL+U
            kill_line(line, &pos);
            continue;
        }
        
        if (c == 23) {  // CTRL+W
            erase_last_word(line, &pos);
            continue;
        }
        
        if (c == '\n' || c == '\r') {
            printf("\n");
            fflush(stdout);
            pos = 0;
            continue;
        }
        
        if (!is_printable(c)) {
            printf("%c", BELL_CHAR);
            fflush(stdout);
            continue;
        }
        
        if (pos < MAX_LINE_LENGTH) {
            line[pos] = c;
            pos++;
            
            printf("%c", c);
            fflush(stdout);
            
            handle_line_wrap(line, &pos);
        } else {
            printf("%c", BELL_CHAR);
            fflush(stdout);
        }
    }
}

int main(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    
    atexit(restore_terminal);
    
    if (configure_terminal() == -1) {
        fprintf(stderr, "Ошибка настройки терминала\n");
        return 1;
    }
    
    process_input();
    
    restore_terminal();
    
    return 0;
}
