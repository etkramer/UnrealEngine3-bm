/*=============================================================================
	UnMathXe.h: Xbox 360 specific vector intrinsics

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef HEADER_UNMATHXE
#define HEADER_UNMATHXE

#include <vectorintrinsics.h>


/*=============================================================================
 *	Helpers:
 *============================================================================*/

/** 16-byte vector register type */
typedef __vector4		VectorRegister;

/**
 * Returns a bitwise equivalent vector based on 4 DWORDs.
 *
 * @param X		1st DWORD component
 * @param Y		2nd DWORD component
 * @param Z		3rd DWORD component
 * @param W		4th DWORD component
 * @return		Bitwise equivalent vector with 4 floats
 */
FORCEINLINE VectorRegister MakeVectorRegister( DWORD X, DWORD Y, DWORD Z, DWORD W )
{
	union { VectorRegister v; DWORD f[4]; } Tmp;
	Tmp.f[0] = X;
	Tmp.f[1] = Y;
	Tmp.f[2] = Z;
	Tmp.f[3] = W;
	return Tmp.v;
}

/**
 * Returns a vector based on 4 FLOATs.
 *
 * @param X		1st FLOAT component
 * @param Y		2nd FLOAT component
 * @param Z		3rd FLOAT component
 * @param W		4th FLOAT component
 * @return		Vector of the 4 FLOATs
 */
FORCEINLINE VectorRegister MakeVectorRegister( FLOAT X, FLOAT Y, FLOAT Z, FLOAT W )
{
	union { VectorRegister v; FLOAT f[4]; } Tmp;
	Tmp.f[0] = X;
	Tmp.f[1] = Y;
	Tmp.f[2] = Z;
	Tmp.f[3] = W;
	return Tmp.v;
}

///** Makes a swizzle mask that selects each component from either Src1 (0-3) or Src2 (4-7). */
//#define SWIZZLEFLOAT( C )	((((C)*4 + 0)<<24) | (((C)*4 + 1)<<16) | (((C)*4 + 2)<<8) | ((C)*4 + 3))
//#define SWIZZLEMASK( X, Y, Z, W )		\
//	MakeVectorRegister( SWIZZLEFLOAT(X), SWIZZLEFLOAT(Y), SWIZZLEFLOAT(Z), SWIZZLEFLOAT(W) )
#define SWIZZLEMASK( X, Y, Z, W )	( ((X)<<6) | ((Y)<<4) | ((Z)<<2) | ((W)<<0) )

/** Vector that represents (1,1,1,1) */
static const VectorRegister XBOXALTIVEC_ONE = MakeVectorRegister( 1.0f, 1.0f, 1.0f, 1.0f );

/** Select mask that selects Src1.XYZ and Src2.W to make (X,Y,Z,W). */
static const VectorRegister XBOXALTIVEC_SELECT_MASK = MakeVectorRegister( 0x00000000, 0x00000000, 0x00000000, (DWORD)0xffffffff );

/** Swizzle mask that selects each byte in X from Src1 and the last byte of W from Src2 to make (WWWX[0],WWWX[1],WWWX[2],WWWX[3]). */
static const VectorRegister	XBOXALTIVEC_SWIZZLE_MASK_UNPACK = MakeVectorRegister( 0x1f1f1f0c, 0x1f1f1f0d, 0x1f1f1f0e, (DWORD)0x1f1f1f0f );

/** Swizzle mask that selects each byte in X in reverse order from Src1 and the last byte of W from Src2 to make (WWWX[3],WWWX[2],WWWX[1],WWWX[0]). */
static const VectorRegister	XBOXALTIVEC_SWIZZLE_MASK_UNPACK_REVERSE = MakeVectorRegister( 0x1f1f1f0f, 0x1f1f1f0e, 0x1f1f1f0d, (DWORD)0x1f1f1f0c );


/**
 * Converts 16 WORDs from two vectors into 16 BYTEs in a single vector. Values are clamped to [0,255].
 *
 * @param Vec1		1st vector (converts into BYTE 0-7)
 * @param Vec2		2nd vector (converts into BYTE 8-15)
 * @return			A VectorRegister where BYTE 0-7 comes from the 8 WORDs in Vec1 and BYTE 8-15 comes from the 8 WORDs in Vec2.
 */
#define VectorU16ToU8( Vec1, Vec2 )				__vpkuhus( Vec1, Vec2 )

/**
 * Converts 8 DWORDs from two vectors into 8 WORDs in a single vector. Values are clamped to [0,65535].
 *
 * @param Vec1		1st vector (converts into WORD 0-3)
 * @param Vec2		2nd vector (converts into WORD 4-7)
 * @return			A VectorRegister where WORD 0-3 comes from the 4 DWORDs in Vec1 and WORD 4-7 comes from the 4 DWORDs in Vec2.
 */
#define VectorU32ToU16( Vec1, Vec2 )			__vpkuwus( Vec1, Vec2 )

/**
 * Converts 4 INTs into 4 FLOATs and returns the result.
 *
 * @param Vec	Source INT vector
 * @return		VectorRegister( FLOAT(Vec.x), FLOAT(Vec.y), FLOAT(Vec.z), FLOAT(Vec.w) )
 */
#define VectorItof( Vec )						__vcfsx( Vec, 0 )

/**
 * Converts 4 UINTs into 4 FLOATs and returns the result.
 *
 * @param Vec	Source UINT vector
 * @return		VectorRegister( FLOAT(Vec.x), FLOAT(Vec.y), FLOAT(Vec.z), FLOAT(Vec.w) )
 */
#define VectorUitof( Vec )						__vcfux( Vec, 0 )

/**
 * Converts 4 FLOATs into 4 INTs (with saturation, rounding towards zero) and returns the result.
 *
 * @param Vec	Source FLOAT vector
 * @return		VectorRegister( INT(Vec.x), INT(Vec.y), INT(Vec.z), INT(Vec.w) )
 */
#define VectorFtoi( Vec )						__vctsxs( Vec, 0 ) )

/**
 * Converts 4 FLOATs into 4 UINTs (with saturation, rounding towards zero) and returns the result.
 *
 * @param Vec	Source FLOAT vector
 * @return		VectorRegister( UINT(Vec.x), UINT(Vec.y), UINT(Vec.z), UINT(Vec.w) )
 */
#define VectorFtoui( Vec )						__vctuxs( Vec, 0 )


/*=============================================================================
 *	Intrinsics:
 *============================================================================*/

/**
 * Returns a vector with all zeros.
 *
 * @return		VectorRegister(0.0f, 0.0f, 0.0f, 0.0f)
 */
#define VectorZero()					__vspltisw( 0 )

/**
 * Returns a vector with all ones.
 *
 * @return		VectorRegister(1.0f, 1.0f, 1.0f, 1.0f)
 */
#define VectorOne()						__vspltw( __vupkd3d(__vspltisw( 0 ), VPACK_NORMSHORT2), 3 )

/**
 * Loads 4 FLOATs from unaligned memory.
 *
 * @param Ptr	Unaligned memory pointer to the 4 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
 */
#define VectorLoad( Ptr )				__vperm( __lvx(Ptr, 0), __lvx(Ptr, 15), __lvsl(Ptr,0) )

/**
 * Loads 3 FLOATs from unaligned memory and leaves W undefined.
 *
 * @param Ptr	Unaligned memory pointer to the 3 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], undefined)
 */
#define VectorLoadFloat3( Ptr )			__vperm( __lvx(Ptr, 0), __lvx(Ptr, 11), __lvsl(Ptr,0) )

/**
 * Loads 3 FLOATs from unaligned memory and sets W=0.
 *
 * @param Ptr	Unaligned memory pointer to the 3 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], 0.0f)
 */
#define VectorLoadFloat3_W0( Ptr )		__vsel( VectorLoadFloat3(Ptr), VectorZero(), XBOXALTIVEC_SELECT_MASK )

/**
 * Loads 3 FLOATs from unaligned memory and sets W=1.
 *
 * @param Ptr	Unaligned memory pointer to the 3 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], 1.0f)
 */
#define VectorLoadFloat3_W1( Ptr )		__vsel( VectorLoadFloat3(Ptr), VectorOne(), XBOXALTIVEC_SELECT_MASK )

/**
 * Loads 4 FLOATs from aligned memory.
 *
 * @param Ptr	Aligned memory pointer to the 4 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
 */
#define VectorLoadAligned( Ptr )		__lvx( Ptr, 0 )

/**
 * Loads 1 FLOAT from unaligned memory and replicates it to all 4 elements.
 *
 * @param Ptr	Unaligned memory pointer to the FLOAT
 * @return		VectorRegister(Ptr[0], Ptr[0], Ptr[0], Ptr[0])
 */
#define VectorLoadFloat1( Ptr )			__vspltw( __vperm(__lvx(Ptr, 0), __lvx(Ptr, 11), __lvsl(Ptr,0)), 0 )

/**
 * Creates a vector out of three FLOATs and leaves W undefined.
 *
 * @param X		1st FLOAT component
 * @param Y		2nd FLOAT component
 * @param Z		3rd FLOAT component
 * @return		VectorRegister(X, Y, Z, undefined)
 */
FORCEINLINE VectorRegister VectorSetFloat3( FLOAT X, FLOAT Y, FLOAT Z )
{
	union { VectorRegister v; float f[4]; } Tmp;
	Tmp.f[0] = X;
	Tmp.f[1] = Y;
	Tmp.f[2] = Z;
	return Tmp.v;
}

/**
 * Creates a vector out of four FLOATs.
 *
 * @param X		1st FLOAT component
 * @param Y		2nd FLOAT component
 * @param Z		3rd FLOAT component
 * @param W		4th FLOAT component
 * @return		VectorRegister(X, Y, Z, W)
 */
#define VectorSet( X, Y, Z, W )					MakeVectorRegister( X, Y, Z, W )

/**
 * Stores a vector to aligned memory.
 *
 * @param Vec	Vector to store
 * @param Ptr	Aligned memory pointer
 */
#define VectorStoreAligned( Vec, Ptr )			__stvx( Vec, Ptr, 0 )

/**
 * Stores a vector to memory (aligned or unaligned).
 *
 * @param Vec	Vector to store
 * @param Ptr	Memory pointer
 */
#define VectorStore( Vec, Ptr )					(__stvlx( Vec, Ptr, 0 ), __stvrx(Vec, Ptr, 16))

/**
 * Stores the XYZ components of a vector to unaligned memory.
 *
 * @param Vec	Vector to store XYZ
 * @param Ptr	Unaligned memory pointer
 */
#define VectorStoreFloat3( Vec, Ptr )			(__stvewx(__vspltw(Vec,0), Ptr, 0), __stvewx(__vspltw(Vec,1), Ptr, 4), __stvewx(__vspltw(Vec,2), Ptr, 8))

/**
 * Stores the X component of a vector to unaligned memory.
 *
 * @param Vec	Vector to store X
 * @param Ptr	Unaligned memory pointer
 */
#define VectorStoreFloat1( Vec, Ptr )			__stvewx(__vspltw(Vec,0), Ptr, 0)

/**
 * Replicates one element into all four elements and returns the new vector.
 *
 * @param Vec			Source vector
 * @param ElementIndex	Index (0-3) of the element to replicate
 * @return				VectorRegister( Vec[ElementIndex], Vec[ElementIndex], Vec[ElementIndex], Vec[ElementIndex] )
 */
#define VectorReplicate( Vec, ElementIndex )	__vspltw( Vec, ElementIndex )

/**
 * Returns the absolute value (component-wise).
 *
 * @param Vec			Source vector
 * @return				VectorRegister( abs(Vec.x), abs(Vec.y), abs(Vec.z), abs(Vec.w) )
 */
FORCEINLINE VectorRegister VectorAbs( const VectorRegister &Vec )
{
	VectorRegister SignMask = __vspltisw(-1);	// SignMask =		(0xffffffff,0xffffffff,0xffffffff,0xffffffff)
    SignMask = __vslw(SignMask, SignMask);		// SignMask << 31	(0x80000000,0x80000000,0x80000000,0x80000000)
    return __vandc(Vec, SignMask);				// Vec & ~SignMask	(AND with 0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff)
}

/**
 * Returns the negated value (component-wise).
 *
 * @param Vec			Source vector
 * @return				VectorRegister( -Vec.x, -Vec.y, -Vec.z, -Vec.w )
 */
FORCEINLINE VectorRegister VectorNegate( const VectorRegister &Vec )
{
	VectorRegister SignMask = __vspltisw(-1);	// SignMask =		(0xffffffff,0xffffffff,0xffffffff,0xffffffff)
    SignMask = __vslw(SignMask, SignMask);		// SignMask << 31	(0x80000000,0x80000000,0x80000000,0x80000000)
    return __vxor(Vec, SignMask);				// Vec ^ SignMask	(XOR with 0x80000000,0x80000000,0x80000000,0x80000000)
}

/**
 * Adds two vectors (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x+Vec2.x, Vec1.y+Vec2.y, Vec1.z+Vec2.z, Vec1.w+Vec2.w )
 */
#define VectorAdd( Vec1, Vec2 )					__vaddfp( Vec1, Vec2 )

/**
 * Subtracts a vector from another (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x-Vec2.x, Vec1.y-Vec2.y, Vec1.z-Vec2.z, Vec1.w-Vec2.w )
 */
#define VectorSubtract( Vec1, Vec2 )			__vsubfp( Vec1, Vec2 )

/**
 * Multiplies two vectors (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x*Vec2.x, Vec1.y*Vec2.y, Vec1.z*Vec2.z, Vec1.w*Vec2.w )
 */
#define VectorMultiply( Vec1, Vec2 )			__vmulfp( Vec1, Vec2 )

/**
 * Multiplies two vectors (component-wise), adds in the third vector and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @param Vec3	3rd vector
 * @return		VectorRegister( Vec1.x*Vec2.x + Vec3.x, Vec1.y*Vec2.y + Vec3.y, Vec1.z*Vec2.z + Vec3.z, Vec1.w*Vec2.w + Vec3.w )
 */
#define VectorMultiplyAdd( Vec1, Vec2, Vec3 )	__vmaddfp( Vec1, Vec2, Vec3 )

/**
 * Calculates the dot3 product of two vectors and returns a vector with the result in all 4 components.
 * Only really efficient on Xbox 360.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		d = dot3(Vec1.xyz, Vec2.xyz), VectorRegister( d, d, d, d )
 */
#define VectorDot3( Vec1, Vec2 )				__vmsum3fp( Vec1, Vec2 )

/**
 * Calculates the dot4 product of two vectors and returns a vector with the result in all 4 components.
 * Only really efficient on Xbox 360.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		d = dot4(Vec1.xyzw, Vec2.xyzw), VectorRegister( d, d, d, d )
 */
#define VectorDot4( Vec1, Vec2 )				__vmsum4fp( Vec1, Vec2 )

/**
 * Calculates the cross product of two vectors (XYZ components). W is set to 0.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		cross(Vec1.xyz, Vec2.xyz). W is set to 0.
 */
FORCEINLINE VectorRegister VectorCross( const VectorRegister& Vec1, const VectorRegister& Vec2 )
{
	VectorRegister A_YZXW = __vpermwi( Vec1, SWIZZLEMASK(1,2,0,3) );
	VectorRegister B_ZXYW = __vpermwi( Vec2, SWIZZLEMASK(2,0,1,3) );
	VectorRegister A_ZXYW = __vpermwi( Vec1, SWIZZLEMASK(2,0,1,3) );
	VectorRegister B_YZXW = __vpermwi( Vec2, SWIZZLEMASK(1,2,0,3) );
	return __vnmsubfp( A_ZXYW, B_YZXW, VectorMultiply(A_YZXW,B_ZXYW) );
}

static const VectorRegister  LC0 = {1.44268966f, -7.21165776e-1f, 4.78684813e-1f, -3.47305417e-1f};
static const VectorRegister  LC1 = {2.41873696e-1f, -1.37531206e-1f, 5.20646796e-2f, -9.31049418e-3f};
static const VectorRegister  EC0 = {1.0f, -6.93147182e-1f, 2.40226462e-1f, -5.55036440e-2f};
static const VectorRegister  EC1 = {9.61597636e-3f, -1.32823968e-3f, 1.47491097e-4f, -1.08635004e-5f};

/**
 * Calculates x raised to the power of y (component-wise).
 *
 * @param Base		Base vector
 * @param Exponent	Exponent vector
 * @return			VectorRegister( Base.x^Exponent.x, Base.y^Exponent.y, Base.z^Exponent.z, Base.w^Exponent.w )
 */
FORCEINLINE VectorRegister VectorPow( const VectorRegister& Base, const VectorRegister& Exponent )
{
	// IEEE 754 fp32:        1 sign-bit, 8 exponent bits, 23 mantissa bits
	// fp32 = s * m * 2^e
	// s    = +1 or -1       (if sign-bit is 0 or 1)
	// m    = 1.mantissa     (e.g. m = 1.11b = 1.75 if the mantissa is 110 0000 0000 0000 0000 0000)
	// e    = exponent - 127 (e.g. e = 2 if exponent is 129)

	VectorRegister AbsBase, Zero, One, SignMask;
	VectorRegister IEEE_ExponentMask, BaseMantissa, BaseMantissa2, BaseMantissa3, BaseMantissa4;
	VectorRegister L0, C0Y, L1, C1Y, C0Z, C0W, C1Z, C1W;
	VectorRegister L, E, E0, E1, Temp, Temp2, Temp3, Temp4;
	VectorRegister Reciprocal, Scale1, Scale2, Refine;
	VectorRegister OneInt, QNaN, ExponentInt, ExpSign, BaseSign, SignedZero;
	VectorRegister BaseIsNeg, ExpIsNeg, ExpIsInt, BaseIsZero, ExpIsZero, AnyIsZero, Result, Result2, Alt, UseResult2;

    Zero = VectorZero();
	One = VectorOne();
	IEEE_ExponentMask = __vslw( __vspltisw(-1), __vspltisw(-9) );	// Shift 0xffffffff left 23 bits (bitmask for IEEE sign-bit and exponent)
	BaseMantissa  = __vsel(Base, One, IEEE_ExponentMask);			// Create a float of the mantissa of "Base" [1.0,2.0)
	BaseMantissa  = VectorSubtract( BaseMantissa, One );				// BaseMantissa - 1
	BaseMantissa2 = VectorMultiply( BaseMantissa, BaseMantissa );	// (BaseMantissa - 1) ^ 2
	BaseMantissa3 = VectorMultiply( BaseMantissa, BaseMantissa2 );	// (BaseMantissa - 1) ^ 3
	BaseMantissa4 = VectorMultiply( BaseMantissa2, BaseMantissa2 );	// (BaseMantissa - 1) ^ 4

	L0  = VectorReplicate( LC0, 0 );
	C0Y = VectorReplicate( LC0, 1 );
	C0Z = VectorReplicate( LC0, 2 );
	C0W = VectorReplicate( LC0, 3 );
	L1  = VectorReplicate( LC1, 0 );
	C1Y = VectorReplicate( LC1, 1 );
	C1Z = VectorReplicate( LC1, 2 );
	C1W = VectorReplicate( LC1, 3 );

	L0 = VectorMultiplyAdd( BaseMantissa, C0Y, L0 );
	L1 = VectorMultiplyAdd( BaseMantissa, C1Y, L1 );
    L0 = VectorMultiplyAdd( BaseMantissa2, C0Z, L0 );
    L1 = VectorMultiplyAdd( BaseMantissa2, C1Z, L1 );
    L0 = VectorMultiplyAdd( BaseMantissa3, C0W, L0 );
    L1 = VectorMultiplyAdd( BaseMantissa3, C1W, L1 );
    L0 = VectorMultiplyAdd( BaseMantissa4, L1, L0 );

    SignMask = __vspltisw(-1);				// SignMask = (0xffffffff,0xffffffff,0xffffffff,0xffffffff)
    SignMask = __vslw(SignMask, SignMask);	// SignMask << 31	(0x80000000,0x80000000,0x80000000,0x80000000)
	AbsBase = __vandc( Base, SignMask );	// VectorAbs( Base )
    L = __vrfim( __vlogefp(AbsBase) );		// RoundToFloor( Log2( Abs( Base ) ) )

    Temp = VectorMultiply( BaseMantissa, Exponent );
    L    = VectorMultiply( L, Exponent );
	L    = VectorMultiplyAdd( Temp, L0, L);
    Temp = __vrfim( L );
    E    = __vexptefp( Temp );

    E0  = VectorReplicate( EC0, 0 );
    C0Y = VectorReplicate( EC0, 1 );
    C0Z = VectorReplicate( EC0, 2 );
    C0W = VectorReplicate( EC0, 3 );
    E1  = VectorReplicate( EC1, 0 );
    C1Y = VectorReplicate( EC1, 1 );
    C1Z = VectorReplicate( EC1, 2 );
    C1W = VectorReplicate( EC1, 3 );

    Temp  = VectorSubtract( L, Temp );
    Temp2 = VectorMultiply( Temp, Temp );
    Temp3 = VectorMultiply( Temp, Temp2 );
    Temp4 = VectorMultiply( Temp2, Temp2 );

	E0 = VectorMultiplyAdd( Temp, C0Y, E0 );
    E1 = VectorMultiplyAdd( Temp, C1Y, E1 );
    E0 = VectorMultiplyAdd( Temp2, C0Z, E0 );
    E1 = VectorMultiplyAdd( Temp2, C1Z, E1 );
    E0 = VectorMultiplyAdd( Temp3, C0W, E0 );
    E1 = VectorMultiplyAdd( Temp3, C1W, E1 );
    E0 = VectorMultiplyAdd( Temp4, E1, E0 );

	Reciprocal	= __vrefp( E0 );
	Scale1		= Scale2 = VectorReplicate( EC0, 0 );
	Scale1		= __vnmsubfp( E0, Reciprocal, Scale1 );
	Scale1		= VectorMultiplyAdd( Reciprocal, Scale1, Reciprocal );
	Scale2		= __vnmsubfp( E0, Scale1, Scale2 );
	Refine		= __vcmpeqfp( Scale1, Scale1 );
	Scale1		= VectorMultiplyAdd( Scale1, Scale2, Scale1 );
	Refine		= __vsel( Reciprocal, Scale1, Refine );
	Refine		= VectorMultiply( E, Refine );

	OneInt		= __vspltisw( 1 );
	ExponentInt	= __vctsxs( Exponent, 0 );
	QNaN		= __vsrw( IEEE_ExponentMask, OneInt );	// Generate NaN value (0 1111 1111 100 0000 0000 0000 0000 0000)
	BaseSign	= __vand( Base, SignMask );
	ExpSign		= __vslw( __vand(ExponentInt, OneInt), __vspltisw(-1) );
	ExpIsInt	= __vcmpeqfp( Exponent, __vrfiz( Exponent ) );
	BaseIsNeg	= __vcmpgtfp( Zero, Base );
	BaseIsZero	= __vcmpeqfp( Base, Zero );
	ExpIsZero	= __vcmpeqfp( Exponent, Zero );
	ExpIsNeg	= __vcmpgtfp( Zero, Exponent );
	BaseSign	= __vand( BaseSign, ExpSign );
	SignedZero	= __vor( Zero, BaseSign );

	Alt			= __vsel( QNaN, SignedZero, __vandc( BaseIsZero, ExpIsNeg ) );
	AnyIsZero	= __vor( BaseIsZero, ExpIsZero );
	Result2		= __vsel( Alt, One, ExpIsZero );
	UseResult2	= __vor( __vandc( BaseIsNeg, ExpIsInt ), AnyIsZero );

	Refine		= __vor( Refine, BaseSign );
	Result		= __vsel( Refine, Result2, UseResult2 );
	return Result;
}

/**
 * Multiplies two 4x4 matrices.
 *
 * @param Result	Pointer to where the result should be stored
 * @param Matrix1	Pointer to the first matrix
 * @param Matrix2	Pointer to the second matrix
 */
FORCEINLINE void VectorMatrixMultiply( void *Result, const void* Matrix1, const void* Matrix2 )
{
    VectorRegister	M0[4], M1[4], M2[4], M3[4];
    VectorRegister	M2T[4], P[4];
    VectorRegister	M00M02, M01M03, M10M12, M11M13,
					M20M22, M21M23, M30M32, M31M33;
	const VectorRegister *A = (const VectorRegister *) Matrix1;
	const VectorRegister *B = (const VectorRegister *) Matrix2;
	VectorRegister *R = (VectorRegister *) Result;

    P[0] = __vmrghw(B[0], B[2]);
    P[1] = __vmrghw(B[1], B[3]);
    P[2] = __vmrglw(B[0], B[2]);
    P[3] = __vmrglw(B[1], B[3]);

    M2T[0] = __vmrghw(P[0], P[1]);
    M2T[2] = __vmrghw(P[2], P[3]);
    M2T[1] = __vmrglw(P[0], P[1]);
    M2T[3] = __vmrglw(P[2], P[3]);

    M0[0] = VectorDot4(A[0], M2T[0]);
    M0[2] = VectorDot4(A[0], M2T[2]);
    M0[1] = VectorDot4(A[0], M2T[1]);
    M0[3] = VectorDot4(A[0], M2T[3]);

    M1[0] = VectorDot4(A[1], M2T[0]);
    M1[2] = VectorDot4(A[1], M2T[2]);
    M1[1] = VectorDot4(A[1], M2T[1]);
    M1[3] = VectorDot4(A[1], M2T[3]);

    M2[0] = VectorDot4(A[2], M2T[0]);
    M2[2] = VectorDot4(A[2], M2T[2]);
    M2[1] = VectorDot4(A[2], M2T[1]);
    M2[3] = VectorDot4(A[2], M2T[3]);

    M3[0] = VectorDot4(A[3], M2T[0]);
    M3[2] = VectorDot4(A[3], M2T[2]);
    M3[1] = VectorDot4(A[3], M2T[1]);
    M3[3] = VectorDot4(A[3], M2T[3]);

    M00M02 = __vmrghw(M0[0], M0[2]);
    M01M03 = __vmrghw(M0[1], M0[3]);
    M10M12 = __vmrghw(M1[0], M1[2]);
    M11M13 = __vmrghw(M1[1], M1[3]);
    M20M22 = __vmrghw(M2[0], M2[2]);
    M21M23 = __vmrghw(M2[1], M2[3]);
    M30M32 = __vmrghw(M3[0], M3[2]);
    M31M33 = __vmrghw(M3[1], M3[3]);

    R[0] = __vmrghw(M00M02, M01M03);
    R[1] = __vmrghw(M10M12, M11M13);
    R[2] = __vmrghw(M20M22, M21M23);
    R[3] = __vmrghw(M30M32, M31M33);
}

/**
 * Calculate the inverse of an FMatrix.
 *
 * @param DstMatrix		FMatrix pointer to where the result should be stored
 * @param SrcMatrix		FMatrix pointer to the Matrix to be inversed
 */
#define VectorMatrixInverse( DstMatrix, SrcMatrix ) do { XMVECTOR Det; *((XMMATRIX*)DstMatrix) = XMMatrixInverse( &Det, *((const XMMATRIX *)SrcMatrix) ); } while(0)

/**
 * Returns the minimum values of two vectors (component-wise).
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( min(Vec1.x,Vec2.x), min(Vec1.y,Vec2.y), min(Vec1.z,Vec2.z), min(Vec1.w,Vec2.w) )
 */
#define VectorMin( Vec1, Vec2 )					__vminfp( Vec1, Vec2 )

/**
 * Returns the maximum values of two vectors (component-wise).
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( max(Vec1.x,Vec2.x), max(Vec1.y,Vec2.y), max(Vec1.z,Vec2.z), max(Vec1.w,Vec2.w) )
 */
#define VectorMax( Vec1, Vec2 )					__vmaxfp( Vec1, Vec2 )

/**
 * Swizzles the 4 components of a vector and returns the result.
 *
 * @param Vec		Source vector
 * @param X			Index for which component to use for X (literal 0-3)
 * @param Y			Index for which component to use for Y (literal 0-3)
 * @param Z			Index for which component to use for Z (literal 0-3)
 * @param W			Index for which component to use for W (literal 0-3)
 * @return			The swizzled vector
 */
#define VectorSwizzle( Vec, X, Y, Z, W )		__vpermwi( Vec, SWIZZLEMASK(X,Y,Z,W) )

/**
 * Loads 4 BYTEs from unaligned memory and converts them into 4 FLOATs.
 * IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
 *
 * @param Ptr			Unaligned memory pointer to the 4 BYTEs.
 * @return				VectorRegister( FLOAT(Ptr[0]), FLOAT(Ptr[1]), FLOAT(Ptr[2]), FLOAT(Ptr[3]) )
 */
#define VectorLoadByte4( Ptr )					VectorUitof( __vperm(VectorLoadFloat1(Ptr), VectorZero(), XBOXALTIVEC_SWIZZLE_MASK_UNPACK) )

/**
 * Loads 4 BYTEs from unaligned memory and converts them into 4 FLOATs in reversed order.
 * IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
 *
 * @param Ptr			Unaligned memory pointer to the 4 BYTEs.
 * @return				VectorRegister( FLOAT(Ptr[3]), FLOAT(Ptr[2]), FLOAT(Ptr[1]), FLOAT(Ptr[0]) )
 */
#define VectorLoadByte4Reverse( Ptr )			VectorUitof( __vperm(VectorLoadFloat1(Ptr), VectorZero(), XBOXALTIVEC_SWIZZLE_MASK_UNPACK_REVERSE) )

/**
 * Converts the 4 FLOATs in the vector to 4 BYTEs, clamped to [0,255], and stores to unaligned memory.
 * IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
 *
 * @param Vec			Vector containing 4 FLOATs
 * @param Ptr			Unaligned memory pointer to store the 4 BYTEs.
 */
#define VectorStoreByte4( Vec, Ptr )			VectorStoreFloat1( VectorU16ToU8(VectorU32ToU16(VectorFtoui(Vec), VectorZero()), VectorZero()), Ptr )

/**
 * Returns non-zero if any element in Vec1 is greater than the corresponding element in Vec2, otherwise 0.
 *
 * @param Vec1			1st source vector
 * @param Vec2			2nd source vector
 * @return				Non-zero integer if (Vec1.x > Vec2.x) || (Vec1.y > Vec2.y) || (Vec1.z > Vec2.z) || (Vec1.w > Vec2.w)
 */
FORCEINLINE DWORD VectoryAnyGreaterThan(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	unsigned int CR;
	__vcmpgtfpR( Vec1, Vec2, &CR );
	return !(CR & 0x00000020);		// Bit 5 = 1 if all tests failed.
}

/**
 * Resets the floating point registers so that they can be used again.
 * Some intrinsics use these for MMX purposes (e.g. VectorLoadByte4 and VectorStoreByte4).
 */
#define VectorResetFloatRegisters()

/**
 * Returns the control register.
 *
 * @return			The DWORD control register
 */
#define VectorGetControlRegister()		0

/**
 * Sets the control register.
 *
 * @param ControlStatus		The DWORD control status value to set
 */
#define	VectorSetControlRegister(ControlStatus)

/**
 * Control status bit to round all floating point math results towards zero.
 */
#define VECTOR_ROUND_TOWARD_ZERO		0


// To be continued...


#endif
