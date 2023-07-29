#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

// gcc client.c -o client.exe -lwsock32 -lWs2_32; ./client.exe

#define DEFAULT_BUFLEN 16
#define DEFAULT_PORT "27015"
#define IP_ADDRESS_LOCALHOST "localhost"
#define IP_ADDRESS_IPv4 "192.168.1.6"

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
    SOCKET ConnectSocket;
} conn_cfg_t;

int connect_open(conn_cfg_t *cfg);
int conenct_send(conn_cfg_t *cfg);
int connect_close(conn_cfg_t *cfg);

int __cdecl main(int argc, char **argv)
{
    conn_cfg_t iCfg = {0};
    iCfg.ConnectSocket = INVALID_SOCKET;
    iCfg.result = NULL;

    connect_open(&iCfg);

    time_t t;
    time(&t);

    header_cfg_t hdr_cfg = {0};
    hdr_cfg.type = TYPE_INT;
    hdr_cfg.sen_info.sen_id = 111;

    char sendbuf[DEFAULT_BUFLEN] = "";
    snprintf(sendbuf, DEFAULT_BUFLEN, "%d", t);
    hdr_cfg.buf_len = strlen(sendbuf);

    char recvbuf[DEFAULT_BUFLEN];
    int iResult = 0;
    int recvbuflen = 1;

    while (true)
    {
        // Send an initial buffer
        iResult = send( iCfg.ConnectSocket, &hdr_cfg, (int) sizeof(header_cfg_t), 0 );
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            break;
        }

        printf("Bytes Sent: %ld\n", iResult);

        iResult = recv(iCfg.ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ) {
            printf("Bytes received: %d\n", iResult);
        } else if ( iResult == 0 ) {
            printf("Connection closed\n");
            break;
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            break;
        }
        if(strcmp(recvbuf, "1") == 0) {
            iResult = send( iCfg.ConnectSocket, sendbuf, (int) hdr_cfg.buf_len, 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                break;
            }
        } else if (strcmp(recvbuf, "2") == 0) {
            iResult = send( iCfg.ConnectSocket, sendbuf, (int) hdr_cfg.buf_len, 0);
            break;
        }
    }

    connect_close(&iCfg);

    return 0;
}

int connect_open(conn_cfg_t *cfg)
{
    int iResult = 0;
    WSADATA wsaData = {0};
    struct addrinfo *ptr = NULL,
                    hints = {0};
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(IP_ADDRESS_LOCALHOST, DEFAULT_PORT, &hints, &cfg->result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=cfg->result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        cfg->ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (cfg->ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( cfg->ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(cfg->ConnectSocket);
            cfg->ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(cfg->result);

    if (cfg->ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

}

int connect_close(conn_cfg_t *cfg)
{
    // shut down the connection, since we are done
    int iResult = shutdown(cfg->ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(cfg->ConnectSocket);
        WSACleanup();
        // return 1;
    }

    // cleanup
    closesocket(cfg->ConnectSocket);
    WSACleanup();
}