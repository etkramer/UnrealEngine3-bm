/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_Ragdoll extends GameplayCam_Default
	config(Camera);


simulated function bool SetFocusPoint(Pawn ViewedPawn)
{
	local GearPawn GP;
	GP = GearPawn(ViewedPawn);
	if (GP != None) 
	{
		GameplayCam.SetFocusOnActor( GP, GP.PelvisBoneName, vect2d(10,10), vect2d(8,11),, TRUE,, TRUE );
		return TRUE;
	}
	
	return FALSE;
}

defaultproperties
{
	ViewOffset={(
		OffsetHigh=(X=-100,Y=0,Z=5),
		OffsetLow=(X=-100,Y=0,Z=5),
		OffsetMid=(X=-100,Y=0,Z=5),
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

	BlendTime=0.15f

	bDoPredictiveAvoidance=FALSE
	bValidateWorstLoc=FALSE

	WorstLocOffset=(X=0,Y=0,Z=64)
}
