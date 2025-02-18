/*=============================================================================
	UnMath.h: Unreal math routines
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Definitions.
-----------------------------------------------------------------------------*/

// Forward declarations.
class  FVector;
class  FVector4;
class  FPlane;
class  FRotator;
class  FGlobalMath;
class  FMatrix;
class  FQuat;
class  FTwoVectors;

// Constants.
#undef  PI
#define PI 					(3.1415926535897932)
#define SMALL_NUMBER		(1.e-8)
#define KINDA_SMALL_NUMBER	(1.e-4)
#define BIG_NUMBER			(3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536)

// Aux constants.
#define INV_PI			(0.31830988618)
#define HALF_PI			(1.57079632679)

// Magic numbers for numerical precision.
#define DELTA			(0.00001f)

#ifndef ENABLE_VECTORINTRINSICS
	/** Turning this on enables the cross-platform intrinsic code path in the engine. */
	#define ENABLE_VECTORINTRINSICS 1
#endif

// Platform specific vector intrinsics include.
#if ENABLE_VECTORINTRINSICS && PS3
	#include "UnMathPS3.h"
#elif ENABLE_VECTORINTRINSICS && XBOX
	#include "UnMathXe.h"
#elif ENABLE_VECTORINTRINSICS && __HAS_SSE__ && !__GNUC__  // !!! FIXME: need to clean out some Windows things in this header.
	#include "UnMathSSE.h"
#else
	#include "UnMathFpu.h"
#endif

/** Vector that represents (1/255,1/255,1/255,1/255) */
extern const VectorRegister VECTOR_INV_255;

/** 32 bit values where GBitFlag[x] == (1<<x) */
extern DWORD GBitFlag[32];

// Includes.
#include "Color.h"

/**
 * Convenience type for referring to axis by name instead of number.
 */
enum EAxis
{
	AXIS_None	= 0,
	AXIS_X		= 1,
	AXIS_Y		= 2,
	AXIS_Z		= 4,
	AXIS_XY		= AXIS_X|AXIS_Y,
	AXIS_XZ		= AXIS_X|AXIS_Z,
	AXIS_YZ		= AXIS_Y|AXIS_Z,
	AXIS_XYZ	= AXIS_X|AXIS_Y|AXIS_Z,
};

/*-----------------------------------------------------------------------------
	Global functions.
-----------------------------------------------------------------------------*/

/** Calculates the percentage along a line from MinValue to MaxValue that Value is. */
inline FLOAT GetRangePct(FLOAT MinValue,FLOAT MaxValue,FLOAT Value)
{
	return (Value - MinValue) / (MaxValue - MinValue);
}

/**
 * Helper function for rand implementations. Returns a random number in [0..A)
 */
inline INT RandHelper(INT A)
{
	// RAND_MAX+1 give interval [0..A) with even distribution.
	return A>0 ? appTrunc(appRand()/(FLOAT)((UINT)RAND_MAX+1) * A) : 0;
}

/** Util to generate a random number in a range. */
inline FLOAT RandRange( FLOAT InMin, FLOAT InMax )
{
	return InMin + (InMax - InMin) * appFrand();
}

/**
 * Snaps a value to the nearest grid multiple.
 */
inline FLOAT FSnap( FLOAT Location, FLOAT Grid )
{
	if( Grid==0.f )	return Location;
	else			return appFloor((Location + 0.5*Grid)/Grid)*Grid;
}

/**
 * Add to a word angle, constraining it within a min (not to cross)
 * and a max (not to cross).  Accounts for funkyness of word angles.
 * Assumes that angle is initially in the desired range.
 */
inline WORD FAddAngleConfined( INT Angle, INT Delta, INT MinThresh, INT MaxThresh )
{
	if( Delta < 0 )
	{
		if ( Delta<=-0x10000L || Delta<=-(INT)((WORD)(Angle-MinThresh)))
			return MinThresh;
	}
	else if( Delta > 0 )
	{
		if( Delta>=0x10000L || Delta>=(INT)((WORD)(MaxThresh-Angle)))
			return MaxThresh;
	}
	return (WORD)(Angle+Delta);
}

/**
 * Eliminates all fractional precision from an angle.
 */
INT ReduceAngle( INT Angle );


/**
 * Fast 32-bit float evaluations.
 * Warning: likely not portable, and useful on Pentium class processors only.
 */
inline UBOOL IsNegativeFloat(const FLOAT& F1)
{
	return ( (*(DWORD*)&F1) >= (DWORD)0x80000000 ); // Detects sign bit.
}

inline FLOAT RangeByteToFloat(BYTE A)
{
	if( A < 128 )
	{
		return (A - 128.f) / 128.f;
	}
	else
	{
		return (A - 128.f) / 127.f;
	}
}

inline BYTE FloatToRangeByte(FLOAT A)
{
	return appRound( 128.f + appFloatSelect( A, Min( 127 * A, 127.f ), Max( 128 * A, -128.f ) ) );
}

/** A logical exclusive or function. */
inline UBOOL XOR(UBOOL A, UBOOL B)
{
	return (A && !B) || (!A && B);
}

inline DWORD FLOAT_TO_DWORD(FLOAT Float)
{
	return *reinterpret_cast<DWORD*>(&Float);
}

/**
 *	Checks if two floating point numbers are nearly equal.
 *	@param A				First number to compare
 *	@param B				Second number to compare
 *	@param ErrorTolerance	Maximum allowed difference for considering them as 'nearly equal'
 *	@return					TRUE if A and B are nearly equal
 **/
FORCEINLINE UBOOL appIsNearlyEqual(FLOAT A, FLOAT B, FLOAT ErrorTolerance = SMALL_NUMBER)
{
	return Abs<FLOAT>( A - B ) < ErrorTolerance;
}

/**
 *	Checks if two floating point numbers are nearly equal.
 *	@param A				First number to compare
 *	@param B				Second number to compare
 *	@param ErrorTolerance	Maximum allowed difference for considering them as 'nearly equal'
 *	@return					TRUE if A and B are nearly equal
 **/
FORCEINLINE UBOOL appIsNearlyEqual(DOUBLE A, DOUBLE B, DOUBLE ErrorTolerance = SMALL_NUMBER)
{
	return Abs<DOUBLE>( A - B ) < ErrorTolerance;
}

/**
 *	Checks if a floating point number is nearly zero.
 *	@param Value			Number to compare
 *	@param ErrorTolerance	Maximum allowed difference for considering Value as 'nearly zero'
 *	@return					TRUE if Value is nearly zero
 **/
FORCEINLINE UBOOL appIsNearlyZero(FLOAT Value, FLOAT ErrorTolerance = SMALL_NUMBER)
{
	return Abs<FLOAT>( Value ) < ErrorTolerance;
}

/**
 *	Checks if a floating point number is nearly zero.
 *	@param Value			Number to compare
 *	@param ErrorTolerance	Maximum allowed difference for considering Value as 'nearly zero'
 *	@return					TRUE if Value is nearly zero
 **/
FORCEINLINE UBOOL appIsNearlyZero(DOUBLE Value, DOUBLE ErrorTolerance = SMALL_NUMBER)
{
	return Abs<DOUBLE>( Value ) < ErrorTolerance;
}


/*-----------------------------------------------------------------------------
	FIntPoint.
-----------------------------------------------------------------------------*/

struct FIntPoint
{
	INT X, Y;
	FIntPoint()
	{}
	FIntPoint( INT InX, INT InY )
		:	X( InX )
		,	Y( InY )
	{}
	explicit FORCEINLINE FIntPoint(EEventParm)
	: X(0), Y(0)
	{}
	static FIntPoint ZeroValue()
	{
		return FIntPoint(0,0);
	}
	static FIntPoint NoneValue()
	{
		return FIntPoint(INDEX_NONE,INDEX_NONE);
	}
	const INT& operator()( INT i ) const
	{
		return (&X)[i];
	}
	INT& operator()( INT i )
	{
		return (&X)[i];
	}
	static INT Num()
	{
		return 2;
	}
	UBOOL operator==( const FIntPoint& Other ) const
	{
		return X==Other.X && Y==Other.Y;
	}
	UBOOL operator!=( const FIntPoint& Other ) const
	{
		return X!=Other.X || Y!=Other.Y;
	}
	FIntPoint& operator*=( INT Scale )
	{
		X *= Scale;
		Y *= Scale;
		return *this;
	}
	FIntPoint& operator+=( const FIntPoint& Other )
	{
		X += Other.X;
		Y += Other.Y;
		return *this;
	}
	FIntPoint& operator-=( const FIntPoint& Other )
	{
		X -= Other.X;
		Y -= Other.Y;
		return *this;
	}
	FIntPoint operator*( INT Scale ) const
	{
		return FIntPoint(*this) *= Scale;
	}
	FIntPoint operator+( const FIntPoint& Other ) const
	{
		return FIntPoint(*this) += Other;
	}
	FIntPoint operator-( const FIntPoint& Other ) const
	{
		return FIntPoint(*this) -= Other;
	}
	INT Size() const
	{
		return INT( appSqrt( FLOAT(X*X + Y*Y) ) );
	}
};

/*-----------------------------------------------------------------------------
	FIntRect.
-----------------------------------------------------------------------------*/

struct FIntRect
{
	FIntPoint Min, Max;
	FIntRect()
	{}
	FIntRect( INT X0, INT Y0, INT X1, INT Y1 )
		:	Min( X0, Y0 )
		,	Max( X1, Y1 )
	{}
	FIntRect( FIntPoint InMin, FIntPoint InMax )
		:	Min( InMin )
		,	Max( InMax )
	{}
	explicit FIntRect(EEventParm)
	: Min(EC_EventParm), Max(EC_EventParm)
	{}
	const FIntPoint& operator()( INT i ) const
	{
		return (&Min)[i];
	}
	FIntPoint& operator()( INT i )
	{
		return (&Min)[i];
	}
	static INT Num()
	{
		return 2;
	}
	UBOOL operator==( const FIntRect& Other ) const
	{
		return Min==Other.Min && Max==Other.Max;
	}
	UBOOL operator!=( const FIntRect& Other ) const
	{
		return Min!=Other.Min || Max!=Other.Max;
	}
	FIntRect Right( INT InWidth ) const
	{
		return FIntRect( ::Max(Min.X,Max.X-InWidth), Min.Y, Max.X, Max.Y );
	}
	FIntRect Bottom( INT InHeight ) const
	{
		return FIntRect( Min.X, ::Max(Min.Y,Max.Y-InHeight), Max.X, Max.Y );
	}
	FIntPoint Size() const
	{
		return FIntPoint( Max.X-Min.X, Max.Y-Min.Y );
	}
	INT Width() const
	{
		return Max.X-Min.X;
	}
	INT Height() const
	{
		return Max.Y-Min.Y;
	}
	FIntRect& operator*=( INT Scale )
	{
		Min *= Scale;
		Max *= Scale;
		return *this;
	}
	FIntRect& operator+=( const FIntPoint& P )
	{
		Min += P;
		Max += P;
		return *this;
	}
	FIntRect& operator-=( const FIntPoint& P )
	{
		Min -= P;
		Max -= P;
		return *this;
	}
	FIntRect operator*( INT Scale ) const
	{
		return FIntRect( Min*Scale, Max*Scale );
	}
	FIntRect operator+( const FIntPoint& P ) const
	{
		return FIntRect( Min+P, Max+P );
	}
	FIntRect operator-( const FIntPoint& P ) const
	{
		return FIntRect( Min-P, Max-P );
	}
	FIntRect operator+( const FIntRect& R ) const
	{
		return FIntRect( Min+R.Min, Max+R.Max );
	}
	FIntRect operator-( const FIntRect& R ) const
	{
		return FIntRect( Min-R.Min, Max-R.Max );
	}
	FIntRect Inner( FIntPoint P ) const
	{
		return FIntRect( Min+P, Max-P );
	}
	UBOOL Contains( FIntPoint P ) const
	{
		return P.X>=Min.X && P.X<Max.X && P.Y>=Min.Y && P.Y<Max.Y;
	}
	INT Area() const
	{
		return (Max.X - Min.X) * (Max.Y - Min.Y);
	}
	void GetCenterAndExtents(FIntPoint& Center, FIntPoint& Extent) const
	{
		Extent.X = (Max.X - Min.X) / 2;
		Extent.Y = (Max.Y - Min.Y) / 2;

		Center.X = Min.X + Extent.X;
		Center.Y = Min.Y + Extent.Y;
	}
	void Clip( const FIntRect& R )
	{
		Min.X = ::Max<INT>(Min.X, R.Min.X);
		Min.Y = ::Max<INT>(Min.Y, R.Min.Y);
		Max.X = ::Min<INT>(Max.X, R.Max.X);
		Max.Y = ::Min<INT>(Max.Y, R.Max.Y);

		// Adjust to zero area if the rects don't overlap.
		Max.X = ::Max<INT>(Min.X, Max.X);
		Max.Y = ::Max<INT>(Min.Y, Max.Y);
	}
};

/**
 * A 2x1 of FLOATs.
 */
struct FVector2D 
{
	FLOAT	X,
			Y;

	// Constructors.
	FORCEINLINE FVector2D()
	{}
	FORCEINLINE FVector2D(FLOAT InX,FLOAT InY)
	:	X(InX), Y(InY)
	{}
	FORCEINLINE FVector2D(FIntPoint InPos)
	{
		X = (FLOAT)InPos.X;
		Y = (FLOAT)InPos.Y;
	}
	explicit FORCEINLINE FVector2D(EEventParm)
	: X(0), Y(0)
	{
	}
	explicit FORCEINLINE FVector2D( const FVector& V );

	// Binary math operators.
	FORCEINLINE FVector2D operator+( const FVector2D& V ) const
	{
		return FVector2D( X + V.X, Y + V.Y );
	}
	FORCEINLINE FVector2D operator-( const FVector2D& V ) const
	{
		return FVector2D( X - V.X, Y - V.Y );
	}
	FORCEINLINE FVector2D operator*( FLOAT Scale ) const
	{
		return FVector2D( X * Scale, Y * Scale );
	}
	FVector2D operator/( FLOAT Scale ) const
	{
		const FLOAT RScale = 1.f/Scale;
		return FVector2D( X * RScale, Y * RScale );
	}
	FORCEINLINE FVector2D operator*( const FVector2D& V ) const
	{
		return FVector2D( X * V.X, Y * V.Y );
	}
	FORCEINLINE FLOAT operator|( const FVector2D& V) const
	{
		return X*V.X + Y*V.Y;
	}
	FORCEINLINE FLOAT operator^( const FVector2D& V) const
	{
		return X*V.Y - Y*V.X;
	}

	// Binary comparison operators.
	UBOOL operator==( const FVector2D& V ) const
	{
		return X==V.X && Y==V.Y;
	}
	UBOOL operator!=( const FVector2D& V ) const
	{
		return X!=V.X || Y!=V.Y;
	}
	UBOOL operator<( const FVector2D& Other ) const
	{
		return X < Other.X && Y < Other.Y;
	}
	UBOOL operator>( const FVector2D& Other ) const
	{
		return X > Other.X && Y > Other.Y;
	}
	UBOOL operator<=( const FVector2D& Other ) const
	{
		return X <= Other.X && Y <= Other.Y;
	}
	UBOOL operator>=( const FVector2D& Other ) const
	{
		return X >= Other.X && Y >= Other.Y;
	}
	// Error-tolerant comparison.
	UBOOL Equals(const FVector2D& V, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs(X-V.X) < Tolerance && Abs(Y-V.Y) < Tolerance;
	}

	// Unary operators.
	FORCEINLINE FVector2D operator-() const
	{
		return FVector2D( -X, -Y );
	}

	// Assignment operators.
	FORCEINLINE FVector2D operator+=( const FVector2D& V )
	{
		X += V.X; Y += V.Y;
		return *this;
	}
	FORCEINLINE FVector2D operator-=( const FVector2D& V )
	{
		X -= V.X; Y -= V.Y;
		return *this;
	}
	FORCEINLINE FVector2D operator*=( FLOAT Scale )
	{
		X *= Scale; Y *= Scale;
		return *this;
	}
	FVector2D operator/=( FLOAT V )
	{
		const FLOAT RV = 1.f/V;
		X *= RV; Y *= RV;
		return *this;
	}
	FVector2D operator*=( const FVector2D& V )
	{
		X *= V.X; Y *= V.Y;
		return *this;
	}
	FVector2D operator/=( const FVector2D& V )
	{
		X /= V.X; Y /= V.Y;
		return *this;
	}
    FLOAT& operator[]( INT i )
	{
		check(i>-1);
		check(i<2);
		if( i == 0 )	return X;
		else			return Y;
	}
	FLOAT operator[]( INT i ) const
	{
		check(i>-1);
		check(i<2);
		return ((i == 0) ? X : Y);
	}
	// Simple functions.
	void Set( FLOAT InX, FLOAT InY )
	{
		X = InX;
		Y = InY;
	}
	FLOAT GetMax() const
	{
		return Max(X,Y);
	}
	FLOAT GetAbsMax() const
	{
		return Max(Abs(X),Abs(Y));
	}
	FLOAT GetMin() const
	{
		return Min(X,Y);
	}
	FLOAT Size() const
	{
		return appSqrt( X*X + Y*Y );
	}
	FLOAT SizeSquared() const
	{
		return X*X + Y*Y;
	}

	FVector2D SafeNormal(FLOAT Tolerance=SMALL_NUMBER) const
	{	
		const FLOAT SquareSum = X*X + Y*Y;
		if( SquareSum > Tolerance )
		{
			const FLOAT Scale = appInvSqrt(SquareSum);
			return FVector2D(X*Scale, Y*Scale);
		}
		return FVector2D(0.f, 0.f);
	}

	void Normalize(FLOAT Tolerance=SMALL_NUMBER)
	{
		const FLOAT SquareSum = X*X + Y*Y;
		if( SquareSum > Tolerance )
		{
			const FLOAT Scale = appInvSqrt(SquareSum);
			X *= Scale;
			Y *= Scale;
			return;
		}
		X = 0.0f;
		Y = 0.0f;
	}

	int IsNearlyZero(FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return	Abs(X)<Tolerance
			&&	Abs(Y)<Tolerance;
	}
	UBOOL IsZero() const
	{
		return X==0.f && Y==0.f;
	}
	FLOAT& Component( INT Index )
	{
		return (&X)[Index];
	}

	FIntPoint IntPoint() const
	{
		return FIntPoint( appRound(X), appRound(Y) );
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("X=%3.3f Y=%3.3f"), X, Y);
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FVector2D& V )
	{
		// @warning BulkSerialize: FVector2D is serialized as memory dump
		// See TArray::BulkSerialize for detailed description of implied limitations.
		return Ar << V.X << V.Y;
	}
};

inline FVector2D operator*( FLOAT Scale, const FVector2D& V )
{
	return V.operator*( Scale );
}

/*-----------------------------------------------------------------------------
	FVector.
-----------------------------------------------------------------------------*/

/**
 * A 3x1 of FLOATs.
 */
class FVector 
{
public:
	// Variables.
	FLOAT X,Y,Z;

	// Constructors.
	FORCEINLINE FVector()
	{}
	explicit FORCEINLINE FVector(FLOAT InF)
	: X(InF), Y(InF), Z(InF)
	{}
	FORCEINLINE FVector( FLOAT InX, FLOAT InY, FLOAT InZ )
	:	X(InX), Y(InY), Z(InZ)
	{}
	explicit FORCEINLINE FVector( const FVector2D V, FLOAT InZ );
	FORCEINLINE FVector( const FVector4& V );
	FVector( FIntPoint A )
	{
		X = A.X - 0.5f;
		Y = A.Y - 0.5f;
		Z = 0.0f;
	}
	explicit FORCEINLINE FVector(EEventParm)
	: X(0.0f), Y(0.0f), Z(0.0f)
	{}

	// Binary math operators.
	FORCEINLINE FVector operator^( const FVector& V ) const
	{
		return FVector
		(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X
		);
	}
	FORCEINLINE FLOAT operator|( const FVector& V ) const
	{
		return X*V.X + Y*V.Y + Z*V.Z;
	}
	FORCEINLINE FVector operator+( const FVector& V ) const
	{
		return FVector( X + V.X, Y + V.Y, Z + V.Z );
	}
	FORCEINLINE FVector operator-( const FVector& V ) const
	{
		return FVector( X - V.X, Y - V.Y, Z - V.Z );
	}
	FORCEINLINE FVector operator*( FLOAT Scale ) const
	{
		return FVector( X * Scale, Y * Scale, Z * Scale );
	}
	FVector operator/( FLOAT Scale ) const
	{
		const FLOAT RScale = 1.f/Scale;
		return FVector( X * RScale, Y * RScale, Z * RScale );
	}
	FORCEINLINE FVector operator*( const FVector& V ) const
	{
		return FVector( X * V.X, Y * V.Y, Z * V.Z );
	}
	FORCEINLINE FVector operator/( const FVector& V ) const
	{
		return FVector( X / V.X, Y / V.Y, Z / V.Z );
	}

	// Binary comparison operators.
	UBOOL operator==( const FVector& V ) const
	{
		return X==V.X && Y==V.Y && Z==V.Z;
	}
	UBOOL operator!=( const FVector& V ) const
	{
		return X!=V.X || Y!=V.Y || Z!=V.Z;
	}

	// Error-tolerant comparison.
	UBOOL Equals(const FVector& V, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs(X-V.X) < Tolerance && Abs(Y-V.Y) < Tolerance && Abs(Z-V.Z) < Tolerance;
	}

	/** Checks whether all components of the vector are the same, within a tolerance. */
	UBOOL AllComponentsEqual(FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs( X - Y ) < Tolerance && Abs( X - Z ) < Tolerance && Abs( Y - Z ) < Tolerance;
	}

	// Unary operators.
	FORCEINLINE FVector operator-() const
	{
		return FVector( -X, -Y, -Z );
	}

	// Assignment operators.
	FORCEINLINE FVector operator+=( const FVector& V )
	{
		X += V.X; Y += V.Y; Z += V.Z;
		return *this;
	}
	FORCEINLINE FVector operator-=( const FVector& V )
	{
		X -= V.X; Y -= V.Y; Z -= V.Z;
		return *this;
	}
	FORCEINLINE FVector operator*=( FLOAT Scale )
	{
		X *= Scale; Y *= Scale; Z *= Scale;
		return *this;
	}
	FVector operator/=( FLOAT V )
	{
		const FLOAT RV = 1.f/V;
		X *= RV; Y *= RV; Z *= RV;
		return *this;
	}
	FVector operator*=( const FVector& V )
	{
		X *= V.X; Y *= V.Y; Z *= V.Z;
		return *this;
	}
	FVector operator/=( const FVector& V )
	{
		X /= V.X; Y /= V.Y; Z /= V.Z;
		return *this;
	}
    FLOAT& operator[]( INT i )
	{
		check(i>-1);
		check(i<3);
		if( i == 0 )		return X;
		else if( i == 1)	return Y;
		else				return Z;
	}
	FLOAT operator[]( INT i )const
	{
		check(i>-1);
		check(i<3);
		if( i == 0 )		return X;
		else if( i == 1)	return Y;
		else				return Z;
	}
	// Simple functions.
	void Set( FLOAT InX, FLOAT InY, FLOAT InZ )
	{
		X = InX;
		Y = InY;
		Z = InZ;
	}
	FLOAT GetMax() const
	{
		return Max(Max(X,Y),Z);
	}
	FLOAT GetAbsMax() const
	{
		return Max(Max(Abs(X),Abs(Y)),Abs(Z));
	}
	FLOAT GetMin() const
	{
		return Min(Min(X,Y),Z);
	}
	FLOAT Size() const
	{
		return appSqrt( X*X + Y*Y + Z*Z );
	}
	FLOAT SizeSquared() const
	{
		return X*X + Y*Y + Z*Z;
	}
	FLOAT Size2D() const 
	{
		return appSqrt( X*X + Y*Y );
	}
	FLOAT SizeSquared2D() const 
	{
		return X*X + Y*Y;
	}
	int IsNearlyZero(FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return
				Abs(X)<Tolerance
			&&	Abs(Y)<Tolerance
			&&	Abs(Z)<Tolerance;
	}
	UBOOL IsZero() const
	{
		return X==0.f && Y==0.f && Z==0.f;
	}

	UBOOL Normalize(FLOAT Tolerance=SMALL_NUMBER)
	{
		const FLOAT SquareSum = X*X + Y*Y + Z*Z;
		if( SquareSum > Tolerance )
		{
			const FLOAT Scale = appInvSqrt(SquareSum);
			X *= Scale; Y *= Scale; Z *= Scale;
			return TRUE;
		}
		return FALSE;
	}

	FVector Projection() const
	{
		const FLOAT RZ = 1.f/Z;
		return FVector( X*RZ, Y*RZ, 1 );
	}
	FORCEINLINE FVector UnsafeNormal() const
	{
		const FLOAT Scale = appInvSqrt(X*X+Y*Y+Z*Z);
		return FVector( X*Scale, Y*Scale, Z*Scale );
	}
	FVector GridSnap( const FLOAT& GridSz ) const
	{
		return FVector( FSnap(X, GridSz),FSnap(Y, GridSz),FSnap(Z, GridSz) );
	}
	FVector BoundToCube( FLOAT Radius ) const
	{
		return FVector
		(
			Clamp(X,-Radius,Radius),
			Clamp(Y,-Radius,Radius),
			Clamp(Z,-Radius,Radius)
		);
	}
	void AddBounded( const FVector& V, FLOAT Radius=MAXSWORD )
	{
		*this = (*this + V).BoundToCube(Radius);
	}
	FLOAT& Component( INT Index )
	{
		return (&X)[Index];
	}

	/**
	 * Returns TRUE if X == Y == Z within the specified tolerance.
	 */
	UBOOL IsUniform(FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return (Abs(X-Y) < Tolerance) && (Abs(Y-Z) < Tolerance);
	}

	/**
	 * Mirror a vector about a normal vector.
	 */
	FVector MirrorByVector( const FVector& MirrorNormal ) const
	{
		return *this - MirrorNormal * (2.f * (*this | MirrorNormal));
	}

	/**
	 * Mirrors a vector about a plane.
	 */
	FVector MirrorByPlane( const FPlane& Plane ) const;

	/**
	 * Rotates around Axis (assumes Axis.Size() == 1).
	 */
	FVector RotateAngleAxis( const INT Angle, const FVector& Axis ) const;

	FORCEINLINE FVector SafeNormal(FLOAT Tolerance=SMALL_NUMBER) const
	{
		const FLOAT SquareSum = X*X + Y*Y + Z*Z;

		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if( SquareSum == 1.f )
		{
			return *this;
		}		
		else if( SquareSum < Tolerance )
		{
			return FVector(0.f);
		}
		const FLOAT Scale = appInvSqrt(SquareSum);
		return FVector(X*Scale, Y*Scale, Z*Scale);
	}

	FORCEINLINE FVector SafeNormal2D(FLOAT Tolerance=SMALL_NUMBER) const
	{
		const FLOAT SquareSum = X*X + Y*Y;

		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if( SquareSum == 1.f )
		{
			if( Z == 0.f )
			{
				return *this;
			}
			else
			{
				return FVector(X, Y, 0.f);
			}
		}
		else if( SquareSum < Tolerance )
		{
			return FVector(0.f);
		}

		const FLOAT Scale = appInvSqrt(SquareSum);
		return FVector(X*Scale, Y*Scale, 0.f);
	}

	/**
	 * Projects this vector onto the input vector.  Does not assume A is unnormalized.
	 */
	FORCEINLINE FVector ProjectOnTo( const FVector& A ) const 
	{ 
		return (A * ((*this | A) / (A | A))); 
	}

	/**
	 * Return the FRotator corresponding to the direction that the vector
	 * is pointing in.  Sets Yaw and Pitch to the proper numbers, and sets
	 * roll to zero because the roll can't be determined from a vector.
	 */
	FRotator Rotation() const;

	/**
	 * Find good arbitrary axis vectors to represent U and V axes of a plane,
	 * given just the normal.
	 */
	void FindBestAxisVectors( FVector& Axis1, FVector& Axis2 ) const;

	/** When this vector contains Euler angles (degrees), ensure that angles are between +/-180 */
	void UnwindEuler();

	/** Utility to check if there are any NaNs in this vector. */
	UBOOL ContainsNaN() const
	{
		return (appIsNaN(X) || !appIsFinite(X) || 
				appIsNaN(Y) || !appIsFinite(Y) ||
				appIsNaN(Z) || !appIsFinite(Z));
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), X, Y, Z);
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FVector& V )
	{
		// @warning BulkSerialize: FVector is serialized as memory dump
		// See TArray::BulkSerialize for detailed description of implied limitations.
		return Ar << V.X << V.Y << V.Z;
	}
};

/** Clamps a vector to not be longer than MaxLength. */
FVector ClampLength( const FVector& V, FLOAT MaxLength);

inline FVector operator*( FLOAT Scale, const FVector& V )
{
	return V.operator*( Scale );
}

/**
 * Create an orthonormal basis from a basis with at least two orthogonal vectors.
 * It may change the directions of the X and Y axes to make the basis orthogonal,
 * but it won't change the direction of the Z axis.
 * All axes will be normalized.
 * @param XAxis - The input basis' XAxis, and upon return the orthonormal basis' XAxis.
 * @param YAxis - The input basis' YAxis, and upon return the orthonormal basis' YAxis.
 * @param ZAxis - The input basis' ZAxis, and upon return the orthonormal basis' ZAxis.
 */
extern void CreateOrthonormalBasis(FVector& XAxis,FVector& YAxis,FVector& ZAxis);

/**
 * Creates a hash value from a FVector. Uses pointers to the elements to
 * bypass any type conversion. This is a simple hash that just ORs the
 * raw 32bit data together
 *
 * @param Vector the vector to create a hash value for
 *
 * @return The hash value from the components
 */
inline DWORD GetTypeHash(const FVector& Vector)
{
	// Note: this assumes there's no padding in FVector that could contain uncompared data.
	return appMemCrc(&Vector,sizeof(FVector));
}

#if __INTEL_BYTE_ORDER__
	#define INTEL_ORDER_VECTOR(x) (x)
#else
	static inline FVector INTEL_ORDER_VECTOR(FVector v)
	{
		return FVector(INTEL_ORDERF(v.X), INTEL_ORDERF(v.Y), INTEL_ORDERF(v.Z));
	}
#endif

/**
 * A 4D homogeneous vector, 4x1 FLOATs, 16-byte aligned.
 */
MS_ALIGN(16) class FVector4
{
public:
	// Variables.
	FLOAT X, Y, Z, W;

	FVector4(const FVector& InVector,FLOAT InW = 1.0f):
		X(InVector.X), Y(InVector.Y), Z(InVector.Z), W(InW)
	{}
	explicit FVector4(FLOAT InX = 0.0f,FLOAT InY = 0.0f,FLOAT InZ = 0.0f,FLOAT InW = 1.0f):
		X(InX), Y(InY), Z(InZ), W(InW)
	{}
	explicit FORCEINLINE FVector4(EEventParm)
	: X(0.f), Y(0.f), Z(0.f), W(0.f)
	{
	}

	/** Compoment Accessors */
	FORCEINLINE FLOAT & operator[]( INT ComponentIndex )
	{
		return (&X)[ ComponentIndex ];
	}
	FORCEINLINE FLOAT operator[]( INT ComponentIndex ) const
	{
		return (&X)[ ComponentIndex ];
	}
	FORCEINLINE void Set( FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InW )
	{
		X = InX;
		Y = InY;
		Z = InZ;
		W = InW;
	}

	// Unary operators.
	FORCEINLINE FVector4 operator-() const
	{
		return FVector4( -X, -Y, -Z, -W );
	}

	// Binary math operators.
	FORCEINLINE FVector4 operator+( const FVector4& V ) const
	{
		return FVector4( X + V.X, Y + V.Y, Z + V.Z, W + V.W );
	}
	FORCEINLINE FVector4 operator-( const FVector4& V ) const
	{
		return FVector4( X - V.X, Y - V.Y, Z - V.Z, W - V.W );
	}
	FORCEINLINE FVector4 operator*( FLOAT Scale ) const
	{
		return FVector4( X * Scale, Y * Scale, Z * Scale, W * Scale );
	}
	FVector4 operator/( FLOAT Scale ) const
	{
		const FLOAT RScale = 1.f/Scale;
		return FVector4( X * RScale, Y * RScale, Z * RScale, W * RScale );
	}
	FORCEINLINE FVector4 operator*( const FVector4& V ) const
	{
		return FVector4( X * V.X, Y * V.Y, Z * V.Z, W * V.W );
	}

	// Simple functions.
	FLOAT& Component( INT Index )
	{
		return (&X)[Index];
	}
	friend FORCEINLINE FLOAT Dot3( const FVector4& V1, const FVector4& V2 )
	{
		return V1.X*V2.X + V1.Y*V2.Y + V1.Z*V2.Z;
	}
	friend FORCEINLINE FLOAT Dot4( const FVector4& V1, const FVector4& V2 )
	{
		return V1.X*V2.X + V1.Y*V2.Y + V1.Z*V2.Z + V1.W*V2.W;
	}
	friend FORCEINLINE FVector4 operator*( FLOAT Scale, const FVector4& V )
	{
		return V.operator*( Scale );
	}

	/** Returns a normalized 3D FVector */
	FORCEINLINE FVector4 SafeNormal(FLOAT Tolerance=SMALL_NUMBER) const
	{
		const FLOAT SquareSum = X*X + Y*Y + Z*Z;
		if( SquareSum > Tolerance )
		{
			const FLOAT Scale = appInvSqrt(SquareSum);
			return FVector4(X*Scale, Y*Scale, Z*Scale, 0.0f);
		}
		return FVector4(0.f);
	}

	/**
	 * Return the FRotator corresponding to the direction that the vector
	 * is pointing in.  Sets Yaw and Pitch to the proper numbers, and sets
	 * roll to zero because the roll can't be determined from a vector.
	 */
	FRotator Rotation() const;

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FVector4& V )
	{
		return Ar << V.X << V.Y << V.Z << V.W;
	}
} GCC_ALIGN(16);


class FTwoVectors
{
public:
	FVector	v1;
	FVector	v2;

	FORCEINLINE	FTwoVectors() :
		v1(0.0f),
		v2(0.0f)
	{
	}
	FORCEINLINE	FTwoVectors(FVector In1, FVector In2) :
		v1(In1),
		v2(In2)
	{
	}
	explicit FORCEINLINE FTwoVectors(EEventParm)
	: v1(EC_EventParm), v2(EC_EventParm)
	{
	}
	// Binary math operators.
	FORCEINLINE FTwoVectors operator+(const FTwoVectors& V) const
	{
		return FTwoVectors(
			FVector(v1 + V.v1),
			FVector(v2 + V.v2)
			);
	}
	FORCEINLINE FTwoVectors operator-(const FTwoVectors& V) const
	{
		return FTwoVectors( 
			FVector(v1 - V.v1),
			FVector(v2 - V.v2)
			);
	}
	FORCEINLINE FTwoVectors operator*(FLOAT Scale) const
	{
		return FTwoVectors(
			FVector(v1 * Scale),
			FVector(v2 * Scale)
			);
	}
	FTwoVectors operator/(FLOAT Scale) const
	{
		const FLOAT RScale = 1.f / Scale;
		return FTwoVectors(
			FVector(v1 * RScale), 
			FVector(v2 * RScale)
			);
	}
	FORCEINLINE FTwoVectors operator*(const FTwoVectors& V) const
	{
		return FTwoVectors(
			FVector(v1 * V.v1),
			FVector(v2 * V.v2)
			);
	}
	FORCEINLINE FTwoVectors operator/(const FTwoVectors& V) const
	{
		return FTwoVectors(
			FVector(v1 / V.v1),
			FVector(v2 / V.v2)
			);
	}

	// Binary comparison operators.
	UBOOL operator==(const FTwoVectors& V) const
	{
		return ((v1 == V.v1) && (v2 == V.v2));
	}
	UBOOL operator!=(const FTwoVectors& V) const
	{
		return ((v1 != V.v1) || (v2 != V.v2));
	}

	// Error-tolerant comparison.
	UBOOL Equals(const FTwoVectors& V, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return v1.Equals( V.v1, Tolerance ) && v2.Equals( V.v2, Tolerance );
	}

	// Unary operators.
	FORCEINLINE FTwoVectors operator-() const
	{
		return FTwoVectors(
			FVector(-v1),
			FVector(-v2)
			);
	}

	// Assignment operators.
	FORCEINLINE FTwoVectors operator+=(const FTwoVectors& V)
	{
		v1 += V.v1;
		v2 += V.v2;
		return *this;
	}
	FORCEINLINE FTwoVectors operator-=(const FTwoVectors& V)
	{
		v1 -= V.v1;
		v2 -= V.v2;
		return *this;
	}
	FORCEINLINE FTwoVectors operator*=(FLOAT Scale)
	{
		v1 *= Scale;
		v2 *= Scale;
		return *this;
	}
	FTwoVectors operator/=(FLOAT V)
	{
		const FLOAT RV = 1.f/V;
		v1 *= RV;
		v2 *= RV;
		return *this;
	}
	FTwoVectors operator*=(const FTwoVectors& V)
	{
		v1 *= V.v1;
		v2 *= V.v2;
		return *this;
	}
	FTwoVectors operator/=(const FTwoVectors& V)
	{
		v1 /= V.v1;
		v2 /= V.v2;
		return *this;
	}

	FLOAT GetMax() const
	{
		const FLOAT MaxMax = Max(Max(v1.X, v1.Y), v1.Z);
		const FLOAT MaxMin = Max(Max(v2.X, v2.Y), v2.Z);
		return Max(MaxMax, MaxMin);
	}

	FLOAT GetMin() const
	{
		const FLOAT MinMax = Min(Min(v1.X, v1.Y), v1.Z);
		const FLOAT MinMin = Min(Min(v2.X, v2.Y), v2.Z);
		return Min(MinMax, MinMin);
	}

    FLOAT& operator[](INT i)
	{
		check(i>-1);
		check(i<6);
		
		switch (i)
		{
		case 0:		return v1.X;
		case 1:		return v2.X;
		case 2:		return v1.Y;
		case 3:		return v2.Y;
		case 4:		return v1.Z;
		default:	return v2.Z;
		}
	}
};

inline FTwoVectors operator*(FLOAT Scale, const FTwoVectors& V)
{
	return V.operator*( Scale );
}

/*-----------------------------------------------------------------------------
	FEdge.
-----------------------------------------------------------------------------*/

class FEdge
{
public:
	// Constructors.
	FEdge()
	{}
	FEdge( FVector v1, FVector v2)
	{
		Vertex[0] = v1;
		Vertex[1] = v2;
	}

	FVector Vertex[2];

	UBOOL operator==( const FEdge& E ) const
	{
		return ( (E.Vertex[0] == Vertex[0] && E.Vertex[1] == Vertex[1]) 
			|| (E.Vertex[0] == Vertex[1] && E.Vertex[1] == Vertex[0]) );
	}
};

/*-----------------------------------------------------------------------------
	FPlane.
	Stores the coeffecients as Ax+By+Cz=D.
	Note that this is different than many other Plane classes that use Ax+By+Cz+D=0.
-----------------------------------------------------------------------------*/

MS_ALIGN(16) class FPlane : public FVector
{
public:
	// Variables.
	FLOAT W;

	// Constructors.
	FORCEINLINE FPlane()
	{}
	FORCEINLINE FPlane( const FPlane& P )
	:	FVector(P)
	,	W(P.W)
	{}
	FORCEINLINE FPlane( const FVector4& V )
	:	FVector(V)
	,	W(V.W)
	{}
	FORCEINLINE FPlane( FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InW )
	:	FVector(InX,InY,InZ)
	,	W(InW)
	{}
	FORCEINLINE FPlane( FVector InNormal, FLOAT InW )
	:	FVector(InNormal), W(InW)
	{}
	FORCEINLINE FPlane( FVector InBase, const FVector &InNormal )
	:	FVector(InNormal)
	,	W(InBase | InNormal)
	{}
	FPlane( FVector A, FVector B, FVector C )
	:	FVector( ((B-A)^(C-A)).SafeNormal() )
	,	W( A | ((B-A)^(C-A)).SafeNormal() )
	{}
	explicit FORCEINLINE FPlane(EEventParm)
	: FVector(EC_EventParm), W(0.f)
	{}

	// Functions.
	FORCEINLINE FLOAT PlaneDot( const FVector &P ) const
	{
		return X*P.X + Y*P.Y + Z*P.Z - W;
	}
	FPlane Flip() const
	{
		return FPlane(-X,-Y,-Z,-W);
	}
	FPlane TransformPlaneByOrtho( const FMatrix& M ) const;
	FPlane TransformBy( const FMatrix& M ) const;
	FPlane TransformByUsingAdjointT( const FMatrix& M, FLOAT DetM, const FMatrix& TA ) const;
	UBOOL operator==( const FPlane& V ) const
	{
		return X==V.X && Y==V.Y && Z==V.Z && W==V.W;
	}
	UBOOL operator!=( const FPlane& V ) const
	{
		return X!=V.X || Y!=V.Y || Z!=V.Z || W!=V.W;
	}

	// Error-tolerant comparison.
	UBOOL Equals(const FPlane& V, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs(X-V.X) < Tolerance && Abs(Y-V.Y) < Tolerance && Abs(Z-V.Z) < Tolerance && Abs(W-V.W) < Tolerance;
	}

	FORCEINLINE FLOAT operator|( const FPlane& V ) const
	{
		return X*V.X + Y*V.Y + Z*V.Z + W*V.W;
	}
	FPlane operator+( const FPlane& V ) const
	{
		return FPlane( X + V.X, Y + V.Y, Z + V.Z, W + V.W );
	}
	FPlane operator-( const FPlane& V ) const
	{
		return FPlane( X - V.X, Y - V.Y, Z - V.Z, W - V.W );
	}
	FPlane operator/( FLOAT Scale ) const
	{
		const FLOAT RScale = 1.f/Scale;
		return FPlane( X * RScale, Y * RScale, Z * RScale, W * RScale );
	}
	FPlane operator*( FLOAT Scale ) const
	{
		return FPlane( X * Scale, Y * Scale, Z * Scale, W * Scale );
	}
	FPlane operator*( const FPlane& V )
	{
		return FPlane ( X*V.X,Y*V.Y,Z*V.Z,W*V.W );
	}
	FPlane operator+=( const FPlane& V )
	{
		X += V.X; Y += V.Y; Z += V.Z; W += V.W;
		return *this;
	}
	FPlane operator-=( const FPlane& V )
	{
		X -= V.X; Y -= V.Y; Z -= V.Z; W -= V.W;
		return *this;
	}
	FPlane operator*=( FLOAT Scale )
	{
		X *= Scale; Y *= Scale; Z *= Scale; W *= Scale;
		return *this;
	}
	FPlane operator*=( const FPlane& V )
	{
		X *= V.X; Y *= V.Y; Z *= V.Z; W *= V.W;
		return *this;
	}
	FPlane operator/=( FLOAT V )
	{
		const FLOAT RV = 1.f/V;
		X *= RV; Y *= RV; Z *= RV; W *= RV;
		return *this;
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FPlane &P )
	{
		return Ar << (FVector&)P << P.W;
	}
}  GCC_ALIGN(16);


/*-----------------------------------------------------------------------------
	FSphere.
-----------------------------------------------------------------------------*/

class FSphere : public FPlane
{
public:
	// Constructors.
	FSphere()
	{}
	FSphere( INT )
	:	FPlane(0,0,0,0)
	{}
	FSphere( FVector InV, FLOAT InW )
	:	FPlane( InV, InW )
	{}
	FSphere( const FVector* Pts, INT Count );

	FSphere TransformBy(const FMatrix& M) const;

	friend FArchive& operator<<( FArchive& Ar, FSphere& S )
	{
		Ar << (FPlane&)S;
		return Ar;
	}
};

/*-----------------------------------------------------------------------------
	FRotator.
-----------------------------------------------------------------------------*/

//
// Rotation.
//
class FRotator
{
public:
	// Variables.
	INT Pitch; // Looking up and down (0=Straight Ahead, +Up, -Down).
	INT Yaw;   // Rotating around (running in circles), 0=East, +North, -South.
	INT Roll;  // Rotation about axis of screen, 0=Straight, +Clockwise, -CCW.

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FRotator& R )
	{
		return Ar << R.Pitch << R.Yaw << R.Roll;
	}

	// Constructors.
	FRotator() {}
	FRotator( INT InPitch, INT InYaw, INT InRoll )
	:	Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}
	explicit FORCEINLINE FRotator(EEventParm)
	: Pitch(0), Yaw(0), Roll(0)
	{}

	FRotator( const FQuat& Quat );

	// Binary arithmetic operators.
	FRotator operator+( const FRotator &R ) const
	{
		return FRotator( Pitch+R.Pitch, Yaw+R.Yaw, Roll+R.Roll );
	}
	FRotator operator-( const FRotator &R ) const
	{
		return FRotator( Pitch-R.Pitch, Yaw-R.Yaw, Roll-R.Roll );
	}
	FRotator operator*( FLOAT Scale ) const
	{
		return FRotator( appTrunc(Pitch*Scale), appTrunc(Yaw*Scale), appTrunc(Roll*Scale) );
	}
	FRotator operator*= (FLOAT Scale)
	{
		Pitch = appTrunc(Pitch*Scale); Yaw = appTrunc(Yaw*Scale); Roll = appTrunc(Roll*Scale);
		return *this;
	}
	// Binary comparison operators.
	UBOOL operator==( const FRotator &R ) const
	{
		return Pitch==R.Pitch && Yaw==R.Yaw && Roll==R.Roll;
	}
	UBOOL operator!=( const FRotator &V ) const
	{
		return Pitch!=V.Pitch || Yaw!=V.Yaw || Roll!=V.Roll;
	}
	// Assignment operators.
	FRotator operator+=( const FRotator &R )
	{
		Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
		return *this;
	}
	FRotator operator-=( const FRotator &R )
	{
		Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
		return *this;
	}
	// Functions.
	FRotator Reduce() const
	{
		return FRotator( ReduceAngle(Pitch), ReduceAngle(Yaw), ReduceAngle(Roll) );
	}
	int IsZero() const
	{
		return ((Pitch&65535)==0) && ((Yaw&65535)==0) && ((Roll&65535)==0);
	}
	FRotator Add( INT DeltaPitch, INT DeltaYaw, INT DeltaRoll )
	{
		Yaw   += DeltaYaw;
		Pitch += DeltaPitch;
		Roll  += DeltaRoll;
		return *this;
	}
	FRotator AddBounded( INT DeltaPitch, INT DeltaYaw, INT DeltaRoll )
	{
		Yaw  += DeltaYaw;
		Pitch = FAddAngleConfined(Pitch,DeltaPitch,192*0x100,64*0x100);
		Roll  = FAddAngleConfined(Roll, DeltaRoll, 192*0x100,64*0x100);
		return *this;
	}
	FRotator GridSnap( const FRotator &RotGrid ) const
	{
		return FRotator
		(
			appTrunc(FSnap(Pitch,RotGrid.Pitch)),
			appTrunc(FSnap(Yaw,  RotGrid.Yaw)),
			appTrunc(FSnap(Roll, RotGrid.Roll))
		);
	}
	FVector Vector() const;
	FQuat Quaternion() const;
	FVector Euler() const;

	static FRotator MakeFromEuler(const FVector& Euler);

	// Resets the rotation values so they fall within the range -65535,65535
	FRotator Clamp() const
	{
		return FRotator( Pitch&65535, Yaw&65535, Roll&65535 );
	}
	FRotator ClampPos() const
	{
		return FRotator( Abs(Pitch)&65535, Abs(Yaw)&65535, Abs(Roll)&65535 );
	}

	static INT NormalizeAxis(INT Angle)
	{
		Angle &= 0xFFFF;
		if( Angle > 32767 ) 
		{
			Angle -= 0x10000;
		}
		return Angle;
	}

	FRotator GetNormalized() const
	{
		FRotator Rot = *this;
		Rot.Pitch = NormalizeAxis(Rot.Pitch);
		Rot.Roll = NormalizeAxis(Rot.Roll);
		Rot.Yaw = NormalizeAxis(Rot.Yaw);
		return Rot;
	}

	FRotator GetDenormalized() const
	{
		FRotator Rot = *this;
		Rot.Pitch	= Rot.Pitch & 0xFFFF;
		Rot.Yaw		= Rot.Yaw & 0xFFFF;
		Rot.Roll	= Rot.Roll & 0xFFFF;
		return Rot;
	}

	void MakeShortestRoute();
	void GetWindingAndRemainder(FRotator& Winding, FRotator& Remainder) const;

	FString ToString() const
	{
		return FString::Printf(TEXT("P=%i Y=%i R=%i"), Pitch, Yaw, Roll );
	}

};

inline FRotator operator*( FLOAT Scale, const FRotator &R )
{
	return R.operator*( Scale );
}

#if __INTEL_BYTE_ORDER__
	#define INTEL_ORDER_ROTATOR(x) (x)
#else
	static inline FRotator INTEL_ORDER_ROTATOR(FRotator r)
	{
		return FRotator(INTEL_ORDER32(r.Pitch), INTEL_ORDER32(r.Yaw), INTEL_ORDER32(r.Roll));
	}
#endif

/*-----------------------------------------------------------------------------
	Bounds.
-----------------------------------------------------------------------------*/

//
// A rectangular minimum bounding volume.
//
class FBox
{
public:
	// Variables.
	FVector Min;
	FVector Max;
	BYTE IsValid;

	// Constructors.
	FBox() {}
	FBox(INT) { Init(); }
	FBox( const FVector& InMin, const FVector& InMax ) : Min(InMin), Max(InMax), IsValid(1) {}
	FBox( const FVector* Points, INT Count );

	/** Utility function to build an AABB from Origin and Extent */
	static FBox BuildAABB( const FVector& Origin, const FVector& Extent )
	{
		FBox NewBox(0);
		NewBox += Origin - Extent;
		NewBox += Origin + Extent;
		return NewBox;
	}

	// Accessors.
	FVector& GetExtrema( int i )
	{
		return (&Min)[i];
	}
	const FVector& GetExtrema( int i ) const
	{
		return (&Min)[i];
	}

	// Functions.
	void Init()
	{
		Min = Max = FVector(0,0,0);
		IsValid = 0;
	}
	FORCEINLINE FBox& operator+=( const FVector &Other )
	{
		if( IsValid )
		{
#if ASM_X86
			__asm
			{
				mov		eax,[Other]
				mov		ecx,[this]
				
				movss	xmm3,[eax]FVector.X
				movss	xmm4,[eax]FVector.Y
				movss	xmm5,[eax]FVector.Z

				movss	xmm0,[ecx]FBox.Min.X
				movss	xmm1,[ecx]FBox.Min.Y
				movss	xmm2,[ecx]FBox.Min.Z
				minss	xmm0,xmm3
				minss	xmm1,xmm4
				minss	xmm2,xmm5
				movss	[ecx]FBox.Min.X,xmm0
				movss	[ecx]FBox.Min.Y,xmm1
				movss	[ecx]FBox.Min.Z,xmm2

				movss	xmm0,[ecx]FBox.Max.X
				movss	xmm1,[ecx]FBox.Max.Y
				movss	xmm2,[ecx]FBox.Max.Z
				maxss	xmm0,xmm3
				maxss	xmm1,xmm4
				maxss	xmm2,xmm5
				movss	[ecx]FBox.Max.X,xmm0
				movss	[ecx]FBox.Max.Y,xmm1
				movss	[ecx]FBox.Max.Z,xmm2
			}
#else
			Min.X = ::Min( Min.X, Other.X );
			Min.Y = ::Min( Min.Y, Other.Y );
			Min.Z = ::Min( Min.Z, Other.Z );

			Max.X = ::Max( Max.X, Other.X );
			Max.Y = ::Max( Max.Y, Other.Y );
			Max.Z = ::Max( Max.Z, Other.Z );
#endif
		}
		else
		{
			Min = Max = Other;
			IsValid = 1;
		}
		return *this;
	}
	FBox operator+( const FVector& Other ) const
	{
		return FBox(*this) += Other;
	}
	FBox& operator+=( const FBox& Other )
	{
		if( IsValid && Other.IsValid )
		{
#if ASM_X86
			__asm
			{
				mov		eax,[Other]
				mov		ecx,[this]

				movss	xmm0,[ecx]FBox.Min.X
				movss	xmm1,[ecx]FBox.Min.Y
				movss	xmm2,[ecx]FBox.Min.Z
				minss	xmm0,[eax]FBox.Min.X
				minss	xmm1,[eax]FBox.Min.Y
				minss	xmm2,[eax]FBox.Min.Z
				movss	[ecx]FBox.Min.X,xmm0
				movss	[ecx]FBox.Min.Y,xmm1
				movss	[ecx]FBox.Min.Z,xmm2

				movss	xmm0,[ecx]FBox.Max.X
				movss	xmm1,[ecx]FBox.Max.Y
				movss	xmm2,[ecx]FBox.Max.Z
				maxss	xmm0,[eax]FBox.Max.X
				maxss	xmm1,[eax]FBox.Max.Y
				maxss	xmm2,[eax]FBox.Max.Z
				movss	[ecx]FBox.Max.X,xmm0
				movss	[ecx]FBox.Max.Y,xmm1
				movss	[ecx]FBox.Max.Z,xmm2
			}
#else
			Min.X = ::Min( Min.X, Other.Min.X );
			Min.Y = ::Min( Min.Y, Other.Min.Y );
			Min.Z = ::Min( Min.Z, Other.Min.Z );

			Max.X = ::Max( Max.X, Other.Max.X );
			Max.Y = ::Max( Max.Y, Other.Max.Y );
			Max.Z = ::Max( Max.Z, Other.Max.Z );
#endif
		}
		else if( Other.IsValid )
		{
			*this = Other;
		}
		return *this;
	}
	FBox operator+( const FBox& Other ) const
	{
		return FBox(*this) += Other;
	}
    FVector& operator[]( INT i )
	{
		check(i>-1);
		check(i<2);
		if( i == 0 )		return Min;
		else				return Max;
	}
	FBox TransformBy( const FMatrix& M ) const;
	FBox TransformProjectBy( const FMatrix& ProjM ) const;
	FBox ExpandBy( FLOAT W ) const
	{
		return FBox( Min - FVector(W,W,W), Max + FVector(W,W,W) );
	}

	// Returns the midpoint between the min and max points.
	FVector GetCenter() const
	{
		return FVector( ( Min + Max ) * 0.5f );
	}
	// Returns the extent around the center
	FVector GetExtent() const
	{
		return 0.5f*(Max - Min);
	}

	void GetCenterAndExtents( FVector & center, FVector & Extents ) const
	{
		Extents = GetExtent();
		center = Min + Extents;
	}

	UBOOL Intersect( const FBox & other ) const
	{
		if( Min.X > other.Max.X || other.Min.X > Max.X )
			return FALSE;
		if( Min.Y > other.Max.Y || other.Min.Y > Max.Y )
			return FALSE;
		if( Min.Z > other.Max.Z || other.Min.Z > Max.Z )
			return FALSE;
		return TRUE;
	}

	UBOOL IntersectXY( const FBox& other ) const
	{
		if( Min.X > other.Max.X || other.Min.X > Max.X )
			return FALSE;
		if( Min.Y > other.Max.Y || other.Min.Y > Max.Y )
			return FALSE;
		return TRUE;
	}

	// Checks to see if the location is inside this box
	UBOOL IsInside( const FVector& In ) const
	{
		return ( In.X > Min.X && In.X < Max.X
				&& In.Y > Min.Y && In.Y < Max.Y 
				&& In.Z > Min.Z && In.Z < Max.Z );
	}

	/** Calculate volume of this box. */
	FLOAT GetVolume() const
	{
		return ((Max.X-Min.X) * (Max.Y-Min.Y) * (Max.Z-Min.Z));
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FBox& Bound )
	{
		return Ar << Bound.Min << Bound.Max << Bound.IsValid;
	}
};

class FSimpleBox
{
public:
	// Variables.
	FVector Min;
	FVector Max;

	// Constructors.
	FSimpleBox() {}
};

/**
 * An axis aligned bounding box and bounding sphere with the same origin. (28 bytes).
 */
struct FBoxSphereBounds
{
	FVector	Origin,
			BoxExtent;
	FLOAT	SphereRadius;

	// Constructor.

	FBoxSphereBounds() {}

	FBoxSphereBounds(const FVector& InOrigin,const FVector& InBoxExtent,FLOAT InSphereRadius):
		Origin(InOrigin),
		BoxExtent(InBoxExtent),
		SphereRadius(InSphereRadius)
	{}

	FBoxSphereBounds(const FBox& Box,const FSphere& Sphere)
	{
		Box.GetCenterAndExtents(Origin,BoxExtent);
		SphereRadius = Min(BoxExtent.Size(),((FVector)Sphere - Origin).Size() + Sphere.W);
	}

	FBoxSphereBounds(const FBox& Box)
	{
		Box.GetCenterAndExtents(Origin,BoxExtent);
		SphereRadius = BoxExtent.Size();
	}

	FBoxSphereBounds(const FVector* Points,UINT NumPoints)
	{
		// Find an axis aligned bounding box for the points.
		FBox	BoundingBox(0);
		for(UINT PointIndex = 0;PointIndex < NumPoints;PointIndex++)
			BoundingBox += Points[PointIndex];
		BoundingBox.GetCenterAndExtents(Origin,BoxExtent);

		// Using the center of the bounding box as the origin of the sphere, find the radius of the bounding sphere.
		SphereRadius = 0.0f;
		for(UINT PointIndex = 0;PointIndex < NumPoints;PointIndex++)
			SphereRadius = Max(SphereRadius,(Points[PointIndex] - Origin).Size());
	}

	// GetBoxExtrema

	FVector GetBoxExtrema(UINT Extrema) const
	{
		if(Extrema)
			return Origin + BoxExtent;
		else
			return Origin - BoxExtent;
	}

	// GetBox

	FBox GetBox() const
	{
		return FBox(Origin - BoxExtent,Origin + BoxExtent);
	}

	// GetSphere

	FSphere GetSphere() const
	{
		return FSphere(Origin,SphereRadius);
	}

	// TransformBy

	FBoxSphereBounds TransformBy(const FMatrix& M) const;

	/**
	 * Constructs a bounding volume containing both A and B.
	 * This is a legacy version of the function used to compute primitive bounds, to avoid the need to rebuild lighting after the change.
	 */
	friend FBoxSphereBounds LegacyUnion(const FBoxSphereBounds& A,const FBoxSphereBounds& B)
	{
		FBox	BoundingBox(0);
		BoundingBox += (A.Origin - A.BoxExtent);
		BoundingBox += (A.Origin + A.BoxExtent);
		BoundingBox += (B.Origin - B.BoxExtent);
		BoundingBox += (B.Origin + B.BoxExtent);

		// Build a bounding sphere from the bounding box's origin and the radii of A and B.
		FBoxSphereBounds	Result(BoundingBox);
		Result.SphereRadius = Min(Result.SphereRadius,Max((A.Origin - Result.Origin).Size() + A.SphereRadius,(B.Origin - Result.Origin).Size()));

		return Result;
	}

	/**
	 * Constructs a bounding volume containing both A and B.
	 */
	FBoxSphereBounds operator+(const FBoxSphereBounds& B) const
	{
		FBox	BoundingBox(0);
		BoundingBox += (this->Origin - this->BoxExtent);
		BoundingBox += (this->Origin + this->BoxExtent);
		BoundingBox += (B.Origin - B.BoxExtent);
		BoundingBox += (B.Origin + B.BoxExtent);

		// Build a bounding sphere from the bounding box's origin and the radii of A and B.

		FBoxSphereBounds	Result(BoundingBox);
		Result.SphereRadius = Min(
			Result.SphereRadius,
			Max(
				(Origin - Result.Origin).Size() + SphereRadius,
				(B.Origin - Result.Origin).Size() + B.SphereRadius
				)
			);

		return Result;
	}

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar,FBoxSphereBounds& Bounds)
	{
		return Ar << Bounds.Origin << Bounds.BoxExtent << Bounds.SphereRadius;
	}
};


/**
 * Point of View Type
 */
struct FTPOV
{
	FVector		Location;
	FRotator	Rotation;
	FLOAT		FOV;

	FTPOV() {}
	FTPOV(FVector InLocation, FRotator InRotation, FLOAT InFOV): Location(InLocation), Rotation(InRotation), FOV(InFOV) {}

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FTPOV& POV)
	{
		return Ar << POV.Location << POV.Rotation << POV.FOV;
	}
};

/*-----------------------------------------------------------------------------
	FGlobalMath.
-----------------------------------------------------------------------------*/

/**
 * Global mathematics info.
 */
class FGlobalMath
{
public:
	// Constants.
	enum {ANGLE_SHIFT 	= 2};		// Bits to right-shift to get lookup value.
	enum {ANGLE_BITS	= 14};		// Number of valid bits in angles.
	enum {NUM_ANGLES 	= 16384}; 	// Number of angles that are in lookup table.
	enum {ANGLE_MASK    =  (((1<<ANGLE_BITS)-1)<<(16-ANGLE_BITS))};

	// Basic math functions.
	FORCEINLINE FLOAT SinTab( int i ) const
	{
		return TrigFLOAT[((i>>ANGLE_SHIFT)&(NUM_ANGLES-1))];
	}
	FORCEINLINE FLOAT CosTab( int i ) const
	{
		return TrigFLOAT[(((i+16384)>>ANGLE_SHIFT)&(NUM_ANGLES-1))];
	}
	FLOAT SinFloat( FLOAT F ) const
	{
		return SinTab(appTrunc((F*65536.f)/(2.f*PI)));
	}
	FLOAT CosFloat( FLOAT F ) const
	{
		return CosTab(appTrunc((F*65536.f)/(2.f*PI)));
	}

	// Constructor.
	FGlobalMath();

private:
	// Tables.
	FLOAT  TrigFLOAT		[NUM_ANGLES];
};

inline INT ReduceAngle( INT Angle )
{
	return Angle & FGlobalMath::ANGLE_MASK;
};


/** Convert a direction vector into a 'heading' angle between +/-PI. 0 is pointing down +X. */
inline FLOAT HeadingAngle(FVector Dir)
{
	// Project Dir into Z plane.
	FVector PlaneDir = Dir;
	PlaneDir.Z = 0.f;
	PlaneDir = PlaneDir.SafeNormal();

	FLOAT Angle = appAcos(PlaneDir.X);

	if(PlaneDir.Y < 0.0f)
	{
		Angle *= -1.0f;
	}

	return Angle;
}

/** Find the smallest angle between two headings (in radians) */
inline FLOAT FindDeltaAngle(FLOAT A1, FLOAT A2)
{
	// Find the difference
	FLOAT Delta = A2 - A1;

	// If change is larger than PI
	if(Delta > PI)
	{
		// Flip to negative equivalent
		Delta = Delta - (PI * 2.0f);
	}
	else if(Delta < -PI)
	{
		// Otherwise, if change is smaller than -PI
		// Flip to positive equivalent
		Delta = Delta + (PI * 2.0f);
	}

	// Return delta in [-PI,PI] range
	return Delta;
}

/** Given a heading which may be outside the +/- PI range, 'unwind' it back into that range. */
inline FLOAT UnwindHeading(FLOAT A)
{
	while(A > PI)
	{
		A -= ((FLOAT)PI * 2.0f);
	}

	while(A < -PI)
	{
		A += ((FLOAT)PI * 2.0f);
	}

	return A;
}

/*-----------------------------------------------------------------------------
	Floating point constants.
-----------------------------------------------------------------------------*/

/**
 * Lengths of normalized vectors (These are half their maximum values
 * to assure that dot products with normalized vectors don't overflow).
 */
#define FLOAT_NORMAL_THRESH				(0.0001f)

//
// Magic numbers for numerical precision.
//
#define THRESH_POINT_ON_PLANE			(0.10f)		/* Thickness of plane for front/back/inside test */
#define THRESH_POINT_ON_SIDE			(0.20f)		/* Thickness of polygon side's side-plane for point-inside/outside/on side test */
#define THRESH_POINTS_ARE_SAME			(0.002f)	/* Two points are same if within this distance */
#define THRESH_POINTS_ARE_NEAR			(0.015f)	/* Two points are near if within this distance and can be combined if imprecise math is ok */
#define THRESH_NORMALS_ARE_SAME			(0.00002f)	/* Two normal points are same if within this distance */
													/* Making this too large results in incorrect CSG classification and disaster */
#define THRESH_VECTORS_ARE_NEAR			(0.0004f)	/* Two vectors are near if within this distance and can be combined if imprecise math is ok */
													/* Making this too large results in lighting problems due to inaccurate texture coordinates */
#define THRESH_SPLIT_POLY_WITH_PLANE	(0.25f)		/* A plane splits a polygon in half */
#define THRESH_SPLIT_POLY_PRECISELY		(0.01f)		/* A plane exactly splits a polygon */
#define THRESH_ZERO_NORM_SQUARED		(0.0001f)	/* Size of a unit normal that is considered "zero", squared */
#define THRESH_VECTORS_ARE_PARALLEL		(0.02f)		/* Vectors are parallel if dot product varies less than this */


/*-----------------------------------------------------------------------------
	FVector transformation.
-----------------------------------------------------------------------------*/

/**
 * Mirrors a vector about a plane.
 */
inline FVector FVector::MirrorByPlane( const FPlane& Plane ) const
{
	return *this - Plane * (2.f * Plane.PlaneDot(*this) );
}

/**
 * Rotate around Axis (assumes Axis.Size() == 1).
 */
inline FVector FVector::RotateAngleAxis( const INT Angle, const FVector& Axis ) const
{
	const FLOAT S	= GMath.SinTab(Angle);
	const FLOAT C	= GMath.CosTab(Angle);

	const FLOAT XX	= Axis.X * Axis.X;
	const FLOAT YY	= Axis.Y * Axis.Y;
	const FLOAT ZZ	= Axis.Z * Axis.Z;

	const FLOAT XY	= Axis.X * Axis.Y;
	const FLOAT YZ	= Axis.Y * Axis.Z;
	const FLOAT ZX	= Axis.Z * Axis.X;

	const FLOAT XS	= Axis.X * S;
	const FLOAT YS	= Axis.Y * S;
	const FLOAT ZS	= Axis.Z * S;

	const FLOAT OMC	= 1.f - C;

	return FVector(
		(OMC * XX + C ) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z,
		(OMC * XY + ZS) * X + (OMC * YY + C ) * Y + (OMC * YZ - XS) * Z,
		(OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C ) * Z
		);
}


/*-----------------------------------------------------------------------------
	FVector friends.
-----------------------------------------------------------------------------*/

/**
 * Compare two points and see if they're the same, using a threshold.
 * Returns 1=yes, 0=no.  Uses fast distance approximation.
 */
inline UBOOL FPointsAreSame( const FVector &P, const FVector &Q )
{
	FLOAT Temp;
	Temp=P.X-Q.X;
	if( (Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME) )
	{
		Temp=P.Y-Q.Y;
		if( (Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME) )
		{
			Temp=P.Z-Q.Z;
			if( (Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME) )
			{
				return 1;
			}
		}
	}
	return 0;
}

/**
 * Compare two points and see if they're the same, using a threshold.
 * Returns 1=yes, 0=no.  Uses fast distance approximation.
 */
inline UBOOL FPointsAreNear( const FVector &Point1, const FVector &Point2, FLOAT Dist )
{
	FLOAT Temp;
	Temp=(Point1.X - Point2.X); if (Abs(Temp)>=Dist) return 0;
	Temp=(Point1.Y - Point2.Y); if (Abs(Temp)>=Dist) return 0;
	Temp=(Point1.Z - Point2.Z); if (Abs(Temp)>=Dist) return 0;
	return 1;
}

/**
 * Calculate the signed distance (in the direction of the normal) between
 * a point and a plane.
 */
inline FLOAT FPointPlaneDist
(
	const FVector &Point,
	const FVector &PlaneBase,
	const FVector &PlaneNormal
)
{
	return (Point - PlaneBase) | PlaneNormal;
}

/**
 * Euclidean distance between two points.
 */
inline FLOAT FDist( const FVector &V1, const FVector &V2 )
{
	return appSqrt( Square(V2.X-V1.X) + Square(V2.Y-V1.Y) + Square(V2.Z-V1.Z) );
}

/**
 * Squared distance between two points.
 */
inline FLOAT FDistSquared( const FVector &V1, const FVector &V2 )
{
	return Square(V2.X-V1.X) + Square(V2.Y-V1.Y) + Square(V2.Z-V1.Z);
}

/**
 * See if two normal vectors (or plane normals) are nearly parallel.
 */
inline UBOOL FParallel( const FVector &Normal1, const FVector &Normal2 )
{
	const FLOAT NormalDot = Normal1 | Normal2;
	return (Abs (NormalDot - 1.f) <= THRESH_VECTORS_ARE_PARALLEL);
}

/**
 * See if two planes are coplanar.
 */
inline UBOOL FCoplanar( const FVector &Base1, const FVector &Normal1, const FVector &Base2, const FVector &Normal2 )
{
	if      (!FParallel(Normal1,Normal2)) return 0;
	else if (FPointPlaneDist (Base2,Base1,Normal1) > THRESH_POINT_ON_PLANE) return 0;
	else    return 1;
}

/**
 * Triple product of three vectors.
 */
inline FLOAT FTriple( const FVector& X, const FVector& Y, const FVector& Z )
{
	return
	(	(X.X * (Y.Y * Z.Z - Y.Z * Z.Y))
	+	(X.Y * (Y.Z * Z.X - Y.X * Z.Z))
	+	(X.Z * (Y.X * Z.Y - Y.Y * Z.X)) );
}

/**
 * Compute pushout of a box from a plane.
 */
inline FLOAT FBoxPushOut( const FVector & Normal, const FVector & Size )
{
    return Abs(Normal.X*Size.X) + Abs(Normal.Y*Size.Y) + Abs(Normal.Z*Size.Z);
}

/*-----------------------------------------------------------------------------
	Random numbers.
-----------------------------------------------------------------------------*/

/**
 * Return a uniformly distributed random unit vector.
 */
inline FVector VRand()
{
	FVector Result;
	do
	{
		// Check random vectors in the unit sphere so result is statistically uniform.
		Result.X = appFrand()*2 - 1;
		Result.Y = appFrand()*2 - 1;
		Result.Z = appFrand()*2 - 1;
	} while( Result.SizeSquared() > 1.f );
	return Result.UnsafeNormal();
}

/*-----------------------------------------------------------------------------
	Advanced geometry.
-----------------------------------------------------------------------------*/

/**
 * Find the intersection of an infinite line (defined by two points) and
 * a plane.  Assumes that the line and plane do indeed intersect; you must
 * make sure they're not parallel before calling.
 */
inline FVector FLinePlaneIntersection
(
	const FVector &Point1,
	const FVector &Point2,
	const FVector &PlaneOrigin,
	const FVector &PlaneNormal
)
{
	return
		Point1
	+	(Point2-Point1)
	*	(((PlaneOrigin - Point1)|PlaneNormal) / ((Point2 - Point1)|PlaneNormal));
}
inline FVector FLinePlaneIntersection
(
	const FVector &Point1,
	const FVector &Point2,
	const FPlane  &Plane
)
{
	return
		Point1
	+	(Point2-Point1)
	*	((Plane.W - (Point1|Plane))/((Point2 - Point1)|Plane));
}

/**
 * Determines whether a point is inside a box.
 */
inline UBOOL FPointBoxIntersection
(
	const FVector&	Point,
	const FBox&		Box
)
{
	if(Point.X >= Box.Min.X && Point.X <= Box.Max.X &&
	   Point.Y >= Box.Min.Y && Point.Y <= Box.Max.Y &&
	   Point.Z >= Box.Min.Z && Point.Z <= Box.Max.Z)
		return 1;
	else
		return 0;
}

/**
 * Determines whether a line intersects a box.
 */
inline UBOOL FLineBoxIntersection
(
	const FBox&		Box,
	const FVector&	Start,
	const FVector&	End,
	const FVector&	Direction,
	const FVector&	OneOverDirection
)
{
	FVector	Time;
	UBOOL	bStartIsOutside = FALSE;

	if(Start.X < Box.Min.X)
	{
		bStartIsOutside = TRUE;
		if(End.X >= Box.Min.X)
		{
			Time.X = (Box.Min.X - Start.X) * OneOverDirection.X;
		}
		else
		{
			return FALSE;
		}
	}
	else if(Start.X > Box.Max.X)
	{
		bStartIsOutside = TRUE;
		if(End.X <= Box.Max.X)
		{
			Time.X = (Box.Max.X - Start.X) * OneOverDirection.X;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.X = 0.0f;
	}

	if(Start.Y < Box.Min.Y)
	{
		bStartIsOutside = TRUE;
		if(End.Y >= Box.Min.Y)
		{
			Time.Y = (Box.Min.Y - Start.Y) * OneOverDirection.Y;
		}
		else
		{
			return FALSE;
		}
	}
	else if(Start.Y > Box.Max.Y)
	{
		bStartIsOutside = TRUE;
		if(End.Y <= Box.Max.Y)
		{
			Time.Y = (Box.Max.Y - Start.Y) * OneOverDirection.Y;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.Y = 0.0f;
	}

	if(Start.Z < Box.Min.Z)
	{
		bStartIsOutside = TRUE;
		if(End.Z >= Box.Min.Z)
		{
			Time.Z = (Box.Min.Z - Start.Z) * OneOverDirection.Z;
		}
		else
		{
			return FALSE;
		}
	}
	else if(Start.Z > Box.Max.Z)
	{
		bStartIsOutside = TRUE;
		if(End.Z <= Box.Max.Z)
		{
			Time.Z = (Box.Max.Z - Start.Z) * OneOverDirection.Z;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.Z = 0.0f;
	}

	if(bStartIsOutside)
	{
		const FLOAT	MaxTime = Max(Time.X,Max(Time.Y,Time.Z));

		if(MaxTime >= 0.0f && MaxTime <= 1.0f)
		{
			const FVector Hit = Start + Direction * MaxTime;
			const FLOAT BOX_SIDE_THRESHOLD = 0.1f;
			if(	Hit.X > Box.Min.X - BOX_SIDE_THRESHOLD && Hit.X < Box.Max.X + BOX_SIDE_THRESHOLD &&
				Hit.Y > Box.Min.Y - BOX_SIDE_THRESHOLD && Hit.Y < Box.Max.Y + BOX_SIDE_THRESHOLD &&
				Hit.Z > Box.Min.Z - BOX_SIDE_THRESHOLD && Hit.Z < Box.Max.Z + BOX_SIDE_THRESHOLD)
			{
				return TRUE;
			}
		}

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

UBOOL FLineExtentBoxIntersection(const FBox& inBox, 
								 const FVector& Start, 
								 const FVector& End,
								 const FVector& Extent,
								 FVector& HitLocation,
								 FVector& HitNormal,
								 FLOAT& HitTime);

/**
 * Determines whether a line intersects a sphere.
 */
inline UBOOL FLineSphereIntersection(const FVector& Start,const FVector& Dir,FLOAT Length,const FVector& Origin,FLOAT Radius)
{
	const FVector	EO = Start - Origin;
	const FLOAT		v = (Dir | (Origin - Start));
	const FLOAT		disc = Radius * Radius - ((EO | EO) - v * v);

	if(disc >= 0.0f)
	{
		const FLOAT	Time = (v - appSqrt(disc)) / Length;

		if(Time >= 0.0f && Time <= 1.0f)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

/*-----------------------------------------------------------------------------
	FPlane functions.
-----------------------------------------------------------------------------*/

/**
 * Compute intersection point of three planes. Return 1 if valid, 0 if infinite.
 */
inline UBOOL FIntersectPlanes3( FVector& I, const FPlane& P1, const FPlane& P2, const FPlane& P3 )
{
	// Compute determinant, the triple product P1|(P2^P3)==(P1^P2)|P3.
	const FLOAT Det = (P1 ^ P2) | P3;
	if( Square(Det) < Square(0.001f) )
	{
		// Degenerate.
		I = FVector(0,0,0);
		return 0;
	}
	else
	{
		// Compute the intersection point, guaranteed valid if determinant is nonzero.
		I = (P1.W*(P2^P3) + P2.W*(P3^P1) + P3.W*(P1^P2)) / Det;
	}
	return 1;
}

/**
 * Compute intersection point and direction of line joining two planes.
 * Return 1 if valid, 0 if infinite.
 */
inline UBOOL FIntersectPlanes2( FVector& I, FVector& D, const FPlane& P1, const FPlane& P2 )
{
	// Compute line direction, perpendicular to both plane normals.
	D = P1 ^ P2;
	const FLOAT DD = D.SizeSquared();
	if( DD < Square(0.001f) )
	{
		// Parallel or nearly parallel planes.
		D = I = FVector(0,0,0);
		return 0;
	}
	else
	{
		// Compute intersection.
		I = (P1.W*(P2^D) + P2.W*(D^P1)) / DD;
		D.Normalize();
		return 1;
	}
}

/*-----------------------------------------------------------------------------
	FQuat.          
-----------------------------------------------------------------------------*/

// 40% faster version of the Quaternion multiplication.
#define USE_FAST_QUAT_MUL 1

/**
 * Floating point quaternion.
 */
class FQuat 
{
public:

	static const FQuat Identity;

	// Variables.
	FLOAT X,Y,Z,W;
	// X,Y,Z, W also doubles as the Axis/Angle format.

	// Constructors.
	FQuat()
	{}

	FQuat( FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InA )
	:	X(InX), Y(InY), Z(InZ), W(InA)
	{}

	FQuat( const FMatrix& M );	

	FString ToString() const
	{
		return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f W=%3.3f"), X, Y, Z, W);
	}

	/**
	 * Assumes Axis is normalized.
	 */
	FQuat( FVector Axis, FLOAT Angle )
	{
		const FLOAT half_a = 0.5f * Angle;
		const FLOAT s = appSin(half_a);
		const FLOAT c = appCos(half_a);

		X = s * Axis.X;
		Y = s * Axis.Y;
		Z = s * Axis.Z;
		W = c;
	}

	static FQuat MakeFromEuler(const FVector& Euler);

	FVector Euler() const;

	// Binary operators.
	FQuat operator+( const FQuat& Q ) const
	{		
		return FQuat( X + Q.X, Y + Q.Y, Z + Q.Z, W + Q.W );
	}

	FQuat operator-( const FQuat& Q ) const
	{
		return FQuat( X - Q.X, Y - Q.Y, Z - Q.Z, W - Q.W );
	}

	FORCEINLINE FQuat operator*( const FQuat& Q ) const
	{
		FQuat Result = *this;
		return (Result *= Q);
	}

	FORCEINLINE FQuat operator*=(const FQuat& Q)
	{
// 40% faster version of the Quaternion multiplication.
#ifdef USE_FAST_QUAT_MUL
		const FLOAT T0 = (Z - Y) * (Q.Y - Q.Z);
		const FLOAT T1 = (W + X) * (Q.W + Q.X);
		const FLOAT T2 = (W - X) * (Q.Y + Q.Z);
		const FLOAT T3 = (Y + Z) * (Q.W - Q.X);
		const FLOAT T4 = (Z - X) * (Q.X - Q.Y);
		const FLOAT T5 = (Z + X) * (Q.X + Q.Y);
		const FLOAT T6 = (W + Y) * (Q.W - Q.Z);
		const FLOAT T7 = (W - Y) * (Q.W + Q.Z);
		const FLOAT T8 = T5 + T6 + T7;
		const FLOAT T9 = 0.5 * (T4 + T8);

		X = T1 + T9 - T8;
		Y = T2 + T9 - T7;
		Z = T3 + T9 - T6;
		W = T0 + T9 - T5;
#else
		// store intermediate results in temporaries
		const FLOAT TX = W*Q.X + X*Q.W + Y*Q.Z - Z*Q.Y;
		const FLOAT TY = W*Q.Y - X*Q.Z + Y*Q.W + Z*Q.X;
		const FLOAT TZ = W*Q.Z + X*Q.Y - Y*Q.X + Z*Q.W;
		const FLOAT TW = W*Q.W - X*Q.X - Y*Q.Y - Z*Q.Z;

		// copy intermediate result to *this
		X = TX;
		Y = TY;
		Z = TZ;
		W = TW;
#endif
	  return *this; 
	}

	FQuat operator*( const FLOAT& Scale ) const
	{
		return FQuat( Scale*X, Scale*Y, Scale*Z, Scale*W);			
	}
	
	// Unary operators.
	FQuat operator-() const
	{
		// For historical reasons, this returns the Multiplicative Inverse, and assumes *this is normalized.
		return FQuat( -X, -Y, -Z, W );
	}

    // Misc operators
	UBOOL operator!=( const FQuat& Q ) const
	{
		return X!=Q.X || Y!=Q.Y || Z!=Q.Z || W!=Q.W;
	}
	
	FLOAT operator|( const FQuat& Q ) const
	{
		return X*Q.X + Y*Q.Y + Z*Q.Z + W*Q.W;
	}

	void Normalize()
	{
		const FLOAT SquareSum = X*X + Y*Y + Z*Z + W*W;
		if( SquareSum > SMALL_NUMBER )
		{
			const FLOAT Scale = appInvSqrt(SquareSum);
			X *= Scale; 
			Y *= Scale; 
			Z *= Scale;
			W *= Scale;
		}
		else
		{
			*this = FQuat::Identity;
		}
	}

	// Return TRUE if this quaternion is normalized
	UBOOL IsNormalized() const
	{
		return (Abs(1.f - SizeSquared()) <= 0.01f);
	}

	FLOAT SizeSquared() const
	{
		return (X*X+Y*Y+Z*Z+W*W);
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FQuat& F )
	{
		return Ar << F.X << F.Y << F.Z << F.W;
	}

	// Warning : assumes normalized quaternions.
	void ToAxisAndAngle(FVector& Axis, FLOAT& Angle) const
	{
		Angle = 2.f * appAcos(W);

		// Ensure we never try to sqrt a neg number
		const FLOAT S = appSqrt( ::Max(1.f-(W*W), 0.f) );
		if (S >= 0.0001f) 
		{ 
			Axis.X = X / S;
			Axis.Y = Y / S;
			Axis.Z = Z / S;
		} 
		else 
		{
			Axis.X = 1.f;
			Axis.Y = 0.f;
			Axis.Z = 0.f;
		}
	};

	FVector RotateVector(FVector v) const
	{	
		// (q.W*q.W-qv.qv)v + 2(qv.v)qv + 2 q.W (qv x v)

		const FVector qv(X, Y, Z);
		FVector vOut = 2.f * W * (qv ^ v);
		vOut += ((W * W) - (qv | qv)) * v;
		vOut += (2.f * (qv | v)) * qv;

		return vOut;
	}

	// Exp should really only be used after Log.
	FQuat Log() const;
	FQuat Exp() const;
};

/**
 * Generates the 'smallest' (geodesic) rotation between these two vectors.
 */
FQuat FQuatFindBetween(const FVector& vec1, const FVector& vec2);

/**
 * Error measure (angle) between two quaternions, ranged [0..1].
 * Returns the hypersphere-angle between two quaternions; alignment shouldn't matter, though 
 * normalized input is expected.
 */
inline FLOAT FQuatError(const FQuat& Q1, const FQuat& Q2)
{
	const FLOAT cosom = Q1.X*Q2.X + Q1.Y*Q2.Y + Q1.Z*Q2.Z + Q1.W*Q2.W;
	return (Abs(cosom) < 0.9999999f) ? appAcos(cosom)*(1.f/PI) : 0.0f;
}

/**
 * Ensures quat1 points to same side of the hypersphere as quat2.
 */
inline void AlignFQuatWith(FQuat &quat1, const FQuat &quat2)
{
	const FLOAT Minus  = Square(quat1.X-quat2.X) + Square(quat1.Y-quat2.Y) + Square(quat1.Z-quat2.Z) + Square(quat1.W-quat2.W);
	const FLOAT Plus   = Square(quat1.X+quat2.X) + Square(quat1.Y+quat2.Y) + Square(quat1.Z+quat2.Z) + Square(quat1.W+quat2.W);

	if (Minus > Plus)
	{
		quat1.X = - quat1.X;
		quat1.Y = - quat1.Y;
		quat1.Z = - quat1.Z;
		quat1.W = - quat1.W;
	}
}

/** 
 * Fast Linear Quaternion Interpolation.
 * Result is NOT normalized.
 */
FORCEINLINE FQuat LerpQuat(const FQuat& A, const FQuat& B, const FLOAT Alpha)
{
	// To ensure the 'shortest route', we make sure the dot product between the both rotations is positive.
	if( (A | B) < 0.f )
	{
		return (B * Alpha) - (A * (1.f - Alpha));
	}

	// Then add on the second rotation..
	return (B * Alpha) + (A * (1.f - Alpha));
}

/** 
 * Bi-Linear Quaternion interpolation.
 * Result is NOT normalized.
 */
FORCEINLINE FQuat BiLerpQuat(const FQuat& P00, const FQuat& P10, const FQuat& P01, const FQuat& P11, FLOAT FracX, FLOAT FracY)
{
	return LerpQuat(
					LerpQuat(P00,P10,FracX),
					LerpQuat(P01,P11,FracX),
					FracY
					);
}

/**
 * Spherical interpolation. Will correct alignment. Output is not normalized.
 */
FQuat SlerpQuat(const FQuat &Quat1,const FQuat &Quat2, FLOAT Slerp);
FQuat SlerpQuatFullPath(const FQuat &quat1, const FQuat &quat2, FLOAT Alpha);
FQuat SquadQuat(const FQuat& quat1, const FQuat& tang1, const FQuat& quat2, const FQuat& tang2, FLOAT Alpha);

/*-----------------------------------------------------------------------------
	FMatrix classes.
-----------------------------------------------------------------------------*/

/**
 *
 */
FORCEINLINE UBOOL MakeFrustumPlane(FLOAT A,FLOAT B,FLOAT C,FLOAT D,FPlane& OutPlane)
{
	const FLOAT	LengthSquared = A * A + B * B + C * C;
	if(LengthSquared > DELTA*DELTA)
	{
		const FLOAT	InvLength = appInvSqrt(LengthSquared);
		OutPlane = FPlane(-A * InvLength,-B * InvLength,-C * InvLength,D * InvLength);
		return 1;
	}
	else
		return 0;
}

/**
 * Matrix-matrix multiplication happens with a pre-multiple of the transpose --
 * in other words, Res = Mat1.operator*(Mat2) means Res = Mat2^T * Mat1, as
 * opposed to Res = Mat1 * Mat2.
 * Matrix elements are accessed with M[RowIndex][ColumnIndex].
 */
class FMatrix
{
public:
	union
	{
		MS_ALIGN(16) FLOAT M[4][4] GCC_ALIGN(16);
	};
	MS_ALIGN(16) static const FMatrix Identity GCC_ALIGN(16);

	// Constructors.

	FORCEINLINE FMatrix();
	FORCEINLINE FMatrix(const FPlane& InX,const FPlane& InY,const FPlane& InZ,const FPlane& InW);
	FORCEINLINE FMatrix(const FVector& InX,const FVector& InY,const FVector& InZ,const FVector& InW);

#if XBOX
	/**
	 * XMMATRIX to FMatrix conversion constructor
	 *
	 * @param InMatrix	XMMATRIX to convert to FMatrix
	 */
	FORCEINLINE FMatrix( const XMMATRIX& InMatrix );

	/**
	 * FMatrix to XMMATRIX conversion operator.
	 */
	FORCEINLINE operator XMMATRIX() const;
#endif

	// Destructor.

	FORCEINLINE ~FMatrix();

	inline void SetIdentity();

	// Concatenation operator.

	FORCEINLINE FMatrix		operator* (const FMatrix& Other) const;
	FORCEINLINE void		operator*=(const FMatrix& Other);

	// Comparison operators.

	inline UBOOL operator==(const FMatrix& Other) const;

	// Error-tolerant comparison.
	inline UBOOL Equals(const FMatrix& Other, FLOAT Tolerance=KINDA_SMALL_NUMBER) const;

	inline UBOOL operator!=(const FMatrix& Other) const;

	// Homogeneous transform.

	FORCEINLINE FVector4 TransformFVector4(const FVector4& V) const;

	// Regular transform.

	/** Transform a location - will take into account translation part of the FMatrix. */
	FORCEINLINE FVector4 TransformFVector(const FVector &V) const;

	/** Inverts the matrix and then transforms V - correctly handles scaling in this matrix. */
	FORCEINLINE FVector InverseTransformFVector(const FVector &V) const;

	/** Faster version of InverseTransformFVector that assumes no scaling. WARNING: Will NOT work correctly if there is scaling in the matrix. */
	FORCEINLINE FVector InverseTransformFVectorNoScale(const FVector &V) const;

	// Normal transform.

	/** 
	 *	Transform a direction vector - will not take into account translation part of the FMatrix. 
	 *	If you want to transform a surface normal (or plane) and correctly account for non-uniform scaling you should use TransformByUsingAdjointT.
	 */
	FORCEINLINE FVector4 TransformNormal(const FVector& V) const;

	/** 
	 *	Transform a direction vector by the inverse of this matrix - will not take into account translation part.
	 *	If you want to transform a surface normal (or plane) and correctly account for non-uniform scaling you should use TransformByUsingAdjointT with adjoint of matrix inverse.
	 */
	FORCEINLINE FVector InverseTransformNormal(const FVector &V) const;

	/** Faster version of InverseTransformNormal that assumes no scaling. WARNING: Will NOT work correctly if there is scaling in the matrix. */
	FORCEINLINE FVector InverseTransformNormalNoScale(const FVector &V) const;

	// Transpose.

	FORCEINLINE FMatrix Transpose() const;

	// Determinant.

	inline FLOAT Determinant() const;

	/** Calculate determinant of rotation 3x3 matrix */
	inline FLOAT RotDeterminant() const;

	// Inverse.
	/** Fast path, doesn't check for nil matrices in final release builds */
	inline FMatrix Inverse() const;
	/** Fast path, and handles nil matrices. */
	inline FMatrix InverseSafe() const;
	/** Slow and safe path */
	inline FMatrix InverseSlow() const;

	inline FMatrix TransposeAdjoint() const;

	// Remove any scaling from this matrix (ie magnitude of each row is 1)
	inline void RemoveScaling(FLOAT Tolerance=SMALL_NUMBER);

	// Remove any translation from this matrix
	inline FMatrix RemoveTranslation() const;

	/** Returns a matrix with an additional translation concatenated. */
	inline FMatrix ConcatTranslation(const FVector& Translation) const;

	/** Returns TRUE if any element of this matrix is NaN */
	inline UBOOL ContainsNaN() const;

	inline void ScaleTranslation(const FVector& Scale3D);

	// GetOrigin

	inline FVector GetOrigin() const;

	inline FVector GetAxis(INT i) const;

	inline void GetAxes(FVector &X, FVector &Y, FVector &Z) const;

	inline void SetAxis( INT i, const FVector& Axis );

	inline void SetOrigin( const FVector& NewOrigin );

	inline void SetAxes(FVector* Axis0 = NULL, FVector* Axis1 = NULL, FVector* Axis2 = NULL, FVector* Origin = NULL);

	inline FVector GetColumn(INT i) const;

	FRotator Rotator() const;

	// Frustum plane extraction.
	FORCEINLINE UBOOL GetFrustumNearPlane(FPlane& OutPlane) const;

	FORCEINLINE UBOOL GetFrustumFarPlane(FPlane& OutPlane) const;

	FORCEINLINE UBOOL GetFrustumLeftPlane(FPlane& OutPlane) const;

	FORCEINLINE UBOOL GetFrustumRightPlane(FPlane& OutPlane) const;

	FORCEINLINE UBOOL GetFrustumTopPlane(FPlane& OutPlane) const;

	FORCEINLINE UBOOL GetFrustumBottomPlane(FPlane& OutPlane) const;

	/**
	 * Utility for mirroring this transform across a certain plane,
	 * and flipping one of the axis as well.
	 */
	inline void Mirror(BYTE MirrorAxis, BYTE FlipAxis);

	/** Output matrix as a string */
	FString ToString() const;

	// Serializer.
	inline friend FArchive& operator<<(FArchive& Ar,FMatrix& M);
};

#include "UnMatrix.h"


/**
 * A storage class for compile-time fixed size matrices.
 */
template<UINT NumRows,UINT NumColumns>
class TMatrix
{
public:

	// Variables.
	MS_ALIGN(16) FLOAT M[NumRows][NumColumns] GCC_ALIGN(16);

	TMatrix()
	{
	}

	TMatrix(const FMatrix& InMatrix)
	{
		for (UINT RowIndex = 0; (RowIndex < NumRows) && (RowIndex < 4); RowIndex++)
		{
			for (UINT ColumnIndex = 0; (ColumnIndex < NumColumns) && (ColumnIndex < 4); ColumnIndex++)
			{
				M[RowIndex][ColumnIndex] = InMatrix.M[RowIndex][ColumnIndex];
			}
		}
	}
};

class FPerspectiveMatrix : public FMatrix
{
public:

	FPerspectiveMatrix(float FOVX, float FOVY, float MultFOVX, float MultFOVY, float MinZ, float MaxZ) :
	  FMatrix(
			FPlane(MultFOVX / appTan(FOVX),		0.0f,							0.0f,							0.0f),
			FPlane(0.0f,						MultFOVY / appTan(FOVY),		0.0f,							0.0f),
			FPlane(0.0f,						0.0f,							MaxZ / (MaxZ - MinZ),			1.0f),
			FPlane(0.0f,						0.0f,							-MinZ * (MaxZ / (MaxZ - MinZ)),	0.0f))
	{
	}

	FPerspectiveMatrix(float FOV, float Width, float Height, float MinZ, float MaxZ) :
		FMatrix(
			FPlane(1.0f / appTan(FOV),	0.0f,							0.0f,							0.0f),
			FPlane(0.0f,				Width / appTan(FOV) / Height,	0.0f,							0.0f),
			FPlane(0.0f,				0.0f,							MaxZ / (MaxZ - MinZ),			1.0f),
			FPlane(0.0f,				0.0f,							-MinZ * (MaxZ / (MaxZ - MinZ)),	0.0f))
	{
	}

#define Z_PRECISION	0.001f

	FPerspectiveMatrix(float FOV, float Width, float Height, float MinZ) :
		FMatrix(
			FPlane(1.0f / appTan(FOV),	0.0f,							0.0f,							0.0f),
			FPlane(0.0f,				Width / appTan(FOV) / Height,	0.0f,							0.0f),
			FPlane(0.0f,				0.0f,							(1.0f - Z_PRECISION),			1.0f),
			FPlane(0.0f,				0.0f,							-MinZ * (1.0f - Z_PRECISION),	0.0f))
	{
	}
};

class FOrthoMatrix : public FMatrix
{
public:

	FOrthoMatrix(float Width,float Height,float ZScale,float ZOffset) :
		FMatrix(
			FPlane(1.0f / Width,	0.0f,			0.0f,				0.0f),
			FPlane(0.0f,			1.0f / Height,	0.0f,				0.0f),
			FPlane(0.0f,			0.0f,			ZScale,				0.0f),
			FPlane(0.0f,			0.0f,			ZOffset * ZScale,	1.0f))
	{
	}
};

class FTranslationMatrix : public FMatrix
{
public:

	FTranslationMatrix(const FVector& Delta) :
		FMatrix(
			FPlane(1.0f,	0.0f,	0.0f,	0.0f),
			FPlane(0.0f,	1.0f,	0.0f,	0.0f),
			FPlane(0.0f,	0.0f,	1.0f,	0.0f),
			FPlane(Delta.X,	Delta.Y,Delta.Z,1.0f))
	{
	}
};

class FRotationMatrix : public FMatrix
{
public:
	FRotationMatrix(const FRotator& Rot)
	{
		const FLOAT	SR	= GMath.SinTab(Rot.Roll);
		const FLOAT	SP	= GMath.SinTab(Rot.Pitch);
		const FLOAT	SY	= GMath.SinTab(Rot.Yaw);
		const FLOAT	CR	= GMath.CosTab(Rot.Roll);
		const FLOAT	CP	= GMath.CosTab(Rot.Pitch);
		const FLOAT	CY	= GMath.CosTab(Rot.Yaw);

		M[0][0]	= CP * CY;
		M[0][1]	= CP * SY;
		M[0][2]	= SP;
		M[0][3]	= 0.f;

		M[1][0]	= SR * SP * CY - CR * SY;
		M[1][1]	= SR * SP * SY + CR * CY;
		M[1][2]	= - SR * CP;
		M[1][3]	= 0.f;

		M[2][0]	= -( CR * SP * CY + SR * SY );
		M[2][1]	= CY * SR - CR * SP * SY;
		M[2][2]	= CR * CP;
		M[2][3]	= 0.f;

		M[3][0]	= 0.f;
		M[3][1]	= 0.f;
		M[3][2]	= 0.f;
		M[3][3]	= 1.f;
	}
};

class FRotationTranslationMatrix : public FMatrix
{
public:
	FRotationTranslationMatrix(const FRotator& Rot, const FVector& Origin)
	{
		const FLOAT	SR	= GMath.SinTab(Rot.Roll);
		const FLOAT	SP	= GMath.SinTab(Rot.Pitch);
		const FLOAT	SY	= GMath.SinTab(Rot.Yaw);
		const FLOAT	CR	= GMath.CosTab(Rot.Roll);
		const FLOAT	CP	= GMath.CosTab(Rot.Pitch);
		const FLOAT	CY	= GMath.CosTab(Rot.Yaw);

		M[0][0]	= CP * CY;
		M[0][1]	= CP * SY;
		M[0][2]	= SP;
		M[0][3]	= 0.f;

		M[1][0]	= SR * SP * CY - CR * SY;
		M[1][1]	= SR * SP * SY + CR * CY;
		M[1][2]	= - SR * CP;
		M[1][3]	= 0.f;

		M[2][0]	= -( CR * SP * CY + SR * SY );
		M[2][1]	= CY * SR - CR * SP * SY;
		M[2][2]	= CR * CP;
		M[2][3]	= 0.f;

		M[3][0]	= Origin.X;
		M[3][1]	= Origin.Y;
		M[3][2]	= Origin.Z;
		M[3][3]	= 1.f;
	}
};

class FScaleRotationTranslationMatrix : public FMatrix
{
public:
	FScaleRotationTranslationMatrix(const FVector& Scale, const FRotator& Rot, const FVector& Origin)
	{
		const FLOAT	SR	= GMath.SinTab(Rot.Roll);
		const FLOAT	SP	= GMath.SinTab(Rot.Pitch);
		const FLOAT	SY	= GMath.SinTab(Rot.Yaw);
		const FLOAT	CR	= GMath.CosTab(Rot.Roll);
		const FLOAT	CP	= GMath.CosTab(Rot.Pitch);
		const FLOAT	CY	= GMath.CosTab(Rot.Yaw);

		M[0][0]	= (CP * CY) * Scale.X;
		M[0][1]	= (CP * SY) * Scale.X;
		M[0][2]	= (SP) * Scale.X;
		M[0][3]	= 0.f;

		M[1][0]	= (SR * SP * CY - CR * SY) * Scale.Y;
		M[1][1]	= (SR * SP * SY + CR * CY) * Scale.Y;
		M[1][2]	= (- SR * CP) * Scale.Y;
		M[1][3]	= 0.f;

		M[2][0]	= ( -( CR * SP * CY + SR * SY ) ) * Scale.Z;
		M[2][1]	= (CY * SR - CR * SP * SY) * Scale.Z;
		M[2][2]	= (CR * CP) * Scale.Z;
		M[2][3]	= 0.f;

		M[3][0]	= Origin.X;
		M[3][1]	= Origin.Y;
		M[3][2]	= Origin.Z;
		M[3][3]	= 1.f;
	}
};

class FQuatRotationTranslationMatrix : public FMatrix
{
public:
	FQuatRotationTranslationMatrix(const FQuat& Q, const FVector& Origin)
	{
#if !FINAL_RELEASE && !CONSOLE
		// Make sure Quaternion is normalized
		check( Q.IsNormalized() );
#endif
		const FLOAT x2 = Q.X + Q.X;  const FLOAT y2 = Q.Y + Q.Y;  const FLOAT z2 = Q.Z + Q.Z;
		const FLOAT xx = Q.X * x2;   const FLOAT xy = Q.X * y2;   const FLOAT xz = Q.X * z2;
		const FLOAT yy = Q.Y * y2;   const FLOAT yz = Q.Y * z2;   const FLOAT zz = Q.Z * z2;
		const FLOAT wx = Q.W * x2;   const FLOAT wy = Q.W * y2;   const FLOAT wz = Q.W * z2;

		M[0][0] = 1.0f - (yy + zz);	M[1][0] = xy - wz;				M[2][0] = xz + wy;			M[3][0] = Origin.X;
		M[0][1] = xy + wz;			M[1][1] = 1.0f - (xx + zz);		M[2][1] = yz - wx;			M[3][1] = Origin.Y;
		M[0][2] = xz - wy;			M[1][2] = yz + wx;				M[2][2] = 1.0f - (xx + yy);	M[3][2] = Origin.Z;
		M[0][3] = 0.0f;				M[1][3] = 0.0f;					M[2][3] = 0.0f;				M[3][3] = 1.0f;
	}
};

class FInverseRotationMatrix : public FMatrix
{
public:

	FInverseRotationMatrix(const FRotator& Rot) :
		FMatrix(
			FMatrix(	// Yaw
				FPlane(+GMath.CosTab(-Rot.Yaw),	+GMath.SinTab(-Rot.Yaw), 0.0f,	0.0f),
				FPlane(-GMath.SinTab(-Rot.Yaw),	+GMath.CosTab(-Rot.Yaw), 0.0f,	0.0f),
				FPlane(0.0f,					0.0f,					1.0f,	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,	1.0f)) *
			FMatrix(	// Pitch
				FPlane(+GMath.CosTab(-Rot.Pitch),0.0f,					+GMath.SinTab(-Rot.Pitch),	0.0f),
				FPlane(0.0f,					1.0f,					0.0f,						0.0f),
				FPlane(-GMath.SinTab(-Rot.Pitch),0.0f,					+GMath.CosTab(-Rot.Pitch),	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,						1.0f)) *
			FMatrix(	// Roll
				FPlane(1.0f,					0.0f,					0.0f,						0.0f),
				FPlane(0.0f,					+GMath.CosTab(-Rot.Roll),-GMath.SinTab(-Rot.Roll),	0.0f),
				FPlane(0.0f,					+GMath.SinTab(-Rot.Roll),+GMath.CosTab(-Rot.Roll),	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,						1.0f))
			)
	{
	}
};

class FScaleMatrix : public FMatrix
{
public:

	/**
	 * Uniform scale.
	 */
	FScaleMatrix(FLOAT Scale) :
	  FMatrix(
		  FPlane(Scale,	0.0f,	0.0f,	0.0f),
		  FPlane(0.0f,	Scale,	0.0f,	0.0f),
		  FPlane(0.0f,	0.0f,	Scale,	0.0f),
		  FPlane(0.0f,	0.0f,	0.0f,	1.0f))
	  {
	  }

	/**
	 * Non-uniform scale.
	 */
	FScaleMatrix(const FVector& Scale) :
		FMatrix(
			FPlane(Scale.X,	0.0f,		0.0f,		0.0f),
			FPlane(0.0f,	Scale.Y,	0.0f,		0.0f),
			FPlane(0.0f,	0.0f,		Scale.Z,	0.0f),
			FPlane(0.0f,	0.0f,		0.0f,		1.0f))
	{
	}
};

//
//	FBasisVectorMatrix
//

struct FBasisVectorMatrix: FMatrix
{
	FBasisVectorMatrix(const FVector& XAxis,const FVector& YAxis,const FVector& ZAxis,const FVector& Origin)
	{
		for(UINT RowIndex = 0;RowIndex < 3;RowIndex++)
		{
			M[RowIndex][0] = (&XAxis.X)[RowIndex];
			M[RowIndex][1] = (&YAxis.X)[RowIndex];
			M[RowIndex][2] = (&ZAxis.X)[RowIndex];
			M[RowIndex][3] = 0.0f;
		}
		M[3][0] = Origin | XAxis;
		M[3][1] = Origin | YAxis;
		M[3][2] = Origin | ZAxis;
		M[3][3] = 1.0f;
	}
};

/**
 * Mirrors a point about an abitrary plane 
 */
class FMirrorMatrix : public FMatrix
{
public:
	/** 
	 * Constructor.
	 * Updated for the fact that our FPlane uses Ax+By+Cz=D.
	 * 
	 * @param	Plane - source plane for mirroring (assumed normalized)
	 */
	FMirrorMatrix( const FPlane& Plane ) :
	  FMatrix(
		  FPlane( -2.f*Plane.X*Plane.X + 1.f,	-2.f*Plane.Y*Plane.X,		-2.f*Plane.Z*Plane.X,		0.f ),
		  FPlane( -2.f*Plane.X*Plane.Y,			-2.f*Plane.Y*Plane.Y + 1.f,	-2.f*Plane.Z*Plane.Y,		0.f ),
		  FPlane( -2.f*Plane.X*Plane.Z,			-2.f*Plane.Y*Plane.Z,		-2.f*Plane.Z*Plane.Z + 1.f,	0.f ),
		  FPlane(  2.f*Plane.X*Plane.W,			 2.f*Plane.Y*Plane.W,		 2.f*Plane.Z*Plane.W,		1.f ) )
	{
		//check( Abs(1.f - Plane.SizeSquared()) < KINDA_SMALL_NUMBER && TEXT("not normalized"));
	}
};

/**
 * Realigns the near plane for an existing projection matrix 
 * with an arbitrary clip plane
 * from: http://www.terathon.com/code/obliqued3d.html
 * Updated for the fact that our FPlane uses Ax+By+Cz=D.
 */
class FClipProjectionMatrix : public FMatrix
{
public:
	/**
	 * Constructor
	 *
	 * @param	SrcProjMat - source projection matrix to premultiply with the clip matrix
	 * @param	Plane - clipping plane used to build the clip matrix (assumed to be in camera space)
	 */
	FClipProjectionMatrix( const FMatrix& SrcProjMat, const FPlane& Plane ) :
	  FMatrix(SrcProjMat)
	{
		// Calculate the clip-space corner point opposite the clipping plane
		// as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
		// transform it into camera space by multiplying it
		// by the inverse of the projection matrix
		FPlane CornerPlane( 
			sgn(Plane.X) / SrcProjMat.M[0][0],
			sgn(Plane.Y) / SrcProjMat.M[1][1],
            1.0f,
			-(1.0f - SrcProjMat.M[2][2]) / SrcProjMat.M[3][2]
			);

		// Calculate the scaled plane vector
		FPlane ProjPlane( Plane * (1.0f / (Plane | CornerPlane)) );

		// use the projected space clip plane in z column 
		// Note: (account for our negated W coefficient)
		M[0][2] = ProjPlane.X;
		M[1][2] = ProjPlane.Y;
		M[2][2] = ProjPlane.Z;
		M[3][2] = -ProjPlane.W;
	}

private:
	/** return sign of a number */
	FORCEINLINE FLOAT sgn( FLOAT a )
	{
		if (a > 0.0f) return (1.0f);
		if (a < 0.0f) return (-1.0f);
		return (0.0f);
	}
};

/*-----------------------------------------------------------------------------
	FPlane implementation.
-----------------------------------------------------------------------------*/

/**
 * Transform a point by a coordinate system, moving
 * it by the coordinate system's origin if nonzero.
 */
inline FPlane FPlane::TransformPlaneByOrtho( const FMatrix& M ) const
{
	const FVector4 Normal = M.TransformNormal(*this);
	return FPlane( Normal, W - Dot3(M.TransformFVector(FVector(0,0,0)), Normal) );
}

inline FPlane FPlane::TransformBy( const FMatrix& M ) const
{
	const FMatrix tmpTA = M.TransposeAdjoint();
	const float DetM = M.Determinant();
	return this->TransformByUsingAdjointT(M, DetM, tmpTA);
}

/**
 * You can optionally pass in the matrices transpose-adjoint, which save it recalculating it.
 * MSM: If we are going to save the transpose-adjoint we should also save the more expensive
 * determinant.
 */
inline FPlane FPlane::TransformByUsingAdjointT( const FMatrix& M, float DetM, const FMatrix& TA ) const
{
	FVector newNorm = TA.TransformNormal(*this).SafeNormal();

	if(DetM < 0.f)
		newNorm *= -1.0f;

	return FPlane(M.TransformFVector(*this * W), newNorm);
}

inline FSphere FSphere::TransformBy(const FMatrix& M) const
{
	FSphere	Result;

	(FVector&)Result = M.TransformFVector(*this);

	const FVector XAxis(M.M[0][0],M.M[0][1],M.M[0][2]);
	const FVector YAxis(M.M[1][0],M.M[1][1],M.M[1][2]);
	const FVector ZAxis(M.M[2][0],M.M[2][1],M.M[2][2]);

	Result.W = appSqrt(Max(XAxis|XAxis,Max(YAxis|YAxis,ZAxis|ZAxis))) * W;

	return Result;
}

inline FBoxSphereBounds FBoxSphereBounds::TransformBy(const FMatrix& M) const
{
	FBoxSphereBounds	Result;

	Result.Origin = M.TransformFVector(Origin);
	Result.BoxExtent = FVector(0,0,0);
	FLOAT	Signs[2] = { -1.0f, 1.0f };
	for(INT X = 0;X < 2;X++)
	{
		for(INT Y = 0;Y < 2;Y++)
		{
			for(INT Z = 0;Z < 2;Z++)
			{
				FVector	Corner = M.TransformNormal(FVector(Signs[X] * BoxExtent.X,Signs[Y] * BoxExtent.Y,Signs[Z] * BoxExtent.Z));
				Result.BoxExtent.X = Max(Corner.X,Result.BoxExtent.X);
				Result.BoxExtent.Y = Max(Corner.Y,Result.BoxExtent.Y);
				Result.BoxExtent.Z = Max(Corner.Z,Result.BoxExtent.Z);
			}
		}
	}

	const FVector XAxis(M.M[0][0],M.M[0][1],M.M[0][2]);
	const FVector YAxis(M.M[1][0],M.M[1][1],M.M[1][2]);
	const FVector ZAxis(M.M[2][0],M.M[2][1],M.M[2][2]);

	Result.SphereRadius = appSqrt(Max(XAxis|XAxis,Max(YAxis|YAxis,ZAxis|ZAxis))) * SphereRadius;

	return Result;
}

/*-----------------------------------------------------------------------------
	Bezier curves
-----------------------------------------------------------------------------*/

/**
 * Generates a list of sample points on a Bezier curve defined by 2 points.
 *
 * @param	ControlPoints	Array of 4 FVectors (vert1, controlpoint1, controlpoint2, vert2).
 * @param	NumPoints		Number of samples.
 * @param	OutPoints		Receives the output samples.
 * @return					Path length.
 */
FLOAT EvaluateBezier(const FVector* ControlPoints, INT NumPoints, TArray<FVector>& OutPoints);

/*-----------------------------------------------------------------------------
	FInterpCurve.
-----------------------------------------------------------------------------*/

/**
 * Clamps a tangent formed by the specified control point values
 */
FLOAT FClampFloatTangent( FLOAT PrevPointVal, FLOAT PrevTime, FLOAT CurPointVal, FLOAT CurTime, FLOAT NextPointVal, FLOAT NextTime );


template< class T, class U > void LegacyAutoCalcTangent( const T& PrevP, const T& P, const T& NextP, const U& Tension, T& OutTan )
{
	OutTan = 0.5f * (1.f - Tension) * ( (P - PrevP) + (NextP - P) );
}

template< class T, class U > void AutoCalcTangent( const T& PrevP, const T& P, const T& NextP, const U& Tension, T& OutTan )
{
	OutTan = (1.f - Tension) * ( (P - PrevP) + (NextP - P) );
}

//////////////////////////////////////////////////////////////////////////
// Support for InterpCurves of Quaternions
template< class U > FQuat Lerp( const FQuat& A, const FQuat& B, const U& Alpha)
{
	return SlerpQuat(A, B, Alpha);
}

inline FQuat BiLerp(const FQuat& P00, const FQuat& P10, const FQuat& P01, const FQuat& P11, FLOAT FracX, FLOAT FracY)
{
	FQuat Result;

	Result = Lerp(
				Lerp(P00,P10,FracX),
				Lerp(P01,P11,FracX),
				FracY
				);

	Result.Normalize();

	return Result;
}


/**
 * In the case of quaternions, we use a bezier like approach.
 * T - Actual 'control' orientations.
 */
template< class U > FQuat CubicInterp( const FQuat& P0, const FQuat& T0, const FQuat& P1, const FQuat& T1, const U& A)
{
	return SquadQuat(P0, T0, P1, T1, A);
}


void LegacyCalcQuatTangents( const FQuat& PrevP, const FQuat& P, const FQuat& NextP, FLOAT Tension, FQuat& OutTan );

template< class U > void LegacyAutoCalcTangent( const FQuat& PrevP, const FQuat& P, const FQuat& NextP, const U& Tension, FQuat& OutTan  )
{
	LegacyCalcQuatTangents(PrevP, P, NextP, Tension, OutTan);
}

void CalcQuatTangents( const FQuat& PrevP, const FQuat& P, const FQuat& NextP, FLOAT Tension, FQuat& OutTan );

/**
 * This actually returns the control point not a tangent. This is expected by the CubicInterp function for Quaternions above.
 */
template< class U > void AutoCalcTangent( const FQuat& PrevP, const FQuat& P, const FQuat& NextP, const U& Tension, FQuat& OutTan  )
{
	CalcQuatTangents(PrevP, P, NextP, Tension, OutTan);
}



/** Computes a tangent for the specified control point.  General case, doesn't support clamping. */
template< class T >
void ComputeCurveTangent( FLOAT PrevTime, const T& PrevPoint,
						  FLOAT CurTime, const T& CurPoint,
						  FLOAT NextTime, const T& NextPoint,
						  FLOAT Tension,
						  UBOOL bWantClamping,
						  T& OutTangent )
{
	// NOTE: Clamping not supported for non-float vector types (bWantClamping is ignored)

	AutoCalcTangent( PrevPoint, CurPoint, NextPoint, Tension, OutTangent );

	const FLOAT PrevToNextTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, NextTime - PrevTime );

	OutTangent /= PrevToNextTimeDiff;
}



/**
 * Computes a tangent for the specified control point; supports clamping, but only works
 * with floats or contiguous arrays of floats.
 */
template< class T >
void ComputeClampableFloatVectorCurveTangent( FLOAT PrevTime, const T& PrevPoint,
											  FLOAT CurTime, const T& CurPoint,
											  FLOAT NextTime, const T& NextPoint,
											  FLOAT Tension,
											  UBOOL bWantClamping,
											  T& OutTangent )
{
	const FLOAT PrevToNextTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, NextTime - PrevTime );

	// Clamp the tangents if we need to do that
	if( bWantClamping )
	{
		// NOTE: We always treat the type as an array of floats
		FLOAT* PrevPointVal = ( FLOAT* )&PrevPoint;
		FLOAT* CurPointVal = ( FLOAT* )&CurPoint;
		FLOAT* NextPointVal = ( FLOAT* )&NextPoint;
		FLOAT* OutTangentVal = ( FLOAT* )&OutTangent;
		for( INT CurValPos = 0; CurValPos < sizeof( T ); CurValPos += sizeof( FLOAT ) )
		{
			// Clamp it!
			const FLOAT ClampedTangent =
				FClampFloatTangent(
					*PrevPointVal, PrevTime,
					*CurPointVal, CurTime,
					*NextPointVal, NextTime );

			// Apply tension value
			*OutTangentVal = ( 1.0f - Tension ) * ClampedTangent;


			// Advance pointers
			++OutTangentVal;
			++PrevPointVal;
			++CurPointVal;
			++NextPointVal;
		}
	}
	else
	{
		// No clamping needed
		AutoCalcTangent( PrevPoint, CurPoint, NextPoint, Tension, OutTangent );

		const FLOAT PrevToNextTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, NextTime - PrevTime );

		OutTangent /= PrevToNextTimeDiff;
	}
}


/** Computes a tangent for the specified control point.  Special case for FLOAT types; supports clamping. */
inline void ComputeCurveTangent( FLOAT PrevTime, const FLOAT& PrevPoint,
								 FLOAT CurTime, const FLOAT& CurPoint,
								 FLOAT NextTime, const FLOAT& NextPoint,
								 FLOAT Tension,
								 UBOOL bWantClamping,
								 FLOAT& OutTangent )
{
	ComputeClampableFloatVectorCurveTangent(
		PrevTime, PrevPoint,
		CurTime, CurPoint,
		NextTime, NextPoint,
		Tension, bWantClamping, OutTangent );
}


/** Computes a tangent for the specified control point.  Special case for FVector types; supports clamping. */
inline void ComputeCurveTangent( FLOAT PrevTime, const FVector& PrevPoint,
								 FLOAT CurTime, const FVector& CurPoint,
								 FLOAT NextTime, const FVector& NextPoint,
								 FLOAT Tension,
								 UBOOL bWantClamping,
								 FVector& OutTangent )
{
	ComputeClampableFloatVectorCurveTangent(
		PrevTime, PrevPoint,
		CurTime, CurPoint,
		NextTime, NextPoint,
		Tension, bWantClamping, OutTangent );
}


/** Computes a tangent for the specified control point.  Special case for FVector2D types; supports clamping. */
inline void ComputeCurveTangent( FLOAT PrevTime, const FVector2D& PrevPoint,
								 FLOAT CurTime, const FVector2D& CurPoint,
								 FLOAT NextTime, const FVector2D& NextPoint,
								 FLOAT Tension,
								 UBOOL bWantClamping,
								 FVector2D& OutTangent )
{
	ComputeClampableFloatVectorCurveTangent(
		PrevTime, PrevPoint,
		CurTime, CurPoint,
		NextTime, NextPoint,
		Tension, bWantClamping, OutTangent );
}


/** Computes a tangent for the specified control point.  Special case for FTwoVectors types; supports clamping. */
inline void ComputeCurveTangent( FLOAT PrevTime, const FTwoVectors& PrevPoint,
								 FLOAT CurTime, const FTwoVectors& CurPoint,
								 FLOAT NextTime, const FTwoVectors& NextPoint,
								 FLOAT Tension,
								 UBOOL bWantClamping,
								 FTwoVectors& OutTangent )
{
	ComputeClampableFloatVectorCurveTangent(
		PrevTime, PrevPoint,
		CurTime, CurPoint,
		NextTime, NextPoint,
		Tension, bWantClamping, OutTangent );
}



//////////////////////////////////////////////////////////////////////////

enum EInterpCurveMode
{
	/** A straight line between two keypoint values. */
	CIM_Linear,

	/** A cubic-hermite curve between two keypoints, using Arrive/Leave tangents. These tangents will be automatically
	    updated when points are moved, etc.  Tangents are unclamped and will plateau at curve start and end points. */
	CIM_CurveAuto,

	/** The out value is held constant until the next key, then will jump to that value. */
	CIM_Constant,

	/** A smooth curve just like CIM_Curve, but tangents are not automatically updated so you can have manual control over them (eg. in Curve Editor). */
	CIM_CurveUser,

	/** A curve like CIM_Curve, but the arrive and leave tangents are not forced to be the same, so you can create a 'corner' at this key. */
	CIM_CurveBreak,

	/** A cubic-hermite curve between two keypoints, using Arrive/Leave tangents. These tangents will be automatically
	    updated when points are moved, etc.  Tangents are clamped and will plateau at curve start and end points. */
	CIM_CurveAutoClamped,

	/** Invalid or unknown curve type. */
	CIM_Unknown
};

template< class T > class FInterpCurvePoint
{
public:
	/** Float input value that corresponds to this key (eg. time). */
	FLOAT		InVal;

	/** Output value of templated type when input is equal to InVal. */
	T			OutVal;

	/** Tangent of curve arrive this point. */
	T			ArriveTangent; 

	/** Tangent of curve leaving this point. */
	T			LeaveTangent; 

	/** Interpolation mode between this point and the next one. @see EInterpCurveMode */
	BYTE		InterpMode; 

	FInterpCurvePoint() {}

	FInterpCurvePoint(const FLOAT In, const T &Out) : 
	InVal(In), 
		OutVal(Out)
	{
		appMemset( &ArriveTangent, 0, sizeof(T) );	
		appMemset( &LeaveTangent, 0, sizeof(T) );

		InterpMode = CIM_Linear;
	}

	FInterpCurvePoint(const FLOAT In, const T &Out, const T &InArriveTangent, const T &InLeaveTangent, const EInterpCurveMode InInterpMode) : 
	InVal(In), 
		OutVal(Out), 
		ArriveTangent(InArriveTangent),
		LeaveTangent(InLeaveTangent),
		InterpMode(InInterpMode)
	{
	}

	FORCEINLINE UBOOL IsCurveKey() const
	{
		return ((InterpMode == CIM_CurveAuto) || (InterpMode == CIM_CurveAutoClamped) || (InterpMode == CIM_CurveUser) || (InterpMode == CIM_CurveBreak));
	}


	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FInterpCurvePoint& Point )
	{
		Ar << Point.InVal << Point.OutVal;
		Ar << Point.ArriveTangent << Point.LeaveTangent;
		Ar << Point.InterpMode;
		return Ar;
	}
};


void CurveFloatFindIntervalBounds( const FInterpCurvePoint<FLOAT>& Start, const FInterpCurvePoint<FLOAT>& End, FLOAT& CurrentMin, FLOAT& CurrentMax );
void CurveVector2DFindIntervalBounds( const FInterpCurvePoint<FVector2D>& Start, const FInterpCurvePoint<FVector2D>& End, FVector2D& CurrentMin, FVector2D& CurrentMax );
void CurveVectorFindIntervalBounds( const FInterpCurvePoint<FVector>& Start, const FInterpCurvePoint<FVector>& End, FVector& CurrentMin, FVector& CurrentMax );
void CurveTwoVectorsFindIntervalBounds(const FInterpCurvePoint<FTwoVectors>& Start, const FInterpCurvePoint<FTwoVectors>& End, FTwoVectors& CurrentMin, FTwoVectors& CurrentMax);

template< class T, class U > void CurveFindIntervalBounds( const FInterpCurvePoint<T>& Start, const FInterpCurvePoint<T>& End, T& CurrentMin, T& CurrentMax, const U& Dummy )
{ }

template< class U > void CurveFindIntervalBounds( const FInterpCurvePoint<FLOAT>& Start, const FInterpCurvePoint<FLOAT>& End, FLOAT& CurrentMin, FLOAT& CurrentMax, const U& Dummy )
{
	CurveFloatFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
}

template< class U > void CurveFindIntervalBounds( const FInterpCurvePoint<FVector2D>& Start, const FInterpCurvePoint<FVector2D>& End, FVector2D& CurrentMin, FVector2D& CurrentMax, const U& Dummy )
{
	CurveVector2DFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
}

template< class U > void CurveFindIntervalBounds( const FInterpCurvePoint<FVector>& Start, const FInterpCurvePoint<FVector>& End, FVector& CurrentMin, FVector& CurrentMax, const U& Dummy )
{
	CurveVectorFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
}

template< class U > void CurveFindIntervalBounds( const FInterpCurvePoint<FTwoVectors>& Start, const FInterpCurvePoint<FTwoVectors>& End, FTwoVectors& CurrentMin, FTwoVectors& CurrentMax, const U& Dummy )
{
	CurveTwoVectorsFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
}


/** Which interpolation method to use, needed for backwards compatibility. */
enum EInterpMethodType
{
	IMT_UseFixedTangentEvalAndNewAutoTangents,
	IMT_UseFixedTangentEval,
	IMT_UseBrokenTangentEval
};


template< class T > class FInterpCurve
{
public:
	TArrayNoInit< FInterpCurvePoint<T> >	Points;

	BYTE InterpMethod;

	/** Add a new keypoint to the InterpCurve with the supplied In and Out value. Returns the index of the new key.*/
	INT AddPoint( const FLOAT InVal, const T &OutVal )
	{
		INT i=0; for( i=0; i<Points.Num() && Points(i).InVal < InVal; i++);
		Points.Insert(i);
		Points(i) = FInterpCurvePoint< T >(InVal, OutVal);
		return i;
	}

	/** Move a keypoint to a new In value. This may change the index of the keypoint, so the new key index is returned. */
	INT MovePoint( INT PointIndex, FLOAT NewInVal )
	{
		if( PointIndex < 0 || PointIndex >= Points.Num() )
			return PointIndex;

		const T OutVal = Points(PointIndex).OutVal;
		const BYTE Mode = Points(PointIndex).InterpMode;
		const T ArriveTan = Points(PointIndex).ArriveTangent;
		const T LeaveTan = Points(PointIndex).LeaveTangent;

		Points.Remove(PointIndex);

		const INT NewPointIndex = AddPoint( NewInVal, OutVal );
		Points(NewPointIndex).InterpMode = Mode;
		Points(NewPointIndex).ArriveTangent = ArriveTan;
		Points(NewPointIndex).LeaveTangent = LeaveTan;

		return NewPointIndex;
	}

	/** Clear all keypoints from InterpCurve. */
	void Reset()
	{
		Points.Empty();
	}

	/** 
	 *	Evaluate the output for an arbitary input value. 
	 *	For inputs outside the range of the keys, the first/last key value is assumed.
	 */
	T Eval( const FLOAT InVal, const T& Default, INT* PtIdx = NULL ) const
	{
		const INT NumPoints = Points.Num();

		// If no point in curve, return the Default value we passed in.
		if( NumPoints == 0 )
		{
			if( PtIdx )
			{
				*PtIdx = -1;
			}
			return Default;
		}

		// If only one point, or before the first point in the curve, return the first points value.
		if( NumPoints < 2 || (InVal <= Points(0).InVal) )
		{
			if( PtIdx )
			{
				*PtIdx = 0;
			}
			return Points(0).OutVal;
		}

		// If beyond the last point in the curve, return its value.
		if( InVal >= Points(NumPoints-1).InVal )
		{
			if( PtIdx )
			{
				*PtIdx = NumPoints - 1;
			}
			return Points(NumPoints-1).OutVal;
		}

		// Somewhere with curve range - linear search to find value.
		for( INT i=1; i<NumPoints; i++ )
		{	
			if( InVal < Points(i).InVal )
			{
				const FLOAT Diff = Points(i).InVal - Points(i-1).InVal;

				if( Diff > 0.f && Points(i-1).InterpMode != CIM_Constant )
				{
					const FLOAT Alpha = (InVal - Points(i-1).InVal) / Diff;

					if( PtIdx )
					{
						*PtIdx = i - 1;
					}

					if( Points(i-1).InterpMode == CIM_Linear )
					{
						return Lerp( Points(i-1).OutVal, Points(i).OutVal, Alpha );
					}
					else
					{
						if(InterpMethod == IMT_UseBrokenTangentEval)
						{
							return CubicInterp( Points(i-1).OutVal, Points(i-1).LeaveTangent, Points(i).OutVal, Points(i).ArriveTangent, Alpha );
						}
						else
						{
							return CubicInterp( Points(i-1).OutVal, Points(i-1).LeaveTangent * Diff, Points(i).OutVal, Points(i).ArriveTangent * Diff, Alpha );
						}
					}
				}
				else
				{
					if( PtIdx )
					{
						*PtIdx = i - 1;
					}

					return Points(i-1).OutVal;
				}
			}
		}

		// Shouldn't really reach here.
		if( PtIdx )
		{
			*PtIdx = NumPoints - 1;
		}

		return Points(NumPoints-1).OutVal;
	}

	T EvalDerivative( const FLOAT InVal, const T& Default, INT* PtIdx = NULL ) const
	{
		const INT NumPoints = Points.Num();

		// If no point in curve, return the Default value we passed in.
		if( NumPoints == 0 )
		{
			if( PtIdx )
			{
				*PtIdx = -1;
			}
			return Default;
		}

		// If only one point, or before the first point in the curve, return the first points value.
		if( NumPoints < 2 || (InVal <= Points(0).InVal) )
		{
			if( PtIdx )
			{
				*PtIdx = 0;
			}
			return Points(0).OutVal;
		}

		// If beyond the last point in the curve, return its value.
		if( InVal >= Points(NumPoints-1).InVal )
		{
			if( PtIdx )
			{
				*PtIdx = NumPoints - 1;
			}
			return Points(NumPoints-1).OutVal;
		}

		// Somewhere with curve range - linear search to find value.
		for( INT i=1; i<NumPoints; i++ )
		{	
			if( InVal < Points(i).InVal )
			{
				const FLOAT Diff = Points(i).InVal - Points(i-1).InVal;

				if( Diff > 0.f && Points(i-1).InterpMode != CIM_Constant )
				{
					if( PtIdx )
					{
						*PtIdx = NumPoints - 1;
					}

					const FLOAT Alpha = (InVal - Points(i-1).InVal) / Diff;

					if( Points(i-1).InterpMode == CIM_Linear )
					{
						return Lerp( Points(i-1).OutVal, Points(i).OutVal, Alpha );
					}
					else
					{
						return CubicInterpDerivative( Points(i-1).OutVal, Points(i-1).LeaveTangent * Diff, Points(i).OutVal, Points(i).ArriveTangent * Diff, Alpha );
					}
				}
				else
				{
					if( PtIdx )
					{
						*PtIdx = -1;
					}

					return FVector(0.f);
				}
			}
		}

		if( PtIdx )
		{
			*PtIdx = NumPoints - 1;
		}

		// Shouldn't really reach here.
		return Points(NumPoints-1).OutVal;
	}

	T EvalSecondDerivative( const FLOAT InVal, const T& Default, INT* PtIdx = NULL ) const
	{
		const INT NumPoints = Points.Num();

		// If no point in curve, return the Default value we passed in.
		if( NumPoints == 0 )
		{
			if( PtIdx )
			{
				*PtIdx = -1;
			}
			return Default;
		}

		// If only one point, or before the first point in the curve, return the first points value.
		if( NumPoints < 2 || (InVal <= Points(0).InVal) )
		{
			if( PtIdx )
			{
				*PtIdx = 0;
			}
			return Points(0).OutVal;
		}

		// If beyond the last point in the curve, return its value.
		if( InVal >= Points(NumPoints-1).InVal )
		{
			if( PtIdx )
			{
				*PtIdx = NumPoints - 1;
			}
			return Points(NumPoints-1).OutVal;
		}

		// Somewhere with curve range - linear search to find value.
		for( INT i=1; i<NumPoints; i++ )
		{	
			if( InVal < Points(i).InVal )
			{
				const FLOAT Diff = Points(i).InVal - Points(i-1).InVal;

				if( Diff > 0.f && Points(i-1).InterpMode != CIM_Constant )
				{
					if( PtIdx )
					{
						*PtIdx = i - 1;
					}
					const FLOAT Alpha = (InVal - Points(i-1).InVal) / Diff;

					if( Points(i-1).InterpMode == CIM_Linear )
					{
						return Lerp( Points(i-1).OutVal, Points(i).OutVal, Alpha );
					}
					else
					{
						return CubicInterpSecondDerivative( Points(i-1).OutVal, Points(i-1).LeaveTangent * Diff, Points(i).OutVal, Points(i).ArriveTangent * Diff, Alpha );
					}
				}
				else
				{
					if( PtIdx )
					{
						*PtIdx = -1;
					}
					return FVector(0.f);
				}
			}
		}
		if( PtIdx )
		{
			*PtIdx = NumPoints - 1;
		}

		// Shouldn't really reach here.
		return Points(NumPoints-1).OutVal;
	}


	void AutoSetTangents(FLOAT Tension = 0.f)
	{
		// Iterate over all points in this InterpCurve
		for(INT PointIndex=0; PointIndex<Points.Num(); PointIndex++)
		{
			T ArriveTangent = Points(PointIndex).ArriveTangent;
			T LeaveTangent = Points(PointIndex).LeaveTangent;

			if(PointIndex == 0)
			{
				if(PointIndex < Points.Num()-1) // Start point
				{
					// If first section is not a curve, or is a curve and first point has manual tangent setting.
					if( Points(PointIndex).InterpMode == CIM_CurveAuto || Points(PointIndex).InterpMode == CIM_CurveAutoClamped )
					{
						appMemset( &LeaveTangent, 0, sizeof(T) );
					}
				}
				else // Only point
				{
					appMemset( &LeaveTangent, 0, sizeof(T) );
				}
			}
			else
			{
				if(PointIndex < Points.Num()-1) // Inner point
				{
					if( Points(PointIndex).InterpMode == CIM_CurveAuto || Points(PointIndex).InterpMode == CIM_CurveAutoClamped )
					{
						if( Points(PointIndex-1).IsCurveKey() && Points(PointIndex).IsCurveKey() )
						{
							if( InterpMethod == IMT_UseFixedTangentEvalAndNewAutoTangents )
							{
								const UBOOL bWantClamping = ( Points( PointIndex ).InterpMode == CIM_CurveAutoClamped );

								ComputeCurveTangent(
									Points( PointIndex - 1 ).InVal,		// Previous time
									Points( PointIndex - 1 ).OutVal,	// Previous point
									Points( PointIndex ).InVal,			// Current time
									Points( PointIndex ).OutVal,		// Current point
									Points( PointIndex + 1 ).InVal,		// Next time
									Points( PointIndex + 1 ).OutVal,	// Next point
									Tension,							// Tension
									bWantClamping,						// Want clamping?
									ArriveTangent );					// Out
							}
							else
							{
								LegacyAutoCalcTangent(
									Points( PointIndex - 1 ).OutVal,
									Points( PointIndex ).OutVal,
									Points( PointIndex + 1 ).OutVal,
									Tension,
									ArriveTangent );
							}

							// In 'auto' mode, arrive and leave tangents are always the same
							LeaveTangent = ArriveTangent;
						}
						else if( Points(PointIndex-1).InterpMode == CIM_Constant || Points(PointIndex).InterpMode == CIM_Constant )
						{
							appMemset( &ArriveTangent, 0, sizeof(T) );
							appMemset( &LeaveTangent, 0, sizeof(T) );
						}
					}
				}
				else // End point
				{
					// If last section is not a curve, or is a curve and final point has manual tangent setting.
					if( Points(PointIndex).InterpMode == CIM_CurveAuto || Points(PointIndex).InterpMode == CIM_CurveAutoClamped )
					{
						appMemset( &ArriveTangent, 0, sizeof(T) );
					}
				}
			}

			Points(PointIndex).ArriveTangent = ArriveTangent;
			Points(PointIndex).LeaveTangent = LeaveTangent;
		}
	}

	/** Calculate the min/max out value that can be returned by this InterpCurve. */
	void CalcBounds(T& OutMin, T& OutMax, const T& Default) const
	{
		if(Points.Num() == 0)
		{
			OutMin = Default;
			OutMax = Default;
		}
		else if(Points.Num() == 1)
		{
			OutMin = Points(0).OutVal;
			OutMax = Points(0).OutVal;
		}
		else
		{
			OutMin = Points(0).OutVal;
			OutMax = Points(0).OutVal;

			for(INT i=1; i<Points.Num(); i++)
			{
				CurveFindIntervalBounds( Points(i-1), Points(i), OutMin, OutMax, 0.f );
			}
		}
	}


	/** Serializer */
	friend FArchive& operator<<( FArchive& Ar, FInterpCurve& Curve )
	{
		// NOTE: This is not used often for FInterpCurves.  Most of the time these are serialized
		//   as inline struct properties in UnClass.cpp!

		Ar << Curve.Points;

		if( Ar.IsLoading() && Ar.Ver() < VER_NEW_CURVE_AUTO_TANGENTS )
		{
			// Old packages store the interp method value one less than what it should be
			Ar << Curve.InterpMethod;
			Curve.InterpMethod++;
		}
		else
		{
			Ar << Curve.InterpMethod;
		}

		return Ar;
	}


	/** Returns TRUE if this curve uses legacy tangent/interp algorithms and may be 'upgraded' */
	UBOOL UsingLegacyInterpMethod() const
	{
		// Unless we're using the latest and greatest tangents, we'll report legacy interp method
		return ( InterpMethod != IMT_UseFixedTangentEvalAndNewAutoTangents );
	}


	/** 'Upgrades' this curve to use the latest tangent/interp algorithms (usually, will 'bake' key tangents.) */
	void UpgradeInterpMethod()
	{
		if( UsingLegacyInterpMethod() )
		{
			// Iterate over all points in this InterpCurve
			for( INT PointIndex = 0; PointIndex < Points.Num(); ++PointIndex )
			{
				FInterpCurvePoint<T>& CurPoint = Points( PointIndex );

				// Bake out the current tangents by setting their interp mode to 'User'
				if( CurPoint.InterpMode == CIM_CurveAuto || CurPoint.InterpMode == CIM_CurveAutoClamped )
				{
					CurPoint.InterpMode = CIM_CurveUser;
				}
			}

			// Update the interp method of this curve
			InterpMethod = IMT_UseFixedTangentEvalAndNewAutoTangents;
		}
	}

};

template< class T > class FInterpCurveInit : public FInterpCurve< T >
{
public:
	FInterpCurveInit()
	{
		appMemzero( &this->Points, sizeof(this->Points) );

		// Always use new auto-tangents unless we're working with older data
		this->InterpMethod = IMT_UseFixedTangentEvalAndNewAutoTangents;
	}
};

typedef FInterpCurve<FLOAT>				FInterpCurveFloat;
typedef FInterpCurve<FVector2D>			FInterpCurveVector2D;
typedef FInterpCurve<FVector>			FInterpCurveVector;
typedef FInterpCurve<FQuat>				FInterpCurveQuat;

typedef FInterpCurve<FTwoVectors>		FInterpCurveTwoVectors;

// These should be used if you are going to declare a variable on the stack for usage in c++ land otherwise you will assert in checkSlow(ArrayNum>=0); as these contain a TArrayNoInit
typedef FInterpCurveInit<FLOAT>			FInterpCurveInitFloat;
typedef FInterpCurveInit<FVector2D>		FInterpCurveInitVector2D;
typedef FInterpCurveInit<FVector>		FInterpCurveInitVector;
typedef FInterpCurveInit<FQuat>			FInterpCurveInitQuat;

typedef FInterpCurveInit<FTwoVectors>	FInterpCurveInitTwoVectors;

/*-----------------------------------------------------------------------------
	FCurveEdInterface
-----------------------------------------------------------------------------*/


/** Interface that allows the CurveEditor to edit this type of object. */
class FCurveEdInterface
{
public:
	/** Get number of keyframes in curve. */
	virtual INT		GetNumKeys() { return 0; }

	/** Get number of 'sub curves' in this Curve. For example, a vector curve will have 3 sub-curves, for X, Y and Z. */
	virtual INT		GetNumSubCurves() { return 0; }

	/** Get the input value for the Key with the specified index. KeyIndex must be within range ie >=0 and < NumKeys. */
	virtual FLOAT	GetKeyIn(INT KeyIndex) { return 0.f; }

	/** 
	 *	Get the output value for the key with the specified index on the specified sub-curve. 
	 *	SubIndex must be within range ie >=0 and < NumSubCurves.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual FLOAT	GetKeyOut(INT SubIndex, INT KeyIndex) { return 0.f; }

	/** Allows the Curve to specify a color for each keypoint. */
	virtual FColor	GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor) { return CurveColor; }

	/** Evaluate a subcurve at an arbitary point. Outside the keyframe range, curves are assumed to continue their end values. */
	virtual FLOAT	EvalSub(INT SubIndex, FLOAT InVal) { return 0.f; }

	/** 
	 *	Get the interpolation mode of the specified keyframe. This can be CIM_Constant, CIM_Linear or CIM_Curve. 
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual BYTE	GetKeyInterpMode(INT KeyIndex) { return CIM_Linear; }

	/** 
	 *	Get the incoming and outgoing tangent for the given subcurve and key.
	 *	SubIndex must be within range ie >=0 and < NumSubCurves.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent) { ArriveTangent=0.f; LeaveTangent=0.f; }

	/** Get input range of keys. Outside this region curve continues constantly the start/end values. */
	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn) { MinIn=0.f; MaxIn=0.f; }

	/** Get overall range of output values. */
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut) { MinOut=0.f; MaxOut=0.f; }

	/** 
	 *	Add a new key to the curve with the specified input. Its initial value is set using EvalSub at that location. 
	 *	Returns the index of the new key.
	 */
	virtual INT		CreateNewKey(FLOAT KeyIn) { return INDEX_NONE; }

	/** 
	 *	Remove the specified key from the curve.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	DeleteKey(INT KeyIndex) {}

	/** 
	 *	Set the input value of the specified Key. This may change the index of the key, so the new index of the key is retured. 
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual INT		SetKeyIn(INT KeyIndex, FLOAT NewInVal) { return KeyIndex; }

	/** 
	 *	Set the output values of the specified key.
	 *	SubIndex must be within range ie >=0 and < NumSubCurves.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) {}

	/** 
	 *	Set the method to use for interpolating between the give keyframe and the next one.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) {}


	/** 
	 *	Set the incoming and outgoing tangent for the given subcurve and key.
	 *	SubIndex must be within range ie >=0 and < NumSubCurves.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent) {}


	/** Returns TRUE if this curve uses legacy tangent/interp algorithms and may be 'upgraded' */
	virtual UBOOL	UsingLegacyInterpMethod() const { return FALSE; }


	/** 'Upgrades' this curve to use the latest tangent/interp algorithms (usually, will 'bake' key tangents.) */
	virtual void	UpgradeInterpMethod() {}
};


/**
 * Calculates the distance of a given Point in world space to a given line,
 * defined by the vector couple (Origin, Direction).
 *
 * @param	Point				point to check distance to Axis
 * @param	Direction			unit vector indicating the direction to check against
 * @param	Origin				point of reference used to calculate distance
 * @param	out_ClosestPoint	optional point that represents the closest point projected onto Axis
 *
 * @return	distance of Point from line defined by (Origin, Direction)
 */
FLOAT PointDistToLine
( 
	const	FVector &Point, 
	const	FVector &Line, 
	const	FVector &Origin, 
			FVector &OutClosestPoint
);

FLOAT PointDistToLine
( 
	const	FVector &Point, 
	const	FVector &Line, 
	const	FVector &Origin
);


FLOAT PointDistToSegment(const FVector &Point, const FVector &StartPoint, const FVector &EndPoint, FVector &OutClosestPoint);

/** 
 * Find closest points between 2 segments.
 * @param	(A1, B1)	defines the first segment.
 * @param	(A2, B2)	defines the second segment.
 * @param	OutP1		Closest point on segment 1 to segment 2.
 * @param	OutP2		Closest point on segment 2 to segment 1.
 */
void SegmentDistToSegment(FVector A1, FVector B1, FVector A2, FVector B2, FVector& OutP1, FVector& OutP2);


/** 
 * Find closest point on a Sphere to a Line.
 * When line intersects		Sphere, then closest point to LineOrigin is returned.
 * @param SphereOrigin		Origin of Sphere
 * @param SphereRadius		Radius of Sphere
 * @param LineOrigin		Origin of line
 * @param LineDir			Direction of line. Needs to be normalized!!
 * @param OutClosestPoint	Closest point on sphere to given line.
 */
void SphereDistToLine(FVector SphereOrigin, FLOAT SphereRadius, FVector LineOrigin, FVector LineDir, FVector& OutClosestPoint);


/**
 * Calculates whether a Point is within a cone segment, and also what percentage within the cone (100% is along the center line, whereas 0% is along the edge)
 *
 * @param Point - The Point in question
 * @param ConeStartPoint - the beginning of the cone (with the smallest radius)
 * @param ConeLine - the line out from the start point that ends at the largest radius point of the cone
 * @param radiusAtStart - the radius at the ConeStartPoint (0 for a 'proper' cone)
 * @param radiusAtEnd - the largest radius of the cone
 * @param percentageOut - output variable the holds how much within the cone the point is (1 = on center line, 0 = on exact edge or outside cone).
 *
 * @return true if the point is within the cone, false otherwise.
 */
UBOOL GetDistanceWithinConeSegment(FVector Point, FVector ConeStartPoint, FVector ConeLine, FLOAT RadiusAtStart, FLOAT RadiusAtEnd, FLOAT &PercentageOut);
/*-----------------------------------------------------------------------------
	Angular distance functions
-----------------------------------------------------------------------------*/

UBOOL GetDotDistance
( 
	FVector2D	&OutDotDist, 
	FVector		&Direction, 
	FVector		&AxisX, 
	FVector		&AxisY, 
	FVector		&AxisZ 
);

UBOOL GetAngularDistance
(
	FVector2D	&OutAngularDist, 
	FVector		&Direction, 
	FVector		&AxisX, 
	FVector		&AxisY, 
	FVector		&AxisZ 	
);

void GetAngularFromDotDist( FVector2D &OutAngDist, FVector2D &DotDist );

/*-----------------------------------------------------------------------------
	Intervals
-----------------------------------------------------------------------------*/

/** An interval of floating-point numbers. */
struct FInterval
{
	/** Construct an empty interval. */
	FInterval() : Min(0.0f), Max(0.0f), bIsEmpty(TRUE) { }

    /** Construct an interval with lower bound InMin and upper bound InMax.
	 *
	 * @param InMin		The lower bound of the constructed interval.
	 * @param InMax		The upper bound of the constructed interval.
	 */
	FInterval(FLOAT InMin, FLOAT InMax) : Min(InMin), Max(InMax), bIsEmpty(FALSE) { }

	/** Offset the interval by adding X. */
	void operator+=(FLOAT X)
	{
		if (!bIsEmpty)
		{
			Min += X;
			Max += X;
		}
	}

	/** Offset the interval by subtracting X. */
	void operator-=(FLOAT X)
	{
		if (!bIsEmpty)
		{
			Min -= X;
			Max -= X;
		}
	}

	/** Expand the interval to both sides by ExpandAmount. */
	void Expand(FLOAT ExpandAmount)
	{
		if (!bIsEmpty)
		{
			Min -= ExpandAmount;
			Max += ExpandAmount;
		}
	}

	/** Expand the interval if necessary to include X. */
	void Include(FLOAT X)
	{
		if (bIsEmpty)
		{
			Min = X;
			Max = X;
			bIsEmpty = FALSE;
		}
		else
		{
			if (X < Min)
			{
				Min = X;
			}
			if (X > Max)
			{
				Max = X;
			}
		}
	}

	/** The lower bound of the interval. */
	FLOAT Min;
	/** The upper bound of the interval. */
	FLOAT Max;
	/** Is the interval empty? */
	UBOOL bIsEmpty;
};

/** An arbitrarily oriented box (i.e. not necessarily axis-aligned). */
struct FOrientedBox
{
	/** Constructs a unit-sized, origin-centered box with axes aligned to the coordinate system. */
	FOrientedBox()
	: Center(0.0f), AxisX(1.0f, 0.0f, 0.0f), AxisY(0.0f, 1.0f, 0.0f), AxisZ(0.0f, 0.0f, 1.0f),
	  ExtentX(1.0f), ExtentY(1.0f), ExtentZ(1.0f) { }	

	/** Fills in the Verts array with the eight vertices of the box.
	 *
	 * @param Verts		The array to fill in with the vertices.
	 */
	void CalcVertices(FVector* Verts) const
	{
		static const FLOAT Signs[] = {-1.0f, 1.0f};
		for (INT i = 0; i < 2; i++)
		{
			for (INT j = 0; j < 2; j++)
			{
				for (INT k = 0; k < 2; k++)
				{
					*Verts++ = Center + Signs[i] * AxisX * ExtentX
									  + Signs[j] * AxisY * ExtentY
									  + Signs[k] * AxisZ * ExtentZ;
				}
			}
		}
	}

	/** Finds the projection interval of the box when projected onto Axis.
	 *
	 * @param Axis	The unit vector defining the axis to project the box onto.
	 */
	FInterval Project(const FVector& Axis) const
	{
		static const FLOAT Signs[] = {-1.0f, 1.0f};

		// Calculate the projections of the box center and the extent-scaled axes.
		FLOAT ProjectedCenter = Axis | Center;
		FLOAT ProjectedAxisX = Axis | (ExtentX * AxisX);
		FLOAT ProjectedAxisY = Axis | (ExtentY * AxisY);
		FLOAT ProjectedAxisZ = Axis | (ExtentZ * AxisZ);

		FInterval ProjectionInterval;
		for (INT i = 0; i < 2; i++)
		{
			for (INT j = 0; j < 2; j++)
			{
				for (INT k = 0; k < 2; k++)
				{
					// Project the box vertex onto the axis.
					FLOAT ProjectedVertex = ProjectedCenter + Signs[i] * ProjectedAxisX
															+ Signs[j] * ProjectedAxisY
															+ Signs[k] * ProjectedAxisZ;
					// If necessary, expand the projection interval to include the box vertex projection.
					ProjectionInterval.Include(ProjectedVertex);
				}
			}
		}
		return ProjectionInterval;
	}

	/** The center of the box. */
	FVector Center;

	/** The x-axis vector of the box. Must be a unit vector. */
	FVector AxisX;
	/** The y-axis vector of the box. Must be a unit vector. */
	FVector AxisY;
	/** The z-axis vector of the box. Must be a unit vector. */
	FVector AxisZ;

	/** The extent of the box along its x-axis. */
	FLOAT ExtentX;
	/** The extent of the box along its y-axis. */
	FLOAT ExtentY;
	/** The extent of the box along its z-axis. */
	FLOAT ExtentZ;
};

/** a bounding cylinder */
struct FCylinder
{
	FLOAT Radius, Height;

	/** empty default constructor */
	FORCEINLINE FCylinder()
	{}

	/** initializing constructor */
	FORCEINLINE FCylinder(FLOAT InRadius, FLOAT InHeight)
		: Radius(InRadius), Height(InHeight)
	{}
	explicit FORCEINLINE FCylinder(EEventParm)
		: Radius(0.0f), Height(0.0f)
	{}

	/** returns the extent for the axis aligned box that most closely represents this cylinder */
	FORCEINLINE FVector GetExtent()
	{
		return FVector(Radius, Radius, Height);
	}
};

/*-----------------------------------------------------------------------------
	Interpolation functions
-----------------------------------------------------------------------------*/

FVector VInterpTo( const FVector& Current, const FVector& Target, FLOAT& DeltaTime, FLOAT InterpSpeed );

FRotator RInterpTo( const FRotator& Current, const FRotator& Target, FLOAT& DeltaTime, FLOAT InterpSpeed );

FLOAT FInterpTo( FLOAT Current, FLOAT Target, FLOAT DeltaTime, FLOAT InterpSpeed );

FLOAT FInterpEaseInOut( FLOAT A, FLOAT B, FLOAT Alpha, FLOAT Exp );

/*-----------------------------------------------------------------------------
	FDistribution functions
-----------------------------------------------------------------------------*/

typedef FLOAT LOOKUPVALUE;

enum ERawDistributionType
{
	RDT_Float_Constant,

	DT_Vector_Constant,
};

enum ERawDistributionOperation
{
	RDO_Uninitialized,
	RDO_None,
	RDO_Random,
	RDO_Extreme,
};

enum ERawDistributionLockFlags
{
	RDL_None,
	RDL_XY,
	RDL_XZ,
	RDL_YZ,
	RDL_XYZ,
};

#define DIST_GET_LOCKFLAG_0(Type)				(Type & 0x07)
#define DIST_GET_LOCKFLAG_1(Type)				(Type & 0x48)
#define DIST_GET_LOCKFLAG(InIndex, Type)		((InIndex == 0) ? DIST_GET_LOCKFLAG_0(Type) : DIST_GET_LOCKFLAG_1(Type))
#define DIST_SET_LOCKFLAG_0(Flag, Type)			(Type |= (Flag & 0x07))
#define DIST_SET_LOCKFLAG_1(Flag, Type)			(Type |= ((Flag & 0x07) << 3))
#define DIST_SET_LOCKFLAG(InIndex, Flag, Type)	((InIndex == 0) ? DIST_SET_LOCKFLAG_0(Flag, Type) : DIST_SET_LOCKFLAG_1(Flag, Type))

#define DIST_UNIFORMCURVE_FLAG					0x80
#define DIST_IS_UNIFORMCURVE(Type)				(Type & DIST_UNIFORMCURVE_FLAG)
#define DIST_SET_UNIFORMCURVE(Flag, Type)		((Flag != 0) ? (Type |= DIST_UNIFORMCURVE_FLAG) : (Type &= ~DIST_UNIFORMCURVE_FLAG))

struct FRawDistribution
{
	/**
	 * Calcuate the float or vector value at the given time 
	 * @param Time The time to evaluate
	 * @param Value An array of (1 or 3) FLOATs to receive the values
	 * @param NumCoords The number of floats in the Value array
	 * @param Extreme For distributions that use one of the extremes, this is which extreme to use
	 */
	void GetValue(FLOAT Time, FLOAT* Value, INT NumCoords, INT Extreme=0);

	// prebaked versions of these
	void GetValue1(FLOAT Time, FLOAT* Value, INT Extreme=0);
	void GetValue3(FLOAT Time, FLOAT* Value, INT Extreme=0);
	inline void GetValue1None(FLOAT Time, FLOAT* InValue) const
	{
		FLOAT* Value = InValue;
		const LOOKUPVALUE* Entry1;
		const LOOKUPVALUE* Entry2;
		FLOAT LerpAlpha = 0.0f;
		GetEntry(Time, Entry1, Entry2, LerpAlpha);
		const LOOKUPVALUE* NewEntry1 = Entry1;
		const LOOKUPVALUE* NewEntry2 = Entry2;
		Value[0] = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
	}
	inline void GetValue3None(FLOAT Time, FLOAT* InValue) const
	{
		FLOAT* Value = InValue;
		const LOOKUPVALUE* Entry1;
		const LOOKUPVALUE* Entry2;
		FLOAT LerpAlpha = 0.0f;
		GetEntry(Time, Entry1, Entry2, LerpAlpha);
		const LOOKUPVALUE* NewEntry1 = Entry1;
		const LOOKUPVALUE* NewEntry2 = Entry2;
		FLOAT T0 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
		FLOAT T1 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
		FLOAT T2 = Lerp(NewEntry1[2], NewEntry2[2], LerpAlpha);
		Value[0] = T0;
		Value[1] = T1;
		Value[2] = T2;
	}
	void GetValue1Extreme(FLOAT Time, FLOAT* Value, INT Extreme=0);
	void GetValue3Extreme(FLOAT Time, FLOAT* Value, INT Extreme=0);
	void GetValue1Random(FLOAT Time, FLOAT* Value);
	void GetValue3Random(FLOAT Time, FLOAT* Value);

	FORCEINLINE UBOOL IsSimple() 
	{
		return Op == RDO_None;
	}

	/**
	 * Return the UDistribution* variable if the given StructProperty
	 * points to a FRawDistribution* struct
	 * @param Property Some UStructProperyy
	 * @param Data Memory that owns the property
	 * @return The UDisitribution* object if this is a FRawDistribution* struct, 
	 *         or NULL otherwise
	 */
	static UObject* TryGetDistributionObjectFromRawDistributionProperty(UStructProperty* Property, BYTE* Data);

protected:
	// let our masters access us
	friend class UDistributionFloat;
	friend class UDistributionVector;

	/**
	 * Get the entry for Time, and possibly the one after it for interpolating (along with 
	 * an alpha for interpolation)
	 * 
	 * @param Time The time we are looking to retrieve
	 * @param Entry1 Out variable that is the first (or only) entry
	 * @param Entry2 Out variable that is the second entry (for interpolating) or NULL or Entry1 is all that's needed
	 * @param LerpAlpha Out variable that is the alpha for interpolating between Entry1 and Entry2 (when Entry2 is valid)
	 */
	inline void GetEntry(FLOAT Time, const LOOKUPVALUE*& Entry1, const LOOKUPVALUE*& Entry2, FLOAT& LerpAlpha) const
	{
		// make time relative to start time
		Time -= LookupTableStartTime;
		Time *= LookupTableTimeScale;
		Time = Max(Time,0.0f); // looks branch-free

		// get the entry before or at the given time
		DWORD Index = appTrunc(Time);
		DWORD Limit = LookupTable.Num() - LookupTableChunkSize;

		DWORD EntryIndex1 = Index * LookupTableChunkSize + 2;
		DWORD EntryIndex2 = EntryIndex1 + LookupTableChunkSize;
		EntryIndex1 = Min<DWORD>(EntryIndex1, Limit); 
		EntryIndex2 = Min<DWORD>(EntryIndex2, Limit); 

		Entry1 = &LookupTable(EntryIndex1);
		Entry2 = &LookupTable(EntryIndex2);
		// calculate the alpha to lerp between entry1 and entry2 
		LerpAlpha = Time - FLOAT(Index);
	}

	/** Type of distribution, NOT NEEDED */
	BYTE Type;

	/** How to process the data in the lookup table */
	BYTE Op;

	/** How many elements per entry (ie, RDO_Random needs two elements to rand between) IMPLIED BY OP!!!*/
	BYTE LookupTableNumElements;

	/** The size of one element (1 for floats, 3 for vectors, etc) multiplied by number of elements */
	BYTE LookupTableChunkSize;

	/** Lookup table of values */
	TArrayNoInit<LOOKUPVALUE> LookupTable;

	/** Time between values in the lookup table */
	FLOAT LookupTableTimeScale;

	/** Absolute time of the first value */
	FLOAT LookupTableStartTime;
};


/*-----------------------------------------------------------------------------
	Container traits for base types - they have empty ctor/dtors.
-----------------------------------------------------------------------------*/

template <> struct TIsPODType<FVector> { enum { Value = true }; };
template <> struct TIsPODType<FPlane> { enum { Value = true }; };
template <> struct TIsPODType<FVector2D> { enum { Value = true }; };
template <> struct TIsPODType<FRotator> { enum { Value = true }; };
template <> struct TIsPODType<FQuat> { enum { Value = true }; };
template <> struct TIsPODType<FBox> { enum { Value = true }; };
template <> struct TIsPODType<FMatrix> { enum { Value = true }; };

/**
 * Computes the base 2 logarithm of the specified value
 *
 * @param Value the value to perform the log on
 *
 * @return the base 2 log of the value
 */
inline FLOAT appLog2(FLOAT Value)
{
	// Cached value for fast conversions
	static const FLOAT LogToLog2 = 1.f / appLoge(2.f);
	// Do the platform specific log and convert using the cached value
	return appLoge(Value) * LogToLog2;
}

/*-----------------------------------------------------------------------------
	Inline functions for FVector
-----------------------------------------------------------------------------*/
FORCEINLINE FVector2D::FVector2D( const FVector& V )
: X(V.X), Y(V.Y)
{
}

FORCEINLINE FVector::FVector( const FVector2D V, FLOAT InZ )
: X(V.X), Y(V.Y), Z(InZ)
{
}

FORCEINLINE FVector::FVector( const FVector4& V )
:	X(V.X), Y(V.Y), Z(V.Z)
{}

/*-----------------------------------------------------------------------------
FFloat32
-----------------------------------------------------------------------------*/

/**
* 32 bit float components
*/
class FFloat32
{
public:
	union
	{
		struct
		{
#if __INTEL_BYTE_ORDER__
			DWORD	Mantissa : 23;
			DWORD	Exponent : 8;
			DWORD	Sign : 1;			
#else
			DWORD	Sign : 1;
			DWORD	Exponent : 8;
			DWORD	Mantissa : 23;			
#endif
		} Components;

		FLOAT	FloatValue;
	};

	FFloat32( FLOAT InValue=0.0f )
		:	FloatValue(InValue)
	{}
};

/*-----------------------------------------------------------------------------
FFloat16
-----------------------------------------------------------------------------*/

/**
* 16 bit float components and conversion
*
*
* IEEE FLOAT 16
* Represented by 10-bit mantissa M, 5-bit exponent E, and 1-bit sign S
*
* Specials:
* 
* E=0, M=0			== 0.0
* E=0, M!=0			== Denormalized value (M / 2^10) * 2^-14
* 0<E<31, M=any		== (1 + M / 2^10) * 2^(E-15)
* E=31, M=0			== Infinity
* E=31, M!=0		== NAN
*
*/
class FFloat16
{
public:
	union
	{
		struct
		{
#if __INTEL_BYTE_ORDER__
			WORD	Mantissa : 10;
			WORD	Exponent : 5;
			WORD	Sign : 1;
#else
			WORD	Sign : 1;
			WORD	Exponent : 5;
			WORD	Mantissa : 10;			
#endif
		} Components;

		WORD	Encoded;
	};

	/** Default constructor */
	FFloat16( )
	:	Encoded(0)
	{
	}

	/** Copy constructor. */
	FFloat16( const FFloat16& FP16Value )
	{
		Encoded = FP16Value.Encoded;
	}

	/** Conversion constructor. Convert from Fp32 to Fp16. */
	FFloat16( FLOAT FP32Value )
	{
		Set( FP32Value );
	}	

	/** Assignment operator. Convert from Fp32 to Fp16. */
	FFloat16& operator=( FLOAT FP32Value )
	{
		Set( FP32Value );
		return *this;
	}

	/** Assignment operator. Copy Fp16 value. */
	FFloat16& operator=( const FFloat16& FP16Value )
	{
		Encoded = FP16Value.Encoded;
		return *this;
	}

	/** Convert from Fp16 to Fp32. */
	operator FLOAT() const
	{
		return GetFloat();
	}

	/** Convert from Fp32 to Fp16. */
	void Set( FLOAT FP32Value )
	{
		FFloat32 FP32(FP32Value);

		// Copy sign-bit
		Components.Sign = FP32.Components.Sign;

		// Check for zero, denormal or too small value.
		if ( FP32.Components.Exponent <= 112 )			// Too small exponent? (0+127-15)
		{
			// Set to 0.
			Components.Exponent = 0;
			Components.Mantissa = 0;
		}
		// Check for INF or NaN, or too high value
		else if ( FP32.Components.Exponent >= 143 )		// Too large exponent? (31+127-15)
		{
			// Set to 65504.0 (max value)
			Components.Exponent = 30;
			Components.Mantissa = 1023;
		}
		// Handle normal number.
		else
		{
			Components.Exponent = INT(FP32.Components.Exponent) - 127 + 15;
			Components.Mantissa = WORD(FP32.Components.Mantissa >> 13);
		}
	}

	/** Convert from Fp16 to Fp32. */
	FLOAT GetFloat() const
	{
		FFloat32	Result;

		Result.Components.Sign = Components.Sign;
		if (Components.Exponent == 0)
		{
			// Zero or denormal. Just clamp to zero...
			Result.Components.Exponent = 0;
			Result.Components.Mantissa = 0;
		}
		else if (Components.Exponent == 31)		// 2^5 - 1
		{
			// Infinity or NaN. Set to 65504.0
			Result.Components.Exponent = 142;
			Result.Components.Mantissa = 8380416;
		}
		else
		{
			// Normal number.
			Result.Components.Exponent = INT(Components.Exponent) - 15 + 127; // Stored exponents are biased by half their range.
			Result.Components.Mantissa = DWORD(Components.Mantissa) << 13;
		}

		return Result.FloatValue;
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FFloat16& V )
	{
		return Ar << V.Encoded;
	}
};

/*-----------------------------------------------------------------------------
FVector2DHalf
-----------------------------------------------------------------------------*/

/**
* A 2x1 of 16 bit floats.
*/
struct FVector2DHalf 
{
	FFloat16	X,
				Y;

	// Constructors.
	FORCEINLINE FVector2DHalf()
	{}
	FORCEINLINE FVector2DHalf(WORD InX,FLOAT InY)
		:	X(InX), Y(InY)
	{}
	FORCEINLINE FVector2DHalf(FLOAT InX,FLOAT InY)
		:	X(InX), Y(InY)
	{}
	FORCEINLINE FVector2DHalf(const FVector2D& Vector2D)
		:	X(Vector2D.X), Y(Vector2D.Y)
	{}
	FString ToString() const
	{
		return FString::Printf(TEXT("X=%3.3f Y=%3.3f"), (FLOAT)X, (FLOAT)Y );
	}
	operator FVector2D() const
	{
		return FVector2D((FLOAT)X,(FLOAT)Y);
	}
	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FVector2DHalf& V )
	{
		return Ar << V.X << V.Y;
	}
};

