#include "parentutils.h"

char* child_path;

int main(int argc, char* argv[], char* envp[]) {
    child_path = getenv("CHILD_PATH");
    if (!child_path) {
        fprintf(stderr, "error: CHILD_PATH is not set\n");
        return 1;
    }

    init_handlers();

    interface();

    return 0;
}