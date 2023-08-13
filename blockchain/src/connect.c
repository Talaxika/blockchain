#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include "include/blockchain.h"
#include "include/connect.h"

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
        iRes = RET_CODE_ERROR;
        return iRes;
    }
    // Setup the TCP listening socket
    iRes = bind( cfg->ListenSocket, cfg->result->ai_addr, (int)cfg->result->ai_addrlen);
    if (iRes == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(cfg->result);
        connect_close(cfg);
        return iRes;
    }

    freeaddrinfo(cfg->result);

    iRes = listen(cfg->ListenSocket, SOMAXCONN);
    if (iRes == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        connect_close(cfg);
        return iRes;
    }

    printf("Waiting for connections...\n");

    return iRes;
}

char* connect_recieve(conn_cfg_t *cfg, header_cfg_t *hdr_cfg, uint32_t rotations)
{
    iResult iRes = RET_CODE_ERROR;
    uint32_t iSendResult = 0;

    char *recv_buffer = NULL;

    // Accept a client socket
    cfg->ClientSocket = accept(cfg->ListenSocket, NULL, NULL);
    if (cfg->ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        connect_close(cfg);
        return NULL;
    }
    printf("Socket accepted\n");

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
        printf("Sending answer: %s\n", send_char);
        iSendResult = send(cfg->ClientSocket, send_char, (int) ANSWER_LENGHT, 0 );
        if (iSendResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(cfg->ClientSocket);
            connect_close(cfg);
            return NULL;
        } else {
            recv_buffer = malloc(sizeof(char) * hdr_cfg->buf_len);
            iRes = recv(cfg->ClientSocket, recv_buffer, hdr_cfg->buf_len, 0);

            /* Because the recv function is a byte stream, it does not put the '\0' symbol.
             * It should be manually put*/
            recv_buffer[hdr_cfg->buf_len] = '\0';
            printf("Bytes received: %d, Data received: %s\n", iRes, recv_buffer);
        }
    }
    else if (iRes == 0)
        printf("Connection closing...\n");
    else  {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(cfg->ClientSocket);
        connect_close(cfg);
        return NULL;
    }

    iRes = shutdown(cfg->ClientSocket, SD_SEND);
    if (iRes == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(cfg->ClientSocket);
        WSACleanup();
        // return 1;
    }
    closesocket(cfg->ClientSocket);
    return recv_buffer;
}

iResult connect_close(conn_cfg_t *cfg)
{
    iResult iRes = RET_CODE_ERROR;

    // cleanup
    iRes = closesocket(cfg->ListenSocket);
    if (iRes == SOCKET_ERROR) {
        printf("Closing listening socket failed with error: %d\n", WSAGetLastError());
    }

    /* Terminates Windows Sockets operations for all threads */
    iRes = WSACleanup();
    if (iRes == SOCKET_ERROR) {
        printf("WSACleanup failed with error: %d\n", WSAGetLastError());
    }

    return iRes;
}

iResult send_broadcast_message(Blockchain *blockchain)
{
    iResult iRes = RET_CODE_SUCCESS;
    WSADATA wsaData = {0};
    iRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iRes != RET_CODE_SUCCESS) {
        fprintf(stderr, "WSAStartup failed\n");
        return iRes;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        fprintf(stderr, "socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        iRes = RET_CODE_ERROR;
        return iRes;
    }

    bool broadcastEnable = TRUE;
    iRes = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const char *)&broadcastEnable, sizeof(broadcastEnable));
    if (iRes == SOCKET_ERROR) {
        fprintf(stderr, "setsockopt failed with error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return iRes;
    }

    struct sockaddr_in broadcastAddr;
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS_LOCALHOST);
    broadcastAddr.sin_port = htons(BROADCAST_PORT);
    char message[] = "I want to join";

    iRes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
    if (iRes == SOCKET_ERROR) {
        fprintf(stderr, "sendto failed with error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return iRes;
    }

    struct sockaddr_in responseAddr;
    int responseAddrLen = sizeof(responseAddr);

    // Set a timeout for receiving responses
    struct timeval timeout;
    timeout.tv_sec = RESPONSE_TIMEOUT;
    timeout.tv_usec = 0;

    iRes = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    if (iRes < 0) {
        perror("setsockopt timeout");
        WSACleanup();
        return iRes;
    }

    char responseBuffer[sizeof(Blockchain)];
    uint64_t response_size = 0;

    int numBytes = recvfrom(sockfd, &response_size, sizeof(uint64_t), 0, (struct sockaddr *)&responseAddr, &responseAddrLen);
    if (numBytes == SOCKET_ERROR) {
        fprintf(stderr, "recvfrom failed or timed out\n");
        closesocket(sockfd);
        WSACleanup();
        return RET_CODE_TIMEOUT; // Return value indicating timeout
    } else {
        // Deserialize the received data into a structure
        numBytes = recvfrom(sockfd, blockchain, response_size, 0, (struct sockaddr *)&responseAddr, &responseAddrLen);
        print_blockchain(*blockchain);
        printf("Received response from %s:%d: %s\n", inet_ntoa(responseAddr.sin_addr), ntohs(responseAddr.sin_port), responseBuffer);
    }

    closesocket(sockfd);
    WSACleanup();
    return iRes;
}