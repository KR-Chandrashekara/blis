/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2022-23, Advanced Micro Devices, Inc. All rights reserved.

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

#include "lpgemm_s32_kern_macros.h"
#include "lpgemm_s32_memcpy_macros.h"

// 6xlt16 int8o32 fringe kernel
LPGEMM_N_LT_NR0_FRINGE_KERN(uint8_t,int8_t,int32_t,u8s8s32o32_6xlt16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_6xLT16_DISABLE,
						  &&POST_OPS_BIAS_6xLT16,
						  &&POST_OPS_RELU_6xLT16,
						  &&POST_OPS_RELU_SCALE_6xLT16,
						  &&POST_OPS_GELU_TANH_6xLT16,
						  &&POST_OPS_GELU_ERF_6xLT16,
						  &&POST_OPS_DOWNSCALE_6xLT16
						};
	dim_t MR = 6;
	dim_t m_full_pieces = m0 / MR;
	dim_t m_full_pieces_loop_limit = m_full_pieces * MR;
	dim_t m_partial_pieces = m0 % MR;

	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint32_t a_kfringe_buf = 0;

	// B matrix storage.
	__m512i b0;

	// A matrix storage.
	__m512i a_int32_0;

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
			b0 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

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
			b0 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

			// Broadcast a[2,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

			// Broadcast a[3,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );

			// Broadcast a[4,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );

			// Broadcast a[5,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 5 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-15] = a[5,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
		}

		// Load alpha and beta
		__m512i selector1 = _mm512_set1_epi32( alpha );
		__m512i selector2 = _mm512_set1_epi32( beta );

		if ( alpha != 1 )
		{
			// Scale by alpha
			c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );

			c_int32_1p0 = _mm512_mullo_epi32( selector1, c_int32_1p0 );

			c_int32_2p0 = _mm512_mullo_epi32( selector1, c_int32_2p0 );

			c_int32_3p0 = _mm512_mullo_epi32( selector1, c_int32_3p0 );

			c_int32_4p0 = _mm512_mullo_epi32( selector1, c_int32_4p0 );

			c_int32_5p0 = _mm512_mullo_epi32( selector1, c_int32_5p0 );
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				MEMCPY_S32_LT16_INIT(n0_rem);

				int8_t* _p0 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
					( post_ops_attr.rs_c_downscale * \
					( post_ops_attr.post_op_c_i + 0 ) ) + post_ops_attr.post_op_c_j;
				int8_t* _p1 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
					( post_ops_attr.rs_c_downscale * \
					( post_ops_attr.post_op_c_i + 1 ) ) + post_ops_attr.post_op_c_j;
				int8_t* _p2 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
					( post_ops_attr.rs_c_downscale * \
					( post_ops_attr.post_op_c_i + 2 ) ) + post_ops_attr.post_op_c_j;
				int8_t* _p3 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
					( post_ops_attr.rs_c_downscale * \
					( post_ops_attr.post_op_c_i + 3 ) ) + post_ops_attr.post_op_c_j;
				int8_t* _p4 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
					( post_ops_attr.rs_c_downscale * \
					( post_ops_attr.post_op_c_i + 4 ) ) + post_ops_attr.post_op_c_j;
				int8_t* _p5 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
					( post_ops_attr.rs_c_downscale * \
					( post_ops_attr.post_op_c_i + 5 ) ) + post_ops_attr.post_op_c_j;

				MEMCPY_S32_LT16_INT8(6,int64_t,int32_t,int16_t,int8_t,buf,_p);

				// c[0,0-15]
				S8_S32_BETA_OP_NLT16F(c_int32_0p0,buf0,selector1,selector2);

				// c[1,0-15]
				S8_S32_BETA_OP_NLT16F(c_int32_1p0,buf1,selector1,selector2);

				// c[2,0-15]
				S8_S32_BETA_OP_NLT16F(c_int32_2p0,buf2,selector1,selector2);

				// c[3,0-15]
				S8_S32_BETA_OP_NLT16F(c_int32_3p0,buf3,selector1,selector2);

				// c[4,0-15]
				S8_S32_BETA_OP_NLT16F(c_int32_4p0,buf4,selector1,selector2);

				// c[5,0-15]
				S8_S32_BETA_OP_NLT16F(c_int32_5p0,buf5,selector1,selector2);
			}
			else
			{
				MEMCPY_S32_LT16_INIT(n0_rem);

				int32_t* _c0 = c + ( rs_c * ( ir + 0 ) ) + ( 0 * 16 );
				int32_t* _c1 = c + ( rs_c * ( ir + 1 ) ) + ( 0 * 16 );
				int32_t* _c2 = c + ( rs_c * ( ir + 2 ) ) + ( 0 * 16 );
				int32_t* _c3 = c + ( rs_c * ( ir + 3 ) ) + ( 0 * 16 );
				int32_t* _c4 = c + ( rs_c * ( ir + 4 ) ) + ( 0 * 16 );
				int32_t* _c5 = c + ( rs_c * ( ir + 5 ) ) + ( 0 * 16 );

				MEMCPY_S32_LT16_INT32(6,int64_t,int32_t,buf,_c);

				// c[0,0-15]
				S32_S32_BETA_OP_NLT16F(c_int32_0p0,buf0,selector1,selector2);

				// c[1,0-15]
				S32_S32_BETA_OP_NLT16F(c_int32_1p0,buf1,selector1,selector2);

				// c[2,0-15]
				S32_S32_BETA_OP_NLT16F(c_int32_2p0,buf2,selector1,selector2);

				// c[3,0-15]
				S32_S32_BETA_OP_NLT16F(c_int32_3p0,buf3,selector1,selector2);

				// c[4,0-15]
				S32_S32_BETA_OP_NLT16F(c_int32_4p0,buf4,selector1,selector2);

				// c[5,0-15]
				S32_S32_BETA_OP_NLT16F(c_int32_5p0,buf5,selector1,selector2);
			}
		}

        // Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_6xLT16:
		{
			memcpy( buf0, ( ( int32_t* )post_ops_list_temp->op_args1 +
						post_ops_attr.post_op_c_j ), ( n0_rem * sizeof( int32_t ) ) );
			selector1 = _mm512_loadu_epi32( buf0 );

			// c[0,0-15]
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			// c[4,0-15]
			c_int32_4p0 = _mm512_add_epi32( selector1, c_int32_4p0 );

			// c[5,0-15]
			c_int32_5p0 = _mm512_add_epi32( selector1, c_int32_5p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_6xLT16:
		{
			selector1 = _mm512_setzero_epi32();

			// c[0,0-15]
			c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_max_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_max_epi32( selector1, c_int32_2p0 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_max_epi32( selector1, c_int32_3p0 );

			// c[4,0-15]
			c_int32_4p0 = _mm512_max_epi32( selector1, c_int32_4p0 );

			// c[5,0-15]
			c_int32_5p0 = _mm512_max_epi32( selector1, c_int32_5p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_6xLT16:
		{
			selector1 = _mm512_setzero_epi32();
			selector2 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

			__mmask16 relu_cmp_mask;

			// c[0, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

			// c[1, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_1p0)

			// c[2, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_2p0)

			// c[3, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_3p0)

			// c[4, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_4p0)

			// c[5, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_5p0)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_6xLT16:
		{
			__m512 dn, z, x, r2, r, y, x_tanh;
			__m512i q;

			// c[0, 0-15]
			GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 0-15]
			GELU_TANH_S32_AVX512(c_int32_1p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[2, 0-15]
			GELU_TANH_S32_AVX512(c_int32_2p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[3, 0-15]
			GELU_TANH_S32_AVX512(c_int32_3p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[4, 0-15]
			GELU_TANH_S32_AVX512(c_int32_4p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[5, 0-15]
			GELU_TANH_S32_AVX512(c_int32_5p0, y, r, r2, x, z, dn, x_tanh, q)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_6xLT16:
		{
			__m512 x, r, y, x_erf;

			// c[0, 0-15]
			GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

			// c[1, 0-15]
			GELU_ERF_S32_AVX512(c_int32_1p0, y, r, x, x_erf)

			// c[2, 0-15]
			GELU_ERF_S32_AVX512(c_int32_2p0, y, r, x, x_erf)

			// c[3, 0-15]
			GELU_ERF_S32_AVX512(c_int32_3p0, y, r, x, x_erf)

			// c[4, 0-15]
			GELU_ERF_S32_AVX512(c_int32_4p0, y, r, x, x_erf)

			// c[5, 0-15]
			GELU_ERF_S32_AVX512(c_int32_5p0, y, r, x, x_erf)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}

POST_OPS_DOWNSCALE_6xLT16:
		{
			// Typecast without data modification, safe operation.
			float* _buf0 = ( float* )buf0;
			float* _p0 = ( ( float* )post_ops_list_temp->scale_factor + \
							post_ops_attr.post_op_c_j );
			MEMCPY_S32_LT16_INIT(n0_rem);
			MEMCPY_S32_LT16_FLOAT(1,double,float,_buf,_p);

			selector1 = _mm512_loadu_epi32( buf0 );

			// c[0, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_0p0,selector1);

			// c[1, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_1p0,selector1);

			// c[2, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_2p0,selector1);

			// c[3, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_3p0,selector1);

			// c[4, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_4p0,selector1);

			// c[5, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_5p0,selector1);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_6xLT16_DISABLE:
		;

		// Store the results.
		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			// Generate a mask16 of all 1's.
			selector1 = _mm512_setzero_epi32();
			selector2 = _mm512_set1_epi32( 10 );
			__mmask16 mask_all1 = _mm512_cmplt_epi32_mask( selector1, selector2 );

			_mm512_mask_cvtsepi32_storeu_epi8( ( int8_t* )buf0, mask_all1, c_int32_0p0 );
			_mm512_mask_cvtsepi32_storeu_epi8( ( int8_t* )buf1, mask_all1, c_int32_1p0 );
			_mm512_mask_cvtsepi32_storeu_epi8( ( int8_t* )buf2, mask_all1, c_int32_2p0 );
			_mm512_mask_cvtsepi32_storeu_epi8( ( int8_t* )buf3, mask_all1, c_int32_3p0 );
			_mm512_mask_cvtsepi32_storeu_epi8( ( int8_t* )buf4, mask_all1, c_int32_4p0 );
			_mm512_mask_cvtsepi32_storeu_epi8( ( int8_t* )buf5, mask_all1, c_int32_5p0 );

			MEMCPY_S32_LT16_INIT(n0_rem);

			_mm256_zeroupper();

			int8_t* _p0 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
				( post_ops_attr.rs_c_downscale * \
				( post_ops_attr.post_op_c_i + 0 ) ) + post_ops_attr.post_op_c_j;
			int8_t* _p1 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
				( post_ops_attr.rs_c_downscale * \
				( post_ops_attr.post_op_c_i + 1 ) ) + post_ops_attr.post_op_c_j;
			int8_t* _p2 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
				( post_ops_attr.rs_c_downscale * \
				( post_ops_attr.post_op_c_i + 2 ) ) + post_ops_attr.post_op_c_j;
			int8_t* _p3 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
				( post_ops_attr.rs_c_downscale * \
				( post_ops_attr.post_op_c_i + 3 ) ) + post_ops_attr.post_op_c_j;
			int8_t* _p4 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
				( post_ops_attr.rs_c_downscale * \
				( post_ops_attr.post_op_c_i + 4 ) ) + post_ops_attr.post_op_c_j;
			int8_t* _p5 = ( ( int8_t* )post_ops_attr.buf_downscale ) + \
				( post_ops_attr.rs_c_downscale * \
				( post_ops_attr.post_op_c_i + 5 ) ) + post_ops_attr.post_op_c_j;

			MEMCPY_S32_LT16_INT8(6,int64_t,int32_t,int16_t,int8_t,_p,buf) ;
		}
		else
		{
			_mm512_storeu_epi32( buf0, c_int32_0p0 );
			_mm512_storeu_epi32( buf1, c_int32_1p0 );
			_mm512_storeu_epi32( buf2, c_int32_2p0 );
			_mm512_storeu_epi32( buf3, c_int32_3p0 );
			_mm512_storeu_epi32( buf4, c_int32_4p0 );
			_mm512_storeu_epi32( buf5, c_int32_5p0 );

			MEMCPY_S32_LT16_INIT(n0_rem);

			_mm256_zeroupper();
			int32_t* _c0 = c + ( rs_c * ( ir + 0 ) );
			int32_t* _c1 = c + ( rs_c * ( ir + 1 ) );
			int32_t* _c2 = c + ( rs_c * ( ir + 2 ) );
			int32_t* _c3 = c + ( rs_c * ( ir + 3 ) );
			int32_t* _c4 = c + ( rs_c * ( ir + 4 ) );
			int32_t* _c5 = c + ( rs_c * ( ir + 5 ) );

			MEMCPY_S32_LT16_INT32(6,int64_t,int32_t,_c,buf);
		}

		a = a + ( MR * ps_a );
		post_ops_attr.post_op_c_i += MR;
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
			  alpha, beta, n0_rem,
			  post_ops_list, post_ops_attr
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
			  alpha, beta, n0_rem,
			  post_ops_list, post_ops_attr
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
			  alpha, beta, n0_rem,
			  post_ops_list, post_ops_attr
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
			  alpha, beta, n0_rem,
			  post_ops_list, post_ops_attr
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
			  alpha, beta, n0_rem,
			  post_ops_list, post_ops_attr
			);
		}
	}
}

// 6x16 int8o32 fringe kernel
LPGEMM_N_FRINGE_KERN(uint8_t,int8_t,int32_t,u8s8s32o32_6x16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_6x16_DISABLE,
						  &&POST_OPS_BIAS_6x16,
						  &&POST_OPS_RELU_6x16,
						  &&POST_OPS_RELU_SCALE_6x16,
						  &&POST_OPS_GELU_TANH_6x16,
						  &&POST_OPS_GELU_ERF_6x16,
						  &&POST_OPS_DOWNSCALE_6x16
						};
	dim_t MR = 6;
	dim_t m_full_pieces = m0 / MR;
	dim_t m_full_pieces_loop_limit = m_full_pieces * MR;
	dim_t m_partial_pieces = m0 % MR;

	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint32_t a_kfringe_buf = 0;

	// B matrix storage.
	__m512i b0;

	// A matrix storage.
	__m512i a_int32_0;

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
			b0 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

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
			b0 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

			// Broadcast a[2,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

			// Broadcast a[3,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );

			// Broadcast a[4,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );

			// Broadcast a[5,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 5 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[5,0-15] = a[5,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_5p0 = _mm512_dpbusd_epi32( c_int32_5p0, a_int32_0, b0 );
		}

		// Load alpha and beta
		__m512i selector1 = _mm512_set1_epi32( alpha );
		__m512i selector2 = _mm512_set1_epi32( beta );

		if ( alpha != 1 )
		{
			// Scale by alpha
			c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );

			c_int32_1p0 = _mm512_mullo_epi32( selector1, c_int32_1p0 );

			c_int32_2p0 = _mm512_mullo_epi32( selector1, c_int32_2p0 );

			c_int32_3p0 = _mm512_mullo_epi32( selector1, c_int32_3p0 );

			c_int32_4p0 = _mm512_mullo_epi32( selector1, c_int32_4p0 );

			c_int32_5p0 = _mm512_mullo_epi32( selector1, c_int32_5p0 );
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				// c[0:0-15]
				S8_S32_BETA_OP(c_int32_0p0,ir,0,0,selector1,selector2);

				// c[1:0-15]
				S8_S32_BETA_OP(c_int32_1p0,ir,1,0,selector1,selector2);

				// c[2:0-15]
				S8_S32_BETA_OP(c_int32_2p0,ir,2,0,selector1,selector2);

				// c[3:0-15]
				S8_S32_BETA_OP(c_int32_3p0,ir,3,0,selector1,selector2);

				// c[4:0-15]
				S8_S32_BETA_OP(c_int32_4p0,ir,4,0,selector1,selector2);

				// c[5:0-15]
				S8_S32_BETA_OP(c_int32_5p0,ir,5,0,selector1,selector2);
			}
			else
			{
				// c[0:0-15]
				S32_S32_BETA_OP(c_int32_0p0,ir,0,0,selector1,selector2);

				// c[1:0-15]
				S32_S32_BETA_OP(c_int32_1p0,ir,1,0,selector1,selector2);

				// c[2:0-15]
				S32_S32_BETA_OP(c_int32_2p0,ir,2,0,selector1,selector2);

				// c[3:0-15]
				S32_S32_BETA_OP(c_int32_3p0,ir,3,0,selector1,selector2);

				// c[4:0-15]
				S32_S32_BETA_OP(c_int32_4p0,ir,4,0,selector1,selector2);

				// c[5:0-15]
				S32_S32_BETA_OP(c_int32_5p0,ir,5,0,selector1,selector2);
			}
		}

        // Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_6x16:
		{
			selector1 =
					_mm512_loadu_epi32( ( int32_t* )post_ops_list_temp->op_args1 +
									post_ops_attr.post_op_c_j );

			// c[0,0-15]
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			// c[4,0-15]
			c_int32_4p0 = _mm512_add_epi32( selector1, c_int32_4p0 );

			// c[5,0-15]
			c_int32_5p0 = _mm512_add_epi32( selector1, c_int32_5p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_6x16:
		{
			selector1 = _mm512_setzero_epi32();

			// c[0,0-15]
			c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_max_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_max_epi32( selector1, c_int32_2p0 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_max_epi32( selector1, c_int32_3p0 );

			// c[4,0-15]
			c_int32_4p0 = _mm512_max_epi32( selector1, c_int32_4p0 );

			// c[5,0-15]
			c_int32_5p0 = _mm512_max_epi32( selector1, c_int32_5p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_6x16:
		{
			selector1 = _mm512_setzero_epi32();
			selector2 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

			__mmask16 relu_cmp_mask;

			// c[0, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

			// c[1, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_1p0)

			// c[2, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_2p0)

			// c[3, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_3p0)

			// c[4, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_4p0)

			// c[5, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_5p0)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_6x16:
		{
			__m512 dn, z, x, r2, r, y, x_tanh;
			__m512i q;

			// c[0, 0-15]
			GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 0-15]
			GELU_TANH_S32_AVX512(c_int32_1p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[2, 0-15]
			GELU_TANH_S32_AVX512(c_int32_2p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[3, 0-15]
			GELU_TANH_S32_AVX512(c_int32_3p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[4, 0-15]
			GELU_TANH_S32_AVX512(c_int32_4p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[5, 0-15]
			GELU_TANH_S32_AVX512(c_int32_5p0, y, r, r2, x, z, dn, x_tanh, q)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_6x16:
		{
			__m512 x, r, y, x_erf;

			// c[0, 0-15]
			GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

			// c[1, 0-15]
			GELU_ERF_S32_AVX512(c_int32_1p0, y, r, x, x_erf)

			// c[2, 0-15]
			GELU_ERF_S32_AVX512(c_int32_2p0, y, r, x, x_erf)

			// c[3, 0-15]
			GELU_ERF_S32_AVX512(c_int32_3p0, y, r, x, x_erf)

			// c[4, 0-15]
			GELU_ERF_S32_AVX512(c_int32_4p0, y, r, x, x_erf)

			// c[5, 0-15]
			GELU_ERF_S32_AVX512(c_int32_5p0, y, r, x, x_erf)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}

POST_OPS_DOWNSCALE_6x16:
	{
		selector1 =
			_mm512_loadu_epi32( ( float* )post_ops_list_temp->scale_factor +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) );

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1);

		// c[4, 0-15]
		CVT_MULRND_CVT32(c_int32_4p0,selector1);

		// c[5, 0-15]
		CVT_MULRND_CVT32(c_int32_5p0,selector1);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_6x16_DISABLE:
		;

		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			// Generate a mask16 of all 1's.
			selector1 = _mm512_setzero_epi32();
			selector2 = _mm512_set1_epi32( 10 );
			__mmask16 mask_all1 = _mm512_cmplt_epi32_mask( selector1, selector2 );

			// Store the results in downscaled type (int8 instead of int32).
			// c[0,0-15]
			CVT_STORE_S32_S8(c_int32_0p0,0,0);

			// c[1,0-15]
			CVT_STORE_S32_S8(c_int32_1p0,1,0);

			// c[2,0-15]
			CVT_STORE_S32_S8(c_int32_2p0,2,0);

			// c[3,0-15]
			CVT_STORE_S32_S8(c_int32_3p0,3,0);

			// c[4,0-15]
			CVT_STORE_S32_S8(c_int32_4p0,4,0);

			// c[5,0-15]
			CVT_STORE_S32_S8(c_int32_5p0,5,0);
		}
		else
		{
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
		}

		a = a + ( MR * ps_a );
		post_ops_attr.post_op_c_i += MR;
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
			);
		}
	}
}

// 6x32 int8o32 fringe kernel
LPGEMM_N_FRINGE_KERN(uint8_t,int8_t,int32_t,u8s8s32o32_6x32)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_6x32_DISABLE,
						  &&POST_OPS_BIAS_6x32,
						  &&POST_OPS_RELU_6x32,
						  &&POST_OPS_RELU_SCALE_6x32,
						  &&POST_OPS_GELU_TANH_6x32,
						  &&POST_OPS_GELU_ERF_6x32,
						  &&POST_OPS_DOWNSCALE_6x32
						};
	dim_t MR = 6;
	dim_t m_full_pieces = m0 / MR;
	dim_t m_full_pieces_loop_limit = m_full_pieces * MR;
	dim_t m_partial_pieces = m0 % MR;

	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint32_t a_kfringe_buf = 0;

	// B matrix storage.
	__m512i b0;
	__m512i b1;

	// A matrix storage.
	__m512i a_int32_0;

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
			b0 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 0 ) );
			b1 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 1 ) );

			// Broadcast a[0,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

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
			b0 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
			b1 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );

			// Broadcast a[0,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

			// Broadcast a[1,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );

			// Broadcast a[2,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );

			// Broadcast a[3,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-31] = a[3,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );

			// Broadcast a[4,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-31] = a[4,kr:kr+4]*b[kr:kr+4,0-31]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );

			// Broadcast a[5,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 5 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
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

		if ( alpha != 1 )
		{
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
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				// c[0:0-15,16-31]
				S8_S32_BETA_OP2(ir,0,selector1,selector2);

				// c[1:0-15,16-31]
				S8_S32_BETA_OP2(ir,1,selector1,selector2);

				// c[2:0-15,16-31]
				S8_S32_BETA_OP2(ir,2,selector1,selector2);

				// c[3:0-15,16-31]
				S8_S32_BETA_OP2(ir,3,selector1,selector2);

				// c[4:0-15,16-31]
				S8_S32_BETA_OP2(ir,4,selector1,selector2);

				// c[5:0-15,16-31]
				S8_S32_BETA_OP2(ir,5,selector1,selector2);
			}
			else
			{
				// c[0:0-15,16-31]
				S32_S32_BETA_OP2(ir,0,selector1,selector2);

				// c[1:0-15,16-31]
				S32_S32_BETA_OP2(ir,1,selector1,selector2);

				// c[2:0-15,16-31]
				S32_S32_BETA_OP2(ir,2,selector1,selector2);

				// c[3:0-15,16-31]
				S32_S32_BETA_OP2(ir,3,selector1,selector2);

				// c[4:0-15,16-31]
				S32_S32_BETA_OP2(ir,4,selector1,selector2);

				// c[5:0-15,16-31]
				S32_S32_BETA_OP2(ir,5,selector1,selector2);
			}
		}

        // Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_6x32:
		{
			selector1 =
					_mm512_loadu_epi32( ( int32_t* )post_ops_list_temp->op_args1 +
									post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
					_mm512_loadu_epi32( ( int32_t* )post_ops_list_temp->op_args1 +
									post_ops_attr.post_op_c_j + ( 1 * 16 ) );

			// c[0,0-15]
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[0, 16-31]
			c_int32_0p1 = _mm512_add_epi32( selector2, c_int32_0p1 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[1, 16-31]
			c_int32_1p1 = _mm512_add_epi32( selector2, c_int32_1p1 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[2, 16-31]
			c_int32_2p1 = _mm512_add_epi32( selector2, c_int32_2p1 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			// c[3, 16-31]
			c_int32_3p1 = _mm512_add_epi32( selector2, c_int32_3p1 );

			// c[4,0-15]
			c_int32_4p0 = _mm512_add_epi32( selector1, c_int32_4p0 );

			// c[4, 16-31]
			c_int32_4p1 = _mm512_add_epi32( selector2, c_int32_4p1 );

			// c[5,0-15]
			c_int32_5p0 = _mm512_add_epi32( selector1, c_int32_5p0 );

			// c[5, 16-31]
			c_int32_5p1 = _mm512_add_epi32( selector2, c_int32_5p1 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_6x32:
		{
			selector1 = _mm512_setzero_epi32();

			// c[0,0-15]
			c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

			// c[0, 16-31]
			c_int32_0p1 = _mm512_max_epi32( selector1, c_int32_0p1 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_max_epi32( selector1, c_int32_1p0 );

			// c[1,16-31]
			c_int32_1p1 = _mm512_max_epi32( selector1, c_int32_1p1 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_max_epi32( selector1, c_int32_2p0 );

			// c[2,16-31]
			c_int32_2p1 = _mm512_max_epi32( selector1, c_int32_2p1 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_max_epi32( selector1, c_int32_3p0 );

			// c[3,16-31]
			c_int32_3p1 = _mm512_max_epi32( selector1, c_int32_3p1 );

			// c[4,0-15]
			c_int32_4p0 = _mm512_max_epi32( selector1, c_int32_4p0 );

			// c[4,16-31]
			c_int32_4p1 = _mm512_max_epi32( selector1, c_int32_4p1 );

			// c[5,0-15]
			c_int32_5p0 = _mm512_max_epi32( selector1, c_int32_5p0 );

			// c[5,16-31]
			c_int32_5p1 = _mm512_max_epi32( selector1, c_int32_5p1 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_6x32:
		{
			selector1 = _mm512_setzero_epi32();
			selector2 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

			__mmask16 relu_cmp_mask;

			// c[0, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

			// c[0, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p1)

			// c[1, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_1p0)

			// c[1, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_1p1)

			// c[2, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_2p0)

			// c[2, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_2p1)

			// c[3, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_3p0)

			// c[3, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_3p1)

			// c[4, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_4p0)

			// c[4, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_4p1)

			// c[5, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_5p0)

			// c[5, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_5p1)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_6x32:
		{
			__m512 dn, z, x, r2, r, y, x_tanh;
			__m512i q;

			// c[0, 0-15]
			GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[0, 16-31]
			GELU_TANH_S32_AVX512(c_int32_0p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 0-15]
			GELU_TANH_S32_AVX512(c_int32_1p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 16-31]
			GELU_TANH_S32_AVX512(c_int32_1p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[2, 0-15]
			GELU_TANH_S32_AVX512(c_int32_2p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[2, 16-31]
			GELU_TANH_S32_AVX512(c_int32_2p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[3, 0-15]
			GELU_TANH_S32_AVX512(c_int32_3p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[3, 16-31]
			GELU_TANH_S32_AVX512(c_int32_3p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[4, 0-15]
			GELU_TANH_S32_AVX512(c_int32_4p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[4, 16-31]
			GELU_TANH_S32_AVX512(c_int32_4p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[5, 0-15]
			GELU_TANH_S32_AVX512(c_int32_5p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[5, 16-31]
			GELU_TANH_S32_AVX512(c_int32_5p1, y, r, r2, x, z, dn, x_tanh, q)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_6x32:
		{
			__m512 x, r, y, x_erf;

			// c[0, 0-15]
			GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

			// c[0, 16-31]
			GELU_ERF_S32_AVX512(c_int32_0p1, y, r, x, x_erf)

			// c[1, 0-15]
			GELU_ERF_S32_AVX512(c_int32_1p0, y, r, x, x_erf)

			// c[1, 16-31]
			GELU_ERF_S32_AVX512(c_int32_1p1, y, r, x, x_erf)

			// c[2, 0-15]
			GELU_ERF_S32_AVX512(c_int32_2p0, y, r, x, x_erf)

			// c[2, 16-31]
			GELU_ERF_S32_AVX512(c_int32_2p1, y, r, x, x_erf)

			// c[3, 0-15]
			GELU_ERF_S32_AVX512(c_int32_3p0, y, r, x, x_erf)

			// c[3, 16-31]
			GELU_ERF_S32_AVX512(c_int32_3p1, y, r, x, x_erf)

			// c[4, 0-15]
			GELU_ERF_S32_AVX512(c_int32_4p0, y, r, x, x_erf)

			// c[4, 16-31]
			GELU_ERF_S32_AVX512(c_int32_4p1, y, r, x, x_erf)

			// c[5, 0-15]
			GELU_ERF_S32_AVX512(c_int32_5p0, y, r, x, x_erf)

			// c[5, 16-31]
			GELU_ERF_S32_AVX512(c_int32_5p1, y, r, x, x_erf)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}

POST_OPS_DOWNSCALE_6x32:
	{
		selector1 =
			_mm512_loadu_epi32( ( float* )post_ops_list_temp->scale_factor +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
			_mm512_loadu_epi32( ( float* )post_ops_list_temp->scale_factor +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) );

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1);

		// c[2, 16-31]
		CVT_MULRND_CVT32(c_int32_2p1,selector2);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1);

		// c[3, 16-31]
		CVT_MULRND_CVT32(c_int32_3p1,selector2);

		// c[4, 0-15]
		CVT_MULRND_CVT32(c_int32_4p0,selector1);

		// c[4, 16-31]
		CVT_MULRND_CVT32(c_int32_4p1,selector2);

		// c[5, 0-15]
		CVT_MULRND_CVT32(c_int32_5p0,selector1);

		// c[5, 16-31]
		CVT_MULRND_CVT32(c_int32_5p1,selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_6x32_DISABLE:
		;

		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			// Generate a mask16 of all 1's.
			selector1 = _mm512_setzero_epi32();
			selector2 = _mm512_set1_epi32( 10 );
			__mmask16 mask_all1 = _mm512_cmplt_epi32_mask( selector1, selector2 );

			// Store the results in downscaled type (int8 instead of int32).
			// c[0,0-15]
			CVT_STORE_S32_S8(c_int32_0p0,0,0);

			// c[0,16-31]
			CVT_STORE_S32_S8(c_int32_0p1,0,1);

			// c[1,0-15]
			CVT_STORE_S32_S8(c_int32_1p0,1,0);

			// c[1,16-31]
			CVT_STORE_S32_S8(c_int32_1p1,1,1);

			// c[2,0-15]
			CVT_STORE_S32_S8(c_int32_2p0,2,0);

			// c[2,16-31]
			CVT_STORE_S32_S8(c_int32_2p1,2,1);

			// c[3,0-15]
			CVT_STORE_S32_S8(c_int32_3p0,3,0);

			// c[3,16-31]
			CVT_STORE_S32_S8(c_int32_3p1,3,1);

			// c[4,0-15]
			CVT_STORE_S32_S8(c_int32_4p0,4,0);

			// c[4,16-31]
			CVT_STORE_S32_S8(c_int32_4p1,4,1);

			// c[5,0-15]
			CVT_STORE_S32_S8(c_int32_5p0,5,0);

			// c[5,16-31]
			CVT_STORE_S32_S8(c_int32_5p1,5,1);
		}
		else
		{
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
		}

		a = a + ( MR * ps_a );
		post_ops_attr.post_op_c_i += MR;
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
			);
		}
	}
}

// 6x48 int8o32 fringe kernel
LPGEMM_N_FRINGE_KERN(uint8_t,int8_t,int32_t,u8s8s32o32_6x48)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_6x48_DISABLE,
						  &&POST_OPS_BIAS_6x48,
						  &&POST_OPS_RELU_6x48,
						  &&POST_OPS_RELU_SCALE_6x48,
						  &&POST_OPS_GELU_TANH_6x48,
						  &&POST_OPS_GELU_ERF_6x48,
						  &&POST_OPS_DOWNSCALE_6x48
						};
	dim_t MR = 6;
	dim_t m_full_pieces = m0 / MR;
	dim_t m_full_pieces_loop_limit = m_full_pieces * MR;
	dim_t m_partial_pieces = m0 % MR;

	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint32_t a_kfringe_buf = 0;

	// B matrix storage.
	__m512i b0;
	__m512i b1;
	__m512i b2;

	// A matrix storage.
	__m512i a_int32_0;

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
			b0 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 0 ) );
			b1 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 1 ) );
			b2 = _mm512_loadu_epi8( b + ( rs_b * kr ) + ( cs_b * 2 ) );

			// Broadcast a[0,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( uint32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

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
			b0 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
			b1 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );
			b2 = _mm512_loadu_epi8( b + ( rs_b * k_full_pieces ) + ( cs_b * 2 ) );

			// Broadcast a[0,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
			c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
			c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

			// Broadcast a[1,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
			c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
			c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );

			// Broadcast a[2,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
			c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
			c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );

			// Broadcast a[3,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-47] = a[3,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
			c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
			c_int32_3p2 = _mm512_dpbusd_epi32( c_int32_3p2, a_int32_0, b2 );

			// Broadcast a[4,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
			);
			a_int32_0 = _mm512_set1_epi32( a_kfringe_buf );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-47] = a[4,kr:kr+4]*b[kr:kr+4,0-47]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
			c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
			c_int32_4p2 = _mm512_dpbusd_epi32( c_int32_4p2, a_int32_0, b2 );

			// Broadcast a[5,kr:kr+4].
			MEMCPY_S32GM_LT4_UINT8
			(
			  &a_kfringe_buf,
			  ( a + ( rs_a * 5 ) + ( cs_a * k_full_pieces ) ),
			  ( k_partial_pieces )
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

		if ( alpha != 1 )
		{
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
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				// c[0:0-15,16-31,32-47]
				S8_S32_BETA_OP3(ir,0,selector1,selector2);

				// c[1:0-15,16-31,32-47]
				S8_S32_BETA_OP3(ir,1,selector1,selector2);

				// c[2:0-15,16-31,32-47]
				S8_S32_BETA_OP3(ir,2,selector1,selector2);

				// c[3:0-15,16-31,32-47]
				S8_S32_BETA_OP3(ir,3,selector1,selector2);

				// c[4:0-15,16-31,32-47]
				S8_S32_BETA_OP3(ir,4,selector1,selector2);

				// c[5:0-15,16-31,32-47]
				S8_S32_BETA_OP3(ir,5,selector1,selector2);
			}
			else
			{
				// c[0:0-15,16-31,32-47]
				S32_S32_BETA_OP3(ir,0,selector1,selector2);

				// c[1:0-15,16-31,32-47]
				S32_S32_BETA_OP3(ir,1,selector1,selector2);

				// c[2:0-15,16-31,32-47]
				S32_S32_BETA_OP3(ir,2,selector1,selector2);

				// c[3:0-15,16-31,32-47]
				S32_S32_BETA_OP3(ir,3,selector1,selector2);

				// c[4:0-15,16-31,32-47]
				S32_S32_BETA_OP3(ir,4,selector1,selector2);

				// c[5:0-15,16-31,32-47]
				S32_S32_BETA_OP3(ir,5,selector1,selector2);
			}
		}

        // Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_6x48:
		{
			selector1 =
					_mm512_loadu_epi32( ( int32_t* )post_ops_list_temp->op_args1 +
									post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
					_mm512_loadu_epi32( ( int32_t* )post_ops_list_temp->op_args1 +
									post_ops_attr.post_op_c_j + ( 1 * 16 ) );
			a_int32_0 =
					_mm512_loadu_epi32( ( int32_t* )post_ops_list_temp->op_args1 +
									post_ops_attr.post_op_c_j + ( 2 * 16 ) );

			// c[0,0-15]
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[0, 16-31]
			c_int32_0p1 = _mm512_add_epi32( selector2, c_int32_0p1 );

			// c[0,32-47]
			c_int32_0p2 = _mm512_add_epi32( a_int32_0, c_int32_0p2 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[1, 16-31]
			c_int32_1p1 = _mm512_add_epi32( selector2, c_int32_1p1 );

			// c[1,32-47]
			c_int32_1p2 = _mm512_add_epi32( a_int32_0, c_int32_1p2 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[2, 16-31]
			c_int32_2p1 = _mm512_add_epi32( selector2, c_int32_2p1 );

			// c[2,32-47]
			c_int32_2p2 = _mm512_add_epi32( a_int32_0, c_int32_2p2 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			// c[3, 16-31]
			c_int32_3p1 = _mm512_add_epi32( selector2, c_int32_3p1 );

			// c[3,32-47]
			c_int32_3p2 = _mm512_add_epi32( a_int32_0, c_int32_3p2 );

			// c[4,0-15]
			c_int32_4p0 = _mm512_add_epi32( selector1, c_int32_4p0 );

			// c[4, 16-31]
			c_int32_4p1 = _mm512_add_epi32( selector2, c_int32_4p1 );

			// c[4,32-47]
			c_int32_4p2 = _mm512_add_epi32( a_int32_0, c_int32_4p2 );

			// c[5,0-15]
			c_int32_5p0 = _mm512_add_epi32( selector1, c_int32_5p0 );

			// c[5, 16-31]
			c_int32_5p1 = _mm512_add_epi32( selector2, c_int32_5p1 );

			// c[5,32-47]
			c_int32_5p2 = _mm512_add_epi32( a_int32_0, c_int32_5p2 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_6x48:
		{
			//printf("relu\n");
			selector1 = _mm512_setzero_epi32();

			// c[0,0-15]
			c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

			// c[0, 16-31]
			c_int32_0p1 = _mm512_max_epi32( selector1, c_int32_0p1 );

			// c[0,32-47]
			c_int32_0p2 = _mm512_max_epi32( selector1, c_int32_0p2 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_max_epi32( selector1, c_int32_1p0 );

			// c[1,16-31]
			c_int32_1p1 = _mm512_max_epi32( selector1, c_int32_1p1 );

			// c[1,32-47]
			c_int32_1p2 = _mm512_max_epi32( selector1, c_int32_1p2 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_max_epi32( selector1, c_int32_2p0 );

			// c[2,16-31]
			c_int32_2p1 = _mm512_max_epi32( selector1, c_int32_2p1 );

			// c[2,32-47]
			c_int32_2p2 = _mm512_max_epi32( selector1, c_int32_2p2 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_max_epi32( selector1, c_int32_3p0 );

			// c[3,16-31]
			c_int32_3p1 = _mm512_max_epi32( selector1, c_int32_3p1 );

			// c[3,32-47]
			c_int32_3p2 = _mm512_max_epi32( selector1, c_int32_3p2 );

			// c[4,0-15]
			c_int32_4p0 = _mm512_max_epi32( selector1, c_int32_4p0 );

			// c[4,16-31]
			c_int32_4p1 = _mm512_max_epi32( selector1, c_int32_4p1 );

			// c[4,32-47]
			c_int32_4p2 = _mm512_max_epi32( selector1, c_int32_4p2 );

			// c[5,0-15]
			c_int32_5p0 = _mm512_max_epi32( selector1, c_int32_5p0 );

			// c[5,16-31]
			c_int32_5p1 = _mm512_max_epi32( selector1, c_int32_5p1 );

			// c[5,32-47]
			c_int32_5p2 = _mm512_max_epi32( selector1, c_int32_5p2 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_6x48:
		{
			selector1 = _mm512_setzero_epi32();
			selector2 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

			__mmask16 relu_cmp_mask;

			// c[0, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

			// c[0, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p1)

			// c[0, 32-47]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p2)

			// c[1, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_1p0)

			// c[1, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_1p1)

			// c[1, 32-47]
			RELU_SCALE_OP_S32_AVX512(c_int32_1p2)

			// c[2, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_2p0)

			// c[2, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_2p1)

			// c[2, 32-47]
			RELU_SCALE_OP_S32_AVX512(c_int32_2p2)

			// c[3, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_3p0)

			// c[3, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_3p1)

			// c[3, 32-47]
			RELU_SCALE_OP_S32_AVX512(c_int32_3p2)

			// c[4, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_4p0)

			// c[4, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_4p1)

			// c[4, 32-47]
			RELU_SCALE_OP_S32_AVX512(c_int32_4p2)

			// c[5, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_5p0)

			// c[5, 16-31]
			RELU_SCALE_OP_S32_AVX512(c_int32_5p1)

			// c[5, 32-47]
			RELU_SCALE_OP_S32_AVX512(c_int32_5p2)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_6x48:
		{
			__m512 dn, z, x, r2, r, y, x_tanh;
			__m512i q;

			// c[0, 0-15]
			GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[0, 16-31]
			GELU_TANH_S32_AVX512(c_int32_0p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[0, 32-47]
			GELU_TANH_S32_AVX512(c_int32_0p2, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 0-15]
			GELU_TANH_S32_AVX512(c_int32_1p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 16-31]
			GELU_TANH_S32_AVX512(c_int32_1p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 32-47]
			GELU_TANH_S32_AVX512(c_int32_1p2, y, r, r2, x, z, dn, x_tanh, q)

			// c[2, 0-15]
			GELU_TANH_S32_AVX512(c_int32_2p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[2, 16-31]
			GELU_TANH_S32_AVX512(c_int32_2p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[2, 32-47]
			GELU_TANH_S32_AVX512(c_int32_2p2, y, r, r2, x, z, dn, x_tanh, q)

			// c[3, 0-15]
			GELU_TANH_S32_AVX512(c_int32_3p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[3, 16-31]
			GELU_TANH_S32_AVX512(c_int32_3p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[3, 32-47]
			GELU_TANH_S32_AVX512(c_int32_3p2, y, r, r2, x, z, dn, x_tanh, q)

			// c[4, 0-15]
			GELU_TANH_S32_AVX512(c_int32_4p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[4, 16-31]
			GELU_TANH_S32_AVX512(c_int32_4p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[4, 32-47]
			GELU_TANH_S32_AVX512(c_int32_4p2, y, r, r2, x, z, dn, x_tanh, q)

			// c[5, 0-15]
			GELU_TANH_S32_AVX512(c_int32_5p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[5, 16-31]
			GELU_TANH_S32_AVX512(c_int32_5p1, y, r, r2, x, z, dn, x_tanh, q)

			// c[5, 32-47]
			GELU_TANH_S32_AVX512(c_int32_5p2, y, r, r2, x, z, dn, x_tanh, q)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_6x48:
		{
			__m512 x, r, y, x_erf;

			// c[0, 0-15]
			GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

			// c[0, 16-31]
			GELU_ERF_S32_AVX512(c_int32_0p1, y, r, x, x_erf)

			// c[0, 32-47]
			GELU_ERF_S32_AVX512(c_int32_0p2, y, r, x, x_erf)

			// c[1, 0-15]
			GELU_ERF_S32_AVX512(c_int32_1p0, y, r, x, x_erf)

			// c[1, 16-31]
			GELU_ERF_S32_AVX512(c_int32_1p1, y, r, x, x_erf)

			// c[1, 32-47]
			GELU_ERF_S32_AVX512(c_int32_1p2, y, r, x, x_erf)

			// c[2, 0-15]
			GELU_ERF_S32_AVX512(c_int32_2p0, y, r, x, x_erf)

			// c[2, 16-31]
			GELU_ERF_S32_AVX512(c_int32_2p1, y, r, x, x_erf)

			// c[2, 32-47]
			GELU_ERF_S32_AVX512(c_int32_2p2, y, r, x, x_erf)

			// c[3, 0-15]
			GELU_ERF_S32_AVX512(c_int32_3p0, y, r, x, x_erf)

			// c[3, 16-31]
			GELU_ERF_S32_AVX512(c_int32_3p1, y, r, x, x_erf)

			// c[3, 32-47]
			GELU_ERF_S32_AVX512(c_int32_3p2, y, r, x, x_erf)

			// c[4, 0-15]
			GELU_ERF_S32_AVX512(c_int32_4p0, y, r, x, x_erf)

			// c[4, 16-31]
			GELU_ERF_S32_AVX512(c_int32_4p1, y, r, x, x_erf)

			// c[4, 32-47]
			GELU_ERF_S32_AVX512(c_int32_4p2, y, r, x, x_erf)

			// c[5, 0-15]
			GELU_ERF_S32_AVX512(c_int32_5p0, y, r, x, x_erf)

			// c[5, 16-31]
			GELU_ERF_S32_AVX512(c_int32_5p1, y, r, x, x_erf)

			// c[5, 32-47]
			GELU_ERF_S32_AVX512(c_int32_5p2, y, r, x, x_erf)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}

POST_OPS_DOWNSCALE_6x48:
	{
		selector1 =
			_mm512_loadu_epi32( ( float* )post_ops_list_temp->scale_factor +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
			_mm512_loadu_epi32( ( float* )post_ops_list_temp->scale_factor +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		a_int32_0 =
			_mm512_loadu_epi32( ( float* )post_ops_list_temp->scale_factor +
							post_ops_attr.post_op_c_j + ( 2 * 16 ) );

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2);

		// c[0, 32-47]
		CVT_MULRND_CVT32(c_int32_0p2,a_int32_0);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2);

		// c[1, 32-47]
		CVT_MULRND_CVT32(c_int32_1p2,a_int32_0);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1);

		// c[2, 16-31]
		CVT_MULRND_CVT32(c_int32_2p1,selector2);

		// c[2, 32-47]
		CVT_MULRND_CVT32(c_int32_2p2,a_int32_0);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1);

		// c[3, 16-31]
		CVT_MULRND_CVT32(c_int32_3p1,selector2);

		// c[3, 32-47]
		CVT_MULRND_CVT32(c_int32_3p2,a_int32_0);

		// c[4, 0-15]
		CVT_MULRND_CVT32(c_int32_4p0,selector1);

		// c[4, 16-31]
		CVT_MULRND_CVT32(c_int32_4p1,selector2);

		// c[4, 32-47]
		CVT_MULRND_CVT32(c_int32_4p2,a_int32_0);

		// c[5, 0-15]
		CVT_MULRND_CVT32(c_int32_5p0,selector1);

		// c[5, 16-31]
		CVT_MULRND_CVT32(c_int32_5p1,selector2);

		// c[5, 32-47]
		CVT_MULRND_CVT32(c_int32_5p2,a_int32_0);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_6x48_DISABLE:
		;

		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			// Generate a mask16 of all 1's.
			selector1 = _mm512_setzero_epi32();
			selector2 = _mm512_set1_epi32( 10 );
			__mmask16 mask_all1 = _mm512_cmplt_epi32_mask( selector1, selector2 );

			// Store the results in downscaled type (int8 instead of int32).
			// c[0,0-15]
			CVT_STORE_S32_S8(c_int32_0p0,0,0);

			// c[0,16-31]
			CVT_STORE_S32_S8(c_int32_0p1,0,1);

			// c[0,32-47]
			CVT_STORE_S32_S8(c_int32_0p2,0,2);

			// c[1,0-15]
			CVT_STORE_S32_S8(c_int32_1p0,1,0);

			// c[1,16-31]
			CVT_STORE_S32_S8(c_int32_1p1,1,1);

			// c[1,32-47]
			CVT_STORE_S32_S8(c_int32_1p2,1,2);

			// c[2,0-15]
			CVT_STORE_S32_S8(c_int32_2p0,2,0);

			// c[2,16-31]
			CVT_STORE_S32_S8(c_int32_2p1,2,1);

			// c[2,32-47]
			CVT_STORE_S32_S8(c_int32_2p2,2,2);

			// c[3,0-15]
			CVT_STORE_S32_S8(c_int32_3p0,3,0);

			// c[3,16-31]
			CVT_STORE_S32_S8(c_int32_3p1,3,1);

			// c[3,32-47]
			CVT_STORE_S32_S8(c_int32_3p2,3,2);

			// c[4,0-15]
			CVT_STORE_S32_S8(c_int32_4p0,4,0);

			// c[4,16-31]
			CVT_STORE_S32_S8(c_int32_4p1,4,1);

			// c[4,32-47]
			CVT_STORE_S32_S8(c_int32_4p2,4,2);

			// c[5,0-15]
			CVT_STORE_S32_S8(c_int32_5p0,5,0);

			// c[5,16-31]
			CVT_STORE_S32_S8(c_int32_5p1,5,1);

			// c[5,32-47]
			CVT_STORE_S32_S8(c_int32_5p2,5,2);
		}
		else
		{
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
		}

		a = a + ( MR * ps_a );
		post_ops_attr.post_op_c_i += MR;
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
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
			  alpha, beta,
			  post_ops_list, post_ops_attr
			);
		}
	}
}
#endif