
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearSpecialMove extends Object
	config(Pawn)
	native(SpecialMoves)
	abstract;

/** GoW global macros */

/** GearPawn owning this special move */
var					GearPawn	PawnOwner;
var					GearPC		PCOwner;
var					GearAI		AIOwner;

/** Default AICommand to push on AI when SpecialMove starts */
var class<AICmd_Base_PushedBySpecialMove>	DefaultAICommand;
var	AICmd_Base_PushedBySpecialMove			AICommand;

/** Running speed/animation play rate modifier */
var() const config	float	SpeedModifier;
/** Speed/animation play rate modifier when carrying a heavy weapon */
var() const config	float	HeavyWeaponSpeedModifier;
/** Gravity Scaling for PawnOwner */
var					float	GravityScale;

/** Is Player allowed to fire while doing this special move? */
var	const 			bool	bCanFireWeapon;

/** Damage scale while performing this special move */
var const config	float	DamageScale;

/**
 * For SpecialMove interactions. Only InteractionPawn allowed to damage me.
 * Note: this is reset to FALSE in SpecialMoveStarted.
 */
var					bool	bOnlyInteractionPawnCanDamageMe;

/**
 * Should abort weapon reload. If using body stances,
 * then should most likely abort weapon reloads to avoid any collision.
 */
var					bool	bShouldAbortWeaponReload;

/** Lock Pawn Rotation, prevent it from being affects by Controller.Rotation */
var	const			bool	bLockPawnRotation;
var private			bool	bPawnRotationLocked;

/** Disable movement (but doesn't necessarily stop physics, just blocks inputs). */
var	const			bool	bDisableMovement;
var private			bool	bMovementDisabled;

/** If TRUE, disables turning in place animations & code. Pawn will always face his rotation. */
var const			bool	bDisableTurnInPlace;
/** Breaks the player from cover upon starting the move */
var	const			bool	bBreakFromCover;
/** Breaks the player from cover upon ending the move */
var const			bool	bBreakFromCoverOnEnd;
var const			bool	bDisableLook;
var	const			bool	bDisableCollision;
var	const			bool	bDisablePhysics;
var const			bool	bDisableLeftHandIK;
var const			bool	bDisablePOIs;

/** Does this SpecialMove bypass Stopping Power? Usually for collision sensitive moves, like jumping over cover. */
var					bool	bNoStoppingPower;

/** TRUE if this is a cover exit mirror transition */
var const			bool	bCoverExitMirrorTransition;

/**
 * If TRUE, special move will trigger OnMirrorTransitionSafeNotify() when it's safe to play an animation not interfering with mirror transitions.
 * This will be triggered once, and is used to delay mirror transitions played from special moves, so they don't overlap with other mirror transitions
 * and produce ugly results.
 */
var 				bool	bMirrorTransitionSafeNotify;

/** TRUE == Have camera focus on Pawn's head */
var	const			bool	bCameraFocusOnPawn;

/** AI should ignore notifications */
var	const			bool	bDisableAI;

/** Generic align to functionality, see GearPawn.FaceRotation */
var 				Actor	AlignToActor;

/**
 * All of our special moves can modify the motion blur values to get that little bit extra goodness
 **/
var const	config	float	MotionBlurAmount;

/** HUD action info associated with this special move */
var const	ActionInfo		Action;

/**
 * Last time CanDoSpecialMove was called.
 */
var transient float LastCanDoSpecialMoveTime;

/**
 * Can we do the current special move?
 */
var private bool bLastCanDoSpecialMove;

/** Flag used when moving Pawn to a precise location */
var const	bool	bReachPreciseDestination;
/** Flag set when Pawn reached precise location */
var	const	bool	bReachedPreciseDestination;
/** World location to reach */
var const	vector	PreciseDestination;
var const	Actor	PreciseDestBase;
var const	vector	PreciseDestRelOffset;

/** Flag used when rotating pawn to a precise rotation */
var const	bool	bReachPreciseRotation;
/** Flag set when pawn reached precise rotation */
var const	bool	bReachedPreciseRotation;
/** Time to interpolate Pawn's rotation */
var const	float	PreciseRotationInterpolationTime;
/** World rotation to face */
var const	Rotator	PreciseRotation;

/** PrecisePosition will not be enforced on non-owning instance unless this flag is set */
var bool	bForcePrecisePosition;

/**
 * Notify Special Move when fire button is pressed. Even if the pawn can't fire his weapon.
 * Calls FirePressedNotification().
 */
var	bool	bForwardFirePressedNotification;

/** AI: if TRUE, CheckInterruptCombatTransitions() will be called from WarAI_Drone::NotifyEndOfSpecialMove() */
var bool bCheckForGlobalInterrupts;

/** Used for shot scoring, how much does this active special count towards the overall score? */
var config const float SceneRatingValue;

/** TRUE if playing a camera anim */
var protected bool		bPlayingCameraAnim;

/////////////// Surface Orienatation

/** If during this special move we should modify the rotation of the Mesh Component to match the floor normal. */
var const bool		bConformMeshRotationToFloor;
/** What the maximum amount of rotation we will apply to match the floor rotation is, in degrees */
var const float		MaxConformToFloorMeshRotation;
/** If during this special move we should modify the translation of the Mesh Component to match the floor better. */
var const bool		bConformMeshTranslationToFloor;
/** What the maximum amount of translation we will apply to better match the floor slope */
var const float		MaxConformToFloorMeshTranslation;


// when this is enabled old movement vars (accel/vel) will be restored to the pawn once the move is finished
// this is useful for moves that have root motion enabled to restore movement/lookdirection to pawns after the translation
// is complete
var bool bRestoreMovementAfterMove;
var BasedPosition OldFocalPoint;
var vector OldAccel;

// C++ functions
cpptext
{
	virtual void PrePerformPhysics(FLOAT DeltaTime);
	virtual void PostProcessPhysics(FLOAT DeltaTime) {}
	virtual void TickSpecialMove(FLOAT DeltaTime);
}

/**
 * Can the special move be chained after the current one finishes?
 */
function bool CanChainMove(ESpecialMove NextMove)
{
	return FALSE;
}

/**
 * Can a new special move override this one before it is finished?
 * This is only if CanDoSpecialMove() == TRUE && !bForce when starting it.
 */
function bool CanOverrideMoveWith(ESpecialMove NewMove)
{
	return FALSE;
}

/**
 * Can this special move override InMove if it is currently playing?
 */
function bool CanOverrideSpecialMove(ESpecialMove InMove)
{
	return FALSE;
}

/**
 * Public accessor to see if this special move can be done, handles
 * caching the results for a single frame.
 * @param bForceCheck - Allows you to skip the single frame condition (which will be incorrect on clients since LastCanDoSpecialMoveTime isn't replicated)
 */
final function bool CanDoSpecialMove( optional bool bForceCheck )
{
	if( PawnOwner != None )
	{
		// update the cached value if outdated
		if( bForceCheck || PawnOwner.WorldInfo.TimeSeconds != LastCanDoSpecialMoveTime )
		{
			bLastCanDoSpecialMove		= InternalCanDoSpecialMove();
			LastCanDoSpecialMoveTime	= PawnOwner.WorldInfo.TimeSeconds;
		}
		// return the cached value
		return bLastCanDoSpecialMove;
	}

	return FALSE;
}


/**
 * Used for Pawn to Pawn interactions.
 * Return TRUE if we can perform an Interaction with this Pawn.
 */
function bool CanInteractWithPawn(GearPawn OtherPawn)
{
	return FALSE;
}


/**
 * Checks to see if this Special Move can be done.
 */
protected function bool InternalCanDoSpecialMove()
{
	return TRUE;
}

/**
 * Event called when Special Move is started.
 */
function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	`LogSMExt(PawnOwner,"");

	PCOwner = GearPC(PawnOwner.Controller);
	AIOwner = GearAI(PawnOwner.Controller);

	if(AIOwner == none && PawnOwner.DrivenVehicle != none)
	{
		AIOwner = GearAI(PawnOwner.DrivenVehicle.Controller);
	}

	// Reset variables
	bPlayingCameraAnim = FALSE;
	bOnlyInteractionPawnCanDamageMe = FALSE;

	// Push AICommand if it is defined.
	//`log(GetFuncName()@self@AIOwner != None @ DefaultAICommand != None @ (AIOwner.GetActiveCommand() == none || AIOwner.GetActiveCommand().AllowPushOfDefaultCommandForSpecialMove(PawnOwner.SpecialMove)));
	if( AIOwner != None && DefaultAICommand != None && (AIOwner.GetActiveCommand() == none || AIOwner.GetActiveCommand().AllowPushOfDefaultCommandForSpecialMove(PawnOwner.SpecialMove)))
	{
		AICommand = DefaultAICommand.static.PushSpecialMoveCommand(AIOwner);
		if( AICommand == None )
		{
			`Warn(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Failed to push AICommand:" @ DefaultAICommand);
		}
	}

	if( PCOwner != None )
	{
		if( bDisableLook )
		{
			PCOwner.IgnoreLookInput(TRUE);
		}
		// clear the local special move icon
		if (PCOwner.MyGearHud != None)
		{
			PCOwner.MyGearHud.ClearActionInfoByType(AT_SpecialMove);
		}
	}

	if( bRestoreMovementAfterMove )
	{
		if(AIOwner != none)
		{
			OldFocalPoint = AIOwner.FocalPosition;
		}
		OldAccel = PawnOwner.Acceleration;
	}

	// Special Moves and Weapon reloads use the body stance animation and notification system.
	// So to prevent any conflict, special moves abort weapon reloads
	if( default.bShouldAbortWeaponReload )
	{
		bShouldAbortWeaponReload = default.bShouldAbortWeaponReload;
		if( PawnOwner.MyGearWeapon != None && PawnOwner.IsReloadingWeapon() )
		{
			PawnOwner.MyGearWeapon.AbortWeaponReload();
		}
	}

	// If movement is disabled set acceleration to zero.
	if( bDisableMovement )
	{
		SetMovementLock(TRUE);
	}

	// If Pawn rotation should be locked, make sure it is.
	if( default.bLockPawnRotation )
	{
		SetLockPawnRotation(TRUE);
	}

	if( bDisableTurnInPlace )
	{
		PawnOwner.bCanDoTurnInPlaceAnim = FALSE;
	}

	// Disable Pawn collision
	if( bDisableCollision )
	{
		TogglePawnCollision(PawnOwner, FALSE);
	}

	if( bDisablePhysics )
	{
		PawnOwner.ZeroMovementVariables();
		if( PawnOwner.Role == ROLE_Authority )
		{
			PawnOwner.SetPhysics(PHYS_None);
			PawnOwner.SetBase( PawnOwner.ClampedBase );
		}
	}

	// Reset to default value. Because turned off when notify is triggered.
	bMirrorTransitionSafeNotify = default.bMirrorTransitionSafeNotify;

	if( bBreakFromCover && !bMirrorTransitionSafeNotify )
	{
		BreakFromCover();
	}

	// See if we should be notifying special move that we're mirror transition safe
	if( bMirrorTransitionSafeNotify && IsMirrorTransitionSafe() )
	{
		OnMirrorTransitionSafeNotifyInternal();
	}
}

/**
 * Called to break player from cover.
 * If bMirrorTransitionSafeNotify is set, then break from cover is called from that event instead of StartSpecialMove().
 */
final function BreakFromCover()
{
	if( PCOwner != None )
	{
		// break for both client/server since the move will always be simulated by the owning client
		if (PCOwner.Role == ROLE_Authority && !PCOwner.IsLocalPlayerController())
		{
			// for servers break immediately instead of waiting till the next tick
			PCOwner.LeaveCover();
		}
		else
		{
			PCOwner.bBreakFromCover = TRUE;
		}
	}
	// don't call InvalidateCover() if the AI isn't actually in the cover, as that will invalidate its current claim
	// which it may be using this special move to get to
	else if (PawnOwner.CurrentLink != None && GearAI_Cover(AIOwner) != None)
	{
		GearAI_Cover(AIOwner).InvalidateCover();
	}
}

/**
 * See if we could play a mirror transition right now.
 * Make sure it won't collide with one playing currently and produce something ugly.
 */
event bool IsMirrorTransitionSafe()
{
	if( PawnOwner.MirrorNode != None )
	{
		`LogSMExt(PawnOwner, "PawnOwner:" @ PawnOwner @ "bDoingMirrorTransition:" @ PawnOwner.bDoingMirrorTransition @ "bBlendingOut:" @ PawnOwner.MirrorNode.bBlendingOut);
	}
	return( PawnOwner == None || !PawnOwner.bDoingMirrorTransition || PawnOwner.MirrorNode == None || PawnOwner.MirrorNode.bBlendingOut );
}

/**
 * Notification called when we're mirror transition safe.
 * Internal. Use OnMirrorTransitionSafeNotify() to implement your code.
 */
final event OnMirrorTransitionSafeNotifyInternal()
{
	`LogSMExt(PawnOwner,"");

	// Turn off flag to check checking in native code.
	bMirrorTransitionSafeNotify = FALSE;

	// If break from cover, do it here
	if( bBreakFromCover )
	{
		BreakFromCover();
	}

	// Call script version, to avoid making all classes implementing this native.
	OnMirrorTransitionSafeNotify();
}

/** Notification called when we're mirror transition safe. */
function OnMirrorTransitionSafeNotify();

/**
 * Event called when Special Move is finished.
 */
function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	`LogSMExt(PawnOwner,"");

	// clear the align for the next use
	AlignToActor = None;

	// AI set special move command.
	if( AIOwner != None && DefaultAICommand != None && AICommand != None )
	{
		if( AIOwner.GetActiveCommand() == AICommand )
		{
			AICommand.Status = 'Success';
			AIOwner.PopCommand(AICommand);
		}
		// Clear reference.
		AICommand = None;
	}

	if( PCOwner != None )
	{
		if( bPlayingCameraAnim )
		{
			PCOwner.MainPlayerInput.ForcePitchCentering(FALSE);
		}
		if( bDisableLook )
		{
			PCOwner.IgnoreLookInput(FALSE);
		}
	}

	if( bRestoreMovementAfterMove )
	{
		if(AIOwner != none)
		{
			AIOwner.FocalPosition = OldFocalPoint;
		}
		PawnOwner.Acceleration = OldAccel;
	}

	// If collision was disabled, restore it
	if( bDisableCollision )
	{
		TogglePawnCollision(PawnOwner, TRUE);
	}

	if( bDisablePhysics )
	{
		if( PawnOwner.Role == ROLE_Authority )
		{
			PawnOwner.SetPhysics(PHYS_Falling);
		}		
	}

	// If movement was disabled, toggle it back on
	if( bMovementDisabled )
	{
		SetMovementLock(FALSE);
	}

	// If Pawn rotation was locked in default properties, unlock it.
	// Otherwise we're in manual mode, so let the special subclass handle everything.
	if( bPawnRotationLocked )
	{
		SetLockPawnRotation(FALSE);
	}

	if( bDisableTurnInPlace )
	{
		PawnOwner.bCanDoTurnInPlaceAnim = PawnOwner.default.bCanDoTurnInPlaceAnim;
	}

	if ( bBreakFromCoverOnEnd )
	{
		BreakFromCover();
	}

}


/** Script Tick function. */
function Tick(float DeltaTime);

/** Affect movement speed and animation play rate. By default returns SpeedModifier. */
native function float GetSpeedModifier();

/**
 * Generic function to send message events to SpecialMoves.
 * Returns TRUE if message has been processed correctly.
 */
function bool MessageEvent(Name EventName, Object Sender)
{
	`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Received unhandled event!" @ EventName @ "from:" @ Sender);
	if( GearPawn(Sender) != None )
	{
		`log(" SpecialMove:" @ GearPawn(Sender).SpecialMove);
	}
	ScriptTrace();

	return FALSE;
}


/**
 * Call from Input system. Gives a chance to read or modify controller input.
 */
function PreProcessInput(GearPlayerInput Input);

/** For double click functionality such as roadie run */
simulated function PreDoubleClickCheck( GearPlayerInput_Base Input );

/**
 * Controller Button Presses notification.
 * Return TRUE to trap input. (=Override this button press)
 */
function bool ButtonPress(Name ButtonName)
{
	return FALSE;
}

/** Notification forwarded by GearAnimNotify_SpecialMove */
function AnimNotify(AnimNodeSequence SeqNode, GearAnimNotify_SpecialMove NotifyObject);

/**
 * Notification called when body stance animation finished playing.
 * @param	SeqNode		- Node that finished playing. You can get to the SkeletalMeshComponent by looking at SeqNode->SkelComponent
 * @param	PlayedTime	- Time played on this animation. (play rate independant).
 * @param	ExcessTime	- how much time overlapped beyond end of animation. (play rate independant).
 */
function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	// By default end this special move.
	`LogSMExt(PawnOwner, GetFuncName() @ Self @ "calling EndSpecialMove()");
	PawnOwner.EndSpecialMove();
}


/**
 * Send Pawn to reach a precise destination.
 * ReachedPrecisePosition() event will be called when Pawn reaches destination.
 * This tries to get the Pawn as close as possible from DestinationToReach in 2D space (so Z is ignored).
 * This doesn't use the path network, so PawnOwner should be already fairly close to destination.
 * A TimeOut should be used to prevent the Pawn from being stuck.
 * @param	DestinationToReach	point in world space to reach. (Z ignored).
 * @param	bCancel				if TRUE, this will cancel any current PreciseDestination movement.
 */
native final function SetReachPreciseDestination(vector DestinationToReach, optional bool bCancel);


/**
 * Force Pawn to face a specific rotation.
 * @param	RotationToFace		Rotation for Pawn to face.
 * @param	InterpolationTime	Time it takes for Pawn to face given rotation.
 */
native final function SetFacePreciseRotation(rotator RotationToFace, float InterpolationTime);

/**
 * Event sent when Pawn has reached precise position.
 * PreciseRotation or PreciseLocation, or Both.
 * When both Rotation and Location are set, the event is fired just once,
 * after the Pawn has reached both.
 */
event ReachedPrecisePosition();


/**
 * Toggle Pawn collision.
 */
final function TogglePawnCollision(GearPawn APawn, bool bToggleOn)
{
	local Vector ColCompOffset, OriginalLocation, ZCheck, ColLocation, Out_HitLoc, Out_HitNormal, CollisionAdjustedOffset, Delta2D;

	if( bToggleOn )
	{
		// Make sure we're going to change anything before calling
		// some expensive functions.
		if( APawn.bCollideWorld != APawn.default.bCollideWorld ||
			APawn.bBlockActors != APawn.default.bBlockActors )
		{
			// Save original pawn location
			OriginalLocation = APawn.Location;

			APawn.SetCollision(APawn.bCollideActors, APawn.default.bBlockActors);
			APawn.bCollideWorld = APawn.default.bCollideWorld;

			// Make sure collision cylinder fits in the world
			APawn.FitCollision();

			// See if we moved and need to perform some adjustments
			CollisionAdjustedOffset = APawn.Location - OriginalLocation;
			ColCompOffset			= APawn.CollisionComponent != None ? APawn.CollisionComponent.Translation : vect(0,0,0);
			if( VSizeSq(CollisionAdjustedOffset) > 0.0001f )
			{
				// When collision is restored, the game tries to fit the collision cylinder in the world,
				// and that can create a pop, and place the cylinder too high.
				// So we're moving the cylinder back against the ground, and offseting the mesh's translation to create a smooth transition.

				// First try to move the pawn as close as possible to the original location in 2d space
				Delta2D		= CollisionAdjustedOffset;
				Delta2D.Z	= 0.f;
				if( VSizeSq(Delta2D) > 0.0001f )
				{
					ColLocation = APawn.Location + ColCompOffset;
					if( APawn.Trace(Out_HitLoc, Out_HitNormal, ColLocation - Delta2D, ColLocation, TRUE, APawn.GetCollisionExtent(),,APawn.TRACEFLAG_Blocking) != None )
					{
						APawn.SetLocation(Out_HitLoc - ColCompOffset);
					}
				}
			}

			// Now move Pawn down to the floor
			ColLocation = APawn.Location + ColCompOffset;
			ZCheck		= Vect(0,0,1) * APawn.CylinderComponent.CollisionHeight;

			if( APawn.Trace(Out_HitLoc, Out_HitNormal, ColLocation - ZCheck, ColLocation + ZCheck, TRUE, APawn.GetCollisionExtent(),,APawn.TRACEFLAG_Blocking) != None )
			{
				APawn.SetLocation(Out_HitLoc - ColCompOffset);
			}

			// This is the displacement caused by adjusting collision.
			CollisionAdjustedOffset = APawn.Location - OriginalLocation;

			// Offset Pawn's mesh by the opposite of that offset, to remove the pop and create a seemless transition.
			APawn.SetMeshTranslationOffset(APawn.MeshTranslationOffset - CollisionAdjustedOffset);
		}
	}
	else
	{
		APawn.SetCollision(APawn.bCollideActors, FALSE);
		APawn.bCollideWorld = FALSE;
	}
}

/** Locks or Unlocks Pawn movement */
final function SetMovementLock(bool bEnable)
{
	if( bMovementDisabled != bEnable )
	{
		bMovementDisabled = bEnable;

		if( PCOwner != None )
		{
			PCOwner.IgnoreMoveInput(bEnable);
		}

		// Set acceleration to zero
		if( bEnable )
		{
			PawnOwner.Acceleration = Vect(0,0,0);
		}
	}
}

/**
 * Locks or UnLocks Pawn rotation.
 * When Pawn rotation is locked, Pawn will stop rotating to match the controller's Yaw.
 */
final function SetLockPawnRotation(bool bLock)
{
	if( bPawnRotationLocked != bLock )
	{
		bPawnRotationLocked = bLock;

		// If we were locked, then interpolate back to new (unlocked) rotation
		if( !bLock )
		{
			PawnOwner.InterpolatePawnRotation();
		}
	}
}


/** accessor function to read status of bLockPawnRotation. Since it's made private. */
final function bool IsPawnRotationLocked()
{
	return bPawnRotationLocked;
}

/** Forces Pawn's rotation to a given Rotator */
final native function ForcePawnRotation(Pawn P, Rotator NewRotation);

/**
 * Notification that root motion mode changed.
 * Called only from SkelMeshComponents that have bRootMotionModeChangeNotify set.
 * This is useful for synchronizing movements.
 * For intance, when using RMM_Translate, and the event is called, we know that root motion will kick in on next frame.
 * It is possible to kill in-game physics, and then use root motion seemlessly.
 */
function RootMotionModeChanged(SkeletalMeshComponent SkelComp);


/**
 * Notification called after root motion has been extracted, and before it's been used.
 * This notification can be used to alter extracted root motion before it is forwarded to physics.
 * It is only called when bRootMotionExtractedNotify is TRUE on the SkeletalMeshComponent.
 * @note: It is fairly slow in Script, so enable only when really needed.
 */
function RootMotionExtracted(SkeletalMeshComponent SkelComp, out BoneAtom ExtractedRootMotionDelta);

/** Notification called when weapon has been temporarily holstered. */
function WeaponTemporarilyHolstered();

/**
 * Notify Special Move when fire button is pressed. Even if the pawn can't fire his weapon.
 * Called when bForwardFirePressedNotification is set.
 */
function FirePressedNotification();

/** Should this special move be replicated to non-owning clients? */
function bool ShouldReplicate()
{
	// by default all moves get replicated via GearPawn.ReplicatedSpecialMove
	return TRUE;
}


/**
* Play a spatialized custom camera shake on all player controllers.
*/
final function PlaySpatializedCustomCameraAnim
(
 Name		AnimName,
 Vector2D	Radius,
 float		Rate,
 optional	float		BlendInTime,
 optional	float		BlendOutTime,
 optional	bool		bLooping,
 optional	bool		bOverride,
 optional	bool		bSingleRayPenetrationOnly,
 optional	bool		bApplyFullMotion,
 optional	float		ScaleFactor = 1.f,
 optional	bool		bDampenWhenTargeting
 )
{
	local PlayerController	PC;
	local GearPawn			WP;
	local float				Dist, OuterCore, DistanceScaling;

	// Early out
	if( AnimName == '' || ScaleFactor <= 0.f )
	{
		return;
	}

	OuterCore = Radius.Y - Radius.X;

	/**
	PawnOwner.FlushPersistentDebugLines();
	PawnOwner.DrawDebugSphere(PawnOwner.Location, Radius.X, 16, 000, 255, 000, TRUE);
	PawnOwner.DrawDebugSphere(PawnOwner.Location, Radius.Y, 16, 255, 000, 000, TRUE);
	*/

	// Play custom camera animation for human controlled players in synch w/ this one.
	foreach PawnOwner.WorldInfo.LocalPlayerControllers(class'PlayerController', PC)
	{
		WP = GearPawn(PC.Pawn);

		if( WP != None )
		{
			Dist = VSize(WP.Location - PawnOwner.Location);

			if( Dist < Radius.Y )
			{
				// Inner Core = max volume
				if( Dist < Radius.X )
				{
					DistanceScaling = 1.f;
				}
				// outer core = linear fall off
				else
				{
					DistanceScaling = 1.f - FClamp( (Dist - Radius.X) / OuterCore, 0.f, 1.f);
				}

				//`log(PawnOwner.WorldInfo.TimeSeconds @ class @ GetFuncName() @ AnimName @ "Dist:" @ Dist @ "ScaleFactor:" @ ScaleFactor);

				if( ScaleFactor * DistanceScaling > 0.f )
				{
//					WP.PlayCustomCameraAnim(AnimName, Rate, BlendInTime, BlendOutTime, bLooping, bOverride, bSingleRayPenetrationOnly, bApplyFullMotion, ScaleFactor * DistanceScaling, bDampenWhenTargeting);
				}
			}
		}
	}
}

/** Called from GearPawn to give special moves a chance to limit view rotation. */
function LimitViewRotation(out Rotator out_ViewRotation);

/** Notification forwarded from RB_BodyInstance, when a spring is over extended and disabled. */
function OnRigidBodySpringOverextension(RB_BodyInstance BodyInstance);

/** Gives special moves a chance to override the offset used for the mesh bone controllers. */
event bool GetAimOffsetOverride(out rotator DeltaRot)
{
	if( IsPawnRotationLocked() )
	{
		DeltaRot = rot(0,0,0);
		return TRUE;
	}

	return FALSE;
}

/** Hook for accessing HUD */
function DrawHUD(HUD H);

function int ChooseRandomCameraAnim(out const array<CameraAnim> Anims, GearPawn PawnToPlay, FLOAT Scale, optional bool bDoNotRandomize)
{
	local GearPC	PCToPlay;

	PCToPlay = GearPC(PawnToPlay.Controller);

	// Only play camera animations on local human players
	if( PCToPlay == None || !PawnToPlay.IsLocallyControlled() || AnimatedCamera(PCToPlay.PlayerCamera) == None )
	{
		`log("ChooseRandomCameraAnim failed PCToPlay:" @ PCToPlay @ "PawnToPlay.IsLocallyControlled():" @ PawnToPlay.IsLocallyControlled() @ "AnimatedCamera:" @ AnimatedCamera(PCToPlay.PlayerCamera));
		return INDEX_NONE;
	}

	return PCToPlay.ChooseRandomCameraAnim(Anims, Scale, bDoNotRandomize);
}

/** Play a random camera animation CameraAnim */
function CameraAnimInst PlayRandomCameraAnim
(
	GearPawn		PawnToPlay,
	out const array<CameraAnim> Anims,
	optional float	Rate=1.f,
	optional float	Scale=1.f,
	optional float	BlendInTime,
	optional float	BlendOutTime,
	optional bool	bLoop,
	optional bool	bRandomStartTime,
	optional float	Duration,
	optional bool	bSingleInstance,
	optional bool	bDoNotRandomize
)
{
	local INT Index;

	Index = ChooseRandomCameraAnim(Anims, PawnToPlay, Scale, bDoNotRandomize);

	if( Index != INDEX_NONE )
	{
		return PlayCameraAnim(PawnToPlay, Anims[Index], Rate, Scale, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Duration, bSingleInstance);
	}
	else
	{
		`log("PlayRandomCameraAnim failed, INDEX_NONE");
	}

	return None;
}

/** Play a CameraAnim */
function CameraAnimInst PlayCameraAnim
(
	GearPawn		PawnToPlay,
	CameraAnim		InCameraAnim,
	optional float	Rate=1.f,
	optional float	Scale=1.f,
	optional float	BlendInTime,
	optional float	BlendOutTime,
	optional bool	bLoop,
	optional bool	bRandomStartTime,
	optional float	Duration,
	optional bool	bSingleInstance
)
{
	local GearPC	PCToPlay;

	PCToPlay = GearPC(PawnToPlay.Controller);

	// Only play camera animations on local human players
	if(  PCToPlay == None ||
		!PawnToPlay.IsLocallyControlled() ||
		 AnimatedCamera(PCToPlay.PlayerCamera) == None ||
		!PCToPlay.CameraAnimHasEnoughSpace(InCameraAnim, Scale) )
	{
		`log("PlayCameraAnim failed CameraAnimHasEnoughSpace:" @ PCToPlay.CameraAnimHasEnoughSpace(InCameraAnim, Scale));
		return None;
	}

	// Set flag so we can cancel force pitch centering at the end.
	// @fixme -- this only works for PCOwner. Need to come up with a better system for any PC
	if( PawnToPlay == PawnOwner )
	{
		bPlayingCameraAnim = TRUE;
	}
	else
	{
		// A little hacky... but should work for what we need.
		if( PawnToPlay.SpecialMove != SM_None )
		{
			PawnToPlay.SpecialMoves[PawnToPlay.SpecialMove].bPlayingCameraAnim = TRUE;
		}
	}

	// Force pitch centering so camera animation plays properly
	PCToPlay.MainPlayerInput.ForcePitchCentering(TRUE);

	// Play camera animation
	return AnimatedCamera(PCToPlay.PlayerCamera).PlayCameraAnim(InCameraAnim, Rate, Scale, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Duration, bSingleInstance);
}

/**
 * Turn a World Space location into an Actor Space relative location.
 */
native final function vector WorldToRelativeOffset(Rotator InRotation, Vector WorldSpaceOffset) const;
native final function vector RelativeToWorldOffset(Rotator InRotation, Vector RelativeSpaceOffset) const;


simulated native function SetBasedPosition( out BasedPosition BP, Vector inLoc );
simulated native function Vector GetBasedPosition( out BasedPosition BP );

event RigidBodyWorldCollision(PrimitiveComponent HitComponent, PrimitiveComponent OtherComponent, const out CollisionImpactData RigidCollisionData);

defaultproperties
{
	bNoStoppingPower=TRUE	// By default we want no stopping power in special moves
	bCanFireWeapon=TRUE

	bShouldAbortWeaponReload=FALSE
	bLockPawnRotation=FALSE
	bBreakFromCover=FALSE
	bDisableMovement=FALSE
	bDisableLook=FALSE
	bDisableCollision=FALSE
	bDisablePhysics=FALSE
	bCameraFocusOnPawn=FALSE
	bDisableAI=FALSE
	bCheckForGlobalInterrupts=TRUE

	GravityScale=1.f

	MaxConformToFloorMeshRotation=20.0
	MaxConformToFloorMeshTranslation=30.0
	bRestoreMovementAfterMove=true

}
