
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_Wretch_RecoverFromRagdoll extends GSM_RecoverFromRagdoll;


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Restore AI to previous state.
	if( PawnOwner.Controller != None && PawnOwner.Controller.IsA('GearAI') )
	{
		PawnOwner.Controller.PopState();
	}
}

defaultproperties
{
	BS_GetUpFront=(AnimName[BS_FullBody]="GetUp_FaceDown")
	BS_GetUpBack=(AnimName[BS_FullBody]="GetUp_FaceUp")

	UpDownBoneName="b_W_Pelvis"
	UpDownAxis=AXIS_Z
	OrientationBoneName="b_W_Pelvis"
	OrientationAxis=AXIS_X
	GetUpFromBackYawOffset=-16384
}