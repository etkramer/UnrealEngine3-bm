/* compr1b.h --

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


#define LZO_NEED_DICT_H
#include "config1b.h"


#if !defined(COMPRESS_ID)
#define COMPRESS_ID     LZO_CPP_ECONCAT2(DD_BITS,CLEVEL)
#endif


#include "lzo1b_c.ch"


/***********************************************************************
//
************************************************************************/

#define LZO_COMPRESS \
    LZO_CPP_ECONCAT3(lzo1b_,COMPRESS_ID,_compress)

#define LZO_COMPRESS_FUNC \
    LZO_CPP_ECONCAT3(_lzo1b_,COMPRESS_ID,_compress_func)



/***********************************************************************
//
************************************************************************/

const lzo_compress_t LZO_COMPRESS_FUNC = do_compress;

LZO_PUBLIC(int)
LZO_COMPRESS ( const lzo_bytep in,  lzo_uint  in_len,
                     lzo_bytep out, lzo_uintp out_len,
                     lzo_voidp wrkmem )
{
    return _lzo1b_do_compress(in,in_len,out,out_len,wrkmem,do_compress);
}

/*
vi:ts=4:et
*/
