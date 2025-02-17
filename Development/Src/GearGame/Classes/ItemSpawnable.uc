/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class ItemSpawnable extends KActorSpawnable
	abstract;

/** This is the struct for a spawnable item **/
struct SpawnableItemDatum
{
	/** 
	 * The Socket name we are going to attach this to. 
	 * when we say: "head" we really mean the HeadSocketName as some guys have completely different
	 * notions/names for the socket that represents the "head".  So there is look up to handle that layer of indirection
	 * in GearInfantry.
	 * 
	 **/
	var name AttachSocketName;

	/** The actual StaticMesh to attach and to eventually probably spawn **/
	var StaticMesh TheMesh;

	/** The actual SkelMesh to attach and to eventually probably spawn **/
	var SkeletalMesh TheSkelMesh;
	var PhysicsAsset ThePhysAsset;


	/** Some of the helmets need to be translated up.  (e.g. goggles)  For the most part this should be 0,0,0 and the content should be fixed **/
	var vector Translation;

	/** This is the template for the Particle Effect that this item has **/
	var ParticleSystem PS_PersistentEffect;

	/** The scale to set this item to **/
	var float ItemScale;


	structdefaultproperties
	{
		ItemScale=1.0f;
	}
};

/** The item info for this node **/
var array<SpawnableItemDatum> SpawnableData;


/** This struct is used for the item types that are choosing randomly from the set of items **/
struct SpawnableClassesDatum
{
	/** The percentage to spawn this item. **/
	var float PercentageToSpawn;
	
	/** The class to use for the random choice **/
	var class<ItemSpawnable> ItemClass;
};

/** This is used for the Random types as we can't replicate out a "random" class to clients as the results will be random on all clients! **/
var array<SpawnableClassesDatum> SpawnableClassesData;

/** The sound to play when the item is shot off **/
var SoundCue BeingShotOffSound;

/** When the item hits the ground for the first time **/
var SoundCue ImpactingGround;



auto state TimeTilDestroySelf
{

begin:
	sleep( 0.1f );
	SetHidden(FALSE);

	sleep( 15.0f );
	Destroy();
}


/** Sound to play when the item is shot off **/
simulated function PlayShotOffSound()
{
	if( BeingShotOffSound != none )
	{
		PlaySound( BeingShotOffSound, TRUE, TRUE, TRUE, Location );
	}
}


/**  When helmet hits the ground play a sound and then turn off the RB collision notifications **/
event RigidBodyCollision( PrimitiveComponent HitComponent, PrimitiveComponent OtherComponent,
				const out CollisionImpactData RigidCollisionData, int ContactIndex )
{
	if( ImpactingGround != none )
	{
		PlaySound( ImpactingGround, TRUE, TRUE, TRUE, Location );
	}
	StaticMeshComponent.SetNotifyRigidBodyCollision( FALSE );
}


/** This will turn off collision of the item. **/
simulated function TurnCollisionOff()
{
	CollisionComponent.SetBlockRigidBody( FALSE );
	CollisionComponent.SetRBChannel( RBCC_Nothing ); // nothing will request to collide with us

	CollisionComponent.SetRBCollidesWithChannel( RBCC_Default, FALSE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_BlockingVolume, FALSE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_Pawn, FALSE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_Vehicle, FALSE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_GameplayPhysics, FALSE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_EffectPhysics, FALSE );

	ReattachComponent(CollisionComponent);
}


/** This will turn on collision of the item. **/
simulated function TurnCollisionOn()
{
	CollisionComponent.SetBlockRigidBody( TRUE );

	CollisionComponent.SetRBCollidesWithChannel( RBCC_Default, TRUE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_BlockingVolume, TRUE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_Pawn, TRUE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_Vehicle, TRUE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_GameplayPhysics, TRUE );
	CollisionComponent.SetRBCollidesWithChannel( RBCC_EffectPhysics, TRUE );

	ReattachComponent(CollisionComponent);

	CollisionComponent.WakeRigidBody();
}


/**
 * This will return the static mesh for this spawnable type.  We use this function
 * in order to push the resources needed for the helmets to the content package where possible.
 **/
simulated static function SpawnableItemDatum GetSpawnableItemDatum()
{
	// clever usage of of not having to duplicate code into each of the children classes as most will only have 1 helmet
	// for those that are different or need special handling then we just override this function
	return default.SpawnableData[0];
}


/**
* This will return the static mesh for this spawnable type.  We use this function
* in order to push the resources needed for the helmets to the content package where possible.
**/
simulated static function class<ItemSpawnable> GetSpawnableItemClass()
{
	// clever usage of of not having to duplicate code into each of the children classes as most will only have 1 helmet
	// for those that are different or need special handling then we just override this function
	if( default.SpawnableClassesData.length > 0 )
	{
		GetSpawnableItemClassRandom();
	}
	else
	{
		return default.class;
	}
}


/**
* This will return the static mesh for this spawnable type.  We use this function
* in order to push the resources needed for the helmets to the content package where possible.
**/
protected simulated static function class<ItemSpawnable> GetSpawnableItemClassRandom()
{
	local float Percent;
	local float CurrPercent;
	local int Idx;

	Percent = FRand();

	for( Idx = 0; Idx < default.SpawnableClassesData.length; ++Idx )
	{
		CurrPercent += default.SpawnableClassesData[Idx].PercentageToSpawn;

		if( Percent <= CurrPercent )
		{
			//`log( "item translation " $  default.SpawnableData[Idx].Translation );
			return default.SpawnableClassesData[Idx].ItemClass.static.GetSpawnableItemClass();
		}

	}
	// else return the 1st entry
	return default.SpawnableClassesData[0].ItemClass.static.GetSpawnableItemClass();
}



defaultproperties
{
	RemoteRole=ROLE_None
	Physics=PHYS_RigidBody

	// defaults for all spawnable items
	Begin Object Name=StaticMeshComponent0
		CastShadow=FALSE
		bCastDynamicShadow=FALSE

		BlockNonZeroExtent=FALSE
		CollideActors=TRUE
		BlockRigidBody=TRUE

		bNotifyRigidBodyCollision=TRUE
		ScriptRigidBodyCollisionThreshold=10.0
	End Object

	bNoEncroachCheck=TRUE
	bCollideWorld=FALSE  // we want the gib to use the rigidbody collision.  Setting this to TRUE means that unreal physics will try to control
	bCollideActors=TRUE
	bBlockActors=FALSE


}
