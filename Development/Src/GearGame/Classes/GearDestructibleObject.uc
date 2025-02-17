/**
 * This should be utilized to place an object in the world and then make
 * and archetype out of it.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GearDestructibleObject extends Actor
	placeable
	native;

/** Is this object chainsawable? */
var() protected bool bCanSpecialMeleeAttacked;

/** This sound is played all of the time the object is in the world.  (good for radios and such) **/
var() protected SoundCue		AmbientSoundCue;
var protected AudioComponent	AmbientSoundComponent;

/** attempt to fix editor bug, not working yet.  remove? */
var transient bool				bComponentsSetUp;

/** TRUE if WDO is only vulnerable to damagetypes in VulnerableToDamageTypes list */
var() bool						bLimitDamageTypes;

/** List of damage types that can affect this object.  Ignored if bLimitDamageTypes is false. */
var() array<class<DamageType> >	VulnerableToDamageType;

/** Light environment used by components of the GDO whose meshes have been changed (and therefore precomputed lighting is no longer valid) */
var() const LightEnvironmentComponent LightEnvironment;

/** Light environment shared by all actors spawned from this GDO, not actually used by the GDO itself */
var(GDOActorSpawnParams) const DynamicLightEnvironmentComponent LightEnvironmentToUseForActorSpawnParams;

/** Lighting channels used by all actors spawned from this GDO, not actually used by the GDO itself */
var(GDOActorSpawnParams) LightingChannelContainer LightingChannelsToUseForActorSpawnParams;

struct native MaterialReplaceMod
{
	var()	MaterialInterface	NewMaterial;

	/** Index in the Materials array to replace with NewMaterial when this action is activated. */
	var()	int					MaterialIndex;
};

struct native MaterialScalarParamMod
{
	var() MaterialInstanceConstant	MatInst;
	var() Name						ParamName;
	var() float						ScalarVal;
};

struct native MaterialTexParamMod
{
	var() MaterialInstanceConstant	MatInst;
	var() Name						ParamName;
	var() Texture					NewTexture;
};

struct native MaterialVectorParamMod
{
	var() MaterialInstanceConstant	MatInst;
	var() Name						ParamName;
	var() LinearColor				VectorVal;
};

/** Parameters to define an actor spawn. */
struct native ActorSpawnParams
{
	/** The actor factory doing the spawning */
	var() instanced ActorFactory	Factory;
	/** Positional offset for actor spawn loc.  Note this is relative to the subobject */
	var() vector					RelativeOffset;
	/** Rotational offset for actor spawn loc.  Note this is relative to the subobject  */
	var() rotator					RelativeRotation;
	/** How long before we forcibly destroy the spawned actor, in seconds.  <= 0 means don't destroy. */
	var() float						LifeTimeSeconds;

	structdefaultproperties
	{
		LifeTimeSeconds=30.0f
	}
};

struct native SplashDamageParams
{
	var() float				BaseDamage;
	var() float				DamageRadius;
	var() class<DamageType>	DamageType;
	var() float				Momentum;
};

struct native DestroyedEffectParams
{
	/** Emitters to spawn */
	var() ParticleSystem		ParticleEffect;
	/** Position offset for detroyed effect.  Note this is relative to the subobject  */
	var() vector				RelativeOffset;
	/** Rotational offset for destroyed effect.  Note this is relative to the subobject */
	var() rotator				RelativeRotation;
};

/** new collision params */
//struct native CollisionMod
//{
//	var() bool bNewCollideActors;
//	var() bool bNewBlockActors;
//	var() bool bNewIgnoreEncroachers;
//};

/** describes a dependency of one subobject on a modification to another */
struct native ObjDamageModifierDependency
{
	/** which subobject we're talking about */
	var() Name			DependentSubObjName;

	/** If dependent subobject is above this health, it will be damaged to this level */
	var() float			MaxHealthToAllow;

	/** Optimization to avoid searches.  Gets filled in on PostBeginPlay() */
	var transient int	DependentSubObjIdx;

	structdefaultproperties
	{
		DependentSubObjIdx=-1
	}
};

struct native CoverModParams
{
	/** index into AttachedCover array of which coverlink to modify */
	var() int		AttachedCoverIndex;

	/** set TRUE to disable this entire coverlink (SlotIndex is ignored) */
	var() bool		bDisableCoverLink;

	/** which slot to modify in the specified coverlink */
	var() int		SlotIndex;

	/** set TRUE to disable this cover slot */
	var() bool		bDisableSlot;

	/** If not CT_None, force slot to this type */
	var() ECoverType	NewCoverType;

	/** TRUE to cause the indicated vars in the cover slot to be updated */
	var() bool		bUpdateCanMantle;
	var() bool		bUpdateCanCoverSlip_Left;
	var() bool		bUpdateCanCoverSlip_Right;
	var() bool		bUpdateCanSwatTurn_Left;
	var() bool		bUpdateCanSwatTurn_Right;
	var() bool		bUpdateAllowPopup;
	var() bool		bUpdateLeanLeft;
	var() bool		bUpdateLeanRight;

	/** New values for coverslot vars.  Ignored unless the corresponding bUpdate* var above is TRUE */
	var() bool		bNewCanMantle;
	var() bool		bNewCanCoverSlip_Left;
	var() bool		bNewCanCoverSlip_Right;
	var() bool		bNewCanSwatTurn_Left;
	var() bool		bNewCanSwatTurn_Right;
	var() bool		bNewAllowPopup;
	var() bool		bNewLeanLeft;
	var() bool		bNewLeanRight;

	structdefaultproperties
	{
		NewCoverType=CT_None
	}
};


/** A modification to be applied to a subobject after a certain damage threshold is reached. */
struct native ObjectDamageModifier
{
	/** Name that can use used to descriptively identify this mod. */
	var() const Name							DamageModName;

	/** apply this Modifier at or below this health */
	var() const float							HealthThreshold;

	/** New static mesh to swap to, or None */
	var() StaticMesh							NewMesh;

	/** Material changes */
	var() const array<MaterialReplaceMod>		MaterialReplacements;
	var() const array<MaterialScalarParamMod>	MaterialScalarParams;
	var() const array<MaterialTexParamMod>		MaterialTexParams;
	var() const array<MaterialVectorParamMod>	MaterialVectorParams;

	/** Sounds to play when applying this modifier */
	var() const array<SoundCue>					Sounds;

	/** TRUE to kill the object at this stage */
	var() const bool							bSelfDestruct;

	/** TRUE to stop any attached ambient sound */
	var() const bool							bStopAmbientSound;

	// maybe use this?  ParticleSystem'Effects_Level_SP_5.SP_Hospital.Effects.P_Wooden_Barrier_Split'
	var() const array<DestroyedEffectParams>	DestroyedEffects;

	/** Set TRUE to forcibly disable attached cover (normal behavior is to reevaluate it */
	var() const bool							bForceDisableAttachedCover;

	/** actors that will be spawned */
	var() const array<ActorSpawnParams>			ActorsToSpawn;

	/** Potential splash damage (if the subj explodes or something) */
	var() const SplashDamageParams				SplashDamage;

	/** list of subobjects that are also affected by this modifier */
	var() const array<ObjDamageModifierDependency>	DependentSubObjs;

	/** modifications to make to attached cover */
	var() array<CoverModParams>					CoverMods;

	/** true when this stage has been applied */
	var transient bool							bApplied;
};

struct native DestructibleSubobject
{
	/** name of the subobject.  should be unique, but not currently enforced */
	var() Name								SubObjName;

	/** the mesh */
	var() StaticMeshComponent				Mesh;

	/** mods that can be applied to this subobject */
	var() array<ObjectDamageModifier>		DamageMods;

	/** hit points this subobject has by default */
	var() float								DefaultHealth;

	/**
	* This is a modifier that is autogenerated at initialization time which represents how to
	* get back to our original state
	* Used in MP to "heal" WDOs between rounds.
	*/
	var transient ObjectDamageModifier		UndoMod;

	structdefaultproperties
	{
		DefaultHealth=50
	}
};

/** Array of objects that, as a collection, comprise this destructible object */
var() array<DestructibleSubobject>		SubObjects;

/**
 * Remaining hit point values for subobjects.
 * Parallel array to Subobjects.  Stored separately so it can be replicated.
 */
var array<float>					SubObjectHealths;

/** Stores how much damage each subobject has taken this frame.  Used to handle the case where a single damage call can damage multiple objects. */
var private transient array<float>	TempDamageCache;


/** The particle effect to play when chainsawing this GDO **/
var() ParticleSystem	PS_ChainsawAttack;

/** Sound to play for chainsaw ripping through the object. */
var() SoundCue			ChainSawRipSound;

/** Refs to cover that is provided by this object.  These coverlinks will be reevaluated when damage is applied. */
var() array<CoverLink>	AttachedCover;

/** used to notify the client when the object should be repaired (because the game was reset) */
var repnotify int ResetCount;

struct native ReplicatedDamageModInfo
{
	var byte ObjIdx;
	var byte ModIdx;
	var bool bPartial;

	structdefaultproperties
	{
		ObjIdx=255
		ModIdx=255
	}
};
var repnotify ReplicatedDamageModInfo ReplicatedDamageMods[32];

/**
 * Client list of processed damage mods, would just stick a variable in ReplicatedDamageMod, but
 * that will just get wiped by the server next time the array is updated.
 */
var byte ProcessedMods[32];

cpptext
{
private:
	void SetupHealthVars();
	void GenerateUndo();
	void GenerateSubObjUndo(FDestructibleSubobject* SubObj);
	void ApplyDamageModInternal(FDestructibleSubobject* SubObj, FObjectDamageModifier* Mod, UBOOL bPartial, class AController* DamageInstigator);
public:
	void PostLoad();
	void PostEditChange(UProperty* PropertyThatChanged);

    /* Overridden to setup collision for damage meshes that will eventually exist*/
	virtual void InitRBPhys();

	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
	virtual UBOOL ReachedBy(APawn* P, const FVector& TestPosition, const FVector& Dest);

	/**
	* Used by the cooker to pre cache the convex data for static meshes within a given actor.
	* Overloaded to account for the damage meshes that replace the original static mesh
	* This data is stored with the level.
	* @param Level - The level the cache is in
	* @param TriByteCount - running total of memory usage for per-tri collision cache
	* @param TriMeshCount - running count of per-tri collision cache
	* @param HullByteCount - running total of memory usage for hull cache
	* @param HullCount - running count of hull cache
	*/
	virtual void BuildPhysStaticMeshCache(ULevel* Level,
										  INT& TriByteCount, INT& TriMeshCount, INT& HullByteCount, INT& HullCount);

	/**
	* Function that gets called from within Map_Check to allow this actor to check itself
	* for any potential errors and register them with map check dialog.
	*/
	virtual void CheckForErrors();
};

replication
{
	if (Role == ROLE_Authority)
		ReplicatedDamageMods, ResetCount;
}


/**
 * Restores a WDO to it's pristine unmolested state.  Used between matches in TDM.
 */
simulated native function UnDestroy();

/** attach all subojects, generally get things ready for play */
simulated private native function SetupComponents();

/** Change the static mesh for this WDO.  */
simulated native function SetSubObjectStaticMesh(const out DestructibleSubobject SubObj, StaticMesh SM);

/**
 * Apply one of the stored damage mods to this object.
 *
 * @param ObjIdx	index of subobject getting the mod
 * @param ModIdx	index into mod array indicating which mod to apply
 * @param bPartial	TRUE for partial application, used if more than one mod triggers
 *					in a frame.  Only 1 mod will be fully applied (to prevent overlapping
 *					sounds and whatnot.
 */
simulated native protected function ApplyDamageMod(int ObjIdx, int ModIdx, bool bPartial, optional Controller DamageInstigator);

/**
 * This function will remove all of the currently attached decals from the object.
 * Basically, we want to have decals attach to these objects and then on state change (due to damage usually), we will
 * just detach them all with the big particle effect that occurs it should not be noticeable.
 **/
simulated native protected function RemoveDecals();

/** one-time initializations.  basically intended to provide some native PostBeginPlay code. */
simulated native protected function OneTimeInit();

/**
 * This starts our ambient sound
 */
simulated event PostBeginPlay()
{
	OneTimeInit();

	TempDamageCache.Length = SubObjectHealths.Length;

	if( AmbientSoundComponent != none )
	{
		AmbientSoundComponent.SoundCue = AmbientSoundCue;
		AmbientSoundComponent.Play();
	}
}

/** Apply damage to a subobject */
protected native function DamageSubObject(int ObjIdx, int Damage, Controller EventInstigator, class<DamageType> DamType);

/** "Turn off" this entire object */
simulated protected function ShutDownObject()
{
	local int Idx;
	local DestructibleSubobject CurrentSubobject;

	// shut down all subobjects
	for (Idx=0; Idx<SubObjects.Length; ++Idx)
	{
		// compiler won't let us pass individual elements of a dynamic array as the value for an out parm, so use a local
		//@fixme ronp - TTPRO #40059
		CurrentSubobject = SubObjects[Idx];
		ShutDownSubObject(CurrentSubobject);
		SubObjects[Idx] = CurrentSubobject;
	}

	// disable all attached cover
	for (Idx=0; Idx<AttachedCover.Length; ++Idx)
	{
		if (AttachedCover[Idx] != None)
		{
			AttachedCover[Idx].SetDisabled(TRUE);
		}
	}
}

/** "Turn off" a subobject, no mesh, no collision, nothing left */
simulated protected event ShutDownSubObject(const out DestructibleSubobject SubObj)
{
	SubObj.Mesh.SetHidden(TRUE);
	SubObj.Mesh.SetTraceBlocking(FALSE, FALSE);
	SubObj.Mesh.SetActorCollision(FALSE, FALSE);
	SubObj.Mesh.SetBlockRigidBody(FALSE);
}

simulated protected event UnShutDownObject()
{
	local int Idx;

	// @fixme: better to store original vals here instead of assuming
	// true across the board, in that assumption is inaccurate
	for (Idx=0; Idx<SubObjects.Length; ++Idx)
	{
		SubObjects[Idx].Mesh.SetHidden(FALSE);
		SubObjects[Idx].Mesh.SetTraceBlocking(TRUE, TRUE);
		SubObjects[Idx].Mesh.SetActorCollision(TRUE, TRUE);
		SubObjects[Idx].Mesh.SetBlockRigidBody(TRUE);
	}
}

simulated function TakeRadiusDamage
(
	Controller			InstigatedBy,
	float				BaseDamage,
	float				DamageRadius,
	class<DamageType>	DamageType,
	float				Momentum,
	vector				HurtOrigin,
	bool				bFullDamage,
	Actor				DamageCauser
	)
{
	local float		DamageScale, Dist, DamageToApply;
	local int		Idx;

	// clear damage cache so we can use it
	for (Idx=0; Idx<TempDamageCache.Length; ++Idx)
	{
		TempDamageCache[Idx] = 0.f;
	}

	// calc damage for each subobject individually
	for (Idx=0; Idx<SubObjects.Length; ++Idx)
	{
		if ( bFullDamage )
		{
			DamageScale = 1.f;
		}
		else
		{
			Dist = FClamp(VSize(SubObjects[Idx].Mesh.GetPosition() - HurtOrigin), 0.f, DamageRadius);
			DamageScale = 1.f - Dist/DamageRadius;
		}
		if (DamageScale > 0.f)
		{
			DamageToApply = (DamageScale * BaseDamage) - TempDamageCache[Idx];
			if (DamageToApply > 0.f)
			{
				DamageSubObject(Idx, DamageToApply, InstigatedBy, DamageType);
			}
		}
	}

	// note TakeDamage doesn't get called -- will this be troublesome? (missing damage seqevents, etc)
}


event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int Idx, HitIdx, DamageToApply;

	// search and find which subobject got hit
	HitIdx = -1;
	if ( HitInfo.HitComponent != None )
	{
		for (Idx=0; Idx<SubObjects.Length; ++Idx)
		{
			if (SubObjects[Idx].Mesh == HitInfo.HitComponent)
			{
				HitIdx = Idx;
				//`log("Got Subobject!");
				break;
			}
		}
	}

	if (HitIdx >= 0)
	{
		if (SubObjectHealths[HitIdx] > 0.f)
		{
			// hit a subobject, damage it
			DamageSubObject(HitIdx, Damage, EventInstigator, DamageType);
		}
	}
	else
	{
		// clear damage cache so we can use it
		for (Idx=0; Idx<TempDamageCache.Length; ++Idx)
		{
			TempDamageCache[Idx] = 0.f;
		}

		// no specific hit subobject, so hit them all equally
		for (Idx=0; Idx<SubObjects.Length; ++Idx)
		{
			DamageToApply = Damage - TempDamageCache[Idx];
			if (DamageToApply > 0.f)
			{
				DamageSubObject(Idx, DamageToApply, EventInstigator, DamageType);
			}
		}
	}

	Super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
}

/** Fire the Destroyed kismet event, called when all subobjects are dead */
private event TriggerDestroyedEvent(Controller EventInstigator)
{
	TriggerEventClass(class'SeqEvent_Destroyed', EventInstigator);
}

simulated function Tick(float DeltaTime)
{
	if (!bComponentsSetUp)
	{
		//`log("Setting up components"@self);
		SetupComponents();
	}
	super.Tick(DeltaTime);
}

simulated event ReplicatedEvent(name VarName)
{
	local int Idx;

	if (VarName == 'ReplicatedDamageMods')
	{
		// look for unprocessed replicated damage mods
		for (Idx = 0; Idx < ArrayCount(ProcessedMods) && Idx < ArrayCount(ReplicatedDamageMods); Idx++)
		{
			if (ProcessedMods[Idx] == 0 && ReplicatedDamageMods[Idx].ObjIdx != 255)
			{
				ProcessedMods[Idx] = 1;
				ApplyDamageMod(ReplicatedDamageMods[Idx].ObjIdx, ReplicatedDamageMods[Idx].ModIdx, ReplicatedDamageMods[Idx].bPartial);
			}
		}
	}
	else if (VarName == 'ResetCount')
	{
		UnDestroy();
	}
}

/**
 * Fills in an empty slot in the replicated damage mod array to be
 * sent to the clients for simulation.
 */
event ReplicateDamageMod(int ObjIdx, int ModIdx, bool bPartial)
{
	local int Idx;
	local ReplicatedDamageModInfo NewMod;

	if ( (ObjIdx < 0) || (ObjIdx > 255)
		|| (ModIdx < 0) || (ModIdx > 255) )
	{
		`warn(self$" replicatedamagemod out of bounds "$ObjIdx@ModIdx);
		return;
	}

	// assign a full struct to workaround the bNetDirty issue with struct arrays
	NewMod.ObjIdx = ObjIdx;
	NewMod.ModIdx = ModIdx;
	NewMod.bPartial = bPartial;
	// find first available
	for (Idx = 0; Idx < ArrayCount(ReplicatedDamageMods); Idx++)
	{
		if (ReplicatedDamageMods[Idx].ObjIdx == 255)
		{
			ReplicatedDamageMods[Idx] = NewMod;
			break;
		}
	}
	// immediate net update
	bForceNetUpdate = TRUE;
}

event ApplySplashDamage(const out vector Origin, const out SplashDamageParams DamageParams)
{
	if (Role == ROLE_Authority)
	{
		HurtRadius(DamageParams.BaseDamage, DamageParams.DamageRadius, DamageParams.DamageType, DamageParams.Momentum, Origin, self);
	}
}

/** Causes any associated DamageModApplied kismet events to be fired */
event bool TriggerDamageModAppliedEvent(const out DestructibleSubobject SubObj, const out ObjectDamageModifier Mod)
{
	local int Idx, ActivateCnt;
	local SeqEvt_WDODamageModApplied Evt;

	for (Idx = 0; Idx < GeneratedEvents.Length; Idx++)
	{
		Evt = SeqEvt_WDODamageModApplied(GeneratedEvents[Idx]);

		if (Evt != None)
		{
			if ( (Evt.SubObjectName == SubObj.SubObjName) && (Evt.DamageModName == Mod.DamageModName) )
			{
				if (GeneratedEvents[Idx].CheckActivate(self, None))
				{
					ActivateCnt++;
				}
			}
		}
	}

	return (ActivateCnt > 0);
}

function Reset()
{
	RemoveDecals();
	UnDestroy();
	ResetCount++;
	bForceNetUpdate = TRUE;
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	local int Idx;

	if (bCanSpecialMeleeAttacked)
	{
		for (Idx = 0; Idx < SubObjectHealths.Length; Idx++)
		{
			if (SubObjectHealths[Idx] > 0)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

function bool ShouldSaveForCheckpoint()
{
	return false;
}

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	Begin Object Class=AudioComponent Name=AmbientSoundComponent0
	    bAutoPlay=TRUE
	    bStopWhenOwnerDestroyed=TRUE
	End Object
	AmbientSoundComponent=AmbientSoundComponent0
	Components.Add(AmbientSoundComponent0)

	// no physics by default
	Physics=PHYS_None
	bMovable=FALSE

	// many of these properties are pulled in from KActor
	// since this class was originally derived from KActor
	RemoteRole=ROLE_SimulatedProxy
	bNoDelete=TRUE
	bWorldGeometry=TRUE

	bNetInitialRotation=true
	bStatic=false
	bCollideWorld=false
	bProjTarget=true
	bBlockActors=true

	bAlwaysRelevant=true
	bSkipActorPropertyReplication=false
	bUpdateSimulatedPosition=true
	bReplicateMovement=true
	NetUpdateFrequency=1

	bCollideActors=true
	bNoEncroachCheck=true
	bBlocksTeleport=true
	bBlocksNavigation=true

	bEdShouldSnap=true
	bGameRelevant=true
	bPathColliding=true

	bConsiderAllStaticMeshComponentsForStreaming=true

	SupportedEvents.Add(class'SeqEvt_WDODamageModApplied')

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		bEnabled=false // will be enabled in SetSubObjectStaticMesh, precomputed lighting is used until the static mesh is changed
		bCastShadows=false // there will be a static shadow so no need to cast a dynamic shadow
		bSynthesizeSHLight=false
		bDynamic=false // using a static light environment to save update time
		bForceNonCompositeDynamicLights=TRUE // needed since we are using a static light environment
		TickGroup=TG_DuringAsyncWork
	End Object
	LightEnvironment=MyLightEnvironment


	LightingChannelsToUseForActorSpawnParams=(bInitialized=TRUE,Static=TRUE,Unnamed_1=TRUE)
	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment2
		bEnabled=FALSE // will be enabled when the first actor is spawned in ApplyDamageModInternal
		bCastShadows=FALSE
		bDynamic=FALSE
		bForceNonCompositeDynamicLights=TRUE // needed since we are using a static light environment, otherwise dynamic lights won't have any affect
		TickGroup=TG_DuringAsyncWork
	End Object
	LightEnvironmentToUseForActorSpawnParams=MyLightEnvironment2
	Components.Add(MyLightEnvironment2)

	bCanSpecialMeleeAttacked=TRUE

	PS_ChainsawAttack=ParticleSystem'Effects_Gameplay.Chainsaw.P_Chainsaw_GDO' // waiting on effects to give us a good effect
}


