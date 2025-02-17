
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustSireBase extends GearPawn_LocustBase
	abstract;

var float StartAttackSpeed;

/** Indicates that rightarm has fallen off - also used to replicate this fact. */
var repnotify bool bLostRightArm;
/** Indicates that left arm has fallen off - also used to replicate this fact. */
var repnotify bool bLostLeftArm;

// scale of the spawner we were spawned from (so we can scale up animation and such to clear the tube on the way out)
var repnotify float SpawnerDrawScale;

replication
{
	if(Role == ROLE_Authority)
		bLostLeftArm, bLostRightArm, SpawnerDrawScale;
}

// sound stubs
function PlayChargeSound();
function PlayAttackSound();

/** Do not force bAnimRotationOnly on those guys. */
simulated function AnimSetsListUpdated();

/** Timer function called to see if an idle break animation should be played */
function ShouldPlayIdleBreakAnim();
simulated function bool PlayIdleBreakAnimation(byte PickedIndex) { return FALSE; }

simulated function ReplicatedEvent(name VarName)
{
	if(VarName == 'SpawnerDrawScale')
	{
		if(IsDoingSpecialMove(GSM_Sire_TankFall))
		{
			GSM_Sire_TankFall(SpecialMoves[GSM_Sire_TankFall]).AdjustForSpawnTubeDrawScale(SpawnerDrawScale);
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}


defaultproperties
{
	bCanDBNO=false
	//bRespondToExplosions=false

	SpecialMoveClasses(SM_FullBodyHitReaction)=None

	bSuppressNoticedGUDSEvents=TRUE
}