
/**
 * Play a full body hit reaction, takes over control for a little bit.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_FullBodyHitReaction extends GSM_BasePlaySingleAnim;

var Array<Name> HitReactionAnimName;

protected function bool InternalCanDoSpecialMove()
{
	// We don't have heavy weapon variation of those :(
	if( PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		return FALSE;
	}

	// Can't do it if we're based on another Pawn (bloodmount, brumak, reavers).
	if( PawnOwner.Base != None && Pawn(PawnOwner.Base) != None )
	{
		return FALSE;
	}

	return TRUE;
}

static function INT PickHitReactionAnim(GearPawn Victim, class<GearDamageType> DamageType, TraceHitInfo HitInfo, vector Momentum, vector HitLocation)
{
	local GearPawn_Infantry	InfantryPawn;
	local Vector			ShotDir, HitNormal, LookDir, RightDir;
	local FLOAT				DotFront, DotRight;

	// Only popping up behind mid level cover really works for those.
	if( Victim.CoverType != CT_None && (Victim.CoverType == CT_Standing || Victim.CoverAction != CA_PopUp) )
	{
		return INDEX_NONE;
	}

	InfantryPawn = GearPawn_Infantry(Victim);

	// Try to figure out shot direction.
	ShotDir		= IsZero(Momentum) ? Normal(Victim.Location - HitLocation) : Normal(Momentum);
	HitNormal	= -1.f * ShotDir;
	HitNormal.Z = 0.f;
	HitNormal	= Normal(HitNormal);

	LookDir		= vector(Victim.Rotation);
	RightDir	= vect(0,0,1) cross LookDir;
	DotFront	= HitNormal dot LookDir;
	DotRight	= HitNormal dot RightDir;

	// Check based on bone names
	if( HitInfo.BoneName != '' )
	{
		if( InfantryPawn != None )
		{
			// Below animations only work when shot from the front.
			if( DotFront > 0.f )
			{
				// Find an animation to play based on which bone was hit.
				if( InfantryPawn.MatchNameArray(HitInfo.BoneName, InfantryPawn.StomachBones) )
				{
					return 6; // Gut hit
				}
				if( InfantryPawn.MatchNameArray(HitInfo.BoneName, InfantryPawn.RightShoulderBones) )
				{
					return (FRand() < 0.67f) ? 4 : 5; // Right shoulder, w/ emphasis on the little hit reaction rather than the long spin
				}
				if( InfantryPawn.MatchNameArray(HitInfo.BoneName, InfantryPawn.LeftShoulderBones) )
				{
					return (FRand() < 0.67f) ? 2 : 3; // Left shoulder, w/ emphasis on the little hit reaction rather than the long spin
				}
			}

			if( InfantryPawn.MatchNameArray(HitInfo.BoneName, InfantryPawn.LeftLegBones) )
			{
				return 7; // left leg
			}
			if( InfantryPawn.MatchNameArray(HitInfo.BoneName, InfantryPawn.RightLegBones) )
			{
				return 8; // right leg
			}
		}
	}

	// Check based on hit location
	if( Abs(DotFront) > Abs(DotRight) )
	{
		if( DotFront > 0.f )
		{
			return 6; // Gut hit
		}
		return (FRand() < 0.67f) ? 0 : 1;	// back hit, w/ more emphasis on quick one than long spin
	}
	if( DotRight > 0.f )
	{
		return (FRand() < 0.67f) ? 4 : 5; // Right shoulder, w/ emphasis on the little hit reaction rather than the long spin
	}
	return (FRand() < 0.67f) ? 2 : 3; // Left shoulder, w/ emphasis on the little hit reaction rather than the long spin
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced, PrevMove);
}

function PlayAnimation()
{
	// Assign proper animation.
	BS_Animation.AnimName[BS_FullBody] = HitReactionAnimName[PawnOwner.SpecialMoveFlags];

	Super.PlayAnimation();

	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate, RBA_Translate);
	// Turn on Root motion on mesh. Use RMM_Accel, to drive full movement.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;

	// Use root rotation to rotate the actor.
	PawnOwner.BodyStanceNodes[BS_FullBody].GetCustomAnimNodeSeq().RootRotationOption[2] = RRO_Extract;
	PawnOwner.Mesh.RootMotionRotationMode = RMRM_RotateActor;

	PawnOwner.BS_SetEarlyAnimEndNotify(BS_Animation, FALSE);
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Reset root rotation
	PawnOwner.BodyStanceNodes[BS_FullBody].GetCustomAnimNodeSeq().RootRotationOption[2] = RRO_Default;
	PawnOwner.Mesh.RootMotionRotationMode = RMRM_Ignore;

	// Restore default root motion mode
	PawnOwner.Mesh.RootMotionMode = PawnOwner.Mesh.default.RootMotionMode;

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard);

	// Make sure animation is stopped if aborted
	if( PawnOwner.BS_IsPlaying(BS_Animation) )
	{
		PawnOwner.BS_Stop(BS_Animation, BlendOutTime);
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	HitReactionAnimName(0)="HitReact_Back"
	HitReactionAnimName(1)="HitReact_Back_Turn"
	HitReactionAnimName(2)="HitReact_Front_LtSho_Sml"
	HitReactionAnimName(3)="HitReact_Front_LtSho_Spin"
	HitReactionAnimName(4)="HitReact_Front_RtSho_Sml"
	HitReactionAnimName(5)="HitReact_Front_RtSho_Spin"
	HitReactionAnimName(6)="HitReact_Gut_001"
	HitReactionAnimName(7)="HitReact_LtLeg_Sml"
	HitReactionAnimName(8)="HitReact_RtLeg_Sml"

	BlendInTime=0.2f
	BlendOutTime=0.42f

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
	bBreakFromCover=TRUE
	bDisableTurnInPlace=TRUE
	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE
	bLockPawnRotation=TRUE
	bDisableAI=TRUE
	bRestoreMovementAfterMove=TRUE
}
