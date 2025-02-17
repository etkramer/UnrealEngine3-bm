/**
 * This is just a Faux SpecialMove which allows us to display ActionInfos on the HUD
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_EvadeFromCoverBase extends GearSpecialMove
	DependsOn(GearPawn);

/** GoW global macros */

protected function bool CanEvadeFromCover(ECoverType InCoverType)
{
	local GearPC	PC;

	// only allowed from in cover
	if( (PawnOwner.CoverType == CT_None) || (PawnOwner.CurrentLink == None) )
	{
		return FALSE;
	}

	// If this is a player
	if( PC != None )
	{
		// Make sure they are close enough to the edge to do a slip and allowed to slip in that direction
		if( ( PawnOwner.bIsMirrored && (!PawnOwner.IsAtLeftEdgeSlot() || PC.RemappedJoyRight > -PC.DeadZoneThreshold) )	||
			(!PawnOwner.bIsMirrored && (!PawnOwner.IsAtRightEdgeSlot() || PC.RemappedJoyRight < PC.DeadZoneThreshold) ) )
		{
			//`log( "FALSE 1" );
			return FALSE;
		}
	}
	else
	{
		// Otherwise, for AI, make sure this is an actual edge slot
		if( ( PawnOwner.bIsMirrored && ( !PawnOwner.CurrentLink.IsLeftEdgeSlot(PawnOwner.CurrentSlotIdx, TRUE))) ||
			(!PawnOwner.bIsMirrored && ( !PawnOwner.CurrentLink.IsRightEdgeSlot(PawnOwner.CurrentSlotIdx, TRUE))) )
		{
			//`log( "FALSE 2" );
			return FALSE;
		}
	}

	// make sure the player is pressing sideways
	PC = GearPC(PawnOwner.Controller);
	if( PC != None && PC.IsLocalPlayerController() )
	{
		if( PawnOwner.CoverType == InCoverType )
		{
			// must be pushing sideways
			if( Abs(PC.RemappedJoyUp) > Abs(PC.RemappedJoyRight) ||
				Abs(PC.RemappedJoyRight) < 0.3 )
			{
				//`log( "FALSE 3" );
				return FALSE;
			}
		}
		else
		{
			//`log( "FALSE 4" );
			// should never hit this
			return FALSE;
		}
	}

	// don't allow if carrying a heavy weapon
	if( PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		return FALSE;
	}

	//`log( "TRUE 0" );
	return TRUE;
}



defaultproperties
{
	//Standing evade 340,289 172x52
	Action={(
			 ActionName=EvadeFromCover,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43))), // A Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=0,V=127,UL=210,VL=107)))	),
			)}
}
