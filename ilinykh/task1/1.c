#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/resource.h>
#include <linux/limits.h>

extern char *optarg;
extern int optopt;

void print_ids() {
    uid_t ruid = getuid();
    uid_t euid = geteuid();
    gid_t rgid = getgid();
    gid_t egid = getegid();
    printf("Real UID: %d, Effective UID: %d\n", ruid, euid);
    printf("Real GID: %d, Effective GID: %d\n", rgid, egid);
}

void print_pids() {
    printf("PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgrp());
}

void print_ulimit() {
    struct rlimit rl;
    if (getrlimit(RLIMIT_NPROC, &rl) == 0) {
        if (rl.rlim_cur == RLIM_INFINITY)
            printf("unlimited\n");
        else
            printf("%lu\n", rl.rlim_cur);
    } else {
        perror("getrlimit");
    }
}

int set_ulimit(const char *val_str) {
    char *endptr;
    long val = strtol(val_str, &endptr, 10);
    if (*endptr != '\0' || val < 0) {
        fprintf(stderr, "Invalid ulimit value: %s\n", val_str);
        return -1;
    }
    struct rlimit rl;
    rl.rlim_cur = rl.rlim_max = val;
    if (setrlimit(RLIMIT_NPROC, &rl) != 0) {
        perror("setrlimit");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    
    while ((opt = getopt(argc, argv, "ispucC:dvV:U:")) != -1) {
        switch (opt) {
            case 'i':
                print_ids();
                break;
            case 's':
                if (setpgid(0, 0) == 0)
                    printf("Set process group leader successfully\n");
                else
                    perror("setpgid");
                break;
            case 'p':
                print_pids();
                break;
            case 'u':
                print_ulimit();
                break;
            case 'U':
                set_ulimit(optarg);
                break;
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("Core file size limit: %lu bytes\n", rl.rlim_cur);
                    printf("Max core limit is %lu.\n", rl.rlim_max);
                } else {
                    perror("getrlimit");
                }
                break;
            }
            case 'C': {
                char *endptr;
                long val = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || val < 0) {
                    fprintf(stderr, "Invalid core file size: %s\n", optarg);
                    break;
                }
                struct rlimit rl;
                rl.rlim_cur = rl.rlim_max = val;
                if (setrlimit(RLIMIT_CORE, &rl) != 0) {
                    perror("setrlimit");
                } else {
                    printf("Core file size changed to %lu\n", val);
                }
                break;
            }
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current working directory: %s\n", cwd);
                } else {
                    perror("getcwd");
                }
                break;
            }
            case 'v': {
                extern char **environ;
                for (char **env = environ; *env != 0; env++) {
                    printf("%s\n", *env);
                }
                break;
            }
            case 'V': {
                char *eq = strchr(optarg, '=');
                if (!eq) {
                    fprintf(stderr, "Invalid environment variable format, expected name=value: %s\n", optarg);
                    break;
                }
                *eq = '\0';
                if (setenv(optarg, eq + 1, 1) == 0) {
                    printf("Environment variable %s set to %s\n", optarg, eq + 1);
                } else {
                    perror("setenv");
                }
                break;
            }
            case '?':
                fprintf(stderr, "Unknown option -%c\n", optopt);
                break;
        }
    }
    return 0;
}
