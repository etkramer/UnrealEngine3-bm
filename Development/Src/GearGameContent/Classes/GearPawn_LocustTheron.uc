
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustTheron extends GearPawn_LocustBase
	config(Pawn);

/** This is needed to have the cooker correctly cook things that are currently DLO'd from config files **/
var class<Inventory> DefaultWeapon;

/** number of normal bullets to know off the helmet**/
var config int NumBulletsToKnockHelmetOff;

/** number of sniper bullets to knock off the helmet**/
var config int NumSniperBulletsToKnockHelmetOff;

/** private to keep track of the helmet shots **/
var private int NumTimeShotInHelmet;
var private int NumTimesSniperShotInHelmet;


/** theron helmets stay on for two hits from sniper and multiple hits from normal bullets **/
simulated function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced )
{
	local class<GearDamageType> GearDamageType;

	GearDamageType = class<GearDamageType>(DamageType);

	if( GearDamageType.default.bAllowHeadShotGib )
	{
		NumTimesSniperShotInHelmet++;
	}
	else
	{
		NumTimeShotInHelmet++;
	}


	if( ( NumTimeShotInHelmet >= NumBulletsToKnockHelmetOff )
		|| ( NumTimesSniperShotInHelmet >= NumSniperBulletsToKnockHelmetOff )
		|| ( bForced == TRUE )
		)
	{
		Super.RemoveAndSpawnAHelmet( ApplyImpulse, DamageType, bForced );
	}
}



simulated function PlayKickSound()
{
	PlaySound(SoundCue'Foley_BodyMoves.BodyMoves.HeavyLocustLegKick_Cue');
}

simulated function ChainSawGore()
{
	BreakConstraint( vect(100,0,0), vect(0,10,0), 'b_MF_Spine_03' );
	BreakConstraint( vect(0,100,0), vect(0,0,10), 'b_MF_UpperArm_R' );
}

// therons drop a boltok 50% of the time
function DropExtraWeapons( class<DamageType> DamageType )
{
	if( ( bAllowInventoryDrops == TRUE ) && ( FRand() > 0.50f ) )
	{
		DropExtraWeapons_Worker( DamageType, class'GearGame.GearWeap_LocustPistol' );
	}
}

simulated function vector GetWeaponAimIKPositionFix()
{
	return Super.GetWeaponAimIKPositionFix() + vect(0,4,-2);
}

defaultproperties
{
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_Bow'
	DefaultInventory(1)=class'GearGame.GearWeap_FragGrenade'
	DefaultInventory(2)=class'GearGame.GearWeap_AssaultRifle'
	DefaultInventory(3)=class'GearGame.GearWeap_LocustPistol'

	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=245,V=0,UL=48,VL=63)
	HelmetType=class'Item_Helmet_LocustTheronRandom'

	ControllerClass=class'GearAI_Locust'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustTheron'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustTheron"
	FAS_ChatterNames.Add("Locust_Theron_Guard.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Theron_Guard.FaceFX.Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Theron_Guard.FaceFX.theron_Guard_FaceFX_Efforts'
	FAS_Efforts(1)=FaceFXAnimSet'Locust_Theron_Guard.FaceFX.theron_Guard_FaceFX_Efforts_Dup'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Locust_Theron_Guard_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
	GoreBreakableJoints=("b_MF_Forearm_L","b_MF_UpperArm_L","b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Spine_03","b_MF_Head","b_MF_Face","b_MF_Calf_L","b_MF_Calf_R","b_MF_Foot_R")
	GoreMorphSets.Add(MorphTargetSet'Locust_Theron_Guard.Theron_Guard_Gore_Morphs')
	HostageHealthBuckets=("b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Calf_R")

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Mesh.Locust_Theron_Guard_Cloth'
		PhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
		Translation=(Z=-80)
		bEnableFullAnimWeightBodies=TRUE
		MorphSets.Add(MorphTargetSet'Locust_Theron_Guard.Mesh.Theron_Guard_Game_Morphs')
	End Object

	PickupFocusBoneName=b_MF_Weapon_L
	PickupFocusBoneNameKickup=b_MF_Weapon_R

	DefaultWeapon=class'GearWeap_Bow'

	SpecialMoveClasses(SM_MidLvlJumpOver)	=class'GSM_MantleOverLocust'

	NoticedGUDSPriority=103
	NoticedGUDSEvent=GUDEvent_NoticedTheron

	MeatShieldMorphTargetName="Meatshield_Morph"
}

