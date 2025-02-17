// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
class UTMutator_NoTranslocator extends UTMutator;

function InitMutator(string Options, out string ErrorMessage)
{
	if ( UTGame(WorldInfo.Game) != None )
	{
		UTGame(WorldInfo.Game).bAllowTranslocator = false;
	}
	Super.InitMutator(Options, ErrorMessage);
}

defaultproperties
{
	GroupNames[0]="TRANSLOC"
}
