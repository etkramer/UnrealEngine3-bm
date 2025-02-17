/* simple.c -- the annotated simple example program for the LZO library

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
// This program shows the basic usage of the LZO library.
// We will compress a block of data and decompress again.
**************************************************************************/

/* We will be using the LZO1X-1 algorithm, so we have
 * to include <lzo/lzopro/lzo1x.h>
 */

#include "lzo/lzoconf.h"
#include "lzo/lzopro/lzo1x.h"

/* portability layer */
#define WANT_LZO_MALLOC 1
#include "examples/portab.h"


/* We want to compress the data block at `in' with length `IN_LEN' to
 * the block at `out'. Because the input block may be incompressible,
 * we must provide a little more output space in case that compression
 * is not possible.
 */

#ifndef IN_LEN
#define IN_LEN      (128*1024L)
#endif
#define OUT_LEN     (IN_LEN + IN_LEN / 16 + 64 + 3)


/* malloc/free callbacks required for lzopro_lzo1[xy]_99_compress() */
static lzo_voidp __LZO_CDECL
cb_nalloc(lzo_callback_p self, lzo_uint items, lzo_uint size)
{
    LZO_UNUSED(self);
    return lzo_malloc(items * size);
}

static void __LZO_CDECL
cb_nfree(lzo_callback_p self, lzo_voidp ptr)
{
    LZO_UNUSED(self);
    lzo_free(ptr);
}


/*************************************************************************
//
**************************************************************************/

int __lzo_cdecl_main main(int argc, char *argv[])
{
    int r;
    lzo_bytep in;
    lzo_bytep out;
    lzo_voidp wrkmem;
    lzo_uint in_len;
    lzo_uint out_len;
    lzo_uint new_len;

    if (argc < 0 && argv == NULL)   /* avoid warning about unused args */
        return 0;

    printf("\nLZO-Professional real-time data compression library (v%s, %s).\n",
           lzo_version_string(), lzo_version_date());
    printf("Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer\nAll Rights Reserved.\n\n");

/*
 * Step 1: initialize the LZO library
 */
    if (lzo_init() != LZO_E_OK)
    {
        printf("internal error - lzo_init() failed !!!\n");
        printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable `-DLZO_DEBUG' for diagnostics)\n");
        return 1;
    }

/*
 * Step 2: allocate blocks and the work-memory
 */
    in = (lzo_bytep) lzo_malloc(IN_LEN);
    out = (lzo_bytep) lzo_malloc(OUT_LEN);
    wrkmem = (lzo_voidp) lzo_malloc(LZOPRO_LZO1X_1_14_MEM_COMPRESS);
    if (in == NULL || out == NULL || wrkmem == NULL)
    {
        printf("out of memory\n");
        return 1;
    }

/*
 * Step 3: prepare the input block that will get compressed.
 *         We just fill it with zeros in this example program,
 *         but you would use your real-world data here.
 */
    in_len = IN_LEN;
    lzo_memset(in, 0, in_len);

/*
 * Step 4: compress from `in' to `out' with LZO1X-1
 */
    r = lzopro_lzo1x_1_14_compress(in, in_len, out, &out_len, wrkmem);
    if (r == LZO_E_OK)
        printf("compressed %lu bytes into %lu bytes\n",
            (unsigned long) in_len, (unsigned long) out_len);
    else
    {
        /* this should NEVER happen */
        printf("internal error - compression failed: %d\n", r);
        return 1;
    }
    /* check for an incompressible block */
    if (out_len >= in_len)
    {
        printf("This block contains incompressible data.\n");
        return 0;
    }

/*
 * Step 5: decompress again, now going from `out' to `in'
 */
    new_len = in_len;
    r = lzopro_lzo1x_decompress(out, out_len, in, &new_len, NULL);
    if (r == LZO_E_OK && new_len == in_len)
        printf("decompressed %lu bytes back into %lu bytes\n",
            (unsigned long) out_len, (unsigned long) in_len);
    else
    {
        /* this should NEVER happen */
        printf("internal error - decompression failed: %d\n", r);
        return 1;
    }

/*
 * Step 6: just a quick example: how to use LZO1X-99/10
 */
#if 1
    {
        lzo_callback_t callbacks;
        int clevel = 10;            /* compression level 1..10 */
        /* setup the malloc/free callbacks */
        callbacks.nalloc = cb_nalloc;
        callbacks.nfree = cb_nfree;
        callbacks.nprogress = (lzo_progress_func_t) 0;
        /* compress */
        r = lzopro_lzo1x_99_compress(in, in_len, out, &out_len, &callbacks, clevel);
        /* check result */
        if (r == LZO_E_OUT_OF_MEMORY)
        {
            printf("out of memory during compression\n");
            exit(1);
        }
        if (r != LZO_E_OK)
        {
            /* this should NEVER happen */
            printf("internal error - compression failed: %d\n", r);
            return 1;
        }
        printf("LZO1X-99/10 compression test passed.\n");
        if (out_len < in_len)
        {
            /* now do something with the compressed data... */
        }
    }
#endif


    lzo_free(wrkmem);
    lzo_free(out);
    lzo_free(in);
    printf("Simple compression test passed.\n");
    return 0;
}

/*
vi:ts=4:et
*/

