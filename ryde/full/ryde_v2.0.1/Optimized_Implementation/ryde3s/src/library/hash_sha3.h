#ifndef HASH_SHA3_H
#define HASH_SHA3_H

#include "KeccakHash.h"
#include "SimpleFIPS202.h"

typedef Keccak_HashInstance hash_sha3_ctx;

void hash_sha3_init(hash_sha3_ctx *ctx);
void hash_sha3_absorb(hash_sha3_ctx *ctx, const uint8_t *input, size_t size);
void hash_sha3_finalize(uint8_t *output, hash_sha3_ctx *ctx);

typedef struct seedexpander_shake_t {
    Keccak_HashInstance state;
} seedexpander_shake_t;

void seedexpander_shake_init(seedexpander_shake_t* seedexpander_shake, const uint8_t* seed, size_t seed_size, const uint8_t* salt, size_t salt_size);
void seedexpander_shake_get_bytes(seedexpander_shake_t* seedexpander_shake, uint8_t* output, size_t output_size);

#endif //HASH_SHA3_H
