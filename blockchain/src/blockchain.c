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

void fprint_hash(FILE* f, uint8_t* hash)
{
    fprintf(f, "0x");
    for (int i = 0; i < 32; ++i)
        fprintf(f, "%02x", hash[i]);
}

block_t build_block(const block_t* previous)
{
    block_t block;
    block.timestamp = (uint64_t)time(NULL);
    block.contents_length = get_num_len(block.timestamp);

    if (previous)
    {
        /* calculate previous block hash */
        calc_sha_256(block.previous_hash, previous, sizeof(block_t));
    }
    else
    {
        /* genesis has no previous. just use zeroed hash */
        memset(block.previous_hash, 0, sizeof(block.previous_hash));
    }
    
    /* add data hash */
    calc_sha_256(block.current_hash, block.timestamp, block.contents_length);
    return block;
}

iResult mine_block(block_t *block, const uint8_t* target)
{
    iResult iRes = RET_CODE_ERROR;

    while (iRes != RET_CODE_SUCCESS)
    {
        /* MINING: start of the mining round */

        /* adjust the nonce until the block header is < the target hash */
        uint8_t block_hash[32] = {0};

        for (uint32_t i = 0; i < UINT32_MAX; ++i)
        {
            block->nonce = i;
            calc_sha_256(block_hash, block, sizeof(block_t));

            if (memcmp(block_hash, target, sizeof(block_hash)) < 0)
                /* we found a good hash */
                iRes = RET_CODE_SUCCESS;
        }
        /* The uint32 expired without finding a valid hash.
           Restart the time, and hope that this time + nonce combo works. */
    }
    iRes = RET_CODE_ERROR;
    return iRes;
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

    block.index = blockchain->num_blocks;

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
        .timestamp = get_timestamp(),
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
    fprint_hash(stdout, block.previous_hash);
    fprint_hash(stdout, block.current_hash);
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



