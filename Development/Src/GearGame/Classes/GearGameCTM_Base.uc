/**
 * GearGameAnnex_Base
 * Gear Game Info for Annex Gametype
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameCTM_Base extends GearGameMP_Base
	abstract
	native
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/// TIMER VARIABLES ///
/** Timer used to make the destination go away if the victim is released for too long */
var config float TimeForRecapture;
var float TimeForUpdateDestinationUI;

var config float TimeToCapture;

/// DESTINATION VARIABLES ///
/** List of navigation points that the victim can spawn at */
var transient array<MeatflagSpawnPoint> VictimSpawnList;
/** List of navigation points that the victim can run to */
var transient array<NavigationPoint> VictimNavList;
/** Current Command Point */
var transient NavigationPoint DestinationPoint;
/** Particle effect at the destination location */
var transient SpawnedGearEmitter DestinationEffect;
/** Class of the DestinationEffect */
var class<SpawnedGearEmitter> DestinationEffectClass;
/** Currently controlling team */
var transient GearTeamInfo ControllingTeam;
/** Max distance player can be from the destination to score */
var float MaxPointDist, MaxPointDistZ;
/** optimal max distance for the destination to be from the point where the victim is picked up */
var config float OptimalMaxDestinationDist;

/// VICTIM VARIABLES ///
/** AI of the victim */
var transient GearAI VictimAI;
/** Pawn of the kidnapper */
var transient GearPawn KidnapperPawn;
/** All of the meatflag classes we can choose from */
var array<class<GearPawn> > MeatflagClasses;
/** The class of the current meatflag */
var class<GearPawn> CurrMeatflagClass;
/** The team of the victim */
var GearTeamInfo VictimTeam;
/** Weapon the victim will default to have */
var class<GearWeapon> VictimDefaultWeaponClass;
/** Most recent known kidnapper. */
var transient GearPawn LastKidnapperPawn;


/// SCORING/OTHER VARIABLES ///
/** Total captures needed to win round */
var config int RoundGoal;
/** Whether we are in sudden death of not */
var bool bIsSuddenDeath;

/// SOUND VARIABLES ///
/** Played when a new team grabs the hostage */
var SoundCue		Sound_ControllingTeamChanged[2];
/** Played the victim is dropped */
var SoundCue		Sound_VictimDropped[2];
/** Played when the destination location expires */
var SoundCue		Sound_DestinationExpired[2];

/** List of players to join the game while in progress */
var array<PlayerController>	JoinInProgressList;
/** Team that currently has the lead */
var transient GearTeamInfo					LeadingTeam;

/** Points awarded to the player capturing the meatflag. */
var config int PointsPerCap;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
* Set the game rules via the online game settings
* @return - whether the game was able to set the rules via the gamesettings object
*/
function bool SetGameRules()
{
	local int IntValue;
	local GearVersusGameSettings MyGameSettings;

	MyGameSettings = GetGameSettingsObject();

	if ( MyGameSettings != None )
	{
		// Meatflag AI Difficulty
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_MEATFLAGDIFFICULTY, IntValue) )
		{
			// TODO: Set the Difficulty of the Meatflag AI
		}

		return Super.SetGameRules();
	}

	return FALSE;
}

/** Team always has respawns */
function bool TeamHasRespawn( int TeamIdx )
{
	return TRUE;
}

/** Send a sound to all player controllers */
function BroadcastSound( SoundCue ASound )
{
	local PlayerController PC;

	foreach WorldInfo.AllControllers( class'PlayerController', PC )
	{
		PC.ClientPlaySound( ASound );
	}
}

/**
* called for MP games.
*
* @see GearPawn:  state Reviving
**/
function bool AutoReviveOnBleedOut( Pawn TestPawn )
{
	if ( bAutoReviveOnBleedOut || (TestPawn == VictimAI.Pawn) )
	{
		return true;
	}

	return false;
}

/** Find all of the places for the victim to spawn and run to */
final function InitializeVictimNavigation()
{
	local GearWeaponPickupFactory Factory;
	local MeatflagSpawnPoint SpawnPoint;

	VictimNavList.length = 0;

	// Loop through each factory
	foreach WorldInfo.AllNavigationPoints( class'GearWeaponPickupFactory', Factory )
	{
		if ( !Factory.bMP_ExcludeMeatflag )
		{
			VictimNavList.AddItem( Factory );
		}
	}

	// Loop through each meatflag spawn point
	foreach WorldInfo.AllNavigationPoints( class'MeatflagSpawnPoint', SpawnPoint )
	{
		VictimSpawnList.AddItem( SpawnPoint );
	}
}

/** Set ring's color */
function SetCTMRingColor( Color RingColor );

/** Returns the color of the team in control of the victim */
function Color GetControllingTeamColor()
{
	// Neutral
	if( ControllingTeam == None )
	{
		return MakeColor(80,80,70,255);
	}
	// COG
	else if( ControllingTeam.TeamIndex == 0 )
	{
		return MakeColor(0,90,120,255);
	}
	// Locust
	else
	{
		return MakeColor(140,0,0,255);
	}
}

/** Check to see if the victim was captured, calls appropriate functions for a capture, and returns whether a capture happened or not */
final function CheckForCapture()
{
	if ( (VictimAI != None) && (KidnapperPawn != None) && (VictimAI.Pawn != None ) && IsNearDestination(KidnapperPawn) )
	{
		// set the timer for the final capture
		if (!IsTimerActive(nameof(CheckForFinalCapture)))
		{
			SetTimer(TimeToCapture,FALSE,'CheckForFinalCapture');
			DoTrainingGroundTutorial(GEARTUT_TRAIN_MeatScore, 0, ControllingTeam.TeamIndex);
		}
	}
	else
	{
		// clear the timer since they're no longer in the ring
		ClearTimer(nameof(CheckForFinalCapture));
	}
}

final function CheckForFinalCapture()
{
	local EGUDEventID CapturedGUDSEvent;
	local int KidnapperTeamIdx;
	if ( (VictimAI != None) && (KidnapperPawn != None) && (VictimAI.Pawn != None ) && IsNearDestination(KidnapperPawn) )
	{
		if ( (KidnapperPawn.Controller != None) && (KidnapperPawn.Controller.PlayerReplicationInfo != None) )
		{
			KidnapperTeamIdx = KidnapperPawn.Controller.PlayerReplicationInfo.Team.TeamIndex;
			GearGRI.GameScore[KidnapperTeamIdx] += 1;
			GearGRI.MeatflagKidnapper = GearPRI(KidnapperPawn.Controller.PlayerReplicationInfo);
			GearGRI.MeatflagKidnapper.ScoreGameSpecific1('MeatflagCaptured',"",PointsPerCap);

			// tell guds
			CapturedGUDSEvent = (KidnapperTeamIdx == 0) ? GUDEvent_MeatFlagCapturedByCOG : GUDEvent_MeatFlagCapturedByLocust;
			TriggerGUDEvent(CapturedGUDSEvent, KidnapperPawn, VictimAI.Pawn);
			ClearTimer('TriggerDraggingGUDS');
		}
   }
}

/** Have players pulse their command point indicators */
function PulseHUDPlayerCPIndicators( int NumPulses, float LengthOfPulse = 1.f )
{
	local GearPC PC;
	foreach WorldInfo.AllControllers( class'GearPC', PC )
	{
		PC.ClientSetCommandPointIndicatorGlow( NumPulses, LengthOfPulse );
	}
}

/** Determines if a pawn is valid and close enough to the destination */
function bool IsNearDestination( GearPawn P, optional float Scale = 1.f, optional out float Dist )
{
	local float DistZ;

	if( DestinationPoint != None &&
		P != None &&
		(P.Health > 0 || P.IsDBNO()) )
	{
		Dist = VSize2D(P.Location - DestinationPoint.Location);
		DistZ = Abs(P.Location.Z-DestinationPoint.Location.Z);

		if( Dist <= MaxPointDist*Scale &&
			DistZ <= MaxPointDistZ )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Log a player in.
 * Fails login if you set the Error string.
 * PreLogin is called before Login, but significant game time may pass before
 * Login is called, especially if content is downloaded.
 */
event PlayerController Login( string Portal, string Options, out string ErrorMessage )
{
	local PlayerController NewPlayer;
	local GearPC WPC;

	// call the super class
	NewPlayer = Super.Login( Portal, Options, ErrorMessage );

	if ( NewPlayer != None )
	{
		// add the player to the join in progress list if a valid player controller exists after the game has begun
		if ( GetGRI().GameStatus == GS_RoundInProgress )
		{
			JoinInProgressList[ JoinInProgressList.length ] = NewPlayer;
		}
		else
		{
			// Set the newly created player to the neutral team position.
			WPC = GearPC(NewPlayer);
			if ( WPC != none && WPC.bDedicatedServerSpectator )
			{
				ChangeTeam( WPC,255,false );
			}
		}
	}

	return NewPlayer;
}

/** Triggers the intermittent "still dragging the meatflag" GUDS event. */
function TriggerDraggingGUDS()
{
	local EGUDEventID DraggingGUDEvent;

	if (KidnapperPawn != None)
	{
		DraggingGUDEvent = (KidnapperPawn.Controller.PlayerReplicationInfo.Team.TeamIndex == 0) ? GUDEvent_MeatFlagStillHeldByCOG : GUDEvent_MeatFlagStillHeldByLocust;
		TriggerGUDEvent(DraggingGUDEvent, KidnapperPawn, VictimAI.Pawn);
	}
	else
	{
		ClearTimer('TriggerDraggingGUDS');
	}
}

/** Function called when someone becomes kidnapped */
function KidnappingStarted( GearPawn Hostage, GearPawn Kidnapper )
{
	local GearTeamInfo KidnappingTeam;
	local EGUDEventID KidnappedGUDEvent;

	// See if the hostage is the victim
	if ( (VictimAI != None) && (VictimAI.Pawn != None) && (VictimAI.Pawn == Hostage) )
	{
		// give out the points to ppl contributing to the meatflag
		GearPRI(Hostage.PlayerReplicationInfo).ScoreDeath(GearPRI(Kidnapper.PlayerReplicationInfo),class'GDT_Hostage',GDT_NORMAL);
		// Set the kidnapper and team references and see if we need a new destination
		KidnapperPawn = Kidnapper;
		KidnappingTeam = GearTeamInfo(Kidnapper.GetTeam());
		if ( (ControllingTeam == None) || (ControllingTeam != KidnappingTeam) || !IsTimerActive('RecaptureTimeExpired') )
		{
			SelectDestination();
		}
		ControllingTeam = KidnappingTeam;
		SetCTMRingColor(GetControllingTeamColor());

		// Set some UI variables
		GearGRI.CommandPoint = GearWeaponPickupFactory(DestinationPoint);
		GearGRI.CPControlTeam = ControllingTeam.TeamIndex;
		GearGRI.CPControlPct = 100;

		// Play the kidnap sound
		BroadcastSound( Sound_ControllingTeamChanged[ControllingTeam.TeamIndex] );

		// Check for TG tutorial
		DoTrainingGroundTutorial(GEARTUT_TRAIN_MeatRng, 0, ControllingTeam.TeamIndex);
		DoTrainingGroundTutorial(GEARTUT_TRAIN_MeatNmy, 1, ControllingTeam.TeamIndex);

		// Clear the recapture timer
		ClearTimer( 'RecaptureTimeExpired' );
		ClearTimer( 'UpdateDestinationUI' );

		// Pulse the indicator
		PulseHUDPlayerCPIndicators( 2, 1.0f );

		// tell guds
		KidnappedGUDEvent = (KidnappingTeam.TeamIndex == 0) ? GUDEvent_MeatFlagGrabbedByCOG : GUDEvent_MeatFlagGrabbedByLocust;
		TriggerGUDEvent(KidnappedGUDEvent, Kidnapper, Hostage, 0.5f);
		SetTimer( 7.f, TRUE, nameof(TriggerDraggingGUDS) );
	}
}

/** Function called when someone is released from kidnapping */
function KidnappingStopped( GearPawn Hostage, GearPawn Kidnapper )
{
	local EGUDEventID ReleasedGUDEvent;

	// See if the hostage is the victim
	if ( (VictimAI != None) && (VictimAI.Pawn != None) && (VictimAI.Pawn == Hostage) )
	{
		// Start the timer that will make the destination change if expired
		SetTimer( TimeForRecapture, false, nameof(RecaptureTimeExpired) );
		SetTimer( TimeForUpdateDestinationUI, true, nameof(UpdateDestinationUI) );

		// Start the timer that will make him run to the farthest place on the map
		SetTimer( 0.5f, false, nameof(OnRunToFarthestLocation) );

		// Give him back his weapon
		VictimAI.Pawn.CreateInventory( VictimDefaultWeaponClass );

		// Play the lost victim sound
		BroadcastSound( Sound_VictimDropped[ControllingTeam.TeamIndex] );

		if ( (GearGRI.GameStatus != GS_RoundOver) && (GearGRI.GameStatus != GS_EndMatch) )
		{
			// tell guds
			ReleasedGUDEvent = (KidnapperPawn.Controller.PlayerReplicationInfo.Team.TeamIndex == 0) ? GUDEvent_MeatFlagReleasedByCOG : GUDEvent_MeatFlagReleasedByLocust;
		}

		TriggerGUDEvent(ReleasedGUDEvent, Kidnapper, Hostage, 0.5f);
		ClearTimer('TriggerDraggingGUDS');

		LastKidnapperPawn = KidnapperPawn;
		KidnapperPawn = None;
	}
}

/** Called on a timer to make the victim run like hell (using timer because of AI DBNO delay) */
function OnRunToFarthestLocation()
{
	local NavigationPoint RunToPoint;

	if( VictimAI.MyGearPawn.IsTimerActive(nameof(VictimAI.MyGearPawn.GetBackUpFromKnockDown)) )
	{
		// wait for the victim to get up
		SetTimer( 0.5f, FALSE, nameof(OnRunToFarthestLocation) );
	}
	else if( !VictimAI.IgnoreNotifies() )
	{
		// Make the victim run like hell
		RunToPoint = FindFarthestNavigationPointFromDestination();
		if ( RunToPoint != None )
		{
			VictimAI.SpawnerSetTether( RunToPoint );
			VictimAI.bTeleportOnTetherFailure = FALSE;
		}
	}
}

/**
 * Timer callback function that is started when the victim is released from being meat-shielded
 * If this timer is hit the next time the team grabs him the destination will change
 */
function RecaptureTimeExpired()
{
	local NavigationPoint FarthestPoint;

	// Play the expired sound
	BroadcastSound( Sound_DestinationExpired[ControllingTeam.TeamIndex] );

	// Set the destination back to null
	ClearDestination();

	// Pulse the indicator
	PulseHUDPlayerCPIndicators( 5, 0.2f );

	// Make victim run away from all players
	if( VictimAI != None && !VictimAI.IgnoreNotifies() )
	{
		FarthestPoint = FindFarthestNavigationPointFromAllPawns();
		if ( FarthestPoint != None )
		{
			VictimAI.SpawnerSetTether( FarthestPoint );
			VictimAI.bTeleportOnTetherFailure = FALSE;
		}
	}
}

/** Function called to update the destination UI */
function UpdateDestinationUI()
{
	local float CurrTime;

	CurrTime = GetTimerCount( 'RecaptureTimeExpired' );

	GearGRI.CPControlPct = int(100.f * (1.0f - (CurrTime / TimeForRecapture)));
}

/** Spawn the victim and tether him to a central location */
final function SpawnVictim()
{
	local GearPawn VictimPawn;
	local NavigationPoint SpawnPoint;

	// Find a spawn location
	SpawnPoint = GetVictimSpawnLocation();

	// Randomly pick a meatflag class to use for this match
	if ( CurrMeatflagClass == None )
	{
		CurrMeatflagClass = MeatflagClasses[ Rand(MeatflagClasses.length) ];
	}

	// Spawn the meatflag pawn
	VictimPawn = Spawn( CurrMeatflagClass, self, , SpawnPoint.Location, SpawnPoint.Rotation );

	if ( VictimPawn != None )
	{
		// Spawn an AI if we haven't done so already
		if ( VictimAI == None )
		{
			VictimAI = Spawn( class'GearAI_CTMVictim', self, , SpawnPoint.Location, SpawnPoint.Rotation );
			GearPRI(VictimAI.PlayerReplicationInfo).Difficulty = class'DifficultySettings_Insane';
			GearPRI(VictimAI.PlayerReplicationInfo).bIsMeatflag = true;
		}

		// Initialize the meatflag
		if ( VictimAI != None )
		{
			GetGRI().MeatflagPawn = VictimPawn;
			VictimPawn.CreateInventory( VictimDefaultWeaponClass );
			VictimPawn.bAllowInventoryDrops = false;
			VictimPawn.MaxDownCount = 99999;
			VictimAI.Possess( VictimPawn, false );
			VictimAI.SpawnerSetTether( SpawnPoint, TRUE );
			VictimAI.bTeleportOnTetherFailure = FALSE;

			// Put the victim on a squad
			if ( VictimTeam == None )
			{
				VictimTeam = Spawn( class'GearTeamInfo', self );
			}
			VictimTeam.JoinSquad( 'Victim', VictimAI, true );
		}
		else
		{
			`log("ERROR: GearGameCTM_Base::SpawnVictim: Could not spawn AI controller GearAI_CTMVictim at spawn point"@SpawnPoint);
		}
	}
	else
	{
		`log("ERROR: GearGameCTM_Base::SpawnVictim: Could not spawn pawn"@CurrMeatflagClass@" at spawn point"@SpawnPoint);
	}
}

/** Pick a destination to drag the victim to */
function SelectDestination()
{
	DestinationPoint = FindDestinationNavigationPoint();

	if ( DestinationPoint != None )
	{
		if ( DestinationEffect == None )
		{
			DestinationEffect = Spawn( DestinationEffectClass,,, DestinationPoint.Location, DestinationPoint.Rotation );
			DestinationEffect.ParticleSystemComponent.ActivateSystem();
			DestinationEffect.SetDrawScale( 0.5f );
		}
		else
		{
			DestinationEffect.SetLocation( DestinationPoint.Location );
		}
	}
}

/** Clear the destination to drag the victim to */
function ClearDestination()
{
	DestinationPoint = None;

	if( DestinationEffect != None )
	{
		DestinationEffect.Destroy();
		DestinationEffect = None;
	}

	// Set some UI variables
	GearGRI.CommandPoint = None;
	GearGRI.CPControlTeam = INDEX_NONE;

	// Clear any timers from the game
	ClearTimer( 'RecaptureTimeExpired' );
	ClearTimer( 'UpdateDestinationUI' );
}

/** Find the navigation point to spawn the meatflag */
final function NavigationPoint GetVictimSpawnLocation()
{
	if ( VictimSpawnList.length > 0 )
	{
		return VictimSpawnList[ Rand(VictimSpawnList.length) ];
	}
	else
	{
		return FindFarthestNavigationPointFromAllPawns();
	}
}

/**
 * Find the destination to carry the meatflag to using the real path finding distance
 * @param MaxDistance - The max distance to look for a navigation point (will return a navigation point greater if need be)
 */
final function NavigationPoint FindDestinationNavigationPoint()
{
	if (VictimAI != None && VictimAI.Pawn != None && VictimNavList.length > 0)
	{
		VictimAI.Pawn.ClearConstraints();
		class'Goal_FarthestNavInRange'.static.FarthestNavInRange( VictimAI.Pawn,
									OptimalMaxDestinationDist,
									VictimNavList );
		if (VictimAI.FindPathToward(VictimNavList[0]) != None)
		{
			return NavigationPoint(VictimAI.RouteGoal);
		}
		else
		{
			// give up and try linear distance
			return FindFarthestNavigationPointFromLocation(VictimAI.Pawn.Location);
		}
	}
	else
	{
		return None;
	}
}

/** Find a location in the map that is the farthest point from where the destination is */
final function NavigationPoint FindFarthestNavigationPointFromDestination()
{
	if ( DestinationPoint != None )
	{
		return FindFarthestNavigationPointFromLocation( DestinationPoint.Location );
	}

	return None;
}

/** Helper function to find the farthest point from a location */
final function NavigationPoint FindFarthestNavigationPointFromLocation( vector StartPoint )
{
	local int NavPointIdx;
	local float FarthestDistance, CurrDistance;
	local NavigationPoint FarthestPoint;

	for ( NavPointIdx = 0; NavPointIdx < VictimNavList.length; NavPointIdx++ )
	{
		CurrDistance = VSizeSq(VictimNavList[NavPointIdx].Location - StartPoint);
		if ( CurrDistance > FarthestDistance )
		{
			FarthestDistance = CurrDistance;
			FarthestPoint = VictimNavList[NavPointIdx];
		}
	}

	return FarthestPoint;
}

/** Find a location in the map that is the farthest point from where all players are */
final function NavigationPoint FindFarthestNavigationPointFromAllPawns()
{
	local GearPawn TestPawn;
	local array<float> NavPointDistancesTeam0, NavPointDistancesTeam1;
	local int NavPointIdx, TeamId;
	local float CurrNavDistance, FarthestNavDistanceDiff, CurrNavDistanceDiff;
	local NavigationPoint FarthestNavPoint;

	// Calculate the distances the nav points are from all of the players
	NavPointDistancesTeam0.length = VictimNavList.length;
	NavPointDistancesTeam1.length = VictimNavList.length;
	foreach WorldInfo.AllPawns(class'GearPawn', TestPawn)
	{
		// Make sure this is a valid pawn in the game
		if ( (TestPawn != None) && (VictimAI == None || TestPawn != VictimAI.Pawn) && (TestPawn.Controller != None) )
		{
			TeamId = TestPawn.Controller.GetTeamNum();
			if ( (TeamId == 0) || (TeamId == 1) )
			{
				for ( NavPointIdx = 0; NavPointIdx < VictimNavList.length; NavPointIdx++ )
				{
					CurrNavDistance = VSizeSq(TestPawn.Location - VictimNavList[NavPointIdx].Location);
					if ( TeamId == 0 )
					{
						NavPointDistancesTeam0[NavPointIdx] += CurrNavDistance;
					}
					else
					{
						NavPointDistancesTeam1[NavPointIdx] += CurrNavDistance;
					}
				}
			}
		}
	}

	// Find the farthest one
	FarthestNavDistanceDiff = -1;
	for ( NavPointIdx = 0; NavPointIdx < VictimNavList.length; NavPointIdx++ )
	{
		CurrNavDistanceDiff = Abs( NavPointDistancesTeam0[NavPointIdx] - NavPointDistancesTeam1[NavPointIdx] );
		if ( (FarthestNavDistanceDiff > CurrNavDistanceDiff) || (FarthestNavDistanceDiff == -1) )
		{
			FarthestNavDistanceDiff = CurrNavDistanceDiff;
			FarthestNavPoint = VictimNavList[NavPointIdx];
		}
	}

	return FarthestNavPoint;
}

/** @return the closest usable PlayerStart to the meatflag destination using the path network */
native final function PlayerStart GetClosestStartToDestination();

function bool CanBeShotFromAfarAndKilledWhileDownButNotOut( Pawn TestPawn, Pawn InstPawn, class<GearDamageType> TheDamageType )
{
	// we explicitly override the damage anyway, but the AI uses this to know it has to execute
	return (TestPawn != VictimAI.Pawn && Super.CanBeShotFromAfarAndKilledWhileDownButNotOut(TestPawn, InstPawn, TheDamageType));
}

/**
* Called on the first tick of the PendingMatch state so that we can initialized data only once,
* but after the Sequence is initialized. This is handy for initialization code that requires
* the kismet system to be ready (such as finding all events of type x)
*/
function InitializeGameOnFirstTick()
{
	Super.InitializeGameOnFirstTick();

	InitializeVictimNavigation();
}

/** Overridden to maybe trigger GUDS. */
function Killed( Controller Killer, Controller KilledPlayer, Pawn KilledPawn, class<DamageType> damageType )
{
	local EGUDEventID KilledGUDEvent;
	super.Killed(Killer, KilledPlayer, KilledPawn, damageType);

	if ( (KilledPawn == LastKidnapperPawn) && (Killer == VictimAI) )
	{
		KilledGUDEvent = (KilledPawn.Controller.PlayerReplicationInfo.Team.TeamIndex == 0) ? GUDEvent_MeatFlagKilledFormerCOGCaptor : GUDEvent_MeatFlagKilledFormerLocustCaptor;
		TriggerGUDEvent(KilledGUDEvent, Killer.Pawn, KilledPawn, 1.f);
	}
}

function NotifyDBNO(Controller InstigatedBy, Controller Victim, Pawn DownedPawn)
{
	super.NotifyDBNO(InstigatedBy, Victim, DownedPawn);

	if (Victim == VictimAI)
	{
		// tell guds meatflag went down
		TriggerGUDEvent(GUDEvent_MeatFlagWentDBNO, InstigatedBy.Pawn, DownedPawn, 0.5f);
	}
}

function Actor GetObjectivePointForAI(GearAI AI)
{
	// if the meatflag is free, or a friendly has captured it, or an enemy has but we've seen that enemy recently
	if ( KidnapperPawn == None ||
		( KidnapperPawn != AI.Pawn &&
			(WorldInfo.GRI.OnSameTeam(KidnapperPawn, AI) || WorldInfo.TimeSeconds - AI.Squad.GetEnemyLastSeenTime(KidnapperPawn) < 5.0) ) )
	{
		// capture the meatflag
		return VictimAI.MyGearPawn;
	}
	else
	{
		// head to the capture destination
		return DestinationPoint;
	}
}

function bool MustStandOnObjective(GearAI AI)
{
	// yes if we have the meatflag
	// or the meatflag is DBNO
	// or the enemy has the meatflag and they are closer to the destination point than we are (so the AI will rush the kidnapper)
	return ( KidnapperPawn == AI.Pawn || VictimAI.MyGearPawn.IsDBNO() ||
		( KidnapperPawn != None && DestinationPoint != None && !WorldInfo.GRI.OnSameTeam(KidnapperPawn, AI) &&
			VSize(KidnapperPawn.Location - DestinationPoint.Location) <= VSize(AI.Pawn.Location - DestinationPoint.Location) ) );
}

function SetAICombatMoodFor(GearAI AI)
{
	local ECombatMood NewCombatMood;

	// if we don't have the meatflag, attack
	if (KidnapperPawn == None || !WorldInfo.GRI.OnSameTeam(KidnapperPawn, AI))
	{
		NewCombatMood = AICM_Aggressive;
	}
	else
	{
		NewCombatMood = AICM_Normal;
	}

	if (AI.CombatMood != NewCombatMood)
	{
		AI.SetCombatMood(NewCombatMood);
	}
}

function AdjustEnemyRating(GearAI AI, out float out_Rating, Pawn EnemyPawn)
{
	if (EnemyPawn == VictimAI.Pawn)
	{
		out_Rating += 0.5;
	}
	else if (EnemyPawn == KidnapperPawn)
	{
		out_Rating += 1.5;
	}
}

event GetSeamlessTravelActorList(bool bToEntry, out array<Actor> ActorList)
{
	Super.GetSeamlessTravelActorList(bToEntry, ActorList);

	// include the meatflag when going to entry so his scores stay available
	if (bToEntry)
	{
		ActorList[ActorList.length] = VictimAI;
	}
}

/************************************************************************
*						PreRound State Begin
* This is the State that the game will enter when between rounds.  It does nothing
* but watch the RoundTime.  When it hits 0, the round begins via StartMatch()
************************************************************************/
state PreRound
{
	function BeginState(Name PreviousStateName)
	{
		local int Idx;

		Super.BeginState(PreviousStateName);

		for ( Idx = 0; Idx < NumTeams; Idx++ )
		{
			NumTeamRespawns[Idx] = TotalNumTeamRespawns;
			GetGRI().NumTeamRespawns[Idx] = NumTeamRespawns[Idx];
		}
		GetGRI().TotalNumTeamRespawns = TotalNumTeamRespawns;
		GetGRI().RespawnTimeInterval = RespawnTimeInterval;
	}
}
/************************************************************************
*						PreRound State End
************************************************************************/


/************************************************************************
*						MatchInProgress State Begin
*	This is the state the game will be in while the match is in
*  progress. The game will remain in this state until the end of match
*  condition is met.
************************************************************************/
state MatchInProgress
{
	/** Allow spawning */
	function RestartPlayer(Controller NewPlayer)
	{
		`log(GetFuncName()@NewPlayer);
		Super(GearGame).RestartPlayer(NewPlayer);
	}

	simulated function BeginState( Name PreviousStateName )
	{
		local int Idx;

		Super.BeginState( PreviousStateName );

		// Clear the teams' points
		for ( Idx = 0; Idx < NumTeams; Idx++ )
		{
			GearGRI.GameScore[Idx] = 0;
		}

		// Clear the sudden death flag
		bIsSuddenDeath = false;

		ClearDestination();

		// Spawn the victim
		SpawnVictim();

		SetTimer(10.f, FALSE, nameof(MeatFlagPlayGUDSTaunt));

		SetTimer(1.f,TRUE,nameof(BroadcastEndGameWarning));
	}

	simulated function EndState( Name NextStateName )
	{
		ClearDestination();

		// clear this list just in case
		JoinInProgressList.length = 0;

		Super.EndState(NextStateName);

		ClearTimer(nameof(BroadcastEndGameWarning));
	}

	// Called every tick
	event Tick( float DeltaTime )
	{
		local int Idx;

		// If there are any players waiting to join in progress, allow them to join
		for ( Idx = 0; Idx < JoinInProgressList.length; Idx++ )
		{
			if ( JoinInProgressList[Idx] != None && JoinInProgressList[Idx].IsInState('PlayerWaiting') )
			{
				GearPC(JoinInProgressList[Idx]).TransitionFromDeadState();
				JoinInProgressList.Remove( Idx--, 1 );
			}
		}
	}

	/**
	 * If round timer expires, take the team with the most captures as the winner, if there is a tie, go into sudden death
	 * @return - whether the round is over or not
	 */
	function bool DoRoundTimerExpired( int WinningTeamIndex )
	{
		// See if we need to go to sudden death or not
		if ( WinningTeamIndex == INDEX_NONE )
		{
			bIsSuddenDeath = true;
			return false;
		}

		return true;
	}

	/** Return the index of the winning team - if there is only one */
	function int GetWinningTeamIndex()
	{
		local int TeamIdx;
		local float HighScore;
		local array<int> HighestScoringTeamIndexes;

		// Make sure there are teams
		if ( Teams.length > 0 )
		{
			// Initialize by having the first team be the winning one
			HighestScoringTeamIndexes.AddItem( 0 );
			HighScore = GearGRI.GameScore[0];

			// See if any other teams have the same score or greater
			for ( TeamIdx = 1; TeamIdx < Teams.length; TeamIdx++ )
			{
				// This team is tied with the others in the list, so add it
				if ( GearGRI.GameScore[TeamIdx] == HighScore )
				{
					HighestScoringTeamIndexes.AddItem( TeamIdx );
				}
				// This team has a higher score so clear the list and make this be the winning team
				else if ( GearGRI.GameScore[TeamIdx] > HighScore )
				{
					HighestScoringTeamIndexes.length = 0;
					HighestScoringTeamIndexes.AddItem( TeamIdx );
				}
			}

			// See if there is a winner
			if ( HighestScoringTeamIndexes.length == 1 )
			{
				return HighestScoringTeamIndexes[0];
			}
		}

		return INDEX_NONE;
	}

	/** Called when we start sudden death, let's us send UI warnings and stuff */
	function StartSuddenDeath()
	{
		bIsSuddenDeath = true;
	}

	/** Callback timer function that calls for a check to see if the end of the match has occured. */
	function bool CheckEndMatch( optional Controller Killed )
	{
		local int WinningTeamIndex;

		// See if a capture has occured
		CheckForCapture();

		// If this is not the tourist mode, see if the match/round is now over
		if ( !bTourist )
		{
			WinningTeamIndex = GetWinningTeamIndex();
			if ( WinningTeamIndex != INDEX_NONE )
			{
				return HandleTeamWin( WinningTeamIndex );
			}
		}

		return Super.CheckEndMatch(Killed);
	}


	/** Make the playerstarts shuffle before broadcasting the respawn message */
	function BroadcastRespawnMessage()
	{
		local GearTeamPlayerStart Best;
		local int i, DefendingTeam;

		// when there is a kidnapper, make the "defending" team always spawn closer to the capture point
		if (KidnapperPawn != None)
		{
			DefendingTeam = Abs(1 - KidnapperPawn.GetTeamNum());
			// find the closest usable PlayerStart
			Best = GearTeamPlayerStart(GetClosestStartToDestination());
			if (Best != None && Best.TeamIndex != TeamPlayerStartIndex[DefendingTeam])
			{
				// take best start index and swap with any team that was previously using it
				for (i = 0; i < TeamPlayerStartIndex.length; i++)
				{
					if (TeamPlayerStartIndex[i] == Best.TeamIndex)
					{
						TeamPlayerStartIndex[i] = TeamPlayerStartIndex[DefendingTeam];
					}
				}
				TeamPlayerStartIndex[DefendingTeam] = Best.TeamIndex;
			}
		}
		else
		{
			ShuffleTeamPlayerStarts();
		}
		Super.BroadcastRespawnMessage();
	}

	function MeatFlagPlayGUDSTaunt()
	{
		// make the meatflag say something inspiring
		GearGame(WorldInfo.Game).UnscriptedDialogueManager.PlayGUDSAction(GUDAction_MeatFlagRoundBegun, VictimAI.Pawn);
	}
}

function MeatFlagPlayGUDSTaunt();


/************************************************************************
*						MatchInProgress State End
************************************************************************/

/**
 * Writes the stats and then tells the clients to add the meatflag to the recent history
 */
function WriteOnlineStats()
{
	local GearPC PC;

	Super.WriteOnlineStats();

	// Tell each client to add the meatflag to the list of recent players
	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		PC.ClientAddMeatflagToRecentPlayers(OnlineStatsWriteClass,GetMeatflagUniqueId());
	}
}


/** Send a warning to players about the end of the game coming */
function BroadcastEndGameWarning()
{
	local GearPC PC;
	//`log(GetFuncName()@`showvar(KidnapperPawn)@`showvar(DestinationPoint));
	if (KidnapperPawn != None && IsNearDestination(KidnapperPawn))
	{
		foreach WorldInfo.AllControllers( class'GearPC', PC )
		{
			PC.ClientEndOfGameWarning( 0, ControllingTeam.TeamIndex );
		}
	}
}

defaultproperties
{
	VictimDefaultWeaponClass=class'GearWeap_Shotgun'
	TimeForUpdateDestinationUI=0.1f

	// Scale scoring distance to match the draw scale of the emitter
	MaxPointDist=128.f
	MaxPointDistZ=128.f

	OnlineStatsWriteClass=class'GearLeaderboardWriteMeatflag'
}
