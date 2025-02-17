/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_Crane_Base extends GearVehicle
	abstract
	native(Vehicle)
	nativereplication;

// Yaw stuff (actually rotates actor)
var()	float	MaxYawAngVel;
var()	float	YawTorque;
var()	float	YawDamping;

var()	int		MinYaw;
var()	int		MaxYaw;

var		float	YawAngVel;
/** Current yaw as float, to avoid 'partial' rotation amounts during simulation */
var		float	CurrentYawAng;

// Raise stuff (operates skel control)
var()	float	MaxRaiseAngVel;
var()	float	RaiseTorque;
var()	float	RaiseDamping;

var()	int		MinRaise;
var()	int		MaxRaise;

struct native CraneRaiseLimit
{
	var()	int LimitYawMin;
	var()	int LimitYawMax;
	var()	int MinRaise;
	var()	int MaxRaise;
	var()	bool bDisableSwingAtBottom;
	var()	bool bHardLimit;
};

/** How close to bottom stop to disable swinging */
var()	int		DisableSwingDist;
/** How quickly we fade out swinging */
var()	float	SwingFadeOutTime;

var()	array<CraneRaiseLimit>	CraneLimitSections;

var		bool	bForceAbove;
var		float	ForceAbovePitch;

/** If -1, forces raising down, if 1 forces raising up. Overrides manual input */
var		int		ForceRaise;

var		float	RaiseAngVel;
/** Current arm raise angle as float, to avoid 'partial' rotation amounts during simulation */
var		float	CurrentRaiseAng;

/** When getting out - this is where it will try and place you */
var(Exit)	Actor	ExitLocationActor;

var		SkelControlSingleBone	ArmRaiseControl;
var		SkelControlSingleBone	HangControl;
var		GearSkelCtrl_Spring		SpringControl;
var		SkelControlSingleBone	DrumControl;

var		float	DrumSpinFactor;

struct native CraneSpringState
{
	var	vector	SpringPos;
	var	float	SpringVelX;
	var	float	SpringVelY;
	var	float	SpringVelZ;
	var byte	bNewData;
};

/** Struct used to replicate state of spring node */
var		CraneSpringState	SpringState;

/** Name (in the ObjComment field) of the Matinee that represents the crane camera path. */
var() const protected Name	CraneCameraMatineeName;

/** We will map [MinYaw..MaxYaw] to [0..1] in the Matinee. */
var protected transient SeqAct_Interp CraneCameraMatinee;

/** Arm moving audio. */
var protected const SoundCue CraneArmMovingLoop;
var protected transient AudioComponent CraneArmMovingLoopAC;
/** Yaw Angular velocity that corresponds with volume 1.f.  Interp is linear to 0. */
var protected const float CraneArmAudioMaxYawAngVel;

/** "Engine" audio. */
var protected const SoundCue CraneEngineIdleLoop;
var protected transient AudioComponent CraneEngineIdleLoopAC;
var protected const SoundCue CraneEngineStartLoop;
var protected const SoundCue CraneEngineStopLoop;




replication
{
	if(Role == ROLE_Authority)
		RaiseAngVel, CurrentRaiseAng, YawAngVel, CurrentYawAng, SpringState, bForceAbove, ForceAbovePitch;
}

cpptext
{
	virtual void physInterpolating(FLOAT DeltaTime);
	virtual INT* GetOptimizedRepList(BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
};

/** Internal.  Plays given sound, non-replicated. */
simulated function protected AudioComponent CranePlayLocalSound(SoundCue Sound, optional float FadeInTime)
{
	local AudioComponent AC;

	if( Sound != None )
	{
		AC = CreateAudioComponent(Sound, FALSE, TRUE);
		if( AC != None )
		{
			AC.bUseOwnerLocation	= TRUE;
			AC.bAutoDestroy			= TRUE;
			AC.bStopWhenOwnerDestroyed = TRUE;
			AttachComponent(AC);
		}
		if (AC != None)
		{
			AC.FadeIn(FadeInTime, 1.f);
			return AC;
		}
	}

	return AC;
}


simulated function PostBeginPlay()
{
	local InterpActor AttachedMover;

	Super.PostBeginPlay();

	// Init float rotation
	CurrentYawAng = Rotation.Yaw;

	// Look for the camera path
	CraneCameraMatinee = FindCraneCameraMatinee();

	// don't save attached movers in checkpoints
	// they shouldn't be moving beyond being attached to us so any handling here should take care of it
	foreach BasedActors(class'InterpActor', AttachedMover)
	{
		AttachedMover.bShouldSaveForCheckpoint = false;
	}
}

/** Internal, used to locate the Crame camera matinee if one exists. */
simulated native protected function SeqAct_Interp FindCraneCameraMatinee() const;

simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	Super.PostInitAnimTree(SkelComp);

	if(SkelComp == Mesh)
	{
		ArmRaiseControl = SkelControlSingleBone(Mesh.FindSkelControl('ArmRaise'));
		CurrentRaiseAng = ArmRaiseControl.BoneRotation.Pitch;

		HangControl = SkelControlSingleBone(Mesh.FindSkelControl('HangControl'));

		SpringControl = GearSkelCtrl_Spring(Mesh.FindSkelControl('SpringNode'));
		SpringControl.bNoZSpring = TRUE;

		DrumControl = SkelControlSingleBone(Mesh.FindSkelControl('DrumControl'));
	}
}

simulated event Tick(float DeltaTime)
{
	Super.Tick(DeltaTime);

	// On server, pack state
	if(Role == ROLE_Authority)
	{
		SpringState.SpringPos = SpringControl.BoneLocation;
		SpringState.SpringVelX = SpringControl.BoneVelocity.X;
		SpringState.SpringVelY = SpringControl.BoneVelocity.Y;
		SpringState.SpringVelZ = SpringControl.BoneVelocity.Z;
		SpringState.bNewData = 1;
	}
	// On client, unpack state
	else
	{
		if(SpringState.bNewData == 1)
		{
			SpringControl.BoneLocation = SpringState.SpringPos;
			SpringControl.BoneVelocity.X = SpringState.SpringVelX;
			SpringControl.BoneVelocity.Y = SpringState.SpringVelY;
			SpringControl.BoneVelocity.Z = SpringState.SpringVelZ;
			SpringState.bNewData = 0;
		}
	}

	if (bDriving)
	{
		UpdateMatineeCamera();

		if (CraneArmMovingLoopAC != None)
		{
			// update arm rotation audio
			CraneArmMovingLoopAC.VolumeMultiplier = FMin(Abs(YawAngVel/CraneArmAudioMaxYawAngVel), 1.f);
		}
	}
}

/** Called when a client presses X while in the vehicle. (server only) */
reliable server function ServerPressedX()
{
	/* disabled - LD forces the exit through scripting
	DriverLeave(TRUE);
	*/
}

/** Function that looks up raise limits for the current yaw. */
simulated native final function bool GetRaiseLimits(out int OutMinRaise, out int OutMaxRaise, out byte bStopSwingLimit);

/** Used for drawing debug info when in crane */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local Canvas	Canvas;
	local int CurrentMinRaise, CurrentMaxRaise;
	local byte bStopAtLimit;

	super.DisplayDebug(HUD, out_YL, out_YPos);

	Canvas = HUD.Canvas;
	Canvas.SetDrawColor(255, 255, 255);

	GetRaiseLimits(CurrentMinRaise, CurrentMaxRaise, bStopAtLimit);

	Canvas.DrawText(" CurrentYawAng:" @ CurrentYawAng @ "(Min:" @ MinYaw @ "Max:" @ MaxYaw @ ")");
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Canvas.DrawText(" CurrentRaiseAng:" @ -CurrentRaiseAng @ "(Min:" @ CurrentMinRaise @ "Max:" @ CurrentMaxRaise @ ")");
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Canvas.DrawText(" Spring Strength:" @ SpringControl.ControlStrength @ " StopSwingLimit:" @ bStopAtLimit);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);
}

function rotator GetExitRotation(Controller C)
{
	//drawdebugline(ExitLocationActor.Location,ExitLocationActor.Location + vector(ExitLocationActor.Rotation)*100,255,255,0,TRUE);
	return ExitLocationActor.Rotation;
}

function bool PlaceExitingDriver(optional Pawn ExitingDriver)
{
	local vector NewActorPos;
	local bool bSuccess;

	if ( ExitingDriver == None )
	{
		ExitingDriver = Driver;
	}

	if ( ExitingDriver == None )
	{
		return FALSE;
	}

	if(ExitLocationActor != None)
	{
		NewActorPos = ExitLocationActor.Location + (ExitingDriver.MaxStepHeight)*vect(0,0,1);

		// try placing driver on floor
		bSuccess = ExitingDriver.SetRotation(ExitLocationActor.Rotation);
		if(!bSuccess)
		{
			`log("EXITCRANE: SetRotation Failed");
			return FALSE;
		}

		bSuccess = ExitingDriver.SetLocation(NewActorPos);
		if(!bSuccess)
		{
			`log("EXITCRANE: SetLocation Failed");
			return FALSE;
		}

		// Fix mesh translation
		ExitingDriver.Mesh.SetTranslation(ExitingDriver.default.Mesh.Translation);

		return TRUE;
	}
	else
	{
		`log("PlaceExitingDriver: No ExitLocationActor"@self);
		return FALSE;
	}
}

simulated function bool WantsCrosshair(PlayerController PC)
{
	return FALSE;
}

simulated function vector GetCameraWorstCaseLoc(int SeatIndex)
{
	return GetCameraFocus(SeatIndex);
}

simulated function rotator GetVehicleSpaceCamRotation(float DeltaTime, bool bPassenger)
{
	return Rotation + rot(0,16384,0);
}

function bool DriverEnter(Pawn P)
{
	local array<Object> ObjVars;
	local CameraActor Camera;
	local int i;

	if (Super.DriverEnter(P))
	{
		if (CraneCameraMatinee != None)
		{
			// 0 is "play"
			CraneCameraMatinee.ForceActivateInput(0);
			// 3 is "pause"
			CraneCameraMatinee.ForceActivateInput(3);

			// set the driver to view the matinee camera instead of us
			CraneCameraMatinee.GetObjectVars(ObjVars);
			for (i = 0; i < ObjVars.length; i++)
			{
				Camera = CameraActor(ObjVars[i]);
				if (Camera != None)
				{
					PlayerController(Controller).SetViewTarget(Camera);
					break;
				}
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

simulated function DrivingStatusChanged()
{
	if (bDriving)
	{
		if (CraneCameraMatinee != None)
		{
			UpdateMatineeCamera();
		}

		CranePlayLocalSound(CraneEngineStartLoop);
		CraneEngineIdleLoopAC = CranePlayLocalSound(CraneEngineIdleLoop, 1.f);
		CraneArmMovingLoopAC = CranePlayLocalSound(CraneArmMovingLoop, 1.f);
	}
	else
	{
		if (CraneCameraMatinee != None && Role == ROLE_Authority)
		{
			// 2 is "stop"
			CraneCameraMatinee.ForceActivateInput(2);
		}

		CranePlayLocalSound(CraneEngineStopLoop);
		if (CraneEngineIdleLoop != None)
		{
			CraneEngineIdleLoopAC.FadeOut(0.2f, 0.f);
		}
		if (CraneArmMovingLoopAC != None)
		{
			CraneArmMovingLoopAC.FadeOut(0.2f, 0.f);
		}
	}

	super.DrivingStatusChanged();
}


/** Updates matinee to proper location based on current yaw.  Client and server. */
simulated final protected function UpdateMatineeCamera()
{
	local float NewPos;

	if (CraneCameraMatinee != None && CraneCameraMatinee.bActive)
	{
		// normalized matinee position, [0..1]
		NewPos = float(Rotation.Yaw - MinYaw) / float(MaxYaw - MinYaw);
		NewPos = FClamp(NewPos, 0.01, 0.99);		// a kludge to ensure that we never reach the ends of the matinee and have it automatically stop

		// convert to matinee time.
		NewPos *= (CraneCameraMatinee.InterpData.InterpLength - 0.01f);

		CraneCameraMatinee.SetPosition(NewPos);
	}
}

/** Handle kismet action for forcing pitch up */
function OnCraneForceUp(SeqAct_CraneForceUp Action)
{
	`log("AAAA");
	bForceAbove = TRUE;
	ForceAbovePitch = Action.ForceAbovePitch;
}

defaultproperties
{
	Physics=PHYS_Interpolating
	bNoDelete=true
}
