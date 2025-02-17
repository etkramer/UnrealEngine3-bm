/**
 * Changes the datastore binding for a widget.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetDatastoreBinding extends UIAction_DataStore;

var() string	NewMarkup;

DefaultProperties
{
	ObjName="Set Datastore Binding"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="New Markup",PropertyName=NewMarkup))
}
