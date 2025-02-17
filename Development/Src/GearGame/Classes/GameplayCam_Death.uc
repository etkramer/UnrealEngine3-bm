/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_Death extends GameplayCam_Default
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


/**
* Returns the "worst case" camera location for this camera mode.
* This is the position that the camera ray is shot from, so it should be
* a guaranteed safe place to put the camera.
*/
simulated event vector GetCameraWorstCaseLoc(Pawn TargetPawn)
{
	local GearPawn WP;

	WP = GearPawn(TargetPawn);

	if ( (WP != None) && WP.bIsGore )
	{
		return WP.LocationWhenKilled;
	}
	else
	{
		return TargetPawn.Location;
	}
}

defaultproperties
{
	ViewOffset={(
		OffsetHigh=(X=-100,Y=0,Z=125),
		OffsetLow=(X=-100,Y=0,Z=125),
		OffsetMid=(X=-100,Y=0,Z=125),
		)}
	ViewOffset_ViewportAdjustments(CVT_16to9_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-40),
		OffsetLow=(X=0,Y=0,Z=-40),
		OffsetMid=(X=0,Y=0,Z=-40),
		)}
	ViewOffset_ViewportAdjustments(CVT_16to9_VertSplit)={(
		OffsetHigh=(X=0,Y=0,Z=0),
		OffsetLow=(X=0,Y=0,Z=0),
		OffsetMid=(X=0,Y=0,Z=0),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_Full)={(
		OffsetHigh=(X=20,Y=0,Z=15),
		OffsetLow=(X=20,Y=0,Z=15),
		OffsetMid=(X=20,Y=0,Z=15),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-40),
		OffsetLow=(X=0,Y=0,Z=-40),
		OffsetMid=(X=0,Y=0,Z=-40),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_VertSplit)={(
		OffsetHigh=(X=0,Y=0,Z=0),
		OffsetLow=(X=0,Y=0,Z=0),
		OffsetMid=(X=0,Y=0,Z=0),
		)}

	BlendTime=0.5f

	bDoPredictiveAvoidance=FALSE
	bValidateWorstLoc=FALSE

	WorstLocOffset=(X=0,Y=0,Z=64)
}
