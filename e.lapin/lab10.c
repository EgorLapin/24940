#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

static char *sh_quote(const char *s) {
    size_t len = 2;
    for (const char *p = s; *p; ++p) {
        if (*p == '\'') len += 4;
        else len += 1;
    }

    char *out = malloc(len + 1);
    if (!out) return NULL;

    char *q = out;
    *q++ = '\'';
    for (const char *p = s; *p; ++p) {
        if (*p == '\'') {
            *q++ = '\'';
            *q++ = '\\';
            *q++ = '\'';
            *q++ = '\'';
        } else {
            *q++ = *p;
        }
    }
    *q++ = '\'';
    *q = '\0';
    return out;
}

static char *build_cmdline(int argc, char *argv[]) {
    size_t cap = 64;
    size_t used = 0;
    char *cmd = malloc(cap);
    if (!cmd) return NULL;
    cmd[0] = '\0';

    for (int i = 1; i < argc; i++) {
        char *q = sh_quote(argv[i]);
        if (!q) { free(cmd); return NULL; }

        size_t need = strlen(q) + 1;
        if (used + need + 1 > cap) {
            while (used + need + 1 > cap) cap *= 2;
            char *tmp = realloc(cmd, cap);
            if (!tmp) { free(q); free(cmd); return NULL; }
            cmd = tmp;
        }

        if (used > 0) cmd[used++] = ' ';
        memcpy(cmd + used, q, strlen(q));
        used += strlen(q);
        cmd[used] = '\0';

        free(q);
    }
    return cmd;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);

        char *cmdline = build_cmdline(argc, argv);
        if (cmdline) {
            execl("/bin/sh", "sh", "-c", cmdline, (char *)NULL);
            free(cmdline);
        }

        perror("exec");
        _exit(127);
    }

    int status;
    for (;;) {
        if (waitpid(pid, &status, 0) == -1) {
            if (errno == EINTR) continue;
            perror("waitpid");
            return 1;
        }
        break;
    }

    if (WIFEXITED(status)) {
        printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Дочерний процесс завершился по сигналу: %d\n", WTERMSIG(status));
    } else {
        printf("Дочерний процесс завершился необычным образом.\n");
    }

    return 0;
}
