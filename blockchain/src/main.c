#include "include/blockchain.h"
#include "include/connect.h"
#include "include/fileIO.h"

#define ROTATIONS_BLK (2)

#define USE_CONNECTION
#define USE_MINING

#define MAX_DEVICES (3u)

/*=========================== Local Typedefs ===========================*/
/*======================================================================*/

/*==================== Global Variable Declarations ====================*/
Blockchain iBlockchain = {0};
block_t block = {0};
HANDLE  thread_calc,
        thread_recv,
        thread_bcast,
        mutex;
HANDLE thread_device[MAX_DEVICES];

transaction_t local_transactions[MAX_TRANSACTIONS_SIZE];
volatile int stop_listening = 0;
volatile bool stop_receiving = false;
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

/* Supportive receiving function.
 *  Main function, called from main_recv(). It is defined inside the main file,
 *  because the mutexes and threads are needed to ease its use and synchronization.
 *  The purpose is signal the function that the program is ready to be finished.
 **/
iResult_thread udp_server_receive(void *);

/* Main mining function.
 *  - This is the mine function that is ran from thread_mine.
 *  - It's purpose is to mine, while the thread_recv (main_recv)
 *  function is receiving. The goal is to achieve parallelism.
 *  - Everything about mining a block and filling it's header structure,
 *  except the transactions, which are generated from main_recv,
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
 *  because the "stop_listening" variable is needed. The purpose is signal the function
 *  that the program is ready to be finished.
 **/
iResult start_upd_broadcast_listener(void);

/* Main de-initializing function.
 *  - Closes the conenctions.
 *  - Prints the blockchain
 *  - Writes the output in a file
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
        printf("CreateThread error when creating broadcast thread: %d\n", GetLastError());
        return 1;
    }

    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }

    /* Begin processes, each rotation is one block added */
    uint32_t rotations_BLK = ROTATIONS_BLK;
    while (rotations_BLK > 0) {
        do
        {
            thread_calc = CreateThread(NULL, 0x0, main_mine, NULL, 0, NULL);
            if (thread_calc == NULL || thread_calc == INVALID_HANDLE_VALUE) {
                printf("CreateThread error: %d\n", GetLastError());
                return 1;
            }
            stop_receiving = false;

            thread_recv = CreateThread(NULL, 0, main_recv, NULL, 0, NULL);

            if (thread_recv == NULL || thread_recv == INVALID_HANDLE_VALUE) {
                printf("CreateThread error: %d\n", GetLastError());
                return 1;
            }
            WaitForSingleObject(thread_calc, INFINITE);
            WaitForMultipleObjects(MAX_DEVICES, thread_device, TRUE, INFINITE);
            stop_receiving = true;
        } while (WaitForSingleObject(thread_recv, INFINITE));
        iBlockchain.blocks[iBlockchain.num_blocks].current_hash = hash_on_transactions(iBlockchain.blocks[iBlockchain.num_blocks]);
        iBlockchain.num_blocks++;
        rotations_BLK--;
    }

MAIN_END:
    /******Clean up, print, etc...*******/
    iRes_main = main_close();

    return iRes_main;
} /* main(void) */


/*==================== Global Function Definitions ====================*/
iResult main_open(void)
{
    iResult iResult = RET_CODE_SUCCESS;

    /* Initiate the genesis block */
    if((iResult = initializeFirstBlock(&iBlockchain)) != RET_CODE_SUCCESS) {
        printf("%s(): Unsuccessful Blockchain initialization", __func__);
        goto END;
    }

    if((iResult = connect_open()) != RET_CODE_SUCCESS) {
        printf("%s(): Unsuccessful connection initialization", __func__);
        goto END;
    }

END:
    return iResult;
} /* main_open(void) */



#ifdef USE_CONNECTION
iResult_thread main_recv(void *)
{
    iResult_thread iRes = RET_CODE_ERROR;
    printf("Started connection operation\n");
    memset(&local_transactions, 0, sizeof(transaction_t));
    do
    {
        for (int i = 0; i < MAX_DEVICES; i++) {
            thread_device[i] = CreateThread(NULL, 0, udp_server_receive, NULL, 0, NULL);
        }
    } while (stop_receiving == false);
    printf("Done connection operation\n");
    return iRes;
} /* main_recv(void *) */
#endif


#ifdef USE_MINING
iResult_thread main_mine(void *)
{
    iResult_thread iRes = RET_CODE_SUCCESS;
    printf("Started mining operation\n");
    /* "Sleep" and an easy hashing algorithms are used, instead of time
     * consuming and complex hashing algorithm tha mines blocks.*/
    Sleep(BLOCK_GENERATION_TIME);

    build_and_verify_block(&iBlockchain);
    
    /* First a block hash is generated based on timestamp,
     * and if transactions have been completed, hash will be re-generated
     * based on them for more security.*/
    // iRes = mine_block(&iBlockchain.blocks[iBlockchain.num_blocks]);

    printf("Done mining operation\n");
    return iRes;
} /* main_mine(void *) */
#endif

iResult_thread WINAPI main_bcast(void *)
{
    iResult_thread iRes = start_upd_broadcast_listener();
    return iRes;
} /* main_bcast(void *) */

iResult main_close(void)
{
    iResult iResult = RET_CODE_SUCCESS;

    stop_listening = 1;
    WaitForSingleObject(thread_bcast, MAX_TIMEOUT_THREAD);
    CloseHandle(thread_calc);
    CloseHandle(thread_recv);
    CloseHandle(thread_bcast);
    CloseHandle(mutex);

    print_blockchain(iBlockchain);
    write_to_file(iBlockchain);

#ifdef USE_CONNECTION
    /* Shutdown connections, close existing sockets */
    iResult = connect_close();
#endif

    return iResult;
} /* main_close(void) */

iResult start_upd_broadcast_listener(void)
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
} /* start_upd_broadcast_listener(void) */

iResult_thread udp_server_receive(void *)
{
    iResult_thread iRes = RET_CODE_SUCCESS;
    char rx_buffer[20] = {0};
    int addr_family = AF_INET;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(DEFAULT_PORT);
    ip_protocol = IPPROTO_IP;

    SOCKET sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    int err = 0;
    if (sock == SOCKET_ERROR) {
        // printf("Unable to create socket: error %d", WSAGetLastError());
    }
#ifdef PRINT_DEBUG
    printf("Socket created\n");
#endif

    // Set timeout
    uint32_t timeout = 3000;
    err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    if (err == SOCKET_ERROR) {
        // printf("Socket unable to bind: error %d", WSAGetLastError());
    }
#ifdef PRINT_DEBUG
    printf("Socket bound, port %d", DEFAULT_PORT);
#endif
    err = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    if (err == SOCKET_ERROR) {
#ifdef PRINT_DEBUG
        printf("setsocketopt error: %d", WSAGetLastError());
#endif
    }
    struct sockaddr_storage source_addr;
    socklen_t socklen = sizeof(source_addr);
    do {
        sensor_info_t sen_info = {0};
        int len = recvfrom(sock, &sen_info, sizeof(sensor_info_t),
                                0, (struct sockaddr *)&source_addr, &socklen);

        if (len < 0) {
// #ifdef PRINT_DEBUG
            // printf("recvfrom failed: error %d\n",  WSAGetLastError());
            break;
// #endif
        }
        uint32_t dwCount=0, dwWaitResult = 0;
        dwWaitResult = WaitForSingleObject(mutex, INFINITE);
        if (dwWaitResult == WAIT_OBJECT_0)
        {
            if (strncmp(sen_info.cmd, "ESP32", ESP32_REQ_SIZE) != 0) {
                printf("Unknown request %s, no permission given\n", sen_info.cmd);
                break;
            } else {
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
        ReleaseMutex(mutex);
        }
    } while (stop_receiving == false);

    if (sock != -1) {
#ifdef PRINT_DEBUG
        printf("Shutting down socket and restarting...");
#endif
        closesocket(sock);
        return 0;
    }
    return 1;
} /* udp_server_receive(void *) */

/*=====================================================================*/