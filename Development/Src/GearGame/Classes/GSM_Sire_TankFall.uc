/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GSM_Sire_TankFall extends GearSpecialMove
	config(Pawn);


/** animation */
var()	GearPawn.BodyStance	BS_Animation;

var() name JumpAnimName;
var() name FallAnimName;
var() float ChanceToFall;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local name AnimName;
	Super.SpecialMoveStarted(bForced,PrevMove);

	AnimName = JumpAnimName;
	if(FRand() < ChanceToFall)
	{
		AnimName = FallAnimName;
	}
	BS_Animation.AnimName[BS_FullBody]=AnimName;
	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);


	PawnOwner.SetPhysics(Phys_None);
	TogglePawnCollision(PawnOwner, FALSE);
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate, RBA_Translate);
	PawnOwner.Mesh.RootMotionMode = RMM_Translate;

	AdjustForSpawnTubeDrawScale(Gearpawn_LocustSireBase(PawnOwner).SpawnerDrawScale);
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.SetPhysics(Phys_Falling);
	// Disable end of animation notification. 
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
	AdjustForSpawnTubeDrawScale(1.0f);
	TogglePawnCollision(PawnOwner, TRUE);
}

function AdjustForSpawnTubeDrawScale(float SpawnTubeScale)
{
	PawnOwner.Mesh.RootMotionAccelScale.X = SpawnTubeScale;
	PawnOwner.Mesh.RootMotionAccelScale.Y = SpawnTubeScale;
	PawnOwner.Mesh.RootMotionAccelScale.Z = SpawnTubeScale;
}

defaultproperties
{
	JumpAnimName="tank_jump"
	FallAnimName="fall"
	bLockPawnRotation=TRUE
	bDisableMovement=TRUE
	bDisablePhysics=TRUE

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
}