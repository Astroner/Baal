#include "tests-new.h"

#include "tests.h"

#include "../Baal.h"

DESCRIBE(deallocate) {
    IT("frees memory to be allocated again") {
        Baal_define(test, 1, 1, 9);

        char* a = Baal_allocMany(test, 3);
        char* b = Baal_allocMany(test, 3);
        Baal_allocMany(test, 3);

        Baal_free(test, a);

        char* d = Baal_allocMany(test, 2);
        EXPECT(d) NOT TO_BE(NULL);

        char* e = Baal_allocMany(test, 2);
        EXPECT(e) TO_BE(NULL);

        Baal_free(test, b);
        e = Baal_allocMany(test, 4);
        EXPECT(e) NOT TO_BE(NULL);
    }
    
    IT("clears memory") {
        Baal_define(test, 1, 1, 9);

        Baal_allocMany(test, 3);

        char* b = Baal_allocMany(test, 9);
        EXPECT(b) TO_BE(NULL);

        Baal_clear(test);

        b = Baal_allocMany(test, 9);
        EXPECT(b) NOT TO_BE(NULL);
    }
}