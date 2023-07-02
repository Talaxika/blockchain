#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define BLOCKCHAIN_SIZE 100
#define BLOCK_TRANSACTION_SIZE 128
#define DIFFICULTY 3
#define MAX_HASH_SIZE 15

typedef char* hash_t;

typedef struct {
  int index;
  hash_t sender;
  char* amount;
  time_t timestamp;
} Transaction;

typedef struct {
  int index;
  Transaction* transactions;
  int num_transactions;
  hash_t prev_hash;
  hash_t hash;
  time_t timestamp;
} Block;

typedef struct {
  Block* blocks;
  int num_blocks;
} Blockchain;

time_t get_timestamp();

void initiateFirstBlock(Blockchain* chain);

void add_block(Blockchain* blockchain, Block block);

void add_transaction(Block* block, char* data);

void print_blockchain(Blockchain chain);

#endif /* BLOCKCHAIN_H */