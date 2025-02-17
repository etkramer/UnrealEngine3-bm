/* selftest.c -- example program: comprehensive self-test

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
// Usage example for Linux, Unix & cygwin:
//   find . -type f -print0 | xargs -0r ./selftest.out
**************************************************************************/

#include "lzo/lzoconf.h"
#include "lzo/lzopro/lzo1x.h"
#include "lzo/lzopro/lzo1y.h"
#include "lzo/lzo1x.h"
#include "lzo/lzo1y.h"

/* portability layer */
#define WANT_LZO_MALLOC 1
#define WANT_LZO_FREAD 1
#define WANT_LZO_WILDARGV 1
#include "examples/portab.h"


#define USE_LZO1X 1
#define USE_LZO1Y 1
enum { M_LZO1X, M_LZO1Y };

#define PARANOID 1      /* enable a _lot_ of paranoid self-tests */


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

/* progress indicator callback */
static void __LZO_CDECL
cb_nprogress(lzo_callback_p self, lzo_uint u_len, lzo_uint c_len, int state)
{
    LZO_UNUSED(self); LZO_UNUSED(u_len); LZO_UNUSED(c_len); LZO_UNUSED(state);
#if 0
    printf("  progress: %9lu %9lu %3d\n", (long)u_len, (long)c_len, state);
    fflush(stdout);
#endif
}


static const char *progname = NULL;
static long opt_max_file_size = 64ul * 1024 * 1024;
static int opt_fast_only = 0;


/*************************************************************************
// decompression
**************************************************************************/

static void verify_decompression(
        lzo_decompress_t decompress_func,
        const lzo_bytep c_buf, lzo_uint c_len,
              lzo_bytep u_buf, lzo_uint u_len,
        lzo_uint32 uncompressed_checksum)
{
    int r;
    lzo_uint new_len;
    lzo_uint32 decompressed_checksum;

    assert(c_len < u_len);

#if defined(PARANOID)
    lzo_memset(u_buf, 0, u_len);    /* clear output buffer */
#endif

    new_len = u_len;
    r = decompress_func(c_buf, c_len, u_buf, &new_len, NULL);
    if (r != LZO_E_OK || new_len != u_len)
    {
        /* this should NEVER happen */
        printf("internal error - decompression failed: %d %lu %lu %lu\n", r,
            (unsigned long)c_len, (unsigned long) u_len, (unsigned long) new_len);
        exit(1);
    }
    decompressed_checksum = lzo_adler32(0, NULL, 0);
    decompressed_checksum = lzo_adler32(decompressed_checksum, u_buf, u_len);
    if (uncompressed_checksum != decompressed_checksum)
    {
        /* this should NEVER happen */
        printf("internal error - decompression data error 0x%08lx 0x%08lx %lu %lu\n",
            (long)uncompressed_checksum, (long) decompressed_checksum,
            (unsigned long) c_len, (unsigned long) u_len);
        exit(1);
    }
}


static void verify_safe_decompression(
        lzo_decompress_t decompress_func,
        const lzo_uint *sbuf,
        const lzo_bytep c_buf, lzo_uint c_len,
              lzo_bytep u_buf, lzo_uint u_len,
        lzo_uint32 uncompressed_checksum)
{
    int r;
    lzo_uint shorten;
    lzo_uint new_len;

    assert(c_len < u_len);

    while ((shorten = *sbuf++) != 0)
    {
        /*
         * Verify safe decompression failure on short input buffer.
         * NOTE: we expect a failure here!
         */
        if (c_len >= shorten)
        {
            new_len = u_len;
            /* c_len - shorten => short input buffer */
            r = decompress_func(c_buf, c_len - shorten, u_buf, &new_len, NULL);
            if (r == LZO_E_OK || new_len > u_len)
            {
                /* this should NEVER happen */
                printf("internal error - safe decompression did not catch short input buffer\n");
                exit(1);
            }
        }

        /*
         * Verify safe decompression failure on short output buffer
         * NOTE: we expect a failure here!
         */
        if (u_len >= shorten)
        {
            new_len = u_len - shorten;  /* => u_len - shorten => short output buffer */
            r = decompress_func(c_buf, c_len, u_buf, &new_len, NULL);
            if (r == LZO_E_OK || new_len > u_len - shorten)
            {
                /* this should NEVER happen */
                printf("internal error - safe decompression did not catch short output buffer\n");
                exit(1);
            }
        }
    }

    /* Finally restore the original 'u_buf'. */
    verify_decompression(decompress_func, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
}


static void verify_all_decompressors(
        int method,
        const lzo_bytep c_buf, lzo_uint c_len,
              lzo_bytep u_buf, lzo_uint u_len,
        lzo_uint32 uncompressed_checksum)
{
    lzo_uint sbuf[9+1] = { 1, 2, 3, 4, 5, 0, 0, 0, 0, 0 };
    sbuf[5] = c_len / 3 * 2;
    sbuf[6] = c_len / 2;
    sbuf[7] = c_len / 3;
    sbuf[8] = c_len;

    assert(c_len < u_len);

#ifdef USE_LZO1X
    if (method == M_LZO1X)
    {
#  if 1
        verify_decompression(lzopro_lzo1x_decompress_safe, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1x_decompress,      c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#    if 1 && defined(PARANOID) && !defined(LZO_BUILD_DLL)
        verify_decompression(lzopro_lzo1x_decompress_a01,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1x_decompress_a02,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1x_decompress_a03,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1x_decompress_a04,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1x_decompress_a05,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1x_decompress_a06,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1x_decompress_a07,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#    endif
        verify_safe_decompression(lzopro_lzo1x_decompress_safe, sbuf, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#  endif
#  if 1
        verify_decompression(lzo1x_decompress_safe, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzo1x_decompress,      c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_safe_decompression(lzo1x_decompress_safe, sbuf, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#  endif
        return;
    }
#endif
#ifdef USE_LZO1Y
    if (method == M_LZO1Y)
    {
#  if 1
        verify_decompression(lzopro_lzo1y_decompress_safe, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1y_decompress,      c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#    if 1 && defined(PARANOID) && !defined(LZO_BUILD_DLL)
        verify_decompression(lzopro_lzo1y_decompress_a01,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1y_decompress_a02,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1y_decompress_a03,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1y_decompress_a04,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1y_decompress_a05,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1y_decompress_a06,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzopro_lzo1y_decompress_a07,  c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#    endif
        verify_safe_decompression(lzopro_lzo1y_decompress_safe, sbuf, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#  endif
#  if 1
        verify_decompression(lzo1y_decompress_safe, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_decompression(lzo1y_decompress,      c_buf, c_len, u_buf, u_len, uncompressed_checksum);
        verify_safe_decompression(lzo1y_decompress_safe, sbuf, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#  endif
        return;
    }
#endif

    /* this should NEVER happen */
    printf("internal error - unknown compression method %d\n", method);
    exit(1);
}


/*************************************************************************
// optimize compressed data representation
**************************************************************************/

static void optimize_compressed_data(
        int method,
        lzo_bytep c_buf, lzo_uint c_len,
        lzo_bytep u_buf, lzo_uint u_len)
{
    int r;
    lzo_uint new_len;

    assert(c_len < u_len);

#if defined(PARANOID)
    /* Optimization does not require any data in the buffer that will
     * hold the uncompressed data. To prove this, we clear the buffer.
     */
    lzo_memset(u_buf, 0, u_len);
#endif

    new_len = u_len;
    r = -100;
#ifdef USE_LZO1X
    if (method == M_LZO1X)
        r = lzo1x_optimize(c_buf, c_len, u_buf, &new_len, NULL);
#endif
#ifdef USE_LZO1Y
    if (method == M_LZO1Y)
        r = lzo1y_optimize(c_buf, c_len, u_buf, &new_len, NULL);
#endif
    if (r != LZO_E_OK || new_len != u_len)
    {
        /* this should NEVER happen */
        printf("internal error - optimization failed: %d\n", r);
        exit(1);
    }
}


/*************************************************************************
// fast compression
**************************************************************************/

#if defined(PARANOID)

static lzo_uint compress_fast(
        int method, lzo_compress_t compress_func,
        lzo_bytep u_buf, lzo_uint u_len,
        lzo_bytep c_buf, lzo_uint c_buf_size,
        lzo_voidp wrkmem,
        lzo_uint32 uncompressed_checksum)
{
    int r;
    lzo_uint c_len = c_buf_size;
    r = compress_func(u_buf, u_len, c_buf, &c_len, wrkmem);
    if (r != LZO_E_OK || c_len > c_buf_size)
    {
        /* this should NEVER happen */
        printf("internal error - fast compression failed: %d %ld\n", r, (long) c_len);
        exit(1);
    }
    if (c_len < u_len)
    {
        verify_all_decompressors(method, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#if defined(PARANOID)
        optimize_compressed_data(method, c_buf, c_len, u_buf, u_len);
        verify_all_decompressors(method, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#endif
    }
    return c_len;
}


static void try_all_fast_compressors(
        int method,
        lzo_bytep u_buf, lzo_uint u_len,
        lzo_bytep c_buf, lzo_uint c_buf_size,
        lzo_voidp wrkmem,
        lzo_uint32 uncompressed_checksum)
{
#ifdef USE_LZO1X
    if (method == M_LZO1X)
    {
        lzo_uint c_pro_1_14 = 0, c_lzo_1_14 = 0;
        LZO_UNUSED(c_pro_1_14); LZO_UNUSED(c_lzo_1_14);
#  if 1
        compress_fast(method, lzopro_lzo1x_1_06_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_07_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_08_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_09_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_10_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_11_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_12_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_13_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        c_pro_1_14 = compress_fast(method, lzopro_lzo1x_1_14_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_15_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1x_1_16_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
#  endif
#  if 1
        compress_fast(method, lzo1x_1_12_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        c_lzo_1_14 = compress_fast(method, lzo1x_1_compress,    u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
#  endif
#  if 0
        if (u_len >= 256 && c_pro_1_14 > 0 && c_lzo_1_14 > 0) {
            double pperc = (double) c_pro_1_14 / (double) u_len * 100.0;
            double lperc = (double) c_lzo_1_14 / (double) u_len * 100.0;
            if (pperc - lperc >= 2.0) {
                printf("    DEC : %10lu => %10lu %10lu   %6.3f   %6.3f   %6.3f\n", (long) u_len, (long) c_pro_1_14, (long) c_lzo_1_14, pperc, lperc, pperc - lperc);
            }
            else if (lperc - pperc >= 2.0) {
                printf("    INC : %10lu => %10lu %10lu   %6.3f   %6.3f   %6.3f\n", (long) u_len, (long) c_pro_1_14, (long) c_lzo_1_14, pperc, lperc, pperc - lperc);
            }
        }
#  endif
        return;
    }
#endif
#ifdef USE_LZO1Y
    if (method == M_LZO1Y)
    {
#  if 1
        compress_fast(method, lzopro_lzo1y_1_06_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_07_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_08_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_09_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_10_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_11_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_12_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_13_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_14_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_15_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
        compress_fast(method, lzopro_lzo1y_1_16_compress, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
#  endif
#  if 1
        compress_fast(method, lzo1y_1_compress,    u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
#  endif
        return;
    }
#endif
    /* this should NEVER happen */
    printf("internal error - unknown compression method %d\n", method);
    exit(1);
}

#endif /* defined(PARANOID) */


/*************************************************************************
// compress a file
**************************************************************************/

static int do_file(const char *in_name, int clevel)
{
    int ret = -1;
    int r;

    lzo_callback_t callbacks;

    /* uncompressed buffer */
    lzo_bytep u_buf = NULL;
    lzo_uint u_len = 0;
    /* compressed buffer */
    lzo_bytep c_buf = NULL;
    lzo_uint c_buf_size = 0;
    lzo_uint c_len = 0;

    lzo_uint32 uncompressed_checksum;

    FILE *fp = NULL;
    long l;


/*
 * Step 1: setup the malloc/free callbacks
 */

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.nalloc = cb_nalloc;
    callbacks.nfree = cb_nfree;
    callbacks.nprogress = (lzo_progress_func_t) 0;
    callbacks.nprogress = cb_nprogress;     /* optional progress callback */


/*
 * Step 2: open the input file
 */

    fp = fopen(in_name, "rb");
    if (fp == NULL)
    {
        printf("%s: cannot open file %s -- skipped\n", progname, in_name);
        goto done;
    }
    fseek(fp, 0, SEEK_END);
    l = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (l <= 0)
    {
empty_file:
        printf("%s: %s: empty file -- skipped\n", progname, in_name);
        goto done;
    }
    if (l > opt_max_file_size) {
        printf("%s: %s: file is too large (%ld) -- skipped\n", progname, in_name, l);
        goto done;
    }

    u_len = (lzo_uint) l;
    /* setup 'c_buf_size' for worst-case expansion during compression */
    c_buf_size = u_len + u_len / 16 + 64 + 3;


/*
 * Step 3: allocate compression buffers and read the file
 */

    u_buf = (lzo_bytep) lzo_malloc(u_len);
    c_buf = (lzo_bytep) lzo_malloc(c_buf_size);
    if (u_buf == NULL || c_buf == NULL)
    {
        printf("%s: out of memory\n", progname);
        exit(1);
    }
    u_len = (lzo_uint) lzo_fread(fp, u_buf, u_len);
    if (u_len == 0)
        goto empty_file;
    printf("%s: loaded file %s: %ld bytes\n", progname, in_name, (long) u_len);
    fclose(fp); fp = NULL;


/*
 * Step 4: compute a checksum of the uncompressed data
 */

    uncompressed_checksum = lzo_adler32(0, NULL, 0);
    uncompressed_checksum = lzo_adler32(uncompressed_checksum, u_buf, u_len);


/*
 * Step 5: first, try all fast compressors
 */

    {
        lzo_voidp wrkmem;
        wrkmem = (lzo_voidp) lzo_malloc(LZOPRO_LZO1X_1_16_MEM_COMPRESS);
        assert(wrkmem != NULL);
#ifdef USE_LZO1X
        try_all_fast_compressors(M_LZO1X, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
#endif
#ifdef USE_LZO1Y
        try_all_fast_compressors(M_LZO1Y, u_buf, u_len, c_buf, c_buf_size, wrkmem, uncompressed_checksum);
#endif
        lzo_free(wrkmem);
    }

    if (opt_fast_only)
    {
        ret = 0;
        goto done;
    }


/*
 * Step 6a: compress from 'u_buf' to 'c_buf' with LZO1X-99
 */

#ifdef USE_LZO1X
        c_len = c_buf_size;
        r = lzopro_lzo1x_99_compress(u_buf, u_len, c_buf, &c_len, &callbacks, clevel);
        if (r != LZO_E_OK)
        {
            if (r == LZO_E_OUT_OF_MEMORY) {
                printf("out of memory during compression\n");
                exit(1);
            }
            /* this should NEVER happen */
            printf("internal error - compression failed: %d\n", r);
            exit(1);
        }
        printf("LZO1X-99/%d: %9lu -> %9lu\n", clevel, (unsigned long) u_len, (unsigned long) c_len);
        if (c_len < u_len)
        {
            verify_all_decompressors(M_LZO1X, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#  if defined(PARANOID)
            optimize_compressed_data(M_LZO1X, c_buf, c_len, u_buf, u_len);
            verify_all_decompressors(M_LZO1X, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#  endif
        }
#endif /* USE_LZO1X */


/*
 * Step 6b: compress from 'u_buf' to 'c_buf' with LZO1Y-99
 */

#ifdef USE_LZO1Y
        c_len = c_buf_size;
        r = lzopro_lzo1y_99_compress(u_buf, u_len, c_buf, &c_len, &callbacks, clevel);
        if (r != LZO_E_OK)
        {
            if (r == LZO_E_OUT_OF_MEMORY) {
                printf("out of memory during compression\n");
                exit(1);
            }
            /* this should NEVER happen */
            printf("internal error - compression failed: %d\n", r);
            exit(1);
        }
        printf("LZO1Y-99/%d: %9lu -> %9lu\n", clevel, (unsigned long) u_len, (unsigned long) c_len);
        if (c_len < u_len)
        {
            verify_all_decompressors(M_LZO1Y, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#  if defined(PARANOID)
            optimize_compressed_data(M_LZO1Y, c_buf, c_len, u_buf, u_len);
            verify_all_decompressors(M_LZO1Y, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#  endif
        }
#endif /* USE_LZO1Y */


    ret = 0;
done:
    if (fp) fclose(fp);
    if (u_buf) lzo_free(u_buf);
    if (c_buf) lzo_free(c_buf);
    return ret;
}


/*************************************************************************
//
**************************************************************************/

static void usage(void)
{
    printf("usage: %s file...\n", progname);
    exit(1);
}


int __lzo_cdecl_main main(int argc, char *argv[])
{
    int i = 1;
    int opt_clevel = 10;            /* compression level 1..10 */

    lzo_wildargv(&argc, &argv);
    progname = argv[0];

    printf("\nLZO-Professional real-time data compression library (v%s, %s).\n",
           lzo_version_string(), lzo_version_date());
    printf("Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer\nAll Rights Reserved.\n\n");


/*
 * Step 1: initialize the LZO library
 */
    if (lzo_init() != LZO_E_OK)
    {
        printf("internal error - lzo_init() failed !!!\n");
        printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics)\n");
        exit(1);
    }


/*
 * Step 2: get options
 */

    while (i < argc && argv[i][0] == '-')
    {
        if (!strcmp(argv[i], "-c"))
            ; /* ignored for compatiblity with precomp.c */
        else if (!strcmp(argv[i], "-f"))
            opt_fast_only = 1;
        else if (!strcmp(argv[i], "-5"))
            opt_clevel = 5;
        else if (!strcmp(argv[i], "-6"))
            opt_clevel = 6;
        else if (!strcmp(argv[i], "-7"))
            opt_clevel = 7;
        else if (!strcmp(argv[i], "-8"))
            opt_clevel = 8;
        else if (!strcmp(argv[i], "-9"))
            opt_clevel = 9;
        else if (!strcmp(argv[i], "--10") || !strcmp(argv[i], "--best"))
            opt_clevel = 10;
        else
            usage();
        i++;
    }
    if (i >= argc)
        usage();


/*
 * Step 3: process args
 */

    {
        int r, files = 0, skipped = 0;
        for ( ; i < argc; i++) {
            r = do_file(argv[i], opt_clevel);
            if (r == 0)
                files++;
            else
                skipped++;
        }
        printf("All done. Processed %d files, %d files skipped.\n", files, skipped);
    }

    return 0;
}


/*
vi:ts=4:et
*/

