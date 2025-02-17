/* copy_lit_impl.ch --

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


/* code  pending literals */

/* lzo_uint t = pd(ip, ii); */

{

#if 1 && defined(LZO_UNALIGNED_OK_4)

    if (t <= 3)
    {
        op[-2] |= LZO_BYTE(t);
        /* copy 4 bytes */
        COPY4(op, ii);
        op += t;
        /* ii += t; */
        ii = ip;
    }
    else
    {
        if (t <= 18)
            *op++ = LZO_BYTE(t - 3);
        else
        {
#if 0
            lzo_uint tt = t + 255 - 18;
            do {
                *op++ = 0;
                tt -= 255;
            } while (tt > 255);
            *op++ = LZO_BYTE(tt);
#else
            t += 255 - 18;
            do {
                *op++ = 0;
                t -= 255;
            } while (t > 255);
            *op++ = LZO_BYTE(t);
#endif
        }
        /* copy 4 bytes */
        do {
            COPY4(op, ii);
            op += 4;
            ii += 4;
        } while (ii < ip);
        op -= (ii - ip);
        ii = ip;
    }

#elif 1

    if (t <= 3)
        op[-2] |= LZO_BYTE(t);
    else if (t <= 18)
        *op++ = LZO_BYTE(t - 3);
    else
    {
        lzo_uint tt = t + 255 - 18;
        do {
            *op++ = 0;
            tt -= 255;
        } while (tt > 255);
        *op++ = LZO_BYTE(tt);
    }
    do *op++ = *ii++; while (--t > 0);

#else

    if (t <= 3)
        op[-2] |= LZO_BYTE(t);
    else if (t <= 18)
        *op++ = LZO_BYTE(t - 3);
    else
    {
        t += 255 - 18;
        do {
            *op++ = 0;
            t -= 255;
        } while (t > 255);
        *op++ = LZO_BYTE(t);
    }
    do *op++ = *ii++; while (ii != ip);

#endif

}


/* vim:set ts=4 et: */
