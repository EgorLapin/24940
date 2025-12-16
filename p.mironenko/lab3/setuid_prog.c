/* -*- coding: koi8-r -*-
 * Программа для работы с идентификаторами пользователя и файлом
 * Кодировка: KOI8-R
 * Компиляция: cc -o prog setuid_prog.c
 * Создать файл touch datafile          
chmod 600 datafile
 * Предоставить доступ для др. польз-ей chmod u+s prog
 * посмотреть права доступа ls -l prog
 * предоставить доступ к группе chmod g+x prog
    chgrp staff (наз. группы) prog
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    FILE *file;
    const char *filename = "datafile";

    //Печатаем реального и эффективного UID
    printf("Начало: Реальный UID: %d, Эффективный UID: %d\n", getuid(), geteuid());

    //Открываем файл и обрабатываем результат
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Ошибка при открытии файла");
    } else {
        printf("Файл %s успешно открыт\n", filename);
        fclose(file);
    }

    //Установка реального и эффективного UID
    if (setuid(geteuid()) == -1) {
        perror("Ошибка setuid");
        return EXIT_FAILURE;
    }
    printf("После setuid: Реальный UID: %d, Эффективный UID: %d\n", getuid(), geteuid());

    //Повторение шагов 1 и 2
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Ошибка при открытии файла");
    } else {
        printf("Файл %s успешно открыт\n", filename);
        fclose(file);
    }

    return EXIT_SUCCESS;
}