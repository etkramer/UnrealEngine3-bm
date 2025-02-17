/* plzo1x_99.c -- LZO1[XY]-99 compression

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


#define LZO_CFG_FREESTANDING 1
#define LZO_CFG_NO_UNALIGNED 1
#define LZO_ABI_NEUTRAL_ENDIAN 1
/* #define LZO_ARCH_GENERIC 1 */ /* disabled, as this may break acc_szof.h */
#include "lzo_conf.h"
#include "lzo/lzopro/lzo1x.h"
#include "lzo/lzopro/lzo1y.h"


#if (LZO_CC_MSC)
#  pragma warning(disable: 4100)    /* Wx: 'xxx' : unreferenced formal parameter */
#  pragma warning(disable: 4146)    /* Wx: unary minus operator applied to unsigned type, result still unsigned */
#  pragma warning(disable: 4255)    /* Wx: 'xxx' : no function prototype given: converting '()' to '(void)' */
#  if ((_MSC_VER >= 1200) && (_MSC_VER < 1300))
#    pragma warning(disable: 4702)  /* W4: unreachable code */
#  endif
#endif


#define plzo_external_ptrdiff_uchar(a,b,c)      ((plzo_ptrdiff_t)((a)-(b)))
#define plzo_external_ptrdiff_M(a,b,c)          ((plzo_ptrdiff_t)((a)-(b)))


#if (LZO_ABI_ILP32 || LZO_ABI_IP32L64)
#  define plzo_int32_t              int
#  define plzo_uint32_t             unsigned int
#  define plzo_ptrdiff_t            plzo_int32_t
#  include "pcbest32.ch"
#  define plzo_abi_uint_t           plzo_uint32_t
#  define plzo_cb_uint_t            plzo_uint32_t
#elif (LZO_ABI_LLP64)
#  define plzo_int32_t              int
#  define plzo_uint32_t             unsigned int
#  define plzo_int64_t              long long
#  define plzo_uint64_t             unsigned long long
#  define PLZO_INT64_C(c)           c##LL
#  define PLZO_UINT64_C(c)          c##ULL
#  define plzo_ptrdiff_t            plzo_int64_t
#  include "pcbest64.ch"
#  define plzo_abi_uint_t           plzo_uint32_t
#  define plzo_cb_uint_t            plzo_uint64_t
#elif (LZO_ABI_LP64)
   /* info: we prefer long over long-long because of portability issues */
#  define plzo_int32_t              int
#  define plzo_uint32_t             unsigned int
#  define plzo_int64_t              long
#  define plzo_uint64_t             unsigned long
#  define PLZO_INT64_C(c)           c##L
#  define PLZO_UINT64_C(c)          c##UL
#  define plzo_ptrdiff_t            plzo_int64_t
#  include "pcbest64.ch"
#  define plzo_abi_uint_t           plzo_uint32_t
#  define plzo_cb_uint_t            plzo_uint64_t
#elif (LZO_ABI_ILP64)
   /* info: we prefer long over int because of Cray PVP fastmd issues */
#  define plzo_int64_t              long
#  define plzo_uint64_t             unsigned long
#  define PLZO_INT64_C(c)           c##L
#  define PLZO_UINT64_C(c)          c##UL
#  define plzo_ptrdiff_t            plzo_int64_t
#  include "pcbest64i.ch"
#  define plzo_abi_uint_t           plzo_uint64_t
#  define plzo_cb_uint_t            plzo_uint64_t
#else
#  error "programming model not supported"
#endif


LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_uint) == sizeof(plzo_cb_uint_t))
    /* TODO: could add more assertions here */
#undef plzo_cb_uint_t
#define plzo_cb_uint_t              lzo_uint

static lzo_voidp __LZO_CDECL
wrap_nalloc(lzo_callback_p self, plzo_cb_uint_t items, plzo_cb_uint_t size)
{
    lzo_callback_p cb = (lzo_callback_p) self->user1;
    return cb->nalloc(cb, items, size);
}

static void __LZO_CDECL
wrap_nfree(lzo_callback_p self, lzo_voidp ptr)
{
    lzo_callback_p cb = (lzo_callback_p) self->user1;
    cb->nfree(cb, ptr);
}

static void __LZO_CDECL
wrap_nprogress(lzo_callback_p self, plzo_cb_uint_t ulen, plzo_cb_uint_t clen, int state)
{
    lzo_callback_p cb = (lzo_callback_p) self->user1;
    if (!cb->nprogress)
        return;
    if (state != 1 && state != 2)
        cb->nprogress(cb, ulen, clen, state);
}

#undef plzo_cb_uint_t



#define LZO1X 1
#include "plzo1x_99.ch"
#undef LZO1X

#define LZO1Y 1
#define lzopro_lzo1x_99_compress            lzopro_lzo1y_99_compress
#define plzo_lzo1x_99_compress_internal     plzo_lzo1y_99_compress_internal
#include "plzo1x_99.ch"
#undef lzopro_lzo1x_99_compress
#undef plzo_lzo1x_99_compress_internal
#undef LZO1Y


/*
vi:ts=4:et
*/
