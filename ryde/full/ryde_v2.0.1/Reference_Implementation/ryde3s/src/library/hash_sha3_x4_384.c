#include "hash_sha3_x4.h"

void hash_sha3_x4_init(hash_sha3_x4_ctx *ctx) {
    Keccak_HashInitializetimes4_SHA3_384(ctx);
}
  
void hash_sha3_x4_absorb(hash_sha3_x4_ctx *ctx, const uint8_t **input, size_t size) {
    Keccak_HashUpdatetimes4(ctx, input, size << 3);
}
  
void hash_sha3_x4_finalize(uint8_t **output, hash_sha3_x4_ctx *ctx) {
    Keccak_HashFinaltimes4(ctx, output);
}

void seedexpander_shake_x4_init(seedexpander_shake_x4_t* seedexpander_shake, const uint8_t** seed, size_t seed_size, const uint8_t** salt, size_t salt_size) {
    Keccak_HashInitializetimes4_SHAKE256(&seedexpander_shake->state);
    Keccak_HashUpdatetimes4(&seedexpander_shake->state, seed, seed_size << 3);
    if(salt != NULL) Keccak_HashUpdatetimes4(&seedexpander_shake->state, salt, salt_size << 3);
    Keccak_HashFinaltimes4(&seedexpander_shake->state, NULL);
}

void seedexpander_shake_x4_get_bytes(seedexpander_shake_x4_t* seedexpander_shake, uint8_t** output, size_t output_size) {
    Keccak_HashSqueezetimes4(&seedexpander_shake->state, output, output_size << 3);
}