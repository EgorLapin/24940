#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <getopt.h>

// Custom strdup implementation for Solaris compatibility
char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup != NULL) {
        strcpy(dup, s);
    }
    return dup;
}

void print_usage(const char *progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("  -i    Print real and effective user and group IDs\n");
    printf("  -s    Process becomes group leader\n");
    printf("  -p    Print process, parent process, and process group IDs\n");
    printf("  -u    Print ulimit value\n");
    printf("  -U    Change ulimit value\n");
    printf("  -c    Print core file size limit in bytes\n");
    printf("  -C    Change core file size limit\n");
    printf("  -d    Print current working directory\n");
    printf("  -v    Print environment variables and their values\n");
    printf("  -V    Set environment variable (name=value)\n");
}

void print_user_group_ids() {
    printf("\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("Real GID: %d\n", getgid());
    printf("Effective GID: %d\n", getegid());
}

void become_group_leader() {
    printf("\n");
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
        return;
    }
    printf("Process became group leader\n");
}

void print_process_ids() {
    printf("\n");
    printf("Process ID: %d\n", getpid());
    printf("Parent Process ID: %d\n", getppid());
    printf("Process Group ID: %d\n", getpgrp());
}

void print_rlimit(int resource, const char *description) {
    printf("\n");
    struct rlimit rlim;
    if (getrlimit(resource, &rlim) == -1) {
        perror("getrlimit");
        return;
    }
    printf("%s: %ld bytes\n", description, (long)rlim.rlim_cur);
}

void change_rlimit(int resource, const char *value, const char *description) {
    printf("\n");
    long new_limit = atol(value);
    if (new_limit < 0) {
        fprintf(stderr, "Invalid %s value: %s\n", description, value);
        return;
    }
    
    struct rlimit rlim;
    rlim.rlim_cur = (rlim_t)new_limit;
    rlim.rlim_max = (rlim_t)new_limit;
    
    if (setrlimit(resource, &rlim) == -1) {
        perror("setrlimit");
        return;
    }
    printf("%s changed to %ld bytes\n", description, new_limit);
}

void print_ulimit() {
    print_rlimit(RLIMIT_FSIZE, "File size limit");
}

void change_ulimit(const char *value) {
    change_rlimit(RLIMIT_FSIZE, value, "File size limit");
}

void print_core_size() {
    print_rlimit(RLIMIT_CORE, "Core file size limit");
}

void change_core_size(const char *value) {
    change_rlimit(RLIMIT_CORE, value, "Core file size limit");
}

void print_current_directory() {
    printf("\n");
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return;
    }
    printf("Current working directory: %s\n", cwd);
}

void print_environment() {
    printf("\n");
    extern char **environ;
    char **env = environ;
    
    printf("Environment variables:\n");
    while (*env != NULL) {
        printf("%s\n", *env);
        env++;
    }
}

void set_environment_variable(const char *name, const char *value) {
    printf("\n");
    if (setenv(name, value, 1) == -1) {
        perror("setenv");
        return;
    }
    printf("Environment variable %s set to %s\n", name, value);
}


int parse_name_value(const char *input, char **name, char **value) {
    char *equals = strchr(input, '=');
    if (equals == NULL) {
        perror("Invalid format for -V. Use: name=value\n");
        return -1;
    }
    
    // Разделяем строку на имя и значение
    size_t name_len = equals - input;
    *name = malloc(name_len + 1);
    *value = strdup(equals + 1);
    
    if (*name == NULL || *value == NULL) {
        perror("malloc/strdup");
        free(*name);
        *name = NULL;
        free(*value);
        *value = NULL;
        return -1;
    }
    
    strncpy(*name, input, name_len);
    (*name)[name_len] = '\0';
    
    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    
    // Обрабатываем опции справа налево
    while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        switch (opt) {
            case 'i':
                print_user_group_ids();
                break;
            case 's':
                become_group_leader();
                break;
            case 'p':
                print_process_ids();
                break;
            case 'u':
                print_ulimit();
                break;
            case 'U':
                change_ulimit(optarg);
                break;
            case 'c':
                print_core_size();
                break;
            case 'C':
                change_core_size(optarg);
                break;
            case 'd':
                print_current_directory();
                break;
            case 'v':
                print_environment();
                break;
            case 'V': {
                char *name, *value;
                if (parse_name_value(optarg, &name, &value) == -1) {
                    break;
                }
                
                set_environment_variable(name, value);
                
                free(name);
                free(value);
                break;
            }
            case '?':
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    return EXIT_SUCCESS;
}