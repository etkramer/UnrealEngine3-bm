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

#ifndef INCLUDED_ENGINE_FOLIAGE_ENUMS
#define INCLUDED_ENGINE_FOLIAGE_ENUMS 1


#endif // !INCLUDED_ENGINE_FOLIAGE_ENUMS
#endif // !NO_ENUMS

#if !ENUMS_ONLY

#ifndef NAMES_ONLY
#define AUTOGENERATE_NAME(name) extern FName ENGINE_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#endif


#ifndef NAMES_ONLY

#ifndef INCLUDED_ENGINE_FOLIAGE_CLASSES
#define INCLUDED_ENGINE_FOLIAGE_CLASSES 1

struct FFoliageMesh
{
    class UStaticMesh* InstanceStaticMesh;
    class UMaterialInterface* Material;
    FLOAT MaxDrawRadius;
    FLOAT MinTransitionRadius;
    FLOAT MinThinningRadius;
    FVector MinScale;
    FVector MaxScale;
    FLOAT MinUniformScale;
    FLOAT MaxUniformScale;
    FLOAT SwayScale;
    INT Seed;
    FLOAT SurfaceAreaPerInstance;
    BITFIELD bCreateInstancesOnBSP:1;
    BITFIELD bCreateInstancesOnStaticMeshes:1;
    BITFIELD bCreateInstancesOnTerrain:1;
    class UFoliageComponent* Component;

    /** Constructors */
    FFoliageMesh() {}
    FFoliageMesh(EEventParm)
    {
        appMemzero(this, sizeof(FFoliageMesh));
    }
};

class AFoliageFactory : public AVolume
{
public:
    //## BEGIN PROPS FoliageFactory
    TArrayNoInit<struct FFoliageMesh> Meshes;
    FLOAT VolumeFalloffRadius;
    FLOAT VolumeFalloffExponent;
    FLOAT SurfaceDensityUpFacing;
    FLOAT SurfaceDensityDownFacing;
    FLOAT SurfaceDensitySideFacing;
    FLOAT FacingFalloffExponent;
    INT MaxInstanceCount;
    //## END PROPS FoliageFactory

    DECLARE_CLASS(AFoliageFactory,AVolume,0,Engine)
	/**
	 * Checks whether an instance should be spawned at a particular point on a surface for this foliage factory.
	 * @param Point - The world-space point on the surface.
	 * @param Normal - The surface normal at the point.
	 * @return TRUE if the instance should be created.
	 */
	virtual UBOOL ShouldCreateInstance(const FVector& Point,const FVector& Normal) const;
	
	/**
	 * Determines whether the foliage factory may spawn foliage on a specific primitive.
	 * @param Primitive - The primitive being considered.
	 * @return TRUE if the foliage factory may spawn foliage on the primitive.
	 */
	virtual UBOOL IsPrimitiveRelevant(const UPrimitiveComponent* Primitive) const;

	/** Regenerates the foliage instances for the actor. */
	void RegenerateFoliageInstances();

	// AActor interface.
	virtual void PostEditMove(UBOOL bFinished);

	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PostLoad();
};

struct FFoliageInstanceBase
{
    FVector Location;
    FVector XAxis;
    FVector YAxis;
    FVector ZAxis;
    FLOAT DistanceFactorSquared;

    /** Constructors */
    FFoliageInstanceBase() {}
    FFoliageInstanceBase(EEventParm)
    {
        appMemzero(this, sizeof(FFoliageInstanceBase));
    }
};

struct FGatheredFoliageInstance : public FFoliageInstanceBase
{
    FColor StaticLighting[4];

    /** Constructors */
    FGatheredFoliageInstance() {}
    FGatheredFoliageInstance(EEventParm)
    {
        appMemzero(this, sizeof(FGatheredFoliageInstance));
    }
};

class UFoliageComponent : public UPrimitiveComponent
{
public:
    //## BEGIN PROPS FoliageComponent
    TArrayNoInit<struct FGatheredFoliageInstance> Instances;
    TArrayNoInit<FGuid> StaticallyRelevantLights;
    TArrayNoInit<FGuid> StaticallyIrrelevantLights;
    FLOAT DirectionalStaticLightingScale[3];
    FLOAT SimpleStaticLightingScale[3];
    class UStaticMesh* InstanceStaticMesh;
    class UMaterialInterface* Material;
    FLOAT MaxDrawRadius;
    FLOAT MinTransitionRadius;
    FLOAT MinThinningRadius;
    FVector MinScale;
    FVector MaxScale;
    FLOAT SwayScale;
    //## END PROPS FoliageComponent

    DECLARE_CLASS(UFoliageComponent,UPrimitiveComponent,0,Engine)
	// UPrimitiveComponent interface.
	virtual void UpdateBounds();
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options);
	virtual void InvalidateLightingCache();
};

#endif // !INCLUDED_ENGINE_FOLIAGE_CLASSES
#endif // !NAMES_ONLY


#ifndef NAMES_ONLY
#undef AUTOGENERATE_NAME
#undef AUTOGENERATE_FUNCTION
#endif

#ifdef STATIC_LINKING_MOJO
#ifndef ENGINE_FOLIAGE_NATIVE_DEFS
#define ENGINE_FOLIAGE_NATIVE_DEFS

DECLARE_NATIVE_TYPE(Engine,UFoliageComponent);
DECLARE_NATIVE_TYPE(Engine,AFoliageFactory);

#define AUTO_INITIALIZE_REGISTRANTS_ENGINE_FOLIAGE \
	UFoliageComponent::StaticClass(); \
	AFoliageFactory::StaticClass(); \

#endif // ENGINE_FOLIAGE_NATIVE_DEFS

#ifdef NATIVES_ONLY
#endif // NATIVES_ONLY
#endif // STATIC_LINKING_MOJO

#ifdef VERIFY_CLASS_SIZES
VERIFY_CLASS_OFFSET_NODIE(U,FoliageComponent,Instances)
VERIFY_CLASS_OFFSET_NODIE(U,FoliageComponent,SwayScale)
VERIFY_CLASS_SIZE_NODIE(UFoliageComponent)
VERIFY_CLASS_OFFSET_NODIE(A,FoliageFactory,Meshes)
VERIFY_CLASS_OFFSET_NODIE(A,FoliageFactory,MaxInstanceCount)
VERIFY_CLASS_SIZE_NODIE(AFoliageFactory)
#endif // VERIFY_CLASS_SIZES
#endif // !ENUMS_ONLY

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif
