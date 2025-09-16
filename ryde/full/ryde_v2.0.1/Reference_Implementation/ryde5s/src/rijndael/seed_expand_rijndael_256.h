#ifndef SEED_EXPAND_RIJNDAEL_H
#define SEED_EXPAND_RIJNDAEL_H

#include <stdint.h>
#include "seed_expand_functions.h"

static inline void expand_seed_rijndael(uint8_t dst[2][32], const uint8_t salt[32], const uint32_t idx, const uint8_t seed[32]) {
    rijndael_256_expand_seed(dst, salt, idx, seed);
}

static inline void commit_rijndael(uint8_t dst[2][32], const uint8_t salt[32], const uint32_t idx, const uint8_t seed[32]) {
    rijndael_256_commit(dst, salt, idx, seed);
}

static inline void expand_share_rijndael(uint8_t (*dst)[32], const uint8_t salt[32], const uint8_t seed[32], uint8_t len) {
    rijndael_256_expand_share(dst, salt, seed, len);
}

#endif //SEED_EXPAND_RIJNDAEL_H
