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

Baal_define(test, 1, 4, 20)

int main(void) {
    char* hi = Baal_allocMany(test, 5);
    strncpy(hi, "Hi!", 4);

    printf("%s\n", hi);
    
    Baal_print(test);

    Baal_reallocBlocks(test, hi, 10);

    // Baal_memorySnapshot(test);

    return 0;
}
