/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2003             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: Header of SSE Function Library
 last mod: $Id: xmmlib.h,v 1.3 2005-07-08 15:00:00+09 blacksword Exp $

 ********************************************************************/

#ifndef _XMMLIB_H_INCLUDED
#define _XMMLIB_H_INCLUDED

#if !defined(STIN)
#define STIN static __inline
#endif

#if defined(__SSE__)
#include <emmintrin.h>

#define PM64( x )		( *( __m64* )( x ) )
#define PM128( x )		( *( __m128* )( x ) )
#ifdef	__SSE2__
#define PM128I( x )		( *( __m128i* )( x ) )
#define PM128D( x )		( *( __m128d* )( x ) )
#endif

typedef union __declspec( intrin_type ) _MM_ALIGN16 __m128x
{
	unsigned long	si32[4];
	float			sf[4];
	__m64			pi64[2];
	__m128			ps;
#ifdef	__SSE2__
	__m128i			pi;
	__m128d			pd;
#endif
} __m128x;

#if defined(__SSE3__)
#define	_mm_lddqu_ps( x )	_mm_castsi128_ps( _mm_lddqu_si128( ( __m128i* )( x ) ) )
#else
#define	_mm_lddqu_ps( x )	_mm_loadu_ps( x )
#endif

extern _MM_ALIGN16 const unsigned long PCS_RRRR[4];
extern _MM_ALIGN16 const unsigned long PABSMASK[4];
extern _MM_ALIGN16 const unsigned long PMASKTABLE[4];

extern const __m128 PFV_0;   
extern const __m128 PFV_1;   
extern const __m128 PFV_2;   
extern const __m128 PFV_4;   
extern const __m128 PFV_8;   
extern const __m128 PFV_INIT;
extern const __m128 PFV_0P5; 
extern const __m128 PFV_M0P5;

extern const __m128 PCS_NNRR2;
extern const __m128 PCS_NRNR2;
extern const __m128 PCS_NRRN2;
extern const __m128 PCS_NRRR2;
extern const __m128 PCS_RNRN2;
extern const __m128 PCS_RRNN2;
extern const __m128 PCS_NNNR2;
extern const __m128 PCS_RNNR2;
extern const __m128 PCS_RRRR2;

extern const __m128 ConstMax;

extern const int bitCountTable[16];

STIN float _mm_add_horz( __m128 x )
{
#if	defined( __SSE3__ )
	x = _mm_hadd_ps( x, x );
	x = _mm_hadd_ps( x, x );
#else
	__m128	y;
	float	r;

	y = _mm_movehl_ps( x, x );
	x = _mm_add_ps( x, y );
	y = x;
	y = _mm_shuffle_ps( y, y, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	x = _mm_add_ss( x, y );
#endif

	_mm_store_ss( &r, x ); 
	return( r );
}

STIN float _mm_max_horz( __m128 x )
{
	__m128	y;
	float	r;

	y = _mm_movehl_ps( x, x );
	x = _mm_max_ps( x, y );
	y = x;
	y = _mm_shuffle_ps( y, y, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	x = _mm_max_ss( x, y );

	_mm_store_ss( &r, x ); 
	return( r );
}

void* xmm_calloc(size_t nitems, size_t size);
void* xmm_align( void* address );
void xmm_copy_forward( float* d, float* s, int count );
void xmm_copy_backward( float* d, float* s, int count );

#endif /* defined(__SSE__) */

#endif /* _XMMLIB_H_INCLUDED */
