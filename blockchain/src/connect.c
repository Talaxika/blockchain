#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include "include/blockchain.h"
#include "include/connect.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

// gcc server.c -o server.exe -lwsock32 -lWs2_32

iResult connect_open(conn_cfg_t *cfg)
{
    printf("Opened socket");
    iResult iRes = 0;
    WSADATA wsaData = {0};

    cfg->result = NULL;
    cfg->ListenSocket = INVALID_SOCKET;
    cfg->ClientSocket = INVALID_SOCKET;
    // struct addrinfo *result = cfg->result;
    
    struct addrinfo hints = {0};
    // Initialize Winsock
    iRes = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iRes != 0) {
        printf("WSAStartup failed with error: %d\n", iRes);
        return iRes;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iRes = getaddrinfo(NULL, DEFAULT_PORT, &hints, &cfg->result);
    if ( iRes != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iRes);
        WSACleanup();
        return iRes;
    }

    // Create a SOCKET for the server to listen for client connections.
    cfg->ListenSocket = socket(cfg->result->ai_family, cfg->result->ai_socktype, cfg->result->ai_protocol);
    if (cfg->ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(cfg->result);
        WSACleanup();
        return iRes;
    }
    // Setup the TCP listening socket
    iRes = bind( cfg->ListenSocket, cfg->result->ai_addr, (int)cfg->result->ai_addrlen);
    if (iRes == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(cfg->result);
        closesocket(cfg->ListenSocket);
        WSACleanup();
        return iRes;
    }

    freeaddrinfo(cfg->result);

    iRes = listen(cfg->ListenSocket, SOMAXCONN);
    if (iRes == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(cfg->ListenSocket);
        WSACleanup();
        return iRes;
    }

    // Accept a client socket
    cfg->ClientSocket = accept(cfg->ListenSocket, NULL, NULL);
    if (cfg->ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(cfg->ListenSocket);
        WSACleanup();
        return iRes;
    }

    // No longer need server socket
    closesocket(cfg->ListenSocket);
    printf("Socket is read to accept");
    return iRes;
}

char* connect_recieve(conn_cfg_t *cfg, header_cfg_t *hdr_cfg)
{
    
    iResult iRes = 0;    
    uint32_t iSendResult = 0;

    uint32_t recvbuflen = DEFAULT_BUFLEN;
    // hdr_cfg->data = malloc(sizeof(char) * recvbuflen);

    char* recv_buffer = malloc(sizeof(char) * DEFAULT_BUFLEN);
    // Receive until the peer shuts down the connection

    // TODO: make it that it sends a cfg/header, the this side returns once to tell it to send
    // the whole data
    do {

        iRes = recv(cfg->ClientSocket, (header_cfg_t*) hdr_cfg, sizeof(header_cfg_t), 0);
        // printf("There: %s\n", hdr_cfg->data);
        if (iRes > 0) {
            printf("Bytes received: %d\n", iRes);

        // Echo the buffer back to the sender
            iSendResult = send(cfg->ClientSocket, "1", (int) ANSWER_LENGHT, 0 );
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(cfg->ClientSocket);
                WSACleanup();
                return NULL;
            } else {
                iRes = recv(cfg->ClientSocket, &recv_buffer, hdr_cfg->buf_len, 0);
            }
        }
        else if (iRes == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(cfg->ClientSocket);
            WSACleanup();
            return NULL;
        }

    } while (iRes > 0);

    return recv_buffer;
}

iResult connect_close(conn_cfg_t *cfg)
{
    iResult iRes = ERROR;

    // shutdown the connection once we're done
    iRes = shutdown(cfg->ClientSocket, SD_SEND);
    if (iRes == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(cfg->ClientSocket);
        WSACleanup();
        return iRes;
    }

    // cleanup
    closesocket(cfg->ClientSocket);
    WSACleanup();

    return iRes;
}