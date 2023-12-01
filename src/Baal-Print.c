#include "Baal.h"
#include "Baal-Defines.h"


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
