/**
 * @file api.h
 * @brief NIST SIGN API
 */


#ifndef NIST_SIGN_API_H
#define NIST_SIGN_API_H

#include "parameters.h"

#define CRYPTO_ALGNAME RYDE_NAME

#define CRYPTO_PUBLICKEYBYTES RYDE_PUBLIC_KEY_BYTES
#define CRYPTO_SECRETKEYBYTES RYDE_SECRET_KEY_BYTES
#define CRYPTO_BYTES          RYDE_SIGNATURE_BYTES

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);
int crypto_sign(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk);
int crypto_sign_open(unsigned char *m, unsigned long long *mlen, const unsigned char *sm, unsigned long long smlen, const unsigned char *pk);

#endif //NIST_SIGN_API_H
