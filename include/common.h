#ifndef common_h
#define common_h

#include "git.h"

#define REALLOC_DELTA 10
#define TIMEOUT_DELTA 60

// bit field mask
#define MASK_LOOP               (1 << 0) // 0000 0001 = 1
#define MASK_DEBUG              (1 << 1) // 0000 0010 = 2
#define MASK_FETCH_AUTO         (1 << 2) // 0000 0100 = 4
#define MASK_FETCH_CREDENTIALS  (1 << 3) // 0000 1000 = 8
#define MASK_UPDATE_INDICATOR   (1 << 4) // 0001 0000 = 16
#define MASK_APPEND_OSD         (1 << 5) // 0010 0000 = 32
#define MASK_ASREAD_OSD         (1 << 6) // 0100 0000 = 64

typedef struct str_thdata
{
    unsigned int bitprop;
    short int mutex;
    unsigned int count;
    struct git *g;
} thdata;

#endif
