class InterpTrackMove extends InterpTrack
	native(Interpolation);

/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Track containing data for moving an actor around over time.
 * There is no UpdateTrack function. In the game, its the PHYS_Interpolating physics mode which 
 * updates the position based on the interp track.
 */

cpptext
{
	// UObject interface
	virtual void PostLoad();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PostEditImport();

	// InterpTrack interface
	virtual INT GetNumKeyframes();
	virtual void GetTimeRange(FLOAT& StartTime, FLOAT& EndTime);
	virtual FLOAT GetKeyframeTime(INT KeyIndex);
	virtual INT AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode);
	virtual void UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst);
	virtual INT SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder=true);
	virtual void RemoveKeyframe(INT KeyIndex);
	virtual INT DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime);
	virtual UBOOL GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition);

	virtual void ConditionalPreviewUpdateTrack(FLOAT NewPosition, class UInterpTrackInst* TrInst);
	virtual void PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst);

	virtual class UMaterial* GetTrackIcon();
	virtual FColor GetKeyframeColor(INT KeyIndex);
	virtual void DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params );
	virtual void Render3DTrack(UInterpTrackInst* TrInst, const FSceneView* View, FPrimitiveDrawInterface* PDI, INT TrackIndex, const FColor& TrackColor, TArray<class FInterpEdSelKey>& SelectedKeys);
	virtual void SetTrackToSensibleDefault();

	// FCurveEdInterface interface
	virtual INT		GetNumKeys();
	virtual INT		GetNumSubCurves();
	virtual FLOAT	GetKeyIn(INT KeyIndex);
	virtual FLOAT	GetKeyOut(INT SubIndex, INT KeyIndex);
	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn);
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);
	virtual FColor	GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor);
	virtual BYTE	GetKeyInterpMode(INT KeyIndex);
	virtual void	GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent);
	virtual FLOAT	EvalSub(INT SubIndex, FLOAT InVal);

	virtual INT		CreateNewKey(FLOAT KeyIn);
	virtual void	DeleteKey(INT KeyIndex);

	virtual INT		SetKeyIn(INT KeyIndex, FLOAT NewInVal);
	virtual void	SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal);
	virtual void	SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode);
	virtual void	SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent);

	/** Returns TRUE if this curve uses legacy tangent/interp algorithms and may be 'upgraded' */
	virtual UBOOL	UsingLegacyInterpMethod() const;

	/** 'Upgrades' this curve to use the latest tangent/interp algorithms (usually, will 'bake' key tangents.) */
	virtual void	UpgradeInterpMethod();


	// InterpTrackMove interface
	virtual FName GetLookupKeyGroupName(INT KeyIndex);
	virtual void SetLookupKeyGroupName(INT KeyIndex, const FName &NewGroupName);
	virtual void ClearLookupKeyGroupName(INT KeyIndex);

	/**
	 * Replacement for the PosTrack eval function that uses GetKeyframePosition.  This is so we can replace keyframes that get their information from other tracks.
	 *
	 * @param TrInst	TrackInst to use for looking up groups.
	 * @param Time		Time to evaluate position at.
	 * @return			Final position at the specified time.
	 */
	FVector EvalPositionAtTime(UInterpTrackInst* TrInst, FLOAT Time);

	/**
	 * Replacement for the EulerTrack eval function that uses GetKeyframeRotation.  This is so we can replace keyframes that get their information from other tracks.
	 *
	 * @param TrInst	TrackInst to use for looking up groups.
	 * @param Time		Time to evaluate rotation at.
	 * @return			Final rotation at the specified time.
	 */
	FVector EvalRotationAtTime(UInterpTrackInst* TrInst, FLOAT Time);

	/**
	 * Gets the position of a keyframe given its key index.  Also optionally retrieves the Arrive and Leave tangents for the key.
	 * This function respects the LookupTrack.
	 *
	 * @param TrInst			TrackInst to use for lookup track positions.
	 * @param KeyIndex			Index of the keyframe to get the position of.
	 * @param OutTime           Final time of the keyframe.
	 * @param OutPos			Final position of the keyframe.
	 * @param OutArriveTangent	Pointer to a vector to store the arrive tangent in, can be NULL.
	 * @param OutLeaveTangent	Pointer to a vector to store the leave tangent in, can be NULL.
	 */
	void GetKeyframePosition(UInterpTrackInst* TrInst, INT KeyIndex, FLOAT& OutTime, FVector &OutPos, FVector *OutArriveTangent, FVector *OutLeaveTangent);

	/**
	 * Gets the rotation of a keyframe given its key index.  Also optionally retrieves the Arrive and Leave tangents for the key.
	 * This function respects the LookupTrack.
	 *
	 * @param TrInst			TrackInst to use for lookup track rotations.
	 * @param KeyIndex			Index of the keyframe to get the rotation of.
	 * @param OutTime           Final time of the keyframe.
	 * @param OutRot			Final rotation of the keyframe.
	 * @param OutArriveTangent	Pointer to a vector to store the arrive tangent in, can be NULL.
	 * @param OutLeaveTangent	Pointer to a vector to store the leave tangent in, can be NULL.
	 */
	void GetKeyframeRotation(UInterpTrackInst* TrInst, INT KeyIndex, FLOAT& OutTime, FVector &OutRot, FVector *OutArriveTangent, FVector *OutLeaveTangent);

    /**
     * Computes the world space coordinates for a key; handles keys that use IMF_RelativeToInitial, basing, etc.
     *
     * @param MoveTrackInst		An instance of this movement track
     * @param RelativeSpacePos	Key position value from curve
     * @param RelativeSpaceRot	Key rotation value from curve
     * @param OutPos			Output world space position
     * @param OutRot			Output world space rotation
     */
    void ComputeWorldSpaceKeyTransform( UInterpTrackInstMove* MoveTrackInst,
                                        const FVector& RelativeSpacePos,
								        const FRotator& RelativeSpaceRot,
								        FVector& OutPos,
                                        FRotator& OutRot );
													      
	virtual void GetKeyTransformAtTime(UInterpTrackInst* TrInst, FLOAT Time, FVector& OutPos, FRotator& OutRot);
	virtual void GetLocationAtTime(UInterpTrackInst* TrInst, FLOAT Time, FVector& OutPos, FRotator& OutRot);
	virtual FMatrix GetMoveRefFrame(UInterpTrackInstMove* MoveTrackInst);

	INT CalcSubIndex(UBOOL bPos, INT InIndex) const;
}

/** Actual position keyframe data. */
var		InterpCurveVector	PosTrack;

/** Actual rotation keyframe data, stored as Euler angles in degrees, for easy editing on curve. */
var		InterpCurveVector	EulerTrack;

/** 
 * Array of group names to retrieve position and rotation data from instead of using the datastored in the keyframe. 
 * A value of NAME_None means to use the PosTrack and EulerTrack data for the keyframe.
 * There needs to be the same amount of elements in this array as there are keyframes. 
 */
struct native InterpLookupPoint
{
	var name	GroupName;
	var float	Time;
};

struct native InterpLookupTrack
{
	structcpptext
	{
		/** Add a new keypoint to the LookupTrack.  Returns the index of the new key.*/
		INT AddPoint( const FLOAT InTime, FName &InGroupName )
		{
			INT PointIdx=0; 
			
			for( PointIdx=0; PointIdx<Points.Num() && Points(PointIdx).Time < InTime; PointIdx++);
			
			Points.Insert(PointIdx);
			Points(PointIdx).Time = InTime;
			Points(PointIdx).GroupName = InGroupName;

			return PointIdx;
		}

		/** Move a keypoint to a new In value. This may change the index of the keypoint, so the new key index is returned. */
		INT MovePoint( INT PointIndex, FLOAT NewTime )
		{
			if( PointIndex < 0 || PointIndex >= Points.Num() )
			{
				return PointIndex;
			}

			FName GroupName = Points(PointIndex).GroupName;

			Points.Remove(PointIndex);

			const INT NewPointIndex = AddPoint( NewTime, GroupName );

			return NewPointIndex;
		}
	}

	var array<InterpLookupPoint>	Points;
};

var		InterpLookupTrack	LookupTrack;

/** When using IMR_LookAtGroup, specifies the Group which this track should always point its actor at. */
var()	name				LookAtGroupName;

/** Controls the tightness of the curve for the translation path. */
var()	float				LinCurveTension;

/** Controls the tightness of the curve for the rotation path. */
var()	float				AngCurveTension;

/** 
 *	Use a Quaternion linear interpolation between keys. 
 *	This is robust and will find the 'shortest' distance between keys, but does not support ease in/out.
 */
var()	bool				bUseQuatInterpolation;

/** In the editor, show a small arrow at each keyframe indicating the rotation at that key. */
var()	bool				bShowArrowAtKeys;

/** Disable previewing of this track - will always position Actor at Time=0.0. Useful when keyframing an object relative to this group. */
var()	bool				bDisableMovement;

/** If false, when this track is displayed on the Curve Editor in Matinee, do not show the Translation tracks. */
var()	bool				bShowTranslationOnCurveEd;

/** If false, when this track is displayed on the Curve Editor in Matinee, do not show the Rotation tracks. */
var()	bool				bShowRotationOnCurveEd;

/** If true, 3D representation of this track in the 3D viewport is disabled. */
var()	bool				bHide3DTrack;

enum EInterpTrackMoveFrame
{
	/** Track should be fixed relative to the world. */
	IMF_World,

	/** Track should move relative to the initial position of the actor when the interp sequence was started. */
	IMF_RelativeToInitial
};

/** Indicates what the movement track should be relative to. */
var()	editconst EInterpTrackMoveFrame	MoveFrame;


enum EInterpTrackMoveRotMode
{
	/** Should take orientation from the . */
	IMR_Keyframed,

	/** Point the X-Axis of the controlled Actor at the group specified by LookAtGroupName. */
	IMR_LookAtGroup

	/** Should look along the direction of the translation path, with Z always up. */
	// IMR_LookAlongPath // TODO!
};

var()	EInterpTrackMoveRotMode RotMode;

defaultproperties
{
	TrackInstClass=class'Engine.InterpTrackInstMove'

	bOnePerGroup=true

	TrackTitle="Movement"

	LinCurveTension=0.0
	AngCurveTension=0.0

	MoveFrame=IMF_World
	RotMode=IMR_Keyframed

	bShowTranslationOnCurveEd=true
	bShowRotationOnCurveEd=false
}
