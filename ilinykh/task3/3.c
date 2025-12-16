#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void test_file(const char* path)
{
    FILE* file = fopen(path, "r");
    if (file == NULL)
    {
        perror("can`t open file");
    }
    else
    {
        printf("opened succesfuly\n");
        fclose(file);
        file = NULL;
    }
}

void printUserIDs() {
    printf("The real user ID: %d\n", getuid());
    printf("The effective user ID: %d\n", geteuid());
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "not enough arguments\n");
        exit(-1);
    }

    printUserIDs();

    test_file(argv[1]);

    if (setuid(getuid()) == -1)
    {
        perror("setuid failed");
        exit(-1);
    }

    printUserIDs();

    test_file(argv[1]);

    exit(0);
}
