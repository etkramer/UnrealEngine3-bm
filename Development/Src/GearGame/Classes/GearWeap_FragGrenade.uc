
/** 
 * GearWeap_FragGrenade
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_FragGrenade extends GearWeap_GrenadeBase;

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
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_ThrowingFragGrenade, Instigator);
	}
	return super.ThrowGrenade();
}

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.GrenadeFrag;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.GrenadeFrag;
}
*/

static function PrimitiveComponent GetPickupMesh(PickupFactory Factory)
{
	return default.PickupMeshComp;
}


/** This will return the COG Emissive Color for this weapon for this map **/
// frags are always this color
static simulated function LinearColor GetWeaponEmisColor_COG( WorldInfo TheWorldInfo, EWeaponClass WeapID )
{
	local LinearColor LC;

	LC.R = 15;
	LC.G = 2;
	LC.B = 0;
	LC.A = 0;

	return LC;
}

/** This will return the Locust Emissive Color for this weapon for this map **/
// frags are always this color
static simulated function LinearColor GetWeaponEmisColor_Locust( WorldInfo TheWorldInfo, EWeaponClass WeapID )
{
	local LinearColor LC;

	LC.R = 15;
	LC.G = 2;
	LC.B = 0;
	LC.A = 0;

	return LC;
}



defaultproperties
{
	DamageTypeClassForUI=class'GDT_FragGrenade'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=222,V=193,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=231,UL=128,VL=29)

	// weapon parameters
	PRM_Behavior=(Value=(0,1,2),Name="Level")

	WeaponProjectiles(0)=class'GearProj_FragGrenade'

	InstantHitDamageTypes(0)=class'GDT_FragGrenade'
	AmmoTypeClass=class'GearAmmoType_GrenadeFrag'

	ThrowGrenadeSound=SoundCue'Weapon_Grenade.Firing.GrenadeThrowCue'

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'COG_Bolo_Grenade.Frag_Grenade'
		PhysicsAsset=PhysicsAsset'COG_Bolo_Grenade.Frag_Grenade_Physics'
		AnimTreeTemplate=AnimTree'COG_Bolo_Grenade.Animations.AT_BoloGrenade'
		AnimSets(0)=AnimSet'COG_Bolo_Grenade.GrenadeAnims'
    End Object
	Mesh=WeaponMesh

	Begin Object Class=SkeletalMeshComponent Name=WeaponPickMesh
		SkeletalMesh=SkeletalMesh'COG_Bolo_Grenade.Frag_Grenade'
		PhysicsAsset=PhysicsAsset'COG_Bolo_Grenade.Frag_Grenade_Physics'
		Rotation=(Pitch=16384)
		Translation=(Z=-20)
    End Object

// 	Begin Object Class=StaticMeshComponent Name=GrenadeMesh1
// 	    StaticMesh=StaticMesh'COG_Bolo_Grenade.Locust_BoloGrenade_Pickup1';
//    	    CollideActors=FALSE
// 		bCastDynamicShadow=TRUE
// 		Scale=1.0f
// 		Translation=(Z=-20)
// 	End Object
// 
// 	Begin Object Class=StaticMeshComponent Name=GrenadeMesh2
// 	    StaticMesh=StaticMesh'COG_Bolo_Grenade.Locust_BoloGrenade_Pickup2';
// 		CollideActors=FALSE
// 		bCastDynamicShadow=TRUE
// 		Scale=1.0f
// 		Translation=(Z=-20)
// 	End Object
// 
// 	Begin Object Class=StaticMeshComponent Name=GrenadeMesh3
// 	    StaticMesh=StaticMesh'COG_Bolo_Grenade.Locust_BoloGrenade_Pickup3';
// 		CollideActors=FALSE
// 		bCastDynamicShadow=TRUE
// 		Scale=1.0f
// 		Translation=(Z=-20)
// 	End Object

	DroppedPickupMesh=WeaponPickMesh

	Begin Object Class=StaticMeshComponent Name=PickupMesh0
		StaticMesh=StaticMesh'COG_Bolo_Grenade.Locust_BoloGrenade_Pickup2'
		Translation=(Z=-20)
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
	End Object
	PickupMeshComp=PickupMesh0

	LC_EmisDefaultCOG=(R=1.0,G=3.0,B=8.0,A=1.0)
	LC_EmisDefaultLocust=(R=15.0,G=2.0,B=0.0,A=1.0)

	WeaponID=WC_FragGrenade
}
