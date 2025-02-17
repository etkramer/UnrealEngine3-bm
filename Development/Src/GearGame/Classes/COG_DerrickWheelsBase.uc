/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class COG_DerrickWheelsBase extends Actor
	native;

var() protected const SkeletalMeshComponent Mesh;

var transient InterpActor_COGDerrickBase	DerrickBody;

var() protected transient SkelControlLookAt SteeringLookatControl;
var() protected const Name					SteeringLookatControlName;

var() protected const Name					SteeringPivotBoneName;
var protected transient vector				LastSteeringPivotLoc;

/** Last steering rotation, in this actor's local space. */
var protected transient rotator				LastLocalSteerRot;

/** Controls how fast the steering wheels rotate */
var() protected const float					SteerInterpSpeed;

//
// Audio!
//

// Audio for player derrick
// main engine loop is nonspatialized stereo loop controlled on the derrick interpactor
var transient AudioComponent	EngineAudio_Player_Left;
var protected const SoundCue	EngineAudio_Player_LeftCue;
var transient AudioComponent	EngineAudio_Player_Right;
var protected const SoundCue	EngineAudio_Player_RightCue;

// suspension audio
var protected AudioComponent	SuspensionAudio_Player_FrontRight;
var protected const SoundCue	SuspensionAudio_Player_FrontRightCue;
var protected AudioComponent	SuspensionAudio_Player_FrontLeft;
var protected const SoundCue	SuspensionAudio_Player_FrontLeftCue;

var() protected const vector2d	SuspensionAudioRandomTimeRange;
var() protected const float		SuspensionAudioVelocityThreshold;


// Audio for non-player derricks
/** Just one big composite cue attached to the derrick's center. */
var transient AudioComponent	DerrickAudio_NonPlayer;
var protected const SoundCue	DerrickAudio_NonPlayerCue;


//
// Wheels
// 

struct native DerrickWheel
{
	var() float				WheelRadius;
	var() Name				BoneName;
	var() Name				SkelControlName;
	var SkelControlWheel	WheelControl;

	var matrix				WheelRefPoseTM;

	/** Controls how fast a free-spinning wheel will decelerate. */
	var() float				FreeSpinningFrictionDecel;

	/** Contact information */
	var bool				bHasContact;
	var	bool				bHadContact;
	var vector				LastContactPos;

	/** How fast the wheel is rolling, in deg/sec. */
	var float				RollVel;

	var rotator				LastRot;

	var vector				Location;
	var rotator				Rotation;

	var() float				WheelRoll;

	/** Local space offset from the wheelbone, defines the position from which to derive the raycast endpoints */
	var() vector			WheelRayBaseOffset;

	/** How long since this wheel did a trace */
	var int					FramesSinceLastTrace;

	/** How much to move wheel each frame to achieve desired displacement */
	var float				WheelDispPerFrameAdjust;
};
var() array<DerrickWheel>	Wheels;

/** How many frames between doing traces against the world for each wheel */
var()	int					WheelTraceInterval;

var protected transient bool bForceUpdateWheelsNextTick;

/** If the size of the derrick on screen is less than this, don't update wheels */
var()	float				MinDistFactorForUpdate;

/** Cached anim node refs we use to control the derrick anims. */
var protected transient GearAnim_BlendList	HatchAnimControl;
var protected transient GearAnim_BlendList	CabinDoorAnimControl;
var protected transient GearAnim_BlendList	LeftArmAnimControl;
var protected transient GearAnim_BlendList	RightArmAnimControl;

var protected transient bool				bHatchOpen;
var protected transient bool				bCabinDoorOpen;
var protected transient bool				bLeftArmPrepared;
var protected transient bool				bLeftArmDeployed;
var protected transient bool				bRightArmPrepared;
var protected transient bool				bRightArmDeployed;

var protected SkeletalMeshComponent			LeftGrindLift;
var protected transient GearAnim_BlendList	LeftGrindLiftAnimControl;
var protected SkeletalMeshComponent			RightGrindLift;
var protected transient GearAnim_BlendList	RightGrindLiftAnimControl;

var protected const SoundCue				HatchOpenCue;
var protected const SoundCue				HatchCloseCue;
var protected const SoundCue				CabinDoorOpenCue;
var protected const SoundCue				CabinDoorCloseCue;
var protected const SoundCue				ArmPrepareCue;
var protected const SoundCue				ArmDeployCue;

/** Jiggle vars */
var protected transient AnimNodeBlend		JiggleAnimControl;
var protected transient vector 				LastVelocity;
var() protected const float					AccelJiggleScalar;
var() protected const float					JiggleFadeTime;


/** Internal.  True to skip any interpolation in the steering/wheel code.  Useful for coming back from a delay in updates. */
var protected transient bool				bResetInterpolation;

cpptext
{
private:
	/** Internal.  Updates wheel positions/orientations, etc.  */
	void UpdateWheels(FLOAT DeltaTime);
	void UpdateSteering(FLOAT DeltaTime);

public:
	virtual void TickSpecial(FLOAT DeltaTime);
	virtual UBOOL InStasis();
	void UpdateEngineAudio(FLOAT EngineSpeed);
};


simulated native final function CacheWheelRefPoseTMs();


/** Internal.  Sets up an audiocomponent. */
simulated protected function AudioComponent CreateAndAttachAudioComponent(SoundCue Cue, Name SocketName, optional bool bPlayImmediately)
{
	local AudioComponent AC;
	AC = CreateAudioComponent(Cue, FALSE, TRUE,,, FALSE);
	if (AC != None)
	{
		AC.bUseOwnerLocation = TRUE;
		Mesh.AttachComponentToSocket(AC, SocketName);
		AC.bAutoDestroy = FALSE;
		AC.bShouldRemainActiveIfDropped = TRUE;

		if (bPlayImmediately)
		{
			AC.Play();
		}
	}

	return AC;
}

simulated protected function TriggerSuspensionSound_FrontRight()
{
	// maybe scale to velocity?
	if (VSize(Velocity) > SuspensionAudioVelocityThreshold)
	{
		SuspensionAudio_Player_FrontRight = PlayDerrickAudioComponent(SuspensionAudio_Player_FrontRight, SuspensionAudio_Player_FrontRightCue, 'SuspensionAudio_FrontRight');
	}
	SetTimer( RandRange(SuspensionAudioRandomTimeRange.X, SuspensionAudioRandomTimeRange.Y), FALSE, nameof(TriggerSuspensionSound_FrontRight) );
}

simulated protected function TriggerSuspensionSound_FrontLeft()
{
	if (VSize(Velocity) > SuspensionAudioVelocityThreshold)
	{
		SuspensionAudio_Player_FrontLeft = PlayDerrickAudioComponent(SuspensionAudio_Player_FrontLeft, SuspensionAudio_Player_FrontLeftCue, 'SuspensionAudio_FrontLeft');
	}
	SetTimer( RandRange(SuspensionAudioRandomTimeRange.X, SuspensionAudioRandomTimeRange.Y), FALSE, nameof(TriggerSuspensionSound_FrontLeft) );
}


simulated function AttachToDerrick(InterpActor_COGDerrickBase Derrick)
{
	DerrickBody = Derrick;

	SetBase(Derrick);
	SetRelativeLocation(vect(0,0,0));
	SetRelativeRotation(rot(0,0,0));

	Mesh.SetShadowParent(Derrick.StaticMeshComponent);
	Mesh.SetLightEnvironment(Derrick.LightEnvironment);

	Mesh.ForcedLODModel = Derrick.StaticMeshComponent.ForcedLODModel;

	if (Derrick.bHidden)
	{
		SetWheelsHidden(TRUE);
	}
}

simulated protected function AudioComponent PlayDerrickAudioComponent(AudioComponent AC, SoundCue Cue, Name SocketName)
{
	if (AC == None)
	{
		AC = CreateAndAttachAudioComponent(Cue, SocketName);
	}
	if (AC != None)
	{
		AC.Play();
	}
	return AC;
}

simulated protected function StopAudioComponent(AudioComponent AC, optional float FadeOutTime)
{
	if (AC != None)
	{
		AC.FadeOut(FadeOutTime, 0.f);
	}
}

simulated function SetupAudio(bool bDoingPlayerAudio, bool bDoingNonPlayerAudio)
{
	if (bDoingPlayerAudio)
	{
		EngineAudio_Player_Left = PlayDerrickAudioComponent(EngineAudio_Player_Left, EngineAudio_Player_LeftCue, 'EngineAudio_Left');
		EngineAudio_Player_Right = PlayDerrickAudioComponent(EngineAudio_Player_Right, EngineAudio_Player_RightCue, 'EngineAudio_Right');

		SetTimer( RandRange(SuspensionAudioRandomTimeRange.X, SuspensionAudioRandomTimeRange.Y), FALSE, nameof(TriggerSuspensionSound_FrontRight) );
		SetTimer( RandRange(SuspensionAudioRandomTimeRange.X, SuspensionAudioRandomTimeRange.Y), FALSE, nameof(TriggerSuspensionSound_FrontLeft) );
	}
	else
	{
		StopAudioComponent(EngineAudio_Player_Left);
		StopAudioComponent(EngineAudio_Player_Right);

		ClearTimer('TriggerSuspensionSound_FrontRight');
		ClearTimer('TriggerSuspensionSound_FrontLeft');
	}

	if (bDoingNonPlayerAudio)
	{
		DerrickAudio_NonPlayer = PlayDerrickAudioComponent(DerrickAudio_NonPlayer, DerrickAudio_NonPlayerCue, 'EngineAudio_Center');
	}
	else
	{
		StopAudioComponent(DerrickAudio_NonPlayer);
	}
}

//simulated function Tick(float DeltaTime)
//{
//	super.Tick(DeltaTime);
//	//DebugRenderAudio();
//}

//function DebugRenderAudioComponent(AudioComponent AC)
//{
//	if (AC != None)
//		if (AC.IsPlaying())
//			DrawDebugBox(AC.ComponentLocation, vect(64,64,64), 0, 255, 0, FALSE);
//		else
//			DrawDebugBox(AC.ComponentLocation, vect(64,64,64), 255, 0, 0, FALSE);
//}

//function DebugRenderAudio()
//{
//	if (!DerrickBody.bHidden)
//	{
//		DebugRenderAudioComponent(SuspensionAudio_Player_FrontRight);
//		DebugRenderAudioComponent(SuspensionAudio_Player_FrontLeft);
//
//		DebugRenderAudioComponent(EngineAudio_Player_Left);
//		DebugRenderAudioComponent(EngineAudio_Player_Right);
//	}
//}

function PostBeginPlay()
{
	local int Idx;

	super.PostBeginPlay();

	CacheWheelRefPoseTMs();

	for (Idx=0; Idx<Wheels.length; ++Idx)
	{
		Wheels[Idx].WheelControl = SkelControlWheel(Mesh.FindSkelControl(Wheels[Idx].SkelControlName));
		Wheels[Idx].FramesSinceLastTrace = Rand(WheelTraceInterval-1);
	}

	HatchAnimControl = GearAnim_BlendList(Mesh.FindAnimNode('HatchControl'));
	CabinDoorAnimControl = GearAnim_BlendList(Mesh.FindAnimNode('CabinDoorControl'));
	LeftArmAnimControl = GearAnim_BlendList(Mesh.FindAnimNode('LeftArmControl'));
	RightArmAnimControl = GearAnim_BlendList(Mesh.FindAnimNode('RightArmControl'));
	JiggleAnimControl = AnimNodeBlend(Mesh.FindAnimNode('JiggleControl'));
}


/** Anim control functions. */
simulated function OpenHatch()
{
	if (!bHatchOpen)
	{
		HatchAnimControl.SetActiveChild(0, 0.f);
		bHatchOpen = TRUE;
	}
}
simulated function CloseHatch()
{
	if (bHatchOpen)
	{
		HatchAnimControl.SetActiveChild(1, 0.f);
		bHatchOpen = FALSE;
	}
}
simulated function OpenCabinDoor()
{
	if (!bCabinDoorOpen)
	{
		CabinDoorAnimControl.SetActiveChild(0, 0.f);
		bCabinDoorOpen = TRUE;
	}
}
simulated function CloseCabinDoor()
{
	if (bCabinDoorOpen)
	{
		CabinDoorAnimControl.SetActiveChild(1, 0.f);
		bCabinDoorOpen = FALSE;
	}
}

// move to content object!!
simulated protected function SkeletalMeshComponent CreateGrindliftMesh(Name AttachName);

simulated function LeftArmPrepare()
{
	if (!bLeftArmPrepared)
	{
		LeftArmAnimControl.SetActiveChild(1, 0.f);

		LeftGrindLift = CreateGrindliftMesh('b_Lt_Boom_Pod');
		LeftGrindLiftAnimControl = GearAnim_BlendList(LeftGrindLift.FindAnimNode('GrindliftAnims'));

		LeftGrindLiftAnimControl.SetActiveChild(3, 0.f);

		bLeftArmPrepared = TRUE;
	}
}
simulated function LeftArmDeploy()
{
	if (!bLeftArmDeployed && bLeftArmPrepared)
	{
		LeftArmAnimControl.SetActiveChild(2, 0.f);
		LeftGrindLiftAnimControl.SetActiveChild(5, 0.f);
		bLeftArmDeployed = TRUE;
	}
}
simulated function RightArmPrepare()
{
	if (!bRightArmPrepared)
	{
		RightArmAnimControl.SetActiveChild(1, 0.f);

		RightGrindLift = CreateGrindliftMesh('b_Rt_Boom_Pod');
		RightGrindLiftAnimControl = GearAnim_BlendList(RightGrindLift.FindAnimNode('GrindliftAnims'));

		RightGrindLiftAnimControl.SetActiveChild(0, 0.f);
		
		bRightArmPrepared = TRUE;
	}
}
simulated function RightArmDeploy()
{
	if (!bRightArmDeployed && bRightArmPrepared)
	{
		RightArmAnimControl.SetActiveChild(2, 0.f);
		RightGrindLiftAnimControl.SetActiveChild(2, 0.f);
		bRightArmDeployed = TRUE;
	}
}

simulated function OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if (RightGrindLiftAnimControl.Children[0].Anim == SeqNode)
	{
		// rightdeploystart finished, play rightidle
		RightGrindLiftAnimControl.SetActiveChild(1, 0.f);
	}
	else if (LeftGrindLiftAnimControl.Children[3].Anim == SeqNode)
	{
		// leftdeploystart finished, play leftidle
		LeftGrindLiftAnimControl.SetActiveChild(4, 0.f);
	}

	// @todo, advance new anim by ExcessTime so it's seamless?

	super.OnAnimEnd(SeqNode, PlayedTime, ExcessTime);
}

simulated function SetWheelsHidden(bool bNewHidden)
{
	SetHidden(bNewHidden);

	if (bNewHidden)
	{
		bResetInterpolation = TRUE;
		SetCollision(FALSE, FALSE, TRUE);
	}
	else
	{
		bForceUpdateWheelsNextTick = TRUE;
		SetCollision(default.bCollideActors, default.bBlockActors, default.bIgnoreEncroachers);
	}
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	// client-side only
	RemoteRole=ROLE_None
	bHardAttach=true

	CollisionType=COLLIDE_BlockWeapons
	bCollideActors=TRUE
	bProjTarget=TRUE

	SteerInterpSpeed=2.f
	SteeringLookatControlName=SteeringPivot
	SteeringPivotBoneName=b_SteeringPivot

	SuspensionAudioRandomTimeRange=(X=8.f,Y=20.f)
	SuspensionAudioVelocityThreshold=-1.f

	WheelTraceInterval=10
	MinDistFactorForUpdate=0.3

	bStasis=TRUE

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent0
		bUpdateSkelWhenNotRendered=FALSE
		CollideActors=TRUE
		BlockActors=TRUE
		BlockNonZeroExtent=FALSE
		BlockZeroExtent=TRUE
		BlockRigidBody=FALSE
	End Object
	Mesh=SkeletalMeshComponent0
	CollisionComponent=SkeletalMeshComponent0
	Components.Add(SkeletalMeshComponent0)

	AccelJiggleScalar=0.015f
	JiggleFadeTime=2.f
}

