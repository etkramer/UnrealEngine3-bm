/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AICmd_VacateAvoidanceVolume extends AICommand
	within GearAI;

var actor AvoidanceVolumeOwner;

var bool bShouldEvade;
var bool bRoadieRun;

static function bool VacateVolume( GearAI AI, Actor UserActor, bool bInShouldEvade, bool bInRoadieRun )
{
	local AICmd_VacateAvoidanceVolume Cmd;

	if( AI != None )
	{
		Cmd = new(AI) Default.Class;
		if( Cmd != None )
		{
			Cmd.AvoidanceVolumeOwner = UserActor;
			Cmd.bShouldEvade = bInShouldEvade;
			Cmd.bRoadieRun = bInRoadieRun;
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}
/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Avoiding:"@AvoidanceVolumeOwner;
}

function Pushed()
{
	//messageplayer(GetFuncName()@AvoidanceVolumeOwner@bShouldEvade@bRoadieRun);
	`AILog(GetFuncName()@AvoidanceVolumeOwner@AvoidanceVolumeOwner.Owner@AIAvoidanceCylinder(AvoidanceVolumeOwner).OwnerName@bShouldEvade@bRoadieRun);

	ReactionManager.SuppressChannel('EnteredAvoidanceZone');
	ReactionManager.SuppressChannel('EnteredEvadeAvoidanceZone');
	ReactionManager.SuppressChannel('EnteredRoadieRunAvoidanceZone');

	Super.Pushed();

	if (CombatMood != AICM_Aggressive)
	{
		SetCombatMood(AICM_Aggressive);
	}

	bShouldRoadieRun = bRoadieRun;
	MyGearPawn.bIsWalking = FALSE;
	GotoState('GetOutTheWay');
}

function Popped()
{
	bShouldRoadieRun = FALSE;
	Outer.NotifyNeedRepath();
	Super.Popped();
	ReactionManager.UnSuppressChannel('EnteredAvoidanceZone');
	ReactionManager.UnSuppressChannel('EnteredEvadeAvoidanceZone');
	ReactionManager.UnSuppressChannel('EnteredRoadieRunAvoidanceZone');
	
}

state GetOutTheWay
{
Begin:

	if(CoverOwner!=none)
	{
		CoverOwner.ResetCoverType();
	}
	// evade first if we have been instructed to
	if(bShouldEvade)
	{
		`AILog("Trying to evade!");
		EvadeAwayFromPoint(AvoidanceVolumeOwner.Location);
	}
	// find a spot outside avoidance volumes
	Pawn.ClearConstraints();
	class'Path_AvoidanceVolumes'.static.AvoidThoseVolumes(Pawn);
	class'Goal_OutsideAvoidanceVolumes'.static.OutsideAvoidanceVolumes(Pawn);
	if(FindPathToward(Pawn) != none)
	{
		class'AICmd_MoveToGoal'.static.MoveToGoal(outer,RouteCache[RouteCache.length-1],,,TRUE,FALSE);
	}
	else
	{
		GotoState('DelayFailure');
	}

	GotoState('DelaySuccess');
}

defaultproperties
{
}
