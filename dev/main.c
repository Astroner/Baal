#include "Baal.h"

Baal_define(test, 1, 1, 10)

int main(void) {
    char* a = Baal_allocMany(test, 3);

    *a = 32;
    Baal_print(test);

    return 0;
}
