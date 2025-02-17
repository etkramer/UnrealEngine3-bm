/**
 * Mortar Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_ClusterMortarBomblet extends GearProj_ClusterMortarBombletBase
	config(Weapon);

defaultproperties
{
	TrailTemplate=ParticleSystem'COG_Mortar.Effects.P_Mortar_Bomblets_Trail'

    Begin Object Name=StaticMeshComponent0
    	StaticMesh=StaticMesh'Locust_Boomshot.locust_boomshot_ammo2'
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
	    MyDamageType=class'GDT_Mortar'
	    MomentumTransferScale=1.f	// Scale momentum defined in DamageType

	    ParticleEmitterTemplate=ParticleSystem'COG_Frag_Grenade.Effects.P_COG_Frag_Grenade_Ground_Explo'
	    ExplosionSound=SoundCue'Weapon_Mortar.Sounds.MortarImpactBomletExplodeCue'
	    ExploLight=ExploLight0
	    ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		
		ExploAnimShake=(AnimBlendInTime=0.1f,Anim=CameraAnim'Effects_Camera.Explosions.CA_Mortar_Explo')
		ExploShakeInnerRadius=650
		ExploShakeOuterRadius=1500

	    FractureMeshRadius=100.0
	    FracturePartVel=500.0

	    DamageDelay=0.5f

	    bAllowPerMaterialFX=TRUE
		bAllowTeammateCringes=TRUE

		bUseOverlapCheck=TRUE
    End Object
    ExplosionTemplate=ExploTemplate0
}
