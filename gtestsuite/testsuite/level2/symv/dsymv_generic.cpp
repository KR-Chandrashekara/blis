#include <gtest/gtest.h>
#include "test_symv.h"

class dsymvTest :
        public ::testing::TestWithParam<std::tuple<char,
                                                   char,
                                                   char,
                                                   char,
                                                   gtint_t,
                                                   double,
                                                   double,
                                                   gtint_t,
                                                   gtint_t,
                                                   gtint_t,
                                                   char>> {};

TEST_P(dsymvTest, RandomData) {
    using T = double;

    //----------------------------------------------------------
    // Initialize values from the parameters passed through
    // test suite instantiation (INSTANTIATE_TEST_SUITE_P).
    //----------------------------------------------------------
    // matrix storage format(row major, column major)
    char storage = std::get<0>(GetParam());
    // denotes whether matrix a is u,l
    char uploa = std::get<1>(GetParam());
    // denotes whether matrix a is n,c
    char conja = std::get<2>(GetParam());
    // denotes whether vector x is n,c
    char conjx = std::get<3>(GetParam());
    // matrix size n
    gtint_t n  = std::get<4>(GetParam());
    // specifies alpha value
    T alpha = std::get<5>(GetParam());
    // specifies beta value
    T beta = std::get<6>(GetParam());
    // stride size for x:
    gtint_t incx = std::get<7>(GetParam());
    // stride size for y
    gtint_t incy = std::get<8>(GetParam());
    // lda increment.
    // If increment is zero, then the array size matches the matrix size.
    // If increment are nonnegative, the array size is bigger than the matrix size.
    gtint_t lda_inc = std::get<9>(GetParam());
    // specifies the datatype for randomgenerators
    char datatype   = std::get<10>(GetParam());

    // Set the threshold for the errors:
    double thresh = 10*n*testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call test body using these parameters
    //----------------------------------------------------------
    test_symv<T>(storage, uploa, conja, conjx, n, alpha, lda_inc, incx, beta, incy, thresh, datatype);
}

class dsymvTestPrint {
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<char,char,char,char,gtint_t,double,double,gtint_t,gtint_t,gtint_t,char>> str) const {
        char sfm       = std::get<0>(str.param);
        char uploa     = std::get<1>(str.param);
        char conja     = std::get<2>(str.param);
        char conjx     = std::get<3>(str.param);
        gtint_t n      = std::get<4>(str.param);
        double alpha   = std::get<5>(str.param);
        double beta    = std::get<6>(str.param);
        gtint_t incx   = std::get<7>(str.param);
        gtint_t incy   = std::get<8>(str.param);
        gtint_t ld_inc = std::get<9>(str.param);
        char datatype  = std::get<10>(str.param);
#ifdef TEST_BLAS
        std::string str_name = "dsymv_";
#elif TEST_CBLAS
        std::string str_name = "cblas_dsymv";
#else  //#elif TEST_BLIS_TYPED
        std::string str_name = "blis_dsymv";
#endif
        str_name    = str_name + "_" + sfm;
        str_name    = str_name + "_" + uploa+conja+conjx;
        str_name    = str_name + "_" + std::to_string(n);
        std::string alpha_str = ( alpha > 0) ? std::to_string(int(alpha)) : ("m" + std::to_string(int(std::abs(alpha))));
        std::string beta_str = ( beta > 0) ? std::to_string(int(beta)) : ("m" + std::to_string(int(std::abs(beta))));
        str_name    = str_name + "_a" + alpha_str;
        str_name    = str_name + "_a" + beta_str;
        std::string incx_str = ( incx > 0) ? std::to_string(incx) : "m" + std::to_string(std::abs(incx));
        std::string incy_str = ( incy > 0) ? std::to_string(incy) : "m" + std::to_string(std::abs(incy));
        str_name    = str_name + "_" + incx_str;
        str_name    = str_name + "_" + incy_str;
        str_name    = str_name + "_" + std::to_string(ld_inc);
        str_name    = str_name + "_" + datatype;
        return str_name;
    }
};

// Black box testing.
INSTANTIATE_TEST_SUITE_P(
        Blackbox,
        dsymvTest,
        ::testing::Combine(
            ::testing::Values('c'
#ifndef TEST_BLAS
            ,'r'
#endif
            ),                                                               // storage format
            ::testing::Values('u','l'),                                      // uploa
            ::testing::Values('n'),                                          // conja
            ::testing::Values('n'),                                          // conjx
            ::testing::Range(gtint_t(10), gtint_t(31), 10),                  // n
            ::testing::Values( 1.0, -2.0 ),                                  // alpha
            ::testing::Values( 2.0, -1.0 ),                                  // beta
            ::testing::Values(gtint_t(1)),                                   // stride size for x
            ::testing::Values(gtint_t(1)),                                   // stride size for y
            ::testing::Values(gtint_t(0), gtint_t(3)),                       // increment to the leading dim of a
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::dsymvTestPrint()
    );