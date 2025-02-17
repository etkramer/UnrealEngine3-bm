
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_LadderClimbUp extends GSM_LadderClimbDown;

simulated function GetLadderEntryPoint(out Vector Out_EntryLoc, out Rotator Out_EntryRot)
{
	PawnOwner.LadderTrigger.GetBottomEntryPoint(Out_EntryLoc, Out_EntryRot);
}

simulated function bool IsRightLadderTrigger()
{
	return !PawnOwner.LadderTrigger.bIsTopOfLadder;
}

simulated function CheckOverrideAnim()
{
	if (PawnOwner.LadderTrigger.ClimbUpAnim != '')
	{
		BS_Animation.AnimName[BS_FullBody] = PawnOwner.LadderTrigger.ClimbUpAnim;
	}
	else
	{
		BS_Animation.AnimName[BS_FullBody] = default.BS_Animation.AnimName[BS_FullBody];
	}
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="AR_Ladder_Up")

	Action={(
	ActionName=ClimbUp,
	ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
						(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=170,V=379,UL=64,VL=94)))	),
	)}

	LadderCameraBoneAnims(0)=(AnimName="Camera_Ladder_Up")
}
