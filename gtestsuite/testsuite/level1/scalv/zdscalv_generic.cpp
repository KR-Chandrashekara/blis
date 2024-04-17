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
#include "test_scalv.h"

class zdscalvGenericTest :
        public ::testing::TestWithParam<std::tuple<char,        // conj_alpha
                                                   gtint_t,     // n
                                                   gtint_t,     // incx
                                                   double>> {}; // alpha


// Tests using random integers as vector elements.
TEST_P( zdscalvGenericTest, RandomData )
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

// Used to generate a test case with a sensible name.
// Beware that we cannot use fp numbers (e.g., 2.3) in the names,
// so we are only printing int(2.3). This should be enough for debugging purposes.
// If this poses an issue, please reach out.
class zdscalvGenericTestPrint {
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<char, gtint_t, gtint_t, double>> str) const {
        char conj_alpha = std::get<0>(str.param);
        gtint_t n = std::get<1>(str.param);
        gtint_t incx = std::get<2>(str.param);
        double alpha = std::get<3>(str.param);
#ifdef TEST_BLAS
        std::string str_name = "blas_";
#elif TEST_CBLAS
        std::string str_name = "cblas_";
#else  //#elif TEST_BLIS_TYPED
        std::string str_name = "bli_";
#endif
        str_name += "_n" + std::to_string(n);
        str_name += (conj_alpha == 'n') ? "_noconjalpha" : "_conjalpha";
        str_name += "_incx_" + testinghelpers::get_value_string(incx);
        str_name += "_alpha_" + testinghelpers::get_value_string(alpha);
        return str_name;
    }
};

// Black box testing for zdscal.
// Tests with unit-positive increment.
INSTANTIATE_TEST_SUITE_P(
        unitPositiveIncrement,
        zdscalvGenericTest,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'
#ifdef TEST_BLIS_TYPED
                            , 'c'       // this option is BLIS-api specific.
#endif
            ),
            // m: size of vector.
            ::testing::Range(gtint_t(10), gtint_t(101), 10),
            // incx: stride of x vector.
            ::testing::Values(gtint_t(1)),
            // alpha: value of scalar.
            ::testing::Values(
                                double(-5.1),
                                double( 0.0),
                                double( 7.3)
            )
        ),
        ::zdscalvGenericTestPrint()
    );


// Tests for non-unit increments.
INSTANTIATE_TEST_SUITE_P(
        nonUnitPositiveIncrement,
        zdscalvGenericTest,
        ::testing::Combine(
            // conj(alpha): uses n (no_conjugate) since it is real.
            ::testing::Values('n'
#ifdef TEST_BLIS_TYPED
                            , 'c'       // this option is BLIS-api specific.
#endif
            ),
            // m: size of vector.
            ::testing::Range(gtint_t(10), gtint_t(101), 10),
            // incx: stride of x vector.
            ::testing::Values(
                                gtint_t(2),
                                gtint_t(3)
            ),
            // alpha: value of scalar.
            ::testing::Values(
                                double(-5.1),
                                double( 0.0),
                                double( 7.3)
            )
        ),
        ::zdscalvGenericTestPrint()
    );
