#include "Baal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum PropertyType {
    PropertyTypeNumber,
    PropertyTypeBoolean,
} PropertyType;

typedef struct Property {
    PropertyType type;
    union {
        float number;
        int boolean;
    } value;
} Property;

Baal_define(test, 1, 1, 10)

int main(void) {
    void* a = Baal_allocMany(test, 2);
    char* b = Baal_allocMany(test, 2);
    void* c = Baal_allocMany(test, 6);

    *b = 32;

    Baal_free(test, a);

    Baal_reallocBlocks(test, b, 3);

    Baal_print(test);

    // Baal_memorySnapshot(test);

    return 0;
}
