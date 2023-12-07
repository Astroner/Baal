#include "tests-new.h"

#include "tests.h"

#include "../Baal.h"

DESCRIBE(allocate) {
    IT("allocates memory until it is full") {
        Baal_define(test, 1, 1, 10);

        EXPECT(Baal_allocMany(test, 2)) NOT TO_BE(NULL);
        EXPECT(Baal_allocMany(test, 2)) NOT TO_BE(NULL);
        EXPECT(Baal_allocMany(test, 3)) NOT TO_BE(NULL);
        EXPECT(Baal_allocMany(test, 10)) TO_BE(NULL);
    }
}