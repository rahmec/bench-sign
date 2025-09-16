/**
 * @file rijndael_avx.h
 * @brief Content for rijndael_avx.h (AES-128 and Rijndael-256 optimized implementation)
 */

#ifndef RIJNDAEL_AVX_H
#define RIJNDAEL_AVX_H

#include <string.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <wmmintrin.h>

#define AES128_ROUNDS 10
#define RIJNDAEL256_ROUNDS 14

static inline __m128i aes_128_assist(__m128i temp1, __m128i temp2) {
    __m128i temp3;
    temp2 = _mm_shuffle_epi32(temp2, 0xff);
    temp3 = _mm_slli_si128(temp1, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp3 = _mm_slli_si128(temp3, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp3 = _mm_slli_si128(temp3, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp1 = _mm_xor_si128(temp1, temp2);
    return temp1;
}

static inline void aes_128_key_expansion(unsigned char *round_keys, const unsigned char *key)
{
    __m128i temp1, temp2;
    __m128i *Key_Schedule = (__m128i *)round_keys;

    temp1 = _mm_loadu_si128((__m128i *)key);
    Key_Schedule[0] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x1);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[1] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x2);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[2] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x4);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[3] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x8);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[4] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x10);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[5] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x20);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[6] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x40);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[7] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x80);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[8] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x1b);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[9] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x36);
    temp1 = aes_128_assist(temp1, temp2);
    Key_Schedule[10] = temp1;
}

static inline void aes_128_encrypt(unsigned char *out, const unsigned char *in, const unsigned char *Key_Schedule) {
    __m128i *KS = (__m128i *)Key_Schedule;
    __m128i data = _mm_loadu_si128(&((__m128i*)in)[0]);

    data = _mm_xor_si128(data, KS[0]);

    int j;
    for (j = 1; j < AES128_ROUNDS; j++) {
        data = _mm_aesenc_si128(data, KS[j]);
    }

    ((__m128i *)out)[0] = _mm_aesenclast_si128(data, KS[j]);
}

static inline __m128i load_high_128(const __m256i* block)
{
    __m128i out;
    memcpy(&out, ((unsigned char*) block) + sizeof(__m128i), sizeof(out));
    return out;
}

static inline void rijndael_256_assist(
        const __m256i* round_key_in, __m128i temp1, __m256i* round_key_out)
{
    __m128i t1, t2, t3, t4;

    memcpy(&t1, round_key_in, sizeof(t1));
    t3 = load_high_128(round_key_in);
    t2 = temp1;

    t2 = _mm_shuffle_epi32(t2, 0xff);
    t4 = _mm_slli_si128(t1, 0x4);
    t1 = _mm_xor_si128(t1, t4);
    t4 = _mm_slli_si128(t4, 0x4);
    t1 = _mm_xor_si128(t1, t4);
    t4 = _mm_slli_si128(t4, 0x4);
    t1 = _mm_xor_si128(t1, t4);
    t1 = _mm_xor_si128(t1, t2);

    memcpy(round_key_out, &t1, sizeof(t1));

    t4 = _mm_aeskeygenassist_si128(t1, 0x00);
    t2 = _mm_shuffle_epi32(t4, 0xaa);
    t4 = _mm_slli_si128(t3, 0x4);
    t3 = _mm_xor_si128(t3, t4);
    t4 = _mm_slli_si128(t4, 0x4);
    t3 = _mm_xor_si128(t3, t4);
    t4 = _mm_slli_si128(t4, 0x4);
    t3 = _mm_xor_si128(t3, t4);
    t3 = _mm_xor_si128(t3, t2);

    memcpy(((unsigned char*) round_key_out) + sizeof(t1), &t3, sizeof(t3));
}

static inline void rijndael_256_key_expansion(unsigned char *round_keys, const unsigned char *key)
{
    __m256i *Key_Schedule = (__m256i *)round_keys;

    __m128i temp1;
    __m256i temp2;
    temp2 = _mm256_load_si256((__m256i *)key);
    Key_Schedule[0] = temp2;
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[0]), 0x01);
    rijndael_256_assist(&Key_Schedule[0], temp1, &Key_Schedule[1]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[1]), 0x02);
    rijndael_256_assist(&Key_Schedule[1], temp1, &Key_Schedule[2]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[2]), 0x04);
    rijndael_256_assist(&Key_Schedule[2], temp1, &Key_Schedule[3]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[3]), 0x08);
    rijndael_256_assist(&Key_Schedule[3], temp1, &Key_Schedule[4]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[4]), 0x10);
    rijndael_256_assist(&Key_Schedule[4], temp1, &Key_Schedule[5]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[5]), 0x20);
    rijndael_256_assist(&Key_Schedule[5], temp1, &Key_Schedule[6]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[6]), 0x40);
    rijndael_256_assist(&Key_Schedule[6], temp1, &Key_Schedule[7]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[7]), 0x80);
    rijndael_256_assist(&Key_Schedule[7], temp1, &Key_Schedule[8]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[8]), 0x1B);
    rijndael_256_assist(&Key_Schedule[8], temp1, &Key_Schedule[9]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[9]), 0x36);
    rijndael_256_assist(&Key_Schedule[9], temp1, &Key_Schedule[10]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[10]), 0x6C);
    rijndael_256_assist(&Key_Schedule[10], temp1, &Key_Schedule[11]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[11]), 0xD8);
    rijndael_256_assist(&Key_Schedule[11], temp1, &Key_Schedule[12]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[12]), 0xAB);
    rijndael_256_assist(&Key_Schedule[12], temp1, &Key_Schedule[13]);
    temp1 = _mm_aeskeygenassist_si128(load_high_128(&Key_Schedule[13]), 0x4D);
    rijndael_256_assist(&Key_Schedule[13], temp1, &Key_Schedule[14]);
}

static inline void rijndael_256_encrypt (unsigned char *out, const unsigned char *in, const unsigned char *Key_Schedule) {
    __m128i tmp1, tmp2, data1 ,data2;
    __m128i RIJNDAEL256_MASK = _mm_set_epi32(0x03020d0c, 0x0f0e0908, 0x0b0a0504, 0x07060100);
    __m128i BLEND_MASK = _mm_set_epi32(0x80000000, 0x80800000, 0x80800000, 0x80808000);
    __m128i *KS = (__m128i *)Key_Schedule;
    int j;
    data1 = _mm_loadu_si128(&((__m128i*)in)[0]); /* load data block */
    data2 = _mm_loadu_si128(&((__m128i*)in)[1]);
    data1 = _mm_xor_si128(data1, KS[0]); /* round 0 (initial xor) */
    data2 = _mm_xor_si128(data2, KS[1]);

    /* Do number_of_rounds-1 AES rounds */
    for(j=1; j < RIJNDAEL256_ROUNDS; j++) {
        /*Blend to compensate for the shift rows shifts bytes between two
        128 bit blocks*/
        tmp1 = _mm_blendv_epi8(data1, data2, BLEND_MASK);
        tmp2 = _mm_blendv_epi8(data2, data1, BLEND_MASK);
        /*Shuffle that compensates for the additional shift in rows 3 and 4
        as opposed to rijndael128 (AES)*/
        tmp1 = _mm_shuffle_epi8(tmp1, RIJNDAEL256_MASK);
        tmp2 = _mm_shuffle_epi8(tmp2, RIJNDAEL256_MASK);
        /*This is the encryption step that includes sub bytes, shift rows,
        mix columns, xor with round key*/
        data1 = _mm_aesenc_si128(tmp1, KS[j*2]);
        data2 = _mm_aesenc_si128(tmp2, KS[j*2+1]);
    }
    tmp1 = _mm_blendv_epi8(data1, data2, BLEND_MASK);
    tmp2 = _mm_blendv_epi8(data2, data1, BLEND_MASK);
    tmp1 = _mm_shuffle_epi8(tmp1, RIJNDAEL256_MASK);
    tmp2 = _mm_shuffle_epi8(tmp2, RIJNDAEL256_MASK);
    tmp1 = _mm_aesenclast_si128(tmp1, KS[j*2+0]); /*last AES round */
    tmp2 = _mm_aesenclast_si128(tmp2, KS[j*2+1]);
    _mm_storeu_si128(&((__m128i*)out)[0],tmp1);
    _mm_storeu_si128(&((__m128i*)out)[1],tmp2);
}

#endif //RIJNDAEL_AVX_H
