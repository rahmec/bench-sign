/** 
 * @file tcith_sign.c
 * @brief Implementation of the sign procedure of the RYDE TCitH scheme
 */

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "rng.h"
#include "hash_sha3.h"
#include "parameters.h"
#include "parsing.h"
#include "tcith.h"
#include "ggm_tree.h"
#include "ryde.h"
 

/**
 * \fn int ryde_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk)
 * \brief Sign algorithm of the RYDE TCitH scheme
 *
 * \param[out] signature String containing the signature
 * \param[in] message String containing the message to be signed
 * \param[in] message_size Length of the message to be signed
 * \param[in] sk String containing the secret key
 * \return EXIT_SUCCESS if no issues appear when sampling the salt and master seed. Otherwise, it returns EXIT_FAILURE.
 */
int ryde_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk) {
    // ---------------------------------------------------------------------------------------------------- Initialization

    if (message == NULL || signature == NULL || sk == NULL) { return EXIT_FAILURE; }
    if (message_size <= 0) { return EXIT_FAILURE; }
    memset(signature, 0, RYDE_SIGNATURE_BYTES);

    // Setup variables related to randomness, hash and challenges
    ryde_tcith_commit_t commit = {0};

    ryde_tcith_seed_t seed = {0};
    ryde_ggm_tree_leaves_t seeds = {0};
    ryde_ggm_tree_t tree = {0};
    uint8_t salt_and_rseed[RYDE_SALT_BYTES + RYDE_SECURITY_BYTES] = {0};
    uint8_t salt[RYDE_SALT_BYTES] = {0};

    uint8_t domain_separator;
    uint8_t h1[RYDE_HASH_BYTES] = {0};
    uint8_t h2[RYDE_HASH_BYTES] = {0};
    uint8_t v_grinding[RYDE_PARAM_W_BYTES * SHAKE_STEP] = {0};
    hash_sha3_ctx ctx_m, ctx_h1, ctx_h2;
    hash_sha3_init(&ctx_m);
    hash_sha3_init(&ctx_h1);

    uint8_t m_digest[RYDE_HASH_BYTES] = {0};

    // Setup variables related to base
    field_vector_t overline_v[RYDE_PARAM_RHO] = {0}, acc_v[RYDE_PARAM_TAU][RYDE_PARAM_RHO] = {0};
    field_vector_t overline_s_[RYDE_PARAM_R - 1] = {0}, acc_s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1] = {0};
    binary_matrix_t overline_C[RYDE_PARAM_R] = {0}, 
                    acc_C[RYDE_PARAM_TAU][RYDE_PARAM_R] = {0};
    field_matrix_t overline_D[RYDE_PARAM_R] = {0};
    ryde_tcith_shares_t base;
    field_matrix_t gamma[RYDE_PARAM_N - RYDE_PARAM_K] = {0};
    field_vector_t sC[RYDE_PARAM_N - RYDE_PARAM_R] = {0},
                       base_a[RYDE_PARAM_RHO] = {0},
                       mid_a[RYDE_PARAM_RHO] = {0};
    ryde_tcith_alpha_t base_a_str[RYDE_PARAM_TAU] = {0};
    ryde_tcith_alpha_t mid_a_str[RYDE_PARAM_TAU] = {0};
    ryde_tcith_challenge_t i_star[SHAKE_STEP] = {0}, psi_i_star = {0};

    ryde_ggm_tree_node_t path[RYDE_PARAM_MAX_OPEN] = {0};

    ryde_tcith_share_s_t aux_s_str[RYDE_PARAM_TAU] = {0};
    ryde_tcith_share_C_t aux_C_str[RYDE_PARAM_TAU] = {0};

    // Setup variables related to sk and pk
    uint8_t pk[RYDE_PUBLIC_KEY_BYTES] = {0};
    field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K] = {0};
    field_vector_t y[RYDE_PARAM_N - RYDE_PARAM_K] = {0}, s[RYDE_PARAM_R] = {0}; // By construction, s refers to the vector (1 | s')
    binary_matrix_t C[RYDE_PARAM_R] = {0};

    // Initialize variables related to base
    field_vector_t mid_alpha[RYDE_PARAM_TAU][RYDE_PARAM_RHO] = {0};

    // Parse secret key and public key
    ryde_secret_key_from_string(y, H, s, C, sk);
    ryde_public_key_to_string(pk, &sk[RYDE_SECURITY_BYTES], y);

    binary_matrix_mul_by_vector_left(sC,
                                     (binary_matrix_t *)&(C[1]),
                                     (field_vector_t *)&s[1],
                                     RYDE_PARAM_R - 1,
                                     RYDE_PARAM_N - RYDE_PARAM_R);

    // Hash message
    domain_separator = DOMAIN_SEPARATOR_MESSAGE;
    hash_sha3_absorb(&ctx_m, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx_m, message, message_size);
    hash_sha3_finalize(m_digest, &ctx_m);

    // -------------------------------------------------------------------------------------------- Shares and commitments

    // Sample salt and root seed
    if (randombytes(salt_and_rseed, RYDE_SALT_BYTES + RYDE_SECURITY_BYTES) != EXIT_SUCCESS) {
        memset(salt_and_rseed, 0, RYDE_SALT_BYTES + RYDE_SECURITY_BYTES);
        return EXIT_FAILURE;
    }

    // Initialize tree

    memcpy(&salt, salt_and_rseed, RYDE_SALT_BYTES);                           // salt
    memcpy(tree[0], &salt_and_rseed[RYDE_SALT_BYTES], RYDE_SECURITY_BYTES);   // root seed
    ryde_ggm_tree_expand(tree, salt);
    ryde_ggm_tree_get_leaves(seeds, tree);

    // Add salt to ctx_h1
    domain_separator = DOMAIN_SEPARATOR_HASH1;
    hash_sha3_absorb(&ctx_h1, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx_h1, salt, RYDE_SALT_BYTES);

    for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
        // Set to zero the accumulator vector and matrix
        field_vector_set_to_zero(base.s[e], RYDE_PARAM_R - 1);
        field_matrix_set_to_zero(base.C[e], RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
        field_vector_set_to_zero(base.v[e], RYDE_PARAM_RHO);

        field_vector_set_to_zero(acc_s[e], RYDE_PARAM_R - 1);
        binary_matrix_set_to_zero(acc_C[e], RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
        field_vector_set_to_zero(acc_v[e], RYDE_PARAM_RHO);

        size_t N = e < RYDE_PARAM_TAU_1? RYDE_PARAM_N_1 : RYDE_PARAM_N_2;
        for(size_t i = 0; i < N; i++) {
            size_t idx = ryde_tcith_psi(i, e);
            memcpy(seed, &seeds[idx], RYDE_SECURITY_BYTES);

            // Compute commit and add it to ctx_h1
            ryde_tcith_commit(commit, salt, e, i, seed);

            hash_sha3_absorb(&ctx_h1, commit, RYDE_HASH_BYTES);

            ryde_tcith_expand_share(overline_s_, overline_C, overline_v, seed, salt);

            field_vector_add(acc_s[e], acc_s[e], overline_s_, RYDE_PARAM_R - 1);
            binary_matrix_add(acc_C[e], acc_C[e], overline_C, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
            field_vector_add(acc_v[e], acc_v[e], overline_v, RYDE_PARAM_RHO);

            // Compute random base
            field_t phi_i;
            ryde_tcith_phi(phi_i, i);

            field_vector_mul_by_constant(overline_s_, overline_s_, phi_i, RYDE_PARAM_R - 1);
            binary_matrix_mul_by_constant(overline_D, overline_C, phi_i, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
            field_vector_mul_by_constant(overline_v, overline_v, phi_i, RYDE_PARAM_RHO);

            field_vector_add(base.s[e], base.s[e], overline_s_, RYDE_PARAM_R - 1);
            field_matrix_add(base.C[e], base.C[e], overline_D, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
            field_vector_add(base.v[e], base.v[e], overline_v, RYDE_PARAM_RHO);
        }


        // Compute (s - acc_s[e])
        field_vector_add(acc_s[e], (field_vector_t *)&s[1], acc_s[e], RYDE_PARAM_R - 1);
        field_vector_to_string(aux_s_str[e], acc_s[e], RYDE_PARAM_R - 1);

        // Compute (C - acc_C[e])
        binary_matrix_add(acc_C[e], C, acc_C[e], RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
        binary_matrix_to_string(aux_C_str[e], acc_C[e], RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);

    }

    for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
        hash_sha3_absorb(&ctx_h1, aux_s_str[e], RYDE_VEC_R_MINUS_ONE_BYTES);
        hash_sha3_absorb(&ctx_h1, aux_C_str[e], RYDE_MAT_FQ_BYTES);
    }

    // --------------------------------------------------------------------------------------------------- First challenge
    // Generate h1 and gamma
    hash_sha3_finalize(h1, &ctx_h1);
    ryde_tcith_expand_challenge_1(gamma, h1, salt);

    // --------------------------------------------------------------------------------------- Compute the Polynomial Proof
    for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
        ryde_tcith_compute_polynomial_proof(base_a, mid_a, base, s, C, acc_v, gamma, H, e);

        field_vector_to_string(base_a_str[e], base_a, RYDE_PARAM_RHO);
        field_vector_to_string(mid_a_str[e], mid_a, RYDE_PARAM_RHO);
        field_vector_copy(mid_alpha[e], mid_a, RYDE_PARAM_RHO);
    }

    // -------------------------------------------------------------------------------------------------- Second challenge

    // Initialize ctx_h2
    hash_sha3_init(&ctx_h2);
    // Add m, pk, salt and h1 to ctx_h2
    domain_separator = DOMAIN_SEPARATOR_HASH2;
    hash_sha3_absorb(&ctx_h2, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx_h2, m_digest, RYDE_HASH_BYTES);
    hash_sha3_absorb(&ctx_h2, pk, RYDE_PUBLIC_KEY_BYTES);
    hash_sha3_absorb(&ctx_h2, salt, RYDE_SALT_BYTES);
    hash_sha3_absorb(&ctx_h2, h1, RYDE_HASH_BYTES);
    for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
        hash_sha3_absorb(&ctx_h2, base_a_str[e], RYDE_VEC_RHO_BYTES);
        hash_sha3_absorb(&ctx_h2, mid_a_str[e], RYDE_VEC_RHO_BYTES);
    }
    hash_sha3_finalize(h2, &ctx_h2);

    // SHAKE input (fixed prefix)
    uint8_t shake_input[SHAKE_STEP * (RYDE_HASH_BYTES + sizeof(uint64_t))] = {0};
    for(size_t index = 0; index < SHAKE_STEP; index++) {
        memcpy(&shake_input[(RYDE_HASH_BYTES + sizeof(uint64_t)) * index], h2, RYDE_HASH_BYTES);
    }

    uint64_t ctr = 0;
    uint64_t number = 0;
    retry:

    // SHAKE input (suffix corresponds with the counter)
    for(size_t index = 0; index < SHAKE_STEP; index++) {
        uint64_t ctr_index = ctr + index;
        memcpy(&shake_input[(RYDE_HASH_BYTES + sizeof(uint64_t)) * index + RYDE_HASH_BYTES], (uint8_t *)&ctr_index, sizeof(uint64_t));
    }
    
#ifdef _SHAKE_TIMES4_
    ryde_tcith_expand_challenge_2_x4(i_star, v_grinding, shake_input);
#else
    ryde_tcith_expand_challenge_2(i_star, v_grinding, shake_input);
#endif

    number = 0;
    size_t path_length = RYDE_PARAM_T_OPEN + 1;
    retry_step:
    if (ryde_tcith_discard_input_challenge_2(&v_grinding[number * RYDE_PARAM_W_BYTES])) {
        memset(&v_grinding[number * RYDE_PARAM_W_BYTES], 0, RYDE_PARAM_W_BYTES);
        memset(path, 0, sizeof(ryde_ggm_tree_node_t));
        number += 1;
        if (number == SHAKE_STEP) {
            ctr += SHAKE_STEP;
            goto retry;
        } else {
            goto retry_step;
        }
    }
        
    // Next we map the challenges to the leaves position (GGM Tree optimization)
    for(size_t e = 0; e < RYDE_PARAM_TAU; e++){
        size_t i = i_star[number][e];
        psi_i_star[e] = ryde_tcith_psi(i, e);
    }

    path_length = ryde_ggm_tree_get_sibling_path(path, (const ryde_ggm_tree_node_t *)tree, psi_i_star);
    if (path_length > RYDE_PARAM_T_OPEN) {
        memset(&v_grinding[number * RYDE_PARAM_W_BYTES], 0, RYDE_PARAM_W_BYTES);
        memset(path, 0, path_length * RYDE_SECURITY_BYTES);
        number += 1;
        if (number == SHAKE_STEP) {
            ctr += SHAKE_STEP;
            goto retry;
        } else {
            goto retry_step;
        }
    }

    ctr += number;

    // --------------------------------------------------------------------------------------------------------- Signature
    memcpy(&signature[0], salt, RYDE_SALT_BYTES);
    memcpy(&signature[RYDE_SALT_BYTES], &ctr, sizeof(uint64_t));
    memcpy(&signature[RYDE_SALT_BYTES + sizeof(uint64_t)], h2, RYDE_HASH_BYTES);
    memcpy(&signature[RYDE_SALT_BYTES + RYDE_HASH_BYTES + sizeof(uint64_t)], path, RYDE_SECURITY_BYTES * RYDE_PARAM_T_OPEN);

    for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
        // Commitment concerning the hidden seed
        size_t idx = ryde_tcith_psi(i_star[number][e], e);
        ryde_tcith_commit(commit, salt, e, i_star[number][e], seeds[idx]);
        memcpy(&signature[
                RYDE_SALT_BYTES +
                RYDE_HASH_BYTES +
                sizeof(uint64_t) +
                RYDE_PARAM_T_OPEN * RYDE_SECURITY_BYTES +
                e * RYDE_HASH_BYTES
            ],
            commit,
            RYDE_HASH_BYTES);
    }
    ryde_pack_matrices_and_vectors(&signature[
                                            RYDE_SALT_BYTES +
                                            RYDE_HASH_BYTES +
                                            sizeof(uint64_t) +
                                            RYDE_PARAM_T_OPEN * RYDE_SECURITY_BYTES +
                                            RYDE_PARAM_TAU * RYDE_HASH_BYTES
                                            ],
                                    acc_s, acc_C, mid_alpha);

    // ------------------------------------------------------------------------------------------------------ Verbose Mode
    #ifdef VERBOSE
        printf("\n\n### SIGN ###\n");

        printf("\nsk: "); for(int i = 0 ; i < RYDE_SECRET_KEY_BYTES ; ++i) printf("%02X", sk[i]);
        printf("\npk: "); for(int i = 0 ; i < RYDE_PUBLIC_KEY_BYTES ; ++i) printf("%02X", pk[i]);

        printf("\nx: (s | sC)");
        uint8_t s_string[RYDE_VEC_R_BYTES];
        field_vector_to_string(s_string, s, RYDE_PARAM_R);
        printf("\n    - s      : "); for(size_t i = 0 ; i < RYDE_VEC_R_BYTES ; i++) { printf("%02X", s_string[i]); }
        memset(s_string, 0, RYDE_VEC_R_BYTES);
        uint8_t C_string[RYDE_MAT_FQ_BYTES] = {0};
        binary_matrix_to_string(C_string, C, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
        printf("\n    - C      : "); for(size_t i = 0 ; i < RYDE_MAT_FQ_BYTES ; i++) { printf("%02X", C_string[i]); }
        memset(C_string, 0, RYDE_MAT_FQ_BYTES);
        printf("\n\nm_digest: "); for(size_t i = 0 ; i < RYDE_HASH_BYTES ; i++) { printf("%02X", m_digest[i]); }

        size_t length = ((RYDE_PARAM_N - RYDE_PARAM_K) * RYDE_PARAM_K * RYDE_PARAM_M + 7 ) / 8;
        uint8_t H_string[length];
        field_matrix_to_string(H_string, H, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);
        printf("\nH: "); for(size_t i = 0 ; i < length ; i++) { printf("%02X", H_string[i]); }
        memset(H_string, 0, length);

        length = ((RYDE_PARAM_N - RYDE_PARAM_K) * RYDE_PARAM_M + 7 ) / 8;
        uint8_t y_string[length];
        field_vector_to_string(y_string, y, RYDE_PARAM_N - RYDE_PARAM_K);
        printf("\ny: "); for(size_t i = 0 ; i < length ; i++) { printf("%02X", y_string[i]); }
        memset(y_string, 0, length);

        printf("\n\ntree: ");
        printf("\n    - root   : ");
        for(size_t i = 0 ; i < RYDE_SECURITY_BYTES ; i++) {
        printf("%02X", salt_and_rseed[i + RYDE_SALT_BYTES]);
        }

        printf("\n\nsalt: "); for(size_t i = 0 ; i < RYDE_SALT_BYTES ; i++) { printf("%02X", salt[i]); }
        printf("\n\nh1: "); for(size_t i = 0 ; i < RYDE_HASH_BYTES ; i++) { printf("%02X", h1[i]); }
        printf("\nh2: "); for(size_t i = 0 ; i < RYDE_HASH_BYTES ; i++) { printf("%02X", h2[i]); }
        printf("\n\nchallenges:");
        printf("\n    - i_star     :"); for(size_t e = 0; e < RYDE_PARAM_TAU; e++) { printf(" %05zu", i_star[number][e]); }
        printf("\n    - psi_i_star :"); for(size_t e = 0; e < RYDE_PARAM_TAU; e++) { printf(" %05zu", psi_i_star[e]); }
        printf("\nsibling path:"); ryde_ggm_tree_print_sibling_path(path);
        printf("\n\nsigma: "); for(size_t i = 0; i < RYDE_SIGNATURE_BYTES; i++) { printf("%02X", signature[i]); }
        printf("\n\nGamma:\n"); field_matrix_print(gamma, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_RHO);
        printf("\n\nctr: %" PRId64 "\n", ctr);

        printf("\nbase_alpha:");
        for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
            printf("\n    - "); for (size_t i = 0; i < RYDE_VEC_RHO_BYTES; i++) { printf("%02X", base_a_str[e][i]); }
        }
        printf("\n\nmid_alpha:");
        for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
            printf("\n    - "); for (size_t i = 0; i < RYDE_VEC_RHO_BYTES; i++) { printf("%02X", mid_a_str[e][i]); }
        }

        printf("\n");
    #endif

    // -------------------------------------------------------------------------------------------------------- Clear Data
    memset(salt, 0, RYDE_SALT_BYTES);
    memset(salt_and_rseed, 0, RYDE_SALT_BYTES + RYDE_SECURITY_BYTES);

    return EXIT_SUCCESS;
}
