#define _POSIX_C_SOURCE 200112L
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

static volatile sig_atomic_t beep_count = 0;
static volatile sig_atomic_t quit_requested = 0;

static void handle_sigint(int signo) {
    (void)signo;
    beep_count++;
    const char bell = '\a';
    
    (void)write(STDOUT_FILENO, &bell, 1);
}

static void handle_sigquit(int signo) {
    (void)signo;
    quit_requested = 1;
}

int main(void) {
    struct sigaction sa_int;
    struct sigaction sa_quit;

    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sa_int.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction(SIGINT)");
        return 1;
    }

    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sa_quit.sa_handler = handle_sigquit;
    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1) {
        perror("sigaction(SIGQUIT)");
        return 1;
    }

    for (;;) {
        if (quit_requested) {
            break;
        }
        pause();
    }

    printf("Signal sounded %d times\n", (int)beep_count);
    return 0;
}
