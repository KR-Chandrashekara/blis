#pragma once

#include "blis.h"
#include "common/testing_helpers.h"

/**
 * @brief Performs the operation:
 *          A := alpha*x*y**T + A,
 *       or A := A + alpha * conjx(x) * conjy(y)^T (BLIS_TYPED only)
 * @param[in]     conjy  specifies the form of xp to be used in
                         the vector multiplication (BLIS_TYPED only)
 * @param[in]     conjy  specifies the form of yp to be used in
                         the vector multiplication (BLIS_TYPED only)
 * @param[in]     m      specifies  the number  of rows  of the  matrix A
 * @param[in]     n      specifies the number  of columns of the matrix A
 * @param[in]     alpha  specifies the scalar alpha.
 * @param[in]     xp     specifies pointer which points to the first element of xp
 * @param[in]     incx   specifies storage spacing between elements of xp.
 * @param[in]     yp     specifies pointer which points to the first element of yp
 * @param[in]     incy   specifies storage spacing between elements of yp.
 * @param[in,out] ap     specifies pointer which points to the first element of ap
 * @param[in]     lda    specifies leading dimension of the matrix.
 */

template<typename T>
static void ger_( char conjy, gtint_t m, gtint_t n, T* alpha,
    T* xp, gtint_t incx, T* yp, gtint_t incy, T* ap, gtint_t lda )
{
    if constexpr (std::is_same<T, float>::value)
        sger_( &m, &n, alpha, xp, &incx, yp, &incy, ap, &lda );
    else if constexpr (std::is_same<T, double>::value)
        dger_( &m, &n, alpha, xp, &incx, yp, &incy, ap, &lda );
    else if constexpr (std::is_same<T, scomplex>::value) {
      if( testinghelpers::chkconj( conjy ) )
        cgerc_( &m, &n, alpha, xp, &incx, yp, &incy, ap, &lda );
      else
        cgeru_( &m, &n, alpha, xp, &incx, yp, &incy, ap, &lda );
    }
    else if constexpr (std::is_same<T, dcomplex>::value) {
      if( testinghelpers::chkconj( conjy ) )
        zgerc_( &m, &n, alpha, xp, &incx, yp, &incy, ap, &lda );
      else
        zgeru_( &m, &n, alpha, xp, &incx, yp, &incy, ap, &lda );
    }
    else
        throw std::runtime_error("Error in testsuite/level2/ger.h: Invalid typename in ger_().");
}

template<typename T>
static void cblas_ger( char storage, char conjy, gtint_t m, gtint_t n,
    T* alpha, T* xp, gtint_t incx,T* yp, gtint_t incy, T* ap, gtint_t lda )
{
    enum CBLAS_ORDER cblas_order;
    if( storage == 'c' || storage == 'C' )
        cblas_order = CblasColMajor;
    else
        cblas_order = CblasRowMajor;

    if constexpr (std::is_same<T, float>::value)
        cblas_sger( cblas_order, m, n, *alpha, xp, incx, yp, incy, ap, lda );
    else if constexpr (std::is_same<T, double>::value)
        cblas_dger( cblas_order, m, n, *alpha, xp, incx, yp, incy, ap, lda );
    else if constexpr (std::is_same<T, scomplex>::value) {
      if( testinghelpers::chkconj( conjy ) )
        cblas_cgerc( cblas_order, m, n, alpha, xp, incx, yp, incy, ap, lda );
      else
        cblas_cgeru( cblas_order, m, n, alpha, xp, incx, yp, incy, ap, lda );
    }
    else if constexpr (std::is_same<T, dcomplex>::value) {
      if( testinghelpers::chkconj( conjy ) )
        cblas_zgerc( cblas_order, m, n, alpha, xp, incx, yp, incy, ap, lda );
      else
        cblas_zgeru( cblas_order, m, n, alpha, xp, incx, yp, incy, ap, lda );
    }
    else
        throw std::runtime_error("Error in testsuite/level2/ger.h: Invalid typename in cblas_ger().");
}

template<typename T>
static void typed_ger(char storage, char conj_x, char conj_y, gtint_t m, gtint_t n,
         T* alpha, T* xp, gtint_t incx, T* yp, gtint_t incy, T* ap, gtint_t lda )
{
    conj_t  conjx;
    conj_t  conjy;

    // Map parameter characters to BLIS constants.
    testinghelpers::char_to_blis_conj ( conj_x, &conjx );
    testinghelpers::char_to_blis_conj ( conj_y, &conjy );

    dim_t rsa,csa;

    rsa=csa=1;
    /* a = m x n   */
    if( (storage == 'c') || (storage == 'C') )
        csa = lda ;
    else
        rsa = lda ;

    if constexpr (std::is_same<T, float>::value)
        bli_sger( conjx, conjy, m, n, alpha, xp, incx, yp, incy, ap, rsa, csa );
    else if constexpr (std::is_same<T, double>::value)
        bli_dger( conjx, conjy, m, n, alpha, xp, incx, yp, incy, ap, rsa, csa );
    else if constexpr (std::is_same<T, scomplex>::value)
        bli_cger( conjx, conjy, m, n, alpha, xp, incx, yp, incy, ap, rsa, csa );
    else if constexpr (std::is_same<T, dcomplex>::value)
        bli_zger( conjx, conjy, m, n, alpha, xp, incx, yp, incy, ap, rsa, csa );
    else
        throw std::runtime_error("Error in testsuite/level2/ger.h: Invalid typename in typed_ger().");
}

template<typename T>
static void ger( char storage, char conjx, char conjy, gtint_t m, gtint_t n,
    T* alpha, T* xp, gtint_t incx, T* yp, gtint_t incy, T* ap, gtint_t lda )
{
#ifdef TEST_BLAS
    if( storage == 'c' || storage == 'C' )
        ger_<T>( conjy, m, n, alpha, xp, incx, yp, incy, ap, lda );
    else
        throw std::runtime_error("Error in testsuite/level2/ger.h: BLAS interface cannot be tested for row-major order.");
#elif TEST_CBLAS
    cblas_ger<T>( storage, conjy, m, n, alpha, xp, incx, yp, incy, ap, lda );
#elif TEST_BLIS_TYPED
    typed_ger<T>( storage, conjx, conjy, m, n, alpha, xp, incx, yp, incy, ap, lda );
#else
    throw std::runtime_error("Error in testsuite/level2/ger.h: No interfaces are set to be tested.");
#endif
}