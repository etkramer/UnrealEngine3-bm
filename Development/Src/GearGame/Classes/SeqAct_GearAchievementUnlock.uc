/**
 * SeqAct_GearAchievementUnlock - Kismet action that allows achievements to be unlocked
 * by events that occur in levels.  This action will attempt to unlock the achievement
 * for all players.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GearAchievementUnlock extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void Activated();
}

/** Enum of the achievements to be unlocked by events that occur in levels */
enum EGearLevelAchievements
{
	eGLACHIEVE_None,
	eGLACHIEVE_GreenAsGrass,
	eGLACHIEVE_ItsATrap,
	eGLACHIEVE_EscortService,
	eGLACHIEVE_GirlAboutTown,
	eGLACHIEVE_ThatSinkingFeeling,
	eGLACHIEVE_Freebaird,
	eGLACHIEVE_HeartBroken,
	eGLACHIEVE_LongitudeAndAttitude,
	eGLACHIEVE_TanksForTheMemories,
	eGLACHIEVE_WaterSports,
	eGLACHIEVE_SavedMaria,
	eGLACHIEVE_WrappedInBeacon,
	eGLACHIEVE_HaveFunStormingTheCastle,
	eGLACHIEVE_AndTheHorseYourRodeInOn,
	eGLACHIEVE_TheyJustKeepComing,
	eGLACHIEVE_BrumakRodeo,
	eGLACHIEVE_DoesThisLookInfectedToYou,
	eGLACHIEVE_DLC1,
	eGLACHIEVE_DLC2,
	eGLACHIEVE_DLC3,
	eGLACHIEVE_DLC4,
	eGLACHIEVE_DLC5,
	eGLACHIEVE_DLC6,
	eGLACHIEVE_DLC7,
	eGLACHIEVE_DLC8,
	eGLACHIEVE_DLC9,
	eGLACHIEVE_DLC10,
	eGLACHIEVE_Seriously,
};

/**
 * Mapping of achievements that can be unlocked via this kismet action to the real achievements types
 * that the achievement system recognizes them as (EGearLevelAchievements -> EGearAchievement)
*/
var array<EGearAchievement> AchievementMap;

/** The achievement you'd like to unlock */
var() EGearLevelAchievements Achievement;

defaultproperties
{
	ObjName="Achievement Unlock"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Unlock")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets,bHidden=TRUE)

	AchievementMap(eGLACHIEVE_None)=eGA_Invalid
	AchievementMap(eGLACHIEVE_GreenAsGrass)=eGA_GreenAsGrass
	AchievementMap(eGLACHIEVE_ItsATrap)=eGA_ItsATrap
	AchievementMap(eGLACHIEVE_EscortService)=eGA_EscortService
	AchievementMap(eGLACHIEVE_GirlAboutTown)=eGA_GirlAboutTown
	AchievementMap(eGLACHIEVE_ThatSinkingFeeling)=eGA_ThatSinkingFeeling
	AchievementMap(eGLACHIEVE_Freebaird)=eGA_Freebaird
	AchievementMap(eGLACHIEVE_HeartBroken)=eGA_HeartBroken
	AchievementMap(eGLACHIEVE_LongitudeAndAttitude)=eGA_LongitudeAndAttitude
	AchievementMap(eGLACHIEVE_TanksForTheMemories)=eGA_TanksForTheMemories
	AchievementMap(eGLACHIEVE_WaterSports)=eGA_WaterSports
	AchievementMap(eGLACHIEVE_SavedMaria)=eGA_SavedMaria
	AchievementMap(eGLACHIEVE_WrappedInBeacon)=eGA_WrappedInBeacon
	AchievementMap(eGLACHIEVE_HaveFunStormingTheCastle)=eGA_HaveFunStormingTheCastle
	AchievementMap(eGLACHIEVE_AndTheHorseYourRodeInOn)=eGA_AndTheHorseYourRodeInOn
	AchievementMap(eGLACHIEVE_TheyJustKeepComing)=eGA_TheyJustKeepComing
	AchievementMap(eGLACHIEVE_BrumakRodeo)=eGA_BrumakRodeo
	AchievementMap(eGLACHIEVE_DoesThisLookInfectedToYou)=eGA_DoesThisLookInfectedToYou
	AchievementMap(eGLACHIEVE_DLC1)=eGA_DLC1
	AchievementMap(eGLACHIEVE_DLC2)=eGA_DLC2
	AchievementMap(eGLACHIEVE_DLC3)=eGA_DLC3
	AchievementMap(eGLACHIEVE_DLC4)=eGA_DLC4
	AchievementMap(eGLACHIEVE_DLC5)=eGA_DLC5
	AchievementMap(eGLACHIEVE_DLC6)=eGA_DLC6
	AchievementMap(eGLACHIEVE_DLC7)=eGA_DLC7
	AchievementMap(eGLACHIEVE_DLC8)=eGA_DLC8
	AchievementMap(eGLACHIEVE_DLC9)=eGA_DLC9
	AchievementMap(eGLACHIEVE_DLC10)=eGA_DLC10
	AchievementMap(eGLACHIEVE_Seriously)=eGA_Seriously2
}
