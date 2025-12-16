#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int c = 0;

void i() {
    signal(SIGINT, i);
    write(1, "\a", 1);
    c++;
}

void q() {
    printf("\nСигнал прозвучал %d раз\n", c);
    exit(0);
}

int main() {
    puts("Программа входит в бесконечный цикл и издает звуковой сигнал на терминале при каждом получении SIGINT (по умолчанию Ctrl+C). При получении SIGQUIT (по умолчанию Ctrl+\\), она выведет сообщение с количеством раз, когда прозвучал сигнал, и завершится.");
    signal(SIGINT, i);
    signal(SIGQUIT, q);
    for (;;) {
        pause();
    }
}