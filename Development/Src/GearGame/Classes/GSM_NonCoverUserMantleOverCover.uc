/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_NonCoverUserMantleOverCover extends GSM_MantleOverCoverBase;

/** This is a mantle base class for any pawns that don't actually use cover, but still need to mantle 
	Works for *AI* only
**/

function MantleReachSpec GetCurrentMantleSpec()
{
	return MantleReachSpec(PawnOwner.Controller.CurrentPath);
}

protected function bool InternalCanDoSpecialMove()
{
	if(GetCurrentMantleSpec() == none)
	{
		return FALSE;
	}

	// Adjust mantle to fit this cover's thickness.
	if( !FindMantleDistance() )
	{
		`log(PawnOwner.WorldInfo.TimeSeconds @ class @ GetFuncName() @ PawnOwner @ "FAILED because couldn't adjust mantle distance.");
		return FALSE;
	}

	return TRUE;
}

simulated function GetMantleInformation( out BasedPosition InMantleStartLoc, out BasedPosition InMantleEndLoc, out float InMantleDistance)
{
	local MantleReachSpec CurSpec;
	local Vector MantleDir;

	CurSpec = GetCurrentMantleSpec();
	MantleDir = CurSpec.GetEnd().Location - PawnOwner.Location;
	MantleDir.Z = 0.f;
	InMantleDistance = VSize2D(MantleDir);

	MantleDir		= Normal(MantleDir);

	SetBasedPosition( InMantleStartLoc, PawnOwner.Location );
	SetBasedPosition( InMantleEndLoc, PawnOwner.Location + MantleDir * InMantleDistance );
}

defaultproperties
{
}