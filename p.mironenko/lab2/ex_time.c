#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main() {
    time_t now;
    struct tm *sp;

    // Установка временной зоны на America/Los_Angeles
    putenv("TZ=America/Los_Angeles");
    tzset(); // Применяем настройки временной зоны

    (void) time(&now);

    // Вывод времени в формате ctime
    printf("%s", ctime(&now));

    // Получение локального времени
    sp = localtime(&now);

    // Если действует DST (PDT), корректируем время на -1 час для PST
    if (sp->tm_isdst > 0) {
        now -= 3600; // Вычитаем 1 час (3600 секунд)
        sp = localtime(&now); // Обновляем структуру tm
    }

    // Вывод в формате: месяц/день/год часы:минуты временная_зона
    printf("%d/%d/%04d %d:%02d %s\n",
           sp->tm_mon + 1, // Месяц (0-11, поэтому +1)
           sp->tm_mday,    // День
           sp->tm_year + 1900, // Год (лет с 1900, поэтому +1900)
           sp->tm_hour,    // Часы
           sp->tm_min,     // Минуты
           "PST"); // Фиксированно PST, как указано в задаче

    exit(0);
}