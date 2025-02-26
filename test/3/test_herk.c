/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin
   Copyright (C) 2018 - 2023, Advanced Micro Devices, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name of The University of Texas nor the names of its
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

#include <unistd.h>
#include "blis.h"

//#define PRINT

int main( int argc, char** argv )
{
	obj_t    a, c;
	obj_t    c_save;
	obj_t    alpha, beta;
	dim_t    m, k;
	dim_t    p;
	dim_t    p_begin, p_max, p_inc;
	int      m_input, k_input;
	ind_t    ind;
	num_t    dt, dt_real;
	char     dt_ch;
	int      r, n_repeats;
	uplo_t   uploc;
	trans_t  transa;
	f77_char f77_uploc;
	f77_char f77_transa;

	double   dtime;
	double   dtime_save;
	double   gflops;

	//bli_init();

	//bli_error_checking_level_set( BLIS_NO_ERROR_CHECKING );

	n_repeats = 3;

	dt      = DT;
	dt_real = bli_dt_proj_to_real( DT );

	ind     = IND;

	p_begin = P_BEGIN;
	p_max   = P_MAX;
	p_inc   = P_INC;

	m_input = -1;
	k_input = -1;


	// Supress compiler warnings about unused variable 'ind'.
	( void )ind;

#if 0

	cntx_t* cntx;

	ind_t ind_mod = ind;

	// Initialize a context for the current induced method and datatype.
	cntx = bli_gks_query_ind_cntx( ind_mod, dt );

	// Set k to the kc blocksize for the current datatype.
	k_input = bli_cntx_get_blksz_def_dt( dt, BLIS_KC, cntx );

#elif 1

	//k_input = 256;

#endif

	// Choose the char corresponding to the requested datatype.
	if      ( bli_is_float( dt ) )    dt_ch = 's';
	else if ( bli_is_double( dt ) )   dt_ch = 'd';
	else if ( bli_is_scomplex( dt ) ) dt_ch = 'c';
	else                              dt_ch = 'z';

	uploc  = BLIS_LOWER;
	transa = BLIS_NO_TRANSPOSE;

	bli_param_map_blis_to_netlib_uplo( uploc, &f77_uploc );
	bli_param_map_blis_to_netlib_trans( transa, &f77_transa );

	// Begin with initializing the last entry to zero so that
	// matlab allocates space for the entire array once up-front.
	for ( p = p_begin; p + p_inc <= p_max; p += p_inc ) ;

	printf( "data_%s_%cherk_%s", THR_STR, dt_ch, STR );
	printf( "( %2lu, 1:3 ) = [ %4lu %4lu %7.2f ];\n",
	        ( unsigned long )(p - p_begin)/p_inc + 1,
	        ( unsigned long )0,
	        ( unsigned long )0, 0.0 );


	//for ( p = p_begin; p <= p_max; p += p_inc )
	for ( p = p_max; p_begin <= p; p -= p_inc )
	{

		if ( m_input < 0 ) m = p / ( dim_t )abs(m_input);
		else               m =     ( dim_t )    m_input;
		if ( k_input < 0 ) k = p / ( dim_t )abs(k_input);
		else               k =     ( dim_t )    k_input;

		bli_obj_create( dt_real, 1, 1, 0, 0, &alpha );
		bli_obj_create( dt,      1, 1, 0, 0, &beta );

		if ( bli_does_trans( transa ) )
			bli_obj_create( dt, k, m, 0, 0, &a );
        else
			bli_obj_create( dt, m, k, 0, 0, &a );
		bli_obj_create( dt, m, m, 0, 0, &c );
		//bli_obj_create( dt, m, k, 2, 2*m, &a );
		//bli_obj_create( dt, k, n, 2, 2*k, &b );
		//bli_obj_create( dt, m, n, 2, 2*m, &c );
		bli_obj_create( dt, m, m, 0, 0, &c_save );

		bli_randm( &a );
		bli_randm( &c );

		bli_obj_set_struc( BLIS_HERMITIAN, &c );
		bli_obj_set_uplo( uploc, &c );

		bli_obj_set_conjtrans( transa, &a );

		bli_setsc(  (2.0/1.0), 0.0, &alpha );
		bli_setsc(  (1.0/1.0), 0.0, &beta );

		bli_copym( &c, &c_save );
	
#if 0 //def BLIS
		bli_ind_disable_all_dt( dt );
		bli_ind_enable_dt( ind, dt );
#endif

		dtime_save = DBL_MAX;

		for ( r = 0; r < n_repeats; ++r )
		{
			bli_copym( &c_save, &c );

			dtime = bli_clock();

#ifdef PRINT
			bli_printm( "a", &a, "%4.1f", "" );
			bli_printm( "c", &c, "%4.1f", "" );
#endif

#ifdef BLIS

			bli_herk( &alpha,
			          &a,
			          &beta,
			          &c );

#else

			if ( bli_is_float( dt ) )
			{
				f77_int   mm     = bli_obj_length( &c );
				f77_int   kk     = bli_obj_width_after_trans( &a );
				f77_int   lda    = bli_obj_col_stride( &a );
				f77_int   ldc    = bli_obj_col_stride( &c );
				float*    alphap = ( float* )bli_obj_buffer( &alpha );
				float*    ap     = ( float* )bli_obj_buffer( &a );
				float*    betap  = ( float* )bli_obj_buffer( &beta );
				float*    cp     = ( float* )bli_obj_buffer( &c );

				ssyrk_( &f77_uploc,
						&f77_transa,
						&mm,
						&kk,
						alphap,
						ap, &lda,
						betap,
						cp, &ldc );
			}
			else if ( bli_is_double( dt ) )
			{
				f77_int   mm     = bli_obj_length( &c );
				f77_int   kk     = bli_obj_width_after_trans( &a );
				f77_int   lda    = bli_obj_col_stride( &a );
				f77_int   ldc    = bli_obj_col_stride( &c );
				double*   alphap = ( double* )bli_obj_buffer( &alpha );
				double*   ap     = ( double* )bli_obj_buffer( &a );
				double*   betap  = ( double* )bli_obj_buffer( &beta );
				double*   cp     = ( double* )bli_obj_buffer( &c );

				dsyrk_( &f77_uploc,
						&f77_transa,
						&mm,
						&kk,
						alphap,
						ap, &lda,
						betap,
						cp, &ldc );
			}
			else if ( bli_is_scomplex( dt ) )
			{
				f77_int   mm     = bli_obj_length( &c );
				f77_int   kk     = bli_obj_width_after_trans( &a );
				f77_int   lda    = bli_obj_col_stride( &a );
				f77_int   ldc    = bli_obj_col_stride( &c );
#ifdef EIGEN
				float*    alphap = ( float*    )bli_obj_buffer( &alpha );
				float*    ap     = ( float*    )bli_obj_buffer( &a );
				float*    betap  = ( float*    )bli_obj_buffer( &beta );
				float*    cp     = ( float*    )bli_obj_buffer( &c );
#else
				float*    alphap = ( float*    )bli_obj_buffer( &alpha );
				scomplex* ap     = ( scomplex* )bli_obj_buffer( &a );
				float*    betap  = ( float*    )bli_obj_buffer( &beta );
				scomplex* cp     = ( scomplex* )bli_obj_buffer( &c );
#endif

				cherk_( &f77_uploc,
						&f77_transa,
						&mm,
						&kk,
						alphap,
						ap, &lda,
						betap,
						cp, &ldc );
			}
			else if ( bli_is_dcomplex( dt ) )
			{
				f77_int   mm     = bli_obj_length( &c );
				f77_int   kk     = bli_obj_width_after_trans( &a );
				f77_int   lda    = bli_obj_col_stride( &a );
				f77_int   ldc    = bli_obj_col_stride( &c );
#ifdef EIGEN
				double*   alphap = ( double*   )bli_obj_buffer( &alpha );
				double*   ap     = ( double*   )bli_obj_buffer( &a );
				double*   betap  = ( double*   )bli_obj_buffer( &beta );
				double*   cp     = ( double*   )bli_obj_buffer( &c );
#else
				double*   alphap = ( double*   )bli_obj_buffer( &alpha );
				dcomplex* ap     = ( dcomplex* )bli_obj_buffer( &a );
				double*   betap  = ( double*   )bli_obj_buffer( &beta );
				dcomplex* cp     = ( dcomplex* )bli_obj_buffer( &c );
#endif

				zherk_( &f77_uploc,
						&f77_transa,
						&mm,
						&kk,
						alphap,
						ap, &lda,
						betap,
						cp, &ldc );
			}
#endif

#ifdef PRINT
			bli_printm( "c after", &c, "%4.1f", "" );
			exit(1);
#endif

			dtime_save = bli_clock_min_diff( dtime_save, dtime );
		}

		gflops = ( 1.0 * m * k * m ) / ( dtime_save * 1.0e9 );

		if ( bli_is_complex( dt ) ) gflops *= 4.0;

		printf( "data_%s_%cherk_%s", THR_STR, dt_ch, STR );
		printf( "( %2lu, 1:3 ) = [ %4lu %4lu %7.2f ];\n",
		        ( unsigned long )(p - p_begin)/p_inc + 1,
		        ( unsigned long )m,
		        ( unsigned long )k, gflops );

		bli_obj_free( &alpha );
		bli_obj_free( &beta );

		bli_obj_free( &a );
		bli_obj_free( &c );
		bli_obj_free( &c_save );
	}

	//bli_finalize();

	return 0;
}

