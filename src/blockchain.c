
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
hash_t isHashValid(Block block)
{
    // return generateHash(block) == block.hash;
}


// TODO:
time_t get_timestamp()
{
    return 0;
}

void add_block(Blockchain *blockchain, Transaction tx) {
    Block block;

    block.index = blockchain->num_blocks;
    block.timestamp = get_timestamp();
    block.transaction = tx;

    Block previous_block = blockchain->blocks[blockchain->num_blocks - 1];
    // memcpy(block.prev_hash, previous_block.hash, MAX_HASH_SIZE);
    block.prev_hash = previous_block.hash;

    // TODO: Implement hash function
    // generateHash(&block);
    block.hash = "randomhash";

    blockchain->blocks = realloc(blockchain->blocks, (blockchain->num_blocks + 1) * sizeof(Block));
    blockchain->blocks[blockchain->num_blocks] = block;
    blockchain->num_blocks++;
}

void initiateFirstBlock(Blockchain *chain)
{
    Transaction tx =
    {
        .amount = 0,
        .sender = NULL,
        .recipient = NULL
    };

    Block block =
    {
        .index = 0,
        .prev_hash = NULL,
        .transaction = tx,
        .timestamp = get_timestamp(),
        .hash = "genesisHASH"
    };

    chain->blocks = malloc(sizeof(Block));
    chain->blocks[0] = block;
    chain->num_blocks = 1;
}

void print_block(Block block) {
    printf("Block %d:\n", block.index);
    printf("  Timestamp: %s", ctime(&block.timestamp));
    printf("  Prev Hash: %s\n", block.prev_hash);
    printf("  Hash: %s\n", block.hash);
    printf("  Transaction:\n");
    printf("    Sender: %s\n", block.transaction.sender);
    printf("    Recipient: %s\n", block.transaction.recipient);
    printf("    Amount: %f\n", block.transaction.amount);
}

void print_blockchain(Blockchain chain) {
    int i = 0;
    for (i = 0; i < chain.num_blocks; i++) {
        print_block(chain.blocks[i]);
    }
}



