#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

void add_to_list(Node **head, Node **tail, char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->str = str;
    new_node->next = NULL;
    
    if (*head == NULL) {
        *head = new_node;
        *tail = new_node;
    } else {
        (*tail)->next = new_node;
        *tail = new_node;
    }
}

static struct termios oldt;

char *read_line(void) {
    int capacity = 16;
    char *line = (char *)malloc(capacity);

    int size = 0;
    int c;

    while (1) {
        c = getchar();
        if (c == '\n') {
            break;
        }

        if (c == 27) {
            c = getchar();
            if (c == '\n') break;
            if (c == '[') {
                do {
                    c = getchar();
                    if (c == '\n') {
                        line[size] = '\0';
                        return line;
                    }
                } while (!(c >= '@' && c <= '~'));
            }
            continue;
        }

        if (c == 127 || c == 8) {
            if (size > 0) {
                size--;
            }
            continue;
        }

        if (isprint((unsigned char)c) || c == '\t') {
            if (size + 1 >= capacity) {
                capacity *= 2;
                char *temp = (char *)realloc(line, capacity);
                line = temp;
            }
            line[size++] = (char)c;
        }
    }

    line[size] = '\0';
    return line;
}

int main(void) {
    Node *head = NULL;
    Node *tail = NULL;

    struct termios newt;

    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (1) {
        char *str = read_line();

        if (strlen(str) == 0) {
            free(str);
            continue;
        }

        if (str[0] == '.') {
            free(str);
            break;
        }

        add_to_list(&head, &tail, str);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    Node *current = head;
    while (current != NULL) {
        printf("%s\n", current->str);
        current = current->next;
    }

    current = head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp->str);
        free(temp);
    }

    return 0;
}