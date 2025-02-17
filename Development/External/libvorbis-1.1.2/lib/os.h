#ifndef _OS_H
#define _OS_H
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

 function: #ifdef jail to whip a few platforms into the UNIX ideal.
 last mod: $Id: os.h 7543 2004-08-13 01:48:19Z conrad $

 ********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <ogg/os_types.h>

#include "misc.h"

#ifndef _V_IFDEFJAIL_H_
#  define _V_IFDEFJAIL_H_

#  ifdef __GNUC__
#    define STIN static __inline__
#  elif _WIN32
#    define STIN static __inline
#  else
#    define STIN static
#  endif

#ifndef M_PI
#  define M_PI ( 3.1415926536f )
#endif

#if defined(_WIN32) && !defined(__SYMBIAN32__)
#  include <malloc.h>
#  define NO_FLOAT_MATH_LIB

_inline long rint( float x ) { return( ( long )floorf( x + 0.5f ) ); }

#pragma warning( disable : 4100 )		// unreferenced formal parameter
#pragma warning( disable : 4127 )		// conditional expression is constant

#endif

#if defined(__SYMBIAN32__) && defined(__WINS__)
void *_alloca( size_t size );
#  define alloca _alloca
#endif

#endif

#ifdef HAVE_ALLOCA_H
#  include <alloca.h>
#endif

#ifdef USE_MEMORY_H
#  include <memory.h>
#endif

#ifndef min
#  define min( x, y )  ( ( x ) > ( y ) ? ( y ) : ( x ) )
#endif

#ifndef max
#  define max( x, y )  ( ( x ) < ( y ) ? ( y ) : ( x ) )
#endif

#endif /* _OS_H */
