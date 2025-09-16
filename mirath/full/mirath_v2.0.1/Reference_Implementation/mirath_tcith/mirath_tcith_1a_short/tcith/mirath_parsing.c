#include <string.h>
#include "framework_data_types.h"
#include "mirath_arith.h"
#include "mirath_ggm_tree.h"
#include "hash_sha3.h"

void unparse_public_key(uint8_t *pk, const seed_t seed_pk, const ff_t y[MIRATH_VAR_FF_Y_BYTES]) {
    memcpy(pk, seed_pk, MIRATH_SECURITY_BYTES);

    memcpy(pk + MIRATH_SECURITY_BYTES, y, MIRATH_VAR_FF_Y_BYTES);
}

void parse_public_key(seed_t seed_pk, ff_t y[MIRATH_VAR_FF_Y_BYTES], const uint8_t *pk) {
    memcpy(seed_pk, pk, MIRATH_SECURITY_BYTES);

    memcpy(y, pk + MIRATH_SECURITY_BYTES, MIRATH_VAR_FF_Y_BYTES);
}

void unparse_secret_key(uint8_t *sk, const seed_t seed_sk, const seed_t seed_pk) {
    memcpy(sk, seed_sk, MIRATH_SECURITY_BYTES);

    memcpy(sk + MIRATH_SECURITY_BYTES, seed_pk, MIRATH_SECURITY_BYTES);
}

void parse_secret_key(seed_t seed_sk, seed_t seed_pk, const uint8_t *sk) {
    memcpy(seed_sk, sk, MIRATH_SECURITY_BYTES);
    memcpy(seed_pk, sk + MIRATH_SECURITY_BYTES, MIRATH_SECURITY_BYTES);
}

void unparse_signature(uint8_t *signature, const uint8_t salt[MIRATH_PARAM_SALT_BYTES], const uint64_t ctr,
                       const hash_sha3_ctx *hash2, const mirath_ggm_tree_node_t path[MIRATH_PARAM_MAX_OPEN],
                       const mirath_tcith_commit_t commits_i_star[MIRATH_PARAM_TAU],
                       const ff_t aux[MIRATH_PARAM_TAU][MIRATH_VAR_FF_AUX_BYTES],
                       const ff_mu_t mid_alpha[MIRATH_PARAM_TAU][MIRATH_PARAM_RHO]) {

    uint8_t *ptr;

    ptr = (uint8_t *)signature;

    memcpy(ptr, salt, MIRATH_PARAM_SALT_BYTES);
    ptr += MIRATH_PARAM_SALT_BYTES;

    memcpy(ptr, &ctr, sizeof(uint64_t));
    ptr += sizeof(uint64_t);

    memcpy(ptr, hash2, 2 * MIRATH_SECURITY_BYTES);
    ptr += 2 * MIRATH_SECURITY_BYTES;

    memcpy(ptr, (uint8_t *)path, MIRATH_SECURITY_BYTES * MIRATH_PARAM_T_OPEN);
    ptr += MIRATH_SECURITY_BYTES * MIRATH_PARAM_T_OPEN;

    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        memcpy(ptr, commits_i_star[e], 2 * MIRATH_SECURITY_BYTES);
        ptr += 2 * MIRATH_SECURITY_BYTES;
    }

    // packing tight field elements
    *ptr = 0;
    uint32_t off_ptr = 8;

    uint32_t n_rows_bytes1 = mirath_matrix_ff_bytes_size(MIRATH_PARAM_M, 1);
    uint32_t n_rows_bytes2 = mirath_matrix_ff_bytes_size(MIRATH_PARAM_R, 1);
    uint32_t on_col1 = 8 - ((8 * n_rows_bytes1) - (4 * MIRATH_PARAM_M));
    uint32_t on_col2 = 8 - ((8 * n_rows_bytes2) - (4 * MIRATH_PARAM_R));

    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        uint8_t *col;
        col = (uint8_t *)aux[e];
        for (uint32_t j = 0; j < MIRATH_PARAM_R; j++) {
            *ptr |= (*col << (8 - off_ptr));

            for (uint32_t i = 0; i < n_rows_bytes1-1; i++) {
                ptr += 1;
                *ptr = (*col >> off_ptr);
                col += 1;
                *ptr |= (*col << (8 - off_ptr));
            }

            if (off_ptr <= on_col1) {
                ptr += 1;
                *ptr = (*col >> off_ptr);
            }
            off_ptr = 8 - ((on_col1 - off_ptr) % 8);
            col += 1;
        }

        for (uint32_t j = 0; j < (MIRATH_PARAM_N - MIRATH_PARAM_R); j++) {
            *ptr |= (*col << (8 - off_ptr));

            for (uint32_t i = 0; i < n_rows_bytes2-1; i++) {
                ptr += 1;
                *ptr = (*col >> off_ptr);
                col += 1;
                *ptr |= (*col << (8 - off_ptr));
            }

            if (off_ptr <= on_col2) {
                ptr += 1;
                *ptr = (*col >> off_ptr);
            }
            off_ptr = 8 - ((on_col2 - off_ptr) % 8);
            col += 1;
        }

        uint32_t on_mu = 4;
        ff_mu_t mask_high_mu = (uint16_t)0x0F00;

        for (uint32_t i = 0; i < MIRATH_PARAM_RHO; i++) {
            ff_mu_t entry = mid_alpha[e][i];
            const uint32_t shift = (8-off_ptr);
            uint8_t entry_low = (entry & 0x00FF);
            uint8_t entry_high = (entry & mask_high_mu) >> 8;
            *ptr |= (entry_low << shift);
            ptr += 1;
            *ptr = (entry_low >> off_ptr);
            *ptr |= (entry_high << shift);
            if (off_ptr > on_mu) {
                off_ptr = off_ptr - on_mu;
            } else if (off_ptr == on_mu) {
                ptr += 1;
                *ptr = 0;
                off_ptr = 8;
            } else {
                ptr += 1;
                *ptr = (entry_high >> off_ptr);
                off_ptr = off_ptr + 8 - on_mu;
            }
        }
    }
}

int parse_signature(uint8_t salt[MIRATH_PARAM_SALT_BYTES], uint64_t *ctr, hash_sha3_ctx *hash2,
                     mirath_ggm_tree_node_t path[MIRATH_PARAM_MAX_OPEN], mirath_tcith_commit_t commits_i_star[MIRATH_PARAM_TAU],
                     ff_t aux[MIRATH_PARAM_TAU][MIRATH_VAR_FF_AUX_BYTES],
                     ff_mu_t mid_alpha[MIRATH_PARAM_TAU][MIRATH_PARAM_RHO], const uint8_t *signature) {

    // Below code catches trivial forgery
    uint32_t tmp_bits = ((MIRATH_PARAM_M * MIRATH_PARAM_R) + (MIRATH_PARAM_R * (MIRATH_PARAM_N - MIRATH_PARAM_R)) + (MIRATH_PARAM_RHO  * MIRATH_PARAM_MU)) * MIRATH_PARAM_TAU;
    tmp_bits *= 4;
    if ((tmp_bits % 8) != 0) {
        const uint8_t mask = (1u << (tmp_bits % 8)) - 1;
        if ((signature[MIRATH_SIGNATURE_BYTES - 1] & mask) != signature[MIRATH_SIGNATURE_BYTES - 1]) {
            return 1;
        }
    }

    uint8_t *ptr = (uint8_t *)signature;

    memcpy(salt, ptr, MIRATH_PARAM_SALT_BYTES);
    ptr += MIRATH_PARAM_SALT_BYTES;

    memcpy(ctr, ptr, sizeof(uint64_t));
    ptr += sizeof(uint64_t);

    memcpy(hash2, ptr, 2 * MIRATH_SECURITY_BYTES);
    ptr += 2 * MIRATH_SECURITY_BYTES;

    memcpy((uint8_t *)path, ptr, MIRATH_SECURITY_BYTES * MIRATH_PARAM_T_OPEN);
    ptr += MIRATH_SECURITY_BYTES * MIRATH_PARAM_T_OPEN;

    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        memcpy(commits_i_star[e], ptr, 2 * MIRATH_SECURITY_BYTES);
        ptr += 2 * MIRATH_SECURITY_BYTES;
    }

    // unpacking the field elements
    uint32_t off_ptr = 8;

    uint32_t n_rows_bytes1 = mirath_matrix_ff_bytes_size(MIRATH_PARAM_M, 1);
    uint32_t n_rows_bytes2 = mirath_matrix_ff_bytes_size(MIRATH_PARAM_R, 1);
    uint32_t on_col1 = 8 - ((8 * n_rows_bytes1) - (4 * MIRATH_PARAM_M));
    uint32_t on_col2 = 8 - ((8 * n_rows_bytes2) - (4 * MIRATH_PARAM_R));

    for (uint32_t e = 0; e < MIRATH_PARAM_TAU; e++) {
        uint8_t *col;
        col = (uint8_t *)aux[e];
        for (uint32_t j = 0; j < MIRATH_PARAM_R; j++) {
            *col = (*ptr >> (8 - off_ptr));

            for (uint32_t i = 0; i < n_rows_bytes1-1; i++) {
                ptr += 1;
                *col |= (*ptr << off_ptr);
                col += 1;
                *col = (*ptr >> (8 - off_ptr));
            }

            if (off_ptr <= on_col1) {
                ptr += 1;
                *col |= (*ptr << off_ptr);
            }
            *col &= (0xFF >> (8 - on_col1));
            off_ptr = 8 - ((on_col1 - off_ptr) % 8);
            col += 1;
        }

        for (uint32_t j = 0; j < (MIRATH_PARAM_N - MIRATH_PARAM_R); j++) {
            *col = (*ptr >> (8 - off_ptr));

            for (uint32_t i = 0; i < n_rows_bytes2-1; i++) {
                ptr += 1;
                *col |= (*ptr << off_ptr);
                col += 1;
                *col = (*ptr >> (8 - off_ptr));
            }

            if (off_ptr <= on_col2) {
                ptr += 1;
                *col |= (*ptr << off_ptr);
            }
            *col &= (0xFF >> (8 - on_col2));
            off_ptr = 8 - ((on_col2 - off_ptr) % 8);
            col += 1;
        }

        uint32_t on_mu = 4;
        ff_mu_t mask_high_mu = (uint16_t)0x0F;

        for (uint32_t i = 0; i < MIRATH_PARAM_RHO; i++) {
            ff_mu_t entry = 0;
            uint8_t entry_low = 0;
            uint8_t entry_high = 0 ;
            entry_low = (*ptr >> (8 - off_ptr));
            ptr += 1;
            entry_low |= (*ptr << off_ptr);
            entry_high = (*ptr >> (8 - off_ptr));
            if (off_ptr > on_mu) {
                entry_high &= mask_high_mu;
                off_ptr = off_ptr - on_mu;
            } else if (off_ptr == on_mu) {
                ptr += 1;
                off_ptr = 8;
            } else {
                ptr += 1;
                entry_high |= ((*ptr << off_ptr) & mask_high_mu);
                off_ptr = off_ptr + 8 - on_mu;
            }
            entry = entry_high;
            entry = ((entry << 8) | (entry_low));
            mid_alpha[e][i] = entry;
            mid_alpha[e][i] = entry;
        }
    }

    return 0;
}
