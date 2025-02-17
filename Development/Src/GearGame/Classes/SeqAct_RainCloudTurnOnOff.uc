/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * 
 */
class SeqAct_RainCloudTurnOnOff extends SequenceAction
	deprecated
	native(Sequence);

/** DEPRECATED.  Use SeqAct_SetWeather now. */

defaultproperties
{
	ObjName="RainCloudTurnOnOff"
	ObjCategory="Gear"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Bool',LinkDesc="ActiveOrNot",bWriteable=FALSE,MinVars=1,MaxVars=1)

}