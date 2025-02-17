/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTWeap_Translocator_Content extends UTWeap_Translocator
	HideDropDown;



simulated function WeaponEmpty()
{
	local AnimNodeSequence ANS;
	super.WeaponEmpty();
	WeaponIdleAnims[0] = EmptyIdleAnim;
	ANS = GetWeaponAnimNodeSeq();
	if(ANS != none && ANS.AnimSeq != none && ANS.AnimSeq.SequenceName == EmptyIdleAnim)
	{
		PlayWeaponAnimation(WeaponIdleAnims[0],0.001);
	}
}

simulated function ReAddAmmo()
{
	local AnimNodeSequence ANS;
	super.ReAddAmmo();
	if(AmmoCount > 0 && WeaponIdleAnims[0] == EmptyIdleAnim && (TransDisc == None || TransDisc.bDeleteMe))
	{
		WeaponIdleAnims[0] = default.WeaponIdleAnims[0];
		ANS = GetWeaponAnimNodeSeq();
		if(ANS != none && ANS.AnimSeq != none && ANS.AnimSeq.SequenceName == EmptyIdleAnim)
		{
			PlayWeaponAnimation(WeaponIdleAnims[0],0.001);
		}
	}
}

defaultproperties
{
	Begin Object class=AnimNodeSequence Name=MeshSequenceA
	End Object

	Begin Object Name=FirstPersonMesh
		SkeletalMesh=SkeletalMesh'WP_Translocator.Mesh.SK_WP_Translocator_1P'
		PhysicsAsset=None
		AnimSets(0)=AnimSet'WP_Translocator.Anims.K_WP_Translocator_1P_Base'
		Materials(0)=MaterialInterface'WP_Translocator.Materials.M_Gun_Ark'
		Materials(1)=MaterialInterface'WP_Translocator.Materials.M_WP_Translocator_1P'
		Animations=MeshSequenceA
		Scale=1
		FOV=55.0
	End Object
	AttachmentClass=class'UTGameContent.UTAttachment_Translocator'

	SkinColors(0)=(R=3.4,G=0.5,B=0.1)
	SkinColors(1)=(R=0.2,G=0.5,B=3.4)

	TeamSkins(0)=MaterialInterface'WP_Translocator.Materials.M_WP_Translocator_1P'
	TeamSkins(1)=MaterialInterface'WP_Translocator.Materials.M_WP_Translocator_1PBlue'
	TeamSkins(2)=none
	TeamSkins(3)=none

	EmptyPutDownAnim=weaponputdownempty
	EmptyEquipAnim=weaponequipempty

	Begin Object Name=PickupMesh
		SkeletalMesh=SkeletalMesh'WP_Translocator.Mesh.SK_WP_Translocator_3p_Mid'
	End Object

	WeaponFireSnd[0]=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_Fire_Cue'
	WeaponFireSnd[1]=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_Teleport_Cue'
	WeaponEquipSnd=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_Raise_Cue'
	WeaponPutDownSnd=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_Lower_Cue'
	TransRecalledSound=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_Recall_Cue'

	WeaponFireTypes(0)=EWFT_Projectile
 	WeaponFireTypes(1)=EWFT_Custom
	WeaponProjectiles(0)=class'UTProj_TransDisc_ContentRed'
	WeaponProjectiles(1)=class'UTProj_TransDisc_ContentBlue'
	FiringStatesArray(1)=RecallFire

	ArmsAnimSet=AnimSet'WP_Translocator.Anims.K_WP_Translocator_1P_Arms'

	EmptyIdleAnim=WeaponIdleEmpty
	WeaponFireAnim(0)=WeaponFire
	WeaponFireAnim(1)=none
	ArmFireAnim(0)=WeaponFire
	ArmFireAnim(1)=none

	FailedTranslocationDamageClass=class'UTDmgType_FailedTranslocation'

	CrossHairCoordinates=(U=0,V=0,UL=64,VL=64)
	IconCoordinates=(U=600,V=461,UL=122,VL=54)

	Begin Object Class=ParticleSystemComponent Name=DiskInEffect
		Template=ParticleSystem'WP_Translocator.Particles.P_WP_Translocator_idle'
		DepthPriorityGroup=SDPG_Foreground
		SecondsBeforeInactive=1.0f
	End Object
	DiskEffect=DiskInEffect
	Components.Add(DiskInEffect)
	MuzzleFlashSocket=MuzzleFlash
	DisruptionDeath=SoundCue'A_Weapon_Translocator.Translocator.A_Weapon_Translocator_DisruptedTeleport_Cue'
}
