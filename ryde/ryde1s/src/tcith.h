/**
 * @file tcith.h
 * @brief Functions concerning the TCitH part of the RYDE scheme
 */

#ifndef RYDE_TCITH_H
#define RYDE_TCITH_H

#include <stdio.h>
#include <stdint.h>
#include "field_matrix.h"
#include "binary_matrix.h"
#include "parameters.h"

#define DOMAIN_SEPARATOR_MESSAGE 0
#define DOMAIN_SEPARATOR_HASH1 1
#define DOMAIN_SEPARATOR_HASH2 2

#define RYDE_BLOCK_LENGTH ((RYDE_VEC_R_MINUS_ONE_BYTES + RYDE_MAT_FQ_BYTES + RYDE_VEC_RHO_BYTES + (RYDE_SECURITY_BYTES - 1)) / RYDE_SECURITY_BYTES)

#if (RYDE_BLOCK_LENGTH > 0xFF)
#error RYDE_BLOCK_LENGTH must fit in uint8_t
#endif

typedef struct {
    field_vector_t s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1];
    field_matrix_t C[RYDE_PARAM_TAU][RYDE_PARAM_R];
    field_vector_t v[RYDE_PARAM_TAU][RYDE_PARAM_RHO];
} ryde_tcith_shares_t;

typedef uint8_t ryde_tcith_alpha_t[RYDE_VEC_RHO_BYTES];
typedef uint8_t ryde_tcith_share_s_t[RYDE_VEC_R_MINUS_ONE_BYTES];
typedef uint8_t ryde_tcith_share_C_t[RYDE_MAT_FQ_BYTES];
typedef size_t ryde_tcith_challenge_t[RYDE_PARAM_TAU];

typedef uint8_t ryde_tcith_seed_t[RYDE_SECURITY_BYTES];
typedef uint8_t ryde_tcith_commit_t[RYDE_HASH_BYTES];

void ryde_tcith_phi(field_t phi_i, size_t i);
size_t ryde_tcith_psi(size_t i, size_t e);
void ryde_tcith_commit(ryde_tcith_commit_t commit, const uint8_t *salt, uint8_t e, size_t i, const uint8_t *seed);
void ryde_tcith_expand_share(field_vector_t *s, binary_matrix_t *C, field_vector_t *v, const uint8_t *seed, const uint8_t *salt);
void ryde_tcith_expand_challenge_1(field_matrix_t *challenge, const uint8_t *seed_input, const uint8_t *salt);
void ryde_tcith_expand_challenge_2(ryde_tcith_challenge_t *challenge, uint8_t *v_grinding, const uint8_t *string_input);
uint8_t ryde_tcith_discard_input_challenge_2(const uint8_t *v_grinding);

#ifdef _SHAKE_TIMES4_
#define SHAKE_STEP 4
void ryde_tcith_expand_challenge_2_x4(ryde_tcith_challenge_t *challenge, uint8_t *v_grinding, const uint8_t *string_input);
#else
#define SHAKE_STEP 1
#endif

void ryde_tcith_compute_polynomial_proof(field_vector_t base_a[RYDE_PARAM_RHO],
                                         field_vector_t mid_a[RYDE_PARAM_RHO],
                                         ryde_tcith_shares_t base,
                                         const field_vector_t s[RYDE_PARAM_R],
                                         const binary_matrix_t C[RYDE_PARAM_R],
                                         const field_vector_t acc_v[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
                                         const field_matrix_t gamma[RYDE_PARAM_N - RYDE_PARAM_K],
                                         const field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K],
                                         size_t e);

void ryde_tcith_recompute_polynomial_proof(field_vector_t base_a[RYDE_PARAM_RHO],
                                           const ryde_tcith_challenge_t i_star,
                                           ryde_tcith_shares_t shares,
                                           const field_matrix_t gamma[RYDE_PARAM_N - RYDE_PARAM_K],
                                           const field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K],
                                           const field_vector_t y[RYDE_PARAM_N - RYDE_PARAM_K],
                                           const field_vector_t mid_a[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
                                           size_t e);

/**
 * \fn static inline void ryde_tcith_shift_to_right_array(uint8_t *string, const size_t length, const int nbits)
 * \brief This function performs a shift to right of the input.
 *
 * \param[in/out] inout_a uint8_t* Representation of a byte string
 * \param[in] length size_t Representation of the byte string length
 * \param[in] nbits size_t Representation of number of bits to be shifted
 */
static inline void ryde_tcith_shift_to_right_array(uint8_t *string, const size_t length, const int nbits) {
    int bits = (nbits > 7) ? 7 : nbits;
    for (int acc = 0; acc < nbits;) {
        for (size_t i = 0; i < length - 1; i++) {
            string[i] = (string[i] >> bits) | (string[i + 1] << (8 - bits));
        }
        string[length - 1] >>= bits;

        acc += bits;
        const int rem = nbits - acc;
        bits = (rem > 7) ? 7 : rem;
    }
}

void ryde_pack_matrices_and_vectors(uint8_t *output,
                                    const field_vector_t aux_s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1],
                                    const binary_matrix_t aux_C[RYDE_PARAM_TAU][RYDE_PARAM_R],
                                    const field_vector_t mid_alpha[RYDE_PARAM_TAU][RYDE_PARAM_RHO]);
void ryde_unpack_matrices_and_vectors(field_vector_t aux_s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1],
                                      binary_matrix_t aux_C[RYDE_PARAM_TAU][RYDE_PARAM_R],
                                      field_vector_t mid_alpha[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
                                      const uint8_t *input);

#endif //RYDE_TCITH_H
