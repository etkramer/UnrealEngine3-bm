// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
class UTMutator_Hero extends UTMutator;

var string HeroPawnClassNameStr;
var bool bNeedToSetHeroMode;

function InitMutator(string Options, out string ErrorMessage)
{
	local class<UTPawn> HeroPawnClass;

	if ( UTGame(WorldInfo.Game) != None )
	{
		HeroPawnClass = class<UTPawn>(DynamicLoadObject(HeroPawnClassNameStr, class'Class'));
		if (HeroPawnClass != None)
		{
			UTGame(WorldInfo.Game).DefaultPawnClass = HeroPawnClass;
		}
	}

	Super.InitMutator(Options, ErrorMessage);
}

function bool CheckReplacement(Actor Other)
{
	local UTGameReplicationInfo UTGRI;

	if ( bNeedToSetHeroMode )
	{
		//Turn off heroes
		UTGRI = UTGameReplicationInfo(Other);
		if (UTGRI != None)
		{
			UTGRI.bHeroesAllowed = true;
			bNeedToSetHeroMode = false;
		}
	}

	return true;
}

defaultproperties
{
	HeroPawnClassNameStr="WrathGameContent.UTHeroPawn"
	bNeedToSetHeroMode=true
}
