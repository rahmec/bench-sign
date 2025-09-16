
#ifndef MIRATH_TCITH_PARAMS_3B_SHORT_H
#define MIRATH_TCITH_PARAMS_3B_SHORT_H

#define MIRATH_SECURITY 192			        /**< Expected security level (bits) */
#define MIRATH_SECURITY_BYTES 24		    /**< Expected security level (bytes) */

#define MIRATH_PARAM_SALT_BYTES (2 * MIRATH_SECURITY_BYTES)

#define MIRATH_SECRET_KEY_BYTES 48		    /**< Secret key size */
#define MIRATH_PUBLIC_KEY_BYTES 84		    /**< Public key size */
#define MIRATH_SIGNATURE_BYTES 6514		    /**< Signature size */

#define MIRATH_PARAM_Q 2			        /**< Parameter q of the scheme (finite field GF(q^m)) */
#define MIRATH_PARAM_M 50			        /**< Parameter m of the scheme (finite field GF(q^m)) */
#define MIRATH_PARAM_K 2024			        /**< Parameter k of the scheme (code dimension) */
#define MIRATH_PARAM_N 50			        /**< Parameter n of the scheme (code length) */
#define MIRATH_PARAM_R 5			        /**< Parameter r of the scheme (rank of vectors) */

#define MIRATH_PARAM_N_1 4096			    /**< Parameter N_1 of the scheme */
#define MIRATH_PARAM_N_2 4096			    /**< Parameter N_2 of the scheme */
#define MIRATH_PARAM_N_1_BITS 12		    /**< Parameter tau_1 of the scheme (bits) */
#define MIRATH_PARAM_N_2_BITS 12		    /**< Parameter tau_2 of the scheme (bits) */
#define MIRATH_PARAM_N_1_BYTES 2		    /**< Parameter tau_1 of the scheme (bytes) */
#define MIRATH_PARAM_N_2_BYTES 2		    /**< Parameter tau_2 of the scheme (bytes) */
#define MIRATH_PARAM_N_1_MASK 0xf		    /**< Parameter tau_1 of the scheme (mask) */
#define MIRATH_PARAM_N_2_MASK 0xf		    /**< Parameter tau_2 of the scheme (mask) */
#define MIRATH_PARAM_TAU 17			        /**< Parameter tau of the scheme (number of iterations) */
#define MIRATH_PARAM_TAU_1 17			    /**< Parameter tau_1 of the scheme (number of iterations concerning N1) */
#define MIRATH_PARAM_TAU_2 0		        /**< Parameter tau_2 of the scheme (number of iterations concerning N2) */
#define MIRATH_PARAM_RHO 16			        /**< Parameter rho of the scheme (dimension of the extension)*/
#define MIRATH_PARAM_MU 12			        /**< Parameter mu of the scheme */

#define MIRATH_PARAM_TREE_LEAVES 69632	    /**< Number of leaves in the tree */

#define MIRATH_PARAM_CHALLENGE_2_BYTES 26	/**< Number of bytes required to store the second challenge */
#define MIRATH_PARAM_HASH_2_MASK_BYTES 1	/**< Number of last bytes in the second hash to be zero */
#define MIRATH_PARAM_HASH_2_MASK 0x1f	    /**< Mask for the last byte in the second hash to be zero */
#define MIRATH_PARAM_T_OPEN 174		        /**< Maximum sibling path length allowed */

#endif //MIRATH_TCITH_PARAMS_3B_SHORT_H
