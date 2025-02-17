/**
 * GearObjectPool
 *
 *   Our Object pool holder.  This should only ever be spawned on the client.  Right now we spawn
 * it from the GearGRI.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class GearObjectPool extends Actor
	native
	dependson(GearTypes);


//
// KActors pool
//

/** Maximum number of KActors */
const MAX_KACTORS = 20;

struct native KActorCacheDatum
{
	/** Index of the most recently used element */
	var	INT				ListIdx;
	/** List of cached elements */
	var	KActorSpawnable	List[MAX_KACTORS];
};

var KActorCacheDatum KActorCache;

/** Array of Static Meshes to cook physics for at level start up. */
var Array<StaticMesh>	CachedStaticMeshes;

//
// Tracers pool
//

/** Maximum number of tracers **/
const MAX_TRACERS = 20;

struct native TracerCacheDatum
{
	var int Type;

	var class<GearProj_BulletTracer> TracerClass;

	/** Index of the most recently used tracer */
	var int ListIdx;

	/** List of tracers this weapon uses, for recycling */
	var GearProj_BulletTracer List[MAX_TRACERS];
};

var TracerCacheDatum TracerCache[8]; // this is GearTypes.EWeaponTracerType EnumCount


//// Emitters for impact effects
const MAX_IMPACT_EMITTERS = 20;
const MAX_IMPACT_EMITTERS_SMALL = 10;


const MAX_IMPACT_PARTICLE_SYSTEMS = 20;
const MAX_IMPACT_PARTICLE_SYSTEMS_SMALL = 10;


struct native EmitterCacheDatum
{
	var int Type;

	/** We want to have different cache lines for effects so we need to store off the size of our line **/
	var int SizeOfList;

	var ParticleSystem ParticleSystemType;

	/** Index of the most recently used tracer */
	var int ListIdx;

	/** List of ParticleSystemComponent this weapon uses, for recycling */
	var ParticleSystemComponent PList[MAX_IMPACT_PARTICLE_SYSTEMS];

	/** List of Emitter this weapon uses, for recycling */
	var Emitter List[MAX_IMPACT_EMITTERS];

};

// to add:
//

enum EWOPCacheTypes
{
	WOP_DEFAULT_GENERAL,
	WOP_DEFAULT_IMPACT,
	WOP_DEFAULT_IMPACT_AR,
	WOP_DEFAULT_IMPACT_METAL,
	WOP_DEFAULT_IMPACT_WOOD,
	WOP_DEFAULT_NO_DAMAGE_SPARKS,

	WOP_WEAPON_TRACER_SMOKETRAIL,
	WOP_WEAPON_TRACER_SMOKETRAIL_AR,

	WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST,
	WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_AR,
	WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_METAL,
	WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_WOOD,

	WOP_DEFAULT_SHOTGUN,
	WOP_DEFAULT_SHOTGUN_METAL,
	WOP_DEFAULT_SHOTGUN_WOOD,

	WOP_DEFAULT_MULCHER_METAL,
	WOP_DEFAULT_MULCHER_STONE,

	WOP_DEFAULT_MULCHER_TRACER_SMOKETRAIL,
	WOP_DEFAULT_MULCHER_TRACER_SMOKETRAIL_AR,


	WOP_DEFAULT_P_TroikaCabal_Impact_Stone,

	WOP_COVER_HIT,
	WOP_PLAYER_SLIDE,
	WOP_BLOOD_MELEE_EMITTER,
	WOP_BLOOD_LOD,
	WOP_BLOOD_METAL_LOD,
	WOP_BLOOD_ARMOR_LOD,
	WOP_BLOOD_DEATH_LOD,
	WOP_BLOOD_GENERAL,
	WOP_BLOOD_FLYING_BODY_PART,
	WOP_BLOOD_SPRAY_HIT_EFFECT,
	WOP_BLOOD_HEADSHOT,
	WOP_BERSERKER_IMPACT,
	WOP_NO_BLOOD,
	WOP_BLOOD_MELEE,
	WOP_PLAYER_ROLL,
	WOP_BLOOD_GROUND_IMPACT,
	WOP_BLOOD_EXIT_WOUND,
	WOP_FRACTURE_CONCRETE_MEDIUM,
	WOP_DEFAULT_IMPACT_CHEAP,
	WOP_PLAYER_BRUMAK_IMPACTS,
};



/** default impacts (assaultCOG, assaultLocust), blood impacts, general pool **/
var array<EmitterCacheDatum> EmitterCache;


// reduce this to reduce cache misses
const MAX_RAINDROP_EMITTERS = 20;
//make separate ones for each type
var Emitter RainDropSelfEmitterList[MAX_RAINDROP_EMITTERS];
var int RainDropSelfEmitterListIdx;

const MAX_HAILIMPACT_EMITTERS = 20;
var Emitter HailImpactPawnEmitterList[MAX_HAILIMPACT_EMITTERS];
var int HailImpactPawnEmitterListIdx;


/** The material used on the bullet tracers **/
var Material BulletMaterial;

/** All of the MaterialInstanceContenstants for the various ActiveReload levels **/
var MaterialInstanceConstant MIC_COG_Default;
var MaterialInstanceConstant MIC_COG_ARTierOne;
var MaterialInstanceConstant MIC_COG_ARTierTwo;
var MaterialInstanceConstant MIC_COG_ARTierThree;

var MaterialInstanceConstant MIC_LOCUST_Default;
var MaterialInstanceConstant MIC_LOCUST_ARTierOne;
var MaterialInstanceConstant MIC_LOCUST_ARTierTwo;
var MaterialInstanceConstant MIC_LOCUST_ARTierThree;



// decal pooling

struct native DecalDatum
{
	var GearDecal GD;
	/** The time this decal was spawned **/
	var float SpawnTime;
	/** The location where this decal was spawned.  We use this to determine if we should spawn decals near it **/
	var vector SpawnLoc;
	/** This is the distance away from another decal of this this type we must be away from so we can spawn ( this reduces having just a ton of decals all on top of each other which don't really add any visual quality and is actually really slow to render **/
	var float CanSpawnDistance;
};


struct native DecalPool
{
	var int MAX_DECALS;
	var int MAX_DECALS_bDropDetail;
	var int MAX_DECALS_bAggressiveLOD ;

	var array<DecalDatum> Decals; //[MAX_<TYPE>];
	var transient int DecalsIdx;
};

var DecalPool DecalPool_Blood;
var DecalPool DecalPool_Bullet;
var DecalPool DecalPool_Explosion;


/** Path constraint pool */
const NUM_CONSTAINT_CLASSES = 15; // this should be the number of path constraint sub classes
const MAX_INSTANCES_PER_CLASS = 3; // currently can't have more than 3 of the same path constraint at once
struct native PathConstraintCacheDatum
{
	var int ListIdx;
	var PathConstraint List[MAX_INSTANCES_PER_CLASS];
};
var array<PathConstraintCacheDatum> PathConstraintCache;


/** Path Goal evaluator pool */
// NOTE: you can't currently have more than once instance of the same goal evaluator class at once!!
const MAX_GOALEVALUATORS = 10; // this should be the number of goal evaluate constraint sub classes
var array<PathGoalEvaluator> PathGoalEvaluatorCache;



simulated event PreBeginPlay()
{
	CreatePools();
}


simulated event Destroyed()
{
	CleanUpPools();

	Super.Destroyed();
}


simulated final function CreatePools()
{
	EmitterCache.Length = EWOPCacheTypes.EnumCount;  // why do this here or not make this be static size above?
	//CreateMaterialInstanceConstants();
	CreateTracers();
	CreateImpactEffects();
	CreateKActors();
	CreateDecals();
	CreatePathConstraintCache();
}


simulated final function CleanUpPools()
{
	CleanUpKActors();
	CleanupDecals();
	CleanupEmitters();
	CleanupPathConstraints();
	CleanupTracers();
	// clean up the raindrops if we ever use them again

	// GC here
	WorldInfo.ForceGarbageCollection();
}


/** This will clean up up all of the emitters we have. **/
simulated final function CleanupEmitters()
{
	local int Idx;
	local int Jdx;

	// for each type of emitter
	for( Idx = 0; Idx < EmitterCache.Length; ++Idx )
	{
		// destroy the PSCs
		for( Jdx = 0; Jdx < MAX_IMPACT_PARTICLE_SYSTEMS; ++Jdx )
		{
			if( EmitterCache[Idx].PList[Jdx] != none )
			{
				EmitterCache[Idx].PList[Jdx].ResetToDefaults();
				EmitterCache[Idx].PList[Jdx] = none;
			}
		}

		// destroy the Emitters
		for( Jdx = 0; Jdx < MAX_IMPACT_EMITTERS; ++Jdx )
		{
			if( EmitterCache[Idx].List[Jdx] != none )
			{
				EmitterCache[Idx].List[Jdx].Destroy();
				EmitterCache[Idx].List[Jdx] = none;
			}
		}
	}
}



simulated final function CleanupTracers()
{
	local int Idx;
	local int Jdx;

	for( Idx = 0; Idx < 8; ++Idx )  // this is GearTypes.EWeaponTracerType EnumCount
	{
		for( Jdx = 0; Jdx < MAX_TRACERS; ++Jdx )
		{
			if( TracerCache[Idx].List[Jdx] != none )
			{
				TracerCache[Idx].List[Jdx].Destroy();
				TracerCache[Idx].List[Jdx] = none;
			}
		}
	}
}


/** This is used to clean up the GOP for a new MP round **/
simulated final function CleanupForNewRound()
{
	CleanupForPToPTransition();  // this will free all out standing allocations and then reinit them to allow the allocator to give back pages to the OS
}


/**
 * This is used to clean up the GOP when we P to P transfer.  We want to destroy all actors and then recreate them to let the allocator
 * "defragment" itself some.  (i.e. a long living cached object could be the only data on a page, so we want to delete it such that the
 * allocator can put the allocation in a better spot and potentially free up the page)
 **/
simulated final function CleanupForPToPTransition()
{
	CleanUpPools();

	// and then on the next next tick go ahead and create these otherwise you will have 2x as much data in memory!
	SetTimer( 0.5f, FALSE, nameof(CleanupForPToPTransition_RecreatePools) );
}

native final function bool IsSafeToRecreatePools();

/** This will recreate the pools AFTER GC has occurred **/
simulated private final function CleanupForPToPTransition_RecreatePools()
{
	if (!IsSafeToRecreatePools())
	{
		SetTimer(0.1, false, nameof(CleanupForPToPTransition_RecreatePools));
	}
	else
	{
		CreatePools();
	}
}



simulated final function CleanupDecals()
{
	local int i;

	for( i = 0; i < DecalPool_Blood.Decals.length; ++i )
	{
		if (DecalPool_Blood.Decals[i].GD != None)
		{
			DecalPool_Blood.Decals[i].GD.ResetToDefaults();
			DecalPool_Blood.Decals[i].GD.LifeSpan = 0.01f;
			DecalPool_Blood.Decals[i].GD = none;
		}
	}

	for( i = 0; i < DecalPool_Bullet.Decals.length; ++i )
	{
		if (DecalPool_Bullet.Decals[i].GD != None)
		{
			DecalPool_Bullet.Decals[i].GD.ResetToDefaults();
			DecalPool_Bullet.Decals[i].GD.LifeSpan = 0.01f;
			DecalPool_Bullet.Decals[i].GD = none;
		}
	}

	for( i = 0; i < DecalPool_Explosion.Decals.length; ++i )
	{
		if (DecalPool_Explosion.Decals[i].GD != None)
		{
			DecalPool_Explosion.Decals[i].GD.ResetToDefaults();
			DecalPool_Explosion.Decals[i].GD.LifeSpan = 0.01f;
			DecalPool_Explosion.Decals[i].GD = none;
		}
	}
}


simulated final function CreateMaterialInstanceConstants()
{
	// COG
	CreateMaterialInstanceConstants_Worker( MIC_COG_Default, MakeLinearColor( 80.0f, 0.0f, 0.0f, 1.0f) ); // cog default

	CreateMaterialInstanceConstants_Worker( MIC_COG_ARTierOne, MakeLinearColor( 10.0f, 20.0f, 50.0f, 1.0f) ); // cog ActiveReload 1

	CreateMaterialInstanceConstants_Worker( MIC_COG_ARTierTwo, MakeLinearColor( 10.0f, 20.0f, 80.0f, 1.0f) ); // cog ActiveReload 2

	CreateMaterialInstanceConstants_Worker( MIC_COG_ARTierThree, MakeLinearColor( 10.0f, 20.0f, 130.0f, 1.0f) ); // cog ActiveReload 3


	// Locust
	CreateMaterialInstanceConstants_Worker( MIC_LOCUST_Default, MakeLinearColor( 80.0f, 0.6f, 0.1f, 1.0f) ); // locust default

	CreateMaterialInstanceConstants_Worker( MIC_LOCUST_ARTierOne, MakeLinearColor( 100.0f, 1.3f, 1.0f, 1.0f) ); // locust ActiveReload 1

	CreateMaterialInstanceConstants_Worker( MIC_LOCUST_ARTierTwo, MakeLinearColor( 120.0f, 1.0f, 1.0f, 1.0f) ); // locust ActiveReload 2

	CreateMaterialInstanceConstants_Worker( MIC_LOCUST_ARTierThree, MakeLinearColor( 150.0f, 1.0f, 1.0f, 1.0f) ); // locust ActiveReload 3

}


simulated final function CreateMaterialInstanceConstants_Worker( out MaterialInstanceConstant TheMIC, LinearColor TheLinearColor )
{
	TheMIC = new(self) class'MaterialInstanceConstant';

	TheMIC.SetParent( BulletMaterial );

	TheMIC.SetVectorParameterValue( 'Tracer1_color', TheLinearColor  );

}


simulated final function CreateTracers()
{
	local int Idx;

	for( Idx = 0; Idx < MAX_TRACERS; ++Idx )
	{
		CreateTracer_Worker( WTT_LongSkinny, class'GearProj_BulletTracer', Idx, TRUE );
	}

	for( Idx = 0; Idx < MAX_TRACERS; ++Idx )
	{
		CreateTracer_Worker( WTT_ShortBullet, class'GearProj_BulletTracer_Locust', Idx, TRUE );
	}

	for( Idx = 0; Idx < MAX_TRACERS; ++Idx )
	{
		CreateTracer_Worker( WTT_MinigunFastFiring, class'GearProj_BulletTracer_HeavyMiniGun', Idx, FALSE );
	}

	for( Idx = 0; Idx < MAX_TRACERS; ++Idx )
	{
		CreateTracer_Worker( WTT_Hammerburst, class'GearProj_BulletTracer_Hammerburst', Idx, TRUE );
	}

	for( Idx = 0; Idx < MAX_TRACERS; ++Idx )
	{
		CreateTracer_Worker( WTT_Boltok, class'GearProj_BulletTracer_Boltok', Idx, FALSE );
	}

	for( Idx = 0; Idx < MAX_TRACERS; ++Idx )
	{
		CreateTracer_Worker( WTT_Brumak, class'GearProj_BulletTracer_Brumak', Idx, FALSE );
	}

	for( Idx = 0; Idx < MAX_TRACERS; ++Idx )
	{
		CreateTracer_Worker( WTT_Reaver, class'GearProj_BulletTracer_Reaver', Idx, FALSE );

	}

	for( Idx = 0; Idx < MAX_TRACERS; ++Idx )
	{
		CreateTracer_Worker( WTT_BrumakPlayer, class'GearProj_BulletTracer_Brumak_Player', Idx, FALSE );
	}


}


simulated final function CreateTracer_Worker( EWeaponTracerType TracerType, class<GearProj_BulletTracer> TracerClass, int Idx, bool bForceCreate )
{
	local vector SpawnLocation;

	// 262144 is HALF_WORLD_MAX @see engine.h
	SpawnLocation = vect(262144,262144,262144);

	// always set the TracerClass
	TracerCache[TracerType].TracerClass = TracerClass;

	if( bForceCreate == TRUE )
	{
		TracerCache[TracerType].List[Idx] = Spawn( TracerClass, self, , SpawnLocation );
		TracerCache[TracerType].List[Idx].SetDrawScale3D( TracerClass.default.TracerDrawScale3D );
		TracerCache[TracerType].List[Idx].Recycle();
	}
}


simulated final function GearProj_BulletTracer GetTracer( EWeaponTracerType TracerType, byte ActiveReloadTier, vector SpawnLocation, rotator SpawnRotation )
{
	return GetTracer_Worker( TracerCache[TracerType], TracerType, ActiveReloadTier, SpawnLocation, SpawnRotation );
}


simulated final protected function GearProj_BulletTracer GetTracer_Worker( out TracerCacheDatum TheDatum, EWeaponTracerType TracerType, byte ActiveReloadTier, vector SpawnLocation, rotator SpawnRotation )
{
	local GearProj_BulletTracer PBT;

	if( ++TheDatum.ListIdx >= MAX_TRACERS )
	{
		TheDatum.ListIdx = 0;
	}

	//`log( "GetTracer_Worker: " $ TheDatum.ListIdx $ " SpawnLocation: " $ SpawnLocation $ " " $ TheDatum.List[TheDatum.ListIdx] );

	PBT = TheDatum.List[TheDatum.ListIdx];

	// if we need to spawn one of these
	if( ( PBT == none ) || ( PBT.bDeleteMe ) )
	{
		CreateTracer_Worker( TracerType, TheDatum.TracerClass, TheDatum.ListIdx, TRUE );
		PBT = TheDatum.List[TheDatum.ListIdx];
	}

	// otherwise unhide the existing one and set to the new location
	PBT.SetLocation( SpawnLocation );
	PBT.SetRotation( SpawnRotation );
	PBT.SetPhysics( PHYS_Projectile );

	PBT.bStasis = FALSE;
// 	// set the color of the tracer based on the ActiveReload tier
// 	switch( TeamNum )
// 	{
// 	    case 0: // COG
//
// 			switch( ActiveReloadTier )
// 			{
// 			case 0: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_COG_Default ); break;
// 			case 1: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_COG_ARTierOne ); break;
// 			case 2: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_COG_ARTierTwo ); break;
// 			case 3: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_COG_ARTierThree ); break;
// 			}
// 			break;
//
//  		case 1: // Locust
//  			switch( ActiveReloadTier )
//  			{
//  			case 0: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_LOCUST_Default ); break;
//  			case 1: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_LOCUST_ARTierOne ); break;
//  			case 2: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_LOCUST_ARTierTwo ); break;
//  			case 3: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_LOCUST_ARTierThree ); break;
//  			}
// 			break;
//
// 		default: // just use the cog default value
// 			StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_COG_Default ); break;
// 	}

	//`log( TracerType @ StaticMeshComponent(PBT.Mesh) @ StaticMeshComponent(PBT.Mesh).Materials[0] );
// 	switch( TracerType )
// 	{
// 		case 0: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_COG_Default );
// 		case 1: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_LOCUST_Default );
//
// 		default: StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_COG_Default );
// 	}

	// this seems to be all hosed and not working
	//MIC_COG_Default.SetVectorParameterValue('Tracer1_color',MakeLinearColor(7.f,2.5.f,1.f,1.f));
	//StaticMeshComponent(PBT.Mesh).SetMaterial( 0, MIC_COG_Default );


	return PBT;
}



// NOTE:  we probably want to call this on first usage of an effect so we are datadriven
simulated final function CreateImpactEffects()
{
	// emitters
	CreateEmitter_List(WOP_DEFAULT_IMPACT,				ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact', MAX_IMPACT_EMITTERS, TRUE );
	CreateEmitter_List(WOP_DEFAULT_IMPACT_AR,				ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_AR', MAX_IMPACT_EMITTERS, TRUE );
	CreateEmitter_List(WOP_DEFAULT_IMPACT_METAL,				ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_Metal_Solid', MAX_IMPACT_EMITTERS, TRUE );
	CreateEmitter_List(WOP_DEFAULT_IMPACT_WOOD,		ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_Wood_new', MAX_IMPACT_EMITTERS, FALSE );

	CreateEmitter_List(WOP_WEAPON_TRACER_SMOKETRAIL,		ParticleSystem'War_BulletEffects.FX.FX_P_BulletTrail_Normal', MAX_IMPACT_EMITTERS, TRUE );
	CreateEmitter_List(WOP_WEAPON_TRACER_SMOKETRAIL_AR,		ParticleSystem'War_BulletEffects.FX.FX_P_BulletTrail_HighPower', MAX_IMPACT_EMITTERS, TRUE );

	CreateEmitter_List(WOP_DEFAULT_NO_DAMAGE_SPARKS,				ParticleSystem'COG_AssaultRifle.Effects.P_Impact_No_Damage_Sparks', MAX_IMPACT_EMITTERS, FALSE );


	CreateEmitter_List(WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST,				ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Impact', MAX_IMPACT_EMITTERS, TRUE );
	CreateEmitter_List(WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_AR,				ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Impact_AR', MAX_IMPACT_EMITTERS, TRUE );
	CreateEmitter_List(WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_METAL,		ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Impact_Metal_Solid', MAX_IMPACT_EMITTERS, FALSE );
	CreateEmitter_List(WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_WOOD,		        ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Impact_Wood', MAX_IMPACT_EMITTERS, FALSE );

	CreateEmitter_List(WOP_DEFAULT_SHOTGUN,					ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_Impact', MAX_IMPACT_EMITTERS_SMALL, TRUE );
	CreateEmitter_List(WOP_DEFAULT_SHOTGUN_METAL,				ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_Impact_Metal', MAX_IMPACT_EMITTERS_SMALL, TRUE );
	CreateEmitter_List(WOP_DEFAULT_SHOTGUN_WOOD,				ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_Impact_Wood', MAX_IMPACT_EMITTERS_SMALL, FALSE );

	CreateEmitter_List(WOP_DEFAULT_MULCHER_METAL,				ParticleSystem'COG_GatlingGun.Effects.P_COG_Gatling_Impact_Metal', MAX_IMPACT_EMITTERS, FALSE );
	CreateEmitter_List(WOP_DEFAULT_MULCHER_STONE,				ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Impact_Stone', MAX_IMPACT_EMITTERS, FALSE );

	CreateEmitter_List(WOP_DEFAULT_MULCHER_TRACER_SMOKETRAIL,				ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Tracer', MAX_IMPACT_EMITTERS, FALSE );
	CreateEmitter_List(WOP_DEFAULT_MULCHER_TRACER_SMOKETRAIL_AR,				ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Tracer_AR', MAX_IMPACT_EMITTERS, FALSE );


	CreateEmitter_List(WOP_COVER_HIT,							ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Cover_Hit', MAX_IMPACT_EMITTERS_SMALL, TRUE );
	CreateEmitter_List(WOP_PLAYER_SLIDE,							ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Slide', MAX_IMPACT_EMITTERS_SMALL, TRUE );
	CreateEmitter_List(WOP_PLAYER_ROLL,							ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Roll_Attached', MAX_IMPACT_EMITTERS_SMALL, TRUE );

	CreateEmitter_List(WOP_BLOOD_GROUND_IMPACT,	ParticleSystem'Effects_Gameplay.Blood.P_Blood_groundimpact_bodypart', MAX_IMPACT_EMITTERS_SMALL, FALSE );
	CreateEmitter_List(WOP_BLOOD_MELEE_EMITTER,					ParticleSystem'Effects_Gameplay.Blood.P_Melee_Blood', MAX_IMPACT_EMITTERS_SMALL, FALSE );

	CreateEmitter_List(WOP_FRACTURE_CONCRETE_MEDIUM,					ParticleSystem'GOW_Fractured_Objects.Effects.P_Fracture_Concrete_Grey_Medium', MAX_IMPACT_EMITTERS, TRUE );
	CreateEmitter_List(WOP_DEFAULT_IMPACT_CHEAP,					ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_CHEAP', MAX_IMPACT_EMITTERS, TRUE );

	CreateEmitter_List(WOP_BLOOD_DEATH_LOD,		ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD_Death', 5, TRUE );

	CreatePS_List(WOP_DEFAULT_IMPACT_CHEAP,					ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_CHEAP', MAX_IMPACT_EMITTERS, TRUE );

	CreatePS_List(WOP_PLAYER_BRUMAK_IMPACTS,					ParticleSystem'Locust_Brumak.Effects.P_Brumak_Gun_Impact', MAX_IMPACT_EMITTERS, FALSE );


	CreateEmitter_List(WOP_DEFAULT_P_TroikaCabal_Impact_Stone,ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_Derrick_Metal_Moving', MAX_IMPACT_EMITTERS_SMALL, FALSE ); // );

	CreateEmitter_List(WOP_DEFAULT_GENERAL, None, MAX_IMPACT_EMITTERS, FALSE ); // );

// 	// general other impacts
// 	for( Idx = 0; Idx < MAX_IMPACT_EMITTERS; ++Idx )
// 	{
// 		EmitterCache[WOP_DEFAULT_GENERAL].List[Idx] = Spawn( class'SpawnedGearEmitter', self );
// 		EmitterCache[WOP_DEFAULT_GENERAL].List[Idx].ParticleSystemComponent.bAutoActivate = FALSE;
// 		GearEmitter(EmitterCache[WOP_DEFAULT_GENERAL].List[Idx]).HideSelf();
// 		EmitterCache[WOP_DEFAULT_GENERAL].List[Idx].ParticleSystemComponent.LODMethod = PARTICLESYSTEMLODMETHOD_DirectSet;
// 		EmitterCache[WOP_DEFAULT_GENERAL].List[Idx].ParticleSystemComponent.bOverrideLODMethod = TRUE;
// 	}

	// particle system component
	CreatePS_List(WOP_BLOOD_METAL_LOD,		ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD_Metal', MAX_IMPACT_PARTICLE_SYSTEMS, FALSE );
	CreatePS_List(WOP_BLOOD_ARMOR_LOD,		ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD_Armor', MAX_IMPACT_PARTICLE_SYSTEMS, TRUE );
	CreatePS_List(WOP_BLOOD_FLYING_BODY_PART,	ParticleSystem'Effects_Gameplay.Blood.P_Blood_flying_bodypart', MAX_IMPACT_PARTICLE_SYSTEMS_SMALL, TRUE );
	CreatePS_List(WOP_BLOOD_SPRAY_HIT_EFFECT,	ParticleSystem'Effects_Gameplay.Blood.P_Bloodspray_hit_effect', MAX_IMPACT_PARTICLE_SYSTEMS_SMALL, TRUE );
	CreatePS_List(WOP_BLOOD_HEADSHOT,			ParticleSystem'Effects_Gameplay.Blood.P_Blood_HeadShot', 5, FALSE );
	CreatePS_List(WOP_NO_BLOOD,			    ParticleSystem'Effects_Gameplay.Blood.P_Hit_Effect_No_Blood', MAX_IMPACT_PARTICLE_SYSTEMS, FALSE );

	CreatePS_List(WOP_BLOOD_MELEE,	        ParticleSystem'Effects_Gameplay.Blood.P_Melee_Blood', 5, FALSE );

	CreatePS_List(WOP_BLOOD_LOD,				ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD', MAX_IMPACT_PARTICLE_SYSTEMS, TRUE );
	CreatePS_List(WOP_BLOOD_EXIT_WOUND,		ParticleSystem'Effects_Gameplay.Blood.P_Blood_Exit_Wound', MAX_IMPACT_PARTICLE_SYSTEMS_SMALL, TRUE );
	CreatePS_List(WOP_BLOOD_GENERAL,			ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD', MAX_IMPACT_PARTICLE_SYSTEMS, TRUE );
}


simulated final function CreateEmitter_List(int EffectType, ParticleSystem PSTemplate, int NumToCreate, bool bForceCreate )
{
	local int Idx;
	local Emitter Emit;

	EmitterCache[EffectType].SizeOfList = NumToCreate;
	EmitterCache[EffectType].Type = EffectType;
	EmitterCache[EffectType].ParticleSystemType = PSTemplate;

	if( bForceCreate == TRUE )
	{
		for( Idx = 0; Idx < NumToCreate; Idx++)
		{
			Emit = EmitterCache[EffectType].List[Idx];
			CreateEmitter_Worker( EffectType, PSTemplate, Emit );
			EmitterCache[EffectType].List[Idx] = Emit;
		}
	}
}



simulated final function CreateEmitter_Worker(int EffectType, ParticleSystem PSTemplate, out Emitter Emit )
{
	local int Idx;

	`if(`notdefined(FINAL_RELEASE))
	if( PSTemplate == None )
	{
		`warn( GetFuncName() @ "PSTemplate was" @ PSTemplate );
		ScriptTrace();
	}
	`endif


	switch (EffectType)
	{
	default:
		if( PSTemplate.bUseFixedRelativeBoundingBox == FALSE )
		{
			PSTemplate.FixedRelativeBoundingBox.IsValid = 1;
			PSTemplate.FixedRelativeBoundingBox.Min.X = -64;
			PSTemplate.FixedRelativeBoundingBox.Min.Y = -64;
			PSTemplate.FixedRelativeBoundingBox.Min.Z = -64;
			PSTemplate.FixedRelativeBoundingBox.Max.X =  64;
			PSTemplate.FixedRelativeBoundingBox.Max.Y =  64;
			PSTemplate.FixedRelativeBoundingBox.Max.Z =  64;

			PSTemplate.bUseFixedRelativeBoundingBox = TRUE;

			//`warn( "bUseFixedRelativeBoundingBox==FALSE PSTemplate was" @ PSTemplate );
		}
		break;
	}

	Emit = Spawn( class'SpawnedGearEmitter', self );
	Emit.LifeSpan = 0;
	GearEmitter(Emit).HideSelf();
	Emit.ParticleSystemComponent.bAutoActivate = FALSE;
	Emit.ParticleSystemComponent.OnSystemFinished = SpawnedGearEmitter(Emit).HideBecauseFinished;
	Emit.SetTemplate( PSTemplate, FALSE );


	Emit.ParticleSystemComponent.LODMethod		    = PARTICLESYSTEMLODMETHOD_DirectSet;
	Emit.ParticleSystemComponent.bOverrideLODMethod = TRUE;

	for( Idx = 0; Idx < Emit.ParticleSystemComponent.EmitterInstances.Length; Idx++)
	{
		Emit.ParticleSystemComponent.SetKillOnDeactivate( Idx, FALSE );
		Emit.ParticleSystemComponent.SetKillOnCompleted( Idx, FALSE );
	}
}


simulated final function CreatePS_List(int EffectType, ParticleSystem PSTemplate, int NumToCreate, bool bForceCreate )
{
	local int Idx;
	local ParticleSystemComponent PSC;

	EmitterCache[EffectType].SizeOfList = NumToCreate;
	EmitterCache[EffectType].Type = EffectType;
	EmitterCache[EffectType].ParticleSystemType = PSTemplate;

	if (bForceCreate)
	{
	 	for( Idx = 0; Idx < NumToCreate; Idx++)
	 	{
	 		PSC = EmitterCache[EffectType].PList[Idx];
	 		CreatePS_Worker( EffectType, PSTemplate, PSC );
	 		EmitterCache[EffectType].PList[Idx] = PSC;
	 	}
	}
}


simulated final function CreatePS_Worker(int EffectType, ParticleSystem PSTemplate, out ParticleSystemComponent PSC )
{
	local int Idx;

	`if(`notdefined(FINAL_RELEASE))
	if( PSTemplate == None )
	{
		`warn( GetFuncName() @ "PSTemplate was" @ PSTemplate );
		ScriptTrace();
	}
	`endif


	switch (EffectType)
	{
	default:
		if( PSTemplate.bUseFixedRelativeBoundingBox == FALSE )
		{
			PSTemplate.FixedRelativeBoundingBox.IsValid = 1;
			PSTemplate.FixedRelativeBoundingBox.Min.X = -64;
			PSTemplate.FixedRelativeBoundingBox.Min.Y = -64;
			PSTemplate.FixedRelativeBoundingBox.Min.Z = -64;
			PSTemplate.FixedRelativeBoundingBox.Max.X =  64;
			PSTemplate.FixedRelativeBoundingBox.Max.Y =  64;
			PSTemplate.FixedRelativeBoundingBox.Max.Z =  64;

			PSTemplate.bUseFixedRelativeBoundingBox = TRUE;

			//`warn( "bUseFixedRelativeBoundingBox==FALSE PSTemplate was" @ PSTemplate );
		}
		break;
	}


	PSC = new(self) class'ParticleSystemComponent';
	PSC.bAutoActivate = FALSE;
	PSC.SetTemplate( PSTemplate );

	PSC.LODMethod				= PARTICLESYSTEMLODMETHOD_DirectSet;
	PSC.bIsCachedInPool			= TRUE;
	PSC.bOverrideLODMethod      = TRUE;
	PSC.SetTickGroup( TG_PostUpdateWork ); // this is needed to make certain ParticleSystemComponents get attached at the correct location on physicy skeletal meshes
	PSC.OnSystemFinished = PoolPSCFinished;

	for( Idx = 0; Idx < PSC.EmitterInstances.Length; Idx++ )
	{
		PSC.SetKillOnDeactivate( Idx, FALSE );
		PSC.SetKillOnCompleted( Idx, FALSE );
	}
}

simulated final function PoolPSCFinished(ParticleSystemComponent FinishedComponent)
{
	FinishedComponent.DetachFromAny();
}

/**
 * This will retrieve an Emitter from our pool.
 *
 * NOTE: turn this into an object pool where we cache the various types of particle systems
 **/
simulated final function Emitter GetImpactEmitter(ParticleSystem PS_Type, vector SpawnLocation, rotator SpawnRotation)
{
	local Emitter Result;
	local EmitterCacheDatum Datum;
	local int PoolIndex;

	if( PS_Type == ParticleSystem'War_BulletEffects.FX.FX_P_BulletTrail_Normal' )
	{
		PoolIndex = WOP_WEAPON_TRACER_SMOKETRAIL;
	}
	else if( PS_Type == ParticleSystem'War_BulletEffects.FX.FX_P_BulletTrail_HighPower' )
	{
		PoolIndex = WOP_WEAPON_TRACER_SMOKETRAIL_AR;
	}

	else if( PS_Type == ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT;
	}
	else if( PS_Type == ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_AR' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_AR;
	}

	else if( PS_Type == ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_CHEAP' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_CHEAP;
	}

	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Blood_groundimpact_bodypart' )
	{
		PoolIndex = WOP_BLOOD_GROUND_IMPACT;
	}

	else if( PS_Type == ParticleSystem'GOW_Fractured_Objects.Effects.P_Fracture_Concrete_Grey_Medium' )
	{
		PoolIndex = WOP_FRACTURE_CONCRETE_MEDIUM;
	}

	else if( PS_Type == ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_Metal_Solid' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_METAL;
	}
	else if( PS_Type == ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_Wood_new' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_WOOD;
	}

	else if( PS_Type == ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Impact' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST;
	}
	else if( PS_Type == ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Impact_AR' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_AR;
	}

	else if( PS_Type == ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Impact_Metal_Solid' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_METAL;
	}
	else if( PS_Type == ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Impact_Wood' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_LOCUST_HAMMERBURST_WOOD;
	}

	else if( PS_Type == ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_Impact' )
	{
		PoolIndex = WOP_DEFAULT_SHOTGUN;
	}

	else if( PS_Type == ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_Impact_Metal' )
	{
		PoolIndex = WOP_DEFAULT_SHOTGUN_METAL;
	}
	else if( PS_Type == ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_Impact_Wood' )
	{
		PoolIndex = WOP_DEFAULT_SHOTGUN_WOOD;
	}

	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD_Death' )
	{
		PoolIndex = WOP_BLOOD_DEATH_LOD;
	}


	else if( PS_Type == ParticleSystem'COG_GatlingGun.Effects.P_COG_Gatling_Impact_Metal' )
	{
		PoolIndex = WOP_DEFAULT_MULCHER_METAL;
	}
	else if( PS_Type == ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Impact_Stone' )
	{
		PoolIndex = WOP_DEFAULT_MULCHER_STONE;
	}

	else if( PS_Type == ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Tracer' )
	{
		PoolIndex = WOP_DEFAULT_MULCHER_TRACER_SMOKETRAIL;
	}
	else if( PS_Type == ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Tracer_AR' )
	{
		PoolIndex = WOP_DEFAULT_MULCHER_TRACER_SMOKETRAIL_AR;
	}

	else if( PS_Type == ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Cover_Hit' )
	{
		PoolIndex = WOP_COVER_HIT;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Slide' )
	{
		PoolIndex = WOP_PLAYER_SLIDE;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Roll_Attached' )
	{
		PoolIndex = WOP_PLAYER_ROLL;
	}

	else if( PS_Type == ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_Derrick_Metal_Moving' )
	{
		PoolIndex = WOP_DEFAULT_P_TroikaCabal_Impact_Stone;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Melee_Blood' )
	{
		PoolIndex = WOP_BLOOD_MELEE_EMITTER;
	}
	else if( PS_Type == ParticleSystem'Locust_Brumak.Effects.P_Brumak_Gun_Impact' )
	{
		PoolIndex = WOP_PLAYER_BRUMAK_IMPACTS;
	}

	else
	{
		//`log( "ParticleSystem NOT CACHED: GetImpactEmitter PS_Type: " $ PS_Type );
		PoolIndex = WOP_DEFAULT_GENERAL;
	}


	Datum = EmitterCache[PoolIndex];
	if( PoolIndex != WOP_DEFAULT_GENERAL )
	{
		Result = GetImpactEmitter_Worker( Datum, SpawnLocation, SpawnRotation, none );
	}
	else // we are using the general pool so send in the PS_Type
	{
		Result = GetImpactEmitter_Worker( Datum, SpawnLocation, SpawnRotation, PS_Type );
	}

	EmitterCache[PoolIndex] = Datum;

	//`log( "Emitter PS_Type: " $ PS_Type @ Result.ParticleSystemComponent.Template.bUseFixedRelativeBoundingBox );
	//ScriptTrace();
	return Result;
}


simulated final function Emitter GetImpactEmitter_Worker( out EmitterCacheDatum TheDatum, vector SpawnLocation, rotator SpawnRotation, ParticleSystem PS_Type )
{
	local Emitter Emit;
	local int Jdx;

	if( ++TheDatum.ListIdx >= TheDatum.SizeOfList )
	{
		TheDatum.ListIdx = 0;
	}

	Emit = TheDatum.List[TheDatum.ListIdx];

	// Spawn a new component if the one in the cache has been destroyed. This can happen if the actor the component is attached to
	// gets destroyed and henceforth is marked as pending kill.
	// @todo also prob need to check if NOT RF_Unreachable  (e.g. the outer of this PSC was somehow destroyed and is not marked RF_PendingKill yet)
	if( Emit == none || Emit.bDeleteMe )
	{
		if( PS_Type == none )
		{
			CreateEmitter_Worker( TheDatum.Type, TheDatum.ParticleSystemType, Emit );
			//`log( "newing a Emit of type CACHED: " $  TheDatum.ParticleSystemType );
		}
		else
		{
			CreateEmitter_Worker( TheDatum.Type, PS_Type, Emit );
			//`log( "newing a Emit of type GENERAL: " $  PS_Type );
		}

		TheDatum.List[TheDatum.ListIdx] = Emit;
	}

	Emit.bStasis = FALSE;

	Emit.SetBase( none );

	//Emit.SetAbsolute( TRUE, TRUE, FALSE );
	//Emit.SetTranslation( SpawnLocation );
	//Emit.SetRotation( SpawnRotation );

	Emit.SetLocation( SpawnLocation );
	Emit.SetRotation( SpawnRotation );


	Emit.SetDrawScale( 1.0 ); // reset this in case caller changed it

	// we check to see if we are getting a non default impact and then if we are
	// we have to SetTemplate on it as the set of all none defaults share their
	// own cache
	if( PS_Type != none )
	{
		Emit.SetTemplate( PS_Type, FALSE ); // this will ActivateSystem if you do not explicitly set ParticleSystemComponent.bAutoActivate = FALSE;
		//`log( "GetImpactEmitter_Worker:  having to SetTemplate :-(  Previous: " $ Emit.ParticleSystemComponent.Template $ " New: " $ PS_Type $ PS_Type.bUseFixedRelativeBoundingBox );
		for( Jdx = 0; Jdx < Emit.ParticleSystemComponent.EmitterInstances.Length; ++Jdx )
		{
			Emit.ParticleSystemComponent.SetKillOnDeactivate(Jdx,FALSE);
			Emit.ParticleSystemComponent.SetKillOnCompleted(Jdx,FALSE);
		}

	}

	Emit.ClearTimer( 'HideSelf' );
	Emit.SetHidden(FALSE);

	//`log( "GetImpactEmitter_Worker: " $ TheDatum.ListIdx $ " PS_Type: " $ PS_Type );

	//Emit.ParticleSystemComponent.bIsCachedInPool = TRUE;

	Emit.ParticleSystemComponent.SetLODLevel( GearGRI(WorldInfo.GRI).GetLODLevelToUse(Emit.ParticleSystemComponent.Template, SpawnLocation ));

	// global safety catch for misbehaving particles.  Usually the OnSystemFinished will do the correct thing
	Emit.SetTimer( 15.0f, FALSE, nameof(Emit.HideSelf) );

	return Emit;
}


simulated final function ParticleSystemComponent GetImpactParticleSystemComponent( ParticleSystem PS_Type )
{
	local ParticleSystemComponent Result;
	local EmitterCacheDatum Datum;
	local int PoolIndex;

	if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD' )
	{
		PoolIndex = WOP_BLOOD_LOD;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Blood_Exit_Wound' )
	{
		PoolIndex = WOP_BLOOD_EXIT_WOUND;
	}

	else if( PS_Type == ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_Impact_CHEAP' )
	{
		PoolIndex = WOP_DEFAULT_IMPACT_CHEAP;
	}

	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD_Metal' )
	{
		PoolIndex = WOP_BLOOD_METAL_LOD;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD_Armor' )
	{
		PoolIndex = WOP_BLOOD_ARMOR_LOD;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Blood_flying_bodypart' )
	{
		PoolIndex = WOP_BLOOD_FLYING_BODY_PART;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Bloodspray_hit_effect' )
	{
		PoolIndex = WOP_BLOOD_SPRAY_HIT_EFFECT;
	}
	else if( PS_Type == ParticleSystem' Effects_Gameplay.Blood.P_Blood_HeadShot' )
	{
		PoolIndex = WOP_BLOOD_HEADSHOT;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Hit_Effect_No_Blood' )
	{
		PoolIndex = WOP_NO_BLOOD;
	}
	else if( PS_Type == ParticleSystem'Locust_Berserker.Particles.P_Berserker_Impact_Weapon_Sparks' )
	{
		PoolIndex = WOP_BERSERKER_IMPACT;
	}
	else if( PS_Type == ParticleSystem'Effects_Gameplay.Blood.P_Melee_Blood' )
	{
		PoolIndex = WOP_BLOOD_MELEE;
	}
	else
	{
		//`log( "ParticleSystem NOT CACHED: ParticleSystemComponent PS_Type: " $ PS_Type );
		PoolIndex = WOP_BLOOD_GENERAL;
	}


	Datum = EmitterCache[PoolIndex];
	if( PoolIndex != WOP_BLOOD_GENERAL )
	{
		Result = GetImpactParticleSystemComponent_Worker( Datum, none );
	}
	else // we are using the general pool so send in the PS_Type
	{
		Result = GetImpactParticleSystemComponent_Worker( Datum, PS_Type );
	}
	EmitterCache[PoolIndex] = Datum;

	//`log( "PSC PS_Type: " $ PS_Type @ Result.Template.bUseFixedRelativeBoundingBox );
	//ScriptTrace();
	return Result;
}


// need to do this as the particle components are attached to the mob
simulated final function ParticleSystemComponent GetImpactParticleSystemComponent_Worker( out EmitterCacheDatum TheDatum, ParticleSystem PS_Type )
{
	local ParticleSystemComponent PSC;
	local int Jdx;

	if( ++TheDatum.ListIdx >= TheDatum.SizeOfList  )
	{
		TheDatum.ListIdx = 0;
	}

	PSC = TheDatum.PList[TheDatum.ListIdx];

	// Spawn a new component if the one in the cache has been destroyed. This can happen if the actor the component is attached to
	// gets destroyed and henceforth is marked as pending kill.
	// @todo also prob need to check if NOT RF_Unreachable  (e.g. the outer of this PSC was somehow destroyed and is not marked RF_PendingKill yet)
	if( PSC == none || PSC.IsPendingKill() )
	{
		if( PS_Type == none )
		{
			CreatePS_Worker( TheDatum.Type, TheDatum.ParticleSystemType, PSC );
			//`log( "newing a PSC of type CACHED: " $  TheDatum.ParticleSystemType );
		}
		else
		{
			CreatePS_Worker( TheDatum.Type, PS_Type, PSC );
			//`log( "newing a PSC of type GENERAL: " $  PS_Type );
		}
		// Replace entry in cache.

		TheDatum.PList[TheDatum.ListIdx] = PSC;
		//`log( "newing a PSC of type: " $  TheDatum.ParticleSystemType  $ " PSC: " $ PSC.Template );
	}

	// Detach from existing actor if we're re-using.
	if( PSC.Owner != none )
	{
		//`log( "Detaching component from: " $ PSC.Owner $ " PSC: " $ PSC.Template );
		PSC.Owner.DetachComponent( PSC );
	}

	PSC.SetHidden( FALSE );

	if( PS_Type != none )
	{
		//`log( "GetImpactParticleSystemComponent_Worker:  having to SetTemplate :-(  Previous: " $ PSC.Template $ " New: " $ PS_Type  $ PS_Type.bUseFixedRelativeBoundingBox );
		PSC.SetTemplate( PS_Type );

		for( Jdx = 0; Jdx < PSC.EmitterInstances.Length; ++Jdx )
		{
			PSC.SetKillOnDeactivate(Jdx,FALSE);
			PSC.SetKillOnCompleted(Jdx,FALSE);
		}

	}

	//`log( TheDatum.ListIdx @ TheDatum.SizeOfList );

	//PSC.OnSystemFinished = OnParticleSystemFinished_Hider;


	return PSC;
}


/**
 * Get a rain emitter from our "object pool"
 **/
simulated final function Emitter GetRainEmitter_Self( vector SpawnLocation, rotator SpawnRotation )
{
	if (++RainDropSelfEmitterListIdx >= MAX_RAINDROP_EMITTERS)
	{
		RainDropSelfEmitterListIdx = 0;
	}

	if( RainDropSelfEmitterList[RainDropSelfEmitterListIdx] == none )
	{
		RainDropSelfEmitterList[RainDropSelfEmitterListIdx] = Spawn( class'SpawnedGearEmitter', self,, SpawnLocation*vect(0,0,-1), );
		RainDropSelfEmitterList[RainDropSelfEmitterListIdx].LifeSpan = 0;
		RainDropSelfEmitterList[RainDropSelfEmitterListIdx].ParticleSystemComponent.bAutoActivate = FALSE;
		RainDropSelfEmitterList[RainDropSelfEmitterListIdx].SetTemplate(ParticleSystem'Effects_Gameplay.Rain.P_FX_Rain_Player_Hit_01', FALSE);
	}

	// these emitters are always going to be valid as we spawn them immediately in PostBeginPlay
	RainDropSelfEmitterList[RainDropSelfEmitterListIdx].SetLocation(SpawnLocation);
	RainDropSelfEmitterList[RainDropSelfEmitterListIdx].SetRotation(SpawnRotation);


	return RainDropSelfEmitterList[RainDropSelfEmitterListIdx];
}

/**
 * Get a hail emitter from our "object pool"
 **/
simulated final function Emitter GetHailImpactEmitter_Pawn( vector SpawnLocation, rotator SpawnRotation )
{
	if (++HailImpactPawnEmitterListIdx >= MAX_HAILIMPACT_EMITTERS)
	{
		HailImpactPawnEmitterListIdx = 0;
	}

	if( HailImpactPawnEmitterList[HailImpactPawnEmitterListIdx] == None )
	{
		HailImpactPawnEmitterList[HailImpactPawnEmitterListIdx] = Spawn( class'SpawnedGearEmitter', self,, SpawnLocation*vect(0,0,-1), );
		HailImpactPawnEmitterList[HailImpactPawnEmitterListIdx].LifeSpan = 0;
		HailImpactPawnEmitterList[HailImpactPawnEmitterListIdx].ParticleSystemComponent.bAutoActivate = FALSE;
		HailImpactPawnEmitterList[HailImpactPawnEmitterListIdx].SetTemplate(ParticleSystem'War_HailEffects.Effects.P_FX_Hail_PlayerImpact', FALSE);
	}

	// these emitters are always going to be valid as we spawn them immediately in PostBeginPlay
	HailImpactPawnEmitterList[HailImpactPawnEmitterListIdx].SetLocation(SpawnLocation);
	HailImpactPawnEmitterList[HailImpactPawnEmitterListIdx].SetRotation(SpawnRotation);

	return HailImpactPawnEmitterList[HailImpactPawnEmitterListIdx];
}



simulated final function OnParticleSystemFinished_Hider(ParticleSystemComponent FinishedComponent)
{
	FinishedComponent.SetHidden( TRUE );
}



//
// KActors Pool
//

simulated final function CreateKActors()
{
	local int				Idx;
	local vector			SpawnLocation;

	// 262144 is HALF_WORLD_MAX @see engine.h
	// reduced a bit from that so that it doesn't get destroyed due to being out of the world
	SpawnLocation = vect(250000,250000,250000);

	for( Idx=0; Idx<MAX_KACTORS; Idx++ )
	{
		if (KActorCache.List[Idx] == None)
		{
			KActorCache.List[Idx] = Spawn( class'KActorSpawnable', self,, SpawnLocation );
			KActorCache.List[Idx].RemoteRole = ROLE_None;
			KActorCache.List[Idx].SetCollision(FALSE, FALSE); // Don't let these actors block players
			KActorCache.List[Idx].bCollideWorld = FALSE;
			KActorCache.List[Idx].bBlocksNavigation = FALSE;
			KActorCache.List[Idx].bNoEncroachCheck = TRUE;

			KActorCache.List[Idx].LightEnvironment.SetEnabled( FALSE );
			KActorCache.List[Idx].LightEnvironment.bCastShadows = FALSE;
			KActorCache.List[Idx].StaticMeshComponent.CastShadow = FALSE; // turn off shadow casting for perf

			KActorCache.List[Idx].StaticMeshComponent.SetLightEnvironment( KActorCache.List[Idx].LightEnvironment );

			KActorCache.List[Idx].Recycle();
			KActorCache.List[Idx].bRecycleScaleToZero = TRUE;
		}
	}
}


simulated final function KActorSpawnable GetKActorSpawnable(Vector SpawnLocation, Rotator SpawnRotation)
{
	local KActorSpawnable KA;
	local int idx;

	if( ++KActorCache.ListIdx >= MAX_KACTORS )
	{
		KActorCache.ListIdx = 0;
	}

	if (KActorCache.List[KActorCache.ListIdx] == None)
	{
		CreateKActors();
	}

	KA = KActorCache.List[KActorCache.ListIdx];

	if( KA != None )
	{
		if( KA.IsTimerActive('Recycle') || KA.bRecycleScaleToZero )
		{
			KA.RecycleInternal();
		}

		bStasis = FALSE;
		KA.LightEnvironment.bDynamic = TRUE; // this will have been turned off by SetLightEnvironmentToNotBeDynamic
		KA.LightEnvironment.SetEnabled( TRUE );
		KA.SetCollision(FALSE, FALSE); // Don't let these actors block players
		KA.bCollideWorld = FALSE;
		KA.bBlocksNavigation = FALSE;
		KA.bNoEncroachCheck = TRUE;

		KA.SetLocation(SpawnLocation);
		KA.SetRotation(SpawnRotation);
		KA.StaticMeshComponent.SetRBLinearVelocity( Vect(0,0,0) );
		KA.StaticMeshComponent.SetRBAngularVelocity( Vect(0,0,0) );
		KA.StaticMeshComponent.SetRBPosition( SpawnLocation );
		KA.StaticMeshComponent.SetRBRotation( SpawnRotation );

		// need to reset the materials here in case people have used them at he caller level
		for( Idx = 0; Idx < KA.StaticMeshComponent.Materials.Length; ++Idx )
		{
			//`log( "clearing material to make gore work" @ Mesh.Materials[Idx] );
			KA.StaticMeshComponent.SetMaterial( Idx, None );
		}

		KA.ResetComponents();
		KA.Initialize();
	}

	//`log( KA @ KA.default.DrawScale @ SpawnLocation @ SpawnRotation );
	return KA;
}


simulated final function CleanUpKActors()
{
	local int Idx;

	for( Idx=0; Idx<MAX_KACTORS; Idx++ )
	{
		if( KActorCache.List[Idx] != none )
		{
			KActorCache.List[Idx].Destroy();
			KActorCache.List[Idx] = none;
		}
	}
}


/** This will create the various lists of decal caches **/
simulated final function CreateDecals()
{
	local int Idx;
	local GearDecal GD;
	local DecalDatum DD;


	DecalPool_Blood.MAX_DECALS = 100;
	DecalPool_Blood.MAX_DECALS_bDropDetail = 80;
	DecalPool_Blood.MAX_DECALS_bAggressiveLOD = 50;

	DecalPool_Bullet.MAX_DECALS = 150;
	DecalPool_Bullet.MAX_DECALS_bDropDetail = 120;
	DecalPool_Bullet.MAX_DECALS_bAggressiveLOD = 80;

	DecalPool_Explosion.MAX_DECALS = 15;
	DecalPool_Explosion.MAX_DECALS_bDropDetail = 10;
	DecalPool_Explosion.MAX_DECALS_bAggressiveLOD = 10;



	for( Idx = 0; Idx < DecalPool_Blood.MAX_DECALS; ++Idx )
	{
		GD = new(self) class'GearDecal';
		GD.MITV_Decal = new(GD) class'MaterialInstanceTimeVarying';
		DecalPool_Blood.Decals[DecalPool_Blood.Decals.Length] = DD;
		DecalPool_Blood.Decals[Idx].GD = GD;
		DecalPool_Blood.Decals[Idx].CanSpawnDistance = 10.0f;
	}

	for( Idx = 0; Idx < DecalPool_Bullet.MAX_DECALS; ++Idx )
	{
		GD = new(self) class'GearDecal';
		GD.MITV_Decal = new(GD) class'MaterialInstanceTimeVarying';
		DecalPool_Bullet.Decals[DecalPool_Bullet.Decals.Length] = DD;
		DecalPool_Bullet.Decals[Idx].GD = GD;
		DecalPool_Bullet.Decals[Idx].CanSpawnDistance = 5.0f;
	}

	for( Idx = 0; Idx < DecalPool_Explosion.MAX_DECALS; ++Idx )
	{
		GD = new(self) class'GearDecal';
		GD.MITV_Decal = new(GD) class'MaterialInstanceTimeVarying';
		DecalPool_Explosion.Decals[DecalPool_Explosion.Decals.Length] = DD;
		DecalPool_Explosion.Decals[Idx].GD = GD;
		DecalPool_Explosion.Decals[Idx].CanSpawnDistance = 20.0f;
	}

}



simulated final function GearDecal GetDecal_Blood( const out vector SpawnLocation )
{
	return GetDecal_Worker( DecalPool_Blood.DecalsIdx, DecalPool_Blood, DecalPool_Blood.Decals, SpawnLocation );
}

simulated final function GearDecal GetDecal_Bullet( const out vector SpawnLocation )
{
	return GetDecal_Worker( DecalPool_Bullet.DecalsIdx, DecalPool_Bullet, DecalPool_Bullet.Decals, SpawnLocation );
}

simulated final function GearDecal GetDecal_Explosion( const out vector SpawnLocation )
{
	return GetDecal_Worker( DecalPool_Explosion.DecalsIdx, DecalPool_Explosion, DecalPool_Explosion.Decals, SpawnLocation );
}


/**
 * This is our generic Decal Cache worker.  All of our indiv functions call this so we can modify it as we want and everyone will
 * get the benefit.
 *
 **/
private native simulated final function GearDecal GetDecal_Worker( out int CurrIdx, const out DecalPool TheDecalPool, out array<DecalDatum> DecalList, const out vector SpawnLocation );


/**
 * populate the array for the path constraint cache
 */
final function CreatePathConstraintCache()
{
	PathConstraintCache.length = NUM_CONSTAINT_CLASSES;
	PathGoalEvaluatorCache.length = MAX_GOALEVALUATORS;
}

final function CleanupPathConstraints()
{
	PathConstraintCache.length=0;
	PathGoalEvaluatorCache.length=0;
}

final function PathConstraint GetPathConstraintFromCache(class<PathConstraint> ConstraintClass, Pawn Requestor)
{
	local int CacheIdx;
	local PathConstraintCacheDatum PDC;
	local PathConstraint ChosenConstraint;

	if( PathConstraintCache.length == 0 )
	{
		CreatePathConstraintCache();
	}


	CacheIdx = ConstraintClass.default.CacheIdx;
	if(CacheIdx < 0 || CacheIdx > NUM_CONSTAINT_CLASSES)
	{
		`if(`notdefined(FINAL_RELEASE))
			`log("OH NOES! Couldn't get path constraint from pool for "$Requestor$"!! Class was "$ConstraintClass$"CacheIdx:"@CacheIdx@"Numclasses:"@NUM_CONSTAINT_CLASSES);
			ScriptTrace();
		`endif
		return new ConstraintClass;
	}

	PDC = PathConstraintCache[CacheIdx];
	if(++PathConstraintCache[CacheIdx].ListIdx >= MAX_INSTANCES_PER_CLASS)
	{
		PathConstraintCache[CacheIdx].ListIdx=0;
	}

	ChosenConstraint = PDC.List[PDC.ListIdx];
	if(ChosenConstraint == none || ChosenConstraint.IsPendingKill())
	{
		//`log(Requestor@"Pool entry was empty for class"@ConstraintClass@CacheIdx@" newing a new constraint..."@PathConstraintCache[CacheIdx].ListIdx);
		ChosenConstraint = new ConstraintClass;
		PDC.List[PDC.ListIdx] = ChosenConstraint;
	}
	else
	{
		//`log(Requestor@"Using"@PathConstraintCache[CacheIdx].List[PathConstraintCache[CacheIdx].ListIdx]@"from pool.. requestedclass:"@ConstraintClass@CacheIdx@PathConstraintCache[CacheIdx].ListIdx);
	}

	ChosenConstraint.Recycle();
	return ChosenConstraint;
}

final function PathGoalEvaluator GetPathGoalEvaluatorFromCache(class<PathGoalEvaluator> GoalEvalClass, Pawn Requestor)
{
	local int CacheIdx;
	local PathGoalEvaluator ChosenEval;

	if( PathGoalEvaluatorCache.length == 0 )
	{
		CreatePathConstraintCache();
	}


	CacheIdx = GoalEvalClass.default.CacheIdx;
	if(CacheIdx < 0 || CacheIdx > MAX_GOALEVALUATORS)
	{
		`if(`notdefined(FINAL_RELEASE))
			`log("OH NOES! Couldn't get path goal evaluator from pool for "$Requestor$"!! Class was "$GoalEvalClass$"CacheIdx:"@CacheIdx@"MaxGoalEvals:"@MAX_GOALEVALUATORS);
			ScriptTrace();
		`endif
		return new GoalEvalClass;
	}

	ChosenEval = PathGoalEvaluatorCache[CacheIdx];
	if(ChosenEval == none || ChosenEval.IsPendingKill())
	{
		//`log(Requestor@"Pool was empty for class"@GoalEvalClass@CacheIdx@" newing a new goal evaluator...");
		ChosenEval = new GoalEvalClass;
		PathGoalEvaluatorCache[CacheIdx] = ChosenEval;
	}
	else
	{
		//`log(Requestor@"Using"@PathGoalEvaluatorCache[CacheIdx]@"from pool.. requestedclass:"@GoalEvalClass@CacheIdx);
	}

	ChosenEval.Recycle();
	return ChosenEval;
}


defaultproperties
{
	TickGroup=TG_PreAsyncWork // can't be TG_DuringAsyncWork since it uses a timer that spawns stuff

	bHidden=TRUE
	bMovable=FALSE

	RainDropSelfEmitterListIdx=-1
	HailImpactPawnEmitterListIdx=-1

	BulletMaterial=Material'COG_AssaultRifle.Materials.Mat_COG_Tracer_new'

// 	CachedStaticMeshes(00)=StaticMesh'COG_Gore.Head_Chunk1'
// 	CachedStaticMeshes(01)=StaticMesh'COG_Gore.Head_Chunk2'
// 	CachedStaticMeshes(02)=StaticMesh'COG_Gore.Head_Chunk3'
// 	CachedStaticMeshes(03)=StaticMesh'COG_Gore.Head_Chunk4'
// 	CachedStaticMeshes(04)=StaticMesh'COG_Gore.Head_Chunk5'
// 	CachedStaticMeshes(05)=StaticMesh'COG_Gore.Head_Chunk6'
// 	CachedStaticMeshes(06)=StaticMesh'COG_Gore.Head_Chunk7'
// 	CachedStaticMeshes(07)=StaticMesh'COG_Gore.Head_Chunk8'
// 	CachedStaticMeshes(08)=StaticMesh'COG_Gore.Head_Chunk9'
// 	CachedStaticMeshes(09)=StaticMesh'COG_Gore.Head_Chunk10'
// 	CachedStaticMeshes(10)=StaticMesh'COG_Gore.Head_Chunk11'
// 	CachedStaticMeshes(11)=StaticMesh'COG_Gore.Head_Chunk12'
// 	CachedStaticMeshes(12)=StaticMesh'COG_Gore.Head_Chunk13'
// 	CachedStaticMeshes(13)=StaticMesh'COG_Gore.Head_Chunk14'
// 	CachedStaticMeshes(14)=StaticMesh'COG_Gore.Head_Chunk15'
// 	CachedStaticMeshes(15)=StaticMesh'COG_AssaultRifle.Assault_Rifle_Magazine'
// 	CachedStaticMeshes(16)=StaticMesh'Locust_Hammerburst.Hammerburst_Magazine'
// 	CachedStaticMeshes(17)=StaticMesh'Locust_Boomshot.Mesh.Locust_Boomshot_Magazine'
//
// 	// cache the helmets' collision
//
// 	CachedStaticMeshes(18)=StaticMesh'COG_MarcusFenix.Helmet1.COG_Helmet1_PH_SMesh'
// 	CachedStaticMeshes(19)=StaticMesh'COG_Redshirt.cog_helmet_alt2'
// 	CachedStaticMeshes(20)=StaticMesh'COG_Redshirt.cog_helmet_alt3'
//
// 	CachedStaticMeshes(21)=StaticMesh'Locust_Theron_Guard.Locust_TheronHelmet1'
// 	CachedStaticMeshes(22)=StaticMesh'Locust_Theron_Guard.Locust_TheronHelmet2'
// 	CachedStaticMeshes(23)=StaticMesh'Locust_Theron_Guard.Locust_TheronHelmet3'
//
// 	CachedStaticMeshes(24)=StaticMesh'Locust_Grunt.locust_grunt_helmet_soc_all'
// 	CachedStaticMeshes(25)=StaticMesh'Locust_Grunt.locust_grunt_helmet_soc_mask'
// 	CachedStaticMeshes(26)=StaticMesh'Locust_Grunt.locust_grunt_helmet_soc_mouth'
// 	CachedStaticMeshes(27)=StaticMesh'Locust_Grunt.locust_grunt_helmet_soc_none'
// 	CachedStaticMeshes(28)=StaticMesh'Locust_Grunt.Mesh.Locust_Grunt_Goggles_Down_Mesh'
// 	CachedStaticMeshes(29)= StaticMesh'Locust_Grunt.Mesh.Locust_Grunt_Goggles_UP_Mesh'
// 	CachedStaticMeshes(30)=StaticMesh'Locust_Grunt.Mesh.Locust_Grunt_Goggles_Down_Mesh'

}


//need to run with logging on to see which pscs need to be cached that are not




