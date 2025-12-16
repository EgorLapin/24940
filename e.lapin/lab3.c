#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];

    printf("Step 1: Initial UIDs\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    printf("\nStep 2: Attempt to open file '%s'\n", filename);
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen failed");
    } else {
        printf("File '%s' opened successfully\n", filename);
        fclose(file);
    }

    printf("\nStep 3: Setting real and effective UID\n");
    if (setuid(geteuid()) == -1) {
        perror("setuid failed");
        exit(EXIT_FAILURE);
    }
    printf("setuid(geteuid()) called successfully\n");

    printf("\nStep 4: UIDs after setuid\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    printf("\nAttempt to open file '%s' again\n", filename);
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen failed");
    } else {
        printf("File '%s' opened successfully\n", filename);
        fclose(file);
    }

    return 0;
}
//3 команды в задании что делают
//4 вывести на экран из буфера 
//5
//отличия 6 от 7
//7 должна появиться таблица. В ней указаны отступы, количество символов, строк. 