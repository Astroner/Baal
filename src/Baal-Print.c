#include "Baal.h"
#include "Baal-Defines.h"

#if defined(BAAL_DEBUG)

void Baal_print(const Baal* baal) {
    BAAL_ITERATE(baal, info, void*, val) {
        BAAL_STD_PRINT("[%.3zu]    %s_CHUNK    SIZE: %zu\n", info.index, info.isFree ? "FREE" : "BUSY", info.chunkSize);
    }
}

void Baal_memorySnapshot(const Baal* baal) {
    size_t totalMemorySize = baal->groupsNumber * baal->groupLength;

    BAAL_STD_PRINT("Total Memory Length: %zu\n", totalMemorySize);
    BAAL_STD_PRINT("Added Info Length: %zu\n", BAAL_ADDED_INFO_SIZE);
    BAAL_STD_PRINT("Block Length: %zu\nGroup Size: %zu\nGroups Number: %zu\n", baal->blockLength, baal->groupSize, baal->groupsNumber);
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

#endif // BAAL_DEBUG
