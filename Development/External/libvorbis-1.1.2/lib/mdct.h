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

 function: modified discrete cosine transform prototypes
 last mod: $Id: mdct.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef _OGG_mdct_H_
#define _OGG_mdct_H_

#include "vorbis/codec.h"

#define cPI3_8	0.38268343236508977175f
#define cPI2_8	0.70710678118654752441f
#define cPI1_8	0.92387953251128675613f

typedef struct 
{
	int		n;
	int		log2n;

	float*	trig;
#ifdef __SSE__												/* SSE Optimize */
	float*	trig_bitreverse;
	float*	trig_forward;
	float*	trig_backward;
	float*	trig_butterfly_first;
	float*	trig_butterfly_generic8;
	float*	trig_butterfly_generic16;
	float*	trig_butterfly_generic32;
	float*	trig_butterfly_generic64;
#endif														/* SSE Optimize */
	int*	bitrev;
	float	scale;
} mdct_lookup;

extern void mdct_init( mdct_lookup* lookup, int n );
extern void mdct_clear( mdct_lookup* l );
#ifdef __SSE__												/* SSE Optimize */
extern void mdct_forward( mdct_lookup* init, float* in, float* out, float* out1 );
#else														/* SSE Optimize */
extern void mdct_forward( mdct_lookup* init, float* in, float* out );
#endif														/* SSE Optimize */
extern void mdct_backward( mdct_lookup* init, float* in, float* out );

#endif












