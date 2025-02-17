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

 function: miscellaneous prototypes
 last mod: $Id: misc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef _V_RANDOM_H_
#define _V_RANDOM_H_

#include "vorbis/codec.h"

extern int analysis_noisy;

#if !defined(STIN)
#define STIN static __inline
#endif

STIN int ilog( unsigned int v )
{
	unsigned long result;

	if( _BitScanReverse( &result, v ) )
	{
		return( result + 1 );
	}

	return( 0 );
}

STIN int ilog2( unsigned int v )
{
	if( v )
	{
		v--;
	}

	return( ilog( v ) );
}

extern void* _vorbis_block_alloc( vorbis_block* vb, long bytes );
extern void _vorbis_block_ripcord( vorbis_block* vb );
extern const float* _vorbis_window_get( int n );
extern void _vorbis_apply_window( float* d, int* winno, long* blocksizes, int lW, int W, int nW );

//#define DEBUG_MALLOC

#undef _ogg_malloc
#undef _ogg_calloc
#undef _ogg_realloc
#undef _ogg_free
#undef _ogg_alloca

#ifdef DEBUG_MALLOC

extern void* _VDBG_malloc( long bytes, char* file, long line ); 
extern void* _VDBG_calloc( long bytes, char* file, long line ); 
extern void* _VDBG_realloc( void* ptr, long bytes, char* file, long line ); 
extern void _VDBG_free( void* ptr, char* file, long line ); 

#define _ogg_malloc( x ) _VDBG_malloc( ( x ), __FILE__, __LINE__ )
#define _ogg_calloc( x, y ) _VDBG_calloc( ( x ) * ( y ), __FILE__, __LINE__ )
#define _ogg_realloc( x, y ) _VDBG_realloc( ( x ), ( y ), __FILE__, __LINE__ )
#define _ogg_free( x ) _VDBG_free( ( x ), __FILE__, __LINE__ )

#else												/* SSE Optimize */

#define _ogg_malloc( x ) _aligned_malloc( ( x ), 16 )
#define _ogg_calloc( x, y ) xmm_calloc( ( x ), ( y ) )
#define _ogg_realloc( x, y ) _aligned_realloc( ( x ), ( y ), 16 )
#define _ogg_free( x ) _aligned_free( ( x ) )

#endif														/* SSE Optimize */

#define _ogg_alloca( x ) xmm_align( alloca( x + 15 ) )

#endif




