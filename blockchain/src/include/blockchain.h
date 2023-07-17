#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "util.h"
#include "connect.h"

#define BLOCK_TRANSACTION_SIZE (128U)
#define DIFFICULTY (3U)

#define MAX_BLOCK_SIZE (256U)
#define MAX_AMOUNT_SIZE (15U)
#define MAX_HASH_SIZE (15U)
#define MAX_TRANSACTIONS_SIZE (64U)

typedef char* hash_t;

typedef struct {
  uint32_t index;
  hash_t sender;
  char amount[MAX_AMOUNT_SIZE];
  time_t timestamp;
} transaction_t;

typedef struct {
  uint32_t index;
  transaction_t transactions[MAX_TRANSACTIONS_SIZE];
  uint32_t num_transactions;
  hash_t prev_hash;
  hash_t hash;
  time_t timestamp;
} Block;

typedef struct {
  Block blocks[MAX_BLOCK_SIZE];
  uint32_t num_blocks;
} Blockchain;

time_t get_timestamp();

iResult initiateFirstBlock(Blockchain* chain);

iResult add_block(Blockchain* blockchain, Block block);

iResult add_transaction(Block* block, header_cfg_t *hdr_cfg, char *data);

void print_blockchain(Blockchain chain);

#endif /* BLOCKCHAIN_H */