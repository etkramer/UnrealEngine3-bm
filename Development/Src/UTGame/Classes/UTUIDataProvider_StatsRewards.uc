/**
 * Dataprovider that returns a row for each reward stat for a user.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_StatsRewards extends UTUIDataProvider_StatsGeneral
	native(UI);

`include(UTStats.uci)

/** @return Returns the number of elements(rows) provided. */
native function int GetElementCount();

DefaultProperties
{
	Stats.Empty();

	Stats.Add((StatName="SPREE_KILLINGSPREE"))
	Stats.Add((StatName="SPREE_RAMPAGE"))
	Stats.Add((StatName="SPREE_DOMINATING"))
	Stats.Add((StatName="SPREE_UNSTOPPABLE"))
	Stats.Add((StatName="SPREE_GODLIKE"))
	Stats.Add((StatName="SPREE_MASSACRE"))

	Stats.Add((StatName="MULTIKILL_DOUBLEKILL"))
	Stats.Add((StatName="MULTIKILL_MULTIKILL"))
	Stats.Add((StatName="MULTIKILL_MEGAKILL"))
	Stats.Add((StatName="MULTIKILL_ULTRAKILL"))
	Stats.Add((StatName="MULTIKILL_MONSTERKILL"))

	Stats.Add((StatName="EVENT_FIRSTBLOOD"))
	Stats.Add((StatName="EVENT_ENDSPREE"))
	Stats.Add((StatName="EVENT_EAGLEEYE"))
	Stats.Add((StatName="EVENT_BULLSEYE"))
	Stats.Add((StatName="EVENT_TOPGUN"))
	Stats.Add((StatName="EVENT_DENIEDREDEEMER"))
	
	Stats.Add((StatName="REWARD_BIGGAMEHUNTER"))
	Stats.Add((StatName="REWARD_BIOHAZARD"))
	Stats.Add((StatName="REWARD_BLUESTREAK"))
	Stats.Add((StatName="REWARD_COMBOKING"))
	Stats.Add((StatName="REWARD_FLAKMASTER"))
	Stats.Add((StatName="REWARD_GUNSLINGER"))
	Stats.Add((StatName="REWARD_HEADHUNTER"))
	Stats.Add((StatName="REWARD_JACKHAMMER"))
	Stats.Add((StatName="REWARD_ROCKETSCIENTIST"))
	Stats.Add((StatName="REWARD_ROADRAMPAGE"))
	Stats.Add((StatName="REWARD_SHAFTMASTER"))
}
