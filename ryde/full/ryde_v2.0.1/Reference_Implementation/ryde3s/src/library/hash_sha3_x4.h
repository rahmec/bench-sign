
#ifndef HASH_SHA3_TIMES4_H
#define HASH_SHA3_TIMES4_H

#include "KeccakHashtimes4.h"

typedef Keccak_HashInstancetimes4 hash_sha3_x4_ctx;

void hash_sha3_x4_init(hash_sha3_x4_ctx *ctx);
void hash_sha3_x4_absorb(hash_sha3_x4_ctx *ctx, const uint8_t **input, size_t size);
void hash_sha3_x4_finalize(uint8_t **output, hash_sha3_x4_ctx *ctx);

typedef struct seedexpander_shake_x4_t {
    Keccak_HashInstancetimes4 state;
} seedexpander_shake_x4_t;

void seedexpander_shake_x4_init(seedexpander_shake_x4_t* seedexpander_shake, const uint8_t** seed, size_t seed_size, const uint8_t** salt, size_t salt_size);
void seedexpander_shake_x4_get_bytes(seedexpander_shake_x4_t* seedexpander_shake, uint8_t** output, size_t output_size);

#endif //HASH_SHA3_TIMES4_H
