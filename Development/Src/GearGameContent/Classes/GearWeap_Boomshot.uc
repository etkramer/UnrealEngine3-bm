/**
 * Boomshot
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_Boomshot extends GearWeap_BoomshotBase
	config(Weapon);

defaultproperties
{
	AIProjectileClass=class'GearProj_BoomshotAI'

	WeaponProjectiles(0)=class'GearProj_Boomshot'
	WeaponFireTypes(0)=EWFT_Projectile
	InstantHitDamageTypes(0)=class'GDT_Boomshot'
	AmmoTypeClass=class'GearAmmoType_Boomer'

	// Weapon anim set
	CustomAnimSets(0)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Boomshot'

	FireSound=SoundCue'Weapon_Boomer.Firing.BoomerFireCue'

	WeaponDeEquipSound=SoundCue'Weapon_Shotgun.Actions.ShotgunRaiseCue'
	WeaponEquipSound=SoundCue'Weapon_Shotgun.Actions.ShotgunLowerCue'

	PickupSound=SoundCue'Weapon_Shotgun.Actions.ShotgunPickupCue'
	WeaponDropSound=SoundCue'Weapon_Sniper.Actions.CogSniperDropCue'

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_MF'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	// Weapon Mesh Transform
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'Locust_Boomshot.Mesh.Locust_Boomshot'
		AnimTreeTemplate=AnimTree'Locust_Boomshot.AT_Boomshot'
		PhysicsAsset=PhysicsAsset'Locust_Boomshot.Mesh.Locust_Boomshot_Physics'
		AnimSets(0)=AnimSet'Locust_Boomshot.Animation.Animset_Locust_Boomshot'
		Rotation=(Pitch=1092)	//+6d
		Translation=(X=+8)
		Scale=0.75f
	End Object

	Begin Object Class=StaticMeshComponent Name=MagazineMesh0
		StaticMesh=StaticMesh'Locust_Boomshot.Mesh.Locust_Boomshot_Magazine'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
	End Object
	MagazineMesh=MagazineMesh0

	DamageTypeClassForUI=class'GDT_Boomshot'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=367,V=146,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=144,UL=128,VL=39)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Boomshot'

	HUDDrawData			= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)
	HUDDrawDataSuper	= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)

	MeleeImpactSound=SoundCue'Weapon_Boomer.Reloads.BoomerHitCue';

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1
}
