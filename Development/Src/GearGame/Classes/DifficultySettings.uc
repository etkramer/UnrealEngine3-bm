/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class DifficultySettings extends Object
	abstract
	native
	config(Game);

// player health modifiers
var() config bool bCanDBNO;
var() config float PlayerHealthMod;
var() config float PlayerHealthRechargeDelayMod;
var() config float PlayerHealthRechargeSpeedMod;

// player weapon modifiers
var() config float PlayerDamageMod;
var() config float PlayerActiveReloadWindowMod;

// friendly ai modifiers
var() config float FriendHealthMod;
var() config float FriendHealthRechargeDelayMod;
var() config float FriendHealthRechargeSpeedMod;

// friendly ai modifiers
var() config float FriendDamageMod;

/** modifier to amount of time before AI pays attention to revive/execute */
var() config float ReviveAndExecuteDelayMod;

/** aiming modifier applied to MP bots */
var() config float MPBotAimErrorMultiplier;
/** active reload attempt/success multiplier for MP bots */
var() config float MPBotActiveReloadMultiplier;
/** hearing radius multiplier for MP bots */
var() config float MPBotHearingMultiplier;
/** rotation rate multiplier for MP bots */
var() config float MPBotRotationRateMultiplier;

/** Scales AI's chance to evade */
var() config float EnemyEvadeChanceScale;

/** whether the ally AI COG guys can be killed/bleedout while they're DBNO */
var() config bool bAllowAICOGDeathWhenDBNO;
/** whether human COG players can be executed while they're DBNO */
var() config bool bHumanCOGCanBeExecuted;

/** Horde score multiplier for playing on this difficulty */
var() config float HordeScoreMultiplier;
/** bleedout time of Pawns in Horde when playing on this difficulty */
var() config float HordeBleedOutTime;

/** whether to allow locust dudes to blind fire when in cover */
var() config bool bAllowLocustBlindFire;

var() const EDifficultyLevel DifficultyLevel;


static function ApplyDifficultySettings(Controller C)
{
	local GearPawn GP;
	local GearAI AI;

	Assert(C.Role == ROLE_Authority);

	AI = GearAI(C);
	if (AI != None && (C.WorldInfo.GRI.IsMultiplayerGame() || AI.GetTeamNum() != 0))
	{
		AI.ReviveDelay = AI.default.ReviveDelay + default.ReviveAndExecuteDelayMod;
		// remove the execute delay if we're playing an execution gametype as otherwise the AI looks unreasonably stupid
		if (GearGameMP_Base(C.WorldInfo.Game) != None && GearGameMP_Base(C.WorldInfo.Game).bExecutionRules)
		{
			AI.ExecuteDelay = 0.0;
		}
		else
		{
			AI.ExecuteDelay = AI.default.ExecuteDelay + default.ReviveAndExecuteDelayMod;
		}
	}

	if (GearGameSP_Base(C.WorldInfo.Game) != None || GearGameHorde_Base(C.WorldInfo.Game) != None)
	{
		if (C.IsA('PlayerController') || C.IsA('GearAI_TDM'))
		{
			GP = GearPawn(C.Pawn);
			if (GP != None)
			{
				GP.DefaultHealth = GP.default.DefaultHealth * default.PlayerHealthMod;
				GP.HealthMax = GP.DefaultHealth;
				GP.Health = GP.DefaultHealth;
				GP.HealthRechargeDelay = GP.default.HealthRechargeDelay * default.PlayerHealthRechargeDelayMod;
				GP.HealthRechargePercentPerSecond = GP.default.HealthRechargePercentPerSecond * default.PlayerHealthRechargeSpeedMod;
			}
		}
		else if (C.GetTeamNum() == 0)
		{
			GP = GearPawn(C.Pawn);
			if (GP != None)
			{
				GP.DefaultHealth = GP.default.DefaultHealth * default.FriendHealthMod;
				GP.HealthMax = GP.DefaultHealth;
				GP.Health = GP.DefaultHealth;
				GP.HealthRechargeDelay = GP.default.HealthRechargeDelay * default.FriendHealthRechargeDelayMod;
				GP.HealthRechargePercentPerSecond = GP.default.HealthRechargePercentPerSecond * default.FriendHealthRechargeSpeedMod;
			}
		}
	}

	if (C.WorldInfo.GRI.IsMultiplayerGame() && C.IsA('GearAI_TDM'))
	{
		AI.AimErrorMultiplier = default.MPBotAimErrorMultiplier;
		AI.RotationRateMultiplier = default.MPBotRotationRateMultiplier;
		AI.ActiveReloadAttemptPct = AI.default.ActiveReloadAttemptPct * default.MPBotActiveReloadMultiplier;
		AI.ActiveReloadSweetPct = AI.default.ActiveReloadSweetPct * default.MPBotActiveReloadMultiplier;
		AI.ActiveReloadJamPct = AI.default.ActiveReloadJamPct / default.MPBotActiveReloadMultiplier;
		if (AI.Pawn != None)
		{
			AI.Pawn.HearingThreshold = AI.Pawn.default.HearingThreshold * default.MPBotHearingMultiplier;
		}
	}
}

/** finds the human player with the lowest (easiest) difficulty setting */
static final function class<DifficultySettings> GetLowestPlayerDifficultyLevel(WorldInfo WI)
{
	local GearPC PC;
	local EDifficultyLevel Lowest;
	local class<DifficultySettings> LowestSettings;
	local GearPRI PCPRI;

	Lowest = DL_Insane;
	LowestSettings = class'GearPRI'.default.Difficulty;
	foreach WI.AllControllers(class'GearPC', PC)
	{
		PCPRI = GearPRI(PC.PlayerReplicationInfo);
		if (PCPRI != none && PCPRI.Difficulty.default.DifficultyLevel < Lowest)
		{
			Lowest = PCPRI.Difficulty.default.DifficultyLevel;
			LowestSettings = PCPRI.Difficulty;
		}
		else if(PCPRI != none && PCPRI.Difficulty.default.DifficultyLevel == Lowest)
		{
			LowestSettings = PCPRI.Difficulty;
		}
	}

	return LowestSettings;
}
