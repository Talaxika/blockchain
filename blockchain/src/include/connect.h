#ifndef CONNECT_H
#define CONNECT_H

#include "util.h"

#define DEFAULT_PORT 12233
#define BROADCAST_PORT 12345
#define MAX_TIMEOUT_THREAD (5U)

#define BCAST_ADDRESS "255.255.255.255"
#define BROADCAST_MESSAGE "Hello"

#define MESSAGE_ESP_SIZE (19u)
#define ESP32_REQ_SIZE (6u)

iResult connect_open(void);

iResult connect_close(void);

#endif /* CONNECT_H */