/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAttachment_SniperRifle extends UTWeaponAttachment;

var ParticleSystem TracerTemplate;
var array<MaterialInterface> TeamSkins;

simulated function SpawnTracer(vector EffectLocation, vector HitLocation)
{
	local ParticleSystemComponent E;
	local vector Dir;

	Dir = HitLocation - EffectLocation;
	if ( VSizeSq(Dir) > 14400.0 )
	{
		E = WorldInfo.MyEmitterPool.SpawnEmitter(TracerTemplate, EffectLocation, rotator(Dir));
		E.SetVectorParameter('Sniper_Endpoint', HitLocation);
	}
}

simulated function FirstPersonFireEffects(Weapon PawnWeapon, vector HitLocation)
{
	Super.FirstPersonFireEffects(PawnWeapon, HitLocation);

	SpawnTracer(UTWeapon(PawnWeapon).GetEffectLocation(), HitLocation);
}

simulated function ThirdPersonFireEffects(vector HitLocation)
{
	Super.ThirdPersonFireEffects(HitLocation);

	SpawnTracer(GetEffectLocation(), HitLocation);
}

simulated function SetSkin(Material NewMaterial)
{
	local int TeamIndex;

	if ( NewMaterial == None ) 	// Clear the materials
	{
		TeamIndex = Instigator.GetTeamNum();
		if ( TeamIndex == 255 )
			TeamIndex = 0;
		Mesh.SetMaterial(0,TeamSkins[TeamIndex]);
	}
	else
	{
		Super.SetSkin(NewMaterial);
	}
}


defaultproperties
{
	bMakeSplash=true

	// Weapon SkeletalMesh
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'WP_SniperRifle.Mesh.SK_WP_SniperRifle_3P_Mid'
		MaxDrawDistance=5000
	End Object

	DefaultImpactEffect=(Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(0)=(MaterialType=Dirt,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(1)=(MaterialType=Gravel,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(2)=(MaterialType=Sand,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(3)=(MaterialType=Dirt_Wet,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactMud_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(4)=(MaterialType=Energy,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactEnergy_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(5)=(MaterialType=WorldBoundary,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactEnergy_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(6)=(MaterialType=Flesh,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(7)=(MaterialType=Flesh_Human,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(8)=(MaterialType=Kraal,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(9)=(MaterialType=Necris,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(10)=(MaterialType=Robot,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(11)=(MaterialType=Foliage,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactFoliage_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(12)=(MaterialType=Glass,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactGlass_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(13)=(MaterialType=Liquid,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(14)=(MaterialType=Water,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(15)=(MaterialType=ShallowWater,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(16)=(MaterialType=Lava,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(17)=(MaterialType=Slime,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(18)=(MaterialType=Metal,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(19)=(MaterialType=Snow,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactSnow_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(20)=(MaterialType=Wood,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactWood_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(21)=(MaterialType=NecrisVehicle,Sound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Surface_Impact',DecalMaterials=(),DecalWidth=16.0,DecalHeight=16.0)

	TracerTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_Beam'
	BulletWhip=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_AmmoWhip_Cue'

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_3P_MuzzleFlash'
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'UTGame.UTRocketMuzzleFlashLight'
	WeaponClass=class'UTWeap_SniperRifle'

	TeamSkins[0]=MaterialInterface'WP_SniperRifle.Materials.M_WP_SniperRifle'
	TeamSkins[1]=MaterialInterface'WP_SniperRifle.Materials.M_WP_SniperRifle_Blue'

}
