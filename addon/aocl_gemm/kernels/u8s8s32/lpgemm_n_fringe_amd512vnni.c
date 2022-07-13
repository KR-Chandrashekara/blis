/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.

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
#include "lpgemm_n_fringe.h"
#include "lpgemm_mn_fringe.h"

// 6xlt16 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_6xlt16
     (
       const dim_t    m0,
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const dim_t    ps_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta,
       const dim_t    n0_rem
     )
{
	dim_t MR = 6;
	dim_t m_full_pieces = m0 / MR;
	dim_t m_full_pieces_loop_limit = m_full_pieces * MR;
	dim_t m_partial_pieces = m0 % MR;

	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint32_t a_kfringe_buf = 0;

	// For corner cases.
	int32_t buf0[16];
	int32_t buf1[16];
	int32_t buf2[16];
	int32_t buf3[16];
	int32_t buf4[16];
	int32_t buf5[16];

	for ( dim_t ir = 0; ir < m_full_pieces_loop_limit; ir += MR )
	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();

		__m512i c_int32_1p0 = _mm512_setzero_epi32();

		__m512i c_int32_2p0 = _mm512_setzero_epi32();
		
		__m512i c_int32_3p0 = _mm512_setzero_epi32();

		__m512i c_int32_4p0 = _mm512_setzero_epi32();

		__m512i c_int32_5p0 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			// Load 4 rows with 16 extended elements each from B to 1 ZMM
			// registers. It is to be noted that the B matrix is packed for use
			// in vnni instructions and each load to ZMM register will have 4
			// elements along k direction and 16 elements across n directions,
			// so 4x16 elements to a ZMM register.
			__m512i b0 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			
			// Broadcast a[1,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			
			// Broadcast a[2,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			
			// Broadcast a[3,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			
			// Broadcast a[4,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 4 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			
			// Broadcast a[5,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 5 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-15] = a[5,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m512i b0 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			__m512i a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			
			// Broadcast a[1,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			
			// Broadcast a[2,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			
			// Broadcast a[3,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			
			// Broadcast a[4,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			
			// Broadcast a[5,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 5 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-15] = a[5,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
		}

		// Load alpha and beta
		__m512i selector1 = _mm512_set1_epi32( alpha );
		__m512i selector2 = _mm512_set1_epi32( beta );

		// Scale by alpha
		c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );

		c_int32_1p0 = _mm512_mullo_epi32( selector1, c_int32_1p0 );
		
		c_int32_2p0 = _mm512_mullo_epi32( selector1, c_int32_2p0 );
		
		c_int32_3p0 = _mm512_mullo_epi32( selector1, c_int32_3p0 );
		
		c_int32_4p0 = _mm512_mullo_epi32( selector1, c_int32_4p0 );
		
		c_int32_5p0 = _mm512_mullo_epi32( selector1, c_int32_5p0 );

		// Scale C by beta.
		if ( beta != 0 )
		{
			memcpy( buf0, ( c + ( rs_c * ( ir + 0 ) ) ), ( n0_rem * sizeof( int32_t ) ) );
			memcpy( buf1, ( c + ( rs_c * ( ir + 1 ) ) ), ( n0_rem * sizeof( int32_t ) ) );
			memcpy( buf2, ( c + ( rs_c * ( ir + 2 ) ) ), ( n0_rem * sizeof( int32_t ) ) );
			memcpy( buf3, ( c + ( rs_c * ( ir + 3 ) ) ), ( n0_rem * sizeof( int32_t ) ) );
			memcpy( buf4, ( c + ( rs_c * ( ir + 4 ) ) ), ( n0_rem * sizeof( int32_t ) ) );
			memcpy( buf5, ( c + ( rs_c * ( ir + 5 ) ) ), ( n0_rem * sizeof( int32_t ) ) );
			
			// c[0,0-15]
			selector1 = _mm512_loadu_epi32( buf0 );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			selector1 = _mm512_loadu_epi32( buf1 );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			selector1 = _mm512_loadu_epi32( buf2 );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[3,0-15]
			selector1 = _mm512_loadu_epi32( buf3 );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			// c[4,0-15]
			selector1 = _mm512_loadu_epi32( buf4 );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_4p0 = _mm512_add_epi32( selector1, c_int32_4p0 );

			// c[5,0-15]
			selector1 = _mm512_loadu_epi32( buf5  );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_5p0 = _mm512_add_epi32( selector1, c_int32_5p0 );
		}
		
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_epi32( buf0, c_int32_0p0 );

		// c[1,0-15]
		_mm512_storeu_epi32( buf1, c_int32_1p0 );

		// c[2,0-15]
		_mm512_storeu_epi32( buf2, c_int32_2p0 );

		// c[3,0-15]
		_mm512_storeu_epi32( buf3, c_int32_3p0 );

		// c[4,0-15]
		_mm512_storeu_epi32( buf4, c_int32_4p0 );

		// c[5,0-15]
		_mm512_storeu_epi32( buf5, c_int32_5p0 );

		// Memcpy partial parts.
		// c[0,0-15]
		memcpy( c + ( rs_c * ( ir + 0 ) ) + ( 0*16 ), buf0, ( n0_rem * sizeof( int32_t ) ) );

		// c[1,0-15]
		memcpy( c + ( rs_c * ( ir + 1 ) ) + ( 0*16 ), buf1, ( n0_rem * sizeof( int32_t ) ) );

		// c[2,0-15]
		memcpy( c + ( rs_c * ( ir + 2 ) ) + ( 0*16 ), buf2, ( n0_rem * sizeof( int32_t ) ) );

		// c[3,0-15]
		memcpy( c + ( rs_c * ( ir + 3 ) ) + ( 0*16 ), buf3, ( n0_rem * sizeof( int32_t ) ) );

		// c[4,0-15]
		memcpy( c + ( rs_c * ( ir + 4 ) ) + ( 0*16 ), buf4, ( n0_rem * sizeof( int32_t ) ) );

		// c[5,0-15]
		memcpy( c + ( rs_c * ( ir + 5 ) ) + ( 0*16 ), buf5, ( n0_rem * sizeof( int32_t ) ) );

		a = a + ( MR * ps_a );
	}

	if ( m_partial_pieces > 0 )
	{
		if ( m_partial_pieces == 5 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 5 );
			lpgemm_rowvar_u8s8s32o32_5xlt16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta, n0_rem
			);
		}
		else if ( m_partial_pieces == 4 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 4 );
			lpgemm_rowvar_u8s8s32o32_4xlt16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta, n0_rem
			);
		}
		else if ( m_partial_pieces == 3 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 3 );
			lpgemm_rowvar_u8s8s32o32_3xlt16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta, n0_rem
			);
		}
		else if ( m_partial_pieces == 2 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 2 );
			lpgemm_rowvar_u8s8s32o32_2xlt16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta, n0_rem
			);
		}
		else if ( m_partial_pieces == 1 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 1 );
			lpgemm_rowvar_u8s8s32o32_1xlt16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta, n0_rem
			);
		}
	}
}

// 6x16 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_6x16
     (
       const dim_t    m0,
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const dim_t    ps_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     )
{
	dim_t MR = 6;
	dim_t m_full_pieces = m0 / MR;
	dim_t m_full_pieces_loop_limit = m_full_pieces * MR;
	dim_t m_partial_pieces = m0 % MR;

	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint32_t a_kfringe_buf = 0;

	for ( dim_t ir = 0; ir < m_full_pieces_loop_limit; ir += MR )
	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();

		__m512i c_int32_1p0 = _mm512_setzero_epi32();

		__m512i c_int32_2p0 = _mm512_setzero_epi32();
		
		__m512i c_int32_3p0 = _mm512_setzero_epi32();

		__m512i c_int32_4p0 = _mm512_setzero_epi32();

		__m512i c_int32_5p0 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			// Load 4 rows with 16 elements each from B to 1 ZMM registers. It
			// is to be noted that the B matrix is packed for use in vnni
			// instructions and each load to ZMM register will have 4 elements
			// along k direction and 16 elements across n directions, so 4x16
			// elements to a ZMM register.
			__m512i b0 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			
			// Broadcast a[1,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			
			// Broadcast a[2,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			
			// Broadcast a[3,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			
			// Broadcast a[4,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 4 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			
			// Broadcast a[5,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 5 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-15] = a[5,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m512i b0 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			__m512i a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			
			// Broadcast a[1,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			
			// Broadcast a[2,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			
			// Broadcast a[3,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			
			// Broadcast a[4,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			
			// Broadcast a[5,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 5 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-15] = a[5,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
		}

		// Load alpha and beta
		__m512i selector1 = _mm512_set1_epi32( alpha );
		__m512i selector2 = _mm512_set1_epi32( beta );

		// Scale by alpha
		c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );

		c_int32_1p0 = _mm512_mullo_epi32( selector1, c_int32_1p0 );
		
		c_int32_2p0 = _mm512_mullo_epi32( selector1, c_int32_2p0 );
		
		c_int32_3p0 = _mm512_mullo_epi32( selector1, c_int32_3p0 );
		
		c_int32_4p0 = _mm512_mullo_epi32( selector1, c_int32_4p0 );
		
		c_int32_5p0 = _mm512_mullo_epi32( selector1, c_int32_5p0 );

		// Scale C by beta.
		if ( beta != 0 )
		{
			// c[0,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[3,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			// c[4,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_4p0 = _mm512_add_epi32( selector1, c_int32_4p0 );

			// c[5,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_5p0 = _mm512_add_epi32( selector1, c_int32_5p0 );
		}
		
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 0*16 ), c_int32_0p0 );

		// c[1,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 0*16 ), c_int32_1p0 );

		// c[2,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 0*16 ), c_int32_2p0 );

		// c[3,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 0*16 ), c_int32_3p0 );

		// c[4,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 0*16 ), c_int32_4p0 );

		// c[5,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 0*16 ), c_int32_5p0 );

		a = a + ( MR * ps_a );
	}

	if ( m_partial_pieces > 0 )
	{
		if ( m_partial_pieces == 5 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 5 );
			lpgemm_rowvar_u8s8s32o32_5x16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 4 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 4 );
			lpgemm_rowvar_u8s8s32o32_4x16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 3 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 3 );
			lpgemm_rowvar_u8s8s32o32_3x16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 2 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 2 );
			lpgemm_rowvar_u8s8s32o32_2x16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 1 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 1 );
			lpgemm_rowvar_u8s8s32o32_1x16
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
	}
}

// 6x32 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_6x32
     (
       const dim_t    m0,
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const dim_t    ps_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     )
{
	dim_t MR = 6;
	dim_t m_full_pieces = m0 / MR;
	dim_t m_full_pieces_loop_limit = m_full_pieces * MR;
	dim_t m_partial_pieces = m0 % MR;

	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint32_t a_kfringe_buf = 0;

	for ( dim_t ir = 0; ir < m_full_pieces_loop_limit; ir += MR )
	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();
		__m512i c_int32_0p1 = _mm512_setzero_epi32();

		__m512i c_int32_1p0 = _mm512_setzero_epi32();
		__m512i c_int32_1p1 = _mm512_setzero_epi32();

		__m512i c_int32_2p0 = _mm512_setzero_epi32();
		__m512i c_int32_2p1 = _mm512_setzero_epi32();
		
		__m512i c_int32_3p0 = _mm512_setzero_epi32();
		__m512i c_int32_3p1 = _mm512_setzero_epi32();

		__m512i c_int32_4p0 = _mm512_setzero_epi32();
		__m512i c_int32_4p1 = _mm512_setzero_epi32();

		__m512i c_int32_5p0 = _mm512_setzero_epi32();
		__m512i c_int32_5p1 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			// Load 4 rows with 32 elements each from B to 2 ZMM registers. It
			// is to be noted that the B matrix is packed for use in vnni
			// instructions and each load to ZMM register will have 4 elements
			// along k direction and 16 elements across n directions, so 4x16
			// elements to a ZMM register.
			__m512i b0 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 0 ) );
			__m512i b1 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 1 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
			
			// Broadcast a[1,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
			
			// Broadcast a[2,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
			
			// Broadcast a[3,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-31] = a[3,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
			
			// Broadcast a[4,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 4 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-31] = a[4,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
			
			// Broadcast a[5,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 5 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-31] = a[5,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
			c_int32_5p1 = _mm512_dpbusd_epi32( c_int32_5p1, a_int32_0, b1 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m512i b0 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
			__m512i b1 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );

			// Broadcast a[0,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			__m512i a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
			
			// Broadcast a[1,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
			
			// Broadcast a[2,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
			
			// Broadcast a[3,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-31] = a[3,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
			
			// Broadcast a[4,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-31] = a[4,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
			
			// Broadcast a[5,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 5 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-31] = a[5,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
			c_int32_5p1 = _mm512_dpbusd_epi32( c_int32_5p1, a_int32_0, b1 );
		}

		// Load alpha and beta
		__m512i selector1 = _mm512_set1_epi32( alpha );
		__m512i selector2 = _mm512_set1_epi32( beta );

		// Scale by alpha
		c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );
		c_int32_0p1 = _mm512_mullo_epi32( selector1, c_int32_0p1 );

		c_int32_1p0 = _mm512_mullo_epi32( selector1, c_int32_1p0 );
		c_int32_1p1 = _mm512_mullo_epi32( selector1, c_int32_1p1 );
		
		c_int32_2p0 = _mm512_mullo_epi32( selector1, c_int32_2p0 );
		c_int32_2p1 = _mm512_mullo_epi32( selector1, c_int32_2p1 );
		
		c_int32_3p0 = _mm512_mullo_epi32( selector1, c_int32_3p0 );
		c_int32_3p1 = _mm512_mullo_epi32( selector1, c_int32_3p1 );
		
		c_int32_4p0 = _mm512_mullo_epi32( selector1, c_int32_4p0 );
		c_int32_4p1 = _mm512_mullo_epi32( selector1, c_int32_4p1 );
		
		c_int32_5p0 = _mm512_mullo_epi32( selector1, c_int32_5p0 );
		c_int32_5p1 = _mm512_mullo_epi32( selector1, c_int32_5p1 );

		// Scale C by beta.
		if ( beta != 0 )
		{
			// c[0,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[0, 16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_0p1 = _mm512_add_epi32( selector1, c_int32_0p1 );

			// c[1,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[1,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_1p1 = _mm512_add_epi32( selector1, c_int32_1p1 );

			// c[2,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[2,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_2p1 = _mm512_add_epi32( selector1, c_int32_2p1 );

			// c[3,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			// c[3,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_3p1 = _mm512_add_epi32( selector1, c_int32_3p1 );

			// c[4,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_4p0 = _mm512_add_epi32( selector1, c_int32_4p0 );

			// c[4,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_4p1 = _mm512_add_epi32( selector1, c_int32_4p1 );

			// c[5,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_5p0 = _mm512_add_epi32( selector1, c_int32_5p0 );

			// c[5,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_5p1 = _mm512_add_epi32( selector1, c_int32_5p1 );
		}
		
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 1*16 ), c_int32_0p1 );

		// c[1,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 1*16 ), c_int32_1p1 );

		// c[2,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 0*16 ), c_int32_2p0 );

		// c[2,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 1*16 ), c_int32_2p1 );

		// c[3,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 0*16 ), c_int32_3p0 );

		// c[3,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 1*16 ), c_int32_3p1 );

		// c[4,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 0*16 ), c_int32_4p0 );

		// c[4,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 1*16 ), c_int32_4p1 );

		// c[5,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 0*16 ), c_int32_5p0 );

		// c[5,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 1*16 ), c_int32_5p1 );

		a = a + ( MR * ps_a );
	}

	if ( m_partial_pieces > 0 )
	{
		if ( m_partial_pieces == 5 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 5 );
			lpgemm_rowvar_u8s8s32o32_5x32
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 4 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 4 );
			lpgemm_rowvar_u8s8s32o32_4x32
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 3 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 3 );
			lpgemm_rowvar_u8s8s32o32_3x32
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 2 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 2 );
			lpgemm_rowvar_u8s8s32o32_2x32
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 1 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 1 );
			lpgemm_rowvar_u8s8s32o32_1x32
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
	}
}

// 6x48 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_6x48
     (
       const dim_t    m0,
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const dim_t    ps_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     )
{
	dim_t MR = 6;
	dim_t m_full_pieces = m0 / MR;
	dim_t m_full_pieces_loop_limit = m_full_pieces * MR;
	dim_t m_partial_pieces = m0 % MR;

	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint32_t a_kfringe_buf = 0;

	for ( dim_t ir = 0; ir < m_full_pieces_loop_limit; ir += MR )
	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();
		__m512i c_int32_0p1 = _mm512_setzero_epi32();
		__m512i c_int32_0p2 = _mm512_setzero_epi32();

		__m512i c_int32_1p0 = _mm512_setzero_epi32();
		__m512i c_int32_1p1 = _mm512_setzero_epi32();
		__m512i c_int32_1p2 = _mm512_setzero_epi32();

		__m512i c_int32_2p0 = _mm512_setzero_epi32();
		__m512i c_int32_2p1 = _mm512_setzero_epi32();
		__m512i c_int32_2p2 = _mm512_setzero_epi32();
		
		__m512i c_int32_3p0 = _mm512_setzero_epi32();
		__m512i c_int32_3p1 = _mm512_setzero_epi32();
		__m512i c_int32_3p2 = _mm512_setzero_epi32();

		__m512i c_int32_4p0 = _mm512_setzero_epi32();
		__m512i c_int32_4p1 = _mm512_setzero_epi32();
		__m512i c_int32_4p2 = _mm512_setzero_epi32();

		__m512i c_int32_5p0 = _mm512_setzero_epi32();
		__m512i c_int32_5p1 = _mm512_setzero_epi32();
		__m512i c_int32_5p2 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			// Load 4 rows with 48 elements each from B to 3 ZMM registers. It
			// is to be noted that the B matrix is packed for use in vnni
			// instructions and each load to ZMM register will have 4 elements
			// along k direction and 16 elements across n directions, so 4x16
			// elements to a ZMM register.
			__m512i b0 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 0 ) );
			__m512i b1 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 1 ) );
			__m512i b2 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 2 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
			c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );
			
			// Broadcast a[1,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
			c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );
			
			// Broadcast a[2,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
			c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );
			
			// Broadcast a[3,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-47] = a[3,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
			c_int32_3p2 = _mm512_dpbusd_epi32( c_int32_3p2, a_int32_0, b2 );
			
			// Broadcast a[4,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 4 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-47] = a[4,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
			c_int32_4p2 = _mm512_dpbusd_epi32( c_int32_4p2, a_int32_0, b2 );
			
			// Broadcast a[5,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 5 ) + ( cs_a * kr ) ) );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-47] = a[5,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
			c_int32_5p1 = _mm512_dpbusd_epi32( c_int32_5p1, a_int32_0, b1 );
			c_int32_5p2 = _mm512_dpbusd_epi32( c_int32_5p2, a_int32_0, b2 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m512i b0 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
			__m512i b1 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );
			__m512i b2 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 2 ) );

			// Broadcast a[0,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			__m512i a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
			c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );
			
			// Broadcast a[1,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
			c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );
			
			// Broadcast a[2,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
			c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );
			
			// Broadcast a[3,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-47] = a[3,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
			c_int32_3p2 = _mm512_dpbusd_epi32( c_int32_3p2, a_int32_0, b2 );
			
			// Broadcast a[4,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-47] = a[4,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
			c_int32_4p2 = _mm512_dpbusd_epi32( c_int32_4p2, a_int32_0, b2 );
			
			// Broadcast a[5,kr:kr+4].
			memcpy
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 5 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces * sizeof( uint8_t ) )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-47] = a[5,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
			c_int32_5p1 = _mm512_dpbusd_epi32( c_int32_5p1, a_int32_0, b1 );
			c_int32_5p2 = _mm512_dpbusd_epi32( c_int32_5p2, a_int32_0, b2 );
		}

		// Load alpha and beta
		__m512i selector1 = _mm512_set1_epi32( alpha );
		__m512i selector2 = _mm512_set1_epi32( beta );

		// Scale by alpha
		c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );
		c_int32_0p1 = _mm512_mullo_epi32( selector1, c_int32_0p1 );
		c_int32_0p2 = _mm512_mullo_epi32( selector1, c_int32_0p2 );

		c_int32_1p0 = _mm512_mullo_epi32( selector1, c_int32_1p0 );
		c_int32_1p1 = _mm512_mullo_epi32( selector1, c_int32_1p1 );
		c_int32_1p2 = _mm512_mullo_epi32( selector1, c_int32_1p2 );
		
		c_int32_2p0 = _mm512_mullo_epi32( selector1, c_int32_2p0 );
		c_int32_2p1 = _mm512_mullo_epi32( selector1, c_int32_2p1 );
		c_int32_2p2 = _mm512_mullo_epi32( selector1, c_int32_2p2 );
		
		c_int32_3p0 = _mm512_mullo_epi32( selector1, c_int32_3p0 );
		c_int32_3p1 = _mm512_mullo_epi32( selector1, c_int32_3p1 );
		c_int32_3p2 = _mm512_mullo_epi32( selector1, c_int32_3p2 );
		
		c_int32_4p0 = _mm512_mullo_epi32( selector1, c_int32_4p0 );
		c_int32_4p1 = _mm512_mullo_epi32( selector1, c_int32_4p1 );
		c_int32_4p2 = _mm512_mullo_epi32( selector1, c_int32_4p2 );
		
		c_int32_5p0 = _mm512_mullo_epi32( selector1, c_int32_5p0 );
		c_int32_5p1 = _mm512_mullo_epi32( selector1, c_int32_5p1 );
		c_int32_5p2 = _mm512_mullo_epi32( selector1, c_int32_5p2 );

		// Scale C by beta.
		if ( beta != 0 )
		{
			// c[0,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[0, 16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_0p1 = _mm512_add_epi32( selector1, c_int32_0p1 );

			// c[0,32-47]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 2*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_0p2 = _mm512_add_epi32( selector1, c_int32_0p2 );

			// c[1,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[1,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_1p1 = _mm512_add_epi32( selector1, c_int32_1p1 );

			// c[1,32-47]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 2*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_1p2 = _mm512_add_epi32( selector1, c_int32_1p2 );

			// c[2,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[2,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_2p1 = _mm512_add_epi32( selector1, c_int32_2p1 );

			// c[2,32-47]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 2*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_2p2 = _mm512_add_epi32( selector1, c_int32_2p2 );

			// c[3,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			// c[3,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_3p1 = _mm512_add_epi32( selector1, c_int32_3p1 );

			// c[3,32-47]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 2*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_3p2 = _mm512_add_epi32( selector1, c_int32_3p2 );

			// c[4,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_4p0 = _mm512_add_epi32( selector1, c_int32_4p0 );

			// c[4,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_4p1 = _mm512_add_epi32( selector1, c_int32_4p1 );

			// c[4,32-47]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 2*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_4p2 = _mm512_add_epi32( selector1, c_int32_4p2 );

			// c[5,0-15]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 0*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_5p0 = _mm512_add_epi32( selector1, c_int32_5p0 );

			// c[5,16-31]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 1*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_5p1 = _mm512_add_epi32( selector1, c_int32_5p1 );

			// c[5,32-47]
			selector1 = _mm512_loadu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 2*16 ) );
			selector1 = _mm512_mullo_epi32( selector2, selector1 );
			c_int32_5p2 = _mm512_add_epi32( selector1, c_int32_5p2 );
		}
		
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 1*16 ), c_int32_0p1 );

		// c[0,32-47]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 0 ) ) + ( 2*16 ), c_int32_0p2 );

		// c[1,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 1*16 ), c_int32_1p1 );

		// c[1,32-47]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 1 ) ) + ( 2*16 ), c_int32_1p2 );

		// c[2,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 0*16 ), c_int32_2p0 );

		// c[2,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 1*16 ), c_int32_2p1 );

		// c[2,32-47]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 2 ) ) + ( 2*16 ), c_int32_2p2 );

		// c[3,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 0*16 ), c_int32_3p0 );

		// c[3,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 1*16 ), c_int32_3p1 );

		// c[3,32-47]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 3 ) ) + ( 2*16 ), c_int32_3p2 );

		// c[4,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 0*16 ), c_int32_4p0 );

		// c[4,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 1*16 ), c_int32_4p1 );

		// c[4,32-47]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 4 ) ) + ( 2*16 ), c_int32_4p2 );

		// c[5,0-15]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 0*16 ), c_int32_5p0 );

		// c[5,16-31]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 1*16 ), c_int32_5p1 );

		// c[5,32-47]
		_mm512_storeu_epi32( c + ( rs_c * ( ir + 5 ) ) + ( 2*16 ), c_int32_5p2 );

		a = a + ( MR * ps_a );
	}

	if ( m_partial_pieces > 0 )
	{
		if ( m_partial_pieces == 5 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 5 );
			lpgemm_rowvar_u8s8s32o32_5x48
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 4 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 4 );
			lpgemm_rowvar_u8s8s32o32_4x48
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 3 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 3 );
			lpgemm_rowvar_u8s8s32o32_3x48
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 2 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 2 );
			lpgemm_rowvar_u8s8s32o32_2x48
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
		else if ( m_partial_pieces == 1 )
		{
			dim_t cs_a_use = ( cs_a == 4 ) ? 4 : ( ( cs_a / 6 ) * 1 );
			lpgemm_rowvar_u8s8s32o32_1x48
			(
			  k0,
			  a, rs_a, cs_a_use,
			  b, rs_b, cs_b,
			  ( c + ( rs_c * m_full_pieces_loop_limit ) ), rs_c,
			  alpha, beta
			);
		}
	}
}