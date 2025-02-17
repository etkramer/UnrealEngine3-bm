/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustKantus extends GearPawn_LocustKantusBase
	config(Pawn);

simulated event ReplicatedEvent(name VarName)
{
	local GearWeap_InkGrenadeKantus Grenade;

	if (VarName == nameof(ThrowCount))
	{
		Grenade = GearWeap_InkGrenadeKantus(RemoteClientGetInventoryByClass(class'GearWeap_InkGrenadeKantus'));
		if (Grenade != None)
		{
			// attach grenade and start it spinning
			Grenade.AttachWeaponTo(Mesh, LeftHandSocketName);
			Grenade.StartGrenadeSpin();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function ChainSawGore()
{
	BreakConstraint( vect(100,0,0), vect(0,10,0), 'b_MF_Spine_03' );
	BreakConstraint( vect(0,100,0), vect(0,0,10), 'b_MF_UpperArm_R' );
}


/** Play HeadShot special death */
simulated function PlayHeadShotDeath()
{
	Super.PlayHeadShotDeath();

	if( HeadShotNeckGoreAttachment != None )
	{
		HeadShotNeckGoreAttachment.SetScale( 1.25f );
	}
}
function DropExtraWeapons( class<DamageType> DamageType )
{
	if( bAllowInventoryDrops == TRUE )
	{
		if(FRand() > 0.50f )
		{
			DropExtraWeapons_Worker( DamageType, class'GearGameContent.GearWeap_InkGrenade' );
		}

	}
}

defaultproperties
{
	DrawScale=1.25
	MPHeadRadius=14
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_LocustBurstPistol_Kantus'
	DefaultInventory(1)=class'GearGameContent.GearWeap_InkGrenadeKantus'

	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=0,V=127,UL=62,VL=62)

	ControllerClass=class'GearAI_Kantus'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustKantus'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustKantus"
	FAS_ChatterNames.Add("Locust_Kantus.FaceFX.Locust_Kantus_FaceFX_Chatter")
	FAS_ChatterNames.Add("Locust_Kantus.FaceFX.Locust_Kantus_FaceFX_Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Kantus.FaceFX.Locust_Kantus_FaceFX_Efforts'
	FAS_Efforts(1)=FaceFXAnimSet'Locust_Kantus.FaceFX.Locust_Kantus_FaceFX_Efforts_Dup'


	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Kantus.Locust_Kantus_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Kantus.PhysicsAsset.Locust_Kantus_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L")

	MeshTranslationNudgeOffset=-12.f
	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Kantus.Locust_Kantus'
		PhysicsAsset=PhysicsAsset'Locust_Kantus.PhysicsAsset.Locust_Kantus_Physics'
		AnimSets.Add(AnimSet'Locust_Kantus.Kantus_animset')
		bEnableFullAnimWeightBodies=TRUE
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0035.000000
		CollisionHeight=+080.000000
	End Object

	PickupFocusBoneName=b_MF_Weapon_L
	PickupFocusBoneNameKickup=b_MF_Weapon_R

	SpecialMoveClasses(SM_MidLvlJumpOver)			=class'GSM_MantleOverLocust'
	SpecialMoveClasses(GSM_Kantus_KnockDownScream)	=class'GSM_Kantus_KnockDownScream'
	SpecialMoveClasses(GSM_Kantus_ReviveScream)		=class'GSM_Kantus_ReviveScream'
	SpecialMoveClasses(GSM_Kantus_SummoningScream)	=class'GSM_Kantus_SummoningScream'

	SightBoneName=None

	PeripheralVision=-1

	//Kantus say “Succumb”
	KantusAttackTelegraphDialogue(0)=SoundCue'Locust_Kantus_Chatter_Cue.DupedRefsForCode.KantusChatter_Succumb_Loud01Cue_Code'
	KantusAttackTelegraphDialogue(1)=SoundCue'Locust_Kantus_Chatter_Cue.AttackingEnemy.KantusChatter_Succumb_Scream01Cue'
	KantusAttackTelegraphDialogue(2)=SoundCue'Locust_Kantus_Chatter_Cue.AttackingEnemy.KantusChatter_Succumb_Medium03Cue'
	KantusAttackTelegraphDialogue(3)=SoundCue'Locust_Kantus_Chatter_Cue.AttackingEnemy.KantusChatter_Succumb_Medium01Cue'

	ReviveParticleSystem=ParticleSystem'Locust_Kantus.Effects.P_Kantus_Revive_Glow'
	MinionClasses(0)=class'Gearpawn_LocustTicker'

	// Don't let AI Kantus take meatsheild b/c animations don't line up
	PawnToPawnInteractionList.Empty
}
