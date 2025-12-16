#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Структура для узла списка
typedef struct Node {
    char *str;           // Указатель на строку
    struct Node *next;   // Указатель на следующий узел
} Node;

// Функция для добавления строки в конец списка
void append(Node **head, char *input_str){

    // Выделяем память под новый узел
    Node *new_node = (Node *)malloc(sizeof(Node));
    
    // Выделяем память под строку (включая завершающий нуль)
    int len = strlen(input_str);
    new_node->str = (char *)malloc(len + 1);

    // Копируем строку
    strcpy(new_node->str, input_str);
    new_node->next = NULL;

    // Если список пуст, новый узел становится головой
    if (*head == NULL) {
        *head = new_node;
        return;
    }

    // Находим последний узел
    Node *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    // Добавляем новый узел в конец
    current->next = new_node;
}

// Функция для вывода списка
void print_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        printf("%s ", current->str);
        current = current->next;
    }
    printf("\n");
}

// Функция для освобождения памяти
void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp->str); // Освобождаем строку
        free(temp);      // Освобождаем узел
    }
}

int main() {
    Node *head = NULL;
    char buffer[1024]; // Буфер для чтения строки

    // Чтение строк до ввода точки
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        // Проверяем, начинается ли строка с точки
        if (buffer[0] == '.') {
            break;
        }

        // Удаляем символ новой строки, если он есть
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Добавляем строку в список
        append(&head, buffer);
    }

    // Выводим список
    print_list(head);

    // Освобождаем память
    free_list(head);

    return 0;
}