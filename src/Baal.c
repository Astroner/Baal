#include "Baal.h"


#include <stdio.h>
#include <stdlib.h>

#ifndef BAAL_STD_MALLOC
    #define BAAL_STD_MALLOC malloc
#endif

#ifndef BAAL_STD_FREE
    #define BAAL_STD_FREE free
#endif

Baal* Baal_create(size_t blockSize, size_t blocksNumber) {
    char* memory = BAAL_STD_MALLOC(
        sizeof(Baal)
        + sizeof(Baal_internal_ChunkInfo) * (blocksNumber + 1)
        + blocksNumber * blockSize
    );

    if(memory == NULL) return NULL;

    Baal* baal = (Baal*)memory;
    Baal_internal_ChunkInfo* info = (Baal_internal_ChunkInfo*)(memory + sizeof(Baal));
    char* buffer = memory + sizeof(Baal) + sizeof(Baal_internal_ChunkInfo) * (blocksNumber + 1);

    info[1].length = blocksNumber;
    info[1].nextIndex = 0;
    info[1].prevIndex = 0;
    info[1].status = 0;

    baal->info = info;
    baal->cursor = 1;
    baal->blockSize = blockSize;
    baal->blocksNumber = blocksNumber;
    baal->buffer = buffer;

    return baal;
}

void Baal_destroy(Baal* baal) {
    BAAL_STD_FREE(baal);
}

void* Baal_alloc(Baal* baal) {
    return Baal_allocMany(baal, 1);
}

void* Baal_allocMany(Baal* baal, size_t number) {
    if(!baal->cursor) return NULL; // NO FREE BLOCKS

    size_t fittestIndex = 0;

    size_t currentIndex = baal->cursor;
    while(1) {
        if(baal->info[currentIndex].length >= number) {
            if(!fittestIndex) { // first entry
                fittestIndex = currentIndex;
            } else {
                size_t diff = baal->info[currentIndex].length - number;
                if(diff == 0) {
                    fittestIndex = currentIndex;
                    break;
                }
                if (baal->info[currentIndex].length - number < baal->info[fittestIndex].length - number) { // further entries
                    fittestIndex = currentIndex;
                }
            }
        }        

        if(baal->info[currentIndex].nextIndex) {
            currentIndex = baal->info[currentIndex].nextIndex;
        } else {
            break;
        }
    }

    if(!fittestIndex) return NULL;

    Baal_internal_ChunkInfo* fittest = baal->info + fittestIndex;
    fittest->status = 1;

    Baal_internal_ChunkInfo* prev = fittest->prevIndex ? baal->info + fittest->prevIndex : NULL;
    Baal_internal_ChunkInfo* next = fittest->nextIndex ? baal->info + fittest->nextIndex : NULL;

    if(fittest->length == number) {
        if(prev) {
            prev->nextIndex = fittest->nextIndex;
        } else {
            baal->cursor = fittest->nextIndex;
        }

        if(next) {
            next->prevIndex = fittest->prevIndex;
        }
    } else {
        
        size_t freeChunkPieceIndex = fittestIndex + number;
        Baal_internal_ChunkInfo* freeChunkPiece = baal->info + freeChunkPieceIndex;
        
        freeChunkPiece->length = fittest->length - number;
        freeChunkPiece->prevIndex = fittest->prevIndex;
        freeChunkPiece->nextIndex = fittest->nextIndex;
        freeChunkPiece->status = 0;

        fittest->length = number;

        if(prev) {
            prev->nextIndex = freeChunkPieceIndex;
        } else {
            baal->cursor = freeChunkPieceIndex;
        }

        if(next) {
            next->prevIndex = freeChunkPieceIndex;
        }
    }

    return baal->buffer + (fittestIndex - 1) * baal->blockSize;
}

int Baal_internal_tryToMerge(Baal* baal, size_t firstIndex, size_t secondIndex) {
    Baal_internal_ChunkInfo* first = baal->info + firstIndex;

    if(firstIndex + first->length != secondIndex) {
        return 0;
    }

    
    Baal_internal_ChunkInfo* second = baal->info + secondIndex;

    first->length += second->length;

    first->nextIndex = second->nextIndex;

    if(first->nextIndex) {
        baal->info[first->nextIndex].prevIndex = firstIndex;
    }

    return 1;
}

void Baal_internal_mergeAround(Baal* baal, size_t chunkIndex, Baal_internal_ChunkInfo* chunk) {    
    size_t operandIndex = chunkIndex;
    if(chunk->prevIndex) {
        if(Baal_internal_tryToMerge(baal, chunk->prevIndex, chunkIndex)) {
            operandIndex = chunk->prevIndex;
        }
    }

    if(chunk->nextIndex) {
        Baal_internal_tryToMerge(baal, operandIndex, chunk->nextIndex);
    }
}

void Baal_free(Baal* baal, void* ptr) {
    size_t ptrIndex = ((size_t)ptr - (size_t)baal->buffer) / baal->blockSize + 1;
    Baal_internal_ChunkInfo* ptrInfo = baal->info + ptrIndex;

    ptrInfo->status = 0;

    if(!baal->cursor) {
        baal->cursor = ptrIndex;

        return;
    }

    size_t currentIndex = baal->cursor;
    while(1) {
        Baal_internal_ChunkInfo* current = baal->info + currentIndex;

        if(ptrIndex < currentIndex) {
            Baal_internal_ChunkInfo* prev = current->prevIndex ? baal->info + current->prevIndex : NULL;

            if(prev) prev->nextIndex = ptrIndex;
            else baal->cursor = ptrIndex;

            ptrInfo->prevIndex = current->prevIndex;
            ptrInfo->nextIndex = currentIndex;

            current->prevIndex = ptrIndex;

            Baal_internal_mergeAround(baal, ptrIndex, ptrInfo);

            break;
        }      

        if(current->nextIndex) {
            currentIndex = current->nextIndex;
        } else {
            current->nextIndex = ptrIndex;

            Baal_internal_mergeAround(baal, ptrIndex, ptrInfo);

            break;
        }
    }
}

void Baal_print(Baal* baal) {
    for(size_t i = 1; i < baal->blocksNumber + 1; i++) {
        for(size_t j = 0; j < baal->info[i].length; j++) {
            printf("[%.3zu]  %s", i + j, baal->info[i].status ? "BUSY" : "FREE");

            if(j == 0 && baal->info[i].length > 1) {
                printf("  CHUNK SIZE: %zu", baal->info[i].length);
            }

            printf("\n");
        }
        i += baal->info[i].length - 1;
    }
}

void Baal_clear(Baal* baal) {
    baal->cursor = 1;

    baal->info[1].status = 0;
    baal->info[1].length = baal->blocksNumber;
    baal->info[1].nextIndex = 0;
    baal->info[1].prevIndex = 0;
}