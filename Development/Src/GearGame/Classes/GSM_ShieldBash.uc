/**
 * Meat Shield: Kidnapper
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_ShieldBash extends GearSpecialMove;

protected function bool InternalCanDoSpecialMove()
{
	return !PawnOwner.IsInCover() && !PawnOwner.IsFiring();
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local vector End, AttackDir, HitLoc, HitNorm;
	local GearPawn HitPawn;
	local TraceHitInfo HitInfo;
	local BodyStance BS_Melee;

	Super.SpecialMoveStarted(bForced, PrevMove);

	PawnOwner.bDoingMeleeAttack = TRUE;

	if (PawnOwner.Role == ROLE_Authority)
	{
		AttackDir = vector(PawnOwner.Rotation);

		End = PawnOwner.Location + AttackDir * 128;

		foreach PawnOwner.TraceActors(class'GearPawn', HitPawn, HitLoc, HitNorm, End, PawnOwner.Location, vect(16,16,64), HitInfo)
		{
			if (PawnOwner.TargetIsValidMeleeTarget(HitPawn, true))
			{
				HitPawn.TakeDamage(250, PawnOwner.Controller, HitLoc, AttackDir * 100.0, class'GDT_ShieldBash', HitInfo);
				HitPawn.DoStumbleFromMeleeSpecialMove();
				if (PCOwner != None)
				{
					PCOwner.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.MeleeHit);
				}
				break;
			}
		}
	}

	BS_Melee.AnimName[BS_Std_Up]='AR_Melee_Smack_A';
	BS_Melee.AnimName[BS_Std_Idle_Lower]='AR_Melee_Smack_A';
	PawnOwner.BS_Play(BS_Melee, 1.f, 0.1f, 0.25f, FALSE, TRUE);

	PawnOwner.SetTimer( 1.5, false, nameof(self.AnimEndTimer), self );
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.bDoingMeleeAttack = FALSE;

	PawnOwner.ClearTimer('AnimEndTimer', self);
}

function AnimEndTimer()
{
	PawnOwner.EndSpecialMove();
}

defaultproperties
{
	bShouldAbortWeaponReload=true
}
