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
#include "test_axpbyv_ukr.h"

class daxpbyvUkrTest :
        public ::testing::TestWithParam<std::tuple<daxpbyv_ker_ft,  // Function pointer type for daxpbyv kernels
                                                   char,            // conjx
                                                   gtint_t,         // n
                                                   gtint_t,         // incx
                                                   gtint_t,         // incy
                                                   double,          // alpha
                                                   double>> {};     // beta

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(daxpbyvUkrTest);

// Tests using random integers as vector elements.
TEST_P( daxpbyvUkrTest, AccuracyCheck )
{
    using T = double;

    //----------------------------------------------------------
    // Initialize values from the parameters passed through
    // test suite instantiation (INSTANTIATE_TEST_SUITE_P).
    //----------------------------------------------------------

    // Assign the kernel address to the function pointer
    daxpbyv_ker_ft ukr_fp = std::get<0>(GetParam());
    // denotes whether x or conj(x) will be added to y:
    char conj_x = std::get<1>(GetParam());
    // vector length:
    gtint_t n = std::get<2>(GetParam());
    // stride size for x:
    gtint_t incx = std::get<3>(GetParam());
    // stride size for y:
    gtint_t incy = std::get<4>(GetParam());
    // alpha
    T alpha = std::get<5>(GetParam());
    // beta
    T beta = std::get<6>(GetParam());

    // Set the threshold for the errors:
    double thresh = 3 * testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call generic test body using those parameters
    //----------------------------------------------------------
    test_axpbyv_ukr<T, daxpbyv_ker_ft>( ukr_fp, conj_x, n, incx, incy, alpha, beta, thresh );
}

// Test-case logger : Used to print the test-case details for unit testing the kernels.
// NOTE : The kernel name is the prefix in instantiator name, and thus is not printed
// with this logger.
class daxpbyvUkrTestPrint {
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<daxpbyv_ker_ft,char,gtint_t,gtint_t,gtint_t,double,double>> str) const {
        char conjx     = std::get<1>(str.param);
        gtint_t n     = std::get<2>(str.param);
        gtint_t incx  = std::get<3>(str.param);
        gtint_t incy  = std::get<4>(str.param);
        double alpha  = std::get<5>(str.param);
        double beta   = std::get<6>(str.param);

        std::string str_name = "daxpbyv_ukr";
        str_name += "_n" + std::to_string(n);
        str_name += ( conjx == 'n' )? "_noconjx" : "_conjx";
        std::string incx_str = ( incx > 0) ? std::to_string(incx) : "m" + std::to_string(std::abs(incx));
        str_name += "_incx" + incx_str;
        std::string incy_str = ( incy > 0) ? std::to_string(incy) : "m" + std::to_string(std::abs(incy));
        str_name += "_incy" + incy_str;
        std::string alpha_str = ( alpha > 0) ? std::to_string(int(alpha)) : "m" + std::to_string(int(std::abs(alpha)));
        str_name = str_name + "_a" + alpha_str;
        std::string beta_str = ( beta > 0) ? std::to_string(int(beta)) : "m" + std::to_string(int(std::abs(beta)));
        str_name = str_name + "_b" + beta_str;
        return str_name;
    }
};

#if defined(BLIS_KERNELS_ZEN) && defined(GTEST_AVX2FMA3)
/*
    Unit testing for functionality of bli_daxpbyv_zen_int10 kernel.
    The code structure for bli_daxpbyv_zen_int10( ... ) is as follows :
    For unit strides :
        Main loop    :  In blocks of 40 --> L40
        Fringe loops :  In blocks of 20 --> L20
                        In blocks of 8  --> L8
                        In blocks of 4  --> L4
                        Element-wise loop --> LScalar

    For non-unit strides : A single loop, to process element wise.
*/

// Unit testing with unit stride, across all loops.
INSTANTIATE_TEST_SUITE_P(
        bli_daxpbyv_zen_int10_unitStrides,
        daxpbyvUkrTest,
        ::testing::Combine(
            ::testing::Values(bli_daxpbyv_zen_int10), // kernel address
            ::testing::Values('n'),                   // use x, not conj(x) (since it is real)
            ::testing::Values(// Testing the loops standalone
                              gtint_t(40),            // size n, for L40
                              gtint_t(20),            // L20
                              gtint_t(8),             // L8
                              gtint_t(4),             // L4
                              gtint_t(2),             // LScalar
                              // Testing the loops with combination
                              // 3*L40
                              gtint_t(120),
                              // 3*L40 + L20
                              gtint_t(140),
                              // 3*L40 + L20 + L8
                              gtint_t(148),
                              // 3*L40 + L20 + L8 + L4
                              gtint_t(152),
                              // 3*L40 + L20 + L8 + L4 + LScalar
                              gtint_t(155)),
            ::testing::Values(gtint_t(1)),            // stride size for x
            ::testing::Values(gtint_t(1)),            // stride size for y
            ::testing::Values(double(2.2)),  // alpha
            ::testing::Values(double(-1.8))  // beta
        ),
        ::daxpbyvUkrTestPrint()
    );

// Unit testing for non unit strides
INSTANTIATE_TEST_SUITE_P(
        bli_daxpbyv_zen_int10_nonUnitStrides,
        daxpbyvUkrTest,
        ::testing::Combine(
            ::testing::Values(bli_daxpbyv_zen_int10), // kernel address
            ::testing::Values('n'),                   // use x, not conj(x) (since it is real)
            ::testing::Values(gtint_t(10),            // n, size of the vector
                              gtint_t(25)),
            ::testing::Values(gtint_t(5)), // stride size for x
            ::testing::Values(gtint_t(3)), // stride size for y
            ::testing::Values(double(2.2)), // alpha
            ::testing::Values(double(-1.8))  // beta
        ),
        ::daxpbyvUkrTestPrint()
    );

/*
    Unit testing for functionality of bli_daxpbyv_zen_int kernel.
    The code structure for bli_daxpbyv_zen_int10( ... ) is as follows :
    For unit strides :
        Main loop    :  In blocks of 16 --> L16
                        Element-wise loop --> LScalar

    For non-unit strides : A single loop, to process element wise.
*/
// Unit testing with Unit Strides, across all loops.
INSTANTIATE_TEST_SUITE_P(
        bli_daxpbyv_zen_int_unitStrides,
        daxpbyvUkrTest,
        ::testing::Combine(
            ::testing::Values(bli_daxpbyv_zen_int),   // kernel address
            ::testing::Values('n'),                   // use x, not conj(x) (since it is real)
            ::testing::Values(gtint_t(16),            // size n, for L16
                              gtint_t(48),            // 3*L16
                              gtint_t(57)),           // 3*L16 + 9(LScalar)
            ::testing::Values(gtint_t(1)),            // stride size for x
            ::testing::Values(gtint_t(1)),            // stride size for y
            ::testing::Values(double(2.2)),  // alpha
            ::testing::Values(double(-1.8))  // beta
        ),
        ::daxpbyvUkrTestPrint()
    );

// Unit testing for Non-Unit Stride
INSTANTIATE_TEST_SUITE_P(
        bli_daxpbyv_zen_int_nonUnitStrides,
        daxpbyvUkrTest,
        ::testing::Combine(
            ::testing::Values(bli_daxpbyv_zen_int),   // kernel address
            ::testing::Values('n'),                   // use x, not conj(x) (since it is real)
            ::testing::Values(gtint_t(10),            // n, size of the vector
                              gtint_t(25)),
            ::testing::Values(gtint_t(5)), // stride size for x
            ::testing::Values(gtint_t(3)), // stride size for y
            ::testing::Values(double(-4.1)), // alpha
            ::testing::Values(double(3.9))   // beta
        ),
        ::daxpbyvUkrTestPrint()
    );
#endif