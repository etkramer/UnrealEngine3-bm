
/**
 * Meat Shield: Kidnapper
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Kidnapper extends GSM_InteractionPawnLeader_Base;

/** Animations */
var	GearPawn.BodyStance	BS_KidnapperGrab, BS_HostageGrab;
var PlayerController	BackupController;
var bool				bWantHostageToLeaveHisSpecialMove;
/** Original weapon carried when grabbing someone */
var	GearWeapon			OriginalWeapon;
/** Offset of Marker from Pawn's location. Used to place kidnapper to grab hostage properly. */
var vector				MarkerOffset, VectToHostage;
var FLOAT				HostageRotationTimeToGo;
var bool				bInterpolatingHostageRotation;
var class<GearDamageType> KidnapperDamageType;

/**
 * Test conditions to take another Pawn has a hostage
 */
function bool CanInteractWithPawn(GearPawn OtherPawn)
{
	local GearPawn_Infantry OtherIP;
	if (PawnOwner.CoverType != CT_None)
	{
		return FALSE;
	}

	if (PawnOwner.IsCarryingShield())
	{
		return FALSE;
	}


	OtherIP = GearPawn_Infantry(OtherPawn);
	if( OtherIP == none 		// Can only take hostage infantry.
		|| !OtherIP.CanBeAHostage()
		|| PawnOwner.IsSameTeam(OtherPawn) 			// Can only take hostage enemies
		|| OtherPawn.Physics == PHYS_Falling 		// Hostage cannot be falling.
		|| OtherPawn.Physics == PHYS_RigidBody 		// Hostage cannot be a rag doll
		|| OtherPawn.bPlayedDeath )					// Hostage cannot be already dead.
	{
		return FALSE;
	}

	// Hostage has to be DBNO of some type
	if( !OtherPawn.IsDoingSpecialMove(SM_DBNO) && !OtherPawn.IsDoingAStumbleGoDown() )
	{
		return FALSE;
	}

	return PawnOwner.IsFacingOther(OtherPawn,0.f);
}


/** Notification called when Special Move starts */
function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearWeapon Weap;
	local GearPC PC;

	// Reset this flag
	bWantHostageToLeaveHisSpecialMove = FALSE;

	// can't mantle while kidnapping
	PawnOwner.bCanMantle  = FALSE;
	PawnOwner.bCanClimbUp = FALSE;

	// Update the attached hostage before physics runs
	PawnOwner.Mesh.bForceUpdateAttachmentsInTick = TRUE;

	// force the kidnapper to switch to pistols
	// Do this only on locally controlled Pawn. So the new weapon properly replicates to all.
	if( PawnOwner.IsLocallyControlled() )
	{
		// Backup weapon, so we can restore it.
		OriginalWeapon = PawnOwner.MyGearWeapon;

		PC = GearPC(PawnOwner.Controller);
		if (PC != None && PC.MyGearHud != None)
		{
			PC.MyGearHud.ClearActionInfoByType(AT_SpecialMove);
		}

		foreach PawnOwner.InvManager.InventoryActors(class'GearWeapon', Weap)
		{
			if( Weap.WeaponType == WT_Holster )
			{
				`log("Forcing kidnapper to switch to:"@Weap);
				PawnOwner.InvManager.SetCurrentWeapon(Weap);
				break;
			}
		}
	}

	Super.SpecialMoveStarted(bForced,PrevMove);
}

/**
 * Begins the interaction.
 */
function StartInteraction()
{
	local GearPRI HostagePRI, KidnapperPRI;

	// Debug to find marker location
	if( FALSE )
	{
		PlayGrab(2.f, 0.f);
		PawnOwner.SetTimer( 0.2f, FALSE, nameof(self.DEBUG_GetAttachBoneLoc), self );
	}

	// Move kidnapper into position to grab hostage.
	MoveToMarkers();

	// award the death of the hostage if allowed to
	if( Follower.CanBeSpecialMeleeAttacked(PawnOwner) )
	{

		if( PawnOwner.WorldInfo.Game != None )
		{
			// Set PRI death variables
			HostagePRI = GearPRI(Follower.PlayerReplicationInfo);
			KidnapperPRI = GearPRI(PawnOwner.PlayerReplicationInfo);
			Follower.SetDeathReplicationData( KidnapperPRI, HostagePRI, class'GDT_Hostage' );
			if (GearPC(HostagePRI.Owner) != None && GearGameMP_Base(PawnOwner.WorldInfo.Game) != None && !GearGameMP_Base(PawnOwner.WorldInfo.Game).bDeadCanTalkToLiving)
			{
				GearGameMP_Base(PawnOwner.WorldInfo.Game).InitSpectateMuteList(GearPC(HostagePRI.Owner));
			}
			KidnapperPRI.ScoreExecution(HostagePRI,class'GDT_Hostage',GDT_NORMAL);
			// Score the kill in the game object
			PawnOwner.WorldInfo.Game.ScoreKill( PawnOwner.Controller, Follower.Controller );
		}
	}
}

/** DEBUG function to find out where attachment socket is located, and bake that result into MarketOffset. */
function DEBUG_GetAttachBoneLoc()
{
	DebugSocketRelativeLocation('HostageGrab');
}

/** Place both characters on markers, so they can connect and play their grab animation for attachment */
function MoveToMarkers()
{
	local Vector	KidnapperDestination;
	local float		BlendInTime, AnimPlayRate;

	VectToHostage = Follower.Location - PawnOwner.Location;
	VectToHostage.Z = 0.f;
	VectToHostage = Normal(VectToHostage);

	KidnapperDestination = Follower.Location + RelativeToWorldOffset(Rotator(-VectToHostage), MarkerOffset);

	BlendInTime = 0.2f;
	AnimPlayRate = 1.67f;

	// Start playing grab animation
	PlayGrab(AnimPlayRate, BlendInTime);

	// Move kidnapper relative to hostage, so his marker aligns with Hostage's root bone.
	SetReachPreciseDestination(KidnapperDestination);
	// Rotate Kidnapper so he faces hostage.
	SetFacePreciseRotation(Rotator(VectToHostage), BlendInTime);
}

function PlayGrab(float PlayRate, float BlendInTime)
{
	// Play animation on both characters.
	PawnOwner.BS_Play(BS_KidnapperGrab, PlayRate, BlendInTime, 0.2f);
	PawnOwner.BS_SetAnimEndNotify(BS_KidnapperGrab, TRUE);

	Follower.BS_Play(BS_HostageGrab, PlayRate, BlendInTime, 0.2f);
	Follower.BS_SetbZeroRootTranslation(BS_HostageGrab, TRUE);
	Follower.RemoveAndSpawnAHelmet( Follower.Location - PawnOwner.Location, class'GearDamageType', TRUE );
	Follower.RemoveAndSpawnAShoulderPadLeft( Follower.Location - PawnOwner.Location, class'GearDamageType' );
	Follower.RemoveAndSpawnAShoulderPadRight( Follower.Location - PawnOwner.Location, class'GearDamageType' );
	Follower.RemoveAndDropAttachments( Follower.Location - PawnOwner.Location, class'GearDamageType' );

	// Activate MeatShield Morph Target to prevent hostage clipping into kidnapper.
// 	Follower.SetActiveMeatShieldMorph(TRUE);

	if( Follower.MeatShieldMorphNodeWeight != None )
	{
		Follower.MeatShieldMorphNodeWeight.SetNodeWeight(1.f);
	}

	// Timer to trigger the grab, do this once we're fully blended in
	if( BlendInTime > 0.f )
	{
		PawnOwner.SetTimer( BlendInTime, FALSE, nameof(self.AttachHostageToKidnapper), self );
	}
	else
	{
		AttachHostageToKidnapper();
	}

	// Hostage turning takes place from 0.65 seconds to 1.15seconds, so it lasts for 0.5 seconds.
	// Scale everything by play rate.
	HostageRotationTimeToGo = 0.5f / PlayRate;
	PawnOwner.SetTimer( 0.65f / PlayRate, FALSE, nameof(self.StartHostageTurn), self );

	// notify guds
	if( PawnOwner.WorldInfo.Game != None )
	{
		GearGame(PawnOwner.WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpMeatShield, PawnOwner, Follower);
	}
}

function StartHostageTurn()
{
	bInterpolatingHostageRotation = TRUE;
}

function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	// Grab animation is done playing, we can now move again.
	SetMovementLock(FALSE);
	SetLockPawnRotation(FALSE);
}

/** Attach Hostage to Kidnapper */
function AttachHostageToKidnapper()
{
	// Cancel kidnapper movement, since hostage is now going to be attached.
	SetReachPreciseDestination(Vect(0,0,0), TRUE);

    // Have the follower drop their weapon
    Follower.TossInventory(Follower.Weapon);

	AttachFollowerToLeader('HostageGrab');

	// Hide weapon attachments from Hostage's back, so it doesn't clip into kidnapper.
	Follower.SetSlotAttachment(EASlot_LeftShoulder, None);
	Follower.SetSlotAttachment(EASlot_RightShoulder, None);

	// detach the controller if this is a MP player that can be executed
	// so that they can spectate, respawn, etc
	if( PawnOwner.WorldInfo.GRI.IsMultiplayerGame() && Follower.CanBeSpecialMeleeAttacked(PawnOwner) )
	{
		Follower.DetachFromController(FALSE);
	}

	// Allow the game object to know if a kidnapping took place
	if( PawnOwner.WorldInfo.Game != None )
	{
		GearGame(PawnOwner.WorldInfo.Game).KidnappingStarted(Follower, PawnOwner);
	}
}

function Tick(float DeltaTime)
{
	local Rotator				DeltaRot;
	local SkeletalMeshSocket	Socket;

	Super.Tick(DeltaTime);

	// Synchronize animations together.
	Follower.AnimTreeRootNode.ForceGroupRelativePosition('Sync_Idle', PawnOwner.AnimTreeRootNode.GetGroupRelativePosition('Sync_Idle'));
	Follower.AnimTreeRootNode.ForceGroupRelativePosition('RunWalk', PawnOwner.AnimTreeRootNode.GetGroupRelativePosition('RunWalk'));

	if( bInterpolatingHostageRotation )
	{
		Socket = PawnOwner.Mesh.GetSocketByName('HostageGrab');
		if( Socket != None )
		{
			if( HostageRotationTimeToGo > DeltaTime )
			{
				// Delta rotation
				DeltaRot = Normalize(Normalize(Socket.RelativeRotation) - Normalize(Follower.RelativeRotation));
				Follower.SetRelativeRotation(Normalize(Follower.RelativeRotation + DeltaRot * (DeltaTime / HostageRotationTimeToGo)));
				HostageRotationTimeToGo -= DeltaTime;
			}
			else
			{
				Follower.SetRelativeRotation(Socket.RelativeRotation);
				bInterpolatingHostageRotation = FALSE;
			}
		}
		else
		{
			bInterpolatingHostageRotation = FALSE;
		}
	}
}


/** Catch melee button to execute hostage */
function bool ButtonPress(Name ButtonName)
{
	// Melee button means drop hostage, and execute him if he's still alive
	if( ButtonName == 'X' )
	{
		// If hostage is not the meatflag (CanBeSpecialMeleeAttacked), trigger execution Special Move
		if( Follower.CanBeSpecialMeleeAttacked(PawnOwner) )
		{
			PawnOwner.LocalDoSpecialMove(SM_Kidnapper_Execution, TRUE, Follower, 0);
		}
		// Otherwise, just drop him with no specific execution animation.
		else
		{
			PawnOwner.LocalEndSpecialMove();
		}
		return TRUE;
	}

	return FALSE;
}

/**
 * Detaches a based Pawn from the Leader.
 */
function DetachPawn(GearPawn APawn)
{
	Super.DetachPawn(APawn);

	// Allow the game object to know if someone was released by a kidnapping
	if( PawnOwner.WorldInfo.Game != None )
	{
		GearGame(PawnOwner.WorldInfo.Game).KidnappingStopped(Follower, PawnOwner);
	}
}

function LockIKStatus(bool bInstant)
{
	// Disable IK on right hand.
	if( bInstant )
	{
		PawnOwner.IKBoneCtrl_RightHand.SetSkelControlStrength(0.f, 0.f);
		PawnOwner.IKBoneCtrl_LeftHand.SetSkelControlStrength(0.f, 0.f);
	}
	else
	{
		PawnOwner.IKBoneCtrl_RightHand.SetSkelControlActive(FALSE);
		PawnOwner.IKBoneCtrl_LeftHand.SetSkelControlActive(FALSE);
	}
	PawnOwner.bLockIKStatus = TRUE;
}

function RestoreIKStatus()
{
	PawnOwner.IKBoneCtrl_RightHand.SetSkelControlActive(TRUE);
	PawnOwner.bLockIKStatus = FALSE;
	// For left hand, depends on what we're doing.
	PawnOwner.UpdateBoneLeftHandIK();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Keep track of when Hostage was released.
	PawnOwner.HostageReleaseTime = PawnOwner.WorldInfo.TimeSeconds;

	// Reset stuffs
	PawnOwner.BS_SetAnimEndNotify(BS_KidnapperGrab, FALSE);
	PawnOwner.bCanMantle  = PawnOwner.default.bCanMantle;
	PawnOwner.bCanClimbUp = PawnOwner.default.bCanClimbUp;
	PawnOwner.Mesh.bForceUpdateAttachmentsInTick = PawnOwner.default.Mesh.bForceUpdateAttachmentsInTick;

	// Make sure timer is cleared
	PawnOwner.ClearTimer('AttachHostageToKidnapper', Self);
	PawnOwner.ClearTimer('StartHostageTurn', Self);

	// If we're not transitioning to an execution, then clean up hostage.
	if( NextMove != SM_Kidnapper_Execution )
	{
		// Make sure IK are disabled. We can't blend out of this because IK setup is different
		// and causes IK over extenstion.
		LockIKStatus(TRUE);

		// Restore IK when animation is done blending out.
		PawnOwner.SetTimer(0.33f, FALSE, 'RestoreIKStatus', Self);

		// Take care of the hostage  if he's still around
		if( Follower != None && !Follower.bDeleteMe  )
		{
			// We're now expecting the hostage to leave his special move
			bWantHostageToLeaveHisSpecialMove = TRUE;
			Follower.Mesh.SetShadowParent(None);
			// Activate MeatShield Morph Target to prevent hostage clipping into kidnapper.
			Follower.SetActiveMeatShieldMorph(FALSE);

			// If the follower has been attached, detach it.
			if( Follower.Role == Role_Authority && !Follower.bPlayedDeath )
			{

				// If Follower is still a hostage, end his special move
				if( Follower.IsDoingSpecialMove(FollowerSpecialMove) )
				{
					Follower.ServerEndSpecialMove(FollowerSpecialMove);
				}

				// kill off the hostage pawn if allowed to
				if( Follower.CanBeSpecialMeleeAttacked(PawnOwner) )
				{
					Follower.bCollideWorld = TRUE;

					Follower.TearOffMomentum = Normal(Follower.Location - PawnOwner.Location);
					Follower.Died(PawnOwner.Controller, KidnapperDamageType, Follower.Location);
				}
				// Otherwise if kidnapper was killed, then hostage is revived
				else
				{
					// Detach Follower from Leader.
					DetachPawn(Follower);
					Follower.FreeHostage(PawnOwner);
				}
			}
			else if (!Follower.bPlayedDeath)
			{
				if( Follower.IsBasedOn(PawnOwner) )
				{
					// Detach Follower from Leader.
					DetachPawn(Follower);
				}
			}
		}

		// See if we should restore our original weapon, prior to taking someone hostage
		// Make sure PawnOwner hasn't already swapped his weapon, this could end the meatshield special move as well.
		// In that case we don't want to clobber his weapon choice.
		if( NextMove == SM_None && PawnOwner.IsLocallyControlled() && PawnOwner.IsAliveAndWell() && PawnOwner.InvManager.PendingWeapon == None && PawnOwner.MyGearWeapon.WeaponType == WT_Holster )
		{
			// Only do this if we had a different weapon, and it has enough ammo in case of emergency.
			// Don't force switch to an empty weapon.
			if( OriginalWeapon != PawnOwner.MyGearWeapon && OriginalWeapon.HasAmmo(0, OriginalWeapon.CriticalAmmoCount) )
			{
				PawnOwner.InvManager.SetCurrentWeapon(OriginalWeapon);
			}
		}
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

/**
 * Hostage is leaving his special move, catch cases where we didn't expect this from hapening
 * This could happen when the hostage logs out, types 'suicice', or is killed by some other way.
 */
function OnFollowerLeavingSpecialMove()
{
	if( !bWantHostageToLeaveHisSpecialMove )
	{
		bWantHostageToLeaveHisSpecialMove = TRUE;

		// Handle this from the server.
		if( PawnOwner.Role == Role_Authority )
		{
			PawnOwner.ServerEndSpecialMove(PawnOwner.SpecialMove);
		}
	}
}

defaultproperties
{
	KidnapperDamageType=class'GDT_Hostage'
	bShouldAbortWeaponReload=FALSE
	bCanFireWeapon=TRUE
	bDisableLook=FALSE

	bDisableMovement=TRUE
	bLockPawnRotation=TRUE

	MarkerOffset=(X=125.46,Y=0.00,Z=0.00)
	FollowerSpecialMove=SM_Hostage
	BS_KidnapperGrab=(AnimName[BS_FullBody]="Kidnapper_Grab_DBNO")
	BS_HostageGrab=(AnimName[BS_FullBody]="Hostage_Grab_DBNO")

	Action={(
		ActionName=GrabMeatShield,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43))), // A Button
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=0,V=303,UL=58,VL=95)))	),
	)}
}
