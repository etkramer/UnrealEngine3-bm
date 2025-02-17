/**
 * Base class for all UI sequence actions which interact with the focus chain.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIAction_FocusActions extends UIAction
	native(inherit)
	abstract;

/**
 * Used to ensure output link indices are synchronized between native and script.
 */
enum EFocusActionResultIndex
{
	FOCUSACTRESULT_Success,
	FOCUSACTRESULT_Failure,
};

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
	ObjName="Focus Actions"
	ObjCategory="Focus"

	// add a variable link which allows the designer to specify the index to use for data retrieval
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets,MaxVars=1)
}
