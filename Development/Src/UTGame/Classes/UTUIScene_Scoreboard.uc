/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_Scoreboard extends UTUIScene_Hud
	native(UI)
	abstract;

var UTPlayerController Host;

cpptext
{
	virtual void Tick( FLOAT DeltaTime );
}


event PostInitialize()
{
	Super.PostInitialize();
	SceneClosedCue='';
}

/** Make sure we free up the reference to the host */
function NotifyGameSessionEnded()
{
	Super.NotifyGameSessionEnded();
	Host = none;
}

event TickScene(float DeltaTime)
{
}


defaultproperties
{
	bCloseOnLevelChange=true
	bIgnoreAxisInput=true
	SceneRenderMode=SPLITRENDER_Fullscreen
}
