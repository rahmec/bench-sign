/**
 * @file field_matrix.c
 * @brief Implementation of field_matrix.h
 */

#include "field_matrix.h"
#include <stdlib.h>
#include <stdio.h>


/**
 * \fn void field_matrix_add(field_matrix_t *o, const field_matrix_t *m1, const field_matrix_t *m2, uint32_t rows, uint32_t columns)
 * \brief This functions adds matrices of finite field elements.
 *
 * \param[out] o field_matrix_t* Representation of matrix \f$ m1 \oplus m2 \f$
 * \param[in] m1 field_matrix_t* Representation of matrix m1
 * \param[in] m2 field_matrix_t* Representation of matrix m2
 * \param[in] rows Row size of the matrices
 * \param[in] columns Column size of the matrices
 */
void field_matrix_add(field_matrix_t *o, const field_matrix_t *m1, const field_matrix_t *m2, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        for(size_t j = 0 ; j < columns ; ++j) {
            field_add(o[i][j], m1[i][j], m2[i][j]);
        }
    }
}


/**
 * \fn void field_matrix_to_string(uint8_t* str, const field_matrix_t *m, uint32_t rows, uint32_t columns)
 * \brief This function parses a matrix of finite field elements into a string.
 *
 * \param[out] str Output string
 * \param[in] m field_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void field_matrix_to_string(uint8_t* str, const field_matrix_t *m, uint32_t rows, uint32_t columns) {
    field_vector_t t[rows * columns];
    for(size_t i = 0 ; i < rows ; i++) {
        for(size_t j = 0 ; j < columns ; j++) {
            field_copy(t[i * columns + j], m[i][j]);
        }
    }
    field_vector_to_string(str, t, rows * columns);
}

  
  
/**
 * \fn void field_matrix_from_string(field_matrix_t *m, uint32_t rows, uint32_t columns, const uint8_t* str)
 * \brief This function parses a string into a matrix of finite field elements.
 *
 * \param[out] m field_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 * \param[in] str String to parse
 */
void field_matrix_from_string(field_matrix_t *m, uint32_t rows, uint32_t columns, const uint8_t* str) {
    field_vector_t t[rows * columns];
    field_vector_from_string(t, rows * columns, str);
    for(size_t i = 0 ; i < rows ; i++) {
        for(size_t j = 0 ; j < columns ; j++) {
            field_copy(m[i][j], t[i * columns + j]);
        }
    }
}


/**
 * \fn void field_matrix_mul_by_vector_right(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns)
 * \brief This functions multiplies a matrix of finite field elements by a vector.
 *
 * \param[out] o field_vector_t equal to \f$ m \times v \f$
 * \param[in] m field_matrix_t* Representation of matrix m
 * \param[in] v field_vector_t
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix and size of the vector
 */
void field_matrix_mul_by_vector_right(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns) {
    field_t tmp, acc;
    for(size_t i = 0 ; i < rows ; ++i) {
        field_set_to_zero(acc);
        for(size_t j = 0 ; j < columns ; ++j) {
            field_mul(tmp, m[i][j], v[j]);
            field_add(acc, acc, tmp);
        }
        field_copy(o[i], acc);
    }
}



/**
 * \fn void field_matrix_mul_by_vector_left(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns)
 * \brief This functions multiplies a vector of finite field elements by a matrix.
 *
 * \param[out] o field_vector_t equal to \f$ v \times m \f$
 * \param[in] m field_matrix_t* Representation of matrix m
 * \param[in] v field_vector_t
 * \param[in] rows Row size of the matrix and size of the vector
 * \param[in] columns Column size of the matrix
 */
void field_matrix_mul_by_vector_left(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns) {
    field_t tmp, acc;
    for(size_t i = 0 ; i < columns ; ++i) {
        field_set_to_zero(acc);
        for(size_t j = 0 ; j < rows ; ++j) {
            field_mul(tmp, m[j][i], v[j]);
            field_add(acc, acc, tmp);
        }
        field_copy(o[i], acc);
    }
}



/**
 * \fn void field_matrix_mul_by_vector_left_transpose(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns)
 * \brief This functions multiplies a vector of finite field elements by the transpose of a matrix.
 *
 * \param[out] o field_vector_t equal to \f$ v \times m^\top \f$
 * \param[in] m field_matrix_t* Representation of matrix m
 * \param[in] v field_vector_t
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix and size of the vector
 */
void field_matrix_mul_by_vector_left_transpose(field_vector_t *o, const field_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns) {
    field_t tmp, acc;
    for(size_t i = 0 ; i < rows ; ++i) {
        field_set_to_zero(acc);
        for(size_t j = 0 ; j < columns ; ++j) {
            field_mul(tmp, m[i][j], v[j]);
            field_add(acc, acc, tmp);
        }
        field_copy(o[i], acc);
    }
}


/**
 * \fn void field_matrix_set_to_zero(field_matrix_t *m, uint32_t rows, uint32_t columns)
 * \brief This function sets a matrix of finite elements to zero.
 *
 * \param[out] m field_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void field_matrix_set_to_zero(field_matrix_t *m, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        field_vector_set_to_zero(m[i], columns);
    }
}


/**
 * \fn void field_matrix_copy(field_matrix_t *o, const field_matrix_t *m, uint32_t rows, uint32_t columns)
 * \brief This function copies a matrix of finite field elements to another one.
 *
 * \param[out] o field_matrix_t* Representation of matrix o
 * \param[in] m field_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void field_matrix_copy(field_matrix_t *o, const field_matrix_t *m, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        field_vector_copy(o[i], m[i], columns);
    }
}


/**
 * \fn void field_matrix_random(seedexpander_shake_t* ctx, field_matrix_t *o, uint32_t rows, uint32_t columns)
 * \brief This function sets a matrix of finite field elements with random values using SHAKE.
 *
 * \param[out] ctx Pointer to a seedexpander_shake_t structure
 * \param[out] m field_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void field_matrix_random(seedexpander_shake_t* ctx, field_matrix_t *o, uint32_t rows, uint32_t columns) {
    for(size_t i = 0 ; i < rows ; ++i) {
        field_vector_random(ctx, o[i], columns);
    }
}


/**
 * \fn void field_matrix_print(const field_matrix_t *m, uint32_t rows, uint32_t columns)
 * \brief Display a field_matrix_t element.
 *
 * \param[out] m field_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void field_matrix_print(const field_matrix_t *m, uint32_t rows, uint32_t columns) {
    printf("[\n");
    for(size_t i = 0 ; i < rows ; ++i) {
        printf("[\t");
        for(size_t j = 0 ; j < columns ; ++j) {
            field_print(m[i][j]);
        }
        printf("\t]\n");
    }
    printf("]\n");
}
