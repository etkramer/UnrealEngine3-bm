/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class CameraVolume extends Volume
	native(Camera)
	placeable;

var() bool				bActive;

/** CameraActor associated with this volume. */
var() CameraActor		CameraActor;

var() bool				bAutomaticFraming;

var() bool				bForcePlayerToWalk;

//
//replication
//{
//	if (bNetDirty)
//		bActive;
//}


/** Internal. Tell all touching pawns that they're touching. */
simulated protected function NotifyPawnsTouched()
{
	local GearPawn GP;
	foreach TouchingActors(class'GearPawn', GP)
	{
		GP.EnteredCameraVolume(self);
	}
}

/** Internal. Tell all touching pawns that they're NOT touching. */
simulated protected function NotifyPawnsUntouched()
{
	local GearPawn GP;
	foreach TouchingActors(class'GearPawn', GP)
	{
		GP.ExitedCameraVolume(self);
	}
}


function Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	local GearPawn OtherGP;

	super.Touch(Other, OtherComp, HitLocation, HitNormal);

	if (bActive)
	{
		OtherGP = GearPawn(Other);
		if (OtherGP != None)
		{
			OtherGP.EnteredCameraVolume(self);
		}
	}
}


function Untouch( Actor Other )
{
	local GearPawn OtherGP;

	super.UnTouch(Other);

	if (bActive)
	{
		OtherGP = GearPawn(Other);
		if (OtherGP != None)
		{
			OtherGP.ExitedCameraVolume(self);
		}
	}
}

function Destroyed()
{
	NotifyPawnsUntouched();
	super.Destroyed();
};


simulated function OnToggle(SeqAct_Toggle Action)
{
	local GearPawn Pawn;
	local bool bOldActive;
	local CameraVolume CamVol;

	bOldActive = bActive;

	if (Action.InputLinks[0].bHasImpulse)
	{
		bActive = TRUE;
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		bActive = FALSE;
	}
	else if (Action.InputLinks[2].bHasImpulse)
	{
		bActive = !bActive;
	}

	if (bActive)
	{
		// abort any active roadie runs
		foreach WorldInfo.AllPawns(class'GearPawn',Pawn)
		{
			if (Pawn.IsDoingSpecialMove(SM_RoadieRun))
			{
				foreach Pawn.TouchingActors(class'CameraVolume',CamVol)
				{
					if (CamVol == self)
					{
						break;
					}
				}
			}
		}
	}

	if (bActive != bOldActive)
	{
		if (bActive)
		{
			// just became active, notify touching pawns.
			NotifyPawnsTouched();
		}
		else
		{
			// becuase inactive, notify touching pawns
			NotifyPawnsUntouched();
		}
	}
	//bForceNetUpdate = TRUE;
}

defaultproperties
{
	bActive=FALSE
	bAutomaticFraming=TRUE
	bForcePlayerToWalk=TRUE
//	RemoteRole=ROLE_SimulatedProxy
//	bAlwaysRelevant=true
}
