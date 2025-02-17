/**
 * This action instructs the owning widget to save its current value to the data store it's bound to.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_PublishValue extends UIAction_DataStore
	native(inherit);

cpptext
{
	/**
	 * If the owning widget implements the UIDataStorePublisher interface, calls the appropriate method for publishing the
	 * owning widget's value to the data store.
	 */
	virtual void Activated();
}

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	if ( !Super.IsValidUISequenceObject(TargetObject) )
	{
		return false;
	}

	if ( TargetObject != None )
	{
		// this op can only be used by widgets that implement the UIDataStorePublisher interface
		return UIDataStorePublisher(TargetObject) != None;
	}

	return true;
}

DefaultProperties
{
	ObjName="Save Value"
}
