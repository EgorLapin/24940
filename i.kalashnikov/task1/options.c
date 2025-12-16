#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

typedef struct {
    char opt;
    char *arg;
} Action;

int main(int argc, char *argv[]) {
    char options[] = "ispuU:cC:dvV:";
    Action *actions = NULL;
    int num_actions = 0;
    int c;

    printf("argc equals %d\n", argc);

    while ((c = getopt(argc, argv, options)) != -1) {
        if (c == '?') {
            printf("invalid option is %c\n", optopt);
            continue;
        }

        actions = (Action *)realloc(actions, (num_actions + 1) * sizeof(Action));
        if (actions == NULL) {
            perror("realloc failed");
            exit(1);
        }
        actions[num_actions].opt = c;
        actions[num_actions].arg = optarg ? strdup(optarg) : NULL;
        num_actions++;
    }

    for (int i = num_actions - 1; i >= 0; i--) {
        char opt = actions[i].opt;
        char *arg = actions[i].arg;
        switch (opt) {
            case 'i':
                // Real and effective IDs
                printf("Real UID: %d, Effective UID: %d, Real GID: %d, Effective GID: %d\n",
                       getuid(), geteuid(), getgid(), getegid());
                break;
            case 's':
                // Make process a group leader
                if (setpgid(0, 0) < 0) {
                    perror("setpgid failed");
                }
                break;
            case 'p':
                // Print PID, PPID, and PGID
                printf("PID: %d, PPID: %d, PGID: %d\n",
                       getpid(), getppid(), getpgrp());
                break;
            case 'u':
                // Print ulimit
                {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_NPROC, &rl) < 0) {
                        perror("getrlimit failed");
                    } else {
                        if (rl.rlim_cur == RLIM_INFINITY) {
                            printf("ulimit: unlimited\n");
                        } else {
                            printf("ulimit: %lld\n", (long long)rl.rlim_cur);
                        }
                    }
                }
                break;
            case 'U':
                // Change ulimit
                {
                    char *endptr;
                    errno = 0;
                    long newlim = strtol(arg, &endptr, 10);
                    if (endptr == arg || *endptr != '\0' || errno != 0 || newlim < 0) {
                        fprintf(stderr, "Invalid value for -U: %s\n", arg);
                    } else {
                        struct rlimit rl;
                        if (getrlimit(RLIMIT_FSIZE, &rl) < 0) {
                            perror("getrlimit failed");
                        } else {
                            rl.rlim_cur = newlim;
                            rl.rlim_max = newlim;
                            if (setrlimit(RLIMIT_FSIZE, &rl) < 0) {
                                perror("setrlimit failed");
                            }
                        }
                    }
                }
                break;
            case 'c':
                // Print the core file size
                {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_CORE, &rl) < 0) {
                        perror("getrlimit failed");
                    } else {
                        if (rl.rlim_cur == RLIM_INFINITY) {
                            printf("core size: unlimited\n");
                        } else {
                            printf("core size: %lld\n", (long long)rl.rlim_cur);
                        }
                    }
                }
                break;
            case 'C':
                // Change the core file size
                {
                    char *endptr;
                    errno = 0;
                    long newlim = strtol(arg, &endptr, 10);
                    if (endptr == arg || *endptr != '\0' || errno != 0 || newlim < 0) {
                        fprintf(stderr, "Invalid value for -C: %s\n", arg);
                    } else {
                        struct rlimit rl;
                        if (getrlimit(RLIMIT_CORE, &rl) < 0) {
                            perror("getrlimit failed");
                        } else {
                            rl.rlim_cur = newlim;
                            rl.rlim_max = newlim;
                            if (setrlimit(RLIMIT_CORE, &rl) < 0) {
                                perror("setrlimit failed");
                            }
                        }
                    }
                }
                break;
            case 'd':
                // Current dir
                {
                    char buf[PATH_MAX];
                    if (getcwd(buf, sizeof(buf)) == NULL) {
                        perror("getcwd failed");
                    } else {
                        printf("%s\n", buf);
                    }
                }
                break;
            case 'v':
                // Print environment variables
                for (char **env = environ; *env != NULL; env++) {
                    printf("%s\n", *env);
                }
                break;
            case 'V':
                // Change environment variables
                if (strchr(arg, '=') == NULL) {
                    fprintf(stderr, "Invalid format for -V: %s (must be name=value)\n", arg);
                } else {
                    if (putenv(arg) != 0) {
                        perror("putenv failed");
                    }
                }
                break;
        }
    }

    for (int i = 0; i < num_actions; i++) {
        if (actions[i].arg) free(actions[i].arg);
    }
    free(actions);

    printf("optind equals %d\n", optind);
    if (optind < argc) {
        printf("next parameter = %s\n", argv[optind]);
    }

    return 0;
}