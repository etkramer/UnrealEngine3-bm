/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_Log extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
};

/** Should this message be drawn on the screen as well as placed in the log? */
var() bool bOutputToScreen;

/** Should ObjComment be included in the log? */
var() bool bIncludeObjComment;

/** Time to leave text floating above Target actor */
var() float TargetDuration;

/** Offset to apply to the Target actor location when positioning debug text */
var() vector TargetOffset;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
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
	return Super.GetObjClassVersion() + 2;
}

defaultproperties
{
	ObjName="Log"
	ObjCategory="Misc"
	bOutputToScreen=TRUE
	bIncludeObjComment=TRUE
	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="String",MinVars=0,bHidden=TRUE)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="Float",MinVars=0,bHidden=TRUE)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",MinVars=0,bHidden=TRUE)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="Object",MinVars=0,bHidden=TRUE)
	VariableLinks(4)=(ExpectedType=class'SeqVar_Int',LinkDesc="Int",MinVars=0,bHidden=TRUE)
	VariableLinks(5)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets)
	VariableLinks(6)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="Obj List",MinVars=0,bHidden=TRUE)
	TargetDuration=-1.f
}
