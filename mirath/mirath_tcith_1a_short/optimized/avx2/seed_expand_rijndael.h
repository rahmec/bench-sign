/**
 * @file seed_expand_functions_avx.h
 * @brief Content for seed_expand_functions_avx.h (Seed expand functions based on AES-128)
 */

#ifndef SEED_EXPAND_RIJNDAEL_128_H
#define SEED_EXPAND_RIJNDAEL_128_H

#include "rijndael.h"
#define DOMAIN_SEPARATOR_PRG 4
#define DOMAIN_SEPARATOR_CMT 3

typedef uint8_t block128_t[16] __attribute__ ((aligned (16)));

static inline void rijndael_expand_seed(uint8_t dst[2][16], const uint8_t salt[16], const uint32_t idx, const uint8_t seed[16]) {
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

static inline void rijndael_commit(uint8_t dst[2][16], const uint8_t salt[16], const uint32_t idx, const uint8_t seed[16]) {
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

static inline void rijndael_expand_share(uint8_t (*dst)[16], const uint8_t salt[16], const uint8_t seed[16], uint8_t len) {
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

#endif //SEED_EXPAND_RIJNDAEL_128_H
