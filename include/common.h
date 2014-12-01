#ifndef common_h
#define common_h

#include "git.h"

#define REALLOC_DELTA 10

typedef struct str_thdata
{
    short int mutex;
    unsigned int count;
    struct git *g;
} thdata;

#endif
