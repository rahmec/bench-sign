#ifndef MIRATH_MIRATH_TCITH_H
#define MIRATH_MIRATH_TCITH_H

#include <stdint.h>
#include <stdlib.h>

#include "mirath_matrix_ff.h"
#include "mirath_ggm_tree.h"
#include "seed_expand_rijndael.h"
#include "framework_data_types.h"
#include "hash_sha3.h"

static inline size_t mirath_tcith_psi(size_t i, size_t e) {
    if (i < MIRATH_PARAM_N_2) {
        return i * MIRATH_PARAM_TAU + e;
    }
    else {
        return MIRATH_PARAM_N_2 * MIRATH_PARAM_TAU + (i - MIRATH_PARAM_N_2) * MIRATH_PARAM_TAU_1 + e;
    }
}

/**
* \fn uint8_t mirath_tcith_discard_input_challenge_2(const uint8_t *v_grinding)
* \brief This function determines if the w most significant bits of the input are zero.
*
* \param[in] v_grinding String containing the input seed
*/
static inline uint8_t mirath_tcith_discard_input_challenge_2(const uint8_t *v_grinding) {
    uint8_t output = 0x00;
    uint8_t mask = MIRATH_PARAM_HASH_2_MASK;
    for(size_t i = 0; i < MIRATH_PARAM_HASH_2_MASK_BYTES; i++) {
        output |= (uint8_t)((v_grinding[i] & mask) != 0);
        mask = 0xFF;
    }

    return output;
}

static inline void mirath_tcith_commit_set_as_a_grid_list(mirath_tcith_commit_t *seeds[MIRATH_PARAM_TAU],
                                            mirath_tcith_commit_1_t *input_1,
                                            mirath_tcith_commit_2_t *input_2) {
    // The below lines represent the leaves as a tau-dimensional grid
    for (size_t i = 0; i < MIRATH_PARAM_TAU_1; i++) {
        seeds[i] = (*input_1)[i];
    }
    for (size_t i = 0; i < MIRATH_PARAM_TAU_2; i++) {
        seeds[i + MIRATH_PARAM_TAU_1] = (*input_2)[i];
    }
}

void mirath_commit(uint8_t pair_node[2][MIRATH_SECURITY_BYTES],
                              const uint8_t salt[MIRATH_PARAM_SALT_BYTES],
                              const uint32_t i,
                              const uint8_t seed[MIRATH_SECURITY_BYTES]);

void mirath_tcith_internal_steps_pk(ff_t y[MIRATH_VAR_FF_Y_BYTES],
                                    const ff_t S[MIRATH_VAR_FF_S_BYTES], const ff_t C[MIRATH_VAR_FF_C_BYTES],
                                    const ff_t H[MIRATH_VAR_FF_H_BYTES]);

void mirath_tcith_commit(mirath_tcith_commit_t commit, const uint8_t *salt, uint16_t e, uint32_t i, const uint8_t *seed);

void commit_witness_polynomials(ff_mu_t S_base[MIRATH_PARAM_TAU][MIRATH_VAR_S],
                              ff_mu_t C_base[MIRATH_PARAM_TAU][MIRATH_VAR_C],
                              ff_mu_t v_base[MIRATH_PARAM_TAU][MIRATH_PARAM_RHO],
                              ff_mu_t v[MIRATH_PARAM_TAU][MIRATH_PARAM_RHO],
                              hash_t hash_sh,
                              mirath_ggm_tree_leaves_t seeds,
                              ff_t aux[MIRATH_PARAM_TAU][MIRATH_VAR_FF_AUX_BYTES],
                              const uint8_t salt[MIRATH_PARAM_SALT_BYTES],
                              const ff_t S[MIRATH_VAR_FF_S_BYTES],
                              const ff_t C[MIRATH_VAR_FF_C_BYTES]);

void compute_share(ff_mu_t S_share[MIRATH_VAR_S], ff_mu_t C_share[MIRATH_VAR_C],
                   ff_mu_t v_share[MIRATH_PARAM_RHO], hash_sha3_ctx *hash_commits,
                   const mirath_tcith_commit_t commits_i_star[MIRATH_PARAM_TAU],
                   uint32_t i_star, const mirath_ggm_tree_leaves_t seeds, uint32_t e,
                   const ff_t aux[MIRATH_VAR_FF_AUX_BYTES],
                   const uint8_t salt[MIRATH_PARAM_SALT_BYTES]);

void compute_polynomial_proof(ff_mu_t base_alpha[MIRATH_PARAM_RHO], ff_mu_t mid_alpha[MIRATH_PARAM_RHO],
                              const ff_t S[MIRATH_VAR_FF_S_BYTES], const ff_mu_t S_rnd[MIRATH_VAR_S],
                              const ff_t C[MIRATH_VAR_FF_C_BYTES], const ff_mu_t C_rnd[MIRATH_VAR_C],
                              const ff_mu_t v[MIRATH_PARAM_RHO], ff_mu_t rnd_v[MIRATH_PARAM_RHO],
                              const ff_mu_t gamma[MIRATH_VAR_GAMMA], const ff_t H[MIRATH_VAR_FF_H_BYTES]);

void recompute_polynomial_proof(ff_mu_t base_alpha[MIRATH_PARAM_RHO], ff_mu_t p,
                     const ff_mu_t S_share[MIRATH_VAR_S], const ff_mu_t C_share[MIRATH_VAR_C],
                     const ff_mu_t v_share[MIRATH_PARAM_RHO], const ff_mu_t gamma[MIRATH_VAR_GAMMA],
                     const ff_t H[MIRATH_VAR_FF_H_BYTES], const ff_t y[MIRATH_VAR_FF_Y_BYTES],
                     const ff_mu_t mid_alpha[MIRATH_PARAM_RHO]);

void mirath_tcith_expand_view_challenge(mirath_tcith_view_challenge_t challenge, uint8_t *v_grinding, const uint8_t *string_input);
#define SHAKE_STEP 1

#endif //MIRATH_MIRATH_TCITH_H
