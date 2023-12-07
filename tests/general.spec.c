#include "tests-new.h"

#include "tests.h"

#include "../Baal.h"

DESCRIBE(general) {
    IT("works") {
        Baal_define(test, 200, 1, 1000);

        char* a = Baal_alloc(test);
        EXPECT(a) NOT TO_BE(NULL);

        char* testState = testPrint__nextString();
        Baal_print(test);

        EXPECT(testState) TO_BE_STRING(
            "[000]    BUSY_CHUNK    SIZE: 1\n"
            "[001]    FREE_CHUNK    SIZE: 999\n"
        )
    }
}