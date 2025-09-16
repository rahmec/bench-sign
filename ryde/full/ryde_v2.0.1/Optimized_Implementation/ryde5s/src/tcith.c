/**
 * @file tcith.c
 * @brief Implementation of tcith.h
 */

#include "tcith.h"
#include "ggm_tree.h"
#include "hash_sha3.h"
#ifdef _SHAKE_TIMES4_
#include "hash_sha3_x4.h"
#endif
#include "seed_expand_rijndael.h"

#ifndef _SHA3_
static inline void ryde_commit(uint8_t pair_node[2][RYDE_SECURITY_BYTES],
                               const uint8_t salt[RYDE_SECURITY_BYTES],
                               const uint32_t i,
                               const uint8_t seed[RYDE_SECURITY_BYTES]) {
    commit_rijndael(pair_node, salt, RYDE_PARAM_TREE_LEAVES + i, seed);
}
#else
static inline void ryde_commit(uint8_t pair_node[2][RYDE_SECURITY_BYTES],
                               const uint8_t salt[RYDE_SALT_BYTES],
                               const uint32_t i,
                               const uint8_t seed[RYDE_SECURITY_BYTES]) {
    uint8_t domain_separator = DOMAIN_SEPARATOR_CMT;
    hash_sha3_ctx ctx;
    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, salt, RYDE_SALT_BYTES);
    hash_sha3_absorb(&ctx, (uint8_t * ) & i, sizeof(uint32_t));
    hash_sha3_absorb(&ctx, seed, RYDE_SECURITY_BYTES);
    hash_sha3_finalize((uint8_t *)pair_node, &ctx);
}
#endif


static inline void ryde_expand_share(uint8_t sample[RYDE_BLOCK_LENGTH][RYDE_SECURITY_BYTES],
                                     const uint8_t salt[RYDE_SECURITY_BYTES],
                                     const uint8_t seed[RYDE_SECURITY_BYTES],
                                     uint8_t length) {
    expand_share_rijndael(sample, salt, seed, length);
}


/**
* \fn void ryde_tcith_phi(field_t phi_i, size_t i)
* \brief This function implements the mapping phi
*
* \param[out] phi field_t Representation of element phi(i)
* \param[in] i Integer corresponding with position i
*/
void ryde_tcith_phi(field_t phi_i, size_t i) {
    size_t i_cast[RYDE_PARAM_M_WORDS] = {0};
    i_cast[0] = i + 1;
    field_copy(phi_i, (uint64_t *)i_cast);
}


/**
* \fn void ryde_tcith_psi(size_t i, size_t e)
* \brief This function implements the mapping psi
*
* \param[in] i Integer corresponding with position i
* \param[in] e Integer corresponding with iteration e
*/
size_t ryde_tcith_psi(size_t i, size_t e) {
    if (i < RYDE_PARAM_N_2) {
        return i * RYDE_PARAM_TAU + e;
    }
    else {
        return RYDE_PARAM_N_2 * RYDE_PARAM_TAU + (i - RYDE_PARAM_N_2) * RYDE_PARAM_TAU_1 + e;
    }
}


/**
* \fn void ryde_tcith_commit(ryde_tcith_commit_t commit, const uint8_t *salt, uint8_t e, size_t i, const uint8_t *seed)
* \brief This function calculates one commitment
*
* \param[out] commit ryde_tcith_commit_t Representation of the commitment(s)
* \param[in] salt String containing the salt
* \param[in] e Integer corresponding with iteration e
* \param[in] i Integer corresponding with position i
* \param[in] seed String containing the input seed
*/
void ryde_tcith_commit(ryde_tcith_commit_t commit, const uint8_t *salt, uint8_t e, size_t i, const uint8_t *seed) {
    size_t idx = ryde_tcith_psi(i, e);
    ryde_commit((uint8_t (*)[RYDE_SECURITY_BYTES])commit, salt, (uint32_t)idx, seed);
}


/**
* \fn void ryde_tcith_expand_share(field_vector_t *s, binary_matrix_t *C, field_vector_t *v, const uint8_t *seed, const uint8_t *salt)
* \brief This function samples (s, C, v) from an input seed and salt
*
* \param[out] s field_vector_t* Representation of vector s
* \param[out] C binary_matrix_t* Representation of matrix C
* \param[out] v field_vector_t* Representation of vector v
* \param[in] seed String containing the input seed
* \param[in] salt String containing the salt
*/
void ryde_tcith_expand_share(field_vector_t *s, binary_matrix_t *C, field_vector_t *v, const uint8_t *seed, const uint8_t *salt) {
    // Remark: (RYDE_VEC_R_MINUS_ONE_BYTES + RYDE_MAT_FQ_BYTES + RYDE_VEC_RHO_BYTES) is less than or equal to (RYDE_BLOCK_LENGTH * RYDE_SECURITY_BYTES)
    uint8_t random[RYDE_BLOCK_LENGTH * RYDE_SECURITY_BYTES] = {0};
    ryde_expand_share((uint8_t (*)[RYDE_SECURITY_BYTES])random, salt, seed, RYDE_BLOCK_LENGTH);

    random[RYDE_VEC_R_MINUS_ONE_BYTES - 1] &= RYDE_VEC_R_MINUS_ONE_MASK;
    random[RYDE_VEC_R_MINUS_ONE_BYTES + RYDE_MAT_FQ_BYTES - 1] &= RYDE_MAT_FQ_MASK;
    random[RYDE_VEC_R_MINUS_ONE_BYTES + RYDE_MAT_FQ_BYTES + RYDE_VEC_RHO_BYTES - 1] &= RYDE_VEC_RHO_MASK;

    field_vector_from_string(s, RYDE_PARAM_R - 1, &random[0]);
    binary_matrix_from_string(C, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R, &random[RYDE_VEC_R_MINUS_ONE_BYTES]);
    field_vector_from_string(v, RYDE_PARAM_RHO, &random[RYDE_VEC_R_MINUS_ONE_BYTES + RYDE_MAT_FQ_BYTES]);
}


/**
* \fn void ryde_tcith_expand_challenge_1(field_matrix_t *challenge, const uint8_t *seed_input, const uint8_t *salt)
* \brief This function generates the first challenges from an input seed
*
* \param[out] challenge field_matrix_t* Representation of matrix challenge
* \param[in] seed_input String containing the input seed
* \param[in] salt String containing the salt
*/
void ryde_tcith_expand_challenge_1(field_matrix_t *challenge, const uint8_t *seed_input, const uint8_t *salt) {

    uint8_t random[RYDE_PARAM_CHALLENGE_1_BYTES] = {0};
    seedexpander_shake_t seedexpander;
    seedexpander_shake_init(&seedexpander, seed_input, RYDE_HASH_BYTES, salt, RYDE_SALT_BYTES);

    seedexpander_shake_get_bytes(&seedexpander, random, RYDE_PARAM_CHALLENGE_1_BYTES);
    field_matrix_from_string(challenge, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_RHO, random);

    memset(random, 0, RYDE_PARAM_CHALLENGE_1_BYTES);
}


/**
void ryde_tcith_expand_challenge_2(ryde_tcith_challenge_t *challenge, uint8_t *v_grinding, const uint8_t *string_input)
* \brief This function generates the second challenge from an input seed
*
* \param[out] challenge ryde_tcith_challenge_t* Representation of challenge
* \param[out] v_grinding String containing w bits
* \param[in] string_input String containing (h2_partial || ctr)
*/
void ryde_tcith_expand_challenge_2(ryde_tcith_challenge_t *challenge, uint8_t *v_grinding, const uint8_t *string_input) {
    uint8_t random[RYDE_PARAM_CHALLENGE_2_BYTES + RYDE_PARAM_W_BYTES] = {0}, mask = 0x00;
    memset(*challenge, 0, sizeof(ryde_tcith_challenge_t));

    seedexpander_shake_t seedexpander_shake;
    seedexpander_shake_init(&seedexpander_shake, string_input, RYDE_HASH_BYTES + sizeof(uint64_t), NULL, 0);
    seedexpander_shake_get_bytes(&seedexpander_shake, random, RYDE_PARAM_CHALLENGE_2_BYTES + RYDE_PARAM_W_BYTES);

    memcpy(v_grinding, &random[RYDE_PARAM_CHALLENGE_2_BYTES], RYDE_PARAM_W_BYTES);  // Obtain v_grinding from random
    v_grinding[RYDE_PARAM_W_BYTES - 1] &= (uint8_t)RYDE_PARAM_W_MASK;
    memset(&random[RYDE_PARAM_CHALLENGE_2_BYTES], 0, RYDE_PARAM_W_BYTES);           // Remove v_grinding from random

    // Challenges concerning N_1
    mask = RYDE_PARAM_N_1_MASK;
    for(size_t i = 0; i < RYDE_PARAM_TAU_1; i++) {
        uint8_t block[RYDE_PARAM_N_1_BYTES] = {0};
        memcpy(block, random, RYDE_PARAM_N_1_BYTES);
        block[RYDE_PARAM_N_1_BYTES - 1] &= mask;
        memcpy((uint8_t *)&(*challenge)[i], block, RYDE_PARAM_N_1_BYTES);
        ryde_tcith_shift_to_right_array(random, RYDE_PARAM_CHALLENGE_2_BYTES, RYDE_PARAM_N_1_BITS);
    }

    #if RYDE_PARAM_TAU_2 > 0
    // Challenges concerning N_2
    mask = RYDE_PARAM_N_2_MASK;
    for(size_t i = 0; i < RYDE_PARAM_TAU_2; i++) {
        uint8_t block[RYDE_PARAM_N_2_BYTES] = {0};
        memcpy(block, random, RYDE_PARAM_N_2_BYTES);
        block[RYDE_PARAM_N_2_BYTES - 1] &= mask;
        memcpy((uint8_t *)&(*challenge)[i + RYDE_PARAM_TAU_1], block, RYDE_PARAM_N_2_BYTES);
        ryde_tcith_shift_to_right_array(random, RYDE_PARAM_CHALLENGE_2_BYTES, RYDE_PARAM_N_2_BITS);
    }
    #endif
}


#ifdef _SHAKE_TIMES4_
/**
void ryde_tcith_expand_challenge_2_x4(ryde_tcith_challenge_t *challenge, uint8_t *v_grinding, const uint8_t *string_input)
* \brief This function is a four times parallel implementation of ryde_tcith_expand_challenge_2.
*
* \param[out] challenge ryde_tcith_challenge_t* Representation of challenges determined by ctr, ctr + 1, ctr + 2 and ctr + 3
* \param[out] v_grinding Strings containing w bits determined by ctr, ctr + 1, ctr + 2 and ctr + 3
* \param[in] string_input String containing (h2_partial || ctr), (h2_partial || ctr + 1), (h2_partial || ctr + 2) and (h2_partial || ctr + 3)
*/
void ryde_tcith_expand_challenge_2_x4(ryde_tcith_challenge_t *challenge, uint8_t *v_grinding, const uint8_t *string_input) {
    uint8_t random0[RYDE_PARAM_CHALLENGE_2_BYTES + RYDE_PARAM_W_BYTES] = {0};
    uint8_t random1[RYDE_PARAM_CHALLENGE_2_BYTES + RYDE_PARAM_W_BYTES] = {0};
    uint8_t random2[RYDE_PARAM_CHALLENGE_2_BYTES + RYDE_PARAM_W_BYTES] = {0};
    uint8_t random3[RYDE_PARAM_CHALLENGE_2_BYTES + RYDE_PARAM_W_BYTES] = {0};
    uint8_t *random[] = {random0, random1, random2, random3};

    uint8_t mask = 0x00;

    const uint8_t *strings[] = {&string_input[0],
                                &string_input[RYDE_HASH_BYTES + sizeof(uint64_t)],
                                &string_input[(RYDE_HASH_BYTES + sizeof(uint64_t)) * 2],
                                &string_input[(RYDE_HASH_BYTES + sizeof(uint64_t)) * 3]};

    seedexpander_shake_x4_t seedexpander_shake;
    seedexpander_shake_x4_init(&seedexpander_shake, strings, RYDE_HASH_BYTES + sizeof(uint64_t), NULL, 0);
    seedexpander_shake_x4_get_bytes(&seedexpander_shake, random, RYDE_PARAM_CHALLENGE_2_BYTES + RYDE_PARAM_W_BYTES);

    for(size_t number = 0; number < SHAKE_STEP; number++) {
        memset(challenge[number], 0, sizeof(ryde_tcith_challenge_t));
        // Obtain v_grinding from random
        memcpy(&v_grinding[number * RYDE_PARAM_W_BYTES], &random[number][RYDE_PARAM_CHALLENGE_2_BYTES], RYDE_PARAM_W_BYTES);
        v_grinding[number * RYDE_PARAM_W_BYTES + RYDE_PARAM_W_BYTES - 1] &= (uint8_t)RYDE_PARAM_W_MASK;
        // Remove v_grinding from random
        memset(&random[number][RYDE_PARAM_CHALLENGE_2_BYTES], 0, RYDE_PARAM_W_BYTES);

        // Challenges concerning N_1
        mask = RYDE_PARAM_N_1_MASK;
        for(size_t i = 0; i < RYDE_PARAM_TAU_1; i++) {
            uint8_t block[RYDE_PARAM_N_1_BYTES] = {0};
            memcpy(block, random[number], RYDE_PARAM_N_1_BYTES);
            block[RYDE_PARAM_N_1_BYTES - 1] &= mask;
            memcpy((uint8_t *)&challenge[number][i], block, RYDE_PARAM_N_1_BYTES);
            ryde_tcith_shift_to_right_array(random[number], RYDE_PARAM_CHALLENGE_2_BYTES, RYDE_PARAM_N_1_BITS);
        }

        #if RYDE_PARAM_TAU_2 > 0
        // Challenges concerning N_2
        mask = RYDE_PARAM_N_2_MASK;
        for(size_t i = 0; i < RYDE_PARAM_TAU_2; i++) {
            uint8_t block[RYDE_PARAM_N_2_BYTES] = {0};
            memcpy(block, random[number], RYDE_PARAM_N_2_BYTES);
            block[RYDE_PARAM_N_2_BYTES - 1] &= mask;
            memcpy((uint8_t *)&challenge[number][i + RYDE_PARAM_TAU_1], block, RYDE_PARAM_N_2_BYTES);
            ryde_tcith_shift_to_right_array(random[number], RYDE_PARAM_CHALLENGE_2_BYTES, RYDE_PARAM_N_2_BITS);
        }
        #endif
    }
}
#endif


/**
* \fn uint8_t ryde_tcith_discard_input_challenge_2(const uint8_t *v_grinding)
* \brief This function determines if the w most significant bits of the input are zero.
*
* \param[in] v_grinding String containing the input seed
*/
uint8_t ryde_tcith_discard_input_challenge_2(const uint8_t *v_grinding) {
    uint8_t output = 0x00;
    uint8_t mask = RYDE_PARAM_W_MASK;
    for(size_t i = 0; i < RYDE_PARAM_W_BYTES; i++) {
        output |= (uint8_t)((v_grinding[i] & mask) != 0);
        mask = 0xFF;
    }

    return output;
}


/**
* \fn void ryde_tcith_compute_polynomial_proof(field_vector_t base_a[RYDE_PARAM_RHO],
*                                              field_vector_t mid_a[RYDE_PARAM_RHO],
*                                              ryde_tcith_shares_t base,
*                                              const field_vector_t s[RYDE_PARAM_R],
*                                              const binary_matrix_t C[RYDE_PARAM_R],
*                                              const field_vector_t acc_v[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
*                                              const field_matrix_t gamma[RYDE_PARAM_N - RYDE_PARAM_K],
*                                              const field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K],
*                                              size_t e)
* \brief This function computes the polynomial proof as required in the TCitH scheme.
*
* \param[out] base_a field_vector_t* Representation of vector base_a
* \param[out] mid_a field_vector_t* Representation of vector mid_a
* \param[in] base ryde_tcith_shares_t Representation of the share
* \param[in] s field_vector_t* Representation of vector s
* \param[in] C binary_matrix_t* Representation of matrix C
* \param[in] acc_v field_vector_t** Representation of the list of the vectors concerning acc_v
* \param[in] gamma field_matrix_t* Representation of matrix gamma
* \param[in] H field_matrix_t* Representation of matrix H
* \param[in] e Integer index
*/
void ryde_tcith_compute_polynomial_proof(field_vector_t base_a[RYDE_PARAM_RHO],
                                         field_vector_t mid_a[RYDE_PARAM_RHO],
                                         ryde_tcith_shares_t base,
                                         const field_vector_t s[RYDE_PARAM_R],
                                         const binary_matrix_t C[RYDE_PARAM_R],
                                         const field_vector_t acc_v[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
                                         const field_matrix_t gamma[RYDE_PARAM_N - RYDE_PARAM_K],
                                         const field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K],
                                         size_t e) {

    field_vector_t base_x[RYDE_PARAM_N - RYDE_PARAM_R] = {0},
                       base_xL[RYDE_PARAM_N - RYDE_PARAM_R - RYDE_PARAM_K] = {0},
                       base_xR[RYDE_PARAM_K] = {0},
                       mid_x[RYDE_PARAM_N - 1] = {0},
                       mid_xL[RYDE_PARAM_N - RYDE_PARAM_K - 1] = {0},
                       mid_xR[RYDE_PARAM_K] = {0},
                       tmp_nk[RYDE_PARAM_N - RYDE_PARAM_K] = {0},
                       tmp_nk1[RYDE_PARAM_N - RYDE_PARAM_K - 1] = {0},
                       tmp_nr[RYDE_PARAM_N - RYDE_PARAM_R] = {0},
                       aux_nr[RYDE_PARAM_N - RYDE_PARAM_R] = {0};
    
    field_vector_set_to_zero(base_a, RYDE_PARAM_RHO);
    field_vector_set_to_zero(mid_a, RYDE_PARAM_RHO);

    // Calculate base_x = (base_xL | base_xR)
    field_matrix_mul_by_vector_left(base_x,
                            (field_matrix_t *)&(base.C[e][1]),
                            base.s[e],
                            RYDE_PARAM_R - 1,
                            RYDE_PARAM_N - RYDE_PARAM_R);

    field_vector_copy(base_xL, base_x, RYDE_PARAM_N - RYDE_PARAM_R - RYDE_PARAM_K);
    field_vector_copy(base_xR,
                (field_vector_t *)&(base_x[RYDE_PARAM_N - RYDE_PARAM_R - RYDE_PARAM_K]),
                RYDE_PARAM_K);

    // Calculate base_a
    field_matrix_mul_by_vector_left_transpose(tmp_nk, H, base_xR, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);
    field_vector_add((field_vector_t *)&(tmp_nk[RYDE_PARAM_R]),
                (field_vector_t *)&(tmp_nk[RYDE_PARAM_R]),
                base_xL,
                RYDE_PARAM_N - RYDE_PARAM_R - RYDE_PARAM_K);
    field_matrix_mul_by_vector_left(base_a, gamma, tmp_nk, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_RHO);
    field_vector_add(base_a, base_a, base.v[e], RYDE_PARAM_RHO);

    // Calculate mid_x
    binary_matrix_mul_by_vector_left(tmp_nr,
                                        (binary_matrix_t *)&(C[1]),
                                        base.s[e],
                                        RYDE_PARAM_R - 1,
                                        RYDE_PARAM_N - RYDE_PARAM_R);
    field_matrix_mul_by_vector_left(aux_nr,
                                        (field_matrix_t *)&(base.C[e][1]),
                                        (field_vector_t *)&s[1],
                                        RYDE_PARAM_R - 1,
                                        RYDE_PARAM_N - RYDE_PARAM_R);
    field_vector_add(tmp_nr, aux_nr, tmp_nr, RYDE_PARAM_N - RYDE_PARAM_R);
    field_vector_add(tmp_nr, tmp_nr, (field_vector_t *)(base.C[e][0]), RYDE_PARAM_N - RYDE_PARAM_R);

    field_vector_copy(mid_x, base.s[e], RYDE_PARAM_R - 1);
    field_vector_copy((field_vector_t *)&mid_x[RYDE_PARAM_R - 1], tmp_nr, RYDE_PARAM_N - RYDE_PARAM_R);

    field_vector_copy(mid_xL, mid_x, RYDE_PARAM_N - RYDE_PARAM_K - 1);
    field_vector_copy(mid_xR, (field_vector_t *)&(mid_x[RYDE_PARAM_N - RYDE_PARAM_K - 1]), RYDE_PARAM_K);

    // Calculate mid_a
    field_matrix_mul_by_vector_left_transpose(tmp_nk, H, mid_xR, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);
    field_vector_add(tmp_nk1, (field_vector_t *)&(tmp_nk[1]), mid_xL, RYDE_PARAM_N - RYDE_PARAM_K - 1);
    field_vector_copy((field_vector_t *)&(tmp_nk[1]), tmp_nk1, RYDE_PARAM_N - RYDE_PARAM_K - 1);
    field_matrix_mul_by_vector_left(mid_a, gamma, tmp_nk, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_RHO);
    field_vector_add(mid_a, mid_a, acc_v[e], RYDE_PARAM_RHO);
}


/**
* \fn void ryde_tcith_recompute_polynomial_proof(field_vector_t base_a[RYDE_PARAM_RHO],
*                                                const ryde_tcith_challenge_t i_star,
*                                                ryde_tcith_shares_t shares,
*                                                const field_matrix_t gamma[RYDE_PARAM_N - RYDE_PARAM_K],
*                                                const field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K],
*                                                const field_vector_t y[RYDE_PARAM_N - RYDE_PARAM_K],
*                                                const field_vector_t mid_a[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
*                                                size_t e)
* \brief This function recomputes the polynomial proof as required in the TCitH scheme.
*
* \param[out] base_a field_vector_t* Representation of vector base_a
* \param[in] i_star ryde_tcith_challenge_t Representation of the challenge
* \param[in] shares ryde_tcith_shares_t Representation of the share
* \param[in] gamma field_matrix_t* Representation of matrix gamma
* \param[in] H field_matrix_t* Representation of matrix H
* \param[in] y field_vector_t* Representation of vector y
* \param[in] mid_a field_vector_t** Representation of the list of vectors concerning mid_a
* \param[in] e Integer index
*/
void ryde_tcith_recompute_polynomial_proof(field_vector_t base_a[RYDE_PARAM_RHO],
                                           const ryde_tcith_challenge_t i_star,
                                           ryde_tcith_shares_t shares,
                                           const field_matrix_t gamma[RYDE_PARAM_N - RYDE_PARAM_K],
                                           const field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K],
                                           const field_vector_t y[RYDE_PARAM_N - RYDE_PARAM_K],
                                           const field_vector_t mid_a[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
                                           size_t e) {
    field_vector_t share_x[RYDE_PARAM_N - 1] = {0},
                       share_xL[RYDE_PARAM_N - RYDE_PARAM_K - 1] = {0},
                       share_xR[RYDE_PARAM_K] = {0},
                       share_a[RYDE_PARAM_RHO] = {0},
                       tmp_nk[RYDE_PARAM_N - RYDE_PARAM_K] = {0},
                       tmp_nk1[RYDE_PARAM_N - RYDE_PARAM_K - 1] = {0},
                       aux_nk[RYDE_PARAM_N - RYDE_PARAM_K] = {0},
                       aux_nr[RYDE_PARAM_N - RYDE_PARAM_R] = {0},
                       tmp_nr[RYDE_PARAM_N - RYDE_PARAM_R] = {0},
                       tmp_r1[RYDE_PARAM_R - 1] = {0},
                       tmp_a[RYDE_PARAM_RHO] = {0};

    field_t phi_i_star, phi_i_star_squared;
    ryde_tcith_phi(phi_i_star, i_star[e]);
    field_sqr(phi_i_star_squared, phi_i_star);

    // Calculate share_x = (share_xL | share_xR)
    field_vector_mul_by_constant(tmp_r1, shares.s[e], phi_i_star, RYDE_PARAM_R - 1);
    field_matrix_mul_by_vector_left(tmp_nr,
                            (field_matrix_t *)&(shares.C[e][1]),
                            shares.s[e],
                            RYDE_PARAM_R - 1,
                            RYDE_PARAM_N - RYDE_PARAM_R);
    field_vector_mul_by_constant(aux_nr, (field_vector_t *)shares.C[e][0], phi_i_star, RYDE_PARAM_N - RYDE_PARAM_R);
    field_vector_add(tmp_nr, aux_nr, tmp_nr, RYDE_PARAM_N - RYDE_PARAM_R);

    field_vector_copy(share_x, tmp_r1, RYDE_PARAM_R - 1);
    field_vector_copy((field_vector_t *)&share_x[RYDE_PARAM_R - 1], tmp_nr, RYDE_PARAM_N - RYDE_PARAM_R);

    field_vector_copy(share_xL, share_x, RYDE_PARAM_N - RYDE_PARAM_K - 1);
    field_vector_copy(share_xR, (field_vector_t *)&(share_x[RYDE_PARAM_N - RYDE_PARAM_K - 1]), RYDE_PARAM_K);

    // Calculate share_a
    field_matrix_mul_by_vector_left_transpose(tmp_nk, H, share_xR, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);
    field_add(tmp_nk[0], tmp_nk[0], phi_i_star_squared);
    field_vector_add(tmp_nk1, (field_vector_t *)&(tmp_nk[1]), share_xL, RYDE_PARAM_N - RYDE_PARAM_K - 1);
    field_vector_copy((field_vector_t *)&(tmp_nk[1]), tmp_nk1, RYDE_PARAM_N - RYDE_PARAM_K - 1);
    field_vector_mul_by_constant(aux_nk, y, phi_i_star_squared, RYDE_PARAM_N - RYDE_PARAM_K);
    field_vector_add(tmp_nk, tmp_nk, aux_nk, RYDE_PARAM_N - RYDE_PARAM_K);
    field_matrix_mul_by_vector_left(share_a, gamma, tmp_nk, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_RHO);
    field_vector_add(share_a, share_a, shares.v[e], RYDE_PARAM_RHO);

    // Calculate base_a
    field_vector_mul_by_constant(tmp_a, mid_a[e], phi_i_star, RYDE_PARAM_RHO);
    field_vector_add(base_a, share_a, tmp_a, RYDE_PARAM_RHO);
}


/**
* \fn void ryde_pack_matrices_and_vectors(uint8_t *output,
*                                         const field_vector_t aux_s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1],
*                                         const binary_matrix_t aux_C[RYDE_PARAM_TAU][RYDE_PARAM_R],
*                                         const field_vector_t mid_alpha[RYDE_PARAM_TAU][RYDE_PARAM_RHO])
* \brief This function parse/pack the list of vectors aux_s, aux_C, and mid_alpha into a byte string.
*
* \param[out] output uint8_t Byte string containing the packed list of elements aux_s (vector), aux_C (matrix), aux_v (vector)
* \param[in] aux_s field_vector_t* Representation of the list of the vectors concerning aux_s
* \param[in] aux_C binary_matrix_t* Representation of the list of the vectors concerning aux_C
* \param[in] mid_alpha field_vector_t* Representation of the list of the vectors concerning aux_v
*/
void ryde_pack_matrices_and_vectors(uint8_t *output,
                                    const field_vector_t aux_s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1],
                                    const binary_matrix_t aux_C[RYDE_PARAM_TAU][RYDE_PARAM_R],
                                    const field_vector_t mid_alpha[RYDE_PARAM_TAU][RYDE_PARAM_RHO]) {
    const size_t BLOCK_VEC = ((RYDE_PARAM_R - 1 + RYDE_PARAM_RHO) * RYDE_PARAM_TAU * RYDE_PARAM_M + 7) / 8;
    const size_t BLOCK_MAT = (RYDE_PARAM_R * RYDE_PARAM_TAU * (RYDE_PARAM_N - RYDE_PARAM_R) + 7) / 8;

    memset(output, 0, BLOCK_VEC + BLOCK_MAT);

    size_t element = 0;
    field_vector_t vectors[(RYDE_PARAM_R - 1 + RYDE_PARAM_RHO) * RYDE_PARAM_TAU] = {0};

    for(size_t i = 0; i < RYDE_PARAM_TAU; i++) {
        for (size_t j = 0; j < (RYDE_PARAM_R - 1); j++) {
            field_copy(vectors[element], aux_s[i][j]);
            element += 1;
        }
    }

    for(size_t i = 0; i < RYDE_PARAM_TAU; i++){
        for(size_t j = 0; j < RYDE_PARAM_RHO; j++){
            field_copy(vectors[element], mid_alpha[i][j]);
            element += 1;
        }
    }
    field_vector_to_string(output, vectors, (RYDE_PARAM_R - 1 + RYDE_PARAM_RHO) * RYDE_PARAM_TAU);

    uint32_t WORDS = (RYDE_PARAM_N - RYDE_PARAM_R + 63) / 64;
    element = 0;
    binary_matrix_t matrices[RYDE_PARAM_R * RYDE_PARAM_TAU] = {0};
    for(size_t i = 0; i < RYDE_PARAM_TAU; i++){
        for(size_t j = 0; j < RYDE_PARAM_R; j++) {
            memcpy(matrices[element], aux_C[i][j], WORDS * sizeof(uint64_t));
            element += 1;
        }
    }
    binary_matrix_to_string(&output[BLOCK_VEC], matrices, RYDE_PARAM_R * RYDE_PARAM_TAU, RYDE_PARAM_N - RYDE_PARAM_R);
}

/**
* \fn void ryde_unpack_matrices_and_vectors(field_vector_t aux_s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1],
*                                           binary_matrix_t aux_C[RYDE_PARAM_TAU][RYDE_PARAM_R],
*                                           field_vector_t mid_alpha[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
*                                           const uint8_t *output)
* \brief This function unparse/unpack the list of vectors aux_s, aux_C, and mid_alpha from a byte string.
*
* \param[out] aux_s field_vector_t* Representation of the list of the vectors concerning aux_s
* \param[out] aux_C binary_matrix_t* Representation of the list of the vectors concerning aux_C
* \param[out] mid_alpha field_vector_t* Representation of the list of the vectors concerning aux_v
* \param[in] input Byte string containing the packed list of elements aux_s (vector), aux_C (matrix), aux_v (vector)
*/
void ryde_unpack_matrices_and_vectors(field_vector_t aux_s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1],
                                      binary_matrix_t aux_C[RYDE_PARAM_TAU][RYDE_PARAM_R],
                                      field_vector_t mid_alpha[RYDE_PARAM_TAU][RYDE_PARAM_RHO],
                                      const uint8_t *input) {

    // We first determine if the input is valid (i.e., it has the right amount of bits)
    size_t TMP_VEC = ((RYDE_PARAM_R - 1 + RYDE_PARAM_RHO) * RYDE_PARAM_TAU * RYDE_PARAM_M) % 8;
    uint8_t MASK_VEC = 0xFF;
    if (TMP_VEC) { MASK_VEC = (1 << TMP_VEC) - 1; }

    size_t TMP_MAT = (RYDE_PARAM_R * RYDE_PARAM_TAU * (RYDE_PARAM_N - RYDE_PARAM_R)) % 8;
    uint8_t MASK_MAT = 0xFF;
    if (TMP_MAT) { MASK_MAT = (1 << TMP_MAT) - 1; }

    const size_t BLOCK_VEC = ((RYDE_PARAM_R - 1 + RYDE_PARAM_RHO) * RYDE_PARAM_TAU * RYDE_PARAM_M + 7) / 8;
    const size_t BLOCK_MAT = (RYDE_PARAM_R * RYDE_PARAM_TAU * (RYDE_PARAM_N - RYDE_PARAM_R) + 7) / 8;

    uint8_t invalid_input_vec = (input[BLOCK_VEC - 1] & MASK_VEC) ^ input[BLOCK_VEC - 1];
    uint8_t invalid_input_mat = (input[BLOCK_VEC + BLOCK_MAT- 1] & MASK_MAT) ^ input[BLOCK_VEC + BLOCK_MAT - 1];
    if (invalid_input_vec | invalid_input_mat) {
        // This branch is for avoiding trivial forgery on the bytes determining aux_s, aux_C, and mid_alpha
        // If the input byt string has more bits than expected, then return zero by default
        for(size_t i = 0; i < RYDE_PARAM_TAU; i++) {
            field_vector_set_to_zero(aux_s[i], RYDE_PARAM_R - 1);
            binary_matrix_set_to_zero(aux_C[i], RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
            field_vector_set_to_zero(mid_alpha[i], RYDE_PARAM_RHO);
        }
    }
    else {
        size_t element = 0;
        field_vector_t vectors[(RYDE_PARAM_R - 1 + RYDE_PARAM_RHO) * RYDE_PARAM_TAU] = {0};
        field_vector_from_string(vectors, (RYDE_PARAM_R - 1 + RYDE_PARAM_RHO) * RYDE_PARAM_TAU, input);

        for (size_t i = 0; i < RYDE_PARAM_TAU; i++) {
            for (size_t j = 0; j < (RYDE_PARAM_R - 1); j++) {
                field_copy(aux_s[i][j], vectors[element]);
                element += 1;
            }
        }

        for (size_t i = 0; i < RYDE_PARAM_TAU; i++) {
            for (size_t j = 0; j < RYDE_PARAM_RHO; j++) {
                field_copy(mid_alpha[i][j], vectors[element]);
                element += 1;
            }
        }

        uint32_t WORDS = (RYDE_PARAM_N - RYDE_PARAM_R + 63) / 64;
        element = 0;
        binary_matrix_t matrices[RYDE_PARAM_R * RYDE_PARAM_TAU] = {0};
        binary_matrix_from_string(matrices, RYDE_PARAM_R * RYDE_PARAM_TAU, RYDE_PARAM_N - RYDE_PARAM_R, &input[BLOCK_VEC]);

        for (size_t i = 0; i < RYDE_PARAM_TAU; i++) {
            for (size_t j = 0; j < RYDE_PARAM_R; j++) {
                memcpy(aux_C[i][j], matrices[element], WORDS * sizeof(uint64_t));
                element += 1;
            }
        }
    }
}
