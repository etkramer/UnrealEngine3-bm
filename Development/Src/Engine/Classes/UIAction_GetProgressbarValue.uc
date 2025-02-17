/**
 * This action retrieves the current value of the progressbar which is the target of this action.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetProgressBarValue extends UIAction_GetValue;

/** Value that will be set by the UIProgressBar handler */
var		float	 Value;

/** Specifies whether the output value will be returned as a percentage of total progressbar range */ 
var()   bool	bPercentageValue;

DefaultProperties
{
	ObjName="Get ProgressBar Value"
	bPercentageValue=false

	// add a variable link to receive the current value of the progressbar
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Value",bWriteable=true,PropertyName=Value))
}