#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>

int main() {
    printf("[PROGRAM]: родитель пишет текст в pipe, дочерний переводит в uppercase и выводит.\n");

    int fd[2];
    pipe(fd);
    
    if (fork() == 0){

        printf("[CHILD]: читаю из pipe, перевожу в uppercase и вывожу.\n");
        close(fd[1]);
        char buf[1024];
        int n;

        while ((n = read(fd[0], buf, sizeof(buf))) > 0) {
            for (int i = 0; i < n; i++) buf[i] = toupper(buf[i]);
            write(1, buf, n);
        }

        close(fd[0]);
        exit(0);

    }else{
        char *text = "Hello iir!\n";
        write(1, text, strlen(text));

        printf("[PARENT]: пишу текст в pipe.\n");
        close(fd[0]);

        write(fd[1], text, strlen(text));

        close(fd[1]);
        wait(NULL);
    }
    return 0;
}