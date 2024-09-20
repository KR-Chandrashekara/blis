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

#include <gtest/gtest.h>
#include "level1/scalv/test_scalv.h"

class cscalvGeneric :
        public ::testing::TestWithParam<std::tuple<char,            // conj_alpha
                                                   gtint_t,         // n
                                                   gtint_t,         // incx
                                                   scomplex>> {};   // alpha


// Tests using random integers as vector elements.
TEST_P( cscalvGeneric, API )
{
    using T = scomplex;
    //----------------------------------------------------------
    // Initialize values from the parameters passed through
    // test suite instantiation (INSTANTIATE_TEST_SUITE_P).
    //----------------------------------------------------------
    // denotes whether alpha or conj(alpha) will be used:
    char conj_alpha = std::get<0>(GetParam());
    // vector length:
    gtint_t n = std::get<1>(GetParam());
    // stride size for x:
    gtint_t incx = std::get<2>(GetParam());
    // alpha
    T alpha = std::get<3>(GetParam());

    // Set the threshold for the errors:
    // Check gtestsuite scalv.h or netlib source code for reminder of the
    // functionality from which we estimate operation count per element
    // of output, and hence the multipler for epsilon.
    // No adjustment applied yet for complex data.
    double thresh;
    if (n == 0)
        thresh = 0.0;
    else if (alpha == testinghelpers::ZERO<T>() || alpha == testinghelpers::ONE<T>())
        thresh = 0.0;
    else
        thresh = testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call generic test body using those parameters
    //----------------------------------------------------------
    test_scalv<T>( conj_alpha, n, incx, alpha, thresh );
}

// Black box testing for generic use of dscal.
INSTANTIATE_TEST_SUITE_P(
        unitPositiveIncrementSmall,
        cscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Range(gtint_t(1), gtint_t(101), 1),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(1)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                scomplex{-5.1, -7.3},
                                scomplex{ 1.0,  1.0},
                                scomplex{ 7.3,  5.1}
            )
        ),
        (::scalvGenericPrint<scomplex, scomplex>())
    );

// Black box testing for generic use of dscal.
INSTANTIATE_TEST_SUITE_P(
        unitPositiveIncrementLarge,
        cscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Values(gtint_t(111), gtint_t(193), gtint_t(403)),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(1)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                scomplex{-5.1, -7.3},
                                scomplex{ 1.0,  1.0},
                                scomplex{ 7.3,  5.1}
            )
        ),
        (::scalvGenericPrint<scomplex, scomplex>())
    );

INSTANTIATE_TEST_SUITE_P(
        nonUnitPositiveIncrementSmall,
        cscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Range(gtint_t(1), gtint_t(9), 1),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(3)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                scomplex{-5.1, -7.3},
                                scomplex{ 1.0,  1.0},
                                scomplex{ 7.3,  5.1}
            )
        ),
        (::scalvGenericPrint<scomplex, scomplex>())
    );

INSTANTIATE_TEST_SUITE_P(
        nonUnitPositiveIncrementLarge,
        cscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Values(gtint_t(111), gtint_t(193), gtint_t(403)),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(5)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                scomplex{-5.1, -7.3},
                                scomplex{ 1.0,  1.0},
                                scomplex{ 7.3,  5.1}
            )
        ),
        (::scalvGenericPrint<scomplex, scomplex>())
    );

#ifndef TEST_BLIS_TYPED
// alpha=0 testing only for BLAS and CBLAS as
// BLIS uses setv and won't propagate Inf and NaNs
INSTANTIATE_TEST_SUITE_P(
        alphaZero,
        cscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Values(gtint_t(1), gtint_t(3), gtint_t(17), gtint_t(64), gtint_t(73)),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(1),
                                gtint_t(2)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                scomplex{ 0.0,  0.0}
            )
        ),
        (::scalvGenericPrint<scomplex, scomplex>())
    );
#endif

#ifdef TEST_BLIS_TYPED
// Test when conjugate of x is used as an argument. This option is BLIS-api specific.
// Only test very few cases as sanity check since conj(x) = x for real types.
// We can modify the values using implementantion details.
INSTANTIATE_TEST_SUITE_P(
        conjalpha,
        cscalvGeneric,
        ::testing::Combine(
            ::testing::Values('c'),                                          // c: use conjugate
            ::testing::Values(gtint_t(3), gtint_t(30), gtint_t(112)),        // m size of vector
            ::testing::Values(gtint_t(1)),                                   // stride size for x
            ::testing::Values(scomplex{ 7.3,  5.1})                          // alpha
        ),
        (::scalvGenericPrint<scomplex, scomplex>())
    );
#endif

#ifndef TEST_BLIS_TYPED
// Test for negative increments.
// Only test very few cases as sanity check.
// We can modify the values using implementantion details.
INSTANTIATE_TEST_SUITE_P(
        NegativeIncrements,
        cscalvGeneric,
        ::testing::Combine(
            ::testing::Values('n'),                                          // n: use x, c: use conj(x)
            ::testing::Values(gtint_t(1), gtint_t(3), gtint_t(17)),          // m size of vector 
            ::testing::Values(gtint_t(-2), gtint_t(-1)),                     // stride size for x
            ::testing::Values(scomplex{ 7.3,  5.1})                          // alpha
        ),
        (::scalvGenericPrint<scomplex, scomplex>())
    );
#endif
