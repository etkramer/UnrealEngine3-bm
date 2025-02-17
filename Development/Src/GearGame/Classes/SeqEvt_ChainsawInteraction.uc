/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqEvt_ChainsawInteraction extends SeqEvt_Interaction
	native(Sequence);

var() bool bDisableCamera;

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
	ObjName="Trigger Chainsawed"
	OutputLinks(0)=(LinkDesc="Begin Chainsaw")
	OutputLinks(1)=(LinkDesc="End Chainsaw")
	OutputLinks(2)=(LinkDesc="Begin Bash")
	OutputLinks(3)=(LinkDesc="End Bash")

	MaxTriggerCount=2

	InteractAction={(
			ActionName=SawInteraction,
            ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=83,V=0,UL=45,VL=32))), // b button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=0,UL=83,VL=66))) ), // saw
			)}
}
