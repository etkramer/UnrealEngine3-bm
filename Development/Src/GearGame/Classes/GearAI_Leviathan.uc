/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Leviathan extends GearAI;

var SeqAct_Leviathan_Mouth  CurrentMouthSeq;
var SeqAct_Leviathan_Jaw	CurrentJawSeq;

var array<Actor>	ValidTargets;
var array<Actor>	ValidMouthTargets;

function bool ShouldSaveForCheckpoint()
{
	return false;
}

function OnLeviathan_Mouth( SeqAct_Leviathan_Mouth inAction )
{
	//debug
	`AILog( GetFuncName()@inAction.InputLinks[0].bHasImpulse@inAction.InputLinks[1].bHasImpulse  );

	if( inAction.InputLinks[0].bHasImpulse )
	{
		CurrentMouthSeq = inAction;

		// Mouth battle
		BeginCombatCommand( class'AICmd_Attack_Leviathan_Mouth', 'OnLeviathan_Mouth' );
	}
	else
	if( inAction.InputLinks[1].bHasImpulse )
	{
		// Cancel battle
		BeginCombatCommand( None, "Mouth battle aborted", TRUE );
	}
}

function OnLeviathan_Jaw( SeqAct_Leviathan_Jaw inAction )
{
	local int Idx;

	//debug
	`AILog( GetFuncName() );

	CurrentJawSeq = inAction;
	for( Idx = 0; Idx < inAction.Victims.Length; Idx++ )
	{
		if( inAction.Victims[Idx] != None )
		{
			ValidTargets[ValidTargets.Length] = GetFinalTarget(inAction.Victims[Idx]);
		}
	}

	BeginCombatCommand( class'AICmd_Attack_Leviathan_Jaw', 'OnLeviathan_Jaw' );
}

function OnLeviathan_Eyes( SeqAct_Leviathan_Eyes inAction )
{
	//debug
	`AILog( GetFuncName() );

	GearPawn_LocustLeviathanBase(Pawn).OpenEyes( inAction.Duration );
}

function OnLeviathan_EmergeFromWater( SeqAct_Leviathan_EmergeFromWater inAction )
{
	//debug
	`AILog( GetFuncName() );

	GearPawn_LocustLeviathanBase(Pawn).EmergeFromWater();
}

function Actor GetMouthTarget()
{
	if( ValidMouthTargets.Length == 0 )
	{
		return None;
	}
	return ValidMouthTargets[Rand(ValidMouthTargets.Length)];
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Leviathan'

	DefaultReactConditionClasses.Empty
	DefaultReactConditions.Empty
}
