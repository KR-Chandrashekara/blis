/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2020 - 2023, Advanced Micro Devices, Inc. All rights reserved.

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


//
// Define BLAS-to-BLIS interfaces.
//

#ifdef BLIS_BLAS3_CALLS_TAPI

#undef  GENTFUNC
#define GENTFUNC( ftype, ch, blasname, blisname ) \
\
void PASTEF77S(ch,blasname) \
     ( \
       const f77_char* transa_array, \
       const f77_char* transb_array, \
       const f77_int*  m_array, \
       const f77_int*  n_array, \
       const f77_int*  k_array, \
       const ftype*    alpha_array, \
       const ftype**   a_array, const f77_int* lda_array, \
       const ftype**   b_array, const f77_int* ldb_array, \
       const ftype*    beta_array, \
             ftype**   c_array, const f77_int* ldc_array, \
       const f77_int*  group_count, \
       const f77_int*  group_size \
     ) \
{ \
    trans_t blis_transa; \
    trans_t blis_transb; \
    dim_t   m0, n0, k0; \
    inc_t   rs_a, cs_a; \
    inc_t   rs_b, cs_b; \
    inc_t   rs_c, cs_c; \
\
    /* Initialize BLIS. */ \
    bli_init_auto(); \
\
    /* Perform BLAS parameter checking. */ \
    f77_int count; \
    for(count = 0; count < *group_count; count++) \
    { \
        PASTEBLACHK(blisname) \
        ( \
          MKSTR(ch), \
          MKSTR(blisname), \
          transa_array+count, \
          transb_array+count, \
          m_array+count, \
          n_array+count, \
          k_array+count, \
          lda_array+count, \
          ldb_array+count, \
          ldc_array+count \
       ); \
    } \
\
    f77_int idx = 0, i, j; \
\
    for(i = 0; i < *group_count; i++) \
    { \
        /* Map BLAS chars to their corresponding BLIS enumerated type value. */ \
        bli_param_map_netlib_to_blis_trans( transa_array[i], &blis_transa ); \
        bli_param_map_netlib_to_blis_trans( transb_array[i], &blis_transb ); \
\
        /* Typecast BLAS integers to BLIS integers. */ \
        bli_convert_blas_dim1( m_array[i], m0 ); \
        bli_convert_blas_dim1( n_array[i], n0 ); \
        bli_convert_blas_dim1( k_array[i], k0 ); \
\
        /* Set the row and column strides of the matrix operands. */ \
        rs_a = 1; \
        cs_a = lda_array[i]; \
        rs_b = 1; \
        cs_b = ldb_array[i]; \
        rs_c = 1; \
        cs_c = ldc_array[i]; \
\
        for(j = 0; j < group_size[i]; j++) \
        { \
            /* Call BLIS interface. */ \
            PASTEMAC2(ch,blisname,BLIS_TAPI_EX_SUF) \
            ( \
              blis_transa, \
              blis_transb, \
              m0, \
              n0, \
              k0, \
              (ftype*)(alpha_array + i), \
              (ftype*)*(a_array + idx), rs_a, cs_a, \
              (ftype*)*(b_array + idx), rs_b, cs_b, \
              (ftype*)(beta_array + i), \
              (ftype*)*(c_array + idx), rs_c, cs_c, \
              NULL, \
              NULL  \
            ); \
\
            idx++; \
        } \
    } \
\
    bli_finalize_auto(); \
} \
IF_BLIS_ENABLE_BLAS(\
void PASTEF77(ch,blasname) \
     ( \
       const f77_char* transa_array, \
       const f77_char* transb_array, \
       const f77_int*  m_array, \
       const f77_int*  n_array, \
       const f77_int*  k_array, \
       const ftype*    alpha_array, \
       const ftype**   a_array, const f77_int* lda_array, \
       const ftype**   b_array, const f77_int* ldb_array, \
       const ftype*    beta_array, \
             ftype**   c_array, const f77_int* ldc_array, \
       const f77_int*  group_count, \
       const f77_int*  group_size \
     ) \
{ \
	PASTEF77S(ch,blasname)( transa_array, transb_array, m_array, n_array, k_array, \
				alpha_array, a_array, lda_array, b_array, ldb_array, beta_array, \
				c_array, ldc_array, group_count, group_size ); \
} \
)

#else

#undef  GENTFUNC
#define GENTFUNC( ftype, ch, blasname, blisname ) \
\
void PASTEF77S(ch,blasname) \
     ( \
       const f77_char* transa_array, \
       const f77_char* transb_array, \
       const f77_int*  m_array, \
       const f77_int*  n_array, \
       const f77_int*  k_array, \
       const ftype*    alpha_array, \
       const ftype**   a_array, const f77_int* lda_array, \
       const ftype**   b_array, const f77_int* ldb_array, \
       const ftype*    beta_array, \
             ftype**   c_array, const f77_int* ldc_array, \
       const f77_int* group_count, \
       const f77_int* group_size ) \
{ \
    trans_t blis_transa; \
    trans_t blis_transb; \
    dim_t   m0, n0, k0; \
\
    /* Initialize BLIS. */ \
    bli_init_auto(); \
\
    /* Perform BLAS parameter checking. */ \
    f77_int count; \
    for(count = 0; count < *group_count; count++) \
    { \
        PASTEBLACHK(blisname) \
        ( \
          MKSTR(ch), \
          MKSTR(blisname), \
          transa_array+count, \
          transb_array+count, \
          m_array+count, \
          n_array+count, \
          k_array+count, \
          lda_array+count, \
          ldb_array+count, \
          ldc_array+count \
       ); \
    } \
\
    const num_t dt     = PASTEMAC(ch,type); \
\
    f77_int idx = 0, i, j; \
\
    for(i = 0; i < *group_count; i++) \
    { \
        /* Map BLAS chars to their corresponding BLIS enumerated type value. */ \
        bli_param_map_netlib_to_blis_trans( transa_array[i], &blis_transa ); \
        bli_param_map_netlib_to_blis_trans( transb_array[i], &blis_transb ); \
\
        /* Typecast BLAS integers to BLIS integers. */ \
        bli_convert_blas_dim1( m_array[i], m0 ); \
        bli_convert_blas_dim1( n_array[i], n0 ); \
        bli_convert_blas_dim1( k_array[i], k0 ); \
\
        /* Set the row and column strides of the matrix operands. */ \
        const inc_t rs_a = 1; \
        const inc_t cs_a = lda_array[i]; \
        const inc_t rs_b = 1; \
        const inc_t cs_b = ldb_array[i]; \
        const inc_t rs_c = 1; \
        const inc_t cs_c = ldc_array[i]; \
\
        obj_t       alphao = BLIS_OBJECT_INITIALIZER_1X1; \
        obj_t       betao  = BLIS_OBJECT_INITIALIZER_1X1; \
\
        dim_t       m0_a, n0_a; \
        dim_t       m0_b, n0_b; \
\
        bli_set_dims_with_trans( blis_transa, m0, k0, &m0_a, &n0_a ); \
        bli_set_dims_with_trans( blis_transb, k0, n0, &m0_b, &n0_b ); \
\
        bli_obj_init_finish_1x1( dt, (ftype*)(alpha_array + i), &alphao ); \
        bli_obj_init_finish_1x1( dt, (ftype*)(beta_array  + i),  &betao ); \
\
        for( j = 0; j < group_size[i]; j++) \
        { \
            obj_t       ao     = BLIS_OBJECT_INITIALIZER; \
            obj_t       bo     = BLIS_OBJECT_INITIALIZER; \
            obj_t       co     = BLIS_OBJECT_INITIALIZER; \
\
            bli_obj_init_finish( dt, m0_a, n0_a, (ftype*)*(a_array + idx), rs_a, cs_a, &ao ); \
            bli_obj_init_finish( dt, m0_b, n0_b, (ftype*)*(b_array + idx), rs_b, cs_b, &bo ); \
            bli_obj_init_finish( dt, m0,   n0,   (ftype*)*(c_array + idx), rs_c, cs_c, &co ); \
            bli_obj_set_conjtrans( blis_transa, &ao ); \
            bli_obj_set_conjtrans( blis_transb, &bo ); \
\
            PASTEMAC(blisname,BLIS_OAPI_EX_SUF) \
            ( \
              &alphao, \
              &ao, \
              &bo, \
              &betao, \
              &co, \
              NULL, \
              NULL  \
            ); \
\
            idx++; \
        } \
    } \
\
    /* Finalize BLIS. */  \
    bli_finalize_auto(); \
} \
IF_BLIS_ENABLE_BLAS(\
void PASTEF77(ch,blasname) \
     ( \
       const f77_char* transa_array, \
       const f77_char* transb_array, \
       const f77_int*  m_array, \
       const f77_int*  n_array, \
       const f77_int*  k_array, \
       const ftype*    alpha_array, \
       const ftype**   a_array, const f77_int* lda_array, \
       const ftype**   b_array, const f77_int* ldb_array, \
       const ftype*    beta_array, \
             ftype**   c_array, const f77_int* ldc_array, \
       const f77_int*  group_count, \
       const f77_int*  group_size \
     ) \
{ \
	PASTEF77S(ch,blasname)( transa_array, transb_array, m_array, n_array, k_array, \
				alpha_array, a_array, lda_array, b_array, ldb_array, beta_array, \
				c_array, ldc_array, group_count, group_size ); \
} \
)

#endif

INSERT_GENTFUNC_BLAS( gemm_batch, gemm )

