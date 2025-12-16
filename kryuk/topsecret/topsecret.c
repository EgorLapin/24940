#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

uid_t print_user_ids() {
    printf("\n");
    uid_t real_uid = getuid();
    printf("Real UID: %d\n", real_uid);
    printf("Effective UID: %d\n", geteuid());
    return real_uid;
}

void open_file(char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
    } else {
        fclose(file);
        printf("File opened successfully\n\n");
    }
}

int main(int argc, char *argv[]) {
    // Check if filename is provided
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    
    char *filename = argv[1];
    
    uid_t real_uid = print_user_ids();

    // open file
    open_file(filename);

    // set uid
    setuid(real_uid);
    printf("UID set to %d\n", real_uid);

    // repeat first and second step
    print_user_ids();

    open_file(filename);
    
    return 0;
}