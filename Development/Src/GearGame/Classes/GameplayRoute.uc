/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Route used to define a squad route through the level
 */
class GameplayRoute extends Route
	native(AI);

/** if this bool is on, then when the squadleader moves backward the squad will follow to that position.  Otherwise it will only move forward */
var() bool bAllowBackwardProgression;

defaultproperties
{
	FudgeFactor=0.8f
	DrawScale=5.f
}