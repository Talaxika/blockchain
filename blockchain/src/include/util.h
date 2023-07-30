#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define RET_CODE_ERROR 1
#define RET_CODE_SUCCESS 0

typedef uint32_t iResult;

typedef struct {
    uint32_t sen_id;
} sensor_info_t;


#endif /* UTIL_H */