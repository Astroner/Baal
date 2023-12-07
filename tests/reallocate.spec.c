#include "tests-new.h"

#define WITH_AFTER_EACH
#include "tests.h"

#include "common.h"

#include "../Baal.h"

AFTER_EACH(reallocate) {
    testPrint__reset();
}

DESCRIBE(reallocate) {
    IT("shrinks allocated memory") {
        Baal_define(test, 1, 1, 9);

        char* a = Baal_allocMany(test, 5);
        char* initState = testPrint__nextString();
        Baal_print(test);
        EXPECT(initState) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 5\n"
            "[005]    FREE_CHUNK    SIZE: 4\n"
        );

        Baal_realloc(test, a, 2);
        char* afterRealloc = testPrint__nextString();
        Baal_print(test);
        EXPECT(afterRealloc) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 2\n"
            "[002]    FREE_CHUNK    SIZE: 7\n"
        );
    }

    IT("extends allocated memory when possible") {
        Baal_define(test, 1, 1, 9);
        
        char* ptr = Baal_allocMany(test, 1);
        char* initState = testPrint__nextString();
        Baal_print(test);
        EXPECT(initState) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 8\n"
        );

        EXPECT(Baal_realloc(test, ptr, 5)) TO_BE(ptr);

        char* afterRealloc = testPrint__nextString();
        Baal_print(test);
        EXPECT(afterRealloc) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 5\n"
            "[005]    FREE_CHUNK    SIZE: 4\n"
        );
    }

    IT("doesn't extend allocated memory when it is impossible") {
        Baal_define(test, 1, 1, 9);
        
        char* a = Baal_allocMany(test, 1);
        char* initState = testPrint__nextString();
        Baal_print(test);
        EXPECT(initState) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 8\n"
        );

        EXPECT(Baal_realloc(test, a, 10)) TO_BE(NULL);
        char* afterRealloc = testPrint__nextString();
        Baal_print(test);
        EXPECT(afterRealloc) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 8\n"
        );
    }

    IT("reallocates memory to extend it's size") {
        Baal_define(test, 1, 1, 9);
        
        char* a = Baal_allocMany(test, 1);
        Baal_allocMany(test, 1);
        char* initState = testPrint__nextString();
        Baal_print(test);
        EXPECT(initState) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    BUSY_CHUNK    SIZE: 1\n"
            "[002]    FREE_CHUNK    SIZE: 7\n"
        );

        
        EXPECT(Baal_realloc(test, a, 5)) NOT TO_BE(a);
        char* afterRealloc = testPrint__nextString();
        Baal_print(test);
        EXPECT(afterRealloc) TO_BE_STRING(
            "[000]    FREE_CHUNK    SIZE: 1\n"
            "[001]    BUSY_CHUNK    SIZE: 1\n"
            "[002]    BUSY_CHUNK    SIZE: 5\n"
            "[007]    FREE_CHUNK    SIZE: 2\n"
        );
    }

    IT("takes space from neighboring free chunks to extend allocated memory if simple reallocation is impossible") {
        Baal_define(test, 1, 1, 12);

        char* a = Baal_allocMany(test, 3);
        char* b = Baal_allocMany(test, 2);
        char* c = Baal_allocMany(test, 3);
        Baal_alloc(test);

        Baal_free(test, a);
        Baal_free(test, c);

        char* initState = testPrint__nextString();
        Baal_print(test);
        EXPECT(initState) TO_BE_STRING(
            "[000]    FREE_CHUNK    SIZE: 3\n"
            "[003]    BUSY_CHUNK    SIZE: 2\n"
            "[005]    FREE_CHUNK    SIZE: 3\n"
            "[008]    BUSY_CHUNK    SIZE: 1\n"
            "[009]    FREE_CHUNK    SIZE: 3\n"
        );

        EXPECT(Baal_realloc(test, b, 6)) NOT TO_BE(b);

        char* afterRealloc = testPrint__nextString();
        Baal_print(test);
        EXPECT(afterRealloc) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 6\n"
            "[006]    FREE_CHUNK    SIZE: 2\n"
            "[008]    BUSY_CHUNK    SIZE: 1\n"
            "[009]    FREE_CHUNK    SIZE: 3\n"
        );
    }

    IT("reallocates data safely") {
        typedef struct TestObject {
            int x;
            int y;
            int w;
            int h;
            int dx;
            int dy;
        } TestObject;

        Baal_define(test, sizeof(TestObject), 1, 10);

        TestObject original = {
            .x = 22,
            .y = 33,
            .w = 100,
            .h = 200,
            .dx = 1,
            .dy = 540,
        };

        TestObject* init = Baal_alloc(test);
        Baal_alloc(test);

        memcpy(init, &original, sizeof(original));

        init = Baal_realloc(test, init, 2);

        EXPECT(init) TO_HAVE_BYTES(&original, sizeof(original));
    }
}