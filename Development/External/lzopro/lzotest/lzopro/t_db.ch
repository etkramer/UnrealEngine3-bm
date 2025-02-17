/* t_db.ch -- compression database

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


#if defined(HAVE_LZO1X_H)

#define entry(l) \
{ "PLZO1X-1(" #l ")", M_PLZO1X_1_ ## l, LZOPRO_LZO1X_1_ ## l ## _MEM_COMPRESS, 0, \
  lzopro_lzo1x_1_ ## l ## _compress, lzo1x_optimize, \
  lzopro_lzo1x_decompress,      lzopro_lzo1x_decompress_safe, \
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe, \
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe, \
  0,                            0 },

entry(06)
entry(07)
entry(08)
entry(09)
entry(10)
entry(11)
entry(12)
entry(13)
entry(14)
entry(14)
entry(15)
entry(16)

#undef entry

#endif


#if defined(HAVE_LZO1Y_H)

#define entry(l) \
{ "PLZO1Y-1(" #l ")", M_PLZO1Y_1_ ## l, LZOPRO_LZO1Y_1_ ## l ## _MEM_COMPRESS, 0, \
  lzopro_lzo1y_1_ ## l ## _compress, lzo1y_optimize, \
  lzopro_lzo1y_decompress,      lzopro_lzo1y_decompress_safe, \
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe, \
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe, \
  0,                            0 },

entry(06)
entry(07)
entry(08)
entry(09)
entry(10)
entry(11)
entry(12)
entry(13)
entry(14)
entry(14)
entry(15)
entry(16)

#undef entry

#endif


/*
vi:ts=4:et
*/

