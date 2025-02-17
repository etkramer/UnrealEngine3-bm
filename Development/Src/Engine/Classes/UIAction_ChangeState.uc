/**
 * Base class for actions that change widget states.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_ChangeState extends UIAction
	abstract
	native(inherit);

/**
 * If not target state is specified, then the first State in the target widget's InactiveStates array
 * that has this class will be activated
 */
var()	class<UIState>		StateType;

/**
 * (Optional)
 * the state that should be activated by this action.  Useful when a widget contains more
 * than one instance of a particular state class in its InactiveStates array (such as a focused state, and
 * a specialized version of the focused state)
 */
var()			UIState		TargetState;

/**
 * Determines whether this action was successfully executed.
 * Should be set by the handler function for this action.
 */
var		bool				bStateChangeFailed;

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
	bCallHandler=false
	OutputLinks(0)=(LinkDesc="Successful")
	OutputLinks(1)=(LinkDesc="Failed")
//	VariableLinks.Add(ExpectedType=class'SeqVar_Object',LinkDesc="Target State")

	bAutoActivateOutputLinks=false
	bAutoTargetOwner=true
}
