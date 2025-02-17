/*=============================================================================
	UnAnimTree.h:	Animation tree element definitions and helper structs.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNANIMTREE_H__
#define __UNANIMTREE_H__

/**
 * Below this weight threshold, animations won't be blended in.
 */
#define ZERO_ANIMWEIGHT_THRESH (0.00001f)  

/**
* Engine stats
*/
enum EAnimStats
{
	/** Skeletal stats */
	STAT_AnimBlendTime = STAT_AnimFirstStat,
	STAT_SkelComposeTime,
	STAT_UpdateSkelPose,
	STAT_AnimTickTime,
	STAT_AnimSyncGroupTime,
	STAT_SkelControlTickTime,
	STAT_SkelComponentTickTime,
	STAT_GetAnimationPose,
	STAT_MirrorBoneAtoms,
	STAT_UpdateFaceFX,
	STAT_SkelControl,
	STAT_UpdateChildComponents,
	STAT_BlendInPhysics,
	STAT_MeshObjectUpdate,
	STAT_SkelCompUpdateTransform,
	STAT_UpdateRBBones,
	STAT_UpdateSkelMeshBounds
};

/** Controls compilation of per-node GetBoneAtom stats. */
#define ENABLE_GETBONEATOM_STATS 0

#if ENABLE_GETBONEATOM_STATS

/** One timing for calling GetBoneAtoms on an AnimNode. */
struct FAnimNodeTimeStat
{
	FName	NodeName;
	DOUBLE	NodeExclusiveTime;

	FAnimNodeTimeStat(FName InNodeName, DOUBLE InExcTime):
	NodeName(InNodeName),
		NodeExclusiveTime(InExcTime)
	{}
};

/** Array of timings for different AnimNodes within the tree. */
extern TArray<FAnimNodeTimeStat> BoneAtomBlendStats;

/** Used for timing how long GetBoneAtoms takes, excluding time taken by children nodes. */
class FScopedBlendTimer
{
public:
	const DOUBLE StartTime;
	DOUBLE TotalChildTime;
	FName NodeName;

	FScopedBlendTimer(FName InNodeName):
		StartTime( appSeconds() ),
		TotalChildTime(0.0),
		NodeName(InNodeName)
	{}

	~FScopedBlendTimer()
	{
		DOUBLE TimeTaken = appSeconds() - StartTime;
		TimeTaken -= TotalChildTime;
		BoneAtomBlendStats.AddItem( FAnimNodeTimeStat(NodeName, TimeTaken) );
	}
};

/** Used for timing how long a child takes to return its BoneAtoms. */
class FScopedChildTimer
{
public:
	const DOUBLE StartTime;
	FScopedBlendTimer* Timer;

	FScopedChildTimer(FScopedBlendTimer* InTimer):
		StartTime( appSeconds() ),
		Timer(InTimer)
	{}

	~FScopedChildTimer()
	{
		DOUBLE ChildTime = appSeconds() - StartTime;
		Timer->TotalChildTime += ChildTime;
	}
};

#define START_GETBONEATOM_TIMER	FScopedBlendTimer BoneAtomTimer( GetFName() );
#define EXCLUDE_CHILD_TIME		FScopedChildTimer ChildTimer( &BoneAtomTimer );

#else // ENABLE_GETBONEATOM_STATS

#define START_GETBONEATOM_TIMER
#define EXCLUDE_CHILD_TIME

#endif // ENABLE_GETBONEATOM_STATS

struct FBoneAtom
{
	FQuat	Rotation;
	FVector	Translation;
	FLOAT	Scale;

	static const FBoneAtom Identity;

	FBoneAtom() {};

	FBoneAtom(const FQuat& InRotation, const FVector& InTranslation, FLOAT InScale) : 
		Rotation(InRotation), 
		Translation(InTranslation), 
		Scale(InScale)
	{}

	/**
	 * Constructor for converting a Matrix into a bone atom. InMatrix should not contain any scaling info.
	 */
	FBoneAtom(const FMatrix& InMatrix)
		:	Rotation( FQuat(InMatrix) )
		,	Translation( InMatrix.GetOrigin() )
		,	Scale( 1.f )
	{}

	/**
	 * Does a debugf of the contents of this BoneAtom.
	 */
	void DebugPrint() const;

	/**
	 * Convert this Atom to a transformation matrix.
	 */
	void ToTransform(FMatrix& OutMatrix) const
	{
#if !FINAL_RELEASE && !CONSOLE
		// Make sure Rotation is normalized when we turn it into a matrix.
		check( Rotation.IsNormalized() );
#endif
		OutMatrix.M[3][0] = Translation.X;
		OutMatrix.M[3][1] = Translation.Y;
		OutMatrix.M[3][2] = Translation.Z;

		const FLOAT x2 = Rotation.X + Rotation.X;	
		const FLOAT y2 = Rotation.Y + Rotation.Y;  
		const FLOAT z2 = Rotation.Z + Rotation.Z;
		{
			const FLOAT xx2 = Rotation.X * x2;
			const FLOAT yy2 = Rotation.Y * y2;			
			const FLOAT zz2 = Rotation.Z * z2;

			OutMatrix.M[0][0] = (1.0f - (yy2 + zz2)) * Scale;	
			OutMatrix.M[1][1] = (1.0f - (xx2 + zz2)) * Scale;
			OutMatrix.M[2][2] = (1.0f - (xx2 + yy2)) * Scale;
		}
		{
			const FLOAT yz2 = Rotation.Y * z2;   
			const FLOAT wx2 = Rotation.W * x2;	

			OutMatrix.M[2][1] = (yz2 - wx2) * Scale;
			OutMatrix.M[1][2] = (yz2 + wx2) * Scale;
		}
		{
			const FLOAT xy2 = Rotation.X * y2;
			const FLOAT wz2 = Rotation.W * z2;

			OutMatrix.M[1][0] = (xy2 - wz2) * Scale;
			OutMatrix.M[0][1] = (xy2 + wz2) * Scale;
		}
		{
			const FLOAT xz2 = Rotation.X * z2;
			const FLOAT wy2 = Rotation.W * y2;   

			OutMatrix.M[2][0] = (xz2 + wy2) * Scale;
			OutMatrix.M[0][2] = (xz2 - wy2) * Scale;
		}

		OutMatrix.M[0][3] = 0.0f;
		OutMatrix.M[1][3] = 0.0f;
		OutMatrix.M[2][3] = 0.0f;
		OutMatrix.M[3][3] = 1.0f;
	}

	/** Set this atom to the weighted blend of the supplied two atoms. */
	FORCEINLINE void Blend(const FBoneAtom& Atom1, const FBoneAtom& Atom2, FLOAT Alpha)
	{
#if !FINAL_RELEASE && !CONSOLE
		// Check that all bone atoms coming from animation are normalized
		check( Atom1.Rotation.IsNormalized() );
		check( Atom2.Rotation.IsNormalized() );
#endif
		if( Alpha <= ZERO_ANIMWEIGHT_THRESH )
		{
			// if blend is all the way for child1, then just copy its bone atoms
			(*this) = Atom1;
		}
		else if( Alpha >= 1.f - ZERO_ANIMWEIGHT_THRESH )
		{
			// if blend is all the way for child2, then just copy its bone atoms
			(*this) = Atom2;
		}
		else
		{
			// Simple linear interpolation for translation and scale.
			Translation = Lerp(Atom1.Translation, Atom2.Translation, Alpha);
			Scale		= Lerp(Atom1.Scale, Atom2.Scale, Alpha);
			Rotation	= LerpQuat(Atom1.Rotation, Atom2.Rotation, Alpha);

			// ..and renormalize
			Rotation.Normalize();
		}
	}

	/**
	 * Quaternion addition is wrong here. This is just a special case for linear interpolation.
	 * Use only within blends!!
	 * Rotation part is NOT normalized!!
	 */
	FORCEINLINE FBoneAtom operator+(const FBoneAtom& Atom) const
	{
		return FBoneAtom(Rotation + Atom.Rotation, Translation + Atom.Translation, Scale + Atom.Scale);
	}

	/** 
	 * For quaternions, delta angles is done by multiplying the conjugate.
	 * Result is normalized.
	 */
	FORCEINLINE FBoneAtom operator-(const FBoneAtom& Atom) const
	{
		return FBoneAtom(Rotation * (-Atom.Rotation), Translation - Atom.Translation, Scale - Atom.Scale);
	}

	FORCEINLINE FBoneAtom& operator+=(const FBoneAtom& Atom)
	{
		Translation += Atom.Translation;

		Rotation.X += Atom.Rotation.X;
		Rotation.Y += Atom.Rotation.Y;
		Rotation.Z += Atom.Rotation.Z;
		Rotation.W += Atom.Rotation.W;

		Scale += Atom.Scale;

		return *this;
	}

	FORCEINLINE FBoneAtom& operator*=(FLOAT Mult)
	{
		Translation *= Mult;
		Rotation.X	*= Mult;
		Rotation.Y	*= Mult;
		Rotation.Z	*= Mult;
		Rotation.W	*= Mult;
		Scale		*= Mult;

		return *this;
	}

	FORCEINLINE FBoneAtom operator*(FLOAT Mult) const
	{
		return FBoneAtom(Rotation * Mult, Translation * Mult, Scale * Mult);
	}
};

typedef TArray< FBoneAtom,TMemStackAllocator<GMainThreadMemStack> >	FBoneAtomArray;

template <> struct TIsPODType<FBoneAtom> { enum { Value = true }; };

inline FLOAT UnWindNormalizedAimAngleGT(FLOAT Angle)
{
	while( Angle > 2.f )
	{
		Angle -= 4.f;
	}

	while( Angle < -2.f )
	{
		Angle += 4.f;
	}

	return Angle;
}

inline FLOAT UnWindNormalizedAimAngleGE(FLOAT Angle)
{
	while( Angle >= 2.f )
	{
		Angle -= 4.f;
	}

	while( Angle < -2.f )
	{
		Angle += 4.f;
	}

	return Angle;
}

#endif // __UNANIMTREE_H__
