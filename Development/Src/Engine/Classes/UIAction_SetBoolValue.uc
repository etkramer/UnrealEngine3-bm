/**
 * This action allows designers to change the value of a widget that contains boolean data.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetBoolValue extends UIAction_SetValue;

var()				bool					bNewValue;

DefaultProperties
{
	ObjName="Set Bool Value"
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="New Value",PropertyName=bNewValue))
}
