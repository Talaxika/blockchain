#include "include/blockchain.h"
#include "include/connect.h"

#define ROTATIONS 2

Blockchain bc;

int main() {

    // Check what is done by other blockchains
    // TODO: if first generate, else wait to recieve BC
    /******SETUP*******/
    Blockchain btc = {0};
    initiateFirstBlock(&btc);
    char* end = NULL;
    Block block = {0};
    
    Transaction* local_transactions;
    static const Block empty_block = {0};

    char* (*recv)(void) = &connect_recieve;

    /* TODO: Make it so that it fills transactions locally, until a block is "solved/generated"
     * Then it puts the transaction array inside the block and then it generates hash of the block
     * based on the transactions or sth.
     * Then add the block to the blockchain and repeat the cycle.
     */

    /******Begin processes, each rotation is one block added*******/
    int rotations = 0;
    while (rotations < ROTATIONS)
    {
        char* recv_buff = (*recv)();
        add_transaction(&block, recv_buff);
        rotations++;
    }

    /******Clean up, print, etc...*******/
    // Verify hashes of other blocks
    add_block(&btc, block);
    print_blockchain(btc);
    // periodically write in file
    return 0;
}