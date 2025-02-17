/**
 * This event is activated the user "clicks" or otherwise chooses a list item.
 *
 * The Activator for this event should be a UIList object.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIEvent_SubmitListData extends UIEvent_SubmitData
	native(inherit);

/**
 * Stores the value of the selected item associated with this event's activator list.
 */
var() editconst	transient	int		SelectedItem;

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
	ObjName="Submit List Selection"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Selected Item",PropertyName=SelectedItem,bWriteable=true))
}
