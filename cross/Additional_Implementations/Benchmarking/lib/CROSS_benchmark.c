/**
 *
 * Reference ISO-C11 Implementation of CROSS.
 *
 * @version 2.0 (February 2025)
 *
 * Authors listed in alphabetical order:
 * 
 * @author: Alessandro Barenghi <alessandro.barenghi@polimi.it>
 * @author: Marco Gianvecchio <marco.gianvecchio@mail.polimi.it>
 * @author: Patrick Karl <patrick.karl@tum.de>
 * @author: Gerardo Pelosi <gerardo.pelosi@polimi.it>
 * @author: Jonas Schupp <jonas.schupp@tum.de>
 * 
 * 
 * This code is hereby placed in the public domain.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define HIGH_PERFORMANCE_X86_64
#include "timing_and_stat.h"
#include "CROSS.h"
#include "csprng_hash.h"
#include "api.h"


#define NUM_TESTS 128

/// Convert seconds to milliseconds
#define SEC_TO_MS(sec) ((sec)*1000)
/// Convert seconds to microseconds
#define SEC_TO_US(sec) ((sec)*1000000)
/// Convert seconds to nanoseconds
#define SEC_TO_NS(sec) ((sec)*1000000000)

/// Convert nanoseconds to seconds
#define NS_TO_SEC(ns)   ((ns)/1000000000)
/// Convert nanoseconds to milliseconds
#define NS_TO_MS(ns)    ((ns)/1000000)
/// Convert nanoseconds to microseconds
#define NS_TO_US(ns)    ((ns)/1000)


void microbench(){
    welford_t timer;
    welford_init(&timer);

    uint64_t cycles;
    for(int i = 0; i <NUM_TESTS; i++) {
        cycles = x86_64_rtdsc();
        welford_update(&timer,(x86_64_rtdsc()-cycles)/1000.0);
    }
    fprintf(stderr,"microbench kCycles (avg,stddev):");
    welford_print(timer);
    printf("\n");

}

void info(){
    fprintf(stderr,"CROSS benchmarking utility\n");
    fprintf(stderr,"Code parameters: n= %d, k= %d, p=%d\n", N,K,P);
    fprintf(stderr,"restriction size: z=%d\n",Z);
    fprintf(stderr,"Fixed weight challenge vector: %d rounds, weight %d \n",T,W);
    fprintf(stderr,"Private key: %luB\n", sizeof(sk_t));
    fprintf(stderr,"Public key %luB\n", sizeof(pk_t));
    fprintf(stderr,"Signature: %luB\n", sizeof(CROSS_sig_t));

}

void CROSS_sign_verify_speed(int print_tex){
    fprintf(stderr,"Computing number of clock cycles as the average of %d runs\n", NUM_TESTS);
    uint64_t cycles;
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    unsigned char m[8] = "Signme!";
    unsigned long long mlen = 8;

    unsigned long long smlen;
    unsigned char sm[sizeof(m) + CRYPTO_BYTES];

    welford_t timer_KG,timer_Sig,timer_Ver;
    welford_init(&timer_KG);
    welford_init(&timer_Sig);
    welford_init(&timer_Ver);

    struct timespec ts;
    uint64_t ms_start;
    uint64_t ms_end;
    uint64_t ms_sum;
    long double ms_kg;
    long double ms_sig;
    long double ms_ver;


    ms_sum=0;

    for(int i = 0; i <NUM_TESTS; i++) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        ms_start = SEC_TO_MS((uint64_t)ts.tv_sec) + NS_TO_MS((uint64_t)ts.tv_nsec);

        cycles = x86_64_rtdsc();
        //CROSS_keygen(&sk,&pk);
        crypto_sign_keypair(pk, sk);
        welford_update(&timer_KG,(x86_64_rtdsc()-cycles)/1000.0);
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        ms_end = SEC_TO_MS((uint64_t)ts.tv_sec) + NS_TO_MS((uint64_t)ts.tv_nsec);
        ms_sum += ms_end-ms_start;

    }
    ms_kg=(long double)ms_sum/NUM_TESTS;
    ms_sum = 0;

    for(int i = 0; i <NUM_TESTS; i++) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        ms_start = SEC_TO_MS((uint64_t)ts.tv_sec) + NS_TO_MS((uint64_t)ts.tv_nsec);
        cycles = x86_64_rtdsc();
        //CROSS_sign(&sk,message,8,&signature);
        crypto_sign(sm, &smlen, m, sizeof(m), sk);
        welford_update(&timer_Sig,(x86_64_rtdsc()-cycles)/1000.0);
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        ms_end = SEC_TO_MS((uint64_t)ts.tv_sec) + NS_TO_MS((uint64_t)ts.tv_nsec);
        ms_sum += ms_end-ms_start;
    }
    ms_sig=(long double)ms_sum/NUM_TESTS;
    ms_sum = 0;
    int is_signature_still_ok = 0;
    for(int i = 0; i <NUM_TESTS; i++) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        ms_start = SEC_TO_MS((uint64_t)ts.tv_sec) + NS_TO_MS((uint64_t)ts.tv_nsec);
        cycles = x86_64_rtdsc();
        //int is_signature_ok = CROSS_verify(&pk,message,8,&signature);
        int is_signature_ok = crypto_sign_open(m, &mlen, sm, smlen, pk);
        welford_update(&timer_Ver,(x86_64_rtdsc()-cycles)/1000.0);
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        ms_end = SEC_TO_MS((uint64_t)ts.tv_sec) + NS_TO_MS((uint64_t)ts.tv_nsec);
        ms_sum += ms_end-ms_start;
        is_signature_still_ok = is_signature_ok || is_signature_still_ok;
    }
    ms_ver=(long double)ms_sum/NUM_TESTS;
    ms_sum = 0;
    if(print_tex){
      /* print a convenient machine extractable table row pair */
      printf("TIME & ");
#if defined(RSDP)
      printf("RSDP    & ");
#elif defined(RSDPG)
      printf("RSDP(G) & ");
#endif
#if defined(SHA2_HASH)
      printf("SHA-2 & ");
#elif defined(SHA3_HASH)
      printf("SHA-3 & ");
#endif
#if defined(CATEGORY_1)
      printf("Cat. 1 & ");
#elif defined(CATEGORY_3)
      printf("Cat. 3 & ");
#elif defined(CATEGORY_5)
      printf("Cat. 5 & ");
#endif
#if defined(SIG_SIZE)
      printf("Size  & ");
#elif defined(BALANCED)
      printf("Balan & ");      
#elif defined(SPEED)
      printf("Speed & ");
#endif
      // printf(" & ");
      welford_print_tex(timer_KG);
      printf(" & ");
      welford_print_tex(timer_Sig);
      printf(" & ");
      welford_print_tex(timer_Ver);
      printf("\n SPACE & ");
#if defined(RSDP)
      printf("RSDP    & ");
#elif defined(RSDPG)
      printf("RSDP(G) & ");
#endif
#if defined(CATEGORY_1)
      printf("Cat. 1 & ");
#elif defined(CATEGORY_3)
      printf("Cat. 3 & ");
#elif defined(CATEGORY_5)
      printf("Cat. 5 & ");
#endif
#if defined(SIG_SIZE)
      printf("Size  & ");
#elif defined(SPEED)
      printf("Speed & ");
#endif
      printf(" %lu &", sizeof(sk_t));
      printf(" %lu &", sizeof(pk_t));
      printf(" %lu ", sizeof(CROSS_sig_t));
      printf(" \\\\\n");
    } else {
        info();
        printf("Timings (kcycles):\n");
        printf("Keygen kCycles (avg,stddev): ");
        welford_print(timer_KG);
        printf("\n");
        printf("Keygen milliseconds (avg): %0.2Lf \n",ms_kg);

        printf("Signing kCycles (avg,stddev): ");
        welford_print(timer_Sig);
        printf("\n");
        printf("Signing milliseconds (avg): %0.2Lf \n",ms_sig);

        printf("Verification kCycles (avg,stddev):");
        welford_print(timer_Ver);
        printf("\n");
        printf("Verification milliseconds (avg): %0.2Lf \n",ms_ver);
        // Changed to 0 because of API
        fprintf(stderr,"Keygen-Sign-Verify: %s", is_signature_still_ok == 0 ? "functional\n": "not functional\n" );
    }
}


int main(int argc, char* argv[]){
    csprng_initialize(&platform_csprng_state,
                      (const unsigned char *)"0123456789012345",16,0);
    fprintf(stderr,"CROSS reference implementation benchmarking tool\n");
    if ( (argc>1) &&
         (argv[1][0] == '-' ) &&
         (argv[1][1] == 'T' )){
        CROSS_sign_verify_speed(1);
    } else {
        CROSS_sign_verify_speed(0);
    }
    return 0;
}
