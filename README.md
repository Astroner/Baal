# Hi There
This is STB-like library for block memory allocation.

> Note, that this library supports multiple blocks allocation, so if you don't need it, and just want to allocate single blocks, consider to use [Zeb](https://github.com/Astroner/Zeb) because it is more memory efficient and fast for this case.

# Include
As with any header only library, you need to include *Baal.h* library to your project and define **BAAL_IMPLEMENTATION** before **include** statement to include the implementation.

# Table of content
 - [General](#general)
 - [Creating instance](#create-instance)
     - [Macros](#macros)
     - [Functions](#functions)
 - [Allocate](#allocate)
     - [Single block](#single-block)
     - [Multiple blocks](#multiple-blocks)
 - [Reallocate](#reallocate)
 - [Clear](#clear)
 - [Debugging](#debugging)
     - [Print](#print)
     - [Memory snapshot](#memory-snapshot)

# General
This library based on an idea of fixed-size block memory allocation, but allows to allocate multiple blocks in sequence. **Block** is a basic unit of the library, but the library operates with **groups** of blocks which size you specify at the initialization. You cannot allocate less memory than group(but you can set group size to 1). Group size cannot be less than **BAAL_GROUP_MIN_SIZE**.

# Create instance
Create Baal instance with macros or with function.

## Macros
**Baal_define(NAME, BLOCK_LENGTH, GROUP_SIZE, GROUPS_NUMBER)**
 - NAME - **Baal** instance name
 - BLOCK_LENGTH - size of a single block
 - GROUP_SIZE - number of blocks in single group
 - GROUPS_NUMBER - total number of groups

This macro expands into several variable definitions and at the end it will define pointer to **Baal** instance with provided **name**
```c
#define BAAL_IMPLEMENTATION
#include "Baal.h"

Baal_define(globalAllocator, 10, 10, 10);

int main(void) {
    Baal_define(localAllocator, 10, 10, 10);

    // 10 groups consisting of 10 blocks of 10 bytes
    // 100 blocks in total
    // 1000 bytes

    return 0;
}
```

**Baal_defineStatic(NAME, BLOCK_LENGTH, GROUP_SIZE, GROUPS_NUMBER)** does the same thing with **Baal_define(NAME, BLOCK_LENGTH, GROUP_SIZE, GROUPS_NUMBER)** but adds modifier **static** to variable definitions.

After macro **Baal** instance has to be initialized. By default this library will check initialization status during memory allocation and if needed initialize it automatically, but to speed the process up, you can define **BAAL_NO_INIT_CHECK** before including the implementation, but in this case you have to initialize the instance py yourself with function __Baal* Baal_init(Baal* baal)__. So if you sure that you can initialize every macro defined instance then do it and get a small speed boost.

## Functions
__Baal* Baal_create(size_t blockLength, size_t groupSize, size_t groupsNumber)__
 - **returns** - **Baal** instance
 - **blockLength** - size of a single block
 - **groupSize** - number of blocks in single group
 - **groupsNumber** - total number of groups

By default this function uses **malloc** to allocate required memory from heap, but it can be overridden by defining **BAAL_STD_MALLOC** macro with desired function before including the implementation.

__void Baal_destroy(Baal*)__ function destroys provided **Baal** instance and frees allocated memory. Basically this function is just an alias for std **free()**, which can be replaced by defining **BAAL_STD_FREE** with desired function.

```c
#define BAAL_IMPLEMENTATION
#include "Baal.h"

int main(void) {
    Baal* baal = Baal_create(10, 10, 10);

    // Do smth

    Baal_destroy(baal);

    return 0;
}
```

__Baal* Baal_construct(Baal* baal, char* buffer, size_t bufferLength, size_t blockLength, size_t groupSize)__
 - **returns** - pointer to initialized **Baal** instance 
 - **baal** - empty **Baal** instance
 - **buffer** - memory region to manage
 - **bufferLength** - size of the memory region
 - **blockLength** - size of a single block in bytes
 - **groupSize** - number of blocks in a single group

Use this function if you want to manage predefined region of memory:
```c
#define BAAL_IMPLEMENTATION
#include "Baal.h"

int main(void) {
    char memory[Baal_getTotalMemorySize(10, 10, 10)];
    Baal baal;

    Baal_construct(&baal, memory, Baal_getTotalMemorySize(10, 10, 10), 10, 10);

    int* a = Baal_alloc(&baal);
    *a = 32;

    return 0;
}
```
As shown above, you can use macro **Baal_getTotalMemorySize** to get the exact size of memory required for specified **BLOCK_LENGTH/GROUP_SIZE/GROUPS_NUMBER**,
but it is not necessary.

# Allocate
Any memory allocated with **Baal** can be freed with __void Baal_free(Baal* baal, void* ptr)__ function.

## Single block
__void* Baal_alloc(Baal* baal)__
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

Baal_defineStatic(objects, sizeof(Obj), 1, 10);

int main(void) {
    Obj* obj = Baal_alloc(objects);

    obj->index = 2;
    obj->counter = 10;

    printf("Index: %d, Counter: %d\n", obj->index, obj->counter);

    Baal_free(objects, obj);

    return 0;
}
```

## Multiple blocks
__void* Baal_allocMany(Baal* baal, size_t blocksNumber)__
 - **returns** - **NULLABLE** - pointer to allocated memory
 - **baal** - **Baal** instance
 - **blocksNumber** - number of blocks to be allocated

This function allocates required number of blocks in a row and returns pointer to the first block. Returns **NULL** if it is not possible to allocate required sequence of blocks.

```c
#include <stdio.h>

#define BAAL_IMPLEMENTATION
#include "Baal.h"

int main(void) {
    Baal_define(baal, 1, 1, 20);

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

# Reallocate
__void* Baal_realloc(Baal* baal, void* ptr, size_t newBlocksNumber)__
 - **returns** - **NULLABLE** - new pointer to data
 - **baal** - **Baal** instance
 - **ptr** - pointer to allocated memory
 - **newBlocksNumber** - new number of blocks after reallocation

Sometimes you need to increase/decrease number of required blocks, for example when array length changes. To do so you don't need to allocate new chunk of memory and free previous one, you can just readjust existing memory with **Baal_realloc**. This function will try to extend currently allocated memory chunk, and if it is impossible it will reallocate to another memory chunk and return pointer to it. If reallocation fails then this function returns **NULL**, which means that reallocation failed but the data are still available.

```c
#include <stdio.h>
#include <assert.h>

#define BAAL_IMPLEMENTATION
#include "Baal.h"

int main(void) {
    Baal_define(baal, 1, 1, 20);

    char* str = Baal_allocMany(baal, 4);

    str[0] = 'H';
    str[1] = 'i';
    str[2] = '!';
    str[3] = '\0';

    printf("%s\n", str); // Hi!

    char* newStr = Baal_realloc(baal, str, 10);

    assert(newStr == str); // In this case we just extended already allocated chunk so the pointer is the same

    newStr[3] = ' ';
    newStr[4] = 'M';
    newStr[5] = 'o';
    newStr[6] = 'm';
    newStr[7] = '?';
    newStr[8] = '\0';

    printf("%s\n", str); // Hi! Mom?
    return 0;
}

```

This function uses different std functions that can be overridden by defining specific macros before including the implementation.

Table of overrides:
| Macro name       | Default value    |
|------------------|------------------|
| BAAL_STD_MEMCPY  | string's memcpy  |
| BAAL_STD_MEMMOVE | string's memmove |

Example:
```c

#define BAAL_STD_MEMCPY myMemCpyImplementation
#define BAAL_IMPLEMENTATION
#include "Baal.h"

int main(void) {
    Baal_define(baal, 1, 1, 20);

    char* a = Baal_alloc(baal);

    Baal_realloc(baal, a, 10);

    return 0;
}
```

# Clear
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

Baal_defineStatic(objects, sizeof(Obj), 1, 10);

int main(void) {
    Obj* obj = Baal_alloc(objects);

    obj->index = 2;
    obj->counter = 10;

    printf("Index: %d, Counter: %d\n", obj->index, obj->counter);

    Baal_clear(objects);

    return 0;
}
```

# Debugging
You need to define **BAAL_DEBUG** to use any of described in this section functions.

By default these functions use **printf()** to log data, but it can be overridden by defining **BAAL_STD_PRINT** with desired function before including the implementation

## Print
**Baal_print()** function prints current memory chunks
```c
#define BAAL_DEBUG
#define BAAL_IMPLEMENTATION
#include "Baal.h"

Baal_defineStatic(numbers, sizeof(int), 1, 10);

int main(void) {
    Baal_alloc(numbers);

    Baal_print(numbers);
    // [000]    BUSY_CHUNK    SIZE: 1
    // [001]    FREE_CHUNK    SIZE: 9

    Baal_clear(numbers);

    return 0;
}
```

## Memory snapshot
__Baal_memorySnapshot()__ function prints full **Baal** instance info
```c
#define BAAL_DEBUG
#define BAAL_IMPLEMENTATION
#include "Baal.h"

Baal_defineStatic(numbers, sizeof(int), 1, 2)

int main(void) {
    int* num = Baal_alloc(numbers);
    *num = 32;

    Baal_memorySnapshot(numbers);

    return 0;
}
/*
OUTPUT:

Total Memory Length: 32
Added Info Length: 8
Block Length: 8
Group Size: 1
Groups Number: 2
First Free Chunk: 0x10055c050
Bytes:
------------------------------------- GROUP START
0x10055c040    0x01
0x10055c041    0x00
0x10055c042    0x00
0x10055c043    0x00
0x10055c044    0x00
0x10055c045    0x00
0x10055c046    0x00
0x10055c047    0x00
--------------------- DATA START
0x10055c048    0x20    AS POINTER 0x20
0x10055c049    0x00
0x10055c04a    0x00
0x10055c04b    0x00
0x10055c04c    0x00
0x10055c04d    0x00
0x10055c04e    0x00
0x10055c04f    0x00
------------------------------------- GROUP START
0x10055c050    0x01
0x10055c051    0x00
0x10055c052    0x00
0x10055c053    0x00
0x10055c054    0x00
0x10055c055    0x00
0x10055c056    0x00
0x10055c057    0x00
--------------------- DATA START
0x10055c058    0x00    AS POINTER 0x0
0x10055c059    0x00
0x10055c05a    0x00
0x10055c05b    0x00
0x10055c05c    0x00
0x10055c05d    0x00
0x10055c05e    0x00
0x10055c05f    0x00
*/
```