/**
 * Base class for behavior of Jack
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Base_Jack extends AICommand_Base_Combat
	within GearAI_Jack;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();
//	GotoState('InCombat');
}

defaultproperties
{
}
