/* precomp.c -- example program: how to generate pre-compressed data

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
// This program shows how to generate pre-compressed data.
//
// Please study simple.c first.
//
// We will be trying both LZO1X-99 and LZO1Y-99 and choose
// the algorithm that achieves the best compression ratio.
//
// INFO: under Linux/Unix you can also use this program as a
// library self-test driver by running
//   find . -type f -print0 | xargs -0r ./precomp.out -c
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
// pre-compress a file
**************************************************************************/

static int do_file(const char *in_name, const char *out_name, int clevel)
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

    int best_compress_method = -1;
    int last_compress_method = -2;
    lzo_uint best_len;

    lzo_uint32 uncompressed_checksum;
    lzo_uint32 compressed_checksum;

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
    best_len = u_len;


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
 * Step 5a: compress from 'u_buf' to 'c_buf' with LZO1X-99
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
        last_compress_method = M_LZO1X;
#if defined(PARANOID)
        if (c_len < u_len)
            verify_all_decompressors(last_compress_method, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#endif
        if (c_len < best_len)
        {
            best_len = c_len;
            best_compress_method = last_compress_method;
        }
#endif /* USE_LZO1X */


/*
 * Step 5b: compress from 'u_buf' to 'c_buf' with LZO1Y-99
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
        last_compress_method = M_LZO1Y;
#if defined(PARANOID)
        if (c_len < u_len)
            verify_all_decompressors(last_compress_method, c_buf, c_len, u_buf, u_len, uncompressed_checksum);
#endif
        if (c_len < best_len)
        {
            best_len = c_len;
            best_compress_method = last_compress_method;
        }
#endif /* USE_LZO1Y */


/*
 * Step 6: check if compressible
 */

    if (best_len >= u_len)
    {
        printf("This file contains incompressible data.\n");
        goto done;
    }


/*
 * Step 7: compress data again using the best compressor found
 */

    if (last_compress_method != best_compress_method)
    {
        c_len = c_buf_size;
        r = -100;
#ifdef USE_LZO1X
        if (best_compress_method == M_LZO1X)
            r = lzopro_lzo1x_99_compress(u_buf, u_len, c_buf, &c_len, &callbacks, clevel);
#endif
#ifdef USE_LZO1Y
        if (best_compress_method == M_LZO1Y)
            r = lzopro_lzo1y_99_compress(u_buf, u_len, c_buf, &c_len, &callbacks, clevel);
#endif
        if (r == LZO_E_OUT_OF_MEMORY) {
            printf("out of memory during compression\n");
            exit(1);
        }
        assert(r == LZO_E_OK);
    }
    assert(c_len == best_len);


/*
 * Step 8: optimize compressed data (compressed data is in 'c_buf' buffer)
 */

    optimize_compressed_data(best_compress_method, c_buf, c_len, u_buf, u_len);


/*
 * Step 9: compute a checksum of the compressed data
 */

    compressed_checksum = lzo_adler32(0, NULL, 0);
    compressed_checksum = lzo_adler32(compressed_checksum, c_buf, c_len);


/*
 * Step 10: verify decompression (using various decompressors)
 */

    verify_all_decompressors(best_compress_method, c_buf, c_len, u_buf, u_len, uncompressed_checksum);


    /* Now you could also verify decompression under similar conditions
     * as in your application, e.g. overlapping decompression etc.
     */


/*
 * Step 11: write compressed data to a file
 */

    printf("%s: %s: %ld -> %ld, checksum 0x%08lx 0x%08lx\n",
            progname, in_name, (long) u_len, (long) c_len,
            (long) uncompressed_checksum, (long) compressed_checksum);

    if (out_name && out_name[0])
    {
        printf("%s: writing to file %s\n", progname, out_name);
        fp = fopen(out_name, "wb");
        if (fp == NULL)
        {
            printf("%s: cannot open output file %s\n", progname, out_name);
            exit(1);
        }
        /* NOTE: you should add a header and write at least both the uncompressed
         *   and compressed size and checksums before the compressed data */
        if (lzo_fwrite(fp, c_buf, c_len) != c_len || fclose(fp) != 0)
        {
            printf("%s: write error !!\n", progname);
            exit(1);
        }
        fp = NULL;
    }


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
    printf("usage: %s    file [output-file]\n", progname);
    printf("usage: %s -c file...\n", progname);
    exit(1);
}


int __lzo_cdecl_main main(int argc, char *argv[])
{
    int i = 1;
    int opt_compress_only = 0;
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
            opt_compress_only = 1;
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

    if (opt_compress_only)
    {
        int r, files = 0, skipped = 0;
        for ( ; i < argc; i++) {
            r = do_file(argv[i], NULL, opt_clevel);
            if (r == 0)
                files++;
            else
                skipped++;
        }
        printf("All done. Processed %d files, %d files skipped.\n", files, skipped);
    }
    else if (i + 1 == argc)
    {
        do_file(argv[i], NULL, opt_clevel);
    }
    else if (i + 2 == argc)
    {
        do_file(argv[i], argv[i+1], opt_clevel);
    }
    else
    {
        usage();
    }

    return 0;
}


/*
vi:ts=4:et
*/

