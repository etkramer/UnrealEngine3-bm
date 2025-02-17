/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AmbientShakeVolume extends Volume
	native
	dependson(GearTypes);

/** Ambient Screen Shake */
var()	ScreenShakeStruct	AmbientShake;

/** Is Ambient Shake enabled? */
var()	bool	bEnableShake;

/**	Handling Toggle event from Kismet. */
simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if( Action.InputLinks[0].bHasImpulse )
	{
		if( !bEnableShake )
		{
			bEnableShake = true;
		}
	}
	// Turn OFF
	else if( Action.InputLinks[1].bHasImpulse )
	{
		if( bEnableShake )
		{
			bEnableShake = false;
		}
	}
	// Toggle
	else if( Action.InputLinks[2].bHasImpulse )
	{
		bEnableShake = !bEnableShake;
	}
}

defaultproperties
{
	bCollideActors=true
	Begin Object Name=BrushComponent0
		CollideActors=true
		BlockActors=false
		BlockZeroExtent=false
		BlockNonZeroExtent=false
		BlockRigidBody=false
		AlwaysLoadOnClient=True
		AlwaysLoadOnServer=True
	End Object

	bColored=true
	BrushColor=(R=225,G=196,B=255,A=255)

	SupportedEvents.Empty
	SupportedEvents(0)=class'SeqEvent_Touch'
}
