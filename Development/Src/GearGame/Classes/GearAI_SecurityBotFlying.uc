/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearAI_SecurityBotFlying extends GearAI;

var config float CirclingRadius;
var config float CirclingPeriod;
var() config float EnemyAcquiredGracePeriod;

var instanced AIReactCond_GenericCallDelegate LostEnemyReaction;

function DoMeleeAttack( optional Pawn NewEnemy );


function OnLostEnemy(Actor InInstigator, AIReactChannel OrigChannel)
{
	GearPawn_SecurityBotFlyingBase(MyGearPawn).OnLostEnemy();
	BeginCombatCommand(none,"Lost LOS, restarting");
}

function OnNewEnemy()
{
	`AILog(GetFuncName());
	GearPawn_SecurityBotFlyingBase(MyGearPawn).OnAcquireEnemy(GearPawn(Enemy));
}

DefaultProperties
{
	DefaultCommand=class'AICmd_Base_SecurityBotFlying'

	DefaultReactConditions.Empty()
	DefaultReactConditionClasses.Empty()

	Begin Object class=AIReactCond_GenericCallDelegate Name=Pusher0
		AutoSubscribeChannels(0)=LostEnemyVisibility
		OutputFunction=OnLostEnemy
	End Object
	DefaultReactConditions.Add(Pusher0)
	LostEnemyReaction=Pusher0
}