/**
 * @file field_matrix.h
 * @brief Matrix implementation (defined the field) of the RYDE scheme
 */


#ifndef RYDE_FIELD_MATRIX_H
#define RYDE_FIELD_MATRIX_H

#include "field_vector.h"
#include "parameters.h"

// The code implementation assumes field matrices of at most RYDE_PARAM_N columns
#if (RYDE_PARAM_K > RYDE_PARAM_N)
#error The current implementation assumes RYDE_PARAM_K must be smaller than or equal to RYDE_PARAM_N
#endif

typedef field_vector_t field_matrix_t[RYDE_PARAM_N];

void field_matrix_add(field_matrix_t *o, const field_matrix_t *m1, const field_matrix_t *m2, uint32_t rows, uint32_t columns);

void field_matrix_to_string(uint8_t* str, const field_matrix_t *m, uint32_t rows, uint32_t columns);
void field_matrix_from_string(field_matrix_t *m, uint32_t rows, uint32_t columns, const uint8_t* str);

void field_matrix_mul_by_vector_right(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns);
void field_matrix_mul_by_vector_left(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns);
void field_matrix_mul_by_vector_left_transpose(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns);

void field_matrix_set_to_zero(field_matrix_t *m, uint32_t rows, uint32_t columns);
void field_matrix_copy(field_matrix_t *o, const field_matrix_t *m, uint32_t rows, uint32_t columns);

void field_matrix_random(seedexpander_shake_t* ctx, field_matrix_t *o, uint32_t rows, uint32_t columns);

void field_matrix_print(const field_matrix_t *m, uint32_t rows, uint32_t columns);

#endif //RYDE_FIELD_MATRIX_H
