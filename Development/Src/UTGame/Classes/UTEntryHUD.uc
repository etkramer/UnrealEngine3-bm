/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTEntryHUD extends GameHUD
	config(Game);

event PostRender()
{
	return;
}

/** We override this here so we do not have the copyright screen show up in envyentry or when you skip past a movie **/
function DrawEngineHUD();



defaultproperties
{
}