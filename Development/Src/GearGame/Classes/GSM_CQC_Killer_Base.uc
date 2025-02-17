
/**
 * CQC (Close Quarter Combat) Killer Base class
 * Used by all execution moves.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_CQC_Killer_Base extends GSM_InteractionPawnLeader_Base
	abstract;

/** Animations */
var	GearPawn.BodyStance	BS_KillerAnim, BS_VictimAnim;
/** Marker relative offset. Used to position Killer relative to Victim's location. */
var	Vector				MarkerRelOffset;
/** This is the damage type which was usedfor the execution! **/
var class<GearDamageType>		ExecutionDamageType;
/** Flag to keep track if Victim was killed or not */
var transient bool		bKilledVictim;
/** Direction vector, from Killer to Victim. */
var vector				DirToVictim;
/** Time to trigger death */
var FLOAT				VictimDeathTime;
/** Do victim rotation interpolation. Internal. */
var protected bool		bVictimRotInterp;
/** Desired Victim's rotation's Yaw. */
var INT					VictimDesiredYaw;
/** Start Time for Victim's rotation interpolation. 0.f ==  Disable. */
var FLOAT				VictimRotStartTime;
/** Victim rotation interpolation speed. 0.f == Instant */
var FLOAT				VictimRotInterpSpeed;
/** Animation blending in/out times. */
var FLOAT				BlendInTime, BlendOutTime;
/** Length of animation */
var FLOAT				AnimLength;

/** Special Move can be done if we can find a nearby victim. */
protected function bool InternalCanDoSpecialMove()
{
	Follower = GetVictimPawn();
	return (Follower != None && Follower.CanBeSpecialMeleeAttacked(PawnOwner) && !Follower.IsDoingSpecialMove(SM_CQC_Victim) && !Follower.IsDoingSpecialMove(SM_ChainsawVictim));
}

/**
* Used for Pawn to Pawn interactions.
* Return TRUE if we can perform an Interaction with this Pawn.
*/
function bool CanInteractWithPawn(GearPawn OtherPawn)
{
	return (OtherPawn.CanBeSpecialMeleeAttacked(OtherPawn) && Super.CanInteractWithPawn(OtherPawn));
}

/**
 * Find closeby DBNO player to execute.
 */
final function GearPawn GetVictimPawn()
{
	local GearPawn	FoundPawn;
	local bool bInteractionObstructed;
	local Actor HitA;
	local vector HitLocation, HitNormal;

	ForEach PawnOwner.VisibleCollidingActors(class'GearPawn', FoundPawn, 64.f, PawnOwner.Location, TRUE)
	{
		if( FoundPawn != PawnOwner && !PawnOwner.IsSameTeam(FoundPawn) && FoundPawn.CanBeCurbStomped() && !FoundPawn.IsDoingSpecialMove(SM_RecoverFromDBNO) && PawnOwner.TimeSince(FoundPawn.TimeOfRevival) > 0.2f )
		{
			// facing the dude
			if ((Normal(FoundPawn.Location - PawnOwner.Location) dot vector(PawnOwner.Rotation)) >= 0.50)
			{
				if ( Abs(FoundPawn.Location.Z - PawnOwner.Location.Z) < PawnOwner.CylinderComponent.CollisionHeight * 0.8f ||
					(AIController(PawnOwner.Controller) != None && PawnOwner.ReachedDestination(FoundPawn)) )
				{
					bInteractionObstructed = FALSE;
					// verify no intervening geometry
					//@note - delayed until we have a successful interaction since this will be the most expensive portion
					foreach PawnOwner.TraceActors(class'Actor',HitA,HitLocation,HitNormal,FoundPawn.Location,PawnOwner.Location,vect(32,32,32))
					{
						if (HitA != FoundPawn && HitA.bBlockActors)
						{
							//`log(self@GetFuncName()@"interaction obstructed to"@`showvar(FoundPawn)@"due to"@`showvar(HitA));
							bInteractionObstructed = TRUE;
							break;
						}
						else if (HitA == FoundPawn)
						{
							break;
						}
					}
					if (!bInteractionObstructed)
					{
						return FoundPawn;
					}
				}
			}
		}
	}

	return None;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// Reset variables
	bVictimRotInterp = FALSE;
	bKilledVictim = FALSE;

	// Throw heavy weapon to do execution
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		if( PawnOwner.IsCarryingAHeavyWeapon() )
		{
			PawnOwner.ThrowActiveWeapon();
		}
	}

	Super.SpecialMoveStarted(bForced, PrevMove);

	PreKillVictim();
}

function bool IsReadyToStartInteraction()
{
	// Make sure we got rid of our heavy weapon, and we're in a state where we can holster our weapon if we need to
	if( !PawnOwner.IsCarryingAHeavyWeapon() )
	{
		return Super.IsReadyToStartInteraction();
	}

	return FALSE;
}

function StartInteraction()
{
	// Make sure Pawns are placed on their markers
	PlaceOnMarkers();
	PlayExecution();

	// Make sure only the kidnapper kills the victim now.
	Follower.SpecialMoves[Follower.SpecialMove].bOnlyInteractionPawnCanDamageMe = TRUE;
}

/** Play Execution Animation */
function PlayExecution()
{
	// Play animation for the killer.
	AnimLength = PawnOwner.BS_Play(BS_KillerAnim, SpeedModifier, BlendInTime/SpeedModifier, BlendOutTime/SpeedModifier, FALSE, TRUE);

	// If we have an animation defined for the victim, play it.
	if( BS_VictimAnim.AnimName.Length > 0 )
	{
		Follower.BS_Play(BS_VictimAnim, SpeedModifier, BlendInTime/SpeedModifier, BlendOutTime/SpeedModifier, FALSE, TRUE);
	}

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_KillerAnim, TRUE);

	// Setup the timer to execute
	if( VictimDeathTime > 0.f )
	{
		PawnOwner.SetTimer( VictimDeathTime/SpeedModifier, FALSE, nameof(self.KillVictim), self );
	}

	// Camera for Victim
	if( Follower.IsLocallyControlled() )
	{
		Follower.bOverrideDeathCamLookat	= TRUE;
		Follower.DeathCamLookatBoneName		= Follower.PelvisBoneName;
		Follower.DeathCamLookatPawn			= PawnOwner;
	}
}

/** Place both actors on their markers, no interpolation. */
function PlaceOnMarkers()
{
	local Vector	KillerDestination;

	DirToVictim = Follower.Location - PawnOwner.Location;
	DirToVictim.Z = 0.f;
	DirToVictim = Normal(DirToVictim);

	KillerDestination = Follower.Location + RelativeToWorldOffset(Rotator(-DirToVictim), MarkerRelOffset);

	// Move Killer relative to victim, so his marker aligns with victim's root bone.
	SetReachPreciseDestination(KillerDestination);
	// Rotate Killer so he faces hostage.
	SetFacePreciseRotation(Rotator(DirToVictim), 0.25f);

	// Desired victim rotation.
	VictimDesiredYaw = NormalizeRotAxis(Rotator(-DirToVictim).Yaw + 16384);
	// Interp victim rotation.
	SetVictimRotation();
}

/** Separate function, so other executions can implement their variations. */
function SetVictimRotation()
{
	if( VictimRotStartTime > 0.f )
	{
		PawnOwner.SetTimer( VictimRotStartTime / SpeedModifier, FALSE, nameof(self.StartVictimRotInterp), self );
	}
	else
	{
		StartVictimRotInterp();
	}
}

/** Do Victim Rotation */
function StartVictimRotInterp()
{
	local Rotator	VictimRotation;

	if( VictimRotInterpSpeed > 0.f )
	{
		bVictimRotInterp = TRUE;
	}
	else
	{
		VictimRotation.Yaw = VictimDesiredYaw;
		ForcePawnRotation(Follower, VictimRotation);
	}
}

function Tick(float DeltaTime)
{
	local Rotator	VictimRotation;

	Super.Tick(DeltaTime);

	if( bVictimRotInterp )
	{
		VictimRotation.Yaw = FInterpTo(NormalizeRotAxis(Follower.Rotation.Yaw), NormalizeRotAxis(VictimDesiredYaw), DeltaTime, VictimRotInterpSpeed);
		ForcePawnRotation(Follower, VictimRotation);
		if( VictimRotation.Yaw == VictimDesiredYaw )
		{
			bVictimRotInterp = FALSE;
		}
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Make sure Victim is killed if we are kicked out of this special move before we had a chance to do so.
	if( !bKilledVictim )
	{
		KillVictim();
	}

	// Disable Precise Destination
	if( bReachPreciseDestination )
	{
		SetReachPreciseDestination(PawnOwner.Location, FALSE);
	}

	// Clear execution timer.
	PawnOwner.ClearTimer('KillVictim');
	PawnOwner.ClearTimer('StartVictimRotInterp');

	// Disable end of animation notification
	PawnOwner.BS_SetAnimEndNotify(BS_KillerAnim, FALSE);

	// we always have to reset the flag because the pawn has usually lost its controller on the client by this point
	// so it will always return false for IsLocallyControlled()
	Follower.bOverrideDeathCamLookat = FALSE;

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

/** Handles scoring for the victim before they are officially dead */
function PreKillVictim()
{
	if( PawnOwner != None && PawnOwner.WorldInfo.NetMode != NM_Client &&
		Follower != None && Follower.IsGameplayRelevant() )
	{
		// award the execution
		GearPRI(PawnOwner.PlayerReplicationInfo).ScoreExecution(GearPRI(Follower.PlayerReplicationInfo),ExecutionDamageType,GDT_NORMAL);
	}
}

/** Kill Victim */
function KillVictim()
{
	bKilledVictim = TRUE;

	if( PawnOwner != None && PawnOwner.WorldInfo.NetMode != NM_Client &&
		Follower != None && Follower.IsGameplayRelevant() )
	{
		// Have the momentum be in the direction the killer is looking
		Follower.TearOffMomentum = Vector(PawnOwner.Rotation);
		Follower.Died(PawnOwner.Controller, ExecutionDamageType, Follower.Location);
	}
}

defaultproperties
{
	FollowerSpecialMove=SM_CQC_Victim
	bForcePrecisePosition=TRUE
	BlendInTime=0.25f
	BlendOutTime=0.25f

	Action={(
			ActionName=CurbStomp,
			ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=284,V=346,UL=104,VL=91)))	),
	)}


	ExecutionDamageType=class'GDT_CurbStomp'

	bDisablePOIs=TRUE
}
