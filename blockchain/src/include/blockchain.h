#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "util.h"
#include "connect.h"

#define BLOCK_TRANSACTION_SIZE (128U)
#define DIFFICULTY (3U)

#define MAX_BLOCK_SIZE (256U)
#define MAX_HASH_SIZE (15U)
#define MAX_TRANSACTIONS_SIZE (64U)

typedef char* hash_t;

typedef struct {
  uint32_t index;
  uint32_t sender_id;
  uint32_t amount;
  time_t timestamp;
} transaction_t;

typedef struct {
  uint32_t index;
  transaction_t transactions[MAX_TRANSACTIONS_SIZE];
  uint32_t num_transactions;
  hash_t prev_hash;
  hash_t hash;
  time_t timestamp;
} block_t;

typedef struct {
  block_t blocks[MAX_BLOCK_SIZE];
  uint32_t num_blocks;
} Blockchain;

hash_t getCurrHash(block_t block);

hash_t getPrevHash(block_t block);

uint32_t get_num_len(uint64_t value);

time_t get_timestamp();

iResult y_hash(block_t *block);

iResult initializeFirstBlock(Blockchain *chain);

iResult add_block(Blockchain *blockchain, block_t block);

iResult add_transaction(block_t *block, header_cfg_t *hdr_cfg, char *data);

void print_blockchain(Blockchain chain);

#endif /* BLOCKCHAIN_H */