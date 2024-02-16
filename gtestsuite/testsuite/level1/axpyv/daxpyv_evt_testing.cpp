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
#include "test_axpyv.h"

class daxpyvEVTTest :
        public ::testing::TestWithParam<std::tuple<char,         // transpose
                                                   gtint_t,      // n, size of the vector
                                                   gtint_t,      // incx
                                                   gtint_t,      // incy
                                                   gtint_t,      // xi, index for exval in x
                                                   double,       // xexval
                                                   gtint_t,      // yi, index for exval in y
                                                   double,       // yexval
                                                   double>> {};  // alpha
// Tests using random values as vector elements,
// with exception values on the passed indices.
TEST_P(daxpyvEVTTest, ExceptionData)
{
    using T = double;
    //----------------------------------------------------------
    // Initialize values from the parameters passed through
    // test suite instantiation (INSTANTIATE_TEST_SUITE_P).
    //----------------------------------------------------------
    // denotes whether x or conj(x) will be added to y:
    char conj_x = std::get<0>(GetParam());
    // vector length:
    gtint_t n = std::get<1>(GetParam());
    // stride size for x:
    gtint_t incx = std::get<2>(GetParam());
    // stride size for y:
    gtint_t incy = std::get<3>(GetParam());
    // index for exval in x
    gtint_t xi = std::get<4>(GetParam());
    // exval for x
    T xexval = std::get<5>(GetParam());
    // index for exval in y
    gtint_t yj = std::get<6>(GetParam());
    // exval for x
    T yexval = std::get<7>(GetParam());
    // alpha
    T alpha = std::get<8>(GetParam());

    // Set the threshold for the errors:
    double thresh = 20 * testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call generic test body using those parameters
    //----------------------------------------------------------
    test_axpyv<T>(conj_x, n, incx, incy, alpha, xi, xexval,
                   yj, yexval, thresh);
}

// Test-case logger : Used to print the test-case details when vectors have exception value.
// The string format is as follows :
// n(vec_size)_(conjx/noconjx)_incx(m)(abs_incx)_incy(m)(abs_incy)_X_(xi)_(xexval)_(yi)_(yexval)_a(alpha_val)
class daxpyvEVTVecPrint
{
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<char, gtint_t, gtint_t, gtint_t, gtint_t, double, gtint_t, double, double>> str) const
    {
        char conjx = std::get<0>(str.param);
        gtint_t n = std::get<1>(str.param);
        gtint_t incx = std::get<2>(str.param);
        gtint_t incy = std::get<3>(str.param);
        gtint_t xi = std::get<4>(str.param);
        double xexval = std::get<5>(str.param);
        gtint_t yj = std::get<6>(str.param);
        double yexval = std::get<7>(str.param);
        double alpha = std::get<8>(str.param);
#ifdef TEST_BLAS
        std::string str_name = "daxpy_";
#elif TEST_CBLAS
        std::string str_name = "cblas_daxpy";
#else // #elif TEST_BLIS_TYPED
        std::string str_name = "bli_daxpyv";
#endif
        str_name += "_n" + std::to_string(n);
        str_name += ( conjx == 'n' )? "_noconjx" : "_conjx";
        std::string incx_str = ( incx > 0) ? std::to_string(incx) : "m" + std::to_string(std::abs(incx));
        str_name += "_incx" + incx_str;
        std::string incy_str = ( incy > 0) ? std::to_string(incy) : "m" + std::to_string(std::abs(incy));
        str_name += "_incy" + incy_str;
        std::string xexval_str = testinghelpers::get_value_string(xexval);
        std::string yexval_str = testinghelpers::get_value_string(yexval);
        str_name = str_name + "_X_" + std::to_string(xi);
        str_name = str_name + "_" + xexval_str;
        str_name = str_name + "_Y_" + std::to_string(yj);
        str_name = str_name + "_" + yexval_str;
        std::string alpha_str = testinghelpers::get_value_string(alpha);
        str_name = str_name + "_a" + alpha_str;
        return str_name;
    }
};

// Test-case logger : Used to print the test-case details when alpha/beta have exception value.
// The string format is as follows :
// n(vec_size)_(conjx/noconjx)_incx(m)(abs_incx)_incy(m)(abs_incy)_a(alpha_val)
class daxpyvAlphaBetaPrint
{
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<char, gtint_t, gtint_t, gtint_t, gtint_t, double, gtint_t, double, double>> str) const
    {
        char conjx = std::get<0>(str.param);
        gtint_t n = std::get<1>(str.param);
        gtint_t incx = std::get<2>(str.param);
        gtint_t incy = std::get<3>(str.param);
        double alpha = std::get<8>(str.param);
#ifdef TEST_BLAS
        std::string str_name = "daxpy_";
#elif TEST_CBLAS
        std::string str_name = "cblas_daxpy";
#else // #elif TEST_BLIS_TYPED
        std::string str_name = "bli_daxpyv";
#endif
        str_name += "_n" + std::to_string(n);
        str_name += ( conjx == 'n' )? "_noconjx" : "_conjx";
        std::string incx_str = ( incx > 0) ? std::to_string(incx) : "m" + std::to_string(std::abs(incx));
        str_name += "_incx" + incx_str;
        std::string incy_str = ( incy > 0) ? std::to_string(incy) : "m" + std::to_string(std::abs(incy));
        str_name += "_incy" + incy_str;
        std::string alpha_str = testinghelpers::get_value_string(alpha);
        str_name = str_name + "_a" + alpha_str;
        return str_name;
    }
};

static double NaN = std::numeric_limits<double>::quiet_NaN();
static double Inf = std::numeric_limits<double>::infinity();

/*
    Exception value testing on vectors(Zen3) :
    DAXPBY currently uses the bli_daxpyv_zen_int10( ... ) kernel for computation on zen3
    machines.
    The sizes and indices given in the instantiator are to ensure code coverage inside
    the kernel, and to verify the compliance accordingly.

    Kernel structure for bli_daxpyv_zen_int10( ... ) :
    Main loop    :  In blocks of 52 --> L52
    Fringe loops :  In blocks of 40 --> L40
                    In blocks of 20 --> L20
                    In blocks of 16 --> L16
                    In blocks of 8  --> L8
                    In blocks of 4  --> L4
                    Element-wise loop --> LScalar

    For size 535 :  L52*10 + L8 + L4 + 3(LScalar)
    Indices are  :  0, 519 -> In L52
                    527    -> In L8
                    531    -> In L4
                    534    -> In LScalar


    For size 556 :  L52*10 + L20 + L16
    Indices are  :  0, 519 -> In L52
                    539    -> In L20
                    555    -> In L16


    For size 560 :  L52*10 + L40
    Indices are  :  0, 519 -> In L52
                    559    -> In L40

    The alpha values are such that they check for compliance against possible
    optimizations that might have been done.

    P.S : Some test cases also check whether NaN has to be induced in the computation
          as a result of 0.0 * { NaN, +Inf, -Inf }.
*/

// Exception value testing(on X vector alone) with unit strides
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_vecX_unitStrides_zen3,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(535), gtint_t(556), gtint_t(560)),            // n, size of vectors with unit-stride
        ::testing::Values(gtint_t(1)),                                          // stride size for x
        ::testing::Values(gtint_t(1)),                                          // stride size for y
        ::testing::Values(gtint_t(0), gtint_t(519), gtint_t(527),
                          gtint_t(531), gtint_t(534), gtint_t(539),
                          gtint_t(555), gtint_t(559)),                          // indices to set exception values on x
        ::testing::Values(NaN, -Inf, Inf),                                      // exception values to set on x
        ::testing::Values(gtint_t(0)),                                          // dummy index on y
        ::testing::Values(double(0.0)),                                         // dummy value on y
        ::testing::Values(double(0.0), double(1.0), double(-1.0), double(-3.3)) // alpha
        ),
    ::daxpyvEVTVecPrint());

// Exception value testing(on Y vector alone) with unit strides
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_vecY_unitStrides_zen3,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(535), gtint_t(556), gtint_t(560)),            // n, size of vectors with unit-stride
        ::testing::Values(gtint_t(1)),                                          // stride size for x
        ::testing::Values(gtint_t(1)),                                          // stride size for y
        ::testing::Values(gtint_t(0)),                                          // dummy index on x
        ::testing::Values(double(0.0)),                                         // dummy value on x
        ::testing::Values(gtint_t(0), gtint_t(519), gtint_t(527),
                          gtint_t(531), gtint_t(534), gtint_t(539),
                          gtint_t(555), gtint_t(559)),                          // indices to set exception values on y
        ::testing::Values(NaN, -Inf, Inf),                                      // exception values to set on y
        ::testing::Values(double(0.0), double(1.0), double(-1.0), double(-3.3)) // alpha
        ),
    ::daxpyvEVTVecPrint());

// Exception value testing(on X and Y vectors) with unit strides
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_vecXY_unitStrides_zen3,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(535), gtint_t(556), gtint_t(560)),            // n, size of vectors with unit-stride
        ::testing::Values(gtint_t(1)),                                          // stride size for x
        ::testing::Values(gtint_t(1)),                                          // stride size for y
        ::testing::Values(gtint_t(0), gtint_t(519), gtint_t(527),
                          gtint_t(531), gtint_t(534), gtint_t(539),
                          gtint_t(555), gtint_t(559)),                          // indices to set exception values on x
        ::testing::Values(NaN, -Inf, Inf),                                      // exception values to set on x
        ::testing::Values(gtint_t(0), gtint_t(519), gtint_t(527),
                          gtint_t(531), gtint_t(534), gtint_t(539),
                          gtint_t(555), gtint_t(559)),                          // indices to set exception values on y
        ::testing::Values(NaN, -Inf, Inf),                                      // exception values to set on y
        ::testing::Values(double(0.0), double(1.0), double(-1.0), double(-3.3)) // alpha
        ),
    ::daxpyvEVTVecPrint());

/*
    Exception value testing on vectors(Zen4) :
    DAXPY currently uses the bli_daxpyv_zen_int_avx512( ... ) kernel for computation on zen4
    machines.
    The sizes and indices given in the instantiator are to ensure code coverage inside
    the kernel, and to verify the compliance accordingly.

    Kernel structure for bli_daxpyv_zen_int_avx512( ... ) :
    Main loop    :  In blocks of 64 --> L52
    Fringe loops :  In blocks of 32 --> L40
                    In blocks of 16 --> L16
                    In blocks of 8  --> L8
                    In blocks of 4  --> L4
                    Element-wise loop --> LScalar

    For size 383 :  L64*5 + L32 + L16 + L8 + L4 + 3(LScalar)
    Indices are  :  0, 319 -> In L64
                    351    -> In L32
                    367    -> In L16
                    375    -> In L8
                    379    -> In L4
                    382    -> In LScalar

    The alpha values are such that they check for compliance against possible
    optimizations that might have been done.

    P.S : Some test cases also check whether NaN has to be induced in the computation
          as a result of 0.0 * { NaN, +Inf, -Inf }.
*/
// Exception value testing(on X vector alone) with unit strides
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_vecX_unitStrides_zen4,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(383)),                                        // n, size of vectors with unit-stride
        ::testing::Values(gtint_t(1)),                                          // stride size for x
        ::testing::Values(gtint_t(1)),                                          // stride size for y
        ::testing::Values(gtint_t(0), gtint_t(319), gtint_t(351),
                          gtint_t(367), gtint_t(375), gtint_t(379),
                          gtint_t(382)),                                        // indices to set exception values on x
        ::testing::Values(NaN, -Inf, Inf),                                      // exception values to set on x
        ::testing::Values(gtint_t(0)),                                          // dummy index on y
        ::testing::Values(double(0.0)),                                         // dummy value on y
        ::testing::Values(double(0.0), double(1.0), double(-1.0), double(-3.3)) // alpha
        ),
    ::daxpyvEVTVecPrint());

// Exception value testing(on Y vector alone) with unit strides
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_vecY_unitStrides_zen4,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(383)),                                        // n, size of vectors with unit-stride
        ::testing::Values(gtint_t(1)),                                          // stride size for x
        ::testing::Values(gtint_t(1)),                                          // stride size for y
        ::testing::Values(gtint_t(0)),                                          // dummy index on x
        ::testing::Values(double(0.0)),                                         // dummy value on x
        ::testing::Values(gtint_t(0), gtint_t(319), gtint_t(351),
                          gtint_t(367), gtint_t(375), gtint_t(379),
                          gtint_t(382)),                                        // indices to set exception values on y
        ::testing::Values(NaN, -Inf, Inf),                                      // exception values to set on y
        ::testing::Values(double(0.0), double(1.0), double(-1.0), double(-3.3)) // alpha
        ),
    ::daxpyvEVTVecPrint());

// Exception value testing(on X and Y vectors) with unit strides
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_vecXY_unitStrides_zen4,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(383)),                                        // n, size of vectors with unit-stride
        ::testing::Values(gtint_t(1)),                                          // stride size for x
        ::testing::Values(gtint_t(1)),                                          // stride size for y
        ::testing::Values(gtint_t(0), gtint_t(319), gtint_t(351),
                          gtint_t(367), gtint_t(375), gtint_t(379),
                          gtint_t(382)),                                        // indices to set exception values on x
        ::testing::Values(NaN, -Inf, Inf),                                      // exception values to set on x
        ::testing::Values(gtint_t(0), gtint_t(319), gtint_t(351),
                          gtint_t(367), gtint_t(375), gtint_t(379),
                          gtint_t(382)),                                        // indices to set exception values on y
        ::testing::Values(NaN, -Inf, Inf),                                      // exception values to set on y
        ::testing::Values(double(0.0), double(1.0), double(-1.0), double(-3.3)) // alpha
        ),
    ::daxpyvEVTVecPrint());

// Exception value testing(on vectors) with non-unit strides
// We have to test a single scalar loop. The indices are such
// that we cover _vecX_, _vecY_ and _vecXY_ cases together.
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_vecXY_nonUnitStrides,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(50)),                                         // n, size of vectors with non-unit strides
        ::testing::Values(gtint_t(3)),                                          // stride size for x
        ::testing::Values(gtint_t(5)),                                          // stride size for y
        ::testing::Values(gtint_t(1), gtint_t(27), gtint_t(49)),                // indices to set exception values on x
        ::testing::Values(NaN, -Inf, Inf, 2.9),                                 // exception values to set on x
        ::testing::Values(gtint_t(0), gtint_t(26), gtint_t(49)),                // indices to set exception values on y
        ::testing::Values(NaN, -Inf, Inf, -1.5),                                // exception values to set on y
        ::testing::Values(double(0.0), double(1.0), double(-1.0), double(-3.3)) // alpha
        ),
    ::daxpyvEVTVecPrint());

/*
    Exception value testing on alpha :
    Alpha values are set to Nan, +Inf or -Inf. A dummy
    value of 0.0 is induced in X and Y vectors, to further
    verify the propagation.

    The size(s) for _zen3 and _zen4 instantiators are chosen such
    that code coverage is ensured in the respective kernels.
*/
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_alpha_unitStrides_zen3,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(535), gtint_t(556), gtint_t(560)),            // n, size of vectors with unit strides
        ::testing::Values(gtint_t(1)),                                          // stride size for x
        ::testing::Values(gtint_t(1)),                                          // stride size for y
        ::testing::Values(gtint_t(0)),                                          // indices to set zero on x
        ::testing::Values(double(0.0)),
        ::testing::Values(gtint_t(0)),                                          // indices to set zero on y
        ::testing::Values(double(0.0)),
        ::testing::Values(NaN, -Inf, Inf)                                       // alpha
        ),
    ::daxpyvEVTVecPrint());

// Exception value testing(on alpha) with unit strided vectors
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_alpha_unitStrides_zen4,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(383)),                                        // n, size of vectors with unit strides
        ::testing::Values(gtint_t(1)),                                          // stride size for x
        ::testing::Values(gtint_t(1)),                                          // stride size for y
        ::testing::Values(gtint_t(0)),                                          // indices to set zero on x
        ::testing::Values(double(0.0)),
        ::testing::Values(gtint_t(0)),                                          // indices to set zero on y
        ::testing::Values(double(0.0)),
        ::testing::Values(NaN, -Inf, Inf)                                       // alpha
        ),
    ::daxpyvEVTVecPrint());

// Exception value testing(on alpha) with non-unit strided vectors
INSTANTIATE_TEST_SUITE_P(
    exceptionValue_alpha_nonUnitStrides,
    daxpyvEVTTest,
    ::testing::Combine(
        ::testing::Values('n' // n: use x, c: use conj(x)
#ifdef TEST_BLIS_TYPED
                          ,
                          'c' // this option is BLIS-api specific.
#endif
                          ),
        ::testing::Values(gtint_t(50)),                 // n, size of vectors with non-unit strides
        ::testing::Values(gtint_t(3)),                  // stride size for x
        ::testing::Values(gtint_t(5)),                  // stride size for y
        ::testing::Values(gtint_t(0), gtint_t(25)),     // indices to set zero on x
        ::testing::Values(double(0.0)),
        ::testing::Values(gtint_t(0), gtint_t(40)),     // indices to set zero on y
        ::testing::Values(double(0.0)),
        ::testing::Values(NaN, -Inf, Inf)               // alpha
        ),
    ::daxpyvEVTVecPrint());