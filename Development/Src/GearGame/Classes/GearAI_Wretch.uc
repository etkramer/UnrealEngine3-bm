/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Wretch extends GearAI_Cover
	native(AI)
	config(AI);

/** GoW global macros */

cpptext
{
	virtual void UpdatePawnRotation();
}

function StopFiring();
function DoCoverHead();

/**
 * Overridden to enforce melee weapon for the wretch.
 */
function NotifyChangedWeapon( Weapon PrevWeapon, Weapon NewWeapon )
{
	if (NewWeapon != None && GearWeap_WretchMelee(NewWeapon) == None)
	{
		`AILog("Fixing switch to non-melee weapon:"@NewWeapon);
		Pawn.InvManager.SetCurrentWeapon(PrevWeapon);
	}
	else
	{
		Super.NotifyChangedWeapon(PrevWeapon,NewWeapon);
	}
}

function bool AllowWeaponInInventory(class<GearWeapon> NewWeaponClass)
{
	// prevent non-melee weapons for the wretch
	if (!ClassIsChildOf(NewWeaponClass,class'GearWeap_WretchMelee'))
	{
		return FALSE;
	}
	return TRUE;
}

function FireFromOpen(optional int InBursts = 1, optional bool bAllowFireWhileMoving=true)
{
}
function FireFromCover()
{
}

function DoMeleeAttack( optional Pawn NewEnemy )
{
	if( NewEnemy != None )
	{
		SetEnemy( NewEnemy );
	}

	class'AICmd_Attack_Melee_Wretch'.static.Melee( self );
}

function NotifyCeilingTransitionFall()
{
	local Vector		JumpVel;

	//debug
	`AILog( GetFuncName()@MyGearPawn.SpecialMove@Pawn.Physics@Pawn.Velocity );

	if( MyGearPawn.IsDoingSpecialMove(GSM_LeapToCeiling) )
	{
		// Get velocity of the jump
		Pawn.SuggestJumpVelocity( JumpVel, CurrentPath.End.Actor.Location, Pawn.Location );
		// Set velocity of pawn
		Pawn.ZeroMovementVariables();

		Pawn.Velocity.Z = JumpVel.Z;
		// Change physics to falling
		Pawn.SetPhysics( PHYS_Falling );

		//debug
		`AILog( JumpVel@Pawn.Velocity@Pawn.Physics );
	}
	else
	if( MyGearPawn.IsDoingSpecialMove(GSM_DropFromCeiling) )
	{
		// Just set physics to falling
		Pawn.SetPhysics( PHYS_Falling );
		Pawn.Velocity = vect(0,0,-450);
	}
}

defaultproperties
{
	bIgnoreFireLinks=TRUE
	bCanExecute=false
	bCanRevive=false

	MinHitWall=1.f
	bNotifyFallingHitWall=true
	RotationRate=(Pitch=60000,Yaw=60000,Roll=2048)
	CombatMood=AICM_Aggressive

//	bTeleporter=TRUE

	DefaultCommand=class'AICmd_Base_Wretch'

	//ignore reactions
	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty
	// have to re-init the delegates here so they get the right function object
	// melee reaction
	Begin Object Name=EnemyInMeleeRangeReaction0
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction0)

	// execute enemy
	Begin Object Name=ExecutEnemyReaction0
		OutputFunction=HandleExecuteReactionAndPushExecuteCommand
	End Object
	DefaultReactConditions.Add(ExecutEnemyReaction0)

	// add back leave cover from damage
	DefaultReactConditionClasses.Add(class'AIReactCond_DmgLeaveCover')
	// broadcast when our cover is exposed to an enemy
	DefaultReactConditionClasses.Add(class'AIReactCond_CoverExposed')

	// remove enemy proximity and weapon range checks from cover evaluation
	Begin Object Name=AtCov_Towards0
		CoverGoalConstraints.Remove(CovGoal_EnemyProx0)
		CoverGoalConstraints.Remove(CovGoal_WeaponRange0)
	End Object
	Begin Object name=AtCov_Away0
		CoverGoalConstraints.Remove(CovGoal_EnemyProx0)
		CoverGoalConstraints.Remove(CovGoal_WeaponRange0)
	End Object
	Begin Object name=AtCov_Near0
		CoverGoalConstraints.Remove(CovGoal_EnemyProx0)
		CoverGoalConstraints.Remove(CovGoal_WeaponRange0)
	End Object

	bUseFireTickets=FALSE

	// don't move in squads I say!
	bIgnoreSquadPosition=TRUE
}
