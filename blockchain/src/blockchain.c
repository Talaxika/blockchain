#include "include/connect.h"
#include "include/blockchain.h"
#include "include/sha256.h"

#define UINT64_MAXVAL    (18446744073709551615U)
#define MULT_PADDING_VAL (10000U)
#define HASH_SPEC_NUM    (2206U)

time_t get_timestamp()
{
    time_t t;
    time(&t);
    return t;
}

iResult add_transaction(block_t *block, header_cfg_t *hdr_cfg, char *data)
{
    iResult iRes = RET_CODE_ERROR;

    transaction_t tx = {0};
    char *end = NULL;

    if (block->num_transactions > MAX_TRANSACTIONS_SIZE) {
        printf("Max transaction size reached.");
        return iRes;
    }

    tx.index = block->num_transactions;
    tx.timestamp = get_timestamp();
    tx.sender_id = (uint32_t) hdr_cfg->sen_info.sen_id;
    tx.amount = strtol(data, &end, 10);

    block->transactions[block->num_transactions] = tx;
    block->num_transactions++;

    iRes = RET_CODE_SUCCESS;
    return iRes;
}

iResult add_block(Blockchain *blockchain, block_t block) {
    iResult iRes = RET_CODE_ERROR;

    block_t previous_block = blockchain->blocks[blockchain->num_blocks - 1];

    /* Number of transactions and the transactions themselves are added
     * before the add_block function. */

    y_hash(&block);
    block.index = blockchain->num_blocks;
    block.timestamp = get_timestamp();
    strncpy(block.prev_hash, previous_block.hash, MAX_HASH_SIZE);

    blockchain->blocks[blockchain->num_blocks] = block;
    blockchain->num_blocks++;
    iRes = RET_CODE_SUCCESS;
    return iRes;
}


iResult initializeFirstBlock(Blockchain *chain)
{
    iResult iRes = RET_CODE_ERROR;

    transaction_t tx =
    {
        .index = 0,
        .amount = 0,
        .sender_id = 0,
        .timestamp = get_timestamp()
    };

    block_t block =
    {
        .index = 0,
        .prev_hash = {0},
        .timestamp = get_timestamp(),
        .hash = "genesisHASH"
    };
    block.transactions[0] = tx;

    chain->blocks[0] = block;
    chain->num_blocks = 1;

    iRes = RET_CODE_SUCCESS;
    return iRes;
}

void print_block(block_t block) {
    printf("Block %d:\n", block.index);
    printf("  Timestamp: %s", ctime(&block.timestamp));
    printf("  Prev Hash: %s\n", block.prev_hash);
    printf("  Hash: %s\n", block.hash);
    for (int i = 0; i <  block.num_transactions; i++)
    {
        printf("  Transactions:\n");
        printf("    Index: %d\n", block.transactions[i].index);
        printf("    Sender_id: %d\n", block.transactions[i].sender_id);
        printf("    Amount: %d\n", block.transactions[i].amount);
        printf("    Timestamp: %lld\n", block.transactions[i].timestamp);
    }
}

void print_blockchain(Blockchain chain) {
    int i = 0;
    for (i = 0; i < chain.num_blocks; i++) {
        print_block(chain.blocks[i]);
    }
}



