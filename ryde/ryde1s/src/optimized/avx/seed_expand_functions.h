/**
 * @file seed_expand_functions.h
 * @brief Content for seed_expand_functions.h (Seed expand functions based on AES-128 and Rijndael-256)
 */

#ifndef SEED_EXPAND_FUNCTIONS_H
#define SEED_EXPAND_FUNCTIONS_H

#include "rijndael.h"
#define DOMAIN_SEPARATOR_CMT 3
#define DOMAIN_SEPARATOR_PRG 4

typedef uint8_t block128_t[16] __attribute__ ((aligned (16)));
typedef uint8_t block256_t[32] __attribute__ ((aligned (32)));

static inline void aes_128_expand_seed(uint8_t dst[2][16], const uint8_t salt[16], const uint32_t idx, const uint8_t seed[16]) {
    // We assume that the output dst contains zeros

    uint8_t domain_separator = (uint8_t)DOMAIN_SEPARATOR_PRG;
    block128_t msg;
    block128_t key[AES128_ROUNDS + 1];
    block128_t dst_vec[2]; // memory must be aligned
    block128_t seed_vec;

    *((__m128i*)msg) = _mm_setzero_si128();
    *((__m128i*)dst_vec[0]) = _mm_setzero_si128();
    *((__m128i*)dst_vec[1]) = _mm_setzero_si128();

    for (uint32_t i = 0; i < AES128_ROUNDS + 1; i++) {
        *((__m128i*)key[i]) = _mm_setzero_si128();
    }

    memcpy(seed_vec, seed, 16);

    aes_128_key_expansion((uint8_t *)key, seed_vec);

    // salt ^ (domain_separator || idx || 0)
    memcpy(msg, salt, sizeof(uint8_t) * 16);
    msg[0] ^= 0x00;

    msg[1] ^= ((uint8_t *)&idx)[0];
    msg[2] ^= ((uint8_t *)&idx)[1];
    msg[3] ^= ((uint8_t *)&idx)[2];
    msg[4] ^= ((uint8_t *)&idx)[3];

    msg[5] ^= domain_separator;
    aes_128_encrypt(dst_vec[0], msg, (uint8_t*)key);

    // salt ^ (domain_separator || idx || 1)
    msg[0] ^= 0x01;
    aes_128_encrypt(dst_vec[1], msg, (uint8_t*)key);

    memcpy(dst[0], dst_vec[0], sizeof(block128_t));
    memcpy(dst[1], dst_vec[1], sizeof(block128_t));
}

static inline void rijndael_192_expand_seed(uint8_t dst[2][24], const uint8_t salt[24], const uint32_t idx, const uint8_t seed[24]) {
    // We assume that the output dst contains zeros

    uint8_t domain_separator = (uint8_t)DOMAIN_SEPARATOR_PRG;
    block256_t msg;
    block256_t key[RIJNDAEL256_ROUNDS + 1];
    block256_t dst_vec[2];
    block256_t seed_vec;

    *((__m256i*)msg) = _mm256_setzero_si256();
    *((__m256i*)dst_vec[0]) = _mm256_setzero_si256();
    *((__m256i*)dst_vec[1]) = _mm256_setzero_si256();
    *((__m256i*)seed_vec) = _mm256_setzero_si256();

    for (uint32_t i = 0; i < RIJNDAEL256_ROUNDS + 1; i++) {
        *((__m256i*)key[i]) = _mm256_setzero_si256();
    }

    memcpy(seed_vec, seed, sizeof(uint8_t) * 24);

    rijndael_256_key_expansion((uint8_t *)key, seed_vec);

    // salt ^ (domain_separator || idx || 0)
    memcpy(msg, salt, sizeof(uint8_t) * 24);
    msg[0] ^= 0x00;

    msg[1] ^= ((uint8_t *)&idx)[0];
    msg[2] ^= ((uint8_t *)&idx)[1];
    msg[3] ^= ((uint8_t *)&idx)[2];
    msg[4] ^= ((uint8_t *)&idx)[3];

    msg[5] ^= domain_separator;
    rijndael_256_encrypt(dst_vec[0], msg, (uint8_t*)key);

    // salt ^ (domain_separator || idx || 1)
    msg[0] ^= 0x01;
    rijndael_256_encrypt(dst_vec[1], msg, (uint8_t*)key);

    memcpy(dst[0], dst_vec[0], sizeof(uint8_t) * 24);
    memcpy(dst[1], dst_vec[1], sizeof(uint8_t) * 24);

}

static inline void rijndael_256_expand_seed(uint8_t dst[2][32], const uint8_t salt[32], const uint32_t idx, const uint8_t seed[32]) {
    // We assume that the output dst contains zeros

    uint8_t domain_separator = (uint8_t)DOMAIN_SEPARATOR_PRG;
    block256_t msg;
    block256_t key[RIJNDAEL256_ROUNDS + 1];
    block256_t dst_vec[2];
    block256_t seed_vec;

    *((__m256i*)msg) = _mm256_setzero_si256();
    *((__m256i*)dst_vec[0]) = _mm256_setzero_si256();
    *((__m256i*)dst_vec[1]) = _mm256_setzero_si256();

    for (uint32_t i = 0; i < RIJNDAEL256_ROUNDS + 1; i++) {
        *((__m256i*)key[i]) = _mm256_setzero_si256();
    }

    memcpy(seed_vec, seed, 32);

    rijndael_256_key_expansion((uint8_t *)key, seed_vec);

    // salt ^ (domain_separator || idx || 0)
    memcpy(msg, salt, sizeof(uint8_t) * 32);
    msg[0] ^= 0x00;

    msg[1] ^= ((uint8_t *)&idx)[0];
    msg[2] ^= ((uint8_t *)&idx)[1];
    msg[3] ^= ((uint8_t *)&idx)[2];
    msg[4] ^= ((uint8_t *)&idx)[3];

    msg[5] ^= domain_separator;
    rijndael_256_encrypt(dst_vec[0], msg, (uint8_t*)key);

    // salt ^ (domain_separator || idx || 1)
    msg[0] ^= 0x01;
    rijndael_256_encrypt(dst_vec[1], msg, (uint8_t*)key);

    memcpy(dst[0], dst_vec[0], sizeof(block256_t));
    memcpy(dst[1], dst_vec[1], sizeof(block256_t));

}

static inline void aes_128_commit(uint8_t dst[2][16], const uint8_t salt[16], const uint32_t idx, const uint8_t seed[16]) {
    // We assume that the output dst contains zeros

    uint8_t domain_separator = (uint8_t)DOMAIN_SEPARATOR_CMT;
    block128_t msg;
    block128_t key[AES128_ROUNDS + 1];
    block128_t dst_vec[2]; // memory must be aligned

    *((__m128i*)msg) = _mm_setzero_si128();
    *((__m128i*)dst_vec[0]) = _mm_setzero_si128();
    *((__m128i*)dst_vec[1]) = _mm_setzero_si128();

    aes_128_key_expansion((uint8_t *)key, seed);

    // salt ^ (domain_separator || idx || 0)
    memcpy(msg, salt, sizeof(uint8_t) * 16);
    msg[0] ^= 0x00;

    msg[1] ^= ((uint8_t *)&idx)[0];
    msg[2] ^= ((uint8_t *)&idx)[1];
    msg[3] ^= ((uint8_t *)&idx)[2];
    msg[4] ^= ((uint8_t *)&idx)[3];

    msg[5] ^= domain_separator;
    aes_128_encrypt(dst_vec[0], msg, (uint8_t*)key);

    // salt ^ (domain_separator || idx || 1)
    msg[0] ^= 0x01;
    aes_128_encrypt(dst_vec[1], msg, (uint8_t*)key);

    memcpy(dst[0], dst_vec[0], sizeof(block128_t));
    memcpy(dst[1], dst_vec[1], sizeof(block128_t));
}

static inline void rijndael_192_commit(uint8_t dst[2][24], const uint8_t salt[24], const uint32_t idx, const uint8_t seed[24]) {
    // We assume that the output dst contains zeros

    uint8_t domain_separator = (uint8_t)DOMAIN_SEPARATOR_CMT;
    block256_t msg;
    block256_t key[RIJNDAEL256_ROUNDS + 1];
    block256_t dst_vec[2];
    block256_t seed_vec;

    *((__m256i*)msg) = _mm256_setzero_si256();
    *((__m256i*)dst_vec[0]) = _mm256_setzero_si256();
    *((__m256i*)dst_vec[1]) = _mm256_setzero_si256();
    *((__m256i*)seed_vec) = _mm256_setzero_si256();

    for (uint32_t i = 0; i < RIJNDAEL256_ROUNDS + 1; i++) {
        *((__m256i*)key[i]) = _mm256_setzero_si256();
    }

    memcpy(seed_vec, seed, sizeof(uint8_t) * 24);

    rijndael_256_key_expansion((uint8_t *)key, seed_vec);

    // salt ^ (domain_separator || idx || 0)
    memcpy(msg, salt, sizeof(uint8_t) * 24);
    msg[0] ^= 0x00;

    msg[1] ^= ((uint8_t *)&idx)[0];
    msg[2] ^= ((uint8_t *)&idx)[1];
    msg[3] ^= ((uint8_t *)&idx)[2];
    msg[4] ^= ((uint8_t *)&idx)[3];

    msg[5] ^= domain_separator;
    rijndael_256_encrypt(dst_vec[0], msg, (uint8_t*)key);

    // salt ^ (domain_separator || idx || 1)
    msg[0] ^= 0x01;
    rijndael_256_encrypt(dst_vec[1], msg, (uint8_t*)key);

    memcpy(dst[0], dst_vec[0], sizeof(uint8_t) * 24);
    memcpy(dst[1], dst_vec[1], sizeof(uint8_t) * 24);

}

static inline void rijndael_256_commit(uint8_t dst[2][32], const uint8_t salt[32], const uint32_t idx, const uint8_t seed[32]) {
    // We assume that the output dst contains zeros

    uint8_t domain_separator = (uint8_t)DOMAIN_SEPARATOR_CMT;
    block256_t msg;
    block256_t key[RIJNDAEL256_ROUNDS + 1];
    block256_t dst_vec[2];
    block256_t seed_vec;

    *((__m256i*)msg) = _mm256_setzero_si256();
    *((__m256i*)dst_vec[0]) = _mm256_setzero_si256();
    *((__m256i*)dst_vec[1]) = _mm256_setzero_si256();

    for (uint32_t i = 0; i < RIJNDAEL256_ROUNDS + 1; i++) {
        *((__m256i*)key[i]) = _mm256_setzero_si256();
    }

    memcpy(seed_vec, seed, 32);

    rijndael_256_key_expansion((uint8_t *)key, seed_vec);

    // salt ^ (domain_separator || idx || 0)
    memcpy(msg, salt, sizeof(uint8_t) * 32);
    msg[0] ^= 0x00;

    msg[1] ^= ((uint8_t *)&idx)[0];
    msg[2] ^= ((uint8_t *)&idx)[1];
    msg[3] ^= ((uint8_t *)&idx)[2];
    msg[4] ^= ((uint8_t *)&idx)[3];

    msg[5] ^= domain_separator;
    rijndael_256_encrypt(dst_vec[0], msg, (uint8_t*)key);

    // salt ^ (domain_separator || idx || 1)
    msg[0] ^= 0x01;
    rijndael_256_encrypt(dst_vec[1], msg, (uint8_t*)key);

    memcpy(dst[0], dst_vec[0], sizeof(block256_t));
    memcpy(dst[1], dst_vec[1], sizeof(block256_t));

}

static inline void aes_128_expand_share(uint8_t (*dst)[16], const uint8_t salt[16], const uint8_t seed[16], uint8_t len) {
    // This function assumes dst has capacity len, and that the len is at most 255

    block128_t key[AES128_ROUNDS + 1];
    block128_t ctr;
    block128_t seed_vec;
    block128_t salt_vec;

    *((__m128i*)ctr) = _mm_setzero_si128();
    for (uint32_t i = 0; i < AES128_ROUNDS + 1; i++) {
        *((__m128i*)key[i]) = _mm_setzero_si128();
    }

    memcpy(seed_vec, seed, sizeof(block128_t));
    memcpy(salt_vec, salt, sizeof(block128_t));

    aes_128_key_expansion((uint8_t *)key, seed_vec);

    for (uint8_t i = 0; i < len; i++) {
        ctr[0] = i;
        block128_t msg;
        *((__m128i*)msg) = _mm_setzero_si128();
        ((__m128i *)msg)[0] = _mm_xor_si128(*(__m128i *)ctr, ((__m128i *)salt_vec)[0]);

        block128_t output;
        *((__m128i*)output) = _mm_setzero_si128();
        aes_128_encrypt(output, msg, (uint8_t*)key);
        memcpy(dst[i], output, sizeof(uint8_t) * 16);
    }
}

static inline void rijndael_192_expand_share(uint8_t (*dst)[24], const uint8_t salt[24], const uint8_t seed[24], uint8_t len) {
    // This function assumes dst has capacity len, and that the len is at most 255

    block256_t key[RIJNDAEL256_ROUNDS + 1];
    block256_t ctr;
    block256_t seed_vec;
    block256_t salt_vec;

    *((__m256i*)ctr) = _mm256_setzero_si256();
    *((__m256i*)salt_vec) = _mm256_setzero_si256();
    *((__m256i*)seed_vec) = _mm256_setzero_si256();

    for (uint32_t i = 0; i < RIJNDAEL256_ROUNDS + 1; i++) {
        *((__m256i*)key[i]) = _mm256_setzero_si256();
    }

    memcpy(seed_vec, seed, sizeof(uint8_t) * 24);
    memcpy(salt_vec, salt, sizeof(uint8_t) * 24);

    rijndael_256_key_expansion((uint8_t *)key, seed_vec);

    for (uint8_t i = 0; i < len; i++) {
        ctr[0] = i;
        block256_t msg;
        *((__m256i*)msg) = _mm256_setzero_si256();
        ((__m256i *)msg)[0] = _mm256_xor_si256(*(__m256i *)ctr, ((__m256i *)salt_vec)[0]);

        block256_t output;
        *((__m256i*)output) = _mm256_setzero_si256();
        rijndael_256_encrypt(output, msg, (uint8_t*)key);
        memcpy(dst[i], output, sizeof(uint8_t) * 24);
    }
}

static inline void rijndael_256_expand_share(uint8_t (*dst)[32], const uint8_t salt[32], const uint8_t seed[32], uint8_t len) {
    // This function assumes dst has capacity len, and that the len is at most 255

    block256_t key[RIJNDAEL256_ROUNDS + 1];
    block256_t ctr;
    block256_t seed_vec;
    block256_t salt_vec;

    *((__m256i*)ctr) = _mm256_setzero_si256();
    *((__m256i*)salt_vec) = _mm256_setzero_si256();
    *((__m256i*)seed_vec) = _mm256_setzero_si256();

    for (uint32_t i = 0; i < RIJNDAEL256_ROUNDS + 1; i++) {
        *((__m256i*)key[i]) = _mm256_setzero_si256();
    }

    memcpy(seed_vec, seed, sizeof(block256_t));
    memcpy(salt_vec, salt, sizeof(block256_t));

    rijndael_256_key_expansion((uint8_t *)key, seed_vec);

    for (uint8_t i = 0; i < len; i++) {
        ctr[0] = i;
        block256_t msg;
        *((__m256i*)msg) = _mm256_setzero_si256();
        ((__m256i *)msg)[0] = _mm256_xor_si256(*(__m256i *)ctr, ((__m256i *)salt_vec)[0]);

        block256_t output;
        *((__m256i*)output) = _mm256_setzero_si256();
        rijndael_256_encrypt(output, msg, (uint8_t*)key);
        memcpy(dst[i], output, sizeof(uint8_t) * 32);
    }
}

#endif //SEED_EXPAND_FUNCTIONS_H