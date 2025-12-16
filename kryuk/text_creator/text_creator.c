#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structure for linked list node
struct Node {
    char *data;         
    struct Node *next;  
};

char* clean_input(const char *input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t len = strlen(input);
    char *cleaned = malloc(len + 1);
    
    int pointer = 0;
    int in_escape = 0;
    
    for (size_t i = 0; i < len; i++) {
        // Check for escape sequence start
        if (input[i] == '\033' && i + 1 < len && input[i + 1] == '[') {
            in_escape = 1;
            continue;
        }
        
        if (in_escape) {
            // Escape sequences typically end with letters or semicolon
            if (isalpha(input[i]) || input[i] == ';' || input[i] == '~') {
                in_escape = 0;
            }
            continue;
        }
        
        // Keep printable characters and tabs
        if (isprint(input[i]) || input[i] == '\t' || input[i] == '.') {
            cleaned[pointer] = input[i];
            pointer++;
        }
    }
    
    cleaned[pointer] = '\0';
    return cleaned;
}

int add_string(struct Node **head, struct Node **current) {
    printf("> ");
        
    char *input = NULL;
    size_t input_size = 0;

    // Read input
    if (getline(&input, &input_size, stdin) == -1) {
        printf("Error reading input\n");
        return 1;
    }
    
    if (input == NULL) {
        printf("Error reading input\n");
        return 1;
    }
    
    // Clean the input to remove escape sequences and control characters
    char *cleaned_input = clean_input(input);
    if (cleaned_input == NULL) {
        printf("Error cleaning input\n");
        free(input);
        return 1;
    }
    
    // Check if input is a dot (termination character)
    if (cleaned_input[0] == '.') {
        free(input);
        free(cleaned_input);
        return 1;
    }
    
    // Check if input is empty after cleaning
    if (strlen(cleaned_input) == 1) {
        free(input);
        free(cleaned_input);
        return 0;  // Continue reading
    }
    
    // Allocate memory
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        free(input);
        free(cleaned_input);
        return 1;
    }
    
    newNode->data = (char*)malloc(strlen(cleaned_input) + 1);
    if (newNode->data == NULL) {
        printf("Memory allocation failed\n");
        free(newNode);
        free(input);
        free(cleaned_input);
        return 1;
    }
    
    // Copy cleaned string to allocated memory
    strcpy(newNode->data, cleaned_input);
    newNode->next = NULL;
    
    // Add node to list
    if (*head == NULL) {
        *head = newNode;
        *current = newNode;
    } else {
        (*current)->next = newNode;
        *current = newNode;
    }

    // Free input buffers
    free(input);
    free(cleaned_input);

    return 0;
}

int main() {
    struct Node *head = NULL;  // Head of the linked list
    struct Node *current = NULL;  // Current node pointer
    
    printf("Enter strings:\n");
    
    while (1) {
        if (add_string(&head, &current)) {
            break;
        }
    }
    
    printf("\nAll entered strings:\n");
    current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }
    
    return 0;
}

