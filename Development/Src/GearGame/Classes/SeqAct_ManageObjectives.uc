/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/*****************************************************************************
 SeqAct_ManageObjectives - Manages the objective system

 Inputs:
	Add Objective - Adds an objective to the system
	Completed Objective - Completes an already existing objective
	Failed Objective - Fails an already existing objective
	Abort Objective - Silently aborts an already existing objective
	Clear All Objectives - Silently clears all objectives from the system

 Outputs:
	Out - Just an action pass through
*****************************************************************************/
class SeqAct_ManageObjectives extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void Activated();
}

/** Name of objective for scripting */
var() Name ObjectiveName;

/** Description of the objective */
var() string ObjectiveDesc;

/** Whether to notify the player about the objective */
var() bool bNotifyPlayer;

defaultproperties
{
	ObjName="Manage Objectives"
	ObjCategory="Gear"

	ObjectiveName=DefaultObjective
	bNotifyPlayer=true

	InputLinks(0)=(LinkDesc="Add/Update Objective")
	InputLinks(1)=(LinkDesc="Completed Objective")
	InputLinks(2)=(LinkDesc="Failed Objective")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets,bHidden=TRUE)
}
