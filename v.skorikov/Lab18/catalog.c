#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file1> [file2] ...\n", argv[0]);
        printf("Log: No arguments provided. Exiting.\n");
        return 1;
    }

    printf("Log: Starting processing of %d files.\n", argc - 1);

    for (int i = 1; i < argc; i++) {
        char *filepath = argv[i];
        char *filename = basename(filepath);  // Get only the filename


        struct stat st;
        if (stat(filepath, &st) != 0) {
            printf("Log: Failed to stat %s. Skipping.\n", filepath);
            perror("stat");
            continue;
        }

        // Type
        char type = '?';
        if (S_ISDIR(st.st_mode)) type = 'd';
        else if (S_ISREG(st.st_mode)) type = '-';

        // Permissions
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

        // Owner and group
        struct passwd *pwd = getpwuid(st.st_uid);
        struct group *grp = getgrgid(st.st_gid);
        char *owner = pwd ? pwd->pw_name : "unknown";
        char *group = grp ? grp->gr_name : "unknown";

        // Size (only for regular files)
        char size_str[20] = "";
        snprintf(size_str, sizeof(size_str), "%ld", (long)st.st_size);

        // Modification time
        char time_str[20];
        struct tm *tm = localtime(&st.st_mtime);
        strftime(time_str, sizeof(time_str), "%d %H:%M", tm);

        // Print in table format with fixed widths
        printf("%c%s %ld %-8s %-8s %-5s %s %s\n",
               type, perm, (long)st.st_nlink, owner, group, size_str, time_str, filename);
    }

    printf("Log: All files processed.\n");
    return 0;
}