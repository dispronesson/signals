#include "parentutils.h"

static volatile sig_atomic_t counter = 0;
static volatile Process processes[100];
extern char* child_path;

static struct sigaction sa_alrm = { .sa_flags = SA_RESTART };

bool validate(char* str) {
    return (str[0] == 's' || str[0] == 'g' || str[0] == 'p')
        && (str[1] >= '0' && str[1] <= '9')
        && (str[2] >= '0' && str[2] <= '9');
}

int transform(char* str) {
    int index = 0;
    
    if (str[1] != '0' || str[2] != '0') {
        index += (str[1] - '0') * 10;
        index += str[2] - '0';
    }

    return index;
}

void create_process() {
    if (counter < 100) {
        char buffer[5];
        snprintf(buffer, 5, "C_%02d", counter);
        char* argv[] = {buffer, NULL};

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
        }
        else if (pid == 0) {
            execve(child_path, argv, NULL);
            perror("CHILD_PATH");
            exit(EXIT_FAILURE);
        }
        
        printf("info: new process %s was created\n", buffer);
        processes[counter].pid = pid;
        processes[counter++].can_output = 0;
    }
    else {
        fprintf(stderr, "error: can't create more than 100 child processes\n");
    }
}

void block_all_processes() {
    for (int i = 0; i < counter; i++) {
        processes[i].can_output = 0;
    }
}

void unblock_all_processes() {
    for (int i = 0; i < counter; i++) {
        processes[i].can_output = 1;
    }
}

void terminate_all_processes() {
    for (int i = 0; i < counter; i++) {
        if (kill(processes[i].pid, SIGTERM) == -1) {
            perror("kill");
        }
        else {
            waitpid(processes[i].pid, NULL, 0);
        }
    }
    counter = 0;
}

void terminate_process() {
    if (counter > 0) {
        if (kill(processes[counter - 1].pid, SIGTERM) == -1) {
            perror("kill");
        }
        else {
            waitpid(processes[--counter].pid, NULL, 0);
            printf("info: child process C_%02d has been terminated\n", counter);

            if (counter != 0) {
                printf("info: %d processes are still active\n", counter);
            }
            else {
                printf("info: there are no more active child processes\n");
            }
        }
    }
    else {
        fprintf(stderr, "error: there are no child processes\n");
    }
}

void print_info() {
    printf("%-7.5s %-7.5s %-7.5s\n", "name", "pid", "ppid");
    printf("%-7.5s %-7.5d %-7.5d\n", "P", getpid(), getppid());

    char buffer[6];
    pid_t pid = getpid();
    
    for (int i = 0; i < counter; i++) {
        snprintf(buffer, 6, "C_%02d", i);
        printf("%-7.5s %-7.5d %-7.5d\n", buffer, processes[i].pid, pid);
    }
}

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

void handle_process_command(char* input) {
    if (validate(input)) {
        int process_index = transform(input);

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
                    kill(processes[process_index].pid, SIGUSR1);
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

void interface() {
    char buffer[4];

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);

        if (buffer[0] == '\n') continue;

        if (strlen(buffer) == 2) {
            handle_command(buffer);
        }
        else {
            handle_process_command(buffer);
        }

        if (buffer[2] != '\0' && buffer[2] != '\n') while (getchar() != '\n');
    }
}

void handler_alarm(int signo) {
    unblock_all_processes();
}

void handler_permission(int signo, siginfo_t* info, void* context) {
    for (int i = 0; i < counter; i++) {
        if (info->si_pid == processes[i].pid) {
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

void init_handlers() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler_permission;
    sigaction(SIGUSR1, &sa, NULL);
}

void change_handler_alrm(void (*handler)(int signo)) {
    sigemptyset(&sa_alrm.sa_mask);
    sa_alrm.sa_handler = handler;
    sigaction(SIGALRM, &sa_alrm, NULL);
}