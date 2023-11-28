#include "Baal.h"

#include <stdio.h>

static inline size_t Baal_internal_chunkGroupIndex(const Baal* baal, const void* chunk) {
    return ((char*)chunk - baal->buffer) / baal->groupLength;
}

static void Baal_internal_printChunksBetween(const Baal* baal, const void* start, const void* finish) {
    const Baal_internal_AddedInfo* current = start;
    while(current != finish) {
        printf("[%.3zu]    BUSY_CHUNK    SIZE: %zu\n", Baal_internal_chunkGroupIndex(baal, current), current->chunkSize);

        current = (Baal_internal_AddedInfo*)((char*)current + current->chunkSize * baal->groupLength);
    }
}

void Baal_print(const Baal* baal) {
    if(baal->first == NULL) {
        for(size_t i = 0; i < baal->groupsNumber; i += 0) {
            Baal_internal_AddedInfo* current = (Baal_internal_AddedInfo*)(
                baal->buffer + i * (BAAL_ADDED_INFO_SIZE + baal->blockLength * baal->groupSize)
            );
            printf("[%.3zu]    BUSY_CHUNK    SIZE: %zu\n", i, current->chunkSize);

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

        printf("[%.3zu]    FREE_CHUNK    SIZE: %zu\n", Baal_internal_chunkGroupIndex(baal, current), current->chunkSize);

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

    printf("Total Memory Size: %zu\n", totalMemorySize);
    printf("Added Info Size: %zu\n", BAAL_ADDED_INFO_SIZE);
    printf("Block Size: %zu\nGroup Size: %zu\nGroups Number: %zu\n", baal->blockLength, baal->groupSize, baal->groupsNumber);
    printf("First Free Chunk: %p\n", (void*)baal->first);
    printf("Bytes:\n");
    for(size_t i = 0; i < totalMemorySize; i++) {
        if(i % (baal->blockLength * baal->groupSize + BAAL_ADDED_INFO_SIZE) == 0) printf("------------------------------------- GROUP START\n");

        int isDataStart = (i - BAAL_ADDED_INFO_SIZE) % baal->groupLength == 0;
        if(isDataStart) printf("--------------------- DATA START\n");

        unsigned int value = (unsigned char)baal->buffer[i];
        printf("%p    0x%.2X", (void*)(baal->buffer + i), value);

        if(isDataStart) {
            void** ptr = (void*)(baal->buffer + i);
            printf("    AS POINTER %p", *ptr);
        }

        printf("\n");
    }
}
