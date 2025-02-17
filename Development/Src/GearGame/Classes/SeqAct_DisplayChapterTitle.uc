/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_DisplayChapterTitle extends SequenceAction;

// Chapter to display
var() EChapterPoint DisplayChapter;
// Total number of seconds to display the chapter title
var() float TotalDisplayTime;
// Time it will take to fade the text in and out
var() float TotalFadeTime;

event Activated()
{
	local GearPC PC;

	GearGameSP_Base(GetWorldInfo().Game).CurrentChapter = DisplayChapter;
	foreach GetWorldInfo().AllControllers(class'GearPC', PC)
	{
		PC.ClientDisplayChapterTitle(DisplayChapter, TotalDisplayTime, TotalFadeTime);
	}
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

defaultproperties
{
	ObjName="Display Chapter Title"
	ObjCategory="Level"
	bCallHandler=false

	TotalDisplayTime=8.0
	TotalFadeTime=2.0

	VariableLinks.Empty
}
