/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_SetWeather extends SequenceAction
	native(Sequence);

/** FALSE for default height, TRUE to use RainEmitterHeight. */
var() bool	bOverrideRainEmitterHeight;
var() float RainEmitterHeight;

/** FALSE for default height, TRUE to use HailEmitterHeight. */
var() bool	bOverrideHailEmitterHeight;
var() float HailEmitterHeight;

cpptext
{
	virtual void Activated();
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
	return Super.GetObjClassVersion() + 0;
}

defaultproperties
{
	ObjName="Set Weather"
	ObjCategory="Gear"

	RainEmitterHeight=64
	HailEmitterHeight=512

	InputLinks(0)=(LinkDesc="Clear")
	InputLinks(1)=(LinkDesc="Rain")
	InputLinks(2)=(LinkDesc="Hail")

	VariableLinks.Empty
}
