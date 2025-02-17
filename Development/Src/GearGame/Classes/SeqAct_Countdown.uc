/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
* This will make all players show a countdown on the screen and allow the designer to
* perform kismet actions based on the result of the countdown.
* Inputs:  Start - Start the countdown
*          Stop - Stop the countdown
* Outputs: Out - Called after the timer was started
*          Stopped - Called after the countdown was stopped
*          Expired - Called when the countdown has expired
*/
class SeqAct_Countdown extends SequenceAction
	native(Sequence);

/** Amount of time the countdown lasts */
var() float TotalCountdownInSeconds;

cpptext
{
	void Activated();
	UBOOL UpdateOp(FLOAT deltaTime);
};

defaultproperties
{
	ObjName="Countdown"
	ObjCategory="Gear"

	TotalCountdownInSeconds=10.f

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")

	OutputLinks(0)=(LinkDesc="Started")
	OutputLinks(1)=(LinkDesc="Stopped")
	OutputLinks(2)=(LinkDesc="Finished")

	VariableLinks.Empty
	bAutoActivateOutputLinks=false
	bLatentExecution=true
}