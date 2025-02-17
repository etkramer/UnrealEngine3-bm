/**
 * This will set the state of the pawn to be in the rain. 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_PawnIsInRain extends SequenceAction
	deprecated;

/** DEPRECATED.  SeqAct_SetWeather should handle this now. */

/** Whether the pawn is in the rain or not **/
var() bool bPawnIsInRain;

defaultproperties
{
	ObjName="PawnIsInRain"
	ObjCategory="Gear"
}