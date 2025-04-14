#ifndef PARENTUTILS_H
#define PARENTUTILS_H

#define _XOPEN_SOURCE 700

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    pid_t pid;
    int can_output;
} Process;

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

void init_handlers();

void change_handler_alrm(void (*handler)(int signo));

#endif //PARENTUTILS_H