#include <stdlib.h>
#include <stdio.h>

#if !defined(TODO)

#define TODO(text)\
    do {\
        printf("Reached TODO at %s:%d\n%s\n", __FILE__, __LINE__, text);\
        exit(1);\
    } while(0);\

#endif // TODO
