/* -*- coding: koi8-r -*-
 * beeper.c — Пищалка (лабораторная работа №21 по сигналам)
 * При Ctrl+C (SIGINT) — издаёт звуковой сигнал
 * При Ctrl+\ (SIGQUIT) — выводит статистику и завершает работу
 * Компиляция: cc -o beeper beeper.c
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/* Глобальный счётчик звуковых сигналов */
volatile sig_atomic_t beep_count = 0;

/* Обработчик сигнала SIGINT (Ctrl+C) */
void sigint_handler(int sig) {
    /* \a — это escape-последовательность для звукового сигнала alert (bell) */
    printf("\a");
    fflush(stdout);        // Чтобы звук сработал сразу
    beep_count++;         // Увеличиваем счётчик
}

/* Обработчик сигнала SIGQUIT (Ctrl+\) */
void sigquit_handler(int sig) {
    printf("\n\nВсего прозвучало сигналов: %d\n", beep_count);
    printf("Программа завершена по SIGQUIT (Ctrl+\\)\n");
    _exit(0);             
}


int main() {
    /* Устанавливаем обработчики сигналов */
    signal(SIGINT, sigint_handler);   /* Ctrl+C → звук */
    signal(SIGQUIT, sigquit_handler); /* Ctrl+\ → выход со статистикой */

    printf("Пищалка запущена!\n");
    printf("Нажмите Ctrl+C — будет звук\n");
    printf("Нажмите Ctrl+\\ — программа завершится с подсчётом сигналов\n");
    printf("Ожидаю ввода...\n\n");

    /* Бесконечный цикл — просто ждём сигналы */
    while (1) {
        pause();   /* Ждём любой сигнал. Экономит CPU */
    }

    return 0;
}