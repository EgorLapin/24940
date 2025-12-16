#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

char* clean_input(const char *input) {
    if (input == NULL) return NULL;
    size_t len = strlen(input);
    char *cleaned = malloc(len + 1);
    if (!cleaned) return NULL;

    int pointer = 0;
    int in_escape = 0;
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\033' && i + 1 < len && input[i + 1] == '[') {
            in_escape = 1;
            i++; 
            continue;
        }
        if (in_escape) {
            if ((input[i] >= 'a' && input[i] <= 'z') || 
                (input[i] >= 'A' && input[i] <= 'Z') || 
                input[i] == ';' || input[i] == '~') {
                in_escape = 0;
            }
            continue;
        }
        if (isprint(input[i]) || input[i] == '.' || input[i] == '\t') {
            cleaned[pointer++] = input[i];
        }
    }
    cleaned[pointer] = '\0';
    return cleaned;
}

void append(Node **head, Node **tail, const char *line) {
    Node *new_node = malloc(sizeof(Node));
    if (!new_node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_node->data = strdup(line);
    if (!new_node->data) {
        perror("strdup");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    new_node->next = NULL;
    if (*head == NULL) {
        *head = new_node;
        *tail = new_node;
    } else {
        (*tail)->next = new_node;
        *tail = new_node;
    }
}

void free_list(Node *head) {
    while (head) {
        Node *tmp = head;
        head = head->next;
        free(tmp->data);
        free(tmp);
    }
}

int main() {
    Node *head = NULL;
    Node *tail = NULL;
    char *buffer = NULL;
    size_t bufsize = 0;

    printf("Введите строки (начинайте строку с '.' чтобы завершить):\n");

    while (1) {
        ssize_t nread = getline(&buffer, &bufsize, stdin);
        if (nread == -1) break; 

        if(nread > 0 && buffer[nread -1] == '\n') {
            buffer[nread -1] = '\0';
        }

        char *cleaned = clean_input(buffer);
        if (!cleaned) {
            fprintf(stderr, "Error cleaning input\n");
            break;
        }

        if (cleaned[0] == '.') {
            free(cleaned);
            break;
        }
        if (strlen(cleaned) == 0) {
            free(cleaned);
            continue; 
        }

        append(&head, &tail, cleaned);
        free(cleaned);
    }

    free(buffer);

    printf("\nВведённые строки:\n");
    for (Node *cur = head; cur != NULL; cur = cur->next) {
        printf("%s\n", cur->data);
    }

    free_list(head);
    return 0;
}
