#ifndef CONNECT_H
#define CONNECT_H

#include "util.h"

#define DEFAULT_BUFLEN 16
#define ANSWER_LENGHT 1

#define DEFAULT_PORT "27015"

typedef enum
{
    TYPE_CHAR,   /** val=1 **/
    TYPE_INT,    /** val=2 **/
    TYPE_DOUBLE, /** val=3 **/

} DataType;


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

#endif /* CONNECT_H */