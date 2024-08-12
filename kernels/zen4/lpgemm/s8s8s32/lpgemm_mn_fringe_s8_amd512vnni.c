/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2023 - 2024, Advanced Micro Devices, Inc. All rights reserved.

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

#include "../u8s8s32/lpgemm_s32_kern_macros.h"
#include "../u8s8s32/lpgemm_s32_memcpy_macros.h"

// 5xlt16 int8o32 fringe kernel
LPGEMM_MN_LT_NR0_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_5xlt16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_5xLT16_DISABLE,
						  &&POST_OPS_BIAS_5xLT16,
						  &&POST_OPS_RELU_5xLT16,
						  &&POST_OPS_RELU_SCALE_5xLT16,
						  &&POST_OPS_GELU_TANH_5xLT16,
						  &&POST_OPS_GELU_ERF_5xLT16,
						  &&POST_OPS_CLIP_5xLT16,
						  &&POST_OPS_DOWNSCALE_5xLT16,
						  &&POST_OPS_MATRIX_ADD_5xLT16,
						  &&POST_OPS_SWISH_5xLT16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();

		__m512i c_int32_1p0 = _mm512_setzero_epi32();

		__m512i c_int32_2p0 = _mm512_setzero_epi32();

		__m512i c_int32_3p0 = _mm512_setzero_epi32();

		__m512i c_int32_4p0 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

            		//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

			// Broadcast a[2,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

            		//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

			// Broadcast a[3,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

            		//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );

			// Broadcast a[4,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 4 ) + ( cs_a * kr ) ) );

            		//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m128i a_kfringe_buf;
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
			__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

			// Broadcast a[2,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

			// Broadcast a[3,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );

			// Broadcast a[4,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
		}

		if ( post_ops_attr.is_last_k == 1 )
		{
			//Subtract B matrix sum column values to compensate
			//for addition of 128 to A matrix elements

			int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

			__m512i b0 = _mm512_loadu_si512( bsumptr );

			c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
			c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
			c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
			c_int32_3p0 = _mm512_sub_epi32( c_int32_3p0 , b0 );
			c_int32_4p0 = _mm512_sub_epi32( c_int32_4p0 , b0 );

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
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_0p0, 0, 0, \
								selector1, selector2 );

				// c[1,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_1p0, 1, 0, \
								selector1, selector2 );

				// c[2,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_2p0, 2, 0, \
								selector1, selector2 );

				// c[3,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_3p0, 3, 0, \
								selector1, selector2 );

				// c[4,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_4p0, 4, 0, \
								selector1, selector2 );
			}
			else
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_0p0, 0, 0, 0, \
								selector1, selector2);

				// c[1,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_1p0, 0, 1, 0, \
								selector1, selector2);

				// c[2,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_2p0, 0, 2, 0, \
								selector1, selector2);

				// c[3,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_3p0, 0, 3, 0, \
								selector1, selector2);

				// c[4,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_4p0, 0, 4, 0, \
								selector1, selector2);
			}
		}

		// Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_5xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			selector1 = _mm512_maskz_loadu_epi32
			(
			  load_mask,
			  ( ( int32_t* )post_ops_list_temp->op_args1 +
				post_ops_attr.post_op_c_j )
			);

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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_5xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_5xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_5xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_5xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_CLIP_5xLT16:
		{
			__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
			__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

			// c[0, 0-15]
			CLIP_S32_AVX512(c_int32_0p0, min, max)

			// c[1, 0-15]
			CLIP_S32_AVX512(c_int32_1p0, min, max)

			// c[2, 0-15]
			CLIP_S32_AVX512(c_int32_2p0, min, max)

			// c[3, 0-15]
			CLIP_S32_AVX512(c_int32_3p0, min, max)

			// c[4, 0-15]
			CLIP_S32_AVX512(c_int32_4p0, min, max)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_DOWNSCALE_5xLT16:
		{
			// Typecast without data modification, safe operation.
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			if ( post_ops_list_temp->scale_factor_len > 1 )
			{
				selector1 = _mm512_maskz_loadu_epi32
							(
							  load_mask,
							  ( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( post_ops_list_temp->scale_factor_len == 1 )
			{
				selector1 =
					( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			}

			// Need to ensure sse not used to avoid avx512 -> sse transition.
			__m128i zero_point = _mm512_castsi512_si128( _mm512_setzero_si512() );
			if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
			{
				zero_point = _mm_maskz_loadu_epi8
							(
							  load_mask,
							  ( ( int8_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
			{
				zero_point = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			}

			// c[0, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_0p0,selector1,zero_point);

			// c[1, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_1p0,selector1,zero_point);

			// c[2, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_2p0,selector1,zero_point);

			// c[3, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_3p0,selector1,zero_point);

			// c[4, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_4p0,selector1,zero_point);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_MATRIX_ADD_5xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
			if ( post_ops_attr.c_stor_type == S8 )
			{
				int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);

				// c[1:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,1);

				// c[2:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,2);

				// c[3:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,3);

				// c[4:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,4);
			}
			else
			{
				int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);

				// c[1:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,1);

				// c[2:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,2);

				// c[3:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,3);

				// c[4:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,4);
			}

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_SWISH_5xLT16:
		{
			selector1 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
			__m512 al = _mm512_cvtepi32_ps( selector1 );

			__m512 fl_reg, al_in, r, r2, z, dn;

			// c[0, 0-15]
			SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[1, 0-15]
			SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[2, 0-15]
			SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[3, 0-15]
			SWISH_S32_AVX512(c_int32_3p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[4, 0-15]
			SWISH_S32_AVX512(c_int32_4p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_5xLT16_DISABLE:
		;

		// Store the results.
		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			__mmask16 mask_all1 = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

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
		}
		else
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results.
			// c[0,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 0 ), load_mask, c_int32_0p0 );

			// c[1,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 1 ), load_mask, c_int32_1p0 );

			// c[2,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 2 ), load_mask, c_int32_2p0 );

			// c[3,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 3 ), load_mask, c_int32_3p0 );

			// c[4,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 4 ), load_mask, c_int32_4p0 );
		}
	}
}

// 4xlt16 int8o32 fringe kernel
LPGEMM_MN_LT_NR0_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_4xlt16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_4xLT16_DISABLE,
						  &&POST_OPS_BIAS_4xLT16,
						  &&POST_OPS_RELU_4xLT16,
						  &&POST_OPS_RELU_SCALE_4xLT16,
						  &&POST_OPS_GELU_TANH_4xLT16,
						  &&POST_OPS_GELU_ERF_4xLT16,
						  &&POST_OPS_CLIP_4xLT16,
						  &&POST_OPS_DOWNSCALE_4xLT16,
						  &&POST_OPS_MATRIX_ADD_4xLT16,
						  &&POST_OPS_SWISH_4xLT16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();

		__m512i c_int32_1p0 = _mm512_setzero_epi32();

		__m512i c_int32_2p0 = _mm512_setzero_epi32();

		__m512i c_int32_3p0 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

			// Broadcast a[2,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

			// Broadcast a[3,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m128i a_kfringe_buf;
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
			__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

			// Broadcast a[2,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

			// Broadcast a[3,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		}

		if ( post_ops_attr.is_last_k == 1 )
		{
			//Subtract B matrix sum column values to compensate
			//for addition of 128 to A matrix elements

			int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

			__m512i b0 = _mm512_loadu_si512( bsumptr );

			c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
			c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
			c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
			c_int32_3p0 = _mm512_sub_epi32( c_int32_3p0 , b0 );
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
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_0p0, 0, 0, \
								selector1, selector2 );

				// c[1,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_1p0, 1, 0, \
								selector1, selector2 );

				// c[2,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_2p0, 2, 0, \
								selector1, selector2 );

				// c[3,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_3p0, 3, 0, \
								selector1, selector2 );
			}
			else
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_0p0, 0, 0, 0, \
								selector1, selector2);

				// c[1,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_1p0, 0, 1, 0, \
								selector1, selector2);

				// c[2,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_2p0, 0, 2, 0, \
								selector1, selector2);

				// c[3,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_3p0, 0, 3, 0, \
								selector1, selector2);
			}
		}

		// Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_4xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			selector1 = _mm512_maskz_loadu_epi32
			(
			  load_mask,
			  ( ( int32_t* )post_ops_list_temp->op_args1 +
				post_ops_attr.post_op_c_j )
			);

			// c[0,0-15]
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			// c[3,0-15]
			c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_4xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_4xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_4xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_4xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_CLIP_4xLT16:
		{
			__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
			__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

			// c[0, 0-15]
			CLIP_S32_AVX512(c_int32_0p0, min, max)

			// c[1, 0-15]
			CLIP_S32_AVX512(c_int32_1p0, min, max)

			// c[2, 0-15]
			CLIP_S32_AVX512(c_int32_2p0, min, max)

			// c[3, 0-15]
			CLIP_S32_AVX512(c_int32_3p0, min, max)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_DOWNSCALE_4xLT16:
		{
			// Typecast without data modification, safe operation.
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			if ( post_ops_list_temp->scale_factor_len > 1 )
			{
				selector1 = _mm512_maskz_loadu_epi32
							(
							  load_mask,
							  ( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( post_ops_list_temp->scale_factor_len == 1 )
			{
				selector1 =
					( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			}

			// Need to ensure sse not used to avoid avx512 -> sse transition.
			__m128i zero_point = _mm512_castsi512_si128( _mm512_setzero_si512() );
			if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
			{
				zero_point = _mm_maskz_loadu_epi8
							(
							  load_mask,
							  ( ( int8_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
			{
				zero_point = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			}

			// c[0, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_0p0,selector1,zero_point);

			// c[1, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_1p0,selector1,zero_point);

			// c[2, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_2p0,selector1,zero_point);

			// c[3, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_3p0,selector1,zero_point);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_MATRIX_ADD_4xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
			if ( post_ops_attr.c_stor_type == S8 )
			{
				int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);

				// c[1:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,1);

				// c[2:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,2);

				// c[3:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,3);
			}
			else
			{
				int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);

				// c[1:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,1);

				// c[2:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,2);

				// c[3:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,3);
			}

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_SWISH_4xLT16:
		{
			selector1 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
			__m512 al = _mm512_cvtepi32_ps( selector1 );

			__m512 fl_reg, al_in, r, r2, z, dn;

			// c[0, 0-15]
			SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[1, 0-15]
			SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[2, 0-15]
			SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[3, 0-15]
			SWISH_S32_AVX512(c_int32_3p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_4xLT16_DISABLE:
		;

		// Store the results.
		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			__mmask16 mask_all1 = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results in downscaled type (int8 instead of int32).
			// c[0,0-15]
			CVT_STORE_S32_S8(c_int32_0p0,0,0);

			// c[1,0-15]
			CVT_STORE_S32_S8(c_int32_1p0,1,0);

			// c[2,0-15]
			CVT_STORE_S32_S8(c_int32_2p0,2,0);

			// c[3,0-15]
			CVT_STORE_S32_S8(c_int32_3p0,3,0);
		}
		else
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results.
			// c[0,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 0 ), load_mask, c_int32_0p0 );

			// c[1,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 1 ), load_mask, c_int32_1p0 );

			// c[2,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 2 ), load_mask, c_int32_2p0 );

			// c[3,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 3 ), load_mask, c_int32_3p0 );
		}
	}
}

// 3xlt16 int8o32 fringe kernel
LPGEMM_MN_LT_NR0_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_3xlt16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_3xLT16_DISABLE,
						  &&POST_OPS_BIAS_3xLT16,
						  &&POST_OPS_RELU_3xLT16,
						  &&POST_OPS_RELU_SCALE_3xLT16,
						  &&POST_OPS_GELU_TANH_3xLT16,
						  &&POST_OPS_GELU_ERF_3xLT16,
						  &&POST_OPS_CLIP_3xLT16,
						  &&POST_OPS_DOWNSCALE_3xLT16,
						  &&POST_OPS_MATRIX_ADD_3xLT16,
						  &&POST_OPS_SWISH_3xLT16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();

		__m512i c_int32_1p0 = _mm512_setzero_epi32();

		__m512i c_int32_2p0 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

			// Broadcast a[2,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m128i a_kfringe_buf;
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
			__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

			// Broadcast a[2,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		}

		if ( post_ops_attr.is_last_k == 1 )
		{
			//Subtract B matrix sum column values to compensate
			//for addition of 128 to A matrix elements

			int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

			__m512i b0 = _mm512_loadu_si512( bsumptr );

			c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
			c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
			c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
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
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_0p0, 0, 0, \
								selector1, selector2 );

				// c[1,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_1p0, 1, 0, \
								selector1, selector2 );

				// c[2,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_2p0, 2, 0, \
								selector1, selector2 );
			}
			else
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_0p0, 0, 0, 0, \
								selector1, selector2);

				// c[1,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_1p0, 0, 1, 0, \
								selector1, selector2);

				// c[2,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_2p0, 0, 2, 0, \
								selector1, selector2);
			}
		}

		// Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_3xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			selector1 = _mm512_maskz_loadu_epi32
			(
			  load_mask,
			  ( ( int32_t* )post_ops_list_temp->op_args1 +
				post_ops_attr.post_op_c_j )
			);

			// c[0,0-15]
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_3xLT16:
		{
			selector1 = _mm512_setzero_epi32();

			// c[0,0-15]
			c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_max_epi32( selector1, c_int32_1p0 );

			// c[2,0-15]
			c_int32_2p0 = _mm512_max_epi32( selector1, c_int32_2p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_3xLT16:
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

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_3xLT16:
		{
			__m512 dn, z, x, r2, r, y, x_tanh;
			__m512i q;

			// c[0, 0-15]
			GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 0-15]
			GELU_TANH_S32_AVX512(c_int32_1p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[2, 0-15]
			GELU_TANH_S32_AVX512(c_int32_2p0, y, r, r2, x, z, dn, x_tanh, q)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_3xLT16:
		{
			__m512 x, r, y, x_erf;

			// c[0, 0-15]
			GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

			// c[1, 0-15]
			GELU_ERF_S32_AVX512(c_int32_1p0, y, r, x, x_erf)

			// c[2, 0-15]
			GELU_ERF_S32_AVX512(c_int32_2p0, y, r, x, x_erf)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_CLIP_3xLT16:
		{
			__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
			__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

			// c[0, 0-15]
			CLIP_S32_AVX512(c_int32_0p0, min, max)

			// c[1, 0-15]
			CLIP_S32_AVX512(c_int32_1p0, min, max)

			// c[2, 0-15]
			CLIP_S32_AVX512(c_int32_2p0, min, max)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_DOWNSCALE_3xLT16:
		{
			// Typecast without data modification, safe operation.
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			if ( post_ops_list_temp->scale_factor_len > 1 )
			{
				selector1 = _mm512_maskz_loadu_epi32
							(
							  load_mask,
							  ( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( post_ops_list_temp->scale_factor_len == 1 )
			{
				selector1 =
					( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			}

			// Need to ensure sse not used to avoid avx512 -> sse transition.
			__m128i zero_point = _mm512_castsi512_si128( _mm512_setzero_si512() );
			if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
			{
				zero_point = _mm_maskz_loadu_epi8
							(
							  load_mask,
							  ( ( int8_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
			{
				zero_point = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			}

			// c[0, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_0p0,selector1,zero_point);

			// c[1, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_1p0,selector1,zero_point);

			// c[2, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_2p0,selector1,zero_point);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_MATRIX_ADD_3xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
			if ( post_ops_attr.c_stor_type == S8 )
			{
				int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);

				// c[1:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,1);

				// c[2:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,2);
			}
			else
			{
				int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);

				// c[1:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,1);

				// c[2:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,2);
			}

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_SWISH_3xLT16:
		{
			selector1 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
			__m512 al = _mm512_cvtepi32_ps( selector1 );

			__m512 fl_reg, al_in, r, r2, z, dn;

			// c[0, 0-15]
			SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[1, 0-15]
			SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[2, 0-15]
			SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_3xLT16_DISABLE:
		;

		// Store the results.
		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			__mmask16 mask_all1 = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results in downscaled type (int8 instead of int32).
			// c[0,0-15]
			CVT_STORE_S32_S8(c_int32_0p0,0,0);

			// c[1,0-15]
			CVT_STORE_S32_S8(c_int32_1p0,1,0);

			// c[2,0-15]
			CVT_STORE_S32_S8(c_int32_2p0,2,0);
		}
		else
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results.
			// c[0,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 0 ), load_mask, c_int32_0p0 );

			// c[1,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 1 ), load_mask, c_int32_1p0 );

			// c[2,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 2 ), load_mask, c_int32_2p0 );
		}
	}
}

// 2xlt16 int8o32 fringe kernel
LPGEMM_MN_LT_NR0_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_2xlt16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_2xLT16_DISABLE,
						  &&POST_OPS_BIAS_2xLT16,
						  &&POST_OPS_RELU_2xLT16,
						  &&POST_OPS_RELU_SCALE_2xLT16,
						  &&POST_OPS_GELU_TANH_2xLT16,
						  &&POST_OPS_GELU_ERF_2xLT16,
						  &&POST_OPS_CLIP_2xLT16,
						  &&POST_OPS_DOWNSCALE_2xLT16,
						  &&POST_OPS_MATRIX_ADD_2xLT16,
						  &&POST_OPS_SWISH_2xLT16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();

		__m512i c_int32_1p0 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m128i a_kfringe_buf;
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
			__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

			// Broadcast a[1,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
			a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		}

		if ( post_ops_attr.is_last_k == 1 )
		{
			//Subtract B matrix sum column values to compensate
			//for addition of 128 to A matrix elements

			int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

			__m512i b0 = _mm512_loadu_si512( bsumptr);

			c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
			c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		}

		// Load alpha and beta
		__m512i selector1 = _mm512_set1_epi32( alpha );
		__m512i selector2 = _mm512_set1_epi32( beta );

		if ( alpha != 1 )
		{
			// Scale by alpha
			c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );

			c_int32_1p0 = _mm512_mullo_epi32( selector1, c_int32_1p0 );
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_0p0, 0, 0, \
								selector1, selector2 );

				// c[1,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_1p0, 1, 0, \
								selector1, selector2 );
			}
			else
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_0p0, 0, 0, 0, \
								selector1, selector2);

				// c[1,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_1p0, 0, 1, 0, \
								selector1, selector2);
			}
		}

		// Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_2xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			selector1 = _mm512_maskz_loadu_epi32
			(
			  load_mask,
			  ( ( int32_t* )post_ops_list_temp->op_args1 +
				post_ops_attr.post_op_c_j )
			);

			// c[0,0-15]
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_2xLT16:
		{
			selector1 = _mm512_setzero_epi32();

			// c[0,0-15]
			c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

			// c[1,0-15]
			c_int32_1p0 = _mm512_max_epi32( selector1, c_int32_1p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_2xLT16:
		{
			selector1 = _mm512_setzero_epi32();
			selector2 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

			__mmask16 relu_cmp_mask;

			// c[0, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

			// c[1, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_1p0)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_2xLT16:
		{
			__m512 dn, z, x, r2, r, y, x_tanh;
			__m512i q;

			// c[0, 0-15]
			GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

			// c[1, 0-15]
			GELU_TANH_S32_AVX512(c_int32_1p0, y, r, r2, x, z, dn, x_tanh, q)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_2xLT16:
		{
			__m512 x, r, y, x_erf;

			// c[0, 0-15]
			GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

			// c[1, 0-15]
			GELU_ERF_S32_AVX512(c_int32_1p0, y, r, x, x_erf)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_CLIP_2xLT16:
		{
			__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
			__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

			// c[0, 0-15]
			CLIP_S32_AVX512(c_int32_0p0, min, max)

			// c[1, 0-15]
			CLIP_S32_AVX512(c_int32_1p0, min, max)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_DOWNSCALE_2xLT16:
		{
			// Typecast without data modification, safe operation.
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			if ( post_ops_list_temp->scale_factor_len > 1 )
			{
				selector1 = _mm512_maskz_loadu_epi32
							(
							  load_mask,
							  ( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( post_ops_list_temp->scale_factor_len == 1 )
			{
				selector1 =
					( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			}

			// Need to ensure sse not used to avoid avx512 -> sse transition.
			__m128i zero_point = _mm512_castsi512_si128( _mm512_setzero_si512() );
			if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
			{
				zero_point = _mm_maskz_loadu_epi8
							(
							  load_mask,
							  ( ( int8_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
			{
				zero_point = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			}

			// c[0, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_0p0,selector1,zero_point);

			// c[1, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_1p0,selector1,zero_point);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_MATRIX_ADD_2xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
			if ( post_ops_attr.c_stor_type == S8 )
			{
				int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);

				// c[1:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,1);
			}
			else
			{
				int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);

				// c[1:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,1);
			}

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_SWISH_2xLT16:
		{
			selector1 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
			__m512 al = _mm512_cvtepi32_ps( selector1 );

			__m512 fl_reg, al_in, r, r2, z, dn;

			// c[0, 0-15]
			SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			// c[1, 0-15]
			SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_2xLT16_DISABLE:
		;

		// Store the results.
		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			__mmask16 mask_all1 = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results in downscaled type (int8 instead of int32).
			// c[0,0-15]
			CVT_STORE_S32_S8(c_int32_0p0,0,0);

			// c[1,0-15]
			CVT_STORE_S32_S8(c_int32_1p0,1,0);
		}
		else
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results.
			// c[0,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 0 ), load_mask, c_int32_0p0 );

			// c[1,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 1 ), load_mask, c_int32_1p0 );
		}
	}
}

// 1xlt16 int8o32 fringe kernel
LPGEMM_MN_LT_NR0_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_1xlt16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_1xLT16_DISABLE,
						  &&POST_OPS_BIAS_1xLT16,
						  &&POST_OPS_RELU_1xLT16,
						  &&POST_OPS_RELU_SCALE_1xLT16,
						  &&POST_OPS_GELU_TANH_1xLT16,
						  &&POST_OPS_GELU_ERF_1xLT16,
						  &&POST_OPS_CLIP_1xLT16,
						  &&POST_OPS_DOWNSCALE_1xLT16,
						  &&POST_OPS_MATRIX_ADD_1xLT16,
						  &&POST_OPS_SWISH_1xLT16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	{
		// Registers to use for accumulating C.
		__m512i c_int32_0p0 = _mm512_setzero_epi32();

		for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
		{
			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		}
		// Handle k remainder.
		if ( k_partial_pieces > 0 )
		{
			__m128i a_kfringe_buf;
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

			__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

			// Broadcast a[0,kr:kr+4].
			a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
			__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

			//convert signed int8 to uint8 for VNNI
			a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

			// Perform column direction mat-mul with k = 4.
			// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
			c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		}

		if ( post_ops_attr.is_last_k == 1 )
		{
			//Subtract B matrix sum column values to compensate
			//for addition of 128 to A matrix elements

			int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

			__m512i b0 = _mm512_loadu_si512( bsumptr);

			c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		}

		// Load alpha and beta
		__m512i selector1 = _mm512_set1_epi32( alpha );
		__m512i selector2 = _mm512_set1_epi32( beta );

		if ( alpha != 1 )
		{
			// Scale by alpha
			c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );
		}

		// Scale C by beta.
		if ( beta != 0 )
		{
			if ( ( post_ops_attr.buf_downscale != NULL ) &&
				 ( post_ops_attr.is_first_k == TRUE ) )
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S8_S32_BETA_OP_NLT16F_MASK( load_mask, c_int32_0p0, 0, 0, \
								selector1, selector2 );
			}
			else
			{
				__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

				// c[0,0-15]
				S32_S32_BETA_OP_NLT16F_MASK(c, load_mask, c_int32_0p0, 0, 0, 0, \
								selector1, selector2);
			}
		}

		// Post Ops
		lpgemm_post_op* post_ops_list_temp = post_ops_list;
		POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_1xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			selector1 = _mm512_maskz_loadu_epi32
			(
			  load_mask,
			  ( ( int32_t* )post_ops_list_temp->op_args1 +
				post_ops_attr.post_op_c_j )
			);

			// c[0,0-15]
			c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_1xLT16:
		{
			selector1 = _mm512_setzero_epi32();

			// c[0,0-15]
			c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_RELU_SCALE_1xLT16:
		{
			selector1 = _mm512_setzero_epi32();
			selector2 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

			__mmask16 relu_cmp_mask;

			// c[0, 0-15]
			RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_TANH_1xLT16:
		{
			__m512 dn, z, x, r2, r, y, x_tanh;
			__m512i q;

			// c[0, 0-15]
			GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_GELU_ERF_1xLT16:
		{
			__m512 x, r, y, x_erf;

			// c[0, 0-15]
			GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_CLIP_1xLT16:
		{
			__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
			__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

			// c[0, 0-15]
			CLIP_S32_AVX512(c_int32_0p0, min, max)

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_DOWNSCALE_1xLT16:
		{
			// Typecast without data modification, safe operation.
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			if ( post_ops_list_temp->scale_factor_len > 1 )
			{
				selector1 = _mm512_maskz_loadu_epi32
							(
							  load_mask,
							  ( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( post_ops_list_temp->scale_factor_len == 1 )
			{
				selector1 =
					( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			}

			// Need to ensure sse not used to avoid avx512 -> sse transition.
			__m128i zero_point = _mm512_castsi512_si128( _mm512_setzero_si512() );
			if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
			{
				zero_point = _mm_maskz_loadu_epi8
							(
							  load_mask,
							  ( ( int8_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j )
							);
			}
			else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
			{
				zero_point = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			}

			// c[0, 0-15]
			CVT_MULRND_CVT32_LT16(c_int32_0p0,selector1,zero_point);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_MATRIX_ADD_1xLT16:
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );
			dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
			if ( post_ops_attr.c_stor_type == S8 )
			{
				int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S8_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);
			}
			else
			{
				int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

				// c[0:0-15]
				S32_S32_MATRIX_ADD_1COL_PAR(load_mask,selector1,0);
			}

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_SWISH_1xLT16:
		{
			selector1 =
				_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
			__m512 al = _mm512_cvtepi32_ps( selector1 );

			__m512 fl_reg, al_in, r, r2, z, dn;

			// c[0, 0-15]
			SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

			POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
		}
POST_OPS_1xLT16_DISABLE:
		;

		// Store the results.
		if ( ( post_ops_attr.buf_downscale != NULL ) && ( post_ops_attr.is_last_k == TRUE ) )
		{
			__mmask16 mask_all1 = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results in downscaled type (int8 instead of int32).
			// c[0,0-15]
			CVT_STORE_S32_S8(c_int32_0p0,0,0);
		}
		else
		{
			__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - n0_rem ) );

			// Store the results.
			// c[0,0-15]
			_mm512_mask_storeu_epi32( c + ( rs_c * 0 ), load_mask, c_int32_0p0 );
		}
	}
}

// 5x16 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_5x16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_5x16_DISABLE,
						  &&POST_OPS_BIAS_5x16,
						  &&POST_OPS_RELU_5x16,
						  &&POST_OPS_RELU_SCALE_5x16,
						  &&POST_OPS_GELU_TANH_5x16,
						  &&POST_OPS_GELU_ERF_5x16,
						  &&POST_OPS_CLIP_5x16,
						  &&POST_OPS_DOWNSCALE_5x16,
						  &&POST_OPS_MATRIX_ADD_5x16,
						  &&POST_OPS_SWISH_5x16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();

	__m512i c_int32_1p0 = _mm512_setzero_epi32();

	__m512i c_int32_2p0 = _mm512_setzero_epi32();

	__m512i c_int32_3p0 = _mm512_setzero_epi32();

	__m512i c_int32_4p0 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

		// Broadcast a[3,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );

		// Broadcast a[4,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 4 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

		// Broadcast a[3,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );

		// Broadcast a[4,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[4,0-15] = a[4,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		__m512i b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
		c_int32_3p0 = _mm512_sub_epi32( c_int32_3p0 , b0 );
		c_int32_4p0 = _mm512_sub_epi32( c_int32_4p0 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15]
			S8_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);

			// c[1:0-15]
			S8_S32_BETA_OP(c_int32_1p0,0,1,0,selector1,selector2);

			// c[2:0-15]
			S8_S32_BETA_OP(c_int32_2p0,0,2,0,selector1,selector2);

			// c[3:0-15]
			S8_S32_BETA_OP(c_int32_3p0,0,3,0,selector1,selector2);

			// c[4:0-15]
			S8_S32_BETA_OP(c_int32_4p0,0,4,0,selector1,selector2);
		}
		else
		{
			// c[0:0-15]
			S32_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);

			// c[1:0-15]
			S32_S32_BETA_OP(c_int32_1p0,0,1,0,selector1,selector2);

			// c[2:0-15]
			S32_S32_BETA_OP(c_int32_2p0,0,2,0,selector1,selector2);

			// c[3:0-15]
			S32_S32_BETA_OP(c_int32_3p0,0,3,0,selector1,selector2);

			// c[4:0-15]
			S32_S32_BETA_OP(c_int32_4p0,0,4,0,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_5x16:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_5x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_5x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_5x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_5x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_5x16:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		// c[3, 0-15]
		CLIP_S32_AVX512(c_int32_3p0, min, max)

		// c[4, 0-15]
		CLIP_S32_AVX512(c_int32_4p0, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_5x16:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1,zero_point0);

		// c[4, 0-15]
		CVT_MULRND_CVT32(c_int32_4p0,selector1,zero_point0);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_5x16:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,0);

			// c[1:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,1);

			// c[2:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,2);

			// c[3:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,3);

			// c[4:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,4);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,0);

			// c[1:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,1);

			// c[2:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,2);

			// c[3:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,3);

			// c[4:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,4);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_5x16:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 0-15]
		SWISH_S32_AVX512(c_int32_3p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[4, 0-15]
		SWISH_S32_AVX512(c_int32_4p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_5x16_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );

		// c[3,0-15]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 0*16 ), c_int32_3p0 );

		// c[4,0-15]
		_mm512_storeu_si512( c + ( rs_c * 4 ) + ( 0*16 ), c_int32_4p0 );
	}
}

// 4x16 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_4x16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_4x16_DISABLE,
						  &&POST_OPS_BIAS_4x16,
						  &&POST_OPS_RELU_4x16,
						  &&POST_OPS_RELU_SCALE_4x16,
						  &&POST_OPS_GELU_TANH_4x16,
						  &&POST_OPS_GELU_ERF_4x16,
						  &&POST_OPS_CLIP_4x16,
						  &&POST_OPS_DOWNSCALE_4x16,
						  &&POST_OPS_MATRIX_ADD_4x16,
						  &&POST_OPS_SWISH_4x16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();

	__m512i c_int32_1p0 = _mm512_setzero_epi32();

	__m512i c_int32_2p0 = _mm512_setzero_epi32();

	__m512i c_int32_3p0 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

		// Broadcast a[3,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

        //convert signed int8 to uint8 for VNNI
        a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );

		// Broadcast a[3,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-15] = a[3,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		__m512i b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
		c_int32_3p0 = _mm512_sub_epi32( c_int32_3p0 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15]
			S8_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);

			// c[1:0-15]
			S8_S32_BETA_OP(c_int32_1p0,0,1,0,selector1,selector2);

			// c[2:0-15]
			S8_S32_BETA_OP(c_int32_2p0,0,2,0,selector1,selector2);

			// c[3:0-15]
			S8_S32_BETA_OP(c_int32_3p0,0,3,0,selector1,selector2);
		}
		else
		{
			// c[0:0-15]
			S32_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);

			// c[1:0-15]
			S32_S32_BETA_OP(c_int32_1p0,0,1,0,selector1,selector2);

			// c[2:0-15]
			S32_S32_BETA_OP(c_int32_2p0,0,2,0,selector1,selector2);

			// c[3:0-15]
			S32_S32_BETA_OP(c_int32_3p0,0,3,0,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_4x16:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j );

		// c[0,0-15]
		c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

		// c[1,0-15]
		c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

		// c[2,0-15]
		c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

		// c[3,0-15]
		c_int32_3p0 = _mm512_add_epi32( selector1, c_int32_3p0 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_4x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_4x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_4x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_4x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_4x16:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		// c[3, 0-15]
		CLIP_S32_AVX512(c_int32_3p0, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_4x16:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1,zero_point0);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_4x16:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,0);

			// c[1:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,1);

			// c[2:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,2);

			// c[3:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,3);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,0);

			// c[1:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,1);

			// c[2:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,2);

			// c[3:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,3);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_4x16:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 0-15]
		SWISH_S32_AVX512(c_int32_3p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_4x16_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );

		// c[3,0-15]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 0*16 ), c_int32_3p0 );
	}
}

// 3x16 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_3x16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_3x16_DISABLE,
						  &&POST_OPS_BIAS_3x16,
						  &&POST_OPS_RELU_3x16,
						  &&POST_OPS_RELU_SCALE_3x16,
						  &&POST_OPS_GELU_TANH_3x16,
						  &&POST_OPS_GELU_ERF_3x16,
						  &&POST_OPS_CLIP_3x16,
						  &&POST_OPS_DOWNSCALE_3x16,
						  &&POST_OPS_MATRIX_ADD_3x16,
						  &&POST_OPS_SWISH_3x16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();

	__m512i c_int32_1p0 = _mm512_setzero_epi32();

	__m512i c_int32_2p0 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-15] = a[2,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		__m512i b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15]
			S8_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);

			// c[1:0-15]
			S8_S32_BETA_OP(c_int32_1p0,0,1,0,selector1,selector2);

			// c[2:0-15]
			S8_S32_BETA_OP(c_int32_2p0,0,2,0,selector1,selector2);
		}
		else
		{
			// c[0:0-15]
			S32_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);

			// c[1:0-15]
			S32_S32_BETA_OP(c_int32_1p0,0,1,0,selector1,selector2);

			// c[2:0-15]
			S32_S32_BETA_OP(c_int32_2p0,0,2,0,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_3x16:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j );

		// c[0,0-15]
		c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

		// c[1,0-15]
		c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

		// c[2,0-15]
		c_int32_2p0 = _mm512_add_epi32( selector1, c_int32_2p0 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_3x16:
	{
		selector1 = _mm512_setzero_epi32();

		// c[0,0-15]
		c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

		// c[1,0-15]
		c_int32_1p0 = _mm512_max_epi32( selector1, c_int32_1p0 );

		// c[2,0-15]
		c_int32_2p0 = _mm512_max_epi32( selector1, c_int32_2p0 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_3x16:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_3x16:
	{
		__m512 dn, z, x, r2, r, y, x_tanh;
		__m512i q;

		// c[0, 0-15]
		GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

		// c[1, 0-15]
		GELU_TANH_S32_AVX512(c_int32_1p0, y, r, r2, x, z, dn, x_tanh, q)

		// c[2, 0-15]
		GELU_TANH_S32_AVX512(c_int32_2p0, y, r, r2, x, z, dn, x_tanh, q)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_3x16:
	{
		__m512 x, r, y, x_erf;

		// c[0, 0-15]
		GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

		// c[1, 0-15]
		GELU_ERF_S32_AVX512(c_int32_1p0, y, r, x, x_erf)

		// c[2, 0-15]
		GELU_ERF_S32_AVX512(c_int32_2p0, y, r, x, x_erf)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_3x16:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_3x16:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_3x16:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,0);

			// c[1:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,1);

			// c[2:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,2);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,0);

			// c[1:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,1);

			// c[2:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,2);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_3x16:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_3x16_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );
	}
}

// 2x16 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_2x16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_2x16_DISABLE,
						  &&POST_OPS_BIAS_2x16,
						  &&POST_OPS_RELU_2x16,
						  &&POST_OPS_RELU_SCALE_2x16,
						  &&POST_OPS_GELU_TANH_2x16,
						  &&POST_OPS_GELU_ERF_2x16,
						  &&POST_OPS_CLIP_2x16,
						  &&POST_OPS_DOWNSCALE_2x16,
						  &&POST_OPS_MATRIX_ADD_2x16,
						  &&POST_OPS_SWISH_2x16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();

	__m512i c_int32_1p0 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-15] = a[1,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		__m512i b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
	}

	// Load alpha and beta
	__m512i selector1 = _mm512_set1_epi32( alpha );
	__m512i selector2 = _mm512_set1_epi32( beta );

	if ( alpha != 1 )
	{
		// Scale by alpha
		c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );

		c_int32_1p0 = _mm512_mullo_epi32( selector1, c_int32_1p0 );
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15]
			S8_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);

			// c[1:0-15]
			S8_S32_BETA_OP(c_int32_1p0,0,1,0,selector1,selector2);
		}
		else
		{
			// c[0:0-15]
			S32_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);

			// c[1:0-15]
			S32_S32_BETA_OP(c_int32_1p0,0,1,0,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_2x16:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j );

		// c[0,0-15]
		c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

		// c[1,0-15]
		c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_2x16:
	{
		selector1 = _mm512_setzero_epi32();

		// c[0,0-15]
		c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

		// c[1,0-15]
		c_int32_1p0 = _mm512_max_epi32( selector1, c_int32_1p0 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_2x16:
	{
		selector1 = _mm512_setzero_epi32();
		selector2 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

		__mmask16 relu_cmp_mask;

		// c[0, 0-15]
		RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

		// c[1, 0-15]
		RELU_SCALE_OP_S32_AVX512(c_int32_1p0)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_2x16:
	{
		__m512 dn, z, x, r2, r, y, x_tanh;
		__m512i q;

		// c[0, 0-15]
		GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

		// c[1, 0-15]
		GELU_TANH_S32_AVX512(c_int32_1p0, y, r, r2, x, z, dn, x_tanh, q)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_2x16:
	{
		__m512 x, r, y, x_erf;

		// c[0, 0-15]
		GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

		// c[1, 0-15]
		GELU_ERF_S32_AVX512(c_int32_1p0, y, r, x, x_erf)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_2x16:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_2x16:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_2x16:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,0);

			// c[1:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,1);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,0);

			// c[1:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,1);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_2x16:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_2x16_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );
	}
}

// 1x16 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_1x16)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_1x16_DISABLE,
						  &&POST_OPS_BIAS_1x16,
						  &&POST_OPS_RELU_1x16,
						  &&POST_OPS_RELU_SCALE_1x16,
						  &&POST_OPS_GELU_TANH_1x16,
						  &&POST_OPS_GELU_ERF_1x16,
						  &&POST_OPS_CLIP_1x16,
						  &&POST_OPS_DOWNSCALE_1x16,
						  &&POST_OPS_MATRIX_ADD_1x16,
						  &&POST_OPS_SWISH_1x16
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		__m512i a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		__m512i b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		__m512i a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-15] = a[0,kr:kr+4]*b[kr:kr+4,0-15]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		__m512i b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
	}

	// Load alpha and beta
	__m512i selector1 = _mm512_set1_epi32( alpha );
	__m512i selector2 = _mm512_set1_epi32( beta );

	if ( alpha != 1 )
	{
		// Scale by alpha
		c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15]
			S8_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);
		}
		else
		{
			// c[0:0-15]
			S32_S32_BETA_OP(c_int32_0p0,0,0,0,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_1x16:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j );

		// c[0,0-15]
		c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_1x16:
	{
		selector1 = _mm512_setzero_epi32();

		// c[0,0-15]
		c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_1x16:
	{
		selector1 = _mm512_setzero_epi32();
		selector2 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

		__mmask16 relu_cmp_mask;

		// c[0, 0-15]
		RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_1x16:
	{
		__m512 dn, z, x, r2, r, y, x_tanh;
		__m512i q;

		// c[0, 0-15]
		GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_1x16:
	{
		__m512 x, r, y, x_erf;

		// c[0, 0-15]
		GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_1x16:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_1x16:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_1x16:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S8_S32_MATRIX_ADD_1COL(selector1,0);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15]
			S32_S32_MATRIX_ADD_1COL(selector1,0);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_1x16:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_1x16_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );
	}
}

// 5x32 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_5x32)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_5x32_DISABLE,
						  &&POST_OPS_BIAS_5x32,
						  &&POST_OPS_RELU_5x32,
						  &&POST_OPS_RELU_SCALE_5x32,
						  &&POST_OPS_GELU_TANH_5x32,
						  &&POST_OPS_GELU_ERF_5x32,
						  &&POST_OPS_CLIP_5x32,
						  &&POST_OPS_DOWNSCALE_5x32,
						  &&POST_OPS_MATRIX_ADD_5x32,
						  &&POST_OPS_SWISH_5x32
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

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

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );

		// Broadcast a[3,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-31] = a[3,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );

		// Broadcast a[4,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 4 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[4,0-31] = a[4,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
		c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );

		// Broadcast a[3,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-31] = a[3,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );

		// Broadcast a[4,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[4,0-31] = a[4,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
		c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
		c_int32_3p0 = _mm512_sub_epi32( c_int32_3p0 , b0 );
		c_int32_4p0 = _mm512_sub_epi32( c_int32_4p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16 );

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
		c_int32_1p1 = _mm512_sub_epi32( c_int32_1p1 , b0 );
		c_int32_2p1 = _mm512_sub_epi32( c_int32_2p1 , b0 );
		c_int32_3p1 = _mm512_sub_epi32( c_int32_3p1 , b0 );
		c_int32_4p1 = _mm512_sub_epi32( c_int32_4p1 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31]
			S8_S32_BETA_OP2(0,0,selector1,selector2);

			// c[1:0-15,16-31]
			S8_S32_BETA_OP2(0,1,selector1,selector2);

			// c[2:0-15,16-31]
			S8_S32_BETA_OP2(0,2,selector1,selector2);

			// c[3:0-15,16-31]
			S8_S32_BETA_OP2(0,3,selector1,selector2);

			// c[4:0-15,16-31]
			S8_S32_BETA_OP2(0,4,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31]
			S32_S32_BETA_OP2(0,0,selector1,selector2);

			// c[1:0-15,16-31]
			S32_S32_BETA_OP2(0,1,selector1,selector2);

			// c[2:0-15,16-31]
			S32_S32_BETA_OP2(0,2,selector1,selector2);

			// c[3:0-15,16-31]
			S32_S32_BETA_OP2(0,3,selector1,selector2);

			// c[4:0-15,16-31]
			S32_S32_BETA_OP2(0,4,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_5x32:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_5x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_5x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_5x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_5x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_5x32:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[1, 16-31]
		CLIP_S32_AVX512(c_int32_1p1, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		// c[2, 16-31]
		CLIP_S32_AVX512(c_int32_2p1, min, max)

		// c[3, 0-15]
		CLIP_S32_AVX512(c_int32_3p0, min, max)

		// c[3, 16-31]
		CLIP_S32_AVX512(c_int32_3p1, min, max)

		// c[4, 0-15]
		CLIP_S32_AVX512(c_int32_4p0, min, max)

		// c[4, 16-31]
		CLIP_S32_AVX512(c_int32_4p1, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_5x32:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2,zero_point1);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		// c[2, 16-31]
		CVT_MULRND_CVT32(c_int32_2p1,selector2,zero_point1);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1,zero_point0);

		// c[3, 16-31]
		CVT_MULRND_CVT32(c_int32_3p1,selector2,zero_point1);

		// c[4, 0-15]
		CVT_MULRND_CVT32(c_int32_4p0,selector1,zero_point0);

		// c[4, 16-31]
		CVT_MULRND_CVT32(c_int32_4p1,selector2,zero_point1);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_5x32:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,0);

			// c[1:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,1);

			// c[2:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,2);

			// c[3:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,3);

			// c[4:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,4);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,0);

			// c[1:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,1);

			// c[2:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,2);

			// c[3:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,3);

			// c[4:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,4);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_5x32:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 16-31]
		SWISH_S32_AVX512(c_int32_1p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 16-31]
		SWISH_S32_AVX512(c_int32_2p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 0-15]
		SWISH_S32_AVX512(c_int32_3p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 16-31]
		SWISH_S32_AVX512(c_int32_3p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[4, 0-15]
		SWISH_S32_AVX512(c_int32_4p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[4, 16-31]
		SWISH_S32_AVX512(c_int32_4p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_5x32_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 1*16 ), c_int32_1p1 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );

		// c[2,16-31]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 1*16 ), c_int32_2p1 );

		// c[3,0-15]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 0*16 ), c_int32_3p0 );

		// c[3,16-31]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 1*16 ), c_int32_3p1 );

		// c[4,0-15]
		_mm512_storeu_si512( c + ( rs_c * 4 ) + ( 0*16 ), c_int32_4p0 );

		// c[4,16-31]
		_mm512_storeu_si512( c + ( rs_c * 4 ) + ( 1*16 ), c_int32_4p1 );
	}
}

// 4x32 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_4x32)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_4x32_DISABLE,
						  &&POST_OPS_BIAS_4x32,
						  &&POST_OPS_RELU_4x32,
						  &&POST_OPS_RELU_SCALE_4x32,
						  &&POST_OPS_GELU_TANH_4x32,
						  &&POST_OPS_GELU_ERF_4x32,
						  &&POST_OPS_CLIP_4x32,
						  &&POST_OPS_DOWNSCALE_4x32,
						  &&POST_OPS_MATRIX_ADD_4x32,
						  &&POST_OPS_SWISH_4x32
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();
	__m512i c_int32_0p1 = _mm512_setzero_epi32();

	__m512i c_int32_1p0 = _mm512_setzero_epi32();
	__m512i c_int32_1p1 = _mm512_setzero_epi32();

	__m512i c_int32_2p0 = _mm512_setzero_epi32();
	__m512i c_int32_2p1 = _mm512_setzero_epi32();

	__m512i c_int32_3p0 = _mm512_setzero_epi32();
	__m512i c_int32_3p1 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );

		// Broadcast a[3,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-31] = a[3,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );

		// Broadcast a[3,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-31] = a[3,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
		c_int32_3p0 = _mm512_sub_epi32( c_int32_3p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16 );

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
		c_int32_1p1 = _mm512_sub_epi32( c_int32_1p1 , b0 );
		c_int32_2p1 = _mm512_sub_epi32( c_int32_2p1 , b0 );
		c_int32_3p1 = _mm512_sub_epi32( c_int32_3p1 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31]
			S8_S32_BETA_OP2(0,0,selector1,selector2);

			// c[1:0-15,16-31]
			S8_S32_BETA_OP2(0,1,selector1,selector2);

			// c[2:0-15,16-31]
			S8_S32_BETA_OP2(0,2,selector1,selector2);

			// c[3:0-15,16-31]
			S8_S32_BETA_OP2(0,3,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31]
			S32_S32_BETA_OP2(0,0,selector1,selector2);

			// c[1:0-15,16-31]
			S32_S32_BETA_OP2(0,1,selector1,selector2);

			// c[2:0-15,16-31]
			S32_S32_BETA_OP2(0,2,selector1,selector2);

			// c[3:0-15,16-31]
			S32_S32_BETA_OP2(0,3,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_4x32:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_4x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_4x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_4x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_4x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_4x32:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[1, 16-31]
		CLIP_S32_AVX512(c_int32_1p1, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		// c[2, 16-31]
		CLIP_S32_AVX512(c_int32_2p1, min, max)

		// c[3, 0-15]
		CLIP_S32_AVX512(c_int32_3p0, min, max)

		// c[3, 16-31]
		CLIP_S32_AVX512(c_int32_3p1, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_4x32:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2,zero_point1);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		// c[2, 16-31]
		CVT_MULRND_CVT32(c_int32_2p1,selector2,zero_point1);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1,zero_point0);

		// c[3, 16-31]
		CVT_MULRND_CVT32(c_int32_3p1,selector2,zero_point1);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_4x32:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,0);

			// c[1:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,1);

			// c[2:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,2);

			// c[3:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,3);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,0);

			// c[1:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,1);

			// c[2:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,2);

			// c[3:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,3);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_4x32:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 16-31]
		SWISH_S32_AVX512(c_int32_1p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 16-31]
		SWISH_S32_AVX512(c_int32_2p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 0-15]
		SWISH_S32_AVX512(c_int32_3p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 16-31]
		SWISH_S32_AVX512(c_int32_3p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_4x32_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 1*16 ), c_int32_1p1 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );

		// c[2,16-31]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 1*16 ), c_int32_2p1 );

		// c[3,0-15]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 0*16 ), c_int32_3p0 );

		// c[3,16-31]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 1*16 ), c_int32_3p1 );
	}
}

// 3x32 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_3x32)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_3x32_DISABLE,
						  &&POST_OPS_BIAS_3x32,
						  &&POST_OPS_RELU_3x32,
						  &&POST_OPS_RELU_SCALE_3x32,
						  &&POST_OPS_GELU_TANH_3x32,
						  &&POST_OPS_GELU_ERF_3x32,
						  &&POST_OPS_CLIP_3x32,
						  &&POST_OPS_DOWNSCALE_3x32,
						  &&POST_OPS_MATRIX_ADD_3x32,
						  &&POST_OPS_SWISH_3x32
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();
	__m512i c_int32_0p1 = _mm512_setzero_epi32();

	__m512i c_int32_1p0 = _mm512_setzero_epi32();
	__m512i c_int32_1p1 = _mm512_setzero_epi32();

	__m512i c_int32_2p0 = _mm512_setzero_epi32();
	__m512i c_int32_2p1 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-31] = a[2,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16 );

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
		c_int32_1p1 = _mm512_sub_epi32( c_int32_1p1 , b0 );
		c_int32_2p1 = _mm512_sub_epi32( c_int32_2p1 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31]
			S8_S32_BETA_OP2(0,0,selector1,selector2);

			// c[1:0-15,16-31]
			S8_S32_BETA_OP2(0,1,selector1,selector2);

			// c[2:0-15,16-31]
			S8_S32_BETA_OP2(0,2,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31]
			S32_S32_BETA_OP2(0,0,selector1,selector2);

			// c[1:0-15,16-31]
			S32_S32_BETA_OP2(0,1,selector1,selector2);

			// c[2:0-15,16-31]
			S32_S32_BETA_OP2(0,2,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_3x32:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_3x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_3x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_3x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_3x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_3x32:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[1, 16-31]
		CLIP_S32_AVX512(c_int32_1p1, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		// c[2, 16-31]
		CLIP_S32_AVX512(c_int32_2p1, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_3x32:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2,zero_point1);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		// c[2, 16-31]
		CVT_MULRND_CVT32(c_int32_2p1,selector2,zero_point1);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_3x32:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,0);

			// c[1:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,1);

			// c[2:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,2);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,0);

			// c[1:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,1);

			// c[2:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,2);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_3x32:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 16-31]
		SWISH_S32_AVX512(c_int32_1p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 16-31]
		SWISH_S32_AVX512(c_int32_2p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_3x32_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 1*16 ), c_int32_1p1 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );

		// c[2,16-31]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 1*16 ), c_int32_2p1 );
	}
}

// 2x32 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_2x32)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_2x32_DISABLE,
						  &&POST_OPS_BIAS_2x32,
						  &&POST_OPS_RELU_2x32,
						  &&POST_OPS_RELU_SCALE_2x32,
						  &&POST_OPS_GELU_TANH_2x32,
						  &&POST_OPS_GELU_ERF_2x32,
						  &&POST_OPS_CLIP_2x32,
						  &&POST_OPS_DOWNSCALE_2x32,
						  &&POST_OPS_MATRIX_ADD_2x32,
						  &&POST_OPS_SWISH_2x32
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();
	__m512i c_int32_0p1 = _mm512_setzero_epi32();

	__m512i c_int32_1p0 = _mm512_setzero_epi32();
	__m512i c_int32_1p1 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-31] = a[1,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16 );

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
		c_int32_1p1 = _mm512_sub_epi32( c_int32_1p1 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31]
			S8_S32_BETA_OP2(0,0,selector1,selector2);

			// c[1:0-15,16-31]
			S8_S32_BETA_OP2(0,1,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31]
			S32_S32_BETA_OP2(0,0,selector1,selector2);

			// c[1:0-15,16-31]
			S32_S32_BETA_OP2(0,1,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_2x32:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );

		// c[0,0-15]
		c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

		// c[0, 16-31]
		c_int32_0p1 = _mm512_add_epi32( selector2, c_int32_0p1 );

		// c[1,0-15]
		c_int32_1p0 = _mm512_add_epi32( selector1, c_int32_1p0 );

		// c[1, 16-31]
		c_int32_1p1 = _mm512_add_epi32( selector2, c_int32_1p1 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_2x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_2x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_2x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_2x32:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_2x32:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[1, 16-31]
		CLIP_S32_AVX512(c_int32_1p1, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_2x32:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2,zero_point1);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_2x32:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,0);

			// c[1:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,1);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,0);

			// c[1:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,1);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_2x32:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 16-31]
		SWISH_S32_AVX512(c_int32_1p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_2x32_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 1*16 ), c_int32_1p1 );
	}
}

// 1x32 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_1x32)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_1x32_DISABLE,
						  &&POST_OPS_BIAS_1x32,
						  &&POST_OPS_RELU_1x32,
						  &&POST_OPS_RELU_SCALE_1x32,
						  &&POST_OPS_GELU_TANH_1x32,
						  &&POST_OPS_GELU_ERF_1x32,
						  &&POST_OPS_CLIP_1x32,
						  &&POST_OPS_DOWNSCALE_1x32,
						  &&POST_OPS_MATRIX_ADD_1x32,
						  &&POST_OPS_SWISH_1x32
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();
	__m512i c_int32_0p1 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-31] = a[0,kr:kr+4]*b[kr:kr+4,0-31]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16);

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
	}

	// Load alpha and beta
	__m512i selector1 = _mm512_set1_epi32( alpha );
	__m512i selector2 = _mm512_set1_epi32( beta );

	if ( alpha != 1 )
	{
		// Scale by alpha
		c_int32_0p0 = _mm512_mullo_epi32( selector1, c_int32_0p0 );
		c_int32_0p1 = _mm512_mullo_epi32( selector1, c_int32_0p1 );
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31]
			S8_S32_BETA_OP2(0,0,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31]
			S32_S32_BETA_OP2(0,0,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_1x32:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );

		// c[0,0-15]
		c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

		// c[0, 16-31]
		c_int32_0p1 = _mm512_add_epi32( selector2, c_int32_0p1 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_1x32:
	{
		selector1 = _mm512_setzero_epi32();

		// c[0,0-15]
		c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

		// c[0, 16-31]
		c_int32_0p1 = _mm512_max_epi32( selector1, c_int32_0p1 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_1x32:
	{
		selector1 = _mm512_setzero_epi32();
		selector2 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );

		__mmask16 relu_cmp_mask;

		// c[0, 0-15]
		RELU_SCALE_OP_S32_AVX512(c_int32_0p0)

		// c[0, 16-31]
		RELU_SCALE_OP_S32_AVX512(c_int32_0p1)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_1x32:
	{
		__m512 dn, z, x, r2, r, y, x_tanh;
		__m512i q;

		// c[0, 0-15]
		GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

		// c[0, 16-31]
		GELU_TANH_S32_AVX512(c_int32_0p1, y, r, r2, x, z, dn, x_tanh, q)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_1x32:
	{
		__m512 x, r, y, x_erf;

		// c[0, 0-15]
		GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

		// c[0, 16-31]
		GELU_ERF_S32_AVX512(c_int32_0p1, y, r, x, x_erf)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_1x32:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_1x32:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_1x32:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S8_S32_MATRIX_ADD_2COL(selector1,selector2,0);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31]
			S32_S32_MATRIX_ADD_2COL(selector1,selector2,0);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_1x32:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_1x32_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );
	}
}

// 5x48 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_5x48)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_5x48_DISABLE,
						  &&POST_OPS_BIAS_5x48,
						  &&POST_OPS_RELU_5x48,
						  &&POST_OPS_RELU_SCALE_5x48,
						  &&POST_OPS_GELU_TANH_5x48,
						  &&POST_OPS_GELU_ERF_5x48,
						  &&POST_OPS_CLIP_5x48,
						  &&POST_OPS_DOWNSCALE_5x48,
						  &&POST_OPS_MATRIX_ADD_5x48,
						  &&POST_OPS_SWISH_5x48
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();
	__m512i b2 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

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

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
		c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
		c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );

		// Broadcast a[3,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-47] = a[3,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
		c_int32_3p2 = _mm512_dpbusd_epi32( c_int32_3p2, a_int32_0, b2 );

		// Broadcast a[4,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 4 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[4,0-47] = a[4,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
		c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
		c_int32_4p2 = _mm512_dpbusd_epi32( c_int32_4p2, a_int32_0, b2 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
		c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
		c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );

		// Broadcast a[3,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-47] = a[3,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
		c_int32_3p2 = _mm512_dpbusd_epi32( c_int32_3p2, a_int32_0, b2 );

		// Broadcast a[4,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 4 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[4,0-47] = a[4,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_4p0 = _mm512_dpbusd_epi32( c_int32_4p0, a_int32_0, b0 );
		c_int32_4p1 = _mm512_dpbusd_epi32( c_int32_4p1, a_int32_0, b1 );
		c_int32_4p2 = _mm512_dpbusd_epi32( c_int32_4p2, a_int32_0, b2 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
		c_int32_3p0 = _mm512_sub_epi32( c_int32_3p0 , b0 );
		c_int32_4p0 = _mm512_sub_epi32( c_int32_4p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16 );

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
		c_int32_1p1 = _mm512_sub_epi32( c_int32_1p1 , b0 );
		c_int32_2p1 = _mm512_sub_epi32( c_int32_2p1 , b0 );
		c_int32_3p1 = _mm512_sub_epi32( c_int32_3p1 , b0 );
		c_int32_4p1 = _mm512_sub_epi32( c_int32_4p1 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 32 );

		c_int32_0p2 = _mm512_sub_epi32( c_int32_0p2 , b0 );
		c_int32_1p2 = _mm512_sub_epi32( c_int32_1p2 , b0 );
		c_int32_2p2 = _mm512_sub_epi32( c_int32_2p2 , b0 );
		c_int32_3p2 = _mm512_sub_epi32( c_int32_3p2 , b0 );
		c_int32_4p2 = _mm512_sub_epi32( c_int32_4p2 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,0,selector1,selector2);

			// c[1:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,1,selector1,selector2);

			// c[2:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,2,selector1,selector2);

			// c[3:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,3,selector1,selector2);

			// c[4:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,4,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,0,selector1,selector2);

			// c[1:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,1,selector1,selector2);

			// c[2:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,2,selector1,selector2);

			// c[3:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,3,selector1,selector2);

			// c[4:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,4,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_5x48:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		a_int32_0 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_5x48:
	{
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_5x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_5x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_5x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_5x48:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[0, 32-47]
		CLIP_S32_AVX512(c_int32_0p2, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[1, 16-31]
		CLIP_S32_AVX512(c_int32_1p1, min, max)

		// c[1, 32-47]
		CLIP_S32_AVX512(c_int32_1p2, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		// c[2, 16-31]
		CLIP_S32_AVX512(c_int32_2p1, min, max)

		// c[2, 32-47]
		CLIP_S32_AVX512(c_int32_2p2, min, max)

		// c[3, 0-15]
		CLIP_S32_AVX512(c_int32_3p0, min, max)

		// c[3, 16-31]
		CLIP_S32_AVX512(c_int32_3p1, min, max)

		// c[3, 32-47]
		CLIP_S32_AVX512(c_int32_3p2, min, max)

		// c[4, 0-15]
		CLIP_S32_AVX512(c_int32_4p0, min, max)

		// c[4, 16-31]
		CLIP_S32_AVX512(c_int32_4p1, min, max)

		// c[4, 32-47]
		CLIP_S32_AVX512(c_int32_4p2, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_5x48:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
			a_int32_0 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 2 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			a_int32_0 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point2 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
			zero_point2 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 2 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point2 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[0, 32-47]
		CVT_MULRND_CVT32(c_int32_0p2,a_int32_0,zero_point2);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2,zero_point1);

		// c[1, 32-47]
		CVT_MULRND_CVT32(c_int32_1p2,a_int32_0,zero_point2);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		// c[2, 16-31]
		CVT_MULRND_CVT32(c_int32_2p1,selector2,zero_point1);

		// c[2, 32-47]
		CVT_MULRND_CVT32(c_int32_2p2,a_int32_0,zero_point2);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1,zero_point0);

		// c[3, 16-31]
		CVT_MULRND_CVT32(c_int32_3p1,selector2,zero_point1);

		// c[3, 32-47]
		CVT_MULRND_CVT32(c_int32_3p2,a_int32_0,zero_point2);

		// c[4, 0-15]
		CVT_MULRND_CVT32(c_int32_4p0,selector1,zero_point0);

		// c[4, 16-31]
		CVT_MULRND_CVT32(c_int32_4p1,selector2,zero_point1);

		// c[4, 32-47]
		CVT_MULRND_CVT32(c_int32_4p2,a_int32_0,zero_point2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_5x48:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);

			// c[1:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,1);

			// c[2:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,2);

			// c[3:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,3);

			// c[4:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,4);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);

			// c[1:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,1);

			// c[2:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,2);

			// c[3:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,3);

			// c[4:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,4);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_5x48:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 32-47]
		SWISH_S32_AVX512(c_int32_0p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 16-31]
		SWISH_S32_AVX512(c_int32_1p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 32-47]
		SWISH_S32_AVX512(c_int32_1p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 16-31]
		SWISH_S32_AVX512(c_int32_2p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 32-47]
		SWISH_S32_AVX512(c_int32_2p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 0-15]
		SWISH_S32_AVX512(c_int32_3p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 16-31]
		SWISH_S32_AVX512(c_int32_3p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 32-47]
		SWISH_S32_AVX512(c_int32_3p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[4, 0-15]
		SWISH_S32_AVX512(c_int32_4p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[4, 16-31]
		SWISH_S32_AVX512(c_int32_4p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[4, 32-47]
		SWISH_S32_AVX512(c_int32_4p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_5x48_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[0,32-47]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 2*16 ), c_int32_0p2 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 1*16 ), c_int32_1p1 );

		// c[1,32-47]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 2*16 ), c_int32_1p2 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );

		// c[2,16-31]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 1*16 ), c_int32_2p1 );

		// c[2,32-47]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 2*16 ), c_int32_2p2 );

		// c[3,0-15]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 0*16 ), c_int32_3p0 );

		// c[3,16-31]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 1*16 ), c_int32_3p1 );

		// c[3,32-47]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 2*16 ), c_int32_3p2 );

		// c[4,0-15]
		_mm512_storeu_si512( c + ( rs_c * 4 ) + ( 0*16 ), c_int32_4p0 );

		// c[4,16-31]
		_mm512_storeu_si512( c + ( rs_c * 4 ) + ( 1*16 ), c_int32_4p1 );

		// c[4,32-47]
		_mm512_storeu_si512( c + ( rs_c * 4 ) + ( 2*16 ), c_int32_4p2 );
	}
}

// 4x48 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_4x48)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_4x48_DISABLE,
						  &&POST_OPS_BIAS_4x48,
						  &&POST_OPS_RELU_4x48,
						  &&POST_OPS_RELU_SCALE_4x48,
						  &&POST_OPS_GELU_TANH_4x48,
						  &&POST_OPS_GELU_ERF_4x48,
						  &&POST_OPS_CLIP_4x48,
						  &&POST_OPS_DOWNSCALE_4x48,
						  &&POST_OPS_MATRIX_ADD_4x48,
						  &&POST_OPS_SWISH_4x48
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();
	__m512i b2 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

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

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
		c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
		c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );

		// Broadcast a[3,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 3 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-47] = a[3,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
		c_int32_3p2 = _mm512_dpbusd_epi32( c_int32_3p2, a_int32_0, b2 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
		c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
		c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );

		// Broadcast a[3,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 3 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[3,0-47] = a[3,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_3p0 = _mm512_dpbusd_epi32( c_int32_3p0, a_int32_0, b0 );
		c_int32_3p1 = _mm512_dpbusd_epi32( c_int32_3p1, a_int32_0, b1 );
		c_int32_3p2 = _mm512_dpbusd_epi32( c_int32_3p2, a_int32_0, b2 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );
		c_int32_3p0 = _mm512_sub_epi32( c_int32_3p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16 );

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
		c_int32_1p1 = _mm512_sub_epi32( c_int32_1p1 , b0 );
		c_int32_2p1 = _mm512_sub_epi32( c_int32_2p1 , b0 );
		c_int32_3p1 = _mm512_sub_epi32( c_int32_3p1 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 32 );

		c_int32_0p2 = _mm512_sub_epi32( c_int32_0p2 , b0 );
		c_int32_1p2 = _mm512_sub_epi32( c_int32_1p2 , b0 );
		c_int32_2p2 = _mm512_sub_epi32( c_int32_2p2 , b0 );
		c_int32_3p2 = _mm512_sub_epi32( c_int32_3p2 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,0,selector1,selector2);

			// c[1:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,1,selector1,selector2);

			// c[2:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,2,selector1,selector2);

			// c[3:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,3,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,0,selector1,selector2);

			// c[1:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,1,selector1,selector2);

			// c[2:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,2,selector1,selector2);

			// c[3:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,3,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_4x48:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		a_int32_0 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_4x48:
	{
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_4x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_4x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_4x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_4x48:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[0, 32-47]
		CLIP_S32_AVX512(c_int32_0p2, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[1, 16-31]
		CLIP_S32_AVX512(c_int32_1p1, min, max)

		// c[1, 32-47]
		CLIP_S32_AVX512(c_int32_1p2, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		// c[2, 16-31]
		CLIP_S32_AVX512(c_int32_2p1, min, max)

		// c[2, 32-47]
		CLIP_S32_AVX512(c_int32_2p2, min, max)

		// c[3, 0-15]
		CLIP_S32_AVX512(c_int32_3p0, min, max)

		// c[3, 16-31]
		CLIP_S32_AVX512(c_int32_3p1, min, max)

		// c[3, 32-47]
		CLIP_S32_AVX512(c_int32_3p2, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_4x48:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
			a_int32_0 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 2 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			a_int32_0 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point2 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
			zero_point2 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 2 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point2 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[0, 32-47]
		CVT_MULRND_CVT32(c_int32_0p2,a_int32_0,zero_point2);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2,zero_point1);

		// c[1, 32-47]
		CVT_MULRND_CVT32(c_int32_1p2,a_int32_0,zero_point2);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		// c[2, 16-31]
		CVT_MULRND_CVT32(c_int32_2p1,selector2,zero_point1);

		// c[2, 32-47]
		CVT_MULRND_CVT32(c_int32_2p2,a_int32_0,zero_point2);

		// c[3, 0-15]
		CVT_MULRND_CVT32(c_int32_3p0,selector1,zero_point0);

		// c[3, 16-31]
		CVT_MULRND_CVT32(c_int32_3p1,selector2,zero_point1);

		// c[3, 32-47]
		CVT_MULRND_CVT32(c_int32_3p2,a_int32_0,zero_point2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_4x48:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);

			// c[1:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,1);

			// c[2:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,2);

			// c[3:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,3);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);

			// c[1:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,1);

			// c[2:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,2);

			// c[3:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,3);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_4x48:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 32-47]
		SWISH_S32_AVX512(c_int32_0p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 16-31]
		SWISH_S32_AVX512(c_int32_1p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 32-47]
		SWISH_S32_AVX512(c_int32_1p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 16-31]
		SWISH_S32_AVX512(c_int32_2p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 32-47]
		SWISH_S32_AVX512(c_int32_2p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 0-15]
		SWISH_S32_AVX512(c_int32_3p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 16-31]
		SWISH_S32_AVX512(c_int32_3p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[3, 32-47]
		SWISH_S32_AVX512(c_int32_3p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_4x48_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[0,32-47]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 2*16 ), c_int32_0p2 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 1*16 ), c_int32_1p1 );

		// c[1,32-47]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 2*16 ), c_int32_1p2 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );

		// c[2,16-31]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 1*16 ), c_int32_2p1 );

		// c[2,32-47]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 2*16 ), c_int32_2p2 );

		// c[3,0-15]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 0*16 ), c_int32_3p0 );

		// c[3,16-31]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 1*16 ), c_int32_3p1 );

		// c[3,32-47]
		_mm512_storeu_si512( c + ( rs_c * 3 ) + ( 2*16 ), c_int32_3p2 );
	}
}

// 3x48 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_3x48)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_3x48_DISABLE,
						  &&POST_OPS_BIAS_3x48,
						  &&POST_OPS_RELU_3x48,
						  &&POST_OPS_RELU_SCALE_3x48,
						  &&POST_OPS_GELU_TANH_3x48,
						  &&POST_OPS_GELU_ERF_3x48,
						  &&POST_OPS_CLIP_3x48,
						  &&POST_OPS_DOWNSCALE_3x48,
						  &&POST_OPS_MATRIX_ADD_3x48,
						  &&POST_OPS_SWISH_3x48
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();
	__m512i b2 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

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

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
		c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );

		// Broadcast a[2,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 2 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
		c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
		c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );

		// Broadcast a[2,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 2 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[2,0-47] = a[2,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_2p0 = _mm512_dpbusd_epi32( c_int32_2p0, a_int32_0, b0 );
		c_int32_2p1 = _mm512_dpbusd_epi32( c_int32_2p1, a_int32_0, b1 );
		c_int32_2p2 = _mm512_dpbusd_epi32( c_int32_2p2, a_int32_0, b2 );
	}
	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );
		c_int32_2p0 = _mm512_sub_epi32( c_int32_2p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16 );

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
		c_int32_1p1 = _mm512_sub_epi32( c_int32_1p1 , b0 );
		c_int32_2p1 = _mm512_sub_epi32( c_int32_2p1 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 32 );

		c_int32_0p2 = _mm512_sub_epi32( c_int32_0p2 , b0 );
		c_int32_1p2 = _mm512_sub_epi32( c_int32_1p2 , b0 );
		c_int32_2p2 = _mm512_sub_epi32( c_int32_2p2 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,0,selector1,selector2);

			// c[1:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,1,selector1,selector2);

			// c[2:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,2,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,0,selector1,selector2);

			// c[1:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,1,selector1,selector2);

			// c[2:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,2,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_3x48:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		a_int32_0 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_3x48:
	{
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_3x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_3x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_3x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_3x48:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[0, 32-47]
		CLIP_S32_AVX512(c_int32_0p2, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[1, 16-31]
		CLIP_S32_AVX512(c_int32_1p1, min, max)

		// c[1, 32-47]
		CLIP_S32_AVX512(c_int32_1p2, min, max)

		// c[2, 0-15]
		CLIP_S32_AVX512(c_int32_2p0, min, max)

		// c[2, 16-31]
		CLIP_S32_AVX512(c_int32_2p1, min, max)

		// c[2, 32-47]
		CLIP_S32_AVX512(c_int32_2p2, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_3x48:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
			a_int32_0 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 2 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			a_int32_0 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point2 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
			zero_point2 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 2 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point2 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[0, 32-47]
		CVT_MULRND_CVT32(c_int32_0p2,a_int32_0,zero_point2);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2,zero_point1);

		// c[1, 32-47]
		CVT_MULRND_CVT32(c_int32_1p2,a_int32_0,zero_point2);

		// c[2, 0-15]
		CVT_MULRND_CVT32(c_int32_2p0,selector1,zero_point0);

		// c[2, 16-31]
		CVT_MULRND_CVT32(c_int32_2p1,selector2,zero_point1);

		// c[2, 32-47]
		CVT_MULRND_CVT32(c_int32_2p2,a_int32_0,zero_point2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_3x48:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);

			// c[1:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,1);

			// c[2:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,2);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);

			// c[1:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,1);

			// c[2:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,2);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_3x48:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 32-47]
		SWISH_S32_AVX512(c_int32_0p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 16-31]
		SWISH_S32_AVX512(c_int32_1p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 32-47]
		SWISH_S32_AVX512(c_int32_1p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 0-15]
		SWISH_S32_AVX512(c_int32_2p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 16-31]
		SWISH_S32_AVX512(c_int32_2p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[2, 32-47]
		SWISH_S32_AVX512(c_int32_2p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_3x48_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[0,32-47]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 2*16 ), c_int32_0p2 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 1*16 ), c_int32_1p1 );

		// c[1,32-47]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 2*16 ), c_int32_1p2 );

		// c[2,0-15]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 0*16 ), c_int32_2p0 );

		// c[2,16-31]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 1*16 ), c_int32_2p1 );

		// c[2,32-47]
		_mm512_storeu_si512( c + ( rs_c * 2 ) + ( 2*16 ), c_int32_2p2 );
	}
}

// 2x48 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_2x48)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_2x48_DISABLE,
						  &&POST_OPS_BIAS_2x48,
						  &&POST_OPS_RELU_2x48,
						  &&POST_OPS_RELU_SCALE_2x48,
						  &&POST_OPS_GELU_TANH_2x48,
						  &&POST_OPS_GELU_ERF_2x48,
						  &&POST_OPS_CLIP_2x48,
						  &&POST_OPS_DOWNSCALE_2x48,
						  &&POST_OPS_MATRIX_ADD_2x48,
						  &&POST_OPS_SWISH_2x48
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();
	__m512i b2 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();
	__m512i c_int32_0p1 = _mm512_setzero_epi32();
	__m512i c_int32_0p2 = _mm512_setzero_epi32();

	__m512i c_int32_1p0 = _mm512_setzero_epi32();
	__m512i c_int32_1p1 = _mm512_setzero_epi32();
	__m512i c_int32_1p2 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

		// Broadcast a[1,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 1 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
		c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );

		// Broadcast a[1,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 1 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[1,0-47] = a[1,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_1p0 = _mm512_dpbusd_epi32( c_int32_1p0, a_int32_0, b0 );
		c_int32_1p1 = _mm512_dpbusd_epi32( c_int32_1p1, a_int32_0, b1 );
		c_int32_1p2 = _mm512_dpbusd_epi32( c_int32_1p2, a_int32_0, b2 );
	}

	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );
		c_int32_1p0 = _mm512_sub_epi32( c_int32_1p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16 );

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );
		c_int32_1p1 = _mm512_sub_epi32( c_int32_1p1 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 32 );

		c_int32_0p2 = _mm512_sub_epi32( c_int32_0p2 , b0 );
		c_int32_1p2 = _mm512_sub_epi32( c_int32_1p2 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,0,selector1,selector2);

			// c[1:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,1,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,0,selector1,selector2);

			// c[1:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,1,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_2x48:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		a_int32_0 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_2x48:
	{
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_2x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_2x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_2x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_2x48:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[0, 32-47]
		CLIP_S32_AVX512(c_int32_0p2, min, max)

		// c[1, 0-15]
		CLIP_S32_AVX512(c_int32_1p0, min, max)

		// c[1, 16-31]
		CLIP_S32_AVX512(c_int32_1p1, min, max)

		// c[1, 32-47]
		CLIP_S32_AVX512(c_int32_1p2, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_2x48:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
			a_int32_0 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 2 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			a_int32_0 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point2 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
			zero_point2 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 2 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point2 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[0, 32-47]
		CVT_MULRND_CVT32(c_int32_0p2,a_int32_0,zero_point2);

		// c[1, 0-15]
		CVT_MULRND_CVT32(c_int32_1p0,selector1,zero_point0);

		// c[1, 16-31]
		CVT_MULRND_CVT32(c_int32_1p1,selector2,zero_point1);

		// c[1, 32-47]
		CVT_MULRND_CVT32(c_int32_1p2,a_int32_0,zero_point2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_2x48:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);

			// c[1:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,1);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);

			// c[1:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,1);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_2x48:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 32-47]
		SWISH_S32_AVX512(c_int32_0p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 0-15]
		SWISH_S32_AVX512(c_int32_1p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 16-31]
		SWISH_S32_AVX512(c_int32_1p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[1, 32-47]
		SWISH_S32_AVX512(c_int32_1p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_2x48_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[0,32-47]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 2*16 ), c_int32_0p2 );

		// c[1,0-15]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 0*16 ), c_int32_1p0 );

		// c[1,16-31]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 1*16 ), c_int32_1p1 );

		// c[1,32-47]
		_mm512_storeu_si512( c + ( rs_c * 1 ) + ( 2*16 ), c_int32_1p2 );
	}
}

// 1x48 int8o32 kernel
LPGEMM_MN_FRINGE_KERN(int8_t,int8_t,int32_t,s8s8s32os32_1x48)
{
	static void* post_ops_labels[] =
						{
						  &&POST_OPS_1x48_DISABLE,
						  &&POST_OPS_BIAS_1x48,
						  &&POST_OPS_RELU_1x48,
						  &&POST_OPS_RELU_SCALE_1x48,
						  &&POST_OPS_GELU_TANH_1x48,
						  &&POST_OPS_GELU_ERF_1x48,
						  &&POST_OPS_CLIP_1x48,
						  &&POST_OPS_DOWNSCALE_1x48,
						  &&POST_OPS_MATRIX_ADD_1x48,
						  &&POST_OPS_SWISH_1x48
						};
	dim_t k_full_pieces = k0 / 4;
	dim_t k_partial_pieces = k0 % 4;

	uint8_t cvt_uint8 = 128;
	__m512i vec_uint8 = _mm512_set1_epi8 (cvt_uint8);

	// B matrix storage.
	__m512i b0 = _mm512_setzero_epi32();
	__m512i b1 = _mm512_setzero_epi32();
	__m512i b2 = _mm512_setzero_epi32();

	// A matrix storage.
	__m512i a_int32_0 = _mm512_setzero_epi32();

	// Registers to use for accumulating C.
	__m512i c_int32_0p0 = _mm512_setzero_epi32();
	__m512i c_int32_0p1 = _mm512_setzero_epi32();
	__m512i c_int32_0p2 = _mm512_setzero_epi32();

	for ( dim_t kr = 0; kr < k_full_pieces; kr += 1 )
	{
		b0 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * kr ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_int32_0 = _mm512_set1_epi32( *( int32_t* )( a + ( rs_a * 0 ) + ( cs_a * kr ) ) );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );
	}
	// Handle k remainder.
	if ( k_partial_pieces > 0 )
	{
		__m128i a_kfringe_buf;
		__mmask16 load_mask = _cvtu32_mask16( 0xFFFF >> ( 16 - k_partial_pieces ) );

		b0 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 0 ) );
		b1 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 1 ) );
		b2 = _mm512_loadu_si512( b + ( rs_b * k_full_pieces ) + ( cs_b * 2 ) );

		// Broadcast a[0,kr:kr+4].
		a_kfringe_buf = _mm_maskz_loadu_epi8( load_mask, ( a + ( rs_a * 0 ) + ( cs_a * k_full_pieces ) ) );
		a_int32_0 = _mm512_broadcastd_epi32( a_kfringe_buf );

		//convert signed int8 to uint8 for VNNI
		a_int32_0 = _mm512_add_epi8( a_int32_0, vec_uint8 );

		// Perform column direction mat-mul with k = 4.
		// c[0,0-47] = a[0,kr:kr+4]*b[kr:kr+4,0-47]
		c_int32_0p0 = _mm512_dpbusd_epi32( c_int32_0p0, a_int32_0, b0 );
		c_int32_0p1 = _mm512_dpbusd_epi32( c_int32_0p1, a_int32_0, b1 );
		c_int32_0p2 = _mm512_dpbusd_epi32( c_int32_0p2, a_int32_0, b2 );
	}

	if ( post_ops_attr.is_last_k == 1 )
	{
		//Subtract B matrix sum column values to compensate
		//for addition of 128 to A matrix elements

		int32_t* bsumptr = post_ops_attr.b_col_sum_vec + post_ops_attr.b_sum_offset;

		b0 = _mm512_loadu_si512( bsumptr);

		c_int32_0p0 = _mm512_sub_epi32( c_int32_0p0 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 16);

		c_int32_0p1 = _mm512_sub_epi32( c_int32_0p1 , b0 );

		b0 = _mm512_loadu_si512( bsumptr + 32);

		c_int32_0p2 = _mm512_sub_epi32( c_int32_0p2 , b0 );
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
	}

	// Scale C by beta.
	if ( beta != 0 )
	{
		if ( ( post_ops_attr.buf_downscale != NULL ) &&
			 ( post_ops_attr.is_first_k == TRUE ) )
		{
			// c[0:0-15,16-31,32-47]
			S8_S32_BETA_OP3(0,0,selector1,selector2);
		}
		else
		{
			// c[0:0-15,16-31,32-47]
			S32_S32_BETA_OP3(0,0,selector1,selector2);
		}
	}

	// Post Ops
	lpgemm_post_op* post_ops_list_temp = post_ops_list;
	POST_OP_LABEL_LASTK_SAFE_JUMP
POST_OPS_BIAS_1x48:
	{
		selector1 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
		selector2 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
		a_int32_0 =
				_mm512_loadu_si512( ( int32_t* )post_ops_list_temp->op_args1 +
								post_ops_attr.post_op_c_j + ( 2 * 16 ) );

		// c[0,0-15]
		c_int32_0p0 = _mm512_add_epi32( selector1, c_int32_0p0 );

		// c[0, 16-31]
		c_int32_0p1 = _mm512_add_epi32( selector2, c_int32_0p1 );

		// c[0,32-47]
		c_int32_0p2 = _mm512_add_epi32( a_int32_0, c_int32_0p2 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_1x48:
	{
		selector1 = _mm512_setzero_epi32();

		// c[0,0-15]
		c_int32_0p0 = _mm512_max_epi32( selector1, c_int32_0p0 );

		// c[0, 16-31]
		c_int32_0p1 = _mm512_max_epi32( selector1, c_int32_0p1 );

		// c[0,32-47]
		c_int32_0p2 = _mm512_max_epi32( selector1, c_int32_0p2 );

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_RELU_SCALE_1x48:
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

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_TANH_1x48:
	{
		__m512 dn, z, x, r2, r, y, x_tanh;
		__m512i q;

		// c[0, 0-15]
		GELU_TANH_S32_AVX512(c_int32_0p0, y, r, r2, x, z, dn, x_tanh, q)

		// c[0, 16-31]
		GELU_TANH_S32_AVX512(c_int32_0p1, y, r, r2, x, z, dn, x_tanh, q)

		// c[0, 32-47]
		GELU_TANH_S32_AVX512(c_int32_0p2, y, r, r2, x, z, dn, x_tanh, q)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_GELU_ERF_1x48:
	{
		__m512 x, r, y, x_erf;

		// c[0, 0-15]
		GELU_ERF_S32_AVX512(c_int32_0p0, y, r, x, x_erf)

		// c[0, 16-31]
		GELU_ERF_S32_AVX512(c_int32_0p1, y, r, x, x_erf)

		// c[0, 32-47]
		GELU_ERF_S32_AVX512(c_int32_0p2, y, r, x, x_erf)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_CLIP_1x48:
	{
		__m512i min = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args2 );
		__m512i max = _mm512_set1_epi32( *( int32_t* )post_ops_list_temp->op_args3 );

		// c[0, 0-15]
		CLIP_S32_AVX512(c_int32_0p0, min, max)

		// c[0, 16-31]
		CLIP_S32_AVX512(c_int32_0p1, min, max)

		// c[0, 32-47]
		CLIP_S32_AVX512(c_int32_0p2, min, max)

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_DOWNSCALE_1x48:
	{
		if ( post_ops_list_temp->scale_factor_len > 1 )
		{
			selector1 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 0 * 16 ) );
			selector2 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 1 * 16 ) );
			a_int32_0 =
				_mm512_loadu_si512( ( float* )post_ops_list_temp->scale_factor +
								post_ops_attr.post_op_c_j + ( 2 * 16 ) );
		}
		else if ( post_ops_list_temp->scale_factor_len == 1 )
		{
			selector1 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			selector2 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
			a_int32_0 =
				( __m512i )_mm512_set1_ps( *( ( float* )post_ops_list_temp->scale_factor ) );
		}

		// Need to ensure sse not used to avoid avx512 -> sse transition.
		__m128i zero_point0 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point1 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		__m128i zero_point2 = _mm512_castsi512_si128( _mm512_setzero_si512() );
		if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) > 1 )
		{
			zero_point0 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 0 * 16 ) ) );
			zero_point1 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 1 * 16 ) ) );
			zero_point2 = _mm_loadu_si128( ( __m128i const* )
							( ( int8_t* )post_ops_list_temp->op_args1 +
							post_ops_attr.post_op_c_j + ( 2 * 16 ) ) );
		}
		else if ( *( ( dim_t* )post_ops_list_temp->op_args3 ) == 1 )
		{
			zero_point0 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point1 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
			zero_point2 = _mm_maskz_set1_epi8( 0xFFFF,
							*( ( int8_t* )post_ops_list_temp->op_args1 ) );
		}

		// c[0, 0-15]
		CVT_MULRND_CVT32(c_int32_0p0,selector1,zero_point0);

		// c[0, 16-31]
		CVT_MULRND_CVT32(c_int32_0p1,selector2,zero_point1);

		// c[0, 32-47]
		CVT_MULRND_CVT32(c_int32_0p2,a_int32_0,zero_point2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_MATRIX_ADD_1x48:
	{
		dim_t ldm = *( dim_t* )post_ops_list_temp->op_args3;
		if ( post_ops_attr.c_stor_type == S8 )
		{
			int8_t* matptr = ( int8_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S8_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);
		}
		else
		{
			int32_t* matptr = ( int32_t* )post_ops_list_temp->op_args1;

			// c[0:0-15,16-31,32-47]
			S32_S32_MATRIX_ADD_3COL(selector1,selector2,a_int32_0,0);
		}

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_SWISH_1x48:
	{
		selector1 =
			_mm512_set1_epi32( *( ( int32_t* )post_ops_list_temp->op_args2 ) );
		__m512 al = _mm512_cvtepi32_ps( selector1 );

		__m512 fl_reg, al_in, r, r2, z, dn;

		// c[0, 0-15]
		SWISH_S32_AVX512(c_int32_0p0, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 16-31]
		SWISH_S32_AVX512(c_int32_0p1, fl_reg, al, al_in, r, r2, z, dn, selector2);

		// c[0, 32-47]
		SWISH_S32_AVX512(c_int32_0p2, fl_reg, al, al_in, r, r2, z, dn, selector2);

		POST_OP_LABEL_LASTK_SAFE_JUMP_WITH_NEXT_PTR
	}
POST_OPS_1x48_DISABLE:
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
	}
	else
	{
		// Store the results.
		// c[0,0-15]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 0*16 ), c_int32_0p0 );

		// c[0, 16-31]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 1*16 ), c_int32_0p1 );

		// c[0,32-47]
		_mm512_storeu_si512( c + ( rs_c * 0 ) + ( 2*16 ), c_int32_0p2 );
	}
}
#endif
