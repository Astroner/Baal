#include "Baal.h"



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
