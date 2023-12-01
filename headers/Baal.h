#if !defined(BAAL_H)
#define BAAL_H

#include <stddef.h>

#define BAAL_INTERNAL_ADDED_INFO\
    size_t chunkSize;\

#define BAAL_INTERNAL_INSERTED_INFO\
    struct Baal_internal_ChunkInfo* nextChunk;

typedef struct Baal_internal_AddedInfo {
    BAAL_INTERNAL_ADDED_INFO
} Baal_internal_AddedInfo;

typedef struct Baal_internal_InsertedInfo {
    BAAL_INTERNAL_INSERTED_INFO
} Baal_internal_InsertedInfo;

typedef struct Baal_internal_ChunkInfo {
    BAAL_INTERNAL_ADDED_INFO
    BAAL_INTERNAL_INSERTED_INFO
} Baal_internal_ChunkInfo;

#define BAAL_GROUP_MIN_SIZE sizeof(Baal_internal_InsertedInfo)
#define BAAL_ADDED_INFO_SIZE sizeof(Baal_internal_AddedInfo)

#define Baal_roundBlockLength(BLOCK_SIZE, GROUP_SIZE) (BLOCK_SIZE * GROUP_SIZE >= BAAL_GROUP_MIN_SIZE ? BLOCK_SIZE : BAAL_GROUP_MIN_SIZE / GROUP_SIZE)

#define Baal_getTotalMemorySize(BLOCK_SIZE, GROUP_SIZE, GROUPS_NUMBER)\
    (GROUPS_NUMBER * (Baal_roundBlockLength(BLOCK_SIZE, GROUP_SIZE) * GROUP_SIZE + BAAL_ADDED_INFO_SIZE))

typedef struct Baal {
    int initialized;
    Baal_internal_ChunkInfo* first;
    char* buffer;
    size_t blockLength;
    size_t groupSize;
    size_t groupLength;
    size_t groupsNumber;
} Baal;

#define Baal_define(NAME, BLOCK_SIZE, GROUP_SIZE, GROUPS_NUMBER)\
    char NAME##__buffer[Baal_getTotalMemorySize(BLOCK_SIZE, GROUP_SIZE, GROUPS_NUMBER)];\
    Baal NAME##__data = {\
        .initialized = 0,\
        .buffer = NAME##__buffer,\
        .blockLength = Baal_roundBlockLength(BLOCK_SIZE, GROUP_SIZE),\
        .groupSize = GROUP_SIZE,\
        .groupsNumber = GROUPS_NUMBER,\
    };\
    Baal* NAME = &NAME##__data;\

#define Baal_defineStatic(NAME, BLOCK_SIZE, GROUP_SIZE, GROUPS_NUMBER)\
    static char NAME##__buffer[Baal_getTotalMemorySize(BLOCK_SIZE, GROUP_SIZE, GROUPS_NUMBER)];\
    static Baal NAME##__data = {\
        .initialized = 0,\
        .buffer = NAME##__buffer,\
        .blockLength = Baal_roundBlockLength(BLOCK_SIZE, GROUP_SIZE),\
        .groupSize = GROUP_SIZE,\
        .groupsNumber = GROUPS_NUMBER,\
    };\
    static Baal* NAME = &NAME##__data;\


Baal* Baal_create(size_t blockLength, size_t groupSize, size_t groupsNumber);
// Just free() alias
void Baal_destroy(Baal*);
Baal* Baal_init(Baal* baal);
Baal* Baal_construct(Baal* baal, char* buffer, size_t bufferLength, size_t blockLength, size_t groupSize);

// allocates one group
void* Baal_alloc(Baal* baal);
// allocates series of groups
void* Baal_allocMany(Baal* baal, size_t blocksNumber);

void* Baal_realloc(Baal* baal, void* ptr, size_t newBlocksNumber);


void Baal_free(Baal* baal, void* ptr);
void Baal_clear(Baal* baal);

void Baal_print(const Baal* baal);
void Baal_memorySnapshot(const Baal* baal);

#endif // BAAL_H
