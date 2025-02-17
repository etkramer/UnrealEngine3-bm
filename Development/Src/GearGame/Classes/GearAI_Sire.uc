/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearAI_Sire extends GearAI_Cover
	config(AI);

function BeginMeleeCommandWrapper( Actor Inst, AIReactChannel OriginatingChannel )
{
	GearPawn_LocustSireBase(MyGearPawn).StartAttackSpeed = VSize2D(MyGearPawn.Velocity);
	Super.BeginMeleeCommandWrapper(Inst,OriginatingChannel);
}

event Possess(Pawn NewPawn, bool bVehicleTransition)
{
	Super.Possess(NewPawn,bVehicleTransition);
	PlayIntialCombatReactionChance=0.f;
}

function NotifyKnockDownStart()
{
	Super.NotifyKnockDownStart();
	AutoAcquireEnemy();
	SelectEnemy();
}

function DoMeleeAttack( optional Pawn NewEnemy )
{
	if( NewEnemy != None )
	{
		SetEnemy( NewEnemy );
	}

	if(Enemy != none)
	{
		class'AICmd_Attack_SireMelee'.static.InitCommand( self );
	}
}

function bool CanExecutePawn( GearPawn GP )
{
	if ( bCanExecute && MyGearPawn != None &&
		GP.IsDBNO() &&
		WorldInfo.TimeSeconds - GP.TimeStampEnteredRevivingState >= ExecuteDelay &&
		GP != Pawn &&
		!Pawn.IsSameTeam( GP ) &&
		!MyGearPawn.IsDBNO() &&
		!GP.IsInPainVolume() &&
		!IsDead() &&
		!GP.IsAHostage() &&
		GP.CanBeSpecialMeleeAttacked(MyGearPawn) &&
		!MyGearPawn.IsAKidnapper() &&
		( WorldInfo.GRI.IsMultiplayerGame() || GP.GetTeamNum() != TEAM_COG ||
		(GP.IsHumanControlled() && GearPRI(GP.PlayerReplicationInfo).Difficulty.default.bHumanCOGCanBeExecuted) )  &&
		IsEnemyWithinCombatZone(GP))
	{
		return TRUE;
	}
	return FALSE;
}

function HandleExecuteReactionAndPushExecuteCommand(Actor Inst, AIReactChannel OrigChan)
{
	class'AICmd_Melee_Forced'.static.ForceMelee(self,Inst);
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Sire'

	DefaultReactConditions.Empty()
	DefaultReactConditionClasses.Empty()
	// melee reaction
	Begin Object Name=EnemyInMeleeRangeReaction0
		AutoSubscribeChannels(0)=EnemyWithinMeleeDistance
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction0)

	// execute enemy
	Begin Object Name=ExecutEnemyReaction0
		AutoSubscribeChannels(0)=HaveEnemyToExecute
		OutputFunction=HandleExecuteReactionAndPushExecuteCommand
	End Object
	DefaultReactConditions.Add(ExecutEnemyReaction0)

	bCanRevive=false
}
