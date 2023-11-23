#include "Baal.h"

#include <assert.h>
#include <stdio.h>

Baal_defineStatic(numbers, sizeof(int), 10);

int main(void) {
    Baal_alloc(numbers);

    Baal_print(numbers);
    // Blocks:
    // 0 : 0x103174060 : busy
    // 1 : 0x103174064 : free
    // 2 : 0x103174068 : free
    // 3 : 0x10317406c : free
    // 4 : 0x103174070 : free
    // 5 : 0x103174074 : free
    // 6 : 0x103174078 : free
    // 7 : 0x10317407c : free
    // 8 : 0x103174080 : free
    // 9 : 0x103174084 : free
    // Free stack size: 0

    Baal_clear(numbers);

    return 0;
}