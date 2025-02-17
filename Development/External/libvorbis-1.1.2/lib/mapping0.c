/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: channel mapping 0 implementation
 last mod: $Id: mapping0.c 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"
#include "codebook.h"
#include "registry.h"
#include "psy.h"
#include "misc.h"
#ifdef __SSE__												/* SSE Optimize */
#include <float.h>
#include "xmmlib.h"
#endif														/* SSE Optimize */

/* + 0.345 is a hack; the original todB	estimation used on IEEE 754 compliant machines had a bug that returned dB values about a third
   of a decibel too high.  The bug was harmless because tunings implicitly took that into account.  However, fixing the bug in the estimator requires
   changing all the tunings as well. For now, it's easier to sync things back up here, and recalibrate the tunings in the next major model upgrade. */

/* simplistic, wasteful way of doing this (unique lookup for each mode/submapping); there should be a central repository for
   identical lookups.  That will require minor work, so I'm putting it off as low priority.

   Why a lookup for each backend in a given mode?  Because the blocksize is set by the mode, and low backend lookups may require
   parameters from other areas of the mode/mapping */

static void mapping0_free_info( vorbis_info_mapping* i )
{
	vorbis_info_mapping0* info = ( vorbis_info_mapping0* )i;
	if( info )
	{
		_ogg_free( info );
	}
}

static void mapping0_pack( vorbis_info* vi, vorbis_info_mapping* vm, oggpack_buffer* opb )
{
	int i;
	vorbis_info_mapping0* info = ( vorbis_info_mapping0* )vm;

  /* another 'we meant to do it this way' hack...  up to beta 4, we packed 4 binary zeros here to signify one submapping in use.  We
     now redefine that to mean four bitflags that indicate use of deeper features; bit0:submappings, bit1:coupling,
     bit2,3:reserved. This is backward compatable with all actual uses of the beta code. */
	if( info->submaps > 1 )
	{
		oggpack_write( opb, 1, 1 );
		oggpack_write( opb, info->submaps - 1, 4 );
	}
	else
	{
		oggpack_write( opb, 0, 1 );
	}

	if( info->coupling_steps > 0 )
	{
		oggpack_write( opb, 1, 1 );
		oggpack_write( opb, info->coupling_steps - 1, 8 );

		for( i = 0; i < info->coupling_steps; i++ )
		{
			oggpack_write( opb, info->coupling_mag[i], ilog2( vi->channels ) );
			oggpack_write( opb, info->coupling_ang[i], ilog2( vi->channels ) );
		}
	}
	else
	{
		oggpack_write( opb, 0, 1 );
	}
  
	/* 2,3:reserved */
	oggpack_write( opb, 0, 2 ); 

	/* we don't write the channel submappings if we only have one... */
	if( info->submaps > 1 )
	{
		for( i = 0; i < vi->channels; i++ )
		{
			oggpack_write( opb, info->chmuxlist[i], 4 );
		}
	}

	for( i = 0; i < info->submaps; i++ )
	{
		/* time submap unused */
		oggpack_write( opb, 0, 8 ); 
		oggpack_write( opb, info->floorsubmap[i], 8 );
		oggpack_write( opb, info->residuesubmap[i], 8 );
	}
}

/* also responsible for range checking */
static vorbis_info_mapping* mapping0_unpack( vorbis_info* vi, oggpack_buffer* opb )
{
	int i;
	vorbis_info_mapping0* info = ( vorbis_info_mapping0* )_ogg_calloc( 1, sizeof( *info ) );
	codec_setup_info* ci = vi->codec_setup;
	memset( info, 0, sizeof( *info ) );

	if( oggpack_read( opb, 1 ) )
	{
		info->submaps = oggpack_read( opb, 4 ) + 1;
	}
	else
	{
		info->submaps = 1;
	}

	if( oggpack_read( opb, 1 ) )
	{
		info->coupling_steps = oggpack_read( opb, 8 ) + 1;

		for( i = 0; i < info->coupling_steps; i++ )
		{
			int testM = info->coupling_mag[i] = oggpack_read( opb, ilog2( vi->channels ) );
			int testA = info->coupling_ang[i] = oggpack_read( opb, ilog2( vi->channels ) );

			if( testM < 0 || testA < 0 || testM == testA || testM >= vi->channels || testA >= vi->channels )
			{
				goto err_out; 
			}
		}
	}

	if( oggpack_read( opb, 2 ) > 0 )
	{
		/* 2,3:reserved */
		goto err_out;
	}

	if( info->submaps > 1 )
	{
		for( i = 0; i < vi->channels; i++ )
		{
			info->chmuxlist[i] = oggpack_read( opb, 4 );
			if( info->chmuxlist[i] >= info->submaps )
			{
				goto err_out;
			}
		}
	}

	for( i = 0; i < info->submaps; i++ )
	{
		/* time submap unused */
		oggpack_read( opb, 8 ); 
		info->floorsubmap[i] = oggpack_read( opb, 8 );
		if( info->floorsubmap[i] >= ci->floors )
		{
			goto err_out;
		}

		info->residuesubmap[i] = oggpack_read( opb, 8 );
		if( info->residuesubmap[i] >= ci->residues )
		{
			goto err_out;
		}
	}

	return( info );

err_out:
	mapping0_free_info( info );
	return( NULL );
}

#include "os.h"
#include "lpc.h"
#include "lsp.h"
#include "envelope.h"
#include "mdct.h"
#include "psy.h"
#include "scales.h"

#ifdef __SSE__												/* SSE Optimize */
static void mapping_forward_sub0( float* pcm, float* logfft, float scale_dB, float* local_ampmax, int i, int n )
{
	const __m128 mparm = { 7.17711438e-7f / 2.0f, 7.17711438e-7f / 2.0f, 7.17711438e-7f / 2.0f, 7.17711438e-7f / 2.0f };

	__m128	SCALEdB, LAM0;
#if	!defined(__SSE2__)
	__m128	LAM1;
#endif
	int	j, k;
	SCALEdB = _mm_set_ps1( scale_dB + 0.345f - 764.6161886f / 2.0f );
	LAM0 = _mm_set_ps1( local_ampmax[i] );
#if	defined(__SSE2__)
	if( n >= 256 && n <= 4096 )
	{
		/* Caution! This routhine is for SSE optimized fft only. */
		float rfv = logfft[0];
		logfft[0] = 0.0f;
		logfft[1] = 0.0f;
#if	defined(__SSE3__)
		/* SSE3 optimized code */
		for( j = 0, k = 0; j < n; j += 16, k += 8 )
		{
			__m128	XMM0, XMM2, XMM1, XMM3;

			XMM0 = _mm_load_ps( pcm + j );
			XMM1 = _mm_load_ps( pcm + j + 4 );
			XMM2 = _mm_load_ps( pcm + j + 8 );
			XMM3 = _mm_load_ps( pcm + j + 12 );
			XMM0 = _mm_mul_ps( XMM0, XMM0 );
			XMM1 = _mm_mul_ps( XMM1, XMM1 );
			XMM2 = _mm_mul_ps( XMM2, XMM2 );
			XMM3 = _mm_mul_ps( XMM3, XMM3 );
			XMM0 = _mm_hadd_ps( XMM0, XMM1 );
			XMM2 = _mm_hadd_ps( XMM2, XMM3 );
			XMM0 = _mm_cvtepi32_ps( _mm_castps_si128( XMM0 ) );
			XMM2 = _mm_cvtepi32_ps( _mm_castps_si128( XMM2 ) );
			XMM0 = _mm_mul_ps( XMM0, mparm );
			XMM2 = _mm_mul_ps( XMM2, mparm );
			XMM0 = _mm_add_ps( XMM0, SCALEdB );
			XMM2 = _mm_add_ps( XMM2, SCALEdB );
			_mm_store_ps( logfft + k, XMM0 );
			_mm_store_ps( logfft + k + 4, XMM2 );
			XMM0 = _mm_max_ps( XMM0, XMM2 );
			LAM0 = _mm_max_ps( LAM0, XMM0 );
		}
#else
		/* SSE2 optimized code */
		for( j = 0, k = 0; j < n; j += 16, k += 8 )
		{
			__m128	XMM0, XMM2, XMM1, XMM3, XMM4, XMM5;

			XMM0 = _mm_load_ps( pcm + j );
			XMM2 = _mm_load_ps( pcm + j + 8 );
			XMM4 = _mm_load_ps( pcm + j + 4 );
			XMM5 = _mm_load_ps( pcm + j + 12 );
			XMM1 = XMM0;
			XMM3 = XMM2;
			XMM0 = _mm_shuffle_ps( XMM0, XMM4, _MM_SHUFFLE( 2, 0, 2, 0 ) );
			XMM1 = _mm_shuffle_ps( XMM1, XMM4, _MM_SHUFFLE( 3, 1, 3, 1 ) );
			XMM2 = _mm_shuffle_ps( XMM2, XMM5, _MM_SHUFFLE( 2, 0, 2, 0 ) );
			XMM3 = _mm_shuffle_ps( XMM3, XMM5, _MM_SHUFFLE( 3, 1, 3, 1 ) );
			XMM0 = _mm_mul_ps( XMM0, XMM0 );
			XMM1 = _mm_mul_ps( XMM1, XMM1 );
			XMM2 = _mm_mul_ps( XMM2, XMM2 );
			XMM3 = _mm_mul_ps( XMM3, XMM3 );
			XMM0 = _mm_add_ps( XMM0, XMM1 );
			XMM2 = _mm_add_ps( XMM2, XMM3 );
			XMM0 = _mm_cvtepi32_ps( _mm_castps_si128( XMM0 ) );
			XMM2 = _mm_cvtepi32_ps( _mm_castps_si128( XMM2 ) );
			XMM0 = _mm_mul_ps( XMM0, mparm );
			XMM2 = _mm_mul_ps( XMM2, mparm );
			XMM0 = _mm_add_ps( XMM0, SCALEdB );
			XMM2 = _mm_add_ps( XMM2, SCALEdB );
			_mm_store_ps( logfft + k, XMM0 );
			_mm_store_ps( logfft + k + 4, XMM2 );
			XMM0 = _mm_max_ps( XMM0, XMM2 );
			LAM0 = _mm_max_ps( LAM0, XMM0 );
		}
#endif
		local_ampmax[i] = _mm_max_horz( LAM0 );
		logfft[0] = rfv;
	}
	else
	{
		/* SSE2 optimized code */
		int Cnt = ( ( n - 2 ) & ( ~15 ) ) + 1;
		for( j = 1; j < Cnt; j += 16 )
		{
		__m128	XMM0, XMM3;
#if	defined(__SSE3__)
			{
				__m128	XMM2, XMM5;

				XMM0 = _mm_lddqu_ps( pcm + j );
				XMM2 = _mm_lddqu_ps( pcm + j + 4 );
				XMM3 = _mm_lddqu_ps( pcm + j + 8 );
				XMM5 = _mm_lddqu_ps( pcm + j + 12 );
				XMM0 = _mm_mul_ps( XMM0, XMM0 );
				XMM2 = _mm_mul_ps( XMM2, XMM2 );
				XMM3 = _mm_mul_ps( XMM3, XMM3 );
				XMM5 = _mm_mul_ps( XMM5, XMM5 );
				XMM0 = _mm_hadd_ps( XMM0, XMM2 );
				XMM3 = _mm_hadd_ps( XMM3, XMM5 );
			}
#else
			{
				__m128	XMM2, XMM5;
				{
					__m128	XMM1, XMM4;

					XMM0 = _mm_loadu_ps( pcm + j );
					XMM1 = _mm_loadu_ps( pcm + j + 4 );
					XMM3 = _mm_loadu_ps( pcm + j + 8 );
					XMM4 = _mm_loadu_ps( pcm + j + 12 );
					XMM2 = XMM0;
					XMM5 = XMM3;
					XMM0 = _mm_shuffle_ps( XMM0, XMM1, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM2 = _mm_shuffle_ps( XMM2, XMM1, _MM_SHUFFLE( 3, 1, 3, 1 ) );
					XMM3 = _mm_shuffle_ps( XMM3, XMM4, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM5 = _mm_shuffle_ps( XMM5, XMM4, _MM_SHUFFLE( 3, 1, 3, 1 ) );
				}
				XMM0 = _mm_mul_ps( XMM0, XMM0 );
				XMM3 = _mm_mul_ps( XMM3, XMM3 );
				XMM2 = _mm_mul_ps( XMM2, XMM2 );
				XMM5 = _mm_mul_ps( XMM5, XMM5 );
				XMM0 = _mm_add_ps( XMM0, XMM2 );
				XMM3 = _mm_add_ps( XMM3, XMM5 );
			}
#endif
			XMM0 = _mm_cvtepi32_ps( _mm_castps_si128( XMM0 ) );
			XMM3 = _mm_cvtepi32_ps( _mm_castps_si128( XMM3 ) );
			XMM0 = _mm_mul_ps( XMM0, mparm );
			XMM3 = _mm_mul_ps( XMM3, mparm + 4 );	// ERROR!
			XMM0 = _mm_add_ps( XMM0, SCALEdB );
			XMM3 = _mm_add_ps( XMM3, SCALEdB );
			_mm_storeu_ps( logfft + ( ( j + 1 ) >> 1 ), XMM0 );
			_mm_storeu_ps( logfft + ( ( j + 9 ) >> 1 ), XMM3 );
			XMM0 = _mm_max_ps( XMM0, XMM3 );
			LAM0 = _mm_max_ps( LAM0, XMM0 );
		}

		Cnt = ( ( n - 2 ) & ( ~7 ) ) + 1;
		for( ; j < Cnt; j += 8 )
		{
			__m128	XMM0;
#if	defined(__SSE3__)
			{
				__m128	XMM1;
				XMM0 = _mm_lddqu_ps( pcm + j );
				XMM1 = _mm_lddqu_ps( pcm + j + 4 );
				XMM0 = _mm_mul_ps( XMM0, XMM0 );
				XMM1 = _mm_mul_ps( XMM1, XMM1 );
				XMM0 = _mm_hadd_ps( XMM0, XMM1 );
			}
#else
		{
				__m128	XMM2;
				{
					__m128	XMM1;
					XMM0 = _mm_loadu_ps( pcm + j );
					XMM1 = _mm_loadu_ps( pcm + j + 4 );
					XMM2 = XMM0;
					XMM0 = _mm_shuffle_ps( XMM0, XMM1, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM2 = _mm_shuffle_ps( XMM2, XMM1, _MM_SHUFFLE( 3, 1, 3, 1 ) );
				}

				XMM0 = _mm_mul_ps( XMM0, XMM0 );
				XMM2 = _mm_mul_ps( XMM2, XMM2 );
				XMM0 = _mm_add_ps( XMM0, XMM2 );
			}
#endif
			XMM0 = _mm_cvtepi32_ps( _mm_castps_si128( XMM0 ) );
			XMM0 = _mm_mul_ps( XMM0, mparm );
			XMM0 = _mm_add_ps( XMM0, SCALEdB );
			_mm_storeu_ps( &logfft[( j + 1 ) >> 1], XMM0 );
			LAM0 = _mm_max_ps( LAM0, XMM0 );
		}

		local_ampmax[i] = _mm_max_horz( LAM0 );
		for( ; j < n; j += 2 )
		{
			float temp = pcm[j] * pcm[j] + pcm[j + 1] * pcm[j + 1];
			/* See note about 0.345 at top of file */
			temp = logfft[( j + 1 ) >> 1] = scale_dB + 0.5f * todB( &temp ) + 0.345; 

			if( temp > local_ampmax[i] )
			{
				local_ampmax[i] = temp;
			}
		}
	}
#else
	/* SSE optimized code */
	LAM1 = LAM0;
	if( n >= 256 && n <= 4096 )
	{
		/* Caution! This routine is for SSE optimized fft only. */
		float rfv = logfft[0];
		logfft[0] = 0.0f;
		logfft[1] = 0.0f;

		for( j = 0, k = 0; j < n; j += 32, k += 16 )
		{
			__m64	MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7;
			__m128x	U0, U1, U2, U3;
			{
				__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;

				XMM0 = _mm_load_ps( pcm + j );
				XMM2 = _mm_load_ps( pcm + j + 8 );
				XMM4 = _mm_load_ps( pcm + j + 4 );
				XMM5 = _mm_load_ps( pcm + j + 12 );
				XMM1 = XMM0;
				XMM3 = XMM2;
				XMM0 = _mm_shuffle_ps( XMM0, XMM4, _MM_SHUFFLE( 2, 0, 2, 0 ) );
				XMM1 = _mm_shuffle_ps( XMM1, XMM4, _MM_SHUFFLE( 3, 1, 3, 1 ) );
				XMM2 = _mm_shuffle_ps( XMM2, XMM5, _MM_SHUFFLE( 2, 0, 2, 0 ) );
				XMM3 = _mm_shuffle_ps( XMM3, XMM5, _MM_SHUFFLE( 3, 1, 3, 1 ) );
				XMM0 = _mm_mul_ps( XMM0, XMM0 );
				XMM1 = _mm_mul_ps( XMM1, XMM1 );
				XMM2 = _mm_mul_ps( XMM2, XMM2 );
				XMM3 = _mm_mul_ps( XMM3, XMM3 );
				XMM0 = _mm_add_ps( XMM0, XMM1 );
				XMM2 = _mm_add_ps( XMM2, XMM3 );
				XMM4 = _mm_load_ps( pcm + j + 16 );
				U0.ps = XMM0;
				U1.ps = XMM2;
				XMM1 = _mm_load_ps( pcm + j + 24 );
				XMM0 = _mm_load_ps( pcm + j + 20 );
				XMM2 = _mm_load_ps( pcm + j + 28 );
				XMM5 = XMM4;
				XMM3 = XMM1;
				XMM4 = _mm_shuffle_ps( XMM4, XMM0, _MM_SHUFFLE( 2, 0, 2, 0 ) );
				XMM5 = _mm_shuffle_ps( XMM5, XMM0, _MM_SHUFFLE( 3, 1, 3, 1 ) );
				XMM1 = _mm_shuffle_ps( XMM1, XMM2, _MM_SHUFFLE( 2, 0, 2, 0 ) );
				XMM3 = _mm_shuffle_ps( XMM3, XMM2, _MM_SHUFFLE( 3, 1, 3, 1 ) );
				MM0 = U0.pi64[1];
				MM1 = U1.pi64[1];
				MM2 = U0.pi64[0];
				MM3 = U1.pi64[0];
				XMM4 = _mm_mul_ps( XMM4, XMM4 );
				XMM5 = _mm_mul_ps( XMM5, XMM5 );
				XMM1 = _mm_mul_ps( XMM1, XMM1 );
				XMM3 = _mm_mul_ps( XMM3, XMM3 );
				XMM4 = _mm_add_ps( XMM4, XMM5 );
				XMM1 = _mm_add_ps( XMM1, XMM3 );
				XMM0 = _mm_cvtpi32_ps( XMM0, MM0 );
				XMM2 = _mm_cvtpi32_ps( XMM2, MM1 );
				U2.ps = XMM4;
				U3.ps = XMM1;
				MM4 = U2.pi64[1];
				MM5 = U3.pi64[1];
				MM6 = U2.pi64[0];
				MM7 = U3.pi64[0];
				XMM5 = _mm_cvtpi32_ps( XMM5, MM4 );
				XMM3 = _mm_cvtpi32_ps( XMM3, MM5 );
				XMM0 = _mm_movelh_ps( XMM0, XMM0 );
				XMM2 = _mm_movelh_ps( XMM2, XMM2 );
				XMM5 = _mm_movelh_ps( XMM5, XMM5 );
				XMM3 = _mm_movelh_ps( XMM3, XMM3 );
				XMM0 = _mm_cvtpi32_ps( XMM0, MM2 );
				XMM2 = _mm_cvtpi32_ps( XMM2, MM3 );
				XMM5 = _mm_cvtpi32_ps( XMM5, MM6 );
				XMM3 = _mm_cvtpi32_ps( XMM3, MM7 );
				XMM0 = _mm_mul_ps( XMM0, mparm );
				XMM2 = _mm_mul_ps( XMM2, mparm );
				XMM5 = _mm_mul_ps( XMM5, mparm );
				XMM3 = _mm_mul_ps( XMM3, mparm );
				XMM0 = _mm_add_ps( XMM0, SCALEdB );
				XMM2 = _mm_add_ps( XMM2, SCALEdB );
				XMM5 = _mm_add_ps( XMM5, SCALEdB );
				XMM3 = _mm_add_ps( XMM3, SCALEdB );
				_mm_store_ps( logfft + k, XMM0 );
				_mm_store_ps( logfft + k + 4, XMM2 );
				_mm_store_ps( logfft + k + 8, XMM5 );
				_mm_store_ps( logfft + k + 12, XMM3 );
				XMM0 = _mm_max_ps( XMM0, XMM2 );
				XMM5 = _mm_max_ps( XMM5, XMM3 );
				LAM0 = _mm_max_ps( LAM0, XMM0 );
				LAM1 = _mm_max_ps( LAM1, XMM5 );
			}
		}
		_mm_empty();

		logfft[0] = rfv;
		LAM0 = _mm_max_ps( LAM0, LAM1 );
		local_ampmax[i] = _mm_max_horz( LAM0 );
	}
	else
	{
		__m64	MM0, MM1, MM2, MM3;
		__m128x	U0, U1;

		int Cnt = ( ( n - 2 ) & ( ~15 ) ) + 1;
		for( j = 1; j < Cnt; j += 16 )
		{
			__m128	XMM0, XMM3;
			{
				__m128	XMM2, XMM5;
				{
					__m128	XMM1, XMM4;

					XMM0 = _mm_loadu_ps( pcm + j );
					XMM1 = _mm_loadu_ps( pcm + j + 4 );
					XMM3 = _mm_loadu_ps( pcm + j + 8 );
					XMM4 = _mm_loadu_ps( pcm + j + 12 );
					XMM2 = XMM0;
					XMM5 = XMM3;
					XMM0 = _mm_shuffle_ps( XMM0, XMM1, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM2 = _mm_shuffle_ps( XMM2, XMM1, _MM_SHUFFLE( 3, 1, 3, 1 ) );
					XMM3 = _mm_shuffle_ps( XMM3, XMM4, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM5 = _mm_shuffle_ps( XMM5, XMM4, _MM_SHUFFLE( 3, 1, 3, 1 ) );
				}

				XMM0 = _mm_mul_ps( XMM0, XMM0 );
				XMM3 = _mm_mul_ps( XMM3, XMM3 );
				XMM2 = _mm_mul_ps( XMM2, XMM2 );
				XMM5 = _mm_mul_ps( XMM5, XMM5 );
				XMM0 = _mm_add_ps( XMM0, XMM2 );
				XMM3 = _mm_add_ps( XMM3, XMM5 );
			}

			U0.ps = XMM0;
			U1.ps = XMM3;
			MM0	= U0.pi64[1];
			MM1	= U1.pi64[1];
			MM2	= U0.pi64[0];
			MM3	= U1.pi64[0];
			XMM0 = _mm_cvtpi32_ps( XMM0, MM0 );
			XMM3 = _mm_cvtpi32_ps( XMM3, MM1 );
			XMM0 = _mm_movelh_ps( XMM0, XMM0 );
			XMM3 = _mm_movelh_ps( XMM3, XMM3 );
			XMM0 = _mm_cvtpi32_ps( XMM0, MM2 );
			XMM3 = _mm_cvtpi32_ps( XMM3, MM3 );
			XMM0 = _mm_mul_ps( XMM0, mparm );
			XMM3 = _mm_mul_ps( XMM3, mparm );
			XMM0 = _mm_add_ps( XMM0, SCALEdB );
			XMM3 = _mm_add_ps( XMM3, SCALEdB );
			_mm_storeu_ps( logfft + ( ( j + 1 ) >> 1 ), XMM0 );
			_mm_storeu_ps( logfft + ( ( j + 9 ) >> 1 ), XMM3 );
			LAM0 = _mm_max_ps( LAM0, XMM0 );
			LAM1 = _mm_max_ps( LAM1, XMM3 );
		}

		Cnt = ( ( n - 2 ) & ( ~7 ) ) + 1;
		for( ; j < Cnt; j += 8 )
		{
			__m128	XMM0;
			{
				__m128	XMM2;
				{
					__m128	XMM1;

					XMM0 = _mm_loadu_ps( pcm + j );
					XMM1 = _mm_loadu_ps( pcm + j + 4 );
					XMM2 = XMM0;
					XMM0 = _mm_shuffle_ps( XMM0, XMM1, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM2 = _mm_shuffle_ps( XMM2, XMM1, _MM_SHUFFLE( 3, 1, 3, 1 ) );
				}

				XMM0 = _mm_mul_ps( XMM0, XMM0 );
				XMM2 = _mm_mul_ps( XMM2, XMM2 );
				XMM0 = _mm_add_ps( XMM0, XMM2 );
			}

			U0.ps = XMM0;
			MM0 = U0.pi64[1];
			MM1 = U0.pi64[0];
			XMM0 = _mm_cvtpi32_ps( XMM0, MM0 );
			XMM0 = _mm_movelh_ps( XMM0, XMM0 );
			XMM0 = _mm_cvtpi32_ps( XMM0, MM1 );
			XMM0 = _mm_mul_ps( XMM0, mparm );
			XMM0 = _mm_add_ps( XMM0, SCALEdB );
			_mm_storeu_ps( logfft + ( ( j + 1 ) >> 1 ), XMM0 );
			LAM0 = _mm_max_ps( LAM0, XMM0 );
		}

		LAM0 = _mm_max_ps( LAM0, LAM1 );
		_mm_empty();

		local_ampmax[i] = _mm_max_horz( LAM0 );
		for( ; j < n; j += 2 )
		{
			float temp = pcm[j] * pcm[j] + pcm[j + 1] * pcm[j + 1];
			/* See note about 0.345 at top of file */
			temp = logfft[( j + 1 ) >> 1] = scale_dB + 0.5f * todB( &temp ) + 0.345f; 
			if( temp > local_ampmax[i] )
			{
				local_ampmax[i]	= temp;
			}
		}
	}
#endif
}

static void mapping_forward_sub1( float* mdct, float* logmdct, int n )
{
	static const __m128 mparm = { 7.17711438e-7f, 7.17711438e-7f, 7.17711438e-7f, 7.17711438e-7f };
	static const __m128 PFV0 = { 0.345f - 764.6161886f,	0.345f - 764.6161886f, 0.345f - 764.6161886f, 0.345f - 764.6161886f };
	int j;
#if	defined(__SSE2__)
	/* SSE2 optimized code */
	for( j = 0; j < n / 2; j += 16 )
	{
		__m128	XMM0, XMM1, XMM2, XMM3;

		XMM0 = _mm_load_ps( mdct + j );
		XMM1 = _mm_load_ps( mdct + j + 4 );
		XMM2 = _mm_load_ps( mdct + j + 8 );
		XMM3 = _mm_load_ps( mdct + j + 12 );
		XMM0 = _mm_and_ps( XMM0, PM128( PABSMASK ) );
		XMM1 = _mm_and_ps( XMM1, PM128( PABSMASK ) );
		XMM2 = _mm_and_ps( XMM2, PM128( PABSMASK ) );
		XMM3 = _mm_and_ps( XMM3, PM128( PABSMASK ) );
		XMM0 = _mm_cvtepi32_ps( _mm_castps_si128( XMM0 ) );
		XMM1 = _mm_cvtepi32_ps( _mm_castps_si128( XMM1 ) );
		XMM2 = _mm_cvtepi32_ps( _mm_castps_si128( XMM2 ) );
		XMM3 = _mm_cvtepi32_ps( _mm_castps_si128( XMM3 ) );
		XMM0 = _mm_mul_ps( XMM0, mparm );
		XMM1 = _mm_mul_ps( XMM1, mparm );
		XMM2 = _mm_mul_ps( XMM2, mparm );
		XMM3 = _mm_mul_ps( XMM3, mparm );
		XMM0 = _mm_add_ps( XMM0, PFV0 );
		XMM1 = _mm_add_ps( XMM1, PFV0 );
		XMM2 = _mm_add_ps( XMM2, PFV0 );
		XMM3 = _mm_add_ps( XMM3, PFV0 );
		_mm_store_ps( logmdct + j, XMM0 );
		_mm_store_ps( logmdct + j + 4, XMM1 );
		_mm_store_ps( logmdct + j + 8, XMM2 );
		_mm_store_ps( logmdct + j + 12, XMM3 );
	}
#else	/* __SSE2__ */
	/* SSE optimized code */
	for( j = 0; j < n / 2; j += 16 )
	{
		__m128x	U0, U1, U2, U3;
		__m128	XMM0, XMM1, XMM2, XMM3;

		XMM0 = _mm_load_ps( mdct + j );
		XMM1 = _mm_load_ps( mdct + j + 4 );
		XMM2 = _mm_load_ps( mdct + j + 8 );
		XMM3 = _mm_load_ps( mdct + j + 12 );
		XMM0 = _mm_and_ps( XMM0, PM128( PABSMASK ) );
		XMM1 = _mm_and_ps( XMM1, PM128( PABSMASK ) );
		XMM2 = _mm_and_ps( XMM2, PM128( PABSMASK ) );
		XMM3 = _mm_and_ps( XMM3, PM128( PABSMASK ) );
		U0.ps = XMM0;
		U1.ps = XMM1;
		U2.ps = XMM2;
		U3.ps = XMM3;
		XMM0 = _mm_cvtpi32_ps( XMM0, U0.pi64[1] );
		XMM1 = _mm_cvtpi32_ps( XMM1, U1.pi64[1] );
		XMM2 = _mm_cvtpi32_ps( XMM2, U2.pi64[1] );
		XMM3 = _mm_cvtpi32_ps( XMM3, U3.pi64[1] );
		XMM0 = _mm_movelh_ps( XMM0, XMM0 );
		XMM1 = _mm_movelh_ps( XMM1, XMM1 );
		XMM2 = _mm_movelh_ps( XMM2, XMM2 );
		XMM3 = _mm_movelh_ps( XMM3, XMM3 );
		XMM0 = _mm_cvtpi32_ps( XMM0, U0.pi64[0] );
		XMM1 = _mm_cvtpi32_ps( XMM1, U1.pi64[0] );
		XMM2 = _mm_cvtpi32_ps( XMM2, U2.pi64[0] );
		XMM3 = _mm_cvtpi32_ps( XMM3, U3.pi64[0] );
		XMM0 = _mm_mul_ps( XMM0, mparm );
		XMM1 = _mm_mul_ps( XMM1, mparm );
		XMM2 = _mm_mul_ps( XMM2, mparm );
		XMM3 = _mm_mul_ps( XMM3, mparm );
		XMM0 = _mm_add_ps( XMM0, PFV0 );
		XMM1 = _mm_add_ps( XMM1, PFV0 );
		XMM2 = _mm_add_ps( XMM2, PFV0 );
		XMM3 = _mm_add_ps( XMM3, PFV0 );
		_mm_store_ps( logmdct + j, XMM0 );
		_mm_store_ps( logmdct + j + 4, XMM1 );
		_mm_store_ps( logmdct + j + 8, XMM2 );
		_mm_store_ps( logmdct + j + 12, XMM3 );
	}
	_mm_empty();
#endif
}
#endif														/* SSE Optimize */

static int mapping0_forward( vorbis_block* vb )
{
	vorbis_dsp_state* vd = vb->vd;
	vorbis_info* vi = vd->vi;
	codec_setup_info* ci = vi->codec_setup;
	private_state* b = vb->vd->backend_state;
	vorbis_block_internal* vbi = ( vorbis_block_internal* )vb->internal;
	vorbis_info_floor1* vif = ci->floor_param[vb->W];
	int n = vb->pcmend;
	int i, j, k;

	int* nonzero = alloca( sizeof( *nonzero ) * vi->channels );

	float** gmdct = _vorbis_block_alloc( vb, vi->channels * sizeof( *gmdct ) );
	float** gmdct_org = _vorbis_block_alloc( vb, vi->channels * sizeof( *gmdct_org ) );
	float** res_org = _vorbis_block_alloc( vb, vi->channels * sizeof( *res_org ) );
	int** ilogmaskch = _vorbis_block_alloc( vb, vi->channels * sizeof( *ilogmaskch ) );
	int*** floor_posts = _vorbis_block_alloc( vb, vi->channels * sizeof( *floor_posts ) );

	float global_ampmax = vbi->ampmax;
	float* local_ampmax = alloca( sizeof( *local_ampmax ) * vi->channels );
	int blocktype = vbi->blocktype;

	int modenumber = vb->W;
	vorbis_info_mapping0* info = ci->map_param[modenumber];
	vorbis_look_psy* psy_look = b->psy + blocktype + ( vb->W ? 2 : 0 );
	vb->mode = modenumber;

	for( i = 0; i < vi->channels; i++ )
	{
		float scale = 4.0f / n;
		float scale_dB;
		float* pcm = vb->pcm[i]; 
		float* logfft = pcm;

		gmdct[i] = _vorbis_block_alloc( vb, n / 2 * sizeof( **gmdct ) );
		gmdct_org[i] = _vorbis_block_alloc( vb, n / 2 * sizeof( **gmdct_org ) );
		res_org[i] = _vorbis_block_alloc( vb, n / 2 * sizeof( **res_org ) );

		/* See note about 0.345 at top of file */
		scale_dB = todB( &scale ) + 0.345f; 

		/* window the PCM data */
		_vorbis_apply_window( pcm, b->window, ci->blocksizes, vb->lW, vb->W, vb->nW );

		/* transform the PCM data */
		/* only MDCT right now.... */
#if	defined(__SSE__)										/* SSE Optimize */
		mdct_forward( b->transform[vb->W][0], pcm,gmdct[i], gmdct_org[i] );
#else														/* SSE Optimize */
		mdct_forward( b->transform[vb->W][0], pcm,gmdct[i] );
		memcpy( gmdct_org[i], gmdct[i], n / 2 * sizeof( **gmdct_org ) );
#endif														/* SSE Optimize */
		/* FFT yields more accurate tonal estimation (not phase sensitive) */
		drft_forward( &b->fft_look[vb->W], pcm );
		/* See note about 0.345 at top of file */
		logfft[0] = scale_dB + todB( pcm ) + 0.345f;
		local_ampmax[i] = logfft[0];
#ifdef __SSE__												/* SSE Optimize */
		mapping_forward_sub0( pcm, logfft, scale_dB, local_ampmax, i, n );
#else														/* SSE Optimize */
		for( j = 1; j < n - 1; j += 2 )
		{
			float temp = pcm[j] * pcm[j] + pcm[j + 1] * pcm[j + 1];
			/* See note about 0.345 at top of file */
			temp = logfft[( j + 1 ) >> 1] = scale_dB + 0.5f * todB( &temp ) + 0.345f; 
			if( temp > local_ampmax[i] )
			{
				local_ampmax[i] = temp;
			}
		}
#endif														/* SSE Optimize */

		if( local_ampmax[i] > 0.0f )
		{
			local_ampmax[i] = 0.0f;
		}

		if( local_ampmax[i] > global_ampmax )
		{
			global_ampmax = local_ampmax[i];
		}

	}
	{
		float* noise = _vorbis_block_alloc( vb, n / 2 * sizeof( *noise ) );
		float* tone = _vorbis_block_alloc( vb, n / 2 * sizeof( *tone ) );

		for( i = 0; i < vi->channels; i++ )
		{
			/* the encoder setup assumes that all the modes used by any specific bitrate tweaking use the same floor */
			int submap = info->chmuxlist[i];

			/* the following makes things clearer to *me* anyway */
			float* mdct = gmdct[i];
			float* logfft = vb->pcm[i];
			float* logmdct = logfft + n / 2;
			float* logmask = logfft;

			float* lastmdct = b->nblock + i * 128;
			float* tempmdct = b->tblock + i * 128;

			float* lowcomp = b->lownoise_compand_level + i;

			vb->mode = modenumber;

			floor_posts[i] = _vorbis_block_alloc( vb, PACKETBLOBS * sizeof( **floor_posts ) );
			memset( floor_posts[i], 0, sizeof( **floor_posts ) * PACKETBLOBS );
      
#ifdef __SSE__												/* SSE Optimize */
			mapping_forward_sub1( mdct, logmdct, n );
#else														/* SSE Optimize */
			for( j = 0; j < n / 2; j++ )
			{
				/* See note about 0.345 at top of file */
				logmdct[j] = todB( mdct + j ) + 0.345f; 
			}
#endif														/* SSE Optimize */

			/* first step; noise masking.  Not only does 'noise masking' give us curves from which we can decide how much resolution
			   to give noise parts of the spectrum, it also implicitly hands us a tonality estimate (the larger the value in the
			   'noise_depth' vector, the more tonal that area is) */
			*lowcomp = lb_loudnoise_fix( psy_look, *lowcomp, logmdct, b->lW_modenumber, blocktype, modenumber );

			/* noise does not have by-frequency offset bias applied yet */
			_vp_noisemask( psy_look, *lowcomp, logmdct, noise ); 

			/* second step: 'all the other crap'; all the stuff that isn't computed/fit for bitrate management goes in the second psy
			   vector.  This includes tone masking, peak limiting and ATH */
			_vp_tonemask( psy_look, logfft, tone, global_ampmax, local_ampmax[i] );

			/* third step; we offset the noise vectors, overlay tone masking.  We then do a floor1-specific line fit.  If we're
			   performing bitrate management, the line fit is performed multiple times for up/down tweakage on demand. */

			_vp_offset_and_mix( psy_look, noise, tone, 1, logmask,  mdct,  logmdct, lastmdct, tempmdct, *lowcomp, vif->n, blocktype, 
#ifdef __SSE__												/* SSE Optimize */
								modenumber, vb->nW, b->lW_blocktype, b->lW_modenumber, b->lW_no, res_org[i]);
#else														/* SSE Optimize */
								modenumber, vb->nW, b->lW_blocktype, b->lW_modenumber, b->lW_no );
#endif														/* SSE Optimize */
	
			/* this algorithm is hardwired to floor 1 for now; abort out if we're *not* floor1.  This won't happen unless someone has
			   broken the encode setup lib.  Guard it anyway. */
			if( ci->floor_type[info->floorsubmap[submap]] != 1 )
			{
				return( -1 );
			}

			floor_posts[i][PACKETBLOBS / 2] = floor1_fit( vb, b->flr[info->floorsubmap[submap]], logmdct, logmask );
      
			/* are we managing bitrate?  If so, perform two more fits for later rate tweaking (fits represent hi/lo) */
			if( vorbis_bitrate_managed( vb ) && floor_posts[i][PACKETBLOBS / 2] )
			{
				/* higher rate by way of lower noise curve */
				_vp_offset_and_mix( psy_look, noise, tone, 2, logmask, mdct, logmdct, lastmdct, tempmdct, *lowcomp, vif->n, blocktype, 
#ifdef __SSE__												/* SSE Optimize */
									modenumber, vb->nW, b->lW_blocktype, b->lW_modenumber, b->lW_no, res_org[i] );
#else														/* SSE Optimize */
									modenumber, vb->nW, b->lW_blocktype, b->lW_modenumber, b->lW_no );
#endif														/* SSE Optimize */

				floor_posts[i][PACKETBLOBS-1] = floor1_fit( vb, b->flr[info->floorsubmap[submap]], logmdct, logmask );
      
				/* lower rate by way of higher noise curve */
				_vp_offset_and_mix( psy_look, noise, tone, 0, logmask, mdct, logmdct, lastmdct, tempmdct, *lowcomp, vif->n, blocktype, 
#ifdef __SSE__												/* SSE Optimize */
									modenumber, vb->nW,b->lW_blocktype, b->lW_modenumber, b->lW_no, res_org[i] );
#else														/* SSE Optimize */
									modenumber, vb->nW,b->lW_blocktype, b->lW_modenumber, b->lW_no );
#endif														/* SSE Optimize */

				floor_posts[i][0] = floor1_fit( vb, b->flr[info->floorsubmap[submap]], logmdct, logmask );
	
				/* we also interpolate a range of intermediate curves for intermediate rates */
				for( k = 1; k < PACKETBLOBS / 2; k++ )
				{
					floor_posts[i][k] = floor1_interpolate_fit( vb, b->flr[info->floorsubmap[submap]], floor_posts[i][0], 
																floor_posts[i][PACKETBLOBS / 2], k * 65536 / ( PACKETBLOBS / 2 ) );
				}

				for( k = PACKETBLOBS / 2 + 1; k < PACKETBLOBS - 1; k++ )
				{
					floor_posts[i][k] = floor1_interpolate_fit( vb, b->flr[info->floorsubmap[submap]], floor_posts[i][PACKETBLOBS / 2], 
																floor_posts[i][PACKETBLOBS - 1], ( k - PACKETBLOBS / 2 ) * 65536 / ( PACKETBLOBS / 2 ) );
				}
			}
		}
	}

	vbi->ampmax = global_ampmax;

	/* the next phases are performed once for vbr-only and PACKETBLOB times for bitrate managed modes.

		1) encode actual mode being used
		2) encode the floor for each channel, compute coded mask curve/res
		3) normalize and couple.
		4) encode residue
		5) save packet bytes to the packetblob vector
	*/

	/* iterate over the many masking curve fits we've created */
	{
		float** res_bundle = alloca( sizeof( *res_bundle ) * vi->channels );
		float** couple_bundle = alloca( sizeof( *couple_bundle ) * vi->channels );
		int* zerobundle = alloca( sizeof( *zerobundle ) * vi->channels );
		int** sortindex = alloca( sizeof( *sortindex ) * vi->channels );
		float** mag_memo = NULL;
		int** mag_sort = NULL;

		if( info->coupling_steps )
		{
			mag_memo = _vp_quantize_couple_memo( vb, &ci->psy_g_param, psy_look, info, gmdct );    
			mag_sort = _vp_quantize_couple_sort( vb, psy_look, info,
#ifdef __SSE__												/* SSE Optimize */
												mag_memo, res_org[0] );    
#else														/* SSE Optimize */
												mag_memo );    
#endif														/* SSE Optimize */
		}

		memset( sortindex, 0, sizeof( *sortindex ) * vi->channels );
		if( psy_look->vi->normal_channel_p )
		{
			for( i = 0; i < vi->channels; i++ )
			{
				float* mdct = gmdct[i];
				sortindex[i] = alloca( sizeof( **sortindex ) * n / 2 );
#ifdef __SSE__												/* SSE Optimize */
				_vp_noise_normalize_sort( psy_look, mdct, sortindex[i], res_org[0] );
#else														/* SSE Optimize */
				_vp_noise_normalize_sort( psy_look, mdct,sortindex[i] );
#endif														/* SSE Optimize */
			}
		}

		for( k = ( vorbis_bitrate_managed( vb ) ? 0 : PACKETBLOBS / 2 ); k <= ( vorbis_bitrate_managed( vb ) ? PACKETBLOBS - 1 : PACKETBLOBS / 2 ); k++ )
		{
			oggpack_buffer* opb = vbi->packetblob[k];

			/* start out our new packet blob with packet type and mode */
			/* Encode the packet type */
			oggpack_write( opb, 0, 1 );
			/* Encode the modenumber */
			/* Encode frame mode, pre,post windowsize, then dispatch */
			oggpack_write( opb, modenumber, b->modebits );
			if( vb->W )
			{
				oggpack_write( opb, vb->lW, 1 );
				oggpack_write( opb, vb->nW, 1 );
			}

			/* encode floor, compute masking curve, sep out residue */
			for( i = 0; i < vi->channels; i++ )
			{
				int submap = info->chmuxlist[i];
				float* mdct = gmdct[i];
				float* mdct_org = gmdct_org[i];
				float* res = vb->pcm[i];
				float* resorgch = res_org[i];
				int* ilogmask = ilogmaskch[i] = _vorbis_block_alloc( vb, n / 2 * sizeof( **gmdct ) );
      
				nonzero[i] = floor1_encode( opb, vb, b->flr[info->floorsubmap[submap]], floor_posts[i][k], ilogmask );

				_vp_remove_floor( psy_look, mdct, ilogmask, res, ci->psy_g_param.sliding_lowpass[vb->W][k] );

				/* stereo threshold */
				_vp_remove_floor( psy_look, mdct_org, ilogmask, resorgch, ci->psy_g_param.sliding_lowpass[vb->W][k] );
		 
				_vp_noise_normalize( psy_look, res, res + n / 2, sortindex[i] );
			}
      
			/* our iteration is now based on masking curve, not prequant and coupling.  Only one prequant/coupling step */

			/* quantize/couple */
			/* incomplete implementation that assumes the tree is all depth one, or no tree at all */
			if( info->coupling_steps )
			{
				_vp_couple( k, &ci->psy_g_param, psy_look, info, vb->pcm, mag_memo, mag_sort, ilogmaskch, nonzero, ci->psy_g_param.sliding_lowpass[vb->W][k], gmdct, res_org );
			}

			/* classify and encode by submap */
			for( i = 0; i < info->submaps; i++ )
			{
				int ch_in_bundle = 0;
				long** classifications;
				int resnum = info->residuesubmap[i];

				for( j = 0; j < vi->channels; j++ )
				{
					if( info->chmuxlist[j] == i )
					{
						zerobundle[ch_in_bundle] = 0;
						if( nonzero[j] )
						{
							zerobundle[ch_in_bundle] = 1;
						}

						res_bundle[ch_in_bundle] = vb->pcm[j];
						couple_bundle[ch_in_bundle++] = vb->pcm[j] + n / 2;
					}
				}
	
				classifications = _residue_P[ci->residue_type[resnum]]->class( vb, b->residue[resnum], couple_bundle, zerobundle, ch_in_bundle );

				_residue_P[ci->residue_type[resnum]]->forward( opb, vb, b->residue[resnum], couple_bundle, NULL, zerobundle, ch_in_bundle, classifications );
			}

			/* set last-window type & number */
			if( ( blocktype == b->lW_blocktype ) && ( modenumber == b->lW_modenumber ) )
			{
				b->lW_no++;
			}
			else
			{
				b->lW_no = 1;
			}

			b->lW_blocktype = blocktype;
			b->lW_modenumber = modenumber;
			/* ok, done encoding.  Next protopacket. */
		}
	}

	return( 0 );
}

static int mapping0_inverse( vorbis_block* vb, vorbis_info_mapping* l )
{
	vorbis_dsp_state* vd = vb->vd;
	vorbis_info* vi = vd->vi;
	codec_setup_info* ci = vi->codec_setup;
	private_state* b = vd->backend_state;
	vorbis_info_mapping0* info = ( vorbis_info_mapping0* )l;
	int i, j;
	long n = vb->pcmend = ci->blocksizes[vb->W];
	float** pcmbundle = alloca( sizeof( *pcmbundle ) * vi->channels );
	int* zerobundle = alloca( sizeof( *zerobundle ) * vi->channels );
	int* nonzero = alloca( sizeof( *nonzero ) * vi->channels );
	void** floormemo = alloca( sizeof( *floormemo ) * vi->channels );

	/* recover the spectral envelope; store it in the PCM vector for now */
	for( i = 0; i < vi->channels; i++ )
	{
		int submap = info->chmuxlist[i];
		floormemo[i] = _floor_P[ci->floor_type[info->floorsubmap[submap]]]->inverse1( vb, b->flr[info->floorsubmap[submap]] );
		if( floormemo[i] )
		{
			nonzero[i] = 1;
		}
		else
		{
			nonzero[i] = 0;      
		}

		memset( vb->pcm[i], 0, sizeof( *vb->pcm[i] ) * n / 2 );
	}

	/* channel coupling can 'dirty' the nonzero listing */
	for( i = 0; i < info->coupling_steps; i++ )
	{
		if( nonzero[info->coupling_mag[i]] || nonzero[info->coupling_ang[i]] )
		{
			nonzero[info->coupling_mag[i]] = 1; 
			nonzero[info->coupling_ang[i]] = 1; 
		}
	}

	/* recover the residue into our working vectors */
	for( i = 0; i < info->submaps; i++ )
	{
		int ch_in_bundle = 0;
		for( j = 0; j < vi->channels; j++ )
		{
			if( info->chmuxlist[j] == i )
			{
				if( nonzero[j] )
				{
					zerobundle[ch_in_bundle] = 1;
				}
				else
				{	
					zerobundle[ch_in_bundle] = 0;
				}

				pcmbundle[ch_in_bundle++] = vb->pcm[j];
			}
		}

		_residue_P[ci->residue_type[info->residuesubmap[i]]]->inverse( vb, b->residue[info->residuesubmap[i]], pcmbundle, zerobundle, ch_in_bundle );
	}

	/* channel coupling */
	for( i = info->coupling_steps - 1; i >= 0; i-- )
	{
#ifdef	__SSE__												/* SSE Optimize */
		{
			float* PCMM = vb->pcm[info->coupling_mag[i]];
			float* PCMA = vb->pcm[info->coupling_ang[i]];
			int	Lim = ( n / 2 ) & ( ~7 );

			for( j = 0; j < Lim; j += 8 )
			{
				__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5;

				XMM0 = _mm_load_ps( PCMA + j );
				XMM3 = _mm_load_ps( PCMA + j + 4 );
				XMM1 = _mm_load_ps( PCMM + j );
				XMM4 = _mm_load_ps( PCMM + j + 4 );
				XMM2 = XMM0;
				XMM5 = XMM3;
				XMM0 = _mm_cmpnle_ps( XMM0, PFV_0 );
				XMM3 = _mm_cmpnle_ps( XMM3, PFV_0 );
				XMM1 = _mm_xor_ps( XMM1, XMM2 );
				XMM4 = _mm_xor_ps( XMM4, XMM5 );
				XMM1 = _mm_andnot_ps( XMM1, PM128( PCS_RRRR ) );
				XMM4 = _mm_andnot_ps( XMM4, PM128( PCS_RRRR ) );
				XMM1 = _mm_xor_ps( XMM1, XMM2 );
				XMM4 = _mm_xor_ps( XMM4, XMM5 );
				XMM2 = XMM1;
				XMM5 = XMM4;
				XMM1 = _mm_and_ps( XMM1, XMM0 );
				XMM4 = _mm_and_ps( XMM4, XMM3 );
				XMM0 = _mm_andnot_ps( XMM0, XMM2 );
				XMM3 = _mm_andnot_ps( XMM3, XMM5 );
				XMM2 = _mm_load_ps( PCMM + j );
				XMM5 = _mm_load_ps( PCMM + j + 4 );
				XMM1 = _mm_add_ps( XMM1, XMM2 );
				XMM4 = _mm_add_ps( XMM4, XMM5 );
				XMM0 = _mm_add_ps( XMM0, XMM2 );
				XMM3 = _mm_add_ps( XMM3, XMM5 );
				_mm_store_ps( PCMA + j, XMM1 );
				_mm_store_ps( PCMA + j + 4, XMM4 );
				_mm_store_ps( PCMM + j, XMM0 );
				_mm_store_ps( PCMM + j + 4, XMM3 );
			}

			Lim	= ( n / 2 ) & ( ~3 );
			for( ; j < Lim; j += 4 )
			{
				__m128	XMM0, XMM1, XMM2;

				XMM0 = _mm_load_ps( PCMA + j );
				XMM1 = _mm_load_ps( PCMM + j );
				XMM2 = XMM0;
				XMM0 = _mm_cmpnle_ps( XMM0, PFV_0 );
				XMM1 = _mm_xor_ps( XMM1, XMM2 );
				XMM1 = _mm_andnot_ps( XMM1, PM128( PCS_RRRR ) );
				XMM1 = _mm_xor_ps( XMM1, XMM2 );
				XMM2 = XMM1;
				XMM1 = _mm_and_ps( XMM1, XMM0 );
				XMM0 = _mm_andnot_ps( XMM0, XMM2 );
				XMM2 = _mm_load_ps( PCMM + j );
				XMM1 = _mm_add_ps( XMM1, XMM2 );
				XMM0 = _mm_add_ps( XMM0, XMM2 );
				_mm_store_ps( PCMA + j, XMM1 );
				_mm_store_ps( PCMM + j, XMM0 );
			}

			for( ; j < n / 2; j++ )
			{
				float mag = PCMM[j];
				float ang = PCMA[j];

				if( ang > 0.0f )
				{
					PCMM[j] = mag;
					PCMA[j] = mag > 0.0f ? mag - ang : mag + ang;
				}
				else
				{
					PCMM[j] = mag > 0.0f ? mag + ang : mag - ang;
					PCMA[j] = mag;
				}
			}
		}
#else														/* SSE Optimize */
		float* pcmM = vb->pcm[info->coupling_mag[i]];
		float* pcmA = vb->pcm[info->coupling_ang[i]];

		for( j = 0; j < n / 2; j++ )
		{
			float mag = pcmM[j];
			float ang = pcmA[j];

			if( mag > 0.0f )
			{
				if( ang > 0.0f )
				{
					pcmM[j] = mag;
					pcmA[j] = mag - ang;
				}
				else
				{
					pcmA[j] = mag;
					pcmM[j] = mag + ang;
				}
			}
			else
			{
				if( ang > 0.0f )
				{
					pcmM[j] = mag;
					pcmA[j] = mag + ang;
				}
				else
				{
					pcmA[j] = mag;
					pcmM[j] = mag - ang;
				}
			}
		}
#endif														/* SSE Optimize */
	}

	/* compute and apply spectral envelope */
	for( i = 0; i < vi->channels; i++ )
	{
		float* pcm = vb->pcm[i];
		int submap = info->chmuxlist[i];
		_floor_P[ci->floor_type[info->floorsubmap[submap]]]->inverse2( vb, b->flr[info->floorsubmap[submap]], floormemo[i], pcm );
	}

	/* transform the PCM data; takes PCM vector, vb; modifies PCM vector */
	/* only MDCT right now.... */
	for( i = 0; i < vi->channels; i++ )
	{
		float* pcm = vb->pcm[i];
		mdct_backward( b->transform[vb->W][0], pcm, pcm );
	}

	/* all done! */
	return( 0 );
}

/* export hooks */
vorbis_func_mapping mapping0_exportbundle =
{
	&mapping0_pack,
	&mapping0_unpack,
	&mapping0_free_info,
	&mapping0_forward,
	&mapping0_inverse
};

