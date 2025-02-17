/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Defines an emergence hole for AI to climb out of
 */
class GearSpawner_EmergenceHoleBase extends GearSpawner
	abstract
	DependsOn(GearPawn)
	native
	placeable;

/** When the ehole opens this explosion will go off **/
var GearExplosion ExplosionTemplate;

/** The spawner's light environment */
var() DynamicLightEnvironmentComponent LightEnvironment;

/** We need to have these as the LightingChannels struct can't be edited in a normal fashion **/
var(LightingStaticMesh) const bool bForceDirectLightMap;
var(LightingStaticMesh) const LightingChannelContainer LightingChannels;
/** Whether to override the lightmap resolution defined in the static mesh */
var(LightingStaticMesh) const bool bOverrideLightMapResolution;
/** Light map resolution used if bOverrideLightMapResolution is TRUE */
var(LightingStaticMesh) const int	 OverriddenLightMapResolution;


/** Static mesh component */
var() editinline StaticMeshComponent StaticMesh;

var editinline export StaticMeshComponent FogMesh;

/** Cork used when the hole has been sealed */
var editinline export CylinderComponent CorkCylinder;

/** Mesh that animates on hole open/collapse */
var editinline export SkeletalMeshComponent Mesh;
var vector MeshTranslationAmount;
var float MeshTranslationStartTime, MeshTranslationEndTime;

/** Mesh to unhide when opening */
var() editinline export StaticMeshComponent BrokenMesh;

/** Mesh to apply material instance to */
var() editinline export StaticMeshComponent CoverMesh;

/** Mesh that is the base box */
var() editinline export StaticMeshComponent BaseMesh;

/** Meshes that appear when the hole is closed */
var() editinline export array<StaticMeshComponent> RubbleMeshes;

/** Animation played to open the hole */
var Name OpenAnimationName;

/** Various camera shakes when opening */
var() ScreenShakeStruct FirstShake, SecondShake;

/** Sounds to play during the opening */
var array<InterpTrackSound.SoundTrackKey> OpenSounds;

/** Material instance applied to the meshes */
var() editinline export MaterialInstanceConstant MatInst;
var InterpCurveFloat RampInterpCurve;
var float RampStartTime, RampEndTime;

/** Texture used for the material instance */
var() Texture2D SourceTexture;
/** Material applied to the other meshes */
var() MaterialInterface SourceMaterial;

/** Particles to kick off when opening/closing */
struct native TimedParticleStruct
{
	var float Time;
	var ParticleSystemComponent ParticleSystem;
};
var array<TimedParticleStruct> OpenParticles, CloseParticles;

/** Current timer used for sounds/fx */
var float CurrentFXTime;

/** Lights associated with open/close */
var() editinline PointLightComponent ExplosionLight, ImulsionLight;

/** Current hole status */
enum EHoleStatus
{
	HS_ReadyToOpen,
	HS_Opening,
	HS_Open,
	HS_Closing,
	HS_Closed,
};
var repnotify EHoleStatus HoleStatus;
/** last replicated hole status (client only) */
var EHoleStatus LastHoleStatus;

struct native TimedDelegate
{
	var delegate<TemplateDelegate> Delegate;
	var float Time;
};
var array<TimedDelegate> OpenDelegates;

/** Should this open at double speed? */
var() bool bFastOpen;
/** Should this start already open? */
var() bool bStartOpen;
/** Should this hole ignore damage (for scripting reasons) */
var() bool bIgnoreDamage;
/** Disable the imulsion light? */
var() bool bDisableImulsionLight;

/** List of AI spawned from here, in case we need to kill them when closed */
var array<GearPawn> Spawns;

/** Time, in seconds, between reminders to close the hold. */
const OpenReminderFrequencySec = 15.f;

/** Last time a reminder was issued about this hole being open. */
var private transient float LastReminderTime;

/** TRUE if we should issue reminders that the hole is open (via guds) */
var private bool			bDoReminders;

/** Optional POI so LDs don't have to drop them on every spawner */
var GearPointOfInterest_EHole	POI;
/** Whether to spawn a POI for this spawner or not */
var() bool bSpawnPOI;


cpptext
{
	virtual void PostLoad();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}



replication
{
	if (bNetDirty)
		HoleStatus;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if (Role == ROLE_Authority)
	{
		if (bStartOpen)
		{
			OpenHoleImmediately();
		}
		LastReminderTime = WorldInfo.TimeSeconds;
	}

	if (bDisableImulsionLight)
	{
		ImulsionLight.SetLightProperties(0.f);
		ImulsionLight.SetEnabled(FALSE);
	}

	if ( bSpawnPOI && (Role == ROLE_Authority) )
	{
		POI = Spawn( class'GearPointOfInterest_EHole', self );
	}
}

/** Toggles the POI on and off */
function TogglePOI( bool bOn )
{
	if ( bSpawnPOI )
	{
		if ( bOn )
		{
			POI.EnablePOI();
		}
		else
		{
			POI.DisablePOI();
		}
	}
}

simulated function OpenHoleImmediately()
{
	local int Idx;

	HoleStatus = HS_Open;
	Mesh.SetTranslation(default.Mesh.Translation + MeshTranslationAmount);
	if (MatInst != None)
	{
		MatInst.SetScalarParameterValue('Ramp',1.f);
		CoverMesh.SetMaterial(0,MatInst);
	}
	UnhideBrokenMesh();
	TurnOffMeshCollision();
	PlayOpenAnimationImmediately();
	// activate the particle systems, since some loop
	for (Idx = 0; Idx < OpenParticles.Length; Idx++)
	{
		OpenParticles[Idx].ParticleSystem.ActivateSystem();
	}
}

/** Templated delegate for timed events */
delegate TemplateDelegate();

native final function ApplySourceMaterial();

/**
 * Overridden to check for the hole needing to be opened.
 */
event bool GetSpawnSlot(out int out_SpawnSlotIdx, out vector out_SpawnLocation, out rotator out_SpawnRotation)
{
	if (HoleStatus == HS_Open)
	{
		return Super.GetSpawnSlot(out_SpawnSlotIdx,out_SpawnLocation,out_SpawnRotation);
	}
	else
	if (HoleStatus == HS_ReadyToOpen)
	{
		OpenHole();
	}
	return FALSE;
}

function DeActivated()
{
	Super.DeActivated();
	if (HoleStatus != HS_Closing && HoleStatus != HS_Closed)
	{
		CloseHole(TRUE);
	}
}

/* Reset()
reset actor to initial state - used when restarting level without reloading.
*/
simulated function Reset()
{
	ResetToOpen();

	Super.Reset();
}

simulated final function ResetToOpen()
{
	local int Idx;

	Mesh.SetTranslation(default.Mesh.Translation);
	if (MatInst != None)
	{
		MatInst.SetScalarParameterValue('Ramp',0.f);
	}

	// reset everything in case this isn't the first time we're opening
	for (Idx = 0; Idx < RubbleMeshes.Length; Idx++)
	{
		RubbleMeshes[Idx].SetHidden(TRUE);
		RubbleMeshes[Idx].SetActorCollision(FALSE,FALSE);
		RubbleMeshes[Idx].SetBlockRigidBody(FALSE);
	}
	CorkCylinder.SetActorCollision(FALSE,FALSE);
	FogMesh.SetHidden(FALSE);

	BrokenMesh.SetHidden(TRUE);
	BrokenMesh.SetActorCollision(FALSE,FALSE);
	BrokenMesh.SetBlockRigidBody(FALSE);

	BaseMesh.SetHidden(FALSE);
	BaseMesh.SetActorCollision(TRUE,TRUE);
	BaseMesh.SetBlockRigidBody(TRUE);
	CoverMesh.SetActorCollision(TRUE,TRUE);
	CoverMesh.SetBlockRigidBody(TRUE);

	for (Idx = 0; Idx < OpenParticles.Length; Idx++)
	{
		OpenParticles[Idx].ParticleSystem.DeactivateSystem();
	}

	HoleStatus = HS_ReadyToOpen;
}

/**
 * Plays the hole opening animation and readys this spawner for spawning.
 */
simulated final function OpenHole()
{
	// set the status
	HoleStatus = HS_Opening;
	CurrentFXTime = 0.f;
	TogglePOI( true );
}

/**
 * Closes the bastard up.
 */
simulated final function CloseHole(optional bool bQuiet)
{
	local int Idx;

	if (Role == ROLE_Authority)
	{
		for (Idx = 0; Idx < Factories.Length; Idx++)
		{
			if (Factories[Idx] != None)
			{
				Factories[Idx].NotifySpawnerDisabled(self);
			}
		}
	}
	for (Idx = 0; Idx < OpenParticles.Length; Idx++)
	{
		OpenParticles[Idx].ParticleSystem.DeactivateSystem();
		//DetachComponent(OpenParticles[Idx].ParticleSystem);
	}
	ImulsionLight.SetLightProperties(0.f);
	ImulsionLight.SetEnabled(FALSE);
	// tell parent i am dying
	StartingToDie();
	// seal with some rubble
	for (Idx = 0; Idx < RubbleMeshes.Length; Idx++)
	{
		RubbleMeshes[Idx].SetHidden(FALSE);
		RubbleMeshes[Idx].SetActorCollision(TRUE,TRUE);
		RubbleMeshes[Idx].SetBlockRigidBody(TRUE);
	}
	// turn on the cork so players can walk over it
	CorkCylinder.SetActorCollision(TRUE,TRUE);
	// hide the fog
	FogMesh.SetHidden(TRUE);
	// if not quiet,
	if (TRUE || !bQuiet)
	{
		// then go through the full closing process
		HoleStatus = HS_Closing;
		CurrentFXTime = 0.f;
		SetTimer( 1.f,FALSE,nameof(FinishedClosing) );
	}
	else
	{
		// otherwise just close now
		FinishedClosing();
	}
}

final function FinishedOpening()
{
	if (HoleStatus == HS_Opening)
	{
		HoleStatus = HS_Open;
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_EHoleOpened, None);
		LastReminderTime = WorldInfo.TimeSeconds;
	}
}

final function FinishedClosing()
{
	if (HoleStatus == HS_Closing)
	{
		// tell parent i am finally dead
		FinishedDying();
		HoleStatus = HS_Closed;
	}
}

simulated function Tick(float DeltaTime)
{
	local AudioComponent AudioComp;
	local float LastFXTime, NewRamp;
	local int Idx;
	local vector NewMeshTranslation;
	local bool bAllSpawned;

	Super.Tick(DeltaTime);

	// update sounds/fx
	if (HoleStatus == HS_Opening)
	{
		LastFXTime = CurrentFXTime;
		CurrentFXTime += (bFastOpen ? DeltaTime * 2.f : DeltaTime) * 1.5f;
		// check for any timed function calls
		for (Idx = 0; Idx < OpenDelegates.Length; Idx++)
		{
			if (CurrentFXTime >= OpenDelegates[Idx].Time && LastFXTime <= OpenDelegates[Idx].Time)
			{
				// call the function
				TemplateDelegate = OpenDelegates[Idx].Delegate;
				TemplateDelegate();
			}
		}
		// check for any timed sounds
		for (Idx = 0; Idx < OpenSounds.Length; Idx++)
		{
			if (CurrentFXTime >= OpenSounds[Idx].Time && LastFXTime <= OpenSounds[Idx].Time)
			{
				// start the sound
				AudioComp = CreateAudioComponent(OpenSounds[Idx].Sound,FALSE,TRUE);
				if ( AudioComp != None )
				{
					AudioComp.VolumeMultiplier = OpenSounds[Idx].Volume;
					AudioComp.PitchMultiplier = OpenSounds[Idx].Pitch;
					AudioComp.Play();
				}
			}
		}
		// timed particles
		for (Idx = 0; Idx < OpenParticles.Length; Idx++)
		{
			if (CurrentFXTime >= OpenParticles[Idx].Time && LastFXTime <= OpenParticles[Idx].Time)
			{
				// activate the particles
				OpenParticles[Idx].ParticleSystem.ActivateSystem();
			}
		}
		// update the skeletal mesh translation
		if (CurrentFXTime <= MeshTranslationStartTime)
		{
			NewMeshTranslation = default.Mesh.Translation;
		}
		else
		if (CurrentFXTime > MeshTranslationStartTime && CurrentFXTime < MeshTranslationEndTime)
		{
			NewMeshTranslation = VLerp(default.Mesh.Translation,default.Mesh.Translation + MeshTranslationAmount, (CurrentFXTime - MeshTranslationStartTime) / (MeshTranslationEndTime - MeshTranslationStartTime));
		}
		else
		if (CurrentFXTime >= MeshTranslationEndTime)
		{
			NewMeshTranslation = default.Mesh.Translation + MeshTranslationAmount;
		}
		Mesh.SetTranslation(NewMeshTranslation);
		// update the material instance ramp
		if (CurrentFXTime <= RampStartTime)
		{
			NewRamp = 0.f;
		}
		else
		if (CurrentFXTime > RampStartTime && CurrentFXTime < RampEndTime)
		{
			NewRamp = Lerp(0.f,1.f,(CurrentFXTime - RampStartTime) / (RampEndTime - RampStartTime));
		}
		else
		{
			NewRamp = 1.f;
		}
		if (MatInst != None)
		{
			MatInst.SetScalarParameterValue('Ramp',NewRamp);
		}
	}
	else
	if (HoleStatus == HS_Closing)
	{
		LastFXTime = CurrentFXTime;
		CurrentFXTime += DeltaTime;
		for (Idx = 0; Idx < CloseParticles.Length; Idx++)
		{
			if (CurrentFXTime >= CloseParticles[Idx].Time && LastFXTime <= CloseParticles[Idx].Time)
			{
				// activate the particles
				CloseParticles[Idx].ParticleSystem.ActivateSystem();
			}
		}
	}
	else if (HoleStatus == HS_Open)
	{
		// deal with reminder timer
		if ( bDoReminders && bActive && !bAllSpawned && (WorldInfo.TimeSince(LastReminderTime) > OpenReminderFrequencySec) )
		{
			bAllSpawned = TRUE;

			for (Idx=0; Idx<Factories.length; ++Idx)
			{
				if (!Factories[Idx].bAllSpawned)
				{
					bAllSpawned = FALSE;
					break;
				}
			}

			if (!bAllSpawned)
			{
				GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_EHoleOpenReminder, None);
				LastReminderTime = WorldInfo.TimeSeconds;
			}
			else
			{
				// turn off reminders altogether, we'll skip the factory loop subsequently.
				bDoReminders = FALSE;
			}
		}
	}
}

simulated function HandleOpeningEmergenceHole()
{
	if (HoleStatus != HS_Open && HoleStatus != HS_Opening)
	{
		if (HoleStatus != HS_ReadyToOpen)
		{
			ResetToOpen();
		}
		OpenHoleImmediately();
	}
}

function HandleClosingEmergenceHole()
{
	if (HoleStatus != HS_Closed && HoleStatus != HS_Closing)
	{
		ResetToOpen();
	}
}

function OnOpenEmergenceHole(SeqAct_OpenEmergenceHole Action)
{
	//`log(GetFuncName()@HoleStatus@Action.InputLinks[0].bHasImpulse@Action.InputLinks[1].bHasImpulse);
	if (Action.InputLinks[0].bHasImpulse)
	{
		HandleOpeningEmergenceHole();
	}
	else
	if (Action.InputLinks[1].bHasImpulse)
	{
		HandleClosingEmergenceHole();
	}
}

event TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int Idx;
	if ((HoleStatus == HS_Opening || HoleStatus == HS_Open) && !bIgnoreDamage)
	{
		if( ((HitLocation.Z < Location.Z) || (DamageCauser != None && VSize2D(DamageCauser.Location - Location) < 256.f))
			&& ClassIsChildOf(DamageType,class'GDT_Explosive')
			&& DamageType != class'GDT_EmergenceHoleOpening'
			)
		{
			Killer = EventInstigator.Pawn;
			CloseHole();

			if (class<GDT_FragGrenade>(DamageType) != None)
			{
				GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_EHoleClosedWithGrenade, EventInstigator.Pawn);
			}

			// kill everything still spawning
			for (Idx = 0; Idx < Spawns.Length; Idx++)
			{
				if (Spawns[Idx] != None && Spawns[Idx].bSpawning)
				{
					Spawns[Idx].TakeDamage(99999,EventInstigator,Spawns[Idx].Location,vect(0,0,0),class'GDT_Explosive');
				}
			}
		}
	}
	Super.TakeDamage(Damage,EventInstigator,HitLocation,Momentum,DamageType,HitInfo,DamageCauser);
}

simulated function PlayOpenAnimation()
{
	local AnimNodeSequence SeqNode;
	SeqNode = AnimNodeSequence(Mesh.Animations);

	if (SeqNode != None)
	{
		SeqNode.SetAnim(OpenAnimationName);
		SeqNode.PlayAnim(FALSE,1.f,0.f);
	}

	if( bDisableImulsionLight == FALSE )
	{
		ImulsionLight.SetEnabled( TRUE );
	}
}

simulated function PlayOpenAnimationImmediately()
{
	local AnimNodeSequence SeqNode;
	SeqNode = AnimNodeSequence(Mesh.Animations);

	if (SeqNode != None)
	{
		SeqNode.SetAnim(OpenAnimationName);
		SeqNode.SetPosition(99999.f,FALSE);
	}

	if( bDisableImulsionLight == FALSE )
	{
		ImulsionLight.SetEnabled( TRUE );
	}
}

function DoOpeningEHoleRadialDamage()
{
	local GearExplosionActor ExplosionActor;

	ExplosionActor = Spawn(class'GearExplosionActor',,, Location+vect(0,0,32), rot(0,0,0));

	if( ExplosionActor != None )
	{
		ExplosionActor.Explode( ExplosionTemplate );	
	}
}

simulated final function ClearAttached()
{
	local GearPawn_COGGear Victim;
	foreach VisibleCollidingActors( class'GearPawn_COGGear', Victim, 384, Location )
	{
		if (Victim.Base == self)
		{
			Victim.SetBase(None);
		}
	}
}

simulated final function BumpPlayers()
{
	local GearPawn_COGGear Victim;
	foreach VisibleCollidingActors( class'GearPawn_COGGear', Victim, 384, Location )
	{
		if (Victim.Base == self)
		{
			Victim.SetBase(None);
			Victim.AddVelocity(vect(0,0,384),Victim.Location,class'GDT_EmergenceHoleOpening');
		}
	}
}

simulated final function UnhideBrokenMesh()
{
	BaseMesh.SetHidden(TRUE);
	BrokenMesh.SetHidden(FALSE);
	BrokenMesh.SetActorCollision(TRUE,TRUE);
	BrokenMesh.SetBlockRigidBody(TRUE);
}

simulated final function TurnOffMeshCollision()
{
	BaseMesh.SetActorCollision(FALSE,FALSE);
	BaseMesh.SetBlockRigidBody(FALSE);
	CoverMesh.SetActorCollision(FALSE,FALSE);
	CoverMesh.SetBlockRigidBody(FALSE);
}

simulated final function PlayFirstCameraShake()
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		PC.ClientPlayCameraShake(FirstShake, true);
	}
}

simulated final function PlaySecondCameraShake()
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		PC.ClientPlayCameraShake(SecondShake, true);
	}
}

/**
 * Overridden to trigger the emergence move for AI.
 * @fixme - support player emerging?
 */
event HandleSpawn(GearPawn NewSpawn, int SlotIdx)
{
	local GearAI AI;

	Super.HandleSpawn(NewSpawn,SlotIdx);

	Spawns[Spawns.Length] = NewSpawn;
	AI = GearAI(NewSpawn.Controller);

	if( AI != None )
	{
		AI.DoEmerge(SM_Emerge_Type1);
		NewSpawn.LocationOfEholeEmergedFrom = Location;
	}
}

event UnRegisterFactory(SeqAct_AIFactory Factory)
{
	super.UnRegisterFactory(Factory);

	// then check to see if all factories are off
	if (Factories.Length == 0)
	{
		bDoReminders = FALSE;
	}
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'HoleStatus')
	{
		switch (HoleStatus)
		{
			case HS_Opening:
				OpenHole();
				break;
			case HS_Open:
				if (LastHoleStatus != HS_Opening)
				{
					OpenHoleImmediately();
				}
				break;
			case HS_Closing:
				if (LastHoleStatus == HS_ReadyToOpen)
				{
					// we need to open it first to get rid of some meshes and such
					OpenHoleImmediately();
					HoleStatus = HS_Closing; // since OpenHoleImmediately() will change it
				}
				CloseHole(false);
				break;
			case HS_Closed:
				if (LastHoleStatus != HS_Closing)
				{
					if (LastHoleStatus == HS_ReadyToOpen)
					{
						// we need to open it first to get rid of some meshes and such
						OpenHoleImmediately();
						HoleStatus = HS_Closing; // since OpenHoleImmediately() will change it
					}
					CloseHole(true);
				}
				break;
			case HS_ReadyToOpen:
				Reset();
				break;
		}
		LastHoleStatus = HoleStatus;
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

defaultproperties
{
}
