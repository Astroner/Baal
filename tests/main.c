#include "tests-new.h"

#define MULTI_TEST
#include "tests.h"
CREATE_PRINTF_LIKE_FUNCTION(testPrint, 1000)

#include "general.spec.c"
#include "allocate.spec.c"
#include "deallocate.spec.c"
#include "reallocate.spec.c"

#define BAAL_STD_PRINT testPrint
#define BAAL_IMPLEMENTATION
#include "../Baal.h"

RUN_TESTS(general, allocate, deallocate, reallocate)
