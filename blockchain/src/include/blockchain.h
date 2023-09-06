#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "util.h"
#include "connect.h"

#define BLOCK_TRANSACTION_SIZE (12U)

#define MAX_BLOCK_SIZE (8U)
#define MAX_TRANSACTIONS_SIZE (16U)

#define BLOCK_GENERATION_TIME  (20000U)

typedef struct {
  uint32_t n;
  uint32_t e;
  uint32_t d;
} RSAKey;

typedef struct {
  uint32_t index;
  float sen_temp;
  time_t timestamp;
  uint8_t base_mac_addr[6];
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

uint32_t rsa_encrypt(uint32_t enc_timestamp, RSAKey publicKey);

uint32_t rsa_decrypt(uint32_t dec_timestamp, RSAKey privateKey);

iResult initializeFirstBlock(Blockchain *chain);

iResult build_and_verify_block(Blockchain *chain);

iResult mine_block(block_t *block);

iResult add_transaction(block_t *block, sensor_info_t *sen_info);

void print_blockchain(Blockchain chain);

iResult send_broadcast_message(Blockchain *blockchain);

#endif /* BLOCKCHAIN_H */