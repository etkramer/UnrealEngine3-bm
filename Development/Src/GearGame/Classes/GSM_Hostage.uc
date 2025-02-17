
/**
 * Meat Shield: Hostage
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Hostage extends GSM_InteractionPawnFollower_Base;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

// 	// Hostages are considered dead (except MeatFlag), so switch to their Gore Skeleton.
// 	if( PawnOwner.CanBeSpecialMeleeAttacked() )
// 	{
// 		PawnOwner.CreateGoreSkeleton(PawnOwner.GoreSkeletalMesh, PawnOwner.GorePhysicsAsset);
// 	}

	// Set his health to default value.
	PawnOwner.HostageHealth = PawnOwner.HostageDefaultHealth;

	// As a hostage, PawnOwner doesn't bleed out anymore.
	// Blargh - why do we have to manually toggle these. Need to refactor this to just check for a flag in the HUD code.
	// It would be much easier to maintain.
	if( PCOwner != None && PCOwner.MyGearHUD != None )
	{
		// stop pp effect for bleed out
		PCOwner.StopPostProcessOverride(eGPP_BleedOut);
		PCOwner.MyGearHUD.ClearActionInfoByType(AT_StayingAlive);
		PCOwner.MyGearHUD.ClearActionInfoByType(AT_SuicideBomb);
	}

	// if pawn is on fire or smoldering, put him out
	PawnOwner.ExtinguishPawn();

	PawnOwner.InteractionPawn.StartBloodTrail( 'SpawnABloodTrail_MeatBag' );

	// If we've been taken hostage from stumbling and not DBNO, then force pawn to go into
	// DBNO state. Consider this to be an execution. One code path for all!
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		if( !PawnOwner.IsInState('Kidnapped') )
		{
			PawnOwner.GotoState('Kidnapped');
		}

		// if carrying a heavy weapon, drop it
		if( PawnOwner.IsCarryingAHeavyWeapon() )
		{
			PawnOwner.ThrowActiveWeapon();
		}

		// if carrying a shield, drop it
		if( PawnOwner.IsCarryingShield() )
		{
			PawnOwner.DropShield();
		}
	}

	// Detach weapon if it hasn't been already.
	if( PawnOwner.MyGearWeapon != None )
	{
		PawnOwner.MyGearWeapon.DetachWeapon();
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	if (NextMove == SM_ChainsawVictim && PawnOwner.Role == ROLE_Authority)
	{
		PawnOwner.InteractionPawn.SetTimer(0.1f,FALSE,'SavedByAMeatshieldCallback');
	}
	PawnOwner.InteractionPawn.StopBloodTrail( 'SpawnABloodTrail_MeatBag' );

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
}