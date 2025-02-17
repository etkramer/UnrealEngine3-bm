
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_ExposeDriver extends GSM_Brumak_BasePlaySingleAnim;

var GearPawn.BodyStance	BS_DriverAnimation, BS_DriverAnimation_B;
var INT					PlayCount;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted( bForced, PrevMove );

	// Count number of times this special move has been played
	PlayCount++;

	if( Brumak != None )
	{
		// Trigger kismet event that brumak is stumbling
//		Brumak.TriggerEventClass(class'SeqEvent_BrumakStumbled', Brumak);

		// Play Driver synched animation
		if( Brumak.Driver != None )
		{
			// Play pointing animation rarely
			if( PlayCount % 3 == 1 )
			{
				Brumak.Driver.BS_Play(BS_DriverAnimation, SpeedModifier, BlendInTime/SpeedModifier, BlendOutTime/SpeedModifier, FALSE, TRUE);
			}
			else
			{
				Brumak.Driver.BS_Play(BS_DriverAnimation_B, SpeedModifier, BlendInTime/SpeedModifier, BlendOutTime/SpeedModifier, FALSE, TRUE);
			}
		}
	}
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="Leg_Injure_expose")
	BS_DriverAnimation=(AnimName[BS_FullBody]="Rider_Roar_React")
	BS_DriverAnimation_B=(AnimName[BS_FullBody]="Rider_Roar_React_B")
	BlendInTime=0.45f
	BlendOutTime=0.67f

	NearbyPlayerSynchedCameraAnimName="Leg_Injure_expose"
	bCamShakeDampenWhenTargeting=TRUE

	bCheckForGlobalInterrupts=FALSE
	bLockPawnRotation=TRUE
}