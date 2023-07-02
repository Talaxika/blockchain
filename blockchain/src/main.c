#include "include/blockchain.h"
#include "include/connect.h"

#define ROTATIONS 2

int main() {
    Blockchain btc = {0};
    initiateFirstBlock(&btc);
    char* end = NULL;
    Block block = {0};
    static const Block empty_block = {0};

    char* (*recv)(void) = &connect_recieve;

    int rotations = 0;
    while (rotations < ROTATIONS)
    {
        char* recv_buff = (*recv)();
        printf("%s\n", recv_buff);
        add_transaction(&block, recv_buff);
        block = empty_block;
        rotations++;
    }

    add_block(&btc, block);
    print_blockchain(btc);
    return 0;
}