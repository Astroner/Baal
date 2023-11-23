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
```