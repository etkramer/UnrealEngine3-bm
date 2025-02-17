/* asm.h -- library assembler function prototypes

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


/*************************************************************************
//
**************************************************************************/

#if defined(LZO_USE_ASM)
#  include "lzo/lzo_asm.h"
#else
#  define lzo1c_decompress_asm              0
#  define lzo1c_decompress_asm_safe         0
#  define lzo1f_decompress_asm_fast         0
#  define lzo1f_decompress_asm_fast_safe    0
#  define lzo1x_decompress_asm              0
#  define lzo1x_decompress_asm_safe         0
#  define lzo1x_decompress_asm_fast         0
#  define lzo1x_decompress_asm_fast_safe    0
#  define lzo1y_decompress_asm              0
#  define lzo1y_decompress_asm_safe         0
#  define lzo1y_decompress_asm_fast         0
#  define lzo1y_decompress_asm_fast_safe    0
#endif


/*************************************************************************
// these are not yet implemented
**************************************************************************/

#define lzo1b_decompress_asm                0
#define lzo1b_decompress_asm_safe           0
#define lzo1b_decompress_asm_fast           0
#define lzo1b_decompress_asm_fast_safe      0

#define lzo1c_decompress_asm_fast           0
#define lzo1c_decompress_asm_fast_safe      0

#define lzo1f_decompress_asm                0
#define lzo1f_decompress_asm_safe           0


/*
vi:ts=4:et
*/

