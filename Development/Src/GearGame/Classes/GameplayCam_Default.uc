/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_Default extends GearGameplayCameraMode
	config(Camera)
	native(Camera);

cpptext
{
	/**
	 * Returns location and rotation, in world space, of the camera's basis point.  The camera will rotate
	 * around this point, offsets are applied from here, etc.
	 */
	virtual void GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot);
};

var() vector EvadePawnRelativeOffset;

/** Z adjustment to camera worst location if target pawn is in aiming stance */
var() private float WorstLocAimingZOffset;

var protected transient bool	bTemporaryOriginRotInterp;
var() protected const float		TemporaryOriginRotInterpSpeed;




/** Get Pawn's relative offset (from location based on pawn's rotation */
function vector GetPawnRelativeOffset(Pawn P)
{
	local vector	FinalOffset;
	local GearPawn	WP;

	// can be None if camera target is not a GearPawn
	WP = GearPawn(P);

	if (WP != None)
	{
		if( WP.IsEvading() )
		{
			FinalOffset = EvadePawnRelativeOffset;
		}
		else
		{
			FinalOffset = PawnRelativeOffset;
		}

		// if pawn is facing left, then mirror Y axis.
		if( WP.bIsMirrored )
		{
			FinalOffset.Y = -FinalOffset.Y;
		}
	}

	return FinalOffset;
}

simulated function vector GetCameraWorstCaseLoc(Pawn TargetPawn)
{
	local GearPawn	WP;
	local vector	WorstLocation;

	WorstLocation = super.GetCameraWorstCaseLoc(TargetPawn);

	WP = GearPawn(TargetPawn);
	if ( (WP != None) && WP.IsInAimingPose() )
	{
		WorstLocation.Z += WorstLocAimingZOffset;
	}
	// else super version just falls through

	return WorstLocation;
}

function OnBecomeActive(Pawn TargetPawn, GearGameplayCameraMode PrevMode)
{
	super.OnBecomeActive(TargetPawn, PrevMode);

	// limiting to grenade modes for now, but might work ok for any modes?
	if ( ( (GameplayCam_CoverTargetingGrenade(PrevMode) != None) || (GameplayCam_TargetingGrenade(PrevMode) != None) ) &&
		 PrevMode.bInterpRotation
		) 
	{
		bTemporaryOriginRotInterp = TRUE;
	}
}

defaultproperties
{
	TemporaryOriginRotInterpSpeed=12.f

	WorstLocOffset=(X=-8,Y=1,Z=95)
	WorstLocAimingZOffset=-10
	bValidateWorstLoc=FALSE

	EvadePawnRelativeOffset=(Z=-28)

	ViewOffset={(
		OffsetHigh=(X=-128,Y=56,Z=40),
		OffsetLow=(X=-160,Y=48,Z=56),
		OffsetMid=(X=-160,Y=48,Z=16),
		)}
	ViewOffset_ViewportAdjustments(CVT_16to9_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-12),
		OffsetLow=(X=0,Y=0,Z=-12),
		OffsetMid=(X=0,Y=0,Z=-12),
		)}
	ViewOffset_ViewportAdjustments(CVT_16to9_VertSplit)={(
		OffsetHigh=(X=0,Y=-20,Z=0),
		OffsetLow=(X=0,Y=-20,Z=0),
		OffsetMid=(X=0,Y=-20,Z=0),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_Full)={(
		OffsetHigh=(X=0,Y=0,Z=17),
		OffsetLow=(X=0,Y=0,Z=17),
		OffsetMid=(X=0,Y=0,Z=17),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-15),
		OffsetLow=(X=0,Y=0,Z=-15),
		OffsetMid=(X=0,Y=0,Z=-15),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_VertSplit)={(
		OffsetHigh=(X=0,Y=0,Z=0),
		OffsetLow=(X=0,Y=0,Z=0),
		OffsetMid=(X=0,Y=0,Z=0),
		)}

	StrafeLeftAdjustment=(X=0,Y=-15,Z=0)
	StrafeRightAdjustment=(X=0,Y=15,Z=0)
    StrafeOffsetScalingThreshold=200

	RunFwdAdjustment=(X=20,Y=0,Z=0)
	RunBackAdjustment=(X=-30,Y=0,Z=0)
	RunOffsetScalingThreshold=200

	BlendTime=0.25
}

