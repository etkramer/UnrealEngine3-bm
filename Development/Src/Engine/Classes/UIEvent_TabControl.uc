/**
 * Base class for all events activated by a UITabControl, UITabButton, or UITabPage.
 *
 * For this event class and its children, the EventActivator is the TabPage which generated this event.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIEvent_TabControl extends UIEvent
	native(inherit)
	abstract;

cpptext
{
	/** USequenceOp interface */
	/**
	 * Allows the operation to initialize the values for any VariableLinks that need to be filled prior to executing this
	 * op's logic.  This is a convenient hook for filling VariableLinks that aren't necessarily associated with an actual
	 * member variable of this op, or for VariableLinks that are used in the execution of this ops logic.
	 *
	 * Initializes the value of the "OwnerTabControl" linked variable by copying the value of the activator tab page's tab
	 * control into the linked variable.
	 */
	virtual void InitializeLinkedVariableValues();
}

/** the tab control that contained the page which generated this event */
var		UITabControl		OwnerTabControl;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	if ( TargetObject == None )
	{
		return true;
	}

	if ( UITabControl(TargetObject) != None || TargetObject.ContainsChildOfClass(class'UITabControl') )
	{
		return true;
	}

	if ( UITabButton(TargetObject) != None || UITabPage(TargetObject) != None )
	{
		return true;
	}

	return false;
}

DefaultProperties
{
	ObjName="Tab Control Event"
	ObjCategory="Tab Control"

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Tab Page",bWriteable=true)
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Owner Tab Control",PropertyName=OwnerTabControl,bWriteable=true,bHidden=true))
}
