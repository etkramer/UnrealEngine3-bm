/* plzo1x_99.ch -- LZO1[XY]-99 compression

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


LZO_PUBLIC(int)
lzopro_lzo1x_99_compress       ( const lzo_bytep src, lzo_uint  src_len,
                                       lzo_bytep dst, lzo_uintp dst_len,
                                       lzo_callback_p cb,
                                       int compression_level )
{
    int r;
    plzo_abi_uint_t d;
    union { const lzo_bytep cp; lzo_bytep p; } u_src; /* UNCONST */
    lzo_callback_t wrap_cb;

    if (src_len >= 0x3fffffffUL)
        return LZO_E_INVALID_ARGUMENT;
    if (!cb || !cb->nalloc || !cb->nfree)
        return LZO_E_INVALID_ARGUMENT;

    memset(&wrap_cb, 0, sizeof(wrap_cb));
    wrap_cb.nalloc = wrap_nalloc;
    wrap_cb.nfree = wrap_nfree;
    if (cb->nprogress)
        wrap_cb.nprogress = wrap_nprogress;
    wrap_cb.user1 = cb;

    u_src.cp = src;
    d = (plzo_abi_uint_t) *dst_len;
    r = plzo_lzo1x_99_compress_internal(u_src.p, (plzo_abi_uint_t) src_len, dst, &d,
                (plzo_callback_t *) (void *) &wrap_cb,
                compression_level, NULL, NULL);
    *dst_len = (lzo_uint) d;

    if (r != 0)
    {
        if (r == -2)
            r = LZO_E_INVALID_ARGUMENT;
        else if (r == -3)
            r = LZO_E_OUT_OF_MEMORY;
        else if (r == -101)
            r = LZO_E_NOT_COMPRESSIBLE;
        else
            r = LZO_E_ERROR;
    }
    return r;
}


/*
vi:ts=4:et
*/
