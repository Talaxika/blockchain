#ifndef BROADCAST_H
#define BROADCAST_H

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define ERROR_RET 1
#define SUCCESS_RET 0

typedef uint32_t iResult;

#define DEFAULT_BUFLEN 16
#define ANSWER_LENGHT 1

#define DEFAULT_PORT "27016"

typedef enum
{
    TYPE_CHAR,   /** val=1 **/
    TYPE_INT,    /** val=2 **/
    TYPE_DOUBLE, /** val=3 **/

} DataType;

typedef struct {
    uint32_t sen_id;
} sensor_info_t;

typedef struct
{
    sensor_info_t sen_info;
    DataType type;
    uint32_t buf_len;
} header_cfg_t;

typedef struct
{
    struct addrinfo *result;
    SOCKET ListenSocket;
    SOCKET ClientSocket;
} conn_cfg_t;

iResult connect_open(conn_cfg_t *cfg);

char* connect_recieve(conn_cfg_t *cfg, header_cfg_t *hdr_cfg, uint32_t rotations);

iResult connect_close(conn_cfg_t *cfg);


#endif /* BROADCAST_H */