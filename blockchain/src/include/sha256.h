#ifndef SHA_256_H
#define SHA_256_H

/* IMPORTANT: The sha256 is not integrated,
 * but is one of the ways the whole program can be improved.
 **/
#include "util.h"

uint8_t* calc_sha_256(uint8_t hash[MAX_HASH_SIZE], void *input, size_t len);

#endif