
/**
 * GSM_InteractionPawnLeader_Base
 * Base class for Pawn to Pawn Interactions.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_InteractionPawnLeader_Base extends GearSpecialMove
	native(SpecialMoves)
	abstract;

/** Pointer to Follower. */
var GearPawn		Follower;
/** if other than SM_None, then force InteractionPawn into this special move. */
var ESpecialMove	FollowerSpecialMove;
/** Max time to wait for Interaction to start. If it can't be made, special move will be aborted. */
var float			InteractionStartTimeOut;

/** If TRUE, Pawns will be aligned with each other. */
var bool			bAlignPawns;
/** Desired distance to align both pawns. */
var	float			AlignDistance;
/** Should Follower look in same dir as me? */
var bool			bAlignFollowerLookSameDirAsMe;

// C++ functions
cpptext
{
	virtual void PrePerformPhysics(FLOAT DeltaTime);
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced, PrevMove);

	// Reset variables
	Follower = None;
	bAlignFollowerLookSameDirAsMe = default.bAlignFollowerLookSameDirAsMe;
	bAlignPawns = default.bAlignPawns;

	// Set up a safety net in case interaction cannot be started
	PawnOwner.SetTimer( InteractionStartTimeOut, FALSE, nameof(self.InteractionStartTimedOut), self );

	// See if we can start interaction right now. If we can't, keep trying until we can.
	CheckReadyToStartInteraction();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Clear timer
	PawnOwner.ClearTimer('CheckReadyToStartInteraction', Self);

	// Clear reference to Interaction Pawn.
	Follower = None;

	//@laurent - Hoping we don't need this.
	/** 
	// On server, always let owning client know that we're leaving an interaction special move
	// Since client is waiting for synchronization in CheckReadyToStartInteraction(), sometimes the Follower
	// can die and be sent to another special move, and the owning client can stay waiting and be stuck.
	if( PawnOwner.WorldInfo.NetMode != NM_Client && !PawnOwner.IsLocallyControlled() )
	{
		PawnOwner.ServerEndSpecialMove(PrevMove);
	}
	*/

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

/** Safety net in case Interaction cannot be started. Abort special move. */
function InteractionStartTimedOut()
{
	`Warn(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "InteractionStartTimeOut hit!! Aborting move." @ Follower @ Follower.SpecialMove );

	// Call on whatever is available. This could be replicated both ways, but we really do not want to be stuck.
	if( PawnOwner.IsLocallyControlled() )
	{
		PawnOwner.LocalEndSpecialMove(PawnOwner.SpecialMove);
	}
	else if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		PawnOwner.ServerEndSpecialMove(PawnOwner.SpecialMove);
	}
}

/** Checks if Interaction is ready to be started, and starts if it is. */
final function CheckReadyToStartInteraction()
{
	// Make sure we have an InteractionPawn, this is a requirement for these types of special moves
	if( PawnOwner.InteractionPawn != None )
	{
		// Save variable here for ease of use.
		Follower = PawnOwner.InteractionPawn;
		// If server, start Follower special move if not already done.
		if( PawnOwner.WorldInfo.NetMode != NM_Client )
		{
			// Make sure we have a valid Pawn to work with
			if( Follower != None && Follower.IsGameplayRelevant() )
			{
				if( !Follower.IsDoingSpecialMove(FollowerSpecialMove) )
				{
					Follower.ServerDoSpecialMove(FollowerSpecialMove, TRUE, PawnOwner);
				}
			}
			else
			{
				// Our Pawn is never going to go in his special move... pop an error, and exit move.
				`Warn(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Follower not gameplay relevant, Interaction cannot be started!!!" @ Follower @ Follower.SpecialMove);
				PawnOwner.ServerEndSpecialMove(PawnOwner.SpecialMove);
			}
		}
	}

	// If not ready, then set timer to try again next tick.
	if( !IsReadyToStartInteraction() )
	{
		if( Follower == None )
		{
			`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Not ready to StartInteraction, delay... Follower:" @ Follower, ,'SpecialMoves');
		}
		else
		{
			`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Not ready to StartInteraction, delay... Follower:" @ Follower @ "Follower.SpecialMove:" @ Follower.SpecialMove, ,'SpecialMoves');
		}

		// Retry next frame...
		PawnOwner.SetTimer( PawnOwner.WorldInfo.DeltaSeconds, FALSE, nameof(self.CheckReadyToStartInteraction), self );
	}
	// otherwise, start interaction now!
	else
	{
		`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "StartInteraction. Follower:" @ Follower @ "Follower.SpecialMove:" @ Follower.SpecialMove, ,'SpecialMoves');

		PawnOwner.ClearTimer('CheckReadyToStartInteraction', Self);
		// Clear timeout timer
		PawnOwner.ClearTimer('InteractionStartTimedOut', Self);
		// Notify follower that we're about to start.
		Follower.SpecialMoveMessageEvent('InteractionStarted', PawnOwner);
		// And start interaction!
		StartInteraction();
	}
}

/**
 * Conditions to determine if interaction is ready to be started.
 * - Need an InteractionPawn
 * - InteractionPawn needs to be doing his InteractionPawnSpecialMove if it is other than SM_None.
 */
function bool IsReadyToStartInteraction()
{
	return (Follower != None && Follower.IsDoingSpecialMove(FollowerSpecialMove));
}

/** StartInteraction */
function StartInteraction();

/** Messages sent to this special move */
function bool MessageEvent(Name EventName, Object Sender)
{
	if( EventName == 'FollowerLeavingSpecialMove' )
	{
		OnFollowerLeavingSpecialMove();
		return TRUE;
	}

	return Super.MessageEvent(EventName, Sender);
}

/** Notification when Follower is leaving his FollowerSpecialMove */
function OnFollowerLeavingSpecialMove();


/**
 * Dump relative location of a Bone to PawnOwner
 * For debugging purposes
 */
function DebugSocketRelativeLocation(name InSocketName)
{
	local Vector MarkerLoc;
`if(`notdefined(FINAL_RELEASE))
	local Vector RelativeLoc;
`endif
	local Rotator MarkerRot;

	// Get position of marker
	//MarkerLoc = PawnOwner.Mesh.GetBoneLocation(BoneName);
	PawnOwner.Mesh.GetSocketWorldLocationAndRotation(InSocketName, MarkerLoc, MarkerRot);

	// Dump a little debug info to display the location we got:
	PawnOwner.DrawDebugSphere(MarkerLoc, 4, 8, 255, 0, 255, TRUE);

`if(`notdefined(FINAL_RELEASE))
	// Transform marker location to be relative to Leader location and rotation.
	RelativeLoc = WorldToRelativeOffset(PawnOwner.Rotation, MarkerLoc - PawnOwner.Location);

	`log(GetFuncName() @ "RelativeLoc:" @ RelativeLoc);
`endif
}

/** 
 * Special Execution version. 
 * Test to play same animation on victim. 
 * WARNING: * ForcePitchCentering not reset for Follower
 *			* Follower's ViewTarget is set to the Leader.
 */
function PlayExecutionCameraAnim(out const array<CameraAnim> InCameraAnims)
{
	local GearPC	FollowerPC;
	local int		AnimIdx;

	if( PCOwner != None && PawnOwner.IsLocallyControlled() )
	{
		AnimIdx = PCOwner.ChooseRandomCameraAnim(InCameraAnims);
		if (AnimIdx != INDEX_NONE)
		{
			Super.PlayCameraAnim(PawnOwner, InCameraAnims[AnimIdx],,, 0.25f, 0.25f);

			// If Follower is human controlled, have him spectate his death from the killer's perspective
			if( Follower.IsHumanControlled() )
			{
				FollowerPC = GearPC(Follower.Controller);
				FollowerPC.SetViewTarget(PawnOwner);

				// Also play the death camera
				if( Follower.IsLocallyControlled() )
				{
					Super.PlayCameraAnim(Follower, InCameraAnims[AnimIdx],,, 0.25f, 0.25f);
				}
			}		
		}
	}
}

/** Attach Follower to Leader */
function AttachFollowerToLeader(Name SocketName, optional Vector AttachLocation, optional Rotator AttachRotation)
{
	local SkeletalMeshSocket	Socket;

	// Force replication update.
	Follower.bForceNetUpdate = TRUE;

	Follower.SetBase(None);
	Follower.SetPhysics(PHYS_None);
	Follower.SetHardAttach(TRUE);

	TogglePawnCollision(Follower, FALSE);

	// Make sure kidnapper has his attachment bone defined.
	Socket = PawnOwner.Mesh.GetSocketByName(SocketName);
	if( Socket != None )
	{
		Follower.SetBase(PawnOwner,, PawnOwner.Mesh, Socket.BoneName);
		Follower.SetRelativeLocation( Socket.RelativeLocation - Follower.default.Mesh.Translation );
		//Follower.SetRelativeRotation( Socket.RelativeRotation );
		Follower.SetMeshTranslationOffset( vect(0,0,0) );
		//Follower.Mesh.SetTranslation( vect(0,0,0) );
	}
	else
	{
		`Warn(PawnOwner.WorldInfo.TimeSeconds @ class @ GetFuncName() @ "Leader" @ PawnOwner.class @ "Has no attachment socket" @ SocketName @ "!!!!");
		
		if( AttachRotation != rot(0,0,0) )
		{
			Follower.SetRotation(AttachRotation);
		}
		else
		{
			Follower.SetRotation(PawnOwner.Rotation);
		}

		// b_MF_Attach doesn't exist on all Pawns!!
		if( AttachLocation != vect(0,0,0) )
		{
			Follower.SetLocation(AttachLocation);
		}
		else
		{
			Follower.SetLocation(PawnOwner.Location);
		}
		Follower.SetBase(PawnOwner);
	}

	// need to set PHYS_None again, because SetBase() changes physics to PHYS_Falling
	Follower.SetPhysics(PHYS_None);

	// Log all debug information.
	`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Attached Follower:" @ Follower
		@ "BaseSkelComponent:" @ Follower.BaseSkelComponent
		@ "BaseBoneName:" @ Follower.BaseBoneName
		@ "RelativeLocation:" @ Follower.RelativeLocation
		@ "RelativeRotation:" @ Follower.RelativeRotation
		@ "bHardAttach:" @ Follower.bHardAttach
		@ "bIgnoreBaseRotation:" @ Follower.bIgnoreBaseRotation, ,'SpecialMoves');

	// Shadow parent, to render only one shadow for both.
	Follower.Mesh.SetShadowParent(PawnOwner.Mesh);
}

/**
 * Detaches a based Pawn from the Leader.
 */
function DetachPawn(GearPawn APawn)
{
	local Actor	ForcedBase;

	if( APawn.ClampedBase != None )
	{
		ForcedBase = APawn.ClampedBase;
	}

	APawn.SetBase(None);
	APawn.SetHardAttach(FALSE);
	APawn.SetPhysics(PHYS_Falling);

	TogglePawnCollision(APawn, TRUE);

	// Set Forced Base
	if( ForcedBase != None )
	{
		APawn.SetBase(ForcedBase);
	}

	// Clear Shadow parent
	APawn.Mesh.SetShadowParent(None);
}

defaultproperties
{
	InteractionStartTimeOut=4.f
	FollowerSpecialMove=SM_None

	bBreakFromCover=TRUE
	bDisableMovement=TRUE
	bDisableTurnInPlace=TRUE
	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE
	bDisableLook=TRUE 
}
