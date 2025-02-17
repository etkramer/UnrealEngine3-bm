/* plzo_dict.h --

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


/***********************************************************************
// just for experimenting
************************************************************************/

#if 1
#undef DX2
#undef DX3
#define DX2(p,s1,s2) \
        ((lzo_xint)(p)[0] ^ ((lzo_xint)(p)[1] << (s1)) ^ ((lzo_xint)(p)[2] << ((s1)+(s2))))
#define DX3(p,s1,s2,s3) \
        ((lzo_xint)(p)[0] ^ ((lzo_xint)(p)[1] << (s1)) ^ ((lzo_xint)(p)[2] << ((s1)+(s2))) ^ ((lzo_xint)(p)[3] << ((s1)+(s2)+(s3))))
#endif


#if 0

#if 0
#undef D_INDEX2
//#define D_INDEX2(d,p)       d = d ^ (D_HIGH | 0x1f)
#define D_INDEX2(d,p)       d = (d & (D_MASK >> 3)) ^ (D_HIGH | (0x1f & (D_MASK >> 3)))
#endif

#if 1 && (D_BITS <= (12 + 1))
#undef D_INDEX2
#define D_INDEX2(d,p)       d = d ^ (D_HIGH | 0xf)
#endif

#if 1 && (D_BITS <= (12 + 1))
#undef D_INDEX2
#define D_INDEX2(d,p)       d = d ^ (p[3] & D_MASK)
#endif

#endif


/***********************************************************************
// dictionary type
// dictionary updating
************************************************************************/

#undef lzo_dict_t
#undef DENTRY
#undef GINDEX

/* these are unused here */
#if (DD_BITS != 0)
#  error
#endif
#undef UPDATE_D
#undef UPDATE_P


#if (LZO_MM_PVP)
   /* special handling */
#  define lzo_dict_t        unsigned char
#  undef UPDATE_I
#  define UPDATE_I(dict,drun,index,p,in) \
        dict[2*(index)+0] = (unsigned char)  ((lzo_uintptr_t)(p) >> 5); \
        dict[2*(index)+1] = (unsigned char) (((lzo_uintptr_t)(p) << 3) | ((lzo_uintptr_t)(p) >> 61))
#  define GINDEX(m_pos,m_off,dict,dindex,in) \
        m_off  = ((lzo_uintptr_t)(ip) << 3) | ((lzo_uintptr_t)(ip) >> 61); \
        m_off -= (dict[2*(dindex)+0]<<8) + (dict[2*(dindex)+1]); \
        m_off &= 0xffff

#elif 0
   /* this is for archs which do not have acc_uint16e_t */
#  define lzo_dict_t        unsigned char
#  undef UPDATE_I
#  define UPDATE_I(dict,drun,index,p,in) \
        dict[2*(index)+0] = (unsigned char) ((lzo_uintptr_t)(p) >> 8); \
        dict[2*(index)+1] = (unsigned char) ((lzo_uintptr_t)(p))
#  define GINDEX(m_pos,m_off,dict,dindex,in) \
        m_off  = ((lzo_uint) (lzo_uintptr_t) (ip)); \
        m_off -= (dict[2*(dindex)+0]<<8) + (dict[2*(dindex)+1]); \
        m_off &= 0xffff

#else
#  define lzo_dict_t        acc_uint16e_t
#  define DENTRY(p,in)      ((lzo_dict_t) (lzo_uintptr_t) (p))
#  define GINDEX(m_pos,m_off,dict,dindex,in) \
        m_off = (DENTRY(ip,in) - dict[dindex]) & 0xffff

#endif


/***********************************************************************
// test for a match
************************************************************************/

#undef LZO_CHECK_MPOS_DET
#undef LZO_CHECK_MPOS_NON_DET

#define LZO_CHECK_MPOS_NON_DET(m_pos,m_off,in,ip,max_offset) \
    ((m_off <= 0 || m_off > max_offset || m_off > pd(ip,in)) || (m_pos = ip - m_off, 0))


/* vim:set ts=4 et: */
