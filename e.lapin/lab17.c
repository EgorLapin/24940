#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#define MAX_LINE 40

void set_terminal_mode(void) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ECHO | ICANON);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void restore_terminal_mode(void) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

int is_printable_char(char ch) {
    return (ch >= 0x20 && ch <= 0x7E);
}

void skip_escape_sequence(void) {
    char ch;

    if (read(STDIN_FILENO, &ch, 1) != 1 || ch != '[') {
        return;
    }

    while (read(STDIN_FILENO, &ch, 1) == 1) {
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '~') {
            break;
        }
    }
}

int main() {
    char ch;
    char buffer[MAX_LINE + 1] = {0};
    int pos = 0;

    set_terminal_mode();
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    char erase = term.c_cc[VERASE];
    char kill = term.c_cc[VKILL];
    char ctrl_w = 0x17;
    char ctrl_d = 0x04;

    while (read(STDIN_FILENO, &ch, 1) == 1) {

        if (ch == 0x1B) {
            skip_escape_sequence();
            printf("\a");
            fflush(stdout);
        }

        else if (ch == ctrl_d && pos == 0) {
            printf("\n");
            break;
        }

        else if (ch == erase && pos > 0) {
            pos--;
            buffer[pos] = '\0';
            printf("\b \b");
            fflush(stdout);
        }

        else if (ch == kill) {
            if (pos > 0) {
                printf("\r\033[K");
                pos = 0;
                buffer[0] = '\0';
                fflush(stdout);
            }
        }

        else if (ch == ctrl_w && pos > 0) {
            int old_pos = pos;

            while (pos > 0 && buffer[pos - 1] == ' ') {
                pos--;
                printf("\b \b");
            }

            while (pos > 0 && buffer[pos - 1] != ' ') {
                pos--;
                printf("\b \b");
            }

            memset(buffer + pos, 0, old_pos - pos);
            fflush(stdout);
        }

        else if (is_printable_char(ch)) {
            if (pos == MAX_LINE && ch != ' ') {
                int word_start = pos;

                while (word_start > 0 && buffer[word_start - 1] != ' ') {
                    word_start--;
                }

                for (int i = word_start; i < pos; i++) {
                    printf("\b \b");
                }

                fflush(stdout);
                printf("\n%s", buffer + word_start);
                fflush(stdout);

                memmove(buffer, buffer + word_start, pos - word_start);
                pos = pos - word_start;
                buffer[pos] = '\0';
            }

            if (pos < MAX_LINE) {
                buffer[pos++] = ch;
                printf("%c", ch);
                fflush(stdout);
            }
        }

        else if (!is_printable_char(ch) && ch != erase && ch != kill && ch != ctrl_w && ch != ctrl_d) {
            printf("\a");
            fflush(stdout);
        }
    }

    restore_terminal_mode();
    return 0;
}