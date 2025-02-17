/**
 * This will hold all of our enums and types and such that we need to
 * use in multiple files where the enum can't be mapped to a specific file.
 * Also to make these type available to the native land without forcing objects to be native.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTTypes extends Object
	native;


enum ECrossfadeType
{
	CFT_BeginningOfMeasure,
	CFT_EndOfMeasure
};


struct native MusicSegment
{
	/** Tempo in Beats per Minute. This allows for the specific segement to have a different BPM **/
	var() float TempoOverride;

	/** crossfading always begins at the beginning of the measure **/
	var ECrossfadeType CrossfadeRule;

	/**
	* How many measures it takes to crossfade to this MusicSegment
	* (e.g. No matter which MusicSegement we are currently in when we crossfade to this one we will
	* fade over this many measures.
	**/
	var() int CrossfadeToMeNumMeasuresDuration;

	var() SoundCue TheCue;

	structdefaultproperties
	{
		CrossfadeRule=CFT_BeginningOfMeasure;
		CrossfadeToMeNumMeasuresDuration=1
	}

};

struct native StingersForAMap
{
	var() SoundCue Died;
	var() SoundCue DoubleKill;
	var() SoundCue EnemyGrabFlag;
	var() SoundCue FirstKillingSpree;
	var() SoundCue FlagReturned;
	var() SoundCue GrabFlag;
	var() SoundCue Kill;
	var() SoundCue LongKillingSpree;
	var() SoundCue MajorKill;
	var() SoundCue MonsterKill;
	var() SoundCue MultiKill;
	var() SoundCue ReturnFlag;
	var() SoundCue ScoreLosing;
	var() SoundCue ScoreTie;
	var() SoundCue ScoreWinning;

};


struct native MusicForAMap
{
	/** Default Tempo in Beats per Minute. **/
	var() float Tempo;

	var() MusicSegment Action;
	var() MusicSegment Ambient;
	var() MusicSegment Intro;
	var() MusicSegment Suspense;
	var() MusicSegment Tension;
	var() MusicSegment Victory;
};

/** Live achievement defines (check UT3.spa.h for order)*/
enum EUTGameAchievements
{
	// Achievement IDs start from 1 so skip Zero
	EUTA_InvalidAchievement,
	EUTA_CAMPAIGN_CompleteAllTraining,
	EUTA_CAMPAIGN_DeployLeviathan,
	EUTA_CAMPAIGN_DefeatLauren,
	EUTA_CAMPAIGN_SignTreaty,
	EUTA_CAMPAIGN_DefeatSelig,
	EUTA_CAMPAIGN_LiandriMainframe,
	EUTA_CAMPAIGN_HijackDarkwalker,
	EUTA_CAMPAIGN_ControlTarydium,
	EUTA_CAMPAIGN_StealNecrisTech,
	EUTA_CAMPAIGN_ReachOmicron,
	EUTA_CAMPAIGN_DefeatLoque,
	EUTA_CAMPAIGN_SignTreatyExpert,
	EUTA_CAMPAIGN_LiandriMainframeExpert,
	EUTA_CAMPAIGN_ReachOmicronExpert,
	EUTA_CAMPAIGN_DefeatLoqueExpert,
	EUTA_CAMPAIGN_DefeatLaurenGodlike,
	EUTA_CAMPAIGN_DefeatSeligGodlike,
	EUTA_CAMPAIGN_DefeatMatrixGodlike,
	EUTA_CAMPAIGN_DefeatLoqueGodlike,
	EUTA_COOP_Complete1,
	EUTA_COOP_Complete10,
	EUTA_COOP_CompleteCampaign,
	EUTA_IA_EveryGameMode,
	EUTA_IA_Untouchable,
	EUTA_EXPLORE_AllPowerups,
	EUTA_EXPLORE_EveryMutator,
	EUTA_EXPLORE_EveryWeaponKill,
	EUTA_WEAPON_BrainSurgeon,
	EUTA_WEAPON_ShockTherapy,
	EUTA_WEAPON_GooGod,
	EUTA_WEAPON_Pistolero,
	EUTA_WEAPON_Hammerhead,
	EUTA_WEAPON_StrongestLink,
	EUTA_WEAPON_BombSquad,
	EUTA_WEAPON_BigGameHunter,
	EUTA_VEHICLE_Hoverboard,
	EUTA_VEHICLE_Armadillo,
	EUTA_VEHICLE_Gunner,
	EUTA_VEHICLE_Ace,
	EUTA_VEHICLE_Deathwish,
	EUTA_HUMILIATION_SerialKiller,
	EUTA_HUMILIATION_SirSlaysALot,
	EUTA_HUMILIATION_PathOfDestruction,
	EUTA_HUMILIATION_Deity,
	EUTA_VERSUS_GetItOn,
	EUTA_VERSUS_SameTeam,
	EUTA_VERSUS_Nemecide,
	EUTA_VERSUS_AroundTheWorld,
	EUTA_VERSUS_Dedication,
	EUTA_VERSUS_MeetInterestingPeople,
	EUTA_VERSUS_GetALife,
	EUTA_VERSUS_KillGetALifers,
	EUTA_RANKED_RememberYourFirst,
	EUTA_RANKED_DontHateThePlayer,
	EUTA_RANKED_MixItUp,
	EUTA_RANKED_HatTrick,
	EUTA_RANKED_BloodSweatTears,
	EUTA_RANKED_TenFootPole
};


enum EUTChapterType
{
	UTCT_JimBrownNeedsToSendMartinEmailWithTheChapterNames,
	UTCT_SoWeCanRockThisUpYo,
};


