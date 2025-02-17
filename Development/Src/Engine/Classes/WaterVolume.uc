/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 Games should create placeable subclasses of WaterVolume for use in game levels.
 */
class WaterVolume extends PhysicsVolume
	notplaceable;

var() SoundCue  EntrySound;	//only if waterzone
var() SoundCue  ExitSound;		// only if waterzone
var() class<actor> EntryActor;	// e.g. a splash (only if water zone)
var() class<actor> ExitActor;	// e.g. a splash (only if water zone)
var() class<actor> PawnEntryActor; // when pawn center enters volume

simulated event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	Super.Touch(Other, OtherComp, HitLocation, HitNormal);

	if ( Other.CanSplash() )
		PlayEntrySplash(Other);
}

function PlayEntrySplash(Actor Other)
{
	if( EntrySound != None )
	{
		Other.PlaySound(EntrySound);
		if ( Other.Instigator != None )
			Other.MakeNoise(1.0);
	}
	if( EntryActor != None )
	{
		Spawn(EntryActor);
	}
}

event untouch(Actor Other)
{
	if ( Other.CanSplash() )
		PlayExitSplash(Other);
}

function PlayExitSplash(Actor Other)
{
	if ( ExitSound != None )
	{
		Other.PlaySound(ExitSound);
		if ( Other.Instigator != None )
			Other.MakeNoise(1.0);
	}
	if( ExitActor != None )
	{
		Spawn(ExitActor);
	}
}

defaultproperties
{
	Begin Object Name=BrushComponent0
		RBChannel=RBCC_Water
	End Object

	bWaterVolume=True
    FluidFriction=+00002.400000
}
