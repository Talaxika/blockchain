#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include "include/blockchain.h"
#include "include/connect.h"

static const char *join_message = "I want to join";

iResult connect_open(void)
{
    iResult iRes = RET_CODE_ERROR;
    WSADATA wsaData = {0};

    // Initialize Winsock
    iRes = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iRes != 0) {
        printf("WSAStartup failed with error: %d\n", iRes);
        return iRes;
    }
    return iRes;
}

iResult connect_close(void)
{
    iResult iRes = RET_CODE_ERROR;

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
    }
    struct sockaddr_in broadcastAddr = {0};
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr(BCAST_ADDRESS);
    broadcastAddr.sin_port = htons(BROADCAST_PORT);

    iRes = sendto(sockfd, join_message, 15, 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
    if (iRes == SOCKET_ERROR) {
        fprintf(stderr, "sendto failed with error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return iRes;
    }

    struct sockaddr_in responseAddr = {0};

    // Set a timeout for receiving responses
    uint32_t timeout = 4000;

    iRes = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    if (iRes < 0) {
        perror("setsockopt timeout");
        WSACleanup();
        return iRes;
    }

    char responseBuffer[sizeof(Blockchain)];
    uint64_t response_size = 0;

    int numBytes = recv(sockfd, &response_size, sizeof(uint64_t), 0);
    if (numBytes == SOCKET_ERROR) {
        if (WSAGetLastError() == 10060) {
            printf("Didn't receive blockchain\n");
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }
        closesocket(sockfd);
        WSACleanup();
        return RET_CODE_TIMEOUT; // Return value indicating timeout
    } else {
        // Deserialize the received data into a structure
        if (response_size != 0) {
            numBytes = recv(sockfd, blockchain, response_size, 0);
            if (numBytes == SOCKET_ERROR) {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(sockfd);
                WSACleanup();
                return RET_CODE_TIMEOUT; // Return value indicating timeout
            }
            print_blockchain(*blockchain);
            printf("Received response from %s:%d: %s\n", inet_ntoa(responseAddr.sin_addr), ntohs(responseAddr.sin_port), responseBuffer);
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return iRes;
}