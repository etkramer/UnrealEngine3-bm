
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAI_Boomer_Gatling extends GearAI_Boomer;

function float GetFireDelay();
function float AboutToFireFromOpen()
{
	`AIlog(GetFuncName()@self);
	MyGearPawn.TelegraphAttack();
	return 1.4f;
}

event vector GetAimLocation(vector StartLoc, optional bool bActuallyFiring, optional Actor AimTarget )
{
	local Vector AimLoc;
	local Vector TestLocation;
	local CoverInfo EnemyCover;
	local GearPawn	E;

	if( AimTarget == None )
	{
		AimTarget = FireTarget;
	}

	AimLoc = Super.GetAimLocation(StartLoc, bActuallyFiring, AimTarget);

	// If enemy is not in cover
	E = GearPawn(AimTarget);
	if( E != None )
	{
		EnemyCover = GetEnemyCover( E );
		if( EnemyCover.Link == None )
		{
			// shoot higher so it doesn't look like I'm shooting at his feet
			TestLocation    = AimLoc;
			TestLocation.Z += E.GetCollisionHeight() * 0.5;

			if( CanSeeByPoints( StartLoc, TestLocation, Rotator(TestLocation - StartLoc) ) )
			{
				AimLoc = TestLocation;
			}
		}
	}

	return AimLoc;
}
defaultproperties
{
	DefaultCommand=class'AICmd_Base_Boomer_Gatling'
	bAimAtFeet=FALSE
}
