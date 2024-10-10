/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2021 - 2024, Advanced Micro Devices, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name(s) of the copyright holder(s) nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "blis.h"


// Benchmark application to process aocl logs generated by BLIS library.
#ifndef DT
#define DT BLIS_DOUBLE
#endif


#define AOCL_MATRIX_INITIALISATION

//#define BLIS_ENABLE_CBLAS

/* For BLIS since logs are collected at BLAS interfaces
 * we disable cblas interfaces for this benchmark application
 */

/* #ifdef BLIS_ENABLE_CBLAS */
/* #define CBLAS */
/* #endif */

int main( int argc, char** argv )
{
    obj_t x, y;
    obj_t a, a_save;
    obj_t alpha;
    dim_t p_inc = 0; // to keep track of number of inputs
    num_t dt;
    char  dt_ch;
    int   r, n_repeats;

    double   dtime;
    double   dtime_save;
    double   gflops;

    FILE* fin  = NULL;
    FILE* fout = NULL;

#ifdef CBLAS
    char stor_scheme;
#endif	

    n_repeats = N_REPEAT;  // This macro will get from Makefile.

    dt = DT;

    if (argc < 3)
      {
        printf("Usage: ./test_ger_XX.x input.csv output.csv\n");
        exit(1);
      }
    fin = fopen(argv[1], "r");
    if (fin == NULL)
      {
        printf("Error opening the file %s\n", argv[1]);
        exit(1);
      }
    fout = fopen(argv[2], "w");
    if (fout == NULL)
      {
        printf("Error opening output file %s\n", argv[2]);
        exit(1);
      }

    fprintf(fout, "Func Dt m n alphaR alphaI incx incy lda gflops\n");

    dim_t m;
    dim_t n;
    double alpha_r, alpha_i;
    inc_t lda;
    inc_t incx;
    inc_t incy;
    char tmp[256]; // to store function name, line no present in logs.

#ifdef CBLAS
    stor_scheme = 'C';
#endif

    // {S,D,C,Z} {transa m n alpha incx incy lda}
    while (fscanf(fin, "%s %c " INT_FS INT_FS " %lf %lf " INT_FS INT_FS INT_FS "\n",
        tmp, &dt_ch, &m, &n,  &alpha_r, &alpha_i, &incx, &incy, &lda) == 9)
      {

#ifdef PRINT
        fprintf (stdout, "Input = %s %c %ld %ld %lf %lf %ld %ld %ld %6.3f\n",
                 tmp, dt_ch, m, n, alpha_r, alpha_i, incx, incy, lda, gflops);
#endif

        if (dt_ch == 'D' || dt_ch == 'd') dt = BLIS_DOUBLE;
        else if (dt_ch == 'Z' || dt_ch == 'z') dt = BLIS_DCOMPLEX;
        else if (dt_ch == 'S' || dt_ch == 's') dt = BLIS_FLOAT;
        else if (dt_ch == 'C' || dt_ch == 'c') dt = BLIS_SCOMPLEX;
        else
          {
            printf("Invalid data type %c\n", dt_ch);
            continue;
          }

        // Create objects with required sizes and strides.

        // ger operation is defined as
        //
        // The ?ger routines perform a matrix-vector operation defined as
        //
        //      A := alpha*x*y'+ A,
        //
        //  where:
        //      alpha is a scalar,
        //      x is an m-element vector,
        //      y is an n-element vector,
        //      A is an m-by-n general matrix.


        bli_obj_create( dt, m, 1, incx, 1, &x );
        bli_obj_create( dt, n, 1, incy, 1, &y );

        bli_obj_create( dt, m, n, 1, lda, &a );
        bli_obj_create( dt, m, n, 1, lda, &a_save );

#ifdef AOCL_MATRIX_INITIALISATION
        bli_randm( &a );
        bli_randm( &x );
        bli_randm( &y );
#endif

        bli_obj_create( dt, 1, 1, 0, 0, &alpha );
        bli_setsc( alpha_r, alpha_i, &alpha );

        bli_copym( &a, &a_save );

        dtime_save = DBL_MAX;

        for ( r = 0; r < n_repeats; ++r )
          {
            bli_copym( &a_save, &a );

#ifdef PRINT
            bli_printm( "a", &a, "%4.1f", "" );
            bli_printm( "x", &x, "%4.1f", "" );
            bli_printm( "y", &y, "%4.1f", "" );
#endif
            dtime = bli_clock();

#ifdef BLIS
            bli_ger( &alpha,
                      &x,
                      &y,
                      &a );

#else // BLIS Interface

#ifdef CBLAS
            enum CBLAS_ORDER     cblas_order;

            if ( ( stor_scheme == 'C' ) || ( stor_scheme == 'c' ) )
              cblas_order = CblasColMajor;
            else
              cblas_order = CblasRowMajor;
#endif
            // Set data type independent inputs for BLAS and
            // CBLAS API's

            f77_int  mm     = bli_obj_length( &a );
            f77_int  nn     = bli_obj_width( &a );
            f77_int  blas_lda    = bli_obj_col_stride( &a );
            f77_int  blas_incx   = incx;
            f77_int  blas_incy   = incy;

            if ( bli_is_float( dt ) ){
                float*  alphap = bli_obj_buffer( &alpha );
                float*  ap     = bli_obj_buffer( &a );
                float*  xp     = bli_obj_buffer( &x );
                float*  yp     = bli_obj_buffer( &y );
#ifdef CBLAS
                cblas_sger(cblas_order,
                            mm,
                            nn,
                            *alphap,
                            xp, blas_incx,
                            yp, blas_incy,
                            ap, blas_lda);
#else // cblas sgemv
                sger_( &mm,
                       &nn,
                       alphap,
                       xp, &blas_incx,
                       yp, &blas_incy,
                       ap, &blas_lda );
#endif // cblas sgemv
            }
            else if ( bli_is_double( dt ) )
            {

                double*  alphap = bli_obj_buffer( &alpha );
                double*  ap     = bli_obj_buffer( &a );
                double*  xp     = bli_obj_buffer( &x );
                double*  yp     = bli_obj_buffer( &y );
#ifdef CBLAS
                cblas_dger(cblas_order,
                            mm,
                            nn,
                            *alphap,
                            xp, blas_incx,
                            yp, blas_incy,
                            ap, blas_lda);
#else // cblas dgemv
                dger_( &mm,
                       &nn,
                       alphap,
                       xp, &blas_incx,
                       yp, &blas_incy,
                       ap, &blas_lda );
#endif // cblas dgemv
            }
            else if ( bli_is_scomplex( dt ) )
            {
                scomplex*  alphap = bli_obj_buffer( &alpha );
                scomplex*  ap     = bli_obj_buffer( &a );
                scomplex*  xp     = bli_obj_buffer( &x );
                scomplex*  yp     = bli_obj_buffer( &y );
#ifdef CBLAS
                cblas_cgeru(cblas_order,
                            mm,
                            nn,
                            alphap,
                            xp, blas_incx,
                            yp, blas_incy,
                            ap, blas_lda);
#else // cblas cgemv
                cgeru_( &mm,
                       &nn,
                       alphap,
                       xp, &blas_incx,
                       yp, &blas_incy,
                       ap, &blas_lda );
#endif // cblas cgemv
            }
            else if ( bli_is_dcomplex( dt ) )
            {
                dcomplex*  alphap = bli_obj_buffer( &alpha );
                dcomplex*  ap     = bli_obj_buffer( &a );
                dcomplex*  xp     = bli_obj_buffer( &x );
                dcomplex*  yp     = bli_obj_buffer( &y );
#ifdef CBLAS
                cblas_zgeru(cblas_order,
                            mm,
                            nn,
                            alphap,
                            xp, blas_incx,
                            yp, blas_incy,
                            ap, blas_lda);
#else // cblas zgemv
                zgeru_( &mm,
                       &nn,
                       alphap,
                       xp, &blas_incx,
                       yp, &blas_incy,
                       ap, &blas_lda );
#endif  // cblas zgemv
            }

#endif // BLIS Interface

#ifdef PRINT
            bli_printm( "a after", &a, "%4.1f", "" );
            exit(1);
#endif

            dtime_save = bli_clock_min_diff( dtime_save, dtime );
          }

        gflops = ( 2.0 * m  * n ) / ( dtime_save * 1.0e9 );

        if ( bli_is_complex( dt ) ) gflops *= 4.0;

        printf( "data_ger_%s", BLAS );

        p_inc++;
        printf("( %2lu, 1:4 ) = [ %4lu %4lu %7.2f ];\n",
               (unsigned long)(p_inc),
               (unsigned long)m,
               (unsigned long)n,
                gflops);

        fprintf (fout, "%s %c %ld %ld %lf %lf %ld %ld %ld %6.3f\n",
                 tmp, dt_ch, m, n, alpha_r, alpha_i, incx, incy, lda, gflops);

        fflush(fout);

        bli_obj_free( &alpha );

        bli_obj_free( &x );
        bli_obj_free( &y );
        bli_obj_free( &a );
        bli_obj_free( &a_save );
      }

    //bli_finalize();
    fclose(fin);
    fclose(fout);

    return 0;
}
