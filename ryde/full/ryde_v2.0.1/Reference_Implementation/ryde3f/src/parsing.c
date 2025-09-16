/** 
 * @file parsing.c
 * @brief Implementation of parsing.h
 */

 #include <string.h>
 #include "parameters.h"
 #include "parsing.h"
 
 
/**
 * \fn void ryde_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const field_vector_t *y)
 * \brief This function parses a public key into a string
 *
 * The public key is composed of the vector <b>y</b> as well as the seed used to generate matrix <b>H</b>.
 *
 * \param[out] pk String containing the public key
 * \param[in] pk_seed Seed used to generate the public key
 * \param[in] y field_vector_t* Representation of vector y
 */
void ryde_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const field_vector_t *y) {
    memcpy(pk, pk_seed, RYDE_SECURITY_BYTES);
    field_vector_to_string(&pk[RYDE_SECURITY_BYTES], y, RYDE_PARAM_N - RYDE_PARAM_K);
}


/**
 * \fn void ryde_public_key_from_string(field_matrix_t *H, field_vector_t *y, const uint8_t* pk)
 * \brief This function parses a public key from a string
 *
 * The public key is composed of the vector <b>y</b> as well as the seed used to generate matrix <b>H</b>.
 *
 * \param[out] H field_matrix_t* Representation of matrix H
 * \param[out] y field_vector_t* Representation of vector y
 * \param[in] pk String containing the public key
 */
void ryde_public_key_from_string(field_matrix_t *H, field_vector_t *y, const uint8_t* pk) {
    uint8_t pk_seed[RYDE_SECURITY_BYTES] = {0};
    seedexpander_shake_t pk_seedexpander;

    // Compute parity-check matrix
    memcpy(pk_seed, pk, RYDE_SECURITY_BYTES);
    seedexpander_shake_init(&pk_seedexpander, pk_seed, RYDE_SECURITY_BYTES, NULL, 0);

    field_matrix_random(&pk_seedexpander, H, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);

    // Compute syndrome
    field_vector_from_string(y, RYDE_PARAM_N - RYDE_PARAM_K, &pk[RYDE_SECURITY_BYTES]);
}


/**
 * \fn void ryde_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk_seed)
 * \brief This function parses a secret key into a string
 *
 * The secret key is composed of the seed used to generate vectors <b>x = (x1,x2)</b> and <b>y</b>.
 * As a technicality, the public key is appended to the secret key in order to respect the NIST API.
 *
 * \param[out] sk String containing the secret key
 * \param[in] sk_seed Seed used to generate the vectors x and y
 * \param[in] pk_seed Seed used to generate the public key
 */
void ryde_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk_seed) {
    memcpy(sk, sk_seed, RYDE_SECURITY_BYTES);
    memcpy(&sk[RYDE_SECURITY_BYTES], pk_seed, RYDE_SECURITY_BYTES);
}

 
/**
 * \fn void ryde_secret_key_from_string(field_vector_t *y, field_matrix_t *H, field_vector_t *s, binary_matrix_t *C, const uint8_t* sk)
 * \brief This function parses a secret key from a string
 *
 * The secret key is composed of the seed used to generate vectors <b>x = (x1,x2)</b> and <b>y</b>.
 * Additionally, it calculates the public matrix <b>H</b> and the annihilator polynomial <b>A</b>.
 *
 * As a technicality, the public key is appended to the secret key in order to respect the NIST API.
 *
 * \param[out] y field_vector_t* Representation of vector y
 * \param[out] H field_matrix_t* Representation of matrix H
 * \param[out] s field_vector_t* Representation of vector s
 * \param[out] C binary_matrix_t* Representation of matrix C
 * \param[in] sk String containing the secret key
 */
void ryde_secret_key_from_string(field_vector_t *y, field_matrix_t *H, field_vector_t *s, binary_matrix_t *C, const uint8_t* sk) {
 
    uint8_t sk_seed[RYDE_SECURITY_BYTES] = {0};
    uint8_t pk_seed[RYDE_SECURITY_BYTES] = {0};

    seedexpander_shake_t sk_seedexpander;
    seedexpander_shake_t pk_seedexpander;

    memcpy(sk_seed, sk, RYDE_SECURITY_BYTES);
    seedexpander_shake_init(&sk_seedexpander, sk_seed, RYDE_SECURITY_BYTES, NULL, 0);

    field_vector_t support[RYDE_PARAM_R] = {0};

    field_vector_t x1[RYDE_PARAM_N - RYDE_PARAM_K] = {0},
                       x2[RYDE_PARAM_K] = {0},
                       x[RYDE_PARAM_N] = {0},
                       c[RYDE_PARAM_N - RYDE_PARAM_R] = {0};

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
    memcpy(pk_seed, &sk[RYDE_SECURITY_BYTES], RYDE_SECURITY_BYTES);
    seedexpander_shake_init(&pk_seedexpander, pk_seed, RYDE_SECURITY_BYTES, NULL, 0);

    field_matrix_random(&pk_seedexpander, H, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);
    field_matrix_mul_by_vector_right(y, H, x2, RYDE_PARAM_N - RYDE_PARAM_K, RYDE_PARAM_K);
    field_vector_add(y, y, x1, RYDE_PARAM_N - RYDE_PARAM_K);

}
 