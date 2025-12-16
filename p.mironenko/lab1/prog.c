/* -*- coding: koi8-r -*-
 * Программа для обработки опций процесса с использованием getopt(3C)
 * Кодировка: KOI8-R
 * Компиляция: gcc -o prog proga.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    char options[] = "ispud:c:vV:U:C:"; /* создание строки допустимых опций */
    int c, invalid = 0; //объявление флагов опций

    printf("argc = %d\n", argc); //кол-во аргументов командной строки

    if (argc == 1) {
        printf("Нет аргументов\n");
    }

    while ((c = getopt(argc, argv, options)) != EOF) {
        switch (c) {
            case 'i': // Вывод реальных и эффективных ID пользователя и группы
                printf("Реальный UID: %d\n", getuid()); //id пользователя
                printf("Эффективный UID: %d\n", geteuid());
                printf("Реальный GID: %d\n", getgid()); //id групп
                printf("Эффективный GID: %d\n", getegid());
                break;
                //реальный - тот, кто запустил процесс, эффективный определяет права доступа процесса к системным ресурсам 
                //и может меняться во время выполнения

            case 's': // Установка процесса как лидера группы
                if (setpgid(0, 0) == -1) {
                    perror("Ошибка setpgid");
                    invalid++; 
                } else {
                    printf("Процесс стал лидером группы\n");
                }
                break;

            case 'p': // Вывод PID, PPID и PGID (идентификаторы процесса, процесса-родителя и группы процессов)
                printf("PID: %d\n", getpid());
                printf("PPID: %d\n", getppid());
                printf("PGID: %d\n", getpgrp());
                break;

            case 'u': // Вывод текущего ulimit
                {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_STACK, &rl) == -1) {
                        perror("Ошибка getrlimit");
                        invalid++;
                    } else {
                        printf("Текущий ulimit: %ld байт\n", (long)rl.rlim_cur);
                    }
                }
                break;
                // ulimit - ограничение на использование ресурсов процессом
                // это файлы, которые создаются опер. системой при аварийном завершении программы. Они содержат "снимок" памяти процесса на момент сбоя, что полезно для отладки

            case 'U': // Изменение ulimit
                {
                    struct rlimit rl;
                    char *endptr;
                    errno = 0;
                    long new_limit = strtol(optarg, &endptr, 10);
                    if (*endptr != '\0' || errno != 0 || new_limit < 0) {
                        fprintf(stderr, "Ошибка: неверное значение ulimit '%s'\n", optarg);
                        invalid++;
                        break;
                    }
                    rl.rlim_cur = new_limit;
                    rl.rlim_max = new_limit;
                    if (setrlimit(RLIMIT_STACK, &rl) == -1) {
                        perror("Ошибка setrlimit");
                        invalid++;
                    } else {
                        printf("Новый ulimit установлен: %ld байт\n", new_limit);
                    }
                }
                break;

            case 'c': // Вывод размера core-файла
                {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("Ошибка getrlimit для core");
                        invalid++;
                    } else {
                        printf("Размер core-файла: %ld байт\n", (long)rl.rlim_cur);
                    }
                }
                break;
                // core-файл - это файлы, которые создаются операционной системой при аварийном завершении программы
                // Они содержат "снимок" памяти процесса на момент сбоя, что полезно для отладки

            case 'C': // Изменение размера core-файла
                {
                    struct rlimit rl;
                    char *endptr;
                    errno = 0;
                    long new_limit = strtol(optarg, &endptr, 10);
                    if (*endptr != '\0' || errno != 0 || new_limit < 0) {
                        fprintf(stderr, "Ошибка: неверное значение размера core '%s'\n", optarg);
                        invalid++;
                        break;
                    }
                    rl.rlim_cur = new_limit;
                    rl.rlim_max = new_limit;
                    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("Ошибка setrlimit для core");
                        invalid++;
                    } else {
                        printf("Новый размер core-файла установлен: %ld байт\n", new_limit);
                    }
                }
                break;

            case 'd': // Вывод текущей рабочей директории
                {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        printf("Текущая рабочая директория: %s\n", cwd);
                    } else {
                        perror("Ошибка getcwd");
                        invalid++;
                    }
                }
                break;

            case 'v': // Вывод переменных среды
                {
                    extern char **environ;
                    for (char **env = environ; *env != NULL; env++) {
                        printf("%s\n", *env);
                    }
                }
                break;

            case 'V': // Вносит новую переменную в среду или изменяет значение существующей переменной
                if (putenv(optarg) == -1) {
                    perror("Ошибка putenv");
                    invalid++;
                } else {
                    printf("Переменная среды установлена: %s\n", optarg);
                }
                break;

            case '?': // Обработка неизвестной опции
                printf("invalid option is %c\n", optopt);
                invalid++;
                break;
        }
    }

    printf("invalid = %d\n", invalid);
    printf("optind = %d\n", optind);
    if (optind < argc) {
        printf("next parameter = %s\n", argv[optind]);
    }

    return invalid ? EXIT_FAILURE : EXIT_SUCCESS;
}