#include "include/connect.h"
#include "include/blockchain.h"
#include "include/sha256.h"

#define UINT64_MAXVAL    (18446744073709551615U)
#define MULT_PADDING_VAL (10000U)
#define SPECIAL_NUM      (2206u)

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


static uint32_t mod_exp(uint32_t base, uint32_t exp, uint32_t mod) {
    uint32_t result = 1;
    base = base % mod;

    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }

        exp = exp >> 1;
        base = (base * base) % mod;
    }

    return result;
}

    /*
    int p = 61; // Prime number
    int q = 53; // Prime number
    int modulus = p * q;
    int phi = (p - 1) * (q - 1);
    int publicKey = 17; // Public exponent
    int privateKey = mod_inverse(publicKey, phi);
    */
// Function to perform RSA encryption
uint32_t rsa_encrypt(uint32_t enc_timestamp, RSAKey publicKey) {
    return mod_exp(enc_timestamp, publicKey.e, publicKey.n);
}

// Function to perform RSA decryption
uint32_t rsa_decrypt(uint32_t dec_timestamp, RSAKey privateKey) {
    return mod_exp(dec_timestamp, privateKey.d, privateKey.n);
}

uint64_t hash(uint64_t input) {
    const uint64_t prime = 1099511628211ULL;  // A large prime number
    const uint64_t offset = 14695981039346656037ULL;  // Another constant

    uint64_t hash = offset;

    for (int i = 0; i < 8; ++i) {
        uint8_t byte = input & 0xFF;
        hash = (hash ^ byte) * prime;
        input >>= 8;
    }
    return hash;
}

iResult build_and_verify_block(Blockchain *blockchain)
{
    block_t curr_block = blockchain->blocks[blockchain->num_blocks];
    block_t prev_block = blockchain->blocks[blockchain->num_blocks - 1];

    curr_block.index = blockchain->num_blocks;
    curr_block.timestamp = get_timestamp();

    curr_block.previous_hash = hash((uint64_t)prev_block.timestamp);
    if (curr_block.previous_hash != prev_block.current_hash) {
        printf("Hash value mismatch on block index %d", curr_block.index);
    }

    blockchain->blocks[blockchain->num_blocks] = curr_block;

    return RET_CODE_SUCCESS;
}

iResult mine_block(block_t *block)
{
    iResult iRes = RET_CODE_ERROR;

    block->current_hash = hash((uint64_t)block->timestamp);

    iRes = RET_CODE_ERROR;
    return iRes;
}

iResult add_transaction(block_t *block, header_cfg_t *hdr_cfg, char *data)
{
    iResult iRes = RET_CODE_ERROR;
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
    chain->blocks[0].previous_hash = 0;
    chain->blocks[0].num_transactions = 1;
    chain->blocks[0].transactions[0].index = 0;
    chain->blocks[0].transactions[0].amount = 0;
    chain->blocks[0].transactions[0].sender_id = 0;
    chain->blocks[0].transactions[0].timestamp = get_timestamp();

    /* con_len, hash and nonce is generated from mining */
    mine_block(&chain->blocks[0]);
    chain->num_blocks = 1;

    iRes = RET_CODE_SUCCESS;
    return iRes;
}

void print_block(block_t block) {
    printf("Block %d:\n", block.index);
    printf("  Timestamp: %llu\n", block.timestamp);
    printf("  Previous hash: %llu", block.previous_hash);
    printf("  Current hash: %llu", block.current_hash);
    printf("  Nonce: %llu\n", block.nonce);
    printf("  Transactions: %d\n", block.num_transactions);
    for (int i = 0; i <  block.num_transactions; i++)
    {
        printf("    -Index: %d\n", block.transactions[i].index);
        printf("    Sender_id: %d\n", block.transactions[i].sender_id);
        printf("    Amount: %d\n", block.transactions[i].amount);
        printf("    Timestamp: %llu\n", block.transactions[i].timestamp);
    }
}

void print_blockchain(Blockchain chain) {
    int i = 0;
    for (i = 0; i < chain.num_blocks; i++) {
        print_block(chain.blocks[i]);
    }
}



