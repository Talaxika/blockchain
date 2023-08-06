#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "util.h"
#include "connect.h"

#define BLOCK_TRANSACTION_SIZE (128U)
#define DIFFICULTY (3U)

#define MAX_BLOCK_SIZE (256U)
#define MAX_HASH_SIZE (20U)
#define MAX_TRANSACTIONS_SIZE (64U)

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

  uint8_t current_hash[32];
  uint8_t previous_hash[32];
  time_t timestamp;

  /* This is adjusted to make the hash of this header fall in the valid range. */
  uint32_t contents_length;
  uint32_t nonce;
} block_t;

typedef struct {
  block_t blocks[MAX_BLOCK_SIZE];
  uint32_t num_blocks;
} Blockchain;

time_t get_timestamp();

void fprint_hash(FILE* f, uint8_t* hash);

iResult initializeFirstBlock(Blockchain *chain);

iResult add_block(Blockchain *blockchain, block_t block);

block_t build_block(const block_t* previous);

iResult mine_block(block_t *block, const uint8_t* target);

iResult add_transaction(block_t *block, header_cfg_t *hdr_cfg, char *data);

void print_blockchain(Blockchain chain);

#endif /* BLOCKCHAIN_H */