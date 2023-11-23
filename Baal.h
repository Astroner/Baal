/*
# Hi There
This is STB-like library for block memory allocation.

# Include
As with any header only library, you need to include *Baal.h* library to your project and define **BAAL_IMPLEMENTATION** before **include** statement to include the implementation.

# Usage
## Create instance
Create Baal instance with macros or with function.

### Macros
**Baal_define(name, blockSize, blocksNumber)**
 - name - **Baal** instance name
 - blockSize - size of single block
 - blocksNumber - total number of blocks

This macro expands into several variable definitions and at the end it will define pointer to **Baal** instance with provided **name**
```c
#define BAAL_IMPLEMENTATION
#include "Baal.h"

Baal_define(globalAllocator, 10, 10);

int main(void) {
    Baal_define(localAllocator, 10, 10);

    return 0;
}
```

**Baal_defineStatic(name, blockSize, blocksNumber)** does the same thing with **Baal_define(name, blockSize, blocksNumber)** but adds modifier **static** to variable definitions.

### Functions
**Baal* Baal_create(size_t blockSize, size_t blocksNumber)**
 - **returns** - **Baal** instance
 - **blockSize** - size of a single block
 - **blocksNumber** - maximum number of blocks to be allocated

By default this function uses **malloc** to allocate required memory from heap, but it can be changed by defining **BAAL_STD_MALLOC** macro with desired function before including the implementation.

**void Baal_destroy(Baal*)** function destroys provided **Baal** instance and frees allocated memory. Basically this function is just an alias for std **free()**, which can be replaced by defining **BAAL_STD_FREE** with desired function.

```c
#define BAAL_IMPLEMENTATION
#include "Baal.h"

int main(void) {
    Baal* baal = Baal_create(10, 20);

    // Do smth

    Baal_destroy(baal);

    return 0;
}
```

## Allocate
### Multiple blocks
**void* Baal_allocMany(Baal* baal, size_t number)**
 - **returns** - **NULLABLE** - pointer to allocated memory
 - **baal** - **Baal** instance
 - **number** - number of blocks to be allocated

This function allocates required number of blocks in a row and returns pointer to the first block. Returns **NULL** if it is not possible to allocate required sequence of blocks.

```c
#include <stdio.h>

#define BAAL_IMPLEMENTATION
#include "Baal.h"

int main(void) {
    Baal_define(baal, 1, 20);

    char* str = Baal_allocMany(baal, 4);

    str[0] = 'H';
    str[1] = 'i';
    str[2] = '!';
    str[3] = '\0';

    printf("%s\n", str);

    Baal_free(baal, str);

    return 0;
}
```

### Single block
**void* Baal_alloc(Baal* baal)**
 - **returns** - **NULLABLE** - pointer to allocated block
 - **baal** - **Baal** instance

Allocates single block of memory and returns it. 

```c
#include <stdio.h>

#define BAAL_IMPLEMENTATION
#include "Baal.h"

typedef struct Obj {
    int index;
    int counter;
} Obj;

Baal_defineStatic(objects, sizeof(Obj), 10);

int main(void) {
    Obj* obj = Baal_alloc(objects);

    obj->index = 2;
    obj->counter = 10;

    printf("Index: %d, Counter: %d\n", obj->index, obj->counter);

    Baal_free(objects, obj);

    return 0;
}
```

## Clear
Use **Baal_clear()** to fully clear the buffer.
> Note, that the function doesn't actually resets the memory, it just resets the pointers pool.
```c
#include <stdio.h>

#define BAAL_IMPLEMENTATION
#include "Baal.h"

typedef struct Obj {
    int index;
    int counter;
} Obj;

Baal_defineStatic(objects, sizeof(Obj), 10);

int main(void) {
    Obj* obj = Baal_alloc(objects);

    obj->index = 2;
    obj->counter = 10;

    printf("Index: %d, Counter: %d\n", obj->index, obj->counter);

    Baal_clear(objects);

    return 0;
}
```

## Print
You can use **Baal_print()** function to print status of memory blocks:
```c
#define BAAL_IMPLEMENTATION
#include "Baal.h"

Baal_defineStatic(numbers, sizeof(int), 10);

int main(void) {
    Baal_alloc(numbers);

    Baal_print(numbers);
    // [001]  BUSY
    // [002]  FREE  CHUNK SIZE: 9
    // [003]  FREE
    // [004]  FREE
    // [005]  FREE
    // [006]  FREE
    // [007]  FREE
    // [008]  FREE
    // [009]  FREE
    // [010]  FREE

    Baal_clear(numbers);

    return 0;
}
```*/


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
#if defined(BAAL_IMPLEMENTATION)


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
#endif
