/* pinit.c -- initialization of the LZO library

   This file is part of the LZO-Professional data compression library.

   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   CONFIDENTIAL & PROPRIETARY SOURCE CODE.

   ANY USAGE OF THIS FILE IS SUBJECT TO YOUR LICENSE AGREEMENT.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/products/lzo-professional/
 */


#include "lzo_conf.h"


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(int)
__lzopro_lzo_init_v2(unsigned v, int s1, int s2, int s3, int s4, int s5,
                     int s6, int s7, int s8, int s9)
{
    LZO_COMPILE_TIME_ASSERT(sizeof(lzo_uint) == sizeof(size_t))
    LZO_COMPILE_TIME_ASSERT(sizeof(lzo_uint) == sizeof(ptrdiff_t))
    return __lzo_init_v2(v, s1, s2, s3, s4, s5, s6, s7, s8, s9);
}


/*
vi:ts=4:et
*/
