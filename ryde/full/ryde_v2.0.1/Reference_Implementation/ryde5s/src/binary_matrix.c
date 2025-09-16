/**
 * @file binary_matrix.c
 * @brief Implementation of binary_matrix.h
 */

#include "binary_matrix.h"
#include <stdlib.h>
#include <stdio.h>


/**
 * \fn void binary_matrix_add(binary_matrix_t *o, const binary_matrix_t *a, const binary_matrix_t *b, uint32_t rows, uint32_t columns)
 * \brief This functions adds two binary matrices.
 *
 * \param[out] o binary_matrix_t* Representation of matrix \f$ a + b \f$
 * \param[in] a binary_matrix_t* Representation of matrix a
 * \param[in] b binary_matrix_t* Representation of matrix b
 * \param[in] rows Row size of the matrices
 * \param[in] columns Column size of the matrices
 */
void binary_matrix_add(binary_matrix_t *o, const binary_matrix_t *a, const binary_matrix_t *b, uint32_t rows, uint32_t columns) {
    size_t block = (columns + 63) / 64;
    for(size_t j = 0 ; j < rows; ++j) {
        for(size_t k = 0 ; k < block; ++k) {
            o[j][k] = a[j][k] ^ b[j][k];
        }
    }
}


/**
 * \fn void binary_matrix_mul_by_constant(field_matrix_t *o, const binary_matrix_t *m, const field_t c, uint32_t rows, uint32_t columns);
 * \brief This functions multiplies a binary matrix by a field_t element.
 *
 * \param[out] o field_matrix_t* Representation of matrix \f$ c \times m \f$
 * \param[in] m binary_matrix_t* Representation of matrix m
 * \param[in] c field_t
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void binary_matrix_mul_by_constant(field_matrix_t *o, const binary_matrix_t *m, const field_t c, uint32_t rows, uint32_t columns) {
    field_t tmp;
    uint64_t mask;
    for(size_t j = 0 ; j < columns ; ++j) {
        for(size_t k = 0 ; k < rows ; ++k) {
            mask = (uint64_t)(m[k][j / 64] >> (j % 64));
            mask = -(mask & 1);
            for(size_t l = 0 ; l < RYDE_FIELD_WORDS; ++l) {
                tmp[l] = c[l] & mask;
            }
            field_copy(o[k][j], tmp);
        }
    }
}


/**
 * \fn void binary_matrix_set_to_zero(binary_matrix_t *m, uint32_t rows, uint32_t columns)
 * \brief This function sets a binary matrix to zero.
 *
 * \param[out] m binary_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void binary_matrix_set_to_zero(binary_matrix_t *m, uint32_t rows, uint32_t columns) {
    uint32_t words = (columns + 63) / 64;
    for(size_t i = 0 ; i < rows ; ++i) {
        memset(m[i], 0, words * sizeof(uint64_t));
    }
}


/**
 * \fn void binary_matrix_from_string(binary_matrix_t *m, uint32_t rows, uint32_t columns, const uint8_t* str)
 * \brief This function parses a string into a binary matrix.
 *
 * \param[out] m binary_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] cols Column size of the matrix
 * \param[in] str String to parse
 */
void binary_matrix_from_string(binary_matrix_t *m, uint32_t rows, uint32_t columns, const uint8_t* str) {
    uint32_t bytes1 = columns / 8;
    uint32_t bytes2 = columns % 8;
    uint32_t index = bytes1 * rows;

    binary_matrix_set_to_zero(m, rows, columns);

    for(size_t i = 0 ; i < rows ; i++) {
        memcpy(m[i], str + i * bytes1, bytes1);
    }

    uint8_t k = 0;
    for(size_t i = 0 ; i < rows ; i++) {
        for(size_t j = 1 ; j <= bytes2 ; j++) {
            uint8_t bit = (str[index] >> k % 8) & 0x01;
            m[i][(columns - j) / 64] |= (uint64_t)bit << ((columns - j) % 64);
            k++;
            if(k % 8 == 0) index++;
        }
    }
}


/**
 * \fn void binary_matrix_to_string(uint8_t* str, const binary_matrix_t *m, uint32_t rows, uint32_t columns)
 * \brief This function parses a binary matrix into a string.
 *
 * \param[out] str Output string
 * \param[in] m binary_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] cols Column size of the matrix
 */
void binary_matrix_to_string(uint8_t* str, const binary_matrix_t *m, uint32_t rows, uint32_t columns) {
    uint32_t bytes1 = columns / 8;
    uint32_t bytes2 = columns % 8;
    uint32_t index = bytes1 * rows;
    uint32_t str_size = ((rows * columns) % 8 == 0) ? (rows * columns) / 8 : (rows * columns) / 8 + 1;

    memset(str, 0, str_size);

    for(size_t i = 0 ; i < rows ; i++) {
        memcpy(str + i * bytes1, m[i], bytes1);
    }

    uint8_t k = 0;
    for(size_t i = 0 ; i < rows ; i++) {
        for(size_t j = 1 ; j <= bytes2 ; j++) {
            uint8_t bit = (uint8_t)(m[i][(columns - j) / 64] >> ((columns - j) % 64)) & 0x01;
            *(str + index) |= (bit << k % 8);
            k++;
            if(k % 8 == 0) index++;
        }
    }
}


/**
 * \fn void binary_matrix_mul_by_vector_right(field_vector_t *o, const binary_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns)
 * \brief This functions multiplies a binary matrix by a vector defined over the field.
 *
 * \param[out] o field_vector_t* Representation of vector \f$ m \times v \f$
 * \param[in] m binary_matrix_t* Representation of matrix m
 * \param[in] v field_vector_t* Representation of vector v
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of matrix and size of the vector
 */
void binary_matrix_mul_by_vector_right(field_vector_t *o, const binary_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns) {
    field_t tmp, acc;
    uint64_t mask;
    for(size_t i = 0 ; i < rows ; ++i) {
        field_set_to_zero(acc);
        for(size_t j = 0 ; j < columns ; ++j) {
            mask = (uint64_t)(m[i][j / 64] >> (j % 64));
            mask = -(mask & 1);
            for(size_t l = 0 ; l < RYDE_FIELD_WORDS; ++l) {
                tmp[l] = v[j][l] & mask;
            }
            field_add(acc, acc, tmp);
        }
        field_copy(o[i], acc);
    }
}


/**
 * \fn void binary_matrix_mul_by_vector_left(field_vector_t *o, const binary_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns)
 * \brief This functions multiplies a vector defined over the field by a binary matrix.
 *
 * \param[out] o field_vector_t* Representation of vector \f$ v \times m \f$
 * \param[in] m binary_matrix_t* Representation of matrix m
 * \param[in] v field_vector_t* Representation of vector v
 * \param[in] rows Row size of matrix and size of the vector
 * \param[in] columns Column size of the matrix
 */
void binary_matrix_mul_by_vector_left(field_vector_t *o, const binary_matrix_t *m, const field_vector_t *v, uint32_t rows, uint32_t columns) {
    field_t tmp, acc;
    uint64_t mask;
    for(size_t j = 0 ; j < columns ; ++j) {
        field_set_to_zero(acc);
        for(size_t k = 0 ; k < rows ; ++k) {
        mask = (uint64_t)(m[k][j / 64] >> (j % 64));
        mask = -(mask & 1);
        for(size_t l = 0 ; l < RYDE_FIELD_WORDS; ++l) {
            tmp[l] = v[k][l] & mask;
        }
        field_add(acc, acc, tmp);
        }
        field_copy(o[j], acc);
    }
}


/**
 * \fn void binary_matrix_copy(binary_matrix_t *o, const binary_matrix_t *m, uint32_t rows, uint32_t columns)
 * \brief This function copies a binary matrix.
 *
 * \param[out] o binary_matrix_t* Representation of matrix o
 * \param[in] m binary_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void binary_matrix_copy(binary_matrix_t *o, const binary_matrix_t *m, uint32_t rows, uint32_t columns) {
    uint32_t words = (columns + 63) / 64;
    for(size_t i = 0 ; i < rows ; ++i) {
        for(size_t j = 0 ; j < words ; ++j) {
            o[i][j] = m[i][j];
        }
    }
}


/**
 * \fn void binary_matrix_random(seedexpander_shake_t* ctx, binary_matrix_t *o, uint32_t rows, uint32_t columns)
 * \brief This function sets a binary matrix with random values using SHAKE.
 *
 * \param[out] ctx seedexpander_shake_t
 * \param[out] o binary_matrix_t* Representation of matrix o
 * \param[in] rows Row size of the matrix
 * \param[in] cols Column size of the matrix
 */
void binary_matrix_random(seedexpander_shake_t* ctx, binary_matrix_t *o, uint32_t rows, uint32_t columns) {
    uint32_t random_size = (rows * columns + 7) / 8;
    uint8_t random[random_size];
    seedexpander_shake_get_bytes(ctx, random, random_size);

    // We mask the random bit-string to ensure the correct amount of random bits
    uint8_t mask = (1 << (rows * columns) % 8) - 1;
    if (((rows * columns) % 8) == 0) { mask = 0xff; }
    random[random_size - 1] &= mask;

    binary_matrix_from_string(o, rows, columns, random);
}


/**
 * \fn void binary_matrix_print(const binary_matrix_t *m, uint32_t rows, uint32_t columns)
 * \brief Display a binary_matrix_t element.
 *
 * \param[in] m binary_matrix_t* Representation of matrix m
 * \param[in] rows Row size of the matrix
 * \param[in] columns Column size of the matrix
 */
void binary_matrix_print(const binary_matrix_t *m, uint32_t rows, uint32_t columns) {
    printf("[\n");
    for(size_t i = 0 ; i < rows ; ++i) {
        printf("[");
        for(size_t j = 0 ; j < columns ; ++j) {
        printf(" %X", (uint8_t)((m[i][j / 64] >> (j % 64)) & 1));
        }
        printf("]\n");
    }
    printf("]\n");
}