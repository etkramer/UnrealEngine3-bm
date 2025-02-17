/* plzo1x_d.ch --- implementation of the LZO1X decompression algorithm

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


#if 0
#define CONFIG_PREINC 1
#endif
#if 0 || (ACC_ARCH_I086)
#define CONFIG_COMPACT_STATES 1
#endif
#if 0
#define CONFIG_FAST 1       /* ATTN: with 3 bytes output overrun ! */
#endif


#include "plzo1_d_fc.h"


/***********************************************************************
// states
************************************************************************/

#if CONFIG_COMPACT_STATES
#  define STATE_M1      0x000
#  define STATE_LR      0x100
#  define STATE_MX      0x110
#  define CASE(n) case STATE_M1 + n
#else
#  define STATE_M1      0x000
#  define STATE_LR      0x100
#  define STATE_MX      0x200
#  define CASE(n) case STATE_M1 + n: case STATE_LR + n: case STATE_MX + n
#endif


#if defined(LZO1Z)
#  define UPDATE_LM     last_m_off = pd(op, m_pos)
#else
#  define UPDATE_LM
#endif


#if defined(LZO1Z)
#  define GET_B14(p)        ((*(p) << 6) + (*((p)+1) >> 2))
#elif 1 && defined(ACC_UA_GET16) && (ACC_ABI_LITTLE_ENDIAN)
#  define GET_B14(p)        (ACC_UA_GET16(p) >> 2)
#else
#  define GET_B14(p)        ((*(p) >> 2) + (*((p)+1) << 6))
#endif


/***********************************************************************
// decompress a block of data.
************************************************************************/

#if defined(DO_DECOMPRESS)
LZO_PUBLIC(int)
DO_DECOMPRESS  ( const lzo_bytep in , lzo_uint  in_len,
                       lzo_bytep out, lzo_uintp out_len,
                       lzo_voidp wrkmem )
#endif
{
    const lzo_bytep ip;
    lzo_bytep op;
    unsigned state;

    const lzo_bytep const ip_end = in + in_len - POFF;
#if defined(HAVE_ANY_OP)
    lzo_bytep const op_end = out + *out_len - POFF;
#endif
#if defined(LZO1Z)
    lzo_uint last_m_off = 0;
#endif

    ACC_UNUSED(wrkmem);

    *out_len = 0;

    op = out - POFF;
    ip = in - POFF;

    state = STATE_LR;

    NEED_IP(3);
    {
        lzo_uint t = ip[POFF];
        if (t > 17)
        {
            ip += 1;
            t -= 17;
            state = t >= 4 ? STATE_MX : STATE_M1;
#if 1
            FC_T1_L(0,0,t);
#else
            NEED_OP(t); NEED_IP(t);
            do POP = PIP_BYTE; while (--t > 0);
#endif
        }
    }


    for (;;) {
        lzo_uint t;
        const lzo_bytep m_pos;

        /* prefetch write cache for writing at "op" */
#if 0
        __builtin_prefetch(op, 1);  /* gcc */
#endif
#if 0 && (LZO_ARCH_ARM)
        /* !!! WARNING: this does overrun "op" by one byte !!! */
        (void) * (volatile char *) op;
#endif

        NEED_IP(3);
        t = PIP;

#if CONFIG_COMPACT_STATES
        if (t >= 16)
            state = STATE_M1;
#endif
        state += (unsigned) t;

        switch (state)
        {


/* .LR long literal */
        case STATE_LR + 0x00:
            /* note: t == 0 */
#if 0 && (ACC_ARCH_SPU && ACC_CC_GNUC)
            /* FIXME: avoid bug in spu-toolchain-2.3-bsc2.0:
             * "Error: Relocation doesn't fit." */
            t = 0; P_LOOP_255(ip, t, 0);
#else
            assert(t == 0); P_LOOP_255(ip, t, 0);
#endif
            FC_T1_L(18,20,t);
            state = STATE_MX; continue;

case STATE_LR+0x01: FC_L(4,4); state=STATE_MX; continue;
case STATE_LR+0x02: FC_L(5,8); state=STATE_MX; continue;
case STATE_LR+0x03: FC_L(6,8); state=STATE_MX; continue;
case STATE_LR+0x04: FC_L(7,8); state=STATE_MX; continue;
case STATE_LR+0x05: FC_L(8,8); state=STATE_MX; continue;
case STATE_LR+0x06: FC_L(9,12); state=STATE_MX; continue;
case STATE_LR+0x07: FC_L(10,12); state=STATE_MX; continue;
case STATE_LR+0x08: FC_L(11,12); state=STATE_MX; continue;
case STATE_LR+0x09: FC_L(12,12); state=STATE_MX; continue;
case STATE_LR+0x0a: FC_L(13,16); state=STATE_MX; continue;
case STATE_LR+0x0b: FC_L(14,16); state=STATE_MX; continue;
case STATE_LR+0x0c: FC_L(15,16); state=STATE_MX; continue;
case STATE_LR+0x0d: FC_L(16,16); state=STATE_MX; continue;
case STATE_LR+0x0e: FC_L(17,20); state=STATE_MX; continue;
case STATE_LR+0x0f: FC_L(18,20); state=STATE_MX; continue;


/* .MX match */
case STATE_MX+0x00: m_pos = op-2049U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_0;
case STATE_MX+0x01: m_pos = op-2049U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_1;
case STATE_MX+0x02: m_pos = op-2049U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_2;
case STATE_MX+0x03: m_pos = op-2049U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_3;
case STATE_MX+0x04: m_pos = op-2050U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_0;
case STATE_MX+0x05: m_pos = op-2050U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_1;
case STATE_MX+0x06: m_pos = op-2050U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_2;
case STATE_MX+0x07: m_pos = op-2050U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_3;
case STATE_MX+0x08: m_pos = op-2051U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_0;
case STATE_MX+0x09: m_pos = op-2051U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_1;
case STATE_MX+0x0a: m_pos = op-2051U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_2;
case STATE_MX+0x0b: m_pos = op-2051U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_3;
case STATE_MX+0x0c: m_pos = op-2052U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_0;
case STATE_MX+0x0d: m_pos = op-2052U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_1;
case STATE_MX+0x0e: m_pos = op-2052U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_2;
case STATE_MX+0x0f: m_pos = op-2052U-(PIP<<2); UPDATE_LM; FC_M4(3,4); goto next_state_3;



/* .M1 match */
case STATE_M1+0x00: m_pos = op-1U-(PIP<<2); UPDATE_LM; FC_M1(2,4); goto next_state_0;
case STATE_M1+0x01: m_pos = op-1U-(PIP<<2); UPDATE_LM; FC_M1(2,4); goto next_state_1;
case STATE_M1+0x02: m_pos = op-1U-(PIP<<2); UPDATE_LM; FC_M1(2,4); goto next_state_2;
case STATE_M1+0x03: m_pos = op-1U-(PIP<<2); UPDATE_LM; FC_M1(2,4); goto next_state_3;
case STATE_M1+0x04: m_pos = op-2U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_0;
case STATE_M1+0x05: m_pos = op-2U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_1;
case STATE_M1+0x06: m_pos = op-2U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_2;
case STATE_M1+0x07: m_pos = op-2U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_3;
case STATE_M1+0x08: m_pos = op-3U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_0;
case STATE_M1+0x09: m_pos = op-3U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_1;
case STATE_M1+0x0a: m_pos = op-3U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_2;
case STATE_M1+0x0b: m_pos = op-3U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_3;
case STATE_M1+0x0c: m_pos = op-4U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_0;
case STATE_M1+0x0d: m_pos = op-4U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_1;
case STATE_M1+0x0e: m_pos = op-4U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_2;
case STATE_M1+0x0f: m_pos = op-4U-(PIP<<2); UPDATE_LM; FC_M4(2,4); goto next_state_3;


/* .M4a match */

CASE(0x10): t=0; P_LOOP_255(ip,t,0); m_pos = op-16384U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_T1_M8(9,12,t); goto next_state;
CASE(0x18): t=0; P_LOOP_255(ip,t,0); m_pos = op-32768U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_T1_M8(9,12,t); goto next_state;

        CASE(0x11):
            m_pos = op - GET_B14(ip+POFF); ip += 2;
            if (m_pos == op) goto eof_found;
            m_pos -= 0x4000;
            UPDATE_LM; FC_M4(3,4);
            goto next_state;

/* .M4b match */

CASE(0x12): m_pos = op-16384U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M4(4,4); goto next_state;
CASE(0x13): m_pos = op-16384U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(5,8); goto next_state;
CASE(0x14): m_pos = op-16384U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(6,8); goto next_state;
CASE(0x15): m_pos = op-16384U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(7,8); goto next_state;
CASE(0x16): m_pos = op-16384U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(8,8); goto next_state;
CASE(0x17): m_pos = op-16384U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(9,12); goto next_state;
CASE(0x19): m_pos = op-32768U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M4(3,4); goto next_state;
CASE(0x1a): m_pos = op-32768U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M4(4,4); goto next_state;
CASE(0x1b): m_pos = op-32768U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(5,8); goto next_state;
CASE(0x1c): m_pos = op-32768U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(6,8); goto next_state;
CASE(0x1d): m_pos = op-32768U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(7,8); goto next_state;
CASE(0x1e): m_pos = op-32768U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(8,8); goto next_state;
CASE(0x1f): m_pos = op-32768U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M8(9,12); goto next_state;


/* .M3 match */
        CASE(0x20):
            t = 0; P_LOOP_255(ip, t, 0);
            m_pos = op - 1 - GET_B14(ip+POFF); ip += 2;
            UPDATE_LM; FC_T1_M1(33,36,t);
            goto next_state;

CASE(0x21): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(3,4); goto next_state;
CASE(0x22): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(4,4); goto next_state;
CASE(0x23): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(5,8); goto next_state;
CASE(0x24): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(6,8); goto next_state;
CASE(0x25): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(7,8); goto next_state;
CASE(0x26): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(8,8); goto next_state;
CASE(0x27): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(9,12); goto next_state;
CASE(0x28): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(10,12); goto next_state;
CASE(0x29): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(11,12); goto next_state;
CASE(0x2a): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(12,12); goto next_state;
CASE(0x2b): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(13,16); goto next_state;
CASE(0x2c): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(14,16); goto next_state;
CASE(0x2d): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(15,16); goto next_state;
CASE(0x2e): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(16,16); goto next_state;
CASE(0x2f): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(17,20); goto next_state;
CASE(0x30): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(18,20); goto next_state;
CASE(0x31): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(19,20); goto next_state;
CASE(0x32): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(20,20); goto next_state;
CASE(0x33): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(21,24); goto next_state;
CASE(0x34): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(22,24); goto next_state;
CASE(0x35): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(23,24); goto next_state;
CASE(0x36): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(24,24); goto next_state;
CASE(0x37): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(25,28); goto next_state;
CASE(0x38): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(26,28); goto next_state;
CASE(0x39): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(27,28); goto next_state;
CASE(0x3a): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(28,28); goto next_state;
CASE(0x3b): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(29,32); goto next_state;
CASE(0x3c): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(30,32); goto next_state;
CASE(0x3d): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(31,32); goto next_state;
CASE(0x3e): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(32,32); goto next_state;
CASE(0x3f): m_pos = op-1U-GET_B14(ip+POFF); ip+=2; UPDATE_LM; FC_M1(33,36); goto next_state;


/* .M2 match */

CASE(0x40): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(3,4); goto next_state_0;
CASE(0x41): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(3,4); goto next_state_1;
CASE(0x42): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(3,4); goto next_state_2;
CASE(0x43): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(3,4); goto next_state_3;
CASE(0x44): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(3,4); goto next_state_0;
CASE(0x45): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(3,4); goto next_state_1;
CASE(0x46): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(3,4); goto next_state_2;
CASE(0x47): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(3,4); goto next_state_3;
CASE(0x48): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_0;
CASE(0x49): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_1;
CASE(0x4a): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_2;
CASE(0x4b): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_3;
CASE(0x4c): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_0;
CASE(0x4d): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_1;
CASE(0x4e): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_2;
CASE(0x4f): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_3;
CASE(0x50): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_0;
CASE(0x51): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_1;
CASE(0x52): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_2;
CASE(0x53): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_3;
CASE(0x54): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_0;
CASE(0x55): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_1;
CASE(0x56): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_2;
CASE(0x57): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_3;
CASE(0x58): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_0;
CASE(0x59): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_1;
CASE(0x5a): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_2;
CASE(0x5b): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_3;
CASE(0x5c): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_0;
CASE(0x5d): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_1;
CASE(0x5e): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_2;
CASE(0x5f): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M4(3,4); goto next_state_3;
CASE(0x60): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(4,4); goto next_state_0;
CASE(0x61): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(4,4); goto next_state_1;
CASE(0x62): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(4,4); goto next_state_2;
CASE(0x63): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(4,4); goto next_state_3;
CASE(0x64): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(4,4); goto next_state_0;
CASE(0x65): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(4,4); goto next_state_1;
CASE(0x66): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(4,4); goto next_state_2;
CASE(0x67): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(4,4); goto next_state_3;
CASE(0x68): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(4,4); goto next_state_0;
CASE(0x69): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(4,4); goto next_state_1;
CASE(0x6a): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(4,4); goto next_state_2;
CASE(0x6b): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(4,4); goto next_state_3;
CASE(0x6c): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_0;
CASE(0x6d): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_1;
CASE(0x6e): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_2;
CASE(0x6f): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_3;
CASE(0x70): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_0;
CASE(0x71): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_1;
CASE(0x72): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_2;
CASE(0x73): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_3;
CASE(0x74): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_0;
CASE(0x75): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_1;
CASE(0x76): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_2;
CASE(0x77): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_3;
CASE(0x78): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_0;
CASE(0x79): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_1;
CASE(0x7a): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_2;
CASE(0x7b): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_3;
CASE(0x7c): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_0;
CASE(0x7d): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_1;
CASE(0x7e): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_2;
CASE(0x7f): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M4(4,4); goto next_state_3;
CASE(0x80): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(5,8); goto next_state_0;
CASE(0x81): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(5,8); goto next_state_1;
CASE(0x82): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(5,8); goto next_state_2;
CASE(0x83): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(5,8); goto next_state_3;
CASE(0x84): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(5,8); goto next_state_0;
CASE(0x85): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(5,8); goto next_state_1;
CASE(0x86): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(5,8); goto next_state_2;
CASE(0x87): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(5,8); goto next_state_3;
CASE(0x88): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(5,8); goto next_state_0;
CASE(0x89): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(5,8); goto next_state_1;
CASE(0x8a): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(5,8); goto next_state_2;
CASE(0x8b): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(5,8); goto next_state_3;
CASE(0x8c): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(5,8); goto next_state_0;
CASE(0x8d): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(5,8); goto next_state_1;
CASE(0x8e): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(5,8); goto next_state_2;
CASE(0x8f): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(5,8); goto next_state_3;
CASE(0x90): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_0;
CASE(0x91): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_1;
CASE(0x92): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_2;
CASE(0x93): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_3;
CASE(0x94): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_0;
CASE(0x95): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_1;
CASE(0x96): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_2;
CASE(0x97): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_3;
CASE(0x98): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_0;
CASE(0x99): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_1;
CASE(0x9a): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_2;
CASE(0x9b): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_3;
CASE(0x9c): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_0;
CASE(0x9d): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_1;
CASE(0x9e): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_2;
CASE(0x9f): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(5,8); goto next_state_3;
CASE(0xa0): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(6,8); goto next_state_0;
CASE(0xa1): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(6,8); goto next_state_1;
CASE(0xa2): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(6,8); goto next_state_2;
CASE(0xa3): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(6,8); goto next_state_3;
CASE(0xa4): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(6,8); goto next_state_0;
CASE(0xa5): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(6,8); goto next_state_1;
CASE(0xa6): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(6,8); goto next_state_2;
CASE(0xa7): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(6,8); goto next_state_3;
CASE(0xa8): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(6,8); goto next_state_0;
CASE(0xa9): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(6,8); goto next_state_1;
CASE(0xaa): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(6,8); goto next_state_2;
CASE(0xab): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(6,8); goto next_state_3;
CASE(0xac): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(6,8); goto next_state_0;
CASE(0xad): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(6,8); goto next_state_1;
CASE(0xae): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(6,8); goto next_state_2;
CASE(0xaf): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(6,8); goto next_state_3;
CASE(0xb0): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(6,8); goto next_state_0;
CASE(0xb1): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(6,8); goto next_state_1;
CASE(0xb2): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(6,8); goto next_state_2;
CASE(0xb3): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(6,8); goto next_state_3;
CASE(0xb4): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_0;
CASE(0xb5): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_1;
CASE(0xb6): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_2;
CASE(0xb7): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_3;
CASE(0xb8): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_0;
CASE(0xb9): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_1;
CASE(0xba): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_2;
CASE(0xbb): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_3;
CASE(0xbc): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_0;
CASE(0xbd): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_1;
CASE(0xbe): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_2;
CASE(0xbf): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(6,8); goto next_state_3;
CASE(0xc0): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(7,8); goto next_state_0;
CASE(0xc1): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(7,8); goto next_state_1;
CASE(0xc2): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(7,8); goto next_state_2;
CASE(0xc3): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(7,8); goto next_state_3;
CASE(0xc4): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(7,8); goto next_state_0;
CASE(0xc5): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(7,8); goto next_state_1;
CASE(0xc6): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(7,8); goto next_state_2;
CASE(0xc7): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(7,8); goto next_state_3;
CASE(0xc8): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(7,8); goto next_state_0;
CASE(0xc9): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(7,8); goto next_state_1;
CASE(0xca): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(7,8); goto next_state_2;
CASE(0xcb): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(7,8); goto next_state_3;
CASE(0xcc): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_0;
CASE(0xcd): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_1;
CASE(0xce): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_2;
CASE(0xcf): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_3;
CASE(0xd0): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_0;
CASE(0xd1): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_1;
CASE(0xd2): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_2;
CASE(0xd3): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_3;
CASE(0xd4): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_0;
CASE(0xd5): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_1;
CASE(0xd6): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_2;
CASE(0xd7): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(7,8); goto next_state_3;
CASE(0xd8): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(7,8); goto next_state_0;
CASE(0xd9): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(7,8); goto next_state_1;
CASE(0xda): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(7,8); goto next_state_2;
CASE(0xdb): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M8(7,8); goto next_state_3;
CASE(0xdc): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(7,8); goto next_state_0;
CASE(0xdd): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(7,8); goto next_state_1;
CASE(0xde): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(7,8); goto next_state_2;
CASE(0xdf): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(7,8); goto next_state_3;
CASE(0xe0): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(8,8); goto next_state_0;
CASE(0xe1): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(8,8); goto next_state_1;
CASE(0xe2): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(8,8); goto next_state_2;
CASE(0xe3): m_pos = op-1U-(PIP<<3); UPDATE_LM; FC_M1(8,8); goto next_state_3;
CASE(0xe4): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(8,8); goto next_state_0;
CASE(0xe5): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(8,8); goto next_state_1;
CASE(0xe6): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(8,8); goto next_state_2;
CASE(0xe7): m_pos = op-2U-(PIP<<3); UPDATE_LM; FC_M2(8,8); goto next_state_3;
CASE(0xe8): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(8,8); goto next_state_0;
CASE(0xe9): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(8,8); goto next_state_1;
CASE(0xea): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(8,8); goto next_state_2;
CASE(0xeb): m_pos = op-3U-(PIP<<3); UPDATE_LM; FC_M2(8,8); goto next_state_3;
CASE(0xec): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_0;
CASE(0xed): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_1;
CASE(0xee): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_2;
CASE(0xef): m_pos = op-4U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_3;
CASE(0xf0): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_0;
CASE(0xf1): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_1;
CASE(0xf2): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_2;
CASE(0xf3): m_pos = op-5U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_3;
CASE(0xf4): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_0;
CASE(0xf5): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_1;
CASE(0xf6): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_2;
CASE(0xf7): m_pos = op-6U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_3;
CASE(0xf8): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_0;
CASE(0xf9): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_1;
CASE(0xfa): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_2;
CASE(0xfb): m_pos = op-7U-(PIP<<3); UPDATE_LM; FC_M4(8,8); goto next_state_3;
CASE(0xfc): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(8,8); goto next_state_0;
CASE(0xfd): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(8,8); goto next_state_1;
CASE(0xfe): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(8,8); goto next_state_2;
CASE(0xff): m_pos = op-8U-(PIP<<3); UPDATE_LM; FC_M8(8,8); goto next_state_3;


#if 0 && !defined(NDEBUG)
        default:
            fprintf(stderr, "error %d %d\n", (int)state, (int)t); exit(1);
#endif


        }

next_state:
        state = STATE_LR;
#if defined(LZO1Z)
        t = ip[POFF-1] & 3;
#else
        t = ip[POFF-2] & 3;
#endif
        if (t) {
#if defined(CONFIG_FAST)
/* See FC_L_CHECK() about NEED_OP() semantics */
/* WARNING: this uses __FC_L() directly */
            NEED_OP(t); NEED_IP(4);
            __FC_L(4,0,0); op += t; ip += t;
#else
            NEED_OP(t); NEED_IP(4);
            do POP = PIP_BYTE; while (--t > 0);
#endif
            state = STATE_M1;
        }
        continue;


next_state_0:
        state = STATE_LR;
        continue;
next_state_1:
        FC_L(1,4);
        state = STATE_M1;
        continue;
next_state_2:
        FC_L(2,4);
        state = STATE_M1;
        continue;
next_state_3:
        FC_L(3,4);
        state = STATE_M1;
        continue;
    }


eof_found:
    assert(ip <= ip_end);
#if defined(HAVE_ANY_OP)
    assert(op <= op_end);
#endif
    *out_len = pd(op + POFF, out);
    return (ip == ip_end ? LZO_E_OK :
           (ip < ip_end  ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN));

#if defined(HAVE_NEED_IP)
input_overrun:
    assert(ip <= ip_end); assert(op <= op_end);
    *out_len = pd(op + POFF, out);
    return LZO_E_INPUT_OVERRUN;
#endif

#if defined(HAVE_NEED_OP)
output_overrun:
    assert(ip <= ip_end); assert(op <= op_end);
    *out_len = pd(op + POFF, out);
    return LZO_E_OUTPUT_OVERRUN;
#endif

#if defined(LZO_TEST_OVERRUN_LOOKBEHIND)
lookbehind_overrun:
    assert(ip <= ip_end); assert(op <= op_end);
    *out_len = pd(op + POFF, out);
    return LZO_E_LOOKBEHIND_OVERRUN;
#endif
}


/* vim:set ts=4 sw=4 et: */
