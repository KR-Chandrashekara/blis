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

#ifndef BLIS_GEMM_INT8_MNFRINGE
#define BLIS_GEMM_INT8_MNFRINGE

// 5xlt16 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_5xlt16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta,
       const dim_t    n0_rem
     );

// 4xlt16 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_4xlt16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta,
       const dim_t    n0_rem
     );

// 3xlt16 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_3xlt16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta,
       const dim_t    n0_rem
     );

// 2xlt16 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_2xlt16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta,
       const dim_t    n0_rem
     );

// 1xlt16 int8o32 fringe kernel
void lpgemm_rowvar_u8s8s32o32_1xlt16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta,
       const dim_t    n0_rem
     );

// 5x16 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_5x16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 4x16 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_4x16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 3x16 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_3x16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 2x16 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_2x16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 1x16 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_1x16
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 5x32 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_5x32
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 4x32 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_4x32
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 3x32 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_3x32
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 2x32 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_2x32
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 1x32 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_1x32
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 5x48 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_5x48
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 4x48 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_4x48
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 3x48 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_3x48
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 2x48 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_2x48
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

// 1x48 int8o32 kernel
void lpgemm_rowvar_u8s8s32o32_1x48
     (
       const dim_t    k0,
       const uint8_t* a,
       const dim_t    rs_a,
       const dim_t    cs_a,
       const int8_t*  b,
       const dim_t    rs_b,
       const dim_t    cs_b,
       int32_t*       c,
       const dim_t    rs_c,
       const int32_t  alpha,
       const int32_t  beta
     );

#endif //BLIS_GEMM_INT8_MNFRINGE