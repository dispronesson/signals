#include "childutils.h"

//Переменные для работы с дочерним процессом
Words memory;
volatile sig_atomic_t counter;
volatile sig_atomic_t keep_going;
const char* name;

int main(int argc, char* argv[]) {
    //Установка имени процесса для глобльной переменной
    name = argv[0];

    //Инициализация обработчиков сигналов
    init_handlers();

    //Структуры для использования в цикле
    Words zeros = { 0, 0 }, ones = { 1, 1 };

    //Переменная для работы с таймером
    struct itimerval timer;

    //Устанавливаем интервал в 30 мс
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 30000;

    //Без автоматического повторения
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    //Бесконечный цикл
    while (1) {
        //Запуск таймера
        setitimer(ITIMER_REAL, &timer, NULL);
        keep_going = 1;

        //Обновление memory то нулями, то единицами
        while (keep_going) {
            memory = zeros;
            memory = ones;
        } //Как придет SIGALRM, keep_going станет 0

        //Проверка кол-ва пройденных итераций
        if (counter >= 100) {
            //Отправка сигнала для запроса у родителя доступа к stdout
            kill(getppid(), SIGUSR1);
            //Ждем ответа...
            pause();
        }
    }
}