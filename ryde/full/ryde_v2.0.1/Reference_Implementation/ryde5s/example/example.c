/**
 * @file example.c
 * @brief Fixed example of the NIST api functions instantiated with RYDE TCitH scheme.
 */

#include <inttypes.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rng.h"
#include "api.h"
 
 
 
int main(void) {
    unsigned long long ryde_mlen = 22;
    unsigned long long ryde_smlen = 0;
    
    uint8_t ryde_pk[CRYPTO_PUBLICKEYBYTES] = {0};
    uint8_t ryde_sk[CRYPTO_SECRETKEYBYTES] = {0};
    uint8_t ryde_sm[CRYPTO_BYTES + ryde_mlen];
    
    // Message corresponds with: Rank sYndrome DEcoding
    uint8_t ryde_m[] = {0x52, 0x61, 0x6e, 0x6b, 0x20, 0x73, 0x59, 0x6e, 0x64, 0x72, 0x6f,
                        0x6d, 0x65, 0x20, 0x44, 0x45, 0x63, 0x6f, 0x64, 0x69, 0x6e, 0x67};

    int okay = 0;
    unsigned char seed[48] = {0};
    randombytes_init(seed, NULL, 256);

    /*************/
    /*    RYDE   */
    /*************/
    printf("# %s", CRYPTO_ALGNAME);

    okay = crypto_sign_keypair(ryde_pk, ryde_sk);
    if (okay != EXIT_SUCCESS) { return EXIT_FAILURE; }
    okay = crypto_sign(ryde_sm, &ryde_smlen, ryde_m, ryde_mlen, ryde_sk);
    if (okay != EXIT_SUCCESS) { return EXIT_FAILURE; }
    okay = crypto_sign_open(ryde_m, &ryde_mlen, ryde_sm, ryde_smlen, ryde_pk);
    if (okay != EXIT_SUCCESS) { return EXIT_FAILURE; }

#ifndef VERBOSE
    printf("\nseed: "); for(int k = 0 ; k < 48 ; ++k) printf("%02x", seed[k]);
    printf("\n\nsk: "); for(int k = 0 ; k < CRYPTO_SECRETKEYBYTES ; ++k) printf("%02x", ryde_sk[k]);
    printf("\npk: "); for(int k = 0 ; k < CRYPTO_PUBLICKEYBYTES ; ++k) printf("%02x", ryde_pk[k]);
    printf("\n\nlenght of m: %" PRId64 "\n", (uint64_t)ryde_mlen);
    printf("\nm: "); for(int k = 0 ; k < (int)ryde_mlen ; ++k) printf("%02x", ryde_m[k]);
    printf("\n\nlenght of sm: %" PRId64 "\n", (uint64_t)ryde_smlen);
    printf("\nsm: "); for(int k = 0 ; k < ((int)ryde_smlen) ; ++k) printf("%02x", ryde_sm[k]);
#endif
    printf("\n\n");

    return EXIT_SUCCESS;
}
