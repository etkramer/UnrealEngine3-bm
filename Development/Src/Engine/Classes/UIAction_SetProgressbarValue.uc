/**
 * Changes the current value of a UIProgressBar
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetProgressBarValue extends UIAction_SetValue;

/** Specifies new progressbar value */
var()	float	NewValue;

/** Specifies whether the NewValue should be treated as a precentage */
var()   bool	bPercentageValue;

DefaultProperties
{
	bPercentageValue=false

	ObjName="Set ProgressBar Value"
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="New Value",PropertyName=NewValue))
}