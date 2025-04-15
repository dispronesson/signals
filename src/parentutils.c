#include "parentutils.h"

//Переменные для работы с процессами
static volatile sig_atomic_t counter;
static volatile Process processes[100];
extern char* child_path;
sigset_t set, oldset;

//Структура sigaction для работы с сигналом SIGALRM
static struct sigaction sa_alrm = { .sa_flags = SA_RESTART };

//Функция проверки корректности ввода
bool validate(char* str) {
    return (str[0] == 's' || str[0] == 'g' || str[0] == 'p')
        && (str[1] >= '0' && str[1] <= '9')
        && (str[2] >= '0' && str[2] <= '9');
}

//Перевод индекса процесса из строки в число
int transform(char* str) {
    int index = 0;
    
    //Если оба символа в строке 0, возвращаем 0
    if (str[1] != '0' || str[2] != '0') {
        index += (str[1] - '0') * 10;
        index += str[2] - '0';
    }

    return index;
}

//Функция создания нового процесса
void create_process() {
    if (counter < 100) {
        //Блокируем все сигналы, за исключением SIGUSR2, во время создания нового процесса
        sigfillset(&set);
        sigdelset(&set, SIGUSR2);
        sigprocmask(SIG_SETMASK, &set, &oldset);

        //Буфер под имя нового процесса
        char buffer[5];
        snprintf(buffer, 5, "C_%02d", counter);
        char* argv[] = {buffer, NULL};

        //Создание нового процесса
        pid_t pid = fork();

        //Если не удалось
        if (pid == -1) {
            perror("fork");
            return;
        }
        else if (pid == 0) { //Если это дочерний процесс
            //Востанавливаем маску для дочернего процесса
            sigprocmask(SIG_SETMASK, &oldset, NULL);
            //Запускаем программу child по пути child_path
            execve(child_path, argv, NULL);
            //Это выполнится, если execve завершится ошибкой
            fprintf(stderr, "fatal error: CHILD_PATH is invalid\n");
            //Посылаем сигнал SIGUSR2 родителю для обработки данной ситуации
            kill(getppid(), SIGUSR2);
            //Завершаем дочерний процесс
            _exit(EXIT_FAILURE);
        }
        
        //При успешном создании процесса уведомляем пользователя
        printf("info: new process %s was created\n", buffer);
        processes[counter].pid = pid;
        processes[counter++].can_output = 0;

        //Востанавливаем маску для родителя
        sigprocmask(SIG_SETMASK, &oldset, NULL);
    }
    else {
        //Больше 100 процессов создать нельзя
        fprintf(stderr, "error: can't create more than 100 child processes\n");
    }
}

//Функция блокировки к stdout для всех дочерних процессов
void block_all_processes() {
    for (int i = 0; i < counter; i++) {
        processes[i].can_output = 0;
    }
}

//Функция разблокировки к stdout для всех дочерних процессов
void unblock_all_processes() {
    for (int i = 0; i < counter; i++) {
        processes[i].can_output = 1;
    }
}

//Функция завершения всех дочерних процессов
void terminate_all_processes() {
    for (int i = 0; i < counter; i++) {
        //Попытка отправки сигнала SIGTERM
        if (kill(processes[i].pid, SIGTERM) == -1) {
            perror("kill");
        }
        //Если удачна, то ждем завершения дочернего процесса
        else {
            waitpid(processes[i].pid, NULL, 0);
        }
    }
    //Обнуляем кол-во всех дочерних процессов
    counter = 0;
}

//Функция завершения последнего созданного процесса
void terminate_process() {
    if (counter > 0) {
        //Отправляем сигнал
        if (kill(processes[counter - 1].pid, SIGTERM) == -1) {
            perror("kill");
        }
        else {
            //Ждем завершения процесса
            waitpid(processes[--counter].pid, NULL, 0);
            printf("info: child process C_%02d has been terminated\n", counter);

            //Информация о кол-ве оставшихся
            if (counter != 0) {
                printf("info: %d processes are still active\n", counter);
            }
            else {
                printf("info: there are no more active child processes\n");
            }
        }
    }
    else {
        //Ошибка, если процессов нет
        fprintf(stderr, "error: there are no child processes\n");
    }
}

//Функция вывода информации о всех процессах
void print_info() {
    printf("%-7s %-7s %-7s\n", "name", "pid", "ppid");
    printf("%-7s %-7d %-7d\n", "P", getpid(), getppid());

    char buffer[6];
    pid_t pid = getpid();
    
    for (int i = 0; i < counter; i++) {
        snprintf(buffer, 6, "C_%02d", i);
        printf("%-7s %-7d %-7d\n", buffer, processes[i].pid, pid);
    }
}

//Функция обработки единичного символа
void handle_command(char* input) {
    switch (input[0]) {
        case '+':
            create_process();
            break;
        case '-':
            terminate_process();
            break;
        case 'l':
            print_info();
            break;
        case 'k':
            terminate_all_processes();
            printf("info: all child processes are terminated\n");
            break;
        case 's':
            block_all_processes();
            printf("info: stats output is blocked\n");
            //Ставим SIGALRM в игнор, чтобы не мешал после команды p<num>
            change_handler_alrm(SIG_IGN);
            break;
        case 'g':
            unblock_all_processes();
            printf("info: stats output is unblocked\n");
            change_handler_alrm(SIG_IGN);
            break;
        case 'q':
            terminate_all_processes();
            printf("info: all child processes are terminated\n");
            printf("exiting program...\n");
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "error: invalid operation\n");
            break;
    }
}

//Функция обработки команды для конкретного процесса
void handle_process_command(char* input) {
    //Провверка корректности ввода
    if (validate(input)) {
        //Получения индекса процесса
        int process_index = transform(input);

        //Если процесса с таким индексом нет..
        if (process_index >= counter) {
            fprintf(stderr, "error: child process C_%02d doesn't exist\n", process_index);
        }
        else {
            switch (input[0]) {
                case 's':
                    processes[process_index].can_output = 0;
                    printf("info: stats output for C_%02d is blocked\n", process_index);
                    break;
                case 'g':
                    processes[process_index].can_output = 1;
                    printf("info: stats output for C_%02d is unblocked\n", process_index);
                    break;
                case 'p':
                    block_all_processes();
                    //Запрашиваем информацию о текущем состоянии дочернего процесса
                    kill(processes[process_index].pid, SIGUSR1);
                    //Изменяем обработчик для SIGALRM, чтобы через 5 секунд разблокировал доступ к stdout
                    change_handler_alrm(handler_alarm);
                    alarm(5);
                    printf("info: stats output is blocked for 5 seconds\n");
                    break;
            }
        }
    }
    else {
        fprintf(stderr, "error: invalid operation\n");
    }
}

//Функция обработки команд
void interface() {
    char buffer[4];

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);

        //Если нажат просто enter...
        if (buffer[0] == '\n') continue;

        //Если один символ + '\n'...
        if (strlen(buffer) == 2) {
            handle_command(buffer);
        }
        else {
            handle_process_command(buffer);
        }

        //Очистка stdin от мусора по необходимости
        if (buffer[2] != '\0' && buffer[2] != '\n') while (getchar() != '\n');
    }
}

//Обработчик для SIGALRM
void handler_alarm(int signo) {
    unblock_all_processes();
}

//Обработчик для проверки у дочернего процесса доступа к stdout
void handler_permission(int signo, siginfo_t* info, void* context) {
    for (int i = 0; i < counter; i++) {
        if (info->si_pid == processes[i].pid) {
            //Если разрешено, отправляем соответствующий сигнал
            if (processes[i].can_output) {
                kill(processes[i].pid, SIGUSR1);
            }
            else {
                kill(processes[i].pid, SIGUSR2);
            }
            break;
        }
    }
}

//Обработчик для случая, если execve завершится ошибкой
void handler_child_err(int signo) {
    wait(NULL);
    exit(EXIT_FAILURE);
}

//Функция инициализации обработчиков сигналов
void init_handlers() {
    struct sigaction sa;
    //Установка необходимых флагов
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    //Установка пустой маски
    sigemptyset(&sa.sa_mask);
    //Привязка обработчика для SIGUSR1
    sa.sa_sigaction = handler_permission;
    sigaction(SIGUSR1, &sa, NULL);

    sa.sa_flags = 0;
    //Привязка обработчика для SIGUSR2
    sa.sa_handler = handler_child_err;
    sigaction(SIGUSR2, &sa, NULL);
}

void change_handler_alrm(void (*handler)(int signo)) {
    sigemptyset(&sa_alrm.sa_mask);
    sa_alrm.sa_handler = handler;
    sigaction(SIGALRM, &sa_alrm, NULL);
}