/**
 * "Untargeted" flavor of the Mortar Projectile, for mortar shots
 * from the hip.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_MortarUntargeted extends GearProj_MortarUntargetedBase
	config(Weapon);

defaultproperties
{
	TrailTemplate=ParticleSystem'COG_Mortar.Effects.P_Mortar_smoke_trail_HipShot'

	InFlightSoundTemplate=None
	IncomingSound=None
	LaunchSound=None
	BounceSound=SoundCue'Weapon_Mortar.Sounds.MortarFireShellBounceCue'

	Begin Object Name=StaticMeshComponent0
    	StaticMesh=StaticMesh'COG_Mortar.Mesh.COG_HW_Mortar_Shell_SM'
    End Object

	// explosion point light
    Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=800.000000
		Brightness=1000.000000
		LightColor=(B=60,G=107,R=249,A=255)
		Translation=(X=16)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
    End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_MortarUntargeted'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'COG_Mortar.Effects.P_Mortar_AIR_Burst'
		ExplosionSound=SoundCue'Weapon_Mortar.Sounds.MortarImpactExplosionCue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
		ExploShakeInnerRadius=1000
		ExploShakeOuterRadius=3000

		FractureMeshRadius=220.0
		FracturePartVel=500.0

		DamageDelay=0.3f			// delay the damage slightly so that clients see it before being killed by it

		bAllowPerMaterialFX=TRUE

		bUsePerMaterialFogVolume=TRUE
	End Object
	ExplosionTemplate=ExploTemplate0

	BombletClass=class'GearProj_ClusterMortarBomblet'

	HotMaterial=Material'COG_Mortar.Materials.COG_HW_Mortar_Shell_Heat'

}
