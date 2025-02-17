/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqEvt_Engage extends SeqEvt_Interaction;


/************************************************************************/
/* Enums, consts, structs, etc.                                         */
/************************************************************************/
enum EEngageEventOutputType
{
	eENGAGEOUT_Interacted,
	eENGAGEOUT_Finished,
	eENGAGEOUT_Started,
	eENGAGEOUT_Stopped,
};

simulated function bool CanDoInteraction(GearPC PC)
{
	return (Super.CanDoInteraction(PC) && PC.CanDoSpecialMove(SM_Engage_Start));
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
	return Super.GetObjClassVersion() + 0;
}

defaultproperties
{
	ObjName="Engage"

	MaxTriggerCount=0
	ReTriggerDelay=0.0f

	OutputLinks(eENGAGEOUT_Interacted)=(LinkDesc="Interacted")
	OutputLinks(eENGAGEOUT_Finished)=(LinkDesc="Finished")
	OutputLinks(eENGAGEOUT_Started)=(LinkDesc="Started")
	OutputLinks(eENGAGEOUT_Stopped)=(LinkDesc="Stopped")

	InteractAction={(
		ActionName=WheelGrab,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=109,V=429,UL=68,VL=71)))	),
	)}
}
