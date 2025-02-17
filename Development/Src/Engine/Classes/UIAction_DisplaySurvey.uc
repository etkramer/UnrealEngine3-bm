/**
 * Displays a gameplay experience survey for the player to respond to
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_DisplaySurvey extends UIAction
	native(inherit);

cpptext
{
	/**
	 * Calls the native layer to trigger the survey
	 */
	virtual void Activated(void);
}

/** The id of the question to show */
var() string QuestionId;

/** The context for the question */
var() string QuestionContext;

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

defaultproperties
{
	ObjName="Display Survey"
	ObjCategory="Survey"
	bAutoTargetOwner=true

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="QuestionId",PropertyName="QuestionId"))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Context",PropertyName="QuestionContext"))
}
