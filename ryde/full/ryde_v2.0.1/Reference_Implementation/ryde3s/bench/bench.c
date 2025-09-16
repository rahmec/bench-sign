/**
 * @file bench.c
 * @brief Benchmarking of the NIST api functions instantiated with RYDE TCitH scheme.
 */

#include <unistd.h>
#include <sys/syscall.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rng.h"
#include "api.h"


#define NB_TEST 25
#define NB_SAMPLES 25


static inline uint64_t get_cycles(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return (uint64_t)hi << 32 | lo;
}



int main(void) {
    unsigned long long ryde_mlen = 1;
    unsigned long long ryde_smlen;
    unsigned char ryde_m[ryde_mlen];
    ryde_m[0] = 0;

    unsigned char ryde_pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char ryde_sk[CRYPTO_SECRETKEYBYTES];
    unsigned char ryde_sm[CRYPTO_BYTES + ryde_mlen];

    unsigned long long timer, t1, t2;
    unsigned long long ryde_keypair_mean = 0, ryde_crypto_sign_mean = 0, ryde_crypto_sign_open_mean = 0;
    int ryde_failures = 0;



    unsigned char seed[48] = {0};
    //    (void)syscall(SYS_getrandom, seed, 48, 0);
    randombytes_init(seed, NULL, 256);


    /*************/
    /*    RYDE   */
    /*************/

    // Cache memory heating
    for(size_t i = 0 ; i < NB_TEST ; i++) {
        crypto_sign_keypair(ryde_pk, ryde_sk);
    }

    // Measurement
    for(size_t i = 0 ; i < NB_SAMPLES ; i++) {
        printf("Benchmark (crypto_sign_keypair):\t");
        printf("%2lu%%", 100 * i / NB_SAMPLES);
        fflush(stdout);
        printf("\r\x1b[K");

        timer = 0;

        for(size_t j = 0 ; j < NB_TEST ; j++) {
            randombytes(seed, 48);
            randombytes_init(seed, NULL, 256);

            t1 = get_cycles();
            crypto_sign_keypair(ryde_pk, ryde_sk);
            t2 = get_cycles();

            timer += t2 - t1;
        }

        ryde_keypair_mean += timer / NB_TEST;
    }
    printf("\nBenchmark (crypto_sign_keypair)\n");



    for(size_t i = 0 ; i < NB_SAMPLES ; i++) {
        printf("Benchmark (crypto_sign):\t");
        printf("%2lu%%", 100 * i / NB_SAMPLES);
        fflush(stdout);
        printf("\r\x1b[K");

        randombytes(seed, 48);
        randombytes_init(seed, NULL, 256);

        crypto_sign_keypair(ryde_pk, ryde_sk);
        timer = 0;

        for(size_t j = 0 ; j < NB_TEST ; j++) {
            randombytes(seed, 48);
            randombytes_init(seed, NULL, 256);

            t1 = get_cycles();
            crypto_sign(ryde_sm, &ryde_smlen, ryde_m, ryde_mlen, ryde_sk);
            t2 = get_cycles();

            timer += t2 - t1;
        }

        ryde_crypto_sign_mean += timer / NB_TEST;
    }
    printf("Benchmark (crypto_sign)\n");



    for(size_t i = 0 ; i < NB_SAMPLES ; i++) {
        printf("Benchmark (crypto_sign_open):\t");
        printf("%2lu%%", 100 * i / NB_SAMPLES);
        fflush(stdout);
        printf("\r\x1b[K");

        randombytes(seed, 48);
        randombytes_init(seed, NULL, 256);

        crypto_sign_keypair(ryde_pk, ryde_sk);
        crypto_sign(ryde_sm, &ryde_smlen, ryde_m, ryde_mlen, ryde_sk);
        if (crypto_sign_open(ryde_m, &ryde_mlen, ryde_sm, ryde_smlen, ryde_pk) == -1) { ryde_failures++; }
        timer = 0;

        for(size_t j = 0 ; j < NB_TEST ; j++) {
            randombytes(seed, 48);
            randombytes_init(seed, NULL, 256);

            t1 = get_cycles();
            crypto_sign_open(ryde_m, &ryde_mlen, ryde_sm, ryde_smlen, ryde_pk);
            t2 = get_cycles();

            timer += t2 - t1;
        }

        ryde_crypto_sign_open_mean += timer / NB_TEST;
    }
    printf("Benchmark (crypto_sign_open)\n");


    printf("\n %s", CRYPTO_ALGNAME);
    printf("\n  Failures: %i", ryde_failures);
    printf("\n  crypto_sign_keypair: %lld CPU cycles", ryde_keypair_mean / NB_SAMPLES);
    printf("\n  crypto_sign:         %lld CPU cycles", ryde_crypto_sign_mean / NB_SAMPLES);
    printf("\n  crypto_sign_open:    %lld CPU cycles", ryde_crypto_sign_open_mean / NB_SAMPLES);
    printf("\n\n");

    return 0;
}
