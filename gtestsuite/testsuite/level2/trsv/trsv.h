#pragma once

#include "blis.h"
#include "common/testing_helpers.h"

/**
 * @brief Performs the operation:
  *    x := alpha * inv(transa(A)) * x_orig
 * @param[in]     storage specifies the form of storage in the memory matrix A
 * @param[in]     uploa  specifies whether the upper or lower triangular part of the array A
 * @param[in]     transa specifies the form of op( A ) to be used in matrix multiplication
 * @param[in]     diaga  specifies whether the upper or lower triangular part of the array A
 * @param[in]     n      specifies the number  of rows  of the  matrix A
 * @param[in]     alpha  specifies the scalar alpha.
 * @param[in]     ap     specifies pointer which points to the first element of ap
 * @param[in]     lda    specifies leading dimension of the matrix.
 * @param[in,out] xp     specifies pointer which points to the first element of xp
 * @param[in]     incx   specifies storage spacing between elements of xp.

 */

template<typename T>
static void trsv_( char uploa, char transa, char diaga, gtint_t n,
                         T *ap, gtint_t lda, T *xp, gtint_t incx )
{
    if constexpr (std::is_same<T, float>::value)
        strsv_( &uploa, &transa, &diaga, &n, ap, &lda, xp, &incx );
    else if constexpr (std::is_same<T, double>::value)
        dtrsv_( &uploa, &transa, &diaga, &n, ap, &lda, xp, &incx );
    else if constexpr (std::is_same<T, scomplex>::value)
        ctrsv_( &uploa, &transa, &diaga, &n, ap, &lda, xp, &incx );
    else if constexpr (std::is_same<T, dcomplex>::value)
        ztrsv_( &uploa, &transa, &diaga, &n, ap, &lda, xp, &incx );
    else
        throw std::runtime_error("Error in testsuite/level2/trsv.h: Invalid typename in trsv_().");
}

template<typename T>
static void cblas_trsv( char storage, char uploa, char transa, char diaga,
                      gtint_t n, T *ap, gtint_t lda, T *xp, gtint_t incx )
{

    enum CBLAS_ORDER cblas_order;
    if( storage == 'c' || storage == 'C' )
        cblas_order = CblasColMajor;
    else
        cblas_order = CblasRowMajor;

    enum CBLAS_UPLO cblas_uploa;
    if( (uploa == 'u') || (uploa == 'U') )
        cblas_uploa = CblasUpper;
    else
        cblas_uploa = CblasLower;

    enum CBLAS_TRANSPOSE cblas_transa;
    if( transa == 't' )
        cblas_transa = CblasTrans;
    else if( transa == 'c' )
        cblas_transa = CblasConjTrans;
    else
        cblas_transa = CblasNoTrans;

    enum CBLAS_DIAG cblas_diaga;
    if( (diaga == 'u') || (diaga == 'U') )
        cblas_diaga = CblasUnit;
    else
        cblas_diaga = CblasNonUnit;

    if constexpr (std::is_same<T, float>::value)
        cblas_strsv( cblas_order, cblas_uploa, cblas_transa, cblas_diaga, n, ap, lda, xp, incx );
    else if constexpr (std::is_same<T, double>::value)
        cblas_dtrsv( cblas_order, cblas_uploa, cblas_transa, cblas_diaga, n, ap, lda, xp, incx );
    else if constexpr (std::is_same<T, scomplex>::value)
        cblas_ctrsv( cblas_order, cblas_uploa, cblas_transa, cblas_diaga, n, ap, lda, xp, incx );
    else if constexpr (std::is_same<T, dcomplex>::value)
        cblas_ztrsv( cblas_order, cblas_uploa, cblas_transa, cblas_diaga, n, ap, lda, xp, incx );
    else
        throw std::runtime_error("Error in testsuite/level2/trsv.h: Invalid typename in cblas_trsv().");
}

template<typename T>
static void typed_trsv( char storage, char uplo, char trans, char diag,
            gtint_t n, T *alpha, T *ap, gtint_t lda, T *xp, gtint_t incx )
{
    uplo_t  uploa;
    trans_t transa;
    diag_t  diaga;

    // Map parameter characters to BLIS constants.
    testinghelpers::char_to_blis_uplo ( uplo, &uploa );
    testinghelpers::char_to_blis_trans( trans, &transa );
    testinghelpers::char_to_blis_diag ( diag, &diaga );

    dim_t rsa,csa;
    rsa=csa=1;
    /* a = n x n   */
    if( (storage == 'c') || (storage == 'C') )
        csa = lda ;
    else
        rsa = lda ;

    if constexpr (std::is_same<T, float>::value)
        bli_strsv( uploa, transa, diaga, n, alpha, ap, rsa, csa, xp, incx );
    else if constexpr (std::is_same<T, double>::value)
        bli_dtrsv( uploa, transa, diaga, n, alpha, ap, rsa, csa, xp, incx );
    else if constexpr (std::is_same<T, scomplex>::value)
        bli_ctrsv( uploa, transa, diaga, n, alpha, ap, rsa, csa, xp, incx );
    else if constexpr (std::is_same<T, dcomplex>::value)
        bli_ztrsv( uploa, transa, diaga, n, alpha, ap, rsa, csa, xp, incx );
    else

        throw std::runtime_error("Error in testsuite/level2/trsv.h: Invalid typename in typed_trsv().");
}

template<typename T>
static void trsv( char storage, char uploa, char transa, char diaga,
    gtint_t n, T *alpha, T *ap, gtint_t lda, T *xp, gtint_t incx )
{
#if (defined TEST_BLAS || defined  TEST_CBLAS)
    T one;
    testinghelpers::initone(one);
#endif

#ifdef TEST_BLAS
    if(( storage == 'c' || storage == 'C' ))
        if( *alpha == one )
            trsv_<T>( uploa, transa, diaga, n, ap, lda, xp, incx );
        else
            throw std::runtime_error("Error in testsuite/level2/trsv.h: BLAS interface cannot be tested for alpha != one.");
    else
        throw std::runtime_error("Error in testsuite/level2/trsv.h: BLAS interface cannot be tested for row-major order.");
#elif TEST_CBLAS
    if( *alpha == one )
        cblas_trsv<T>( storage, uploa, transa, diaga, n, ap, lda, xp, incx );
    else
      throw std::runtime_error("Error in testsuite/level2/trsv.h: CBLAS interface cannot be tested for alpha != one.");
#elif TEST_BLIS_TYPED
    typed_trsv<T>( storage, uploa, transa, diaga, n, alpha, ap, lda, xp, incx );
#else
    throw std::runtime_error("Error in testsuite/level2/trsv.h: No interfaces are set to be tested.");
#endif
}