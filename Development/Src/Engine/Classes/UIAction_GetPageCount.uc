/**
 * Provides access to the number of pages in the tab control.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetPageCount extends UIAction_TabControl;

/* == Events == */
/**
 * Called when this event is activated.  Performs the logic for this action.
 */
event Activated()
{
	local int PageCount;
	local SeqVar_Int IntVar;
	local bool bSuccess;

	Super.Activated();

	if ( TabControl != None )
	{
		PageCount = TabControl.GetPageCount();
		foreach LinkedVariables( class'SeqVar_Int', IntVar, "Count" )
		{
			IntVar.IntValue = PageCount;
			bSuccess = true;
		}
	}

	if ( bSuccess )
	{
		if ( !OutputLinks[0].bDisabled )
		{
			OutputLinks[0].bHasImpulse = true;
		}
	}
	else
	{
		if ( !OutputLinks[1].bDisabled )
		{
			OutputLinks[1].bHasImpulse = true;
		}
	}
}

DefaultProperties
{
	ObjName="Get Page Count"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Count",bWriteable=true))
}
