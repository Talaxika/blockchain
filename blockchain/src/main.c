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
        mutex_recv;
transaction_t local_transactions[MAX_TRANSACTIONS_SIZE];

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

/* Main de-initializing function.
 *  - Closes the sockets.
 *  - Prints the blockchain
 *
 **/
iResult main_close(void);
/*======================================================================*/


int main(int argc, char* argv[]) {

    iResult iRes_main = RET_CODE_ERROR;
    // Check what is done by other blockchains
    // TODO: if first generate, else wait to recieve BC
    /******SETUP*******/

    if((iRes_main = main_open()) != RET_CODE_SUCCESS) {
        goto MAIN_END;
    }

    // print_blockchain(iBlockchain);

    mutex_recv = CreateMutex(NULL, FALSE, NULL);

    if (mutex_recv == NULL) {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }

    /* TODO: Make it so that it fills transactions locally, until a block is "solved/generated"
     * Then it puts the transaction array inside the block and then it generates hash of the block
     * based on the transactions or sth.
     * Then add the block to the blockchain and repeat the cycle.
     */

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

#ifdef USE_CONNECTION
    /* Open the socket and start listening */
    if((iResult = connect_open(&iCfg)) != RET_CODE_SUCCESS) {
        printf("%s(): Unsuccessful Socket initialization", __func__);
        goto END;
    }
#endif /* USE_CONNECTION */

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

iResult main_close(void)
{
    iResult iResult = RET_CODE_SUCCESS;

#if 1
    print_blockchain(iBlockchain);
    write_to_file(iBlockchain);
#endif

    CloseHandle(thread_calc);
    CloseHandle(thread_recv);
    CloseHandle(mutex_recv);

#ifdef USE_CONNECTION
    /* Shutdown connections, close existing sockets */
    iResult = connect_close(&iCfg);
#endif

    return iResult;
} /* main_close() */
/*=====================================================================*/