#include "include/connect.h"
#include "include/blockchain.h"

#define HASH_SHIFT_VAL (4U)

hash_t getCurrHash(block_t block)
{
    return block.hash;
}
hash_t getPrevHash(block_t block)
{
    return block.prev_hash;
}

time_t get_timestamp()
{
    time_t t;
    time(&t);
    return t;
}

uint32_t get_num_len(uint64_t value)
{
    uint32_t l=1;

    while(value > 9)
    {
        l++;
        value/=10;
    }

    return l;
}

/* Hashing function used for block hash generation. */
iResult y_hash(block_t *block)
{
    iResult iRes = ERROR_RET;
    uint64_t num_hash = 0;
    uint32_t y = 2206;

    if (block->num_transactions != 0)
    {
        for (int i = 0; i < block->num_transactions; i++)
        {
            num_hash += (block->transactions[i].amount + block->transactions[i].timestamp);
        }
        num_hash /= block->num_transactions;
        num_hash += block->index;
    }

    /* No matter what the resulting number is, it is shifted to left by 4 to add 0 zeroes at the end
     * of the number, which then is added with the 'y' number. That way, the block is always
     * appended this number at the end, which can be used for verification. */
    uint32_t num_len = get_num_len(num_hash);
    uint32_t shift_val = 0;

    if (num_len > MAX_HASH_SIZE + HASH_SHIFT_VAL) {
        shift_val = num_len - MAX_HASH_SIZE + HASH_SHIFT_VAL;
    }

    num_hash = (num_hash >> shift_val) << HASH_SHIFT_VAL;
    num_hash += y;
    snprintf(block->hash, num_len, "%lld", num_hash);

    iRes = SUCCESS_RET;
    return iRes;
}

iResult add_transaction(block_t *block, header_cfg_t *hdr_cfg, char *data)
{
    iResult iRes = ERROR_RET;

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

    iRes = SUCCESS_RET;
    return iRes;
}

iResult add_block(Blockchain *blockchain, block_t block) {
    iResult iRes = ERROR_RET;

    block_t previous_block = blockchain->blocks[blockchain->num_blocks - 1];

    /* Number of transactions and the transactions themselves are added
     * before the add_block function. */

    y_hash(&block);
    block.index = blockchain->num_blocks;
    block.timestamp = get_timestamp();
    memcpy(block.prev_hash, previous_block.hash, MAX_HASH_SIZE);

    blockchain->blocks[blockchain->num_blocks] = block;
    blockchain->num_blocks++;

    iRes = SUCCESS_RET;
    return iRes;
}

iResult initializeFirstBlock(Blockchain *chain)
{
    iResult iRes = ERROR_RET;

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
        .prev_hash = NULL,
        .timestamp = get_timestamp(),
        .hash = "genesisHASH"
    };
    block.transactions[0] = tx;

    chain->blocks[0] = block;
    chain->num_blocks = 1;

    iRes = SUCCESS_RET;
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



