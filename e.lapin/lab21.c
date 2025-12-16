#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t counter = 0;

void sigint_handler(int sig) {
    counter++;
    write(STDOUT_FILENO, "\a", 1);
}

void sigquit_handler(int sig) {
    char buf[64];
    int n = snprintf(buf, sizeof(buf),
                     "\nПищалка сработала %d раз(а).\n", counter);
    write(STDOUT_FILENO, buf, n);
    _exit(0);
}

int main() {
    struct sigaction sa_int = {0};
    struct sigaction sa_quit = {0};

    sa_int.sa_handler = sigint_handler;
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    sa_quit.sa_handler = sigquit_handler;
    sigaction(SIGQUIT, &sa_quit, NULL);

    printf("Ctrl+C — писк, Ctrl+\\ — завершение.\n");

    while (1) {
        pause();
    }

    return 0;
}