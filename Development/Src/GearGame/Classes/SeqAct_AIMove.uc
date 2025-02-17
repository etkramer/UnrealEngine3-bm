/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqAct_AIMove extends SequenceAction
	native(Sequence);

cpptext
{
	UBOOL UpdateOp(FLOAT DeltaTime);
	void PreActorHandle(AActor *Actor);
	virtual void DeActivated();
	virtual void OnReceivedImpulse( class USequenceOp* ActivatorOp, INT InputLinkIndex );
}

/** Distance AI is allowed to use in regards to tethering */
var() float TetherDistance;

/** Direction we are moving along the route */
var() ERouteDirection RouteDirection;

/** List of actual move targets to pick from */
var() array<Actor> MoveTargets;

/** Focus while moving */
var() array<Actor> FocusTarget;

/** Is this interruptable by combat? */
var() bool bInterruptable;

/** Should the tether be cleared once it's reached? */
var() bool bClearTetherOnArrival;

/** The AI this action was sent to */
var array<GearAI> AITargets;
/** AI that reached goal since last output called */
var array<GearAI> AIReachedGoal;

enum EAIMoveStyle
{
	EMS_Normal,
	EMS_Slow,
	EMS_Fast,
};
var() EAIMoveStyle MovementStyle;

var() EAIMoveMood MovementMood;

/** if our Destination is a coverlink, move to the slot of that coverlink at this index instead (-1 means go to the link not a slot)*/
var() int DestinationSlotIndex;

/** generated from OnAIMove in GearAI, list of tethers we have available to chose from (when the LD gives us a list) */
var array<Actor> AvailableTethers;

/** Called once the AI has reached the goal */
final native function ReachedGoal(GearAI AI);

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
	return Super.GetObjClassVersion() + 10;
}

defaultproperties
{
	ObjName="AI: Go Here"
	ObjCategory="AI"

	bLatentExecution=TRUE
	bAutoActivateOutputLinks=FALSE

	TetherDistance=256.f

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Move To",PropertyName=MoveTargets)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Look At",PropertyName=FocusTarget)
	VariableLinks(3)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="Last Reached",bWriteable=TRUE)

	OutputLinks(0)=(LinkDesc="Moved")
	OutputLinks(1)=(LinkDesc="Reached Goal")
	OutputLinks(2)=(LinkDesc="Out")
	DestinationSlotIndex=-1
}
