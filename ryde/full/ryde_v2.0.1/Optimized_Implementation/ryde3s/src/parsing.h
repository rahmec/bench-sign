/** 
 * @file parsing.h
 * @brief Functions to parse secret key and public key of the RYDE signature scheme
 */

 #ifndef RYDE_PARSING_H
 #define RYDE_PARSING_H
 
 #include "field_matrix.h"
 #include "binary_matrix.h"
 
 void ryde_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const field_vector_t *y);
 void ryde_public_key_from_string(field_matrix_t *H, field_vector_t *y, const uint8_t* pk);
 
 void ryde_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk_seed);
 void ryde_secret_key_from_string(field_vector_t *y, field_matrix_t *H, field_vector_t *s, binary_matrix_t *C, const uint8_t* sk);
 
 #endif //RYDE_PARSING_H
 
 