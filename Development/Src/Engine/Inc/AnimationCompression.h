/*=============================================================================
	AnimationCompression.h: Skeletal mesh animation compression.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#ifndef __ANIMATIONCOMPRESSION_H__
#define __ANIMATIONCOMPRESSION_H__

#include "FloatPacker.h"

#define Quant16BitDiv     (32767.f)
#define Quant16BitFactor  (32767.f)
#define Quant16BitOffs    (32767)

#define Quant10BitDiv     (511.f)
#define Quant10BitFactor  (511.f)
#define Quant10BitOffs    (511)

#define Quant11BitDiv     (1023.f)
#define Quant11BitFactor  (1023.f)
#define Quant11BitOffs    (1023)

#if BATMAN
// https://github.com/gildor2/UEViewer/blob/a0bfb468d42be831b126632fd8a0ae6b3614f981/Unreal/UnrealMesh/UnMeshTypes.h#L414
class FQuatFixed48Max
{
public:
	WORD Data[3]; // layout: V2[15] : V1[15] : V0[15] : S[2]

	FQuatFixed48Max()
	{}

	explicit FQuatFixed48Max(const FQuat& Quat)
	{
		FromQuat( Quat );
	}

	void FromQuat(const FQuat& Quat)
	{
		// Unimplemented...
	}

	void ToQuat(FQuat& Out) const
	{
		unsigned tmp;
		tmp = (Data[1] << 16) | Data[0];
		int S = tmp & 3;								// bits [0..1]
		int L = (tmp >> 2) & 0x7FFF;					// bits [2..16]
		tmp = (Data[2] << 16) | Data[1];
		int M = (tmp >> 1) & 0x7FFF;					// bits [17..31]
		int H = (tmp >> 16) & 0x7FFF;					// bits [32..46]
		// bit 47 is unused ...

		static const float shift = 0.70710678118f;		// sqrt(0.5)
		static const float scale = 1.41421356237f;		// sqrt(0.5)*2
		float l = (L - 0.5f) / 32767 * scale - shift;
		float m = (M - 0.5f) / 32767 * scale - shift;
		float h = (H - 0.5f) / 32767 * scale - shift;
		float a = sqrt(1.0f - (l * l + m * m + h * h));
		// "l", "m", "h" are serialized values, in a range -shift .. +shift
		// if we will remove one of maximal values ("a"), other values will be smaller
		// that "a"; if "a" is 1, all other values are 0; when "a" becomes smaller,
		// other values may grow; maximal value of "other" equals to "a" when
		// 2 quaternion components equals to "a" and 2 other is 0; so, maximal
		// stored value can be computed from equation "a*a + a*a + 0 + 0 = 1", so
		// maximal value is sqrt(1/2) ...
		// "a" is computed value, up to 1

		switch (S)			// choose where to place "a"
		{
		case 0:
			Out.X = a;
			Out.Y = l;
			Out.Z = m;
			Out.W = h;
			break;
		case 1:
			Out.X = l;
			Out.Y = a;
			Out.Z = m;
			Out.W = h;
			break;
		case 2:
			Out.X = l;
			Out.Y = m;
			Out.Z = a;
			Out.W = h;
			break;
		default: // 3
			Out.X = l;
			Out.Y = m;
			Out.Z = h;
			Out.W = a;
		}
	}

	friend FArchive& operator<<(FArchive& Ar, FQuatFixed48Max& Quat)
	{
		Ar << Quat.Data[0];
		Ar << Quat.Data[1];
		Ar << Quat.Data[2];
		return Ar;
	}
};
#endif

class FQuatFixed48NoW
{
public:
	WORD X;
	WORD Y;
	WORD Z;

	FQuatFixed48NoW()
	{}

	explicit FQuatFixed48NoW(const FQuat& Quat)
	{
		FromQuat( Quat );
	}

	void FromQuat(const FQuat& Quat)
	{
		FQuat Temp( Quat );
		if ( Temp.W < 0.f )
		{
			Temp.X = -Temp.X;
			Temp.Y = -Temp.Y;
			Temp.Z = -Temp.Z;
			Temp.W = -Temp.W;
		}
		Temp.Normalize();

		X = (INT)(Temp.X * Quant16BitFactor) + Quant16BitOffs;
		Y = (INT)(Temp.Y * Quant16BitFactor) + Quant16BitOffs;
		Z = (INT)(Temp.Z * Quant16BitFactor) + Quant16BitOffs;
	}

	void ToQuat(FQuat& Out) const
	{
		const FLOAT FX = ((INT)X - (INT)Quant16BitOffs) / Quant16BitDiv;
		const FLOAT FY = ((INT)Y - (INT)Quant16BitOffs) / Quant16BitDiv;
		const FLOAT FZ = ((INT)Z - (INT)Quant16BitOffs) / Quant16BitDiv;
		const FLOAT WSquared = 1.f - FX*FX - FY*FY - FZ*FZ;

		Out.X = FX;
		Out.Y = FY;
		Out.Z = FZ;
		Out.W = WSquared > 0.f ? appSqrt( WSquared ) : 0.f;
	}

	friend FArchive& operator<<(FArchive& Ar, FQuatFixed48NoW& Quat)
	{
		Ar << Quat.X;
		Ar << Quat.Y;
		Ar << Quat.Z;
		return Ar;
	}
};

class FQuatFixed32NoW
{
public:
	DWORD Packed;

	FQuatFixed32NoW()
	{}

	explicit FQuatFixed32NoW(const FQuat& Quat)
	{
		FromQuat( Quat );
	}

	void FromQuat(const FQuat& Quat)
	{
		FQuat Temp( Quat );
		if ( Temp.W < 0.f )
		{
			Temp.X = -Temp.X;
			Temp.Y = -Temp.Y;
			Temp.Z = -Temp.Z;
			Temp.W = -Temp.W;
		}
		Temp.Normalize();

		const DWORD PackedX = (INT)(Temp.X * Quant11BitFactor) + Quant11BitOffs;
		const DWORD PackedY = (INT)(Temp.Y * Quant11BitFactor) + Quant11BitOffs;
		const DWORD PackedZ = (INT)(Temp.Z * Quant10BitFactor) + Quant10BitOffs;

		// 21-31 X, 10-20 Y, 0-9 Z.
		const DWORD XShift = 21;
		const DWORD YShift = 10;
		Packed = (PackedX << XShift) | (PackedY << YShift) | (PackedZ);
	}

	void ToQuat(FQuat& Out) const
	{
		const DWORD XShift = 21;
		const DWORD YShift = 10;
		const DWORD ZMask = 0x000003ff;
		const DWORD YMask = 0x001ffc00;
		const DWORD XMask = 0xffe00000;

		const DWORD UnpackedX = Packed >> XShift;
		const DWORD UnpackedY = (Packed & YMask) >> YShift;
		const DWORD UnpackedZ = (Packed & ZMask);

		const FLOAT X = ((INT)UnpackedX - (INT)Quant11BitOffs) / Quant11BitDiv;
		const FLOAT Y = ((INT)UnpackedY - (INT)Quant11BitOffs) / Quant11BitDiv;
		const FLOAT Z = ((INT)UnpackedZ - (INT)Quant10BitOffs) / Quant10BitDiv;
		const FLOAT WSquared = 1.f - X*X - Y*Y - Z*Z;

		Out.X = X;
		Out.Y = Y;
		Out.Z = Z;
		Out.W = WSquared > 0.f ? appSqrt( WSquared ) : 0.f;
	}

	friend FArchive& operator<<(FArchive& Ar, FQuatFixed32NoW& Quat)
	{
		Ar << Quat.Packed;
		return Ar;
	}
};

class FQuatFloat96NoW
{
public:
	FLOAT X;
	FLOAT Y;
	FLOAT Z;

	FQuatFloat96NoW()
	{}

	explicit FQuatFloat96NoW(const FQuat& Quat)
	{
		FromQuat( Quat );
	}

	FQuatFloat96NoW(FLOAT InX, FLOAT InY, FLOAT InZ)
		:	X( InX )
		,	Y( InY )
		,	Z( InZ )
	{}

	void FromQuat(const FQuat& Quat)
	{
		FQuat Temp( Quat );
		if ( Temp.W < 0.f )
		{
			Temp.X = -Temp.X;
			Temp.Y = -Temp.Y;
			Temp.Z = -Temp.Z;
			Temp.W = -Temp.W;
		}
		Temp.Normalize();
		X = Temp.X;
		Y = Temp.Y;
		Z = Temp.Z;
	}

	void ToQuat(FQuat& Out) const
	{
		const FLOAT WSquared = 1.f - X*X - Y*Y - Z*Z;

		Out.X = X;
		Out.Y = Y;
		Out.Z = Z;
		Out.W = WSquared > 0.f ? appSqrt( WSquared ) : 0.f;
	}

	friend FArchive& operator<<(FArchive& Ar, FQuatFloat96NoW& Quat)
	{
		Ar << Quat.X;
		Ar << Quat.Y;
		Ar << Quat.Z;
		return Ar;
	}
};

class FQuatIntervalFixed32NoW
{
public:
	DWORD Packed;

	FQuatIntervalFixed32NoW()
	{}

	explicit FQuatIntervalFixed32NoW(const FQuat& Quat, const FLOAT* Mins, const FLOAT *Ranges)
	{
		FromQuat( Quat, Mins, Ranges );
	}

	void FromQuat(const FQuat& Quat, const FLOAT* Mins, const FLOAT *Ranges)
	{
		FQuat Temp( Quat );
		if ( Temp.W < 0.f )
		{
			Temp.X = -Temp.X;
			Temp.Y = -Temp.Y;
			Temp.Z = -Temp.Z;
			Temp.W = -Temp.W;
		}
		Temp.Normalize();

		Temp.X -= Mins[0];
		Temp.Y -= Mins[1];
		Temp.Z -= Mins[2];

		const DWORD PackedX = (INT)((Temp.X / Ranges[0]) * Quant11BitFactor ) + Quant11BitOffs;
		const DWORD PackedY = (INT)((Temp.Y / Ranges[1]) * Quant11BitFactor ) + Quant11BitOffs;
		const DWORD PackedZ = (INT)((Temp.Z / Ranges[2]) * Quant10BitFactor ) + Quant10BitOffs;

		// 21-31 X, 10-20 Y, 0-9 Z.
		const DWORD XShift = 21;
		const DWORD YShift = 10;
		Packed = (PackedX << XShift) | (PackedY << YShift) | (PackedZ);
	}

	void ToQuat(FQuat& Out, const FLOAT* Mins, const FLOAT *Ranges) const
	{
		const DWORD XShift = 21;
		const DWORD YShift = 10;
		const DWORD ZMask = 0x000003ff;
		const DWORD YMask = 0x001ffc00;
		const DWORD XMask = 0xffe00000;

		const DWORD UnpackedX = Packed >> XShift;
		const DWORD UnpackedY = (Packed & YMask) >> YShift;
		const DWORD UnpackedZ = (Packed & ZMask);

		const FLOAT X = ( (((INT)UnpackedX - (INT)Quant11BitOffs) / Quant11BitDiv) * Ranges[0] + Mins[0] );
		const FLOAT Y = ( (((INT)UnpackedY - (INT)Quant11BitOffs) / Quant11BitDiv) * Ranges[1] + Mins[1] );
		const FLOAT Z = ( (((INT)UnpackedZ - (INT)Quant10BitOffs) / Quant10BitDiv) * Ranges[2] + Mins[2] );
		const FLOAT WSquared = 1.f - X*X - Y*Y - Z*Z;

		Out.X = X;
		Out.Y = Y;
		Out.Z = Z;
		Out.W = WSquared > 0.f ? appSqrt( WSquared ) : 0.f;
	}

	friend FArchive& operator<<(FArchive& Ar, FQuatIntervalFixed32NoW& Quat)
	{
		Ar << Quat.Packed;
		return Ar;
	}
};

class FQuatFloat32NoW
{
public:
	DWORD Packed;

	FQuatFloat32NoW()
	{}

	explicit FQuatFloat32NoW(const FQuat& Quat)
	{
		FromQuat( Quat );
	}

	void FromQuat(const FQuat& Quat)
	{
		FQuat Temp( Quat );
		if ( Temp.W < 0.f )
		{
			Temp.X = -Temp.X;
			Temp.Y = -Temp.Y;
			Temp.Z = -Temp.Z;
			Temp.W = -Temp.W;
		}
		Temp.Normalize();

		TFloatPacker<3, 7, TRUE> Packer7e3;
		TFloatPacker<3, 6, TRUE> Packer6e3;

		const DWORD PackedX = Packer7e3.Encode( Temp.X );
		const DWORD PackedY = Packer7e3.Encode( Temp.Y );
		const DWORD PackedZ = Packer6e3.Encode( Temp.Z );

		// 21-31 X, 10-20 Y, 0-9 Z.
		const DWORD XShift = 21;
		const DWORD YShift = 10;
		Packed = (PackedX << XShift) | (PackedY << YShift) | (PackedZ);
	}

	void ToQuat(FQuat& Out) const
	{
		const DWORD XShift = 21;
		const DWORD YShift = 10;
		const DWORD ZMask = 0x000003ff;
		const DWORD YMask = 0x001ffc00;
		const DWORD XMask = 0xffe00000;

		const DWORD UnpackedX = Packed >> XShift;
		const DWORD UnpackedY = (Packed & YMask) >> YShift;
		const DWORD UnpackedZ = (Packed & ZMask);

		TFloatPacker<3, 7, TRUE> Packer7e3;
		TFloatPacker<3, 6, TRUE> Packer6e3;

		const FLOAT X = Packer7e3.Decode( UnpackedX );
		const FLOAT Y = Packer7e3.Decode( UnpackedY );
		const FLOAT Z = Packer6e3.Decode( UnpackedZ );
		const FLOAT WSquared = 1.f - X*X - Y*Y - Z*Z;

		Out.X = X;
		Out.Y = Y;
		Out.Z = Z;
		Out.W = WSquared > 0.f ? appSqrt( WSquared ) : 0.f;
	}

	friend FArchive& operator<<(FArchive& Ar, FQuatFloat32NoW& Quat)
	{
		Ar << Quat.Packed;
		return Ar;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Handy Template Decompressors
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Templated Rotation Decompressor. Generates a unique decompressor per known quantization format
 *
 * @param	Out				The FQuat to fill in.
 * @param	TopOfStream		The start of the compressed rotation stream data.
 * @param	KeyData			The compressed rotation data to decompress.
 * @return	None. 
 */
template <INT FORMAT>
FORCEINLINE void DecompressRotation(FQuat& Out, const BYTE* RESTRICT TopOfStream, const BYTE* RESTRICT KeyData)
{
	// this if-else stack gets compiled away to a single result based on the template parameter
	if ( FORMAT == ACF_None )
	{
		Out = *((FQuat*)KeyData);
	}
	else if ( FORMAT == ACF_Float96NoW )
	{
		((FQuatFloat96NoW*)KeyData)->ToQuat( Out );
	}
	else if ( FORMAT == ACF_Fixed32NoW )
	{
		((FQuatFixed32NoW*)KeyData)->ToQuat( Out );
	}
	else if ( FORMAT == ACF_Fixed48NoW )
	{
		((FQuatFixed48NoW*)KeyData)->ToQuat( Out );
	}
#if BATMAN
	else if (FORMAT == ACF_Fixed48Max)
	{
		((FQuatFixed48Max*)KeyData)->ToQuat(Out);
	}
#endif
	else if ( FORMAT == ACF_IntervalFixed32NoW )
	{
		const FLOAT* RESTRICT Mins = (FLOAT*)TopOfStream;
		const FLOAT* RESTRICT Ranges = (FLOAT*)(TopOfStream+sizeof(FLOAT)*3);
		((FQuatIntervalFixed32NoW*)KeyData)->ToQuat( Out, Mins, Ranges );
	}
	else if ( FORMAT == ACF_Float32NoW )
	{
		((FQuatFloat32NoW*)KeyData)->ToQuat( Out );
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation compression format"), (INT)FORMAT );
		Out= FQuat::Identity;
	}
}

/**
 * Templated Translation Decompressor. Generates a unique decompressor per known quantization format
 *
 * @param	Out				The FVector to fill in.
 * @param	TopOfStream		The start of the compressed translation stream data.
 * @param	KeyData			The compressed translation data to decompress.
 * @return	None. 
 */
template <INT FORMAT>
FORCEINLINE void DecompressTranslation(FVector& Out, const BYTE* RESTRICT Stream, const BYTE* RESTRICT KeyData)
{
	if ( FORMAT == ACF_None )
	{
		Out = *((FVector*)KeyData);
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation compression format"), (INT)FORMAT );
		// Silence compilers warning about a value potentially not being assigned.
		Out = FVector(0.f,0.f,0.f);
	}
}


#endif // __ANIMATIONCOMPRESSION_H__
