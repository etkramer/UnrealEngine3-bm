/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This event will be fired when a level is loaded and made visible. It is primarily 
 * used for notifying when a sublevel is associated with the world.
 *
 **/
class SeqEvent_LevelLoaded extends SequenceEvent
	native(Sequence);

defaultproperties
{
	ObjName="Level Loaded And Visible"
	VariableLinks.Empty
	bPlayerOnly=false
}
