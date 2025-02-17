/* plzo1_d_fc.h -- fastcopy

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


#if defined(CONFIG_MODE)
#  if (CONFIG_MODE & 1)
#    define CONFIG_COMPACT_STATES 1
#  endif
#  if (CONFIG_MODE & 2)
#    define CONFIG_PREINC 1
#  endif
#  if (CONFIG_MODE & 4)
#    define CONFIG_ADVANCE_FIRST 1
#  endif
#endif

/*
CONFIG_FAST 1           ATTN: with 3 bytes output overrun !

ip, op, m_pos

out, NEED_IP, NEED_OP, TEST_LB
*/


#if 0
#undef NDEBUG
/*#define NDEBUG*/
#include <assert.h>
#endif

#if !defined(ACC_UA_COPY32)
#  undef CONFIG_FAST
#endif
#include "acc_fccm.h"


/***********************************************************************
// postinc, preinc
************************************************************************/

#if CONFIG_PREINC
#define PINC(ptr)   (*++ptr)
#define POFF        1
#else
#define PINC(ptr)   (*ptr++)
#define POFF        0
#endif

#if 1
/* for 64-bit systems */
#define PIP         ((lzo_uint) PINC(ip))
#define POP         PINC(op)
#define PMP         PINC(m_pos)
#else
#define PIP         PINC(ip)
#define POP         PINC(op)
#define PMP         PINC(m_pos)
#endif
/* for copy loops */
#define PIP_BYTE    PINC(ip)


#if 0
#  error "FIXME - NEED_IP is missing"
#  define P_LOOP_255(ptr,t,add) \
        while (ptr[POFF] == 0) ptr++, t += 255; t += add + PINC(ptr)
#elif 0
#  error "FIXME - NEED_IP is missing"
#  define P_LOOP_255(ptr,t,add) \
        while (PINC(ptr) == 0) t += 255; t += add + ptr[POFF-1]
#else
/* note: 3 is the EOF length */
#  define P_LOOP_255(ptr,t,add) \
        do { t += 255; NEED_IP(1+3); } while (PINC(ptr) == 0); t += add - 255 + ptr[POFF-1]
#endif


/***********************************************************************
// FC_ fastcopy
************************************************************************/

/* note: all __FC and _FC are private! really. */


#define __FC_CTA(e)           ACC_COMPILE_TIME_ASSERT(e)
#define __FC_CTA_NF(n,f)      ACC_COMPILE_TIME_ASSERT(((n + 3) &~ 3) == f)
#define __FC_CTA_UNIT(x)      ACC_COMPILE_TIME_ASSERT(x==1||x==2||x==4||x==8)

#define __FC_TEST_LBO(n,f,unit) __FC_CTA(n > 0); __FC_CTA_UNIT(unit); assert(op - m_pos >= (n < unit ? n : unit)); TEST_LBO(m_pos+POFF, (n < unit ? n : unit) - 1 - POFF)
/*#define __FC_TEST_LBO(n,f,unit) __FC_CTA(n > 0); __FC_CTA_UNIT(unit); TEST_LBO(m_pos+POFF, (n < unit ? n : unit) - 1 - POFF)*/


/* public - checks */

#if 0 && defined(CONFIG_FAST)
/* NOTE: the _asm_fast version does not change the semantics of NEED_OP(),
 * so we also don't do that here */
#  define FC_L_CHECK(n,f,t,tmin)         __FC_CTA_NF(n,f); assert(t >= tmin); NEED_IP(f+t); NEED_OP(f+t)
#  define FC_M_CHECK(n,f,t,tmin,unit)    __FC_CTA_NF(n,f); assert(t >= tmin); __FC_TEST_LBO(n,f,unit); NEED_OP(f+t)
#else
#  define FC_L_CHECK(n,f,t,tmin)         __FC_CTA_NF(n,f); assert(t >= tmin); NEED_IP(n+t); NEED_OP(n+t)
#  define FC_M_CHECK(n,f,t,tmin,unit)    __FC_CTA_NF(n,f); assert(t >= tmin); __FC_TEST_LBO(n,f,unit); NEED_OP(n+t)
#endif


/* __FC -- core layer */

#if !defined(__FC_L)
#if 0
/* debug */
static __lzo_noinline void
fc_debug_bcpy(lzo_bytep d, const lzo_bytep s, lzo_uint n, lzo_uint unit) {
    if (unit) assert(d - s >= (n < unit ? n : unit));
    if (n) do *d++ = *s++; while (--n > 0);
}
#define __FC_L(n,x,y)   fc_debug_bcpy(op+x+POFF, ip+y+POFF, n, 0)
#define __FC_M1(n,x,y)  fc_debug_bcpy(op+x+POFF, m_pos+y+POFF, n, 1)
#define __FC_M2(n,x,y)  fc_debug_bcpy(op+x+POFF, m_pos+y+POFF, n, 2)
#define __FC_M4(n,x,y)  fc_debug_bcpy(op+x+POFF, m_pos+y+POFF, n, 4)
#define __FC_M8(n,x,y)  fc_debug_bcpy(op+x+POFF, m_pos+y+POFF, n, 8)
#elif 1
/* unrolled copy using acc_fccm.h */
#define __FC_L(n,x,y)   ACC_CPP_ECONCAT2(acc_fc_bcpy_ua8_,n)(op,x+POFF,ip,y+POFF)
#define __FC_M1(n,x,y)  ACC_CPP_ECONCAT2(acc_fc_bcpy_,n)(op,x+POFF,m_pos,y+POFF)
#define __FC_M2(n,x,y)  ACC_CPP_ECONCAT2(acc_fc_bcpy_ua2_,n)(op,x+POFF,m_pos,y+POFF)
#define __FC_M4(n,x,y)  ACC_CPP_ECONCAT2(acc_fc_bcpy_ua4_,n)(op,x+POFF,m_pos,y+POFF)
#define __FC_M8(n,x,y)  ACC_CPP_ECONCAT2(acc_fc_bcpy_ua8_,n)(op,x+POFF,m_pos,y+POFF)
#elif 1 && (ACC_ARCH_I386)
/* info: VC71 generates inline memcpy() very much like our unrolled fc_bcpy */
#define __FC_L(n,x,y)   ACC_CPP_ECONCAT2(acc_fc_bcpy_ua8_,n)(op,x+POFF,ip,y+POFF)
#define __FC_M1(n,x,y)  ACC_CPP_ECONCAT2(acc_fc_bcpy_,n)(op,x+POFF,m_pos,y+POFF)
#define __FC_M2(n,x,y)  ACC_CPP_ECONCAT2(acc_fc_bcpy_ua2_,n)(op,x+POFF,m_pos,y+POFF)
#define __FC_M4(n,x,y)  memcpy(op+x+POFF, m_pos+y+POFF, n)
#define __FC_M8(n,x,y)  memcpy(op+x+POFF, m_pos+y+POFF, n)
#else
/* FIXME: VC71 - this does not work ??? */
#define __FC_L(n,x,y)   acc_fc_rep_movsb(op+x+POFF, ip+y+POFF, n)
#define __FC_M1(n,x,y)  acc_fc_rep_movsb(op+x+POFF, m_pos+y+POFF, n)
#define __FC_M2(n,x,y)  acc_fc_rep_movsb(op+x+POFF, m_pos+y+POFF, n)
#define __FC_M4(n,x,y)  acc_fc_rep_movsb(op+x+POFF, m_pos+y+POFF, n)
#endif
#endif /* !defined(__FC_L) */


/* _FC -- advance pointers, handle FAST, and possibly optimize */

/* note: to assert a sane semantics we also advance m_pos - the
 * optimizer should remove unneeded statements anyway. */

#if defined(CONFIG_FAST)

#define _FC_L(n,f)      __FC_CTA_NF(n,f); __FC_L(f,0,0); op += n; ip += n
#if 1 && defined(ACC_UA_COPY32)
#define _FC_M1(n,f)     __FC_CTA_NF(n,f); if (op - m_pos >= 4) { __FC_M4(f,0,0); } else { __FC_M1(n,0,0); } op += n
#define _FC_M2(n,f)     __FC_CTA_NF(n,f); if (op - m_pos >= 4) { __FC_M4(f,0,0); } else { __FC_M2(n,0,0); } op += n
#else
#define _FC_M1(n,f)     __FC_CTA_NF(n,f); __FC_M1(f,0,0); op += n
#define _FC_M2(n,f)     __FC_CTA_NF(n,f); __FC_M2(f,0,0); op += n
#endif
#define _FC_M4(n,f)     __FC_CTA_NF(n,f); __FC_M4(f,0,0); op += n
#define _FC_M8(n,f)     __FC_CTA_NF(n,f); __FC_M8(f,0,0); op += n

#define _FC_M_ADVANCE_MPOS(n) m_pos += n


#elif defined(CONFIG_ADVANCE_FIRST)

/* advance first */
/* this allows the compiler to join the bcpy tails -> somewhat smaller
 * code, but also a little bit slower (because of the jmp needed to join)
 */

#define _FC_L(n,f)      __FC_CTA_NF(n,f); op += n; ip += n; __FC_L(n,-n,-n)
#if 1 && defined(ACC_UA_COPY32)
#define _FC_M1(n,f)     __FC_CTA_NF(n,f); op += n; m_pos += n; if (op - m_pos >= 4) { __FC_M4(n,-n,-n); } else { __FC_M1(n,-n,-n); }
#define _FC_M2(n,f)     __FC_CTA_NF(n,f); op += n; m_pos += n; if (op - m_pos >= 4) { __FC_M4(n,-n,-n); } else { __FC_M2(n,-n,-n); }
#else
#define _FC_M1(n,f)     __FC_CTA_NF(n,f); op += n; m_pos += n; __FC_M1(n,-n,-n)
#define _FC_M2(n,f)     __FC_CTA_NF(n,f); op += n; m_pos += n; __FC_M2(n,-n,-n)
#endif
#define _FC_M4(n,f)     __FC_CTA_NF(n,f); op += n; m_pos += n; __FC_M4(n,-n,-n)
#define _FC_M8(n,f)     __FC_CTA_NF(n,f); op += n; m_pos += n; __FC_M8(n,-n,-n)

#define _FC_M_ADVANCE_MPOS(n)


#else

#define _FC_L(n,f)      __FC_CTA_NF(n,f); __FC_L(n,0,0); op += n; ip += n
#if 1 && defined(ACC_UA_COPY32)
#define _FC_M1(n,f)     __FC_CTA_NF(n,f); if (op - m_pos >= 4) { __FC_M4(n,0,0); } else { __FC_M1(n,0,0); } op += n
#define _FC_M2(n,f)     __FC_CTA_NF(n,f); if (op - m_pos >= 4) { __FC_M4(n,0,0); } else { __FC_M2(n,0,0); } op += n
#else
#define _FC_M1(n,f)     __FC_CTA_NF(n,f); __FC_M1(n,0,0); op += n
#define _FC_M2(n,f)     __FC_CTA_NF(n,f); __FC_M2(n,0,0); op += n
#endif
#define _FC_M4(n,f)     __FC_CTA_NF(n,f); __FC_M4(n,0,0); op += n
#define _FC_M8(n,f)     __FC_CTA_NF(n,f); __FC_M8(n,0,0); op += n

#define _FC_M_ADVANCE_MPOS(n) m_pos += n

#endif


/* FC -- public, with checks */

#if 1

#define FC_L(n,f)       FC_L_CHECK(n,f,0,0);   _FC_L(n,f)
#define FC_M1(n,f)      FC_M_CHECK(n,f,0,0,1); _FC_M1(n,f)
#define FC_M2(n,f)      FC_M_CHECK(n,f,0,0,2); _FC_M2(n,f)
#define FC_M4(n,f)      FC_M_CHECK(n,f,0,0,4); _FC_M4(n,f)
#define FC_M8(n,f)      FC_M_CHECK(n,f,0,0,8); _FC_M8(n,f)

#else

/* just a test if smaller code sizes works better (small icache) */

#define _fc_cpy_pip(n) \
    { lzo_bytep xop = op + n; do POP = PIP_BYTE; while (op != xop); }
#define _fc_cpy_pmp(n) \
    { lzo_bytep xop = op + n; do POP = PMP; while (op != xop); }

#define FC_L(n,f)       FC_L_CHECK(n,f,0,0);   if (n >= 5) { _fc_cpy_pip(n); } else {_FC_L(n,f); }
#define FC_M1(n,f)      FC_M_CHECK(n,f,0,0,1); if (n >= 5) { _fc_cpy_pmp(n); } else {_FC_M1(n,f); }
#define FC_M2(n,f)      FC_M_CHECK(n,f,0,0,2); if (n >= 5) { _fc_cpy_pmp(n); } else {_FC_M2(n,f); }
#define FC_M4(n,f)      FC_M_CHECK(n,f,0,0,4); if (n >= 5) { _fc_cpy_pmp(n); } else {_FC_M4(n,f); }
#define FC_M8(n,f)      FC_M_CHECK(n,f,0,0,8); if (n >= 5) { _fc_cpy_pmp(n); } else {_FC_M8(n,f); }

#endif


/***********************************************************************
// FC_T1_
************************************************************************/

/*
 * Mx means a we use a copy-unit of x bytes
 * Tx means that t >= x
 */


#if 1 && defined(ACC_UA_COPY32)

#define _FC_T1_L(n,f,t) \
    _FC_L(n,f); \
    if (t >= 4) do { \
        __FC_L(4,0,0); op += 4; ip += 4; t -= 4; \
    } while (t >= 4); \
    if (t) do POP = PIP_BYTE; while (--t > 0);

#define _FC_T1_M1(n,f,t) \
    if (op - m_pos >= 4) { \
        _FC_T1_M4(n,f,t); \
    } else { \
        __FC_M1(n,0,0); op += n; m_pos += n; \
        do POP = PMP; while (--t > 0); \
    }

#define _FC_T1_M4(n,f,t) \
    __FC_M4(n,0,0); op += n; m_pos += n; \
    if (t >= 4) do { \
        __FC_M4(4,0,0); op += 4; m_pos += 4; t -= 4; \
    } while (t >= 4); \
    if (t) do POP = PMP; while (--t > 0)

/* note: not all are here, use _M1 and _M4 instead */
#define _FC_T1_M2(n,f,t)     _FC_T1_M1(n,f,t)
#define _FC_T1_M8(n,f,t)     _FC_T1_M4(n,f,t)

#else

#define _FC_T1_L(n,f,t) \
    _FC_L(n,f); do POP = PIP_BYTE; while (--t > 0)

#define _FC_T1_M1(n,f,t) \
    _FC_M1(n,f); _FC_M_ADVANCE_MPOS(n); do POP = PMP; while (--t > 0)

#define _FC_T1_M2(n,f,t) \
    _FC_M2(n,f); _FC_M_ADVANCE_MPOS(n); do POP = PMP; while (--t > 0)

#define _FC_T1_M4(n,f,t) \
    _FC_M4(n,f); _FC_M_ADVANCE_MPOS(n); do POP = PMP; while (--t > 0)

#define _FC_T1_M8(n,f,t) \
    _FC_M8(n,f); _FC_M_ADVANCE_MPOS(n); do POP = PMP; while (--t > 0)

#endif



/* FC -- public, with checks */

#define FC_T1_L(n,f,t)  FC_L_CHECK(n,f,t,1);   _FC_T1_L(n,f,t)
#define FC_T1_M1(n,f,t) FC_M_CHECK(n,f,t,1,1); _FC_T1_M1(n,f,t)
#define FC_T1_M2(n,f,t) FC_M_CHECK(n,f,t,1,2); _FC_T1_M2(n,f,t)
#define FC_T1_M4(n,f,t) FC_M_CHECK(n,f,t,1,4); _FC_T1_M4(n,f,t)
#define FC_T1_M8(n,f,t) FC_M_CHECK(n,f,t,1,8); _FC_T1_M8(n,f,t)



/*
vi:ts=4:et
*/

