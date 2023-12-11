#include "tests-new.h"

#include "common.h"

#include "tests.h"

#include "../Baal.h"

DESCRIBE(iterator) {
    IT("properly counts free and busy groups") {
        Baal_define(test, 1, 1, 10);

        Baal_alloc(test);
        char* a = Baal_alloc(test);
        Baal_alloc(test);
        Baal_alloc(test);

        Baal_free(test, a);

        BaalIterator iter;

        BaalIterator_init(&iter, test);

        int free = 0;
        int busy = 0;
        while(BaalIterator_next(&iter)) {
            if(iter.isFree) free += iter.chunkSize;
            else busy += iter.chunkSize;
        }

        EXPECT(free) TO_BE(7)
        EXPECT(busy) TO_BE(3)
    }

    IT("handles on-fly frees") {
        testPrint__reset();

        Baal_define(test, 1, 1, 10);

        Baal_alloc(test);
        char* a = Baal_alloc(test);
        Baal_alloc(test);
        Baal_alloc(test);

        Baal_free(test, a);

        char* init = testPrint__nextString();
        Baal_print(test);
        
        EXPECT(init) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 1\n"
            "[002]    BUSY_CHUNK    SIZE: 1\n"
            "[003]    BUSY_CHUNK    SIZE: 1\n"
            "[004]    FREE_CHUNK    SIZE: 6\n"
        );

        BAAL_ITERATE(test, iter, char*, ptr) {
            if(iter.isFree) continue;

            BaalIterator_freeCurrent(&iter);
        }

        char* after = testPrint__nextString();
        Baal_print(test);

        EXPECT(after) TO_BE_STRING(
            "[000]    FREE_CHUNK    SIZE: 10\n"
        );
    }
}