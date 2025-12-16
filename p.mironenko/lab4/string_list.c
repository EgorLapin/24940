/* -*- coding: koi8-r -*-
 * Программа для создания списка строк с динамическим выделением памяти
 * Кодировка: KOI8-R
 * Компиляция: cc -o string_list string_list.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Структура для узла списка
typedef struct Node {
    char *str;          // Указатель на строку
    struct Node *next;  // Указатель на следующий узел
} Node;

// Функция для добавления строки в конец списка
Node *add_string(Node *head, const char *input) {
    Node *new_node = (Node *)malloc(sizeof(Node)); //Выделяет память для нового узла и строки
    if (new_node == NULL) { //проверка на выделение памяти
        perror("Ошибка выделения памяти для узла");
        return head;
    }

    // Выделяем память под строку (+1 для нулевого символа)
    new_node->str = (char *)malloc(strlen(input) + 1);
    if (new_node->str == NULL) {
        perror("Ошибка выделения памяти для строки");
        free(new_node);
        return head;
    }

    // Копируем строку (добавляет узел в конец списка)
    strcpy(new_node->str, input);
    new_node->next = NULL;

    // Если список пуст, новый узел становится головой
    if (head == NULL) {
        return new_node;
    }

    // Ищем последний узел
    Node *current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;

    return head;
}

// Ф-ия для вывода списка строк
void print_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        printf("%s", current->str);
        current = current->next;
    }
}

// Функция для освобождения памяти списка
void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp->str);  // Освобождаем строку
        free(temp);       // Освобождаем узел
    }
}

int main() {
    char buffer[1024];  // Буфер для ввода строки
    Node *head = NULL;  // Голова списка

    printf("Введите строки (ввод завершится при строке, начинающейся с '.')\n");

    // Чтение строк до точки
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            perror("Ошибка чтения строки");
            break;
        }

        // Удаляем символ новой строки, если он есть
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        // Проверяем, начинается ли строка с точки
        if (len > 0 && buffer[0] == '.') {
            break;
        }

        // Добавляем строку в список
        head = add_string(head, buffer);
    }

    // Выводим список
    printf("\nСписок введенных строк:\n");
    print_list(head);
    printf("\n");

    // Освобождаем память
    free_list(head);

    return 0;
}