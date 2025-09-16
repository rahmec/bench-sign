/**
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **/

#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "codes.h"
#include "macro.h"
#include "transpose.h"
#include "fq_arith.h"
#include "parameters.h"
#include <assert.h>

// Select low 8-bit, skip the high 8-bit in 16 bit type
const uint8_t shuff_low_half[32] = {
        0x0, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x0, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};


/// swap N uint8 in r and s.
/// \param r[in/out]
/// \param s[in/out]
/*
void swap_rows_old(FQ_ELEM r[N],
               FQ_ELEM s[N]) {
    FQ_ELEM tmp[N];
    memcpy(tmp, r, sizeof(FQ_ELEM) * N);
    memcpy(r, s, sizeof(FQ_ELEM) * N);
    memcpy(s, tmp, sizeof(FQ_ELEM) * N);
}*/ /* end swap_rows */

void swap_rows(FQ_ELEM r[N_pad], 
               FQ_ELEM s[N_pad]){
    vec256_t a, b;
    for(uint32_t i=0; i<N_pad; i+=32) {
         vload256(a, (const vec256_t *)(r + i));
         vload256(b, (const vec256_t *)(s + i));
         vstore256((vec256_t *)(r + i), b);
         vstore256((vec256_t *)(s + i), a);
    }
} /* end swap_rows */


/* Calculate pivot flag array */
void generator_get_pivot_flags(const rref_generator_mat_t *const G,
                               uint8_t pivot_flag [N]) {
    for (uint32_t i = 0; i < N; i = i + 1) {
        pivot_flag[i] = 1;
    }

    for (uint32_t i = 0; i < K; i = i + 1) {
        pivot_flag[G->column_pos[i]] = 0;
    }
}

int generator_RREF(generator_mat_t *G, uint8_t is_pivot_column[N_pad]) {
    int i, j, pivc;
    uint8_t tmp, sc;

    vec256_t *gm[K] __attribute__((aligned(32)));
    vec256_t em[0x80][NW];
    vec256_t *ep[0x80];
    vec256_t c01, c7f;
    vec256_t x, t, *rp, *rg;

    vset8(c01, 0x01);
    vset8(c7f, 0x7f);

    for (i = 0; i < K; i++) {
        gm[i] = (vec256_t *) G->values[i];
    }

    for (i = 0; i < K; i++) {
        j = i;
        /*start by searching the pivot in the col = row*/
        pivc = i;

        while (pivc < N) {
            while (j < K) {
                sc = G->values[j][pivc];
                if (sc != 0) {
                    goto found;
                }

                j++;
            }
            pivc++;     /* move to next col */
            j = i;      /*starting from row to red */
        }

        if (pivc >= N) {
            /* no pivot candidates left, report failure */
            return 0;
        }

        found:
        is_pivot_column[pivc] = 1; /* pivot found, mark the column*/

        /* if we found the pivot on a row which has an index > pivot_column
         * we need to swap the rows */
        if (i != j) {
            for (uint32_t k = 0; k < NW; k++) {
                t = gm[i][k];
                gm[i][k] = gm[j][k];
                gm[j][k] = t;
            }
        }

        /* Compute rescaling factor */
        /* rescale pivot row to have pivot = 1. Values at the left of the pivot
         * are already set to zero by previous iterations */

        //	generate the em matrix
        rg = gm[i];
        memcpy(em[1], rg, LESS_WSZ * NW);

        for (j = 2; j < 127; j++) {
            for (uint32_t k = 0; k < NW; k++) {
                vadd8(x, em[j - 1][k], rg[k])
                W_RED127_(x);
                em[j][k] = x;
            }
        }

        //	shuffle the pointers into ep
        sc = ((uint8_t *) rg)[pivc];
        ep[0] = em[0];
        tmp = sc;
        for (j = 1; j < 127; j++) {
            ep[tmp] = em[j];
            tmp += sc;
            tmp = (tmp + (tmp >> 7)) & 0x7F;
        }
        ep[0x7F] = em[0];

        //	copy back the normalized one
        memcpy(rg, ep[1], LESS_WSZ * NW);

        /* Subtract the now placed and reduced pivot rows, from the others,
         * after rescaling it */
        for (j = 0; j < K; j++) {
            sc = ((uint8_t *) gm[j])[pivc] & 0x7F;
            if (j != i && (sc != 0x00)) {
                rp = ep[127 - sc];
                for (uint32_t k = 0; k < NW; k++) {
                    vadd8(x, gm[j][k], rp[k])
                    W_RED127_(x);
                    gm[j][k] = x;
                }
            }
        }

    }

    return 1;
} /* end generator_RREF */

/// \param G[in/out]: generator matrix K \times N
/// \param is_pivot_column[out]: N bytes, set to 1 if this column
///                 is a pivot column
/// \param was_pivot_column[out]: N bytes, set to 1 if this column
///                 is a pivot column
/// \param pvt_reuse_limit:[in]:
/// \return 0 on failure
///         1 on success
int generator_RREF_pivot_reuse(generator_mat_t *G,
                               uint8_t is_pivot_column[N],
                               uint8_t was_pivot_column[N],
                               const int pvt_reuse_limit) {
   int pvt_reuse_cnt = 0;

    // row swap pre-process - swap previous pivot elements to corresponding row to reduce likelihood of corruption
    if (pvt_reuse_limit != 0) {
        for (int preproc_col = K - 1; preproc_col >= 0; preproc_col--) {
            if (was_pivot_column[preproc_col] == 1) {
                // find pivot row
                uint32_t pivot_el_row = -1;
                for (uint32_t row = 0; row < K; row = row + 1) {
                    if (G->values[row][preproc_col] != 0) {
                        pivot_el_row = row;
                    }
                }
                swap_rows(G->values[preproc_col], G->values[pivot_el_row]);
            }
        }
    }

    for (uint32_t row_to_reduce = 0; row_to_reduce < K; row_to_reduce++) {
        uint32_t pivot_row = row_to_reduce;
        /*start by searching the pivot in the col = row*/
        uint32_t pivot_column = row_to_reduce;
        while ((pivot_column < N) && (G->values[pivot_row][pivot_column] == 0)) {
            while ((pivot_row < K) && (G->values[pivot_row][pivot_column] == 0)) {
                pivot_row++;
            }
            if (pivot_row >= K) { /*entire column tail swept*/
                pivot_column++; /* move to next col */
                pivot_row = row_to_reduce; /*starting from row to red */
            }
        }
        if (pivot_column >= N) {
            return 0; /* no pivot candidates left, report failure */
        }
        is_pivot_column[pivot_column] = 1; /* pivot found, mark the column*/

        /* if we found the pivot on a row which has an index > pivot_column
         * we need to swap the rows */
        if (row_to_reduce != pivot_row) {
            was_pivot_column[pivot_row] = 0; // pivot no longer reusable - will be corrupted during reduce row
            swap_rows(G->values[row_to_reduce], G->values[pivot_row]);
        }
        pivot_row = row_to_reduce; /* row with pivot now in place */

        /// NOTE: this needs explenation. We can skip the reduction of the pivot row, because for
        /// the CF it doesnt matter. The only thing that is important for the CF is the number of
        /// zeros, and this doest change if we reduce a reused pivot row.
        if (((was_pivot_column[pivot_column] == 1) && (pvt_reuse_cnt < pvt_reuse_limit) && (pivot_column < K))) {
            pvt_reuse_cnt++;
            continue;
        }

        /* Compute rescaling factor */
        const FQ_ELEM scaling_factor = fq_inv(G->values[pivot_row][pivot_column]);

        /* rescale pivot row to have pivot = 1. Values at the left of the pivot
         * are already set to zero by previous iterations */
        for (uint32_t i = pivot_column; i < N; i++) {
            G->values[pivot_row][i] = fq_mul(scaling_factor, G->values[pivot_row][i]);
        }

        /* Subtract the now placed and reduced pivot rows, from the others,
         * after rescaling it */
        for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
            if (row_idx != pivot_row) {
                FQ_ELEM multiplier = G->values[row_idx][pivot_column];
                /* all elements before the pivot in the pivot row are null, no need to
                 * subtract them from other rows. */
                for (int col_idx = 0; col_idx < N; col_idx++) {
                    FQ_ELEM tmp = fq_mul(multiplier, G->values[pivot_row][col_idx]);
                    G->values[row_idx][col_idx] = fq_sub(G->values[row_idx][col_idx], tmp);
                }
            }
        }
    }

    return 1;
} /* end generator_RREF_pivot_reuse */

/* Compresses a generator matrix in RREF storing only non-pivot columns and
 * their position */
void generator_rref_compact(rref_generator_mat_t *compact,
                            const generator_mat_t *const full,
                            const uint8_t is_pivot_column[N]) {
    int dst_col_idx = 0;
    for (uint32_t src_col_idx = 0; src_col_idx < N; src_col_idx++) {
        if (!is_pivot_column[src_col_idx]) {
            for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
                compact->values[row_idx][dst_col_idx] = full->values[row_idx][src_col_idx];
            }
            compact->column_pos[dst_col_idx] = src_col_idx;
            dst_col_idx++;
        }
    }
} /* end generator_rref_compact */


void generator_rref_compact_speck(FQ_ELEM compact[K][K_pad],
                            const generator_mat_t *const full,
                            const uint8_t is_pivot_column[N]) {
    int dst_col_idx = 0;
    for (uint32_t src_col_idx = 0; src_col_idx < N; src_col_idx++) {
        if (!is_pivot_column[src_col_idx]) {
            for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
                compact[row_idx][dst_col_idx] = full->values[row_idx][src_col_idx];
            }
            dst_col_idx++;
        }
    }
} /* end generator_rref_compact */

/* Compresses a generator matrix in RREF into an array of bytes */
void compress_rref(uint8_t *compressed,
                   const generator_mat_t *const full,
                   const uint8_t is_pivot_column[N]) {
    // Compress pivot flags
    for (uint32_t col_byte = 0; col_byte < N / 8; col_byte++) {
        compressed[col_byte] = is_pivot_column[8 * col_byte + 0] |
                               (is_pivot_column[8 * col_byte + 1] << 1) |
                               (is_pivot_column[8 * col_byte + 2] << 2) |
                               (is_pivot_column[8 * col_byte + 3] << 3) |
                               (is_pivot_column[8 * col_byte + 4] << 4) |
                               (is_pivot_column[8 * col_byte + 5] << 5) |
                               (is_pivot_column[8 * col_byte + 6] << 6) |
                               (is_pivot_column[8 * col_byte + 7] << 7);
    }

#if (CATEGORY == 252) || (CATEGORY == 548)
    // Compress last flags
    compressed[N / 8] = is_pivot_column[N - 4] | (is_pivot_column[N - 3] << 1) |
                        (is_pivot_column[N - 2] << 2) |
                        (is_pivot_column[N - 1] << 3);

    int compress_idx = N / 8 + 1;
#else
    int compress_idx = N / 8;
#endif

    // Compress non-pivot columns row-by-row
    int encode_state = 0;
    for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
        for (uint32_t col_idx = 0; col_idx < N; col_idx++) {
            if (!is_pivot_column[col_idx]) {
                switch (encode_state) {
                    case 0:
                        compressed[compress_idx] = full->values[row_idx][col_idx];
                        break;
                    case 1:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 7);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 1);
                        break;
                    case 2:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 6);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 2);
                        break;
                    case 3:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 5);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 3);
                        break;
                    case 4:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 4);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 4);
                        break;
                    case 5:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 3);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 5);
                        break;
                    case 6:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 2);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 6);
                        break;
                    case 7:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 1);
                        compress_idx++;
                        break;
                }

                if (encode_state != 7) {
                    encode_state++;
                } else {
                    encode_state = 0;
                }
            }
        }
    } /* end compress_rref */
}

/* Expands a compressed RREF generator matrix into a full one */
void expand_to_rref(generator_mat_t *full,
                    const uint8_t *compressed,
                    uint8_t is_pivot_column[N]) {
    // Decompress pivot flags
    for (int i = 0; i < N; i++) {
        is_pivot_column[i] = 0;
    }

    for (int col_byte = 0; col_byte < N / 8; col_byte++) {
        is_pivot_column[col_byte * 8 + 0] = compressed[col_byte] & 0x1;
        is_pivot_column[col_byte * 8 + 1] = (compressed[col_byte] >> 1) & 0x1;
        is_pivot_column[col_byte * 8 + 2] = (compressed[col_byte] >> 2) & 0x1;
        is_pivot_column[col_byte * 8 + 3] = (compressed[col_byte] >> 3) & 0x1;
        is_pivot_column[col_byte * 8 + 4] = (compressed[col_byte] >> 4) & 0x1;
        is_pivot_column[col_byte * 8 + 5] = (compressed[col_byte] >> 5) & 0x1;
        is_pivot_column[col_byte * 8 + 6] = (compressed[col_byte] >> 6) & 0x1;
        is_pivot_column[col_byte * 8 + 7] = (compressed[col_byte] >> 7) & 0x1;
    }

#if (CATEGORY == 252) || (CATEGORY == 548)
    // Decompress last flags
    is_pivot_column[N - 4] = compressed[N / 8] & 0x1;
    is_pivot_column[N - 3] = (compressed[N / 8] >> 1) & 0x1;
    is_pivot_column[N - 2] = (compressed[N / 8] >> 2) & 0x1;
    is_pivot_column[N - 1] = (compressed[N / 8] >> 3) & 0x1;

    int compress_idx = N / 8 + 1;
#else
    int compress_idx = N / 8;
#endif

    // Decompress columns row-by-row
    int decode_state = 0;
    for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
        int pivot_idx = 0;
        for (uint32_t col_idx = 0; col_idx < N; col_idx++) {
            if (!is_pivot_column[col_idx]) {
                // Decompress non-pivot
                switch (decode_state) {
                    case 0:
                        full->values[row_idx][col_idx] = compressed[compress_idx] & MASK_Q;
                        break;
                    case 1:
                        full->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 7) |
                                 (compressed[compress_idx + 1] << 1)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 2:
                        full->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 6) |
                                 (compressed[compress_idx + 1] << 2)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 3:
                        full->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 5) |
                                 (compressed[compress_idx + 1] << 3)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 4:
                        full->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 4) |
                                 (compressed[compress_idx + 1] << 4)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 5:
                        full->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 3) |
                                 (compressed[compress_idx + 1] << 5)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 6:
                        full->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 2) |
                                 (compressed[compress_idx + 1] << 6)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 7:
                        full->values[row_idx][col_idx] =
                                (compressed[compress_idx] >> 1) & MASK_Q;
                        compress_idx++;
                        break;
                }

                if (decode_state != 7) {
                    decode_state++;
                } else {
                    decode_state = 0;
                }
            } else {
                // Decompress pivot
                full->values[row_idx][col_idx] = ((uint32_t)row_idx == (uint32_t)pivot_idx);
                pivot_idx++;
            }
        }
    }

} /* end expand_to_rref */


/* Expands a compressed RREF generator matrix into a full one */
void generator_rref_expand(generator_mat_t *full,
                           const rref_generator_mat_t *const compact) {
    int placed_dense_cols = 0;
    for (uint32_t col_idx = 0; col_idx < N; col_idx++) {
        if ((placed_dense_cols < N - K) && (col_idx == compact->column_pos[placed_dense_cols])) {
            /* non-pivot column, restore one full column */
            for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
                full->values[row_idx][col_idx] = compact->values[row_idx][placed_dense_cols];
            }
            placed_dense_cols++;
        } else {
            /* regenerate the appropriate pivot column */
            for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
                full->values[row_idx][col_idx] = (row_idx == col_idx - placed_dense_cols);

            }
        }
    }
} /* end generator_rref_expand */

/// \param V[in/out]: K \times N-K matrix in which row `row1` and
///                 row `row2` are swapped
/// \param row1[in]: first row
/// \param row2[in]: second row
void normalized_row_swap(normalized_IS_t *V,
                         const POSITION_T row1,
                         const POSITION_T row2) {
    if (row1 == row2) { return; }
    for(uint32_t i = 0; i < N-K; i++){
        POSITION_T tmp = V->values[row1][i];
        V->values[row1][i] = V->values[row2][i];
        V->values[row2][i] = tmp;
    }
}

/// \param res[out]: full rank generator matrix K \times N-K
/// \param seed[int] seed for the prng
void generator_sample(rref_generator_mat_t *res, const unsigned char seed[SEED_LENGTH_BYTES]) {
    antiorthogonal_sample(res->values,seed);
    for (uint32_t i = 0; i < N - K; i++) {
        res->column_pos[i] = i + K;
    }
} /* end generator_seed_expand */

void row_mat_mult_old(FQ_ELEM *out,
                    const FQ_ELEM *row,
                    FQ_ELEM M[K][K_pad],
                    uint8_t r,
                    uint8_t c){

    for(uint8_t i=0; i<c; i++){
        out[i] = 0;
        for(uint8_t j=0; j<r; j++){
            out[i] = fq_add(out[i],fq_mul(row[j],M[j][i])); 
        }
    }

}

/*
void row_mat_mult(FQ_ELEM *out,
                    const FQ_ELEM *vec,
                    FQ_ELEM M[K][K_pad],
                    uint8_t r,
                    uint8_t c){

    vec256_t shuffle, t, c7f, c01, b, a, a_lo, a_hi, b_lo, b_hi;
    vec128_t tmp;
    vec256_t res;
    vset8(c7f, 127);
    vset8(c01, 1);

    for (uint32_t col = 0; (col+32) <= NEXT_MULTIPLE(c, 32); col+=32) {
        // precompute 
        
        vset8(res, 0);
        vload256(shuffle, (vec256_t *) shuff_low_half);
        //vset8(c7f, 127);
        //vset8(c01, 1);

        for (uint32_t row = 0; row < r; row+=1){
            // precompute b
            vset8(b, vec[row]);
            vget_lo(tmp, b);
            vextend8_16(b_lo, tmp);
            vget_hi(tmp, b);
            vextend8_16(b_hi, tmp);

            vload256(a, (vec256_t *)(M[row]+col));

            vget_lo(tmp, a);
            vextend8_16(a_lo, tmp);
            vget_hi(tmp, a);
            vextend8_16(a_hi, tmp);

            barrett_mul_u16(a_lo, a_lo, b_lo, t);
            barrett_mul_u16(a_hi, a_hi, b_hi, t);

            vshuffle8(a_lo, a_lo, shuffle);
            vshuffle8(a_hi, a_hi, shuffle);

            vpermute_4x64(a_lo, a_lo, 0xd8);
            vpermute_4x64(a_hi, a_hi, 0xd8);

            vpermute2(t, a_lo, a_hi, 0x20);

            // barrett_red8(t, r, c7f, c01);
            W_RED127_(t);
            vadd8(res,res,t)
            W_RED127_(res);
        }

        vstore256((vec256_t *)(out + col), res);
    }

}
*/


void swap_columns(FQ_ELEM M[K][K_pad], uint8_t c1, uint8_t c2, uint8_t r){
    FQ_ELEM tmp;
    for(uint8_t i=0; i<r; i++){
        tmp = M[i][c1];
        M[i][c1] = M[i][c2];
        M[i][c2] = tmp;
    }
}

/// \param res[out]: full rank generator matrix K \times K
/// \param seed[int] seed for the prng
void antiorthogonal_sample(FQ_ELEM A[K][K_pad], const unsigned char seed[SEED_LENGTH_BYTES]) {
    SHAKE_STATE_STRUCT csprng_state;
    initialize_csprng(&csprng_state, seed, SEED_LENGTH_BYTES);
    FQ_ELEM G[K][K_pad];
    FQ_ELEM c[K_pad] = {0};

    while(c[0] == 0 || !(anti_normalize(c))){
        rand_range_q_state_elements(&csprng_state, c, K);
    }

    set_row(A[0],c);
    row_mul(c,fq_inv(c[0]));
    set_row(G[0],c);

    FQ_ELEM M[K][K_pad] = {0};
    uint8_t kp = 1;

    while (kp < K) {

        for(uint8_t i=0; i<K-kp; i++){
            for(uint8_t j=0; j<kp; j++){
                M[i][j] = fq_sub(0,G[j][i+kp]);
            }
        }

        FQ_ELEM u[K-kp];

        do{
            rand_range_q_state_elements(&csprng_state, u, K-kp);
            row_mat_mult(c,u,M,K-kp,kp);
            memcpy(c+kp, u, sizeof(FQ_ELEM)*(K-kp));
        }while(!anti_normalize(c));

        set_row(A[kp],c);
        set_row(G[kp],c);
        for(uint8_t i=0; i<kp; i++){
            if(G[kp][i] != 0){
                row_sum(G[kp],G[i],fq_sub(0,G[kp][i]),K);
            }
        }

        if(G[kp][kp] == 0){
            for(uint8_t i=kp+1; i<K; i++){
                if(G[kp][i] != 0){
                    swap_columns(G,kp,i,kp+1);
                    swap_columns(A,kp,i,kp+1);
                    break;
                }
            }
        }
        
        row_mul(G[kp],fq_inv(G[kp][kp]));

        for(int i=0; i<kp; i++){
            if(G[i][kp] != 0){
                row_sum(G[i], G[kp], fq_sub(0,G[i][kp]), K);
            }
        }

        kp += 1;
    }
}

/* permutes generator columns */
void permute_generator(generator_mat_t *res,
                        const generator_mat_t *const G,
                        const permutation_t *const perm) {
   for(uint32_t col_idx = 0; col_idx < N; col_idx++) {
      for(uint32_t row_idx = 0; row_idx < K; row_idx++) {
         res->values[row_idx][col_idx] = G->values[row_idx][perm->values[col_idx]];
      }
   }
} 

/* permutes a codeword */
void permute_codeword(FQ_ELEM res[N],
                        const FQ_ELEM to_perm[N],
                        const permutation_t *const perm) {
   for(uint32_t col_idx = 0; col_idx < N; col_idx++) {
         res[col_idx] = to_perm[perm->values[col_idx]];
    }
} 

/* seems constant time */
void generate_rref_perm(permutation_t *res,
                        uint8_t is_pivot_column[N]){
        POSITION_T ctr_is = 0;
        POSITION_T ctr_non_is = 0;
        for(uint8_t i=0; i<N; i++){
            if(is_pivot_column[i] == 1){
               res->values[ctr_is] = i; 
               ctr_is++;
            } else {
               res->values[K+ctr_non_is] = i; 
               ctr_non_is++;
            }
        }
}

/* Compresses a generator matrix in RREF into an array of bytes */
void compress_rref_speck(uint8_t *compressed,
                   const generator_mat_t *const full,
                   const uint8_t is_pivot_column[N]) {
    int compress_idx = 0;

    // Compress non-pivot columns row-by-row
    int encode_state = 0;
    for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
        for (uint32_t col_idx = 0; col_idx < N; col_idx++) {
            if (!is_pivot_column[col_idx]) {
                switch (encode_state) {
                    case 0:
                        compressed[compress_idx] = full->values[row_idx][col_idx];
                        break;
                    case 1:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 7);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 1);
                        break;
                    case 2:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 6);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 2);
                        break;
                    case 3:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 5);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 3);
                        break;
                    case 4:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 4);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 4);
                        break;
                    case 5:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 3);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 5);
                        break;
                    case 6:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 2);
                        compress_idx++;
                        compressed[compress_idx] = (full->values[row_idx][col_idx] >> 6);
                        break;
                    case 7:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (full->values[row_idx][col_idx] << 1);
                        compress_idx++;
                        break;
                }

                if (encode_state != 7) {
                    encode_state++;
                } else {
                    encode_state = 0;
                }
            }
        }
    } /* end compress_rref */
}

/* Expands a compressed RREF generator matrix into a full one */
void expand_to_rref_speck(rref_generator_mat_t *matr,
                    const uint8_t *compressed) {
    int compress_idx = 0;

    // Decompress columns row-by-row
    int decode_state = 0;
    for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
        for (uint32_t col_idx = 0; col_idx < K; col_idx++) {
                // Decompress non-pivot
                switch (decode_state) {
                    case 0:
                        matr->values[row_idx][col_idx] = compressed[compress_idx] & MASK_Q;
                        break;
                    case 1:
                        matr->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 7) |
                                 (compressed[compress_idx + 1] << 1)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 2:
                        matr->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 6) |
                                 (compressed[compress_idx + 1] << 2)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 3:
                        matr->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 5) |
                                 (compressed[compress_idx + 1] << 3)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 4:
                        matr->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 4) |
                                 (compressed[compress_idx + 1] << 4)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 5:
                        matr->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 3) |
                                 (compressed[compress_idx + 1] << 5)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 6:
                        matr->values[row_idx][col_idx] =
                                ((compressed[compress_idx] >> 2) |
                                 (compressed[compress_idx + 1] << 6)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 7:
                        matr->values[row_idx][col_idx] =
                                (compressed[compress_idx] >> 1) & MASK_Q;
                        compress_idx++;
                        break;
                }

                if (decode_state != 7) {
                    decode_state++;
                } else {
                    decode_state = 0;
                }
        }
    }

} /* end expand_to_rref */

void sample_codeword_rref(FQ_ELEM c[N_pad],const rref_generator_mat_t G){
    rand_range_q_elements(c, K);
    row_mat_mult(c+K,c,G.values,K,K);
    /*
    memcpy(c+K,c1,sizeof(FQ_ELEM)*K);
    */
}

/* Compresses a generator matrix in RREF into an array of bytes */
void compress_rref_speck_non_IS(uint8_t *compressed,
                   const rref_generator_mat_t *const G_rref) {
    int compress_idx = 0;

    // Compress non-pivot columns row-by-row
    int encode_state = 0;
    for (uint32_t row_idx = 0; row_idx < K; row_idx++) {
        for (uint32_t col_idx = 0; col_idx < K; col_idx++) {
                switch (encode_state) {
                    case 0:
                        compressed[compress_idx] = G_rref->values[row_idx][col_idx];
                        break;
                    case 1:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (G_rref->values[row_idx][col_idx] << 7);
                        compress_idx++;
                        compressed[compress_idx] = (G_rref->values[row_idx][col_idx] >> 1);
                        break;
                    case 2:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (G_rref->values[row_idx][col_idx] << 6);
                        compress_idx++;
                        compressed[compress_idx] = (G_rref->values[row_idx][col_idx] >> 2);
                        break;
                    case 3:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (G_rref->values[row_idx][col_idx] << 5);
                        compress_idx++;
                        compressed[compress_idx] = (G_rref->values[row_idx][col_idx] >> 3);
                        break;
                    case 4:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (G_rref->values[row_idx][col_idx] << 4);
                        compress_idx++;
                        compressed[compress_idx] = (G_rref->values[row_idx][col_idx] >> 4);
                        break;
                    case 5:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (G_rref->values[row_idx][col_idx] << 3);
                        compress_idx++;
                        compressed[compress_idx] = (G_rref->values[row_idx][col_idx] >> 5);
                        break;
                    case 6:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (G_rref->values[row_idx][col_idx] << 2);
                        compress_idx++;
                        compressed[compress_idx] = (G_rref->values[row_idx][col_idx] >> 6);
                        break;
                    case 7:
                        compressed[compress_idx] =
                                compressed[compress_idx] | (G_rref->values[row_idx][col_idx] << 1);
                        compress_idx++;
                        break;
                }

                if (encode_state != 7) {
                    encode_state++;
                } else {
                    encode_state = 0;
                }
        }
    } /* end compress_rref */
}

/* Compresses a generator matrix in RREF into an array of bytes */
void compress_c1s(uint8_t *compressed_c1s,
                   FQ_ELEM c1s[W][K_pad]) {
    int compress_idx = 0;

    // Compress non-pivot columns row-by-row
    int encode_state = 0;
    for (uint32_t row_idx = 0; row_idx < W; row_idx++) {
        for (uint32_t chal1_index = 0; chal1_index < K; chal1_index++) {
                switch (encode_state) {
                    case 0:
                        compressed_c1s[compress_idx] = c1s[row_idx][chal1_index];
                        break;
                    case 1:
                        compressed_c1s[compress_idx] =
                                compressed_c1s[compress_idx] | (c1s[row_idx][chal1_index] << 7);
                        compress_idx++;
                        compressed_c1s[compress_idx] = (c1s[row_idx][chal1_index] >> 1);
                        break;
                    case 2:
                        compressed_c1s[compress_idx] =
                                compressed_c1s[compress_idx] | (c1s[row_idx][chal1_index] << 6);
                        compress_idx++;
                        compressed_c1s[compress_idx] = (c1s[row_idx][chal1_index] >> 2);
                        break;
                    case 3:
                        compressed_c1s[compress_idx] =
                                compressed_c1s[compress_idx] | (c1s[row_idx][chal1_index] << 5);
                        compress_idx++;
                        compressed_c1s[compress_idx] = (c1s[row_idx][chal1_index] >> 3);
                        break;
                    case 4:
                        compressed_c1s[compress_idx] =
                                compressed_c1s[compress_idx] | (c1s[row_idx][chal1_index] << 4);
                        compress_idx++;
                        compressed_c1s[compress_idx] = (c1s[row_idx][chal1_index] >> 4);
                        break;
                    case 5:
                        compressed_c1s[compress_idx] =
                                compressed_c1s[compress_idx] | (c1s[row_idx][chal1_index] << 3);
                        compress_idx++;
                        compressed_c1s[compress_idx] = (c1s[row_idx][chal1_index] >> 5);
                        break;
                    case 6:
                        compressed_c1s[compress_idx] =
                                compressed_c1s[compress_idx] | (c1s[row_idx][chal1_index] << 2);
                        compress_idx++;
                        compressed_c1s[compress_idx] = (c1s[row_idx][chal1_index] >> 6);
                        break;
                    case 7:
                        compressed_c1s[compress_idx] =
                                compressed_c1s[compress_idx] | (c1s[row_idx][chal1_index] << 1);
                        compress_idx++;
                        break;
                }

                if (encode_state != 7) {
                    encode_state++;
                } else {
                    encode_state = 0;
                }
        }
    } /* end compress_rref */
}

/* Expands a compressed RREF generator matrix into a full one */
void expand_c1s(FQ_ELEM c1s[W][K_pad],
                const uint8_t *compressed_c1s){
    int compress_idx = 0;

    // Decompress columns row-by-row
    int decode_state = 0;
    for (uint32_t row_idx = 0; row_idx < W; row_idx++) {
        for (uint32_t chal1_index = 0; chal1_index < K; chal1_index++) {
                // Decompress non-pivot
                switch (decode_state) {
                    case 0:
                        c1s[row_idx][chal1_index] = compressed_c1s[compress_idx] & MASK_Q;
                        break;
                    case 1:
                        c1s[row_idx][chal1_index] =
                                ((compressed_c1s[compress_idx] >> 7) |
                                 (compressed_c1s[compress_idx + 1] << 1)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 2:
                        c1s[row_idx][chal1_index] =
                                ((compressed_c1s[compress_idx] >> 6) |
                                 (compressed_c1s[compress_idx + 1] << 2)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 3:
                        c1s[row_idx][chal1_index] =
                                ((compressed_c1s[compress_idx] >> 5) |
                                 (compressed_c1s[compress_idx + 1] << 3)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 4:
                        c1s[row_idx][chal1_index] =
                                ((compressed_c1s[compress_idx] >> 4) |
                                 (compressed_c1s[compress_idx + 1] << 4)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 5:
                        c1s[row_idx][chal1_index] =
                                ((compressed_c1s[compress_idx] >> 3) |
                                 (compressed_c1s[compress_idx + 1] << 5)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 6:
                        c1s[row_idx][chal1_index] =
                                ((compressed_c1s[compress_idx] >> 2) |
                                 (compressed_c1s[compress_idx + 1] << 6)) &
                                MASK_Q;
                        compress_idx++;
                        break;
                    case 7:
                        c1s[row_idx][chal1_index] =
                                (compressed_c1s[compress_idx] >> 1) & MASK_Q;
                        compress_idx++;
                        break;
                }

                if (decode_state != 7) {
                    decode_state++;
                } else {
                    decode_state = 0;
                }
        }
    }

} /* end expand_to_rref */
