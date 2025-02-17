/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_React_KnockDown extends AICommand;

/** GoW global macros */


function Pushed()
{
	Super.Pushed();

	ReactionManager.SuppressAll();
	GotoState( 'WaitForRecovery' );	
}

function Popped()
{
	local float Dist;
	local NavigationPoint FoundAnchor;
	Super.Popped();
	ReactionManager.UnSuppressAll();

	// check to see if we have a valid anchor after recovery.. if not kill ourselllllllllllves!
	if(!Pawn.ValidAnchor())
	{
		FoundAnchor = Pawn.GetBestAnchor(Pawn,Pawn.Location,FALSE,FALSE,Dist);
		Pawn.SetAnchor(FoundAnchor);

		if(FoundAnchor == none)
		{
			`AILog("Couldn't find an anchor after ragdoll!  CUTTING MY OWN THROAT... OHHH WHAT A WORLD");
			Pawn.Died(outer,class'GDT_ScriptedRagdoll',Pawn.Location);
		}
	}
}

function bool AllowTransitionTo( class<AICommand> AttemptCommand )
{
	return false;
}

state WaitForRecovery
{
Begin:
	// wait for the ragdoll to settle
	do
	{
		Sleep(0.1f);
		
	} until (!Pawn.IsInState('KnockedDown'));

	`AILog("Done recovering.. going back to normal");
	PopCommand(Self);
}
	
defaultproperties
{
	bIgnoreNotifies=true
}