#include <gtest/gtest.h>
#include "test_ger.h"

class cgerTest :
        public ::testing::TestWithParam<std::tuple<char,
                                                   char,
                                                   char,
                                                   gtint_t,
                                                   gtint_t,
                                                   scomplex,
                                                   gtint_t,
                                                   gtint_t,
                                                   gtint_t,
                                                   char>> {};

TEST_P(cgerTest, RandomData) {
    using T = scomplex;

    //----------------------------------------------------------
    // Initialize values from the parameters passed through
    // test suite instantiation (INSTANTIATE_TEST_SUITE_P).
    //----------------------------------------------------------
    // matrix storage format(row major, column major)
    char storage = std::get<0>(GetParam());
    // denotes whether vector x is n,c
    char conjx = std::get<1>(GetParam());
    // denotes whether vector y is n,c
    char conjy = std::get<2>(GetParam());
    // matrix size m
    gtint_t m  = std::get<3>(GetParam());
    // matrix size n
    gtint_t n  = std::get<4>(GetParam());
    // specifies alpha value
    T alpha = std::get<5>(GetParam());
    // stride size for x:
    gtint_t incx = std::get<6>(GetParam());
    // stride size for y:
    gtint_t incy = std::get<7>(GetParam());
    // lda increment.
    // If increment is zero, then the array size matches the matrix size.
    // If increment are nonnegative, the array size is bigger than the matrix size.
    gtint_t lda_inc = std::get<8>(GetParam());
    // specifies the datatype for randomgenerators
    char datatype   = std::get<9>(GetParam());

    // Set the threshold for the errors:
    double thresh = 2*std::max(m,n)*testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call test body using these parameters
    //----------------------------------------------------------
    test_ger<T>(storage, conjx, conjy, m, n, alpha, incx, incy, lda_inc, thresh, datatype);
}

class cgerTestPrint {
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<char,char,char,gtint_t,gtint_t,scomplex,gtint_t,gtint_t,gtint_t,char>> str) const {
        char sfm       = std::get<0>(str.param);
        char conjx     = std::get<1>(str.param);
        char conjy     = std::get<2>(str.param);
        gtint_t m      = std::get<3>(str.param);
        gtint_t n      = std::get<4>(str.param);
        scomplex alpha = std::get<5>(str.param);
        gtint_t incx   = std::get<6>(str.param);
        gtint_t incy   = std::get<7>(str.param);
        gtint_t ld_inc = std::get<8>(str.param);
        char datatype  = std::get<9>(str.param);
#ifdef TEST_BLAS
        std::string str_name = "cger_";
#elif TEST_CBLAS
        std::string str_name = "cblas_cger";
#else  //#elif TEST_BLIS_TYPED
        std::string str_name = "blis_cger";
#endif
        str_name    = str_name + "_" + sfm;
        str_name    = str_name + "_" + conjx+conjy;
        str_name    = str_name + "_" + std::to_string(m);
        str_name    = str_name + "_" + std::to_string(n);
        std::string incx_str = ( incx > 0) ? std::to_string(incx) : "m" + std::to_string(std::abs(incx));
        std::string incy_str = ( incy > 0) ? std::to_string(incy) : "m" + std::to_string(std::abs(incy));
        str_name    = str_name + "_" + incx_str;
        str_name    = str_name + "_" + incy_str;
        std::string alpha_str = ( alpha.real > 0) ? std::to_string(int(alpha.real)) : ("m" + std::to_string(int(std::abs(alpha.real))));
                    alpha_str = alpha_str + "pi" + (( alpha.imag > 0) ? std::to_string(int(alpha.imag)) : ("m" + std::to_string(int(std::abs(alpha.imag)))));
        str_name    = str_name + "_a" + alpha_str;
        str_name    = str_name + "_" + std::to_string(ld_inc);
        str_name    = str_name + "_" + datatype;
        return str_name;
    }
};

// Black box testing.
INSTANTIATE_TEST_SUITE_P(
        Blackbox,
        cgerTest,
        ::testing::Combine(
            ::testing::Values('c'
#ifndef TEST_BLAS
            ,'r'
#endif
            ),                                                               // storage format
            ::testing::Values('n'),                                          // conjx
            ::testing::Values('n','c'),                                      // conjy
            ::testing::Range(gtint_t(10), gtint_t(31), 10),                  // m
            ::testing::Range(gtint_t(10), gtint_t(31), 10),                  // n
            ::testing::Values(scomplex{1.0, -2.0}),                          // alpha
            ::testing::Values(gtint_t(1)),                                   // stride size for x
            ::testing::Values(gtint_t(1)),                                   // stride size for y
            ::testing::Values(gtint_t(0), gtint_t(2)),                       // increment to the leading dim of a
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::cgerTestPrint()
    );