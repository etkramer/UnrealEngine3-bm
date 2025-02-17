/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPRI extends PlayerReplicationInfo
	config(Pawn)
	native
	nativereplication;

/** Logging pre-processor macros */
/** GoW global macros */
`include( GearGame\GearStats.uci )

/** Natively, the GearPRI monitors it's pawn and adjusts it's PlayerStatus as needed. */
enum EGearPlayerStatus
{
	WPS_Spectating,		// Controller is spectating
	WPS_Alive,			// Controller is alive and in the world
	WPS_Down,			// Controller is down but not dead
	WPS_Dead,			// controller is dead
	WPS_Respawning,		// controller is respawning
};

var databinding EGearPlayerStatus PlayerStatus;

/** This is pretty ugly.  At the end of the round, since all players are killed and the PC is returned to
    the spectating status we have to store off a second copy of their status before getting killed so the
    scoreboard can be correct.  This get's set in GearGameTDM.KillRemainingPlayers() */

var EGearPlayerStatus EndOfRoundPlayerStatus;

/** Is this player dead? */
var databinding bool bIsDead;

/** Is the player the team leader (for gametypes like kill the leader */
var bool bIsLeader;

/** definitive record of tags collected is stored in GearProfileSettings */
var databinding int NumberOfCOGTagsCollected;

var config float PointsFor_TakeDowns;
var config float PointsFor_Executions;
var config float PointsFor_InstantKills;
var config float PointsFor_Revives;
var config float PointsFor_CurbStomps;
var config float PointsFor_DamagePct;

/** List of players who have damaged this player */
struct native PlayerDamageInfo
{
	var GearPRI PRI;
	var float DamageTotal;
};
var array<PlayerDamageInfo> PlayerDamageList;

/** Name of the squad this player belongs to. */
var databinding Name SquadName;
var bool bSquadLeader;

/** Should this player always appear in TacCom for its team regardless of squad? */
var bool bForceShowInTaccom;

/** Pawn class used */
var repnotify class<GearPawn> PawnClass;
/** Weapon selection, 0 == Lancer, 1 == HB */
var	databinding		byte	InitialWeaponType;

var bool bHostIsReady;

/** Whether this PRI is the PRI of the meatflag */
var bool bIsMeatflag;

var float ChatFadeValue;
var float ChatFadeTime;
var float TaccomChatFadeStart;

///////// PROFILE SETTINGS ///////////
/** This will determine whether or not we show gore.  This will be set from the Player' ProfileSettings. **/
var bool bShowGore;
/** This will determine whether or not we show the HUD.  This will be set from the Player' ProfileSettings. **/
var bool bShowHUD;
/** This will determine if the HUD always draws the weapon indicator in the hud (except in cines and other crazy places **/
var bool bAlwaysDrawWeaponIndicatorInHUD;
/** Determines if trigger controls should be flipped */
var EGearTriggerConfigOpts TrigConfig;
/** Determines if there are modifications to default controls */
var EGearStickConfigOpts	  StickConfig;

/**
 * The ProfileId for the character data provider that this player has chosen; must match one of the values
 * from either the ECogMPCharacter or ELocustMPCharacter enums, depending on the player's team.
 */
var		databinding		byte	SelectedCharacterProfileId;

/** Index to the preferred COG character for versus */
var int PreferredCOGIndex;
/** Index to the preferred Locuat character for versus */
var int PreferredLocustIndex;
/** Determines if we show the tooltip pictograms */
var bool bShowPictograms;
/** The current sensitivity level of the controller */
var EProfileControllerSensitivityOptions ControllerSensitivityConfig;
/** The current sensitivity level for targetting */
var EProfileControllerSensitivityOptions TargetSensitivityConfig;
/** The current sensitivity level for zooming */
var EProfileControllerSensitivityOptions ZoomSensitivityConfig;
/** Whether the player receives the team bonus multiplier for scoring (for CombatTrials) */
var bool bReceivesTeamScoringBonus;

/** struct to store inventory that we would like to persist from one round to another in MP games */
struct native PersistentInventoryData
{
	var class<Inventory>	LeftShoulderWeaponClass;
	var class<Inventory>	RightShoulderWeaponClass;
	var class<Inventory>	HolsterWeaponClass;
	var class<Inventory>	BeltWeaponClass;
	var class<Inventory>	ShieldClass;
};
/** Instance of the above PersistentInventory struct so that we can have persistent inventory across MP rounds */
var PersistentInventoryData	PersistentInventory;

/** Difficulty setting for this player */
var class<DifficultySettings> Difficulty;

// Publically replicated properties for EndOfRound screen
/** PRI of the last player to DBNO me */
var PlayerReplicationInfo LastToDBNOMePRI;
/** PRI of the last player to kill me */
var PlayerReplicationInfo LastToKillMePRI;
/** PRI of the last player I killed */
var PlayerReplicationInfo LastIKilledPRI;
/** The damage type to kill me */
var class<GearDamageType> DamageTypeToKillMe;

// Publically replcated stats for players
/** Number of players I have killed. */
var databinding int Score_Kills;
/** Number of players I have knocked DBNO. */
var databinding int Score_Takedowns;
/** Number of times I have revived teammates. */
var databinding int Score_Revives;
/** Game specific scoring properties. */
var databinding int Score_GameSpecific1, Score_GameSpecific2, Score_GameSpecific3;

/** Generic point accumulator used by different game types */
var float PointAccumulator;
/** Number KOTH ring points the player had at the beginning of the round */
var int KOTHRingPointsToBeginRound;

/**
 * The bit-shifted variable which communicates which DLC packs the player possesses
 * NOTE: only the first 31 bits will be valid and -1 must be tested against before
 *       trying to determine what DLC the player has as -1 means that this variable
 *		 has not replicated yet
 */
var int DLCFlag;

/** List of currently planted grenades for this player to limit the number active */
var transient array<GearProj_Grenade> PlantedGrenades;


//@STATS
/****************** Stats Tracking ***********************/

/** Holds the amount of time the player was roadie running */
var float TotalRoadieRunTime;

/** Holds the time when the owner of this PRI started roadie running */
var float RoadieRunStartTime;

/** Holds the total amount of time a player is alive */
var float TotalAliveTime;

/** Holds the start time of when the player spawned */
var float AliveStartTime;

/** Holds the total amount of time this player was in cover */
var float TotalTimeInCover;

/** Holds the # of kills this player has made */
var int KillStreak;
var int MaxKillStreak;

/** Track # of assists for stats */

var int Score_Assists;
var int Was_TakenDown;
var int Was_Revived;
var int Score_Executions;
var int Was_Executed;
var int NoTeamKills;

var int NoGibs, NoWasGibbed;
var int NoHeadShots, NoWasHeadShot;
var int NoGrenadeTags, NoTimesGrenadeTagged;
var int NoGrenadeMines, NoGrenadeMinesTriggered, NoGrenadeMinesTriggeredByMe;
var int NoGrenadeMartyr, NoGrenadeMartyrDeaths, NoTimesIDiedByGrenadeMartyr;

var int NoChainsawKills, NoChainSawDeaths;
var int NoChainsawDuelsWon;
var int NoChainsawDuelsLost;
var int NoChainsawDuelsTied;
var int NoMeleeKills, NoMeleeDeaths;
var int NoMeleeKnockdowns, NoMeleeWasKnockedDown;
var int NoMeleeHits, NoTimesMeleeHit;

var float TotalFlameTime;
var float TotalFlameDmgTime;

struct native AggregatedWeaponStats
{
	var EWeaponClass WeaponID;
	var int KillsWith;
	var int KilledBy;
	var int Knockdowns;
	var int WasKnockedDown;
	var int ShotsFired;
	var int ShotsHit;
	var int ReloadSkips;
	var int ReloadFailures;
	var int ReloadSuccess;
	var int ReloadSuperSuccess;
};

var array<AggregatedWeaponStats> WeaponStats;

var bool bSkipWeaponAggregate;

/** Stats Mask Defines */

const STATS_LEVEL1	= 0x01;
const STATS_LEVEL2 	= 0x02;
const STATS_LEVEL4	= 0x08;
const STATS_LEVEL7	= 0x40;


cpptext
{
	// Networking
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	// sent to everyone
	if (bNetDirty)
		bIsLeader, bIsDead, PawnClass, InitialWeaponType, SelectedCharacterProfileId, PlayerStatus, EndOfRoundPlayerStatus,
		Difficulty, bReceivesTeamScoringBonus, LastToDBNOMePRI, LastToKillMePRI, LastIKilledPRI,
		DamageTypeToKillMe, Score_Kills, Score_Takedowns, Score_Revives, Score_GameSpecific1, Score_GameSpecific2, Score_GameSpecific3,
		KOTHRingPointsToBeginRound, DLCFlag, SquadName, bForceShowInTaccom, bIsMeatflag;
}

function PreBeginPlay()
{
	Difficulty = class'DifficultySettings'.static.GetLowestPlayerDifficultyLevel(WorldInfo);

	Super.PreBeginPlay();
}

event SetPlayerName(string S)
{
	local GearPC PC;

	if ( S == "" )
	{
		// if SetPlayerName is called with an empty string, it usually indicates that the player isn't signed into a profile.
		PC = GearPC(Owner);
		if ( PC != None )
		{
			if ( SplitscreenIndex == 0 )
			{
				S = "Marcus";
			}
			else
			{
				S = "Dom";
			}
		}

		if ( S == "" )
		{
			S = "Marcus";
		}
	}

	PlayerName = S;

	if ( !(PlayerName ~= OldName) )
	{
		// ReplicatedEvent() won't get called by net code if we are the server
		if (WorldInfo.NetMode == NM_Standalone || WorldInfo.NetMode == NM_ListenServer)
		{
			ReplicatedEvent('PlayerName');
			if (Owner.IsPlayerOwned())
			{
				ReplicatedDataBinding('PlayerName');
			}
		}

		bForceNetUpdate = true;
	}
	OldName = PlayerName;
}

simulated final function GearPRI FindLocalPRI()
{
	local GameReplicationInfo GRI;
	local PlayerReplicationInfo PRI;
	GRI = WorldInfo.GRI;
	if (GRI != None)
	{
		foreach GRI.PRIArray(PRI)
		{
			if (GearPRI(PRI) != None && PRI.Owner != None)
			{
				return GearPRI(PRI);
			}
		}
	}
	return None;
}

simulated event ReplicatedEvent(Name VarName)
{
	if (VarName == nameof(PawnClass))
	{
		CheckForPrestreamingTextures();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated final function SetPawnClass(class<GearPawn> NewPawnClass)
{
	if (PawnClass != NewPawnClass)
	{
		PawnClass = NewPawnClass;
		CheckForPrestreamingTextures();
	}
}

simulated final function CheckForPrestreamingTextures()
{
	local GearPRI PRI;
	// ignore SP versions of Marcus/Dom since those are set to never stream and this would overwrite that
	if (PawnClass != None && PawnClass != class'GearPawn_COGMarcus' && PawnClass != class'GearPawn_COGDom')
	{
		PRI = FindLocalPRI();
		if (PRI == None || PRI.Team == Team)
		{
			`log("Streaming textures for"@`showvar(PawnClass));
			PawnClass.default.Mesh.PrestreamTextures(25.f,TRUE);
		}
	}
}

/**
 * Setter for PlayerStatus
 *
 * @todo - since PlayerStatus is a databinding, need to make PlayerStatus private so that we can issue an update anytime it's modified.
 */
simulated function SetPlayerStatus( EGearPlayerStatus NewStatus )
{
	local bool bIssueUpdate;

	bIssueUpdate = PlayerStatus != NewStatus;
	PlayerStatus = NewStatus;

	if ( bIssueUpdate )
	{
		UpdatePlayerDataProvider(nameof(PlayerStatus));
	}
}

/**
 * Wrapper function for calling the GearPC function that checks for TG tutorial
 *
 * @param TutType - Tutorial type to attempt
 * @param TrainType - the traing gound session we are in
 * @param TeamCheck - 0 Tests against same team only, 1 Tests against enemy team only, -1 Doesn't care
 * @param VictimPRI - The PRI of the victim for which we are doing our TeamCheck against
 */
final function AttemptTrainingGroundTutorial(EGearTutorialType TutType, EGearTrainingType TrainType, optional int TeamCheck = -1, optional GearPRI VictimPRI = none)
{
	local GearPC PC;
	local bool bProcessAttempt;

	// See if we need to play a TG tutorial
	if (GearGRI(WorldInfo.GRI).TrainingGroundsID >= 0)
	{
		PC = GearPC(Owner);
		if (PC != none)
		{
			bProcessAttempt = (TeamCheck == -1);
			if (!bProcessAttempt && VictimPRI != none && VictimPRI != self)
			{
				if (TeamCheck == 0)
				{
					bProcessAttempt = (PC.PlayerReplicationInfo.GetTeamNum() == VictimPRI.GetTeamNum());
				}
				else
				{
					bProcessAttempt = (PC.PlayerReplicationInfo.GetTeamNum() != VictimPRI.GetTeamNum());
				}
			}

			if (bProcessAttempt)
			{
				PC.AttemptTrainingGroundTutorial(TutType, TrainType);
			}
		}
	}
}

/** scales all the scoring values (PointsFor_*) by the specified multiplier
 * used by e.g. Horde to give weaker/stronger enemies appropriately scaled point values
 */
final function ScaleScoringValues(float Multiplier)
{
	PointsFor_TakeDowns *= Multiplier;
	PointsFor_Executions *= Multiplier;
	PointsFor_InstantKills *= Multiplier;
	PointsFor_Revives *= Multiplier;
	PointsFor_CurbStomps *= Multiplier;
	PointsFor_DamagePct *= Multiplier;
}

/**
 * Player has been damaged, track the total done to be scored on execution.
 */
final function ScoreHit(GearPRI InstigatorPRI, float MaxHealth, float CurrentHealth, float DamageInflicted, class<GearDamageType> DamageType)
{
	local float ActualDamageInflicted;
	local int PlayerIdx;
	local PlayerDamageInfo DummyDamageInfo;

	if (GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress && InstigatorPRI != self)
	{
		ActualDamageInflicted = Clamp(DamageInflicted, 1, CurrentHealth);
		PlayerIdx = PlayerDamageList.Find('PRI',InstigatorPRI);
		if (PlayerIdx == INDEX_NONE)
		{
			// add new entry
			DummyDamageInfo.PRI = InstigatorPRI;
			DummyDamageInfo.DamageTotal = 0;
			PlayerIdx = PlayerDamageList.Length;
			PlayerDamageList.Length = PlayerIdx + 1;
			PlayerDamageList[PlayerIdx] = DummyDamageInfo;
		}

		PlayerDamageList[PlayerIdx].DamageTotal += ActualDamageInflicted;

		if (InstigatorPRI != None)
		{
			// @STATS - Track Melee Hits
			if ( ClassIsChildOf(DamageType, class'GDT_Melee') )
			{
				AggregateMeleeHit(true);
				InstigatorPRI.AggregateMeleeHit(false);
			}
			else
			{
				`RecordStat(STATS_LEVEL7,'TookDamage',Controller(Owner),DamageType,Controller(InstigatorPRI.Owner));
				InstigatorPRI.AggWeaponFireStat(DamageType.default.WeaponID,false);
			}
	    }
	}
}

/** Central point to modify a player's score. */
final function ModifyScore(int Amt)
{
	if (GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress)
	{
		`RecordStat(STATS_LEVEL4,'UpdatedScore',Controller(Owner),Score);
		Score += Amt;
		// kinda hacky but it seems overly complicated to call into the GameInfo just to end up back in here
		// really this function itself should be in GearGame :)
		if (GearGameHorde_Base(WorldInfo.Game) != None)
		{
			Score_GameSpecific2 += Amt;
			if (Team != None)
			{
				Team.Score += Amt;
			}
		}
		bForceNetUpdate = TRUE;
	}
}

/** Called from ScoreKnockdown() as a result of damaging somebody to the DBNO state. */
final function ScoreTakeDown(GearPRI VictimPRI, class<GearDamageType> TakedownType)
{
	local GearPC PC;
	if (GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress)
	{


		if ( ClassIsChildOf(TakedownType,class'GDT_Melee') )
		{
			AggregateMeleeKnockDown(false);
		}
		else
		{
			// Score the take down for this weapon
			AggWeaponTakedown(TakedownType.default.WeaponID, false);
		}

		if ( Team == VictimPRI.Team )
		{
			// negative score for friendly takedowns
			ModifyScore(-VictimPRI.PointsFor_TakeDowns);
		}
		else
		{
			Score_Takedowns++;
			// skip awarding points for the meatflag takedown
			if (Controller(VictimPRI.Owner) == None || Controller(VictimPRI.Owner).Pawn == None || !Controller(VictimPRI.Owner).Pawn.IsA('GearPawn_MeatflagBase'))
			{
				PC = GearPC(Owner);
				if (PC != None)
				{
					PC.AddDeathMessage(self,VictimPRI,class'GDT_KnockedDown',GDT_NORMAL,VictimPRI.PointsFor_TakeDowns,FALSE,TRUE);
				}
				ModifyScore(VictimPRI.PointsFor_TakeDowns);
			}

			// See if we need to play a TG tutorial
			if (VictimPRI.bIsLeader)
			{
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_ExeLead, eGEARTRAIN_Respawn, 1, VictimPRI);
			}
			if (VictimPRI.bIsMeatflag)
			{
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_MeatPick, eGEARTRAIN_Meatflag, 1, VictimPRI);
			}
		}
	}
}

/** Called when player is knocked to DBNO via damage, automatically calls ScoreTakedown() for the instigator. */
final function ScoreKnockdown(GearPRI InstigatorPRI, class<GearDamageType> KnockdownType)
{
	if (GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress)
	{
		// stats tracking
		`RecordStat(STATS_LEVEL2,'KnockedDown', Controller(Owner), KnockdownType, Controller(InstigatorPRI.Owner));
		Was_TakenDown++;
		if (InstigatorPRI != None)
		{

			if ( ClassIsChildOf(KnockDownType,class'GDT_Melee') )
			{
				AggregateMeleeKnockDown(true);
			}
			else
			{
				AggWeaponTakedown(KnockdownType.default.WeaponID,true);
			}

			InstigatorPRI.ScoreTakeDown(self, KnockdownType);
		}
		// Mark the last player to DBNO me
		LastToDBNOMePRI = InstigatorPRI;
	}
}

/** Called when this player revives another. */
final function ScoreRevive(GearPRI VictimPRI)
{
	if (GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress)
	{
		ModifyScore(VictimPRI.PointsFor_Revives);
		if (GearPC(Owner) != None)
		{
			GearPC(Owner).AddScoreGameMessage(69,VictimPRI.PointsFor_Revives,VictimPRI);
		}
		`RecordStat(STATS_LEVEL2,'Revived',Controller(VictimPRI.Owner),"",Controller(Owner));
		Score_Revives++;
		// clear the player damage list on the target
		if (VictimPRI != None)
		{
			VictimPRI.Was_Revived++;
			VictimPRI.PlayerDamageList.Length = 0;
		}
	}
}

/** Called when a player is normally executed from a DBNO. */
final function ScoreExecution(GearPRI VictimPRI, class<GearDamageType> KillType, EGearDeathType DeathType)
{
	local int Pts;
	if (GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress && VictimPRI != None && VictimPRI.PlayerStatus != WPS_Dead)
	{
		Pts = VictimPRI.PointsFor_Executions;
		if ( Team == VictimPRI.Team )
		{
			if (VictimPRI != self)
			{
				NoTeamKills++;
			}
			// negative score for friendly takedowns
			Pts = -Pts;
		}
		else
		{
			Score_Kills++;
		}

		if (KillType == class'GDT_BledOut')
		{
			`RecordStat(STATS_LEVEL2,'BledOut',Controller(VictimPRI.Owner),string(KillType),Controller(Owner));
			Pts = 0;
		}
		else if ( ClassIsChildOf(KillType, class'GDT_Execution_Base')  )
		{

			// Hack to get around meatbagging the Meat giving score and executions
			if ( GearAI_CTMVictim(VictimPRI.Owner) == none )
			{
				//@STATS
				Score_Executions++;
				VictimPRI.Was_Executed++;

				`RecordStat(STATS_LEVEL2,'Executed',Controller(VictimPRI.Owner),string(KillType),Controller(Owner));
			}
		}
		else
		{
			AggWeaponFireStat(KillType.default.WeaponID, false);	// Aggregate the hit
		}
		// track the death for the victim
		VictimPRI.ScoreDeath(self, KillType, DeathType, Pts);

		// See if we need to play a TG tutorial for killing an enemy who will respawn
		if (GearGRI(WorldInfo.GRI).TrainingGroundsID >= 0)
		{
			if (!VictimPRI.bIsLeader && GearGame(WorldInfo.Game).TeamHasRespawn(VictimPRI.GetTeamNum()))
			{
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_KSpawn, eGEARTRAIN_Respawn, 1, VictimPRI);
			}
		}
	}
	else
	{
		`log("Discarded kill!"@`showvar(VictimPRI)@`showvar(self));
	}
}

/** Called when a player is killed instantly, bypassing DBNO. */
final function ScoreInstantKill(GearPRI VictimPRI, class<GearDamageType> KillType, EGearDeathType DeathType)
{
	local GearPawn Victim;
	local int Pts;
	if (GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress && VictimPRI != None && VictimPRI.PlayerStatus != WPS_Dead)
	{
		// only award points if it isn't the final down rule
		Victim = GearPawn(Controller(VictimPRI.Owner).Pawn);
		if (Victim == None || Victim.DownCount < Victim.MaxDownCount)
		{
			Pts = VictimPRI.PointsFor_InstantKills;
		}
		if (Team == VictimPRI.Team)
		{
			if (VictimPRI != self)
			{
				NoTeamKills++;
			}
			Pts = -Pts;
		}
		else
		{
			Score_Kills++;
		}

		//@STATS
		if (DeathType == GDT_Headshot)
		{
			`RecordStat(STATS_LEVEL2,'Headshot',Controller(VictimPRI.Owner),string(KillType),Controller(Owner));
			NoHeadShots++;
			VictimPRI.NoWasHeadShot++;
		}
		else
		{
			`RecordStat(STATS_LEVEL2,'Gibbed',Controller(VictimPRI.Owner),string(KillType),Controller(Owner));
			NoGibs++;
			VictimPRI.NoWasGibbed++;
		}

		// Record the hit
		if ( !ClassIsChildOf(KillType,class'GDT_Execution_Base') && KillType != class'GDT_TorqueBow_Explosion' && DeathType != GDT_HEADSHOT  )
		{
	        AggWeaponFireStat(KillType.default.WeaponID,false);
	    }

		// track the death for the victim
		VictimPRI.ScoreDeath(self, KillType, DeathType, Pts);

		// See if we need to play a TG tutorial for killing an enemy who will respawn
		if (GearGRI(WorldInfo.GRI).TrainingGroundsID >= 0)
		{
			if (!VictimPRI.bIsLeader && GearGame(WorldInfo.Game).TeamHasRespawn(VictimPRI.GetTeamNum()))
			{
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_KSpawn, eGEARTRAIN_Respawn, 1, VictimPRI);
			}
		}
	}
	else
	{
		`log("Discarded kill!"@`showvar(VictimPRI)@`showvar(self));
	}
}

function IncrementDeaths(optional int Amt = 1)
{
	// this is handled via ScoreDeath
}

/** Called when this player is killed, gives out points to all players contributing to the kill. */
final function ScoreDeath(GearPRI KillerPRI, class<GearDamageType> KillType, EGearDeathType DeathType, optional int AdditionalPts)
{
	local float TotalDamage, AwardedScore;
	local int Idx;
	local GearPRI AssistPRI;
	local int AssistScore;
	local GearPC PC;
	local array<GearPC> NotifiedPCs;
	local bool bNotifiedKiller;

	if (AdditionalPts != 0)
	{
		if (KillerPRI != None)
		{
			KillerPRI.ModifyScore(AdditionalPts);
		}
		else
		{
			ModifyScore(AdditionalPts);
		}
	}

	//@STATS
	TotalAliveTime += WorldInfo.TimeSeconds - AliveStartTime;

	// mark this PRI as dead to prevent extra points awarded if someone gibs the person being executed/chainsawed/etc
	SetPlayerStatus(WPS_Dead);

	// tally death

	// Track some stats

	if ( ClassIsChildOf(KillType,class'GDT_Chainsaw') )
	{
		AggregateChainsawKill(true);
		if (KillerPRI != none && KillerPRI != self)
		{
			KillerPRI.AggregateChainsawKill(false);
		}
	}
	else if (KillType == class'GDT_FragMartyr' || KillType == class'GDT_InkMartyr')
	{
		KillerPRI.NoGrenadeMartyrDeaths++;
		NoTimesIDiedByGrenadeMartyr++;
	}
	else if ( ClassIsChildOf(KillType,class'GDT_Melee') )
	{
		AggregateMeleeKill(true);
		if (KillerPRI != none && KillerPRI != self)
		{
			KillerPRI.AggregateMeleeKill(false);
		}
	}
	else if ( !ClassIsChildOf(KillType,class'GDT_Execution_Base') && KillType != class'GDT_BledOut' )
	{
		AggWeaponKillStat(KillType.default.WeaponID,true);
		if (KillerPRI != none && KillerPRI != self)
		{
			KillerPRI.AggWeaponKillStat(KillType.default.WeaponID,false);
		}
	}

    bSkipWeaponAggregate = false;

	// If we are on a killing streak, record it here
	if (KillStreak>MaxKillStreak)
	{
		MaxKillStreak = KillStreak;
	}
	KillStreak = 0;

	// Log the kill for the killer's streak
	if (KillerPRI != None && KillerPRI != self && GearAI_CTMVictim(Owner) == None)
	{
		KillerPRI.KillStreak++;
	}

	Deaths += 1;
	// and award assists
	AssistPRI = None;
	AssistScore = 0;
	// calculate the total first
	for (Idx = 0; Idx < PlayerDamageList.Length; Idx++)
	{
		TotalDamage += PlayerDamageList[Idx].DamageTotal;
	}

	// @STATS - Score stats for a team kill
	if ( WorldInfo.GRI.OnSameTeam(KillerPRI.Owner, Owner) )
	{
		NoTeamKills++;
	}

	// award point pool based on each players damage contribution
	for (Idx = 0; Idx < PlayerDamageList.Length; Idx++)
	{
		if (PlayerDamageList[Idx].PRI != None)
		{
			AwardedScore = PlayerDamageList[Idx].DamageTotal/TotalDamage * PointsFor_DamagePct;
			if (AwardedScore > AssistScore && AwardedScore > 10 && PlayerDamageList[Idx].PRI != KillerPRI)
			{
				AssistPRI = PlayerDamageList[Idx].PRI;
				AssistScore = AwardedScore;
			}

			if (WorldInfo.GRI.OnSameTeam(PlayerDamageList[Idx].PRI, self))
			{
				AwardedScore = -AwardedScore;
			}

			// We removed the Gave Damage Stat.  It's not that important since we already score the hit
			`RecordStat(STATS_LEVEL7,'GaveDamage',Controller(PlayerDamageList[Idx].PRI.Owner),PlayerDamageList[Idx].DamageTotal);
			PlayerDamageList[Idx].PRI.ModifyScore(AwardedScore);
			// send the death message to the player
			//@note - skip for negative, font doesn't support that character, so just skip pts for those messages
			PC = GearPC(PlayerDamageList[Idx].PRI.Owner);
			if (PC != None && AwardedScore > 0)
			{
				if (PlayerDamageList[Idx].PRI == KillerPRI)
				{
					bNotifiedKiller = TRUE;
					PC.AddDeathMessage(KillerPRI,self,KillType,DeathType,AwardedScore+AdditionalPts);
				}
				else
				{
					PC.AddDeathMessage(KillerPRI,self,KillType,DeathType,AwardedScore);
				}
				NotifiedPCs.AddItem(PC);
			}
		}
	}
	if (!bNotifiedKiller && KillerPRI != None && GearPC(KillerPRI.Owner) != None)
	{
		GearPC(KillerPRI.Owner).AddDeathMessage(KillerPRI,self,KillType,DeathType,AdditionalPts);
		NotifiedPCs.AddItem(GearPC(KillerPRI.Owner));
	}


	if (AssistPRI != none && AssistPRI != self)
	{
		`RecordStat(STATS_LEVEL2,'Assist',Controller(AssistPRI.Owner));
		AssistPRI.Score_Assists++;
	}
	// empty list just to be safe
	PlayerDamageList.Length = 0;
	bForceNetUpdate = TRUE;
	// notify other players who didn't contribute
	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		if (NotifiedPCs.Find(PC) == INDEX_NONE)
		{
			PC.AddDeathMessage(KillerPRI,self,KillType,DeathType);
		}
	}

	// Check for TG tutorial
	if (WorldInfo.Game != none && GearGRI(WorldInfo.GRI).TrainingGroundsID >= 0)
	{
		if (!bIsLeader)
		{
			if (GearGame(WorldInfo.Game).TeamHasRespawn(GetTeamNum()))
			{
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_DSpawn, eGEARTRAIN_Respawn);
			}
			else
			{
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_UDed, eGEARTRAIN_Respawn);
			}
		}
	}
}

/** Generic accessor for game specific scoring */
protected final function ScoreGameSpecific(Name Type, string Desc, float Points, out int GameSpecificScore, optional int Id, optional bool bSkipHUDUpdate)
{
	if (GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress)
	{
		GameSpecificScore++;
		if (Points != 0)
		{
			ModifyScore(Points);
		}
		if (GearPC(Owner) != None && !bSkipHUDUpdate)
		{
			GearPC(Owner).AddScoreGameMessage(Id,Points);
		}
		`RecordStat(STATS_LEVEL1,Type,Controller(Owner),Desc,None);
	}
}
final function ScoreGameSpecific1(Name Type, string Desc, optional float Points, optional bool bSkipHUDUpdate) { ScoreGameSpecific(Type,Desc,Points,Score_GameSpecific1,0,bSkipHUDUpdate); }
final function ScoreGameSpecific2(Name Type, string Desc, optional float Points, optional bool bSkipHUDUpdate) { ScoreGameSpecific(Type,Desc,Points,Score_GameSpecific2,1,bSkipHUDUpdate); }
final function ScoreGameSpecific3(Name Type, string Desc, optional float Points, optional bool bSkipHUDUpdate) { ScoreGameSpecific(Type,Desc,Points,Score_GameSpecific3,2,bSkipHUDUpdate); }

/** === Begin Stat accessor functions === */

simulated final function int GetKills()			{ return Score_Kills; }
simulated final function int GetTakeDowns()		{ return Score_Takedowns; }
simulated final function int GetDeaths()		{ return Deaths; }
simulated final function int GetRevives()		{ return Score_Revives; }
simulated final function int GetScore()			{ return Score; }
simulated final function int GetGameScore1()	{ return Score_GameSpecific1; }
simulated final function int GetGameScore2()	{ return Score_GameSpecific2; }
simulated final function int GetGameScore3()	{ return Score_GameSpecific3; }

/** === End Stat accessor functions === */

/** Called from client to change team selection */
reliable server function MenuTeamChange(int NewTeam)
{
	WorldInfo.Game.ChangeTeam( PlayerController(Owner), NewTeam, true);
	bForceNetUpdate = TRUE;
}

/** Clear all variables dealing with someone killing this PRI */
final function ClearDeathVariables()
{
	LastIKilledPRI = None;
	LastToDBNOMePRI = None;
	LastToKillMePRI = None;
	DamageTypeToKillMe = None;
	bIsDead = false;
	PlayerDamageList.Length = 0;
}

/**
 * Notifies the game info that this player wishes to change his character class.
 *
 * @param	DesiredCharacterId	the id of the character to switch to; will be one of the values from either the ECogMPCharacter
 *								or ELocustMPCharacter enums, depending on the player's team.
 */
reliable server function ServerSetCharacterId( byte CharacterId )
{
	local GearGame GearGI;

	GearGI = GearGame(WorldInfo.Game);
	if ( GearGI != None )
	{
		GearGI.SetPlayerClassFromPlayerSelection(Self, CharacterId);
		UpdatePlayerDataProvider(nameof(SelectedCharacterProfileId));
		UpdatePlayerDataProvider(nameof(PawnClass));
	}
}

/**
 * Changes the player's default weapon class.
 *
 * @param	WeaponId	the id of the weapon to switch to
 */
reliable server function ServerSetWeaponId( byte WeaponId )
{
	InitialWeaponType = WeaponId;
	UpdatePlayerDataProvider(nameof(InitialWeaponType));
}

reliable server function MenuSetReady(bool bIsReadyToPlay)
{
	bReadyToPlay = bIsReadyToPlay;
	bForceNetUpdate = TRUE;
}

reliable server function HostIsReady()
{
	bHostIsReady = true;
	bForceNetUpdate = TRUE;
}

function Reset()
{
	Super.Reset();

	bHostIsReady = FALSE;
}

simulated function bool ShouldBroadCastWelcomeMessage(optional bool bExiting)
{
	return !bExiting && (!bBot && (GearPC(Owner) == None || !GearPC(Owner).bCheckpointDummy) && Super.ShouldBroadCastWelcomeMessage(bExiting));
}

reliable client simulated function ClientPlayEncouragementSound(int TeamIndex, EEncouragementType EncType, byte SoundIndex)
{
	local class<GearGame> TheGameClass;
	local int NumSounds;
	local SoundCue Cue;
	local AudioComponent AC;

	//@todo: FIXME: what about splitscreen? Do we really want to play two sounds?
	if (PlayerController(Owner) != None)
	{
		TheGameClass = class<GearGame>(WorldInfo.GRI.GameClass);

		if ( TeamIndex == 0 )
		{
			NumSounds = TheGameClass.default.EncouragementData[EncType].COGSounds.Length;
			if ( NumSounds > 0 )
			{
				SoundIndex = SoundIndex % byte(NumSounds);
				Cue = TheGameClass.default.EncouragementData[EncType].COGSounds[ SoundIndex ];
			}
		}
		else
		{
			NumSounds = TheGameClass.default.EncouragementData[EncType].LocustSounds.Length;
			if ( NumSounds > 0 )
			{
				SoundIndex = SoundIndex % byte(NumSounds);
				Cue = TheGameClass.default.EncouragementData[EncType].LocustSounds[ SoundIndex ];
			}
		}

		if ( Cue != None )
		{
			AC = Owner.CreateAudioComponent(Cue, false, true);
		}

		if (AC != None)
		{
			AC.bUseOwnerLocation = TRUE;
			AC.bAutoDestroy = TRUE;
			AC.bSuppressSubtitles = TRUE;
			AC.VolumeMultiplier = 1.25f;
			AC.bAllowSpatialization = FALSE;
			AC.Play();
		}
	}

}

/** Plays when your coop buddy dies during a split */
unreliable client simulated function ClientPlayCoopSplitDeathSound( bool bMarcusDied )
{
	local AudioComponent AC;
	local SoundCue Cue;
	local GearPC PC;
	local GearPawn WP;
	local class<GearGame> TheGameClass;

	TheGameClass = class<GearGame>(WorldInfo.GRI.GameClass);
	Cue = bMarcusDied ? TheGameClass.default.MarcusIsDownCue : TheGameClass.default.DomIsDownCue;
	PC = GearPC(Owner);

	if ( (PC != None) && (Cue != None) )
	{
		WP = GearPawn(PC.Pawn);
		if ( (WP != None) && !WP.IsDBNO() && !WP.bTearOff )
		{
			AC = Owner.CreateAudioComponent(Cue, false, true);
			AC.bUseOwnerLocation = TRUE;
			AC.bAutoDestroy = TRUE;
			AC.bSuppressSubtitles = FALSE;
			AC.VolumeMultiplier = 1.25f;
			AC.Play();
		}
	}
}


function EndOfGameEncouragement(byte EncouragementIndex)
{
	local int MyTeam, OtherTeam, ScoreDiff;
	local EEncouragementType EncType;

	if (Team == None)
	{
		return;
	}

	MyTeam = Team.TeamIndex;
	OtherTeam = MyTeam == 0 ? 1 : 0;

	`log("ENCOURAGEMENT myteam"@MyTeam@WorldInfo.GRI.Teams[MyTeam].Score@"othertheam"@OtherTeam@WorldInfo.GRI.Teams[OtherTeam].Score);

	if (WorldInfo.GRI.Teams[MyTeam].Score == 0)
	{
		// shutout loss
		EncType = ET_LostMatchByShutout;
	}
	else if (WorldInfo.GRI.Teams[OtherTeam].Score == 0)
	{
		// shutout win
		EncType = ET_WonMatchByShutout;
	}
	else
	{
		ScoreDiff = WorldInfo.GRI.Teams[MyTeam].Score - WorldInfo.GRI.Teams[OtherTeam].Score;
		if (ScoreDiff == 0)
		{
			EncType = ET_MatchOrRoundDraw;
		}
		else if (ScoreDiff > 0)
		{
			EncType = ET_WonMatch;
		}
		else
		{
			EncType = ET_LostMatch;
		}
	}

	// play dialogue
	ClientPlayEncouragementSound(MyTeam, EncType, EncouragementIndex);
}


function EndOfRoundEncouragement(byte EncouragementIndex)
{
	local EEncouragementType EncType;
	local TeamInfo WinningTeam;

	if (Team == None)
	{
		return;
	}

	WinningTeam = TeamInfo(GearGRI(WorldInfo.GRI).Winner);

	if (WinningTeam == None)
	{
		// draw
		EncType = ET_MatchOrRoundDraw;
	}
	else if (Team == WinningTeam)
	{
		// my team won, yay for me
		EncType = ET_WonRound;
	}
	else
	{
		// we lost, boo lag teamz
		EncType = ET_LostRound;
	}

	// play dialogue
	ClientPlayEncouragementSound(Team.TeamIndex, EncType, EncouragementIndex);
}



simulated function ClientInitialize(Controller C)
{
	Super.ClientInitialize(C);

	// MP has no subtitles!
	if (GearGRI(WorldInfo.GRI) != None && GearGRI(WorldInfo.GRI).IsMultiPlayerGame())
	{
		PlayerController(C).SetShowSubtitles( false );
	}
}

function CopyProperties(PlayerReplicationInfo PRI)
{
	local GearPRI GPRI;

	Super.CopyProperties(PRI);

	GPRI = GearPRI(PRI);
	if (GPRI != None)
	{
		GPRI.SetPawnClass(PawnClass);
		GPRI.InitialWeaponType = InitialWeaponType;
		GPRI.SelectedCharacterProfileId = SelectedCharacterProfileId;
		GPRI.ServerSetCharacterId(SelectedCharacterProfileId);
	}
}

/**
 * @STATS
 * Finds a weapon stat index for a given weapon.  If the weapon isn't already being tracked, it will be added
 *
 * @Param WeaponID - The ID of the weapon we are reloading
 * @Param WSI - Weapon Stats Index - The Cached index
 * @retuns a cachable index in to the stats array
 */

function int ResolveWeaponStatIndex(EWeaponClass WeaponID, optional int WSI=-1)
{
	local int IDX;

	// Grab the cached WeaponStatIndex and validate it.
	if ( WSI>=0 && (WSI >= WeaponStats.Length || WeaponStats[WSI].WeaponID != WeaponID ) )
	{
		// It's invalid, reset
		WSI = -1;
	}

	// If we do not have a valid WSI, then find/add this weapon to this player's stats list
	if (WSI<0)
	{
		// Look for it
		for (IDX=0;IDX<WeaponStats.Length;IDX++)
		{
			if (WeaponStats[IDX].WeaponID == WeaponID)
			{
				WSI = IDX;
				break;
			}
		}

		if (WSI<0)	// Add it
		{
			WSI = WeaponStats.Length;
			WeaponStats.Length = WeaponStats.Length + 1;
			WeaponStats[WSI].WeaponID = WeaponID;

		}
	}

	return WSI;
}


/**
 * @STATS
 * Aggregates a weapon stat.
 *
 * @Param WeaponID - The ID of the weapon we are reloading
 * @param bFired  - True if this weapon was fired, otherwise it's a hit/damage with
 * @Param WSI - Weapon Stats Index - The Cached index
 * @retuns a cachable index in to the stats array
 */

function int AggWeaponFireStat(EWeaponClass WeaponID, bool bFired, optional int WSI=-1)
{

	// Immediately return if this is an invalid weapon
	if ( WeaponID == WC_WretchMeleeSlash)
	{
		return -1;
	}


	WSI = ResolveWeaponStatIndex(WeaponID, WSI);

	// track the stat


	if (bFired)
	{
		WeaponStats[WSI].ShotsFired++;
	}
	else
	{
		WeaponStats[WSI].ShotsHit++;
	}

	return WSI;
}

/**
 * @Stats
 * Aggregate a take down for a given weapon
 *
 * @Param WeaponID - The ID of the weapon we are reloading
 * @Param bWasTakenDown - will be true if the Player was taken down
 */

function AggWeaponTakedown(EWeaponClass WeaponID, bool bWasTakenDown)
{
	local int WSI;

	// Immediately return if this is an invalid weapon
	if ( WeaponID == WC_WretchMeleeSlash)
	{
		return;
	}

	WSI = ResolveWeaponStatIndex(WeaponID);

	if (bWasTakenDown)
	{
		WeaponStats[WSI].WasKnockedDown++;
	}
	else
	{
    	WeaponStats[WSI].Knockdowns++;
    }
}

/**
 * @STATS
 * Aggregates a weapon stat.
 *
 * @Param WeaponID - The ID of the weapon we are reloading
 * @param bWasKilled - True if the player was killed with this weapon
 * @Param WSI - Weapon Stats Index - The Cached index
 * @retuns a cachable index in to the stats array
 */

function int AggWeaponKillStat(EWeaponClass WeaponID, bool bWasKilled, optional int WSI=-1)
{

	// Immediately return if this is an invalid weapon
	if ( WeaponID == WC_WretchMeleeSlash)
	{
		return -1;
	}

	WSI = ResolveWeaponStatIndex(WeaponID, WSI);

	// track the stat


	if (bWasKilled)
	{
		WeaponStats[WSI].KilledBy++;
	}
	else
	{
		WeaponStats[WSI].KillsWith++;
	}

	return WSI;
}


/**
 * @STATS
 * Aggregates a weapon's reload stats
 *
 * @Param WeaponClass - The class of weapon to track
 * @param ReloadType - Type of reload, 0 = Skipped, 1 = Failure, 2= success, 3=super success
 * @retuns a cachable index in to the stats array
 */

function int AggWeaponReloadStat(EWeaponClass WeaponID, int ReloadType, int WSI)
{
	WSI = ResolveWeaponStatIndex(WeaponID, WSI);

	switch (ReloadType)
	{
		case 0:	WeaponStats[WSI].ReloadSkips++; break;
		case 1: WeaponStats[WSI].ReloadFailures++; break;
		case 2: WeaponStats[WSI].ReloadSuccess++; break;
		case 3: WeaponStats[WSI].ReloadSuperSuccess++; break;
	}

	return WSI;
}


/**
 * @Stats
 * At the end of a round, store off the last time stats
 */
function RecordEndRound()
{
	local GearPawn P;

	TotalAliveTime += WorldInfo.TimeSeconds - AliveStartTime;

	P = GearPawn(Controller(Owner).Pawn);
	if (P != none && P.SpecialMove == SM_RoadieRun)
	{
		TotalRoadieRunTime += WorldInfo.TimeSeconds - RoadieRunStartTime;
		if (P.CoverType != CT_None)
		{
			TotalTimeInCover += WorldInfo.TimeSeconds - P.LastCoverTime;
		}
	}
}

/**
 * @STATS
 * Create stats events for the end of game stats
 */

function RecordEndGameStats()
{
	local string Values;
	local int i;

	// If this is WingMan... put the team score in to GameSpecific1
	if ( GearGameWingman_Base(WorldInfo.Game) != none )
	{
		Score_GameSpecific1 = int(Team.Score);
	}

	Values = string(int(Score));
	Values = Values $","$ Score_Kills $","$ Score_Takedowns $","$ Score_Revives $","$ Score_Assists $","$ Score_Executions;
	Values = Values $","$ int(Deaths) $","$ Was_TakenDown $","$ Was_Revived $","$ Was_Executed;
	Values = Values $","$ NoGibs $","$ NoWasGibbed;
	Values = Values $","$ NoHeadShots $","$ NoWasHeadShot;
	Values = Values $","$ NoGrenadeTags $","$ NoTimesGrenadeTagged;
	Values = Values $","$ NoGrenadeMines $","$ NoGrenadeMinesTriggered $","$ NoGrenadeMinesTriggeredByMe;
	Values = Values $","$ NoGrenadeMartyr $","$ NoGrenadeMartyrDeaths $","$ NoTimesIDiedByGrenadeMartyr;
	Values = Values $","$ NoTeamKills;
	Values = Values $","$ Score_GameSpecific1 $","$ Score_GameSpecific2 $","$ Score_GameSpecific3;


	`RecordStat(STATS_LEVEL1,'EndGamePlayer',Controller(Owner),Values);
	`RecordStat(STATS_LEVEL1,'TotalTimes',Controller(Owner),TotalAliveTime $ ","  $ TotalRoadieRunTime $","$ TotalTimeInCover);

	//`log("### [STATS] "@PlayerName);

	for (i=0;i<WeaponStats.Length;i++)
	{
		Values = string(WeaponStats[i].WeaponID);
		Values = Values $","$ WeaponStats[i].KillsWith $","$ WeaponStats[i].KilledBy;
		Values = Values $","$ WeaponStats[i].Knockdowns $","$ WeaponStats[i].WasKnockedDown;
		Values = Values $","$ WeaponStats[i].ShotsFired $","$ WeaponStats[i].ShotsHit;
		Values = Values $","$ WeaponStats[i].ReloadSkips;
		Values = Values $","$ WeaponStats[i].ReloadFailures;
		Values = Values $","$ WeaponStats[i].ReloadSuccess;
		Values = Values $","$ WeaponStats[i].ReloadSuperSuccess;
		`RecordStat(STATS_LEVEL1,'WeaponStat',Controller(Owner),Values);
		//`log("### Weapon Stat:"@Values);

	}

	`RecordStat(STATS_LEVEL1,'Flamethrower',Controller(Owner),TotalFlameTime $","$ TotalFlameDmgTime);
 	`RecordStat(STATS_LEVEL1,'Chainsaw',Controller(Owner),NoChainsawKills $","$ NoChainsawDeaths $","$ NoChainsawDuelsWon $","$ NoChainsawDuelsLost $","$ NoChainsawDuelsTied);
 	`RecordStat(STATS_LEVEL1,'Melee',Controller(Owner),NoMeleeKills $","$ NoMeleeDeaths $","$ NoMeleeKnockdowns $","$ NoMeleeWasKnockedDown $","$ NoMeleeHits $","$ NoTimesMeleeHit);

	// If we are on a killing streak, record it here

	if (KillStreak > MaxKillStreak)
	{
		MaxKillStreak = KillStreak;
	}

	if (MaxKillStreak>0)
	{
		`RecordStat(STATS_LEVEL1,'KillStreak',Controller(Owner),MaxKillStreak);
	}

}

/**
 * @STATS
 * Collect stat data regarding the chainsaw
 *
 * @Param	bWon		Was the duel won?
 * @Param	bTied		Was the duel tied?
 */
function AggregateChainSawDuel(bool bWon, bool bTied)
{
	if (!bTied)
	{
		if (bWon)
		{
			NoChainsawDuelsWon++;
		}
		else
		{
			NoChainsawDuelsLost++;
		}
	}
	else
	{
		NoChainsawDuelsTied++;
	}
}

/**
 * @STATS
 * Collect stat data about melee hits
 *
 * @Param	bWasHit		Was this player hitting or being it
 */
function AggregateMeleeHit(bool bWasHit)
{

	if (bWasHit)
	{
		NoTimesMeleeHit++;
	}
	else
	{
		NoMeleeHits++;
	}
}

/**
 * @STATS
 * Collect stat data about melee hits
 *
 * @Param	bWasKilled		Was this player killed
 */
function AggregateMeleeKill(bool bWasKilled)
{

	if (bWasKilled)
	{
		NoMeleeDeaths++;
	}
	else
	{
		NoMeleeKills++;
	}
}

/**
 * @STATS
 * Collect stat data about melee hits
 *
 * @Param	bWasKilled		Was this player killed
 */
function AggregateMeleeKnockDown(bool bWasKnockedDown)
{

	if (bWasKnockedDown)
	{
		NoMeleeWasKnockedDown++;
	}
	else
	{
		NoMeleeKnockdowns++;
	}
}


/**
 * @STATS
 * Collect stat data about chainsaw kills
 *
 * @Param	bWasKilled		Was this player killed
 */
function AggregateChainsawKill(bool bWasKilled)
{

	if (bWasKilled)
	{
		NoChainsawDeaths++;
	}
	else
	{
		NoChainsawKills++;
	}
}

/**
 * Super, duper mega hack. JIP breaks the session registration process so hack
 * the player into the party session when you don't have a game session but do
 * have a party session
 */
simulated function RegisterPlayerWithSession()
{
	local OnlineSubsystem Online;

	Super.RegisterPlayerWithSession();

	Online = class'GameEngine'.static.GetOnlineSubsystem();
	if (Online != None &&
		Online.GameInterface != None &&
		SessionName == 'Game' &&
		Online.GameInterface.GetGameSettings(SessionName) == None &&
		Online.GameInterface.GetGameSettings('Party') != None)
	{
		// Register the player as part of the session
		Online.GameInterface.RegisterPlayer('Party',UniqueId,false);
	}
}

defaultproperties
{
`if(`notdefined(FINAL_RELEASE))
	PlayerName="Marcus"
`else
	PlayerName=" "
`endif

	GameMessageClass=class'GearGameMessage'

	bShowGore=TRUE
	bShowHUD=TRUE
	bShowPictograms=TRUE
	bAlwaysDrawWeaponIndicatorInHUD=FALSE
	ControllerSensitivityConfig=PCSO_Medium
	TargetSensitivityConfig=PCSO_Medium
	ZoomSensitivityConfig=PCSO_Medium

	Difficulty=class'GearGame.DifficultySettings_Hardcore'
	DLCFlag=-1

	PreferredCOGIndex=-1
	PreferredLocustIndex=-1
}
