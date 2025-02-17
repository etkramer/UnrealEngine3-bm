
/**
 * Deploying Shield, crouched.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_DeployShield extends GearSpecialMove;

var() const protected SoundCue				ShieldDeploySound;
var() const private float					DeploySoundDelay;

protected function bool InternalCanDoSpecialMove()
{
	if (PawnOwner.IsDoingSpecialMove(SM_Engage_Start) || PawnOwner.IsDoingSpecialMove(SM_Engage_End) || PawnOwner.IsDoingSpecialMove(SM_Engage_Loop) || PawnOwner.IsDoingSpecialMove(SM_Engage_Idle))
	{
		return FALSE;
	}
	return Super.InternalCanDoSpecialMove();
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	PawnOwner.SetTimer( DeploySoundDelay, FALSE, nameof(self.PlayDeploySound), self );

	if( PawnOwner.EquippedShield != None )
	{
		PawnOwner.EquippedShield.Expand();
	}

	if( PCOwner != None )
	{	
		PCOwner.TargetHeavyWeaponBaseRotationYaw = PawnOwner.Rotation.Yaw;
	}

	Super.SpecialMoveStarted(bForced,PrevMove);
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	if( PawnOwner.EquippedShield != None && NextMove != SM_PlantShield )
	{
		PawnOwner.EquippedShield.Retract();
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

simulated private function PlayDeploySound()
{
	PawnOwner.PlaySound(ShieldDeploySound);
}

defaultproperties
{
	ShieldDeploySound=SoundCue'Foley_Footsteps.FootSteps.Footsteps_Locust_MetalSolidLand_Cue'
	DeploySoundDelay=0.5f

	bBreakFromCover=FALSE
	bCanFireWeapon=TRUE
	bCameraFocusOnPawn=FALSE
	bDisableLook=FALSE
}
