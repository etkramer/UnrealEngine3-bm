/**
 * Grenade_Frag
 * Frag Grenade projectile implementation
 *
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_FragGrenade extends GearProj_Grenade
	config(Weapon);

/** Placed when the grenade impacts the ground **/
var DecalMaterial ScorchDecal;

/** Effect to play when grenade explodes in the air */
var() protected const ParticleSystem InAirExplosionEmitterTemplate;
/** Effect to play when grenade explodes on a surface without a material-specific effect */
var() protected const ParticleSystem DefaultOnGroundExplosionEmitterTemplate;
/** Effect to play when grenade explodes on a surface without a material-specific effect */
var() protected const ParticleSystem DefaultTaggedEmitterTemplate;

/** Effect to play when grenade explodes when attached to player */
var() protected const ParticleSystem StuckInPawnExplosionEmitterTemplate;
var() protected const ParticleSystem StuckInPawnExplosionNoGoreEmitterTemplate;

simulated function PostBeginPlay()
{
	super.PostBeginPlay();
	Disable('Tick');
}


simulated function TriggerExplosion(vector HitLocation, vector HitNormal, Actor HitActor)
{
	local GearSpawner_EmergenceHoleBase Hole;
	local vector Momentum;
	local Emit_SmokeGrenade Smoke;

	if (!bIsSimulating && !bGrenadeHasExploded)
	{
		// @fixme, not sure why hitlocation is passed in for momentum, but that's how the old code was...
		// double check this to see if this ia bug
		//Momentum = MomentumTransfer;
		Momentum = HitLocation + HitNormal * 3;

		if( Role == Role_Authority )
		{
			// blow up melee/held guys
			// this stops any exploits of some how getting the HurtOrigin inside a wall or object such that the line checks will fail to hurt the victim
			if( MeleeVictim != none )
			{
				MeleeVictim.TakeDamage( 9999.f, InstigatorController, HitLocation, Momentum, class'GDT_FragTag',, self );
			}
			if ( HoldVictim != None )
			{
				HoldVictim.TakeDamage( 9999.f, InstigatorController, HitLocation, Momentum, class'GDT_FragMartyr',, self );
			}
		}

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
			Smoke.SetEmissionDuration(6.0f);
		}

		Super.TriggerExplosion(HitLocation, HitNormal, HitActor);

		if( Role == Role_Authority )
		{
			// check for e-hole near-misses to give player feedback
			// do this after the super call, so the damage has been applied
			foreach CollidingActors(class'GearSpawner_EmergenceHoleBase', Hole, ExploDamageRadius*2, HitLocation)
			{
				if (Hole.HoleStatus == HS_Open)
				{
					// didn't die in the hurtradius call, notify
					GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_EHoleGrenadeMissed, Instigator);
				}
			}
		}
	}
}

//simulated function Bounce(Vector HitLocation, Vector HitNormal)
//{
//	Super.Bounce(HitLocation, HitNormal);
//
//}


simulated function PrepareExplosionTemplate()
{
	super.PrepareExplosionTemplate();

	// choose which effect to play
	ExplosionTemplate.ParticleEmitterTemplate = DetermineWhichParticleSystemToPlay();
}


simulated function protected ParticleSystem DetermineWhichParticleSystemToPlay()
{
	local ParticleSystem Retval;
	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceDest;
	local vector TraceStart;
	local vector TraceExtent;
	local TraceHitInfo HitInfo;
	local PhysicalMaterial ParentPhysMaterial;
	local ParticleSystem ImpactEffect;

    // if we are attached to a pawn we want to the the tagged explosion template
	if( Pawn(MeleeVictim) != None )
	{
		//`log( "using player TAGGED Emitter!" );
		if( WorldInfo.GRI.ShouldShowGore() )
		{
			ImpactEffect = StuckInPawnExplosionEmitterTemplate;
		}
		else
		{
			ImpactEffect = StuckInPawnExplosionNoGoreEmitterTemplate;
		}

		ExplosionTemplate.bParticleSystemIsBeingOverriddenDontUsePhysMatVersion = TRUE;
		ExplosionTemplate.bAllowPerMaterialFX = TRUE; // we need to set this here otherwise the GearExplosionActor will override
	}
	// attached to wall from wall tag
	else if( MeleeVictim != None )
	{
		//`log( "using TAGGED Emitter!" );
		ImpactEffect = DefaultTaggedEmitterTemplate;
		ExplosionTemplate.bParticleSystemIsBeingOverriddenDontUsePhysMatVersion = TRUE;
		ExplosionTemplate.bAllowPerMaterialFX = TRUE; // we need to set this here otherwise the GearExplosionActor will override
	}
	// use the material based effect
	else
	{
		// we need to trace and see if the grenade is still doing small
		// bounces or if it is actually in the air
		TraceStart = Location;
		TraceDest = Location - vect(0, 0, 5);

		// trace down and see what we are standing on
		// once the material system is updated we will be able to get the material type and play
		// something off that!  TTP 13101
		Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, false, TraceExtent, HitInfo, TRACEFLAG_PhysicsVolumes );

		// if there is no material then we can not do anything
		if( none == HitInfo.Material )
		{
			Retval = InAirExplosionEmitterTemplate;
			return Retval;
		}

		// so here we need to look up the "hierarchy" if we have a none in the attribute we want
		ImpactEffect = GetImpactEffect( HitInfo.Material.PhysMaterial );
		//`log("GetImpactEffect returned"@ImpactEffect);

		if( none == HitInfo.Material.PhysMaterial )
		{
			ParentPhysMaterial = none;
		}
		else
		{
			ParentPhysMaterial = HitInfo.Material.PhysMaterial.Parent;
		}


		// this will walk the tree until our parent is null
		// at which point we will break out (which is basically an exception case)
		// but there are no exceptions in .uc land
		while( ( none == ImpactEffect )
			&& ( none != ParentPhysMaterial )
			)
		{
			// look at our parent's data
			ImpactEffect = GetImpactEffect( ParentPhysMaterial );
			ParentPhysMaterial = ParentPhysMaterial.Parent;
		}


		//`log( "use the material based effect" );
		ImpactEffect = ImpactEffect;
	}


	// do default behavior
	if( none == ImpactEffect )
	{
		//`Log( "ERROR:  none == ImpactEffect" );
		//`log("using default impact effect");
		ImpactEffect = DefaultOnGroundExplosionEmitterTemplate;
	}

	//`log("DetermineWhichParticleSystemToPlay ImpactEffect"@ImpactEffect);

	return ImpactEffect;
}



/**
* Looks to see if the Effect on the current PhysicalMaterial is valid.
**/
simulated function ParticleSystem GetImpactEffect( PhysicalMaterial PMaterial )
{
	local ParticleSystem Retval;

	Retval = none;

	if( ( none == PMaterial )
		|| ( none == PMaterial.PhysicalMaterialProperty )
		|| ( none == GearPhysicalMaterialProperty(PMaterial.PhysicalMaterialProperty) )
		)
	{
		Retval = none;
	}
	// our Specific properties exists now we need to call our function to get out
	// the specificProperty
	else
	{
		Retval = GetWeaponSpecificImpactEffect( GearPhysicalMaterialProperty(PMaterial.PhysicalMaterialProperty) );

	}

	return Retval;
}


simulated function ParticleSystem GetWeaponSpecificImpactEffect( GearPhysicalMaterialProperty ImpactEffects )
{
	return none;
}

simulated function name AttachInit(Actor Attachee)
{
	local GearPawn VictimGP;

	VictimGP = GearPawn(Attachee);
	if (VictimGP != None)
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_StuckExplosiveToEnemy, Instigator, VictimGP, 1.f);
	}

	return super.AttachInit(Attachee);
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



/*
	// used to debug damage radius
	Begin Object Class=DrawSphereComponent Name=DrawSphere0
			SphereColor=(R=0,G=255,B=0,A=255)
			SphereRadius=896
			HiddenGame=FALSE
	End Object
	Components.Add(DrawSphere0)
*/
defaultproperties
{
    bNetTemporary=false

	InFlightSoundTemplate=SoundCue'Weapon_Grenade.Firing.GrenadeInAirLoopCue'
	GrenadeAttachingToFoeSound=SoundCue'Weapon_Grenade.Actions.GrenadeStick01Cue'
	GrenadeAttachingToWorldSound=SoundCue'Weapon_Grenade.Actions.GrenadeStick01Cue'
	BeepBeepAboutToExplodeSound=SoundCue'Weapon_Grenade.Actions.GrenadeBeep01Cue'
	GrenadeBounceSound=SoundCue'Weapon_Grenade.Impacts.GrenadeBounceCue'


	Begin Object Class=SkeletalMeshComponent Name=COGGrenadeMesh0
		SkeletalMesh=SkeletalMesh'COG_Bolo_Grenade.Frag_Grenade'
		PhysicsAsset=PhysicsAsset'COG_Bolo_Grenade.Frag_Grenade_Physics'
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

	TrailTemplate=ParticleSystem'COG_Frag_Grenade.Effects.SmokeTrail'
	ScorchDecal=DecalMaterial'War_Gameplay_Decals.Textures.Granade_Decal'

	InAirExplosionEmitterTemplate=ParticleSystem'COG_Frag_Grenade.Effects.P_COG_Frag_Grenade_Ground_Explo'
	DefaultOnGroundExplosionEmitterTemplate=ParticleSystem'COG_Frag_Grenade.Effects.P_COG_Frag_Grenade_Ground_Explo'
	DefaultTaggedEmitterTemplate=ParticleSystem'COG_Frag_Grenade.Effects.P_Frag_Tag_Explosion'
	StuckInPawnExplosionEmitterTemplate=ParticleSystem'COG_Frag_Grenade.Effects.P_Frag_Tag_Player_Explosion'
	StuckInPawnExplosionNoGoreEmitterTemplate=ParticleSystem'COG_Frag_Grenade.Effects.P_Frag_Tag_Player_Explosion_NOGORE'

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
		MyDamageType=class'GDT_FragGrenade'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'COG_Frag_Grenade.Effects.P_COG_Frag_Grenade_Ground_Explo'
		ExplosionSound=SoundCue'Weapon_Grenade.Impacts.GrenadeBoloExplosionCue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
		ExploShakeInnerRadius=650
		ExploShakeOuterRadius=1500

		FractureMeshRadius=200.0
		FracturePartVel=500.0

		bAllowPerMaterialFX=TRUE
		DecalTraceDistance=64.0f

		bUsePerMaterialFogVolume=TRUE
	End Object
	ExplosionTemplate=ExploTemplate0

}
