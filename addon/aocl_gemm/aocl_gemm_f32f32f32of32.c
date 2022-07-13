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

#include "blis.h"
#include "aocl_gemm_f32f32f32of32.h"
#include "lpgemm_types.h"
#include "lpgemm_thread_decor_openmp.h"
#include "lpgemm_utils.h"
#include "lpgemm_f32f32f32.h"

void aocl_gemm_f32f32f32of32
     (
       const char   transa,
       const char   transb,
       const dim_t  m,
       const dim_t  n,
       const dim_t  k,
       const float  alpha,
       const float* a,
       const dim_t  lda,
       const char   mem_format_a,
       const float* b,
       const dim_t  ldb,
       const char   mem_format_b,
       const float  beta,
       float*       c,
       const dim_t  ldc
     )
{
	trans_t blis_transa;
	trans_t blis_transb;

	// Check if avx ISA is supported, lpgemm fp32 matmul only works with it.
	if ( bli_cpuid_is_avx_supported() == FALSE )
	{
		printf(" AVX2 ISA not supported by processor, cannot perform lpgemm.\n");
		return; // Error.
	}

	/* Initialize BLIS. */
	bli_init_auto();

	AOCL_DTL_TRACE_ENTRY(AOCL_DTL_LEVEL_TRACE_1);
	AOCL_DTL_LOG_GEMM_INPUTS(AOCL_DTL_LEVEL_TRACE_1, *MKSTR(s), transa, transb, m, n, k,\
	      (void*)&alpha, lda, ldb, (void*)&beta, ldc);

	// Null check for pointers.
	if ( ( a == NULL ) || ( b == NULL ) || ( c == NULL ) )
	{
		AOCL_DTL_TRACE_EXIT_ERR(AOCL_DTL_LEVEL_TRACE_1, \
						"Invalid pointers provided for input parameters.");
		return; // Error.
	}

	/* Map BLAS chars to their corresponding BLIS enumerated type value. */
	bli_param_map_netlib_to_blis_trans( transa, &blis_transa );
	bli_param_map_netlib_to_blis_trans( transb, &blis_transb );

	/* Perform BLAS parameter checking. */
	// Transpose not supported.
	if ( ( blis_transa != BLIS_NO_TRANSPOSE ) ||
	     ( blis_transb != BLIS_NO_TRANSPOSE ) )
	{
		AOCL_DTL_TRACE_EXIT_ERR(AOCL_DTL_LEVEL_TRACE_1, \
						"Input matrix transpose not supported.");
		return; // Error.
	}

	// Row major input expected with leading dimensions equal to row stride.
	if ( ( lda != k ) || ( ldb != n ) || ( ldc != n ) )
	{
		AOCL_DTL_TRACE_EXIT_ERR(AOCL_DTL_LEVEL_TRACE_1, \
						"Column major and general stride not supported.");
		return; // Error.
	}

	// Check if dimensions are valid.
	if ( ( m <= 0) || ( n <= 0 ) || ( k <= 0 ) ||
	     ( lda <= 0 ) || ( ldb <= 0 ) || ( ldc <= 0 ) )
	{
		AOCL_DTL_TRACE_EXIT_ERR(AOCL_DTL_LEVEL_TRACE_1, \
						"Invalid matrix dimensions.");
		return; // Error.
	}

	const inc_t rs_a = lda;
	const inc_t cs_a = 1;
	const inc_t rs_b = ldb;
	const inc_t cs_b = 1;
	const inc_t rs_c = ldc;

	AOCL_MEMORY_TAG mtag_a;
	AOCL_MEMORY_TAG mtag_b;

	bli_param_map_char_to_lpmtag( mem_format_a, &mtag_a );
	bli_param_map_char_to_lpmtag( mem_format_b, &mtag_b );

	// Only unreordered A supported now.
	if ( mtag_a != UNPACKED )
	{
		AOCL_DTL_TRACE_EXIT_ERR(AOCL_DTL_LEVEL_TRACE_1, \
						"A matrix packing/reordering not supported.");
		return; // Error.
	}

	// Initialize a local runtime with global settings if necessary. Note
	// that in the case that a runtime is passed in, we make a local copy.
	rntm_t rntm_g;
	bli_rntm_init_from_global( &rntm_g );
	bli_membrk_rntm_set_membrk( &rntm_g );

#ifdef BLIS_ENABLE_OPENMP
	lpgemm_f32f32f32of32_openmp_thread_decorator
	(
	  m, n, k,
	  a, rs_a, cs_a, mtag_a,
	  b, rs_b, cs_b, mtag_b,
	  c, rs_c,
	  alpha, beta,
	  &rntm_g
	);
#else
	// Setting pack A by default for non open mp case.
	bli_rntm_set_pack_a( 1, &rntm_g );

	lpgemm_rowvar_f32f32f32of32
	(
	  m, n, k,
	  a, rs_a, cs_a, mtag_a,
	  b, rs_b, cs_b, mtag_b,
	  c, rs_c,
	  alpha, beta,
	  &rntm_g,
	  NULL
	);
#endif

	AOCL_DTL_TRACE_EXIT(AOCL_DTL_LEVEL_TRACE_1);
}