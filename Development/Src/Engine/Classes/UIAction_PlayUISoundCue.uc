/**
 * This class allows the designer to play a sound cue using a UISoundCue alias name.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_PlayUISoundCue extends UIAction
	native(inherit);

cpptext
{
	/**
	 * Plays the UISoundCue indicated by SoundCueName.
	 */
	virtual void Activated();
}

var()		string			SoundCueName;

DefaultProperties
{
	ObjName="Play UI Sound Cue"
	ObjCategory="Sound"

	bAutoActivateOutputLinks=false
	bAutoTargetOwner=true

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Cue Name",PropertyName=SoundCueName))

	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failure")
}
