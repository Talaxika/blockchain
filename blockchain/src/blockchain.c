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
    for (int i = 0; i < MAX_HASH_SIZE; ++i)
        fprintf(f, "%02x", hash[i]);
    fprintf(f, "\n");
}

iResult build_block(Blockchain *blockchain)
{
    block_t prev_block = blockchain->blocks[blockchain->num_blocks-1];

    blockchain->blocks[blockchain->num_blocks].index = blockchain->num_blocks;
    blockchain->blocks[blockchain->num_blocks].timestamp = get_timestamp();

    uint64_t input = prev_block.timestamp + prev_block.nonce;

    /* calculate previous block hash */
    for (int i = 0; i < MAX_HASH_SIZE; i++)
    {
        blockchain->blocks[blockchain->num_blocks].previous_hash[i] = prev_block.current_hash[i];
    }
    
    memcpy(blockchain->blocks[blockchain->num_blocks].previous_hash,
            calc_sha_256(blockchain->blocks[blockchain->num_blocks].previous_hash, &input, prev_block.contents_length),
            sizeof(prev_block.current_hash));
    printf("From Build block:   ");
    fprint_hash(stdout, blockchain->blocks[blockchain->num_blocks].previous_hash);
    // if (memcmp(current_block.previous_hash, prev_block.current_hash , MAX_HASH_SIZE) != 0) {
    //     printf("Hash verification error at block number: %d", current_block.index);
    // }


    // printf("block.prev_hash:");
    // fprint_hash(stdout, prev_block.previous_hash);

    return RET_CODE_SUCCESS;
}

iResult mine_block(block_t *block, const uint8_t* target)
{
    iResult iRes = RET_CODE_ERROR;
    uint64_t input = 0;

    // while (true)
    // {
        /* MINING: start of the mining round */

        /* adjust the nonce until the block header is < the target hash */

        // for (uint32_t i = 0; i < UINT32_MAX; ++i)
        // {

            block->nonce = rand();
            input = block->nonce + block->timestamp;
            block->contents_length = get_num_len(input);
            printf("From Mine block:   ");
            memcpy(block->current_hash, calc_sha_256(block->current_hash, &input, block->contents_length), MAX_HASH_SIZE);
            fprint_hash(stdout, block->current_hash);

            // if (memcmp(block->current_hash, target, sizeof(block->current_hash)) < 0) {

                // if (memcmp(block->previous_hash, block->current_hash, MAX_HASH_SIZE) != 0) {
                //     /* we found a good hash */
                //     iRes = RET_CODE_SUCCESS;
                //     return iRes;
                // }
            // }
        // }
        /* The uint32 expired without finding a valid hash.
           Restart the time, and hope that this time + nonce combo works. */
    // }

    iRes = RET_CODE_ERROR;
    return iRes;
}

iResult add_transaction(block_t *block, header_cfg_t *hdr_cfg, char *data)
{
    iResult iRes = RET_CODE_ERROR;
    printf("num_transactions: %d\n", block->num_transactions);
    transaction_t tx = {0};
    char *end = NULL;

    if (block->num_transactions > MAX_TRANSACTIONS_SIZE) {
        printf("Max transaction size reached.\n");
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

iResult initializeFirstBlock(Blockchain *chain)
{
    iResult iRes = RET_CODE_ERROR;
    chain->blocks[0].index = 0;
    chain->blocks[0].timestamp = get_timestamp();
    chain->blocks[0].num_transactions = 1;
    chain->blocks[0].transactions[0].index = 0;
    chain->blocks[0].transactions[0].amount = 0;
    chain->blocks[0].transactions[0].sender_id = 0;
    chain->blocks[0].transactions[0].timestamp = get_timestamp();

    uint8_t target[MAX_HASH_SIZE] = {0};
    target[2] = 0x01;

    /* con_len, hash and nonce is generated from mining */
    mine_block(&chain->blocks[0], target);
    memset(chain->blocks[0].previous_hash, 0, sizeof(chain->blocks[0].previous_hash));
    // memcpy(block.current_hash, calc_sha_256(block.current_hash, &block.timestamp, block.contents_length), MAX_HASH_SIZE);
    chain->num_blocks = 1;

    iRes = RET_CODE_SUCCESS;
    return iRes;
}

void print_block(block_t block) {
    printf("Block %d:\n", block.index);
    printf("  Timestamp: %lld\n", block.timestamp);

    printf("  Previous hash:");
    fprint_hash(stdout, block.previous_hash);

    printf("  Current hash:");
    fprint_hash(stdout, block.current_hash);

    printf("  Contents_length: %d\n", block.contents_length);
    printf("  Nonce: %d\n", block.nonce);
    printf("  Transactions: %d\n", block.num_transactions);
    for (int i = 0; i <  block.num_transactions; i++)
    {
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



