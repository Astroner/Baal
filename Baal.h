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
#if defined(BAAL_IMPLEMENTATION)
#if !defined(BAAL_DEFINES_H)
#define BAAL_DEFINES_H

#if !defined(BAAL_STD_MEMCPY)
    #include <string.h>
    #define BAAL_STD_MEMCPY memcpy
#endif // 

#if !defined(BAAL_STD_MEMMOVE)
    #include <string.h>
    #define BAAL_STD_MEMMOVE memmove
#endif // BAAL_STD_MEMMOVE

#if !defined(BAAL_STD_MALLOC)
    #include <stdlib.h>
    #define BAAL_STD_MALLOC malloc
#endif

#if !defined(BAAL_STD_FREE)
    #include <stdlib.h>
    #define BAAL_STD_FREE free
#endif

#if !defined(BAAL_STD_PRINT)
    #include <stdio.h>
    #define BAAL_STD_PRINT printf
#endif // BAAL_STD_PRINT


#endif // BAAL_DEFINES_H


void* Baal_alloc(Baal* baal) {
    return Baal_allocMany(baal, baal->groupSize);
}

void* Baal_allocMany(Baal* baal, size_t blocksNumber) {
    #if !defined(BAAL_SKIP_INIT_CHECK)
        if(!baal->initialized) {
            Baal_init(baal);
        }
    #endif

    if(baal->first == NULL) return NULL;

    size_t groupsToAllocate = blocksNumber / baal->groupSize;
    if(groupsToAllocate * baal->groupSize != blocksNumber) {
        groupsToAllocate += 1;
    }

    Baal_internal_ChunkInfo* fittest = NULL;
    Baal_internal_ChunkInfo* fittestPrev = NULL;

    Baal_internal_ChunkInfo* prev = NULL;
    Baal_internal_ChunkInfo* current = baal->first;
    while(1) {
        if(groupsToAllocate <= current->chunkSize) {
            if(fittest == NULL) {
                fittestPrev = prev;
                fittest = current;
                if(current->chunkSize == groupsToAllocate) { // first is perfect fit
                    break;
                }
            } else if(current->chunkSize == groupsToAllocate) { // perfect fit
                fittestPrev = prev;
                fittest = current;
                break;
            } else if(current->chunkSize - groupsToAllocate < fittest->chunkSize - groupsToAllocate) {
                fittestPrev = prev;
                fittest = current;
            }
        }

        if(current->nextChunk) {
            prev = current;
            current = current->nextChunk;
        } else {
            break;
        }
    }

    if(!fittest) return NULL;

    Baal_internal_ChunkInfo* fittestNext = fittest->nextChunk;
    if(fittest->chunkSize == groupsToAllocate) {
        if(fittestPrev) {
            fittestPrev->nextChunk = fittestNext;
        } else {
            baal->first = fittestNext;
        }
    } else {
        Baal_internal_ChunkInfo* newChunk = (Baal_internal_ChunkInfo*)((char*)fittest + groupsToAllocate * baal->groupLength);
        newChunk->nextChunk = fittestNext;
        newChunk->chunkSize = fittest->chunkSize - groupsToAllocate;
        fittest->chunkSize = groupsToAllocate;

        if(fittestPrev) {
            fittestPrev->nextChunk = newChunk;
        } else {
            baal->first = newChunk;
        }
    }


    return (char*)fittest + BAAL_ADDED_INFO_SIZE;
}


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


static int Baal_internal_mergeWithNext(Baal* baal, Baal_internal_ChunkInfo* chunk) {
    if((char*)chunk + chunk->chunkSize * baal->groupLength != (char*)chunk->nextChunk) return 0;

    chunk->chunkSize += chunk->nextChunk->chunkSize;
    chunk->nextChunk = chunk->nextChunk->nextChunk;

    return 1;
}

static void Baal_internal_findNextFreeChunk(Baal* baal, Baal_internal_ChunkInfo* chunk, Baal_internal_ChunkInfo** prevChunk, Baal_internal_ChunkInfo** nextChunk) {
    *prevChunk = NULL;
    *nextChunk = NULL;

    Baal_internal_ChunkInfo* prev = NULL;
    Baal_internal_ChunkInfo* current = baal->first;
    while(1) {
        if(chunk < current) {
            *nextChunk = current;
            *prevChunk = prev;
            break;
        }

        if(current->nextChunk) {
            prev = current;
            current = current->nextChunk;
        } else {
            *prevChunk = current;
            break;
        }
    }

    return;
}

static void Baal_internal_freeChunk(Baal* baal, Baal_internal_ChunkInfo* chunkToDeallocate) {
    if(baal->first == NULL) {
        baal->first = chunkToDeallocate;
        chunkToDeallocate->nextChunk = NULL;

        return;
    }
    Baal_internal_ChunkInfo* prevChunk = NULL;
    Baal_internal_ChunkInfo* nextChunk = NULL;

    Baal_internal_findNextFreeChunk(baal, chunkToDeallocate, &prevChunk, &nextChunk);

    if(nextChunk == NULL) {
        prevChunk->nextChunk = chunkToDeallocate;
        chunkToDeallocate->nextChunk = NULL;
        Baal_internal_mergeWithNext(baal, prevChunk);
        return;
    }

    chunkToDeallocate->nextChunk = nextChunk;
    
    Baal_internal_ChunkInfo* chunkToMerge = chunkToDeallocate;
    if(prevChunk) {
        prevChunk->nextChunk = chunkToDeallocate;
        if(Baal_internal_mergeWithNext(baal, prevChunk)) {
            chunkToMerge = prevChunk;
        }
    } else {
        baal->first = chunkToDeallocate;
    }

    Baal_internal_mergeWithNext(baal, chunkToMerge);
}

void Baal_free(Baal* baal, void* ptr) {
    if((char*)ptr < baal->buffer || (char*)ptr > baal->buffer + Baal_getTotalMemorySize(baal->blockLength, baal->groupSize, baal->groupsNumber)) {
        return;
    }

    Baal_internal_ChunkInfo* chunkToDeallocate = (Baal_internal_ChunkInfo*)((char*)ptr - BAAL_ADDED_INFO_SIZE);

    Baal_internal_freeChunk(baal, chunkToDeallocate);
}

void Baal_clear(Baal* baal) {
    baal->first = (Baal_internal_ChunkInfo*)baal->buffer;
    baal->first->nextChunk = NULL;
    baal->first->chunkSize = baal->groupsNumber;
}

// If impossible to increase memory size, returns NULL
void* Baal_realloc(Baal* baal, void* ptr, size_t newBlocksNumber) {
    if((char*)ptr < baal->buffer || (char*)ptr > baal->buffer + Baal_getTotalMemorySize(baal->blockLength, baal->groupSize, baal->groupsNumber)) {
        return NULL;
    }

    Baal_internal_ChunkInfo* chunkToReallocate = (Baal_internal_ChunkInfo*)((char*)ptr - BAAL_ADDED_INFO_SIZE);

    if(newBlocksNumber == 0) {
        Baal_internal_freeChunk(baal, chunkToReallocate);
        return NULL;
    }

    size_t requiredGroups = newBlocksNumber / baal->groupSize;
    if(requiredGroups * baal->groupSize != newBlocksNumber) {
        requiredGroups += 1;
    }


    if(requiredGroups == chunkToReallocate->chunkSize) {
        return ptr;
    }

    if(requiredGroups < chunkToReallocate->chunkSize) {
        Baal_internal_ChunkInfo* newChunk = (Baal_internal_ChunkInfo*)((char*)chunkToReallocate + requiredGroups * baal->groupLength);

        newChunk->chunkSize = chunkToReallocate->chunkSize - requiredGroups;
        chunkToReallocate->chunkSize = requiredGroups;

        Baal_internal_freeChunk(baal, newChunk);

        return ptr;
    }

    if(baal->first == NULL) return NULL;


    int checkedNeighbors = 0;
    Baal_internal_ChunkInfo* chunkToReallocatePrevPrev = NULL;
    Baal_internal_ChunkInfo* chunkToReallocatePrev = NULL;
    Baal_internal_ChunkInfo* chunkToReallocateNext = NULL; // This value should be used only if chunkToReallocatePrev is not available

    int hasPerfectHit = 0;
    Baal_internal_ChunkInfo* fittestPrev = NULL;
    Baal_internal_ChunkInfo* fittest = NULL;

    Baal_internal_ChunkInfo* prevPrev = NULL;
    Baal_internal_ChunkInfo* prev = NULL;
    Baal_internal_ChunkInfo* current = baal->first;
    while(1) {
        if(chunkToReallocate < current) {
            if(
                (char*)chunkToReallocate + chunkToReallocate->chunkSize * baal->groupLength == (char*)current
                && (requiredGroups <= chunkToReallocate->chunkSize + current->chunkSize)
            ) {
                size_t groupsToAdd = requiredGroups - chunkToReallocate->chunkSize;
                if(groupsToAdd == current->chunkSize) {
                    chunkToReallocate->chunkSize = requiredGroups;

                    if(prev) {
                        prev->nextChunk = current->nextChunk;
                    } else {
                        baal->first = current->nextChunk;
                    }
                } else {
                    Baal_internal_ChunkInfo* newChunk = (Baal_internal_ChunkInfo*)((char*)current + groupsToAdd * baal->groupLength);
                    newChunk->nextChunk = current->nextChunk;
                    newChunk->chunkSize = current->chunkSize - groupsToAdd;

                    chunkToReallocate->chunkSize = requiredGroups;

                    if(prev) {
                        prev->nextChunk = newChunk;
                    } else {
                        baal->first = newChunk;
                    }
                }

                return ptr;
            } else {
                chunkToReallocatePrevPrev = prevPrev;
                chunkToReallocatePrev = prev;
                chunkToReallocateNext = current;
                checkedNeighbors = 1;
            }
        }

        if(!hasPerfectHit && requiredGroups <= current->chunkSize) {
            if(fittest == NULL) {
                fittest = current;
                fittestPrev = prev;

                if(current->chunkSize == requiredGroups) {
                    hasPerfectHit = 1;
                }
            } else if(current->chunkSize == requiredGroups) {
                fittest = current;
                fittestPrev = prev;

                hasPerfectHit = 1;
            } else if(current->chunkSize - requiredGroups < fittest->chunkSize - requiredGroups) {
                fittest = current;
                fittestPrev = prev;
            }
        }

        if(checkedNeighbors && hasPerfectHit) {
            break;
        }
        
        if(current->nextChunk) {
            prevPrev = prev;
            prev = current;
            current = current->nextChunk;
        } else {
            if(!checkedNeighbors) {
                chunkToReallocatePrev = current;
            }
            break;
        }
    }

    if(fittest) {
        Baal_internal_ChunkInfo* newChunk = NULL;
        if(hasPerfectHit) {
            if(fittestPrev) {
                fittestPrev->nextChunk = fittest->nextChunk;
            } else {
                baal->first = fittest->nextChunk;
            }

            if(chunkToReallocatePrev == fittest) {
                chunkToReallocatePrev = fittestPrev;
            }

            if(chunkToReallocateNext == fittest) {
                chunkToReallocateNext = fittest->nextChunk;
            }
        } else {
            newChunk = (Baal_internal_ChunkInfo*)((char*)fittest + requiredGroups * baal->groupLength);
            newChunk->chunkSize = fittest->chunkSize - requiredGroups;
            newChunk->nextChunk = fittest->nextChunk;

            fittest->chunkSize = requiredGroups;

            if(fittestPrev) {
                fittestPrev->nextChunk = newChunk;
            } else {
                baal->first = newChunk;
            }

            if(chunkToReallocatePrev == fittest) {
                chunkToReallocatePrev = newChunk;
            }

            if(chunkToReallocateNext == fittest) {
                chunkToReallocateNext = newChunk;
            }
        }

        void* newPtr = (char*)fittest + BAAL_ADDED_INFO_SIZE;

        BAAL_STD_MEMCPY(newPtr, ptr, chunkToReallocate->chunkSize * baal->groupLength - BAAL_ADDED_INFO_SIZE);

        if(chunkToReallocateNext) {
            chunkToReallocate->nextChunk = chunkToReallocateNext;
            Baal_internal_mergeWithNext(baal, chunkToReallocate);
        }

        if(chunkToReallocatePrev) {
            chunkToReallocatePrev->nextChunk = chunkToReallocate;
            Baal_internal_mergeWithNext(baal, chunkToReallocatePrev);
        } else {
            baal->first = chunkToReallocate;
        }

        return newPtr;
    }

    // At this point we already checked next chunk after the one to reallocate
    // So if we got here we already know that next chunk is either not neighboring or not sufficient
    // So without neighboring prev chunk it is pointless to continue
    if(
        chunkToReallocatePrev == NULL 
        || (char*)chunkToReallocatePrev + chunkToReallocatePrev->chunkSize * baal->groupLength != (char*)chunkToReallocate
    ) return NULL;

    // To keep fragmentation low, it is better to at first try to merge neighboring chunks to operate with bigger pieces
    if(
        chunkToReallocateNext != NULL
        && (char*)chunkToReallocate + chunkToReallocate->chunkSize * baal->groupLength == (char*)chunkToReallocateNext
    ) {
        size_t sumSize = chunkToReallocatePrev->chunkSize + chunkToReallocate->chunkSize + chunkToReallocateNext->chunkSize;

        if(sumSize < requiredGroups) return NULL;

        chunkToReallocatePrev->chunkSize = sumSize;

        if(sumSize == requiredGroups) {
            if(chunkToReallocatePrevPrev) {
                chunkToReallocatePrevPrev->nextChunk = chunkToReallocateNext->nextChunk;
            } else {
                baal->first = chunkToReallocateNext->nextChunk;
            }
        } else {
            size_t groupsLeft = sumSize - requiredGroups;

            Baal_internal_ChunkInfo* newChunk = (Baal_internal_ChunkInfo*)((char*)chunkToReallocatePrev + requiredGroups * baal->groupLength);
            newChunk->chunkSize = groupsLeft;
            newChunk->nextChunk = chunkToReallocateNext->nextChunk;

            chunkToReallocatePrev->chunkSize = requiredGroups;

            if(chunkToReallocatePrevPrev) {
                chunkToReallocatePrevPrev->nextChunk = newChunk;
            } else {
                baal->first = newChunk;
            }
        }

        char* newPtr = (char*)chunkToReallocatePrev + BAAL_ADDED_INFO_SIZE;

        BAAL_STD_MEMMOVE(newPtr, ptr, chunkToReallocate->chunkSize * baal->groupLength - BAAL_ADDED_INFO_SIZE);

        return newPtr;
    }

    // Now we only left with prev and current
    size_t prevCurrentSize = chunkToReallocatePrev->chunkSize + chunkToReallocate->chunkSize;
    if(prevCurrentSize < requiredGroups) return NULL;
    
    // Take entire prev chunk
    if(prevCurrentSize == requiredGroups) {
        if(chunkToReallocatePrevPrev != NULL) {
            chunkToReallocatePrevPrev->nextChunk = chunkToReallocateNext;
        } else {
            baal->first = chunkToReallocateNext;
        }

        chunkToReallocatePrev->chunkSize = requiredGroups;

        void* newPtr = (char*)chunkToReallocatePrev + BAAL_ADDED_INFO_SIZE;

        BAAL_STD_MEMMOVE(newPtr, ptr, chunkToReallocate->chunkSize * baal->groupLength - BAAL_ADDED_INFO_SIZE);

        return newPtr;
    }

    // Take only a part of prev chunk
    chunkToReallocatePrev->chunkSize -= requiredGroups - chunkToReallocate->chunkSize;
    Baal_internal_ChunkInfo* newChunk = (Baal_internal_ChunkInfo*)((char*)chunkToReallocatePrev + chunkToReallocatePrev->chunkSize * baal->groupLength);
    newChunk->chunkSize = requiredGroups;

    void* newPtr = (char*)newChunk + BAAL_ADDED_INFO_SIZE;

    BAAL_STD_MEMMOVE(newPtr, ptr, chunkToReallocate->chunkSize * baal->groupLength - BAAL_ADDED_INFO_SIZE);

    return newPtr;
}


static inline size_t Baal_internal_chunkGroupIndex(const Baal* baal, const void* chunk) {
    return ((char*)chunk - baal->buffer) / baal->groupLength;
}

static void Baal_internal_printChunksBetween(const Baal* baal, const void* start, const void* finish) {
    const Baal_internal_AddedInfo* current = start;
    while(current != finish) {
        BAAL_STD_PRINT("[%.3zu]    BUSY_CHUNK    SIZE: %zu\n", Baal_internal_chunkGroupIndex(baal, current), current->chunkSize);

        current = (Baal_internal_AddedInfo*)((char*)current + current->chunkSize * baal->groupLength);
    }
}

void Baal_print(const Baal* baal) {
    if(baal->first == NULL) {
        for(size_t i = 0; i < baal->groupsNumber; i += 0) {
            Baal_internal_AddedInfo* current = (Baal_internal_AddedInfo*)(
                baal->buffer + i * (BAAL_ADDED_INFO_SIZE + baal->blockLength * baal->groupSize)
            );
            BAAL_STD_PRINT("[%.3zu]    BUSY_CHUNK    SIZE: %zu\n", i, current->chunkSize);

            i += current->chunkSize;
        }

        return;
    }
    
    Baal_internal_ChunkInfo* prev = NULL;
    Baal_internal_ChunkInfo* current = baal->first;
    while(1) {
        if(prev != NULL) {
            Baal_internal_printChunksBetween(
                baal, 
                (char*)prev + prev->chunkSize * baal->groupLength,
                current
            );
        } else {
            Baal_internal_printChunksBetween(
                baal, 
                baal->buffer,
                current
            );
        }

        BAAL_STD_PRINT("[%.3zu]    FREE_CHUNK    SIZE: %zu\n", Baal_internal_chunkGroupIndex(baal, current), current->chunkSize);

        if(current->nextChunk) {
            prev = current;
            current = current->nextChunk;
        } else {
            break;
        }
    }

    char* currentChunkEnd = (char*)current + current->chunkSize * baal->groupLength;
    char* bufferEnd = baal->buffer + baal->groupsNumber * baal->groupLength;

    if(currentChunkEnd != bufferEnd) {
        Baal_internal_printChunksBetween(baal, currentChunkEnd, bufferEnd);
    }
}

void Baal_memorySnapshot(const Baal* baal) {
    size_t totalMemorySize = baal->groupsNumber * baal->groupLength;

    BAAL_STD_PRINT("Total Memory Size: %zu\n", totalMemorySize);
    BAAL_STD_PRINT("Added Info Size: %zu\n", BAAL_ADDED_INFO_SIZE);
    BAAL_STD_PRINT("Block Size: %zu\nGroup Size: %zu\nGroups Number: %zu\n", baal->blockLength, baal->groupSize, baal->groupsNumber);
    BAAL_STD_PRINT("First Free Chunk: %p\n", (void*)baal->first);
    BAAL_STD_PRINT("Bytes:\n");
    for(size_t i = 0; i < totalMemorySize; i++) {
        if(i % (baal->blockLength * baal->groupSize + BAAL_ADDED_INFO_SIZE) == 0) BAAL_STD_PRINT("------------------------------------- GROUP START\n");

        int isDataStart = (i - BAAL_ADDED_INFO_SIZE) % baal->groupLength == 0;
        if(isDataStart) BAAL_STD_PRINT("--------------------- DATA START\n");

        unsigned int value = (unsigned char)baal->buffer[i];
        BAAL_STD_PRINT("%p    0x%.2X", (void*)(baal->buffer + i), value);

        if(isDataStart) {
            void** ptr = (void*)(baal->buffer + i);
            BAAL_STD_PRINT("    AS POINTER %p", *ptr);
        }

        BAAL_STD_PRINT("\n");
    }
}

#endif
