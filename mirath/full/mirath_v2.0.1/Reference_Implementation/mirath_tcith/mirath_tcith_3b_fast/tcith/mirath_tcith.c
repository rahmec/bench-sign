/**
 * @file tcith.c
 * @brief Implementation of TCitH related functions
 */

#include "mirath_tcith.h"

void mirath_commit(uint8_t pair_node[2][MIRATH_SECURITY_BYTES],
                              const uint8_t salt[MIRATH_PARAM_SALT_BYTES],
                              const uint32_t i,
                              const uint8_t seed[MIRATH_SECURITY_BYTES]) {
    uint8_t domain_separator = DOMAIN_SEPARATOR_CMT;
    hash_sha3_ctx ctx;
    hash_init(&ctx);
    hash_update(&ctx, &domain_separator, sizeof(uint8_t));
    hash_update(&ctx, salt, MIRATH_PARAM_SALT_BYTES);
    hash_update(&ctx, (uint8_t * ) & i, sizeof(uint32_t));
    hash_update(&ctx, seed, MIRATH_SECURITY_BYTES);
    hash_finalize((uint8_t *)pair_node, &ctx);
}

/**
* \fn void mirath_tcith_shift_to_left_array(uint8_t *inout_a, size_t length)
* \brief This function performs a shift to right of the input.
*
* \param[in/out] inout_a uint8_t* Representation of a byte string
* \param[in] length size_t Representation of the byte string length
* \param[in] nbits int Representation of the number of bits to be shifted
*/
static inline void mirath_tcith_shift_to_right_array(uint8_t *string, const size_t length, const int nbits) {
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

void mirath_tcith_internal_steps_pk(ff_t y[MIRATH_VAR_FF_Y_BYTES],
                                    const ff_t S[MIRATH_VAR_FF_S_BYTES],
                                    const ff_t C[MIRATH_VAR_FF_C_BYTES],
                                    const ff_t H[MIRATH_VAR_FF_H_BYTES]) {
    ff_t e_A[MIRATH_VAR_FF_Y_BYTES] = {0};
    ff_t e_B[mirath_matrix_ff_bytes_size(MIRATH_PARAM_K, 1)] = {0};

    ff_t T[MIRATH_VAR_FF_T_BYTES] = {0};
    ff_t E[MIRATH_VAR_FF_E_BYTES] = {0};

    mirath_matrix_ff_product(T, S, C, MIRATH_PARAM_M, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
    mirath_matrix_ff_horizontal_concat(E, S, T, MIRATH_PARAM_M, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);

    const uint32_t bytes_e_B = mirath_matrix_ff_bytes_size(MIRATH_PARAM_K, 1);

    memcpy(e_A, E, MIRATH_VAR_FF_Y_BYTES);
#if (OFF_E_A > 0)
    const uint8_t mask = (1 << (8 - OFF_E_A)) - 1;
    e_A[MIRATH_VAR_FF_Y_BYTES - 1] &= mask;

    for (uint32_t i = 0; i < bytes_e_B - 1 ; i++) {
        e_B[i] = ((E[MIRATH_VAR_FF_Y_BYTES - 1 + i]) >> (8 - OFF_E_A));
        e_B[i] ^= ((E[MIRATH_VAR_FF_Y_BYTES + i]) << (OFF_E_A));
    }
#if ((OFF_E_A + OFF_E_B) >= 8)
    e_B[bytes_e_B - 1] = ((E[MIRATH_VAR_FF_E_BYTES - 1]) >> (8 - OFF_E_A));
#else
    e_B[bytes_e_B - 1] = (E[MIRATH_VAR_FF_E_BYTES - 2] >> (8 - OFF_E_A));
    e_B[bytes_e_B - 1] ^= (E[MIRATH_VAR_FF_E_BYTES - 1] << OFF_E_A);
#endif
#else
    memcpy(e_B, E + MIRATH_VAR_FF_Y_BYTES, bytes_e_B);
#endif

    memset(y, 0, MIRATH_VAR_FF_Y_BYTES);
    mirath_matrix_ff_product(y, H, e_B, MIRATH_PARAM_M * MIRATH_PARAM_N - MIRATH_PARAM_K, MIRATH_PARAM_K, 1);

    mirath_vec_ff_add_arith(y, y, e_A, MIRATH_VAR_FF_Y_BYTES);
}

/**
 * \brief This function does the commit the polynomials P_s, P_c' and P_v. Furthermore, does all-but-one vector
 * commitments (Algorithm 10 Routine BAVC.Commit in specs)
 */
void commit_witness_polynomials(ff_mu_t S_base[MIRATH_PARAM_TAU][MIRATH_VAR_S],
                              ff_mu_t C_base[MIRATH_PARAM_TAU][MIRATH_VAR_C],
                              ff_mu_t v_base[MIRATH_PARAM_TAU][MIRATH_PARAM_RHO],
                              ff_mu_t v[MIRATH_PARAM_TAU][MIRATH_PARAM_RHO],
                              hash_t hash_sh,
                              mirath_ggm_tree_leaves_t seeds,
                              ff_t aux[MIRATH_PARAM_TAU][MIRATH_VAR_FF_AUX_BYTES],
                              const uint8_t salt[MIRATH_PARAM_SALT_BYTES],
                              const ff_t S[MIRATH_VAR_FF_S_BYTES],
                              const ff_t C[MIRATH_VAR_FF_C_BYTES]) {

    memset(aux, 0, MIRATH_PARAM_TAU * MIRATH_VAR_FF_AUX_BYTES);
    memset(S_base, 0, sizeof(ff_mu_t) * MIRATH_PARAM_TAU * MIRATH_VAR_S);
    memset(C_base, 0, sizeof(ff_mu_t) * MIRATH_PARAM_TAU * MIRATH_VAR_C);
    memset(v_base, 0, sizeof(ff_mu_t) *  MIRATH_PARAM_TAU * MIRATH_PARAM_RHO);
    memset(v, 0, sizeof(ff_mu_t) * MIRATH_PARAM_TAU * MIRATH_PARAM_RHO);

    hash_t h_com;

    uint8_t domain_separator_commits;
    hash_sha3_ctx hash_commits;

    domain_separator_commits = DOMAIN_SEPARATOR_COMMITMENT;
    hash_init(&hash_commits);
    hash_update(&hash_commits, &domain_separator_commits, sizeof(uint8_t));

    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        const uint16_t N = e < MIRATH_PARAM_TAU_1 ? MIRATH_PARAM_N_1 : MIRATH_PARAM_N_2;
        ff_t S_acc[MIRATH_VAR_FF_S_BYTES] = {0};
        ff_t C_acc[MIRATH_VAR_FF_C_BYTES] = {0};

        for (uint16_t i = 0; i < N; i++) {
            ff_t S_rnd[MIRATH_VAR_FF_S_BYTES];
            ff_t C_rnd[MIRATH_VAR_FF_C_BYTES];
            ff_mu_t v_rnd[MIRATH_PARAM_RHO] = {0};

            const uint32_t idx = mirath_tcith_psi((size_t)i, (size_t)e);

            mirath_tcith_commit_t commit;
            mirath_commit((uint8_t (*)[MIRATH_SECURITY_BYTES])commit, salt, (uint32_t)idx, seeds[idx]);
            hash_update(&hash_commits, (uint8_t *)commit, sizeof(mirath_tcith_commit_t));

            uint8_t sample[MIRATH_BLOCK_LENGTH * MIRATH_SECURITY_BYTES] = {0};
            rijndael_expand_share((uint8_t (*)[MIRATH_SECURITY_BYTES])sample, salt, seeds[idx], MIRATH_BLOCK_LENGTH);

            memcpy(S_rnd, sample, MIRATH_VAR_FF_S_BYTES);
            mirath_matrix_set_to_ff(S_rnd, MIRATH_PARAM_M, MIRATH_PARAM_R);
            memcpy(C_rnd, sample + MIRATH_VAR_FF_S_BYTES, MIRATH_VAR_FF_C_BYTES);
            mirath_matrix_set_to_ff(C_rnd, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
            mirath_vector_set_to_ff_mu(v_rnd, sample + MIRATH_VAR_FF_S_BYTES + MIRATH_VAR_FF_C_BYTES, MIRATH_PARAM_RHO);

            // Performs S_acc = S_acc + S_rnd, C_acc = C_acc + C_rnd and v[e] = v[e] + v_rnd
            mirath_matrix_ff_add(S_acc, S_acc, S_rnd, MIRATH_PARAM_M, MIRATH_PARAM_R);
            mirath_matrix_ff_add(C_acc, C_acc, C_rnd, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
            mirath_vector_ff_mu_add(v[e], v[e], v_rnd, MIRATH_PARAM_RHO);

            const ff_mu_t phi_i = (ff_mu_t)i;
            mirath_matrix_ff_mu_add_multiple_ff(S_base[e], phi_i, S_rnd, MIRATH_PARAM_M, MIRATH_PARAM_R);
            mirath_matrix_ff_mu_add_multiple_ff(C_base[e], phi_i, C_rnd, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
            mirath_vector_ff_mu_add_multiple(v_base[e], v_base[e], phi_i, v_rnd, MIRATH_PARAM_RHO);
        }

        // S - acc_S
        mirath_matrix_ff_add(aux[e], S, S_acc, MIRATH_PARAM_M, MIRATH_PARAM_R);
        const uint32_t n_bytes = mirath_matrix_ff_bytes_size(MIRATH_PARAM_M, MIRATH_PARAM_R);
        // C - acc_C
        mirath_matrix_ff_add(aux[e] + n_bytes, C, C_acc, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
    }

    // This line refers to Step 8 in Algorithm 10.
    hash_finalize(h_com, &hash_commits); // Second output of MultiVC.Commit

    hash_sha3_ctx hash_sh_ctx;
    uint8_t domain_separator;

    domain_separator = DOMAIN_SEPARATOR_HASH1;
    hash_init(&hash_sh_ctx);
    hash_update(&hash_sh_ctx, &domain_separator, sizeof(uint8_t));
    hash_update(&hash_sh_ctx, salt, MIRATH_PARAM_SALT_BYTES);
    hash_update(&hash_sh_ctx, h_com, 2 * MIRATH_SECURITY_BYTES);

    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        hash_update(&hash_sh_ctx, aux[e], MIRATH_VAR_FF_AUX_BYTES);
    }

    hash_finalize(hash_sh, &hash_sh_ctx);
}

void compute_share(ff_mu_t S_share[MIRATH_VAR_S], ff_mu_t C_share[MIRATH_VAR_C],
                   ff_mu_t v_share[MIRATH_PARAM_RHO], hash_sha3_ctx *hash_commits,
                   const mirath_tcith_commit_t commits_i_star[MIRATH_PARAM_TAU],
                   const uint32_t i_star, const mirath_ggm_tree_leaves_t seeds, const uint32_t e,
                   const ff_t aux[MIRATH_VAR_FF_AUX_BYTES],
                   const uint8_t salt[MIRATH_PARAM_SALT_BYTES]) {

    const ff_t *aux_S = aux;
    const ff_t *aux_C = aux + MIRATH_VAR_FF_S_BYTES;

    const uint16_t N = e < MIRATH_PARAM_TAU_1 ? MIRATH_PARAM_N_1 : MIRATH_PARAM_N_2;
    for (uint16_t i = 0; i < N; i++) {
        if ((uint32_t)i != i_star) {
            ff_t Si[MIRATH_VAR_FF_S_BYTES];
            ff_t Ci[MIRATH_VAR_FF_C_BYTES];
            ff_mu_t vi[MIRATH_PARAM_RHO] = {0};

            const uint32_t idx = mirath_tcith_psi((size_t) i, (size_t) e);
            mirath_tcith_commit_t commit;
            mirath_commit((uint8_t (*)[MIRATH_SECURITY_BYTES])commit, salt, (uint32_t)idx, seeds[idx]);
            hash_update(hash_commits, (uint8_t *)commit, sizeof(mirath_tcith_commit_t));

            uint8_t sample[2 * MIRATH_BLOCK_LENGTH * MIRATH_SECURITY_BYTES] = {0};
            rijndael_expand_share((uint8_t (*)[MIRATH_SECURITY_BYTES])sample, salt, seeds[idx], 2 * MIRATH_BLOCK_LENGTH);

            memcpy(Si, sample, MIRATH_VAR_FF_S_BYTES);
            mirath_matrix_set_to_ff(Si, MIRATH_PARAM_M, MIRATH_PARAM_R);
            memcpy(Ci, sample + MIRATH_VAR_FF_S_BYTES, MIRATH_VAR_FF_C_BYTES);
            mirath_matrix_set_to_ff(Ci, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
            mirath_vector_set_to_ff_mu(vi, sample + MIRATH_VAR_FF_S_BYTES + MIRATH_VAR_FF_C_BYTES, MIRATH_PARAM_RHO);

            const ff_mu_t sc = (ff_mu_t) ((uint16_t) i_star ^ i);

            mirath_matrix_ff_mu_add_multiple_ff(S_share, sc, Si, MIRATH_PARAM_M, MIRATH_PARAM_R);
            mirath_matrix_ff_mu_add_multiple_ff(C_share, sc, Ci, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
            mirath_vector_ff_mu_add_multiple(v_share, v_share, sc, vi, MIRATH_PARAM_RHO);
        } else {
            hash_update(hash_commits, (uint8_t *)commits_i_star[e], sizeof(mirath_tcith_commit_t));
        }
    }

    const ff_mu_t phi_i = i_star;
    mirath_matrix_ff_mu_add_multiple_ff(S_share, phi_i, aux_S, MIRATH_PARAM_M, MIRATH_PARAM_R);
    mirath_matrix_ff_mu_add_multiple_ff(C_share, phi_i, aux_C, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
}

static inline void split_codeword_ff_mu(ff_mu_t e_A[MIRATH_VAR_E_A], ff_mu_t e_B[MIRATH_PARAM_K],
                          const ff_mu_t in_X[MIRATH_VAR_S], const ff_mu_t in_Y[MIRATH_VAR_BASE_MID]) {
    ff_mu_t tmp[MIRATH_VAR_T];

    uint32_t n_bytes1 = MIRATH_VAR_FF_MU_S_BYTES;
    uint32_t n_bytes2 = MIRATH_VAR_FF_MU_T_BYTES;

    memcpy((uint8_t *)tmp, (uint8_t *)in_X, n_bytes1);
    memcpy((uint8_t *)tmp + n_bytes1, (uint8_t *)in_Y, n_bytes2);

    n_bytes1 = MIRATH_VAR_FF_MU_E_A_BYTES;
    n_bytes2 = MIRATH_VAR_FF_MU_K_BYTES;

    memcpy((uint8_t *)e_A, (uint8_t *)tmp, n_bytes1);
    memcpy((uint8_t *)e_B, (uint8_t *)tmp + n_bytes1, n_bytes2);
}

void compute_polynomial_proof(ff_mu_t base_alpha[MIRATH_PARAM_RHO], ff_mu_t mid_alpha[MIRATH_PARAM_RHO],
                              const ff_t S[MIRATH_VAR_FF_S_BYTES], const ff_mu_t S_rnd[MIRATH_VAR_S],
                              const ff_t C[MIRATH_VAR_FF_C_BYTES], const ff_mu_t C_rnd[MIRATH_VAR_C],
                              const ff_mu_t v[MIRATH_PARAM_RHO], ff_mu_t rnd_v[MIRATH_PARAM_RHO],
                              const ff_mu_t gamma[MIRATH_VAR_GAMMA], const ff_t H[MIRATH_VAR_FF_H_BYTES]) {

    ff_mu_t aux_E[MIRATH_VAR_BASE_MID];
    ff_mu_t e_A[MIRATH_VAR_E_A];
    ff_mu_t e_B[MIRATH_PARAM_K];

    // rnd_S * rnd_C
    mirath_matrix_ff_mu_product(aux_E, S_rnd, C_rnd, MIRATH_PARAM_M, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);

    ff_mu_t zero[MIRATH_VAR_S] = {0};

    split_codeword_ff_mu(e_A, e_B, zero, aux_E);

    ff_mu_t tmp[MIRATH_VAR_E_A];

    // H * e_B
    mirath_matrix_ff_mu_product_ff1mu(tmp, H, e_B, MIRATH_PARAM_M * MIRATH_PARAM_N - MIRATH_PARAM_K, MIRATH_PARAM_K, 1);
    // e_A + (H * e_B)
    mirath_vector_ff_mu_add(tmp, tmp, e_A, MIRATH_VAR_E_A);
    // gamma * [e_A + (H * e_B)]
    mirath_matrix_ff_mu_product(base_alpha, gamma, tmp, MIRATH_PARAM_RHO, MIRATH_PARAM_M * MIRATH_PARAM_N - MIRATH_PARAM_K, 1);
    // gamma * [e_A + (H * e_B)] + rnd_V
    mirath_vector_ff_mu_add(base_alpha, base_alpha, rnd_v, MIRATH_PARAM_RHO);

    ff_mu_t aux_s[MIRATH_VAR_S];
    ff_mu_t aux_c[MIRATH_VAR_C];
    ff_mu_t aux_sc[MIRATH_VAR_BASE_MID];

    ff_t sc[MIRATH_VAR_FF_T_BYTES];

    // S + rnd_S
    mirath_matrix_ff_mu_add_mu1ff(aux_s, S_rnd, S, MIRATH_PARAM_M, MIRATH_PARAM_R);
    // C + rnd_C
    mirath_matrix_ff_mu_add_mu1ff(aux_c, C_rnd, C, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
    // (S + rnd_S)(C + rnd_C)
    mirath_matrix_ff_mu_product(aux_sc, aux_s, aux_c, MIRATH_PARAM_M, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
    // (S + rnd_S)(C + rnd_C) - base_E
    mirath_matrix_ff_mu_add(aux_E, aux_E, aux_sc, MIRATH_PARAM_M, MIRATH_PARAM_N - MIRATH_PARAM_R);
    // S * C
    mirath_matrix_ff_product(sc, S, C, MIRATH_PARAM_M, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);
    // (S + rnd_S)(C + rnd_C) - base_E - (S * C)
    mirath_matrix_ff_mu_add_mu1ff(aux_E, aux_E, sc, MIRATH_PARAM_M, MIRATH_PARAM_N - MIRATH_PARAM_R);

    split_codeword_ff_mu(e_A, e_B, S_rnd, aux_E);

    // H * e'_B
    mirath_matrix_ff_mu_product_ff1mu(tmp, H, e_B, MIRATH_PARAM_M * MIRATH_PARAM_N - MIRATH_PARAM_K, MIRATH_PARAM_K, 1);
    // e'_A + (H * e'_B)
    mirath_vector_ff_mu_add(tmp, tmp, e_A, MIRATH_VAR_E_A);
    // gamma * [e'_A + (H * e'_B)]
    mirath_matrix_ff_mu_product(mid_alpha, gamma, tmp, MIRATH_PARAM_RHO, MIRATH_PARAM_M * MIRATH_PARAM_N - MIRATH_PARAM_K, 1);
    // gamma * [e_A + (H * e_B)] + v
    mirath_vector_ff_mu_add(mid_alpha, mid_alpha, v, MIRATH_PARAM_RHO);
}

void recompute_polynomial_proof(ff_mu_t base_alpha[MIRATH_PARAM_RHO], const ff_mu_t p,
                     const ff_mu_t S_share[MIRATH_VAR_S], const ff_mu_t C_share[MIRATH_VAR_C],
                     const ff_mu_t v_share[MIRATH_PARAM_RHO], const ff_mu_t gamma[MIRATH_VAR_GAMMA],
                     const ff_t H[MIRATH_VAR_FF_H_BYTES], const ff_t y[MIRATH_VAR_FF_Y_BYTES],
                     const ff_mu_t mid_alpha[MIRATH_PARAM_RHO]) {

    ff_mu_t e_A[MIRATH_VAR_E_A];
    ff_mu_t e_B[MIRATH_PARAM_K];

    ff_mu_t aux[MIRATH_VAR_BASE_MID];
    ff_mu_t Ts[MIRATH_VAR_S] = {0};

    // p * S_share
    mirath_matrix_ff_mu_add_multiple_2(Ts, p, S_share, MIRATH_PARAM_M, MIRATH_PARAM_R);
    // S_share * C_share
    mirath_matrix_ff_mu_product(aux, S_share, C_share, MIRATH_PARAM_M, MIRATH_PARAM_R, MIRATH_PARAM_N - MIRATH_PARAM_R);

    split_codeword_ff_mu(e_A, e_B, Ts, aux);

    ff_mu_t tmp[MIRATH_VAR_E_A];

    // H * e_B
    mirath_matrix_ff_mu_product_ff1mu(tmp, H, e_B, MIRATH_PARAM_M * MIRATH_PARAM_N - MIRATH_PARAM_K, MIRATH_PARAM_K, 1);
    // e_A + (H * e_B)
    mirath_vector_ff_mu_add(tmp, tmp, e_A, MIRATH_VAR_E_A);
    // (e_A + (H * e_B)) - y * p^2
    mirath_vector_ff_mu_add_multiple_ff(tmp, tmp, mirath_ff_mu_mult(p, p), y, MIRATH_PARAM_M * MIRATH_PARAM_N - MIRATH_PARAM_K);
    // gamma * [(e_A + (H * e_B)) - y * p^2]
    mirath_matrix_ff_mu_product(base_alpha, gamma, tmp, MIRATH_PARAM_RHO, MIRATH_PARAM_M * MIRATH_PARAM_N - MIRATH_PARAM_K, 1);
    // gamma * [(e_A + (H * e_B)) - y * p^2] + v_share
    mirath_vector_ff_mu_add(base_alpha, base_alpha, v_share, MIRATH_PARAM_RHO);

    // share_alpha - mid_alpha * p
    mirath_vector_ff_mu_add_multiple(base_alpha, base_alpha, p, mid_alpha, MIRATH_PARAM_RHO);
}

/**
void mirath_tcith_expand_view_challenge(mirath_tcith_view_challenge_t challenge, uint8_t *v_grinding, const uint8_t *string_input)
* \brief This function generates the second challenges from an input seed
*
* \param[out] challenge mirath_tcith_view_challenge_t Representation of challenge
* \param[out] v_grinding String containing w bits
* \param[in] string_input String containing (h2_partial || ctr)
*/
void mirath_tcith_expand_view_challenge(mirath_tcith_view_challenge_t challenge, uint8_t *v_grinding, const uint8_t *string_input) {
    uint8_t random[MIRATH_PARAM_CHALLENGE_2_BYTES + MIRATH_PARAM_HASH_2_MASK_BYTES] = {0};

    memset(challenge, 0, sizeof(mirath_tcith_view_challenge_t));

    shake_prng_t seedexpander_shake;
    seedexpander_shake_init(&seedexpander_shake, string_input, 2 * MIRATH_SECURITY_BYTES + sizeof(uint64_t), NULL, 0);
    seedexpander_shake_get_bytes(&seedexpander_shake, random, MIRATH_PARAM_CHALLENGE_2_BYTES + MIRATH_PARAM_HASH_2_MASK_BYTES);

    memcpy(v_grinding, &random[MIRATH_PARAM_CHALLENGE_2_BYTES], MIRATH_PARAM_HASH_2_MASK_BYTES);  // Obtain v_grinding from random
    v_grinding[MIRATH_PARAM_HASH_2_MASK_BYTES - 1] &= (uint8_t)MIRATH_PARAM_HASH_2_MASK;

    // Challenges concerning N_1
    for(size_t i = 0; i < MIRATH_PARAM_TAU_1; i++) {
        uint8_t block[MIRATH_PARAM_N_1_BYTES] = {0};
        memcpy(block, random, MIRATH_PARAM_N_1_BYTES);
        block[MIRATH_PARAM_N_1_BYTES - 1] &= MIRATH_PARAM_N_1_MASK;
        memcpy((uint8_t *)&challenge[i], block, MIRATH_PARAM_N_1_BYTES);
        // Left shift bits: starts
        mirath_tcith_shift_to_right_array(random, MIRATH_PARAM_CHALLENGE_2_BYTES, MIRATH_PARAM_N_1_BITS);
        // Left shift bits: ends
    }

    // Challenges concerning N_2
    for(size_t i = 0; i < MIRATH_PARAM_TAU_2; i++) {
        uint8_t block[MIRATH_PARAM_N_2_BYTES] = {0};
        memcpy(block, random, MIRATH_PARAM_N_2_BYTES);
        block[MIRATH_PARAM_N_2_BYTES - 1] &= MIRATH_PARAM_N_2_MASK;
        memcpy((uint8_t *)&challenge[i + MIRATH_PARAM_TAU_1], block, MIRATH_PARAM_N_2_BYTES);
        // Left shift bits: starts
        mirath_tcith_shift_to_right_array(random, MIRATH_PARAM_CHALLENGE_2_BYTES, MIRATH_PARAM_N_2_BITS);
        // Left shift bits: ends
    }
}

