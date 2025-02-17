/**
 * This action allows designers to change the index of a UIList.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetListIndex extends UIAction_SetValue;

/** the index to change the list to */
var()	int		NewIndex;

/** whether to clamp invalid index values */
var()	bool	bClampInvalidValues;

/** whether the list should activate the ListIndexChanged event */
var()	bool	bActivateListChangeEvent;

DefaultProperties
{
	ObjName="Set List Index"
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="New Index",PropertyName=NewIndex))
	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failed")

	NewIndex=-1
	bClampInvalidValues=true
	bActivateListChangeEvent=true
}
