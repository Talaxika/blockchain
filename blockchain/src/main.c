#include "include/blockchain.h"
#include "include/connect.h"

#define ROTATIONS_BLK (2U)
#define ROTATIONS_TRX (3U)

/*==================== Global Variable Declarations ====================*/
Blockchain b_chain;
conn_cfg_t iCfg;

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
iResult main_init(void);

/* Main de-initializing function.
 *  - Closes the sockets.
 *  - Prints the blockchain
 *
 **/
iResult main_deinit(void);
/*======================================================================*/


int main(int argc, char* argv[]) {

    iResult iResult = ERROR_RET;
    // Check what is done by other blockchains
    // TODO: if first generate, else wait to recieve BC
    /******SETUP*******/

    if((iResult = main_init()) != SUCCESS_RET) {
        goto MAIN_END;
    }

    char* end = NULL;
    block_t block = {0};

    static const block_t empty_block = {0};
    static const header_cfg_t empty_hdr_cfg = {0};
    transaction_t local_transactions[MAX_TRANSACTIONS_SIZE] = {0};


    /* TODO: Make it so that it fills transactions locally, until a block is "solved/generated"
     * Then it puts the transaction array inside the block and then it generates hash of the block
     * based on the transactions or sth.
     * Then add the block to the blockchain and repeat the cycle.
     */

    /******Begin processes, each rotation is one block added*******/
    uint32_t rotations_TRX = 0;
    uint32_t rotations_BLK = ROTATIONS_BLK;
    header_cfg_t iHdr_cfg = {0};

    while (rotations_BLK > 0)
    {
        /* HASH 1 = 540805969
         * HASH 2 = 540805971 */
        memset(&block, 0, sizeof(block_t));
        memset(&local_transactions, 0, sizeof(transaction_t));
        while (rotations_TRX < ROTATIONS_TRX)
        {
            char* recv_buff = (*recv_cb)(&iCfg, &iHdr_cfg, rotations_BLK);
            add_transaction(&block, &iHdr_cfg, recv_buff);

            iHdr_cfg = empty_hdr_cfg;
            rotations_TRX++;
        }
        add_block(&b_chain, block);
        rotations_BLK--;
        rotations_TRX = 0;
    }


MAIN_END:

    /******Clean up, print, etc...*******/
    // Verify hashes of other blocks
    iResult = main_deinit();
    // periodically write in file
    return 0;
} /* main() */


/*==================== Global Function Definitions ====================*/
iResult main_init(void)
{
    iResult iResult = ERROR_RET;

    /* Initiate the genesis block */
    if((iResult = initializeFirstBlock(&b_chain)) != SUCCESS_RET) {
        printf("%s(): Unsuccessful Block initialization", __func__);
        goto END;
    }

    /* Open the socket and start listening */
    if((iResult = connect_open(&iCfg)) != SUCCESS_RET) {
        printf("%s(): Unsuccessful Socket initialization", __func__);
        goto END;
    }
    /* That is the recv function. */
    recv_cb = &connect_recieve;

END:
    return iResult;
} /* main_init() */

iResult main_deinit(void)
{
    iResult iResult = ERROR_RET;

    print_blockchain(b_chain);
    iResult = connect_close(&iCfg);

    iResult = SUCCESS_RET;
    return iResult;
} /* main_deinit() */
/*=====================================================================*/