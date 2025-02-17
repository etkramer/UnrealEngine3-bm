
/**
 * Meat Shield: Execution
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Kidnapper_Execution extends GSM_Kidnapper;

var BodyStance BS_KidnapperAnim, BS_HostageAnim;

/** The effect to play from head socket location on the executee **/
var ParticleSystem PS_BloodSprayFromMouth;


/**
 * Begins the interaction.
 */
function StartInteraction()
{
	local vector Loc;
	local rotator Rot;
	local Emitter AnEmitter;
	local GearPC PC;

	// Play animation on both characters.
	PawnOwner.BS_Play(BS_KidnapperAnim, 1.f, 0.33f, 0.33f);
	PawnOwner.BS_SetAnimEndNotify(BS_KidnapperAnim, TRUE);

	Follower.BS_Play(BS_HostageAnim, 1.f, 0.33f, -1.f);

	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_MeatbagExecutionEffort, true);

	if( PawnOwner.WorldInfo.GRI.ShouldShowGore() )
	{
		if( GearGRI(Follower.WorldInfo.GRI).IsEffectRelevant( PawnOwner, PawnOwner.Location, 1024.0f, FALSE, GearGRI(Follower.WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
		{
			// play a blood spewing from the mouth of the executee
			if( Follower.Mesh.GetSocketWorldLocationAndRotation( 'Head', Loc, Rot ) == TRUE )
			{
				AnEmitter = GearGRI(Follower.WorldInfo.GRI).GOP.GetImpactEmitter( PS_BloodSprayFromMouth, Loc, Rot );
				AnEmitter.ParticleSystemComponent.ActivateSystem();
			}
		}
	}

	// Make sure only the kidnapper kills the victim now.
	Follower.SpecialMoves[Follower.SpecialMove].bOnlyInteractionPawnCanDamageMe = TRUE;
	
	// Remove IK when doing this animation
	LockIKStatus(FALSE);

	// Have local players check for their execution achievement
	PC = GearPC(PawnOwner.Controller);
	if (PC != none && PC.IsLocalPlayerController() && PC.ProfileSettings != none)
	{
		PC.ProfileSettings.UpdateExecutionProgression(class'GDT_NeckBreak', PC);
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Restore IK when animation is done blending out.
	PawnOwner.SetTimer(0.33f, FALSE, 'RestoreIKStatus', Self);
}


function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	PawnOwner.EndSpecialMove();
}

defaultproperties
{
	KidnapperDamageType=class'GDT_NeckBreak'
	BS_KidnapperAnim=(AnimName[BS_FullBody]="Kidnapper_Execute")
	BS_HostageAnim=(AnimName[BS_FullBody]="Hostage_Execute")

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableLook=TRUE

	PS_BloodSprayFromMouth=ParticleSystem'Effects_Gameplay.Blood.P_Blood_Head_Necksnap_Bloodspray'
}
