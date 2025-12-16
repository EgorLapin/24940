#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){

    uid_t r_uid = getuid();
    uid_t e_uid = geteuid();

    printf("%d - Реальный UID, %d - Эффективный UID\n", r_uid, e_uid);

    printf("Попытка открыть файл №1 с %d ruid и %d euid:\n", r_uid, e_uid);
    FILE* new_file = fopen("text.txt", "r");
    if(new_file != NULL){
        printf("Файл был открыт\n");
        fclose(new_file);
    }else{
        perror("Ошибка открытия файла");
    }

    setuid(r_uid);


    // Достаем актуальные айди
    r_uid = getuid();
    e_uid = geteuid();

    printf("Попытка открыть файл №2 с %d ruid и %d euid:\n", r_uid, e_uid);
    new_file = fopen("text.txt", "r");
    if(new_file != NULL){
        printf("Файл был открыт\n");
        fclose(new_file);
    }else{
        perror("Ошибка открытия файла");
    }

}