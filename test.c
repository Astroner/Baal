#include "tests.h"

CREATE_PRINTF_LIKE_FUNCTION(testPrint, 500);

#define BAAL_STD_PRINT testPrint
#define BAAL_IMPLEMENTATION
#include "Baal.h"

DESCRIBE("Baal") {
    IT("works") {
        testPrint__reset();
        Baal_define(test, 200, 1, 1000);

        char* a = Baal_alloc(test);
        EXPECT(a) NOT TO_BE(NULL);

        Baal_print(test);

        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 999\n"
        )
    }

    IT("allocates memory until it is full") {
        Baal_define(test, 1, 1, 10);

        EXPECT(Baal_allocMany(test, 2)) NOT TO_BE(NULL);
        EXPECT(Baal_allocMany(test, 2)) NOT TO_BE(NULL);
        EXPECT(Baal_allocMany(test, 3)) NOT TO_BE(NULL);
        EXPECT(Baal_allocMany(test, 10)) TO_BE(NULL);
    }

    IT("frees memory to be allocated again") {
        Baal_define(test, 1, 1, 9);

        char* a = Baal_allocMany(test, 3);
        char* b = Baal_allocMany(test, 3);
        char* c = Baal_allocMany(test, 3);

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

        char* a = Baal_allocMany(test, 3);

        char* b = Baal_allocMany(test, 9);
        EXPECT(b) TO_BE(NULL);

        Baal_clear(test);

        b = Baal_allocMany(test, 9);
        EXPECT(b) NOT TO_BE(NULL);
    }

    IT("shrinks allocated memory") {
        Baal_define(test, 1, 1, 9);

        testPrint__reset();
        char* a = Baal_allocMany(test, 5);
        Baal_print(test);

        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 5\n"
            "[005]    FREE_CHUNK    SIZE: 4\n"
        );

        testPrint__reset();
        Baal_realloc(test, a, 2);
        Baal_print(test);
        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 2\n"
            "[002]    FREE_CHUNK    SIZE: 7\n"
        );
    }

    IT("extends allocated memory when possible") {
        Baal_define(test, 1, 1, 9);
        
        testPrint__reset();
        char* a = Baal_allocMany(test, 1);
        Baal_print(test);

        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 8\n"
        );

        testPrint__reset();
        Baal_realloc(test, a, 5);
        Baal_print(test);
        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 5\n"
            "[005]    FREE_CHUNK    SIZE: 4\n"
        );
    }

    IT("doesn't extend allocated memory when it is impossible") {
        Baal_define(test, 1, 1, 9);
        
        testPrint__reset();
        char* a = Baal_allocMany(test, 1);
        Baal_print(test);

        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 8\n"
        );

        testPrint__reset();
        EXPECT(Baal_realloc(test, a, 10)) TO_BE(NULL);
        Baal_print(test);
        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 8\n"
        );
    }

    IT("reallocates memory to extend it's size") {
        Baal_define(test, 1, 1, 9);
        
        char* a = Baal_allocMany(test, 1);
        Baal_allocMany(test, 1);

        testPrint__reset();
        Baal_print(test);


        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    BUSY_CHUNK    SIZE: 1\n"
            "[002]    FREE_CHUNK    SIZE: 7\n"
        );

        EXPECT(Baal_realloc(test, a, 5)) NOT TO_BE(NULL);
        testPrint__reset();
        Baal_print(test);
        EXPECT((char*)testPrint__buffer) TO_BE_STRING(
            "[000]    FREE_CHUNK    SIZE: 1\n"
            "[001]    BUSY_CHUNK    SIZE: 1\n"
            "[002]    BUSY_CHUNK    SIZE: 5\n"
            "[007]    FREE_CHUNK    SIZE: 2\n"
        );
    }
}
