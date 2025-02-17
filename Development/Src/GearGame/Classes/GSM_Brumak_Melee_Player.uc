/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_Melee_Player extends GSM_Brumak_MeleeAttack;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearPawn_LocustBrumakPlayerBase PB;

	Super.SpecialMoveStarted( bForced, PrevMove );

	PB = GearPawn_LocustBrumakPlayerBase(Brumak);
	if( PB != None )
	{
		PB.LeftGunAimController.SetLookAtAlpha( 0.f, 0.1f );
		PB.RightGunAimController.SetLookAtAlpha( 0.f, 0.1f );
	}	
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="punch_melee")
	BlendInTime=0.1f
	BlendOutTime=0.1f

	NearbyPlayerSynchedCameraAnimName=""

	bCheckForGlobalInterrupts=FALSE
}