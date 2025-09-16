/**
 * @file field_ur_61.h
 * @brief Polynomial multiplication over GF(2) of the RYDE scheme
 */

#ifndef RYDE_FIELD_UR_H
#define RYDE_FIELD_UR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RYDE_FIELD_UR_M        61
#define RYDE_FIELD_UR_WORDS    1

/**
 * \fn static inline void field_ur_mul(uint64_t o[2 * RYDE_FIELD_UR_WORDS], const uint64_t e1[RYDE_FIELD_UR_WORDS], const uint64_t e2[RYDE_FIELD_UR_WORDS])
 * \brief This function computes the unreduced multiplication of two finite field elements.
 *
 * \param[out] o Product of <b>e1</b> and <b>e2</b>
 * \param[in] e1 uint64_t array of size RYDE_FIELD_UR_WORDS
 * \param[in] e2 uint64_t array of size RYDE_FIELD_UR_WORDS
 */
static inline void field_ur_mul(uint64_t o[2 * RYDE_FIELD_UR_WORDS], const uint64_t e1[RYDE_FIELD_UR_WORDS], const uint64_t e2[RYDE_FIELD_UR_WORDS]) {
    uint64_t shifts[64][RYDE_FIELD_UR_WORDS + 1];
    for(uint8_t i=0 ; i<RYDE_FIELD_UR_WORDS ; i++) {
        shifts[0][i] = e2[i];
    }
    shifts[0][RYDE_FIELD_UR_WORDS] = 0;

    for(uint8_t shift=1 ; shift<64 ; shift++) {
        shifts[shift][0] = shifts[shift-1][0] << 1;
        for(uint8_t i=1 ; i<RYDE_FIELD_UR_WORDS + 1 ; i++) {
            shifts[shift][i] = (shifts[shift-1][i] << 1) | (shifts[shift-1][i-1] >> 63);
        }
    }

    for(uint8_t i=0 ; i<(2*RYDE_FIELD_UR_WORDS) ; i++) {
        o[i] = 0;
    }
    for(uint8_t i=0 ; i<RYDE_FIELD_UR_M ; i++) {
        uint8_t shift = i % 64;
        uint8_t offset = i / 64;
        uint64_t multiplier = (e1[offset] >> shift) & 0x1;
        for(uint8_t j=0 ; j<RYDE_FIELD_UR_WORDS + 1 ; j++) {
            o[j+offset] ^= multiplier * shifts[shift][j];
        }
    }
}

/**
 * \fn static inline void field_ur_sqr(uint64_t o[2 * RYDE_FIELD_UR_WORDS], const uint64_t e[RYDE_FIELD_UR_WORDS])
 * \brief This function computes the unreduced square of a finite field element.
 *
 * \param[out] o Square of <b>e</b>
 * \param[in]  e uint64_t array of size RYDE_FIELD_UR_WORDS
*/
static inline void field_ur_sqr(uint64_t o[2 * RYDE_FIELD_UR_WORDS], const uint64_t e[RYDE_FIELD_UR_WORDS]) {
    field_ur_mul(o, e, e);
}

#endif //RYDE_FIELD_UR_H
