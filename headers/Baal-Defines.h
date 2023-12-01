#if !defined(BAAL_DEFINES_H)
#define BAAL_DEFINES_H

#if !defined(BAAL_STD_MEMCPY)
    #include <string.h>
    #define BAAL_STD_MEMCPY memcpy
#endif // 

#if !defined(BAAL_STD_MEMMOVE)
    #include <string.h>
    #define BAAL_STD_MEMMOVE memmove
#endif // BAAL_STD_MEMMOVE

#if !defined(BAAL_STD_MALLOC)
    #include <stdlib.h>
    #define BAAL_STD_MALLOC malloc
#endif

#if !defined(BAAL_STD_FREE)
    #include <stdlib.h>
    #define BAAL_STD_FREE free
#endif

#if !defined(BAAL_STD_PRINT)
    #include <stdio.h>
    #define BAAL_STD_PRINT printf
#endif // BAAL_STD_PRINT


#endif // BAAL_DEFINES_H
