#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>
#include <locale.h>

void print_permissions(mode_t m) {
    putchar(S_ISDIR(m) ? 'd' : S_ISREG(m) ? '-' : S_ISLNK(m) ? 'l' :
            S_ISFIFO(m) ? 'p' : S_ISSOCK(m) ? 's' : S_ISCHR(m) ? 'c' :
            S_ISBLK(m) ? 'b' : '?');
    putchar(m & S_IRUSR ? 'r' : '-');
    putchar(m & S_IWUSR ? 'w' : '-');
    putchar(m & S_IXUSR ? 'x' : '-');
    putchar(m & S_IRGRP ? 'r' : '-');
    putchar(m & S_IWGRP ? 'w' : '-');
    putchar(m & S_IXGRP ? 'x' : '-');
    putchar(m & S_IROTH ? 'r' : '-');
    putchar(m & S_IWOTH ? 'w' : '-');
    putchar(m & S_IXOTH ? 'x' : '-');
}

void print_file_info(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    print_permissions(st.st_mode);
    printf(" %ld ", st.st_nlink);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    const char *user = pw ? pw->pw_name : "unknown";
    const char *group = gr ? gr->gr_name : "unknown";
    printf("%s %s ", user, group);

    if (S_ISREG(st.st_mode))
        printf("%lld ", (long long)st.st_size);
    else if (S_ISDIR(st.st_mode))
        printf("%ld ", st.st_blocks * 512);
    else
        printf("    ");

    struct tm *tm = localtime(&st.st_mtime);
    char timebuf[64];
    time_t now = time(NULL);
    time_t six_months_ago = now - 183L * 24 * 60 * 60;

    if (st.st_mtime > six_months_ago)
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
    else
        strftime(timebuf, sizeof(timebuf), "%b %e  %Y", tm);

    printf("%s %s\n", timebuf, basename((char*)path));
}

int main(int argc, char *argv[]) {
    setlocale(LC_TIME, "");

    if (argc == 1) {
        print_file_info(".");
        return 0;
    }

    int i;
    for (i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return 0;
}