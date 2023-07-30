#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include "broadcast.h"

// Need to link with Ws2_32.lib
// #pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

// gcc server.c -o server.exe -lwsock32 -lWs2_32

iResult connect_open(conn_cfg_t *cfg)
{
    iResult iRes = RET_CODE_ERROR;
    WSADATA wsaData = {0};

    /* Prepare configuration */
    cfg->result = NULL;
    cfg->ListenSocket = INVALID_SOCKET;
    cfg->ClientSocket = INVALID_SOCKET;

    // Initialize Winsock
    iRes = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iRes != 0) {
        printf("WSAStartup failed with error: %d\n", iRes);
        return iRes;
    }

    /* Prepare hints */
    struct addrinfo hints = {0};
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iRes = getaddrinfo(NULL, DEFAULT_PORT, &hints, &cfg->result);
    if ( iRes != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iRes);
        WSACleanup();
        return iRes;
    }

    // Create a SOCKET for the server to listen for client connections.
    cfg->ListenSocket = WSASocket(AF_INET, SOCK_RAW, IPPROTO_UDP, NULL, 0, 0);
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

    printf("Socket is ready to accept\n");

    return iRes;
}

char* connect_recieve(conn_cfg_t *cfg, header_cfg_t *hdr_cfg, uint32_t rotations)
{

    iResult iRes = RET_CODE_ERROR;
    uint32_t iSendResult = 0;


    // hdr_cfg->data = malloc(sizeof(char) * recvbuflen);
    // Receive until the peer shuts down the connection

    // TODO: make it that it sends a cfg/header, the this side returns once to tell it to send
    // the whole data
    char *recv_buffer = NULL;

    /* It is expected to first receive the header configuration which contains the sen info,
     * the buffer length and info type. */
    iRes = recv(cfg->ClientSocket, hdr_cfg, sizeof(header_cfg_t), 0);

    if (iRes > 0) {

        char send_char[2] = "1";
        printf("Bytes received: %d\n", iRes);
        printf("Buffer length: %d, Sensor ID: %d, Data Type: %d\n",
        hdr_cfg->buf_len,
        hdr_cfg->sen_info.sen_id,
        hdr_cfg->type);

        /* After the header is received and verified, 1 is echo-ed to the sender, to show him, that
         * he can send the data. */
        if (rotations == 1) {
            strncpy(send_char, "2", 2);
        }

        iSendResult = send(cfg->ClientSocket, send_char, (int) ANSWER_LENGHT, 0 );
        if (iSendResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(cfg->ClientSocket);
            WSACleanup();
            return NULL;
        } else {
            recv_buffer = malloc(sizeof(char) * hdr_cfg->buf_len);
            iRes = recv(cfg->ClientSocket, recv_buffer, hdr_cfg->buf_len, 0);

            /* Because the recv function is a byte stream, it does not put the '\0' symbol.
             * It should be manually put*/
            recv_buffer[hdr_cfg->buf_len] = '\0';
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


    return recv_buffer;
}

iResult connect_close(conn_cfg_t *cfg)
{
    iResult iRes = RET_CODE_ERROR;

    // shutdown the connection once we're done
    iRes = shutdown(cfg->ClientSocket, SD_SEND);
    if (iRes == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(cfg->ClientSocket);
        WSACleanup();
        return iRes;
    }

    // cleanup
    closesocket(cfg->ListenSocket);
    closesocket(cfg->ClientSocket);
    WSACleanup();

    return iRes;
}

int main(void) {
    conn_cfg_t iCfg = {0};
    connect_open(&iCfg);

}