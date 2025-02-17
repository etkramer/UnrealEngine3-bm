/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
/** Meatshield reaction condition - when we notice a new meatshield situation, trigger a reaction **/
class AIReactCond_MeatShield extends AIReactCondition_Base;

/** GoW global macros */

/** keep an array of known meatshield kidnappers and their hostages so we can tell when a new situation arises **/
struct native MeatShieldEntry
{
	var GearPawn Kidnapper;
	var GearPawn Hostage;
};
var array<MeatShieldEntry> MeatShield_Reaction_Hostages;

var GearPawn LastInstigator;
event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan)
{
	LastInstigator = GearPawn(EventInstigator);
	// if this is a new meat shield, then kick on the reaction
	if(Super.ShouldActivate(EventInstigator,OrigChan) && IsNewMeatShield(LastInstigator))
	{
		return true;
	}
	return false;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel)
{
	Super.Activate(EventInstigator,OriginatingChannel);
	NotifyEnemyUsingMeatShield(LastInstigator);
}

/**
 * IsNewMeatShield
 * Checks to see if we should meatshield react to the pawn passed in
 */
function bool IsNewMeatShield(GearPawn GP)
{
	local int Idx;
	local GearPawn CurHostage;
	local GearPawn CurKidnapper;
	local bool bNewHostageSituation;
	local bool bNeedsAdd;
	// early out if something isn't right
	if(GP == none || !GP.IsAKidnapper())
		return false;

	// if we are the hostage, don't do anything
	if(GP.InteractionPawn == Pawn)
	{
		return false;
	}

	bNewHostageSituation = true;
	bNeedsAdd = true;
	for(Idx=MeatShield_Reaction_Hostages.Length-1; Idx >= 0; Idx--)
	{
		CurHostage = MeatShield_Reaction_Hostages[Idx].Hostage;
		CurKidnapper = MeatShield_Reaction_Hostages[Idx].Kidnapper;

		// if either hostage or kidnapper are bogus remove the entry from the list
		if(CurKidnapper == none || CurKidnapper.bDeleteMe)
		{

				MeatShield_Reaction_Hostages.Remove(Idx,1);
				continue;
		}

		// if we already have an entry for this kidnapper
		if(CurKidnapper == GP)
		{// then make sure it's still valid
			bNewHostageSituation = false;
			bNeedsAdd = false;

			if (CurHostage != GP.InteractionPawn)
			{// if it's a different hostage, save the change and react
				bNewHostageSituation = true;
				MeatShield_Reaction_Hostages[Idx].Hostage = GP.InteractionPawn;
			}
			break;
		}
		else
		if(CurHostage == none || CurHostage.bDeleteMe)
		{
			MeatShield_Reaction_Hostages.Remove(Idx,1);
		}
	}

	if(bNeedsAdd)
	{
		`AILog(GetFuncName()@"Adding a new meatshield record for kidnapper("$GP$").");
		MeatShield_Reaction_Hostages.Add(1);
		MeatShield_Reaction_Hostages[MeatShield_Reaction_Hostages.Length-1].Hostage = GP.InteractionPawn;
		MeatShield_Reaction_Hostages[MeatShield_Reaction_Hostages.Length-1].Kidnapper = GP;
	}

	if(bNewHostageSituation)
	{
		`AILog(GetFuncName()@"Reacting to new kidnapper ("$GP$").");
		return true;
	}

	return false;
}


defaultproperties
{
	AutoSubscribeChannels(0)=SightPlayer
}
