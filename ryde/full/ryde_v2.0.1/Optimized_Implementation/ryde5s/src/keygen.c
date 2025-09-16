/** 
 * @file keygen.c
 * @brief Implementation of the key generation (keygen) of RYDE scheme
 */

#include <stdlib.h>
#include <string.h>
#include "rng.h"
#include "parameters.h"
#include "parsing.h"
#include "ryde.h"


/**
 * \fn int ryde_keygen(uint8_t* pk, uint8_t* sk)
 * \brief Keygen of RYDE TCitH scheme
 *
 * The public key is composed of the syndrome <b>s</b> as well as the seed used to generate vectors <b>g</b> and <b>h</b>.
 *
 * The secret key is composed of the seed used to generate the vectors <b>x</b> and <b>y</b>.
 * As a technicality, the public key is appended to the secret key in order to respect the NIST API.
 *
 * \param[out] pk String containing the public key
 * \param[out] sk String containing the secret key
 * \return EXIT_SUCCESS if verify is successful. Otherwise, it returns EXIT_FAILURE
 */
int ryde_keygen(uint8_t* pk, uint8_t* sk) {

    uint8_t sk_seed[RYDE_SECURITY_BYTES] = {0};
    uint8_t pk_seed[RYDE_SECURITY_BYTES] = {0};

    seedexpander_shake_t sk_seedexpander;
    seedexpander_shake_t pk_seedexpander;

    field_vector_t support[RYDE_PARAM_R] = {0};
    field_matrix_t H[RYDE_PARAM_N - RYDE_PARAM_K] = {0};
    field_vector_t s[RYDE_PARAM_R] = {0},
                       c[RYDE_PARAM_N - RYDE_PARAM_R] = {0},
                       x1[RYDE_PARAM_N - RYDE_PARAM_K] = {0},
                       x2[RYDE_PARAM_K] = {0},
                       x[RYDE_PARAM_N] = {0},
                       y[RYDE_PARAM_N - RYDE_PARAM_K] = {0};
    binary_matrix_t C[RYDE_PARAM_R] = {0};

    // Create seed expanders for public key and secret key
    if (randombytes(sk_seed, RYDE_SECURITY_BYTES) != EXIT_SUCCESS) {
        memset(sk_seed, 0, RYDE_SECURITY_BYTES);
        return EXIT_FAILURE;
    }
    if (randombytes(pk_seed, RYDE_SECURITY_BYTES) != EXIT_SUCCESS) {
        memset(sk_seed, 0, RYDE_SECURITY_BYTES);
        memset(pk_seed, 0, RYDE_SECURITY_BYTES);
        return EXIT_FAILURE;
    }

    seedexpander_shake_init(&sk_seedexpander, sk_seed, RYDE_SECURITY_BYTES, NULL, 0);
    seedexpander_shake_init(&pk_seedexpander, pk_seed, RYDE_SECURITY_BYTES, NULL, 0);

    // Compute secret key
    // Compute first part vector s of the secret key
    field_vector_random_full_rank_with_one(&sk_seedexpander, support, RYDE_PARAM_R);
    binary_matrix_random(&sk_seedexpander, C, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
    // Calculate last part vector c of the secret key as sC
    for(size_t i = 1; i < RYDE_PARAM_R; i++) {
        field_copy(s[i], support[i - 1]);
    }
    field_copy(s[0], support[RYDE_PARAM_R - 1]);
    binary_matrix_mul_by_vector_left(c, C, s, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
    // Set x as (s | c)
    for(size_t i = 0; i < RYDE_PARAM_R; i++) {
        field_copy(x[i], s[i]);
    }
    for(size_t i = RYDE_PARAM_R; i < RYDE_PARAM_N; i++) {
        field_copy(x[i], c[i - RYDE_PARAM_R]);
    }
    // Split x as (x1 | x2)
    for(size_t i = 0; i < RYDE_PARAM_N - RYDE_PARAM_K; i++) {
        field_copy(x1[i], x[i]);
    }
    for(size_t i = 0; i < RYDE_PARAM_K; i++) {
        field_copy(x2[i], x[i + RYDE_PARAM_N - RYDE_PARAM_K]);
    }
    // Compute public key
    field_matrix_random(&pk_seedexpander, H, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);
    field_matrix_mul_by_vector_right(y, H, x2, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);
    field_vector_add(y, y, x1, RYDE_PARAM_N - RYDE_PARAM_K);

    // Parse keys to string
    ryde_public_key_to_string(pk, pk_seed, y);
    ryde_secret_key_to_string(sk, sk_seed, pk_seed);

    #ifdef VERBOSE
        printf("\n\n\n### KEYGEN ###");

        printf("\n\nsk_seed: "); for(int i = 0 ; i < RYDE_SECURITY_BYTES ; ++i) printf("%02X", sk_seed[i]);
        printf("\npk_seed: ");   for(int i = 0 ; i < RYDE_SECURITY_BYTES ; ++i) printf("%02X", pk_seed[i]);
        printf("\nsk: "); for(int i = 0 ; i < RYDE_SECRET_KEY_BYTES ; ++i) printf("%02X", sk[i]);
        printf("\npk: "); for(int i = 0 ; i < RYDE_PUBLIC_KEY_BYTES ; ++i) printf("%02X", pk[i]);

        printf("\nx: (s | sC)");
        uint8_t s_string[RYDE_VEC_R_BYTES];
        field_vector_to_string(s_string, s, RYDE_PARAM_R);
        printf("\n    - s   : "); for(size_t i = 0 ; i < RYDE_VEC_R_BYTES ; i++) { printf("%02X", s_string[i]); }
        memset(s_string, 0, RYDE_VEC_R_BYTES);
        uint8_t C_string[RYDE_MAT_FQ_BYTES] = {0};
        binary_matrix_to_string(C_string, C, RYDE_PARAM_R, RYDE_PARAM_N - RYDE_PARAM_R);
        printf("\n    - C   : "); for(size_t i = 0 ; i < RYDE_MAT_FQ_BYTES ; i++) { printf("%02X", C_string[i]); }
        memset(C_string, 0, RYDE_MAT_FQ_BYTES);

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
        printf("\n");
    #endif

    return EXIT_SUCCESS;
}
