/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_SmokeGrenade extends GearProj_Grenade;

/** Effect to play on the grenade after grenade tagging someone but before it blows up. */
var() const protected ParticleSystem		GrenadeTagPreExplosionSmokeEffect;
var() const protected ParticleSystem        GrenadeTagPreExplosionSmokeEffectPlanted;
/** Ref to the spawned pre-explosion grenade tag emitter. */
var protected transient SpawnedGearEmitter	GrenadeTagPreExplosionSmokeEmitter;

/** How long to emit smoke. */
var() protected const config float 			CloudDuration;

simulated function TriggerExplosion(vector HitLocation, vector HitNormal, Actor HitActor)
{
	local Emit_SmokeGrenade Smoke;

	if (!bIsSimulating && !bGrenadeHasExploded)
	{
		// regular explosion
		Super.TriggerExplosion(HitLocation, HitNormal, HitActor);

		// plus smoke!
		// @todo, if attached to a pawn, spawn the smoke at the pawns feet?
		Smoke = Spawn( SmokeEmitterClass,,, HitLocation + HitNormal*3, rotator(HitNormal) );
		if (Smoke != None)
		{
			Smoke.bTriggeredByMartyr = (HoldVictim != None);
			Smoke.bTriggeredBySticky = (MeleeVictim != None && Pawn(MeleeVictim) == None);
			if ( MeleeVictim != None && Pawn(MeleeVictim) != None )
			{
				Smoke.IgnoringDamagePawn = Pawn(MeleeVictim);
			}
			Smoke.InstigatorController = InstigatorController;
			Smoke.SetEmissionDuration(CloudDuration);
		}
	}
}

simulated function name AttachInit(Actor Attachee)
{
	local Pawn TaggedPawn;
	local name BoneName;

	BoneName = super.AttachInit(Attachee);

    GrenadeTagPreExplosionSmokeEmitter = Spawn(class'SpawnedGearEmitter',,, Mesh.GetPosition());
    if (GrenadeTagPreExplosionSmokeEmitter != None)
    {
    	TaggedPawn = Pawn(Attachee);
    	if (TaggedPawn != None)
    	{
    		// attach a special particle system to the mesh
	    GrenadeTagPreExplosionSmokeEmitter.SetTemplate(GrenadeTagPreExplosionSmokeEffect, true);
	    GrenadeTagPreExplosionSmokeEmitter.SetBase(TaggedPawn,, TaggedPawn.Mesh, BoneName);
    	}
	else if (GrenadeTagPreExplosionSmokeEffectPlanted != None)
	{
	    GrenadeTagPreExplosionSmokeEmitter.SetLocation(Mesh.GetPosition() + vect(0,0,12));
	    GrenadeTagPreExplosionSmokeEmitter.SetTemplate(GrenadeTagPreExplosionSmokeEffectPlanted, true);
	    GrenadeTagPreExplosionSmokeEmitter.SetBase(Attachee);
	}
    }
	return BoneName;
}

simulated function Destroyed()
{
	super.Destroyed();

	// destroy the pre-explosion tag emitter as well, if playing
	if (GrenadeTagPreExplosionSmokeEmitter != None)
	{
		// will destroy itself when it's done
		GrenadeTagPreExplosionSmokeEmitter.ParticleSystemComponent.DeactivateSystem();
		GrenadeTagPreExplosionSmokeEmitter = None;
	}
}

static function float GetAIEffectiveRadius()
{
	return default.KnockdownRadius;
}

/**
 * @STATS
 * Records a grenade tag or mine planting
 *
 * @Param Attachee - The pawn that the grenade is attached to
 */

function RecordTagStat(actor Attachee)
{
	local GearPawn GP;
	GP = GearPawn(Attachee);
	if (GP != none)
	{
		GearPRI(GP.PlayerReplicationInfo).NoTimesGrenadeTagged++;
		GearPRI(Instigator.PlayerReplicationInfo).NoGrenadeTags++;
	}
	else
	{
		GearPRI(Instigator.PlayerReplicationInfo).NoGrenadeMines++;
	}
}

/**
 * @STATS
 * Records the # of times someone triggers a mine
 *
 * @Param TrippedBy - The pawn that tripped the mine
 */
function RecordMineStat(actor TrippedBy)
{
	local GearPawn GP;
	GP = GearPawn(TrippedBy);
	if (GP != none)
	{
		GearPRI(GP.PlayerReplicationInfo).NoGrenadeMinesTriggeredByMe++;
		GearPRI(Instigator.PlayerReplicationInfo).NoGrenadeMinesTriggered++;
	}

}


defaultproperties
{
	Damage=0
	DamageRadius=0
	MyDamageType=none
	MomentumTransfer=0

	InFlightSoundTemplate=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeWhipInAirLoopCue'
	GrenadeAttachingToFoeSound=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeStickEnemyCue'
	GrenadeAttachingToWorldSound=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeStickSurfaceCue'
	BeepBeepAboutToExplodeSound=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeWarningCue'
	GrenadeBounceSound=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeBounceCue'

	Begin Object Class=SkeletalMeshComponent Name=COGGrenadeMesh0
	    SkeletalMesh=SkeletalMesh'COG_Bolo_Grenade.Smoke_Grenade'
		PhysicsAsset=PhysicsAsset'COG_Bolo_Grenade.Smoke_Grenade_Physics'
		AnimTreeTemplate=AnimTree'COG_Bolo_Grenade.Animations.AT_BoloGrenade'
		AnimSets(0)=AnimSet'COG_Bolo_Grenade.GrenadeAnims'
		BlockActors=FALSE
		bCastDynamicShadow=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=FALSE
		CollideActors=TRUE
		BlockRigidBody=TRUE
		RBCollideWithChannels=(Default=TRUE)
		Scale=1.5
		LightEnvironment=MyLightEnvironment
	bAcceptsDynamicDecals=FALSE
	End Object
	Mesh=COGGrenadeMesh0
	Components.Add(COGGrenadeMesh0)

	Begin Object Name=CollisionCylinder
		CollideActors=TRUE
	End Object

	TrailTemplate=ParticleSystem'COG_Smoke_Grenade.Effects.Smoke_Grenade_Smoke_Trail'

	// explosion point light
    Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=800.000000
		Brightness=300.000000
		LightColor=(B=170,G=200,R=249,A=255)
		Translation=(X=16)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
    End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_SmokeGrenade'
		MomentumTransferScale=0.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'COG_Smoke_Grenade.Effects.P_COG_Smoke_Grenade_Explo'
		ExplosionSound=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeExplosionCue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(Anim=CameraAnim'Effects_Camera.Explosions.CA_SmokeGrenade_Explo')
		ExploShakeInnerRadius=256
		ExploShakeOuterRadius=700
		ExploShakeFalloff=1.f

		PawnKnockdownEffort=GearEffort_ImpactCough

		FractureMeshRadius=200.0
		FracturePartVel=500.0
	End Object
	ExplosionTemplate=ExploTemplate0

	GrenadeTagPreExplosionSmokeEffect=ParticleSystem'COG_Smoke_Grenade.Effects.P_COG_Smoke_Grenade_Stick_On'
    GrenadeTagPreExplosionSmokeEffectPlanted=ParticleSystem'COG_Smoke_Grenade.Effects.P_COG_Smoke_Grenade_Tagged'
	SmokeEmitterClass=class'Emit_SmokeGrenade'
}
