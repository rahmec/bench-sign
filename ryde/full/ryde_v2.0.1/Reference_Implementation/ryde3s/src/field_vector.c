/**
 * @file field_vector.c
 * @brief Implementation of field_vector.h
 */


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "field_vector.h"

/**
 * \fn void field_vector_add(field_vector_t *o, const field_vector_t *v1, const field_vector_t *v2, uint32_t size)
 * \brief This function adds two vectors of finite field elements.
 *
 * \param[out] o field_vector_t* Representation of vector <b>v1</b> and <b>v2</b>
 * \param[in] v1 field_vector_t* Representation of vector v1
 * \param[in] v2 field_vector_t* Representation of vector v2
 * \param[in] size Size of the vectors
 */
void field_vector_add(field_vector_t *o, const field_vector_t *v1, const field_vector_t *v2, uint32_t size) {
    for(size_t i = 0 ; i < size ; ++i) {
        field_add(o[i], v1[i], v2[i]);
    }
}


/**
 * \fn void field_vector_mul_by_constant(field_vector_t *o, const field_vector_t *v, const field_t e, uint32_t size)
 * \brief This functions multiplies a vector of finite field elements by a scalar.
 *
 * \param[out] o field_vector_t* Representation of vector \f$ e \times v \f$
 * \param[in] v field_vector_t* Representation of vector v
 * \param[in] e field_t
 * \param[in] size Size of the vector
 */
void field_vector_mul_by_constant(field_vector_t *o, const field_vector_t *v, const field_t e, uint32_t size) {
    for(size_t i = 0 ; i < size ; ++i) {
      field_mul(o[i], v[i], e);
    }
  }
  

/**
 * \fn void field_vector_to_string(uint8_t* str, const field_vector_t *v, uint32_t size)
 * \brief This function parses a vector of finite field elements into a string.
 *
 * \param[out] str Output string
 * \param[in] v field_vector_t* Representation of vector v
 * \param[in] size Size of the vector
 */
void field_vector_to_string(uint8_t* str, const field_vector_t *v, uint32_t size) {
    uint32_t bytes1 = RYDE_FIELD_M / 8;
    uint32_t bytes2 = RYDE_FIELD_M % 8;
    uint32_t index = bytes1 * size;
    uint32_t str_size = ((size * RYDE_FIELD_M) % 8 == 0) ? (size * RYDE_FIELD_M) / 8 : (size * RYDE_FIELD_M) / 8 + 1;

    memset(str, 0, str_size);

    for(size_t i = 0 ; i < size ; i++) {
        memcpy(str + i * bytes1, v[i], bytes1);
    }

    uint8_t k = 0;
    for(size_t i = 0 ; i < size ; i++) {
        for(size_t j = 1 ; j <= bytes2 ; j++) {
            uint8_t bit = field_get_coefficient(v[i], RYDE_FIELD_M - j);
            *(str + index) |= (bit << k % 8);
            k++;
            if(k % 8 == 0) index++;
        }
    }
}


/**
 * \fn void field_vector_from_string(field_vector_t *v, uint32_t size, const uint8_t* str)
 * \brief This function parses a string into a vector of finite field elements.
 *
 * \param[out] v field_vector_t* Representation of vector v
 * \param[in] size Size of the vector
 * \param[in] str String to parse
 */
void field_vector_from_string(field_vector_t *v, uint32_t size, const uint8_t* str) {
    uint32_t bytes1 = RYDE_FIELD_M / 8;
    uint32_t bytes2 = RYDE_FIELD_M % 8;
    uint32_t index = bytes1 * size;

    field_vector_set_to_zero(v, size);

    for(size_t i = 0 ; i < size ; i++) {
        memcpy(v[i], str + i * bytes1, bytes1);
    }

    uint8_t k = 0;
    for(size_t i = 0 ; i < size ; i++) {
        for(size_t j = 1 ; j <= bytes2 ; j++) {
            uint8_t bit = (str[index] >> k % 8) & 0x01;
            field_set_coefficient(v[i], RYDE_FIELD_M - j, bit);
            k++;
            if(k % 8 == 0) index++;
        }
    }
}


/**
 * \fn void field_vector_set_to_zero(field_vector_t *v, uint32_t size)
 * \brief This function sets a vector to zero.
 *
 * \param[out] v field_vector_t* Representation of vector v
 * \param[in] size Size of the vector
 */
void field_vector_set_to_zero(field_vector_t *v, uint32_t size) {
    for(size_t i = 0 ; i < size ; ++i) {
        field_set_to_zero(v[i]);
    }
}


/**
 * \fn void field_vector_copy(field_vector_t *o, const field_vector_t *v, uint32_t size)
 * \brief This function copies a vector to another one.
 *
 * \param[out] o field_vector_t* Representation of vector o
 * \param[in] v field_vector_t* Representation of vector v
 * \param[in] size Size of the vectors
 */
void field_vector_copy(field_vector_t *o, const field_vector_t *v, uint32_t size) {
    for(size_t i = 0 ; i < size ; ++i) {
        field_copy(o[i], v[i]);
    }
}


/**
 * \fn void field_vector_random(seedexpander_shake_t* ctx, field_vector_t *o, uint32_t size)
 * \brief This function sets a vector of finite field elements with random values using SHAKE.
 *
 * \param[out] ctx Pointer to a seedexpander_shake_t structure
 * \param[out] v field_vector_t* Representation of vector v
 * \param[in] size Size of the vector
 */
void field_vector_random(seedexpander_shake_t* ctx, field_vector_t *o, uint32_t size) {
    uint8_t random[size * RYDE_FIELD_BYTES];

    field_vector_set_to_zero(o, size);
    seedexpander_shake_get_bytes(ctx, random, size * RYDE_FIELD_BYTES);

    for(size_t i = 0 ; i < size ; ++i) {
        random[(i + 1) * RYDE_FIELD_BYTES - 1] &= RYDE_FIELD_MASK;
        memcpy(o[i], random + i * RYDE_FIELD_BYTES, RYDE_FIELD_BYTES);
    }
}


/**
 * \fn uint32_t field_vector_gauss(field_vector_t *v, uint32_t size, uint8_t reduced_flag, field_vector_t **other_matrices, uint32_t nMatrices)
 * \brief This function transform a vector of finite field elements to its row echelon form and returns its rank.
 *
 * Replicates linear operations on the nMatrices matrices indexed by other_matrices.
 *
 * \param[in] v field_vector_t* Representation of vector v
 * \param[in] size Size of the vector
 * \param[in] reduced_flag If set, the function computes the reduced row echelon form
 * \param[in] other_matrices Pointer to other matrices to replicate the operations on
 * \param[in] nMatrices Number of other matrices
 *
 * \return Rank of the vector <b>v</b>
 */
uint32_t field_vector_gauss(field_vector_t *v, uint32_t size, uint8_t reduced_flag, field_vector_t **other_matrices, uint32_t nMatrices) {
    uint32_t dimension = 0;
    field_t tmp, zero;
    uint8_t mask;
    field_set_to_zero(zero);

    //For each column
    for(uint32_t p = 0 ; p < size ; p++) {
        field_t acc;
        field_copy(acc, v[p]);
        for(uint32_t i=p+1 ; i<size ; i++) {
            for(uint32_t j=0 ; j<RYDE_FIELD_WORDS ; j++) {
                acc[j] |= v[i][j];
            }
        }

        int column = field_get_degree(acc);
        column += (column < 0);

        dimension += (!field_is_zero(acc));

        //For each line below
        for(uint32_t i=p+1 ; i < size ; i++) {
            mask = field_get_coefficient(v[p], column);
            field_cmov(tmp, zero, v[i], mask);
            field_add(v[p], v[p], tmp);

            for(uint32_t k=0 ; k<nMatrices ; k++) {
                field_cmov(tmp, zero, other_matrices[k][i], mask);
                field_add(other_matrices[k][p], other_matrices[k][p], tmp);
            }
        }

        //For each other line
        if(reduced_flag) {
            for(uint32_t i=0 ; i < p ; i++) {
                mask = field_get_coefficient(v[i], column);
                field_cmov(tmp, v[p], zero, mask);
                field_add(v[i], v[i], tmp);

                for(uint32_t k=0 ; k<nMatrices ; k++) {
                    field_cmov(tmp, other_matrices[k][p], zero, mask);
                    field_add(other_matrices[k][i], other_matrices[k][i], tmp);
                }
            }
        }

        for(uint32_t i=p+1 ; i < size ; i++) {
            mask = field_get_coefficient(v[i], column);
            field_cmov(tmp, v[p], zero, mask);
            field_add(v[i], v[i], tmp);

            for(uint32_t k=0 ; k<nMatrices ; k++) {
                field_cmov(tmp, other_matrices[k][p], zero, mask);
                field_add(other_matrices[k][i], other_matrices[k][i], tmp);
            }
        }
    }

    return dimension;
}


/**
 * \fn uint32_t field_vector_get_rank(const field_vector_t *v, uint32_t size)
 * \brief This function computes the rank of a vector of finite field elements.
 *
 * \param[in] v field_vector_t* Representation of vector v
 * \param[in] size Size of the vector
 *
 * \return Rank of the vector <b>v</b>
 */
uint32_t field_vector_get_rank(const field_vector_t *v, uint32_t size) {
    field_vector_t copy[size];
    uint32_t dimension;

    field_vector_copy(copy, v, size);
    dimension = field_vector_gauss(copy, size, 0, NULL, 0);

    return dimension;
}


/**
 * \fn void field_vector_random_full_rank_with_one(seedexpander_shake_t* ctx, field_vector_t *o, uint32_t size)
 * \brief This function sets a full-rank vector of finite field elements with random values using SHAKE. The output vectors has one in the last coordinate.
 *
 * \param[out] ctx Seed expander
 * \param[out] o field_vector_t* Representation of vector o
 * \param[in] size Size of the vector
 */
void field_vector_random_full_rank_with_one(seedexpander_shake_t* ctx, field_vector_t *o, uint32_t size) {
    uint32_t rank_max = RYDE_FIELD_M < size ? RYDE_FIELD_M : size;
    uint32_t rank = 0xFFFFFFFF;

    while (is_nonequal_u32(rank, rank_max)) {
        field_vector_random(ctx, o, size - 1);
        field_set_to_one(o[size - 1]);
        rank = field_vector_get_rank(o, size);
    }
}


/**
 * \fn void field_vector_print(const field_vector_t *v, uint32_t size)
 * \brief Display an field_vector_t element.
 *
 * \param[out] v field_vector_t* Representation of vector v
 * \param[in] size Size of the vector
 */
void field_vector_print(const field_vector_t *v, uint32_t size) {
    printf("[\n");
    for(size_t i = 0 ; i < size ; ++i) {
        field_print(v[i]);
        printf("\n");
    }
    printf("]\n");
}
