#include <stdint.h>
#include <string.h>

#include "mirath_matrix_ff.h"
#include "mirath_parsing.h"
#include "mirath_tcith.h"
#include "utils.h"

int mirath_verify(uint8_t *msg, const size_t *msg_len, uint8_t *sig_msg, uint8_t *pk) {
    uint8_t salt[MIRATH_PARAM_SALT_BYTES];
    hash_t h_mpc_prime = {0};
    hash_t h_mpc = {0};
    uint64_t ctr;
    mirath_tcith_view_challenge_t i_star;
    ff_t H[MIRATH_VAR_FF_H_BYTES];
    ff_t y[MIRATH_VAR_FF_Y_BYTES];
    mirath_tcith_commit_t commits_i_star[MIRATH_PARAM_TAU];
    mirath_ggm_tree_node_t path[MIRATH_PARAM_MAX_OPEN] = {0};
    ff_mu_t S_share[MIRATH_PARAM_TAU][MIRATH_VAR_S];
    ff_mu_t C_share[MIRATH_PARAM_TAU][MIRATH_VAR_C];
    ff_mu_t v_share[MIRATH_PARAM_TAU][MIRATH_PARAM_RHO];
    ff_mu_t Gamma[MIRATH_VAR_GAMMA];
    ff_t aux[MIRATH_PARAM_TAU][MIRATH_VAR_FF_AUX_BYTES] = {0};
    ff_mu_t alpha_mid[MIRATH_PARAM_TAU][MIRATH_PARAM_RHO] = {0};

    int ret = 0;

    /*
     * Phase 0: Initialization (parsing and expansion)
     * step 1
     */
    ret = parse_signature(salt, &ctr, h_mpc, path, commits_i_star, aux, alpha_mid, sig_msg);
    if (ret != 0) {
        return -1;
    }

    // step 2
    mirath_matrix_decompress_pk(H, y, pk);

    /*
     * Phase 1: Computing Opened Evaluations.
     * step 3
     */
    uint8_t v_grinding[MIRATH_PARAM_HASH_2_MASK_BYTES] = {0};
    uint8_t shake_input[2 * MIRATH_SECURITY_BYTES + sizeof(uint64_t)] = {0};
    memcpy(&shake_input[0], h_mpc, 2 * MIRATH_SECURITY_BYTES);
    memcpy(&shake_input[2 * MIRATH_SECURITY_BYTES], (uint8_t *)&ctr, sizeof(uint64_t));
    mirath_tcith_expand_view_challenge(i_star, v_grinding, shake_input);

    uint8_t domain_separator;

    size_t psi_i_star[MIRATH_PARAM_TAU];
    size_t path_length = 0;

    for(size_t e = 0; e < MIRATH_PARAM_TAU; e++){
        const size_t i = i_star[e];
        psi_i_star[e] = mirath_tcith_psi(i, e);
    }

    for(uint32_t i = 0; i < MIRATH_PARAM_T_OPEN; i++) {
        const uint8_t zero[MIRATH_SECURITY_BYTES] = {0};
        if (memcmp(zero, &path[i], MIRATH_SECURITY_BYTES) == 0) { continue; }
        path_length += 1;
    }

    // step 5, step 6, and 7
    mirath_ggm_tree_t tree = {0};
    if (mirath_ggm_tree_partial_expand(tree, salt, path, path_length, psi_i_star) != 0) {
        return -1;
    }

    mirath_ggm_tree_leaves_t seeds = {0};
    mirath_ggm_tree_get_leaves(seeds, tree);

    domain_separator = DOMAIN_SEPARATOR_COMMITMENT;
    hash_sha3_ctx hash_commits;
    hash_init(&hash_commits);
    hash_update(&hash_commits, &domain_separator, sizeof(uint8_t));

    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        memset(S_share[e], 0, sizeof(ff_mu_t) * MIRATH_VAR_S);
        memset(C_share[e], 0, sizeof(ff_mu_t) * MIRATH_VAR_C);
        memset(v_share[e], 0, sizeof(ff_mu_t) * MIRATH_PARAM_RHO);

        compute_share(S_share[e], C_share[e], v_share[e], &hash_commits, commits_i_star, i_star[e], seeds, e, aux[e], salt);
    }

    hash_t h_com = {0};
    hash_finalize(h_com, &hash_commits);

    hash_sha3_ctx hash_sh_ctx;
    domain_separator = DOMAIN_SEPARATOR_HASH1;
    hash_init(&hash_sh_ctx);
    hash_update(&hash_sh_ctx, &domain_separator, sizeof(uint8_t));
    hash_update(&hash_sh_ctx, salt, MIRATH_PARAM_SALT_BYTES);
    hash_update(&hash_sh_ctx, h_com, 2 * MIRATH_SECURITY_BYTES);

    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        hash_update(&hash_sh_ctx, aux[e], MIRATH_VAR_FF_AUX_BYTES);
    }

    hash_t h_sh;
    hash_finalize(h_sh, &hash_sh_ctx);

    /*
     * Phase 2: Recomputation of the Polynomial Proof P_alpha(X).
     * step 4
     * This block of code refers to Algorithm 9 Challenge matrix Gamma.
     */
    shake_prng_t seedexpander_shake;
    seedexpander_shake_init(&seedexpander_shake, h_sh, 2 * MIRATH_SECURITY_BYTES, NULL, 0);
    seedexpander_shake_get_bytes(&seedexpander_shake, (uint8_t*)Gamma, sizeof(ff_mu_t) * MIRATH_VAR_GAMMA);

    domain_separator = DOMAIN_SEPARATOR_HASH2_PARTIAL;
    hash_sha3_ctx hash_mpc_ctx;
    hash_init(&hash_mpc_ctx);
    hash_update(&hash_mpc_ctx, &domain_separator, sizeof(uint8_t));
    hash_update(&hash_mpc_ctx, pk, MIRATH_PUBLIC_KEY_BYTES);
    hash_update(&hash_mpc_ctx, salt, MIRATH_PARAM_SALT_BYTES);
    hash_update(&hash_mpc_ctx, msg, *msg_len);
    hash_update(&hash_mpc_ctx, h_sh, 2 * MIRATH_SECURITY_BYTES);

    // steps 5 and 6
    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        ff_mu_t alpha_base[MIRATH_PARAM_RHO];
        recompute_polynomial_proof(alpha_base, i_star[e], S_share[e], C_share[e], v_share[e], Gamma, H, y, alpha_mid[e]);

        hash_update(&hash_mpc_ctx, (uint8_t*)alpha_base, sizeof(ff_mu_t) * MIRATH_PARAM_RHO);
        hash_update(&hash_mpc_ctx, (uint8_t*)alpha_mid[e], sizeof(ff_mu_t) * MIRATH_PARAM_RHO);
    }

    /*
     * Phase 3: Verification
     * step 7
     */
    hash_finalize(h_mpc_prime, &hash_mpc_ctx);

    // step 8
    if (!hash_equal(h_mpc, h_mpc_prime, 2 * MIRATH_SECURITY_BYTES)) {
        ret = -1;
    }

    return ret;
}
