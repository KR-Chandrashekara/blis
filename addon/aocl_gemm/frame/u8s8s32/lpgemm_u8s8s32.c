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
#include "lpgemm_u8s8s32.h"
#include "lpgemm_packa.h"
#include "lpgemm_packb.h"
#include "lpgemm_6x64rowmajor.h"
#include "lpgemm_utils.h"
#include "lpgemm_thrinfo_utils.h"
#include "lpgemm_config.h"

// B should always be packed.
void lpgemm_rowvar_u8s8s32o32
     (
       const dim_t           m,
       const dim_t           n,
       const dim_t           k,
       const uint8_t*        a,
       const dim_t           rs_a,
       const dim_t           cs_a,
       const AOCL_MEMORY_TAG mtag_a,
       const int8_t*         b,
       const dim_t           rs_b,
       const dim_t           cs_b,
       const AOCL_MEMORY_TAG mtag_b,
       int32_t*              c,
       const dim_t           rs_c,
       int32_t               alpha,
       int32_t               beta,
       rntm_t*               rntm,
       lpgemm_thrinfo_t*     thread
     )
{
	dim_t NC = lpgemm_get_block_size_NC_global_cntx( U8S8S32OS32 );
	dim_t KC = lpgemm_get_block_size_KC_global_cntx( U8S8S32OS32 );
	dim_t MC = lpgemm_get_block_size_MC_global_cntx( U8S8S32OS32 );
	dim_t NR = lpgemm_get_block_size_NR_global_cntx( U8S8S32OS32 );
	dim_t MR = lpgemm_get_block_size_MR_global_cntx( U8S8S32OS32 );

	if ( mtag_b == UNPACKED )
	{
		//Error: can only work with packed B now.
		return;
	}

	// Strides are updated based on matrix packing/reordering.
	const uint8_t* a_use = NULL;
	dim_t rs_a_use = rs_a;
	dim_t cs_a_use = cs_a;
	dim_t a_block_stride = 0;

	const int8_t* b_use = NULL;
	dim_t rs_b_use = rs_b;
	dim_t cs_b_use = cs_b;

	int32_t* c_use_jc = NULL;
	int32_t* c_use_ic = NULL;

	// Pack buffer for A.
	uint8_t* pack_a_buffer_u8s8s32o32;
	mem_t mem_a = BLIS_MEM_INITIALIZER;
	siz_t mem_a_size_req = 0;

	// Pack buffer for B.
	int8_t* pack_b_buffer_u8s8s32o32;
	mem_t mem_b = BLIS_MEM_INITIALIZER;
	siz_t mem_b_size_req = 0;
	dim_t packb_min_NR = get_packb_u8s8s32o32_min_NR();

	// kc needs to be a multiple of 4 so that it can be used with vpdpbusd
	// instruction. Padding is added in cases this condition is not
	// satisfied, and therefore the k offset used for packed/reordered
	// buffer needs to be updated.
	dim_t k_updated = make_multiple_of_n( k, 4 );

	// Generate thrinfo objects for jc and ic loops from lpgemm_thrinfo_t.
	thrinfo_t thread_jc;
	thrinfo_t thread_ic;

	lpgemm_gen_thrinfo( thread, &thread_jc, &thread_ic );

	// Compute the JC loop thread range for the current thread.
	dim_t jc_start, jc_end;
	bli_thread_range_sub( &thread_jc, n, NR, FALSE, &jc_start, &jc_end );

	for ( dim_t jc = jc_start; jc < jc_end; jc += NC )
	{
		dim_t nc0 = bli_min( ( jc_end - jc ), NC );
		c_use_jc = c + jc;

		dim_t jc_cur_loop = jc;
		dim_t jc_cur_loop_rem = 0;
		dim_t n_sub_updated;

		if ( mtag_b == REORDERED )
		{
			get_B_panel_reordered_start_offset_width
			(
			  jc, n, NC, packb_min_NR,
			  &jc_cur_loop, &jc_cur_loop_rem,
			  &nc0, &n_sub_updated
			);
		}

		for ( dim_t pc = 0; pc < k; pc += KC )
		{
			int32_t beta0 = ( pc == 0 ) ? beta : 1;
			dim_t kc0 = bli_min( ( k - pc ), KC );

			// kc0 needs to be a multiple of 4 so that it can be
			// used with vpdpbusd instruction. Padding is added in
			// cases this condition is not satisfied, and therefore
			// the kc0 offsets used for packed/reordered buffers
			// needs to be updated.
			dim_t kc0_updated = make_multiple_of_n( kc0, 4 );

			if ( mtag_b == PACK )
			{
				// Pack B chunks are based on jc work id.
				dim_t jc_work_id = bli_thread_work_id( &thread_jc );

				// Using child thrinfo (thread_ic) tid to decide chief thread
				// per B matrix chunk (jc work id group)
				if ( bli_thread_am_ochief( &thread_ic ) )
				{
					// nc0 needs to be a multiple of 16 since this gives maximum
					// vectorization. Packing B always results in buffers with width
					// which is a multiple of 16. Subsequently the nc0 offsets used
					// for packed/reordered buffers needs to be updated.
					dim_t nc0_updated = make_multiple_of_n( nc0, packb_min_NR );
					mem_b_size_req = sizeof( int8_t ) * nc0_updated * kc0_updated;

					lpgemm_alloc_mem_panel
					(
					  mem_b_size_req, BLIS_BUFFER_FOR_B_PANEL,
					  &mem_b, rntm
					);

					thread->comm[jc_work_id].sent_object =
													bli_mem_buffer( &mem_b );
				}

				// All threads in work group should wait till chief thread has
				// finished allocating the packing buffers.
				bli_thrcomm_barrier
				(
				  bli_thread_ocomm_id( &thread_ic ),
				  &thread->comm[jc_work_id]
				);

				pack_b_buffer_u8s8s32o32 =
						( int8_t* ) thread->comm[jc_work_id].sent_object;

				// Compute the B panel per thread loop range for parallel
				// packing using ic_ways number of threads. Since atmost only
				// ic_ways threads can be used, the thread_ic attributes are
				// used to split the loop range.
				dim_t jc_packb_start, jc_packb_end;
				bli_thread_range_sub
				(
				  &thread_ic, nc0, NR, FALSE,
				  &jc_packb_start, &jc_packb_end
				);

				// Ensure thread ranges are valid, especially cases where no:
				// of threads available for parallelization are greater than
				// no: of B panel NR chunks.
				if ( ( jc_packb_end > jc_packb_start ) &&
					 ( jc_packb_start < ( jc + nc0 ) ) )
				{
					packb_nr64_u8s8s32o32
					(
					  pack_b_buffer_u8s8s32o32 + ( jc_packb_start * kc0_updated ),
					  ( b + ( rs_b * pc ) + ( cs_b * jc ) +
					    ( cs_b * jc_packb_start ) ), rs_b,
					  ( jc_packb_end - jc_packb_start ), kc0,
					  &rs_b_use, &cs_b_use
					);
				}
				else
				{
					get_packb_nr64_u8s8s32o32_strides( &rs_b_use, &cs_b_use );
				}

				// All threads in work group should wait till B matrix packing
				// is completed by the participating threads.
				bli_thrcomm_barrier
				(
				  bli_thread_ocomm_id( &thread_ic ),
				  &thread->comm[jc_work_id]
				);
				b_use = pack_b_buffer_u8s8s32o32;
			}
			else if ( mtag_b == REORDERED )
			{
				// In multi-threaded scenarios, an extra offset into a given
				// packed B panel is required, since the jc loop split can
				// result in per thread start offset inside the panel, instead
				// of panel boundaries.
				b_use = b + ( jc_cur_loop * k_updated ) +
						( n_sub_updated * pc ) +
						( jc_cur_loop_rem * kc0_updated );

				get_packb_nr64_u8s8s32o32_strides( &rs_b_use, &cs_b_use );
			}
			else
			{
				//Unpacked B not supported.
				return;
			}

			dim_t ic_start, ic_end;
			bli_thread_range_sub( &thread_ic, m, MR, FALSE, &ic_start, &ic_end );

			for ( dim_t ic = ic_start; ic < ic_end; ic += MC )
			{
				dim_t mc0 = bli_min( ( ic_end - ic ), MC );
				c_use_ic = c_use_jc + ( rs_c * ic );

				// Matrix A packed and reordered code path is not triggerred
				// currently since we do not support it yet.
				if ( mtag_a == PACK )
				{
					mem_a_size_req = sizeof( uint8_t ) * mc0 * kc0_updated;

					lpgemm_alloc_mem_panel
					(
					  mem_a_size_req, BLIS_BUFFER_FOR_A_BLOCK,
					  &mem_a, rntm
					);
					pack_a_buffer_u8s8s32o32 = ( uint8_t* )bli_mem_buffer( &mem_a );

					packa_k64_u8s8s32o32
					(
					  pack_a_buffer_u8s8s32o32,
					  ( a + ( rs_a * ic ) + pc ), rs_a,
					  mc0, kc0,
					  &rs_a_use, &cs_a_use
					);
					a_use = pack_a_buffer_u8s8s32o32;
					a_block_stride = kc0_updated;
				}
				else if ( mtag_a == REORDERED )
				{
					get_packa_k64_u8s8s32o32_strides( &rs_a_use, &cs_a_use );
					a_use = a + ( pc * m ) + ( kc0_updated * ic );
					a_block_stride = kc0_updated;
				}
				else
				{
					a_use = a + ( rs_a * ic ) + ( cs_a * pc );

					// Int8 kernel reads 4 elements, totalling 4 bytes in a
					// single broadcast for use in vnni instruction.
					// Non vnni based kernel requires update to this code.
					cs_a_use = 4;
					a_block_stride = rs_a;
				}

				for ( dim_t jr = 0; jr < nc0; jr += NR )
				{
					dim_t nr0 = bli_min( ( nc0 - jr ), NR );

					// Reorder/Packed B, Reorder/Packed/Unpacked A call.
					lpgemm_rowvar_u8s8s32o32_6x64
					(
					  mc0, nr0, kc0,
					  a_use, rs_a_use, cs_a_use, a_block_stride,
					  ( b_use + ( jr * kc0_updated ) ), rs_b_use, cs_b_use,
					  ( c_use_ic + jr ), rs_c, 1,
					  alpha, beta0
					);
				}
			}
		}
		if ( mtag_b == REORDERED )
		{
			adjust_B_panel_reordered_jc( &jc, jc_cur_loop );
		}
	}

	// Release pack buffers.
	if ( mtag_b == PACK )
	{
		// All threads in work group should wait till B matrix usage is
		// completed by the participating threads.
		bli_thrcomm_barrier
		(
		  bli_thread_ocomm_id( &thread_jc ),
		  &thread->comm[bli_thread_work_id( &thread_jc)]
		);

		if ( bli_thread_am_ochief( &thread_ic ) )
		{
			if ( bli_mem_is_alloc( &mem_b ) )
			{
				bli_membrk_release( rntm, &mem_b );
			}
		}
	}
	if ( mtag_a == PACK )
	{
		if ( bli_mem_is_alloc( &mem_a ) )
		{
			bli_membrk_release( rntm, &mem_a );
		}
	}
}