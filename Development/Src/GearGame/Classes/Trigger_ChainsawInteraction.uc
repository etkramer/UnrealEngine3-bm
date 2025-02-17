/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Trigger_ChainsawInteraction extends Trigger;

var() Actor ActorToFace, ActorToMoveTo;

var() bool bWithBlood;

var() int NumberOfMeleeHitsNeeded;

var() ParticleSystem ChainsawFX;

var() repnotify		bool	bDisplayIcon;
var()				float	IconAnimSpeed;

var() SoundCue ChainsawRipSound;

/** If TRUE, don't look for or try to fire any kismet events - just call TriggerChainsawed. */
var bool bNoKismet;

var bool bActivatedByMelee;

var Pawn CurrentActivator;

replication 
{
	if( Role == ROLE_Authority )
		bDisplayIcon, CurrentActivator;
}

simulated event ReplicatedEvent( name VarName )
{
	local GearPC PC;

	switch( VarName )
	{
		case 'bDisplayIcon':
			foreach LocalPlayerControllers( class'GearPC', PC )
			{
				UpdateDisplayIcon( GearPawn(PC.Pawn) );
			}
			break;
	}

	Super.ReplicatedEvent( VarName );
}

/** Called from GSM_ChainsawAttack_Object.SpecialMoveStarted - allows trigger to do script reaction to being chansawed. */
simulated function TriggerBeginChainsaw(GearPawn Pawn)
{
	if( bNoKismet )
	{
		bDisplayIcon = FALSE;
	}
}

/** Called from GSM_ChainsawAttack_Object.SpecialMoveEnded - allows trigger to do script reaction to being chansawed. */
simulated function TriggerEndChainsaw(GearPawn Pawn)
{
	local GearPC PC;
	PC = GearPC(Pawn.Controller);
	if (PC != None)
	{
		PC.ClientToggleKismetIcon( FALSE, eGEARBUTTON_ChainsawMash, IconAnimSpeed );
	}
}

/** Called when you try and grenade tag one of these trigger. */
simulated function bool GrenadeTagged(GearWeap_GrenadeBase GrenadeWeap, vector HitLocation, vector HitNorm, TraceHitInfo HitInfo)
{
	return FALSE;
}

simulated function HitByMelee(GearPawn Pawn)
{
	if( bNoKismet )
	{
		bDisplayIcon = FALSE;
	}
	UpdateDisplayIcon( Pawn );
}

simulated function Destroyed()
{
	local GearPawn GP;

	Super.Destroyed();

	foreach TouchingActors( class'GearPawn', GP )
	{
		UnTouch( GP );
	}
	bDisplayIcon = FALSE;
}

simulated function bool ShouldDisplayIcon( Actor Other )
{
	local GearPawn GP;
	if( bDisplayIcon )
	{
		if( bNoKismet || TriggerEventClass( class'SeqEvt_ChainsawInteraction', Other, 0, TRUE ) )
		{
			GP = GearPawn(Other);
			if (NumberOfMeleeHitsNeeded > 0 || GP == None || GearWeap_AssaultRifle(GP.Weapon) != None)
			{
				return TRUE;
			}
		}
	}

	return FALSE;	
}

simulated function UpdateDisplayIcon( Actor Other )
{
	local GearPawn	GP;
	local GearPC	PC;

	GP = GearPawn(Other);
	if( GP != None )
	{
		PC = GearPC(GP.Controller);
		if( PC != None )
		{
			PC.ClientToggleKismetIcon( ShouldDisplayIcon(Other), eGEARBUTTON_ChainsawMash, IconAnimSpeed );
		}
	}
}

simulated event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	local GearPawn	GP;
	local GearPC	PC;

	GP = GearPawn(Other);
	if( GP != None )
	{
		PC = GearPC(GP.Controller);
		if( PC != None && PC.IsLocalPlayerController() )
		{
			UpdateDisplayIcon( Other );
		}
    }
}

simulated event UnTouch( Actor Other )
{
	local GearPawn	GP;
	local GearPC	PC;

	GP = GearPawn(Other);
	if( GP != None )
	{
		PC = GearPC(GP.Controller);
		if( PC != None && PC.IsLocalPlayerController() )
		{
			PC.ClientToggleKismetIcon( FALSE, eGEARBUTTON_ChainsawMash, IconAnimSpeed );
		}
	}
}

defaultproperties
{
	Begin Object Name=CollisionCylinder
		CollisionRadius=128.f
		CollisionHeight=128.f
	End Object
	
	SupportedEvents.Add(class'SeqEvt_ChainsawInteraction')
	SupportedEvents.Remove(class'SeqEvent_Used')

	NumberOfMeleeHitsNeeded=3

	bDisplayIcon=TRUE
	IconAnimSpeed=0.f
}
