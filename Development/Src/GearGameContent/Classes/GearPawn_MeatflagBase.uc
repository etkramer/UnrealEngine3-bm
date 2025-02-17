/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_MeatflagBase extends GearPawn_NPCCOGBase
	abstract
	config(Pawn);

/** POI to look at the victim */
var transient GearPointOfInterest_Meatflag VictimPOI;

/** time until we get up after being kidnapped */
var config float KidnapRecoveryTime;


/**
 * All of the MP characters are going to need a couple of things set for them.  For one thing, they’ll need to
 * have their MP shader set on them (list forthcoming) and we’ll have to figure out what values we want for each
 * MP map for the rimlight-at-distance.
 **/
var protected MaterialInstanceConstant RimShaderMaterialSpecific;
var protected MaterialInstanceConstant RimShaderMaterialSpecificHead;

var() LinearColor RimColor;
var() float RimDistance; // – Default is 1024
var() float EdgeWidth; //– Default is 0.80

var() float	NoDamageOrSpecialMoveHackDelay;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( (Role == ROLE_Authority) && (VictimPOI == None) )
	{
		VictimPOI = Spawn( class'GearPointOfInterest_Meatflag', self );
	}

	// Add timers to check getting stuck in things
	if(Role == ROLE_Authority)
	{
		SetTimer(0.5, TRUE, nameof(CheckStuckInWorld));

		SetTimer(NoDamageOrSpecialMoveHackDelay, FALSE, nameof(HandleNoDamageOrSpecialMove));
	}
}

/** Can never be killed */
function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	// Have kidnapper drop hostage if meat flag was supposed to die.
	if( IsAHostage() )
	{
		InteractionPawn.ServerEndSpecialMove();
	}

	return FALSE;
}


simulated protected function InitMPRimShader()
{
	if( RimShaderMaterialSpecific != none )
	{
		MPRimShader = new(Outer) class'MaterialInstanceConstant';
		MPRimShader.SetParent( RimShaderMaterialSpecific );
		Mesh.SetMaterial( 0, MPRimShader );
	}
	else
	{
		MPRimShader = Mesh.CreateAndSetMaterialInstanceConstant(0);
	}

	SetUpRimShader( MPRimShader );

}

/** This will set the character specific rim shader values that have been set for the specific map **/
simulated protected function SetUpRimShader( out MaterialInstanceConstant TheMIC )
{
	// This will propagate down the inheritance chain to the appropriate instance shaders.
	TheMIC.SetVectorParameterValue( 'RimColor', RimColor );
	TheMIC.SetScalarParameterValue( 'RimDistance', RimDistance );
	TheMIC.SetScalarParameterValue( 'EdgeWidth', EdgeWidth );
}


/** Victims can't be special melee attacked **/
simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

/** Give the pawn a chance to override whether it should DBNO or not */
function bool ShouldForceDBNO( class<GearDamageType> GearDmgType, Pawn InstigatedBy, vector HitLocation )
{
	return TRUE;
}

/** Give the pawn a chance to override whether it should Gib or not */
function bool ShouldGib( class<GearDamageType> GearDmgType, Pawn InstigatedBy )
{
	return FALSE;
}

/** see if we should play the kidnap recovery anim */
function CheckKidnapRecovery()
{
	if( KidnapRecoveryTime > 0.0 )
	{
		// Abort current special move.
		if( IsDoingASpecialMove() )
		{
			ServerEndSpecialMove(SpecialMove);
		}

		DoStumbleGoDownSpecialMove();
		SetTimer(KidnapRecoveryTime, FALSE, nameof(GetBackUpFromKnockDown));
	}
}

simulated function bool CanBeAHostage()
{
	// don't allow the meatflag to be picked up again when he was very recently dropped and recovering
	// prevents an exploit where a player spams drop and grab commands
	return (!IsTimerActive(nameof(GetBackUpFromKnockDown)) || GetTimerCount(nameof(GetBackUpFromKnockDown)) > 1.0);
}

state DBNO
{
	event BeginState(name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);
		ClearTimer('GetBackUpFromKnockDown');
	}

	// can't die while DBNO
	function DoFatalDeath(Controller InKillerController, class<GearDamageType> InDamageType, Vector InHitLocation, Vector InMomentum);
}

state Kidnapped
{
	event BeginState(name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		ClearTimer('GetBackUpFromKnockDown');

		if( InteractionPawn != None && InteractionPawn.Controller != None && InteractionPawn.Controller.PlayerReplicationInfo != None )
		{
			VictimPOI.KidnapperTeamIndex = InteractionPawn.Controller.PlayerReplicationInfo.GetTeamNum();
			VictimPOI.SetEnabled(TRUE);
		}
	}

	event EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);

		VictimPOI.SetEnabled(FALSE);

		// Special recovery for meatflag.
		CheckKidnapRecovery();
	}
}

simulated function int GetLookAtPriority( GearPC PC, int DefaultPriority )
{
	return VictimPOI.LookAtPriority;
}

simulated event FellOutOfWorld(class<DamageType> DmgType)
{
	local NavigationPoint N;

	if (Role == ROLE_Authority)
	{
		N = GearGameCTM_Base(WorldInfo.Game).GetVictimSpawnLocation();
		SetLocation(N.Location);
		SetRotation(N.Rotation);
		Velocity = vect(0,0,0);
		Acceleration = vect(0,0,0);

		bCollideWorld = TRUE;

		if (Controller != None)
		{
			Controller.MoveTimer = -1.0;
		}
	}
}

simulated event OutsideWorldBounds()
{
	if (Role == ROLE_Authority)
	{
		FellOutOfWorld(None);
		SetTimer( 0.01, false, nameof(BackInsideWorld) );
	}
}

/** called to restore collision/physics after safely being returned inside the world after being outside */
function BackInsideWorld()
{
	SetPhysics(PHYS_Falling);
	SetCollision(true, true);
}

/** Fallback to see if meatflag gets stuck, and teleport him back to a good spot */
function CheckStuckInWorld()
{
	// Only do this check if you are not a hostage
	if(!IsAHostage() && InfantryStuckInWorld())
	{
		`log("!!! TELEPORTING"@self@"to nearest node, due to 'stuck in world' check.");
		TeleportToNearestPathNode();
	}
}

/** Util to teleport flag back to nearest path node */
function TeleportToNearestPathNode()
{
	local NavigationPoint	N;

	// Only do this if not a hostage
	if(IsAHostage())
	{
		N = class'NavigationPoint'.static.GetNearestNavToActor(self);
		SetLocation(N.Location);
		//SetRotation(N.Rotation);
		Velocity = vect(0,0,0);
		Acceleration = vect(0,0,0);

		bCollideWorld = TRUE;

		if (Controller != None)
		{
			Controller.MoveTimer = -1.0;
		}
		SetPhysics(PHYS_Falling);
		SetCollision(true, true);
	}
}

/** Handle nothing happening to meatflag for a while */
function HandleNoDamageOrSpecialMove()
{
	if (Role == ROLE_Authority)
	{
		// If not a hostage - teleport to a path node
		if(!IsAHostage())
		{
			`log("!!! TELEPORTING"@self@"to nearest node, due to lack of damage/special move.");
			TeleportToNearestPathNode();
		}

		// Re-set timer to fire again
		SetTimer(NoDamageOrSpecialMoveHackDelay, FALSE, nameof(HandleNoDamageOrSpecialMove));
	}
}

function TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	Super.TakeDamage( DamageAmount, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );

	if (Role == ROLE_Authority)
	{
		// Delay teleport when taking damage
		SetTimer(NoDamageOrSpecialMoveHackDelay, FALSE, nameof(HandleNoDamageOrSpecialMove));
	}
}

simulated event DoSpecialMove(ESpecialMove NewMove, optional bool bForceMove, optional GearPawn InInteractionPawn, optional INT InSpecialMoveFlags)
{
	Super.DoSpecialMove(NewMove, bForceMove, InInteractionPawn, InSpecialMoveFlags);

	if (Role == ROLE_Authority)
	{
		// Delay teleport when doing a special move
		SetTimer(NoDamageOrSpecialMoveHackDelay, FALSE, nameof(HandleNoDamageOrSpecialMove));
	}
}

state KnockedDown
{
	// ignored so we can make sure the meatflag can't die here
	simulated function SetDontModifyDamage(bool NewValue)
	{
		bAllowDamageModification = true;
	}

	function AdjustPawnDamage( out int Damage, Pawn InstigatedBy, vector HitLocation, out vector Momentum,
					class<GearDamageType> GearDamageType, optional out TraceHitInfo HitInfo )
	{
		Super.AdjustPawnDamage(Damage, InstigatedBy, HitLocation, Momentum, GearDamageType, HitInfo);
		// prevent meatflag from taking fatal damage while knocked down
		Damage = Min(Damage, Health - 1);
	}
}

defaultproperties
{
	SightBoneName=none
	ControllerClass=class'GearAI_CTMVictim'

	RimColor=(R=4.0,G=4.0,B=4.0,A=0.0)
	RimDistance=1024.0f
	EdgeWidth=1.80f

	NoDamageOrSpecialMoveHackDelay=60.0

	bModifyReachSpecCost=false
}
