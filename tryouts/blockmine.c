#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define BLOCK_SIZE 80
#define DIFFICULTY 5 // Number of leading zeros required in the hash

// Simple hash function
uint32_t simple_hash(const char* data) {
    uint32_t hash = 0;
    while (*data) {
        hash = (hash * 31) + *data;
        data++;
        printf("%d\t", hash);
    }
    return hash;
}

// Function to check if the calculated hash satisfies the difficulty condition
int hash_satisfies_difficulty(uint32_t hash, uint32_t difficulty) {
    return (hash & ((1 << difficulty) - 1)) == 0;
}

// Function to mine a block
int mine_block(uint32_t difficulty, const char* data) {
    uint32_t nonce = 0;

    do {
        uint64_t hash = simple_hash(data) + nonce;
        if (hash_satisfies_difficulty(hash, difficulty)) {
            printf("Block mined!\n");
            printf("Nonce: %u\n", nonce);
            return hash;
        }
        nonce++;
    } while (1);
}

int main() {
    // Sample data for the block
    const char* data = "Sample Data";

    // Mine the block with the specified difficulty
    uint64_t hash = mine_block(DIFFICULTY, data);
    printf("%lld", hash);

    return 0;
}
