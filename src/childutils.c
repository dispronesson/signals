#include "childutils.h"

//Переменные для работы с процессом
volatile sig_atomic_t stats[4];
extern volatile sig_atomic_t counter;
extern volatile sig_atomic_t keep_going;
extern const char* name;
extern Words memory;

//Функция вывода в stdout информации о состоянии процесса
void handler_print(int signo) {
    printf("name: %-7s pid: %-8d ppid: %-8d ", name, getpid(), getppid());
    printf("00: %-5d 01: %-5d 10: %-5d 11: %d\n", stats[0], stats[1], stats[2], stats[3]);
    //Обнуление данных
    counter = 0;
    for (size_t i = 0; i < sizeof(stats) / sizeof(sig_atomic_t); i++) {
        stats[i] = 0;
    }
}

//Функция для ведения статистика
void handler_stat(int signo) {
    if (memory.first == 0 && memory.second == 0) {
        stats[0]++;
    }
    else if (memory.first == 0 && memory.second == 1) {
        stats[1]++;
    }
    else if (memory.first == 1 && memory.second == 0) {
        stats[2]++;
    }
    else {
        stats[3]++;
    }
    counter++;
    keep_going = 0;
}

//Обработчик-заглушка, если доступ к stdout закрыт
void handler_dummy(int signo) {
    counter = 0;
    for (size_t i = 0; i < sizeof(stats) / sizeof(sig_atomic_t); i++) {
        stats[i] = 0;
    }
}

//Функция инициализации обработчиков сигналов
void init_handlers() {
    struct sigaction sa;
    sa.sa_flags = 0;
    //Привязка обработчика для SIGUSR1
    sa.sa_handler = handler_print;

    //Добавляем в маску сигнал SIGALRM
    sigset_t masks;
    sigemptyset(&masks);
    sigaddset(&masks, SIGALRM);
    sa.sa_mask = masks;

    sigaction(SIGUSR1, &sa, NULL);

    sigemptyset(&sa.sa_mask);
    //Привязка обработчика для SIGUSR2
    sa.sa_handler = handler_dummy;
    sigaction(SIGUSR2, &sa, NULL);

    //Привязка обработчика для SIGALRM
    sa.sa_handler = handler_stat;
    sigaction(SIGALRM, &sa, NULL);
}