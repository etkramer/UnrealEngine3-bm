/*=============================================================================
	UnMath.cpp: Unreal math routines, implementation of FGlobalMath class
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/

const VectorRegister VECTOR_INV_255 = { 1.f/255.f, 1.f/255.f, 1.f/255.f, 1.f/255.f };

/** 32 bit values where GBitFlag[x] == (1<<x) */
DWORD GBitFlag[32] =
{
	(1 << 0),	(1 << 1),	(1 << 2),	(1 << 3),
	(1 << 4),	(1 << 5),	(1 << 6),	(1 << 7),
	(1 << 8),	(1 << 9),	(1 << 10),	(1 << 11),
	(1 << 12),	(1 << 13),	(1 << 14),	(1 << 15),
	(1 << 16),	(1 << 17),	(1 << 18),	(1 << 19),
	(1 << 20),	(1 << 21),	(1 << 22),	(1 << 23),
	(1 << 24),	(1 << 25),	(1 << 26),	(1 << 27),
	(1 << 28),	(1 << 29),	(1 << 30),	(1 << 31),
};

/*-----------------------------------------------------------------------------
	FGlobalMath constructor.
-----------------------------------------------------------------------------*/

// Constructor.
FGlobalMath::FGlobalMath()
{
	// Init base angle table.
	{for( INT i=0; i<NUM_ANGLES; i++ )
		TrigFLOAT[i] = appSin((FLOAT)i * 2.f * PI / (FLOAT)NUM_ANGLES);}
}

/*-----------------------------------------------------------------------------
	Conversion functions.
-----------------------------------------------------------------------------*/

// Return the FRotator corresponding to the direction that the vector
// is pointing in.  Sets Yaw and Pitch to the proper numbers, and sets
// roll to zero because the roll can't be determined from a vector.
FRotator FVector::Rotation() const
{
	FRotator R;

	// Find yaw.
	R.Yaw = appRound(appAtan2(Y,X) * (FLOAT)MAXWORD / (2.f*PI));

	// Find pitch.
	R.Pitch = appRound(appAtan2(Z,appSqrt(X*X+Y*Y)) * (FLOAT)MAXWORD / (2.f*PI));

	// Find roll.
	R.Roll = 0;

	return R;
}

// Return the FRotator corresponding to the direction that the vector
// is pointing in.  Sets Yaw and Pitch to the proper numbers, and sets
// roll to zero because the roll can't be determined from a vector.
FRotator FVector4::Rotation() const
{
	FRotator R;

	// Find yaw.
	R.Yaw = appRound(appAtan2(Y,X) * (FLOAT)MAXWORD / (2.f*PI));

	// Find pitch.
	R.Pitch = appRound(appAtan2(Z,appSqrt(X*X+Y*Y)) * (FLOAT)MAXWORD / (2.f*PI));

	// Find roll.
	R.Roll = 0;

	return R;
}

/**
 * Find good arbitrary axis vectors to represent U and V axes of a plane,
 * given just the normal.
 */
void FVector::FindBestAxisVectors( FVector& Axis1, FVector& Axis2 ) const
{
	const FLOAT NX = Abs(X);
	const FLOAT NY = Abs(Y);
	const FLOAT NZ = Abs(Z);

	// Find best basis vectors.
	if( NZ>NX && NZ>NY )	Axis1 = FVector(1,0,0);
	else					Axis1 = FVector(0,0,1);

	Axis1 = (Axis1 - *this * (Axis1 | *this)).SafeNormal();
	Axis2 = Axis1 ^ *this;
}

/** Clamps a vector to not be longer than MaxLength. */
FVector ClampLength( const FVector& V, FLOAT MaxLength)
{
	FVector ResultV = V;
	FLOAT Length = V.Size();
	if(Length > MaxLength)
	{
		ResultV *= (MaxLength/Length);
	}
	return ResultV;
}

void CreateOrthonormalBasis(FVector& XAxis,FVector& YAxis,FVector& ZAxis)
{
	// Project the X and Y axes onto the plane perpendicular to the Z axis.
	XAxis -= (XAxis | ZAxis) / (ZAxis | ZAxis) * ZAxis;
	YAxis -= (YAxis | ZAxis) / (ZAxis | ZAxis) * ZAxis;

	// If the X axis was parallel to the Z axis, choose a vector which is orthogonal to the Y and Z axes.
	if(XAxis.SizeSquared() < DELTA*DELTA)
	{
		XAxis = YAxis ^ ZAxis;
	}

	// If the Y axis was parallel to the Z axis, choose a vector which is orthogonal to the X and Z axes.
	if(YAxis.SizeSquared() < DELTA*DELTA)
	{
		YAxis = XAxis ^ ZAxis;
	}

	// Normalize the basis vectors.
	XAxis.Normalize();
	YAxis.Normalize();
	ZAxis.Normalize();
}

/** Utility to ensure angle is between +/- 180 degrees by unwinding. */
static void UnwindDegreeComponent(FLOAT& A)
{
	while(A > 180.f)
	{
		A -= 360.f;
	}

	while(A < -180.f)
	{
		A += 360.f;
	}
}

/** When this vector contains Euler angles (degrees), ensure that angles are between +/-180 */
void FVector::UnwindEuler()
{
	UnwindDegreeComponent(X);
	UnwindDegreeComponent(Y);
	UnwindDegreeComponent(Z);
}

/*-----------------------------------------------------------------------------
	FSphere implementation.
-----------------------------------------------------------------------------*/

//
// Compute a bounding sphere from an array of points.
//
FSphere::FSphere( const FVector* Pts, INT Count )
: FPlane(0,0,0,0)
{
	if( Count )
	{
		const FBox Box( Pts, Count );
		*this = FSphere( (Box.Min+Box.Max)/2, 0 );
		for( INT i=0; i<Count; i++ )
		{
			const FLOAT Dist = FDistSquared(Pts[i],*this);
			if( Dist > W )
			{
				W = Dist;
			}
		}
		W = appSqrt(W) * 1.001f;
	}
}

/*-----------------------------------------------------------------------------
	FBox implementation.
-----------------------------------------------------------------------------*/

FBox::FBox( const FVector* Points, INT Count )
: Min(0,0,0), Max(0,0,0), IsValid(0)
{
	for( INT i=0; i<Count; i++ )
	{
		*this += Points[i];
	}
}

//
//	FBox::TransformBy
//
FBox FBox::TransformBy(const FMatrix& M) const
{
	// If we are not valid, return another invalid box.
	if(!IsValid)
	{
		return FBox(0);
	}

	VectorRegister Vertices[8];
	VectorRegister m0 = VectorLoadAligned( M.M[0] );
	VectorRegister m1 = VectorLoadAligned( M.M[1] );
	VectorRegister m2 = VectorLoadAligned( M.M[2] );
	VectorRegister m3 = VectorLoadAligned( M.M[3] );
	Vertices[0]   = VectorLoadFloat3( &Min );
	Vertices[1]   = VectorSetFloat3( Min.X, Min.Y, Max.Z );
	Vertices[2]   = VectorSetFloat3( Min.X, Max.Y, Min.Z );
	Vertices[3]   = VectorSetFloat3( Max.X, Min.Y, Min.Z );
	Vertices[4]   = VectorSetFloat3( Max.X, Max.Y, Min.Z );
	Vertices[5]   = VectorSetFloat3( Max.X, Min.Y, Max.Z );
	Vertices[6]   = VectorSetFloat3( Min.X, Max.Y, Max.Z );
	Vertices[7]   = VectorLoadFloat3( &Max );
	VectorRegister r0 = VectorMultiply( VectorReplicate(Vertices[0],0), m0 );
	VectorRegister r1 = VectorMultiply( VectorReplicate(Vertices[1],0), m0 );
	VectorRegister r2 = VectorMultiply( VectorReplicate(Vertices[2],0), m0 );
	VectorRegister r3 = VectorMultiply( VectorReplicate(Vertices[3],0), m0 );
	VectorRegister r4 = VectorMultiply( VectorReplicate(Vertices[4],0), m0 );
	VectorRegister r5 = VectorMultiply( VectorReplicate(Vertices[5],0), m0 );
	VectorRegister r6 = VectorMultiply( VectorReplicate(Vertices[6],0), m0 );
	VectorRegister r7 = VectorMultiply( VectorReplicate(Vertices[7],0), m0 );

	r0 = VectorMultiplyAdd( VectorReplicate(Vertices[0],1), m1, r0 );
	r1 = VectorMultiplyAdd( VectorReplicate(Vertices[1],1), m1, r1 );
	r2 = VectorMultiplyAdd( VectorReplicate(Vertices[2],1), m1, r2 );
	r3 = VectorMultiplyAdd( VectorReplicate(Vertices[3],1), m1, r3 );
	r4 = VectorMultiplyAdd( VectorReplicate(Vertices[4],1), m1, r4 );
	r5 = VectorMultiplyAdd( VectorReplicate(Vertices[5],1), m1, r5 );
	r6 = VectorMultiplyAdd( VectorReplicate(Vertices[6],1), m1, r6 );
	r7 = VectorMultiplyAdd( VectorReplicate(Vertices[7],1), m1, r7 );

	r0 = VectorMultiplyAdd( VectorReplicate(Vertices[0],2), m2, r0 );
	r1 = VectorMultiplyAdd( VectorReplicate(Vertices[1],2), m2, r1 );
	r2 = VectorMultiplyAdd( VectorReplicate(Vertices[2],2), m2, r2 );
	r3 = VectorMultiplyAdd( VectorReplicate(Vertices[3],2), m2, r3 );
	r4 = VectorMultiplyAdd( VectorReplicate(Vertices[4],2), m2, r4 );
	r5 = VectorMultiplyAdd( VectorReplicate(Vertices[5],2), m2, r5 );
	r6 = VectorMultiplyAdd( VectorReplicate(Vertices[6],2), m2, r6 );
	r7 = VectorMultiplyAdd( VectorReplicate(Vertices[7],2), m2, r7 );

	r0 = VectorAdd( r0, m3 );
	r1 = VectorAdd( r1, m3 );
	r2 = VectorAdd( r2, m3 );
	r3 = VectorAdd( r3, m3 );
	r4 = VectorAdd( r4, m3 );
	r5 = VectorAdd( r5, m3 );
	r6 = VectorAdd( r6, m3 );
	r7 = VectorAdd( r7, m3 );

	FBox NewBox;
	VectorRegister min0 = VectorMin( r0, r1 );
	VectorRegister min1 = VectorMin( r2, r3 );
	VectorRegister min2 = VectorMin( r4, r5 );
	VectorRegister min3 = VectorMin( r6, r7 );
	VectorRegister max0 = VectorMax( r0, r1 );
	VectorRegister max1 = VectorMax( r2, r3 );
	VectorRegister max2 = VectorMax( r4, r5 );
	VectorRegister max3 = VectorMax( r6, r7 );
	min0 = VectorMin( min0, min1 );
	min1 = VectorMin( min2, min3 );
	max0 = VectorMax( max0, max1 );
	max1 = VectorMax( max2, max3 );
	min0 = VectorMin( min0, min1 );
	max0 = VectorMax( max0, max1 );
	VectorStoreFloat3( min0, &NewBox.Min );
	VectorStoreFloat3( max0, &NewBox.Max );
	NewBox.IsValid = 1;

	return NewBox;
}


/** 
* Transforms and projects a world bounding box to screen space
*
* @param	ProjM - projection matrix
* @return	transformed box
*/
FBox FBox::TransformProjectBy( const FMatrix& ProjM ) const
{
	FVector Vertices[8] = 
	{
		FVector(Min),
		FVector(Min.X, Min.Y, Max.Z),
		FVector(Min.X, Max.Y, Min.Z),
		FVector(Max.X, Min.Y, Min.Z),
		FVector(Max.X, Max.Y, Min.Z),
		FVector(Max.X, Min.Y, Max.Z),
		FVector(Min.X, Max.Y, Max.Z),
		FVector(Max)
	};

	FBox NewBox(0);
	for(INT VertexIndex = 0;VertexIndex < ARRAY_COUNT(Vertices);VertexIndex++)
	{
		FVector4 ProjectedVertex = ProjM.TransformFVector(Vertices[VertexIndex]);
		NewBox += ((FVector)ProjectedVertex) / ProjectedVertex.W;
	}

	return NewBox;
}

/*-----------------------------------------------------------------------------
	FRotator functions.
-----------------------------------------------------------------------------*/

FRotator::FRotator(const FQuat& Quat)
{
	*this = FQuatRotationTranslationMatrix( Quat,  FVector(0.f) ).Rotator();
}

//
// Convert a rotation into a vector facing in its direction.
//
FVector FRotator::Vector() const
{
	return FRotationMatrix( *this ).GetAxis(0);
}

//
// Convert a rotation into a quaternion.
//

FQuat FRotator::Quaternion() const
{
	return FQuat( FRotationMatrix( *this ) );
}

/** Convert a Rotator into floating-point Euler angles (in degrees). */
FVector FRotator::Euler() const
{
	return FVector( Roll * (180.f / 32768.f), Pitch * (180.f / 32768.f), Yaw * (180.f / 32768.f) );
}

/** Convert a vector of floating-point Euler angles (in degrees) into a Rotator. */
FRotator FRotator::MakeFromEuler(const FVector& Euler)
{
	return FRotator( appTrunc(Euler.Y * (32768.f / 180.f)), appTrunc(Euler.Z * (32768.f / 180.f)), appTrunc(Euler.X * (32768.f / 180.f)) );
}



/** 
 *	Decompose this Rotator into a Winding part (multiples of 65536) and a Remainder part. 
 *	Remainder will always be in -32768 -> 32768 range.
 */
void FRotator::GetWindingAndRemainder(FRotator& Winding, FRotator& Remainder) const
{
	//// YAW
	Remainder.Yaw = Yaw & 65535;
	Winding.Yaw = Yaw - Remainder.Yaw;

	if(Remainder.Yaw > 32768)
	{
		Winding.Yaw += 65536;
		Remainder.Yaw -= 65536;
	}
	else if(Remainder.Yaw < -32768)
	{
		Winding.Yaw -= 65536;
		Remainder.Yaw += 65536;
	}

	//// PITCH
	Remainder.Pitch = Pitch & 65535;
	Winding.Pitch = Pitch - Remainder.Pitch;

	if(Remainder.Pitch > 32768)
	{
		Winding.Pitch += 65536;
		Remainder.Pitch -= 65536;
	}
	else if(Remainder.Pitch < -32768)
	{
		Winding.Pitch -= 65536;
		Remainder.Pitch += 65536;
	}

	//// ROLL
	Remainder.Roll = Roll & 65535;
	Winding.Roll = Roll - Remainder.Roll;

	if(Remainder.Roll > 32768)
	{
		Winding.Roll += 65536;
		Remainder.Roll -= 65536;
	}
	else if(Remainder.Roll < -32768)
	{
		Winding.Roll -= 65536;
		Remainder.Roll += 65536;
	}
}

/** 
 *	Alter this Rotator to form the 'shortest' rotation that will have the effect. 
 *	First clips the rotation between +/- 65535, then checks direction for each component.
 */
void FRotator::MakeShortestRoute()
{
	//// YAW

	// Clip rotation to +/- 65535 range
	Yaw = Yaw & 65535;

	// Then ensure it takes the 'shortest' route.
	if(Yaw > 32768)
		Yaw -= 65535;
	else if(Yaw < -32768)
		Yaw += 65535;


	//// PITCH

	Pitch = Pitch & 65535;

	if(Pitch > 32768)
		Pitch -= 65535;
	else if(Pitch < -32768)
		Pitch += 65535;


	//// ROLL

	Roll = Roll & 65535;

	if(Roll > 32768)
		Roll -= 65535;
	else if(Roll < -32768)
		Roll += 65535;
}

/*-----------------------------------------------------------------------------
	FQuaternion and FMatrix support functions
-----------------------------------------------------------------------------*/

FRotator FMatrix::Rotator() const
{
	const FVector		XAxis	= GetAxis( 0 );
	const FVector		YAxis	= GetAxis( 1 );
	const FVector		ZAxis	= GetAxis( 2 );

	FRotator	Rotator	= FRotator( 
									appRound(appAtan2( XAxis.Z, appSqrt(Square(XAxis.X)+Square(XAxis.Y)) ) * 32768.f / PI), 
									appRound(appAtan2( XAxis.Y, XAxis.X ) * 32768.f / PI), 
									0 
								);
	
	const FVector		SYAxis	= FRotationMatrix( Rotator ).GetAxis(1);
	Rotator.Roll		= appRound(appAtan2( ZAxis | SYAxis, YAxis | SYAxis ) * 32768.f / PI);
	return Rotator;
}

const FMatrix FMatrix::Identity(FPlane(1,0,0,0),FPlane(0,1,0,0),FPlane(0,0,1,0),FPlane(0,0,0,1));

const FQuat FQuat::Identity(0,0,0,1);

FString FMatrix::ToString() const
{
	FString Output;

	Output += FString::Printf(TEXT("[%f %f %f %f] "), M[0][0], M[0][1], M[0][2], M[0][3]);
	Output += FString::Printf(TEXT("[%f %f %f %f] "), M[1][0], M[1][1], M[1][2], M[1][3]);
	Output += FString::Printf(TEXT("[%f %f %f %f] "), M[2][0], M[2][1], M[2][2], M[2][3]);
	Output += FString::Printf(TEXT("[%f %f %f %f] "), M[3][0], M[3][1], M[3][2], M[3][3]);

	return Output;
}

FQuat::FQuat( const FMatrix& M )
{
	// If Matrix is NULL, return Identity quaternion.
	if( M.GetAxis(0).IsNearlyZero() && M.GetAxis(1).IsNearlyZero() && M.GetAxis(2).IsNearlyZero() )
	{
		*this = FQuat::Identity;
		return;
	}

#if !FINAL_RELEASE && !CONSOLE
	// Make sure the Rotation part of the Matrix is unit length.
	check( (Abs(1.f - Abs(M.RotDeterminant())) <= 0.01f) );
#endif

	//const MeReal *const t = (MeReal *) tm;
	FLOAT	s;

	// Check diagonal (trace)
	const FLOAT tr = M.M[0][0] + M.M[1][1] + M.M[2][2];

	if (tr > 0.0f) 
	{
		FLOAT InvS = appInvSqrt(tr + 1.f);
		this->W = 0.5f * (1.f / InvS);
		s = 0.5f * InvS;

		this->X = (M.M[1][2] - M.M[2][1]) * s;
		this->Y = (M.M[2][0] - M.M[0][2]) * s;
		this->Z = (M.M[0][1] - M.M[1][0]) * s;
	} 
	else 
	{
		// diagonal is negative
		INT i = 0;

		if (M.M[1][1] > M.M[0][0])
			i = 1;

		if (M.M[2][2] > M.M[i][i])
			i = 2;

		static const INT nxt[3] = { 1, 2, 0 };
		const INT j = nxt[i];
		const INT k = nxt[j];

		s = M.M[i][i] - M.M[j][j] - M.M[k][k] + 1.0f;

		FLOAT InvS = appInvSqrt(s);

		FLOAT qt[4];
		qt[i] = 0.5f * (1.f / InvS);

		s = 0.5f * InvS;

		qt[3] = (M.M[j][k] - M.M[k][j]) * s;
		qt[j] = (M.M[i][j] + M.M[j][i]) * s;
		qt[k] = (M.M[i][k] + M.M[k][i]) * s;

		this->X = qt[0];
		this->Y = qt[1];
		this->Z = qt[2];
		this->W = qt[3];
	}
}

/** Convert a vector of floating-point Euler angles (in degrees) into a Quaternion. */
FQuat FQuat::MakeFromEuler(const FVector& Euler)
{
	return FQuat( FRotationTranslationMatrix( FRotator::MakeFromEuler(Euler), FVector(0.f) ) );
}

/** Convert a Quaternion into floating-point Euler angles (in degrees). */
FVector FQuat::Euler() const
{
	return FQuatRotationTranslationMatrix( *this, FVector(0.f) ).Rotator().Euler();
}

FQuat FQuatFindBetween(const FVector& vec1, const FVector& vec2)
{
	const FVector cross = vec1 ^ vec2;
	const FLOAT crossMag = cross.Size();

	// If these vectors are basically parallel - just return identity quaternion (ie no rotation).
	if(crossMag < KINDA_SMALL_NUMBER)
	{
		return FQuat::Identity;
	}

	if(crossMag < KINDA_SMALL_NUMBER)
	{
		const FLOAT Dot = vec1 | vec2;
		if(Dot > -KINDA_SMALL_NUMBER)
		{
			return FQuat::Identity; // no rotation
		}
		else
		{
			// rotation by 180 degrees around a vector orthogonal to vec1 & vec2
			FVector Vec = vec1.SizeSquared() > vec2.SizeSquared() ? vec1 : vec2;
			Vec.Normalize();

			FVector AxisA, AxisB;
			Vec.FindBestAxisVectors(AxisA, AxisB);

			return FQuat(AxisA.X, AxisA.Y, AxisA.Z, 0.f); // (axis*sin(pi/2), cos(pi/2)) = (axis, 0)
		}
	}

	FLOAT angle = appAsin(crossMag);

	const FLOAT dot = vec1 | vec2;
	if(dot < 0.0f)
	{
		angle = PI - angle;
	}

	const FLOAT sinHalfAng = appSin(0.5f * angle);
	const FLOAT cosHalfAng = appCos(0.5f * angle);
	const FVector axis = cross / crossMag;

	return FQuat(
		sinHalfAng * axis.X,
		sinHalfAng * axis.Y,
		sinHalfAng * axis.Z,
		cosHalfAng );
}

// Returns quaternion with W=0 and V=theta*v.

FQuat FQuat::Log() const
{
	FQuat Result;
	Result.W = 0.f;

	if ( Abs(W) < 1.f )
	{
		const FLOAT Angle = appAcos(W);
		const FLOAT SinAngle = appSin(Angle);

		if ( Abs(SinAngle) >= SMALL_NUMBER )
		{
			const FLOAT Scale = Angle/SinAngle;
			Result.X = Scale*X;
			Result.Y = Scale*Y;
			Result.Z = Scale*Z;

			return Result;
		}
	}

	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;

	return Result;
}

// Assumes a quaternion with W=0 and V=theta*v (where |v| = 1).
// Exp(q) = (sin(theta)*v, cos(theta))

FQuat FQuat::Exp() const
{
	const FLOAT Angle = appSqrt(X*X + Y*Y + Z*Z);
	const FLOAT SinAngle = appSin(Angle);

	FQuat Result;
	Result.W = appCos(Angle);

	if ( Abs(SinAngle) >= SMALL_NUMBER )
	{
		const FLOAT Scale = SinAngle/Angle;
		Result.X = Scale*X;
		Result.Y = Scale*Y;
		Result.Z = Scale*Z;
	}
	else
	{
		Result.X = X;
		Result.Y = Y;
		Result.Z = Z;
	}

	return Result;
}

/*-----------------------------------------------------------------------------
	Swept-Box vs Box test.
-----------------------------------------------------------------------------*/

/* Line-extent/Box Test Util */
UBOOL FLineExtentBoxIntersection(const FBox& inBox, 
								 const FVector& Start, 
								 const FVector& End,
								 const FVector& Extent,
								 FVector& HitLocation,
								 FVector& HitNormal,
								 FLOAT& HitTime)
{
	FBox box = inBox;
	box.Max.X += Extent.X;
	box.Max.Y += Extent.Y;
	box.Max.Z += Extent.Z;
	
	box.Min.X -= Extent.X;
	box.Min.Y -= Extent.Y;
	box.Min.Z -= Extent.Z;

	const FVector Dir = (End - Start);
	
	FVector	Time;
	UBOOL	Inside = 1;
	FLOAT   faceDir[3] = {1, 1, 1};
	
	/////////////// X
	if(Start.X < box.Min.X)
	{
		if(Dir.X <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			faceDir[0] = -1;
			Time.X = (box.Min.X - Start.X) / Dir.X;
		}
	}
	else if(Start.X > box.Max.X)
	{
		if(Dir.X >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.X = (box.Max.X - Start.X) / Dir.X;
		}
	}
	else
		Time.X = 0.0f;
	
	/////////////// Y
	if(Start.Y < box.Min.Y)
	{
		if(Dir.Y <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			faceDir[1] = -1;
			Time.Y = (box.Min.Y - Start.Y) / Dir.Y;
		}
	}
	else if(Start.Y > box.Max.Y)
	{
		if(Dir.Y >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.Y = (box.Max.Y - Start.Y) / Dir.Y;
		}
	}
	else
		Time.Y = 0.0f;
	
	/////////////// Z
	if(Start.Z < box.Min.Z)
	{
		if(Dir.Z <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			faceDir[2] = -1;
			Time.Z = (box.Min.Z - Start.Z) / Dir.Z;
		}
	}
	else if(Start.Z > box.Max.Z)
	{
		if(Dir.Z >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.Z = (box.Max.Z - Start.Z) / Dir.Z;
		}
	}
	else
		Time.Z = 0.0f;
	
	// If the line started inside the box (ie. player started in contact with the fluid)
	if(Inside)
	{
		HitLocation = Start;
		HitNormal = FVector(0, 0, 1);
		HitTime = 0;
		return 1;
	}
	// Otherwise, calculate when hit occured
	else
	{	
		if(Time.Y > Time.Z)
		{
			HitTime = Time.Y;
			HitNormal = FVector(0, faceDir[1], 0);
		}
		else
		{
			HitTime = Time.Z;
			HitNormal = FVector(0, 0, faceDir[2]);
		}
		
		if(Time.X > HitTime)
		{
			HitTime = Time.X;
			HitNormal = FVector(faceDir[0], 0, 0);
		}
		
		if(HitTime >= 0.0f && HitTime <= 1.0f)
		{
			HitLocation = Start + Dir * HitTime;
			const FLOAT BOX_SIDE_THRESHOLD = 0.1f;
			if(	HitLocation.X > box.Min.X - BOX_SIDE_THRESHOLD && HitLocation.X < box.Max.X + BOX_SIDE_THRESHOLD &&
				HitLocation.Y > box.Min.Y - BOX_SIDE_THRESHOLD && HitLocation.Y < box.Max.Y + BOX_SIDE_THRESHOLD &&
				HitLocation.Z > box.Min.Z - BOX_SIDE_THRESHOLD && HitLocation.Z < box.Max.Z + BOX_SIDE_THRESHOLD)
			{				
				return 1;
			}
		}
		
		return 0;
	}
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
FLOAT EvaluateBezier(const FVector* ControlPoints, INT NumPoints, TArray<FVector>& OutPoints)
{
	check( ControlPoints );
	check( NumPoints >= 2 );

	// var q is the change in t between successive evaluations.
	const FLOAT q = 1.f/(NumPoints-1); // q is dependent on the number of GAPS = POINTS-1

	// recreate the names used in the derivation
	const FVector& P0 = ControlPoints[0];
	const FVector& P1 = ControlPoints[1];
	const FVector& P2 = ControlPoints[2];
	const FVector& P3 = ControlPoints[3];

	// coefficients of the cubic polynomial that we're FDing -
	const FVector a = P0;
	const FVector b = 3*(P1-P0);
	const FVector c = 3*(P2-2*P1+P0);
	const FVector d = P3-3*P2+3*P1-P0;

	// initial values of the poly and the 3 diffs -
	FVector S  = a;						// the poly value
	FVector U  = b*q + c*q*q + d*q*q*q;	// 1st order diff (quadratic)
	FVector V  = 2*c*q*q + 6*d*q*q*q;	// 2nd order diff (linear)
	FVector W  = 6*d*q*q*q;				// 3rd order diff (constant)

	// Path length.
	FLOAT Length = 0.f;

	FVector OldPos = P0;
	OutPoints.AddItem( P0 );	// first point on the curve is always P0.

	for( INT i = 1 ; i < NumPoints ; ++i )
	{
		// calculate the next value and update the deltas
		S += U;			// update poly value
		U += V;			// update 1st order diff value
		V += W;			// update 2st order diff value
		// 3rd order diff is constant => no update needed.

		// Update Length.
		Length += FDist( S, OldPos );
		OldPos  = S;

		OutPoints.AddItem( S );
	}

	// Return path length as experienced in sequence (linear interpolation between points).
	return Length;
}

/*-----------------------------------------------------------------------------
	Cubic Quaternion interpolation support functions
-----------------------------------------------------------------------------*/

/**
 * Spherical interpolation. Will correct alignment. Output is not normalized.
 */
FQuat SlerpQuat(const FQuat& Quat1,const FQuat& Quat2, float Slerp)
{
	// Get cosine of angle between quats.
	const FLOAT RawCosom = 
		    Quat1.X * Quat2.X +
			Quat1.Y * Quat2.Y +
			Quat1.Z * Quat2.Z +
			Quat1.W * Quat2.W;
	// Unaligned quats - compensate, results in taking shorter route.
	const FLOAT Cosom = appFloatSelect( RawCosom, RawCosom, -RawCosom );
	
	FLOAT Scale0, Scale1;

	if( Cosom < 0.9999f )
	{	
		const FLOAT Omega = appAcos(Cosom);
		const FLOAT InvSin = 1.f/appSin(Omega);
		Scale0 = appSin( (1.f - Slerp) * Omega ) * InvSin;
		Scale1 = appSin( Slerp * Omega ) * InvSin;
	}
	else
	{
		// Use linear interpolation.
		Scale0 = 1.0f - Slerp;
		Scale1 = Slerp;	
	}

	// In keeping with our flipped Cosom:
	Scale1 = appFloatSelect( RawCosom, Scale1, -Scale1 );

	FQuat Result;
		
	Result.X = Scale0 * Quat1.X + Scale1 * Quat2.X;
	Result.Y = Scale0 * Quat1.Y + Scale1 * Quat2.Y;
	Result.Z = Scale0 * Quat1.Z + Scale1 * Quat2.Z;
	Result.W = Scale0 * Quat1.W + Scale1 * Quat2.W;

	return Result;
}

// Simpler Slerp that doesn't do any checks for 'shortest distance' etc.
// We need this for the cubic interpolation stuff so that the multiple Slerps dont go in different directions.
FQuat SlerpQuatFullPath(const FQuat &quat1, const FQuat &quat2, FLOAT Alpha )
{
	const FLOAT CosAngle = Clamp(quat1 | quat2, -1.f, 1.f);
	const FLOAT Angle = appAcos(CosAngle);

	//debugf( TEXT("CosAngle: %f Angle: %f"), CosAngle, Angle );

	if ( Abs(Angle) < KINDA_SMALL_NUMBER )
	{
		return quat1;
	}

	const FLOAT SinAngle = appSin(Angle);
	const FLOAT InvSinAngle = 1.f/SinAngle;

	const FLOAT Scale0 = appSin((1.0f-Alpha)*Angle)*InvSinAngle;
	const FLOAT Scale1 = appSin(Alpha*Angle)*InvSinAngle;

	return quat1*Scale0 + quat2*Scale1;
}

// Given start and end quaternions of quat1 and quat2, and tangents at those points tang1 and tang2, calculate the point at Alpha (between 0 and 1) between them.
FQuat SquadQuat(const FQuat& quat1, const FQuat& tang1, const FQuat& quat2, const FQuat& tang2, FLOAT Alpha)
{
	const FQuat Q1 = SlerpQuatFullPath(quat1, quat2, Alpha);
	//debugf(TEXT("Q1: %f %f %f %f"), Q1.X, Q1.Y, Q1.Z, Q1.W);

	const FQuat Q2 = SlerpQuatFullPath(tang1, tang2, Alpha);
	//debugf(TEXT("Q2: %f %f %f %f"), Q2.X, Q2.Y, Q2.Z, Q2.W);

	const FQuat Result = SlerpQuatFullPath(Q1, Q2, 2.f * Alpha * (1.f - Alpha));
	//FQuat Result = SlerpQuat(Q1, Q2, 2.f * Alpha * (1.f - Alpha));
	//debugf(TEXT("Result: %f %f %f %f"), Result.X, Result.Y, Result.Z, Result.W);

	return Result;
}


void LegacyCalcQuatTangents( const FQuat& PrevP, const FQuat& P, const FQuat& NextP, FLOAT Tension, FQuat& OutTan )
{
	const FQuat InvP = -P;
	const FQuat Part1 = (InvP * PrevP).Log();
	const FQuat Part2 = (InvP * NextP).Log();

	const FQuat PreExp = (Part1 + Part2) * -0.25f;

	OutTan = P * PreExp.Exp();
}


void CalcQuatTangents( const FQuat& PrevP, const FQuat& P, const FQuat& NextP, FLOAT Tension, FQuat& OutTan )
{
	const FQuat InvP = -P;
	const FQuat Part1 = (InvP * PrevP).Log();
	const FQuat Part2 = (InvP * NextP).Log();

	const FQuat PreExp = (Part1 + Part2) * -0.5f;

	OutTan = P * PreExp.Exp();
}

static void FindBounds( FLOAT& OutMin, FLOAT& OutMax,  FLOAT Start, FLOAT StartLeaveTan, FLOAT StartT, FLOAT End, FLOAT EndArriveTan, FLOAT EndT, UBOOL bCurve )
{
	OutMin = Min( Start, End );
	OutMax = Max( Start, End );

	// Do we need to consider extermeties of a curve?
	if(bCurve)
	{
		const FLOAT a = 6.f*Start + 3.f*StartLeaveTan + 3.f*EndArriveTan - 6.f*End;
		const FLOAT b = -6.f*Start - 4.f*StartLeaveTan - 2.f*EndArriveTan + 6.f*End;
		const FLOAT c = StartLeaveTan;

		const FLOAT Discriminant = (b*b) - (4.f*a*c);
		if(Discriminant > 0.f)
		{
			const FLOAT SqrtDisc = appSqrt( Discriminant );

			const FLOAT x0 = (-b + SqrtDisc)/(2.f*a); // x0 is the 'Alpha' ie between 0 and 1
			const FLOAT t0 = StartT + x0*(EndT - StartT); // Then t0 is the actual 'time' on the curve
			if(t0 > StartT && t0 < EndT)
			{
				const FLOAT Val = CubicInterp( Start, StartLeaveTan, End, EndArriveTan, x0 );

				OutMin = ::Min( OutMin, Val );
				OutMax = ::Max( OutMax, Val );
			}

			const FLOAT x1 = (-b - SqrtDisc)/(2.f*a);
			const FLOAT t1 = StartT + x1*(EndT - StartT);
			if(t1 > StartT && t1 < EndT)
			{
				const FLOAT Val = CubicInterp( Start, StartLeaveTan, End, EndArriveTan, x1 );

				OutMin = ::Min( OutMin, Val );
				OutMax = ::Max( OutMax, Val );
			}
		}
	}
}

void CurveFloatFindIntervalBounds( const FInterpCurvePoint<FLOAT>& Start, const FInterpCurvePoint<FLOAT>& End, FLOAT& CurrentMin, FLOAT& CurrentMax )
{
	const UBOOL bIsCurve = Start.IsCurveKey();

	FLOAT OutMin, OutMax;

	FindBounds(OutMin, OutMax, Start.OutVal, Start.LeaveTangent, Start.InVal, End.OutVal, End.ArriveTangent, End.InVal, bIsCurve);

	CurrentMin = ::Min( CurrentMin, OutMin );
	CurrentMax = ::Max( CurrentMax, OutMax );
}

void CurveVector2DFindIntervalBounds( const FInterpCurvePoint<FVector2D>& Start, const FInterpCurvePoint<FVector2D>& End, FVector2D& CurrentMin, FVector2D& CurrentMax )
{
	const UBOOL bIsCurve = Start.IsCurveKey();

	FLOAT OutMin, OutMax;

	FindBounds(OutMin, OutMax, Start.OutVal.X, Start.LeaveTangent.X, Start.InVal, End.OutVal.X, End.ArriveTangent.X, End.InVal, bIsCurve);
	CurrentMin.X = ::Min( CurrentMin.X, OutMin );
	CurrentMax.X = ::Max( CurrentMax.X, OutMax );

	FindBounds(OutMin, OutMax, Start.OutVal.Y, Start.LeaveTangent.Y, Start.InVal, End.OutVal.Y, End.ArriveTangent.Y, End.InVal, bIsCurve);
	CurrentMin.Y = ::Min( CurrentMin.Y, OutMin );
	CurrentMax.Y = ::Max( CurrentMax.Y, OutMax );
}

void CurveVectorFindIntervalBounds( const FInterpCurvePoint<FVector>& Start, const FInterpCurvePoint<FVector>& End, FVector& CurrentMin, FVector& CurrentMax )
{
	const UBOOL bIsCurve = Start.IsCurveKey();

	FLOAT OutMin, OutMax;

	FindBounds(OutMin, OutMax, Start.OutVal.X, Start.LeaveTangent.X, Start.InVal, End.OutVal.X, End.ArriveTangent.X, End.InVal, bIsCurve);
	CurrentMin.X = ::Min( CurrentMin.X, OutMin );
	CurrentMax.X = ::Max( CurrentMax.X, OutMax );

	FindBounds(OutMin, OutMax, Start.OutVal.Y, Start.LeaveTangent.Y, Start.InVal, End.OutVal.Y, End.ArriveTangent.Y, End.InVal, bIsCurve);
	CurrentMin.Y = ::Min( CurrentMin.Y, OutMin );
	CurrentMax.Y = ::Max( CurrentMax.Y, OutMax );

	FindBounds(OutMin, OutMax, Start.OutVal.Z, Start.LeaveTangent.Z, Start.InVal, End.OutVal.Z, End.ArriveTangent.Z, End.InVal, bIsCurve);
	CurrentMin.Z = ::Min( CurrentMin.Z, OutMin );
	CurrentMax.Z = ::Max( CurrentMax.Z, OutMax );
}

void CurveTwoVectorsFindIntervalBounds(const FInterpCurvePoint<FTwoVectors>& Start, const FInterpCurvePoint<FTwoVectors>& End, FTwoVectors& CurrentMin, FTwoVectors& CurrentMax)
{
	const UBOOL bIsCurve = Start.IsCurveKey();

	FLOAT OutMin;
	FLOAT OutMax;

	// Do the first curve
	FindBounds(OutMin, OutMax, Start.OutVal.v1.X, Start.LeaveTangent.v1.X, Start.InVal, End.OutVal.v1.X, End.ArriveTangent.v1.X, End.InVal, bIsCurve);
	CurrentMin.v1.X = ::Min( CurrentMin.v1.X, OutMin );
	CurrentMax.v1.X = ::Max( CurrentMax.v1.X, OutMax );

	FindBounds(OutMin, OutMax, Start.OutVal.v1.Y, Start.LeaveTangent.v1.Y, Start.InVal, End.OutVal.v1.Y, End.ArriveTangent.v1.Y, End.InVal, bIsCurve);
	CurrentMin.v1.Y = ::Min( CurrentMin.v1.Y, OutMin );
	CurrentMax.v1.Y = ::Max( CurrentMax.v1.Y, OutMax );

	FindBounds(OutMin, OutMax, Start.OutVal.v1.Z, Start.LeaveTangent.v1.Z, Start.InVal, End.OutVal.v1.Z, End.ArriveTangent.v1.Z, End.InVal, bIsCurve);
	CurrentMin.v1.Z = ::Min( CurrentMin.v1.Z, OutMin );
	CurrentMax.v1.Z = ::Max( CurrentMax.v1.Z, OutMax );

	// Do the second curve
	FindBounds(OutMin, OutMax, Start.OutVal.v2.X, Start.LeaveTangent.v2.X, Start.InVal, End.OutVal.v2.X, End.ArriveTangent.v2.X, End.InVal, bIsCurve);
	CurrentMin.v2.X = ::Min( CurrentMin.v2.X, OutMin );
	CurrentMax.v2.X = ::Max( CurrentMax.v2.X, OutMax );

	FindBounds(OutMin, OutMax, Start.OutVal.v2.Y, Start.LeaveTangent.v2.Y, Start.InVal, End.OutVal.v2.Y, End.ArriveTangent.v2.Y, End.InVal, bIsCurve);
	CurrentMin.v2.Y = ::Min( CurrentMin.v2.Y, OutMin );
	CurrentMax.v2.Y = ::Max( CurrentMax.v2.Y, OutMax );

	FindBounds(OutMin, OutMax, Start.OutVal.v2.Z, Start.LeaveTangent.v2.Z, Start.InVal, End.OutVal.v2.Z, End.ArriveTangent.v2.Z, End.InVal, bIsCurve);
	CurrentMin.v2.Z = ::Min( CurrentMin.v2.Z, OutMin );
	CurrentMax.v2.Z = ::Max( CurrentMax.v2.Z, OutMax );
}


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
FLOAT PointDistToLine(const FVector &Point, const FVector &Line, const FVector &Origin, FVector &OutClosestPoint)
{
	const FVector SafeDir = Line.SafeNormal();
	OutClosestPoint = Origin + (SafeDir * ((Point-Origin) | SafeDir));
	return (OutClosestPoint-Point).Size();
}

FLOAT PointDistToLine(const FVector &Point, const FVector &Line, const FVector &Origin)
{
	const FVector SafeDir = Line.SafeNormal();
	const FVector OutClosestPoint = Origin + (SafeDir * ((Point-Origin) | SafeDir));
	return (OutClosestPoint-Point).Size();
}


/**
 * Returns closest distance from a point to a segment.
 * The idea is to project point on line formed by segment.
 * Then we see if the closest point on the line is outside of segment or inside.
 *
 * @param	Point			point to check distance for
 * @param	StartPoint		StartPoint of segment
 * @param	EndPoint		EndPoint of segment
 * @param	OutClosestPoint	Closest point on segment.
 *
 * @return	closest distance from Point to segment defined by (StartPoint, EndPoint).
 */
FLOAT PointDistToSegment(const FVector &Point, const FVector &StartPoint, const FVector &EndPoint, FVector &OutClosestPoint) 
{
    const FVector Segment = EndPoint - StartPoint;
    const FVector VectToPoint = Point - StartPoint;

	// See if closest point is before StartPoint
	const FLOAT Dot1 = VectToPoint | Segment;
    if( Dot1 <= 0 )
	{
		OutClosestPoint = StartPoint;
        return VectToPoint.Size();
	}

	// See if closest point is beyond EndPoint
    const FLOAT Dot2 = Segment | Segment;
    if( Dot2 <= Dot1 )
	{
		OutClosestPoint = EndPoint;
        return (Point - EndPoint).Size();
	}

	// Closest Point is within segment
	OutClosestPoint = StartPoint + Segment * (Dot1 / Dot2);
    return (Point - OutClosestPoint).Size();
}

/** 
 * Find closest points between 2 segments.
 * @param	(A1, B1)	defines the first segment.
 * @param	(A2, B2)	defines the second segment.
 * @param	OutP1		Closest point on segment 1 to segment 2.
 * @param	OutP2		Closest point on segment 2 to segment 1.
 */
void SegmentDistToSegment(FVector A1, FVector B1, FVector A2, FVector B2, FVector& OutP1, FVector& OutP2)
{
	// Segments
	const	FVector	S1		= B1 - A1;
	const	FVector	S2		= B2 - A2;
	const	FVector	S3		= A1 - A2;

	const	FLOAT	Dot11	= S1 | S1;	// always >= 0
	const	FLOAT	Dot22	= S2 | S2;	// always >= 0
	const	FLOAT	Dot12	= S1 | S2;
	const	FLOAT	Dot13	= S1 | S3;
	const	FLOAT	Dot23	= S2 | S3;

	// Numerator
			FLOAT	N1, N2;

	// Denominator
	const	FLOAT	D		= Dot11*Dot22 - Dot12*Dot12;	// always >= 0
			FLOAT	D1		= D;		// T1 = N1 / D1, default D1 = D >= 0
			FLOAT	D2		= D;		// T2 = N2 / D2, default D2 = D >= 0

	// compute the line parameters of the two closest points
	if( D < KINDA_SMALL_NUMBER ) 
	{ 
		// the lines are almost parallel
		N1 = 0.f;	// force using point A on segment S1
		D1 = 1.f;	// to prevent possible division by 0 later
		N2 = Dot23;
		D2 = Dot22;
	}
	else 
	{                
		// get the closest points on the infinite lines
		N1 = (Dot12*Dot23 - Dot22*Dot13);
		N2 = (Dot11*Dot23 - Dot12*Dot13);

		if( N1 < 0.f ) 
		{
			// t1 < 0.f => the s==0 edge is visible
			N1 = 0.f;
			N2 = Dot23;
			D2 = Dot22;
		}
		else if( N1 > D1 ) 
		{
			// t1 > 1 => the t1==1 edge is visible
			N1 = D1;
			N2 = Dot23 + Dot12;
			D2 = Dot22;
		}
	}

	if( N2 < 0.f ) 
	{           
		// t2 < 0 => the t2==0 edge is visible
		N2 = 0.f;

		// recompute t1 for this edge
		if( -Dot13 < 0.f )
		{
			N1 = 0.f;
		}
		else if( -Dot13 > Dot11 )
		{
			N1 = D1;
		}
		else 
		{
			N1 = -Dot13;
			D1 = Dot11;
		}
	}
	else if( N2 > D2 ) 
	{      
		// t2 > 1 => the t2=1 edge is visible
		N2 = D2;

		// recompute t1 for this edge
		if( (-Dot13 + Dot12) < 0.f )
		{
			N1 = 0.f;
		}
		else if( (-Dot13 + Dot12) > Dot11 )
		{
			N1 = D1;
		}
		else 
		{
			N1 = (-Dot13 + Dot12);
			D1 = Dot11;
		}
	}

	// finally do the division to get the points' location
	const FLOAT T1 = (Abs(N1) < KINDA_SMALL_NUMBER ? 0.f : N1 / D1);
	const FLOAT T2 = (Abs(N2) < KINDA_SMALL_NUMBER ? 0.f : N2 / D2);

	// return the closest points
	OutP1 = A1 + T1 * S1;
	OutP2 = A2 + T2 * S2;
}


/** 
 * Find closest point on a Sphere to a Line.
 * When line intersects	Sphere, then closest point to LineOrigin is returned.
 * @param SphereOrigin		Origin of Sphere
 * @param SphereRadius		Radius of Sphere
 * @param LineOrigin		Origin of line
 * @param LineDir			Direction of line.
 * @param OutClosestPoint	Closest point on sphere to given line.
 */
void SphereDistToLine(FVector SphereOrigin, FLOAT SphereRadius, FVector LineOrigin, FVector LineDir, FVector& OutClosestPoint)
{
	const FLOAT A	= LineDir | LineDir;
	const FLOAT B	= 2.f * (LineDir | (LineOrigin - SphereOrigin));
	const FLOAT C	= (SphereOrigin|SphereOrigin) + (LineOrigin|LineOrigin) - 2.f *(SphereOrigin|LineOrigin) - Square(SphereRadius);
	const FLOAT D	= Square(B) - 4.f * A * C;

	if( D <= KINDA_SMALL_NUMBER )
	{
		// line is not intersecting sphere (or is tangent at one point if D == 0 )
		const FVector PointOnLine = LineOrigin + ( -B / 2.f * A ) * LineDir;
		OutClosestPoint = SphereOrigin + (PointOnLine - SphereOrigin).SafeNormal() * SphereRadius;
	}
	else
	{
		// Line intersecting sphere in 2 points. Pick closest to line origin.
		const FLOAT	E	= appSqrt(D);
		const FLOAT T1	= (-B + E) / (2.f * A);
		const FLOAT T2	= (-B - E) / (2.f * A);
		const FLOAT T	= Abs(T1) < Abs(T2) ? T1 : T2;

		OutClosestPoint	= LineOrigin + T * LineDir;
	}
}

/**
 * Calculates whether a Point is within a cone segment, and also what percentage within the cone
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
UBOOL GetDistanceWithinConeSegment(FVector Point, FVector ConeStartPoint, FVector ConeLine, FLOAT RadiusAtStart, FLOAT RadiusAtEnd, FLOAT &PercentageOut)
{
	// -- First we'll draw out a line from the ConeStartPoint down the ConeLine. We'll find the closest point on that line to Point.
	//    If we're outside the max distance, or behind the StartPoint, we bail out as that means we've no chance to be in the cone.

	FVector PointOnCone; // Stores the point on the cone's center line closest to our target point.

	FLOAT Distance = PointDistToLine(Point, ConeLine, ConeStartPoint, PointOnCone); // distance is how far from the viewline we are

	PercentageOut = 0.0; // start assuming we're outside cone until proven otherwise.

	if(ConeStartPoint.X != ConeLine.X) // Since points are colinear we just have to find a dimension they differ on, and check that our point is inside the segment.
	{
		if(!(PointOnCone.X < Max(ConeStartPoint.X,ConeStartPoint.X+ConeLine.X) && PointOnCone.X > Min(ConeStartPoint.X,ConeStartPoint.X+ConeLine.X)))
			return FALSE; // outside cone
	}
	else if(ConeStartPoint.Y != ConeLine.Y)
	{
		if(!(PointOnCone.Y < Max(ConeStartPoint.Y,ConeStartPoint.Y+ConeLine.Y) && PointOnCone.Y > Min(ConeStartPoint.Y,ConeStartPoint.Y+ConeLine.Y)))
			return FALSE; // outside cone
	}
	else if(ConeStartPoint.Z != ConeLine.Z)
	{
		if(!(PointOnCone.Z < Max(ConeStartPoint.Z,ConeStartPoint.Z+ConeLine.Z) && PointOnCone.Z > Min(ConeStartPoint.Z,ConeStartPoint.Z+ConeLine.Z)))
			return FALSE; // outside cone
	}
	else
	{
		debugf(NAME_Warning, TEXT("Gave two equal points for the Cone extremes in GetDistanceWithinConeSegment."));
		return FALSE;
	}

	FLOAT PercentAlongCone = (PointOnCone-ConeStartPoint).Size()/(ConeLine.Size()); // don't have to catch outside 0->1 due to above code (saves 2 sqrts if outside)
	FLOAT RadiusAtPoint = RadiusAtStart + (RadiusAtEnd * PercentAlongCone);

	if(Distance > RadiusAtPoint) // target is farther from the line than the radius at that distance)
		return FALSE;

	PercentageOut = (RadiusAtPoint-Distance)/RadiusAtPoint;

	return TRUE;
}

/**
 * Calculates the dotted distance of vector 'Direction' to coordinate system O(AxisX,AxisY,AxisZ).
 *
 * Orientation: (consider 'O' the first person view of the player, and 'Direction' a vector pointing to an enemy)
 * - positive azimuth means enemy is on the right of crosshair. (negative means left).
 * - positive elevation means enemy is on top of crosshair, negative means below.
 *
 * @Note: 'Azimuth' (.X) sign is changed to represent left/right and not front/behind. front/behind is the funtion's return value.
 *
 * @param	OutDotDist	.X = 'Direction' dot AxisX relative to plane (AxisX,AxisZ). (== Cos(Azimuth))
 *						.Y = 'Direction' dot AxisX relative to plane (AxisX,AxisY). (== Sin(Elevation))
 * @param	Direction	direction of target.
 * @param	AxisX		X component of reference system.
 * @param	AxisY		Y component of reference system.
 * @param	AxisZ		Z component of reference system.
 *
 * @return	true if 'Direction' is facing AxisX (Direction dot AxisX >= 0.f)
 */

UBOOL GetDotDistance
( 
	FVector2D	&OutDotDist, 
	FVector		&Direction, 
	FVector		&AxisX, 
	FVector		&AxisY, 
	FVector		&AxisZ 
)
{
	const FVector NormalDir = Direction.SafeNormal();

	// Find projected point (on AxisX and AxisY, remove AxisZ component)
	FVector NoZProjDir = ( NormalDir - (NormalDir | AxisZ) * AxisZ );
	NoZProjDir.SafeNormal();
	
	// Figure out on which Azimuth dot is on right or left.
	const FLOAT AzimuthSign = ( (NoZProjDir | AxisY) < 0.f ) ? -1.f : 1.f;

	OutDotDist.Y	= NormalDir | AxisZ;
	const FLOAT DirDotX	= NoZProjDir | AxisX;
	OutDotDist.X	= AzimuthSign * Abs(DirDotX);

	return (DirDotX >= 0.f );
}


/**
 * Calculates the angular distance of vector 'Direction' to coordinate system O(AxisX,AxisY,AxisZ).
 *
 * Orientation: (consider 'O' the first person view of the player, and 'Direction' a vector pointing to an enemy)
 * - positive azimuth means enemy is on the right of crosshair. (negative means left).
 * - positive elevation means enemy is on top of crosshair, negative means below.
 *
 * @param	OutAngularDist	.X = Azimuth angle (in radians) of 'Direction' vector compared to plane (AxisX,AxisZ).
 *							.Y = Elevation angle (in radians) of 'Direction' vector compared to plane (AxisX,AxisY).
 * @param	Direction		Direction of target.
 * @param	AxisX			X component of reference system.
 * @param	AxisY			Y component of reference system.
 * @param	AxisZ			Z component of reference system.
 *
 * @return	true if 'Direction' is facing AxisX (Direction dot AxisX >= 0.f)
 */

UBOOL GetAngularDistance
(
	FVector2D	&OutAngularDist, 
	FVector		&Direction, 
	FVector		&AxisX, 
	FVector		&AxisY, 
	FVector		&AxisZ 	
)
{
	FVector2D	DotDist;

	// Get Dotted distance
	const UBOOL bIsInFront = GetDotDistance( DotDist, Direction, AxisX, AxisY, AxisZ );
	GetAngularFromDotDist( OutAngularDist, DotDist );

	return bIsInFront;
}

/**
* Converts Dot distance to angular distance.
* @see	GetAngularDistance() and GetDotDistance().
*
* @param	OutAngDist	Angular distance in radians.
* @param	DotDist		Dot distance.
*/

void GetAngularFromDotDist( FVector2D &OutAngDist, FVector2D &DotDist )
{
	const FLOAT AzimuthSign = (DotDist.X < 0.f) ? -1.f : 1.f;
	DotDist.X = Abs(DotDist.X);

	// Convert to angles in Radian.
	// Because of mathematical imprecision, make sure dot values are within the [-1.f,1.f] range.
	OutAngDist.X = appAcos(DotDist.X) * AzimuthSign;
	OutAngDist.Y = appAsin(DotDist.Y);
}


FVector VInterpTo( const FVector& Current, const FVector& Target, FLOAT& DeltaTime, FLOAT InterpSpeed )
{
	// If no interp speed, jump to target value
	if( InterpSpeed <= 0.f )
	{
		return Target;
	}

	// Distance to reach
	const FVector Dist = Target - Current;

	// If distance is too small, just set the desired location
	if( Dist.SizeSquared() < KINDA_SMALL_NUMBER )
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FVector	DeltaMove = Dist * Clamp<FLOAT>(DeltaTime * InterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}

FRotator RInterpTo( const FRotator& Current, const FRotator& Target, FLOAT& DeltaTime, FLOAT InterpSpeed )
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if( DeltaTime == 0.f || Current == Target )
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if( InterpSpeed <= 0.f )
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FRotator DeltaMove = (Target - Current).GetNormalized() * Clamp<FLOAT>(DeltaTime * InterpSpeed, 0.f, 1.f);

	// If steps are too small, just return Target and consider we've reached our destination.
	if( DeltaMove.IsZero() )
	{
		return Target;
	}

	return (Current + DeltaMove).GetNormalized();
}


FLOAT FInterpTo( FLOAT Current, FLOAT Target, FLOAT DeltaTime, FLOAT InterpSpeed )
{
	// If no interp speed, jump to target value
	if( InterpSpeed == 0.f )
	{
		return Target;
	}

	// Distance to reach
	const FLOAT Dist = Target - Current;

	// If distance is too small, just set the desired location
	if( Square(Dist) < SMALL_NUMBER )
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FLOAT DeltaMove = Dist * Clamp<FLOAT>(DeltaTime * InterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}

/** Interpolate between A and B, applying an ease in/out function.  Exp controls the degree of the curve. */
FLOAT FInterpEaseInOut( FLOAT A, FLOAT B, FLOAT Alpha, FLOAT Exp )
{
	FLOAT ModifiedAlpha;

	if( Alpha < 0.5f )
	{
		ModifiedAlpha = 0.5f * appPow(2.f * Alpha, Exp);
	}
	else
	{
		ModifiedAlpha = 1.f - 0.5f * appPow(2.f * (1.f - Alpha), Exp);
	}

	return Lerp<FLOAT>(A, B, ModifiedAlpha);
}


/**
 * Clamps a tangent formed by the specified control point values
 */
FLOAT FClampFloatTangent( FLOAT PrevPointVal, FLOAT PrevTime, FLOAT CurPointVal, FLOAT CurTime, FLOAT NextPointVal, FLOAT NextTime )
{
	const FLOAT PrevToNextTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, NextTime - PrevTime );
	const FLOAT PrevToCurTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, CurTime - PrevTime );
	const FLOAT CurToNextTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, NextTime - CurTime );

	FLOAT OutTangentVal = 0.0f;

	const FLOAT PrevToNextHeightDiff = NextPointVal - PrevPointVal;
	const FLOAT PrevToCurHeightDiff = CurPointVal - PrevPointVal;
	const FLOAT CurToNextHeightDiff = NextPointVal - CurPointVal;

	// Check to see if the current point is crest
	if( ( PrevToCurHeightDiff >= 0.0f && CurToNextHeightDiff <= 0.0f ) ||
		( PrevToCurHeightDiff <= 0.0f && CurToNextHeightDiff >= 0.0f ) )
	{
		// Neighbor points are both both on the same side, so zero out the tangent
		OutTangentVal = 0.0f;
	}
	else
	{
		// The three points form a slope

		// Constants
		const FLOAT ClampThreshold = 0.333f;

		// Compute height deltas
		const FLOAT CurToNextTangent = CurToNextHeightDiff / CurToNextTimeDiff;
		const FLOAT PrevToCurTangent = PrevToCurHeightDiff / PrevToCurTimeDiff;
		const FLOAT PrevToNextTangent = PrevToNextHeightDiff / PrevToNextTimeDiff;

		// Default to not clamping
		const FLOAT UnclampedTangent = PrevToNextTangent;
		FLOAT ClampedTangent = UnclampedTangent;

		const FLOAT LowerClampThreshold = ClampThreshold;
		const FLOAT UpperClampThreshold = 1.0f - ClampThreshold;

		// @todo: Would we get better results using percentange of TIME instead of HEIGHT?
		const FLOAT CurHeightAlpha = PrevToCurHeightDiff / PrevToNextHeightDiff;

		if( PrevToNextHeightDiff > 0.0f )
		{
			if( CurHeightAlpha < LowerClampThreshold )
			{
				// 1.0 = maximum clamping (flat), 0.0 = minimal clamping (don't touch)
				const FLOAT ClampAlpha = 1.0f - CurHeightAlpha / ClampThreshold;
				const FLOAT LowerClamp = Lerp( PrevToNextTangent, PrevToCurTangent, ClampAlpha );
				ClampedTangent = Min( ClampedTangent, LowerClamp );
			}

			if( CurHeightAlpha > UpperClampThreshold )
			{
				// 1.0 = maximum clamping (flat), 0.0 = minimal clamping (don't touch)
				const FLOAT ClampAlpha = ( CurHeightAlpha - UpperClampThreshold ) / ClampThreshold;
				const FLOAT UpperClamp = Lerp( PrevToNextTangent, CurToNextTangent, ClampAlpha );
				ClampedTangent = Min( ClampedTangent, UpperClamp );
			}
		}
		else
		{

			if( CurHeightAlpha < LowerClampThreshold )
			{
				// 1.0 = maximum clamping (flat), 0.0 = minimal clamping (don't touch)
				const FLOAT ClampAlpha = 1.0f - CurHeightAlpha / ClampThreshold;
				const FLOAT LowerClamp = Lerp( PrevToNextTangent, PrevToCurTangent, ClampAlpha );
				ClampedTangent = Max( ClampedTangent, LowerClamp );
			}

			if( CurHeightAlpha > UpperClampThreshold )
			{
				// 1.0 = maximum clamping (flat), 0.0 = minimal clamping (don't touch)
				const FLOAT ClampAlpha = ( CurHeightAlpha - UpperClampThreshold ) / ClampThreshold;
				const FLOAT UpperClamp = Lerp( PrevToNextTangent, CurToNextTangent, ClampAlpha );
				ClampedTangent = Max( ClampedTangent, UpperClamp );
			}
		}

		OutTangentVal = ClampedTangent;
	}

	return OutTangentVal;
}
