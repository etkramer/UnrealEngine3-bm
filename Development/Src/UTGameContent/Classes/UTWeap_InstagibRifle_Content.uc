/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class UTWeap_InstagibRifle_Content extends UTWeap_InstagibRifle;


defaultproperties
{
	// Weapon SkeletalMesh
	Begin Object Name=FirstPersonMesh
		SkeletalMesh=SkeletalMesh'WP_ShockRifle.Mesh.SK_WP_ShockRifle_1P'
		AnimSets(0)=AnimSet'WP_ShockRifle.Anim.K_WP_ShockRifle_1P_Base'
		Animations=MeshSequenceA
		Rotation=(Yaw=-16384)
		FOV=60.0
	End Object
	AttachmentClass=class'UTGameContent.UTAttachment_InstagibRifle'

	WeaponFireSnd[0]=SoundCue'A_Weapon_ShockRifle.Cue.A_Weapon_SR_InstagibFireCue'
	WeaponFireSnd[1]=SoundCue'A_Weapon_ShockRifle.Cue.A_Weapon_SR_InstagibFireCue'
	WeaponEquipSnd=SoundCue'A_Weapon_ShockRifle.Cue.A_Weapon_SR_RaiseCue'
	WeaponPutDownSnd=SoundCue'A_Weapon_ShockRifle.Cue.A_Weapon_SR_LowerCue'

	PickupSound=SoundCue'A_Pickups.Weapons.Cue.A_Pickup_Weapons_Shock_Cue'

	TeamSkins[0]=MaterialInterface'WP_ShockRifle.Materials.M_WP_ShockRifle_Instagib_Red'
	TeamSkins[1]=MaterialInterface'WP_ShockRifle.Materials.M_WP_ShockRifle_Instagib_Blue'
	TeamMuzzleFlashes[0]=ParticleSystem'WP_Shockrifle.Particles.P_Shockrifle_Instagib_MF_Red'
	TeamMuzzleFlashes[1]=ParticleSystem'WP_Shockrifle.Particles.P_Shockrifle_Instagib_MF_Blue'
}