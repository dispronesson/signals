#include "parentutils.h"

char* child_path; //Путь к дочерней программе

int main(int argc, char* argv[], char* envp[]) {
    child_path = getenv("CHILD_PATH"); 
    if (!child_path) { //Проверка установки переменной окружения
        fprintf(stderr, "error: CHILD_PATH is not set\n");
        return 1;
    }

    init_handlers(); //Инициализация обработчиков сигналов

    interface(); //Вход в цикл ожидания команды

    return 0;
}