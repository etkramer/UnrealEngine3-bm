
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_ChainsawDuel_Follower extends GSM_InteractionPawnFollower_Base;

var ESpecialMove PrevSpecialMove;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	PrevSpecialMove = PrevMove;
	
	Super.SpecialMoveStarted(bForced, PrevMove);

	if( PCOwner != None && PCOwner.MyGearHUD != None )
	{
		PCOwner.MyGearHUD.SetActionInfo(AT_SpecialMove, Action, PawnOwner.bIsMirrored);
	}
}

function SpecialMoveEnded( ESpecialMove PrevMove, ESpecialMove NextMove )
{
	if( PawnOwner != None )
	{
		PawnOwner.ClearTimer( 'ShowHUDAction', self );
	}
	
	Super.SpecialMoveEnded( PrevMove, NextMove );
}

defaultproperties
{
	Action={(
		ActionName=SawDuel,
		IconAnimationSpeed=0.1f,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=83,V=0,UL=45,VL=32),	// mash B button
										  (Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=83,V=33,UL=45,VL=32))),
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=0,UL=83,VL=66),	// saw
										  (Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=68,UL=83,VL=66))) ),
		)}

	bDisablePOIs=TRUE
}