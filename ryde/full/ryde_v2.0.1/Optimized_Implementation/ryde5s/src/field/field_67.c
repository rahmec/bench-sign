/**
 * \file field.c
 * \brief Implementation of field.h
 */


 #include <stdio.h>
 #include <inttypes.h>
 
 #include "field_67.h"
 #include "field_ur_67.h"
 
 
/**
 * \fn void field_add(field_t o, const field_t e1, const field_t e2)
 * \brief This function adds two finite field elements.
 *
 * \param[out] o Sum of <b>e1</b> and <b>e2</b>
 * \param[in] e1 field_t
 * \param[in] e2 field_t
 */
void field_add(field_t o, const field_t e1, const field_t e2) {
    for(uint8_t i = 0 ; i < RYDE_FIELD_WORDS ; i++) {
        o[i] = e1[i] ^ e2[i];
    }
}

 
/**
 * \fn static inline void field_redc(field_t o, const field_ur_t e)
 * \brief This function reduces a finite field element.
 *
 * \param[out] o field_t equal to $ e \pmod f $
 * \param[in] e field_ur_t
 */
static inline void field_redc(field_t o, const field_ur_t e) {
    uint64_t tmp = (e[1] >> 62) ^ (e[2] << 2);
    o[1] = e[1] ^ tmp ^ (tmp >> 3) ^ (tmp >> 4) ^ (tmp >> 5);

    tmp = (o[1] >> 3) ^ (e[2] << 61);
    o[0] = e[0] ^ tmp ^ (tmp << 1) ^ (tmp << 2) ^ (tmp << 5);

    o[1] &= 0x0000000000000007;
}
 
 
/**
 * \fn void field_mul(field_t o, const field_t e1, const field_t e2)
 * \brief This function multiplies two finite field elements.
 *
 * \param[out] o Product of <b>e1</b> and <b>e2</b>
 * \param[in] e1 field_t
 * \param[in] e2 field_t
 */
void field_mul(field_t o, const field_t e1, const field_t e2) {
    field_ur_t tmp;
    field_ur_mul(tmp, e1, e2);
    field_redc(o, tmp);
}
 
 
/**
 * \fn void field_sqr(field_t o, const field_t e)
 * \brief This function computes the square of a finite field element.
 *
 * \param[out] o Square of <b>e</b>
 * \param[in] e field_t
 */
void field_sqr(field_t o, const field_t e) {
    field_ur_t tmp;
    field_ur_sqr(tmp, e);
    field_redc(o, tmp);
}
 
 
/**
 * \fn void field_set_to_zero(field_t o)
 * \brief This function sets a finite field element to zero.
 *
 * \param[out] o field_t
 */
    void field_set_to_zero(field_t o) {
    for(uint32_t i = 0; i < RYDE_FIELD_WORDS; i++) {
        o[i] = 0;
    }
}
 
 
/**
 * \fn void field_set_to_one(field_t o)
 * \brief This function sets a finite field element to one.
 *
 * \param[out] o field_t
 */
void field_set_to_one(field_t o) {
    o[0] = 1;
    for(uint32_t i = 1; i < RYDE_FIELD_WORDS; i++) {
        o[i] = 0;
    }
}
  
   
/**
 * \fn void field_copy(field_t o, const field_t e)
 * \brief This function copies a finite field element into another one.
 *
 * \param[out] o field_t
 * \param[in] e field_t
 */
void field_copy(field_t o, const field_t e) {
    for(uint32_t i = 0; i < RYDE_FIELD_WORDS; i++) {
        o[i] = e[i];
    }
}

/**
 * \fn void field_cmov(field_t o, const field_t e1, const field_t e2, uint8_t mask)
 * \brief This function copies either e1 or e2 into o depending on the mask value
 *
 * \param[out] o field_t
 * \param[in] e1 field_t
 * \param[in] e2 field_t
 * \param[in] mask 1 to copy e1 and 0 to copy e2
 */
void field_cmov(field_t o, const field_t e1, const field_t e2, uint8_t mask) {
    for(uint8_t i = 0 ; i < RYDE_FIELD_WORDS ; i++) {
        cmove_u64(&o[i], mask, e1[i], e2[i]);
    }
}


/**
 * \fn uint8_t field_is_zero(field_t field_t e)
 * \brief This function tests if a finite field element is equal to zero.
 * 
 * \param[in] e field_t
 * \return 1 if <b>e</b> is equal to zero, 0 otherwise
 */
uint8_t field_is_zero(const field_t e) {
    uint8_t result = 1;
    for(uint8_t i = 0 ; i < RYDE_FIELD_WORDS ; i++) {
        result &= is_zero_u64(e[i]);
    }
    return result;
}


/**
 * \fn uint8_t field_is_equal(const field_t e1, const field_t e2)
 * \brief This function tests if two finite field elements are equal.
 *
 * \param[in] e1 field_t
 * \param[in] e2 field_t
 * \return 1 if <b>e1</b> and <b>e2</b> are equal, 0 otherwise
 */
uint8_t field_is_equal(const field_t e1, const field_t e2) {
    uint64_t r = 0;
    for(uint8_t i = 0 ; i < RYDE_FIELD_WORDS ; i++) {
        r |= e1[i] ^ e2[i];
    }
    r = (-r);
    r = r >> 63;
    return (uint8_t)(1 - r);
}


/**
 * \fn int32_t field_get_degree(const field_t e)
 * \brief This function returns the degree of a finite field element.
 *
 * \param[in] e field_t
 * \return Degree of <b>e</b> 
 */
int32_t field_get_degree(const field_t e) {
    int64_t index = 0;
    uint64_t result = 0;
    int8_t mask = 0;

    for(uint8_t i = 0 ; i < RYDE_FIELD_WORDS ; i++) {
        __asm__ volatile("bsr %1,%0;" : "=r"(index) : "r"(e[i]));
        mask = is_nonzero_u64(e[i]);
        cmove_u64(&result, mask, index + 64 * i, result);
    }

    return (int32_t)result;
}


/**
 * \fn uint8_t field_get_coefficient(const field_t e, uint32_t index)
 * \brief This function returns the coefficient of the polynomial <b>e</b> at a given index.
 *
 * \param[in] e field_t
 * \param[in] index Index of the coefficient
 * \return Coefficient of <b>e</b> at the given index
 */
uint8_t field_get_coefficient(const field_t e, uint32_t index) {
    uint64_t w = 0;

    for(uint8_t i = 0 ; i < RYDE_FIELD_WORDS ; i++) {
        w |= -is_zero_u64((uint64_t)(i ^ (index >> 6))) & e[i];
    }

    return (w >> (index & 63)) & 1;
}

/**
 * \fn void field_set_coefficient(field_t o, uint32_t index, uint8_t bit)
 * \brief This function set a coefficient of the polynomial <b>e</b>.
 *
 * \param[in] e field_t
 * \param[in] index Index of the coefficient
 * \param[in] bit Value of the coefficient
 */
void field_set_coefficient(field_t o, uint32_t index, uint8_t bit) {
    size_t position = index / 64;
    o[position] |= (uint64_t) bit << (index % 64);
}


/**
 * \fn void field_to_string(uint8_t* str, const field_t e)
 * \brief This function parses a finite field element into a string.
 *
 * \param[out] str Byte string representation of the field element
 * \param[in] e field_t
 */
void field_to_string(uint8_t* str, const field_t e) {
    uint32_t bytes1 = RYDE_FIELD_M / 8;
    uint32_t bytes2 = RYDE_FIELD_M % 8;

    memset(str, 0, RYDE_FIELD_BYTES);
    memcpy(str, e, bytes1);

    uint8_t k = 0;
    for(size_t j = 1 ; j <= bytes2 ; j++) {
        uint8_t bit = field_get_coefficient(e, RYDE_FIELD_M - j);
        *(str + bytes1) |= (bit << k % 8);
        k++;
        if(k % 8 == 0) { bytes1++; }
    }
}

/**
 * \fn void field_print(const field_t e)
 * \brief This function displays a finite field element.
 *
 * \param[in] e field_t
 */
void field_print(const field_t e) {
    printf("[");
    for(uint8_t i = 0 ; i < RYDE_FIELD_WORDS ; i++) {
        printf(" %16" PRIx64 , e[i]);
    }
    printf(" ]");
}
