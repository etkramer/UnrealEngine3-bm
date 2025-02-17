/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Event mapped to controller input.
 */
class SeqEvt_Input extends SequenceEvent
	native(Sequence);

cpptext
{
	UBOOL RegisterEvent();
}

/** Button press to catch */
var() array<EGameButtons> ButtonNames<autocomment=true>;

/** Should the input be eaten by the event, or allowed to propagate to gameplay? */
var() bool bTrapInput;

enum InputRegisterType
{
	IRT_AllPlayers,
	IRT_MarcusOnly,
	IRT_DomOnly,
	IRT_COGOnly,
	IRT_LocustOnly,
};
var() InputRegisterType RegisterType;

native final function bool CheckInputActivate(EGameButtons Button,bool bPressed);

defaultproperties
{
	ObjName="Input"
	ObjCategory="Misc"
	bTrapInput=TRUE

	VariableLinks(1)=(LinkDesc="Button",ExpectedType=class'SeqVar_String')

	OutputLinks(0)=(LinkDesc="Pressed")
	OutputLinks(1)=(LinkDesc="Released")

	MaxTriggerCount=0
	ReTriggerDelay=0.01
}
