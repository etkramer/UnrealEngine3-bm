/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 *  Base Gear COG Gear Character class
 */
class GearPawn_COGGear extends GearPawn_Infantry
	config(Pawn)
	native(Pawn);


simulated function ChainSawGore()
{
	BreakConstraint( vect(100,0,0), vect(0,10,0), 'b_MF_Spine_03' );
	BreakConstraint( vect(0,100,0), vect(0,0,10), 'b_MF_UpperArm_R' );
}


simulated function OnDestroy(SeqAct_Destroy Action)
{
	if (PlayerController(Controller) != None &&
		WorldInfo.NetMode != NM_Standalone)
	{
		`Warn("Not destroying"@self);
	}
	else
	{
		Super.OnDestroy(Action);
	}
}


simulated function PlayKickSound()
{
	PlaySound(SoundCue'Foley_BodyMoves.BodyMoves.SquadLegKick_Cue');
}

simulated function bool CanBeSpecialMeleeAttacked(GearPawn Attacker)
{
	// don't allow friendly AI COGs to be executed unless playing on a high difficulty
	// and the AI has been left DBNO for a while
	if (!Super.CanBeSpecialMeleeAttacked(Attacker))
	{
		return false;
	}
	else if (!IsDBNO() || GearGameSP_Base(WorldInfo.Game) == None)
	{
		return true;
	}
	else if (WorldInfo.TimeSeconds - TimeStampEnteredRevivingState < 10.0)
	{
		return false;
	}
	else if (IsHumanControlled())
	{
		return GearPRI(PlayerReplicationInfo).Difficulty.default.bHumanCOGCanBeExecuted;
	}
	else
	{
		return false;
	}
}


simulated function byte GetMPWeaponColorBasedOnClass()
{
	return 0;
}



defaultproperties
{
	DefaultInventory(0)=class'GearGame.GearWeap_AssaultRifle'
	HelmetType=class'Item_Helmet_None'

	// HeadShot neck attachment
	Begin Object Class=StaticMeshComponent Name=GearHeadShotMesh1
	    StaticMesh=StaticMesh'COG_Gore.COG_Headshot_Gore'
		CollideActors=FALSE
		LightEnvironment=MyLightEnvironment
	End Object
	HeadShotNeckGoreAttachment=GearHeadShotMesh1
	bCanPlayHeadShotDeath=TRUE

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0034.000000
		CollisionHeight=+0072.000000
	End Object

	CrouchHeight=+45.0
	CrouchRadius=+34.0

	SpecialMoveClasses(SM_MidLvlJumpOver)	=class'GSM_MantleOverGears'
	DeadBodyImpactSound=SoundCue'Foley_BodyMoves.BodyMoves.BodyFall_CogCue'
}
