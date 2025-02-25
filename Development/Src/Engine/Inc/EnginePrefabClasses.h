/*===========================================================================
    C++ class definitions exported from UnrealScript.
    This is automatically generated by the tools.
    DO NOT modify this manually! Edit the corresponding .uc files instead!
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
===========================================================================*/
#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif


// Split enums from the rest of the header so they can be included earlier
// than the rest of the header file by including this file twice with different
// #define wrappers. See Engine.h and look at EngineClasses.h for an example.
#if !NO_ENUMS && !defined(NAMES_ONLY)

#ifndef INCLUDED_ENGINE_PREFAB_ENUMS
#define INCLUDED_ENGINE_PREFAB_ENUMS 1


#endif // !INCLUDED_ENGINE_PREFAB_ENUMS
#endif // !NO_ENUMS

#if !ENUMS_ONLY

#ifndef NAMES_ONLY
#define AUTOGENERATE_NAME(name) extern FName ENGINE_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#endif


#ifndef NAMES_ONLY

#ifndef INCLUDED_ENGINE_PREFAB_CLASSES
#define INCLUDED_ENGINE_PREFAB_CLASSES 1

class APrefabInstance : public AActor
{
public:
    //## BEGIN PROPS PrefabInstance
    class UPrefab* TemplatePrefab;
    INT TemplateVersion;
    TMap< UObject*,UObject* > ArchetypeToInstanceMap;
    class UPrefabSequence* SequenceInstance;
    INT PI_PackageVersion;
    INT PI_LicenseePackageVersion;
    TArrayNoInit<BYTE> PI_Bytes;
    TArrayNoInit<class UObject*> PI_CompleteObjects;
    TArrayNoInit<class UObject*> PI_ReferencedObjects;
    TArrayNoInit<FString> PI_SavedNames;
    TMap< UObject*,INT > PI_ObjectMap;
    //## END PROPS PrefabInstance

    DECLARE_CLASS(APrefabInstance,AActor,0,Engine)

	// UObject interface
	virtual void			Serialize(FArchive& Ar);
	virtual void			PreSave();
	virtual void			PostLoad();

#if REQUIRES_SAMECLASS_ARCHETYPE
	/**
	 * Provides PrefabInstance & UIPrefabInstanc objects with a way to override incorrect behavior in ConditionalPostLoad()
	 * until different-class archetypes are supported.
	 *
	 * @fixme - temporary hack; correct fix would be to support archetypes of a different class
	 *
	 * @return	pointer to an object instancing graph to use for logic in ConditionalPostLoad().
	 */
	virtual struct FObjectInstancingGraph* GetCustomPostLoadInstanceGraph();
#endif

	/**
	 * Callback used to allow object register its direct object references that are not already covered by
	 * the token stream.
	 *
	 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
	 */
	void AddReferencedObjects( TArray<UObject*>& ObjectArray );

	/**
	 * Create an instance of the supplied Prefab, including creating all objects and Kismet sequence.
	 */
	void InstancePrefab(UPrefab* InPrefab);

	/**
	 * Do any teardown to destroy anything instanced for this PrefabInstance.
	 * Sets TemplatePrefab back to NULL.
	 */
	void DestroyPrefab(class USelection* Selection);

	/**
	 * Update this instance of a prefab to match the template prefab.
	 * This will destroy/create objects as necessary.
	 * It also recreates the Kismet sequence.
	 */
	void UpdatePrefabInstance(class USelection* Selection);

	/**
	 * Convert this prefab instance to look like the Prefab archetype version of it (by changing object refs to archetype refs and
	 * converting positions to local space). Then serialise it, so we only get things that are unique to this instance. We store this
	 * archive in the PrefabInstance.
	 */
	void SavePrefabDifferences();

	/**
	 * Iterates through the ArchetypeToInstanceMap and verifies that the archetypes for each of this PrefabInstance's actors exist.
	 * For any actors contained by this PrefabInstance that do not have a corresponding archetype, removes the actor from the
	 * ArchetypeToInstanceMap.  This is normally caused by adding a new actor to a PrefabInstance, updating the source Prefab, then loading
	 * a new map without first saving the package containing the updated Prefab.  When the original map is reloaded, though it contains
	 * an entry for the newly added actor, the source Prefab's linker does not contain an entry for the corresponding archetype.
	 *
	 * @return	TRUE if each pair in the ArchetypeToInstanceMap had a valid archetype.  FALSE if one or more archetypes were NULL.
	 */
	UBOOL VerifyMemberArchetypes();

	/**
	 * Utility for getting all Actors that are part of this PrefabInstance.
	 */
	void GetActorsInPrefabInstance( TArray<AActor*>& OutActors ) const;

	/**
	 * Examines the selection status of each actor in this prefab instance, and
	 * returns TRUE if the selection state of all actors matches the input state.
	 */
	UBOOL GetActorSelectionStatus(UBOOL bInSelected) const;

	/** Instance the Kismet sequence if we have one into the 'Prefabs' subsequence. */
	void InstanceKismetSequence(USequence* SrcSequence, const FString& InSeqName);
	/** Destroy the Kismet sequence associated with this Prefab instance. */
	void DestroyKismetSequence();

	/** Copy information to a FPrefabUpdateArchive from this PrefabInstance for updating a PrefabInstance with. */
	void CopyToArchive(FPrefabUpdateArc* InArc);
	/** Copy information from a FPrefabUpdateArchive into this PrefabInstance for saving etc. */
	void CopyFromArchive(FPrefabUpdateArc* InArc);

	/** Applies a transform to the object if its an actor. */
	static void ApplyTransformIfActor(UObject* Obj, const FMatrix& Transform);

	/** Utility for taking a map and inverting it. */
	static void CreateInverseMap(TMap<UObject*, UObject*>& OutMap, TMap<UObject*, UObject*>& InMap);

	/**
	 * Utility	for copying UModel from one ABrush to another.
	 * Sees if DestActor is an ABrush. If so, assumes SrcActor is one. Then uses StaticDuplicateObject to copy UModel from
	 * SrcActor to DestActor.
	 */
	static void CopyModelIfBrush(UObject* DestObj, UObject* SrcObj);
};

class UPrefab : public UObject
{
public:
    //## BEGIN PROPS Prefab
    INT PrefabVersion;
    TArrayNoInit<class UObject*> PrefabArchetypes;
    TArrayNoInit<class UObject*> RemovedArchetypes;
    class UPrefabSequence* PrefabSequence;
    class UTexture2D* PrefabPreview;
    //## END PROPS Prefab

    DECLARE_CLASS(UPrefab,UObject,0,Engine)
	// UObject interface

	/**
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc();

	/**
	 * Called after the data for this prefab has been loaded from disk.  Removes any NULL elements from the PrefabArchetypes array.
	 */
	virtual void PostLoad();

	// Prefab interface
	/**
	 * Fixes up object references within a group of archetypes.  For any references which refer
	 * to an actor from the original set, replaces that reference with a reference to the archetype
	 * class itself.
	 *
	 * @param	ArchetypeBaseMap	map of original actor instances to archetypes
	 * @param	bNullPrivateRefs	should we null references to any private objects
	 */
	static void ResolveInterDependencies( TMap<UObject*,UObject*>& ArchetypeBaseMap, UBOOL bNullPrivateRefs );

	/** Utility for copying a USequence from the level into a Prefab in a package, including fixing up references. */
	void CopySequenceIntoPrefab(USequence* InPrefabSeq, TMap<UObject*,UObject*>& InstanceToArchetypeMap);
};

#endif // !INCLUDED_ENGINE_PREFAB_CLASSES
#endif // !NAMES_ONLY


#ifndef NAMES_ONLY
#undef AUTOGENERATE_NAME
#undef AUTOGENERATE_FUNCTION
#endif

#ifdef STATIC_LINKING_MOJO
#ifndef ENGINE_PREFAB_NATIVE_DEFS
#define ENGINE_PREFAB_NATIVE_DEFS

DECLARE_NATIVE_TYPE(Engine,UPrefab);
DECLARE_NATIVE_TYPE(Engine,APrefabInstance);

#define AUTO_INITIALIZE_REGISTRANTS_ENGINE_PREFAB \
	UPrefab::StaticClass(); \
	APrefabInstance::StaticClass(); \

#endif // ENGINE_PREFAB_NATIVE_DEFS

#ifdef NATIVES_ONLY
#endif // NATIVES_ONLY
#endif // STATIC_LINKING_MOJO

#ifdef VERIFY_CLASS_SIZES
VERIFY_CLASS_OFFSET_NODIE(U,Prefab,PrefabVersion)
VERIFY_CLASS_OFFSET_NODIE(U,Prefab,PrefabPreview)
VERIFY_CLASS_SIZE_NODIE(UPrefab)
VERIFY_CLASS_OFFSET_NODIE(A,PrefabInstance,TemplatePrefab)
VERIFY_CLASS_OFFSET_NODIE(A,PrefabInstance,PI_ObjectMap)
VERIFY_CLASS_SIZE_NODIE(APrefabInstance)
#endif // VERIFY_CLASS_SIZES
#endif // !ENUMS_ONLY

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif
