#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_ids() {
    uid_t u_id = getuid();
    uid_t u_eid = geteuid();

    printf("%u\n", u_id);
    printf("%u\n", u_eid);
}

void open_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        perror("Cannot open file\n");
    } else {
        fclose(file);
    }
}


int main(int argc, char* argv[]) {

    const char *filename = "secret.txt";

    if(argc == 2) {
        filename = argv[1];
    } else if(argc > 2){
        perror("Not valid args\n");
    }

    print_ids();
    open_file(filename);

    if(getuid() != geteuid()) {
        if(setuid(geteuid()) == -1) {
            perror("Error");
        } else {
            printf("All good\n");
        }
    } else {
        printf("Already equal\n");
    }
        

    print_ids();
    open_file(filename);

    return 0;
}