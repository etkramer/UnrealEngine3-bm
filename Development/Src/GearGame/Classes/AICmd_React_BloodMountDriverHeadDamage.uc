/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * - special move for the blood mount driver, when driver's mount takes head damage
 */
class AICmd_React_BloodMountDriverHeadDamage extends AICommand_SpecialMove;

var AIReactCond_DmgThreshold DmgThreshReaction;
var GearPawn_LocustBloodmount BloodMount;
var() int DamageThresholdForKnockoff;
/** Simple constructor that pushes a new instance of the command for the AI */
static function bool React( GearAI AI, GearPawn_LocustBloodmount BM )
{
	local AICmd_React_BloodMountDriverHeadDamage Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_React_BloodMountDriverHeadDamage';
	if( Cmd != None )
	{
	    Cmd.BloodMount=BM;
			AI.PushCommand(Cmd);
	    return TRUE;
	}
	}

	return FALSE;
}

function Pushed()
{
	Super.Pushed();

	// setup a new reaction to wait for 100 damage incoming
	DmgThreshReaction = new(outer) class'AIReactCond_DmgThreshold';
	DmgThreshReaction.OutputFunction = HitDamageThresh;
	DmgThreshReaction.DamageThreshold = 100;
	DmgThreshReaction.Initialize();
	// make sure the damage channel is unsuppressed
	ReactionManager.UnsuppressChannel('Damage');

	//MessagePlayer(Pawn@"Playing OMG my mount got shot in the face animation! (DmgReaction:"@DmgThreshReaction@")");

	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	Super.Popped();
	// remove damage threshold reaction
	//MessagePlayer("Unsubscribing damage reaction:"@DmgThreshReaction);
	DmgThreshReaction.UnsubscribeAll();
	DmgThreshReaction = none;

	ReactionManager.SuppressChannel('Damage');
}

function HitDamageThresh(Actor EventInstigator, AIReactChannel OrigChannel)
{
	`AILog("Just took "$DamageThresholdForKnockoff$" damage during mount thrash around.. ejecting myself from the bloodmount");
	bloodmount.EjectDriver();
	AbortCommand(self);
	CheckCombatTransition();
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );
	}

	function ESpecialMove GetSpecialMove()
	{
		return SM_BloodMountDriver_CalmMount;
	}
}

defaultproperties
{
}
