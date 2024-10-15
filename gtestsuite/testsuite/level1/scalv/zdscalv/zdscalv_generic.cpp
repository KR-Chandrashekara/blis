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
#include "level1/scalv/test_scalv.h"

class zdscalvGeneric :
        public ::testing::TestWithParam<std::tuple<char,        // conj_alpha
                                                   gtint_t,     // n
                                                   gtint_t,     // incx
                                                   double>> {}; // alpha

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(zdscalvGeneric);

// Tests using random integers as vector elements.
TEST_P( zdscalvGeneric, API )
{
    using T = dcomplex;
    using U = double;
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
    U alpha = std::get<3>(GetParam());

    // Set the threshold for the errors:
    // Check gtestsuite scalv.h or netlib source code for reminder of the
    // functionality from which we estimate operation count per element
    // of output, and hence the multipler for epsilon.
    // No adjustment applied yet for complex data.
    double thresh;
    if (n == 0)
        thresh = 0.0;
    else if (alpha == testinghelpers::ZERO<U>() || alpha == testinghelpers::ONE<U>())
        thresh = 0.0;
    else
        thresh = testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call generic test body using those parameters
    //----------------------------------------------------------
    test_scalv<T, U>( conj_alpha, n, incx, alpha, thresh );
}

// bli_zdscal not present in BLIS
#ifndef TEST_BLIS_TYPED

// Black box testing for generic use of dscal.
INSTANTIATE_TEST_SUITE_P(
        unitPositiveIncrementSmall,
        zdscalvGeneric,
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
                                double( 7.0),
                                double(-3.0)
            )
        ),
        (::scalvGenericPrint<double, double>())
    );

// Black box testing for generic use of dscal.
INSTANTIATE_TEST_SUITE_P(
        unitPositiveIncrementLarge,
        zdscalvGeneric,
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
                                double( 7.0),
                                double(-3.0)
            )
        ),
        (::scalvGenericPrint<double, double>())
    );

INSTANTIATE_TEST_SUITE_P(
        nonUnitPositiveIncrementSmall,
        zdscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Range(gtint_t(1), gtint_t(9), 1),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(2)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                double( 7.0),
                                double(-3.0)
            )
        ),
        (::scalvGenericPrint<double, double>())
    );

INSTANTIATE_TEST_SUITE_P(
        nonUnitPositiveIncrementLarge,
        zdscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Values(gtint_t(111), gtint_t(193), gtint_t(403)),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(3)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                double( 7.0),
                                double(-3.0)
            )
        ),
        (::scalvGenericPrint<double, double>())
    );

// alpha=0 testing only for BLAS and CBLAS as
// BLIS uses setv and won't propagate Inf and NaNs
INSTANTIATE_TEST_SUITE_P(
        alphaZero,
        zdscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Values(gtint_t(1), gtint_t(3), gtint_t(17)),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(1),
                                gtint_t(3)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                double( 0.0)
            )
        ),
        (::scalvGenericPrint<double, double>())
    );

// Test for negative increments.
// Only test very few cases as sanity check.
// We can modify the values using implementantion details.
INSTANTIATE_TEST_SUITE_P(
        NegativeIncrements,
        zdscalvGeneric,
        ::testing::Combine(
            ::testing::Values('n'),                                          // n: use x, c: use conj(x)
            ::testing::Range(gtint_t(10), gtint_t(31), 10),                  // m size of vector
            ::testing::Values(gtint_t(-2), gtint_t(-1)),                     // stride size for x
            ::testing::Values(3)                                             // alpha
        ),
        (::scalvGenericPrint<double, double>())
    );

#if defined(BLIS_ENABLE_OPENMP) && defined(AOCL_DYNAMIC)
INSTANTIATE_TEST_SUITE_P(
        AOCLDynamic,
        zdscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Values(
                               gtint_t(   10000),     // nt_ideal = 1
                               gtint_t(   20000),     // nt_ideal = 4
                               gtint_t(  486919),     // nt_ideal = 8
                               gtint_t( 1000000),     // nt_ideal = 8
                               gtint_t( 2500000),     // nt_ideal = 12
                               gtint_t( 5000000),     // nt_ideal = 32
                               gtint_t( 7000000)      // nt_ideal = max_available
            ),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(1),
                                gtint_t(3)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                double( 7.0)
            )
        ),
        (::scalvGenericPrint<double, double>())
    );

INSTANTIATE_TEST_SUITE_P(
        AOCLDynamicAlphaZero,
        zdscalvGeneric,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'),
            // m: size of vector.
            ::testing::Values(
                               gtint_t(  486919),     // nt_ideal = 8
                               gtint_t( 7000000)      // nt_ideal = max_available
            ),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(1),
                                gtint_t(3)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                double( 0.0)
            )
        ),
        (::scalvGenericPrint<double, double>())
    );
#endif

#endif // not TEST_BLIS_TYPED