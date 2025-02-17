/**
 * This event is activated when a checkbox is checked or unchecked.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_CheckValueChanged extends UIEvent_ValueChanged
	native(inherit);

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return TargetObject == None || UICheckBox(TargetObject) != None || TargetObject.ContainsChildOfClass(class'UICheckBox');
}

DefaultProperties
{
	ObjName="Checkbox Value Changed"
	OutputLinks(0)=(LinkDesc="False")
	OutputLinks(1)=(LinkDesc="True")

	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Value",bWriteable=true))
}
