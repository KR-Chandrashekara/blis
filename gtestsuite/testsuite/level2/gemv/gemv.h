#pragma once

#include "blis.h"
#include "common/testing_helpers.h"

/**
 * @brief Performs the operation:
 *   y := alpha*A*x + beta*y,   or   y := alpha*A**T*x + beta*y,   or
 *   y := alpha*A**H*x + beta*y,
 * 
 * or y := beta * y + alpha * transa(A) * conjx(x) (BLIS_TYPED only)
 * 
 * @param[in]     transa specifies the form of op( A ) to be used in
                         the matrix multiplication
 * @param[in]     conjx  specifies the form of xp to be used in
                         the vector multiplication (BLIS_TYPED only)
 * @param[in]     m      specifies  the number  of rows  of the  matrix A
 * @param[in]     n      specifies the number  of columns of the matrix A
 * @param[in]     alpha  specifies the scalar alpha.
 * @param[in]     ap     specifies pointer which points to the first element of ap
 * @param[in]     lda    specifies leading dimension of the matrix.
 * @param[in]     xp     specifies pointer which points to the first element of xp
 * @param[in]     incx   specifies storage spacing between elements of xp.
 * @param[in]     beta   specifies the scalar beta.
 * @param[in,out] yp     specifies pointer which points to the first element of yp
 * @param[in]     incy   specifies storage spacing between elements of yp.
 */

template<typename T>
static void gemv_( char transa, gtint_t m, gtint_t n, T* alpha, T* ap, gtint_t lda,
  T* xp, gtint_t incx, T* beta, T* yp, gtint_t incy )
{
    if constexpr (std::is_same<T, float>::value)
        sgemv_( &transa, &m, &n, alpha, ap, &lda, xp, &incx, beta, yp, &incy );
    else if constexpr (std::is_same<T, double>::value)
        dgemv_( &transa, &m, &n, alpha, ap, &lda, xp, &incx, beta, yp, &incy );
    else if constexpr (std::is_same<T, scomplex>::value)
        cgemv_( &transa, &m, &n, alpha, ap, &lda, xp, &incx, beta, yp, &incy );
    else if constexpr (std::is_same<T, dcomplex>::value)
        zgemv_( &transa, &m, &n, alpha, ap, &lda, xp, &incx, beta, yp, &incy );
    else
        throw std::runtime_error("Error in testsuite/level2/gemv.h: Invalid typename in gemv_().");
}

template<typename T>
static void cblas_gemv( char storage, char trans, gtint_t m, gtint_t n, T* alpha,
    T* ap, gtint_t lda,  T* xp, gtint_t incx, T* beta, T* yp, gtint_t incy )
{
    enum CBLAS_ORDER cblas_order;
    if( storage == 'c' || storage == 'C' )
        cblas_order = CblasColMajor;
    else
        cblas_order = CblasRowMajor;

    enum CBLAS_TRANSPOSE cblas_trans;
    if( trans == 't' )
        cblas_trans = CblasTrans;
    else if( trans == 'c' )
        cblas_trans = CblasConjTrans;
    else
        cblas_trans = CblasNoTrans;

    if constexpr (std::is_same<T, float>::value)
        cblas_sgemv( cblas_order, cblas_trans, m, n, *alpha, ap, lda, xp, incx, *beta, yp, incy );
    else if constexpr (std::is_same<T, double>::value)
        cblas_dgemv( cblas_order, cblas_trans, m, n, *alpha, ap, lda, xp, incx, *beta, yp, incy );
    else if constexpr (std::is_same<T, scomplex>::value)
        cblas_cgemv( cblas_order, cblas_trans, m, n, alpha, ap, lda, xp, incx, beta, yp, incy );
    else if constexpr (std::is_same<T, dcomplex>::value)
        cblas_zgemv( cblas_order, cblas_trans, m, n, alpha, ap, lda, xp, incx, beta, yp, incy );
    else
        throw std::runtime_error("Error in testsuite/level2/gemv.h: Invalid typename in cblas_gemv().");
}

template<typename T>
static void typed_gemv(char storage, char trans, char conj_x,
    gtint_t m, gtint_t n, T* alpha, T* ap, gtint_t lda,
    T* xp, gtint_t incx, T* beta, T* yp, gtint_t incy)
{
    trans_t transa;
    conj_t  conjx;

    // Map parameter characters to BLIS constants.
    testinghelpers::char_to_blis_trans( trans, &transa );
    testinghelpers::char_to_blis_conj ( conj_x, &conjx );

    dim_t rsa,csa;

    rsa=csa=1;
    /* a = m x n   */
    if( (storage == 'c') || (storage == 'C') )
        csa = lda ;
    else
        rsa = lda ;

    if constexpr (std::is_same<T, float>::value)
        bli_sgemv( transa, conjx, m, n, alpha, ap, rsa, csa, xp, incx, beta, yp, incy );
    else if constexpr (std::is_same<T, double>::value)
        bli_dgemv( transa, conjx, m, n, alpha, ap, rsa, csa, xp, incx, beta, yp, incy );
    else if constexpr (std::is_same<T, scomplex>::value)
        bli_cgemv( transa, conjx, m, n, alpha, ap, rsa, csa, xp, incx, beta, yp, incy );
    else if constexpr (std::is_same<T, dcomplex>::value)
        bli_zgemv( transa, conjx, m, n, alpha, ap, rsa, csa, xp, incx, beta, yp, incy );
    else
        throw std::runtime_error("Error in testsuite/level2/gemv.h: Invalid typename in typed_gemv().");
}

template<typename T>
static void gemv( char storage, char trans, char conj_x, gtint_t m, gtint_t n,
    T* alpha, T* ap, gtint_t lda, T* xp, gtint_t incx, T* beta, T* yp, gtint_t incy )
{
#ifdef TEST_BLAS
    if( storage == 'c' || storage == 'C' )
        gemv_<T>( trans, m, n, alpha, ap, lda, xp, incx, beta, yp, incy );
    else
        throw std::runtime_error("Error in testsuite/level2/gemv.h: BLAS interface cannot be tested for row-major order.");
#elif TEST_CBLAS
    cblas_gemv<T>( storage, trans, m, n, alpha, ap, lda, xp, incx, beta, yp, incy );
#elif TEST_BLIS_TYPED
    typed_gemv<T>( storage, trans, conj_x, m, n, alpha, ap, lda, xp, incx, beta, yp, incy );
#else
    throw std::runtime_error("Error in testsuite/level2/gemv.h: No interfaces are set to be tested.");
#endif
}