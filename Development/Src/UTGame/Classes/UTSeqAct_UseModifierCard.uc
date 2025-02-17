/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTSeqAct_UseModifierCard extends SequenceAction;

var() name CardType;
var() bool bRemoveKey;

event Activated()
{
	local PlayerController PC;
	local UTPlayerController UTPC;

	foreach GetWorldInfo().LocalPlayerControllers(class'PlayerController', PC)
	{
		UTPC = UTPlayerController(PC);
		if (UTPC != none)
		{
			break;
		}
	}

	if ( UTPC != none )
	{
		UTPC.UseModifierCard(CardType);
	}
}


defaultproperties
{
	ObjCategory="Story"
	ObjName="Use Modifier Card"
}
