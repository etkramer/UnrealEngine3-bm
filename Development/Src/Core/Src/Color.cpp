/*=============================================================================
	Color.cpp: Unreal color implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

// Common colors.
const FLinearColor FLinearColor::White(1,1,1);
const FLinearColor FLinearColor::Black(0,0,0);

// Vector constants used by color math.
#if ENABLE_VECTORINTRINSICS && XBOX
	static const VectorRegister ColorPowExponent = { 2.2f, 2.2f, 2.2f, 1.0f };
#endif

//	FColor->FLinearColor conversion.
FLinearColor::FLinearColor(const FColor& C)
#if !(ENABLE_VECTORINTRINSICS && XBOX)
	:
	R(appPow(C.R / 255.0f,2.2f)),
	G(appPow(C.G / 255.0f,2.2f)),
	B(appPow(C.B / 255.0f,2.2f)),
	A(C.A / 255.0f)
#endif
{
#if ENABLE_VECTORINTRINSICS && XBOX
#if XBOX
	VectorRegister Temp = VectorLoadByte4Reverse( &C );
#else
	VectorRegister Temp = VectorLoadByte4( &C );
#endif
	VectorResetFloatRegisters();
	Temp = VectorPow( VectorMultiply( Temp, VECTOR_INV_255 ), ColorPowExponent );
	VectorStore( VectorSwizzle(Temp,2,1,0,3), this );
#endif
}

// Convert from float to RGBE as outlined in Gregory Ward's Real Pixels article, Graphics Gems II, page 80.
FColor FLinearColor::ToRGBE() const
{
	const FLOAT	Primary = Max3( R, G, B );
	FColor	Color;

	if( Primary < 1E-32 )
	{
		Color = FColor(0,0,0,0);
	}
	else
	{
		INT	Exponent;
		const FLOAT Scale	= frexp(Primary, &Exponent) / Primary * 255.f;

		Color.R		= Clamp(appTrunc(R * Scale), 0, 255);
		Color.G		= Clamp(appTrunc(G * Scale), 0, 255);
		Color.B		= Clamp(appTrunc(B * Scale), 0, 255);
		Color.A		= Clamp(appTrunc(Exponent),-128,127) + 128;
	}

	return Color;
}

FColor FLinearColor::Quantize() const
{
	return FColor(
		(BYTE)Clamp<INT>(appTrunc(R*255.f),0,255),
		(BYTE)Clamp<INT>(appTrunc(G*255.f),0,255),
		(BYTE)Clamp<INT>(appTrunc(B*255.f),0,255),
		(BYTE)Clamp<INT>(appTrunc(A*255.f),0,255)
		);
}

/**
 * Returns a desaturated color, with 0 meaning no desaturation and 1 == full desaturation
 *
 * @param	Desaturation	Desaturation factor in range [0..1]
 * @return	Desaturated color
 */
FLinearColor FLinearColor::Desaturate( FLOAT Desaturation ) const
{
	FLOAT Luminance = R * 0.3 + G * 0.59 + B * 0.11;
	return Lerp( *this, FLinearColor( Luminance, Luminance, Luminance, 0 ), Desaturation );
}

// Convert from RGBE to float as outlined in Gregory Ward's Real Pixels article, Graphics Gems II, page 80.
FLinearColor FColor::FromRGBE() const
{
	if( A == 0 )
		return FLinearColor::Black;
	else
	{
		const FLOAT Scale = ldexp( 1 / 255.0, A - 128 );
		return FLinearColor( R * Scale, G * Scale, B * Scale, 1.0f );
	}
}

/**
 * Converts byte hue-saturation-brightness to floating point red-green-blue.
 */
FLinearColor FLinearColor::FGetHSV( BYTE H, BYTE S, BYTE V )
{
	FLOAT Brightness = V * 1.4f / 255.f;
	Brightness *= 0.7f/(0.01f + appSqrt(Brightness));
	Brightness  = Clamp(Brightness,0.f,1.f);
	const FVector Hue = (H<86) ? FVector((85-H)/85.f,(H-0)/85.f,0) : (H<171) ? FVector(0,(170-H)/85.f,(H-85)/85.f) : FVector((H-170)/85.f,0,(255-H)/84.f);
	const FVector ColorVector = (Hue + S/255.f * (FVector(1,1,1) - Hue)) * Brightness;
	return FLinearColor(ColorVector.X,ColorVector.Y,ColorVector.Z,1);
}

/**
 * Makes a random but quite nice color.
 */
FColor FColor::MakeRandomColor()
{
	const BYTE Hue = (BYTE)( appFrand()*255.f );
	return FColor( FLinearColor::FGetHSV(Hue, 0, 255) );
}

void ComputeAndFixedColorAndIntensity(const FLinearColor& InLinearColor,FColor& OutColor,FLOAT& OutIntensity)
{
	FLOAT MaxComponent = Max(DELTA,Max(InLinearColor.R,Max(InLinearColor.G,InLinearColor.B)));
	OutColor = InLinearColor / MaxComponent;
	OutIntensity = MaxComponent;
}
