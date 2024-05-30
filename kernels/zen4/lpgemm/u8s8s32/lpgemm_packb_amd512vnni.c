/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name(s) of the copyright holder(s) nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <immintrin.h>
#include <string.h>
#include "blis.h"

#ifdef BLIS_ADDON_LPGEMM

void packb_nrlt16_u8s8s32o32_row_major
     (
       int8_t*       pack_b_buffer,
       const int8_t* b,
       const dim_t   rs_b,
       const dim_t   KC,
       const dim_t   n0_partial_rem
     );

void packb_nr16_u8s8s32o32_row_major
     (
       int8_t*       pack_b_buffer,
       const int8_t* b,
       const dim_t   rs_b,
       const dim_t   KC
     );

void packb_nr32_u8s8s32o32_row_major
     (
       int8_t*       pack_b_buffer,
       const int8_t* b,
       const dim_t   rs_b,
       const dim_t   KC
     );

void packb_nr48_u8s8s32o32_row_major
     (
       int8_t*       pack_b_buffer,
       const int8_t* b,
       const dim_t   rs_b,
       const dim_t   KC
     );

void packb_nr64_u8s8s32o32_row_major(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t rs_b,
    const dim_t NC,
    const dim_t KC,
    dim_t *rs_p,
    dim_t *cs_p);

void packb_nr64_u8s8s32o32_col_major(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t rs_b,
    const dim_t NC,
    const dim_t KC,
    dim_t *rs_p,
    dim_t *cs_p);

void packb_nrlt16_u8s8s32o32_col_major(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t rs_b,
    const dim_t KC,
    const dim_t n0_partial_rem);

void packb_nr_mult_16_u8s8s32o32_col_major(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t NR,
    const dim_t ldb,
    const dim_t KC);

void packb_nrlt16_u8s8s32o32_col_major(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t ldb,
    const dim_t KC,
    const dim_t n0_partial_rem);

void packb_nr64_u8s8s32o32
(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t rs_b,
    const dim_t cs_b,
    const dim_t NC,
    const dim_t KC,
    dim_t *rs_p,
    dim_t *cs_p)
{
    if (cs_b == 1)
    {
        packb_nr64_u8s8s32o32_row_major(pack_b_buffer,
                                       b, rs_b, NC, KC, rs_p, cs_p);
    }
    else
    {
        packb_nr64_u8s8s32o32_col_major(pack_b_buffer,
                                        b, cs_b, NC, KC, rs_p, cs_p);
    }
}

void packb_nr64_u8s8s32o32_row_major
    (
        int8_t *pack_b_buffer,
        const int8_t *b,
        const dim_t rs_b,
        const dim_t NC,
        const dim_t KC,
        dim_t *rs_p,
        dim_t *cs_p
    )
{

    dim_t NR = 64;

    // Used for permuting the mm512i elements for use in vpdpbusd instruction.
    // These are indexes of the format a0-a1-b0-b1-a2-a3-b2-b3 and a0-a1-a2-a3-b0-b1-b2-b3.
    // Adding int32 wise all4 gives format a4-a5-b4-b5-a6-a7-b6-b7 and a4-a5-a6-a7-b4-b5-b6-b7.
    __m512i selector1 = _mm512_setr_epi64( 0x0, 0x1, 0x8, 0x9, 0x2, 0x3, 0xA, 0xB );
    __m512i selector1_1 = _mm512_setr_epi64( 0x4, 0x5, 0xC, 0xD, 0x6, 0x7, 0xE, 0xF );
    
    __m512i selector2 = _mm512_setr_epi64( 0x0, 0x1, 0x2, 0x3, 0x8, 0x9, 0xA, 0xB );
    __m512i selector2_1 = _mm512_setr_epi64( 0x4, 0x5, 0x6, 0x7, 0xC, 0xD, 0xE, 0xF );

    dim_t n_full_pieces = NC / NR;
    dim_t n_full_pieces_loop_limit = n_full_pieces * NR;
    dim_t n_partial_pieces = NC % NR;

    dim_t k_full_pieces_blks = KC / 4;
    dim_t k_full_pieces = k_full_pieces_blks * 4;
    dim_t k_partial_pieces = KC % 4;

    // KC when not multiple of 4 will have padding to make it multiple of 4 in packed buffer.
    dim_t KC_updated = KC;
    if ( k_partial_pieces > 0 )
    {
        KC_updated += ( 4 - k_partial_pieces );
    }

    __m512i a0;
    __m512i b0;
    __m512i c0;
    __m512i d0;
    __m512i a01;
    __m512i c01;

    for ( dim_t jc = 0; jc < n_full_pieces_loop_limit; jc += NR )
    {
        for ( dim_t kr = 0; kr < k_full_pieces; kr += 4 )
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a0 = _mm512_loadu_si512( b + ( rs_b * ( kr + 0 ) ) + jc );
            b0 = _mm512_loadu_si512( b + ( rs_b * ( kr + 1 ) ) + jc );
            c0 = _mm512_loadu_si512( b + ( rs_b * ( kr + 2 ) ) + jc );
            d0 = _mm512_loadu_si512( b + ( rs_b * ( kr + 3 ) ) + jc );

            a01 = _mm512_unpacklo_epi8( a0, b0 );
            a0 = _mm512_unpackhi_epi8( a0, b0 );

            c01 = _mm512_unpacklo_epi8( c0, d0 );
            c0 = _mm512_unpackhi_epi8( c0, d0 );

            b0 = _mm512_unpacklo_epi16( a01, c01 );
            a01 = _mm512_unpackhi_epi16( a01, c01 );

            d0 = _mm512_unpacklo_epi16( a0, c0 );
            c01 = _mm512_unpackhi_epi16( a0, c0 );

            a0 = _mm512_permutex2var_epi64( b0, selector1, a01 );
            c0 = _mm512_permutex2var_epi64( d0, selector1, c01 );
            b0 = _mm512_permutex2var_epi64( b0, selector1_1, a01 );
            d0 = _mm512_permutex2var_epi64( d0, selector1_1, c01 );

            a01 = _mm512_permutex2var_epi64( a0, selector2, c0 ); // b[0]
            c01 = _mm512_permutex2var_epi64( b0, selector2, d0 ); // b[2]
            a0 = _mm512_permutex2var_epi64( a0, selector2_1, c0 ); // b[1]
            c0 = _mm512_permutex2var_epi64( b0, selector2_1, d0 ); // b[3]

            _mm512_storeu_si512( pack_b_buffer + 
                                ( ( jc * KC_updated ) + ( ( kr + 0 ) * NR ) ), a01 );
            _mm512_storeu_si512( pack_b_buffer + 
                                ( ( jc * KC_updated ) + ( ( kr + 1 ) * NR ) ) , a0 );
            _mm512_storeu_si512( pack_b_buffer + 
                                ( ( jc * KC_updated ) + ( ( kr + 2 ) * NR ) ), c01 );
            _mm512_storeu_si512( pack_b_buffer + 
                                ( ( jc * KC_updated ) + ( ( kr + 3 ) * NR ) ), c0 );
        }
        // Handle k remainder.
        if ( k_partial_pieces > 0 )
        {
            if ( k_partial_pieces == 3 )
            {
                a0 = _mm512_loadu_si512( b + ( rs_b * ( k_full_pieces + 0 ) ) + jc );
                b0 = _mm512_loadu_si512( b + ( rs_b * ( k_full_pieces + 1 ) ) + jc );
                c0 = _mm512_loadu_si512( b + ( rs_b * ( k_full_pieces + 2 ) ) + jc );
                d0 = _mm512_setzero_si512();

            }
            else if( k_partial_pieces == 2 )
            {
                a0 = _mm512_loadu_si512( b + ( rs_b * ( k_full_pieces + 0 ) ) + jc );
                b0 = _mm512_loadu_si512( b + ( rs_b * ( k_full_pieces + 1 ) ) + jc );
                c0 = _mm512_setzero_si512();
                d0 = _mm512_setzero_si512();
            }
            else //k_partial_pieces == 1
            {
                a0 = _mm512_loadu_si512( b + ( rs_b * ( k_full_pieces + 0 ) ) + jc );
                b0 = _mm512_setzero_si512();
                c0 = _mm512_setzero_si512();
                d0 = _mm512_setzero_si512();
            }

            a01 = _mm512_unpacklo_epi8( a0, b0 );
            a0 = _mm512_unpackhi_epi8( a0, b0 );

            c01 = _mm512_unpacklo_epi8( c0, d0 );
            c0 = _mm512_unpackhi_epi8( c0, d0 );

            b0 = _mm512_unpacklo_epi16( a01, c01 );
            a01 = _mm512_unpackhi_epi16( a01, c01 );

            d0 = _mm512_unpacklo_epi16( a0, c0 );
            c01 = _mm512_unpackhi_epi16( a0, c0 );

            a0 = _mm512_permutex2var_epi64( b0, selector1, a01 );
            c0 = _mm512_permutex2var_epi64( d0, selector1, c01 );
            b0 = _mm512_permutex2var_epi64( b0, selector1_1, a01 );
            d0 = _mm512_permutex2var_epi64( d0, selector1_1, c01 );

            a01 = _mm512_permutex2var_epi64( a0, selector2, c0 ); // b[0]
            c01 = _mm512_permutex2var_epi64( b0, selector2, d0 ); // b[2]
            a0 = _mm512_permutex2var_epi64( a0, selector2_1, c0 ); // b[1]
            c0 = _mm512_permutex2var_epi64( b0, selector2_1, d0 ); // b[3]

            _mm512_storeu_si512( pack_b_buffer + 
                                ( ( jc * KC_updated ) + ( ( k_full_pieces + 0 ) * NR ) ), a01 );
            _mm512_storeu_si512( pack_b_buffer + 
                                ( ( jc * KC_updated ) + ( ( k_full_pieces + 1 ) * NR ) ) , a0 );
            _mm512_storeu_si512( pack_b_buffer +
                                 ( ( jc * KC_updated ) + ( ( k_full_pieces + 2 ) * NR ) ), c01 );
            _mm512_storeu_si512( pack_b_buffer + 
                                ( ( jc * KC_updated ) + ( ( k_full_pieces + 3 ) * NR ) ), c0 );    
        }
    }
    
    // Contiguous packing of fringe panel (n` < NR).
    if ( n_partial_pieces > 0 )
    {
        dim_t n0_partial_rem = n_partial_pieces % 16;
        dim_t n0_partial_pack = 0;

        // Split into multiple smaller fringe kernels, so as to maximize
        // vectorization after packing. Any n0 < NR(64) can be expressed
        // as n0 = 48 + n` / n0 = 32 + n` / n0 = 16 + n`, where n` < 16.
        dim_t n0_48 = n_partial_pieces / 48;
        dim_t n0_32 = n_partial_pieces / 32;
        dim_t n0_16 = n_partial_pieces / 16;

        if ( n0_48 == 1 )
        {
            packb_nr48_u8s8s32o32_row_major
            (
              ( pack_b_buffer + ( n_full_pieces_loop_limit * KC_updated ) ),
              ( b + n_full_pieces_loop_limit ), rs_b, KC
            );

            n0_partial_pack = 48;
        }
        else if ( n0_32 == 1 )
        {
            packb_nr32_u8s8s32o32_row_major
            (
              ( pack_b_buffer + ( n_full_pieces_loop_limit * KC_updated ) ),
              ( b + n_full_pieces_loop_limit ), rs_b, KC
            );

            n0_partial_pack = 32;
        }
        else if ( n0_16 == 1 )
        {
            packb_nr16_u8s8s32o32_row_major
            (
              ( pack_b_buffer + ( n_full_pieces_loop_limit * KC_updated ) ),
              ( b + n_full_pieces_loop_limit ), rs_b, KC
            );

            n0_partial_pack = 16;
        }

        if ( n0_partial_rem > 0 )
        {
            packb_nrlt16_u8s8s32o32_row_major
            (
              ( pack_b_buffer + ( n_full_pieces_loop_limit * KC_updated ) +
                ( n0_partial_pack * KC_updated ) ),
              ( b + n_full_pieces_loop_limit + n0_partial_pack ), rs_b, KC,
              n0_partial_rem
            );
        }
    }
    *rs_p = NR * 4;
    *cs_p = NR;
}

void packb_nr48_u8s8s32o32_row_major
     (
       int8_t*       pack_b_buffer,
       const int8_t* b,
       const dim_t   rs_b,
       const dim_t   KC
     )
{
    dim_t NR = 64;
    dim_t kr_new = 0;

    dim_t k_full_pieces_blks = KC / 4;
    dim_t k_full_pieces = k_full_pieces_blks * 4;
    dim_t k_partial_pieces = KC % 4;

    __m256i a0_32;
    __m256i b0_32;
    __m256i c0_32;
    __m256i d0_32;
    __m256i a01_32;
    __m256i c01_32;
    __m512i a0_zmm;
    __m512i b0_zmm;
    __m128i a0_16;
    __m128i b0_16;
    __m128i c0_16;
    __m128i d0_16;
    __m128i a01_16;
    __m128i c01_16;

    for ( dim_t kr = 0; kr < k_full_pieces; kr += 4 )
    {
        // Rearrange for vpdpbusd, read 4 rows from B with 32 elements in each row.
        a0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, b + ( rs_b * ( kr + 0 ) ) );
        b0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, b + ( rs_b * ( kr + 1 ) ) );
        c0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, b + ( rs_b * ( kr + 2 ) ) );
        d0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, b + ( rs_b * ( kr + 3 ) ) );

        a01_32 = _mm256_unpacklo_epi8( a0_32, b0_32 );
        a0_32 = _mm256_unpackhi_epi8( a0_32, b0_32 );

        c01_32 = _mm256_unpacklo_epi8( c0_32, d0_32 );
        c0_32 = _mm256_unpackhi_epi8( c0_32, d0_32 );

        b0_32 = _mm256_unpacklo_epi16( a01_32, c01_32 );
        a01_32 = _mm256_unpackhi_epi16( a01_32, c01_32 );

        d0_32 = _mm256_unpacklo_epi16( a0_32, c0_32 );
        c01_32 = _mm256_unpackhi_epi16( a0_32, c0_32 );

        a0_32 = _mm256_shuffle_i32x4( b0_32,  a01_32, 0x0 ); // 0 elem
        c0_32 = _mm256_shuffle_i32x4( b0_32,  a01_32, 0x3 ); // 2 elem
        b0_32 = _mm256_shuffle_i32x4( d0_32,  c01_32, 0x0 ); // 1 elem
        d0_32 = _mm256_shuffle_i32x4( d0_32,  c01_32, 0x3 ); // 3 elem

        a0_zmm = _mm512_castsi256_si512( a0_32 );
        a0_zmm = _mm512_inserti32x8( a0_zmm, b0_32, 0x1 );
        b0_zmm = _mm512_castsi256_si512( c0_32 );
        b0_zmm = _mm512_inserti32x8( b0_zmm, d0_32, 0x1 );

        // First 4x32 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 0 ) * NR ), a0_zmm );
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 1 ) * NR ), b0_zmm );

        // Rearrange for vpdpbusd, read 4 rows from B with next 16 elements in each row.
        a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( kr + 0 ) ) + ( 32 ) );
        b0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( kr + 1 ) ) + ( 32 ) );
        c0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( kr + 2 ) ) + ( 32 ) );
        d0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( kr + 3 ) ) + ( 32 ) );

        a01_16 = _mm_unpacklo_epi8( a0_16, b0_16 );
        a0_16 = _mm_unpackhi_epi8( a0_16, b0_16 );

        c01_16 = _mm_unpacklo_epi8( c0_16, d0_16 );
        c0_16 = _mm_unpackhi_epi8( c0_16, d0_16 );

        b0_16 = _mm_unpacklo_epi16( a01_16, c01_16 ); // 0 elem
        a01_16 = _mm_unpackhi_epi16( a01_16, c01_16 ); // 1 elem
        d0_16 = _mm_unpacklo_epi16( a0_16, c0_16 ); // 2 elem
        c01_16 = _mm_unpackhi_epi16( a0_16, c0_16 ); // 3 elem

        a0_zmm = _mm512_castsi128_si512( b0_16 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, a01_16, 0x1 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, d0_16, 0x2 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, c01_16, 0x3 );

        // Last 4x16 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 2 ) * NR ), a0_zmm );

        // The 4th 16byte chunk will be ignored, since its not part of the original data,
        // but is here due to the packing in 4 16byte chunks format.
        kr_new += 3;
    }
    // Handle k remainder.
    if ( k_partial_pieces > 0 )
    {
        if ( k_partial_pieces == 3 )
        {
            a0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 0)));
            b0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 1)));
            c0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 2)));
            d0_32 = _mm256_setzero_si256();

            a0_16 = _mm_maskz_loadu_epi8(0xFFFF,
                                         b + (rs_b * (k_full_pieces + 0)) + (32));
            b0_16 = _mm_maskz_loadu_epi8(0xFFFF,
                                         b + (rs_b * (k_full_pieces + 1)) + (32));
            c0_16 = _mm_maskz_loadu_epi8(0xFFFF,
                                         b + (rs_b * (k_full_pieces + 2)) + (32));
            d0_16 = _mm_setzero_si128();

        }
        else if( k_partial_pieces == 2 )
        {
            a0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 0)));
            b0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 1)));
            c0_32 = _mm256_setzero_si256();
            d0_32 = _mm256_setzero_si256();

            a0_16 = _mm_maskz_loadu_epi8(0xFFFF,
                                         b + (rs_b * (k_full_pieces + 0)) + (32));
            b0_16 = _mm_maskz_loadu_epi8(0xFFFF,
                                         b + (rs_b * (k_full_pieces + 1)) + (32));
            c0_16 = _mm_setzero_si128();
            d0_16 = _mm_setzero_si128();
        }
        else //k_partial_pieces == 1
        {
            a0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 0)));
            b0_32 = _mm256_setzero_si256();
            c0_32 = _mm256_setzero_si256();
            d0_32 = _mm256_setzero_si256();

            a0_16 = _mm_maskz_loadu_epi8(0xFFFF,
                                         b + (rs_b * (k_full_pieces + 0)) + (32));
            b0_16 = _mm_setzero_si128();
            c0_16 = _mm_setzero_si128();
            d0_16 = _mm_setzero_si128();
        }

        a01_32 = _mm256_unpacklo_epi8( a0_32, b0_32 );
        a0_32 = _mm256_unpackhi_epi8( a0_32, b0_32 );

        c01_32 = _mm256_unpacklo_epi8( c0_32, d0_32 );
        c0_32 = _mm256_unpackhi_epi8( c0_32, d0_32 );

        b0_32 = _mm256_unpacklo_epi16( a01_32, c01_32 );
        a01_32 = _mm256_unpackhi_epi16( a01_32, c01_32 );

        d0_32 = _mm256_unpacklo_epi16( a0_32, c0_32 );
        c01_32 = _mm256_unpackhi_epi16( a0_32, c0_32 );

        a0_32 = _mm256_shuffle_i32x4( b0_32,  a01_32, 0x0 ); // 0 elem
        c0_32 = _mm256_shuffle_i32x4( b0_32,  a01_32, 0x3 ); // 2 elem
        b0_32 = _mm256_shuffle_i32x4( d0_32,  c01_32, 0x0 ); // 1 elem
        d0_32 = _mm256_shuffle_i32x4( d0_32,  c01_32, 0x3 ); // 3 elem

        a0_zmm = _mm512_castsi256_si512( a0_32 );
        a0_zmm = _mm512_inserti32x8( a0_zmm, b0_32, 0x1 );
        b0_zmm = _mm512_castsi256_si512( c0_32 );
        b0_zmm = _mm512_inserti32x8( b0_zmm, d0_32, 0x1 );

        // First 4x32 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 0 ) * NR ), a0_zmm );
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 1 ) * NR ), b0_zmm );

        a01_16 = _mm_unpacklo_epi8( a0_16, b0_16 );
        a0_16 = _mm_unpackhi_epi8( a0_16, b0_16 );

        c01_16 = _mm_unpacklo_epi8( c0_16, d0_16 );
        c0_16 = _mm_unpackhi_epi8( c0_16, d0_16 );

        b0_16 = _mm_unpacklo_epi16( a01_16, c01_16 ); // 0 elem
        a01_16 = _mm_unpackhi_epi16( a01_16, c01_16 ); // 1 elem
        d0_16 = _mm_unpacklo_epi16( a0_16, c0_16 ); // 2 elem
        c01_16 = _mm_unpackhi_epi16( a0_16, c0_16 ); // 3 elem

        a0_zmm = _mm512_castsi128_si512( b0_16 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, a01_16, 0x1 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, d0_16, 0x2 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, c01_16, 0x3 );

        // Last 4x16 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 2 ) * NR ), a0_zmm );
    }
}

void packb_nr32_u8s8s32o32_row_major
     (
       int8_t*       pack_b_buffer,
       const int8_t* b,
       const dim_t   rs_b,
       const dim_t   KC
     )
{
    dim_t NR = 64;
    dim_t kr_new = 0;

    dim_t k_full_pieces_blks = KC / 4;
    dim_t k_full_pieces = k_full_pieces_blks * 4;
    dim_t k_partial_pieces = KC % 4;

    __m256i a0_32;
    __m256i b0_32;
    __m256i c0_32;
    __m256i d0_32;
    __m256i a01_32;
    __m256i c01_32;
    __m512i a0_zmm;
    __m512i b0_zmm;

    for ( dim_t kr = 0; kr < k_full_pieces; kr += 4 )
    {
        // Rearrange for vpdpbusd, read 4 rows from B with 32 elements in each row.
        a0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, b + ( rs_b * ( kr + 0 ) ) );
        b0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, b + ( rs_b * ( kr + 1 ) ) );
        c0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, b + ( rs_b * ( kr + 2 ) ) );
        d0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, b + ( rs_b * ( kr + 3 ) ) );

        a01_32 = _mm256_unpacklo_epi8( a0_32, b0_32 );
        a0_32 = _mm256_unpackhi_epi8( a0_32, b0_32 );

        c01_32 = _mm256_unpacklo_epi8( c0_32, d0_32 );
        c0_32 = _mm256_unpackhi_epi8( c0_32, d0_32 );

        b0_32 = _mm256_unpacklo_epi16( a01_32, c01_32 );
        a01_32 = _mm256_unpackhi_epi16( a01_32, c01_32 );

        d0_32 = _mm256_unpacklo_epi16( a0_32, c0_32 );
        c01_32 = _mm256_unpackhi_epi16( a0_32, c0_32 );

        a0_32 = _mm256_shuffle_i32x4( b0_32,  a01_32, 0x0 ); // 0 elem
        c0_32 = _mm256_shuffle_i32x4( b0_32,  a01_32, 0x3 ); // 2 elem
        b0_32 = _mm256_shuffle_i32x4( d0_32,  c01_32, 0x0 ); // 1 elem
        d0_32 = _mm256_shuffle_i32x4( d0_32,  c01_32, 0x3 ); // 3 elem

        a0_zmm = _mm512_castsi256_si512( a0_32 );
        a0_zmm = _mm512_inserti32x8( a0_zmm, b0_32, 0x1 );
        b0_zmm = _mm512_castsi256_si512( c0_32 );
        b0_zmm = _mm512_inserti32x8( b0_zmm, d0_32, 0x1 );

        // First 4x32 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 0 ) * NR ), a0_zmm );
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 1 ) * NR ), b0_zmm );

        // The 3rd and 4th 16byte chunk will be ignored, since its not part of 
        // the original data,but is here due to the packing in 4 16byte chunks format.
        kr_new += 2;
    }
    // Handle k remainder.
    if ( k_partial_pieces > 0 )
    {
        if ( k_partial_pieces == 3 )
        {
            a0_32 = _mm256_maskz_loadu_epi8( 0xFFFFFFFF, 
                                             b + ( rs_b * ( k_full_pieces + 0 ) ) );
            b0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 1)));
            c0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 2)));
            d0_32 = _mm256_setzero_si256();

        }
        else if( k_partial_pieces == 2 )
        {
            a0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 0)));
            b0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 1)));
            c0_32 = _mm256_setzero_si256();
            d0_32 = _mm256_setzero_si256();
        }
        else //k_partial_pieces == 1
        {
            a0_32 = _mm256_maskz_loadu_epi8(0xFFFFFFFF,
                                            b + (rs_b * (k_full_pieces + 0)));
            b0_32 = _mm256_setzero_si256();
            c0_32 = _mm256_setzero_si256();
            d0_32 = _mm256_setzero_si256();
        }

        a01_32 = _mm256_unpacklo_epi8( a0_32, b0_32 );
        a0_32 = _mm256_unpackhi_epi8( a0_32, b0_32 );

        c01_32 = _mm256_unpacklo_epi8( c0_32, d0_32 );
        c0_32 = _mm256_unpackhi_epi8( c0_32, d0_32 );

        b0_32 = _mm256_unpacklo_epi16( a01_32, c01_32 );
        a01_32 = _mm256_unpackhi_epi16( a01_32, c01_32 );

        d0_32 = _mm256_unpacklo_epi16( a0_32, c0_32 );
        c01_32 = _mm256_unpackhi_epi16( a0_32, c0_32 );

        a0_32 = _mm256_shuffle_i32x4( b0_32,  a01_32, 0x0 ); // 0 elem
        c0_32 = _mm256_shuffle_i32x4( b0_32,  a01_32, 0x3 ); // 2 elem
        b0_32 = _mm256_shuffle_i32x4( d0_32,  c01_32, 0x0 ); // 1 elem
        d0_32 = _mm256_shuffle_i32x4( d0_32,  c01_32, 0x3 ); // 3 elem

        a0_zmm = _mm512_castsi256_si512( a0_32 );
        a0_zmm = _mm512_inserti32x8( a0_zmm, b0_32, 0x1 );
        b0_zmm = _mm512_castsi256_si512( c0_32 );
        b0_zmm = _mm512_inserti32x8( b0_zmm, d0_32, 0x1 );

        // First 4x32 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 0 ) * NR ), a0_zmm );
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 1 ) * NR ), b0_zmm );
    }
}

void packb_nr16_u8s8s32o32_row_major
     (
       int8_t*       pack_b_buffer,
       const int8_t* b,
       const dim_t   rs_b,
       const dim_t   KC
     )
{
    dim_t NR = 64;
    dim_t kr_new = 0;

    dim_t k_full_pieces_blks = KC / 4;
    dim_t k_full_pieces = k_full_pieces_blks * 4;
    dim_t k_partial_pieces = KC % 4;

    __m128i a0_16;
    __m128i b0_16;
    __m128i c0_16;
    __m128i d0_16;
    __m128i a01_16;
    __m128i c01_16;
    __m512i a0_zmm;

    for ( dim_t kr = 0; kr < k_full_pieces; kr += 4 )
    {
        // Rearrange for vpdpbusd, read 4 rows from B with next 16 elements in each row.
        a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( kr + 0 ) ) );
        b0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( kr + 1 ) ) );
        c0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( kr + 2 ) ) );
        d0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( kr + 3 ) ) );

        a01_16 = _mm_unpacklo_epi8( a0_16, b0_16 );
        a0_16 = _mm_unpackhi_epi8( a0_16, b0_16 );

        c01_16 = _mm_unpacklo_epi8( c0_16, d0_16 );
        c0_16 = _mm_unpackhi_epi8( c0_16, d0_16 );

        b0_16 = _mm_unpacklo_epi16( a01_16, c01_16 ); // 0 elem
        a01_16 = _mm_unpackhi_epi16( a01_16, c01_16 ); // 1 elem
        d0_16 = _mm_unpacklo_epi16( a0_16, c0_16 ); // 2 elem
        c01_16 = _mm_unpackhi_epi16( a0_16, c0_16 ); // 3 elem

        a0_zmm = _mm512_castsi128_si512( b0_16 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, a01_16, 0x1 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, d0_16, 0x2 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, c01_16, 0x3 );

        // Last 4x16 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 0 ) * NR ), a0_zmm );

        // The 2nd, 3rd, and 4th 16byte chunk will be ignored, since its not part of
        // the original data, but is here due to the packing in 4 16byte chunks format.
        kr_new += 1;
    }
    // Handle k remainder.
    if ( k_partial_pieces > 0 )
    {
        if ( k_partial_pieces == 3 )
        {
            a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( k_full_pieces + 0 ) ) );
            b0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( k_full_pieces + 1 ) ) );
            c0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( k_full_pieces + 2 ) ) );
            d0_16 = _mm_setzero_si128();

        }
        else if( k_partial_pieces == 2 )
        {
            a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( k_full_pieces + 0 ) ) );
            b0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( k_full_pieces + 1 ) ) );
            c0_16 = _mm_setzero_si128();
            d0_16 = _mm_setzero_si128();
        }
        else //k_partial_pieces == 1
        {
            a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, b + ( rs_b * ( k_full_pieces + 0 ) ) );
            b0_16 = _mm_setzero_si128();
            c0_16 = _mm_setzero_si128();
            d0_16 = _mm_setzero_si128();
        }

        a01_16 = _mm_unpacklo_epi8( a0_16, b0_16 );
        a0_16 = _mm_unpackhi_epi8( a0_16, b0_16 );

        c01_16 = _mm_unpacklo_epi8( c0_16, d0_16 );
        c0_16 = _mm_unpackhi_epi8( c0_16, d0_16 );

        b0_16 = _mm_unpacklo_epi16( a01_16, c01_16 ); // 0 elem
        a01_16 = _mm_unpackhi_epi16( a01_16, c01_16 ); // 1 elem
        d0_16 = _mm_unpacklo_epi16( a0_16, c0_16 ); // 2 elem
        c01_16 = _mm_unpackhi_epi16( a0_16, c0_16 ); // 3 elem

        __m512i a0_zmm = _mm512_castsi128_si512( b0_16 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, a01_16, 0x1 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, d0_16, 0x2 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, c01_16, 0x3 );

        // Last 4x16 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 0 ) * NR ), a0_zmm );
    }
}

void packb_nrlt16_u8s8s32o32_row_major
     (
       int8_t*       pack_b_buffer,
       const int8_t* b,
       const dim_t   rs_b,
       const dim_t   KC,
       const dim_t   n0_partial_rem
     )
{
    dim_t NR = 64;

    int8_t buf0[16];
    int8_t buf1[16];
    int8_t buf2[16];
    int8_t buf3[16];

    dim_t kr_new = 0;

    dim_t k_full_pieces_blks = KC / 4;
    dim_t k_full_pieces = k_full_pieces_blks * 4;
    dim_t k_partial_pieces = KC % 4;

    __m128i a0_16;
    __m128i b0_16;
    __m128i c0_16;
    __m128i d0_16;
    __m128i a01_16;
    __m128i c01_16;
    __m512i a0_zmm;

    for ( dim_t kr = 0; kr < k_full_pieces; kr += 4 )
    {
        memcpy( buf0, ( b + ( rs_b * ( kr + 0 ) ) ), 
                ( n0_partial_rem * sizeof( int8_t ) ) );
        memcpy( buf1, ( b + ( rs_b * ( kr + 1 ) ) ), 
                ( n0_partial_rem * sizeof( int8_t ) ) );
        memcpy( buf2, ( b + ( rs_b * ( kr + 2 ) ) ), 
                ( n0_partial_rem * sizeof( int8_t ) ) );
        memcpy( buf3, ( b + ( rs_b * ( kr + 3 ) ) ), 
                ( n0_partial_rem * sizeof( int8_t ) ) );

        // Rearrange for vpdpbusd, read 4 rows from B with next 16 elements in each row.
        a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf0 );
        b0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf1 );
        c0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf2 );
        d0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf3 );

        a01_16 = _mm_unpacklo_epi8( a0_16, b0_16 );
        a0_16 = _mm_unpackhi_epi8( a0_16, b0_16 );

        c01_16 = _mm_unpacklo_epi8( c0_16, d0_16 );
        c0_16 = _mm_unpackhi_epi8( c0_16, d0_16 );

        b0_16 = _mm_unpacklo_epi16( a01_16, c01_16 ); // 0 elem
        a01_16 = _mm_unpackhi_epi16( a01_16, c01_16 ); // 1 elem
        d0_16 = _mm_unpacklo_epi16( a0_16, c0_16 ); // 2 elem
        c01_16 = _mm_unpackhi_epi16( a0_16, c0_16 ); // 3 elem

        a0_zmm = _mm512_castsi128_si512( b0_16 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, a01_16, 0x1 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, d0_16, 0x2 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, c01_16, 0x3 );

        // Last 4x16 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 0 ) * NR ), a0_zmm );

        // The 2nd, 3rd, and 4th 16byte chunk will be ignored, since its not 
        // part of the original data, but is here due to the packing in 4 
        // 16byte chunks format.
        kr_new += 1;
    }
    // Handle k remainder.
    if ( k_partial_pieces > 0 )
    {
        if ( k_partial_pieces == 3 )
        {
            memcpy( buf0, ( b + ( rs_b * ( k_full_pieces + 0 ) ) ), 
                    ( n0_partial_rem * sizeof( int8_t ) ) );
            memcpy( buf1, ( b + ( rs_b * ( k_full_pieces + 1 ) ) ), 
                    ( n0_partial_rem * sizeof( int8_t ) ) );
            memcpy( buf2, ( b + ( rs_b * ( k_full_pieces + 2 ) ) ), 
                    ( n0_partial_rem * sizeof( int8_t ) ) );

            a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf0 );
            b0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf1 );
            c0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf2 );
            d0_16 = _mm_setzero_si128();

        }
        else if( k_partial_pieces == 2 )
        {
            memcpy( buf0, ( b + ( rs_b * ( k_full_pieces + 0 ) ) ), 
                    ( n0_partial_rem * sizeof( int8_t ) ) );
            memcpy( buf1, ( b + ( rs_b * ( k_full_pieces + 1 ) ) ), 
                    ( n0_partial_rem * sizeof( int8_t ) ) );

            a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf0 );
            b0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf1 );
            c0_16 = _mm_setzero_si128();
            d0_16 = _mm_setzero_si128();
        }
        else //k_partial_pieces == 1
        {
            memcpy( buf0, ( b + ( rs_b * ( k_full_pieces + 0 ) ) ), 
                    ( n0_partial_rem * sizeof( int8_t ) ) );

            a0_16 = _mm_maskz_loadu_epi8( 0xFFFF, buf0 );
            b0_16 = _mm_setzero_si128();
            c0_16 = _mm_setzero_si128();
            d0_16 = _mm_setzero_si128();
        }

        a01_16 = _mm_unpacklo_epi8( a0_16, b0_16 );
        a0_16 = _mm_unpackhi_epi8( a0_16, b0_16 );

        c01_16 = _mm_unpacklo_epi8( c0_16, d0_16 );
        c0_16 = _mm_unpackhi_epi8( c0_16, d0_16 );

        b0_16 = _mm_unpacklo_epi16( a01_16, c01_16 ); // 0 elem
        a01_16 = _mm_unpackhi_epi16( a01_16, c01_16 ); // 1 elem
        d0_16 = _mm_unpacklo_epi16( a0_16, c0_16 ); // 2 elem
        c01_16 = _mm_unpackhi_epi16( a0_16, c0_16 ); // 3 elem

        __m512i a0_zmm = _mm512_castsi128_si512( b0_16 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, a01_16, 0x1 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, d0_16, 0x2 );
        a0_zmm = _mm512_inserti32x4( a0_zmm, c01_16, 0x3 );

        // Last 4x16 elements.
        _mm512_storeu_si512( pack_b_buffer + ( ( kr_new + 0 ) * NR ), a0_zmm );
    }
}

#define LOAD_16_COLS_AVX512                                     \
    a_reg[0] = _mm512_loadu_si512(b + (ldb * (jr + 0)) + kr);   \
    a_reg[1] = _mm512_loadu_si512(b + (ldb * (jr + 1)) + kr);   \
    a_reg[2] = _mm512_loadu_si512(b + (ldb * (jr + 2)) + kr);   \
    a_reg[3] = _mm512_loadu_si512(b + (ldb * (jr + 3)) + kr);   \
    a_reg[4] = _mm512_loadu_si512(b + (ldb * (jr + 4)) + kr);   \
    a_reg[5] = _mm512_loadu_si512(b + (ldb * (jr + 5)) + kr);   \
    a_reg[6] = _mm512_loadu_si512(b + (ldb * (jr + 6)) + kr);   \
    a_reg[7] = _mm512_loadu_si512(b + (ldb * (jr + 7)) + kr);   \
    a_reg[8] = _mm512_loadu_si512(b + (ldb * (jr + 8)) + kr);   \
    a_reg[9] = _mm512_loadu_si512(b + (ldb * (jr + 9)) + kr);   \
    a_reg[10] = _mm512_loadu_si512(b + (ldb * (jr + 10)) + kr); \
    a_reg[11] = _mm512_loadu_si512(b + (ldb * (jr + 11)) + kr); \
    a_reg[12] = _mm512_loadu_si512(b + (ldb * (jr + 12)) + kr); \
    a_reg[13] = _mm512_loadu_si512(b + (ldb * (jr + 13)) + kr); \
    a_reg[14] = _mm512_loadu_si512(b + (ldb * (jr + 14)) + kr); \
    a_reg[15] = _mm512_loadu_si512(b + (ldb * (jr + 15)) + kr);

#define UNPACKHILO32_AVX512                                  \
    b_reg[0] = _mm512_unpacklo_epi32(a_reg[0], a_reg[1]);    \
    b_reg[2] = _mm512_unpacklo_epi32(a_reg[2], a_reg[3]);    \
    b_reg[4] = _mm512_unpacklo_epi32(a_reg[4], a_reg[5]);    \
    b_reg[6] = _mm512_unpacklo_epi32(a_reg[6], a_reg[7]);    \
    b_reg[8] = _mm512_unpacklo_epi32(a_reg[8], a_reg[9]);    \
    b_reg[10] = _mm512_unpacklo_epi32(a_reg[10], a_reg[11]); \
    b_reg[12] = _mm512_unpacklo_epi32(a_reg[12], a_reg[13]); \
    b_reg[14] = _mm512_unpacklo_epi32(a_reg[14], a_reg[15]); \
                                                             \
    b_reg[1] = _mm512_unpackhi_epi32(a_reg[0], a_reg[1]);    \
    b_reg[3] = _mm512_unpackhi_epi32(a_reg[2], a_reg[3]);    \
    b_reg[5] = _mm512_unpackhi_epi32(a_reg[4], a_reg[5]);    \
    b_reg[7] = _mm512_unpackhi_epi32(a_reg[6], a_reg[7]);    \
    b_reg[9] = _mm512_unpackhi_epi32(a_reg[8], a_reg[9]);    \
    b_reg[11] = _mm512_unpackhi_epi32(a_reg[10], a_reg[11]); \
    b_reg[13] = _mm512_unpackhi_epi32(a_reg[12], a_reg[13]); \
    b_reg[15] = _mm512_unpackhi_epi32(a_reg[14], a_reg[15]);

#define UNPACKHILO64_AVX512                                  \
    a_reg[0] = _mm512_unpacklo_epi64(b_reg[0], b_reg[2]);    \
    a_reg[1] = _mm512_unpacklo_epi64(b_reg[4], b_reg[6]);    \
    a_reg[2] = _mm512_unpacklo_epi64(b_reg[8], b_reg[10]);   \
    a_reg[3] = _mm512_unpacklo_epi64(b_reg[12], b_reg[14]);  \
    a_reg[4] = _mm512_unpacklo_epi64(b_reg[1], b_reg[3]);    \
    a_reg[5] = _mm512_unpacklo_epi64(b_reg[5], b_reg[7]);    \
    a_reg[6] = _mm512_unpacklo_epi64(b_reg[9], b_reg[11]);   \
    a_reg[7] = _mm512_unpacklo_epi64(b_reg[13], b_reg[15]);  \
                                                             \
    a_reg[8] = _mm512_unpackhi_epi64(b_reg[0], b_reg[2]);    \
    a_reg[9] = _mm512_unpackhi_epi64(b_reg[4], b_reg[6]);    \
    a_reg[10] = _mm512_unpackhi_epi64(b_reg[8], b_reg[10]);  \
    a_reg[11] = _mm512_unpackhi_epi64(b_reg[12], b_reg[14]); \
    a_reg[12] = _mm512_unpackhi_epi64(b_reg[1], b_reg[3]);   \
    a_reg[13] = _mm512_unpackhi_epi64(b_reg[5], b_reg[7]);   \
    a_reg[14] = _mm512_unpackhi_epi64(b_reg[9], b_reg[11]);  \
    a_reg[15] = _mm512_unpackhi_epi64(b_reg[13], b_reg[15]);

#define PERMUTEX2_VAR64_AVX512                                              \
    b_reg[0] = _mm512_permutex2var_epi64(a_reg[0], selector1, a_reg[1]);    \
    b_reg[1] = _mm512_permutex2var_epi64(a_reg[2], selector1, a_reg[3]);    \
    b_reg[2] = _mm512_permutex2var_epi64(a_reg[8], selector1, a_reg[9]);    \
    b_reg[3] = _mm512_permutex2var_epi64(a_reg[10], selector1, a_reg[11]);  \
    b_reg[4] = _mm512_permutex2var_epi64(a_reg[4], selector1, a_reg[5]);    \
    b_reg[5] = _mm512_permutex2var_epi64(a_reg[6], selector1, a_reg[7]);    \
    b_reg[6] = _mm512_permutex2var_epi64(a_reg[12], selector1, a_reg[13]);  \
    b_reg[7] = _mm512_permutex2var_epi64(a_reg[14], selector1, a_reg[15]);  \
    b_reg[8] = _mm512_permutex2var_epi64(a_reg[0], selector2, a_reg[1]);    \
    b_reg[9] = _mm512_permutex2var_epi64(a_reg[2], selector2, a_reg[3]);    \
    b_reg[10] = _mm512_permutex2var_epi64(a_reg[8], selector2, a_reg[9]);   \
    b_reg[11] = _mm512_permutex2var_epi64(a_reg[10], selector2, a_reg[11]); \
    b_reg[12] = _mm512_permutex2var_epi64(a_reg[4], selector2, a_reg[5]);   \
    b_reg[13] = _mm512_permutex2var_epi64(a_reg[6], selector2, a_reg[7]);   \
    b_reg[14] = _mm512_permutex2var_epi64(a_reg[12], selector2, a_reg[13]); \
    b_reg[15] = _mm512_permutex2var_epi64(a_reg[14], selector2, a_reg[15]);

#define SHUFFLE64x2_AVX512                                        \
    a_reg[0] = _mm512_shuffle_i64x2(b_reg[0], b_reg[1], 0x44);    \
    a_reg[1] = _mm512_shuffle_i64x2(b_reg[2], b_reg[3], 0x44);    \
    a_reg[2] = _mm512_shuffle_i64x2(b_reg[4], b_reg[5], 0x44);    \
    a_reg[3] = _mm512_shuffle_i64x2(b_reg[6], b_reg[7], 0x44);    \
    a_reg[4] = _mm512_shuffle_i64x2(b_reg[8], b_reg[9], 0x44);    \
    a_reg[5] = _mm512_shuffle_i64x2(b_reg[10], b_reg[11], 0x44);  \
    a_reg[6] = _mm512_shuffle_i64x2(b_reg[12], b_reg[13], 0x44);  \
    a_reg[7] = _mm512_shuffle_i64x2(b_reg[14], b_reg[15], 0x44);  \
    a_reg[8] = _mm512_shuffle_i64x2(b_reg[0], b_reg[1], 0xEE);    \
    a_reg[9] = _mm512_shuffle_i64x2(b_reg[2], b_reg[3], 0xEE);    \
    a_reg[10] = _mm512_shuffle_i64x2(b_reg[4], b_reg[5], 0xEE);   \
    a_reg[11] = _mm512_shuffle_i64x2(b_reg[6], b_reg[7], 0xEE);   \
    a_reg[12] = _mm512_shuffle_i64x2(b_reg[8], b_reg[9], 0xEE);   \
    a_reg[13] = _mm512_shuffle_i64x2(b_reg[10], b_reg[11], 0xEE); \
    a_reg[14] = _mm512_shuffle_i64x2(b_reg[12], b_reg[13], 0xEE); \
    a_reg[15] = _mm512_shuffle_i64x2(b_reg[14], b_reg[15], 0xEE);

#define MASK_LOAD_16_COLS_AVX512(mask)                                      \
    a_reg[0] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 0)) + kr);   \
    a_reg[1] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 1)) + kr);   \
    a_reg[2] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 2)) + kr);   \
    a_reg[3] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 3)) + kr);   \
    a_reg[4] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 4)) + kr);   \
    a_reg[5] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 5)) + kr);   \
    a_reg[6] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 6)) + kr);   \
    a_reg[7] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 7)) + kr);   \
    a_reg[8] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 8)) + kr);   \
    a_reg[9] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 9)) + kr);   \
    a_reg[10] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 10)) + kr); \
    a_reg[11] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 11)) + kr); \
    a_reg[12] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 12)) + kr); \
    a_reg[13] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 13)) + kr); \
    a_reg[14] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 14)) + kr); \
    a_reg[15] = _mm512_maskz_loadu_epi8(mask, b + (ldb * (jr + 15)) + kr);


void packb_nr64_u8s8s32o32_col_major(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t ldb,
    const dim_t NC,
    const dim_t KC,
    dim_t *rs_p,
    dim_t *cs_p)
{
    dim_t NR = 64;

    dim_t n_full_pieces = NC / NR;
    dim_t n_full_pieces_loop_limit = n_full_pieces * NR;
    dim_t n_partial_pieces = NC % NR;

    dim_t k_partial_pieces = KC % 4;

    dim_t KC_updated = KC;
    if (k_partial_pieces > 0)
    {
        KC_updated += (4 - k_partial_pieces);
    }

    for (dim_t jc = 0; jc < n_full_pieces_loop_limit; jc += NR)
    {
        packb_nr_mult_16_u8s8s32o32_col_major(pack_b_buffer + (jc * KC_updated),
                                              b + (jc * ldb), 64, ldb, KC);
    }

    if (n_partial_pieces > 0)
    {
        dim_t n0_partial_rem = n_partial_pieces % 16;
        dim_t n0_partial_pack = 0;

        // Split into multiple smaller fringe kernels, so as to maximize
        // vectorization after packing. Any n0 < NR(64) can be expressed
        // as n0 = 48 + n` / n0 = 32 + n` / n0 = 16 + n`, where n` < 16.
        dim_t n0_48 = n_partial_pieces / 48;
        dim_t n0_32 = n_partial_pieces / 32;
        dim_t n0_16 = n_partial_pieces / 16;

        if (n0_48 == 1)
        {
            packb_nr_mult_16_u8s8s32o32_col_major(
                (pack_b_buffer + (n_full_pieces_loop_limit * KC_updated)),
                (b + n_full_pieces_loop_limit * ldb), 48, ldb, KC);

            n0_partial_pack = 48;
        }
        else if (n0_32 == 1)
        {
            packb_nr_mult_16_u8s8s32o32_col_major(
                (pack_b_buffer + (n_full_pieces_loop_limit * KC_updated)),
                (b + n_full_pieces_loop_limit * ldb), 32, ldb, KC);

            n0_partial_pack = 32;
        }
        else if (n0_16 == 1)
        {
            packb_nr_mult_16_u8s8s32o32_col_major(
                (pack_b_buffer + (n_full_pieces_loop_limit * KC_updated)),
                (b + n_full_pieces_loop_limit * ldb), 16, ldb, KC);

            n0_partial_pack = 16;
        }

        if (n0_partial_rem > 0)
        {
            packb_nrlt16_u8s8s32o32_col_major(
                (pack_b_buffer + (n_full_pieces_loop_limit * KC_updated) +
                 (n0_partial_pack * KC_updated)),
                (b + (n_full_pieces_loop_limit + n0_partial_pack) * ldb), ldb, KC,
                n0_partial_rem);
        }
    }

    *rs_p = NR * 4;
    *cs_p = NR / 4;
}

void packb_nr_mult_16_u8s8s32o32_col_major(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t NR,
    const dim_t ldb,
    const dim_t KC)
{
    // Used for permuting the mm512i elements for use in vpdpbusd instruction.
    __m512i selector1 = _mm512_setr_epi64(0x0, 0x1, 0x8, 0x9, 0x4, 0x5, 0xC, 0xD);
    __m512i selector2 = _mm512_setr_epi64(0x2, 0x3, 0xA, 0xB, 0x6, 0x7, 0xE, 0xF);

    __m512i a_reg[16];
    __m512i b_reg[16];

    dim_t kr = 0;
    for (kr = 0; (kr + 63) < KC; kr += 64)
    {
        for (dim_t jr = 0; jr < NR; jr += 16)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            LOAD_16_COLS_AVX512
            UNPACKHILO32_AVX512
            UNPACKHILO64_AVX512
            PERMUTEX2_VAR64_AVX512
            SHUFFLE64x2_AVX512

            // store to pack_b buffer
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 0) * NR), a_reg[0]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 4) * NR), a_reg[1]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 8) * NR), a_reg[2]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 12) * NR), a_reg[3]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 16) * NR), a_reg[4]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 20) * NR), a_reg[5]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 24) * NR), a_reg[6]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 28) * NR), a_reg[7]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 32) * NR), a_reg[8]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 36) * NR), a_reg[9]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 40) * NR), a_reg[10]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 44) * NR), a_reg[11]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 48) * NR), a_reg[12]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 52) * NR), a_reg[13]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 56) * NR), a_reg[14]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 60) * NR), a_reg[15]);
        }
    }

    for (; (kr + 31) < KC; kr += 32)
    {
        for (dim_t jr = 0; jr < NR; jr += 16)
        {
            //  Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            MASK_LOAD_16_COLS_AVX512((__mmask64 )0xFFFFFFFF)
            UNPACKHILO32_AVX512
            UNPACKHILO64_AVX512
            PERMUTEX2_VAR64_AVX512
            SHUFFLE64x2_AVX512

            // store to pack_b buffer
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 0) * NR), a_reg[0]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 4) * NR), a_reg[1]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 8) * NR), a_reg[2]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 12) * NR), a_reg[3]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 16) * NR), a_reg[4]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 20) * NR), a_reg[5]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 24) * NR), a_reg[6]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 28) * NR), a_reg[7]);
        }
    }

    for (; (kr + 15) < KC; kr += 16)
    {
        for (dim_t jr = 0; jr < NR; jr += 16)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            MASK_LOAD_16_COLS_AVX512((__mmask64)0xFFFF)
            UNPACKHILO32_AVX512
            UNPACKHILO64_AVX512
            PERMUTEX2_VAR64_AVX512
            SHUFFLE64x2_AVX512

            // store to pack_b buffer
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 0) * NR), a_reg[0]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 4) * NR), a_reg[1]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 8) * NR), a_reg[2]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 12) * NR), a_reg[3]);
        }
    }

    for (; (kr + 7) < KC; kr += 8)
    {
        for (dim_t jr = 0; jr < NR; jr += 16)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            MASK_LOAD_16_COLS_AVX512((__mmask64)0xFF)
            UNPACKHILO32_AVX512
            UNPACKHILO64_AVX512
            PERMUTEX2_VAR64_AVX512
            SHUFFLE64x2_AVX512

            // store to pack_b buffer
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 0) * NR), a_reg[0]);
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + ((kr + 4) * NR), a_reg[1]);
        }
    }

    for (; (kr + 3) < KC; kr += 4)
    {
        for (dim_t jr = 0; jr < NR; jr += 16)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            MASK_LOAD_16_COLS_AVX512((__mmask64)0x0F)
            UNPACKHILO32_AVX512
            UNPACKHILO64_AVX512
            PERMUTEX2_VAR64_AVX512
            SHUFFLE64x2_AVX512

            // store to pack_b buffer
            _mm512_storeu_si512(pack_b_buffer + (jr * 4) + (kr * NR), a_reg[0]);
        }
    }

    for (; (kr + 2) < KC; kr += 3)
    {
        for (dim_t jr = 0; jr < NR; jr += 16)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            MASK_LOAD_16_COLS_AVX512((__mmask64)0x07)
            UNPACKHILO32_AVX512
            UNPACKHILO64_AVX512
            PERMUTEX2_VAR64_AVX512
            SHUFFLE64x2_AVX512

            // store to pack_b buffer
            _mm512_storeu_si512((pack_b_buffer + (jr * 4) + (kr * NR)), a_reg[0]);
        }
    }

    for (; (kr + 1) < KC; kr += 2)
    {
        for (dim_t jr = 0; jr < NR; jr += 16)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            MASK_LOAD_16_COLS_AVX512((__mmask64)0x03)
            UNPACKHILO32_AVX512
            UNPACKHILO64_AVX512
            PERMUTEX2_VAR64_AVX512
            SHUFFLE64x2_AVX512

            // store to pack_b buffer
            _mm512_storeu_si512((pack_b_buffer + (jr * 4) + (kr * NR)), a_reg[0]);
        }
    }

    for (; kr < KC; kr += 1)
    {
        for (dim_t jr = 0; jr < NR; jr += 16)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            MASK_LOAD_16_COLS_AVX512((__mmask64)0x01)
            UNPACKHILO32_AVX512
            UNPACKHILO64_AVX512
            PERMUTEX2_VAR64_AVX512
            SHUFFLE64x2_AVX512

            // store to pack_b buffer
            _mm512_storeu_si512((pack_b_buffer + (jr * 4) + (kr * NR)), a_reg[0]);
        }
    }
}

void packb_nrlt16_u8s8s32o32_col_major(
    int8_t *pack_b_buffer,
    const int8_t *b,
    const dim_t ldb,
    const dim_t KC,
    const dim_t n0_partial_rem)
{
    dim_t NR = 16;

    // Used for permuting the mm512i elements for use in vpdpbusd instruction.
    __m512i selector1 = _mm512_setr_epi64(0x0, 0x1, 0x8, 0x9, 0x4, 0x5, 0xC, 0xD);
    __m512i selector2 = _mm512_setr_epi64(0x2, 0x3, 0xA, 0xB, 0x6, 0x7, 0xE, 0xF);

    __m512i a_reg[16];
    __m512i b_reg[16];

    dim_t kr = 0, jr = 0;
    for (kr = 0; (kr + 63) < KC; kr += 64)
    {
        for (jr = 0; jr < n0_partial_rem; jr += 1)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a_reg[jr] = _mm512_loadu_si512(b + (ldb * (jr + 0)) + kr);
        }

        for (; jr < NR; jr++)
        {
            a_reg[jr] = _mm512_setzero_si512();
        }

        UNPACKHILO32_AVX512
        UNPACKHILO64_AVX512
        PERMUTEX2_VAR64_AVX512
        SHUFFLE64x2_AVX512

        _mm512_storeu_si512(pack_b_buffer + ((kr + 0) * NR), a_reg[0]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 4) * NR), a_reg[1]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 8) * NR), a_reg[2]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 12) * NR), a_reg[3]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 16) * NR), a_reg[4]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 20) * NR), a_reg[5]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 24) * NR), a_reg[6]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 28) * NR), a_reg[7]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 32) * NR), a_reg[8]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 36) * NR), a_reg[9]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 40) * NR), a_reg[10]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 44) * NR), a_reg[11]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 48) * NR), a_reg[12]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 52) * NR), a_reg[13]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 56) * NR), a_reg[14]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 60) * NR), a_reg[15]);
    }

    for (; (kr + 31) < KC; kr += 32)
    {
        for (jr = 0; jr < n0_partial_rem; jr += 1)
        {
            //  Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a_reg[jr] = _mm512_maskz_loadu_epi8(0xFFFFFFFF, b + (ldb * (jr + 0)) + kr);
        }

        for (; jr < NR; jr++)
        {
            a_reg[jr] = _mm512_setzero_si512();
        }

        UNPACKHILO32_AVX512
        UNPACKHILO64_AVX512
        PERMUTEX2_VAR64_AVX512
        SHUFFLE64x2_AVX512

        _mm512_storeu_si512(pack_b_buffer + ((kr + 0) * NR), a_reg[0]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 4) * NR), a_reg[1]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 8) * NR), a_reg[2]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 12) * NR), a_reg[3]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 16) * NR), a_reg[4]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 20) * NR), a_reg[5]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 24) * NR), a_reg[6]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 28) * NR), a_reg[7]);
    }

    for (; (kr + 15) < KC; kr += 16)
    {
        for (jr = 0; jr < n0_partial_rem; jr += 1)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a_reg[jr] = _mm512_maskz_loadu_epi8(0xFFFF, b + (ldb * (jr + 0)) + kr);
        }

        for (; jr < NR; jr++)
        {
            a_reg[jr] = _mm512_setzero_si512();
        }

        UNPACKHILO32_AVX512
        UNPACKHILO64_AVX512
        PERMUTEX2_VAR64_AVX512
        SHUFFLE64x2_AVX512

        _mm512_storeu_si512(pack_b_buffer + ((kr + 0) * NR), a_reg[0]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 4) * NR), a_reg[1]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 8) * NR), a_reg[2]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 12) * NR), a_reg[3]);
    }

    for (; (kr + 7) < KC; kr += 8)
    {
        for (jr = 0; jr < n0_partial_rem; jr += 1)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a_reg[jr] = _mm512_maskz_loadu_epi8(0xFF, b + (ldb * (jr + 0)) + kr);
        }

        for (; jr < NR; jr++)
        {
            a_reg[jr] = _mm512_setzero_si512();
        }

        UNPACKHILO32_AVX512
        UNPACKHILO64_AVX512
        PERMUTEX2_VAR64_AVX512
        SHUFFLE64x2_AVX512

        _mm512_storeu_si512(pack_b_buffer + ((kr + 0) * NR), a_reg[0]);
        _mm512_storeu_si512(pack_b_buffer + ((kr + 4) * NR), a_reg[1]);
    }

    for (; (kr + 3) < KC; kr += 4)
    {
        for (jr = 0; jr < n0_partial_rem; jr += 1)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a_reg[jr] = _mm512_maskz_loadu_epi8(0x0F, b + (ldb * (jr + 0)) + kr);
        }

        for (; jr < NR; jr++)
        {
            a_reg[jr] = _mm512_setzero_si512();
        }

        UNPACKHILO32_AVX512
        UNPACKHILO64_AVX512
        PERMUTEX2_VAR64_AVX512
        SHUFFLE64x2_AVX512

        _mm512_storeu_si512(pack_b_buffer + ((kr + 0) * NR), a_reg[0]);
    }

    for (; (kr + 2) < KC; kr += 3)
    {
        for (jr = 0; jr < n0_partial_rem; jr += 1)
        {
            //  Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a_reg[jr] = _mm512_maskz_loadu_epi8(0x07, b + (ldb * (jr + 0)) + kr);
        }

        for (; jr < NR; jr++)
        {
            a_reg[jr] = _mm512_setzero_si512();
        }
        UNPACKHILO32_AVX512
        UNPACKHILO64_AVX512
        PERMUTEX2_VAR64_AVX512
        SHUFFLE64x2_AVX512

        _mm512_storeu_si512(pack_b_buffer + ((kr + 0) * NR), a_reg[0]);
    }

    for (; (kr + 1) < KC; kr += 2)
    {
        for (jr = 0; jr < n0_partial_rem; jr += 1)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a_reg[jr] = _mm512_maskz_loadu_epi8(0x03, b + (ldb * (jr + 0)) + kr);
        }

        for (; jr < NR; jr++)
        {
            a_reg[jr] = _mm512_setzero_si512();
        }
        UNPACKHILO32_AVX512
        UNPACKHILO64_AVX512
        PERMUTEX2_VAR64_AVX512
        SHUFFLE64x2_AVX512

        _mm512_storeu_si512(pack_b_buffer + ((kr + 0) * NR), a_reg[0]);
    }

    for (; kr < KC; kr += 1)
    {
        for (jr = 0; jr < n0_partial_rem; jr += 1)
        {
            // Rearrange for vpdpbusd, read 4 rows from B with 64 elements in each row.
            a_reg[jr] = _mm512_maskz_loadu_epi8(0x01, b + (ldb * (jr + 0)) + kr);
        }

        for (; jr < NR; jr++)
        {
            a_reg[jr] = _mm512_setzero_si512();
        }

        UNPACKHILO32_AVX512
        UNPACKHILO64_AVX512
        PERMUTEX2_VAR64_AVX512
        SHUFFLE64x2_AVX512

        _mm512_storeu_si512(pack_b_buffer + ((kr + 0) * NR), a_reg[0]);
    }
}

#endif
