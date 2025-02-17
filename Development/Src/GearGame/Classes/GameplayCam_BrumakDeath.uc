/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_BrumakDeath extends GameplayCam_BrumakDriver
	native(Camera)
	config(Camera);

cpptext
{
	/** Returns View relative offsets */
	virtual void GetBaseViewOffsets(class APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High);
}


simulated function bool SetFocusPoint(Pawn ViewedPawn)
{
	local GearPawn_LocustBrumakBase Brumak;

	Brumak = GearPawn_LocustBrumakBase(ViewedPawn);
	if ( (Brumak != None) && Brumak.Health <= 0 )
	{
		// when centaur is dead, camera will freeze in position and use a focus point to follow it
		GameplayCam.SetFocusOnActor( Brumak, '', vect2d(10,10), vect2d(8,11),, TRUE,, TRUE );
		return TRUE;
	}

	return FALSE;
}

defaultproperties
{
	BlendTime=0.25

	// defaults, in case the vehicle's data doesn't work out
	ViewOffset={(
		OffsetHigh=(X=-1200,Y=0,Z=0),
		OffsetLow=(X=-1200,Y=0,Z=500),
		OffsetMid=(X=-1000,Y=0,Z=1000),
	)}
}
