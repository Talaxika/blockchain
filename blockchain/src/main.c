#include "include/blockchain.h"
#include "include/connect.h"
#include "include/sha256.h"
#include "include/fileIO.h"

#define ROTATIONS_BLK (4)
#define ROTATIONS_TRX (2U)

#define USE_CONNECTION
#define USE_MINING

#define MAX_DEVICES (3u)

/*=========================== Local Typedefs ===========================*/
/*======================================================================*/

/*==================== Global Variable Declarations ====================*/
Blockchain iBlockchain = {0};
block_t block = {0};
conn_cfg_t iCfg;
HANDLE  thread_calc,
        thread_recv,
        thread_bcast,
        mutex;
HANDLE thread_device[MAX_DEVICES];

transaction_t local_transactions[MAX_TRANSACTIONS_SIZE];
volatile int stop_listening = 0;
volatile bool stop_receiving = false;
/* Make a function pointer so that it's more flexible.
 * It will be equal to the recv function*/
// char* (*recv_cb)(conn_cfg_t *cfg, header_cfg_t *hdr_cfg, uint32_t rotations);
/*======================================================================*/

/*==================== Global Function Declarations ====================*/

/* Main initializing function.
 *  - Opens the sockets for connection.
 *  - Initializes the blockchain
 *
 **/
iResult main_open(void);

/* Main receiving function.
 *  - This is the receive function that is ran from thread_recv.
 *  - It's purpose is to receive, while the thread_calc (main_mine)
 *  function is generating the block. The goal is to achieve parallelism.
 *  - Everything about receiveing from a socket is collected here, to ease
 *  running it through a thread.
 **/
iResult_thread WINAPI main_recv(void *);

/* Main mining function.
 *  - This is the mine function that is ran from thread_mine.
 *  - It's purpose is to mine, while the thread_recv (main_recv)
 *  function is receiving. The goal is to achieve parallelism.
 *  - Everything about mining a block and filling it's header structure,
 *  except the transactions, which are generated fro main-recv,
 *  is generated from this function.
 *  - After the main_mine and main_recv functions are completed
 *  successfully, the process of block generation is successful.
 **/
iResult_thread WINAPI main_mine(void *);

/* Main broadcasting function.
 *  - This is the broadcasting function that is ran from thread_bcast.
 *  - If this is the first computer, that joined the blockchain, it will
 *  generate the blockchain, otherwise it will wait for the blockchain to
 *  be sent to it. Then it starts waiting other PC's, and will be the sender.
 *  The goal is to achieve parallelism. While the block generation and
 *  transactions are being done, this will be the synchronisation between PC's.
 **/
iResult_thread WINAPI main_bcast(void *);

/* Supportive broadcasting function.
 *  Main function, called from main_bcast(). It is defined inside the main file,
 *  because we need the "stop_listening" variable. The purpose is signal the function
 *  that the program is ready to be finished.
 **/
iResult start_upd_broadcast_listener();

/* Main de-initializing function.
 *  - Closes the sockets.
 *  - Prints the blockchain
 *
 **/
iResult main_close(void);
/*======================================================================*/


int main(int argc, char* argv[]) {

    iResult iRes_main = RET_CODE_ERROR;

    iRes_main = send_broadcast_message(&iBlockchain);
    if (iRes_main == RET_CODE_TIMEOUT) {
        printf("First member, generating blockchain\n");
        if((iRes_main = main_open()) != RET_CODE_SUCCESS) {
            goto MAIN_END;
        }
    } else if (iRes_main == RET_CODE_ERROR) {
        goto MAIN_END;
    }

    thread_bcast = CreateThread(NULL, 0, main_bcast, NULL, 0, NULL);
    if (thread_bcast == NULL || thread_bcast == INVALID_HANDLE_VALUE) {
        printf("CreateThread error: %d\n", GetLastError());
        return 1;
    }

#ifdef USE_CONNECTION
    /* Open the socket and start listening */
    if((iRes_main = connect_open()) != RET_CODE_SUCCESS) {
        printf("%s(): Unsuccessful Socket initialization", __func__);
        goto MAIN_END;
    }
#endif /* USE_CONNECTION */
    // Check what is done by other blockchains
    // TODO: if first generate, else wait to recieve BC
    /******SETUP*******/

    // print_blockchain(iBlockchain);

    mutex = CreateMutex(NULL, FALSE, NULL);

    if (mutex == NULL) {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }

    /******Begin processes, each rotation is one block added*******/
    uint32_t rotations_BLK = ROTATIONS_BLK;

#ifdef USE_MINING
    while (rotations_BLK > 0) {
        iResult_thread iRes_mine = 0;
        iResult_thread iRes_recv = 0;
        do
        {
            Sleep(500);
            thread_calc = CreateThread(NULL, 0x0, main_mine, NULL, 0, NULL);
            if (thread_calc == NULL || thread_calc == INVALID_HANDLE_VALUE) {
                printf("CreateThread error: %d\n", GetLastError());
                return 1;
            }
            stop_receiving = false;

#ifdef USE_CONNECTION
            thread_recv = CreateThread(NULL, 0, main_recv, NULL, 0, NULL);

            if (thread_recv == NULL || thread_recv == INVALID_HANDLE_VALUE) {
                printf("CreateThread error: %d\n", GetLastError());
                return 1;
            }
            WaitForSingleObject(thread_calc, INFINITE);
            WaitForMultipleObjects(MAX_DEVICES, thread_device, TRUE, INFINITE);
            stop_receiving = true;
            // printf("%d", GetLastError());
#endif
        } while (WaitForSingleObject(thread_recv, INFINITE));
        // printf("%d", GetLastError());
        iBlockchain.num_blocks++;
        rotations_BLK--;
    }
#endif

MAIN_END:

    /******Clean up, print, etc...*******/
    // Verify hashes of other blocks
    iRes_main = main_close();
    // periodically write in file
    return 0;
} /* main() */


/*==================== Global Function Definitions ====================*/
iResult main_open(void)
{
    iResult iResult = RET_CODE_SUCCESS;

    /* Initiate the genesis block */
    if((iResult = initializeFirstBlock(&iBlockchain)) != RET_CODE_SUCCESS) {
        printf("%s(): Unsuccessful Block initialization", __func__);
        goto END;
    }



END:
    return iResult;
} /* main_open() */



#ifdef USE_CONNECTION
iResult_thread main_recv(void *)
{
    iResult_thread iRes = RET_CODE_ERROR;

    printf("Started connection operation\n");

    /* TODO: make threads accept and handle clients, the mutex should be used there */


    header_cfg_t iHdr_cfg = {0};
    uint32_t rotations_TRX = ROTATIONS_TRX;
    memset(&local_transactions, 0, sizeof(transaction_t));

    /* deosnt work as expected*/
    do
    {
        for (int i = 0; i < MAX_DEVICES; i++) {
            thread_device[i] = (HANDLE)_beginthreadex(NULL, 0, &udp_server_receive, NULL, 0, NULL);
        }
    } while (stop_receiving == false);

    //         if (! ReleaseMutex(mutex))
    //         {
    //             // Handle error.
    //         }
    //         break;

    //     // The thread got ownership of an abandoned mutex
    //     case WAIT_ABANDONED:
    //         return FALSE;
    // }
    printf("Done connection operation\n");
    return iRes;
} /* main_recv() */
#endif


#ifdef USE_MINING
iResult_thread main_mine(void *)
{
    iResult_thread iRes = RET_CODE_SUCCESS;
    printf("Started mining operation\n");
    Sleep(BLOCK_GENERATION_TIME);

    build_and_verify_block(&iBlockchain);
    iRes = mine_block(&iBlockchain.blocks[iBlockchain.num_blocks]);

    printf("Done mining operation\n");
    return iRes;
} /* main_mine() */
#endif

iResult_thread WINAPI main_bcast(void *)
{
    iResult_thread iRes = start_upd_broadcast_listener(&iBlockchain);
    return iRes;
}

iResult main_close(void)
{
    iResult iResult = RET_CODE_SUCCESS;

#if 1
    print_blockchain(iBlockchain);
    write_to_file(iBlockchain);
#endif

    stop_listening = 1;
    WaitForSingleObject(thread_bcast, MAX_TIMEOUT);
    CloseHandle(thread_calc);
    CloseHandle(thread_recv);
    CloseHandle(thread_bcast);
    CloseHandle(mutex);

#ifdef USE_CONNECTION
    /* Shutdown connections, close existing sockets */
    iResult = connect_close();
#endif

    return iResult;
} /* main_close() */
/*=====================================================================*/

iResult start_upd_broadcast_listener()
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
        fprintf(stderr, "socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        iRes = RET_CODE_ERROR;
        return iRes;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(BROADCAST_PORT);

    iRes = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (iRes == SOCKET_ERROR) {
        fprintf(stderr, "bind failed with error: %ld\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return iRes;
    }
    
    char buffer[1024] = {0};
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (!stop_listening) {
        int numBytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (numBytes == SOCKET_ERROR) {
            fprintf(stderr, "recvfrom failed with error: %ld\n", WSAGetLastError());
            closesocket(sockfd);
            WSACleanup();
            iRes = RET_CODE_ERROR;
            return iRes;
        }
        WaitForSingleObject(thread_recv, INFINITE);
        uint32_t dwCount=0, dwWaitResult = 0;
        dwWaitResult = WaitForSingleObject(mutex, INFINITE);
        if (dwWaitResult == WAIT_OBJECT_0)
        {
            buffer[numBytes] = '\0';
            printf("Received message from %s:%d: %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buffer);

            // Send a response back
            // Serialize the structure before sending
            // char serializedData[sizeof(Blockchain)];
            // memcpy(serializedData, blockchain, sizeof(Blockchain));
            uint64_t send_size = (uint64_t) sizeof(iBlockchain);
            printf("Sending blockchain.\n");
            iRes = sendto(sockfd, &send_size, sizeof(uint64_t), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
            if (iRes == SOCKET_ERROR) {
                fprintf(stderr, "sendto failed with error: %ld\n", WSAGetLastError());
                closesocket(sockfd);
                WSACleanup();
                return 1;
            } else {
                iRes = sendto(sockfd, &iBlockchain, send_size, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
                if (iRes == SOCKET_ERROR) {
                    fprintf(stderr, "sendto failed with error: %ld\n", WSAGetLastError());
                    closesocket(sockfd);
                    WSACleanup();
                    return 1;
                }
            }
            ReleaseMutex(mutex);
        }
    }
    closesocket(sockfd);
    WSACleanup();
    return 0;
}

iResult udp_server_receive()
{
    char rx_buffer[20] = {0};
    int addr_family = AF_INET;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(DEFAULT_PORT);
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
#ifdef PRINT_UDP
    if (sock < 0) {
        printf("Unable to create socket: errno %d", errno);
        break;
    }
    printf("Socket created");
#endif

    // Set timeout
    // struct timeval timeout;
    // timeout.tv_sec = 10;
    // timeout.tv_usec = 0;
    uint32_t timeout = 3000;
    
    int err = 0;
    err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
#ifdef PRINT_UDP
    if (err < 0) {
        printf("Socket unable to bind: errno %d", errno);
    }
    printf("Socket bound, port %d", PORT);
#endif
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
        printf("setsocketopt error: %d", WSAGetLastError());
    }
    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);

    while (stop_receiving == false) {
        int len = 0;

        len = recvfrom(sock, rx_buffer, ESP32_REQ_SIZE - 1, 0, (struct sockaddr *)&source_addr, &socklen);

        // Error occurred during receiving
        if (len < 0) {
#ifdef PRINT_UDP
            printf("recvfrom failed: errno %d", WSAGetLastError());
#endif
            break;
        }
        uint32_t dwCount=0, dwWaitResult = 0;
        dwWaitResult = WaitForSingleObject(mutex, INFINITE);
        if (dwWaitResult == WAIT_OBJECT_0)
        {
            rx_buffer[ESP32_REQ_SIZE] = '\0'; // Null-terminate whatever we received and treat like a string
            printf("Received request: %s\n", rx_buffer);
            if (strncmp(rx_buffer, "ESP32", ESP32_REQ_SIZE) != 0) {
                printf("Unknown request %s, no permission given\n", rx_buffer);
                break;
            } else {
                sensor_info_t sen_info = {0};
                len = recvfrom(sock, &sen_info, sizeof(sensor_info_t),
                                    0, (struct sockaddr *)&source_addr, &socklen);
                if (len < 0) {
    // #ifdef PRINT_UDP
                printf("recvfrom failed: errno %d\n",  WSAGetLastError());
    // #endif
                break;
                }
                // Data received
                else {
                printf("Transaction number: %d\n", iBlockchain.blocks[iBlockchain.num_blocks].num_transactions);
                printf("Sensor temperature: %f Sensor mac address: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
                    sen_info.sen_temp,
                    sen_info.base_mac_addr[0],
                    sen_info.base_mac_addr[1],
                    sen_info.base_mac_addr[2],
                    sen_info.base_mac_addr[3],
                    sen_info.base_mac_addr[4],
                    sen_info.base_mac_addr[5]);
                    add_transaction(&iBlockchain.blocks[iBlockchain.num_blocks], &sen_info);
                }
            }
            ReleaseMutex(mutex);
        }
    }

    if (sock != -1) {
#ifdef PRINT_UDP
        printf("Shutting down socket and restarting...");
#endif
        closesocket(sock);
        return 0;
    }
    return 1;
}