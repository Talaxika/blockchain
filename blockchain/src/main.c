#include "include/blockchain.h"
#include "include/connect.h"

#define ROTATIONS 2

/*==================== Global Variable Declarations ====================*/
Blockchain b_chain;
conn_cfg_t iCfg;
char* (*recv_cb)(conn_cfg_t *cfg, header_cfg_t *hdr_cfg);
transaction_t local_transactions[MAX_TRANSACTIONS_SIZE];
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


int main() {

    iResult iResult = SUCCESS;
    // Check what is done by other blockchains
    // TODO: if first generate, else wait to recieve BC
    /******SETUP*******/
    
    iResult = main_init();

    char* end = NULL;
    Block block = {0};

    static const Block empty_block = {0};
     

    /* TODO: Make it so that it fills transactions locally, until a block is "solved/generated"
     * Then it puts the transaction array inside the block and then it generates hash of the block
     * based on the transactions or sth.
     * Then add the block to the blockchain and repeat the cycle.
     */

    /******Begin processes, each rotation is one block added*******/
    int rotations = 0;
    while (rotations < ROTATIONS)
    {
        header_cfg_t iHdr_cfg = {0};
        char* recv_buff = (*recv_cb)(&iCfg, &iHdr_cfg);
        add_transaction(&block, &iHdr_cfg, recv_buff);
        rotations++;
    }

    /******Clean up, print, etc...*******/
    // Verify hashes of other blocks
    add_block(&b_chain, block);
    
    iResult = main_deinit();
    // periodically write in file
    return 0;
} /* main() */


/*==================== Global Function Definitions ====================*/
iResult main_init(void)
{
    iResult iResult = ERROR;

    /* Initiate the genesis block */
    initiateFirstBlock(&b_chain);
    
    /* Open the socket and start listening */
    iResult = connect_open(&iCfg);
    
    /* Make a function pointer so that it's more flexible. That is the recv function. */
    recv_cb = &connect_recieve;

    iResult = SUCCESS;
    return iResult;
} /* main_init() */

iResult main_deinit(void)
{
    iResult iResult = ERROR;

    print_blockchain(b_chain);
    iResult = connect_close(&iCfg);

    iResult = SUCCESS;
    return iResult;
} /* main_deinit() */
/*=====================================================================*/