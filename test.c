#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

char testMem[1000];
int mallocCalls = 0;
size_t mallocLastCalledWith = 0;

int freeCalls = 0;
void* freeLastCalledWith = NULL;
    
void* testMalloc(size_t size) {
    mallocCalls++;
    mallocLastCalledWith = size;

    if(size <= sizeof(testMem)) return testMem;
    return NULL;
}

void testFree(void* ptr) {
    freeCalls++;
    freeLastCalledWith = ptr;
}

#define BAAL_STD_MALLOC testMalloc
#define BAAL_STD_FREE testFree
#define BAAL_IMPLEMENTATION
#include "./Baal.h"

char* error = NULL;
int errorLine = 0;

void printCamelCase(char* text) {
    char ch;
    while((ch = *text++)) {
        if(ch >= 'A' && ch <= 'Z') {
            putchar(' ');
            putchar(ch + 32);
        } else {
            putchar(ch);
        }
    }
}

#define EXPECT(expression)\
    errorLine = __LINE__;\
    if(!(expression)) {\
        error = #expression;\
    }\

#define IT(funcName)\
    do {\
        funcName();\
        if(error != NULL) {\
            printf("it ");\
            printCamelCase(#funcName);\
            printf(" - FAILED\nLINE: %d  'EXPECT(%s)'\n\n", errorLine, error);\
            return 1;\
        } else {\
            printf("it ");\
            printCamelCase(#funcName);\
            printf(" - PASS\n");\
        }\
    } while(0);\

void works() {
    Baal_define(baal, 1, 20);

    char* str = Baal_allocMany(baal, 10);
    EXPECT(str != NULL);

    char* str2 = Baal_allocMany(baal, 10);
    EXPECT(str2 != NULL);

    EXPECT(Baal_allocMany(baal, 10) == NULL);

    Baal_free(baal, str);

    char* str3 = Baal_allocMany(baal, 3);

    EXPECT(str3 == str);
}

void allocatesChunksAsExpected() {
    Baal_define(baal, 1, 20);

    baal->info[1].length = 2;
    baal->info[1].nextIndex = 3;

    baal->info[3].status = 0;
    baal->info[3].length = 1;
    baal->info[3].prevIndex = 1;
    baal->info[3].nextIndex = 4;

    baal->info[4].status = 0;
    baal->info[4].length = 17;
    baal->info[4].prevIndex = 3;
    baal->info[4].nextIndex = 0;

    char* str = Baal_allocMany(baal, 5);
    EXPECT(str != NULL);
    EXPECT(baal->info[4].status == 1);
    EXPECT(baal->info[4].length == 5);

    EXPECT(baal->info[9].status == 0);
    EXPECT(baal->info[9].length == 12);
    EXPECT(baal->info[9].prevIndex == 3);
    EXPECT(baal->info[9].nextIndex == 0);
}

void deallocatesChunksAsExpected() {
    Baal_define(baal, 1, 20);

    baal->info[1].status = 0;
    baal->info[1].length = 2;
    baal->info[1].prevIndex = 0;
    baal->info[1].nextIndex = 8;

    baal->info[3].status = 1;
    baal->info[3].length = 5;

    baal->info[8].status = 0;
    baal->info[8].length = 13;
    baal->info[8].prevIndex = 1;
    baal->info[8].nextIndex = 0;

    char* ptr = baal->buffer + 2;

    Baal_free(baal, ptr);

    EXPECT(baal->info[1].status == 0);
    EXPECT(baal->info[1].length == 20);
}

void createsAndDestroysInstanceWithProvidedFunctions() {
    Baal* baal = Baal_create(10, 20);
    EXPECT(mallocCalls == 1);
    EXPECT(mallocLastCalledWith < sizeof(testMem));
    EXPECT((void*)baal == (void*)testMem);

    Baal_destroy(baal);

    EXPECT(freeCalls == 1);
    EXPECT(freeLastCalledWith == baal);
}

void clears() {
    Baal_define(baal, 1, 20);

    Baal_allocMany(baal, 15);
    char* str1 = Baal_allocMany(baal, 15);

    EXPECT(str1 == NULL);

    Baal_clear(baal);

    str1 = Baal_allocMany(baal, 15);

    EXPECT(str1 != NULL);
}

int main(void) {
    printf("\nLaunching test cases:\n");
    IT(works);
    IT(allocatesChunksAsExpected);
    IT(deallocatesChunksAsExpected);
    IT(clears);
    IT(createsAndDestroysInstanceWithProvidedFunctions);

    printf("\n\n");
    return 0;
}