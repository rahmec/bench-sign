/**
 * @file field_ur_67.h
 * @brief Polynomial multiplication over GF(2) of the RYDE scheme
 */

#ifndef RYDE_FIELD_UR_H
#define RYDE_FIELD_UR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x86intrin.h>

#define RYDE_FIELD_UR_WORDS    2

static const __m128i RYDE_FIELD_UR_SQUARE_MASK = {
    0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F
};
static const __m128i RYDE_FIELD_UR_SQUARE_LOOKUP_TABLE = {
    0x1514111005040100, 0x5554515045444140
};


/**
 * \fn static inline void field_ur_mul(bigint_ur_t o, const bigint_t e1, const bigint_t e2)
 * \brief This function computes the unreduced multiplication of two finite field elements.
 *
 * \param[out] o Product of <b>e1</b> and <b>e2</b>
 * \param[in] e1 uint64_t array of size RYDE_FIELD_UR_WORDS
 * \param[in] e2 uint64_t array of size RYDE_FIELD_UR_WORDS
 */
static inline void field_ur_mul(uint64_t o[2 * RYDE_FIELD_UR_WORDS], const uint64_t e1[RYDE_FIELD_UR_WORDS], const uint64_t e2[RYDE_FIELD_UR_WORDS]) {
    __m128i a = _mm_load_si128((__m128i*) e1);
    __m128i b = _mm_load_si128((__m128i*) e2);
  
    __m128i a0_b0 = _mm_clmulepi64_si128(a, b, 0x00);
  
    __m128i a0_b1 = _mm_clmulepi64_si128(a, b, 0x10);
    __m128i a1_b0 = _mm_clmulepi64_si128(a, b, 0x01);
  
    __m128i a1_b1 = _mm_clmulepi64_si128(a, b, 0x11);
  
    __m128i c1 = _mm_xor_si128(a0_b1, a1_b0);
  
    o[0] = _mm_extract_epi64(a0_b0, 0);
    o[1] = _mm_extract_epi64(a0_b0, 1) ^ _mm_extract_epi64(c1, 0);
    o[2] = _mm_extract_epi64(a1_b1, 0) ^ _mm_extract_epi64(c1, 1);
    o[3] = _mm_extract_epi64(a1_b1, 1);
}


/**
 * \fn static inline void field_ur_sqr(uint64_t o[2 * RYDE_FIELD_UR_WORDS], const uint64_t e[RYDE_FIELD_UR_WORDS])
 * \brief This function computes the unreduced square of a finite field element.
 *
 * \param[out] o Square of <b>e</b>
 * \param[in]  e uint64_t array of size RYDE_FIELD_UR_WORDS
*/
static inline void field_ur_sqr(uint64_t o[2 * RYDE_FIELD_UR_WORDS], const uint64_t e[RYDE_FIELD_UR_WORDS]) {
    __m128i tmp1, e_l, e_h;
    __m128i tmp2[2];
  
    tmp1 = _mm_load_si128((__m128i*) e);
    e_l = _mm_and_si128(tmp1, RYDE_FIELD_UR_SQUARE_MASK);
    e_h = _mm_and_si128(_mm_srli_epi64(tmp1, 4), RYDE_FIELD_UR_SQUARE_MASK);
  
    e_l = _mm_shuffle_epi8(RYDE_FIELD_UR_SQUARE_LOOKUP_TABLE, e_l);
    e_h = _mm_shuffle_epi8(RYDE_FIELD_UR_SQUARE_LOOKUP_TABLE, e_h);
  
    tmp2[0] = _mm_unpacklo_epi8(e_l, e_h);
    tmp2[1] = _mm_unpackhi_epi8(e_l, e_h);
  
    _mm_store_si128((__m128i*) o, tmp2[0]);
    _mm_store_si128((__m128i*) (o + 2), tmp2[1]);
}

#endif //RYDE_FIELD_UR_H
