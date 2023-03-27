#include <gtest/gtest.h>
#include "test_dotxv.h"

class sdotxvGenericTest :
        public ::testing::TestWithParam<std::tuple<gtint_t, char, char, gtint_t, gtint_t, float, float, char>> {};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(sdotxvGenericTest);

// Tests using random integers as vector elements.
TEST_P( sdotxvGenericTest, RandomData )
{
    using T = float;
    //----------------------------------------------------------
    // Initialize values from the parameters passed through
    // test suite instantiation (INSTANTIATE_TEST_SUITE_P).
    //----------------------------------------------------------
    // vector length:
    gtint_t n = std::get<0>(GetParam());
    // denotes whether vec x is n,c
    char conj_x = std::get<1>(GetParam());
    // denotes whether vec y is n,c
    char conj_y = std::get<2>(GetParam());
    // stride size for x:
    gtint_t incx = std::get<3>(GetParam());
    // stride size for y:
    gtint_t incy = std::get<4>(GetParam());
    // alpha
    T alpha = std::get<5>(GetParam());
    // beta
    T beta  = std::get<6>(GetParam());
    // specifies the datatype for randomgenerators
    char datatype = std::get<7>(GetParam());

    // Set the threshold for the errors:
    double thresh = n*testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call generic test body using those parameters
    //----------------------------------------------------------
    test_dotxv<T>(n, conj_x, conj_y, alpha, incx, incy, beta, thresh, datatype);
}

// Used to generate a test case with a sensible name.
// Beware that we cannot use fp numbers (e.g., 2.3) in the names,
// so we are only printing int(2.3). This should be enough for debugging purposes.
// If this poses an issue, please reach out.
class sdotxvGenericTestPrint {
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<gtint_t,char,char,gtint_t,gtint_t,float,float,char>> str) const {
        gtint_t n     = std::get<0>(str.param);
        char conjx    = std::get<1>(str.param);
        char conjy    = std::get<2>(str.param);
        gtint_t incx  = std::get<3>(str.param);
        gtint_t incy  = std::get<4>(str.param);
        float alpha   = std::get<5>(str.param);
        float beta    = std::get<6>(str.param);
        char datatype = std::get<7>(str.param);
        std::string str_name = "bli_sdotxv";
        str_name += "_" + std::to_string(n);
        str_name += "_" + std::string(&conjx, 1);
        str_name += "_" + std::string(&conjy, 1);
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

#ifdef TEST_BLIS_TYPED
// Black box testing for generic and main use of sdotxv.
INSTANTIATE_TEST_SUITE_P(
        Blackbox,
        sdotxvGenericTest,
        ::testing::Combine(
            ::testing::Range(gtint_t(10), gtint_t(101), 10),                 // m size of vector takes values from 10 to 100 with step size of 10.
            ::testing::Values('n'),                                          // n: use x, not conj(x) (since it is real)
            ::testing::Values('n'),                                          // n: use y, not conj(y) (since it is real)
            ::testing::Values(gtint_t(1)),                                   // stride size for x
            ::testing::Values(gtint_t(1)),                                   // stride size for y
            ::testing::Values(1.0, 2.0),                                     // alpha
            ::testing::Values(2.0, 3.0),                                     // beta
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::sdotxvGenericTestPrint()
    );

// Test when conjugate of x is used as an argument.
// Only test very few cases as sanity check since conj(x) = x for real types.
// We can modify the values using implementantion details.
INSTANTIATE_TEST_SUITE_P(
        ConjX,
        sdotxvGenericTest,
        ::testing::Combine(
            ::testing::Values(gtint_t(3), gtint_t(30), gtint_t(112)),        // m size of vector
            ::testing::Values('c'),                                          // c: use conj(x)
            ::testing::Values('c'),                                          // c: use conj(y)
            ::testing::Values(gtint_t(1)),                                   // stride size for x
            ::testing::Values(gtint_t(1)),                                   // stride size for y
            ::testing::Values(1.0, 2.0),                                     // alpha
            ::testing::Values(2.0, 3.0),                                     // beta
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::sdotxvGenericTestPrint()
    );

// Test for non-unit increments.
// Only test very few cases as sanity check.
// We can modify the values using implementantion details.
INSTANTIATE_TEST_SUITE_P(
        NonUnitIncrements,
        sdotxvGenericTest,
        ::testing::Combine(
            ::testing::Values(gtint_t(3), gtint_t(30), gtint_t(112)),        // m size of vector
            ::testing::Values('n'),                                          // n: use x, not conj(x) (since it is real)
            ::testing::Values('n'),                                          // n: use y, not conj(y) (since it is real)
            ::testing::Values(gtint_t(2), gtint_t(11)),                      // stride size for x
            ::testing::Values(gtint_t(3), gtint_t(33)),                      // stride size for y
            ::testing::Values(1.0, 2.0),                                     // alpha
            ::testing::Values(2.0, 3.0),                                     // beta
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::sdotxvGenericTestPrint()
    );
#endif