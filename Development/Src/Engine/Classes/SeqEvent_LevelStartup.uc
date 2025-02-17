/**
 * Event which is activated by GameInfo.StartMatch when the match begins.
 * Originator: current WorldInfo
 * Instigator: None
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqEvent_LevelStartup extends SequenceEvent
	native(Sequence);

defaultproperties
{
	ObjName="Level Startup"
	VariableLinks.Empty
	bPlayerOnly=false
}
