/** 
 * @file tcith_verify.c
 * @brief Implementation of the verify procedure of the RYDE TCitH scheme
 */

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "hash_sha3.h"
#include "parameters.h"
#include "parsing.h"
#include "tcith.h"
#include "ggm_tree.h"
#include "ryde.h"


/**
 * \fn int ryde_verify(const uint8_t* message, size_t message_size, const uint8_t* signature, size_t signature_size, const uint8_t* pk)
 * \brief Verify of the RYDE TCitH scheme
 *
 * The public key is composed of the vector <b>y</b> as well as the seed used to generate matrix <b>H</b>.
 *
 * \param[in] signature String containing the signature
 * \param[in] signature_size Integer determining the signed message byte-length
 * \param[in] message String containing the message to be signed
 * \param[in] message_size Integer determining the message byte-length
 * \param[in] pk String containing the public key
 * \return EXIT_SUCCESS if verify is successful. Otherwise, it returns EXIT_FAILURE
 */
int ryde_verify(const uint8_t* signature, size_t signature_size, const uint8_t* message, size_t message_size, const uint8_t* pk) {
    // ---------------------------------------------------------------------------------------------------- Initialization

    if(signature == NULL || message == NULL || pk == NULL) { return EXIT_FAILURE; }
    if(signature_size != RYDE_SIGNATURE_BYTES) { return EXIT_FAILURE; }

    // Setup variables related to randomness, salt, hash and challenges
    ryde_tcith_commit_t commit = {0};

    uint8_t salt[RYDE_SALT_BYTES] = {0};
    uint64_t ctr = 0;

    uint8_t domain_separator;
    uint8_t h1[RYDE_HASH_BYTES] = {0};
    uint8_t h2[RYDE_HASH_BYTES] = {0};
    uint8_t h2_[RYDE_HASH_BYTES] = {0};
    uint8_t v_grinding[RYDE_PARAM_W_BYTES] = {0};
    hash_sha3_ctx ctx_m, ctx_h1, ctx_h2;
    hash_sha3_init(&ctx_m);
    hash_sha3_init(&ctx_h1);

    uint8_t m_digest[RYDE_HASH_BYTES] = {0};

    ryde_tcith_challenge_t i_star = {0}, psi_i_star = {0};
    ryde_tcith_share_s_t aux_s_str[RYDE_PARAM_TAU] = {0};
    ryde_tcith_share_C_t aux_C_str[RYDE_PARAM_TAU] = {0};

    ryde_tcith_seed_t seed = {0};
    ryde_ggm_tree_leaves_t seeds = {0};
    ryde_ggm_tree_t tree = {0};
    ryde_ggm_tree_node_t path[RYDE_PARAM_MAX_OPEN] = {0};

    // Setup variables related to shares
    field_vector_t overline_v[RYDE_PARAM_RHO] = {0};
    field_vector_t overline_s_[RYDE_PARAM_R - 1] = {0};
    binary_matrix_t overline_C[RYDE_PARAM_R] = {0};
    field_matrix_t overline_D[RYDE_PARAM_R] = {0};
    ryde_tcith_shares_t shares;
    field_matrix_t gamma[RYDE_PARAM_N - RYDE_PARAM_K] = {0};
    field_vector_t base_a[RYDE_PARAM_RHO] = {0};
    ryde_tcith_alpha_t base_a_str[RYDE_PARAM_TAU] = {0};
    ryde_tcith_alpha_t mid_a_str[RYDE_PARAM_TAU] = {0};

    // Setup variables related to pk
    field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K] = {0};
    field_vector_t y[RYDE_PARAM_N - RYDE_PARAM_K] = {0};

    field_vector_t aux_s[RYDE_PARAM_TAU][RYDE_PARAM_R - 1] = {0},
                       mid_alpha[RYDE_PARAM_TAU][RYDE_PARAM_RHO] = {0};
    binary_matrix_t aux_C[RYDE_PARAM_TAU][RYDE_PARAM_R] = {0};
    
    ryde_unpack_matrices_and_vectors(aux_s, aux_C, mid_alpha,
                                    &signature[
                                            RYDE_SALT_BYTES +
                                            RYDE_HASH_BYTES +
                                            sizeof(uint64_t) +
                                            RYDE_PARAM_T_OPEN * RYDE_SECURITY_BYTES +
                                            RYDE_PARAM_TAU * RYDE_HASH_BYTES
                                            ]);

    // Parse public key
    ryde_public_key_from_string(H, y, pk);

    // Hash message
    domain_separator = DOMAIN_SEPARATOR_MESSAGE;
    hash_sha3_absorb(&ctx_m, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx_m, message, message_size);
    hash_sha3_finalize(m_digest, &ctx_m);

    // Parse signature data
    memcpy(salt, &signature[0], RYDE_SALT_BYTES);
    memcpy(&ctr, &signature[RYDE_SALT_BYTES], sizeof(uint64_t));
    memcpy(h2,   &signature[RYDE_SALT_BYTES + sizeof(uint64_t)], RYDE_HASH_BYTES);
    memcpy(path, &signature[RYDE_SALT_BYTES + RYDE_HASH_BYTES + sizeof(uint64_t)], RYDE_SECURITY_BYTES * RYDE_PARAM_T_OPEN);

    // SHAKE input
    uint8_t shake_input[RYDE_HASH_BYTES + sizeof(uint64_t)] = {0};
    memcpy(&shake_input[0], h2, RYDE_HASH_BYTES);
    memcpy(&shake_input[RYDE_HASH_BYTES], (uint8_t *)&ctr, sizeof(uint64_t));
    ryde_tcith_expand_challenge_2(&i_star, v_grinding, shake_input);

    // Next we map the challenges to the leaves position (GGM Tree optimization)
    for(size_t e = 0; e < RYDE_PARAM_TAU; e++){
        size_t i = i_star[e];
        psi_i_star[e] = ryde_tcith_psi(i, e);
    }

    // Get sibling path length: starts
    size_t path_length = 0;
    for(size_t i = 0; i < RYDE_PARAM_T_OPEN; i++) {
        uint8_t zero[RYDE_SECURITY_BYTES] = {0};
        if (memcmp(zero, &path[i], RYDE_SECURITY_BYTES) == 0) { continue; }
        path_length += 1;
    }
    // Get sibling path length: ends

    // Add salt to ctx_h1
    domain_separator = DOMAIN_SEPARATOR_HASH1;
    hash_sha3_absorb(&ctx_h1, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx_h1, salt, RYDE_SALT_BYTES);

    // ---------------------------------------------------------------------------------- Recompute shares and commitments

    int wrong_path = ryde_ggm_tree_partial_expand(tree, salt, (const ryde_ggm_tree_node_t*)path, path_length, psi_i_star);
    ryde_ggm_tree_get_leaves(seeds, tree);


    for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
        // Set to zero the accumulator vector and matrix
        field_vector_set_to_zero(shares.s[e], RYDE_PARAM_R - 1);
        field_matrix_set_to_zero(shares.C[e], RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
        field_vector_set_to_zero(shares.v[e], RYDE_PARAM_RHO);

        field_t phi_i_star;
        ryde_tcith_phi(phi_i_star, i_star[e]);

        size_t N = e < RYDE_PARAM_TAU_1? RYDE_PARAM_N_1 : RYDE_PARAM_N_2;
        for(size_t i = 0; i < N; i++) {
            size_t idx = ryde_tcith_psi(i, e);
            memcpy(seed, &seeds[idx], RYDE_SECURITY_BYTES);

            if (i == (size_t)i_star[e]) {
                memcpy(commit,
                    &signature[
                        RYDE_SALT_BYTES + RYDE_HASH_BYTES + sizeof(uint64_t) + RYDE_PARAM_T_OPEN * RYDE_SECURITY_BYTES +
                        e * RYDE_HASH_BYTES
                    ],
                    RYDE_HASH_BYTES);
                hash_sha3_absorb(&ctx_h1, commit, RYDE_HASH_BYTES);
            }
            else {
                // Compute commit and add it to ctx_h1
                ryde_tcith_commit(commit, salt, e, i, seed);

                hash_sha3_absorb(&ctx_h1, commit, RYDE_HASH_BYTES);

                ryde_tcith_expand_share(overline_s_, overline_C, overline_v, seed, salt);

                // Compute shares
                field_t scalar;
                ryde_tcith_phi(scalar, i);
                field_add(scalar, phi_i_star, scalar);

                field_vector_mul_by_constant(overline_s_, overline_s_, scalar, RYDE_PARAM_R - 1);
                binary_matrix_mul_by_constant(overline_D, overline_C, scalar, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
                field_vector_mul_by_constant(overline_v, overline_v, scalar, RYDE_PARAM_RHO);

                field_vector_add(shares.s[e], shares.s[e], overline_s_, RYDE_PARAM_R - 1);
                field_matrix_add(shares.C[e], shares.C[e], overline_D, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
                field_vector_add(shares.v[e], shares.v[e], overline_v, RYDE_PARAM_RHO);
            }
        }


        // Operations concerning vector se
        field_vector_to_string(aux_s_str[e], aux_s[e], RYDE_PARAM_R - 1);
        field_vector_mul_by_constant(overline_s_, aux_s[e], phi_i_star, RYDE_PARAM_R - 1);

        // Operations concerning matrix Ce
        binary_matrix_to_string(aux_C_str[e], aux_C[e], RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
        binary_matrix_mul_by_constant(overline_D, aux_C[e], phi_i_star, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);


        field_vector_add(shares.s[e], shares.s[e], overline_s_, RYDE_PARAM_R - 1);
        field_matrix_add(shares.C[e], shares.C[e], overline_D, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
    }

    for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
        hash_sha3_absorb(&ctx_h1, aux_s_str[e], RYDE_VEC_R_MINUS_ONE_BYTES);
        hash_sha3_absorb(&ctx_h1, aux_C_str[e], RYDE_MAT_FQ_BYTES);
    }

    // --------------------------------------------------------------------------------------------------- First Challenge

    // Generate h1 and gamma
    hash_sha3_finalize(h1, &ctx_h1);
    ryde_tcith_expand_challenge_1(gamma, h1, salt);

    // ------------------------------------------------------------------------------------- Recompute the Polynomial Proof
    for(size_t e = 0; e < RYDE_PARAM_TAU; e++) {
        ryde_tcith_recompute_polynomial_proof(base_a, i_star, shares, gamma, H, y, mid_alpha, e);
        
        field_vector_to_string(mid_a_str[e], mid_alpha[e], RYDE_PARAM_RHO);
        field_vector_to_string(base_a_str[e], base_a, RYDE_PARAM_RHO);
    }

    // ------------------------------------------------------------------------------------------------------ Verification

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
    hash_sha3_finalize(h2_, &ctx_h2);

    // ------------------------------------------------------------------------------------------------------ Verbose Mode
    #ifdef VERBOSE
        printf("\n### VERIFY ###\n");

        printf("\npk: "); for(int i = 0 ; i < RYDE_PUBLIC_KEY_BYTES ; ++i) printf("%02X", pk[i]);

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

        printf("\n\nm_digest: "); for(size_t i = 0 ; i < RYDE_HASH_BYTES ; i++) { printf("%02X", m_digest[i]); }

        printf("\n\nsigma: "); for(size_t i = 0; i < RYDE_SIGNATURE_BYTES; i++) { printf("%02X", signature[i]); } printf("\n");

        printf("\n\nsalt: "); for(size_t i = 0 ; i < RYDE_SALT_BYTES ; i++) { printf("%02X", salt[i]); }
        printf("\n\nh1: "); for(size_t i = 0 ; i < RYDE_HASH_BYTES ; i++) { printf("%02X", h1[i]); }
        printf("\nh2: "); for(size_t i = 0 ; i < RYDE_HASH_BYTES ; i++) { printf("%02X", h2[i]); }
        printf("\n\nchallenges:");
        printf("\n    - i_star     :"); for(size_t e = 0; e < RYDE_PARAM_TAU; e++) { printf(" %05zu", i_star[e]); }
        printf("\n    - psi_i_star :"); for(size_t e = 0; e < RYDE_PARAM_TAU; e++) { printf(" %05zu", psi_i_star[e]); }
        printf("\nsibling path:"); ryde_ggm_tree_print_sibling_path(path);
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

        printf("\n\nh2\': "); for(size_t i = 0 ; i < RYDE_HASH_BYTES ; i++) { printf("%02X", h2_[i]); }
        printf("\n");
    #endif

    // ------------------------------------------------------------------------------------------------------ Profile Mode


    // ------------------------------------------------------------------------------------------------------ Verification
    if ((memcmp(h2, h2_, RYDE_HASH_BYTES) != 0) || wrong_path || ryde_tcith_discard_input_challenge_2(v_grinding)) {
        memset(h1, 0, RYDE_HASH_BYTES);
        memset(h2, 0, RYDE_HASH_BYTES);;
        memset(h2_, 0, RYDE_HASH_BYTES);

        return EXIT_FAILURE;
    }
    else {
        memset(h1, 0, RYDE_HASH_BYTES);
        memset(h2, 0, RYDE_HASH_BYTES);
        memset(h2_, 0, RYDE_HASH_BYTES);

        return EXIT_SUCCESS;
}
}
