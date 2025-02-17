/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class InterpActor_COGDerrickBase extends InterpActor_GearBasePlatform
	native
	notplaceable;

var() protected transient COG_DerrickWheelsBase				WheelActor;

var protected const editconst class<COG_DerrickWheelsBase>	WheelActorClass;

/** Skelmesh we carry around for previewing in the editor. */
var protected transient SkeletalMeshComponent				EditorWheelsPreviewMesh;

/** TRUE to suppress all audio for this derrick.  Useful for derricks in the distance. */
var() protected const bool		bNoAudio;

var transient bool				bPlayerAudioIsPlaying;
var transient bool				bNonPlayerAudioIsPlaying;

///** True if the derrick's engine is running. */
var() repnotify protected bool			bEngineRunning;

/** Not actual RPM, but an abstract concept of engine "effort" above the idle baseline, used to control engine audio.  Normalized to be [0..1]. */
var() protected transient float	EngineRPM;

var protected transient float	LastEngineRPM;
var() protected const float		EngineRPMInterpSpeed;

var() protected const vector2d	EngineRPMAccelRange;
var() protected const vector2d	EngineRPMVelRange;

var protected transient vector	LastActualVelocity;

//
// AUDIO
//

/** Plays only on derricks ridden by a player.  Nonspatialized stereo loops. */
var AudioComponent					PlayerEngineMainLoop;
var() protected transient SoundCue	PlayerEngineMainLoopCue;

var() protected const SoundCue		StopEngineSound;
var() protected const SoundCue		StartEngineSound;


/** TRUE means this derrick will have no glass. */
var() protected const bool			bNoGlass;
var protected StaticMeshComponent	GlassMesh;

var() protected const bool			bNoJiggle;

/** This forces the wheels to always update, even when derrick is not rendered. This is obviously more costly, but will fix popping issues when you first see the derrick */
var() bool							bAlwaysUpdateWheels;


enum EDerrickStripeColor
{
	DerrickStripe_DefaultBlue,
	DerrickStripe_LightBlue,
	DerrickStripe_White,
	DerrickStripe_Yellow,
	DerrickStripe_Green,
};
var protected const array<LinearColor>				AvailableStripeColors;

/** MIC for stripe coloring */
var protected transient MaterialInstanceConstant	MIC_Stripe;

var() protected const EDerrickStripeColor			StripeColor;

/** replicated to tell clients about hatch state */
var repnotify bool bHatchOpened;
/** replicated to tell clients about cabin state */
var repnotify bool bCabinOpened;

cpptext
{
	virtual void TickSpecial(FLOAT DeltaTime);
	virtual void PostNetReceive();
};

replication
{
	if (bNetDirty)
		bEngineRunning, bHatchOpened, bCabinOpened;
}

/** Internal.  Sets up an audiocomponent. */
simulated protected function AudioComponent CreateNonSpatializedComponent(SoundCue Cue, Pawn P)
{
	local AudioComponent AC;

	AC = P.CreateAudioComponent(Cue, FALSE, TRUE);
	if (AC != None)
	{
		AC.bUseOwnerLocation = TRUE;
		AC.bAllowSpatialization = FALSE;
		AC.bAutoDestroy = FALSE;
		AC.bShouldRemainActiveIfDropped = TRUE;
		AC.Play();
	}

	return AC;
}


simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	// spawn the wheels
	WheelActor = Spawn(WheelActorClass, self,, Location, Rotation,, TRUE);
	if (WheelActor != None)
	{
		WheelActor.AttachToDerrick(self);
	}

	// set up glass
	if (bNoGlass)
	{
		GlassMesh.SetHidden(TRUE);
		DetachComponent(GlassMesh);
		GlassMesh = None;
	}
	else
	{
		GlassMesh.SetShadowParent(StaticMeshComponent);
	}

	// propagate forced lod level to other meshes
	if (GlassMesh != None)
	{
		GlassMesh.ForcedLODModel = StaticMeshComponent.ForcedLODModel;
	}

	if (StripeColor != DerrickStripe_DefaultBlue)
	{
		MIC_Stripe = StaticMeshComponent.CreateAndSetMaterialInstanceConstant(4);
		MIC_Stripe.SetVectorParameterValue('Derrick_Stripe_Color', AvailableStripeColors[StripeColor]);
	}

	// oddly, it seems as if the staticmeshcomponent can lose it's DLE when it is placed
	// and then moved to a different sublevel.  This makes sure it is set properly.
	if (StaticMeshComponent != None)
	{
		StaticMeshComponent.SetLightEnvironment(LightEnvironment);
	}

	// since we're in the game, not the editor, release the ref to this so the object can be destroyed
	DetachComponent(EditorWheelsPreviewMesh);
	EditorWheelsPreviewMesh = None;


	// so we have just attached a bunch of stuff we need to recheck this and call it again.   Super.PostBeginPlay() could be moved lower
	// but don't want to break anything
	if (bShouldShadowParentAllAttachedActors)
	{
		SetShadowParentOnAllAttachedComponents();
	}
}

/** Set up audio to be player, nonplayer, or neither.  Should be rarely called. */
simulated protected event SetupAudio(bool bDoingPlayerAudio, bool bDoingNonPlayerAudio, Pawn LocallyControlledPawn)
{
	if (bDoingPlayerAudio)
	{
		// make sure the stereo loops exist and are playing
		if (PlayerEngineMainLoop == None)
		{
			PlayerEngineMainLoop = CreateNonSpatializedComponent(PlayerEngineMainLoopCue, LocallyControlledPawn);
		}
		if (PlayerEngineMainLoop != None)
		{
			PlayerEngineMainLoop.Play();
		}
	}
	else
	{
		if (PlayerEngineMainLoop != None)
		{
			PlayerEngineMainLoop.Stop();
		}
	}

	WheelActor.SetupAudio(bDoingPlayerAudio, bDoingNonPlayerAudio);

	bPlayerAudioIsPlaying = bDoingPlayerAudio;
	bNonPlayerAudioIsPlaying = bDoingNonPlayerAudio;
}

/** Overridden to clean up wheel actor. */
simulated event ShutDown()
{
	Super.ShutDown();
	WheelActor.Destroy();
	WheelActor = None;
}


//simulated function bool EncroachingOn(Actor Other)
//{
//	local bool bEncroachUnresolved;
//
//	if ( (Other.Base == self) && (GearPawn(Other) != None) ) //  /* && Other.bHardAttach && !Other.bBlockActors*/ )
//	{
//		// allow this encroachment without complaint
//		`log("***"@self@"encroached"@Other);
//		bEncroachUnresolved = FALSE;
//	}
//	else
//	{
//		bEncroachUnresolved = super.EncroachingOn(Other);
//	}
//
//	return bEncroachUnresolved;
//}
//
//simulated function Tick(float DeltaTime)
//{
//	super.Tick(DeltaTime);
//
//	if (bPlayerAudioIsPlaying)
//	{
//		`log("**** "@self@"stereo:"@PlayerEngineMainLoop.SoundCue@PlayerEngineMainLoop.IsPlaying()@PlayerEngineMainLoop.CurrentVolume);
//		`log("     "@"wheels left:"@WheelActor.EngineAudio_Player_Left.SoundCue@WheelActor.EngineAudio_Player_Left.IsPlaying()@WheelActor.EngineAudio_Player_Left.CurrentVolume);
//		`log("     "@"wheels rght:"@WheelActor.EngineAudio_Player_Right.SoundCue@WheelActor.EngineAudio_Player_Right.IsPlaying()@WheelActor.EngineAudio_Player_Right.CurrentVolume);
//	}
//}


/** Overridden to hide the wheels, too. */
simulated function OnToggleHidden(SeqAct_ToggleHidden Action)
{
	Super.OnToggleHidden(Action);

	// keep wheels hidden-status in sync.
	// this isn't ideal, but SetHidden is final in the engine.  It should work for how we're using the derrick, though.
	if (bHidden != WheelActor.bHidden)
	{
		WheelActor.SetWheelsHidden(bHidden);
	}

	if (bHidden)
	{
		SetupAudio(FALSE, FALSE, None);
	}
	else
	{
		// make sure our audio will automatically restart in tick()
		bPlayerAudioIsPlaying = FALSE;
		bNonPlayerAudioIsPlaying = FALSE;
	}
}

simulated function StopEngine()
{
	SetupAudio(FALSE, FALSE, None);

	PlaySound(StopEngineSound, true);

	bEngineRunning = FALSE;
}

simulated function StartEngine()
{
	PlaySound(StartEngineSound, true);
	bEngineRunning = TRUE;

	// next tick will setup engine audio
	bPlayerAudioIsPlaying = FALSE;
	bNonPlayerAudioIsPlaying = FALSE;

}

simulated function OnDerrickControl(SeqAct_DerrickControl Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		// "start engine" input
		if (!bEngineRunning)
		{
			StartEngine();
		}
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		if (bEngineRunning)
		{
			// "stop engine" input
			StopEngine();
		}
	}
	else if (Action.InputLinks[2].bHasImpulse)
	{
		// "Open Hatch"
		WheelActor.OpenHatch();
		bHatchOpened = true;
	}
	else if (Action.InputLinks[3].bHasImpulse)
	{
		// "Close Hatch"
		WheelActor.CloseHatch();
		bHatchOpened = false;
	}
	else if (Action.InputLinks[4].bHasImpulse)
	{
		// "Open Cabin Door"
		WheelActor.OpenCabinDoor();
		bCabinOpened = true;
	}
	else if (Action.InputLinks[5].bHasImpulse)
	{
		// "Close Cabin Door"
		WheelActor.CloseCabinDoor();
		bCabinOpened = false;
	}
	else if (Action.InputLinks[6].bHasImpulse)
	{
		// "Left Arm Prepare"
		WheelActor.LeftArmPrepare();
	}
	else if (Action.InputLinks[7].bHasImpulse)
	{
		// "Left Arm Deploy"
		WheelActor.LeftArmDeploy();
	}
	else if (Action.InputLinks[8].bHasImpulse)
	{
		// "Right Arm Prepare"
		WheelActor.RightArmPrepare();
	}
	else if (Action.InputLinks[9].bHasImpulse)
	{
		// "Right Arm Deploy"
		WheelActor.RightArmDeploy();
	}

	ForceNetRelevant();
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == nameof(bHatchOpened))
	{
		if (WheelActor != None)
		{
			if (bHatchOpened)
			{
				WheelActor.OpenHatch();
			}
			else
			{
				WheelActor.CloseHatch();
			}
		}
	}
	else if (VarName == nameof(bCabinOpened))
	{
		if (WheelActor != None)
		{
			if (bCabinOpened)
			{
				WheelActor.OpenCabinDoor();
			}
			else
			{
				WheelActor.CloseCabinDoor();
			}
		}
	}
	else if (VarName == nameof(bEngineRunning))
	{
		if (bEngineRunning)
		{
			StartEngine();
		}
		else
		{
			StopEngine();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function ApplyCheckpointRecord(const out InterpActor.CheckpointRecord Record)
{
	Super.ApplyCheckpointRecord(Record);

	// apply bHidden to wheels
	if (WheelActor != None)
	{
		WheelActor.SetWheelsHidden(bHidden);
	}
}

defaultproperties
{
	BlockRigidBody=TRUE

	CollisionType=COLLIDE_BlockAll

	bEngineRunning=TRUE

	Begin Object Name=MyLightEnvironment
		bEnabled=TRUE
	End Object

	Begin Object Class=StaticMeshComponent Name=GlassMesh0
	End Object
	GlassMesh=GlassMesh0
	Components.Add(GlassMesh0)

	// this obj is designed solely to provide collision for rigid bodies
	Begin Object Class=StaticMeshComponent Name=RBCollision0
	End Object
	Components.Add(RBCollision0)


	EngineRPMInterpSpeed=0.35f

	EngineRPMAccelRange=(X=30,Y=2500)
	EngineRPMVelRange=(X=100,Y=1400)

	bStasis=TRUE

	bNoJiggle=TRUE

	StripeColor=DerrickStripe_DefaultBlue
	AvailableStripeColors(DerrickStripe_DefaultBlue)=(R=0.f,G=0.02f,B=0.09f)
	AvailableStripeColors(DerrickStripe_LightBlue)=(R=0.02f,G=0.12f,B=0.3f)
	AvailableStripeColors(DerrickStripe_White)=(R=0.6f,G=0.55f,B=0.4f)
	AvailableStripeColors(DerrickStripe_Yellow)=(R=0.4f,G=0.25f,B=0.05f)
	AvailableStripeColors(DerrickStripe_Green)=(R=0.09f,G=0.13f,B=0.01f)

	bAlwaysConfineToClampedBase=TRUE
}
