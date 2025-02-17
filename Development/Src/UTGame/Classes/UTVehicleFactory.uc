/**
 * Vehicle spawner.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTVehicleFactory extends NavigationPoint
	abstract
	native(Vehicle)
	nativereplication
	placeable;

/** full package.class for the vehicle class. You should set this in the default properties, NOT VehicleClass.
 * this indirection is needed for the cooker, so it can fully clear references to vehicles that won't be spawned on the target platform
 * if the direct class reference were in the default properties, this wouldn't be possible without deleting the factory outright,
 * which we can't do without breaking paths
 */
var string VehicleClassPath;
/** actual vehicle class to spawn. DO NOT SET THIS IN DEFAULT PROPERTIES - set VehicleClassPath instead */
var		class<UTVehicle>	VehicleClass;
var		UTVehicle			ChildVehicle;

var		float			SpawnZOffset;

var		float			RespawnProgress;		/** Timer for determining when to spawn vehicles */
var		float			RespawnRateModifier;

/** Reverse spawn direction depending on which team controls vehicle factory */
var()   bool            bMayReverseSpawnDirection;

/** Not applicable to Warfare */
var()	bool			bStartNeutral;

/** Whether vehicles spawned at this factory are initially team locked */
var		bool			bHasLockedVehicle;

/** vehicle factory can't be activated while this is set */
var() bool bDisabled;

/** if set, replicate ChildVehicle reference */
var bool bReplicateChildVehicle;

var		UTGameObjective	ReverseObjective;		/** Reverse spawn dir if controlled by same team controlling this objective */
var     int             TeamNum;

var		vector			HUDLocation;

/** This array holds the initial gun rotations for a spawned vehicle. */
var() array<Rotator>	InitialGunRotations;

/** allows setting this vehicle factory to only spawn when one team controls this factory */
var() enum ETeamSpawning
{
	TS_All,
	TS_AxonOnly,
	TS_NecrisOnly
} TeamSpawningControl;

/** If set, vehicles from this factory will be key vehicles (for AI) and show up on minimap */
var() bool bKeyVehicle;
/** if set, force bAvoidReversing to true on the vehicle for the AI */
var() bool bForceAvoidReversing;

/** if set, vehicle factory doesn't do anything on PS3 */
var() bool bIgnoreOnPS3;

cpptext
{
	virtual void CheckForErrors();
	virtual void TickSpecial( FLOAT DeltaSeconds );
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	virtual void Spawned();
	virtual void PostLoad();
	virtual void StripData(UE3::EPlatformType TargetPlatform);
}

replication
{
	if (bNetDirty && Role == ROLE_Authority)
		bHasLockedVehicle;
	if (bNetDirty && Role == ROLE_Authority && bReplicateChildVehicle)
		ChildVehicle;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( Role == ROLE_Authority )
	{
		if ( UTGame(WorldInfo.Game) != None )
		{
			UTGame(WorldInfo.Game).ActivateVehicleFactory(self);
		}
		else
		{
			bStartNeutral = true;
			Activate(255);
		}
	}
	else
	{
		AddToClosestObjective();
	}
}

simulated function AddToClosestObjective()
{
	local UTGameObjective O, Best;
	local float Distance, BestDistance;

	foreach WorldInfo.AllNavigationPoints(class'UTGameObjective', O)
	{
		Distance = VSize(Location - O.Location);
		if ( (Best == None) || (Distance < BestDistance) )
		{
			BestDistance = Distance;
			Best = O;
		}
	}
	if ( Best != None )
	{
		Best.VehicleFactories[Best.VehicleFactories.Length] = self;
	}
}

// Called after PostBeginPlay.
//
simulated event SetInitialState()
{
	bScriptInitialized = true;
}

/** function used to update where icon for this actor should be rendered on the HUD
 *  @param NewHUDLocation is a vector whose X and Y components are the X and Y components of this actor's icon's 2D position on the HUD
 */
simulated native function SetHUDLocation(vector NewHUDLocation);

simulated function RenderMapIcon(UTMapInfo MP, Canvas Canvas, UTPlayerController PlayerOwner, LinearColor FinalColor)
{
	local LinearColor DrawColor;
	if ( !bHasLockedVehicle )
		return;

	DrawColor = MakeLinearColor(0,0,0,0.6);
	MP.DrawRotatedTile(Canvas,class'UTHUD'.default.IconHudTexture, HUDLocation, Rotation.Yaw + 16384, VehicleClass.Default.MapSize * 1.05, VehicleClass.Default.IconCoords, DrawColor);
	FinalColor.A = 0.6;
	MP.DrawRotatedTile(Canvas,class'UTHUD'.default.IconHudTexture, HUDLocation, Rotation.Yaw + 16384, VehicleClass.Default.MapSize, VehicleClass.Default.IconCoords, FinalColor);
}

function bool CanActivateForTeam(byte T)
{
	local UTGameReplicationInfo GRI;

	GRI = UTGameReplicationInfo(WorldInfo.GRI);
	return ( TeamSpawningControl == TS_All || GRI == None ||
		(TeamSpawningControl == TS_AxonOnly ? !GRI.IsNecrisTeam(T) : GRI.IsNecrisTeam(T)) );
}

function Activate(byte T)
{
	if (!bDisabled && (!bIgnoreOnPS3 || !WorldInfo.IsConsoleBuild(CONSOLE_PS3)) && CanActivateForTeam(T))
	{
		TeamNum = T;
		GotoState('Active');
	}
}

function Deactivate()
{
	local vector HitLocation, HitNormal;

	GotoState('');
	TeamNum = 255;
	if (ChildVehicle != None && !ChildVehicle.bDeleteMe && ChildVehicle.bTeamLocked)
	{
		if (UTGame(WorldInfo.Game).MatchIsInProgress())
		{
			HitLocation = Location;
			ChildVehicle.Health = -2 * ChildVehicle.HealthMax;
			ChildVehicle.TearOffMomentum = vect(0,0,1);
			TraceComponent(HitLocation, HitNormal, ChildVehicle.Mesh, ChildVehicle.Location, Location);
			ChildVehicle.Died(None, class'UTDmgType_Telefrag', HitLocation);
		}
		else
		{
			ChildVehicle.Destroy();
		}
	}
}

function TarydiumBoost(float Quantity);

/** called when someone starts driving our child vehicle */
function VehicleTaken()
{
	TriggerEventClass(class'UTSeqEvent_VehicleFactory', None, 1);
	bHasLockedVehicle = false;
	// it's possible that someone could enter and immediately exit the vehicle, but if that happens we mark the
	// vehicle as a navigation obstruction and the AI will use that codepath to avoid it, so this extra cost isn't necessary
	ExtraCost = 0;
}

function VehicleDestroyed( UTVehicle V )
{
	TriggerEventClass(class'UTSeqEvent_VehicleFactory', None, 2);
	ChildVehicle = None;
	bHasLockedVehicle = false;
	ExtraCost = 0;
}

simulated native function byte GetTeamNum();

event SpawnVehicle();

function TriggerSpawnedEvent()
{
	TriggerEventClass(class'UTSeqEvent_VehicleFactory', None, 0);
}

function OnToggle(SeqAct_Toggle Action)
{
	local UTGameObjective Objective;

	if (Action.InputLinks[0].bHasImpulse)
	{
		bDisabled = false;
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		bDisabled = true;
	}
	else
	{
		bDisabled = !bDisabled;
	}

	if (bDisabled)
	{
		Deactivate();
	}
	else
	{
		// find the objective that owns us and use it to activate us
		foreach WorldInfo.AllNavigationPoints(class'UTGameObjective', Objective)
		{
			if (Objective.VehicleFactories.Find(self) != INDEX_NONE)
			{
				Activate(Objective.GetTeamNum());
				RespawnProgress = 0.0;
				SpawnVehicle();
				break;
			}
		}
	}
}

function rotator GetSpawnRotation()
{
	local rotator SpawnRot;

	SpawnRot = Rotation;
	if ( bMayReverseSpawnDirection && (ReverseObjective != None) && (ReverseObjective.DefenderTeamIndex == TeamNum) )
	{
		SpawnRot.Yaw += 32768;
	}
	return SpawnRot;
}

state Active
{
	function TarydiumBoost(float Quantity)
	{
		RespawnProgress -= Quantity;
	}

	function VehicleDestroyed( UTVehicle V )
	{
		Global.VehicleDestroyed(V);
		RespawnProgress = VehicleClass.Default.RespawnTime - VehicleClass.Default.SpawnInTime;
	}

	event SpawnVehicle()
	{
		local Pawn P;
		local bool bIsBlocked;
		local Rotator SpawnRot, TurretRot;
		local vector SpawnLoc;
		local int i;
		local UTGame G;

		if ( (ChildVehicle != None) && !ChildVehicle.bDeleteMe )
		{
			return;
		}

		// tell AI to avoid navigating through factories with a vehicle on top of them
		ExtraCost = FMax(ExtraCost,5000);

		foreach CollidingActors(class'Pawn', P, VehicleClass.default.SpawnRadius)
		{
			bIsBlocked = true;
			if (PlayerController(P.Controller) != None)
				PlayerController(P.Controller).ReceiveLocalizedMessage(class'UTVehicleMessage', 2);
		}

		if (bIsBlocked)
		{
			SetTimer(1.0, false, 'SpawnVehicle'); //try again later
		}
		else
		{
			SpawnRot = GetSpawnRotation();
			SpawnLoc = Location + (vect(0,0,1)*SpawnZOffset);
			ChildVehicle = spawn(VehicleClass,,,SpawnLoc, SpawnRot);
			if (ChildVehicle != None )
			{
				ChildVehicle.SetTeamNum(TeamNum);
				ChildVehicle.ParentFactory = Self;
				if ( bStartNeutral )
					ChildVehicle.bTeamLocked = false;
				else if ( ChildVehicle.bTeamLocked )
					bHasLockedVehicle = true;
				if ( bKeyVehicle )
				{
					ChildVehicle.SetKeyVehicle();
					// don't let defenders use key vehicles as they may be requied to complete objectives
					ChildVehicle.AIPurpose = AIP_Offensive;
				}
				if (bForceAvoidReversing)
				{
					ChildVehicle.bAvoidReversing = true;
				}
				ChildVehicle.Mesh.WakeRigidBody();

				for (i=0; i<ChildVehicle.Seats.Length;i++)
				{
					if (i < InitialGunRotations.Length)
					{
						TurretRot = InitialGunRotations[i];
						if ( bMayReverseSpawnDirection && (ReverseObjective != None) && (ReverseObjective.DefenderTeamIndex == TeamNum) )
						{
							TurretRot.Yaw += 32768;
						}
					}
					else
					{
						TurretRot = SpawnRot;
					}

					ChildVehicle.ForceWeaponRotation(i,TurretRot);
				}
				G = UTGame(WorldInfo.Game);
				if ( G.MatchIsInProgress() )
				{
					ChildVehicle.PlaySpawnEffect();
				}
				// if gameplay hasn't started yet, we need to wait a bit for everything to be initialized
				if (WorldInfo.bStartup)
				{
					SetTimer(0.1, false, 'TriggerSpawnedEvent');
				}
				else
				{
					TriggerSpawnedEvent();
				}
				if ( G.bHeavyArmor && (TeamNum == 0) )
				{
					ChildVehicle.Health *= 1.4;
				}
				if ( G.bNecrisLocked && ChildVehicle.bIsNecrisVehicle && (TeamNum == 1) )
				{
					ChildVehicle.bEnteringUnlocks = false;
				}
			}
		}
	}

	function Activate(byte T)
	{
		if (!CanActivateForTeam(T))
		{
			Deactivate();
		}
		else
		{
			TeamNum = T;
			if (ChildVehicle != None)
			{
				// if we have an unused vehicle available, just change its team
				if (ChildVehicle.bTeamLocked)
				{
					ChildVehicle.SetTeamNum(T);
				}
			}
			else
			{
				// force a new vehicle to be spawned
				RespawnProgress = 0.0;
				ClearTimer('SpawnVehicle');
				SpawnVehicle();
			}
		}
	}

	function BeginState(name PreviousStateName)
	{
		if ( UTGame(WorldInfo.Game).MatchIsInProgress() )
		{
			RespawnProgress = VehicleClass.Default.InitialSpawnDelay - VehicleClass.Default.SpawnInTime;
			if (RespawnProgress <= 0.0)
			{
				SpawnVehicle();
			}
		}
		else
		{
			RespawnProgress = 0.0;
			SpawnVehicle();
		}
	}

	function EndState(name NextStateName)
	{
		RespawnProgress = 0.0;
		ClearTimer('SpawnVehicle');
	}
}

defaultproperties
{
	Begin Object Class=SkeletalMeshComponent Name=SVehicleMesh
		CollideActors=false
		HiddenGame=true
		AlwaysLoadOnClient=false
		AlwaysLoadOnServer=false
		bUpdateSkelWhenNotRendered=false
	End Object
	Components.Add(SVehicleMesh)

	Components.Remove(Sprite2)
	GoodSprite=None
	BadSprite=None

	bHidden=true
	bBlockable=true
	bAlwaysRelevant=true
	bSkipActorPropertyReplication=true
	RemoteRole=ROLE_SimulatedProxy
	bStatic=False
	bNoDelete=True
	TeamNum=255
	RespawnRateModifier=1.0
	NetUpdateFrequency=1.0

	SupportedEvents.Add(class'UTSeqEvent_VehicleFactory')
}
