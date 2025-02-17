
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_DeathAnim extends GSM_DeathAnimBase;

/** If TRUE, show debugging information. */
var bool bDebug;

/** Restrictions for doing Death Animation. */
protected function bool InternalCanDoSpecialMove()
{
	// We need a mesh to do any of this.
	// Don't play death animation when doing a mantle over. Just go into ragdoll
	if( PawnOwner.Mesh == None || PawnOwner.SpecialMoveWhenDead == SM_MidLvlJumpOver )
	{
		return FALSE;
	}

	// We don't have animations when crawling, so just go straight to rag doll.
	// Unless we've been headshotted!
	if( !PawnOwner.bHasPlayedHeadShotDeath && PawnOwner.SpecialMoveWhenDead == SM_DBNO )
	{
		return FALSE;
	}

	return TRUE;
}

/** This special move can override pretty much any other special moves. */
function bool CanOverrideSpecialMove(ESpecialMove InMove)
{
	return TRUE;
}

/** 
 * Pick a random death animation from supplied list.
 * Also makes sure we don't pick up the same animation twice in a row.
 */
final function Name PickDeathAnimationFromList(Array<Name> DeathAnimList, out byte GlobalAnimIndex)
{
	local byte CurrentIndex;

	// Get a random animation to play
	CurrentIndex = Rand(DeathAnimList.Length);

	// If this is the one we've played last, then pick the next one in the list to play
	if( CurrentIndex == GlobalAnimIndex )
	{
		CurrentIndex = ++CurrentIndex % DeathAnimList.Length;
	}

	// Update index so we remember what it was
	GlobalAnimIndex = CurrentIndex;

	return DeathAnimList[CurrentIndex];
}

/** 
 * Play a death animation followed by rag doll physics. 
 * Return TRUE if successful, FALSE if couldn't find an animation and wants to go into ragdoll.
 */
function bool PlayDeathAnimation(class<GearDamageType> DamageType, vector HitLoc)
{
	local BodyStance	BS_DeathAnim;
	local bool			bLoop, bPickedDeathAnimation, bRotateToShotDir, bShotFromSide;
	local Rotator		ShotRotation, PawnLookRotation;
	local GearGRI		MyGRI;
	local Vector		WorldHitDir, LookX, LookY, LookZ, WorldHitX, WorldHitY, WorldHitZ;
	local Vector		LocalHitDir;

	bPickedDeathAnimation = FALSE;
	bLoop = FALSE;

	// Pointer to GameReplicationInfo.
	MyGRI = GearGRI(PawnOwner.WorldInfo.GRI);

	// Pawn look rotation is pawn rotation + his aimoffset horizontal value. This is what really matters to find out how the pawn should fall.
	PawnLookRotation.Yaw = NormalizeRotAxis(PawnOwner.Rotation.Yaw + PawnOwner.AimOffsetPct.X * 16384.f);

	// Did pawn got shot in the back?
	if( VSizeSq(PawnOwner.TearOffMomentum) > 0.01f )
	{
		// Figure out where the shot came from so we can trigger proper death animations.
		GetAxes(PawnLookRotation, LookX, LookY, LookZ);
		
		// Normalize and turn into 2D space. We don't care about Z axis.
		WorldHitDir = -1.f * (PawnOwner.TearOffMomentum);
		WorldHitDir.Z = 0.f;
		WorldHitDir = Normal(WorldHitDir);

		LocalHitDir.X = WorldHitDir dot LookX;
		LocalHitDir.Y = WorldHitDir dot LookY;

		bShotFromSide = Abs(LocalHitDir.Y) > Abs(LocalHitDir.X);
		GetAxes(Rotator(WorldHitDir), WorldHitX, WorldHitY, WorldHitZ);
	}
	else
	{
		`Warn(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "TearOffMomentum is zero:" @ PawnOwner.TearOffMomentum @ "for" @ DamageType @ Normal(HitLoc-PawnOwner.Location));
	}

	// If cannot play a death animation, we'll just hold the current animation pose before collapsing in rag doll
	// But if we can, let's pick which animation to play.
	if( PawnOwner.bPlayDeathAnimations && !PawnOwner.bHasPlayedHeadShotDeath )
	{
		// Pick a high kick death animation (for explosives/shotgun).
		if( !bPickedDeathAnimation && DamageType.default.bHighKickDeathAnimation )
		{
			// See if Pawn got shot in the back, then do forward fall
			if( LocalHitDir.X < 0.f && PawnOwner.DeathAnimHighFwd.Length != 0 )
			{
				BS_DeathAnim.AnimName[BS_FullBody] = PickDeathAnimationFromList(PawnOwner.DeathAnimHighFwd, MyGRI.LastDeathAnimHighFwdIndex);
				bRotateToShotDir = TRUE;
				bPickedDeathAnimation = TRUE;
				ShotRotation.Yaw = Rotator(-WorldHitX).Yaw;
			}
			// See if Pawn got shot in the front, then do a backward fall
			else if( LocalHitDir.X >= 0.f && PawnOwner.DeathAnimHighBwd.Length != 0 )
			{
				BS_DeathAnim.AnimName[BS_FullBody] = PickDeathAnimationFromList(PawnOwner.DeathAnimHighBwd, MyGRI.LastDeathAnimHighBwdIndex);
				bRotateToShotDir = TRUE;
				bPickedDeathAnimation = TRUE;
				ShotRotation.Yaw = Rotator(WorldHitX).Yaw;
			}

			`if(`notdefined(FINAL_RELEASE))
			if( bDebug )
			{
				if( bPickedDeathAnimation )
				{
					`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Picked a High Kick death animation." @ BS_DeathAnim.AnimName[BS_FullBody] @ "LocalHitDir:" @ LocalHitDir);
				}
				else
				{
					`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Has empty DeathAnimHighFwd or DeathAnimHighBwd couldn't play high kick death animation."  @ "LocalHitDir:" @ LocalHitDir);
				}
			}
			`endif
		}

		// Pick a standard kill animation (for standard weapons).
		if( TRUE )
		{
			// See if we can play a death from the side first, these are less common than Fwd/Bwd deaths
			// If the Pawn doesn't have sideways deaths, we can always fall back to Fwd/Bwd.
			if( !bPickedDeathAnimation && bShotFromSide )
			{
				// See if Pawn got Shot from left, then do right fall
				if( LocalHitDir.Y < 0.f && PawnOwner.DeathAnimStdRt.Length != 0 )
				{
					BS_DeathAnim.AnimName[BS_FullBody] = PickDeathAnimationFromList(PawnOwner.DeathAnimStdRt, MyGRI.LastDeathAnimStdRtIndex);
					bRotateToShotDir = TRUE;
					bPickedDeathAnimation = TRUE;
					ShotRotation.Yaw = Rotator(WorldHitY).Yaw;
				}
				// See if Pawn got Shot from right, then do left fall
				else if( LocalHitDir.Y >= 0.f && PawnOwner.DeathAnimStdLt.Length != 0 )
				{
					BS_DeathAnim.AnimName[BS_FullBody] = PickDeathAnimationFromList(PawnOwner.DeathAnimStdLt, MyGRI.LastDeathAnimStdLtIndex);
					bRotateToShotDir = TRUE;
					bPickedDeathAnimation = TRUE;
					ShotRotation.Yaw = Rotator(-WorldHitY).Yaw;
				}
			}

			// Fwd/Bwd deaths
			if( !bPickedDeathAnimation )
			{
				// See if Pawn got shot in the back, then do forward fall
				if( LocalHitDir.X < 0.f && PawnOwner.DeathAnimStdFwd.Length != 0 )
				{
					BS_DeathAnim.AnimName[BS_FullBody] = PickDeathAnimationFromList(PawnOwner.DeathAnimStdFwd, MyGRI.LastDeathAnimStdFwdIndex);
					bRotateToShotDir = TRUE;
					bPickedDeathAnimation = TRUE;
					ShotRotation.Yaw = Rotator(-WorldHitX).Yaw;
				}
				// See if Pawn got shot in the front, then do a backward fall
				else if( LocalHitDir.X >= 0.f && PawnOwner.DeathAnimStdBwd.Length != 0 )
				{
					BS_DeathAnim.AnimName[BS_FullBody] = PickDeathAnimationFromList(PawnOwner.DeathAnimStdBwd, MyGRI.LastDeathAnimStdBwdIndex);
					bRotateToShotDir = TRUE;
					bPickedDeathAnimation = TRUE;
					ShotRotation.Yaw = Rotator(WorldHitX).Yaw;
				}
			}

			`if(`notdefined(FINAL_RELEASE))
			if( bDebug )
			{
				if( bPickedDeathAnimation )
				{
					`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Picked a standard death animation." @ BS_DeathAnim.AnimName[BS_FullBody] @ "LocalHitDir:" @ LocalHitDir);
				}
				else
				{
					`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Couldn't find suitable std death animations!!"  @ "LocalHitDir:" @ LocalHitDir);
				}
			}
			`endif
		}

		// Play one shot death animation
		if( bPickedDeathAnimation )
		{
			PawnOwner.BS_Play(BS_DeathAnim, 1.25f, 0.2f, -1.f, bLoop, FALSE);
		}
		else
		{
			`Warn(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Couldn't find a death animation to play! LocalHitDir:" @ LocalHitDir);
			// Do rag doll death instead.
			return FALSE;
		}

		// if we should rotate Pawn to match direction of shot, do so
		if( bRotateToShotDir )
		{
			`if(`notdefined(FINAL_RELEASE))
			if( bDebug )
			{
				PawnOwner.FlushPersistentDebugLines();
				PawnOwner.DrawDebugLine(PawnOwner.Location, PawnOwner.Location + Vector(PawnLookRotation) * 50,		255, 000, 000, TRUE);
				PawnOwner.DrawDebugLine(PawnOwner.Location, PawnOwner.Location + Vector(ShotRotation) * 50,			000, 255, 000, TRUE);
				PawnOwner.DrawDebugLine(PawnOwner.Location, PawnOwner.Location - PawnOwner.TearOffMomentum * 50,	000, 000, 255, TRUE);
			}
			`endif

			// @note laurent: Interpolation doesn't work. Probably physics mode, or tick not getting called when killed. Need to look into it.
			//SetFacePreciseRotation(ShotRotation, 0.2f);
			PawnOwner.SetRotation(ShotRotation);
		}
	}

	// TRUE = blend to physics/motors. FALSE = plain animation, no physics used. Used for debugging.
	if( TRUE )
	{
		DeathAnimBlendToMotors();
	}
	else
	{
		PawnOwner.EndSpecialMove();
	}

	return TRUE;
}

/** 
 * Event called from Anim Notify when body rests in animation, to trigger transition to full rag doll. 
 * This is an optional setting, not all death animations have that notify. Default rest body rest time is set in timer call to 'DeathAnimRagDoll'. 
 */
function OnDeathAnimBodyRest()
{
	if( PawnOwner.IsTimerActive('DeathAnimRagDoll', Self) )
	{
		PawnOwner.ClearTimer('DeathAnimRagDoll', Self);
		DeathAnimRagDoll();
	}
}

/** Notification forwarded from RB_BodyInstance, when a spring is over extended and disabled. */
function OnRigidBodySpringOverextension(RB_BodyInstance BodyInstance)
{
	local Name				PelvisBoneName;
	local RB_BodyInstance	PelvisBodyInstance;

	// Make sure we have correct bone name if this is a socket.
	PelvisBoneName = PawnOwner.Mesh.GetSocketBoneName(PawnOwner.PelvisBoneName);
	PelvisBodyInstance = PawnOwner.Mesh.FindBodyInstanceNamed(PelvisBoneName);

	// If Pelvis spring was broken, we can't keep the other limbs (hands & legs) under spring influence, it would look too bad.
	// So we just turn them off and force the Pawn into rag doll.
	if( PelvisBodyInstance == BodyInstance )
	{
		PawnOwner.ClearTimer('DeathAnimRagDoll', Self);
		DeathAnimRagDoll();
	}
}

function DeathAnimBlendToMotors()
{
	local Actor PawnBase;
	local vector HitLoc, HitNormal;
	PawnBase = PawnOwner.Base;
	if (PawnBase == None)
	{
		PawnBase = PawnOwner.Trace(HitLoc,HitNormal,PawnOwner.Location + vect(0,0,-128.f),PawnOwner.Location);
	}
	//`log(`showvar(PawnBase)@`showvar(PawnBase.bMovable)@`showvar(PawnOwner.bHasBrokenConstraints));
	// Setup a bunch of springs to have physics match animation (as long as we are standing on non-moving thing)
	// Don't do it if we have broken constraints. We could figure out which ones are broken... For now, just do nothing.
	if( PawnBase != None && !PawnBase.bMovable && !PawnOwner.bHasBrokenConstraints )
	{
		SetSpringForBone(PawnOwner.PelvisBoneName, TRUE);
		SetSpringForBone(PawnOwner.RightHandBoneName, TRUE);
		SetSpringForBone(PawnOwner.LeftHandBoneName, TRUE);
		SetSpringForBone(PawnOwner.LeftFootBoneName, TRUE);
		SetSpringForBone(PawnOwner.RightFootBoneName, TRUE);

		Super.DeathAnimBlendToMotors();
	
		// Set when we turn motors off, and ignore animation
		PawnOwner.SetTimer( 1.33f, FALSE, nameof(self.DeathAnimRagDoll), self );
	
		// Decrease joint limits, to prevent nasty poses once in complete rag doll.
		if( !PawnOwner.bIsGore && PawnOwner.ScaleLimitTimeToGo <= 0 )
		{
			PawnOwner.ReduceConstraintLimits();
		}
	}
	else
	{
		Super.DeathAnimBlendToMotors();
		// straight to ragdoll
		DeathAnimRagDoll();
	}
}

function DeathAnimRagDoll()
{
	`if(`notdefined(FINAL_RELEASE))
	if( bDebug )
	{
		`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "switching to rag doll");
		ScriptTrace();
	}
	`endif

	// Turn off bone springs
	SetSpringForBone(PawnOwner.PelvisBoneName, FALSE);
	SetSpringForBone(PawnOwner.RightHandBoneName, FALSE);
	SetSpringForBone(PawnOwner.LeftHandBoneName, FALSE);
	SetSpringForBone(PawnOwner.LeftFootBoneName, FALSE);
	SetSpringForBone(PawnOwner.RightFootBoneName, FALSE);

	Super.DeathAnimRagDoll();

	PawnOwner.EndSpecialMove();
}


defaultproperties
{
	bDebug=FALSE
}
