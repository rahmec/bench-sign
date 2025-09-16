/**
 * @file field_53.h
 * @brief Basefield implementation of the RYDE scheme
 */

#ifndef RYDE_FIELD_53_H
#define RYDE_FIELD_53_H

#include <stdint.h>

#define RYDE_FIELD_Q        2
#define RYDE_FIELD_M        53
#define RYDE_FIELD_WORDS    1
#define RYDE_FIELD_WORDS_UR 2
#define RYDE_FIELD_BITS     64
#define RYDE_FIELD_BYTES    7
#define RYDE_FIELD_BYTES_UR 14
#define RYDE_FIELD_MASK     31

typedef uint64_t field_t[RYDE_FIELD_WORDS];
typedef uint64_t field_ur_t[RYDE_FIELD_BYTES_UR];

void field_add(field_t o, const field_t e1, const field_t e2);
void field_mul(field_t o, const field_t e1, const field_t e2);
void field_sqr(field_t o, const field_t e);

void field_set_to_zero(field_t o);
void field_set_to_one(field_t o);
void field_copy(field_t o, const field_t e);
void field_cmov(field_t o, const field_t e1, const field_t e2, uint8_t mask);

uint8_t field_is_zero(const field_t e);
uint8_t field_is_equal(const field_t e1, const field_t e2);

int32_t field_get_degree(const field_t e);
uint8_t field_get_coefficient(const field_t e, uint32_t index);
void field_set_coefficient(field_t o, uint32_t index, uint8_t bit);

void field_to_string(uint8_t* str, const field_t e);
void field_print(const field_t e);

static inline void cmove_u64(uint64_t *output, uint8_t input_decision, uint64_t input_a, uint64_t input_b) {
    uint8_t x1;
    uint64_t x2;
    uint64_t x3;
    x1 = (!(!input_decision));
    x2 = ((int8_t) (UINT64_C(0x0) - x1) & UINT64_C(0xffffffffffffffff));
    x3 = ((x2 & input_b) | ((~x2) & input_a));
    *output = x3;
}

static inline uint64_t is_nonzero_u64(uint64_t x) {
    return (uint8_t) ((x | (UINT64_C(0) - x)) >> 63);
}

static inline uint64_t is_zero_u64(uint64_t x) {
    return (uint8_t) (UINT64_C(1) ^ is_nonzero_u64(x));
}

#endif //RYDE_FIELD_53_H
