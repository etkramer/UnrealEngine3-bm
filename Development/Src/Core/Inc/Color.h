/*=============================================================================
	Color.h: Unreal color definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * A linear, 32-bit/component floating point RGBA color.
 */
struct FLinearColor
{
	FLOAT	R,
			G,
			B,
			A;

	FLinearColor() {}
	FLinearColor(FLOAT InR,FLOAT InG,FLOAT InB,FLOAT InA = 1.0f): R(InR), G(InG), B(InB), A(InA) {}
	FLinearColor(const FColor& C);

	// Serializer.

	friend FArchive& operator<<(FArchive& Ar,FLinearColor& Color)
	{
		return Ar << Color.R << Color.G << Color.B << Color.A;
	}

	// Conversions.
	FColor ToRGBE() const;

	// Operators.

	FLOAT& Component(INT Index)
	{
		return (&R)[Index];
	}

	const FLOAT& Component(INT Index) const
	{
		return (&R)[Index];
	}

	FLinearColor operator+(const FLinearColor& ColorB) const
	{
		return FLinearColor(
			this->R + ColorB.R,
			this->G + ColorB.G,
			this->B + ColorB.B,
			this->A + ColorB.A
			);
	}
	FLinearColor& operator+=(const FLinearColor& ColorB)
	{
		R += ColorB.R;
		G += ColorB.G;
		B += ColorB.B;
		A += ColorB.A;
		return *this;
	}

	FLinearColor operator-(const FLinearColor& ColorB) const
	{
		return FLinearColor(
			this->R - ColorB.R,
			this->G - ColorB.G,
			this->B - ColorB.B,
			this->A - ColorB.A
			);
	}
	FLinearColor& operator-=(const FLinearColor& ColorB)
	{
		R -= ColorB.R;
		G -= ColorB.G;
		B -= ColorB.B;
		A -= ColorB.A;
		return *this;
	}

	FLinearColor operator*(const FLinearColor& ColorB) const
	{
		return FLinearColor(
			this->R * ColorB.R,
			this->G * ColorB.G,
			this->B * ColorB.B,
			this->A * ColorB.A
			);
	}
	FLinearColor& operator*=(const FLinearColor& ColorB)
	{
		R *= ColorB.R;
		G *= ColorB.G;
		B *= ColorB.B;
		A *= ColorB.A;
		return *this;
	}

	FLinearColor operator*(FLOAT Scalar) const
	{
		return FLinearColor(
			this->R * Scalar,
			this->G * Scalar,
			this->B * Scalar,
			this->A * Scalar
			);
	}

	FLinearColor& operator*=(FLOAT Scalar)
	{
		R *= Scalar;
		G *= Scalar;
		B *= Scalar;
		A *= Scalar;
		return *this;
	}

	FLinearColor operator/(const FLinearColor& ColorB) const
	{
		return FLinearColor(
			this->R / ColorB.R,
			this->G / ColorB.G,
			this->B / ColorB.B,
			this->A / ColorB.A
			);
	}
	FLinearColor& operator/=(const FLinearColor& ColorB)
	{
		R /= ColorB.R;
		G /= ColorB.G;
		B /= ColorB.B;
		A /= ColorB.A;
		return *this;
	}

	FLinearColor operator/(FLOAT Scalar) const
	{
		const FLOAT	InvScalar = 1.0f / Scalar;
		return FLinearColor(
			this->R * InvScalar,
			this->G * InvScalar,
			this->B * InvScalar,
			this->A * InvScalar
			);
	}
	FLinearColor& operator/=(FLOAT Scalar)
	{
		const FLOAT	InvScalar = 1.0f / Scalar;
		R *= InvScalar;
		G *= InvScalar;
		B *= InvScalar;
		A *= InvScalar;
		return *this;
	}

	/** Comaprison operators */
	UBOOL operator==(const FLinearColor& ColorB) const
	{
		return this->R == ColorB.R && this->G == ColorB.G && this->B == ColorB.B && this->A == ColorB.A;
	}
	UBOOL operator!=(const FLinearColor& Other) const
	{
		return this->R != Other.R || this->G != Other.G || this->B != Other.B || this->A != Other.A;
	}

	// Error-tolerant comparison.
	UBOOL Equals(const FLinearColor& ColorB, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs(this->R - ColorB.R) < Tolerance && Abs(this->G - ColorB.G) < Tolerance && Abs(this->B - ColorB.B) < Tolerance && Abs(this->A - ColorB.A) < Tolerance;
	}

	/**
	 * Converts byte hue-saturation-brightness to floating point red-green-blue.
	 */
	static FLinearColor FGetHSV(BYTE H,BYTE S,BYTE V);

	/** Quantizes the linear color and returns the result as a FColor.  This bypasses the SRGB conversion. */
	FColor Quantize() const;

	/**
	 * Returns a desaturated color, with 0 meaning no desaturation and 1 == full desaturation
	 *
	 * @param	Desaturation	Desaturation factor in range [0..1]
	 * @return	Desaturated color
	 */
	FLinearColor Desaturate( FLOAT Desaturation ) const;

	// Common colors.	
	static const FLinearColor White;
	static const FLinearColor Black;
};

inline FLinearColor operator*(FLOAT Scalar,const FLinearColor& Color)
{
	return Color.operator*( Scalar );
}

//
//	FColor
//

class FColor
{
public:
	// Variables.
#if __INTEL_BYTE_ORDER__
    #if _MSC_VER
		// Win32 x86
	    union { struct{ BYTE B,G,R,A; }; DWORD AlignmentDummy; };
    #else
		// Linux x86, etc
	    BYTE B GCC_ALIGN(4);
	    BYTE G,R,A;
    #endif
#else // __INTEL_BYTE_ORDER__
	union { struct{ BYTE A,R,G,B; }; DWORD AlignmentDummy; };
#endif

	DWORD& DWColor(void) {return *((DWORD*)this);}
	const DWORD& DWColor(void) const {return *((DWORD*)this);}

	// Constructors.
	FColor() {}
	FColor( BYTE InR, BYTE InG, BYTE InB, BYTE InA = 255 )
		:	A(InA), R(InR), G(InG), B(InB) {}
	
	FColor(const FLinearColor& C)
		:	A(Clamp(appTrunc(       C.A              * 255.0f),0,255))
		,	R(Clamp(appTrunc(appPow(C.R,1.0f / 2.2f) * 255.0f),0,255))
		,	G(Clamp(appTrunc(appPow(C.G,1.0f / 2.2f) * 255.0f),0,255))
		,	B(Clamp(appTrunc(appPow(C.B,1.0f / 2.2f) * 255.0f),0,255))
			
	{}

	explicit FColor( DWORD InColor )
	{ DWColor() = InColor; }

	// Serializer.
	friend FArchive& operator<< (FArchive &Ar, FColor &Color )
	{
		return Ar << Color.DWColor();
	}

	// Operators.
	UBOOL operator==( const FColor &C ) const
	{
		return DWColor() == C.DWColor();
	}
	UBOOL operator!=( const FColor& C ) const
	{
		return DWColor() != C.DWColor();
	}
	void operator+=(const FColor& C)
	{
		R = (BYTE) Min((INT) R + (INT) C.R,255);
		G = (BYTE) Min((INT) G + (INT) C.G,255);
		B = (BYTE) Min((INT) B + (INT) C.B,255);
		A = (BYTE) Min((INT) A + (INT) C.A,255);
	}
	FLinearColor FromRGBE() const;

	/**
	 * Makes a random but quite nice color.
	 */
	static FColor MakeRandomColor();

	/** Reinterprets the color as a linear color. */
	FLinearColor ReinterpretAsLinear() const
	{
		return FLinearColor(R/255.f,G/255.f,B/255.f,A/255.f);
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("(R=%i,G=%i,B=%i,A=%i)"),R,G,B,A);
	}

	/**
	 * Initialize this Color based on an FString. The String is expected to contain R=, G=, B=, A=.
	 * The FColor will be bogus when InitFromString returns FALSE.
1	 *
	 * @param	InSourceString	FString containing the color values.
	 *
	 * @return TRUE if the four values were read successfully; FALSE otherwise.
	 */
	UBOOL InitFromString( const FString & InSourceString )
	{
		R = G = B = 0;
		A = 255;

		if (!Parse(*InSourceString,TEXT("R="),R) ||
			!Parse(*InSourceString,TEXT("G="),G) ||
			!Parse(*InSourceString,TEXT("B="),B))
		{
			// Alpha is optional, so if we don't find it just assume it's 255 (default)
			Parse(*InSourceString,TEXT("A="),A);

			return FALSE;
		}

		return TRUE;
	}
};

inline DWORD GetTypeHash( const FColor& Color )
{
	return Color.DWColor();
}

/** Computes a brightness and a fixed point color from a floating point color. */
extern void ComputeAndFixedColorAndIntensity(const FLinearColor& InLinearColor,FColor& OutColor,FLOAT& OutIntensity);

// These act like a POD
template <> struct TIsPODType<FColor> { enum { Value = true }; };
template <> struct TIsPODType<FLinearColor> { enum { Value = true }; };
