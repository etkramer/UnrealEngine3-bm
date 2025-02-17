/**
 * This event is activated when a control that contains text data submits the string value.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_SubmitTextData extends UIEvent_SubmitData
	native(inherit);

/** the string value that is being published */
var()	string		Value;

/** Indicates whether the string value in the owning widget should be cleared */
var()	bool		bClearValue;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return TargetObject == None || UIEditBox(TargetObject) != None || TargetObject.ContainsChildOfClass(class'UIEditBox');
}

DefaultProperties
{
	ObjName="Submit Text"

	bClearValue=true
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Value",PropertyName=Value,bWriteable=true))
}
