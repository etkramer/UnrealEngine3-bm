/**
 * Class to handle the AI controlling of the Reaver
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Reaver extends GearAI
	config(AI);

/** GoW global macros */

var() Vehicle_Reaver_Base Reaver;

function Possess( Pawn NewPawn, bool bVehicleTransition )
{
	super.Possess( NewPawn, bVehicleTransition );

	Reaver = Vehicle_Reaver_Base(Pawn);

	bForceDesiredRotation = TRUE;
	DesiredRotation = Reaver.Rotation;
}

function CleanupBoringDude()
{
	local GearAI GAI;
	GAI = GearAI(Reaver.Driver.Controller);

	if(GAI != none && GAI != self)
	{
		GAI.CleanupBoringDude();
	}

	GAI = GearAI(Reaver.Gunner.Controller);
	if(GAI != none && GAI != self)
	{
		GAI.CleanupBoringDude();
	}
	
	Pawn.Destroy();
	Destroy();
}

function SetEnableDeleteWhenStale(bool bAmIExpendable)
{
	local GearAI GAI;

	Super.SetEnableDeleteWhenStale(bAmIExpendable);

	GAI = GearAI(Reaver.Driver.Controller);
	if(GAI != none && GAI != self)
	{
		GAI.SetEnableDeleteWhenStale(bAmIExpendable);
	}

	GAI = GearAI(Reaver.Gunner.Controller);
	if(GAI != none && GAI != self)
	{
		GAI.SetEnableDeleteWhenStale(bAmIExpendable);
	}
}

function NotifyKilled( Controller Killer, Controller Killed, Pawn KilledPawn )
{
	if( KilledPawn == Reaver.Driver )
	{
		NotifyDriverDied( KilledPawn, Killer );
		KilledPawn.LifeSpan=3.0f;
	}
	else if( KilledPawn == Reaver.Gunner )
	{
		NotifyGunnerDied( KilledPawn, Killer );
		KilledPawn.LifeSpan=3.0f;
	}
	else
	{
		super.NotifyKilled( Killer, Killed, KilledPawn );
	}
}

function NotifyDriverDied( Pawn DeadPawn, Controller Killer )
{
	//debug
	`AILog( GetFuncName()@DeadPawn@Killer );
}

function NotifyGunnerDied( Pawn DeadPawn, Controller Killer )
{
	//debug
	`AILog( GetFuncName()@DeadPawn@Killer );
}

function OnAIToggleCombat(SeqAct_AIToggleCombat Action)
{
	// pass the call along to our driver, and gunner
	if(Reaver != none && Reaver.Gunner != none)
	{
		Reaver.Gunner.MyGearAI.OnAIToggleCombat(Action);
	}

	if(Reaver != none && Reaver.Driver != none)
	{
		GearPawn(Reaver.Driver).MyGearAI.OnAIToggleCombat(Action);
	}

	Super.OnAIToggleCombat(Action);
}

function SeePlayer( Pawn Seen )
{
	if(Reaver != None)
	{
		Reaver.ReaverSeePlayer(Seen);
	}
}

function AdjustEnemyRating(out float out_Rating, Pawn EnemyPawn)
{
	Super.AdjustEnemyRating(out_Rating, EnemyPawn);

	// landed reaver prioritizes humans
	if (out_Rating > 0.0 && Pawn.Physics == PHYS_RigidBody && EnemyPawn.IsHumanControlled())
	{
		out_Rating += 1.0;
	}
}

/** Don't let reavers move outside combat zones */
function bool IsValidDirectMoveGoal( Actor A )
{
	if( !IsWithinCombatZone( A.Location ) )
	{
		return FALSE;
	}
	return Super.IsValidDirectMoveGoal( A );
}

event Actor GeneratePathTo( Actor Goal, optional float Distance, optional bool bAllowPartialPath )
{
	class'Path_TowardGoal'.static.TowardGoal( Pawn, Goal );
	class'Path_AvoidanceVolumes'.static.AvoidThoseVolumes( Pawn );
	class'Path_WithinCombatZone'.static.WithinCombatZone( Pawn, 10000000 );

	class'Goal_AtActor'.static.AtActor( Pawn, Goal, Distance, bAllowPartialPath );


	return FindPathToward( Goal );
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Reaver'

	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty
	// have to re-init the delegates here so they get the right function object
	// melee reaction
	Begin Object Name=EnemyInMeleeRangeReaction0
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction0)

	bCanRevive=FALSE
}
