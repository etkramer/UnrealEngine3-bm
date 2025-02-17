
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_DoorKick extends GSM_DoorPush;

var ForceFeedbackWaveform KickFF;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);
	if (PCOwner != None && PCOwner.IsLocalPlayerController())
	{
		PCOwner.SetTimer(0.7,FALSE,nameof(PlayKickFF),self);
	}
}

function PlayKickFF()
{
	PCOwner.ClientPlayForceFeedbackWaveform(KickFF);
}

defaultproperties
{
	SpecialMoveCameraBoneAnims(0)=(AnimName="Camera_Door_Kick")

	BS_Animation=(AnimName[BS_FullBody]="door_kick")
	SpecialMoveID=SM_DoorKick

	bDisablePOIs=TRUE

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformKick1
		Samples(0)=(LeftAmplitude=65,RightAmplitude=65,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.2)
	End Object
	KickFF=ForceFeedbackWaveformKick1
}