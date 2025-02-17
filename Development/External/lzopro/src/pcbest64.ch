/* pcbest64.ch -- LZO1[XY]-99 compression

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


struct l_struct_2E_A2;
struct l_struct_2E_A2_3A__3A_A3;
struct l_struct_2E_A5;
struct l_struct_2E_A6;
struct l_struct_2E_A7;
struct l_struct_2E_A8;
struct l_struct_2E_A9;
struct l_struct_2E_E;
struct l_struct_2E_E_3A__3A_A10;
struct l_struct_2E_E_3A__3A_A11;
struct l_struct_2E_E_3A__3A_A12;
struct l_struct_2E_E_3A__3A_A13;
struct l_struct_2E_E_3A__3A_A14;
struct l_struct_2E_E_3A__3A_A15;
struct l_struct_2E_E_3A__3A_A4;
struct l_struct_2E_Lzo1E;
struct l_struct_2E_Lzo1xE;
struct l_struct_2E_Lzo1yE;
struct l_struct_2E_M;
struct plzo_callback_t;
struct plzo_compress_config_t;


typedef struct l_struct_2E_A2 l_struct_2E_A2;
typedef struct l_struct_2E_A2_3A__3A_A3 l_struct_2E_A2_3A__3A_A3;
typedef struct l_struct_2E_A5 l_struct_2E_A5;
typedef struct l_struct_2E_A6 l_struct_2E_A6;
typedef struct l_struct_2E_A7 l_struct_2E_A7;
typedef struct l_struct_2E_A8 l_struct_2E_A8;
typedef struct l_struct_2E_A9 l_struct_2E_A9;
typedef struct l_struct_2E_E l_struct_2E_E;
typedef struct l_struct_2E_E_3A__3A_A10 l_struct_2E_E_3A__3A_A10;
typedef struct l_struct_2E_E_3A__3A_A11 l_struct_2E_E_3A__3A_A11;
typedef struct l_struct_2E_E_3A__3A_A12 l_struct_2E_E_3A__3A_A12;
typedef struct l_struct_2E_E_3A__3A_A13 l_struct_2E_E_3A__3A_A13;
typedef struct l_struct_2E_E_3A__3A_A14 l_struct_2E_E_3A__3A_A14;
typedef struct l_struct_2E_E_3A__3A_A15 l_struct_2E_E_3A__3A_A15;
typedef struct l_struct_2E_E_3A__3A_A4 l_struct_2E_E_3A__3A_A4;
typedef struct l_struct_2E_Lzo1E l_struct_2E_Lzo1E;
typedef struct l_struct_2E_Lzo1xE l_struct_2E_Lzo1xE;
typedef struct l_struct_2E_Lzo1yE l_struct_2E_Lzo1yE;
typedef struct l_struct_2E_M l_struct_2E_M;
typedef struct plzo_callback_t plzo_callback_t;
typedef struct plzo_compress_config_t plzo_compress_config_t;


struct l_struct_2E_A2 {
  plzo_uint32_t field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  struct l_struct_2E_A2_3A__3A_A3 *field3;
  struct l_struct_2E_A9 *field4;
};

struct l_struct_2E_A2_3A__3A_A3 {
  plzo_uint32_t field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
};

struct l_struct_2E_A5 {
  plzo_uint32_t field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_uint32_t field4;
  unsigned char *field5;
  unsigned char *field6;
};

struct l_struct_2E_A6 {
  unsigned char *field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_uint32_t field4;
  plzo_uint32_t field5;
  plzo_uint32_t field6;
  plzo_uint32_t field7;
  plzo_uint32_t field8;
  plzo_uint32_t field9;
  plzo_uint32_t field10;
  plzo_uint32_t field11;
  plzo_uint32_t field12;
  plzo_uint32_t field13;
  plzo_uint32_t field14;
  plzo_uint32_t field15;
  plzo_uint32_t field16;
  struct l_struct_2E_A7 *field17;
  struct l_struct_2E_M *field18;
  plzo_uint32_t field19;
  plzo_uint32_t field20;
  plzo_uint32_t field21;
  plzo_int32_t field22;
  plzo_uint32_t field23;
  plzo_uint32_t field24;
  unsigned char *field25[1025];
  plzo_uint32_t field26[65536];
  plzo_uint32_t field27[256];
};

struct l_struct_2E_A7 {
  plzo_uint32_t field0;
  plzo_uint32_t field1[8];
};

struct l_struct_2E_A8 {
  struct l_struct_2E_A7 *field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  unsigned char *field4;
  plzo_uint32_t field5;
  struct l_struct_2E_A9 *field6;
  struct l_struct_2E_A6 *field7;
};

struct plzo_callback_t {
  signed char * (*field0) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  void  (*field1) (struct plzo_callback_t *, signed char *);
  void  (*field2) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t );
  signed char *field3;
  plzo_uint32_t field4;
  plzo_uint32_t field5;
};

struct l_struct_2E_A9 {
  plzo_uint32_t field0;
  plzo_uint32_t field1;
  struct plzo_callback_t field2;
};

struct l_struct_2E_E_3A__3A_A11 {
  plzo_uint32_t field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_uint32_t field4;
  plzo_uint32_t field5;
  plzo_uint32_t field6;
  plzo_uint32_t field7;
  plzo_uint32_t field8[8];
};

struct l_struct_2E_E_3A__3A_A10 {
  plzo_int32_t field0;
  plzo_int32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_uint32_t field4;
  plzo_uint32_t field5;
  plzo_uint32_t field6;
  plzo_uint32_t field7[17];
};

struct l_struct_2E_E_3A__3A_A13 {
  plzo_int32_t field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_uint32_t field4;
  plzo_uint32_t field5;
  plzo_uint32_t field6;
  plzo_uint32_t field7;
  plzo_uint32_t field8;
  plzo_int32_t field9;
  plzo_int32_t field10;
  plzo_int32_t field11;
  plzo_uint32_t field12;
  plzo_uint32_t field13;
  plzo_int32_t field14;
  plzo_int32_t field15;
};

struct l_struct_2E_E_3A__3A_A14 {
  unsigned char *field0;
  plzo_uint32_t field1;
  plzo_uint32_t  (*field2) (plzo_uint32_t );
  void  (*field3) (struct l_struct_2E_A5 *, plzo_uint32_t );
};

struct l_struct_2E_E_3A__3A_A15 {
  unsigned char field0;
};

struct l_struct_2E_E {
  plzo_int32_t  (**field0) ();
  struct l_struct_2E_A5 field1;
  struct l_struct_2E_E_3A__3A_A11 field2;
  plzo_int32_t field3;
  plzo_uint32_t field4[17];
  plzo_uint32_t field5;
  struct l_struct_2E_E_3A__3A_A10 field6;
  struct l_struct_2E_E_3A__3A_A13 field7;
  unsigned char *field8;
  unsigned char *field9;
  unsigned char *field10;
  plzo_uint32_t field11;
  plzo_uint32_t field12;
  plzo_uint32_t field13;
  plzo_uint32_t field14;
  plzo_uint32_t field15;
  plzo_uint32_t field16;
  plzo_uint32_t field17;
  plzo_uint32_t field18;
  struct l_struct_2E_A8 field19;
  struct l_struct_2E_A2 field20;
  plzo_uint32_t field21;
  struct l_struct_2E_E_3A__3A_A14 field22;
  struct l_struct_2E_E_3A__3A_A14 field23;
  struct l_struct_2E_A9 field24;
  struct l_struct_2E_E_3A__3A_A15 field25;
  struct l_struct_2E_E_3A__3A_A15 *field26;
};

struct l_struct_2E_E_3A__3A_A12 {
  plzo_uint32_t field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_uint32_t field4;
  plzo_uint32_t field5;
};

struct l_struct_2E_E_3A__3A_A4 {
  plzo_uint32_t field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_uint32_t field4;
  plzo_uint32_t field5;
  plzo_uint32_t field6;
  plzo_uint32_t field7;
};

struct l_struct_2E_Lzo1E {
  struct l_struct_2E_E field0;
  plzo_uint32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_uint32_t field4;
  plzo_uint32_t field5;
  plzo_uint32_t field6;
  plzo_uint32_t field7;
  plzo_uint32_t field8;
  plzo_uint32_t field9;
  plzo_uint32_t field10;
  plzo_uint32_t field11;
  plzo_uint32_t field12;
  plzo_uint32_t field13;
  plzo_uint32_t field14;
  plzo_uint32_t field15;
  plzo_uint32_t field16;
  plzo_uint32_t field17;
  plzo_uint32_t field18;
  plzo_uint32_t field19;
  plzo_uint32_t field20;
  plzo_uint32_t field21;
};

struct l_struct_2E_Lzo1xE {
  struct l_struct_2E_Lzo1E field0;
};

struct l_struct_2E_Lzo1yE {
  struct l_struct_2E_Lzo1xE field0;
};

struct l_struct_2E_M {
  struct l_struct_2E_M *field0;
  struct l_struct_2E_M *field1;
};

struct plzo_compress_config_t {
  plzo_int32_t field0;
  plzo_int32_t field1;
  plzo_uint32_t field2;
  plzo_uint32_t field3;
  plzo_int32_t field4;
  plzo_int32_t field5;
  plzo_int32_t field6;
  plzo_int32_t field7;
  plzo_uint32_t field8;
};


static void ltmp_0_1000(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_1_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static plzo_int32_t ltmp_2_1002(struct l_struct_2E_E *, plzo_int32_t , struct plzo_callback_t *);
static plzo_int32_t ltmp_3_1003(struct l_struct_2E_E *, struct plzo_compress_config_t *);
static plzo_int32_t ltmp_4_1004(struct l_struct_2E_E *, unsigned char *, plzo_uint32_t , unsigned char *, plzo_uint32_t *);
static void ltmp_5_1005(struct l_struct_2E_E *);
static struct l_struct_2E_E_3A__3A_A12 *ltmp_6_1006(struct l_struct_2E_E *, plzo_int32_t );
static void ltmp_7_1005(struct l_struct_2E_E *);
static void ltmp_8_1005(struct l_struct_2E_E *);
static plzo_int32_t ltmp_9_1007(struct l_struct_2E_E *);
static void ltmp_10_1005(struct l_struct_2E_E *);
static void ltmp_11_1000(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t );
static plzo_uint32_t ltmp_12_1008(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t );
static plzo_uint32_t ltmp_13_1008(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t );
static plzo_uint32_t ltmp_14_1008(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_15_1000(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_16_1009(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );
static plzo_uint32_t ltmp_17_1008(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t );
static struct l_struct_2E_M *ltmp_18_100A(struct l_struct_2E_A6 *);
static void ltmp_19_100B(struct l_struct_2E_A8 *);
static void ltmp_20_100C(struct l_struct_2E_A5 *);
static void ltmp_21_100D(struct l_struct_2E_Lzo1xE *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_22_100E(struct l_struct_2E_Lzo1xE *, unsigned char *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_23_100D(struct l_struct_2E_Lzo1xE *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_24_100F(struct l_struct_2E_Lzo1E *, plzo_uint32_t , plzo_uint32_t );
static plzo_int32_t plzo_lzo1x_99_compress_internal(unsigned char *, plzo_uint32_t , unsigned char *, plzo_uint32_t *, struct plzo_callback_t *, plzo_int32_t , struct plzo_compress_config_t *, plzo_uint32_t *);
static void ltmp_25_1010(struct l_struct_2E_Lzo1E *);
static plzo_int32_t plzo_lzo1y_99_compress_internal(unsigned char *, plzo_uint32_t , unsigned char *, plzo_uint32_t *, struct plzo_callback_t *, plzo_int32_t , struct plzo_compress_config_t *, plzo_uint32_t *);
static signed char *ltmp_26_1011(struct l_struct_2E_E *);
static void ltmp_27_1010(struct l_struct_2E_Lzo1E *);
static plzo_int32_t ltmp_28_1012(struct l_struct_2E_Lzo1E *);
static void ltmp_29_1013(struct l_struct_2E_Lzo1xE *);
static void ltmp_30_1014(struct l_struct_2E_Lzo1E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t );
static plzo_uint32_t ltmp_31_1015(struct l_struct_2E_Lzo1E *);
static plzo_int32_t ltmp_32_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static plzo_int32_t ltmp_33_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static void ltmp_34_100F(struct l_struct_2E_Lzo1E *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_35_1017(struct l_struct_2E_Lzo1E *, unsigned char *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_36_1010(struct l_struct_2E_Lzo1E *);
static plzo_uint32_t ltmp_37_1015(struct l_struct_2E_Lzo1E *);
static plzo_int32_t ltmp_38_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static plzo_int32_t ltmp_39_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static void ltmp_40_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_41_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_42_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_43_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_44_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_45_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_46_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_47_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_48_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static signed char *ltmp_49_1018(struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_50_1019(struct plzo_callback_t *, signed char *);
static void ltmp_51_101A(struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t );
static void ltmp_52_101B(struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A14 *);
static void ltmp_53_101C(struct l_struct_2E_E_3A__3A_A14 *);
static void  (*ltmp_54_101D(void  (*) (struct l_struct_2E_A5 *, plzo_uint32_t ), plzo_uint32_t , plzo_uint32_t )) (struct l_struct_2E_A5 *, plzo_uint32_t );
static plzo_int32_t ltmp_55_1007(struct l_struct_2E_E *);
static void ltmp_56_101E(struct l_struct_2E_E *, unsigned char *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_57_1000(struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_58_1005(struct l_struct_2E_E *);
static void ltmp_59_101F(struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t );
static plzo_uint32_t ltmp_60_1020(struct l_struct_2E_E *);
static plzo_int32_t ltmp_61_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static plzo_int32_t ltmp_62_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static void ltmp_63_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_64_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_65_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_66_1001(struct l_struct_2E_A5 *, plzo_uint32_t );
static void ltmp_67_1001(struct l_struct_2E_A5 *, plzo_uint32_t );


static plzo_int32_t  (*ltmp_68_1021[24]) () = { ((plzo_int32_t  (*) ())0), ((plzo_int32_t  (*) ())0), ((plzo_int32_t  (*) ())ltmp_26_1011), ((plzo_int32_t  (*) ())ltmp_4_1004), ((plzo_int32_t  (*) ())ltmp_6_1006), ((plzo_int32_t  (*) ())ltmp_22_100E), ((plzo_int32_t  (*) ())ltmp_21_100D), ((plzo_int32_t  (*) ())ltmp_27_1010), ((plzo_int32_t  (*) ())ltmp_7_1005), ((plzo_int32_t  (*) ())ltmp_28_1012), ((plzo_int32_t  (*) ())ltmp_29_1013), ((plzo_int32_t  (*) ())ltmp_9_1007), ((plzo_int32_t  (*) ())ltmp_10_1005), ((plzo_int32_t  (*) ())ltmp_23_100D), ((plzo_int32_t  (*) ())ltmp_12_1008), ((plzo_int32_t  (*) ())ltmp_13_1008), ((plzo_int32_t  (*) ())ltmp_14_1008), ((plzo_int32_t  (*) ())ltmp_15_1000), ((plzo_int32_t  (*) ())ltmp_16_1009), ((plzo_int32_t  (*) ())ltmp_30_1014), ((plzo_int32_t  (*) ())ltmp_31_1015), ((plzo_int32_t  (*) ())ltmp_17_1008), ((plzo_int32_t  (*) ())ltmp_32_1016), ((plzo_int32_t  (*) ())ltmp_33_1016) };
static plzo_int32_t  (*ltmp_69_1021[24]) () = { ((plzo_int32_t  (*) ())0), ((plzo_int32_t  (*) ())0), ((plzo_int32_t  (*) ())ltmp_26_1011), ((plzo_int32_t  (*) ())ltmp_4_1004), ((plzo_int32_t  (*) ())ltmp_6_1006), ((plzo_int32_t  (*) ())ltmp_35_1017), ((plzo_int32_t  (*) ())ltmp_34_100F), ((plzo_int32_t  (*) ())ltmp_27_1010), ((plzo_int32_t  (*) ())ltmp_7_1005), ((plzo_int32_t  (*) ())ltmp_28_1012), ((plzo_int32_t  (*) ())ltmp_36_1010), ((plzo_int32_t  (*) ())ltmp_9_1007), ((plzo_int32_t  (*) ())ltmp_10_1005), ((plzo_int32_t  (*) ())ltmp_24_100F), ((plzo_int32_t  (*) ())ltmp_12_1008), ((plzo_int32_t  (*) ())ltmp_13_1008), ((plzo_int32_t  (*) ())ltmp_14_1008), ((plzo_int32_t  (*) ())ltmp_15_1000), ((plzo_int32_t  (*) ())ltmp_16_1009), ((plzo_int32_t  (*) ())ltmp_30_1014), ((plzo_int32_t  (*) ())ltmp_37_1015), ((plzo_int32_t  (*) ())ltmp_17_1008), ((plzo_int32_t  (*) ())ltmp_38_1016), ((plzo_int32_t  (*) ())ltmp_39_1016) };
static plzo_int32_t  (*ltmp_70_1021[24]) () = { ((plzo_int32_t  (*) ())0), ((plzo_int32_t  (*) ())0), ((plzo_int32_t  (*) ())ltmp_26_1011), ((plzo_int32_t  (*) ())ltmp_4_1004), ((plzo_int32_t  (*) ())ltmp_6_1006), ((plzo_int32_t  (*) ())ltmp_56_101E), ((plzo_int32_t  (*) ())ltmp_57_1000), ((plzo_int32_t  (*) ())ltmp_58_1005), ((plzo_int32_t  (*) ())ltmp_7_1005), ((plzo_int32_t  (*) ())ltmp_55_1007), ((plzo_int32_t  (*) ())ltmp_8_1005), ((plzo_int32_t  (*) ())ltmp_9_1007), ((plzo_int32_t  (*) ())ltmp_10_1005), ((plzo_int32_t  (*) ())ltmp_11_1000), ((plzo_int32_t  (*) ())ltmp_12_1008), ((plzo_int32_t  (*) ())ltmp_13_1008), ((plzo_int32_t  (*) ())ltmp_14_1008), ((plzo_int32_t  (*) ())ltmp_15_1000), ((plzo_int32_t  (*) ())ltmp_16_1009), ((plzo_int32_t  (*) ())ltmp_59_101F), ((plzo_int32_t  (*) ())ltmp_60_1020), ((plzo_int32_t  (*) ())ltmp_17_1008), ((plzo_int32_t  (*) ())ltmp_61_1016), ((plzo_int32_t  (*) ())ltmp_62_1016) };
static struct l_struct_2E_E_3A__3A_A12 ltmp_71_1022[10] = { { 65535u, 8u, 1u, 1u, 4u, 0u }, { 65535u, 16u, 1u, 1u, 8u, 1u }, { 65535u, 32u, 3u, 1u, 16u, 2u }, { 131071u, 16u, 4u, 4u, 16u, 3u }, { 131071u, 32u, 16u, 8u, 32u, 3u }, { 131071u, 128u, 16u, 8u, 128u, 4u }, { 0u, 128u, 32u, 8u, 256u, 4u }, { 0u, 256u, 128u, 32u, 2048u, 4u }, { 0u, 1024u, 1024u, 32u, 4096u, 4u }, { 0u, 1024u, 1024u, 128u, 8192u, 7u } };




static void ltmp_0_1000(struct l_struct_2E_E *ltmp_72_1023, plzo_uint32_t ltmp_73_7, plzo_uint32_t ltmp_74_7) {
  plzo_uint32_t *ltmp_75_17;
  plzo_uint32_t ltmp_76_7;
  plzo_uint32_t *ltmp_77_17;
  plzo_uint32_t ltmp_78_7;
  plzo_uint32_t *ltmp_79_17;
  plzo_uint32_t ltmp_80_7;
  plzo_uint32_t ltmp_81_7;
  plzo_uint32_t ltmp_82_7;
  plzo_uint32_t ltmp_83_7;
  plzo_uint32_t ltmp_84_7;
  plzo_uint32_t ltmp_85_7;

  ltmp_75_17 = &ltmp_72_1023->field2.field6;
  ltmp_76_7 = *ltmp_75_17;
  if ((ltmp_76_7 == 0u)) {
    goto ltmp_86_D;
  } else {
    goto ltmp_87_D;
  }

ltmp_86_D:
  *ltmp_75_17 = ltmp_74_7;
  *(&ltmp_72_1023->field2.field3) = ltmp_73_7;
  *(&ltmp_72_1023->field2.field2) = ltmp_73_7;
  *(&ltmp_72_1023->field2.field1) = ltmp_74_7;
  *(&ltmp_72_1023->field2.field0) = ltmp_74_7;
  return;
ltmp_87_D:
  ltmp_77_17 = &ltmp_72_1023->field2.field3;
  ltmp_78_7 = *ltmp_77_17;
  ltmp_79_17 = &ltmp_72_1023->field2.field2;
  if ((ltmp_78_7 < ltmp_73_7)) {
    goto ltmp_88_D;
  } else {
    goto ltmp_89_D;
  }

ltmp_88_D:
  *ltmp_77_17 = ltmp_73_7;
  ltmp_80_7 = *ltmp_79_17;
  if ((ltmp_80_7 > ltmp_73_7)) {
    goto ltmp_90_D;
  } else {
    goto ltmp_91_D;
  }

ltmp_89_D:
  ltmp_81_7 = *ltmp_79_17;
  if ((ltmp_81_7 > ltmp_73_7)) {
    goto ltmp_90_D;
  } else {
    goto ltmp_91_D;
  }

ltmp_90_D:
  *ltmp_79_17 = ltmp_73_7;
  ltmp_82_7 = *(&ltmp_72_1023->field2.field1);
  if ((ltmp_82_7 < ltmp_74_7)) {
    goto ltmp_92_D;
  } else {
    goto ltmp_93_D;
  }

ltmp_91_D:
  ltmp_83_7 = *(&ltmp_72_1023->field2.field1);
  if ((ltmp_83_7 < ltmp_74_7)) {
    goto ltmp_92_D;
  } else {
    goto ltmp_93_D;
  }

ltmp_92_D:
  *(&ltmp_72_1023->field2.field1) = ltmp_74_7;
  ltmp_84_7 = *(&ltmp_72_1023->field2.field0);
  if ((ltmp_84_7 > ltmp_74_7)) {
    goto ltmp_94_D;
  } else {
    goto ltmp_95_D;
  }

ltmp_93_D:
  ltmp_85_7 = *(&ltmp_72_1023->field2.field0);
  if ((ltmp_85_7 > ltmp_74_7)) {
    goto ltmp_94_D;
  } else {
    goto ltmp_95_D;
  }

ltmp_94_D:
  *(&ltmp_72_1023->field2.field0) = ltmp_74_7;
  return;
ltmp_95_D:
  return;
}


static void ltmp_1_1001(struct l_struct_2E_A5 *ltmp_96_1024, plzo_uint32_t ltmp_97_7) {
  plzo_uint32_t *ltmp_98_17;
  plzo_uint32_t ltmp_99_7;
  plzo_uint32_t ltmp_100_7;
  unsigned char **ltmp_101_1025;
  unsigned char *ltmp_102_13;
  plzo_uint32_t ltmp_103_7;
  plzo_uint32_t *ltmp_104_17;
  plzo_uint32_t ltmp_105_7;
  unsigned char **ltmp_106_1025;
  unsigned char *ltmp_107_13;
  plzo_uint32_t ltmp_108_7;

  ltmp_98_17 = &ltmp_96_1024->field1;
  ltmp_99_7 = *ltmp_98_17;
  ltmp_100_7 = *(&ltmp_96_1024->field3);
  if ((ltmp_99_7 < ltmp_100_7)) {
    goto ltmp_109_D;
  } else {
    goto ltmp_110_D;
  }

ltmp_109_D:
  if ((ltmp_99_7 == 0u)) {
    goto ltmp_111_D;
  } else {
    goto ltmp_112_D;
  }

ltmp_111_D:
  ltmp_101_1025 = &ltmp_96_1024->field6;
  ltmp_102_13 = *ltmp_101_1025;
  *(&ltmp_96_1024->field5) = ltmp_102_13;
  ltmp_103_7 = *(&ltmp_96_1024->field4);
  *ltmp_101_1025 = (&ltmp_102_13[((plzo_uint64_t )ltmp_103_7)]);
  goto ltmp_112_D;

ltmp_112_D:
  ltmp_104_17 = &ltmp_96_1024->field0;
  ltmp_105_7 = *ltmp_104_17;
  *ltmp_104_17 = ((ltmp_105_7 << 1) + ltmp_97_7);
  *ltmp_98_17 = (ltmp_99_7 + 1u);
  return;
ltmp_110_D:
  ltmp_20_100C(ltmp_96_1024);
  ltmp_106_1025 = &ltmp_96_1024->field6;
  ltmp_107_13 = *ltmp_106_1025;
  *(&ltmp_96_1024->field5) = ltmp_107_13;
  ltmp_108_7 = *(&ltmp_96_1024->field4);
  *ltmp_106_1025 = (&ltmp_107_13[((plzo_uint64_t )ltmp_108_7)]);
  *(&ltmp_96_1024->field0) = ltmp_97_7;
  *ltmp_98_17 = 1u;
  return;
}


static plzo_int32_t ltmp_2_1002(struct l_struct_2E_E *ltmp_113_1023, plzo_int32_t ltmp_114_8, struct plzo_callback_t *ltmp_115_1026) {
  struct plzo_compress_config_t ltmp_116_1027;
  struct l_struct_2E_E_3A__3A_A14 *ltmp_117_1028;
  struct l_struct_2E_E_3A__3A_A14 *ltmp_118_1028;
  unsigned char *ltmp_119_13;
  plzo_uint64_t ltmp_120_9;
  plzo_uint64_t ltmp_120_9_2E;
  plzo_uint64_t ltmp_121_9;
  plzo_int32_t  (**ltmp_122_1029) ();
  plzo_int32_t  (*ltmp_123_102A) ();
  struct l_struct_2E_E_3A__3A_A12 *ltmp_124_102B;
  plzo_uint32_t *ltmp_125_17;
  plzo_uint32_t ltmp_126_7;
  plzo_uint32_t ltmp_127_7;
  plzo_uint32_t ltmp_128_7;
  plzo_uint32_t ltmp_129_7;
  plzo_uint32_t ltmp_130_7;
  plzo_uint32_t ltmp_131_7;
  plzo_uint32_t ltmp_132_7;
  plzo_uint32_t ltmp_133_7;
  unsigned char *ltmp_134_13;
  plzo_uint64_t ltmp_135_9;
  plzo_uint64_t ltmp_135_9_2E;
  plzo_uint64_t ltmp_136_9;
  plzo_uint32_t ltmp_137_7;
  plzo_int32_t ltmp_138_8;
  plzo_int32_t ltmp_139_8;
  plzo_int32_t ltmp_139_8_2E;

  if (((((plzo_uint32_t )ltmp_114_8) + 4294967295u) > 9u)) {
    goto ltmp_140_D;
  } else {
    goto ltmp_141_D;
  }

ltmp_141_D:
  *(&ltmp_113_1023->field24.field2.field3) = ((signed char *)ltmp_115_1026);
  ltmp_117_1028 = &ltmp_113_1023->field22;
  ltmp_52_101B(ltmp_113_1023, ltmp_117_1028);
  ltmp_53_101C(ltmp_117_1028);
  ltmp_118_1028 = &ltmp_113_1023->field23;
  ltmp_52_101B(ltmp_113_1023, ltmp_118_1028);
  ltmp_53_101C(ltmp_118_1028);
  ltmp_119_13 = (unsigned char *)(&ltmp_113_1023->field7);
  ltmp_120_9_2E = PLZO_UINT64_C(0);
  goto ltmp_142_D;

  do {
ltmp_142_D:
  ltmp_120_9 = ltmp_120_9_2E;
  *(&ltmp_119_13[ltmp_120_9]) = 0;
  ltmp_121_9 = ltmp_120_9 + PLZO_UINT64_C(1);
  if ((ltmp_121_9 == PLZO_UINT64_C(64))) {
    goto ltmp_143_D;
  } else {
    ltmp_120_9_2E = ltmp_121_9;
    goto ltmp_142_D;
  }

  } while (1);
ltmp_143_D:
  if (((((plzo_uint32_t )ltmp_114_8) + 4294967295u) > 9u)) {
    ltmp_139_8_2E = -2;
    goto ltmp_144_D;
  } else {
    goto ltmp_145_D;
  }

ltmp_145_D:
  ltmp_122_1029 = *(&ltmp_113_1023->field0);
  ltmp_123_102A = *(&ltmp_122_1029[PLZO_INT64_C(2)]);
  ltmp_124_102B = ((struct l_struct_2E_E_3A__3A_A12 * (*) (struct l_struct_2E_E *, plzo_int32_t ))ltmp_123_102A)(ltmp_113_1023, ltmp_114_8);
  if ((ltmp_124_102B == ((struct l_struct_2E_E_3A__3A_A12 *)0))) {
    ltmp_139_8_2E = -2;
    goto ltmp_144_D;
  } else {
    goto ltmp_146_D;
  }

ltmp_146_D:
  *(&ltmp_113_1023->field7.field0) = ltmp_114_8;
  ltmp_125_17 = &ltmp_113_1023->field7.field2;
  ltmp_126_7 = *(&ltmp_113_1023->field6.field3);
  *ltmp_125_17 = ltmp_126_7;
  ltmp_127_7 = *(&ltmp_113_1023->field6.field4);
  *(&ltmp_113_1023->field7.field3) = ltmp_127_7;
  ltmp_128_7 = *(&ltmp_113_1023->field6.field5);
  *(&ltmp_113_1023->field7.field4) = ltmp_128_7;
  ltmp_129_7 = *(&ltmp_124_102B->field0);
  if ((ltmp_129_7 == 0u)) {
    goto ltmp_147_D;
  } else {
    goto ltmp_148_D;
  }

ltmp_148_D:
  *ltmp_125_17 = ((((ltmp_126_7 <= ltmp_129_7)) ? (ltmp_126_7) : (ltmp_129_7)));
  goto ltmp_147_D;

ltmp_147_D:
  ltmp_130_7 = *(&ltmp_124_102B->field1);
  *(&ltmp_113_1023->field7.field5) = ltmp_130_7;
  ltmp_131_7 = *(&ltmp_124_102B->field2);
  *(&ltmp_113_1023->field7.field6) = ltmp_131_7;
  ltmp_132_7 = *(&ltmp_124_102B->field3);
  *(&ltmp_113_1023->field7.field7) = ltmp_132_7;
  ltmp_133_7 = *(&ltmp_124_102B->field4);
  *(&ltmp_113_1023->field7.field8) = ltmp_133_7;
  ltmp_134_13 = (unsigned char *)(&ltmp_116_1027);
  ltmp_135_9_2E = PLZO_UINT64_C(0);
  goto ltmp_149_D;

  do {
ltmp_149_D:
  ltmp_135_9 = ltmp_135_9_2E;
  *(&ltmp_134_13[ltmp_135_9]) = 255;
  ltmp_136_9 = ltmp_135_9 + PLZO_UINT64_C(1);
  if ((ltmp_136_9 == PLZO_UINT64_C(36))) {
    goto ltmp_150_D;
  } else {
    ltmp_135_9_2E = ltmp_136_9;
    goto ltmp_149_D;
  }

  } while (1);
ltmp_150_D:
  ltmp_137_7 = *(&ltmp_124_102B->field5);
  *(&ltmp_116_1027.field6) = (((plzo_int32_t )ltmp_137_7) & 7);
  *(&ltmp_116_1027.field8) = 10000u;
  ltmp_138_8 = ltmp_3_1003(ltmp_113_1023, (&ltmp_116_1027));
  ltmp_139_8_2E = ltmp_138_8;
  goto ltmp_144_D;

ltmp_144_D:
  ltmp_139_8 = ltmp_139_8_2E;
  *(&ltmp_113_1023->field3) = 1;
  return ltmp_139_8;
ltmp_140_D:
  return -2;
}


static plzo_int32_t ltmp_3_1003(struct l_struct_2E_E *ltmp_151_1023, struct plzo_compress_config_t *ltmp_152_1027) {
  plzo_int32_t ltmp_153_8;
  plzo_int32_t ltmp_154_8;
  plzo_int32_t ltmp_155_8;
  plzo_int32_t ltmp_156_8;
  plzo_int32_t ltmp_156_8_2E;
  plzo_uint32_t ltmp_157_7;
  plzo_uint32_t *ltmp_158_17;
  plzo_uint32_t ltmp_159_7;
  plzo_uint32_t ltmp_160_7;
  plzo_uint32_t ltmp_161_7;
  plzo_uint32_t ltmp_162_7;
  plzo_uint32_t ltmp_163_7;
  plzo_uint32_t ltmp_164_7;
  plzo_int32_t ltmp_165_8;
  plzo_int32_t ltmp_166_8;
  plzo_int32_t ltmp_167_8;
  plzo_uint32_t ltmp_168_7;
  plzo_uint32_t ltmp_169_7;
  plzo_uint32_t ltmp_170_7;
  plzo_int32_t ltmp_171_8;
  plzo_int32_t ltmp_172_8;
  plzo_uint32_t ltmp_173_7;
  plzo_uint32_t ltmp_174_7;
  plzo_uint32_t ltmp_175_7;
  plzo_uint32_t ltmp_176_7;
  plzo_uint32_t ltmp_177_7;
  plzo_uint32_t ltmp_178_7;
  plzo_uint32_t ltmp_179_7;
  plzo_int32_t *ltmp_180_18;
  plzo_int32_t ltmp_181_8;
  plzo_int32_t ltmp_182_8;
  plzo_uint32_t ltmp_183_7;
  plzo_uint32_t ltmp_184_7;
  plzo_uint32_t ltmp_185_7;
  plzo_uint32_t ltmp_186_7;
  plzo_uint32_t ltmp_187_7;
  plzo_uint32_t ltmp_188_7;
  plzo_uint32_t ltmp_189_7;
  plzo_uint32_t ltmp_190_7;
  plzo_uint32_t ltmp_191_7;
  plzo_uint32_t ltmp_192_7;
  plzo_uint32_t ltmp_193_7;
  plzo_uint32_t ltmp_194_7;
  plzo_uint32_t ltmp_195_7;
  plzo_uint32_t ltmp_196_7;
  plzo_uint32_t ltmp_197_7;
  plzo_uint32_t ltmp_198_7;
  plzo_int32_t ltmp_199_8;
  plzo_int32_t ltmp_199_8_2E;

  ltmp_153_8 = *(&ltmp_151_1023->field7.field0);
  if ((ltmp_153_8 > 0)) {
    goto ltmp_200_D;
  } else {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  }

ltmp_200_D:
  ltmp_154_8 = *(&ltmp_152_1027->field0);
  ltmp_155_8 = *(&ltmp_152_1027->field1);
  if ((ltmp_154_8 == -1)) {
    goto ltmp_202_D;
  } else {
    goto ltmp_203_D;
  }

ltmp_203_D:
  *(&ltmp_151_1023->field1.field2) = ((plzo_uint32_t )(ltmp_154_8 != 0));
  if ((ltmp_155_8 == -1)) {
    goto ltmp_204_D;
  } else {
    goto ltmp_205_D;
  }

ltmp_202_D:
  if ((ltmp_155_8 == -1)) {
    goto ltmp_204_D;
  } else {
    goto ltmp_205_D;
  }

ltmp_205_D:
  if (((ltmp_155_8 == 32) | (((ltmp_155_8 != 8) & (ltmp_155_8 != 16)) ^ 1))) {
    goto ltmp_206_D;
  } else {
    ltmp_156_8_2E = -2;
    goto ltmp_207_D;
  }

ltmp_206_D:
  *(&ltmp_151_1023->field1.field3) = ((plzo_uint32_t )ltmp_155_8);
  *(&ltmp_151_1023->field1.field4) = ((plzo_uint32_t )(ltmp_155_8/8));
  goto ltmp_204_D;

ltmp_204_D:
  *(&ltmp_151_1023->field1.field0) = 0u;
  *(&ltmp_151_1023->field1.field1) = 0u;
  *(&ltmp_151_1023->field1.field5) = ((unsigned char *)0);
  *(&ltmp_151_1023->field1.field6) = ((unsigned char *)0);
  ltmp_156_8_2E = 0;
  goto ltmp_207_D;

ltmp_207_D:
  ltmp_156_8 = ltmp_156_8_2E;
  if ((ltmp_156_8 == 0)) {
    goto ltmp_208_D;
  } else {
    ltmp_199_8_2E = ltmp_156_8;
    goto ltmp_201_D;
  }

ltmp_208_D:
  ltmp_157_7 = *(&ltmp_152_1027->field2);
  ltmp_158_17 = &ltmp_152_1027->field3;
  if ((ltmp_157_7 == 4294967295u)) {
    goto ltmp_209_D;
  } else {
    goto ltmp_210_D;
  }

ltmp_210_D:
  ltmp_159_7 = *(&ltmp_151_1023->field6.field3);
  *(&ltmp_151_1023->field7.field2) = ((((ltmp_157_7 <= ltmp_159_7)) ? (ltmp_157_7) : (ltmp_159_7)));
  ltmp_160_7 = *ltmp_158_17;
  if ((ltmp_160_7 == 4294967295u)) {
    goto ltmp_211_D;
  } else {
    goto ltmp_212_D;
  }

ltmp_209_D:
  ltmp_161_7 = *ltmp_158_17;
  if ((ltmp_161_7 == 4294967295u)) {
    goto ltmp_211_D;
  } else {
    goto ltmp_212_D;
  }

ltmp_212_D:
  ltmp_162_7 = *ltmp_158_17;
  ltmp_163_7 = *(&ltmp_151_1023->field6.field4);
  if ((ltmp_162_7 < ltmp_163_7)) {
    goto ltmp_211_D;
  } else {
    goto ltmp_213_D;
  }

ltmp_213_D:
  ltmp_164_7 = *(&ltmp_151_1023->field6.field5);
  *(&ltmp_151_1023->field7.field4) = ((((ltmp_162_7 <= ltmp_164_7)) ? (ltmp_162_7) : (ltmp_164_7)));
  ltmp_165_8 = *(&ltmp_152_1027->field7);
  if ((ltmp_165_8 == -1)) {
    goto ltmp_214_D;
  } else {
    goto ltmp_215_D;
  }

ltmp_211_D:
  ltmp_166_8 = *(&ltmp_152_1027->field7);
  if ((ltmp_166_8 == -1)) {
    goto ltmp_214_D;
  } else {
    goto ltmp_215_D;
  }

ltmp_215_D:
  ltmp_167_8 = *(&ltmp_152_1027->field7);
  *(&ltmp_151_1023->field7.field14) = ltmp_167_8;
  ltmp_168_7 = *(&ltmp_152_1027->field8);
  if ((ltmp_168_7 == 4294967295u)) {
    goto ltmp_216_D;
  } else {
    goto ltmp_217_D;
  }

ltmp_214_D:
  ltmp_169_7 = *(&ltmp_152_1027->field8);
  if ((ltmp_169_7 == 4294967295u)) {
    goto ltmp_216_D;
  } else {
    goto ltmp_217_D;
  }

ltmp_217_D:
  ltmp_170_7 = *(&ltmp_152_1027->field8);
  *(&ltmp_151_1023->field7.field1) = ltmp_170_7;
  ltmp_171_8 = *(&ltmp_152_1027->field6);
  switch (ltmp_171_8) {
  default:
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
;
  case 7:
    goto ltmp_218_D;
  case 6:
    goto ltmp_219_D;
  case 5:
    goto ltmp_220_D;
  case 4:
    goto ltmp_221_D;
  case 3:
    goto ltmp_222_D;
  case 2:
    goto ltmp_223_D;
  case 1:
    goto ltmp_224_D;
  case 0:
    goto ltmp_225_D;
  case -1:
    goto ltmp_226_D;
  }
ltmp_216_D:
  ltmp_172_8 = *(&ltmp_152_1027->field6);
  switch (ltmp_172_8) {
  default:
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
;
  case 7:
    goto ltmp_218_D;
  case 6:
    goto ltmp_219_D;
  case 5:
    goto ltmp_220_D;
  case 4:
    goto ltmp_221_D;
  case 3:
    goto ltmp_222_D;
  case 2:
    goto ltmp_223_D;
  case 1:
    goto ltmp_224_D;
  case 0:
    goto ltmp_225_D;
    break;
  case -1:
    goto ltmp_226_D;
  }
ltmp_225_D:
  *(&ltmp_151_1023->field7.field11) = 0;
  ltmp_173_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_173_7;
  *(&ltmp_151_1023->field7.field13) = 0u;
  goto ltmp_226_D;

ltmp_224_D:
  *(&ltmp_151_1023->field7.field11) = 1;
  ltmp_174_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_174_7;
  *(&ltmp_151_1023->field7.field13) = 0u;
  goto ltmp_226_D;

ltmp_223_D:
  *(&ltmp_151_1023->field7.field11) = 2;
  ltmp_175_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_175_7;
  *(&ltmp_151_1023->field7.field13) = 0u;
  goto ltmp_226_D;

ltmp_222_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  ltmp_176_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_176_7 + 1u);
  *(&ltmp_151_1023->field7.field13) = 1u;
  goto ltmp_226_D;

ltmp_221_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  ltmp_177_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_177_7 + 6u);
  *(&ltmp_151_1023->field7.field13) = 4u;
  goto ltmp_226_D;

ltmp_220_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  ltmp_178_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_178_7 + 32u);
  *(&ltmp_151_1023->field7.field13) = 16u;
  goto ltmp_226_D;

ltmp_219_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  ltmp_179_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_179_7 + 32u);
  *(&ltmp_151_1023->field7.field13) = 32u;
  goto ltmp_226_D;

ltmp_218_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  *(&ltmp_151_1023->field7.field12) = 0u;
  *(&ltmp_151_1023->field7.field13) = 0u;
  goto ltmp_226_D;

ltmp_226_D:
  ltmp_180_18 = &ltmp_151_1023->field7.field9;
  *ltmp_180_18 = 0;
  ltmp_181_8 = *(&ltmp_151_1023->field7.field11);
  if ((((plzo_uint32_t )ltmp_181_8) > 1u)) {
    goto ltmp_227_D;
  } else {
    goto ltmp_228_D;
  }

ltmp_228_D:
  *ltmp_180_18 = 0;
  goto ltmp_229_D;

ltmp_227_D:
  if ((ltmp_153_8 > 9)) {
    goto ltmp_230_D;
  } else {
    goto ltmp_231_D;
  }

ltmp_231_D:
  *ltmp_180_18 = 1;
  goto ltmp_229_D;

ltmp_230_D:
  *ltmp_180_18 = 2;
  goto ltmp_229_D;

ltmp_229_D:
  *(&ltmp_151_1023->field7.field10) = 0;
  ltmp_182_8 = *(&ltmp_151_1023->field6.field1);
  if (((ltmp_182_8 & 1) == 0)) {
    goto ltmp_232_D;
  } else {
    goto ltmp_233_D;
  }

ltmp_233_D:
  if ((ltmp_153_8 > 6)) {
    goto ltmp_234_D;
  } else {
    goto ltmp_235_D;
  }

ltmp_235_D:
  *(&ltmp_151_1023->field7.field15) = 0;
  ltmp_183_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_184_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_183_7 > ltmp_184_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_234_D:
  switch (ltmp_153_8) {
  default:
    goto ltmp_237_D;
;
  case 7:
    goto ltmp_238_D;
    break;
  case 8:
    goto ltmp_239_D;
  case 9:
    goto ltmp_240_D;
  }
ltmp_238_D:
  *(&ltmp_151_1023->field7.field15) = 1;
  ltmp_185_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_186_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_185_7 > ltmp_186_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_239_D:
  *(&ltmp_151_1023->field7.field15) = 2;
  ltmp_187_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_188_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_187_7 > ltmp_188_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_240_D:
  *(&ltmp_151_1023->field7.field15) = 4;
  ltmp_189_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_190_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_189_7 > ltmp_190_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_237_D:
  if ((ltmp_153_8 > 9)) {
    goto ltmp_241_D;
  } else {
    goto ltmp_232_D;
  }

ltmp_241_D:
  *(&ltmp_151_1023->field7.field15) = 6;
  ltmp_191_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_192_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_191_7 > ltmp_192_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_232_D:
  ltmp_193_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_194_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_193_7 > ltmp_194_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_236_D:
  ltmp_195_7 = *(&ltmp_151_1023->field7.field4);
  ltmp_196_7 = *(&ltmp_151_1023->field6.field5);
  if ((ltmp_195_7 > ltmp_196_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_242_D;
  }

ltmp_242_D:
  ltmp_197_7 = *(&ltmp_151_1023->field7.field3);
  ltmp_198_7 = *(&ltmp_151_1023->field6.field4);
  if ((ltmp_197_7 < ltmp_198_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_243_D;
  }

ltmp_243_D:
  return ((((ltmp_197_7 > ltmp_195_7)) ? (-2) : (ltmp_156_8)));
ltmp_201_D:
  ltmp_199_8 = ltmp_199_8_2E;
  return ltmp_199_8;
}


static plzo_int32_t ltmp_4_1004(struct l_struct_2E_E *ltmp_244_1023, unsigned char *ltmp_245_13, plzo_uint32_t ltmp_246_7, unsigned char *ltmp_247_13, plzo_uint32_t *ltmp_248_17) {
  plzo_uint32_t *ltmp_249_17;
  unsigned char **ltmp_250_1025;
  unsigned char **ltmp_251_1025;
  plzo_uint32_t *ltmp_252_17;
  plzo_int32_t  (***ltmp_253_102C) ();
  plzo_int32_t  (**ltmp_254_1029) ();
  plzo_int32_t  (*ltmp_255_102A) ();
  plzo_int32_t ltmp_256_8;
  plzo_int32_t ltmp_257_8;
  plzo_int32_t  (**ltmp_258_1029) ();
  plzo_int32_t  (*ltmp_259_102A) ();
  plzo_uint32_t ltmp_260_7;
  plzo_uint32_t ltmp_261_7;
  plzo_int32_t  (**ltmp_262_1029) ();
  plzo_int32_t  (*ltmp_263_102A) ();
  plzo_int32_t ltmp_264_8;
  plzo_int32_t ltmp_265_8;
  plzo_int32_t ltmp_265_8_2E;
  plzo_int32_t  (**ltmp_266_1029) ();
  plzo_int32_t  (*ltmp_267_102A) ();
  plzo_int32_t ltmp_268_8;
  plzo_int32_t ltmp_268_8_2E;
  unsigned char *ltmp_269_13;
  unsigned char *ltmp_270_13;
  plzo_int64_t ltmp_271_A;
  plzo_uint64_t ltmp_272_9;
  plzo_uint64_t ltmp_273_9;
  signed char *ltmp_274_14;
  struct plzo_callback_t *ltmp_275_1026;
  void  (*ltmp_276_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t );
  plzo_int32_t ltmp_277_8;
  plzo_int32_t ltmp_277_8_2E;

  *(&ltmp_244_1023->field10) = ltmp_245_13;
  *(&ltmp_244_1023->field8) = ltmp_245_13;
  ltmp_249_17 = &ltmp_244_1023->field11;
  *ltmp_249_17 = ltmp_246_7;
  ltmp_250_1025 = &ltmp_244_1023->field9;
  ltmp_251_1025 = &ltmp_244_1023->field1.field6;
  *ltmp_251_1025 = ltmp_247_13;
  *ltmp_250_1025 = ltmp_247_13;
  *(&ltmp_244_1023->field12) = 0u;
  ltmp_252_17 = &ltmp_244_1023->field13;
  *ltmp_252_17 = 0u;
  ltmp_253_102C = &ltmp_244_1023->field0;
  ltmp_254_1029 = *ltmp_253_102C;
  ltmp_255_102A = *(&ltmp_254_1029[PLZO_INT64_C(7)]);
  ltmp_256_8 = ((plzo_int32_t  (*) (struct l_struct_2E_E *))ltmp_255_102A)(ltmp_244_1023);
  if ((ltmp_256_8 == 0)) {
    goto ltmp_278_D;
  } else {
    ltmp_277_8_2E = ltmp_256_8;
    goto ltmp_279_D;
  }

ltmp_278_D:
  ltmp_257_8 = *(&ltmp_244_1023->field3);
  if ((ltmp_257_8 == 0)) {
    ltmp_277_8_2E = -1;
    goto ltmp_279_D;
  } else {
    goto ltmp_280_D;
  }

ltmp_280_D:
  ltmp_258_1029 = *ltmp_253_102C;
  ltmp_259_102A = *(&ltmp_258_1029[PLZO_INT64_C(8)]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_259_102A)(ltmp_244_1023);
  ltmp_260_7 = *ltmp_249_17;
  ltmp_261_7 = *(&ltmp_244_1023->field6.field2);
  if ((ltmp_260_7 < ltmp_261_7)) {
    goto ltmp_281_D;
  } else {
    goto ltmp_282_D;
  }

ltmp_282_D:
  ltmp_262_1029 = *ltmp_253_102C;
  ltmp_263_102A = *(&ltmp_262_1029[PLZO_INT64_C(9)]);
  ltmp_264_8 = ((plzo_int32_t  (*) (struct l_struct_2E_E *))ltmp_263_102A)(ltmp_244_1023);
  if ((ltmp_264_8 == 0)) {
    ltmp_265_8_2E = ltmp_264_8;
    goto ltmp_283_D;
  } else {
    ltmp_268_8_2E = ltmp_264_8;
    goto ltmp_284_D;
  }

ltmp_281_D:
  *ltmp_252_17 = ltmp_260_7;
  if ((ltmp_256_8 == 0)) {
    ltmp_265_8_2E = ltmp_256_8;
    goto ltmp_283_D;
  } else {
    ltmp_268_8_2E = ltmp_256_8;
    goto ltmp_284_D;
  }

ltmp_283_D:
  ltmp_265_8 = ltmp_265_8_2E;
  ltmp_266_1029 = *ltmp_253_102C;
  ltmp_267_102A = *(&ltmp_266_1029[PLZO_INT64_C(10)]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_267_102A)(ltmp_244_1023);
  ltmp_268_8_2E = ltmp_265_8;
  goto ltmp_284_D;

ltmp_284_D:
  ltmp_268_8 = ltmp_268_8_2E;
  ltmp_269_13 = *ltmp_251_1025;
  ltmp_270_13 = *ltmp_250_1025;
  ltmp_271_A = plzo_external_ptrdiff_uchar(ltmp_269_13, ltmp_270_13, PLZO_UINT64_C(1));
  *ltmp_248_17 = ((plzo_uint32_t )ltmp_271_A);
  ltmp_272_9 = (plzo_uint64_t )ltmp_246_7;
  ltmp_273_9 = ((plzo_uint64_t )ltmp_271_A) & PLZO_UINT64_C(4294967295);
  ltmp_274_14 = *(&ltmp_244_1023->field24.field2.field3);
  ltmp_275_1026 = (struct plzo_callback_t *)ltmp_274_14;
  if ((ltmp_274_14 == ((signed char *)0))) {
    ltmp_277_8_2E = ltmp_268_8;
    goto ltmp_279_D;
  } else {
    goto ltmp_285_D;
  }

ltmp_285_D:
  ltmp_276_101A = *(&ltmp_275_1026->field2);
  if ((ltmp_276_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t ))0))) {
    ltmp_277_8_2E = ltmp_268_8;
    goto ltmp_279_D;
  } else {
    goto ltmp_286_D;
  }

ltmp_286_D:
  ltmp_276_101A(ltmp_275_1026, ltmp_272_9, ltmp_273_9, 4);
  return ltmp_268_8;
ltmp_279_D:
  ltmp_277_8 = ltmp_277_8_2E;
  return ltmp_277_8;
}


static void ltmp_5_1005(struct l_struct_2E_E *ltmp_287_1023) {
  unsigned char **ltmp_288_1025;
  unsigned char *ltmp_289_13;
  signed char *ltmp_290_14;
  signed char **ltmp_291_102D;
  signed char *ltmp_292_14;
  struct plzo_callback_t *ltmp_293_1026;
  void  (*ltmp_294_1019) (struct plzo_callback_t *, signed char *);
  unsigned char **ltmp_295_1025;
  unsigned char *ltmp_296_13;
  signed char *ltmp_297_14;
  signed char *ltmp_298_14;
  struct plzo_callback_t *ltmp_299_1026;
  void  (*ltmp_300_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2 *ltmp_301_102E;
  struct l_struct_2E_A9 *ltmp_302_102F;
  struct l_struct_2E_A2 *ltmp_303_102E;
  struct l_struct_2E_A9 *ltmp_304_102F;
  struct l_struct_2E_A2 *ltmp_305_102E;
  struct l_struct_2E_A2 *ltmp_305_102E_2E;
  struct l_struct_2E_A9 *ltmp_306_102F;
  void  (*ltmp_307_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_308_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_309_1031;

  *(&ltmp_287_1023->field0) = (&(ltmp_70_1021[PLZO_INT64_C(2)]));
  ltmp_288_1025 = &ltmp_287_1023->field22.field0;
  ltmp_289_13 = *ltmp_288_1025;
  ltmp_290_14 = (signed char *)ltmp_289_13;
  ltmp_291_102D = &ltmp_287_1023->field24.field2.field3;
  ltmp_292_14 = *ltmp_291_102D;
  ltmp_293_1026 = (struct plzo_callback_t *)ltmp_292_14;
  if (((ltmp_292_14 == ((signed char *)0)) | (ltmp_289_13 == ((unsigned char *)0)))) {
    goto ltmp_310_D;
  } else {
    goto ltmp_311_D;
  }

ltmp_311_D:
  ltmp_294_1019 = *(&ltmp_293_1026->field1);
  if ((ltmp_294_1019 == ((void  (*) (struct plzo_callback_t *, signed char *))0))) {
    goto ltmp_310_D;
  } else {
    goto ltmp_312_D;
  }

ltmp_312_D:
  ltmp_294_1019(ltmp_293_1026, ltmp_290_14);
  *ltmp_288_1025 = ((unsigned char *)0);
  *(&ltmp_287_1023->field22.field1) = 0u;
  goto ltmp_313_D;

ltmp_310_D:
  *ltmp_288_1025 = ((unsigned char *)0);
  *(&ltmp_287_1023->field22.field1) = 0u;
  goto ltmp_313_D;

ltmp_313_D:
  ltmp_295_1025 = &ltmp_287_1023->field23.field0;
  ltmp_296_13 = *ltmp_295_1025;
  ltmp_297_14 = (signed char *)ltmp_296_13;
  ltmp_298_14 = *ltmp_291_102D;
  ltmp_299_1026 = (struct plzo_callback_t *)ltmp_298_14;
  if (((ltmp_298_14 == ((signed char *)0)) | (ltmp_296_13 == ((unsigned char *)0)))) {
    goto ltmp_314_D;
  } else {
    goto ltmp_315_D;
  }

ltmp_315_D:
  ltmp_300_1019 = *(&ltmp_299_1026->field1);
  if ((ltmp_300_1019 == ((void  (*) (struct plzo_callback_t *, signed char *))0))) {
    goto ltmp_314_D;
  } else {
    goto ltmp_316_D;
  }

ltmp_316_D:
  ltmp_300_1019(ltmp_299_1026, ltmp_297_14);
  *ltmp_295_1025 = ((unsigned char *)0);
  *(&ltmp_287_1023->field23.field1) = 0u;
  ltmp_301_102E = &ltmp_287_1023->field20;
  ltmp_302_102F = *(&ltmp_287_1023->field20.field4);
  if ((ltmp_302_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_317_D;
  } else {
    ltmp_305_102E_2E = ltmp_301_102E;
    goto ltmp_318_D;
  }

ltmp_314_D:
  *ltmp_295_1025 = ((unsigned char *)0);
  *(&ltmp_287_1023->field23.field1) = 0u;
  ltmp_303_102E = &ltmp_287_1023->field20;
  ltmp_304_102F = *(&ltmp_287_1023->field20.field4);
  if ((ltmp_304_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_317_D;
  } else {
    ltmp_305_102E_2E = ltmp_303_102E;
    goto ltmp_318_D;
  }

ltmp_318_D:
  ltmp_305_102E = ltmp_305_102E_2E;
  ltmp_306_102F = *(&ltmp_305_102E->field4);
  ltmp_307_1019 = *(&ltmp_306_102F->field2.field1);
  ltmp_308_1030 = &ltmp_305_102E->field3;
  ltmp_309_1031 = *ltmp_308_1030;
  ltmp_307_1019((&ltmp_306_102F->field2), ((signed char *)ltmp_309_1031));
  *ltmp_308_1030 = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  return;
ltmp_317_D:
  return;
}


static struct l_struct_2E_E_3A__3A_A12 *ltmp_6_1006(struct l_struct_2E_E *ltmp_319_1023, plzo_int32_t ltmp_320_8) {

  if (((((plzo_uint32_t )ltmp_320_8) + 4294967295u) > 9u)) {
    goto ltmp_321_D;
  } else {
    goto ltmp_322_D;
  }

ltmp_322_D:
  return (&ltmp_71_1022[(ltmp_320_8 + -1)]);
ltmp_321_D:
  return ((struct l_struct_2E_E_3A__3A_A12 *)0);
}


static void ltmp_7_1005(struct l_struct_2E_E *ltmp_323_1023) {
  plzo_uint32_t *ltmp_324_17;
  plzo_uint32_t ltmp_325_7;
  plzo_uint32_t ltmp_326_7;
  plzo_uint32_t ltmp_326_7_2E;
  plzo_uint32_t ltmp_327_7;
  plzo_uint32_t ltmp_327_7_2E;
  plzo_int64_t ltmp_328_A;
  plzo_uint32_t ltmp_329_7;
  plzo_uint32_t ltmp_330_7;
  plzo_uint32_t *ltmp_331_17;
  plzo_uint32_t ltmp_332_7;
  plzo_uint32_t ltmp_333_7;
  plzo_uint32_t ltmp_334_7;
  plzo_uint32_t ltmp_334_7_2E;

  ltmp_324_17 = &ltmp_323_1023->field5;
  ltmp_325_7 = *(&ltmp_323_1023->field7.field3);
  *ltmp_324_17 = ltmp_325_7;
  ltmp_326_7_2E = ltmp_325_7;
  ltmp_327_7_2E = 0u;
  goto ltmp_335_D;

  do {
ltmp_335_D:
  ltmp_326_7 = ltmp_326_7_2E;
  ltmp_327_7 = ltmp_327_7_2E;
  ltmp_328_A = (plzo_int64_t )ltmp_327_7;
  if ((ltmp_325_7 > ltmp_327_7)) {
    goto ltmp_336_D;
  } else {
    goto ltmp_337_D;
  }

ltmp_336_D:
  *(&ltmp_323_1023->field4[ltmp_328_A]) = 0u;
  ltmp_329_7 = ltmp_327_7 + 1u;
  if ((ltmp_329_7 > 16u)) {
    ltmp_334_7_2E = ltmp_326_7;
    goto ltmp_338_D;
  } else {
    ltmp_326_7_2E = ltmp_326_7;
    ltmp_327_7_2E = ltmp_329_7;
    goto ltmp_335_D;
  }

ltmp_339_D:
  *ltmp_331_17 = (ltmp_330_7 + 1u);
  ltmp_332_7 = ltmp_327_7 + 1u;
  if ((ltmp_332_7 > 16u)) {
    ltmp_334_7_2E = ltmp_327_7;
    goto ltmp_338_D;
  } else {
    ltmp_326_7_2E = ltmp_327_7;
    ltmp_327_7_2E = ltmp_332_7;
    goto ltmp_335_D;
  }

ltmp_337_D:
  ltmp_330_7 = *(&ltmp_323_1023->field6.field7[ltmp_328_A]);
  ltmp_331_17 = &ltmp_323_1023->field4[ltmp_328_A];
  if ((ltmp_330_7 == 0u)) {
    goto ltmp_340_D;
  } else {
    goto ltmp_339_D;
  }

ltmp_340_D:
  *ltmp_331_17 = 4294967295u;
  ltmp_333_7 = ltmp_327_7 + 1u;
  if ((ltmp_333_7 > 16u)) {
    ltmp_334_7_2E = ltmp_326_7;
    goto ltmp_338_D;
  } else {
    ltmp_326_7_2E = ltmp_326_7;
    ltmp_327_7_2E = ltmp_333_7;
    goto ltmp_335_D;
  }

  } while (1);
ltmp_338_D:
  ltmp_334_7 = ltmp_334_7_2E;
  *ltmp_324_17 = ltmp_334_7;
  return;
}


static void ltmp_8_1005(struct l_struct_2E_E *ltmp_341_1023) {

  return;
}


static plzo_int32_t ltmp_9_1007(struct l_struct_2E_E *ltmp_342_1023) {
  struct l_struct_2E_M ltmp_343_1032;
  struct l_struct_2E_M ltmp_344_1032;
  void  (**ltmp_345_1033) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (*ltmp_346_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  plzo_uint32_t ltmp_347_7;
  plzo_uint32_t ltmp_348_7;
  void  (*ltmp_349_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (**ltmp_350_1033) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (*ltmp_351_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (*ltmp_352_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  plzo_int32_t  (***ltmp_353_102C) ();
  plzo_int32_t  (**ltmp_354_1029) ();
  plzo_int32_t  (*ltmp_355_102A) ();
  plzo_int32_t ltmp_356_8;
  plzo_uint32_t *ltmp_357_17;
  plzo_uint32_t ltmp_358_7;
  plzo_int32_t  (**ltmp_359_1029) ();
  plzo_int32_t  (*ltmp_360_102A) ();
  plzo_uint32_t ltmp_361_7;
  plzo_int32_t ltmp_362_8;
  plzo_int32_t ltmp_362_8_2E;
  struct l_struct_2E_A8 *ltmp_363_1034;
  unsigned char *ltmp_364_13;
  plzo_uint64_t ltmp_365_9;
  plzo_uint64_t ltmp_365_9_2E;
  plzo_uint64_t ltmp_366_9;
  struct l_struct_2E_A9 **ltmp_367_1035;
  struct l_struct_2E_A9 *ltmp_368_102F;
  unsigned char *ltmp_369_13;
  plzo_uint32_t *ltmp_370_17;
  plzo_uint32_t ltmp_371_7;
  signed char * (*ltmp_372_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_373_14;
  struct l_struct_2E_A6 *ltmp_374_1036;
  unsigned char *ltmp_375_13;
  plzo_uint64_t ltmp_376_9;
  plzo_uint64_t ltmp_376_9_2E;
  plzo_uint64_t ltmp_377_9;
  plzo_uint32_t *ltmp_378_17;
  plzo_uint32_t *ltmp_379_17;
  plzo_uint32_t ltmp_380_7;
  plzo_int32_t ltmp_381_8;
  plzo_int32_t ltmp_381_8_2E;
  plzo_uint32_t *ltmp_382_17;
  plzo_uint32_t ltmp_383_7;
  plzo_uint32_t ltmp_384_7;
  plzo_uint32_t ltmp_385_7;
  plzo_uint32_t ltmp_386_7;
  plzo_uint32_t ltmp_387_7;
  plzo_uint32_t ltmp_388_7;
  struct l_struct_2E_A6 *ltmp_389_1036;
  plzo_uint32_t ltmp_390_7;
  plzo_uint32_t ltmp_391_7;
  plzo_uint32_t ltmp_392_7;
  plzo_uint32_t ltmp_392_7_2E;
  plzo_uint32_t ltmp_393_7;
  plzo_uint32_t *ltmp_394_17;
  plzo_uint32_t *ltmp_395_17;
  plzo_uint32_t *ltmp_396_17;
  plzo_uint32_t *ltmp_397_17;
  plzo_uint32_t *ltmp_398_17;
  plzo_uint32_t ltmp_399_7;
  plzo_uint32_t ltmp_400_7;
  plzo_uint32_t ltmp_401_7;
  plzo_uint32_t ltmp_402_7;
  plzo_uint32_t ltmp_402_7_2E;
  plzo_uint32_t *ltmp_403_17;
  plzo_uint32_t ltmp_404_7;
  plzo_uint32_t ltmp_405_7;
  struct l_struct_2E_A7 **ltmp_406_1037;
  struct l_struct_2E_A9 *ltmp_407_102F;
  signed char * (*ltmp_408_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  plzo_uint32_t ltmp_409_7;
  signed char *ltmp_410_14;
  plzo_uint32_t *ltmp_411_17;
  plzo_uint32_t ltmp_412_7;
  plzo_uint32_t ltmp_413_7;
  struct l_struct_2E_A9 *ltmp_414_102F;
  signed char * (*ltmp_415_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_416_14;
  struct l_struct_2E_A9 *ltmp_417_102F;
  void  (*ltmp_418_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A7 *ltmp_419_1038;
  plzo_uint32_t ltmp_420_7;
  plzo_uint32_t ltmp_421_7;
  plzo_int32_t *ltmp_422_18;
  plzo_uint32_t ltmp_423_7;
  plzo_uint32_t ltmp_424_7;
  unsigned char *ltmp_425_13;
  plzo_uint64_t ltmp_426_9;
  plzo_uint64_t ltmp_426_9_2E;
  plzo_uint64_t ltmp_427_9;
  unsigned char *ltmp_428_13;
  plzo_uint64_t ltmp_429_9;
  plzo_uint64_t ltmp_429_9_2E;
  plzo_uint64_t ltmp_430_9;
  struct l_struct_2E_A7 **ltmp_431_1037;
  struct l_struct_2E_A7 *ltmp_432_1038;
  unsigned char *ltmp_433_13;
  plzo_uint64_t ltmp_434_9;
  plzo_uint64_t ltmp_434_9_2E;
  plzo_uint64_t ltmp_435_9;
  struct l_struct_2E_A7 *ltmp_436_1038;
  plzo_uint32_t ltmp_437_7;
  unsigned char *ltmp_438_13;
  plzo_uint64_t ltmp_439_9;
  plzo_uint64_t ltmp_439_9_2E;
  plzo_uint64_t ltmp_440_9;
  plzo_int32_t ltmp_441_8;
  plzo_int32_t ltmp_441_8_2E;
  plzo_uint32_t ltmp_442_7;
  plzo_int32_t ltmp_443_8;
  plzo_int32_t ltmp_443_8_2E;
  struct l_struct_2E_A9 **ltmp_444_1035;
  signed char **ltmp_445_102D;
  plzo_uint32_t ltmp_446_7;
  plzo_uint32_t ltmp_447_7;
  plzo_uint32_t ltmp_448_7;
  signed char * (*ltmp_449_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_450_14;
  plzo_int32_t ltmp_451_8;
  signed char *ltmp_452_14;
  signed char *ltmp_453_14;
  plzo_int32_t ltmp_454_8;
  plzo_int32_t ltmp_454_8_2E;
  signed char *ltmp_455_14;
  signed char *ltmp_455_14_2E;
  struct plzo_callback_t *ltmp_456_1026;
  void  (*ltmp_457_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t );
  plzo_int32_t ltmp_458_8;
  plzo_int32_t ltmp_458_8_2E;
  plzo_int32_t ltmp_459_8;
  plzo_int32_t ltmp_459_8_2E;
  plzo_uint32_t *ltmp_460_17;
  unsigned char **ltmp_461_1025;
  unsigned char **ltmp_462_1025;
  plzo_uint32_t *ltmp_463_17;
  plzo_uint32_t *ltmp_464_17;
  struct l_struct_2E_A6 **ltmp_465_1039;
  plzo_uint32_t *ltmp_466_17;
  struct l_struct_2E_A7 **ltmp_467_1037;
  struct l_struct_2E_M **ltmp_468_103A;
  struct l_struct_2E_M **ltmp_469_103A;
  struct l_struct_2E_M **ltmp_470_103A;
  struct l_struct_2E_M **ltmp_471_103A;
  plzo_uint32_t ltmp_472_7;
  plzo_uint32_t ltmp_472_7_2E;
  plzo_int32_t ltmp_473_8;
  plzo_int32_t ltmp_473_8_2E;
  plzo_uint32_t ltmp_474_7;
  struct l_struct_2E_A6 *ltmp_475_1036;
  plzo_uint32_t ltmp_476_7;
  plzo_uint32_t ltmp_477_7;
  plzo_uint32_t ltmp_478_7;
  plzo_uint32_t ltmp_479_7;
  plzo_uint32_t *ltmp_480_17;
  struct l_struct_2E_A7 *ltmp_481_1038;
  plzo_uint32_t *ltmp_482_17;
  plzo_uint32_t ltmp_483_7;
  plzo_uint32_t ltmp_484_7;
  plzo_uint32_t ltmp_485_7;
  plzo_uint32_t ltmp_486_7;
  plzo_uint32_t ltmp_487_7;
  unsigned char *ltmp_488_13;
  unsigned char *ltmp_489_13;
  plzo_uint64_t ltmp_490_9;
  plzo_uint64_t ltmp_491_9;
  plzo_uint64_t ltmp_491_9_2E;
  unsigned char ltmp_492_3;
  plzo_uint64_t ltmp_493_9;
  plzo_uint64_t ltmp_494_9;
  plzo_uint64_t ltmp_494_9_2E;
  unsigned char ltmp_495_3;
  plzo_uint64_t ltmp_496_9;
  struct l_struct_2E_A7 *ltmp_497_1038;
  plzo_uint32_t ltmp_498_7;
  plzo_uint32_t ltmp_499_7;
  plzo_uint32_t ltmp_499_7_2E;
  plzo_uint32_t ltmp_500_7;
  plzo_uint32_t ltmp_500_7_2E;
  struct l_struct_2E_A7 *ltmp_501_1038;
  struct l_struct_2E_A7 *ltmp_501_1038_2E;
  plzo_uint32_t *ltmp_502_17;
  struct l_struct_2E_A6 *ltmp_503_1036;
  plzo_uint32_t ltmp_504_7;
  plzo_uint32_t *ltmp_505_17;
  plzo_uint32_t ltmp_506_7;
  plzo_uint32_t *ltmp_507_17;
  plzo_uint32_t *ltmp_508_17;
  unsigned char **ltmp_509_1025;
  plzo_uint32_t *ltmp_510_17;
  plzo_uint32_t *ltmp_511_17;
  unsigned char **ltmp_512_1025;
  unsigned char **ltmp_513_1025;
  plzo_int32_t *ltmp_514_18;
  plzo_uint32_t *ltmp_515_17;
  plzo_uint32_t *ltmp_516_17;
  plzo_uint32_t *ltmp_517_17;
  struct l_struct_2E_M **ltmp_518_103A;
  plzo_uint32_t *ltmp_519_17;
  plzo_uint32_t *ltmp_520_17;
  plzo_uint32_t *ltmp_521_17;
  plzo_uint32_t *ltmp_522_17;
  plzo_uint32_t ltmp_523_7;
  plzo_uint32_t ltmp_523_7_2E;
  struct l_struct_2E_A7 *ltmp_524_1038;
  struct l_struct_2E_A7 *ltmp_524_1038_2E;
  plzo_uint32_t ltmp_525_7;
  plzo_uint32_t ltmp_526_7;
  plzo_uint32_t ltmp_527_7;
  plzo_uint32_t ltmp_528_7;
  plzo_uint32_t *ltmp_529_17;
  struct l_struct_2E_M *ltmp_530_1032;
  struct l_struct_2E_M **ltmp_531_103A;
  struct l_struct_2E_M **ltmp_532_103A;
  unsigned char *ltmp_533_13;
  plzo_uint64_t ltmp_534_9;
  plzo_int64_t ltmp_535_A;
  unsigned char *ltmp_536_13;
  plzo_uint32_t ltmp_537_7;
  unsigned char ltmp_538_3;
  plzo_uint32_t *ltmp_539_17;
  plzo_uint32_t ltmp_540_7;
  plzo_uint32_t ltmp_541_7;
  plzo_uint32_t ltmp_542_7;
  plzo_uint32_t ltmp_543_7;
  plzo_uint32_t ltmp_543_7_2E;
  unsigned char ltmp_544_3;
  unsigned char ltmp_545_3;
  plzo_uint32_t *ltmp_546_17;
  plzo_uint32_t ltmp_547_7;
  plzo_uint32_t ltmp_548_7;
  plzo_uint32_t ltmp_549_7;
  unsigned char *ltmp_550_13;
  plzo_int32_t ltmp_551_8;
  plzo_int64_t ltmp_552_A;
  plzo_uint32_t ltmp_553_7;
  plzo_uint32_t ltmp_554_7;
  plzo_uint32_t ltmp_555_7;
  plzo_uint32_t ltmp_556_7;
  plzo_uint32_t ltmp_557_7;
  plzo_uint32_t ltmp_558_7;
  plzo_uint32_t ltmp_559_7;
  plzo_uint32_t ltmp_559_7_2E;
  struct l_struct_2E_M *ltmp_560_1032;
  struct l_struct_2E_M *ltmp_561_1032;
  struct l_struct_2E_M *ltmp_562_1032;
  struct l_struct_2E_M *ltmp_562_1032_2E;
  struct l_struct_2E_M *ltmp_563_1032;
  plzo_int64_t ltmp_564_A;
  plzo_uint32_t ltmp_565_7;
  plzo_uint32_t ltmp_566_7;
  plzo_uint32_t ltmp_567_7;
  plzo_uint32_t ltmp_568_7;
  plzo_uint32_t ltmp_569_7;
  plzo_uint32_t ltmp_569_7_2E;
  plzo_uint32_t ltmp_570_7;
  plzo_int32_t ltmp_571_8;
  unsigned char *ltmp_572_13;
  struct l_struct_2E_M *ltmp_573_1032;
  plzo_int64_t ltmp_574_A;
  plzo_uint32_t ltmp_575_7;
  plzo_uint32_t ltmp_576_7;
  plzo_uint32_t ltmp_577_7;
  plzo_uint32_t ltmp_578_7;
  plzo_uint32_t ltmp_579_7;
  plzo_uint32_t ltmp_579_7_2E;
  plzo_uint32_t ltmp_580_7;
  unsigned char *ltmp_581_13;
  unsigned char *ltmp_582_13;
  plzo_int64_t ltmp_583_A;
  unsigned char *ltmp_584_13;
  unsigned char *ltmp_584_13_2E;
  plzo_uint32_t ltmp_585_7;
  plzo_uint32_t ltmp_585_7_2E;
  plzo_uint32_t ltmp_586_7;
  plzo_uint32_t ltmp_586_7_2E;
  plzo_uint32_t ltmp_587_7;
  plzo_uint32_t ltmp_587_7_2E;
  struct l_struct_2E_M *ltmp_588_1032;
  struct l_struct_2E_M *ltmp_588_1032_2E;
  struct l_struct_2E_M *ltmp_589_1032;
  struct l_struct_2E_M *ltmp_589_1032_2E;
  struct l_struct_2E_M *ltmp_590_1032;
  struct l_struct_2E_M *ltmp_590_1032_2E;
  plzo_uint32_t ltmp_591_7;
  unsigned char *ltmp_592_13;
  unsigned char *ltmp_592_13_2E;
  plzo_uint32_t ltmp_593_7;
  plzo_uint32_t ltmp_593_7_2E;
  plzo_uint32_t ltmp_594_7;
  plzo_uint32_t ltmp_594_7_2E;
  plzo_uint32_t ltmp_595_7;
  plzo_uint32_t ltmp_595_7_2E;
  struct l_struct_2E_M *ltmp_596_1032;
  struct l_struct_2E_M *ltmp_596_1032_2E;
  struct l_struct_2E_M *ltmp_597_1032;
  struct l_struct_2E_M *ltmp_597_1032_2E;
  struct l_struct_2E_M *ltmp_598_1032;
  struct l_struct_2E_M *ltmp_598_1032_2E;
  plzo_uint32_t ltmp_599_7;
  plzo_uint32_t ltmp_599_7_2E;
  plzo_uint32_t ltmp_600_7;
  plzo_uint32_t ltmp_600_7_2E;
  plzo_uint32_t ltmp_601_7;
  plzo_int64_t ltmp_602_A;
  unsigned char ltmp_603_3;
  unsigned char ltmp_604_3;
  plzo_uint32_t ltmp_605_7;
  plzo_uint32_t ltmp_606_7;
  plzo_uint32_t ltmp_607_7;
  plzo_uint32_t ltmp_607_7_2E;
  plzo_int32_t ltmp_608_8;
  plzo_uint32_t ltmp_609_7;
  plzo_uint32_t ltmp_609_7_2E;
  plzo_uint32_t ltmp_610_7;
  plzo_uint32_t ltmp_611_7;
  plzo_uint32_t ltmp_612_7;
  plzo_uint32_t ltmp_612_7_2E;
  plzo_uint32_t ltmp_613_7;
  plzo_uint32_t ltmp_613_7_2E;
  struct l_struct_2E_M *ltmp_614_1032;
  struct l_struct_2E_M *ltmp_615_1032;
  plzo_int64_t ltmp_616_A;
  plzo_uint32_t ltmp_617_7;
  plzo_uint32_t ltmp_618_7;
  plzo_uint32_t ltmp_619_7;
  plzo_uint32_t ltmp_620_7;
  plzo_uint32_t ltmp_621_7;
  plzo_uint32_t ltmp_621_7_2E;
  plzo_uint32_t ltmp_622_7;
  unsigned char *ltmp_623_13;
  unsigned char *ltmp_624_13;
  plzo_uint32_t ltmp_625_7;
  plzo_uint32_t ltmp_625_7_2E;
  plzo_uint32_t ltmp_626_7;
  plzo_uint32_t ltmp_627_7;
  plzo_uint32_t ltmp_628_7;
  plzo_uint32_t ltmp_628_7_2E;
  plzo_uint32_t ltmp_629_7;
  plzo_uint32_t ltmp_629_7_2E;
  struct l_struct_2E_M *ltmp_630_1032;
  struct l_struct_2E_M *ltmp_631_1032;
  plzo_int64_t ltmp_632_A;
  plzo_uint32_t ltmp_633_7;
  plzo_uint32_t ltmp_634_7;
  plzo_uint32_t ltmp_635_7;
  plzo_uint32_t ltmp_636_7;
  plzo_uint32_t ltmp_637_7;
  plzo_uint32_t ltmp_637_7_2E;
  plzo_uint32_t ltmp_638_7;
  unsigned char *ltmp_639_13;
  unsigned char *ltmp_640_13;
  plzo_uint32_t ltmp_641_7;
  plzo_uint32_t ltmp_641_7_2E;
  plzo_uint32_t ltmp_642_7;
  struct l_struct_2E_M **ltmp_643_103A;
  struct l_struct_2E_M *ltmp_644_1032;
  struct l_struct_2E_M *ltmp_645_1032;
  plzo_int64_t ltmp_646_A;
  plzo_uint32_t ltmp_647_7;
  plzo_uint32_t ltmp_648_7;
  plzo_uint32_t ltmp_649_7;
  plzo_uint32_t ltmp_650_7;
  plzo_uint32_t ltmp_651_7;
  plzo_uint32_t ltmp_651_7_2E;
  plzo_uint32_t ltmp_652_7;
  unsigned char *ltmp_653_13;
  struct l_struct_2E_M *ltmp_654_1032;
  struct l_struct_2E_M *ltmp_655_1032;
  struct l_struct_2E_M **ltmp_656_103A;
  struct l_struct_2E_M *ltmp_657_1032;
  struct l_struct_2E_M *ltmp_658_1032;
  plzo_int64_t ltmp_659_A;
  plzo_uint32_t ltmp_660_7;
  plzo_uint32_t ltmp_661_7;
  plzo_uint32_t ltmp_662_7;
  plzo_uint32_t ltmp_663_7;
  plzo_uint32_t ltmp_664_7;
  plzo_uint32_t ltmp_664_7_2E;
  plzo_uint32_t ltmp_665_7;
  unsigned char *ltmp_666_13;
  struct l_struct_2E_M *ltmp_667_1032;
  struct l_struct_2E_M *ltmp_668_1032;
  plzo_uint32_t ltmp_669_7;
  plzo_uint32_t ltmp_669_7_2E;
  struct l_struct_2E_M *ltmp_670_1032;
  struct l_struct_2E_M *ltmp_671_1032;
  struct l_struct_2E_M *ltmp_672_1032;
  struct l_struct_2E_M *ltmp_673_1032;
  struct l_struct_2E_M *ltmp_674_1032;
  plzo_int64_t ltmp_675_A;
  unsigned char *ltmp_676_13;
  plzo_int64_t ltmp_677_A;
  struct l_struct_2E_M *ltmp_678_1032;
  plzo_int64_t ltmp_679_A;
  unsigned char *ltmp_680_13;
  unsigned char *ltmp_681_13;
  unsigned char *ltmp_682_13;
  unsigned char *ltmp_682_13_2E;
  plzo_uint32_t ltmp_683_7;
  plzo_uint32_t ltmp_683_7_2E;
  plzo_uint32_t ltmp_684_7;
  plzo_uint32_t ltmp_684_7_2E;
  plzo_uint32_t ltmp_685_7;
  plzo_uint32_t ltmp_685_7_2E;
  struct l_struct_2E_M *ltmp_686_1032;
  struct l_struct_2E_M *ltmp_686_1032_2E;
  struct l_struct_2E_M *ltmp_687_1032;
  struct l_struct_2E_M *ltmp_687_1032_2E;
  struct l_struct_2E_M *ltmp_688_1032;
  struct l_struct_2E_M *ltmp_688_1032_2E;
  plzo_uint32_t ltmp_689_7;
  unsigned char *ltmp_690_13;
  unsigned char *ltmp_690_13_2E;
  plzo_uint32_t ltmp_691_7;
  plzo_uint32_t ltmp_691_7_2E;
  plzo_uint32_t ltmp_692_7;
  plzo_uint32_t ltmp_692_7_2E;
  plzo_uint32_t ltmp_693_7;
  plzo_uint32_t ltmp_693_7_2E;
  struct l_struct_2E_M *ltmp_694_1032;
  struct l_struct_2E_M *ltmp_694_1032_2E;
  struct l_struct_2E_M *ltmp_695_1032;
  struct l_struct_2E_M *ltmp_695_1032_2E;
  struct l_struct_2E_M *ltmp_696_1032;
  struct l_struct_2E_M *ltmp_696_1032_2E;
  plzo_uint32_t ltmp_697_7;
  plzo_uint32_t ltmp_697_7_2E;
  plzo_uint32_t ltmp_698_7;
  plzo_uint32_t ltmp_698_7_2E;
  plzo_uint32_t ltmp_699_7;
  plzo_int64_t ltmp_700_A;
  unsigned char ltmp_701_3;
  unsigned char ltmp_702_3;
  plzo_uint32_t ltmp_703_7;
  plzo_uint32_t ltmp_704_7;
  plzo_uint32_t ltmp_705_7;
  plzo_uint32_t ltmp_705_7_2E;
  plzo_int32_t ltmp_706_8;
  plzo_uint32_t ltmp_707_7;
  plzo_uint32_t ltmp_707_7_2E;
  plzo_uint32_t ltmp_708_7;
  plzo_uint32_t ltmp_709_7;
  plzo_uint32_t ltmp_710_7;
  plzo_uint32_t ltmp_710_7_2E;
  plzo_uint32_t ltmp_711_7;
  plzo_uint32_t ltmp_711_7_2E;
  struct l_struct_2E_M *ltmp_712_1032;
  struct l_struct_2E_M *ltmp_713_1032;
  plzo_int64_t ltmp_714_A;
  unsigned char *ltmp_715_13;
  unsigned char *ltmp_716_13;
  plzo_uint32_t ltmp_717_7;
  plzo_uint32_t ltmp_717_7_2E;
  plzo_uint32_t ltmp_718_7;
  plzo_uint32_t ltmp_719_7;
  plzo_uint32_t ltmp_720_7;
  plzo_uint32_t ltmp_720_7_2E;
  plzo_uint32_t ltmp_721_7;
  plzo_uint32_t ltmp_721_7_2E;
  struct l_struct_2E_M *ltmp_722_1032;
  struct l_struct_2E_M *ltmp_723_1032;
  plzo_int64_t ltmp_724_A;
  unsigned char *ltmp_725_13;
  unsigned char *ltmp_726_13;
  plzo_uint32_t ltmp_727_7;
  plzo_uint32_t ltmp_727_7_2E;
  plzo_uint32_t ltmp_728_7;
  struct l_struct_2E_M *ltmp_729_1032;
  struct l_struct_2E_M *ltmp_730_1032;
  plzo_uint32_t ltmp_731_7;
  plzo_uint32_t ltmp_731_7_2E;
  struct l_struct_2E_M *ltmp_732_1032;
  struct l_struct_2E_M *ltmp_733_1032;
  plzo_uint32_t ltmp_734_7;
  plzo_uint32_t ltmp_734_7_2E;
  plzo_uint32_t ltmp_735_7;
  plzo_uint32_t ltmp_736_7;
  plzo_uint32_t ltmp_737_7;
  unsigned char *ltmp_738_13;
  unsigned char *ltmp_739_13;
  plzo_uint32_t ltmp_740_7;
  plzo_uint32_t ltmp_741_7;
  plzo_uint32_t ltmp_742_7;
  plzo_uint32_t ltmp_742_7_2E;
  plzo_uint32_t ltmp_743_7;
  plzo_uint32_t ltmp_743_7_2E;
  plzo_uint32_t ltmp_744_7;
  plzo_uint32_t ltmp_744_7_2E;
  plzo_uint32_t ltmp_745_7;
  unsigned char *ltmp_746_13;
  plzo_int64_t ltmp_747_A;
  plzo_uint32_t ltmp_748_7;
  plzo_uint32_t ltmp_749_7;
  plzo_uint32_t ltmp_750_7;
  plzo_int64_t ltmp_751_A;
  plzo_uint32_t ltmp_752_7;
  plzo_uint32_t ltmp_753_7;
  plzo_uint32_t ltmp_754_7;
  plzo_uint32_t ltmp_755_7;
  plzo_uint32_t ltmp_755_7_2E;
  plzo_uint32_t ltmp_756_7;
  plzo_int64_t ltmp_757_A;
  unsigned char ltmp_758_3;
  unsigned char ltmp_759_3;
  plzo_uint32_t ltmp_760_7;
  plzo_uint32_t ltmp_761_7;
  plzo_uint32_t ltmp_762_7;
  plzo_uint32_t ltmp_762_7_2E;
  plzo_uint32_t ltmp_763_7;
  plzo_uint32_t ltmp_764_7;
  plzo_uint32_t ltmp_764_7_2E;
  plzo_uint32_t ltmp_765_7;
  plzo_uint32_t ltmp_765_7_2E;
  plzo_uint32_t ltmp_766_7;
  plzo_uint32_t ltmp_767_7;
  plzo_uint32_t ltmp_768_7;
  plzo_uint32_t ltmp_768_7_2E;
  plzo_uint32_t ltmp_769_7;
  plzo_uint32_t ltmp_769_7_2E;
  plzo_uint32_t ltmp_770_7;
  struct l_struct_2E_M *ltmp_771_1032;
  plzo_uint32_t ltmp_772_7;
  plzo_uint32_t ltmp_773_7;
  plzo_uint32_t ltmp_774_7;
  plzo_uint32_t ltmp_775_7;
  plzo_uint64_t ltmp_776_9;
  plzo_uint64_t ltmp_777_9;
  plzo_uint64_t ltmp_777_9_2E;
  plzo_int64_t ltmp_778_A;
  struct l_struct_2E_M *ltmp_779_1032;
  plzo_uint64_t ltmp_780_9;
  struct l_struct_2E_A7 *ltmp_781_1038;
  plzo_uint32_t ltmp_782_7;
  plzo_uint32_t ltmp_783_7;
  plzo_uint32_t ltmp_783_7_2E;
  struct l_struct_2E_A7 *ltmp_784_1038;
  struct l_struct_2E_A7 *ltmp_784_1038_2E;
  plzo_uint32_t ltmp_785_7;
  struct l_struct_2E_A7 *ltmp_786_1038;
  plzo_uint32_t ltmp_787_7;
  plzo_uint32_t ltmp_788_7;
  plzo_uint32_t ltmp_789_7;
  plzo_uint32_t ltmp_790_7;
  plzo_uint32_t ltmp_791_7;
  plzo_int32_t  (**ltmp_792_1029) ();
  plzo_int32_t  (*ltmp_793_102A) ();
  plzo_uint32_t ltmp_794_7;
  plzo_uint32_t ltmp_795_7;
  plzo_int32_t  (**ltmp_796_1029) ();
  plzo_int32_t  (*ltmp_797_102A) ();
  plzo_uint32_t ltmp_798_7;
  plzo_uint32_t ltmp_799_7;
  plzo_uint32_t ltmp_799_7_2E;
  plzo_uint64_t ltmp_800_9;
  unsigned char *ltmp_801_13;
  unsigned char *ltmp_802_13;
  plzo_int64_t ltmp_803_A;
  plzo_uint64_t ltmp_804_9;
  signed char *ltmp_805_14;
  struct plzo_callback_t *ltmp_806_1026;
  void  (*ltmp_807_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t );
  plzo_int32_t  (**ltmp_808_1029) ();
  plzo_int32_t  (*ltmp_809_102A) ();
  plzo_uint32_t ltmp_810_7;
  plzo_int32_t  (*ltmp_811_102A) ();
  plzo_uint32_t ltmp_812_7;
  plzo_int32_t  (**ltmp_813_1029) ();
  plzo_int32_t  (*ltmp_814_102A) ();
  plzo_uint32_t ltmp_815_7;
  plzo_int32_t  (*ltmp_816_102A) ();
  plzo_uint32_t ltmp_817_7;
  unsigned char *ltmp_818_13;
  unsigned char *ltmp_819_13;
  plzo_int64_t ltmp_820_A;
  plzo_uint64_t ltmp_821_9;
  signed char *ltmp_822_14;
  struct plzo_callback_t *ltmp_823_1026;
  void  (*ltmp_824_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t );
  plzo_int32_t  (**ltmp_825_1029) ();
  plzo_int32_t  (*ltmp_826_102A) ();
  plzo_int32_t  (**ltmp_827_1029) ();
  plzo_int32_t  (*ltmp_828_102A) ();
  plzo_int32_t  (*ltmp_829_102A) ();
  plzo_uint32_t ltmp_830_7;
  plzo_uint32_t ltmp_831_7;
  unsigned char *ltmp_832_13;
  unsigned char *ltmp_833_13;
  plzo_int64_t ltmp_834_A;
  plzo_uint64_t ltmp_835_9;
  signed char *ltmp_836_14;
  struct plzo_callback_t *ltmp_837_1026;
  void  (*ltmp_838_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t );
  plzo_int32_t ltmp_839_8;
  plzo_int32_t ltmp_839_8_2E;
  struct l_struct_2E_A9 *ltmp_840_102F;
  void  (*ltmp_841_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_842_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_843_1031;

  ltmp_345_1033 = &ltmp_342_1023->field22.field3;
  ltmp_346_1001 = *ltmp_345_1033;
  ltmp_347_7 = *(&ltmp_342_1023->field1.field2);
  ltmp_348_7 = *(&ltmp_342_1023->field1.field3);
  ltmp_349_1001 = ltmp_54_101D(ltmp_346_1001, ltmp_347_7, ltmp_348_7);
  *ltmp_345_1033 = ltmp_349_1001;
  ltmp_350_1033 = &ltmp_342_1023->field23.field3;
  ltmp_351_1001 = *ltmp_350_1033;
  ltmp_352_1001 = ltmp_54_101D(ltmp_351_1001, ltmp_347_7, ltmp_348_7);
  *ltmp_350_1033 = ltmp_352_1001;
  ltmp_353_102C = &ltmp_342_1023->field0;
  ltmp_354_1029 = *ltmp_353_102C;
  ltmp_355_102A = *(&ltmp_354_1029[PLZO_INT64_C(6)]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_355_102A)(ltmp_342_1023);
  ltmp_356_8 = *(&ltmp_342_1023->field7.field11);
  ltmp_357_17 = &ltmp_342_1023->field21;
  ltmp_358_7 = *ltmp_357_17;
  if ((ltmp_358_7 == 1u)) {
    goto ltmp_844_D;
  } else {
    ltmp_362_8_2E = 3;
    goto ltmp_845_D;
  }

ltmp_844_D:
  if ((ltmp_356_8 == 2)) {
    goto ltmp_846_D;
  } else {
    ltmp_362_8_2E = ltmp_356_8;
    goto ltmp_845_D;
  }

ltmp_846_D:
  ltmp_359_1029 = *ltmp_353_102C;
  ltmp_360_102A = *(&ltmp_359_1029[PLZO_INT64_C(18)]);
  ltmp_361_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *))ltmp_360_102A)(ltmp_342_1023);
  if ((ltmp_361_7 == 0u)) {
    goto ltmp_847_D;
  } else {
    ltmp_362_8_2E = ltmp_356_8;
    goto ltmp_845_D;
  }

ltmp_847_D:
  ltmp_362_8_2E = 3;
  goto ltmp_845_D;

ltmp_845_D:
  ltmp_362_8 = ltmp_362_8_2E;
  ltmp_363_1034 = &ltmp_342_1023->field19;
  ltmp_364_13 = (unsigned char *)ltmp_363_1034;
  ltmp_365_9_2E = PLZO_UINT64_C(0);
  goto ltmp_848_D;

  do {
ltmp_848_D:
  ltmp_365_9 = ltmp_365_9_2E;
  *(&ltmp_364_13[ltmp_365_9]) = 0;
  ltmp_366_9 = ltmp_365_9 + PLZO_UINT64_C(1);
  if ((ltmp_366_9 == PLZO_UINT64_C(56))) {
    goto ltmp_849_D;
  } else {
    ltmp_365_9_2E = ltmp_366_9;
    goto ltmp_848_D;
  }

  } while (1);
ltmp_849_D:
  ltmp_367_1035 = &ltmp_342_1023->field19.field6;
  ltmp_368_102F = &ltmp_342_1023->field24;
  *ltmp_367_1035 = ltmp_368_102F;
  ltmp_369_13 = *(&ltmp_342_1023->field8);
  ltmp_370_17 = &ltmp_342_1023->field11;
  ltmp_371_7 = *ltmp_370_17;
  if (((ltmp_371_7 + 4294967293u) > 2147483642u)) {
    ltmp_381_8_2E = -2;
    goto ltmp_850_D;
  } else {
    goto ltmp_851_D;
  }

ltmp_851_D:
  ltmp_372_1018 = *(&ltmp_342_1023->field24.field2.field0);
  ltmp_373_14 = ltmp_372_1018((&ltmp_342_1023->field24.field2), PLZO_UINT64_C(1), PLZO_UINT64_C(271480));
  ltmp_374_1036 = (struct l_struct_2E_A6 *)ltmp_373_14;
  *(&ltmp_342_1023->field19.field7) = ltmp_374_1036;
  if ((ltmp_373_14 == ((signed char *)0))) {
    ltmp_381_8_2E = -3;
    goto ltmp_850_D;
  } else {
    goto ltmp_852_D;
  }

ltmp_852_D:
  ltmp_375_13 = (unsigned char *)ltmp_373_14;
  ltmp_376_9_2E = PLZO_UINT64_C(0);
  goto ltmp_853_D;

  do {
ltmp_853_D:
  ltmp_376_9 = ltmp_376_9_2E;
  *(&ltmp_375_13[ltmp_376_9]) = 0;
  ltmp_377_9 = ltmp_376_9 + PLZO_UINT64_C(1);
  if ((ltmp_377_9 == PLZO_UINT64_C(271480))) {
    goto ltmp_854_D;
  } else {
    ltmp_376_9_2E = ltmp_377_9;
    goto ltmp_853_D;
  }

  } while (1);
ltmp_854_D:
  *(&ltmp_374_1036->field0) = ltmp_369_13;
  *(&ltmp_342_1023->field19.field4) = ltmp_369_13;
  ltmp_378_17 = &ltmp_374_1036->field2;
  *ltmp_378_17 = ltmp_371_7;
  *(&ltmp_342_1023->field19.field5) = ltmp_371_7;
  ltmp_379_17 = &ltmp_374_1036->field3;
  ltmp_380_7 = *ltmp_378_17;
  if ((ltmp_380_7 > 3u)) {
    goto ltmp_855_D;
  } else {
    goto ltmp_856_D;
  }

ltmp_855_D:
  *ltmp_379_17 = (ltmp_380_7 + 4294967293u);
  ltmp_381_8_2E = 0;
  goto ltmp_850_D;

ltmp_856_D:
  *ltmp_379_17 = 0u;
  ltmp_381_8_2E = 0;
  goto ltmp_850_D;

ltmp_850_D:
  ltmp_381_8 = ltmp_381_8_2E;
  if ((ltmp_381_8 == 0)) {
    goto ltmp_857_D;
  } else {
    ltmp_443_8_2E = ltmp_381_8;
    goto ltmp_858_D;
  }

ltmp_857_D:
  ltmp_382_17 = &ltmp_342_1023->field7.field1;
  ltmp_383_7 = *ltmp_382_17;
  ltmp_384_7 = *(&ltmp_342_1023->field7.field2);
  ltmp_385_7 = *(&ltmp_342_1023->field7.field3);
  ltmp_386_7 = *(&ltmp_342_1023->field7.field4);
  ltmp_387_7 = *(&ltmp_342_1023->field7.field5);
  ltmp_388_7 = *(&ltmp_342_1023->field7.field6);
  ltmp_389_1036 = *(&ltmp_342_1023->field19.field7);
  if ((ltmp_384_7 == 0u)) {
    goto ltmp_859_D;
  } else {
    goto ltmp_860_D;
  }

ltmp_860_D:
  ltmp_390_7 = *(&ltmp_389_1036->field2);
  if ((ltmp_390_7 < ltmp_384_7)) {
    goto ltmp_859_D;
  } else {
    ltmp_392_7_2E = ltmp_384_7;
    goto ltmp_861_D;
  }

ltmp_859_D:
  ltmp_391_7 = *(&ltmp_389_1036->field2);
  ltmp_392_7_2E = ltmp_391_7;
  goto ltmp_861_D;

ltmp_861_D:
  ltmp_392_7 = ltmp_392_7_2E;
  ltmp_393_7 = (((ltmp_386_7 == 0u)) ? (4294967295u) : (ltmp_386_7));
  ltmp_394_17 = &ltmp_389_1036->field14;
  *ltmp_394_17 = ltmp_392_7;
  ltmp_395_17 = &ltmp_389_1036->field12;
  *ltmp_395_17 = ltmp_385_7;
  ltmp_396_17 = &ltmp_389_1036->field13;
  *ltmp_396_17 = ltmp_393_7;
  ltmp_397_17 = &ltmp_389_1036->field15;
  ltmp_398_17 = &ltmp_389_1036->field16;
  if ((ltmp_387_7 == 0u)) {
    goto ltmp_862_D;
  } else {
    goto ltmp_863_D;
  }

ltmp_863_D:
  *ltmp_397_17 = ((((ltmp_393_7 <= ltmp_387_7)) ? (ltmp_393_7) : (ltmp_387_7)));
  if ((ltmp_388_7 == 0u)) {
    goto ltmp_864_D;
  } else {
    goto ltmp_865_D;
  }

ltmp_862_D:
  *ltmp_397_17 = 4294967295u;
  if ((ltmp_388_7 == 0u)) {
    goto ltmp_864_D;
  } else {
    goto ltmp_865_D;
  }

ltmp_865_D:
  ltmp_399_7 = (((ltmp_393_7 <= ltmp_388_7)) ? (ltmp_393_7) : (ltmp_388_7));
  *ltmp_398_17 = ltmp_399_7;
  if ((ltmp_399_7 > 1u)) {
    goto ltmp_866_D;
  } else {
    goto ltmp_867_D;
  }

ltmp_864_D:
  *ltmp_398_17 = 4294967295u;
  goto ltmp_866_D;

ltmp_867_D:
  *ltmp_398_17 = 2u;
  if ((ltmp_383_7 == 0u)) {
    goto ltmp_868_D;
  } else {
    goto ltmp_869_D;
  }

ltmp_866_D:
  if ((ltmp_383_7 == 0u)) {
    goto ltmp_868_D;
  } else {
    goto ltmp_869_D;
  }

ltmp_869_D:
  ltmp_400_7 = *(&ltmp_389_1036->field2);
  if ((ltmp_400_7 < ltmp_383_7)) {
    goto ltmp_868_D;
  } else {
    ltmp_402_7_2E = ltmp_383_7;
    goto ltmp_870_D;
  }

ltmp_868_D:
  ltmp_401_7 = *(&ltmp_389_1036->field2);
  ltmp_402_7_2E = ltmp_401_7;
  goto ltmp_870_D;

ltmp_870_D:
  ltmp_402_7 = ltmp_402_7_2E;
  ltmp_403_17 = &ltmp_342_1023->field19.field3;
  *(&ltmp_389_1036->field6) = ltmp_402_7;
  *ltmp_403_17 = ltmp_402_7;
  *(&ltmp_389_1036->field5) = 0u;
  *(&ltmp_342_1023->field19.field1) = 0u;
  *(&ltmp_342_1023->field19.field2) = 0u;
  *(&ltmp_389_1036->field8) = 0u;
  *(&ltmp_389_1036->field9) = 0u;
  *(&ltmp_389_1036->field10) = 0u;
  *(&ltmp_389_1036->field11) = 0u;
  *(&ltmp_389_1036->field25[PLZO_INT64_C(0)]) = ((unsigned char *)0);
  ltmp_404_7 = *ltmp_395_17;
  if ((ltmp_404_7 == 0u)) {
    ltmp_441_8_2E = -2;
    goto ltmp_871_D;
  } else {
    goto ltmp_872_D;
  }

ltmp_872_D:
  ltmp_405_7 = *ltmp_396_17;
  if (((ltmp_404_7 > ltmp_405_7) | (ltmp_405_7 < 4u))) {
    ltmp_441_8_2E = -2;
    goto ltmp_871_D;
  } else {
    goto ltmp_873_D;
  }

ltmp_873_D:
  ltmp_406_1037 = &ltmp_389_1036->field17;
  ltmp_407_102F = *ltmp_367_1035;
  ltmp_408_1018 = *(&ltmp_407_102F->field2.field0);
  ltmp_409_7 = *ltmp_403_17;
  ltmp_410_14 = ltmp_408_1018((&ltmp_407_102F->field2), ((plzo_uint64_t )(ltmp_409_7 + 2u)), PLZO_UINT64_C(36));
  *ltmp_406_1037 = ((struct l_struct_2E_A7 *)ltmp_410_14);
  if ((ltmp_410_14 == ((signed char *)0))) {
    ltmp_441_8_2E = -3;
    goto ltmp_871_D;
  } else {
    goto ltmp_874_D;
  }

ltmp_874_D:
  ltmp_411_17 = &ltmp_389_1036->field19;
  ltmp_412_7 = *ltmp_394_17;
  ltmp_413_7 = ltmp_412_7 + 1u;
  *ltmp_411_17 = ltmp_413_7;
  ltmp_414_102F = *ltmp_367_1035;
  ltmp_415_1018 = *(&ltmp_414_102F->field2.field0);
  ltmp_416_14 = ltmp_415_1018((&ltmp_414_102F->field2), ((plzo_uint64_t )ltmp_413_7), PLZO_UINT64_C(16));
  *(&ltmp_389_1036->field18) = ((struct l_struct_2E_M *)ltmp_416_14);
  if ((ltmp_416_14 == ((signed char *)0))) {
    goto ltmp_875_D;
  } else {
    goto ltmp_876_D;
  }

ltmp_875_D:
  ltmp_417_102F = *ltmp_367_1035;
  ltmp_418_1019 = *(&ltmp_417_102F->field2.field1);
  ltmp_419_1038 = *ltmp_406_1037;
  ltmp_418_1019((&ltmp_417_102F->field2), ((signed char *)ltmp_419_1038));
  *ltmp_406_1037 = ((struct l_struct_2E_A7 *)0);
  ltmp_441_8_2E = -3;
  goto ltmp_871_D;

ltmp_876_D:
  ltmp_420_7 = *ltmp_411_17;
  ltmp_421_7 = ltmp_420_7 + 4294967295u;
  *(&ltmp_389_1036->field20) = ltmp_421_7;
  *(&ltmp_389_1036->field21) = ltmp_421_7;
  *(&ltmp_389_1036->field1) = ltmp_421_7;
  ltmp_422_18 = &ltmp_389_1036->field22;
  *ltmp_422_18 = 1;
  ltmp_423_7 = *ltmp_394_17;
  ltmp_424_7 = *(&ltmp_389_1036->field2);
  if ((ltmp_423_7 == ltmp_424_7)) {
    goto ltmp_877_D;
  } else {
    goto ltmp_878_D;
  }

ltmp_877_D:
  *ltmp_422_18 = 0;
  goto ltmp_878_D;

ltmp_878_D:
  ltmp_425_13 = (unsigned char *)(&ltmp_389_1036->field26);
  ltmp_426_9_2E = PLZO_UINT64_C(0);
  goto ltmp_879_D;

  do {
ltmp_879_D:
  ltmp_426_9 = ltmp_426_9_2E;
  *(&ltmp_425_13[ltmp_426_9]) = 255;
  ltmp_427_9 = ltmp_426_9 + PLZO_UINT64_C(1);
  if ((ltmp_427_9 == PLZO_UINT64_C(262144))) {
    goto ltmp_880_D;
  } else {
    ltmp_426_9_2E = ltmp_427_9;
    goto ltmp_879_D;
  }

  } while (1);
ltmp_880_D:
  ltmp_428_13 = (unsigned char *)(&ltmp_389_1036->field27);
  ltmp_429_9_2E = PLZO_UINT64_C(0);
  goto ltmp_881_D;

  do {
ltmp_881_D:
  ltmp_429_9 = ltmp_429_9_2E;
  *(&ltmp_428_13[ltmp_429_9]) = 255;
  ltmp_430_9 = ltmp_429_9 + PLZO_UINT64_C(1);
  if ((ltmp_430_9 == PLZO_UINT64_C(1024))) {
    goto ltmp_882_D;
  } else {
    ltmp_429_9_2E = ltmp_430_9;
    goto ltmp_881_D;
  }

  } while (1);
ltmp_882_D:
  *(&ltmp_389_1036->field23) = 0u;
  *(&ltmp_389_1036->field24) = 0u;
  ltmp_431_1037 = &ltmp_342_1023->field19.field0;
  ltmp_432_1038 = *ltmp_406_1037;
  *ltmp_431_1037 = (&ltmp_432_1038[PLZO_INT64_C(1)]);
  ltmp_433_13 = (unsigned char *)ltmp_432_1038;
  ltmp_434_9_2E = PLZO_UINT64_C(0);
  goto ltmp_883_D;

  do {
ltmp_883_D:
  ltmp_434_9 = ltmp_434_9_2E;
  *(&ltmp_433_13[ltmp_434_9]) = 0;
  ltmp_435_9 = ltmp_434_9 + PLZO_UINT64_C(1);
  if ((ltmp_435_9 == PLZO_UINT64_C(36))) {
    goto ltmp_884_D;
  } else {
    ltmp_434_9_2E = ltmp_435_9;
    goto ltmp_883_D;
  }

  } while (1);
ltmp_884_D:
  ltmp_436_1038 = *ltmp_431_1037;
  ltmp_437_7 = *ltmp_403_17;
  ltmp_438_13 = (unsigned char *)(&ltmp_436_1038[((plzo_int64_t )ltmp_437_7)]);
  ltmp_439_9_2E = PLZO_UINT64_C(0);
  goto ltmp_885_D;

  do {
ltmp_885_D:
  ltmp_439_9 = ltmp_439_9_2E;
  *(&ltmp_438_13[ltmp_439_9]) = 0;
  ltmp_440_9 = ltmp_439_9 + PLZO_UINT64_C(1);
  if ((ltmp_440_9 == PLZO_UINT64_C(36))) {
    ltmp_441_8_2E = 0;
    goto ltmp_871_D;
  } else {
    ltmp_439_9_2E = ltmp_440_9;
    goto ltmp_885_D;
  }

  } while (1);
ltmp_871_D:
  ltmp_441_8 = ltmp_441_8_2E;
  ltmp_442_7 = *ltmp_403_17;
  *ltmp_382_17 = ltmp_442_7;
  ltmp_443_8_2E = ltmp_441_8;
  goto ltmp_858_D;

ltmp_858_D:
  ltmp_443_8 = ltmp_443_8_2E;
  ltmp_444_1035 = &ltmp_342_1023->field20.field4;
  *ltmp_444_1035 = ltmp_368_102F;
  ltmp_445_102D = &ltmp_342_1023->field24.field2.field3;
  if (((ltmp_443_8 == 0) & (ltmp_362_8 > 1))) {
    goto ltmp_886_D;
  } else {
    goto ltmp_887_D;
  }

ltmp_886_D:
  ltmp_446_7 = *(&ltmp_342_1023->field19.field3);
  ltmp_447_7 = ltmp_446_7 + 1u;
  ltmp_448_7 = *ltmp_357_17;
  *(&ltmp_342_1023->field20.field0) = ltmp_447_7;
  *(&ltmp_342_1023->field20.field1) = ltmp_448_7;
  *(&ltmp_342_1023->field20.field2) = 0u;
  ltmp_449_1018 = *(&ltmp_342_1023->field24.field2.field0);
  ltmp_450_14 = ltmp_449_1018((&ltmp_342_1023->field24.field2), ((plzo_uint64_t )(ltmp_448_7*ltmp_447_7)), PLZO_UINT64_C(16));
  *(&ltmp_342_1023->field20.field3) = ((struct l_struct_2E_A2_3A__3A_A3 *)ltmp_450_14);
  ltmp_451_8 = (((ltmp_450_14 == ((signed char *)0))) ? (-3) : (0));
  ltmp_452_14 = *ltmp_445_102D;
  if ((ltmp_452_14 == ((signed char *)0))) {
    ltmp_458_8_2E = ltmp_451_8;
    goto ltmp_888_D;
  } else {
    ltmp_454_8_2E = ltmp_451_8;
    ltmp_455_14_2E = ltmp_452_14;
    goto ltmp_889_D;
  }

ltmp_887_D:
  ltmp_453_14 = *ltmp_445_102D;
  if ((ltmp_453_14 == ((signed char *)0))) {
    ltmp_458_8_2E = ltmp_443_8;
    goto ltmp_888_D;
  } else {
    ltmp_454_8_2E = ltmp_443_8;
    ltmp_455_14_2E = ltmp_453_14;
    goto ltmp_889_D;
  }

ltmp_889_D:
  ltmp_454_8 = ltmp_454_8_2E;
  ltmp_455_14 = ltmp_455_14_2E;
  ltmp_456_1026 = (struct plzo_callback_t *)ltmp_455_14;
  ltmp_457_101A = *(&ltmp_456_1026->field2);
  if ((ltmp_457_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t ))0))) {
    ltmp_458_8_2E = ltmp_454_8;
    goto ltmp_888_D;
  } else {
    goto ltmp_890_D;
  }

ltmp_890_D:
  ltmp_457_101A(ltmp_456_1026, PLZO_UINT64_C(0), PLZO_UINT64_C(0), 0);
  if ((ltmp_454_8 == 0)) {
    ltmp_459_8_2E = ltmp_454_8;
    goto ltmp_891_D;
  } else {
    ltmp_839_8_2E = ltmp_454_8;
    goto ltmp_892_D;
  }

ltmp_888_D:
  ltmp_458_8 = ltmp_458_8_2E;
  if ((ltmp_458_8 == 0)) {
    ltmp_459_8_2E = ltmp_458_8;
    goto ltmp_891_D;
  } else {
    ltmp_839_8_2E = ltmp_458_8;
    goto ltmp_892_D;
  }

ltmp_891_D:
  ltmp_459_8 = ltmp_459_8_2E;
  ltmp_460_17 = &ltmp_342_1023->field19.field2;
  ltmp_461_1025 = &ltmp_342_1023->field1.field6;
  ltmp_462_1025 = &ltmp_342_1023->field9;
  ltmp_463_17 = &ltmp_342_1023->field24.field0;
  ltmp_464_17 = &ltmp_342_1023->field24.field1;
  ltmp_465_1039 = &ltmp_342_1023->field19.field7;
  ltmp_466_17 = &ltmp_342_1023->field19.field1;
  ltmp_467_1037 = &ltmp_342_1023->field19.field0;
  ltmp_468_103A = &ltmp_343_1032.field0;
  ltmp_469_103A = &ltmp_343_1032.field1;
  ltmp_470_103A = &ltmp_344_1032.field0;
  ltmp_471_103A = &ltmp_344_1032.field1;
  ltmp_472_7_2E = 0u;
  ltmp_473_8_2E = ltmp_459_8;
  goto ltmp_893_D;

  do {
ltmp_893_D:
  ltmp_472_7 = ltmp_472_7_2E;
  ltmp_473_8 = ltmp_473_8_2E;
  ltmp_474_7 = *ltmp_370_17;
  if ((ltmp_474_7 > ltmp_472_7)) {
    goto ltmp_894_D;
  } else {
    ltmp_839_8_2E = ltmp_473_8;
    goto ltmp_892_D;
  }

ltmp_895_D:
  ltmp_830_7 = *ltmp_463_17;
  ltmp_831_7 = *ltmp_464_17;
  if (((ltmp_799_7 - ltmp_830_7) < ltmp_831_7)) {
    ltmp_472_7_2E = ltmp_799_7;
    ltmp_473_8_2E = 0;
    goto ltmp_893_D;
  } else {
    goto ltmp_896_D;
  }

ltmp_897_D:
  ltmp_825_1029 = *ltmp_353_102C;
  ltmp_826_102A = *(&ltmp_825_1029[PLZO_INT64_C(16)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))ltmp_826_102A)(ltmp_342_1023, ltmp_472_7, ltmp_799_7, 0);
  goto ltmp_895_D;

ltmp_898_D:
  ltmp_824_101A(ltmp_823_1026, ltmp_800_9, ltmp_821_9, 2);
  if ((ltmp_362_8 == 0)) {
    goto ltmp_897_D;
  } else {
    goto ltmp_899_D;
  }

ltmp_900_D:
  ltmp_824_101A = *(&ltmp_823_1026->field2);
  if ((ltmp_824_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t ))0))) {
    goto ltmp_901_D;
  } else {
    goto ltmp_898_D;
  }

ltmp_902_D:
  ltmp_818_13 = *ltmp_461_1025;
  ltmp_819_13 = *ltmp_462_1025;
  ltmp_820_A = plzo_external_ptrdiff_uchar(ltmp_818_13, ltmp_819_13, PLZO_UINT64_C(1));
  ltmp_821_9 = ((plzo_uint64_t )ltmp_820_A) & PLZO_UINT64_C(4294967295);
  ltmp_822_14 = *ltmp_445_102D;
  ltmp_823_1026 = (struct plzo_callback_t *)ltmp_822_14;
  if ((ltmp_822_14 == ((signed char *)0))) {
    goto ltmp_901_D;
  } else {
    goto ltmp_900_D;
  }

ltmp_903_D:
  ltmp_807_101A(ltmp_806_1026, ltmp_800_9, ltmp_804_9, 1);
  if ((ltmp_362_8 > 1)) {
    goto ltmp_904_D;
  } else {
    goto ltmp_902_D;
  }

ltmp_905_D:
  ltmp_807_101A = *(&ltmp_806_1026->field2);
  if ((ltmp_807_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t ))0))) {
    goto ltmp_906_D;
  } else {
    goto ltmp_903_D;
  }

ltmp_907_D:
  ltmp_799_7 = ltmp_799_7_2E;
  ltmp_800_9 = (plzo_uint64_t )ltmp_799_7;
  ltmp_801_13 = *ltmp_461_1025;
  ltmp_802_13 = *ltmp_462_1025;
  ltmp_803_A = plzo_external_ptrdiff_uchar(ltmp_801_13, ltmp_802_13, PLZO_UINT64_C(1));
  ltmp_804_9 = ((plzo_uint64_t )ltmp_803_A) & PLZO_UINT64_C(4294967295);
  ltmp_805_14 = *ltmp_445_102D;
  ltmp_806_1026 = (struct plzo_callback_t *)ltmp_805_14;
  if ((ltmp_805_14 == ((signed char *)0))) {
    goto ltmp_906_D;
  } else {
    goto ltmp_905_D;
  }

ltmp_908_D:
  ltmp_792_1029 = *ltmp_353_102C;
  ltmp_793_102A = *(&ltmp_792_1029[PLZO_INT64_C(11)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_793_102A)(ltmp_342_1023, ltmp_472_7, ltmp_791_7);
  ltmp_794_7 = *ltmp_370_17;
  if (((ltmp_794_7 == ltmp_791_7) | (ltmp_362_8 < 2))) {
    ltmp_799_7_2E = ltmp_791_7;
    goto ltmp_907_D;
  } else {
    goto ltmp_909_D;
  }

ltmp_910_D:
  ltmp_789_7 = *ltmp_502_17;
  *ltmp_460_17 = ltmp_789_7;
  ltmp_790_7 = *ltmp_502_17;
  *ltmp_482_17 = ltmp_790_7;
  *(&ltmp_475_1036->field11) = (ltmp_790_7 - ltmp_472_7);
  ltmp_791_7 = *ltmp_460_17;
  if ((ltmp_362_8 > 1)) {
    goto ltmp_908_D;
  } else {
    goto ltmp_911_D;
  }

ltmp_912_D:
  ltmp_499_7 = ltmp_499_7_2E;
  ltmp_500_7 = ltmp_500_7_2E;
  ltmp_501_1038 = ltmp_501_1038_2E;
  *(&ltmp_475_1036->field8) = ltmp_500_7;
  ltmp_502_17 = &ltmp_475_1036->field9;
  *ltmp_502_17 = (ltmp_500_7 + ltmp_499_7);
  *(&ltmp_475_1036->field4) = ltmp_499_7;
  ltmp_503_1036 = *ltmp_465_1039;
  ltmp_504_7 = *(&ltmp_503_1036->field8);
  ltmp_505_17 = &ltmp_503_1036->field9;
  ltmp_506_7 = *ltmp_505_17;
  if ((ltmp_506_7 > ltmp_504_7)) {
    goto ltmp_913_D;
  } else {
    goto ltmp_910_D;
  }

ltmp_894_D:
  ltmp_475_1036 = *ltmp_465_1039;
  *(&ltmp_475_1036->field5) = ltmp_472_7;
  *ltmp_466_17 = ltmp_472_7;
  ltmp_476_7 = *(&ltmp_475_1036->field2);
  ltmp_477_7 = ltmp_476_7 - ltmp_472_7;
  ltmp_478_7 = *(&ltmp_475_1036->field6);
  ltmp_479_7 = (((ltmp_478_7 < ltmp_477_7)) ? (ltmp_478_7) : (ltmp_477_7));
  ltmp_480_17 = &ltmp_475_1036->field7;
  *ltmp_480_17 = ltmp_478_7;
  ltmp_481_1038 = *ltmp_467_1037;
  ltmp_482_17 = &ltmp_475_1036->field10;
  ltmp_483_7 = *ltmp_482_17;
  if ((ltmp_483_7 > ltmp_472_7)) {
    goto ltmp_914_D;
  } else {
    ltmp_499_7_2E = ltmp_479_7;
    ltmp_500_7_2E = ltmp_472_7;
    ltmp_501_1038_2E = ltmp_481_1038;
    goto ltmp_912_D;
  }

ltmp_915_D:
  ltmp_497_1038 = &ltmp_481_1038[((plzo_uint64_t )ltmp_484_7)];
  ltmp_498_7 = ltmp_479_7 - ltmp_484_7;
  ltmp_499_7_2E = ltmp_498_7;
  ltmp_500_7_2E = ltmp_483_7;
  ltmp_501_1038_2E = ltmp_497_1038;
  goto ltmp_912_D;

ltmp_914_D:
  ltmp_484_7 = ltmp_483_7 - ltmp_472_7;
  ltmp_485_7 = *(&ltmp_475_1036->field11);
  ltmp_486_7 = ltmp_485_7 - ltmp_484_7;
  *ltmp_480_17 = ltmp_486_7;
  ltmp_487_7 = ltmp_484_7*36u;
  ltmp_488_13 = (unsigned char *)ltmp_481_1038;
  ltmp_489_13 = (unsigned char *)(&ltmp_481_1038[((plzo_int64_t )ltmp_486_7)]);
  if (((ltmp_487_7 == 0u) | (ltmp_485_7 == ltmp_484_7))) {
    goto ltmp_915_D;
  } else {
    goto ltmp_916_D;
  }

  do {
ltmp_917_D:
  ltmp_491_9 = ltmp_491_9_2E;
  ltmp_492_3 = *(&ltmp_489_13[ltmp_491_9]);
  *(&ltmp_488_13[ltmp_491_9]) = ltmp_492_3;
  ltmp_493_9 = ltmp_491_9 + PLZO_UINT64_C(1);
  if ((ltmp_493_9 == ltmp_490_9)) {
    goto ltmp_915_D;
  } else {
    ltmp_491_9_2E = ltmp_493_9;
    goto ltmp_917_D;
  }

  } while (1);
ltmp_916_D:
  ltmp_490_9 = (plzo_uint64_t )ltmp_487_7;
  if ((ltmp_488_13 < ltmp_489_13)) {
    ltmp_491_9_2E = PLZO_UINT64_C(0);
    goto ltmp_917_D;
  } else {
    ltmp_494_9_2E = PLZO_UINT64_C(0);
    goto ltmp_918_D;
  }

  do {
ltmp_918_D:
  ltmp_494_9 = ltmp_494_9_2E;
  ltmp_495_3 = *(&ltmp_489_13[(ltmp_490_9 + (((plzo_uint64_t )(-(((plzo_int64_t )ltmp_494_9)))) + PLZO_UINT64_C(18446744073709551615)))]);
  *(&ltmp_488_13[(ltmp_490_9 + (ltmp_494_9 ^ PLZO_UINT64_C(18446744073709551615)))]) = ltmp_495_3;
  ltmp_496_9 = ltmp_494_9 + PLZO_UINT64_C(1);
  if ((ltmp_496_9 == ltmp_490_9)) {
    goto ltmp_915_D;
  } else {
    ltmp_494_9_2E = ltmp_496_9;
    goto ltmp_918_D;
  }

  } while (1);
  do {
ltmp_919_D:
  ltmp_523_7 = ltmp_523_7_2E;
  ltmp_524_1038 = ltmp_524_1038_2E;
  ltmp_525_7 = *ltmp_507_17;
  if ((ltmp_525_7 < ltmp_523_7)) {
    goto ltmp_920_D;
  } else {
    goto ltmp_921_D;
  }

ltmp_922_D:
  ltmp_783_7 = ltmp_783_7_2E;
  ltmp_784_1038 = ltmp_784_1038_2E;
  ltmp_785_7 = ltmp_783_7 + 1u;
  ltmp_786_1038 = &ltmp_784_1038[PLZO_INT64_C(1)];
  ltmp_787_7 = *ltmp_520_17;
  *ltmp_520_17 = (ltmp_787_7 + 4294967295u);
  ltmp_788_7 = *ltmp_505_17;
  if ((ltmp_788_7 > ltmp_785_7)) {
    ltmp_523_7_2E = ltmp_785_7;
    ltmp_524_1038_2E = ltmp_786_1038;
    goto ltmp_919_D;
  } else {
    goto ltmp_910_D;
  }

ltmp_923_D:
  ltmp_772_7 = *(&ltmp_524_1038->field0);
  ltmp_773_7 = *ltmp_522_17;
  if ((ltmp_772_7 < ltmp_773_7)) {
    ltmp_783_7_2E = ltmp_523_7;
    ltmp_784_1038_2E = ltmp_524_1038;
    goto ltmp_922_D;
  } else {
    goto ltmp_924_D;
  }

ltmp_925_D:
  *ltmp_529_17 = 0u;
  goto ltmp_923_D;

ltmp_926_D:
  ltmp_736_7 = *ltmp_529_17;
  ltmp_737_7 = *ltmp_510_17;
  if ((ltmp_736_7 < ltmp_737_7)) {
    goto ltmp_925_D;
  } else {
    goto ltmp_927_D;
  }

ltmp_928_D:
  ltmp_734_7 = ltmp_734_7_2E;
  *ltmp_546_17 = ltmp_523_7;
  *ltmp_529_17 = ltmp_734_7;
  ltmp_735_7 = *ltmp_520_17;
  if ((ltmp_734_7 > ltmp_735_7)) {
    goto ltmp_929_D;
  } else {
    goto ltmp_926_D;
  }

ltmp_930_D:
  ltmp_543_7 = ltmp_543_7_2E;
  ltmp_544_3 = *ltmp_536_13;
  ltmp_545_3 = *(&ltmp_533_13[(ltmp_535_A + PLZO_INT64_C(1))]);
  ltmp_546_17 = &ltmp_503_1036->field26[((plzo_int64_t )((((plzo_uint32_t )ltmp_545_3) << 8) | ((plzo_uint32_t )ltmp_544_3)))];
  ltmp_547_7 = *ltmp_546_17;
  if ((ltmp_547_7 < ltmp_523_7)) {
    goto ltmp_931_D;
  } else {
    ltmp_734_7_2E = ltmp_543_7;
    goto ltmp_928_D;
  }

ltmp_921_D:
  ltmp_526_7 = *ltmp_508_17;
  ltmp_527_7 = ltmp_526_7 - ltmp_523_7;
  ltmp_528_7 = (((ltmp_527_7 < 1025u)) ? (ltmp_527_7) : (1024u));
  ltmp_529_17 = &ltmp_524_1038->field0;
  ltmp_530_1032 = ltmp_18_100A(ltmp_503_1036);
  ltmp_531_103A = &ltmp_530_1032->field0;
  ltmp_532_103A = &ltmp_530_1032->field1;
  *ltmp_532_103A = ((struct l_struct_2E_M *)0);
  *ltmp_531_103A = ((struct l_struct_2E_M *)0);
  ltmp_533_13 = *ltmp_509_1025;
  ltmp_534_9 = (plzo_uint64_t )ltmp_523_7;
  ltmp_535_A = (plzo_int64_t )ltmp_523_7;
  ltmp_536_13 = &ltmp_533_13[ltmp_534_9];
  ltmp_537_7 = *ltmp_510_17;
  if ((ltmp_537_7 == 1u)) {
    goto ltmp_932_D;
  } else {
    ltmp_543_7_2E = 0u;
    goto ltmp_930_D;
  }

ltmp_933_D:
  *ltmp_512_1025 = (&ltmp_533_13[(ltmp_534_9 - ((plzo_uint64_t )ltmp_541_7))]);
  *ltmp_539_17 = ltmp_523_7;
  ltmp_543_7_2E = 1u;
  goto ltmp_930_D;

ltmp_934_D:
  ltmp_541_7 = ltmp_523_7 - ltmp_540_7;
  ltmp_542_7 = *ltmp_511_17;
  if ((ltmp_542_7 < ltmp_541_7)) {
    goto ltmp_935_D;
  } else {
    goto ltmp_933_D;
  }

ltmp_932_D:
  ltmp_538_3 = *ltmp_536_13;
  ltmp_539_17 = &ltmp_503_1036->field27[((plzo_int64_t )ltmp_538_3)];
  ltmp_540_7 = *ltmp_539_17;
  if ((ltmp_540_7 < ltmp_523_7)) {
    goto ltmp_934_D;
  } else {
    goto ltmp_935_D;
  }

ltmp_935_D:
  *ltmp_539_17 = ltmp_523_7;
  ltmp_543_7_2E = 0u;
  goto ltmp_930_D;

ltmp_931_D:
  ltmp_548_7 = ltmp_523_7 - ltmp_547_7;
  ltmp_549_7 = *ltmp_511_17;
  if ((ltmp_549_7 < ltmp_548_7)) {
    ltmp_734_7_2E = ltmp_543_7;
    goto ltmp_928_D;
  } else {
    goto ltmp_936_D;
  }

ltmp_937_D:
  ltmp_669_7 = ltmp_669_7_2E;
  ltmp_670_1032 = *ltmp_469_103A;
  *ltmp_531_103A = ltmp_670_1032;
  ltmp_671_1032 = *ltmp_468_103A;
  *ltmp_532_103A = ltmp_671_1032;
  ltmp_734_7_2E = ltmp_669_7;
  goto ltmp_928_D;

ltmp_938_D:
  *(&ltmp_596_1032->field0) = ltmp_598_1032;
  *(&ltmp_597_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_669_7_2E = ltmp_607_7;
  goto ltmp_937_D;

  do {
ltmp_939_D:
  ltmp_592_13 = ltmp_592_13_2E;
  ltmp_593_7 = ltmp_593_7_2E;
  ltmp_594_7 = ltmp_594_7_2E;
  ltmp_595_7 = ltmp_595_7_2E;
  ltmp_596_1032 = ltmp_596_1032_2E;
  ltmp_597_1032 = ltmp_597_1032_2E;
  ltmp_598_1032 = ltmp_598_1032_2E;
  ltmp_599_7 = ltmp_599_7_2E;
  ltmp_600_7 = ltmp_600_7_2E;
  ltmp_601_7 = ltmp_600_7 + ltmp_599_7;
  ltmp_602_A = (plzo_int64_t )ltmp_601_7;
  ltmp_603_3 = *(&ltmp_572_13[(ltmp_602_A + ltmp_583_A)]);
  ltmp_604_3 = *(&ltmp_592_13[ltmp_602_A]);
  if ((ltmp_603_3 == ltmp_604_3)) {
    goto ltmp_940_D;
  } else {
    ltmp_607_7_2E = ltmp_601_7;
    goto ltmp_941_D;
  }

ltmp_942_D:
  ltmp_584_13 = ltmp_584_13_2E;
  ltmp_585_7 = ltmp_585_7_2E;
  ltmp_586_7 = ltmp_586_7_2E;
  ltmp_587_7 = ltmp_587_7_2E;
  ltmp_588_1032 = ltmp_588_1032_2E;
  ltmp_589_1032 = ltmp_589_1032_2E;
  ltmp_590_1032 = ltmp_590_1032_2E;
  ltmp_591_7 = (((ltmp_586_7 <= ltmp_585_7)) ? (ltmp_586_7) : (ltmp_585_7));
  ltmp_592_13_2E = ltmp_584_13;
  ltmp_593_7_2E = ltmp_585_7;
  ltmp_594_7_2E = ltmp_586_7;
  ltmp_595_7_2E = ltmp_587_7;
  ltmp_596_1032_2E = ltmp_588_1032;
  ltmp_597_1032_2E = ltmp_589_1032;
  ltmp_598_1032_2E = ltmp_590_1032;
  ltmp_599_7_2E = ltmp_591_7;
  ltmp_600_7_2E = 0u;
  goto ltmp_939_D;

ltmp_943_D:
  ltmp_621_7 = ltmp_621_7_2E;
  ltmp_622_7 = *ltmp_515_17;
  ltmp_623_13 = *ltmp_509_1025;
  ltmp_624_13 = &ltmp_623_13[(((plzo_int32_t )ltmp_622_7) + ((plzo_int32_t )ltmp_621_7))];
  if ((ltmp_624_13 < ltmp_592_13)) {
    ltmp_584_13_2E = ltmp_624_13;
    ltmp_585_7_2E = ltmp_593_7;
    ltmp_586_7_2E = ltmp_612_7;
    ltmp_587_7_2E = ltmp_613_7;
    ltmp_588_1032_2E = ltmp_598_1032;
    ltmp_589_1032_2E = ltmp_597_1032;
    ltmp_590_1032_2E = ltmp_614_1032;
    goto ltmp_942_D;
  } else {
    goto ltmp_944_D;
  }

ltmp_945_D:
  ltmp_615_1032 = *ltmp_518_103A;
  ltmp_616_A = plzo_external_ptrdiff_M(ltmp_614_1032, ltmp_615_1032, PLZO_UINT64_C(16));
  ltmp_617_7 = *ltmp_516_17;
  ltmp_618_7 = ((plzo_uint32_t )ltmp_616_A) - ltmp_617_7;
  if ((ltmp_618_7 > 2147483647u)) {
    ltmp_621_7_2E = ltmp_618_7;
    goto ltmp_943_D;
  } else {
    goto ltmp_946_D;
  }

ltmp_947_D:
  ltmp_612_7 = ltmp_612_7_2E;
  ltmp_613_7 = ltmp_613_7_2E;
  *(&ltmp_596_1032->field0) = ltmp_598_1032;
  ltmp_614_1032 = *(&ltmp_598_1032->field0);
  if ((ltmp_614_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_948_D;
  } else {
    goto ltmp_945_D;
  }

ltmp_949_D:
  if ((ltmp_607_7 > ltmp_594_7)) {
    goto ltmp_950_D;
  } else {
    ltmp_612_7_2E = ltmp_594_7;
    ltmp_613_7_2E = ltmp_595_7;
    goto ltmp_947_D;
  }

ltmp_941_D:
  ltmp_607_7 = ltmp_607_7_2E;
  ltmp_608_8 = ((plzo_int32_t )ltmp_603_3) - ((plzo_int32_t )ltmp_604_3);
  if ((ltmp_608_8 < 0)) {
    goto ltmp_949_D;
  } else {
    goto ltmp_951_D;
  }

ltmp_940_D:
  ltmp_605_7 = ltmp_601_7 + 1u;
  ltmp_606_7 = ltmp_600_7 + 1u;
  if ((ltmp_605_7 < ltmp_528_7)) {
    ltmp_592_13_2E = ltmp_592_13;
    ltmp_593_7_2E = ltmp_593_7;
    ltmp_594_7_2E = ltmp_594_7;
    ltmp_595_7_2E = ltmp_595_7;
    ltmp_596_1032_2E = ltmp_596_1032;
    ltmp_597_1032_2E = ltmp_597_1032;
    ltmp_598_1032_2E = ltmp_598_1032;
    ltmp_599_7_2E = ltmp_599_7;
    ltmp_600_7_2E = ltmp_606_7;
    goto ltmp_939_D;
  } else {
    ltmp_607_7_2E = ltmp_605_7;
    goto ltmp_941_D;
  }

ltmp_950_D:
  if ((ltmp_607_7 > ltmp_595_7)) {
    ltmp_609_7_2E = 0u;
    goto ltmp_952_D;
  } else {
    ltmp_612_7_2E = ltmp_607_7;
    ltmp_613_7_2E = ltmp_595_7;
    goto ltmp_947_D;
  }

ltmp_953_D:
  ltmp_611_7 = *ltmp_519_17;
  if ((ltmp_611_7 > ltmp_607_7)) {
    ltmp_612_7_2E = ltmp_607_7;
    ltmp_613_7_2E = ltmp_607_7;
    goto ltmp_947_D;
  } else {
    goto ltmp_938_D;
  }

  do {
ltmp_952_D:
  ltmp_609_7 = ltmp_609_7_2E;
  ltmp_610_7 = (ltmp_609_7 + ltmp_595_7) + 1u;
  *(&ltmp_503_1036->field25[((plzo_int64_t )ltmp_610_7)]) = ltmp_592_13;
  if ((ltmp_610_7 < ltmp_607_7)) {
    ltmp_609_7_2E = (ltmp_609_7 + 1u);
    goto ltmp_952_D;
  } else {
    goto ltmp_953_D;
  }

  } while (1);
ltmp_946_D:
  ltmp_619_7 = *ltmp_517_17;
  ltmp_620_7 = ltmp_618_7 - ltmp_619_7;
  ltmp_621_7_2E = ltmp_620_7;
  goto ltmp_943_D;

ltmp_954_D:
  ltmp_637_7 = ltmp_637_7_2E;
  ltmp_638_7 = *ltmp_515_17;
  ltmp_639_13 = *ltmp_509_1025;
  ltmp_640_13 = &ltmp_639_13[(((plzo_int32_t )ltmp_638_7) + ((plzo_int32_t )ltmp_637_7))];
  if ((ltmp_640_13 < ltmp_592_13)) {
    ltmp_584_13_2E = ltmp_640_13;
    ltmp_585_7_2E = ltmp_628_7;
    ltmp_586_7_2E = ltmp_594_7;
    ltmp_587_7_2E = ltmp_629_7;
    ltmp_588_1032_2E = ltmp_596_1032;
    ltmp_589_1032_2E = ltmp_598_1032;
    ltmp_590_1032_2E = ltmp_630_1032;
    goto ltmp_942_D;
  } else {
    goto ltmp_955_D;
  }

ltmp_956_D:
  ltmp_631_1032 = *ltmp_518_103A;
  ltmp_632_A = plzo_external_ptrdiff_M(ltmp_630_1032, ltmp_631_1032, PLZO_UINT64_C(16));
  ltmp_633_7 = *ltmp_516_17;
  ltmp_634_7 = ((plzo_uint32_t )ltmp_632_A) - ltmp_633_7;
  if ((ltmp_634_7 > 2147483647u)) {
    ltmp_637_7_2E = ltmp_634_7;
    goto ltmp_954_D;
  } else {
    goto ltmp_957_D;
  }

ltmp_958_D:
  ltmp_628_7 = ltmp_628_7_2E;
  ltmp_629_7 = ltmp_629_7_2E;
  *(&ltmp_597_1032->field1) = ltmp_598_1032;
  ltmp_630_1032 = *(&ltmp_598_1032->field1);
  if ((ltmp_630_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_959_D;
  } else {
    goto ltmp_956_D;
  }

ltmp_960_D:
  if ((ltmp_607_7 > ltmp_593_7)) {
    goto ltmp_961_D;
  } else {
    ltmp_628_7_2E = ltmp_593_7;
    ltmp_629_7_2E = ltmp_595_7;
    goto ltmp_958_D;
  }

ltmp_951_D:
  if ((ltmp_608_8 > 0)) {
    goto ltmp_960_D;
  } else {
    ltmp_641_7_2E = 0u;
    goto ltmp_962_D;
  }

ltmp_961_D:
  if ((ltmp_607_7 > ltmp_595_7)) {
    ltmp_625_7_2E = 0u;
    goto ltmp_963_D;
  } else {
    ltmp_628_7_2E = ltmp_607_7;
    ltmp_629_7_2E = ltmp_595_7;
    goto ltmp_958_D;
  }

ltmp_964_D:
  ltmp_627_7 = *ltmp_519_17;
  if ((ltmp_627_7 > ltmp_607_7)) {
    ltmp_628_7_2E = ltmp_607_7;
    ltmp_629_7_2E = ltmp_607_7;
    goto ltmp_958_D;
  } else {
    goto ltmp_965_D;
  }

  do {
ltmp_963_D:
  ltmp_625_7 = ltmp_625_7_2E;
  ltmp_626_7 = (ltmp_625_7 + ltmp_595_7) + 1u;
  *(&ltmp_503_1036->field25[((plzo_int64_t )ltmp_626_7)]) = ltmp_592_13;
  if ((ltmp_626_7 < ltmp_607_7)) {
    ltmp_625_7_2E = (ltmp_625_7 + 1u);
    goto ltmp_963_D;
  } else {
    goto ltmp_964_D;
  }

  } while (1);
ltmp_957_D:
  ltmp_635_7 = *ltmp_517_17;
  ltmp_636_7 = ltmp_634_7 - ltmp_635_7;
  ltmp_637_7_2E = ltmp_636_7;
  goto ltmp_954_D;

  } while (1);
ltmp_966_D:
  ltmp_579_7 = ltmp_579_7_2E;
  ltmp_580_7 = *ltmp_515_17;
  ltmp_581_13 = *ltmp_509_1025;
  ltmp_582_13 = &ltmp_581_13[(((plzo_int32_t )ltmp_580_7) + ((plzo_int32_t )ltmp_579_7))];
  ltmp_583_A = (plzo_int64_t )ltmp_571_8;
  ltmp_592_13_2E = ltmp_582_13;
  ltmp_593_7_2E = 2u;
  ltmp_594_7_2E = 2u;
  ltmp_595_7_2E = 2u;
  ltmp_596_1032_2E = (&ltmp_343_1032);
  ltmp_597_1032_2E = (&ltmp_343_1032);
  ltmp_598_1032_2E = ltmp_562_1032;
  ltmp_599_7_2E = 2u;
  ltmp_600_7_2E = 0u;
  goto ltmp_939_D;

ltmp_967_D:
  ltmp_569_7 = ltmp_569_7_2E;
  ltmp_570_7 = *ltmp_515_17;
  ltmp_571_8 = ((plzo_int32_t )ltmp_570_7) + ((plzo_int32_t )ltmp_569_7);
  ltmp_572_13 = *ltmp_509_1025;
  ltmp_573_1032 = *ltmp_518_103A;
  ltmp_574_A = plzo_external_ptrdiff_M(ltmp_562_1032, ltmp_573_1032, PLZO_UINT64_C(16));
  ltmp_575_7 = *ltmp_516_17;
  ltmp_576_7 = ((plzo_uint32_t )ltmp_574_A) - ltmp_575_7;
  if ((ltmp_576_7 > 2147483647u)) {
    ltmp_579_7_2E = ltmp_576_7;
    goto ltmp_966_D;
  } else {
    goto ltmp_968_D;
  }

ltmp_969_D:
  ltmp_562_1032 = ltmp_562_1032_2E;
  *ltmp_468_103A = ((struct l_struct_2E_M *)0);
  *ltmp_469_103A = ((struct l_struct_2E_M *)0);
  ltmp_563_1032 = *ltmp_518_103A;
  ltmp_564_A = plzo_external_ptrdiff_M(ltmp_530_1032, ltmp_563_1032, PLZO_UINT64_C(16));
  ltmp_565_7 = *ltmp_516_17;
  ltmp_566_7 = ((plzo_uint32_t )ltmp_564_A) - ltmp_565_7;
  if ((ltmp_566_7 > 2147483647u)) {
    ltmp_569_7_2E = ltmp_566_7;
    goto ltmp_967_D;
  } else {
    goto ltmp_970_D;
  }

ltmp_971_D:
  ltmp_553_7 = *ltmp_515_17;
  ltmp_554_7 = ((plzo_uint32_t )ltmp_552_A) - ltmp_553_7;
  if ((ltmp_554_7 > 2147483647u)) {
    goto ltmp_972_D;
  } else {
    ltmp_562_1032_2E = ((struct l_struct_2E_M *)0);
    goto ltmp_969_D;
  }

ltmp_936_D:
  ltmp_550_13 = &ltmp_533_13[(ltmp_535_A - ((plzo_int64_t )ltmp_548_7))];
  *ltmp_513_1025 = ltmp_550_13;
  ltmp_551_8 = *ltmp_514_18;
  ltmp_552_A = plzo_external_ptrdiff_uchar(ltmp_550_13, ltmp_533_13, PLZO_UINT64_C(1));
  if ((ltmp_551_8 == 0)) {
    goto ltmp_973_D;
  } else {
    goto ltmp_971_D;
  }

ltmp_974_D:
  ltmp_557_7 = *ltmp_517_17;
  ltmp_558_7 = ltmp_557_7 + ltmp_556_7;
  if ((ltmp_558_7 > 2147483647u)) {
    ltmp_562_1032_2E = ((struct l_struct_2E_M *)0);
    goto ltmp_969_D;
  } else {
    ltmp_559_7_2E = ltmp_558_7;
    goto ltmp_975_D;
  }

ltmp_972_D:
  ltmp_555_7 = *ltmp_516_17;
  ltmp_556_7 = ltmp_555_7 + ltmp_554_7;
  if ((ltmp_556_7 > 2147483647u)) {
    goto ltmp_974_D;
  } else {
    ltmp_559_7_2E = ltmp_556_7;
    goto ltmp_975_D;
  }

ltmp_975_D:
  ltmp_559_7 = ltmp_559_7_2E;
  ltmp_560_1032 = *ltmp_518_103A;
  ltmp_561_1032 = &ltmp_560_1032[((plzo_int32_t )ltmp_559_7)];
  ltmp_562_1032_2E = ltmp_561_1032;
  goto ltmp_969_D;

ltmp_970_D:
  ltmp_567_7 = *ltmp_517_17;
  ltmp_568_7 = ltmp_566_7 - ltmp_567_7;
  ltmp_569_7_2E = ltmp_568_7;
  goto ltmp_967_D;

ltmp_968_D:
  ltmp_577_7 = *ltmp_517_17;
  ltmp_578_7 = ltmp_576_7 - ltmp_577_7;
  ltmp_579_7_2E = ltmp_578_7;
  goto ltmp_966_D;

ltmp_948_D:
  *(&ltmp_597_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_669_7_2E = ltmp_613_7;
  goto ltmp_937_D;

ltmp_944_D:
  *(&ltmp_597_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_669_7_2E = ltmp_613_7;
  goto ltmp_937_D;

ltmp_965_D:
  *(&ltmp_597_1032->field1) = ltmp_598_1032;
  *(&ltmp_596_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_669_7_2E = ltmp_607_7;
  goto ltmp_937_D;

ltmp_959_D:
  *(&ltmp_596_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_669_7_2E = ltmp_629_7;
  goto ltmp_937_D;

ltmp_955_D:
  *(&ltmp_596_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_669_7_2E = ltmp_629_7;
  goto ltmp_937_D;

ltmp_976_D:
  ltmp_667_1032 = *ltmp_643_103A;
  *(&ltmp_597_1032->field1) = ltmp_667_1032;
  ltmp_668_1032 = *(&ltmp_598_1032->field1);
  *(&ltmp_596_1032->field0) = ltmp_668_1032;
  ltmp_669_7_2E = ltmp_607_7;
  goto ltmp_937_D;

ltmp_977_D:
  *ltmp_643_103A = ((struct l_struct_2E_M *)0);
  ltmp_654_1032 = *(&ltmp_598_1032->field1);
  if ((ltmp_654_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_976_D;
  } else {
    goto ltmp_978_D;
  }

ltmp_979_D:
  ltmp_651_7 = ltmp_651_7_2E;
  ltmp_652_7 = *ltmp_515_17;
  ltmp_653_13 = *ltmp_509_1025;
  if (((&ltmp_653_13[(((plzo_int32_t )ltmp_652_7) + ((plzo_int32_t )ltmp_651_7))]) < ltmp_592_13)) {
    goto ltmp_980_D;
  } else {
    goto ltmp_977_D;
  }

ltmp_981_D:
  ltmp_645_1032 = *ltmp_518_103A;
  ltmp_646_A = plzo_external_ptrdiff_M(ltmp_644_1032, ltmp_645_1032, PLZO_UINT64_C(16));
  ltmp_647_7 = *ltmp_516_17;
  ltmp_648_7 = ((plzo_uint32_t )ltmp_646_A) - ltmp_647_7;
  if ((ltmp_648_7 > 2147483647u)) {
    ltmp_651_7_2E = ltmp_648_7;
    goto ltmp_979_D;
  } else {
    goto ltmp_982_D;
  }

ltmp_983_D:
  ltmp_643_103A = &ltmp_598_1032->field0;
  ltmp_644_1032 = *ltmp_643_103A;
  if ((ltmp_644_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_980_D;
  } else {
    goto ltmp_981_D;
  }

  do {
ltmp_962_D:
  ltmp_641_7 = ltmp_641_7_2E;
  ltmp_642_7 = (ltmp_641_7 + ltmp_595_7) + 1u;
  *(&ltmp_503_1036->field25[((plzo_int64_t )ltmp_642_7)]) = ltmp_592_13;
  if ((ltmp_642_7 < ltmp_607_7)) {
    ltmp_641_7_2E = (ltmp_641_7 + 1u);
    goto ltmp_962_D;
  } else {
    goto ltmp_983_D;
  }

  } while (1);
ltmp_982_D:
  ltmp_649_7 = *ltmp_517_17;
  ltmp_650_7 = ltmp_648_7 - ltmp_649_7;
  ltmp_651_7_2E = ltmp_650_7;
  goto ltmp_979_D;

ltmp_980_D:
  ltmp_655_1032 = *(&ltmp_598_1032->field1);
  if ((ltmp_655_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_976_D;
  } else {
    goto ltmp_978_D;
  }

ltmp_984_D:
  ltmp_664_7 = ltmp_664_7_2E;
  ltmp_665_7 = *ltmp_515_17;
  ltmp_666_13 = *ltmp_509_1025;
  if (((&ltmp_666_13[(((plzo_int32_t )ltmp_665_7) + ((plzo_int32_t )ltmp_664_7))]) < ltmp_592_13)) {
    goto ltmp_976_D;
  } else {
    goto ltmp_985_D;
  }

ltmp_978_D:
  ltmp_656_103A = &ltmp_598_1032->field1;
  ltmp_657_1032 = *ltmp_656_103A;
  ltmp_658_1032 = *ltmp_518_103A;
  ltmp_659_A = plzo_external_ptrdiff_M(ltmp_657_1032, ltmp_658_1032, PLZO_UINT64_C(16));
  ltmp_660_7 = *ltmp_516_17;
  ltmp_661_7 = ((plzo_uint32_t )ltmp_659_A) - ltmp_660_7;
  if ((ltmp_661_7 > 2147483647u)) {
    ltmp_664_7_2E = ltmp_661_7;
    goto ltmp_984_D;
  } else {
    goto ltmp_986_D;
  }

ltmp_986_D:
  ltmp_662_7 = *ltmp_517_17;
  ltmp_663_7 = ltmp_661_7 - ltmp_662_7;
  ltmp_664_7_2E = ltmp_663_7;
  goto ltmp_984_D;

ltmp_985_D:
  *ltmp_656_103A = ((struct l_struct_2E_M *)0);
  goto ltmp_976_D;

ltmp_987_D:
  ltmp_731_7 = ltmp_731_7_2E;
  ltmp_732_1032 = *ltmp_471_103A;
  *ltmp_531_103A = ltmp_732_1032;
  ltmp_733_1032 = *ltmp_470_103A;
  *ltmp_532_103A = ltmp_733_1032;
  ltmp_734_7_2E = ltmp_731_7;
  goto ltmp_928_D;

ltmp_988_D:
  *(&ltmp_694_1032->field0) = ltmp_696_1032;
  *(&ltmp_695_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_731_7_2E = ltmp_705_7;
  goto ltmp_987_D;

  do {
ltmp_989_D:
  ltmp_690_13 = ltmp_690_13_2E;
  ltmp_691_7 = ltmp_691_7_2E;
  ltmp_692_7 = ltmp_692_7_2E;
  ltmp_693_7 = ltmp_693_7_2E;
  ltmp_694_1032 = ltmp_694_1032_2E;
  ltmp_695_1032 = ltmp_695_1032_2E;
  ltmp_696_1032 = ltmp_696_1032_2E;
  ltmp_697_7 = ltmp_697_7_2E;
  ltmp_698_7 = ltmp_698_7_2E;
  ltmp_699_7 = ltmp_698_7 + ltmp_697_7;
  ltmp_700_A = (plzo_int64_t )ltmp_699_7;
  ltmp_701_3 = *(&ltmp_676_13[(ltmp_700_A + ltmp_677_A)]);
  ltmp_702_3 = *(&ltmp_690_13[ltmp_700_A]);
  if ((ltmp_701_3 == ltmp_702_3)) {
    goto ltmp_990_D;
  } else {
    ltmp_705_7_2E = ltmp_699_7;
    goto ltmp_991_D;
  }

ltmp_992_D:
  ltmp_682_13 = ltmp_682_13_2E;
  ltmp_683_7 = ltmp_683_7_2E;
  ltmp_684_7 = ltmp_684_7_2E;
  ltmp_685_7 = ltmp_685_7_2E;
  ltmp_686_1032 = ltmp_686_1032_2E;
  ltmp_687_1032 = ltmp_687_1032_2E;
  ltmp_688_1032 = ltmp_688_1032_2E;
  ltmp_689_7 = (((ltmp_684_7 <= ltmp_683_7)) ? (ltmp_684_7) : (ltmp_683_7));
  ltmp_690_13_2E = ltmp_682_13;
  ltmp_691_7_2E = ltmp_683_7;
  ltmp_692_7_2E = ltmp_684_7;
  ltmp_693_7_2E = ltmp_685_7;
  ltmp_694_1032_2E = ltmp_686_1032;
  ltmp_695_1032_2E = ltmp_687_1032;
  ltmp_696_1032_2E = ltmp_688_1032;
  ltmp_697_7_2E = ltmp_689_7;
  ltmp_698_7_2E = 0u;
  goto ltmp_989_D;

ltmp_993_D:
  ltmp_713_1032 = *ltmp_518_103A;
  ltmp_714_A = plzo_external_ptrdiff_M(ltmp_712_1032, ltmp_713_1032, PLZO_UINT64_C(16));
  ltmp_715_13 = *ltmp_509_1025;
  ltmp_716_13 = &ltmp_715_13[(ltmp_714_A & PLZO_INT64_C(4294967295))];
  ltmp_682_13_2E = ltmp_716_13;
  ltmp_683_7_2E = ltmp_691_7;
  ltmp_684_7_2E = ltmp_710_7;
  ltmp_685_7_2E = ltmp_711_7;
  ltmp_686_1032_2E = ltmp_696_1032;
  ltmp_687_1032_2E = ltmp_695_1032;
  ltmp_688_1032_2E = ltmp_712_1032;
  goto ltmp_992_D;

ltmp_994_D:
  ltmp_710_7 = ltmp_710_7_2E;
  ltmp_711_7 = ltmp_711_7_2E;
  *(&ltmp_694_1032->field0) = ltmp_696_1032;
  ltmp_712_1032 = *(&ltmp_696_1032->field0);
  if ((ltmp_712_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_995_D;
  } else {
    goto ltmp_993_D;
  }

ltmp_996_D:
  if ((ltmp_705_7 > ltmp_692_7)) {
    goto ltmp_997_D;
  } else {
    ltmp_710_7_2E = ltmp_692_7;
    ltmp_711_7_2E = ltmp_693_7;
    goto ltmp_994_D;
  }

ltmp_991_D:
  ltmp_705_7 = ltmp_705_7_2E;
  ltmp_706_8 = ((plzo_int32_t )ltmp_701_3) - ((plzo_int32_t )ltmp_702_3);
  if ((ltmp_706_8 < 0)) {
    goto ltmp_996_D;
  } else {
    goto ltmp_998_D;
  }

ltmp_990_D:
  ltmp_703_7 = ltmp_699_7 + 1u;
  ltmp_704_7 = ltmp_698_7 + 1u;
  if ((ltmp_703_7 < ltmp_528_7)) {
    ltmp_690_13_2E = ltmp_690_13;
    ltmp_691_7_2E = ltmp_691_7;
    ltmp_692_7_2E = ltmp_692_7;
    ltmp_693_7_2E = ltmp_693_7;
    ltmp_694_1032_2E = ltmp_694_1032;
    ltmp_695_1032_2E = ltmp_695_1032;
    ltmp_696_1032_2E = ltmp_696_1032;
    ltmp_697_7_2E = ltmp_697_7;
    ltmp_698_7_2E = ltmp_704_7;
    goto ltmp_989_D;
  } else {
    ltmp_705_7_2E = ltmp_703_7;
    goto ltmp_991_D;
  }

ltmp_997_D:
  if ((ltmp_705_7 > ltmp_693_7)) {
    ltmp_707_7_2E = 0u;
    goto ltmp_999_D;
  } else {
    ltmp_710_7_2E = ltmp_705_7;
    ltmp_711_7_2E = ltmp_693_7;
    goto ltmp_994_D;
  }

ltmp_1000_D:
  ltmp_709_7 = *ltmp_519_17;
  if ((ltmp_709_7 > ltmp_705_7)) {
    ltmp_710_7_2E = ltmp_705_7;
    ltmp_711_7_2E = ltmp_705_7;
    goto ltmp_994_D;
  } else {
    goto ltmp_988_D;
  }

  do {
ltmp_999_D:
  ltmp_707_7 = ltmp_707_7_2E;
  ltmp_708_7 = (ltmp_707_7 + ltmp_693_7) + 1u;
  *(&ltmp_503_1036->field25[((plzo_int64_t )ltmp_708_7)]) = ltmp_690_13;
  if ((ltmp_708_7 < ltmp_705_7)) {
    ltmp_707_7_2E = (ltmp_707_7 + 1u);
    goto ltmp_999_D;
  } else {
    goto ltmp_1000_D;
  }

  } while (1);
ltmp_1001_D:
  ltmp_723_1032 = *ltmp_518_103A;
  ltmp_724_A = plzo_external_ptrdiff_M(ltmp_722_1032, ltmp_723_1032, PLZO_UINT64_C(16));
  ltmp_725_13 = *ltmp_509_1025;
  ltmp_726_13 = &ltmp_725_13[(ltmp_724_A & PLZO_INT64_C(4294967295))];
  ltmp_682_13_2E = ltmp_726_13;
  ltmp_683_7_2E = ltmp_720_7;
  ltmp_684_7_2E = ltmp_692_7;
  ltmp_685_7_2E = ltmp_721_7;
  ltmp_686_1032_2E = ltmp_694_1032;
  ltmp_687_1032_2E = ltmp_696_1032;
  ltmp_688_1032_2E = ltmp_722_1032;
  goto ltmp_992_D;

ltmp_1002_D:
  ltmp_720_7 = ltmp_720_7_2E;
  ltmp_721_7 = ltmp_721_7_2E;
  *(&ltmp_695_1032->field1) = ltmp_696_1032;
  ltmp_722_1032 = *(&ltmp_696_1032->field1);
  if ((ltmp_722_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_1003_D;
  } else {
    goto ltmp_1001_D;
  }

ltmp_1004_D:
  if ((ltmp_705_7 > ltmp_691_7)) {
    goto ltmp_1005_D;
  } else {
    ltmp_720_7_2E = ltmp_691_7;
    ltmp_721_7_2E = ltmp_693_7;
    goto ltmp_1002_D;
  }

ltmp_998_D:
  if ((ltmp_706_8 > 0)) {
    goto ltmp_1004_D;
  } else {
    ltmp_727_7_2E = 0u;
    goto ltmp_1006_D;
  }

ltmp_1005_D:
  if ((ltmp_705_7 > ltmp_693_7)) {
    ltmp_717_7_2E = 0u;
    goto ltmp_1007_D;
  } else {
    ltmp_720_7_2E = ltmp_705_7;
    ltmp_721_7_2E = ltmp_693_7;
    goto ltmp_1002_D;
  }

ltmp_1008_D:
  ltmp_719_7 = *ltmp_519_17;
  if ((ltmp_719_7 > ltmp_705_7)) {
    ltmp_720_7_2E = ltmp_705_7;
    ltmp_721_7_2E = ltmp_705_7;
    goto ltmp_1002_D;
  } else {
    goto ltmp_1009_D;
  }

  do {
ltmp_1007_D:
  ltmp_717_7 = ltmp_717_7_2E;
  ltmp_718_7 = (ltmp_717_7 + ltmp_693_7) + 1u;
  *(&ltmp_503_1036->field25[((plzo_int64_t )ltmp_718_7)]) = ltmp_690_13;
  if ((ltmp_718_7 < ltmp_705_7)) {
    ltmp_717_7_2E = (ltmp_717_7 + 1u);
    goto ltmp_1007_D;
  } else {
    goto ltmp_1008_D;
  }

  } while (1);
  } while (1);
ltmp_973_D:
  ltmp_672_1032 = *ltmp_518_103A;
  ltmp_673_1032 = &ltmp_672_1032[(ltmp_552_A & PLZO_INT64_C(4294967295))];
  *ltmp_470_103A = ((struct l_struct_2E_M *)0);
  *ltmp_471_103A = ((struct l_struct_2E_M *)0);
  ltmp_674_1032 = *ltmp_518_103A;
  ltmp_675_A = plzo_external_ptrdiff_M(ltmp_530_1032, ltmp_674_1032, PLZO_UINT64_C(16));
  ltmp_676_13 = *ltmp_509_1025;
  ltmp_677_A = ltmp_675_A & PLZO_INT64_C(4294967295);
  ltmp_678_1032 = *ltmp_518_103A;
  ltmp_679_A = plzo_external_ptrdiff_M(ltmp_673_1032, ltmp_678_1032, PLZO_UINT64_C(16));
  ltmp_680_13 = *ltmp_509_1025;
  ltmp_681_13 = &ltmp_680_13[(ltmp_679_A & PLZO_INT64_C(4294967295))];
  ltmp_690_13_2E = ltmp_681_13;
  ltmp_691_7_2E = 2u;
  ltmp_692_7_2E = 2u;
  ltmp_693_7_2E = 2u;
  ltmp_694_1032_2E = (&ltmp_344_1032);
  ltmp_695_1032_2E = (&ltmp_344_1032);
  ltmp_696_1032_2E = ltmp_673_1032;
  ltmp_697_7_2E = 2u;
  ltmp_698_7_2E = 0u;
  goto ltmp_989_D;

ltmp_995_D:
  *(&ltmp_695_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_731_7_2E = ltmp_711_7;
  goto ltmp_987_D;

ltmp_1009_D:
  *(&ltmp_695_1032->field1) = ltmp_696_1032;
  *(&ltmp_694_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_731_7_2E = ltmp_705_7;
  goto ltmp_987_D;

ltmp_1003_D:
  *(&ltmp_694_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_731_7_2E = ltmp_721_7;
  goto ltmp_987_D;

ltmp_1010_D:
  ltmp_729_1032 = *(&ltmp_696_1032->field0);
  *(&ltmp_695_1032->field1) = ltmp_729_1032;
  ltmp_730_1032 = *(&ltmp_696_1032->field1);
  *(&ltmp_694_1032->field0) = ltmp_730_1032;
  ltmp_731_7_2E = ltmp_705_7;
  goto ltmp_987_D;

  do {
ltmp_1006_D:
  ltmp_727_7 = ltmp_727_7_2E;
  ltmp_728_7 = (ltmp_727_7 + ltmp_693_7) + 1u;
  *(&ltmp_503_1036->field25[((plzo_int64_t )ltmp_728_7)]) = ltmp_690_13;
  if ((ltmp_728_7 < ltmp_705_7)) {
    ltmp_727_7_2E = (ltmp_727_7 + 1u);
    goto ltmp_1006_D;
  } else {
    goto ltmp_1010_D;
  }

  } while (1);
ltmp_929_D:
  *ltmp_529_17 = ltmp_735_7;
  goto ltmp_926_D;

ltmp_1011_D:
  ltmp_749_7 = *ltmp_529_17;
  if ((ltmp_749_7 == 1024u)) {
    goto ltmp_1012_D;
  } else {
    goto ltmp_923_D;
  }

  do {
ltmp_1013_D:
  ltmp_742_7 = ltmp_742_7_2E;
  ltmp_743_7 = ltmp_743_7_2E;
  ltmp_744_7 = ltmp_744_7_2E;
  ltmp_745_7 = ltmp_743_7 - ltmp_744_7;
  ltmp_746_13 = *(&ltmp_503_1036->field25[((plzo_int64_t )ltmp_745_7)]);
  ltmp_747_A = plzo_external_ptrdiff_uchar(ltmp_739_13, ltmp_746_13, PLZO_UINT64_C(1));
  *(&ltmp_524_1038->field1[((plzo_int64_t )ltmp_744_7)]) = ((plzo_uint32_t )ltmp_747_A);
  ltmp_748_7 = ltmp_744_7 + 1u;
  if ((ltmp_748_7 > 7u)) {
    goto ltmp_1011_D;
  } else {
    goto ltmp_1014_D;
  }

ltmp_1014_D:
  if (((ltmp_745_7 + 4294967295u) < ltmp_742_7)) {
    goto ltmp_1011_D;
  } else {
    ltmp_742_7_2E = ltmp_742_7;
    ltmp_743_7_2E = ltmp_743_7;
    ltmp_744_7_2E = ltmp_748_7;
    goto ltmp_1013_D;
  }

  } while (1);
ltmp_927_D:
  ltmp_738_13 = *ltmp_509_1025;
  ltmp_739_13 = &ltmp_738_13[ltmp_534_9];
  ltmp_740_7 = *ltmp_521_17;
  if ((ltmp_736_7 > ltmp_740_7)) {
    goto ltmp_1015_D;
  } else {
    ltmp_742_7_2E = ltmp_737_7;
    ltmp_743_7_2E = ltmp_736_7;
    ltmp_744_7_2E = 0u;
    goto ltmp_1013_D;
  }

ltmp_1015_D:
  *ltmp_529_17 = ltmp_740_7;
  ltmp_741_7 = *ltmp_510_17;
  ltmp_742_7_2E = ltmp_741_7;
  ltmp_743_7_2E = ltmp_740_7;
  ltmp_744_7_2E = 0u;
  goto ltmp_1013_D;

ltmp_1016_D:
  ltmp_762_7 = ltmp_762_7_2E;
  if ((ltmp_749_7 < ltmp_762_7)) {
    goto ltmp_1017_D;
  } else {
    goto ltmp_923_D;
  }

ltmp_1012_D:
  ltmp_750_7 = *(&ltmp_524_1038->field1[PLZO_INT64_C(0)]);
  ltmp_751_A = ltmp_535_A - ((plzo_int64_t )ltmp_750_7);
  ltmp_752_7 = *ltmp_520_17;
  ltmp_753_7 = *ltmp_521_17;
  ltmp_754_7 = (((ltmp_752_7 <= ltmp_753_7)) ? (ltmp_752_7) : (ltmp_753_7));
  if ((ltmp_749_7 < ltmp_754_7)) {
    ltmp_755_7_2E = 0u;
    goto ltmp_1018_D;
  } else {
    ltmp_762_7_2E = ltmp_749_7;
    goto ltmp_1016_D;
  }

  do {
ltmp_1018_D:
  ltmp_755_7 = ltmp_755_7_2E;
  ltmp_756_7 = ltmp_755_7 + ltmp_749_7;
  ltmp_757_A = (plzo_int64_t )ltmp_756_7;
  ltmp_758_3 = *(&ltmp_738_13[(ltmp_757_A + ltmp_535_A)]);
  ltmp_759_3 = *(&ltmp_738_13[(ltmp_757_A + ltmp_751_A)]);
  if ((ltmp_758_3 == ltmp_759_3)) {
    goto ltmp_1019_D;
  } else {
    ltmp_762_7_2E = ltmp_756_7;
    goto ltmp_1016_D;
  }

ltmp_1019_D:
  ltmp_760_7 = ltmp_756_7 + 1u;
  ltmp_761_7 = ltmp_755_7 + 1u;
  if ((ltmp_760_7 < ltmp_754_7)) {
    ltmp_755_7_2E = ltmp_761_7;
    goto ltmp_1018_D;
  } else {
    ltmp_762_7_2E = ltmp_760_7;
    goto ltmp_1016_D;
  }

  } while (1);
ltmp_1020_D:
  *ltmp_529_17 = ltmp_762_7;
  goto ltmp_923_D;

  do {
ltmp_1021_D:
  ltmp_768_7 = ltmp_768_7_2E;
  ltmp_769_7 = ltmp_769_7_2E;
  *(&ltmp_524_1038->field1[((plzo_int64_t )((ltmp_768_7 - ltmp_769_7) + 4294967295u))]) = ltmp_750_7;
  ltmp_770_7 = ltmp_769_7 + 1u;
  if ((ltmp_770_7 == ltmp_768_7)) {
    goto ltmp_1020_D;
  } else {
    ltmp_768_7_2E = ltmp_768_7;
    ltmp_769_7_2E = ltmp_770_7;
    goto ltmp_1021_D;
  }

  } while (1);
ltmp_1017_D:
  ltmp_763_7 = ltmp_762_7 - ltmp_749_7;
  if ((ltmp_763_7 < 8u)) {
    goto ltmp_1022_D;
  } else {
    ltmp_768_7_2E = 8u;
    ltmp_769_7_2E = 0u;
    goto ltmp_1021_D;
  }

  do {
ltmp_1023_D:
  ltmp_764_7 = ltmp_764_7_2E;
  ltmp_765_7 = ltmp_765_7_2E;
  ltmp_766_7 = (ltmp_764_7 - ltmp_765_7) + 4294967295u;
  ltmp_767_7 = *(&ltmp_524_1038->field1[((plzo_int64_t )(ltmp_766_7 - ltmp_763_7))]);
  *(&ltmp_524_1038->field1[((plzo_int64_t )ltmp_766_7)]) = ltmp_767_7;
  if ((ltmp_766_7 > ltmp_763_7)) {
    ltmp_764_7_2E = ltmp_764_7;
    ltmp_765_7_2E = (ltmp_765_7 + 1u);
    goto ltmp_1023_D;
  } else {
    ltmp_768_7_2E = ltmp_766_7;
    ltmp_769_7_2E = 0u;
    goto ltmp_1021_D;
  }

  } while (1);
ltmp_1022_D:
  if ((ltmp_762_7 < 8u)) {
    goto ltmp_1024_D;
  } else {
    ltmp_764_7_2E = 8u;
    ltmp_765_7_2E = 0u;
    goto ltmp_1023_D;
  }

ltmp_1024_D:
  ltmp_764_7_2E = ltmp_762_7;
  ltmp_765_7_2E = 0u;
  goto ltmp_1023_D;

ltmp_920_D:
  *(&ltmp_524_1038->field0) = 0u;
  ltmp_771_1032 = ltmp_18_100A(ltmp_503_1036);
  *(&ltmp_771_1032->field1) = ((struct l_struct_2E_M *)0);
  *(&ltmp_771_1032->field0) = ((struct l_struct_2E_M *)0);
  goto ltmp_923_D;

ltmp_1025_D:
  ltmp_781_1038 = &ltmp_524_1038[ltmp_778_A];
  ltmp_782_7 = (ltmp_772_7 + ltmp_523_7) + 4294967295u;
  ltmp_783_7_2E = ltmp_782_7;
  ltmp_784_1038_2E = ltmp_781_1038;
  goto ltmp_922_D;

  do {
ltmp_1026_D:
  ltmp_777_9 = ltmp_777_9_2E;
  ltmp_778_A = ((plzo_int64_t )ltmp_777_9) + PLZO_INT64_C(1);
  *(&ltmp_524_1038[ltmp_778_A].field0) = 0u;
  ltmp_779_1032 = ltmp_18_100A(ltmp_503_1036);
  *(&ltmp_779_1032->field1) = ((struct l_struct_2E_M *)0);
  *(&ltmp_779_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_780_9 = ltmp_777_9 + PLZO_UINT64_C(1);
  if ((ltmp_780_9 == ltmp_776_9)) {
    goto ltmp_1025_D;
  } else {
    ltmp_777_9_2E = ltmp_780_9;
    goto ltmp_1026_D;
  }

  } while (1);
ltmp_924_D:
  ltmp_774_7 = ltmp_772_7 + 4294967295u;
  ltmp_775_7 = *ltmp_520_17;
  *ltmp_520_17 = (ltmp_775_7 - ltmp_774_7);
  ltmp_776_9 = (plzo_uint64_t )ltmp_774_7;
  ltmp_777_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1026_D;

  } while (1);
ltmp_913_D:
  ltmp_507_17 = &ltmp_503_1036->field3;
  ltmp_508_17 = &ltmp_503_1036->field2;
  ltmp_509_1025 = &ltmp_503_1036->field0;
  ltmp_510_17 = &ltmp_503_1036->field12;
  ltmp_511_17 = &ltmp_503_1036->field14;
  ltmp_512_1025 = &ltmp_503_1036->field25[PLZO_INT64_C(1)];
  ltmp_513_1025 = &ltmp_503_1036->field25[PLZO_INT64_C(2)];
  ltmp_514_18 = &ltmp_503_1036->field22;
  ltmp_515_17 = &ltmp_503_1036->field1;
  ltmp_516_17 = &ltmp_503_1036->field21;
  ltmp_517_17 = &ltmp_503_1036->field19;
  ltmp_518_103A = &ltmp_503_1036->field18;
  ltmp_519_17 = &ltmp_503_1036->field15;
  ltmp_520_17 = &ltmp_503_1036->field4;
  ltmp_521_17 = &ltmp_503_1036->field13;
  ltmp_522_17 = &ltmp_503_1036->field16;
  ltmp_523_7_2E = ltmp_504_7;
  ltmp_524_1038_2E = ltmp_501_1038;
  goto ltmp_919_D;

ltmp_911_D:
  ltmp_795_7 = *ltmp_370_17;
  if (((ltmp_795_7 == ltmp_791_7) | (ltmp_362_8 < 2))) {
    ltmp_799_7_2E = ltmp_791_7;
    goto ltmp_907_D;
  } else {
    goto ltmp_909_D;
  }

ltmp_909_D:
  ltmp_796_1029 = *ltmp_353_102C;
  ltmp_797_102A = *(&ltmp_796_1029[PLZO_INT64_C(19)]);
  ltmp_798_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_797_102A)(ltmp_342_1023, ltmp_472_7, ltmp_791_7);
  ltmp_799_7_2E = ltmp_798_7;
  goto ltmp_907_D;

ltmp_906_D:
  if ((ltmp_362_8 > 1)) {
    goto ltmp_904_D;
  } else {
    goto ltmp_902_D;
  }

ltmp_1027_D:
  ltmp_809_102A = *(&ltmp_808_1029[PLZO_INT64_C(14)]);
  ltmp_810_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_809_102A)(ltmp_342_1023, ltmp_472_7, ltmp_799_7);
  goto ltmp_902_D;

ltmp_904_D:
  ltmp_808_1029 = *ltmp_353_102C;
  if ((ltmp_362_8 == 2)) {
    goto ltmp_1027_D;
  } else {
    goto ltmp_1028_D;
  }

ltmp_1029_D:
  ltmp_814_102A = *(&ltmp_813_1029[PLZO_INT64_C(13)]);
  ltmp_815_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_814_102A)(ltmp_342_1023, ltmp_472_7, ltmp_799_7);
  goto ltmp_902_D;

ltmp_1028_D:
  ltmp_811_102A = *(&ltmp_808_1029[PLZO_INT64_C(18)]);
  ltmp_812_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *))ltmp_811_102A)(ltmp_342_1023);
  ltmp_813_1029 = *ltmp_353_102C;
  if ((ltmp_812_7 == 0u)) {
    goto ltmp_1030_D;
  } else {
    goto ltmp_1029_D;
  }

ltmp_1030_D:
  ltmp_816_102A = *(&ltmp_813_1029[PLZO_INT64_C(12)]);
  ltmp_817_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_816_102A)(ltmp_342_1023, ltmp_472_7, ltmp_799_7);
  goto ltmp_902_D;

ltmp_901_D:
  if ((ltmp_362_8 == 0)) {
    goto ltmp_897_D;
  } else {
    goto ltmp_899_D;
  }

ltmp_1031_D:
  ltmp_828_102A = *(&ltmp_827_1029[PLZO_INT64_C(16)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))ltmp_828_102A)(ltmp_342_1023, ltmp_472_7, ltmp_799_7, 1);
  goto ltmp_895_D;

ltmp_899_D:
  ltmp_827_1029 = *ltmp_353_102C;
  if ((ltmp_362_8 == 1)) {
    goto ltmp_1031_D;
  } else {
    goto ltmp_1032_D;
  }

ltmp_1032_D:
  ltmp_829_102A = *(&ltmp_827_1029[PLZO_INT64_C(15)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_829_102A)(ltmp_342_1023, ltmp_472_7, ltmp_799_7);
  goto ltmp_895_D;

ltmp_1033_D:
  ltmp_838_101A(ltmp_837_1026, ltmp_800_9, ltmp_835_9, 3);
  *ltmp_463_17 = ltmp_799_7;
  ltmp_472_7_2E = ltmp_799_7;
  ltmp_473_8_2E = 0;
  goto ltmp_893_D;

ltmp_1034_D:
  ltmp_838_101A = *(&ltmp_837_1026->field2);
  if ((ltmp_838_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t ))0))) {
    goto ltmp_1035_D;
  } else {
    goto ltmp_1033_D;
  }

ltmp_896_D:
  ltmp_832_13 = *ltmp_461_1025;
  ltmp_833_13 = *ltmp_462_1025;
  ltmp_834_A = plzo_external_ptrdiff_uchar(ltmp_832_13, ltmp_833_13, PLZO_UINT64_C(1));
  ltmp_835_9 = ((plzo_uint64_t )ltmp_834_A) & PLZO_UINT64_C(4294967295);
  ltmp_836_14 = *ltmp_445_102D;
  ltmp_837_1026 = (struct plzo_callback_t *)ltmp_836_14;
  if ((ltmp_836_14 == ((signed char *)0))) {
    goto ltmp_1035_D;
  } else {
    goto ltmp_1034_D;
  }

ltmp_1035_D:
  *ltmp_463_17 = ltmp_799_7;
  ltmp_472_7_2E = ltmp_799_7;
  ltmp_473_8_2E = 0;
  goto ltmp_893_D;

  } while (1);
ltmp_892_D:
  ltmp_839_8 = ltmp_839_8_2E;
  ltmp_840_102F = *ltmp_444_1035;
  if ((ltmp_840_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_1036_D;
  } else {
    goto ltmp_1037_D;
  }

ltmp_1037_D:
  ltmp_841_1019 = *(&ltmp_840_102F->field2.field1);
  ltmp_842_1030 = &ltmp_342_1023->field20.field3;
  ltmp_843_1031 = *ltmp_842_1030;
  ltmp_841_1019((&ltmp_840_102F->field2), ((signed char *)ltmp_843_1031));
  *ltmp_842_1030 = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  ltmp_19_100B(ltmp_363_1034);
  return ltmp_839_8;
ltmp_1036_D:
  ltmp_19_100B(ltmp_363_1034);
  return ltmp_839_8;
}


static void ltmp_10_1005(struct l_struct_2E_E *ltmp_1038_1023) {
  plzo_uint32_t *ltmp_1039_17;
  plzo_uint32_t ltmp_1040_7;
  plzo_uint32_t ltmp_1041_7;
  plzo_uint32_t ltmp_1042_7;
  plzo_int32_t  (**ltmp_1043_1029) ();
  plzo_int32_t  (*ltmp_1044_102A) ();
  unsigned char *ltmp_1045_13;
  plzo_int32_t  (**ltmp_1046_1029) ();
  plzo_int32_t  (*ltmp_1047_102A) ();

  ltmp_1039_17 = &ltmp_1038_1023->field13;
  ltmp_1040_7 = *ltmp_1039_17;
  if ((ltmp_1040_7 == 0u)) {
    goto ltmp_1048_D;
  } else {
    goto ltmp_1049_D;
  }

ltmp_1049_D:
  ltmp_1041_7 = *(&ltmp_1038_1023->field11);
  ltmp_1042_7 = ltmp_1041_7 - ltmp_1040_7;
  *(&ltmp_1038_1023->field12) = ltmp_1042_7;
  ltmp_1043_1029 = *(&ltmp_1038_1023->field0);
  ltmp_1044_102A = *(&ltmp_1043_1029[PLZO_INT64_C(3)]);
  ltmp_1045_13 = *(&ltmp_1038_1023->field8);
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint32_t , plzo_uint32_t ))ltmp_1044_102A)(ltmp_1038_1023, (&ltmp_1045_13[((plzo_uint64_t )ltmp_1042_7)]), ltmp_1040_7, 0u);
  *ltmp_1039_17 = 0u;
  goto ltmp_1048_D;

ltmp_1048_D:
  ltmp_1046_1029 = *(&ltmp_1038_1023->field0);
  ltmp_1047_102A = *(&ltmp_1046_1029[PLZO_INT64_C(5)]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_1047_102A)(ltmp_1038_1023);
  return;
}


static void ltmp_11_1000(struct l_struct_2E_E *ltmp_1050_1023, plzo_uint32_t ltmp_1051_7, plzo_uint32_t ltmp_1052_7) {
  struct l_struct_2E_A7 *ltmp_1053_1038;
  struct l_struct_2E_A7 *ltmp_1054_1038;
  struct l_struct_2E_A7 *ltmp_1054_1038_2E;
  plzo_uint32_t ltmp_1055_7;
  plzo_uint32_t *ltmp_1056_17;
  plzo_uint32_t ltmp_1057_7;
  plzo_uint32_t ltmp_1057_7_2E;
  struct l_struct_2E_A7 *ltmp_1058_1038;
  struct l_struct_2E_A7 *ltmp_1058_1038_2E;
  plzo_uint32_t ltmp_1059_7;
  plzo_uint32_t *ltmp_1060_17;
  plzo_uint32_t ltmp_1061_7;
  plzo_uint32_t ltmp_1062_7;
  plzo_uint32_t ltmp_1063_7;
  plzo_uint32_t ltmp_1064_7;
  struct l_struct_2E_A7 *ltmp_1065_1038;
  struct l_struct_2E_A7 *ltmp_1066_1038;

  ltmp_1053_1038 = *(&ltmp_1050_1023->field19.field0);
  if ((ltmp_1051_7 < ltmp_1052_7)) {
    goto ltmp_1067_D;
  } else {
    goto ltmp_1068_D;
  }

ltmp_1067_D:
  ltmp_1056_17 = &ltmp_1050_1023->field5;
  ltmp_1057_7_2E = 0u;
  ltmp_1058_1038_2E = ltmp_1053_1038;
  goto ltmp_1069_D;

  do {
ltmp_1069_D:
  ltmp_1057_7 = ltmp_1057_7_2E;
  ltmp_1058_1038 = ltmp_1058_1038_2E;
  ltmp_1059_7 = ltmp_1057_7 + ltmp_1051_7;
  ltmp_1060_17 = &ltmp_1058_1038->field0;
  ltmp_1061_7 = *ltmp_1060_17;
  if ((ltmp_1061_7 == 0u)) {
    goto ltmp_1070_D;
  } else {
    goto ltmp_1071_D;
  }

ltmp_1072_D:
  ltmp_1054_1038 = ltmp_1054_1038_2E;
  ltmp_1055_7 = ltmp_1057_7 + 1u;
  if (((ltmp_1059_7 + 1u) < ltmp_1052_7)) {
    ltmp_1057_7_2E = ltmp_1055_7;
    ltmp_1058_1038_2E = ltmp_1054_1038;
    goto ltmp_1069_D;
  } else {
    goto ltmp_1068_D;
  }

ltmp_1073_D:
  *ltmp_1060_17 = 0u;
  ltmp_1065_1038 = &ltmp_1058_1038[PLZO_INT64_C(1)];
  ltmp_1054_1038_2E = ltmp_1065_1038;
  goto ltmp_1072_D;

ltmp_1074_D:
  ltmp_1063_7 = *(&ltmp_1058_1038->field1[PLZO_INT64_C(0)]);
  ltmp_1064_7 = *(&ltmp_1050_1023->field4[((plzo_int64_t )ltmp_1061_7)]);
  if ((ltmp_1063_7 < ltmp_1064_7)) {
    goto ltmp_1070_D;
  } else {
    goto ltmp_1073_D;
  }

ltmp_1071_D:
  ltmp_1062_7 = *ltmp_1056_17;
  if ((ltmp_1061_7 > ltmp_1062_7)) {
    goto ltmp_1070_D;
  } else {
    goto ltmp_1074_D;
  }

ltmp_1070_D:
  ltmp_1066_1038 = &ltmp_1058_1038[PLZO_INT64_C(1)];
  ltmp_1054_1038_2E = ltmp_1066_1038;
  goto ltmp_1072_D;

  } while (1);
ltmp_1068_D:
  return;
}


static plzo_uint32_t ltmp_12_1008(struct l_struct_2E_E *ltmp_1075_1023, plzo_uint32_t ltmp_1076_7, plzo_uint32_t ltmp_1077_7) {
  struct l_struct_2E_E_3A__3A_A4 ltmp_1078_103B;
  plzo_uint32_t ltmp_1079_7;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1080_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1081_1031;
  plzo_uint32_t *ltmp_1082_17;
  plzo_uint32_t ltmp_1083_7;
  plzo_uint32_t ltmp_1084_7;
  unsigned char *ltmp_1085_13;
  plzo_uint64_t ltmp_1086_9;
  plzo_uint64_t ltmp_1087_9;
  plzo_uint64_t ltmp_1087_9_2E;
  plzo_uint64_t ltmp_1088_9;
  unsigned char *ltmp_1089_13;
  plzo_uint64_t ltmp_1090_9;
  plzo_uint64_t ltmp_1090_9_2E;
  plzo_uint64_t ltmp_1091_9;
  plzo_uint32_t ltmp_1092_7;
  struct l_struct_2E_A7 **ltmp_1093_1037;
  plzo_uint32_t *ltmp_1094_17;
  plzo_uint32_t *ltmp_1095_17;
  plzo_uint32_t *ltmp_1096_17;
  plzo_uint32_t *ltmp_1097_17;
  plzo_int32_t  (***ltmp_1098_102C) ();
  plzo_uint32_t *ltmp_1099_17;
  plzo_uint32_t *ltmp_1100_17;
  plzo_int32_t *ltmp_1101_18;
  plzo_uint32_t *ltmp_1102_17;
  plzo_uint32_t *ltmp_1103_17;
  plzo_uint32_t ltmp_1104_7;
  plzo_uint32_t ltmp_1104_7_2E;
  plzo_uint32_t ltmp_1105_7;
  struct l_struct_2E_A7 *ltmp_1106_1038;
  plzo_uint32_t ltmp_1107_7;
  plzo_uint32_t ltmp_1108_7;
  plzo_uint32_t ltmp_1109_7;
  plzo_uint32_t ltmp_1110_7;
  plzo_uint32_t ltmp_1111_7;
  plzo_uint32_t ltmp_1112_7;
  plzo_uint32_t ltmp_1113_7;
  plzo_uint32_t ltmp_1114_7;
  plzo_uint32_t ltmp_1114_7_2E;
  plzo_uint32_t ltmp_1115_7;
  plzo_uint32_t ltmp_1115_7_2E;
  plzo_uint32_t ltmp_1116_7;
  plzo_uint32_t ltmp_1117_7;
  plzo_uint32_t ltmp_1118_7;
  plzo_uint32_t ltmp_1119_7;
  plzo_uint32_t ltmp_1119_7_2E;
  plzo_uint32_t ltmp_1120_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1121_1031;
  plzo_uint32_t ltmp_1122_7;
  plzo_int64_t ltmp_1123_A;
  plzo_int32_t  (**ltmp_1124_1029) ();
  plzo_int32_t  (*ltmp_1125_102A) ();
  plzo_uint32_t ltmp_1126_7;
  plzo_uint32_t *ltmp_1127_17;
  plzo_uint32_t *ltmp_1128_17;
  plzo_uint32_t *ltmp_1129_17;
  plzo_uint32_t *ltmp_1130_17;
  plzo_uint32_t ltmp_1131_7;
  plzo_uint32_t ltmp_1132_7;
  plzo_uint32_t ltmp_1133_7;
  plzo_uint32_t ltmp_1133_7_2E;
  plzo_uint32_t ltmp_1134_7;
  plzo_int32_t  (**ltmp_1135_1029) ();
  plzo_int32_t  (*ltmp_1136_102A) ();
  plzo_uint32_t ltmp_1137_7;
  plzo_uint32_t ltmp_1138_7;
  plzo_uint32_t ltmp_1139_7;
  plzo_int32_t ltmp_1140_8;
  plzo_uint32_t ltmp_1141_7;
  plzo_uint32_t ltmp_1142_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1143_1031;
  plzo_uint32_t ltmp_1144_7;

  ltmp_1079_7 = ltmp_1077_7 - ltmp_1076_7;
  *(&ltmp_1075_1023->field20.field2) = (ltmp_1079_7 + 1u);
  ltmp_1080_1030 = &ltmp_1075_1023->field20.field3;
  ltmp_1081_1031 = *ltmp_1080_1030;
  ltmp_1082_17 = &ltmp_1075_1023->field20.field1;
  ltmp_1083_7 = *ltmp_1082_17;
  ltmp_1084_7 = ltmp_1083_7 << 4;
  ltmp_1085_13 = (unsigned char *)(&ltmp_1081_1031[((plzo_int64_t )(ltmp_1083_7*ltmp_1079_7))]);
  if ((ltmp_1084_7 == 0u)) {
    goto ltmp_1145_D;
  } else {
    goto ltmp_1146_D;
  }

ltmp_1146_D:
  ltmp_1086_9 = (plzo_uint64_t )ltmp_1084_7;
  ltmp_1087_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1147_D;

  do {
ltmp_1147_D:
  ltmp_1087_9 = ltmp_1087_9_2E;
  *(&ltmp_1085_13[ltmp_1087_9]) = 0;
  ltmp_1088_9 = ltmp_1087_9 + PLZO_UINT64_C(1);
  if ((ltmp_1088_9 == ltmp_1086_9)) {
    goto ltmp_1145_D;
  } else {
    ltmp_1087_9_2E = ltmp_1088_9;
    goto ltmp_1147_D;
  }

  } while (1);
ltmp_1145_D:
  ltmp_1089_13 = (unsigned char *)(&ltmp_1078_103B);
  ltmp_1090_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1148_D;

  do {
ltmp_1148_D:
  ltmp_1090_9 = ltmp_1090_9_2E;
  *(&ltmp_1089_13[ltmp_1090_9]) = 0;
  ltmp_1091_9 = ltmp_1090_9 + PLZO_UINT64_C(1);
  if ((ltmp_1091_9 == PLZO_UINT64_C(32))) {
    goto ltmp_1149_D;
  } else {
    ltmp_1090_9_2E = ltmp_1091_9;
    goto ltmp_1148_D;
  }

  } while (1);
ltmp_1149_D:
  *(&ltmp_1078_103B.field3) = ltmp_1076_7;
  *(&ltmp_1078_103B.field4) = ltmp_1077_7;
  *(&ltmp_1078_103B.field5) = ltmp_1079_7;
  if ((ltmp_1077_7 == ltmp_1076_7)) {
    goto ltmp_1150_D;
  } else {
    goto ltmp_1151_D;
  }

ltmp_1151_D:
  ltmp_1093_1037 = &ltmp_1075_1023->field19.field0;
  ltmp_1094_17 = &ltmp_1075_1023->field7.field12;
  ltmp_1095_17 = &ltmp_1075_1023->field7.field13;
  ltmp_1096_17 = &ltmp_1075_1023->field7.field6;
  ltmp_1097_17 = &ltmp_1075_1023->field21;
  ltmp_1098_102C = &ltmp_1075_1023->field0;
  ltmp_1099_17 = &ltmp_1078_103B.field0;
  ltmp_1100_17 = &ltmp_1075_1023->field7.field3;
  ltmp_1101_18 = &ltmp_1075_1023->field7.field14;
  ltmp_1102_17 = &ltmp_1078_103B.field1;
  ltmp_1103_17 = &ltmp_1078_103B.field2;
  ltmp_1104_7_2E = 0u;
  goto ltmp_1152_D;

  do {
ltmp_1152_D:
  ltmp_1104_7 = ltmp_1104_7_2E;
  ltmp_1105_7 = (ltmp_1079_7 - ltmp_1104_7) + 4294967295u;
  ltmp_1106_1038 = *ltmp_1093_1037;
  ltmp_1107_7 = *(&ltmp_1106_1038[((plzo_int64_t )ltmp_1105_7)].field0);
  ltmp_1108_7 = *ltmp_1094_17;
  ltmp_1109_7 = *ltmp_1095_17;
  ltmp_1110_7 = *ltmp_1096_17;
  if ((ltmp_1110_7 > ltmp_1107_7)) {
    ltmp_1114_7_2E = ltmp_1109_7;
    ltmp_1115_7_2E = ltmp_1108_7;
    goto ltmp_1153_D;
  } else {
    goto ltmp_1154_D;
  }

ltmp_1155_D:
  ltmp_1092_7 = ltmp_1104_7 + 1u;
  if ((ltmp_1092_7 == ltmp_1079_7)) {
    goto ltmp_1150_D;
  } else {
    ltmp_1104_7_2E = ltmp_1092_7;
    goto ltmp_1152_D;
  }

ltmp_1153_D:
  ltmp_1114_7 = ltmp_1114_7_2E;
  ltmp_1115_7 = ltmp_1115_7_2E;
  ltmp_1116_7 = ((((ltmp_1115_7 + ltmp_1114_7) < ltmp_1107_7)) ? ((ltmp_1107_7 - ltmp_1114_7)) : (ltmp_1115_7));
  ltmp_1117_7 = *ltmp_1097_17;
  if ((ltmp_1117_7 == 0u)) {
    goto ltmp_1155_D;
  } else {
    ltmp_1119_7_2E = 0u;
    goto ltmp_1156_D;
  }

ltmp_1154_D:
  ltmp_1111_7 = *ltmp_1100_17;
  ltmp_1112_7 = ltmp_1111_7 + 1u;
  ltmp_1113_7 = (plzo_uint32_t )(ltmp_1109_7 != 0u);
  ltmp_1114_7_2E = ltmp_1113_7;
  ltmp_1115_7_2E = ltmp_1112_7;
  goto ltmp_1153_D;

  do {
ltmp_1156_D:
  ltmp_1119_7 = ltmp_1119_7_2E;
  ltmp_1120_7 = (ltmp_1117_7 - ltmp_1119_7) + 4294967295u;
  ltmp_1121_1031 = *ltmp_1080_1030;
  ltmp_1122_7 = *ltmp_1082_17;
  ltmp_1123_A = (plzo_int64_t )((ltmp_1122_7*ltmp_1105_7) + ltmp_1120_7);
  ltmp_1124_1029 = *ltmp_1098_102C;
  ltmp_1125_102A = *(&ltmp_1124_1029[PLZO_INT64_C(17)]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t ))ltmp_1125_102A)(ltmp_1075_1023, (&ltmp_1078_103B), 0u, ltmp_1105_7, ltmp_1120_7);
  ltmp_1126_7 = *ltmp_1099_17;
  ltmp_1127_17 = &ltmp_1121_1031[ltmp_1123_A].field0;
  *ltmp_1127_17 = ltmp_1126_7;
  ltmp_1128_17 = &ltmp_1121_1031[ltmp_1123_A].field1;
  *ltmp_1128_17 = 0u;
  ltmp_1129_17 = &ltmp_1121_1031[ltmp_1123_A].field2;
  *ltmp_1129_17 = 0u;
  ltmp_1130_17 = &ltmp_1121_1031[ltmp_1123_A].field3;
  *ltmp_1130_17 = 0u;
  ltmp_1131_7 = *ltmp_1100_17;
  if ((ltmp_1131_7 > ltmp_1107_7)) {
    goto ltmp_1157_D;
  } else {
    ltmp_1133_7_2E = ltmp_1131_7;
    goto ltmp_1158_D;
  }

ltmp_1157_D:
  ltmp_1118_7 = ltmp_1119_7 + 1u;
  if ((ltmp_1118_7 == ltmp_1117_7)) {
    goto ltmp_1155_D;
  } else {
    ltmp_1119_7_2E = ltmp_1118_7;
    goto ltmp_1156_D;
  }

  do {
ltmp_1158_D:
  ltmp_1133_7 = ltmp_1133_7_2E;
  ltmp_1134_7 = (((ltmp_1133_7 == ltmp_1115_7)) ? (ltmp_1116_7) : (ltmp_1133_7));
  ltmp_1135_1029 = *ltmp_1098_102C;
  ltmp_1136_102A = *(&ltmp_1135_1029[PLZO_INT64_C(17)]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t ))ltmp_1136_102A)(ltmp_1075_1023, (&ltmp_1078_103B), ltmp_1134_7, ltmp_1105_7, ltmp_1120_7);
  ltmp_1137_7 = *ltmp_1099_17;
  ltmp_1138_7 = *ltmp_1127_17;
  if ((ltmp_1137_7 > ltmp_1138_7)) {
    goto ltmp_1159_D;
  } else {
    goto ltmp_1160_D;
  }

ltmp_1159_D:
  ltmp_1132_7 = ltmp_1134_7 + 1u;
  if ((ltmp_1132_7 > ltmp_1107_7)) {
    goto ltmp_1157_D;
  } else {
    ltmp_1133_7_2E = ltmp_1132_7;
    goto ltmp_1158_D;
  }

ltmp_1161_D:
  if (((ltmp_1140_8 & 1) == 0)) {
    goto ltmp_1162_D;
  } else {
    goto ltmp_1159_D;
  }

ltmp_1163_D:
  ltmp_1139_7 = *ltmp_1128_17;
  ltmp_1140_8 = *ltmp_1101_18;
  if ((ltmp_1139_7 == 0u)) {
    goto ltmp_1161_D;
  } else {
    goto ltmp_1164_D;
  }

ltmp_1160_D:
  if ((ltmp_1137_7 == ltmp_1138_7)) {
    goto ltmp_1163_D;
  } else {
    goto ltmp_1162_D;
  }

ltmp_1164_D:
  if (((((plzo_uint32_t )(ltmp_1140_8 >> 1)) & 1u) == 0u)) {
    goto ltmp_1162_D;
  } else {
    goto ltmp_1159_D;
  }

ltmp_1162_D:
  *ltmp_1127_17 = ltmp_1137_7;
  *ltmp_1128_17 = ltmp_1134_7;
  ltmp_1141_7 = *ltmp_1102_17;
  *ltmp_1129_17 = ltmp_1141_7;
  ltmp_1142_7 = *ltmp_1103_17;
  *ltmp_1130_17 = ltmp_1142_7;
  goto ltmp_1159_D;

  } while (1);
  } while (1);
  } while (1);
ltmp_1150_D:
  ltmp_1143_1031 = *ltmp_1080_1030;
  ltmp_1144_7 = *(&ltmp_1143_1031->field0);
  return ltmp_1144_7;
}


static plzo_uint32_t ltmp_13_1008(struct l_struct_2E_E *ltmp_1165_1023, plzo_uint32_t ltmp_1166_7, plzo_uint32_t ltmp_1167_7) {
  struct l_struct_2E_E_3A__3A_A4 ltmp_1168_103B;
  plzo_uint32_t ltmp_1169_7;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1170_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1171_1031;
  plzo_uint32_t ltmp_1172_7;
  plzo_uint32_t ltmp_1173_7;
  unsigned char *ltmp_1174_13;
  plzo_uint64_t ltmp_1175_9;
  plzo_uint64_t ltmp_1176_9;
  plzo_uint64_t ltmp_1176_9_2E;
  plzo_uint64_t ltmp_1177_9;
  plzo_int32_t  (***ltmp_1178_102C) ();
  plzo_int32_t  (**ltmp_1179_1029) ();
  plzo_int32_t  (*ltmp_1180_102A) ();
  plzo_uint32_t ltmp_1181_7;
  unsigned char *ltmp_1182_13;
  plzo_uint64_t ltmp_1183_9;
  plzo_uint64_t ltmp_1183_9_2E;
  plzo_uint64_t ltmp_1184_9;
  plzo_uint32_t ltmp_1185_7;
  struct l_struct_2E_A7 **ltmp_1186_1037;
  plzo_uint32_t *ltmp_1187_17;
  plzo_uint32_t *ltmp_1188_17;
  plzo_uint32_t *ltmp_1189_17;
  plzo_uint32_t *ltmp_1190_17;
  plzo_uint32_t *ltmp_1191_17;
  plzo_int32_t *ltmp_1192_18;
  plzo_uint32_t *ltmp_1193_17;
  plzo_uint32_t *ltmp_1194_17;
  plzo_uint32_t ltmp_1195_7;
  plzo_uint32_t ltmp_1195_7_2E;
  plzo_uint32_t ltmp_1196_7;
  struct l_struct_2E_A7 *ltmp_1197_1038;
  plzo_int64_t ltmp_1198_A;
  plzo_uint32_t ltmp_1199_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1200_1031;
  plzo_uint32_t ltmp_1201_7;
  plzo_uint32_t *ltmp_1202_17;
  plzo_uint32_t *ltmp_1203_17;
  plzo_uint32_t *ltmp_1204_17;
  plzo_uint32_t *ltmp_1205_17;
  plzo_uint32_t ltmp_1206_7;
  plzo_uint32_t ltmp_1207_7;
  plzo_uint32_t ltmp_1208_7;
  plzo_uint32_t ltmp_1209_7;
  plzo_uint32_t ltmp_1210_7;
  plzo_uint32_t ltmp_1211_7;
  plzo_uint32_t ltmp_1212_7;
  plzo_uint32_t ltmp_1212_7_2E;
  plzo_uint32_t ltmp_1213_7;
  plzo_uint32_t ltmp_1213_7_2E;
  plzo_uint32_t ltmp_1214_7;
  plzo_uint32_t ltmp_1215_7;
  plzo_uint32_t ltmp_1216_7;
  plzo_uint32_t ltmp_1216_7_2E;
  plzo_uint32_t ltmp_1217_7;
  plzo_int32_t  (**ltmp_1218_1029) ();
  plzo_int32_t  (*ltmp_1219_102A) ();
  plzo_uint32_t ltmp_1220_7;
  plzo_uint32_t ltmp_1221_7;
  plzo_uint32_t ltmp_1222_7;
  plzo_int32_t ltmp_1223_8;
  plzo_uint32_t ltmp_1224_7;
  plzo_uint32_t ltmp_1225_7;
  plzo_uint32_t ltmp_1226_7;
  plzo_uint32_t ltmp_1227_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1228_1031;
  plzo_uint32_t ltmp_1229_7;

  ltmp_1169_7 = ltmp_1167_7 - ltmp_1166_7;
  *(&ltmp_1165_1023->field20.field2) = (ltmp_1169_7 + 1u);
  ltmp_1170_1030 = &ltmp_1165_1023->field20.field3;
  ltmp_1171_1031 = *ltmp_1170_1030;
  ltmp_1172_7 = *(&ltmp_1165_1023->field20.field1);
  ltmp_1173_7 = ltmp_1172_7 << 4;
  ltmp_1174_13 = (unsigned char *)(&ltmp_1171_1031[((plzo_int64_t )(ltmp_1172_7*ltmp_1169_7))]);
  if ((ltmp_1173_7 == 0u)) {
    goto ltmp_1230_D;
  } else {
    goto ltmp_1231_D;
  }

ltmp_1231_D:
  ltmp_1175_9 = (plzo_uint64_t )ltmp_1173_7;
  ltmp_1176_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1232_D;

  do {
ltmp_1232_D:
  ltmp_1176_9 = ltmp_1176_9_2E;
  *(&ltmp_1174_13[ltmp_1176_9]) = 0;
  ltmp_1177_9 = ltmp_1176_9 + PLZO_UINT64_C(1);
  if ((ltmp_1177_9 == ltmp_1175_9)) {
    goto ltmp_1230_D;
  } else {
    ltmp_1176_9_2E = ltmp_1177_9;
    goto ltmp_1232_D;
  }

  } while (1);
ltmp_1230_D:
  ltmp_1178_102C = &ltmp_1165_1023->field0;
  ltmp_1179_1029 = *ltmp_1178_102C;
  ltmp_1180_102A = *(&ltmp_1179_1029[PLZO_INT64_C(18)]);
  ltmp_1181_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *))ltmp_1180_102A)(ltmp_1165_1023);
  ltmp_1182_13 = (unsigned char *)(&ltmp_1168_103B);
  ltmp_1183_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1233_D;

  do {
ltmp_1233_D:
  ltmp_1183_9 = ltmp_1183_9_2E;
  *(&ltmp_1182_13[ltmp_1183_9]) = 0;
  ltmp_1184_9 = ltmp_1183_9 + PLZO_UINT64_C(1);
  if ((ltmp_1184_9 == PLZO_UINT64_C(32))) {
    goto ltmp_1234_D;
  } else {
    ltmp_1183_9_2E = ltmp_1184_9;
    goto ltmp_1233_D;
  }

  } while (1);
ltmp_1234_D:
  *(&ltmp_1168_103B.field3) = ltmp_1166_7;
  *(&ltmp_1168_103B.field4) = ltmp_1167_7;
  *(&ltmp_1168_103B.field5) = ltmp_1169_7;
  if ((ltmp_1167_7 == ltmp_1166_7)) {
    goto ltmp_1235_D;
  } else {
    goto ltmp_1236_D;
  }

ltmp_1236_D:
  ltmp_1186_1037 = &ltmp_1165_1023->field19.field0;
  ltmp_1187_17 = &ltmp_1165_1023->field7.field12;
  ltmp_1188_17 = &ltmp_1165_1023->field7.field13;
  ltmp_1189_17 = &ltmp_1165_1023->field7.field6;
  ltmp_1190_17 = &ltmp_1165_1023->field7.field3;
  ltmp_1191_17 = &ltmp_1168_103B.field0;
  ltmp_1192_18 = &ltmp_1165_1023->field7.field14;
  ltmp_1193_17 = &ltmp_1168_103B.field1;
  ltmp_1194_17 = &ltmp_1168_103B.field2;
  ltmp_1195_7_2E = 0u;
  goto ltmp_1237_D;

  do {
ltmp_1237_D:
  ltmp_1195_7 = ltmp_1195_7_2E;
  ltmp_1196_7 = (ltmp_1169_7 - ltmp_1195_7) + 4294967295u;
  ltmp_1197_1038 = *ltmp_1186_1037;
  ltmp_1198_A = (plzo_int64_t )ltmp_1196_7;
  ltmp_1199_7 = *(&ltmp_1197_1038[ltmp_1198_A].field0);
  ltmp_1200_1031 = *ltmp_1170_1030;
  ltmp_1201_7 = *(&ltmp_1200_1031[(ltmp_1198_A + PLZO_INT64_C(1))].field0);
  ltmp_1202_17 = &ltmp_1200_1031[ltmp_1198_A].field0;
  *ltmp_1202_17 = (ltmp_1201_7 + ltmp_1181_7);
  ltmp_1203_17 = &ltmp_1200_1031[ltmp_1198_A].field1;
  *ltmp_1203_17 = 0u;
  ltmp_1204_17 = &ltmp_1200_1031[ltmp_1198_A].field2;
  *ltmp_1204_17 = 0u;
  ltmp_1205_17 = &ltmp_1200_1031[ltmp_1198_A].field3;
  *ltmp_1205_17 = 0u;
  if ((ltmp_1199_7 == 0u)) {
    goto ltmp_1238_D;
  } else {
    goto ltmp_1239_D;
  }

ltmp_1238_D:
  ltmp_1185_7 = ltmp_1195_7 + 1u;
  if ((ltmp_1185_7 == ltmp_1169_7)) {
    goto ltmp_1235_D;
  } else {
    ltmp_1195_7_2E = ltmp_1185_7;
    goto ltmp_1237_D;
  }

ltmp_1240_D:
  ltmp_1212_7 = ltmp_1212_7_2E;
  ltmp_1213_7 = ltmp_1213_7_2E;
  ltmp_1214_7 = ((((ltmp_1213_7 + ltmp_1212_7) < ltmp_1199_7)) ? ((ltmp_1199_7 - ltmp_1213_7)) : (ltmp_1212_7));
  ltmp_1215_7 = *ltmp_1190_17;
  if ((ltmp_1215_7 > ltmp_1199_7)) {
    goto ltmp_1238_D;
  } else {
    ltmp_1216_7_2E = ltmp_1215_7;
    goto ltmp_1241_D;
  }

ltmp_1239_D:
  ltmp_1206_7 = *ltmp_1187_17;
  ltmp_1207_7 = *ltmp_1188_17;
  ltmp_1208_7 = *ltmp_1189_17;
  if ((ltmp_1208_7 > ltmp_1199_7)) {
    ltmp_1212_7_2E = ltmp_1206_7;
    ltmp_1213_7_2E = ltmp_1207_7;
    goto ltmp_1240_D;
  } else {
    goto ltmp_1242_D;
  }

ltmp_1242_D:
  ltmp_1209_7 = *ltmp_1190_17;
  ltmp_1210_7 = ltmp_1209_7 + 1u;
  ltmp_1211_7 = (plzo_uint32_t )(ltmp_1207_7 != 0u);
  ltmp_1212_7_2E = ltmp_1210_7;
  ltmp_1213_7_2E = ltmp_1211_7;
  goto ltmp_1240_D;

  do {
ltmp_1241_D:
  ltmp_1216_7 = ltmp_1216_7_2E;
  ltmp_1217_7 = (((ltmp_1216_7 == ltmp_1212_7)) ? (ltmp_1214_7) : (ltmp_1216_7));
  ltmp_1218_1029 = *ltmp_1178_102C;
  ltmp_1219_102A = *(&ltmp_1218_1029[PLZO_INT64_C(17)]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t ))ltmp_1219_102A)(ltmp_1165_1023, (&ltmp_1168_103B), ltmp_1217_7, ltmp_1196_7, 0u);
  ltmp_1220_7 = *ltmp_1191_17;
  ltmp_1221_7 = *ltmp_1202_17;
  if ((ltmp_1220_7 > ltmp_1221_7)) {
    goto ltmp_1243_D;
  } else {
    goto ltmp_1244_D;
  }

ltmp_1245_D:
  *ltmp_1202_17 = ltmp_1220_7;
  *ltmp_1203_17 = ltmp_1217_7;
  ltmp_1224_7 = *ltmp_1193_17;
  *ltmp_1204_17 = ltmp_1224_7;
  ltmp_1225_7 = *ltmp_1194_17;
  *ltmp_1205_17 = ltmp_1225_7;
  ltmp_1226_7 = ltmp_1217_7 + 1u;
  if ((ltmp_1226_7 > ltmp_1199_7)) {
    goto ltmp_1238_D;
  } else {
    ltmp_1216_7_2E = ltmp_1226_7;
    goto ltmp_1241_D;
  }

ltmp_1244_D:
  if ((ltmp_1220_7 == ltmp_1221_7)) {
    goto ltmp_1246_D;
  } else {
    goto ltmp_1245_D;
  }

ltmp_1247_D:
  if (((ltmp_1223_8 & 1) == 0)) {
    goto ltmp_1245_D;
  } else {
    goto ltmp_1243_D;
  }

ltmp_1246_D:
  ltmp_1222_7 = *ltmp_1203_17;
  ltmp_1223_8 = *ltmp_1192_18;
  if ((ltmp_1222_7 == 0u)) {
    goto ltmp_1247_D;
  } else {
    goto ltmp_1248_D;
  }

ltmp_1248_D:
  if (((((plzo_uint32_t )(ltmp_1223_8 >> 1)) & 1u) == 0u)) {
    goto ltmp_1245_D;
  } else {
    goto ltmp_1243_D;
  }

ltmp_1243_D:
  ltmp_1227_7 = ltmp_1217_7 + 1u;
  if ((ltmp_1227_7 > ltmp_1199_7)) {
    goto ltmp_1238_D;
  } else {
    ltmp_1216_7_2E = ltmp_1227_7;
    goto ltmp_1241_D;
  }

  } while (1);
  } while (1);
ltmp_1235_D:
  ltmp_1228_1031 = *ltmp_1170_1030;
  ltmp_1229_7 = *(&ltmp_1228_1031->field0);
  return ltmp_1229_7;
}


static plzo_uint32_t ltmp_14_1008(struct l_struct_2E_E *ltmp_1249_1023, plzo_uint32_t ltmp_1250_7, plzo_uint32_t ltmp_1251_7) {
  struct l_struct_2E_E_3A__3A_A4 ltmp_1252_103B;
  struct l_struct_2E_E_3A__3A_A4 ltmp_1253_103B;
  plzo_uint32_t ltmp_1254_7;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1255_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1256_1031;
  plzo_uint32_t ltmp_1257_7;
  plzo_uint32_t ltmp_1258_7;
  unsigned char *ltmp_1259_13;
  plzo_uint64_t ltmp_1260_9;
  plzo_uint64_t ltmp_1261_9;
  plzo_uint64_t ltmp_1261_9_2E;
  plzo_uint64_t ltmp_1262_9;
  plzo_int32_t  (***ltmp_1263_102C) ();
  plzo_int32_t  (**ltmp_1264_1029) ();
  plzo_int32_t  (*ltmp_1265_102A) ();
  plzo_uint32_t ltmp_1266_7;
  unsigned char *ltmp_1267_13;
  plzo_uint64_t ltmp_1268_9;
  plzo_uint64_t ltmp_1268_9_2E;
  plzo_uint64_t ltmp_1269_9;
  unsigned char *ltmp_1270_13;
  plzo_uint64_t ltmp_1271_9;
  plzo_uint64_t ltmp_1271_9_2E;
  plzo_uint64_t ltmp_1272_9;
  struct l_struct_2E_A7 **ltmp_1273_1037;
  plzo_uint32_t *ltmp_1274_17;
  plzo_uint32_t *ltmp_1275_17;
  plzo_uint32_t ltmp_1276_7;
  plzo_uint32_t ltmp_1276_7_2E;
  plzo_uint32_t ltmp_1277_7;
  struct l_struct_2E_A7 *ltmp_1278_1038;
  plzo_int64_t ltmp_1279_A;
  plzo_uint32_t ltmp_1280_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1281_1031;
  plzo_uint32_t ltmp_1282_7;
  plzo_uint32_t ltmp_1283_7;
  plzo_int32_t  (**ltmp_1284_1029) ();
  plzo_int32_t  (*ltmp_1285_102A) ();
  plzo_uint32_t ltmp_1286_7;
  plzo_uint32_t *ltmp_1287_17;
  plzo_uint32_t *ltmp_1288_17;
  plzo_uint32_t ltmp_1289_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1290_1031;
  plzo_uint32_t ltmp_1291_7;

  ltmp_1254_7 = ltmp_1251_7 - ltmp_1250_7;
  *(&ltmp_1249_1023->field20.field2) = (ltmp_1254_7 + 1u);
  ltmp_1255_1030 = &ltmp_1249_1023->field20.field3;
  ltmp_1256_1031 = *ltmp_1255_1030;
  ltmp_1257_7 = *(&ltmp_1249_1023->field20.field1);
  ltmp_1258_7 = ltmp_1257_7 << 4;
  ltmp_1259_13 = (unsigned char *)(&ltmp_1256_1031[((plzo_int64_t )(ltmp_1257_7*ltmp_1254_7))]);
  if ((ltmp_1258_7 == 0u)) {
    goto ltmp_1292_D;
  } else {
    goto ltmp_1293_D;
  }

ltmp_1293_D:
  ltmp_1260_9 = (plzo_uint64_t )ltmp_1258_7;
  ltmp_1261_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1294_D;

  do {
ltmp_1294_D:
  ltmp_1261_9 = ltmp_1261_9_2E;
  *(&ltmp_1259_13[ltmp_1261_9]) = 0;
  ltmp_1262_9 = ltmp_1261_9 + PLZO_UINT64_C(1);
  if ((ltmp_1262_9 == ltmp_1260_9)) {
    goto ltmp_1292_D;
  } else {
    ltmp_1261_9_2E = ltmp_1262_9;
    goto ltmp_1294_D;
  }

  } while (1);
ltmp_1292_D:
  ltmp_1263_102C = &ltmp_1249_1023->field0;
  ltmp_1264_1029 = *ltmp_1263_102C;
  ltmp_1265_102A = *(&ltmp_1264_1029[PLZO_INT64_C(18)]);
  ltmp_1266_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *))ltmp_1265_102A)(ltmp_1249_1023);
  ltmp_1267_13 = (unsigned char *)(&ltmp_1252_103B);
  ltmp_1268_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1295_D;

  do {
ltmp_1295_D:
  ltmp_1268_9 = ltmp_1268_9_2E;
  *(&ltmp_1267_13[ltmp_1268_9]) = 0;
  ltmp_1269_9 = ltmp_1268_9 + PLZO_UINT64_C(1);
  if ((ltmp_1269_9 == PLZO_UINT64_C(32))) {
    goto ltmp_1296_D;
  } else {
    ltmp_1268_9_2E = ltmp_1269_9;
    goto ltmp_1295_D;
  }

  } while (1);
ltmp_1296_D:
  *(&ltmp_1252_103B.field3) = ltmp_1250_7;
  *(&ltmp_1252_103B.field4) = ltmp_1251_7;
  *(&ltmp_1252_103B.field5) = ltmp_1254_7;
  ltmp_1270_13 = (unsigned char *)(&ltmp_1253_103B);
  ltmp_1271_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1297_D;

  do {
ltmp_1297_D:
  ltmp_1271_9 = ltmp_1271_9_2E;
  *(&ltmp_1270_13[ltmp_1271_9]) = 0;
  ltmp_1272_9 = ltmp_1271_9 + PLZO_UINT64_C(1);
  if ((ltmp_1272_9 == PLZO_UINT64_C(32))) {
    goto ltmp_1298_D;
  } else {
    ltmp_1271_9_2E = ltmp_1272_9;
    goto ltmp_1297_D;
  }

  } while (1);
ltmp_1298_D:
  *(&ltmp_1253_103B.field3) = ltmp_1250_7;
  *(&ltmp_1253_103B.field4) = ltmp_1251_7;
  *(&ltmp_1253_103B.field5) = ltmp_1254_7;
  if ((ltmp_1251_7 == ltmp_1250_7)) {
    goto ltmp_1299_D;
  } else {
    goto ltmp_1300_D;
  }

ltmp_1300_D:
  ltmp_1273_1037 = &ltmp_1249_1023->field19.field0;
  ltmp_1274_17 = &ltmp_1252_103B.field0;
  ltmp_1275_17 = &ltmp_1253_103B.field0;
  ltmp_1276_7_2E = 0u;
  goto ltmp_1301_D;

  do {
ltmp_1301_D:
  ltmp_1276_7 = ltmp_1276_7_2E;
  ltmp_1277_7 = (ltmp_1254_7 - ltmp_1276_7) + 4294967295u;
  ltmp_1278_1038 = *ltmp_1273_1037;
  ltmp_1279_A = (plzo_int64_t )ltmp_1277_7;
  ltmp_1280_7 = *(&ltmp_1278_1038[ltmp_1279_A].field0);
  ltmp_1281_1031 = *ltmp_1255_1030;
  ltmp_1282_7 = *(&ltmp_1281_1031[(ltmp_1279_A + PLZO_INT64_C(1))].field0);
  ltmp_1283_7 = ltmp_1282_7 + ltmp_1266_7;
  *ltmp_1274_17 = ltmp_1283_7;
  ltmp_1284_1029 = *ltmp_1263_102C;
  ltmp_1285_102A = *(&ltmp_1284_1029[PLZO_INT64_C(17)]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t ))ltmp_1285_102A)(ltmp_1249_1023, (&ltmp_1253_103B), ltmp_1280_7, ltmp_1277_7, 0u);
  ltmp_1286_7 = *ltmp_1275_17;
  ltmp_1287_17 = &ltmp_1281_1031[ltmp_1279_A].field0;
  ltmp_1288_17 = &ltmp_1281_1031[ltmp_1279_A].field3;
  if ((ltmp_1283_7 > ltmp_1286_7)) {
    goto ltmp_1302_D;
  } else {
    goto ltmp_1303_D;
  }

ltmp_1304_D:
  ltmp_1289_7 = ltmp_1276_7 + 1u;
  ltmp_1276_7_2E = ltmp_1289_7;
  goto ltmp_1301_D;

ltmp_1303_D:
  *ltmp_1287_17 = ltmp_1283_7;
  *(&ltmp_1281_1031[ltmp_1279_A].field1) = 0u;
  *(&ltmp_1281_1031[ltmp_1279_A].field2) = 0u;
  *ltmp_1288_17 = 0u;
  if ((ltmp_1277_7 == 0u)) {
    goto ltmp_1299_D;
  } else {
    goto ltmp_1304_D;
  }

ltmp_1302_D:
  *ltmp_1287_17 = ltmp_1286_7;
  *(&ltmp_1281_1031[ltmp_1279_A].field1) = ltmp_1280_7;
  *(&ltmp_1281_1031[ltmp_1279_A].field2) = 0u;
  *ltmp_1288_17 = 0u;
  if ((ltmp_1277_7 == 0u)) {
    goto ltmp_1299_D;
  } else {
    goto ltmp_1304_D;
  }

  } while (1);
ltmp_1299_D:
  ltmp_1290_1031 = *ltmp_1255_1030;
  ltmp_1291_7 = *(&ltmp_1290_1031->field0);
  return ltmp_1291_7;
}


static void ltmp_15_1000(struct l_struct_2E_E *ltmp_1305_1023, plzo_uint32_t ltmp_1306_7, plzo_uint32_t ltmp_1307_7) {
  struct l_struct_2E_A7 *ltmp_1308_1038;
  plzo_uint32_t *ltmp_1309_17;
  plzo_uint32_t ltmp_1310_7;
  struct l_struct_2E_A7 *ltmp_1311_1038;
  plzo_uint32_t *ltmp_1312_17;
  plzo_uint32_t ltmp_1313_7;
  plzo_uint32_t ltmp_1314_7;
  plzo_uint32_t *ltmp_1315_17;
  plzo_uint32_t ltmp_1316_7;
  plzo_uint32_t ltmp_1317_7;
  plzo_uint32_t ltmp_1318_7;
  plzo_uint32_t ltmp_1319_7;
  plzo_uint32_t ltmp_1320_7;
  plzo_uint32_t ltmp_1320_7_2E;
  plzo_uint32_t ltmp_1321_7;
  plzo_uint32_t ltmp_1321_7_2E;
  struct l_struct_2E_A7 *ltmp_1322_1038;
  struct l_struct_2E_A7 *ltmp_1322_1038_2E;
  plzo_uint32_t ltmp_1323_7;
  plzo_uint32_t ltmp_1323_7_2E;
  plzo_uint32_t ltmp_1324_7;
  plzo_uint32_t ltmp_1324_7_2E;
  struct l_struct_2E_A7 *ltmp_1325_1038;
  struct l_struct_2E_A7 *ltmp_1325_1038_2E;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1326_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1327_1031;
  plzo_uint32_t *ltmp_1328_17;
  plzo_uint32_t ltmp_1329_7;
  plzo_int64_t ltmp_1330_A;
  plzo_uint32_t ltmp_1331_7;
  plzo_uint32_t ltmp_1332_7;
  plzo_uint32_t ltmp_1333_7;
  plzo_uint32_t ltmp_1334_7;
  plzo_uint32_t ltmp_1335_7;
  struct l_struct_2E_A7 *ltmp_1336_1038;
  plzo_uint32_t ltmp_1337_7;
  plzo_uint32_t ltmp_1338_7;
  struct l_struct_2E_A7 *ltmp_1339_1038;
  plzo_uint32_t *ltmp_1340_17;
  plzo_uint32_t ltmp_1341_7;
  plzo_int32_t ltmp_1342_8;
  plzo_uint32_t ltmp_1343_7;
  plzo_uint32_t ltmp_1344_7;
  plzo_uint32_t ltmp_1345_7;
  plzo_uint32_t ltmp_1346_7;
  plzo_uint32_t ltmp_1347_7;
  plzo_uint32_t ltmp_1348_7;
  plzo_uint32_t ltmp_1349_7;
  plzo_uint32_t ltmp_1350_7;
  plzo_uint32_t ltmp_1351_7;
  plzo_uint32_t ltmp_1352_7;
  plzo_uint32_t ltmp_1353_7;
  plzo_uint32_t ltmp_1354_7;
  plzo_uint32_t ltmp_1355_7;
  plzo_uint32_t ltmp_1356_7;
  plzo_uint32_t ltmp_1357_7;
  plzo_uint32_t ltmp_1357_7_2E;
  plzo_uint32_t ltmp_1358_7;
  plzo_uint32_t ltmp_1359_7;
  plzo_uint32_t ltmp_1360_7;
  plzo_uint32_t ltmp_1361_7;
  plzo_uint32_t ltmp_1361_7_2E;
  unsigned char *ltmp_1362_13;
  plzo_uint64_t ltmp_1363_9;
  plzo_int64_t ltmp_1364_A;
  plzo_uint64_t ltmp_1365_9;
  plzo_uint64_t ltmp_1365_9_2E;
  unsigned char ltmp_1366_3;
  unsigned char ltmp_1367_3;
  plzo_int32_t ltmp_1368_8;
  plzo_uint64_t ltmp_1369_9;
  plzo_int32_t ltmp_1370_8;
  plzo_int32_t ltmp_1370_8_2E;
  plzo_uint32_t ltmp_1371_7;
  plzo_uint32_t ltmp_1371_7_2E;
  plzo_uint32_t *ltmp_1372_17;
  plzo_uint32_t ltmp_1373_7;
  plzo_uint32_t ltmp_1374_7;
  plzo_int32_t  (***ltmp_1375_102C) ();
  plzo_int32_t  (**ltmp_1376_1029) ();
  plzo_int32_t  (*ltmp_1377_102A) ();
  unsigned char *ltmp_1378_13;
  plzo_int32_t  (**ltmp_1379_1029) ();
  plzo_int32_t  (*ltmp_1380_102A) ();
  plzo_uint32_t ltmp_1381_7;
  plzo_uint32_t ltmp_1382_7;
  plzo_uint32_t ltmp_1383_7;
  struct l_struct_2E_A7 *ltmp_1384_1038;
  plzo_uint32_t ltmp_1385_7;
  plzo_uint32_t ltmp_1386_7;
  plzo_uint32_t ltmp_1387_7;
  plzo_uint32_t ltmp_1388_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1389_1031;
  plzo_uint32_t ltmp_1390_7;
  plzo_uint32_t ltmp_1391_7;
  plzo_uint32_t ltmp_1392_7;
  struct l_struct_2E_A7 *ltmp_1393_1038;

  ltmp_1308_1038 = *(&ltmp_1305_1023->field19.field0);
  ltmp_1309_17 = &ltmp_1305_1023->field14;
  ltmp_1310_7 = *ltmp_1309_17;
  ltmp_1311_1038 = &ltmp_1308_1038[((plzo_int64_t )ltmp_1310_7)];
  ltmp_1312_17 = &ltmp_1305_1023->field13;
  ltmp_1313_7 = *ltmp_1312_17;
  ltmp_1314_7 = ltmp_1310_7 + ltmp_1313_7;
  *ltmp_1312_17 = ltmp_1314_7;
  ltmp_1315_17 = &ltmp_1305_1023->field21;
  ltmp_1316_7 = *ltmp_1315_17;
  if ((ltmp_1314_7 < ltmp_1316_7)) {
    goto ltmp_1394_D;
  } else {
    goto ltmp_1395_D;
  }

ltmp_1394_D:
  ltmp_1317_7 = ltmp_1310_7 + ltmp_1306_7;
  if ((ltmp_1317_7 < ltmp_1307_7)) {
    ltmp_1323_7_2E = ltmp_1314_7;
    ltmp_1324_7_2E = ltmp_1317_7;
    ltmp_1325_1038_2E = ltmp_1311_1038;
    goto ltmp_1396_D;
  } else {
    goto ltmp_1397_D;
  }

ltmp_1395_D:
  ltmp_1318_7 = ltmp_1316_7 + 4294967295u;
  ltmp_1319_7 = ltmp_1310_7 + ltmp_1306_7;
  ltmp_1320_7_2E = ltmp_1318_7;
  ltmp_1321_7_2E = ltmp_1319_7;
  ltmp_1322_1038_2E = ltmp_1311_1038;
  goto ltmp_1398_D;

ltmp_1398_D:
  ltmp_1320_7 = ltmp_1320_7_2E;
  ltmp_1321_7 = ltmp_1321_7_2E;
  ltmp_1322_1038 = ltmp_1322_1038_2E;
  if ((ltmp_1321_7 < ltmp_1307_7)) {
    ltmp_1323_7_2E = ltmp_1320_7;
    ltmp_1324_7_2E = ltmp_1321_7;
    ltmp_1325_1038_2E = ltmp_1322_1038;
    goto ltmp_1396_D;
  } else {
    goto ltmp_1397_D;
  }

ltmp_1396_D:
  ltmp_1323_7 = ltmp_1323_7_2E;
  ltmp_1324_7 = ltmp_1324_7_2E;
  ltmp_1325_1038 = ltmp_1325_1038_2E;
  ltmp_1326_1030 = &ltmp_1305_1023->field20.field3;
  ltmp_1327_1031 = *ltmp_1326_1030;
  ltmp_1328_17 = &ltmp_1305_1023->field20.field1;
  ltmp_1329_7 = *ltmp_1328_17;
  ltmp_1330_A = (plzo_int64_t )((ltmp_1329_7*(ltmp_1324_7 - ltmp_1306_7)) + ltmp_1323_7);
  ltmp_1331_7 = *(&ltmp_1327_1031[ltmp_1330_A].field1);
  if ((ltmp_1331_7 == 0u)) {
    goto ltmp_1399_D;
  } else {
    goto ltmp_1400_D;
  }

ltmp_1399_D:
  ltmp_1332_7 = *ltmp_1312_17;
  ltmp_1333_7 = ltmp_1332_7 + 1u;
  *ltmp_1312_17 = ltmp_1333_7;
  ltmp_1334_7 = *ltmp_1315_17;
  if ((ltmp_1333_7 < ltmp_1334_7)) {
    goto ltmp_1401_D;
  } else {
    goto ltmp_1402_D;
  }

ltmp_1401_D:
  ltmp_1335_7 = ltmp_1324_7 + 1u;
  ltmp_1336_1038 = &ltmp_1325_1038[PLZO_INT64_C(1)];
  ltmp_1320_7_2E = ltmp_1333_7;
  ltmp_1321_7_2E = ltmp_1335_7;
  ltmp_1322_1038_2E = ltmp_1336_1038;
  goto ltmp_1398_D;

ltmp_1402_D:
  ltmp_1337_7 = ltmp_1334_7 + 4294967295u;
  ltmp_1338_7 = ltmp_1324_7 + 1u;
  ltmp_1339_1038 = &ltmp_1325_1038[PLZO_INT64_C(1)];
  ltmp_1320_7_2E = ltmp_1337_7;
  ltmp_1321_7_2E = ltmp_1338_7;
  ltmp_1322_1038_2E = ltmp_1339_1038;
  goto ltmp_1398_D;

ltmp_1400_D:
  ltmp_1340_17 = &ltmp_1327_1031[ltmp_1330_A].field2;
  ltmp_1341_7 = *ltmp_1340_17;
  if ((ltmp_1341_7 == 0u)) {
    goto ltmp_1403_D;
  } else {
    ltmp_1371_7_2E = ltmp_1341_7;
    goto ltmp_1404_D;
  }

ltmp_1403_D:
  ltmp_1342_8 = *(&ltmp_1305_1023->field6.field1);
  if (((((plzo_uint32_t )ltmp_1342_8) & 1u) == 0u)) {
    goto ltmp_1405_D;
  } else {
    goto ltmp_1406_D;
  }

ltmp_1405_D:
  ltmp_1343_7 = *(&ltmp_1325_1038->field0);
  ltmp_1344_7 = ltmp_1343_7 - ltmp_1331_7;
  ltmp_1345_7 = *(&ltmp_1325_1038->field1[((((ltmp_1344_7 < 8u)) ? (((plzo_int64_t )ltmp_1344_7)) : (PLZO_INT64_C(7))))]);
  ltmp_1371_7_2E = ltmp_1345_7;
  goto ltmp_1404_D;

ltmp_1406_D:
  ltmp_1346_7 = *(&ltmp_1305_1023->field5);
  if ((ltmp_1346_7 < ltmp_1331_7)) {
    goto ltmp_1407_D;
  } else {
    goto ltmp_1408_D;
  }

ltmp_1408_D:
  ltmp_1347_7 = *(&ltmp_1305_1023->field16);
  ltmp_1348_7 = *(&ltmp_1305_1023->field4[((plzo_int64_t )ltmp_1331_7)]);
  if ((ltmp_1347_7 < ltmp_1348_7)) {
    goto ltmp_1407_D;
  } else {
    goto ltmp_1409_D;
  }

ltmp_1409_D:
  ltmp_1349_7 = *(&ltmp_1325_1038->field0);
  ltmp_1350_7 = ltmp_1349_7 - ltmp_1331_7;
  ltmp_1351_7 = *(&ltmp_1325_1038->field1[((((ltmp_1350_7 < 8u)) ? (((plzo_int64_t )ltmp_1350_7)) : (PLZO_INT64_C(7))))]);
  ltmp_1371_7_2E = ltmp_1351_7;
  goto ltmp_1404_D;

ltmp_1407_D:
  ltmp_1352_7 = *(&ltmp_1305_1023->field16);
  ltmp_1353_7 = *(&ltmp_1325_1038->field0);
  ltmp_1354_7 = ltmp_1353_7 - ltmp_1331_7;
  ltmp_1355_7 = (((ltmp_1354_7 < 8u)) ? (ltmp_1354_7) : (7u));
  ltmp_1356_7 = *(&ltmp_1325_1038->field1[((plzo_int64_t )ltmp_1355_7)]);
  if (((ltmp_1355_7 != 0u) & (ltmp_1356_7 < ltmp_1352_7))) {
    ltmp_1357_7_2E = 0u;
    goto ltmp_1410_D;
  } else {
    ltmp_1361_7_2E = ltmp_1356_7;
    goto ltmp_1411_D;
  }

  do {
ltmp_1410_D:
  ltmp_1357_7 = ltmp_1357_7_2E;
  ltmp_1358_7 = ltmp_1355_7 - ltmp_1357_7;
  ltmp_1359_7 = *(&ltmp_1325_1038->field1[((plzo_int64_t )(ltmp_1358_7 + 4294967295u))]);
  if ((ltmp_1359_7 == ltmp_1352_7)) {
    ltmp_1361_7_2E = ltmp_1352_7;
    goto ltmp_1411_D;
  } else {
    goto ltmp_1412_D;
  }

ltmp_1412_D:
  ltmp_1360_7 = ltmp_1357_7 + 1u;
  if ((ltmp_1358_7 == 1u)) {
    ltmp_1361_7_2E = ltmp_1356_7;
    goto ltmp_1411_D;
  } else {
    ltmp_1357_7_2E = ltmp_1360_7;
    goto ltmp_1410_D;
  }

  } while (1);
ltmp_1411_D:
  ltmp_1361_7 = ltmp_1361_7_2E;
  if ((((ltmp_1352_7 == ltmp_1361_7) | (ltmp_1331_7 < 2u)) | (ltmp_1352_7 == 4294967295u))) {
    ltmp_1371_7_2E = ltmp_1361_7;
    goto ltmp_1404_D;
  } else {
    goto ltmp_1413_D;
  }

ltmp_1413_D:
  ltmp_1362_13 = *(&ltmp_1305_1023->field8);
  ltmp_1363_9 = (plzo_uint64_t )ltmp_1324_7;
  ltmp_1364_A = ((plzo_int64_t )ltmp_1324_7) - ((plzo_int64_t )ltmp_1352_7);
  if ((ltmp_1331_7 == 0u)) {
    ltmp_1370_8_2E = 0;
    goto ltmp_1414_D;
  } else {
    ltmp_1365_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1415_D;
  }

  do {
ltmp_1415_D:
  ltmp_1365_9 = ltmp_1365_9_2E;
  ltmp_1366_3 = *(&ltmp_1362_13[(ltmp_1363_9 + ltmp_1365_9)]);
  ltmp_1367_3 = *(&ltmp_1362_13[(ltmp_1364_A + ((plzo_int64_t )ltmp_1365_9))]);
  ltmp_1368_8 = ((plzo_int32_t )ltmp_1366_3) - ((plzo_int32_t )ltmp_1367_3);
  if ((ltmp_1366_3 == ltmp_1367_3)) {
    goto ltmp_1416_D;
  } else {
    ltmp_1370_8_2E = ltmp_1368_8;
    goto ltmp_1414_D;
  }

ltmp_1416_D:
  ltmp_1369_9 = ltmp_1365_9 + PLZO_UINT64_C(1);
  if (((ltmp_1331_7 - ((plzo_uint32_t )ltmp_1365_9)) == 1u)) {
    ltmp_1370_8_2E = 0;
    goto ltmp_1414_D;
  } else {
    ltmp_1365_9_2E = ltmp_1369_9;
    goto ltmp_1415_D;
  }

  } while (1);
ltmp_1414_D:
  ltmp_1370_8 = ltmp_1370_8_2E;
  if ((ltmp_1370_8 == 0)) {
    goto ltmp_1417_D;
  } else {
    ltmp_1371_7_2E = ltmp_1361_7;
    goto ltmp_1404_D;
  }

ltmp_1417_D:
  ltmp_1371_7_2E = ltmp_1352_7;
  goto ltmp_1404_D;

ltmp_1404_D:
  ltmp_1371_7 = ltmp_1371_7_2E;
  ltmp_1372_17 = &ltmp_1305_1023->field12;
  ltmp_1373_7 = *ltmp_1312_17;
  ltmp_1374_7 = ltmp_1324_7 - ltmp_1373_7;
  *ltmp_1372_17 = ltmp_1374_7;
  ltmp_1375_102C = &ltmp_1305_1023->field0;
  ltmp_1376_1029 = *ltmp_1375_102C;
  ltmp_1377_102A = *(&ltmp_1376_1029[PLZO_INT64_C(3)]);
  ltmp_1378_13 = *(&ltmp_1305_1023->field8);
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint32_t , plzo_uint32_t ))ltmp_1377_102A)(ltmp_1305_1023, (&ltmp_1378_13[((plzo_uint64_t )ltmp_1374_7)]), ltmp_1373_7, ltmp_1331_7);
  *ltmp_1312_17 = 0u;
  *ltmp_1372_17 = ltmp_1324_7;
  ltmp_1379_1029 = *ltmp_1375_102C;
  ltmp_1380_102A = *(&ltmp_1379_1029[PLZO_INT64_C(4)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_1380_102A)(ltmp_1305_1023, ltmp_1331_7, ltmp_1371_7);
  ltmp_1381_7 = *(&ltmp_1327_1031[ltmp_1330_A].field3);
  if ((ltmp_1381_7 == 0u)) {
    goto ltmp_1418_D;
  } else {
    goto ltmp_1419_D;
  }

ltmp_1419_D:
  ltmp_1382_7 = ltmp_1381_7 + (ltmp_1306_7 - ltmp_1324_7);
  ltmp_1383_7 = ltmp_1382_7 + ltmp_1324_7;
  ltmp_1384_1038 = &ltmp_1325_1038[((plzo_uint64_t )ltmp_1382_7)];
  ltmp_1385_7 = *ltmp_1312_17;
  ltmp_1386_7 = (ltmp_1385_7 - ltmp_1331_7) + ltmp_1382_7;
  *ltmp_1312_17 = ltmp_1386_7;
  ltmp_1387_7 = *ltmp_1315_17;
  ltmp_1388_7 = (((ltmp_1386_7 < ltmp_1387_7)) ? (ltmp_1386_7) : ((ltmp_1387_7 + 4294967295u)));
  ltmp_1389_1031 = *ltmp_1326_1030;
  ltmp_1390_7 = *ltmp_1328_17;
  ltmp_1391_7 = *ltmp_1340_17;
  *(&ltmp_1389_1031[((plzo_int64_t )((ltmp_1390_7*(ltmp_1383_7 - ltmp_1306_7)) + ltmp_1388_7))].field2) = ltmp_1391_7;
  ltmp_1320_7_2E = ltmp_1388_7;
  ltmp_1321_7_2E = ltmp_1383_7;
  ltmp_1322_1038_2E = ltmp_1384_1038;
  goto ltmp_1398_D;

ltmp_1418_D:
  ltmp_1392_7 = ltmp_1331_7 + ltmp_1324_7;
  ltmp_1393_1038 = &ltmp_1325_1038[((plzo_uint64_t )ltmp_1331_7)];
  ltmp_1320_7_2E = 0u;
  ltmp_1321_7_2E = ltmp_1392_7;
  ltmp_1322_1038_2E = ltmp_1393_1038;
  goto ltmp_1398_D;

ltmp_1397_D:
  *ltmp_1309_17 = 0u;
  return;
}


static void ltmp_16_1009(struct l_struct_2E_E *ltmp_1420_1023, plzo_uint32_t ltmp_1421_7, plzo_uint32_t ltmp_1422_7, plzo_int32_t ltmp_1423_8) {
  struct l_struct_2E_A7 *ltmp_1424_1038;
  plzo_uint32_t *ltmp_1425_17;
  plzo_uint32_t ltmp_1426_7;
  struct l_struct_2E_A7 *ltmp_1427_1038;
  plzo_uint32_t *ltmp_1428_17;
  plzo_uint32_t ltmp_1429_7;
  plzo_uint32_t ltmp_1430_7;
  plzo_uint32_t *ltmp_1431_17;
  plzo_uint32_t *ltmp_1432_17;
  plzo_uint32_t *ltmp_1433_17;
  plzo_int32_t  (***ltmp_1434_102C) ();
  unsigned char **ltmp_1435_1025;
  plzo_uint32_t ltmp_1436_7;
  plzo_uint32_t ltmp_1436_7_2E;
  struct l_struct_2E_A7 *ltmp_1437_1038;
  struct l_struct_2E_A7 *ltmp_1437_1038_2E;
  plzo_uint32_t ltmp_1438_7;
  plzo_uint32_t ltmp_1439_7;
  plzo_uint32_t ltmp_1440_7;
  plzo_uint32_t ltmp_1441_7;
  plzo_uint32_t ltmp_1442_7;
  plzo_uint32_t ltmp_1443_7;
  plzo_uint32_t ltmp_1443_7_2E;
  plzo_uint32_t ltmp_1444_7;
  plzo_uint32_t ltmp_1445_7;
  plzo_uint32_t ltmp_1445_7_2E;
  plzo_uint32_t ltmp_1446_7;
  plzo_uint32_t ltmp_1447_7;
  plzo_uint32_t ltmp_1448_7;
  struct l_struct_2E_A7 *ltmp_1449_1038;
  plzo_uint32_t ltmp_1450_7;
  plzo_uint32_t ltmp_1451_7;
  plzo_int32_t  (**ltmp_1452_1029) ();
  plzo_int32_t  (*ltmp_1453_102A) ();
  unsigned char *ltmp_1454_13;
  plzo_int32_t  (**ltmp_1455_1029) ();
  plzo_int32_t  (*ltmp_1456_102A) ();
  plzo_uint32_t ltmp_1457_7;
  struct l_struct_2E_A7 *ltmp_1458_1038;

  ltmp_1424_1038 = *(&ltmp_1420_1023->field19.field0);
  ltmp_1425_17 = &ltmp_1420_1023->field14;
  ltmp_1426_7 = *ltmp_1425_17;
  ltmp_1427_1038 = &ltmp_1424_1038[((plzo_int64_t )ltmp_1426_7)];
  ltmp_1428_17 = &ltmp_1420_1023->field13;
  ltmp_1429_7 = *ltmp_1428_17;
  *ltmp_1428_17 = (ltmp_1426_7 + ltmp_1429_7);
  ltmp_1430_7 = ltmp_1426_7 + ltmp_1421_7;
  if ((ltmp_1430_7 < ltmp_1422_7)) {
    goto ltmp_1459_D;
  } else {
    goto ltmp_1460_D;
  }

ltmp_1459_D:
  ltmp_1431_17 = &ltmp_1420_1023->field7.field3;
  ltmp_1432_17 = &ltmp_1420_1023->field5;
  ltmp_1433_17 = &ltmp_1420_1023->field12;
  ltmp_1434_102C = &ltmp_1420_1023->field0;
  ltmp_1435_1025 = &ltmp_1420_1023->field8;
  ltmp_1436_7_2E = ltmp_1430_7;
  ltmp_1437_1038_2E = ltmp_1427_1038;
  goto ltmp_1461_D;

  do {
ltmp_1461_D:
  ltmp_1436_7 = ltmp_1436_7_2E;
  ltmp_1437_1038 = ltmp_1437_1038_2E;
  ltmp_1438_7 = *(&ltmp_1437_1038->field0);
  ltmp_1439_7 = *ltmp_1431_17;
  if ((ltmp_1439_7 > ltmp_1438_7)) {
    goto ltmp_1462_D;
  } else {
    goto ltmp_1463_D;
  }

ltmp_1464_D:
  ltmp_1447_7 = *ltmp_1428_17;
  *ltmp_1428_17 = (ltmp_1447_7 + 1u);
  ltmp_1448_7 = ltmp_1436_7 + 1u;
  ltmp_1449_1038 = &ltmp_1437_1038[PLZO_INT64_C(1)];
  if ((ltmp_1448_7 < ltmp_1422_7)) {
    ltmp_1436_7_2E = ltmp_1448_7;
    ltmp_1437_1038_2E = ltmp_1449_1038;
    goto ltmp_1461_D;
  } else {
    goto ltmp_1460_D;
  }

ltmp_1465_D:
  ltmp_1443_7 = ltmp_1443_7_2E;
  ltmp_1444_7 = *(&ltmp_1420_1023->field4[((plzo_int64_t )ltmp_1438_7)]);
  if ((ltmp_1444_7 > ltmp_1443_7)) {
    ltmp_1445_7_2E = ltmp_1443_7;
    goto ltmp_1466_D;
  } else {
    goto ltmp_1464_D;
  }

ltmp_1463_D:
  ltmp_1440_7 = *(&ltmp_1437_1038->field1[PLZO_INT64_C(0)]);
  ltmp_1441_7 = *ltmp_1432_17;
  if ((ltmp_1441_7 < ltmp_1438_7)) {
    ltmp_1445_7_2E = ltmp_1440_7;
    goto ltmp_1466_D;
  } else {
    ltmp_1443_7_2E = ltmp_1440_7;
    goto ltmp_1465_D;
  }

ltmp_1462_D:
  ltmp_1442_7 = *ltmp_1432_17;
  if ((ltmp_1442_7 < ltmp_1438_7)) {
    ltmp_1445_7_2E = 0u;
    goto ltmp_1466_D;
  } else {
    ltmp_1443_7_2E = 0u;
    goto ltmp_1465_D;
  }

ltmp_1467_D:
  ltmp_1446_7 = *(&ltmp_1437_1038[PLZO_INT64_C(1)].field0);
  if ((ltmp_1446_7 > ltmp_1438_7)) {
    goto ltmp_1464_D;
  } else {
    goto ltmp_1468_D;
  }

ltmp_1466_D:
  ltmp_1445_7 = ltmp_1445_7_2E;
  if ((ltmp_1423_8 == 0)) {
    goto ltmp_1468_D;
  } else {
    goto ltmp_1467_D;
  }

ltmp_1468_D:
  ltmp_1450_7 = *ltmp_1428_17;
  ltmp_1451_7 = ltmp_1436_7 - ltmp_1450_7;
  *ltmp_1433_17 = ltmp_1451_7;
  ltmp_1452_1029 = *ltmp_1434_102C;
  ltmp_1453_102A = *(&ltmp_1452_1029[PLZO_INT64_C(3)]);
  ltmp_1454_13 = *ltmp_1435_1025;
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint32_t , plzo_uint32_t ))ltmp_1453_102A)(ltmp_1420_1023, (&ltmp_1454_13[((plzo_uint64_t )ltmp_1451_7)]), ltmp_1450_7, ltmp_1438_7);
  *ltmp_1428_17 = 0u;
  *ltmp_1433_17 = ltmp_1436_7;
  ltmp_1455_1029 = *ltmp_1434_102C;
  ltmp_1456_102A = *(&ltmp_1455_1029[PLZO_INT64_C(4)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_1456_102A)(ltmp_1420_1023, ltmp_1438_7, ltmp_1445_7);
  ltmp_1457_7 = ltmp_1438_7 + ltmp_1436_7;
  ltmp_1458_1038 = &ltmp_1437_1038[((plzo_uint64_t )ltmp_1438_7)];
  if ((ltmp_1457_7 < ltmp_1422_7)) {
    ltmp_1436_7_2E = ltmp_1457_7;
    ltmp_1437_1038_2E = ltmp_1458_1038;
    goto ltmp_1461_D;
  } else {
    goto ltmp_1460_D;
  }

  } while (1);
ltmp_1460_D:
  *ltmp_1425_17 = 0u;
  return;
}


static plzo_uint32_t ltmp_17_1008(struct l_struct_2E_E *ltmp_1469_1023, plzo_uint32_t ltmp_1470_7, plzo_uint32_t ltmp_1471_7) {
  struct l_struct_2E_A7 *ltmp_1472_1038;
  plzo_uint32_t ltmp_1473_7;
  plzo_uint32_t ltmp_1473_7_2E;
  plzo_uint32_t ltmp_1474_7;
  plzo_uint32_t ltmp_1474_7_2E;
  plzo_uint32_t ltmp_1475_7;
  plzo_uint32_t ltmp_1475_7_2E;
  struct l_struct_2E_A7 *ltmp_1476_1038;
  struct l_struct_2E_A7 *ltmp_1476_1038_2E;
  plzo_uint32_t ltmp_1477_7;
  plzo_int32_t  (***ltmp_1478_102C) ();
  plzo_uint32_t ltmp_1479_7;
  plzo_uint32_t ltmp_1479_7_2E;
  plzo_uint32_t ltmp_1480_7;
  plzo_uint32_t ltmp_1480_7_2E;
  plzo_uint32_t ltmp_1481_7;
  plzo_uint32_t ltmp_1481_7_2E;
  plzo_uint32_t ltmp_1482_7;
  plzo_uint32_t ltmp_1482_7_2E;
  struct l_struct_2E_A7 *ltmp_1483_1038;
  struct l_struct_2E_A7 *ltmp_1483_1038_2E;
  plzo_uint32_t ltmp_1484_7;
  plzo_uint32_t ltmp_1485_7;
  plzo_uint32_t ltmp_1486_7;
  plzo_int32_t  (**ltmp_1487_1029) ();
  plzo_int32_t  (*ltmp_1488_102A) ();
  plzo_int32_t ltmp_1489_8;
  plzo_int32_t  (**ltmp_1490_1029) ();
  plzo_int32_t  (*ltmp_1491_102A) ();
  plzo_int32_t ltmp_1492_8;
  plzo_uint32_t ltmp_1493_7;
  struct l_struct_2E_A7 *ltmp_1494_1038;
  plzo_uint32_t ltmp_1495_7;
  plzo_uint32_t ltmp_1495_7_2E;
  plzo_uint32_t ltmp_1496_7;
  plzo_uint32_t ltmp_1497_7;
  plzo_uint32_t ltmp_1498_7;
  plzo_uint32_t ltmp_1498_7_2E;
  plzo_uint32_t ltmp_1499_7;
  plzo_uint32_t ltmp_1499_7_2E;
  struct l_struct_2E_A7 *ltmp_1500_1038;
  plzo_uint32_t ltmp_1501_7;
  plzo_uint32_t ltmp_1501_7_2E;
  plzo_uint32_t ltmp_1502_7;
  plzo_uint32_t ltmp_1502_7_2E;

  ltmp_1472_1038 = *(&ltmp_1469_1023->field19.field0);
  if ((ltmp_1470_7 < ltmp_1471_7)) {
    goto ltmp_1503_D;
  } else {
    ltmp_1501_7_2E = ltmp_1470_7;
    ltmp_1502_7_2E = ltmp_1470_7;
    goto ltmp_1504_D;
  }

ltmp_1503_D:
  ltmp_1478_102C = &ltmp_1469_1023->field0;
  ltmp_1479_7_2E = 0u;
  ltmp_1480_7_2E = ltmp_1470_7;
  ltmp_1481_7_2E = ltmp_1470_7;
  ltmp_1482_7_2E = ltmp_1470_7;
  ltmp_1483_1038_2E = ltmp_1472_1038;
  goto ltmp_1505_D;

  do {
ltmp_1505_D:
  ltmp_1479_7 = ltmp_1479_7_2E;
  ltmp_1480_7 = ltmp_1480_7_2E;
  ltmp_1481_7 = ltmp_1481_7_2E;
  ltmp_1482_7 = ltmp_1482_7_2E;
  ltmp_1483_1038 = ltmp_1483_1038_2E;
  ltmp_1484_7 = ltmp_1479_7 + ltmp_1470_7;
  ltmp_1485_7 = *(&ltmp_1483_1038->field0);
  ltmp_1486_7 = (((ltmp_1485_7 == 0u)) ? (1u) : (ltmp_1485_7));
  if ((ltmp_1484_7 == ltmp_1480_7)) {
    goto ltmp_1506_D;
  } else {
    goto ltmp_1507_D;
  }

ltmp_1508_D:
  ltmp_1473_7 = ltmp_1473_7_2E;
  ltmp_1474_7 = ltmp_1474_7_2E;
  ltmp_1475_7 = ltmp_1475_7_2E;
  ltmp_1476_1038 = ltmp_1476_1038_2E;
  ltmp_1477_7 = ltmp_1479_7 + 1u;
  if (((ltmp_1484_7 + 1u) < ltmp_1471_7)) {
    ltmp_1479_7_2E = ltmp_1477_7;
    ltmp_1480_7_2E = ltmp_1473_7;
    ltmp_1481_7_2E = ltmp_1474_7;
    ltmp_1482_7_2E = ltmp_1475_7;
    ltmp_1483_1038_2E = ltmp_1476_1038;
    goto ltmp_1505_D;
  } else {
    ltmp_1501_7_2E = ltmp_1474_7;
    ltmp_1502_7_2E = ltmp_1475_7;
    goto ltmp_1504_D;
  }

ltmp_1509_D:
  ltmp_1493_7 = ltmp_1486_7 + ltmp_1484_7;
  ltmp_1494_1038 = &ltmp_1483_1038[PLZO_INT64_C(1)];
  ltmp_1473_7_2E = ltmp_1493_7;
  ltmp_1474_7_2E = ltmp_1484_7;
  ltmp_1475_7_2E = ltmp_1482_7;
  ltmp_1476_1038_2E = ltmp_1494_1038;
  goto ltmp_1508_D;

ltmp_1510_D:
  ltmp_1490_1029 = *ltmp_1478_102C;
  ltmp_1491_102A = *(&ltmp_1490_1029[PLZO_INT64_C(21)]);
  ltmp_1492_8 = ((plzo_int32_t  (*) (struct l_struct_2E_E *, struct l_struct_2E_A7 *))ltmp_1491_102A)(ltmp_1469_1023, ltmp_1483_1038);
  if ((ltmp_1492_8 == 0)) {
    ltmp_1495_7_2E = ltmp_1482_7;
    goto ltmp_1511_D;
  } else {
    goto ltmp_1509_D;
  }

ltmp_1506_D:
  ltmp_1487_1029 = *ltmp_1478_102C;
  ltmp_1488_102A = *(&ltmp_1487_1029[PLZO_INT64_C(20)]);
  ltmp_1489_8 = ((plzo_int32_t  (*) (struct l_struct_2E_E *, struct l_struct_2E_A7 *))ltmp_1488_102A)(ltmp_1469_1023, ltmp_1483_1038);
  if ((ltmp_1489_8 == 0)) {
    goto ltmp_1510_D;
  } else {
    ltmp_1495_7_2E = ltmp_1484_7;
    goto ltmp_1511_D;
  }

ltmp_1512_D:
  ltmp_1498_7 = ltmp_1498_7_2E;
  ltmp_1499_7 = ltmp_1499_7_2E;
  ltmp_1500_1038 = &ltmp_1483_1038[PLZO_INT64_C(1)];
  ltmp_1473_7_2E = ltmp_1498_7;
  ltmp_1474_7_2E = ltmp_1481_7;
  ltmp_1475_7_2E = ltmp_1499_7;
  ltmp_1476_1038_2E = ltmp_1500_1038;
  goto ltmp_1508_D;

ltmp_1511_D:
  ltmp_1495_7 = ltmp_1495_7_2E;
  ltmp_1496_7 = ltmp_1486_7 + ltmp_1484_7;
  ltmp_1498_7_2E = ltmp_1496_7;
  ltmp_1499_7_2E = ltmp_1495_7;
  goto ltmp_1512_D;

ltmp_1507_D:
  ltmp_1497_7 = ltmp_1486_7 + ltmp_1484_7;
  if ((ltmp_1497_7 > ltmp_1480_7)) {
    goto ltmp_1513_D;
  } else {
    ltmp_1498_7_2E = ltmp_1480_7;
    ltmp_1499_7_2E = ltmp_1482_7;
    goto ltmp_1512_D;
  }

ltmp_1513_D:
  ltmp_1498_7_2E = ltmp_1497_7;
  ltmp_1499_7_2E = ltmp_1482_7;
  goto ltmp_1512_D;

  } while (1);
ltmp_1504_D:
  ltmp_1501_7 = ltmp_1501_7_2E;
  ltmp_1502_7 = ltmp_1502_7_2E;
  if ((ltmp_1502_7 == ltmp_1470_7)) {
    goto ltmp_1514_D;
  } else {
    goto ltmp_1515_D;
  }

ltmp_1514_D:
  return ((((ltmp_1501_7 == ltmp_1470_7)) ? (ltmp_1471_7) : (ltmp_1501_7)));
ltmp_1515_D:
  return ltmp_1502_7;
}


static struct l_struct_2E_M *ltmp_18_100A(struct l_struct_2E_A6 *ltmp_1516_1036) {
  plzo_uint32_t *ltmp_1517_17;
  plzo_uint32_t ltmp_1518_7;
  plzo_uint32_t ltmp_1519_7;
  plzo_uint32_t ltmp_1520_7;
  plzo_uint32_t *ltmp_1521_17;
  plzo_uint32_t ltmp_1522_7;
  plzo_uint32_t ltmp_1523_7;
  plzo_uint32_t ltmp_1524_7;
  plzo_uint32_t ltmp_1525_7;
  plzo_uint32_t *ltmp_1526_17;
  plzo_uint32_t ltmp_1527_7;
  plzo_uint32_t ltmp_1528_7;
  struct l_struct_2E_M *ltmp_1529_1032;
  plzo_uint32_t ltmp_1530_7;

  ltmp_1517_17 = &ltmp_1516_1036->field20;
  ltmp_1518_7 = *ltmp_1517_17;
  ltmp_1519_7 = ltmp_1518_7 + 1u;
  *ltmp_1517_17 = ltmp_1519_7;
  ltmp_1520_7 = *(&ltmp_1516_1036->field19);
  ltmp_1521_17 = &ltmp_1516_1036->field21;
  if ((ltmp_1519_7 == ltmp_1520_7)) {
    goto ltmp_1531_D;
  } else {
    goto ltmp_1532_D;
  }

ltmp_1531_D:
  *ltmp_1517_17 = 0u;
  ltmp_1522_7 = *ltmp_1521_17;
  if ((ltmp_1522_7 == 0u)) {
    goto ltmp_1533_D;
  } else {
    goto ltmp_1534_D;
  }

ltmp_1532_D:
  ltmp_1523_7 = *ltmp_1521_17;
  if ((ltmp_1519_7 == ltmp_1523_7)) {
    goto ltmp_1533_D;
  } else {
    goto ltmp_1534_D;
  }

ltmp_1533_D:
  ltmp_1524_7 = *ltmp_1521_17;
  ltmp_1525_7 = ltmp_1524_7 + 1u;
  *ltmp_1521_17 = ltmp_1525_7;
  ltmp_1526_17 = &ltmp_1516_1036->field1;
  if ((ltmp_1525_7 == ltmp_1520_7)) {
    goto ltmp_1535_D;
  } else {
    goto ltmp_1536_D;
  }

ltmp_1535_D:
  *ltmp_1521_17 = 0u;
  ltmp_1527_7 = *ltmp_1526_17;
  *ltmp_1526_17 = (ltmp_1527_7 + 1u);
  goto ltmp_1534_D;

ltmp_1536_D:
  ltmp_1528_7 = *ltmp_1526_17;
  *ltmp_1526_17 = (ltmp_1528_7 + 1u);
  goto ltmp_1534_D;

ltmp_1534_D:
  ltmp_1529_1032 = *(&ltmp_1516_1036->field18);
  ltmp_1530_7 = *ltmp_1517_17;
  return (&ltmp_1529_1032[((plzo_int64_t )ltmp_1530_7)]);
}


static void ltmp_19_100B(struct l_struct_2E_A8 *ltmp_1537_1034) {
  struct l_struct_2E_A6 *ltmp_1538_1036;
  struct l_struct_2E_A9 **ltmp_1539_1035;
  struct l_struct_2E_A9 *ltmp_1540_102F;
  void  (*ltmp_1541_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_M *ltmp_1542_1032;
  struct l_struct_2E_A9 *ltmp_1543_102F;
  void  (*ltmp_1544_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A7 *ltmp_1545_1038;
  struct l_struct_2E_A9 *ltmp_1546_102F;
  void  (*ltmp_1547_1019) (struct plzo_callback_t *, signed char *);
  unsigned char *ltmp_1548_13;
  plzo_uint64_t ltmp_1549_9;
  plzo_uint64_t ltmp_1549_9_2E;
  plzo_uint64_t ltmp_1550_9;
  unsigned char *ltmp_1551_13;
  plzo_uint64_t ltmp_1552_9;
  plzo_uint64_t ltmp_1552_9_2E;
  plzo_uint64_t ltmp_1553_9;

  if ((ltmp_1537_1034 == ((struct l_struct_2E_A8 *)0))) {
    goto ltmp_1554_D;
  } else {
    goto ltmp_1555_D;
  }

ltmp_1555_D:
  ltmp_1538_1036 = *(&ltmp_1537_1034->field7);
  if ((ltmp_1538_1036 == ((struct l_struct_2E_A6 *)0))) {
    goto ltmp_1556_D;
  } else {
    goto ltmp_1557_D;
  }

ltmp_1557_D:
  ltmp_1539_1035 = &ltmp_1537_1034->field6;
  ltmp_1540_102F = *ltmp_1539_1035;
  ltmp_1541_1019 = *(&ltmp_1540_102F->field2.field1);
  ltmp_1542_1032 = *(&ltmp_1538_1036->field18);
  ltmp_1541_1019((&ltmp_1540_102F->field2), ((signed char *)ltmp_1542_1032));
  ltmp_1543_102F = *ltmp_1539_1035;
  ltmp_1544_1019 = *(&ltmp_1543_102F->field2.field1);
  ltmp_1545_1038 = *(&ltmp_1538_1036->field17);
  ltmp_1544_1019((&ltmp_1543_102F->field2), ((signed char *)ltmp_1545_1038));
  ltmp_1546_102F = *ltmp_1539_1035;
  ltmp_1547_1019 = *(&ltmp_1546_102F->field2.field1);
  ltmp_1547_1019((&ltmp_1546_102F->field2), ((signed char *)ltmp_1538_1036));
  ltmp_1548_13 = (unsigned char *)ltmp_1537_1034;
  ltmp_1549_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1558_D;

  do {
ltmp_1558_D:
  ltmp_1549_9 = ltmp_1549_9_2E;
  *(&ltmp_1548_13[ltmp_1549_9]) = 0;
  ltmp_1550_9 = ltmp_1549_9 + PLZO_UINT64_C(1);
  if ((ltmp_1550_9 == PLZO_UINT64_C(56))) {
    goto ltmp_1559_D;
  } else {
    ltmp_1549_9_2E = ltmp_1550_9;
    goto ltmp_1558_D;
  }

  } while (1);
ltmp_1559_D:
  return;
ltmp_1556_D:
  ltmp_1551_13 = (unsigned char *)ltmp_1537_1034;
  ltmp_1552_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1560_D;

  do {
ltmp_1560_D:
  ltmp_1552_9 = ltmp_1552_9_2E;
  *(&ltmp_1551_13[ltmp_1552_9]) = 0;
  ltmp_1553_9 = ltmp_1552_9 + PLZO_UINT64_C(1);
  if ((ltmp_1553_9 == PLZO_UINT64_C(56))) {
    goto ltmp_1561_D;
  } else {
    ltmp_1552_9_2E = ltmp_1553_9;
    goto ltmp_1560_D;
  }

  } while (1);
ltmp_1561_D:
  return;
ltmp_1554_D:
  return;
}


static void ltmp_20_100C(struct l_struct_2E_A5 *ltmp_1562_1024) {
  unsigned char *ltmp_1563_13;
  plzo_uint32_t ltmp_1564_7;
  plzo_uint32_t ltmp_1565_7;
  plzo_uint32_t *ltmp_1566_17;
  plzo_uint32_t ltmp_1567_7;
  plzo_uint32_t ltmp_1568_7;
  plzo_uint32_t ltmp_1569_7;
  plzo_uint32_t ltmp_1570_7;
  plzo_uint32_t *ltmp_1571_17;
  plzo_uint32_t ltmp_1572_7;
  unsigned char *ltmp_1573_13;
  plzo_uint32_t ltmp_1574_7;
  unsigned char *ltmp_1575_13;
  unsigned char *ltmp_1575_13_2E;
  unsigned char *ltmp_1576_13;
  plzo_uint32_t ltmp_1577_7;
  unsigned char *ltmp_1578_13;
  unsigned char *ltmp_1578_13_2E;
  plzo_uint32_t ltmp_1579_7;
  unsigned char *ltmp_1580_13;
  unsigned char *ltmp_1580_13_2E;
  unsigned char *ltmp_1581_13;
  unsigned char *ltmp_1581_13_2E;

  ltmp_1563_13 = *(&ltmp_1562_1024->field5);
  ltmp_1564_7 = *(&ltmp_1562_1024->field0);
  ltmp_1565_7 = *(&ltmp_1562_1024->field2);
  if ((ltmp_1565_7 == 0u)) {
    goto ltmp_1582_D;
  } else {
    goto ltmp_1583_D;
  }

ltmp_1582_D:
  *ltmp_1563_13 = ((unsigned char )ltmp_1564_7);
  ltmp_1566_17 = &ltmp_1562_1024->field3;
  ltmp_1567_7 = *ltmp_1566_17;
  if ((ltmp_1567_7 > 15u)) {
    goto ltmp_1584_D;
  } else {
    goto ltmp_1585_D;
  }

ltmp_1584_D:
  *(&ltmp_1563_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1564_7 >> 8));
  ltmp_1568_7 = *ltmp_1566_17;
  if ((ltmp_1568_7 > 23u)) {
    goto ltmp_1586_D;
  } else {
    goto ltmp_1587_D;
  }

ltmp_1585_D:
  if ((ltmp_1567_7 > 23u)) {
    goto ltmp_1586_D;
  } else {
    goto ltmp_1587_D;
  }

ltmp_1586_D:
  *(&ltmp_1563_13[PLZO_INT64_C(2)]) = ((unsigned char )(ltmp_1564_7 >> 16));
  ltmp_1569_7 = *ltmp_1566_17;
  if ((ltmp_1569_7 > 31u)) {
    goto ltmp_1588_D;
  } else {
    goto ltmp_1589_D;
  }

ltmp_1587_D:
  ltmp_1570_7 = *ltmp_1566_17;
  if ((ltmp_1570_7 > 31u)) {
    goto ltmp_1588_D;
  } else {
    goto ltmp_1589_D;
  }

ltmp_1588_D:
  *(&ltmp_1563_13[PLZO_INT64_C(3)]) = ((unsigned char )(ltmp_1564_7 >> 24));
  return;
ltmp_1583_D:
  ltmp_1571_17 = &ltmp_1562_1024->field3;
  ltmp_1572_7 = *ltmp_1571_17;
  if ((ltmp_1572_7 > 31u)) {
    goto ltmp_1590_D;
  } else {
    goto ltmp_1591_D;
  }

ltmp_1590_D:
  ltmp_1573_13 = &ltmp_1563_13[PLZO_INT64_C(1)];
  *ltmp_1563_13 = ((unsigned char )(ltmp_1564_7 >> 24));
  ltmp_1574_7 = *ltmp_1571_17;
  if ((ltmp_1574_7 > 23u)) {
    ltmp_1575_13_2E = ltmp_1573_13;
    goto ltmp_1592_D;
  } else {
    ltmp_1578_13_2E = ltmp_1573_13;
    goto ltmp_1593_D;
  }

ltmp_1591_D:
  if ((ltmp_1572_7 > 23u)) {
    ltmp_1575_13_2E = ltmp_1563_13;
    goto ltmp_1592_D;
  } else {
    ltmp_1578_13_2E = ltmp_1563_13;
    goto ltmp_1593_D;
  }

ltmp_1592_D:
  ltmp_1575_13 = ltmp_1575_13_2E;
  ltmp_1576_13 = &ltmp_1575_13[PLZO_INT64_C(1)];
  *ltmp_1575_13 = ((unsigned char )(ltmp_1564_7 >> 16));
  ltmp_1577_7 = *ltmp_1571_17;
  if ((ltmp_1577_7 > 15u)) {
    ltmp_1580_13_2E = ltmp_1576_13;
    goto ltmp_1594_D;
  } else {
    ltmp_1581_13_2E = ltmp_1576_13;
    goto ltmp_1595_D;
  }

ltmp_1593_D:
  ltmp_1578_13 = ltmp_1578_13_2E;
  ltmp_1579_7 = *ltmp_1571_17;
  if ((ltmp_1579_7 > 15u)) {
    ltmp_1580_13_2E = ltmp_1578_13;
    goto ltmp_1594_D;
  } else {
    ltmp_1581_13_2E = ltmp_1578_13;
    goto ltmp_1595_D;
  }

ltmp_1594_D:
  ltmp_1580_13 = ltmp_1580_13_2E;
  *ltmp_1580_13 = ((unsigned char )(ltmp_1564_7 >> 8));
  *(&ltmp_1580_13[PLZO_INT64_C(1)]) = ((unsigned char )ltmp_1564_7);
  return;
ltmp_1595_D:
  ltmp_1581_13 = ltmp_1581_13_2E;
  *ltmp_1581_13 = ((unsigned char )ltmp_1564_7);
  return;
ltmp_1589_D:
  return;
}


static void ltmp_21_100D(struct l_struct_2E_Lzo1xE *ltmp_1596_103C, plzo_uint32_t ltmp_1597_7, plzo_uint32_t ltmp_1598_7) {
  unsigned char **ltmp_1599_1025;
  unsigned char *ltmp_1600_13;
  plzo_uint32_t ltmp_1601_7;
  plzo_uint32_t ltmp_1602_7;
  unsigned char *ltmp_1603_13;
  plzo_uint32_t ltmp_1604_7;
  plzo_uint32_t ltmp_1605_7;
  plzo_uint32_t ltmp_1606_7;
  plzo_int32_t ltmp_1607_8;
  unsigned char *ltmp_1608_13;
  unsigned char ltmp_1609_3;
  unsigned char *ltmp_1610_13;
  unsigned char *ltmp_1611_13;
  plzo_uint32_t ltmp_1612_7;
  plzo_uint32_t ltmp_1613_7;
  plzo_uint32_t ltmp_1614_7;
  plzo_uint32_t ltmp_1615_7;
  plzo_uint32_t ltmp_1616_7;
  plzo_uint32_t ltmp_1617_7;
  unsigned char *ltmp_1618_13;
  plzo_uint32_t ltmp_1619_7;
  plzo_uint32_t ltmp_1620_7;
  plzo_uint32_t ltmp_1621_7;
  unsigned char *ltmp_1622_13;
  plzo_uint32_t ltmp_1623_7;
  plzo_uint32_t ltmp_1624_7;
  unsigned char *ltmp_1625_13;
  plzo_uint32_t ltmp_1626_7;
  plzo_uint64_t ltmp_1627_9;
  plzo_uint64_t ltmp_1627_9_2E;
  plzo_uint64_t ltmp_1628_9;
  plzo_uint32_t ltmp_1629_7;
  unsigned char *ltmp_1630_13;
  unsigned char *ltmp_1631_13;
  unsigned char *ltmp_1631_13_2E;
  plzo_uint32_t ltmp_1632_7;
  plzo_uint32_t ltmp_1632_7_2E;
  unsigned char *ltmp_1633_13;
  unsigned char *ltmp_1634_13;
  unsigned char *ltmp_1634_13_2E;
  unsigned char *ltmp_1635_13;
  plzo_uint32_t ltmp_1636_7;
  plzo_uint32_t ltmp_1637_7;
  plzo_uint32_t ltmp_1638_7;
  unsigned char *ltmp_1639_13;
  plzo_uint32_t ltmp_1640_7;
  plzo_uint32_t ltmp_1641_7;
  unsigned char *ltmp_1642_13;
  plzo_uint32_t ltmp_1643_7;
  plzo_uint64_t ltmp_1644_9;
  plzo_uint64_t ltmp_1644_9_2E;
  plzo_uint64_t ltmp_1645_9;
  plzo_uint32_t ltmp_1646_7;
  unsigned char *ltmp_1647_13;
  unsigned char *ltmp_1648_13;
  unsigned char *ltmp_1648_13_2E;
  plzo_uint32_t ltmp_1649_7;
  plzo_uint32_t ltmp_1649_7_2E;
  unsigned char *ltmp_1650_13;
  unsigned char *ltmp_1651_13;
  unsigned char *ltmp_1651_13_2E;
  unsigned char *ltmp_1652_13;
  unsigned char *ltmp_1653_13;
  unsigned char *ltmp_1653_13_2E;

  ltmp_0_1000((&ltmp_1596_103C->field0.field0), ltmp_1597_7, ltmp_1598_7);
  ltmp_1599_1025 = &ltmp_1596_103C->field0.field0.field1.field6;
  ltmp_1600_13 = *ltmp_1599_1025;
  if ((ltmp_1597_7 == 2u)) {
    goto ltmp_1654_D;
  } else {
    goto ltmp_1655_D;
  }

ltmp_1654_D:
  ltmp_1601_7 = ltmp_1598_7 + 4294967295u;
  ltmp_1602_7 = *(&ltmp_1596_103C->field0.field1);
  *ltmp_1600_13 = (((unsigned char )(((unsigned char )ltmp_1602_7) | (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1601_7) << 2))) & 12))))));
  ltmp_1603_13 = &ltmp_1600_13[PLZO_INT64_C(2)];
  *(&ltmp_1600_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1601_7 >> 2));
  ltmp_1653_13_2E = ltmp_1603_13;
  goto ltmp_1656_D;

ltmp_1655_D:
  ltmp_1604_7 = *(&ltmp_1596_103C->field0.field18);
  if ((ltmp_1604_7 < ltmp_1597_7)) {
    goto ltmp_1657_D;
  } else {
    goto ltmp_1658_D;
  }

ltmp_1658_D:
  ltmp_1605_7 = *(&ltmp_1596_103C->field0.field7);
  if ((ltmp_1605_7 < ltmp_1598_7)) {
    goto ltmp_1657_D;
  } else {
    goto ltmp_1659_D;
  }

ltmp_1659_D:
  ltmp_1606_7 = ltmp_1598_7 + 4294967295u;
  ltmp_1607_8 = *(&ltmp_1596_103C->field0.field0.field6.field0);
  ltmp_1608_13 = &ltmp_1600_13[PLZO_INT64_C(1)];
  ltmp_1609_3 = (unsigned char )ltmp_1597_7;
  if ((ltmp_1607_8 == 0)) {
    goto ltmp_1660_D;
  } else {
    goto ltmp_1661_D;
  }

ltmp_1660_D:
  *ltmp_1600_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1606_7) << 2))) & 28))) | (((unsigned char )((((unsigned char )(ltmp_1609_3 << 5))) + 224))))));
  ltmp_1610_13 = &ltmp_1600_13[PLZO_INT64_C(2)];
  *ltmp_1608_13 = ((unsigned char )(ltmp_1606_7 >> 3));
  ltmp_1653_13_2E = ltmp_1610_13;
  goto ltmp_1656_D;

ltmp_1661_D:
  *ltmp_1600_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1606_7) << 2))) & 12))) | (((unsigned char )((((unsigned char )(ltmp_1609_3 << 4))) + 16))))));
  ltmp_1611_13 = &ltmp_1600_13[PLZO_INT64_C(2)];
  *ltmp_1608_13 = ((unsigned char )(ltmp_1606_7 >> 2));
  ltmp_1653_13_2E = ltmp_1611_13;
  goto ltmp_1656_D;

ltmp_1657_D:
  ltmp_1612_7 = *(&ltmp_1596_103C->field0.field13);
  if ((ltmp_1612_7 == ltmp_1597_7)) {
    goto ltmp_1662_D;
  } else {
    goto ltmp_1663_D;
  }

ltmp_1662_D:
  ltmp_1613_7 = *(&ltmp_1596_103C->field0.field11);
  if ((ltmp_1613_7 < ltmp_1598_7)) {
    goto ltmp_1663_D;
  } else {
    goto ltmp_1664_D;
  }

ltmp_1664_D:
  ltmp_1614_7 = *(&ltmp_1596_103C->field0.field0.field17);
  if ((ltmp_1614_7 > 3u)) {
    goto ltmp_1665_D;
  } else {
    goto ltmp_1663_D;
  }

ltmp_1665_D:
  ltmp_1615_7 = *(&ltmp_1596_103C->field0.field7);
  ltmp_1616_7 = (ltmp_1598_7 - ltmp_1615_7) + 4294967295u;
  ltmp_1617_7 = *(&ltmp_1596_103C->field0.field1);
  *ltmp_1600_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1616_7) << 2))) & 12))) | ((unsigned char )ltmp_1617_7))));
  ltmp_1618_13 = &ltmp_1600_13[PLZO_INT64_C(2)];
  *(&ltmp_1600_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1616_7 >> 2));
  ltmp_1653_13_2E = ltmp_1618_13;
  goto ltmp_1656_D;

ltmp_1663_D:
  ltmp_1619_7 = *(&ltmp_1596_103C->field0.field8);
  if ((ltmp_1619_7 < ltmp_1598_7)) {
    goto ltmp_1666_D;
  } else {
    goto ltmp_1667_D;
  }

ltmp_1667_D:
  ltmp_1620_7 = ltmp_1598_7 + 4294967295u;
  ltmp_1621_7 = *(&ltmp_1596_103C->field0.field19);
  if ((ltmp_1621_7 < ltmp_1597_7)) {
    goto ltmp_1668_D;
  } else {
    goto ltmp_1669_D;
  }

ltmp_1669_D:
  ltmp_1622_13 = &ltmp_1600_13[PLZO_INT64_C(1)];
  ltmp_1623_7 = *(&ltmp_1596_103C->field0.field3);
  *ltmp_1600_13 = (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1597_7) + 254))) + ((unsigned char )ltmp_1623_7))));
  ltmp_1634_13_2E = ltmp_1622_13;
  goto ltmp_1670_D;

ltmp_1668_D:
  ltmp_1624_7 = ltmp_1597_7 - ltmp_1621_7;
  ltmp_1625_13 = &ltmp_1600_13[PLZO_INT64_C(1)];
  ltmp_1626_7 = *(&ltmp_1596_103C->field0.field3);
  *ltmp_1600_13 = ((unsigned char )ltmp_1626_7);
  if ((ltmp_1624_7 > 255u)) {
    ltmp_1627_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1671_D;
  } else {
    ltmp_1631_13_2E = ltmp_1625_13;
    ltmp_1632_7_2E = ltmp_1624_7;
    goto ltmp_1672_D;
  }

  do {
ltmp_1671_D:
  ltmp_1627_9 = ltmp_1627_9_2E;
  ltmp_1628_9 = ltmp_1627_9 + PLZO_UINT64_C(1);
  ltmp_1629_7 = ((((plzo_uint32_t )ltmp_1627_9)*4294967041u) + ltmp_1624_7) + 4294967041u;
  *(&ltmp_1600_13[ltmp_1628_9]) = 0;
  if ((ltmp_1629_7 > 255u)) {
    ltmp_1627_9_2E = ltmp_1628_9;
    goto ltmp_1671_D;
  } else {
    goto ltmp_1673_D;
  }

  } while (1);
ltmp_1673_D:
  ltmp_1630_13 = &ltmp_1600_13[(((plzo_int64_t )ltmp_1627_9) + PLZO_INT64_C(2))];
  ltmp_1631_13_2E = ltmp_1630_13;
  ltmp_1632_7_2E = ltmp_1629_7;
  goto ltmp_1672_D;

ltmp_1672_D:
  ltmp_1631_13 = ltmp_1631_13_2E;
  ltmp_1632_7 = ltmp_1632_7_2E;
  ltmp_1633_13 = &ltmp_1631_13[PLZO_INT64_C(1)];
  *ltmp_1631_13 = ((unsigned char )ltmp_1632_7);
  ltmp_1634_13_2E = ltmp_1633_13;
  goto ltmp_1670_D;

ltmp_1670_D:
  ltmp_1634_13 = ltmp_1634_13_2E;
  *ltmp_1634_13 = (((unsigned char )(((unsigned char )ltmp_1620_7) << 2)));
  ltmp_1635_13 = &ltmp_1634_13[PLZO_INT64_C(2)];
  *(&ltmp_1634_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1620_7 >> 6));
  ltmp_1653_13_2E = ltmp_1635_13;
  goto ltmp_1656_D;

ltmp_1666_D:
  ltmp_1636_7 = ltmp_1598_7 - ltmp_1619_7;
  ltmp_1637_7 = (ltmp_1636_7 >> 11) & 8u;
  ltmp_1638_7 = *(&ltmp_1596_103C->field0.field20);
  if ((ltmp_1638_7 < ltmp_1597_7)) {
    goto ltmp_1674_D;
  } else {
    goto ltmp_1675_D;
  }

ltmp_1675_D:
  ltmp_1639_13 = &ltmp_1600_13[PLZO_INT64_C(1)];
  ltmp_1640_7 = *(&ltmp_1596_103C->field0.field4);
  *ltmp_1600_13 = (((unsigned char )(((unsigned char )ltmp_1640_7) + (((unsigned char )(((unsigned char )ltmp_1637_7) | (((unsigned char )(((unsigned char )ltmp_1597_7) + 254)))))))));
  ltmp_1651_13_2E = ltmp_1639_13;
  goto ltmp_1676_D;

ltmp_1674_D:
  ltmp_1641_7 = ltmp_1597_7 - ltmp_1638_7;
  ltmp_1642_13 = &ltmp_1600_13[PLZO_INT64_C(1)];
  ltmp_1643_7 = *(&ltmp_1596_103C->field0.field4);
  *ltmp_1600_13 = (((unsigned char )(((unsigned char )ltmp_1643_7) + ((unsigned char )ltmp_1637_7))));
  if ((ltmp_1641_7 > 255u)) {
    ltmp_1644_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1677_D;
  } else {
    ltmp_1648_13_2E = ltmp_1642_13;
    ltmp_1649_7_2E = ltmp_1641_7;
    goto ltmp_1678_D;
  }

  do {
ltmp_1677_D:
  ltmp_1644_9 = ltmp_1644_9_2E;
  ltmp_1645_9 = ltmp_1644_9 + PLZO_UINT64_C(1);
  ltmp_1646_7 = ((((plzo_uint32_t )ltmp_1644_9)*4294967041u) + ltmp_1641_7) + 4294967041u;
  *(&ltmp_1600_13[ltmp_1645_9]) = 0;
  if ((ltmp_1646_7 > 255u)) {
    ltmp_1644_9_2E = ltmp_1645_9;
    goto ltmp_1677_D;
  } else {
    goto ltmp_1679_D;
  }

  } while (1);
ltmp_1679_D:
  ltmp_1647_13 = &ltmp_1600_13[(((plzo_int64_t )ltmp_1644_9) + PLZO_INT64_C(2))];
  ltmp_1648_13_2E = ltmp_1647_13;
  ltmp_1649_7_2E = ltmp_1646_7;
  goto ltmp_1678_D;

ltmp_1678_D:
  ltmp_1648_13 = ltmp_1648_13_2E;
  ltmp_1649_7 = ltmp_1649_7_2E;
  ltmp_1650_13 = &ltmp_1648_13[PLZO_INT64_C(1)];
  *ltmp_1648_13 = ((unsigned char )ltmp_1649_7);
  ltmp_1651_13_2E = ltmp_1650_13;
  goto ltmp_1676_D;

ltmp_1676_D:
  ltmp_1651_13 = ltmp_1651_13_2E;
  *ltmp_1651_13 = (((unsigned char )(((unsigned char )ltmp_1636_7) << 2)));
  ltmp_1652_13 = &ltmp_1651_13[PLZO_INT64_C(2)];
  *(&ltmp_1651_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1636_7 >> 6));
  ltmp_1653_13_2E = ltmp_1652_13;
  goto ltmp_1656_D;

ltmp_1656_D:
  ltmp_1653_13 = ltmp_1653_13_2E;
  *(&ltmp_1596_103C->field0.field0.field15) = ltmp_1597_7;
  *(&ltmp_1596_103C->field0.field0.field16) = ltmp_1598_7;
  *ltmp_1599_1025 = ltmp_1653_13;
  return;
}


static void ltmp_22_100E(struct l_struct_2E_Lzo1xE *ltmp_1680_103C, unsigned char *ltmp_1681_13, plzo_uint32_t ltmp_1682_7, plzo_uint32_t ltmp_1683_7) {
  plzo_uint32_t *ltmp_1684_17;
  plzo_uint32_t ltmp_1685_7;
  plzo_uint32_t *ltmp_1686_17;
  plzo_uint32_t ltmp_1687_7;
  plzo_uint32_t ltmp_1688_7;
  plzo_uint32_t *ltmp_1689_17;
  unsigned char **ltmp_1690_1025;
  unsigned char *ltmp_1691_13;
  unsigned char *ltmp_1692_13;
  unsigned char *ltmp_1693_13;
  unsigned char *ltmp_1694_13;
  unsigned char ltmp_1695_3;
  unsigned char *ltmp_1696_13;
  plzo_uint32_t ltmp_1697_7;
  unsigned char *ltmp_1698_13;
  plzo_uint64_t ltmp_1699_9;
  plzo_uint64_t ltmp_1699_9_2E;
  plzo_uint64_t ltmp_1700_9;
  plzo_uint32_t ltmp_1701_7;
  unsigned char *ltmp_1702_13;
  plzo_uint32_t ltmp_1703_7;
  plzo_uint32_t ltmp_1703_7_2E;
  unsigned char *ltmp_1704_13;
  unsigned char *ltmp_1704_13_2E;
  unsigned char *ltmp_1705_13;
  unsigned char *ltmp_1706_13;
  unsigned char *ltmp_1706_13_2E;
  plzo_uint64_t ltmp_1707_9;
  plzo_uint64_t ltmp_1708_9;
  plzo_uint64_t ltmp_1708_9_2E;
  unsigned char ltmp_1709_3;
  plzo_uint64_t ltmp_1710_9;

  if ((ltmp_1682_7 == 0u)) {
    goto ltmp_1711_D;
  } else {
    goto ltmp_1712_D;
  }

ltmp_1712_D:
  ltmp_1684_17 = &ltmp_1680_103C->field0.field0.field2.field5;
  ltmp_1685_7 = *ltmp_1684_17;
  ltmp_1686_17 = &ltmp_1680_103C->field0.field0.field2.field4;
  if ((ltmp_1685_7 < ltmp_1682_7)) {
    goto ltmp_1713_D;
  } else {
    goto ltmp_1714_D;
  }

ltmp_1713_D:
  *ltmp_1684_17 = ltmp_1682_7;
  ltmp_1687_7 = *ltmp_1686_17;
  if ((ltmp_1687_7 > ltmp_1682_7)) {
    goto ltmp_1715_D;
  } else {
    goto ltmp_1711_D;
  }

ltmp_1714_D:
  ltmp_1688_7 = *ltmp_1686_17;
  if ((ltmp_1688_7 > ltmp_1682_7)) {
    goto ltmp_1715_D;
  } else {
    goto ltmp_1711_D;
  }

ltmp_1715_D:
  *ltmp_1686_17 = ltmp_1682_7;
  goto ltmp_1711_D;

ltmp_1711_D:
  ltmp_1689_17 = &ltmp_1680_103C->field0.field0.field18;
  if ((ltmp_1682_7 == 0u)) {
    goto ltmp_1716_D;
  } else {
    goto ltmp_1717_D;
  }

ltmp_1716_D:
  *ltmp_1689_17 = 0u;
  *(&ltmp_1680_103C->field0.field0.field17) = 0u;
  return;
ltmp_1717_D:
  *ltmp_1689_17 = ltmp_1683_7;
  *(&ltmp_1680_103C->field0.field0.field17) = ltmp_1682_7;
  ltmp_1690_1025 = &ltmp_1680_103C->field0.field0.field1.field6;
  ltmp_1691_13 = *ltmp_1690_1025;
  ltmp_1692_13 = *(&ltmp_1680_103C->field0.field0.field9);
  if (((ltmp_1692_13 == ltmp_1691_13) & (ltmp_1682_7 < 239u))) {
    goto ltmp_1718_D;
  } else {
    goto ltmp_1719_D;
  }

ltmp_1718_D:
  ltmp_1693_13 = &ltmp_1691_13[PLZO_INT64_C(1)];
  *ltmp_1691_13 = (((unsigned char )(((unsigned char )ltmp_1682_7) + 17)));
  ltmp_1706_13_2E = ltmp_1693_13;
  goto ltmp_1720_D;

ltmp_1719_D:
  if ((ltmp_1682_7 > 3u)) {
    goto ltmp_1721_D;
  } else {
    goto ltmp_1722_D;
  }

ltmp_1722_D:
  ltmp_1694_13 = &ltmp_1691_13[PLZO_INT64_C(-2)];
  ltmp_1695_3 = *ltmp_1694_13;
  *ltmp_1694_13 = (((unsigned char )(ltmp_1695_3 | ((unsigned char )ltmp_1682_7))));
  ltmp_1706_13_2E = ltmp_1691_13;
  goto ltmp_1720_D;

ltmp_1721_D:
  if ((ltmp_1682_7 > 18u)) {
    goto ltmp_1723_D;
  } else {
    goto ltmp_1724_D;
  }

ltmp_1724_D:
  ltmp_1696_13 = &ltmp_1691_13[PLZO_INT64_C(1)];
  *ltmp_1691_13 = (((unsigned char )(((unsigned char )ltmp_1682_7) + 253)));
  ltmp_1706_13_2E = ltmp_1696_13;
  goto ltmp_1720_D;

ltmp_1723_D:
  ltmp_1697_7 = ltmp_1682_7 + 4294967278u;
  ltmp_1698_13 = &ltmp_1691_13[PLZO_INT64_C(1)];
  *ltmp_1691_13 = 0;
  if ((ltmp_1697_7 > 255u)) {
    ltmp_1699_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1725_D;
  } else {
    ltmp_1703_7_2E = ltmp_1697_7;
    ltmp_1704_13_2E = ltmp_1698_13;
    goto ltmp_1726_D;
  }

  do {
ltmp_1725_D:
  ltmp_1699_9 = ltmp_1699_9_2E;
  ltmp_1700_9 = ltmp_1699_9 + PLZO_UINT64_C(1);
  ltmp_1701_7 = ((((plzo_uint32_t )ltmp_1699_9)*4294967041u) + ltmp_1697_7) + 4294967041u;
  *(&ltmp_1691_13[ltmp_1700_9]) = 0;
  if ((ltmp_1701_7 > 255u)) {
    ltmp_1699_9_2E = ltmp_1700_9;
    goto ltmp_1725_D;
  } else {
    goto ltmp_1727_D;
  }

  } while (1);
ltmp_1727_D:
  ltmp_1702_13 = &ltmp_1691_13[(((plzo_int64_t )ltmp_1699_9) + PLZO_INT64_C(2))];
  ltmp_1703_7_2E = ltmp_1701_7;
  ltmp_1704_13_2E = ltmp_1702_13;
  goto ltmp_1726_D;

ltmp_1726_D:
  ltmp_1703_7 = ltmp_1703_7_2E;
  ltmp_1704_13 = ltmp_1704_13_2E;
  ltmp_1705_13 = &ltmp_1704_13[PLZO_INT64_C(1)];
  *ltmp_1704_13 = ((unsigned char )ltmp_1703_7);
  ltmp_1706_13_2E = ltmp_1705_13;
  goto ltmp_1720_D;

ltmp_1720_D:
  ltmp_1706_13 = ltmp_1706_13_2E;
  ltmp_1707_9 = (plzo_uint64_t )ltmp_1682_7;
  ltmp_1708_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1728_D;

  do {
ltmp_1728_D:
  ltmp_1708_9 = ltmp_1708_9_2E;
  ltmp_1709_3 = *(&ltmp_1681_13[ltmp_1708_9]);
  *(&ltmp_1706_13[ltmp_1708_9]) = ltmp_1709_3;
  ltmp_1710_9 = ltmp_1708_9 + PLZO_UINT64_C(1);
  if ((ltmp_1710_9 == ltmp_1707_9)) {
    goto ltmp_1729_D;
  } else {
    ltmp_1708_9_2E = ltmp_1710_9;
    goto ltmp_1728_D;
  }

  } while (1);
ltmp_1729_D:
  *ltmp_1690_1025 = (&ltmp_1706_13[(((plzo_int64_t )ltmp_1708_9) + PLZO_INT64_C(1))]);
  return;
}


static void ltmp_23_100D(struct l_struct_2E_Lzo1xE *ltmp_1730_103C, plzo_uint32_t ltmp_1731_7, plzo_uint32_t ltmp_1732_7) {
  struct l_struct_2E_A7 *ltmp_1733_1038;
  struct l_struct_2E_A7 *ltmp_1734_1038;
  struct l_struct_2E_A7 *ltmp_1734_1038_2E;
  plzo_uint32_t ltmp_1735_7;
  plzo_uint32_t ltmp_1736_7;
  plzo_uint32_t ltmp_1737_7;
  plzo_uint32_t ltmp_1737_7_2E;
  struct l_struct_2E_A7 *ltmp_1738_1038;
  struct l_struct_2E_A7 *ltmp_1738_1038_2E;
  plzo_uint32_t ltmp_1739_7;
  plzo_uint32_t *ltmp_1740_17;
  plzo_uint32_t ltmp_1741_7;
  struct l_struct_2E_A7 *ltmp_1742_1038;
  struct l_struct_2E_A7 *ltmp_1743_1038;

  ltmp_11_1000((&ltmp_1730_103C->field0.field0), ltmp_1731_7, ltmp_1732_7);
  if ((ltmp_1731_7 == 0u)) {
    goto ltmp_1744_D;
  } else {
    goto ltmp_1745_D;
  }

ltmp_1744_D:
  ltmp_1733_1038 = *(&ltmp_1730_103C->field0.field0.field19.field0);
  if (((ltmp_1731_7 < ltmp_1732_7) & (ltmp_1731_7 < 4u))) {
    ltmp_1737_7_2E = 0u;
    ltmp_1738_1038_2E = ltmp_1733_1038;
    goto ltmp_1746_D;
  } else {
    goto ltmp_1745_D;
  }

  do {
ltmp_1746_D:
  ltmp_1737_7 = ltmp_1737_7_2E;
  ltmp_1738_1038 = ltmp_1738_1038_2E;
  ltmp_1739_7 = ltmp_1737_7 + ltmp_1731_7;
  ltmp_1740_17 = &ltmp_1738_1038->field0;
  ltmp_1741_7 = *ltmp_1740_17;
  if ((ltmp_1741_7 == 2u)) {
    goto ltmp_1747_D;
  } else {
    goto ltmp_1748_D;
  }

ltmp_1749_D:
  ltmp_1734_1038 = ltmp_1734_1038_2E;
  ltmp_1735_7 = ltmp_1739_7 + 1u;
  ltmp_1736_7 = ltmp_1737_7 + 1u;
  if (((ltmp_1735_7 < ltmp_1732_7) & (ltmp_1735_7 < 4u))) {
    ltmp_1737_7_2E = ltmp_1736_7;
    ltmp_1738_1038_2E = ltmp_1734_1038;
    goto ltmp_1746_D;
  } else {
    goto ltmp_1745_D;
  }

ltmp_1747_D:
  *ltmp_1740_17 = 0u;
  ltmp_1742_1038 = &ltmp_1738_1038[PLZO_INT64_C(1)];
  ltmp_1734_1038_2E = ltmp_1742_1038;
  goto ltmp_1749_D;

ltmp_1748_D:
  ltmp_1743_1038 = &ltmp_1738_1038[PLZO_INT64_C(1)];
  ltmp_1734_1038_2E = ltmp_1743_1038;
  goto ltmp_1749_D;

  } while (1);
ltmp_1745_D:
  return;
}


static void ltmp_24_100F(struct l_struct_2E_Lzo1E *ltmp_1750_103D, plzo_uint32_t ltmp_1751_7, plzo_uint32_t ltmp_1752_7) {

  ltmp_11_1000((&ltmp_1750_103D->field0), ltmp_1751_7, ltmp_1752_7);
  return;
}


static plzo_int32_t plzo_lzo1x_99_compress_internal(unsigned char *ltmp_1753_13, plzo_uint32_t ltmp_1754_7, unsigned char *ltmp_1755_13, plzo_uint32_t *ltmp_1756_17, struct plzo_callback_t *ltmp_1757_1026, plzo_int32_t ltmp_1758_8, struct plzo_compress_config_t *ltmp_1759_1027, plzo_uint32_t *ltmp_1760_17) {
  struct l_struct_2E_Lzo1xE ltmp_1761_103C;
  struct l_struct_2E_E *ltmp_1762_1023;
  plzo_int32_t ltmp_1763_8;
  plzo_int32_t ltmp_1764_8;
  plzo_int32_t ltmp_1765_8;
  plzo_int32_t ltmp_1765_8_2E;
  plzo_int32_t ltmp_1766_8;
  plzo_int32_t ltmp_1767_8;
  plzo_int32_t ltmp_1767_8_2E;
  plzo_uint32_t ltmp_1768_7;
  plzo_uint32_t ltmp_1769_7;
  plzo_uint32_t ltmp_1770_7;
  plzo_uint32_t ltmp_1771_7;
  plzo_uint32_t ltmp_1772_7;
  plzo_uint32_t ltmp_1773_7;
  plzo_uint32_t ltmp_1774_7;
  plzo_uint32_t ltmp_1775_7;

  ltmp_25_1010((&ltmp_1761_103C.field0));
  *(&ltmp_1761_103C.field0.field0.field0) = (&(ltmp_68_1021[PLZO_INT64_C(2)]));
  *(&ltmp_1761_103C.field0.field1) = 0u;
  *(&ltmp_1761_103C.field0.field2) = 64u;
  *(&ltmp_1761_103C.field0.field3) = 32u;
  *(&ltmp_1761_103C.field0.field4) = 16u;
  *(&ltmp_1761_103C.field0.field5) = 0u;
  *(&ltmp_1761_103C.field0.field6) = 1024u;
  *(&ltmp_1761_103C.field0.field7) = 2048u;
  *(&ltmp_1761_103C.field0.field8) = 16384u;
  *(&ltmp_1761_103C.field0.field9) = 49151u;
  *(&ltmp_1761_103C.field0.field10) = 0u;
  *(&ltmp_1761_103C.field0.field11) = 3072u;
  *(&ltmp_1761_103C.field0.field12) = 2u;
  *(&ltmp_1761_103C.field0.field13) = 3u;
  *(&ltmp_1761_103C.field0.field14) = 3u;
  *(&ltmp_1761_103C.field0.field15) = 3u;
  *(&ltmp_1761_103C.field0.field16) = 0u;
  *(&ltmp_1761_103C.field0.field17) = 2u;
  *(&ltmp_1761_103C.field0.field18) = 8u;
  *(&ltmp_1761_103C.field0.field19) = 33u;
  *(&ltmp_1761_103C.field0.field20) = 9u;
  *(&ltmp_1761_103C.field0.field21) = 0u;
  *(&ltmp_1761_103C.field0.field0.field6.field0) = 0;
  *(&ltmp_1761_103C.field0.field0.field6.field3) = 49151u;
  ltmp_1762_1023 = &ltmp_1761_103C.field0.field0;
  ltmp_1763_8 = ltmp_2_1002(ltmp_1762_1023, ltmp_1758_8, ltmp_1757_1026);
  if (((ltmp_1763_8 == 0) & (ltmp_1759_1027 != ((struct plzo_compress_config_t *)0)))) {
    goto ltmp_1776_D;
  } else {
    ltmp_1765_8_2E = ltmp_1763_8;
    goto ltmp_1777_D;
  }

ltmp_1776_D:
  ltmp_1764_8 = ltmp_3_1003(ltmp_1762_1023, ltmp_1759_1027);
  ltmp_1765_8_2E = ltmp_1764_8;
  goto ltmp_1777_D;

ltmp_1777_D:
  ltmp_1765_8 = ltmp_1765_8_2E;
  if ((ltmp_1765_8 == 0)) {
    goto ltmp_1778_D;
  } else {
    ltmp_1767_8_2E = ltmp_1765_8;
    goto ltmp_1779_D;
  }

ltmp_1778_D:
  ltmp_1766_8 = ltmp_4_1004(ltmp_1762_1023, ltmp_1753_13, ltmp_1754_7, ltmp_1755_13, ltmp_1756_17);
  ltmp_1767_8_2E = ltmp_1766_8;
  goto ltmp_1779_D;

ltmp_1779_D:
  ltmp_1767_8 = ltmp_1767_8_2E;
  if (((ltmp_1767_8 == 0) & (ltmp_1760_17 != ((plzo_uint32_t *)0)))) {
    goto ltmp_1780_D;
  } else {
    goto ltmp_1781_D;
  }

ltmp_1780_D:
  ltmp_1768_7 = *(&ltmp_1761_103C.field0.field0.field2.field0);
  *ltmp_1760_17 = ltmp_1768_7;
  ltmp_1769_7 = *(&ltmp_1761_103C.field0.field0.field2.field1);
  *(&ltmp_1760_17[PLZO_INT64_C(1)]) = ltmp_1769_7;
  ltmp_1770_7 = *(&ltmp_1761_103C.field0.field0.field2.field2);
  *(&ltmp_1760_17[PLZO_INT64_C(2)]) = ltmp_1770_7;
  ltmp_1771_7 = *(&ltmp_1761_103C.field0.field0.field2.field3);
  *(&ltmp_1760_17[PLZO_INT64_C(3)]) = ltmp_1771_7;
  ltmp_1772_7 = *(&ltmp_1761_103C.field0.field0.field2.field4);
  *(&ltmp_1760_17[PLZO_INT64_C(4)]) = ltmp_1772_7;
  ltmp_1773_7 = *(&ltmp_1761_103C.field0.field0.field2.field5);
  *(&ltmp_1760_17[PLZO_INT64_C(5)]) = ltmp_1773_7;
  ltmp_1774_7 = *(&ltmp_1761_103C.field0.field0.field2.field6);
  *(&ltmp_1760_17[PLZO_INT64_C(6)]) = ltmp_1774_7;
  ltmp_1775_7 = *(&ltmp_1761_103C.field0.field0.field2.field7);
  *(&ltmp_1760_17[PLZO_INT64_C(7)]) = ltmp_1775_7;
  goto ltmp_1781_D;

ltmp_1781_D:
  *(&ltmp_1761_103C.field0.field0.field0) = (&(ltmp_69_1021[PLZO_INT64_C(2)]));
  ltmp_5_1005(ltmp_1762_1023);
  return ltmp_1767_8;
}


static void ltmp_25_1010(struct l_struct_2E_Lzo1E *ltmp_1782_103D) {
  unsigned char **ltmp_1783_1025;
  unsigned char *ltmp_1784_13;
  plzo_uint64_t ltmp_1785_9;
  plzo_uint64_t ltmp_1785_9_2E;
  plzo_uint64_t ltmp_1786_9;
  unsigned char *ltmp_1787_13;
  plzo_uint64_t ltmp_1788_9;
  plzo_uint64_t ltmp_1788_9_2E;
  plzo_uint64_t ltmp_1789_9;
  unsigned char *ltmp_1790_13;
  plzo_uint64_t ltmp_1791_9;
  plzo_uint64_t ltmp_1791_9_2E;
  plzo_uint64_t ltmp_1792_9;
  plzo_uint32_t *ltmp_1793_17;
  plzo_uint32_t *ltmp_1794_17;
  unsigned char *ltmp_1795_13;
  plzo_uint64_t ltmp_1796_9;
  plzo_uint64_t ltmp_1796_9_2E;
  plzo_uint64_t ltmp_1797_9;
  unsigned char *ltmp_1798_13;
  plzo_uint64_t ltmp_1799_9;
  plzo_uint64_t ltmp_1799_9_2E;
  plzo_uint64_t ltmp_1800_9;
  unsigned char *ltmp_1801_13;
  plzo_uint64_t ltmp_1802_9;
  plzo_uint64_t ltmp_1802_9_2E;
  plzo_uint64_t ltmp_1803_9;
  unsigned char *ltmp_1804_13;
  plzo_uint64_t ltmp_1805_9;
  plzo_uint64_t ltmp_1805_9_2E;
  plzo_uint64_t ltmp_1806_9;
  plzo_uint32_t *ltmp_1807_17;
  struct l_struct_2E_E_3A__3A_A15 *ltmp_1808_103E;
  unsigned char *ltmp_1809_13;
  plzo_uint64_t ltmp_1810_9;
  plzo_uint64_t ltmp_1810_9_2E;
  plzo_uint64_t ltmp_1811_9;

  *(&ltmp_1782_103D->field0.field1.field2) = 0u;
  *(&ltmp_1782_103D->field0.field1.field3) = 8u;
  *(&ltmp_1782_103D->field0.field1.field4) = 1u;
  *(&ltmp_1782_103D->field0.field1.field0) = 0u;
  *(&ltmp_1782_103D->field0.field1.field1) = 0u;
  *(&ltmp_1782_103D->field0.field1.field5) = ((unsigned char *)0);
  ltmp_1783_1025 = &ltmp_1782_103D->field0.field1.field6;
  *ltmp_1783_1025 = ((unsigned char *)0);
  *(&ltmp_1782_103D->field0.field0) = (&(ltmp_70_1021[PLZO_INT64_C(2)]));
  *(&ltmp_1782_103D->field0.field20.field0) = 0u;
  *(&ltmp_1782_103D->field0.field20.field1) = 0u;
  *(&ltmp_1782_103D->field0.field20.field2) = 0u;
  *(&ltmp_1782_103D->field0.field20.field3) = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  *(&ltmp_1782_103D->field0.field20.field4) = ((struct l_struct_2E_A9 *)0);
  *(&ltmp_1782_103D->field0.field3) = 0;
  ltmp_1784_13 = (unsigned char *)(&ltmp_1782_103D->field0.field6);
  ltmp_1785_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1812_D;

  do {
ltmp_1812_D:
  ltmp_1785_9 = ltmp_1785_9_2E;
  *(&ltmp_1784_13[ltmp_1785_9]) = 0;
  ltmp_1786_9 = ltmp_1785_9 + PLZO_UINT64_C(1);
  if ((ltmp_1786_9 == PLZO_UINT64_C(96))) {
    goto ltmp_1813_D;
  } else {
    ltmp_1785_9_2E = ltmp_1786_9;
    goto ltmp_1812_D;
  }

  } while (1);
ltmp_1813_D:
  ltmp_1787_13 = (unsigned char *)(&ltmp_1782_103D->field0.field7);
  ltmp_1788_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1814_D;

  do {
ltmp_1814_D:
  ltmp_1788_9 = ltmp_1788_9_2E;
  *(&ltmp_1787_13[ltmp_1788_9]) = 0;
  ltmp_1789_9 = ltmp_1788_9 + PLZO_UINT64_C(1);
  if ((ltmp_1789_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1815_D;
  } else {
    ltmp_1788_9_2E = ltmp_1789_9;
    goto ltmp_1814_D;
  }

  } while (1);
ltmp_1815_D:
  ltmp_1790_13 = (unsigned char *)(&ltmp_1782_103D->field0.field24);
  ltmp_1791_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1816_D;

  do {
ltmp_1816_D:
  ltmp_1791_9 = ltmp_1791_9_2E;
  *(&ltmp_1790_13[ltmp_1791_9]) = 0;
  ltmp_1792_9 = ltmp_1791_9 + PLZO_UINT64_C(1);
  if ((ltmp_1792_9 == PLZO_UINT64_C(48))) {
    goto ltmp_1817_D;
  } else {
    ltmp_1791_9_2E = ltmp_1792_9;
    goto ltmp_1816_D;
  }

  } while (1);
ltmp_1817_D:
  *(&ltmp_1782_103D->field0.field24.field2.field0) = ltmp_49_1018;
  *(&ltmp_1782_103D->field0.field24.field2.field1) = ltmp_50_1019;
  *(&ltmp_1782_103D->field0.field24.field2.field2) = ltmp_51_101A;
  *(&ltmp_1782_103D->field0.field24.field2.field3) = ((signed char *)0);
  *(&ltmp_1782_103D->field0.field24.field0) = 0u;
  *(&ltmp_1782_103D->field0.field24.field1) = 1024u;
  ltmp_1793_17 = &ltmp_1782_103D->field0.field6.field2;
  *ltmp_1793_17 = 3u;
  *(&ltmp_1782_103D->field0.field6.field4) = 2u;
  *(&ltmp_1782_103D->field0.field6.field5) = 4294967295u;
  ltmp_1794_17 = &ltmp_1782_103D->field0.field6.field3;
  *ltmp_1794_17 = 4294967295u;
  *ltmp_1783_1025 = ((unsigned char *)0);
  *(&ltmp_1782_103D->field0.field10) = ((unsigned char *)0);
  *(&ltmp_1782_103D->field0.field8) = ((unsigned char *)0);
  *(&ltmp_1782_103D->field0.field11) = 0u;
  *(&ltmp_1782_103D->field0.field9) = ((unsigned char *)0);
  *(&ltmp_1782_103D->field0.field12) = 0u;
  *(&ltmp_1782_103D->field0.field13) = 0u;
  *(&ltmp_1782_103D->field0.field14) = 0u;
  *(&ltmp_1782_103D->field0.field15) = 0u;
  *(&ltmp_1782_103D->field0.field16) = 0u;
  *(&ltmp_1782_103D->field0.field17) = 0u;
  *(&ltmp_1782_103D->field0.field18) = 0u;
  ltmp_1795_13 = (unsigned char *)(&ltmp_1782_103D->field0.field2);
  ltmp_1796_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1818_D;

  do {
ltmp_1818_D:
  ltmp_1796_9 = ltmp_1796_9_2E;
  *(&ltmp_1795_13[ltmp_1796_9]) = 0;
  ltmp_1797_9 = ltmp_1796_9 + PLZO_UINT64_C(1);
  if ((ltmp_1797_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1819_D;
  } else {
    ltmp_1796_9_2E = ltmp_1797_9;
    goto ltmp_1818_D;
  }

  } while (1);
ltmp_1819_D:
  *(&ltmp_1782_103D->field0.field2.field0) = 4294967295u;
  *(&ltmp_1782_103D->field0.field2.field2) = 4294967295u;
  *(&ltmp_1782_103D->field0.field2.field4) = 4294967295u;
  ltmp_1798_13 = (unsigned char *)(&ltmp_1782_103D->field0.field22);
  ltmp_1799_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1820_D;

  do {
ltmp_1820_D:
  ltmp_1799_9 = ltmp_1799_9_2E;
  *(&ltmp_1798_13[ltmp_1799_9]) = 0;
  ltmp_1800_9 = ltmp_1799_9 + PLZO_UINT64_C(1);
  if ((ltmp_1800_9 == PLZO_UINT64_C(32))) {
    goto ltmp_1821_D;
  } else {
    ltmp_1799_9_2E = ltmp_1800_9;
    goto ltmp_1820_D;
  }

  } while (1);
ltmp_1821_D:
  ltmp_1801_13 = (unsigned char *)(&ltmp_1782_103D->field0.field23);
  ltmp_1802_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1822_D;

  do {
ltmp_1822_D:
  ltmp_1802_9 = ltmp_1802_9_2E;
  *(&ltmp_1801_13[ltmp_1802_9]) = 0;
  ltmp_1803_9 = ltmp_1802_9 + PLZO_UINT64_C(1);
  if ((ltmp_1803_9 == PLZO_UINT64_C(32))) {
    goto ltmp_1823_D;
  } else {
    ltmp_1802_9_2E = ltmp_1803_9;
    goto ltmp_1822_D;
  }

  } while (1);
ltmp_1823_D:
  *(&ltmp_1782_103D->field0.field22.field1) = 512u;
  *(&ltmp_1782_103D->field0.field23.field1) = 4096u;
  ltmp_1804_13 = (unsigned char *)(&ltmp_1782_103D->field0.field19);
  ltmp_1805_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1824_D;

  do {
ltmp_1824_D:
  ltmp_1805_9 = ltmp_1805_9_2E;
  *(&ltmp_1804_13[ltmp_1805_9]) = 0;
  ltmp_1806_9 = ltmp_1805_9 + PLZO_UINT64_C(1);
  if ((ltmp_1806_9 == PLZO_UINT64_C(56))) {
    goto ltmp_1825_D;
  } else {
    ltmp_1805_9_2E = ltmp_1806_9;
    goto ltmp_1824_D;
  }

  } while (1);
ltmp_1825_D:
  ltmp_1807_17 = &ltmp_1782_103D->field0.field21;
  *ltmp_1807_17 = 1u;
  ltmp_1808_103E = &ltmp_1782_103D->field0.field25;
  *(&ltmp_1782_103D->field0.field26) = ltmp_1808_103E;
  ltmp_1809_13 = (unsigned char *)ltmp_1808_103E;
  ltmp_1810_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1826_D;

  do {
ltmp_1826_D:
  ltmp_1810_9 = ltmp_1810_9_2E;
  *(&ltmp_1809_13[ltmp_1810_9]) = 0;
  ltmp_1811_9 = ltmp_1810_9 + PLZO_UINT64_C(1);
  if ((ltmp_1811_9 == PLZO_UINT64_C(1))) {
    goto ltmp_1827_D;
  } else {
    ltmp_1810_9_2E = ltmp_1811_9;
    goto ltmp_1826_D;
  }

  } while (1);
ltmp_1827_D:
  *(&ltmp_1782_103D->field0.field0) = (&(ltmp_69_1021[PLZO_INT64_C(2)]));
  *ltmp_1807_17 = 5u;
  *(&ltmp_1782_103D->field1) = 0u;
  *(&ltmp_1782_103D->field2) = 64u;
  *(&ltmp_1782_103D->field3) = 36u;
  *(&ltmp_1782_103D->field4) = 16u;
  *(&ltmp_1782_103D->field5) = 32u;
  *(&ltmp_1782_103D->field6) = 1024u;
  *(&ltmp_1782_103D->field7) = 2048u;
  *(&ltmp_1782_103D->field8) = 16384u;
  *(&ltmp_1782_103D->field9) = 49151u;
  *(&ltmp_1782_103D->field10) = 1064960u;
  *(&ltmp_1782_103D->field11) = 3072u;
  *(&ltmp_1782_103D->field12) = 2u;
  *(&ltmp_1782_103D->field13) = 3u;
  *(&ltmp_1782_103D->field14) = 3u;
  *(&ltmp_1782_103D->field15) = 3u;
  *(&ltmp_1782_103D->field16) = 4u;
  *(&ltmp_1782_103D->field17) = 2u;
  *(&ltmp_1782_103D->field18) = 8u;
  *(&ltmp_1782_103D->field19) = 29u;
  *(&ltmp_1782_103D->field20) = 10u;
  *(&ltmp_1782_103D->field21) = 18u;
  *ltmp_1793_17 = 4u;
  *ltmp_1794_17 = 1064960u;
  return;
}


static plzo_int32_t plzo_lzo1y_99_compress_internal(unsigned char *ltmp_1828_13, plzo_uint32_t ltmp_1829_7, unsigned char *ltmp_1830_13, plzo_uint32_t *ltmp_1831_17, struct plzo_callback_t *ltmp_1832_1026, plzo_int32_t ltmp_1833_8, struct plzo_compress_config_t *ltmp_1834_1027, plzo_uint32_t *ltmp_1835_17) {
  struct l_struct_2E_Lzo1yE ltmp_1836_103F;
  plzo_int32_t  (***ltmp_1837_102C) ();
  struct l_struct_2E_E *ltmp_1838_1023;
  plzo_int32_t ltmp_1839_8;
  plzo_int32_t ltmp_1840_8;
  plzo_int32_t ltmp_1841_8;
  plzo_int32_t ltmp_1841_8_2E;
  plzo_int32_t ltmp_1842_8;
  plzo_int32_t ltmp_1843_8;
  plzo_int32_t ltmp_1843_8_2E;
  plzo_uint32_t ltmp_1844_7;
  plzo_uint32_t ltmp_1845_7;
  plzo_uint32_t ltmp_1846_7;
  plzo_uint32_t ltmp_1847_7;
  plzo_uint32_t ltmp_1848_7;
  plzo_uint32_t ltmp_1849_7;
  plzo_uint32_t ltmp_1850_7;
  plzo_uint32_t ltmp_1851_7;

  ltmp_25_1010((&ltmp_1836_103F.field0.field0));
  ltmp_1837_102C = &ltmp_1836_103F.field0.field0.field0.field0;
  *(&ltmp_1836_103F.field0.field0.field1) = 0u;
  *(&ltmp_1836_103F.field0.field0.field2) = 64u;
  *(&ltmp_1836_103F.field0.field0.field3) = 32u;
  *(&ltmp_1836_103F.field0.field0.field4) = 16u;
  *(&ltmp_1836_103F.field0.field0.field5) = 0u;
  *(&ltmp_1836_103F.field0.field0.field6) = 1024u;
  *(&ltmp_1836_103F.field0.field0.field8) = 16384u;
  *(&ltmp_1836_103F.field0.field0.field9) = 49151u;
  *(&ltmp_1836_103F.field0.field0.field10) = 0u;
  *(&ltmp_1836_103F.field0.field0.field12) = 2u;
  *(&ltmp_1836_103F.field0.field0.field13) = 3u;
  *(&ltmp_1836_103F.field0.field0.field14) = 3u;
  *(&ltmp_1836_103F.field0.field0.field15) = 3u;
  *(&ltmp_1836_103F.field0.field0.field16) = 0u;
  *(&ltmp_1836_103F.field0.field0.field17) = 2u;
  *(&ltmp_1836_103F.field0.field0.field19) = 33u;
  *(&ltmp_1836_103F.field0.field0.field20) = 9u;
  *(&ltmp_1836_103F.field0.field0.field21) = 0u;
  *(&ltmp_1836_103F.field0.field0.field0.field6.field3) = 49151u;
  *(&ltmp_1836_103F.field0.field0.field0.field0) = (&(ltmp_68_1021[PLZO_INT64_C(2)]));
  *(&ltmp_1836_103F.field0.field0.field18) = 14u;
  *(&ltmp_1836_103F.field0.field0.field7) = 1024u;
  *(&ltmp_1836_103F.field0.field0.field11) = 2048u;
  *(&ltmp_1836_103F.field0.field0.field0.field6.field0) = 1;
  ltmp_1838_1023 = &ltmp_1836_103F.field0.field0.field0;
  ltmp_1839_8 = ltmp_2_1002(ltmp_1838_1023, ltmp_1833_8, ltmp_1832_1026);
  if (((ltmp_1839_8 == 0) & (ltmp_1834_1027 != ((struct plzo_compress_config_t *)0)))) {
    goto ltmp_1852_D;
  } else {
    ltmp_1841_8_2E = ltmp_1839_8;
    goto ltmp_1853_D;
  }

ltmp_1852_D:
  ltmp_1840_8 = ltmp_3_1003(ltmp_1838_1023, ltmp_1834_1027);
  ltmp_1841_8_2E = ltmp_1840_8;
  goto ltmp_1853_D;

ltmp_1853_D:
  ltmp_1841_8 = ltmp_1841_8_2E;
  if ((ltmp_1841_8 == 0)) {
    goto ltmp_1854_D;
  } else {
    ltmp_1843_8_2E = ltmp_1841_8;
    goto ltmp_1855_D;
  }

ltmp_1854_D:
  ltmp_1842_8 = ltmp_4_1004(ltmp_1838_1023, ltmp_1828_13, ltmp_1829_7, ltmp_1830_13, ltmp_1831_17);
  ltmp_1843_8_2E = ltmp_1842_8;
  goto ltmp_1855_D;

ltmp_1855_D:
  ltmp_1843_8 = ltmp_1843_8_2E;
  if (((ltmp_1843_8 == 0) & (ltmp_1835_17 != ((plzo_uint32_t *)0)))) {
    goto ltmp_1856_D;
  } else {
    goto ltmp_1857_D;
  }

ltmp_1856_D:
  ltmp_1844_7 = *(&ltmp_1836_103F.field0.field0.field0.field2.field0);
  *ltmp_1835_17 = ltmp_1844_7;
  ltmp_1845_7 = *(&ltmp_1836_103F.field0.field0.field0.field2.field1);
  *(&ltmp_1835_17[PLZO_INT64_C(1)]) = ltmp_1845_7;
  ltmp_1846_7 = *(&ltmp_1836_103F.field0.field0.field0.field2.field2);
  *(&ltmp_1835_17[PLZO_INT64_C(2)]) = ltmp_1846_7;
  ltmp_1847_7 = *(&ltmp_1836_103F.field0.field0.field0.field2.field3);
  *(&ltmp_1835_17[PLZO_INT64_C(3)]) = ltmp_1847_7;
  ltmp_1848_7 = *(&ltmp_1836_103F.field0.field0.field0.field2.field4);
  *(&ltmp_1835_17[PLZO_INT64_C(4)]) = ltmp_1848_7;
  ltmp_1849_7 = *(&ltmp_1836_103F.field0.field0.field0.field2.field5);
  *(&ltmp_1835_17[PLZO_INT64_C(5)]) = ltmp_1849_7;
  ltmp_1850_7 = *(&ltmp_1836_103F.field0.field0.field0.field2.field6);
  *(&ltmp_1835_17[PLZO_INT64_C(6)]) = ltmp_1850_7;
  ltmp_1851_7 = *(&ltmp_1836_103F.field0.field0.field0.field2.field7);
  *(&ltmp_1835_17[PLZO_INT64_C(7)]) = ltmp_1851_7;
  goto ltmp_1857_D;

ltmp_1857_D:
  *ltmp_1837_102C = (&(ltmp_69_1021[PLZO_INT64_C(2)]));
  ltmp_5_1005(ltmp_1838_1023);
  return ltmp_1843_8;
}


static signed char *ltmp_26_1011(struct l_struct_2E_E *ltmp_1858_1023) {

  return ((signed char *)0);
}


static void ltmp_27_1010(struct l_struct_2E_Lzo1E *ltmp_1859_103D) {
  unsigned char **ltmp_1860_1025;
  unsigned char *ltmp_1861_13;
  plzo_uint32_t ltmp_1862_7;
  unsigned char *ltmp_1863_13;
  unsigned char *ltmp_1864_13;

  ltmp_1860_1025 = &ltmp_1859_103D->field0.field1.field6;
  ltmp_1861_13 = *ltmp_1860_1025;
  *ltmp_1860_1025 = (&ltmp_1861_13[PLZO_INT64_C(1)]);
  ltmp_1862_7 = *(&ltmp_1859_103D->field4);
  *ltmp_1861_13 = (((unsigned char )(((unsigned char )ltmp_1862_7) + 1)));
  ltmp_1863_13 = *ltmp_1860_1025;
  *ltmp_1860_1025 = (&ltmp_1863_13[PLZO_INT64_C(1)]);
  *ltmp_1863_13 = 0;
  ltmp_1864_13 = *ltmp_1860_1025;
  *ltmp_1860_1025 = (&ltmp_1864_13[PLZO_INT64_C(1)]);
  *ltmp_1864_13 = 0;
  return;
}


static plzo_int32_t ltmp_28_1012(struct l_struct_2E_Lzo1E *ltmp_1865_103D) {

  return 0;
}


static void ltmp_29_1013(struct l_struct_2E_Lzo1xE *ltmp_1866_103C) {

  return;
}


static void ltmp_30_1014(struct l_struct_2E_Lzo1E *ltmp_1867_103D, struct l_struct_2E_E_3A__3A_A4 *ltmp_1868_103B, plzo_uint32_t ltmp_1869_7, plzo_uint32_t ltmp_1870_7, plzo_uint32_t ltmp_1871_7) {
  plzo_uint32_t ltmp_1872_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1873_1031;
  plzo_uint32_t ltmp_1874_7;
  plzo_uint32_t ltmp_1875_7;
  struct l_struct_2E_A7 *ltmp_1876_1038;
  plzo_int64_t ltmp_1877_A;
  plzo_uint32_t ltmp_1878_7;
  plzo_uint32_t ltmp_1879_7;
  plzo_uint32_t ltmp_1880_7;
  plzo_uint32_t ltmp_1881_7;
  plzo_uint32_t ltmp_1882_7;
  plzo_uint32_t ltmp_1883_7;
  plzo_uint32_t ltmp_1884_7;
  plzo_uint32_t ltmp_1885_7;
  plzo_uint32_t ltmp_1886_7;
  plzo_uint32_t ltmp_1887_7;
  plzo_uint32_t ltmp_1888_7;
  plzo_uint32_t ltmp_1889_7;
  plzo_uint32_t ltmp_1890_7;
  plzo_uint32_t ltmp_1891_7;
  plzo_uint32_t ltmp_1892_7;
  plzo_uint32_t ltmp_1893_7;
  plzo_uint32_t ltmp_1894_7;
  plzo_uint32_t ltmp_1895_7;
  plzo_uint32_t ltmp_1896_7;
  plzo_uint32_t ltmp_1897_7;
  plzo_uint32_t ltmp_1897_7_2E;
  plzo_uint32_t ltmp_1898_7;
  plzo_uint32_t ltmp_1898_7_2E;
  plzo_uint32_t ltmp_1899_7;
  plzo_uint32_t ltmp_1899_7_2E;
  plzo_uint32_t ltmp_1900_7;
  plzo_uint32_t ltmp_1900_7_2E;
  plzo_uint32_t ltmp_1901_7;
  plzo_uint32_t ltmp_1902_7;
  plzo_uint32_t ltmp_1902_7_2E;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1903_1031;
  plzo_uint32_t ltmp_1904_7;
  plzo_uint32_t ltmp_1905_7;

  if ((ltmp_1869_7 == 0u)) {
    goto ltmp_1906_D;
  } else {
    goto ltmp_1907_D;
  }

ltmp_1906_D:
  ltmp_1872_7 = ltmp_1871_7 + 1u;
  ltmp_1873_1031 = *(&ltmp_1867_103D->field0.field20.field3);
  ltmp_1874_7 = *(&ltmp_1867_103D->field0.field20.field1);
  ltmp_1875_7 = *(&ltmp_1873_1031[((plzo_int64_t )((ltmp_1874_7*(ltmp_1870_7 + 1u)) + ((((ltmp_1874_7 > ltmp_1872_7)) ? (ltmp_1872_7) : ((ltmp_1874_7 + 4294967295u))))))].field0);
  *(&ltmp_1868_103B->field0) = (ltmp_1875_7 + ((((ltmp_1871_7 == 3u)) ? (16u) : (8u))));
  return;
ltmp_1907_D:
  ltmp_1876_1038 = *(&ltmp_1867_103D->field0.field19.field0);
  ltmp_1877_A = (plzo_int64_t )ltmp_1870_7;
  ltmp_1878_7 = *(&ltmp_1876_1038[ltmp_1877_A].field0);
  ltmp_1879_7 = ltmp_1878_7 - ltmp_1869_7;
  ltmp_1880_7 = *(&ltmp_1876_1038[ltmp_1877_A].field1[((((ltmp_1879_7 < 8u)) ? (((plzo_int64_t )ltmp_1879_7)) : (PLZO_INT64_C(7))))]);
  if ((ltmp_1869_7 > 1u)) {
    goto ltmp_1908_D;
  } else {
    goto ltmp_1909_D;
  }

ltmp_1908_D:
  if ((ltmp_1869_7 == 2u)) {
    goto ltmp_1910_D;
  } else {
    goto ltmp_1911_D;
  }

ltmp_1910_D:
  ltmp_1881_7 = *(&ltmp_1867_103D->field6);
  if ((((ltmp_1871_7 == 0u) | (ltmp_1871_7 > 3u)) | (ltmp_1881_7 < ltmp_1880_7))) {
    goto ltmp_1909_D;
  } else {
    goto ltmp_1911_D;
  }

ltmp_1911_D:
  ltmp_1882_7 = *(&ltmp_1867_103D->field16);
  if ((ltmp_1882_7 > ltmp_1869_7)) {
    goto ltmp_1912_D;
  } else {
    goto ltmp_1913_D;
  }

ltmp_1912_D:
  ltmp_1883_7 = *(&ltmp_1867_103D->field9);
  if ((ltmp_1883_7 < ltmp_1880_7)) {
    goto ltmp_1909_D;
  } else {
    goto ltmp_1913_D;
  }

ltmp_1913_D:
  if ((ltmp_1869_7 == 2u)) {
    ltmp_1897_7_2E = 16u;
    goto ltmp_1914_D;
  } else {
    goto ltmp_1915_D;
  }

ltmp_1915_D:
  ltmp_1884_7 = *(&ltmp_1867_103D->field18);
  if ((ltmp_1884_7 < ltmp_1869_7)) {
    goto ltmp_1916_D;
  } else {
    goto ltmp_1917_D;
  }

ltmp_1917_D:
  ltmp_1885_7 = *(&ltmp_1867_103D->field7);
  if ((ltmp_1885_7 < ltmp_1880_7)) {
    goto ltmp_1916_D;
  } else {
    ltmp_1897_7_2E = 16u;
    goto ltmp_1914_D;
  }

ltmp_1916_D:
  ltmp_1886_7 = *(&ltmp_1867_103D->field13);
  if ((ltmp_1886_7 == ltmp_1869_7)) {
    goto ltmp_1918_D;
  } else {
    goto ltmp_1919_D;
  }

ltmp_1918_D:
  ltmp_1887_7 = *(&ltmp_1867_103D->field11);
  if (((ltmp_1887_7 >= ltmp_1880_7) & (ltmp_1871_7 > 3u))) {
    ltmp_1897_7_2E = 16u;
    goto ltmp_1914_D;
  } else {
    goto ltmp_1919_D;
  }

ltmp_1919_D:
  ltmp_1888_7 = *(&ltmp_1867_103D->field8);
  if ((ltmp_1888_7 < ltmp_1880_7)) {
    goto ltmp_1920_D;
  } else {
    goto ltmp_1921_D;
  }

ltmp_1921_D:
  ltmp_1889_7 = *(&ltmp_1867_103D->field19);
  if ((ltmp_1889_7 < ltmp_1869_7)) {
    goto ltmp_1922_D;
  } else {
    ltmp_1897_7_2E = 24u;
    goto ltmp_1914_D;
  }

ltmp_1922_D:
  ltmp_1890_7 = ltmp_1869_7 - ltmp_1889_7;
  if ((ltmp_1890_7 > 255u)) {
    ltmp_1898_7_2E = ltmp_1890_7;
    ltmp_1899_7_2E = 32u;
    ltmp_1900_7_2E = 0u;
    goto ltmp_1923_D;
  } else {
    ltmp_1902_7_2E = 32u;
    goto ltmp_1924_D;
  }

ltmp_1920_D:
  ltmp_1891_7 = *(&ltmp_1867_103D->field20);
  if ((ltmp_1891_7 < ltmp_1869_7)) {
    goto ltmp_1925_D;
  } else {
    goto ltmp_1926_D;
  }

ltmp_1926_D:
  ltmp_1892_7 = *(&ltmp_1867_103D->field9);
  if ((ltmp_1892_7 < ltmp_1880_7)) {
    goto ltmp_1925_D;
  } else {
    goto ltmp_1927_D;
  }

ltmp_1925_D:
  ltmp_1893_7 = *(&ltmp_1867_103D->field10);
  if ((ltmp_1893_7 == 0u)) {
    goto ltmp_1927_D;
  } else {
    goto ltmp_1928_D;
  }

ltmp_1927_D:
  if ((ltmp_1891_7 < ltmp_1869_7)) {
    goto ltmp_1929_D;
  } else {
    ltmp_1897_7_2E = 24u;
    goto ltmp_1914_D;
  }

ltmp_1929_D:
  ltmp_1894_7 = ltmp_1869_7 - ltmp_1891_7;
  if ((ltmp_1894_7 > 255u)) {
    ltmp_1898_7_2E = ltmp_1894_7;
    ltmp_1899_7_2E = 32u;
    ltmp_1900_7_2E = 0u;
    goto ltmp_1923_D;
  } else {
    ltmp_1902_7_2E = 32u;
    goto ltmp_1924_D;
  }

ltmp_1928_D:
  ltmp_1895_7 = *(&ltmp_1867_103D->field21);
  if ((ltmp_1895_7 < ltmp_1869_7)) {
    goto ltmp_1930_D;
  } else {
    ltmp_1897_7_2E = 32u;
    goto ltmp_1914_D;
  }

ltmp_1930_D:
  ltmp_1896_7 = ltmp_1869_7 - ltmp_1895_7;
  if ((ltmp_1896_7 > 255u)) {
    ltmp_1898_7_2E = ltmp_1896_7;
    ltmp_1899_7_2E = 40u;
    ltmp_1900_7_2E = 0u;
    goto ltmp_1923_D;
  } else {
    ltmp_1902_7_2E = 40u;
    goto ltmp_1924_D;
  }

ltmp_1914_D:
  ltmp_1897_7 = ltmp_1897_7_2E;
  if ((ltmp_1869_7 > 255u)) {
    ltmp_1898_7_2E = ltmp_1869_7;
    ltmp_1899_7_2E = ltmp_1897_7;
    ltmp_1900_7_2E = 0u;
    goto ltmp_1923_D;
  } else {
    ltmp_1902_7_2E = ltmp_1897_7;
    goto ltmp_1924_D;
  }

  do {
ltmp_1923_D:
  ltmp_1898_7 = ltmp_1898_7_2E;
  ltmp_1899_7 = ltmp_1899_7_2E;
  ltmp_1900_7 = ltmp_1900_7_2E;
  if (((((ltmp_1900_7*4294967041u) + ltmp_1898_7) + 4294967041u) > 255u)) {
    ltmp_1898_7_2E = ltmp_1898_7;
    ltmp_1899_7_2E = ltmp_1899_7;
    ltmp_1900_7_2E = (ltmp_1900_7 + 1u);
    goto ltmp_1923_D;
  } else {
    goto ltmp_1931_D;
  }

  } while (1);
ltmp_1931_D:
  ltmp_1901_7 = ((ltmp_1900_7 << 3) + ltmp_1899_7) + 8u;
  ltmp_1902_7_2E = ltmp_1901_7;
  goto ltmp_1924_D;

ltmp_1924_D:
  ltmp_1902_7 = ltmp_1902_7_2E;
  ltmp_1903_1031 = *(&ltmp_1867_103D->field0.field20.field3);
  ltmp_1904_7 = *(&ltmp_1867_103D->field0.field20.field1);
  ltmp_1905_7 = *(&ltmp_1903_1031[((plzo_int64_t )(ltmp_1904_7*(ltmp_1870_7 + ltmp_1869_7)))].field0);
  *(&ltmp_1868_103B->field0) = (ltmp_1905_7 + ltmp_1902_7);
  return;
ltmp_1909_D:
  *(&ltmp_1868_103B->field0) = 4294967295u;
  return;
}


static plzo_uint32_t ltmp_31_1015(struct l_struct_2E_Lzo1E *ltmp_1932_103D) {

  return 0u;
}


static plzo_int32_t ltmp_32_1016(struct l_struct_2E_E *ltmp_1933_1023, struct l_struct_2E_A7 *ltmp_1934_1038) {
  plzo_uint32_t ltmp_1935_7;

  ltmp_1935_7 = *(&ltmp_1934_1038->field0);
  return ((plzo_int32_t )(ltmp_1935_7 == 0u));
}


static plzo_int32_t ltmp_33_1016(struct l_struct_2E_E *ltmp_1936_1023, struct l_struct_2E_A7 *ltmp_1937_1038) {
  plzo_uint32_t ltmp_1938_7;

  ltmp_1938_7 = *(&ltmp_1937_1038->field0);
  return ((plzo_int32_t )(ltmp_1938_7 < 3u));
}


static void ltmp_34_100F(struct l_struct_2E_Lzo1E *ltmp_1939_103D, plzo_uint32_t ltmp_1940_7, plzo_uint32_t ltmp_1941_7) {
  unsigned char **ltmp_1942_1025;
  unsigned char *ltmp_1943_13;
  plzo_uint32_t ltmp_1944_7;
  plzo_uint32_t ltmp_1945_7;
  unsigned char *ltmp_1946_13;
  plzo_uint32_t ltmp_1947_7;
  plzo_uint32_t ltmp_1948_7;
  plzo_uint32_t ltmp_1949_7;
  unsigned char *ltmp_1950_13;
  plzo_uint32_t ltmp_1951_7;
  plzo_uint32_t ltmp_1952_7;
  plzo_uint32_t ltmp_1953_7;
  plzo_uint32_t ltmp_1954_7;
  plzo_uint32_t ltmp_1955_7;
  plzo_uint32_t ltmp_1956_7;
  unsigned char *ltmp_1957_13;
  plzo_uint32_t ltmp_1958_7;
  plzo_uint32_t ltmp_1959_7;
  plzo_uint32_t ltmp_1960_7;
  unsigned char *ltmp_1961_13;
  plzo_uint32_t ltmp_1962_7;
  plzo_uint32_t ltmp_1963_7;
  unsigned char *ltmp_1964_13;
  plzo_uint32_t ltmp_1965_7;
  plzo_uint64_t ltmp_1966_9;
  plzo_uint64_t ltmp_1966_9_2E;
  plzo_uint64_t ltmp_1967_9;
  plzo_uint32_t ltmp_1968_7;
  unsigned char *ltmp_1969_13;
  unsigned char *ltmp_1970_13;
  unsigned char *ltmp_1970_13_2E;
  plzo_uint32_t ltmp_1971_7;
  plzo_uint32_t ltmp_1971_7_2E;
  unsigned char *ltmp_1972_13;
  unsigned char *ltmp_1973_13;
  unsigned char *ltmp_1973_13_2E;
  unsigned char *ltmp_1974_13;
  plzo_uint32_t ltmp_1975_7;
  plzo_uint32_t ltmp_1976_7;
  plzo_uint32_t ltmp_1977_7;
  plzo_uint32_t ltmp_1978_7;
  unsigned char *ltmp_1979_13;
  plzo_uint32_t ltmp_1980_7;
  unsigned char *ltmp_1981_13;
  plzo_uint32_t ltmp_1982_7;
  unsigned char ltmp_1983_3;
  unsigned char *ltmp_1984_13;
  unsigned char ltmp_1985_3;
  plzo_uint32_t ltmp_1986_7;
  unsigned char ltmp_1987_3;
  plzo_uint32_t ltmp_1988_7;
  plzo_uint64_t ltmp_1989_9;
  plzo_uint64_t ltmp_1989_9_2E;
  plzo_uint32_t ltmp_1990_7;
  unsigned char *ltmp_1991_13;
  unsigned char *ltmp_1992_13;
  unsigned char *ltmp_1992_13_2E;
  plzo_uint32_t ltmp_1993_7;
  plzo_uint32_t ltmp_1993_7_2E;
  unsigned char *ltmp_1994_13;
  unsigned char *ltmp_1995_13;
  unsigned char *ltmp_1995_13_2E;
  unsigned char *ltmp_1996_13;
  unsigned char *ltmp_1997_13;
  unsigned char *ltmp_1997_13_2E;

  ltmp_0_1000((&ltmp_1939_103D->field0), ltmp_1940_7, ltmp_1941_7);
  ltmp_1942_1025 = &ltmp_1939_103D->field0.field1.field6;
  ltmp_1943_13 = *ltmp_1942_1025;
  if ((ltmp_1940_7 == 2u)) {
    goto ltmp_1998_D;
  } else {
    goto ltmp_1999_D;
  }

ltmp_1998_D:
  ltmp_1944_7 = ltmp_1941_7 + 4294967295u;
  ltmp_1945_7 = *(&ltmp_1939_103D->field1);
  *ltmp_1943_13 = (((unsigned char )(((unsigned char )ltmp_1945_7) | ((unsigned char )(ltmp_1944_7 >> 6)))));
  ltmp_1946_13 = &ltmp_1943_13[PLZO_INT64_C(2)];
  *(&ltmp_1943_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1944_7) << 2)));
  ltmp_1997_13_2E = ltmp_1946_13;
  goto ltmp_2000_D;

ltmp_1999_D:
  ltmp_1947_7 = *(&ltmp_1939_103D->field18);
  if ((ltmp_1947_7 < ltmp_1940_7)) {
    goto ltmp_2001_D;
  } else {
    goto ltmp_2002_D;
  }

ltmp_2002_D:
  ltmp_1948_7 = *(&ltmp_1939_103D->field7);
  if ((ltmp_1948_7 < ltmp_1941_7)) {
    goto ltmp_2001_D;
  } else {
    goto ltmp_2003_D;
  }

ltmp_2003_D:
  ltmp_1949_7 = ltmp_1941_7 + 4294967295u;
  *ltmp_1943_13 = (((unsigned char )(((unsigned char )(ltmp_1949_7 >> 6)) | (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1940_7) << 5))) + 224))))));
  ltmp_1950_13 = &ltmp_1943_13[PLZO_INT64_C(2)];
  *(&ltmp_1943_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1949_7) << 2)));
  ltmp_1997_13_2E = ltmp_1950_13;
  goto ltmp_2000_D;

ltmp_2001_D:
  ltmp_1951_7 = *(&ltmp_1939_103D->field13);
  if ((ltmp_1951_7 == ltmp_1940_7)) {
    goto ltmp_2004_D;
  } else {
    goto ltmp_2005_D;
  }

ltmp_2004_D:
  ltmp_1952_7 = *(&ltmp_1939_103D->field11);
  if ((ltmp_1952_7 < ltmp_1941_7)) {
    goto ltmp_2005_D;
  } else {
    goto ltmp_2006_D;
  }

ltmp_2006_D:
  ltmp_1953_7 = *(&ltmp_1939_103D->field0.field17);
  if ((ltmp_1953_7 > 3u)) {
    goto ltmp_2007_D;
  } else {
    goto ltmp_2005_D;
  }

ltmp_2007_D:
  ltmp_1954_7 = *(&ltmp_1939_103D->field7);
  ltmp_1955_7 = (ltmp_1941_7 - ltmp_1954_7) + 4294967295u;
  ltmp_1956_7 = *(&ltmp_1939_103D->field1);
  *ltmp_1943_13 = (((unsigned char )(((unsigned char )(ltmp_1955_7 >> 6)) | ((unsigned char )ltmp_1956_7))));
  ltmp_1957_13 = &ltmp_1943_13[PLZO_INT64_C(2)];
  *(&ltmp_1943_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1955_7) << 2)));
  ltmp_1997_13_2E = ltmp_1957_13;
  goto ltmp_2000_D;

ltmp_2005_D:
  ltmp_1958_7 = *(&ltmp_1939_103D->field8);
  if ((ltmp_1958_7 < ltmp_1941_7)) {
    goto ltmp_2008_D;
  } else {
    goto ltmp_2009_D;
  }

ltmp_2009_D:
  ltmp_1959_7 = ltmp_1941_7 + 4294967295u;
  ltmp_1960_7 = *(&ltmp_1939_103D->field19);
  if ((ltmp_1960_7 < ltmp_1940_7)) {
    goto ltmp_2010_D;
  } else {
    goto ltmp_2011_D;
  }

ltmp_2011_D:
  ltmp_1961_13 = &ltmp_1943_13[PLZO_INT64_C(1)];
  ltmp_1962_7 = *(&ltmp_1939_103D->field3);
  *ltmp_1943_13 = (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1940_7) + 254))) + ((unsigned char )ltmp_1962_7))));
  ltmp_1973_13_2E = ltmp_1961_13;
  goto ltmp_2012_D;

ltmp_2010_D:
  ltmp_1963_7 = ltmp_1940_7 - ltmp_1960_7;
  ltmp_1964_13 = &ltmp_1943_13[PLZO_INT64_C(1)];
  ltmp_1965_7 = *(&ltmp_1939_103D->field3);
  *ltmp_1943_13 = ((unsigned char )ltmp_1965_7);
  if ((ltmp_1963_7 > 255u)) {
    ltmp_1966_9_2E = PLZO_UINT64_C(0);
    goto ltmp_2013_D;
  } else {
    ltmp_1970_13_2E = ltmp_1964_13;
    ltmp_1971_7_2E = ltmp_1963_7;
    goto ltmp_2014_D;
  }

  do {
ltmp_2013_D:
  ltmp_1966_9 = ltmp_1966_9_2E;
  ltmp_1967_9 = ltmp_1966_9 + PLZO_UINT64_C(1);
  ltmp_1968_7 = ((((plzo_uint32_t )ltmp_1966_9)*4294967041u) + ltmp_1963_7) + 4294967041u;
  *(&ltmp_1943_13[ltmp_1967_9]) = 0;
  if ((ltmp_1968_7 > 255u)) {
    ltmp_1966_9_2E = ltmp_1967_9;
    goto ltmp_2013_D;
  } else {
    goto ltmp_2015_D;
  }

  } while (1);
ltmp_2015_D:
  ltmp_1969_13 = &ltmp_1943_13[(((plzo_int64_t )ltmp_1966_9) + PLZO_INT64_C(2))];
  ltmp_1970_13_2E = ltmp_1969_13;
  ltmp_1971_7_2E = ltmp_1968_7;
  goto ltmp_2014_D;

ltmp_2014_D:
  ltmp_1970_13 = ltmp_1970_13_2E;
  ltmp_1971_7 = ltmp_1971_7_2E;
  ltmp_1972_13 = &ltmp_1970_13[PLZO_INT64_C(1)];
  *ltmp_1970_13 = ((unsigned char )ltmp_1971_7);
  ltmp_1973_13_2E = ltmp_1972_13;
  goto ltmp_2012_D;

ltmp_2012_D:
  ltmp_1973_13 = ltmp_1973_13_2E;
  *ltmp_1973_13 = ((unsigned char )(ltmp_1959_7 >> 6));
  ltmp_1974_13 = &ltmp_1973_13[PLZO_INT64_C(2)];
  *(&ltmp_1973_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1959_7) << 2)));
  ltmp_1997_13_2E = ltmp_1974_13;
  goto ltmp_2000_D;

ltmp_2008_D:
  ltmp_1975_7 = *(&ltmp_1939_103D->field20);
  if ((ltmp_1975_7 < ltmp_1940_7)) {
    goto ltmp_2016_D;
  } else {
    goto ltmp_2017_D;
  }

ltmp_2017_D:
  ltmp_1976_7 = *(&ltmp_1939_103D->field9);
  if ((ltmp_1976_7 < ltmp_1941_7)) {
    goto ltmp_2016_D;
  } else {
    goto ltmp_2018_D;
  }

ltmp_2018_D:
  ltmp_1977_7 = ltmp_1941_7 - ltmp_1958_7;
  ltmp_1978_7 = *(&ltmp_1939_103D->field4);
  *ltmp_1943_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )(ltmp_1977_7 >> 11)) & 8))) | (((unsigned char )(((unsigned char )ltmp_1940_7) + 253)))))) + ((unsigned char )ltmp_1978_7))));
  *(&ltmp_1943_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1977_7 >> 6));
  ltmp_1979_13 = &ltmp_1943_13[PLZO_INT64_C(3)];
  *(&ltmp_1943_13[PLZO_INT64_C(2)]) = (((unsigned char )(((unsigned char )ltmp_1977_7) << 2)));
  ltmp_1997_13_2E = ltmp_1979_13;
  goto ltmp_2000_D;

ltmp_2016_D:
  ltmp_1980_7 = (ltmp_1941_7 - ltmp_1958_7) + 4294967295u;
  ltmp_1981_13 = &ltmp_1943_13[PLZO_INT64_C(1)];
  ltmp_1982_7 = *(&ltmp_1939_103D->field5);
  ltmp_1983_3 = (unsigned char )ltmp_1982_7;
  *ltmp_1943_13 = ltmp_1983_3;
  ltmp_1984_13 = &ltmp_1943_13[PLZO_INT64_C(2)];
  ltmp_1985_3 = (unsigned char )(ltmp_1980_7 >> 14);
  *ltmp_1981_13 = ltmp_1985_3;
  ltmp_1986_7 = *(&ltmp_1939_103D->field21);
  if ((ltmp_1986_7 < ltmp_1940_7)) {
    goto ltmp_2019_D;
  } else {
    goto ltmp_2020_D;
  }

ltmp_2020_D:
  ltmp_1987_3 = ((unsigned char )(((unsigned char )ltmp_1940_7) + 253));
  *ltmp_1943_13 = (((unsigned char )(ltmp_1983_3 | (((unsigned char )(ltmp_1987_3 & 3))))));
  *ltmp_1981_13 = (((unsigned char )(ltmp_1985_3 | (((unsigned char )((((unsigned char )(ltmp_1987_3 << 4))) & 192))))));
  ltmp_1995_13_2E = ltmp_1984_13;
  goto ltmp_2021_D;

ltmp_2019_D:
  ltmp_1988_7 = ltmp_1940_7 - ltmp_1986_7;
  if ((ltmp_1988_7 > 255u)) {
    ltmp_1989_9_2E = PLZO_UINT64_C(0);
    goto ltmp_2022_D;
  } else {
    ltmp_1992_13_2E = ltmp_1984_13;
    ltmp_1993_7_2E = ltmp_1988_7;
    goto ltmp_2023_D;
  }

  do {
ltmp_2022_D:
  ltmp_1989_9 = ltmp_1989_9_2E;
  ltmp_1990_7 = ((((plzo_uint32_t )ltmp_1989_9)*4294967041u) + ltmp_1988_7) + 4294967041u;
  *(&ltmp_1943_13[(ltmp_1989_9 + PLZO_UINT64_C(2))]) = 0;
  if ((ltmp_1990_7 > 255u)) {
    ltmp_1989_9_2E = (ltmp_1989_9 + PLZO_UINT64_C(1));
    goto ltmp_2022_D;
  } else {
    goto ltmp_2024_D;
  }

  } while (1);
ltmp_2024_D:
  ltmp_1991_13 = &ltmp_1943_13[(((plzo_int64_t )ltmp_1989_9) + PLZO_INT64_C(3))];
  ltmp_1992_13_2E = ltmp_1991_13;
  ltmp_1993_7_2E = ltmp_1990_7;
  goto ltmp_2023_D;

ltmp_2023_D:
  ltmp_1992_13 = ltmp_1992_13_2E;
  ltmp_1993_7 = ltmp_1993_7_2E;
  ltmp_1994_13 = &ltmp_1992_13[PLZO_INT64_C(1)];
  *ltmp_1992_13 = ((unsigned char )ltmp_1993_7);
  ltmp_1995_13_2E = ltmp_1994_13;
  goto ltmp_2021_D;

ltmp_2021_D:
  ltmp_1995_13 = ltmp_1995_13_2E;
  *ltmp_1995_13 = ((unsigned char )(ltmp_1980_7 >> 6));
  ltmp_1996_13 = &ltmp_1995_13[PLZO_INT64_C(2)];
  *(&ltmp_1995_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1980_7) << 2)));
  ltmp_1997_13_2E = ltmp_1996_13;
  goto ltmp_2000_D;

ltmp_2000_D:
  ltmp_1997_13 = ltmp_1997_13_2E;
  *(&ltmp_1939_103D->field0.field15) = ltmp_1940_7;
  *(&ltmp_1939_103D->field0.field16) = ltmp_1941_7;
  *ltmp_1942_1025 = ltmp_1997_13;
  return;
}


static void ltmp_35_1017(struct l_struct_2E_Lzo1E *ltmp_2025_103D, unsigned char *ltmp_2026_13, plzo_uint32_t ltmp_2027_7, plzo_uint32_t ltmp_2028_7) {
  plzo_uint32_t *ltmp_2029_17;
  plzo_uint32_t ltmp_2030_7;
  plzo_uint32_t *ltmp_2031_17;
  plzo_uint32_t ltmp_2032_7;
  plzo_uint32_t ltmp_2033_7;
  plzo_uint32_t *ltmp_2034_17;
  unsigned char **ltmp_2035_1025;
  unsigned char *ltmp_2036_13;
  unsigned char *ltmp_2037_13;
  unsigned char ltmp_2038_3;
  unsigned char *ltmp_2039_13;
  plzo_uint32_t ltmp_2040_7;
  unsigned char *ltmp_2041_13;
  plzo_uint64_t ltmp_2042_9;
  plzo_uint64_t ltmp_2042_9_2E;
  plzo_uint64_t ltmp_2043_9;
  plzo_uint32_t ltmp_2044_7;
  unsigned char *ltmp_2045_13;
  plzo_uint32_t ltmp_2046_7;
  plzo_uint32_t ltmp_2046_7_2E;
  unsigned char *ltmp_2047_13;
  unsigned char *ltmp_2047_13_2E;
  unsigned char *ltmp_2048_13;
  unsigned char *ltmp_2049_13;
  unsigned char *ltmp_2049_13_2E;
  plzo_uint64_t ltmp_2050_9;
  plzo_uint64_t ltmp_2051_9;
  plzo_uint64_t ltmp_2051_9_2E;
  unsigned char ltmp_2052_3;
  plzo_uint64_t ltmp_2053_9;

  if ((ltmp_2027_7 == 0u)) {
    goto ltmp_2054_D;
  } else {
    goto ltmp_2055_D;
  }

ltmp_2055_D:
  ltmp_2029_17 = &ltmp_2025_103D->field0.field2.field5;
  ltmp_2030_7 = *ltmp_2029_17;
  ltmp_2031_17 = &ltmp_2025_103D->field0.field2.field4;
  if ((ltmp_2030_7 < ltmp_2027_7)) {
    goto ltmp_2056_D;
  } else {
    goto ltmp_2057_D;
  }

ltmp_2056_D:
  *ltmp_2029_17 = ltmp_2027_7;
  ltmp_2032_7 = *ltmp_2031_17;
  if ((ltmp_2032_7 > ltmp_2027_7)) {
    goto ltmp_2058_D;
  } else {
    goto ltmp_2054_D;
  }

ltmp_2057_D:
  ltmp_2033_7 = *ltmp_2031_17;
  if ((ltmp_2033_7 > ltmp_2027_7)) {
    goto ltmp_2058_D;
  } else {
    goto ltmp_2054_D;
  }

ltmp_2058_D:
  *ltmp_2031_17 = ltmp_2027_7;
  goto ltmp_2054_D;

ltmp_2054_D:
  ltmp_2034_17 = &ltmp_2025_103D->field0.field18;
  if ((ltmp_2027_7 == 0u)) {
    goto ltmp_2059_D;
  } else {
    goto ltmp_2060_D;
  }

ltmp_2059_D:
  *ltmp_2034_17 = 0u;
  *(&ltmp_2025_103D->field0.field17) = 0u;
  return;
ltmp_2060_D:
  *ltmp_2034_17 = ltmp_2028_7;
  *(&ltmp_2025_103D->field0.field17) = ltmp_2027_7;
  ltmp_2035_1025 = &ltmp_2025_103D->field0.field1.field6;
  ltmp_2036_13 = *ltmp_2035_1025;
  if ((ltmp_2027_7 > 3u)) {
    goto ltmp_2061_D;
  } else {
    goto ltmp_2062_D;
  }

ltmp_2062_D:
  ltmp_2037_13 = &ltmp_2036_13[PLZO_INT64_C(-1)];
  ltmp_2038_3 = *ltmp_2037_13;
  *ltmp_2037_13 = (((unsigned char )(ltmp_2038_3 | ((unsigned char )ltmp_2027_7))));
  ltmp_2049_13_2E = ltmp_2036_13;
  goto ltmp_2063_D;

ltmp_2061_D:
  if ((ltmp_2027_7 > 18u)) {
    goto ltmp_2064_D;
  } else {
    goto ltmp_2065_D;
  }

ltmp_2065_D:
  ltmp_2039_13 = &ltmp_2036_13[PLZO_INT64_C(1)];
  *ltmp_2036_13 = (((unsigned char )(((unsigned char )ltmp_2027_7) + 253)));
  ltmp_2049_13_2E = ltmp_2039_13;
  goto ltmp_2063_D;

ltmp_2064_D:
  ltmp_2040_7 = ltmp_2027_7 + 4294967278u;
  ltmp_2041_13 = &ltmp_2036_13[PLZO_INT64_C(1)];
  *ltmp_2036_13 = 0;
  if ((ltmp_2040_7 > 255u)) {
    ltmp_2042_9_2E = PLZO_UINT64_C(0);
    goto ltmp_2066_D;
  } else {
    ltmp_2046_7_2E = ltmp_2040_7;
    ltmp_2047_13_2E = ltmp_2041_13;
    goto ltmp_2067_D;
  }

  do {
ltmp_2066_D:
  ltmp_2042_9 = ltmp_2042_9_2E;
  ltmp_2043_9 = ltmp_2042_9 + PLZO_UINT64_C(1);
  ltmp_2044_7 = ((((plzo_uint32_t )ltmp_2042_9)*4294967041u) + ltmp_2040_7) + 4294967041u;
  *(&ltmp_2036_13[ltmp_2043_9]) = 0;
  if ((ltmp_2044_7 > 255u)) {
    ltmp_2042_9_2E = ltmp_2043_9;
    goto ltmp_2066_D;
  } else {
    goto ltmp_2068_D;
  }

  } while (1);
ltmp_2068_D:
  ltmp_2045_13 = &ltmp_2036_13[(((plzo_int64_t )ltmp_2042_9) + PLZO_INT64_C(2))];
  ltmp_2046_7_2E = ltmp_2044_7;
  ltmp_2047_13_2E = ltmp_2045_13;
  goto ltmp_2067_D;

ltmp_2067_D:
  ltmp_2046_7 = ltmp_2046_7_2E;
  ltmp_2047_13 = ltmp_2047_13_2E;
  ltmp_2048_13 = &ltmp_2047_13[PLZO_INT64_C(1)];
  *ltmp_2047_13 = ((unsigned char )ltmp_2046_7);
  ltmp_2049_13_2E = ltmp_2048_13;
  goto ltmp_2063_D;

ltmp_2063_D:
  ltmp_2049_13 = ltmp_2049_13_2E;
  ltmp_2050_9 = (plzo_uint64_t )ltmp_2027_7;
  ltmp_2051_9_2E = PLZO_UINT64_C(0);
  goto ltmp_2069_D;

  do {
ltmp_2069_D:
  ltmp_2051_9 = ltmp_2051_9_2E;
  ltmp_2052_3 = *(&ltmp_2026_13[ltmp_2051_9]);
  *(&ltmp_2049_13[ltmp_2051_9]) = ltmp_2052_3;
  ltmp_2053_9 = ltmp_2051_9 + PLZO_UINT64_C(1);
  if ((ltmp_2053_9 == ltmp_2050_9)) {
    goto ltmp_2070_D;
  } else {
    ltmp_2051_9_2E = ltmp_2053_9;
    goto ltmp_2069_D;
  }

  } while (1);
ltmp_2070_D:
  *ltmp_2035_1025 = (&ltmp_2049_13[(((plzo_int64_t )ltmp_2051_9) + PLZO_INT64_C(1))]);
  return;
}


static void ltmp_36_1010(struct l_struct_2E_Lzo1E *ltmp_2071_103D) {
  unsigned char **ltmp_2072_1025;
  unsigned char *ltmp_2073_13;

  ltmp_2072_1025 = &ltmp_2071_103D->field0.field1.field6;
  ltmp_2073_13 = *ltmp_2072_1025;
  *ltmp_2072_1025 = (&ltmp_2073_13[PLZO_INT64_C(1)]);
  *ltmp_2073_13 = 0;
  return;
}


static plzo_uint32_t ltmp_37_1015(struct l_struct_2E_Lzo1E *ltmp_2074_103D) {

  return 0u;
}


static plzo_int32_t ltmp_38_1016(struct l_struct_2E_E *ltmp_2075_1023, struct l_struct_2E_A7 *ltmp_2076_1038) {
  plzo_uint32_t ltmp_2077_7;

  ltmp_2077_7 = *(&ltmp_2076_1038->field0);
  return ((plzo_int32_t )(ltmp_2077_7 == 0u));
}


static plzo_int32_t ltmp_39_1016(struct l_struct_2E_E *ltmp_2078_1023, struct l_struct_2E_A7 *ltmp_2079_1038) {
  plzo_uint32_t ltmp_2080_7;

  ltmp_2080_7 = *(&ltmp_2079_1038->field0);
  return ((plzo_int32_t )(ltmp_2080_7 < 3u));
}


static void ltmp_40_1001(struct l_struct_2E_A5 *ltmp_2081_1024, plzo_uint32_t ltmp_2082_7) {
  plzo_uint32_t ltmp_2083_7;
  plzo_uint32_t ltmp_2084_7;
  plzo_uint32_t ltmp_2084_7_2E;
  plzo_uint32_t ltmp_2085_7;
  plzo_uint32_t ltmp_2086_7;
  plzo_uint32_t ltmp_2086_7_2E;
  plzo_uint32_t ltmp_2087_7;
  plzo_uint32_t ltmp_2088_7;
  plzo_uint32_t ltmp_2088_7_2E;
  plzo_uint32_t ltmp_2089_7;
  plzo_uint32_t ltmp_2090_7;
  plzo_uint32_t ltmp_2090_7_2E;

  if ((ltmp_2082_7 > 1u)) {
    goto ltmp_2091_D;
  } else {
    ltmp_2090_7_2E = ltmp_2082_7;
    goto ltmp_2092_D;
  }

ltmp_2091_D:
  ltmp_2083_7 = ltmp_2082_7 + 2u;
  if ((ltmp_2083_7 < 8u)) {
    ltmp_2086_7_2E = 4u;
    goto ltmp_2093_D;
  } else {
    ltmp_2084_7_2E = 8u;
    goto ltmp_2094_D;
  }

  do {
ltmp_2094_D:
  ltmp_2084_7 = ltmp_2084_7_2E;
  ltmp_2085_7 = ltmp_2084_7 << 1;
  if ((ltmp_2083_7 < ltmp_2085_7)) {
    ltmp_2086_7_2E = ltmp_2084_7;
    goto ltmp_2093_D;
  } else {
    ltmp_2084_7_2E = ltmp_2085_7;
    goto ltmp_2094_D;
  }

  } while (1);
ltmp_2093_D:
  ltmp_2086_7 = ltmp_2086_7_2E;
  ltmp_2087_7 = ltmp_2086_7 & 2147483647u;
  ltmp_2088_7_2E = ltmp_2087_7;
  goto ltmp_2095_D;

  do {
ltmp_2095_D:
  ltmp_2088_7 = ltmp_2088_7_2E;
  ltmp_2089_7 = ltmp_2088_7 >> 1;
  ltmp_64_1001(ltmp_2081_1024, ((plzo_uint32_t )((ltmp_2089_7 & ltmp_2083_7) != 0u)));
  ltmp_64_1001(ltmp_2081_1024, 0u);
  if ((ltmp_2089_7 > 2u)) {
    ltmp_2088_7_2E = ltmp_2089_7;
    goto ltmp_2095_D;
  } else {
    ltmp_2090_7_2E = ltmp_2083_7;
    goto ltmp_2092_D;
  }

  } while (1);
ltmp_2092_D:
  ltmp_2090_7 = ltmp_2090_7_2E;
  ltmp_64_1001(ltmp_2081_1024, (ltmp_2090_7 & 1u));
  ltmp_64_1001(ltmp_2081_1024, 1u);
  return;
}


static void ltmp_41_1001(struct l_struct_2E_A5 *ltmp_2096_1024, plzo_uint32_t ltmp_2097_7) {
  plzo_uint32_t ltmp_2098_7;
  plzo_uint32_t ltmp_2099_7;
  plzo_uint32_t ltmp_2100_7;
  plzo_uint32_t ltmp_2101_7;
  plzo_uint32_t ltmp_2101_7_2E;
  plzo_uint32_t ltmp_2102_7;
  plzo_uint32_t ltmp_2103_7;
  plzo_uint32_t ltmp_2103_7_2E;
  plzo_uint32_t ltmp_2104_7;
  plzo_uint32_t ltmp_2105_7;
  plzo_uint32_t ltmp_2105_7_2E;
  plzo_uint32_t ltmp_2106_7;
  plzo_uint32_t ltmp_2107_7;
  plzo_uint32_t ltmp_2107_7_2E;

  if ((ltmp_2097_7 > 2u)) {
    goto ltmp_2108_D;
  } else {
    goto ltmp_2109_D;
  }

ltmp_2109_D:
  ltmp_2098_7 = ltmp_2097_7 + 1u;
  ltmp_64_1001(ltmp_2096_1024, ((plzo_uint32_t )(ltmp_2098_7 > 1u)));
  ltmp_64_1001(ltmp_2096_1024, (ltmp_2098_7 & 1u));
  return;
ltmp_2108_D:
  ltmp_2099_7 = ltmp_2097_7 + 4294967293u;
  ltmp_64_1001(ltmp_2096_1024, 0u);
  ltmp_64_1001(ltmp_2096_1024, 0u);
  if ((ltmp_2099_7 > 1u)) {
    goto ltmp_2110_D;
  } else {
    ltmp_2107_7_2E = ltmp_2099_7;
    goto ltmp_2111_D;
  }

ltmp_2110_D:
  ltmp_2100_7 = ltmp_2097_7 + 4294967295u;
  if ((ltmp_2100_7 < 8u)) {
    ltmp_2103_7_2E = 4u;
    goto ltmp_2112_D;
  } else {
    ltmp_2101_7_2E = 8u;
    goto ltmp_2113_D;
  }

  do {
ltmp_2113_D:
  ltmp_2101_7 = ltmp_2101_7_2E;
  ltmp_2102_7 = ltmp_2101_7 << 1;
  if ((ltmp_2100_7 < ltmp_2102_7)) {
    ltmp_2103_7_2E = ltmp_2101_7;
    goto ltmp_2112_D;
  } else {
    ltmp_2101_7_2E = ltmp_2102_7;
    goto ltmp_2113_D;
  }

  } while (1);
ltmp_2112_D:
  ltmp_2103_7 = ltmp_2103_7_2E;
  ltmp_2104_7 = ltmp_2103_7 & 2147483647u;
  ltmp_2105_7_2E = ltmp_2104_7;
  goto ltmp_2114_D;

  do {
ltmp_2114_D:
  ltmp_2105_7 = ltmp_2105_7_2E;
  ltmp_2106_7 = ltmp_2105_7 >> 1;
  ltmp_64_1001(ltmp_2096_1024, ((plzo_uint32_t )((ltmp_2106_7 & ltmp_2100_7) != 0u)));
  ltmp_64_1001(ltmp_2096_1024, 0u);
  if ((ltmp_2106_7 > 2u)) {
    ltmp_2105_7_2E = ltmp_2106_7;
    goto ltmp_2114_D;
  } else {
    ltmp_2107_7_2E = ltmp_2100_7;
    goto ltmp_2111_D;
  }

  } while (1);
ltmp_2111_D:
  ltmp_2107_7 = ltmp_2107_7_2E;
  ltmp_64_1001(ltmp_2096_1024, (ltmp_2107_7 & 1u));
  ltmp_64_1001(ltmp_2096_1024, 1u);
  return;
}


static void ltmp_42_1001(struct l_struct_2E_A5 *ltmp_2115_1024, plzo_uint32_t ltmp_2116_7) {
  plzo_uint32_t ltmp_2117_7;
  plzo_uint32_t ltmp_2118_7;
  plzo_uint32_t ltmp_2118_7_2E;
  plzo_uint32_t ltmp_2119_7;
  plzo_uint32_t ltmp_2120_7;
  plzo_uint32_t ltmp_2120_7_2E;
  plzo_uint32_t ltmp_2121_7;
  plzo_uint32_t ltmp_2122_7;
  plzo_uint32_t ltmp_2122_7_2E;
  plzo_uint32_t ltmp_2123_7;
  plzo_uint32_t ltmp_2124_7;
  plzo_uint32_t ltmp_2124_7_2E;

  if ((ltmp_2116_7 > 1u)) {
    goto ltmp_2125_D;
  } else {
    ltmp_2124_7_2E = ltmp_2116_7;
    goto ltmp_2126_D;
  }

ltmp_2125_D:
  ltmp_2117_7 = ltmp_2116_7 + 2u;
  if ((ltmp_2117_7 < 8u)) {
    ltmp_2120_7_2E = 4u;
    goto ltmp_2127_D;
  } else {
    ltmp_2118_7_2E = 8u;
    goto ltmp_2128_D;
  }

  do {
ltmp_2128_D:
  ltmp_2118_7 = ltmp_2118_7_2E;
  ltmp_2119_7 = ltmp_2118_7 << 1;
  if ((ltmp_2117_7 < ltmp_2119_7)) {
    ltmp_2120_7_2E = ltmp_2118_7;
    goto ltmp_2127_D;
  } else {
    ltmp_2118_7_2E = ltmp_2119_7;
    goto ltmp_2128_D;
  }

  } while (1);
ltmp_2127_D:
  ltmp_2120_7 = ltmp_2120_7_2E;
  ltmp_2121_7 = ltmp_2120_7 & 2147483647u;
  ltmp_2122_7_2E = ltmp_2121_7;
  goto ltmp_2129_D;

  do {
ltmp_2129_D:
  ltmp_2122_7 = ltmp_2122_7_2E;
  ltmp_2123_7 = ltmp_2122_7 >> 1;
  ltmp_66_1001(ltmp_2115_1024, ((plzo_uint32_t )((ltmp_2123_7 & ltmp_2117_7) != 0u)));
  ltmp_66_1001(ltmp_2115_1024, 0u);
  if ((ltmp_2123_7 > 2u)) {
    ltmp_2122_7_2E = ltmp_2123_7;
    goto ltmp_2129_D;
  } else {
    ltmp_2124_7_2E = ltmp_2117_7;
    goto ltmp_2126_D;
  }

  } while (1);
ltmp_2126_D:
  ltmp_2124_7 = ltmp_2124_7_2E;
  ltmp_66_1001(ltmp_2115_1024, (ltmp_2124_7 & 1u));
  ltmp_66_1001(ltmp_2115_1024, 1u);
  return;
}


static void ltmp_43_1001(struct l_struct_2E_A5 *ltmp_2130_1024, plzo_uint32_t ltmp_2131_7) {
  plzo_uint32_t ltmp_2132_7;
  plzo_uint32_t ltmp_2133_7;
  plzo_uint32_t ltmp_2134_7;
  plzo_uint32_t ltmp_2135_7;
  plzo_uint32_t ltmp_2135_7_2E;
  plzo_uint32_t ltmp_2136_7;
  plzo_uint32_t ltmp_2137_7;
  plzo_uint32_t ltmp_2137_7_2E;
  plzo_uint32_t ltmp_2138_7;
  plzo_uint32_t ltmp_2139_7;
  plzo_uint32_t ltmp_2139_7_2E;
  plzo_uint32_t ltmp_2140_7;
  plzo_uint32_t ltmp_2141_7;
  plzo_uint32_t ltmp_2141_7_2E;

  if ((ltmp_2131_7 > 2u)) {
    goto ltmp_2142_D;
  } else {
    goto ltmp_2143_D;
  }

ltmp_2143_D:
  ltmp_2132_7 = ltmp_2131_7 + 1u;
  ltmp_66_1001(ltmp_2130_1024, ((plzo_uint32_t )(ltmp_2132_7 > 1u)));
  ltmp_66_1001(ltmp_2130_1024, (ltmp_2132_7 & 1u));
  return;
ltmp_2142_D:
  ltmp_2133_7 = ltmp_2131_7 + 4294967293u;
  ltmp_66_1001(ltmp_2130_1024, 0u);
  ltmp_66_1001(ltmp_2130_1024, 0u);
  if ((ltmp_2133_7 > 1u)) {
    goto ltmp_2144_D;
  } else {
    ltmp_2141_7_2E = ltmp_2133_7;
    goto ltmp_2145_D;
  }

ltmp_2144_D:
  ltmp_2134_7 = ltmp_2131_7 + 4294967295u;
  if ((ltmp_2134_7 < 8u)) {
    ltmp_2137_7_2E = 4u;
    goto ltmp_2146_D;
  } else {
    ltmp_2135_7_2E = 8u;
    goto ltmp_2147_D;
  }

  do {
ltmp_2147_D:
  ltmp_2135_7 = ltmp_2135_7_2E;
  ltmp_2136_7 = ltmp_2135_7 << 1;
  if ((ltmp_2134_7 < ltmp_2136_7)) {
    ltmp_2137_7_2E = ltmp_2135_7;
    goto ltmp_2146_D;
  } else {
    ltmp_2135_7_2E = ltmp_2136_7;
    goto ltmp_2147_D;
  }

  } while (1);
ltmp_2146_D:
  ltmp_2137_7 = ltmp_2137_7_2E;
  ltmp_2138_7 = ltmp_2137_7 & 2147483647u;
  ltmp_2139_7_2E = ltmp_2138_7;
  goto ltmp_2148_D;

  do {
ltmp_2148_D:
  ltmp_2139_7 = ltmp_2139_7_2E;
  ltmp_2140_7 = ltmp_2139_7 >> 1;
  ltmp_66_1001(ltmp_2130_1024, ((plzo_uint32_t )((ltmp_2140_7 & ltmp_2134_7) != 0u)));
  ltmp_66_1001(ltmp_2130_1024, 0u);
  if ((ltmp_2140_7 > 2u)) {
    ltmp_2139_7_2E = ltmp_2140_7;
    goto ltmp_2148_D;
  } else {
    ltmp_2141_7_2E = ltmp_2134_7;
    goto ltmp_2145_D;
  }

  } while (1);
ltmp_2145_D:
  ltmp_2141_7 = ltmp_2141_7_2E;
  ltmp_66_1001(ltmp_2130_1024, (ltmp_2141_7 & 1u));
  ltmp_66_1001(ltmp_2130_1024, 1u);
  return;
}


static void ltmp_44_1001(struct l_struct_2E_A5 *ltmp_2149_1024, plzo_uint32_t ltmp_2150_7) {
  plzo_uint32_t ltmp_2151_7;
  plzo_uint32_t ltmp_2151_7_2E;
  plzo_uint32_t ltmp_2152_7;
  plzo_uint32_t ltmp_2152_7_2E;
  plzo_uint32_t ltmp_2153_7;
  plzo_uint32_t ltmp_2154_7;
  plzo_uint32_t ltmp_2155_7;
  plzo_uint32_t ltmp_2155_7_2E;
  plzo_uint32_t ltmp_2156_7;
  plzo_uint32_t ltmp_2157_7;
  plzo_uint32_t ltmp_2157_7_2E;

  if ((ltmp_2150_7 > 1u)) {
    ltmp_2151_7_2E = 2u;
    ltmp_2152_7_2E = ltmp_2150_7;
    goto ltmp_2158_D;
  } else {
    ltmp_2157_7_2E = ltmp_2150_7;
    goto ltmp_2159_D;
  }

  do {
ltmp_2158_D:
  ltmp_2151_7 = ltmp_2151_7_2E;
  ltmp_2152_7 = ltmp_2152_7_2E;
  ltmp_2153_7 = ltmp_2152_7 - ltmp_2151_7;
  ltmp_2154_7 = ltmp_2151_7 << 2;
  if ((ltmp_2153_7 < ltmp_2154_7)) {
    ltmp_2155_7_2E = ltmp_2154_7;
    goto ltmp_2160_D;
  } else {
    ltmp_2151_7_2E = ltmp_2154_7;
    ltmp_2152_7_2E = ltmp_2153_7;
    goto ltmp_2158_D;
  }

  } while (1);
  do {
ltmp_2160_D:
  ltmp_2155_7 = ltmp_2155_7_2E;
  ltmp_64_1001(ltmp_2149_1024, ((plzo_uint32_t )(((ltmp_2155_7 >> 1) & ltmp_2153_7) != 0u)));
  ltmp_64_1001(ltmp_2149_1024, 0u);
  ltmp_2156_7 = ltmp_2155_7 >> 2;
  ltmp_64_1001(ltmp_2149_1024, ((plzo_uint32_t )((ltmp_2156_7 & ltmp_2153_7) != 0u)));
  if ((ltmp_2156_7 > 2u)) {
    ltmp_2155_7_2E = ltmp_2156_7;
    goto ltmp_2160_D;
  } else {
    ltmp_2157_7_2E = ltmp_2153_7;
    goto ltmp_2159_D;
  }

  } while (1);
ltmp_2159_D:
  ltmp_2157_7 = ltmp_2157_7_2E;
  ltmp_64_1001(ltmp_2149_1024, (ltmp_2157_7 & 1u));
  ltmp_64_1001(ltmp_2149_1024, 1u);
  return;
}


static void ltmp_45_1001(struct l_struct_2E_A5 *ltmp_2161_1024, plzo_uint32_t ltmp_2162_7) {
  plzo_uint32_t ltmp_2163_7;
  plzo_uint32_t ltmp_2163_7_2E;
  plzo_uint32_t ltmp_2164_7;
  plzo_uint32_t ltmp_2164_7_2E;
  plzo_uint32_t ltmp_2165_7;
  plzo_uint32_t ltmp_2166_7;
  plzo_uint32_t ltmp_2167_7;
  plzo_uint32_t ltmp_2167_7_2E;
  plzo_uint32_t ltmp_2168_7;
  plzo_uint32_t ltmp_2169_7;
  plzo_uint32_t ltmp_2169_7_2E;

  if ((ltmp_2162_7 > 1u)) {
    ltmp_2163_7_2E = 2u;
    ltmp_2164_7_2E = ltmp_2162_7;
    goto ltmp_2170_D;
  } else {
    ltmp_2169_7_2E = ltmp_2162_7;
    goto ltmp_2171_D;
  }

  do {
ltmp_2170_D:
  ltmp_2163_7 = ltmp_2163_7_2E;
  ltmp_2164_7 = ltmp_2164_7_2E;
  ltmp_2165_7 = ltmp_2164_7 - ltmp_2163_7;
  ltmp_2166_7 = ltmp_2163_7 << 2;
  if ((ltmp_2165_7 < ltmp_2166_7)) {
    ltmp_2167_7_2E = ltmp_2166_7;
    goto ltmp_2172_D;
  } else {
    ltmp_2163_7_2E = ltmp_2166_7;
    ltmp_2164_7_2E = ltmp_2165_7;
    goto ltmp_2170_D;
  }

  } while (1);
  do {
ltmp_2172_D:
  ltmp_2167_7 = ltmp_2167_7_2E;
  ltmp_66_1001(ltmp_2161_1024, ((plzo_uint32_t )(((ltmp_2167_7 >> 1) & ltmp_2165_7) != 0u)));
  ltmp_66_1001(ltmp_2161_1024, 0u);
  ltmp_2168_7 = ltmp_2167_7 >> 2;
  ltmp_66_1001(ltmp_2161_1024, ((plzo_uint32_t )((ltmp_2168_7 & ltmp_2165_7) != 0u)));
  if ((ltmp_2168_7 > 2u)) {
    ltmp_2167_7_2E = ltmp_2168_7;
    goto ltmp_2172_D;
  } else {
    ltmp_2169_7_2E = ltmp_2165_7;
    goto ltmp_2171_D;
  }

  } while (1);
ltmp_2171_D:
  ltmp_2169_7 = ltmp_2169_7_2E;
  ltmp_66_1001(ltmp_2161_1024, (ltmp_2169_7 & 1u));
  ltmp_66_1001(ltmp_2161_1024, 1u);
  return;
}


static void ltmp_46_1001(struct l_struct_2E_A5 *ltmp_2173_1024, plzo_uint32_t ltmp_2174_7) {
  plzo_uint32_t ltmp_2175_7;
  plzo_uint32_t ltmp_2176_7;
  plzo_uint32_t ltmp_2176_7_2E;
  plzo_uint32_t ltmp_2177_7;
  plzo_uint32_t ltmp_2178_7;
  plzo_uint32_t ltmp_2178_7_2E;
  plzo_uint32_t ltmp_2179_7;
  plzo_uint32_t ltmp_2180_7;
  plzo_uint32_t ltmp_2180_7_2E;
  plzo_uint32_t ltmp_2181_7;
  plzo_uint32_t ltmp_2182_7;
  plzo_uint32_t ltmp_2182_7_2E;

  if ((ltmp_2174_7 > 1u)) {
    goto ltmp_2183_D;
  } else {
    ltmp_2182_7_2E = ltmp_2174_7;
    goto ltmp_2184_D;
  }

ltmp_2183_D:
  ltmp_2175_7 = ltmp_2174_7 + 2u;
  if ((ltmp_2175_7 < 8u)) {
    ltmp_2178_7_2E = 4u;
    goto ltmp_2185_D;
  } else {
    ltmp_2176_7_2E = 8u;
    goto ltmp_2186_D;
  }

  do {
ltmp_2186_D:
  ltmp_2176_7 = ltmp_2176_7_2E;
  ltmp_2177_7 = ltmp_2176_7 << 1;
  if ((ltmp_2175_7 < ltmp_2177_7)) {
    ltmp_2178_7_2E = ltmp_2176_7;
    goto ltmp_2185_D;
  } else {
    ltmp_2176_7_2E = ltmp_2177_7;
    goto ltmp_2186_D;
  }

  } while (1);
ltmp_2185_D:
  ltmp_2178_7 = ltmp_2178_7_2E;
  ltmp_2179_7 = ltmp_2178_7 & 2147483647u;
  ltmp_2180_7_2E = ltmp_2179_7;
  goto ltmp_2187_D;

  do {
ltmp_2187_D:
  ltmp_2180_7 = ltmp_2180_7_2E;
  ltmp_2181_7 = ltmp_2180_7 >> 1;
  ltmp_1_1001(ltmp_2173_1024, ((plzo_uint32_t )((ltmp_2181_7 & ltmp_2175_7) != 0u)));
  ltmp_1_1001(ltmp_2173_1024, 0u);
  if ((ltmp_2181_7 > 2u)) {
    ltmp_2180_7_2E = ltmp_2181_7;
    goto ltmp_2187_D;
  } else {
    ltmp_2182_7_2E = ltmp_2175_7;
    goto ltmp_2184_D;
  }

  } while (1);
ltmp_2184_D:
  ltmp_2182_7 = ltmp_2182_7_2E;
  ltmp_1_1001(ltmp_2173_1024, (ltmp_2182_7 & 1u));
  ltmp_1_1001(ltmp_2173_1024, 1u);
  return;
}


static void ltmp_47_1001(struct l_struct_2E_A5 *ltmp_2188_1024, plzo_uint32_t ltmp_2189_7) {
  plzo_uint32_t ltmp_2190_7;
  plzo_uint32_t ltmp_2190_7_2E;
  plzo_uint32_t ltmp_2191_7;
  plzo_uint32_t ltmp_2191_7_2E;
  plzo_uint32_t ltmp_2192_7;
  plzo_uint32_t ltmp_2193_7;
  plzo_uint32_t ltmp_2194_7;
  plzo_uint32_t ltmp_2194_7_2E;
  plzo_uint32_t ltmp_2195_7;
  plzo_uint32_t ltmp_2196_7;
  plzo_uint32_t ltmp_2196_7_2E;

  if ((ltmp_2189_7 > 1u)) {
    ltmp_2190_7_2E = 2u;
    ltmp_2191_7_2E = ltmp_2189_7;
    goto ltmp_2197_D;
  } else {
    ltmp_2196_7_2E = ltmp_2189_7;
    goto ltmp_2198_D;
  }

  do {
ltmp_2197_D:
  ltmp_2190_7 = ltmp_2190_7_2E;
  ltmp_2191_7 = ltmp_2191_7_2E;
  ltmp_2192_7 = ltmp_2191_7 - ltmp_2190_7;
  ltmp_2193_7 = ltmp_2190_7 << 2;
  if ((ltmp_2192_7 < ltmp_2193_7)) {
    ltmp_2194_7_2E = ltmp_2193_7;
    goto ltmp_2199_D;
  } else {
    ltmp_2190_7_2E = ltmp_2193_7;
    ltmp_2191_7_2E = ltmp_2192_7;
    goto ltmp_2197_D;
  }

  } while (1);
  do {
ltmp_2199_D:
  ltmp_2194_7 = ltmp_2194_7_2E;
  ltmp_1_1001(ltmp_2188_1024, ((plzo_uint32_t )(((ltmp_2194_7 >> 1) & ltmp_2192_7) != 0u)));
  ltmp_1_1001(ltmp_2188_1024, 0u);
  ltmp_2195_7 = ltmp_2194_7 >> 2;
  ltmp_1_1001(ltmp_2188_1024, ((plzo_uint32_t )((ltmp_2195_7 & ltmp_2192_7) != 0u)));
  if ((ltmp_2195_7 > 2u)) {
    ltmp_2194_7_2E = ltmp_2195_7;
    goto ltmp_2199_D;
  } else {
    ltmp_2196_7_2E = ltmp_2192_7;
    goto ltmp_2198_D;
  }

  } while (1);
ltmp_2198_D:
  ltmp_2196_7 = ltmp_2196_7_2E;
  ltmp_1_1001(ltmp_2188_1024, (ltmp_2196_7 & 1u));
  ltmp_1_1001(ltmp_2188_1024, 1u);
  return;
}


static void ltmp_48_1001(struct l_struct_2E_A5 *ltmp_2200_1024, plzo_uint32_t ltmp_2201_7) {
  plzo_uint32_t ltmp_2202_7;
  plzo_uint32_t ltmp_2203_7;
  plzo_uint32_t ltmp_2204_7;
  plzo_uint32_t ltmp_2205_7;
  plzo_uint32_t ltmp_2205_7_2E;
  plzo_uint32_t ltmp_2206_7;
  plzo_uint32_t ltmp_2207_7;
  plzo_uint32_t ltmp_2207_7_2E;
  plzo_uint32_t ltmp_2208_7;
  plzo_uint32_t ltmp_2209_7;
  plzo_uint32_t ltmp_2209_7_2E;
  plzo_uint32_t ltmp_2210_7;
  plzo_uint32_t ltmp_2211_7;
  plzo_uint32_t ltmp_2211_7_2E;

  if ((ltmp_2201_7 > 2u)) {
    goto ltmp_2212_D;
  } else {
    goto ltmp_2213_D;
  }

ltmp_2213_D:
  ltmp_2202_7 = ltmp_2201_7 + 1u;
  ltmp_1_1001(ltmp_2200_1024, ((plzo_uint32_t )(ltmp_2202_7 > 1u)));
  ltmp_1_1001(ltmp_2200_1024, (ltmp_2202_7 & 1u));
  return;
ltmp_2212_D:
  ltmp_2203_7 = ltmp_2201_7 + 4294967293u;
  ltmp_1_1001(ltmp_2200_1024, 0u);
  ltmp_1_1001(ltmp_2200_1024, 0u);
  if ((ltmp_2203_7 > 1u)) {
    goto ltmp_2214_D;
  } else {
    ltmp_2211_7_2E = ltmp_2203_7;
    goto ltmp_2215_D;
  }

ltmp_2214_D:
  ltmp_2204_7 = ltmp_2201_7 + 4294967295u;
  if ((ltmp_2204_7 < 8u)) {
    ltmp_2207_7_2E = 4u;
    goto ltmp_2216_D;
  } else {
    ltmp_2205_7_2E = 8u;
    goto ltmp_2217_D;
  }

  do {
ltmp_2217_D:
  ltmp_2205_7 = ltmp_2205_7_2E;
  ltmp_2206_7 = ltmp_2205_7 << 1;
  if ((ltmp_2204_7 < ltmp_2206_7)) {
    ltmp_2207_7_2E = ltmp_2205_7;
    goto ltmp_2216_D;
  } else {
    ltmp_2205_7_2E = ltmp_2206_7;
    goto ltmp_2217_D;
  }

  } while (1);
ltmp_2216_D:
  ltmp_2207_7 = ltmp_2207_7_2E;
  ltmp_2208_7 = ltmp_2207_7 & 2147483647u;
  ltmp_2209_7_2E = ltmp_2208_7;
  goto ltmp_2218_D;

  do {
ltmp_2218_D:
  ltmp_2209_7 = ltmp_2209_7_2E;
  ltmp_2210_7 = ltmp_2209_7 >> 1;
  ltmp_1_1001(ltmp_2200_1024, ((plzo_uint32_t )((ltmp_2210_7 & ltmp_2204_7) != 0u)));
  ltmp_1_1001(ltmp_2200_1024, 0u);
  if ((ltmp_2210_7 > 2u)) {
    ltmp_2209_7_2E = ltmp_2210_7;
    goto ltmp_2218_D;
  } else {
    ltmp_2211_7_2E = ltmp_2204_7;
    goto ltmp_2215_D;
  }

  } while (1);
ltmp_2215_D:
  ltmp_2211_7 = ltmp_2211_7_2E;
  ltmp_1_1001(ltmp_2200_1024, (ltmp_2211_7 & 1u));
  ltmp_1_1001(ltmp_2200_1024, 1u);
  return;
}


static signed char *ltmp_49_1018(struct plzo_callback_t *ltmp_2219_1026, plzo_uint64_t ltmp_2220_9, plzo_uint64_t ltmp_2221_9) {
  signed char *ltmp_2222_14;
  struct plzo_callback_t *ltmp_2223_1026;
  signed char * (*ltmp_2224_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_2225_14;

  ltmp_2222_14 = *(&ltmp_2219_1026->field3);
  ltmp_2223_1026 = (struct plzo_callback_t *)ltmp_2222_14;
  if ((ltmp_2222_14 == ((signed char *)0))) {
    goto ltmp_2226_D;
  } else {
    goto ltmp_2227_D;
  }

ltmp_2227_D:
  ltmp_2224_1018 = *(&ltmp_2223_1026->field0);
  if ((ltmp_2224_1018 == ((signed char * (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t ))0))) {
    goto ltmp_2226_D;
  } else {
    goto ltmp_2228_D;
  }

ltmp_2228_D:
  ltmp_2225_14 = ltmp_2224_1018(ltmp_2223_1026, ltmp_2220_9, ltmp_2221_9);
  return ltmp_2225_14;
ltmp_2226_D:
  return ((signed char *)0);
}


static void ltmp_50_1019(struct plzo_callback_t *ltmp_2229_1026, signed char *ltmp_2230_14) {
  signed char *ltmp_2231_14;
  struct plzo_callback_t *ltmp_2232_1026;
  void  (*ltmp_2233_1019) (struct plzo_callback_t *, signed char *);

  ltmp_2231_14 = *(&ltmp_2229_1026->field3);
  ltmp_2232_1026 = (struct plzo_callback_t *)ltmp_2231_14;
  if (((ltmp_2231_14 == ((signed char *)0)) | (ltmp_2230_14 == ((signed char *)0)))) {
    goto ltmp_2234_D;
  } else {
    goto ltmp_2235_D;
  }

ltmp_2235_D:
  ltmp_2233_1019 = *(&ltmp_2232_1026->field1);
  if ((ltmp_2233_1019 == ((void  (*) (struct plzo_callback_t *, signed char *))0))) {
    goto ltmp_2234_D;
  } else {
    goto ltmp_2236_D;
  }

ltmp_2236_D:
  ltmp_2233_1019(ltmp_2232_1026, ltmp_2230_14);
  return;
ltmp_2234_D:
  return;
}


static void ltmp_51_101A(struct plzo_callback_t *ltmp_2237_1026, plzo_uint64_t ltmp_2238_9, plzo_uint64_t ltmp_2239_9, plzo_int32_t ltmp_2240_8) {
  signed char *ltmp_2241_14;
  struct plzo_callback_t *ltmp_2242_1026;
  void  (*ltmp_2243_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t );

  ltmp_2241_14 = *(&ltmp_2237_1026->field3);
  ltmp_2242_1026 = (struct plzo_callback_t *)ltmp_2241_14;
  if ((ltmp_2241_14 == ((signed char *)0))) {
    goto ltmp_2244_D;
  } else {
    goto ltmp_2245_D;
  }

ltmp_2245_D:
  ltmp_2243_101A = *(&ltmp_2242_1026->field2);
  if ((ltmp_2243_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int32_t ))0))) {
    goto ltmp_2244_D;
  } else {
    goto ltmp_2246_D;
  }

ltmp_2246_D:
  ltmp_2243_101A(ltmp_2242_1026, ltmp_2238_9, ltmp_2239_9, ltmp_2240_8);
  return;
ltmp_2244_D:
  return;
}


static void ltmp_52_101B(struct l_struct_2E_E *ltmp_2247_1023, struct l_struct_2E_E_3A__3A_A14 *ltmp_2248_1028) {
  void  (*ltmp_2249_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  plzo_uint32_t *ltmp_2250_17;
  plzo_uint32_t ltmp_2251_7;
  unsigned char **ltmp_2252_1025;
  plzo_uint64_t ltmp_2253_9;
  signed char *ltmp_2254_14;
  struct plzo_callback_t *ltmp_2255_1026;
  signed char * (*ltmp_2256_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_2257_14;

  ltmp_2249_1001 = *(&ltmp_2248_1028->field3);
  if ((ltmp_2249_1001 == ((void  (*) (struct l_struct_2E_A5 *, plzo_uint32_t ))0))) {
    goto ltmp_2258_D;
  } else {
    goto ltmp_2259_D;
  }

ltmp_2259_D:
  ltmp_2250_17 = &ltmp_2248_1028->field1;
  ltmp_2251_7 = *ltmp_2250_17;
  if ((ltmp_2251_7 == 0u)) {
    goto ltmp_2258_D;
  } else {
    goto ltmp_2260_D;
  }

ltmp_2260_D:
  ltmp_2252_1025 = &ltmp_2248_1028->field0;
  ltmp_2253_9 = (plzo_uint64_t )ltmp_2251_7;
  ltmp_2254_14 = *(&ltmp_2247_1023->field24.field2.field3);
  ltmp_2255_1026 = (struct plzo_callback_t *)ltmp_2254_14;
  if ((ltmp_2254_14 == ((signed char *)0))) {
    goto ltmp_2261_D;
  } else {
    goto ltmp_2262_D;
  }

ltmp_2262_D:
  ltmp_2256_1018 = *(&ltmp_2255_1026->field0);
  if ((ltmp_2256_1018 == ((signed char * (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t ))0))) {
    goto ltmp_2261_D;
  } else {
    goto ltmp_2263_D;
  }

ltmp_2263_D:
  ltmp_2257_14 = ltmp_2256_1018(ltmp_2255_1026, ltmp_2253_9, PLZO_UINT64_C(1));
  *ltmp_2252_1025 = ((unsigned char *)ltmp_2257_14);
  if ((ltmp_2257_14 == ((signed char *)0))) {
    goto ltmp_2264_D;
  } else {
    goto ltmp_2258_D;
  }

ltmp_2261_D:
  *ltmp_2252_1025 = ((unsigned char *)0);
  goto ltmp_2264_D;

ltmp_2264_D:
  *ltmp_2250_17 = 0u;
  return;
ltmp_2258_D:
  return;
}


static void ltmp_53_101C(struct l_struct_2E_E_3A__3A_A14 *ltmp_2265_1028) {
  plzo_uint32_t  (**ltmp_2266_1040) (plzo_uint32_t );
  plzo_uint32_t  (*ltmp_2267_1041) (plzo_uint32_t );
  unsigned char **ltmp_2268_1025;
  unsigned char *ltmp_2269_13;
  plzo_uint32_t *ltmp_2270_17;
  plzo_uint32_t ltmp_2271_7;
  plzo_uint32_t ltmp_2272_7;
  plzo_uint32_t ltmp_2272_7_2E;
  plzo_uint32_t  (*ltmp_2273_1041) (plzo_uint32_t );
  plzo_uint32_t ltmp_2274_7;
  unsigned char *ltmp_2275_13;
  plzo_uint32_t ltmp_2276_7;
  plzo_uint32_t ltmp_2277_7;

  ltmp_2266_1040 = &ltmp_2265_1028->field2;
  ltmp_2267_1041 = *ltmp_2266_1040;
  if ((ltmp_2267_1041 == ((plzo_uint32_t  (*) (plzo_uint32_t ))0))) {
    goto ltmp_2278_D;
  } else {
    goto ltmp_2279_D;
  }

ltmp_2279_D:
  ltmp_2268_1025 = &ltmp_2265_1028->field0;
  ltmp_2269_13 = *ltmp_2268_1025;
  if ((ltmp_2269_13 == ((unsigned char *)0))) {
    goto ltmp_2278_D;
  } else {
    goto ltmp_2280_D;
  }

ltmp_2280_D:
  ltmp_2270_17 = &ltmp_2265_1028->field1;
  ltmp_2271_7 = *ltmp_2270_17;
  if ((ltmp_2271_7 == 0u)) {
    goto ltmp_2281_D;
  } else {
    ltmp_2272_7_2E = 0u;
    goto ltmp_2282_D;
  }

  do {
ltmp_2282_D:
  ltmp_2272_7 = ltmp_2272_7_2E;
  ltmp_2273_1041 = *ltmp_2266_1040;
  ltmp_2274_7 = ltmp_2273_1041(ltmp_2272_7);
  ltmp_2275_13 = *ltmp_2268_1025;
  *(&ltmp_2275_13[((plzo_int64_t )ltmp_2272_7)]) = ((unsigned char )ltmp_2274_7);
  ltmp_2276_7 = ltmp_2272_7 + 1u;
  ltmp_2277_7 = *ltmp_2270_17;
  if ((ltmp_2277_7 > ltmp_2276_7)) {
    ltmp_2272_7_2E = ltmp_2276_7;
    goto ltmp_2282_D;
  } else {
    goto ltmp_2281_D;
  }

  } while (1);
ltmp_2278_D:
  *(&ltmp_2265_1028->field1) = 0u;
  return;
ltmp_2281_D:
  return;
}


static void  (*ltmp_54_101D(void  (*ltmp_2283_1001) (struct l_struct_2E_A5 *, plzo_uint32_t ), plzo_uint32_t ltmp_2284_7, plzo_uint32_t ltmp_2285_7)) (struct l_struct_2E_A5 *, plzo_uint32_t ) {
  void  (*ltmp_2286_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (*ltmp_2286_1001_2E) (struct l_struct_2E_A5 *, plzo_uint32_t );

  if ((ltmp_2283_1001 == ltmp_46_1001)) {
    goto ltmp_2287_D;
  } else {
    goto ltmp_2288_D;
  }

ltmp_2287_D:
  if ((ltmp_2285_7 == 8u)) {
    ltmp_2286_1001_2E = ltmp_40_1001;
    goto ltmp_2289_D;
  } else {
    goto ltmp_2290_D;
  }

ltmp_2290_D:
  if (((ltmp_2285_7 == 32u) & (ltmp_2284_7 == 0u))) {
    ltmp_2286_1001_2E = ltmp_42_1001;
    goto ltmp_2289_D;
  } else {
    goto ltmp_2288_D;
  }

ltmp_2288_D:
  if ((ltmp_2283_1001 == ltmp_47_1001)) {
    goto ltmp_2291_D;
  } else {
    goto ltmp_2292_D;
  }

ltmp_2291_D:
  if ((ltmp_2285_7 == 8u)) {
    ltmp_2286_1001_2E = ltmp_44_1001;
    goto ltmp_2289_D;
  } else {
    goto ltmp_2293_D;
  }

ltmp_2293_D:
  if (((ltmp_2285_7 == 32u) & (ltmp_2284_7 == 0u))) {
    ltmp_2286_1001_2E = ltmp_45_1001;
    goto ltmp_2289_D;
  } else {
    goto ltmp_2292_D;
  }

ltmp_2292_D:
  if ((ltmp_2283_1001 == ltmp_48_1001)) {
    goto ltmp_2294_D;
  } else {
    goto ltmp_2295_D;
  }

ltmp_2294_D:
  if ((ltmp_2285_7 == 8u)) {
    ltmp_2286_1001_2E = ltmp_41_1001;
    goto ltmp_2289_D;
  } else {
    goto ltmp_2296_D;
  }

ltmp_2296_D:
  if (((ltmp_2285_7 == 32u) & (ltmp_2284_7 == 0u))) {
    ltmp_2286_1001_2E = ltmp_43_1001;
    goto ltmp_2289_D;
  } else {
    goto ltmp_2295_D;
  }

ltmp_2295_D:
  if ((ltmp_2283_1001 == ltmp_63_1001)) {
    goto ltmp_2297_D;
  } else {
    ltmp_2286_1001_2E = ltmp_2283_1001;
    goto ltmp_2289_D;
  }

ltmp_2297_D:
  if ((ltmp_2285_7 == 8u)) {
    ltmp_2286_1001_2E = ltmp_65_1001;
    goto ltmp_2289_D;
  } else {
    goto ltmp_2298_D;
  }

ltmp_2298_D:
  return (((((ltmp_2285_7 == 32u) & (ltmp_2284_7 == 0u))) ? (ltmp_67_1001) : (ltmp_2283_1001)));
ltmp_2289_D:
  ltmp_2286_1001 = ltmp_2286_1001_2E;
  return ltmp_2286_1001;
}


static plzo_int32_t ltmp_55_1007(struct l_struct_2E_E *ltmp_2299_1023) {

  return 0;
}


static void ltmp_56_101E(struct l_struct_2E_E *ltmp_2300_1023, unsigned char *ltmp_2301_13, plzo_uint32_t ltmp_2302_7, plzo_uint32_t ltmp_2303_7) {

  goto ltmp_2304_D;

  do {
ltmp_2304_D:
  goto ltmp_2304_D;

  } while (1);
}


static void ltmp_57_1000(struct l_struct_2E_E *ltmp_2305_1023, plzo_uint32_t ltmp_2306_7, plzo_uint32_t ltmp_2307_7) {

  goto ltmp_2308_D;

  do {
ltmp_2308_D:
  goto ltmp_2308_D;

  } while (1);
}


static void ltmp_58_1005(struct l_struct_2E_E *ltmp_2309_1023) {

  goto ltmp_2310_D;

  do {
ltmp_2310_D:
  goto ltmp_2310_D;

  } while (1);
}


static void ltmp_59_101F(struct l_struct_2E_E *ltmp_2311_1023, struct l_struct_2E_E_3A__3A_A4 *ltmp_2312_103B, plzo_uint32_t ltmp_2313_7, plzo_uint32_t ltmp_2314_7, plzo_uint32_t ltmp_2315_7) {

  goto ltmp_2316_D;

  do {
ltmp_2316_D:
  goto ltmp_2316_D;

  } while (1);
}


static plzo_uint32_t ltmp_60_1020(struct l_struct_2E_E *ltmp_2317_1023) {

  return 9u;
}


static plzo_int32_t ltmp_61_1016(struct l_struct_2E_E *ltmp_2318_1023, struct l_struct_2E_A7 *ltmp_2319_1038) {
  plzo_uint32_t ltmp_2320_7;

  ltmp_2320_7 = *(&ltmp_2319_1038->field0);
  return ((plzo_int32_t )(ltmp_2320_7 == 0u));
}


static plzo_int32_t ltmp_62_1016(struct l_struct_2E_E *ltmp_2321_1023, struct l_struct_2E_A7 *ltmp_2322_1038) {
  plzo_uint32_t ltmp_2323_7;

  ltmp_2323_7 = *(&ltmp_2322_1038->field0);
  return ((plzo_int32_t )(ltmp_2323_7 < 3u));
}


static void ltmp_63_1001(struct l_struct_2E_A5 *ltmp_2324_1024, plzo_uint32_t ltmp_2325_7) {
  plzo_uint32_t ltmp_2326_7;
  plzo_uint32_t ltmp_2327_7;
  plzo_uint32_t ltmp_2328_7;
  plzo_uint32_t ltmp_2328_7_2E;
  plzo_uint32_t ltmp_2329_7;
  plzo_uint32_t ltmp_2329_7_2E;
  plzo_uint32_t ltmp_2330_7;
  plzo_uint32_t ltmp_2331_7;
  plzo_uint32_t ltmp_2332_7;
  plzo_uint32_t ltmp_2332_7_2E;
  plzo_uint32_t ltmp_2333_7;
  plzo_uint32_t ltmp_2334_7;
  plzo_uint32_t ltmp_2334_7_2E;

  if ((ltmp_2325_7 > 2u)) {
    goto ltmp_2335_D;
  } else {
    goto ltmp_2336_D;
  }

ltmp_2336_D:
  ltmp_2326_7 = ltmp_2325_7 + 1u;
  ltmp_1_1001(ltmp_2324_1024, ((plzo_uint32_t )(ltmp_2326_7 > 1u)));
  ltmp_1_1001(ltmp_2324_1024, (ltmp_2326_7 & 1u));
  return;
ltmp_2335_D:
  ltmp_2327_7 = ltmp_2325_7 + 4294967293u;
  ltmp_1_1001(ltmp_2324_1024, 0u);
  ltmp_1_1001(ltmp_2324_1024, 0u);
  if ((ltmp_2327_7 > 1u)) {
    ltmp_2328_7_2E = 2u;
    ltmp_2329_7_2E = ltmp_2327_7;
    goto ltmp_2337_D;
  } else {
    ltmp_2334_7_2E = ltmp_2327_7;
    goto ltmp_2338_D;
  }

  do {
ltmp_2337_D:
  ltmp_2328_7 = ltmp_2328_7_2E;
  ltmp_2329_7 = ltmp_2329_7_2E;
  ltmp_2330_7 = ltmp_2329_7 - ltmp_2328_7;
  ltmp_2331_7 = ltmp_2328_7 << 2;
  if ((ltmp_2330_7 < ltmp_2331_7)) {
    ltmp_2332_7_2E = ltmp_2331_7;
    goto ltmp_2339_D;
  } else {
    ltmp_2328_7_2E = ltmp_2331_7;
    ltmp_2329_7_2E = ltmp_2330_7;
    goto ltmp_2337_D;
  }

  } while (1);
  do {
ltmp_2339_D:
  ltmp_2332_7 = ltmp_2332_7_2E;
  ltmp_1_1001(ltmp_2324_1024, ((plzo_uint32_t )(((ltmp_2332_7 >> 1) & ltmp_2330_7) != 0u)));
  ltmp_1_1001(ltmp_2324_1024, 0u);
  ltmp_2333_7 = ltmp_2332_7 >> 2;
  ltmp_1_1001(ltmp_2324_1024, ((plzo_uint32_t )((ltmp_2333_7 & ltmp_2330_7) != 0u)));
  if ((ltmp_2333_7 > 2u)) {
    ltmp_2332_7_2E = ltmp_2333_7;
    goto ltmp_2339_D;
  } else {
    ltmp_2334_7_2E = ltmp_2330_7;
    goto ltmp_2338_D;
  }

  } while (1);
ltmp_2338_D:
  ltmp_2334_7 = ltmp_2334_7_2E;
  ltmp_1_1001(ltmp_2324_1024, (ltmp_2334_7 & 1u));
  ltmp_1_1001(ltmp_2324_1024, 1u);
  return;
}


static void ltmp_64_1001(struct l_struct_2E_A5 *ltmp_2340_1024, plzo_uint32_t ltmp_2341_7) {
  plzo_uint32_t *ltmp_2342_17;
  plzo_uint32_t ltmp_2343_7;
  unsigned char *ltmp_2344_13;
  plzo_uint32_t ltmp_2345_7;
  plzo_uint32_t *ltmp_2346_17;
  plzo_uint32_t ltmp_2347_7;
  plzo_uint32_t ltmp_2348_7;
  unsigned char **ltmp_2349_1025;
  unsigned char *ltmp_2350_13;

  ltmp_2342_17 = &ltmp_2340_1024->field1;
  ltmp_2343_7 = *ltmp_2342_17;
  if ((ltmp_2343_7 == 8u)) {
    goto ltmp_2351_D;
  } else {
    goto ltmp_2352_D;
  }

ltmp_2351_D:
  ltmp_2344_13 = *(&ltmp_2340_1024->field5);
  ltmp_2345_7 = *(&ltmp_2340_1024->field0);
  *ltmp_2344_13 = ((unsigned char )ltmp_2345_7);
  *ltmp_2342_17 = 0u;
  goto ltmp_2352_D;

ltmp_2352_D:
  ltmp_2346_17 = &ltmp_2340_1024->field0;
  ltmp_2347_7 = *ltmp_2346_17;
  *ltmp_2346_17 = ((ltmp_2347_7 << 1) + ltmp_2341_7);
  ltmp_2348_7 = *ltmp_2342_17;
  *ltmp_2342_17 = (ltmp_2348_7 + 1u);
  if ((ltmp_2348_7 == 0u)) {
    goto ltmp_2353_D;
  } else {
    goto ltmp_2354_D;
  }

ltmp_2353_D:
  ltmp_2349_1025 = &ltmp_2340_1024->field6;
  ltmp_2350_13 = *ltmp_2349_1025;
  *(&ltmp_2340_1024->field5) = ltmp_2350_13;
  *ltmp_2349_1025 = (&ltmp_2350_13[PLZO_INT64_C(1)]);
  return;
ltmp_2354_D:
  return;
}


static void ltmp_65_1001(struct l_struct_2E_A5 *ltmp_2355_1024, plzo_uint32_t ltmp_2356_7) {
  plzo_uint32_t ltmp_2357_7;
  plzo_uint32_t ltmp_2358_7;
  plzo_uint32_t ltmp_2359_7;
  plzo_uint32_t ltmp_2359_7_2E;
  plzo_uint32_t ltmp_2360_7;
  plzo_uint32_t ltmp_2360_7_2E;
  plzo_uint32_t ltmp_2361_7;
  plzo_uint32_t ltmp_2362_7;
  plzo_uint32_t ltmp_2363_7;
  plzo_uint32_t ltmp_2363_7_2E;
  plzo_uint32_t ltmp_2364_7;
  plzo_uint32_t ltmp_2365_7;
  plzo_uint32_t ltmp_2365_7_2E;

  if ((ltmp_2356_7 > 2u)) {
    goto ltmp_2366_D;
  } else {
    goto ltmp_2367_D;
  }

ltmp_2367_D:
  ltmp_2357_7 = ltmp_2356_7 + 1u;
  ltmp_64_1001(ltmp_2355_1024, ((plzo_uint32_t )(ltmp_2357_7 > 1u)));
  ltmp_64_1001(ltmp_2355_1024, (ltmp_2357_7 & 1u));
  return;
ltmp_2366_D:
  ltmp_2358_7 = ltmp_2356_7 + 4294967293u;
  ltmp_64_1001(ltmp_2355_1024, 0u);
  ltmp_64_1001(ltmp_2355_1024, 0u);
  if ((ltmp_2358_7 > 1u)) {
    ltmp_2359_7_2E = 2u;
    ltmp_2360_7_2E = ltmp_2358_7;
    goto ltmp_2368_D;
  } else {
    ltmp_2365_7_2E = ltmp_2358_7;
    goto ltmp_2369_D;
  }

  do {
ltmp_2368_D:
  ltmp_2359_7 = ltmp_2359_7_2E;
  ltmp_2360_7 = ltmp_2360_7_2E;
  ltmp_2361_7 = ltmp_2360_7 - ltmp_2359_7;
  ltmp_2362_7 = ltmp_2359_7 << 2;
  if ((ltmp_2361_7 < ltmp_2362_7)) {
    ltmp_2363_7_2E = ltmp_2362_7;
    goto ltmp_2370_D;
  } else {
    ltmp_2359_7_2E = ltmp_2362_7;
    ltmp_2360_7_2E = ltmp_2361_7;
    goto ltmp_2368_D;
  }

  } while (1);
  do {
ltmp_2370_D:
  ltmp_2363_7 = ltmp_2363_7_2E;
  ltmp_64_1001(ltmp_2355_1024, ((plzo_uint32_t )(((ltmp_2363_7 >> 1) & ltmp_2361_7) != 0u)));
  ltmp_64_1001(ltmp_2355_1024, 0u);
  ltmp_2364_7 = ltmp_2363_7 >> 2;
  ltmp_64_1001(ltmp_2355_1024, ((plzo_uint32_t )((ltmp_2364_7 & ltmp_2361_7) != 0u)));
  if ((ltmp_2364_7 > 2u)) {
    ltmp_2363_7_2E = ltmp_2364_7;
    goto ltmp_2370_D;
  } else {
    ltmp_2365_7_2E = ltmp_2361_7;
    goto ltmp_2369_D;
  }

  } while (1);
ltmp_2369_D:
  ltmp_2365_7 = ltmp_2365_7_2E;
  ltmp_64_1001(ltmp_2355_1024, (ltmp_2365_7 & 1u));
  ltmp_64_1001(ltmp_2355_1024, 1u);
  return;
}


static void ltmp_66_1001(struct l_struct_2E_A5 *ltmp_2371_1024, plzo_uint32_t ltmp_2372_7) {
  plzo_uint32_t *ltmp_2373_17;
  plzo_uint32_t ltmp_2374_7;
  plzo_uint32_t ltmp_2375_7;
  unsigned char **ltmp_2376_1025;
  unsigned char *ltmp_2377_13;
  unsigned char *ltmp_2378_13;
  unsigned char *ltmp_2379_13;
  unsigned char *ltmp_2380_13;
  plzo_uint32_t *ltmp_2381_17;
  plzo_uint32_t ltmp_2382_7;
  plzo_uint32_t ltmp_2383_7;
  unsigned char **ltmp_2384_1025;
  unsigned char *ltmp_2385_13;

  ltmp_2373_17 = &ltmp_2371_1024->field1;
  ltmp_2374_7 = *ltmp_2373_17;
  if ((ltmp_2374_7 == 32u)) {
    goto ltmp_2386_D;
  } else {
    goto ltmp_2387_D;
  }

ltmp_2386_D:
  ltmp_2375_7 = *(&ltmp_2371_1024->field0);
  ltmp_2376_1025 = &ltmp_2371_1024->field5;
  ltmp_2377_13 = *ltmp_2376_1025;
  *ltmp_2377_13 = ((unsigned char )ltmp_2375_7);
  ltmp_2378_13 = *ltmp_2376_1025;
  *(&ltmp_2378_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_2375_7 >> 8));
  ltmp_2379_13 = *ltmp_2376_1025;
  *(&ltmp_2379_13[PLZO_INT64_C(2)]) = ((unsigned char )(ltmp_2375_7 >> 16));
  ltmp_2380_13 = *ltmp_2376_1025;
  *(&ltmp_2380_13[PLZO_INT64_C(3)]) = ((unsigned char )(ltmp_2375_7 >> 24));
  *ltmp_2373_17 = 0u;
  goto ltmp_2387_D;

ltmp_2387_D:
  ltmp_2381_17 = &ltmp_2371_1024->field0;
  ltmp_2382_7 = *ltmp_2381_17;
  *ltmp_2381_17 = ((ltmp_2382_7 << 1) + ltmp_2372_7);
  ltmp_2383_7 = *ltmp_2373_17;
  *ltmp_2373_17 = (ltmp_2383_7 + 1u);
  if ((ltmp_2383_7 == 0u)) {
    goto ltmp_2388_D;
  } else {
    goto ltmp_2389_D;
  }

ltmp_2388_D:
  ltmp_2384_1025 = &ltmp_2371_1024->field6;
  ltmp_2385_13 = *ltmp_2384_1025;
  *(&ltmp_2371_1024->field5) = ltmp_2385_13;
  *ltmp_2384_1025 = (&ltmp_2385_13[PLZO_INT64_C(4)]);
  return;
ltmp_2389_D:
  return;
}


static void ltmp_67_1001(struct l_struct_2E_A5 *ltmp_2390_1024, plzo_uint32_t ltmp_2391_7) {
  plzo_uint32_t ltmp_2392_7;
  plzo_uint32_t ltmp_2393_7;
  plzo_uint32_t ltmp_2394_7;
  plzo_uint32_t ltmp_2394_7_2E;
  plzo_uint32_t ltmp_2395_7;
  plzo_uint32_t ltmp_2395_7_2E;
  plzo_uint32_t ltmp_2396_7;
  plzo_uint32_t ltmp_2397_7;
  plzo_uint32_t ltmp_2398_7;
  plzo_uint32_t ltmp_2398_7_2E;
  plzo_uint32_t ltmp_2399_7;
  plzo_uint32_t ltmp_2400_7;
  plzo_uint32_t ltmp_2400_7_2E;

  if ((ltmp_2391_7 > 2u)) {
    goto ltmp_2401_D;
  } else {
    goto ltmp_2402_D;
  }

ltmp_2402_D:
  ltmp_2392_7 = ltmp_2391_7 + 1u;
  ltmp_66_1001(ltmp_2390_1024, ((plzo_uint32_t )(ltmp_2392_7 > 1u)));
  ltmp_66_1001(ltmp_2390_1024, (ltmp_2392_7 & 1u));
  return;
ltmp_2401_D:
  ltmp_2393_7 = ltmp_2391_7 + 4294967293u;
  ltmp_66_1001(ltmp_2390_1024, 0u);
  ltmp_66_1001(ltmp_2390_1024, 0u);
  if ((ltmp_2393_7 > 1u)) {
    ltmp_2394_7_2E = 2u;
    ltmp_2395_7_2E = ltmp_2393_7;
    goto ltmp_2403_D;
  } else {
    ltmp_2400_7_2E = ltmp_2393_7;
    goto ltmp_2404_D;
  }

  do {
ltmp_2403_D:
  ltmp_2394_7 = ltmp_2394_7_2E;
  ltmp_2395_7 = ltmp_2395_7_2E;
  ltmp_2396_7 = ltmp_2395_7 - ltmp_2394_7;
  ltmp_2397_7 = ltmp_2394_7 << 2;
  if ((ltmp_2396_7 < ltmp_2397_7)) {
    ltmp_2398_7_2E = ltmp_2397_7;
    goto ltmp_2405_D;
  } else {
    ltmp_2394_7_2E = ltmp_2397_7;
    ltmp_2395_7_2E = ltmp_2396_7;
    goto ltmp_2403_D;
  }

  } while (1);
  do {
ltmp_2405_D:
  ltmp_2398_7 = ltmp_2398_7_2E;
  ltmp_66_1001(ltmp_2390_1024, ((plzo_uint32_t )(((ltmp_2398_7 >> 1) & ltmp_2396_7) != 0u)));
  ltmp_66_1001(ltmp_2390_1024, 0u);
  ltmp_2399_7 = ltmp_2398_7 >> 2;
  ltmp_66_1001(ltmp_2390_1024, ((plzo_uint32_t )((ltmp_2399_7 & ltmp_2396_7) != 0u)));
  if ((ltmp_2399_7 > 2u)) {
    ltmp_2398_7_2E = ltmp_2399_7;
    goto ltmp_2405_D;
  } else {
    ltmp_2400_7_2E = ltmp_2396_7;
    goto ltmp_2404_D;
  }

  } while (1);
ltmp_2404_D:
  ltmp_2400_7 = ltmp_2400_7_2E;
  ltmp_66_1001(ltmp_2390_1024, (ltmp_2400_7 & 1u));
  ltmp_66_1001(ltmp_2390_1024, 1u);
  return;
}


