/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTSeqCond_HasPersistentKey extends SequenceCondition;

var PlayerController Target;
var() ESinglePlayerPersistentKeys SearchKey;

event Activated()
{
	local PlayerController PC;
	local UTPlayerController UTPC;

	foreach GetWorldInfo().AllControllers(class'PlayerController', PC)
	{
		UTPC = UTPlayerController(PC);
		if (UTPC != none)
		{
			break;
		}
	}

	if ( UTPC != none )
	{
		OutputLinks[ UTPC.HasPersistentKey(SearchKey) ? 0 : 1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjName="Has Persistent Key"
	OutputLinks(0)=(LinkDesc="Has Key")
	OutputLinks(1)=(LinkDesc="No Key")
}
