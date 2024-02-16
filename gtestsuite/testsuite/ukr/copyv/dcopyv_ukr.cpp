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
#include "test_copyv_ukr.h"

class dcopyvUkrTest :
        public ::testing::TestWithParam<std::tuple<dcopyv_ker_ft,
                                                   char,
                                                   gtint_t,
                                                   gtint_t,
                                                   gtint_t>> {};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(dcopyvUkrTest);

// Tests using random integers as vector elements.
TEST_P( dcopyvUkrTest, AccuracyCheck )
{
    using T = double;
    //----------------------------------------------------------
    // Initialize values from the parameters passed through
    // test suite instantiation (INSTANTIATE_TEST_SUITE_P).
    //----------------------------------------------------------
    dcopyv_ker_ft ukr_fp = std::get<0>(GetParam());
    // denotes whether vec x is n,c
    char conjx = std::get<1>(GetParam());
    // vector length:
    gtint_t n = std::get<2>(GetParam());
    // stride size for x:
    gtint_t incx = std::get<3>(GetParam());
    // stride size for y:
    gtint_t incy = std::get<4>(GetParam());

    // Set the threshold for the errors:
    double thresh = testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call generic test body using those parameters
    //----------------------------------------------------------
    test_copyv_ukr<T, dcopyv_ker_ft>( ukr_fp, conjx, n, incx, incy, thresh );
}

// Used to generate a test case with a sensible name.
// Beware that we cannot use fp numbers (e.g., 2.3) in the names,
// so we are only printing int(2.3). This should be enough for debugging purposes.
// If this poses an issue, please reach out.
class dcopyvUkrTestPrint {
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<dcopyv_ker_ft,char,gtint_t,gtint_t,gtint_t>> str) const {
        char conjx    = std::get<1>(str.param);
        gtint_t n     = std::get<2>(str.param);
        gtint_t incx  = std::get<3>(str.param);
        gtint_t incy  = std::get<4>(str.param);

        std::string str_name = "dcopyv_ukr";
        str_name += "_n" + std::to_string(n);
        str_name += "_conjx" + std::string(&conjx, 1);
        std::string incx_str = ( incx > 0) ? std::to_string(incx) : "m" + std::to_string(std::abs(incx));
        str_name += "_incx" + incx_str;
        std::string incy_str = ( incy > 0) ? std::to_string(incy) : "m" + std::to_string(std::abs(incy));
        str_name += "_incy" + incy_str;
        return str_name;
    }
};

#if defined(BLIS_KERNELS_ZEN) && defined(GTEST_AVX2FMA3)
/*
    Unit testing for functionality of bli_dcopyv_zen_int kernel.
    The code structure for bli_dcopyv_zen_int( ... ) is as follows :
    For unit strides :
        Main loop    :  In blocks of 64 -->   L64
        Fringe loops :  In blocks of 32 -->   L32
                        In blocks of 16 -->   L16
                        In blocks of 8  -->   L8
                        In blocks of 4  -->   L4
                        Element-wise loop --> LScalar

    For non-unit strides : A single loop, to process element wise.
*/
// Unit testing with Unit Strides(US), across all loops.
INSTANTIATE_TEST_SUITE_P(
        bli_dcopyv_zen_int_unitStrides,
        dcopyvUkrTest,
        ::testing::Combine(
            ::testing::Values(bli_dcopyv_zen_int),
            ::testing::Values('n'),                   // conjugate parameter, 'n' for dcopyv
            ::testing::Values(// Testing the loops standalone
                              gtint_t(64),            // size n, for L64
                              gtint_t(32),            // L32
                              gtint_t(16),            // L16
                              gtint_t(8),             // L8
                              gtint_t(4),             // L4
                              gtint_t(3),             // LScalar
                              // Testing the loops with combinations
                              // 5*L64
                              gtint_t(320),
                              // 3*L64 + L32
                              gtint_t(352),
                              // 3*L64 + L32 + L16
                              gtint_t(368),
                              // 3*L64 + L32 + L16 + L8
                              gtint_t(376),
                              // 3*L64 + L32 + L16 + L8 + L4
                              gtint_t(380),
                              // 3*L64 + L32 + L16 + L8 + L4 + LScalar
                              gtint_t(383)),
            ::testing::Values(gtint_t(1)),             // stride size for x
            ::testing::Values(gtint_t(1))              // stride size for y
        ),
        ::dcopyvUkrTestPrint()
    );

// Unit testing with Non-Unit Strides(US), across all loops.
INSTANTIATE_TEST_SUITE_P(
        bli_dcopyv_zen_int_nonUnitStrides,
        dcopyvUkrTest,
        ::testing::Combine(
            ::testing::Values(bli_dcopyv_zen_int),
            ::testing::Values('n'),                   // conjugate parameter, 'n' for dcopyv
            ::testing::Values(gtint_t(25), gtint_t(37)), // size of the vector
            ::testing::Values(gtint_t(5)), // stride size for x
            ::testing::Values(gtint_t(3)) // stride size for y
        ),
        ::dcopyvUkrTestPrint()
    );
#endif