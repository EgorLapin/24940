/* -*- coding: koi8-r -*-
 * myls.c — простая реализация команды ls -l
 * Лабораторная работа №18
 * Полностью рабочая версия с подробными комментариями
 * Работает на macOS и Linux
 * Компиляция: cc -o myls proga18.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>               /* Для PATH_MAX на macOS и Linux */
#ifdef __APPLE__
    #include <sys/syslimits.h>   /* Дополнительная страховка для macOS */
#endif

/* ==============================================================
   Функция: выводит информацию об одном файле в формате ls -l
   path — полный путь к файлу (нужен для lstat)
   name — имя файла для отображения (может быть относительным)
   ============================================================== */
void print_file_info(const char *path, const char *name) {
    struct stat st;

    /* Получаем метаданные файла. lstat — чтобы не переходить по символическим ссылкам */
    if (lstat(path, &st) == -1) {
        perror("lstat");      /* Выводим ошибку, если файл недоступен */
        return;
    }

    /* 1. Определяем тип файла: каталог, ссылка, обычный файл и т.д. */
    char type;
    if (S_ISDIR(st.st_mode))  type = 'd';   /* каталог */
    else if (S_ISLNK(st.st_mode)) type = 'l';   /* символическая ссылка */
    else if (S_ISREG(st.st_mode)) type = '-';   /* обычный файл */
    else if (S_ISCHR(st.st_mode)) type = 'c';   /* символьное устройство */
    else if (S_ISBLK(st.st_mode)) type = 'b';   /* блочное устройство */
    else if (S_ISFIFO(st.st_mode)) type = 'p';  /* FIFO (именованный канал) */
    else if (S_ISSOCK(st.st_mode)) type = 's';  /* сокет */
    else type = '?';                            /* неизвестный тип */

    /* 2. Формируем строку прав доступа (rwxr-xr-x) */
    char perms[11];
    perms[0] = type;
    perms[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    perms[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';

    /* Учитываем специальные биты: setuid, setgid, sticky */
    if (st.st_mode & S_ISUID) perms[3] = (perms[3] == 'x') ? 's' : 'S';
    if (st.st_mode & S_ISGID) perms[6] = (perms[6] == 'x') ? 's' : 'S';
    if (st.st_mode & S_ISVTX) perms[9] = (perms[9] == 'x') ? 't' : 'T';

    /* 3. Получаем имена владельца и группы */
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    const char *owner = pw ? pw->pw_name : "unknown";
    const char *group = gr ? gr->gr_name : "unknown";

    /* 4. Форматируем дату и время последней модификации */
    char timebuf[64];
    struct tm *tm = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", tm);

    /* 5. Выводим строку в нужном формате */
    printf("%c %s %4ld %-8s %-8s %8lld %s %s\n",
           type,                    /* T — тип файла */
           perms + 1,               /* права без первого символа (rwxr-xr-x) */
           (long)st.st_nlink,       /* количество жёстких ссылок */
           owner,                   /* владелец */
           group,                   /* группа */
           (long long)st.st_size,   /* ← РАЗМЕР ФАЙЛА В БАЙТАХ! */
           timebuf,                 /* дата и время */
           name);                   /* имя файла */
}

/* ==============================================================
   Функция: выводит содержимое каталога (кроме . и ..)
   ============================================================== */
void list_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Пропускаем записи "." и ".." */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        /* Формируем полный путь к элементу каталога */
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        /* Выводим информацию о файле */
        print_file_info(fullpath, entry->d_name);
    }
    closedir(dir);
}

/* ==============================================================
   Функция: обрабатывает один аргумент командной строки
   (файл или каталог)
   ============================================================== */
void process_path(const char *path) {
    struct stat st;

    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        /* Если это каталог — выводим заголовок и содержимое */
        printf("\nСодержимое каталога: %s\n", path);
        printf("T Права      Связи Владелец  Группа   Размер Дата         Имя\n");
        printf("------------------------------------------------------------------------\n");

        /* Сначала выводим сам каталог (как точку) */
        print_file_info(path, path);

        /* Затем — его содержимое */
        list_directory(path);
    } else {
        /* Если это обычный файл — просто выводим информацию */
        printf("T Права      Связи Владелец  Группа   Размер Дата         Имя\n");
        printf("------------------------------------------------------------------------\n");
        print_file_info(path, path);
    }
}

/* ==============================================================
   Главная функция
   ============================================================== */
int main(int argc, char *argv[]) {
    /* Если программа запущена без аргументов — показываем текущий каталог */
    if (argc == 1) {
        printf("T Права      Связи Владелец  Группа   Размер Дата         Имя\n");
        printf("------------------------------------------------------------------------\n");
        process_path(".");
        return 0;
    }

    /* Обрабатываем все переданные аргументы */
    for (int i = 1; i < argc; i++) {
        process_path(argv[i]);
    }

    return 0;
}