
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_GrapplingHook_Climb extends GearSpecialMove;

var BodyStance		BS_Throw, BS_Climb;
var Vector			GroundMarker, MarkerLocation;
var Rotator			MarkerRotation;
var GearAnim_Slot	GrapplingHookSlot;

var bool	bKillMeshTranslation;

// DEBUG
var bool	bDebug_ViewPawnGrappling;

/** This is the saved off value of bNoEncroachCheck which is restored at the end of the move. **/
var bool bSavedOffbNoEncroachCheck;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	bSavedOffbNoEncroachCheck = PawnOwner.bNoEncroachCheck;
	PawnOwner.bNoEncroachCheck = TRUE;

	// fixes some issues with complicated attachment setups and is unnecessary since the whole special move's location is based
	PawnOwner.bUpdateSimulatedPosition = false;

	// Set Up Pawn so he's ready to climb
	SetupPawnToClimb();

	//PlayPositioningTest();
	//PlayClimbingAnimation();
	PlayGrappleThrow();

	`if(`notdefined(FINAL_RELEASE))
	// debug
	if( bDebug_ViewPawnGrappling )
	{
		PawnOwner.ViewMe();
	}
	`endif
}

function SetupPawnToClimb()
{
	local Vector	X, Y, Z;

	PawnOwner.SetBase(None);
	PawnOwner.SetPhysics(PHYS_None);
	PawnOwner.SetHardAttach(TRUE);
	TogglePawnCollision(PawnOwner, FALSE);

	// Get marker position on ground
	GroundMarker = PawnOwner.GetGrapplingHookGroundMarker();
	MarkerLocation = PawnOwner.CurrentGrapplingMarker.Location;
	MarkerRotation = PawnOwner.CurrentGrapplingMarker.Rotation;

	// Move Pawn to ground marker to make sure he reaches attached grappling hook marker.
	// Marker could be oriented weirdly, so make sure we take that into account.
	if( bKillMeshTranslation )
	{
		PawnOwner.SetLocation(GroundMarker);
		PawnOwner.SetMeshTranslationOffset(vect(0,0,0));
		PawnOwner.Mesh.SetTranslation(vect(0,0,0));
	}
	else
	{
		GetAxes(MarkerRotation, X, Y, Z);
		PawnOwner.SetLocation(GroundMarker + Z * PawnOwner.GetCollisionHeight());
	}
	PawnOwner.bCanDoStepsSmoothing = FALSE;

	// Make sure Pawn is facing proper rotation.
	ForcePawnRotation(PawnOwner, MarkerRotation);

	// Base Pawn on whatever the marker is based on.
	`log(PawnOwner.WorldInfo.TimeSeconds @ Class @ GetFuncName() @ PawnOwner @ "setting base on" @ PawnOwner.CurrentGrapplingMarker.Base);
	PawnOwner.SetBase(PawnOwner.CurrentGrapplingMarker.Base);

	// Create Grappling Hook on updated Pawn position.
	PawnOwner.CreateGrapplingHook(GroundMarker, MarkerRotation);
	GrapplingHookSlot = PawnOwner.GetGrapplingHookAnimSlot();
}

/** Script Tick function. */
function Tick(float DeltaTime)
{
	Super.Tick(DeltaTime);

	//`log("PawnOwner.Mesh.Translation:" @ PawnOwner.Mesh.Translation @ "MeshTranslationOffset:" @ PawnOwner.MeshTranslationOffset);
}

function PlayPositioningTest()
{
	PawnOwner.BS_Play(BS_Climb, 1.f);
	PawnOwner.BS_SetPlayingFlag(BS_Climb, FALSE);
	PawnOwner.BS_SetPosition(BS_Climb, 3.5f);

	GrapplingHookSlot.PlayCustomAnim('GP_Release', SpeedModifier, 0.f, -1.f);
	GrapplingHookSlot.GetCustomAnimNodeSeq().bPlaying = FALSE;
	GrapplingHookSlot.GetCustomAnimNodeSeq().SetPosition(3.5f, FALSE);
}

function PlayGrappleThrow()
{
	PawnOwner.bAnimForceVelocityFromBase = TRUE;
	PawnOwner.bCanRoadieRun = FALSE;
	PawnOwner.BS_Play(BS_Throw, SpeedModifier, 0.2f / SpeedModifier, -1.f);
	PawnOwner.BS_SetAnimEndNotify(BS_Throw, TRUE);

	GrapplingHookSlot.PlayCustomAnim('GP_Release', SpeedModifier, 0.f, -1.f);
}

function CleanUpGrappleThrow()
{
	PawnOwner.BS_SetAnimEndNotify(BS_Throw, FALSE);
	PawnOwner.bAnimForceVelocityFromBase = PawnOwner.default.bAnimForceVelocityFromBase;
	PawnOwner.bCanRoadieRun = PawnOwner.default.bCanRoadieRun;
}

function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if( PawnOwner.BS_SeqNodeBelongsTo(SeqNode, BS_Throw) )
	{
		CleanUpGrappleThrow();
		PlayClimbingAnimation();
	}
	else
	{
		PawnOwner.EndSpecialMove();
	}
}

function PlayClimbingAnimation()
{
	local float		DistanceScale;
	local Vector	VectToGroundMarker, LocalVectToGroundMarker;

	VectToGroundMarker = (GroundMarker - MarkerLocation);
	LocalVectToGroundMarker = WorldToRelativeOffset(MarkerRotation, VectToGroundMarker);

	DistanceScale = VSize(LocalVectToGroundMarker) / VSize(PawnOwner.CurrentGrapplingMarker.MarkerOffset);

	PawnOwner.BS_Play(BS_Climb, SpeedModifier, 0.2f / SpeedModifier, 0.2f / SpeedModifier);
	PawnOwner.BS_SetAnimEndNotify(BS_Climb, TRUE);
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Climb, RBA_Translate, RBA_Translate, RBA_Translate);
	PawnOwner.Mesh.RootMotionMode = RMM_Relative;
	PawnOwner.Mesh.RootMotionAccelScale = Vect(1,1,1) * DistanceScale;

	`log("DistanceScale:" @ DistanceScale @ "RootMotionAccelScale:" @ PawnOwner.Mesh.RootMotionAccelScale
		@ "LocalVectToGroundMarker:" @ LocalVectToGroundMarker
		@ "ScaledMarkerOffset:" @ PawnOwner.CurrentGrapplingMarker.MarkerOffset * PawnOwner.Mesh.RootMotionAccelScale );

	GrapplingHookSlot.PlayCustomAnim('GP_Climb', SpeedModifier, 0.2f / SpeedModifier, 0.2f / SpeedModifier);

	// Set Timer when mantle over starts
	PawnOwner.SetTimer( 3.5f, FALSE, nameof(self.MantleOverNotify), self );
}

function PostClimbCleanup()
{
	PawnOwner.SetBase(None);
	PawnOwner.SetHardAttach(FALSE);
	PawnOwner.SetPhysics(PHYS_Falling);

	PawnOwner.bCanDoStepsSmoothing = PawnOwner.default.bCanDoStepsSmoothing;
	if( bKillMeshTranslation )
	{
		PawnOwner.Mesh.SetTranslation(PawnOwner.default.Mesh.Translation);
		PawnOwner.SetLocation(PawnOwner.Location - PawnOwner.Mesh.Translation);
	}

	TogglePawnCollision(PawnOwner, TRUE);

	PawnOwner.BS_SetAnimEndNotify(BS_Climb, FALSE);
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;

	PawnOwner.DestroyGrapplingHook();
}

function MantleOverNotify()
{
	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_GrapplingHookMantleOverEffort, true);
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	`if(`notdefined(FINAL_RELEASE))
	// debug
	if( bDebug_ViewPawnGrappling )
	{
		PawnOwner.CancelViewMe();
	}
	`endif

	PawnOwner.ClearTimer('MantleOverNotify', Self);
	CleanUpGrappleThrow();
	PostClimbCleanup();

	// Make sure frozen anim "Throw" has been stopped.
	PawnOwner.BS_Stop(BS_Throw);

	// @todo handle grapple if pawnowner has been killed during this special move...

	PawnOwner.bNoEncroachCheck = bSavedOffbNoEncroachCheck;
	PawnOwner.bUpdateSimulatedPosition = true;

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	bKillMeshTranslation=FALSE
	bDebug_ViewPawnGrappling=FALSE

	BS_Throw=(AnimName[BS_Std_Idle_Lower]="GP_Release",AnimName[BS_Std_Up]="GP_Release")
	BS_Climb=(AnimName[BS_FullBody]="GP_Climb")

	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE
	bDisableMovement=TRUE
	bDisableTurnInPlace=TRUE
	bShouldAbortWeaponReload=TRUE

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
	bDisableAI=TRUE
}
