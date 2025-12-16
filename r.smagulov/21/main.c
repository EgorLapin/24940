#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static int signal_counter = 0;
static void signal_handler(int signum){
    switch (signum) {
        case SIGINT:
            putchar('\a');
            fflush(stdout);
            signal_counter++;
            signal(SIGINT, signal_handler);
            break;
        
        case SIGQUIT:
            printf("\nВсего сигналов получено: %d\n", signal_counter);
            _exit(0);
            break;
    }
}

int main(void){
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction for SIGINT");
        return EXIT_FAILURE;
    }
    if (sigaction(SIGQUIT, &sa, NULL) < 0) {
        perror("sigaction for SIGQUIT");
        return EXIT_FAILURE;
    }
    printf("Программа запущена. Нажмите Ctrl+C для пищалки, Ctrl+\\ чтобы выйти.\n");
    while (1) {
        pause();
    }
    return EXIT_SUCCESS;
}