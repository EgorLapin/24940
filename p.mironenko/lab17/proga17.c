//Задача 17: Строчный редактор (raw mode + ERASE, KILL, ^W, word wrap 40)

// Компиляция: cc -o editor proga17.c
// Запуск: ./editor
//
// Управление:
//   Ctrl+D (в начале строки) - выход (если строка пустая)
//   Backspace - удалить последний символ
//   Ctrl+U - очистить строку
//   Ctrl+W - удалить последнее слово
//   Enter - завершить ввод строки
//   Использовать стрелки left right up down запрещено!


#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAX_COLS        40
#define ERASE           127     // DEL
#define KILL            21      // Ctrl+U
#define WERASE          23      // Ctrl+W
#define EOF_CHAR        4       // Ctrl+D
#define BELL            7

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/* Полностью перерисовывает буфер с переносом по словам и возвращает курсор в конец */
void redraw(const char *buf, int len) {
    printf("\r\033[K");  // в начало строки + очистить строку

    int col = 0;
    int i = 0;
    int line_count = 1;         // сколько строк уже вывели (начинаем с 1)

    while (i < len) {
        int word_start = i;
        while (i < len && !isspace(buf[i])) i++;
        int word_len = i - word_start;

        int spaces_after = 0;
        int space_start = i;
        while (i < len && isspace(buf[i])) {
            i++;
            spaces_after++;
        }

        // Проверяем, помещается ли слово
        if (col > 0 && col + word_len > MAX_COLS) {
            printf("\n\033[K");  // новая строка + очистить её
            line_count++;
            col = 0;
        }

        // Печатаем слово
        if (col == 0 && word_len > MAX_COLS) {
            // Очень длинное слово — режем по 40
            for (int k = 0; k < MAX_COLS; k++) {
                putchar(buf[word_start + k]);
            }
            col = MAX_COLS;
        } else {
            for (int k = word_start; k < word_start + word_len; k++) {
                putchar(buf[k]);
            }
            col += word_len;
        }

        // Печатаем пробелы (но не больше, чем до конца строки)
        int spaces_to_print = spaces_after;
        if (col + spaces_to_print > MAX_COLS) {
            spaces_to_print = MAX_COLS - col;
        }
        for (int s = 0; s < spaces_to_print; s++) {
            putchar(' ');
        }
        col += spaces_to_print;

        // Если строка заполнена — перенос
        if (col >= MAX_COLS && i < len) {
            printf("\n\033[K");
            line_count++;
            col = 0;
        }
    }

    // Теперь возвращаем курсор в конец текста
    // Мы находимся в конце последней строки, нужно подняться на (line_count - 1) строк вверх
    if (line_count > 1) {
        printf("\033[%dA", line_count - 1);  // поднять курсор
    }
    // И переместить в нужную колонку (col)
    if (col > 0) {
        printf("\033[%dG", col + 1);  // \033[nG — абсолютная колонка (1-based)
    } else {
        printf("\r");                 // если col == 0 — в начало строки
    }

    fflush(stdout);
}

int main() {
    enableRawMode();

    char buffer[4096] = {0};
    int pos = 0;

    printf("\033[?25l");  // скрыть курсор на старте (по желанию)
    redraw(buffer, pos);

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) continue;

        if (c == EOF_CHAR && pos == 0) {
            printf("\n");
            break;
        }

        if (c == ERASE || c == '\b' || c == 8) {  // 8 = Ctrl+H
            if (pos > 0) {
                pos--;
                buffer[pos] = '\0';
                redraw(buffer, pos);
            } else {
                putchar(BELL); fflush(stdout);
            }
        }
        else if (c == KILL) {
            pos = 0;
            buffer[0] = '\0';
            redraw(buffer, pos);
        }
        else if (c == WERASE) {
            if (pos == 0) { putchar(BELL); fflush(stdout); continue; }

            while (pos > 0 && isspace(buffer[pos-1])) pos--;
            while (pos > 0 && !isspace(buffer[pos-1])) pos--;
            buffer[pos] = '\0';
            redraw(buffer, pos);
        }
        else if (isprint(c) || c == ' ' || c == '\t') {
            if (c == '\t') c = ' ';
            if (pos >= sizeof(buffer)-1) {
                putchar(BELL); fflush(stdout);
                continue;
            }
            buffer[pos++] = c;
            buffer[pos] = '\0';
            redraw(buffer, pos);
        }
        else {
            putchar(BELL); fflush(stdout);
        }
    }

    printf("\033[?25h");  // показать курсор обратно
    disableRawMode();
    return 0;
}