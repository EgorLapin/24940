#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

extern char** environ;

void print_user_ids()
{
    printf("The real user ID: %d.\n", getuid());
    printf("The effective user ID: %d.\n", geteuid());
}

void print_group_ids()
{
    printf("The real group ID: %d.\n", getgid());
    printf("The effective group ID: %d.\n", getegid());
}

void print_proc_id()
{
    pid_t proc_id = getpid();
    printf("The process ID: %d.\n", proc_id);
}

void print_parent_proc_id()
{
    pid_t parent_id = getppid();
    printf("The parent process ID: %d.\n", parent_id);
}

void print_group_proc_id()
{
    pid_t group_id = getpgid(0);
    printf("The group process ID: %d.\n", group_id);
}

void print_u_limit()
{
    long limit = ulimit(UL_GETFSIZE);
    if (limit == -1 && errno != 0)
    {
        perror("Error while getting ulimit");
    }
    else
    {
        printf("ulimit is %ld.\n", limit);
    }
}

void set_u_limit(long new_u_limit)
{
    long error = ulimit(UL_SETFSIZE, new_u_limit);
    if (error == -1 && errno != 0)
    {
        perror("Error while setting ulimit");
    }
    else
    {
        printf("Success! New ulimit is %ld.\n", new_u_limit);
    }
}

void print_core_limit()
{
    struct rlimit rlim;
    int error = getrlimit(RLIMIT_CORE, &rlim);
    if (error == -1 && errno != 0)
    {
        perror("Error while getting RLIMIT_CORE");
    }
    else
    {
        printf("Cur core limit is %ld.\n", rlim.rlim_cur);
        printf("Max core limit is %ld.\n", rlim.rlim_max);
    }
}

void set_core_limit(long new_core_size)
{
    struct rlimit rlim;
    rlim.rlim_cur = new_core_size;
    rlim.rlim_max = new_core_size;

    int error = setrlimit(RLIMIT_CORE, &rlim);
    if (error == -1)
    {
        perror("Error while setting RLIMIT_CORE");
    }
    else
    {
        printf("Success! Core limit is %ld.\n", new_core_size);
    }
}

void print_directory()
{
    char buffer[PATH_MAX + 1] = {0};
    char* error = getcwd(buffer, PATH_MAX);

    if (error == NULL)
    {
        perror("Error getting current directory");
    }
    else
    {
        buffer[PATH_MAX] = '\0';
        printf("Current work directory is '%s'.\n", buffer);
    }
}

void print_env_vars()
{
    printf("Environment variables:\n");
    for (int i = 0; environ[i] != NULL; i++)
    {
        printf("\t%s\n", environ[i]);
    }
}

void new_or_change_env_var(char* str)
{
    int error = putenv(str);
    if (error == 0)
    {
        printf("putenv() was successful.\n");
    }
    else
    {
        perror("Error when creating or changing environment variable");
    }
}

int main(int argc, char* argv[])
{
    char options[] = "ispuU:cC:dvV:";
    int c = 0;
    int arg_cnt = 0;

    long new_u_limit = 0;
    long new_core_size = 0;
    char* endptr;

    if (argc <= 1)
    {
        printf("No arguments.\n");
        return 0;
    }
    while ((c = getopt(argc, argv, options)) != EOF)
    {
        arg_cnt++;
        switch (c)
        {
            case 'i':
                print_user_ids();
                print_group_ids();
                break;
            case 's':
                setpgid(0, 0);
                break;
            case 'p':
                print_proc_id();
                print_parent_proc_id();
                print_group_proc_id();
                break;
            case 'u':
                print_u_limit();
                break;
            case 'U':
                new_u_limit = strtol(optarg, &endptr, 10);
                if (endptr != optarg)
                {
                    set_u_limit(new_u_limit);
                }
                else
                {
                    printf("Argument '%s' isn't number.\n", optarg);
                }
                break;
            case 'c':
                print_core_limit();
                break;
            case 'C':
                new_core_size = strtol(optarg, &endptr, 10);
                if (endptr != optarg)
                {
                    set_core_limit(new_core_size);
                }
                else
                {
                    printf("Argument '%s' isn't number.\n", optarg);
                }
                break;
            case 'd':
                print_directory();
                break;
            case 'v':
                print_env_vars();
                break;
            case 'V':
                new_or_change_env_var(optarg);
                break;
            case '?':
                printf("invalid option is '%c'.\n", optopt);
                break;
        }
    }

    if (arg_cnt == 0)
    {
        printf("No options.\n");
    }

    return 0;
}
