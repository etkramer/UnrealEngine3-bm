/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_TravelMap extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
}

/** The map name to open **/
var() string MapName;

/** The paramaters to pass in after the map name.  (e.f.  ?key0=value0?key1=value1 **/
var() string Parameters;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Travel Map"
	ObjCategory="Level"

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Map Name",PropertyName=MapName))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parameters",PropertyName=Parameters))
}
