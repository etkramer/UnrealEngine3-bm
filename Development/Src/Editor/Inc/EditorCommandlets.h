/*=============================================================================
	Editor.h: Unreal editor public header file.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_EDITOR_COMMANDLETS
#define _INC_EDITOR_COMMANDLETS

BEGIN_COMMANDLET(AnalyzeScript,Editor)
	static UFunction* FindSuperFunction(UFunction* evilFunc);
END_COMMANDLET

BEGIN_COMMANDLET(AnalyzeContent,Editor)
	void StaticInitialize();
END_COMMANDLET


/**
 * Contains stats about a single resource in a package file.
 */
struct FObjectResourceStat
{
	/** the complete path name for this resource */
	INT ResourceNameIndex;

	/** the name of the class for this resource */
	FName ClassName;

	/** the size of this resource, on disk */
	INT ResourceSize;

	/** Standard Constructor */
	FObjectResourceStat( FName InClassName, const FString& InResourceName, INT InResourceSize );

	/** Copy constructor */
	FObjectResourceStat( const FObjectResourceStat& Other )
	{
		ResourceNameIndex = Other.ResourceNameIndex;
		ClassName = Other.ClassName;
		ResourceSize = Other.ResourceSize;
	}
};

/**
 * A mapping of class name to the resource stats for objects of that class
 */
class FClassResourceMap : public TMultiMap<FName,FObjectResourceStat>
{
};

struct FPackageResourceStat
{
	/** the name of the package this struct contains resource stats for */
	FName				PackageName;

	/** the filename of the package; will be different from PackageName if this package is one of the loc packages */
	FName				PackageFilename;

	/** the map of 'Class name' to 'object resources of that class' for this package */
	FClassResourceMap	PackageResources;

	/**
	 * Constructor
	 */
	FPackageResourceStat( FName InPackageName )
	: PackageName(InPackageName)
	{ }

	/**
	 * Creates a new resource stat using the specified parameters.
	 *
	 * @param	ResourceClassName	the name of the class for the resource
	 * @param	ResourcePathName	the complete path name for the resource
	 * @param	ResourceSize		the size on disk for the resource
	 *
	 * @return	a pointer to the FObjectResourceStat that was added
	 */
	struct FObjectResourceStat* AddResourceStat( FName ResourceClassName, const FString& ResourcePathName, INT ResourceSize );
};



enum EReportOutputType
{
	/** write the results to the log only */
	OUTPUTTYPE_Log,

	/** write the results to a CSV file */
	OUTPUTTYPE_CSV,

	/** write the results to an XML file (not implemented) */
	OUTPUTTYPE_XML,
};

/**
 * Generates various types of reports for the list of resources collected by the AnalyzeCookedContent commandlet.  Each derived version of this struct
 * generates a different type of report.
 */
struct FResourceStatReporter
{
	EReportOutputType OutputType;

	/**
	 * Creates a report using the specified stats.  The type of report created depends on the reporter type.
	 *
	 * @param	ResourceStats	the list of resource stats to create a report for.
	 *
	 * @return	TRUE if the report was created successfully; FALSE otherwise.
	 */
	virtual UBOOL CreateReport( const TArray<struct FPackageResourceStat>& ResourceStats )=0;

	/** Constructor */
	FResourceStatReporter()
	: OutputType(OUTPUTTYPE_Log)
	{}

	/** Destructor */
	virtual ~FResourceStatReporter()
	{}
};

/**
 * This reporter generates a report on the disk-space taken by each asset type.
 */
struct FResourceStatReporter_TotalMemoryPerAsset : public FResourceStatReporter
{
	/**
	 * Creates a report using the specified stats.  The type of report created depends on the reporter type.
	 *
	 * @param	ResourceStats	the list of resource stats to create a report for.
	 *
	 * @return	TRUE if the report was created successfully; FALSE otherwise.
	 */
	virtual UBOOL CreateReport( const TArray<struct FPackageResourceStat>& ResourceStats );
};

/**
 * This reporter generates a report which displays objects which are duplicated into more than one package.
 */
struct FResourceStatReporter_AssetDuplication : public FResourceStatReporter
{
	/**
	 * Creates a report using the specified stats.  The type of report created depends on the reporter type.
	 *
	 * @param	ResourceStats	the list of resource stats to create a report for.
	 *
	 * @return	TRUE if the report was created successfully; FALSE otherwise.
	 */
	virtual UBOOL CreateReport( const TArray<struct FPackageResourceStat>& ResourceStats );
};

struct FResourceDiskSize
{
	FString ClassName;
	QWORD TotalSize;

	/** Default constructor */
	FResourceDiskSize( FName InClassName )
	: ClassName(InClassName.ToString()), TotalSize(0)
	{}

	/** Copy constructor */
	FResourceDiskSize( const FResourceDiskSize& Other )
	{
		ClassName = Other.ClassName;
		TotalSize = Other.TotalSize;
	}
};

BEGIN_COMMANDLET(AnalyzeCookedContent,Editor)

	/**
	 * the list of packages to process
	 */
	TArray<FFilename> CookedPackageNames;

	/**
	 * the class, path name, and size on disk for all resources
	 */
	TArray<struct FPackageResourceStat> PackageResourceStats;

	/**
	 * Builds the list of package names to load
	 */
	void Init();

	/**
	 * Loads each package and adds stats for its exports to the main list of stats
	 */
	void AssembleResourceStats();

	/**
	 * Determines which report type is desired based on the command-line parameters specified and creates the appropriate reporter.
	 *
	 * @param	Params	the command-line parameters passed to Main
	 *
	 * @return	a pointer to a reporter which generates output in the desired format, or NULL if no valid report type was specified.
	 */
	struct FResourceStatReporter* CreateReporter( const FString& Params );

END_COMMANDLET

BEGIN_COMMANDLET(AnalyzeCookedPackages,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(MineCookedPackages,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(AnalyzeReferencedContent,Editor)
	//@todo: this code is in dire need of refactoring


	// this holds all of the common data for our structs
	typedef TMap<FString,UINT> PerLevelDataMap;
	struct FAssetStatsBase
	{
		/** Mapping from LevelName to the number of instances of this type in that level */
		PerLevelDataMap LevelNameToInstanceCount;

		/** Maps this static mesh was used in.								*/
		TArray<FString> MapsUsedIn;

		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}
	};


	/**
	 * Encapsulates gathered stats for a particular UStaticMesh object.
	 */
	struct FStaticMeshStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members. */
		FStaticMeshStats( UStaticMesh* StaticMesh );

		/**
		 * Stringifies gathered stats in CSV format.
		 *
		 * @return comma separated list of stats
		 */
		FString ToCSV() const;
		/** This takes a LevelName and then looks for the number of Instances of this StatMesh used within that level **/
		FString ToCSV( const FString& LevelName ) const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}

		/**
		 * Returns a header row for CSV
		 *
		 * @return comma separated header row
		 */
		static FString GetCSVHeaderRow();

		/** Resource type.																*/
		FString ResourceType;
		/** Resource name.																*/
		FString ResourceName;
		/** Number of static mesh instances overall.									*/
		INT	NumInstances;
		/** Triangle count of mesh.														*/
		INT	NumTriangles;
		/** Section count of mesh.														*/
		INT NumSections;
		/** Number of convex hulls in the collision geometry of mesh.					*/
		INT NumConvexPrimitives;
		/** Does this static mesh use simple collision */
		INT bUsesSimpleRigidBodyCollision;
        /** Number of sections that have collision enabled                              */
        INT NumElementsWithCollision;
		/** Whether resource is referenced by script.									*/
		UBOOL bIsReferencedByScript;
        /** Whether resource is referenced by particle system                           */
        UBOOL bIsReferencedByParticles;
		/** Resource size of static mesh.												*/
		INT	ResourceSize;
		/** Is this mesh scaled non-uniformly in a level								*/
		UBOOL bIsMeshNonUniformlyScaled;
		/** Does this mesh have box collision that should be converted					*/
		UBOOL bShouldConvertBoxColl;
		/** Array of different scales that this mesh is used at							*/
		TArray<FVector> UsedAtScales;
	};

	/**
	* Encapsulates gathered stats for a particular UShadowMap1D object.
	*/
	struct FShadowMap1DStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members */
		FShadowMap1DStats(UShadowMap1D* ShadowMap1D);
		/**
		* Stringifies gathered stats in CSV format.
		*
		* @return comma separated list of stats
		*/
		FString ToCSV() const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}
		/**
		* Returns a header row for CSV
		*
		* @return comma separated header row
		*/
		static FString GetCSVHeaderRow();

		/** Resource type.																*/
		FString ResourceType;
		/** Resource name.																*/
		FString ResourceName;
		/** Size in Bytes of the resource												*/
		INT ResourceSize;
		/** Number of Vertex samples used for the 1D shadowmap							*/
		INT NumSamples;		
		/** Light that caused the creation of the 1D shadowmap							*/
		FString UsedByLight;
	};

	/**
	* Encapsulates gathered stats for a particular UShadowMap2D object.
	*/
	struct FShadowMap2DStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members */
		FShadowMap2DStats(UShadowMap2D* ShadowMap2D);
		/**
		* Stringifies gathered stats in CSV format.
		*
		* @return comma separated list of stats
		*/
		FString ToCSV() const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}
		/**
		* Returns a header row for CSV
		*
		* @return comma separated header row
		*/
		static FString GetCSVHeaderRow();

		/** Resource type.																*/
		FString ResourceType;
		/** Resource name.																*/
		FString ResourceName;
		/** The texture which contains the shadow-map data.								*/
		FString ShadowMapTexture2D;
		/** size of shadow map texture X												*/
		INT ShadowMapTexture2DSizeX;
		/** size of shadow map texture Y												*/
		INT ShadowMapTexture2DSizeY;
		/** format of shadow map texture 												*/
		FString ShadowMapTexture2DFormat;
		/** Light that caused the creation of the 1D shadowmap							*/
		FString UsedByLight;
	};	

	/**
	 * Encapsulates gathered stats for a particular UTexture object.
	 */
	struct FTextureStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members */
		FTextureStats( UTexture* Texture );

		/**
		 * Stringifies gathered stats in CSV format.
		 *
		 * @return comma separated list of stats
		 */
		FString ToCSV() const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}

		/**
		 * Returns a header row for CSV
		 *
		 * @return comma separated header row
		 */
		static FString GetCSVHeaderRow();

		/** Resource type.																*/
		FString ResourceType;
		/** Resource name.																*/
		FString ResourceName;
		/** Map of materials this textures is being used by.							*/
		TMap<FString,INT> MaterialsUsedBy;
		/** Whether resource is referenced by script.									*/
		UBOOL bIsReferencedByScript;
		/** Resource size of texture.													*/
		INT	ResourceSize;
		/** LOD bias.																	*/
		INT	LODBias;
		/** LOD group.																	*/
		INT LODGroup;
		/** Texture pixel format.														*/
		FString Format;
	};

	/**
	 * Encapsulates gathered stats for a particular UMaterial object.
	 */
	struct FMaterialStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members */
		FMaterialStats( UMaterial* Material, UAnalyzeReferencedContentCommandlet* Commandlet );

		/**
		 * Stringifies gathered stats in CSV format.
		 *
		 * @return comma separated list of stats
		 */
		FString ToCSV() const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}

		/**
		 * Returns a header row for CSV
		 *
		 * @return comma separated header row
		 */
		static FString GetCSVHeaderRow();

		/** Resource type.																*/
		FString ResourceType;
		/** Resource name.																*/
		FString ResourceName;
		/** Number of BSP surfaces this material is applied to.							*/
		INT	NumBrushesAppliedTo;
		/** Number of static mesh instances this material is applied to.				*/
		INT NumStaticMeshInstancesAppliedTo;
		/** Map of static meshes this material is used by.								*/
		TMap<FString,INT> StaticMeshesAppliedTo;
		/** Whether resource is referenced by script.									*/
		UBOOL bIsReferencedByScript;
		/** Array of textures used. Also implies count.									*/
		TArray<FString>	TexturesUsed;
		/** Number of texture samples made to render this material.						*/
		INT NumTextureSamples;
		/** Max depth of dependent texture read chain.									*/
		INT MaxTextureDependencyLength;
		/** Translucent instruction count.												*/
		INT NumInstructionsTranslucent;
		/** Additive instruction count.													*/
		INT NumInstructionsAdditive;
		/** Modulate instruction count.													*/
		INT NumInstructionsModulate;
		/** Base pass no lightmap instruction count.										*/
		INT NumInstructionsBasePassNoLightmap;
		/** Base pass with vertex lightmap instruction count.							*/
		INT NumInstructionsBasePassAndLightmap;
		/** Point light with shadow map instruction count.								*/
		INT NumInstructionsPointLightWithShadowMap;
		/** Resource size of all referenced/ used textures.								*/
		INT ResourceSizeOfReferencedTextures;
	};

	/**
	 * Encapsulates gathered stats for a particular UParticleSystem object
	 */
	struct FParticleStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members */
		FParticleStats( UParticleSystem* ParticleSystem );

		/**
		 * Stringifies gathered stats in CSV format.
		 *
		 * @return comma separated list of stats
		 */
		FString ToCSV() const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const;

		/**
		 * Returns a header row for CSV
		 *
		 * @return comma separated header row
		 */
		static FString GetCSVHeaderRow();

		/** Resource type.																*/
		FString ResourceType;
		/** Resource name.																*/
		FString ResourceName;
		/** Whether resource is referenced by script.									*/
		UBOOL bIsReferencedByScript;	
		/** Number of emitters in this system.											*/
		INT NumEmitters;
		/** Combined number of modules in all emitters used.							*/
		INT NumModules;
		/** Combined number of peak particles in system.								*/
		INT NumPeakActiveParticles;
        /** Combined number of collision modules across emitters                        */
        INT NumEmittersUsingCollision;
        /** Combined number of emitters that have active physics                        */
        INT NumEmittersUsingPhysics;
        /** Maximum number of particles drawn per frame                                 */
        INT MaxNumDrawnPerFrame;
        /** Ratio of particles simulated to particles drawn                             */
        FLOAT PeakActiveToMaxDrawnRatio;
		/** This is the size in bytes that this Particle System will use                */
		INT NumBytesUsed;
		/** if any modules/emitters use distortion material */
		UBOOL bUsesDistortionMaterial;
		/** if any modules/emitters use scene color material */
		UBOOL bUsesSceneTextureMaterial;
		/** materials that use distortion */
		TArray<FString> DistortMaterialNames;
		/** materials that use scene color */
		TArray<FString> SceneColorMaterialNames;
		/** If any modules have mesh emitters that have DoCollision == TRUE wich is more than likely bad and perf costing */
		UBOOL bMeshEmitterHasDoCollisions;
		/** If any modules have mesh emitters that have DoCollision == TRUE wich is more than likely bad and perf costing */
		UBOOL bMeshEmitterHasCastShadows;
		/** If the particle system has warm up time greater than N seconds**/
		FLOAT WarmUpTime;
		/** If any of the emitters are PhysX emitters == TRUE */
		UBOOL bHasPhysXEmitters;
	};

	/**
	 * Encapsulates gathered textures-->particle systems information for all particle systems
	 */
	struct FTextureToParticleSystemStats : public FAssetStatsBase
	{
		FTextureToParticleSystemStats(UTexture* InTexture);
		void AddParticleSystem(UParticleSystem* InParticleSystem);

		/**
		 * Stringifies gathered stats in CSV format.
		 *
		 * @return comma separated list of stats
		 */
		FString ToCSV() const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}

		/**
		 * Returns a header row for CSV
		 *
		 * @return comma separated header row
		 */
		static FString GetCSVHeaderRow();

		const INT GetParticleSystemsContainedInCount() const
		{
			return ParticleSystemsContainedIn.Num();
		}

		FString GetParticleSystemContainedIn(INT Index) const
		{
			if ((Index >= 0) && (Index < ParticleSystemsContainedIn.Num()))
			{
				return ParticleSystemsContainedIn(Index);
			}

			return TEXT("*** INVALID ***");
		}

	protected:
		/** Texture name.										*/
		FString TextureName;
		/** Texture size.										*/
		FString TextureSize;
		/** Texture pixel format.								*/
		FString Format;
		TArray<FString> ParticleSystemsContainedIn;
	};

	struct FAnimSequenceStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members */
		FAnimSequenceStats( UAnimSequence* Sequence );

		/**
		 * Stringifies gathered stats in CSV format.
		 *
		 * @return comma separated list of stats
		 */
		FString ToCSV() const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}

		/**
		 * Returns a header row for CSV
		 *
		 * @return comma separated header row
		 */
		static FString GetCSVHeaderRow();

		/** Resource type.																*/
		FString ResourceType;
		/** Resource name.																*/
		FString ResourceName;
		/** Whether resource is referenced by script.									*/
		UBOOL bIsReferencedByScript;	
		/** Type of compression used on this animation.									*/
		enum AnimationCompressionFormat TranslationFormat;
		/** Type of compression used on this animation.									*/
		enum AnimationCompressionFormat RotationFormat;
		/** Name of compression algo class used. */
		FString CompressionScheme;
		/** Size in bytes of this animation. */
		INT	AnimationSize;
		
		/** Total Tracks in this animation. */
		INT	TotalTracks;
		/** Total Tracks with no animated translation. */
		INT NumTransTracksWithOneKey;
		/** Total Tracks with no animated rotation. */
		INT NumRotTracksWithOneKey;
		/** Size in bytes of this animation's track table. */
		INT	TrackTableSize;
		/** total translation keys. */
		INT TotalNumTransKeys;
		/** total rotation keys. */
		INT TotalNumRotKeys;
		/** Size of a single translation key. */
		INT	TranslationKeySize;
		/** Size of a single rotation key. */
		INT	RotationKeySize;
		/** Total Frames in this animation. */
		INT	TotalFrames;
	};

	struct FLightingOptimizationStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members */
		FLightingOptimizationStats( AStaticMeshActor* StaticMeshActor );

		/**
		* Stringifies gathered stats in CSV format.
		*
		* @return comma separated list of stats
		*/
		FString ToCSV() const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}

		/**
		* Returns a header row for CSV
		*
		* @return comma separated header row
		*/
		static FString GetCSVHeaderRow();

		/**
		*   Calculate the memory required to light a mesh with given NumVertices using vertex lighting
		*/
		static INT CalculateVertexLightingBytesUsed(INT NumVertices);

		/** Assuming DXT1 lightmaps...
		*   4 bits/pixel * width * height = Highest MIP Level * 1.333 MIP Factor for approx usage for a full mip chain
		*   Either 1 or 3 textures if we're doing simple or directional (3-axis) lightmap
		*   Most lightmaps require a second UV channel which is probably an extra 4 bytes (2 floats compressed to SHORT) 
		*/
		static INT CalculateLightmapLightingBytesUsed(INT Width, INT Height, INT NumVertices, INT UVChannelIndex);

		/** 
		*	For a given list of parameters, compute a full spread of potential savings values using vertex light, or 256, 128, 64, 32 pixel square light maps
		*  @param LMType - Current type of lighting being used
		*  @param NumVertices - Number of vertices in the given mesh
		*  @param Width - Width of current lightmap
		*  @param Height - Height of current lightmap
		*  @param TexCoordIndex - channel index of the uvs currently used for lightmaps
		*  @param LOI - A struct to be filled in by the function with the potential savings
		*/
		static void CalculateLightingOptimizationInfo(ELightMapInteractionType LMType, INT NumVertices, INT Width, INT Height, INT TexCoordIndex, FLightingOptimizationStats& LOStats);

		static const INT NumLightmapTextureSizes = 4;
		static const INT LightMapSizes[NumLightmapTextureSizes];

		/** Name of the Level this StaticMeshActor is on */
		FString						LevelName;
		/** Name of the StaticMeshActor this optimization is for */
		FString						ActorName;
		/** Name of the StaticMesh belonging to the above StaticMeshActor */
		FString						SMName;
		/** Current type of lighting scheme used */
		ELightMapInteractionType    IsType;        
		/** Texture size of the current lighting scheme, if texture, 0 otherwise */
		INT                         TextureSize;   
		/** Amount of memory used by the current lighting scheme */
		INT							CurrentBytesUsed;
		/** Amount of memory savings for each lighting scheme (256,128,64,32 pixel lightmaps + vertex lighting) */
		INT							BytesSaved[NumLightmapTextureSizes + 1];
	};

	/**
	 * Encapsulates gathered stats for a particular USoundCue object.
	 */
	struct FSoundCueStats : public FAssetStatsBase
	{
		/** Constructor, initializing all members. */
		FSoundCueStats( USoundCue* SoundCue );

		/**
		 * Stringifies gathered stats in CSV format.
		 *
		 * @return comma separated list of stats
		 */
		FString ToCSV() const;
		/** This takes a LevelName and then looks for the number of Instances of this StatMesh used within that level **/
		FString ToCSV( const FString& LevelName ) const;
		/** @return TRUE if this asset type should be logged */
		UBOOL ShouldLogStat() const
		{
			return TRUE;
		}

		/**
		 * Returns a header row for CSV
		 *
		 * @return comma separated header row
		 */
		static FString GetCSVHeaderRow();

		/** Resource type.																*/
		FString ResourceType;
		/** Resource name.																*/
		FString ResourceName;
		/** FaceFX anim  name.															*/
		FString FaceFXAnimName;
		/** FaceFX group name.															*/
		FString FaceFXGroupName;
		/** Whether resource is referenced by script.									*/
		UBOOL bIsReferencedByScript;
		/** Resource size of static mesh.												*/
		INT	ResourceSize;
	};

	/**
	 * Retrieves/ creates material stats associated with passed in material.
	 *
	 * @warning: returns pointer into TMap, only valid till next time Set is called
	 *
	 * @param	Material	Material to retrieve/ create material stats for
	 * @return	pointer to material stats associated with material
	 */
	FMaterialStats* GetMaterialStats( UMaterial* Material );

	/**
	 * Retrieves/ creates texture stats associated with passed in texture.
	 *
	 * @warning: returns pointer into TMap, only valid till next time Set is called
	 *
	 * @param	Texture		Texture to retrieve/ create texture stats for
	 * @return	pointer to texture stats associated with texture
	 */
	FTextureStats* GetTextureStats( UTexture* Texture );
	
	/**
	 * Retrieves/ creates static mesh stats associated with passed in static mesh.
	 *
	 * @warning: returns pointer into TMap, only valid till next time Set is called
	 *
	 * @param	StaticMesh	Static mesh to retrieve/ create static mesh stats for
	 * @return	pointer to static mesh stats associated with static mesh
	 */
	FStaticMeshStats* GetStaticMeshStats( UStaticMesh* StaticMesh, UPackage* LevelPackage );

	/**
	 * Retrieves/ creates particle stats associated with passed in particle system.
	 *
	 * @warning: returns pointer into TMap, only valid till next time Set is called
	 *
	 * @param	ParticleSystem	Particle system to retrieve/ create static mesh stats for
	 * @return	pointer to particle system stats associated with static mesh
	 */
	FParticleStats* GetParticleStats( UParticleSystem* ParticleSystem );

	/**
	 * Retrieves/creates texture in particle system stats associated with the passed in texture.
	 *
	 * @warning: returns pointer into TMap, only valid till next time Set is called
	 *
	 * @param	InTexture	The texture to retrieve/create stats for
	 * @return	pointer to textureinparticlesystem stats
	 */
	FTextureToParticleSystemStats* GetTextureToParticleSystemStats(UTexture* InTexture);

	/**
	 * Retrieves/ creates animation sequence stats associated with passed in animation sequence.
	 *
	 * @warning: returns pointer into TMap, only valid till next time Set is called
	 *
	 * @param	AnimSequence	Anim sequence to retrieve/ create anim sequence stats for
	 * @return	pointer to particle system stats associated with anim sequence
	 */
	FAnimSequenceStats* GetAnimSequenceStats( UAnimSequence* AnimSequence );

	/**
	* Retrieves/ creates lighting optimization stats associated with passed in static mesh actor.
	*
	* @warning: returns pointer into TMap, only valid till next time Set is called
	*
	* @param	ActorComponent	Actor component to calculate potential light map savings stats for
	* @return	pointer to lighting optimization stats associated with this actor component
	*/
	FLightingOptimizationStats* GetLightingOptimizationStats( AStaticMeshActor* ActorComponent );

	/**
	 * Retrieves/ creates sound cue stats associated with passed in sound cue.
	 *
	 * @warning: returns pointer into TMap, only valid till next time Set is called
	 *
	 * @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
	 * @return				pointer to sound cue  stats associated with sound cue  
	 */
	FSoundCueStats* GetSoundCueStats( USoundCue* SoundCue, UPackage* LevelPackage );

	/**
	* Retrieves/ creates shadowmap 1D stats associated with passed in shadowmap 1D object.
	*
	* @warning: returns pointer into TMap, only valid till next time Set is called
	*
	* @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
	* @return				pointer to sound cue  stats associated with sound cue  
	*/
	FShadowMap1DStats* GetShadowMap1DStats( UShadowMap1D* ShadowMap1D, UPackage* LevelPackage );

	/**
	* Retrieves/ creates shadowmap 2D stats associated with passed in shadowmap 2D object.
	*
	* @warning: returns pointer into TMap, only valid till next time Set is called
	*
	* @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
	* @return				pointer to sound cue  stats associated with sound cue  
	*/
	FShadowMap2DStats* GetShadowMap2DStats( UShadowMap2D* ShadowMap2D, UPackage* LevelPackage );

	void StaticInitialize();

	/**
	 * Handles encountered object, routing to various sub handlers.
	 *
	 * @param	Object			Object to handle
	 * @param	LevelPackage	Currently loaded level package, can be NULL if not a level
	 * @param	bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleObject( UObject* Object, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	
	/**
	 * Handles gathering stats for passed in static mesh.
	 *
	 * @param StaticMesh	StaticMesh to gather stats for.
	 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleStaticMesh( UStaticMesh* StaticMesh, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	/**
	 * Handles gathering stats for passed in static mesh component.
	 *
	 * @param StaticMeshComponent	StaticMeshComponent to gather stats for
	 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleStaticMeshComponent( UStaticMeshComponent* StaticMeshComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	/**
	* Handles special case for stats for passed in static mesh component who is part of a ParticleSystemComponent
	*
	* @param ParticleSystemComponent	ParticleSystemComponent to gather stats for
	* @param LevelPackage	Currently loaded level package, can be NULL if not a level
	* @param bIsScriptReferenced Whether object is handled because there is a script reference
	*/
	void HandleStaticMeshOnAParticleSystemComponent( UParticleSystemComponent* ParticleSystemComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced );

	/**
	 * Handles gathering stats for passed in material.
	 *
	 * @param Material		Material to gather stats for
 	 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleMaterial( UMaterial* Material, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	/**
	 * Handles gathering stats for passed in texture.
	 *
	 * @param Texture		Texture to gather stats for
	 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleTexture( UTexture* Texture, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	/**
	 * Handles gathering stats for passed in brush.
	 *
	 * @param Brush			Brush to gather stats for
	 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleBrush( ABrush* Brush, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	/**
	 * Handles gathering stats for passed in particle system.
	 *
	 * @param ParticleSystem	Particle system to gather stats for
	 * @param LevelPackage		Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleParticleSystem( UParticleSystem* ParticleSystem, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	/**
	 * Handles gathering stats for passed in animation sequence.
	 *
	 * @param AnimSequence		AnimSequence to gather stats for
	 * @param LevelPackage		Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleAnimSequence( UAnimSequence* AnimSequence, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	/**
	 * Handles gathering stats for passed in level.
	 *
	 * @param Level				Level to gather stats for
	 * @param LevelPackage		Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced Whether object is handled because there is a script reference
	 */
	void HandleLevel( ULevel* Level, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	/**
	* Handles gathering stats for passed in static actor component.
	*
	* @param Level				Level to gather stats for
	* @param LevelPackage		Currently loaded level package, can be NULL if not a level
	* @param bIsScriptReferenced Whether object is handled because there is a script reference
	*/
	void HandleStaticMeshActor( AStaticMeshActor* ActorComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced );

	/**
	 * Handles gathering stats for passed in sound cue.
	 *
	 * @param SoundCue				SoundCue to gather stats for.
	 * @param LevelPackage			Currently loaded level package, can be NULL if not a level
	 * @param bIsScriptReferenced	Whether object is handled because there is a script reference
	 */
	void HandleSoundCue( USoundCue* SoundCue, UPackage* LevelPackage, UBOOL bIsScriptReferenced );

	/**
	* Handles gathering stats for passed in shadow map 1D.
	*
	* @param SoundCue				SoundCue to gather stats for.
	* @param LevelPackage			Currently loaded level package, can be NULL if not a level
	* @param bIsScriptReferenced	Whether object is handled because there is a script reference
	*/
	void HandleShadowMap1D( UShadowMap1D* ShadowMap1D, UPackage* LevelPackage, UBOOL bIsScriptReferenced );

	/**
	* Handles gathering stats for passed in shadow map 2D.
	*
	* @param SoundCue				SoundCue to gather stats for.
	* @param LevelPackage			Currently loaded level package, can be NULL if not a level
	* @param bIsScriptReferenced	Whether object is handled because there is a script reference
	*/
	void HandleShadowMap2D( UShadowMap2D* ShadowMap2D, UPackage* LevelPackage, UBOOL bIsScriptReferenced );
	

	/** Mapping from a fully qualified resource string (including type) to static mesh stats info.								*/
	TMap<FString,FStaticMeshStats> ResourceNameToStaticMeshStats;
	/** Mapping from a fully qualified resource string (including type) to texture stats info.									*/
	TMap<FString,FTextureStats> ResourceNameToTextureStats;
	/** Mapping from a fully qualified resource string (including type) to material stats info.									*/
	TMap<FString,FMaterialStats> ResourceNameToMaterialStats;
	/** Mapping from a fully qualified resource string (including type) to particle stats info.									*/
	TMap<FString,FParticleStats> ResourceNameToParticleStats;
	/** Mapping from a full qualified resource string (including type) to texutreToParticleSystem stats info					*/
	TMap<FString,FTextureToParticleSystemStats> ResourceNameToTextureToParticleSystemStats;
	/** Mapping from a fully qualified resource string (including type) to anim stats info.										*/
	TMap<FString,FAnimSequenceStats> ResourceNameToAnimStats;																
	/** Mapping from a fully qualified resource string (including type) to lighting optimization stats info.					*/
	TMap<FString,FLightingOptimizationStats> ResourceNameToLightingStats;
	/** Mapping from a fully qualified resource string (including type) to sound cue stats info.								*/
	TMap<FString,FSoundCueStats> ResourceNameToSoundCueStats;
	/** Mapping from a fully qualified resource string (including type) to shadowmap 1D.										*/
	TMap<FString,FShadowMap1DStats> ResourceNameToShadowMap1DStats;
	/** Mapping from a fully qualified resource string (including type) to shadowmap 2D.										*/
	TMap<FString,FShadowMap2DStats> ResourceNameToShadowMap2DStats;

	//@todo. If you add new object types, make sure to update this enumeration
	//		 as well as the optional command line.
	enum EIgnoreObjectFlags
	{
		IGNORE_StaticMesh				= 0x00000001,
		IGNORE_StaticMeshComponent		= 0x00000002,
		IGNORE_StaticMeshActor			= 0x00000004,
		IGNORE_Texture					= 0x00000008,
		IGNORE_Material					= 0x00000010,
		IGNORE_Particle					= 0x00000020,
		IGNORE_AnimSequence				= 0x00000040,
		IGNORE_LightingOptimization		= 0x00000080,
		IGNORE_SoundCue					= 0x00000100,
		IGNORE_Brush					= 0x00000200,
		IGNORE_Level					= 0x00000400,
		IGNORE_ShadowMap				= 0x00000800
	};

	INT IgnoreObjects;

	// Various shader types for logging

	/** Shader type for base pass pixel shader (no lightmap).  */
	FShaderType*	ShaderTypeBasePassNoLightmap;
	/** Shader type for base pass pixel shader (including lightmap). */
	FShaderType*	ShaderTypeBasePassAndLightmap;
	/** Shader type for point light with shadow map pixel shader. */
	FShaderType*	ShaderTypePointLightWithShadowMap;
END_COMMANDLET


BEGIN_COMMANDLET(AnalyzeFallbackMaterials,Editor)
/**
* Encapsulates gathered stats for a particular UMaterial object.
*/
struct FMaterialStats
{
	/** Constructor, initializing all members */
	FMaterialStats( UMaterial* Material, UAnalyzeFallbackMaterialsCommandlet* Commandlet );

	/**
	* Stringifies gathered stats for generated fallbacks in CSV format.
	*
	* @return comma separated list of stats
	*/
	FString GeneratedFallbackToCSV() const;

	/**
	* Stringifies gathered stats for manually specified fallbacks with errors in CSV format.
	*
	* @return comma separated list of stats
	*/
	FString FallbackErrorsToCSV() const;

	/**
	* Returns a Generated Fallback header row for CSV
	*
	* @return comma separated header row
	*/
	static FString GetGeneratedFallbackCSVHeaderRow();

	/**
	* Returns a Fallback Error header row for CSV
	*
	* @return comma separated header row
	*/
	static FString GetFallbackErrorsCSVHeaderRow();

	/** Resource type.																*/
	FString ResourceType;
	/** Resource name.																*/
	FString ResourceName;
	/** Whether resource is referenced by script.									*/
	UBOOL bIsReferencedByScript;
	/** The components that were dropped when compiling this material for sm2.		*/
	DWORD DroppedFallbackComponents;
	/** Whether the material is a fallback material.								*/
	UBOOL bIsFallbackMaterial;
	/** Compile errors from trying to cache the material's shaders.					*/
	TArray<FString> CompileErrors;
};

/**
* Retrieves/ creates material stats associated with passed in material.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	Material	Material to retrieve/ create material stats for
* @return	pointer to material stats associated with material
*/
FMaterialStats* GetMaterialStats( UMaterial* Material );

/**
* Handles encountered object, routing to various sub handlers.
*
* @param	Object			Object to handle
* @param	LevelPackage	Currently loaded level package, can be NULL if not a level
* @param	bIsScriptReferenced Whether object is handled because there is a script reference
*/
void HandleObject( UObject* Object, UPackage* LevelPackage, UBOOL bIsScriptReferenced );

/**
* Handles gathering stats for passed in material.
*
* @param Material		Material to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void HandleMaterial( UMaterial* Material, UPackage* LevelPackage, UBOOL bIsScriptReferenced );

/** Mapping from a fully qualified resource string (including type) to material stats info.									*/
TMap<FString,FMaterialStats> ResourceNameToMaterialStats;
END_COMMANDLET


BEGIN_COMMANDLET(BatchExport,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ExportLoc,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(CompareLoc,Editor)
	
	/**
	 * Contains information about a single localization file, any language.
	 */
	struct FLocalizationFile
	{
	private:
		/**
		 * The filename for the FConfigFile this FLocalizationFile represents.
		 */
		FFilename LocFilename;

		/** sections that do not exist in the counterpart file. */
		TArray<FString> UnmatchedSections;

		/** properties that are missing from the corresponding section in the other file */
		TArray<FString> UnmatchedProperties;

		/** properties that have identical values in the other file */
		TArray<FString> IdenticalProperties;

		/** the FConfigFile which contains the data for this loc file */
		FConfigFile* LocFile;

	public:

		/**
		 * Standard constructor
		 */
		FLocalizationFile( const FString& InPath );

		/** Copy ctor */
		FLocalizationFile( const FLocalizationFile& Other );

		/** Dtor */
		~FLocalizationFile();

		/**
		 * Determines whether this file is the counterpart for the loc file specified
		 */
		UBOOL IsCounterpartFor( const FLocalizationFile& Other ) const;

		/**
		 * Compares the data in this loc file against the data in the specified counterpart file, placing the results in the various tracking arrays.
		 */
		void CompareToCounterpart( FLocalizationFile* Other );

		/** Accessors */
		const FString GetFullName()			const	{ return LocFilename; }
		const FString GetDirectoryName()	const	{ return LocFilename.GetPath(); }
		const FString GetFilename()			const	{ return LocFilename.GetBaseFilename(); }
		const FString GetExtension()		const	{ return LocFilename.GetExtension(); }
		class FConfigFile* GetFile()		const	{ return LocFile; }

		void GetMissingSections( TArray<FString>& out_Sections ) const;
		void GetMissingProperties( TArray<FString>& out_Properties ) const;
		void GetIdenticalProperties( TArray<FString>& out_Properties ) const;
	};

	/**
	 * Contains information about a localization file and its english counterpart.
	 */
	struct FLocalizationFilePair
	{
		FLocalizationFile* EnglishFile, *ForeignFile;

		/** Default ctor */
		FLocalizationFilePair() : EnglishFile(NULL), ForeignFile(NULL) {}
		~FLocalizationFilePair();

		/**
		 * Compares the two loc files against each other.
		 */
		void CompareFiles();

		/**
		 * Builds a list of files which exist in the english directory but don't have a counterpart in the foreign directory.
		 */
		void GetMissingLocFiles( TArray<FString>& Files );

		/**
		 * Builds a list of files which no longer exist in the english loc directories.
		 */
		void GetObsoleteLocFiles( TArray<FString>& Files );

		/**
		 * Builds a list of section names which exist in the english version of the file but don't exist in the foreign version.
		 */
		void GetMissingSections( TArray<FString>& Sections );

		/**
		 * Builds a list of section names which exist in the foreign version but no longer exist in the english version.
		 */
		void GetObsoleteSections( TArray<FString>& Sections );

		/**
		 * Builds a list of key names which exist in the english version of the file but don't exist in the foreign version.
		 */
		void GetMissingProperties( TArray<FString>& Properties );

		/**
		 * Builds a list of section names which exist in the foreign version but no longer exist in the english version.
		 */
		void GetObsoleteProperties( TArray<FString>& Properties );

		/**
		 * Builds a list of property names which have the same value in the english and localized version of the file, indicating that the value isn't translated.
		 */
		void GetUntranslatedProperties( TArray<FString>& Properties );

		/**
		 * Assigns the english version of the loc file pair.
		 */
		UBOOL SetEnglishFile( const FString& EnglishFilename );

		/**
		 * Assigns the foreign version of this loc file pair.
		 */
		UBOOL SetForeignFile( const FString& ForeignFilename );

		/** returns the filename (without path or extension info) for this file pair */
		const FString GetFilename();
		UBOOL HasEnglishFile();
		UBOOL HasForeignFile();
		UBOOL HasEnglishFile( const FString& Filename );
		UBOOL HasForeignFile( const FString& Filename );
	};

	/**
	 * Returns the index of the loc file pair that contains the english version of the specified filename, or INDEX_NONE if it isn't found
	 */
	INT FindEnglishIndex( const FString& Filename );

	/**
	 * Returns the index of the loc file pair that contains the english version of the specified filename, or INDEX_NONE if it isn't found
	 */
	INT FindForeignIndex( const FString& Filename );

	/**
	 * Adds the specified file as the english version for a loc file pair
	 */
	void AddEnglishFile( const FString& Filename );

	/**
	 * Adds the specified file as the foreign version for a loc file pair
	 */
	void AddForeignFile( const FString& Filename );

	/**
	 * Initializes the LocPairs arrays using the list of filenames provided.
	 */
	void ReadLocFiles( const TArray<FString>& EnglishFilenames, TArray<FString>& ForeignFilenames );

	FString LangExt;
	TArray<FLocalizationFilePair> LocPairs;

END_COMMANDLET

BEGIN_COMMANDLET(Conform,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(LoadPackage,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ConvertEmitters,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(DumpEmitters,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ConvertUberEmitters,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(FixupEmitters,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(FindEmitterMismatchedLODs,Editor)
	void CheckPackageForMismatchedLODs( const FFilename& Filename, UBOOL bFixup );
END_COMMANDLET

BEGIN_COMMANDLET(FindEmitterModifiedLODs,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(FindEmitterModuleLODErrors,Editor)
	UBOOL CheckModulesForLODErrors( INT LODIndex, INT ModuleIndex, 
		const UParticleEmitter* Emitter, const UParticleModule* CurrModule );
	void CheckPackageForModuleLODErrors( const FFilename& Filename, UBOOL bFixup );
END_COMMANDLET

BEGIN_COMMANDLET(FixupRedirects,Editor)
	/**
	 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. If
	 * the commandlet implements this function, it should fully initialize the UEngine object as well.  Commandlets
	 * should indicate that they have implemented this function by assigning the custom UEngine to GEngine.
	 */
	virtual void CreateCustomEngine();
END_COMMANDLET

BEGIN_COMMANDLET(FixupSourceUVs,Editor)
	virtual void FixupUVs( INT step, FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles );
	virtual UBOOL FindUV( const FStaticMeshTriangle * RawTriangleData, INT UVChannel, INT NumRawTriangles, const FVector2D &UV );
	virtual UBOOL ValidateUVChannels( const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles );
	virtual UBOOL CheckFixableType( INT step, const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles );
	virtual INT CheckFixable( const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles );
	virtual UBOOL CheckUVs( FStaticMeshRenderData * StaticMeshRenderData, const FStaticMeshTriangle * RawTriangleData );
END_COMMANDLET

BEGIN_COMMANDLET(CreateStreamingWorld,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ListPackagesReferencing,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(PkgInfo,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ResavePackages,Editor)
	/** only packages that have this version or higher will be resaved; a value of IGNORE_PACKAGE_VERSION indicates that there is no minimum package version */
	INT MinResaveVersion;

	/** only packages that have this version or lower will be resaved; a value of IGNORE_PACKAGE_VERSION indicates that there is no maximum package version */
	INT MaxResaveVersion;

	/** allows users to save only packages with a particular class in them (useful for fixing content) */
	TLookupMap<FName> ResaveClasses;

	/**
	 * Evalutes the command-line to determine which maps to check.  By default all maps are checked (except PIE and trash-can maps)
	 * Provides child classes with a chance to initialize any variables, parse the command line, etc.
	 *
	 * @param	Tokens			the list of tokens that were passed to the commandlet
	 * @param	Switches		the list of switches that were passed on the commandline
	 * @param	MapPathNames	receives the list of path names for the maps that will be checked.
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT InitializeResaveParameters( const TArray<FString>& Tokens, const TArray<FString>& Switches, TArray<FFilename>& MapPathNames );

	/**
	 * Allow the commandlet to perform any operations on the export/import table of the package before all objects in the package are loaded.
	 *
	 * @param	PackageLinker	the linker for the package about to be loaded
	 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
	 *							[out]	set to TRUE to resave the package
	 */
	virtual UBOOL PerformPreloadOperations( ULinkerLoad* PackageLinker, UBOOL& bSavePackage );

	/**
	 * Allows the commandlet to perform any additional operations on the object before it is resaved.
	 *
	 * @param	Object			the object in the current package that is currently being processed
	 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
	 *							[out]	set to TRUE to resave the package
	 */
	virtual void PerformAdditionalOperations( class UObject* Object, UBOOL& bSavePackage );

	/**
	 * Allows the commandlet to perform any additional operations on the package before it is resaved.
	 *
	 * @param	Package			the package that is currently being processed
	 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
	 *							[out]	set to TRUE to resave the package
	 */
	void PerformAdditionalOperations( class UPackage* Package, UBOOL& bSavePackage );

	/**
	 * Removes any UClass exports from packages which aren't script packages.
	 *
	 * @param	Package			the package that is currently being processed
	 *
	 * @return	TRUE to resave the package
	 */
	UBOOL CleanClassesFromContentPackages( class UPackage* Package );

	/**
	 * Instances subobjects for any existing objects with subobject properties pointing to the default object.
	 * This is currently the case when a classes has an object property and subobject definition added to it --
	 * existing instances of such a class will see the new object property refer to the template object.
	 *
	 * @param	Package			The package that is currently being processed.
	 *
	 * @return					TRUE to resave the package.
	 */
	UBOOL InstanceMissingSubObjects(class UPackage* Package);

END_COMMANDLET

BEGIN_CHILD_COMMANDLET(ChangePrefabSequenceClass, ResavePackages, Editor)

	/**
	 * Allow the commandlet to perform any operations on the export/import table of the package before all objects in the package are loaded.
	 *
	 * @param	PackageLinker	the linker for the package about to be loaded
	 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
	 *							[out]	set to TRUE to resave the package
	 */
	virtual UBOOL PerformPreloadOperations( ULinkerLoad* PackageLinker, UBOOL& bSavePackage );

	/**
	 * Allows the commandlet to perform any additional operations on the object before it is resaved.
	 *
	 * @param	Object			the object in the current package that is currently being processed
	 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
	 *							[out]	set to TRUE to resave the package
	 */
	virtual void PerformAdditionalOperations( class UObject* Object, UBOOL& bSavePackage );

END_CHILD_COMMANDLET

BEGIN_COMMANDLET(CutDownContent,Editor)
	/**
	 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. If
	 * the commandlet implements this function, it should fully initialize the UEngine object as well. Commandlets
	 * should indicate that they have implemented this function by assigning the custom UEngine to GEngine.
	 */
	virtual void CreateCustomEngine();
END_COMMANDLET

BEGIN_COMMANDLET(ScaleAudioVolume,Editor)
END_COMMANDLET

/** Forward declaration of config cache class... */
class FConfigCacheIni;

BEGIN_COMMANDLET(CookPackages,Editor)
	/** The implementation of the PC-side support functions for the targetted platform. */
	class FConsoleSupport* ConsoleSupport;

	/** What platform are we cooking for?														*/
	UE3::EPlatformType				Platform;
	/** The shader type with which to compile shaders											*/
	EShaderPlatform					ShaderPlatform;
	
	/**
	 * Cooking helper classes for resources requiring platform specific cooking
	 */
	struct FConsoleTextureCooker*		TextureCooker;
	class FConsoleSoundCooker*			SoundCooker;
	struct FConsoleSkeletalMeshCooker*	SkeletalMeshCooker;
	struct FConsoleStaticMeshCooker*	StaticMeshCooker;

	/** The command-line tokens and switches */
	TArray<FString> Tokens;
	TArray<FString> Switches;

	/** Cooked data directory																	*/
	FString							CookedDir;
	
	/** Whether to only cook dependencies of passed in packages/ maps							*/
	UBOOL							bOnlyCookDependencies;
	/** Whether to skip cooking maps.															*/
	UBOOL							bSkipCookingMaps;
	/** Whether to skip saving maps.															*/
	UBOOL							bSkipSavingMaps;
	/**	Whether to skip loading and saving maps not necessarily required like texture packages.	*/
	UBOOL							bSkipNotRequiredPackages;
	/** Always recook seekfree files, even if they haven't changed since cooking.				*/
	UBOOL							bForceRecookSeekfree;
	/** Only cook ini and localization files.													*/
	UBOOL							bIniFilesOnly;
	/** Generate SHA hashes.																	*/
	UBOOL							bGenerateSHAHashes;
	/** Should the cooker preassemble ini files and copy those to the Xbox						*/
	UBOOL							bShouldPreFinalizeIniFilesInCooker;
	/** TRUE if shared MP resources should be cooked separately rather than duplicated into seek-free MP level packages. */
	UBOOL							bSeparateSharedMPResources;
	/** TRUE to cook out static mesh actors														*/
	UBOOL							bCookOutStaticMeshActors;
	/** TRUE to cook out static light actors													*/
	UBOOL							bCookOutStaticLightActors;
	/** TRUE to bake and prune matinees that are tagged as such.								*/
	UBOOL							bBakeAndPruneDuringCook;
	/** TRUE to pull FaceFX anims used in level into the Persistent level itself.				*/
	UBOOL							bGeneratePersistentMapAnimSet;
	/** TRUE if the cooker is in user-mode, which won't cook script, etc						*/
	UBOOL							bIsInUserMode;
	/** Whether we are using the texture file cache for texture streaming.						*/
	UBOOL							bUseTextureFileCache;
	/** Alignment for bulk data stored in the texture file cache								*/
	DWORD							TextureFileCacheBulkDataAlignment;
	/** TRUE if we want to cook all maps														*/
	UBOOL							bCookAllMaps;
	/** TRUE if we are cooking as a distributed job												*/
	UBOOL							bIsDistributed;
	/** TRUE if job merges should happen														*/
	UBOOL							bMergeJobs;
	/** TRUE if we only want to cook maps, skipping the non-seekfree, script, startup, etc packages */
	UBOOL							bCookMapsOnly;
	/** TRUE if we want to only save the shader caches when we're done							*/
	UBOOL							bSaveShaderCacheAtEnd;
	/** Disallow map and package compression if option is set.									*/
	UBOOL							bDisallowPackageCompression;
	/** TRUE if we want to convert sRGB textures to a piecewise-linear approximation of sRGB	*/
	UBOOL							bShouldConvertPWLGamma;
	/** 
	 *	TRUE if we want to generate coalesced files for only the language being cooked for.		
	 *	Also, will not put the subtitles of other languages in the SoundNodeWaves!
	 *	Indicated by commandline option 'NOLOCCOOKING'
	 */
	UBOOL							bCookCurrentLanguageOnly;
	/** 
	 *	TRUE if we want to verify the texture file cache for all textures that get cooked.
	 *	Indicated by commandline option 'VERIFYTFC'
	 */
	UBOOL							bVerifyTextureFileCache;
	/** Array of names of editor-only packages.													*/
	TArray<FString>					EditorOnlyContentPackageNames;
	/** Detail mode of target platform.															*/
	INT								PlatformDetailMode;
	/** Archive used to write texture file cache.												*/
	TMap<FName,FArchive*>			TextureCacheNameToArMap;
	/** Filenames of texture cache Archives														*/
	TMap<FName,FString>				TextureCacheNameToFilenameMap;
	/** Mod name to cook for user-created mods													*/
	FString							UserModName;
	/** Name of the job, for where to output to													*/
	FString							JobName;
	/** Packages to cook, for this job															*/
	TSet<FString>					JobPackagesToCook;
	/** Set containing all packages that need to be cooked if bOnlyCookDependencies	== TRUE.	*/
	TMap<FString,INT>				PackageDependencies;
	/** Regular packages required to be cooked/ present.										*/
	TArray<FString>					RequiredPackages;
	/** Persistent data stored by the cooking process, e.g. used for bulk data separation.		*/
	UPersistentCookerData*			PersistentCookerData;
	/** Guid cache saved by the cooker, used at runtime for looking up packages guids for packagemap */
	class UGuidCache*				GuidCache;
	/** LOD settings used on target platform.													*/
	FTextureLODSettings				PlatformLODSettings;
	/** Shader cache saved into seekfree packages.												*/
	UShaderCache*					ShaderCache;
	/** Set of materials that have already been put into an always loaded shader cache.			*/
	TSet<FString>					AlreadyHandledMaterials;
	/** Set of material instances that have already been put into an always loaded shader cache. */
	TSet<FString>					AlreadyHandledMaterialInstances;
	/** A list of files to generate SHA hash values for											*/
	TArray<FString>					FilesForSHA;
	/** Remember the name of the target platform's engine .ini file								*/
	TCHAR							PlatformEngineConfigFilename[1024];
	/** List of objects to NEVER cook into packages. */
	TMap<FString,INT>				NeverCookObjects;
	/** Entries for verifying the texture file cache. */
	TMap<FString,FTextureFileCacheEntry>	TFCVerificationData;
	TArray<FTextureFileCacheEntry> TFCCheckData;
	TArray<FTextureFileCacheEntry> CharTFCCheckData;

	/** Tracking Kismet-based items to cook out */
	TArray<UInterpData*> UnreferencedMatineeData;

	/** Helper structure for tracking real FaceFX animations to the mangled ones */
	struct FMangledFaceFXInfo
	{
		FString FaceFXAnimSet;
		FString OriginalFaceFXGroupName;
		FString OriginalFaceFXAnimName;
		FString MangledFaceFXGroupName;
		FString MangledFaceFXAnimName;

		FMangledFaceFXInfo()
		{
			appMemzero(this, sizeof(FMangledFaceFXInfo));
		}

		FMangledFaceFXInfo(const FMangledFaceFXInfo& Src)
		{
			FaceFXAnimSet = Src.FaceFXAnimSet;
			OriginalFaceFXGroupName = Src.OriginalFaceFXGroupName;
			OriginalFaceFXAnimName = Src.OriginalFaceFXAnimName;
			MangledFaceFXGroupName = Src.MangledFaceFXGroupName;
			MangledFaceFXAnimName = Src.MangledFaceFXAnimName;
		}

		FMangledFaceFXInfo& operator=(const FMangledFaceFXInfo& Src)
		{
			FaceFXAnimSet = Src.FaceFXAnimSet;
			OriginalFaceFXGroupName = Src.OriginalFaceFXGroupName;
			OriginalFaceFXAnimName = Src.OriginalFaceFXAnimName;
			MangledFaceFXGroupName = Src.MangledFaceFXGroupName;
			MangledFaceFXAnimName = Src.MangledFaceFXAnimName;
			return *this;
		}

		UBOOL operator==(const FMangledFaceFXInfo& Src) const
		{
			if ((FaceFXAnimSet == Src.FaceFXAnimSet) && 
				(OriginalFaceFXGroupName == Src.OriginalFaceFXGroupName) && 
				(OriginalFaceFXAnimName == Src.OriginalFaceFXAnimName) && 
				(MangledFaceFXGroupName == Src.MangledFaceFXGroupName) && 
				(MangledFaceFXAnimName == Src.MangledFaceFXAnimName))
			{
				return TRUE;
			}
			return FALSE;
		}

		UBOOL operator!=(const FMangledFaceFXInfo& Src) const
		{
			return !(*this == Src);
		}
	};

	/** Helper typedef for a string to MangledFaceFXInfo map */
	/** AnimSetRef name --> MangledFaceFXInfo */
	typedef TMap<FString, FMangledFaceFXInfo> TMangledFaceFXMap;
	/** Group.Anim name --> TMangledFaceFXMap */
	typedef TMap<FString, TMangledFaceFXMap> TGroupAnimToMangledFaceFXMap;
	/** PMap name --> TGroupAnimToMangledFaceFXMap */
	typedef TMap<FString, TGroupAnimToMangledFaceFXMap> TPMapToGroupAnimLookupMap;

	TPMapToGroupAnimLookupMap PersistentMapToGroupAnimLookupMap;
	TGroupAnimToMangledFaceFXMap* CurrentPMapGroupAnimLookupMap;

	typedef TArray<TMangledFaceFXMap> TMangledFaceFXMapArray;

	TMap<FString, TArray<FMangledFaceFXInfo>> PersistentMapFaceFXArray;
	TArray<FMangledFaceFXInfo>* CurrentPersistentMapFaceFXArray;

	/** Mapping of Sublevels to their parent Persistent map */
	TMap<FString, FString> LevelToPersistentLevelMap;
	/** The GroupName concatanation to the mangled group name */
	TMap<FString, FString> GroupNameToMangledMap;
	/** The current mangled name count */
	INT FaceFXMangledNameCount;

	/** Script-references FaceFXAnimSets (so they properly cook out) */
	TMap<FString, INT> ScriptReferencedFaceFXAnimSets;

	/** 
	 *	If TRUE, then log out the generation of persistent facefx...
	 *	Pass "-LOGPERSISTENTFACEFX" on the commandline
	 */
	UBOOL bLogPersistentFaceFXGeneration;

	/**
	 *	Generate the mappings of sub-levels to PMaps...
	 *
	 *	@param	CommandLineMapList		The array of map names passed in on the command-line
	 */
	void GeneratePersistentMapList(TArray<FString>& CommandLineMapList);

	/** 
	 *	Check the given map package for persistent level.
	 *	If it is, then generate the corresponding FaceFX animation information.
	 *
	 *	@param	InPackageFile	The name of the package being loaded
	 */
	void SetupPersistentMapFaceFXAnimation(const FFilename& InPackageFile);

	/** 
	 *	Generate the persistent level FaceFX AnimSet.
	 *
	 *	@param	InPackageFile	The name of the package being loaded
	 *	@return	UFaceFXAnimSet*	The generated anim set, or NULL if not the persistent map
	 */
	UFaceFXAnimSet* GeneratePersistentMapFaceFXAnimSet(const FFilename& InPackageFile);

	/** 
	 *	Prep the given package w.r.t. localization.
	 */
	void PrepPackageLocalization( UPackage* Package );

	/**
	 * Cooks passed in object if it hasn't been already.
	 *
	 * @param	Package						Package going to be saved
	 * @param	 Object		Object to cook
	 * @param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
	 */
	void CookObject( UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage );
	/**
	 * Helper function used by CookObject - performs texture specific cooking.
	 *
	 * @param	Package						Package going to be saved
	 * @param	Texture2D	Texture to cook
	 * @param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
	 */
	void CookTexture( UPackage* Package, UTexture2D* Texture2D, UBOOL bIsSavedInSeekFreePackage );
	/**
	* Helper function used by CookObject - performs movie specific cooking.
	*
	* @param	TextureMovie	Movie texture to cook
	*/
	void CookMovieTexture( UTextureMovie* TextureMovie );

	/**
	 * Helper function used by CookObject - performs ParticleSystem specific cooking.
	 *
	 * @param	ParticleSystem	ParticleSystem to cook
	 */
	void CookParticleSystem(UParticleSystem* ParticleSystem);

	/**
	 * Helper function used by CookObject - performs SkeletalMesh specific cooking.
	 *
	 * @param	SkeletalMesh	SkeletalMesh to cook
	 */
	void CookSkeletalMesh(USkeletalMesh* SkeletalMesh);

	/**
	 * Helper function used by CookObject - performs StaticMesh specific cooking.
	 *
	 * @param	StaticMesh	StaticMesh to cook
	 */
	void CookStaticMesh(UStaticMesh* StaticMesh);

	/**
	 * Cooks out all static mesh actors in the specified package by re-attaching their StaticMeshComponents to
	 * a StaticMeshCollectionActor referenced by the world.
	 *
	 * @param	Package		the package being cooked
	 */
	void CookStaticMeshActors( UPackage* Package );

	/**
	 * Cooks out all static Light actors in the specified package by re-attaching their LightComponents to a 
	 * StaticLightCollectionActor referenced by the world.
	 */
	void CookStaticLightActors( UPackage* Package );

	/**
	 *	Clean up the kismet for the given level...
	 *	Remove 'danglers' - sequences that don't actually hook up to anything, etc.
	 *
	 *	@param	Package		The map being cooked
	 */
	void CleanupKismet(UPackage* Package);

	/**
	 *	Bake and prune all matinee sequences that are tagged as such.
	 */
	void BakeAndPruneMatinee( UPackage* Package );

	/**
	 * Prepares object for saving into package. Called once for each object being saved 
	 * into a new package.
	 *
	 * @param	Package						Package going to be saved
	 * @param	Object						Object to prepare
	 * @param	bIsSavedInSeekFreePackage	Whether object is going to be saved into a seekfree package
	 * @param	bIsTextureOnlyFile			Whether file is only going to contain texture mips
	 */
	void PrepareForSaving( UPackage* Package, UObject* Object, UBOOL bIsSavedInSeekFreePackage, UBOOL bIsTextureOnlyFile );
	
	/**
	 * Helper function used by CookObject - performs sound cue specific cooking.
	 */
	void CookSoundCue( USoundCue* SoundCue );

	/**
	 * Helper function used by CookObject - performs sound specific cooking.
	 */
	void CookSoundNodeWave( USoundNodeWave* SoundNodeWave );

	/**
	 * Helper function used by CookSoundNodeWave - localises sound
	 */
	void LocSoundNodeWave( USoundNodeWave* SoundNodeWave );

	/**
	 * Make sure materials are compiled for Xbox 360 and add them to the shader cache embedded into seekfree packages.
	 * @param Material - Material to process
	 */
	void CompileMaterialShaders( UMaterial* Material );

	/**
	* Make sure material instances are compiled and add them to the shader cache embedded into seekfree packages.
	* @param MaterialInterface - MaterialInterface to process
	*/
	void CompileMaterialInstanceShaders( UMaterialInstance* MaterialInterface );

	/**
	 * Setup the commandlet's platform setting based on commandlet params
	 * @param Params The commandline parameters to the commandlet - should include "platform=xxx"
	 *
	 * @return TRUE if a good known platform was found in Params
	 */
	UBOOL SetPlatform(const FString& Params);

	/**
	 * Tried to load the DLLs and bind entry points.
	 *
	 * @return	TRUE if successful, FALSE otherwise
	 */
	UBOOL BindDLLs();

	/**
	* Update all game .ini files from defaults
	*
	* @param IniPrefix	prefix for ini filename in case we want to write to cooked path
	*/
	void UpdateGameIniFilesFromDefaults(const TCHAR* IniPrefix);

	/**
	 * Precreate all the .ini files that the platform will use at runtime
	 * @param bAddForHashing - TRUE if running with -sha and ini files should be added to list of hashed files
	 */
	void CreateIniFiles(UBOOL bAddForHashing);

	/**
	* If -sha is specified then iterate over all FilesForSHA and generate their hashes
	* The results are written to Hashes.sha
	*/
	void GenerateSHAHashes();

	/** 
	 * Prepares shader files for the given platform to make sure they can be used for compiling
	 *
	 *	@return			TRUE on success, FALSE on failure.
	 */
	UBOOL PrepareShaderFiles();

	/** 
	 * Cleans up shader files for the given platform 
	 */
	void CleanupShaderFiles();

	/**
	 * Warns the user if the map they are cooking has references to editor content (EditorMeshes, etc)
	 *
	 * @param Package Package that has been loaded by the cooker
	 */
	void WarnAboutEditorContentReferences(UPackage* Package);

	/**
	 * Loads a package that will be used for cooking. This will cache the source file time
	 * and add the package to the Guid Cache
	 *
	 * @param Filename Name of package to load
	 *
	 * @return Package that was loaded and cached
	 */
	UPackage* LoadPackageForCooking(const TCHAR* Filename);

	/**
	 * Force load a package and emit some useful info
	 * 
	 * @param PackageName Name of package, could be different from filename due to localization
	 * @param bRequireServerSideOnly If TRUE, the loaded packages are required to have the PKG_ServerSideOnly flag set for this function to succeed
	 *
	 * @return TRUE if successful
	 */
	UBOOL ForceLoadPackage(const FString& PackageName, UBOOL bRequireServerSideOnly=FALSE);

	/**
	 * Load all packages in a specified ini section with the Package= key
	 * @param SectionName Name of the .ini section ([Engine.PackagesToAlwaysCook])
	 * @param PackageNames Paths of the loaded packages
	 * @param KeyName Optional name for the key of the list to load (defaults to "Package")
	 * @param bShouldSkipLoading If TRUE, this function will only fill out PackageNames, and not load the package
	 * @param bRequireServerSideOnly If TRUE, the loaded packages are required to have the PKG_ServerSideOnly flag set for this function to succeed
	 * @return if loading was required, whether we successfully loaded all the packages; otherwise, always TRUE
	 */
	UBOOL LoadSectionPackages(const TCHAR* SectionName, TArray<FString>& PackageNames, const TCHAR* KeyName=TEXT("Package"), UBOOL bShouldSkipLoading=FALSE, UBOOL bRequireServerSideOnly=FALSE);

	/**
	 * We use the CreateCustomEngine call to set some flags which will allow SerializeTaggedProperties to have the correct settings
	 * such that editoronly and notforconsole data can correctly NOT be loaded from startup packages (e.g. engine.u)
	 *
	 **/
	virtual void CreateCustomEngine();

	/**
	 * Performs command line and engine specific initialization.
	 *
	 * @param	Params	command line
	 * @param	bQuitAfterInit [out] If TRUE, the caller will quit the commandlet, even if the Init function returns TRUE
	 * @return	TRUE if successful, FALSE otherwise
	 */
	UBOOL Init( const TCHAR* Params, UBOOL& bQuitAfterInit );

	/**
	 * Check if any dependencies of a seekfree package are newer than the cooked seekfree package.
	 * If they are, the package needs to be recooked.
	 *
	 * @param SrcLinker Optional source package linker to check dependencies for when no src file is available
	 * @param SrcFilename Name of the source of the seekfree package
	 * @param DstTimestamp Timestamp of the cooked version of this package
	 *
	 * @return TRUE if any dependencies are newer, meaning to recook the package
	 */
	UBOOL AreSeekfreeDependenciesNewer(ULinkerLoad* SrcLinker, const FString& SrcFilename, DOUBLE DstTimestamp);

	/**
	 * Generates list of src/ dst filename mappings of packages that need to be cooked after taking the command
	 * line options into account.
	 *
	 * @param [out] FirstStartupIndex		index of first startup package in returned array, untouched if there are none
	 * @param [out]	FirstScriptIndex		index of first script package in returned array, untouched if there are none
	 * @param [out] FirstGameScriptIndex	index of first game script package in returned array, untouched if there are none
	 * @param [out] FirstMapIndex			index of first map package in returned array, untouched if there are none
	 * @param [out] FirstMPMapIndex			index of first map package in returned array, untouched if there are none
	 *
	 * @return	array of src/ dst filename mappings for packages that need to be cooked
	 */
	TArray<FPackageCookerInfo> GeneratePackageList( INT& FirstStartupIndex, INT& FirstScriptIndex, INT& FirstGameScriptIndex, INT& FirstMapIndex, INT& FirstMPMapIndex );
	/**
	 * Cleans up DLL handles and destroys cookers
	 */
	void Cleanup();
	/**
	 * Collects garbage and verifies all maps have been garbage collected.
	 */
	void CollectGarbageAndVerify();
	/**
	 * Handles duplicating cubemap faces that are about to be saved with the passed in package.
	 *
	 * @param	Package	 Package for which cubemaps that are going to be saved with it need to be handled.
	 */
	void HandleCubemaps( UPackage* Package );

	/**
	* Adds the mip data payload for the given texture and mip index to the texture file cache.
	* If an entry exists it will try to replace it if the mip is <= the existing entry or
	* the mip data will be appended to the end of the TFC file.
	* Also updates the bulk data entry for the texture mip with the saved size/offset.
	*
	* @param Package - Package for texture that is going to be saved
	* @param Texture - 2D texture with mips to be saved
	* @param MipIndex - index of mip entry in the texture that needs to be saved
	*/
	void SaveMipToTextureFileCache( UPackage* Package, UTexture2D* Texture, INT MipIndex );

	/**
	 * Saves the passed in package, gathers and stores bulk data info and keeps track of time spent.
	 *
	 * @param	Package						Package to save
	 * @param	Base						Base/ root object passed to SavePackage, can be NULL
	 * @param	TopLevelFlags				Top level "keep"/ save flags, passed onto SavePackage
	 * @param	DstFilename					Filename to save under
	 * @param	bStripEverythingButTextures	Whether to strip everything but textures
	 * @param	bCleanupAfterSave			Whether or not objects should have certain object flags cleared and remember if objects were saved
	 * @param	bRememberSavedObjects		TRUE if objects should be marked to not be saved again, as well as materials (if bRemeberSavedObjects is set)
	 */
	void SaveCookedPackage( UPackage* Package, UObject* Base, EObjectFlags TopLevelFlags, const TCHAR* DstFilename, UBOOL bStripEverythingButTextures, UBOOL bCleanupAfterSave=FALSE, UBOOL bRememberSavedObjects=FALSE );
	/**
	 * Returns whether there are any localized resources that need to be handled.
	 *
	 * @param Package			Current package that is going to be saved
	 * @param TopLevelFlags		TopLevelFlags that are going to be passed to SavePackage
	 * 
	 * @return TRUE if there are any localized resources pending save, FALSE otherwise
	 */
	UBOOL AreThereLocalizedResourcesPendingSave( UPackage* Package, EObjectFlags TopLevelFlags );

	/**
	 * Cooks the specified cues into the shared MP sound package.
	 *
	 * @param	MPCueNames		The set of sound cue names to cook.
	 */
	void CookMPSoundPackages(const TArray<FString>& MPCueNames);
	
	/**
	 * Merges the uncooked local and reference shader caches for the given platform and saves the merged cache as the
	 * cooked reference shader cache.
	 * @param	CookShaderPlatform	The platform whose ref shader cache will be copied.
	 */
	void CookReferenceShaderCache(EShaderPlatform CookShaderPlatform);

	/**
	 * Saves the global shader cache for the given platform in the cooked content folder with appropriate byte-order.
	 * @param	CookShaderPlatform	The platform whose ref shader cache will be copied.
	 */
	void CookGlobalShaderCache(EShaderPlatform CookShaderPlatform);

	/**
	 * Merge together the cache files resulting from distributed cooking jobs
	 *
	 * @param JobDirectories List of directories (inside appGameDir()) to merge together
	 */
	void CombineCaches(const TArray<FString>& JobDirectories);


	/**
	 * @return A string representing the platform
	 */
	FString GetPlatformString();

	/**
	 * @return THe prefix used in ini files, etc for the platform
	 */
	FString GetPlatformPrefix();

	/**
	 * @return The name of the output cooked data directory
	 */
	FString GetCookedDirectory();

	/**
	 * @return The name of the directory where cooked ini files go
	 */
	FString GetConfigOutputDirectory();

	/**
	 * @return The prefix to pass to appCreateIniNamesAndThenCheckForOutdatedness for non-platform specific inis
	 */
	FString GetConfigOutputPrefix();

	/**
	 * @return The prefix to pass to appCreateIniNamesAndThenCheckForOutdatedness for platform specific inis
	 */
	FString GetPlatformConfigOutputPrefix();

	/**
	 * @return The default ini prefix to pass to appCreateIniNamesAndThenCheckForOutdatedness for 
	 */
	FString GetPlatformDefaultIniPrefix();

	/**
	 * @return TRUE if the destination platform expects pre-byteswapped data (packages, coalesced ini files, etc)
	 */
	UBOOL ShouldByteSwapData();

	/**
	 * @return The name of the bulk data container file to use
	 */
	FString GetBulkDataContainerFilename();

	/**
	* Get the destination filename for the given source package file based on platform
	*
	* @param SrcFilename - source package file path
	* @return cooked filename destination path
	*/
	FFilename GetCookedPackageFilename( const FFilename& SrcFilename );

	/*
	 * @return The name of the guid cache file to use
	 */
	FString GetGuidCacheFilename();

	/**
	 * Determines the name of the texture cache to use for the specified texture.
	 * @param Texture - The texture to determine the texture cache name for.
	 * @return The name of the texture cache file to use.
	 */
	FString GetTextureCacheName(UTexture* Texture);

	/**
	 * Returns the archive which contains the texture file cache with the specified name.
	 * @param TextureCacheName - The name of the texture cache to access.
	 * @return A pointer to the archive; will always be non-NULL.
	 */
	FArchive* GetTextureCacheArchive(const FName& TextureCacheName);

	/**
	 *	Verifies that the data in the texture file cache for the given texture
	 *	is a valid 'header' packet...
	 *
	 *	@param	Package		The package the texture was cooked into.
	 *	@param	Texture2D	The texture that was cooked.
	 *	@param	bIsSaved...	If TRUE, the texture was saved in a seekfree pacakge
	 *
	 *	@return	UBOOL		TRUE if the texture cache entry was valid, FALSE if not.
	 */
	UBOOL VerifyTextureFileCacheEntry();

	UBOOL AddVerificationTextureFileCacheEntry(UPackage* Package, UTexture2D* Texture2D, UBOOL bIsSavedInSeekFreePackage);

	FTextureFileCacheEntry* FindTFCEntryOverlap(FTextureFileCacheEntry& InEntry);

END_COMMANDLET

BEGIN_COMMANDLET(PrecompileShaders, Editor)
	FString ForceName;
	TArray<EShaderPlatform> ShaderPlatforms;
	void ProcessMaterial(UMaterial* Material);
	void ProcessMaterialInstance(UMaterialInstance* MaterialInstance);
	void ProcessTerrain(class ATerrain* Terrain);
END_COMMANDLET

BEGIN_COMMANDLET(DumpShaders, Editor)
	TArray<EShaderPlatform> ShaderPlatforms;
END_COMMANDLET

BEGIN_COMMANDLET(RebuildMap,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(TestCompression,Editor)
	/**
	 * Run a compression/decompress test with the given package and compression options
	 *
	 * @param PackageName		The package to compress/decompress
	 * @param Flags				The options for compression
	 * @param UncompressedSize	The size of the uncompressed package file
	 * @param CompressedSize	The options for compressed package file
	 */
	void RunTest(const FFilename& PackageName, ECompressionFlags Flags, DWORD& UncompressedSize, DWORD& CompressedSize);
END_COMMANDLET

BEGIN_COMMANDLET(StripSource,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(MergePackages,Editor)
END_COMMANDLET


//====================================================================
// UDiffPackagesCommandlet and helper structs
//====================================================================

/**
 * The different types of comparison differences that can exist between packages.
 */
enum EObjectDiff
{
	/** no difference */
	OD_None,

	/** the object exist in the first package only */
	OD_AOnly,

	/** the object exists in the second package only */
	OD_BOnly,

	/** (three-way merges) the value has been changed from the ancestor package, but the new value is identical in the two packages being compared */
	OD_ABSame,

	/** @todo */
	OD_ABConflict,

	/** @todo */
	OD_Invalid,
};

/**
 * Contains an object and the object's path name.
 */
struct FObjectReference
{
	UObject* Object;
	FString ObjectPathName;

	FObjectReference( UObject* InObject )
	: Object(InObject)
	{
		if ( Object != NULL )
		{
			ObjectPathName = Object->GetPathName();
		}
	}
};


/**
 * Represents a single top-level object along with all its subobjects.
 */
struct FObjectGraph
{
	/**
	 * The list of objects in this object graph.  The first element is always the root object.
	 */
	TArray<struct FObjectReference> Objects;

	/**
	 * Constructor
	 *
	 * @param	RootObject			the top-level object for this object graph
	 * @param	PackageIndex		the index [into the Packages array] for the package that this object graph belongs to
	 * @param	ObjectsToIgnore		optional list of objects to not include in this object graph, even if they are contained within RootObject
	 */
	FObjectGraph( UObject* RootObject, INT PackageIndex, TArray<struct FObjectComparison>* ObjectsToIgnore=NULL);

	/**
	 * Returns the root of this object graph.
	 */
	inline UObject* GetRootObject() const { return Objects(0).Object; }
};

/**
 * Contains the natively serialized property data for a single UObject.
 */
struct FNativePropertyData 
{
	/** the object that this property data is for */
	UObject*				Object;

	/** the raw bytes corresponding to this object's natively serialized property data */
	TArray<BYTE>			PropertyData;

	/** the property names and textual representations of this object's natively serialized data */
	TMap<FString,FString>	PropertyText;

	/** Constructor */
	FNativePropertyData( UObject* InObject );

	/**
	 * Changes the UObject associated with this native property data container and re-initializes the
	 * PropertyData and PropertyText members
	 */
	void SetObject( UObject* NewObject );

	/** Comparison operators */
	inline UBOOL operator==( const FNativePropertyData& Other ) const
	{
		return ((Object == NULL) == (Other.Object == NULL)) && PropertyData == Other.PropertyData && LegacyCompareEqual(PropertyText,Other.PropertyText);
	}
	inline UBOOL operator!=( const FNativePropertyData& Other ) const
	{
		return ((Object == NULL) != (Other.Object == NULL)) || PropertyData != Other.PropertyData || LegacyCompareNotEqual(PropertyText,Other.PropertyText);
	}

	/** bool operator */
	inline operator UBOOL() const
	{
		return PropertyData.Num() || PropertyText.Num();
	}
};

BEGIN_COMMANDLET(DiffPackages,Editor)

	/**
	 * Parses the command-line and loads the packages being compared.
	 *
	 * @param	Parms	the full command-line used to invoke this commandlet
	 *
	 * @return	TRUE if all parameters were parsed successfully; FALSE if any of the specified packages couldn't be loaded
	 *			or the parameters were otherwise invalid.
	 */
	UBOOL Initialize( const TCHAR* Parms );

	/**
	 * Generates object graphs for the specified object and its corresponding objects in all packages being diffed.
	 *
	 * @param	RootObject			the object to generate the object comparison for
	 * @param	out_Comparison		the object graphs for the specified object for each package being diffed
	 * @param	ObjectsToIgnore		if specified, this list will be passed to the FObjectGraphs created for this comparison; this will prevent those object graphs from containing
	 *								these objects.  Useful when generating an object comparison for package-root type objects, such as levels, worlds, etc. to prevent their comparison's
	 *								object graphs from containing all objects in the level/world
	 *
	 * @return	TRUE if RootObject was found in any of the packages being compared.
	 */
	UBOOL GenerateObjectComparison( UObject* RootObject, struct FObjectComparison& out_Comparison, TArray<struct FObjectComparison>* ObjectsToIgnore=NULL );
	UBOOL ProcessDiff(struct FObjectComparison& Diff);

	EObjectDiff DiffObjects(UObject* ObjA, UObject* ObjB, UObject* ObjAncestor, struct FObjectComparison& PropDiffs);

	/**
	 * Copies the raw property values for the natively serialized properties of the specified object into the output var.
	 *
	 * @param	Object	the object to load values for
	 * @param	out_NativePropertyData	receives the raw bytes corresponding to Object's natively serialized property values.
	 */
	static void LoadNativePropertyData( UObject* Object, TArray<BYTE>& out_NativePropertyData );

	/**
	 * Compares the natively serialized property values for the specified objects by comparing the non-script serialized portion of each object's data as it
	 * is on disk.  If a different is detected, gives each object the chance to generate a textual representation of its natively serialized property values
	 * that will be displayed to the user in the final comparison report.
	 *
	 * @param	ObjA		the object from the first package being compared.  Can be NULL if both ObjB and ObjAncestor are valid, which indicates that this object
	 *						doesn't exist in the first package.
	 * @param	ObjB		the object from the second package being compared.  Can be NULL if both ObjA and ObjAncestor are valid, which indicates that this object
	 *						doesn't exist in the second package.
	 * @param	ObjAncestor	the object from the optional common base package.  Can only be NULL if both ObjA and ObjB are valid, which indicates that this is either
	 *						a two-comparison (if NumPackages == 2) or the object was added to both packages (if NumPackages == 3)
	 * @param	PropertyValueComparisons	contains the results for all property values that were different so far; for any native property values which are determined
	 *										to be different, new entries will be added to the ObjectComparison's list of PropDiffs.
	 *
	 * @return	The cumulative comparison result type for a comparison of all natively serialized property values.
	 */
	EObjectDiff CompareNativePropertyValues( UObject* ObjA, UObject* ObjB, UObject* ObjAncestor, struct FObjectComparison& PropertyValueComparisons );

	UBOOL bDiffNonEditProps;
	UBOOL bDiffAllProps;

	UPackage* Packages[3];
	INT NumPackages;
END_COMMANDLET

BEGIN_COMMANDLET(Make,Editor)
	/**
	 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. If
	 * the commandlet implements this function, it should fully initialize the UEngine object as well.  Commandlets
	 * should indicate that they have implemented this function by assigning the custom UEngine to GEngine.
	 */
	virtual void CreateCustomEngine();
	/**
	 * Deletes all dependent .u files.  Given an index into the EditPackages array, deletes the .u files corresponding to
	 * that index, as well as the .u files corresponding to subsequent members of the EditPackages array.
	 *
	 * @param	ScriptOutputDir		output directory for script packages.
	 * @param	PackageList			list of package names to delete
	 * @param	StartIndex			index to start deleting packages
	 * @param	Count				number of packages to delete - defaults to all
	 */
	void DeleteEditPackages( const FString& ScriptOutputDir, const TArray<FString>& PackageList, INT StartIndex, INT Count=INDEX_NONE ) const;
END_COMMANDLET

BEGIN_COMMANDLET(ShowTaggedProps,Editor)

	/**
	 * Optional list of properties to display values for.  If this array is empty, all serialized property values are logged.
	 */
	TLookupMap<UProperty*> SearchProperties;

	/**
	 * Optional list of properties to ignore when logging values.
	 */
	TLookupMap<UProperty*> IgnoreProperties;
	void ShowSavedProperties( UObject* Object ) const;
END_COMMANDLET

/**
 * Container for parameters of the UShowObjectCount::ProcessPackages.
 */
struct FObjectCountExecutionParms
{
	/** the list of classes to search for */
	TArray<UClass*> SearchClasses;

	/** Bitmask of flags that are required in order for a match to be listed in the results */
	EObjectFlags ObjectMask;

	/** TRUE to print out the names of all objects found */
	UBOOL bShowObjectNames:1;

	/** TRUE to include cooked packages in the list of packages to check */	
	UBOOL bIncludeCookedPackages:1;
	
	/** TRUE to check cooked packages only */
	UBOOL bCookedPackagesOnly:1;
	
	/** TRUE to ignore objects of derived classes */
	UBOOL bIgnoreChildren:1;
	
	/** TRUE to ignore packages which are writeable (useful for skipping test packages) */
	UBOOL bIgnoreCheckedOutPackages:1;
	
	/** TRUE to skip checking script packages */
	UBOOL bIgnoreScriptPackages:1;
	
	/** TRUE to skip checking map packages */
	UBOOL bIgnoreMapPackages:1;
	
	/** TRUE to skip checking content (non-map, non-script) packages */
	UBOOL bIgnoreContentPackages:1;
	
	/** Constructor */
	FObjectCountExecutionParms( const TArray<UClass*>& InClasses, EObjectFlags InMask=RF_LoadForClient|RF_LoadForServer|RF_LoadForEdit );
};

struct FPackageObjectCount
{
	/** the number of objects for a single class in a single package */
	INT				Count;
	/** the name of the package */
	FString			PackageName;
	/** the name of the object class this count is for */
	FString			ClassName;
	/** the path names for the objects found */
	TArray<FString>	ObjectPathNames;

	/** Constructors */
	FPackageObjectCount()
	: Count(0)
	{
	}
	FPackageObjectCount( const FString& inPackageName, const FString& inClassName, INT InCount=0 )
	: Count(InCount), PackageName(inPackageName), ClassName(inClassName)
	{
	}
};

BEGIN_COMMANDLET(ShowObjectCount,Editor)
	void StaticInitialize();

	/**
	 * Searches all packages for the objects which meet the criteria specified.
	 *
	 * @param	Parms				specifies the parameters to use for the search
	 * @param	Results				receives the results of the search
	 * @param	bUnsortedResults	by default, the list of results will be sorted according to the number of objects in each package;
	 *								specify TRUE to override this behavior.
	 */
	void ProcessPackages( const FObjectCountExecutionParms& Parms, TArray<FPackageObjectCount>& Results, UBOOL bUnsortedResults=FALSE );
END_COMMANDLET

BEGIN_COMMANDLET(CreateDefaultStyle,Editor)
	class UUISkin* DefaultSkin;

	class UUIStyle_Text* CreateTextStyle( const TCHAR* StyleName=TEXT("DefaultTextStyle"), FLinearColor StyleColor=FLinearColor(1.f,1.f,1.f,1.f) ) const;
	class UUIStyle_Image* CreateImageStyle( const TCHAR* StyleName=TEXT("DefaultImageStyle"), FLinearColor StyleColor=FLinearColor(1.f,1.f,1.f,1.f) ) const;
	class UUIStyle_Combo* CreateComboStyle( UUIStyle_Text* TextStyle, UUIStyle_Image* ImageStyle, const TCHAR* StyleName=TEXT("DefaultComboStyle") ) const;

	void CreateAdditionalStyles() const;

	void CreateConsoleStyles() const;

	void CreateMouseCursors() const;
END_COMMANDLET

BEGIN_COMMANDLET(ShowStyles,Editor)
	void DisplayStyleInfo( class UUIStyle* Style );

	INT GetStyleDataIndent( class UUIStyle* Style );
END_COMMANDLET

BEGIN_COMMANDLET(ExamineOuters,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ListCorruptedComponents,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(FindSoundCuesWithMissingGroups,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(FindTexturesWithMissingPhysicalMaterials,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(FindQuestionableTextures,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(FindDuplicateKismetObjects,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(SetTextureLODGroup,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(CompressAnimations,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(WrangleContent,Editor)
END_COMMANDLET


/** lists all content referenced in the default properties of script classes */
BEGIN_COMMANDLET(ListScriptReferencedContent,Editor)
	/** processes a value found by ListReferencedContent(), possibly recursing for inline objects
	 * @param Value the object to be processed
	 * @param Property the property where Value was found (for a dynamic array, this is the Inner property)
	 * @param PropertyDesc string printed as the property Value was assigned to (usually *Property->GetName(), except for arrays, where it's the array name and index)
	 * @param Tab string with a number of tabs for the current tab level of the output
	 */
	void ProcessObjectValue(UObject* Value, UProperty* Property, const FString& PropertyDesc, const FString& Tab);
	/** lists content referenced by the given data
	 * @param Struct the type of the Default data
	 * @param Default the data to look for referenced objects in
	 * @param HeaderName header string printed before any content references found (only if the data might contain content references)
	 * @param Tab string with a number of tabs for the current tab level of the output
	 */
	void ListReferencedContent(UStruct* Struct, BYTE* Default, const FString& HeaderName, const FString& Tab = TEXT(""));
END_COMMANDLET

BEGIN_COMMANDLET(FixAmbiguousMaterialParameters,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(TestWordWrap,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(SetMaterialUsage,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(SetPackageFlags,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(PIEToNormal,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(UT3MapStats,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(PerformMapCheck,Editor)
	/** the list of filenames for the maps that will be checked */
	TArray<FFilename> MapNames;

	/** the index of map currently being processed */
	INT MapIndex;

	/**
	 * The number of maps processed so far.
	 */
	INT TotalMapsChecked;

	/**
	 * unused for now.
	 */
	UBOOL bTestOnly;

	/**
	 * Evalutes the command-line to determine which maps to check.  By default all maps are checked (except PIE and trash-can maps)
	 * Provides child classes with a chance to initialize any variables, parse the command line, etc.
	 *
	 * @param	Tokens			the list of tokens that were passed to the commandlet
	 * @param	Switches		the list of switches that were passed on the commandline
	 * @param	MapPathNames	receives the list of path names for the maps that will be checked.
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT InitializeMapCheck( const TArray<FString>& Tokens, const TArray<FString>& Switches, TArray<FFilename>& MapPathNames );

	/**
	 * The main worker method - performs the commandlets tests on the package.
	 *
	 * @param	MapPackage	the current package to be processed
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT CheckMapPackage( UPackage* MapPackage );

	/**
	 * Called after all packages have been processed - provides commandlets with an opportunity to print out test results or
	 * provide feedback.
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT ProcessResults() { return 0; }
END_COMMANDLET

BEGIN_CHILD_COMMANDLET(FindStaticActorsRefs,PerformMapCheck,Editor)

	UBOOL bStaticKismetRefs;
	UBOOL bShowObjectNames;
	UBOOL bLogObjectNames;
	UBOOL bShowReferencers;
	UBOOL bFixPrefabSequences;

	INT TotalStaticMeshActors;
	INT TotalStaticLightActors;
	INT TotalReferencedStaticMeshActors;
	INT TotalReferencedStaticLightActors;
	INT TotalMapsChecked;
	TMap<INT, INT> ReferencedStaticMeshActorMap;
	TMap<INT, INT> ReferencedStaticLightActorMap;

	/**
	 * Evalutes the command-line to determine which maps to check.  By default all maps are checked (except PIE and trash-can maps)
	 * Provides child classes with a chance to initialize any variables, parse the command line, etc.
	 *
	 * @param	Tokens			the list of tokens that were passed to the commandlet
	 * @param	Switches		the list of switches that were passed on the commandline
	 * @param	MapPathNames	receives the list of path names for the maps that will be checked.
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT InitializeMapCheck( const TArray<FString>& Tokens, const TArray<FString>& Switches, TArray<FFilename>& MapPathNames );

	/**
	 * The main worker method - performs the commandlets tests on the package.
	 *
	 * @param	MapPackage	the current package to be processed
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT CheckMapPackage( UPackage* MapPackage );


	/**
	 * Called after all packages have been processed - provides commandlets with an opportunity to print out test results or
	 * provide feedback.
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT ProcessResults();
END_COMMANDLET

BEGIN_CHILD_COMMANDLET(FindRenamedPrefabSequences,PerformMapCheck,Editor)
	TLookupMap<FString>	RenamedPrefabSequenceContainers;

	/**
	 * Evalutes the command-line to determine which maps to check.  By default all maps are checked (except PIE and trash-can maps)
	 * Provides child classes with a chance to initialize any variables, parse the command line, etc.
	 *
	 * @param	Tokens			the list of tokens that were passed to the commandlet
	 * @param	Switches		the list of switches that were passed on the commandline
	 * @param	MapPathNames	receives the list of path names for the maps that will be checked.
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT InitializeMapCheck( const TArray<FString>& Tokens, const TArray<FString>& Switches, TArray<FFilename>& MapPathNames );

	/**
	 * The main worker method - performs the commandlets tests on the package.
	 *
	 * @param	MapPackage	the current package to be processed
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT CheckMapPackage( UPackage* MapPackage );


	/**
	 * Called after all packages have been processed - provides commandlets with an opportunity to print out test results or
	 * provide feedback.
	 *
	 * @return	0 to indicate that the commandlet should continue; otherwise, the error code that should be returned by Main()
	 */
	virtual INT ProcessResults();
END_COMMANDLET

BEGIN_COMMANDLET(DumpLightmapInfo,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(PerformTerrainMaterialDump,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(PatchScript,Editor)
	void StaticInitialize();
	class FScriptPatchWorker* Worker;
END_COMMANDLET

BEGIN_COMMANDLET(ListLoopingEmitters,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(SoundGroupInfo,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ListDistanceCrossFadeNodes,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(LocSoundInfo,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ListPSysFixedBoundSetting,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ListEmittersUsingModule,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ReplaceActor,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(BuildContentTagIndex,Editor)
END_COMMANDLET

BEGIN_COMMANDLET(ListSoundNodeWaves,Editor)
END_COMMANDLET

#endif
