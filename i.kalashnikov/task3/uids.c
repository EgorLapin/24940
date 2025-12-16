#include <stdio.h>
#include <unistd.h>

void func() {
    int uid = getuid();
    int euid = geteuid();
    printf("%d %d\n", uid, euid);

    FILE *file = fopen("file.txt", "r+");

    if (file == NULL) {
        perror("File not opened");
    }
    else {
        printf("File opened successfully\n");
        fclose(file);
    }
}

int main(){

    func();
    setuid(getuid());
    func();

    return 0;
}