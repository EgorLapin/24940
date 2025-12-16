#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <utility>
#include <limits.h>
#include <errno.h>

extern char **environ;

int main(int argc, char **argv) {
    std::vector<std::pair<int, std::string>> options;
    int opt;
    opterr = 0; // Отключаем стандартные сообщения об ошибках getopt


    // Парсинг опций
    while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if (opt == '?') {
            fprintf(stderr, "Недопустимая опция: -%c\n", optopt);
        } else {
            options.push_back({opt, optarg ? optarg : ""});
        }   
    }


    // Обработка опций в обратном порядке (справа налево)
    for (auto it = options.rbegin(); it != options.rend(); ++it) {
        int option = it->first;
        const std::string& arg = it->second;

        switch (option) {
            case 'i':
                printf("Реальный UID: %u, Эффективный UID: %u\n", getuid(), geteuid());
                printf("Реальный GID: %u, Эффективный GID: %u\n", getgid(), getegid());
                break;

            case 's':
                if (setpgid(0, 0) == -1) {
                    fprintf(stderr, "Ошибка setpgid: %s\n", strerror(errno));
                }
                break;

            case 'p':
                printf("PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgrp());
                break;

            case 'u': {
                struct rlimit rlim;
                if (getrlimit(RLIMIT_FSIZE, &rlim) == -1) {
                    fprintf(stderr, "Ошибка getrlimit (размер файла): %s\n", strerror(errno));
                } else {
                    if (rlim.rlim_cur == RLIM_INFINITY) {
                        printf("ulimit (размер файла): неограничен\n");
                    } else {
                        printf("ulimit (размер файла): %llu байт\n", (unsigned long long)rlim.rlim_cur);
                    }
                }
                break;
            }

            case 'U': {
                char *end;
                errno = 0;
                long val = strtol(arg.c_str(), &end, 10);
                if (errno != 0 || *end != '\0' || val < 0) {
                    fprintf(stderr, "Недопустимое значение ulimit: %s\n", arg.c_str());
                } else {
                    struct rlimit rlim;
                    rlim.rlim_cur = (rlim_t)val;
                    rlim.rlim_max = (rlim_t)val;
                    if (setrlimit(RLIMIT_FSIZE, &rlim) == -1) {
                        fprintf(stderr, "Ошибка setrlimit (размер файла): %s\n", strerror(errno));
                    }
                }
                break;
            }

            case 'c': {
                struct rlimit rlim;
                if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
                    fprintf(stderr, "Ошибка getrlimit (core-файл): %s\n", strerror(errno));
                } else {
                    if (rlim.rlim_cur == RLIM_INFINITY) {
                        printf("Размер core-файла: неограничен\n");
                    } else {
                        printf("Размер core-файла: %llu байт\n", (unsigned long long)rlim.rlim_cur);
                    }
                }
                break;
            }

            case 'C': {
                char *end;
                errno = 0;
                long val = strtol(arg.c_str(), &end, 10);
                if (errno != 0 || *end != '\0' || val < 0) {
                    fprintf(stderr, "Недопустимое значение размера core-файла: %s\n", arg.c_str());
                } else {
                    struct rlimit rlim;
                    rlim.rlim_cur = (rlim_t)val;
                    rlim.rlim_max = (rlim_t)val;
                    if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
                        fprintf(stderr, "Ошибка setrlimit (core-файл): %s\n", strerror(errno));
                    }
                }
                break;
            }

            case 'd': {
                char buf[PATH_MAX];
                if (getcwd(buf, sizeof(buf)) != NULL) {
                    printf("Текущая директория: %s\n", buf);
                } else {
                    fprintf(stderr, "Ошибка getcwd: %s\n", strerror(errno));
                }
                break;
            }

            case 'v':
                for (char **env = environ; *env != NULL; ++env) {
                    printf("%s\n", *env);
                }
                break;

            case 'V': {
                // Проверка формата name=value
                if (arg.find('=') == std::string::npos) {
                    fprintf(stderr, "Недопустимый формат переменной окружения: %s (ожидается name=value)\n", arg.c_str());
                } else if (putenv(const_cast<char *>(arg.c_str())) != 0) {
                    fprintf(stderr, "Ошибка putenv: %s\n", strerror(errno));
                }
                break;
            }
        }
    }

    // Обработка неопционных аргументов (если есть)
    if (optind < argc) {
        printf("Неопционные аргументы:");
        while (optind < argc) {
            printf(" %s", argv[optind++]);
        }
        printf("\n");
    }

    return 0;
}