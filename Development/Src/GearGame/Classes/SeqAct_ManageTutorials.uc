/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/*****************************************************************************
 SeqAct_ManageTutorials - Manages the tutorial system

 Inputs:
	Start Tutorial - Adds a tutorial to the system
	Stop Tutorial - Removes a tutorial from the system but does not complete it
	Complete Tutorial - Completes the tutorial and removes it from the system
	System On - Turns the tutorial system on, which will additionally load all the auto-initialized tutorials
	System Off - Turns the tutorial system off and removes all existing tutorials from the system

 Outputs:
	Out - Just an action pass through
	Completed - Level Designer way of completed a tutorials - completion can also happen from logic within the tutorial itself
	Stopped - Level Designer way of having a tutorial removed but not completed

*****************************************************************************/
class SeqAct_ManageTutorials extends SequenceAction
	native(Sequence);

cpptext
{
	/** Callback for when the event is activated. */
	virtual void Activated();

	/** Callback for when the event is deactivated. */
	virtual void DeActivated();

	/**
	* Polls to see if the async action is done
	*
	* @param ignored
	*
	* @return TRUE if the operation has completed, FALSE otherwise
	*/
	UBOOL UpdateOp(FLOAT);
}

/** Enum of the outputs for this action */
enum EManageTutorialOutputs
{
	eMTOUTPUT_Added,
	eMTOUTPUT_Removed,
	eMTOUTPUT_Started,
	eMTOUTPUT_Stopped,
	eMTOUTPUT_Completed,
	eMTOUTPUT_SystemOn,
	eMTOUTPUT_SystemOff,
	eMTOUTPUT_UISceneOpened,
	eMTOUTPUT_AlreadyComplete,
	eMTOUTPUT_SystemDead,
	eMTOUTPUT_AutosOn,
	eMTOUTPUT_AutosOff,
};

/** The tutorial type to manage */
var() EGearTutorialType Tutorial_Type;

/** Whether to automatically start the tutorial when it gets added */
var() bool bStartTutorialWhenAdded;

/** If accompanying this tutorial with an objective: Name of objective for scripting */
var() Name Objective_Name;
/** If accompanying this tutorial with an objective: Description of the objective */
var() string Objective_Desc;
/** If accompanying this tutorial with an objective: Whether to notify the player about the objective */
var() bool Objective_bNotifyPlayer;

/** Whether the system should remove the tutorials when the system is turned on */
var() bool bWipeTutorialsOnSystemOn;
/** Whether the system should remove the tutorials when the system is turned off */
var() bool bWipeTutorialsOnSystemOff;

/** Whether this action is finished or not */
var bool bActionIsDone;
/** Whether this tutorial was completed or not - for determining which output link to fire */
var bool bTutorialCompleted;
/** Whether this tutorial was already completed or not - for determining which output link to fire */
var bool bTutorialAlreadyCompleted;
/** Whether this tutorial is completed and dead - will no longer function */
var bool bTutorialIsDead;
/** Whether the tutorial system is dead - not being used by the game */
var bool bTutorialSystemIsDead;


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
	return Super.GetObjClassVersion() + 3;
}

defaultproperties
{
	ObjName="Tutorial Manager"
	ObjCategory="Gear"

	bStartTutorialWhenAdded=true
	Objective_bNotifyPlayer=true

	bAutoActivateOutputLinks=false
	bLatentExecution=true
	bWipeTutorialsOnSystemOff=true

	InputLinks(eMTINPUT_AddTutorial)=(LinkDesc="Add Tutorial")
	InputLinks(eMTINPUT_RemoveTutorial)=(LinkDesc="Remove Tutorial")
	InputLinks(eMTINPUT_StartTutorial)=(LinkDesc="Start Tutorial")
	InputLinks(eMTINPUT_StopTutorial)=(LinkDesc="Stop Tutorial")
	InputLinks(eMTINPUT_CompleteTutorial)=(LinkDesc="Complete Tutorial")
	InputLinks(eMTINPUT_SystemOn)=(LinkDesc="System On")
	InputLinks(eMTINPUT_SystemOff)=(LinkDesc="System Off")
	InputLinks(eMTINPUT_AutosOn)=(LinkDesc="AutoTuts On")
	InputLinks(eMTINPUT_AutosOff)=(LinkDesc="AutoTuts Off")

	OutputLinks(eMTOUTPUT_Added)=(LinkDesc="Added")
	OutputLinks(eMTOUTPUT_Removed)=(LinkDesc="Removed")
	OutputLinks(eMTOUTPUT_Started)=(LinkDesc="Started")
	OutputLinks(eMTOUTPUT_Stopped)=(LinkDesc="Stopped")
	OutputLinks(eMTOUTPUT_Completed)=(LinkDesc="Completed")
	OutputLinks(eMTOUTPUT_SystemOn)=(LinkDesc="SystemOn")
	OutputLinks(eMTOUTPUT_SystemOff)=(LinkDesc="SystemOff")
	OutputLinks(eMTOUTPUT_UISceneOpened)=(LinkDesc="UI Scene Opened")
	OutputLinks(eMTOUTPUT_AlreadyComplete)=(LinkDesc="Already Complete")
	OutputLinks(eMTOUTPUT_SystemDead)=(LinkDesc="System Dead")
	OutputLinks(eMTOUTPUT_AutosOn)=(LinkDesc="AutosOn")
	OutputLinks(eMTOUTPUT_AutosOff)=(LinkDesc="AutosOff")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets,bHidden=TRUE)
}
