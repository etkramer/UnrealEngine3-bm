/**
 * Class description
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIEvent_TabPageInserted extends UIEvent_TabControl;

/** the position [into the tab control's pages array] where this page was inserted */
var	int	InsertedIndex;

DefaultProperties
{
	ObjName="Page Added"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Page Location",bWriteable=true,PropertyName=InsertedIndex))
}
