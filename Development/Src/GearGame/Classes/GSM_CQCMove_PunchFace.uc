
/**
 * CQC Move (Close Quarter Combat) PunchFace
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_CQCMove_PunchFace extends GSM_CQC_Killer_Base;

/** Should we wait for the weapon to be holstered before starting the execution. */
var bool	bWaitForWeaponHolstered;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// Reset variables
	bWaitForWeaponHolstered = FALSE;

	Super.SpecialMoveStarted(bForced, PrevMove);
}

function StartInteraction()
{
	// Set execution damage type.
	ExecutionDamageType = PawnOwner.MyGearWeapon.CQC_Long_DamageType;

	// See if Killer should Holster his weapon.
	bWaitForWeaponHolstered = PawnOwner.MyGearWeapon.CQC_Long_bHolsterWeapon && PawnOwner.HolsterWeapon();

	Super.StartInteraction();
}

function PlaceOnMarkers()
{
	if( IsReadyToPlayExecution() )
	{
		Super.PlaceOnMarkers();
	}
}

function WeaponTemporarilyHolstered()
{
	bWaitForWeaponHolstered = FALSE;
	PlayExecution();
	PlaceOnMarkers();
}

function bool IsReadyToPlayExecution()
{
	return !bWaitForWeaponHolstered;
}

function PlayExecution()
{
	if( !IsReadyToPlayExecution() )
	{
		return;
	}

	// Play execution Camera Animation.
	if( PawnOwner.MyGearWeapon.CQC_Long_CameraAnims.Length > 0 )
	{
		PlayExecutionCameraAnim(PawnOwner.MyGearWeapon.CQC_Long_CameraAnims);
	}

	// Play Face FX Emotion
	if( !PawnOwner.IsActorPlayingFaceFXAnim() )
	{
		PawnOwner.PlayActorFaceFXAnim(None, "Emotions", "Strained", None);
	}

	PawnOwner.SoundGroup.PlayEffort(PawnOwner, PawnOwner.MyGearWeapon.CQC_Long_EffortID, true);

	// Setup the timer to execute
	VictimDeathTime = PawnOwner.MyGearWeapon.CQC_Long_VictimDeathTime;

	// Setup animation, parent function will play it
	BS_KillerAnim.AnimName[BS_FullBody] = PawnOwner.MyGearWeapon.CQC_Long_KillerAnim;
	BS_VictimAnim.AnimName[BS_FullBody] = PawnOwner.MyGearWeapon.CQC_Long_VictimAnim;

	Super.PlayExecution();
}

/** Separate function, so other executions can implement their variations. */
function SetVictimRotation()
{
	// Override rotation.
	VictimDesiredYaw		= NormalizeRotAxis(Rotator(-DirToVictim).Yaw + 16384);
	// Get parameters from Weapon.
	VictimRotStartTime		= PawnOwner.MyGearWeapon.CQC_Long_VictimRotStartTime;
	VictimRotInterpSpeed	= PawnOwner.MyGearWeapon.CQC_Long_VictimRotInterpSpeed;

	Super.SetVictimRotation();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Unholster weapon after execution finishes.
	if( PawnOwner.IsWeaponHolstered() )
	{
		PawnOwner.UnHolsterWeapon();
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	ExecutionDamageType=class'GDT_Execution_PunchFace'
	MarkerRelOffset=(X=84.57,Y=27.19,Z=0.0)
}
