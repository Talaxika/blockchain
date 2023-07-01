#include <stdio.h>
#include <time.h>
#include "include/blockchain.h"

int main() {
    Blockchain btc;

    // The first block in the blockchain is empty
    initiateFirstBlock(&btc);
    // TODO: make an interesting way to do transactions

    Transaction tx1 = {.amount = 1.5, .recipient = "sasha", .sender = "peter"};
    add_block(&btc, tx1);
    print_blockchain(btc);
    return 0;
}