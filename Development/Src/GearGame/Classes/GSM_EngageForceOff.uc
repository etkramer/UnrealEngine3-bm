
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_EngageForceOff extends GSM_Engage;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local BodyStance EndStance;

	Super.SpecialMoveStarted(bForced,PrevMove);

	if( PawnOwner.EngageTrigger != None )
	{
		if( (PawnOwner.EngageTrigger.ENGAGE_AnimName_End != '' || PawnOwner.EngageTrigger.ENGAGE_AnimName_ForceOff != '') &&
			(PawnOwner.EngageTrigger.ENGAGE_bPlayAnimationForHeavyWeapons || PawnOwner.MyGearWeapon.WeaponType != WT_Heavy) )
		{
			// Create the STOP stance
			if( PawnOwner.EngageTrigger.ENGAGE_AnimName_ForceOff == '' )
			{
				EndStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_End;
			}
			else
			{
				EndStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_ForceOff;
			}

			// Play body stance animation.
			PawnOwner.BS_Play(EndStance, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

			// Turn off Root motion on animation node
			PawnOwner.BS_SetRootBoneAxisOptions(EndStance, RBA_Discard, RBA_Discard, RBA_Discard);

			// Enable end of animation notification. This is going to call SpecialMoveEnded()
			PawnOwner.BS_SetAnimEndNotify(EndStance, TRUE);
		}
		else
		{
			PawnOwner.SetTimer(0.1f, FALSE, 'EndInteraction', Self);
		}

		PawnOwner.EngageTrigger.EngagedPawn = None;
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local BodyStance EndStance;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	if( PawnOwner.EngageTrigger != None )
	{
		if( (PawnOwner.EngageTrigger.ENGAGE_AnimName_End != '' || PawnOwner.EngageTrigger.ENGAGE_AnimName_ForceOff != '') &&
			(PawnOwner.EngageTrigger.ENGAGE_bPlayAnimationForHeavyWeapons || PawnOwner.MyGearWeapon.WeaponType != WT_Heavy) )
		{
			// Create the STOP stance
			if( PawnOwner.EngageTrigger.ENGAGE_AnimName_ForceOff == '' )
			{
				EndStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_End;
			}
			else
			{
				EndStance.AnimName[BS_FullBody] = PawnOwner.EngageTrigger.ENGAGE_AnimName_ForceOff;
			}
		}
		PawnOwner.BS_SetAnimEndNotify(EndStance, FALSE);
	}

	// unholster the weapon
	PawnOwner.MyGearWeapon.HolsterWeaponTemporarily(FALSE);
}
