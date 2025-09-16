/**
 * @file field_vector.h
 * @brief Vectors implementation (defined the field) of the RYDE scheme
 */


#ifndef RYDE_FIELD_VECTOR_H
#define RYDE_FIELD_VECTOR_H

#include "field.h"
#include "hash_sha3.h"
#include "parameters.h"

typedef field_t field_vector_t;


void field_vector_add(field_vector_t *o,  const field_vector_t *v1,  const field_vector_t *v2,  uint32_t size);
void field_vector_mul_by_constant(field_vector_t *o,  const field_vector_t *v,  const field_t e, uint32_t size);

void field_vector_to_string(uint8_t* str, const field_vector_t *v,  uint32_t size);
void field_vector_from_string(field_vector_t *v,  uint32_t size, const uint8_t* str);

void field_vector_set_to_zero(field_vector_t *v,  uint32_t size);
void field_vector_copy(field_vector_t *o,  const field_vector_t *v,  uint32_t size);

void field_vector_random(seedexpander_shake_t* ctx, field_vector_t *o,  uint32_t size);

uint32_t field_vector_gauss(field_vector_t *v,  uint32_t size, uint8_t reduced_flag, field_vector_t **other_matrices, uint32_t nMatrices);
uint32_t field_vector_get_rank(const field_vector_t *v,  uint32_t size);
void field_vector_random_full_rank_with_one(seedexpander_shake_t* ctx, field_vector_t *o,  uint32_t size);

void field_vector_print(const field_vector_t *v,  uint32_t size);

static inline uint32_t is_nonequal_u32(uint32_t a, uint32_t b) {
	uint32_t r = 0;
	unsigned char *ta = (unsigned char *)&a;
	unsigned char *tb = (unsigned char *)&b;
	r = (ta[0] ^ tb[0]) | (ta[1] ^ tb[1]) | (ta[2] ^ tb[2]) |  (ta[3] ^ tb[3]);
	r = (-r);
	r = r >> 31;
	return (uint32_t)r;
}

#endif //RYDE_FIELD_VECTOR_H
