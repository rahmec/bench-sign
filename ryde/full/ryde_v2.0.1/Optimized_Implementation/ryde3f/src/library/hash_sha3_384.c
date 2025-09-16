#include "hash_sha3.h"

void hash_sha3_init(hash_sha3_ctx *ctx) {
    Keccak_HashInitialize_SHA3_384(ctx);
}

void hash_sha3_absorb(hash_sha3_ctx *ctx, const uint8_t *input, size_t size) {
    Keccak_HashUpdate(ctx, input, size << 3);
}

void hash_sha3_finalize(uint8_t *output, hash_sha3_ctx *ctx) {
    Keccak_HashFinal(ctx, output);
}

void seedexpander_shake_init(seedexpander_shake_t* seedexpander_shake, const uint8_t* seed, size_t seed_size, const uint8_t* salt, size_t salt_size) {
    Keccak_HashInitialize_SHAKE256(&seedexpander_shake->state);
    Keccak_HashUpdate(&seedexpander_shake->state, seed, seed_size << 3);
    if(salt != NULL) Keccak_HashUpdate(&seedexpander_shake->state, salt, salt_size << 3);
    Keccak_HashFinal(&seedexpander_shake->state, NULL);
}
  
void seedexpander_shake_get_bytes(seedexpander_shake_t* seedexpander_shake, uint8_t* output, size_t output_size) {
    Keccak_HashSqueeze(&seedexpander_shake->state, output, output_size << 3);
}

