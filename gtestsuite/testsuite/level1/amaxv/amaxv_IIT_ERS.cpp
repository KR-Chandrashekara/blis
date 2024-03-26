/*
   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.

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

#include <gtest/gtest.h>
#include "test_amaxv.h"
#include "level1/ref_amaxv.h"
#include "common/wrong_inputs_helpers.h"
#include "common/testing_helpers.h"
#include "inc/check_error.h"

template <typename T>
class amaxv_IIT_ERS_Test : public ::testing::Test {};
typedef ::testing::Types<float, double, scomplex, dcomplex> TypeParam;
TYPED_TEST_SUITE(amaxv_IIT_ERS_Test, TypeParam);

using namespace testinghelpers::IIT;

#if defined(TEST_BLAS) || defined(TEST_CBLAS)
/*

   Early Return Scenarios(ERS) for BLAS/CBLAS compliance :

    The AMAX API is expected to return early in the following cases:
    1. When n < 1.
    2. When incx <= 0.

    The index returned in these cases is expected to be 0.

    Further, the API is expected to return early when:
    3. When n == 1.

    The index returned in this case is expected to be 1(BLAS)
    or 0(CBLAS).
*/

// n < 1, with non-unit stride
TYPED_TEST(amaxv_IIT_ERS_Test, n_lt_one_nonUnitStride)
{
 using T = TypeParam;
 gtint_t n = 0;
 gtint_t inc = 5;

  // Initialize vectors with random numbers.
  std::vector<T> x = testinghelpers::get_random_vector<T>( -10, 10, N, inc );

// Invoking AMAXV with an value of n.
#ifdef TEST_BLAS
  gtint_t idx = amaxv_<T>( n, x.data(), inc );
#else
  gtint_t idx = cblas_amaxv<T>( n, x.data(), inc );
#endif
 
  // Computing the difference.
  EXPECT_EQ( idx, gtint_t(0) );
}

// inc == 0, with non-unit stride
TYPED_TEST(amaxv_IIT_ERS_Test, incx_eq_zero)
{
    using T = TypeParam;
    gtint_t inc = 0;

    // Initialize vectors with random numbers.
    std::vector<T> x = testinghelpers::get_random_vector<T>( -10, 10, N, 1 );

// Invoking AMAXV with an invalid value of n.
#ifdef TEST_BLAS
  gtint_t idx = amaxv_<T>( N, x.data(), inc );
#else
  gtint_t idx = cblas_amaxv<T>( N, x.data(), inc );
#endif

 // Computing the difference.
 EXPECT_EQ( idx, gtint_t(0) );
}

// n < 1, with unit stride
TYPED_TEST(amaxv_IIT_ERS_Test, n_lt_one_unitStride)
{
 using T = TypeParam;
 gtint_t n = 0;
 gtint_t unit_inc = 1;

 // Initialize vectors with random numbers.
 std::vector<T> x = testinghelpers::get_random_vector<T>( -10, 10, N, unit_inc );

// Invoking AMAXV with an value of n.
#ifdef TEST_BLAS
  gtint_t idx = amaxv_<T>( n, x.data(), unit_inc );
#else
  gtint_t idx = cblas_amaxv<T>( n, x.data(), unit_inc );
#endif

 // Computing the difference.
 EXPECT_EQ( idx, gtint_t(0) );
}

// n == 1, with unit stride
TYPED_TEST(amaxv_IIT_ERS_Test, n_eq_one_unitStride)
{
 using T = TypeParam;
 gtint_t n = 1;
 gtint_t unit_inc = 1;

 // Initialize vectors with random numbers.
 std::vector<T> x = testinghelpers::get_random_vector<T>( -10, 10, N, unit_inc );

// Invoking AMAXV with an value of n.
#ifdef TEST_BLAS
  gtint_t idx = amaxv_<T>( n, x.data(), unit_inc );
  EXPECT_EQ( idx, gtint_t(1) );
#else
  gtint_t idx = cblas_amaxv<T>( n, x.data(), unit_inc );
  EXPECT_EQ( idx, gtint_t(0) );
#endif

}

TYPED_TEST(amaxv_IIT_ERS_Test, n_eq_one_nonUnitStrides)
{
  using T = TypeParam;
  gtint_t n = 1;
  gtint_t inc = 5;
  // Initialize vectors with random numbers.
  std::vector<T> x = testinghelpers::get_random_vector<T>( -10, 10, N, inc );
  
#ifdef TEST_BLAS
  gtint_t idx = amaxv_<T>( n, x.data(), inc );
  EXPECT_EQ( idx, gtint_t(1) );
#else
  gtint_t idx = cblas_amaxv<T>( n, x.data(), inc );
  EXPECT_EQ( idx, gtint_t(0) );
#endif
}

#endif