/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
* navigation point used along with a gameplay route that allows extra meta data 
*/
class GameplayRouteNode extends PathNode
	placeable;

/** if this is on combatzones from this gameplay route node will be assigned to AIs when they use get to this squad destination */
var() bool bAssignCombatZones;
/** list of combat zones to be assigned to AIs when their squad is at this location */
var() array<ActorReference> CombatZones;
/** if this is on the links in restricttocoverlink will be copied to AIs assigned to this node (even if this node's list is blank)*/
var() bool bAssignResictedCoverLinks;
/** list of cover links to restrict AIs to when they're at this gameplayroute node, if none AI will use any cover within their assigned combat zones */
var() array<ActorReference> RestrictToCoverLinks;


function OnAssignedToThisNode(GearAI AI)
{
	local int i;
	local GearAI_Cover CovAI;
	if(bAssignCombatZones)
	{
		AI.ClearCombatZones();
		for(i=0;i<CombatZones.length;i++)
		{
			AI.CombatZoneList[AI.CombatZoneList.length] = CombatZone(CombatZones[i].Actor);
		}
	}

	if(bAssignResictedCoverLinks)
	{
		CovAI = GearAI_Cover(AI);
		if(CovAI != none)
		{
			CovAI.AllowedCoverLinks.length=0;
			for(i=0;i<RestrictToCoverLinks.length;i++)
			{
				CovAI.AllowedCoverLinks[i] = CoverLink(RestrictToCoverLinks[i].Actor);
			}
		}

	}
}

function OnUnAssignedFromThisNode(GearAI AI)
{
	// pick out cover links and combat zones we assigned when they used to be at this node
	local GearAI_Cover CovAI;
	if(bAssignCombatZones)
	{
		`AILog_Ext("Clearing combat zones because dude is leaving GameplayRouteNode"@self,,AI);
		AI.ClearCombatZones();
	}

	if(bAssignResictedCoverLinks)
	{
		CovAI = GearAI_Cover(AI);
		if(CovAI != none)
		{
			`AILog_Ext("Clearing allowed cover links because dude is leaving GameplayRouteNode"@self,,AI);
			CovAI.AllowedCoverLinks.length=0;			
		}
	}
}

DefaultProperties
{
	bAssignCombatZones=true
	bAssignResictedCoverLinks=true


	Begin Object NAME=Sprite
		Sprite=Texture2D'GearsIcons.GameplayRouteNode'
		Scale=0.25f
	End Object
}