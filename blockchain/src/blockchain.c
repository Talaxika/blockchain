#include "include/connect.h"
#include "include/blockchain.h"


// TODO: implement
hash_t getCurrHash(Block block)
{
    return block.hash;
}
hash_t getPrevHash(Block block)
{
    return block.prev_hash;
}
hash_t generateHash(Block *block);
//     hash<string> hash1;
//     hash<hash_t> hash2;
//     hash<hash_t> finalHash;
//     string toHash = to_string(data.amount) + data.recieverKey + data.senderKey + to_string(data.timestamp);

//     return finalHash(hash1(toHash) + hash2(previousHash));
// hash_t isHashValid(Block block)
// {
//     // return generateHash(block) == block.hash;
// }

time_t get_timestamp()
{
    time_t t;
    time(&t);
    return t;
}

iResult add_transaction(Block *block, header_cfg_t *hdr_cfg, char *data) {
    iResult iRes = ERROR;

    transaction_t tx;

    if (block->num_transactions > MAX_TRANSACTIONS_SIZE) {
        printf("Max transaction size reached.");
        return iRes;
    }
    
    tx.index = block->num_transactions;
    tx.timestamp = get_timestamp();
    tx.sender = "";
    strncpy(tx.amount, data, MAX_AMOUNT_SIZE);

    block->transactions[block->num_transactions] = tx;
    block->num_transactions++;

    iRes = SUCCESS;
    return iRes;
}

iResult add_block(Blockchain *blockchain, Block block) {
    iResult iRes = ERROR;

    block.index = blockchain->num_blocks;
    block.timestamp = get_timestamp();

    Block previous_block = blockchain->blocks[blockchain->num_blocks - 1];
    // memcpy(block.prev_hash, previous_block.hash, MAX_HASH_SIZE);
    block.prev_hash = previous_block.hash;

    // TODO: Implement hash function
    // generateHash(&block);
    block.hash = "randomhash";

    blockchain->blocks[blockchain->num_blocks] = block;
    blockchain->num_blocks++;

    iRes = SUCCESS;
    return iRes;
}

iResult initiateFirstBlock(Blockchain *chain)
{
    iResult iRes = ERROR;

    transaction_t tx =
    {
        .index = 0,
        .amount = "0",
        .sender = "genesisSender",
        .timestamp = get_timestamp()
    };

    Block block =
    {
        .index = 0,
        .prev_hash = NULL,
        .timestamp = get_timestamp(),
        .hash = "genesisHASH"
    };
    block.transactions[0] = tx;

    chain->blocks[0] = block;
    chain->num_blocks = 1;

    iRes = SUCCESS;
    return iRes;
}

void print_block(Block block) {
    printf("Block %d:\n", block.index);
    printf("  Timestamp: %s", ctime(&block.timestamp));
    printf("  Prev Hash: %s\n", block.prev_hash);
    printf("  Hash: %s\n", block.hash);
    for (int i = 0; i <  block.num_transactions; i++)
    {
        printf("  Transactions:\n");
        printf("    Index: %d\n", block.transactions[i].index);
        printf("    Sender: %s\n", block.transactions[i].sender);
        printf("    Amount: %s\n", block.transactions[i].amount);
        printf("    Timestamp: %lld\n", block.transactions[i].timestamp);
    }
}

void print_blockchain(Blockchain chain) {
    int i = 0;
    for (i = 0; i < chain.num_blocks; i++) {
        print_block(chain.blocks[i]);
    }
}



