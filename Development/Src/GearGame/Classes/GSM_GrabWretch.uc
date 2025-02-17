
/**
 * Wretch Grabbing test
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_GrabWretch extends GSM_BasePawnToPawnInteractionLeader
	native(SpecialMoves);

// Animations

/* Last Played Animation */
var					GearPawn.BodyStance	BS_LastPlayedAnimation;
/** Grabbing and Grabbed Idle animations */
var()				GearPawn.BodyStance	BS_GrabAnim, BS_GrabIdleAnim, BS_ThrowWretch;

/** True when leader has been synchronized with follower(s). */
var		bool		bIsSynchronized;
/** Set to TRUE if Pawn is holstering his weapon, and we're waiting for that to be done. */
var		bool		bWaitingForWeaponToBeHolstered;
/** Flag set when Follower is moving to Marker position. */
var		bool		bWaitingForFollowerToBeInPosition;
/** Flag set when Leader is moving to Marker position. */
var		bool		bWaitingForLeaderToBeInPosition;

/** Where to place follower relative to leader. */
var()	vector		AlignmentOffset;

var		Vector		LeaderLoc, FollowerLoc;
var		Rotator		FacingRot;

/** Wretch throw magnitude */
var		float		ThrowMagnitude;


/** 
 * Used for Pawn to Pawn interactions. 
 * Return TRUE if we can perform an Interaction with this Pawn.
 */
function bool CanInteractWithPawn(GearPawn OtherPawn)
{
	// Conditions for grab
	if( !OtherPawn.IsA('GearPawn_LocustWretchFriendly') ||		// Can only grab friendly wretches.
		!PawnOwner.IsSameTeam(OtherPawn) ||						// Friendly Wretch has to be in same team as us!
		OtherPawn.IsDoingASpecialMove() ||						// Friendly wretch cannot be doing a special move. (ie recovering from rag doll, being thrown etc.)
		OtherPawn.Physics == PHYS_Falling ||					// Wretch cannot be falling.
		OtherPawn.Physics == PHYS_RigidBody )					// Cannot be a rag doll
	{
		return FALSE;
	}

	return TRUE;
}


/**
 * Play a custom body stance animation.
 * Supports many features, including blending in and out.
 *
 * @param	Stance			BodyStance animation to play.
 * @param	Rate			Rate animation should be played at.
 * @param	BlendInTime		Blend duration to play anim.
 * @param	BlendOutTime	Time before animation ends (in seconds) to blend out.
 *							-1.f means no blend out.
 *							0.f = instant switch, no blend.
 *							otherwise it's starting to blend out at AnimDuration - BlendOutTime seconds.
 * @param	bLooping		Should the anim loop? (and play forever until told to stop)
 * @param	bOverride		play same animation over again only if bOverride is set to true.
 *
 * @return	PlayBack length of animation.
 */
final function float BS_Play
(
	BodyStance	Stance,
	float		Rate,
	optional	float		BlendInTime,
	optional	float		BlendOutTime,
	optional	bool		bLooping,
	optional	bool		bOverride = TRUE
)
{
	// If we have a PlayerOwner, then we can play the body stance on him
	if( PawnOwner != None )
	{
		// Keep track of animation played
		BS_LastPlayedAnimation = Stance;

		// Play animation
		return PawnOwner.BS_Play(Stance, Rate, BlendInTime, BlendOutTime, bLooping, bOverride);
	}
	else
	{
		`Log( class @ GetFuncName() @ "PawnOwner == None");
		return 0.f;
	}
}


/** 
 * Begins the interaction.
 */
function BeginInteraction()
{
	// Reset flags
	bIsSynchronized						= FALSE;
	bWaitingForWeaponToBeHolstered		= FALSE;
	bWaitingForFollowerToBeInPosition	= FALSE;
	bWaitingForLeaderToBeInPosition		= FALSE;

	// Tell Leader to holster his weapon
	bWaitingForWeaponToBeHolstered = PawnOwner.HolsterWeapon();

	// Move both Leader and Follower in position
	MoveToMarkers();

	// Check if we're ready to start interaction
	CheckIfReadyToStartInteraction();
}


/**
 * Move Leader and Follower in position.
 * Once they're where they should be, we can start the interaction.
 */
function MoveToMarkers()
{
	local GearSpecialMove	FollowerSM;

	// Figure out where actors should be placed, and which direction they should be facing
	PrePositioning_MRLeader_RFollower(PawnOwner, Follower, LeaderLoc, FollowerLoc, FacingRot);

	//`log("LeaderLoc:" @ LeaderLoc @ "FollowerLoc:" @ FollowerLoc @ "FacingRot:" @ FacingRot);

	// Rotator both
	SetFacePreciseRotation(FacingRot, 0.3f);
	FollowerSM = Follower.SpecialMoves[Follower.SpecialMove];
	FollowerSM.SetFacePreciseRotation(FacingRot, 0.3f);

	// Move them both if needed
	if( !IsZero(LeaderLoc) )
	{
		bWaitingForLeaderToBeInPosition = TRUE;
		SetReachPreciseDestination(LeaderLoc);
	}

	if( !IsZero(FollowerLoc) )
	{
		bWaitingForFollowerToBeInPosition = TRUE;
		FollowerSM.SetReachPreciseDestination(FollowerLoc);
	}

	// Disable BlockActor on Follower, so leader can get close enough.
	Follower.SetCollision(TRUE, FALSE);

	// Set a timeout in case we fail to get in position.
	PawnOwner.SetTimer( 2.f, FALSE, nameof(self.PositionTimeOut), self );
}


/** 
 * Pre-Positioning before attachment.
 * This version doesn't move the follower, but can only rotate him.
 * Leader however can move and rotate.
 */
native final function PrePositioning_MRLeader_RFollower(GearPawn ALeader, GearPawn AFollower, out Vector out_LeaderLoc, out Vector out_FollowerLoc, out Rotator out_FacingRot);


function bool MessageEvent(Name EventName, Object Sender)
{
	// Follower is letting us know that he is now in position
	if( EventName == 'FollowerInPosition' )
	{
		FollowerReachedPosition();
		return TRUE;
	}
	else if( EventName == 'FollowerReadyToBeDetached' )
	{
		FollowerReadyToBeDetached();
		return TRUE;
	}

	return Super.MessageEvent(EventName, Sender);
}


/** Leader is in Position */
event ReachedPrecisePosition()
{
	if( bWaitingForLeaderToBeInPosition )
	{
		bWaitingForLeaderToBeInPosition = FALSE;
		
		// Ensure Leader sits at exact Location
		if( !IsZero(LeaderLoc) )
		{
			LeaderLoc.Z = PawnOwner.Location.Z;
			PawnOwner.SetLocation(LeaderLoc);
		}

		// See if we're ready to start now.
		CheckIfReadyToStartInteraction();
	}
}


/** Follower is in Position. */
function FollowerReachedPosition()
{
	if( bWaitingForFollowerToBeInPosition )
	{
		bWaitingForFollowerToBeInPosition = FALSE;

		// Ensure Follower is at the exact location.
		FollowerLoc.Z = Follower.Location.Z;
		Follower.SetLocation(FollowerLoc);

		// See if we're ready to start.
		CheckIfReadyToStartInteraction();
	}
}


/** 
 * TimeOut if position can't be resolved, we resort to having an ugly transition rather than blocking the player.
 */
function PositionTimeOut()
{
	if( bWaitingForFollowerToBeInPosition )
	{
		`log(class @ GetFuncName() @ "Couldn't resolve Follower position in time. :(");
		FollowerReachedPosition();
	}

	if( bWaitingForLeaderToBeInPosition )
	{
		`log(class @ GetFuncName() @ "Couldn't resolve Leader position in time. :(");
		ReachedPrecisePosition();
		
		// Cancel movement.
		SetReachPreciseDestination(vect(0,0,0), FALSE);
	}
}


/*
function Tick(float DeltaTime)
{
	local Vector MarkerLoc, X, Y, Z;

	// Marcus has a bone used a location marker to place the wretch.
	//MarkerLoc = PawnOwner.Mesh.GetBoneLocation('b_MF_Attach');

	// @fixme laurent - can't get it early enough since it's baked in the grab animation.
	// so i extract it from there, and hardcode it, so it can be used before actually playing the grab animation.
	GetAxes(PawnOwner.Rotation, X, Y, Z);
	MarkerLoc = PawnOwner.Location + X * AlignmentOffset.X + Y * AlignmentOffset.Y + Z * AlignmentOffset.Z;

	// Draw AlignmentOffet for debugging
	PawnOwner.DrawDebugCoordinateSystem(MarkerLoc, PawnOwner.Rotation, 25.f, FALSE);

	Super.Tick(DeltaTime);
}
*/

/** Notification called when weapon has been temporarily holstered. */
function WeaponTemporarilyHolstered()
{
	// if we were holstering our weapon
	if( bWaitingForWeaponToBeHolstered )
	{
		// then we can clear the flag, we're done with that
		bWaitingForWeaponToBeHolstered = FALSE;

		// Check if we're ready to start
		CheckIfReadyToStartInteraction();
	}
}


/**
 * Checks if leader is ready to start the synchronized animation.
 */
function CheckIfReadyToStartInteraction()
{
	// If we're holstering our weapon we're not ready to start.
	if( bIsSynchronized || bWaitingForWeaponToBeHolstered || bWaitingForFollowerToBeInPosition || bWaitingForLeaderToBeInPosition )
	{
		return;
	}

	//`log("LeaderLoc:" @ PawnOwner.Location @ "FollowerLoc:" @ Follower.Location @ "LeaderRot:" @ PawnOwner.Rotation @ "FollowerRot:" @ Follower.Rotation);

	// We're ready, start!
	bIsSynchronized = TRUE;

	// We can start the interaction now.
	InteractionStarted();
}


/** 
 * Leader and Follower are ready to start the interaction.
 * (They moved in postion, are in the correct states...)
 */
function InteractionStarted()
{
	// Start Follower synched animation
	if( !Follower.SpecialMoveMessageEvent('FollowerStartAnim', Self) )
	{
		`log(class @ GetFuncName() @ "MessageEvent FollowerStartAnim not processed by InteractionPawn!!" @ Follower);
	}

	// if Pawn was holstering weapon, stop the holstering animations
	if( PawnOwner.IsWeaponHolstered() )
	{
		PawnOwner.StopHolsterAnims();
	}

	// Have Pawn play grabbing animation on full body
	BS_Play(BS_GrabAnim, SpeedModifier, 0.2f/SpeedModifier, -1.f, FALSE, TRUE);

	// Set up notify to get call back when animation is done playing
	PawnOwner.BS_SetAnimEndNotify(BS_GrabAnim, TRUE);

	// Turn On IK once we're fully blended to the new animation. (We need bone marker to correct location)
	PawnOwner.SetTimer( 0.25f/SpeedModifier, FALSE, nameof(self.TurnOnHandIK), self );

	// Authorize weapon firing notifications.
	bForwardFirePressedNotification = TRUE;

	// Function to get MarkerLoc Offset from grab animation
	// As we can't get it before we play the animation, but we need it before to align both characters :(
	//PawnOwner.SetTimer( 0.4f, FALSE, nameof(self.GetMarkerLoc), self );

	// Test: Attach wretch on marker for perfect synchronization.
	// Used to compare with or without.
	//PawnOwner.SetTimer( 0.25f/SpeedModifier, FALSE, nameof(self.Test_AttachWretchToMarker), self );
}


function Test_AttachWretchToMarker()
{
	local Vector	WretchOffset, ActorSpaceOffset;

	Follower.SetPhysics(PHYS_None);
	// Turn off blocking collision
	Follower.SetCollision(TRUE, FALSE);
	// Disable world collision
	Follower.bCollideWorld = FALSE;

	WretchOffset		= Follower.Mesh.GetBoneLocation('Wretch_Root') - Follower.Location;
	ActorSpaceOffset	= GetActorSpaceOffset(PawnOwner, WretchOffset);

	Follower.SetBase(PawnOwner,, PawnOwner.Mesh, 'MarkerA');
	Follower.SetRelativeLocation(Follower.RelativeLocation - ActorSpaceOffset);
}


/** 
 * Turn a WorldSpaceOffset into an ActorSpaceOffset.
 */
final function vector GetActorSpaceOffset(Actor A, Vector WorldSpaceOffset)
{
	local vector X, Y, Z, ActorSpaceOffset;

	GetAxes(A.Rotation, X, Y, Z);

	ActorSpaceOffset.X = WorldSpaceOffset dot X;
	ActorSpaceOffset.Y = WorldSpaceOffset dot Y;
	ActorSpaceOffset.Z = WorldSpaceOffset dot Z;

	return ActorSpaceOffset;
}


/** 
 * Utility function to get relative offset from Marker location.
 * The problem is that the location is known only after the grab animation has started playing.
 * And we need the marker location before to we can align both characters prior to playing
 * the synched animation.
 * So this function dumps the relative offset, that we can hardcode and use earlier...
 */
function GetMarkerLoc()
{
	local Vector MarkerLoc;
`if(`notdefined(FINAL_RELEASE))
	local vector RelativeOffset, WretchLoc;
`endif
	// Get position of marker
	MarkerLoc = PawnOwner.Mesh.GetBoneLocation('b_MF_Attach');

	// Dump a little debug info to display the location we got:
	PawnOwner.DrawDebugSphere(MarkerLoc, 4, 8, 255, 0, 255, TRUE);

`if(`notdefined(FINAL_RELEASE))
	// Transform marker location to be relative to Leader location and rotation.
	RelativeOffset = GetActorSpaceOffset(PawnOwner, MarkerLoc - PawnOwner.Location);
	`log(GetFuncName() @ "RelativeOffset:" @ RelativeOffset);

	// Get default offset, on flat ground, from marker to wretch.
	WretchLoc = Follower.Mesh.GetBoneLocation('Wretch_Root');
	`log(GetFuncName() @ "WretchLoc:" @ WretchLoc @ "MarkerLoc" @ MarkerLoc @ "Wretch Marker Offset:" @ (WretchLoc - MarkerLoc));
`endif
}


/** Notification called when an Animation is done playing */
function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	// If playing grabbing animation
	if( PawnOwner.BS_IsPlaying(BS_GrabAnim) )
	{
		// Grab Synched anim is finished, we can now attach follower to leader.
		LeaderGrabFollower();
	}
	else
	{
		Super.BS_AnimEndNotify(SeqNode, PlayedTime, ExcessTime);
	}
}


/** 
 * Grab Animation is finished.
 * Attach follower to Leader.
 */
function LeaderGrabFollower()
{
	local GearPC	PC;

	// Turn off full body grab animation
	PawnOwner.BS_Stop(BS_GrabAnim, 0.2f);

	// Turn off IKs
	TurnOffHandIK();

	// Perform attachment
	AttachFollowerToLeader();

	// Let follower know he's been attached
	if( !Follower.SpecialMoveMessageEvent('AttachedToLeader', Self) )
	{
		`log(class @ GetFuncName() @ "MessageEvent AttachedToLeader not processed by InteractionPawn!!" @ Follower);
	}

	// Play Grabbed idle animation on upper body
	BS_Play(BS_GrabIdleAnim, SpeedModifier, 0.f, 0.f, TRUE, TRUE);

	// Enable movement on leader
	PC = GearPC(PawnOwner.Controller);
	if( PC != None )
	{
		PC.IgnoreMoveInput(FALSE);
		PC.IgnoreLookInput(FALSE);
	}
}


/** 
 * Attach Follower to Leader's mesh.
 */
function AttachFollowerToLeader()
{
	Follower.SetPhysics(PHYS_None);
	// Turn off blocking collision
	Follower.SetCollision(TRUE, FALSE);
	Follower.SetBase(PawnOwner,, PawnOwner.Mesh, 'GrabHandle');
	//Follower.SetBase(PawnOwner);
}

/** 
 * Use Right Hand IK to compensate for height differences.
 * We've moved the actors on the marker locations. X and Y and pretty close, but Z is very likely wrong.
 * So we get the offset from the FollowerLocation (Wretch) to the Marker location,
 * and use that on the Leader's (Marcus) Right Hand IK to compensate for that difference, and achieve perfect alignement.
 */
function TurnOnHandIK()
{
	//local Vector MarkerOffset, ActorSpaceOffset;

	//if( FALSE )
// 	if( PawnOwner.BoneCtrl_GripOffset != None )
// 	{
// 		// Figure out the offset from marker to wretch origin.
// 		// This is what we need to compensate for.
// 		MarkerOffset = Follower.Mesh.GetBoneLocation('Wretch_Root') - PawnOwner.Mesh.GetBoneLocation('b_MF_Attach');
// 
// 		// Turn WorldSpace offset to ActorSpace
// 		ActorSpaceOffset = GetActorSpaceOffset(PawnOwner, MarkerOffset);
// 		//`log(GetFuncName() @ "ActorSpaceOffset:" @ ActorSpaceOffset);
// 		
// 		// Assign Actor Space offset to grip bone. The IK Hand Controller will be attached to that bone.
// 		PawnOwner.BoneCtrl_GripOffset.BoneTranslation = ActorSpaceOffset;
// 		PawnOwner.BoneCtrl_GripOffset.SetSkelControlActive(TRUE);
// 	}

	// Set up IK from HandBone to follow adjusted GripBone.
	//if( FALSE )
	if( PawnOwner.IKCtrl_RightHand != None )
	{
		PawnOwner.IKCtrl_RightHand.EffectorLocationSpace = BCS_OtherBoneSpace;
		PawnOwner.IKCtrl_RightHand.EffectorSpaceBoneName = 'b_MF_Grip';
		PawnOwner.IKCtrl_RightHand.SetSkelControlActive(TRUE);
	}
}


/** 
 * Turn off IK.
 */
function TurnOffHandIK()
{
	if( PawnOwner.IKCtrl_RightHand != None )
	{
		PawnOwner.IKCtrl_RightHand.BlendOutTime = 0.3f;
		PawnOwner.IKCtrl_RightHand.SetSkelControlActive(FALSE);
	}

// 	if( PawnOwner.BoneCtrl_GripOffset != None )
// 	{
// 		PawnOwner.BoneCtrl_GripOffset.BlendOutTime = 0.3f;
// 		PawnOwner.BoneCtrl_GripOffset.SetSkelControlActive(FALSE);
// 	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local Vector	X, Y, Z;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// If we were playing an animation, turn it off
	if( PawnOwner.BS_IsPlaying(BS_LastPlayedAnimation) )
	{
		PawnOwner.BS_Stop(BS_LastPlayedAnimation, 0.2f);
		PawnOwner.BS_SetAnimEndNotify(BS_LastPlayedAnimation, FALSE);
	}

	// Make sure IK are turned off, if we have aborted too early.
	TurnOffHandIK();

	// If weapon is holstered, unholster it
	if( PawnOwner.IsWeaponHolstered() )
	{
		PawnOwner.UnHolsterWeapon();
	}

	// If the follower has been attached, detach it.
	if( Follower.IsBasedOn(PawnOwner) )
	{
		// Detach Follower from Leader.
		DetachPawn(Follower);

		// Add a little velocity to move the wretch off us.
		GetAxes(PawnOwner.Rotation, X, Y, Z);
		Follower.AddVelocity( X * 20 + Y * 100, Follower.Location, None);
	}
}


/** 
 * @note: Currently this is called on the owning client, fix this to be replicated to the server.
 */
function FirePressedNotification()
{
	// Follower.EndSpecialMove();
	// PawnOwner.EndSpecialMove();

	// Disable fire pressed notification.
	bForwardFirePressedNotification = FALSE;

	// Throw Wretch
	ThrowWretch();
}


/**
 * Throw Wretch.
 */
function ThrowWretch()
{
	// Blend out from previously played animation
	PawnOwner.BS_Stop(BS_LastPlayedAnimation, 0.2f);

	// Play Throw animation
	BS_Play(BS_ThrowWretch, SpeedModifier, 0.2f, 0.f, FALSE, TRUE);
	
	// Set up notify to get call back when animation is done playing
	PawnOwner.BS_SetAnimEndNotify(BS_ThrowWretch, TRUE);

	// Start Synched Animation on Follower
	// Start Follower synched animation
	if( !Follower.SpecialMoveMessageEvent('FollowerThrown', Self) )
	{
		`log(class @ GetFuncName() @ "MessageEvent FollowerThrown not processed by InteractionPawn!!" @ Follower);
	}
}


/** 
 * Called when Wretch is ready to be detached from marcus.
 */
function FollowerReadyToBeDetached()
{
	local Rotator	NewRotation;

	Follower = Follower;

	// Detach Wretch
	DetachPawn(Follower);

	// Make follower match leader rotation. Because of SetBase, Follower is completely messed up and matching the socket/animation.
	NewRotation = PawnOwner.Rotation;
	NewRotation.Pitch = 0;

	Follower.SetRotation(NewRotation);

	// Add velocity boost
	Follower.AddVelocity(Vector(PawnOwner.Rotation) * ThrowMagnitude, Vect(0,0,0), None);
	// Put him in a special state to he can detect collision and rag doll properly.
	Follower.GotoState('GRABTEST_Thrown');
}

defaultproperties
{
	bDisableMovement=TRUE
	bDisableLook=TRUE

	FollowerSpecialMove=SM_GrabWretchFollower
	AlignmentOffset=(X=-5.56,Y=+58.76,Z=-74.03)
	ThrowMagnitude=1000.f

	BS_GrabAnim=(AnimName[BS_FullBody]="PickUp_Wretch")
	BS_GrabIdleAnim=(AnimName[BS_Std_Up]="PickUp_Wretch_idle")
	BS_ThrowWretch=(AnimName[BS_Std_Up]="Throw_Wretch",AnimName[BS_Std_Idle_Lower]="Throw_Wretch")

	Action={(
		ActionName=GrabWretch,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=284,V=346,UL=104,VL=91)))	),
	)}
}
