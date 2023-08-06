#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include "include/blockchain.h"
#include "include/connect.h"

// Need to link with Ws2_32.lib
// #pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

// gcc server.c -o server.exe -lwsock32 -lWs2_32

void handle_transaction(void* socketPtr) {
    SOCKET clientSocket = *((SOCKET*)socketPtr);
    free(socketPtr); // Clean up memory allocated for the socket pointer
    header_cfg_t hdr_cfg = {0};

    /* It is expected to first receive the header configuration which contains the sen info,
     * the buffer length and info type. */
    iResult iRes = recv(clientSocket, &hdr_cfg, sizeof(header_cfg_t), 0);
    char *recv_buffer = NULL;

    if (iRes > 0) {

        char send_char[2] = "1";
        printf("Bytes received: %d\n", iRes);
        printf("Buffer length: %d, Sensor ID: %d, Data Type: %d\n",
        hdr_cfg.buf_len,
        hdr_cfg.sen_info.sen_id,
        hdr_cfg.type);

        /* After the header is received and verified, 1 is echo-ed to the sender, to show him, that
         * he can send the data. */
        printf("Sending answer: %s\n", send_char);
        iResult iSendResult = send(clientSocket, send_char, (int) ANSWER_LENGHT, 0 );
        if (iSendResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            return NULL;
        } else {
            recv_buffer = malloc(sizeof(char) * hdr_cfg.buf_len);
            iRes = recv(clientSocket, recv_buffer, hdr_cfg.buf_len, 0);

            /* Because the recv function is a byte stream, it does not put the '\0' symbol.
             * It should be manually put*/
            recv_buffer[hdr_cfg.buf_len] = '\0';
            printf("Bytes received: %d, Data received: %s\n", iRes, recv_buffer);
        }
    }
    else if (iRes == 0) {
        printf("Connection closing...\n");
    }
    else {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }

    closesocket(clientSocket);
    return NULL;
}

iResult connect_open(conn_cfg_t *cfg)
{
    iResult iRes = RET_CODE_ERROR;
    WSADATA wsaData = {0};

    // Initialize Winsock
    iRes = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iRes != 0) {
        printf("WSAStartup failed with error: %d\n", iRes);
        return iRes;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(DEFAULT_PORT); // Change to your desired port

    iRes = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (iRes == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    iRes = listen(serverSocket, SOMAXCONN);
    if (iRes == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return iRes;
    }
    printf("Waiting for connections...\n");

    HANDLE threadHandle = NULL;

    while (1) {
        struct sockaddr_in clientAddress;
        int clientAddressLength = sizeof(clientAddress);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == INVALID_SOCKET) {
            fprintf(stderr, "accept failed\n");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

        // Create a new thread to handle the client
        SOCKET* socketPtr = (SOCKET*)malloc(sizeof(SOCKET));
        *socketPtr = clientSocket;

        // Start a new thread to handle the client using CreateThread()
        threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_transaction, (LPVOID)socketPtr, 0, NULL);
        if (threadHandle == NULL) {
            fprintf(stderr, "CreateThread failed\n");
            closesocket(clientSocket);
            continue;
        }

        // Close the thread handle as we won't need it anymore (thread will continue to run)
        CloseHandle(threadHandle);
    }

    WaitForSingleObject(threadHandle, INFINITE);

    closesocket(serverSocket);
    WSACleanup();

    return iRes;
}

// char* connect_recieve(conn_cfg_t *cfg, header_cfg_t *hdr_cfg, uint32_t rotations)
// {
//     iResult iRes = RET_CODE_ERROR;
//     uint32_t iSendResult = 0;

//     char *recv_buffer = NULL;

//     /* It is expected to first receive the header configuration which contains the sen info,
//      * the buffer length and info type. */
//     iRes = recv(cfg->ClientSocket, hdr_cfg, sizeof(header_cfg_t), 0);

//     if (iRes > 0) {

//         char send_char[2] = "1";
//         printf("Bytes received: %d\n", iRes);
//         printf("Buffer length: %d, Sensor ID: %d, Data Type: %d\n",
//         hdr_cfg->buf_len,
//         hdr_cfg->sen_info.sen_id,
//         hdr_cfg->type);

//         /* After the header is received and verified, 1 is echo-ed to the sender, to show him, that
//          * he can send the data. */
//         if (rotations == 1) {
//             strncpy(send_char, "2", 2);
//         }
//         printf("Sending answer: %s\n", send_char);
//         iSendResult = send(cfg->ClientSocket, send_char, (int) ANSWER_LENGHT, 0 );
//         if (iSendResult == SOCKET_ERROR) {
//             printf("send failed with error: %d\n", WSAGetLastError());
//             connect_close(cfg);
//             return NULL;
//         } else {
//             recv_buffer = malloc(sizeof(char) * hdr_cfg->buf_len);
//             iRes = recv(cfg->ClientSocket, recv_buffer, hdr_cfg->buf_len, 0);

//             /* Because the recv function is a byte stream, it does not put the '\0' symbol.
//              * It should be manually put*/
//             recv_buffer[hdr_cfg->buf_len] = '\0';
//             printf("Bytes received: %d, Data received: %s\n", iRes, recv_buffer);
//         }
//     }
//     else if (iRes == 0)
//         printf("Connection closing...\n");
//     else  {
//         printf("recv failed with error: %d\n", WSAGetLastError());
//         connect_close(cfg);
//         return NULL;
//     }


//     return recv_buffer;
// }

iResult connect_close(conn_cfg_t *cfg)
{
    iResult iRes = RET_CODE_ERROR;

    // shutdown the connection once we're done
    iRes = shutdown(cfg->ClientSocket, SD_SEND);
    if (iRes == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
    }

    // cleanup
    iRes = closesocket(cfg->ListenSocket);
    if (iRes == SOCKET_ERROR) {
        printf("Closing listening socket failed with error: %d\n", WSAGetLastError());
    }

    iRes = closesocket(cfg->ClientSocket);
    if (iRes == SOCKET_ERROR) {
        printf("Closing client socket failed with error: %d\n", WSAGetLastError());
    }

    /* Terminates Windows Sockets operations for all threads */
    iRes = WSACleanup();
    if (iRes == SOCKET_ERROR) {
        printf("WSACleanup failed with error: %d\n", WSAGetLastError());
    }

    return iRes;
}