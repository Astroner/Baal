#if !defined(BAAL_H)
#define BAAL_H

#include <stddef.h>
#include <string.h>

// INDEXING STARTS WITH 1 NOT WITH 0
// to allow not existing prev and next indexes
// therefore lengths and info arrays have N + 1 length
typedef struct Baal_internal_ChunkInfo {
    size_t prevIndex;
    size_t nextIndex;
    int status;
    size_t length;
} Baal_internal_ChunkInfo;

typedef struct Baal {
    Baal_internal_ChunkInfo* info; // array of chunk info
    size_t cursor; // pointer to first free chunk

    char* buffer;
    size_t blockSize;
    size_t blocksNumber;
} Baal;

#define Baal_define(name, blockSizeArg, blocksNumberArg)\
    Baal_internal_ChunkInfo name##__info[blocksNumberArg + 1] = {\
        [1] = {\
            .prevIndex = 0,\
            .nextIndex = 0,\
            .status = 0,\
            .length = blocksNumberArg\
        }\
    };\
    char name##__buffer[blockSizeArg * blocksNumberArg];\
    Baal name##__data = {\
        .info = name##__info,\
        .cursor = 1,\
        \
        .buffer = name##__buffer,\
        .blockSize = blockSizeArg,\
        .blocksNumber = blocksNumberArg,\
    };\
    Baal* name = &name##__data;

#define Baal_defineStatic(name, blockSizeArg, blocksNumberArg)\
    static Baal_internal_ChunkInfo name##__info[blocksNumberArg + 1] = {\
        [1] = {\
            .prevIndex = 0,\
            .nextIndex = 0,\
            .status = 0,\
            .length = blocksNumberArg\
        }\
    };\
    static char name##__buffer[blockSizeArg * blocksNumberArg];\
    static Baal name##__data = {\
        .info = name##__info,\
        .cursor = 1,\
        \
        .buffer = name##__buffer,\
        .blockSize = blockSizeArg,\
        .blocksNumber = blocksNumberArg,\
    };\
    static Baal* name = &name##__data;

Baal* Baal_create(size_t blockSize, size_t blocksNumber);
// Just free() alias
void Baal_destroy(Baal*);

void* Baal_alloc(Baal* baal);
void* Baal_allocMany(Baal* baal, size_t number);
void Baal_free(Baal* baal, void* ptr);
void Baal_clear(Baal* baal);

void Baal_print(Baal* baal);

#endif // BAAL_H
