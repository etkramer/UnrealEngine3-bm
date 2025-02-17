/* plzo1_c.ch --

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


#if !defined(LZO1F) && !defined(LZO1X) && !defined(LZO1Y) && !defined(LZO1Z)
#  define LZO1X
#endif


#define LZO_NEED_DICT_H
#if defined(LZO1F)
#  include "config1f.h"
#  include "lzo/lzopro/lzo1f.h"
#elif defined(LZO1X)
#  include "config1x.h"
#  include "lzo/lzopro/lzo1x.h"
#elif defined(LZO1Y)
#  include "config1y.h"
#  include "lzo/lzopro/lzo1y.h"
#else
#  error
#endif


#include "plzo_dict.h"

#if 1 && defined(DO_COMPRESS) && !defined(do_compress)
   /* choose a unique name to better help PGO optimizations */
#  define do_compress       LZO_CPP_ECONCAT2(DO_COMPRESS,_core)
#endif

#include "plzo1x_c.ch"


/* vim:set ts=4 et: */
