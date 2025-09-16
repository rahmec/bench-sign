/**
 * @file binary_matrix.h
 * @brief Matrix implementation (defined the GF(2)) of the RYDE scheme
 */


#ifndef RYDE_BINARY_MATRIX_H
#define RYDE_BINARY_MATRIX_H

#include "field_matrix.h"
#include "parameters.h"

// The code implementation assumes binary matrices of at most (RYDE_PARAM_N - RYDE_PARAM_R) columns

#define BINARY_MATRIX_COLUMNS(x) (((x) + 63) / 64)
typedef uint64_t binary_matrix_t[BINARY_MATRIX_COLUMNS(RYDE_PARAM_N - RYDE_PARAM_R)];

void binary_matrix_add(binary_matrix_t *o, const binary_matrix_t *a, const binary_matrix_t *b, uint32_t rows, uint32_t columns);
void binary_matrix_mul_by_constant(field_matrix_t *o, const binary_matrix_t *m, const field_t c, uint32_t rows, uint32_t columns);

void binary_matrix_set_to_zero(binary_matrix_t *m, uint32_t rows, uint32_t columns);
void binary_matrix_from_string(binary_matrix_t *m, uint32_t rows, uint32_t columns, const uint8_t* str);
void binary_matrix_to_string(uint8_t* str, const binary_matrix_t *m, uint32_t rows, uint32_t columns);

void binary_matrix_mul_by_vector_right(field_vector_t *o, const binary_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns);
void binary_matrix_mul_by_vector_left(field_vector_t *o, const binary_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns);

void binary_matrix_copy(binary_matrix_t *o, const binary_matrix_t *m, uint32_t rows, uint32_t columns);

void binary_matrix_random(seedexpander_shake_t* ctx, binary_matrix_t *o, uint32_t rows, uint32_t columns);

void binary_matrix_print(const binary_matrix_t *m, uint32_t rows, uint32_t columns);

#endif //RYDE_BINARY_MATRIX_H
