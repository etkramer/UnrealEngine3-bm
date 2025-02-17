/** 
 * GearWeap_InkGrenade
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_InkGrenade extends GearWeap_GrenadeBase;

/** projectile behavior upgrade */
var()	TWeaponParam	PRM_Behavior;

var MeshComponent PickupMeshComp;


/** 
 * ThrowGrenade Projectile 
 */
function GearProj_Grenade ThrowGrenade()
{
	if ( WorldInfo.NetMode != NM_Client )
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_ThrowingInkGrenade, Instigator);
	}
	return super.ThrowGrenade();
}

static function PrimitiveComponent GetPickupMesh(PickupFactory Factory)
{
	return default.PickupMeshComp;
}


/** This will return the COG Emissive Color for this weapon for this map **/
// inks are always this color
static simulated function LinearColor GetWeaponEmisColor_COG( WorldInfo TheWorldInfo, EWeaponClass WeapID )
{
	local LinearColor LC;

	LC.R = 3;
	LC.G = 4;
	LC.B = 8;
	LC.A = 1;

	return LC;
}

/** This will return the Locust Emissive Color for this weapon for this map **/
// inks are always this color
static simulated function LinearColor GetWeaponEmisColor_Locust( WorldInfo TheWorldInfo, EWeaponClass WeapID )
{
	local LinearColor LC;

	LC.R = 3;
	LC.G = 4;
	LC.B = 8;
	LC.A = 1;

	return LC;
}



defaultproperties
{
	DamageTypeClassForUI=class'GDT_InkGrenade'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=0,V=97,UL=143,VL=46)
	AnnexWeaponIcon=(U=128,V=0,UL=128,VL=28)

	// weapon parameters
	PRM_Behavior=(Value=(0,1,2),Name="Level")

	WeaponProjectiles(0)=class'GearProj_InkGrenade'

	InstantHitDamageTypes(0)=class'GDT_InkGrenade'
	AmmoTypeClass=class'GearAmmoType_GrenadeInk'

	ThrowGrenadeSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeFireCue'
	GrenadeSwingSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeSwingCue'

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'Locust_PoisonGrenade.Locust_Poison_Grenade'
		PhysicsAsset=PhysicsAsset'Locust_PoisonGrenade.Poison_Grenade_Physics'
		AnimTreeTemplate=AnimTree'COG_Bolo_Grenade.Animations.AT_BoloGrenade'
		AnimSets(0)=AnimSet'COG_Bolo_Grenade.GrenadeAnims'
    End Object
	Mesh=WeaponMesh

	Begin Object Class=SkeletalMeshComponent Name=WeaponPickMesh
		SkeletalMesh=SkeletalMesh'Locust_PoisonGrenade.Locust_Poison_Grenade'
		PhysicsAsset=PhysicsAsset'Locust_PoisonGrenade.Poison_Grenade_Physics'
		Rotation=(Pitch=16384)
		Translation=(Z=-20)
    End Object
	DroppedPickupMesh=WeaponPickMesh

	Begin Object Class=StaticMeshComponent Name=PickupMesh0
		StaticMesh=StaticMesh'Locust_PoisonGrenade.Meshes.Locust_PoisonGrenade_Pickup1_SM'
		Translation=(Z=-20)
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
	End Object
	PickupMeshComp=PickupMesh0

	LC_EmisDefaultCOG=(R=1.0,G=3.0,B=8.0,A=1.0)
	LC_EmisDefaultLocust=(R=15.0,G=2.0,B=0.0,A=1.0)

	WeaponID=WC_InkGrenade

	WeaponEquipSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeUnStowCue'
	WeaponDeEquipSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeStowCue'

	SpinningParticleEffectTemplate=ParticleSystem'Weap_Ink_Grenade.Effects.P_Weap_Ink_Grenade_PreThrow'
}
