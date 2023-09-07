#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#define RET_CODE_ERROR   (1U)
#define RET_CODE_SUCCESS (0U)
#define RET_CODE_TIMEOUT (200U)

// #define PRINT_DEBUG
#define TIMEOUT_RECV (3000u) /* in ms */
#define MAX_HASH_SIZE (32U)

typedef uint32_t iResult;

typedef long unsigned int iResult_thread;

typedef struct {
    uint8_t base_mac_addr[6];
    float sen_temp;
    char cmd[6];
} sensor_info_t;

#endif /* UTIL_H */