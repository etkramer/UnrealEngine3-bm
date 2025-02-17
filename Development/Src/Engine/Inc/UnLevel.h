/*=============================================================================
	UnLevel.h: ULevel definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Network notification sink.
-----------------------------------------------------------------------------*/

//
// Accepting connection responses.
//
enum EAcceptConnection
{
	ACCEPTC_Reject,	// Reject the connection.
	ACCEPTC_Accept, // Accept the connection.
	ACCEPTC_Ignore, // Ignore it, sending no reply, while server travelling.
};

/**
 * The net code uses this to send notifications.
 */
class FNetworkNotify
{
public:
	virtual EAcceptConnection NotifyAcceptingConnection() PURE_VIRTUAL(FNetworkNotify::NotifyAcceptedConnection,return ACCEPTC_Ignore;);
	virtual void NotifyAcceptedConnection( class UNetConnection* Connection ) PURE_VIRTUAL(FNetworkNotify::NotifyAcceptedConnection,);
	virtual UBOOL NotifyAcceptingChannel( class UChannel* Channel ) PURE_VIRTUAL(FNetworkNotify::NotifyAcceptingChannel,return FALSE;);
	virtual class UWorld* NotifyGetWorld() PURE_VIRTUAL(FNetworkNotify::NotifyGetWorld,return NULL;);
	virtual void NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text ) PURE_VIRTUAL(FNetworkNotify::NotifyReceivedText,);
	virtual UBOOL NotifySendingFile( UNetConnection* Connection, FGuid GUID ) PURE_VIRTUAL(FNetworkNotify::NotifySendingFile,return FALSE;);
	virtual void NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const TCHAR* Error, UBOOL Skipped ) PURE_VIRTUAL(FNetworkNotify::NotifyReceivedFile,);
	virtual void NotifyProgress( EProgressMessageType MessageType, const FString& Title, const FString& Message ) PURE_VIRTUAL(FNetworkNotify::NotifyProgress,);
};

/**
 * An interface for making spatial queries against the primitives in a level.
 */
class FPrimitiveHashBase
{
public:
	// FPrimitiveHashBase interface.
	virtual ~FPrimitiveHashBase() {};
	virtual void Tick()=0;
	virtual void AddPrimitive( UPrimitiveComponent* Primitive )=0;
	virtual void RemovePrimitive( UPrimitiveComponent* Primitive )=0;
	virtual FCheckResult* ActorLineCheck( FMemStack& Mem, const FVector& End, const FVector& Start, const FVector& Extent, DWORD TraceFlags, AActor *SourceActor, ULightComponent* SourceLight )=0;
	virtual FCheckResult* ActorPointCheck( FMemStack& Mem, const FVector& Location, const FVector& Extent, DWORD TraceFlags )=0;
	/**
	 * Finds all actors that are touched by a sphere (point + radius). If
	 * bUseOverlap is false, only the centers of the bounding boxes are
	 * considered. If true, it does a full sphere/box check.
	 *
	 * @param Mem the mem stack to allocate results from
	 * @param Location the center of the sphere
	 * @param Radius the size of the sphere to check for overlaps with
	 * @param bUseOverlap whether to use the full box or just the center
	 */
	virtual FCheckResult* ActorRadiusCheck( FMemStack& Mem, const FVector& Location, FLOAT Radius, UBOOL bUseOverlap = FALSE )=0;
	virtual FCheckResult* ActorEncroachmentCheck( FMemStack& Mem, AActor* Actor, FVector Location, FRotator Rotation, DWORD TraceFlags )=0;

	/**
	 * Finds all actors that are touched by a sphere (point + radius).
	 *
	 * @param	Mem			The mem stack to allocate results from.
	 * @param	Actor		The actor to ignore overlaps with.
	 * @param	Location	The center of the sphere.
	 * @param	Radius		The size of the sphere to check for overlaps with.
	 */
	virtual FCheckResult* ActorOverlapCheck(FMemStack& Mem, AActor* Actor, const FVector& Location, FLOAT Radius)=0;

	/**
	 * Finds all actors that are touched by a sphere (point + radius).
	 *
	 * @param	Mem			The mem stack to allocate results from.
	 * @param	Actor		The actor to ignore overlaps with.
	 * @param	Location	The center of the sphere.
	 * @param	Radius		The size of the sphere to check for overlaps with.
	 * @param	TraceFlags	Options for the trace.
	 */
	virtual FCheckResult* ActorOverlapCheck(FMemStack& Mem, AActor* Actor, const FVector& Location, FLOAT Radius, DWORD TraceFlags)=0;

	/**
	 * Finds all pawns that are touched by a sphere (point + radius).
	 *
	 * @param	Mem			The mem stack to allocate results from.
	 * @param	Actor		The actor to ignore overlaps with.
	 * @param	Location	The center of the sphere.
	 * @param	Radius		The size of the sphere to check for overlaps with.
	 */
	virtual FCheckResult* PawnOverlapCheck(FMemStack& Mem, AActor* Actor, const FVector& Location, FLOAT Radius)=0;

	virtual void GetIntersectingPrimitives(const FBox& Box,TArray<UPrimitiveComponent*>& Primitives) = 0;
	/**
	 * Retrieves all primitives in hash.
	 *
	 * @param	Primitives [out]	Array primitives are being added to
	 */
	virtual void GetPrimitives(TArray<UPrimitiveComponent*>& Primitives) = 0;

	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar) = 0;
};

//
//	ULineBatchComponent
//
class ULineBatchComponent : public UPrimitiveComponent, public FPrimitiveDrawInterface
{
	DECLARE_CLASS(ULineBatchComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	struct FLine
	{
		FVector			Start,
						End;
		FLinearColor	Color;
		FLOAT			RemainingLifeTime;
		BYTE			DepthPriority;

		FLine(const FVector& InStart,const FVector& InEnd,const FLinearColor& InColor,FLOAT InLifeTime,BYTE InDepthPriority):
			Start(InStart),
			End(InEnd),
			Color(InColor),
			RemainingLifeTime(InLifeTime),
			DepthPriority(InDepthPriority)
		{}
	};

	TArray<FLine>	BatchedLines;
	FLOAT			DefaultLifeTime;

	/** Default constructor. */
	ULineBatchComponent():
		FPrimitiveDrawInterface(NULL),
		DefaultLifeTime(0.f)
	{}

	/**
	 * Draws a line which disappears after a certain amount of time.
	 */
	virtual void DrawLine(const FVector& Start,const FVector& End,FColor Color,FLOAT LifeTime,BYTE InDepthPriority);

	/** Provide many lines to draw - faster than calling DrawLine many times. */
	virtual void DrawLines(const TArray<FLine>& InLines);

	/** Draw a box */
	virtual void DrawBox(const FBox& Box, const FMatrix& TM, const FColor& Color, BYTE DepthPriorityGroup);

	// FPrimitiveDrawInterface
	virtual UBOOL IsHitTesting() { return FALSE; }
	virtual void SetHitProxy(HHitProxy* HitProxy) {}
	virtual void RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource) {}
	virtual INT DrawMesh(const FMeshElement& Mesh) { return 0; }
	// FPrimitiveDrawInterface.
	virtual void DrawSprite(
		const FVector& Position,
		FLOAT SizeX,
		FLOAT SizeY,
		const FTexture* Sprite,
		const FLinearColor& Color,
		BYTE DepthPriority
		) {}

	virtual void DrawLine(
		const FVector& Start,
		const FVector& End,
		const FLinearColor& Color,
		BYTE DepthPriority
		);

	virtual void DrawPoint(
		const FVector& Position,
		const FLinearColor& Color,
		FLOAT PointSize,
		BYTE DepthPriority
		) {}

	/** Allocates a vertex for dynamic mesh rendering. */
	virtual INT AddVertex(
		const FVector4& InPosition,
		const FVector2D& InTextureCoordinate,
		const FLinearColor& InColor,
		BYTE InDepthPriorityGroup
		) { return INDEX_NONE; }

	/** Draws dynamic mesh triangle using vertices specified by AddVertex. */
	virtual void DrawTriangle(
		INT V0,
		INT V1,
		INT V2,
		const FMaterialRenderProxy* Material,
		BYTE InDepthPriorityGroup
		) {}

	// UPrimitiveComponent interface.

	/**
	 * Creates a new scene proxy for the line batcher component.
	 * @return	Pointer to the FLineBatcherSceneProxy
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void Tick(FLOAT DeltaTime);
};

//
// MoveActor options.
//
enum EMoveFlags
{
	// Bitflags.
	MOVE_IgnoreBases		= 0x00001, // ignore collisions with things the Actor is based on
	MOVE_NoFail				= 0x00002, // ignore conditions that would normally cause MoveActor() to abort (such as encroachment)
	MOVE_TraceHitMaterial	= 0x00004, // figure out material was hit for any collisions
};

//
// Trace options.
//
enum ETraceFlags
{
	// Bitflags.
	TRACE_Pawns					= 0x00001, // Check collision with pawns.
	TRACE_Movers				= 0x00002, // Check collision with movers.
	TRACE_Level					= 0x00004, // Check collision with BSP level geometry.
	TRACE_Volumes				= 0x00008, // Check collision with soft volume boundaries.
	TRACE_Others				= 0x00010, // Check collision with all other kinds of actors.
	TRACE_OnlyProjActor			= 0x00020, // Check collision with other actors only if they are projectile targets
	TRACE_Blocking				= 0x00040, // Check collision with other actors only if they block the check actor
	TRACE_LevelGeometry			= 0x00080, // Check collision with other actors which are static level geometry
	TRACE_ShadowCast			= 0x00100, // Check collision with shadow casting actors
	TRACE_StopAtAnyHit			= 0x00200, // Stop when find any collision (for visibility checks)
	TRACE_SingleResult			= 0x00400, // Stop when find guaranteed first nearest collision (for SingleLineCheck)
	TRACE_Material				= 0x00800, // Request that Hit.Material return the material the trace hit.
	TRACE_Visible				= 0x01000,
	TRACE_Terrain				= 0x02000, // Check collision with terrain
	TRACE_Tesselation			= 0x04000, // Check collision against highest tessellation level (not valid for box checks)  (no longer used)
	TRACE_PhysicsVolumes		= 0x08000, // Check collision with physics volumes
	TRACE_TerrainIgnoreHoles	= 0x10000, // Ignore terrain holes when checking collision
	TRACE_ComplexCollision		= 0x20000, // Ignore simple collision on static meshes and always do per poly
	TRACE_AllComponents			= 0x40000, // Don't discard collision results of actors that have already been tagged.  Currently adhered to only by ActorOverlapCheck.

	// Combinations.
	TRACE_Hash					= TRACE_Pawns	|	TRACE_Movers |	TRACE_Volumes	|	TRACE_Others			|	TRACE_Terrain	|	TRACE_LevelGeometry,
	TRACE_Actors				= TRACE_Pawns	|	TRACE_Movers |	TRACE_Others	|	TRACE_LevelGeometry		|	TRACE_Terrain,
	TRACE_World					= TRACE_Level	|	TRACE_Movers |	TRACE_Terrain	|	TRACE_LevelGeometry,
	TRACE_AllColliding			= TRACE_Level	|	TRACE_Actors |	TRACE_Volumes,
	TRACE_ProjTargets			= TRACE_AllColliding	| TRACE_OnlyProjActor,
	TRACE_AllBlocking			= TRACE_Blocking		| TRACE_AllColliding,
};

//
//	ELevelViewportType
//

enum ELevelViewportType
{
	LVT_None = -1,
	LVT_OrthoXY = 0,
	LVT_OrthoXZ = 1,
	LVT_OrthoYZ = 2,
	LVT_Perspective = 3
};

struct FLevelViewportInfo
{
	FVector CamPosition;
	FRotator CamRotation;
	FLOAT CamOrthoZoom;

	FLevelViewportInfo() {}

	FLevelViewportInfo(const FVector& InCamPosition, const FRotator& InCamRotation, FLOAT InCamOrthoZoom)
	{
		CamPosition = InCamPosition;
		CamRotation = InCamRotation;
		CamOrthoZoom = InCamOrthoZoom;
	}

	friend FArchive& operator<<( FArchive& Ar, FLevelViewportInfo& I )
	{
		return Ar << I.CamPosition << I.CamRotation << I.CamOrthoZoom;
	}
};

/*-----------------------------------------------------------------------------
	ULevel base.
-----------------------------------------------------------------------------*/

//
// A game level.
//
class ULevelBase : public UObject
{
	DECLARE_ABSTRACT_CLASS(ULevelBase,UObject,CLASS_Intrinsic,Engine)

	/** Array of actors in this level, used by FActorIteratorBase and derived classes */
	TTransArray<AActor*>	Actors;
	/** URL associated with this level. */
	FURL					URL;

	// Constructors.
	ULevelBase( const FURL& InURL );

	/**
	 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
	 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
	 * properties for native- only classes.
	 */
	void StaticConstructor();

	void Serialize( FArchive& Ar );

protected:
	ULevelBase()
	:	Actors( this )
	{}
};

/*-----------------------------------------------------------------------------
	ULevel class.
-----------------------------------------------------------------------------*/

/** 
 *	Used for storing cached data for simplified static mesh collision at a particular scaling.
 *	One entry, relating to a particular static mesh cached at a particular scaling, giving an index into the CachedPhysSMDataStore. 
 */
struct FCachedPhysSMData
{
	/** Scale of mesh that the data was cached for. */
	FVector				Scale3D;

	/** Index into CachedPhysSMDataStore that this cached data is stored at. */
	INT					CachedDataIndex;

	/** Serializer function. */
	friend FArchive& operator<<( FArchive& Ar, FCachedPhysSMData& D )
	{
		Ar << D.Scale3D << D.CachedDataIndex;
		return Ar;
	}
};

/** 
 *	Used for storing cached data for per-tri static mesh collision at a particular scaling. 
 */
struct FCachedPerTriPhysSMData
{
	/** Scale of mesh that the data was cached for. */
	FVector			Scale3D;

	/** Index into array Cached data for this mesh at this scale. */
	INT				CachedDataIndex;

	/** Serializer function. */
	friend FArchive& operator<<( FArchive& Ar, FCachedPerTriPhysSMData& D )
	{
		Ar << D.Scale3D << D.CachedDataIndex;
		return Ar;
	}
};

//
// The level object.  Contains the level's actor list, Bsp information, and brush list.
//
class ULevel : public ULevelBase
{
	DECLARE_CLASS(ULevel,ULevelBase,CLASS_Intrinsic,Engine)
	NO_DEFAULT_CONSTRUCTOR(ULevel)

	/** BSP UModel.																									*/
	UModel*										Model;
	/** BSP Model components used for rendering.																	*/
	TArray<UModelComponent*>					ModelComponents;
	/** The level's Kismet sequences */
	class TArray<USequence*>					GameSequences;

	/** Static information used by texture streaming code, generated during PreSave									*/
	TMap<UTexture2D*,TArray<FStreamableTextureInstance> >	TextureToInstancesMap;

	/** Set of textures used by PrimitiveComponents that have bForceMipStreaming checked. */
	TMap<UTexture2D*,UBOOL>									ForceStreamTextures;

	/** Index into Actors array pointing to first net relevant actor. Used as an optimization for FActorIterator	*/
	INT											iFirstNetRelevantActor;
	/** Index into Actors array pointing to first dynamic actor. Used as an optimization for FActorIterator			*/
	INT											iFirstDynamicActor;

#if WITH_NOVODEX
	/** Physics scene index. */
	INT											SceneIndex;

	/** Novodex shape for the level BSP. */
	class NxTriangleMesh*						LevelBSPPhysMesh;
   
   	/** Novodex representation of the level BSP geometry (triangle mesh). */
	class NxActor*								LevelBSPActor;
#endif
	
	/** Cached BSP triangle-mesh data for use with the physics engine. */
	TArray<BYTE>								CachedPhysBSPData;

	/** 
	 *	Indicates version that CachedPhysBSPData was created at. 
	 *	Compared against CurrentCachedPhysDataVersion.
	 */
	INT											CachedPhysBSPDataVersion;

	/** 
	 *	Mapping between a particular static mesh used in the level and indices of pre-cooked physics data for it at various scales. 
	 *	Index in FCachedPhysSMData refers to object in the CachedPhysSMDataStore.
	 */
	TMultiMap<UStaticMesh*, FCachedPhysSMData>	CachedPhysSMDataMap;

	/** 
	 *	Indicates version that CachedPhysSMDataMap was created at. 
	 *	Compared against CurrentCachedPhysDataVersion.
	 */
	INT											CachedPhysSMDataVersion;

	/** 
	 *	Store of processsed physics data for static meshes at different scales. 
	 *	Use CachedPhysSMDataMap to find index into this array.
	 */
	TArray<FKCachedConvexData>					CachedPhysSMDataStore;

	/** 
	 *	Store of physics cooked mesh data for per-triangle static mesh collision.
	 *	Index in FCachedPerTriPhysSMData refers to object in the CachedPhysPerTriSMDataStore.
	 */
	TMultiMap<UStaticMesh*, FCachedPerTriPhysSMData>	CachedPhysPerTriSMDataMap;

	/** 
	 *	Store of processsed physics data for static meshes per-tri collision at different scales. 
	 *	Use CachedPhysPerTriSMDataMap to find index into this array.
	 */
	TArray<FKCachedPerTriData>					CachedPhysPerTriSMDataStore;


	/**
	 * Start and end of the navigation list for this level, used for quickly fixing up
	 * when streaming this level in/out.
	 */
	class ANavigationPoint						*NavListStart, *NavListEnd;
	class ACoverLink							*CoverListStart, *CoverListEnd;

	/**
	 * List of actors that have cross level actor references that need to be fixed up when
	 * streaming a level in/out.
	 */
	TArray<class AActor*>						CrossLevelActors;

	/** Whether components are currently attached or not. */
	UBOOL										bAreComponentsCurrentlyAttached;

	/** The below variables are used temporarily while making a level visible.				*/

	/** Whether the level is currently pending being made visible.							*/
	UBOOL										bHasVisibilityRequestPending;
	/** Whether we already moved actors.													*/
	UBOOL										bAlreadyMovedActors;
	/** Whether we already updated components.												*/
	UBOOL										bAlreadyUpdatedComponents;
	/** Whether we already associated streamable resources.									*/
	UBOOL										bAlreadyAssociatedStreamableResources;
	/** Whether we already initialized actors.												*/
	UBOOL										bAlreadyInitializedActors;
	/** Whether we already routed beginplay on actors.										*/
	UBOOL										bAlreadyRoutedActorBeginPlay;
	/** Whether we already fixed up cross-level paths.										*/
	UBOOL										bAlreadyFixedUpCrossLevelRefs;
	/** Whether we already routed sequence begin play.										*/
	UBOOL										bAlreadyRoutedSequenceBeginPlay;
	/** Whether we already sorted the actor list.											*/
	UBOOL										bAlreadySortedActorList;
	/** Current index into actors array for updating components.							*/
	INT											CurrentActorIndexForUpdateComponents;
	/** Whether we already created the physics engine version of the levelBSP.				*/
	UBOOL										bAlreadyCreateBSPPhysMesh;
	/** Whether we have already called InitActorRBPhys on all Actors in the level.			*/
	UBOOL										bAlreadyInitializedAllActorRBPhys;
	/** Current index into actors array for initializing Actor rigid-body physics.			*/
	INT											CurrentActorIndexForInitActorsRBPhys;
	

	// Constructor.
	ULevel( const FURL& InURL );

	/**
	 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
	 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
	 * properties for native- only classes.
	 */
	void StaticConstructor();

	// UObject interface.
	void Serialize( FArchive& Ar );
	void FinishDestroy();

	/** Called before an Undo action occurs */
	virtual void PreEditUndo();

	/** Called after an Undo action occurs */
	virtual void PostEditUndo();

	/**
	 * Callback used to allow object register its direct object references that are not already covered by
	 * the token stream.
	 *
	 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
	 */
	void AddReferencedObjects( TArray<UObject*>& ObjectArray );

	/**
	 * Removes existing line batch components from actors and associates streaming data with level.
	 */
	void PostLoad();

	/**
	 * Clears all components of actors associated with this level (aka in Actors array) and 
	 * also the BSP model components.
	 */
	void ClearComponents();

	/**
	 * Updates all components of actors associated with this level (aka in Actors array) and 
	 * creates the BSP model components.
	 */
	void UpdateComponents();

	/**
	 * Incrementally updates all components of actors associated with this level.
	 *
	 * @param NumComponentsToUpdate	Number of components to update in this run, 0 for all
	 */
	void IncrementalUpdateComponents( INT NumComponentsToUpdate );

	/**
	 * Invalidates the cached data used to render the level's UModel.
	 */
	void InvalidateModelGeometry();

	/**
	 * Updates the model components associated with this level
	 */
	void UpdateModelComponents();

	/**
	 * Commits changes made to the UModel's surfaces.
	 */
	void CommitModelSurfaces();

	/**
	 * Discards the cached data used to render the level's UModel.  Assumes that the
	 * faces and vertex positions haven't changed, only the applied materials.
	 */
	void InvalidateModelSurface();

	/**
	 * Makes sure that all light components have valid GUIDs associated.
	 */
	void ValidateLightGUIDs();

	/**
	 * Sorts the actor list by net relevancy and static behaviour. First all net relevant static
	 * actors, then all remaining static actors and then the rest. This is done to allow the dynamic
	 * and net relevant actor iterators to skip large amounts of actors.
	 */
	void SortActorList();

	/**
	 * Initializes all actors after loading completed.
	 *
	 * @param bForDynamicActorsOnly If TRUE, this function will only act on non static actors
	 */
	void InitializeActors(UBOOL bForDynamicActorsOnly=FALSE);

	/**
	 * Routes pre and post begin play to actors and also sets volumes.
	 *
	 * @param bForDynamicActorsOnly If TRUE, this function will only act on non static actors
	 */
	void RouteBeginPlay(UBOOL bForDynamicActorsOnly=FALSE);

	/**
	 * Presave function, gets called once before the level gets serialized (multiple times) for saving.
	 * Used to rebuild streaming data on save.
	 */
	void PreSave();	

	/**		
	 * Rebuilds static streaming data.	
	 *
	 *	@param	TargetTexture	The texture to build the streaming data for
	 *							If NULL, build for all textures (PreSave).
	 */
	void BuildStreamingData(UTexture2D* TargetTexture = NULL);

	/**
	 *	Retrieves the array of streamable texture isntances.
	 *
	 */
	TArray<FStreamableTextureInstance>* GetStreamableTextureInstances(UTexture2D*& TargetTexture);

	/**
	 * Returns the default brush for this level.
	 *
	 * @return		The default brush for this level.
	 */
	ABrush* GetBrush() const;

	/**
	 * Returns the world info for this level.
	 *
	 * @return		The AWorldInfo for this level.
	 */
	AWorldInfo* GetWorldInfo() const;

	/** 
	 * Associate teleporters with the portal volume they are in
	 */
	void AssociatePortals( void );

	/**
	 * Returns the sequence located at the index specified.
	 * @return	a pointer to the USequence object located at the specified element of the GameSequences array.  Returns
	 *			NULL if the index is not a valid index for the GameSequences array.
	 */
	USequence* GetGameSequence() const;

	/** Create physics engine representation of the level BSP. */
	void InitLevelBSPPhysMesh();

	/**
	 *	Iterates over all actors calling InitRBPhys on them.
	 */
	void IncrementalInitActorsRBPhys(INT NumActorsToInit);

	/**
	 *	Terminate physics for this level. Destroy level BSP physics, and iterates over all actors
	 *	calling TermRBPhys on them.
	 */
	void TermLevelRBPhys(FRBPhysScene* Scene);

	/** Reset stats used for seeing where time goes initialising physics. */
	void ResetInitRBPhysStats();

	/** Output stats for initialising physics. */
	void OutputInitRBPhysStats();

	/** Build cached BSP triangle-mesh data for physics engine. */
	void BuildPhysBSPData();

	/** Create the cache of cooked collision data for static meshes used in this level. */
	void BuildPhysStaticMeshCache();

	/**  Clear the static mesh cooked collision data cache. */
	void ClearPhysStaticMeshCache();

	/** 
	 *	Utility for finding if we have cached data for a paricular static mesh at a particular scale.
	 *	Returns NULL if there is no cached data.
	 */
	FKCachedConvexData* FindPhysStaticMeshCachedData(UStaticMesh* InMesh, const FVector& InScale3D);

	/** 
	 *	Utility for finding if we have cached per-triangle data for a paricular static mesh at a particular scale.
	 *	Returns NULL if there is no cached data.
	 */
	FKCachedPerTriData* FindPhysPerTriStaticMeshCachedData(UStaticMesh* InMesh, const FVector& InScale3D);

	/**
	 * Utility searches this level's actor list for any actors of the specified type.
	 */
	UBOOL HasAnyActorsOfType(UClass *SearchType);

	/**
	 * Returns TRUE if this level has any path nodes.
	 */
	UBOOL HasPathNodes();

	/**
	 * Adds a pathnode to the navigation list.
	 */
	void AddToNavList( class ANavigationPoint *Nav, UBOOL bDebugNavList = FALSE );

	/**
	 * Removes a pathnode from the navigation list.
	 */
	void RemoveFromNavList( class ANavigationPoint *Nav, UBOOL bDebugNavList = FALSE );

	/**
	 * Resets the level nav list.
	 */
	void ResetNavList();
};

