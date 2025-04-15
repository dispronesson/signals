#ifndef PARENTUTILS_H
#define PARENTUTILS_H

//Подключение POSIX-совместимых функций и расширения X/Open
#define _XOPEN_SOURCE 700

//Подключение библиотек
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

//Структура для хранения информации о дочернем процессе
typedef struct {
    pid_t pid;
    sig_atomic_t can_output;
} Process;

//Объявление функций
bool validate(char* str);

int transform(char* str);

void create_process();

void block_all_processes();

void unblock_all_processes();

void terminate_all_processes();

void terminate_process();

void print_info();

void handle_command(char* input);

void handle_process_command(char* input);

void interface();

void handler_alarm(int signo);

void handler_permission(int signo, siginfo_t* info, void* context);

void handler_child_err(int signo);

void init_handlers();

void change_handler_alrm(void (*handler)(int signo));

#endif //PARENTUTILS_H