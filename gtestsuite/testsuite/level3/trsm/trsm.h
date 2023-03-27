#pragma once

#include "blis.h"
#include "common/testing_helpers.h"

/**
 * @brief Performs the operation:
 *        B := alpha*op( A )*B,   or   B := alpha*B*op( A )
 * where  op( A ) is one of
 *        op( A ) = A   or   op( A ) = A**T   or   op( A ) = A**H,
 * @param[in]     storage specifies storage format used for the matrices
 * @param[in]     side   specifies if the symmetric matrix A appears left or right in
                         the matrix multiplication
 * @param[in]     uplo   specifies if the upper or lower triangular part of A is used
 * @param[in]     transa specifies the form of op( A ) to be used in
                         the matrix multiplication
 * @param[in]     diaga  specifies whether upper or lower triangular part of the matrix A
 * @param[in]     m      specifies the number of rows and cols of the  matrix
                         op( A ) and rows of the matrix C and B
 * @param[in]     n      specifies the number of columns of the matrix
                         op( B ) and the number of columns of the matrix C
 * @param[in]     alpha  specifies the scalar alpha.
 * @param[in]     ap     specifies pointer which points to the first element of ap
 * @param[in]     rsa    specifies row increment of ap.
 * @param[in]     csa    specifies column increment of ap.
 * @param[in,out] bp     specifies pointer which points to the first element of bp
 * @param[in]     rsb    specifies row increment of bp.
 * @param[in]     csb    specifies column increment of bp.
 */

template<typename T>
static void trsm_( char side, char uploa, char transa, char diaga, gtint_t m,
               gtint_t n, T* alpha, T* ap, gtint_t lda, T* bp, gtint_t ldb )
{
    if constexpr (std::is_same<T, float>::value)
        strsm_( &side, &uploa, &transa, &diaga, &m, &n, alpha, ap, &lda, bp, &ldb );
    else if constexpr (std::is_same<T, double>::value)
        dtrsm_( &side, &uploa, &transa, &diaga, &m, &n, alpha, ap, &lda, bp, &ldb );
    else if constexpr (std::is_same<T, scomplex>::value)
        ctrsm_( &side, &uploa, &transa, &diaga, &m, &n, alpha, ap, &lda, bp, &ldb );
    else if constexpr (std::is_same<T, dcomplex>::value)
        ztrsm_( &side, &uploa, &transa, &diaga, &m, &n, alpha, ap, &lda, bp, &ldb );
    else
        throw std::runtime_error("Error in testsuite/level3/trsm.h: Invalid typename in trsm_().");
}

template<typename T>
static void cblas_trsm( char storage, char side, char uploa, char transa,
    char diaga, gtint_t m, gtint_t n, T* alpha, T* ap, gtint_t lda,
    T* bp, gtint_t ldb )
{
    enum CBLAS_ORDER cblas_order;
    if( storage == 'c' || storage == 'C' )
        cblas_order = CblasColMajor;
    else
        cblas_order = CblasRowMajor;

    enum CBLAS_SIDE cblas_side;
    if( (side == 'l') || (side == 'L') )
        cblas_side = CblasLeft;
    else
        cblas_side = CblasRight;

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
        cblas_strsm( cblas_order, cblas_side, cblas_uploa, cblas_transa, cblas_diaga, m, n, *alpha, ap, lda, bp, ldb );
    else if constexpr (std::is_same<T, double>::value)
        cblas_dtrsm( cblas_order, cblas_side, cblas_uploa, cblas_transa, cblas_diaga, m, n, *alpha, ap, lda, bp, ldb );
    else if constexpr (std::is_same<T, scomplex>::value)
        cblas_ctrsm( cblas_order, cblas_side, cblas_uploa, cblas_transa, cblas_diaga, m, n, alpha, ap, lda, bp, ldb );
    else if constexpr (std::is_same<T, dcomplex>::value)
        cblas_ztrsm( cblas_order, cblas_side, cblas_uploa, cblas_transa, cblas_diaga, m, n, alpha, ap, lda, bp, ldb );
    else
        throw std::runtime_error("Error in testsuite/level3/trsm.h: Invalid typename in cblas_trsm().");
}

template<typename T>
static void typed_trsm( char storage, char side, char uplo, char trans,
    char diag, gtint_t m, gtint_t n, T* alpha, T* ap, gtint_t lda,
    T* bp, gtint_t ldb )
{
    side_t  sidea;
    uplo_t  uploa;
    trans_t transa;
    diag_t  diaga;

    // Map parameter characters to BLIS constants.
    testinghelpers::char_to_blis_side( side, &sidea );
    testinghelpers::char_to_blis_uplo( uplo, &uploa );
    testinghelpers::char_to_blis_trans( trans, &transa );
    testinghelpers::char_to_blis_diag( diag, &diaga );

    dim_t rsa,csa;
    dim_t rsb,csb;

    rsa=rsb=1;
    csa=csb=1;
    /* a = m x m       b = m x n  */
    if( (storage == 'c') || (storage == 'C') ) {
        csa = lda ;
        csb = ldb ;
    } else {
        rsa = lda ;
        rsb = ldb ;
    }

    if constexpr (std::is_same<T, float>::value)
        bli_strsm( sidea, uploa, transa, diaga, m, n, alpha, ap, rsa, csa, bp, rsb, csb );
    else if constexpr (std::is_same<T, double>::value)
        bli_dtrsm( sidea, uploa, transa, diaga, m, n, alpha, ap, rsa, csa, bp, rsb, csb );
    else if constexpr (std::is_same<T, scomplex>::value)
        bli_ctrsm( sidea, uploa, transa, diaga, m, n, alpha, ap, rsa, csa, bp, rsb, csb );
    else if constexpr (std::is_same<T, dcomplex>::value)
        bli_ztrsm( sidea, uploa, transa, diaga, m, n, alpha, ap, rsa, csa, bp, rsb, csb );
    else
        throw std::runtime_error("Error in testsuite/level3/trsm.h: Invalid typename in typed_trsm().");
}

template<typename T>
static void trsm( char storage, char side, char uploa, char transa, char diaga,
    gtint_t m, gtint_t n, T *alpha, T *ap, gtint_t lda, T *bp, gtint_t ldb )
{
#ifdef TEST_BLAS
    if( storage == 'c' || storage == 'C' )
        trsm_<T>( side, uploa, transa, diaga, m, n, alpha, ap, lda, bp, ldb );
    else
        throw std::runtime_error("Error in testsuite/level3/trsm.h: BLAS interface cannot be tested for row-major order.");

#elif TEST_CBLAS
    cblas_trsm<T>( storage, side, uploa, transa, diaga, m, n, alpha, ap, lda, bp, ldb );
#elif TEST_BLIS_TYPED
    typed_trsm<T>( storage, side, uploa, transa, diaga, m, n, alpha, ap, lda, bp, ldb );
#else
    throw std::runtime_error("Error in testsuite/level3/trsm.h: No interfaces are set to be tested.");
#endif
}