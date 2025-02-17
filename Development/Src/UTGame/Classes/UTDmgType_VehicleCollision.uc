/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTDmgType_VehicleCollision extends UTDamageType
	abstract;

// no K impulse because this occurs during async tick, so we can add impulse to kactors
defaultproperties
{
	KillStatsName=EVENT_RANOVERKILLS
	DeathStatsName=EVENT_RANOVERDEATHS
	SuicideStatsName=SUICIDES_ENVIRONMENT
	KDamageImpulse=0
	KImpulseRadius=0.0
	DamageOverlayTime=0.0
	bLocationalHit=false
	bArmorStops=false
}
