#include <stdlib.h>
#include <stdio.h>

#if !defined(TODO)

#define TODO_STR2(arg) #arg
#define TODO_STR(arg) TODO_STR2(arg)

/*
    TODO(text, args)
    TODO("add this feature")
    TODO("fix function for number %d", 4)
*/
#define TODO(...)\
    do {\
        printf("\x1B[1;33mReached TODO at "__FILE__":"TODO_STR(__LINE__)"\n");\
        printf(__VA_ARGS__);\
        printf("\x1B[0m\n");\
        exit(1);\
    } while(0);\

#endif // TODO
