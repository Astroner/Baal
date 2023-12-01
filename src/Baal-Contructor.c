#include "Baal.h"
#include "Baal-Defines.h"


Baal* Baal_create(size_t blockLength, size_t groupSize, size_t groupsNumber) {
    size_t memSize = sizeof(Baal) + Baal_getTotalMemorySize(blockLength, groupSize, groupsNumber);
    char* mem = BAAL_STD_MALLOC(memSize);

    if(mem == NULL) return NULL;

    Baal* baal = (Baal*)mem;
    char* buffer = mem + sizeof(Baal);

    return Baal_construct(baal, buffer, memSize, blockLength, groupSize);
}

void Baal_destroy(Baal* baal) {
    BAAL_STD_FREE(baal);
}

Baal* Baal_init(Baal* baal) {
    baal->initialized = 1;
    
    baal->first = (Baal_internal_ChunkInfo*)baal->buffer;
    baal->first->nextChunk = NULL;
    baal->first->chunkSize = baal->groupsNumber;

    baal->groupLength = BAAL_ADDED_INFO_SIZE + baal->blockLength * baal->groupSize;

    return baal;
}

Baal* Baal_construct(Baal* baal, char* buffer, size_t bufferLength, size_t blockLength, size_t groupSize) {
    size_t realBlockSize = Baal_roundBlockLength(blockLength, groupSize);

    size_t groupsNumber = bufferLength / (BAAL_ADDED_INFO_SIZE + groupSize * realBlockSize);

    if(groupsNumber == 0) return NULL;

    baal->blockLength = realBlockSize;
    baal->buffer = buffer;
    baal->groupSize = groupSize;
    baal->groupsNumber = groupsNumber;
    baal->groupLength = BAAL_ADDED_INFO_SIZE + realBlockSize * groupSize;

    Baal_init(baal);

    return baal;
}
