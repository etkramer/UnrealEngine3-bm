
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_ReviveTeammate extends GearSpecialMove
	config(Pawn);

/** Pawn To Revive */
var		GearPawn				PawnToRevive;

/** Weapon pickup body stance animation */
var()	GearPawn.BodyStance	BS_Pickup;

/** Revive Search radius */
var()	config	float	ReviveSearchRadius;

protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.IsInCover() || PawnOwner.IsDBNO() )
	{
		return FALSE;
	}

	return FindPawnToRevive();
}

private function bool FindPawnToRevive()
{
	local bool		bResult;
	local GearPawn	FoundPawn;

	foreach PawnOwner.VisibleCollidingActors( class'GearPawn', FoundPawn, ReviveSearchRadius, PawnOwner.Location, TRUE )
	{
		// Cannot revive hostages.
		if( FoundPawn.bCanRecoverFromDBNO && FoundPawn.IsDBNO() && FoundPawn.IsDoingSpecialMove(SM_DBNO) && (PawnOwner != FoundPawn) && PawnOwner.IsSameTeam(FoundPawn) && !FoundPawn.IsAHostage() )
		{
			PawnToRevive = FoundPawn;
			bResult = TRUE;
			break;
		}
	}

	return bResult;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	PawnOwner.InterpolatePawnRotation();

	// Revive Pawn we found on the server.
	if( PawnOwner.Role == Role_Authority )
	{
		if( PCOwner != None )
		{
			PCOwner.ServerReviveTeamMate(PawnToRevive);
		}
		else
		if( AIOwner != None && FindPawnToRevive() )
		{
			AIOwner.ReviveTeamMate(PawnToRevive);
		}
	}

	AlignToActor = PawnToRevive;

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Pickup, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE );

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Pickup, TRUE);

	// Trigger the revive delegates in the GearPC
	if ( PCOwner != None )
	{
		PCOwner.TriggerGearEventDelegates( eGED_Revive );
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.InterpolatePawnRotation();

	PawnOwner.BS_SetAnimEndNotify(BS_Pickup, FALSE);
}


defaultproperties
{
	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE

	BS_Pickup=(AnimName[BS_FullBody]="AR_Idle_Ready_Pickup")

	Action={(
			ActionName=ReviveTeammate,
			ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=175,V=239,UL=136,VL=75)))	),
	)}
}

