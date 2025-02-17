/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class WalkVolume extends PhysicsVolume
	native;

var() bool bActive;

replication
{
	if (bNetDirty)
		bActive;
}

simulated function OnToggle(SeqAct_Toggle Action)
{
	local GearPawn Pawn;
	if (Action.InputLinks[0].bHasImpulse)
	{
		bActive = TRUE;
	}
	else
	if (Action.InputLinks[1].bHasImpulse)
	{
		bActive = FALSE;
	}
	else
	if (Action.InputLinks[2].bHasImpulse)
	{
		bActive = !bActive;
	}
	if (bActive)
	{
		// abort any active roadie runs
		foreach WorldInfo.AllPawns(class'GearPawn',Pawn)
		{
			if (Pawn.PhysicsVolume == self &&
				Pawn.IsDoingSpecialMove(SM_RoadieRun))
			{
				Pawn.EndSpecialMove(SM_RoadieRun);
			}
		}
	}
	bForceNetUpdate = TRUE;
}

defaultproperties
{
	bActive=TRUE
	RemoteRole=ROLE_SimulatedProxy
	NetUpdateFrequency=1.0
	bAlwaysRelevant=true
}
