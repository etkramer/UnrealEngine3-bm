/**
 * This action is used to retrieve the value from widgets that support string data.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetTextValue extends UIAction_GetValue;

/** Stores the value from the GetTextValue handler */
var	string	StringValue;

DefaultProperties
{
	ObjName="Get Text Value"

	// add a variable link to receive the value of the widget's text data
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Text Value",bWriteable=true,PropertyName=StringValue))
}
