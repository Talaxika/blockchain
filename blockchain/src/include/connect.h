#ifndef CONNECT_H
#define CONNECT_H

#include "util.h"

#define IP_ADDRESS_LOCALHOST "localhost"
#define IP_ADDRESS_IPv4 "192.168.1.6"
#define DEFAULT_PORT "27015"
#define BROADCAST_PORT 12345
#define RESPONSE_TIMEOUT "5000" // Response timeout in seconds
#define MAX_TIMEOUT (5U)

#define BCAST_ADDRESS "255.255.255.255"
/* za prashtane do vsichki, po dobra versiq na ipv4*/

#define BROADCAST_MESSAGE "Hello"
#define DEFAULT_BUFLEN 16
#define ANSWER_LENGHT 2

typedef struct
{
    sensor_info_t sen_info;
    uint32_t buf_len;
} header_cfg_t;

typedef struct
{
    struct addrinfo *result;
    SOCKET ListenSocket;
    SOCKET ClientSocket;
} conn_cfg_t;

iResult udp_server_receive(conn_cfg_t *cfg, sensor_info_t *sen_info, uint32_t rotations);

iResult connect_open(conn_cfg_t *cfg);

char* connect_recieve(conn_cfg_t *cfg, header_cfg_t *hdr_cfg, uint32_t rotations);

iResult connect_close(conn_cfg_t *cfg);

#endif /* CONNECT_H */