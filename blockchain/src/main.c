#include "include/blockchain.h"
#include "include/connect.h"
#include "include/sha256.h"
#include "include/fileIO.h"

#define ROTATIONS_BLK (2U)
#define ROTATIONS_TRX (2U)

#define USE_CONNECTION
#define USE_MINING

/*=========================== Local Typedefs ===========================*/
/*======================================================================*/

/*==================== Global Variable Declarations ====================*/
Blockchain iBlockchain = {0};
block_t block = {0};
conn_cfg_t iCfg;
HANDLE  thread_calc,
        thread_recv,
        thread_bcast,
        mutex_recv;
transaction_t local_transactions[MAX_TRANSACTIONS_SIZE];
volatile int stop_listening = 0;

/* Make a function pointer so that it's more flexible.
 * It will be equal to the recv function*/
char* (*recv_cb)(conn_cfg_t *cfg, header_cfg_t *hdr_cfg, uint32_t rotations);
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

iResult_thread WINAPI main_bcast(void *);

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
    if((iRes_main = connect_open(&iCfg)) != RET_CODE_SUCCESS) {
        printf("%s(): Unsuccessful Socket initialization", __func__);
        goto MAIN_END;
    }
#endif /* USE_CONNECTION */
    // Check what is done by other blockchains
    // TODO: if first generate, else wait to recieve BC
    /******SETUP*******/

    // print_blockchain(iBlockchain);

    mutex_recv = CreateMutex(NULL, FALSE, NULL);

    if (mutex_recv == NULL) {
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
            thread_calc = CreateThread(NULL, 0x0, main_mine, NULL, 0, NULL);
            if (thread_calc == NULL || thread_calc == INVALID_HANDLE_VALUE) {
                printf("CreateThread error: %d\n", GetLastError());
                return 1;
            }

#ifdef USE_CONNECTION
            thread_recv = CreateThread(NULL, 0, main_recv, NULL, 0, NULL);

            if (thread_recv == NULL || thread_recv == INVALID_HANDLE_VALUE) {
                printf("CreateThread error: %d\n", GetLastError());
                return 1;
            }
            WaitForSingleObject(thread_recv, INFINITE);
            // printf("%d", GetLastError());
#endif
        } while (WaitForSingleObject(thread_calc, INFINITE));
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

    // uint32_t dwCount=0, dwWaitResult = 0;
    // dwWaitResult = WaitForSingleObject(mutex_recv, INFINITE);
    // switch (dwWaitResult)
    // {
    //     // The thread got ownership of the mutex
    //     case WAIT_OBJECT_0:

    header_cfg_t iHdr_cfg = {0};
    uint32_t rotations_TRX = ROTATIONS_TRX;
    memset(&local_transactions, 0, sizeof(transaction_t));

    while (rotations_TRX > 0)
    {
        char* recv_buff = connect_recieve(&iCfg, &iHdr_cfg, rotations_TRX);
        add_transaction(&iBlockchain.blocks[iBlockchain.num_blocks], &iHdr_cfg, recv_buff);

        rotations_TRX--;
    }
    //         if (! ReleaseMutex(mutex_recv))
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
    Sleep(SLEEP_TIME);

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
    CloseHandle(mutex_recv);

#ifdef USE_CONNECTION
    /* Shutdown connections, close existing sockets */
    iResult = connect_close(&iCfg);
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

    char buffer[1024];
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
    }
    printf("Got Here\n");
    closesocket(sockfd);
    WSACleanup();
    return 0;
}