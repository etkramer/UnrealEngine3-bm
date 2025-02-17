
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Berserker_Charge extends GearSpecialMove
	DependsOn(AnimSequence)
	native(SpecialMoves);

/** Desired Rotation to use when charging. */
var		Rotator	DesiredChargeRot;
/** If TRUE, perform rotation interpolation */
var		bool	bInterpolateRotation;
/** Time given for rotation interpolation */
var		float	RotationInterpolationTime;

var()	GearPawn.BodyStance	BS_PlayedStance, BS_Idle2ChargeFd, BS_Idle2ChargeBd, BS_Idle2ChargeLt, BS_Idle2ChargeRt;


// C++ functions
cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	bInterpolateRotation = FALSE;

	//`log(GetFuncName() @ GearPawnOwner.WorldInfo.TimeSeconds );

	// If we have a direction set, start charging right away!
	if (GearPawn_LocustBerserkerBase(PawnOwner).HasValidDesiredRootRotation())
	{
		StartCharging();
	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	//`log(GetFuncName() @ GearPawnOwner.WorldInfo.TimeSeconds );

	PawnOwner.bForceMaxAccel = FALSE;

	if (GearPawn_LocustBerserkerBase(PawnOwner) != None)
	{
		GearPawn_LocustBerserkerBase(PawnOwner).bAttackInProgress = false;
	}
	// Clear Timer
	if( PawnOwner.IsTimerActive('IntroEnded', Self) )
	{
		// We got aborted too early, clean up
		IntroEnded();
		PawnOwner.ClearTimer('IntroEnded', Self);
	}
}


simulated function StartCharging()
{
	//SkipChargeIntro();
	PlayChargeIntro();
}

simulated function SkipChargeIntro()
{
	local GearAI_Berserker	BerserkerController;

	// Convert Yaw into a full rotator
	DesiredChargeRot.Yaw = NormalizeRotAxis(GearPawn_LocustBerserkerBase(PawnOwner).DesiredYawRootRotation);

	if (PawnOwner.Controller != None)
	{
		PawnOwner.Controller.bForceDesiredRotation = true;	// Fix pawn rotation and set the rotation
		PawnOwner.Controller.DesiredRotation = DesiredChargeRot;
	}
	PawnOwner.DesiredRotation					= DesiredChargeRot;
	PawnOwner.SetRotation( DesiredChargeRot );

	// This along with bPreciseDestination (set in GearAI_Berserker) will push the pawn at full speed.
	PawnOwner.bForceMaxAccel	= TRUE;

	BerserkerController = GearAI_Berserker(PawnOwner.Controller);
	if( BerserkerController != None )
	{
		BerserkerController.ChargeStartedNotify();
	}
}

/** Get Animation Play Rate */
simulated function float GetAnimPlayRate()
{
	local GearPawn_LocustBerserkerBase	Berserker;

	Berserker = GearPawn_LocustBerserkerBase(PawnOwner);

	// When heated up, animations play faster
	if( Berserker != None && Berserker.bVulnerableToDamage )
	{
		return Berserker.HeatedUpSpeedScale;
	}

	return 1.f;
}

simulated function PlayChargeIntro()
{
	local float		DeltaYaw, IntroAnimTime, AnimRate;
	local rotator	PawnRot;

	PawnRot = Normalize(PawnOwner.Rotation);

	// Convert Yaw into a full rotator
	DesiredChargeRot.Yaw = NormalizeRotAxis(GearPawn_LocustBerserkerBase(PawnOwner).DesiredYawRootRotation);

	// Force Rotation towards the goal.
	if (PawnOwner.Controller != None)
	{
		PawnOwner.Controller.bForceDesiredRotation = true;	// Fix pawn rotation and set the rotation
		PawnOwner.Controller.DesiredRotation = PawnRot;
	}
	PawnOwner.DesiredRotation					= PawnRot;
	PawnOwner.SetRotation( PawnRot );

	// Activate HeadLook Control
	PawnOwner.HeadControl.SetSkelControlActive(TRUE);
	PawnOwner.HeadControl.TargetLocation		= PawnOwner.Location + Vector(DesiredChargeRot) * 65536.f;
	PawnOwner.HeadControl.DesiredTargetLocation	= PawnOwner.HeadControl.TargetLocation;

	// This along with bPreciseDestination (set in GearAI_Berserker) will push the pawn at full speed.
	PawnOwner.bForceMaxAccel	= TRUE;

	// How much we have to turn to face proper direction.
	DeltaYaw = NormalizeRotAxis(DesiredChargeRot.Yaw - PawnOwner.Rotation.Yaw);

	//`log(GetFuncName() @ GearPawnOwner.WorldInfo.TimeSeconds );
	//`log("DeltaYaw:" @ DeltaYaw @ "DesiredChargeRot:" @ DesiredChargeRot.Yaw @ "PawnRot:" @ GearPawnOwner.Rotation.Yaw);

	// Play proper turn animation depending on where we're turning.

	// Forward -45d + 45d
	if( DeltaYaw >= -16384 && DeltaYaw < 16384 )
	{
		BS_PlayedStance = BS_Idle2ChargeFd;
		//`log("Play Turn Fd" @ IntroAnimTime );
	}
	// Right, +45d, +135d
	else if( DeltaYaw >= 16384 && DeltaYaw < 49152 )
	{
		BS_PlayedStance = BS_Idle2ChargeRt;
		//`log("Play Turn Rt" @ IntroAnimTime );
	}
	// Left, -135d, -45d
	else if( DeltaYaw >= -49152 && DeltaYaw < -16384)
	{
		BS_PlayedStance = BS_Idle2ChargeLt;
		//`log("Play Turn Lt" @ IntroAnimTime );
	}
	// Back, +135d, -135d
	else
	{
		BS_PlayedStance = BS_Idle2ChargeBd;
		//`log("Play Turn Bd" @ IntroAnimTime );
	}

	AnimRate = GetAnimPlayRate();
	IntroAnimTime = PawnOwner.BS_Play(BS_PlayedStance, AnimRate, 0.2f/AnimRate, -1.f, FALSE, TRUE);

	// Timer to blend out to the charge animation.
	PawnOwner.SetTimer( IntroAnimTime, FALSE, nameof(self.IntroEnded), self );

	// Have animation forward root motion.
	PawnOwner.BS_SetRootBoneAxisOptions(BS_PlayedStance, RBA_Translate, RBA_Translate);

	// Turn on Root motion on mesh. Use RMM_Accel, to drive full movement.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
}

/** Notification forwarded by GearAnimNotify_SpecialMove */
simulated function AnimNotify(AnimNodeSequence SeqNode, GearAnimNotify_SpecialMove NotifyObject)
{
	local int						i, StartNotifyIndex, EndNotifyIndex;
	local GearAnimNotify_SpecialMove	SMNotify;

	//`log(GetFuncName() @ GearPawnOwner.WorldInfo.TimeSeconds @ NotifyObject.Info );

	if( NotifyObject.Info == 'RotStart' )
	{
		bInterpolateRotation		= TRUE;
		RotationInterpolationTime	= 0.f;

		StartNotifyIndex	= -1;
		EndNotifyIndex		= -1;

		if( SeqNode != None && SeqNode.AnimSeq != None )
		{
			for(i=0; i<SeqNode.AnimSeq.Notifies.Length; i++)
			{
				SMNotify = GearAnimNotify_SpecialMove(SeqNode.AnimSeq.Notifies[i].Notify);

				if( SMNotify != None )
				{
					if( SMNotify.Info == 'RotStart' )
					{
						StartNotifyIndex = i;
					}
					else if( SMNotify.Info == 'RotEnd' )
					{
						EndNotifyIndex = i;
					}
				}

				// If we're done, stop searching
				if( StartNotifyIndex != -1 && EndNotifyIndex != -1 )
				{
					break;
				}
			}

			if( StartNotifyIndex != -1 && EndNotifyIndex != -1 )
			{
				RotationInterpolationTime = SeqNode.AnimSeq.Notifies[EndNotifyIndex].Time - SeqNode.AnimSeq.Notifies[StartNotifyIndex].Time;
				RotationInterpolationTime = RotationInterpolationTime / (SeqNode.Rate * SeqNode.AnimSeq.RateScale);
			}
		}
	}
}


simulated function IntroEnded()
{
	local GearAI_Berserker	BerserkerController;

	//`log(GetFuncName() @ GearPawnOwner.WorldInfo.TimeSeconds );

	// Turn off HeadLook Control
	PawnOwner.HeadControl.SetSkelControlActive(FALSE);

	BerserkerController = GearAI_Berserker(PawnOwner.Controller);
	if( BerserkerController != None )
	{
		BerserkerController.ChargeStartedNotify();
		GearPawn_LocustBerserkerBase(PawnOwner).PlayChargeTakeOffEffect(PawnOwner.Location, Normal(PawnOwner.Location - PawnOwner.HeadControl.TargetLocation) );
	}

	// Turn off rotation interpolation if it didn't finish
	bInterpolateRotation = FALSE;

	// Restore default root motion mode
	PawnOwner.Mesh.RootMotionMode = PawnOwner.Mesh.default.RootMotionMode;

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_PlayedStance, RBA_Discard, RBA_Discard, RBA_Discard);

	// Stop transition instantly
	PawnOwner.BS_Stop(BS_PlayedStance, 0.1f);
}


defaultproperties
{
	BS_Idle2ChargeFd=(AnimName[BS_FullBody]="idle2run_fwd")
	BS_Idle2ChargeBd=(AnimName[BS_FullBody]="idle2run_back")
	BS_Idle2ChargeLt=(AnimName[BS_FullBody]="idle2run_lt")
	BS_Idle2ChargeRt=(AnimName[BS_FullBody]="idle2run_rt")
}
