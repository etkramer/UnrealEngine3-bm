
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustKantusBase extends GearPawn_LocustBase
	abstract
	native(Pawn)
	config(Pawn);

var repnotify byte ThrowCount; // like flash count, but with more hax

/** e.g. "Boom", "Crush", etc. */
var protected const array<SoundCue>		KantusAttackTelegraphDialogue;

var ParticleSystem	ReviveParticleSystem;

var array< class<Gearpawn> > MinionClasses;
replication
{
	if (bNetDirty)
		ThrowCount;
}

simulated function DoKantusReviveEffects(Gearpawn GP)
{
	GP.KantusRevivePSC = new(GP) class'ParticleSystemComponent';
	GP.KantusRevivePSC.SetTemplate(default.ReviveParticleSystem);
	GP.mesh.AttachComponent(GP.KantusRevivePSC,GP.MeleeDamageBoneName);
	GP.KantusRevivePSC.ActivateSystem();
	if(GP != self)
	{
		GP.DesiredRotation = Rotator(location - GP.Location);
		GP.Controller.Focus = self;
		GP.Controller.DesiredRotation = rotator(location - GP.Location);
	}
}

simulated function TelegraphGrenadeThrow()
{
	local int RandIdx;

	// speak telegraph line
	if (KantusAttackTelegraphDialogue.length > 0)
	{
		RandIdx = Rand(KantusAttackTelegraphDialogue.length);
		// make addressee AI's target?
		SpeakLine(None, KantusAttackTelegraphDialogue[RandIdx], "", 0.f, Speech_GUDS,,,,,, TRUE);
	}
}

function ThrowingInkGrenade(float Delay)
{
	local GearAI_Kantus KAI;
	ThrowCount++;

	KAI = GearAI_Kantus(Controller);
	if(KAI != none)
	{
		// let the controller do whatever it needs to when we throw this thing
		KAI.ThrowingInkGrenade(Delay);
	}
}

function DoKnockDown()
{
	local GearPawn	FoundPawn;
	local vector MeToVictimNormal;
	local vector LinearVel;
	local float DistFromMe;
	local int i;
	local GearAI_Kantus KantusAI;

	KantusAI = GearAI_Kantus(Controller);

	if(KantusAI == none || Role < ROLE_Authority)
	{
		return;
	}

	for(i=0;i<KantusAI.PawnsToKnockDown.length;i++)
	{
		FoundPawn = KantusAI.PawnsToKnockDown[i];
		//`log(self$GetFuncName()@"Victim:"@FoundPawn);
		MeToVictimNormal = FoundPawn.Location - Location;
		DistFromMe = VSize(MeToVictimNormal);
		// normalize, making use of previous sqrt
		MeToVictimNormal /= DistFromMe;

		if( (KantusAI != None) 
			&& (FastTrace(FoundPawn.Location,Location))
			&& (!FoundPawn.IsProtectedByShield(MeToVictimNormal,FALSE,Location))
			&& (FoundPawn.DrivenVehicle == None || !FoundPawn.DrivenVehicle.IsA('Turret_TroikaCabal'))  // need to check this otherwise you end up in lala land with camera all crazy and knocked down guy shooting troika
			)
		{
			LinearVel = MeToVictimNormal * 256.f * FClamp(512.f/DistFromMe,0.4f,1.f);
			LinearVel.X = int(LinearVel.X);
			LinearVel.Y = int(LinearVel.Y);
			LinearVel.Z = Min(LinearVel.Z,0);	// clamp the z to only push down
	
			//`log(self@GetFuncName()@"CALLING KNOCKDOWN ON "$FoundPawn);
			if(FoundPawn.CurrentLink != none)
			{
				FoundPawn.Controller.NotifyCoverDisabled( FoundPawn.CurrentLink, FoundPawn.CurrentSlotIdx );
			}

			FoundPawn.Knockdown(LinearVel, vect(0,0,0));
		}
	}
}

simulated function bool CanBeAHostage()
{
	return FALSE;
}


simulated native final function bool IsThrowingOffHandGrenade();
native simulated function bool CanFireWeapon(optional bool bTestingForTargeting);

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	if(!Super.CanBeSpecialMeleeAttacked(Attacker))
	{
		return false;
	}
	
	// if we're doing our knockdown scream, can't be chainsawed!
	if(IsDoingSpecialMove( GSM_Kantus_KnockDownScream ))
	{
		return false;
	}

	return true;

}

defaultproperties
{
	NoticedGUDSPriority=150
	NoticedGUDSEvent=GUDEvent_NoticedKantus
	bCanDBNO=FALSE
}
