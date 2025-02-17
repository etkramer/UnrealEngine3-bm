/* pcbest64i.ch -- LZO1[XY]-99 compression

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
  plzo_uint64_t field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  struct l_struct_2E_A2_3A__3A_A3 *field3;
  struct l_struct_2E_A9 *field4;
};

struct l_struct_2E_A2_3A__3A_A3 {
  plzo_uint64_t field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
};

struct l_struct_2E_A5 {
  plzo_uint64_t field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_uint64_t field4;
  unsigned char *field5;
  unsigned char *field6;
};

struct l_struct_2E_A6 {
  unsigned char *field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_uint64_t field4;
  plzo_uint64_t field5;
  plzo_uint64_t field6;
  plzo_uint64_t field7;
  plzo_uint64_t field8;
  plzo_uint64_t field9;
  plzo_uint64_t field10;
  plzo_uint64_t field11;
  plzo_uint64_t field12;
  plzo_uint64_t field13;
  plzo_uint64_t field14;
  plzo_uint64_t field15;
  plzo_uint64_t field16;
  struct l_struct_2E_A7 *field17;
  struct l_struct_2E_M *field18;
  plzo_uint64_t field19;
  plzo_uint64_t field20;
  plzo_uint64_t field21;
  plzo_int64_t field22;
  plzo_uint64_t field23;
  plzo_uint64_t field24;
  unsigned char *field25[1025];
  plzo_uint64_t field26[65536];
  plzo_uint64_t field27[256];
};

struct l_struct_2E_A7 {
  plzo_uint64_t field0;
  plzo_uint64_t field1[8];
};

struct l_struct_2E_A8 {
  struct l_struct_2E_A7 *field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  unsigned char *field4;
  plzo_uint64_t field5;
  struct l_struct_2E_A9 *field6;
  struct l_struct_2E_A6 *field7;
};

struct plzo_callback_t {
  signed char * (*field0) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  void  (*field1) (struct plzo_callback_t *, signed char *);
  void  (*field2) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );
  signed char *field3;
  plzo_uint64_t field4;
  plzo_uint64_t field5;
};

struct l_struct_2E_A9 {
  plzo_uint64_t field0;
  plzo_uint64_t field1;
  struct plzo_callback_t field2;
};

struct l_struct_2E_E_3A__3A_A11 {
  plzo_uint64_t field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_uint64_t field4;
  plzo_uint64_t field5;
  plzo_uint64_t field6;
  plzo_uint64_t field7;
  plzo_uint64_t field8[8];
};

struct l_struct_2E_E_3A__3A_A10 {
  plzo_int64_t field0;
  plzo_int64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_uint64_t field4;
  plzo_uint64_t field5;
  plzo_uint64_t field6;
  plzo_uint64_t field7[17];
};

struct l_struct_2E_E_3A__3A_A13 {
  plzo_int64_t field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_uint64_t field4;
  plzo_uint64_t field5;
  plzo_uint64_t field6;
  plzo_uint64_t field7;
  plzo_uint64_t field8;
  plzo_int64_t field9;
  plzo_int64_t field10;
  plzo_int64_t field11;
  plzo_uint64_t field12;
  plzo_uint64_t field13;
  plzo_int64_t field14;
  plzo_int64_t field15;
};

struct l_struct_2E_E_3A__3A_A14 {
  unsigned char *field0;
  plzo_uint64_t field1;
  plzo_uint64_t  (*field2) (plzo_uint64_t );
  void  (*field3) (struct l_struct_2E_A5 *, plzo_uint64_t );
};

struct l_struct_2E_E_3A__3A_A15 {
  unsigned char field0;
};

struct l_struct_2E_E {
  plzo_int64_t  (**field0) ();
  struct l_struct_2E_A5 field1;
  struct l_struct_2E_E_3A__3A_A11 field2;
  plzo_int64_t field3;
  plzo_uint64_t field4[17];
  plzo_uint64_t field5;
  struct l_struct_2E_E_3A__3A_A10 field6;
  struct l_struct_2E_E_3A__3A_A13 field7;
  unsigned char *field8;
  unsigned char *field9;
  unsigned char *field10;
  plzo_uint64_t field11;
  plzo_uint64_t field12;
  plzo_uint64_t field13;
  plzo_uint64_t field14;
  plzo_uint64_t field15;
  plzo_uint64_t field16;
  plzo_uint64_t field17;
  plzo_uint64_t field18;
  struct l_struct_2E_A8 field19;
  struct l_struct_2E_A2 field20;
  plzo_uint64_t field21;
  struct l_struct_2E_E_3A__3A_A14 field22;
  struct l_struct_2E_E_3A__3A_A14 field23;
  struct l_struct_2E_A9 field24;
  struct l_struct_2E_E_3A__3A_A15 field25;
  struct l_struct_2E_E_3A__3A_A15 *field26;
};

struct l_struct_2E_E_3A__3A_A12 {
  plzo_uint64_t field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_uint64_t field4;
  plzo_uint64_t field5;
};

struct l_struct_2E_E_3A__3A_A4 {
  plzo_uint64_t field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_uint64_t field4;
  plzo_uint64_t field5;
  plzo_uint64_t field6;
  plzo_uint64_t field7;
};

struct l_struct_2E_Lzo1E {
  struct l_struct_2E_E field0;
  plzo_uint64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_uint64_t field4;
  plzo_uint64_t field5;
  plzo_uint64_t field6;
  plzo_uint64_t field7;
  plzo_uint64_t field8;
  plzo_uint64_t field9;
  plzo_uint64_t field10;
  plzo_uint64_t field11;
  plzo_uint64_t field12;
  plzo_uint64_t field13;
  plzo_uint64_t field14;
  plzo_uint64_t field15;
  plzo_uint64_t field16;
  plzo_uint64_t field17;
  plzo_uint64_t field18;
  plzo_uint64_t field19;
  plzo_uint64_t field20;
  plzo_uint64_t field21;
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
  plzo_int64_t field0;
  plzo_int64_t field1;
  plzo_uint64_t field2;
  plzo_uint64_t field3;
  plzo_int64_t field4;
  plzo_int64_t field5;
  plzo_int64_t field6;
  plzo_int64_t field7;
  plzo_uint64_t field8;
};


static void ltmp_0_1000(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_1_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static plzo_int64_t ltmp_2_1002(struct l_struct_2E_E *, plzo_int64_t , struct plzo_callback_t *);
static plzo_int64_t ltmp_3_1003(struct l_struct_2E_E *, struct plzo_compress_config_t *);
static plzo_int64_t ltmp_4_1004(struct l_struct_2E_E *, unsigned char *, plzo_uint64_t , unsigned char *, plzo_uint64_t *);
static void ltmp_5_1005(struct l_struct_2E_E *);
static struct l_struct_2E_E_3A__3A_A12 *ltmp_6_1006(struct l_struct_2E_E *, plzo_int64_t );
static void ltmp_7_1005(struct l_struct_2E_E *);
static void ltmp_8_1005(struct l_struct_2E_E *);
static plzo_int64_t ltmp_9_1007(struct l_struct_2E_E *);
static void ltmp_10_1005(struct l_struct_2E_E *);
static void ltmp_11_1000(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t );
static plzo_uint64_t ltmp_12_1008(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t );
static plzo_uint64_t ltmp_13_1008(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t );
static plzo_uint64_t ltmp_14_1008(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_15_1000(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_16_1009(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );
static plzo_uint64_t ltmp_17_1008(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t );
static struct l_struct_2E_M *ltmp_18_100A(struct l_struct_2E_A6 *);
static void ltmp_19_100B(struct l_struct_2E_A8 *);
static void ltmp_20_100C(struct l_struct_2E_A5 *);
static void ltmp_21_100D(struct l_struct_2E_Lzo1xE *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_22_100E(struct l_struct_2E_Lzo1xE *, unsigned char *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_23_100D(struct l_struct_2E_Lzo1xE *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_24_100F(struct l_struct_2E_Lzo1E *, plzo_uint64_t , plzo_uint64_t );
static plzo_int64_t plzo_lzo1x_99_compress_internal(unsigned char *, plzo_uint64_t , unsigned char *, plzo_uint64_t *, struct plzo_callback_t *, plzo_int64_t , struct plzo_compress_config_t *, plzo_uint64_t *);
static void ltmp_25_1010(struct l_struct_2E_Lzo1E *);
static plzo_int64_t plzo_lzo1y_99_compress_internal(unsigned char *, plzo_uint64_t , unsigned char *, plzo_uint64_t *, struct plzo_callback_t *, plzo_int64_t , struct plzo_compress_config_t *, plzo_uint64_t *);
static signed char *ltmp_26_1011(struct l_struct_2E_E *);
static void ltmp_27_1010(struct l_struct_2E_Lzo1E *);
static plzo_int64_t ltmp_28_1012(struct l_struct_2E_Lzo1E *);
static void ltmp_29_1013(struct l_struct_2E_Lzo1xE *);
static void ltmp_30_1014(struct l_struct_2E_Lzo1E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint64_t , plzo_uint64_t , plzo_uint64_t );
static plzo_uint64_t ltmp_31_1015(struct l_struct_2E_Lzo1E *);
static plzo_int64_t ltmp_32_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static plzo_int64_t ltmp_33_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static void ltmp_34_100F(struct l_struct_2E_Lzo1E *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_35_1017(struct l_struct_2E_Lzo1E *, unsigned char *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_36_1010(struct l_struct_2E_Lzo1E *);
static plzo_uint64_t ltmp_37_1015(struct l_struct_2E_Lzo1E *);
static plzo_int64_t ltmp_38_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static plzo_int64_t ltmp_39_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static void ltmp_40_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_41_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_42_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_43_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_44_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_45_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_46_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_47_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_48_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static signed char *ltmp_49_1018(struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_50_1019(struct plzo_callback_t *, signed char *);
static void ltmp_51_101A(struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );
static void ltmp_52_101B(struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A14 *);
static void ltmp_53_101C(struct l_struct_2E_E_3A__3A_A14 *);
static void  (*ltmp_54_101D(void  (*) (struct l_struct_2E_A5 *, plzo_uint64_t ), plzo_uint64_t , plzo_uint64_t )) (struct l_struct_2E_A5 *, plzo_uint64_t );
static plzo_int64_t ltmp_55_1007(struct l_struct_2E_E *);
static void ltmp_56_101E(struct l_struct_2E_E *, unsigned char *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_57_1000(struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t );
static void ltmp_58_1005(struct l_struct_2E_E *);
static void ltmp_59_101F(struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint64_t , plzo_uint64_t , plzo_uint64_t );
static plzo_uint64_t ltmp_60_1020(struct l_struct_2E_E *);
static plzo_int64_t ltmp_61_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static plzo_int64_t ltmp_62_1016(struct l_struct_2E_E *, struct l_struct_2E_A7 *);
static void ltmp_63_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_64_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_65_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_66_1001(struct l_struct_2E_A5 *, plzo_uint64_t );
static void ltmp_67_1001(struct l_struct_2E_A5 *, plzo_uint64_t );


static plzo_int64_t  (*ltmp_68_1021[24]) () = { ((plzo_int64_t  (*) ())0), ((plzo_int64_t  (*) ())0), ((plzo_int64_t  (*) ())ltmp_26_1011), ((plzo_int64_t  (*) ())ltmp_4_1004), ((plzo_int64_t  (*) ())ltmp_6_1006), ((plzo_int64_t  (*) ())ltmp_22_100E), ((plzo_int64_t  (*) ())ltmp_21_100D), ((plzo_int64_t  (*) ())ltmp_27_1010), ((plzo_int64_t  (*) ())ltmp_7_1005), ((plzo_int64_t  (*) ())ltmp_28_1012), ((plzo_int64_t  (*) ())ltmp_29_1013), ((plzo_int64_t  (*) ())ltmp_9_1007), ((plzo_int64_t  (*) ())ltmp_10_1005), ((plzo_int64_t  (*) ())ltmp_23_100D), ((plzo_int64_t  (*) ())ltmp_12_1008), ((plzo_int64_t  (*) ())ltmp_13_1008), ((plzo_int64_t  (*) ())ltmp_14_1008), ((plzo_int64_t  (*) ())ltmp_15_1000), ((plzo_int64_t  (*) ())ltmp_16_1009), ((plzo_int64_t  (*) ())ltmp_30_1014), ((plzo_int64_t  (*) ())ltmp_31_1015), ((plzo_int64_t  (*) ())ltmp_17_1008), ((plzo_int64_t  (*) ())ltmp_32_1016), ((plzo_int64_t  (*) ())ltmp_33_1016) };
static plzo_int64_t  (*ltmp_69_1021[24]) () = { ((plzo_int64_t  (*) ())0), ((plzo_int64_t  (*) ())0), ((plzo_int64_t  (*) ())ltmp_26_1011), ((plzo_int64_t  (*) ())ltmp_4_1004), ((plzo_int64_t  (*) ())ltmp_6_1006), ((plzo_int64_t  (*) ())ltmp_35_1017), ((plzo_int64_t  (*) ())ltmp_34_100F), ((plzo_int64_t  (*) ())ltmp_27_1010), ((plzo_int64_t  (*) ())ltmp_7_1005), ((plzo_int64_t  (*) ())ltmp_28_1012), ((plzo_int64_t  (*) ())ltmp_36_1010), ((plzo_int64_t  (*) ())ltmp_9_1007), ((plzo_int64_t  (*) ())ltmp_10_1005), ((plzo_int64_t  (*) ())ltmp_24_100F), ((plzo_int64_t  (*) ())ltmp_12_1008), ((plzo_int64_t  (*) ())ltmp_13_1008), ((plzo_int64_t  (*) ())ltmp_14_1008), ((plzo_int64_t  (*) ())ltmp_15_1000), ((plzo_int64_t  (*) ())ltmp_16_1009), ((plzo_int64_t  (*) ())ltmp_30_1014), ((plzo_int64_t  (*) ())ltmp_37_1015), ((plzo_int64_t  (*) ())ltmp_17_1008), ((plzo_int64_t  (*) ())ltmp_38_1016), ((plzo_int64_t  (*) ())ltmp_39_1016) };
static plzo_int64_t  (*ltmp_70_1021[24]) () = { ((plzo_int64_t  (*) ())0), ((plzo_int64_t  (*) ())0), ((plzo_int64_t  (*) ())ltmp_26_1011), ((plzo_int64_t  (*) ())ltmp_4_1004), ((plzo_int64_t  (*) ())ltmp_6_1006), ((plzo_int64_t  (*) ())ltmp_56_101E), ((plzo_int64_t  (*) ())ltmp_57_1000), ((plzo_int64_t  (*) ())ltmp_58_1005), ((plzo_int64_t  (*) ())ltmp_7_1005), ((plzo_int64_t  (*) ())ltmp_55_1007), ((plzo_int64_t  (*) ())ltmp_8_1005), ((plzo_int64_t  (*) ())ltmp_9_1007), ((plzo_int64_t  (*) ())ltmp_10_1005), ((plzo_int64_t  (*) ())ltmp_11_1000), ((plzo_int64_t  (*) ())ltmp_12_1008), ((plzo_int64_t  (*) ())ltmp_13_1008), ((plzo_int64_t  (*) ())ltmp_14_1008), ((plzo_int64_t  (*) ())ltmp_15_1000), ((plzo_int64_t  (*) ())ltmp_16_1009), ((plzo_int64_t  (*) ())ltmp_59_101F), ((plzo_int64_t  (*) ())ltmp_60_1020), ((plzo_int64_t  (*) ())ltmp_17_1008), ((plzo_int64_t  (*) ())ltmp_61_1016), ((plzo_int64_t  (*) ())ltmp_62_1016) };
static struct l_struct_2E_E_3A__3A_A12 ltmp_71_1022[10] = { { PLZO_UINT64_C(65535), PLZO_UINT64_C(8), PLZO_UINT64_C(1), PLZO_UINT64_C(1), PLZO_UINT64_C(4), PLZO_UINT64_C(0) }, { PLZO_UINT64_C(65535), PLZO_UINT64_C(16), PLZO_UINT64_C(1), PLZO_UINT64_C(1), PLZO_UINT64_C(8), PLZO_UINT64_C(1) }, { PLZO_UINT64_C(65535), PLZO_UINT64_C(32), PLZO_UINT64_C(3), PLZO_UINT64_C(1), PLZO_UINT64_C(16), PLZO_UINT64_C(2) }, { PLZO_UINT64_C(131071), PLZO_UINT64_C(16), PLZO_UINT64_C(4), PLZO_UINT64_C(4), PLZO_UINT64_C(16), PLZO_UINT64_C(3) }, { PLZO_UINT64_C(131071), PLZO_UINT64_C(32), PLZO_UINT64_C(16), PLZO_UINT64_C(8), PLZO_UINT64_C(32), PLZO_UINT64_C(3) }, { PLZO_UINT64_C(131071), PLZO_UINT64_C(128), PLZO_UINT64_C(16), PLZO_UINT64_C(8), PLZO_UINT64_C(128), PLZO_UINT64_C(4) }, { PLZO_UINT64_C(0), PLZO_UINT64_C(128), PLZO_UINT64_C(32), PLZO_UINT64_C(8), PLZO_UINT64_C(256), PLZO_UINT64_C(4) }, { PLZO_UINT64_C(0), PLZO_UINT64_C(256), PLZO_UINT64_C(128), PLZO_UINT64_C(32), PLZO_UINT64_C(2048), PLZO_UINT64_C(4) }, { PLZO_UINT64_C(0), PLZO_UINT64_C(1024), PLZO_UINT64_C(1024), PLZO_UINT64_C(32), PLZO_UINT64_C(4096), PLZO_UINT64_C(4) }, { PLZO_UINT64_C(0), PLZO_UINT64_C(1024), PLZO_UINT64_C(1024), PLZO_UINT64_C(128), PLZO_UINT64_C(8192), PLZO_UINT64_C(7) } };




static void ltmp_0_1000(struct l_struct_2E_E *ltmp_72_1023, plzo_uint64_t ltmp_73_9, plzo_uint64_t ltmp_74_9) {
  plzo_uint64_t *ltmp_75_19;
  plzo_uint64_t ltmp_76_9;
  plzo_uint64_t *ltmp_77_19;
  plzo_uint64_t ltmp_78_9;
  plzo_uint64_t *ltmp_79_19;
  plzo_uint64_t ltmp_80_9;
  plzo_uint64_t ltmp_81_9;
  plzo_uint64_t ltmp_82_9;
  plzo_uint64_t ltmp_83_9;
  plzo_uint64_t ltmp_84_9;
  plzo_uint64_t ltmp_85_9;

  ltmp_75_19 = &ltmp_72_1023->field2.field6;
  ltmp_76_9 = *ltmp_75_19;
  if ((ltmp_76_9 == PLZO_UINT64_C(0))) {
    goto ltmp_86_D;
  } else {
    goto ltmp_87_D;
  }

ltmp_86_D:
  *ltmp_75_19 = ltmp_74_9;
  *(&ltmp_72_1023->field2.field3) = ltmp_73_9;
  *(&ltmp_72_1023->field2.field2) = ltmp_73_9;
  *(&ltmp_72_1023->field2.field1) = ltmp_74_9;
  *(&ltmp_72_1023->field2.field0) = ltmp_74_9;
  return;
ltmp_87_D:
  ltmp_77_19 = &ltmp_72_1023->field2.field3;
  ltmp_78_9 = *ltmp_77_19;
  ltmp_79_19 = &ltmp_72_1023->field2.field2;
  if ((ltmp_78_9 < ltmp_73_9)) {
    goto ltmp_88_D;
  } else {
    goto ltmp_89_D;
  }

ltmp_88_D:
  *ltmp_77_19 = ltmp_73_9;
  ltmp_80_9 = *ltmp_79_19;
  if ((ltmp_80_9 > ltmp_73_9)) {
    goto ltmp_90_D;
  } else {
    goto ltmp_91_D;
  }

ltmp_89_D:
  ltmp_81_9 = *ltmp_79_19;
  if ((ltmp_81_9 > ltmp_73_9)) {
    goto ltmp_90_D;
  } else {
    goto ltmp_91_D;
  }

ltmp_90_D:
  *ltmp_79_19 = ltmp_73_9;
  ltmp_82_9 = *(&ltmp_72_1023->field2.field1);
  if ((ltmp_82_9 < ltmp_74_9)) {
    goto ltmp_92_D;
  } else {
    goto ltmp_93_D;
  }

ltmp_91_D:
  ltmp_83_9 = *(&ltmp_72_1023->field2.field1);
  if ((ltmp_83_9 < ltmp_74_9)) {
    goto ltmp_92_D;
  } else {
    goto ltmp_93_D;
  }

ltmp_92_D:
  *(&ltmp_72_1023->field2.field1) = ltmp_74_9;
  ltmp_84_9 = *(&ltmp_72_1023->field2.field0);
  if ((ltmp_84_9 > ltmp_74_9)) {
    goto ltmp_94_D;
  } else {
    goto ltmp_95_D;
  }

ltmp_93_D:
  ltmp_85_9 = *(&ltmp_72_1023->field2.field0);
  if ((ltmp_85_9 > ltmp_74_9)) {
    goto ltmp_94_D;
  } else {
    goto ltmp_95_D;
  }

ltmp_94_D:
  *(&ltmp_72_1023->field2.field0) = ltmp_74_9;
  return;
ltmp_95_D:
  return;
}


static void ltmp_1_1001(struct l_struct_2E_A5 *ltmp_96_1024, plzo_uint64_t ltmp_97_9) {
  plzo_uint64_t *ltmp_98_19;
  plzo_uint64_t ltmp_99_9;
  plzo_uint64_t ltmp_100_9;
  unsigned char **ltmp_101_1025;
  unsigned char *ltmp_102_13;
  plzo_uint64_t ltmp_103_9;
  plzo_uint64_t *ltmp_104_19;
  plzo_uint64_t ltmp_105_9;
  unsigned char **ltmp_106_1025;
  unsigned char *ltmp_107_13;
  plzo_uint64_t ltmp_108_9;

  ltmp_98_19 = &ltmp_96_1024->field1;
  ltmp_99_9 = *ltmp_98_19;
  ltmp_100_9 = *(&ltmp_96_1024->field3);
  if ((ltmp_99_9 < ltmp_100_9)) {
    goto ltmp_109_D;
  } else {
    goto ltmp_110_D;
  }

ltmp_109_D:
  if ((ltmp_99_9 == PLZO_UINT64_C(0))) {
    goto ltmp_111_D;
  } else {
    goto ltmp_112_D;
  }

ltmp_111_D:
  ltmp_101_1025 = &ltmp_96_1024->field6;
  ltmp_102_13 = *ltmp_101_1025;
  *(&ltmp_96_1024->field5) = ltmp_102_13;
  ltmp_103_9 = *(&ltmp_96_1024->field4);
  *ltmp_101_1025 = (&ltmp_102_13[ltmp_103_9]);
  goto ltmp_112_D;

ltmp_112_D:
  ltmp_104_19 = &ltmp_96_1024->field0;
  ltmp_105_9 = *ltmp_104_19;
  *ltmp_104_19 = ((ltmp_105_9 << 1) + ltmp_97_9);
  *ltmp_98_19 = (ltmp_99_9 + PLZO_UINT64_C(1));
  return;
ltmp_110_D:
  ltmp_20_100C(ltmp_96_1024);
  ltmp_106_1025 = &ltmp_96_1024->field6;
  ltmp_107_13 = *ltmp_106_1025;
  *(&ltmp_96_1024->field5) = ltmp_107_13;
  ltmp_108_9 = *(&ltmp_96_1024->field4);
  *ltmp_106_1025 = (&ltmp_107_13[ltmp_108_9]);
  *(&ltmp_96_1024->field0) = ltmp_97_9;
  *ltmp_98_19 = PLZO_UINT64_C(1);
  return;
}


static plzo_int64_t ltmp_2_1002(struct l_struct_2E_E *ltmp_113_1023, plzo_int64_t ltmp_114_A, struct plzo_callback_t *ltmp_115_1026) {
  struct plzo_compress_config_t ltmp_116_1027;
  struct l_struct_2E_E_3A__3A_A14 *ltmp_117_1028;
  struct l_struct_2E_E_3A__3A_A14 *ltmp_118_1028;
  unsigned char *ltmp_119_13;
  plzo_uint64_t ltmp_120_9;
  plzo_uint64_t ltmp_120_9_2E;
  plzo_uint64_t ltmp_121_9;
  plzo_int64_t  (**ltmp_122_1029) ();
  plzo_int64_t  (*ltmp_123_102A) ();
  struct l_struct_2E_E_3A__3A_A12 *ltmp_124_102B;
  plzo_uint64_t *ltmp_125_19;
  plzo_uint64_t ltmp_126_9;
  plzo_uint64_t ltmp_127_9;
  plzo_uint64_t ltmp_128_9;
  plzo_uint64_t ltmp_129_9;
  plzo_uint64_t ltmp_130_9;
  plzo_uint64_t ltmp_131_9;
  plzo_uint64_t ltmp_132_9;
  plzo_uint64_t ltmp_133_9;
  unsigned char *ltmp_134_13;
  plzo_uint64_t ltmp_135_9;
  plzo_uint64_t ltmp_135_9_2E;
  plzo_uint64_t ltmp_136_9;
  plzo_uint64_t ltmp_137_9;
  plzo_int64_t ltmp_138_A;
  plzo_int64_t ltmp_139_A;
  plzo_int64_t ltmp_139_A_2E;

  if (((((plzo_uint64_t )ltmp_114_A) + PLZO_UINT64_C(18446744073709551615)) > PLZO_UINT64_C(9))) {
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
  if ((ltmp_121_9 == PLZO_UINT64_C(128))) {
    goto ltmp_143_D;
  } else {
    ltmp_120_9_2E = ltmp_121_9;
    goto ltmp_142_D;
  }

  } while (1);
ltmp_143_D:
  if (((((plzo_uint64_t )ltmp_114_A) + PLZO_UINT64_C(18446744073709551615)) > PLZO_UINT64_C(9))) {
    ltmp_139_A_2E = PLZO_INT64_C(-2);
    goto ltmp_144_D;
  } else {
    goto ltmp_145_D;
  }

ltmp_145_D:
  ltmp_122_1029 = *(&ltmp_113_1023->field0);
  ltmp_123_102A = *(&ltmp_122_1029[PLZO_INT64_C(2)]);
  ltmp_124_102B = ((struct l_struct_2E_E_3A__3A_A12 * (*) (struct l_struct_2E_E *, plzo_int64_t ))ltmp_123_102A)(ltmp_113_1023, ltmp_114_A);
  if ((ltmp_124_102B == ((struct l_struct_2E_E_3A__3A_A12 *)0))) {
    ltmp_139_A_2E = PLZO_INT64_C(-2);
    goto ltmp_144_D;
  } else {
    goto ltmp_146_D;
  }

ltmp_146_D:
  *(&ltmp_113_1023->field7.field0) = ltmp_114_A;
  ltmp_125_19 = &ltmp_113_1023->field7.field2;
  ltmp_126_9 = *(&ltmp_113_1023->field6.field3);
  *ltmp_125_19 = ltmp_126_9;
  ltmp_127_9 = *(&ltmp_113_1023->field6.field4);
  *(&ltmp_113_1023->field7.field3) = ltmp_127_9;
  ltmp_128_9 = *(&ltmp_113_1023->field6.field5);
  *(&ltmp_113_1023->field7.field4) = ltmp_128_9;
  ltmp_129_9 = *(&ltmp_124_102B->field0);
  if ((ltmp_129_9 == PLZO_UINT64_C(0))) {
    goto ltmp_147_D;
  } else {
    goto ltmp_148_D;
  }

ltmp_148_D:
  *ltmp_125_19 = ((((ltmp_126_9 <= ltmp_129_9)) ? (ltmp_126_9) : (ltmp_129_9)));
  goto ltmp_147_D;

ltmp_147_D:
  ltmp_130_9 = *(&ltmp_124_102B->field1);
  *(&ltmp_113_1023->field7.field5) = ltmp_130_9;
  ltmp_131_9 = *(&ltmp_124_102B->field2);
  *(&ltmp_113_1023->field7.field6) = ltmp_131_9;
  ltmp_132_9 = *(&ltmp_124_102B->field3);
  *(&ltmp_113_1023->field7.field7) = ltmp_132_9;
  ltmp_133_9 = *(&ltmp_124_102B->field4);
  *(&ltmp_113_1023->field7.field8) = ltmp_133_9;
  ltmp_134_13 = (unsigned char *)(&ltmp_116_1027);
  ltmp_135_9_2E = PLZO_UINT64_C(0);
  goto ltmp_149_D;

  do {
ltmp_149_D:
  ltmp_135_9 = ltmp_135_9_2E;
  *(&ltmp_134_13[ltmp_135_9]) = 255;
  ltmp_136_9 = ltmp_135_9 + PLZO_UINT64_C(1);
  if ((ltmp_136_9 == PLZO_UINT64_C(72))) {
    goto ltmp_150_D;
  } else {
    ltmp_135_9_2E = ltmp_136_9;
    goto ltmp_149_D;
  }

  } while (1);
ltmp_150_D:
  ltmp_137_9 = *(&ltmp_124_102B->field5);
  *(&ltmp_116_1027.field6) = (((plzo_int64_t )ltmp_137_9) & PLZO_INT64_C(7));
  *(&ltmp_116_1027.field8) = PLZO_UINT64_C(10000);
  ltmp_138_A = ltmp_3_1003(ltmp_113_1023, (&ltmp_116_1027));
  ltmp_139_A_2E = ltmp_138_A;
  goto ltmp_144_D;

ltmp_144_D:
  ltmp_139_A = ltmp_139_A_2E;
  *(&ltmp_113_1023->field3) = PLZO_INT64_C(1);
  return ltmp_139_A;
ltmp_140_D:
  return PLZO_INT64_C(-2);
}


static plzo_int64_t ltmp_3_1003(struct l_struct_2E_E *ltmp_151_1023, struct plzo_compress_config_t *ltmp_152_1027) {
  plzo_int64_t ltmp_153_A;
  plzo_int64_t ltmp_154_A;
  plzo_int64_t ltmp_155_A;
  plzo_int64_t ltmp_156_A;
  plzo_int64_t ltmp_156_A_2E;
  plzo_uint64_t ltmp_157_9;
  plzo_uint64_t *ltmp_158_19;
  plzo_uint64_t ltmp_159_9;
  plzo_uint64_t ltmp_160_9;
  plzo_uint64_t ltmp_161_9;
  plzo_uint64_t ltmp_162_9;
  plzo_uint64_t ltmp_163_9;
  plzo_uint64_t ltmp_164_9;
  plzo_int64_t ltmp_165_A;
  plzo_int64_t ltmp_166_A;
  plzo_int64_t ltmp_167_A;
  plzo_uint64_t ltmp_168_9;
  plzo_uint64_t ltmp_169_9;
  plzo_uint64_t ltmp_170_9;
  plzo_int64_t ltmp_171_A;
  plzo_int64_t ltmp_172_A;
  plzo_uint64_t ltmp_173_9;
  plzo_uint64_t ltmp_174_9;
  plzo_uint64_t ltmp_175_9;
  plzo_uint64_t ltmp_176_9;
  plzo_uint64_t ltmp_177_9;
  plzo_uint64_t ltmp_178_9;
  plzo_uint64_t ltmp_179_9;
  plzo_int64_t *ltmp_180_1A;
  plzo_int64_t ltmp_181_A;
  plzo_int64_t ltmp_182_A;
  plzo_uint64_t ltmp_183_9;
  plzo_uint64_t ltmp_184_9;
  plzo_uint64_t ltmp_185_9;
  plzo_uint64_t ltmp_186_9;
  plzo_uint64_t ltmp_187_9;
  plzo_uint64_t ltmp_188_9;
  plzo_uint64_t ltmp_189_9;
  plzo_uint64_t ltmp_190_9;
  plzo_uint64_t ltmp_191_9;
  plzo_uint64_t ltmp_192_9;
  plzo_uint64_t ltmp_193_9;
  plzo_uint64_t ltmp_194_9;
  plzo_uint64_t ltmp_195_9;
  plzo_uint64_t ltmp_196_9;
  plzo_uint64_t ltmp_197_9;
  plzo_uint64_t ltmp_198_9;
  plzo_int64_t ltmp_199_A;
  plzo_int64_t ltmp_199_A_2E;

  ltmp_153_A = *(&ltmp_151_1023->field7.field0);
  if ((ltmp_153_A > PLZO_INT64_C(0))) {
    goto ltmp_200_D;
  } else {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  }

ltmp_200_D:
  ltmp_154_A = *(&ltmp_152_1027->field0);
  ltmp_155_A = *(&ltmp_152_1027->field1);
  if ((ltmp_154_A == PLZO_INT64_C(-1))) {
    goto ltmp_202_D;
  } else {
    goto ltmp_203_D;
  }

ltmp_203_D:
  *(&ltmp_151_1023->field1.field2) = ((plzo_uint64_t )(ltmp_154_A != PLZO_INT64_C(0)));
  if ((ltmp_155_A == PLZO_INT64_C(-1))) {
    goto ltmp_204_D;
  } else {
    goto ltmp_205_D;
  }

ltmp_202_D:
  if ((ltmp_155_A == PLZO_INT64_C(-1))) {
    goto ltmp_204_D;
  } else {
    goto ltmp_205_D;
  }

ltmp_205_D:
  if (((ltmp_155_A == PLZO_INT64_C(32)) | (((ltmp_155_A != PLZO_INT64_C(8)) & (ltmp_155_A != PLZO_INT64_C(16))) ^ 1))) {
    goto ltmp_206_D;
  } else {
    ltmp_156_A_2E = PLZO_INT64_C(-2);
    goto ltmp_207_D;
  }

ltmp_206_D:
  *(&ltmp_151_1023->field1.field3) = ((plzo_uint64_t )ltmp_155_A);
  *(&ltmp_151_1023->field1.field4) = ((plzo_uint64_t )(ltmp_155_A/PLZO_INT64_C(8)));
  goto ltmp_204_D;

ltmp_204_D:
  *(&ltmp_151_1023->field1.field0) = PLZO_UINT64_C(0);
  *(&ltmp_151_1023->field1.field1) = PLZO_UINT64_C(0);
  *(&ltmp_151_1023->field1.field5) = ((unsigned char *)0);
  *(&ltmp_151_1023->field1.field6) = ((unsigned char *)0);
  ltmp_156_A_2E = PLZO_INT64_C(0);
  goto ltmp_207_D;

ltmp_207_D:
  ltmp_156_A = ltmp_156_A_2E;
  if ((ltmp_156_A == PLZO_INT64_C(0))) {
    goto ltmp_208_D;
  } else {
    ltmp_199_A_2E = ltmp_156_A;
    goto ltmp_201_D;
  }

ltmp_208_D:
  ltmp_157_9 = *(&ltmp_152_1027->field2);
  ltmp_158_19 = &ltmp_152_1027->field3;
  if ((ltmp_157_9 == PLZO_UINT64_C(18446744073709551615))) {
    goto ltmp_209_D;
  } else {
    goto ltmp_210_D;
  }

ltmp_210_D:
  ltmp_159_9 = *(&ltmp_151_1023->field6.field3);
  *(&ltmp_151_1023->field7.field2) = ((((ltmp_157_9 <= ltmp_159_9)) ? (ltmp_157_9) : (ltmp_159_9)));
  ltmp_160_9 = *ltmp_158_19;
  if ((ltmp_160_9 == PLZO_UINT64_C(18446744073709551615))) {
    goto ltmp_211_D;
  } else {
    goto ltmp_212_D;
  }

ltmp_209_D:
  ltmp_161_9 = *ltmp_158_19;
  if ((ltmp_161_9 == PLZO_UINT64_C(18446744073709551615))) {
    goto ltmp_211_D;
  } else {
    goto ltmp_212_D;
  }

ltmp_212_D:
  ltmp_162_9 = *ltmp_158_19;
  ltmp_163_9 = *(&ltmp_151_1023->field6.field4);
  if ((ltmp_162_9 < ltmp_163_9)) {
    goto ltmp_211_D;
  } else {
    goto ltmp_213_D;
  }

ltmp_213_D:
  ltmp_164_9 = *(&ltmp_151_1023->field6.field5);
  *(&ltmp_151_1023->field7.field4) = ((((ltmp_162_9 <= ltmp_164_9)) ? (ltmp_162_9) : (ltmp_164_9)));
  ltmp_165_A = *(&ltmp_152_1027->field7);
  if ((ltmp_165_A == PLZO_INT64_C(-1))) {
    goto ltmp_214_D;
  } else {
    goto ltmp_215_D;
  }

ltmp_211_D:
  ltmp_166_A = *(&ltmp_152_1027->field7);
  if ((ltmp_166_A == PLZO_INT64_C(-1))) {
    goto ltmp_214_D;
  } else {
    goto ltmp_215_D;
  }

ltmp_215_D:
  ltmp_167_A = *(&ltmp_152_1027->field7);
  *(&ltmp_151_1023->field7.field14) = ltmp_167_A;
  ltmp_168_9 = *(&ltmp_152_1027->field8);
  if ((ltmp_168_9 == PLZO_UINT64_C(18446744073709551615))) {
    goto ltmp_216_D;
  } else {
    goto ltmp_217_D;
  }

ltmp_214_D:
  ltmp_169_9 = *(&ltmp_152_1027->field8);
  if ((ltmp_169_9 == PLZO_UINT64_C(18446744073709551615))) {
    goto ltmp_216_D;
  } else {
    goto ltmp_217_D;
  }

ltmp_217_D:
  ltmp_170_9 = *(&ltmp_152_1027->field8);
  *(&ltmp_151_1023->field7.field1) = ltmp_170_9;
  ltmp_171_A = *(&ltmp_152_1027->field6);
  switch (ltmp_171_A) {
  default:
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
;
  case PLZO_INT64_C(7):
    goto ltmp_218_D;
  case PLZO_INT64_C(6):
    goto ltmp_219_D;
  case PLZO_INT64_C(5):
    goto ltmp_220_D;
  case PLZO_INT64_C(4):
    goto ltmp_221_D;
  case PLZO_INT64_C(3):
    goto ltmp_222_D;
  case PLZO_INT64_C(2):
    goto ltmp_223_D;
  case PLZO_INT64_C(1):
    goto ltmp_224_D;
  case PLZO_INT64_C(0):
    goto ltmp_225_D;
  case PLZO_INT64_C(-1):
    goto ltmp_226_D;
  }
ltmp_216_D:
  ltmp_172_A = *(&ltmp_152_1027->field6);
  switch (ltmp_172_A) {
  default:
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
;
  case PLZO_INT64_C(7):
    goto ltmp_218_D;
  case PLZO_INT64_C(6):
    goto ltmp_219_D;
  case PLZO_INT64_C(5):
    goto ltmp_220_D;
  case PLZO_INT64_C(4):
    goto ltmp_221_D;
  case PLZO_INT64_C(3):
    goto ltmp_222_D;
  case PLZO_INT64_C(2):
    goto ltmp_223_D;
  case PLZO_INT64_C(1):
    goto ltmp_224_D;
  case PLZO_INT64_C(0):
    goto ltmp_225_D;
    break;
  case PLZO_INT64_C(-1):
    goto ltmp_226_D;
  }
ltmp_225_D:
  *(&ltmp_151_1023->field7.field11) = PLZO_INT64_C(0);
  ltmp_173_9 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_173_9;
  *(&ltmp_151_1023->field7.field13) = PLZO_UINT64_C(0);
  goto ltmp_226_D;

ltmp_224_D:
  *(&ltmp_151_1023->field7.field11) = PLZO_INT64_C(1);
  ltmp_174_9 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_174_9;
  *(&ltmp_151_1023->field7.field13) = PLZO_UINT64_C(0);
  goto ltmp_226_D;

ltmp_223_D:
  *(&ltmp_151_1023->field7.field11) = PLZO_INT64_C(2);
  ltmp_175_9 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_175_9;
  *(&ltmp_151_1023->field7.field13) = PLZO_UINT64_C(0);
  goto ltmp_226_D;

ltmp_222_D:
  *(&ltmp_151_1023->field7.field11) = PLZO_INT64_C(3);
  ltmp_176_9 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_176_9 + PLZO_UINT64_C(1));
  *(&ltmp_151_1023->field7.field13) = PLZO_UINT64_C(1);
  goto ltmp_226_D;

ltmp_221_D:
  *(&ltmp_151_1023->field7.field11) = PLZO_INT64_C(3);
  ltmp_177_9 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_177_9 + PLZO_UINT64_C(6));
  *(&ltmp_151_1023->field7.field13) = PLZO_UINT64_C(4);
  goto ltmp_226_D;

ltmp_220_D:
  *(&ltmp_151_1023->field7.field11) = PLZO_INT64_C(3);
  ltmp_178_9 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_178_9 + PLZO_UINT64_C(32));
  *(&ltmp_151_1023->field7.field13) = PLZO_UINT64_C(16);
  goto ltmp_226_D;

ltmp_219_D:
  *(&ltmp_151_1023->field7.field11) = PLZO_INT64_C(3);
  ltmp_179_9 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_179_9 + PLZO_UINT64_C(32));
  *(&ltmp_151_1023->field7.field13) = PLZO_UINT64_C(32);
  goto ltmp_226_D;

ltmp_218_D:
  *(&ltmp_151_1023->field7.field11) = PLZO_INT64_C(3);
  *(&ltmp_151_1023->field7.field12) = PLZO_UINT64_C(0);
  *(&ltmp_151_1023->field7.field13) = PLZO_UINT64_C(0);
  goto ltmp_226_D;

ltmp_226_D:
  ltmp_180_1A = &ltmp_151_1023->field7.field9;
  *ltmp_180_1A = PLZO_INT64_C(0);
  ltmp_181_A = *(&ltmp_151_1023->field7.field11);
  if ((((plzo_uint64_t )ltmp_181_A) > PLZO_UINT64_C(1))) {
    goto ltmp_227_D;
  } else {
    goto ltmp_228_D;
  }

ltmp_228_D:
  *ltmp_180_1A = PLZO_INT64_C(0);
  goto ltmp_229_D;

ltmp_227_D:
  if ((ltmp_153_A > PLZO_INT64_C(9))) {
    goto ltmp_230_D;
  } else {
    goto ltmp_231_D;
  }

ltmp_231_D:
  *ltmp_180_1A = PLZO_INT64_C(1);
  goto ltmp_229_D;

ltmp_230_D:
  *ltmp_180_1A = PLZO_INT64_C(2);
  goto ltmp_229_D;

ltmp_229_D:
  *(&ltmp_151_1023->field7.field10) = PLZO_INT64_C(0);
  ltmp_182_A = *(&ltmp_151_1023->field6.field1);
  if (((ltmp_182_A & PLZO_INT64_C(1)) == PLZO_INT64_C(0))) {
    goto ltmp_232_D;
  } else {
    goto ltmp_233_D;
  }

ltmp_233_D:
  if ((ltmp_153_A > PLZO_INT64_C(6))) {
    goto ltmp_234_D;
  } else {
    goto ltmp_235_D;
  }

ltmp_235_D:
  *(&ltmp_151_1023->field7.field15) = PLZO_INT64_C(0);
  ltmp_183_9 = *(&ltmp_151_1023->field7.field2);
  ltmp_184_9 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_183_9 > ltmp_184_9)) {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_234_D:
  switch (ltmp_153_A) {
  default:
    goto ltmp_237_D;
;
  case PLZO_INT64_C(7):
    goto ltmp_238_D;
    break;
  case PLZO_INT64_C(8):
    goto ltmp_239_D;
  case PLZO_INT64_C(9):
    goto ltmp_240_D;
  }
ltmp_238_D:
  *(&ltmp_151_1023->field7.field15) = PLZO_INT64_C(1);
  ltmp_185_9 = *(&ltmp_151_1023->field7.field2);
  ltmp_186_9 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_185_9 > ltmp_186_9)) {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_239_D:
  *(&ltmp_151_1023->field7.field15) = PLZO_INT64_C(2);
  ltmp_187_9 = *(&ltmp_151_1023->field7.field2);
  ltmp_188_9 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_187_9 > ltmp_188_9)) {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_240_D:
  *(&ltmp_151_1023->field7.field15) = PLZO_INT64_C(4);
  ltmp_189_9 = *(&ltmp_151_1023->field7.field2);
  ltmp_190_9 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_189_9 > ltmp_190_9)) {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_237_D:
  if ((ltmp_153_A > PLZO_INT64_C(9))) {
    goto ltmp_241_D;
  } else {
    goto ltmp_232_D;
  }

ltmp_241_D:
  *(&ltmp_151_1023->field7.field15) = PLZO_INT64_C(6);
  ltmp_191_9 = *(&ltmp_151_1023->field7.field2);
  ltmp_192_9 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_191_9 > ltmp_192_9)) {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_232_D:
  ltmp_193_9 = *(&ltmp_151_1023->field7.field2);
  ltmp_194_9 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_193_9 > ltmp_194_9)) {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  } else {
    goto ltmp_236_D;
  }

ltmp_236_D:
  ltmp_195_9 = *(&ltmp_151_1023->field7.field4);
  ltmp_196_9 = *(&ltmp_151_1023->field6.field5);
  if ((ltmp_195_9 > ltmp_196_9)) {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  } else {
    goto ltmp_242_D;
  }

ltmp_242_D:
  ltmp_197_9 = *(&ltmp_151_1023->field7.field3);
  ltmp_198_9 = *(&ltmp_151_1023->field6.field4);
  if ((ltmp_197_9 < ltmp_198_9)) {
    ltmp_199_A_2E = PLZO_INT64_C(-2);
    goto ltmp_201_D;
  } else {
    goto ltmp_243_D;
  }

ltmp_243_D:
  return ((((ltmp_197_9 > ltmp_195_9)) ? (PLZO_INT64_C(-2)) : (ltmp_156_A)));
ltmp_201_D:
  ltmp_199_A = ltmp_199_A_2E;
  return ltmp_199_A;
}


static plzo_int64_t ltmp_4_1004(struct l_struct_2E_E *ltmp_244_1023, unsigned char *ltmp_245_13, plzo_uint64_t ltmp_246_9, unsigned char *ltmp_247_13, plzo_uint64_t *ltmp_248_19) {
  plzo_uint64_t *ltmp_249_19;
  unsigned char **ltmp_250_1025;
  unsigned char **ltmp_251_1025;
  plzo_uint64_t *ltmp_252_19;
  plzo_int64_t  (***ltmp_253_102C) ();
  plzo_int64_t  (**ltmp_254_1029) ();
  plzo_int64_t  (*ltmp_255_102A) ();
  plzo_int64_t ltmp_256_A;
  plzo_int64_t ltmp_257_A;
  plzo_int64_t  (**ltmp_258_1029) ();
  plzo_int64_t  (*ltmp_259_102A) ();
  plzo_uint64_t ltmp_260_9;
  plzo_uint64_t ltmp_261_9;
  plzo_int64_t  (**ltmp_262_1029) ();
  plzo_int64_t  (*ltmp_263_102A) ();
  plzo_int64_t ltmp_264_A;
  plzo_int64_t ltmp_265_A;
  plzo_int64_t ltmp_265_A_2E;
  plzo_int64_t  (**ltmp_266_1029) ();
  plzo_int64_t  (*ltmp_267_102A) ();
  plzo_int64_t ltmp_268_A;
  plzo_int64_t ltmp_268_A_2E;
  unsigned char *ltmp_269_13;
  unsigned char *ltmp_270_13;
  plzo_int64_t ltmp_271_A;
  plzo_uint64_t ltmp_272_9;
  signed char *ltmp_273_14;
  struct plzo_callback_t *ltmp_274_1026;
  void  (*ltmp_275_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );
  plzo_int64_t ltmp_276_A;
  plzo_int64_t ltmp_276_A_2E;

  *(&ltmp_244_1023->field10) = ltmp_245_13;
  *(&ltmp_244_1023->field8) = ltmp_245_13;
  ltmp_249_19 = &ltmp_244_1023->field11;
  *ltmp_249_19 = ltmp_246_9;
  ltmp_250_1025 = &ltmp_244_1023->field9;
  ltmp_251_1025 = &ltmp_244_1023->field1.field6;
  *ltmp_251_1025 = ltmp_247_13;
  *ltmp_250_1025 = ltmp_247_13;
  *(&ltmp_244_1023->field12) = PLZO_UINT64_C(0);
  ltmp_252_19 = &ltmp_244_1023->field13;
  *ltmp_252_19 = PLZO_UINT64_C(0);
  ltmp_253_102C = &ltmp_244_1023->field0;
  ltmp_254_1029 = *ltmp_253_102C;
  ltmp_255_102A = *(&ltmp_254_1029[PLZO_INT64_C(7)]);
  ltmp_256_A = ((plzo_int64_t  (*) (struct l_struct_2E_E *))ltmp_255_102A)(ltmp_244_1023);
  if ((ltmp_256_A == PLZO_INT64_C(0))) {
    goto ltmp_277_D;
  } else {
    ltmp_276_A_2E = ltmp_256_A;
    goto ltmp_278_D;
  }

ltmp_277_D:
  ltmp_257_A = *(&ltmp_244_1023->field3);
  if ((ltmp_257_A == PLZO_INT64_C(0))) {
    ltmp_276_A_2E = PLZO_INT64_C(-1);
    goto ltmp_278_D;
  } else {
    goto ltmp_279_D;
  }

ltmp_279_D:
  ltmp_258_1029 = *ltmp_253_102C;
  ltmp_259_102A = *(&ltmp_258_1029[PLZO_INT64_C(8)]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_259_102A)(ltmp_244_1023);
  ltmp_260_9 = *ltmp_249_19;
  ltmp_261_9 = *(&ltmp_244_1023->field6.field2);
  if ((ltmp_260_9 < ltmp_261_9)) {
    goto ltmp_280_D;
  } else {
    goto ltmp_281_D;
  }

ltmp_281_D:
  ltmp_262_1029 = *ltmp_253_102C;
  ltmp_263_102A = *(&ltmp_262_1029[PLZO_INT64_C(9)]);
  ltmp_264_A = ((plzo_int64_t  (*) (struct l_struct_2E_E *))ltmp_263_102A)(ltmp_244_1023);
  if ((ltmp_264_A == PLZO_INT64_C(0))) {
    ltmp_265_A_2E = ltmp_264_A;
    goto ltmp_282_D;
  } else {
    ltmp_268_A_2E = ltmp_264_A;
    goto ltmp_283_D;
  }

ltmp_280_D:
  *ltmp_252_19 = ltmp_260_9;
  if ((ltmp_256_A == PLZO_INT64_C(0))) {
    ltmp_265_A_2E = ltmp_256_A;
    goto ltmp_282_D;
  } else {
    ltmp_268_A_2E = ltmp_256_A;
    goto ltmp_283_D;
  }

ltmp_282_D:
  ltmp_265_A = ltmp_265_A_2E;
  ltmp_266_1029 = *ltmp_253_102C;
  ltmp_267_102A = *(&ltmp_266_1029[PLZO_INT64_C(10)]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_267_102A)(ltmp_244_1023);
  ltmp_268_A_2E = ltmp_265_A;
  goto ltmp_283_D;

ltmp_283_D:
  ltmp_268_A = ltmp_268_A_2E;
  ltmp_269_13 = *ltmp_251_1025;
  ltmp_270_13 = *ltmp_250_1025;
  ltmp_271_A = plzo_external_ptrdiff_uchar(ltmp_269_13, ltmp_270_13, PLZO_UINT64_C(1));
  ltmp_272_9 = (plzo_uint64_t )ltmp_271_A;
  *ltmp_248_19 = ltmp_272_9;
  ltmp_273_14 = *(&ltmp_244_1023->field24.field2.field3);
  ltmp_274_1026 = (struct plzo_callback_t *)ltmp_273_14;
  if ((ltmp_273_14 == ((signed char *)0))) {
    ltmp_276_A_2E = ltmp_268_A;
    goto ltmp_278_D;
  } else {
    goto ltmp_284_D;
  }

ltmp_284_D:
  ltmp_275_101A = *(&ltmp_274_1026->field2);
  if ((ltmp_275_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t ))0))) {
    ltmp_276_A_2E = ltmp_268_A;
    goto ltmp_278_D;
  } else {
    goto ltmp_285_D;
  }

ltmp_285_D:
  ltmp_275_101A(ltmp_274_1026, ltmp_246_9, ltmp_272_9, PLZO_INT64_C(4));
  return ltmp_268_A;
ltmp_278_D:
  ltmp_276_A = ltmp_276_A_2E;
  return ltmp_276_A;
}


static void ltmp_5_1005(struct l_struct_2E_E *ltmp_286_1023) {
  unsigned char **ltmp_287_1025;
  unsigned char *ltmp_288_13;
  signed char *ltmp_289_14;
  signed char **ltmp_290_102D;
  signed char *ltmp_291_14;
  struct plzo_callback_t *ltmp_292_1026;
  void  (*ltmp_293_1019) (struct plzo_callback_t *, signed char *);
  unsigned char **ltmp_294_1025;
  unsigned char *ltmp_295_13;
  signed char *ltmp_296_14;
  signed char *ltmp_297_14;
  struct plzo_callback_t *ltmp_298_1026;
  void  (*ltmp_299_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2 *ltmp_300_102E;
  struct l_struct_2E_A9 *ltmp_301_102F;
  struct l_struct_2E_A2 *ltmp_302_102E;
  struct l_struct_2E_A9 *ltmp_303_102F;
  struct l_struct_2E_A2 *ltmp_304_102E;
  struct l_struct_2E_A2 *ltmp_304_102E_2E;
  struct l_struct_2E_A9 *ltmp_305_102F;
  void  (*ltmp_306_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_307_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_308_1031;

  *(&ltmp_286_1023->field0) = (&(ltmp_70_1021[PLZO_INT64_C(2)]));
  ltmp_287_1025 = &ltmp_286_1023->field22.field0;
  ltmp_288_13 = *ltmp_287_1025;
  ltmp_289_14 = (signed char *)ltmp_288_13;
  ltmp_290_102D = &ltmp_286_1023->field24.field2.field3;
  ltmp_291_14 = *ltmp_290_102D;
  ltmp_292_1026 = (struct plzo_callback_t *)ltmp_291_14;
  if (((ltmp_291_14 == ((signed char *)0)) | (ltmp_288_13 == ((unsigned char *)0)))) {
    goto ltmp_309_D;
  } else {
    goto ltmp_310_D;
  }

ltmp_310_D:
  ltmp_293_1019 = *(&ltmp_292_1026->field1);
  if ((ltmp_293_1019 == ((void  (*) (struct plzo_callback_t *, signed char *))0))) {
    goto ltmp_309_D;
  } else {
    goto ltmp_311_D;
  }

ltmp_311_D:
  ltmp_293_1019(ltmp_292_1026, ltmp_289_14);
  *ltmp_287_1025 = ((unsigned char *)0);
  *(&ltmp_286_1023->field22.field1) = PLZO_UINT64_C(0);
  goto ltmp_312_D;

ltmp_309_D:
  *ltmp_287_1025 = ((unsigned char *)0);
  *(&ltmp_286_1023->field22.field1) = PLZO_UINT64_C(0);
  goto ltmp_312_D;

ltmp_312_D:
  ltmp_294_1025 = &ltmp_286_1023->field23.field0;
  ltmp_295_13 = *ltmp_294_1025;
  ltmp_296_14 = (signed char *)ltmp_295_13;
  ltmp_297_14 = *ltmp_290_102D;
  ltmp_298_1026 = (struct plzo_callback_t *)ltmp_297_14;
  if (((ltmp_297_14 == ((signed char *)0)) | (ltmp_295_13 == ((unsigned char *)0)))) {
    goto ltmp_313_D;
  } else {
    goto ltmp_314_D;
  }

ltmp_314_D:
  ltmp_299_1019 = *(&ltmp_298_1026->field1);
  if ((ltmp_299_1019 == ((void  (*) (struct plzo_callback_t *, signed char *))0))) {
    goto ltmp_313_D;
  } else {
    goto ltmp_315_D;
  }

ltmp_315_D:
  ltmp_299_1019(ltmp_298_1026, ltmp_296_14);
  *ltmp_294_1025 = ((unsigned char *)0);
  *(&ltmp_286_1023->field23.field1) = PLZO_UINT64_C(0);
  ltmp_300_102E = &ltmp_286_1023->field20;
  ltmp_301_102F = *(&ltmp_286_1023->field20.field4);
  if ((ltmp_301_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_316_D;
  } else {
    ltmp_304_102E_2E = ltmp_300_102E;
    goto ltmp_317_D;
  }

ltmp_313_D:
  *ltmp_294_1025 = ((unsigned char *)0);
  *(&ltmp_286_1023->field23.field1) = PLZO_UINT64_C(0);
  ltmp_302_102E = &ltmp_286_1023->field20;
  ltmp_303_102F = *(&ltmp_286_1023->field20.field4);
  if ((ltmp_303_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_316_D;
  } else {
    ltmp_304_102E_2E = ltmp_302_102E;
    goto ltmp_317_D;
  }

ltmp_317_D:
  ltmp_304_102E = ltmp_304_102E_2E;
  ltmp_305_102F = *(&ltmp_304_102E->field4);
  ltmp_306_1019 = *(&ltmp_305_102F->field2.field1);
  ltmp_307_1030 = &ltmp_304_102E->field3;
  ltmp_308_1031 = *ltmp_307_1030;
  ltmp_306_1019((&ltmp_305_102F->field2), ((signed char *)ltmp_308_1031));
  *ltmp_307_1030 = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  return;
ltmp_316_D:
  return;
}


static struct l_struct_2E_E_3A__3A_A12 *ltmp_6_1006(struct l_struct_2E_E *ltmp_318_1023, plzo_int64_t ltmp_319_A) {

  if (((((plzo_uint64_t )ltmp_319_A) + PLZO_UINT64_C(18446744073709551615)) > PLZO_UINT64_C(9))) {
    goto ltmp_320_D;
  } else {
    goto ltmp_321_D;
  }

ltmp_321_D:
  return (&ltmp_71_1022[(ltmp_319_A + PLZO_INT64_C(-1))]);
ltmp_320_D:
  return ((struct l_struct_2E_E_3A__3A_A12 *)0);
}


static void ltmp_7_1005(struct l_struct_2E_E *ltmp_322_1023) {
  plzo_uint64_t *ltmp_323_19;
  plzo_uint64_t ltmp_324_9;
  plzo_uint64_t ltmp_325_9;
  plzo_uint64_t ltmp_325_9_2E;
  plzo_uint64_t ltmp_326_9;
  plzo_uint64_t ltmp_326_9_2E;
  plzo_uint64_t ltmp_327_9;
  plzo_uint64_t ltmp_328_9;
  plzo_uint64_t *ltmp_329_19;
  plzo_uint64_t ltmp_330_9;
  plzo_uint64_t ltmp_331_9;
  plzo_uint64_t ltmp_332_9;
  plzo_uint64_t ltmp_332_9_2E;

  ltmp_323_19 = &ltmp_322_1023->field5;
  ltmp_324_9 = *(&ltmp_322_1023->field7.field3);
  *ltmp_323_19 = ltmp_324_9;
  ltmp_325_9_2E = ltmp_324_9;
  ltmp_326_9_2E = PLZO_UINT64_C(0);
  goto ltmp_333_D;

  do {
ltmp_333_D:
  ltmp_325_9 = ltmp_325_9_2E;
  ltmp_326_9 = ltmp_326_9_2E;
  if ((ltmp_324_9 > ltmp_326_9)) {
    goto ltmp_334_D;
  } else {
    goto ltmp_335_D;
  }

ltmp_334_D:
  *(&ltmp_322_1023->field4[ltmp_326_9]) = PLZO_UINT64_C(0);
  ltmp_327_9 = ltmp_326_9 + PLZO_UINT64_C(1);
  if ((ltmp_327_9 > PLZO_UINT64_C(16))) {
    ltmp_332_9_2E = ltmp_325_9;
    goto ltmp_336_D;
  } else {
    ltmp_325_9_2E = ltmp_325_9;
    ltmp_326_9_2E = ltmp_327_9;
    goto ltmp_333_D;
  }

ltmp_337_D:
  *ltmp_329_19 = (ltmp_328_9 + PLZO_UINT64_C(1));
  ltmp_330_9 = ltmp_326_9 + PLZO_UINT64_C(1);
  if ((ltmp_330_9 > PLZO_UINT64_C(16))) {
    ltmp_332_9_2E = ltmp_326_9;
    goto ltmp_336_D;
  } else {
    ltmp_325_9_2E = ltmp_326_9;
    ltmp_326_9_2E = ltmp_330_9;
    goto ltmp_333_D;
  }

ltmp_335_D:
  ltmp_328_9 = *(&ltmp_322_1023->field6.field7[ltmp_326_9]);
  ltmp_329_19 = &ltmp_322_1023->field4[ltmp_326_9];
  if ((ltmp_328_9 == PLZO_UINT64_C(0))) {
    goto ltmp_338_D;
  } else {
    goto ltmp_337_D;
  }

ltmp_338_D:
  *ltmp_329_19 = PLZO_UINT64_C(18446744073709551615);
  ltmp_331_9 = ltmp_326_9 + PLZO_UINT64_C(1);
  if ((ltmp_331_9 > PLZO_UINT64_C(16))) {
    ltmp_332_9_2E = ltmp_325_9;
    goto ltmp_336_D;
  } else {
    ltmp_325_9_2E = ltmp_325_9;
    ltmp_326_9_2E = ltmp_331_9;
    goto ltmp_333_D;
  }

  } while (1);
ltmp_336_D:
  ltmp_332_9 = ltmp_332_9_2E;
  *ltmp_323_19 = ltmp_332_9;
  return;
}


static void ltmp_8_1005(struct l_struct_2E_E *ltmp_339_1023) {

  return;
}


static plzo_int64_t ltmp_9_1007(struct l_struct_2E_E *ltmp_340_1023) {
  struct l_struct_2E_M ltmp_341_1032;
  struct l_struct_2E_M ltmp_342_1032;
  void  (**ltmp_343_1033) (struct l_struct_2E_A5 *, plzo_uint64_t );
  void  (*ltmp_344_1001) (struct l_struct_2E_A5 *, plzo_uint64_t );
  plzo_uint64_t ltmp_345_9;
  plzo_uint64_t ltmp_346_9;
  void  (*ltmp_347_1001) (struct l_struct_2E_A5 *, plzo_uint64_t );
  void  (**ltmp_348_1033) (struct l_struct_2E_A5 *, plzo_uint64_t );
  void  (*ltmp_349_1001) (struct l_struct_2E_A5 *, plzo_uint64_t );
  void  (*ltmp_350_1001) (struct l_struct_2E_A5 *, plzo_uint64_t );
  plzo_int64_t  (***ltmp_351_102C) ();
  plzo_int64_t  (**ltmp_352_1029) ();
  plzo_int64_t  (*ltmp_353_102A) ();
  plzo_int64_t ltmp_354_A;
  plzo_uint64_t *ltmp_355_19;
  plzo_uint64_t ltmp_356_9;
  plzo_int64_t  (**ltmp_357_1029) ();
  plzo_int64_t  (*ltmp_358_102A) ();
  plzo_uint64_t ltmp_359_9;
  plzo_int64_t ltmp_360_A;
  plzo_int64_t ltmp_360_A_2E;
  struct l_struct_2E_A8 *ltmp_361_1034;
  unsigned char *ltmp_362_13;
  plzo_uint64_t ltmp_363_9;
  plzo_uint64_t ltmp_363_9_2E;
  plzo_uint64_t ltmp_364_9;
  struct l_struct_2E_A9 **ltmp_365_1035;
  struct l_struct_2E_A9 *ltmp_366_102F;
  unsigned char *ltmp_367_13;
  plzo_uint64_t *ltmp_368_19;
  plzo_uint64_t ltmp_369_9;
  signed char * (*ltmp_370_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_371_14;
  struct l_struct_2E_A6 *ltmp_372_1036;
  unsigned char *ltmp_373_13;
  plzo_uint64_t ltmp_374_9;
  plzo_uint64_t ltmp_374_9_2E;
  plzo_uint64_t ltmp_375_9;
  plzo_uint64_t *ltmp_376_19;
  plzo_uint64_t *ltmp_377_19;
  plzo_uint64_t ltmp_378_9;
  plzo_int64_t ltmp_379_A;
  plzo_int64_t ltmp_379_A_2E;
  plzo_uint64_t *ltmp_380_19;
  plzo_uint64_t ltmp_381_9;
  plzo_uint64_t ltmp_382_9;
  plzo_uint64_t ltmp_383_9;
  plzo_uint64_t ltmp_384_9;
  plzo_uint64_t ltmp_385_9;
  plzo_uint64_t ltmp_386_9;
  struct l_struct_2E_A6 *ltmp_387_1036;
  plzo_uint64_t ltmp_388_9;
  plzo_uint64_t ltmp_389_9;
  plzo_uint64_t ltmp_390_9;
  plzo_uint64_t ltmp_390_9_2E;
  plzo_uint64_t ltmp_391_9;
  plzo_uint64_t *ltmp_392_19;
  plzo_uint64_t *ltmp_393_19;
  plzo_uint64_t *ltmp_394_19;
  plzo_uint64_t *ltmp_395_19;
  plzo_uint64_t *ltmp_396_19;
  plzo_uint64_t ltmp_397_9;
  plzo_uint64_t ltmp_398_9;
  plzo_uint64_t ltmp_399_9;
  plzo_uint64_t ltmp_400_9;
  plzo_uint64_t ltmp_400_9_2E;
  plzo_uint64_t *ltmp_401_19;
  plzo_uint64_t ltmp_402_9;
  plzo_uint64_t ltmp_403_9;
  struct l_struct_2E_A7 **ltmp_404_1037;
  struct l_struct_2E_A9 *ltmp_405_102F;
  signed char * (*ltmp_406_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  plzo_uint64_t ltmp_407_9;
  signed char *ltmp_408_14;
  plzo_uint64_t *ltmp_409_19;
  plzo_uint64_t ltmp_410_9;
  plzo_uint64_t ltmp_411_9;
  struct l_struct_2E_A9 *ltmp_412_102F;
  signed char * (*ltmp_413_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_414_14;
  struct l_struct_2E_A9 *ltmp_415_102F;
  void  (*ltmp_416_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A7 *ltmp_417_1038;
  plzo_uint64_t ltmp_418_9;
  plzo_uint64_t ltmp_419_9;
  plzo_int64_t *ltmp_420_1A;
  plzo_uint64_t ltmp_421_9;
  plzo_uint64_t ltmp_422_9;
  unsigned char *ltmp_423_13;
  plzo_uint64_t ltmp_424_9;
  plzo_uint64_t ltmp_424_9_2E;
  plzo_uint64_t ltmp_425_9;
  unsigned char *ltmp_426_13;
  plzo_uint64_t ltmp_427_9;
  plzo_uint64_t ltmp_427_9_2E;
  plzo_uint64_t ltmp_428_9;
  struct l_struct_2E_A7 **ltmp_429_1037;
  struct l_struct_2E_A7 *ltmp_430_1038;
  unsigned char *ltmp_431_13;
  plzo_uint64_t ltmp_432_9;
  plzo_uint64_t ltmp_432_9_2E;
  plzo_uint64_t ltmp_433_9;
  struct l_struct_2E_A7 *ltmp_434_1038;
  plzo_uint64_t ltmp_435_9;
  unsigned char *ltmp_436_13;
  plzo_uint64_t ltmp_437_9;
  plzo_uint64_t ltmp_437_9_2E;
  plzo_uint64_t ltmp_438_9;
  plzo_int64_t ltmp_439_A;
  plzo_int64_t ltmp_439_A_2E;
  plzo_uint64_t ltmp_440_9;
  plzo_int64_t ltmp_441_A;
  plzo_int64_t ltmp_441_A_2E;
  struct l_struct_2E_A9 **ltmp_442_1035;
  signed char **ltmp_443_102D;
  plzo_uint64_t ltmp_444_9;
  plzo_uint64_t ltmp_445_9;
  plzo_uint64_t ltmp_446_9;
  signed char * (*ltmp_447_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_448_14;
  plzo_int64_t ltmp_449_A;
  signed char *ltmp_450_14;
  signed char *ltmp_451_14;
  plzo_int64_t ltmp_452_A;
  plzo_int64_t ltmp_452_A_2E;
  signed char *ltmp_453_14;
  signed char *ltmp_453_14_2E;
  struct plzo_callback_t *ltmp_454_1026;
  void  (*ltmp_455_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );
  plzo_int64_t ltmp_456_A;
  plzo_int64_t ltmp_456_A_2E;
  plzo_int64_t ltmp_457_A;
  plzo_int64_t ltmp_457_A_2E;
  plzo_uint64_t *ltmp_458_19;
  unsigned char **ltmp_459_1025;
  unsigned char **ltmp_460_1025;
  plzo_uint64_t *ltmp_461_19;
  plzo_uint64_t *ltmp_462_19;
  struct l_struct_2E_A6 **ltmp_463_1039;
  plzo_uint64_t *ltmp_464_19;
  struct l_struct_2E_A7 **ltmp_465_1037;
  struct l_struct_2E_M **ltmp_466_103A;
  struct l_struct_2E_M **ltmp_467_103A;
  struct l_struct_2E_M **ltmp_468_103A;
  struct l_struct_2E_M **ltmp_469_103A;
  plzo_uint64_t ltmp_470_9;
  plzo_uint64_t ltmp_470_9_2E;
  plzo_int64_t ltmp_471_A;
  plzo_int64_t ltmp_471_A_2E;
  plzo_uint64_t ltmp_472_9;
  struct l_struct_2E_A6 *ltmp_473_1036;
  plzo_uint64_t ltmp_474_9;
  plzo_uint64_t ltmp_475_9;
  plzo_uint64_t ltmp_476_9;
  plzo_uint64_t ltmp_477_9;
  plzo_uint64_t *ltmp_478_19;
  struct l_struct_2E_A7 *ltmp_479_1038;
  plzo_uint64_t *ltmp_480_19;
  plzo_uint64_t ltmp_481_9;
  plzo_uint64_t ltmp_482_9;
  plzo_uint64_t ltmp_483_9;
  plzo_uint64_t ltmp_484_9;
  plzo_uint64_t ltmp_485_9;
  unsigned char *ltmp_486_13;
  unsigned char *ltmp_487_13;
  plzo_uint64_t ltmp_488_9;
  plzo_uint64_t ltmp_488_9_2E;
  unsigned char ltmp_489_3;
  plzo_uint64_t ltmp_490_9;
  plzo_uint64_t ltmp_491_9;
  plzo_uint64_t ltmp_491_9_2E;
  unsigned char ltmp_492_3;
  plzo_uint64_t ltmp_493_9;
  struct l_struct_2E_A7 *ltmp_494_1038;
  plzo_uint64_t ltmp_495_9;
  plzo_uint64_t ltmp_496_9;
  plzo_uint64_t ltmp_496_9_2E;
  plzo_uint64_t ltmp_497_9;
  plzo_uint64_t ltmp_497_9_2E;
  struct l_struct_2E_A7 *ltmp_498_1038;
  struct l_struct_2E_A7 *ltmp_498_1038_2E;
  plzo_uint64_t *ltmp_499_19;
  struct l_struct_2E_A6 *ltmp_500_1036;
  plzo_uint64_t ltmp_501_9;
  plzo_uint64_t *ltmp_502_19;
  plzo_uint64_t ltmp_503_9;
  plzo_uint64_t *ltmp_504_19;
  plzo_uint64_t *ltmp_505_19;
  unsigned char **ltmp_506_1025;
  plzo_uint64_t *ltmp_507_19;
  plzo_uint64_t *ltmp_508_19;
  unsigned char **ltmp_509_1025;
  unsigned char **ltmp_510_1025;
  plzo_int64_t *ltmp_511_1A;
  plzo_uint64_t *ltmp_512_19;
  plzo_uint64_t *ltmp_513_19;
  plzo_uint64_t *ltmp_514_19;
  struct l_struct_2E_M **ltmp_515_103A;
  plzo_uint64_t *ltmp_516_19;
  plzo_uint64_t *ltmp_517_19;
  plzo_uint64_t *ltmp_518_19;
  plzo_uint64_t *ltmp_519_19;
  plzo_uint64_t ltmp_520_9;
  plzo_uint64_t ltmp_520_9_2E;
  struct l_struct_2E_A7 *ltmp_521_1038;
  struct l_struct_2E_A7 *ltmp_521_1038_2E;
  plzo_uint64_t ltmp_522_9;
  plzo_uint64_t ltmp_523_9;
  plzo_uint64_t ltmp_524_9;
  plzo_uint64_t ltmp_525_9;
  plzo_uint64_t *ltmp_526_19;
  struct l_struct_2E_M *ltmp_527_1032;
  struct l_struct_2E_M **ltmp_528_103A;
  struct l_struct_2E_M **ltmp_529_103A;
  unsigned char *ltmp_530_13;
  plzo_int64_t ltmp_531_A;
  unsigned char *ltmp_532_13;
  plzo_uint64_t ltmp_533_9;
  unsigned char ltmp_534_3;
  plzo_uint64_t *ltmp_535_19;
  plzo_uint64_t ltmp_536_9;
  plzo_uint64_t ltmp_537_9;
  plzo_uint64_t ltmp_538_9;
  plzo_uint64_t ltmp_539_9;
  plzo_uint64_t ltmp_539_9_2E;
  unsigned char ltmp_540_3;
  unsigned char ltmp_541_3;
  plzo_uint64_t *ltmp_542_19;
  plzo_uint64_t ltmp_543_9;
  plzo_uint64_t ltmp_544_9;
  plzo_uint64_t ltmp_545_9;
  unsigned char *ltmp_546_13;
  plzo_int64_t ltmp_547_A;
  plzo_int64_t ltmp_548_A;
  plzo_uint64_t ltmp_549_9;
  plzo_uint64_t ltmp_550_9;
  plzo_uint64_t ltmp_551_9;
  plzo_uint64_t ltmp_552_9;
  plzo_uint64_t ltmp_553_9;
  plzo_uint64_t ltmp_554_9;
  plzo_uint64_t ltmp_555_9;
  plzo_uint64_t ltmp_555_9_2E;
  struct l_struct_2E_M *ltmp_556_1032;
  struct l_struct_2E_M *ltmp_557_1032;
  struct l_struct_2E_M *ltmp_558_1032;
  struct l_struct_2E_M *ltmp_558_1032_2E;
  struct l_struct_2E_M *ltmp_559_1032;
  plzo_int64_t ltmp_560_A;
  plzo_uint64_t ltmp_561_9;
  plzo_uint64_t ltmp_562_9;
  plzo_uint64_t ltmp_563_9;
  plzo_uint64_t ltmp_564_9;
  plzo_uint64_t ltmp_565_9;
  plzo_uint64_t ltmp_565_9_2E;
  plzo_uint64_t ltmp_566_9;
  unsigned char *ltmp_567_13;
  struct l_struct_2E_M *ltmp_568_1032;
  plzo_int64_t ltmp_569_A;
  plzo_uint64_t ltmp_570_9;
  plzo_uint64_t ltmp_571_9;
  plzo_uint64_t ltmp_572_9;
  plzo_uint64_t ltmp_573_9;
  plzo_uint64_t ltmp_574_9;
  plzo_uint64_t ltmp_574_9_2E;
  plzo_uint64_t ltmp_575_9;
  unsigned char *ltmp_576_13;
  unsigned char *ltmp_577_13;
  plzo_uint64_t ltmp_578_9;
  unsigned char *ltmp_579_13;
  unsigned char *ltmp_579_13_2E;
  plzo_uint64_t ltmp_580_9;
  plzo_uint64_t ltmp_580_9_2E;
  plzo_uint64_t ltmp_581_9;
  plzo_uint64_t ltmp_581_9_2E;
  plzo_uint64_t ltmp_582_9;
  plzo_uint64_t ltmp_582_9_2E;
  struct l_struct_2E_M *ltmp_583_1032;
  struct l_struct_2E_M *ltmp_583_1032_2E;
  struct l_struct_2E_M *ltmp_584_1032;
  struct l_struct_2E_M *ltmp_584_1032_2E;
  struct l_struct_2E_M *ltmp_585_1032;
  struct l_struct_2E_M *ltmp_585_1032_2E;
  plzo_uint64_t ltmp_586_9;
  unsigned char *ltmp_587_13;
  unsigned char *ltmp_587_13_2E;
  plzo_uint64_t ltmp_588_9;
  plzo_uint64_t ltmp_588_9_2E;
  plzo_uint64_t ltmp_589_9;
  plzo_uint64_t ltmp_589_9_2E;
  plzo_uint64_t ltmp_590_9;
  plzo_uint64_t ltmp_590_9_2E;
  struct l_struct_2E_M *ltmp_591_1032;
  struct l_struct_2E_M *ltmp_591_1032_2E;
  struct l_struct_2E_M *ltmp_592_1032;
  struct l_struct_2E_M *ltmp_592_1032_2E;
  struct l_struct_2E_M *ltmp_593_1032;
  struct l_struct_2E_M *ltmp_593_1032_2E;
  plzo_uint64_t ltmp_594_9;
  plzo_uint64_t ltmp_594_9_2E;
  plzo_uint64_t ltmp_595_9;
  plzo_uint64_t ltmp_595_9_2E;
  plzo_uint64_t ltmp_596_9;
  unsigned char ltmp_597_3;
  unsigned char ltmp_598_3;
  plzo_uint64_t ltmp_599_9;
  plzo_uint64_t ltmp_600_9;
  plzo_uint64_t ltmp_601_9;
  plzo_uint64_t ltmp_601_9_2E;
  plzo_int64_t ltmp_602_A;
  plzo_uint64_t ltmp_603_9;
  plzo_uint64_t ltmp_604_9;
  plzo_uint64_t ltmp_604_9_2E;
  plzo_uint64_t ltmp_605_9;
  plzo_uint64_t ltmp_606_9;
  plzo_uint64_t ltmp_607_9;
  plzo_uint64_t ltmp_607_9_2E;
  plzo_uint64_t ltmp_608_9;
  plzo_uint64_t ltmp_608_9_2E;
  struct l_struct_2E_M *ltmp_609_1032;
  struct l_struct_2E_M *ltmp_610_1032;
  plzo_int64_t ltmp_611_A;
  plzo_uint64_t ltmp_612_9;
  plzo_uint64_t ltmp_613_9;
  plzo_uint64_t ltmp_614_9;
  plzo_uint64_t ltmp_615_9;
  plzo_uint64_t ltmp_616_9;
  plzo_uint64_t ltmp_616_9_2E;
  plzo_uint64_t ltmp_617_9;
  unsigned char *ltmp_618_13;
  unsigned char *ltmp_619_13;
  plzo_uint64_t ltmp_620_9;
  plzo_uint64_t ltmp_621_9;
  plzo_uint64_t ltmp_621_9_2E;
  plzo_uint64_t ltmp_622_9;
  plzo_uint64_t ltmp_623_9;
  plzo_uint64_t ltmp_624_9;
  plzo_uint64_t ltmp_624_9_2E;
  plzo_uint64_t ltmp_625_9;
  plzo_uint64_t ltmp_625_9_2E;
  struct l_struct_2E_M *ltmp_626_1032;
  struct l_struct_2E_M *ltmp_627_1032;
  plzo_int64_t ltmp_628_A;
  plzo_uint64_t ltmp_629_9;
  plzo_uint64_t ltmp_630_9;
  plzo_uint64_t ltmp_631_9;
  plzo_uint64_t ltmp_632_9;
  plzo_uint64_t ltmp_633_9;
  plzo_uint64_t ltmp_633_9_2E;
  plzo_uint64_t ltmp_634_9;
  unsigned char *ltmp_635_13;
  unsigned char *ltmp_636_13;
  plzo_uint64_t ltmp_637_9;
  plzo_uint64_t ltmp_638_9;
  plzo_uint64_t ltmp_638_9_2E;
  plzo_uint64_t ltmp_639_9;
  struct l_struct_2E_M **ltmp_640_103A;
  struct l_struct_2E_M *ltmp_641_1032;
  struct l_struct_2E_M *ltmp_642_1032;
  plzo_int64_t ltmp_643_A;
  plzo_uint64_t ltmp_644_9;
  plzo_uint64_t ltmp_645_9;
  plzo_uint64_t ltmp_646_9;
  plzo_uint64_t ltmp_647_9;
  plzo_uint64_t ltmp_648_9;
  plzo_uint64_t ltmp_648_9_2E;
  plzo_uint64_t ltmp_649_9;
  unsigned char *ltmp_650_13;
  struct l_struct_2E_M *ltmp_651_1032;
  struct l_struct_2E_M *ltmp_652_1032;
  struct l_struct_2E_M **ltmp_653_103A;
  struct l_struct_2E_M *ltmp_654_1032;
  struct l_struct_2E_M *ltmp_655_1032;
  plzo_int64_t ltmp_656_A;
  plzo_uint64_t ltmp_657_9;
  plzo_uint64_t ltmp_658_9;
  plzo_uint64_t ltmp_659_9;
  plzo_uint64_t ltmp_660_9;
  plzo_uint64_t ltmp_661_9;
  plzo_uint64_t ltmp_661_9_2E;
  plzo_uint64_t ltmp_662_9;
  unsigned char *ltmp_663_13;
  struct l_struct_2E_M *ltmp_664_1032;
  struct l_struct_2E_M *ltmp_665_1032;
  plzo_uint64_t ltmp_666_9;
  plzo_uint64_t ltmp_666_9_2E;
  struct l_struct_2E_M *ltmp_667_1032;
  struct l_struct_2E_M *ltmp_668_1032;
  struct l_struct_2E_M *ltmp_669_1032;
  struct l_struct_2E_M *ltmp_670_1032;
  struct l_struct_2E_M *ltmp_671_1032;
  plzo_int64_t ltmp_672_A;
  unsigned char *ltmp_673_13;
  struct l_struct_2E_M *ltmp_674_1032;
  plzo_int64_t ltmp_675_A;
  unsigned char *ltmp_676_13;
  unsigned char *ltmp_677_13;
  unsigned char *ltmp_678_13;
  unsigned char *ltmp_678_13_2E;
  plzo_uint64_t ltmp_679_9;
  plzo_uint64_t ltmp_679_9_2E;
  plzo_uint64_t ltmp_680_9;
  plzo_uint64_t ltmp_680_9_2E;
  plzo_uint64_t ltmp_681_9;
  plzo_uint64_t ltmp_681_9_2E;
  struct l_struct_2E_M *ltmp_682_1032;
  struct l_struct_2E_M *ltmp_682_1032_2E;
  struct l_struct_2E_M *ltmp_683_1032;
  struct l_struct_2E_M *ltmp_683_1032_2E;
  struct l_struct_2E_M *ltmp_684_1032;
  struct l_struct_2E_M *ltmp_684_1032_2E;
  plzo_uint64_t ltmp_685_9;
  unsigned char *ltmp_686_13;
  unsigned char *ltmp_686_13_2E;
  plzo_uint64_t ltmp_687_9;
  plzo_uint64_t ltmp_687_9_2E;
  plzo_uint64_t ltmp_688_9;
  plzo_uint64_t ltmp_688_9_2E;
  plzo_uint64_t ltmp_689_9;
  plzo_uint64_t ltmp_689_9_2E;
  struct l_struct_2E_M *ltmp_690_1032;
  struct l_struct_2E_M *ltmp_690_1032_2E;
  struct l_struct_2E_M *ltmp_691_1032;
  struct l_struct_2E_M *ltmp_691_1032_2E;
  struct l_struct_2E_M *ltmp_692_1032;
  struct l_struct_2E_M *ltmp_692_1032_2E;
  plzo_uint64_t ltmp_693_9;
  plzo_uint64_t ltmp_693_9_2E;
  plzo_uint64_t ltmp_694_9;
  plzo_uint64_t ltmp_694_9_2E;
  plzo_uint64_t ltmp_695_9;
  unsigned char ltmp_696_3;
  unsigned char ltmp_697_3;
  plzo_uint64_t ltmp_698_9;
  plzo_uint64_t ltmp_699_9;
  plzo_uint64_t ltmp_700_9;
  plzo_uint64_t ltmp_700_9_2E;
  plzo_int64_t ltmp_701_A;
  plzo_uint64_t ltmp_702_9;
  plzo_uint64_t ltmp_703_9;
  plzo_uint64_t ltmp_703_9_2E;
  plzo_uint64_t ltmp_704_9;
  plzo_uint64_t ltmp_705_9;
  plzo_uint64_t ltmp_706_9;
  plzo_uint64_t ltmp_706_9_2E;
  plzo_uint64_t ltmp_707_9;
  plzo_uint64_t ltmp_707_9_2E;
  struct l_struct_2E_M *ltmp_708_1032;
  struct l_struct_2E_M *ltmp_709_1032;
  plzo_int64_t ltmp_710_A;
  unsigned char *ltmp_711_13;
  unsigned char *ltmp_712_13;
  plzo_uint64_t ltmp_713_9;
  plzo_uint64_t ltmp_714_9;
  plzo_uint64_t ltmp_714_9_2E;
  plzo_uint64_t ltmp_715_9;
  plzo_uint64_t ltmp_716_9;
  plzo_uint64_t ltmp_717_9;
  plzo_uint64_t ltmp_717_9_2E;
  plzo_uint64_t ltmp_718_9;
  plzo_uint64_t ltmp_718_9_2E;
  struct l_struct_2E_M *ltmp_719_1032;
  struct l_struct_2E_M *ltmp_720_1032;
  plzo_int64_t ltmp_721_A;
  unsigned char *ltmp_722_13;
  unsigned char *ltmp_723_13;
  plzo_uint64_t ltmp_724_9;
  plzo_uint64_t ltmp_725_9;
  plzo_uint64_t ltmp_725_9_2E;
  plzo_uint64_t ltmp_726_9;
  struct l_struct_2E_M *ltmp_727_1032;
  struct l_struct_2E_M *ltmp_728_1032;
  plzo_uint64_t ltmp_729_9;
  plzo_uint64_t ltmp_729_9_2E;
  struct l_struct_2E_M *ltmp_730_1032;
  struct l_struct_2E_M *ltmp_731_1032;
  plzo_uint64_t ltmp_732_9;
  plzo_uint64_t ltmp_732_9_2E;
  plzo_uint64_t ltmp_733_9;
  plzo_uint64_t ltmp_734_9;
  plzo_uint64_t ltmp_735_9;
  unsigned char *ltmp_736_13;
  unsigned char *ltmp_737_13;
  plzo_uint64_t ltmp_738_9;
  plzo_uint64_t ltmp_739_9;
  plzo_uint64_t ltmp_740_9;
  plzo_uint64_t ltmp_740_9_2E;
  plzo_uint64_t ltmp_741_9;
  plzo_uint64_t ltmp_741_9_2E;
  plzo_uint64_t ltmp_742_9;
  plzo_uint64_t ltmp_742_9_2E;
  plzo_uint64_t ltmp_743_9;
  unsigned char *ltmp_744_13;
  plzo_int64_t ltmp_745_A;
  plzo_uint64_t ltmp_746_9;
  plzo_uint64_t ltmp_747_9;
  plzo_uint64_t ltmp_748_9;
  plzo_int64_t ltmp_749_A;
  plzo_uint64_t ltmp_750_9;
  plzo_uint64_t ltmp_751_9;
  plzo_uint64_t ltmp_752_9;
  plzo_uint64_t ltmp_753_9;
  plzo_uint64_t ltmp_753_9_2E;
  plzo_uint64_t ltmp_754_9;
  plzo_int64_t ltmp_755_A;
  unsigned char ltmp_756_3;
  unsigned char ltmp_757_3;
  plzo_uint64_t ltmp_758_9;
  plzo_uint64_t ltmp_759_9;
  plzo_uint64_t ltmp_760_9;
  plzo_uint64_t ltmp_760_9_2E;
  plzo_uint64_t ltmp_761_9;
  plzo_uint64_t ltmp_762_9;
  plzo_uint64_t ltmp_762_9_2E;
  plzo_uint64_t ltmp_763_9;
  plzo_uint64_t ltmp_763_9_2E;
  plzo_uint64_t ltmp_764_9;
  plzo_uint64_t ltmp_765_9;
  plzo_uint64_t ltmp_766_9;
  plzo_uint64_t ltmp_766_9_2E;
  plzo_uint64_t ltmp_767_9;
  plzo_uint64_t ltmp_767_9_2E;
  plzo_uint64_t ltmp_768_9;
  struct l_struct_2E_M *ltmp_769_1032;
  plzo_uint64_t ltmp_770_9;
  plzo_uint64_t ltmp_771_9;
  plzo_uint64_t ltmp_772_9;
  plzo_uint64_t ltmp_773_9;
  plzo_uint64_t ltmp_774_9;
  plzo_uint64_t ltmp_774_9_2E;
  plzo_int64_t ltmp_775_A;
  struct l_struct_2E_M *ltmp_776_1032;
  plzo_uint64_t ltmp_777_9;
  struct l_struct_2E_A7 *ltmp_778_1038;
  plzo_uint64_t ltmp_779_9;
  plzo_uint64_t ltmp_780_9;
  plzo_uint64_t ltmp_780_9_2E;
  struct l_struct_2E_A7 *ltmp_781_1038;
  struct l_struct_2E_A7 *ltmp_781_1038_2E;
  plzo_uint64_t ltmp_782_9;
  struct l_struct_2E_A7 *ltmp_783_1038;
  plzo_uint64_t ltmp_784_9;
  plzo_uint64_t ltmp_785_9;
  plzo_uint64_t ltmp_786_9;
  plzo_uint64_t ltmp_787_9;
  plzo_uint64_t ltmp_788_9;
  plzo_int64_t  (**ltmp_789_1029) ();
  plzo_int64_t  (*ltmp_790_102A) ();
  plzo_uint64_t ltmp_791_9;
  plzo_uint64_t ltmp_792_9;
  plzo_int64_t  (**ltmp_793_1029) ();
  plzo_int64_t  (*ltmp_794_102A) ();
  plzo_uint64_t ltmp_795_9;
  plzo_uint64_t ltmp_796_9;
  plzo_uint64_t ltmp_796_9_2E;
  unsigned char *ltmp_797_13;
  unsigned char *ltmp_798_13;
  plzo_int64_t ltmp_799_A;
  plzo_uint64_t ltmp_800_9;
  signed char *ltmp_801_14;
  struct plzo_callback_t *ltmp_802_1026;
  void  (*ltmp_803_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );
  plzo_int64_t  (**ltmp_804_1029) ();
  plzo_int64_t  (*ltmp_805_102A) ();
  plzo_uint64_t ltmp_806_9;
  plzo_int64_t  (*ltmp_807_102A) ();
  plzo_uint64_t ltmp_808_9;
  plzo_int64_t  (**ltmp_809_1029) ();
  plzo_int64_t  (*ltmp_810_102A) ();
  plzo_uint64_t ltmp_811_9;
  plzo_int64_t  (*ltmp_812_102A) ();
  plzo_uint64_t ltmp_813_9;
  unsigned char *ltmp_814_13;
  unsigned char *ltmp_815_13;
  plzo_int64_t ltmp_816_A;
  plzo_uint64_t ltmp_817_9;
  signed char *ltmp_818_14;
  struct plzo_callback_t *ltmp_819_1026;
  void  (*ltmp_820_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );
  plzo_int64_t  (**ltmp_821_1029) ();
  plzo_int64_t  (*ltmp_822_102A) ();
  plzo_int64_t  (**ltmp_823_1029) ();
  plzo_int64_t  (*ltmp_824_102A) ();
  plzo_int64_t  (*ltmp_825_102A) ();
  plzo_uint64_t ltmp_826_9;
  plzo_uint64_t ltmp_827_9;
  unsigned char *ltmp_828_13;
  unsigned char *ltmp_829_13;
  plzo_int64_t ltmp_830_A;
  plzo_uint64_t ltmp_831_9;
  signed char *ltmp_832_14;
  struct plzo_callback_t *ltmp_833_1026;
  void  (*ltmp_834_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );
  plzo_int64_t ltmp_835_A;
  plzo_int64_t ltmp_835_A_2E;
  struct l_struct_2E_A9 *ltmp_836_102F;
  void  (*ltmp_837_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_838_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_839_1031;

  ltmp_343_1033 = &ltmp_340_1023->field22.field3;
  ltmp_344_1001 = *ltmp_343_1033;
  ltmp_345_9 = *(&ltmp_340_1023->field1.field2);
  ltmp_346_9 = *(&ltmp_340_1023->field1.field3);
  ltmp_347_1001 = ltmp_54_101D(ltmp_344_1001, ltmp_345_9, ltmp_346_9);
  *ltmp_343_1033 = ltmp_347_1001;
  ltmp_348_1033 = &ltmp_340_1023->field23.field3;
  ltmp_349_1001 = *ltmp_348_1033;
  ltmp_350_1001 = ltmp_54_101D(ltmp_349_1001, ltmp_345_9, ltmp_346_9);
  *ltmp_348_1033 = ltmp_350_1001;
  ltmp_351_102C = &ltmp_340_1023->field0;
  ltmp_352_1029 = *ltmp_351_102C;
  ltmp_353_102A = *(&ltmp_352_1029[PLZO_INT64_C(6)]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_353_102A)(ltmp_340_1023);
  ltmp_354_A = *(&ltmp_340_1023->field7.field11);
  ltmp_355_19 = &ltmp_340_1023->field21;
  ltmp_356_9 = *ltmp_355_19;
  if ((ltmp_356_9 == PLZO_UINT64_C(1))) {
    goto ltmp_840_D;
  } else {
    ltmp_360_A_2E = PLZO_INT64_C(3);
    goto ltmp_841_D;
  }

ltmp_840_D:
  if ((ltmp_354_A == PLZO_INT64_C(2))) {
    goto ltmp_842_D;
  } else {
    ltmp_360_A_2E = ltmp_354_A;
    goto ltmp_841_D;
  }

ltmp_842_D:
  ltmp_357_1029 = *ltmp_351_102C;
  ltmp_358_102A = *(&ltmp_357_1029[PLZO_INT64_C(18)]);
  ltmp_359_9 = ((plzo_uint64_t  (*) (struct l_struct_2E_E *))ltmp_358_102A)(ltmp_340_1023);
  if ((ltmp_359_9 == PLZO_UINT64_C(0))) {
    goto ltmp_843_D;
  } else {
    ltmp_360_A_2E = ltmp_354_A;
    goto ltmp_841_D;
  }

ltmp_843_D:
  ltmp_360_A_2E = PLZO_INT64_C(3);
  goto ltmp_841_D;

ltmp_841_D:
  ltmp_360_A = ltmp_360_A_2E;
  ltmp_361_1034 = &ltmp_340_1023->field19;
  ltmp_362_13 = (unsigned char *)ltmp_361_1034;
  ltmp_363_9_2E = PLZO_UINT64_C(0);
  goto ltmp_844_D;

  do {
ltmp_844_D:
  ltmp_363_9 = ltmp_363_9_2E;
  *(&ltmp_362_13[ltmp_363_9]) = 0;
  ltmp_364_9 = ltmp_363_9 + PLZO_UINT64_C(1);
  if ((ltmp_364_9 == PLZO_UINT64_C(64))) {
    goto ltmp_845_D;
  } else {
    ltmp_363_9_2E = ltmp_364_9;
    goto ltmp_844_D;
  }

  } while (1);
ltmp_845_D:
  ltmp_365_1035 = &ltmp_340_1023->field19.field6;
  ltmp_366_102F = &ltmp_340_1023->field24;
  *ltmp_365_1035 = ltmp_366_102F;
  ltmp_367_13 = *(&ltmp_340_1023->field8);
  ltmp_368_19 = &ltmp_340_1023->field11;
  ltmp_369_9 = *ltmp_368_19;
  if (((ltmp_369_9 + PLZO_UINT64_C(18446744073709551613)) > PLZO_UINT64_C(9223372036854775802))) {
    ltmp_379_A_2E = PLZO_INT64_C(-2);
    goto ltmp_846_D;
  } else {
    goto ltmp_847_D;
  }

ltmp_847_D:
  ltmp_370_1018 = *(&ltmp_340_1023->field24.field2.field0);
  ltmp_371_14 = ltmp_370_1018((&ltmp_340_1023->field24.field2), PLZO_UINT64_C(1), PLZO_UINT64_C(534736));
  ltmp_372_1036 = (struct l_struct_2E_A6 *)ltmp_371_14;
  *(&ltmp_340_1023->field19.field7) = ltmp_372_1036;
  if ((ltmp_371_14 == ((signed char *)0))) {
    ltmp_379_A_2E = PLZO_INT64_C(-3);
    goto ltmp_846_D;
  } else {
    goto ltmp_848_D;
  }

ltmp_848_D:
  ltmp_373_13 = (unsigned char *)ltmp_371_14;
  ltmp_374_9_2E = PLZO_UINT64_C(0);
  goto ltmp_849_D;

  do {
ltmp_849_D:
  ltmp_374_9 = ltmp_374_9_2E;
  *(&ltmp_373_13[ltmp_374_9]) = 0;
  ltmp_375_9 = ltmp_374_9 + PLZO_UINT64_C(1);
  if ((ltmp_375_9 == PLZO_UINT64_C(534736))) {
    goto ltmp_850_D;
  } else {
    ltmp_374_9_2E = ltmp_375_9;
    goto ltmp_849_D;
  }

  } while (1);
ltmp_850_D:
  *(&ltmp_372_1036->field0) = ltmp_367_13;
  *(&ltmp_340_1023->field19.field4) = ltmp_367_13;
  ltmp_376_19 = &ltmp_372_1036->field2;
  *ltmp_376_19 = ltmp_369_9;
  *(&ltmp_340_1023->field19.field5) = ltmp_369_9;
  ltmp_377_19 = &ltmp_372_1036->field3;
  ltmp_378_9 = *ltmp_376_19;
  if ((ltmp_378_9 > PLZO_UINT64_C(3))) {
    goto ltmp_851_D;
  } else {
    goto ltmp_852_D;
  }

ltmp_851_D:
  *ltmp_377_19 = (ltmp_378_9 + PLZO_UINT64_C(18446744073709551613));
  ltmp_379_A_2E = PLZO_INT64_C(0);
  goto ltmp_846_D;

ltmp_852_D:
  *ltmp_377_19 = PLZO_UINT64_C(0);
  ltmp_379_A_2E = PLZO_INT64_C(0);
  goto ltmp_846_D;

ltmp_846_D:
  ltmp_379_A = ltmp_379_A_2E;
  if ((ltmp_379_A == PLZO_INT64_C(0))) {
    goto ltmp_853_D;
  } else {
    ltmp_441_A_2E = ltmp_379_A;
    goto ltmp_854_D;
  }

ltmp_853_D:
  ltmp_380_19 = &ltmp_340_1023->field7.field1;
  ltmp_381_9 = *ltmp_380_19;
  ltmp_382_9 = *(&ltmp_340_1023->field7.field2);
  ltmp_383_9 = *(&ltmp_340_1023->field7.field3);
  ltmp_384_9 = *(&ltmp_340_1023->field7.field4);
  ltmp_385_9 = *(&ltmp_340_1023->field7.field5);
  ltmp_386_9 = *(&ltmp_340_1023->field7.field6);
  ltmp_387_1036 = *(&ltmp_340_1023->field19.field7);
  if ((ltmp_382_9 == PLZO_UINT64_C(0))) {
    goto ltmp_855_D;
  } else {
    goto ltmp_856_D;
  }

ltmp_856_D:
  ltmp_388_9 = *(&ltmp_387_1036->field2);
  if ((ltmp_388_9 < ltmp_382_9)) {
    goto ltmp_855_D;
  } else {
    ltmp_390_9_2E = ltmp_382_9;
    goto ltmp_857_D;
  }

ltmp_855_D:
  ltmp_389_9 = *(&ltmp_387_1036->field2);
  ltmp_390_9_2E = ltmp_389_9;
  goto ltmp_857_D;

ltmp_857_D:
  ltmp_390_9 = ltmp_390_9_2E;
  ltmp_391_9 = (((ltmp_384_9 == PLZO_UINT64_C(0))) ? (PLZO_UINT64_C(18446744073709551615)) : (ltmp_384_9));
  ltmp_392_19 = &ltmp_387_1036->field14;
  *ltmp_392_19 = ltmp_390_9;
  ltmp_393_19 = &ltmp_387_1036->field12;
  *ltmp_393_19 = ltmp_383_9;
  ltmp_394_19 = &ltmp_387_1036->field13;
  *ltmp_394_19 = ltmp_391_9;
  ltmp_395_19 = &ltmp_387_1036->field15;
  ltmp_396_19 = &ltmp_387_1036->field16;
  if ((ltmp_385_9 == PLZO_UINT64_C(0))) {
    goto ltmp_858_D;
  } else {
    goto ltmp_859_D;
  }

ltmp_859_D:
  *ltmp_395_19 = ((((ltmp_391_9 <= ltmp_385_9)) ? (ltmp_391_9) : (ltmp_385_9)));
  if ((ltmp_386_9 == PLZO_UINT64_C(0))) {
    goto ltmp_860_D;
  } else {
    goto ltmp_861_D;
  }

ltmp_858_D:
  *ltmp_395_19 = PLZO_UINT64_C(18446744073709551615);
  if ((ltmp_386_9 == PLZO_UINT64_C(0))) {
    goto ltmp_860_D;
  } else {
    goto ltmp_861_D;
  }

ltmp_861_D:
  ltmp_397_9 = (((ltmp_391_9 <= ltmp_386_9)) ? (ltmp_391_9) : (ltmp_386_9));
  *ltmp_396_19 = ltmp_397_9;
  if ((ltmp_397_9 > PLZO_UINT64_C(1))) {
    goto ltmp_862_D;
  } else {
    goto ltmp_863_D;
  }

ltmp_860_D:
  *ltmp_396_19 = PLZO_UINT64_C(18446744073709551615);
  goto ltmp_862_D;

ltmp_863_D:
  *ltmp_396_19 = PLZO_UINT64_C(2);
  if ((ltmp_381_9 == PLZO_UINT64_C(0))) {
    goto ltmp_864_D;
  } else {
    goto ltmp_865_D;
  }

ltmp_862_D:
  if ((ltmp_381_9 == PLZO_UINT64_C(0))) {
    goto ltmp_864_D;
  } else {
    goto ltmp_865_D;
  }

ltmp_865_D:
  ltmp_398_9 = *(&ltmp_387_1036->field2);
  if ((ltmp_398_9 < ltmp_381_9)) {
    goto ltmp_864_D;
  } else {
    ltmp_400_9_2E = ltmp_381_9;
    goto ltmp_866_D;
  }

ltmp_864_D:
  ltmp_399_9 = *(&ltmp_387_1036->field2);
  ltmp_400_9_2E = ltmp_399_9;
  goto ltmp_866_D;

ltmp_866_D:
  ltmp_400_9 = ltmp_400_9_2E;
  ltmp_401_19 = &ltmp_340_1023->field19.field3;
  *(&ltmp_387_1036->field6) = ltmp_400_9;
  *ltmp_401_19 = ltmp_400_9;
  *(&ltmp_387_1036->field5) = PLZO_UINT64_C(0);
  *(&ltmp_340_1023->field19.field1) = PLZO_UINT64_C(0);
  *(&ltmp_340_1023->field19.field2) = PLZO_UINT64_C(0);
  *(&ltmp_387_1036->field8) = PLZO_UINT64_C(0);
  *(&ltmp_387_1036->field9) = PLZO_UINT64_C(0);
  *(&ltmp_387_1036->field10) = PLZO_UINT64_C(0);
  *(&ltmp_387_1036->field11) = PLZO_UINT64_C(0);
  *(&ltmp_387_1036->field25[PLZO_INT64_C(0)]) = ((unsigned char *)0);
  ltmp_402_9 = *ltmp_393_19;
  if ((ltmp_402_9 == PLZO_UINT64_C(0))) {
    ltmp_439_A_2E = PLZO_INT64_C(-2);
    goto ltmp_867_D;
  } else {
    goto ltmp_868_D;
  }

ltmp_868_D:
  ltmp_403_9 = *ltmp_394_19;
  if (((ltmp_402_9 > ltmp_403_9) | (ltmp_403_9 < PLZO_UINT64_C(4)))) {
    ltmp_439_A_2E = PLZO_INT64_C(-2);
    goto ltmp_867_D;
  } else {
    goto ltmp_869_D;
  }

ltmp_869_D:
  ltmp_404_1037 = &ltmp_387_1036->field17;
  ltmp_405_102F = *ltmp_365_1035;
  ltmp_406_1018 = *(&ltmp_405_102F->field2.field0);
  ltmp_407_9 = *ltmp_401_19;
  ltmp_408_14 = ltmp_406_1018((&ltmp_405_102F->field2), (ltmp_407_9 + PLZO_UINT64_C(2)), PLZO_UINT64_C(72));
  *ltmp_404_1037 = ((struct l_struct_2E_A7 *)ltmp_408_14);
  if ((ltmp_408_14 == ((signed char *)0))) {
    ltmp_439_A_2E = PLZO_INT64_C(-3);
    goto ltmp_867_D;
  } else {
    goto ltmp_870_D;
  }

ltmp_870_D:
  ltmp_409_19 = &ltmp_387_1036->field19;
  ltmp_410_9 = *ltmp_392_19;
  ltmp_411_9 = ltmp_410_9 + PLZO_UINT64_C(1);
  *ltmp_409_19 = ltmp_411_9;
  ltmp_412_102F = *ltmp_365_1035;
  ltmp_413_1018 = *(&ltmp_412_102F->field2.field0);
  ltmp_414_14 = ltmp_413_1018((&ltmp_412_102F->field2), ltmp_411_9, PLZO_UINT64_C(16));
  *(&ltmp_387_1036->field18) = ((struct l_struct_2E_M *)ltmp_414_14);
  if ((ltmp_414_14 == ((signed char *)0))) {
    goto ltmp_871_D;
  } else {
    goto ltmp_872_D;
  }

ltmp_871_D:
  ltmp_415_102F = *ltmp_365_1035;
  ltmp_416_1019 = *(&ltmp_415_102F->field2.field1);
  ltmp_417_1038 = *ltmp_404_1037;
  ltmp_416_1019((&ltmp_415_102F->field2), ((signed char *)ltmp_417_1038));
  *ltmp_404_1037 = ((struct l_struct_2E_A7 *)0);
  ltmp_439_A_2E = PLZO_INT64_C(-3);
  goto ltmp_867_D;

ltmp_872_D:
  ltmp_418_9 = *ltmp_409_19;
  ltmp_419_9 = ltmp_418_9 + PLZO_UINT64_C(18446744073709551615);
  *(&ltmp_387_1036->field20) = ltmp_419_9;
  *(&ltmp_387_1036->field21) = ltmp_419_9;
  *(&ltmp_387_1036->field1) = ltmp_419_9;
  ltmp_420_1A = &ltmp_387_1036->field22;
  *ltmp_420_1A = PLZO_INT64_C(1);
  ltmp_421_9 = *ltmp_392_19;
  ltmp_422_9 = *(&ltmp_387_1036->field2);
  if ((ltmp_421_9 == ltmp_422_9)) {
    goto ltmp_873_D;
  } else {
    goto ltmp_874_D;
  }

ltmp_873_D:
  *ltmp_420_1A = PLZO_INT64_C(0);
  goto ltmp_874_D;

ltmp_874_D:
  ltmp_423_13 = (unsigned char *)(&ltmp_387_1036->field26);
  ltmp_424_9_2E = PLZO_UINT64_C(0);
  goto ltmp_875_D;

  do {
ltmp_875_D:
  ltmp_424_9 = ltmp_424_9_2E;
  *(&ltmp_423_13[ltmp_424_9]) = 255;
  ltmp_425_9 = ltmp_424_9 + PLZO_UINT64_C(1);
  if ((ltmp_425_9 == PLZO_UINT64_C(524288))) {
    goto ltmp_876_D;
  } else {
    ltmp_424_9_2E = ltmp_425_9;
    goto ltmp_875_D;
  }

  } while (1);
ltmp_876_D:
  ltmp_426_13 = (unsigned char *)(&ltmp_387_1036->field27);
  ltmp_427_9_2E = PLZO_UINT64_C(0);
  goto ltmp_877_D;

  do {
ltmp_877_D:
  ltmp_427_9 = ltmp_427_9_2E;
  *(&ltmp_426_13[ltmp_427_9]) = 255;
  ltmp_428_9 = ltmp_427_9 + PLZO_UINT64_C(1);
  if ((ltmp_428_9 == PLZO_UINT64_C(2048))) {
    goto ltmp_878_D;
  } else {
    ltmp_427_9_2E = ltmp_428_9;
    goto ltmp_877_D;
  }

  } while (1);
ltmp_878_D:
  *(&ltmp_387_1036->field23) = PLZO_UINT64_C(0);
  *(&ltmp_387_1036->field24) = PLZO_UINT64_C(0);
  ltmp_429_1037 = &ltmp_340_1023->field19.field0;
  ltmp_430_1038 = *ltmp_404_1037;
  *ltmp_429_1037 = (&ltmp_430_1038[PLZO_INT64_C(1)]);
  ltmp_431_13 = (unsigned char *)ltmp_430_1038;
  ltmp_432_9_2E = PLZO_UINT64_C(0);
  goto ltmp_879_D;

  do {
ltmp_879_D:
  ltmp_432_9 = ltmp_432_9_2E;
  *(&ltmp_431_13[ltmp_432_9]) = 0;
  ltmp_433_9 = ltmp_432_9 + PLZO_UINT64_C(1);
  if ((ltmp_433_9 == PLZO_UINT64_C(72))) {
    goto ltmp_880_D;
  } else {
    ltmp_432_9_2E = ltmp_433_9;
    goto ltmp_879_D;
  }

  } while (1);
ltmp_880_D:
  ltmp_434_1038 = *ltmp_429_1037;
  ltmp_435_9 = *ltmp_401_19;
  ltmp_436_13 = (unsigned char *)(&ltmp_434_1038[ltmp_435_9]);
  ltmp_437_9_2E = PLZO_UINT64_C(0);
  goto ltmp_881_D;

  do {
ltmp_881_D:
  ltmp_437_9 = ltmp_437_9_2E;
  *(&ltmp_436_13[ltmp_437_9]) = 0;
  ltmp_438_9 = ltmp_437_9 + PLZO_UINT64_C(1);
  if ((ltmp_438_9 == PLZO_UINT64_C(72))) {
    ltmp_439_A_2E = PLZO_INT64_C(0);
    goto ltmp_867_D;
  } else {
    ltmp_437_9_2E = ltmp_438_9;
    goto ltmp_881_D;
  }

  } while (1);
ltmp_867_D:
  ltmp_439_A = ltmp_439_A_2E;
  ltmp_440_9 = *ltmp_401_19;
  *ltmp_380_19 = ltmp_440_9;
  ltmp_441_A_2E = ltmp_439_A;
  goto ltmp_854_D;

ltmp_854_D:
  ltmp_441_A = ltmp_441_A_2E;
  ltmp_442_1035 = &ltmp_340_1023->field20.field4;
  *ltmp_442_1035 = ltmp_366_102F;
  ltmp_443_102D = &ltmp_340_1023->field24.field2.field3;
  if (((ltmp_441_A == PLZO_INT64_C(0)) & (ltmp_360_A > PLZO_INT64_C(1)))) {
    goto ltmp_882_D;
  } else {
    goto ltmp_883_D;
  }

ltmp_882_D:
  ltmp_444_9 = *(&ltmp_340_1023->field19.field3);
  ltmp_445_9 = ltmp_444_9 + PLZO_UINT64_C(1);
  ltmp_446_9 = *ltmp_355_19;
  *(&ltmp_340_1023->field20.field0) = ltmp_445_9;
  *(&ltmp_340_1023->field20.field1) = ltmp_446_9;
  *(&ltmp_340_1023->field20.field2) = PLZO_UINT64_C(0);
  ltmp_447_1018 = *(&ltmp_340_1023->field24.field2.field0);
  ltmp_448_14 = ltmp_447_1018((&ltmp_340_1023->field24.field2), (ltmp_446_9*ltmp_445_9), PLZO_UINT64_C(32));
  *(&ltmp_340_1023->field20.field3) = ((struct l_struct_2E_A2_3A__3A_A3 *)ltmp_448_14);
  ltmp_449_A = (((ltmp_448_14 == ((signed char *)0))) ? (PLZO_INT64_C(-3)) : (PLZO_INT64_C(0)));
  ltmp_450_14 = *ltmp_443_102D;
  if ((ltmp_450_14 == ((signed char *)0))) {
    ltmp_456_A_2E = ltmp_449_A;
    goto ltmp_884_D;
  } else {
    ltmp_452_A_2E = ltmp_449_A;
    ltmp_453_14_2E = ltmp_450_14;
    goto ltmp_885_D;
  }

ltmp_883_D:
  ltmp_451_14 = *ltmp_443_102D;
  if ((ltmp_451_14 == ((signed char *)0))) {
    ltmp_456_A_2E = ltmp_441_A;
    goto ltmp_884_D;
  } else {
    ltmp_452_A_2E = ltmp_441_A;
    ltmp_453_14_2E = ltmp_451_14;
    goto ltmp_885_D;
  }

ltmp_885_D:
  ltmp_452_A = ltmp_452_A_2E;
  ltmp_453_14 = ltmp_453_14_2E;
  ltmp_454_1026 = (struct plzo_callback_t *)ltmp_453_14;
  ltmp_455_101A = *(&ltmp_454_1026->field2);
  if ((ltmp_455_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t ))0))) {
    ltmp_456_A_2E = ltmp_452_A;
    goto ltmp_884_D;
  } else {
    goto ltmp_886_D;
  }

ltmp_886_D:
  ltmp_455_101A(ltmp_454_1026, PLZO_UINT64_C(0), PLZO_UINT64_C(0), PLZO_INT64_C(0));
  if ((ltmp_452_A == PLZO_INT64_C(0))) {
    ltmp_457_A_2E = ltmp_452_A;
    goto ltmp_887_D;
  } else {
    ltmp_835_A_2E = ltmp_452_A;
    goto ltmp_888_D;
  }

ltmp_884_D:
  ltmp_456_A = ltmp_456_A_2E;
  if ((ltmp_456_A == PLZO_INT64_C(0))) {
    ltmp_457_A_2E = ltmp_456_A;
    goto ltmp_887_D;
  } else {
    ltmp_835_A_2E = ltmp_456_A;
    goto ltmp_888_D;
  }

ltmp_887_D:
  ltmp_457_A = ltmp_457_A_2E;
  ltmp_458_19 = &ltmp_340_1023->field19.field2;
  ltmp_459_1025 = &ltmp_340_1023->field1.field6;
  ltmp_460_1025 = &ltmp_340_1023->field9;
  ltmp_461_19 = &ltmp_340_1023->field24.field0;
  ltmp_462_19 = &ltmp_340_1023->field24.field1;
  ltmp_463_1039 = &ltmp_340_1023->field19.field7;
  ltmp_464_19 = &ltmp_340_1023->field19.field1;
  ltmp_465_1037 = &ltmp_340_1023->field19.field0;
  ltmp_466_103A = &ltmp_341_1032.field0;
  ltmp_467_103A = &ltmp_341_1032.field1;
  ltmp_468_103A = &ltmp_342_1032.field0;
  ltmp_469_103A = &ltmp_342_1032.field1;
  ltmp_470_9_2E = PLZO_UINT64_C(0);
  ltmp_471_A_2E = ltmp_457_A;
  goto ltmp_889_D;

  do {
ltmp_889_D:
  ltmp_470_9 = ltmp_470_9_2E;
  ltmp_471_A = ltmp_471_A_2E;
  ltmp_472_9 = *ltmp_368_19;
  if ((ltmp_472_9 > ltmp_470_9)) {
    goto ltmp_890_D;
  } else {
    ltmp_835_A_2E = ltmp_471_A;
    goto ltmp_888_D;
  }

ltmp_891_D:
  ltmp_826_9 = *ltmp_461_19;
  ltmp_827_9 = *ltmp_462_19;
  if (((ltmp_796_9 - ltmp_826_9) < ltmp_827_9)) {
    ltmp_470_9_2E = ltmp_796_9;
    ltmp_471_A_2E = PLZO_INT64_C(0);
    goto ltmp_889_D;
  } else {
    goto ltmp_892_D;
  }

ltmp_893_D:
  ltmp_821_1029 = *ltmp_351_102C;
  ltmp_822_102A = *(&ltmp_821_1029[PLZO_INT64_C(16)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t ))ltmp_822_102A)(ltmp_340_1023, ltmp_470_9, ltmp_796_9, PLZO_INT64_C(0));
  goto ltmp_891_D;

ltmp_894_D:
  ltmp_820_101A(ltmp_819_1026, ltmp_796_9, ltmp_817_9, PLZO_INT64_C(2));
  if ((ltmp_360_A == PLZO_INT64_C(0))) {
    goto ltmp_893_D;
  } else {
    goto ltmp_895_D;
  }

ltmp_896_D:
  ltmp_820_101A = *(&ltmp_819_1026->field2);
  if ((ltmp_820_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t ))0))) {
    goto ltmp_897_D;
  } else {
    goto ltmp_894_D;
  }

ltmp_898_D:
  ltmp_814_13 = *ltmp_459_1025;
  ltmp_815_13 = *ltmp_460_1025;
  ltmp_816_A = plzo_external_ptrdiff_uchar(ltmp_814_13, ltmp_815_13, PLZO_UINT64_C(1));
  ltmp_817_9 = (plzo_uint64_t )ltmp_816_A;
  ltmp_818_14 = *ltmp_443_102D;
  ltmp_819_1026 = (struct plzo_callback_t *)ltmp_818_14;
  if ((ltmp_818_14 == ((signed char *)0))) {
    goto ltmp_897_D;
  } else {
    goto ltmp_896_D;
  }

ltmp_899_D:
  ltmp_803_101A(ltmp_802_1026, ltmp_796_9, ltmp_800_9, PLZO_INT64_C(1));
  if ((ltmp_360_A > PLZO_INT64_C(1))) {
    goto ltmp_900_D;
  } else {
    goto ltmp_898_D;
  }

ltmp_901_D:
  ltmp_803_101A = *(&ltmp_802_1026->field2);
  if ((ltmp_803_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t ))0))) {
    goto ltmp_902_D;
  } else {
    goto ltmp_899_D;
  }

ltmp_903_D:
  ltmp_796_9 = ltmp_796_9_2E;
  ltmp_797_13 = *ltmp_459_1025;
  ltmp_798_13 = *ltmp_460_1025;
  ltmp_799_A = plzo_external_ptrdiff_uchar(ltmp_797_13, ltmp_798_13, PLZO_UINT64_C(1));
  ltmp_800_9 = (plzo_uint64_t )ltmp_799_A;
  ltmp_801_14 = *ltmp_443_102D;
  ltmp_802_1026 = (struct plzo_callback_t *)ltmp_801_14;
  if ((ltmp_801_14 == ((signed char *)0))) {
    goto ltmp_902_D;
  } else {
    goto ltmp_901_D;
  }

ltmp_904_D:
  ltmp_789_1029 = *ltmp_351_102C;
  ltmp_790_102A = *(&ltmp_789_1029[PLZO_INT64_C(11)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t ))ltmp_790_102A)(ltmp_340_1023, ltmp_470_9, ltmp_788_9);
  ltmp_791_9 = *ltmp_368_19;
  if (((ltmp_791_9 == ltmp_788_9) | (ltmp_360_A < PLZO_INT64_C(2)))) {
    ltmp_796_9_2E = ltmp_788_9;
    goto ltmp_903_D;
  } else {
    goto ltmp_905_D;
  }

ltmp_906_D:
  ltmp_786_9 = *ltmp_499_19;
  *ltmp_458_19 = ltmp_786_9;
  ltmp_787_9 = *ltmp_499_19;
  *ltmp_480_19 = ltmp_787_9;
  *(&ltmp_473_1036->field11) = (ltmp_787_9 - ltmp_470_9);
  ltmp_788_9 = *ltmp_458_19;
  if ((ltmp_360_A > PLZO_INT64_C(1))) {
    goto ltmp_904_D;
  } else {
    goto ltmp_907_D;
  }

ltmp_908_D:
  ltmp_496_9 = ltmp_496_9_2E;
  ltmp_497_9 = ltmp_497_9_2E;
  ltmp_498_1038 = ltmp_498_1038_2E;
  *(&ltmp_473_1036->field8) = ltmp_497_9;
  ltmp_499_19 = &ltmp_473_1036->field9;
  *ltmp_499_19 = (ltmp_497_9 + ltmp_496_9);
  *(&ltmp_473_1036->field4) = ltmp_496_9;
  ltmp_500_1036 = *ltmp_463_1039;
  ltmp_501_9 = *(&ltmp_500_1036->field8);
  ltmp_502_19 = &ltmp_500_1036->field9;
  ltmp_503_9 = *ltmp_502_19;
  if ((ltmp_503_9 > ltmp_501_9)) {
    goto ltmp_909_D;
  } else {
    goto ltmp_906_D;
  }

ltmp_890_D:
  ltmp_473_1036 = *ltmp_463_1039;
  *(&ltmp_473_1036->field5) = ltmp_470_9;
  *ltmp_464_19 = ltmp_470_9;
  ltmp_474_9 = *(&ltmp_473_1036->field2);
  ltmp_475_9 = ltmp_474_9 - ltmp_470_9;
  ltmp_476_9 = *(&ltmp_473_1036->field6);
  ltmp_477_9 = (((ltmp_476_9 < ltmp_475_9)) ? (ltmp_476_9) : (ltmp_475_9));
  ltmp_478_19 = &ltmp_473_1036->field7;
  *ltmp_478_19 = ltmp_476_9;
  ltmp_479_1038 = *ltmp_465_1037;
  ltmp_480_19 = &ltmp_473_1036->field10;
  ltmp_481_9 = *ltmp_480_19;
  if ((ltmp_481_9 > ltmp_470_9)) {
    goto ltmp_910_D;
  } else {
    ltmp_496_9_2E = ltmp_477_9;
    ltmp_497_9_2E = ltmp_470_9;
    ltmp_498_1038_2E = ltmp_479_1038;
    goto ltmp_908_D;
  }

ltmp_911_D:
  ltmp_494_1038 = &ltmp_479_1038[ltmp_482_9];
  ltmp_495_9 = ltmp_477_9 - ltmp_482_9;
  ltmp_496_9_2E = ltmp_495_9;
  ltmp_497_9_2E = ltmp_481_9;
  ltmp_498_1038_2E = ltmp_494_1038;
  goto ltmp_908_D;

ltmp_910_D:
  ltmp_482_9 = ltmp_481_9 - ltmp_470_9;
  ltmp_483_9 = *(&ltmp_473_1036->field11);
  ltmp_484_9 = ltmp_483_9 - ltmp_482_9;
  *ltmp_478_19 = ltmp_484_9;
  ltmp_485_9 = ltmp_482_9*PLZO_UINT64_C(72);
  ltmp_486_13 = (unsigned char *)ltmp_479_1038;
  ltmp_487_13 = (unsigned char *)(&ltmp_479_1038[ltmp_484_9]);
  if (((ltmp_485_9 == PLZO_UINT64_C(0)) | (ltmp_483_9 == ltmp_482_9))) {
    goto ltmp_911_D;
  } else {
    goto ltmp_912_D;
  }

  do {
ltmp_913_D:
  ltmp_488_9 = ltmp_488_9_2E;
  ltmp_489_3 = *(&ltmp_487_13[ltmp_488_9]);
  *(&ltmp_486_13[ltmp_488_9]) = ltmp_489_3;
  ltmp_490_9 = ltmp_488_9 + PLZO_UINT64_C(1);
  if ((ltmp_490_9 == ltmp_485_9)) {
    goto ltmp_911_D;
  } else {
    ltmp_488_9_2E = ltmp_490_9;
    goto ltmp_913_D;
  }

  } while (1);
ltmp_912_D:
  if ((ltmp_486_13 < ltmp_487_13)) {
    ltmp_488_9_2E = PLZO_UINT64_C(0);
    goto ltmp_913_D;
  } else {
    ltmp_491_9_2E = PLZO_UINT64_C(0);
    goto ltmp_914_D;
  }

  do {
ltmp_914_D:
  ltmp_491_9 = ltmp_491_9_2E;
  ltmp_492_3 = *(&ltmp_487_13[((((plzo_uint64_t )(-(((plzo_int64_t )ltmp_491_9)))) + PLZO_UINT64_C(18446744073709551615)) + ltmp_485_9)]);
  *(&ltmp_486_13[(ltmp_485_9 + (ltmp_491_9 ^ PLZO_UINT64_C(18446744073709551615)))]) = ltmp_492_3;
  ltmp_493_9 = ltmp_491_9 + PLZO_UINT64_C(1);
  if ((ltmp_493_9 == ltmp_485_9)) {
    goto ltmp_911_D;
  } else {
    ltmp_491_9_2E = ltmp_493_9;
    goto ltmp_914_D;
  }

  } while (1);
  do {
ltmp_915_D:
  ltmp_520_9 = ltmp_520_9_2E;
  ltmp_521_1038 = ltmp_521_1038_2E;
  ltmp_522_9 = *ltmp_504_19;
  if ((ltmp_522_9 < ltmp_520_9)) {
    goto ltmp_916_D;
  } else {
    goto ltmp_917_D;
  }

ltmp_918_D:
  ltmp_780_9 = ltmp_780_9_2E;
  ltmp_781_1038 = ltmp_781_1038_2E;
  ltmp_782_9 = ltmp_780_9 + PLZO_UINT64_C(1);
  ltmp_783_1038 = &ltmp_781_1038[PLZO_INT64_C(1)];
  ltmp_784_9 = *ltmp_517_19;
  *ltmp_517_19 = (ltmp_784_9 + PLZO_UINT64_C(18446744073709551615));
  ltmp_785_9 = *ltmp_502_19;
  if ((ltmp_785_9 > ltmp_782_9)) {
    ltmp_520_9_2E = ltmp_782_9;
    ltmp_521_1038_2E = ltmp_783_1038;
    goto ltmp_915_D;
  } else {
    goto ltmp_906_D;
  }

ltmp_919_D:
  ltmp_770_9 = *(&ltmp_521_1038->field0);
  ltmp_771_9 = *ltmp_519_19;
  if ((ltmp_770_9 < ltmp_771_9)) {
    ltmp_780_9_2E = ltmp_520_9;
    ltmp_781_1038_2E = ltmp_521_1038;
    goto ltmp_918_D;
  } else {
    goto ltmp_920_D;
  }

ltmp_921_D:
  *ltmp_526_19 = PLZO_UINT64_C(0);
  goto ltmp_919_D;

ltmp_922_D:
  ltmp_734_9 = *ltmp_526_19;
  ltmp_735_9 = *ltmp_507_19;
  if ((ltmp_734_9 < ltmp_735_9)) {
    goto ltmp_921_D;
  } else {
    goto ltmp_923_D;
  }

ltmp_924_D:
  ltmp_732_9 = ltmp_732_9_2E;
  *ltmp_542_19 = ltmp_520_9;
  *ltmp_526_19 = ltmp_732_9;
  ltmp_733_9 = *ltmp_517_19;
  if ((ltmp_732_9 > ltmp_733_9)) {
    goto ltmp_925_D;
  } else {
    goto ltmp_922_D;
  }

ltmp_926_D:
  ltmp_539_9 = ltmp_539_9_2E;
  ltmp_540_3 = *ltmp_532_13;
  ltmp_541_3 = *(&ltmp_530_13[(ltmp_531_A + PLZO_INT64_C(1))]);
  ltmp_542_19 = &ltmp_500_1036->field26[((((plzo_int64_t )ltmp_541_3) << 8) | ((plzo_int64_t )ltmp_540_3))];
  ltmp_543_9 = *ltmp_542_19;
  if ((ltmp_543_9 < ltmp_520_9)) {
    goto ltmp_927_D;
  } else {
    ltmp_732_9_2E = ltmp_539_9;
    goto ltmp_924_D;
  }

ltmp_917_D:
  ltmp_523_9 = *ltmp_505_19;
  ltmp_524_9 = ltmp_523_9 - ltmp_520_9;
  ltmp_525_9 = (((ltmp_524_9 < PLZO_UINT64_C(1025))) ? (ltmp_524_9) : (PLZO_UINT64_C(1024)));
  ltmp_526_19 = &ltmp_521_1038->field0;
  ltmp_527_1032 = ltmp_18_100A(ltmp_500_1036);
  ltmp_528_103A = &ltmp_527_1032->field0;
  ltmp_529_103A = &ltmp_527_1032->field1;
  *ltmp_529_103A = ((struct l_struct_2E_M *)0);
  *ltmp_528_103A = ((struct l_struct_2E_M *)0);
  ltmp_530_13 = *ltmp_506_1025;
  ltmp_531_A = (plzo_int64_t )ltmp_520_9;
  ltmp_532_13 = &ltmp_530_13[ltmp_520_9];
  ltmp_533_9 = *ltmp_507_19;
  if ((ltmp_533_9 == PLZO_UINT64_C(1))) {
    goto ltmp_928_D;
  } else {
    ltmp_539_9_2E = PLZO_UINT64_C(0);
    goto ltmp_926_D;
  }

ltmp_929_D:
  *ltmp_509_1025 = (&ltmp_530_13[(ltmp_520_9 - ltmp_537_9)]);
  *ltmp_535_19 = ltmp_520_9;
  ltmp_539_9_2E = PLZO_UINT64_C(1);
  goto ltmp_926_D;

ltmp_930_D:
  ltmp_537_9 = ltmp_520_9 - ltmp_536_9;
  ltmp_538_9 = *ltmp_508_19;
  if ((ltmp_538_9 < ltmp_537_9)) {
    goto ltmp_931_D;
  } else {
    goto ltmp_929_D;
  }

ltmp_928_D:
  ltmp_534_3 = *ltmp_532_13;
  ltmp_535_19 = &ltmp_500_1036->field27[((plzo_uint64_t )ltmp_534_3)];
  ltmp_536_9 = *ltmp_535_19;
  if ((ltmp_536_9 < ltmp_520_9)) {
    goto ltmp_930_D;
  } else {
    goto ltmp_931_D;
  }

ltmp_931_D:
  *ltmp_535_19 = ltmp_520_9;
  ltmp_539_9_2E = PLZO_UINT64_C(0);
  goto ltmp_926_D;

ltmp_927_D:
  ltmp_544_9 = ltmp_520_9 - ltmp_543_9;
  ltmp_545_9 = *ltmp_508_19;
  if ((ltmp_545_9 < ltmp_544_9)) {
    ltmp_732_9_2E = ltmp_539_9;
    goto ltmp_924_D;
  } else {
    goto ltmp_932_D;
  }

ltmp_933_D:
  ltmp_666_9 = ltmp_666_9_2E;
  ltmp_667_1032 = *ltmp_467_103A;
  *ltmp_528_103A = ltmp_667_1032;
  ltmp_668_1032 = *ltmp_466_103A;
  *ltmp_529_103A = ltmp_668_1032;
  ltmp_732_9_2E = ltmp_666_9;
  goto ltmp_924_D;

ltmp_934_D:
  *(&ltmp_591_1032->field0) = ltmp_593_1032;
  *(&ltmp_592_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_666_9_2E = ltmp_601_9;
  goto ltmp_933_D;

  do {
ltmp_935_D:
  ltmp_587_13 = ltmp_587_13_2E;
  ltmp_588_9 = ltmp_588_9_2E;
  ltmp_589_9 = ltmp_589_9_2E;
  ltmp_590_9 = ltmp_590_9_2E;
  ltmp_591_1032 = ltmp_591_1032_2E;
  ltmp_592_1032 = ltmp_592_1032_2E;
  ltmp_593_1032 = ltmp_593_1032_2E;
  ltmp_594_9 = ltmp_594_9_2E;
  ltmp_595_9 = ltmp_595_9_2E;
  ltmp_596_9 = ltmp_595_9 + ltmp_594_9;
  ltmp_597_3 = *(&ltmp_567_13[(ltmp_578_9 + ltmp_596_9)]);
  ltmp_598_3 = *(&ltmp_587_13[ltmp_596_9]);
  if ((ltmp_597_3 == ltmp_598_3)) {
    goto ltmp_936_D;
  } else {
    ltmp_601_9_2E = ltmp_596_9;
    goto ltmp_937_D;
  }

ltmp_938_D:
  ltmp_579_13 = ltmp_579_13_2E;
  ltmp_580_9 = ltmp_580_9_2E;
  ltmp_581_9 = ltmp_581_9_2E;
  ltmp_582_9 = ltmp_582_9_2E;
  ltmp_583_1032 = ltmp_583_1032_2E;
  ltmp_584_1032 = ltmp_584_1032_2E;
  ltmp_585_1032 = ltmp_585_1032_2E;
  ltmp_586_9 = (((ltmp_581_9 <= ltmp_580_9)) ? (ltmp_581_9) : (ltmp_580_9));
  ltmp_587_13_2E = ltmp_579_13;
  ltmp_588_9_2E = ltmp_580_9;
  ltmp_589_9_2E = ltmp_581_9;
  ltmp_590_9_2E = ltmp_582_9;
  ltmp_591_1032_2E = ltmp_583_1032;
  ltmp_592_1032_2E = ltmp_584_1032;
  ltmp_593_1032_2E = ltmp_585_1032;
  ltmp_594_9_2E = ltmp_586_9;
  ltmp_595_9_2E = PLZO_UINT64_C(0);
  goto ltmp_935_D;

ltmp_939_D:
  ltmp_616_9 = ltmp_616_9_2E;
  ltmp_617_9 = *ltmp_512_19;
  ltmp_618_13 = *ltmp_506_1025;
  ltmp_619_13 = &ltmp_618_13[(ltmp_617_9 + ltmp_616_9)];
  if ((ltmp_619_13 < ltmp_587_13)) {
    ltmp_579_13_2E = ltmp_619_13;
    ltmp_580_9_2E = ltmp_588_9;
    ltmp_581_9_2E = ltmp_607_9;
    ltmp_582_9_2E = ltmp_608_9;
    ltmp_583_1032_2E = ltmp_593_1032;
    ltmp_584_1032_2E = ltmp_592_1032;
    ltmp_585_1032_2E = ltmp_609_1032;
    goto ltmp_938_D;
  } else {
    goto ltmp_940_D;
  }

ltmp_941_D:
  ltmp_610_1032 = *ltmp_515_103A;
  ltmp_611_A = plzo_external_ptrdiff_M(ltmp_609_1032, ltmp_610_1032, PLZO_UINT64_C(16));
  ltmp_612_9 = *ltmp_513_19;
  ltmp_613_9 = ((plzo_uint64_t )ltmp_611_A) - ltmp_612_9;
  if ((ltmp_613_9 > PLZO_UINT64_C(9223372036854775807))) {
    ltmp_616_9_2E = ltmp_613_9;
    goto ltmp_939_D;
  } else {
    goto ltmp_942_D;
  }

ltmp_943_D:
  ltmp_607_9 = ltmp_607_9_2E;
  ltmp_608_9 = ltmp_608_9_2E;
  *(&ltmp_591_1032->field0) = ltmp_593_1032;
  ltmp_609_1032 = *(&ltmp_593_1032->field0);
  if ((ltmp_609_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_944_D;
  } else {
    goto ltmp_941_D;
  }

ltmp_945_D:
  if ((ltmp_601_9 > ltmp_589_9)) {
    goto ltmp_946_D;
  } else {
    ltmp_607_9_2E = ltmp_589_9;
    ltmp_608_9_2E = ltmp_590_9;
    goto ltmp_943_D;
  }

ltmp_937_D:
  ltmp_601_9 = ltmp_601_9_2E;
  ltmp_602_A = ((plzo_int64_t )ltmp_597_3) - ((plzo_int64_t )ltmp_598_3);
  if ((ltmp_602_A < PLZO_INT64_C(0))) {
    goto ltmp_945_D;
  } else {
    goto ltmp_947_D;
  }

ltmp_936_D:
  ltmp_599_9 = ltmp_596_9 + PLZO_UINT64_C(1);
  ltmp_600_9 = ltmp_595_9 + PLZO_UINT64_C(1);
  if ((ltmp_599_9 < ltmp_525_9)) {
    ltmp_587_13_2E = ltmp_587_13;
    ltmp_588_9_2E = ltmp_588_9;
    ltmp_589_9_2E = ltmp_589_9;
    ltmp_590_9_2E = ltmp_590_9;
    ltmp_591_1032_2E = ltmp_591_1032;
    ltmp_592_1032_2E = ltmp_592_1032;
    ltmp_593_1032_2E = ltmp_593_1032;
    ltmp_594_9_2E = ltmp_594_9;
    ltmp_595_9_2E = ltmp_600_9;
    goto ltmp_935_D;
  } else {
    ltmp_601_9_2E = ltmp_599_9;
    goto ltmp_937_D;
  }

ltmp_946_D:
  if ((ltmp_601_9 > ltmp_590_9)) {
    goto ltmp_948_D;
  } else {
    ltmp_607_9_2E = ltmp_601_9;
    ltmp_608_9_2E = ltmp_590_9;
    goto ltmp_943_D;
  }

ltmp_949_D:
  ltmp_606_9 = *ltmp_516_19;
  if ((ltmp_606_9 > ltmp_601_9)) {
    ltmp_607_9_2E = ltmp_601_9;
    ltmp_608_9_2E = ltmp_601_9;
    goto ltmp_943_D;
  } else {
    goto ltmp_934_D;
  }

ltmp_948_D:
  ltmp_603_9 = ltmp_590_9 + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_603_9]) = ltmp_587_13;
  if ((ltmp_603_9 < ltmp_601_9)) {
    ltmp_604_9_2E = PLZO_UINT64_C(0);
    goto ltmp_950_D;
  } else {
    goto ltmp_949_D;
  }

  do {
ltmp_950_D:
  ltmp_604_9 = ltmp_604_9_2E;
  ltmp_605_9 = (ltmp_604_9 + ltmp_603_9) + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_605_9]) = ltmp_587_13;
  if ((ltmp_605_9 < ltmp_601_9)) {
    ltmp_604_9_2E = (ltmp_604_9 + PLZO_UINT64_C(1));
    goto ltmp_950_D;
  } else {
    goto ltmp_949_D;
  }

  } while (1);
ltmp_942_D:
  ltmp_614_9 = *ltmp_514_19;
  ltmp_615_9 = ltmp_613_9 - ltmp_614_9;
  ltmp_616_9_2E = ltmp_615_9;
  goto ltmp_939_D;

ltmp_951_D:
  ltmp_633_9 = ltmp_633_9_2E;
  ltmp_634_9 = *ltmp_512_19;
  ltmp_635_13 = *ltmp_506_1025;
  ltmp_636_13 = &ltmp_635_13[(ltmp_634_9 + ltmp_633_9)];
  if ((ltmp_636_13 < ltmp_587_13)) {
    ltmp_579_13_2E = ltmp_636_13;
    ltmp_580_9_2E = ltmp_624_9;
    ltmp_581_9_2E = ltmp_589_9;
    ltmp_582_9_2E = ltmp_625_9;
    ltmp_583_1032_2E = ltmp_591_1032;
    ltmp_584_1032_2E = ltmp_593_1032;
    ltmp_585_1032_2E = ltmp_626_1032;
    goto ltmp_938_D;
  } else {
    goto ltmp_952_D;
  }

ltmp_953_D:
  ltmp_627_1032 = *ltmp_515_103A;
  ltmp_628_A = plzo_external_ptrdiff_M(ltmp_626_1032, ltmp_627_1032, PLZO_UINT64_C(16));
  ltmp_629_9 = *ltmp_513_19;
  ltmp_630_9 = ((plzo_uint64_t )ltmp_628_A) - ltmp_629_9;
  if ((ltmp_630_9 > PLZO_UINT64_C(9223372036854775807))) {
    ltmp_633_9_2E = ltmp_630_9;
    goto ltmp_951_D;
  } else {
    goto ltmp_954_D;
  }

ltmp_955_D:
  ltmp_624_9 = ltmp_624_9_2E;
  ltmp_625_9 = ltmp_625_9_2E;
  *(&ltmp_592_1032->field1) = ltmp_593_1032;
  ltmp_626_1032 = *(&ltmp_593_1032->field1);
  if ((ltmp_626_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_956_D;
  } else {
    goto ltmp_953_D;
  }

ltmp_957_D:
  if ((ltmp_601_9 > ltmp_588_9)) {
    goto ltmp_958_D;
  } else {
    ltmp_624_9_2E = ltmp_588_9;
    ltmp_625_9_2E = ltmp_590_9;
    goto ltmp_955_D;
  }

ltmp_947_D:
  if ((ltmp_602_A > PLZO_INT64_C(0))) {
    goto ltmp_957_D;
  } else {
    goto ltmp_959_D;
  }

ltmp_958_D:
  if ((ltmp_601_9 > ltmp_590_9)) {
    goto ltmp_960_D;
  } else {
    ltmp_624_9_2E = ltmp_601_9;
    ltmp_625_9_2E = ltmp_590_9;
    goto ltmp_955_D;
  }

ltmp_961_D:
  ltmp_623_9 = *ltmp_516_19;
  if ((ltmp_623_9 > ltmp_601_9)) {
    ltmp_624_9_2E = ltmp_601_9;
    ltmp_625_9_2E = ltmp_601_9;
    goto ltmp_955_D;
  } else {
    goto ltmp_962_D;
  }

ltmp_960_D:
  ltmp_620_9 = ltmp_590_9 + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_620_9]) = ltmp_587_13;
  if ((ltmp_620_9 < ltmp_601_9)) {
    ltmp_621_9_2E = PLZO_UINT64_C(0);
    goto ltmp_963_D;
  } else {
    goto ltmp_961_D;
  }

  do {
ltmp_963_D:
  ltmp_621_9 = ltmp_621_9_2E;
  ltmp_622_9 = (ltmp_621_9 + ltmp_620_9) + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_622_9]) = ltmp_587_13;
  if ((ltmp_622_9 < ltmp_601_9)) {
    ltmp_621_9_2E = (ltmp_621_9 + PLZO_UINT64_C(1));
    goto ltmp_963_D;
  } else {
    goto ltmp_961_D;
  }

  } while (1);
ltmp_954_D:
  ltmp_631_9 = *ltmp_514_19;
  ltmp_632_9 = ltmp_630_9 - ltmp_631_9;
  ltmp_633_9_2E = ltmp_632_9;
  goto ltmp_951_D;

  } while (1);
ltmp_964_D:
  ltmp_574_9 = ltmp_574_9_2E;
  ltmp_575_9 = *ltmp_512_19;
  ltmp_576_13 = *ltmp_506_1025;
  ltmp_577_13 = &ltmp_576_13[(ltmp_575_9 + ltmp_574_9)];
  ltmp_578_9 = ltmp_566_9 + ltmp_565_9;
  ltmp_587_13_2E = ltmp_577_13;
  ltmp_588_9_2E = PLZO_UINT64_C(2);
  ltmp_589_9_2E = PLZO_UINT64_C(2);
  ltmp_590_9_2E = PLZO_UINT64_C(2);
  ltmp_591_1032_2E = (&ltmp_341_1032);
  ltmp_592_1032_2E = (&ltmp_341_1032);
  ltmp_593_1032_2E = ltmp_558_1032;
  ltmp_594_9_2E = PLZO_UINT64_C(2);
  ltmp_595_9_2E = PLZO_UINT64_C(0);
  goto ltmp_935_D;

ltmp_965_D:
  ltmp_565_9 = ltmp_565_9_2E;
  ltmp_566_9 = *ltmp_512_19;
  ltmp_567_13 = *ltmp_506_1025;
  ltmp_568_1032 = *ltmp_515_103A;
  ltmp_569_A = plzo_external_ptrdiff_M(ltmp_558_1032, ltmp_568_1032, PLZO_UINT64_C(16));
  ltmp_570_9 = *ltmp_513_19;
  ltmp_571_9 = ((plzo_uint64_t )ltmp_569_A) - ltmp_570_9;
  if ((ltmp_571_9 > PLZO_UINT64_C(9223372036854775807))) {
    ltmp_574_9_2E = ltmp_571_9;
    goto ltmp_964_D;
  } else {
    goto ltmp_966_D;
  }

ltmp_967_D:
  ltmp_558_1032 = ltmp_558_1032_2E;
  *ltmp_466_103A = ((struct l_struct_2E_M *)0);
  *ltmp_467_103A = ((struct l_struct_2E_M *)0);
  ltmp_559_1032 = *ltmp_515_103A;
  ltmp_560_A = plzo_external_ptrdiff_M(ltmp_527_1032, ltmp_559_1032, PLZO_UINT64_C(16));
  ltmp_561_9 = *ltmp_513_19;
  ltmp_562_9 = ((plzo_uint64_t )ltmp_560_A) - ltmp_561_9;
  if ((ltmp_562_9 > PLZO_UINT64_C(9223372036854775807))) {
    ltmp_565_9_2E = ltmp_562_9;
    goto ltmp_965_D;
  } else {
    goto ltmp_968_D;
  }

ltmp_969_D:
  ltmp_549_9 = *ltmp_512_19;
  ltmp_550_9 = ((plzo_uint64_t )ltmp_548_A) - ltmp_549_9;
  if ((ltmp_550_9 > PLZO_UINT64_C(9223372036854775807))) {
    goto ltmp_970_D;
  } else {
    ltmp_558_1032_2E = ((struct l_struct_2E_M *)0);
    goto ltmp_967_D;
  }

ltmp_932_D:
  ltmp_546_13 = &ltmp_530_13[(((plzo_int64_t )(-(ltmp_544_9))) + ltmp_531_A)];
  *ltmp_510_1025 = ltmp_546_13;
  ltmp_547_A = *ltmp_511_1A;
  ltmp_548_A = plzo_external_ptrdiff_uchar(ltmp_546_13, ltmp_530_13, PLZO_UINT64_C(1));
  if ((ltmp_547_A == PLZO_INT64_C(0))) {
    goto ltmp_971_D;
  } else {
    goto ltmp_969_D;
  }

ltmp_972_D:
  ltmp_553_9 = *ltmp_514_19;
  ltmp_554_9 = ltmp_553_9 + ltmp_552_9;
  if ((ltmp_554_9 > PLZO_UINT64_C(9223372036854775807))) {
    ltmp_558_1032_2E = ((struct l_struct_2E_M *)0);
    goto ltmp_967_D;
  } else {
    ltmp_555_9_2E = ltmp_554_9;
    goto ltmp_973_D;
  }

ltmp_970_D:
  ltmp_551_9 = *ltmp_513_19;
  ltmp_552_9 = ltmp_551_9 + ltmp_550_9;
  if ((ltmp_552_9 > PLZO_UINT64_C(9223372036854775807))) {
    goto ltmp_972_D;
  } else {
    ltmp_555_9_2E = ltmp_552_9;
    goto ltmp_973_D;
  }

ltmp_973_D:
  ltmp_555_9 = ltmp_555_9_2E;
  ltmp_556_1032 = *ltmp_515_103A;
  ltmp_557_1032 = &ltmp_556_1032[ltmp_555_9];
  ltmp_558_1032_2E = ltmp_557_1032;
  goto ltmp_967_D;

ltmp_968_D:
  ltmp_563_9 = *ltmp_514_19;
  ltmp_564_9 = ltmp_562_9 - ltmp_563_9;
  ltmp_565_9_2E = ltmp_564_9;
  goto ltmp_965_D;

ltmp_966_D:
  ltmp_572_9 = *ltmp_514_19;
  ltmp_573_9 = ltmp_571_9 - ltmp_572_9;
  ltmp_574_9_2E = ltmp_573_9;
  goto ltmp_964_D;

ltmp_944_D:
  *(&ltmp_592_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_666_9_2E = ltmp_608_9;
  goto ltmp_933_D;

ltmp_940_D:
  *(&ltmp_592_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_666_9_2E = ltmp_608_9;
  goto ltmp_933_D;

ltmp_962_D:
  *(&ltmp_592_1032->field1) = ltmp_593_1032;
  *(&ltmp_591_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_666_9_2E = ltmp_601_9;
  goto ltmp_933_D;

ltmp_956_D:
  *(&ltmp_591_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_666_9_2E = ltmp_625_9;
  goto ltmp_933_D;

ltmp_952_D:
  *(&ltmp_591_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_666_9_2E = ltmp_625_9;
  goto ltmp_933_D;

ltmp_974_D:
  ltmp_664_1032 = *ltmp_640_103A;
  *(&ltmp_592_1032->field1) = ltmp_664_1032;
  ltmp_665_1032 = *(&ltmp_593_1032->field1);
  *(&ltmp_591_1032->field0) = ltmp_665_1032;
  ltmp_666_9_2E = ltmp_601_9;
  goto ltmp_933_D;

ltmp_975_D:
  *ltmp_640_103A = ((struct l_struct_2E_M *)0);
  ltmp_651_1032 = *(&ltmp_593_1032->field1);
  if ((ltmp_651_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_974_D;
  } else {
    goto ltmp_976_D;
  }

ltmp_977_D:
  ltmp_648_9 = ltmp_648_9_2E;
  ltmp_649_9 = *ltmp_512_19;
  ltmp_650_13 = *ltmp_506_1025;
  if (((&ltmp_650_13[(ltmp_649_9 + ltmp_648_9)]) < ltmp_587_13)) {
    goto ltmp_978_D;
  } else {
    goto ltmp_975_D;
  }

ltmp_979_D:
  ltmp_642_1032 = *ltmp_515_103A;
  ltmp_643_A = plzo_external_ptrdiff_M(ltmp_641_1032, ltmp_642_1032, PLZO_UINT64_C(16));
  ltmp_644_9 = *ltmp_513_19;
  ltmp_645_9 = ((plzo_uint64_t )ltmp_643_A) - ltmp_644_9;
  if ((ltmp_645_9 > PLZO_UINT64_C(9223372036854775807))) {
    ltmp_648_9_2E = ltmp_645_9;
    goto ltmp_977_D;
  } else {
    goto ltmp_980_D;
  }

ltmp_981_D:
  ltmp_640_103A = &ltmp_593_1032->field0;
  ltmp_641_1032 = *ltmp_640_103A;
  if ((ltmp_641_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_978_D;
  } else {
    goto ltmp_979_D;
  }

ltmp_959_D:
  ltmp_637_9 = ltmp_590_9 + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_637_9]) = ltmp_587_13;
  if ((ltmp_637_9 < ltmp_601_9)) {
    ltmp_638_9_2E = PLZO_UINT64_C(0);
    goto ltmp_982_D;
  } else {
    goto ltmp_981_D;
  }

  do {
ltmp_982_D:
  ltmp_638_9 = ltmp_638_9_2E;
  ltmp_639_9 = (ltmp_638_9 + ltmp_637_9) + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_639_9]) = ltmp_587_13;
  if ((ltmp_639_9 < ltmp_601_9)) {
    ltmp_638_9_2E = (ltmp_638_9 + PLZO_UINT64_C(1));
    goto ltmp_982_D;
  } else {
    goto ltmp_981_D;
  }

  } while (1);
ltmp_980_D:
  ltmp_646_9 = *ltmp_514_19;
  ltmp_647_9 = ltmp_645_9 - ltmp_646_9;
  ltmp_648_9_2E = ltmp_647_9;
  goto ltmp_977_D;

ltmp_978_D:
  ltmp_652_1032 = *(&ltmp_593_1032->field1);
  if ((ltmp_652_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_974_D;
  } else {
    goto ltmp_976_D;
  }

ltmp_983_D:
  ltmp_661_9 = ltmp_661_9_2E;
  ltmp_662_9 = *ltmp_512_19;
  ltmp_663_13 = *ltmp_506_1025;
  if (((&ltmp_663_13[(ltmp_662_9 + ltmp_661_9)]) < ltmp_587_13)) {
    goto ltmp_974_D;
  } else {
    goto ltmp_984_D;
  }

ltmp_976_D:
  ltmp_653_103A = &ltmp_593_1032->field1;
  ltmp_654_1032 = *ltmp_653_103A;
  ltmp_655_1032 = *ltmp_515_103A;
  ltmp_656_A = plzo_external_ptrdiff_M(ltmp_654_1032, ltmp_655_1032, PLZO_UINT64_C(16));
  ltmp_657_9 = *ltmp_513_19;
  ltmp_658_9 = ((plzo_uint64_t )ltmp_656_A) - ltmp_657_9;
  if ((ltmp_658_9 > PLZO_UINT64_C(9223372036854775807))) {
    ltmp_661_9_2E = ltmp_658_9;
    goto ltmp_983_D;
  } else {
    goto ltmp_985_D;
  }

ltmp_985_D:
  ltmp_659_9 = *ltmp_514_19;
  ltmp_660_9 = ltmp_658_9 - ltmp_659_9;
  ltmp_661_9_2E = ltmp_660_9;
  goto ltmp_983_D;

ltmp_984_D:
  *ltmp_653_103A = ((struct l_struct_2E_M *)0);
  goto ltmp_974_D;

ltmp_986_D:
  ltmp_729_9 = ltmp_729_9_2E;
  ltmp_730_1032 = *ltmp_469_103A;
  *ltmp_528_103A = ltmp_730_1032;
  ltmp_731_1032 = *ltmp_468_103A;
  *ltmp_529_103A = ltmp_731_1032;
  ltmp_732_9_2E = ltmp_729_9;
  goto ltmp_924_D;

ltmp_987_D:
  *(&ltmp_690_1032->field0) = ltmp_692_1032;
  *(&ltmp_691_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_729_9_2E = ltmp_700_9;
  goto ltmp_986_D;

  do {
ltmp_988_D:
  ltmp_686_13 = ltmp_686_13_2E;
  ltmp_687_9 = ltmp_687_9_2E;
  ltmp_688_9 = ltmp_688_9_2E;
  ltmp_689_9 = ltmp_689_9_2E;
  ltmp_690_1032 = ltmp_690_1032_2E;
  ltmp_691_1032 = ltmp_691_1032_2E;
  ltmp_692_1032 = ltmp_692_1032_2E;
  ltmp_693_9 = ltmp_693_9_2E;
  ltmp_694_9 = ltmp_694_9_2E;
  ltmp_695_9 = ltmp_694_9 + ltmp_693_9;
  ltmp_696_3 = *(&ltmp_673_13[(((plzo_int64_t )ltmp_695_9) + ltmp_672_A)]);
  ltmp_697_3 = *(&ltmp_686_13[ltmp_695_9]);
  if ((ltmp_696_3 == ltmp_697_3)) {
    goto ltmp_989_D;
  } else {
    ltmp_700_9_2E = ltmp_695_9;
    goto ltmp_990_D;
  }

ltmp_991_D:
  ltmp_678_13 = ltmp_678_13_2E;
  ltmp_679_9 = ltmp_679_9_2E;
  ltmp_680_9 = ltmp_680_9_2E;
  ltmp_681_9 = ltmp_681_9_2E;
  ltmp_682_1032 = ltmp_682_1032_2E;
  ltmp_683_1032 = ltmp_683_1032_2E;
  ltmp_684_1032 = ltmp_684_1032_2E;
  ltmp_685_9 = (((ltmp_680_9 <= ltmp_679_9)) ? (ltmp_680_9) : (ltmp_679_9));
  ltmp_686_13_2E = ltmp_678_13;
  ltmp_687_9_2E = ltmp_679_9;
  ltmp_688_9_2E = ltmp_680_9;
  ltmp_689_9_2E = ltmp_681_9;
  ltmp_690_1032_2E = ltmp_682_1032;
  ltmp_691_1032_2E = ltmp_683_1032;
  ltmp_692_1032_2E = ltmp_684_1032;
  ltmp_693_9_2E = ltmp_685_9;
  ltmp_694_9_2E = PLZO_UINT64_C(0);
  goto ltmp_988_D;

ltmp_992_D:
  ltmp_709_1032 = *ltmp_515_103A;
  ltmp_710_A = plzo_external_ptrdiff_M(ltmp_708_1032, ltmp_709_1032, PLZO_UINT64_C(16));
  ltmp_711_13 = *ltmp_506_1025;
  ltmp_712_13 = &ltmp_711_13[ltmp_710_A];
  ltmp_678_13_2E = ltmp_712_13;
  ltmp_679_9_2E = ltmp_687_9;
  ltmp_680_9_2E = ltmp_706_9;
  ltmp_681_9_2E = ltmp_707_9;
  ltmp_682_1032_2E = ltmp_692_1032;
  ltmp_683_1032_2E = ltmp_691_1032;
  ltmp_684_1032_2E = ltmp_708_1032;
  goto ltmp_991_D;

ltmp_993_D:
  ltmp_706_9 = ltmp_706_9_2E;
  ltmp_707_9 = ltmp_707_9_2E;
  *(&ltmp_690_1032->field0) = ltmp_692_1032;
  ltmp_708_1032 = *(&ltmp_692_1032->field0);
  if ((ltmp_708_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_994_D;
  } else {
    goto ltmp_992_D;
  }

ltmp_995_D:
  if ((ltmp_700_9 > ltmp_688_9)) {
    goto ltmp_996_D;
  } else {
    ltmp_706_9_2E = ltmp_688_9;
    ltmp_707_9_2E = ltmp_689_9;
    goto ltmp_993_D;
  }

ltmp_990_D:
  ltmp_700_9 = ltmp_700_9_2E;
  ltmp_701_A = ((plzo_int64_t )ltmp_696_3) - ((plzo_int64_t )ltmp_697_3);
  if ((ltmp_701_A < PLZO_INT64_C(0))) {
    goto ltmp_995_D;
  } else {
    goto ltmp_997_D;
  }

ltmp_989_D:
  ltmp_698_9 = ltmp_695_9 + PLZO_UINT64_C(1);
  ltmp_699_9 = ltmp_694_9 + PLZO_UINT64_C(1);
  if ((ltmp_698_9 < ltmp_525_9)) {
    ltmp_686_13_2E = ltmp_686_13;
    ltmp_687_9_2E = ltmp_687_9;
    ltmp_688_9_2E = ltmp_688_9;
    ltmp_689_9_2E = ltmp_689_9;
    ltmp_690_1032_2E = ltmp_690_1032;
    ltmp_691_1032_2E = ltmp_691_1032;
    ltmp_692_1032_2E = ltmp_692_1032;
    ltmp_693_9_2E = ltmp_693_9;
    ltmp_694_9_2E = ltmp_699_9;
    goto ltmp_988_D;
  } else {
    ltmp_700_9_2E = ltmp_698_9;
    goto ltmp_990_D;
  }

ltmp_996_D:
  if ((ltmp_700_9 > ltmp_689_9)) {
    goto ltmp_998_D;
  } else {
    ltmp_706_9_2E = ltmp_700_9;
    ltmp_707_9_2E = ltmp_689_9;
    goto ltmp_993_D;
  }

ltmp_999_D:
  ltmp_705_9 = *ltmp_516_19;
  if ((ltmp_705_9 > ltmp_700_9)) {
    ltmp_706_9_2E = ltmp_700_9;
    ltmp_707_9_2E = ltmp_700_9;
    goto ltmp_993_D;
  } else {
    goto ltmp_987_D;
  }

ltmp_998_D:
  ltmp_702_9 = ltmp_689_9 + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_702_9]) = ltmp_686_13;
  if ((ltmp_702_9 < ltmp_700_9)) {
    ltmp_703_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1000_D;
  } else {
    goto ltmp_999_D;
  }

  do {
ltmp_1000_D:
  ltmp_703_9 = ltmp_703_9_2E;
  ltmp_704_9 = (ltmp_703_9 + ltmp_702_9) + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_704_9]) = ltmp_686_13;
  if ((ltmp_704_9 < ltmp_700_9)) {
    ltmp_703_9_2E = (ltmp_703_9 + PLZO_UINT64_C(1));
    goto ltmp_1000_D;
  } else {
    goto ltmp_999_D;
  }

  } while (1);
ltmp_1001_D:
  ltmp_720_1032 = *ltmp_515_103A;
  ltmp_721_A = plzo_external_ptrdiff_M(ltmp_719_1032, ltmp_720_1032, PLZO_UINT64_C(16));
  ltmp_722_13 = *ltmp_506_1025;
  ltmp_723_13 = &ltmp_722_13[ltmp_721_A];
  ltmp_678_13_2E = ltmp_723_13;
  ltmp_679_9_2E = ltmp_717_9;
  ltmp_680_9_2E = ltmp_688_9;
  ltmp_681_9_2E = ltmp_718_9;
  ltmp_682_1032_2E = ltmp_690_1032;
  ltmp_683_1032_2E = ltmp_692_1032;
  ltmp_684_1032_2E = ltmp_719_1032;
  goto ltmp_991_D;

ltmp_1002_D:
  ltmp_717_9 = ltmp_717_9_2E;
  ltmp_718_9 = ltmp_718_9_2E;
  *(&ltmp_691_1032->field1) = ltmp_692_1032;
  ltmp_719_1032 = *(&ltmp_692_1032->field1);
  if ((ltmp_719_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_1003_D;
  } else {
    goto ltmp_1001_D;
  }

ltmp_1004_D:
  if ((ltmp_700_9 > ltmp_687_9)) {
    goto ltmp_1005_D;
  } else {
    ltmp_717_9_2E = ltmp_687_9;
    ltmp_718_9_2E = ltmp_689_9;
    goto ltmp_1002_D;
  }

ltmp_997_D:
  if ((ltmp_701_A > PLZO_INT64_C(0))) {
    goto ltmp_1004_D;
  } else {
    goto ltmp_1006_D;
  }

ltmp_1005_D:
  if ((ltmp_700_9 > ltmp_689_9)) {
    goto ltmp_1007_D;
  } else {
    ltmp_717_9_2E = ltmp_700_9;
    ltmp_718_9_2E = ltmp_689_9;
    goto ltmp_1002_D;
  }

ltmp_1008_D:
  ltmp_716_9 = *ltmp_516_19;
  if ((ltmp_716_9 > ltmp_700_9)) {
    ltmp_717_9_2E = ltmp_700_9;
    ltmp_718_9_2E = ltmp_700_9;
    goto ltmp_1002_D;
  } else {
    goto ltmp_1009_D;
  }

ltmp_1007_D:
  ltmp_713_9 = ltmp_689_9 + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_713_9]) = ltmp_686_13;
  if ((ltmp_713_9 < ltmp_700_9)) {
    ltmp_714_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1010_D;
  } else {
    goto ltmp_1008_D;
  }

  do {
ltmp_1010_D:
  ltmp_714_9 = ltmp_714_9_2E;
  ltmp_715_9 = (ltmp_714_9 + ltmp_713_9) + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_715_9]) = ltmp_686_13;
  if ((ltmp_715_9 < ltmp_700_9)) {
    ltmp_714_9_2E = (ltmp_714_9 + PLZO_UINT64_C(1));
    goto ltmp_1010_D;
  } else {
    goto ltmp_1008_D;
  }

  } while (1);
  } while (1);
ltmp_971_D:
  ltmp_669_1032 = *ltmp_515_103A;
  ltmp_670_1032 = &ltmp_669_1032[ltmp_548_A];
  *ltmp_468_103A = ((struct l_struct_2E_M *)0);
  *ltmp_469_103A = ((struct l_struct_2E_M *)0);
  ltmp_671_1032 = *ltmp_515_103A;
  ltmp_672_A = plzo_external_ptrdiff_M(ltmp_527_1032, ltmp_671_1032, PLZO_UINT64_C(16));
  ltmp_673_13 = *ltmp_506_1025;
  ltmp_674_1032 = *ltmp_515_103A;
  ltmp_675_A = plzo_external_ptrdiff_M(ltmp_670_1032, ltmp_674_1032, PLZO_UINT64_C(16));
  ltmp_676_13 = *ltmp_506_1025;
  ltmp_677_13 = &ltmp_676_13[ltmp_675_A];
  ltmp_686_13_2E = ltmp_677_13;
  ltmp_687_9_2E = PLZO_UINT64_C(2);
  ltmp_688_9_2E = PLZO_UINT64_C(2);
  ltmp_689_9_2E = PLZO_UINT64_C(2);
  ltmp_690_1032_2E = (&ltmp_342_1032);
  ltmp_691_1032_2E = (&ltmp_342_1032);
  ltmp_692_1032_2E = ltmp_670_1032;
  ltmp_693_9_2E = PLZO_UINT64_C(2);
  ltmp_694_9_2E = PLZO_UINT64_C(0);
  goto ltmp_988_D;

ltmp_994_D:
  *(&ltmp_691_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_729_9_2E = ltmp_707_9;
  goto ltmp_986_D;

ltmp_1009_D:
  *(&ltmp_691_1032->field1) = ltmp_692_1032;
  *(&ltmp_690_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_729_9_2E = ltmp_700_9;
  goto ltmp_986_D;

ltmp_1003_D:
  *(&ltmp_690_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_729_9_2E = ltmp_718_9;
  goto ltmp_986_D;

ltmp_1011_D:
  ltmp_727_1032 = *(&ltmp_692_1032->field0);
  *(&ltmp_691_1032->field1) = ltmp_727_1032;
  ltmp_728_1032 = *(&ltmp_692_1032->field1);
  *(&ltmp_690_1032->field0) = ltmp_728_1032;
  ltmp_729_9_2E = ltmp_700_9;
  goto ltmp_986_D;

ltmp_1006_D:
  ltmp_724_9 = ltmp_689_9 + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_724_9]) = ltmp_686_13;
  if ((ltmp_724_9 < ltmp_700_9)) {
    ltmp_725_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1012_D;
  } else {
    goto ltmp_1011_D;
  }

  do {
ltmp_1012_D:
  ltmp_725_9 = ltmp_725_9_2E;
  ltmp_726_9 = (ltmp_725_9 + ltmp_724_9) + PLZO_UINT64_C(1);
  *(&ltmp_500_1036->field25[ltmp_726_9]) = ltmp_686_13;
  if ((ltmp_726_9 < ltmp_700_9)) {
    ltmp_725_9_2E = (ltmp_725_9 + PLZO_UINT64_C(1));
    goto ltmp_1012_D;
  } else {
    goto ltmp_1011_D;
  }

  } while (1);
ltmp_925_D:
  *ltmp_526_19 = ltmp_733_9;
  goto ltmp_922_D;

ltmp_1013_D:
  ltmp_747_9 = *ltmp_526_19;
  if ((ltmp_747_9 == PLZO_UINT64_C(1024))) {
    goto ltmp_1014_D;
  } else {
    goto ltmp_919_D;
  }

  do {
ltmp_1015_D:
  ltmp_740_9 = ltmp_740_9_2E;
  ltmp_741_9 = ltmp_741_9_2E;
  ltmp_742_9 = ltmp_742_9_2E;
  ltmp_743_9 = ltmp_741_9 - ltmp_742_9;
  ltmp_744_13 = *(&ltmp_500_1036->field25[ltmp_743_9]);
  ltmp_745_A = plzo_external_ptrdiff_uchar(ltmp_737_13, ltmp_744_13, PLZO_UINT64_C(1));
  *(&ltmp_521_1038->field1[ltmp_742_9]) = ((plzo_uint64_t )ltmp_745_A);
  ltmp_746_9 = ltmp_742_9 + PLZO_UINT64_C(1);
  if ((ltmp_746_9 > PLZO_UINT64_C(7))) {
    goto ltmp_1013_D;
  } else {
    goto ltmp_1016_D;
  }

ltmp_1016_D:
  if (((ltmp_743_9 + PLZO_UINT64_C(18446744073709551615)) < ltmp_740_9)) {
    goto ltmp_1013_D;
  } else {
    ltmp_740_9_2E = ltmp_740_9;
    ltmp_741_9_2E = ltmp_741_9;
    ltmp_742_9_2E = ltmp_746_9;
    goto ltmp_1015_D;
  }

  } while (1);
ltmp_923_D:
  ltmp_736_13 = *ltmp_506_1025;
  ltmp_737_13 = &ltmp_736_13[ltmp_520_9];
  ltmp_738_9 = *ltmp_518_19;
  if ((ltmp_734_9 > ltmp_738_9)) {
    goto ltmp_1017_D;
  } else {
    ltmp_740_9_2E = ltmp_735_9;
    ltmp_741_9_2E = ltmp_734_9;
    ltmp_742_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1015_D;
  }

ltmp_1017_D:
  *ltmp_526_19 = ltmp_738_9;
  ltmp_739_9 = *ltmp_507_19;
  ltmp_740_9_2E = ltmp_739_9;
  ltmp_741_9_2E = ltmp_738_9;
  ltmp_742_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1015_D;

ltmp_1018_D:
  ltmp_760_9 = ltmp_760_9_2E;
  if ((ltmp_747_9 < ltmp_760_9)) {
    goto ltmp_1019_D;
  } else {
    goto ltmp_919_D;
  }

ltmp_1014_D:
  ltmp_748_9 = *(&ltmp_521_1038->field1[PLZO_INT64_C(0)]);
  ltmp_749_A = ((plzo_int64_t )(-(ltmp_748_9))) + ltmp_531_A;
  ltmp_750_9 = *ltmp_517_19;
  ltmp_751_9 = *ltmp_518_19;
  ltmp_752_9 = (((ltmp_750_9 <= ltmp_751_9)) ? (ltmp_750_9) : (ltmp_751_9));
  if ((ltmp_747_9 < ltmp_752_9)) {
    ltmp_753_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1020_D;
  } else {
    ltmp_760_9_2E = ltmp_747_9;
    goto ltmp_1018_D;
  }

  do {
ltmp_1020_D:
  ltmp_753_9 = ltmp_753_9_2E;
  ltmp_754_9 = ltmp_753_9 + ltmp_747_9;
  ltmp_755_A = (plzo_int64_t )ltmp_754_9;
  ltmp_756_3 = *(&ltmp_736_13[(ltmp_755_A + ltmp_531_A)]);
  ltmp_757_3 = *(&ltmp_736_13[(ltmp_755_A + ltmp_749_A)]);
  if ((ltmp_756_3 == ltmp_757_3)) {
    goto ltmp_1021_D;
  } else {
    ltmp_760_9_2E = ltmp_754_9;
    goto ltmp_1018_D;
  }

ltmp_1021_D:
  ltmp_758_9 = ltmp_754_9 + PLZO_UINT64_C(1);
  ltmp_759_9 = ltmp_753_9 + PLZO_UINT64_C(1);
  if ((ltmp_758_9 < ltmp_752_9)) {
    ltmp_753_9_2E = ltmp_759_9;
    goto ltmp_1020_D;
  } else {
    ltmp_760_9_2E = ltmp_758_9;
    goto ltmp_1018_D;
  }

  } while (1);
ltmp_1022_D:
  *ltmp_526_19 = ltmp_760_9;
  goto ltmp_919_D;

  do {
ltmp_1023_D:
  ltmp_766_9 = ltmp_766_9_2E;
  ltmp_767_9 = ltmp_767_9_2E;
  *(&ltmp_521_1038->field1[((ltmp_766_9 - ltmp_767_9) + PLZO_UINT64_C(18446744073709551615))]) = ltmp_748_9;
  ltmp_768_9 = ltmp_767_9 + PLZO_UINT64_C(1);
  if ((ltmp_768_9 == ltmp_766_9)) {
    goto ltmp_1022_D;
  } else {
    ltmp_766_9_2E = ltmp_766_9;
    ltmp_767_9_2E = ltmp_768_9;
    goto ltmp_1023_D;
  }

  } while (1);
ltmp_1019_D:
  ltmp_761_9 = ltmp_760_9 - ltmp_747_9;
  if ((ltmp_761_9 < PLZO_UINT64_C(8))) {
    goto ltmp_1024_D;
  } else {
    ltmp_766_9_2E = PLZO_UINT64_C(8);
    ltmp_767_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1023_D;
  }

  do {
ltmp_1025_D:
  ltmp_762_9 = ltmp_762_9_2E;
  ltmp_763_9 = ltmp_763_9_2E;
  ltmp_764_9 = (ltmp_762_9 - ltmp_763_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_765_9 = *(&ltmp_521_1038->field1[(ltmp_764_9 - ltmp_761_9)]);
  *(&ltmp_521_1038->field1[ltmp_764_9]) = ltmp_765_9;
  if ((ltmp_764_9 > ltmp_761_9)) {
    ltmp_762_9_2E = ltmp_762_9;
    ltmp_763_9_2E = (ltmp_763_9 + PLZO_UINT64_C(1));
    goto ltmp_1025_D;
  } else {
    ltmp_766_9_2E = ltmp_764_9;
    ltmp_767_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1023_D;
  }

  } while (1);
ltmp_1024_D:
  if ((ltmp_760_9 < PLZO_UINT64_C(8))) {
    goto ltmp_1026_D;
  } else {
    ltmp_762_9_2E = PLZO_UINT64_C(8);
    ltmp_763_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1025_D;
  }

ltmp_1026_D:
  ltmp_762_9_2E = ltmp_760_9;
  ltmp_763_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1025_D;

ltmp_916_D:
  *(&ltmp_521_1038->field0) = PLZO_UINT64_C(0);
  ltmp_769_1032 = ltmp_18_100A(ltmp_500_1036);
  *(&ltmp_769_1032->field1) = ((struct l_struct_2E_M *)0);
  *(&ltmp_769_1032->field0) = ((struct l_struct_2E_M *)0);
  goto ltmp_919_D;

ltmp_1027_D:
  ltmp_778_1038 = &ltmp_521_1038[ltmp_775_A];
  ltmp_779_9 = (ltmp_770_9 + ltmp_520_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_780_9_2E = ltmp_779_9;
  ltmp_781_1038_2E = ltmp_778_1038;
  goto ltmp_918_D;

  do {
ltmp_1028_D:
  ltmp_774_9 = ltmp_774_9_2E;
  ltmp_775_A = ((plzo_int64_t )ltmp_774_9) + PLZO_INT64_C(1);
  *(&ltmp_521_1038[ltmp_775_A].field0) = PLZO_UINT64_C(0);
  ltmp_776_1032 = ltmp_18_100A(ltmp_500_1036);
  *(&ltmp_776_1032->field1) = ((struct l_struct_2E_M *)0);
  *(&ltmp_776_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_777_9 = ltmp_774_9 + PLZO_UINT64_C(1);
  if ((ltmp_777_9 == ltmp_772_9)) {
    goto ltmp_1027_D;
  } else {
    ltmp_774_9_2E = ltmp_777_9;
    goto ltmp_1028_D;
  }

  } while (1);
ltmp_920_D:
  ltmp_772_9 = ltmp_770_9 + PLZO_UINT64_C(18446744073709551615);
  ltmp_773_9 = *ltmp_517_19;
  *ltmp_517_19 = (ltmp_773_9 - ltmp_772_9);
  ltmp_774_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1028_D;

  } while (1);
ltmp_909_D:
  ltmp_504_19 = &ltmp_500_1036->field3;
  ltmp_505_19 = &ltmp_500_1036->field2;
  ltmp_506_1025 = &ltmp_500_1036->field0;
  ltmp_507_19 = &ltmp_500_1036->field12;
  ltmp_508_19 = &ltmp_500_1036->field14;
  ltmp_509_1025 = &ltmp_500_1036->field25[PLZO_INT64_C(1)];
  ltmp_510_1025 = &ltmp_500_1036->field25[PLZO_INT64_C(2)];
  ltmp_511_1A = &ltmp_500_1036->field22;
  ltmp_512_19 = &ltmp_500_1036->field1;
  ltmp_513_19 = &ltmp_500_1036->field21;
  ltmp_514_19 = &ltmp_500_1036->field19;
  ltmp_515_103A = &ltmp_500_1036->field18;
  ltmp_516_19 = &ltmp_500_1036->field15;
  ltmp_517_19 = &ltmp_500_1036->field4;
  ltmp_518_19 = &ltmp_500_1036->field13;
  ltmp_519_19 = &ltmp_500_1036->field16;
  ltmp_520_9_2E = ltmp_501_9;
  ltmp_521_1038_2E = ltmp_498_1038;
  goto ltmp_915_D;

ltmp_907_D:
  ltmp_792_9 = *ltmp_368_19;
  if (((ltmp_792_9 == ltmp_788_9) | (ltmp_360_A < PLZO_INT64_C(2)))) {
    ltmp_796_9_2E = ltmp_788_9;
    goto ltmp_903_D;
  } else {
    goto ltmp_905_D;
  }

ltmp_905_D:
  ltmp_793_1029 = *ltmp_351_102C;
  ltmp_794_102A = *(&ltmp_793_1029[PLZO_INT64_C(19)]);
  ltmp_795_9 = ((plzo_uint64_t  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t ))ltmp_794_102A)(ltmp_340_1023, ltmp_470_9, ltmp_788_9);
  ltmp_796_9_2E = ltmp_795_9;
  goto ltmp_903_D;

ltmp_902_D:
  if ((ltmp_360_A > PLZO_INT64_C(1))) {
    goto ltmp_900_D;
  } else {
    goto ltmp_898_D;
  }

ltmp_1029_D:
  ltmp_805_102A = *(&ltmp_804_1029[PLZO_INT64_C(14)]);
  ltmp_806_9 = ((plzo_uint64_t  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t ))ltmp_805_102A)(ltmp_340_1023, ltmp_470_9, ltmp_796_9);
  goto ltmp_898_D;

ltmp_900_D:
  ltmp_804_1029 = *ltmp_351_102C;
  if ((ltmp_360_A == PLZO_INT64_C(2))) {
    goto ltmp_1029_D;
  } else {
    goto ltmp_1030_D;
  }

ltmp_1031_D:
  ltmp_810_102A = *(&ltmp_809_1029[PLZO_INT64_C(13)]);
  ltmp_811_9 = ((plzo_uint64_t  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t ))ltmp_810_102A)(ltmp_340_1023, ltmp_470_9, ltmp_796_9);
  goto ltmp_898_D;

ltmp_1030_D:
  ltmp_807_102A = *(&ltmp_804_1029[PLZO_INT64_C(18)]);
  ltmp_808_9 = ((plzo_uint64_t  (*) (struct l_struct_2E_E *))ltmp_807_102A)(ltmp_340_1023);
  ltmp_809_1029 = *ltmp_351_102C;
  if ((ltmp_808_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1032_D;
  } else {
    goto ltmp_1031_D;
  }

ltmp_1032_D:
  ltmp_812_102A = *(&ltmp_809_1029[PLZO_INT64_C(12)]);
  ltmp_813_9 = ((plzo_uint64_t  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t ))ltmp_812_102A)(ltmp_340_1023, ltmp_470_9, ltmp_796_9);
  goto ltmp_898_D;

ltmp_897_D:
  if ((ltmp_360_A == PLZO_INT64_C(0))) {
    goto ltmp_893_D;
  } else {
    goto ltmp_895_D;
  }

ltmp_1033_D:
  ltmp_824_102A = *(&ltmp_823_1029[PLZO_INT64_C(16)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t ))ltmp_824_102A)(ltmp_340_1023, ltmp_470_9, ltmp_796_9, PLZO_INT64_C(1));
  goto ltmp_891_D;

ltmp_895_D:
  ltmp_823_1029 = *ltmp_351_102C;
  if ((ltmp_360_A == PLZO_INT64_C(1))) {
    goto ltmp_1033_D;
  } else {
    goto ltmp_1034_D;
  }

ltmp_1034_D:
  ltmp_825_102A = *(&ltmp_823_1029[PLZO_INT64_C(15)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t ))ltmp_825_102A)(ltmp_340_1023, ltmp_470_9, ltmp_796_9);
  goto ltmp_891_D;

ltmp_1035_D:
  ltmp_834_101A(ltmp_833_1026, ltmp_796_9, ltmp_831_9, PLZO_INT64_C(3));
  *ltmp_461_19 = ltmp_796_9;
  ltmp_470_9_2E = ltmp_796_9;
  ltmp_471_A_2E = PLZO_INT64_C(0);
  goto ltmp_889_D;

ltmp_1036_D:
  ltmp_834_101A = *(&ltmp_833_1026->field2);
  if ((ltmp_834_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t ))0))) {
    goto ltmp_1037_D;
  } else {
    goto ltmp_1035_D;
  }

ltmp_892_D:
  ltmp_828_13 = *ltmp_459_1025;
  ltmp_829_13 = *ltmp_460_1025;
  ltmp_830_A = plzo_external_ptrdiff_uchar(ltmp_828_13, ltmp_829_13, PLZO_UINT64_C(1));
  ltmp_831_9 = (plzo_uint64_t )ltmp_830_A;
  ltmp_832_14 = *ltmp_443_102D;
  ltmp_833_1026 = (struct plzo_callback_t *)ltmp_832_14;
  if ((ltmp_832_14 == ((signed char *)0))) {
    goto ltmp_1037_D;
  } else {
    goto ltmp_1036_D;
  }

ltmp_1037_D:
  *ltmp_461_19 = ltmp_796_9;
  ltmp_470_9_2E = ltmp_796_9;
  ltmp_471_A_2E = PLZO_INT64_C(0);
  goto ltmp_889_D;

  } while (1);
ltmp_888_D:
  ltmp_835_A = ltmp_835_A_2E;
  ltmp_836_102F = *ltmp_442_1035;
  if ((ltmp_836_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_1038_D;
  } else {
    goto ltmp_1039_D;
  }

ltmp_1039_D:
  ltmp_837_1019 = *(&ltmp_836_102F->field2.field1);
  ltmp_838_1030 = &ltmp_340_1023->field20.field3;
  ltmp_839_1031 = *ltmp_838_1030;
  ltmp_837_1019((&ltmp_836_102F->field2), ((signed char *)ltmp_839_1031));
  *ltmp_838_1030 = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  ltmp_19_100B(ltmp_361_1034);
  return ltmp_835_A;
ltmp_1038_D:
  ltmp_19_100B(ltmp_361_1034);
  return ltmp_835_A;
}


static void ltmp_10_1005(struct l_struct_2E_E *ltmp_1040_1023) {
  plzo_uint64_t *ltmp_1041_19;
  plzo_uint64_t ltmp_1042_9;
  plzo_uint64_t ltmp_1043_9;
  plzo_uint64_t ltmp_1044_9;
  plzo_int64_t  (**ltmp_1045_1029) ();
  plzo_int64_t  (*ltmp_1046_102A) ();
  unsigned char *ltmp_1047_13;
  plzo_int64_t  (**ltmp_1048_1029) ();
  plzo_int64_t  (*ltmp_1049_102A) ();

  ltmp_1041_19 = &ltmp_1040_1023->field13;
  ltmp_1042_9 = *ltmp_1041_19;
  if ((ltmp_1042_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1050_D;
  } else {
    goto ltmp_1051_D;
  }

ltmp_1051_D:
  ltmp_1043_9 = *(&ltmp_1040_1023->field11);
  ltmp_1044_9 = ltmp_1043_9 - ltmp_1042_9;
  *(&ltmp_1040_1023->field12) = ltmp_1044_9;
  ltmp_1045_1029 = *(&ltmp_1040_1023->field0);
  ltmp_1046_102A = *(&ltmp_1045_1029[PLZO_INT64_C(3)]);
  ltmp_1047_13 = *(&ltmp_1040_1023->field8);
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint64_t , plzo_uint64_t ))ltmp_1046_102A)(ltmp_1040_1023, (&ltmp_1047_13[ltmp_1044_9]), ltmp_1042_9, PLZO_UINT64_C(0));
  *ltmp_1041_19 = PLZO_UINT64_C(0);
  goto ltmp_1050_D;

ltmp_1050_D:
  ltmp_1048_1029 = *(&ltmp_1040_1023->field0);
  ltmp_1049_102A = *(&ltmp_1048_1029[PLZO_INT64_C(5)]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_1049_102A)(ltmp_1040_1023);
  return;
}


static void ltmp_11_1000(struct l_struct_2E_E *ltmp_1052_1023, plzo_uint64_t ltmp_1053_9, plzo_uint64_t ltmp_1054_9) {
  struct l_struct_2E_A7 *ltmp_1055_1038;
  struct l_struct_2E_A7 *ltmp_1056_1038;
  struct l_struct_2E_A7 *ltmp_1056_1038_2E;
  plzo_uint64_t ltmp_1057_9;
  plzo_uint64_t *ltmp_1058_19;
  plzo_uint64_t ltmp_1059_9;
  plzo_uint64_t ltmp_1059_9_2E;
  struct l_struct_2E_A7 *ltmp_1060_1038;
  struct l_struct_2E_A7 *ltmp_1060_1038_2E;
  plzo_uint64_t ltmp_1061_9;
  plzo_uint64_t *ltmp_1062_19;
  plzo_uint64_t ltmp_1063_9;
  plzo_uint64_t ltmp_1064_9;
  plzo_uint64_t ltmp_1065_9;
  plzo_uint64_t ltmp_1066_9;
  struct l_struct_2E_A7 *ltmp_1067_1038;
  struct l_struct_2E_A7 *ltmp_1068_1038;

  ltmp_1055_1038 = *(&ltmp_1052_1023->field19.field0);
  if ((ltmp_1053_9 < ltmp_1054_9)) {
    goto ltmp_1069_D;
  } else {
    goto ltmp_1070_D;
  }

ltmp_1069_D:
  ltmp_1058_19 = &ltmp_1052_1023->field5;
  ltmp_1059_9_2E = PLZO_UINT64_C(0);
  ltmp_1060_1038_2E = ltmp_1055_1038;
  goto ltmp_1071_D;

  do {
ltmp_1071_D:
  ltmp_1059_9 = ltmp_1059_9_2E;
  ltmp_1060_1038 = ltmp_1060_1038_2E;
  ltmp_1061_9 = ltmp_1059_9 + ltmp_1053_9;
  ltmp_1062_19 = &ltmp_1060_1038->field0;
  ltmp_1063_9 = *ltmp_1062_19;
  if ((ltmp_1063_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1072_D;
  } else {
    goto ltmp_1073_D;
  }

ltmp_1074_D:
  ltmp_1056_1038 = ltmp_1056_1038_2E;
  ltmp_1057_9 = ltmp_1059_9 + PLZO_UINT64_C(1);
  if (((ltmp_1061_9 + PLZO_UINT64_C(1)) < ltmp_1054_9)) {
    ltmp_1059_9_2E = ltmp_1057_9;
    ltmp_1060_1038_2E = ltmp_1056_1038;
    goto ltmp_1071_D;
  } else {
    goto ltmp_1070_D;
  }

ltmp_1075_D:
  *ltmp_1062_19 = PLZO_UINT64_C(0);
  ltmp_1067_1038 = &ltmp_1060_1038[PLZO_INT64_C(1)];
  ltmp_1056_1038_2E = ltmp_1067_1038;
  goto ltmp_1074_D;

ltmp_1076_D:
  ltmp_1065_9 = *(&ltmp_1060_1038->field1[PLZO_INT64_C(0)]);
  ltmp_1066_9 = *(&ltmp_1052_1023->field4[ltmp_1063_9]);
  if ((ltmp_1065_9 < ltmp_1066_9)) {
    goto ltmp_1072_D;
  } else {
    goto ltmp_1075_D;
  }

ltmp_1073_D:
  ltmp_1064_9 = *ltmp_1058_19;
  if ((ltmp_1063_9 > ltmp_1064_9)) {
    goto ltmp_1072_D;
  } else {
    goto ltmp_1076_D;
  }

ltmp_1072_D:
  ltmp_1068_1038 = &ltmp_1060_1038[PLZO_INT64_C(1)];
  ltmp_1056_1038_2E = ltmp_1068_1038;
  goto ltmp_1074_D;

  } while (1);
ltmp_1070_D:
  return;
}


static plzo_uint64_t ltmp_12_1008(struct l_struct_2E_E *ltmp_1077_1023, plzo_uint64_t ltmp_1078_9, plzo_uint64_t ltmp_1079_9) {
  struct l_struct_2E_E_3A__3A_A4 ltmp_1080_103B;
  plzo_uint64_t ltmp_1081_9;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1082_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1083_1031;
  plzo_uint64_t *ltmp_1084_19;
  plzo_uint64_t ltmp_1085_9;
  plzo_uint64_t ltmp_1086_9;
  unsigned char *ltmp_1087_13;
  plzo_uint64_t ltmp_1088_9;
  plzo_uint64_t ltmp_1088_9_2E;
  plzo_uint64_t ltmp_1089_9;
  unsigned char *ltmp_1090_13;
  plzo_uint64_t ltmp_1091_9;
  plzo_uint64_t ltmp_1091_9_2E;
  plzo_uint64_t ltmp_1092_9;
  plzo_uint64_t ltmp_1093_9;
  struct l_struct_2E_A7 **ltmp_1094_1037;
  plzo_uint64_t *ltmp_1095_19;
  plzo_uint64_t *ltmp_1096_19;
  plzo_uint64_t *ltmp_1097_19;
  plzo_uint64_t *ltmp_1098_19;
  plzo_int64_t  (***ltmp_1099_102C) ();
  plzo_uint64_t *ltmp_1100_19;
  plzo_uint64_t *ltmp_1101_19;
  plzo_int64_t *ltmp_1102_1A;
  plzo_uint64_t *ltmp_1103_19;
  plzo_uint64_t *ltmp_1104_19;
  plzo_uint64_t ltmp_1105_9;
  plzo_uint64_t ltmp_1105_9_2E;
  plzo_uint64_t ltmp_1106_9;
  struct l_struct_2E_A7 *ltmp_1107_1038;
  plzo_uint64_t ltmp_1108_9;
  plzo_uint64_t ltmp_1109_9;
  plzo_uint64_t ltmp_1110_9;
  plzo_uint64_t ltmp_1111_9;
  plzo_uint64_t ltmp_1112_9;
  plzo_uint64_t ltmp_1113_9;
  plzo_uint64_t ltmp_1114_9;
  plzo_uint64_t ltmp_1115_9;
  plzo_uint64_t ltmp_1115_9_2E;
  plzo_uint64_t ltmp_1116_9;
  plzo_uint64_t ltmp_1116_9_2E;
  plzo_uint64_t ltmp_1117_9;
  plzo_uint64_t ltmp_1118_9;
  plzo_uint64_t ltmp_1119_9;
  plzo_uint64_t ltmp_1120_9;
  plzo_uint64_t ltmp_1120_9_2E;
  plzo_uint64_t ltmp_1121_9;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1122_1031;
  plzo_uint64_t ltmp_1123_9;
  plzo_uint64_t ltmp_1124_9;
  plzo_int64_t  (**ltmp_1125_1029) ();
  plzo_int64_t  (*ltmp_1126_102A) ();
  plzo_uint64_t ltmp_1127_9;
  plzo_uint64_t *ltmp_1128_19;
  plzo_uint64_t *ltmp_1129_19;
  plzo_uint64_t *ltmp_1130_19;
  plzo_uint64_t *ltmp_1131_19;
  plzo_uint64_t ltmp_1132_9;
  plzo_uint64_t ltmp_1133_9;
  plzo_uint64_t ltmp_1134_9;
  plzo_uint64_t ltmp_1134_9_2E;
  plzo_uint64_t ltmp_1135_9;
  plzo_int64_t  (**ltmp_1136_1029) ();
  plzo_int64_t  (*ltmp_1137_102A) ();
  plzo_uint64_t ltmp_1138_9;
  plzo_uint64_t ltmp_1139_9;
  plzo_uint64_t ltmp_1140_9;
  plzo_int64_t ltmp_1141_A;
  plzo_uint64_t ltmp_1142_9;
  plzo_uint64_t ltmp_1143_9;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1144_1031;
  plzo_uint64_t ltmp_1145_9;

  ltmp_1081_9 = ltmp_1079_9 - ltmp_1078_9;
  *(&ltmp_1077_1023->field20.field2) = (ltmp_1081_9 + PLZO_UINT64_C(1));
  ltmp_1082_1030 = &ltmp_1077_1023->field20.field3;
  ltmp_1083_1031 = *ltmp_1082_1030;
  ltmp_1084_19 = &ltmp_1077_1023->field20.field1;
  ltmp_1085_9 = *ltmp_1084_19;
  ltmp_1086_9 = ltmp_1085_9 << 5;
  ltmp_1087_13 = (unsigned char *)(&ltmp_1083_1031[(ltmp_1085_9*ltmp_1081_9)]);
  if ((ltmp_1086_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1146_D;
  } else {
    ltmp_1088_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1147_D;
  }

  do {
ltmp_1147_D:
  ltmp_1088_9 = ltmp_1088_9_2E;
  *(&ltmp_1087_13[ltmp_1088_9]) = 0;
  ltmp_1089_9 = ltmp_1088_9 + PLZO_UINT64_C(1);
  if ((ltmp_1089_9 == ltmp_1086_9)) {
    goto ltmp_1146_D;
  } else {
    ltmp_1088_9_2E = ltmp_1089_9;
    goto ltmp_1147_D;
  }

  } while (1);
ltmp_1146_D:
  ltmp_1090_13 = (unsigned char *)(&ltmp_1080_103B);
  ltmp_1091_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1148_D;

  do {
ltmp_1148_D:
  ltmp_1091_9 = ltmp_1091_9_2E;
  *(&ltmp_1090_13[ltmp_1091_9]) = 0;
  ltmp_1092_9 = ltmp_1091_9 + PLZO_UINT64_C(1);
  if ((ltmp_1092_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1149_D;
  } else {
    ltmp_1091_9_2E = ltmp_1092_9;
    goto ltmp_1148_D;
  }

  } while (1);
ltmp_1149_D:
  *(&ltmp_1080_103B.field3) = ltmp_1078_9;
  *(&ltmp_1080_103B.field4) = ltmp_1079_9;
  *(&ltmp_1080_103B.field5) = ltmp_1081_9;
  if ((ltmp_1079_9 == ltmp_1078_9)) {
    goto ltmp_1150_D;
  } else {
    goto ltmp_1151_D;
  }

ltmp_1151_D:
  ltmp_1094_1037 = &ltmp_1077_1023->field19.field0;
  ltmp_1095_19 = &ltmp_1077_1023->field7.field12;
  ltmp_1096_19 = &ltmp_1077_1023->field7.field13;
  ltmp_1097_19 = &ltmp_1077_1023->field7.field6;
  ltmp_1098_19 = &ltmp_1077_1023->field21;
  ltmp_1099_102C = &ltmp_1077_1023->field0;
  ltmp_1100_19 = &ltmp_1080_103B.field0;
  ltmp_1101_19 = &ltmp_1077_1023->field7.field3;
  ltmp_1102_1A = &ltmp_1077_1023->field7.field14;
  ltmp_1103_19 = &ltmp_1080_103B.field1;
  ltmp_1104_19 = &ltmp_1080_103B.field2;
  ltmp_1105_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1152_D;

  do {
ltmp_1152_D:
  ltmp_1105_9 = ltmp_1105_9_2E;
  ltmp_1106_9 = (ltmp_1081_9 - ltmp_1105_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_1107_1038 = *ltmp_1094_1037;
  ltmp_1108_9 = *(&ltmp_1107_1038[ltmp_1106_9].field0);
  ltmp_1109_9 = *ltmp_1095_19;
  ltmp_1110_9 = *ltmp_1096_19;
  ltmp_1111_9 = *ltmp_1097_19;
  if ((ltmp_1111_9 > ltmp_1108_9)) {
    ltmp_1115_9_2E = ltmp_1110_9;
    ltmp_1116_9_2E = ltmp_1109_9;
    goto ltmp_1153_D;
  } else {
    goto ltmp_1154_D;
  }

ltmp_1155_D:
  ltmp_1093_9 = ltmp_1105_9 + PLZO_UINT64_C(1);
  if ((ltmp_1093_9 == ltmp_1081_9)) {
    goto ltmp_1150_D;
  } else {
    ltmp_1105_9_2E = ltmp_1093_9;
    goto ltmp_1152_D;
  }

ltmp_1153_D:
  ltmp_1115_9 = ltmp_1115_9_2E;
  ltmp_1116_9 = ltmp_1116_9_2E;
  ltmp_1117_9 = ((((ltmp_1116_9 + ltmp_1115_9) < ltmp_1108_9)) ? ((ltmp_1108_9 - ltmp_1115_9)) : (ltmp_1116_9));
  ltmp_1118_9 = *ltmp_1098_19;
  if ((ltmp_1118_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1155_D;
  } else {
    ltmp_1120_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1156_D;
  }

ltmp_1154_D:
  ltmp_1112_9 = *ltmp_1101_19;
  ltmp_1113_9 = ltmp_1112_9 + PLZO_UINT64_C(1);
  ltmp_1114_9 = (plzo_uint64_t )(ltmp_1110_9 != PLZO_UINT64_C(0));
  ltmp_1115_9_2E = ltmp_1114_9;
  ltmp_1116_9_2E = ltmp_1113_9;
  goto ltmp_1153_D;

  do {
ltmp_1156_D:
  ltmp_1120_9 = ltmp_1120_9_2E;
  ltmp_1121_9 = (ltmp_1118_9 - ltmp_1120_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_1122_1031 = *ltmp_1082_1030;
  ltmp_1123_9 = *ltmp_1084_19;
  ltmp_1124_9 = (ltmp_1123_9*ltmp_1106_9) + ltmp_1121_9;
  ltmp_1125_1029 = *ltmp_1099_102C;
  ltmp_1126_102A = *(&ltmp_1125_1029[PLZO_INT64_C(17)]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint64_t , plzo_uint64_t , plzo_uint64_t ))ltmp_1126_102A)(ltmp_1077_1023, (&ltmp_1080_103B), PLZO_UINT64_C(0), ltmp_1106_9, ltmp_1121_9);
  ltmp_1127_9 = *ltmp_1100_19;
  ltmp_1128_19 = &ltmp_1122_1031[ltmp_1124_9].field0;
  *ltmp_1128_19 = ltmp_1127_9;
  ltmp_1129_19 = &ltmp_1122_1031[ltmp_1124_9].field1;
  *ltmp_1129_19 = PLZO_UINT64_C(0);
  ltmp_1130_19 = &ltmp_1122_1031[ltmp_1124_9].field2;
  *ltmp_1130_19 = PLZO_UINT64_C(0);
  ltmp_1131_19 = &ltmp_1122_1031[ltmp_1124_9].field3;
  *ltmp_1131_19 = PLZO_UINT64_C(0);
  ltmp_1132_9 = *ltmp_1101_19;
  if ((ltmp_1132_9 > ltmp_1108_9)) {
    goto ltmp_1157_D;
  } else {
    ltmp_1134_9_2E = ltmp_1132_9;
    goto ltmp_1158_D;
  }

ltmp_1157_D:
  ltmp_1119_9 = ltmp_1120_9 + PLZO_UINT64_C(1);
  if ((ltmp_1119_9 == ltmp_1118_9)) {
    goto ltmp_1155_D;
  } else {
    ltmp_1120_9_2E = ltmp_1119_9;
    goto ltmp_1156_D;
  }

  do {
ltmp_1158_D:
  ltmp_1134_9 = ltmp_1134_9_2E;
  ltmp_1135_9 = (((ltmp_1134_9 == ltmp_1116_9)) ? (ltmp_1117_9) : (ltmp_1134_9));
  ltmp_1136_1029 = *ltmp_1099_102C;
  ltmp_1137_102A = *(&ltmp_1136_1029[PLZO_INT64_C(17)]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint64_t , plzo_uint64_t , plzo_uint64_t ))ltmp_1137_102A)(ltmp_1077_1023, (&ltmp_1080_103B), ltmp_1135_9, ltmp_1106_9, ltmp_1121_9);
  ltmp_1138_9 = *ltmp_1100_19;
  ltmp_1139_9 = *ltmp_1128_19;
  if ((ltmp_1138_9 > ltmp_1139_9)) {
    goto ltmp_1159_D;
  } else {
    goto ltmp_1160_D;
  }

ltmp_1159_D:
  ltmp_1133_9 = ltmp_1135_9 + PLZO_UINT64_C(1);
  if ((ltmp_1133_9 > ltmp_1108_9)) {
    goto ltmp_1157_D;
  } else {
    ltmp_1134_9_2E = ltmp_1133_9;
    goto ltmp_1158_D;
  }

ltmp_1161_D:
  if (((ltmp_1141_A & PLZO_INT64_C(1)) == PLZO_INT64_C(0))) {
    goto ltmp_1162_D;
  } else {
    goto ltmp_1159_D;
  }

ltmp_1163_D:
  ltmp_1140_9 = *ltmp_1129_19;
  ltmp_1141_A = *ltmp_1102_1A;
  if ((ltmp_1140_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1161_D;
  } else {
    goto ltmp_1164_D;
  }

ltmp_1160_D:
  if ((ltmp_1138_9 == ltmp_1139_9)) {
    goto ltmp_1163_D;
  } else {
    goto ltmp_1162_D;
  }

ltmp_1164_D:
  if (((((plzo_uint64_t )(ltmp_1141_A >> 1)) & PLZO_UINT64_C(1)) == PLZO_UINT64_C(0))) {
    goto ltmp_1162_D;
  } else {
    goto ltmp_1159_D;
  }

ltmp_1162_D:
  *ltmp_1128_19 = ltmp_1138_9;
  *ltmp_1129_19 = ltmp_1135_9;
  ltmp_1142_9 = *ltmp_1103_19;
  *ltmp_1130_19 = ltmp_1142_9;
  ltmp_1143_9 = *ltmp_1104_19;
  *ltmp_1131_19 = ltmp_1143_9;
  goto ltmp_1159_D;

  } while (1);
  } while (1);
  } while (1);
ltmp_1150_D:
  ltmp_1144_1031 = *ltmp_1082_1030;
  ltmp_1145_9 = *(&ltmp_1144_1031->field0);
  return ltmp_1145_9;
}


static plzo_uint64_t ltmp_13_1008(struct l_struct_2E_E *ltmp_1165_1023, plzo_uint64_t ltmp_1166_9, plzo_uint64_t ltmp_1167_9) {
  struct l_struct_2E_E_3A__3A_A4 ltmp_1168_103B;
  plzo_uint64_t ltmp_1169_9;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1170_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1171_1031;
  plzo_uint64_t ltmp_1172_9;
  plzo_uint64_t ltmp_1173_9;
  unsigned char *ltmp_1174_13;
  plzo_uint64_t ltmp_1175_9;
  plzo_uint64_t ltmp_1175_9_2E;
  plzo_uint64_t ltmp_1176_9;
  plzo_int64_t  (***ltmp_1177_102C) ();
  plzo_int64_t  (**ltmp_1178_1029) ();
  plzo_int64_t  (*ltmp_1179_102A) ();
  plzo_uint64_t ltmp_1180_9;
  unsigned char *ltmp_1181_13;
  plzo_uint64_t ltmp_1182_9;
  plzo_uint64_t ltmp_1182_9_2E;
  plzo_uint64_t ltmp_1183_9;
  plzo_uint64_t ltmp_1184_9;
  struct l_struct_2E_A7 **ltmp_1185_1037;
  plzo_uint64_t *ltmp_1186_19;
  plzo_uint64_t *ltmp_1187_19;
  plzo_uint64_t *ltmp_1188_19;
  plzo_uint64_t *ltmp_1189_19;
  plzo_uint64_t *ltmp_1190_19;
  plzo_int64_t *ltmp_1191_1A;
  plzo_uint64_t *ltmp_1192_19;
  plzo_uint64_t *ltmp_1193_19;
  plzo_uint64_t ltmp_1194_9;
  plzo_uint64_t ltmp_1194_9_2E;
  plzo_uint64_t ltmp_1195_9;
  struct l_struct_2E_A7 *ltmp_1196_1038;
  plzo_uint64_t ltmp_1197_9;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1198_1031;
  plzo_uint64_t ltmp_1199_9;
  plzo_uint64_t *ltmp_1200_19;
  plzo_uint64_t *ltmp_1201_19;
  plzo_uint64_t *ltmp_1202_19;
  plzo_uint64_t *ltmp_1203_19;
  plzo_uint64_t ltmp_1204_9;
  plzo_uint64_t ltmp_1205_9;
  plzo_uint64_t ltmp_1206_9;
  plzo_uint64_t ltmp_1207_9;
  plzo_uint64_t ltmp_1208_9;
  plzo_uint64_t ltmp_1209_9;
  plzo_uint64_t ltmp_1210_9;
  plzo_uint64_t ltmp_1210_9_2E;
  plzo_uint64_t ltmp_1211_9;
  plzo_uint64_t ltmp_1211_9_2E;
  plzo_uint64_t ltmp_1212_9;
  plzo_uint64_t ltmp_1213_9;
  plzo_uint64_t ltmp_1214_9;
  plzo_uint64_t ltmp_1214_9_2E;
  plzo_uint64_t ltmp_1215_9;
  plzo_int64_t  (**ltmp_1216_1029) ();
  plzo_int64_t  (*ltmp_1217_102A) ();
  plzo_uint64_t ltmp_1218_9;
  plzo_uint64_t ltmp_1219_9;
  plzo_uint64_t ltmp_1220_9;
  plzo_int64_t ltmp_1221_A;
  plzo_uint64_t ltmp_1222_9;
  plzo_uint64_t ltmp_1223_9;
  plzo_uint64_t ltmp_1224_9;
  plzo_uint64_t ltmp_1225_9;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1226_1031;
  plzo_uint64_t ltmp_1227_9;

  ltmp_1169_9 = ltmp_1167_9 - ltmp_1166_9;
  *(&ltmp_1165_1023->field20.field2) = (ltmp_1169_9 + PLZO_UINT64_C(1));
  ltmp_1170_1030 = &ltmp_1165_1023->field20.field3;
  ltmp_1171_1031 = *ltmp_1170_1030;
  ltmp_1172_9 = *(&ltmp_1165_1023->field20.field1);
  ltmp_1173_9 = ltmp_1172_9 << 5;
  ltmp_1174_13 = (unsigned char *)(&ltmp_1171_1031[(ltmp_1172_9*ltmp_1169_9)]);
  if ((ltmp_1173_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1228_D;
  } else {
    ltmp_1175_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1229_D;
  }

  do {
ltmp_1229_D:
  ltmp_1175_9 = ltmp_1175_9_2E;
  *(&ltmp_1174_13[ltmp_1175_9]) = 0;
  ltmp_1176_9 = ltmp_1175_9 + PLZO_UINT64_C(1);
  if ((ltmp_1176_9 == ltmp_1173_9)) {
    goto ltmp_1228_D;
  } else {
    ltmp_1175_9_2E = ltmp_1176_9;
    goto ltmp_1229_D;
  }

  } while (1);
ltmp_1228_D:
  ltmp_1177_102C = &ltmp_1165_1023->field0;
  ltmp_1178_1029 = *ltmp_1177_102C;
  ltmp_1179_102A = *(&ltmp_1178_1029[PLZO_INT64_C(18)]);
  ltmp_1180_9 = ((plzo_uint64_t  (*) (struct l_struct_2E_E *))ltmp_1179_102A)(ltmp_1165_1023);
  ltmp_1181_13 = (unsigned char *)(&ltmp_1168_103B);
  ltmp_1182_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1230_D;

  do {
ltmp_1230_D:
  ltmp_1182_9 = ltmp_1182_9_2E;
  *(&ltmp_1181_13[ltmp_1182_9]) = 0;
  ltmp_1183_9 = ltmp_1182_9 + PLZO_UINT64_C(1);
  if ((ltmp_1183_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1231_D;
  } else {
    ltmp_1182_9_2E = ltmp_1183_9;
    goto ltmp_1230_D;
  }

  } while (1);
ltmp_1231_D:
  *(&ltmp_1168_103B.field3) = ltmp_1166_9;
  *(&ltmp_1168_103B.field4) = ltmp_1167_9;
  *(&ltmp_1168_103B.field5) = ltmp_1169_9;
  if ((ltmp_1167_9 == ltmp_1166_9)) {
    goto ltmp_1232_D;
  } else {
    goto ltmp_1233_D;
  }

ltmp_1233_D:
  ltmp_1185_1037 = &ltmp_1165_1023->field19.field0;
  ltmp_1186_19 = &ltmp_1165_1023->field7.field12;
  ltmp_1187_19 = &ltmp_1165_1023->field7.field13;
  ltmp_1188_19 = &ltmp_1165_1023->field7.field6;
  ltmp_1189_19 = &ltmp_1165_1023->field7.field3;
  ltmp_1190_19 = &ltmp_1168_103B.field0;
  ltmp_1191_1A = &ltmp_1165_1023->field7.field14;
  ltmp_1192_19 = &ltmp_1168_103B.field1;
  ltmp_1193_19 = &ltmp_1168_103B.field2;
  ltmp_1194_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1234_D;

  do {
ltmp_1234_D:
  ltmp_1194_9 = ltmp_1194_9_2E;
  ltmp_1195_9 = (ltmp_1169_9 - ltmp_1194_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_1196_1038 = *ltmp_1185_1037;
  ltmp_1197_9 = *(&ltmp_1196_1038[ltmp_1195_9].field0);
  ltmp_1198_1031 = *ltmp_1170_1030;
  ltmp_1199_9 = *(&ltmp_1198_1031[(((plzo_int64_t )ltmp_1195_9) + PLZO_INT64_C(1))].field0);
  ltmp_1200_19 = &ltmp_1198_1031[ltmp_1195_9].field0;
  *ltmp_1200_19 = (ltmp_1199_9 + ltmp_1180_9);
  ltmp_1201_19 = &ltmp_1198_1031[ltmp_1195_9].field1;
  *ltmp_1201_19 = PLZO_UINT64_C(0);
  ltmp_1202_19 = &ltmp_1198_1031[ltmp_1195_9].field2;
  *ltmp_1202_19 = PLZO_UINT64_C(0);
  ltmp_1203_19 = &ltmp_1198_1031[ltmp_1195_9].field3;
  *ltmp_1203_19 = PLZO_UINT64_C(0);
  if ((ltmp_1197_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1235_D;
  } else {
    goto ltmp_1236_D;
  }

ltmp_1235_D:
  ltmp_1184_9 = ltmp_1194_9 + PLZO_UINT64_C(1);
  if ((ltmp_1184_9 == ltmp_1169_9)) {
    goto ltmp_1232_D;
  } else {
    ltmp_1194_9_2E = ltmp_1184_9;
    goto ltmp_1234_D;
  }

ltmp_1237_D:
  ltmp_1210_9 = ltmp_1210_9_2E;
  ltmp_1211_9 = ltmp_1211_9_2E;
  ltmp_1212_9 = ((((ltmp_1211_9 + ltmp_1210_9) < ltmp_1197_9)) ? ((ltmp_1197_9 - ltmp_1211_9)) : (ltmp_1210_9));
  ltmp_1213_9 = *ltmp_1189_19;
  if ((ltmp_1213_9 > ltmp_1197_9)) {
    goto ltmp_1235_D;
  } else {
    ltmp_1214_9_2E = ltmp_1213_9;
    goto ltmp_1238_D;
  }

ltmp_1236_D:
  ltmp_1204_9 = *ltmp_1186_19;
  ltmp_1205_9 = *ltmp_1187_19;
  ltmp_1206_9 = *ltmp_1188_19;
  if ((ltmp_1206_9 > ltmp_1197_9)) {
    ltmp_1210_9_2E = ltmp_1204_9;
    ltmp_1211_9_2E = ltmp_1205_9;
    goto ltmp_1237_D;
  } else {
    goto ltmp_1239_D;
  }

ltmp_1239_D:
  ltmp_1207_9 = *ltmp_1189_19;
  ltmp_1208_9 = ltmp_1207_9 + PLZO_UINT64_C(1);
  ltmp_1209_9 = (plzo_uint64_t )(ltmp_1205_9 != PLZO_UINT64_C(0));
  ltmp_1210_9_2E = ltmp_1208_9;
  ltmp_1211_9_2E = ltmp_1209_9;
  goto ltmp_1237_D;

  do {
ltmp_1238_D:
  ltmp_1214_9 = ltmp_1214_9_2E;
  ltmp_1215_9 = (((ltmp_1214_9 == ltmp_1210_9)) ? (ltmp_1212_9) : (ltmp_1214_9));
  ltmp_1216_1029 = *ltmp_1177_102C;
  ltmp_1217_102A = *(&ltmp_1216_1029[PLZO_INT64_C(17)]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint64_t , plzo_uint64_t , plzo_uint64_t ))ltmp_1217_102A)(ltmp_1165_1023, (&ltmp_1168_103B), ltmp_1215_9, ltmp_1195_9, PLZO_UINT64_C(0));
  ltmp_1218_9 = *ltmp_1190_19;
  ltmp_1219_9 = *ltmp_1200_19;
  if ((ltmp_1218_9 > ltmp_1219_9)) {
    goto ltmp_1240_D;
  } else {
    goto ltmp_1241_D;
  }

ltmp_1242_D:
  *ltmp_1200_19 = ltmp_1218_9;
  *ltmp_1201_19 = ltmp_1215_9;
  ltmp_1222_9 = *ltmp_1192_19;
  *ltmp_1202_19 = ltmp_1222_9;
  ltmp_1223_9 = *ltmp_1193_19;
  *ltmp_1203_19 = ltmp_1223_9;
  ltmp_1224_9 = ltmp_1215_9 + PLZO_UINT64_C(1);
  if ((ltmp_1224_9 > ltmp_1197_9)) {
    goto ltmp_1235_D;
  } else {
    ltmp_1214_9_2E = ltmp_1224_9;
    goto ltmp_1238_D;
  }

ltmp_1241_D:
  if ((ltmp_1218_9 == ltmp_1219_9)) {
    goto ltmp_1243_D;
  } else {
    goto ltmp_1242_D;
  }

ltmp_1244_D:
  if (((ltmp_1221_A & PLZO_INT64_C(1)) == PLZO_INT64_C(0))) {
    goto ltmp_1242_D;
  } else {
    goto ltmp_1240_D;
  }

ltmp_1243_D:
  ltmp_1220_9 = *ltmp_1201_19;
  ltmp_1221_A = *ltmp_1191_1A;
  if ((ltmp_1220_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1244_D;
  } else {
    goto ltmp_1245_D;
  }

ltmp_1245_D:
  if (((((plzo_uint64_t )(ltmp_1221_A >> 1)) & PLZO_UINT64_C(1)) == PLZO_UINT64_C(0))) {
    goto ltmp_1242_D;
  } else {
    goto ltmp_1240_D;
  }

ltmp_1240_D:
  ltmp_1225_9 = ltmp_1215_9 + PLZO_UINT64_C(1);
  if ((ltmp_1225_9 > ltmp_1197_9)) {
    goto ltmp_1235_D;
  } else {
    ltmp_1214_9_2E = ltmp_1225_9;
    goto ltmp_1238_D;
  }

  } while (1);
  } while (1);
ltmp_1232_D:
  ltmp_1226_1031 = *ltmp_1170_1030;
  ltmp_1227_9 = *(&ltmp_1226_1031->field0);
  return ltmp_1227_9;
}


static plzo_uint64_t ltmp_14_1008(struct l_struct_2E_E *ltmp_1246_1023, plzo_uint64_t ltmp_1247_9, plzo_uint64_t ltmp_1248_9) {
  struct l_struct_2E_E_3A__3A_A4 ltmp_1249_103B;
  struct l_struct_2E_E_3A__3A_A4 ltmp_1250_103B;
  plzo_uint64_t ltmp_1251_9;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1252_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1253_1031;
  plzo_uint64_t ltmp_1254_9;
  plzo_uint64_t ltmp_1255_9;
  unsigned char *ltmp_1256_13;
  plzo_uint64_t ltmp_1257_9;
  plzo_uint64_t ltmp_1257_9_2E;
  plzo_uint64_t ltmp_1258_9;
  plzo_int64_t  (***ltmp_1259_102C) ();
  plzo_int64_t  (**ltmp_1260_1029) ();
  plzo_int64_t  (*ltmp_1261_102A) ();
  plzo_uint64_t ltmp_1262_9;
  unsigned char *ltmp_1263_13;
  plzo_uint64_t ltmp_1264_9;
  plzo_uint64_t ltmp_1264_9_2E;
  plzo_uint64_t ltmp_1265_9;
  unsigned char *ltmp_1266_13;
  plzo_uint64_t ltmp_1267_9;
  plzo_uint64_t ltmp_1267_9_2E;
  plzo_uint64_t ltmp_1268_9;
  struct l_struct_2E_A7 **ltmp_1269_1037;
  plzo_uint64_t *ltmp_1270_19;
  plzo_uint64_t *ltmp_1271_19;
  plzo_uint64_t ltmp_1272_9;
  plzo_uint64_t ltmp_1272_9_2E;
  plzo_uint64_t ltmp_1273_9;
  struct l_struct_2E_A7 *ltmp_1274_1038;
  plzo_uint64_t ltmp_1275_9;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1276_1031;
  plzo_uint64_t ltmp_1277_9;
  plzo_uint64_t ltmp_1278_9;
  plzo_int64_t  (**ltmp_1279_1029) ();
  plzo_int64_t  (*ltmp_1280_102A) ();
  plzo_uint64_t ltmp_1281_9;
  plzo_uint64_t *ltmp_1282_19;
  plzo_uint64_t ltmp_1283_9;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1284_1031;
  plzo_uint64_t ltmp_1285_9;

  ltmp_1251_9 = ltmp_1248_9 - ltmp_1247_9;
  *(&ltmp_1246_1023->field20.field2) = (ltmp_1251_9 + PLZO_UINT64_C(1));
  ltmp_1252_1030 = &ltmp_1246_1023->field20.field3;
  ltmp_1253_1031 = *ltmp_1252_1030;
  ltmp_1254_9 = *(&ltmp_1246_1023->field20.field1);
  ltmp_1255_9 = ltmp_1254_9 << 5;
  ltmp_1256_13 = (unsigned char *)(&ltmp_1253_1031[(ltmp_1254_9*ltmp_1251_9)]);
  if ((ltmp_1255_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1286_D;
  } else {
    ltmp_1257_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1287_D;
  }

  do {
ltmp_1287_D:
  ltmp_1257_9 = ltmp_1257_9_2E;
  *(&ltmp_1256_13[ltmp_1257_9]) = 0;
  ltmp_1258_9 = ltmp_1257_9 + PLZO_UINT64_C(1);
  if ((ltmp_1258_9 == ltmp_1255_9)) {
    goto ltmp_1286_D;
  } else {
    ltmp_1257_9_2E = ltmp_1258_9;
    goto ltmp_1287_D;
  }

  } while (1);
ltmp_1286_D:
  ltmp_1259_102C = &ltmp_1246_1023->field0;
  ltmp_1260_1029 = *ltmp_1259_102C;
  ltmp_1261_102A = *(&ltmp_1260_1029[PLZO_INT64_C(18)]);
  ltmp_1262_9 = ((plzo_uint64_t  (*) (struct l_struct_2E_E *))ltmp_1261_102A)(ltmp_1246_1023);
  ltmp_1263_13 = (unsigned char *)(&ltmp_1249_103B);
  ltmp_1264_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1288_D;

  do {
ltmp_1288_D:
  ltmp_1264_9 = ltmp_1264_9_2E;
  *(&ltmp_1263_13[ltmp_1264_9]) = 0;
  ltmp_1265_9 = ltmp_1264_9 + PLZO_UINT64_C(1);
  if ((ltmp_1265_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1289_D;
  } else {
    ltmp_1264_9_2E = ltmp_1265_9;
    goto ltmp_1288_D;
  }

  } while (1);
ltmp_1289_D:
  *(&ltmp_1249_103B.field3) = ltmp_1247_9;
  *(&ltmp_1249_103B.field4) = ltmp_1248_9;
  *(&ltmp_1249_103B.field5) = ltmp_1251_9;
  ltmp_1266_13 = (unsigned char *)(&ltmp_1250_103B);
  ltmp_1267_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1290_D;

  do {
ltmp_1290_D:
  ltmp_1267_9 = ltmp_1267_9_2E;
  *(&ltmp_1266_13[ltmp_1267_9]) = 0;
  ltmp_1268_9 = ltmp_1267_9 + PLZO_UINT64_C(1);
  if ((ltmp_1268_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1291_D;
  } else {
    ltmp_1267_9_2E = ltmp_1268_9;
    goto ltmp_1290_D;
  }

  } while (1);
ltmp_1291_D:
  *(&ltmp_1250_103B.field3) = ltmp_1247_9;
  *(&ltmp_1250_103B.field4) = ltmp_1248_9;
  *(&ltmp_1250_103B.field5) = ltmp_1251_9;
  if ((ltmp_1248_9 == ltmp_1247_9)) {
    goto ltmp_1292_D;
  } else {
    goto ltmp_1293_D;
  }

ltmp_1293_D:
  ltmp_1269_1037 = &ltmp_1246_1023->field19.field0;
  ltmp_1270_19 = &ltmp_1249_103B.field0;
  ltmp_1271_19 = &ltmp_1250_103B.field0;
  ltmp_1272_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1294_D;

  do {
ltmp_1294_D:
  ltmp_1272_9 = ltmp_1272_9_2E;
  ltmp_1273_9 = (ltmp_1251_9 - ltmp_1272_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_1274_1038 = *ltmp_1269_1037;
  ltmp_1275_9 = *(&ltmp_1274_1038[ltmp_1273_9].field0);
  ltmp_1276_1031 = *ltmp_1252_1030;
  ltmp_1277_9 = *(&ltmp_1276_1031[(((plzo_int64_t )ltmp_1273_9) + PLZO_INT64_C(1))].field0);
  ltmp_1278_9 = ltmp_1277_9 + ltmp_1262_9;
  *ltmp_1270_19 = ltmp_1278_9;
  ltmp_1279_1029 = *ltmp_1259_102C;
  ltmp_1280_102A = *(&ltmp_1279_1029[PLZO_INT64_C(17)]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint64_t , plzo_uint64_t , plzo_uint64_t ))ltmp_1280_102A)(ltmp_1246_1023, (&ltmp_1250_103B), ltmp_1275_9, ltmp_1273_9, PLZO_UINT64_C(0));
  ltmp_1281_9 = *ltmp_1271_19;
  ltmp_1282_19 = &ltmp_1276_1031[ltmp_1273_9].field0;
  if ((ltmp_1278_9 > ltmp_1281_9)) {
    goto ltmp_1295_D;
  } else {
    goto ltmp_1296_D;
  }

ltmp_1297_D:
  ltmp_1283_9 = ltmp_1272_9 + PLZO_UINT64_C(1);
  ltmp_1272_9_2E = ltmp_1283_9;
  goto ltmp_1294_D;

ltmp_1296_D:
  *ltmp_1282_19 = ltmp_1278_9;
  *(&ltmp_1276_1031[ltmp_1273_9].field1) = PLZO_UINT64_C(0);
  *(&ltmp_1276_1031[ltmp_1273_9].field2) = PLZO_UINT64_C(0);
  *(&ltmp_1276_1031[ltmp_1273_9].field3) = PLZO_UINT64_C(0);
  if ((ltmp_1273_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1292_D;
  } else {
    goto ltmp_1297_D;
  }

ltmp_1295_D:
  *ltmp_1282_19 = ltmp_1281_9;
  *(&ltmp_1276_1031[ltmp_1273_9].field1) = ltmp_1275_9;
  *(&ltmp_1276_1031[ltmp_1273_9].field2) = PLZO_UINT64_C(0);
  *(&ltmp_1276_1031[ltmp_1273_9].field3) = PLZO_UINT64_C(0);
  if ((ltmp_1273_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1292_D;
  } else {
    goto ltmp_1297_D;
  }

  } while (1);
ltmp_1292_D:
  ltmp_1284_1031 = *ltmp_1252_1030;
  ltmp_1285_9 = *(&ltmp_1284_1031->field0);
  return ltmp_1285_9;
}


static void ltmp_15_1000(struct l_struct_2E_E *ltmp_1298_1023, plzo_uint64_t ltmp_1299_9, plzo_uint64_t ltmp_1300_9) {
  struct l_struct_2E_A7 *ltmp_1301_1038;
  plzo_uint64_t *ltmp_1302_19;
  plzo_uint64_t ltmp_1303_9;
  struct l_struct_2E_A7 *ltmp_1304_1038;
  plzo_uint64_t *ltmp_1305_19;
  plzo_uint64_t ltmp_1306_9;
  plzo_uint64_t ltmp_1307_9;
  plzo_uint64_t *ltmp_1308_19;
  plzo_uint64_t ltmp_1309_9;
  plzo_uint64_t ltmp_1310_9;
  plzo_uint64_t ltmp_1311_9;
  plzo_uint64_t ltmp_1312_9;
  plzo_uint64_t ltmp_1313_9;
  plzo_uint64_t ltmp_1313_9_2E;
  plzo_uint64_t ltmp_1314_9;
  plzo_uint64_t ltmp_1314_9_2E;
  struct l_struct_2E_A7 *ltmp_1315_1038;
  struct l_struct_2E_A7 *ltmp_1315_1038_2E;
  plzo_uint64_t ltmp_1316_9;
  plzo_uint64_t ltmp_1316_9_2E;
  plzo_uint64_t ltmp_1317_9;
  plzo_uint64_t ltmp_1317_9_2E;
  struct l_struct_2E_A7 *ltmp_1318_1038;
  struct l_struct_2E_A7 *ltmp_1318_1038_2E;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1319_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1320_1031;
  plzo_uint64_t *ltmp_1321_19;
  plzo_uint64_t ltmp_1322_9;
  plzo_uint64_t ltmp_1323_9;
  plzo_uint64_t ltmp_1324_9;
  plzo_uint64_t ltmp_1325_9;
  plzo_uint64_t ltmp_1326_9;
  plzo_uint64_t ltmp_1327_9;
  plzo_uint64_t ltmp_1328_9;
  struct l_struct_2E_A7 *ltmp_1329_1038;
  plzo_uint64_t ltmp_1330_9;
  plzo_uint64_t ltmp_1331_9;
  struct l_struct_2E_A7 *ltmp_1332_1038;
  plzo_uint64_t *ltmp_1333_19;
  plzo_uint64_t ltmp_1334_9;
  plzo_int64_t ltmp_1335_A;
  plzo_uint64_t ltmp_1336_9;
  plzo_uint64_t ltmp_1337_9;
  plzo_uint64_t ltmp_1338_9;
  plzo_uint64_t ltmp_1339_9;
  plzo_uint64_t ltmp_1340_9;
  plzo_uint64_t ltmp_1341_9;
  plzo_uint64_t ltmp_1342_9;
  plzo_uint64_t ltmp_1343_9;
  plzo_uint64_t ltmp_1344_9;
  plzo_uint64_t ltmp_1345_9;
  plzo_uint64_t ltmp_1346_9;
  plzo_uint64_t ltmp_1347_9;
  plzo_uint64_t ltmp_1348_9;
  plzo_uint64_t ltmp_1349_9;
  plzo_uint64_t ltmp_1350_9;
  plzo_uint64_t ltmp_1350_9_2E;
  plzo_uint64_t ltmp_1351_9;
  plzo_uint64_t ltmp_1352_9;
  plzo_uint64_t ltmp_1353_9;
  plzo_uint64_t ltmp_1354_9;
  plzo_uint64_t ltmp_1354_9_2E;
  unsigned char *ltmp_1355_13;
  plzo_int64_t ltmp_1356_A;
  plzo_uint64_t ltmp_1357_9;
  plzo_uint64_t ltmp_1357_9_2E;
  unsigned char ltmp_1358_3;
  unsigned char ltmp_1359_3;
  plzo_int64_t ltmp_1360_A;
  plzo_uint64_t ltmp_1361_9;
  plzo_int64_t ltmp_1362_A;
  plzo_int64_t ltmp_1362_A_2E;
  plzo_uint64_t ltmp_1363_9;
  plzo_uint64_t ltmp_1363_9_2E;
  plzo_uint64_t *ltmp_1364_19;
  plzo_uint64_t ltmp_1365_9;
  plzo_uint64_t ltmp_1366_9;
  plzo_int64_t  (***ltmp_1367_102C) ();
  plzo_int64_t  (**ltmp_1368_1029) ();
  plzo_int64_t  (*ltmp_1369_102A) ();
  unsigned char *ltmp_1370_13;
  plzo_int64_t  (**ltmp_1371_1029) ();
  plzo_int64_t  (*ltmp_1372_102A) ();
  plzo_uint64_t ltmp_1373_9;
  plzo_uint64_t ltmp_1374_9;
  plzo_uint64_t ltmp_1375_9;
  struct l_struct_2E_A7 *ltmp_1376_1038;
  plzo_uint64_t ltmp_1377_9;
  plzo_uint64_t ltmp_1378_9;
  plzo_uint64_t ltmp_1379_9;
  plzo_uint64_t ltmp_1380_9;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1381_1031;
  plzo_uint64_t ltmp_1382_9;
  plzo_uint64_t ltmp_1383_9;
  plzo_uint64_t ltmp_1384_9;
  struct l_struct_2E_A7 *ltmp_1385_1038;

  ltmp_1301_1038 = *(&ltmp_1298_1023->field19.field0);
  ltmp_1302_19 = &ltmp_1298_1023->field14;
  ltmp_1303_9 = *ltmp_1302_19;
  ltmp_1304_1038 = &ltmp_1301_1038[ltmp_1303_9];
  ltmp_1305_19 = &ltmp_1298_1023->field13;
  ltmp_1306_9 = *ltmp_1305_19;
  ltmp_1307_9 = ltmp_1303_9 + ltmp_1306_9;
  *ltmp_1305_19 = ltmp_1307_9;
  ltmp_1308_19 = &ltmp_1298_1023->field21;
  ltmp_1309_9 = *ltmp_1308_19;
  if ((ltmp_1307_9 < ltmp_1309_9)) {
    goto ltmp_1386_D;
  } else {
    goto ltmp_1387_D;
  }

ltmp_1386_D:
  ltmp_1310_9 = ltmp_1303_9 + ltmp_1299_9;
  if ((ltmp_1310_9 < ltmp_1300_9)) {
    ltmp_1316_9_2E = ltmp_1307_9;
    ltmp_1317_9_2E = ltmp_1310_9;
    ltmp_1318_1038_2E = ltmp_1304_1038;
    goto ltmp_1388_D;
  } else {
    goto ltmp_1389_D;
  }

ltmp_1387_D:
  ltmp_1311_9 = ltmp_1309_9 + PLZO_UINT64_C(18446744073709551615);
  ltmp_1312_9 = ltmp_1303_9 + ltmp_1299_9;
  ltmp_1313_9_2E = ltmp_1311_9;
  ltmp_1314_9_2E = ltmp_1312_9;
  ltmp_1315_1038_2E = ltmp_1304_1038;
  goto ltmp_1390_D;

ltmp_1390_D:
  ltmp_1313_9 = ltmp_1313_9_2E;
  ltmp_1314_9 = ltmp_1314_9_2E;
  ltmp_1315_1038 = ltmp_1315_1038_2E;
  if ((ltmp_1314_9 < ltmp_1300_9)) {
    ltmp_1316_9_2E = ltmp_1313_9;
    ltmp_1317_9_2E = ltmp_1314_9;
    ltmp_1318_1038_2E = ltmp_1315_1038;
    goto ltmp_1388_D;
  } else {
    goto ltmp_1389_D;
  }

ltmp_1388_D:
  ltmp_1316_9 = ltmp_1316_9_2E;
  ltmp_1317_9 = ltmp_1317_9_2E;
  ltmp_1318_1038 = ltmp_1318_1038_2E;
  ltmp_1319_1030 = &ltmp_1298_1023->field20.field3;
  ltmp_1320_1031 = *ltmp_1319_1030;
  ltmp_1321_19 = &ltmp_1298_1023->field20.field1;
  ltmp_1322_9 = *ltmp_1321_19;
  ltmp_1323_9 = (ltmp_1322_9*(ltmp_1317_9 - ltmp_1299_9)) + ltmp_1316_9;
  ltmp_1324_9 = *(&ltmp_1320_1031[ltmp_1323_9].field1);
  if ((ltmp_1324_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1391_D;
  } else {
    goto ltmp_1392_D;
  }

ltmp_1391_D:
  ltmp_1325_9 = *ltmp_1305_19;
  ltmp_1326_9 = ltmp_1325_9 + PLZO_UINT64_C(1);
  *ltmp_1305_19 = ltmp_1326_9;
  ltmp_1327_9 = *ltmp_1308_19;
  if ((ltmp_1326_9 < ltmp_1327_9)) {
    goto ltmp_1393_D;
  } else {
    goto ltmp_1394_D;
  }

ltmp_1393_D:
  ltmp_1328_9 = ltmp_1317_9 + PLZO_UINT64_C(1);
  ltmp_1329_1038 = &ltmp_1318_1038[PLZO_INT64_C(1)];
  ltmp_1313_9_2E = ltmp_1326_9;
  ltmp_1314_9_2E = ltmp_1328_9;
  ltmp_1315_1038_2E = ltmp_1329_1038;
  goto ltmp_1390_D;

ltmp_1394_D:
  ltmp_1330_9 = ltmp_1327_9 + PLZO_UINT64_C(18446744073709551615);
  ltmp_1331_9 = ltmp_1317_9 + PLZO_UINT64_C(1);
  ltmp_1332_1038 = &ltmp_1318_1038[PLZO_INT64_C(1)];
  ltmp_1313_9_2E = ltmp_1330_9;
  ltmp_1314_9_2E = ltmp_1331_9;
  ltmp_1315_1038_2E = ltmp_1332_1038;
  goto ltmp_1390_D;

ltmp_1392_D:
  ltmp_1333_19 = &ltmp_1320_1031[ltmp_1323_9].field2;
  ltmp_1334_9 = *ltmp_1333_19;
  if ((ltmp_1334_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1395_D;
  } else {
    ltmp_1363_9_2E = ltmp_1334_9;
    goto ltmp_1396_D;
  }

ltmp_1395_D:
  ltmp_1335_A = *(&ltmp_1298_1023->field6.field1);
  if (((((plzo_uint64_t )ltmp_1335_A) & PLZO_UINT64_C(1)) == PLZO_UINT64_C(0))) {
    goto ltmp_1397_D;
  } else {
    goto ltmp_1398_D;
  }

ltmp_1397_D:
  ltmp_1336_9 = *(&ltmp_1318_1038->field0);
  ltmp_1337_9 = ltmp_1336_9 - ltmp_1324_9;
  ltmp_1338_9 = *(&ltmp_1318_1038->field1[((((ltmp_1337_9 < PLZO_UINT64_C(8))) ? (ltmp_1337_9) : (PLZO_UINT64_C(7))))]);
  ltmp_1363_9_2E = ltmp_1338_9;
  goto ltmp_1396_D;

ltmp_1398_D:
  ltmp_1339_9 = *(&ltmp_1298_1023->field5);
  if ((ltmp_1339_9 < ltmp_1324_9)) {
    goto ltmp_1399_D;
  } else {
    goto ltmp_1400_D;
  }

ltmp_1400_D:
  ltmp_1340_9 = *(&ltmp_1298_1023->field16);
  ltmp_1341_9 = *(&ltmp_1298_1023->field4[ltmp_1324_9]);
  if ((ltmp_1340_9 < ltmp_1341_9)) {
    goto ltmp_1399_D;
  } else {
    goto ltmp_1401_D;
  }

ltmp_1401_D:
  ltmp_1342_9 = *(&ltmp_1318_1038->field0);
  ltmp_1343_9 = ltmp_1342_9 - ltmp_1324_9;
  ltmp_1344_9 = *(&ltmp_1318_1038->field1[((((ltmp_1343_9 < PLZO_UINT64_C(8))) ? (ltmp_1343_9) : (PLZO_UINT64_C(7))))]);
  ltmp_1363_9_2E = ltmp_1344_9;
  goto ltmp_1396_D;

ltmp_1399_D:
  ltmp_1345_9 = *(&ltmp_1298_1023->field16);
  ltmp_1346_9 = *(&ltmp_1318_1038->field0);
  ltmp_1347_9 = ltmp_1346_9 - ltmp_1324_9;
  ltmp_1348_9 = (((ltmp_1347_9 < PLZO_UINT64_C(8))) ? (ltmp_1347_9) : (PLZO_UINT64_C(7)));
  ltmp_1349_9 = *(&ltmp_1318_1038->field1[ltmp_1348_9]);
  if (((ltmp_1348_9 != PLZO_UINT64_C(0)) & (ltmp_1349_9 < ltmp_1345_9))) {
    ltmp_1350_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1402_D;
  } else {
    ltmp_1354_9_2E = ltmp_1349_9;
    goto ltmp_1403_D;
  }

  do {
ltmp_1402_D:
  ltmp_1350_9 = ltmp_1350_9_2E;
  ltmp_1351_9 = ltmp_1348_9 - ltmp_1350_9;
  ltmp_1352_9 = *(&ltmp_1318_1038->field1[(ltmp_1351_9 + PLZO_UINT64_C(18446744073709551615))]);
  if ((ltmp_1352_9 == ltmp_1345_9)) {
    ltmp_1354_9_2E = ltmp_1345_9;
    goto ltmp_1403_D;
  } else {
    goto ltmp_1404_D;
  }

ltmp_1404_D:
  ltmp_1353_9 = ltmp_1350_9 + PLZO_UINT64_C(1);
  if ((ltmp_1351_9 == PLZO_UINT64_C(1))) {
    ltmp_1354_9_2E = ltmp_1349_9;
    goto ltmp_1403_D;
  } else {
    ltmp_1350_9_2E = ltmp_1353_9;
    goto ltmp_1402_D;
  }

  } while (1);
ltmp_1403_D:
  ltmp_1354_9 = ltmp_1354_9_2E;
  if ((((ltmp_1345_9 == ltmp_1354_9) | (ltmp_1324_9 < PLZO_UINT64_C(2))) | (ltmp_1345_9 == PLZO_UINT64_C(18446744073709551615)))) {
    ltmp_1363_9_2E = ltmp_1354_9;
    goto ltmp_1396_D;
  } else {
    goto ltmp_1405_D;
  }

ltmp_1405_D:
  ltmp_1355_13 = *(&ltmp_1298_1023->field8);
  ltmp_1356_A = ((plzo_int64_t )(-(ltmp_1345_9))) + ((plzo_int64_t )ltmp_1317_9);
  if ((ltmp_1324_9 == PLZO_UINT64_C(0))) {
    ltmp_1362_A_2E = PLZO_INT64_C(0);
    goto ltmp_1406_D;
  } else {
    ltmp_1357_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1407_D;
  }

  do {
ltmp_1407_D:
  ltmp_1357_9 = ltmp_1357_9_2E;
  ltmp_1358_3 = *(&ltmp_1355_13[(ltmp_1317_9 + ltmp_1357_9)]);
  ltmp_1359_3 = *(&ltmp_1355_13[(ltmp_1356_A + ((plzo_int64_t )ltmp_1357_9))]);
  ltmp_1360_A = ((plzo_int64_t )ltmp_1358_3) - ((plzo_int64_t )ltmp_1359_3);
  if ((ltmp_1358_3 == ltmp_1359_3)) {
    goto ltmp_1408_D;
  } else {
    ltmp_1362_A_2E = ltmp_1360_A;
    goto ltmp_1406_D;
  }

ltmp_1408_D:
  ltmp_1361_9 = ltmp_1357_9 + PLZO_UINT64_C(1);
  if (((ltmp_1324_9 - ltmp_1357_9) == PLZO_UINT64_C(1))) {
    ltmp_1362_A_2E = PLZO_INT64_C(0);
    goto ltmp_1406_D;
  } else {
    ltmp_1357_9_2E = ltmp_1361_9;
    goto ltmp_1407_D;
  }

  } while (1);
ltmp_1406_D:
  ltmp_1362_A = ltmp_1362_A_2E;
  if ((ltmp_1362_A == PLZO_INT64_C(0))) {
    goto ltmp_1409_D;
  } else {
    ltmp_1363_9_2E = ltmp_1354_9;
    goto ltmp_1396_D;
  }

ltmp_1409_D:
  ltmp_1363_9_2E = ltmp_1345_9;
  goto ltmp_1396_D;

ltmp_1396_D:
  ltmp_1363_9 = ltmp_1363_9_2E;
  ltmp_1364_19 = &ltmp_1298_1023->field12;
  ltmp_1365_9 = *ltmp_1305_19;
  ltmp_1366_9 = ltmp_1317_9 - ltmp_1365_9;
  *ltmp_1364_19 = ltmp_1366_9;
  ltmp_1367_102C = &ltmp_1298_1023->field0;
  ltmp_1368_1029 = *ltmp_1367_102C;
  ltmp_1369_102A = *(&ltmp_1368_1029[PLZO_INT64_C(3)]);
  ltmp_1370_13 = *(&ltmp_1298_1023->field8);
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint64_t , plzo_uint64_t ))ltmp_1369_102A)(ltmp_1298_1023, (&ltmp_1370_13[ltmp_1366_9]), ltmp_1365_9, ltmp_1324_9);
  *ltmp_1305_19 = PLZO_UINT64_C(0);
  *ltmp_1364_19 = ltmp_1317_9;
  ltmp_1371_1029 = *ltmp_1367_102C;
  ltmp_1372_102A = *(&ltmp_1371_1029[PLZO_INT64_C(4)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t ))ltmp_1372_102A)(ltmp_1298_1023, ltmp_1324_9, ltmp_1363_9);
  ltmp_1373_9 = *(&ltmp_1320_1031[ltmp_1323_9].field3);
  if ((ltmp_1373_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1410_D;
  } else {
    goto ltmp_1411_D;
  }

ltmp_1411_D:
  ltmp_1374_9 = ltmp_1373_9 + (ltmp_1299_9 - ltmp_1317_9);
  ltmp_1375_9 = ltmp_1374_9 + ltmp_1317_9;
  ltmp_1376_1038 = &ltmp_1318_1038[ltmp_1374_9];
  ltmp_1377_9 = *ltmp_1305_19;
  ltmp_1378_9 = (ltmp_1377_9 - ltmp_1324_9) + ltmp_1374_9;
  *ltmp_1305_19 = ltmp_1378_9;
  ltmp_1379_9 = *ltmp_1308_19;
  ltmp_1380_9 = (((ltmp_1378_9 < ltmp_1379_9)) ? (ltmp_1378_9) : ((ltmp_1379_9 + PLZO_UINT64_C(18446744073709551615))));
  ltmp_1381_1031 = *ltmp_1319_1030;
  ltmp_1382_9 = *ltmp_1321_19;
  ltmp_1383_9 = *ltmp_1333_19;
  *(&ltmp_1381_1031[((ltmp_1382_9*(ltmp_1375_9 - ltmp_1299_9)) + ltmp_1380_9)].field2) = ltmp_1383_9;
  ltmp_1313_9_2E = ltmp_1380_9;
  ltmp_1314_9_2E = ltmp_1375_9;
  ltmp_1315_1038_2E = ltmp_1376_1038;
  goto ltmp_1390_D;

ltmp_1410_D:
  ltmp_1384_9 = ltmp_1324_9 + ltmp_1317_9;
  ltmp_1385_1038 = &ltmp_1318_1038[ltmp_1324_9];
  ltmp_1313_9_2E = PLZO_UINT64_C(0);
  ltmp_1314_9_2E = ltmp_1384_9;
  ltmp_1315_1038_2E = ltmp_1385_1038;
  goto ltmp_1390_D;

ltmp_1389_D:
  *ltmp_1302_19 = PLZO_UINT64_C(0);
  return;
}


static void ltmp_16_1009(struct l_struct_2E_E *ltmp_1412_1023, plzo_uint64_t ltmp_1413_9, plzo_uint64_t ltmp_1414_9, plzo_int64_t ltmp_1415_A) {
  struct l_struct_2E_A7 *ltmp_1416_1038;
  plzo_uint64_t *ltmp_1417_19;
  plzo_uint64_t ltmp_1418_9;
  struct l_struct_2E_A7 *ltmp_1419_1038;
  plzo_uint64_t *ltmp_1420_19;
  plzo_uint64_t ltmp_1421_9;
  plzo_uint64_t ltmp_1422_9;
  plzo_uint64_t *ltmp_1423_19;
  plzo_uint64_t *ltmp_1424_19;
  plzo_uint64_t *ltmp_1425_19;
  plzo_int64_t  (***ltmp_1426_102C) ();
  unsigned char **ltmp_1427_1025;
  plzo_uint64_t ltmp_1428_9;
  plzo_uint64_t ltmp_1428_9_2E;
  struct l_struct_2E_A7 *ltmp_1429_1038;
  struct l_struct_2E_A7 *ltmp_1429_1038_2E;
  plzo_uint64_t ltmp_1430_9;
  plzo_uint64_t ltmp_1431_9;
  plzo_uint64_t ltmp_1432_9;
  plzo_uint64_t ltmp_1433_9;
  plzo_uint64_t ltmp_1434_9;
  plzo_uint64_t ltmp_1435_9;
  plzo_uint64_t ltmp_1435_9_2E;
  plzo_uint64_t ltmp_1436_9;
  plzo_uint64_t ltmp_1437_9;
  plzo_uint64_t ltmp_1437_9_2E;
  plzo_uint64_t ltmp_1438_9;
  plzo_uint64_t ltmp_1439_9;
  plzo_uint64_t ltmp_1440_9;
  struct l_struct_2E_A7 *ltmp_1441_1038;
  plzo_uint64_t ltmp_1442_9;
  plzo_uint64_t ltmp_1443_9;
  plzo_int64_t  (**ltmp_1444_1029) ();
  plzo_int64_t  (*ltmp_1445_102A) ();
  unsigned char *ltmp_1446_13;
  plzo_int64_t  (**ltmp_1447_1029) ();
  plzo_int64_t  (*ltmp_1448_102A) ();
  plzo_uint64_t ltmp_1449_9;
  struct l_struct_2E_A7 *ltmp_1450_1038;

  ltmp_1416_1038 = *(&ltmp_1412_1023->field19.field0);
  ltmp_1417_19 = &ltmp_1412_1023->field14;
  ltmp_1418_9 = *ltmp_1417_19;
  ltmp_1419_1038 = &ltmp_1416_1038[ltmp_1418_9];
  ltmp_1420_19 = &ltmp_1412_1023->field13;
  ltmp_1421_9 = *ltmp_1420_19;
  *ltmp_1420_19 = (ltmp_1418_9 + ltmp_1421_9);
  ltmp_1422_9 = ltmp_1418_9 + ltmp_1413_9;
  if ((ltmp_1422_9 < ltmp_1414_9)) {
    goto ltmp_1451_D;
  } else {
    goto ltmp_1452_D;
  }

ltmp_1451_D:
  ltmp_1423_19 = &ltmp_1412_1023->field7.field3;
  ltmp_1424_19 = &ltmp_1412_1023->field5;
  ltmp_1425_19 = &ltmp_1412_1023->field12;
  ltmp_1426_102C = &ltmp_1412_1023->field0;
  ltmp_1427_1025 = &ltmp_1412_1023->field8;
  ltmp_1428_9_2E = ltmp_1422_9;
  ltmp_1429_1038_2E = ltmp_1419_1038;
  goto ltmp_1453_D;

  do {
ltmp_1453_D:
  ltmp_1428_9 = ltmp_1428_9_2E;
  ltmp_1429_1038 = ltmp_1429_1038_2E;
  ltmp_1430_9 = *(&ltmp_1429_1038->field0);
  ltmp_1431_9 = *ltmp_1423_19;
  if ((ltmp_1431_9 > ltmp_1430_9)) {
    goto ltmp_1454_D;
  } else {
    goto ltmp_1455_D;
  }

ltmp_1456_D:
  ltmp_1439_9 = *ltmp_1420_19;
  *ltmp_1420_19 = (ltmp_1439_9 + PLZO_UINT64_C(1));
  ltmp_1440_9 = ltmp_1428_9 + PLZO_UINT64_C(1);
  ltmp_1441_1038 = &ltmp_1429_1038[PLZO_INT64_C(1)];
  if ((ltmp_1440_9 < ltmp_1414_9)) {
    ltmp_1428_9_2E = ltmp_1440_9;
    ltmp_1429_1038_2E = ltmp_1441_1038;
    goto ltmp_1453_D;
  } else {
    goto ltmp_1452_D;
  }

ltmp_1457_D:
  ltmp_1435_9 = ltmp_1435_9_2E;
  ltmp_1436_9 = *(&ltmp_1412_1023->field4[ltmp_1430_9]);
  if ((ltmp_1436_9 > ltmp_1435_9)) {
    ltmp_1437_9_2E = ltmp_1435_9;
    goto ltmp_1458_D;
  } else {
    goto ltmp_1456_D;
  }

ltmp_1455_D:
  ltmp_1432_9 = *(&ltmp_1429_1038->field1[PLZO_INT64_C(0)]);
  ltmp_1433_9 = *ltmp_1424_19;
  if ((ltmp_1433_9 < ltmp_1430_9)) {
    ltmp_1437_9_2E = ltmp_1432_9;
    goto ltmp_1458_D;
  } else {
    ltmp_1435_9_2E = ltmp_1432_9;
    goto ltmp_1457_D;
  }

ltmp_1454_D:
  ltmp_1434_9 = *ltmp_1424_19;
  if ((ltmp_1434_9 < ltmp_1430_9)) {
    ltmp_1437_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1458_D;
  } else {
    ltmp_1435_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1457_D;
  }

ltmp_1459_D:
  ltmp_1438_9 = *(&ltmp_1429_1038[PLZO_INT64_C(1)].field0);
  if ((ltmp_1438_9 > ltmp_1430_9)) {
    goto ltmp_1456_D;
  } else {
    goto ltmp_1460_D;
  }

ltmp_1458_D:
  ltmp_1437_9 = ltmp_1437_9_2E;
  if ((ltmp_1415_A == PLZO_INT64_C(0))) {
    goto ltmp_1460_D;
  } else {
    goto ltmp_1459_D;
  }

ltmp_1460_D:
  ltmp_1442_9 = *ltmp_1420_19;
  ltmp_1443_9 = ltmp_1428_9 - ltmp_1442_9;
  *ltmp_1425_19 = ltmp_1443_9;
  ltmp_1444_1029 = *ltmp_1426_102C;
  ltmp_1445_102A = *(&ltmp_1444_1029[PLZO_INT64_C(3)]);
  ltmp_1446_13 = *ltmp_1427_1025;
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint64_t , plzo_uint64_t ))ltmp_1445_102A)(ltmp_1412_1023, (&ltmp_1446_13[ltmp_1443_9]), ltmp_1442_9, ltmp_1430_9);
  *ltmp_1420_19 = PLZO_UINT64_C(0);
  *ltmp_1425_19 = ltmp_1428_9;
  ltmp_1447_1029 = *ltmp_1426_102C;
  ltmp_1448_102A = *(&ltmp_1447_1029[PLZO_INT64_C(4)]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint64_t , plzo_uint64_t ))ltmp_1448_102A)(ltmp_1412_1023, ltmp_1430_9, ltmp_1437_9);
  ltmp_1449_9 = ltmp_1430_9 + ltmp_1428_9;
  ltmp_1450_1038 = &ltmp_1429_1038[ltmp_1430_9];
  if ((ltmp_1449_9 < ltmp_1414_9)) {
    ltmp_1428_9_2E = ltmp_1449_9;
    ltmp_1429_1038_2E = ltmp_1450_1038;
    goto ltmp_1453_D;
  } else {
    goto ltmp_1452_D;
  }

  } while (1);
ltmp_1452_D:
  *ltmp_1417_19 = PLZO_UINT64_C(0);
  return;
}


static plzo_uint64_t ltmp_17_1008(struct l_struct_2E_E *ltmp_1461_1023, plzo_uint64_t ltmp_1462_9, plzo_uint64_t ltmp_1463_9) {
  struct l_struct_2E_A7 *ltmp_1464_1038;
  plzo_uint64_t ltmp_1465_9;
  plzo_uint64_t ltmp_1465_9_2E;
  plzo_uint64_t ltmp_1466_9;
  plzo_uint64_t ltmp_1466_9_2E;
  plzo_uint64_t ltmp_1467_9;
  plzo_uint64_t ltmp_1467_9_2E;
  struct l_struct_2E_A7 *ltmp_1468_1038;
  struct l_struct_2E_A7 *ltmp_1468_1038_2E;
  plzo_uint64_t ltmp_1469_9;
  plzo_int64_t  (***ltmp_1470_102C) ();
  plzo_uint64_t ltmp_1471_9;
  plzo_uint64_t ltmp_1471_9_2E;
  plzo_uint64_t ltmp_1472_9;
  plzo_uint64_t ltmp_1472_9_2E;
  plzo_uint64_t ltmp_1473_9;
  plzo_uint64_t ltmp_1473_9_2E;
  plzo_uint64_t ltmp_1474_9;
  plzo_uint64_t ltmp_1474_9_2E;
  struct l_struct_2E_A7 *ltmp_1475_1038;
  struct l_struct_2E_A7 *ltmp_1475_1038_2E;
  plzo_uint64_t ltmp_1476_9;
  plzo_uint64_t ltmp_1477_9;
  plzo_uint64_t ltmp_1478_9;
  plzo_int64_t  (**ltmp_1479_1029) ();
  plzo_int64_t  (*ltmp_1480_102A) ();
  plzo_int64_t ltmp_1481_A;
  plzo_int64_t  (**ltmp_1482_1029) ();
  plzo_int64_t  (*ltmp_1483_102A) ();
  plzo_int64_t ltmp_1484_A;
  plzo_uint64_t ltmp_1485_9;
  struct l_struct_2E_A7 *ltmp_1486_1038;
  plzo_uint64_t ltmp_1487_9;
  plzo_uint64_t ltmp_1487_9_2E;
  plzo_uint64_t ltmp_1488_9;
  plzo_uint64_t ltmp_1489_9;
  plzo_uint64_t ltmp_1490_9;
  plzo_uint64_t ltmp_1490_9_2E;
  plzo_uint64_t ltmp_1491_9;
  plzo_uint64_t ltmp_1491_9_2E;
  struct l_struct_2E_A7 *ltmp_1492_1038;
  plzo_uint64_t ltmp_1493_9;
  plzo_uint64_t ltmp_1493_9_2E;
  plzo_uint64_t ltmp_1494_9;
  plzo_uint64_t ltmp_1494_9_2E;

  ltmp_1464_1038 = *(&ltmp_1461_1023->field19.field0);
  if ((ltmp_1462_9 < ltmp_1463_9)) {
    goto ltmp_1495_D;
  } else {
    ltmp_1493_9_2E = ltmp_1462_9;
    ltmp_1494_9_2E = ltmp_1462_9;
    goto ltmp_1496_D;
  }

ltmp_1495_D:
  ltmp_1470_102C = &ltmp_1461_1023->field0;
  ltmp_1471_9_2E = PLZO_UINT64_C(0);
  ltmp_1472_9_2E = ltmp_1462_9;
  ltmp_1473_9_2E = ltmp_1462_9;
  ltmp_1474_9_2E = ltmp_1462_9;
  ltmp_1475_1038_2E = ltmp_1464_1038;
  goto ltmp_1497_D;

  do {
ltmp_1497_D:
  ltmp_1471_9 = ltmp_1471_9_2E;
  ltmp_1472_9 = ltmp_1472_9_2E;
  ltmp_1473_9 = ltmp_1473_9_2E;
  ltmp_1474_9 = ltmp_1474_9_2E;
  ltmp_1475_1038 = ltmp_1475_1038_2E;
  ltmp_1476_9 = ltmp_1471_9 + ltmp_1462_9;
  ltmp_1477_9 = *(&ltmp_1475_1038->field0);
  ltmp_1478_9 = (((ltmp_1477_9 == PLZO_UINT64_C(0))) ? (PLZO_UINT64_C(1)) : (ltmp_1477_9));
  if ((ltmp_1476_9 == ltmp_1472_9)) {
    goto ltmp_1498_D;
  } else {
    goto ltmp_1499_D;
  }

ltmp_1500_D:
  ltmp_1465_9 = ltmp_1465_9_2E;
  ltmp_1466_9 = ltmp_1466_9_2E;
  ltmp_1467_9 = ltmp_1467_9_2E;
  ltmp_1468_1038 = ltmp_1468_1038_2E;
  ltmp_1469_9 = ltmp_1471_9 + PLZO_UINT64_C(1);
  if (((ltmp_1476_9 + PLZO_UINT64_C(1)) < ltmp_1463_9)) {
    ltmp_1471_9_2E = ltmp_1469_9;
    ltmp_1472_9_2E = ltmp_1465_9;
    ltmp_1473_9_2E = ltmp_1466_9;
    ltmp_1474_9_2E = ltmp_1467_9;
    ltmp_1475_1038_2E = ltmp_1468_1038;
    goto ltmp_1497_D;
  } else {
    ltmp_1493_9_2E = ltmp_1466_9;
    ltmp_1494_9_2E = ltmp_1467_9;
    goto ltmp_1496_D;
  }

ltmp_1501_D:
  ltmp_1485_9 = ltmp_1478_9 + ltmp_1476_9;
  ltmp_1486_1038 = &ltmp_1475_1038[PLZO_INT64_C(1)];
  ltmp_1465_9_2E = ltmp_1485_9;
  ltmp_1466_9_2E = ltmp_1476_9;
  ltmp_1467_9_2E = ltmp_1474_9;
  ltmp_1468_1038_2E = ltmp_1486_1038;
  goto ltmp_1500_D;

ltmp_1502_D:
  ltmp_1482_1029 = *ltmp_1470_102C;
  ltmp_1483_102A = *(&ltmp_1482_1029[PLZO_INT64_C(21)]);
  ltmp_1484_A = ((plzo_int64_t  (*) (struct l_struct_2E_E *, struct l_struct_2E_A7 *))ltmp_1483_102A)(ltmp_1461_1023, ltmp_1475_1038);
  if ((ltmp_1484_A == PLZO_INT64_C(0))) {
    ltmp_1487_9_2E = ltmp_1474_9;
    goto ltmp_1503_D;
  } else {
    goto ltmp_1501_D;
  }

ltmp_1498_D:
  ltmp_1479_1029 = *ltmp_1470_102C;
  ltmp_1480_102A = *(&ltmp_1479_1029[PLZO_INT64_C(20)]);
  ltmp_1481_A = ((plzo_int64_t  (*) (struct l_struct_2E_E *, struct l_struct_2E_A7 *))ltmp_1480_102A)(ltmp_1461_1023, ltmp_1475_1038);
  if ((ltmp_1481_A == PLZO_INT64_C(0))) {
    goto ltmp_1502_D;
  } else {
    ltmp_1487_9_2E = ltmp_1476_9;
    goto ltmp_1503_D;
  }

ltmp_1504_D:
  ltmp_1490_9 = ltmp_1490_9_2E;
  ltmp_1491_9 = ltmp_1491_9_2E;
  ltmp_1492_1038 = &ltmp_1475_1038[PLZO_INT64_C(1)];
  ltmp_1465_9_2E = ltmp_1490_9;
  ltmp_1466_9_2E = ltmp_1473_9;
  ltmp_1467_9_2E = ltmp_1491_9;
  ltmp_1468_1038_2E = ltmp_1492_1038;
  goto ltmp_1500_D;

ltmp_1503_D:
  ltmp_1487_9 = ltmp_1487_9_2E;
  ltmp_1488_9 = ltmp_1478_9 + ltmp_1476_9;
  ltmp_1490_9_2E = ltmp_1488_9;
  ltmp_1491_9_2E = ltmp_1487_9;
  goto ltmp_1504_D;

ltmp_1499_D:
  ltmp_1489_9 = ltmp_1478_9 + ltmp_1476_9;
  if ((ltmp_1489_9 > ltmp_1472_9)) {
    goto ltmp_1505_D;
  } else {
    ltmp_1490_9_2E = ltmp_1472_9;
    ltmp_1491_9_2E = ltmp_1474_9;
    goto ltmp_1504_D;
  }

ltmp_1505_D:
  ltmp_1490_9_2E = ltmp_1489_9;
  ltmp_1491_9_2E = ltmp_1474_9;
  goto ltmp_1504_D;

  } while (1);
ltmp_1496_D:
  ltmp_1493_9 = ltmp_1493_9_2E;
  ltmp_1494_9 = ltmp_1494_9_2E;
  if ((ltmp_1494_9 == ltmp_1462_9)) {
    goto ltmp_1506_D;
  } else {
    goto ltmp_1507_D;
  }

ltmp_1506_D:
  return ((((ltmp_1493_9 == ltmp_1462_9)) ? (ltmp_1463_9) : (ltmp_1493_9)));
ltmp_1507_D:
  return ltmp_1494_9;
}


static struct l_struct_2E_M *ltmp_18_100A(struct l_struct_2E_A6 *ltmp_1508_1036) {
  plzo_uint64_t *ltmp_1509_19;
  plzo_uint64_t ltmp_1510_9;
  plzo_uint64_t ltmp_1511_9;
  plzo_uint64_t ltmp_1512_9;
  plzo_uint64_t *ltmp_1513_19;
  plzo_uint64_t ltmp_1514_9;
  plzo_uint64_t ltmp_1515_9;
  plzo_uint64_t ltmp_1516_9;
  plzo_uint64_t ltmp_1517_9;
  plzo_uint64_t *ltmp_1518_19;
  plzo_uint64_t ltmp_1519_9;
  struct l_struct_2E_M *ltmp_1520_1032;
  plzo_uint64_t ltmp_1521_9;
  plzo_uint64_t ltmp_1522_9;
  struct l_struct_2E_M *ltmp_1523_1032;
  plzo_uint64_t ltmp_1524_9;
  struct l_struct_2E_M *ltmp_1525_1032;
  plzo_uint64_t ltmp_1526_9;

  ltmp_1509_19 = &ltmp_1508_1036->field20;
  ltmp_1510_9 = *ltmp_1509_19;
  ltmp_1511_9 = ltmp_1510_9 + PLZO_UINT64_C(1);
  *ltmp_1509_19 = ltmp_1511_9;
  ltmp_1512_9 = *(&ltmp_1508_1036->field19);
  ltmp_1513_19 = &ltmp_1508_1036->field21;
  if ((ltmp_1511_9 == ltmp_1512_9)) {
    goto ltmp_1527_D;
  } else {
    goto ltmp_1528_D;
  }

ltmp_1527_D:
  *ltmp_1509_19 = PLZO_UINT64_C(0);
  ltmp_1514_9 = *ltmp_1513_19;
  if ((ltmp_1514_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1529_D;
  } else {
    goto ltmp_1530_D;
  }

ltmp_1528_D:
  ltmp_1515_9 = *ltmp_1513_19;
  if ((ltmp_1511_9 == ltmp_1515_9)) {
    goto ltmp_1529_D;
  } else {
    goto ltmp_1530_D;
  }

ltmp_1529_D:
  ltmp_1516_9 = *ltmp_1513_19;
  ltmp_1517_9 = ltmp_1516_9 + PLZO_UINT64_C(1);
  *ltmp_1513_19 = ltmp_1517_9;
  ltmp_1518_19 = &ltmp_1508_1036->field1;
  if ((ltmp_1517_9 == ltmp_1512_9)) {
    goto ltmp_1531_D;
  } else {
    goto ltmp_1532_D;
  }

ltmp_1531_D:
  *ltmp_1513_19 = PLZO_UINT64_C(0);
  ltmp_1519_9 = *ltmp_1518_19;
  *ltmp_1518_19 = (ltmp_1519_9 + PLZO_UINT64_C(1));
  ltmp_1520_1032 = *(&ltmp_1508_1036->field18);
  ltmp_1521_9 = *ltmp_1509_19;
  return (&ltmp_1520_1032[ltmp_1521_9]);
ltmp_1532_D:
  ltmp_1522_9 = *ltmp_1518_19;
  *ltmp_1518_19 = (ltmp_1522_9 + PLZO_UINT64_C(1));
  ltmp_1523_1032 = *(&ltmp_1508_1036->field18);
  ltmp_1524_9 = *ltmp_1509_19;
  return (&ltmp_1523_1032[ltmp_1524_9]);
ltmp_1530_D:
  ltmp_1525_1032 = *(&ltmp_1508_1036->field18);
  ltmp_1526_9 = *ltmp_1509_19;
  return (&ltmp_1525_1032[ltmp_1526_9]);
}


static void ltmp_19_100B(struct l_struct_2E_A8 *ltmp_1533_1034) {
  struct l_struct_2E_A6 *ltmp_1534_1036;
  struct l_struct_2E_A9 **ltmp_1535_1035;
  struct l_struct_2E_A9 *ltmp_1536_102F;
  void  (*ltmp_1537_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_M *ltmp_1538_1032;
  struct l_struct_2E_A9 *ltmp_1539_102F;
  void  (*ltmp_1540_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A7 *ltmp_1541_1038;
  struct l_struct_2E_A9 *ltmp_1542_102F;
  void  (*ltmp_1543_1019) (struct plzo_callback_t *, signed char *);
  unsigned char *ltmp_1544_13;
  plzo_uint64_t ltmp_1545_9;
  plzo_uint64_t ltmp_1545_9_2E;
  plzo_uint64_t ltmp_1546_9;
  unsigned char *ltmp_1547_13;
  plzo_uint64_t ltmp_1548_9;
  plzo_uint64_t ltmp_1548_9_2E;
  plzo_uint64_t ltmp_1549_9;

  if ((ltmp_1533_1034 == ((struct l_struct_2E_A8 *)0))) {
    goto ltmp_1550_D;
  } else {
    goto ltmp_1551_D;
  }

ltmp_1551_D:
  ltmp_1534_1036 = *(&ltmp_1533_1034->field7);
  if ((ltmp_1534_1036 == ((struct l_struct_2E_A6 *)0))) {
    goto ltmp_1552_D;
  } else {
    goto ltmp_1553_D;
  }

ltmp_1553_D:
  ltmp_1535_1035 = &ltmp_1533_1034->field6;
  ltmp_1536_102F = *ltmp_1535_1035;
  ltmp_1537_1019 = *(&ltmp_1536_102F->field2.field1);
  ltmp_1538_1032 = *(&ltmp_1534_1036->field18);
  ltmp_1537_1019((&ltmp_1536_102F->field2), ((signed char *)ltmp_1538_1032));
  ltmp_1539_102F = *ltmp_1535_1035;
  ltmp_1540_1019 = *(&ltmp_1539_102F->field2.field1);
  ltmp_1541_1038 = *(&ltmp_1534_1036->field17);
  ltmp_1540_1019((&ltmp_1539_102F->field2), ((signed char *)ltmp_1541_1038));
  ltmp_1542_102F = *ltmp_1535_1035;
  ltmp_1543_1019 = *(&ltmp_1542_102F->field2.field1);
  ltmp_1543_1019((&ltmp_1542_102F->field2), ((signed char *)ltmp_1534_1036));
  ltmp_1544_13 = (unsigned char *)ltmp_1533_1034;
  ltmp_1545_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1554_D;

  do {
ltmp_1554_D:
  ltmp_1545_9 = ltmp_1545_9_2E;
  *(&ltmp_1544_13[ltmp_1545_9]) = 0;
  ltmp_1546_9 = ltmp_1545_9 + PLZO_UINT64_C(1);
  if ((ltmp_1546_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1555_D;
  } else {
    ltmp_1545_9_2E = ltmp_1546_9;
    goto ltmp_1554_D;
  }

  } while (1);
ltmp_1555_D:
  return;
ltmp_1552_D:
  ltmp_1547_13 = (unsigned char *)ltmp_1533_1034;
  ltmp_1548_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1556_D;

  do {
ltmp_1556_D:
  ltmp_1548_9 = ltmp_1548_9_2E;
  *(&ltmp_1547_13[ltmp_1548_9]) = 0;
  ltmp_1549_9 = ltmp_1548_9 + PLZO_UINT64_C(1);
  if ((ltmp_1549_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1557_D;
  } else {
    ltmp_1548_9_2E = ltmp_1549_9;
    goto ltmp_1556_D;
  }

  } while (1);
ltmp_1557_D:
  return;
ltmp_1550_D:
  return;
}


static void ltmp_20_100C(struct l_struct_2E_A5 *ltmp_1558_1024) {
  unsigned char *ltmp_1559_13;
  plzo_uint64_t ltmp_1560_9;
  plzo_uint64_t ltmp_1561_9;
  plzo_uint64_t *ltmp_1562_19;
  plzo_uint64_t ltmp_1563_9;
  plzo_uint64_t ltmp_1564_9;
  plzo_uint64_t ltmp_1565_9;
  plzo_uint64_t ltmp_1566_9;
  plzo_uint64_t *ltmp_1567_19;
  plzo_uint64_t ltmp_1568_9;
  unsigned char *ltmp_1569_13;
  plzo_uint64_t ltmp_1570_9;
  unsigned char *ltmp_1571_13;
  unsigned char *ltmp_1571_13_2E;
  unsigned char *ltmp_1572_13;
  plzo_uint64_t ltmp_1573_9;
  unsigned char *ltmp_1574_13;
  unsigned char *ltmp_1574_13_2E;
  plzo_uint64_t ltmp_1575_9;
  unsigned char *ltmp_1576_13;
  unsigned char *ltmp_1576_13_2E;
  unsigned char *ltmp_1577_13;
  unsigned char *ltmp_1577_13_2E;

  ltmp_1559_13 = *(&ltmp_1558_1024->field5);
  ltmp_1560_9 = *(&ltmp_1558_1024->field0);
  ltmp_1561_9 = *(&ltmp_1558_1024->field2);
  if ((ltmp_1561_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1578_D;
  } else {
    goto ltmp_1579_D;
  }

ltmp_1578_D:
  *ltmp_1559_13 = ((unsigned char )ltmp_1560_9);
  ltmp_1562_19 = &ltmp_1558_1024->field3;
  ltmp_1563_9 = *ltmp_1562_19;
  if ((ltmp_1563_9 > PLZO_UINT64_C(15))) {
    goto ltmp_1580_D;
  } else {
    goto ltmp_1581_D;
  }

ltmp_1580_D:
  *(&ltmp_1559_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1560_9 >> 8));
  ltmp_1564_9 = *ltmp_1562_19;
  if ((ltmp_1564_9 > PLZO_UINT64_C(23))) {
    goto ltmp_1582_D;
  } else {
    goto ltmp_1583_D;
  }

ltmp_1581_D:
  if ((ltmp_1563_9 > PLZO_UINT64_C(23))) {
    goto ltmp_1582_D;
  } else {
    goto ltmp_1583_D;
  }

ltmp_1582_D:
  *(&ltmp_1559_13[PLZO_INT64_C(2)]) = ((unsigned char )(ltmp_1560_9 >> 16));
  ltmp_1565_9 = *ltmp_1562_19;
  if ((ltmp_1565_9 > PLZO_UINT64_C(31))) {
    goto ltmp_1584_D;
  } else {
    goto ltmp_1585_D;
  }

ltmp_1583_D:
  ltmp_1566_9 = *ltmp_1562_19;
  if ((ltmp_1566_9 > PLZO_UINT64_C(31))) {
    goto ltmp_1584_D;
  } else {
    goto ltmp_1585_D;
  }

ltmp_1584_D:
  *(&ltmp_1559_13[PLZO_INT64_C(3)]) = ((unsigned char )(ltmp_1560_9 >> 24));
  return;
ltmp_1579_D:
  ltmp_1567_19 = &ltmp_1558_1024->field3;
  ltmp_1568_9 = *ltmp_1567_19;
  if ((ltmp_1568_9 > PLZO_UINT64_C(31))) {
    goto ltmp_1586_D;
  } else {
    goto ltmp_1587_D;
  }

ltmp_1586_D:
  ltmp_1569_13 = &ltmp_1559_13[PLZO_INT64_C(1)];
  *ltmp_1559_13 = ((unsigned char )(ltmp_1560_9 >> 24));
  ltmp_1570_9 = *ltmp_1567_19;
  if ((ltmp_1570_9 > PLZO_UINT64_C(23))) {
    ltmp_1571_13_2E = ltmp_1569_13;
    goto ltmp_1588_D;
  } else {
    ltmp_1574_13_2E = ltmp_1569_13;
    goto ltmp_1589_D;
  }

ltmp_1587_D:
  if ((ltmp_1568_9 > PLZO_UINT64_C(23))) {
    ltmp_1571_13_2E = ltmp_1559_13;
    goto ltmp_1588_D;
  } else {
    ltmp_1574_13_2E = ltmp_1559_13;
    goto ltmp_1589_D;
  }

ltmp_1588_D:
  ltmp_1571_13 = ltmp_1571_13_2E;
  ltmp_1572_13 = &ltmp_1571_13[PLZO_INT64_C(1)];
  *ltmp_1571_13 = ((unsigned char )(ltmp_1560_9 >> 16));
  ltmp_1573_9 = *ltmp_1567_19;
  if ((ltmp_1573_9 > PLZO_UINT64_C(15))) {
    ltmp_1576_13_2E = ltmp_1572_13;
    goto ltmp_1590_D;
  } else {
    ltmp_1577_13_2E = ltmp_1572_13;
    goto ltmp_1591_D;
  }

ltmp_1589_D:
  ltmp_1574_13 = ltmp_1574_13_2E;
  ltmp_1575_9 = *ltmp_1567_19;
  if ((ltmp_1575_9 > PLZO_UINT64_C(15))) {
    ltmp_1576_13_2E = ltmp_1574_13;
    goto ltmp_1590_D;
  } else {
    ltmp_1577_13_2E = ltmp_1574_13;
    goto ltmp_1591_D;
  }

ltmp_1590_D:
  ltmp_1576_13 = ltmp_1576_13_2E;
  *ltmp_1576_13 = ((unsigned char )(ltmp_1560_9 >> 8));
  *(&ltmp_1576_13[PLZO_INT64_C(1)]) = ((unsigned char )ltmp_1560_9);
  return;
ltmp_1591_D:
  ltmp_1577_13 = ltmp_1577_13_2E;
  *ltmp_1577_13 = ((unsigned char )ltmp_1560_9);
  return;
ltmp_1585_D:
  return;
}


static void ltmp_21_100D(struct l_struct_2E_Lzo1xE *ltmp_1592_103C, plzo_uint64_t ltmp_1593_9, plzo_uint64_t ltmp_1594_9) {
  unsigned char **ltmp_1595_1025;
  unsigned char *ltmp_1596_13;
  plzo_uint64_t ltmp_1597_9;
  plzo_uint64_t ltmp_1598_9;
  unsigned char *ltmp_1599_13;
  plzo_uint64_t ltmp_1600_9;
  plzo_uint64_t ltmp_1601_9;
  plzo_uint64_t ltmp_1602_9;
  plzo_int64_t ltmp_1603_A;
  unsigned char *ltmp_1604_13;
  unsigned char ltmp_1605_3;
  unsigned char *ltmp_1606_13;
  unsigned char *ltmp_1607_13;
  plzo_uint64_t ltmp_1608_9;
  plzo_uint64_t ltmp_1609_9;
  plzo_uint64_t ltmp_1610_9;
  plzo_uint64_t ltmp_1611_9;
  plzo_uint64_t ltmp_1612_9;
  plzo_uint64_t ltmp_1613_9;
  unsigned char *ltmp_1614_13;
  plzo_uint64_t ltmp_1615_9;
  plzo_uint64_t ltmp_1616_9;
  plzo_uint64_t ltmp_1617_9;
  unsigned char *ltmp_1618_13;
  plzo_uint64_t ltmp_1619_9;
  plzo_uint64_t ltmp_1620_9;
  unsigned char *ltmp_1621_13;
  plzo_uint64_t ltmp_1622_9;
  plzo_uint64_t ltmp_1623_9;
  plzo_uint64_t ltmp_1623_9_2E;
  plzo_uint64_t ltmp_1624_9;
  plzo_uint64_t ltmp_1625_9;
  unsigned char *ltmp_1626_13;
  unsigned char *ltmp_1627_13;
  unsigned char *ltmp_1627_13_2E;
  plzo_uint64_t ltmp_1628_9;
  plzo_uint64_t ltmp_1628_9_2E;
  unsigned char *ltmp_1629_13;
  unsigned char *ltmp_1630_13;
  unsigned char *ltmp_1630_13_2E;
  unsigned char *ltmp_1631_13;
  plzo_uint64_t ltmp_1632_9;
  plzo_uint64_t ltmp_1633_9;
  plzo_uint64_t ltmp_1634_9;
  unsigned char *ltmp_1635_13;
  plzo_uint64_t ltmp_1636_9;
  plzo_uint64_t ltmp_1637_9;
  unsigned char *ltmp_1638_13;
  plzo_uint64_t ltmp_1639_9;
  plzo_uint64_t ltmp_1640_9;
  plzo_uint64_t ltmp_1640_9_2E;
  plzo_uint64_t ltmp_1641_9;
  plzo_uint64_t ltmp_1642_9;
  unsigned char *ltmp_1643_13;
  unsigned char *ltmp_1644_13;
  unsigned char *ltmp_1644_13_2E;
  plzo_uint64_t ltmp_1645_9;
  plzo_uint64_t ltmp_1645_9_2E;
  unsigned char *ltmp_1646_13;
  unsigned char *ltmp_1647_13;
  unsigned char *ltmp_1647_13_2E;
  unsigned char *ltmp_1648_13;
  unsigned char *ltmp_1649_13;
  unsigned char *ltmp_1649_13_2E;

  ltmp_0_1000((&ltmp_1592_103C->field0.field0), ltmp_1593_9, ltmp_1594_9);
  ltmp_1595_1025 = &ltmp_1592_103C->field0.field0.field1.field6;
  ltmp_1596_13 = *ltmp_1595_1025;
  if ((ltmp_1593_9 == PLZO_UINT64_C(2))) {
    goto ltmp_1650_D;
  } else {
    goto ltmp_1651_D;
  }

ltmp_1650_D:
  ltmp_1597_9 = ltmp_1594_9 + PLZO_UINT64_C(18446744073709551615);
  ltmp_1598_9 = *(&ltmp_1592_103C->field0.field1);
  *ltmp_1596_13 = (((unsigned char )(((unsigned char )ltmp_1598_9) | (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1597_9) << 2))) & 12))))));
  ltmp_1599_13 = &ltmp_1596_13[PLZO_INT64_C(2)];
  *(&ltmp_1596_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1597_9 >> 2));
  ltmp_1649_13_2E = ltmp_1599_13;
  goto ltmp_1652_D;

ltmp_1651_D:
  ltmp_1600_9 = *(&ltmp_1592_103C->field0.field18);
  if ((ltmp_1600_9 < ltmp_1593_9)) {
    goto ltmp_1653_D;
  } else {
    goto ltmp_1654_D;
  }

ltmp_1654_D:
  ltmp_1601_9 = *(&ltmp_1592_103C->field0.field7);
  if ((ltmp_1601_9 < ltmp_1594_9)) {
    goto ltmp_1653_D;
  } else {
    goto ltmp_1655_D;
  }

ltmp_1655_D:
  ltmp_1602_9 = ltmp_1594_9 + PLZO_UINT64_C(18446744073709551615);
  ltmp_1603_A = *(&ltmp_1592_103C->field0.field0.field6.field0);
  ltmp_1604_13 = &ltmp_1596_13[PLZO_INT64_C(1)];
  ltmp_1605_3 = (unsigned char )ltmp_1593_9;
  if ((ltmp_1603_A == PLZO_INT64_C(0))) {
    goto ltmp_1656_D;
  } else {
    goto ltmp_1657_D;
  }

ltmp_1656_D:
  *ltmp_1596_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1602_9) << 2))) & 28))) | (((unsigned char )((((unsigned char )(ltmp_1605_3 << 5))) + 224))))));
  ltmp_1606_13 = &ltmp_1596_13[PLZO_INT64_C(2)];
  *ltmp_1604_13 = ((unsigned char )(ltmp_1602_9 >> 3));
  ltmp_1649_13_2E = ltmp_1606_13;
  goto ltmp_1652_D;

ltmp_1657_D:
  *ltmp_1596_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1602_9) << 2))) & 12))) | (((unsigned char )((((unsigned char )(ltmp_1605_3 << 4))) + 16))))));
  ltmp_1607_13 = &ltmp_1596_13[PLZO_INT64_C(2)];
  *ltmp_1604_13 = ((unsigned char )(ltmp_1602_9 >> 2));
  ltmp_1649_13_2E = ltmp_1607_13;
  goto ltmp_1652_D;

ltmp_1653_D:
  ltmp_1608_9 = *(&ltmp_1592_103C->field0.field13);
  if ((ltmp_1608_9 == ltmp_1593_9)) {
    goto ltmp_1658_D;
  } else {
    goto ltmp_1659_D;
  }

ltmp_1658_D:
  ltmp_1609_9 = *(&ltmp_1592_103C->field0.field11);
  if ((ltmp_1609_9 < ltmp_1594_9)) {
    goto ltmp_1659_D;
  } else {
    goto ltmp_1660_D;
  }

ltmp_1660_D:
  ltmp_1610_9 = *(&ltmp_1592_103C->field0.field0.field17);
  if ((ltmp_1610_9 > PLZO_UINT64_C(3))) {
    goto ltmp_1661_D;
  } else {
    goto ltmp_1659_D;
  }

ltmp_1661_D:
  ltmp_1611_9 = *(&ltmp_1592_103C->field0.field7);
  ltmp_1612_9 = (ltmp_1594_9 - ltmp_1611_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_1613_9 = *(&ltmp_1592_103C->field0.field1);
  *ltmp_1596_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1612_9) << 2))) & 12))) | ((unsigned char )ltmp_1613_9))));
  ltmp_1614_13 = &ltmp_1596_13[PLZO_INT64_C(2)];
  *(&ltmp_1596_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1612_9 >> 2));
  ltmp_1649_13_2E = ltmp_1614_13;
  goto ltmp_1652_D;

ltmp_1659_D:
  ltmp_1615_9 = *(&ltmp_1592_103C->field0.field8);
  if ((ltmp_1615_9 < ltmp_1594_9)) {
    goto ltmp_1662_D;
  } else {
    goto ltmp_1663_D;
  }

ltmp_1663_D:
  ltmp_1616_9 = ltmp_1594_9 + PLZO_UINT64_C(18446744073709551615);
  ltmp_1617_9 = *(&ltmp_1592_103C->field0.field19);
  if ((ltmp_1617_9 < ltmp_1593_9)) {
    goto ltmp_1664_D;
  } else {
    goto ltmp_1665_D;
  }

ltmp_1665_D:
  ltmp_1618_13 = &ltmp_1596_13[PLZO_INT64_C(1)];
  ltmp_1619_9 = *(&ltmp_1592_103C->field0.field3);
  *ltmp_1596_13 = (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1593_9) + 254))) + ((unsigned char )ltmp_1619_9))));
  ltmp_1630_13_2E = ltmp_1618_13;
  goto ltmp_1666_D;

ltmp_1664_D:
  ltmp_1620_9 = ltmp_1593_9 - ltmp_1617_9;
  ltmp_1621_13 = &ltmp_1596_13[PLZO_INT64_C(1)];
  ltmp_1622_9 = *(&ltmp_1592_103C->field0.field3);
  *ltmp_1596_13 = ((unsigned char )ltmp_1622_9);
  if ((ltmp_1620_9 > PLZO_UINT64_C(255))) {
    ltmp_1623_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1667_D;
  } else {
    ltmp_1627_13_2E = ltmp_1621_13;
    ltmp_1628_9_2E = ltmp_1620_9;
    goto ltmp_1668_D;
  }

  do {
ltmp_1667_D:
  ltmp_1623_9 = ltmp_1623_9_2E;
  ltmp_1624_9 = ltmp_1623_9 + PLZO_UINT64_C(1);
  ltmp_1625_9 = ((ltmp_1623_9*PLZO_UINT64_C(18446744073709551361)) + ltmp_1620_9) + PLZO_UINT64_C(18446744073709551361);
  *(&ltmp_1596_13[ltmp_1624_9]) = 0;
  if ((ltmp_1625_9 > PLZO_UINT64_C(255))) {
    ltmp_1623_9_2E = ltmp_1624_9;
    goto ltmp_1667_D;
  } else {
    goto ltmp_1669_D;
  }

  } while (1);
ltmp_1669_D:
  ltmp_1626_13 = &ltmp_1596_13[(((plzo_int64_t )ltmp_1623_9) + PLZO_INT64_C(2))];
  ltmp_1627_13_2E = ltmp_1626_13;
  ltmp_1628_9_2E = ltmp_1625_9;
  goto ltmp_1668_D;

ltmp_1668_D:
  ltmp_1627_13 = ltmp_1627_13_2E;
  ltmp_1628_9 = ltmp_1628_9_2E;
  ltmp_1629_13 = &ltmp_1627_13[PLZO_INT64_C(1)];
  *ltmp_1627_13 = ((unsigned char )ltmp_1628_9);
  ltmp_1630_13_2E = ltmp_1629_13;
  goto ltmp_1666_D;

ltmp_1666_D:
  ltmp_1630_13 = ltmp_1630_13_2E;
  *ltmp_1630_13 = (((unsigned char )(((unsigned char )ltmp_1616_9) << 2)));
  ltmp_1631_13 = &ltmp_1630_13[PLZO_INT64_C(2)];
  *(&ltmp_1630_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1616_9 >> 6));
  ltmp_1649_13_2E = ltmp_1631_13;
  goto ltmp_1652_D;

ltmp_1662_D:
  ltmp_1632_9 = ltmp_1594_9 - ltmp_1615_9;
  ltmp_1633_9 = (ltmp_1632_9 >> 11) & PLZO_UINT64_C(8);
  ltmp_1634_9 = *(&ltmp_1592_103C->field0.field20);
  if ((ltmp_1634_9 < ltmp_1593_9)) {
    goto ltmp_1670_D;
  } else {
    goto ltmp_1671_D;
  }

ltmp_1671_D:
  ltmp_1635_13 = &ltmp_1596_13[PLZO_INT64_C(1)];
  ltmp_1636_9 = *(&ltmp_1592_103C->field0.field4);
  *ltmp_1596_13 = (((unsigned char )(((unsigned char )ltmp_1636_9) + (((unsigned char )(((unsigned char )ltmp_1633_9) | (((unsigned char )(((unsigned char )ltmp_1593_9) + 254)))))))));
  ltmp_1647_13_2E = ltmp_1635_13;
  goto ltmp_1672_D;

ltmp_1670_D:
  ltmp_1637_9 = ltmp_1593_9 - ltmp_1634_9;
  ltmp_1638_13 = &ltmp_1596_13[PLZO_INT64_C(1)];
  ltmp_1639_9 = *(&ltmp_1592_103C->field0.field4);
  *ltmp_1596_13 = (((unsigned char )(((unsigned char )ltmp_1639_9) + ((unsigned char )ltmp_1633_9))));
  if ((ltmp_1637_9 > PLZO_UINT64_C(255))) {
    ltmp_1640_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1673_D;
  } else {
    ltmp_1644_13_2E = ltmp_1638_13;
    ltmp_1645_9_2E = ltmp_1637_9;
    goto ltmp_1674_D;
  }

  do {
ltmp_1673_D:
  ltmp_1640_9 = ltmp_1640_9_2E;
  ltmp_1641_9 = ltmp_1640_9 + PLZO_UINT64_C(1);
  ltmp_1642_9 = ((ltmp_1640_9*PLZO_UINT64_C(18446744073709551361)) + ltmp_1637_9) + PLZO_UINT64_C(18446744073709551361);
  *(&ltmp_1596_13[ltmp_1641_9]) = 0;
  if ((ltmp_1642_9 > PLZO_UINT64_C(255))) {
    ltmp_1640_9_2E = ltmp_1641_9;
    goto ltmp_1673_D;
  } else {
    goto ltmp_1675_D;
  }

  } while (1);
ltmp_1675_D:
  ltmp_1643_13 = &ltmp_1596_13[(((plzo_int64_t )ltmp_1640_9) + PLZO_INT64_C(2))];
  ltmp_1644_13_2E = ltmp_1643_13;
  ltmp_1645_9_2E = ltmp_1642_9;
  goto ltmp_1674_D;

ltmp_1674_D:
  ltmp_1644_13 = ltmp_1644_13_2E;
  ltmp_1645_9 = ltmp_1645_9_2E;
  ltmp_1646_13 = &ltmp_1644_13[PLZO_INT64_C(1)];
  *ltmp_1644_13 = ((unsigned char )ltmp_1645_9);
  ltmp_1647_13_2E = ltmp_1646_13;
  goto ltmp_1672_D;

ltmp_1672_D:
  ltmp_1647_13 = ltmp_1647_13_2E;
  *ltmp_1647_13 = (((unsigned char )(((unsigned char )ltmp_1632_9) << 2)));
  ltmp_1648_13 = &ltmp_1647_13[PLZO_INT64_C(2)];
  *(&ltmp_1647_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1632_9 >> 6));
  ltmp_1649_13_2E = ltmp_1648_13;
  goto ltmp_1652_D;

ltmp_1652_D:
  ltmp_1649_13 = ltmp_1649_13_2E;
  *(&ltmp_1592_103C->field0.field0.field15) = ltmp_1593_9;
  *(&ltmp_1592_103C->field0.field0.field16) = ltmp_1594_9;
  *ltmp_1595_1025 = ltmp_1649_13;
  return;
}


static void ltmp_22_100E(struct l_struct_2E_Lzo1xE *ltmp_1676_103C, unsigned char *ltmp_1677_13, plzo_uint64_t ltmp_1678_9, plzo_uint64_t ltmp_1679_9) {
  plzo_uint64_t *ltmp_1680_19;
  plzo_uint64_t ltmp_1681_9;
  plzo_uint64_t *ltmp_1682_19;
  plzo_uint64_t ltmp_1683_9;
  plzo_uint64_t ltmp_1684_9;
  plzo_uint64_t *ltmp_1685_19;
  unsigned char **ltmp_1686_1025;
  unsigned char *ltmp_1687_13;
  unsigned char *ltmp_1688_13;
  unsigned char *ltmp_1689_13;
  unsigned char *ltmp_1690_13;
  unsigned char ltmp_1691_3;
  unsigned char *ltmp_1692_13;
  plzo_uint64_t ltmp_1693_9;
  unsigned char *ltmp_1694_13;
  plzo_uint64_t ltmp_1695_9;
  plzo_uint64_t ltmp_1695_9_2E;
  plzo_uint64_t ltmp_1696_9;
  plzo_uint64_t ltmp_1697_9;
  unsigned char *ltmp_1698_13;
  plzo_uint64_t ltmp_1699_9;
  plzo_uint64_t ltmp_1699_9_2E;
  unsigned char *ltmp_1700_13;
  unsigned char *ltmp_1700_13_2E;
  unsigned char *ltmp_1701_13;
  unsigned char *ltmp_1702_13;
  unsigned char *ltmp_1702_13_2E;
  plzo_uint64_t ltmp_1703_9;
  plzo_uint64_t ltmp_1703_9_2E;
  unsigned char ltmp_1704_3;
  plzo_uint64_t ltmp_1705_9;

  if ((ltmp_1678_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1706_D;
  } else {
    goto ltmp_1707_D;
  }

ltmp_1707_D:
  ltmp_1680_19 = &ltmp_1676_103C->field0.field0.field2.field5;
  ltmp_1681_9 = *ltmp_1680_19;
  ltmp_1682_19 = &ltmp_1676_103C->field0.field0.field2.field4;
  if ((ltmp_1681_9 < ltmp_1678_9)) {
    goto ltmp_1708_D;
  } else {
    goto ltmp_1709_D;
  }

ltmp_1708_D:
  *ltmp_1680_19 = ltmp_1678_9;
  ltmp_1683_9 = *ltmp_1682_19;
  if ((ltmp_1683_9 > ltmp_1678_9)) {
    goto ltmp_1710_D;
  } else {
    goto ltmp_1706_D;
  }

ltmp_1709_D:
  ltmp_1684_9 = *ltmp_1682_19;
  if ((ltmp_1684_9 > ltmp_1678_9)) {
    goto ltmp_1710_D;
  } else {
    goto ltmp_1706_D;
  }

ltmp_1710_D:
  *ltmp_1682_19 = ltmp_1678_9;
  goto ltmp_1706_D;

ltmp_1706_D:
  ltmp_1685_19 = &ltmp_1676_103C->field0.field0.field18;
  if ((ltmp_1678_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1711_D;
  } else {
    goto ltmp_1712_D;
  }

ltmp_1711_D:
  *ltmp_1685_19 = PLZO_UINT64_C(0);
  *(&ltmp_1676_103C->field0.field0.field17) = PLZO_UINT64_C(0);
  return;
ltmp_1712_D:
  *ltmp_1685_19 = ltmp_1679_9;
  *(&ltmp_1676_103C->field0.field0.field17) = ltmp_1678_9;
  ltmp_1686_1025 = &ltmp_1676_103C->field0.field0.field1.field6;
  ltmp_1687_13 = *ltmp_1686_1025;
  ltmp_1688_13 = *(&ltmp_1676_103C->field0.field0.field9);
  if (((ltmp_1688_13 == ltmp_1687_13) & (ltmp_1678_9 < PLZO_UINT64_C(239)))) {
    goto ltmp_1713_D;
  } else {
    goto ltmp_1714_D;
  }

ltmp_1713_D:
  ltmp_1689_13 = &ltmp_1687_13[PLZO_INT64_C(1)];
  *ltmp_1687_13 = (((unsigned char )(((unsigned char )ltmp_1678_9) + 17)));
  ltmp_1702_13_2E = ltmp_1689_13;
  ltmp_1703_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1715_D;

ltmp_1714_D:
  if ((ltmp_1678_9 > PLZO_UINT64_C(3))) {
    goto ltmp_1716_D;
  } else {
    goto ltmp_1717_D;
  }

ltmp_1717_D:
  ltmp_1690_13 = &ltmp_1687_13[PLZO_INT64_C(-2)];
  ltmp_1691_3 = *ltmp_1690_13;
  *ltmp_1690_13 = (((unsigned char )(ltmp_1691_3 | ((unsigned char )ltmp_1678_9))));
  ltmp_1702_13_2E = ltmp_1687_13;
  ltmp_1703_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1715_D;

ltmp_1716_D:
  if ((ltmp_1678_9 > PLZO_UINT64_C(18))) {
    goto ltmp_1718_D;
  } else {
    goto ltmp_1719_D;
  }

ltmp_1719_D:
  ltmp_1692_13 = &ltmp_1687_13[PLZO_INT64_C(1)];
  *ltmp_1687_13 = (((unsigned char )(((unsigned char )ltmp_1678_9) + 253)));
  ltmp_1702_13_2E = ltmp_1692_13;
  ltmp_1703_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1715_D;

ltmp_1718_D:
  ltmp_1693_9 = ltmp_1678_9 + PLZO_UINT64_C(18446744073709551598);
  ltmp_1694_13 = &ltmp_1687_13[PLZO_INT64_C(1)];
  *ltmp_1687_13 = 0;
  if ((ltmp_1693_9 > PLZO_UINT64_C(255))) {
    ltmp_1695_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1720_D;
  } else {
    ltmp_1699_9_2E = ltmp_1693_9;
    ltmp_1700_13_2E = ltmp_1694_13;
    goto ltmp_1721_D;
  }

  do {
ltmp_1720_D:
  ltmp_1695_9 = ltmp_1695_9_2E;
  ltmp_1696_9 = ltmp_1695_9 + PLZO_UINT64_C(1);
  ltmp_1697_9 = ((ltmp_1695_9*PLZO_UINT64_C(18446744073709551361)) + ltmp_1693_9) + PLZO_UINT64_C(18446744073709551361);
  *(&ltmp_1687_13[ltmp_1696_9]) = 0;
  if ((ltmp_1697_9 > PLZO_UINT64_C(255))) {
    ltmp_1695_9_2E = ltmp_1696_9;
    goto ltmp_1720_D;
  } else {
    goto ltmp_1722_D;
  }

  } while (1);
ltmp_1722_D:
  ltmp_1698_13 = &ltmp_1687_13[(((plzo_int64_t )ltmp_1695_9) + PLZO_INT64_C(2))];
  ltmp_1699_9_2E = ltmp_1697_9;
  ltmp_1700_13_2E = ltmp_1698_13;
  goto ltmp_1721_D;

ltmp_1721_D:
  ltmp_1699_9 = ltmp_1699_9_2E;
  ltmp_1700_13 = ltmp_1700_13_2E;
  ltmp_1701_13 = &ltmp_1700_13[PLZO_INT64_C(1)];
  *ltmp_1700_13 = ((unsigned char )ltmp_1699_9);
  ltmp_1702_13_2E = ltmp_1701_13;
  ltmp_1703_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1715_D;

  do {
ltmp_1715_D:
  ltmp_1702_13 = ltmp_1702_13_2E;
  ltmp_1703_9 = ltmp_1703_9_2E;
  ltmp_1704_3 = *(&ltmp_1677_13[ltmp_1703_9]);
  *(&ltmp_1702_13[ltmp_1703_9]) = ltmp_1704_3;
  ltmp_1705_9 = ltmp_1703_9 + PLZO_UINT64_C(1);
  if ((ltmp_1705_9 == ltmp_1678_9)) {
    goto ltmp_1723_D;
  } else {
    ltmp_1702_13_2E = ltmp_1702_13;
    ltmp_1703_9_2E = ltmp_1705_9;
    goto ltmp_1715_D;
  }

  } while (1);
ltmp_1723_D:
  *ltmp_1686_1025 = (&ltmp_1702_13[(((plzo_int64_t )ltmp_1703_9) + PLZO_INT64_C(1))]);
  return;
}


static void ltmp_23_100D(struct l_struct_2E_Lzo1xE *ltmp_1724_103C, plzo_uint64_t ltmp_1725_9, plzo_uint64_t ltmp_1726_9) {
  struct l_struct_2E_A7 *ltmp_1727_1038;
  struct l_struct_2E_A7 *ltmp_1728_1038;
  struct l_struct_2E_A7 *ltmp_1728_1038_2E;
  plzo_uint64_t ltmp_1729_9;
  plzo_uint64_t ltmp_1730_9;
  plzo_uint64_t ltmp_1731_9;
  plzo_uint64_t ltmp_1731_9_2E;
  struct l_struct_2E_A7 *ltmp_1732_1038;
  struct l_struct_2E_A7 *ltmp_1732_1038_2E;
  plzo_uint64_t ltmp_1733_9;
  plzo_uint64_t *ltmp_1734_19;
  plzo_uint64_t ltmp_1735_9;
  struct l_struct_2E_A7 *ltmp_1736_1038;
  struct l_struct_2E_A7 *ltmp_1737_1038;

  ltmp_11_1000((&ltmp_1724_103C->field0.field0), ltmp_1725_9, ltmp_1726_9);
  if ((ltmp_1725_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1738_D;
  } else {
    goto ltmp_1739_D;
  }

ltmp_1738_D:
  ltmp_1727_1038 = *(&ltmp_1724_103C->field0.field0.field19.field0);
  if (((ltmp_1725_9 < ltmp_1726_9) & (ltmp_1725_9 < PLZO_UINT64_C(4)))) {
    ltmp_1731_9_2E = PLZO_UINT64_C(0);
    ltmp_1732_1038_2E = ltmp_1727_1038;
    goto ltmp_1740_D;
  } else {
    goto ltmp_1739_D;
  }

  do {
ltmp_1740_D:
  ltmp_1731_9 = ltmp_1731_9_2E;
  ltmp_1732_1038 = ltmp_1732_1038_2E;
  ltmp_1733_9 = ltmp_1731_9 + ltmp_1725_9;
  ltmp_1734_19 = &ltmp_1732_1038->field0;
  ltmp_1735_9 = *ltmp_1734_19;
  if ((ltmp_1735_9 == PLZO_UINT64_C(2))) {
    goto ltmp_1741_D;
  } else {
    goto ltmp_1742_D;
  }

ltmp_1743_D:
  ltmp_1728_1038 = ltmp_1728_1038_2E;
  ltmp_1729_9 = ltmp_1733_9 + PLZO_UINT64_C(1);
  ltmp_1730_9 = ltmp_1731_9 + PLZO_UINT64_C(1);
  if (((ltmp_1729_9 < ltmp_1726_9) & (ltmp_1729_9 < PLZO_UINT64_C(4)))) {
    ltmp_1731_9_2E = ltmp_1730_9;
    ltmp_1732_1038_2E = ltmp_1728_1038;
    goto ltmp_1740_D;
  } else {
    goto ltmp_1739_D;
  }

ltmp_1741_D:
  *ltmp_1734_19 = PLZO_UINT64_C(0);
  ltmp_1736_1038 = &ltmp_1732_1038[PLZO_INT64_C(1)];
  ltmp_1728_1038_2E = ltmp_1736_1038;
  goto ltmp_1743_D;

ltmp_1742_D:
  ltmp_1737_1038 = &ltmp_1732_1038[PLZO_INT64_C(1)];
  ltmp_1728_1038_2E = ltmp_1737_1038;
  goto ltmp_1743_D;

  } while (1);
ltmp_1739_D:
  return;
}


static void ltmp_24_100F(struct l_struct_2E_Lzo1E *ltmp_1744_103D, plzo_uint64_t ltmp_1745_9, plzo_uint64_t ltmp_1746_9) {

  ltmp_11_1000((&ltmp_1744_103D->field0), ltmp_1745_9, ltmp_1746_9);
  return;
}


static plzo_int64_t plzo_lzo1x_99_compress_internal(unsigned char *ltmp_1747_13, plzo_uint64_t ltmp_1748_9, unsigned char *ltmp_1749_13, plzo_uint64_t *ltmp_1750_19, struct plzo_callback_t *ltmp_1751_1026, plzo_int64_t ltmp_1752_A, struct plzo_compress_config_t *ltmp_1753_1027, plzo_uint64_t *ltmp_1754_19) {
  struct l_struct_2E_Lzo1xE ltmp_1755_103C;
  struct l_struct_2E_E *ltmp_1756_1023;
  plzo_int64_t ltmp_1757_A;
  plzo_int64_t ltmp_1758_A;
  plzo_int64_t ltmp_1759_A;
  plzo_int64_t ltmp_1759_A_2E;
  plzo_int64_t ltmp_1760_A;
  plzo_int64_t ltmp_1761_A;
  plzo_int64_t ltmp_1761_A_2E;
  plzo_uint64_t ltmp_1762_9;
  plzo_uint64_t ltmp_1763_9;
  plzo_uint64_t ltmp_1764_9;
  plzo_uint64_t ltmp_1765_9;
  plzo_uint64_t ltmp_1766_9;
  plzo_uint64_t ltmp_1767_9;
  plzo_uint64_t ltmp_1768_9;
  plzo_uint64_t ltmp_1769_9;

  ltmp_25_1010((&ltmp_1755_103C.field0));
  *(&ltmp_1755_103C.field0.field0.field0) = (&(ltmp_68_1021[PLZO_INT64_C(2)]));
  *(&ltmp_1755_103C.field0.field1) = PLZO_UINT64_C(0);
  *(&ltmp_1755_103C.field0.field2) = PLZO_UINT64_C(64);
  *(&ltmp_1755_103C.field0.field3) = PLZO_UINT64_C(32);
  *(&ltmp_1755_103C.field0.field4) = PLZO_UINT64_C(16);
  *(&ltmp_1755_103C.field0.field5) = PLZO_UINT64_C(0);
  *(&ltmp_1755_103C.field0.field6) = PLZO_UINT64_C(1024);
  *(&ltmp_1755_103C.field0.field7) = PLZO_UINT64_C(2048);
  *(&ltmp_1755_103C.field0.field8) = PLZO_UINT64_C(16384);
  *(&ltmp_1755_103C.field0.field9) = PLZO_UINT64_C(49151);
  *(&ltmp_1755_103C.field0.field10) = PLZO_UINT64_C(0);
  *(&ltmp_1755_103C.field0.field11) = PLZO_UINT64_C(3072);
  *(&ltmp_1755_103C.field0.field12) = PLZO_UINT64_C(2);
  *(&ltmp_1755_103C.field0.field13) = PLZO_UINT64_C(3);
  *(&ltmp_1755_103C.field0.field14) = PLZO_UINT64_C(3);
  *(&ltmp_1755_103C.field0.field15) = PLZO_UINT64_C(3);
  *(&ltmp_1755_103C.field0.field16) = PLZO_UINT64_C(0);
  *(&ltmp_1755_103C.field0.field17) = PLZO_UINT64_C(2);
  *(&ltmp_1755_103C.field0.field18) = PLZO_UINT64_C(8);
  *(&ltmp_1755_103C.field0.field19) = PLZO_UINT64_C(33);
  *(&ltmp_1755_103C.field0.field20) = PLZO_UINT64_C(9);
  *(&ltmp_1755_103C.field0.field21) = PLZO_UINT64_C(0);
  *(&ltmp_1755_103C.field0.field0.field6.field0) = PLZO_INT64_C(0);
  *(&ltmp_1755_103C.field0.field0.field6.field3) = PLZO_UINT64_C(49151);
  ltmp_1756_1023 = &ltmp_1755_103C.field0.field0;
  ltmp_1757_A = ltmp_2_1002(ltmp_1756_1023, ltmp_1752_A, ltmp_1751_1026);
  if (((ltmp_1757_A == PLZO_INT64_C(0)) & (ltmp_1753_1027 != ((struct plzo_compress_config_t *)0)))) {
    goto ltmp_1770_D;
  } else {
    ltmp_1759_A_2E = ltmp_1757_A;
    goto ltmp_1771_D;
  }

ltmp_1770_D:
  ltmp_1758_A = ltmp_3_1003(ltmp_1756_1023, ltmp_1753_1027);
  ltmp_1759_A_2E = ltmp_1758_A;
  goto ltmp_1771_D;

ltmp_1771_D:
  ltmp_1759_A = ltmp_1759_A_2E;
  if ((ltmp_1759_A == PLZO_INT64_C(0))) {
    goto ltmp_1772_D;
  } else {
    ltmp_1761_A_2E = ltmp_1759_A;
    goto ltmp_1773_D;
  }

ltmp_1772_D:
  ltmp_1760_A = ltmp_4_1004(ltmp_1756_1023, ltmp_1747_13, ltmp_1748_9, ltmp_1749_13, ltmp_1750_19);
  ltmp_1761_A_2E = ltmp_1760_A;
  goto ltmp_1773_D;

ltmp_1773_D:
  ltmp_1761_A = ltmp_1761_A_2E;
  if (((ltmp_1761_A == PLZO_INT64_C(0)) & (ltmp_1754_19 != ((plzo_uint64_t *)0)))) {
    goto ltmp_1774_D;
  } else {
    goto ltmp_1775_D;
  }

ltmp_1774_D:
  ltmp_1762_9 = *(&ltmp_1755_103C.field0.field0.field2.field0);
  *ltmp_1754_19 = ltmp_1762_9;
  ltmp_1763_9 = *(&ltmp_1755_103C.field0.field0.field2.field1);
  *(&ltmp_1754_19[PLZO_INT64_C(1)]) = ltmp_1763_9;
  ltmp_1764_9 = *(&ltmp_1755_103C.field0.field0.field2.field2);
  *(&ltmp_1754_19[PLZO_INT64_C(2)]) = ltmp_1764_9;
  ltmp_1765_9 = *(&ltmp_1755_103C.field0.field0.field2.field3);
  *(&ltmp_1754_19[PLZO_INT64_C(3)]) = ltmp_1765_9;
  ltmp_1766_9 = *(&ltmp_1755_103C.field0.field0.field2.field4);
  *(&ltmp_1754_19[PLZO_INT64_C(4)]) = ltmp_1766_9;
  ltmp_1767_9 = *(&ltmp_1755_103C.field0.field0.field2.field5);
  *(&ltmp_1754_19[PLZO_INT64_C(5)]) = ltmp_1767_9;
  ltmp_1768_9 = *(&ltmp_1755_103C.field0.field0.field2.field6);
  *(&ltmp_1754_19[PLZO_INT64_C(6)]) = ltmp_1768_9;
  ltmp_1769_9 = *(&ltmp_1755_103C.field0.field0.field2.field7);
  *(&ltmp_1754_19[PLZO_INT64_C(7)]) = ltmp_1769_9;
  goto ltmp_1775_D;

ltmp_1775_D:
  *(&ltmp_1755_103C.field0.field0.field0) = (&(ltmp_69_1021[PLZO_INT64_C(2)]));
  ltmp_5_1005(ltmp_1756_1023);
  return ltmp_1761_A;
}


static void ltmp_25_1010(struct l_struct_2E_Lzo1E *ltmp_1776_103D) {
  unsigned char **ltmp_1777_1025;
  unsigned char *ltmp_1778_13;
  plzo_uint64_t ltmp_1779_9;
  plzo_uint64_t ltmp_1779_9_2E;
  plzo_uint64_t ltmp_1780_9;
  unsigned char *ltmp_1781_13;
  plzo_uint64_t ltmp_1782_9;
  plzo_uint64_t ltmp_1782_9_2E;
  plzo_uint64_t ltmp_1783_9;
  unsigned char *ltmp_1784_13;
  plzo_uint64_t ltmp_1785_9;
  plzo_uint64_t ltmp_1785_9_2E;
  plzo_uint64_t ltmp_1786_9;
  plzo_uint64_t *ltmp_1787_19;
  plzo_uint64_t *ltmp_1788_19;
  unsigned char *ltmp_1789_13;
  plzo_uint64_t ltmp_1790_9;
  plzo_uint64_t ltmp_1790_9_2E;
  plzo_uint64_t ltmp_1791_9;
  unsigned char *ltmp_1792_13;
  plzo_uint64_t ltmp_1793_9;
  plzo_uint64_t ltmp_1793_9_2E;
  plzo_uint64_t ltmp_1794_9;
  unsigned char *ltmp_1795_13;
  plzo_uint64_t ltmp_1796_9;
  plzo_uint64_t ltmp_1796_9_2E;
  plzo_uint64_t ltmp_1797_9;
  unsigned char *ltmp_1798_13;
  plzo_uint64_t ltmp_1799_9;
  plzo_uint64_t ltmp_1799_9_2E;
  plzo_uint64_t ltmp_1800_9;
  plzo_uint64_t *ltmp_1801_19;
  struct l_struct_2E_E_3A__3A_A15 *ltmp_1802_103E;
  unsigned char *ltmp_1803_13;
  plzo_uint64_t ltmp_1804_9;
  plzo_uint64_t ltmp_1804_9_2E;
  plzo_uint64_t ltmp_1805_9;

  *(&ltmp_1776_103D->field0.field1.field2) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field1.field3) = PLZO_UINT64_C(8);
  *(&ltmp_1776_103D->field0.field1.field4) = PLZO_UINT64_C(1);
  *(&ltmp_1776_103D->field0.field1.field0) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field1.field1) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field1.field5) = ((unsigned char *)0);
  ltmp_1777_1025 = &ltmp_1776_103D->field0.field1.field6;
  *ltmp_1777_1025 = ((unsigned char *)0);
  *(&ltmp_1776_103D->field0.field0) = (&(ltmp_70_1021[PLZO_INT64_C(2)]));
  *(&ltmp_1776_103D->field0.field20.field0) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field20.field1) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field20.field2) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field20.field3) = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  *(&ltmp_1776_103D->field0.field20.field4) = ((struct l_struct_2E_A9 *)0);
  *(&ltmp_1776_103D->field0.field3) = PLZO_INT64_C(0);
  ltmp_1778_13 = (unsigned char *)(&ltmp_1776_103D->field0.field6);
  ltmp_1779_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1806_D;

  do {
ltmp_1806_D:
  ltmp_1779_9 = ltmp_1779_9_2E;
  *(&ltmp_1778_13[ltmp_1779_9]) = 0;
  ltmp_1780_9 = ltmp_1779_9 + PLZO_UINT64_C(1);
  if ((ltmp_1780_9 == PLZO_UINT64_C(192))) {
    goto ltmp_1807_D;
  } else {
    ltmp_1779_9_2E = ltmp_1780_9;
    goto ltmp_1806_D;
  }

  } while (1);
ltmp_1807_D:
  ltmp_1781_13 = (unsigned char *)(&ltmp_1776_103D->field0.field7);
  ltmp_1782_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1808_D;

  do {
ltmp_1808_D:
  ltmp_1782_9 = ltmp_1782_9_2E;
  *(&ltmp_1781_13[ltmp_1782_9]) = 0;
  ltmp_1783_9 = ltmp_1782_9 + PLZO_UINT64_C(1);
  if ((ltmp_1783_9 == PLZO_UINT64_C(128))) {
    goto ltmp_1809_D;
  } else {
    ltmp_1782_9_2E = ltmp_1783_9;
    goto ltmp_1808_D;
  }

  } while (1);
ltmp_1809_D:
  ltmp_1784_13 = (unsigned char *)(&ltmp_1776_103D->field0.field24);
  ltmp_1785_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1810_D;

  do {
ltmp_1810_D:
  ltmp_1785_9 = ltmp_1785_9_2E;
  *(&ltmp_1784_13[ltmp_1785_9]) = 0;
  ltmp_1786_9 = ltmp_1785_9 + PLZO_UINT64_C(1);
  if ((ltmp_1786_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1811_D;
  } else {
    ltmp_1785_9_2E = ltmp_1786_9;
    goto ltmp_1810_D;
  }

  } while (1);
ltmp_1811_D:
  *(&ltmp_1776_103D->field0.field24.field2.field0) = ltmp_49_1018;
  *(&ltmp_1776_103D->field0.field24.field2.field1) = ltmp_50_1019;
  *(&ltmp_1776_103D->field0.field24.field2.field2) = ltmp_51_101A;
  *(&ltmp_1776_103D->field0.field24.field2.field3) = ((signed char *)0);
  *(&ltmp_1776_103D->field0.field24.field0) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field24.field1) = PLZO_UINT64_C(1024);
  ltmp_1787_19 = &ltmp_1776_103D->field0.field6.field2;
  *ltmp_1787_19 = PLZO_UINT64_C(3);
  *(&ltmp_1776_103D->field0.field6.field4) = PLZO_UINT64_C(2);
  *(&ltmp_1776_103D->field0.field6.field5) = PLZO_UINT64_C(18446744073709551615);
  ltmp_1788_19 = &ltmp_1776_103D->field0.field6.field3;
  *ltmp_1788_19 = PLZO_UINT64_C(18446744073709551615);
  *ltmp_1777_1025 = ((unsigned char *)0);
  *(&ltmp_1776_103D->field0.field10) = ((unsigned char *)0);
  *(&ltmp_1776_103D->field0.field8) = ((unsigned char *)0);
  *(&ltmp_1776_103D->field0.field11) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field9) = ((unsigned char *)0);
  *(&ltmp_1776_103D->field0.field12) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field13) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field14) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field15) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field16) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field17) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field0.field18) = PLZO_UINT64_C(0);
  ltmp_1789_13 = (unsigned char *)(&ltmp_1776_103D->field0.field2);
  ltmp_1790_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1812_D;

  do {
ltmp_1812_D:
  ltmp_1790_9 = ltmp_1790_9_2E;
  *(&ltmp_1789_13[ltmp_1790_9]) = 0;
  ltmp_1791_9 = ltmp_1790_9 + PLZO_UINT64_C(1);
  if ((ltmp_1791_9 == PLZO_UINT64_C(128))) {
    goto ltmp_1813_D;
  } else {
    ltmp_1790_9_2E = ltmp_1791_9;
    goto ltmp_1812_D;
  }

  } while (1);
ltmp_1813_D:
  *(&ltmp_1776_103D->field0.field2.field0) = PLZO_UINT64_C(18446744073709551615);
  *(&ltmp_1776_103D->field0.field2.field2) = PLZO_UINT64_C(18446744073709551615);
  *(&ltmp_1776_103D->field0.field2.field4) = PLZO_UINT64_C(18446744073709551615);
  ltmp_1792_13 = (unsigned char *)(&ltmp_1776_103D->field0.field22);
  ltmp_1793_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1814_D;

  do {
ltmp_1814_D:
  ltmp_1793_9 = ltmp_1793_9_2E;
  *(&ltmp_1792_13[ltmp_1793_9]) = 0;
  ltmp_1794_9 = ltmp_1793_9 + PLZO_UINT64_C(1);
  if ((ltmp_1794_9 == PLZO_UINT64_C(32))) {
    goto ltmp_1815_D;
  } else {
    ltmp_1793_9_2E = ltmp_1794_9;
    goto ltmp_1814_D;
  }

  } while (1);
ltmp_1815_D:
  ltmp_1795_13 = (unsigned char *)(&ltmp_1776_103D->field0.field23);
  ltmp_1796_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1816_D;

  do {
ltmp_1816_D:
  ltmp_1796_9 = ltmp_1796_9_2E;
  *(&ltmp_1795_13[ltmp_1796_9]) = 0;
  ltmp_1797_9 = ltmp_1796_9 + PLZO_UINT64_C(1);
  if ((ltmp_1797_9 == PLZO_UINT64_C(32))) {
    goto ltmp_1817_D;
  } else {
    ltmp_1796_9_2E = ltmp_1797_9;
    goto ltmp_1816_D;
  }

  } while (1);
ltmp_1817_D:
  *(&ltmp_1776_103D->field0.field22.field1) = PLZO_UINT64_C(512);
  *(&ltmp_1776_103D->field0.field23.field1) = PLZO_UINT64_C(4096);
  ltmp_1798_13 = (unsigned char *)(&ltmp_1776_103D->field0.field19);
  ltmp_1799_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1818_D;

  do {
ltmp_1818_D:
  ltmp_1799_9 = ltmp_1799_9_2E;
  *(&ltmp_1798_13[ltmp_1799_9]) = 0;
  ltmp_1800_9 = ltmp_1799_9 + PLZO_UINT64_C(1);
  if ((ltmp_1800_9 == PLZO_UINT64_C(64))) {
    goto ltmp_1819_D;
  } else {
    ltmp_1799_9_2E = ltmp_1800_9;
    goto ltmp_1818_D;
  }

  } while (1);
ltmp_1819_D:
  ltmp_1801_19 = &ltmp_1776_103D->field0.field21;
  *ltmp_1801_19 = PLZO_UINT64_C(1);
  ltmp_1802_103E = &ltmp_1776_103D->field0.field25;
  *(&ltmp_1776_103D->field0.field26) = ltmp_1802_103E;
  ltmp_1803_13 = (unsigned char *)ltmp_1802_103E;
  ltmp_1804_9_2E = PLZO_UINT64_C(0);
  goto ltmp_1820_D;

  do {
ltmp_1820_D:
  ltmp_1804_9 = ltmp_1804_9_2E;
  *(&ltmp_1803_13[ltmp_1804_9]) = 0;
  ltmp_1805_9 = ltmp_1804_9 + PLZO_UINT64_C(1);
  if ((ltmp_1805_9 == PLZO_UINT64_C(1))) {
    goto ltmp_1821_D;
  } else {
    ltmp_1804_9_2E = ltmp_1805_9;
    goto ltmp_1820_D;
  }

  } while (1);
ltmp_1821_D:
  *(&ltmp_1776_103D->field0.field0) = (&(ltmp_69_1021[PLZO_INT64_C(2)]));
  *ltmp_1801_19 = PLZO_UINT64_C(5);
  *(&ltmp_1776_103D->field1) = PLZO_UINT64_C(0);
  *(&ltmp_1776_103D->field2) = PLZO_UINT64_C(64);
  *(&ltmp_1776_103D->field3) = PLZO_UINT64_C(36);
  *(&ltmp_1776_103D->field4) = PLZO_UINT64_C(16);
  *(&ltmp_1776_103D->field5) = PLZO_UINT64_C(32);
  *(&ltmp_1776_103D->field6) = PLZO_UINT64_C(1024);
  *(&ltmp_1776_103D->field7) = PLZO_UINT64_C(2048);
  *(&ltmp_1776_103D->field8) = PLZO_UINT64_C(16384);
  *(&ltmp_1776_103D->field9) = PLZO_UINT64_C(49151);
  *(&ltmp_1776_103D->field10) = PLZO_UINT64_C(1064960);
  *(&ltmp_1776_103D->field11) = PLZO_UINT64_C(3072);
  *(&ltmp_1776_103D->field12) = PLZO_UINT64_C(2);
  *(&ltmp_1776_103D->field13) = PLZO_UINT64_C(3);
  *(&ltmp_1776_103D->field14) = PLZO_UINT64_C(3);
  *(&ltmp_1776_103D->field15) = PLZO_UINT64_C(3);
  *(&ltmp_1776_103D->field16) = PLZO_UINT64_C(4);
  *(&ltmp_1776_103D->field17) = PLZO_UINT64_C(2);
  *(&ltmp_1776_103D->field18) = PLZO_UINT64_C(8);
  *(&ltmp_1776_103D->field19) = PLZO_UINT64_C(29);
  *(&ltmp_1776_103D->field20) = PLZO_UINT64_C(10);
  *(&ltmp_1776_103D->field21) = PLZO_UINT64_C(18);
  *ltmp_1787_19 = PLZO_UINT64_C(4);
  *ltmp_1788_19 = PLZO_UINT64_C(1064960);
  return;
}


static plzo_int64_t plzo_lzo1y_99_compress_internal(unsigned char *ltmp_1822_13, plzo_uint64_t ltmp_1823_9, unsigned char *ltmp_1824_13, plzo_uint64_t *ltmp_1825_19, struct plzo_callback_t *ltmp_1826_1026, plzo_int64_t ltmp_1827_A, struct plzo_compress_config_t *ltmp_1828_1027, plzo_uint64_t *ltmp_1829_19) {
  struct l_struct_2E_Lzo1yE ltmp_1830_103F;
  plzo_int64_t  (***ltmp_1831_102C) ();
  struct l_struct_2E_E *ltmp_1832_1023;
  plzo_int64_t ltmp_1833_A;
  plzo_int64_t ltmp_1834_A;
  plzo_int64_t ltmp_1835_A;
  plzo_int64_t ltmp_1835_A_2E;
  plzo_int64_t ltmp_1836_A;
  plzo_int64_t ltmp_1837_A;
  plzo_int64_t ltmp_1837_A_2E;
  plzo_uint64_t ltmp_1838_9;
  plzo_uint64_t ltmp_1839_9;
  plzo_uint64_t ltmp_1840_9;
  plzo_uint64_t ltmp_1841_9;
  plzo_uint64_t ltmp_1842_9;
  plzo_uint64_t ltmp_1843_9;
  plzo_uint64_t ltmp_1844_9;
  plzo_uint64_t ltmp_1845_9;

  ltmp_25_1010((&ltmp_1830_103F.field0.field0));
  ltmp_1831_102C = &ltmp_1830_103F.field0.field0.field0.field0;
  *(&ltmp_1830_103F.field0.field0.field1) = PLZO_UINT64_C(0);
  *(&ltmp_1830_103F.field0.field0.field2) = PLZO_UINT64_C(64);
  *(&ltmp_1830_103F.field0.field0.field3) = PLZO_UINT64_C(32);
  *(&ltmp_1830_103F.field0.field0.field4) = PLZO_UINT64_C(16);
  *(&ltmp_1830_103F.field0.field0.field5) = PLZO_UINT64_C(0);
  *(&ltmp_1830_103F.field0.field0.field6) = PLZO_UINT64_C(1024);
  *(&ltmp_1830_103F.field0.field0.field8) = PLZO_UINT64_C(16384);
  *(&ltmp_1830_103F.field0.field0.field9) = PLZO_UINT64_C(49151);
  *(&ltmp_1830_103F.field0.field0.field10) = PLZO_UINT64_C(0);
  *(&ltmp_1830_103F.field0.field0.field12) = PLZO_UINT64_C(2);
  *(&ltmp_1830_103F.field0.field0.field13) = PLZO_UINT64_C(3);
  *(&ltmp_1830_103F.field0.field0.field14) = PLZO_UINT64_C(3);
  *(&ltmp_1830_103F.field0.field0.field15) = PLZO_UINT64_C(3);
  *(&ltmp_1830_103F.field0.field0.field16) = PLZO_UINT64_C(0);
  *(&ltmp_1830_103F.field0.field0.field17) = PLZO_UINT64_C(2);
  *(&ltmp_1830_103F.field0.field0.field19) = PLZO_UINT64_C(33);
  *(&ltmp_1830_103F.field0.field0.field20) = PLZO_UINT64_C(9);
  *(&ltmp_1830_103F.field0.field0.field21) = PLZO_UINT64_C(0);
  *(&ltmp_1830_103F.field0.field0.field0.field6.field3) = PLZO_UINT64_C(49151);
  *(&ltmp_1830_103F.field0.field0.field0.field0) = (&(ltmp_68_1021[PLZO_INT64_C(2)]));
  *(&ltmp_1830_103F.field0.field0.field18) = PLZO_UINT64_C(14);
  *(&ltmp_1830_103F.field0.field0.field7) = PLZO_UINT64_C(1024);
  *(&ltmp_1830_103F.field0.field0.field11) = PLZO_UINT64_C(2048);
  *(&ltmp_1830_103F.field0.field0.field0.field6.field0) = PLZO_INT64_C(1);
  ltmp_1832_1023 = &ltmp_1830_103F.field0.field0.field0;
  ltmp_1833_A = ltmp_2_1002(ltmp_1832_1023, ltmp_1827_A, ltmp_1826_1026);
  if (((ltmp_1833_A == PLZO_INT64_C(0)) & (ltmp_1828_1027 != ((struct plzo_compress_config_t *)0)))) {
    goto ltmp_1846_D;
  } else {
    ltmp_1835_A_2E = ltmp_1833_A;
    goto ltmp_1847_D;
  }

ltmp_1846_D:
  ltmp_1834_A = ltmp_3_1003(ltmp_1832_1023, ltmp_1828_1027);
  ltmp_1835_A_2E = ltmp_1834_A;
  goto ltmp_1847_D;

ltmp_1847_D:
  ltmp_1835_A = ltmp_1835_A_2E;
  if ((ltmp_1835_A == PLZO_INT64_C(0))) {
    goto ltmp_1848_D;
  } else {
    ltmp_1837_A_2E = ltmp_1835_A;
    goto ltmp_1849_D;
  }

ltmp_1848_D:
  ltmp_1836_A = ltmp_4_1004(ltmp_1832_1023, ltmp_1822_13, ltmp_1823_9, ltmp_1824_13, ltmp_1825_19);
  ltmp_1837_A_2E = ltmp_1836_A;
  goto ltmp_1849_D;

ltmp_1849_D:
  ltmp_1837_A = ltmp_1837_A_2E;
  if (((ltmp_1837_A == PLZO_INT64_C(0)) & (ltmp_1829_19 != ((plzo_uint64_t *)0)))) {
    goto ltmp_1850_D;
  } else {
    goto ltmp_1851_D;
  }

ltmp_1850_D:
  ltmp_1838_9 = *(&ltmp_1830_103F.field0.field0.field0.field2.field0);
  *ltmp_1829_19 = ltmp_1838_9;
  ltmp_1839_9 = *(&ltmp_1830_103F.field0.field0.field0.field2.field1);
  *(&ltmp_1829_19[PLZO_INT64_C(1)]) = ltmp_1839_9;
  ltmp_1840_9 = *(&ltmp_1830_103F.field0.field0.field0.field2.field2);
  *(&ltmp_1829_19[PLZO_INT64_C(2)]) = ltmp_1840_9;
  ltmp_1841_9 = *(&ltmp_1830_103F.field0.field0.field0.field2.field3);
  *(&ltmp_1829_19[PLZO_INT64_C(3)]) = ltmp_1841_9;
  ltmp_1842_9 = *(&ltmp_1830_103F.field0.field0.field0.field2.field4);
  *(&ltmp_1829_19[PLZO_INT64_C(4)]) = ltmp_1842_9;
  ltmp_1843_9 = *(&ltmp_1830_103F.field0.field0.field0.field2.field5);
  *(&ltmp_1829_19[PLZO_INT64_C(5)]) = ltmp_1843_9;
  ltmp_1844_9 = *(&ltmp_1830_103F.field0.field0.field0.field2.field6);
  *(&ltmp_1829_19[PLZO_INT64_C(6)]) = ltmp_1844_9;
  ltmp_1845_9 = *(&ltmp_1830_103F.field0.field0.field0.field2.field7);
  *(&ltmp_1829_19[PLZO_INT64_C(7)]) = ltmp_1845_9;
  goto ltmp_1851_D;

ltmp_1851_D:
  *ltmp_1831_102C = (&(ltmp_69_1021[PLZO_INT64_C(2)]));
  ltmp_5_1005(ltmp_1832_1023);
  return ltmp_1837_A;
}


static signed char *ltmp_26_1011(struct l_struct_2E_E *ltmp_1852_1023) {

  return ((signed char *)0);
}


static void ltmp_27_1010(struct l_struct_2E_Lzo1E *ltmp_1853_103D) {
  unsigned char **ltmp_1854_1025;
  unsigned char *ltmp_1855_13;
  plzo_uint64_t ltmp_1856_9;
  unsigned char *ltmp_1857_13;
  unsigned char *ltmp_1858_13;

  ltmp_1854_1025 = &ltmp_1853_103D->field0.field1.field6;
  ltmp_1855_13 = *ltmp_1854_1025;
  *ltmp_1854_1025 = (&ltmp_1855_13[PLZO_INT64_C(1)]);
  ltmp_1856_9 = *(&ltmp_1853_103D->field4);
  *ltmp_1855_13 = (((unsigned char )(((unsigned char )ltmp_1856_9) + 1)));
  ltmp_1857_13 = *ltmp_1854_1025;
  *ltmp_1854_1025 = (&ltmp_1857_13[PLZO_INT64_C(1)]);
  *ltmp_1857_13 = 0;
  ltmp_1858_13 = *ltmp_1854_1025;
  *ltmp_1854_1025 = (&ltmp_1858_13[PLZO_INT64_C(1)]);
  *ltmp_1858_13 = 0;
  return;
}


static plzo_int64_t ltmp_28_1012(struct l_struct_2E_Lzo1E *ltmp_1859_103D) {

  return PLZO_INT64_C(0);
}


static void ltmp_29_1013(struct l_struct_2E_Lzo1xE *ltmp_1860_103C) {

  return;
}


static void ltmp_30_1014(struct l_struct_2E_Lzo1E *ltmp_1861_103D, struct l_struct_2E_E_3A__3A_A4 *ltmp_1862_103B, plzo_uint64_t ltmp_1863_9, plzo_uint64_t ltmp_1864_9, plzo_uint64_t ltmp_1865_9) {
  plzo_uint64_t ltmp_1866_9;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1867_1031;
  plzo_uint64_t ltmp_1868_9;
  plzo_uint64_t ltmp_1869_9;
  struct l_struct_2E_A7 *ltmp_1870_1038;
  plzo_uint64_t ltmp_1871_9;
  plzo_uint64_t ltmp_1872_9;
  plzo_uint64_t ltmp_1873_9;
  plzo_uint64_t ltmp_1874_9;
  plzo_uint64_t ltmp_1875_9;
  plzo_uint64_t ltmp_1876_9;
  plzo_uint64_t ltmp_1877_9;
  plzo_uint64_t ltmp_1878_9;
  plzo_uint64_t ltmp_1879_9;
  plzo_uint64_t ltmp_1880_9;
  plzo_uint64_t ltmp_1881_9;
  plzo_uint64_t ltmp_1882_9;
  plzo_uint64_t ltmp_1883_9;
  plzo_uint64_t ltmp_1884_9;
  plzo_uint64_t ltmp_1885_9;
  plzo_uint64_t ltmp_1886_9;
  plzo_uint64_t ltmp_1887_9;
  plzo_uint64_t ltmp_1888_9;
  plzo_uint64_t ltmp_1889_9;
  plzo_uint64_t ltmp_1890_9;
  plzo_uint64_t ltmp_1890_9_2E;
  plzo_uint64_t ltmp_1891_9;
  plzo_uint64_t ltmp_1891_9_2E;
  plzo_uint64_t ltmp_1892_9;
  plzo_uint64_t ltmp_1892_9_2E;
  plzo_uint64_t ltmp_1893_9;
  plzo_uint64_t ltmp_1893_9_2E;
  plzo_uint64_t ltmp_1894_9;
  plzo_uint64_t ltmp_1895_9;
  plzo_uint64_t ltmp_1895_9_2E;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1896_1031;
  plzo_uint64_t ltmp_1897_9;
  plzo_uint64_t ltmp_1898_9;

  if ((ltmp_1863_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1899_D;
  } else {
    goto ltmp_1900_D;
  }

ltmp_1899_D:
  ltmp_1866_9 = ltmp_1865_9 + PLZO_UINT64_C(1);
  ltmp_1867_1031 = *(&ltmp_1861_103D->field0.field20.field3);
  ltmp_1868_9 = *(&ltmp_1861_103D->field0.field20.field1);
  ltmp_1869_9 = *(&ltmp_1867_1031[((ltmp_1868_9*(ltmp_1864_9 + PLZO_UINT64_C(1))) + ((((ltmp_1868_9 > ltmp_1866_9)) ? (ltmp_1866_9) : ((ltmp_1868_9 + PLZO_UINT64_C(18446744073709551615))))))].field0);
  *(&ltmp_1862_103B->field0) = (ltmp_1869_9 + ((((ltmp_1865_9 == PLZO_UINT64_C(3))) ? (PLZO_UINT64_C(16)) : (PLZO_UINT64_C(8)))));
  return;
ltmp_1900_D:
  ltmp_1870_1038 = *(&ltmp_1861_103D->field0.field19.field0);
  ltmp_1871_9 = *(&ltmp_1870_1038[ltmp_1864_9].field0);
  ltmp_1872_9 = ltmp_1871_9 - ltmp_1863_9;
  ltmp_1873_9 = *(&ltmp_1870_1038[ltmp_1864_9].field1[((((ltmp_1872_9 < PLZO_UINT64_C(8))) ? (ltmp_1872_9) : (PLZO_UINT64_C(7))))]);
  if ((ltmp_1863_9 > PLZO_UINT64_C(1))) {
    goto ltmp_1901_D;
  } else {
    goto ltmp_1902_D;
  }

ltmp_1901_D:
  if ((ltmp_1863_9 == PLZO_UINT64_C(2))) {
    goto ltmp_1903_D;
  } else {
    goto ltmp_1904_D;
  }

ltmp_1903_D:
  ltmp_1874_9 = *(&ltmp_1861_103D->field6);
  if ((((ltmp_1865_9 == PLZO_UINT64_C(0)) | (ltmp_1865_9 > PLZO_UINT64_C(3))) | (ltmp_1874_9 < ltmp_1873_9))) {
    goto ltmp_1902_D;
  } else {
    goto ltmp_1904_D;
  }

ltmp_1904_D:
  ltmp_1875_9 = *(&ltmp_1861_103D->field16);
  if ((ltmp_1875_9 > ltmp_1863_9)) {
    goto ltmp_1905_D;
  } else {
    goto ltmp_1906_D;
  }

ltmp_1905_D:
  ltmp_1876_9 = *(&ltmp_1861_103D->field9);
  if ((ltmp_1876_9 < ltmp_1873_9)) {
    goto ltmp_1902_D;
  } else {
    goto ltmp_1906_D;
  }

ltmp_1906_D:
  if ((ltmp_1863_9 == PLZO_UINT64_C(2))) {
    ltmp_1890_9_2E = PLZO_UINT64_C(16);
    goto ltmp_1907_D;
  } else {
    goto ltmp_1908_D;
  }

ltmp_1908_D:
  ltmp_1877_9 = *(&ltmp_1861_103D->field18);
  if ((ltmp_1877_9 < ltmp_1863_9)) {
    goto ltmp_1909_D;
  } else {
    goto ltmp_1910_D;
  }

ltmp_1910_D:
  ltmp_1878_9 = *(&ltmp_1861_103D->field7);
  if ((ltmp_1878_9 < ltmp_1873_9)) {
    goto ltmp_1909_D;
  } else {
    ltmp_1890_9_2E = PLZO_UINT64_C(16);
    goto ltmp_1907_D;
  }

ltmp_1909_D:
  ltmp_1879_9 = *(&ltmp_1861_103D->field13);
  if ((ltmp_1879_9 == ltmp_1863_9)) {
    goto ltmp_1911_D;
  } else {
    goto ltmp_1912_D;
  }

ltmp_1911_D:
  ltmp_1880_9 = *(&ltmp_1861_103D->field11);
  if (((ltmp_1880_9 >= ltmp_1873_9) & (ltmp_1865_9 > PLZO_UINT64_C(3)))) {
    ltmp_1890_9_2E = PLZO_UINT64_C(16);
    goto ltmp_1907_D;
  } else {
    goto ltmp_1912_D;
  }

ltmp_1912_D:
  ltmp_1881_9 = *(&ltmp_1861_103D->field8);
  if ((ltmp_1881_9 < ltmp_1873_9)) {
    goto ltmp_1913_D;
  } else {
    goto ltmp_1914_D;
  }

ltmp_1914_D:
  ltmp_1882_9 = *(&ltmp_1861_103D->field19);
  if ((ltmp_1882_9 < ltmp_1863_9)) {
    goto ltmp_1915_D;
  } else {
    ltmp_1890_9_2E = PLZO_UINT64_C(24);
    goto ltmp_1907_D;
  }

ltmp_1915_D:
  ltmp_1883_9 = ltmp_1863_9 - ltmp_1882_9;
  if ((ltmp_1883_9 > PLZO_UINT64_C(255))) {
    ltmp_1891_9_2E = ltmp_1883_9;
    ltmp_1892_9_2E = PLZO_UINT64_C(32);
    ltmp_1893_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1916_D;
  } else {
    ltmp_1895_9_2E = PLZO_UINT64_C(32);
    goto ltmp_1917_D;
  }

ltmp_1913_D:
  ltmp_1884_9 = *(&ltmp_1861_103D->field20);
  if ((ltmp_1884_9 < ltmp_1863_9)) {
    goto ltmp_1918_D;
  } else {
    goto ltmp_1919_D;
  }

ltmp_1919_D:
  ltmp_1885_9 = *(&ltmp_1861_103D->field9);
  if ((ltmp_1885_9 < ltmp_1873_9)) {
    goto ltmp_1918_D;
  } else {
    goto ltmp_1920_D;
  }

ltmp_1918_D:
  ltmp_1886_9 = *(&ltmp_1861_103D->field10);
  if ((ltmp_1886_9 == PLZO_UINT64_C(0))) {
    goto ltmp_1920_D;
  } else {
    goto ltmp_1921_D;
  }

ltmp_1920_D:
  if ((ltmp_1884_9 < ltmp_1863_9)) {
    goto ltmp_1922_D;
  } else {
    ltmp_1890_9_2E = PLZO_UINT64_C(24);
    goto ltmp_1907_D;
  }

ltmp_1922_D:
  ltmp_1887_9 = ltmp_1863_9 - ltmp_1884_9;
  if ((ltmp_1887_9 > PLZO_UINT64_C(255))) {
    ltmp_1891_9_2E = ltmp_1887_9;
    ltmp_1892_9_2E = PLZO_UINT64_C(32);
    ltmp_1893_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1916_D;
  } else {
    ltmp_1895_9_2E = PLZO_UINT64_C(32);
    goto ltmp_1917_D;
  }

ltmp_1921_D:
  ltmp_1888_9 = *(&ltmp_1861_103D->field21);
  if ((ltmp_1888_9 < ltmp_1863_9)) {
    goto ltmp_1923_D;
  } else {
    ltmp_1890_9_2E = PLZO_UINT64_C(32);
    goto ltmp_1907_D;
  }

ltmp_1923_D:
  ltmp_1889_9 = ltmp_1863_9 - ltmp_1888_9;
  if ((ltmp_1889_9 > PLZO_UINT64_C(255))) {
    ltmp_1891_9_2E = ltmp_1889_9;
    ltmp_1892_9_2E = PLZO_UINT64_C(40);
    ltmp_1893_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1916_D;
  } else {
    ltmp_1895_9_2E = PLZO_UINT64_C(40);
    goto ltmp_1917_D;
  }

ltmp_1907_D:
  ltmp_1890_9 = ltmp_1890_9_2E;
  if ((ltmp_1863_9 > PLZO_UINT64_C(255))) {
    ltmp_1891_9_2E = ltmp_1863_9;
    ltmp_1892_9_2E = ltmp_1890_9;
    ltmp_1893_9_2E = PLZO_UINT64_C(0);
    goto ltmp_1916_D;
  } else {
    ltmp_1895_9_2E = ltmp_1890_9;
    goto ltmp_1917_D;
  }

  do {
ltmp_1916_D:
  ltmp_1891_9 = ltmp_1891_9_2E;
  ltmp_1892_9 = ltmp_1892_9_2E;
  ltmp_1893_9 = ltmp_1893_9_2E;
  if (((((ltmp_1893_9*PLZO_UINT64_C(18446744073709551361)) + ltmp_1891_9) + PLZO_UINT64_C(18446744073709551361)) > PLZO_UINT64_C(255))) {
    ltmp_1891_9_2E = ltmp_1891_9;
    ltmp_1892_9_2E = ltmp_1892_9;
    ltmp_1893_9_2E = (ltmp_1893_9 + PLZO_UINT64_C(1));
    goto ltmp_1916_D;
  } else {
    goto ltmp_1924_D;
  }

  } while (1);
ltmp_1924_D:
  ltmp_1894_9 = ((ltmp_1893_9 << 3) + ltmp_1892_9) + PLZO_UINT64_C(8);
  ltmp_1895_9_2E = ltmp_1894_9;
  goto ltmp_1917_D;

ltmp_1917_D:
  ltmp_1895_9 = ltmp_1895_9_2E;
  ltmp_1896_1031 = *(&ltmp_1861_103D->field0.field20.field3);
  ltmp_1897_9 = *(&ltmp_1861_103D->field0.field20.field1);
  ltmp_1898_9 = *(&ltmp_1896_1031[(ltmp_1897_9*(ltmp_1864_9 + ltmp_1863_9))].field0);
  *(&ltmp_1862_103B->field0) = (ltmp_1898_9 + ltmp_1895_9);
  return;
ltmp_1902_D:
  *(&ltmp_1862_103B->field0) = PLZO_UINT64_C(18446744073709551615);
  return;
}


static plzo_uint64_t ltmp_31_1015(struct l_struct_2E_Lzo1E *ltmp_1925_103D) {

  return PLZO_UINT64_C(0);
}


static plzo_int64_t ltmp_32_1016(struct l_struct_2E_E *ltmp_1926_1023, struct l_struct_2E_A7 *ltmp_1927_1038) {
  plzo_uint64_t ltmp_1928_9;

  ltmp_1928_9 = *(&ltmp_1927_1038->field0);
  return ((plzo_int64_t )(ltmp_1928_9 == PLZO_UINT64_C(0)));
}


static plzo_int64_t ltmp_33_1016(struct l_struct_2E_E *ltmp_1929_1023, struct l_struct_2E_A7 *ltmp_1930_1038) {
  plzo_uint64_t ltmp_1931_9;

  ltmp_1931_9 = *(&ltmp_1930_1038->field0);
  return ((plzo_int64_t )(ltmp_1931_9 < PLZO_UINT64_C(3)));
}


static void ltmp_34_100F(struct l_struct_2E_Lzo1E *ltmp_1932_103D, plzo_uint64_t ltmp_1933_9, plzo_uint64_t ltmp_1934_9) {
  unsigned char **ltmp_1935_1025;
  unsigned char *ltmp_1936_13;
  plzo_uint64_t ltmp_1937_9;
  plzo_uint64_t ltmp_1938_9;
  unsigned char *ltmp_1939_13;
  plzo_uint64_t ltmp_1940_9;
  plzo_uint64_t ltmp_1941_9;
  plzo_uint64_t ltmp_1942_9;
  unsigned char *ltmp_1943_13;
  plzo_uint64_t ltmp_1944_9;
  plzo_uint64_t ltmp_1945_9;
  plzo_uint64_t ltmp_1946_9;
  plzo_uint64_t ltmp_1947_9;
  plzo_uint64_t ltmp_1948_9;
  plzo_uint64_t ltmp_1949_9;
  unsigned char *ltmp_1950_13;
  plzo_uint64_t ltmp_1951_9;
  plzo_uint64_t ltmp_1952_9;
  plzo_uint64_t ltmp_1953_9;
  unsigned char *ltmp_1954_13;
  plzo_uint64_t ltmp_1955_9;
  plzo_uint64_t ltmp_1956_9;
  unsigned char *ltmp_1957_13;
  plzo_uint64_t ltmp_1958_9;
  plzo_uint64_t ltmp_1959_9;
  plzo_uint64_t ltmp_1959_9_2E;
  plzo_uint64_t ltmp_1960_9;
  plzo_uint64_t ltmp_1961_9;
  unsigned char *ltmp_1962_13;
  unsigned char *ltmp_1963_13;
  unsigned char *ltmp_1963_13_2E;
  plzo_uint64_t ltmp_1964_9;
  plzo_uint64_t ltmp_1964_9_2E;
  unsigned char *ltmp_1965_13;
  unsigned char *ltmp_1966_13;
  unsigned char *ltmp_1966_13_2E;
  unsigned char *ltmp_1967_13;
  plzo_uint64_t ltmp_1968_9;
  plzo_uint64_t ltmp_1969_9;
  plzo_uint64_t ltmp_1970_9;
  plzo_uint64_t ltmp_1971_9;
  unsigned char *ltmp_1972_13;
  plzo_uint64_t ltmp_1973_9;
  unsigned char *ltmp_1974_13;
  plzo_uint64_t ltmp_1975_9;
  unsigned char ltmp_1976_3;
  unsigned char *ltmp_1977_13;
  unsigned char ltmp_1978_3;
  plzo_uint64_t ltmp_1979_9;
  unsigned char ltmp_1980_3;
  plzo_uint64_t ltmp_1981_9;
  plzo_uint64_t ltmp_1982_9;
  plzo_uint64_t ltmp_1982_9_2E;
  plzo_uint64_t ltmp_1983_9;
  unsigned char *ltmp_1984_13;
  unsigned char *ltmp_1985_13;
  unsigned char *ltmp_1985_13_2E;
  plzo_uint64_t ltmp_1986_9;
  plzo_uint64_t ltmp_1986_9_2E;
  unsigned char *ltmp_1987_13;
  unsigned char *ltmp_1988_13;
  unsigned char *ltmp_1988_13_2E;
  unsigned char *ltmp_1989_13;
  unsigned char *ltmp_1990_13;
  unsigned char *ltmp_1990_13_2E;

  ltmp_0_1000((&ltmp_1932_103D->field0), ltmp_1933_9, ltmp_1934_9);
  ltmp_1935_1025 = &ltmp_1932_103D->field0.field1.field6;
  ltmp_1936_13 = *ltmp_1935_1025;
  if ((ltmp_1933_9 == PLZO_UINT64_C(2))) {
    goto ltmp_1991_D;
  } else {
    goto ltmp_1992_D;
  }

ltmp_1991_D:
  ltmp_1937_9 = ltmp_1934_9 + PLZO_UINT64_C(18446744073709551615);
  ltmp_1938_9 = *(&ltmp_1932_103D->field1);
  *ltmp_1936_13 = (((unsigned char )(((unsigned char )ltmp_1938_9) | ((unsigned char )(ltmp_1937_9 >> 6)))));
  ltmp_1939_13 = &ltmp_1936_13[PLZO_INT64_C(2)];
  *(&ltmp_1936_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1937_9) << 2)));
  ltmp_1990_13_2E = ltmp_1939_13;
  goto ltmp_1993_D;

ltmp_1992_D:
  ltmp_1940_9 = *(&ltmp_1932_103D->field18);
  if ((ltmp_1940_9 < ltmp_1933_9)) {
    goto ltmp_1994_D;
  } else {
    goto ltmp_1995_D;
  }

ltmp_1995_D:
  ltmp_1941_9 = *(&ltmp_1932_103D->field7);
  if ((ltmp_1941_9 < ltmp_1934_9)) {
    goto ltmp_1994_D;
  } else {
    goto ltmp_1996_D;
  }

ltmp_1996_D:
  ltmp_1942_9 = ltmp_1934_9 + PLZO_UINT64_C(18446744073709551615);
  *ltmp_1936_13 = (((unsigned char )(((unsigned char )(ltmp_1942_9 >> 6)) | (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1933_9) << 5))) + 224))))));
  ltmp_1943_13 = &ltmp_1936_13[PLZO_INT64_C(2)];
  *(&ltmp_1936_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1942_9) << 2)));
  ltmp_1990_13_2E = ltmp_1943_13;
  goto ltmp_1993_D;

ltmp_1994_D:
  ltmp_1944_9 = *(&ltmp_1932_103D->field13);
  if ((ltmp_1944_9 == ltmp_1933_9)) {
    goto ltmp_1997_D;
  } else {
    goto ltmp_1998_D;
  }

ltmp_1997_D:
  ltmp_1945_9 = *(&ltmp_1932_103D->field11);
  if ((ltmp_1945_9 < ltmp_1934_9)) {
    goto ltmp_1998_D;
  } else {
    goto ltmp_1999_D;
  }

ltmp_1999_D:
  ltmp_1946_9 = *(&ltmp_1932_103D->field0.field17);
  if ((ltmp_1946_9 > PLZO_UINT64_C(3))) {
    goto ltmp_2000_D;
  } else {
    goto ltmp_1998_D;
  }

ltmp_2000_D:
  ltmp_1947_9 = *(&ltmp_1932_103D->field7);
  ltmp_1948_9 = (ltmp_1934_9 - ltmp_1947_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_1949_9 = *(&ltmp_1932_103D->field1);
  *ltmp_1936_13 = (((unsigned char )(((unsigned char )(ltmp_1948_9 >> 6)) | ((unsigned char )ltmp_1949_9))));
  ltmp_1950_13 = &ltmp_1936_13[PLZO_INT64_C(2)];
  *(&ltmp_1936_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1948_9) << 2)));
  ltmp_1990_13_2E = ltmp_1950_13;
  goto ltmp_1993_D;

ltmp_1998_D:
  ltmp_1951_9 = *(&ltmp_1932_103D->field8);
  if ((ltmp_1951_9 < ltmp_1934_9)) {
    goto ltmp_2001_D;
  } else {
    goto ltmp_2002_D;
  }

ltmp_2002_D:
  ltmp_1952_9 = ltmp_1934_9 + PLZO_UINT64_C(18446744073709551615);
  ltmp_1953_9 = *(&ltmp_1932_103D->field19);
  if ((ltmp_1953_9 < ltmp_1933_9)) {
    goto ltmp_2003_D;
  } else {
    goto ltmp_2004_D;
  }

ltmp_2004_D:
  ltmp_1954_13 = &ltmp_1936_13[PLZO_INT64_C(1)];
  ltmp_1955_9 = *(&ltmp_1932_103D->field3);
  *ltmp_1936_13 = (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1933_9) + 254))) + ((unsigned char )ltmp_1955_9))));
  ltmp_1966_13_2E = ltmp_1954_13;
  goto ltmp_2005_D;

ltmp_2003_D:
  ltmp_1956_9 = ltmp_1933_9 - ltmp_1953_9;
  ltmp_1957_13 = &ltmp_1936_13[PLZO_INT64_C(1)];
  ltmp_1958_9 = *(&ltmp_1932_103D->field3);
  *ltmp_1936_13 = ((unsigned char )ltmp_1958_9);
  if ((ltmp_1956_9 > PLZO_UINT64_C(255))) {
    ltmp_1959_9_2E = PLZO_UINT64_C(0);
    goto ltmp_2006_D;
  } else {
    ltmp_1963_13_2E = ltmp_1957_13;
    ltmp_1964_9_2E = ltmp_1956_9;
    goto ltmp_2007_D;
  }

  do {
ltmp_2006_D:
  ltmp_1959_9 = ltmp_1959_9_2E;
  ltmp_1960_9 = ltmp_1959_9 + PLZO_UINT64_C(1);
  ltmp_1961_9 = ((ltmp_1959_9*PLZO_UINT64_C(18446744073709551361)) + ltmp_1956_9) + PLZO_UINT64_C(18446744073709551361);
  *(&ltmp_1936_13[ltmp_1960_9]) = 0;
  if ((ltmp_1961_9 > PLZO_UINT64_C(255))) {
    ltmp_1959_9_2E = ltmp_1960_9;
    goto ltmp_2006_D;
  } else {
    goto ltmp_2008_D;
  }

  } while (1);
ltmp_2008_D:
  ltmp_1962_13 = &ltmp_1936_13[(((plzo_int64_t )ltmp_1959_9) + PLZO_INT64_C(2))];
  ltmp_1963_13_2E = ltmp_1962_13;
  ltmp_1964_9_2E = ltmp_1961_9;
  goto ltmp_2007_D;

ltmp_2007_D:
  ltmp_1963_13 = ltmp_1963_13_2E;
  ltmp_1964_9 = ltmp_1964_9_2E;
  ltmp_1965_13 = &ltmp_1963_13[PLZO_INT64_C(1)];
  *ltmp_1963_13 = ((unsigned char )ltmp_1964_9);
  ltmp_1966_13_2E = ltmp_1965_13;
  goto ltmp_2005_D;

ltmp_2005_D:
  ltmp_1966_13 = ltmp_1966_13_2E;
  *ltmp_1966_13 = ((unsigned char )(ltmp_1952_9 >> 6));
  ltmp_1967_13 = &ltmp_1966_13[PLZO_INT64_C(2)];
  *(&ltmp_1966_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1952_9) << 2)));
  ltmp_1990_13_2E = ltmp_1967_13;
  goto ltmp_1993_D;

ltmp_2001_D:
  ltmp_1968_9 = *(&ltmp_1932_103D->field20);
  if ((ltmp_1968_9 < ltmp_1933_9)) {
    goto ltmp_2009_D;
  } else {
    goto ltmp_2010_D;
  }

ltmp_2010_D:
  ltmp_1969_9 = *(&ltmp_1932_103D->field9);
  if ((ltmp_1969_9 < ltmp_1934_9)) {
    goto ltmp_2009_D;
  } else {
    goto ltmp_2011_D;
  }

ltmp_2011_D:
  ltmp_1970_9 = ltmp_1934_9 - ltmp_1951_9;
  ltmp_1971_9 = *(&ltmp_1932_103D->field4);
  *ltmp_1936_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )(ltmp_1970_9 >> 11)) & 8))) | (((unsigned char )(((unsigned char )ltmp_1933_9) + 253)))))) + ((unsigned char )ltmp_1971_9))));
  *(&ltmp_1936_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_1970_9 >> 6));
  ltmp_1972_13 = &ltmp_1936_13[PLZO_INT64_C(3)];
  *(&ltmp_1936_13[PLZO_INT64_C(2)]) = (((unsigned char )(((unsigned char )ltmp_1970_9) << 2)));
  ltmp_1990_13_2E = ltmp_1972_13;
  goto ltmp_1993_D;

ltmp_2009_D:
  ltmp_1973_9 = (ltmp_1934_9 - ltmp_1951_9) + PLZO_UINT64_C(18446744073709551615);
  ltmp_1974_13 = &ltmp_1936_13[PLZO_INT64_C(1)];
  ltmp_1975_9 = *(&ltmp_1932_103D->field5);
  ltmp_1976_3 = (unsigned char )ltmp_1975_9;
  *ltmp_1936_13 = ltmp_1976_3;
  ltmp_1977_13 = &ltmp_1936_13[PLZO_INT64_C(2)];
  ltmp_1978_3 = (unsigned char )(ltmp_1973_9 >> 14);
  *ltmp_1974_13 = ltmp_1978_3;
  ltmp_1979_9 = *(&ltmp_1932_103D->field21);
  if ((ltmp_1979_9 < ltmp_1933_9)) {
    goto ltmp_2012_D;
  } else {
    goto ltmp_2013_D;
  }

ltmp_2013_D:
  ltmp_1980_3 = ((unsigned char )(((unsigned char )ltmp_1933_9) + 253));
  *ltmp_1936_13 = (((unsigned char )(ltmp_1976_3 | (((unsigned char )(ltmp_1980_3 & 3))))));
  *ltmp_1974_13 = (((unsigned char )(ltmp_1978_3 | (((unsigned char )((((unsigned char )(ltmp_1980_3 << 4))) & 192))))));
  ltmp_1988_13_2E = ltmp_1977_13;
  goto ltmp_2014_D;

ltmp_2012_D:
  ltmp_1981_9 = ltmp_1933_9 - ltmp_1979_9;
  if ((ltmp_1981_9 > PLZO_UINT64_C(255))) {
    ltmp_1982_9_2E = PLZO_UINT64_C(0);
    goto ltmp_2015_D;
  } else {
    ltmp_1985_13_2E = ltmp_1977_13;
    ltmp_1986_9_2E = ltmp_1981_9;
    goto ltmp_2016_D;
  }

  do {
ltmp_2015_D:
  ltmp_1982_9 = ltmp_1982_9_2E;
  ltmp_1983_9 = ((ltmp_1982_9*PLZO_UINT64_C(18446744073709551361)) + ltmp_1981_9) + PLZO_UINT64_C(18446744073709551361);
  *(&ltmp_1936_13[(ltmp_1982_9 + PLZO_UINT64_C(2))]) = 0;
  if ((ltmp_1983_9 > PLZO_UINT64_C(255))) {
    ltmp_1982_9_2E = (ltmp_1982_9 + PLZO_UINT64_C(1));
    goto ltmp_2015_D;
  } else {
    goto ltmp_2017_D;
  }

  } while (1);
ltmp_2017_D:
  ltmp_1984_13 = &ltmp_1936_13[(((plzo_int64_t )ltmp_1982_9) + PLZO_INT64_C(3))];
  ltmp_1985_13_2E = ltmp_1984_13;
  ltmp_1986_9_2E = ltmp_1983_9;
  goto ltmp_2016_D;

ltmp_2016_D:
  ltmp_1985_13 = ltmp_1985_13_2E;
  ltmp_1986_9 = ltmp_1986_9_2E;
  ltmp_1987_13 = &ltmp_1985_13[PLZO_INT64_C(1)];
  *ltmp_1985_13 = ((unsigned char )ltmp_1986_9);
  ltmp_1988_13_2E = ltmp_1987_13;
  goto ltmp_2014_D;

ltmp_2014_D:
  ltmp_1988_13 = ltmp_1988_13_2E;
  *ltmp_1988_13 = ((unsigned char )(ltmp_1973_9 >> 6));
  ltmp_1989_13 = &ltmp_1988_13[PLZO_INT64_C(2)];
  *(&ltmp_1988_13[PLZO_INT64_C(1)]) = (((unsigned char )(((unsigned char )ltmp_1973_9) << 2)));
  ltmp_1990_13_2E = ltmp_1989_13;
  goto ltmp_1993_D;

ltmp_1993_D:
  ltmp_1990_13 = ltmp_1990_13_2E;
  *(&ltmp_1932_103D->field0.field15) = ltmp_1933_9;
  *(&ltmp_1932_103D->field0.field16) = ltmp_1934_9;
  *ltmp_1935_1025 = ltmp_1990_13;
  return;
}


static void ltmp_35_1017(struct l_struct_2E_Lzo1E *ltmp_2018_103D, unsigned char *ltmp_2019_13, plzo_uint64_t ltmp_2020_9, plzo_uint64_t ltmp_2021_9) {
  plzo_uint64_t *ltmp_2022_19;
  plzo_uint64_t ltmp_2023_9;
  plzo_uint64_t *ltmp_2024_19;
  plzo_uint64_t ltmp_2025_9;
  plzo_uint64_t ltmp_2026_9;
  plzo_uint64_t *ltmp_2027_19;
  unsigned char **ltmp_2028_1025;
  unsigned char *ltmp_2029_13;
  unsigned char *ltmp_2030_13;
  unsigned char ltmp_2031_3;
  unsigned char *ltmp_2032_13;
  plzo_uint64_t ltmp_2033_9;
  unsigned char *ltmp_2034_13;
  plzo_uint64_t ltmp_2035_9;
  plzo_uint64_t ltmp_2035_9_2E;
  plzo_uint64_t ltmp_2036_9;
  plzo_uint64_t ltmp_2037_9;
  unsigned char *ltmp_2038_13;
  plzo_uint64_t ltmp_2039_9;
  plzo_uint64_t ltmp_2039_9_2E;
  unsigned char *ltmp_2040_13;
  unsigned char *ltmp_2040_13_2E;
  unsigned char *ltmp_2041_13;
  unsigned char *ltmp_2042_13;
  unsigned char *ltmp_2042_13_2E;
  plzo_uint64_t ltmp_2043_9;
  plzo_uint64_t ltmp_2043_9_2E;
  unsigned char ltmp_2044_3;
  plzo_uint64_t ltmp_2045_9;

  if ((ltmp_2020_9 == PLZO_UINT64_C(0))) {
    goto ltmp_2046_D;
  } else {
    goto ltmp_2047_D;
  }

ltmp_2047_D:
  ltmp_2022_19 = &ltmp_2018_103D->field0.field2.field5;
  ltmp_2023_9 = *ltmp_2022_19;
  ltmp_2024_19 = &ltmp_2018_103D->field0.field2.field4;
  if ((ltmp_2023_9 < ltmp_2020_9)) {
    goto ltmp_2048_D;
  } else {
    goto ltmp_2049_D;
  }

ltmp_2048_D:
  *ltmp_2022_19 = ltmp_2020_9;
  ltmp_2025_9 = *ltmp_2024_19;
  if ((ltmp_2025_9 > ltmp_2020_9)) {
    goto ltmp_2050_D;
  } else {
    goto ltmp_2046_D;
  }

ltmp_2049_D:
  ltmp_2026_9 = *ltmp_2024_19;
  if ((ltmp_2026_9 > ltmp_2020_9)) {
    goto ltmp_2050_D;
  } else {
    goto ltmp_2046_D;
  }

ltmp_2050_D:
  *ltmp_2024_19 = ltmp_2020_9;
  goto ltmp_2046_D;

ltmp_2046_D:
  ltmp_2027_19 = &ltmp_2018_103D->field0.field18;
  if ((ltmp_2020_9 == PLZO_UINT64_C(0))) {
    goto ltmp_2051_D;
  } else {
    goto ltmp_2052_D;
  }

ltmp_2051_D:
  *ltmp_2027_19 = PLZO_UINT64_C(0);
  *(&ltmp_2018_103D->field0.field17) = PLZO_UINT64_C(0);
  return;
ltmp_2052_D:
  *ltmp_2027_19 = ltmp_2021_9;
  *(&ltmp_2018_103D->field0.field17) = ltmp_2020_9;
  ltmp_2028_1025 = &ltmp_2018_103D->field0.field1.field6;
  ltmp_2029_13 = *ltmp_2028_1025;
  if ((ltmp_2020_9 > PLZO_UINT64_C(3))) {
    goto ltmp_2053_D;
  } else {
    goto ltmp_2054_D;
  }

ltmp_2054_D:
  ltmp_2030_13 = &ltmp_2029_13[PLZO_INT64_C(-1)];
  ltmp_2031_3 = *ltmp_2030_13;
  *ltmp_2030_13 = (((unsigned char )(ltmp_2031_3 | ((unsigned char )ltmp_2020_9))));
  ltmp_2042_13_2E = ltmp_2029_13;
  ltmp_2043_9_2E = PLZO_UINT64_C(0);
  goto ltmp_2055_D;

ltmp_2053_D:
  if ((ltmp_2020_9 > PLZO_UINT64_C(18))) {
    goto ltmp_2056_D;
  } else {
    goto ltmp_2057_D;
  }

ltmp_2057_D:
  ltmp_2032_13 = &ltmp_2029_13[PLZO_INT64_C(1)];
  *ltmp_2029_13 = (((unsigned char )(((unsigned char )ltmp_2020_9) + 253)));
  ltmp_2042_13_2E = ltmp_2032_13;
  ltmp_2043_9_2E = PLZO_UINT64_C(0);
  goto ltmp_2055_D;

ltmp_2056_D:
  ltmp_2033_9 = ltmp_2020_9 + PLZO_UINT64_C(18446744073709551598);
  ltmp_2034_13 = &ltmp_2029_13[PLZO_INT64_C(1)];
  *ltmp_2029_13 = 0;
  if ((ltmp_2033_9 > PLZO_UINT64_C(255))) {
    ltmp_2035_9_2E = PLZO_UINT64_C(0);
    goto ltmp_2058_D;
  } else {
    ltmp_2039_9_2E = ltmp_2033_9;
    ltmp_2040_13_2E = ltmp_2034_13;
    goto ltmp_2059_D;
  }

  do {
ltmp_2058_D:
  ltmp_2035_9 = ltmp_2035_9_2E;
  ltmp_2036_9 = ltmp_2035_9 + PLZO_UINT64_C(1);
  ltmp_2037_9 = ((ltmp_2035_9*PLZO_UINT64_C(18446744073709551361)) + ltmp_2033_9) + PLZO_UINT64_C(18446744073709551361);
  *(&ltmp_2029_13[ltmp_2036_9]) = 0;
  if ((ltmp_2037_9 > PLZO_UINT64_C(255))) {
    ltmp_2035_9_2E = ltmp_2036_9;
    goto ltmp_2058_D;
  } else {
    goto ltmp_2060_D;
  }

  } while (1);
ltmp_2060_D:
  ltmp_2038_13 = &ltmp_2029_13[(((plzo_int64_t )ltmp_2035_9) + PLZO_INT64_C(2))];
  ltmp_2039_9_2E = ltmp_2037_9;
  ltmp_2040_13_2E = ltmp_2038_13;
  goto ltmp_2059_D;

ltmp_2059_D:
  ltmp_2039_9 = ltmp_2039_9_2E;
  ltmp_2040_13 = ltmp_2040_13_2E;
  ltmp_2041_13 = &ltmp_2040_13[PLZO_INT64_C(1)];
  *ltmp_2040_13 = ((unsigned char )ltmp_2039_9);
  ltmp_2042_13_2E = ltmp_2041_13;
  ltmp_2043_9_2E = PLZO_UINT64_C(0);
  goto ltmp_2055_D;

  do {
ltmp_2055_D:
  ltmp_2042_13 = ltmp_2042_13_2E;
  ltmp_2043_9 = ltmp_2043_9_2E;
  ltmp_2044_3 = *(&ltmp_2019_13[ltmp_2043_9]);
  *(&ltmp_2042_13[ltmp_2043_9]) = ltmp_2044_3;
  ltmp_2045_9 = ltmp_2043_9 + PLZO_UINT64_C(1);
  if ((ltmp_2045_9 == ltmp_2020_9)) {
    goto ltmp_2061_D;
  } else {
    ltmp_2042_13_2E = ltmp_2042_13;
    ltmp_2043_9_2E = ltmp_2045_9;
    goto ltmp_2055_D;
  }

  } while (1);
ltmp_2061_D:
  *ltmp_2028_1025 = (&ltmp_2042_13[(((plzo_int64_t )ltmp_2043_9) + PLZO_INT64_C(1))]);
  return;
}


static void ltmp_36_1010(struct l_struct_2E_Lzo1E *ltmp_2062_103D) {
  unsigned char **ltmp_2063_1025;
  unsigned char *ltmp_2064_13;

  ltmp_2063_1025 = &ltmp_2062_103D->field0.field1.field6;
  ltmp_2064_13 = *ltmp_2063_1025;
  *ltmp_2063_1025 = (&ltmp_2064_13[PLZO_INT64_C(1)]);
  *ltmp_2064_13 = 0;
  return;
}


static plzo_uint64_t ltmp_37_1015(struct l_struct_2E_Lzo1E *ltmp_2065_103D) {

  return PLZO_UINT64_C(0);
}


static plzo_int64_t ltmp_38_1016(struct l_struct_2E_E *ltmp_2066_1023, struct l_struct_2E_A7 *ltmp_2067_1038) {
  plzo_uint64_t ltmp_2068_9;

  ltmp_2068_9 = *(&ltmp_2067_1038->field0);
  return ((plzo_int64_t )(ltmp_2068_9 == PLZO_UINT64_C(0)));
}


static plzo_int64_t ltmp_39_1016(struct l_struct_2E_E *ltmp_2069_1023, struct l_struct_2E_A7 *ltmp_2070_1038) {
  plzo_uint64_t ltmp_2071_9;

  ltmp_2071_9 = *(&ltmp_2070_1038->field0);
  return ((plzo_int64_t )(ltmp_2071_9 < PLZO_UINT64_C(3)));
}


static void ltmp_40_1001(struct l_struct_2E_A5 *ltmp_2072_1024, plzo_uint64_t ltmp_2073_9) {
  plzo_uint64_t ltmp_2074_9;
  plzo_uint64_t ltmp_2075_9;
  plzo_uint64_t ltmp_2075_9_2E;
  plzo_uint64_t ltmp_2076_9;
  plzo_uint64_t ltmp_2077_9;
  plzo_uint64_t ltmp_2077_9_2E;
  plzo_uint64_t ltmp_2078_9;
  plzo_uint64_t ltmp_2079_9;
  plzo_uint64_t ltmp_2079_9_2E;
  plzo_uint64_t ltmp_2080_9;
  plzo_uint64_t ltmp_2081_9;
  plzo_uint64_t ltmp_2081_9_2E;

  if ((ltmp_2073_9 > PLZO_UINT64_C(1))) {
    goto ltmp_2082_D;
  } else {
    ltmp_2081_9_2E = ltmp_2073_9;
    goto ltmp_2083_D;
  }

ltmp_2082_D:
  ltmp_2074_9 = ltmp_2073_9 + PLZO_UINT64_C(2);
  if ((ltmp_2074_9 < PLZO_UINT64_C(8))) {
    ltmp_2077_9_2E = PLZO_UINT64_C(4);
    goto ltmp_2084_D;
  } else {
    ltmp_2075_9_2E = PLZO_UINT64_C(8);
    goto ltmp_2085_D;
  }

  do {
ltmp_2085_D:
  ltmp_2075_9 = ltmp_2075_9_2E;
  ltmp_2076_9 = ltmp_2075_9 << 1;
  if ((ltmp_2074_9 < ltmp_2076_9)) {
    ltmp_2077_9_2E = ltmp_2075_9;
    goto ltmp_2084_D;
  } else {
    ltmp_2075_9_2E = ltmp_2076_9;
    goto ltmp_2085_D;
  }

  } while (1);
ltmp_2084_D:
  ltmp_2077_9 = ltmp_2077_9_2E;
  ltmp_2078_9 = ltmp_2077_9 & PLZO_UINT64_C(9223372036854775807);
  ltmp_2079_9_2E = ltmp_2078_9;
  goto ltmp_2086_D;

  do {
ltmp_2086_D:
  ltmp_2079_9 = ltmp_2079_9_2E;
  ltmp_2080_9 = ltmp_2079_9 >> 1;
  ltmp_64_1001(ltmp_2072_1024, ((plzo_uint64_t )((ltmp_2080_9 & ltmp_2074_9) != PLZO_UINT64_C(0))));
  ltmp_64_1001(ltmp_2072_1024, PLZO_UINT64_C(0));
  if ((ltmp_2080_9 > PLZO_UINT64_C(2))) {
    ltmp_2079_9_2E = ltmp_2080_9;
    goto ltmp_2086_D;
  } else {
    ltmp_2081_9_2E = ltmp_2074_9;
    goto ltmp_2083_D;
  }

  } while (1);
ltmp_2083_D:
  ltmp_2081_9 = ltmp_2081_9_2E;
  ltmp_64_1001(ltmp_2072_1024, (ltmp_2081_9 & PLZO_UINT64_C(1)));
  ltmp_64_1001(ltmp_2072_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_41_1001(struct l_struct_2E_A5 *ltmp_2087_1024, plzo_uint64_t ltmp_2088_9) {
  plzo_uint64_t ltmp_2089_9;
  plzo_uint64_t ltmp_2090_9;
  plzo_uint64_t ltmp_2091_9;
  plzo_uint64_t ltmp_2092_9;
  plzo_uint64_t ltmp_2092_9_2E;
  plzo_uint64_t ltmp_2093_9;
  plzo_uint64_t ltmp_2094_9;
  plzo_uint64_t ltmp_2094_9_2E;
  plzo_uint64_t ltmp_2095_9;
  plzo_uint64_t ltmp_2096_9;
  plzo_uint64_t ltmp_2096_9_2E;
  plzo_uint64_t ltmp_2097_9;
  plzo_uint64_t ltmp_2098_9;
  plzo_uint64_t ltmp_2098_9_2E;

  if ((ltmp_2088_9 > PLZO_UINT64_C(2))) {
    goto ltmp_2099_D;
  } else {
    goto ltmp_2100_D;
  }

ltmp_2100_D:
  ltmp_2089_9 = ltmp_2088_9 + PLZO_UINT64_C(1);
  ltmp_64_1001(ltmp_2087_1024, ((plzo_uint64_t )(ltmp_2089_9 > PLZO_UINT64_C(1))));
  ltmp_64_1001(ltmp_2087_1024, (ltmp_2089_9 & PLZO_UINT64_C(1)));
  return;
ltmp_2099_D:
  ltmp_2090_9 = ltmp_2088_9 + PLZO_UINT64_C(18446744073709551613);
  ltmp_64_1001(ltmp_2087_1024, PLZO_UINT64_C(0));
  ltmp_64_1001(ltmp_2087_1024, PLZO_UINT64_C(0));
  if ((ltmp_2090_9 > PLZO_UINT64_C(1))) {
    goto ltmp_2101_D;
  } else {
    ltmp_2098_9_2E = ltmp_2090_9;
    goto ltmp_2102_D;
  }

ltmp_2101_D:
  ltmp_2091_9 = ltmp_2088_9 + PLZO_UINT64_C(18446744073709551615);
  if ((ltmp_2091_9 < PLZO_UINT64_C(8))) {
    ltmp_2094_9_2E = PLZO_UINT64_C(4);
    goto ltmp_2103_D;
  } else {
    ltmp_2092_9_2E = PLZO_UINT64_C(8);
    goto ltmp_2104_D;
  }

  do {
ltmp_2104_D:
  ltmp_2092_9 = ltmp_2092_9_2E;
  ltmp_2093_9 = ltmp_2092_9 << 1;
  if ((ltmp_2091_9 < ltmp_2093_9)) {
    ltmp_2094_9_2E = ltmp_2092_9;
    goto ltmp_2103_D;
  } else {
    ltmp_2092_9_2E = ltmp_2093_9;
    goto ltmp_2104_D;
  }

  } while (1);
ltmp_2103_D:
  ltmp_2094_9 = ltmp_2094_9_2E;
  ltmp_2095_9 = ltmp_2094_9 & PLZO_UINT64_C(9223372036854775807);
  ltmp_2096_9_2E = ltmp_2095_9;
  goto ltmp_2105_D;

  do {
ltmp_2105_D:
  ltmp_2096_9 = ltmp_2096_9_2E;
  ltmp_2097_9 = ltmp_2096_9 >> 1;
  ltmp_64_1001(ltmp_2087_1024, ((plzo_uint64_t )((ltmp_2097_9 & ltmp_2091_9) != PLZO_UINT64_C(0))));
  ltmp_64_1001(ltmp_2087_1024, PLZO_UINT64_C(0));
  if ((ltmp_2097_9 > PLZO_UINT64_C(2))) {
    ltmp_2096_9_2E = ltmp_2097_9;
    goto ltmp_2105_D;
  } else {
    ltmp_2098_9_2E = ltmp_2091_9;
    goto ltmp_2102_D;
  }

  } while (1);
ltmp_2102_D:
  ltmp_2098_9 = ltmp_2098_9_2E;
  ltmp_64_1001(ltmp_2087_1024, (ltmp_2098_9 & PLZO_UINT64_C(1)));
  ltmp_64_1001(ltmp_2087_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_42_1001(struct l_struct_2E_A5 *ltmp_2106_1024, plzo_uint64_t ltmp_2107_9) {
  plzo_uint64_t ltmp_2108_9;
  plzo_uint64_t ltmp_2109_9;
  plzo_uint64_t ltmp_2109_9_2E;
  plzo_uint64_t ltmp_2110_9;
  plzo_uint64_t ltmp_2111_9;
  plzo_uint64_t ltmp_2111_9_2E;
  plzo_uint64_t ltmp_2112_9;
  plzo_uint64_t ltmp_2113_9;
  plzo_uint64_t ltmp_2113_9_2E;
  plzo_uint64_t ltmp_2114_9;
  plzo_uint64_t ltmp_2115_9;
  plzo_uint64_t ltmp_2115_9_2E;

  if ((ltmp_2107_9 > PLZO_UINT64_C(1))) {
    goto ltmp_2116_D;
  } else {
    ltmp_2115_9_2E = ltmp_2107_9;
    goto ltmp_2117_D;
  }

ltmp_2116_D:
  ltmp_2108_9 = ltmp_2107_9 + PLZO_UINT64_C(2);
  if ((ltmp_2108_9 < PLZO_UINT64_C(8))) {
    ltmp_2111_9_2E = PLZO_UINT64_C(4);
    goto ltmp_2118_D;
  } else {
    ltmp_2109_9_2E = PLZO_UINT64_C(8);
    goto ltmp_2119_D;
  }

  do {
ltmp_2119_D:
  ltmp_2109_9 = ltmp_2109_9_2E;
  ltmp_2110_9 = ltmp_2109_9 << 1;
  if ((ltmp_2108_9 < ltmp_2110_9)) {
    ltmp_2111_9_2E = ltmp_2109_9;
    goto ltmp_2118_D;
  } else {
    ltmp_2109_9_2E = ltmp_2110_9;
    goto ltmp_2119_D;
  }

  } while (1);
ltmp_2118_D:
  ltmp_2111_9 = ltmp_2111_9_2E;
  ltmp_2112_9 = ltmp_2111_9 & PLZO_UINT64_C(9223372036854775807);
  ltmp_2113_9_2E = ltmp_2112_9;
  goto ltmp_2120_D;

  do {
ltmp_2120_D:
  ltmp_2113_9 = ltmp_2113_9_2E;
  ltmp_2114_9 = ltmp_2113_9 >> 1;
  ltmp_66_1001(ltmp_2106_1024, ((plzo_uint64_t )((ltmp_2114_9 & ltmp_2108_9) != PLZO_UINT64_C(0))));
  ltmp_66_1001(ltmp_2106_1024, PLZO_UINT64_C(0));
  if ((ltmp_2114_9 > PLZO_UINT64_C(2))) {
    ltmp_2113_9_2E = ltmp_2114_9;
    goto ltmp_2120_D;
  } else {
    ltmp_2115_9_2E = ltmp_2108_9;
    goto ltmp_2117_D;
  }

  } while (1);
ltmp_2117_D:
  ltmp_2115_9 = ltmp_2115_9_2E;
  ltmp_66_1001(ltmp_2106_1024, (ltmp_2115_9 & PLZO_UINT64_C(1)));
  ltmp_66_1001(ltmp_2106_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_43_1001(struct l_struct_2E_A5 *ltmp_2121_1024, plzo_uint64_t ltmp_2122_9) {
  plzo_uint64_t ltmp_2123_9;
  plzo_uint64_t ltmp_2124_9;
  plzo_uint64_t ltmp_2125_9;
  plzo_uint64_t ltmp_2126_9;
  plzo_uint64_t ltmp_2126_9_2E;
  plzo_uint64_t ltmp_2127_9;
  plzo_uint64_t ltmp_2128_9;
  plzo_uint64_t ltmp_2128_9_2E;
  plzo_uint64_t ltmp_2129_9;
  plzo_uint64_t ltmp_2130_9;
  plzo_uint64_t ltmp_2130_9_2E;
  plzo_uint64_t ltmp_2131_9;
  plzo_uint64_t ltmp_2132_9;
  plzo_uint64_t ltmp_2132_9_2E;

  if ((ltmp_2122_9 > PLZO_UINT64_C(2))) {
    goto ltmp_2133_D;
  } else {
    goto ltmp_2134_D;
  }

ltmp_2134_D:
  ltmp_2123_9 = ltmp_2122_9 + PLZO_UINT64_C(1);
  ltmp_66_1001(ltmp_2121_1024, ((plzo_uint64_t )(ltmp_2123_9 > PLZO_UINT64_C(1))));
  ltmp_66_1001(ltmp_2121_1024, (ltmp_2123_9 & PLZO_UINT64_C(1)));
  return;
ltmp_2133_D:
  ltmp_2124_9 = ltmp_2122_9 + PLZO_UINT64_C(18446744073709551613);
  ltmp_66_1001(ltmp_2121_1024, PLZO_UINT64_C(0));
  ltmp_66_1001(ltmp_2121_1024, PLZO_UINT64_C(0));
  if ((ltmp_2124_9 > PLZO_UINT64_C(1))) {
    goto ltmp_2135_D;
  } else {
    ltmp_2132_9_2E = ltmp_2124_9;
    goto ltmp_2136_D;
  }

ltmp_2135_D:
  ltmp_2125_9 = ltmp_2122_9 + PLZO_UINT64_C(18446744073709551615);
  if ((ltmp_2125_9 < PLZO_UINT64_C(8))) {
    ltmp_2128_9_2E = PLZO_UINT64_C(4);
    goto ltmp_2137_D;
  } else {
    ltmp_2126_9_2E = PLZO_UINT64_C(8);
    goto ltmp_2138_D;
  }

  do {
ltmp_2138_D:
  ltmp_2126_9 = ltmp_2126_9_2E;
  ltmp_2127_9 = ltmp_2126_9 << 1;
  if ((ltmp_2125_9 < ltmp_2127_9)) {
    ltmp_2128_9_2E = ltmp_2126_9;
    goto ltmp_2137_D;
  } else {
    ltmp_2126_9_2E = ltmp_2127_9;
    goto ltmp_2138_D;
  }

  } while (1);
ltmp_2137_D:
  ltmp_2128_9 = ltmp_2128_9_2E;
  ltmp_2129_9 = ltmp_2128_9 & PLZO_UINT64_C(9223372036854775807);
  ltmp_2130_9_2E = ltmp_2129_9;
  goto ltmp_2139_D;

  do {
ltmp_2139_D:
  ltmp_2130_9 = ltmp_2130_9_2E;
  ltmp_2131_9 = ltmp_2130_9 >> 1;
  ltmp_66_1001(ltmp_2121_1024, ((plzo_uint64_t )((ltmp_2131_9 & ltmp_2125_9) != PLZO_UINT64_C(0))));
  ltmp_66_1001(ltmp_2121_1024, PLZO_UINT64_C(0));
  if ((ltmp_2131_9 > PLZO_UINT64_C(2))) {
    ltmp_2130_9_2E = ltmp_2131_9;
    goto ltmp_2139_D;
  } else {
    ltmp_2132_9_2E = ltmp_2125_9;
    goto ltmp_2136_D;
  }

  } while (1);
ltmp_2136_D:
  ltmp_2132_9 = ltmp_2132_9_2E;
  ltmp_66_1001(ltmp_2121_1024, (ltmp_2132_9 & PLZO_UINT64_C(1)));
  ltmp_66_1001(ltmp_2121_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_44_1001(struct l_struct_2E_A5 *ltmp_2140_1024, plzo_uint64_t ltmp_2141_9) {
  plzo_uint64_t ltmp_2142_9;
  plzo_uint64_t ltmp_2142_9_2E;
  plzo_uint64_t ltmp_2143_9;
  plzo_uint64_t ltmp_2143_9_2E;
  plzo_uint64_t ltmp_2144_9;
  plzo_uint64_t ltmp_2145_9;
  plzo_uint64_t ltmp_2146_9;
  plzo_uint64_t ltmp_2146_9_2E;
  plzo_uint64_t ltmp_2147_9;
  plzo_uint64_t ltmp_2148_9;
  plzo_uint64_t ltmp_2148_9_2E;

  if ((ltmp_2141_9 > PLZO_UINT64_C(1))) {
    ltmp_2142_9_2E = PLZO_UINT64_C(2);
    ltmp_2143_9_2E = ltmp_2141_9;
    goto ltmp_2149_D;
  } else {
    ltmp_2148_9_2E = ltmp_2141_9;
    goto ltmp_2150_D;
  }

  do {
ltmp_2149_D:
  ltmp_2142_9 = ltmp_2142_9_2E;
  ltmp_2143_9 = ltmp_2143_9_2E;
  ltmp_2144_9 = ltmp_2143_9 - ltmp_2142_9;
  ltmp_2145_9 = ltmp_2142_9 << 2;
  if ((ltmp_2144_9 < ltmp_2145_9)) {
    ltmp_2146_9_2E = ltmp_2145_9;
    goto ltmp_2151_D;
  } else {
    ltmp_2142_9_2E = ltmp_2145_9;
    ltmp_2143_9_2E = ltmp_2144_9;
    goto ltmp_2149_D;
  }

  } while (1);
  do {
ltmp_2151_D:
  ltmp_2146_9 = ltmp_2146_9_2E;
  ltmp_64_1001(ltmp_2140_1024, ((plzo_uint64_t )(((ltmp_2146_9 >> 1) & ltmp_2144_9) != PLZO_UINT64_C(0))));
  ltmp_64_1001(ltmp_2140_1024, PLZO_UINT64_C(0));
  ltmp_2147_9 = ltmp_2146_9 >> 2;
  ltmp_64_1001(ltmp_2140_1024, ((plzo_uint64_t )((ltmp_2147_9 & ltmp_2144_9) != PLZO_UINT64_C(0))));
  if ((ltmp_2147_9 > PLZO_UINT64_C(2))) {
    ltmp_2146_9_2E = ltmp_2147_9;
    goto ltmp_2151_D;
  } else {
    ltmp_2148_9_2E = ltmp_2144_9;
    goto ltmp_2150_D;
  }

  } while (1);
ltmp_2150_D:
  ltmp_2148_9 = ltmp_2148_9_2E;
  ltmp_64_1001(ltmp_2140_1024, (ltmp_2148_9 & PLZO_UINT64_C(1)));
  ltmp_64_1001(ltmp_2140_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_45_1001(struct l_struct_2E_A5 *ltmp_2152_1024, plzo_uint64_t ltmp_2153_9) {
  plzo_uint64_t ltmp_2154_9;
  plzo_uint64_t ltmp_2154_9_2E;
  plzo_uint64_t ltmp_2155_9;
  plzo_uint64_t ltmp_2155_9_2E;
  plzo_uint64_t ltmp_2156_9;
  plzo_uint64_t ltmp_2157_9;
  plzo_uint64_t ltmp_2158_9;
  plzo_uint64_t ltmp_2158_9_2E;
  plzo_uint64_t ltmp_2159_9;
  plzo_uint64_t ltmp_2160_9;
  plzo_uint64_t ltmp_2160_9_2E;

  if ((ltmp_2153_9 > PLZO_UINT64_C(1))) {
    ltmp_2154_9_2E = PLZO_UINT64_C(2);
    ltmp_2155_9_2E = ltmp_2153_9;
    goto ltmp_2161_D;
  } else {
    ltmp_2160_9_2E = ltmp_2153_9;
    goto ltmp_2162_D;
  }

  do {
ltmp_2161_D:
  ltmp_2154_9 = ltmp_2154_9_2E;
  ltmp_2155_9 = ltmp_2155_9_2E;
  ltmp_2156_9 = ltmp_2155_9 - ltmp_2154_9;
  ltmp_2157_9 = ltmp_2154_9 << 2;
  if ((ltmp_2156_9 < ltmp_2157_9)) {
    ltmp_2158_9_2E = ltmp_2157_9;
    goto ltmp_2163_D;
  } else {
    ltmp_2154_9_2E = ltmp_2157_9;
    ltmp_2155_9_2E = ltmp_2156_9;
    goto ltmp_2161_D;
  }

  } while (1);
  do {
ltmp_2163_D:
  ltmp_2158_9 = ltmp_2158_9_2E;
  ltmp_66_1001(ltmp_2152_1024, ((plzo_uint64_t )(((ltmp_2158_9 >> 1) & ltmp_2156_9) != PLZO_UINT64_C(0))));
  ltmp_66_1001(ltmp_2152_1024, PLZO_UINT64_C(0));
  ltmp_2159_9 = ltmp_2158_9 >> 2;
  ltmp_66_1001(ltmp_2152_1024, ((plzo_uint64_t )((ltmp_2159_9 & ltmp_2156_9) != PLZO_UINT64_C(0))));
  if ((ltmp_2159_9 > PLZO_UINT64_C(2))) {
    ltmp_2158_9_2E = ltmp_2159_9;
    goto ltmp_2163_D;
  } else {
    ltmp_2160_9_2E = ltmp_2156_9;
    goto ltmp_2162_D;
  }

  } while (1);
ltmp_2162_D:
  ltmp_2160_9 = ltmp_2160_9_2E;
  ltmp_66_1001(ltmp_2152_1024, (ltmp_2160_9 & PLZO_UINT64_C(1)));
  ltmp_66_1001(ltmp_2152_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_46_1001(struct l_struct_2E_A5 *ltmp_2164_1024, plzo_uint64_t ltmp_2165_9) {
  plzo_uint64_t ltmp_2166_9;
  plzo_uint64_t ltmp_2167_9;
  plzo_uint64_t ltmp_2167_9_2E;
  plzo_uint64_t ltmp_2168_9;
  plzo_uint64_t ltmp_2169_9;
  plzo_uint64_t ltmp_2169_9_2E;
  plzo_uint64_t ltmp_2170_9;
  plzo_uint64_t ltmp_2171_9;
  plzo_uint64_t ltmp_2171_9_2E;
  plzo_uint64_t ltmp_2172_9;
  plzo_uint64_t ltmp_2173_9;
  plzo_uint64_t ltmp_2173_9_2E;

  if ((ltmp_2165_9 > PLZO_UINT64_C(1))) {
    goto ltmp_2174_D;
  } else {
    ltmp_2173_9_2E = ltmp_2165_9;
    goto ltmp_2175_D;
  }

ltmp_2174_D:
  ltmp_2166_9 = ltmp_2165_9 + PLZO_UINT64_C(2);
  if ((ltmp_2166_9 < PLZO_UINT64_C(8))) {
    ltmp_2169_9_2E = PLZO_UINT64_C(4);
    goto ltmp_2176_D;
  } else {
    ltmp_2167_9_2E = PLZO_UINT64_C(8);
    goto ltmp_2177_D;
  }

  do {
ltmp_2177_D:
  ltmp_2167_9 = ltmp_2167_9_2E;
  ltmp_2168_9 = ltmp_2167_9 << 1;
  if ((ltmp_2166_9 < ltmp_2168_9)) {
    ltmp_2169_9_2E = ltmp_2167_9;
    goto ltmp_2176_D;
  } else {
    ltmp_2167_9_2E = ltmp_2168_9;
    goto ltmp_2177_D;
  }

  } while (1);
ltmp_2176_D:
  ltmp_2169_9 = ltmp_2169_9_2E;
  ltmp_2170_9 = ltmp_2169_9 & PLZO_UINT64_C(9223372036854775807);
  ltmp_2171_9_2E = ltmp_2170_9;
  goto ltmp_2178_D;

  do {
ltmp_2178_D:
  ltmp_2171_9 = ltmp_2171_9_2E;
  ltmp_2172_9 = ltmp_2171_9 >> 1;
  ltmp_1_1001(ltmp_2164_1024, ((plzo_uint64_t )((ltmp_2172_9 & ltmp_2166_9) != PLZO_UINT64_C(0))));
  ltmp_1_1001(ltmp_2164_1024, PLZO_UINT64_C(0));
  if ((ltmp_2172_9 > PLZO_UINT64_C(2))) {
    ltmp_2171_9_2E = ltmp_2172_9;
    goto ltmp_2178_D;
  } else {
    ltmp_2173_9_2E = ltmp_2166_9;
    goto ltmp_2175_D;
  }

  } while (1);
ltmp_2175_D:
  ltmp_2173_9 = ltmp_2173_9_2E;
  ltmp_1_1001(ltmp_2164_1024, (ltmp_2173_9 & PLZO_UINT64_C(1)));
  ltmp_1_1001(ltmp_2164_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_47_1001(struct l_struct_2E_A5 *ltmp_2179_1024, plzo_uint64_t ltmp_2180_9) {
  plzo_uint64_t ltmp_2181_9;
  plzo_uint64_t ltmp_2181_9_2E;
  plzo_uint64_t ltmp_2182_9;
  plzo_uint64_t ltmp_2182_9_2E;
  plzo_uint64_t ltmp_2183_9;
  plzo_uint64_t ltmp_2184_9;
  plzo_uint64_t ltmp_2185_9;
  plzo_uint64_t ltmp_2185_9_2E;
  plzo_uint64_t ltmp_2186_9;
  plzo_uint64_t ltmp_2187_9;
  plzo_uint64_t ltmp_2187_9_2E;

  if ((ltmp_2180_9 > PLZO_UINT64_C(1))) {
    ltmp_2181_9_2E = PLZO_UINT64_C(2);
    ltmp_2182_9_2E = ltmp_2180_9;
    goto ltmp_2188_D;
  } else {
    ltmp_2187_9_2E = ltmp_2180_9;
    goto ltmp_2189_D;
  }

  do {
ltmp_2188_D:
  ltmp_2181_9 = ltmp_2181_9_2E;
  ltmp_2182_9 = ltmp_2182_9_2E;
  ltmp_2183_9 = ltmp_2182_9 - ltmp_2181_9;
  ltmp_2184_9 = ltmp_2181_9 << 2;
  if ((ltmp_2183_9 < ltmp_2184_9)) {
    ltmp_2185_9_2E = ltmp_2184_9;
    goto ltmp_2190_D;
  } else {
    ltmp_2181_9_2E = ltmp_2184_9;
    ltmp_2182_9_2E = ltmp_2183_9;
    goto ltmp_2188_D;
  }

  } while (1);
  do {
ltmp_2190_D:
  ltmp_2185_9 = ltmp_2185_9_2E;
  ltmp_1_1001(ltmp_2179_1024, ((plzo_uint64_t )(((ltmp_2185_9 >> 1) & ltmp_2183_9) != PLZO_UINT64_C(0))));
  ltmp_1_1001(ltmp_2179_1024, PLZO_UINT64_C(0));
  ltmp_2186_9 = ltmp_2185_9 >> 2;
  ltmp_1_1001(ltmp_2179_1024, ((plzo_uint64_t )((ltmp_2186_9 & ltmp_2183_9) != PLZO_UINT64_C(0))));
  if ((ltmp_2186_9 > PLZO_UINT64_C(2))) {
    ltmp_2185_9_2E = ltmp_2186_9;
    goto ltmp_2190_D;
  } else {
    ltmp_2187_9_2E = ltmp_2183_9;
    goto ltmp_2189_D;
  }

  } while (1);
ltmp_2189_D:
  ltmp_2187_9 = ltmp_2187_9_2E;
  ltmp_1_1001(ltmp_2179_1024, (ltmp_2187_9 & PLZO_UINT64_C(1)));
  ltmp_1_1001(ltmp_2179_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_48_1001(struct l_struct_2E_A5 *ltmp_2191_1024, plzo_uint64_t ltmp_2192_9) {
  plzo_uint64_t ltmp_2193_9;
  plzo_uint64_t ltmp_2194_9;
  plzo_uint64_t ltmp_2195_9;
  plzo_uint64_t ltmp_2196_9;
  plzo_uint64_t ltmp_2196_9_2E;
  plzo_uint64_t ltmp_2197_9;
  plzo_uint64_t ltmp_2198_9;
  plzo_uint64_t ltmp_2198_9_2E;
  plzo_uint64_t ltmp_2199_9;
  plzo_uint64_t ltmp_2200_9;
  plzo_uint64_t ltmp_2200_9_2E;
  plzo_uint64_t ltmp_2201_9;
  plzo_uint64_t ltmp_2202_9;
  plzo_uint64_t ltmp_2202_9_2E;

  if ((ltmp_2192_9 > PLZO_UINT64_C(2))) {
    goto ltmp_2203_D;
  } else {
    goto ltmp_2204_D;
  }

ltmp_2204_D:
  ltmp_2193_9 = ltmp_2192_9 + PLZO_UINT64_C(1);
  ltmp_1_1001(ltmp_2191_1024, ((plzo_uint64_t )(ltmp_2193_9 > PLZO_UINT64_C(1))));
  ltmp_1_1001(ltmp_2191_1024, (ltmp_2193_9 & PLZO_UINT64_C(1)));
  return;
ltmp_2203_D:
  ltmp_2194_9 = ltmp_2192_9 + PLZO_UINT64_C(18446744073709551613);
  ltmp_1_1001(ltmp_2191_1024, PLZO_UINT64_C(0));
  ltmp_1_1001(ltmp_2191_1024, PLZO_UINT64_C(0));
  if ((ltmp_2194_9 > PLZO_UINT64_C(1))) {
    goto ltmp_2205_D;
  } else {
    ltmp_2202_9_2E = ltmp_2194_9;
    goto ltmp_2206_D;
  }

ltmp_2205_D:
  ltmp_2195_9 = ltmp_2192_9 + PLZO_UINT64_C(18446744073709551615);
  if ((ltmp_2195_9 < PLZO_UINT64_C(8))) {
    ltmp_2198_9_2E = PLZO_UINT64_C(4);
    goto ltmp_2207_D;
  } else {
    ltmp_2196_9_2E = PLZO_UINT64_C(8);
    goto ltmp_2208_D;
  }

  do {
ltmp_2208_D:
  ltmp_2196_9 = ltmp_2196_9_2E;
  ltmp_2197_9 = ltmp_2196_9 << 1;
  if ((ltmp_2195_9 < ltmp_2197_9)) {
    ltmp_2198_9_2E = ltmp_2196_9;
    goto ltmp_2207_D;
  } else {
    ltmp_2196_9_2E = ltmp_2197_9;
    goto ltmp_2208_D;
  }

  } while (1);
ltmp_2207_D:
  ltmp_2198_9 = ltmp_2198_9_2E;
  ltmp_2199_9 = ltmp_2198_9 & PLZO_UINT64_C(9223372036854775807);
  ltmp_2200_9_2E = ltmp_2199_9;
  goto ltmp_2209_D;

  do {
ltmp_2209_D:
  ltmp_2200_9 = ltmp_2200_9_2E;
  ltmp_2201_9 = ltmp_2200_9 >> 1;
  ltmp_1_1001(ltmp_2191_1024, ((plzo_uint64_t )((ltmp_2201_9 & ltmp_2195_9) != PLZO_UINT64_C(0))));
  ltmp_1_1001(ltmp_2191_1024, PLZO_UINT64_C(0));
  if ((ltmp_2201_9 > PLZO_UINT64_C(2))) {
    ltmp_2200_9_2E = ltmp_2201_9;
    goto ltmp_2209_D;
  } else {
    ltmp_2202_9_2E = ltmp_2195_9;
    goto ltmp_2206_D;
  }

  } while (1);
ltmp_2206_D:
  ltmp_2202_9 = ltmp_2202_9_2E;
  ltmp_1_1001(ltmp_2191_1024, (ltmp_2202_9 & PLZO_UINT64_C(1)));
  ltmp_1_1001(ltmp_2191_1024, PLZO_UINT64_C(1));
  return;
}


static signed char *ltmp_49_1018(struct plzo_callback_t *ltmp_2210_1026, plzo_uint64_t ltmp_2211_9, plzo_uint64_t ltmp_2212_9) {
  signed char *ltmp_2213_14;
  struct plzo_callback_t *ltmp_2214_1026;
  signed char * (*ltmp_2215_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_2216_14;

  ltmp_2213_14 = *(&ltmp_2210_1026->field3);
  ltmp_2214_1026 = (struct plzo_callback_t *)ltmp_2213_14;
  if ((ltmp_2213_14 == ((signed char *)0))) {
    goto ltmp_2217_D;
  } else {
    goto ltmp_2218_D;
  }

ltmp_2218_D:
  ltmp_2215_1018 = *(&ltmp_2214_1026->field0);
  if ((ltmp_2215_1018 == ((signed char * (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t ))0))) {
    goto ltmp_2217_D;
  } else {
    goto ltmp_2219_D;
  }

ltmp_2219_D:
  ltmp_2216_14 = ltmp_2215_1018(ltmp_2214_1026, ltmp_2211_9, ltmp_2212_9);
  return ltmp_2216_14;
ltmp_2217_D:
  return ((signed char *)0);
}


static void ltmp_50_1019(struct plzo_callback_t *ltmp_2220_1026, signed char *ltmp_2221_14) {
  signed char *ltmp_2222_14;
  struct plzo_callback_t *ltmp_2223_1026;
  void  (*ltmp_2224_1019) (struct plzo_callback_t *, signed char *);

  ltmp_2222_14 = *(&ltmp_2220_1026->field3);
  ltmp_2223_1026 = (struct plzo_callback_t *)ltmp_2222_14;
  if (((ltmp_2222_14 == ((signed char *)0)) | (ltmp_2221_14 == ((signed char *)0)))) {
    goto ltmp_2225_D;
  } else {
    goto ltmp_2226_D;
  }

ltmp_2226_D:
  ltmp_2224_1019 = *(&ltmp_2223_1026->field1);
  if ((ltmp_2224_1019 == ((void  (*) (struct plzo_callback_t *, signed char *))0))) {
    goto ltmp_2225_D;
  } else {
    goto ltmp_2227_D;
  }

ltmp_2227_D:
  ltmp_2224_1019(ltmp_2223_1026, ltmp_2221_14);
  return;
ltmp_2225_D:
  return;
}


static void ltmp_51_101A(struct plzo_callback_t *ltmp_2228_1026, plzo_uint64_t ltmp_2229_9, plzo_uint64_t ltmp_2230_9, plzo_int64_t ltmp_2231_A) {
  signed char *ltmp_2232_14;
  struct plzo_callback_t *ltmp_2233_1026;
  void  (*ltmp_2234_101A) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t );

  ltmp_2232_14 = *(&ltmp_2228_1026->field3);
  ltmp_2233_1026 = (struct plzo_callback_t *)ltmp_2232_14;
  if ((ltmp_2232_14 == ((signed char *)0))) {
    goto ltmp_2235_D;
  } else {
    goto ltmp_2236_D;
  }

ltmp_2236_D:
  ltmp_2234_101A = *(&ltmp_2233_1026->field2);
  if ((ltmp_2234_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t , plzo_int64_t ))0))) {
    goto ltmp_2235_D;
  } else {
    goto ltmp_2237_D;
  }

ltmp_2237_D:
  ltmp_2234_101A(ltmp_2233_1026, ltmp_2229_9, ltmp_2230_9, ltmp_2231_A);
  return;
ltmp_2235_D:
  return;
}


static void ltmp_52_101B(struct l_struct_2E_E *ltmp_2238_1023, struct l_struct_2E_E_3A__3A_A14 *ltmp_2239_1028) {
  void  (*ltmp_2240_1001) (struct l_struct_2E_A5 *, plzo_uint64_t );
  plzo_uint64_t *ltmp_2241_19;
  plzo_uint64_t ltmp_2242_9;
  unsigned char **ltmp_2243_1025;
  signed char *ltmp_2244_14;
  struct plzo_callback_t *ltmp_2245_1026;
  signed char * (*ltmp_2246_1018) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t );
  signed char *ltmp_2247_14;

  ltmp_2240_1001 = *(&ltmp_2239_1028->field3);
  if ((ltmp_2240_1001 == ((void  (*) (struct l_struct_2E_A5 *, plzo_uint64_t ))0))) {
    goto ltmp_2248_D;
  } else {
    goto ltmp_2249_D;
  }

ltmp_2249_D:
  ltmp_2241_19 = &ltmp_2239_1028->field1;
  ltmp_2242_9 = *ltmp_2241_19;
  if ((ltmp_2242_9 == PLZO_UINT64_C(0))) {
    goto ltmp_2248_D;
  } else {
    goto ltmp_2250_D;
  }

ltmp_2250_D:
  ltmp_2243_1025 = &ltmp_2239_1028->field0;
  ltmp_2244_14 = *(&ltmp_2238_1023->field24.field2.field3);
  ltmp_2245_1026 = (struct plzo_callback_t *)ltmp_2244_14;
  if ((ltmp_2244_14 == ((signed char *)0))) {
    goto ltmp_2251_D;
  } else {
    goto ltmp_2252_D;
  }

ltmp_2252_D:
  ltmp_2246_1018 = *(&ltmp_2245_1026->field0);
  if ((ltmp_2246_1018 == ((signed char * (*) (struct plzo_callback_t *, plzo_uint64_t , plzo_uint64_t ))0))) {
    goto ltmp_2251_D;
  } else {
    goto ltmp_2253_D;
  }

ltmp_2253_D:
  ltmp_2247_14 = ltmp_2246_1018(ltmp_2245_1026, ltmp_2242_9, PLZO_UINT64_C(1));
  *ltmp_2243_1025 = ((unsigned char *)ltmp_2247_14);
  if ((ltmp_2247_14 == ((signed char *)0))) {
    goto ltmp_2254_D;
  } else {
    goto ltmp_2248_D;
  }

ltmp_2251_D:
  *ltmp_2243_1025 = ((unsigned char *)0);
  goto ltmp_2254_D;

ltmp_2254_D:
  *ltmp_2241_19 = PLZO_UINT64_C(0);
  return;
ltmp_2248_D:
  return;
}


static void ltmp_53_101C(struct l_struct_2E_E_3A__3A_A14 *ltmp_2255_1028) {
  plzo_uint64_t  (**ltmp_2256_1040) (plzo_uint64_t );
  plzo_uint64_t  (*ltmp_2257_1041) (plzo_uint64_t );
  unsigned char **ltmp_2258_1025;
  unsigned char *ltmp_2259_13;
  plzo_uint64_t *ltmp_2260_19;
  plzo_uint64_t ltmp_2261_9;
  plzo_uint64_t ltmp_2262_9;
  plzo_uint64_t ltmp_2262_9_2E;
  plzo_uint64_t  (*ltmp_2263_1041) (plzo_uint64_t );
  plzo_uint64_t ltmp_2264_9;
  unsigned char *ltmp_2265_13;
  plzo_uint64_t ltmp_2266_9;
  plzo_uint64_t ltmp_2267_9;

  ltmp_2256_1040 = &ltmp_2255_1028->field2;
  ltmp_2257_1041 = *ltmp_2256_1040;
  if ((ltmp_2257_1041 == ((plzo_uint64_t  (*) (plzo_uint64_t ))0))) {
    goto ltmp_2268_D;
  } else {
    goto ltmp_2269_D;
  }

ltmp_2269_D:
  ltmp_2258_1025 = &ltmp_2255_1028->field0;
  ltmp_2259_13 = *ltmp_2258_1025;
  if ((ltmp_2259_13 == ((unsigned char *)0))) {
    goto ltmp_2268_D;
  } else {
    goto ltmp_2270_D;
  }

ltmp_2270_D:
  ltmp_2260_19 = &ltmp_2255_1028->field1;
  ltmp_2261_9 = *ltmp_2260_19;
  if ((ltmp_2261_9 == PLZO_UINT64_C(0))) {
    goto ltmp_2271_D;
  } else {
    ltmp_2262_9_2E = PLZO_UINT64_C(0);
    goto ltmp_2272_D;
  }

  do {
ltmp_2272_D:
  ltmp_2262_9 = ltmp_2262_9_2E;
  ltmp_2263_1041 = *ltmp_2256_1040;
  ltmp_2264_9 = ltmp_2263_1041(ltmp_2262_9);
  ltmp_2265_13 = *ltmp_2258_1025;
  *(&ltmp_2265_13[ltmp_2262_9]) = ((unsigned char )ltmp_2264_9);
  ltmp_2266_9 = ltmp_2262_9 + PLZO_UINT64_C(1);
  ltmp_2267_9 = *ltmp_2260_19;
  if ((ltmp_2267_9 > ltmp_2266_9)) {
    ltmp_2262_9_2E = ltmp_2266_9;
    goto ltmp_2272_D;
  } else {
    goto ltmp_2271_D;
  }

  } while (1);
ltmp_2268_D:
  *(&ltmp_2255_1028->field1) = PLZO_UINT64_C(0);
  return;
ltmp_2271_D:
  return;
}


static void  (*ltmp_54_101D(void  (*ltmp_2273_1001) (struct l_struct_2E_A5 *, plzo_uint64_t ), plzo_uint64_t ltmp_2274_9, plzo_uint64_t ltmp_2275_9)) (struct l_struct_2E_A5 *, plzo_uint64_t ) {
  void  (*ltmp_2276_1001) (struct l_struct_2E_A5 *, plzo_uint64_t );
  void  (*ltmp_2276_1001_2E) (struct l_struct_2E_A5 *, plzo_uint64_t );

  if ((ltmp_2273_1001 == ltmp_46_1001)) {
    goto ltmp_2277_D;
  } else {
    goto ltmp_2278_D;
  }

ltmp_2277_D:
  if ((ltmp_2275_9 == PLZO_UINT64_C(8))) {
    ltmp_2276_1001_2E = ltmp_40_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2280_D;
  }

ltmp_2280_D:
  if (((ltmp_2275_9 == PLZO_UINT64_C(32)) & (ltmp_2274_9 == PLZO_UINT64_C(0)))) {
    ltmp_2276_1001_2E = ltmp_42_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2278_D;
  }

ltmp_2278_D:
  if ((ltmp_2273_1001 == ltmp_47_1001)) {
    goto ltmp_2281_D;
  } else {
    goto ltmp_2282_D;
  }

ltmp_2281_D:
  if ((ltmp_2275_9 == PLZO_UINT64_C(8))) {
    ltmp_2276_1001_2E = ltmp_44_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2283_D;
  }

ltmp_2283_D:
  if (((ltmp_2275_9 == PLZO_UINT64_C(32)) & (ltmp_2274_9 == PLZO_UINT64_C(0)))) {
    ltmp_2276_1001_2E = ltmp_45_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2282_D;
  }

ltmp_2282_D:
  if ((ltmp_2273_1001 == ltmp_48_1001)) {
    goto ltmp_2284_D;
  } else {
    goto ltmp_2285_D;
  }

ltmp_2284_D:
  if ((ltmp_2275_9 == PLZO_UINT64_C(8))) {
    ltmp_2276_1001_2E = ltmp_41_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2286_D;
  }

ltmp_2286_D:
  if (((ltmp_2275_9 == PLZO_UINT64_C(32)) & (ltmp_2274_9 == PLZO_UINT64_C(0)))) {
    ltmp_2276_1001_2E = ltmp_43_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2285_D;
  }

ltmp_2285_D:
  if ((ltmp_2273_1001 == ltmp_63_1001)) {
    goto ltmp_2287_D;
  } else {
    ltmp_2276_1001_2E = ltmp_2273_1001;
    goto ltmp_2279_D;
  }

ltmp_2287_D:
  if ((ltmp_2275_9 == PLZO_UINT64_C(8))) {
    ltmp_2276_1001_2E = ltmp_65_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2288_D;
  }

ltmp_2288_D:
  return (((((ltmp_2275_9 == PLZO_UINT64_C(32)) & (ltmp_2274_9 == PLZO_UINT64_C(0)))) ? (ltmp_67_1001) : (ltmp_2273_1001)));
ltmp_2279_D:
  ltmp_2276_1001 = ltmp_2276_1001_2E;
  return ltmp_2276_1001;
}


static plzo_int64_t ltmp_55_1007(struct l_struct_2E_E *ltmp_2289_1023) {

  return PLZO_INT64_C(0);
}


static void ltmp_56_101E(struct l_struct_2E_E *ltmp_2290_1023, unsigned char *ltmp_2291_13, plzo_uint64_t ltmp_2292_9, plzo_uint64_t ltmp_2293_9) {

  goto ltmp_2294_D;

  do {
ltmp_2294_D:
  goto ltmp_2294_D;

  } while (1);
}


static void ltmp_57_1000(struct l_struct_2E_E *ltmp_2295_1023, plzo_uint64_t ltmp_2296_9, plzo_uint64_t ltmp_2297_9) {

  goto ltmp_2298_D;

  do {
ltmp_2298_D:
  goto ltmp_2298_D;

  } while (1);
}


static void ltmp_58_1005(struct l_struct_2E_E *ltmp_2299_1023) {

  goto ltmp_2300_D;

  do {
ltmp_2300_D:
  goto ltmp_2300_D;

  } while (1);
}


static void ltmp_59_101F(struct l_struct_2E_E *ltmp_2301_1023, struct l_struct_2E_E_3A__3A_A4 *ltmp_2302_103B, plzo_uint64_t ltmp_2303_9, plzo_uint64_t ltmp_2304_9, plzo_uint64_t ltmp_2305_9) {

  goto ltmp_2306_D;

  do {
ltmp_2306_D:
  goto ltmp_2306_D;

  } while (1);
}


static plzo_uint64_t ltmp_60_1020(struct l_struct_2E_E *ltmp_2307_1023) {

  return PLZO_UINT64_C(9);
}


static plzo_int64_t ltmp_61_1016(struct l_struct_2E_E *ltmp_2308_1023, struct l_struct_2E_A7 *ltmp_2309_1038) {
  plzo_uint64_t ltmp_2310_9;

  ltmp_2310_9 = *(&ltmp_2309_1038->field0);
  return ((plzo_int64_t )(ltmp_2310_9 == PLZO_UINT64_C(0)));
}


static plzo_int64_t ltmp_62_1016(struct l_struct_2E_E *ltmp_2311_1023, struct l_struct_2E_A7 *ltmp_2312_1038) {
  plzo_uint64_t ltmp_2313_9;

  ltmp_2313_9 = *(&ltmp_2312_1038->field0);
  return ((plzo_int64_t )(ltmp_2313_9 < PLZO_UINT64_C(3)));
}


static void ltmp_63_1001(struct l_struct_2E_A5 *ltmp_2314_1024, plzo_uint64_t ltmp_2315_9) {
  plzo_uint64_t ltmp_2316_9;
  plzo_uint64_t ltmp_2317_9;
  plzo_uint64_t ltmp_2318_9;
  plzo_uint64_t ltmp_2318_9_2E;
  plzo_uint64_t ltmp_2319_9;
  plzo_uint64_t ltmp_2319_9_2E;
  plzo_uint64_t ltmp_2320_9;
  plzo_uint64_t ltmp_2321_9;
  plzo_uint64_t ltmp_2322_9;
  plzo_uint64_t ltmp_2322_9_2E;
  plzo_uint64_t ltmp_2323_9;
  plzo_uint64_t ltmp_2324_9;
  plzo_uint64_t ltmp_2324_9_2E;

  if ((ltmp_2315_9 > PLZO_UINT64_C(2))) {
    goto ltmp_2325_D;
  } else {
    goto ltmp_2326_D;
  }

ltmp_2326_D:
  ltmp_2316_9 = ltmp_2315_9 + PLZO_UINT64_C(1);
  ltmp_1_1001(ltmp_2314_1024, ((plzo_uint64_t )(ltmp_2316_9 > PLZO_UINT64_C(1))));
  ltmp_1_1001(ltmp_2314_1024, (ltmp_2316_9 & PLZO_UINT64_C(1)));
  return;
ltmp_2325_D:
  ltmp_2317_9 = ltmp_2315_9 + PLZO_UINT64_C(18446744073709551613);
  ltmp_1_1001(ltmp_2314_1024, PLZO_UINT64_C(0));
  ltmp_1_1001(ltmp_2314_1024, PLZO_UINT64_C(0));
  if ((ltmp_2317_9 > PLZO_UINT64_C(1))) {
    ltmp_2318_9_2E = PLZO_UINT64_C(2);
    ltmp_2319_9_2E = ltmp_2317_9;
    goto ltmp_2327_D;
  } else {
    ltmp_2324_9_2E = ltmp_2317_9;
    goto ltmp_2328_D;
  }

  do {
ltmp_2327_D:
  ltmp_2318_9 = ltmp_2318_9_2E;
  ltmp_2319_9 = ltmp_2319_9_2E;
  ltmp_2320_9 = ltmp_2319_9 - ltmp_2318_9;
  ltmp_2321_9 = ltmp_2318_9 << 2;
  if ((ltmp_2320_9 < ltmp_2321_9)) {
    ltmp_2322_9_2E = ltmp_2321_9;
    goto ltmp_2329_D;
  } else {
    ltmp_2318_9_2E = ltmp_2321_9;
    ltmp_2319_9_2E = ltmp_2320_9;
    goto ltmp_2327_D;
  }

  } while (1);
  do {
ltmp_2329_D:
  ltmp_2322_9 = ltmp_2322_9_2E;
  ltmp_1_1001(ltmp_2314_1024, ((plzo_uint64_t )(((ltmp_2322_9 >> 1) & ltmp_2320_9) != PLZO_UINT64_C(0))));
  ltmp_1_1001(ltmp_2314_1024, PLZO_UINT64_C(0));
  ltmp_2323_9 = ltmp_2322_9 >> 2;
  ltmp_1_1001(ltmp_2314_1024, ((plzo_uint64_t )((ltmp_2323_9 & ltmp_2320_9) != PLZO_UINT64_C(0))));
  if ((ltmp_2323_9 > PLZO_UINT64_C(2))) {
    ltmp_2322_9_2E = ltmp_2323_9;
    goto ltmp_2329_D;
  } else {
    ltmp_2324_9_2E = ltmp_2320_9;
    goto ltmp_2328_D;
  }

  } while (1);
ltmp_2328_D:
  ltmp_2324_9 = ltmp_2324_9_2E;
  ltmp_1_1001(ltmp_2314_1024, (ltmp_2324_9 & PLZO_UINT64_C(1)));
  ltmp_1_1001(ltmp_2314_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_64_1001(struct l_struct_2E_A5 *ltmp_2330_1024, plzo_uint64_t ltmp_2331_9) {
  plzo_uint64_t *ltmp_2332_19;
  plzo_uint64_t ltmp_2333_9;
  unsigned char *ltmp_2334_13;
  plzo_uint64_t ltmp_2335_9;
  plzo_uint64_t *ltmp_2336_19;
  plzo_uint64_t ltmp_2337_9;
  plzo_uint64_t ltmp_2338_9;
  unsigned char **ltmp_2339_1025;
  unsigned char *ltmp_2340_13;

  ltmp_2332_19 = &ltmp_2330_1024->field1;
  ltmp_2333_9 = *ltmp_2332_19;
  if ((ltmp_2333_9 == PLZO_UINT64_C(8))) {
    goto ltmp_2341_D;
  } else {
    goto ltmp_2342_D;
  }

ltmp_2341_D:
  ltmp_2334_13 = *(&ltmp_2330_1024->field5);
  ltmp_2335_9 = *(&ltmp_2330_1024->field0);
  *ltmp_2334_13 = ((unsigned char )ltmp_2335_9);
  *ltmp_2332_19 = PLZO_UINT64_C(0);
  goto ltmp_2342_D;

ltmp_2342_D:
  ltmp_2336_19 = &ltmp_2330_1024->field0;
  ltmp_2337_9 = *ltmp_2336_19;
  *ltmp_2336_19 = ((ltmp_2337_9 << 1) + ltmp_2331_9);
  ltmp_2338_9 = *ltmp_2332_19;
  *ltmp_2332_19 = (ltmp_2338_9 + PLZO_UINT64_C(1));
  if ((ltmp_2338_9 == PLZO_UINT64_C(0))) {
    goto ltmp_2343_D;
  } else {
    goto ltmp_2344_D;
  }

ltmp_2343_D:
  ltmp_2339_1025 = &ltmp_2330_1024->field6;
  ltmp_2340_13 = *ltmp_2339_1025;
  *(&ltmp_2330_1024->field5) = ltmp_2340_13;
  *ltmp_2339_1025 = (&ltmp_2340_13[PLZO_INT64_C(1)]);
  return;
ltmp_2344_D:
  return;
}


static void ltmp_65_1001(struct l_struct_2E_A5 *ltmp_2345_1024, plzo_uint64_t ltmp_2346_9) {
  plzo_uint64_t ltmp_2347_9;
  plzo_uint64_t ltmp_2348_9;
  plzo_uint64_t ltmp_2349_9;
  plzo_uint64_t ltmp_2349_9_2E;
  plzo_uint64_t ltmp_2350_9;
  plzo_uint64_t ltmp_2350_9_2E;
  plzo_uint64_t ltmp_2351_9;
  plzo_uint64_t ltmp_2352_9;
  plzo_uint64_t ltmp_2353_9;
  plzo_uint64_t ltmp_2353_9_2E;
  plzo_uint64_t ltmp_2354_9;
  plzo_uint64_t ltmp_2355_9;
  plzo_uint64_t ltmp_2355_9_2E;

  if ((ltmp_2346_9 > PLZO_UINT64_C(2))) {
    goto ltmp_2356_D;
  } else {
    goto ltmp_2357_D;
  }

ltmp_2357_D:
  ltmp_2347_9 = ltmp_2346_9 + PLZO_UINT64_C(1);
  ltmp_64_1001(ltmp_2345_1024, ((plzo_uint64_t )(ltmp_2347_9 > PLZO_UINT64_C(1))));
  ltmp_64_1001(ltmp_2345_1024, (ltmp_2347_9 & PLZO_UINT64_C(1)));
  return;
ltmp_2356_D:
  ltmp_2348_9 = ltmp_2346_9 + PLZO_UINT64_C(18446744073709551613);
  ltmp_64_1001(ltmp_2345_1024, PLZO_UINT64_C(0));
  ltmp_64_1001(ltmp_2345_1024, PLZO_UINT64_C(0));
  if ((ltmp_2348_9 > PLZO_UINT64_C(1))) {
    ltmp_2349_9_2E = PLZO_UINT64_C(2);
    ltmp_2350_9_2E = ltmp_2348_9;
    goto ltmp_2358_D;
  } else {
    ltmp_2355_9_2E = ltmp_2348_9;
    goto ltmp_2359_D;
  }

  do {
ltmp_2358_D:
  ltmp_2349_9 = ltmp_2349_9_2E;
  ltmp_2350_9 = ltmp_2350_9_2E;
  ltmp_2351_9 = ltmp_2350_9 - ltmp_2349_9;
  ltmp_2352_9 = ltmp_2349_9 << 2;
  if ((ltmp_2351_9 < ltmp_2352_9)) {
    ltmp_2353_9_2E = ltmp_2352_9;
    goto ltmp_2360_D;
  } else {
    ltmp_2349_9_2E = ltmp_2352_9;
    ltmp_2350_9_2E = ltmp_2351_9;
    goto ltmp_2358_D;
  }

  } while (1);
  do {
ltmp_2360_D:
  ltmp_2353_9 = ltmp_2353_9_2E;
  ltmp_64_1001(ltmp_2345_1024, ((plzo_uint64_t )(((ltmp_2353_9 >> 1) & ltmp_2351_9) != PLZO_UINT64_C(0))));
  ltmp_64_1001(ltmp_2345_1024, PLZO_UINT64_C(0));
  ltmp_2354_9 = ltmp_2353_9 >> 2;
  ltmp_64_1001(ltmp_2345_1024, ((plzo_uint64_t )((ltmp_2354_9 & ltmp_2351_9) != PLZO_UINT64_C(0))));
  if ((ltmp_2354_9 > PLZO_UINT64_C(2))) {
    ltmp_2353_9_2E = ltmp_2354_9;
    goto ltmp_2360_D;
  } else {
    ltmp_2355_9_2E = ltmp_2351_9;
    goto ltmp_2359_D;
  }

  } while (1);
ltmp_2359_D:
  ltmp_2355_9 = ltmp_2355_9_2E;
  ltmp_64_1001(ltmp_2345_1024, (ltmp_2355_9 & PLZO_UINT64_C(1)));
  ltmp_64_1001(ltmp_2345_1024, PLZO_UINT64_C(1));
  return;
}


static void ltmp_66_1001(struct l_struct_2E_A5 *ltmp_2361_1024, plzo_uint64_t ltmp_2362_9) {
  plzo_uint64_t *ltmp_2363_19;
  plzo_uint64_t ltmp_2364_9;
  plzo_uint64_t ltmp_2365_9;
  unsigned char **ltmp_2366_1025;
  unsigned char *ltmp_2367_13;
  unsigned char *ltmp_2368_13;
  unsigned char *ltmp_2369_13;
  unsigned char *ltmp_2370_13;
  plzo_uint64_t *ltmp_2371_19;
  plzo_uint64_t ltmp_2372_9;
  plzo_uint64_t ltmp_2373_9;
  unsigned char **ltmp_2374_1025;
  unsigned char *ltmp_2375_13;

  ltmp_2363_19 = &ltmp_2361_1024->field1;
  ltmp_2364_9 = *ltmp_2363_19;
  if ((ltmp_2364_9 == PLZO_UINT64_C(32))) {
    goto ltmp_2376_D;
  } else {
    goto ltmp_2377_D;
  }

ltmp_2376_D:
  ltmp_2365_9 = *(&ltmp_2361_1024->field0);
  ltmp_2366_1025 = &ltmp_2361_1024->field5;
  ltmp_2367_13 = *ltmp_2366_1025;
  *ltmp_2367_13 = ((unsigned char )ltmp_2365_9);
  ltmp_2368_13 = *ltmp_2366_1025;
  *(&ltmp_2368_13[PLZO_INT64_C(1)]) = ((unsigned char )(ltmp_2365_9 >> 8));
  ltmp_2369_13 = *ltmp_2366_1025;
  *(&ltmp_2369_13[PLZO_INT64_C(2)]) = ((unsigned char )(ltmp_2365_9 >> 16));
  ltmp_2370_13 = *ltmp_2366_1025;
  *(&ltmp_2370_13[PLZO_INT64_C(3)]) = ((unsigned char )(ltmp_2365_9 >> 24));
  *ltmp_2363_19 = PLZO_UINT64_C(0);
  goto ltmp_2377_D;

ltmp_2377_D:
  ltmp_2371_19 = &ltmp_2361_1024->field0;
  ltmp_2372_9 = *ltmp_2371_19;
  *ltmp_2371_19 = ((ltmp_2372_9 << 1) + ltmp_2362_9);
  ltmp_2373_9 = *ltmp_2363_19;
  *ltmp_2363_19 = (ltmp_2373_9 + PLZO_UINT64_C(1));
  if ((ltmp_2373_9 == PLZO_UINT64_C(0))) {
    goto ltmp_2378_D;
  } else {
    goto ltmp_2379_D;
  }

ltmp_2378_D:
  ltmp_2374_1025 = &ltmp_2361_1024->field6;
  ltmp_2375_13 = *ltmp_2374_1025;
  *(&ltmp_2361_1024->field5) = ltmp_2375_13;
  *ltmp_2374_1025 = (&ltmp_2375_13[PLZO_INT64_C(4)]);
  return;
ltmp_2379_D:
  return;
}


static void ltmp_67_1001(struct l_struct_2E_A5 *ltmp_2380_1024, plzo_uint64_t ltmp_2381_9) {
  plzo_uint64_t ltmp_2382_9;
  plzo_uint64_t ltmp_2383_9;
  plzo_uint64_t ltmp_2384_9;
  plzo_uint64_t ltmp_2384_9_2E;
  plzo_uint64_t ltmp_2385_9;
  plzo_uint64_t ltmp_2385_9_2E;
  plzo_uint64_t ltmp_2386_9;
  plzo_uint64_t ltmp_2387_9;
  plzo_uint64_t ltmp_2388_9;
  plzo_uint64_t ltmp_2388_9_2E;
  plzo_uint64_t ltmp_2389_9;
  plzo_uint64_t ltmp_2390_9;
  plzo_uint64_t ltmp_2390_9_2E;

  if ((ltmp_2381_9 > PLZO_UINT64_C(2))) {
    goto ltmp_2391_D;
  } else {
    goto ltmp_2392_D;
  }

ltmp_2392_D:
  ltmp_2382_9 = ltmp_2381_9 + PLZO_UINT64_C(1);
  ltmp_66_1001(ltmp_2380_1024, ((plzo_uint64_t )(ltmp_2382_9 > PLZO_UINT64_C(1))));
  ltmp_66_1001(ltmp_2380_1024, (ltmp_2382_9 & PLZO_UINT64_C(1)));
  return;
ltmp_2391_D:
  ltmp_2383_9 = ltmp_2381_9 + PLZO_UINT64_C(18446744073709551613);
  ltmp_66_1001(ltmp_2380_1024, PLZO_UINT64_C(0));
  ltmp_66_1001(ltmp_2380_1024, PLZO_UINT64_C(0));
  if ((ltmp_2383_9 > PLZO_UINT64_C(1))) {
    ltmp_2384_9_2E = PLZO_UINT64_C(2);
    ltmp_2385_9_2E = ltmp_2383_9;
    goto ltmp_2393_D;
  } else {
    ltmp_2390_9_2E = ltmp_2383_9;
    goto ltmp_2394_D;
  }

  do {
ltmp_2393_D:
  ltmp_2384_9 = ltmp_2384_9_2E;
  ltmp_2385_9 = ltmp_2385_9_2E;
  ltmp_2386_9 = ltmp_2385_9 - ltmp_2384_9;
  ltmp_2387_9 = ltmp_2384_9 << 2;
  if ((ltmp_2386_9 < ltmp_2387_9)) {
    ltmp_2388_9_2E = ltmp_2387_9;
    goto ltmp_2395_D;
  } else {
    ltmp_2384_9_2E = ltmp_2387_9;
    ltmp_2385_9_2E = ltmp_2386_9;
    goto ltmp_2393_D;
  }

  } while (1);
  do {
ltmp_2395_D:
  ltmp_2388_9 = ltmp_2388_9_2E;
  ltmp_66_1001(ltmp_2380_1024, ((plzo_uint64_t )(((ltmp_2388_9 >> 1) & ltmp_2386_9) != PLZO_UINT64_C(0))));
  ltmp_66_1001(ltmp_2380_1024, PLZO_UINT64_C(0));
  ltmp_2389_9 = ltmp_2388_9 >> 2;
  ltmp_66_1001(ltmp_2380_1024, ((plzo_uint64_t )((ltmp_2389_9 & ltmp_2386_9) != PLZO_UINT64_C(0))));
  if ((ltmp_2389_9 > PLZO_UINT64_C(2))) {
    ltmp_2388_9_2E = ltmp_2389_9;
    goto ltmp_2395_D;
  } else {
    ltmp_2390_9_2E = ltmp_2386_9;
    goto ltmp_2394_D;
  }

  } while (1);
ltmp_2394_D:
  ltmp_2390_9 = ltmp_2390_9_2E;
  ltmp_66_1001(ltmp_2380_1024, (ltmp_2390_9 & PLZO_UINT64_C(1)));
  ltmp_66_1001(ltmp_2380_1024, PLZO_UINT64_C(1));
  return;
}


