#include "Baal.h"

#include <assert.h>
#include <stdio.h>

int main(void) {

Baal_define(baal, 1, 20);
    char* str = Baal_allocMany(baal, 10);

    char* str2 = Baal_allocMany(baal, 10);

    Baal_free(baal, str);

    char* str3 = Baal_allocMany(baal, 3);

    Baal_print(baal);


    return 0;
}