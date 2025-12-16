#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 256

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

Node *create_node(char *str) {
    Node *res = malloc(sizeof(Node));
    size_t len = strlen(str) + 1;
    res->str = malloc(len);
    strcpy(res->str, str);
    res->next = NULL;
    return res;
}

void print(Node *head) {
    Node *temp = head;
    while (temp != NULL) {
        printf("%s\n", temp->str);
        temp = temp->next;
    }
}

void free_list(Node *head) {
    Node *temp;
    while (head) {
        temp = head->next;
        free(head->str);
        free(head);
        head = temp;
    }
}

int main() {
    char buffer[MAX_LEN] = {0};
    Node *head = NULL, *tail = NULL;

    while (1) {

        if (!fgets(buffer, sizeof(buffer), stdin))
            break;
        if (buffer[0] == '.')
            break;
            
        buffer[strcspn(buffer, "\n")] = '\0';
        
        Node *new_node = create_node(buffer);

        if (head == NULL) {
            head = new_node; 
            tail = new_node; 
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    print(head);
    free_list(head);
    return 0;
}
