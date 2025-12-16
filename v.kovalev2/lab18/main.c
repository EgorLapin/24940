#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file1> [file2] ...\n", argv[0]);
        printf("No arguments provided. Exiting.\n");
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        char *filepath = argv[i];

        char resolved_path[PATH_MAX];
        char *path_to_print = filepath;

        struct stat st;
        if (stat(path_to_print, &st) != 0)
        {
            printf("Failed to stat %s. Skipping.\n", path_to_print);
            perror("stat");
            continue;
        }

        char type = '?';
        if (S_ISDIR(st.st_mode))
        {
            type = 'd';
        }
        else if (S_ISREG(st.st_mode))
        {
            type = '-';
        }

        char perm[10];
        perm[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
        perm[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
        perm[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
        perm[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
        perm[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
        perm[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
        perm[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
        perm[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
        perm[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
        perm[9] = '\0';

        struct passwd *pwd = getpwuid(st.st_uid);
        struct group *grp = getgrgid(st.st_gid);
        char *owner = pwd ? pwd->pw_name : (char *)"unknown";
        char *group = grp ? grp->gr_name : (char *)"unknown";

        char size_str[20];
        snprintf(size_str, sizeof(size_str), "%ld", (long)st.st_size);

        char time_str[20];
        struct tm *tm = localtime(&st.st_mtime);
        strftime(time_str, sizeof(time_str), "%b %e %H:%M", tm);

        printf("%c%s %ld %s %s %ld %s %s\n", type, perm, (long)st.st_nlink, owner, group, (long)st.st_size, time_str, path_to_print);

    }

    return 0;
}