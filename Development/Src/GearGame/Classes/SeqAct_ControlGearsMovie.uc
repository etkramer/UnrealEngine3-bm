/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ControlGearsMovie extends SequenceAction
	native(Sequence);

/** Which movie to play */
var() string MovieName;

cpptext
{
	/**
	 * Executes the action when it is triggered 
	 */
	void Activated();
}

defaultproperties
{
	ObjName="Control Gears Movie"
	ObjCategory="Cinematic"

	InputLinks(0)=(LinkDesc="Play")
	InputLinks(1)=(LinkDesc="Stop")
	
	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="MovieName",PropertyName=MovieName)
} 
