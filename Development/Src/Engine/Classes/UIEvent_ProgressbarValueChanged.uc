/**
 * This event is activated when the value of a UIProgressBar changes.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_ProgressBarValueChanged extends UIEvent_ValueChanged
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
	return TargetObject == None || UIProgressBar(TargetObject) != None || TargetObject.ContainsChildOfClass(class'UIProgressBar');
}

DefaultProperties
{
	ObjName="ProgressBar Value Changed"

	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Value",bWriteable=true))
}
