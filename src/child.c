#include "childutils.h"

Words memory;
volatile sig_atomic_t counter;
volatile sig_atomic_t keep_going;
const char* name;

int main(int argc, char* argv[]) {
    name = argv[0];

    init_handlers();

    Words zeros = { 0, 0 }, ones = { 1, 1 };

    struct itimerval timer;

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 30000;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    while (1) {
        setitimer(ITIMER_REAL, &timer, NULL);
        keep_going = 1;

        while (keep_going) {
            memory = zeros;
            memory = ones;
        }

        if (counter >= 100) {
            kill(getppid(), SIGUSR1);
            pause();
        }
    }
}