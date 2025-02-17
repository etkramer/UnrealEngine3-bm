/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_ChainsawAttack_Object extends GSM_BasePlaySingleAnim;

var Trigger_ChainsawInteraction ChainsawTrigger;
var bool bWithBlood;
var Actor ActorToMoveTo;

protected function bool InternalCanDoSpecialMove()
{
	return TRUE;
}

 function bool GetAimOffsetOverride(out rotator DeltaRot)
 {
 	// eliminate the offset
 	DeltaRot = rot(0,0,0);
 	return TRUE;
 }

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearWeap_AssaultRifle Rifle;
	local array<SequenceEvent> Evts;
	local SeqEvt_ChainsawInteraction Evt;

	Super(GearSpecialMove).SpecialMoveStarted(bForced, PrevMove);

	ActorToMoveTo = None;
	PawnOwner.InterpolatePawnRotation();
	if (PawnOwner.TargetedGDO == None)
	{
		// Call script function so trigger knows it star
		foreach PawnOwner.TouchingActors(class'Trigger_ChainsawInteraction',ChainsawTrigger)
		{
			ActorToMoveTO = None;
			bWithBlood = FALSE;
			// skip over triggers being used by other players
			if (ChainsawTrigger.CurrentActivator != None && ChainsawTrigger.CurrentActivator != PawnOwner && !ChainsawTrigger.bActivatedByMelee)
			{
				continue;
			}
			// mini-hack - if activated by melee first we need to reset the trigger count otherwise the chainsawing will fail to activate the event and soft-lock the user
			if (ChainsawTrigger.bActivatedByMelee)
			{
				ChainsawTrigger.FindEventsOfClass(class'SeqEvt_ChainsawInteraction',Evts);
				Evt = SeqEvt_ChainsawInteraction(Evts[0]);
				if (Evt != None && Evt.TriggerCount < Evt.MaxTriggerCount)
				{
					Evt.TriggerCount = 0;
					ChainsawTrigger.bActivatedByMelee = FALSE;
				}
			}
			ChainsawTrigger.CurrentActivator = PawnOwner;
			AlignToActor = ChainsawTrigger.ActorToFace;
			if (AlignToActor != None && GearPC(PawnOwner.Controller) != None)
			{
				GearPC(PawnOwner.Controller).SetFocusPoint(TRUE,AlignToActor,vect2d(5,7),vect2d(1,1),40,TRUE,TRUE,TRUE);
			}
			ActorToMoveTo = ChainsawTrigger.ActorToMoveTo;
			bWithBlood = ChainsawTrigger.bWithBlood;
			ChainsawTrigger.TriggerEventClass(class'SeqEvt_ChainsawInteraction',PawnOwner,0,FALSE);
			ChainsawTrigger.TriggerBeginChainsaw(PawnOwner);
			`logSMExt(PawnOwner,"Found trigger:"@`showvar(ChainsawTrigger)@`showvar(ActorToMoveTo)@`showvar(AlignToActor));
			break;
		}
	}
	else
	{
		AlignToActor = PawnOwner.TargetedGDO;
		bWithBlood = FALSE;
		// set a timer to deal periodic damage to the GDO
		PawnOwner.SetTimer( 0.35,FALSE,nameof(self.DamageTargetedGDO), self );
	}


	// set blood effects here in case ReachedPrecisePosition() doesn't get called (doesn't seem to work in all cases?)
	Rifle = GearWeap_AssaultRifle(PawnOwner.Weapon);
	if (Rifle != None)
	{
		Rifle.bDisableChainsawBloodFX = !bWithBlood;
	}

	if (ActorToMoveTo != None)
	{
		SetReachPreciseDestination(ActorToMoveTo.Location);
		PawnOwner.SetTimer(1.f,FALSE,nameof(SafetyTeleport),self);
	}
	else
	{
		StartAttack();
	}
}

function SafetyTeleport()
{
	if (PawnOwner.Role == ROLE_Authority && ActorToMoveTo != None && VSize(ActorToMoveTo.Location - PawnOwner.Location) > 32.f)
	{
		PawnOwner.SetLocation(ActorToMoveTo.Location);
		StartAttack();
	}
}

event ReachedPrecisePosition()
{
	PawnOwner.ClearTimer(nameof(SafetyTeleport),self);
	StartAttack();
}

function StartAttack()
{
	local GearWeap_AssaultRifle Rifle;
	local ParticleSystem ChainsawFX;

	PlayAnimation();
	if (PawnOwner.Role == ROLE_Authority)
	{
		// tell the rifle to start the slashing audio/fx
		Rifle = GearWeap_AssaultRifle(PawnOwner.Weapon);
		if (Rifle != None && !Rifle.bStartedRipping)
		{
			// skip blood if fighting a GDO, pass in trigger override if available
			if( ChainsawTrigger != none )
			{
				ChainsawFX = ChainsawTrigger.ChainsawFX;
			}
			else if( PawnOwner.TargetedGDO != none )
			{
				ChainsawFX = PawnOwner.TargetedGDO.PS_ChainsawAttack;
			}
			else
			{
				ChainsawFX = none;
			}
	
			Rifle.StartAttackObject( !bWithBlood, ChainsawFX );
		}
   }
}

function ChainsawAnimNotify()
{
	PawnOwner.TargetedGDO = None;
}

function DamageTargetedGDO()
{
	if (PawnOwner != None && PawnOwner.TargetedGDO != None)
	{
		PawnOwner.TargetedGDO.TakeDamage(150,PawnOwner.Controller,PawnOwner.Location + vector(PawnOwner.Rotation) * 64.f,vector(PawnOwner.Rotation),class'GDT_Chainsaw');
		PawnOwner.SetTimer( 0.25,FALSE,nameof(self.DamageTargetedGDO), self );
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);
	PawnOwner.ClearTimer(nameof(SafetyTeleport),self);
	if (ChainsawTrigger != None)
	{
		ChainsawTrigger.CurrentActivator = None;
		if (GearPC(PawnOwner.Controller) != None)
		{
			GearPC(PawnOwner.Controller).ClearFocusPoint(TRUE,TRUE);
		}
		ChainsawTrigger.TriggerEventClass(class'SeqEvt_ChainsawInteraction',PawnOwner,1,FALSE);
		ChainsawTrigger.TriggerEndChainsaw(PawnOwner);
	}
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="AR_Melee_Saw_A")
	BlendInTime=0.25
	BlendOutTime=0.25

	SpecialMoveCameraBoneAnims(0)=(AnimName="Camera_COG_ChainsawDuel_Win",CollisionTestVector=(X=0.f,Y=0.f,Z=0.f))

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE
	bDisableLook=TRUE
	bLockPawnRotation=TRUE
	bDisablePOIs=TRUE
	bForcePrecisePosition=TRUE
}
