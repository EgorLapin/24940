#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH_LEN 1024

char get_file_type(mode_t mode) {
    if (S_ISDIR(mode)) {
        return 'd';
    } else if (S_ISREG(mode)) {
        return '-';
    } else {
        return '?';
    }
}

void get_permissions(mode_t mode, char *perm_str) {
    perm_str[0] = (mode & S_IRUSR) ? 'r' : '-';
    perm_str[1] = (mode & S_IWUSR) ? 'w' : '-';
    perm_str[2] = (mode & S_IXUSR) ? 'x' : '-';
    
    perm_str[3] = (mode & S_IRGRP) ? 'r' : '-';
    perm_str[4] = (mode & S_IWGRP) ? 'w' : '-';
    perm_str[5] = (mode & S_IXGRP) ? 'x' : '-';
    
    perm_str[6] = (mode & S_IROTH) ? 'r' : '-';
    perm_str[7] = (mode & S_IWOTH) ? 'w' : '-';
    perm_str[8] = (mode & S_IXOTH) ? 'x' : '-';
    
    perm_str[9] = '\0';
}

const char* get_filename(const char* path) {
    const char* filename = strrchr(path, '/');
    return filename ? filename + 1 : path;
}

int process_file(const char* filepath) {
    struct stat file_stat;
    
    if (stat(filepath, &file_stat) != 0) {
        fprintf(stderr, "cataloger: cannot access '%s': %s\n", filepath, strerror(errno));
        return -1;
    }
    
    char file_type = get_file_type(file_stat.st_mode);
    
    char permissions[10];
    get_permissions(file_stat.st_mode, permissions);
    
    struct passwd *pw = getpwuid(file_stat.st_uid);
    const char* owner_name = pw ? pw->pw_name : "unknown";
    
    struct group *gr = getgrgid(file_stat.st_gid);
    const char* group_name = gr ? gr->gr_name : "unknown";
    
    struct tm *tm_info = localtime(&file_stat.st_mtime);
    char date_str[32];
    strftime(date_str, sizeof(date_str), "%b %d %H:%M", tm_info);
    
    const char* filename = get_filename(filepath);
    
    if (file_type == '-') {
        printf("%c%s %3ld %-8s %-8s %8ld %s %s\n",
               file_type,
               permissions,
               file_stat.st_nlink,
               owner_name,
               group_name,
               file_stat.st_size,
               date_str,
               filename);
    } else {
        printf("%c%s %3ld %-8s %-8s %8s %s %s\n",
               file_type,
               permissions,
               file_stat.st_nlink,
               owner_name,
               group_name,
               "",
               date_str,
               filename);
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        return process_file(".");
    }
    
    int error_count = 0;
    for (int i = 1; i < argc; i++) {
        if (process_file(argv[i]) != 0) {
            error_count++;
        }
    }
    
    return error_count > 0 ? 1 : 0;
}
