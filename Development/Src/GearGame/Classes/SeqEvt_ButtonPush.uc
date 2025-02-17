/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqEvt_ButtonPush extends SeqEvt_Interaction;

defaultproperties
{
	ObjName="Button Pushed"

	OutputLinks(0)=(LinkDesc="Pushed")

	InteractAction={(
			ActionName=ButtonPush,
            ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // x button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=360,V=0,UL=58,VL=55))),
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=343,V=437,UL=44,VL=59))) ),
			)}
}