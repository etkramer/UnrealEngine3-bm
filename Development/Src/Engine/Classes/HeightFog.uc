/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class HeightFog extends Info
	showcategories(Movement)
	placeable;

var() const editconst HeightFogComponent	Component;

/** replicated copy of HeightFogComponent's bEnabled property */
var repnotify bool bEnabled;

replication
{
	if (Role == ROLE_Authority)
		bEnabled;
}

event PostBeginPlay()
{
	Super.PostBeginPlay();

	bEnabled = Component.bEnabled;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'bEnabled')
	{
		Component.SetEnabled(bEnabled);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/* epic ===============================================
* ::OnToggle
*
* Scripted support for toggling height fog, checks which
* operation to perform by looking at the action input.
*
* Input 1: turn on
* Input 2: turn off
* Input 3: toggle
*
* =====================================================
*/
simulated function OnToggle(SeqAct_Toggle action)
{
	if (action.InputLinks[0].bHasImpulse)
	{
		// turn on
		Component.SetEnabled(TRUE);
	}
	else if (action.InputLinks[1].bHasImpulse)
	{
		// turn off
		Component.SetEnabled(FALSE);
	}
	else if (action.InputLinks[2].bHasImpulse)
	{
		// toggle
		Component.SetEnabled(!Component.bEnabled);
	}
	bEnabled = Component.bEnabled;
	ForceNetRelevant();
	SetForcedInitialReplicatedProperty(Property'Engine.HeightFog.bEnabled', (bEnabled == default.bEnabled));
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	Begin Object Class=HeightFogComponent Name=HeightFogComponent0
	End Object
	Component=HeightFogComponent0
	Components.Add(HeightFogComponent0)

	bStatic=FALSE
	bNoDelete=true
}
