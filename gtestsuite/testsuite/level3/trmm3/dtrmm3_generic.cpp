#include <gtest/gtest.h>
#include "test_trmm3.h"

class dtrmm3Test :
        public ::testing::TestWithParam<std::tuple<char,
                                                   char,
                                                   char,
                                                   char,
                                                   char,
                                                   char,
                                                   gtint_t,
                                                   gtint_t,
                                                   double,
                                                   double,
                                                   gtint_t,
                                                   gtint_t,
                                                   gtint_t,
                                                   char>> {};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(dtrmm3Test);

TEST_P(dtrmm3Test, RandomData) {
    using T = double;
    //----------------------------------------------------------
    // Initialize values from the parameters passed through
    // test suite instantiation (INSTANTIATE_TEST_SUITE_P).
    //----------------------------------------------------------
    // matrix storage format(row major, column major)
    char storage = std::get<0>(GetParam());
    // specifies matrix A appears left or right in
    // the matrix multiplication
    char side = std::get<1>(GetParam());
    // specifies upper or lower triangular part of A is used
    char uploa = std::get<2>(GetParam());
    // denotes whether matrix a is n,c,t,h
    char transa = std::get<3>(GetParam());
    // denotes whether matrix b is n,c,t,h
    char transb = std::get<4>(GetParam());
    // denotes whether matrix a in unit or non-unit diagonal
    char diaga = std::get<5>(GetParam());
    // matrix size m
    gtint_t m  = std::get<6>(GetParam());
    // matrix size n
    gtint_t n  = std::get<7>(GetParam());
    // specifies alpha value
    T alpha = std::get<8>(GetParam());
    // specifies alpha value
    T beta = std::get<9>(GetParam());
    // lda, ldb, ldc increments.
    // If increments are zero, then the array size matches the matrix size.
    // If increments are nonnegative, the array size is bigger than the matrix size.
    gtint_t lda_inc = std::get<10>(GetParam());
    gtint_t ldb_inc = std::get<11>(GetParam());
    gtint_t ldc_inc = std::get<12>(GetParam());
    // specifies the datatype for randomgenerators
    char datatype   = std::get<13>(GetParam());

    // Set the threshold for the errors:
    double thresh = m*n*testinghelpers::getEpsilon<T>();

    //----------------------------------------------------------
    //     Call test body using these parameters
    //----------------------------------------------------------
    test_trmm3<T>( storage, side, uploa, transa, diaga, transb, m, n, alpha, lda_inc, ldb_inc, beta, ldc_inc, thresh, datatype );
}

class dtrmm3TestPrint {
public:
    std::string operator()(
        testing::TestParamInfo<std::tuple<char, char, char, char, char, char, gtint_t, gtint_t, double, double, gtint_t, gtint_t, gtint_t, char>> str) const {
        char sfm        = std::get<0>(str.param);
        char side       = std::get<1>(str.param);
        char uploa      = std::get<2>(str.param);
        char transa     = std::get<3>(str.param);
        char transb     = std::get<4>(str.param);
        char diaga      = std::get<5>(str.param);
        gtint_t m       = std::get<6>(str.param);
        gtint_t n       = std::get<7>(str.param);
        double alpha    = std::get<8>(str.param);
        double beta     = std::get<9>(str.param);
        gtint_t lda_inc = std::get<10>(str.param);
        gtint_t ldb_inc = std::get<11>(str.param);
        gtint_t ldc_inc = std::get<12>(str.param);
        char datatype   = std::get<13>(str.param);
        std::string str_name = "blis_dtrmm3";
        str_name = str_name + "_" + sfm+sfm+sfm;
        str_name = str_name + "_" + side + uploa + transa + transb;
        str_name = str_name + "_d" + diaga;
        str_name = str_name + "_" + std::to_string(m);
        str_name = str_name + "_" + std::to_string(n);
        std::string alpha_str = ( alpha > 0) ? std::to_string(int(alpha)) : "m" + std::to_string(int(std::abs(alpha)));
        std::string beta_str = ( beta > 0) ? std::to_string(int(beta)) : "m" + std::to_string(int(std::abs(beta)));
        str_name = str_name + "_a" + alpha_str;
        str_name = str_name + "_b" + beta_str;
        str_name = str_name + "_" + std::to_string(lda_inc);
        str_name = str_name + "_" + std::to_string(ldb_inc);
        str_name = str_name + "_" + std::to_string(ldc_inc);
        str_name = str_name + "_" + datatype;
        return str_name;
    }
};

#ifdef TEST_BLIS_TYPED
// Black box testing.
INSTANTIATE_TEST_SUITE_P(
        Blackbox,
        dtrmm3Test,
        ::testing::Combine(
            ::testing::Values('c','r'),                                      // storage format
            ::testing::Values('l','r'),                                      // side  l:left, r:right
            ::testing::Values('u','l'),                                      // uplo  u:upper, l:lower
            ::testing::Values('n','t'),                                      // transa
            ::testing::Values('n'),                                          // transb  /*transb works only for 'n' case*/
            ::testing::Values('n','u'),                                      // diaga , n=nonunit u=unit
            ::testing::Range(gtint_t(10), gtint_t(31), 10),                  // m
            ::testing::Range(gtint_t(10), gtint_t(31), 10),                  // n
            ::testing::Values( 1.0, -2.0),                                   // alpha
            ::testing::Values(-1.0,  2.0),                                   // beta
            ::testing::Values(gtint_t(0)),                                   // increment to the leading dim of a
            ::testing::Values(gtint_t(0)),                                   // increment to the leading dim of b
            ::testing::Values(gtint_t(0)),                                   // increment to the leading dim of c
            ::testing::Values(ELEMENT_TYPE)                                  // i : integer, f : float  datatype type tested
        ),
        ::dtrmm3TestPrint()
    );
#endif