#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

/* Простой редактор строки с поддержкой:
 * - Backspace/DEL (ERASE)
 * - Ctrl-U (KILL) — удаление текущей строки
 * - Ctrl-W (WERASE) — удаление последнего слова в текущей строке + пробелов
 * - Ctrl-D (EOF) — выход при пустой строке
 * - Ограничение 40 символов на строку
 * - Перенос слов на новую строку
 * - Игнорирование ESC-последовательностей (F-клавиши, стрелки и т.д.) и звуковой сигнал    
 * - Звуковой сигнал для всех неизвестных непечатаемых символов
 */

int main() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);  // Выключаем канонический режим и эхо
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    char buffer[4096] = {0};           // Буфер для всех строк
    int total_len = 0;                 // Общая длина буфера

    // Специальные символы
    char erase_char = oldt.c_cc[VERASE];   // Обычно Backspace или DEL
    char kill_char  = oldt.c_cc[VKILL];    // Ctrl-U
    char werase_char = '\027';             // Ctrl-W
    char eof_char   = '\004';              // Ctrl-D
    char bell_char  = '\007';              // CTRL-G
    char esc_char   = '\033';              // ESC

    // Приветственное сообщение
    printf("String editor started. Use Backspace or DEL for erase char (VERASE), "
           "Ctrl-U for kill line (VKILL), Ctrl-W for kill word, "
           "Ctrl-D at empty to exit.\n"
           "Non-printable keys beep.\n");
    fflush(stdout);

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;

        // === Определяем начало текущей строки ===
        int line_start = total_len - 1;
        while (line_start >= 0 && buffer[line_start] != '\n') line_start--;
        line_start += 1;
        int line_len = total_len - line_start;

        // === Обработка специальных символов ===
        if (c == eof_char) {
            if (line_len == 0) break;      // Ctrl-D в начале строки — выход
            else putchar(bell_char);

        } else if (c == erase_char) {
            if (line_len > 0) {
                total_len--;
                printf("\b \b");           // Стираем символ
            }

        } else if (c == kill_char) {
            for (int i = 0; i < line_len; i++) {
                printf("\b \b");
            }
            total_len = line_start;        // Удаляем всю строку

        } else if (c == werase_char) {

            int pos = total_len;

            // Пропускаем конечные пробелы
            while (pos > line_start && buffer[pos - 1] == ' ') pos--;

            // Случай, когда только пробелы — удаляем всю строку
            if (pos == line_start) {
                
                for (int i = 0; i < line_len; i++) printf("\b \b");
                total_len = line_start;
                continue;
            }

            // Пропускаем слово
            int word_end = pos;
            while (pos > line_start && buffer[pos - 1] != ' ') pos--;
            int word_start = pos;

            // Пропускаем пробелы перед словом
            while (pos > line_start && buffer[pos - 1] == ' ') pos--;

            // Удаляем от pos до total_len
            int erase_count = total_len - pos;
            for (int i = 0; i < erase_count; i++) {
                printf("\b \b");
            }
            total_len = pos;

        } else if (c == esc_char) {
            // Игнорируем ESC-последовательности (F-клавиши, стрелки и т.д.)
            putchar(bell_char);
            char next;
            if (read(STDIN_FILENO, &next, 1) == 1) {
                if (next == '[') {  // CSI: \e[...]
                    while (read(STDIN_FILENO, &next, 1) == 1) {
                        if (next >= '@' && next <= '~') break;
                    }
                } else if (next == 'O') {  // SS3: \eO...
                    read(STDIN_FILENO, &next, 1);
                }
            }

        } else if (isprint(c)) {

            // === Обработка печатаемых символов ===
            if (line_len < 40) {
                buffer[total_len++] = c;
                putchar(c);
            } else {
                if (c == ' ') {
                    putchar(bell_char);    // Пробел на 40-й позиции — сигнал
                } else {

                    int last_is_space = (total_len > line_start && buffer[total_len - 1] == ' ');
                    if (last_is_space) {
                        // Можно начать новую строку
                        buffer[total_len++] = '\n';
                        putchar('\n');
                        buffer[total_len++] = c;
                        putchar(c);
                    } else {
                        // Переносим последнее слово на новую строку
                        int word_end = total_len;
                        int word_start = total_len;
                        while (word_start > line_start && buffer[word_start - 1] != ' ') {
                            word_start--;
                        }
                        int word_len = word_end - word_start;
                        char word[50];
                        memcpy(word, buffer + word_start, word_len);

                        // Стираем слово
                        for (int i = 0; i < word_len; i++) printf("\b \b");
                        total_len = word_start;

                        // Добавляем перенос
                        buffer[total_len++] = '\n';
                        putchar('\n');

                        // Возвращаем слово
                        memcpy(buffer + total_len, word, word_len);
                        total_len += word_len;
                        for (int i = 0; i < word_len; i++) putchar(word[i]);

                        // Добавляем новый символ
                        buffer[total_len++] = c;
                        putchar(c);
                    }
                }
            }

        } else {
            // Все остальные непечатаемые символы
            putchar(bell_char);
        }

        fflush(stdout);
    }

    // Восстанавливаем терминал
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return 0;
}