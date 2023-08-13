#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "util.h"
#include "connect.h"

#define BLOCK_TRANSACTION_SIZE (12U)

#define MAX_BLOCK_SIZE (6U)
#define MAX_TRANSACTIONS_SIZE (12U)

#define SLEEP_TIME  (7000U)

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

  uint64_t current_hash;
  uint64_t previous_hash;
  time_t timestamp;

  uint64_t nonce;
} block_t;

typedef struct {
  block_t blocks[MAX_BLOCK_SIZE];
  uint32_t num_blocks;
} Blockchain;

time_t get_timestamp();

iResult initializeFirstBlock(Blockchain *chain);

iResult build_and_verify_block(Blockchain *chain);

iResult mine_block(block_t *block);

iResult add_transaction(block_t *block, header_cfg_t *hdr_cfg, char *data);

void print_blockchain(Blockchain chain);

iResult send_broadcast_message(Blockchain *blockchain);

#endif /* BLOCKCHAIN_H */