/**
 * This action gives focus to a widget in the scene.  When giving focus to a widget not contained within the
 * same scene, the scene containing that widget will become the focused scene.  If the scene which contains
 * the widget is not loaded, the scene containing the Target widget is opened.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetFocus extends UIAction_FocusActions
	native(inherit);

cpptext
{
	/**
	 * Calls set focus on all attached targets.
	 */
	virtual void Activated();
}

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

DefaultProperties
{
	ObjName="Set Focus"
	bAutoTargetOwner=false

	OutputLinks(FOCUSACTRESULT_Success)=(LinkDesc="Success")
	OutputLinks(FOCUSACTRESULT_Failure)=(LinkDesc="Failed")
}
