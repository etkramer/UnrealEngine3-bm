/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_MeleeDoor_Player extends GSM_Brumak_MeleeAttack;

var Trigger_ChainsawInteraction Trig;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearPawn_LocustBrumakPlayerBase PB;

	Super.SpecialMoveStarted( bForced, PrevMove );

	PB = GearPawn_LocustBrumakPlayerBase(Brumak);
	if( PB != None )
	{
		PB.LeftGunAimController.SetLookAtAlpha( 0.f, 0.1f );
		PB.RightGunAimController.SetLookAtAlpha( 0.f, 0.1f );

		if( PB.HumanGunner == None )
		{
			PB.MainGunAimController1.SetLookAtAlpha( 0.f, 0.1f );
			PB.MainGunAimController2.SetLookAtAlpha( 0.f, 0.1f );
		}

		if( PB.IsLocallyControlled() )
		{
			PB.PlayShoulderMeleeCameraAnim();
		}

		foreach PawnOwner.TouchingActors( class'Trigger_ChainsawInteraction', Trig )
		{
			Trig.TriggerEventClass(class'SeqEvt_ChainsawInteraction',PawnOwner,0,FALSE);
			Trig.TriggerBeginChainsaw( PawnOwner );
			break;
		}
	}
}

function SpecialMoveEnded( ESpecialMove PrevMove, ESpecialMove NextMove )
{
	Super.SpecialMoveEnded( PrevMove, NextMove );

	Trig.TriggerEventClass(class'SeqEvt_ChainsawInteraction',PawnOwner,1,FALSE);
	Trig.TriggerEndChainsaw( PawnOwner );
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="smash_melee")
	BlendInTime=0.1f
	BlendOutTime=0.1f

	NearbyPlayerSynchedCameraAnimName=""

	bCheckForGlobalInterrupts=FALSE
}