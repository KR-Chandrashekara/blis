#include <gtest/gtest.h>
#include "test_axpbyv.h"

class saxpbyvGenericTest :
        public ::testing::TestWithParam<std::tuple<char,
                                                   gtint_t,
                                                   gtint_t,
                                                   gtint_t,
                                                   float,
                                                   float,
                                                   char>> {};
// Tests using random integers as vector elements.
TEST_P( saxpbyvGenericTest, RandomData )
{
    using T = float;
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
    // alpha
    T alpha = std::get<4>(GetParam());
    // beta
    T beta = std::get<5>(GetParam());
    // specifies the datatype for randomgenerators
    char datatype = std::get<6>(GetParam());

    // Set the threshold for the errors:
    float thresh = testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call generic test body using those parameters
    //----------------------------------------------------------
    test_axpbyv<T>(conj_x, n, incx, incy, alpha, beta, thresh, datatype);
}

// Used to generate a test case with a sensible name.
// Beware that we cannot use fp numbers (e.g., 2.3) in the names,
// so we are only printing int(2.3). This should be enough for debugging purposes.
// If this poses an issue, please reach out.
class saxpbyvGenericTestPrint {
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<char,gtint_t,gtint_t,gtint_t,float,float,char>> str) const {
        char conj     = std::get<0>(str.param);
        gtint_t n     = std::get<1>(str.param);
        gtint_t incx  = std::get<2>(str.param);
        gtint_t incy  = std::get<3>(str.param);
        float alpha   = std::get<4>(str.param);
        float beta    = std::get<5>(str.param);
        char datatype = std::get<6>(str.param);
#ifdef TEST_BLAS
        std::string str_name = "saxpby_";
#elif TEST_CBLAS
        std::string str_name = "cblas_saxpby";
#else  //#elif TEST_BLIS_TYPED
        std::string str_name = "bli_saxpbyv";
#endif
        str_name += "_" + std::to_string(n);
        str_name += "_" + std::string(&conj, 1);
        std::string incx_str = ( incx > 0) ? std::to_string(incx) : "m" + std::to_string(std::abs(incx));
        str_name += "_" + incx_str;
        std::string incy_str = ( incy > 0) ? std::to_string(incy) : "m" + std::to_string(std::abs(incy));
        str_name += "_" + incy_str;
        std::string alpha_str = ( alpha > 0) ? std::to_string(int(alpha)) : "m" + std::to_string(int(std::abs(alpha)));
        str_name = str_name + "_a" + alpha_str;
        std::string beta_str = ( beta > 0) ? std::to_string(int(beta)) : "m" + std::to_string(int(std::abs(beta)));
        str_name = str_name + "_b" + beta_str;
        str_name = str_name + "_" + datatype;
        return str_name;
    }
};

// Black box testing for generic and main use of caxpy.
INSTANTIATE_TEST_SUITE_P(
        Blackbox,
        saxpbyvGenericTest,
        ::testing::Combine(
            ::testing::Values('n'),                                          // n: use x, not conj(x) (since it is real)
            ::testing::Range(gtint_t(10), gtint_t(101), 10),                 // m size of vector takes values from 10 to 100 with step size of 10.
            ::testing::Values(gtint_t(1)),                                   // stride size for x
            ::testing::Values(gtint_t(1)),                                   // stride size for y
            ::testing::Values(float(2.0), float(-2.0)),                      // alpha
            ::testing::Values(float(-1.0)),                                  // beta
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::saxpbyvGenericTestPrint()
    );

#ifdef TEST_BLIS_TYPED
// Test when conjugate of x is used as an argument. This option is BLIS-api specific.
// Only test very few cases as sanity check since conj(x) = x for real types.
// We can modify the values using implementantion details.
INSTANTIATE_TEST_SUITE_P(
        ConjX,
        saxpbyvGenericTest,
        ::testing::Combine(
            ::testing::Values('c'),                                          // c: use conj(x)
            ::testing::Values(gtint_t(3), gtint_t(30), gtint_t(112)),        // m size of vector
            ::testing::Values(gtint_t(1)),                                   // stride size for x
            ::testing::Values(gtint_t(1)),                                   // stride size for y
            ::testing::Values(float(2.0)),                                   // alpha
            ::testing::Values(float(1.0)),                                   // beta
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::saxpbyvGenericTestPrint()
    );
#endif

// Test for non-unit increments.
// Only test very few cases as sanity check.
// We can modify the values using implementantion details.
INSTANTIATE_TEST_SUITE_P(
        NonUnitPositiveIncrements,
        saxpbyvGenericTest,
        ::testing::Combine(
            ::testing::Values('n'),                                          // use x, not conj(x) (since it is real)
            ::testing::Values(gtint_t(3), gtint_t(30), gtint_t(112)),        // m size of vector
            ::testing::Values(gtint_t(11)),                                  /*(gtint_t(-5), gtint_t(-17))*/// stride size for x
            ::testing::Values(gtint_t(3)),                                   /*(gtint_t(-12), gtint_t(-4))*/// stride size for y
            ::testing::Values(float(4.0)),                                   // alpha
            ::testing::Values(float(2.0)),                                   // beta
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::saxpbyvGenericTestPrint()
    );

#ifndef TEST_BLIS_TYPED
// Test for negative increments.
// Only test very few cases as sanity check.
// We can modify the values using implementantion details.
INSTANTIATE_TEST_SUITE_P(
        NegativeIncrements,
        saxpbyvGenericTest,
        ::testing::Combine(
            ::testing::Values('n'),                                          // n: use x, c: use conj(x)
            ::testing::Range(gtint_t(10), gtint_t(31), 10),                  // m size of vector takes values from 10 to 100 with step size of 10.
            ::testing::Values(gtint_t(11), gtint_t(-11)),                    // stride size for x
            ::testing::Values(gtint_t(-3), gtint_t(4)),                      // stride size for y
            ::testing::Values(4.0),                                          // alpha
            ::testing::Values(-2.0),                                         // beta
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::saxpbyvGenericTestPrint()
    );
#endif