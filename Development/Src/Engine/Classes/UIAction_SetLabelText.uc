/**
 * Changes the value of a label's text
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetLabelText extends UIAction_SetValue;

var() string	NewText;

DefaultProperties
{
	ObjName="Set Text Value"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="New Value",PropertyName=NewText))
}
