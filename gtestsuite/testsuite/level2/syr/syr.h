#pragma once

#include "blis.h"
#include "common/testing_helpers.h"

/**
 * @brief Performs the operation:
 *          A := alpha*x*x**T + A
 * @param[in]     storage specifies the form of storage in the memory matrix A
 * @param[in]     uploa  specifies whether the upper or lower triangular part of the array A
 * @param[in]     m      specifies the number  of rows  of the  matrix A
 * @param[in]     alpha  specifies the scalar alpha.
 * @param[in]     xp     specifies pointer which points to the first element of xp
 * @param[in]     incx   specifies storage spacing between elements of xp.
 * @param[in,out] ap     specifies pointer which points to the first element of ap
 * @param[in]     lda    specifies leading dimension of the matrix.
 */


template<typename T>
static void syr_( char uploa, gtint_t n, T* alpha, T* xp, gtint_t incx,
                                                  T* ap, gtint_t lda )
{
    if constexpr (std::is_same<T, float>::value)
        ssyr_( &uploa, &n, alpha, xp, &incx, ap, &lda );
    else if constexpr (std::is_same<T, double>::value)
        dsyr_( &uploa, &n, alpha, xp, &incx, ap, &lda );
    else
        throw std::runtime_error("Error in testsuite/level2/syr.h: Invalid typename in syr_().");
}

template<typename T>
static void cblas_syr( char storage, char uploa, gtint_t n, T* alpha,
                            T* xp, gtint_t incx, T* ap, gtint_t lda )
{
    enum CBLAS_ORDER cblas_order;
    if( storage == 'c' || storage == 'C' )
        cblas_order = CblasColMajor;
    else
        cblas_order = CblasRowMajor;

    enum CBLAS_UPLO cblas_uplo;
    if( (uploa == 'u') || (uploa == 'U') )
        cblas_uplo = CblasUpper;
    else
        cblas_uplo = CblasLower;

    if constexpr (std::is_same<T, float>::value)
        cblas_ssyr( cblas_order, cblas_uplo, n, *alpha, xp, incx, ap, lda );
    else if constexpr (std::is_same<T, double>::value)
        cblas_dsyr( cblas_order, cblas_uplo, n, *alpha, xp, incx, ap, lda );
    else
        throw std::runtime_error("Error in testsuite/level2/syr.h: Invalid typename in cblas_syr().");
}

template<typename T>
static void typed_syr( char storage, char uplo, char conj_x, gtint_t n,
                    T* alpha, T* x, gtint_t incx, T* a, gtint_t lda )
{
    uplo_t uploa;
    conj_t conjx;

    // Map parameter characters to BLIS constants.
    testinghelpers::char_to_blis_uplo ( uplo, &uploa );
    testinghelpers::char_to_blis_conj ( conj_x, &conjx );

    dim_t rsa,csa;

    rsa=csa=1;
    /* a = n x n   */
    if( (storage == 'c') || (storage == 'C') )
        csa = lda ;
    else
        rsa = lda ;

    if constexpr (std::is_same<T, float>::value)
        bli_ssyr( uploa, conjx, n, alpha, x, incx, a, rsa, csa );
    else if constexpr (std::is_same<T, double>::value)
        bli_dsyr( uploa, conjx, n, alpha, x, incx, a, rsa, csa );
    else
        throw std::runtime_error("Error in testsuite/level2/syr.h: Invalid typename in typed_syr().");
}

template<typename T>
static void syr( char storage, char uploa, char conj_x, gtint_t n, T* alpha,
                               T* xp, gtint_t incx, T* ap, gtint_t lda )
{
#ifdef TEST_BLAS
    if( storage == 'c' || storage == 'C' )
        syr_<T>( uploa, n, alpha, xp, incx, ap, lda );
    else
        throw std::runtime_error("Error in testsuite/level2/syr.h: BLAS interface cannot be tested for row-major order.");
#elif TEST_CBLAS
    cblas_syr<T>( storage, uploa, n, alpha, xp, incx, ap, lda );
#elif TEST_BLIS_TYPED
    typed_syr<T>( storage, uploa, conj_x, n, alpha, xp, incx, ap, lda );
#else
    throw std::runtime_error("Error in testsuite/level2/syr.h: No interfaces are set to be tested.");
#endif
}