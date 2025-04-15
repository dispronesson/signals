#ifndef CHILDTUTILS_H
#define CHILDTUTILS_H

//Подключение POSIX-совместимых функций и расширения X/Open
#define _XOPEN_SOURCE 700

//Подключение библиотек
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

typedef struct {
    int first;
    int second;
} Words;

//Объявление функций
void handler_print(int signo);

void handler_stat(int signo);

void handler_dummy(int signo);

void init_handlers();

#endif //CHILDTUTILS_H