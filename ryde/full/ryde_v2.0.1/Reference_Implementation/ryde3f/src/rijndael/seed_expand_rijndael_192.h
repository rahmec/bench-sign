#ifndef SEED_EXPAND_RIJNDAEL_H
#define SEED_EXPAND_RIJNDAEL_H

#include <stdint.h>
#include "seed_expand_functions.h"

static inline void expand_seed_rijndael(uint8_t dst[2][24], const uint8_t salt[24], const uint32_t idx, const uint8_t seed[24]) {
    rijndael_192_expand_seed(dst, salt, idx, seed);
}

static inline void commit_rijndael(uint8_t dst[2][24], const uint8_t salt[24], const uint32_t idx, const uint8_t seed[24]) {
    rijndael_192_commit(dst, salt, idx, seed);
}

static inline void expand_share_rijndael(uint8_t (*dst)[24], const uint8_t salt[24], const uint8_t seed[24], uint8_t len) {
    rijndael_192_expand_share(dst, salt, seed, len);
}

#endif //SEED_EXPAND_RIJNDAEL_H
