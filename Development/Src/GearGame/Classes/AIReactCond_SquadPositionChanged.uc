/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class AIReactCond_SquadPositionChanged extends AIReactCondition_base
	within GearAI;

var Actor	CurrentSquadPosition;


static function AIReactCond_SquadPositionChanged AbortMoveWhenSquadPositionChanges(GearAI AI, Actor InCurrentSquadPosition)
{
	local AIReactCond_SquadPositionChanged Reaction;

	Reaction = new(AI) class'AIReactCond_SquadPositionChanged';
	if(Reaction != none)
	{
		Reaction.CurrentSquadPosition = InCurrentSquadPosition;
		`AILog_Ext("Pushed "@Reaction@".. will abort squad move when position changes from "@InCurrentSquadPosition,,AI);
		Reaction.Initialize();
	}
	else
	{
		`warn(GetFuncName()@"Couldn't spawn reaction!");
	}

	return Reaction;
}



event bool ShouldActivate( Actor EventInstigator, AIReactChannel OrigChan )
{
	local Actor NewSquadPosition;

	if (!Super.ShouldActivate(EventInstigator,OrigChan) || (MyGearPawn != None && MyGearPawn.IsDoingSpecialMove(SM_MidLvlJumpOver)))
	{
		return false;
	}

	NewSquadPosition = GetSquadPosition();
	if( bMovingToSquadLeader && NewSquadPosition != CurrentSquadPosition &&
		NewSquadPosition != none)
	{
		`AILog(self@"Old squad position"@CurrentSquadPosition@"new"@NewSquadPosition);
		return true;
	}


	return false;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	Super.Activate(EventInstigator,OriginatingChannel);
	`AILog(self@"New squad position! Aborting current moveto squad leader move...");
	AbortCommand(none,class'AICmd_MoveToSquadLeader');
	class'AICmd_MoveToSquadLeader'.static.MoveToSquadLeader(outer);
}


defaultproperties
{
	AutoSubscribeChannels(0)=Timer
	bOneTimeOnly=true
	TimerInterval=0.5f
	bAlwaysNotify=true
}
