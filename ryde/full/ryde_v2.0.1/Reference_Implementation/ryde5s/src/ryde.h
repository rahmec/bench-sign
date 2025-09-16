/** 
 * @file tcith_scheme.h
 * @brief Functions of the RYDE TCitH signature scheme
 */

#ifndef RYDE_TCITH_SCHEME_H
#define RYDE_TCITH_SCHEME_H

#include <stdint.h>
#include <stdio.h>

int ryde_keygen(uint8_t* pk, uint8_t* sk);
int ryde_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk);
int ryde_verify(const uint8_t* signature, size_t signature_size, const uint8_t* message, size_t message_size, const uint8_t* pk);

#endif //RYDE_TCITH_SCHEME_H
