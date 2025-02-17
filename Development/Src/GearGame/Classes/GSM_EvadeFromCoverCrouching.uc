/**
 * This is just a Faux SpecialMove which allows us to display ActionInfos on the HUD 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_EvadeFromCoverCrouching extends GSM_EvadeFromCoverBase
	DependsOn(GearPawn);

/** GoW global macros */

protected function bool InternalCanDoSpecialMove()
{
	return CanEvadeFromCover(CT_MidLevel);
}

defaultproperties
{
	//Crouch-evade 0,445 138x39
	Action={(
			 ActionName=EvadeFromCover,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43))), // A Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=242,V=57,UL=235,VL=82)))	),
			)}
}