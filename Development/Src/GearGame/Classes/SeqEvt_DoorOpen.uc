/**
* Activated when a Trigger_DoorInteraction is used.
* Originator: the usable actor that was used
* Instigator: The pawn associated with the player did the using
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqEvt_DoorOpen extends SeqEvt_Interaction;

simulated function bool CanDoInteraction(GearPC PC)
{
	return (Super.CanDoInteraction(PC) &&
			PC.DoorTriggerDatum.DoorTrigger != None &&
			PC.DoorTriggerDatum.bInsideDoorTrigger &&
			(PC.Role < ROLE_Authority || CheckActivate(PC.DoorTriggerDatum.DoorTrigger,PC.Pawn,TRUE)));
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
	ObjName="DoorOpen"

	// support being pushed open or kicked
	OutputLinks(0)=(LinkDesc="Opened")
	OutputLinks(1)=(LinkDesc="Kicked")

	InteractAction={(
				ActionName=DoorOpen,
				ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // A Button
									(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=287,V=140,UL=91,VL=98)))	),
				)}
}
