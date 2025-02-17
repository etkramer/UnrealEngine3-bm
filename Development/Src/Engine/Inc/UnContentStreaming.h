/*=============================================================================
	UnContentStreaming.h: Definitions of classes used for content streaming.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Stats.
-----------------------------------------------------------------------------*/

/**
 * Streaming stats
 */
enum EStreamingStats
{
	STAT_StreamingTextures = STAT_AsyncLoadingTime + 1,
	STAT_StreamingTexturesSize,
	STAT_StreamingTexturesMaxSize,
	STAT_TexturePoolAllocatedSize,
	STAT_RequestsInCancelationPhase,
	STAT_RequestsInUpdatePhase,
	STAT_RequestsInFinalizePhase,
	STAT_IntermediateTextures,
	STAT_IntermediateTexturesSize,
	STAT_RequestSizeCurrentFrame,
	STAT_RequestSizeTotal,
	STAT_StreamingFudgeFactor,
	STAT_GameThreadUpdateTime,
	STAT_RenderingThreadUpdateTime,
	STAT_RenderingThreadFinalizeTime,
	STAT_AudioResourceCreationTime,
	STAT_VolumeStreamingTickTime,
	STAT_VolumeStreamingChecks,
	STAT_GrowingReallocations,
	STAT_ShrinkingReallocations,
	STAT_FullReallocations,
	STAT_FailedReallocations,
	STAT_PanicDefragmentations,
	STAT_AddToWorldTime,
	STAT_RemoveFromWorldTime,
	STAT_UpdateLevelStreamingTime,
};

/*-----------------------------------------------------------------------------
	Base streaming classes.
-----------------------------------------------------------------------------*/

/**
 * Helper structure containing all relevant information for streaming.
 */
struct FStreamingViewInfo
{
	/** View origin */
	FVector ViewOrigin;
	/** Screen size, not taking FOV into account */
	FLOAT	ScreenSize;
	/** Screen size, taking FOV into account */
	FLOAT	FOVScreenSize;
};

/**
 * Pure irtual base class of a streaming manager.
 */
struct FStreamingManagerBase
{	
	/** Virtual destructor */
	virtual ~FStreamingManagerBase()
	{}

	/**
	 * Updates streaming, taking into account all view infos.
	 *
	 * @param DeltaTime		Time since last call in seconds
	 */
	virtual void UpdateResourceStreaming( FLOAT DeltaTime ) = 0;

	/**
	 * Blocks till all pending requests are fulfilled.
	 */
	virtual void BlockTillAllRequestsFinished() = 0;

	/**
	 * Notifies manager of "level" change.
	 */
	virtual void NotifyLevelChange() = 0;

	/**
	 * Adds the passed in view information to the static array.
	 *
	 * @param ScreenSize		Screen size
	 * @param FOVScreenSize		Screen size taking FOV into account
	 */
	void AddViewInformation( const FVector& ViewOrigin, FLOAT ScreenSize, FLOAT FOVScreenSize );

	/**
	 *	Queue up view "slave" locations to the streaming system.
	 *	These locations will be added properly at the next call to AddViewInformation,
	 *	re-using the screensize and FOV settings.
	 *	@param SlaveLocation	World-space view origin
	 */
	void AddViewSlaveLocation( const FVector& SlaveLocation );

	/** Don't stream world resources for the next NumFrames. */
	virtual void SetDisregardWorldResourcesForFrames( INT NumFrames ) = 0;

	/**
	 *	Try to stream out texture mip-levels to free up more memory.
	 *	@param RequiredMemorySize	- Required minimum available texture memory
	 *	@return						- Whether it succeeded or not
	 **/
	virtual UBOOL StreamOutTextureData( INT RequiredMemorySize )
	{
		return FALSE;
	}

protected:
	/** Collection of views that need to be taken into account for streaming. */
	static TArray<FStreamingViewInfo> ViewInfos;

	/** Collection of view locations that will be added at the next call to AddViewInformation. */
	static TArray<FVector> SlaveLocations;
};

/**
 * Streaming manager collection, routing function calls to streaming managers that have been added
 * via AddStreamingManager.
 */
struct FStreamingManagerCollection : public FStreamingManagerBase
{
	/** Default constructor, initializing all member variables. */
	FStreamingManagerCollection()
	:	NumIterations(1)
	,	DisableResourceStreamingCount(0)
	{}

	/**
	 * Updates streaming, taking into account all view infos and empties ViewInfos array.
	 *
	 * @param DeltaTime		Time since last call in seconds
	 */
	virtual void UpdateResourceStreaming( FLOAT DeltaTime );

	/**
	 * Blocks till all pending requests are fulfilled.
	 */
	virtual void BlockTillAllRequestsFinished();

	/**
	 * Notifies manager of "level" change.
	 */
	virtual void NotifyLevelChange();

	/**
	 * Adds a streaming manager to the array of managers to route function calls to.
	 *
	 * @param StreamingManager	Streaming manager to add
	 */
	void AddStreamingManager( FStreamingManagerBase* StreamingManager );

	/**
	 * Removes a streaming manager from the array of managers to route function calls to.
	 *
	 * @param StreamingManager	Streaming manager to remove
	 */
	void RemoveStreamingManager( FStreamingManagerBase* StreamingManager );

	/**
	 * Sets the number of iterations to use for the next time UpdateResourceStreaming is being called. This 
	 * is being reset to 1 afterwards.
	 *
	 * @param NumIterations	Number of iterations to perform the next time UpdateResourceStreaming is being called.
	 */
	void SetNumIterationsForNextFrame( INT NumIterations );

	/** Don't stream world resources for the next NumFrames. */
	virtual void SetDisregardWorldResourcesForFrames( INT NumFrames );

	/**
	 * Disables resource streaming. Enable with EnableResourceStreaming. Disable/enable can be called multiple times nested
	 */
	void DisableResourceStreaming();

	/**
	 * Enables resource streaming, previously disabled with enableResourceStreaming. Disable/enable can be called multiple times nested
	 * (this will only actually enable when all disables are matched with enables)
	 */
	void EnableResourceStreaming();

	/**
	 *	Try to stream out texture mip-levels to free up more memory.
	 *	@param RequiredMemorySize	- Required minimum available texture memory
	 *	@return						- Whether it succeeded or not
	 **/
	virtual UBOOL StreamOutTextureData( INT RequiredMemorySize );

protected:
	/** Array of streaming managers to route function calls to */
	TArray<FStreamingManagerBase*> StreamingManagers;
	/** Number of iterations to perform. Gets reset to 1 each frame. */
	INT NumIterations;

	/** Count of how many nested DisableResourceStreaming's were called - will enable when this is 0 */
	INT DisableResourceStreamingCount;
};

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

/** Global streaming manager */
extern FStreamingManagerCollection*	GStreamingManager;

/*-----------------------------------------------------------------------------
	Texture streaming.
-----------------------------------------------------------------------------*/

struct FStreamingHandlerTextureBase;

/**
 * Streaming manager dealing with textures.
 */
struct FStreamingManagerTexture : public FStreamingManagerBase
{
	/** Constructor, initializing all members */
	FStreamingManagerTexture();

	/**
	 * Updates streaming, taking into all views infos.
	 *
	 * @param DeltaTime		Time since last call in seconds
	 */
	virtual void UpdateResourceStreaming( FLOAT DeltaTime );

	/**
	 * Blocks till all pending requests are fulfilled.
	 */
	virtual void BlockTillAllRequestsFinished();

	/**
	 * Notifies manager of "level" change so it can prioritize character textures for a few frames.
	 */
	virtual void NotifyLevelChange();

	/**
	 * Adds a textures streaming handler to the array of handlers used to determine which
	 * miplevels need to be streamed in/ out.
	 *
	 * @param TextureStreamingHandler	Handler to add
	 */
	void AddTextureStreamingHandler( FStreamingHandlerTextureBase* TextureStreamingHandler );

	/**
	 * Removes a textures streaming handler from the array of handlers used to determine which
	 * miplevels need to be streamed in/ out.
	 *
	 * @param TextureStreamingHandler	Handler to remove
	 */
	void RemoveTextureStreamingHandler( FStreamingHandlerTextureBase* TextureStreamingHandler );

	/** Don't stream world resources for the next NumFrames. */
	virtual void SetDisregardWorldResourcesForFrames( INT NumFrames );

	/**
	 *	Try to stream out texture mip-levels to free up more memory.
	 *	@param RequiredMemorySize	- Required minimum available texture memory
	 *	@return						- Whether it succeeded or not
	 **/
	virtual UBOOL StreamOutTextureData( INT RequiredMemorySize );

protected:
	/** Array of texture streaming objects to use during update. */
	TArray<FStreamingHandlerTextureBase*> TextureStreamingHandlers;
	/** Remaining ticks to disregard world textures. */
	INT RemainingTicksToDisregardWorldTextures;
	/** Remaining ticks to suspend texture requests for. Used to sync up with async memory allocations. */
	INT	RemainingTicksToSuspendActivity;

	/** Fudge factor used to balance memory allocations/ use. */
	FLOAT FudgeFactor;
	/** Fudge factor rate of change. */
	FLOAT FudgeFactorRateOfChange;
	/** Last time NotifyLevelChange was called. */
	DOUBLE LastLevelChangeTime;
	/** Whether to use MinRequestedMipsToConsider limit. */
	UBOOL bUseMinRequestLimit;
	/** Time of last full iteration over all textures. */
	FLOAT LastFullIterationTime;
	/** Time of current iteration over all textures. */
	FLOAT CurrentFullIterationTime;

	// Config variables.
	/** Limit for change of fudge factor range to avoid hysteresis */
	INT MemoryHysteresisLimit;
	/** Determines when to fiddle with fudge factor to stream out miplevels. */
	INT	MemoryDropMipLevelsLimit;
	/** Determines when to start disallowing texture miplevel increases. */
	INT MemoryStopIncreasingLimit;
	/** Limit when stop issueing new streaming requests altogether */
	INT	MemoryStopStreamingLimit;
	/** Minimum value of fudge factor. A fudge factor of 1 means neutral though with sufficient memory we can prefetch textures. */
	FLOAT MinFudgeFactor;
	/** Rate of change to use when increasing fudge factor */
	FLOAT FudgeFactorIncreaseRateOfChange;
	/** Rate of change to use when decreasing fudge factor */
	FLOAT FudgeFactorDecreaseRateOfChange;
	/** 
	 * Minimum number of requested mips at which texture stream-in request is still going to be considered. 
	 * This is used by the texture priming code to prioritize large request for higher miplevels before smaller
	 * ones for background textures to avoid seeking and texture popping.
	 */
	INT MinRequestedMipsToConsider;
	/**
	 * Minimum number of bytes to evict when we need to stream out textures because of a failed allocation.
	 **/
	INT MinEvictSize;
	/** Minimum amount of time in seconds to guarantee MinRequestedMipsToConsider to be respected. */
	FLOAT MinTimeToGuaranteeMinMipCount;
	/** Maximum amount of time in seconds to guarantee MinRequestedMipsToConsider to be respected. */
	FLOAT MaxTimeToGuaranteeMinMipCount;

	// Stats we need to keep across frames as we only iterate over a subset of textures.

	/** Number of streaming textures */
	DWORD NumStreamingTextures;
	/** Number of requests in cancelation phase. */
	DWORD NumRequestsInCancelationPhase;
	/** Number of requests in mip update phase. */
	DWORD NumRequestsInUpdatePhase;
	/** Number of requests in mip finalization phase. */
	DWORD NumRequestsInFinalizePhase;
	/** Size ot all intermerdiate textures in flight. */
	DWORD TotalIntermediateTexturesSize;
	/** Number of intermediate textures in flight. */
	DWORD NumIntermediateTextures;
	/** Size of all streaming testures. */
	DWORD TotalStreamingTexturesSize;
	/** Maximum size of all streaming textures. */
	DWORD TotalStreamingTexturesMaxSize;
	/** Number of mip count increase requests in flight. */
	DWORD TotalMipCountIncreaseRequestsInFlight;
};

/*-----------------------------------------------------------------------------
	Texture streaming handler.
-----------------------------------------------------------------------------*/

/**
 * Base of texture streaming handler functionality.
 */
struct FStreamingHandlerTextureBase
{
	/**
	 * Returns mip count wanted by this handler for the passed in texture. 
	 * 
	 * @param	Texture			Texture to determine wanted mip count for
	 * @param	ViewInfos		Array of view infos to use for mip count determination
	 * @param	FudgeFactor		Fudge factor hint; [1,inf] with higher values meaning that more mips should be dropped
	 * @param	LODAllowedMips	Max number of mips allowed by LOD code
	 * @return	Number of miplevels that should be streamed in or INDEX_NONE if 
	 *			texture isn't handled by this handler.
	 */
	virtual INT GetWantedMips( UTexture2D* Texture, const TArray<FStreamingViewInfo>& ViewInfos, FLOAT FudgeFactor, INT LODAllowedMips ) = 0;
};

/**
 * Static texture streaming handler. Used to stream textures on static level geometry.
 */
struct FStreamingHandlerTextureStatic : public FStreamingHandlerTextureBase
{
	/**
	 * Returns mip count wanted by this handler for the passed in texture. 
	 * 
	 * @param	Texture		Texture to determine wanted mip count for
	 * @param	ViewInfos	Array of view infos to use for mip count determination
	 * @param	FudgeFactor	Fudge factor hint; [1,inf] with higher values meaning that more mips should be dropped
	 * @param	LODAllowedMips	Max number of mips allowed by LOD code
	 * @return	Number of miplevels that should be streamed in or INDEX_NONE if 
	 *			texture isn't handled by this handler.
	 */
	virtual INT GetWantedMips( UTexture2D* Texture, const TArray<FStreamingViewInfo>& ViewInfos, FLOAT FudgeFactor, INT LODAllowedMips );
};

/**
 * Streaming handler that bases its decision on the last render time.
 */
struct FStreamingHandlerTextureLastRender : public FStreamingHandlerTextureBase
{
	/**
	 * Returns mip count wanted by this handler for the passed in texture. 
	 * 
	 * @param	Texture		Texture to determine wanted mip count for
	 * @param	ViewInfos	Array of view infos to use for mip count determination
	 * @param	FudgeFactor	Fudge factor hint; [1,inf] with higher values meaning that more mips should be dropped
	 * @param	LODAllowedMips	Max number of mips allowed by LOD code
	 * @return	Number of miplevels that should be streamed in or INDEX_NONE if 
	 *			texture isn't handled by this handler.
	 */
	virtual INT GetWantedMips( UTexture2D* Texture, const TArray<FStreamingViewInfo>& ViewInfos, FLOAT FudgeFactor, INT LODAllowedMips );
};

/**
 *	Streaming handler that bases its decision on the bForceMipStreaming flag in PrimitiveComponent.
 */
struct FStreamingHandlerTextureLevelForced : public FStreamingHandlerTextureBase
{
	/**
	 * Returns mip count wanted by this handler for the passed in texture. 
	 * 
	 * @param	Texture		Texture to determine wanted mip count for
	 * @param	ViewInfos	Array of view infos to use for mip count determination
	 * @param	FudgeFactor	Fudge factor hint; [1,inf] with higher values meaning that more mips should be dropped
	 * @param	LODAllowedMips	Max number of mips allowed by LOD code
	 * @return	Number of miplevels that should be streamed in or INDEX_NONE if 
	 *			texture isn't handled by this handler.
	 */
	virtual INT GetWantedMips( UTexture2D* Texture, const TArray<FStreamingViewInfo>& ViewInfos, FLOAT FudgeFactor, INT LODAllowedMips );
};

/*-----------------------------------------------------------------------------
	Texture streaming helper structs.
-----------------------------------------------------------------------------*/

/**
 * Structure containing all information needed for determining the screen space
 * size of an object/ texture instance.
 */
struct FStreamableTextureInstance
{
	/** Bounding sphere/ box of object */
	FSphere BoundingSphere;
	/** Object (and bounding sphere) specfic texel scale factor  */
	FLOAT	TexelFactor;

	/**
	 * FStreamableTextureInstance serialize operator.
	 *
	 * @param	Ar					Archive to to serialize object to/ from
	 * @param	TextureInstance		Object to serialize
	 * @return	Returns the archive passed in
	 */
	friend FArchive& operator<<( FArchive& Ar, FStreamableTextureInstance& TextureInstance );
};

