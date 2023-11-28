#include "Baal.h"
#include "todo.h"


#include <stdio.h>
#include <stdlib.h>



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
void* Baal_reallocBlocks(Baal* baal, void* ptr, size_t newBlocksSize) {
    if((char*)ptr < baal->buffer || (char*)ptr > baal->buffer + Baal_getTotalMemorySize(baal->blockLength, baal->groupSize, baal->groupsNumber)) {
        return NULL;
    }

    Baal_internal_ChunkInfo* chunkToReallocate = (Baal_internal_ChunkInfo*)((char*)ptr - BAAL_ADDED_INFO_SIZE);

    if(newBlocksSize == 0) {
        Baal_internal_freeChunk(baal, chunkToReallocate);
        return NULL;
    }

    size_t requiredGroups = newBlocksSize / baal->groupSize;
    if(requiredGroups * baal->groupSize != newBlocksSize) {
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
    Baal_internal_ChunkInfo* chunkToReallocatePrev = NULL;
    Baal_internal_ChunkInfo* chunkToReallocateNext = NULL;

    int hasPerfectHit = 0;
    Baal_internal_ChunkInfo* fittestPrev = NULL;
    Baal_internal_ChunkInfo* fittest = NULL;

    Baal_internal_ChunkInfo* prev = NULL;
    Baal_internal_ChunkInfo* current = baal->first;
    while(1) {
        if(chunkToReallocate < current) {
            if(
                (char*)chunkToReallocate + chunkToReallocate->chunkSize * baal->groupLength == (char*)current
                && requiredGroups <= chunkToReallocate->chunkSize + current->chunkSize
            ) {
                TODO("Expand chunk by taking space from the next chunk");

                return chunkToReallocate;
            } else {
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
        TODO("Allocate new chunk and copy data there");

        return fittest;
    }

    TODO("Try to concatenate neighbors to reach required groups size");

    return NULL;
}
