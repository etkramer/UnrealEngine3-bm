/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_NemaSlug extends GearAI
	config(AI);

/** GoW global macros */

var float MeleeRadius;

function bool ShouldMoveToSquad( optional bool bSkipLastPositionCheck )
{
	return FALSE;
}

singular function bool StepAsideFor( Pawn ChkPawn );


function DoMeleeAttack( optional Pawn NewEnemy )
{
	`AIlog(GetFuncName());
	if (CommandList == None || CommandList.IsAllowedToFireWeapon())
	{
		if (NewEnemy != None)
		{
			SetEnemy(NewEnemy);
		}

		class'AICmd_Attack_NemaSlugMelee'.static.InitCommand(self);
	}
}

function BeginMeleeCommandWrapper( Actor Inst, AIReactChannel OriginatingChannel )
{
	`AILog(GetFuncName());
	DoMeleeAttack(Pawn(Inst));
}
function bool NotifyBump( Actor Other, vector HitNormal );

defaultproperties
{
	DefaultCommand=class'AICmd_Base_NemaSlug'
	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty

	// melee reaction
	Begin Object Name=EnemyInMeleeRangeReaction0
		AutoSubscribeChannels(0)=EnemyWithinMeleeDistance
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction0)

	bCanRevive=false
	bCanExecute=false
}
