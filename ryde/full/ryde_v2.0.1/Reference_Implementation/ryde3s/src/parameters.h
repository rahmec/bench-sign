/**
 * @file parameters.h
 * @brief Parameters of the RYDE scheme
 */

#ifndef RYDE_3S_PARAMETER_H
#define RYDE_3S_PARAMETER_H

#define RYDE_NAME "RYDE-3S"              /**< RYDE variant name */
#define RYDE_SECURITY 192                /**< Expected security level (bits) */
#define RYDE_SECURITY_BYTES 24           /**< Expected security level (bytes) */

#define RYDE_SECRET_KEY_BYTES 48         /**< Secret key size */
#define RYDE_PUBLIC_KEY_BYTES 101        /**< Public key size */
#define RYDE_SIGNATURE_BYTES 6728        /**< Signature size */

#define RYDE_SALT_BYTES 48               /**< Expected salt (bytes) */
#define RYDE_HASH_BYTES 48               /**< Expected hash (bytes) */

#define RYDE_PARAM_Q 2                   /**< Parameter q of the scheme (finite field GF(q^m)) */
#define RYDE_PARAM_M 61                  /**< Parameter m of the scheme (finite field GF(q^m)) */
#define RYDE_PARAM_K 51                  /**< Parameter k of the scheme (code dimension) */
#define RYDE_PARAM_N 61                  /**< Parameter n of the scheme (code length) */
#define RYDE_PARAM_R 5                   /**< Parameter r of the scheme (rank of vectors) */

#define RYDE_PARAM_M_MASK 0x1f           /**< Mask related to the representation of finite field elements */
#define RYDE_PARAM_M_BYTES 8             /**< Number of bytes required to store a GF(2^m) element */
#define RYDE_PARAM_M_WORDS 1             /**< Number of 64-bits words required to store a GF(2^m) element */
#define RYDE_VEC_K_BYTES 389             /**< Number of bytes required to store a vector of size k */
#define RYDE_VEC_N_BYTES 466             /**< Number of bytes required to store a vector of size n */
#define RYDE_VEC_R_BYTES 39              /**< Number of bytes required to store a vector of size r */
#define RYDE_VEC_N_MINUS_ONE_BYTES 458   /**< Number of bytes required to store a vector of size (n - 1) */
#define RYDE_VEC_R_MINUS_ONE_BYTES 31    /**< Number of bytes required to store a vector of size (r - 1) */
#define RYDE_VEC_R_MINUS_ONE_MASK 0xf    /**< Mask related to the representation of a vector of size (r - 1) */
#define RYDE_VEC_RHO_BYTES 31            /**< Number of bytes required to store a vector of size rho */
#define RYDE_VEC_RHO_MASK 0xf            /**< Mask related to the representation of a vector of size rho */
#define RYDE_MAT_FQ_BYTES 35             /**< Number of bytes required to store a binary matrix of size r x (n - r) */
#define RYDE_MAT_FQ_MASK 0xff            /**< Mask related to the representation of a binary matrix of size r x (n - r) */

#define RYDE_PARAM_N_1 4096              /**< Parameter N_1 of the scheme */
#define RYDE_PARAM_N_2 4096              /**< Parameter N_2 of the scheme */
#define RYDE_PARAM_N_1_BITS 12           /**< Parameter N_1 of the scheme (bits) */
#define RYDE_PARAM_N_2_BITS 12           /**< Parameter N_2 of the scheme (bits) */
#define RYDE_PARAM_N_1_BYTES 2           /**< Parameter N_1 of the scheme (bytes) */
#define RYDE_PARAM_N_2_BYTES 2           /**< Parameter N_2 of the scheme (bytes) */
#define RYDE_PARAM_N_1_MASK 0xf          /**< Parameter N_1 of the scheme (mask) */
#define RYDE_PARAM_N_2_MASK 0xf          /**< Parameter N_2 of the scheme (mask) */
#define RYDE_PARAM_TAU 17                /**< Parameter tau of the scheme (number of iterations) */
#define RYDE_PARAM_TAU_1 17              /**< Parameter tau_1 of the scheme (number of iterations concerning N1) */
#define RYDE_PARAM_TAU_2 0               /**< Parameter tau_2 of the scheme (number of iterations concerning N2) */
#define RYDE_PARAM_RHO 4                 /**< Parameter rho of the scheme (dimension of the extension)*/

#define RYDE_PARAM_TREE_LEAVES 69632     /**< Number of leaves in the tree */

#define RYDE_PARAM_CHALLENGE_1_BYTES 305 /**< Number of bytes required to store the first challenge */
#define RYDE_PARAM_CHALLENGE_2_BYTES 26  /**< Number of bytes required to store the second challenge */
#define RYDE_PARAM_W_BYTES 1             /**< Number of bytes in the second hash to be zero */
#define RYDE_PARAM_W_MASK 0xf8           /**< Mask for the most significant byte in the second hash */
#define RYDE_PARAM_T_OPEN 174            /**< Maximum sibling path length allowed */

#endif //RYDE_3S_PARAMETER_H
