/**
 * This event is activated when the index of a UIList is changed.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_ListIndexChanged extends UIEvent_ValueChanged
	native(inherit);

/** the index of the UIList before this event was called */
var	transient	int		PreviousIndex;

/** the index that the UIList changed to */
var	transient	int		CurrentIndex;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return TargetObject == None || UIList(TargetObject) != None || TargetObject.ContainsChildOfClass(class'UIList');
}

DefaultProperties
{
	ObjName="List Index Changed"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Old Index",bWriteable=true,PropertyName="PreviousIndex"))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="New Index",bWriteable=true,PropertyName="CurrentIndex"))
}
