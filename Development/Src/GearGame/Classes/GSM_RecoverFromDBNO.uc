
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_RecoverFromDBNO extends GSM_StumbleGetUp;

var		bool				bRevivedByKantus;
var()	GearPawn.BodyStance	BS_GetUp, BS_KantusRevive;

function bool CanChainMove(ESpecialMove NextMove)
{
	// allow everything to chain from recovery
	return TRUE;
}

static function INT PackSpecialMoveFlags(bool inbRevivedByKantus)
{
	return INT(inbRevivedByKantus);
}

/** Unpack Special Move flags used for replication */
function UnpackSpecialMoveFlags()
{
	bRevivedByKantus = bool(PawnOwner.SpecialMoveFlags & 1);
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// clear the was DBNO flag since we didn't die
	PawnOwner.bWasDBNO = FALSE;

	UnpackSpecialMoveFlags();
	
	if(bRevivedByKantus)
	{
		PawnOwner.DesiredKantusFadeVal = 1.0f;
		PawnOwner.Controller.DesiredRotation = rotator(PawnOwner.KantusReviver.Location - PawnOwner.Location);
		PawnOwner.DesiredRotation = PawnOwner.Controller.DesiredRotation;
		PawnOwner.SetRotation(PawnOwner.DesiredRotation);
		PawnOwner.Controller.SetRotation(PawnOwner.DesiredRotation);
		PawnOWner.Controller.bForceDesiredRotation = true;
		PawnOwner.Controller.SetFocalPoint(vect(0,0,0));

	}
	BS_Animation = bRevivedByKantus ? BS_KantusRevive : BS_GetUp;

	Super.SpecialMoveStarted(bForced,PrevMove);

	// note the time of revival
	PawnOwner.TimeOfRevival = PawnOwner.WorldInfo.TimeSeconds;

	// if alive and owning client
	if( PawnOwner.IsLocallyControlled() && PawnOwner.IsHumanControlled() )
	{
		PCOwner.StopPostProcessOverride(eGPP_BleedOut);
		// then kick out the dbno exit cue
		PawnOwner.PlaySound(SoundCue'Interface_Audio.Interface.DeathModeExit01Cue', TRUE);
	}

	// put out any fire or smoke
	PawnOwner.ExtinguishPawn();

	// make sure weapon is attached
	if( PawnOwner.MyGearWeapon != None )
	{
		PawnOwner.AttachWeapon();
		PawnOwner.MyGearWeapon.EndFire(PawnOwner.MyGearWeapon.CurrentFireMode);
	}

	// clear some extra flags jic
	PawnOwner.bWantsToMelee = FALSE;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local Rotator PawnRot;

	Super.SpecialMoveEnded(PrevMove,NextMove);
	if(bRevivedByKantus)
	{
		PawnOwner.DesiredKantusFadeVal = 0.0f;
		if (bRevivedByKantus)
		{
			if(PawnOwner.HealthRechargeDelay > 0.f)
			{
				PawnOwner.EnableHealthRecharge();
			}
			else
			{
				// assumes we were put at 15% of health by dorevival, so add back up to 100%.. 
				// (means if we took some damage durnig the interim we will still maintain that damage)
				PawnOwner.Health += pawnOwner.DefaultHealth * 0.85f;
			}
		}
		PawnOWner.Controller.bForceDesiredRotation = PawnOwner.Controller.Default.bForceDesiredRotation;

	}
	if (PCOwner != None && !PawnOwner.Weapon.HasAnyAmmo() && PCOwner.IsLocalPlayerController())
	{
		GearInventoryManager(PawnOwner.InvManager).AutoSwitchWeapon();;
	}

	// Clear all pitch when recovering
	PawnRot = PawnOwner.Rotation;
	PawnRot.Pitch = 0;
	PawnOwner.SetRotation(PawnRot);
}

defaultproperties
{
	BS_GetUp=(AnimName[BS_FullBody]="AR_Injured_Getup")
	BS_KantusRevive=(AnimName[BS_FullBody]="DBNO_Revive")
}