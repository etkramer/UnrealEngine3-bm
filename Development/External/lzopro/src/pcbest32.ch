/* pcbest32.ch -- LZO1[XY]-99 compression

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
  signed char * (*field0) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t );
  void  (*field1) (struct plzo_callback_t *, signed char *);
  void  (*field2) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );
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
static signed char *ltmp_49_1018(struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t );
static void ltmp_50_1019(struct plzo_callback_t *, signed char *);
static void ltmp_51_101A(struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );
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
  *ltmp_101_1025 = (&ltmp_102_13[ltmp_103_7]);
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
  *ltmp_106_1025 = (&ltmp_107_13[ltmp_108_7]);
  *(&ltmp_96_1024->field0) = ltmp_97_7;
  *ltmp_98_17 = 1u;
  return;
}


static plzo_int32_t ltmp_2_1002(struct l_struct_2E_E *ltmp_113_1023, plzo_int32_t ltmp_114_8, struct plzo_callback_t *ltmp_115_1026) {
  struct plzo_compress_config_t ltmp_116_1027;
  struct l_struct_2E_E_3A__3A_A14 *ltmp_117_1028;
  struct l_struct_2E_E_3A__3A_A14 *ltmp_118_1028;
  unsigned char *ltmp_119_13;
  plzo_uint32_t ltmp_120_7;
  plzo_uint32_t ltmp_120_7_2E;
  plzo_uint32_t ltmp_121_7;
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
  plzo_uint32_t ltmp_135_7;
  plzo_uint32_t ltmp_135_7_2E;
  plzo_uint32_t ltmp_136_7;
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
  ltmp_120_7_2E = 0u;
  goto ltmp_142_D;

  do {
ltmp_142_D:
  ltmp_120_7 = ltmp_120_7_2E;
  *(&ltmp_119_13[ltmp_120_7]) = 0;
  ltmp_121_7 = ltmp_120_7 + 1u;
  if ((ltmp_121_7 == 64u)) {
    goto ltmp_143_D;
  } else {
    ltmp_120_7_2E = ltmp_121_7;
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
  ltmp_123_102A = *(&ltmp_122_1029[2]);
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
  ltmp_135_7_2E = 0u;
  goto ltmp_149_D;

  do {
ltmp_149_D:
  ltmp_135_7 = ltmp_135_7_2E;
  *(&ltmp_134_13[ltmp_135_7]) = 255;
  ltmp_136_7 = ltmp_135_7 + 1u;
  if ((ltmp_136_7 == 36u)) {
    goto ltmp_150_D;
  } else {
    ltmp_135_7_2E = ltmp_136_7;
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
  switch (ltmp_155_8) {
  default:
    ltmp_156_8_2E = -2;
    goto ltmp_204_D;
;
  case -1:
    goto ltmp_205_D;
  case 8:
    goto ltmp_206_D;
  case 16:
    goto ltmp_206_D;
  case 32:
    goto ltmp_206_D;
  }
ltmp_202_D:
  switch (ltmp_155_8) {
  default:
    ltmp_156_8_2E = -2;
    goto ltmp_204_D;
;
  case -1:
    goto ltmp_205_D;
  case 8:
    goto ltmp_206_D;
    break;
  case 16:
    goto ltmp_206_D;
    break;
  case 32:
    goto ltmp_206_D;
    break;
  }
ltmp_206_D:
  *(&ltmp_151_1023->field1.field3) = ((plzo_uint32_t )ltmp_155_8);
  *(&ltmp_151_1023->field1.field4) = ((plzo_uint32_t )(ltmp_155_8/8));
  goto ltmp_205_D;

ltmp_205_D:
  *(&ltmp_151_1023->field1.field0) = 0u;
  *(&ltmp_151_1023->field1.field1) = 0u;
  *(&ltmp_151_1023->field1.field5) = ((unsigned char *)0);
  *(&ltmp_151_1023->field1.field6) = ((unsigned char *)0);
  ltmp_156_8_2E = 0;
  goto ltmp_204_D;

ltmp_204_D:
  ltmp_156_8 = ltmp_156_8_2E;
  if ((ltmp_156_8 == 0)) {
    goto ltmp_207_D;
  } else {
    ltmp_199_8_2E = ltmp_156_8;
    goto ltmp_201_D;
  }

ltmp_207_D:
  ltmp_157_7 = *(&ltmp_152_1027->field2);
  ltmp_158_17 = &ltmp_152_1027->field3;
  if ((ltmp_157_7 == 4294967295u)) {
    goto ltmp_208_D;
  } else {
    goto ltmp_209_D;
  }

ltmp_209_D:
  ltmp_159_7 = *(&ltmp_151_1023->field6.field3);
  *(&ltmp_151_1023->field7.field2) = ((((ltmp_157_7 <= ltmp_159_7)) ? (ltmp_157_7) : (ltmp_159_7)));
  ltmp_160_7 = *ltmp_158_17;
  if ((ltmp_160_7 == 4294967295u)) {
    goto ltmp_210_D;
  } else {
    goto ltmp_211_D;
  }

ltmp_208_D:
  ltmp_161_7 = *ltmp_158_17;
  if ((ltmp_161_7 == 4294967295u)) {
    goto ltmp_210_D;
  } else {
    goto ltmp_211_D;
  }

ltmp_211_D:
  ltmp_162_7 = *ltmp_158_17;
  ltmp_163_7 = *(&ltmp_151_1023->field6.field4);
  if ((ltmp_162_7 < ltmp_163_7)) {
    goto ltmp_210_D;
  } else {
    goto ltmp_212_D;
  }

ltmp_212_D:
  ltmp_164_7 = *(&ltmp_151_1023->field6.field5);
  *(&ltmp_151_1023->field7.field4) = ((((ltmp_162_7 <= ltmp_164_7)) ? (ltmp_162_7) : (ltmp_164_7)));
  ltmp_165_8 = *(&ltmp_152_1027->field7);
  if ((ltmp_165_8 == -1)) {
    goto ltmp_213_D;
  } else {
    goto ltmp_214_D;
  }

ltmp_210_D:
  ltmp_166_8 = *(&ltmp_152_1027->field7);
  if ((ltmp_166_8 == -1)) {
    goto ltmp_213_D;
  } else {
    goto ltmp_214_D;
  }

ltmp_214_D:
  ltmp_167_8 = *(&ltmp_152_1027->field7);
  *(&ltmp_151_1023->field7.field14) = ltmp_167_8;
  ltmp_168_7 = *(&ltmp_152_1027->field8);
  if ((ltmp_168_7 == 4294967295u)) {
    goto ltmp_215_D;
  } else {
    goto ltmp_216_D;
  }

ltmp_213_D:
  ltmp_169_7 = *(&ltmp_152_1027->field8);
  if ((ltmp_169_7 == 4294967295u)) {
    goto ltmp_215_D;
  } else {
    goto ltmp_216_D;
  }

ltmp_216_D:
  ltmp_170_7 = *(&ltmp_152_1027->field8);
  *(&ltmp_151_1023->field7.field1) = ltmp_170_7;
  ltmp_171_8 = *(&ltmp_152_1027->field6);
  switch (ltmp_171_8) {
  default:
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
;
  case 7:
    goto ltmp_217_D;
  case 6:
    goto ltmp_218_D;
  case 5:
    goto ltmp_219_D;
  case 4:
    goto ltmp_220_D;
  case 3:
    goto ltmp_221_D;
  case 2:
    goto ltmp_222_D;
  case 1:
    goto ltmp_223_D;
  case 0:
    goto ltmp_224_D;
  case -1:
    goto ltmp_225_D;
  }
ltmp_215_D:
  ltmp_172_8 = *(&ltmp_152_1027->field6);
  switch (ltmp_172_8) {
  default:
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
;
  case 7:
    goto ltmp_217_D;
  case 6:
    goto ltmp_218_D;
  case 5:
    goto ltmp_219_D;
  case 4:
    goto ltmp_220_D;
  case 3:
    goto ltmp_221_D;
  case 2:
    goto ltmp_222_D;
  case 1:
    goto ltmp_223_D;
  case 0:
    goto ltmp_224_D;
    break;
  case -1:
    goto ltmp_225_D;
  }
ltmp_224_D:
  *(&ltmp_151_1023->field7.field11) = 0;
  ltmp_173_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_173_7;
  *(&ltmp_151_1023->field7.field13) = 0u;
  goto ltmp_225_D;

ltmp_223_D:
  *(&ltmp_151_1023->field7.field11) = 1;
  ltmp_174_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_174_7;
  *(&ltmp_151_1023->field7.field13) = 0u;
  goto ltmp_225_D;

ltmp_222_D:
  *(&ltmp_151_1023->field7.field11) = 2;
  ltmp_175_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = ltmp_175_7;
  *(&ltmp_151_1023->field7.field13) = 0u;
  goto ltmp_225_D;

ltmp_221_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  ltmp_176_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_176_7 + 1u);
  *(&ltmp_151_1023->field7.field13) = 1u;
  goto ltmp_225_D;

ltmp_220_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  ltmp_177_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_177_7 + 6u);
  *(&ltmp_151_1023->field7.field13) = 4u;
  goto ltmp_225_D;

ltmp_219_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  ltmp_178_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_178_7 + 32u);
  *(&ltmp_151_1023->field7.field13) = 16u;
  goto ltmp_225_D;

ltmp_218_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  ltmp_179_7 = *(&ltmp_151_1023->field7.field3);
  *(&ltmp_151_1023->field7.field12) = (ltmp_179_7 + 32u);
  *(&ltmp_151_1023->field7.field13) = 32u;
  goto ltmp_225_D;

ltmp_217_D:
  *(&ltmp_151_1023->field7.field11) = 3;
  *(&ltmp_151_1023->field7.field12) = 0u;
  *(&ltmp_151_1023->field7.field13) = 0u;
  goto ltmp_225_D;

ltmp_225_D:
  ltmp_180_18 = &ltmp_151_1023->field7.field9;
  *ltmp_180_18 = 0;
  ltmp_181_8 = *(&ltmp_151_1023->field7.field11);
  if ((((plzo_uint32_t )ltmp_181_8) > 1u)) {
    goto ltmp_226_D;
  } else {
    goto ltmp_227_D;
  }

ltmp_227_D:
  *ltmp_180_18 = 0;
  goto ltmp_228_D;

ltmp_226_D:
  if ((ltmp_153_8 > 9)) {
    goto ltmp_229_D;
  } else {
    goto ltmp_230_D;
  }

ltmp_230_D:
  *ltmp_180_18 = 1;
  goto ltmp_228_D;

ltmp_229_D:
  *ltmp_180_18 = 2;
  goto ltmp_228_D;

ltmp_228_D:
  *(&ltmp_151_1023->field7.field10) = 0;
  ltmp_182_8 = *(&ltmp_151_1023->field6.field1);
  if (((ltmp_182_8 & 1) == 0)) {
    goto ltmp_231_D;
  } else {
    goto ltmp_232_D;
  }

ltmp_232_D:
  if ((ltmp_153_8 > 6)) {
    goto ltmp_233_D;
  } else {
    goto ltmp_234_D;
  }

ltmp_234_D:
  *(&ltmp_151_1023->field7.field15) = 0;
  ltmp_183_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_184_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_183_7 > ltmp_184_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_235_D;
  }

ltmp_233_D:
  switch (ltmp_153_8) {
  default:
    goto ltmp_236_D;
;
  case 7:
    goto ltmp_237_D;
    break;
  case 8:
    goto ltmp_238_D;
  case 9:
    goto ltmp_239_D;
  }
ltmp_237_D:
  *(&ltmp_151_1023->field7.field15) = 1;
  ltmp_185_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_186_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_185_7 > ltmp_186_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_235_D;
  }

ltmp_238_D:
  *(&ltmp_151_1023->field7.field15) = 2;
  ltmp_187_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_188_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_187_7 > ltmp_188_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_235_D;
  }

ltmp_239_D:
  *(&ltmp_151_1023->field7.field15) = 4;
  ltmp_189_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_190_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_189_7 > ltmp_190_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_235_D;
  }

ltmp_236_D:
  if ((ltmp_153_8 > 9)) {
    goto ltmp_240_D;
  } else {
    goto ltmp_231_D;
  }

ltmp_240_D:
  *(&ltmp_151_1023->field7.field15) = 6;
  ltmp_191_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_192_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_191_7 > ltmp_192_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_235_D;
  }

ltmp_231_D:
  ltmp_193_7 = *(&ltmp_151_1023->field7.field2);
  ltmp_194_7 = *(&ltmp_151_1023->field6.field3);
  if ((ltmp_193_7 > ltmp_194_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_235_D;
  }

ltmp_235_D:
  ltmp_195_7 = *(&ltmp_151_1023->field7.field4);
  ltmp_196_7 = *(&ltmp_151_1023->field6.field5);
  if ((ltmp_195_7 > ltmp_196_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_241_D;
  }

ltmp_241_D:
  ltmp_197_7 = *(&ltmp_151_1023->field7.field3);
  ltmp_198_7 = *(&ltmp_151_1023->field6.field4);
  if ((ltmp_197_7 < ltmp_198_7)) {
    ltmp_199_8_2E = -2;
    goto ltmp_201_D;
  } else {
    goto ltmp_242_D;
  }

ltmp_242_D:
  return ((((ltmp_197_7 > ltmp_195_7)) ? (-2) : (ltmp_156_8)));
ltmp_201_D:
  ltmp_199_8 = ltmp_199_8_2E;
  return ltmp_199_8;
}


static plzo_int32_t ltmp_4_1004(struct l_struct_2E_E *ltmp_243_1023, unsigned char *ltmp_244_13, plzo_uint32_t ltmp_245_7, unsigned char *ltmp_246_13, plzo_uint32_t *ltmp_247_17) {
  plzo_uint32_t *ltmp_248_17;
  unsigned char **ltmp_249_1025;
  unsigned char **ltmp_250_1025;
  plzo_uint32_t *ltmp_251_17;
  plzo_int32_t  (***ltmp_252_102C) ();
  plzo_int32_t  (**ltmp_253_1029) ();
  plzo_int32_t  (*ltmp_254_102A) ();
  plzo_int32_t ltmp_255_8;
  plzo_int32_t ltmp_256_8;
  plzo_int32_t  (**ltmp_257_1029) ();
  plzo_int32_t  (*ltmp_258_102A) ();
  plzo_uint32_t ltmp_259_7;
  plzo_uint32_t ltmp_260_7;
  plzo_int32_t  (**ltmp_261_1029) ();
  plzo_int32_t  (*ltmp_262_102A) ();
  plzo_int32_t ltmp_263_8;
  plzo_int32_t ltmp_264_8;
  plzo_int32_t ltmp_264_8_2E;
  plzo_int32_t  (**ltmp_265_1029) ();
  plzo_int32_t  (*ltmp_266_102A) ();
  plzo_int32_t ltmp_267_8;
  plzo_int32_t ltmp_267_8_2E;
  unsigned char *ltmp_268_13;
  unsigned char *ltmp_269_13;
  plzo_int32_t ltmp_270_8;
  plzo_uint32_t ltmp_271_7;
  signed char *ltmp_272_14;
  struct plzo_callback_t *ltmp_273_1026;
  void  (*ltmp_274_101A) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );
  plzo_int32_t ltmp_275_8;
  plzo_int32_t ltmp_275_8_2E;

  *(&ltmp_243_1023->field10) = ltmp_244_13;
  *(&ltmp_243_1023->field8) = ltmp_244_13;
  ltmp_248_17 = &ltmp_243_1023->field11;
  *ltmp_248_17 = ltmp_245_7;
  ltmp_249_1025 = &ltmp_243_1023->field9;
  ltmp_250_1025 = &ltmp_243_1023->field1.field6;
  *ltmp_250_1025 = ltmp_246_13;
  *ltmp_249_1025 = ltmp_246_13;
  *(&ltmp_243_1023->field12) = 0u;
  ltmp_251_17 = &ltmp_243_1023->field13;
  *ltmp_251_17 = 0u;
  ltmp_252_102C = &ltmp_243_1023->field0;
  ltmp_253_1029 = *ltmp_252_102C;
  ltmp_254_102A = *(&ltmp_253_1029[7]);
  ltmp_255_8 = ((plzo_int32_t  (*) (struct l_struct_2E_E *))ltmp_254_102A)(ltmp_243_1023);
  if ((ltmp_255_8 == 0)) {
    goto ltmp_276_D;
  } else {
    ltmp_275_8_2E = ltmp_255_8;
    goto ltmp_277_D;
  }

ltmp_276_D:
  ltmp_256_8 = *(&ltmp_243_1023->field3);
  if ((ltmp_256_8 == 0)) {
    ltmp_275_8_2E = -1;
    goto ltmp_277_D;
  } else {
    goto ltmp_278_D;
  }

ltmp_278_D:
  ltmp_257_1029 = *ltmp_252_102C;
  ltmp_258_102A = *(&ltmp_257_1029[8]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_258_102A)(ltmp_243_1023);
  ltmp_259_7 = *ltmp_248_17;
  ltmp_260_7 = *(&ltmp_243_1023->field6.field2);
  if ((ltmp_259_7 < ltmp_260_7)) {
    goto ltmp_279_D;
  } else {
    goto ltmp_280_D;
  }

ltmp_280_D:
  ltmp_261_1029 = *ltmp_252_102C;
  ltmp_262_102A = *(&ltmp_261_1029[9]);
  ltmp_263_8 = ((plzo_int32_t  (*) (struct l_struct_2E_E *))ltmp_262_102A)(ltmp_243_1023);
  if ((ltmp_263_8 == 0)) {
    ltmp_264_8_2E = ltmp_263_8;
    goto ltmp_281_D;
  } else {
    ltmp_267_8_2E = ltmp_263_8;
    goto ltmp_282_D;
  }

ltmp_279_D:
  *ltmp_251_17 = ltmp_259_7;
  if ((ltmp_255_8 == 0)) {
    ltmp_264_8_2E = ltmp_255_8;
    goto ltmp_281_D;
  } else {
    ltmp_267_8_2E = ltmp_255_8;
    goto ltmp_282_D;
  }

ltmp_281_D:
  ltmp_264_8 = ltmp_264_8_2E;
  ltmp_265_1029 = *ltmp_252_102C;
  ltmp_266_102A = *(&ltmp_265_1029[10]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_266_102A)(ltmp_243_1023);
  ltmp_267_8_2E = ltmp_264_8;
  goto ltmp_282_D;

ltmp_282_D:
  ltmp_267_8 = ltmp_267_8_2E;
  ltmp_268_13 = *ltmp_250_1025;
  ltmp_269_13 = *ltmp_249_1025;
  ltmp_270_8 = plzo_external_ptrdiff_uchar(ltmp_268_13, ltmp_269_13, 1u);
  ltmp_271_7 = (plzo_uint32_t )ltmp_270_8;
  *ltmp_247_17 = ltmp_271_7;
  ltmp_272_14 = *(&ltmp_243_1023->field24.field2.field3);
  ltmp_273_1026 = (struct plzo_callback_t *)ltmp_272_14;
  if ((ltmp_272_14 == ((signed char *)0))) {
    ltmp_275_8_2E = ltmp_267_8;
    goto ltmp_277_D;
  } else {
    goto ltmp_283_D;
  }

ltmp_283_D:
  ltmp_274_101A = *(&ltmp_273_1026->field2);
  if ((ltmp_274_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))0))) {
    ltmp_275_8_2E = ltmp_267_8;
    goto ltmp_277_D;
  } else {
    goto ltmp_284_D;
  }

ltmp_284_D:
  ltmp_274_101A(ltmp_273_1026, ltmp_245_7, ltmp_271_7, 4);
  return ltmp_267_8;
ltmp_277_D:
  ltmp_275_8 = ltmp_275_8_2E;
  return ltmp_275_8;
}


static void ltmp_5_1005(struct l_struct_2E_E *ltmp_285_1023) {
  unsigned char **ltmp_286_1025;
  unsigned char *ltmp_287_13;
  signed char *ltmp_288_14;
  signed char **ltmp_289_102D;
  signed char *ltmp_290_14;
  struct plzo_callback_t *ltmp_291_1026;
  void  (*ltmp_292_1019) (struct plzo_callback_t *, signed char *);
  unsigned char **ltmp_293_1025;
  unsigned char *ltmp_294_13;
  signed char *ltmp_295_14;
  signed char *ltmp_296_14;
  struct plzo_callback_t *ltmp_297_1026;
  void  (*ltmp_298_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2 *ltmp_299_102E;
  struct l_struct_2E_A9 *ltmp_300_102F;
  struct l_struct_2E_A2 *ltmp_301_102E;
  struct l_struct_2E_A9 *ltmp_302_102F;
  struct l_struct_2E_A2 *ltmp_303_102E;
  struct l_struct_2E_A2 *ltmp_303_102E_2E;
  struct l_struct_2E_A9 *ltmp_304_102F;
  void  (*ltmp_305_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_306_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_307_1031;

  *(&ltmp_285_1023->field0) = (&(ltmp_70_1021[2]));
  ltmp_286_1025 = &ltmp_285_1023->field22.field0;
  ltmp_287_13 = *ltmp_286_1025;
  ltmp_288_14 = (signed char *)ltmp_287_13;
  ltmp_289_102D = &ltmp_285_1023->field24.field2.field3;
  ltmp_290_14 = *ltmp_289_102D;
  ltmp_291_1026 = (struct plzo_callback_t *)ltmp_290_14;
  if (((ltmp_290_14 == ((signed char *)0)) | (ltmp_287_13 == ((unsigned char *)0)))) {
    goto ltmp_308_D;
  } else {
    goto ltmp_309_D;
  }

ltmp_309_D:
  ltmp_292_1019 = *(&ltmp_291_1026->field1);
  if ((ltmp_292_1019 == ((void  (*) (struct plzo_callback_t *, signed char *))0))) {
    goto ltmp_308_D;
  } else {
    goto ltmp_310_D;
  }

ltmp_310_D:
  ltmp_292_1019(ltmp_291_1026, ltmp_288_14);
  *ltmp_286_1025 = ((unsigned char *)0);
  *(&ltmp_285_1023->field22.field1) = 0u;
  goto ltmp_311_D;

ltmp_308_D:
  *ltmp_286_1025 = ((unsigned char *)0);
  *(&ltmp_285_1023->field22.field1) = 0u;
  goto ltmp_311_D;

ltmp_311_D:
  ltmp_293_1025 = &ltmp_285_1023->field23.field0;
  ltmp_294_13 = *ltmp_293_1025;
  ltmp_295_14 = (signed char *)ltmp_294_13;
  ltmp_296_14 = *ltmp_289_102D;
  ltmp_297_1026 = (struct plzo_callback_t *)ltmp_296_14;
  if (((ltmp_296_14 == ((signed char *)0)) | (ltmp_294_13 == ((unsigned char *)0)))) {
    goto ltmp_312_D;
  } else {
    goto ltmp_313_D;
  }

ltmp_313_D:
  ltmp_298_1019 = *(&ltmp_297_1026->field1);
  if ((ltmp_298_1019 == ((void  (*) (struct plzo_callback_t *, signed char *))0))) {
    goto ltmp_312_D;
  } else {
    goto ltmp_314_D;
  }

ltmp_314_D:
  ltmp_298_1019(ltmp_297_1026, ltmp_295_14);
  *ltmp_293_1025 = ((unsigned char *)0);
  *(&ltmp_285_1023->field23.field1) = 0u;
  ltmp_299_102E = &ltmp_285_1023->field20;
  ltmp_300_102F = *(&ltmp_285_1023->field20.field4);
  if ((ltmp_300_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_315_D;
  } else {
    ltmp_303_102E_2E = ltmp_299_102E;
    goto ltmp_316_D;
  }

ltmp_312_D:
  *ltmp_293_1025 = ((unsigned char *)0);
  *(&ltmp_285_1023->field23.field1) = 0u;
  ltmp_301_102E = &ltmp_285_1023->field20;
  ltmp_302_102F = *(&ltmp_285_1023->field20.field4);
  if ((ltmp_302_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_315_D;
  } else {
    ltmp_303_102E_2E = ltmp_301_102E;
    goto ltmp_316_D;
  }

ltmp_316_D:
  ltmp_303_102E = ltmp_303_102E_2E;
  ltmp_304_102F = *(&ltmp_303_102E->field4);
  ltmp_305_1019 = *(&ltmp_304_102F->field2.field1);
  ltmp_306_1030 = &ltmp_303_102E->field3;
  ltmp_307_1031 = *ltmp_306_1030;
  ltmp_305_1019((&ltmp_304_102F->field2), ((signed char *)ltmp_307_1031));
  *ltmp_306_1030 = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  return;
ltmp_315_D:
  return;
}


static struct l_struct_2E_E_3A__3A_A12 *ltmp_6_1006(struct l_struct_2E_E *ltmp_317_1023, plzo_int32_t ltmp_318_8) {

  if (((((plzo_uint32_t )ltmp_318_8) + 4294967295u) > 9u)) {
    goto ltmp_319_D;
  } else {
    goto ltmp_320_D;
  }

ltmp_320_D:
  return (&ltmp_71_1022[(ltmp_318_8 + -1)]);
ltmp_319_D:
  return ((struct l_struct_2E_E_3A__3A_A12 *)0);
}


static void ltmp_7_1005(struct l_struct_2E_E *ltmp_321_1023) {
  plzo_uint32_t *ltmp_322_17;
  plzo_uint32_t ltmp_323_7;
  plzo_uint32_t ltmp_324_7;
  plzo_uint32_t ltmp_324_7_2E;
  plzo_uint32_t ltmp_325_7;
  plzo_uint32_t ltmp_325_7_2E;
  plzo_uint32_t ltmp_326_7;
  plzo_uint32_t ltmp_327_7;
  plzo_uint32_t *ltmp_328_17;
  plzo_uint32_t ltmp_329_7;
  plzo_uint32_t ltmp_330_7;
  plzo_uint32_t ltmp_331_7;
  plzo_uint32_t ltmp_331_7_2E;

  ltmp_322_17 = &ltmp_321_1023->field5;
  ltmp_323_7 = *(&ltmp_321_1023->field7.field3);
  *ltmp_322_17 = ltmp_323_7;
  ltmp_324_7_2E = ltmp_323_7;
  ltmp_325_7_2E = 0u;
  goto ltmp_332_D;

  do {
ltmp_332_D:
  ltmp_324_7 = ltmp_324_7_2E;
  ltmp_325_7 = ltmp_325_7_2E;
  if ((ltmp_323_7 > ltmp_325_7)) {
    goto ltmp_333_D;
  } else {
    goto ltmp_334_D;
  }

ltmp_333_D:
  *(&ltmp_321_1023->field4[ltmp_325_7]) = 0u;
  ltmp_326_7 = ltmp_325_7 + 1u;
  if ((ltmp_326_7 > 16u)) {
    ltmp_331_7_2E = ltmp_324_7;
    goto ltmp_335_D;
  } else {
    ltmp_324_7_2E = ltmp_324_7;
    ltmp_325_7_2E = ltmp_326_7;
    goto ltmp_332_D;
  }

ltmp_336_D:
  *ltmp_328_17 = (ltmp_327_7 + 1u);
  ltmp_329_7 = ltmp_325_7 + 1u;
  if ((ltmp_329_7 > 16u)) {
    ltmp_331_7_2E = ltmp_325_7;
    goto ltmp_335_D;
  } else {
    ltmp_324_7_2E = ltmp_325_7;
    ltmp_325_7_2E = ltmp_329_7;
    goto ltmp_332_D;
  }

ltmp_334_D:
  ltmp_327_7 = *(&ltmp_321_1023->field6.field7[ltmp_325_7]);
  ltmp_328_17 = &ltmp_321_1023->field4[ltmp_325_7];
  if ((ltmp_327_7 == 0u)) {
    goto ltmp_337_D;
  } else {
    goto ltmp_336_D;
  }

ltmp_337_D:
  *ltmp_328_17 = 4294967295u;
  ltmp_330_7 = ltmp_325_7 + 1u;
  if ((ltmp_330_7 > 16u)) {
    ltmp_331_7_2E = ltmp_324_7;
    goto ltmp_335_D;
  } else {
    ltmp_324_7_2E = ltmp_324_7;
    ltmp_325_7_2E = ltmp_330_7;
    goto ltmp_332_D;
  }

  } while (1);
ltmp_335_D:
  ltmp_331_7 = ltmp_331_7_2E;
  *ltmp_322_17 = ltmp_331_7;
  return;
}


static void ltmp_8_1005(struct l_struct_2E_E *ltmp_338_1023) {

  return;
}


static plzo_int32_t ltmp_9_1007(struct l_struct_2E_E *ltmp_339_1023) {
  struct l_struct_2E_M ltmp_340_1032;
  struct l_struct_2E_M ltmp_341_1032;
  void  (**ltmp_342_1033) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (*ltmp_343_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  plzo_uint32_t ltmp_344_7;
  plzo_uint32_t ltmp_345_7;
  void  (*ltmp_346_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (**ltmp_347_1033) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (*ltmp_348_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (*ltmp_349_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  plzo_int32_t  (***ltmp_350_102C) ();
  plzo_int32_t  (**ltmp_351_1029) ();
  plzo_int32_t  (*ltmp_352_102A) ();
  plzo_int32_t ltmp_353_8;
  plzo_uint32_t *ltmp_354_17;
  plzo_uint32_t ltmp_355_7;
  plzo_int32_t  (**ltmp_356_1029) ();
  plzo_int32_t  (*ltmp_357_102A) ();
  plzo_uint32_t ltmp_358_7;
  plzo_int32_t ltmp_359_8;
  plzo_int32_t ltmp_359_8_2E;
  struct l_struct_2E_A8 *ltmp_360_1034;
  unsigned char *ltmp_361_13;
  plzo_uint32_t ltmp_362_7;
  plzo_uint32_t ltmp_362_7_2E;
  plzo_uint32_t ltmp_363_7;
  struct l_struct_2E_A9 **ltmp_364_1035;
  struct l_struct_2E_A9 *ltmp_365_102F;
  unsigned char *ltmp_366_13;
  plzo_uint32_t *ltmp_367_17;
  plzo_uint32_t ltmp_368_7;
  signed char * (*ltmp_369_1018) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t );
  signed char *ltmp_370_14;
  struct l_struct_2E_A6 *ltmp_371_1036;
  unsigned char *ltmp_372_13;
  plzo_uint32_t ltmp_373_7;
  plzo_uint32_t ltmp_373_7_2E;
  plzo_uint32_t ltmp_374_7;
  plzo_uint32_t *ltmp_375_17;
  plzo_uint32_t *ltmp_376_17;
  plzo_uint32_t ltmp_377_7;
  plzo_int32_t ltmp_378_8;
  plzo_int32_t ltmp_378_8_2E;
  plzo_uint32_t *ltmp_379_17;
  plzo_uint32_t ltmp_380_7;
  plzo_uint32_t ltmp_381_7;
  plzo_uint32_t ltmp_382_7;
  plzo_uint32_t ltmp_383_7;
  plzo_uint32_t ltmp_384_7;
  plzo_uint32_t ltmp_385_7;
  struct l_struct_2E_A6 *ltmp_386_1036;
  plzo_uint32_t ltmp_387_7;
  plzo_uint32_t ltmp_388_7;
  plzo_uint32_t ltmp_389_7;
  plzo_uint32_t ltmp_389_7_2E;
  plzo_uint32_t ltmp_390_7;
  plzo_uint32_t *ltmp_391_17;
  plzo_uint32_t *ltmp_392_17;
  plzo_uint32_t *ltmp_393_17;
  plzo_uint32_t *ltmp_394_17;
  plzo_uint32_t *ltmp_395_17;
  plzo_uint32_t ltmp_396_7;
  plzo_uint32_t ltmp_397_7;
  plzo_uint32_t ltmp_398_7;
  plzo_uint32_t ltmp_399_7;
  plzo_uint32_t ltmp_399_7_2E;
  plzo_uint32_t *ltmp_400_17;
  plzo_uint32_t ltmp_401_7;
  plzo_uint32_t ltmp_402_7;
  struct l_struct_2E_A7 **ltmp_403_1037;
  struct l_struct_2E_A9 *ltmp_404_102F;
  signed char * (*ltmp_405_1018) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t );
  plzo_uint32_t ltmp_406_7;
  signed char *ltmp_407_14;
  plzo_uint32_t *ltmp_408_17;
  plzo_uint32_t ltmp_409_7;
  plzo_uint32_t ltmp_410_7;
  struct l_struct_2E_A9 *ltmp_411_102F;
  signed char * (*ltmp_412_1018) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t );
  signed char *ltmp_413_14;
  struct l_struct_2E_A9 *ltmp_414_102F;
  void  (*ltmp_415_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A7 *ltmp_416_1038;
  plzo_uint32_t ltmp_417_7;
  plzo_uint32_t ltmp_418_7;
  plzo_int32_t *ltmp_419_18;
  plzo_uint32_t ltmp_420_7;
  plzo_uint32_t ltmp_421_7;
  unsigned char *ltmp_422_13;
  plzo_uint32_t ltmp_423_7;
  plzo_uint32_t ltmp_423_7_2E;
  plzo_uint32_t ltmp_424_7;
  unsigned char *ltmp_425_13;
  plzo_uint32_t ltmp_426_7;
  plzo_uint32_t ltmp_426_7_2E;
  plzo_uint32_t ltmp_427_7;
  struct l_struct_2E_A7 **ltmp_428_1037;
  struct l_struct_2E_A7 *ltmp_429_1038;
  unsigned char *ltmp_430_13;
  plzo_uint32_t ltmp_431_7;
  plzo_uint32_t ltmp_431_7_2E;
  plzo_uint32_t ltmp_432_7;
  struct l_struct_2E_A7 *ltmp_433_1038;
  plzo_uint32_t ltmp_434_7;
  unsigned char *ltmp_435_13;
  plzo_uint32_t ltmp_436_7;
  plzo_uint32_t ltmp_436_7_2E;
  plzo_uint32_t ltmp_437_7;
  plzo_int32_t ltmp_438_8;
  plzo_int32_t ltmp_438_8_2E;
  plzo_uint32_t ltmp_439_7;
  plzo_int32_t ltmp_440_8;
  plzo_int32_t ltmp_440_8_2E;
  struct l_struct_2E_A9 **ltmp_441_1035;
  signed char **ltmp_442_102D;
  plzo_uint32_t ltmp_443_7;
  plzo_uint32_t ltmp_444_7;
  plzo_uint32_t ltmp_445_7;
  signed char * (*ltmp_446_1018) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t );
  signed char *ltmp_447_14;
  plzo_int32_t ltmp_448_8;
  signed char *ltmp_449_14;
  signed char *ltmp_450_14;
  plzo_int32_t ltmp_451_8;
  plzo_int32_t ltmp_451_8_2E;
  signed char *ltmp_452_14;
  signed char *ltmp_452_14_2E;
  struct plzo_callback_t *ltmp_453_1026;
  void  (*ltmp_454_101A) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );
  plzo_int32_t ltmp_455_8;
  plzo_int32_t ltmp_455_8_2E;
  plzo_int32_t ltmp_456_8;
  plzo_int32_t ltmp_456_8_2E;
  plzo_uint32_t *ltmp_457_17;
  unsigned char **ltmp_458_1025;
  unsigned char **ltmp_459_1025;
  plzo_uint32_t *ltmp_460_17;
  plzo_uint32_t *ltmp_461_17;
  struct l_struct_2E_A6 **ltmp_462_1039;
  plzo_uint32_t *ltmp_463_17;
  struct l_struct_2E_A7 **ltmp_464_1037;
  struct l_struct_2E_M **ltmp_465_103A;
  struct l_struct_2E_M **ltmp_466_103A;
  struct l_struct_2E_M **ltmp_467_103A;
  struct l_struct_2E_M **ltmp_468_103A;
  plzo_uint32_t ltmp_469_7;
  plzo_uint32_t ltmp_469_7_2E;
  plzo_int32_t ltmp_470_8;
  plzo_int32_t ltmp_470_8_2E;
  plzo_uint32_t ltmp_471_7;
  struct l_struct_2E_A6 *ltmp_472_1036;
  plzo_uint32_t ltmp_473_7;
  plzo_uint32_t ltmp_474_7;
  plzo_uint32_t ltmp_475_7;
  plzo_uint32_t ltmp_476_7;
  plzo_uint32_t *ltmp_477_17;
  struct l_struct_2E_A7 *ltmp_478_1038;
  plzo_uint32_t *ltmp_479_17;
  plzo_uint32_t ltmp_480_7;
  plzo_uint32_t ltmp_481_7;
  plzo_uint32_t ltmp_482_7;
  plzo_uint32_t ltmp_483_7;
  plzo_uint32_t ltmp_484_7;
  unsigned char *ltmp_485_13;
  unsigned char *ltmp_486_13;
  plzo_uint32_t ltmp_487_7;
  plzo_uint32_t ltmp_487_7_2E;
  unsigned char ltmp_488_3;
  plzo_uint32_t ltmp_489_7;
  plzo_uint32_t ltmp_490_7;
  plzo_uint32_t ltmp_490_7_2E;
  unsigned char ltmp_491_3;
  plzo_uint32_t ltmp_492_7;
  struct l_struct_2E_A7 *ltmp_493_1038;
  plzo_uint32_t ltmp_494_7;
  plzo_uint32_t ltmp_495_7;
  plzo_uint32_t ltmp_495_7_2E;
  plzo_uint32_t ltmp_496_7;
  plzo_uint32_t ltmp_496_7_2E;
  struct l_struct_2E_A7 *ltmp_497_1038;
  struct l_struct_2E_A7 *ltmp_497_1038_2E;
  plzo_uint32_t *ltmp_498_17;
  struct l_struct_2E_A6 *ltmp_499_1036;
  plzo_uint32_t ltmp_500_7;
  plzo_uint32_t *ltmp_501_17;
  plzo_uint32_t ltmp_502_7;
  plzo_uint32_t *ltmp_503_17;
  plzo_uint32_t *ltmp_504_17;
  unsigned char **ltmp_505_1025;
  plzo_uint32_t *ltmp_506_17;
  plzo_uint32_t *ltmp_507_17;
  unsigned char **ltmp_508_1025;
  unsigned char **ltmp_509_1025;
  plzo_int32_t *ltmp_510_18;
  plzo_uint32_t *ltmp_511_17;
  plzo_uint32_t *ltmp_512_17;
  plzo_uint32_t *ltmp_513_17;
  struct l_struct_2E_M **ltmp_514_103A;
  plzo_uint32_t *ltmp_515_17;
  plzo_uint32_t *ltmp_516_17;
  plzo_uint32_t *ltmp_517_17;
  plzo_uint32_t *ltmp_518_17;
  plzo_uint32_t ltmp_519_7;
  plzo_uint32_t ltmp_519_7_2E;
  struct l_struct_2E_A7 *ltmp_520_1038;
  struct l_struct_2E_A7 *ltmp_520_1038_2E;
  plzo_uint32_t ltmp_521_7;
  plzo_uint32_t ltmp_522_7;
  plzo_uint32_t ltmp_523_7;
  plzo_uint32_t ltmp_524_7;
  plzo_uint32_t *ltmp_525_17;
  struct l_struct_2E_M *ltmp_526_1032;
  struct l_struct_2E_M **ltmp_527_103A;
  struct l_struct_2E_M **ltmp_528_103A;
  unsigned char *ltmp_529_13;
  plzo_int32_t ltmp_530_8;
  unsigned char *ltmp_531_13;
  plzo_uint32_t ltmp_532_7;
  unsigned char ltmp_533_3;
  plzo_uint32_t *ltmp_534_17;
  plzo_uint32_t ltmp_535_7;
  plzo_uint32_t ltmp_536_7;
  plzo_uint32_t ltmp_537_7;
  plzo_uint32_t ltmp_538_7;
  plzo_uint32_t ltmp_538_7_2E;
  unsigned char ltmp_539_3;
  unsigned char ltmp_540_3;
  plzo_uint32_t *ltmp_541_17;
  plzo_uint32_t ltmp_542_7;
  plzo_uint32_t ltmp_543_7;
  plzo_uint32_t ltmp_544_7;
  unsigned char *ltmp_545_13;
  plzo_int32_t ltmp_546_8;
  plzo_int32_t ltmp_547_8;
  plzo_uint32_t ltmp_548_7;
  plzo_uint32_t ltmp_549_7;
  plzo_uint32_t ltmp_550_7;
  plzo_uint32_t ltmp_551_7;
  plzo_uint32_t ltmp_552_7;
  plzo_uint32_t ltmp_553_7;
  plzo_uint32_t ltmp_554_7;
  plzo_uint32_t ltmp_554_7_2E;
  struct l_struct_2E_M *ltmp_555_1032;
  struct l_struct_2E_M *ltmp_556_1032;
  struct l_struct_2E_M *ltmp_557_1032;
  struct l_struct_2E_M *ltmp_557_1032_2E;
  struct l_struct_2E_M *ltmp_558_1032;
  plzo_int32_t ltmp_559_8;
  plzo_uint32_t ltmp_560_7;
  plzo_uint32_t ltmp_561_7;
  plzo_uint32_t ltmp_562_7;
  plzo_uint32_t ltmp_563_7;
  plzo_uint32_t ltmp_564_7;
  plzo_uint32_t ltmp_564_7_2E;
  plzo_uint32_t ltmp_565_7;
  unsigned char *ltmp_566_13;
  struct l_struct_2E_M *ltmp_567_1032;
  plzo_int32_t ltmp_568_8;
  plzo_uint32_t ltmp_569_7;
  plzo_uint32_t ltmp_570_7;
  plzo_uint32_t ltmp_571_7;
  plzo_uint32_t ltmp_572_7;
  plzo_uint32_t ltmp_573_7;
  plzo_uint32_t ltmp_573_7_2E;
  plzo_uint32_t ltmp_574_7;
  unsigned char *ltmp_575_13;
  unsigned char *ltmp_576_13;
  plzo_uint32_t ltmp_577_7;
  unsigned char *ltmp_578_13;
  unsigned char *ltmp_578_13_2E;
  plzo_uint32_t ltmp_579_7;
  plzo_uint32_t ltmp_579_7_2E;
  plzo_uint32_t ltmp_580_7;
  plzo_uint32_t ltmp_580_7_2E;
  plzo_uint32_t ltmp_581_7;
  plzo_uint32_t ltmp_581_7_2E;
  struct l_struct_2E_M *ltmp_582_1032;
  struct l_struct_2E_M *ltmp_582_1032_2E;
  struct l_struct_2E_M *ltmp_583_1032;
  struct l_struct_2E_M *ltmp_583_1032_2E;
  struct l_struct_2E_M *ltmp_584_1032;
  struct l_struct_2E_M *ltmp_584_1032_2E;
  plzo_uint32_t ltmp_585_7;
  unsigned char *ltmp_586_13;
  unsigned char *ltmp_586_13_2E;
  plzo_uint32_t ltmp_587_7;
  plzo_uint32_t ltmp_587_7_2E;
  plzo_uint32_t ltmp_588_7;
  plzo_uint32_t ltmp_588_7_2E;
  plzo_uint32_t ltmp_589_7;
  plzo_uint32_t ltmp_589_7_2E;
  struct l_struct_2E_M *ltmp_590_1032;
  struct l_struct_2E_M *ltmp_590_1032_2E;
  struct l_struct_2E_M *ltmp_591_1032;
  struct l_struct_2E_M *ltmp_591_1032_2E;
  struct l_struct_2E_M *ltmp_592_1032;
  struct l_struct_2E_M *ltmp_592_1032_2E;
  plzo_uint32_t ltmp_593_7;
  plzo_uint32_t ltmp_593_7_2E;
  plzo_uint32_t ltmp_594_7;
  plzo_uint32_t ltmp_594_7_2E;
  plzo_uint32_t ltmp_595_7;
  unsigned char ltmp_596_3;
  unsigned char ltmp_597_3;
  plzo_uint32_t ltmp_598_7;
  plzo_uint32_t ltmp_599_7;
  plzo_uint32_t ltmp_600_7;
  plzo_uint32_t ltmp_600_7_2E;
  plzo_int32_t ltmp_601_8;
  plzo_uint32_t ltmp_602_7;
  plzo_uint32_t ltmp_603_7;
  plzo_uint32_t ltmp_603_7_2E;
  plzo_uint32_t ltmp_604_7;
  plzo_uint32_t ltmp_605_7;
  plzo_uint32_t ltmp_606_7;
  plzo_uint32_t ltmp_606_7_2E;
  plzo_uint32_t ltmp_607_7;
  plzo_uint32_t ltmp_607_7_2E;
  struct l_struct_2E_M *ltmp_608_1032;
  struct l_struct_2E_M *ltmp_609_1032;
  plzo_int32_t ltmp_610_8;
  plzo_uint32_t ltmp_611_7;
  plzo_uint32_t ltmp_612_7;
  plzo_uint32_t ltmp_613_7;
  plzo_uint32_t ltmp_614_7;
  plzo_uint32_t ltmp_615_7;
  plzo_uint32_t ltmp_615_7_2E;
  plzo_uint32_t ltmp_616_7;
  unsigned char *ltmp_617_13;
  unsigned char *ltmp_618_13;
  plzo_uint32_t ltmp_619_7;
  plzo_uint32_t ltmp_620_7;
  plzo_uint32_t ltmp_620_7_2E;
  plzo_uint32_t ltmp_621_7;
  plzo_uint32_t ltmp_622_7;
  plzo_uint32_t ltmp_623_7;
  plzo_uint32_t ltmp_623_7_2E;
  plzo_uint32_t ltmp_624_7;
  plzo_uint32_t ltmp_624_7_2E;
  struct l_struct_2E_M *ltmp_625_1032;
  struct l_struct_2E_M *ltmp_626_1032;
  plzo_int32_t ltmp_627_8;
  plzo_uint32_t ltmp_628_7;
  plzo_uint32_t ltmp_629_7;
  plzo_uint32_t ltmp_630_7;
  plzo_uint32_t ltmp_631_7;
  plzo_uint32_t ltmp_632_7;
  plzo_uint32_t ltmp_632_7_2E;
  plzo_uint32_t ltmp_633_7;
  unsigned char *ltmp_634_13;
  unsigned char *ltmp_635_13;
  plzo_uint32_t ltmp_636_7;
  plzo_uint32_t ltmp_637_7;
  plzo_uint32_t ltmp_637_7_2E;
  plzo_uint32_t ltmp_638_7;
  struct l_struct_2E_M **ltmp_639_103A;
  struct l_struct_2E_M *ltmp_640_1032;
  struct l_struct_2E_M *ltmp_641_1032;
  plzo_int32_t ltmp_642_8;
  plzo_uint32_t ltmp_643_7;
  plzo_uint32_t ltmp_644_7;
  plzo_uint32_t ltmp_645_7;
  plzo_uint32_t ltmp_646_7;
  plzo_uint32_t ltmp_647_7;
  plzo_uint32_t ltmp_647_7_2E;
  plzo_uint32_t ltmp_648_7;
  unsigned char *ltmp_649_13;
  struct l_struct_2E_M *ltmp_650_1032;
  struct l_struct_2E_M *ltmp_651_1032;
  struct l_struct_2E_M **ltmp_652_103A;
  struct l_struct_2E_M *ltmp_653_1032;
  struct l_struct_2E_M *ltmp_654_1032;
  plzo_int32_t ltmp_655_8;
  plzo_uint32_t ltmp_656_7;
  plzo_uint32_t ltmp_657_7;
  plzo_uint32_t ltmp_658_7;
  plzo_uint32_t ltmp_659_7;
  plzo_uint32_t ltmp_660_7;
  plzo_uint32_t ltmp_660_7_2E;
  plzo_uint32_t ltmp_661_7;
  unsigned char *ltmp_662_13;
  struct l_struct_2E_M *ltmp_663_1032;
  struct l_struct_2E_M *ltmp_664_1032;
  plzo_uint32_t ltmp_665_7;
  plzo_uint32_t ltmp_665_7_2E;
  struct l_struct_2E_M *ltmp_666_1032;
  struct l_struct_2E_M *ltmp_667_1032;
  struct l_struct_2E_M *ltmp_668_1032;
  struct l_struct_2E_M *ltmp_669_1032;
  struct l_struct_2E_M *ltmp_670_1032;
  plzo_int32_t ltmp_671_8;
  unsigned char *ltmp_672_13;
  struct l_struct_2E_M *ltmp_673_1032;
  plzo_int32_t ltmp_674_8;
  unsigned char *ltmp_675_13;
  unsigned char *ltmp_676_13;
  unsigned char *ltmp_677_13;
  unsigned char *ltmp_677_13_2E;
  plzo_uint32_t ltmp_678_7;
  plzo_uint32_t ltmp_678_7_2E;
  plzo_uint32_t ltmp_679_7;
  plzo_uint32_t ltmp_679_7_2E;
  plzo_uint32_t ltmp_680_7;
  plzo_uint32_t ltmp_680_7_2E;
  struct l_struct_2E_M *ltmp_681_1032;
  struct l_struct_2E_M *ltmp_681_1032_2E;
  struct l_struct_2E_M *ltmp_682_1032;
  struct l_struct_2E_M *ltmp_682_1032_2E;
  struct l_struct_2E_M *ltmp_683_1032;
  struct l_struct_2E_M *ltmp_683_1032_2E;
  plzo_uint32_t ltmp_684_7;
  unsigned char *ltmp_685_13;
  unsigned char *ltmp_685_13_2E;
  plzo_uint32_t ltmp_686_7;
  plzo_uint32_t ltmp_686_7_2E;
  plzo_uint32_t ltmp_687_7;
  plzo_uint32_t ltmp_687_7_2E;
  plzo_uint32_t ltmp_688_7;
  plzo_uint32_t ltmp_688_7_2E;
  struct l_struct_2E_M *ltmp_689_1032;
  struct l_struct_2E_M *ltmp_689_1032_2E;
  struct l_struct_2E_M *ltmp_690_1032;
  struct l_struct_2E_M *ltmp_690_1032_2E;
  struct l_struct_2E_M *ltmp_691_1032;
  struct l_struct_2E_M *ltmp_691_1032_2E;
  plzo_uint32_t ltmp_692_7;
  plzo_uint32_t ltmp_692_7_2E;
  plzo_uint32_t ltmp_693_7;
  plzo_uint32_t ltmp_693_7_2E;
  plzo_uint32_t ltmp_694_7;
  unsigned char ltmp_695_3;
  unsigned char ltmp_696_3;
  plzo_uint32_t ltmp_697_7;
  plzo_uint32_t ltmp_698_7;
  plzo_uint32_t ltmp_699_7;
  plzo_uint32_t ltmp_699_7_2E;
  plzo_int32_t ltmp_700_8;
  plzo_uint32_t ltmp_701_7;
  plzo_uint32_t ltmp_702_7;
  plzo_uint32_t ltmp_702_7_2E;
  plzo_uint32_t ltmp_703_7;
  plzo_uint32_t ltmp_704_7;
  plzo_uint32_t ltmp_705_7;
  plzo_uint32_t ltmp_705_7_2E;
  plzo_uint32_t ltmp_706_7;
  plzo_uint32_t ltmp_706_7_2E;
  struct l_struct_2E_M *ltmp_707_1032;
  struct l_struct_2E_M *ltmp_708_1032;
  plzo_int32_t ltmp_709_8;
  unsigned char *ltmp_710_13;
  unsigned char *ltmp_711_13;
  plzo_uint32_t ltmp_712_7;
  plzo_uint32_t ltmp_713_7;
  plzo_uint32_t ltmp_713_7_2E;
  plzo_uint32_t ltmp_714_7;
  plzo_uint32_t ltmp_715_7;
  plzo_uint32_t ltmp_716_7;
  plzo_uint32_t ltmp_716_7_2E;
  plzo_uint32_t ltmp_717_7;
  plzo_uint32_t ltmp_717_7_2E;
  struct l_struct_2E_M *ltmp_718_1032;
  struct l_struct_2E_M *ltmp_719_1032;
  plzo_int32_t ltmp_720_8;
  unsigned char *ltmp_721_13;
  unsigned char *ltmp_722_13;
  plzo_uint32_t ltmp_723_7;
  plzo_uint32_t ltmp_724_7;
  plzo_uint32_t ltmp_724_7_2E;
  plzo_uint32_t ltmp_725_7;
  struct l_struct_2E_M *ltmp_726_1032;
  struct l_struct_2E_M *ltmp_727_1032;
  plzo_uint32_t ltmp_728_7;
  plzo_uint32_t ltmp_728_7_2E;
  struct l_struct_2E_M *ltmp_729_1032;
  struct l_struct_2E_M *ltmp_730_1032;
  plzo_uint32_t ltmp_731_7;
  plzo_uint32_t ltmp_731_7_2E;
  plzo_uint32_t ltmp_732_7;
  plzo_uint32_t ltmp_733_7;
  plzo_uint32_t ltmp_734_7;
  unsigned char *ltmp_735_13;
  unsigned char *ltmp_736_13;
  plzo_uint32_t ltmp_737_7;
  plzo_uint32_t ltmp_738_7;
  plzo_uint32_t ltmp_739_7;
  plzo_uint32_t ltmp_739_7_2E;
  plzo_uint32_t ltmp_740_7;
  plzo_uint32_t ltmp_740_7_2E;
  plzo_uint32_t ltmp_741_7;
  plzo_uint32_t ltmp_741_7_2E;
  plzo_uint32_t ltmp_742_7;
  unsigned char *ltmp_743_13;
  plzo_int32_t ltmp_744_8;
  plzo_uint32_t ltmp_745_7;
  plzo_uint32_t ltmp_746_7;
  plzo_uint32_t ltmp_747_7;
  plzo_int32_t ltmp_748_8;
  plzo_uint32_t ltmp_749_7;
  plzo_uint32_t ltmp_750_7;
  plzo_uint32_t ltmp_751_7;
  plzo_uint32_t ltmp_752_7;
  plzo_uint32_t ltmp_752_7_2E;
  plzo_uint32_t ltmp_753_7;
  plzo_int32_t ltmp_754_8;
  unsigned char ltmp_755_3;
  unsigned char ltmp_756_3;
  plzo_uint32_t ltmp_757_7;
  plzo_uint32_t ltmp_758_7;
  plzo_uint32_t ltmp_759_7;
  plzo_uint32_t ltmp_759_7_2E;
  plzo_uint32_t ltmp_760_7;
  plzo_uint32_t ltmp_761_7;
  plzo_uint32_t ltmp_761_7_2E;
  plzo_uint32_t ltmp_762_7;
  plzo_uint32_t ltmp_762_7_2E;
  plzo_uint32_t ltmp_763_7;
  plzo_uint32_t ltmp_764_7;
  plzo_uint32_t ltmp_765_7;
  plzo_uint32_t ltmp_765_7_2E;
  plzo_uint32_t ltmp_766_7;
  plzo_uint32_t ltmp_766_7_2E;
  plzo_uint32_t ltmp_767_7;
  struct l_struct_2E_M *ltmp_768_1032;
  plzo_uint32_t ltmp_769_7;
  plzo_uint32_t ltmp_770_7;
  plzo_uint32_t ltmp_771_7;
  plzo_uint32_t ltmp_772_7;
  plzo_uint32_t ltmp_773_7;
  plzo_uint32_t ltmp_773_7_2E;
  plzo_int32_t ltmp_774_8;
  struct l_struct_2E_M *ltmp_775_1032;
  plzo_uint32_t ltmp_776_7;
  struct l_struct_2E_A7 *ltmp_777_1038;
  plzo_uint32_t ltmp_778_7;
  plzo_uint32_t ltmp_779_7;
  plzo_uint32_t ltmp_779_7_2E;
  struct l_struct_2E_A7 *ltmp_780_1038;
  struct l_struct_2E_A7 *ltmp_780_1038_2E;
  plzo_uint32_t ltmp_781_7;
  struct l_struct_2E_A7 *ltmp_782_1038;
  plzo_uint32_t ltmp_783_7;
  plzo_uint32_t ltmp_784_7;
  plzo_uint32_t ltmp_785_7;
  plzo_uint32_t ltmp_786_7;
  plzo_uint32_t ltmp_787_7;
  plzo_int32_t  (**ltmp_788_1029) ();
  plzo_int32_t  (*ltmp_789_102A) ();
  plzo_uint32_t ltmp_790_7;
  plzo_int32_t  (**ltmp_791_1029) ();
  plzo_int32_t  (*ltmp_792_102A) ();
  plzo_uint32_t ltmp_793_7;
  plzo_uint32_t ltmp_794_7;
  plzo_uint32_t ltmp_794_7_2E;
  unsigned char *ltmp_795_13;
  unsigned char *ltmp_796_13;
  plzo_int32_t ltmp_797_8;
  plzo_uint32_t ltmp_798_7;
  signed char *ltmp_799_14;
  struct plzo_callback_t *ltmp_800_1026;
  void  (*ltmp_801_101A) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );
  plzo_int32_t  (**ltmp_802_1029) ();
  plzo_int32_t  (*ltmp_803_102A) ();
  plzo_uint32_t ltmp_804_7;
  plzo_int32_t  (*ltmp_805_102A) ();
  plzo_uint32_t ltmp_806_7;
  plzo_int32_t  (**ltmp_807_1029) ();
  plzo_int32_t  (*ltmp_808_102A) ();
  plzo_uint32_t ltmp_809_7;
  plzo_int32_t  (*ltmp_810_102A) ();
  plzo_uint32_t ltmp_811_7;
  unsigned char *ltmp_812_13;
  unsigned char *ltmp_813_13;
  plzo_int32_t ltmp_814_8;
  plzo_uint32_t ltmp_815_7;
  signed char *ltmp_816_14;
  struct plzo_callback_t *ltmp_817_1026;
  void  (*ltmp_818_101A) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );
  plzo_int32_t  (**ltmp_819_1029) ();
  plzo_int32_t  (*ltmp_820_102A) ();
  plzo_int32_t  (**ltmp_821_1029) ();
  plzo_int32_t  (*ltmp_822_102A) ();
  plzo_int32_t  (*ltmp_823_102A) ();
  plzo_uint32_t ltmp_824_7;
  plzo_uint32_t ltmp_825_7;
  unsigned char *ltmp_826_13;
  unsigned char *ltmp_827_13;
  plzo_int32_t ltmp_828_8;
  plzo_uint32_t ltmp_829_7;
  signed char *ltmp_830_14;
  struct plzo_callback_t *ltmp_831_1026;
  void  (*ltmp_832_101A) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );
  plzo_int32_t ltmp_833_8;
  plzo_int32_t ltmp_833_8_2E;
  struct l_struct_2E_A9 *ltmp_834_102F;
  void  (*ltmp_835_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_836_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_837_1031;

  ltmp_342_1033 = &ltmp_339_1023->field22.field3;
  ltmp_343_1001 = *ltmp_342_1033;
  ltmp_344_7 = *(&ltmp_339_1023->field1.field2);
  ltmp_345_7 = *(&ltmp_339_1023->field1.field3);
  ltmp_346_1001 = ltmp_54_101D(ltmp_343_1001, ltmp_344_7, ltmp_345_7);
  *ltmp_342_1033 = ltmp_346_1001;
  ltmp_347_1033 = &ltmp_339_1023->field23.field3;
  ltmp_348_1001 = *ltmp_347_1033;
  ltmp_349_1001 = ltmp_54_101D(ltmp_348_1001, ltmp_344_7, ltmp_345_7);
  *ltmp_347_1033 = ltmp_349_1001;
  ltmp_350_102C = &ltmp_339_1023->field0;
  ltmp_351_1029 = *ltmp_350_102C;
  ltmp_352_102A = *(&ltmp_351_1029[6]);
  ((void  (*) (struct l_struct_2E_E *))ltmp_352_102A)(ltmp_339_1023);
  ltmp_353_8 = *(&ltmp_339_1023->field7.field11);
  ltmp_354_17 = &ltmp_339_1023->field21;
  ltmp_355_7 = *ltmp_354_17;
  if ((ltmp_355_7 == 1u)) {
    goto ltmp_838_D;
  } else {
    ltmp_359_8_2E = 3;
    goto ltmp_839_D;
  }

ltmp_838_D:
  if ((ltmp_353_8 == 2)) {
    goto ltmp_840_D;
  } else {
    ltmp_359_8_2E = ltmp_353_8;
    goto ltmp_839_D;
  }

ltmp_840_D:
  ltmp_356_1029 = *ltmp_350_102C;
  ltmp_357_102A = *(&ltmp_356_1029[18]);
  ltmp_358_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *))ltmp_357_102A)(ltmp_339_1023);
  if ((ltmp_358_7 == 0u)) {
    goto ltmp_841_D;
  } else {
    ltmp_359_8_2E = ltmp_353_8;
    goto ltmp_839_D;
  }

ltmp_841_D:
  ltmp_359_8_2E = 3;
  goto ltmp_839_D;

ltmp_839_D:
  ltmp_359_8 = ltmp_359_8_2E;
  ltmp_360_1034 = &ltmp_339_1023->field19;
  ltmp_361_13 = (unsigned char *)ltmp_360_1034;
  ltmp_362_7_2E = 0u;
  goto ltmp_842_D;

  do {
ltmp_842_D:
  ltmp_362_7 = ltmp_362_7_2E;
  *(&ltmp_361_13[ltmp_362_7]) = 0;
  ltmp_363_7 = ltmp_362_7 + 1u;
  if ((ltmp_363_7 == 32u)) {
    goto ltmp_843_D;
  } else {
    ltmp_362_7_2E = ltmp_363_7;
    goto ltmp_842_D;
  }

  } while (1);
ltmp_843_D:
  ltmp_364_1035 = &ltmp_339_1023->field19.field6;
  ltmp_365_102F = &ltmp_339_1023->field24;
  *ltmp_364_1035 = ltmp_365_102F;
  ltmp_366_13 = *(&ltmp_339_1023->field8);
  ltmp_367_17 = &ltmp_339_1023->field11;
  ltmp_368_7 = *ltmp_367_17;
  if (((ltmp_368_7 + 4294967293u) > 2147483642u)) {
    ltmp_378_8_2E = -2;
    goto ltmp_844_D;
  } else {
    goto ltmp_845_D;
  }

ltmp_845_D:
  ltmp_369_1018 = *(&ltmp_339_1023->field24.field2.field0);
  ltmp_370_14 = ltmp_369_1018((&ltmp_339_1023->field24.field2), 1u, 267368u);
  ltmp_371_1036 = (struct l_struct_2E_A6 *)ltmp_370_14;
  *(&ltmp_339_1023->field19.field7) = ltmp_371_1036;
  if ((ltmp_370_14 == ((signed char *)0))) {
    ltmp_378_8_2E = -3;
    goto ltmp_844_D;
  } else {
    goto ltmp_846_D;
  }

ltmp_846_D:
  ltmp_372_13 = (unsigned char *)ltmp_370_14;
  ltmp_373_7_2E = 0u;
  goto ltmp_847_D;

  do {
ltmp_847_D:
  ltmp_373_7 = ltmp_373_7_2E;
  *(&ltmp_372_13[ltmp_373_7]) = 0;
  ltmp_374_7 = ltmp_373_7 + 1u;
  if ((ltmp_374_7 == 267368u)) {
    goto ltmp_848_D;
  } else {
    ltmp_373_7_2E = ltmp_374_7;
    goto ltmp_847_D;
  }

  } while (1);
ltmp_848_D:
  *(&ltmp_371_1036->field0) = ltmp_366_13;
  *(&ltmp_339_1023->field19.field4) = ltmp_366_13;
  ltmp_375_17 = &ltmp_371_1036->field2;
  *ltmp_375_17 = ltmp_368_7;
  *(&ltmp_339_1023->field19.field5) = ltmp_368_7;
  ltmp_376_17 = &ltmp_371_1036->field3;
  ltmp_377_7 = *ltmp_375_17;
  if ((ltmp_377_7 > 3u)) {
    goto ltmp_849_D;
  } else {
    goto ltmp_850_D;
  }

ltmp_849_D:
  *ltmp_376_17 = (ltmp_377_7 + 4294967293u);
  ltmp_378_8_2E = 0;
  goto ltmp_844_D;

ltmp_850_D:
  *ltmp_376_17 = 0u;
  ltmp_378_8_2E = 0;
  goto ltmp_844_D;

ltmp_844_D:
  ltmp_378_8 = ltmp_378_8_2E;
  if ((ltmp_378_8 == 0)) {
    goto ltmp_851_D;
  } else {
    ltmp_440_8_2E = ltmp_378_8;
    goto ltmp_852_D;
  }

ltmp_851_D:
  ltmp_379_17 = &ltmp_339_1023->field7.field1;
  ltmp_380_7 = *ltmp_379_17;
  ltmp_381_7 = *(&ltmp_339_1023->field7.field2);
  ltmp_382_7 = *(&ltmp_339_1023->field7.field3);
  ltmp_383_7 = *(&ltmp_339_1023->field7.field4);
  ltmp_384_7 = *(&ltmp_339_1023->field7.field5);
  ltmp_385_7 = *(&ltmp_339_1023->field7.field6);
  ltmp_386_1036 = *(&ltmp_339_1023->field19.field7);
  if ((ltmp_381_7 == 0u)) {
    goto ltmp_853_D;
  } else {
    goto ltmp_854_D;
  }

ltmp_854_D:
  ltmp_387_7 = *(&ltmp_386_1036->field2);
  if ((ltmp_387_7 < ltmp_381_7)) {
    goto ltmp_853_D;
  } else {
    ltmp_389_7_2E = ltmp_381_7;
    goto ltmp_855_D;
  }

ltmp_853_D:
  ltmp_388_7 = *(&ltmp_386_1036->field2);
  ltmp_389_7_2E = ltmp_388_7;
  goto ltmp_855_D;

ltmp_855_D:
  ltmp_389_7 = ltmp_389_7_2E;
  ltmp_390_7 = (((ltmp_383_7 == 0u)) ? (4294967295u) : (ltmp_383_7));
  ltmp_391_17 = &ltmp_386_1036->field14;
  *ltmp_391_17 = ltmp_389_7;
  ltmp_392_17 = &ltmp_386_1036->field12;
  *ltmp_392_17 = ltmp_382_7;
  ltmp_393_17 = &ltmp_386_1036->field13;
  *ltmp_393_17 = ltmp_390_7;
  ltmp_394_17 = &ltmp_386_1036->field15;
  ltmp_395_17 = &ltmp_386_1036->field16;
  if ((ltmp_384_7 == 0u)) {
    goto ltmp_856_D;
  } else {
    goto ltmp_857_D;
  }

ltmp_857_D:
  *ltmp_394_17 = ((((ltmp_390_7 <= ltmp_384_7)) ? (ltmp_390_7) : (ltmp_384_7)));
  if ((ltmp_385_7 == 0u)) {
    goto ltmp_858_D;
  } else {
    goto ltmp_859_D;
  }

ltmp_856_D:
  *ltmp_394_17 = 4294967295u;
  if ((ltmp_385_7 == 0u)) {
    goto ltmp_858_D;
  } else {
    goto ltmp_859_D;
  }

ltmp_859_D:
  ltmp_396_7 = (((ltmp_390_7 <= ltmp_385_7)) ? (ltmp_390_7) : (ltmp_385_7));
  *ltmp_395_17 = ltmp_396_7;
  if ((ltmp_396_7 > 1u)) {
    goto ltmp_860_D;
  } else {
    goto ltmp_861_D;
  }

ltmp_858_D:
  *ltmp_395_17 = 4294967295u;
  goto ltmp_860_D;

ltmp_861_D:
  *ltmp_395_17 = 2u;
  if ((ltmp_380_7 == 0u)) {
    goto ltmp_862_D;
  } else {
    goto ltmp_863_D;
  }

ltmp_860_D:
  if ((ltmp_380_7 == 0u)) {
    goto ltmp_862_D;
  } else {
    goto ltmp_863_D;
  }

ltmp_863_D:
  ltmp_397_7 = *(&ltmp_386_1036->field2);
  if ((ltmp_397_7 < ltmp_380_7)) {
    goto ltmp_862_D;
  } else {
    ltmp_399_7_2E = ltmp_380_7;
    goto ltmp_864_D;
  }

ltmp_862_D:
  ltmp_398_7 = *(&ltmp_386_1036->field2);
  ltmp_399_7_2E = ltmp_398_7;
  goto ltmp_864_D;

ltmp_864_D:
  ltmp_399_7 = ltmp_399_7_2E;
  ltmp_400_17 = &ltmp_339_1023->field19.field3;
  *(&ltmp_386_1036->field6) = ltmp_399_7;
  *ltmp_400_17 = ltmp_399_7;
  *(&ltmp_386_1036->field5) = 0u;
  *(&ltmp_339_1023->field19.field1) = 0u;
  *(&ltmp_339_1023->field19.field2) = 0u;
  *(&ltmp_386_1036->field8) = 0u;
  *(&ltmp_386_1036->field9) = 0u;
  *(&ltmp_386_1036->field10) = 0u;
  *(&ltmp_386_1036->field11) = 0u;
  *(&ltmp_386_1036->field25[0]) = ((unsigned char *)0);
  ltmp_401_7 = *ltmp_392_17;
  if ((ltmp_401_7 == 0u)) {
    ltmp_438_8_2E = -2;
    goto ltmp_865_D;
  } else {
    goto ltmp_866_D;
  }

ltmp_866_D:
  ltmp_402_7 = *ltmp_393_17;
  if (((ltmp_401_7 > ltmp_402_7) | (ltmp_402_7 < 4u))) {
    ltmp_438_8_2E = -2;
    goto ltmp_865_D;
  } else {
    goto ltmp_867_D;
  }

ltmp_867_D:
  ltmp_403_1037 = &ltmp_386_1036->field17;
  ltmp_404_102F = *ltmp_364_1035;
  ltmp_405_1018 = *(&ltmp_404_102F->field2.field0);
  ltmp_406_7 = *ltmp_400_17;
  ltmp_407_14 = ltmp_405_1018((&ltmp_404_102F->field2), (ltmp_406_7 + 2u), 36u);
  *ltmp_403_1037 = ((struct l_struct_2E_A7 *)ltmp_407_14);
  if ((ltmp_407_14 == ((signed char *)0))) {
    ltmp_438_8_2E = -3;
    goto ltmp_865_D;
  } else {
    goto ltmp_868_D;
  }

ltmp_868_D:
  ltmp_408_17 = &ltmp_386_1036->field19;
  ltmp_409_7 = *ltmp_391_17;
  ltmp_410_7 = ltmp_409_7 + 1u;
  *ltmp_408_17 = ltmp_410_7;
  ltmp_411_102F = *ltmp_364_1035;
  ltmp_412_1018 = *(&ltmp_411_102F->field2.field0);
  ltmp_413_14 = ltmp_412_1018((&ltmp_411_102F->field2), ltmp_410_7, 8u);
  *(&ltmp_386_1036->field18) = ((struct l_struct_2E_M *)ltmp_413_14);
  if ((ltmp_413_14 == ((signed char *)0))) {
    goto ltmp_869_D;
  } else {
    goto ltmp_870_D;
  }

ltmp_869_D:
  ltmp_414_102F = *ltmp_364_1035;
  ltmp_415_1019 = *(&ltmp_414_102F->field2.field1);
  ltmp_416_1038 = *ltmp_403_1037;
  ltmp_415_1019((&ltmp_414_102F->field2), ((signed char *)ltmp_416_1038));
  *ltmp_403_1037 = ((struct l_struct_2E_A7 *)0);
  ltmp_438_8_2E = -3;
  goto ltmp_865_D;

ltmp_870_D:
  ltmp_417_7 = *ltmp_408_17;
  ltmp_418_7 = ltmp_417_7 + 4294967295u;
  *(&ltmp_386_1036->field20) = ltmp_418_7;
  *(&ltmp_386_1036->field21) = ltmp_418_7;
  *(&ltmp_386_1036->field1) = ltmp_418_7;
  ltmp_419_18 = &ltmp_386_1036->field22;
  *ltmp_419_18 = 1;
  ltmp_420_7 = *ltmp_391_17;
  ltmp_421_7 = *(&ltmp_386_1036->field2);
  if ((ltmp_420_7 == ltmp_421_7)) {
    goto ltmp_871_D;
  } else {
    goto ltmp_872_D;
  }

ltmp_871_D:
  *ltmp_419_18 = 0;
  goto ltmp_872_D;

ltmp_872_D:
  ltmp_422_13 = (unsigned char *)(&ltmp_386_1036->field26);
  ltmp_423_7_2E = 0u;
  goto ltmp_873_D;

  do {
ltmp_873_D:
  ltmp_423_7 = ltmp_423_7_2E;
  *(&ltmp_422_13[ltmp_423_7]) = 255;
  ltmp_424_7 = ltmp_423_7 + 1u;
  if ((ltmp_424_7 == 262144u)) {
    goto ltmp_874_D;
  } else {
    ltmp_423_7_2E = ltmp_424_7;
    goto ltmp_873_D;
  }

  } while (1);
ltmp_874_D:
  ltmp_425_13 = (unsigned char *)(&ltmp_386_1036->field27);
  ltmp_426_7_2E = 0u;
  goto ltmp_875_D;

  do {
ltmp_875_D:
  ltmp_426_7 = ltmp_426_7_2E;
  *(&ltmp_425_13[ltmp_426_7]) = 255;
  ltmp_427_7 = ltmp_426_7 + 1u;
  if ((ltmp_427_7 == 1024u)) {
    goto ltmp_876_D;
  } else {
    ltmp_426_7_2E = ltmp_427_7;
    goto ltmp_875_D;
  }

  } while (1);
ltmp_876_D:
  *(&ltmp_386_1036->field23) = 0u;
  *(&ltmp_386_1036->field24) = 0u;
  ltmp_428_1037 = &ltmp_339_1023->field19.field0;
  ltmp_429_1038 = *ltmp_403_1037;
  *ltmp_428_1037 = (&ltmp_429_1038[1]);
  ltmp_430_13 = (unsigned char *)ltmp_429_1038;
  ltmp_431_7_2E = 0u;
  goto ltmp_877_D;

  do {
ltmp_877_D:
  ltmp_431_7 = ltmp_431_7_2E;
  *(&ltmp_430_13[ltmp_431_7]) = 0;
  ltmp_432_7 = ltmp_431_7 + 1u;
  if ((ltmp_432_7 == 36u)) {
    goto ltmp_878_D;
  } else {
    ltmp_431_7_2E = ltmp_432_7;
    goto ltmp_877_D;
  }

  } while (1);
ltmp_878_D:
  ltmp_433_1038 = *ltmp_428_1037;
  ltmp_434_7 = *ltmp_400_17;
  ltmp_435_13 = (unsigned char *)(&ltmp_433_1038[ltmp_434_7]);
  ltmp_436_7_2E = 0u;
  goto ltmp_879_D;

  do {
ltmp_879_D:
  ltmp_436_7 = ltmp_436_7_2E;
  *(&ltmp_435_13[ltmp_436_7]) = 0;
  ltmp_437_7 = ltmp_436_7 + 1u;
  if ((ltmp_437_7 == 36u)) {
    ltmp_438_8_2E = 0;
    goto ltmp_865_D;
  } else {
    ltmp_436_7_2E = ltmp_437_7;
    goto ltmp_879_D;
  }

  } while (1);
ltmp_865_D:
  ltmp_438_8 = ltmp_438_8_2E;
  ltmp_439_7 = *ltmp_400_17;
  *ltmp_379_17 = ltmp_439_7;
  ltmp_440_8_2E = ltmp_438_8;
  goto ltmp_852_D;

ltmp_852_D:
  ltmp_440_8 = ltmp_440_8_2E;
  ltmp_441_1035 = &ltmp_339_1023->field20.field4;
  *ltmp_441_1035 = ltmp_365_102F;
  ltmp_442_102D = &ltmp_339_1023->field24.field2.field3;
  if (((ltmp_440_8 == 0) & (ltmp_359_8 > 1))) {
    goto ltmp_880_D;
  } else {
    goto ltmp_881_D;
  }

ltmp_880_D:
  ltmp_443_7 = *(&ltmp_339_1023->field19.field3);
  ltmp_444_7 = ltmp_443_7 + 1u;
  ltmp_445_7 = *ltmp_354_17;
  *(&ltmp_339_1023->field20.field0) = ltmp_444_7;
  *(&ltmp_339_1023->field20.field1) = ltmp_445_7;
  *(&ltmp_339_1023->field20.field2) = 0u;
  ltmp_446_1018 = *(&ltmp_339_1023->field24.field2.field0);
  ltmp_447_14 = ltmp_446_1018((&ltmp_339_1023->field24.field2), (ltmp_445_7*ltmp_444_7), 16u);
  *(&ltmp_339_1023->field20.field3) = ((struct l_struct_2E_A2_3A__3A_A3 *)ltmp_447_14);
  ltmp_448_8 = (((ltmp_447_14 == ((signed char *)0))) ? (-3) : (0));
  ltmp_449_14 = *ltmp_442_102D;
  if ((ltmp_449_14 == ((signed char *)0))) {
    ltmp_455_8_2E = ltmp_448_8;
    goto ltmp_882_D;
  } else {
    ltmp_451_8_2E = ltmp_448_8;
    ltmp_452_14_2E = ltmp_449_14;
    goto ltmp_883_D;
  }

ltmp_881_D:
  ltmp_450_14 = *ltmp_442_102D;
  if ((ltmp_450_14 == ((signed char *)0))) {
    ltmp_455_8_2E = ltmp_440_8;
    goto ltmp_882_D;
  } else {
    ltmp_451_8_2E = ltmp_440_8;
    ltmp_452_14_2E = ltmp_450_14;
    goto ltmp_883_D;
  }

ltmp_883_D:
  ltmp_451_8 = ltmp_451_8_2E;
  ltmp_452_14 = ltmp_452_14_2E;
  ltmp_453_1026 = (struct plzo_callback_t *)ltmp_452_14;
  ltmp_454_101A = *(&ltmp_453_1026->field2);
  if ((ltmp_454_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))0))) {
    ltmp_455_8_2E = ltmp_451_8;
    goto ltmp_882_D;
  } else {
    goto ltmp_884_D;
  }

ltmp_884_D:
  ltmp_454_101A(ltmp_453_1026, 0u, 0u, 0);
  if ((ltmp_451_8 == 0)) {
    ltmp_456_8_2E = ltmp_451_8;
    goto ltmp_885_D;
  } else {
    ltmp_833_8_2E = ltmp_451_8;
    goto ltmp_886_D;
  }

ltmp_882_D:
  ltmp_455_8 = ltmp_455_8_2E;
  if ((ltmp_455_8 == 0)) {
    ltmp_456_8_2E = ltmp_455_8;
    goto ltmp_885_D;
  } else {
    ltmp_833_8_2E = ltmp_455_8;
    goto ltmp_886_D;
  }

ltmp_885_D:
  ltmp_456_8 = ltmp_456_8_2E;
  ltmp_457_17 = &ltmp_339_1023->field19.field2;
  ltmp_458_1025 = &ltmp_339_1023->field1.field6;
  ltmp_459_1025 = &ltmp_339_1023->field9;
  ltmp_460_17 = &ltmp_339_1023->field24.field0;
  ltmp_461_17 = &ltmp_339_1023->field24.field1;
  ltmp_462_1039 = &ltmp_339_1023->field19.field7;
  ltmp_463_17 = &ltmp_339_1023->field19.field1;
  ltmp_464_1037 = &ltmp_339_1023->field19.field0;
  ltmp_465_103A = &ltmp_340_1032.field0;
  ltmp_466_103A = &ltmp_340_1032.field1;
  ltmp_467_103A = &ltmp_341_1032.field0;
  ltmp_468_103A = &ltmp_341_1032.field1;
  ltmp_469_7_2E = 0u;
  ltmp_470_8_2E = ltmp_456_8;
  goto ltmp_887_D;

  do {
ltmp_887_D:
  ltmp_469_7 = ltmp_469_7_2E;
  ltmp_470_8 = ltmp_470_8_2E;
  ltmp_471_7 = *ltmp_367_17;
  if ((ltmp_471_7 > ltmp_469_7)) {
    goto ltmp_888_D;
  } else {
    ltmp_833_8_2E = ltmp_470_8;
    goto ltmp_886_D;
  }

ltmp_889_D:
  ltmp_824_7 = *ltmp_460_17;
  ltmp_825_7 = *ltmp_461_17;
  if (((ltmp_794_7 - ltmp_824_7) < ltmp_825_7)) {
    ltmp_469_7_2E = ltmp_794_7;
    ltmp_470_8_2E = 0;
    goto ltmp_887_D;
  } else {
    goto ltmp_890_D;
  }

ltmp_891_D:
  ltmp_819_1029 = *ltmp_350_102C;
  ltmp_820_102A = *(&ltmp_819_1029[16]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))ltmp_820_102A)(ltmp_339_1023, ltmp_469_7, ltmp_794_7, 0);
  goto ltmp_889_D;

ltmp_892_D:
  ltmp_818_101A(ltmp_817_1026, ltmp_794_7, ltmp_815_7, 2);
  if ((ltmp_359_8 == 0)) {
    goto ltmp_891_D;
  } else {
    goto ltmp_893_D;
  }

ltmp_894_D:
  ltmp_818_101A = *(&ltmp_817_1026->field2);
  if ((ltmp_818_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))0))) {
    goto ltmp_895_D;
  } else {
    goto ltmp_892_D;
  }

ltmp_896_D:
  ltmp_812_13 = *ltmp_458_1025;
  ltmp_813_13 = *ltmp_459_1025;
  ltmp_814_8 = plzo_external_ptrdiff_uchar(ltmp_812_13, ltmp_813_13, 1u);
  ltmp_815_7 = (plzo_uint32_t )ltmp_814_8;
  ltmp_816_14 = *ltmp_442_102D;
  ltmp_817_1026 = (struct plzo_callback_t *)ltmp_816_14;
  if ((ltmp_816_14 == ((signed char *)0))) {
    goto ltmp_895_D;
  } else {
    goto ltmp_894_D;
  }

ltmp_897_D:
  ltmp_801_101A(ltmp_800_1026, ltmp_794_7, ltmp_798_7, 1);
  if ((ltmp_359_8 > 1)) {
    goto ltmp_898_D;
  } else {
    goto ltmp_896_D;
  }

ltmp_899_D:
  ltmp_801_101A = *(&ltmp_800_1026->field2);
  if ((ltmp_801_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))0))) {
    goto ltmp_900_D;
  } else {
    goto ltmp_897_D;
  }

ltmp_901_D:
  ltmp_794_7 = ltmp_794_7_2E;
  ltmp_795_13 = *ltmp_458_1025;
  ltmp_796_13 = *ltmp_459_1025;
  ltmp_797_8 = plzo_external_ptrdiff_uchar(ltmp_795_13, ltmp_796_13, 1u);
  ltmp_798_7 = (plzo_uint32_t )ltmp_797_8;
  ltmp_799_14 = *ltmp_442_102D;
  ltmp_800_1026 = (struct plzo_callback_t *)ltmp_799_14;
  if ((ltmp_799_14 == ((signed char *)0))) {
    goto ltmp_900_D;
  } else {
    goto ltmp_899_D;
  }

ltmp_902_D:
  ltmp_790_7 = *ltmp_367_17;
  if (((ltmp_790_7 != ltmp_787_7) & (ltmp_359_8 > 1))) {
    goto ltmp_903_D;
  } else {
    ltmp_794_7_2E = ltmp_787_7;
    goto ltmp_901_D;
  }

ltmp_904_D:
  ltmp_785_7 = *ltmp_498_17;
  *ltmp_457_17 = ltmp_785_7;
  ltmp_786_7 = *ltmp_498_17;
  *ltmp_479_17 = ltmp_786_7;
  *(&ltmp_472_1036->field11) = (ltmp_786_7 - ltmp_469_7);
  ltmp_787_7 = *ltmp_457_17;
  if ((ltmp_359_8 > 1)) {
    goto ltmp_905_D;
  } else {
    goto ltmp_902_D;
  }

ltmp_906_D:
  ltmp_495_7 = ltmp_495_7_2E;
  ltmp_496_7 = ltmp_496_7_2E;
  ltmp_497_1038 = ltmp_497_1038_2E;
  *(&ltmp_472_1036->field8) = ltmp_496_7;
  ltmp_498_17 = &ltmp_472_1036->field9;
  *ltmp_498_17 = (ltmp_496_7 + ltmp_495_7);
  *(&ltmp_472_1036->field4) = ltmp_495_7;
  ltmp_499_1036 = *ltmp_462_1039;
  ltmp_500_7 = *(&ltmp_499_1036->field8);
  ltmp_501_17 = &ltmp_499_1036->field9;
  ltmp_502_7 = *ltmp_501_17;
  if ((ltmp_502_7 > ltmp_500_7)) {
    goto ltmp_907_D;
  } else {
    goto ltmp_904_D;
  }

ltmp_888_D:
  ltmp_472_1036 = *ltmp_462_1039;
  *(&ltmp_472_1036->field5) = ltmp_469_7;
  *ltmp_463_17 = ltmp_469_7;
  ltmp_473_7 = *(&ltmp_472_1036->field2);
  ltmp_474_7 = ltmp_473_7 - ltmp_469_7;
  ltmp_475_7 = *(&ltmp_472_1036->field6);
  ltmp_476_7 = (((ltmp_475_7 < ltmp_474_7)) ? (ltmp_475_7) : (ltmp_474_7));
  ltmp_477_17 = &ltmp_472_1036->field7;
  *ltmp_477_17 = ltmp_475_7;
  ltmp_478_1038 = *ltmp_464_1037;
  ltmp_479_17 = &ltmp_472_1036->field10;
  ltmp_480_7 = *ltmp_479_17;
  if ((ltmp_480_7 > ltmp_469_7)) {
    goto ltmp_908_D;
  } else {
    ltmp_495_7_2E = ltmp_476_7;
    ltmp_496_7_2E = ltmp_469_7;
    ltmp_497_1038_2E = ltmp_478_1038;
    goto ltmp_906_D;
  }

ltmp_909_D:
  ltmp_493_1038 = &ltmp_478_1038[ltmp_481_7];
  ltmp_494_7 = ltmp_476_7 - ltmp_481_7;
  ltmp_495_7_2E = ltmp_494_7;
  ltmp_496_7_2E = ltmp_480_7;
  ltmp_497_1038_2E = ltmp_493_1038;
  goto ltmp_906_D;

ltmp_908_D:
  ltmp_481_7 = ltmp_480_7 - ltmp_469_7;
  ltmp_482_7 = *(&ltmp_472_1036->field11);
  ltmp_483_7 = ltmp_482_7 - ltmp_481_7;
  *ltmp_477_17 = ltmp_483_7;
  ltmp_484_7 = ltmp_481_7*36u;
  ltmp_485_13 = (unsigned char *)ltmp_478_1038;
  ltmp_486_13 = (unsigned char *)(&ltmp_478_1038[ltmp_483_7]);
  if (((ltmp_484_7 == 0u) | (ltmp_482_7 == ltmp_481_7))) {
    goto ltmp_909_D;
  } else {
    goto ltmp_910_D;
  }

  do {
ltmp_911_D:
  ltmp_487_7 = ltmp_487_7_2E;
  ltmp_488_3 = *(&ltmp_486_13[ltmp_487_7]);
  *(&ltmp_485_13[ltmp_487_7]) = ltmp_488_3;
  ltmp_489_7 = ltmp_487_7 + 1u;
  if ((ltmp_489_7 == ltmp_484_7)) {
    goto ltmp_909_D;
  } else {
    ltmp_487_7_2E = ltmp_489_7;
    goto ltmp_911_D;
  }

  } while (1);
ltmp_910_D:
  if ((ltmp_485_13 < ltmp_486_13)) {
    ltmp_487_7_2E = 0u;
    goto ltmp_911_D;
  } else {
    ltmp_490_7_2E = 0u;
    goto ltmp_912_D;
  }

  do {
ltmp_912_D:
  ltmp_490_7 = ltmp_490_7_2E;
  ltmp_491_3 = *(&ltmp_486_13[((((plzo_uint32_t )(-(((plzo_int32_t )ltmp_490_7)))) + 4294967295u) + ltmp_484_7)]);
  *(&ltmp_485_13[(ltmp_484_7 + (ltmp_490_7 ^ 4294967295u))]) = ltmp_491_3;
  ltmp_492_7 = ltmp_490_7 + 1u;
  if ((ltmp_492_7 == ltmp_484_7)) {
    goto ltmp_909_D;
  } else {
    ltmp_490_7_2E = ltmp_492_7;
    goto ltmp_912_D;
  }

  } while (1);
  do {
ltmp_913_D:
  ltmp_519_7 = ltmp_519_7_2E;
  ltmp_520_1038 = ltmp_520_1038_2E;
  ltmp_521_7 = *ltmp_503_17;
  if ((ltmp_521_7 < ltmp_519_7)) {
    goto ltmp_914_D;
  } else {
    goto ltmp_915_D;
  }

ltmp_916_D:
  ltmp_779_7 = ltmp_779_7_2E;
  ltmp_780_1038 = ltmp_780_1038_2E;
  ltmp_781_7 = ltmp_779_7 + 1u;
  ltmp_782_1038 = &ltmp_780_1038[1];
  ltmp_783_7 = *ltmp_516_17;
  *ltmp_516_17 = (ltmp_783_7 + 4294967295u);
  ltmp_784_7 = *ltmp_501_17;
  if ((ltmp_784_7 > ltmp_781_7)) {
    ltmp_519_7_2E = ltmp_781_7;
    ltmp_520_1038_2E = ltmp_782_1038;
    goto ltmp_913_D;
  } else {
    goto ltmp_904_D;
  }

ltmp_917_D:
  ltmp_769_7 = *(&ltmp_520_1038->field0);
  ltmp_770_7 = *ltmp_518_17;
  if ((ltmp_769_7 < ltmp_770_7)) {
    ltmp_779_7_2E = ltmp_519_7;
    ltmp_780_1038_2E = ltmp_520_1038;
    goto ltmp_916_D;
  } else {
    goto ltmp_918_D;
  }

ltmp_919_D:
  *ltmp_525_17 = 0u;
  goto ltmp_917_D;

ltmp_920_D:
  ltmp_733_7 = *ltmp_525_17;
  ltmp_734_7 = *ltmp_506_17;
  if ((ltmp_733_7 < ltmp_734_7)) {
    goto ltmp_919_D;
  } else {
    goto ltmp_921_D;
  }

ltmp_922_D:
  ltmp_731_7 = ltmp_731_7_2E;
  *ltmp_541_17 = ltmp_519_7;
  *ltmp_525_17 = ltmp_731_7;
  ltmp_732_7 = *ltmp_516_17;
  if ((ltmp_731_7 > ltmp_732_7)) {
    goto ltmp_923_D;
  } else {
    goto ltmp_920_D;
  }

ltmp_924_D:
  ltmp_538_7 = ltmp_538_7_2E;
  ltmp_539_3 = *ltmp_531_13;
  ltmp_540_3 = *(&ltmp_529_13[(ltmp_530_8 + 1)]);
  ltmp_541_17 = &ltmp_499_1036->field26[((((plzo_int32_t )ltmp_540_3) << 8) | ((plzo_int32_t )ltmp_539_3))];
  ltmp_542_7 = *ltmp_541_17;
  if ((ltmp_542_7 < ltmp_519_7)) {
    goto ltmp_925_D;
  } else {
    ltmp_731_7_2E = ltmp_538_7;
    goto ltmp_922_D;
  }

ltmp_915_D:
  ltmp_522_7 = *ltmp_504_17;
  ltmp_523_7 = ltmp_522_7 - ltmp_519_7;
  ltmp_524_7 = (((ltmp_523_7 < 1025u)) ? (ltmp_523_7) : (1024u));
  ltmp_525_17 = &ltmp_520_1038->field0;
  ltmp_526_1032 = ltmp_18_100A(ltmp_499_1036);
  ltmp_527_103A = &ltmp_526_1032->field0;
  ltmp_528_103A = &ltmp_526_1032->field1;
  *ltmp_528_103A = ((struct l_struct_2E_M *)0);
  *ltmp_527_103A = ((struct l_struct_2E_M *)0);
  ltmp_529_13 = *ltmp_505_1025;
  ltmp_530_8 = (plzo_int32_t )ltmp_519_7;
  ltmp_531_13 = &ltmp_529_13[ltmp_519_7];
  ltmp_532_7 = *ltmp_506_17;
  if ((ltmp_532_7 == 1u)) {
    goto ltmp_926_D;
  } else {
    ltmp_538_7_2E = 0u;
    goto ltmp_924_D;
  }

ltmp_927_D:
  *ltmp_508_1025 = (&ltmp_529_13[(ltmp_519_7 - ltmp_536_7)]);
  *ltmp_534_17 = ltmp_519_7;
  ltmp_538_7_2E = 1u;
  goto ltmp_924_D;

ltmp_928_D:
  ltmp_536_7 = ltmp_519_7 - ltmp_535_7;
  ltmp_537_7 = *ltmp_507_17;
  if ((ltmp_537_7 < ltmp_536_7)) {
    goto ltmp_929_D;
  } else {
    goto ltmp_927_D;
  }

ltmp_926_D:
  ltmp_533_3 = *ltmp_531_13;
  ltmp_534_17 = &ltmp_499_1036->field27[((plzo_uint32_t )ltmp_533_3)];
  ltmp_535_7 = *ltmp_534_17;
  if ((ltmp_535_7 < ltmp_519_7)) {
    goto ltmp_928_D;
  } else {
    goto ltmp_929_D;
  }

ltmp_929_D:
  *ltmp_534_17 = ltmp_519_7;
  ltmp_538_7_2E = 0u;
  goto ltmp_924_D;

ltmp_925_D:
  ltmp_543_7 = ltmp_519_7 - ltmp_542_7;
  ltmp_544_7 = *ltmp_507_17;
  if ((ltmp_544_7 < ltmp_543_7)) {
    ltmp_731_7_2E = ltmp_538_7;
    goto ltmp_922_D;
  } else {
    goto ltmp_930_D;
  }

ltmp_931_D:
  ltmp_665_7 = ltmp_665_7_2E;
  ltmp_666_1032 = *ltmp_466_103A;
  *ltmp_527_103A = ltmp_666_1032;
  ltmp_667_1032 = *ltmp_465_103A;
  *ltmp_528_103A = ltmp_667_1032;
  ltmp_731_7_2E = ltmp_665_7;
  goto ltmp_922_D;

ltmp_932_D:
  *(&ltmp_590_1032->field0) = ltmp_592_1032;
  *(&ltmp_591_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_665_7_2E = ltmp_600_7;
  goto ltmp_931_D;

  do {
ltmp_933_D:
  ltmp_586_13 = ltmp_586_13_2E;
  ltmp_587_7 = ltmp_587_7_2E;
  ltmp_588_7 = ltmp_588_7_2E;
  ltmp_589_7 = ltmp_589_7_2E;
  ltmp_590_1032 = ltmp_590_1032_2E;
  ltmp_591_1032 = ltmp_591_1032_2E;
  ltmp_592_1032 = ltmp_592_1032_2E;
  ltmp_593_7 = ltmp_593_7_2E;
  ltmp_594_7 = ltmp_594_7_2E;
  ltmp_595_7 = ltmp_594_7 + ltmp_593_7;
  ltmp_596_3 = *(&ltmp_566_13[(ltmp_577_7 + ltmp_595_7)]);
  ltmp_597_3 = *(&ltmp_586_13[ltmp_595_7]);
  if ((ltmp_596_3 == ltmp_597_3)) {
    goto ltmp_934_D;
  } else {
    ltmp_600_7_2E = ltmp_595_7;
    goto ltmp_935_D;
  }

ltmp_936_D:
  ltmp_578_13 = ltmp_578_13_2E;
  ltmp_579_7 = ltmp_579_7_2E;
  ltmp_580_7 = ltmp_580_7_2E;
  ltmp_581_7 = ltmp_581_7_2E;
  ltmp_582_1032 = ltmp_582_1032_2E;
  ltmp_583_1032 = ltmp_583_1032_2E;
  ltmp_584_1032 = ltmp_584_1032_2E;
  ltmp_585_7 = (((ltmp_580_7 <= ltmp_579_7)) ? (ltmp_580_7) : (ltmp_579_7));
  ltmp_586_13_2E = ltmp_578_13;
  ltmp_587_7_2E = ltmp_579_7;
  ltmp_588_7_2E = ltmp_580_7;
  ltmp_589_7_2E = ltmp_581_7;
  ltmp_590_1032_2E = ltmp_582_1032;
  ltmp_591_1032_2E = ltmp_583_1032;
  ltmp_592_1032_2E = ltmp_584_1032;
  ltmp_593_7_2E = ltmp_585_7;
  ltmp_594_7_2E = 0u;
  goto ltmp_933_D;

ltmp_937_D:
  ltmp_615_7 = ltmp_615_7_2E;
  ltmp_616_7 = *ltmp_511_17;
  ltmp_617_13 = *ltmp_505_1025;
  ltmp_618_13 = &ltmp_617_13[(ltmp_616_7 + ltmp_615_7)];
  if ((ltmp_618_13 < ltmp_586_13)) {
    ltmp_578_13_2E = ltmp_618_13;
    ltmp_579_7_2E = ltmp_587_7;
    ltmp_580_7_2E = ltmp_606_7;
    ltmp_581_7_2E = ltmp_607_7;
    ltmp_582_1032_2E = ltmp_592_1032;
    ltmp_583_1032_2E = ltmp_591_1032;
    ltmp_584_1032_2E = ltmp_608_1032;
    goto ltmp_936_D;
  } else {
    goto ltmp_938_D;
  }

ltmp_939_D:
  ltmp_609_1032 = *ltmp_514_103A;
  ltmp_610_8 = plzo_external_ptrdiff_M(ltmp_608_1032, ltmp_609_1032, 8u);
  ltmp_611_7 = *ltmp_512_17;
  ltmp_612_7 = ((plzo_uint32_t )ltmp_610_8) - ltmp_611_7;
  if ((ltmp_612_7 > 2147483647u)) {
    ltmp_615_7_2E = ltmp_612_7;
    goto ltmp_937_D;
  } else {
    goto ltmp_940_D;
  }

ltmp_941_D:
  ltmp_606_7 = ltmp_606_7_2E;
  ltmp_607_7 = ltmp_607_7_2E;
  *(&ltmp_590_1032->field0) = ltmp_592_1032;
  ltmp_608_1032 = *(&ltmp_592_1032->field0);
  if ((ltmp_608_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_942_D;
  } else {
    goto ltmp_939_D;
  }

ltmp_943_D:
  if ((ltmp_600_7 > ltmp_588_7)) {
    goto ltmp_944_D;
  } else {
    ltmp_606_7_2E = ltmp_588_7;
    ltmp_607_7_2E = ltmp_589_7;
    goto ltmp_941_D;
  }

ltmp_935_D:
  ltmp_600_7 = ltmp_600_7_2E;
  ltmp_601_8 = ((plzo_int32_t )ltmp_596_3) - ((plzo_int32_t )ltmp_597_3);
  if ((ltmp_601_8 < 0)) {
    goto ltmp_943_D;
  } else {
    goto ltmp_945_D;
  }

ltmp_934_D:
  ltmp_598_7 = ltmp_595_7 + 1u;
  ltmp_599_7 = ltmp_594_7 + 1u;
  if ((ltmp_598_7 < ltmp_524_7)) {
    ltmp_586_13_2E = ltmp_586_13;
    ltmp_587_7_2E = ltmp_587_7;
    ltmp_588_7_2E = ltmp_588_7;
    ltmp_589_7_2E = ltmp_589_7;
    ltmp_590_1032_2E = ltmp_590_1032;
    ltmp_591_1032_2E = ltmp_591_1032;
    ltmp_592_1032_2E = ltmp_592_1032;
    ltmp_593_7_2E = ltmp_593_7;
    ltmp_594_7_2E = ltmp_599_7;
    goto ltmp_933_D;
  } else {
    ltmp_600_7_2E = ltmp_598_7;
    goto ltmp_935_D;
  }

ltmp_944_D:
  if ((ltmp_600_7 > ltmp_589_7)) {
    goto ltmp_946_D;
  } else {
    ltmp_606_7_2E = ltmp_600_7;
    ltmp_607_7_2E = ltmp_589_7;
    goto ltmp_941_D;
  }

ltmp_947_D:
  ltmp_605_7 = *ltmp_515_17;
  if ((ltmp_605_7 > ltmp_600_7)) {
    ltmp_606_7_2E = ltmp_600_7;
    ltmp_607_7_2E = ltmp_600_7;
    goto ltmp_941_D;
  } else {
    goto ltmp_932_D;
  }

ltmp_946_D:
  ltmp_602_7 = ltmp_589_7 + 1u;
  *(&ltmp_499_1036->field25[ltmp_602_7]) = ltmp_586_13;
  if ((ltmp_602_7 < ltmp_600_7)) {
    ltmp_603_7_2E = 0u;
    goto ltmp_948_D;
  } else {
    goto ltmp_947_D;
  }

  do {
ltmp_948_D:
  ltmp_603_7 = ltmp_603_7_2E;
  ltmp_604_7 = (ltmp_603_7 + ltmp_602_7) + 1u;
  *(&ltmp_499_1036->field25[ltmp_604_7]) = ltmp_586_13;
  if ((ltmp_604_7 < ltmp_600_7)) {
    ltmp_603_7_2E = (ltmp_603_7 + 1u);
    goto ltmp_948_D;
  } else {
    goto ltmp_947_D;
  }

  } while (1);
ltmp_940_D:
  ltmp_613_7 = *ltmp_513_17;
  ltmp_614_7 = ltmp_612_7 - ltmp_613_7;
  ltmp_615_7_2E = ltmp_614_7;
  goto ltmp_937_D;

ltmp_949_D:
  ltmp_632_7 = ltmp_632_7_2E;
  ltmp_633_7 = *ltmp_511_17;
  ltmp_634_13 = *ltmp_505_1025;
  ltmp_635_13 = &ltmp_634_13[(ltmp_633_7 + ltmp_632_7)];
  if ((ltmp_635_13 < ltmp_586_13)) {
    ltmp_578_13_2E = ltmp_635_13;
    ltmp_579_7_2E = ltmp_623_7;
    ltmp_580_7_2E = ltmp_588_7;
    ltmp_581_7_2E = ltmp_624_7;
    ltmp_582_1032_2E = ltmp_590_1032;
    ltmp_583_1032_2E = ltmp_592_1032;
    ltmp_584_1032_2E = ltmp_625_1032;
    goto ltmp_936_D;
  } else {
    goto ltmp_950_D;
  }

ltmp_951_D:
  ltmp_626_1032 = *ltmp_514_103A;
  ltmp_627_8 = plzo_external_ptrdiff_M(ltmp_625_1032, ltmp_626_1032, 8u);
  ltmp_628_7 = *ltmp_512_17;
  ltmp_629_7 = ((plzo_uint32_t )ltmp_627_8) - ltmp_628_7;
  if ((ltmp_629_7 > 2147483647u)) {
    ltmp_632_7_2E = ltmp_629_7;
    goto ltmp_949_D;
  } else {
    goto ltmp_952_D;
  }

ltmp_953_D:
  ltmp_623_7 = ltmp_623_7_2E;
  ltmp_624_7 = ltmp_624_7_2E;
  *(&ltmp_591_1032->field1) = ltmp_592_1032;
  ltmp_625_1032 = *(&ltmp_592_1032->field1);
  if ((ltmp_625_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_954_D;
  } else {
    goto ltmp_951_D;
  }

ltmp_955_D:
  if ((ltmp_600_7 > ltmp_587_7)) {
    goto ltmp_956_D;
  } else {
    ltmp_623_7_2E = ltmp_587_7;
    ltmp_624_7_2E = ltmp_589_7;
    goto ltmp_953_D;
  }

ltmp_945_D:
  if ((ltmp_601_8 > 0)) {
    goto ltmp_955_D;
  } else {
    goto ltmp_957_D;
  }

ltmp_956_D:
  if ((ltmp_600_7 > ltmp_589_7)) {
    goto ltmp_958_D;
  } else {
    ltmp_623_7_2E = ltmp_600_7;
    ltmp_624_7_2E = ltmp_589_7;
    goto ltmp_953_D;
  }

ltmp_959_D:
  ltmp_622_7 = *ltmp_515_17;
  if ((ltmp_622_7 > ltmp_600_7)) {
    ltmp_623_7_2E = ltmp_600_7;
    ltmp_624_7_2E = ltmp_600_7;
    goto ltmp_953_D;
  } else {
    goto ltmp_960_D;
  }

ltmp_958_D:
  ltmp_619_7 = ltmp_589_7 + 1u;
  *(&ltmp_499_1036->field25[ltmp_619_7]) = ltmp_586_13;
  if ((ltmp_619_7 < ltmp_600_7)) {
    ltmp_620_7_2E = 0u;
    goto ltmp_961_D;
  } else {
    goto ltmp_959_D;
  }

  do {
ltmp_961_D:
  ltmp_620_7 = ltmp_620_7_2E;
  ltmp_621_7 = (ltmp_620_7 + ltmp_619_7) + 1u;
  *(&ltmp_499_1036->field25[ltmp_621_7]) = ltmp_586_13;
  if ((ltmp_621_7 < ltmp_600_7)) {
    ltmp_620_7_2E = (ltmp_620_7 + 1u);
    goto ltmp_961_D;
  } else {
    goto ltmp_959_D;
  }

  } while (1);
ltmp_952_D:
  ltmp_630_7 = *ltmp_513_17;
  ltmp_631_7 = ltmp_629_7 - ltmp_630_7;
  ltmp_632_7_2E = ltmp_631_7;
  goto ltmp_949_D;

  } while (1);
ltmp_962_D:
  ltmp_573_7 = ltmp_573_7_2E;
  ltmp_574_7 = *ltmp_511_17;
  ltmp_575_13 = *ltmp_505_1025;
  ltmp_576_13 = &ltmp_575_13[(ltmp_574_7 + ltmp_573_7)];
  ltmp_577_7 = ltmp_565_7 + ltmp_564_7;
  ltmp_586_13_2E = ltmp_576_13;
  ltmp_587_7_2E = 2u;
  ltmp_588_7_2E = 2u;
  ltmp_589_7_2E = 2u;
  ltmp_590_1032_2E = (&ltmp_340_1032);
  ltmp_591_1032_2E = (&ltmp_340_1032);
  ltmp_592_1032_2E = ltmp_557_1032;
  ltmp_593_7_2E = 2u;
  ltmp_594_7_2E = 0u;
  goto ltmp_933_D;

ltmp_963_D:
  ltmp_564_7 = ltmp_564_7_2E;
  ltmp_565_7 = *ltmp_511_17;
  ltmp_566_13 = *ltmp_505_1025;
  ltmp_567_1032 = *ltmp_514_103A;
  ltmp_568_8 = plzo_external_ptrdiff_M(ltmp_557_1032, ltmp_567_1032, 8u);
  ltmp_569_7 = *ltmp_512_17;
  ltmp_570_7 = ((plzo_uint32_t )ltmp_568_8) - ltmp_569_7;
  if ((ltmp_570_7 > 2147483647u)) {
    ltmp_573_7_2E = ltmp_570_7;
    goto ltmp_962_D;
  } else {
    goto ltmp_964_D;
  }

ltmp_965_D:
  ltmp_557_1032 = ltmp_557_1032_2E;
  *ltmp_465_103A = ((struct l_struct_2E_M *)0);
  *ltmp_466_103A = ((struct l_struct_2E_M *)0);
  ltmp_558_1032 = *ltmp_514_103A;
  ltmp_559_8 = plzo_external_ptrdiff_M(ltmp_526_1032, ltmp_558_1032, 8u);
  ltmp_560_7 = *ltmp_512_17;
  ltmp_561_7 = ((plzo_uint32_t )ltmp_559_8) - ltmp_560_7;
  if ((ltmp_561_7 > 2147483647u)) {
    ltmp_564_7_2E = ltmp_561_7;
    goto ltmp_963_D;
  } else {
    goto ltmp_966_D;
  }

ltmp_967_D:
  ltmp_548_7 = *ltmp_511_17;
  ltmp_549_7 = ((plzo_uint32_t )ltmp_547_8) - ltmp_548_7;
  if ((ltmp_549_7 > 2147483647u)) {
    goto ltmp_968_D;
  } else {
    ltmp_557_1032_2E = ((struct l_struct_2E_M *)0);
    goto ltmp_965_D;
  }

ltmp_930_D:
  ltmp_545_13 = &ltmp_529_13[(((plzo_int32_t )(-(ltmp_543_7))) + ltmp_530_8)];
  *ltmp_509_1025 = ltmp_545_13;
  ltmp_546_8 = *ltmp_510_18;
  ltmp_547_8 = plzo_external_ptrdiff_uchar(ltmp_545_13, ltmp_529_13, 1u);
  if ((ltmp_546_8 == 0)) {
    goto ltmp_969_D;
  } else {
    goto ltmp_967_D;
  }

ltmp_970_D:
  ltmp_552_7 = *ltmp_513_17;
  ltmp_553_7 = ltmp_552_7 + ltmp_551_7;
  if ((ltmp_553_7 > 2147483647u)) {
    ltmp_557_1032_2E = ((struct l_struct_2E_M *)0);
    goto ltmp_965_D;
  } else {
    ltmp_554_7_2E = ltmp_553_7;
    goto ltmp_971_D;
  }

ltmp_968_D:
  ltmp_550_7 = *ltmp_512_17;
  ltmp_551_7 = ltmp_550_7 + ltmp_549_7;
  if ((ltmp_551_7 > 2147483647u)) {
    goto ltmp_970_D;
  } else {
    ltmp_554_7_2E = ltmp_551_7;
    goto ltmp_971_D;
  }

ltmp_971_D:
  ltmp_554_7 = ltmp_554_7_2E;
  ltmp_555_1032 = *ltmp_514_103A;
  ltmp_556_1032 = &ltmp_555_1032[ltmp_554_7];
  ltmp_557_1032_2E = ltmp_556_1032;
  goto ltmp_965_D;

ltmp_966_D:
  ltmp_562_7 = *ltmp_513_17;
  ltmp_563_7 = ltmp_561_7 - ltmp_562_7;
  ltmp_564_7_2E = ltmp_563_7;
  goto ltmp_963_D;

ltmp_964_D:
  ltmp_571_7 = *ltmp_513_17;
  ltmp_572_7 = ltmp_570_7 - ltmp_571_7;
  ltmp_573_7_2E = ltmp_572_7;
  goto ltmp_962_D;

ltmp_942_D:
  *(&ltmp_591_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_665_7_2E = ltmp_607_7;
  goto ltmp_931_D;

ltmp_938_D:
  *(&ltmp_591_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_665_7_2E = ltmp_607_7;
  goto ltmp_931_D;

ltmp_960_D:
  *(&ltmp_591_1032->field1) = ltmp_592_1032;
  *(&ltmp_590_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_665_7_2E = ltmp_600_7;
  goto ltmp_931_D;

ltmp_954_D:
  *(&ltmp_590_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_665_7_2E = ltmp_624_7;
  goto ltmp_931_D;

ltmp_950_D:
  *(&ltmp_590_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_665_7_2E = ltmp_624_7;
  goto ltmp_931_D;

ltmp_972_D:
  ltmp_663_1032 = *ltmp_639_103A;
  *(&ltmp_591_1032->field1) = ltmp_663_1032;
  ltmp_664_1032 = *(&ltmp_592_1032->field1);
  *(&ltmp_590_1032->field0) = ltmp_664_1032;
  ltmp_665_7_2E = ltmp_600_7;
  goto ltmp_931_D;

ltmp_973_D:
  *ltmp_639_103A = ((struct l_struct_2E_M *)0);
  ltmp_650_1032 = *(&ltmp_592_1032->field1);
  if ((ltmp_650_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_972_D;
  } else {
    goto ltmp_974_D;
  }

ltmp_975_D:
  ltmp_647_7 = ltmp_647_7_2E;
  ltmp_648_7 = *ltmp_511_17;
  ltmp_649_13 = *ltmp_505_1025;
  if (((&ltmp_649_13[(ltmp_648_7 + ltmp_647_7)]) < ltmp_586_13)) {
    goto ltmp_976_D;
  } else {
    goto ltmp_973_D;
  }

ltmp_977_D:
  ltmp_641_1032 = *ltmp_514_103A;
  ltmp_642_8 = plzo_external_ptrdiff_M(ltmp_640_1032, ltmp_641_1032, 8u);
  ltmp_643_7 = *ltmp_512_17;
  ltmp_644_7 = ((plzo_uint32_t )ltmp_642_8) - ltmp_643_7;
  if ((ltmp_644_7 > 2147483647u)) {
    ltmp_647_7_2E = ltmp_644_7;
    goto ltmp_975_D;
  } else {
    goto ltmp_978_D;
  }

ltmp_979_D:
  ltmp_639_103A = &ltmp_592_1032->field0;
  ltmp_640_1032 = *ltmp_639_103A;
  if ((ltmp_640_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_976_D;
  } else {
    goto ltmp_977_D;
  }

ltmp_957_D:
  ltmp_636_7 = ltmp_589_7 + 1u;
  *(&ltmp_499_1036->field25[ltmp_636_7]) = ltmp_586_13;
  if ((ltmp_636_7 < ltmp_600_7)) {
    ltmp_637_7_2E = 0u;
    goto ltmp_980_D;
  } else {
    goto ltmp_979_D;
  }

  do {
ltmp_980_D:
  ltmp_637_7 = ltmp_637_7_2E;
  ltmp_638_7 = (ltmp_637_7 + ltmp_636_7) + 1u;
  *(&ltmp_499_1036->field25[ltmp_638_7]) = ltmp_586_13;
  if ((ltmp_638_7 < ltmp_600_7)) {
    ltmp_637_7_2E = (ltmp_637_7 + 1u);
    goto ltmp_980_D;
  } else {
    goto ltmp_979_D;
  }

  } while (1);
ltmp_978_D:
  ltmp_645_7 = *ltmp_513_17;
  ltmp_646_7 = ltmp_644_7 - ltmp_645_7;
  ltmp_647_7_2E = ltmp_646_7;
  goto ltmp_975_D;

ltmp_976_D:
  ltmp_651_1032 = *(&ltmp_592_1032->field1);
  if ((ltmp_651_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_972_D;
  } else {
    goto ltmp_974_D;
  }

ltmp_981_D:
  ltmp_660_7 = ltmp_660_7_2E;
  ltmp_661_7 = *ltmp_511_17;
  ltmp_662_13 = *ltmp_505_1025;
  if (((&ltmp_662_13[(ltmp_661_7 + ltmp_660_7)]) < ltmp_586_13)) {
    goto ltmp_972_D;
  } else {
    goto ltmp_982_D;
  }

ltmp_974_D:
  ltmp_652_103A = &ltmp_592_1032->field1;
  ltmp_653_1032 = *ltmp_652_103A;
  ltmp_654_1032 = *ltmp_514_103A;
  ltmp_655_8 = plzo_external_ptrdiff_M(ltmp_653_1032, ltmp_654_1032, 8u);
  ltmp_656_7 = *ltmp_512_17;
  ltmp_657_7 = ((plzo_uint32_t )ltmp_655_8) - ltmp_656_7;
  if ((ltmp_657_7 > 2147483647u)) {
    ltmp_660_7_2E = ltmp_657_7;
    goto ltmp_981_D;
  } else {
    goto ltmp_983_D;
  }

ltmp_983_D:
  ltmp_658_7 = *ltmp_513_17;
  ltmp_659_7 = ltmp_657_7 - ltmp_658_7;
  ltmp_660_7_2E = ltmp_659_7;
  goto ltmp_981_D;

ltmp_982_D:
  *ltmp_652_103A = ((struct l_struct_2E_M *)0);
  goto ltmp_972_D;

ltmp_984_D:
  ltmp_728_7 = ltmp_728_7_2E;
  ltmp_729_1032 = *ltmp_468_103A;
  *ltmp_527_103A = ltmp_729_1032;
  ltmp_730_1032 = *ltmp_467_103A;
  *ltmp_528_103A = ltmp_730_1032;
  ltmp_731_7_2E = ltmp_728_7;
  goto ltmp_922_D;

ltmp_985_D:
  *(&ltmp_689_1032->field0) = ltmp_691_1032;
  *(&ltmp_690_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_728_7_2E = ltmp_699_7;
  goto ltmp_984_D;

  do {
ltmp_986_D:
  ltmp_685_13 = ltmp_685_13_2E;
  ltmp_686_7 = ltmp_686_7_2E;
  ltmp_687_7 = ltmp_687_7_2E;
  ltmp_688_7 = ltmp_688_7_2E;
  ltmp_689_1032 = ltmp_689_1032_2E;
  ltmp_690_1032 = ltmp_690_1032_2E;
  ltmp_691_1032 = ltmp_691_1032_2E;
  ltmp_692_7 = ltmp_692_7_2E;
  ltmp_693_7 = ltmp_693_7_2E;
  ltmp_694_7 = ltmp_693_7 + ltmp_692_7;
  ltmp_695_3 = *(&ltmp_672_13[(((plzo_int32_t )ltmp_694_7) + ltmp_671_8)]);
  ltmp_696_3 = *(&ltmp_685_13[ltmp_694_7]);
  if ((ltmp_695_3 == ltmp_696_3)) {
    goto ltmp_987_D;
  } else {
    ltmp_699_7_2E = ltmp_694_7;
    goto ltmp_988_D;
  }

ltmp_989_D:
  ltmp_677_13 = ltmp_677_13_2E;
  ltmp_678_7 = ltmp_678_7_2E;
  ltmp_679_7 = ltmp_679_7_2E;
  ltmp_680_7 = ltmp_680_7_2E;
  ltmp_681_1032 = ltmp_681_1032_2E;
  ltmp_682_1032 = ltmp_682_1032_2E;
  ltmp_683_1032 = ltmp_683_1032_2E;
  ltmp_684_7 = (((ltmp_679_7 <= ltmp_678_7)) ? (ltmp_679_7) : (ltmp_678_7));
  ltmp_685_13_2E = ltmp_677_13;
  ltmp_686_7_2E = ltmp_678_7;
  ltmp_687_7_2E = ltmp_679_7;
  ltmp_688_7_2E = ltmp_680_7;
  ltmp_689_1032_2E = ltmp_681_1032;
  ltmp_690_1032_2E = ltmp_682_1032;
  ltmp_691_1032_2E = ltmp_683_1032;
  ltmp_692_7_2E = ltmp_684_7;
  ltmp_693_7_2E = 0u;
  goto ltmp_986_D;

ltmp_990_D:
  ltmp_708_1032 = *ltmp_514_103A;
  ltmp_709_8 = plzo_external_ptrdiff_M(ltmp_707_1032, ltmp_708_1032, 8u);
  ltmp_710_13 = *ltmp_505_1025;
  ltmp_711_13 = &ltmp_710_13[ltmp_709_8];
  ltmp_677_13_2E = ltmp_711_13;
  ltmp_678_7_2E = ltmp_686_7;
  ltmp_679_7_2E = ltmp_705_7;
  ltmp_680_7_2E = ltmp_706_7;
  ltmp_681_1032_2E = ltmp_691_1032;
  ltmp_682_1032_2E = ltmp_690_1032;
  ltmp_683_1032_2E = ltmp_707_1032;
  goto ltmp_989_D;

ltmp_991_D:
  ltmp_705_7 = ltmp_705_7_2E;
  ltmp_706_7 = ltmp_706_7_2E;
  *(&ltmp_689_1032->field0) = ltmp_691_1032;
  ltmp_707_1032 = *(&ltmp_691_1032->field0);
  if ((ltmp_707_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_992_D;
  } else {
    goto ltmp_990_D;
  }

ltmp_993_D:
  if ((ltmp_699_7 > ltmp_687_7)) {
    goto ltmp_994_D;
  } else {
    ltmp_705_7_2E = ltmp_687_7;
    ltmp_706_7_2E = ltmp_688_7;
    goto ltmp_991_D;
  }

ltmp_988_D:
  ltmp_699_7 = ltmp_699_7_2E;
  ltmp_700_8 = ((plzo_int32_t )ltmp_695_3) - ((plzo_int32_t )ltmp_696_3);
  if ((ltmp_700_8 < 0)) {
    goto ltmp_993_D;
  } else {
    goto ltmp_995_D;
  }

ltmp_987_D:
  ltmp_697_7 = ltmp_694_7 + 1u;
  ltmp_698_7 = ltmp_693_7 + 1u;
  if ((ltmp_697_7 < ltmp_524_7)) {
    ltmp_685_13_2E = ltmp_685_13;
    ltmp_686_7_2E = ltmp_686_7;
    ltmp_687_7_2E = ltmp_687_7;
    ltmp_688_7_2E = ltmp_688_7;
    ltmp_689_1032_2E = ltmp_689_1032;
    ltmp_690_1032_2E = ltmp_690_1032;
    ltmp_691_1032_2E = ltmp_691_1032;
    ltmp_692_7_2E = ltmp_692_7;
    ltmp_693_7_2E = ltmp_698_7;
    goto ltmp_986_D;
  } else {
    ltmp_699_7_2E = ltmp_697_7;
    goto ltmp_988_D;
  }

ltmp_994_D:
  if ((ltmp_699_7 > ltmp_688_7)) {
    goto ltmp_996_D;
  } else {
    ltmp_705_7_2E = ltmp_699_7;
    ltmp_706_7_2E = ltmp_688_7;
    goto ltmp_991_D;
  }

ltmp_997_D:
  ltmp_704_7 = *ltmp_515_17;
  if ((ltmp_704_7 > ltmp_699_7)) {
    ltmp_705_7_2E = ltmp_699_7;
    ltmp_706_7_2E = ltmp_699_7;
    goto ltmp_991_D;
  } else {
    goto ltmp_985_D;
  }

ltmp_996_D:
  ltmp_701_7 = ltmp_688_7 + 1u;
  *(&ltmp_499_1036->field25[ltmp_701_7]) = ltmp_685_13;
  if ((ltmp_701_7 < ltmp_699_7)) {
    ltmp_702_7_2E = 0u;
    goto ltmp_998_D;
  } else {
    goto ltmp_997_D;
  }

  do {
ltmp_998_D:
  ltmp_702_7 = ltmp_702_7_2E;
  ltmp_703_7 = (ltmp_702_7 + ltmp_701_7) + 1u;
  *(&ltmp_499_1036->field25[ltmp_703_7]) = ltmp_685_13;
  if ((ltmp_703_7 < ltmp_699_7)) {
    ltmp_702_7_2E = (ltmp_702_7 + 1u);
    goto ltmp_998_D;
  } else {
    goto ltmp_997_D;
  }

  } while (1);
ltmp_999_D:
  ltmp_719_1032 = *ltmp_514_103A;
  ltmp_720_8 = plzo_external_ptrdiff_M(ltmp_718_1032, ltmp_719_1032, 8u);
  ltmp_721_13 = *ltmp_505_1025;
  ltmp_722_13 = &ltmp_721_13[ltmp_720_8];
  ltmp_677_13_2E = ltmp_722_13;
  ltmp_678_7_2E = ltmp_716_7;
  ltmp_679_7_2E = ltmp_687_7;
  ltmp_680_7_2E = ltmp_717_7;
  ltmp_681_1032_2E = ltmp_689_1032;
  ltmp_682_1032_2E = ltmp_691_1032;
  ltmp_683_1032_2E = ltmp_718_1032;
  goto ltmp_989_D;

ltmp_1000_D:
  ltmp_716_7 = ltmp_716_7_2E;
  ltmp_717_7 = ltmp_717_7_2E;
  *(&ltmp_690_1032->field1) = ltmp_691_1032;
  ltmp_718_1032 = *(&ltmp_691_1032->field1);
  if ((ltmp_718_1032 == ((struct l_struct_2E_M *)0))) {
    goto ltmp_1001_D;
  } else {
    goto ltmp_999_D;
  }

ltmp_1002_D:
  if ((ltmp_699_7 > ltmp_686_7)) {
    goto ltmp_1003_D;
  } else {
    ltmp_716_7_2E = ltmp_686_7;
    ltmp_717_7_2E = ltmp_688_7;
    goto ltmp_1000_D;
  }

ltmp_995_D:
  if ((ltmp_700_8 > 0)) {
    goto ltmp_1002_D;
  } else {
    goto ltmp_1004_D;
  }

ltmp_1003_D:
  if ((ltmp_699_7 > ltmp_688_7)) {
    goto ltmp_1005_D;
  } else {
    ltmp_716_7_2E = ltmp_699_7;
    ltmp_717_7_2E = ltmp_688_7;
    goto ltmp_1000_D;
  }

ltmp_1006_D:
  ltmp_715_7 = *ltmp_515_17;
  if ((ltmp_715_7 > ltmp_699_7)) {
    ltmp_716_7_2E = ltmp_699_7;
    ltmp_717_7_2E = ltmp_699_7;
    goto ltmp_1000_D;
  } else {
    goto ltmp_1007_D;
  }

ltmp_1005_D:
  ltmp_712_7 = ltmp_688_7 + 1u;
  *(&ltmp_499_1036->field25[ltmp_712_7]) = ltmp_685_13;
  if ((ltmp_712_7 < ltmp_699_7)) {
    ltmp_713_7_2E = 0u;
    goto ltmp_1008_D;
  } else {
    goto ltmp_1006_D;
  }

  do {
ltmp_1008_D:
  ltmp_713_7 = ltmp_713_7_2E;
  ltmp_714_7 = (ltmp_713_7 + ltmp_712_7) + 1u;
  *(&ltmp_499_1036->field25[ltmp_714_7]) = ltmp_685_13;
  if ((ltmp_714_7 < ltmp_699_7)) {
    ltmp_713_7_2E = (ltmp_713_7 + 1u);
    goto ltmp_1008_D;
  } else {
    goto ltmp_1006_D;
  }

  } while (1);
  } while (1);
ltmp_969_D:
  ltmp_668_1032 = *ltmp_514_103A;
  ltmp_669_1032 = &ltmp_668_1032[ltmp_547_8];
  *ltmp_467_103A = ((struct l_struct_2E_M *)0);
  *ltmp_468_103A = ((struct l_struct_2E_M *)0);
  ltmp_670_1032 = *ltmp_514_103A;
  ltmp_671_8 = plzo_external_ptrdiff_M(ltmp_526_1032, ltmp_670_1032, 8u);
  ltmp_672_13 = *ltmp_505_1025;
  ltmp_673_1032 = *ltmp_514_103A;
  ltmp_674_8 = plzo_external_ptrdiff_M(ltmp_669_1032, ltmp_673_1032, 8u);
  ltmp_675_13 = *ltmp_505_1025;
  ltmp_676_13 = &ltmp_675_13[ltmp_674_8];
  ltmp_685_13_2E = ltmp_676_13;
  ltmp_686_7_2E = 2u;
  ltmp_687_7_2E = 2u;
  ltmp_688_7_2E = 2u;
  ltmp_689_1032_2E = (&ltmp_341_1032);
  ltmp_690_1032_2E = (&ltmp_341_1032);
  ltmp_691_1032_2E = ltmp_669_1032;
  ltmp_692_7_2E = 2u;
  ltmp_693_7_2E = 0u;
  goto ltmp_986_D;

ltmp_992_D:
  *(&ltmp_690_1032->field1) = ((struct l_struct_2E_M *)0);
  ltmp_728_7_2E = ltmp_706_7;
  goto ltmp_984_D;

ltmp_1007_D:
  *(&ltmp_690_1032->field1) = ltmp_691_1032;
  *(&ltmp_689_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_728_7_2E = ltmp_699_7;
  goto ltmp_984_D;

ltmp_1001_D:
  *(&ltmp_689_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_728_7_2E = ltmp_717_7;
  goto ltmp_984_D;

ltmp_1009_D:
  ltmp_726_1032 = *(&ltmp_691_1032->field0);
  *(&ltmp_690_1032->field1) = ltmp_726_1032;
  ltmp_727_1032 = *(&ltmp_691_1032->field1);
  *(&ltmp_689_1032->field0) = ltmp_727_1032;
  ltmp_728_7_2E = ltmp_699_7;
  goto ltmp_984_D;

ltmp_1004_D:
  ltmp_723_7 = ltmp_688_7 + 1u;
  *(&ltmp_499_1036->field25[ltmp_723_7]) = ltmp_685_13;
  if ((ltmp_723_7 < ltmp_699_7)) {
    ltmp_724_7_2E = 0u;
    goto ltmp_1010_D;
  } else {
    goto ltmp_1009_D;
  }

  do {
ltmp_1010_D:
  ltmp_724_7 = ltmp_724_7_2E;
  ltmp_725_7 = (ltmp_724_7 + ltmp_723_7) + 1u;
  *(&ltmp_499_1036->field25[ltmp_725_7]) = ltmp_685_13;
  if ((ltmp_725_7 < ltmp_699_7)) {
    ltmp_724_7_2E = (ltmp_724_7 + 1u);
    goto ltmp_1010_D;
  } else {
    goto ltmp_1009_D;
  }

  } while (1);
ltmp_923_D:
  *ltmp_525_17 = ltmp_732_7;
  goto ltmp_920_D;

ltmp_1011_D:
  ltmp_746_7 = *ltmp_525_17;
  if ((ltmp_746_7 == 1024u)) {
    goto ltmp_1012_D;
  } else {
    goto ltmp_917_D;
  }

  do {
ltmp_1013_D:
  ltmp_739_7 = ltmp_739_7_2E;
  ltmp_740_7 = ltmp_740_7_2E;
  ltmp_741_7 = ltmp_741_7_2E;
  ltmp_742_7 = ltmp_740_7 - ltmp_741_7;
  ltmp_743_13 = *(&ltmp_499_1036->field25[ltmp_742_7]);
  ltmp_744_8 = plzo_external_ptrdiff_uchar(ltmp_736_13, ltmp_743_13, 1u);
  *(&ltmp_520_1038->field1[ltmp_741_7]) = ((plzo_uint32_t )ltmp_744_8);
  ltmp_745_7 = ltmp_741_7 + 1u;
  if ((ltmp_745_7 > 7u)) {
    goto ltmp_1011_D;
  } else {
    goto ltmp_1014_D;
  }

ltmp_1014_D:
  if (((ltmp_742_7 + 4294967295u) < ltmp_739_7)) {
    goto ltmp_1011_D;
  } else {
    ltmp_739_7_2E = ltmp_739_7;
    ltmp_740_7_2E = ltmp_740_7;
    ltmp_741_7_2E = ltmp_745_7;
    goto ltmp_1013_D;
  }

  } while (1);
ltmp_921_D:
  ltmp_735_13 = *ltmp_505_1025;
  ltmp_736_13 = &ltmp_735_13[ltmp_519_7];
  ltmp_737_7 = *ltmp_517_17;
  if ((ltmp_733_7 > ltmp_737_7)) {
    goto ltmp_1015_D;
  } else {
    ltmp_739_7_2E = ltmp_734_7;
    ltmp_740_7_2E = ltmp_733_7;
    ltmp_741_7_2E = 0u;
    goto ltmp_1013_D;
  }

ltmp_1015_D:
  *ltmp_525_17 = ltmp_737_7;
  ltmp_738_7 = *ltmp_506_17;
  ltmp_739_7_2E = ltmp_738_7;
  ltmp_740_7_2E = ltmp_737_7;
  ltmp_741_7_2E = 0u;
  goto ltmp_1013_D;

ltmp_1016_D:
  ltmp_759_7 = ltmp_759_7_2E;
  if ((ltmp_746_7 < ltmp_759_7)) {
    goto ltmp_1017_D;
  } else {
    goto ltmp_917_D;
  }

ltmp_1012_D:
  ltmp_747_7 = *(&ltmp_520_1038->field1[0]);
  ltmp_748_8 = ((plzo_int32_t )(-(ltmp_747_7))) + ltmp_530_8;
  ltmp_749_7 = *ltmp_516_17;
  ltmp_750_7 = *ltmp_517_17;
  ltmp_751_7 = (((ltmp_749_7 <= ltmp_750_7)) ? (ltmp_749_7) : (ltmp_750_7));
  if ((ltmp_746_7 < ltmp_751_7)) {
    ltmp_752_7_2E = 0u;
    goto ltmp_1018_D;
  } else {
    ltmp_759_7_2E = ltmp_746_7;
    goto ltmp_1016_D;
  }

  do {
ltmp_1018_D:
  ltmp_752_7 = ltmp_752_7_2E;
  ltmp_753_7 = ltmp_752_7 + ltmp_746_7;
  ltmp_754_8 = (plzo_int32_t )ltmp_753_7;
  ltmp_755_3 = *(&ltmp_735_13[(ltmp_754_8 + ltmp_530_8)]);
  ltmp_756_3 = *(&ltmp_735_13[(ltmp_754_8 + ltmp_748_8)]);
  if ((ltmp_755_3 == ltmp_756_3)) {
    goto ltmp_1019_D;
  } else {
    ltmp_759_7_2E = ltmp_753_7;
    goto ltmp_1016_D;
  }

ltmp_1019_D:
  ltmp_757_7 = ltmp_753_7 + 1u;
  ltmp_758_7 = ltmp_752_7 + 1u;
  if ((ltmp_757_7 < ltmp_751_7)) {
    ltmp_752_7_2E = ltmp_758_7;
    goto ltmp_1018_D;
  } else {
    ltmp_759_7_2E = ltmp_757_7;
    goto ltmp_1016_D;
  }

  } while (1);
ltmp_1020_D:
  *ltmp_525_17 = ltmp_759_7;
  goto ltmp_917_D;

  do {
ltmp_1021_D:
  ltmp_765_7 = ltmp_765_7_2E;
  ltmp_766_7 = ltmp_766_7_2E;
  *(&ltmp_520_1038->field1[((ltmp_765_7 - ltmp_766_7) + 4294967295u)]) = ltmp_747_7;
  ltmp_767_7 = ltmp_766_7 + 1u;
  if ((ltmp_767_7 == ltmp_765_7)) {
    goto ltmp_1020_D;
  } else {
    ltmp_765_7_2E = ltmp_765_7;
    ltmp_766_7_2E = ltmp_767_7;
    goto ltmp_1021_D;
  }

  } while (1);
ltmp_1017_D:
  ltmp_760_7 = ltmp_759_7 - ltmp_746_7;
  if ((ltmp_760_7 < 8u)) {
    goto ltmp_1022_D;
  } else {
    ltmp_765_7_2E = 8u;
    ltmp_766_7_2E = 0u;
    goto ltmp_1021_D;
  }

  do {
ltmp_1023_D:
  ltmp_761_7 = ltmp_761_7_2E;
  ltmp_762_7 = ltmp_762_7_2E;
  ltmp_763_7 = (ltmp_761_7 - ltmp_762_7) + 4294967295u;
  ltmp_764_7 = *(&ltmp_520_1038->field1[(ltmp_763_7 - ltmp_760_7)]);
  *(&ltmp_520_1038->field1[ltmp_763_7]) = ltmp_764_7;
  if ((ltmp_763_7 > ltmp_760_7)) {
    ltmp_761_7_2E = ltmp_761_7;
    ltmp_762_7_2E = (ltmp_762_7 + 1u);
    goto ltmp_1023_D;
  } else {
    ltmp_765_7_2E = ltmp_763_7;
    ltmp_766_7_2E = 0u;
    goto ltmp_1021_D;
  }

  } while (1);
ltmp_1022_D:
  if ((ltmp_759_7 < 8u)) {
    goto ltmp_1024_D;
  } else {
    ltmp_761_7_2E = 8u;
    ltmp_762_7_2E = 0u;
    goto ltmp_1023_D;
  }

ltmp_1024_D:
  ltmp_761_7_2E = ltmp_759_7;
  ltmp_762_7_2E = 0u;
  goto ltmp_1023_D;

ltmp_914_D:
  *(&ltmp_520_1038->field0) = 0u;
  ltmp_768_1032 = ltmp_18_100A(ltmp_499_1036);
  *(&ltmp_768_1032->field1) = ((struct l_struct_2E_M *)0);
  *(&ltmp_768_1032->field0) = ((struct l_struct_2E_M *)0);
  goto ltmp_917_D;

ltmp_1025_D:
  ltmp_777_1038 = &ltmp_520_1038[ltmp_774_8];
  ltmp_778_7 = (ltmp_769_7 + ltmp_519_7) + 4294967295u;
  ltmp_779_7_2E = ltmp_778_7;
  ltmp_780_1038_2E = ltmp_777_1038;
  goto ltmp_916_D;

  do {
ltmp_1026_D:
  ltmp_773_7 = ltmp_773_7_2E;
  ltmp_774_8 = ((plzo_int32_t )ltmp_773_7) + 1;
  *(&ltmp_520_1038[ltmp_774_8].field0) = 0u;
  ltmp_775_1032 = ltmp_18_100A(ltmp_499_1036);
  *(&ltmp_775_1032->field1) = ((struct l_struct_2E_M *)0);
  *(&ltmp_775_1032->field0) = ((struct l_struct_2E_M *)0);
  ltmp_776_7 = ltmp_773_7 + 1u;
  if ((ltmp_776_7 == ltmp_771_7)) {
    goto ltmp_1025_D;
  } else {
    ltmp_773_7_2E = ltmp_776_7;
    goto ltmp_1026_D;
  }

  } while (1);
ltmp_918_D:
  ltmp_771_7 = ltmp_769_7 + 4294967295u;
  ltmp_772_7 = *ltmp_516_17;
  *ltmp_516_17 = (ltmp_772_7 - ltmp_771_7);
  ltmp_773_7_2E = 0u;
  goto ltmp_1026_D;

  } while (1);
ltmp_907_D:
  ltmp_503_17 = &ltmp_499_1036->field3;
  ltmp_504_17 = &ltmp_499_1036->field2;
  ltmp_505_1025 = &ltmp_499_1036->field0;
  ltmp_506_17 = &ltmp_499_1036->field12;
  ltmp_507_17 = &ltmp_499_1036->field14;
  ltmp_508_1025 = &ltmp_499_1036->field25[1];
  ltmp_509_1025 = &ltmp_499_1036->field25[2];
  ltmp_510_18 = &ltmp_499_1036->field22;
  ltmp_511_17 = &ltmp_499_1036->field1;
  ltmp_512_17 = &ltmp_499_1036->field21;
  ltmp_513_17 = &ltmp_499_1036->field19;
  ltmp_514_103A = &ltmp_499_1036->field18;
  ltmp_515_17 = &ltmp_499_1036->field15;
  ltmp_516_17 = &ltmp_499_1036->field4;
  ltmp_517_17 = &ltmp_499_1036->field13;
  ltmp_518_17 = &ltmp_499_1036->field16;
  ltmp_519_7_2E = ltmp_500_7;
  ltmp_520_1038_2E = ltmp_497_1038;
  goto ltmp_913_D;

ltmp_905_D:
  ltmp_788_1029 = *ltmp_350_102C;
  ltmp_789_102A = *(&ltmp_788_1029[11]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_789_102A)(ltmp_339_1023, ltmp_469_7, ltmp_787_7);
  goto ltmp_902_D;

ltmp_903_D:
  ltmp_791_1029 = *ltmp_350_102C;
  ltmp_792_102A = *(&ltmp_791_1029[19]);
  ltmp_793_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_792_102A)(ltmp_339_1023, ltmp_469_7, ltmp_787_7);
  ltmp_794_7_2E = ltmp_793_7;
  goto ltmp_901_D;

ltmp_900_D:
  if ((ltmp_359_8 > 1)) {
    goto ltmp_898_D;
  } else {
    goto ltmp_896_D;
  }

ltmp_1027_D:
  ltmp_803_102A = *(&ltmp_802_1029[14]);
  ltmp_804_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_803_102A)(ltmp_339_1023, ltmp_469_7, ltmp_794_7);
  goto ltmp_896_D;

ltmp_898_D:
  ltmp_802_1029 = *ltmp_350_102C;
  if ((ltmp_359_8 == 2)) {
    goto ltmp_1027_D;
  } else {
    goto ltmp_1028_D;
  }

ltmp_1029_D:
  ltmp_808_102A = *(&ltmp_807_1029[13]);
  ltmp_809_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_808_102A)(ltmp_339_1023, ltmp_469_7, ltmp_794_7);
  goto ltmp_896_D;

ltmp_1028_D:
  ltmp_805_102A = *(&ltmp_802_1029[18]);
  ltmp_806_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *))ltmp_805_102A)(ltmp_339_1023);
  ltmp_807_1029 = *ltmp_350_102C;
  if ((ltmp_806_7 == 0u)) {
    goto ltmp_1030_D;
  } else {
    goto ltmp_1029_D;
  }

ltmp_1030_D:
  ltmp_810_102A = *(&ltmp_807_1029[12]);
  ltmp_811_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_810_102A)(ltmp_339_1023, ltmp_469_7, ltmp_794_7);
  goto ltmp_896_D;

ltmp_895_D:
  if ((ltmp_359_8 == 0)) {
    goto ltmp_891_D;
  } else {
    goto ltmp_893_D;
  }

ltmp_1031_D:
  ltmp_822_102A = *(&ltmp_821_1029[16]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))ltmp_822_102A)(ltmp_339_1023, ltmp_469_7, ltmp_794_7, 1);
  goto ltmp_889_D;

ltmp_893_D:
  ltmp_821_1029 = *ltmp_350_102C;
  if ((ltmp_359_8 == 1)) {
    goto ltmp_1031_D;
  } else {
    goto ltmp_1032_D;
  }

ltmp_1032_D:
  ltmp_823_102A = *(&ltmp_821_1029[15]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_823_102A)(ltmp_339_1023, ltmp_469_7, ltmp_794_7);
  goto ltmp_889_D;

ltmp_1033_D:
  ltmp_832_101A(ltmp_831_1026, ltmp_794_7, ltmp_829_7, 3);
  *ltmp_460_17 = ltmp_794_7;
  ltmp_469_7_2E = ltmp_794_7;
  ltmp_470_8_2E = 0;
  goto ltmp_887_D;

ltmp_1034_D:
  ltmp_832_101A = *(&ltmp_831_1026->field2);
  if ((ltmp_832_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))0))) {
    goto ltmp_1035_D;
  } else {
    goto ltmp_1033_D;
  }

ltmp_890_D:
  ltmp_826_13 = *ltmp_458_1025;
  ltmp_827_13 = *ltmp_459_1025;
  ltmp_828_8 = plzo_external_ptrdiff_uchar(ltmp_826_13, ltmp_827_13, 1u);
  ltmp_829_7 = (plzo_uint32_t )ltmp_828_8;
  ltmp_830_14 = *ltmp_442_102D;
  ltmp_831_1026 = (struct plzo_callback_t *)ltmp_830_14;
  if ((ltmp_830_14 == ((signed char *)0))) {
    goto ltmp_1035_D;
  } else {
    goto ltmp_1034_D;
  }

ltmp_1035_D:
  *ltmp_460_17 = ltmp_794_7;
  ltmp_469_7_2E = ltmp_794_7;
  ltmp_470_8_2E = 0;
  goto ltmp_887_D;

  } while (1);
ltmp_886_D:
  ltmp_833_8 = ltmp_833_8_2E;
  ltmp_834_102F = *ltmp_441_1035;
  if ((ltmp_834_102F == ((struct l_struct_2E_A9 *)0))) {
    goto ltmp_1036_D;
  } else {
    goto ltmp_1037_D;
  }

ltmp_1037_D:
  ltmp_835_1019 = *(&ltmp_834_102F->field2.field1);
  ltmp_836_1030 = &ltmp_339_1023->field20.field3;
  ltmp_837_1031 = *ltmp_836_1030;
  ltmp_835_1019((&ltmp_834_102F->field2), ((signed char *)ltmp_837_1031));
  *ltmp_836_1030 = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  ltmp_19_100B(ltmp_360_1034);
  return ltmp_833_8;
ltmp_1036_D:
  ltmp_19_100B(ltmp_360_1034);
  return ltmp_833_8;
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
  ltmp_1044_102A = *(&ltmp_1043_1029[3]);
  ltmp_1045_13 = *(&ltmp_1038_1023->field8);
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint32_t , plzo_uint32_t ))ltmp_1044_102A)(ltmp_1038_1023, (&ltmp_1045_13[ltmp_1042_7]), ltmp_1040_7, 0u);
  *ltmp_1039_17 = 0u;
  goto ltmp_1048_D;

ltmp_1048_D:
  ltmp_1046_1029 = *(&ltmp_1038_1023->field0);
  ltmp_1047_102A = *(&ltmp_1046_1029[5]);
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
  ltmp_1065_1038 = &ltmp_1058_1038[1];
  ltmp_1054_1038_2E = ltmp_1065_1038;
  goto ltmp_1072_D;

ltmp_1074_D:
  ltmp_1063_7 = *(&ltmp_1058_1038->field1[0]);
  ltmp_1064_7 = *(&ltmp_1050_1023->field4[ltmp_1061_7]);
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
  ltmp_1066_1038 = &ltmp_1058_1038[1];
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
  plzo_uint32_t ltmp_1086_7;
  plzo_uint32_t ltmp_1086_7_2E;
  plzo_uint32_t ltmp_1087_7;
  unsigned char *ltmp_1088_13;
  plzo_uint32_t ltmp_1089_7;
  plzo_uint32_t ltmp_1089_7_2E;
  plzo_uint32_t ltmp_1090_7;
  plzo_uint32_t ltmp_1091_7;
  struct l_struct_2E_A7 **ltmp_1092_1037;
  plzo_uint32_t *ltmp_1093_17;
  plzo_uint32_t *ltmp_1094_17;
  plzo_uint32_t *ltmp_1095_17;
  plzo_uint32_t *ltmp_1096_17;
  plzo_int32_t  (***ltmp_1097_102C) ();
  plzo_uint32_t *ltmp_1098_17;
  plzo_uint32_t *ltmp_1099_17;
  plzo_int32_t *ltmp_1100_18;
  plzo_uint32_t *ltmp_1101_17;
  plzo_uint32_t *ltmp_1102_17;
  plzo_uint32_t ltmp_1103_7;
  plzo_uint32_t ltmp_1103_7_2E;
  plzo_uint32_t ltmp_1104_7;
  struct l_struct_2E_A7 *ltmp_1105_1038;
  plzo_uint32_t ltmp_1106_7;
  plzo_uint32_t ltmp_1107_7;
  plzo_uint32_t ltmp_1108_7;
  plzo_uint32_t ltmp_1109_7;
  plzo_uint32_t ltmp_1110_7;
  plzo_uint32_t ltmp_1111_7;
  plzo_uint32_t ltmp_1112_7;
  plzo_uint32_t ltmp_1113_7;
  plzo_uint32_t ltmp_1113_7_2E;
  plzo_uint32_t ltmp_1114_7;
  plzo_uint32_t ltmp_1114_7_2E;
  plzo_uint32_t ltmp_1115_7;
  plzo_uint32_t ltmp_1116_7;
  plzo_uint32_t ltmp_1117_7;
  plzo_uint32_t ltmp_1118_7;
  plzo_uint32_t ltmp_1118_7_2E;
  plzo_uint32_t ltmp_1119_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1120_1031;
  plzo_uint32_t ltmp_1121_7;
  plzo_uint32_t ltmp_1122_7;
  plzo_int32_t  (**ltmp_1123_1029) ();
  plzo_int32_t  (*ltmp_1124_102A) ();
  plzo_uint32_t ltmp_1125_7;
  plzo_uint32_t *ltmp_1126_17;
  plzo_uint32_t *ltmp_1127_17;
  plzo_uint32_t *ltmp_1128_17;
  plzo_uint32_t *ltmp_1129_17;
  plzo_uint32_t ltmp_1130_7;
  plzo_uint32_t ltmp_1131_7;
  plzo_uint32_t ltmp_1132_7;
  plzo_uint32_t ltmp_1132_7_2E;
  plzo_uint32_t ltmp_1133_7;
  plzo_int32_t  (**ltmp_1134_1029) ();
  plzo_int32_t  (*ltmp_1135_102A) ();
  plzo_uint32_t ltmp_1136_7;
  plzo_uint32_t ltmp_1137_7;
  plzo_uint32_t ltmp_1138_7;
  plzo_int32_t ltmp_1139_8;
  plzo_uint32_t ltmp_1140_7;
  plzo_uint32_t ltmp_1141_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1142_1031;
  plzo_uint32_t ltmp_1143_7;

  ltmp_1079_7 = ltmp_1077_7 - ltmp_1076_7;
  *(&ltmp_1075_1023->field20.field2) = (ltmp_1079_7 + 1u);
  ltmp_1080_1030 = &ltmp_1075_1023->field20.field3;
  ltmp_1081_1031 = *ltmp_1080_1030;
  ltmp_1082_17 = &ltmp_1075_1023->field20.field1;
  ltmp_1083_7 = *ltmp_1082_17;
  ltmp_1084_7 = ltmp_1083_7 << 4;
  ltmp_1085_13 = (unsigned char *)(&ltmp_1081_1031[(ltmp_1083_7*ltmp_1079_7)]);
  if ((ltmp_1084_7 == 0u)) {
    goto ltmp_1144_D;
  } else {
    ltmp_1086_7_2E = 0u;
    goto ltmp_1145_D;
  }

  do {
ltmp_1145_D:
  ltmp_1086_7 = ltmp_1086_7_2E;
  *(&ltmp_1085_13[ltmp_1086_7]) = 0;
  ltmp_1087_7 = ltmp_1086_7 + 1u;
  if ((ltmp_1087_7 == ltmp_1084_7)) {
    goto ltmp_1144_D;
  } else {
    ltmp_1086_7_2E = ltmp_1087_7;
    goto ltmp_1145_D;
  }

  } while (1);
ltmp_1144_D:
  ltmp_1088_13 = (unsigned char *)(&ltmp_1078_103B);
  ltmp_1089_7_2E = 0u;
  goto ltmp_1146_D;

  do {
ltmp_1146_D:
  ltmp_1089_7 = ltmp_1089_7_2E;
  *(&ltmp_1088_13[ltmp_1089_7]) = 0;
  ltmp_1090_7 = ltmp_1089_7 + 1u;
  if ((ltmp_1090_7 == 32u)) {
    goto ltmp_1147_D;
  } else {
    ltmp_1089_7_2E = ltmp_1090_7;
    goto ltmp_1146_D;
  }

  } while (1);
ltmp_1147_D:
  *(&ltmp_1078_103B.field3) = ltmp_1076_7;
  *(&ltmp_1078_103B.field4) = ltmp_1077_7;
  *(&ltmp_1078_103B.field5) = ltmp_1079_7;
  if ((ltmp_1077_7 == ltmp_1076_7)) {
    goto ltmp_1148_D;
  } else {
    goto ltmp_1149_D;
  }

ltmp_1149_D:
  ltmp_1092_1037 = &ltmp_1075_1023->field19.field0;
  ltmp_1093_17 = &ltmp_1075_1023->field7.field12;
  ltmp_1094_17 = &ltmp_1075_1023->field7.field13;
  ltmp_1095_17 = &ltmp_1075_1023->field7.field6;
  ltmp_1096_17 = &ltmp_1075_1023->field21;
  ltmp_1097_102C = &ltmp_1075_1023->field0;
  ltmp_1098_17 = &ltmp_1078_103B.field0;
  ltmp_1099_17 = &ltmp_1075_1023->field7.field3;
  ltmp_1100_18 = &ltmp_1075_1023->field7.field14;
  ltmp_1101_17 = &ltmp_1078_103B.field1;
  ltmp_1102_17 = &ltmp_1078_103B.field2;
  ltmp_1103_7_2E = 0u;
  goto ltmp_1150_D;

  do {
ltmp_1150_D:
  ltmp_1103_7 = ltmp_1103_7_2E;
  ltmp_1104_7 = (ltmp_1079_7 - ltmp_1103_7) + 4294967295u;
  ltmp_1105_1038 = *ltmp_1092_1037;
  ltmp_1106_7 = *(&ltmp_1105_1038[ltmp_1104_7].field0);
  ltmp_1107_7 = *ltmp_1093_17;
  ltmp_1108_7 = *ltmp_1094_17;
  ltmp_1109_7 = *ltmp_1095_17;
  if ((ltmp_1109_7 > ltmp_1106_7)) {
    ltmp_1113_7_2E = ltmp_1108_7;
    ltmp_1114_7_2E = ltmp_1107_7;
    goto ltmp_1151_D;
  } else {
    goto ltmp_1152_D;
  }

ltmp_1153_D:
  ltmp_1091_7 = ltmp_1103_7 + 1u;
  if ((ltmp_1091_7 == ltmp_1079_7)) {
    goto ltmp_1148_D;
  } else {
    ltmp_1103_7_2E = ltmp_1091_7;
    goto ltmp_1150_D;
  }

ltmp_1151_D:
  ltmp_1113_7 = ltmp_1113_7_2E;
  ltmp_1114_7 = ltmp_1114_7_2E;
  ltmp_1115_7 = ((((ltmp_1114_7 + ltmp_1113_7) < ltmp_1106_7)) ? ((ltmp_1106_7 - ltmp_1113_7)) : (ltmp_1114_7));
  ltmp_1116_7 = *ltmp_1096_17;
  if ((ltmp_1116_7 == 0u)) {
    goto ltmp_1153_D;
  } else {
    ltmp_1118_7_2E = 0u;
    goto ltmp_1154_D;
  }

ltmp_1152_D:
  ltmp_1110_7 = *ltmp_1099_17;
  ltmp_1111_7 = ltmp_1110_7 + 1u;
  ltmp_1112_7 = (plzo_uint32_t )(ltmp_1108_7 != 0u);
  ltmp_1113_7_2E = ltmp_1112_7;
  ltmp_1114_7_2E = ltmp_1111_7;
  goto ltmp_1151_D;

  do {
ltmp_1154_D:
  ltmp_1118_7 = ltmp_1118_7_2E;
  ltmp_1119_7 = (ltmp_1116_7 - ltmp_1118_7) + 4294967295u;
  ltmp_1120_1031 = *ltmp_1080_1030;
  ltmp_1121_7 = *ltmp_1082_17;
  ltmp_1122_7 = (ltmp_1121_7*ltmp_1104_7) + ltmp_1119_7;
  ltmp_1123_1029 = *ltmp_1097_102C;
  ltmp_1124_102A = *(&ltmp_1123_1029[17]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t ))ltmp_1124_102A)(ltmp_1075_1023, (&ltmp_1078_103B), 0u, ltmp_1104_7, ltmp_1119_7);
  ltmp_1125_7 = *ltmp_1098_17;
  ltmp_1126_17 = &ltmp_1120_1031[ltmp_1122_7].field0;
  *ltmp_1126_17 = ltmp_1125_7;
  ltmp_1127_17 = &ltmp_1120_1031[ltmp_1122_7].field1;
  *ltmp_1127_17 = 0u;
  ltmp_1128_17 = &ltmp_1120_1031[ltmp_1122_7].field2;
  *ltmp_1128_17 = 0u;
  ltmp_1129_17 = &ltmp_1120_1031[ltmp_1122_7].field3;
  *ltmp_1129_17 = 0u;
  ltmp_1130_7 = *ltmp_1099_17;
  if ((ltmp_1130_7 > ltmp_1106_7)) {
    goto ltmp_1155_D;
  } else {
    ltmp_1132_7_2E = ltmp_1130_7;
    goto ltmp_1156_D;
  }

ltmp_1155_D:
  ltmp_1117_7 = ltmp_1118_7 + 1u;
  if ((ltmp_1117_7 == ltmp_1116_7)) {
    goto ltmp_1153_D;
  } else {
    ltmp_1118_7_2E = ltmp_1117_7;
    goto ltmp_1154_D;
  }

  do {
ltmp_1156_D:
  ltmp_1132_7 = ltmp_1132_7_2E;
  ltmp_1133_7 = (((ltmp_1132_7 == ltmp_1114_7)) ? (ltmp_1115_7) : (ltmp_1132_7));
  ltmp_1134_1029 = *ltmp_1097_102C;
  ltmp_1135_102A = *(&ltmp_1134_1029[17]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t ))ltmp_1135_102A)(ltmp_1075_1023, (&ltmp_1078_103B), ltmp_1133_7, ltmp_1104_7, ltmp_1119_7);
  ltmp_1136_7 = *ltmp_1098_17;
  ltmp_1137_7 = *ltmp_1126_17;
  if ((ltmp_1136_7 > ltmp_1137_7)) {
    goto ltmp_1157_D;
  } else {
    goto ltmp_1158_D;
  }

ltmp_1157_D:
  ltmp_1131_7 = ltmp_1133_7 + 1u;
  if ((ltmp_1131_7 > ltmp_1106_7)) {
    goto ltmp_1155_D;
  } else {
    ltmp_1132_7_2E = ltmp_1131_7;
    goto ltmp_1156_D;
  }

ltmp_1159_D:
  if (((ltmp_1139_8 & 1) == 0)) {
    goto ltmp_1160_D;
  } else {
    goto ltmp_1157_D;
  }

ltmp_1161_D:
  ltmp_1138_7 = *ltmp_1127_17;
  ltmp_1139_8 = *ltmp_1100_18;
  if ((ltmp_1138_7 == 0u)) {
    goto ltmp_1159_D;
  } else {
    goto ltmp_1162_D;
  }

ltmp_1158_D:
  if ((ltmp_1136_7 == ltmp_1137_7)) {
    goto ltmp_1161_D;
  } else {
    goto ltmp_1160_D;
  }

ltmp_1162_D:
  if (((((plzo_uint32_t )(ltmp_1139_8 >> 1)) & 1u) == 0u)) {
    goto ltmp_1160_D;
  } else {
    goto ltmp_1157_D;
  }

ltmp_1160_D:
  *ltmp_1126_17 = ltmp_1136_7;
  *ltmp_1127_17 = ltmp_1133_7;
  ltmp_1140_7 = *ltmp_1101_17;
  *ltmp_1128_17 = ltmp_1140_7;
  ltmp_1141_7 = *ltmp_1102_17;
  *ltmp_1129_17 = ltmp_1141_7;
  goto ltmp_1157_D;

  } while (1);
  } while (1);
  } while (1);
ltmp_1148_D:
  ltmp_1142_1031 = *ltmp_1080_1030;
  ltmp_1143_7 = *(&ltmp_1142_1031->field0);
  return ltmp_1143_7;
}


static plzo_uint32_t ltmp_13_1008(struct l_struct_2E_E *ltmp_1163_1023, plzo_uint32_t ltmp_1164_7, plzo_uint32_t ltmp_1165_7) {
  struct l_struct_2E_E_3A__3A_A4 ltmp_1166_103B;
  plzo_uint32_t ltmp_1167_7;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1168_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1169_1031;
  plzo_uint32_t ltmp_1170_7;
  plzo_uint32_t ltmp_1171_7;
  unsigned char *ltmp_1172_13;
  plzo_uint32_t ltmp_1173_7;
  plzo_uint32_t ltmp_1173_7_2E;
  plzo_uint32_t ltmp_1174_7;
  plzo_int32_t  (***ltmp_1175_102C) ();
  plzo_int32_t  (**ltmp_1176_1029) ();
  plzo_int32_t  (*ltmp_1177_102A) ();
  plzo_uint32_t ltmp_1178_7;
  unsigned char *ltmp_1179_13;
  plzo_uint32_t ltmp_1180_7;
  plzo_uint32_t ltmp_1180_7_2E;
  plzo_uint32_t ltmp_1181_7;
  plzo_uint32_t ltmp_1182_7;
  struct l_struct_2E_A7 **ltmp_1183_1037;
  plzo_uint32_t *ltmp_1184_17;
  plzo_uint32_t *ltmp_1185_17;
  plzo_uint32_t *ltmp_1186_17;
  plzo_uint32_t *ltmp_1187_17;
  plzo_uint32_t *ltmp_1188_17;
  plzo_int32_t *ltmp_1189_18;
  plzo_uint32_t *ltmp_1190_17;
  plzo_uint32_t *ltmp_1191_17;
  plzo_uint32_t ltmp_1192_7;
  plzo_uint32_t ltmp_1192_7_2E;
  plzo_uint32_t ltmp_1193_7;
  struct l_struct_2E_A7 *ltmp_1194_1038;
  plzo_uint32_t ltmp_1195_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1196_1031;
  plzo_uint32_t ltmp_1197_7;
  plzo_uint32_t *ltmp_1198_17;
  plzo_uint32_t *ltmp_1199_17;
  plzo_uint32_t *ltmp_1200_17;
  plzo_uint32_t *ltmp_1201_17;
  plzo_uint32_t ltmp_1202_7;
  plzo_uint32_t ltmp_1203_7;
  plzo_uint32_t ltmp_1204_7;
  plzo_uint32_t ltmp_1205_7;
  plzo_uint32_t ltmp_1206_7;
  plzo_uint32_t ltmp_1207_7;
  plzo_uint32_t ltmp_1208_7;
  plzo_uint32_t ltmp_1208_7_2E;
  plzo_uint32_t ltmp_1209_7;
  plzo_uint32_t ltmp_1209_7_2E;
  plzo_uint32_t ltmp_1210_7;
  plzo_uint32_t ltmp_1211_7;
  plzo_uint32_t ltmp_1212_7;
  plzo_uint32_t ltmp_1212_7_2E;
  plzo_uint32_t ltmp_1213_7;
  plzo_int32_t  (**ltmp_1214_1029) ();
  plzo_int32_t  (*ltmp_1215_102A) ();
  plzo_uint32_t ltmp_1216_7;
  plzo_uint32_t ltmp_1217_7;
  plzo_uint32_t ltmp_1218_7;
  plzo_int32_t ltmp_1219_8;
  plzo_uint32_t ltmp_1220_7;
  plzo_uint32_t ltmp_1221_7;
  plzo_uint32_t ltmp_1222_7;
  plzo_uint32_t ltmp_1223_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1224_1031;
  plzo_uint32_t ltmp_1225_7;

  ltmp_1167_7 = ltmp_1165_7 - ltmp_1164_7;
  *(&ltmp_1163_1023->field20.field2) = (ltmp_1167_7 + 1u);
  ltmp_1168_1030 = &ltmp_1163_1023->field20.field3;
  ltmp_1169_1031 = *ltmp_1168_1030;
  ltmp_1170_7 = *(&ltmp_1163_1023->field20.field1);
  ltmp_1171_7 = ltmp_1170_7 << 4;
  ltmp_1172_13 = (unsigned char *)(&ltmp_1169_1031[(ltmp_1170_7*ltmp_1167_7)]);
  if ((ltmp_1171_7 == 0u)) {
    goto ltmp_1226_D;
  } else {
    ltmp_1173_7_2E = 0u;
    goto ltmp_1227_D;
  }

  do {
ltmp_1227_D:
  ltmp_1173_7 = ltmp_1173_7_2E;
  *(&ltmp_1172_13[ltmp_1173_7]) = 0;
  ltmp_1174_7 = ltmp_1173_7 + 1u;
  if ((ltmp_1174_7 == ltmp_1171_7)) {
    goto ltmp_1226_D;
  } else {
    ltmp_1173_7_2E = ltmp_1174_7;
    goto ltmp_1227_D;
  }

  } while (1);
ltmp_1226_D:
  ltmp_1175_102C = &ltmp_1163_1023->field0;
  ltmp_1176_1029 = *ltmp_1175_102C;
  ltmp_1177_102A = *(&ltmp_1176_1029[18]);
  ltmp_1178_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *))ltmp_1177_102A)(ltmp_1163_1023);
  ltmp_1179_13 = (unsigned char *)(&ltmp_1166_103B);
  ltmp_1180_7_2E = 0u;
  goto ltmp_1228_D;

  do {
ltmp_1228_D:
  ltmp_1180_7 = ltmp_1180_7_2E;
  *(&ltmp_1179_13[ltmp_1180_7]) = 0;
  ltmp_1181_7 = ltmp_1180_7 + 1u;
  if ((ltmp_1181_7 == 32u)) {
    goto ltmp_1229_D;
  } else {
    ltmp_1180_7_2E = ltmp_1181_7;
    goto ltmp_1228_D;
  }

  } while (1);
ltmp_1229_D:
  *(&ltmp_1166_103B.field3) = ltmp_1164_7;
  *(&ltmp_1166_103B.field4) = ltmp_1165_7;
  *(&ltmp_1166_103B.field5) = ltmp_1167_7;
  if ((ltmp_1165_7 == ltmp_1164_7)) {
    goto ltmp_1230_D;
  } else {
    goto ltmp_1231_D;
  }

ltmp_1231_D:
  ltmp_1183_1037 = &ltmp_1163_1023->field19.field0;
  ltmp_1184_17 = &ltmp_1163_1023->field7.field12;
  ltmp_1185_17 = &ltmp_1163_1023->field7.field13;
  ltmp_1186_17 = &ltmp_1163_1023->field7.field6;
  ltmp_1187_17 = &ltmp_1163_1023->field7.field3;
  ltmp_1188_17 = &ltmp_1166_103B.field0;
  ltmp_1189_18 = &ltmp_1163_1023->field7.field14;
  ltmp_1190_17 = &ltmp_1166_103B.field1;
  ltmp_1191_17 = &ltmp_1166_103B.field2;
  ltmp_1192_7_2E = 0u;
  goto ltmp_1232_D;

  do {
ltmp_1232_D:
  ltmp_1192_7 = ltmp_1192_7_2E;
  ltmp_1193_7 = (ltmp_1167_7 - ltmp_1192_7) + 4294967295u;
  ltmp_1194_1038 = *ltmp_1183_1037;
  ltmp_1195_7 = *(&ltmp_1194_1038[ltmp_1193_7].field0);
  ltmp_1196_1031 = *ltmp_1168_1030;
  ltmp_1197_7 = *(&ltmp_1196_1031[(((plzo_int32_t )ltmp_1193_7) + 1)].field0);
  ltmp_1198_17 = &ltmp_1196_1031[ltmp_1193_7].field0;
  *ltmp_1198_17 = (ltmp_1197_7 + ltmp_1178_7);
  ltmp_1199_17 = &ltmp_1196_1031[ltmp_1193_7].field1;
  *ltmp_1199_17 = 0u;
  ltmp_1200_17 = &ltmp_1196_1031[ltmp_1193_7].field2;
  *ltmp_1200_17 = 0u;
  ltmp_1201_17 = &ltmp_1196_1031[ltmp_1193_7].field3;
  *ltmp_1201_17 = 0u;
  if ((ltmp_1195_7 == 0u)) {
    goto ltmp_1233_D;
  } else {
    goto ltmp_1234_D;
  }

ltmp_1233_D:
  ltmp_1182_7 = ltmp_1192_7 + 1u;
  if ((ltmp_1182_7 == ltmp_1167_7)) {
    goto ltmp_1230_D;
  } else {
    ltmp_1192_7_2E = ltmp_1182_7;
    goto ltmp_1232_D;
  }

ltmp_1235_D:
  ltmp_1208_7 = ltmp_1208_7_2E;
  ltmp_1209_7 = ltmp_1209_7_2E;
  ltmp_1210_7 = ((((ltmp_1209_7 + ltmp_1208_7) < ltmp_1195_7)) ? ((ltmp_1195_7 - ltmp_1209_7)) : (ltmp_1208_7));
  ltmp_1211_7 = *ltmp_1187_17;
  if ((ltmp_1211_7 > ltmp_1195_7)) {
    goto ltmp_1233_D;
  } else {
    ltmp_1212_7_2E = ltmp_1211_7;
    goto ltmp_1236_D;
  }

ltmp_1234_D:
  ltmp_1202_7 = *ltmp_1184_17;
  ltmp_1203_7 = *ltmp_1185_17;
  ltmp_1204_7 = *ltmp_1186_17;
  if ((ltmp_1204_7 > ltmp_1195_7)) {
    ltmp_1208_7_2E = ltmp_1202_7;
    ltmp_1209_7_2E = ltmp_1203_7;
    goto ltmp_1235_D;
  } else {
    goto ltmp_1237_D;
  }

ltmp_1237_D:
  ltmp_1205_7 = *ltmp_1187_17;
  ltmp_1206_7 = ltmp_1205_7 + 1u;
  ltmp_1207_7 = (plzo_uint32_t )(ltmp_1203_7 != 0u);
  ltmp_1208_7_2E = ltmp_1206_7;
  ltmp_1209_7_2E = ltmp_1207_7;
  goto ltmp_1235_D;

  do {
ltmp_1236_D:
  ltmp_1212_7 = ltmp_1212_7_2E;
  ltmp_1213_7 = (((ltmp_1212_7 == ltmp_1208_7)) ? (ltmp_1210_7) : (ltmp_1212_7));
  ltmp_1214_1029 = *ltmp_1175_102C;
  ltmp_1215_102A = *(&ltmp_1214_1029[17]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t ))ltmp_1215_102A)(ltmp_1163_1023, (&ltmp_1166_103B), ltmp_1213_7, ltmp_1193_7, 0u);
  ltmp_1216_7 = *ltmp_1188_17;
  ltmp_1217_7 = *ltmp_1198_17;
  if ((ltmp_1216_7 > ltmp_1217_7)) {
    goto ltmp_1238_D;
  } else {
    goto ltmp_1239_D;
  }

ltmp_1240_D:
  *ltmp_1198_17 = ltmp_1216_7;
  *ltmp_1199_17 = ltmp_1213_7;
  ltmp_1220_7 = *ltmp_1190_17;
  *ltmp_1200_17 = ltmp_1220_7;
  ltmp_1221_7 = *ltmp_1191_17;
  *ltmp_1201_17 = ltmp_1221_7;
  ltmp_1222_7 = ltmp_1213_7 + 1u;
  if ((ltmp_1222_7 > ltmp_1195_7)) {
    goto ltmp_1233_D;
  } else {
    ltmp_1212_7_2E = ltmp_1222_7;
    goto ltmp_1236_D;
  }

ltmp_1239_D:
  if ((ltmp_1216_7 == ltmp_1217_7)) {
    goto ltmp_1241_D;
  } else {
    goto ltmp_1240_D;
  }

ltmp_1242_D:
  if (((ltmp_1219_8 & 1) == 0)) {
    goto ltmp_1240_D;
  } else {
    goto ltmp_1238_D;
  }

ltmp_1241_D:
  ltmp_1218_7 = *ltmp_1199_17;
  ltmp_1219_8 = *ltmp_1189_18;
  if ((ltmp_1218_7 == 0u)) {
    goto ltmp_1242_D;
  } else {
    goto ltmp_1243_D;
  }

ltmp_1243_D:
  if (((((plzo_uint32_t )(ltmp_1219_8 >> 1)) & 1u) == 0u)) {
    goto ltmp_1240_D;
  } else {
    goto ltmp_1238_D;
  }

ltmp_1238_D:
  ltmp_1223_7 = ltmp_1213_7 + 1u;
  if ((ltmp_1223_7 > ltmp_1195_7)) {
    goto ltmp_1233_D;
  } else {
    ltmp_1212_7_2E = ltmp_1223_7;
    goto ltmp_1236_D;
  }

  } while (1);
  } while (1);
ltmp_1230_D:
  ltmp_1224_1031 = *ltmp_1168_1030;
  ltmp_1225_7 = *(&ltmp_1224_1031->field0);
  return ltmp_1225_7;
}


static plzo_uint32_t ltmp_14_1008(struct l_struct_2E_E *ltmp_1244_1023, plzo_uint32_t ltmp_1245_7, plzo_uint32_t ltmp_1246_7) {
  struct l_struct_2E_E_3A__3A_A4 ltmp_1247_103B;
  struct l_struct_2E_E_3A__3A_A4 ltmp_1248_103B;
  plzo_uint32_t ltmp_1249_7;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1250_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1251_1031;
  plzo_uint32_t ltmp_1252_7;
  plzo_uint32_t ltmp_1253_7;
  unsigned char *ltmp_1254_13;
  plzo_uint32_t ltmp_1255_7;
  plzo_uint32_t ltmp_1255_7_2E;
  plzo_uint32_t ltmp_1256_7;
  plzo_int32_t  (***ltmp_1257_102C) ();
  plzo_int32_t  (**ltmp_1258_1029) ();
  plzo_int32_t  (*ltmp_1259_102A) ();
  plzo_uint32_t ltmp_1260_7;
  unsigned char *ltmp_1261_13;
  plzo_uint32_t ltmp_1262_7;
  plzo_uint32_t ltmp_1262_7_2E;
  plzo_uint32_t ltmp_1263_7;
  unsigned char *ltmp_1264_13;
  plzo_uint32_t ltmp_1265_7;
  plzo_uint32_t ltmp_1265_7_2E;
  plzo_uint32_t ltmp_1266_7;
  struct l_struct_2E_A7 **ltmp_1267_1037;
  plzo_uint32_t *ltmp_1268_17;
  plzo_uint32_t *ltmp_1269_17;
  plzo_uint32_t ltmp_1270_7;
  plzo_uint32_t ltmp_1270_7_2E;
  plzo_uint32_t ltmp_1271_7;
  struct l_struct_2E_A7 *ltmp_1272_1038;
  plzo_uint32_t ltmp_1273_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1274_1031;
  plzo_uint32_t ltmp_1275_7;
  plzo_uint32_t ltmp_1276_7;
  plzo_int32_t  (**ltmp_1277_1029) ();
  plzo_int32_t  (*ltmp_1278_102A) ();
  plzo_uint32_t ltmp_1279_7;
  plzo_uint32_t *ltmp_1280_17;
  plzo_uint32_t ltmp_1281_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1282_1031;
  plzo_uint32_t ltmp_1283_7;

  ltmp_1249_7 = ltmp_1246_7 - ltmp_1245_7;
  *(&ltmp_1244_1023->field20.field2) = (ltmp_1249_7 + 1u);
  ltmp_1250_1030 = &ltmp_1244_1023->field20.field3;
  ltmp_1251_1031 = *ltmp_1250_1030;
  ltmp_1252_7 = *(&ltmp_1244_1023->field20.field1);
  ltmp_1253_7 = ltmp_1252_7 << 4;
  ltmp_1254_13 = (unsigned char *)(&ltmp_1251_1031[(ltmp_1252_7*ltmp_1249_7)]);
  if ((ltmp_1253_7 == 0u)) {
    goto ltmp_1284_D;
  } else {
    ltmp_1255_7_2E = 0u;
    goto ltmp_1285_D;
  }

  do {
ltmp_1285_D:
  ltmp_1255_7 = ltmp_1255_7_2E;
  *(&ltmp_1254_13[ltmp_1255_7]) = 0;
  ltmp_1256_7 = ltmp_1255_7 + 1u;
  if ((ltmp_1256_7 == ltmp_1253_7)) {
    goto ltmp_1284_D;
  } else {
    ltmp_1255_7_2E = ltmp_1256_7;
    goto ltmp_1285_D;
  }

  } while (1);
ltmp_1284_D:
  ltmp_1257_102C = &ltmp_1244_1023->field0;
  ltmp_1258_1029 = *ltmp_1257_102C;
  ltmp_1259_102A = *(&ltmp_1258_1029[18]);
  ltmp_1260_7 = ((plzo_uint32_t  (*) (struct l_struct_2E_E *))ltmp_1259_102A)(ltmp_1244_1023);
  ltmp_1261_13 = (unsigned char *)(&ltmp_1247_103B);
  ltmp_1262_7_2E = 0u;
  goto ltmp_1286_D;

  do {
ltmp_1286_D:
  ltmp_1262_7 = ltmp_1262_7_2E;
  *(&ltmp_1261_13[ltmp_1262_7]) = 0;
  ltmp_1263_7 = ltmp_1262_7 + 1u;
  if ((ltmp_1263_7 == 32u)) {
    goto ltmp_1287_D;
  } else {
    ltmp_1262_7_2E = ltmp_1263_7;
    goto ltmp_1286_D;
  }

  } while (1);
ltmp_1287_D:
  *(&ltmp_1247_103B.field3) = ltmp_1245_7;
  *(&ltmp_1247_103B.field4) = ltmp_1246_7;
  *(&ltmp_1247_103B.field5) = ltmp_1249_7;
  ltmp_1264_13 = (unsigned char *)(&ltmp_1248_103B);
  ltmp_1265_7_2E = 0u;
  goto ltmp_1288_D;

  do {
ltmp_1288_D:
  ltmp_1265_7 = ltmp_1265_7_2E;
  *(&ltmp_1264_13[ltmp_1265_7]) = 0;
  ltmp_1266_7 = ltmp_1265_7 + 1u;
  if ((ltmp_1266_7 == 32u)) {
    goto ltmp_1289_D;
  } else {
    ltmp_1265_7_2E = ltmp_1266_7;
    goto ltmp_1288_D;
  }

  } while (1);
ltmp_1289_D:
  *(&ltmp_1248_103B.field3) = ltmp_1245_7;
  *(&ltmp_1248_103B.field4) = ltmp_1246_7;
  *(&ltmp_1248_103B.field5) = ltmp_1249_7;
  if ((ltmp_1246_7 == ltmp_1245_7)) {
    goto ltmp_1290_D;
  } else {
    goto ltmp_1291_D;
  }

ltmp_1291_D:
  ltmp_1267_1037 = &ltmp_1244_1023->field19.field0;
  ltmp_1268_17 = &ltmp_1247_103B.field0;
  ltmp_1269_17 = &ltmp_1248_103B.field0;
  ltmp_1270_7_2E = 0u;
  goto ltmp_1292_D;

  do {
ltmp_1292_D:
  ltmp_1270_7 = ltmp_1270_7_2E;
  ltmp_1271_7 = (ltmp_1249_7 - ltmp_1270_7) + 4294967295u;
  ltmp_1272_1038 = *ltmp_1267_1037;
  ltmp_1273_7 = *(&ltmp_1272_1038[ltmp_1271_7].field0);
  ltmp_1274_1031 = *ltmp_1250_1030;
  ltmp_1275_7 = *(&ltmp_1274_1031[(((plzo_int32_t )ltmp_1271_7) + 1)].field0);
  ltmp_1276_7 = ltmp_1275_7 + ltmp_1260_7;
  *ltmp_1268_17 = ltmp_1276_7;
  ltmp_1277_1029 = *ltmp_1257_102C;
  ltmp_1278_102A = *(&ltmp_1277_1029[17]);
  ((void  (*) (struct l_struct_2E_E *, struct l_struct_2E_E_3A__3A_A4 *, plzo_uint32_t , plzo_uint32_t , plzo_uint32_t ))ltmp_1278_102A)(ltmp_1244_1023, (&ltmp_1248_103B), ltmp_1273_7, ltmp_1271_7, 0u);
  ltmp_1279_7 = *ltmp_1269_17;
  ltmp_1280_17 = &ltmp_1274_1031[ltmp_1271_7].field0;
  if ((ltmp_1276_7 > ltmp_1279_7)) {
    goto ltmp_1293_D;
  } else {
    goto ltmp_1294_D;
  }

ltmp_1295_D:
  ltmp_1281_7 = ltmp_1270_7 + 1u;
  ltmp_1270_7_2E = ltmp_1281_7;
  goto ltmp_1292_D;

ltmp_1294_D:
  *ltmp_1280_17 = ltmp_1276_7;
  *(&ltmp_1274_1031[ltmp_1271_7].field1) = 0u;
  *(&ltmp_1274_1031[ltmp_1271_7].field2) = 0u;
  *(&ltmp_1274_1031[ltmp_1271_7].field3) = 0u;
  if ((ltmp_1271_7 == 0u)) {
    goto ltmp_1290_D;
  } else {
    goto ltmp_1295_D;
  }

ltmp_1293_D:
  *ltmp_1280_17 = ltmp_1279_7;
  *(&ltmp_1274_1031[ltmp_1271_7].field1) = ltmp_1273_7;
  *(&ltmp_1274_1031[ltmp_1271_7].field2) = 0u;
  *(&ltmp_1274_1031[ltmp_1271_7].field3) = 0u;
  if ((ltmp_1271_7 == 0u)) {
    goto ltmp_1290_D;
  } else {
    goto ltmp_1295_D;
  }

  } while (1);
ltmp_1290_D:
  ltmp_1282_1031 = *ltmp_1250_1030;
  ltmp_1283_7 = *(&ltmp_1282_1031->field0);
  return ltmp_1283_7;
}


static void ltmp_15_1000(struct l_struct_2E_E *ltmp_1296_1023, plzo_uint32_t ltmp_1297_7, plzo_uint32_t ltmp_1298_7) {
  struct l_struct_2E_A7 *ltmp_1299_1038;
  plzo_uint32_t *ltmp_1300_17;
  plzo_uint32_t ltmp_1301_7;
  struct l_struct_2E_A7 *ltmp_1302_1038;
  plzo_uint32_t *ltmp_1303_17;
  plzo_uint32_t ltmp_1304_7;
  plzo_uint32_t ltmp_1305_7;
  plzo_uint32_t *ltmp_1306_17;
  plzo_uint32_t ltmp_1307_7;
  plzo_uint32_t ltmp_1308_7;
  plzo_uint32_t ltmp_1309_7;
  plzo_uint32_t ltmp_1310_7;
  plzo_uint32_t ltmp_1311_7;
  plzo_uint32_t ltmp_1311_7_2E;
  plzo_uint32_t ltmp_1312_7;
  plzo_uint32_t ltmp_1312_7_2E;
  struct l_struct_2E_A7 *ltmp_1313_1038;
  struct l_struct_2E_A7 *ltmp_1313_1038_2E;
  plzo_uint32_t ltmp_1314_7;
  plzo_uint32_t ltmp_1314_7_2E;
  plzo_uint32_t ltmp_1315_7;
  plzo_uint32_t ltmp_1315_7_2E;
  struct l_struct_2E_A7 *ltmp_1316_1038;
  struct l_struct_2E_A7 *ltmp_1316_1038_2E;
  struct l_struct_2E_A2_3A__3A_A3 **ltmp_1317_1030;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1318_1031;
  plzo_uint32_t *ltmp_1319_17;
  plzo_uint32_t ltmp_1320_7;
  plzo_uint32_t ltmp_1321_7;
  plzo_uint32_t ltmp_1322_7;
  plzo_uint32_t ltmp_1323_7;
  plzo_uint32_t ltmp_1324_7;
  plzo_uint32_t ltmp_1325_7;
  plzo_uint32_t ltmp_1326_7;
  struct l_struct_2E_A7 *ltmp_1327_1038;
  plzo_uint32_t ltmp_1328_7;
  plzo_uint32_t ltmp_1329_7;
  struct l_struct_2E_A7 *ltmp_1330_1038;
  plzo_uint32_t *ltmp_1331_17;
  plzo_uint32_t ltmp_1332_7;
  plzo_int32_t ltmp_1333_8;
  plzo_uint32_t ltmp_1334_7;
  plzo_uint32_t ltmp_1335_7;
  plzo_uint32_t ltmp_1336_7;
  plzo_uint32_t ltmp_1337_7;
  plzo_uint32_t ltmp_1338_7;
  plzo_uint32_t ltmp_1339_7;
  plzo_uint32_t ltmp_1340_7;
  plzo_uint32_t ltmp_1341_7;
  plzo_uint32_t ltmp_1342_7;
  plzo_uint32_t ltmp_1343_7;
  plzo_uint32_t ltmp_1344_7;
  plzo_uint32_t ltmp_1345_7;
  plzo_uint32_t ltmp_1346_7;
  plzo_uint32_t ltmp_1347_7;
  plzo_uint32_t ltmp_1348_7;
  plzo_uint32_t ltmp_1348_7_2E;
  plzo_uint32_t ltmp_1349_7;
  plzo_uint32_t ltmp_1350_7;
  plzo_uint32_t ltmp_1351_7;
  plzo_uint32_t ltmp_1352_7;
  plzo_uint32_t ltmp_1352_7_2E;
  unsigned char *ltmp_1353_13;
  plzo_int32_t ltmp_1354_8;
  plzo_uint32_t ltmp_1355_7;
  plzo_uint32_t ltmp_1355_7_2E;
  unsigned char ltmp_1356_3;
  unsigned char ltmp_1357_3;
  plzo_int32_t ltmp_1358_8;
  plzo_uint32_t ltmp_1359_7;
  plzo_int32_t ltmp_1360_8;
  plzo_int32_t ltmp_1360_8_2E;
  plzo_uint32_t ltmp_1361_7;
  plzo_uint32_t ltmp_1361_7_2E;
  plzo_uint32_t *ltmp_1362_17;
  plzo_uint32_t ltmp_1363_7;
  plzo_uint32_t ltmp_1364_7;
  plzo_int32_t  (***ltmp_1365_102C) ();
  plzo_int32_t  (**ltmp_1366_1029) ();
  plzo_int32_t  (*ltmp_1367_102A) ();
  unsigned char *ltmp_1368_13;
  plzo_int32_t  (**ltmp_1369_1029) ();
  plzo_int32_t  (*ltmp_1370_102A) ();
  plzo_uint32_t ltmp_1371_7;
  plzo_uint32_t ltmp_1372_7;
  plzo_uint32_t ltmp_1373_7;
  struct l_struct_2E_A7 *ltmp_1374_1038;
  plzo_uint32_t ltmp_1375_7;
  plzo_uint32_t ltmp_1376_7;
  plzo_uint32_t ltmp_1377_7;
  plzo_uint32_t ltmp_1378_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1379_1031;
  plzo_uint32_t ltmp_1380_7;
  plzo_uint32_t ltmp_1381_7;
  plzo_uint32_t ltmp_1382_7;
  struct l_struct_2E_A7 *ltmp_1383_1038;

  ltmp_1299_1038 = *(&ltmp_1296_1023->field19.field0);
  ltmp_1300_17 = &ltmp_1296_1023->field14;
  ltmp_1301_7 = *ltmp_1300_17;
  ltmp_1302_1038 = &ltmp_1299_1038[ltmp_1301_7];
  ltmp_1303_17 = &ltmp_1296_1023->field13;
  ltmp_1304_7 = *ltmp_1303_17;
  ltmp_1305_7 = ltmp_1301_7 + ltmp_1304_7;
  *ltmp_1303_17 = ltmp_1305_7;
  ltmp_1306_17 = &ltmp_1296_1023->field21;
  ltmp_1307_7 = *ltmp_1306_17;
  if ((ltmp_1305_7 < ltmp_1307_7)) {
    goto ltmp_1384_D;
  } else {
    goto ltmp_1385_D;
  }

ltmp_1384_D:
  ltmp_1308_7 = ltmp_1301_7 + ltmp_1297_7;
  if ((ltmp_1308_7 < ltmp_1298_7)) {
    ltmp_1314_7_2E = ltmp_1305_7;
    ltmp_1315_7_2E = ltmp_1308_7;
    ltmp_1316_1038_2E = ltmp_1302_1038;
    goto ltmp_1386_D;
  } else {
    goto ltmp_1387_D;
  }

ltmp_1385_D:
  ltmp_1309_7 = ltmp_1307_7 + 4294967295u;
  ltmp_1310_7 = ltmp_1301_7 + ltmp_1297_7;
  ltmp_1311_7_2E = ltmp_1309_7;
  ltmp_1312_7_2E = ltmp_1310_7;
  ltmp_1313_1038_2E = ltmp_1302_1038;
  goto ltmp_1388_D;

ltmp_1388_D:
  ltmp_1311_7 = ltmp_1311_7_2E;
  ltmp_1312_7 = ltmp_1312_7_2E;
  ltmp_1313_1038 = ltmp_1313_1038_2E;
  if ((ltmp_1312_7 < ltmp_1298_7)) {
    ltmp_1314_7_2E = ltmp_1311_7;
    ltmp_1315_7_2E = ltmp_1312_7;
    ltmp_1316_1038_2E = ltmp_1313_1038;
    goto ltmp_1386_D;
  } else {
    goto ltmp_1387_D;
  }

ltmp_1386_D:
  ltmp_1314_7 = ltmp_1314_7_2E;
  ltmp_1315_7 = ltmp_1315_7_2E;
  ltmp_1316_1038 = ltmp_1316_1038_2E;
  ltmp_1317_1030 = &ltmp_1296_1023->field20.field3;
  ltmp_1318_1031 = *ltmp_1317_1030;
  ltmp_1319_17 = &ltmp_1296_1023->field20.field1;
  ltmp_1320_7 = *ltmp_1319_17;
  ltmp_1321_7 = (ltmp_1320_7*(ltmp_1315_7 - ltmp_1297_7)) + ltmp_1314_7;
  ltmp_1322_7 = *(&ltmp_1318_1031[ltmp_1321_7].field1);
  if ((ltmp_1322_7 == 0u)) {
    goto ltmp_1389_D;
  } else {
    goto ltmp_1390_D;
  }

ltmp_1389_D:
  ltmp_1323_7 = *ltmp_1303_17;
  ltmp_1324_7 = ltmp_1323_7 + 1u;
  *ltmp_1303_17 = ltmp_1324_7;
  ltmp_1325_7 = *ltmp_1306_17;
  if ((ltmp_1324_7 < ltmp_1325_7)) {
    goto ltmp_1391_D;
  } else {
    goto ltmp_1392_D;
  }

ltmp_1391_D:
  ltmp_1326_7 = ltmp_1315_7 + 1u;
  ltmp_1327_1038 = &ltmp_1316_1038[1];
  ltmp_1311_7_2E = ltmp_1324_7;
  ltmp_1312_7_2E = ltmp_1326_7;
  ltmp_1313_1038_2E = ltmp_1327_1038;
  goto ltmp_1388_D;

ltmp_1392_D:
  ltmp_1328_7 = ltmp_1325_7 + 4294967295u;
  ltmp_1329_7 = ltmp_1315_7 + 1u;
  ltmp_1330_1038 = &ltmp_1316_1038[1];
  ltmp_1311_7_2E = ltmp_1328_7;
  ltmp_1312_7_2E = ltmp_1329_7;
  ltmp_1313_1038_2E = ltmp_1330_1038;
  goto ltmp_1388_D;

ltmp_1390_D:
  ltmp_1331_17 = &ltmp_1318_1031[ltmp_1321_7].field2;
  ltmp_1332_7 = *ltmp_1331_17;
  if ((ltmp_1332_7 == 0u)) {
    goto ltmp_1393_D;
  } else {
    ltmp_1361_7_2E = ltmp_1332_7;
    goto ltmp_1394_D;
  }

ltmp_1393_D:
  ltmp_1333_8 = *(&ltmp_1296_1023->field6.field1);
  if (((((plzo_uint32_t )ltmp_1333_8) & 1u) == 0u)) {
    goto ltmp_1395_D;
  } else {
    goto ltmp_1396_D;
  }

ltmp_1395_D:
  ltmp_1334_7 = *(&ltmp_1316_1038->field0);
  ltmp_1335_7 = ltmp_1334_7 - ltmp_1322_7;
  ltmp_1336_7 = *(&ltmp_1316_1038->field1[((((ltmp_1335_7 < 8u)) ? (ltmp_1335_7) : (7u)))]);
  ltmp_1361_7_2E = ltmp_1336_7;
  goto ltmp_1394_D;

ltmp_1396_D:
  ltmp_1337_7 = *(&ltmp_1296_1023->field5);
  if ((ltmp_1337_7 < ltmp_1322_7)) {
    goto ltmp_1397_D;
  } else {
    goto ltmp_1398_D;
  }

ltmp_1398_D:
  ltmp_1338_7 = *(&ltmp_1296_1023->field16);
  ltmp_1339_7 = *(&ltmp_1296_1023->field4[ltmp_1322_7]);
  if ((ltmp_1338_7 < ltmp_1339_7)) {
    goto ltmp_1397_D;
  } else {
    goto ltmp_1399_D;
  }

ltmp_1399_D:
  ltmp_1340_7 = *(&ltmp_1316_1038->field0);
  ltmp_1341_7 = ltmp_1340_7 - ltmp_1322_7;
  ltmp_1342_7 = *(&ltmp_1316_1038->field1[((((ltmp_1341_7 < 8u)) ? (ltmp_1341_7) : (7u)))]);
  ltmp_1361_7_2E = ltmp_1342_7;
  goto ltmp_1394_D;

ltmp_1397_D:
  ltmp_1343_7 = *(&ltmp_1296_1023->field16);
  ltmp_1344_7 = *(&ltmp_1316_1038->field0);
  ltmp_1345_7 = ltmp_1344_7 - ltmp_1322_7;
  ltmp_1346_7 = (((ltmp_1345_7 < 8u)) ? (ltmp_1345_7) : (7u));
  ltmp_1347_7 = *(&ltmp_1316_1038->field1[ltmp_1346_7]);
  if (((ltmp_1346_7 == 0u) | (ltmp_1347_7 >= ltmp_1343_7))) {
    ltmp_1352_7_2E = ltmp_1347_7;
    goto ltmp_1400_D;
  } else {
    ltmp_1348_7_2E = 0u;
    goto ltmp_1401_D;
  }

  do {
ltmp_1401_D:
  ltmp_1348_7 = ltmp_1348_7_2E;
  ltmp_1349_7 = ltmp_1346_7 - ltmp_1348_7;
  ltmp_1350_7 = *(&ltmp_1316_1038->field1[(ltmp_1349_7 + 4294967295u)]);
  if ((ltmp_1350_7 == ltmp_1343_7)) {
    ltmp_1352_7_2E = ltmp_1343_7;
    goto ltmp_1400_D;
  } else {
    goto ltmp_1402_D;
  }

ltmp_1402_D:
  ltmp_1351_7 = ltmp_1348_7 + 1u;
  if ((ltmp_1349_7 == 1u)) {
    ltmp_1352_7_2E = ltmp_1347_7;
    goto ltmp_1400_D;
  } else {
    ltmp_1348_7_2E = ltmp_1351_7;
    goto ltmp_1401_D;
  }

  } while (1);
ltmp_1400_D:
  ltmp_1352_7 = ltmp_1352_7_2E;
  if ((((ltmp_1343_7 == ltmp_1352_7) | (ltmp_1322_7 < 2u)) | (ltmp_1343_7 == 4294967295u))) {
    ltmp_1361_7_2E = ltmp_1352_7;
    goto ltmp_1394_D;
  } else {
    goto ltmp_1403_D;
  }

ltmp_1403_D:
  ltmp_1353_13 = *(&ltmp_1296_1023->field8);
  ltmp_1354_8 = ((plzo_int32_t )(-(ltmp_1343_7))) + ((plzo_int32_t )ltmp_1315_7);
  if ((ltmp_1322_7 == 0u)) {
    ltmp_1360_8_2E = 0;
    goto ltmp_1404_D;
  } else {
    ltmp_1355_7_2E = 0u;
    goto ltmp_1405_D;
  }

  do {
ltmp_1405_D:
  ltmp_1355_7 = ltmp_1355_7_2E;
  ltmp_1356_3 = *(&ltmp_1353_13[(ltmp_1315_7 + ltmp_1355_7)]);
  ltmp_1357_3 = *(&ltmp_1353_13[(ltmp_1354_8 + ((plzo_int32_t )ltmp_1355_7))]);
  ltmp_1358_8 = ((plzo_int32_t )ltmp_1356_3) - ((plzo_int32_t )ltmp_1357_3);
  if ((ltmp_1356_3 == ltmp_1357_3)) {
    goto ltmp_1406_D;
  } else {
    ltmp_1360_8_2E = ltmp_1358_8;
    goto ltmp_1404_D;
  }

ltmp_1406_D:
  ltmp_1359_7 = ltmp_1355_7 + 1u;
  if (((ltmp_1322_7 - ltmp_1355_7) == 1u)) {
    ltmp_1360_8_2E = 0;
    goto ltmp_1404_D;
  } else {
    ltmp_1355_7_2E = ltmp_1359_7;
    goto ltmp_1405_D;
  }

  } while (1);
ltmp_1404_D:
  ltmp_1360_8 = ltmp_1360_8_2E;
  if ((ltmp_1360_8 == 0)) {
    goto ltmp_1407_D;
  } else {
    ltmp_1361_7_2E = ltmp_1352_7;
    goto ltmp_1394_D;
  }

ltmp_1407_D:
  ltmp_1361_7_2E = ltmp_1343_7;
  goto ltmp_1394_D;

ltmp_1394_D:
  ltmp_1361_7 = ltmp_1361_7_2E;
  ltmp_1362_17 = &ltmp_1296_1023->field12;
  ltmp_1363_7 = *ltmp_1303_17;
  ltmp_1364_7 = ltmp_1315_7 - ltmp_1363_7;
  *ltmp_1362_17 = ltmp_1364_7;
  ltmp_1365_102C = &ltmp_1296_1023->field0;
  ltmp_1366_1029 = *ltmp_1365_102C;
  ltmp_1367_102A = *(&ltmp_1366_1029[3]);
  ltmp_1368_13 = *(&ltmp_1296_1023->field8);
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint32_t , plzo_uint32_t ))ltmp_1367_102A)(ltmp_1296_1023, (&ltmp_1368_13[ltmp_1364_7]), ltmp_1363_7, ltmp_1322_7);
  *ltmp_1303_17 = 0u;
  *ltmp_1362_17 = ltmp_1315_7;
  ltmp_1369_1029 = *ltmp_1365_102C;
  ltmp_1370_102A = *(&ltmp_1369_1029[4]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_1370_102A)(ltmp_1296_1023, ltmp_1322_7, ltmp_1361_7);
  ltmp_1371_7 = *(&ltmp_1318_1031[ltmp_1321_7].field3);
  if ((ltmp_1371_7 == 0u)) {
    goto ltmp_1408_D;
  } else {
    goto ltmp_1409_D;
  }

ltmp_1409_D:
  ltmp_1372_7 = ltmp_1371_7 + (ltmp_1297_7 - ltmp_1315_7);
  ltmp_1373_7 = ltmp_1372_7 + ltmp_1315_7;
  ltmp_1374_1038 = &ltmp_1316_1038[ltmp_1372_7];
  ltmp_1375_7 = *ltmp_1303_17;
  ltmp_1376_7 = (ltmp_1375_7 - ltmp_1322_7) + ltmp_1372_7;
  *ltmp_1303_17 = ltmp_1376_7;
  ltmp_1377_7 = *ltmp_1306_17;
  ltmp_1378_7 = (((ltmp_1376_7 < ltmp_1377_7)) ? (ltmp_1376_7) : ((ltmp_1377_7 + 4294967295u)));
  ltmp_1379_1031 = *ltmp_1317_1030;
  ltmp_1380_7 = *ltmp_1319_17;
  ltmp_1381_7 = *ltmp_1331_17;
  *(&ltmp_1379_1031[((ltmp_1380_7*(ltmp_1373_7 - ltmp_1297_7)) + ltmp_1378_7)].field2) = ltmp_1381_7;
  ltmp_1311_7_2E = ltmp_1378_7;
  ltmp_1312_7_2E = ltmp_1373_7;
  ltmp_1313_1038_2E = ltmp_1374_1038;
  goto ltmp_1388_D;

ltmp_1408_D:
  ltmp_1382_7 = ltmp_1322_7 + ltmp_1315_7;
  ltmp_1383_1038 = &ltmp_1316_1038[ltmp_1322_7];
  ltmp_1311_7_2E = 0u;
  ltmp_1312_7_2E = ltmp_1382_7;
  ltmp_1313_1038_2E = ltmp_1383_1038;
  goto ltmp_1388_D;

ltmp_1387_D:
  *ltmp_1300_17 = 0u;
  return;
}


static void ltmp_16_1009(struct l_struct_2E_E *ltmp_1410_1023, plzo_uint32_t ltmp_1411_7, plzo_uint32_t ltmp_1412_7, plzo_int32_t ltmp_1413_8) {
  struct l_struct_2E_A7 *ltmp_1414_1038;
  plzo_uint32_t *ltmp_1415_17;
  plzo_uint32_t ltmp_1416_7;
  struct l_struct_2E_A7 *ltmp_1417_1038;
  plzo_uint32_t *ltmp_1418_17;
  plzo_uint32_t ltmp_1419_7;
  plzo_uint32_t ltmp_1420_7;
  plzo_uint32_t *ltmp_1421_17;
  plzo_uint32_t *ltmp_1422_17;
  plzo_uint32_t *ltmp_1423_17;
  plzo_int32_t  (***ltmp_1424_102C) ();
  unsigned char **ltmp_1425_1025;
  plzo_uint32_t ltmp_1426_7;
  plzo_uint32_t ltmp_1426_7_2E;
  struct l_struct_2E_A7 *ltmp_1427_1038;
  struct l_struct_2E_A7 *ltmp_1427_1038_2E;
  plzo_uint32_t ltmp_1428_7;
  plzo_uint32_t ltmp_1429_7;
  plzo_uint32_t ltmp_1430_7;
  plzo_uint32_t ltmp_1431_7;
  plzo_uint32_t ltmp_1432_7;
  plzo_uint32_t ltmp_1433_7;
  plzo_uint32_t ltmp_1433_7_2E;
  plzo_uint32_t ltmp_1434_7;
  plzo_uint32_t ltmp_1435_7;
  plzo_uint32_t ltmp_1435_7_2E;
  plzo_uint32_t ltmp_1436_7;
  plzo_uint32_t ltmp_1437_7;
  plzo_uint32_t ltmp_1438_7;
  struct l_struct_2E_A7 *ltmp_1439_1038;
  plzo_uint32_t ltmp_1440_7;
  plzo_uint32_t ltmp_1441_7;
  plzo_int32_t  (**ltmp_1442_1029) ();
  plzo_int32_t  (*ltmp_1443_102A) ();
  unsigned char *ltmp_1444_13;
  plzo_int32_t  (**ltmp_1445_1029) ();
  plzo_int32_t  (*ltmp_1446_102A) ();
  plzo_uint32_t ltmp_1447_7;
  struct l_struct_2E_A7 *ltmp_1448_1038;

  ltmp_1414_1038 = *(&ltmp_1410_1023->field19.field0);
  ltmp_1415_17 = &ltmp_1410_1023->field14;
  ltmp_1416_7 = *ltmp_1415_17;
  ltmp_1417_1038 = &ltmp_1414_1038[ltmp_1416_7];
  ltmp_1418_17 = &ltmp_1410_1023->field13;
  ltmp_1419_7 = *ltmp_1418_17;
  *ltmp_1418_17 = (ltmp_1416_7 + ltmp_1419_7);
  ltmp_1420_7 = ltmp_1416_7 + ltmp_1411_7;
  if ((ltmp_1420_7 < ltmp_1412_7)) {
    goto ltmp_1449_D;
  } else {
    goto ltmp_1450_D;
  }

ltmp_1449_D:
  ltmp_1421_17 = &ltmp_1410_1023->field7.field3;
  ltmp_1422_17 = &ltmp_1410_1023->field5;
  ltmp_1423_17 = &ltmp_1410_1023->field12;
  ltmp_1424_102C = &ltmp_1410_1023->field0;
  ltmp_1425_1025 = &ltmp_1410_1023->field8;
  ltmp_1426_7_2E = ltmp_1420_7;
  ltmp_1427_1038_2E = ltmp_1417_1038;
  goto ltmp_1451_D;

  do {
ltmp_1451_D:
  ltmp_1426_7 = ltmp_1426_7_2E;
  ltmp_1427_1038 = ltmp_1427_1038_2E;
  ltmp_1428_7 = *(&ltmp_1427_1038->field0);
  ltmp_1429_7 = *ltmp_1421_17;
  if ((ltmp_1429_7 > ltmp_1428_7)) {
    goto ltmp_1452_D;
  } else {
    goto ltmp_1453_D;
  }

ltmp_1454_D:
  ltmp_1437_7 = *ltmp_1418_17;
  *ltmp_1418_17 = (ltmp_1437_7 + 1u);
  ltmp_1438_7 = ltmp_1426_7 + 1u;
  ltmp_1439_1038 = &ltmp_1427_1038[1];
  if ((ltmp_1438_7 < ltmp_1412_7)) {
    ltmp_1426_7_2E = ltmp_1438_7;
    ltmp_1427_1038_2E = ltmp_1439_1038;
    goto ltmp_1451_D;
  } else {
    goto ltmp_1450_D;
  }

ltmp_1455_D:
  ltmp_1433_7 = ltmp_1433_7_2E;
  ltmp_1434_7 = *(&ltmp_1410_1023->field4[ltmp_1428_7]);
  if ((ltmp_1434_7 > ltmp_1433_7)) {
    ltmp_1435_7_2E = ltmp_1433_7;
    goto ltmp_1456_D;
  } else {
    goto ltmp_1454_D;
  }

ltmp_1453_D:
  ltmp_1430_7 = *(&ltmp_1427_1038->field1[0]);
  ltmp_1431_7 = *ltmp_1422_17;
  if ((ltmp_1431_7 < ltmp_1428_7)) {
    ltmp_1435_7_2E = ltmp_1430_7;
    goto ltmp_1456_D;
  } else {
    ltmp_1433_7_2E = ltmp_1430_7;
    goto ltmp_1455_D;
  }

ltmp_1452_D:
  ltmp_1432_7 = *ltmp_1422_17;
  if ((ltmp_1432_7 < ltmp_1428_7)) {
    ltmp_1435_7_2E = 0u;
    goto ltmp_1456_D;
  } else {
    ltmp_1433_7_2E = 0u;
    goto ltmp_1455_D;
  }

ltmp_1457_D:
  ltmp_1436_7 = *(&ltmp_1427_1038[1].field0);
  if ((ltmp_1436_7 > ltmp_1428_7)) {
    goto ltmp_1454_D;
  } else {
    goto ltmp_1458_D;
  }

ltmp_1456_D:
  ltmp_1435_7 = ltmp_1435_7_2E;
  if ((ltmp_1413_8 == 0)) {
    goto ltmp_1458_D;
  } else {
    goto ltmp_1457_D;
  }

ltmp_1458_D:
  ltmp_1440_7 = *ltmp_1418_17;
  ltmp_1441_7 = ltmp_1426_7 - ltmp_1440_7;
  *ltmp_1423_17 = ltmp_1441_7;
  ltmp_1442_1029 = *ltmp_1424_102C;
  ltmp_1443_102A = *(&ltmp_1442_1029[3]);
  ltmp_1444_13 = *ltmp_1425_1025;
  ((void  (*) (struct l_struct_2E_E *, unsigned char *, plzo_uint32_t , plzo_uint32_t ))ltmp_1443_102A)(ltmp_1410_1023, (&ltmp_1444_13[ltmp_1441_7]), ltmp_1440_7, ltmp_1428_7);
  *ltmp_1418_17 = 0u;
  *ltmp_1423_17 = ltmp_1426_7;
  ltmp_1445_1029 = *ltmp_1424_102C;
  ltmp_1446_102A = *(&ltmp_1445_1029[4]);
  ((void  (*) (struct l_struct_2E_E *, plzo_uint32_t , plzo_uint32_t ))ltmp_1446_102A)(ltmp_1410_1023, ltmp_1428_7, ltmp_1435_7);
  ltmp_1447_7 = ltmp_1428_7 + ltmp_1426_7;
  ltmp_1448_1038 = &ltmp_1427_1038[ltmp_1428_7];
  if ((ltmp_1447_7 < ltmp_1412_7)) {
    ltmp_1426_7_2E = ltmp_1447_7;
    ltmp_1427_1038_2E = ltmp_1448_1038;
    goto ltmp_1451_D;
  } else {
    goto ltmp_1450_D;
  }

  } while (1);
ltmp_1450_D:
  *ltmp_1415_17 = 0u;
  return;
}


static plzo_uint32_t ltmp_17_1008(struct l_struct_2E_E *ltmp_1459_1023, plzo_uint32_t ltmp_1460_7, plzo_uint32_t ltmp_1461_7) {
  struct l_struct_2E_A7 *ltmp_1462_1038;
  plzo_uint32_t ltmp_1463_7;
  plzo_uint32_t ltmp_1463_7_2E;
  plzo_uint32_t ltmp_1464_7;
  plzo_uint32_t ltmp_1464_7_2E;
  plzo_uint32_t ltmp_1465_7;
  plzo_uint32_t ltmp_1465_7_2E;
  struct l_struct_2E_A7 *ltmp_1466_1038;
  struct l_struct_2E_A7 *ltmp_1466_1038_2E;
  plzo_uint32_t ltmp_1467_7;
  plzo_int32_t  (***ltmp_1468_102C) ();
  plzo_uint32_t ltmp_1469_7;
  plzo_uint32_t ltmp_1469_7_2E;
  plzo_uint32_t ltmp_1470_7;
  plzo_uint32_t ltmp_1470_7_2E;
  plzo_uint32_t ltmp_1471_7;
  plzo_uint32_t ltmp_1471_7_2E;
  plzo_uint32_t ltmp_1472_7;
  plzo_uint32_t ltmp_1472_7_2E;
  struct l_struct_2E_A7 *ltmp_1473_1038;
  struct l_struct_2E_A7 *ltmp_1473_1038_2E;
  plzo_uint32_t ltmp_1474_7;
  plzo_uint32_t ltmp_1475_7;
  plzo_uint32_t ltmp_1476_7;
  plzo_int32_t  (**ltmp_1477_1029) ();
  plzo_int32_t  (*ltmp_1478_102A) ();
  plzo_int32_t ltmp_1479_8;
  plzo_int32_t  (**ltmp_1480_1029) ();
  plzo_int32_t  (*ltmp_1481_102A) ();
  plzo_int32_t ltmp_1482_8;
  plzo_uint32_t ltmp_1483_7;
  struct l_struct_2E_A7 *ltmp_1484_1038;
  plzo_uint32_t ltmp_1485_7;
  plzo_uint32_t ltmp_1485_7_2E;
  plzo_uint32_t ltmp_1486_7;
  plzo_uint32_t ltmp_1487_7;
  plzo_uint32_t ltmp_1488_7;
  plzo_uint32_t ltmp_1488_7_2E;
  plzo_uint32_t ltmp_1489_7;
  plzo_uint32_t ltmp_1489_7_2E;
  struct l_struct_2E_A7 *ltmp_1490_1038;
  plzo_uint32_t ltmp_1491_7;
  plzo_uint32_t ltmp_1491_7_2E;
  plzo_uint32_t ltmp_1492_7;
  plzo_uint32_t ltmp_1492_7_2E;

  ltmp_1462_1038 = *(&ltmp_1459_1023->field19.field0);
  if ((ltmp_1460_7 < ltmp_1461_7)) {
    goto ltmp_1493_D;
  } else {
    ltmp_1491_7_2E = ltmp_1460_7;
    ltmp_1492_7_2E = ltmp_1460_7;
    goto ltmp_1494_D;
  }

ltmp_1493_D:
  ltmp_1468_102C = &ltmp_1459_1023->field0;
  ltmp_1469_7_2E = 0u;
  ltmp_1470_7_2E = ltmp_1460_7;
  ltmp_1471_7_2E = ltmp_1460_7;
  ltmp_1472_7_2E = ltmp_1460_7;
  ltmp_1473_1038_2E = ltmp_1462_1038;
  goto ltmp_1495_D;

  do {
ltmp_1495_D:
  ltmp_1469_7 = ltmp_1469_7_2E;
  ltmp_1470_7 = ltmp_1470_7_2E;
  ltmp_1471_7 = ltmp_1471_7_2E;
  ltmp_1472_7 = ltmp_1472_7_2E;
  ltmp_1473_1038 = ltmp_1473_1038_2E;
  ltmp_1474_7 = ltmp_1469_7 + ltmp_1460_7;
  ltmp_1475_7 = *(&ltmp_1473_1038->field0);
  ltmp_1476_7 = (((ltmp_1475_7 == 0u)) ? (1u) : (ltmp_1475_7));
  if ((ltmp_1474_7 == ltmp_1470_7)) {
    goto ltmp_1496_D;
  } else {
    goto ltmp_1497_D;
  }

ltmp_1498_D:
  ltmp_1463_7 = ltmp_1463_7_2E;
  ltmp_1464_7 = ltmp_1464_7_2E;
  ltmp_1465_7 = ltmp_1465_7_2E;
  ltmp_1466_1038 = ltmp_1466_1038_2E;
  ltmp_1467_7 = ltmp_1469_7 + 1u;
  if (((ltmp_1474_7 + 1u) < ltmp_1461_7)) {
    ltmp_1469_7_2E = ltmp_1467_7;
    ltmp_1470_7_2E = ltmp_1463_7;
    ltmp_1471_7_2E = ltmp_1464_7;
    ltmp_1472_7_2E = ltmp_1465_7;
    ltmp_1473_1038_2E = ltmp_1466_1038;
    goto ltmp_1495_D;
  } else {
    ltmp_1491_7_2E = ltmp_1464_7;
    ltmp_1492_7_2E = ltmp_1465_7;
    goto ltmp_1494_D;
  }

ltmp_1499_D:
  ltmp_1483_7 = ltmp_1476_7 + ltmp_1474_7;
  ltmp_1484_1038 = &ltmp_1473_1038[1];
  ltmp_1463_7_2E = ltmp_1483_7;
  ltmp_1464_7_2E = ltmp_1474_7;
  ltmp_1465_7_2E = ltmp_1472_7;
  ltmp_1466_1038_2E = ltmp_1484_1038;
  goto ltmp_1498_D;

ltmp_1500_D:
  ltmp_1480_1029 = *ltmp_1468_102C;
  ltmp_1481_102A = *(&ltmp_1480_1029[21]);
  ltmp_1482_8 = ((plzo_int32_t  (*) (struct l_struct_2E_E *, struct l_struct_2E_A7 *))ltmp_1481_102A)(ltmp_1459_1023, ltmp_1473_1038);
  if ((ltmp_1482_8 == 0)) {
    ltmp_1485_7_2E = ltmp_1472_7;
    goto ltmp_1501_D;
  } else {
    goto ltmp_1499_D;
  }

ltmp_1496_D:
  ltmp_1477_1029 = *ltmp_1468_102C;
  ltmp_1478_102A = *(&ltmp_1477_1029[20]);
  ltmp_1479_8 = ((plzo_int32_t  (*) (struct l_struct_2E_E *, struct l_struct_2E_A7 *))ltmp_1478_102A)(ltmp_1459_1023, ltmp_1473_1038);
  if ((ltmp_1479_8 == 0)) {
    goto ltmp_1500_D;
  } else {
    ltmp_1485_7_2E = ltmp_1474_7;
    goto ltmp_1501_D;
  }

ltmp_1502_D:
  ltmp_1488_7 = ltmp_1488_7_2E;
  ltmp_1489_7 = ltmp_1489_7_2E;
  ltmp_1490_1038 = &ltmp_1473_1038[1];
  ltmp_1463_7_2E = ltmp_1488_7;
  ltmp_1464_7_2E = ltmp_1471_7;
  ltmp_1465_7_2E = ltmp_1489_7;
  ltmp_1466_1038_2E = ltmp_1490_1038;
  goto ltmp_1498_D;

ltmp_1501_D:
  ltmp_1485_7 = ltmp_1485_7_2E;
  ltmp_1486_7 = ltmp_1476_7 + ltmp_1474_7;
  ltmp_1488_7_2E = ltmp_1486_7;
  ltmp_1489_7_2E = ltmp_1485_7;
  goto ltmp_1502_D;

ltmp_1497_D:
  ltmp_1487_7 = ltmp_1476_7 + ltmp_1474_7;
  if ((ltmp_1487_7 > ltmp_1470_7)) {
    goto ltmp_1503_D;
  } else {
    ltmp_1488_7_2E = ltmp_1470_7;
    ltmp_1489_7_2E = ltmp_1472_7;
    goto ltmp_1502_D;
  }

ltmp_1503_D:
  ltmp_1488_7_2E = ltmp_1487_7;
  ltmp_1489_7_2E = ltmp_1472_7;
  goto ltmp_1502_D;

  } while (1);
ltmp_1494_D:
  ltmp_1491_7 = ltmp_1491_7_2E;
  ltmp_1492_7 = ltmp_1492_7_2E;
  if ((ltmp_1492_7 == ltmp_1460_7)) {
    goto ltmp_1504_D;
  } else {
    goto ltmp_1505_D;
  }

ltmp_1504_D:
  return ((((ltmp_1491_7 == ltmp_1460_7)) ? (ltmp_1461_7) : (ltmp_1491_7)));
ltmp_1505_D:
  return ltmp_1492_7;
}


static struct l_struct_2E_M *ltmp_18_100A(struct l_struct_2E_A6 *ltmp_1506_1036) {
  plzo_uint32_t *ltmp_1507_17;
  plzo_uint32_t ltmp_1508_7;
  plzo_uint32_t ltmp_1509_7;
  plzo_uint32_t ltmp_1510_7;
  plzo_uint32_t *ltmp_1511_17;
  plzo_uint32_t ltmp_1512_7;
  plzo_uint32_t ltmp_1513_7;
  plzo_uint32_t ltmp_1514_7;
  plzo_uint32_t ltmp_1515_7;
  plzo_uint32_t *ltmp_1516_17;
  plzo_uint32_t ltmp_1517_7;
  struct l_struct_2E_M *ltmp_1518_1032;
  plzo_uint32_t ltmp_1519_7;
  plzo_uint32_t ltmp_1520_7;
  struct l_struct_2E_M *ltmp_1521_1032;
  plzo_uint32_t ltmp_1522_7;
  struct l_struct_2E_M *ltmp_1523_1032;
  plzo_uint32_t ltmp_1524_7;

  ltmp_1507_17 = &ltmp_1506_1036->field20;
  ltmp_1508_7 = *ltmp_1507_17;
  ltmp_1509_7 = ltmp_1508_7 + 1u;
  *ltmp_1507_17 = ltmp_1509_7;
  ltmp_1510_7 = *(&ltmp_1506_1036->field19);
  ltmp_1511_17 = &ltmp_1506_1036->field21;
  if ((ltmp_1509_7 == ltmp_1510_7)) {
    goto ltmp_1525_D;
  } else {
    goto ltmp_1526_D;
  }

ltmp_1525_D:
  *ltmp_1507_17 = 0u;
  ltmp_1512_7 = *ltmp_1511_17;
  if ((ltmp_1512_7 == 0u)) {
    goto ltmp_1527_D;
  } else {
    goto ltmp_1528_D;
  }

ltmp_1526_D:
  ltmp_1513_7 = *ltmp_1511_17;
  if ((ltmp_1509_7 == ltmp_1513_7)) {
    goto ltmp_1527_D;
  } else {
    goto ltmp_1528_D;
  }

ltmp_1527_D:
  ltmp_1514_7 = *ltmp_1511_17;
  ltmp_1515_7 = ltmp_1514_7 + 1u;
  *ltmp_1511_17 = ltmp_1515_7;
  ltmp_1516_17 = &ltmp_1506_1036->field1;
  if ((ltmp_1515_7 == ltmp_1510_7)) {
    goto ltmp_1529_D;
  } else {
    goto ltmp_1530_D;
  }

ltmp_1529_D:
  *ltmp_1511_17 = 0u;
  ltmp_1517_7 = *ltmp_1516_17;
  *ltmp_1516_17 = (ltmp_1517_7 + 1u);
  ltmp_1518_1032 = *(&ltmp_1506_1036->field18);
  ltmp_1519_7 = *ltmp_1507_17;
  return (&ltmp_1518_1032[ltmp_1519_7]);
ltmp_1530_D:
  ltmp_1520_7 = *ltmp_1516_17;
  *ltmp_1516_17 = (ltmp_1520_7 + 1u);
  ltmp_1521_1032 = *(&ltmp_1506_1036->field18);
  ltmp_1522_7 = *ltmp_1507_17;
  return (&ltmp_1521_1032[ltmp_1522_7]);
ltmp_1528_D:
  ltmp_1523_1032 = *(&ltmp_1506_1036->field18);
  ltmp_1524_7 = *ltmp_1507_17;
  return (&ltmp_1523_1032[ltmp_1524_7]);
}


static void ltmp_19_100B(struct l_struct_2E_A8 *ltmp_1531_1034) {
  struct l_struct_2E_A6 *ltmp_1532_1036;
  struct l_struct_2E_A9 **ltmp_1533_1035;
  struct l_struct_2E_A9 *ltmp_1534_102F;
  void  (*ltmp_1535_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_M *ltmp_1536_1032;
  struct l_struct_2E_A9 *ltmp_1537_102F;
  void  (*ltmp_1538_1019) (struct plzo_callback_t *, signed char *);
  struct l_struct_2E_A7 *ltmp_1539_1038;
  struct l_struct_2E_A9 *ltmp_1540_102F;
  void  (*ltmp_1541_1019) (struct plzo_callback_t *, signed char *);
  unsigned char *ltmp_1542_13;
  plzo_uint32_t ltmp_1543_7;
  plzo_uint32_t ltmp_1543_7_2E;
  plzo_uint32_t ltmp_1544_7;
  unsigned char *ltmp_1545_13;
  plzo_uint32_t ltmp_1546_7;
  plzo_uint32_t ltmp_1546_7_2E;
  plzo_uint32_t ltmp_1547_7;

  if ((ltmp_1531_1034 == ((struct l_struct_2E_A8 *)0))) {
    goto ltmp_1548_D;
  } else {
    goto ltmp_1549_D;
  }

ltmp_1549_D:
  ltmp_1532_1036 = *(&ltmp_1531_1034->field7);
  if ((ltmp_1532_1036 == ((struct l_struct_2E_A6 *)0))) {
    goto ltmp_1550_D;
  } else {
    goto ltmp_1551_D;
  }

ltmp_1551_D:
  ltmp_1533_1035 = &ltmp_1531_1034->field6;
  ltmp_1534_102F = *ltmp_1533_1035;
  ltmp_1535_1019 = *(&ltmp_1534_102F->field2.field1);
  ltmp_1536_1032 = *(&ltmp_1532_1036->field18);
  ltmp_1535_1019((&ltmp_1534_102F->field2), ((signed char *)ltmp_1536_1032));
  ltmp_1537_102F = *ltmp_1533_1035;
  ltmp_1538_1019 = *(&ltmp_1537_102F->field2.field1);
  ltmp_1539_1038 = *(&ltmp_1532_1036->field17);
  ltmp_1538_1019((&ltmp_1537_102F->field2), ((signed char *)ltmp_1539_1038));
  ltmp_1540_102F = *ltmp_1533_1035;
  ltmp_1541_1019 = *(&ltmp_1540_102F->field2.field1);
  ltmp_1541_1019((&ltmp_1540_102F->field2), ((signed char *)ltmp_1532_1036));
  ltmp_1542_13 = (unsigned char *)ltmp_1531_1034;
  ltmp_1543_7_2E = 0u;
  goto ltmp_1552_D;

  do {
ltmp_1552_D:
  ltmp_1543_7 = ltmp_1543_7_2E;
  *(&ltmp_1542_13[ltmp_1543_7]) = 0;
  ltmp_1544_7 = ltmp_1543_7 + 1u;
  if ((ltmp_1544_7 == 32u)) {
    goto ltmp_1553_D;
  } else {
    ltmp_1543_7_2E = ltmp_1544_7;
    goto ltmp_1552_D;
  }

  } while (1);
ltmp_1553_D:
  return;
ltmp_1550_D:
  ltmp_1545_13 = (unsigned char *)ltmp_1531_1034;
  ltmp_1546_7_2E = 0u;
  goto ltmp_1554_D;

  do {
ltmp_1554_D:
  ltmp_1546_7 = ltmp_1546_7_2E;
  *(&ltmp_1545_13[ltmp_1546_7]) = 0;
  ltmp_1547_7 = ltmp_1546_7 + 1u;
  if ((ltmp_1547_7 == 32u)) {
    goto ltmp_1555_D;
  } else {
    ltmp_1546_7_2E = ltmp_1547_7;
    goto ltmp_1554_D;
  }

  } while (1);
ltmp_1555_D:
  return;
ltmp_1548_D:
  return;
}


static void ltmp_20_100C(struct l_struct_2E_A5 *ltmp_1556_1024) {
  unsigned char *ltmp_1557_13;
  plzo_uint32_t ltmp_1558_7;
  plzo_uint32_t ltmp_1559_7;
  plzo_uint32_t *ltmp_1560_17;
  plzo_uint32_t ltmp_1561_7;
  plzo_uint32_t ltmp_1562_7;
  plzo_uint32_t ltmp_1563_7;
  plzo_uint32_t ltmp_1564_7;
  plzo_uint32_t *ltmp_1565_17;
  plzo_uint32_t ltmp_1566_7;
  unsigned char *ltmp_1567_13;
  plzo_uint32_t ltmp_1568_7;
  unsigned char *ltmp_1569_13;
  unsigned char *ltmp_1569_13_2E;
  unsigned char *ltmp_1570_13;
  plzo_uint32_t ltmp_1571_7;
  unsigned char *ltmp_1572_13;
  unsigned char *ltmp_1572_13_2E;
  plzo_uint32_t ltmp_1573_7;
  unsigned char *ltmp_1574_13;
  unsigned char *ltmp_1574_13_2E;
  unsigned char *ltmp_1575_13;
  unsigned char *ltmp_1575_13_2E;

  ltmp_1557_13 = *(&ltmp_1556_1024->field5);
  ltmp_1558_7 = *(&ltmp_1556_1024->field0);
  ltmp_1559_7 = *(&ltmp_1556_1024->field2);
  if ((ltmp_1559_7 == 0u)) {
    goto ltmp_1576_D;
  } else {
    goto ltmp_1577_D;
  }

ltmp_1576_D:
  *ltmp_1557_13 = ((unsigned char )ltmp_1558_7);
  ltmp_1560_17 = &ltmp_1556_1024->field3;
  ltmp_1561_7 = *ltmp_1560_17;
  if ((ltmp_1561_7 > 15u)) {
    goto ltmp_1578_D;
  } else {
    goto ltmp_1579_D;
  }

ltmp_1578_D:
  *(&ltmp_1557_13[1]) = ((unsigned char )(ltmp_1558_7 >> 8));
  ltmp_1562_7 = *ltmp_1560_17;
  if ((ltmp_1562_7 > 23u)) {
    goto ltmp_1580_D;
  } else {
    goto ltmp_1581_D;
  }

ltmp_1579_D:
  if ((ltmp_1561_7 > 23u)) {
    goto ltmp_1580_D;
  } else {
    goto ltmp_1581_D;
  }

ltmp_1580_D:
  *(&ltmp_1557_13[2]) = ((unsigned char )(ltmp_1558_7 >> 16));
  ltmp_1563_7 = *ltmp_1560_17;
  if ((ltmp_1563_7 > 31u)) {
    goto ltmp_1582_D;
  } else {
    goto ltmp_1583_D;
  }

ltmp_1581_D:
  ltmp_1564_7 = *ltmp_1560_17;
  if ((ltmp_1564_7 > 31u)) {
    goto ltmp_1582_D;
  } else {
    goto ltmp_1583_D;
  }

ltmp_1582_D:
  *(&ltmp_1557_13[3]) = ((unsigned char )(ltmp_1558_7 >> 24));
  return;
ltmp_1577_D:
  ltmp_1565_17 = &ltmp_1556_1024->field3;
  ltmp_1566_7 = *ltmp_1565_17;
  if ((ltmp_1566_7 > 31u)) {
    goto ltmp_1584_D;
  } else {
    goto ltmp_1585_D;
  }

ltmp_1584_D:
  ltmp_1567_13 = &ltmp_1557_13[1];
  *ltmp_1557_13 = ((unsigned char )(ltmp_1558_7 >> 24));
  ltmp_1568_7 = *ltmp_1565_17;
  if ((ltmp_1568_7 > 23u)) {
    ltmp_1569_13_2E = ltmp_1567_13;
    goto ltmp_1586_D;
  } else {
    ltmp_1572_13_2E = ltmp_1567_13;
    goto ltmp_1587_D;
  }

ltmp_1585_D:
  if ((ltmp_1566_7 > 23u)) {
    ltmp_1569_13_2E = ltmp_1557_13;
    goto ltmp_1586_D;
  } else {
    ltmp_1572_13_2E = ltmp_1557_13;
    goto ltmp_1587_D;
  }

ltmp_1586_D:
  ltmp_1569_13 = ltmp_1569_13_2E;
  ltmp_1570_13 = &ltmp_1569_13[1];
  *ltmp_1569_13 = ((unsigned char )(ltmp_1558_7 >> 16));
  ltmp_1571_7 = *ltmp_1565_17;
  if ((ltmp_1571_7 > 15u)) {
    ltmp_1574_13_2E = ltmp_1570_13;
    goto ltmp_1588_D;
  } else {
    ltmp_1575_13_2E = ltmp_1570_13;
    goto ltmp_1589_D;
  }

ltmp_1587_D:
  ltmp_1572_13 = ltmp_1572_13_2E;
  ltmp_1573_7 = *ltmp_1565_17;
  if ((ltmp_1573_7 > 15u)) {
    ltmp_1574_13_2E = ltmp_1572_13;
    goto ltmp_1588_D;
  } else {
    ltmp_1575_13_2E = ltmp_1572_13;
    goto ltmp_1589_D;
  }

ltmp_1588_D:
  ltmp_1574_13 = ltmp_1574_13_2E;
  *ltmp_1574_13 = ((unsigned char )(ltmp_1558_7 >> 8));
  *(&ltmp_1574_13[1]) = ((unsigned char )ltmp_1558_7);
  return;
ltmp_1589_D:
  ltmp_1575_13 = ltmp_1575_13_2E;
  *ltmp_1575_13 = ((unsigned char )ltmp_1558_7);
  return;
ltmp_1583_D:
  return;
}


static void ltmp_21_100D(struct l_struct_2E_Lzo1xE *ltmp_1590_103C, plzo_uint32_t ltmp_1591_7, plzo_uint32_t ltmp_1592_7) {
  unsigned char **ltmp_1593_1025;
  unsigned char *ltmp_1594_13;
  plzo_uint32_t ltmp_1595_7;
  plzo_uint32_t ltmp_1596_7;
  unsigned char *ltmp_1597_13;
  plzo_uint32_t ltmp_1598_7;
  plzo_uint32_t ltmp_1599_7;
  plzo_uint32_t ltmp_1600_7;
  plzo_int32_t ltmp_1601_8;
  unsigned char *ltmp_1602_13;
  unsigned char ltmp_1603_3;
  unsigned char *ltmp_1604_13;
  unsigned char *ltmp_1605_13;
  plzo_uint32_t ltmp_1606_7;
  plzo_uint32_t ltmp_1607_7;
  plzo_uint32_t ltmp_1608_7;
  plzo_uint32_t ltmp_1609_7;
  plzo_uint32_t ltmp_1610_7;
  plzo_uint32_t ltmp_1611_7;
  unsigned char *ltmp_1612_13;
  plzo_uint32_t ltmp_1613_7;
  plzo_uint32_t ltmp_1614_7;
  plzo_uint32_t ltmp_1615_7;
  unsigned char *ltmp_1616_13;
  plzo_uint32_t ltmp_1617_7;
  plzo_uint32_t ltmp_1618_7;
  unsigned char *ltmp_1619_13;
  plzo_uint32_t ltmp_1620_7;
  plzo_uint32_t ltmp_1621_7;
  plzo_uint32_t ltmp_1621_7_2E;
  plzo_uint32_t ltmp_1622_7;
  plzo_uint32_t ltmp_1623_7;
  unsigned char *ltmp_1624_13;
  unsigned char *ltmp_1625_13;
  unsigned char *ltmp_1625_13_2E;
  plzo_uint32_t ltmp_1626_7;
  plzo_uint32_t ltmp_1626_7_2E;
  unsigned char *ltmp_1627_13;
  unsigned char *ltmp_1628_13;
  unsigned char *ltmp_1628_13_2E;
  unsigned char *ltmp_1629_13;
  plzo_uint32_t ltmp_1630_7;
  plzo_uint32_t ltmp_1631_7;
  plzo_uint32_t ltmp_1632_7;
  unsigned char *ltmp_1633_13;
  plzo_uint32_t ltmp_1634_7;
  plzo_uint32_t ltmp_1635_7;
  unsigned char *ltmp_1636_13;
  plzo_uint32_t ltmp_1637_7;
  plzo_uint32_t ltmp_1638_7;
  plzo_uint32_t ltmp_1638_7_2E;
  plzo_uint32_t ltmp_1639_7;
  plzo_uint32_t ltmp_1640_7;
  unsigned char *ltmp_1641_13;
  unsigned char *ltmp_1642_13;
  unsigned char *ltmp_1642_13_2E;
  plzo_uint32_t ltmp_1643_7;
  plzo_uint32_t ltmp_1643_7_2E;
  unsigned char *ltmp_1644_13;
  unsigned char *ltmp_1645_13;
  unsigned char *ltmp_1645_13_2E;
  unsigned char *ltmp_1646_13;
  unsigned char *ltmp_1647_13;
  unsigned char *ltmp_1647_13_2E;

  ltmp_0_1000((&ltmp_1590_103C->field0.field0), ltmp_1591_7, ltmp_1592_7);
  ltmp_1593_1025 = &ltmp_1590_103C->field0.field0.field1.field6;
  ltmp_1594_13 = *ltmp_1593_1025;
  if ((ltmp_1591_7 == 2u)) {
    goto ltmp_1648_D;
  } else {
    goto ltmp_1649_D;
  }

ltmp_1648_D:
  ltmp_1595_7 = ltmp_1592_7 + 4294967295u;
  ltmp_1596_7 = *(&ltmp_1590_103C->field0.field1);
  *ltmp_1594_13 = (((unsigned char )(((unsigned char )ltmp_1596_7) | (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1595_7) << 2))) & 12))))));
  ltmp_1597_13 = &ltmp_1594_13[2];
  *(&ltmp_1594_13[1]) = ((unsigned char )(ltmp_1595_7 >> 2));
  ltmp_1647_13_2E = ltmp_1597_13;
  goto ltmp_1650_D;

ltmp_1649_D:
  ltmp_1598_7 = *(&ltmp_1590_103C->field0.field18);
  if ((ltmp_1598_7 < ltmp_1591_7)) {
    goto ltmp_1651_D;
  } else {
    goto ltmp_1652_D;
  }

ltmp_1652_D:
  ltmp_1599_7 = *(&ltmp_1590_103C->field0.field7);
  if ((ltmp_1599_7 < ltmp_1592_7)) {
    goto ltmp_1651_D;
  } else {
    goto ltmp_1653_D;
  }

ltmp_1653_D:
  ltmp_1600_7 = ltmp_1592_7 + 4294967295u;
  ltmp_1601_8 = *(&ltmp_1590_103C->field0.field0.field6.field0);
  ltmp_1602_13 = &ltmp_1594_13[1];
  ltmp_1603_3 = (unsigned char )ltmp_1591_7;
  if ((ltmp_1601_8 == 0)) {
    goto ltmp_1654_D;
  } else {
    goto ltmp_1655_D;
  }

ltmp_1654_D:
  *ltmp_1594_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1600_7) << 2))) & 28))) | (((unsigned char )((((unsigned char )(ltmp_1603_3 << 5))) + 224))))));
  ltmp_1604_13 = &ltmp_1594_13[2];
  *ltmp_1602_13 = ((unsigned char )(ltmp_1600_7 >> 3));
  ltmp_1647_13_2E = ltmp_1604_13;
  goto ltmp_1650_D;

ltmp_1655_D:
  *ltmp_1594_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1600_7) << 2))) & 12))) | (((unsigned char )((((unsigned char )(ltmp_1603_3 << 4))) + 16))))));
  ltmp_1605_13 = &ltmp_1594_13[2];
  *ltmp_1602_13 = ((unsigned char )(ltmp_1600_7 >> 2));
  ltmp_1647_13_2E = ltmp_1605_13;
  goto ltmp_1650_D;

ltmp_1651_D:
  ltmp_1606_7 = *(&ltmp_1590_103C->field0.field13);
  if ((ltmp_1606_7 == ltmp_1591_7)) {
    goto ltmp_1656_D;
  } else {
    goto ltmp_1657_D;
  }

ltmp_1656_D:
  ltmp_1607_7 = *(&ltmp_1590_103C->field0.field11);
  if ((ltmp_1607_7 < ltmp_1592_7)) {
    goto ltmp_1657_D;
  } else {
    goto ltmp_1658_D;
  }

ltmp_1658_D:
  ltmp_1608_7 = *(&ltmp_1590_103C->field0.field0.field17);
  if ((ltmp_1608_7 > 3u)) {
    goto ltmp_1659_D;
  } else {
    goto ltmp_1657_D;
  }

ltmp_1659_D:
  ltmp_1609_7 = *(&ltmp_1590_103C->field0.field7);
  ltmp_1610_7 = (ltmp_1592_7 - ltmp_1609_7) + 4294967295u;
  ltmp_1611_7 = *(&ltmp_1590_103C->field0.field1);
  *ltmp_1594_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )ltmp_1610_7) << 2))) & 12))) | ((unsigned char )ltmp_1611_7))));
  ltmp_1612_13 = &ltmp_1594_13[2];
  *(&ltmp_1594_13[1]) = ((unsigned char )(ltmp_1610_7 >> 2));
  ltmp_1647_13_2E = ltmp_1612_13;
  goto ltmp_1650_D;

ltmp_1657_D:
  ltmp_1613_7 = *(&ltmp_1590_103C->field0.field8);
  if ((ltmp_1613_7 < ltmp_1592_7)) {
    goto ltmp_1660_D;
  } else {
    goto ltmp_1661_D;
  }

ltmp_1661_D:
  ltmp_1614_7 = ltmp_1592_7 + 4294967295u;
  ltmp_1615_7 = *(&ltmp_1590_103C->field0.field19);
  if ((ltmp_1615_7 < ltmp_1591_7)) {
    goto ltmp_1662_D;
  } else {
    goto ltmp_1663_D;
  }

ltmp_1663_D:
  ltmp_1616_13 = &ltmp_1594_13[1];
  ltmp_1617_7 = *(&ltmp_1590_103C->field0.field3);
  *ltmp_1594_13 = (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1591_7) + 254))) + ((unsigned char )ltmp_1617_7))));
  ltmp_1628_13_2E = ltmp_1616_13;
  goto ltmp_1664_D;

ltmp_1662_D:
  ltmp_1618_7 = ltmp_1591_7 - ltmp_1615_7;
  ltmp_1619_13 = &ltmp_1594_13[1];
  ltmp_1620_7 = *(&ltmp_1590_103C->field0.field3);
  *ltmp_1594_13 = ((unsigned char )ltmp_1620_7);
  if ((ltmp_1618_7 > 255u)) {
    ltmp_1621_7_2E = 0u;
    goto ltmp_1665_D;
  } else {
    ltmp_1625_13_2E = ltmp_1619_13;
    ltmp_1626_7_2E = ltmp_1618_7;
    goto ltmp_1666_D;
  }

  do {
ltmp_1665_D:
  ltmp_1621_7 = ltmp_1621_7_2E;
  ltmp_1622_7 = ltmp_1621_7 + 1u;
  ltmp_1623_7 = ((ltmp_1621_7*4294967041u) + ltmp_1618_7) + 4294967041u;
  *(&ltmp_1594_13[ltmp_1622_7]) = 0;
  if ((ltmp_1623_7 > 255u)) {
    ltmp_1621_7_2E = ltmp_1622_7;
    goto ltmp_1665_D;
  } else {
    goto ltmp_1667_D;
  }

  } while (1);
ltmp_1667_D:
  ltmp_1624_13 = &ltmp_1594_13[(((plzo_int32_t )ltmp_1621_7) + 2)];
  ltmp_1625_13_2E = ltmp_1624_13;
  ltmp_1626_7_2E = ltmp_1623_7;
  goto ltmp_1666_D;

ltmp_1666_D:
  ltmp_1625_13 = ltmp_1625_13_2E;
  ltmp_1626_7 = ltmp_1626_7_2E;
  ltmp_1627_13 = &ltmp_1625_13[1];
  *ltmp_1625_13 = ((unsigned char )ltmp_1626_7);
  ltmp_1628_13_2E = ltmp_1627_13;
  goto ltmp_1664_D;

ltmp_1664_D:
  ltmp_1628_13 = ltmp_1628_13_2E;
  *ltmp_1628_13 = (((unsigned char )(((unsigned char )ltmp_1614_7) << 2)));
  ltmp_1629_13 = &ltmp_1628_13[2];
  *(&ltmp_1628_13[1]) = ((unsigned char )(ltmp_1614_7 >> 6));
  ltmp_1647_13_2E = ltmp_1629_13;
  goto ltmp_1650_D;

ltmp_1660_D:
  ltmp_1630_7 = ltmp_1592_7 - ltmp_1613_7;
  ltmp_1631_7 = (ltmp_1630_7 >> 11) & 8u;
  ltmp_1632_7 = *(&ltmp_1590_103C->field0.field20);
  if ((ltmp_1632_7 < ltmp_1591_7)) {
    goto ltmp_1668_D;
  } else {
    goto ltmp_1669_D;
  }

ltmp_1669_D:
  ltmp_1633_13 = &ltmp_1594_13[1];
  ltmp_1634_7 = *(&ltmp_1590_103C->field0.field4);
  *ltmp_1594_13 = (((unsigned char )(((unsigned char )ltmp_1634_7) + (((unsigned char )(((unsigned char )ltmp_1631_7) | (((unsigned char )(((unsigned char )ltmp_1591_7) + 254)))))))));
  ltmp_1645_13_2E = ltmp_1633_13;
  goto ltmp_1670_D;

ltmp_1668_D:
  ltmp_1635_7 = ltmp_1591_7 - ltmp_1632_7;
  ltmp_1636_13 = &ltmp_1594_13[1];
  ltmp_1637_7 = *(&ltmp_1590_103C->field0.field4);
  *ltmp_1594_13 = (((unsigned char )(((unsigned char )ltmp_1637_7) + ((unsigned char )ltmp_1631_7))));
  if ((ltmp_1635_7 > 255u)) {
    ltmp_1638_7_2E = 0u;
    goto ltmp_1671_D;
  } else {
    ltmp_1642_13_2E = ltmp_1636_13;
    ltmp_1643_7_2E = ltmp_1635_7;
    goto ltmp_1672_D;
  }

  do {
ltmp_1671_D:
  ltmp_1638_7 = ltmp_1638_7_2E;
  ltmp_1639_7 = ltmp_1638_7 + 1u;
  ltmp_1640_7 = ((ltmp_1638_7*4294967041u) + ltmp_1635_7) + 4294967041u;
  *(&ltmp_1594_13[ltmp_1639_7]) = 0;
  if ((ltmp_1640_7 > 255u)) {
    ltmp_1638_7_2E = ltmp_1639_7;
    goto ltmp_1671_D;
  } else {
    goto ltmp_1673_D;
  }

  } while (1);
ltmp_1673_D:
  ltmp_1641_13 = &ltmp_1594_13[(((plzo_int32_t )ltmp_1638_7) + 2)];
  ltmp_1642_13_2E = ltmp_1641_13;
  ltmp_1643_7_2E = ltmp_1640_7;
  goto ltmp_1672_D;

ltmp_1672_D:
  ltmp_1642_13 = ltmp_1642_13_2E;
  ltmp_1643_7 = ltmp_1643_7_2E;
  ltmp_1644_13 = &ltmp_1642_13[1];
  *ltmp_1642_13 = ((unsigned char )ltmp_1643_7);
  ltmp_1645_13_2E = ltmp_1644_13;
  goto ltmp_1670_D;

ltmp_1670_D:
  ltmp_1645_13 = ltmp_1645_13_2E;
  *ltmp_1645_13 = (((unsigned char )(((unsigned char )ltmp_1630_7) << 2)));
  ltmp_1646_13 = &ltmp_1645_13[2];
  *(&ltmp_1645_13[1]) = ((unsigned char )(ltmp_1630_7 >> 6));
  ltmp_1647_13_2E = ltmp_1646_13;
  goto ltmp_1650_D;

ltmp_1650_D:
  ltmp_1647_13 = ltmp_1647_13_2E;
  *(&ltmp_1590_103C->field0.field0.field15) = ltmp_1591_7;
  *(&ltmp_1590_103C->field0.field0.field16) = ltmp_1592_7;
  *ltmp_1593_1025 = ltmp_1647_13;
  return;
}


static void ltmp_22_100E(struct l_struct_2E_Lzo1xE *ltmp_1674_103C, unsigned char *ltmp_1675_13, plzo_uint32_t ltmp_1676_7, plzo_uint32_t ltmp_1677_7) {
  plzo_uint32_t *ltmp_1678_17;
  plzo_uint32_t ltmp_1679_7;
  plzo_uint32_t *ltmp_1680_17;
  plzo_uint32_t ltmp_1681_7;
  plzo_uint32_t ltmp_1682_7;
  plzo_uint32_t *ltmp_1683_17;
  unsigned char **ltmp_1684_1025;
  unsigned char *ltmp_1685_13;
  unsigned char *ltmp_1686_13;
  unsigned char *ltmp_1687_13;
  unsigned char *ltmp_1688_13;
  unsigned char ltmp_1689_3;
  unsigned char *ltmp_1690_13;
  plzo_uint32_t ltmp_1691_7;
  unsigned char *ltmp_1692_13;
  plzo_uint32_t ltmp_1693_7;
  plzo_uint32_t ltmp_1693_7_2E;
  plzo_uint32_t ltmp_1694_7;
  plzo_uint32_t ltmp_1695_7;
  unsigned char *ltmp_1696_13;
  plzo_uint32_t ltmp_1697_7;
  plzo_uint32_t ltmp_1697_7_2E;
  unsigned char *ltmp_1698_13;
  unsigned char *ltmp_1698_13_2E;
  unsigned char *ltmp_1699_13;
  unsigned char *ltmp_1700_13;
  unsigned char *ltmp_1700_13_2E;
  plzo_uint32_t ltmp_1701_7;
  plzo_uint32_t ltmp_1701_7_2E;
  unsigned char ltmp_1702_3;
  plzo_uint32_t ltmp_1703_7;

  if ((ltmp_1676_7 == 0u)) {
    goto ltmp_1704_D;
  } else {
    goto ltmp_1705_D;
  }

ltmp_1705_D:
  ltmp_1678_17 = &ltmp_1674_103C->field0.field0.field2.field5;
  ltmp_1679_7 = *ltmp_1678_17;
  ltmp_1680_17 = &ltmp_1674_103C->field0.field0.field2.field4;
  if ((ltmp_1679_7 < ltmp_1676_7)) {
    goto ltmp_1706_D;
  } else {
    goto ltmp_1707_D;
  }

ltmp_1706_D:
  *ltmp_1678_17 = ltmp_1676_7;
  ltmp_1681_7 = *ltmp_1680_17;
  if ((ltmp_1681_7 > ltmp_1676_7)) {
    goto ltmp_1708_D;
  } else {
    goto ltmp_1704_D;
  }

ltmp_1707_D:
  ltmp_1682_7 = *ltmp_1680_17;
  if ((ltmp_1682_7 > ltmp_1676_7)) {
    goto ltmp_1708_D;
  } else {
    goto ltmp_1704_D;
  }

ltmp_1708_D:
  *ltmp_1680_17 = ltmp_1676_7;
  goto ltmp_1704_D;

ltmp_1704_D:
  ltmp_1683_17 = &ltmp_1674_103C->field0.field0.field18;
  if ((ltmp_1676_7 == 0u)) {
    goto ltmp_1709_D;
  } else {
    goto ltmp_1710_D;
  }

ltmp_1709_D:
  *ltmp_1683_17 = 0u;
  *(&ltmp_1674_103C->field0.field0.field17) = 0u;
  return;
ltmp_1710_D:
  *ltmp_1683_17 = ltmp_1677_7;
  *(&ltmp_1674_103C->field0.field0.field17) = ltmp_1676_7;
  ltmp_1684_1025 = &ltmp_1674_103C->field0.field0.field1.field6;
  ltmp_1685_13 = *ltmp_1684_1025;
  ltmp_1686_13 = *(&ltmp_1674_103C->field0.field0.field9);
  if (((ltmp_1686_13 != ltmp_1685_13) | (ltmp_1676_7 > 238u))) {
    goto ltmp_1711_D;
  } else {
    goto ltmp_1712_D;
  }

ltmp_1712_D:
  ltmp_1687_13 = &ltmp_1685_13[1];
  *ltmp_1685_13 = (((unsigned char )(((unsigned char )ltmp_1676_7) + 17)));
  ltmp_1700_13_2E = ltmp_1687_13;
  ltmp_1701_7_2E = 0u;
  goto ltmp_1713_D;

ltmp_1711_D:
  if ((ltmp_1676_7 > 3u)) {
    goto ltmp_1714_D;
  } else {
    goto ltmp_1715_D;
  }

ltmp_1715_D:
  ltmp_1688_13 = &ltmp_1685_13[-2];
  ltmp_1689_3 = *ltmp_1688_13;
  *ltmp_1688_13 = (((unsigned char )(ltmp_1689_3 | ((unsigned char )ltmp_1676_7))));
  ltmp_1700_13_2E = ltmp_1685_13;
  ltmp_1701_7_2E = 0u;
  goto ltmp_1713_D;

ltmp_1714_D:
  if ((ltmp_1676_7 > 18u)) {
    goto ltmp_1716_D;
  } else {
    goto ltmp_1717_D;
  }

ltmp_1717_D:
  ltmp_1690_13 = &ltmp_1685_13[1];
  *ltmp_1685_13 = (((unsigned char )(((unsigned char )ltmp_1676_7) + 253)));
  ltmp_1700_13_2E = ltmp_1690_13;
  ltmp_1701_7_2E = 0u;
  goto ltmp_1713_D;

ltmp_1716_D:
  ltmp_1691_7 = ltmp_1676_7 + 4294967278u;
  ltmp_1692_13 = &ltmp_1685_13[1];
  *ltmp_1685_13 = 0;
  if ((ltmp_1691_7 > 255u)) {
    ltmp_1693_7_2E = 0u;
    goto ltmp_1718_D;
  } else {
    ltmp_1697_7_2E = ltmp_1691_7;
    ltmp_1698_13_2E = ltmp_1692_13;
    goto ltmp_1719_D;
  }

  do {
ltmp_1718_D:
  ltmp_1693_7 = ltmp_1693_7_2E;
  ltmp_1694_7 = ltmp_1693_7 + 1u;
  ltmp_1695_7 = ((ltmp_1693_7*4294967041u) + ltmp_1691_7) + 4294967041u;
  *(&ltmp_1685_13[ltmp_1694_7]) = 0;
  if ((ltmp_1695_7 > 255u)) {
    ltmp_1693_7_2E = ltmp_1694_7;
    goto ltmp_1718_D;
  } else {
    goto ltmp_1720_D;
  }

  } while (1);
ltmp_1720_D:
  ltmp_1696_13 = &ltmp_1685_13[(((plzo_int32_t )ltmp_1693_7) + 2)];
  ltmp_1697_7_2E = ltmp_1695_7;
  ltmp_1698_13_2E = ltmp_1696_13;
  goto ltmp_1719_D;

ltmp_1719_D:
  ltmp_1697_7 = ltmp_1697_7_2E;
  ltmp_1698_13 = ltmp_1698_13_2E;
  ltmp_1699_13 = &ltmp_1698_13[1];
  *ltmp_1698_13 = ((unsigned char )ltmp_1697_7);
  ltmp_1700_13_2E = ltmp_1699_13;
  ltmp_1701_7_2E = 0u;
  goto ltmp_1713_D;

  do {
ltmp_1713_D:
  ltmp_1700_13 = ltmp_1700_13_2E;
  ltmp_1701_7 = ltmp_1701_7_2E;
  ltmp_1702_3 = *(&ltmp_1675_13[ltmp_1701_7]);
  *(&ltmp_1700_13[ltmp_1701_7]) = ltmp_1702_3;
  ltmp_1703_7 = ltmp_1701_7 + 1u;
  if ((ltmp_1703_7 == ltmp_1676_7)) {
    goto ltmp_1721_D;
  } else {
    ltmp_1700_13_2E = ltmp_1700_13;
    ltmp_1701_7_2E = ltmp_1703_7;
    goto ltmp_1713_D;
  }

  } while (1);
ltmp_1721_D:
  *ltmp_1684_1025 = (&ltmp_1700_13[(((plzo_int32_t )ltmp_1701_7) + 1)]);
  return;
}


static void ltmp_23_100D(struct l_struct_2E_Lzo1xE *ltmp_1722_103C, plzo_uint32_t ltmp_1723_7, plzo_uint32_t ltmp_1724_7) {
  struct l_struct_2E_A7 *ltmp_1725_1038;
  struct l_struct_2E_A7 *ltmp_1726_1038;
  struct l_struct_2E_A7 *ltmp_1726_1038_2E;
  plzo_uint32_t ltmp_1727_7;
  plzo_uint32_t ltmp_1728_7;
  plzo_uint32_t ltmp_1729_7;
  plzo_uint32_t ltmp_1729_7_2E;
  struct l_struct_2E_A7 *ltmp_1730_1038;
  struct l_struct_2E_A7 *ltmp_1730_1038_2E;
  plzo_uint32_t ltmp_1731_7;
  plzo_uint32_t *ltmp_1732_17;
  plzo_uint32_t ltmp_1733_7;
  struct l_struct_2E_A7 *ltmp_1734_1038;
  struct l_struct_2E_A7 *ltmp_1735_1038;

  ltmp_11_1000((&ltmp_1722_103C->field0.field0), ltmp_1723_7, ltmp_1724_7);
  if ((ltmp_1723_7 == 0u)) {
    goto ltmp_1736_D;
  } else {
    goto ltmp_1737_D;
  }

ltmp_1736_D:
  ltmp_1725_1038 = *(&ltmp_1722_103C->field0.field0.field19.field0);
  if (((ltmp_1723_7 >= ltmp_1724_7) | (ltmp_1723_7 > 3u))) {
    goto ltmp_1737_D;
  } else {
    ltmp_1729_7_2E = 0u;
    ltmp_1730_1038_2E = ltmp_1725_1038;
    goto ltmp_1738_D;
  }

  do {
ltmp_1738_D:
  ltmp_1729_7 = ltmp_1729_7_2E;
  ltmp_1730_1038 = ltmp_1730_1038_2E;
  ltmp_1731_7 = ltmp_1729_7 + ltmp_1723_7;
  ltmp_1732_17 = &ltmp_1730_1038->field0;
  ltmp_1733_7 = *ltmp_1732_17;
  if ((ltmp_1733_7 == 2u)) {
    goto ltmp_1739_D;
  } else {
    goto ltmp_1740_D;
  }

ltmp_1741_D:
  ltmp_1726_1038 = ltmp_1726_1038_2E;
  ltmp_1727_7 = ltmp_1731_7 + 1u;
  ltmp_1728_7 = ltmp_1729_7 + 1u;
  if (((ltmp_1727_7 > 3u) | (ltmp_1727_7 >= ltmp_1724_7))) {
    goto ltmp_1737_D;
  } else {
    ltmp_1729_7_2E = ltmp_1728_7;
    ltmp_1730_1038_2E = ltmp_1726_1038;
    goto ltmp_1738_D;
  }

ltmp_1739_D:
  *ltmp_1732_17 = 0u;
  ltmp_1734_1038 = &ltmp_1730_1038[1];
  ltmp_1726_1038_2E = ltmp_1734_1038;
  goto ltmp_1741_D;

ltmp_1740_D:
  ltmp_1735_1038 = &ltmp_1730_1038[1];
  ltmp_1726_1038_2E = ltmp_1735_1038;
  goto ltmp_1741_D;

  } while (1);
ltmp_1737_D:
  return;
}


static void ltmp_24_100F(struct l_struct_2E_Lzo1E *ltmp_1742_103D, plzo_uint32_t ltmp_1743_7, plzo_uint32_t ltmp_1744_7) {

  ltmp_11_1000((&ltmp_1742_103D->field0), ltmp_1743_7, ltmp_1744_7);
  return;
}


static plzo_int32_t plzo_lzo1x_99_compress_internal(unsigned char *ltmp_1745_13, plzo_uint32_t ltmp_1746_7, unsigned char *ltmp_1747_13, plzo_uint32_t *ltmp_1748_17, struct plzo_callback_t *ltmp_1749_1026, plzo_int32_t ltmp_1750_8, struct plzo_compress_config_t *ltmp_1751_1027, plzo_uint32_t *ltmp_1752_17) {
  struct l_struct_2E_Lzo1xE ltmp_1753_103C;
  plzo_int32_t  (***ltmp_1754_102C) ();
  struct l_struct_2E_E *ltmp_1755_1023;
  plzo_int32_t ltmp_1756_8;
  plzo_int32_t ltmp_1757_8;
  plzo_int32_t ltmp_1758_8;
  plzo_int32_t ltmp_1758_8_2E;
  plzo_int32_t ltmp_1759_8;
  plzo_int32_t ltmp_1760_8;
  plzo_int32_t ltmp_1760_8_2E;
  plzo_uint32_t ltmp_1761_7;
  plzo_uint32_t ltmp_1762_7;
  plzo_uint32_t ltmp_1763_7;
  plzo_uint32_t ltmp_1764_7;
  plzo_uint32_t ltmp_1765_7;
  plzo_uint32_t ltmp_1766_7;
  plzo_uint32_t ltmp_1767_7;
  plzo_uint32_t ltmp_1768_7;

  ltmp_25_1010((&ltmp_1753_103C.field0));
  ltmp_1754_102C = &ltmp_1753_103C.field0.field0.field0;
  *ltmp_1754_102C = (&(ltmp_68_1021[2]));
  *(&ltmp_1753_103C.field0.field1) = 0u;
  *(&ltmp_1753_103C.field0.field2) = 64u;
  *(&ltmp_1753_103C.field0.field3) = 32u;
  *(&ltmp_1753_103C.field0.field4) = 16u;
  *(&ltmp_1753_103C.field0.field5) = 0u;
  *(&ltmp_1753_103C.field0.field6) = 1024u;
  *(&ltmp_1753_103C.field0.field7) = 2048u;
  *(&ltmp_1753_103C.field0.field8) = 16384u;
  *(&ltmp_1753_103C.field0.field9) = 49151u;
  *(&ltmp_1753_103C.field0.field10) = 0u;
  *(&ltmp_1753_103C.field0.field11) = 3072u;
  *(&ltmp_1753_103C.field0.field12) = 2u;
  *(&ltmp_1753_103C.field0.field13) = 3u;
  *(&ltmp_1753_103C.field0.field14) = 3u;
  *(&ltmp_1753_103C.field0.field15) = 3u;
  *(&ltmp_1753_103C.field0.field16) = 0u;
  *(&ltmp_1753_103C.field0.field17) = 2u;
  *(&ltmp_1753_103C.field0.field18) = 8u;
  *(&ltmp_1753_103C.field0.field19) = 33u;
  *(&ltmp_1753_103C.field0.field20) = 9u;
  *(&ltmp_1753_103C.field0.field21) = 0u;
  *(&ltmp_1753_103C.field0.field0.field6.field0) = 0;
  *(&ltmp_1753_103C.field0.field0.field6.field3) = 49151u;
  ltmp_1755_1023 = &ltmp_1753_103C.field0.field0;
  ltmp_1756_8 = ltmp_2_1002(ltmp_1755_1023, ltmp_1750_8, ltmp_1749_1026);
  if (((ltmp_1756_8 != 0) | (ltmp_1751_1027 == ((struct plzo_compress_config_t *)0)))) {
    ltmp_1758_8_2E = ltmp_1756_8;
    goto ltmp_1769_D;
  } else {
    goto ltmp_1770_D;
  }

ltmp_1770_D:
  ltmp_1757_8 = ltmp_3_1003(ltmp_1755_1023, ltmp_1751_1027);
  ltmp_1758_8_2E = ltmp_1757_8;
  goto ltmp_1769_D;

ltmp_1769_D:
  ltmp_1758_8 = ltmp_1758_8_2E;
  if ((ltmp_1758_8 == 0)) {
    goto ltmp_1771_D;
  } else {
    ltmp_1760_8_2E = ltmp_1758_8;
    goto ltmp_1772_D;
  }

ltmp_1771_D:
  ltmp_1759_8 = ltmp_4_1004(ltmp_1755_1023, ltmp_1745_13, ltmp_1746_7, ltmp_1747_13, ltmp_1748_17);
  ltmp_1760_8_2E = ltmp_1759_8;
  goto ltmp_1772_D;

ltmp_1772_D:
  ltmp_1760_8 = ltmp_1760_8_2E;
  if (((ltmp_1760_8 != 0) | (ltmp_1752_17 == ((plzo_uint32_t *)0)))) {
    goto ltmp_1773_D;
  } else {
    goto ltmp_1774_D;
  }

ltmp_1774_D:
  ltmp_1761_7 = *(&ltmp_1753_103C.field0.field0.field2.field0);
  *ltmp_1752_17 = ltmp_1761_7;
  ltmp_1762_7 = *(&ltmp_1753_103C.field0.field0.field2.field1);
  *(&ltmp_1752_17[1]) = ltmp_1762_7;
  ltmp_1763_7 = *(&ltmp_1753_103C.field0.field0.field2.field2);
  *(&ltmp_1752_17[2]) = ltmp_1763_7;
  ltmp_1764_7 = *(&ltmp_1753_103C.field0.field0.field2.field3);
  *(&ltmp_1752_17[3]) = ltmp_1764_7;
  ltmp_1765_7 = *(&ltmp_1753_103C.field0.field0.field2.field4);
  *(&ltmp_1752_17[4]) = ltmp_1765_7;
  ltmp_1766_7 = *(&ltmp_1753_103C.field0.field0.field2.field5);
  *(&ltmp_1752_17[5]) = ltmp_1766_7;
  ltmp_1767_7 = *(&ltmp_1753_103C.field0.field0.field2.field6);
  *(&ltmp_1752_17[6]) = ltmp_1767_7;
  ltmp_1768_7 = *(&ltmp_1753_103C.field0.field0.field2.field7);
  *(&ltmp_1752_17[7]) = ltmp_1768_7;
  goto ltmp_1773_D;

ltmp_1773_D:
  *ltmp_1754_102C = (&(ltmp_69_1021[2]));
  ltmp_5_1005(ltmp_1755_1023);
  return ltmp_1760_8;
}


static void ltmp_25_1010(struct l_struct_2E_Lzo1E *ltmp_1775_103D) {
  unsigned char **ltmp_1776_1025;
  plzo_int32_t  (***ltmp_1777_102C) ();
  unsigned char *ltmp_1778_13;
  plzo_uint32_t ltmp_1779_7;
  plzo_uint32_t ltmp_1779_7_2E;
  plzo_uint32_t ltmp_1780_7;
  unsigned char *ltmp_1781_13;
  plzo_uint32_t ltmp_1782_7;
  plzo_uint32_t ltmp_1782_7_2E;
  plzo_uint32_t ltmp_1783_7;
  unsigned char *ltmp_1784_13;
  plzo_uint32_t ltmp_1785_7;
  plzo_uint32_t ltmp_1785_7_2E;
  plzo_uint32_t ltmp_1786_7;
  plzo_uint32_t *ltmp_1787_17;
  plzo_uint32_t *ltmp_1788_17;
  unsigned char *ltmp_1789_13;
  plzo_uint32_t ltmp_1790_7;
  plzo_uint32_t ltmp_1790_7_2E;
  plzo_uint32_t ltmp_1791_7;
  unsigned char *ltmp_1792_13;
  plzo_uint32_t ltmp_1793_7;
  plzo_uint32_t ltmp_1793_7_2E;
  plzo_uint32_t ltmp_1794_7;
  unsigned char *ltmp_1795_13;
  plzo_uint32_t ltmp_1796_7;
  plzo_uint32_t ltmp_1796_7_2E;
  plzo_uint32_t ltmp_1797_7;
  unsigned char *ltmp_1798_13;
  plzo_uint32_t ltmp_1799_7;
  plzo_uint32_t ltmp_1799_7_2E;
  plzo_uint32_t ltmp_1800_7;
  plzo_uint32_t *ltmp_1801_17;
  struct l_struct_2E_E_3A__3A_A15 *ltmp_1802_103E;
  unsigned char *ltmp_1803_13;
  plzo_uint32_t ltmp_1804_7;
  plzo_uint32_t ltmp_1804_7_2E;
  plzo_uint32_t ltmp_1805_7;

  *(&ltmp_1775_103D->field0.field1.field2) = 0u;
  *(&ltmp_1775_103D->field0.field1.field3) = 8u;
  *(&ltmp_1775_103D->field0.field1.field4) = 1u;
  *(&ltmp_1775_103D->field0.field1.field0) = 0u;
  *(&ltmp_1775_103D->field0.field1.field1) = 0u;
  *(&ltmp_1775_103D->field0.field1.field5) = ((unsigned char *)0);
  ltmp_1776_1025 = &ltmp_1775_103D->field0.field1.field6;
  *ltmp_1776_1025 = ((unsigned char *)0);
  ltmp_1777_102C = &ltmp_1775_103D->field0.field0;
  *ltmp_1777_102C = (&(ltmp_70_1021[2]));
  *(&ltmp_1775_103D->field0.field20.field0) = 0u;
  *(&ltmp_1775_103D->field0.field20.field1) = 0u;
  *(&ltmp_1775_103D->field0.field20.field2) = 0u;
  *(&ltmp_1775_103D->field0.field20.field3) = ((struct l_struct_2E_A2_3A__3A_A3 *)0);
  *(&ltmp_1775_103D->field0.field20.field4) = ((struct l_struct_2E_A9 *)0);
  *(&ltmp_1775_103D->field0.field3) = 0;
  ltmp_1778_13 = (unsigned char *)(&ltmp_1775_103D->field0.field6);
  ltmp_1779_7_2E = 0u;
  goto ltmp_1806_D;

  do {
ltmp_1806_D:
  ltmp_1779_7 = ltmp_1779_7_2E;
  *(&ltmp_1778_13[ltmp_1779_7]) = 0;
  ltmp_1780_7 = ltmp_1779_7 + 1u;
  if ((ltmp_1780_7 == 96u)) {
    goto ltmp_1807_D;
  } else {
    ltmp_1779_7_2E = ltmp_1780_7;
    goto ltmp_1806_D;
  }

  } while (1);
ltmp_1807_D:
  ltmp_1781_13 = (unsigned char *)(&ltmp_1775_103D->field0.field7);
  ltmp_1782_7_2E = 0u;
  goto ltmp_1808_D;

  do {
ltmp_1808_D:
  ltmp_1782_7 = ltmp_1782_7_2E;
  *(&ltmp_1781_13[ltmp_1782_7]) = 0;
  ltmp_1783_7 = ltmp_1782_7 + 1u;
  if ((ltmp_1783_7 == 64u)) {
    goto ltmp_1809_D;
  } else {
    ltmp_1782_7_2E = ltmp_1783_7;
    goto ltmp_1808_D;
  }

  } while (1);
ltmp_1809_D:
  ltmp_1784_13 = (unsigned char *)(&ltmp_1775_103D->field0.field24);
  ltmp_1785_7_2E = 0u;
  goto ltmp_1810_D;

  do {
ltmp_1810_D:
  ltmp_1785_7 = ltmp_1785_7_2E;
  *(&ltmp_1784_13[ltmp_1785_7]) = 0;
  ltmp_1786_7 = ltmp_1785_7 + 1u;
  if ((ltmp_1786_7 == 32u)) {
    goto ltmp_1811_D;
  } else {
    ltmp_1785_7_2E = ltmp_1786_7;
    goto ltmp_1810_D;
  }

  } while (1);
ltmp_1811_D:
  *(&ltmp_1775_103D->field0.field24.field2.field0) = ltmp_49_1018;
  *(&ltmp_1775_103D->field0.field24.field2.field1) = ltmp_50_1019;
  *(&ltmp_1775_103D->field0.field24.field2.field2) = ltmp_51_101A;
  *(&ltmp_1775_103D->field0.field24.field2.field3) = ((signed char *)0);
  *(&ltmp_1775_103D->field0.field24.field0) = 0u;
  *(&ltmp_1775_103D->field0.field24.field1) = 1024u;
  ltmp_1787_17 = &ltmp_1775_103D->field0.field6.field2;
  *ltmp_1787_17 = 3u;
  *(&ltmp_1775_103D->field0.field6.field4) = 2u;
  *(&ltmp_1775_103D->field0.field6.field5) = 4294967295u;
  ltmp_1788_17 = &ltmp_1775_103D->field0.field6.field3;
  *ltmp_1788_17 = 4294967295u;
  *ltmp_1776_1025 = ((unsigned char *)0);
  *(&ltmp_1775_103D->field0.field10) = ((unsigned char *)0);
  *(&ltmp_1775_103D->field0.field8) = ((unsigned char *)0);
  *(&ltmp_1775_103D->field0.field11) = 0u;
  *(&ltmp_1775_103D->field0.field9) = ((unsigned char *)0);
  *(&ltmp_1775_103D->field0.field12) = 0u;
  *(&ltmp_1775_103D->field0.field13) = 0u;
  *(&ltmp_1775_103D->field0.field14) = 0u;
  *(&ltmp_1775_103D->field0.field15) = 0u;
  *(&ltmp_1775_103D->field0.field16) = 0u;
  *(&ltmp_1775_103D->field0.field17) = 0u;
  *(&ltmp_1775_103D->field0.field18) = 0u;
  ltmp_1789_13 = (unsigned char *)(&ltmp_1775_103D->field0.field2);
  ltmp_1790_7_2E = 0u;
  goto ltmp_1812_D;

  do {
ltmp_1812_D:
  ltmp_1790_7 = ltmp_1790_7_2E;
  *(&ltmp_1789_13[ltmp_1790_7]) = 0;
  ltmp_1791_7 = ltmp_1790_7 + 1u;
  if ((ltmp_1791_7 == 64u)) {
    goto ltmp_1813_D;
  } else {
    ltmp_1790_7_2E = ltmp_1791_7;
    goto ltmp_1812_D;
  }

  } while (1);
ltmp_1813_D:
  *(&ltmp_1775_103D->field0.field2.field0) = 4294967295u;
  *(&ltmp_1775_103D->field0.field2.field2) = 4294967295u;
  *(&ltmp_1775_103D->field0.field2.field4) = 4294967295u;
  ltmp_1792_13 = (unsigned char *)(&ltmp_1775_103D->field0.field22);
  ltmp_1793_7_2E = 0u;
  goto ltmp_1814_D;

  do {
ltmp_1814_D:
  ltmp_1793_7 = ltmp_1793_7_2E;
  *(&ltmp_1792_13[ltmp_1793_7]) = 0;
  ltmp_1794_7 = ltmp_1793_7 + 1u;
  if ((ltmp_1794_7 == 16u)) {
    goto ltmp_1815_D;
  } else {
    ltmp_1793_7_2E = ltmp_1794_7;
    goto ltmp_1814_D;
  }

  } while (1);
ltmp_1815_D:
  ltmp_1795_13 = (unsigned char *)(&ltmp_1775_103D->field0.field23);
  ltmp_1796_7_2E = 0u;
  goto ltmp_1816_D;

  do {
ltmp_1816_D:
  ltmp_1796_7 = ltmp_1796_7_2E;
  *(&ltmp_1795_13[ltmp_1796_7]) = 0;
  ltmp_1797_7 = ltmp_1796_7 + 1u;
  if ((ltmp_1797_7 == 16u)) {
    goto ltmp_1817_D;
  } else {
    ltmp_1796_7_2E = ltmp_1797_7;
    goto ltmp_1816_D;
  }

  } while (1);
ltmp_1817_D:
  *(&ltmp_1775_103D->field0.field22.field1) = 512u;
  *(&ltmp_1775_103D->field0.field23.field1) = 4096u;
  ltmp_1798_13 = (unsigned char *)(&ltmp_1775_103D->field0.field19);
  ltmp_1799_7_2E = 0u;
  goto ltmp_1818_D;

  do {
ltmp_1818_D:
  ltmp_1799_7 = ltmp_1799_7_2E;
  *(&ltmp_1798_13[ltmp_1799_7]) = 0;
  ltmp_1800_7 = ltmp_1799_7 + 1u;
  if ((ltmp_1800_7 == 32u)) {
    goto ltmp_1819_D;
  } else {
    ltmp_1799_7_2E = ltmp_1800_7;
    goto ltmp_1818_D;
  }

  } while (1);
ltmp_1819_D:
  ltmp_1801_17 = &ltmp_1775_103D->field0.field21;
  *ltmp_1801_17 = 1u;
  ltmp_1802_103E = &ltmp_1775_103D->field0.field25;
  *(&ltmp_1775_103D->field0.field26) = ltmp_1802_103E;
  ltmp_1803_13 = (unsigned char *)ltmp_1802_103E;
  ltmp_1804_7_2E = 0u;
  goto ltmp_1820_D;

  do {
ltmp_1820_D:
  ltmp_1804_7 = ltmp_1804_7_2E;
  *(&ltmp_1803_13[ltmp_1804_7]) = 0;
  ltmp_1805_7 = ltmp_1804_7 + 1u;
  if ((ltmp_1805_7 == 1u)) {
    goto ltmp_1821_D;
  } else {
    ltmp_1804_7_2E = ltmp_1805_7;
    goto ltmp_1820_D;
  }

  } while (1);
ltmp_1821_D:
  *ltmp_1777_102C = (&(ltmp_69_1021[2]));
  *ltmp_1801_17 = 5u;
  *(&ltmp_1775_103D->field1) = 0u;
  *(&ltmp_1775_103D->field2) = 64u;
  *(&ltmp_1775_103D->field3) = 36u;
  *(&ltmp_1775_103D->field4) = 16u;
  *(&ltmp_1775_103D->field5) = 32u;
  *(&ltmp_1775_103D->field6) = 1024u;
  *(&ltmp_1775_103D->field7) = 2048u;
  *(&ltmp_1775_103D->field8) = 16384u;
  *(&ltmp_1775_103D->field9) = 49151u;
  *(&ltmp_1775_103D->field10) = 1064960u;
  *(&ltmp_1775_103D->field11) = 3072u;
  *(&ltmp_1775_103D->field12) = 2u;
  *(&ltmp_1775_103D->field13) = 3u;
  *(&ltmp_1775_103D->field14) = 3u;
  *(&ltmp_1775_103D->field15) = 3u;
  *(&ltmp_1775_103D->field16) = 4u;
  *(&ltmp_1775_103D->field17) = 2u;
  *(&ltmp_1775_103D->field18) = 8u;
  *(&ltmp_1775_103D->field19) = 29u;
  *(&ltmp_1775_103D->field20) = 10u;
  *(&ltmp_1775_103D->field21) = 18u;
  *ltmp_1787_17 = 4u;
  *ltmp_1788_17 = 1064960u;
  return;
}


static plzo_int32_t plzo_lzo1y_99_compress_internal(unsigned char *ltmp_1822_13, plzo_uint32_t ltmp_1823_7, unsigned char *ltmp_1824_13, plzo_uint32_t *ltmp_1825_17, struct plzo_callback_t *ltmp_1826_1026, plzo_int32_t ltmp_1827_8, struct plzo_compress_config_t *ltmp_1828_1027, plzo_uint32_t *ltmp_1829_17) {
  struct l_struct_2E_Lzo1yE ltmp_1830_103F;
  plzo_int32_t  (***ltmp_1831_102C) ();
  struct l_struct_2E_E *ltmp_1832_1023;
  plzo_int32_t ltmp_1833_8;
  plzo_int32_t ltmp_1834_8;
  plzo_int32_t ltmp_1835_8;
  plzo_int32_t ltmp_1835_8_2E;
  plzo_int32_t ltmp_1836_8;
  plzo_int32_t ltmp_1837_8;
  plzo_int32_t ltmp_1837_8_2E;
  plzo_uint32_t ltmp_1838_7;
  plzo_uint32_t ltmp_1839_7;
  plzo_uint32_t ltmp_1840_7;
  plzo_uint32_t ltmp_1841_7;
  plzo_uint32_t ltmp_1842_7;
  plzo_uint32_t ltmp_1843_7;
  plzo_uint32_t ltmp_1844_7;
  plzo_uint32_t ltmp_1845_7;

  ltmp_25_1010((&ltmp_1830_103F.field0.field0));
  ltmp_1831_102C = &ltmp_1830_103F.field0.field0.field0.field0;
  *(&ltmp_1830_103F.field0.field0.field1) = 0u;
  *(&ltmp_1830_103F.field0.field0.field2) = 64u;
  *(&ltmp_1830_103F.field0.field0.field3) = 32u;
  *(&ltmp_1830_103F.field0.field0.field4) = 16u;
  *(&ltmp_1830_103F.field0.field0.field5) = 0u;
  *(&ltmp_1830_103F.field0.field0.field6) = 1024u;
  *(&ltmp_1830_103F.field0.field0.field8) = 16384u;
  *(&ltmp_1830_103F.field0.field0.field9) = 49151u;
  *(&ltmp_1830_103F.field0.field0.field10) = 0u;
  *(&ltmp_1830_103F.field0.field0.field12) = 2u;
  *(&ltmp_1830_103F.field0.field0.field13) = 3u;
  *(&ltmp_1830_103F.field0.field0.field14) = 3u;
  *(&ltmp_1830_103F.field0.field0.field15) = 3u;
  *(&ltmp_1830_103F.field0.field0.field16) = 0u;
  *(&ltmp_1830_103F.field0.field0.field17) = 2u;
  *(&ltmp_1830_103F.field0.field0.field19) = 33u;
  *(&ltmp_1830_103F.field0.field0.field20) = 9u;
  *(&ltmp_1830_103F.field0.field0.field21) = 0u;
  *(&ltmp_1830_103F.field0.field0.field0.field6.field3) = 49151u;
  *ltmp_1831_102C = (&(ltmp_68_1021[2]));
  *(&ltmp_1830_103F.field0.field0.field18) = 14u;
  *(&ltmp_1830_103F.field0.field0.field7) = 1024u;
  *(&ltmp_1830_103F.field0.field0.field11) = 2048u;
  *(&ltmp_1830_103F.field0.field0.field0.field6.field0) = 1;
  ltmp_1832_1023 = &ltmp_1830_103F.field0.field0.field0;
  ltmp_1833_8 = ltmp_2_1002(ltmp_1832_1023, ltmp_1827_8, ltmp_1826_1026);
  if (((ltmp_1833_8 != 0) | (ltmp_1828_1027 == ((struct plzo_compress_config_t *)0)))) {
    ltmp_1835_8_2E = ltmp_1833_8;
    goto ltmp_1846_D;
  } else {
    goto ltmp_1847_D;
  }

ltmp_1847_D:
  ltmp_1834_8 = ltmp_3_1003(ltmp_1832_1023, ltmp_1828_1027);
  ltmp_1835_8_2E = ltmp_1834_8;
  goto ltmp_1846_D;

ltmp_1846_D:
  ltmp_1835_8 = ltmp_1835_8_2E;
  if ((ltmp_1835_8 == 0)) {
    goto ltmp_1848_D;
  } else {
    ltmp_1837_8_2E = ltmp_1835_8;
    goto ltmp_1849_D;
  }

ltmp_1848_D:
  ltmp_1836_8 = ltmp_4_1004(ltmp_1832_1023, ltmp_1822_13, ltmp_1823_7, ltmp_1824_13, ltmp_1825_17);
  ltmp_1837_8_2E = ltmp_1836_8;
  goto ltmp_1849_D;

ltmp_1849_D:
  ltmp_1837_8 = ltmp_1837_8_2E;
  if (((ltmp_1837_8 != 0) | (ltmp_1829_17 == ((plzo_uint32_t *)0)))) {
    goto ltmp_1850_D;
  } else {
    goto ltmp_1851_D;
  }

ltmp_1851_D:
  ltmp_1838_7 = *(&ltmp_1830_103F.field0.field0.field0.field2.field0);
  *ltmp_1829_17 = ltmp_1838_7;
  ltmp_1839_7 = *(&ltmp_1830_103F.field0.field0.field0.field2.field1);
  *(&ltmp_1829_17[1]) = ltmp_1839_7;
  ltmp_1840_7 = *(&ltmp_1830_103F.field0.field0.field0.field2.field2);
  *(&ltmp_1829_17[2]) = ltmp_1840_7;
  ltmp_1841_7 = *(&ltmp_1830_103F.field0.field0.field0.field2.field3);
  *(&ltmp_1829_17[3]) = ltmp_1841_7;
  ltmp_1842_7 = *(&ltmp_1830_103F.field0.field0.field0.field2.field4);
  *(&ltmp_1829_17[4]) = ltmp_1842_7;
  ltmp_1843_7 = *(&ltmp_1830_103F.field0.field0.field0.field2.field5);
  *(&ltmp_1829_17[5]) = ltmp_1843_7;
  ltmp_1844_7 = *(&ltmp_1830_103F.field0.field0.field0.field2.field6);
  *(&ltmp_1829_17[6]) = ltmp_1844_7;
  ltmp_1845_7 = *(&ltmp_1830_103F.field0.field0.field0.field2.field7);
  *(&ltmp_1829_17[7]) = ltmp_1845_7;
  goto ltmp_1850_D;

ltmp_1850_D:
  *ltmp_1831_102C = (&(ltmp_69_1021[2]));
  ltmp_5_1005(ltmp_1832_1023);
  return ltmp_1837_8;
}


static signed char *ltmp_26_1011(struct l_struct_2E_E *ltmp_1852_1023) {

  return ((signed char *)0);
}


static void ltmp_27_1010(struct l_struct_2E_Lzo1E *ltmp_1853_103D) {
  unsigned char **ltmp_1854_1025;
  unsigned char *ltmp_1855_13;
  plzo_uint32_t ltmp_1856_7;
  unsigned char *ltmp_1857_13;
  unsigned char *ltmp_1858_13;

  ltmp_1854_1025 = &ltmp_1853_103D->field0.field1.field6;
  ltmp_1855_13 = *ltmp_1854_1025;
  *ltmp_1854_1025 = (&ltmp_1855_13[1]);
  ltmp_1856_7 = *(&ltmp_1853_103D->field4);
  *ltmp_1855_13 = (((unsigned char )(((unsigned char )ltmp_1856_7) + 1)));
  ltmp_1857_13 = *ltmp_1854_1025;
  *ltmp_1854_1025 = (&ltmp_1857_13[1]);
  *ltmp_1857_13 = 0;
  ltmp_1858_13 = *ltmp_1854_1025;
  *ltmp_1854_1025 = (&ltmp_1858_13[1]);
  *ltmp_1858_13 = 0;
  return;
}


static plzo_int32_t ltmp_28_1012(struct l_struct_2E_Lzo1E *ltmp_1859_103D) {

  return 0;
}


static void ltmp_29_1013(struct l_struct_2E_Lzo1xE *ltmp_1860_103C) {

  return;
}


static void ltmp_30_1014(struct l_struct_2E_Lzo1E *ltmp_1861_103D, struct l_struct_2E_E_3A__3A_A4 *ltmp_1862_103B, plzo_uint32_t ltmp_1863_7, plzo_uint32_t ltmp_1864_7, plzo_uint32_t ltmp_1865_7) {
  plzo_uint32_t ltmp_1866_7;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1867_1031;
  plzo_uint32_t ltmp_1868_7;
  plzo_uint32_t ltmp_1869_7;
  struct l_struct_2E_A7 *ltmp_1870_1038;
  plzo_uint32_t ltmp_1871_7;
  plzo_uint32_t ltmp_1872_7;
  plzo_uint32_t ltmp_1873_7;
  plzo_uint32_t ltmp_1874_7;
  plzo_uint32_t ltmp_1875_7;
  plzo_uint32_t ltmp_1876_7;
  plzo_uint32_t ltmp_1877_7;
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
  plzo_uint32_t ltmp_1890_7_2E;
  plzo_uint32_t ltmp_1891_7;
  plzo_uint32_t ltmp_1891_7_2E;
  plzo_uint32_t ltmp_1892_7;
  plzo_uint32_t ltmp_1892_7_2E;
  plzo_uint32_t ltmp_1893_7;
  plzo_uint32_t ltmp_1893_7_2E;
  plzo_uint32_t ltmp_1894_7;
  plzo_uint32_t ltmp_1895_7;
  plzo_uint32_t ltmp_1895_7_2E;
  struct l_struct_2E_A2_3A__3A_A3 *ltmp_1896_1031;
  plzo_uint32_t ltmp_1897_7;
  plzo_uint32_t ltmp_1898_7;

  if ((ltmp_1863_7 == 0u)) {
    goto ltmp_1899_D;
  } else {
    goto ltmp_1900_D;
  }

ltmp_1899_D:
  ltmp_1866_7 = ltmp_1865_7 + 1u;
  ltmp_1867_1031 = *(&ltmp_1861_103D->field0.field20.field3);
  ltmp_1868_7 = *(&ltmp_1861_103D->field0.field20.field1);
  ltmp_1869_7 = *(&ltmp_1867_1031[((ltmp_1868_7*(ltmp_1864_7 + 1u)) + ((((ltmp_1868_7 > ltmp_1866_7)) ? (ltmp_1866_7) : ((ltmp_1868_7 + 4294967295u)))))].field0);
  *(&ltmp_1862_103B->field0) = (ltmp_1869_7 + ((((ltmp_1865_7 == 3u)) ? (16u) : (8u))));
  return;
ltmp_1900_D:
  ltmp_1870_1038 = *(&ltmp_1861_103D->field0.field19.field0);
  ltmp_1871_7 = *(&ltmp_1870_1038[ltmp_1864_7].field0);
  ltmp_1872_7 = ltmp_1871_7 - ltmp_1863_7;
  ltmp_1873_7 = *(&ltmp_1870_1038[ltmp_1864_7].field1[((((ltmp_1872_7 < 8u)) ? (ltmp_1872_7) : (7u)))]);
  if ((ltmp_1863_7 > 1u)) {
    goto ltmp_1901_D;
  } else {
    goto ltmp_1902_D;
  }

ltmp_1901_D:
  if ((ltmp_1863_7 == 2u)) {
    goto ltmp_1903_D;
  } else {
    goto ltmp_1904_D;
  }

ltmp_1903_D:
  ltmp_1874_7 = *(&ltmp_1861_103D->field6);
  if ((((ltmp_1865_7 == 0u) | (ltmp_1865_7 > 3u)) | (ltmp_1874_7 < ltmp_1873_7))) {
    goto ltmp_1902_D;
  } else {
    goto ltmp_1904_D;
  }

ltmp_1904_D:
  ltmp_1875_7 = *(&ltmp_1861_103D->field16);
  if ((ltmp_1875_7 > ltmp_1863_7)) {
    goto ltmp_1905_D;
  } else {
    goto ltmp_1906_D;
  }

ltmp_1905_D:
  ltmp_1876_7 = *(&ltmp_1861_103D->field9);
  if ((ltmp_1876_7 < ltmp_1873_7)) {
    goto ltmp_1902_D;
  } else {
    goto ltmp_1906_D;
  }

ltmp_1906_D:
  if ((ltmp_1863_7 == 2u)) {
    ltmp_1890_7_2E = 16u;
    goto ltmp_1907_D;
  } else {
    goto ltmp_1908_D;
  }

ltmp_1908_D:
  ltmp_1877_7 = *(&ltmp_1861_103D->field18);
  if ((ltmp_1877_7 < ltmp_1863_7)) {
    goto ltmp_1909_D;
  } else {
    goto ltmp_1910_D;
  }

ltmp_1910_D:
  ltmp_1878_7 = *(&ltmp_1861_103D->field7);
  if ((ltmp_1878_7 < ltmp_1873_7)) {
    goto ltmp_1909_D;
  } else {
    ltmp_1890_7_2E = 16u;
    goto ltmp_1907_D;
  }

ltmp_1909_D:
  ltmp_1879_7 = *(&ltmp_1861_103D->field13);
  if ((ltmp_1879_7 == ltmp_1863_7)) {
    goto ltmp_1911_D;
  } else {
    goto ltmp_1912_D;
  }

ltmp_1911_D:
  ltmp_1880_7 = *(&ltmp_1861_103D->field11);
  if (((ltmp_1880_7 >= ltmp_1873_7) & (ltmp_1865_7 > 3u))) {
    ltmp_1890_7_2E = 16u;
    goto ltmp_1907_D;
  } else {
    goto ltmp_1912_D;
  }

ltmp_1912_D:
  ltmp_1881_7 = *(&ltmp_1861_103D->field8);
  if ((ltmp_1881_7 < ltmp_1873_7)) {
    goto ltmp_1913_D;
  } else {
    goto ltmp_1914_D;
  }

ltmp_1914_D:
  ltmp_1882_7 = *(&ltmp_1861_103D->field19);
  if ((ltmp_1882_7 < ltmp_1863_7)) {
    goto ltmp_1915_D;
  } else {
    ltmp_1890_7_2E = 24u;
    goto ltmp_1907_D;
  }

ltmp_1915_D:
  ltmp_1883_7 = ltmp_1863_7 - ltmp_1882_7;
  if ((ltmp_1883_7 > 255u)) {
    ltmp_1891_7_2E = ltmp_1883_7;
    ltmp_1892_7_2E = 32u;
    ltmp_1893_7_2E = 0u;
    goto ltmp_1916_D;
  } else {
    ltmp_1895_7_2E = 32u;
    goto ltmp_1917_D;
  }

ltmp_1913_D:
  ltmp_1884_7 = *(&ltmp_1861_103D->field20);
  if ((ltmp_1884_7 < ltmp_1863_7)) {
    goto ltmp_1918_D;
  } else {
    goto ltmp_1919_D;
  }

ltmp_1919_D:
  ltmp_1885_7 = *(&ltmp_1861_103D->field9);
  if ((ltmp_1885_7 < ltmp_1873_7)) {
    goto ltmp_1918_D;
  } else {
    goto ltmp_1920_D;
  }

ltmp_1918_D:
  ltmp_1886_7 = *(&ltmp_1861_103D->field10);
  if ((ltmp_1886_7 == 0u)) {
    goto ltmp_1920_D;
  } else {
    goto ltmp_1921_D;
  }

ltmp_1920_D:
  if ((ltmp_1884_7 < ltmp_1863_7)) {
    goto ltmp_1922_D;
  } else {
    ltmp_1890_7_2E = 24u;
    goto ltmp_1907_D;
  }

ltmp_1922_D:
  ltmp_1887_7 = ltmp_1863_7 - ltmp_1884_7;
  if ((ltmp_1887_7 > 255u)) {
    ltmp_1891_7_2E = ltmp_1887_7;
    ltmp_1892_7_2E = 32u;
    ltmp_1893_7_2E = 0u;
    goto ltmp_1916_D;
  } else {
    ltmp_1895_7_2E = 32u;
    goto ltmp_1917_D;
  }

ltmp_1921_D:
  ltmp_1888_7 = *(&ltmp_1861_103D->field21);
  if ((ltmp_1888_7 < ltmp_1863_7)) {
    goto ltmp_1923_D;
  } else {
    ltmp_1890_7_2E = 32u;
    goto ltmp_1907_D;
  }

ltmp_1923_D:
  ltmp_1889_7 = ltmp_1863_7 - ltmp_1888_7;
  if ((ltmp_1889_7 > 255u)) {
    ltmp_1891_7_2E = ltmp_1889_7;
    ltmp_1892_7_2E = 40u;
    ltmp_1893_7_2E = 0u;
    goto ltmp_1916_D;
  } else {
    ltmp_1895_7_2E = 40u;
    goto ltmp_1917_D;
  }

ltmp_1907_D:
  ltmp_1890_7 = ltmp_1890_7_2E;
  if ((ltmp_1863_7 > 255u)) {
    ltmp_1891_7_2E = ltmp_1863_7;
    ltmp_1892_7_2E = ltmp_1890_7;
    ltmp_1893_7_2E = 0u;
    goto ltmp_1916_D;
  } else {
    ltmp_1895_7_2E = ltmp_1890_7;
    goto ltmp_1917_D;
  }

  do {
ltmp_1916_D:
  ltmp_1891_7 = ltmp_1891_7_2E;
  ltmp_1892_7 = ltmp_1892_7_2E;
  ltmp_1893_7 = ltmp_1893_7_2E;
  if (((((ltmp_1893_7*4294967041u) + ltmp_1891_7) + 4294967041u) > 255u)) {
    ltmp_1891_7_2E = ltmp_1891_7;
    ltmp_1892_7_2E = ltmp_1892_7;
    ltmp_1893_7_2E = (ltmp_1893_7 + 1u);
    goto ltmp_1916_D;
  } else {
    goto ltmp_1924_D;
  }

  } while (1);
ltmp_1924_D:
  ltmp_1894_7 = ((ltmp_1893_7 << 3) + ltmp_1892_7) + 8u;
  ltmp_1895_7_2E = ltmp_1894_7;
  goto ltmp_1917_D;

ltmp_1917_D:
  ltmp_1895_7 = ltmp_1895_7_2E;
  ltmp_1896_1031 = *(&ltmp_1861_103D->field0.field20.field3);
  ltmp_1897_7 = *(&ltmp_1861_103D->field0.field20.field1);
  ltmp_1898_7 = *(&ltmp_1896_1031[(ltmp_1897_7*(ltmp_1864_7 + ltmp_1863_7))].field0);
  *(&ltmp_1862_103B->field0) = (ltmp_1898_7 + ltmp_1895_7);
  return;
ltmp_1902_D:
  *(&ltmp_1862_103B->field0) = 4294967295u;
  return;
}


static plzo_uint32_t ltmp_31_1015(struct l_struct_2E_Lzo1E *ltmp_1925_103D) {

  return 0u;
}


static plzo_int32_t ltmp_32_1016(struct l_struct_2E_E *ltmp_1926_1023, struct l_struct_2E_A7 *ltmp_1927_1038) {
  plzo_uint32_t ltmp_1928_7;

  ltmp_1928_7 = *(&ltmp_1927_1038->field0);
  return ((plzo_int32_t )(ltmp_1928_7 == 0u));
}


static plzo_int32_t ltmp_33_1016(struct l_struct_2E_E *ltmp_1929_1023, struct l_struct_2E_A7 *ltmp_1930_1038) {
  plzo_uint32_t ltmp_1931_7;

  ltmp_1931_7 = *(&ltmp_1930_1038->field0);
  return ((plzo_int32_t )(ltmp_1931_7 < 3u));
}


static void ltmp_34_100F(struct l_struct_2E_Lzo1E *ltmp_1932_103D, plzo_uint32_t ltmp_1933_7, plzo_uint32_t ltmp_1934_7) {
  unsigned char **ltmp_1935_1025;
  unsigned char *ltmp_1936_13;
  plzo_uint32_t ltmp_1937_7;
  plzo_uint32_t ltmp_1938_7;
  unsigned char *ltmp_1939_13;
  plzo_uint32_t ltmp_1940_7;
  plzo_uint32_t ltmp_1941_7;
  plzo_uint32_t ltmp_1942_7;
  unsigned char *ltmp_1943_13;
  plzo_uint32_t ltmp_1944_7;
  plzo_uint32_t ltmp_1945_7;
  plzo_uint32_t ltmp_1946_7;
  plzo_uint32_t ltmp_1947_7;
  plzo_uint32_t ltmp_1948_7;
  plzo_uint32_t ltmp_1949_7;
  unsigned char *ltmp_1950_13;
  plzo_uint32_t ltmp_1951_7;
  plzo_uint32_t ltmp_1952_7;
  plzo_uint32_t ltmp_1953_7;
  unsigned char *ltmp_1954_13;
  plzo_uint32_t ltmp_1955_7;
  plzo_uint32_t ltmp_1956_7;
  unsigned char *ltmp_1957_13;
  plzo_uint32_t ltmp_1958_7;
  plzo_uint32_t ltmp_1959_7;
  plzo_uint32_t ltmp_1959_7_2E;
  plzo_uint32_t ltmp_1960_7;
  plzo_uint32_t ltmp_1961_7;
  unsigned char *ltmp_1962_13;
  unsigned char *ltmp_1963_13;
  unsigned char *ltmp_1963_13_2E;
  plzo_uint32_t ltmp_1964_7;
  plzo_uint32_t ltmp_1964_7_2E;
  unsigned char *ltmp_1965_13;
  unsigned char *ltmp_1966_13;
  unsigned char *ltmp_1966_13_2E;
  unsigned char *ltmp_1967_13;
  plzo_uint32_t ltmp_1968_7;
  plzo_uint32_t ltmp_1969_7;
  plzo_uint32_t ltmp_1970_7;
  plzo_uint32_t ltmp_1971_7;
  unsigned char *ltmp_1972_13;
  plzo_uint32_t ltmp_1973_7;
  unsigned char *ltmp_1974_13;
  plzo_uint32_t ltmp_1975_7;
  unsigned char ltmp_1976_3;
  unsigned char *ltmp_1977_13;
  unsigned char ltmp_1978_3;
  plzo_uint32_t ltmp_1979_7;
  unsigned char ltmp_1980_3;
  plzo_uint32_t ltmp_1981_7;
  plzo_uint32_t ltmp_1982_7;
  plzo_uint32_t ltmp_1982_7_2E;
  plzo_uint32_t ltmp_1983_7;
  unsigned char *ltmp_1984_13;
  unsigned char *ltmp_1985_13;
  unsigned char *ltmp_1985_13_2E;
  plzo_uint32_t ltmp_1986_7;
  plzo_uint32_t ltmp_1986_7_2E;
  unsigned char *ltmp_1987_13;
  unsigned char *ltmp_1988_13;
  unsigned char *ltmp_1988_13_2E;
  unsigned char *ltmp_1989_13;
  unsigned char *ltmp_1990_13;
  unsigned char *ltmp_1990_13_2E;

  ltmp_0_1000((&ltmp_1932_103D->field0), ltmp_1933_7, ltmp_1934_7);
  ltmp_1935_1025 = &ltmp_1932_103D->field0.field1.field6;
  ltmp_1936_13 = *ltmp_1935_1025;
  if ((ltmp_1933_7 == 2u)) {
    goto ltmp_1991_D;
  } else {
    goto ltmp_1992_D;
  }

ltmp_1991_D:
  ltmp_1937_7 = ltmp_1934_7 + 4294967295u;
  ltmp_1938_7 = *(&ltmp_1932_103D->field1);
  *ltmp_1936_13 = (((unsigned char )(((unsigned char )ltmp_1938_7) | ((unsigned char )(ltmp_1937_7 >> 6)))));
  ltmp_1939_13 = &ltmp_1936_13[2];
  *(&ltmp_1936_13[1]) = (((unsigned char )(((unsigned char )ltmp_1937_7) << 2)));
  ltmp_1990_13_2E = ltmp_1939_13;
  goto ltmp_1993_D;

ltmp_1992_D:
  ltmp_1940_7 = *(&ltmp_1932_103D->field18);
  if ((ltmp_1940_7 < ltmp_1933_7)) {
    goto ltmp_1994_D;
  } else {
    goto ltmp_1995_D;
  }

ltmp_1995_D:
  ltmp_1941_7 = *(&ltmp_1932_103D->field7);
  if ((ltmp_1941_7 < ltmp_1934_7)) {
    goto ltmp_1994_D;
  } else {
    goto ltmp_1996_D;
  }

ltmp_1996_D:
  ltmp_1942_7 = ltmp_1934_7 + 4294967295u;
  *ltmp_1936_13 = (((unsigned char )(((unsigned char )(ltmp_1942_7 >> 6)) | (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1933_7) << 5))) + 224))))));
  ltmp_1943_13 = &ltmp_1936_13[2];
  *(&ltmp_1936_13[1]) = (((unsigned char )(((unsigned char )ltmp_1942_7) << 2)));
  ltmp_1990_13_2E = ltmp_1943_13;
  goto ltmp_1993_D;

ltmp_1994_D:
  ltmp_1944_7 = *(&ltmp_1932_103D->field13);
  if ((ltmp_1944_7 == ltmp_1933_7)) {
    goto ltmp_1997_D;
  } else {
    goto ltmp_1998_D;
  }

ltmp_1997_D:
  ltmp_1945_7 = *(&ltmp_1932_103D->field11);
  if ((ltmp_1945_7 < ltmp_1934_7)) {
    goto ltmp_1998_D;
  } else {
    goto ltmp_1999_D;
  }

ltmp_1999_D:
  ltmp_1946_7 = *(&ltmp_1932_103D->field0.field17);
  if ((ltmp_1946_7 > 3u)) {
    goto ltmp_2000_D;
  } else {
    goto ltmp_1998_D;
  }

ltmp_2000_D:
  ltmp_1947_7 = *(&ltmp_1932_103D->field7);
  ltmp_1948_7 = (ltmp_1934_7 - ltmp_1947_7) + 4294967295u;
  ltmp_1949_7 = *(&ltmp_1932_103D->field1);
  *ltmp_1936_13 = (((unsigned char )(((unsigned char )(ltmp_1948_7 >> 6)) | ((unsigned char )ltmp_1949_7))));
  ltmp_1950_13 = &ltmp_1936_13[2];
  *(&ltmp_1936_13[1]) = (((unsigned char )(((unsigned char )ltmp_1948_7) << 2)));
  ltmp_1990_13_2E = ltmp_1950_13;
  goto ltmp_1993_D;

ltmp_1998_D:
  ltmp_1951_7 = *(&ltmp_1932_103D->field8);
  if ((ltmp_1951_7 < ltmp_1934_7)) {
    goto ltmp_2001_D;
  } else {
    goto ltmp_2002_D;
  }

ltmp_2002_D:
  ltmp_1952_7 = ltmp_1934_7 + 4294967295u;
  ltmp_1953_7 = *(&ltmp_1932_103D->field19);
  if ((ltmp_1953_7 < ltmp_1933_7)) {
    goto ltmp_2003_D;
  } else {
    goto ltmp_2004_D;
  }

ltmp_2004_D:
  ltmp_1954_13 = &ltmp_1936_13[1];
  ltmp_1955_7 = *(&ltmp_1932_103D->field3);
  *ltmp_1936_13 = (((unsigned char )((((unsigned char )(((unsigned char )ltmp_1933_7) + 254))) + ((unsigned char )ltmp_1955_7))));
  ltmp_1966_13_2E = ltmp_1954_13;
  goto ltmp_2005_D;

ltmp_2003_D:
  ltmp_1956_7 = ltmp_1933_7 - ltmp_1953_7;
  ltmp_1957_13 = &ltmp_1936_13[1];
  ltmp_1958_7 = *(&ltmp_1932_103D->field3);
  *ltmp_1936_13 = ((unsigned char )ltmp_1958_7);
  if ((ltmp_1956_7 > 255u)) {
    ltmp_1959_7_2E = 0u;
    goto ltmp_2006_D;
  } else {
    ltmp_1963_13_2E = ltmp_1957_13;
    ltmp_1964_7_2E = ltmp_1956_7;
    goto ltmp_2007_D;
  }

  do {
ltmp_2006_D:
  ltmp_1959_7 = ltmp_1959_7_2E;
  ltmp_1960_7 = ltmp_1959_7 + 1u;
  ltmp_1961_7 = ((ltmp_1959_7*4294967041u) + ltmp_1956_7) + 4294967041u;
  *(&ltmp_1936_13[ltmp_1960_7]) = 0;
  if ((ltmp_1961_7 > 255u)) {
    ltmp_1959_7_2E = ltmp_1960_7;
    goto ltmp_2006_D;
  } else {
    goto ltmp_2008_D;
  }

  } while (1);
ltmp_2008_D:
  ltmp_1962_13 = &ltmp_1936_13[(((plzo_int32_t )ltmp_1959_7) + 2)];
  ltmp_1963_13_2E = ltmp_1962_13;
  ltmp_1964_7_2E = ltmp_1961_7;
  goto ltmp_2007_D;

ltmp_2007_D:
  ltmp_1963_13 = ltmp_1963_13_2E;
  ltmp_1964_7 = ltmp_1964_7_2E;
  ltmp_1965_13 = &ltmp_1963_13[1];
  *ltmp_1963_13 = ((unsigned char )ltmp_1964_7);
  ltmp_1966_13_2E = ltmp_1965_13;
  goto ltmp_2005_D;

ltmp_2005_D:
  ltmp_1966_13 = ltmp_1966_13_2E;
  *ltmp_1966_13 = ((unsigned char )(ltmp_1952_7 >> 6));
  ltmp_1967_13 = &ltmp_1966_13[2];
  *(&ltmp_1966_13[1]) = (((unsigned char )(((unsigned char )ltmp_1952_7) << 2)));
  ltmp_1990_13_2E = ltmp_1967_13;
  goto ltmp_1993_D;

ltmp_2001_D:
  ltmp_1968_7 = *(&ltmp_1932_103D->field20);
  if ((ltmp_1968_7 < ltmp_1933_7)) {
    goto ltmp_2009_D;
  } else {
    goto ltmp_2010_D;
  }

ltmp_2010_D:
  ltmp_1969_7 = *(&ltmp_1932_103D->field9);
  if ((ltmp_1969_7 < ltmp_1934_7)) {
    goto ltmp_2009_D;
  } else {
    goto ltmp_2011_D;
  }

ltmp_2011_D:
  ltmp_1970_7 = ltmp_1934_7 - ltmp_1951_7;
  ltmp_1971_7 = *(&ltmp_1932_103D->field4);
  *ltmp_1936_13 = (((unsigned char )((((unsigned char )((((unsigned char )(((unsigned char )(ltmp_1970_7 >> 11)) & 8))) | (((unsigned char )(((unsigned char )ltmp_1933_7) + 253)))))) + ((unsigned char )ltmp_1971_7))));
  *(&ltmp_1936_13[1]) = ((unsigned char )(ltmp_1970_7 >> 6));
  ltmp_1972_13 = &ltmp_1936_13[3];
  *(&ltmp_1936_13[2]) = (((unsigned char )(((unsigned char )ltmp_1970_7) << 2)));
  ltmp_1990_13_2E = ltmp_1972_13;
  goto ltmp_1993_D;

ltmp_2009_D:
  ltmp_1973_7 = (ltmp_1934_7 - ltmp_1951_7) + 4294967295u;
  ltmp_1974_13 = &ltmp_1936_13[1];
  ltmp_1975_7 = *(&ltmp_1932_103D->field5);
  ltmp_1976_3 = (unsigned char )ltmp_1975_7;
  *ltmp_1936_13 = ltmp_1976_3;
  ltmp_1977_13 = &ltmp_1936_13[2];
  ltmp_1978_3 = (unsigned char )(ltmp_1973_7 >> 14);
  *ltmp_1974_13 = ltmp_1978_3;
  ltmp_1979_7 = *(&ltmp_1932_103D->field21);
  if ((ltmp_1979_7 < ltmp_1933_7)) {
    goto ltmp_2012_D;
  } else {
    goto ltmp_2013_D;
  }

ltmp_2013_D:
  ltmp_1980_3 = ((unsigned char )(((unsigned char )ltmp_1933_7) + 253));
  *ltmp_1936_13 = (((unsigned char )(ltmp_1976_3 | (((unsigned char )(ltmp_1980_3 & 3))))));
  *ltmp_1974_13 = (((unsigned char )(ltmp_1978_3 | (((unsigned char )((((unsigned char )(ltmp_1980_3 << 4))) & 192))))));
  ltmp_1988_13_2E = ltmp_1977_13;
  goto ltmp_2014_D;

ltmp_2012_D:
  ltmp_1981_7 = ltmp_1933_7 - ltmp_1979_7;
  if ((ltmp_1981_7 > 255u)) {
    ltmp_1982_7_2E = 0u;
    goto ltmp_2015_D;
  } else {
    ltmp_1985_13_2E = ltmp_1977_13;
    ltmp_1986_7_2E = ltmp_1981_7;
    goto ltmp_2016_D;
  }

  do {
ltmp_2015_D:
  ltmp_1982_7 = ltmp_1982_7_2E;
  ltmp_1983_7 = ((ltmp_1982_7*4294967041u) + ltmp_1981_7) + 4294967041u;
  *(&ltmp_1936_13[(ltmp_1982_7 + 2u)]) = 0;
  if ((ltmp_1983_7 > 255u)) {
    ltmp_1982_7_2E = (ltmp_1982_7 + 1u);
    goto ltmp_2015_D;
  } else {
    goto ltmp_2017_D;
  }

  } while (1);
ltmp_2017_D:
  ltmp_1984_13 = &ltmp_1936_13[(((plzo_int32_t )ltmp_1982_7) + 3)];
  ltmp_1985_13_2E = ltmp_1984_13;
  ltmp_1986_7_2E = ltmp_1983_7;
  goto ltmp_2016_D;

ltmp_2016_D:
  ltmp_1985_13 = ltmp_1985_13_2E;
  ltmp_1986_7 = ltmp_1986_7_2E;
  ltmp_1987_13 = &ltmp_1985_13[1];
  *ltmp_1985_13 = ((unsigned char )ltmp_1986_7);
  ltmp_1988_13_2E = ltmp_1987_13;
  goto ltmp_2014_D;

ltmp_2014_D:
  ltmp_1988_13 = ltmp_1988_13_2E;
  *ltmp_1988_13 = ((unsigned char )(ltmp_1973_7 >> 6));
  ltmp_1989_13 = &ltmp_1988_13[2];
  *(&ltmp_1988_13[1]) = (((unsigned char )(((unsigned char )ltmp_1973_7) << 2)));
  ltmp_1990_13_2E = ltmp_1989_13;
  goto ltmp_1993_D;

ltmp_1993_D:
  ltmp_1990_13 = ltmp_1990_13_2E;
  *(&ltmp_1932_103D->field0.field15) = ltmp_1933_7;
  *(&ltmp_1932_103D->field0.field16) = ltmp_1934_7;
  *ltmp_1935_1025 = ltmp_1990_13;
  return;
}


static void ltmp_35_1017(struct l_struct_2E_Lzo1E *ltmp_2018_103D, unsigned char *ltmp_2019_13, plzo_uint32_t ltmp_2020_7, plzo_uint32_t ltmp_2021_7) {
  plzo_uint32_t *ltmp_2022_17;
  plzo_uint32_t ltmp_2023_7;
  plzo_uint32_t *ltmp_2024_17;
  plzo_uint32_t ltmp_2025_7;
  plzo_uint32_t ltmp_2026_7;
  plzo_uint32_t *ltmp_2027_17;
  unsigned char **ltmp_2028_1025;
  unsigned char *ltmp_2029_13;
  unsigned char *ltmp_2030_13;
  unsigned char ltmp_2031_3;
  unsigned char *ltmp_2032_13;
  plzo_uint32_t ltmp_2033_7;
  unsigned char *ltmp_2034_13;
  plzo_uint32_t ltmp_2035_7;
  plzo_uint32_t ltmp_2035_7_2E;
  plzo_uint32_t ltmp_2036_7;
  plzo_uint32_t ltmp_2037_7;
  unsigned char *ltmp_2038_13;
  plzo_uint32_t ltmp_2039_7;
  plzo_uint32_t ltmp_2039_7_2E;
  unsigned char *ltmp_2040_13;
  unsigned char *ltmp_2040_13_2E;
  unsigned char *ltmp_2041_13;
  unsigned char *ltmp_2042_13;
  unsigned char *ltmp_2042_13_2E;
  plzo_uint32_t ltmp_2043_7;
  plzo_uint32_t ltmp_2043_7_2E;
  unsigned char ltmp_2044_3;
  plzo_uint32_t ltmp_2045_7;

  if ((ltmp_2020_7 == 0u)) {
    goto ltmp_2046_D;
  } else {
    goto ltmp_2047_D;
  }

ltmp_2047_D:
  ltmp_2022_17 = &ltmp_2018_103D->field0.field2.field5;
  ltmp_2023_7 = *ltmp_2022_17;
  ltmp_2024_17 = &ltmp_2018_103D->field0.field2.field4;
  if ((ltmp_2023_7 < ltmp_2020_7)) {
    goto ltmp_2048_D;
  } else {
    goto ltmp_2049_D;
  }

ltmp_2048_D:
  *ltmp_2022_17 = ltmp_2020_7;
  ltmp_2025_7 = *ltmp_2024_17;
  if ((ltmp_2025_7 > ltmp_2020_7)) {
    goto ltmp_2050_D;
  } else {
    goto ltmp_2046_D;
  }

ltmp_2049_D:
  ltmp_2026_7 = *ltmp_2024_17;
  if ((ltmp_2026_7 > ltmp_2020_7)) {
    goto ltmp_2050_D;
  } else {
    goto ltmp_2046_D;
  }

ltmp_2050_D:
  *ltmp_2024_17 = ltmp_2020_7;
  goto ltmp_2046_D;

ltmp_2046_D:
  ltmp_2027_17 = &ltmp_2018_103D->field0.field18;
  if ((ltmp_2020_7 == 0u)) {
    goto ltmp_2051_D;
  } else {
    goto ltmp_2052_D;
  }

ltmp_2051_D:
  *ltmp_2027_17 = 0u;
  *(&ltmp_2018_103D->field0.field17) = 0u;
  return;
ltmp_2052_D:
  *ltmp_2027_17 = ltmp_2021_7;
  *(&ltmp_2018_103D->field0.field17) = ltmp_2020_7;
  ltmp_2028_1025 = &ltmp_2018_103D->field0.field1.field6;
  ltmp_2029_13 = *ltmp_2028_1025;
  if ((ltmp_2020_7 > 3u)) {
    goto ltmp_2053_D;
  } else {
    goto ltmp_2054_D;
  }

ltmp_2054_D:
  ltmp_2030_13 = &ltmp_2029_13[-1];
  ltmp_2031_3 = *ltmp_2030_13;
  *ltmp_2030_13 = (((unsigned char )(ltmp_2031_3 | ((unsigned char )ltmp_2020_7))));
  ltmp_2042_13_2E = ltmp_2029_13;
  ltmp_2043_7_2E = 0u;
  goto ltmp_2055_D;

ltmp_2053_D:
  if ((ltmp_2020_7 > 18u)) {
    goto ltmp_2056_D;
  } else {
    goto ltmp_2057_D;
  }

ltmp_2057_D:
  ltmp_2032_13 = &ltmp_2029_13[1];
  *ltmp_2029_13 = (((unsigned char )(((unsigned char )ltmp_2020_7) + 253)));
  ltmp_2042_13_2E = ltmp_2032_13;
  ltmp_2043_7_2E = 0u;
  goto ltmp_2055_D;

ltmp_2056_D:
  ltmp_2033_7 = ltmp_2020_7 + 4294967278u;
  ltmp_2034_13 = &ltmp_2029_13[1];
  *ltmp_2029_13 = 0;
  if ((ltmp_2033_7 > 255u)) {
    ltmp_2035_7_2E = 0u;
    goto ltmp_2058_D;
  } else {
    ltmp_2039_7_2E = ltmp_2033_7;
    ltmp_2040_13_2E = ltmp_2034_13;
    goto ltmp_2059_D;
  }

  do {
ltmp_2058_D:
  ltmp_2035_7 = ltmp_2035_7_2E;
  ltmp_2036_7 = ltmp_2035_7 + 1u;
  ltmp_2037_7 = ((ltmp_2035_7*4294967041u) + ltmp_2033_7) + 4294967041u;
  *(&ltmp_2029_13[ltmp_2036_7]) = 0;
  if ((ltmp_2037_7 > 255u)) {
    ltmp_2035_7_2E = ltmp_2036_7;
    goto ltmp_2058_D;
  } else {
    goto ltmp_2060_D;
  }

  } while (1);
ltmp_2060_D:
  ltmp_2038_13 = &ltmp_2029_13[(((plzo_int32_t )ltmp_2035_7) + 2)];
  ltmp_2039_7_2E = ltmp_2037_7;
  ltmp_2040_13_2E = ltmp_2038_13;
  goto ltmp_2059_D;

ltmp_2059_D:
  ltmp_2039_7 = ltmp_2039_7_2E;
  ltmp_2040_13 = ltmp_2040_13_2E;
  ltmp_2041_13 = &ltmp_2040_13[1];
  *ltmp_2040_13 = ((unsigned char )ltmp_2039_7);
  ltmp_2042_13_2E = ltmp_2041_13;
  ltmp_2043_7_2E = 0u;
  goto ltmp_2055_D;

  do {
ltmp_2055_D:
  ltmp_2042_13 = ltmp_2042_13_2E;
  ltmp_2043_7 = ltmp_2043_7_2E;
  ltmp_2044_3 = *(&ltmp_2019_13[ltmp_2043_7]);
  *(&ltmp_2042_13[ltmp_2043_7]) = ltmp_2044_3;
  ltmp_2045_7 = ltmp_2043_7 + 1u;
  if ((ltmp_2045_7 == ltmp_2020_7)) {
    goto ltmp_2061_D;
  } else {
    ltmp_2042_13_2E = ltmp_2042_13;
    ltmp_2043_7_2E = ltmp_2045_7;
    goto ltmp_2055_D;
  }

  } while (1);
ltmp_2061_D:
  *ltmp_2028_1025 = (&ltmp_2042_13[(((plzo_int32_t )ltmp_2043_7) + 1)]);
  return;
}


static void ltmp_36_1010(struct l_struct_2E_Lzo1E *ltmp_2062_103D) {
  unsigned char **ltmp_2063_1025;
  unsigned char *ltmp_2064_13;

  ltmp_2063_1025 = &ltmp_2062_103D->field0.field1.field6;
  ltmp_2064_13 = *ltmp_2063_1025;
  *ltmp_2063_1025 = (&ltmp_2064_13[1]);
  *ltmp_2064_13 = 0;
  return;
}


static plzo_uint32_t ltmp_37_1015(struct l_struct_2E_Lzo1E *ltmp_2065_103D) {

  return 0u;
}


static plzo_int32_t ltmp_38_1016(struct l_struct_2E_E *ltmp_2066_1023, struct l_struct_2E_A7 *ltmp_2067_1038) {
  plzo_uint32_t ltmp_2068_7;

  ltmp_2068_7 = *(&ltmp_2067_1038->field0);
  return ((plzo_int32_t )(ltmp_2068_7 == 0u));
}


static plzo_int32_t ltmp_39_1016(struct l_struct_2E_E *ltmp_2069_1023, struct l_struct_2E_A7 *ltmp_2070_1038) {
  plzo_uint32_t ltmp_2071_7;

  ltmp_2071_7 = *(&ltmp_2070_1038->field0);
  return ((plzo_int32_t )(ltmp_2071_7 < 3u));
}


static void ltmp_40_1001(struct l_struct_2E_A5 *ltmp_2072_1024, plzo_uint32_t ltmp_2073_7) {
  plzo_uint32_t ltmp_2074_7;
  plzo_uint32_t ltmp_2075_7;
  plzo_uint32_t ltmp_2075_7_2E;
  plzo_uint32_t ltmp_2076_7;
  plzo_uint32_t ltmp_2077_7;
  plzo_uint32_t ltmp_2077_7_2E;
  plzo_uint32_t ltmp_2078_7;
  plzo_uint32_t ltmp_2079_7;
  plzo_uint32_t ltmp_2079_7_2E;
  plzo_uint32_t ltmp_2080_7;
  plzo_uint32_t ltmp_2081_7;
  plzo_uint32_t ltmp_2081_7_2E;

  if ((ltmp_2073_7 > 1u)) {
    goto ltmp_2082_D;
  } else {
    ltmp_2081_7_2E = ltmp_2073_7;
    goto ltmp_2083_D;
  }

ltmp_2082_D:
  ltmp_2074_7 = ltmp_2073_7 + 2u;
  if ((ltmp_2074_7 < 8u)) {
    ltmp_2077_7_2E = 4u;
    goto ltmp_2084_D;
  } else {
    ltmp_2075_7_2E = 8u;
    goto ltmp_2085_D;
  }

  do {
ltmp_2085_D:
  ltmp_2075_7 = ltmp_2075_7_2E;
  ltmp_2076_7 = ltmp_2075_7 << 1;
  if ((ltmp_2074_7 < ltmp_2076_7)) {
    ltmp_2077_7_2E = ltmp_2075_7;
    goto ltmp_2084_D;
  } else {
    ltmp_2075_7_2E = ltmp_2076_7;
    goto ltmp_2085_D;
  }

  } while (1);
ltmp_2084_D:
  ltmp_2077_7 = ltmp_2077_7_2E;
  ltmp_2078_7 = ltmp_2077_7 & 2147483647u;
  ltmp_2079_7_2E = ltmp_2078_7;
  goto ltmp_2086_D;

  do {
ltmp_2086_D:
  ltmp_2079_7 = ltmp_2079_7_2E;
  ltmp_2080_7 = ltmp_2079_7 >> 1;
  ltmp_64_1001(ltmp_2072_1024, ((plzo_uint32_t )((ltmp_2080_7 & ltmp_2074_7) != 0u)));
  ltmp_64_1001(ltmp_2072_1024, 0u);
  if ((ltmp_2080_7 > 2u)) {
    ltmp_2079_7_2E = ltmp_2080_7;
    goto ltmp_2086_D;
  } else {
    ltmp_2081_7_2E = ltmp_2074_7;
    goto ltmp_2083_D;
  }

  } while (1);
ltmp_2083_D:
  ltmp_2081_7 = ltmp_2081_7_2E;
  ltmp_64_1001(ltmp_2072_1024, (ltmp_2081_7 & 1u));
  ltmp_64_1001(ltmp_2072_1024, 1u);
  return;
}


static void ltmp_41_1001(struct l_struct_2E_A5 *ltmp_2087_1024, plzo_uint32_t ltmp_2088_7) {
  plzo_uint32_t ltmp_2089_7;
  plzo_uint32_t ltmp_2090_7;
  plzo_uint32_t ltmp_2091_7;
  plzo_uint32_t ltmp_2092_7;
  plzo_uint32_t ltmp_2092_7_2E;
  plzo_uint32_t ltmp_2093_7;
  plzo_uint32_t ltmp_2094_7;
  plzo_uint32_t ltmp_2094_7_2E;
  plzo_uint32_t ltmp_2095_7;
  plzo_uint32_t ltmp_2096_7;
  plzo_uint32_t ltmp_2096_7_2E;
  plzo_uint32_t ltmp_2097_7;
  plzo_uint32_t ltmp_2098_7;
  plzo_uint32_t ltmp_2098_7_2E;

  if ((ltmp_2088_7 > 2u)) {
    goto ltmp_2099_D;
  } else {
    goto ltmp_2100_D;
  }

ltmp_2100_D:
  ltmp_2089_7 = ltmp_2088_7 + 1u;
  ltmp_64_1001(ltmp_2087_1024, ((plzo_uint32_t )(ltmp_2089_7 > 1u)));
  ltmp_64_1001(ltmp_2087_1024, (ltmp_2089_7 & 1u));
  return;
ltmp_2099_D:
  ltmp_2090_7 = ltmp_2088_7 + 4294967293u;
  ltmp_64_1001(ltmp_2087_1024, 0u);
  ltmp_64_1001(ltmp_2087_1024, 0u);
  if ((ltmp_2090_7 > 1u)) {
    goto ltmp_2101_D;
  } else {
    ltmp_2098_7_2E = ltmp_2090_7;
    goto ltmp_2102_D;
  }

ltmp_2101_D:
  ltmp_2091_7 = ltmp_2088_7 + 4294967295u;
  if ((ltmp_2091_7 < 8u)) {
    ltmp_2094_7_2E = 4u;
    goto ltmp_2103_D;
  } else {
    ltmp_2092_7_2E = 8u;
    goto ltmp_2104_D;
  }

  do {
ltmp_2104_D:
  ltmp_2092_7 = ltmp_2092_7_2E;
  ltmp_2093_7 = ltmp_2092_7 << 1;
  if ((ltmp_2091_7 < ltmp_2093_7)) {
    ltmp_2094_7_2E = ltmp_2092_7;
    goto ltmp_2103_D;
  } else {
    ltmp_2092_7_2E = ltmp_2093_7;
    goto ltmp_2104_D;
  }

  } while (1);
ltmp_2103_D:
  ltmp_2094_7 = ltmp_2094_7_2E;
  ltmp_2095_7 = ltmp_2094_7 & 2147483647u;
  ltmp_2096_7_2E = ltmp_2095_7;
  goto ltmp_2105_D;

  do {
ltmp_2105_D:
  ltmp_2096_7 = ltmp_2096_7_2E;
  ltmp_2097_7 = ltmp_2096_7 >> 1;
  ltmp_64_1001(ltmp_2087_1024, ((plzo_uint32_t )((ltmp_2097_7 & ltmp_2091_7) != 0u)));
  ltmp_64_1001(ltmp_2087_1024, 0u);
  if ((ltmp_2097_7 > 2u)) {
    ltmp_2096_7_2E = ltmp_2097_7;
    goto ltmp_2105_D;
  } else {
    ltmp_2098_7_2E = ltmp_2091_7;
    goto ltmp_2102_D;
  }

  } while (1);
ltmp_2102_D:
  ltmp_2098_7 = ltmp_2098_7_2E;
  ltmp_64_1001(ltmp_2087_1024, (ltmp_2098_7 & 1u));
  ltmp_64_1001(ltmp_2087_1024, 1u);
  return;
}


static void ltmp_42_1001(struct l_struct_2E_A5 *ltmp_2106_1024, plzo_uint32_t ltmp_2107_7) {
  plzo_uint32_t ltmp_2108_7;
  plzo_uint32_t ltmp_2109_7;
  plzo_uint32_t ltmp_2109_7_2E;
  plzo_uint32_t ltmp_2110_7;
  plzo_uint32_t ltmp_2111_7;
  plzo_uint32_t ltmp_2111_7_2E;
  plzo_uint32_t ltmp_2112_7;
  plzo_uint32_t ltmp_2113_7;
  plzo_uint32_t ltmp_2113_7_2E;
  plzo_uint32_t ltmp_2114_7;
  plzo_uint32_t ltmp_2115_7;
  plzo_uint32_t ltmp_2115_7_2E;

  if ((ltmp_2107_7 > 1u)) {
    goto ltmp_2116_D;
  } else {
    ltmp_2115_7_2E = ltmp_2107_7;
    goto ltmp_2117_D;
  }

ltmp_2116_D:
  ltmp_2108_7 = ltmp_2107_7 + 2u;
  if ((ltmp_2108_7 < 8u)) {
    ltmp_2111_7_2E = 4u;
    goto ltmp_2118_D;
  } else {
    ltmp_2109_7_2E = 8u;
    goto ltmp_2119_D;
  }

  do {
ltmp_2119_D:
  ltmp_2109_7 = ltmp_2109_7_2E;
  ltmp_2110_7 = ltmp_2109_7 << 1;
  if ((ltmp_2108_7 < ltmp_2110_7)) {
    ltmp_2111_7_2E = ltmp_2109_7;
    goto ltmp_2118_D;
  } else {
    ltmp_2109_7_2E = ltmp_2110_7;
    goto ltmp_2119_D;
  }

  } while (1);
ltmp_2118_D:
  ltmp_2111_7 = ltmp_2111_7_2E;
  ltmp_2112_7 = ltmp_2111_7 & 2147483647u;
  ltmp_2113_7_2E = ltmp_2112_7;
  goto ltmp_2120_D;

  do {
ltmp_2120_D:
  ltmp_2113_7 = ltmp_2113_7_2E;
  ltmp_2114_7 = ltmp_2113_7 >> 1;
  ltmp_66_1001(ltmp_2106_1024, ((plzo_uint32_t )((ltmp_2114_7 & ltmp_2108_7) != 0u)));
  ltmp_66_1001(ltmp_2106_1024, 0u);
  if ((ltmp_2114_7 > 2u)) {
    ltmp_2113_7_2E = ltmp_2114_7;
    goto ltmp_2120_D;
  } else {
    ltmp_2115_7_2E = ltmp_2108_7;
    goto ltmp_2117_D;
  }

  } while (1);
ltmp_2117_D:
  ltmp_2115_7 = ltmp_2115_7_2E;
  ltmp_66_1001(ltmp_2106_1024, (ltmp_2115_7 & 1u));
  ltmp_66_1001(ltmp_2106_1024, 1u);
  return;
}


static void ltmp_43_1001(struct l_struct_2E_A5 *ltmp_2121_1024, plzo_uint32_t ltmp_2122_7) {
  plzo_uint32_t ltmp_2123_7;
  plzo_uint32_t ltmp_2124_7;
  plzo_uint32_t ltmp_2125_7;
  plzo_uint32_t ltmp_2126_7;
  plzo_uint32_t ltmp_2126_7_2E;
  plzo_uint32_t ltmp_2127_7;
  plzo_uint32_t ltmp_2128_7;
  plzo_uint32_t ltmp_2128_7_2E;
  plzo_uint32_t ltmp_2129_7;
  plzo_uint32_t ltmp_2130_7;
  plzo_uint32_t ltmp_2130_7_2E;
  plzo_uint32_t ltmp_2131_7;
  plzo_uint32_t ltmp_2132_7;
  plzo_uint32_t ltmp_2132_7_2E;

  if ((ltmp_2122_7 > 2u)) {
    goto ltmp_2133_D;
  } else {
    goto ltmp_2134_D;
  }

ltmp_2134_D:
  ltmp_2123_7 = ltmp_2122_7 + 1u;
  ltmp_66_1001(ltmp_2121_1024, ((plzo_uint32_t )(ltmp_2123_7 > 1u)));
  ltmp_66_1001(ltmp_2121_1024, (ltmp_2123_7 & 1u));
  return;
ltmp_2133_D:
  ltmp_2124_7 = ltmp_2122_7 + 4294967293u;
  ltmp_66_1001(ltmp_2121_1024, 0u);
  ltmp_66_1001(ltmp_2121_1024, 0u);
  if ((ltmp_2124_7 > 1u)) {
    goto ltmp_2135_D;
  } else {
    ltmp_2132_7_2E = ltmp_2124_7;
    goto ltmp_2136_D;
  }

ltmp_2135_D:
  ltmp_2125_7 = ltmp_2122_7 + 4294967295u;
  if ((ltmp_2125_7 < 8u)) {
    ltmp_2128_7_2E = 4u;
    goto ltmp_2137_D;
  } else {
    ltmp_2126_7_2E = 8u;
    goto ltmp_2138_D;
  }

  do {
ltmp_2138_D:
  ltmp_2126_7 = ltmp_2126_7_2E;
  ltmp_2127_7 = ltmp_2126_7 << 1;
  if ((ltmp_2125_7 < ltmp_2127_7)) {
    ltmp_2128_7_2E = ltmp_2126_7;
    goto ltmp_2137_D;
  } else {
    ltmp_2126_7_2E = ltmp_2127_7;
    goto ltmp_2138_D;
  }

  } while (1);
ltmp_2137_D:
  ltmp_2128_7 = ltmp_2128_7_2E;
  ltmp_2129_7 = ltmp_2128_7 & 2147483647u;
  ltmp_2130_7_2E = ltmp_2129_7;
  goto ltmp_2139_D;

  do {
ltmp_2139_D:
  ltmp_2130_7 = ltmp_2130_7_2E;
  ltmp_2131_7 = ltmp_2130_7 >> 1;
  ltmp_66_1001(ltmp_2121_1024, ((plzo_uint32_t )((ltmp_2131_7 & ltmp_2125_7) != 0u)));
  ltmp_66_1001(ltmp_2121_1024, 0u);
  if ((ltmp_2131_7 > 2u)) {
    ltmp_2130_7_2E = ltmp_2131_7;
    goto ltmp_2139_D;
  } else {
    ltmp_2132_7_2E = ltmp_2125_7;
    goto ltmp_2136_D;
  }

  } while (1);
ltmp_2136_D:
  ltmp_2132_7 = ltmp_2132_7_2E;
  ltmp_66_1001(ltmp_2121_1024, (ltmp_2132_7 & 1u));
  ltmp_66_1001(ltmp_2121_1024, 1u);
  return;
}


static void ltmp_44_1001(struct l_struct_2E_A5 *ltmp_2140_1024, plzo_uint32_t ltmp_2141_7) {
  plzo_uint32_t ltmp_2142_7;
  plzo_uint32_t ltmp_2142_7_2E;
  plzo_uint32_t ltmp_2143_7;
  plzo_uint32_t ltmp_2143_7_2E;
  plzo_uint32_t ltmp_2144_7;
  plzo_uint32_t ltmp_2145_7;
  plzo_uint32_t ltmp_2146_7;
  plzo_uint32_t ltmp_2146_7_2E;
  plzo_uint32_t ltmp_2147_7;
  plzo_uint32_t ltmp_2148_7;
  plzo_uint32_t ltmp_2148_7_2E;

  if ((ltmp_2141_7 > 1u)) {
    ltmp_2142_7_2E = 2u;
    ltmp_2143_7_2E = ltmp_2141_7;
    goto ltmp_2149_D;
  } else {
    ltmp_2148_7_2E = ltmp_2141_7;
    goto ltmp_2150_D;
  }

  do {
ltmp_2149_D:
  ltmp_2142_7 = ltmp_2142_7_2E;
  ltmp_2143_7 = ltmp_2143_7_2E;
  ltmp_2144_7 = ltmp_2143_7 - ltmp_2142_7;
  ltmp_2145_7 = ltmp_2142_7 << 2;
  if ((ltmp_2144_7 < ltmp_2145_7)) {
    ltmp_2146_7_2E = ltmp_2145_7;
    goto ltmp_2151_D;
  } else {
    ltmp_2142_7_2E = ltmp_2145_7;
    ltmp_2143_7_2E = ltmp_2144_7;
    goto ltmp_2149_D;
  }

  } while (1);
  do {
ltmp_2151_D:
  ltmp_2146_7 = ltmp_2146_7_2E;
  ltmp_64_1001(ltmp_2140_1024, ((plzo_uint32_t )(((ltmp_2146_7 >> 1) & ltmp_2144_7) != 0u)));
  ltmp_64_1001(ltmp_2140_1024, 0u);
  ltmp_2147_7 = ltmp_2146_7 >> 2;
  ltmp_64_1001(ltmp_2140_1024, ((plzo_uint32_t )((ltmp_2147_7 & ltmp_2144_7) != 0u)));
  if ((ltmp_2147_7 > 2u)) {
    ltmp_2146_7_2E = ltmp_2147_7;
    goto ltmp_2151_D;
  } else {
    ltmp_2148_7_2E = ltmp_2144_7;
    goto ltmp_2150_D;
  }

  } while (1);
ltmp_2150_D:
  ltmp_2148_7 = ltmp_2148_7_2E;
  ltmp_64_1001(ltmp_2140_1024, (ltmp_2148_7 & 1u));
  ltmp_64_1001(ltmp_2140_1024, 1u);
  return;
}


static void ltmp_45_1001(struct l_struct_2E_A5 *ltmp_2152_1024, plzo_uint32_t ltmp_2153_7) {
  plzo_uint32_t ltmp_2154_7;
  plzo_uint32_t ltmp_2154_7_2E;
  plzo_uint32_t ltmp_2155_7;
  plzo_uint32_t ltmp_2155_7_2E;
  plzo_uint32_t ltmp_2156_7;
  plzo_uint32_t ltmp_2157_7;
  plzo_uint32_t ltmp_2158_7;
  plzo_uint32_t ltmp_2158_7_2E;
  plzo_uint32_t ltmp_2159_7;
  plzo_uint32_t ltmp_2160_7;
  plzo_uint32_t ltmp_2160_7_2E;

  if ((ltmp_2153_7 > 1u)) {
    ltmp_2154_7_2E = 2u;
    ltmp_2155_7_2E = ltmp_2153_7;
    goto ltmp_2161_D;
  } else {
    ltmp_2160_7_2E = ltmp_2153_7;
    goto ltmp_2162_D;
  }

  do {
ltmp_2161_D:
  ltmp_2154_7 = ltmp_2154_7_2E;
  ltmp_2155_7 = ltmp_2155_7_2E;
  ltmp_2156_7 = ltmp_2155_7 - ltmp_2154_7;
  ltmp_2157_7 = ltmp_2154_7 << 2;
  if ((ltmp_2156_7 < ltmp_2157_7)) {
    ltmp_2158_7_2E = ltmp_2157_7;
    goto ltmp_2163_D;
  } else {
    ltmp_2154_7_2E = ltmp_2157_7;
    ltmp_2155_7_2E = ltmp_2156_7;
    goto ltmp_2161_D;
  }

  } while (1);
  do {
ltmp_2163_D:
  ltmp_2158_7 = ltmp_2158_7_2E;
  ltmp_66_1001(ltmp_2152_1024, ((plzo_uint32_t )(((ltmp_2158_7 >> 1) & ltmp_2156_7) != 0u)));
  ltmp_66_1001(ltmp_2152_1024, 0u);
  ltmp_2159_7 = ltmp_2158_7 >> 2;
  ltmp_66_1001(ltmp_2152_1024, ((plzo_uint32_t )((ltmp_2159_7 & ltmp_2156_7) != 0u)));
  if ((ltmp_2159_7 > 2u)) {
    ltmp_2158_7_2E = ltmp_2159_7;
    goto ltmp_2163_D;
  } else {
    ltmp_2160_7_2E = ltmp_2156_7;
    goto ltmp_2162_D;
  }

  } while (1);
ltmp_2162_D:
  ltmp_2160_7 = ltmp_2160_7_2E;
  ltmp_66_1001(ltmp_2152_1024, (ltmp_2160_7 & 1u));
  ltmp_66_1001(ltmp_2152_1024, 1u);
  return;
}


static void ltmp_46_1001(struct l_struct_2E_A5 *ltmp_2164_1024, plzo_uint32_t ltmp_2165_7) {
  plzo_uint32_t ltmp_2166_7;
  plzo_uint32_t ltmp_2167_7;
  plzo_uint32_t ltmp_2167_7_2E;
  plzo_uint32_t ltmp_2168_7;
  plzo_uint32_t ltmp_2169_7;
  plzo_uint32_t ltmp_2169_7_2E;
  plzo_uint32_t ltmp_2170_7;
  plzo_uint32_t ltmp_2171_7;
  plzo_uint32_t ltmp_2171_7_2E;
  plzo_uint32_t ltmp_2172_7;
  plzo_uint32_t ltmp_2173_7;
  plzo_uint32_t ltmp_2173_7_2E;

  if ((ltmp_2165_7 > 1u)) {
    goto ltmp_2174_D;
  } else {
    ltmp_2173_7_2E = ltmp_2165_7;
    goto ltmp_2175_D;
  }

ltmp_2174_D:
  ltmp_2166_7 = ltmp_2165_7 + 2u;
  if ((ltmp_2166_7 < 8u)) {
    ltmp_2169_7_2E = 4u;
    goto ltmp_2176_D;
  } else {
    ltmp_2167_7_2E = 8u;
    goto ltmp_2177_D;
  }

  do {
ltmp_2177_D:
  ltmp_2167_7 = ltmp_2167_7_2E;
  ltmp_2168_7 = ltmp_2167_7 << 1;
  if ((ltmp_2166_7 < ltmp_2168_7)) {
    ltmp_2169_7_2E = ltmp_2167_7;
    goto ltmp_2176_D;
  } else {
    ltmp_2167_7_2E = ltmp_2168_7;
    goto ltmp_2177_D;
  }

  } while (1);
ltmp_2176_D:
  ltmp_2169_7 = ltmp_2169_7_2E;
  ltmp_2170_7 = ltmp_2169_7 & 2147483647u;
  ltmp_2171_7_2E = ltmp_2170_7;
  goto ltmp_2178_D;

  do {
ltmp_2178_D:
  ltmp_2171_7 = ltmp_2171_7_2E;
  ltmp_2172_7 = ltmp_2171_7 >> 1;
  ltmp_1_1001(ltmp_2164_1024, ((plzo_uint32_t )((ltmp_2172_7 & ltmp_2166_7) != 0u)));
  ltmp_1_1001(ltmp_2164_1024, 0u);
  if ((ltmp_2172_7 > 2u)) {
    ltmp_2171_7_2E = ltmp_2172_7;
    goto ltmp_2178_D;
  } else {
    ltmp_2173_7_2E = ltmp_2166_7;
    goto ltmp_2175_D;
  }

  } while (1);
ltmp_2175_D:
  ltmp_2173_7 = ltmp_2173_7_2E;
  ltmp_1_1001(ltmp_2164_1024, (ltmp_2173_7 & 1u));
  ltmp_1_1001(ltmp_2164_1024, 1u);
  return;
}


static void ltmp_47_1001(struct l_struct_2E_A5 *ltmp_2179_1024, plzo_uint32_t ltmp_2180_7) {
  plzo_uint32_t ltmp_2181_7;
  plzo_uint32_t ltmp_2181_7_2E;
  plzo_uint32_t ltmp_2182_7;
  plzo_uint32_t ltmp_2182_7_2E;
  plzo_uint32_t ltmp_2183_7;
  plzo_uint32_t ltmp_2184_7;
  plzo_uint32_t ltmp_2185_7;
  plzo_uint32_t ltmp_2185_7_2E;
  plzo_uint32_t ltmp_2186_7;
  plzo_uint32_t ltmp_2187_7;
  plzo_uint32_t ltmp_2187_7_2E;

  if ((ltmp_2180_7 > 1u)) {
    ltmp_2181_7_2E = 2u;
    ltmp_2182_7_2E = ltmp_2180_7;
    goto ltmp_2188_D;
  } else {
    ltmp_2187_7_2E = ltmp_2180_7;
    goto ltmp_2189_D;
  }

  do {
ltmp_2188_D:
  ltmp_2181_7 = ltmp_2181_7_2E;
  ltmp_2182_7 = ltmp_2182_7_2E;
  ltmp_2183_7 = ltmp_2182_7 - ltmp_2181_7;
  ltmp_2184_7 = ltmp_2181_7 << 2;
  if ((ltmp_2183_7 < ltmp_2184_7)) {
    ltmp_2185_7_2E = ltmp_2184_7;
    goto ltmp_2190_D;
  } else {
    ltmp_2181_7_2E = ltmp_2184_7;
    ltmp_2182_7_2E = ltmp_2183_7;
    goto ltmp_2188_D;
  }

  } while (1);
  do {
ltmp_2190_D:
  ltmp_2185_7 = ltmp_2185_7_2E;
  ltmp_1_1001(ltmp_2179_1024, ((plzo_uint32_t )(((ltmp_2185_7 >> 1) & ltmp_2183_7) != 0u)));
  ltmp_1_1001(ltmp_2179_1024, 0u);
  ltmp_2186_7 = ltmp_2185_7 >> 2;
  ltmp_1_1001(ltmp_2179_1024, ((plzo_uint32_t )((ltmp_2186_7 & ltmp_2183_7) != 0u)));
  if ((ltmp_2186_7 > 2u)) {
    ltmp_2185_7_2E = ltmp_2186_7;
    goto ltmp_2190_D;
  } else {
    ltmp_2187_7_2E = ltmp_2183_7;
    goto ltmp_2189_D;
  }

  } while (1);
ltmp_2189_D:
  ltmp_2187_7 = ltmp_2187_7_2E;
  ltmp_1_1001(ltmp_2179_1024, (ltmp_2187_7 & 1u));
  ltmp_1_1001(ltmp_2179_1024, 1u);
  return;
}


static void ltmp_48_1001(struct l_struct_2E_A5 *ltmp_2191_1024, plzo_uint32_t ltmp_2192_7) {
  plzo_uint32_t ltmp_2193_7;
  plzo_uint32_t ltmp_2194_7;
  plzo_uint32_t ltmp_2195_7;
  plzo_uint32_t ltmp_2196_7;
  plzo_uint32_t ltmp_2196_7_2E;
  plzo_uint32_t ltmp_2197_7;
  plzo_uint32_t ltmp_2198_7;
  plzo_uint32_t ltmp_2198_7_2E;
  plzo_uint32_t ltmp_2199_7;
  plzo_uint32_t ltmp_2200_7;
  plzo_uint32_t ltmp_2200_7_2E;
  plzo_uint32_t ltmp_2201_7;
  plzo_uint32_t ltmp_2202_7;
  plzo_uint32_t ltmp_2202_7_2E;

  if ((ltmp_2192_7 > 2u)) {
    goto ltmp_2203_D;
  } else {
    goto ltmp_2204_D;
  }

ltmp_2204_D:
  ltmp_2193_7 = ltmp_2192_7 + 1u;
  ltmp_1_1001(ltmp_2191_1024, ((plzo_uint32_t )(ltmp_2193_7 > 1u)));
  ltmp_1_1001(ltmp_2191_1024, (ltmp_2193_7 & 1u));
  return;
ltmp_2203_D:
  ltmp_2194_7 = ltmp_2192_7 + 4294967293u;
  ltmp_1_1001(ltmp_2191_1024, 0u);
  ltmp_1_1001(ltmp_2191_1024, 0u);
  if ((ltmp_2194_7 > 1u)) {
    goto ltmp_2205_D;
  } else {
    ltmp_2202_7_2E = ltmp_2194_7;
    goto ltmp_2206_D;
  }

ltmp_2205_D:
  ltmp_2195_7 = ltmp_2192_7 + 4294967295u;
  if ((ltmp_2195_7 < 8u)) {
    ltmp_2198_7_2E = 4u;
    goto ltmp_2207_D;
  } else {
    ltmp_2196_7_2E = 8u;
    goto ltmp_2208_D;
  }

  do {
ltmp_2208_D:
  ltmp_2196_7 = ltmp_2196_7_2E;
  ltmp_2197_7 = ltmp_2196_7 << 1;
  if ((ltmp_2195_7 < ltmp_2197_7)) {
    ltmp_2198_7_2E = ltmp_2196_7;
    goto ltmp_2207_D;
  } else {
    ltmp_2196_7_2E = ltmp_2197_7;
    goto ltmp_2208_D;
  }

  } while (1);
ltmp_2207_D:
  ltmp_2198_7 = ltmp_2198_7_2E;
  ltmp_2199_7 = ltmp_2198_7 & 2147483647u;
  ltmp_2200_7_2E = ltmp_2199_7;
  goto ltmp_2209_D;

  do {
ltmp_2209_D:
  ltmp_2200_7 = ltmp_2200_7_2E;
  ltmp_2201_7 = ltmp_2200_7 >> 1;
  ltmp_1_1001(ltmp_2191_1024, ((plzo_uint32_t )((ltmp_2201_7 & ltmp_2195_7) != 0u)));
  ltmp_1_1001(ltmp_2191_1024, 0u);
  if ((ltmp_2201_7 > 2u)) {
    ltmp_2200_7_2E = ltmp_2201_7;
    goto ltmp_2209_D;
  } else {
    ltmp_2202_7_2E = ltmp_2195_7;
    goto ltmp_2206_D;
  }

  } while (1);
ltmp_2206_D:
  ltmp_2202_7 = ltmp_2202_7_2E;
  ltmp_1_1001(ltmp_2191_1024, (ltmp_2202_7 & 1u));
  ltmp_1_1001(ltmp_2191_1024, 1u);
  return;
}


static signed char *ltmp_49_1018(struct plzo_callback_t *ltmp_2210_1026, plzo_uint32_t ltmp_2211_7, plzo_uint32_t ltmp_2212_7) {
  signed char *ltmp_2213_14;
  struct plzo_callback_t *ltmp_2214_1026;
  signed char * (*ltmp_2215_1018) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t );
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
  if ((ltmp_2215_1018 == ((signed char * (*) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t ))0))) {
    goto ltmp_2217_D;
  } else {
    goto ltmp_2219_D;
  }

ltmp_2219_D:
  ltmp_2216_14 = ltmp_2215_1018(ltmp_2214_1026, ltmp_2211_7, ltmp_2212_7);
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


static void ltmp_51_101A(struct plzo_callback_t *ltmp_2228_1026, plzo_uint32_t ltmp_2229_7, plzo_uint32_t ltmp_2230_7, plzo_int32_t ltmp_2231_8) {
  signed char *ltmp_2232_14;
  struct plzo_callback_t *ltmp_2233_1026;
  void  (*ltmp_2234_101A) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t );

  ltmp_2232_14 = *(&ltmp_2228_1026->field3);
  ltmp_2233_1026 = (struct plzo_callback_t *)ltmp_2232_14;
  if ((ltmp_2232_14 == ((signed char *)0))) {
    goto ltmp_2235_D;
  } else {
    goto ltmp_2236_D;
  }

ltmp_2236_D:
  ltmp_2234_101A = *(&ltmp_2233_1026->field2);
  if ((ltmp_2234_101A == ((void  (*) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t , plzo_int32_t ))0))) {
    goto ltmp_2235_D;
  } else {
    goto ltmp_2237_D;
  }

ltmp_2237_D:
  ltmp_2234_101A(ltmp_2233_1026, ltmp_2229_7, ltmp_2230_7, ltmp_2231_8);
  return;
ltmp_2235_D:
  return;
}


static void ltmp_52_101B(struct l_struct_2E_E *ltmp_2238_1023, struct l_struct_2E_E_3A__3A_A14 *ltmp_2239_1028) {
  void  (*ltmp_2240_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  plzo_uint32_t *ltmp_2241_17;
  plzo_uint32_t ltmp_2242_7;
  unsigned char **ltmp_2243_1025;
  signed char *ltmp_2244_14;
  struct plzo_callback_t *ltmp_2245_1026;
  signed char * (*ltmp_2246_1018) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t );
  signed char *ltmp_2247_14;

  ltmp_2240_1001 = *(&ltmp_2239_1028->field3);
  if ((ltmp_2240_1001 == ((void  (*) (struct l_struct_2E_A5 *, plzo_uint32_t ))0))) {
    goto ltmp_2248_D;
  } else {
    goto ltmp_2249_D;
  }

ltmp_2249_D:
  ltmp_2241_17 = &ltmp_2239_1028->field1;
  ltmp_2242_7 = *ltmp_2241_17;
  if ((ltmp_2242_7 == 0u)) {
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
  if ((ltmp_2246_1018 == ((signed char * (*) (struct plzo_callback_t *, plzo_uint32_t , plzo_uint32_t ))0))) {
    goto ltmp_2251_D;
  } else {
    goto ltmp_2253_D;
  }

ltmp_2253_D:
  ltmp_2247_14 = ltmp_2246_1018(ltmp_2245_1026, ltmp_2242_7, 1u);
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
  *ltmp_2241_17 = 0u;
  return;
ltmp_2248_D:
  return;
}


static void ltmp_53_101C(struct l_struct_2E_E_3A__3A_A14 *ltmp_2255_1028) {
  plzo_uint32_t  (**ltmp_2256_1040) (plzo_uint32_t );
  plzo_uint32_t  (*ltmp_2257_1041) (plzo_uint32_t );
  unsigned char **ltmp_2258_1025;
  unsigned char *ltmp_2259_13;
  plzo_uint32_t *ltmp_2260_17;
  plzo_uint32_t ltmp_2261_7;
  plzo_uint32_t ltmp_2262_7;
  plzo_uint32_t ltmp_2262_7_2E;
  plzo_uint32_t  (*ltmp_2263_1041) (plzo_uint32_t );
  plzo_uint32_t ltmp_2264_7;
  unsigned char *ltmp_2265_13;
  plzo_uint32_t ltmp_2266_7;
  plzo_uint32_t ltmp_2267_7;

  ltmp_2256_1040 = &ltmp_2255_1028->field2;
  ltmp_2257_1041 = *ltmp_2256_1040;
  if ((ltmp_2257_1041 == ((plzo_uint32_t  (*) (plzo_uint32_t ))0))) {
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
  ltmp_2260_17 = &ltmp_2255_1028->field1;
  ltmp_2261_7 = *ltmp_2260_17;
  if ((ltmp_2261_7 == 0u)) {
    goto ltmp_2271_D;
  } else {
    ltmp_2262_7_2E = 0u;
    goto ltmp_2272_D;
  }

  do {
ltmp_2272_D:
  ltmp_2262_7 = ltmp_2262_7_2E;
  ltmp_2263_1041 = *ltmp_2256_1040;
  ltmp_2264_7 = ltmp_2263_1041(ltmp_2262_7);
  ltmp_2265_13 = *ltmp_2258_1025;
  *(&ltmp_2265_13[ltmp_2262_7]) = ((unsigned char )ltmp_2264_7);
  ltmp_2266_7 = ltmp_2262_7 + 1u;
  ltmp_2267_7 = *ltmp_2260_17;
  if ((ltmp_2267_7 > ltmp_2266_7)) {
    ltmp_2262_7_2E = ltmp_2266_7;
    goto ltmp_2272_D;
  } else {
    goto ltmp_2271_D;
  }

  } while (1);
ltmp_2268_D:
  *(&ltmp_2255_1028->field1) = 0u;
  return;
ltmp_2271_D:
  return;
}


static void  (*ltmp_54_101D(void  (*ltmp_2273_1001) (struct l_struct_2E_A5 *, plzo_uint32_t ), plzo_uint32_t ltmp_2274_7, plzo_uint32_t ltmp_2275_7)) (struct l_struct_2E_A5 *, plzo_uint32_t ) {
  void  (*ltmp_2276_1001) (struct l_struct_2E_A5 *, plzo_uint32_t );
  void  (*ltmp_2276_1001_2E) (struct l_struct_2E_A5 *, plzo_uint32_t );

  if ((ltmp_2273_1001 == ltmp_46_1001)) {
    goto ltmp_2277_D;
  } else {
    goto ltmp_2278_D;
  }

ltmp_2277_D:
  if ((ltmp_2275_7 == 8u)) {
    ltmp_2276_1001_2E = ltmp_40_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2280_D;
  }

ltmp_2280_D:
  if (((ltmp_2275_7 == 32u) & (ltmp_2274_7 == 0u))) {
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
  if ((ltmp_2275_7 == 8u)) {
    ltmp_2276_1001_2E = ltmp_44_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2283_D;
  }

ltmp_2283_D:
  if (((ltmp_2275_7 == 32u) & (ltmp_2274_7 == 0u))) {
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
  if ((ltmp_2275_7 == 8u)) {
    ltmp_2276_1001_2E = ltmp_41_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2286_D;
  }

ltmp_2286_D:
  if (((ltmp_2275_7 == 32u) & (ltmp_2274_7 == 0u))) {
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
  if ((ltmp_2275_7 == 8u)) {
    ltmp_2276_1001_2E = ltmp_65_1001;
    goto ltmp_2279_D;
  } else {
    goto ltmp_2288_D;
  }

ltmp_2288_D:
  return (((((ltmp_2275_7 == 32u) & (ltmp_2274_7 == 0u))) ? (ltmp_67_1001) : (ltmp_2273_1001)));
ltmp_2279_D:
  ltmp_2276_1001 = ltmp_2276_1001_2E;
  return ltmp_2276_1001;
}


static plzo_int32_t ltmp_55_1007(struct l_struct_2E_E *ltmp_2289_1023) {

  return 0;
}


static void ltmp_56_101E(struct l_struct_2E_E *ltmp_2290_1023, unsigned char *ltmp_2291_13, plzo_uint32_t ltmp_2292_7, plzo_uint32_t ltmp_2293_7) {

  goto ltmp_2294_D;

  do {
ltmp_2294_D:
  goto ltmp_2294_D;

  } while (1);
}


static void ltmp_57_1000(struct l_struct_2E_E *ltmp_2295_1023, plzo_uint32_t ltmp_2296_7, plzo_uint32_t ltmp_2297_7) {

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


static void ltmp_59_101F(struct l_struct_2E_E *ltmp_2301_1023, struct l_struct_2E_E_3A__3A_A4 *ltmp_2302_103B, plzo_uint32_t ltmp_2303_7, plzo_uint32_t ltmp_2304_7, plzo_uint32_t ltmp_2305_7) {

  goto ltmp_2306_D;

  do {
ltmp_2306_D:
  goto ltmp_2306_D;

  } while (1);
}


static plzo_uint32_t ltmp_60_1020(struct l_struct_2E_E *ltmp_2307_1023) {

  return 9u;
}


static plzo_int32_t ltmp_61_1016(struct l_struct_2E_E *ltmp_2308_1023, struct l_struct_2E_A7 *ltmp_2309_1038) {
  plzo_uint32_t ltmp_2310_7;

  ltmp_2310_7 = *(&ltmp_2309_1038->field0);
  return ((plzo_int32_t )(ltmp_2310_7 == 0u));
}


static plzo_int32_t ltmp_62_1016(struct l_struct_2E_E *ltmp_2311_1023, struct l_struct_2E_A7 *ltmp_2312_1038) {
  plzo_uint32_t ltmp_2313_7;

  ltmp_2313_7 = *(&ltmp_2312_1038->field0);
  return ((plzo_int32_t )(ltmp_2313_7 < 3u));
}


static void ltmp_63_1001(struct l_struct_2E_A5 *ltmp_2314_1024, plzo_uint32_t ltmp_2315_7) {
  plzo_uint32_t ltmp_2316_7;
  plzo_uint32_t ltmp_2317_7;
  plzo_uint32_t ltmp_2318_7;
  plzo_uint32_t ltmp_2318_7_2E;
  plzo_uint32_t ltmp_2319_7;
  plzo_uint32_t ltmp_2319_7_2E;
  plzo_uint32_t ltmp_2320_7;
  plzo_uint32_t ltmp_2321_7;
  plzo_uint32_t ltmp_2322_7;
  plzo_uint32_t ltmp_2322_7_2E;
  plzo_uint32_t ltmp_2323_7;
  plzo_uint32_t ltmp_2324_7;
  plzo_uint32_t ltmp_2324_7_2E;

  if ((ltmp_2315_7 > 2u)) {
    goto ltmp_2325_D;
  } else {
    goto ltmp_2326_D;
  }

ltmp_2326_D:
  ltmp_2316_7 = ltmp_2315_7 + 1u;
  ltmp_1_1001(ltmp_2314_1024, ((plzo_uint32_t )(ltmp_2316_7 > 1u)));
  ltmp_1_1001(ltmp_2314_1024, (ltmp_2316_7 & 1u));
  return;
ltmp_2325_D:
  ltmp_2317_7 = ltmp_2315_7 + 4294967293u;
  ltmp_1_1001(ltmp_2314_1024, 0u);
  ltmp_1_1001(ltmp_2314_1024, 0u);
  if ((ltmp_2317_7 > 1u)) {
    ltmp_2318_7_2E = 2u;
    ltmp_2319_7_2E = ltmp_2317_7;
    goto ltmp_2327_D;
  } else {
    ltmp_2324_7_2E = ltmp_2317_7;
    goto ltmp_2328_D;
  }

  do {
ltmp_2327_D:
  ltmp_2318_7 = ltmp_2318_7_2E;
  ltmp_2319_7 = ltmp_2319_7_2E;
  ltmp_2320_7 = ltmp_2319_7 - ltmp_2318_7;
  ltmp_2321_7 = ltmp_2318_7 << 2;
  if ((ltmp_2320_7 < ltmp_2321_7)) {
    ltmp_2322_7_2E = ltmp_2321_7;
    goto ltmp_2329_D;
  } else {
    ltmp_2318_7_2E = ltmp_2321_7;
    ltmp_2319_7_2E = ltmp_2320_7;
    goto ltmp_2327_D;
  }

  } while (1);
  do {
ltmp_2329_D:
  ltmp_2322_7 = ltmp_2322_7_2E;
  ltmp_1_1001(ltmp_2314_1024, ((plzo_uint32_t )(((ltmp_2322_7 >> 1) & ltmp_2320_7) != 0u)));
  ltmp_1_1001(ltmp_2314_1024, 0u);
  ltmp_2323_7 = ltmp_2322_7 >> 2;
  ltmp_1_1001(ltmp_2314_1024, ((plzo_uint32_t )((ltmp_2323_7 & ltmp_2320_7) != 0u)));
  if ((ltmp_2323_7 > 2u)) {
    ltmp_2322_7_2E = ltmp_2323_7;
    goto ltmp_2329_D;
  } else {
    ltmp_2324_7_2E = ltmp_2320_7;
    goto ltmp_2328_D;
  }

  } while (1);
ltmp_2328_D:
  ltmp_2324_7 = ltmp_2324_7_2E;
  ltmp_1_1001(ltmp_2314_1024, (ltmp_2324_7 & 1u));
  ltmp_1_1001(ltmp_2314_1024, 1u);
  return;
}


static void ltmp_64_1001(struct l_struct_2E_A5 *ltmp_2330_1024, plzo_uint32_t ltmp_2331_7) {
  plzo_uint32_t *ltmp_2332_17;
  plzo_uint32_t ltmp_2333_7;
  unsigned char *ltmp_2334_13;
  plzo_uint32_t ltmp_2335_7;
  plzo_uint32_t *ltmp_2336_17;
  plzo_uint32_t ltmp_2337_7;
  plzo_uint32_t ltmp_2338_7;
  unsigned char **ltmp_2339_1025;
  unsigned char *ltmp_2340_13;

  ltmp_2332_17 = &ltmp_2330_1024->field1;
  ltmp_2333_7 = *ltmp_2332_17;
  if ((ltmp_2333_7 == 8u)) {
    goto ltmp_2341_D;
  } else {
    goto ltmp_2342_D;
  }

ltmp_2341_D:
  ltmp_2334_13 = *(&ltmp_2330_1024->field5);
  ltmp_2335_7 = *(&ltmp_2330_1024->field0);
  *ltmp_2334_13 = ((unsigned char )ltmp_2335_7);
  *ltmp_2332_17 = 0u;
  goto ltmp_2342_D;

ltmp_2342_D:
  ltmp_2336_17 = &ltmp_2330_1024->field0;
  ltmp_2337_7 = *ltmp_2336_17;
  *ltmp_2336_17 = ((ltmp_2337_7 << 1) + ltmp_2331_7);
  ltmp_2338_7 = *ltmp_2332_17;
  *ltmp_2332_17 = (ltmp_2338_7 + 1u);
  if ((ltmp_2338_7 == 0u)) {
    goto ltmp_2343_D;
  } else {
    goto ltmp_2344_D;
  }

ltmp_2343_D:
  ltmp_2339_1025 = &ltmp_2330_1024->field6;
  ltmp_2340_13 = *ltmp_2339_1025;
  *(&ltmp_2330_1024->field5) = ltmp_2340_13;
  *ltmp_2339_1025 = (&ltmp_2340_13[1]);
  return;
ltmp_2344_D:
  return;
}


static void ltmp_65_1001(struct l_struct_2E_A5 *ltmp_2345_1024, plzo_uint32_t ltmp_2346_7) {
  plzo_uint32_t ltmp_2347_7;
  plzo_uint32_t ltmp_2348_7;
  plzo_uint32_t ltmp_2349_7;
  plzo_uint32_t ltmp_2349_7_2E;
  plzo_uint32_t ltmp_2350_7;
  plzo_uint32_t ltmp_2350_7_2E;
  plzo_uint32_t ltmp_2351_7;
  plzo_uint32_t ltmp_2352_7;
  plzo_uint32_t ltmp_2353_7;
  plzo_uint32_t ltmp_2353_7_2E;
  plzo_uint32_t ltmp_2354_7;
  plzo_uint32_t ltmp_2355_7;
  plzo_uint32_t ltmp_2355_7_2E;

  if ((ltmp_2346_7 > 2u)) {
    goto ltmp_2356_D;
  } else {
    goto ltmp_2357_D;
  }

ltmp_2357_D:
  ltmp_2347_7 = ltmp_2346_7 + 1u;
  ltmp_64_1001(ltmp_2345_1024, ((plzo_uint32_t )(ltmp_2347_7 > 1u)));
  ltmp_64_1001(ltmp_2345_1024, (ltmp_2347_7 & 1u));
  return;
ltmp_2356_D:
  ltmp_2348_7 = ltmp_2346_7 + 4294967293u;
  ltmp_64_1001(ltmp_2345_1024, 0u);
  ltmp_64_1001(ltmp_2345_1024, 0u);
  if ((ltmp_2348_7 > 1u)) {
    ltmp_2349_7_2E = 2u;
    ltmp_2350_7_2E = ltmp_2348_7;
    goto ltmp_2358_D;
  } else {
    ltmp_2355_7_2E = ltmp_2348_7;
    goto ltmp_2359_D;
  }

  do {
ltmp_2358_D:
  ltmp_2349_7 = ltmp_2349_7_2E;
  ltmp_2350_7 = ltmp_2350_7_2E;
  ltmp_2351_7 = ltmp_2350_7 - ltmp_2349_7;
  ltmp_2352_7 = ltmp_2349_7 << 2;
  if ((ltmp_2351_7 < ltmp_2352_7)) {
    ltmp_2353_7_2E = ltmp_2352_7;
    goto ltmp_2360_D;
  } else {
    ltmp_2349_7_2E = ltmp_2352_7;
    ltmp_2350_7_2E = ltmp_2351_7;
    goto ltmp_2358_D;
  }

  } while (1);
  do {
ltmp_2360_D:
  ltmp_2353_7 = ltmp_2353_7_2E;
  ltmp_64_1001(ltmp_2345_1024, ((plzo_uint32_t )(((ltmp_2353_7 >> 1) & ltmp_2351_7) != 0u)));
  ltmp_64_1001(ltmp_2345_1024, 0u);
  ltmp_2354_7 = ltmp_2353_7 >> 2;
  ltmp_64_1001(ltmp_2345_1024, ((plzo_uint32_t )((ltmp_2354_7 & ltmp_2351_7) != 0u)));
  if ((ltmp_2354_7 > 2u)) {
    ltmp_2353_7_2E = ltmp_2354_7;
    goto ltmp_2360_D;
  } else {
    ltmp_2355_7_2E = ltmp_2351_7;
    goto ltmp_2359_D;
  }

  } while (1);
ltmp_2359_D:
  ltmp_2355_7 = ltmp_2355_7_2E;
  ltmp_64_1001(ltmp_2345_1024, (ltmp_2355_7 & 1u));
  ltmp_64_1001(ltmp_2345_1024, 1u);
  return;
}


static void ltmp_66_1001(struct l_struct_2E_A5 *ltmp_2361_1024, plzo_uint32_t ltmp_2362_7) {
  plzo_uint32_t *ltmp_2363_17;
  plzo_uint32_t ltmp_2364_7;
  plzo_uint32_t ltmp_2365_7;
  unsigned char **ltmp_2366_1025;
  unsigned char *ltmp_2367_13;
  unsigned char *ltmp_2368_13;
  unsigned char *ltmp_2369_13;
  unsigned char *ltmp_2370_13;
  plzo_uint32_t *ltmp_2371_17;
  plzo_uint32_t ltmp_2372_7;
  plzo_uint32_t ltmp_2373_7;
  unsigned char **ltmp_2374_1025;
  unsigned char *ltmp_2375_13;

  ltmp_2363_17 = &ltmp_2361_1024->field1;
  ltmp_2364_7 = *ltmp_2363_17;
  if ((ltmp_2364_7 == 32u)) {
    goto ltmp_2376_D;
  } else {
    goto ltmp_2377_D;
  }

ltmp_2376_D:
  ltmp_2365_7 = *(&ltmp_2361_1024->field0);
  ltmp_2366_1025 = &ltmp_2361_1024->field5;
  ltmp_2367_13 = *ltmp_2366_1025;
  *ltmp_2367_13 = ((unsigned char )ltmp_2365_7);
  ltmp_2368_13 = *ltmp_2366_1025;
  *(&ltmp_2368_13[1]) = ((unsigned char )(ltmp_2365_7 >> 8));
  ltmp_2369_13 = *ltmp_2366_1025;
  *(&ltmp_2369_13[2]) = ((unsigned char )(ltmp_2365_7 >> 16));
  ltmp_2370_13 = *ltmp_2366_1025;
  *(&ltmp_2370_13[3]) = ((unsigned char )(ltmp_2365_7 >> 24));
  *ltmp_2363_17 = 0u;
  goto ltmp_2377_D;

ltmp_2377_D:
  ltmp_2371_17 = &ltmp_2361_1024->field0;
  ltmp_2372_7 = *ltmp_2371_17;
  *ltmp_2371_17 = ((ltmp_2372_7 << 1) + ltmp_2362_7);
  ltmp_2373_7 = *ltmp_2363_17;
  *ltmp_2363_17 = (ltmp_2373_7 + 1u);
  if ((ltmp_2373_7 == 0u)) {
    goto ltmp_2378_D;
  } else {
    goto ltmp_2379_D;
  }

ltmp_2378_D:
  ltmp_2374_1025 = &ltmp_2361_1024->field6;
  ltmp_2375_13 = *ltmp_2374_1025;
  *(&ltmp_2361_1024->field5) = ltmp_2375_13;
  *ltmp_2374_1025 = (&ltmp_2375_13[4]);
  return;
ltmp_2379_D:
  return;
}


static void ltmp_67_1001(struct l_struct_2E_A5 *ltmp_2380_1024, plzo_uint32_t ltmp_2381_7) {
  plzo_uint32_t ltmp_2382_7;
  plzo_uint32_t ltmp_2383_7;
  plzo_uint32_t ltmp_2384_7;
  plzo_uint32_t ltmp_2384_7_2E;
  plzo_uint32_t ltmp_2385_7;
  plzo_uint32_t ltmp_2385_7_2E;
  plzo_uint32_t ltmp_2386_7;
  plzo_uint32_t ltmp_2387_7;
  plzo_uint32_t ltmp_2388_7;
  plzo_uint32_t ltmp_2388_7_2E;
  plzo_uint32_t ltmp_2389_7;
  plzo_uint32_t ltmp_2390_7;
  plzo_uint32_t ltmp_2390_7_2E;

  if ((ltmp_2381_7 > 2u)) {
    goto ltmp_2391_D;
  } else {
    goto ltmp_2392_D;
  }

ltmp_2392_D:
  ltmp_2382_7 = ltmp_2381_7 + 1u;
  ltmp_66_1001(ltmp_2380_1024, ((plzo_uint32_t )(ltmp_2382_7 > 1u)));
  ltmp_66_1001(ltmp_2380_1024, (ltmp_2382_7 & 1u));
  return;
ltmp_2391_D:
  ltmp_2383_7 = ltmp_2381_7 + 4294967293u;
  ltmp_66_1001(ltmp_2380_1024, 0u);
  ltmp_66_1001(ltmp_2380_1024, 0u);
  if ((ltmp_2383_7 > 1u)) {
    ltmp_2384_7_2E = 2u;
    ltmp_2385_7_2E = ltmp_2383_7;
    goto ltmp_2393_D;
  } else {
    ltmp_2390_7_2E = ltmp_2383_7;
    goto ltmp_2394_D;
  }

  do {
ltmp_2393_D:
  ltmp_2384_7 = ltmp_2384_7_2E;
  ltmp_2385_7 = ltmp_2385_7_2E;
  ltmp_2386_7 = ltmp_2385_7 - ltmp_2384_7;
  ltmp_2387_7 = ltmp_2384_7 << 2;
  if ((ltmp_2386_7 < ltmp_2387_7)) {
    ltmp_2388_7_2E = ltmp_2387_7;
    goto ltmp_2395_D;
  } else {
    ltmp_2384_7_2E = ltmp_2387_7;
    ltmp_2385_7_2E = ltmp_2386_7;
    goto ltmp_2393_D;
  }

  } while (1);
  do {
ltmp_2395_D:
  ltmp_2388_7 = ltmp_2388_7_2E;
  ltmp_66_1001(ltmp_2380_1024, ((plzo_uint32_t )(((ltmp_2388_7 >> 1) & ltmp_2386_7) != 0u)));
  ltmp_66_1001(ltmp_2380_1024, 0u);
  ltmp_2389_7 = ltmp_2388_7 >> 2;
  ltmp_66_1001(ltmp_2380_1024, ((plzo_uint32_t )((ltmp_2389_7 & ltmp_2386_7) != 0u)));
  if ((ltmp_2389_7 > 2u)) {
    ltmp_2388_7_2E = ltmp_2389_7;
    goto ltmp_2395_D;
  } else {
    ltmp_2390_7_2E = ltmp_2386_7;
    goto ltmp_2394_D;
  }

  } while (1);
ltmp_2394_D:
  ltmp_2390_7 = ltmp_2390_7_2E;
  ltmp_66_1001(ltmp_2380_1024, (ltmp_2390_7 & 1u));
  ltmp_66_1001(ltmp_2380_1024, 1u);
  return;
}


