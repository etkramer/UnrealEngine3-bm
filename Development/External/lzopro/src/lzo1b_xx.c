/* lzo1b_xx.c -- LZO1B compression public entry point

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


#include "config1b.h"


/***********************************************************************
//
************************************************************************/

static const lzo_compress_t * const c_funcs [9] =
{
    &_lzo1b_1_compress_func,
    &_lzo1b_2_compress_func,
    &_lzo1b_3_compress_func,
    &_lzo1b_4_compress_func,
    &_lzo1b_5_compress_func,
    &_lzo1b_6_compress_func,
    &_lzo1b_7_compress_func,
    &_lzo1b_8_compress_func,
    &_lzo1b_9_compress_func
};


lzo_compress_t _lzo1b_get_compress_func(int clevel)
{
    const lzo_compress_t *f;

    if (clevel < LZO1B_BEST_SPEED || clevel > LZO1B_BEST_COMPRESSION)
    {
        if (clevel == LZO1B_DEFAULT_COMPRESSION)
            clevel = LZO1B_BEST_SPEED;
        else
            return 0;
    }
    f = c_funcs[clevel-1];
    assert(f && *f);
    return *f;
}


LZO_PUBLIC(int)
lzo1b_compress ( const lzo_bytep src, lzo_uint  src_len,
                       lzo_bytep dst, lzo_uintp dst_len,
                       lzo_voidp wrkmem,
                       int clevel )
{
    lzo_compress_t f;

    f = _lzo1b_get_compress_func(clevel);
    if (!f)
        return LZO_E_ERROR;
    return _lzo1b_do_compress(src,src_len,dst,dst_len,wrkmem,f);
}



/*
vi:ts=4:et
*/

