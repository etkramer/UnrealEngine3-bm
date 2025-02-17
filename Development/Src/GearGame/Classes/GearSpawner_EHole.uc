/**
 * Defines an emergence hole for AI to climb out of
 *
 * This is the actor which is the gameplay side of things.  It keeps track of the state of the ehole and then sends events
 * to the prefab that this spawner is referencing.  That prefab then will do the visuals.  This allows the Content teams to make
 * arbitrary spawner visualization.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearSpawner_EHole extends GearSpawner
	DependsOn(GearPawn,GearSpawner_EmergenceHoleBase)
	native
	placeable;


/** The ehole visuals in prefab form! **/
var() PrefabInstance EHolePrefab;

/** When the ehole opens this explosion will go off **/
var GearExplosion ExplosionTemplate;


var repnotify EHoleStatus HoleStatus;
/** last replicated hole status (client only) */
var EHoleStatus LastHoleStatus;


var array<TimedDelegate> OpenDelegates;

/** Should this open at double speed? */
var() bool bFastOpen;
/** Should this start already open? */
var() bool bStartOpen;
/** Should this hole ignore damage (for scripting reasons) */
var() bool bIgnoreDamage;


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


replication
{
	if (bNetDirty)
		HoleStatus;
}

/** @param TagOfEvent this is the name of the even to trigger (usually Open or OpenImmediately) **/
native simulated function OpenEHole_Visuals( const out String TagOfEvent );

native simulated function OpenEHoleNormal_Visuals();

native simulated function OpenEHoleImmediately_Visuals();

native simulated function ClosEHole_Visuals();



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
			DoOpeningEHoleRadialDamage(); // We spawn the POI whent he hole opens 
		}
		else
		{
			POI.DisablePOI();
		}
	}
}

simulated function OpenHoleImmediately()
{
	HoleStatus = HS_Open;

	GUDEHoleAlert();

	TogglePOI( true );

	OpenEHoleImmediately_Visuals();
}

/** Templated delegate for timed events */
delegate TemplateDelegate();



/**
 * Returns the first available spawn slot index, or -1 if
 * none are available.
 * This is our TEMP function which doesn't used the Location +  instead it gets the exact location to spawn from the SpawnSlots
 */
event bool GetSpawnSlotTemp(out int out_SpawnSlotIdx, out vector out_SpawnLocation, out rotator out_SpawnRotation)
{
	local int BaseIdx, Idx;
	local array<int> Indices;
	if (bActive)
	{
		// do a quick shuffle of the slots
		Indices[Indices.Length] = RandRange(0,SpawnSlots.Length-1);
		BaseIdx = 0;
		Idx = Indices[BaseIdx];
		while (--Idx >= 0)
		{
			if (FRand() > 0.5f)
			{
				Indices.Insert(BaseIdx+1,1);
				Indices[BaseIdx+1] = Idx;
			}
			else
			{
				Indices.Insert(BaseIdx,1);
				Indices[BaseIdx] = Idx;
				BaseIdx++;
			}
		}
		Idx = Indices[BaseIdx];
		while (++Idx < SpawnSlots.Length)
		{
			if (FRand() > 0.5f)
			{
				Indices.Insert(BaseIdx+1,1);
				Indices[BaseIdx+1] = Idx;
			}
			else
			{
				Indices.Insert(BaseIdx,1);
				Indices[BaseIdx] = Idx;
				BaseIdx++;
			}
		}
		// and then pick the first available one
		for (BaseIdx = 0; BaseIdx < Indices.Length; BaseIdx++)
		{
			Idx = Indices[BaseIdx];
			if (SpawnSlots[Idx].bEnabled && (SpawnSlots[Idx].SpawningPawn == None || SpawnSlots[Idx].SpawningPawn.Health <= 0 || !SpawnSlots[Idx].SpawningPawn.bSpawning))
			{
				out_SpawnLocation = (SpawnSlots[Idx].LocationOffset >> Rotation);
				out_SpawnRotation = SpawnSlots[Idx].RotationOffset;
				out_SpawnSlotIdx = Idx;
				//DrawDebugCoordinateSystem( out_SpawnLocation, out_SpawnRotation, 10.0f );
				return TRUE;
			}
		}
	}

	return FALSE;
}



/**
 * Overridden to check for the hole needing to be opened.
 */
event bool GetSpawnSlot(out int out_SpawnSlotIdx, out vector out_SpawnLocation, out rotator out_SpawnRotation)
{
	if (HoleStatus == HS_Open)
	{
		return GetSpawnSlotTemp(out_SpawnSlotIdx,out_SpawnLocation,out_SpawnRotation);
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
	HoleStatus = HS_ReadyToOpen;
}

/**
 * Plays the hole opening animation and readys this spawner for spawning.
 */
simulated final function OpenHole()
{
	// set the status
	HoleStatus = HS_Open;

	SetTimer( 3.4f, FALSE, nameof(DoTogglePOIToTrue) );

	OpenEHoleNormal_Visuals();
}

simulated final function DoTogglePOIToTrue()
{
	TogglePOI( TRUE );
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

	ClosEHole_Visuals();

	// tell parent i am dying
	StartingToDie();

	// if not quiet,
	if (TRUE || !bQuiet)
	{
		// then go through the full closing process
		HoleStatus = HS_Closing;

		SetTimer( 1.f,FALSE,nameof(FinishedClosing) );
	}
	else
	{
		// otherwise just close now
		FinishedClosing();
	}
}

final function GUDEHoleAlert()
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

	local int Idx;

	local bool bAllSpawned;

	Super.Tick(DeltaTime);

	// update sounds/fx
	if (HoleStatus == HS_Opening)
	{
// 		LastFXTime = CurrentFXTime;
// 		CurrentFXTime += (bFastOpen ? DeltaTime * 2.f : DeltaTime) * 1.5f;
// 		// check for any timed function calls
// 		for (Idx = 0; Idx < OpenDelegates.Length; Idx++)
// 		{
// 			if (CurrentFXTime >= OpenDelegates[Idx].Time && LastFXTime <= OpenDelegates[Idx].Time)
// 			{
// 				// call the function
// 				TemplateDelegate = OpenDelegates[Idx].Delegate;
// 				TemplateDelegate();
// 			}
// 		}
	}
	else
	if (HoleStatus == HS_Closing)
	{

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

simulated function HandleOpeningEmergenceHole( bool bOpenImmediately )
{
	if (HoleStatus != HS_Open && HoleStatus != HS_Opening)
	{
		if (HoleStatus != HS_ReadyToOpen)
		{
			ResetToOpen();
		}
		if( bOpenImmediately == TRUE )
		{
			OpenHoleImmediately();
		}
		else
		{
			OpenHole();
		}
	}
}

function HandleClosingEmergenceHole()
{
	if (HoleStatus != HS_Closed && HoleStatus != HS_Closing)
	{
		ClosEHole_Visuals();
		ResetToOpen();
	}
}

function OnOpenEmergenceHole(SeqAct_OpenEmergenceHole Action)
{
	//`log(GetFuncName()@HoleStatus@Action.InputLinks[0].bHasImpulse@Action.InputLinks[1].bHasImpulse);
	if (Action.InputLinks[0].bHasImpulse)
	{
		HandleOpeningEmergenceHole( Action.bOpenImmediately );
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



/**
 * Overridden to triggle the emergence move for AI.
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
		if( SpawnSlots[SlotIdx].EmergeAnim == EHEA_CrawlUp )
		{
			AI.DoEmerge(SM_Emerge_Type1);
		}
		else if( SpawnSlots[SlotIdx].EmergeAnim == EHEA_Jump )
		{
			AI.DoEmerge(SM_Emerge_Type2);
		}
		else // EHEA_Random
		{
			if( FRand() < 0.50f )
			{
				AI.DoEmerge(SM_Emerge_Type2);
			}
			else
			{
				AI.DoEmerge(SM_Emerge_Type1);
			}
		}

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
	bDoReminders=TRUE

	bEdShouldSnap=TRUE
	bBlockActors=TRUE
	bCollideActors=TRUE
	bGameRelevant=TRUE
	bMovable=FALSE
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true
	bOnlyDirtyReplication=true

	bSpawnPOI=TRUE


// 	OpenDelegates(0)=(Time=2.5f,Delegate=GearSpawner_EmergenceHole.PlayFirstCameraShake)
// 	OpenDelegates(1)=(Time=3.5f,Delegate=GearSpawner_EmergenceHole.PlaySecondCameraShake)
// 	OpenDelegates(2)=(Time=3.85f,Delegate=GearSpawner_EmergenceHole.PlayOpenAnimation)
// 	OpenDelegates(8)=(Time=4.f,Delegate=GearSpawner_EmergenceHole.BumpPlayers)
// 	OpenDelegates(3)=(Time=4.23f,Delegate=GearSpawner_EmergenceHole.UnhideBrokenMesh)
// 	OpenDelegates(4)=(Time=6.85f,Delegate=GearSpawner_EmergenceHole.DoOpeningEHoleRadialDamage)
// 	OpenDelegates(5)=(Time=7.f,Delegate=GearSpawner_EmergenceHole.TurnOffMeshCollision)
// 	OpenDelegates(6)=(Time=7.2f,Delegate=GearSpawner_EmergenceHole.ClearAttached)
// 	OpenDelegates(7)=(Time=9.3f,Delegate=GearSpawner_EmergenceHole.FinishedOpening)


	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=TRUE
		AlwaysLoadOnClient=FALSE
		AlwaysLoadOnServer=FALSE
		bIsScreenSizeScaled=TRUE
		ScreenSize=0.0025
	End Object
	Components.Add(Sprite)


	// explosion when the ehole opens
	Begin Object Class=GearExplosion Name=ExploTemplate0
	    MyDamageType=class'GDT_EmergenceHoleOpening'
		MomentumTransferScale=0.f	// Scale momentum defined in DamageType
		ActorClassToIgnoreForDamage=class'GearGame.GearPawn_LocustBase'
		ActorClassToIgnoreForKnockdownsAndCringes=class'GearGame.GearPawn_LocustBase'

		FractureMeshRadius=100.0
		FracturePartVel=300.0

		Damage=700
		DamageRadius=512
		DamageFalloffExponent=1.0f

		KnockDownRadius=700
		CringeRadius=1024
	End Object
	ExplosionTemplate=ExploTemplate0

	SpawnSlots.Empty() // we don't want to have any implicit spawnslots as those are populated AGearSpawner_EHole::OpenEHole_Visuals
}
