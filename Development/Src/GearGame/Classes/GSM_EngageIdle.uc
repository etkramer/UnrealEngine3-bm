
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_EngageIdle extends GSM_Engage;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local BodyStance IdleStance;

	Super.SpecialMoveStarted(bForced,PrevMove);

	if( PawnOwner.EngageTrigger != None && PawnOwner.EngageTrigger.ENGAGE_AnimName_Idle != '' )
	{
		if( PawnOwner.EngageTrigger.ENGAGE_bPlayAnimationForHeavyWeapons || PawnOwner.MyGearWeapon.WeaponType != WT_Heavy )
		{
			// Create the stance
			IdleStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_Idle;

			// Play Looping Idle animation
			PawnOwner.BS_Play(IdleStance, SpeedModifier, 0.2f/SpeedModifier, 0.f, TRUE, TRUE);
		}
	}
}