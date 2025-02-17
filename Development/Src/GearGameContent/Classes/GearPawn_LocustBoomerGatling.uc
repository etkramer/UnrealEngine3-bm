
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBoomerGatling extends GearPawn_LocustBoomer
	config(Pawn);

var				BodyStance		BS_AttackTelegraphAnim;
var repnotify	byte			AttackTelegraphCount;

replication
{
	if( Role == ROLE_Authority )
		AttackTelegraphCount;
}

simulated event ReplicatedEvent(name VarName)
{
	Super.ReplicatedEvent(VarName);

	if( VarName == 'AttackTelegraphCount' )
	{
		TelegraphAttack();
	}
}

simulated function TelegraphAttack()
{
	// Replicate to clients
	if( Role == ROLE_Authority && WorldInfo.NetMode != NM_Standalone )
	{
		AttackTelegraphCount++;
	}

	if( FullBodyNode != None && !FullBodyNode.bIsPlayingCustomAnim && !IsReloadingWeapon() )
	{
		// Play attack telegraph additive animation.
		BS_Play(BS_AttackTelegraphAnim, 1.f, 0.2f, 0.2f);
	}

	super.TelegraphAttack();
}

defaultproperties
{
	BS_AttackTelegraphAnim=(AnimName[BS_FullBody]="ADD_HW_Gatling_Boomer_GetReady")

	HelmetType=class'Item_Helmet_LocustBoomerGatling'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustBoomerB'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustBoomerB"
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer2Chatter")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer2Chatter_Dup")

	DefaultWeapon=class'GearGameContent.GearWeap_Boomer_Minigun'
	DefaultInventory(0)=class'GearGameContent.GearWeap_Boomer_Minigun'

	Begin Object Name=GearPawnMesh
		AnimTreeTemplate=AnimTree'Locust_Boomer.Animations.AT_LocustBoomer_Gatling'
		AnimSets.Add(AnimSet'Locust_Boomer.Animations.AnimSetBoomer_CamSkel_Gatling')
	End Object

	// gatling boomer (aka Grinder) is boomerB and boomerA, says "Grind"
	BoomerAttackTelegraphDialogue.Empty
	BoomerAttackTelegraphDialogue(0)=SoundCue'Locust_Boomer1_Chatter_Cue.AttackingEnemy.Boomer1Chatter_Grind_Loud04Cue'
	BoomerAttackTelegraphDialogue(1)=SoundCue'Locust_Boomer2_Chatter_Cue.AttackingEnemy.Boomer2Chatter_Grind_Loud01Cue'

	BoomerAttackChuckleDialogue.Empty
	BoomerAttackChuckleDialogue(0)=SoundCue'Locust_Boomer1_Chatter_Cue.DupedRefsForCode.Boomer1Chatter_Laugh_Medium09Cue_Code'
	BoomerAttackChuckleDialogue(1)=SoundCue'Locust_Boomer2_Chatter_Cue.KilledEnemy.Boomer2Chatter_Laugh_Loud02Cue'
	BoomerAttackChuckleDialogue(2)=SoundCue'Locust_Boomer2_Chatter_Cue.DupedRefsForCode.Boomer2Chatter_Laugh_Medium09Cue_Code'
	BoomerAttackChuckleDialogue(3)=SoundCue'Locust_Boomer2_Chatter_Cue.DupedRefsForCode.Boomer2Chatter_Laugh_Medium10Cue_Code'
	BoomerAttackChuckleChance=0.5f

	NoticedGUDSEvent=GUDEvent_NoticedGrinder
	bPlayWeaponFireAnim=false
}
