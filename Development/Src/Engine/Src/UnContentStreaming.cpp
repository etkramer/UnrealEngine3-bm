/*=============================================================================
	UnContentStreaming.cpp: Implementation of content streaming classes.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Stats.
-----------------------------------------------------------------------------*/

/** Streaming stats */
DECLARE_STATS_GROUP(TEXT("Streaming"),STATGROUP_Streaming);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Streaming Textures"),STAT_StreamingTextures,STATGROUP_Streaming);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Intermediate Textures"),STAT_IntermediateTextures,STATGROUP_Streaming);

DECLARE_MEMORY_STAT2(TEXT("Streaming Textures Size"),STAT_StreamingTexturesSize,STATGROUP_Streaming,MCR_TexturePool1,TRUE);
DECLARE_MEMORY_STAT2(TEXT("Textures Max Size"),STAT_StreamingTexturesMaxSize,STATGROUP_Streaming,MCR_TexturePool1,TRUE);
DECLARE_MEMORY_STAT2(TEXT("Intermediate Textures Size"),STAT_IntermediateTexturesSize,STATGROUP_Streaming,MCR_TexturePool1,TRUE);

//DECLARE_MEMORY_STAT2(TEXT("Texture Pool Size"),STAT_TexturePoolAllocatedSize,STATGROUP_Streaming,MCR_TexturePool1,TRUE);
// texture pool stat in the memory groupthat is hidden if nothing is allocated in it
DECLARE_MEMORY_STAT2(TEXT("Texture Pool Size"), STAT_TexturePoolAllocatedSize, STATGROUP_Memory, MCR_TexturePool1, FALSE);

DECLARE_MEMORY_STAT2(TEXT("Request Size Current Frame"),STAT_RequestSizeCurrentFrame,STATGROUP_Streaming,MCR_TexturePool1,TRUE);
DECLARE_MEMORY_STAT2(TEXT("Request Size Total"),STAT_RequestSizeTotal,STATGROUP_Streaming,MCR_TexturePool1,TRUE);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Requests In Cancelation Phase"),STAT_RequestsInCancelationPhase,STATGROUP_Streaming);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Requests In Update Phase"),STAT_RequestsInUpdatePhase,STATGROUP_Streaming);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Requests in Finalize Phase"),STAT_RequestsInFinalizePhase,STATGROUP_Streaming);

DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Streaming fudge factor"),STAT_StreamingFudgeFactor,STATGROUP_Streaming);

DECLARE_CYCLE_STAT(TEXT("Game Thread Update Time"),STAT_GameThreadUpdateTime,STATGROUP_Streaming);
DECLARE_CYCLE_STAT(TEXT("Rendering Thread Update Time"),STAT_RenderingThreadUpdateTime,STATGROUP_Streaming);
DECLARE_CYCLE_STAT(TEXT("Rendering Thread Finalize Time"),STAT_RenderingThreadFinalizeTime,STATGROUP_Streaming)

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Growing reallocations"),STAT_GrowingReallocations,STATGROUP_Streaming);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Shrinking reallocations"),STAT_ShrinkingReallocations,STATGROUP_Streaming);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Full reallocations"),STAT_FullReallocations,STATGROUP_Streaming);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Failed reallocations"),STAT_FailedReallocations,STATGROUP_Streaming);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Panic defragmentations"),STAT_PanicDefragmentations,STATGROUP_Streaming);

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

/** Global streaming manager */
FStreamingManagerCollection* GStreamingManager;

/** Collection of views that need to be taken into account for streaming. */
TArray<FStreamingViewInfo> FStreamingManagerBase::ViewInfos;

/** Collection of view locations that will be added at the next call to AddViewInformation. */
TArray<FVector> FStreamingManagerBase::SlaveLocations;

/**
 * Helper function to flush resource streaming from within Core project.
 */
void FlushResourceStreaming()
{
	RETURN_IF_EXIT_REQUESTED;
	GStreamingManager->BlockTillAllRequestsFinished();
}

/*-----------------------------------------------------------------------------
	FStreamingManagerBase implementation.
-----------------------------------------------------------------------------*/

/**
 * Adds the passed in view information to the static array.
 *
 * @param ViewOrigin		View origin
 * @param ScreenSize		Screen size
 * @param FOVScreenSize		Screen size taking FOV into account
 */
void FStreamingManagerBase::AddViewInformation( const FVector& ViewOrigin, FLOAT ScreenSize, FLOAT FOVScreenSize )
{
	FStreamingViewInfo ViewInfo;
	ViewInfo.ViewOrigin = ViewOrigin;
	ViewInfo.ScreenSize = ScreenSize;
	if (GEngine->IsSplitScreen())
	{
		ViewInfo.ScreenSize *= 0.75f;
	}
	ViewInfo.FOVScreenSize	= FOVScreenSize;
	ViewInfos.AddItem( ViewInfo );

	// Add slave locations if we have any.
	for ( INT SlaveLocationIndex=0; SlaveLocationIndex < SlaveLocations.Num(); SlaveLocationIndex++ )
	{
		ViewInfo.ViewOrigin = SlaveLocations(SlaveLocationIndex);
		ViewInfos.AddItem( ViewInfo );
	}
	SlaveLocations.Empty();
}

/**
 *	Queue up view "slave" locations to the streaming system.
 *	These locations will be added properly at the next call to AddViewInformation,
 *	re-using the screensize and FOV settings.
 *	@param SlaveLocation	World-space view origin
 */
void FStreamingManagerBase::AddViewSlaveLocation( const FVector& SlaveLocation )
{
	SlaveLocations.AddItem( SlaveLocation );
}

/*-----------------------------------------------------------------------------
	FStreamingManagerCollection implementation.
-----------------------------------------------------------------------------*/

/**
 * Sets the number of iterations to use for the next time UpdateResourceStreaming is being called. This 
 * is being reset to 1 afterwards.
 *
 * @param InNumIterations	Number of iterations to perform the next time UpdateResourceStreaming is being called.
 */
void FStreamingManagerCollection::SetNumIterationsForNextFrame( INT InNumIterations )
{
	NumIterations = InNumIterations;
}

/**
 * Updates streaming, taking into account all view infos and empties ViewInfos array.
 *
 * @param DeltaTime		Time since last call in seconds
 */
void FStreamingManagerCollection::UpdateResourceStreaming( FLOAT DeltaTime )
{
	// only allow this if its not disabled
	if (DisableResourceStreamingCount == 0)
	{
		for( INT Iteration=0; Iteration<NumIterations; Iteration++ )
		{
			// Flush rendering commands in the case of multiple iterations to sync up resource streaming
			// with the GPU/ rendering thread.
			if( Iteration > 0 )
			{
				FlushRenderingCommands();
			}

			// Route to streaming managers.
			for( INT ManagerIndex=0; ManagerIndex<StreamingManagers.Num(); ManagerIndex++ )
			{
				FStreamingManagerBase* StreamingManager = StreamingManagers(ManagerIndex);
				StreamingManager->UpdateResourceStreaming( DeltaTime );
			}
		}

		// Reset number of iterations to 1 for next frame.
		NumIterations = 1;
	}

	// Empty view info and slave locaton arrays to be populated again next frame.
	ViewInfos.Empty();
	SlaveLocations.Empty();
}

/**
 * Blocks till all pending requests are fulfilled.
 */
void FStreamingManagerCollection::BlockTillAllRequestsFinished()
{
	// Route to streaming managers.
	for( INT ManagerIndex=0; ManagerIndex<StreamingManagers.Num(); ManagerIndex++ )
	{
		FStreamingManagerBase* StreamingManager = StreamingManagers(ManagerIndex);
		StreamingManager->BlockTillAllRequestsFinished();
	}
}

/**
 * Notifies managers of "level" change.
 */
void FStreamingManagerCollection::NotifyLevelChange()
{
	// Route to streaming managers.
	for( INT ManagerIndex=0; ManagerIndex<StreamingManagers.Num(); ManagerIndex++ )
	{
		FStreamingManagerBase* StreamingManager = StreamingManagers(ManagerIndex);
		StreamingManager->NotifyLevelChange();
	}
}

/** Don't stream world resources for the next NumFrames. */
void FStreamingManagerCollection::SetDisregardWorldResourcesForFrames(INT NumFrames )
{
	// Route to streaming managers.
	for( INT ManagerIndex=0; ManagerIndex<StreamingManagers.Num(); ManagerIndex++ )
	{
		FStreamingManagerBase* StreamingManager = StreamingManagers(ManagerIndex);
		StreamingManager->SetDisregardWorldResourcesForFrames(NumFrames);
	}
}

/**
 * Adds a streaming manager to the array of managers to route function calls to.
 *
 * @param StreamingManager	Streaming manager to add
 */
void FStreamingManagerCollection::AddStreamingManager( FStreamingManagerBase* StreamingManager )
{
	StreamingManagers.AddItem( StreamingManager );
}

/**
 * Removes a streaming manager from the array of managers to route function calls to.
 *
 * @param StreamingManager	Streaming manager to remove
 */
void FStreamingManagerCollection::RemoveStreamingManager( FStreamingManagerBase* StreamingManager )
{
	StreamingManagers.RemoveItem( StreamingManager );
}

/**
 * Disables resource streaming. Enable with EnableResourceStreaming. Disable/enable can be called multiple times nested
 */
void FStreamingManagerCollection::DisableResourceStreaming()
{
	// push on a disable
	appInterlockedIncrement(&DisableResourceStreamingCount);
}

/**
 * Enables resource streaming, previously disabled with enableResourceStreaming. Disable/enable can be called multiple times nested
 * (this will only actually enable when all disables are matched with enables)
 */
void FStreamingManagerCollection::EnableResourceStreaming()
{
	// pop off a disable
	appInterlockedDecrement(&DisableResourceStreamingCount);

	checkf(DisableResourceStreamingCount >= 0, TEXT("Mismatched number of calls to FStreamingManagerCollection::DisableResourceStreaming/EnableResourceStreaming"));
}

/**
 *	Try to stream out texture mip-levels to free up more memory.
 *	@param RequiredMemorySize	- Required minimum available texture memory
 *	@return						- Whether it succeeded or not
 **/
UBOOL FStreamingManagerCollection::StreamOutTextureData( INT RequiredMemorySize )
{
	// Route to streaming managers.
	for( INT ManagerIndex=0; ManagerIndex<StreamingManagers.Num(); ManagerIndex++ )
	{
		FStreamingManagerBase* StreamingManager = StreamingManagers(ManagerIndex);
		if ( StreamingManager->StreamOutTextureData( RequiredMemorySize ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}


/*-----------------------------------------------------------------------------
	FStreamingManagerTexture implementation.
-----------------------------------------------------------------------------*/

/** Constructor, initializing all members and  */
FStreamingManagerTexture::FStreamingManagerTexture()
:	RemainingTicksToDisregardWorldTextures(0)
,	RemainingTicksToSuspendActivity(0)
,	FudgeFactor(1.f)
,	FudgeFactorRateOfChange(0.f)
,	LastLevelChangeTime(0.0)
,	bUseMinRequestLimit(FALSE)
,	LastFullIterationTime(0)
,	CurrentFullIterationTime(0)
,	MemoryHysteresisLimit(0)
,	MemoryDropMipLevelsLimit(0)
,	MemoryStopIncreasingLimit(0)
,	MemoryStopStreamingLimit(0)
,	FudgeFactorIncreaseRateOfChange(0)
,	FudgeFactorDecreaseRateOfChange(0)
,	MinRequestedMipsToConsider(0)
,	MinEvictSize(0)
,	MinTimeToGuaranteeMinMipCount(0)
,	MaxTimeToGuaranteeMinMipCount(0)
,	NumStreamingTextures(0)
,	NumRequestsInCancelationPhase(0)
,	NumRequestsInUpdatePhase(0)
,	NumRequestsInFinalizePhase(0)
,	TotalIntermediateTexturesSize(0)
,	NumIntermediateTextures(0)
,	TotalStreamingTexturesSize(0)
,	TotalStreamingTexturesMaxSize(0)
,	TotalMipCountIncreaseRequestsInFlight(0)
{
	// Read settings from ini file.
	verify( GConfig->GetInt( TEXT("TextureStreaming"), TEXT("HysteresisLimit"),				MemoryHysteresisLimit,				GEngineIni ) );
	verify( GConfig->GetInt( TEXT("TextureStreaming"), TEXT("DropMipLevelsLimit"),			MemoryDropMipLevelsLimit,			GEngineIni ) );
	verify( GConfig->GetInt( TEXT("TextureStreaming"), TEXT("StopIncreasingLimit"),			MemoryStopIncreasingLimit,			GEngineIni ) );
	verify( GConfig->GetInt( TEXT("TextureStreaming"), TEXT("StopStreamingLimit"),			MemoryStopStreamingLimit,			GEngineIni ) );
	verify( GConfig->GetInt( TEXT("TextureStreaming"), TEXT("MinRequestedMipsToConsider"),	MinRequestedMipsToConsider,			GEngineIni ) );
	verify( GConfig->GetInt( TEXT("TextureStreaming"), TEXT("MinEvictSize"),				MinEvictSize,						GEngineIni ) );

	verify( GConfig->GetFloat( TEXT("TextureStreaming"), TEXT("MinTimeToGuaranteeMinMipCount"),		MinTimeToGuaranteeMinMipCount,	 GEngineIni ) );
	verify( GConfig->GetFloat( TEXT("TextureStreaming"), TEXT("MaxTimeToGuaranteeMinMipCount"),		MaxTimeToGuaranteeMinMipCount,	 GEngineIni ) );

	verify( GConfig->GetFloat( TEXT("TextureStreaming"), TEXT("MinFudgeFactor"),					MinFudgeFactor, GEngineIni ) );
	verify( GConfig->GetFloat( TEXT("TextureStreaming"), TEXT("FudgeFactorIncreaseRateOfChange"),	FudgeFactorIncreaseRateOfChange, GEngineIni ) );
	verify( GConfig->GetFloat( TEXT("TextureStreaming"), TEXT("FudgeFactorDecreaseRateOfChange"),	FudgeFactorDecreaseRateOfChange, GEngineIni ) );

	// Convert from MByte to byte.
	MemoryHysteresisLimit		*= 1024 * 1024;
	MemoryDropMipLevelsLimit	*= 1024 * 1024;
	MemoryStopIncreasingLimit	*= 1024 * 1024;
	MemoryStopStreamingLimit	*= 1024 * 1024;
	MinEvictSize				*= 1024 * 1024;
}

/** Number of ticks for full iteration over texture list. */
#define NUM_TICKS_FOR_FULL_ITERATION 10

/** Maximum number of bytes to change per frame */
#define MAX_PER_FRAME_REQUEST_LIMIT (3 * 1024 * 1024)

/**
 * Notifies manager of "level" change so it can prioritize character textures for a few frames.
 */
void FStreamingManagerTexture::NotifyLevelChange()
{
	// Disregard world textures for one iteration to prioritize other requests.
	RemainingTicksToDisregardWorldTextures = NUM_TICKS_FOR_FULL_ITERATION;
	// Keep track of last time this function was called.
	LastLevelChangeTime = appSeconds();
	// Prioritize higher miplevels to avoid texture popping on initial level load.
	bUseMinRequestLimit = TRUE;
}

/** Don't stream world resources for the next NumFrames. */
void FStreamingManagerTexture::SetDisregardWorldResourcesForFrames(INT NumFrames )
{
	RemainingTicksToDisregardWorldTextures = NumFrames;
}

/**
 *	Helper struct that represents a texture and the parameters used for sorting and streaming out high-res mip-levels.
 **/
struct FTextureSortElement
{
	/**
	 *	Constructor.
	 *
	 *	@param InTexture					- The texture to represent
	 *	@param InSize						- Size of the whole texture and all current mip-levels, in bytes
	 *	@param bInIsCharacterTexture		- 1 if this is a character texture, otherwise 0
	 *	@param InTextureDataAddress			- Starting address of the texture data
	 *	@param InNumRequiredResidentMips	- Minimum number of mip-levels required to stay in memory
	 */
	FTextureSortElement( UTexture2D* InTexture, INT InSize, INT bInIsCharacterTexture, DWORD InTextureDataAddress, INT InNumRequiredResidentMips )
	:	Texture( InTexture )
	,	Size( InSize )
	,	bIsCharacterTexture( bInIsCharacterTexture )
	,	TextureDataAddress( InTextureDataAddress )
	,	NumRequiredResidentMips( InNumRequiredResidentMips )
	{
	}
	/** The texture that this element represents */
	UTexture2D*	Texture;
	/** Size of the whole texture and all current mip-levels, in bytes. */
	INT			Size;
	/** 1 if this is a character texture, otherwise 0 */
	INT			bIsCharacterTexture;
	/** Starting address of the texture data. */
	DWORD		TextureDataAddress;			
	/** Minimum number of mip-levels required to stay in memory. */
	INT			NumRequiredResidentMips;
};

/**
 *	Helper struct to compare two FTextureSortElement objects.
 **/
struct FTextureStreamingCompare
{
	/** 
	 *	Called by Sort<>() to compare two elements.
	 *	@param Texture1		- First object to compare
	 *	@param Texture2		- Second object to compare
	 *	@return				- Negative value if Texture1 should be sorted earlier in the array than Texture2, zero if arbitrary order, positive if opposite order.
	 */
	static INT Compare( const FTextureSortElement& Texture1, const FTextureSortElement& Texture2 )
	{
		// Character textures get lower priority (sorted later in the array).
		if ( Texture1.bIsCharacterTexture != Texture2.bIsCharacterTexture )
		{
			return Texture1.bIsCharacterTexture - Texture2.bIsCharacterTexture;
		}

		// Larger textures get higher priority (sorted earlier in the array).
		if ( Texture2.Size - Texture1.Size )
		{
			return Texture2.Size - Texture1.Size;
		}

		// Then sort on baseaddress, so that textures at the end of the texture pool gets higher priority (sorted earlier in the array).
		// (It's faster to defrag the texture pool when the holes are at the end.)
		return INT(Texture2.TextureDataAddress - Texture1.TextureDataAddress);
	}
};

/**
 *	Renderthread function: Try to stream out texture mip-levels to free up more memory.
 *	@param InCandidateTextures	- Array of possible textures to shrink
 *	@param RequiredMemorySize	- Amount of memory to try to free up, in bytes
 *	@param bSucceeded			- [out] Upon return, whether it succeeded or not
 **/
void Renderthread_StreamOutTextureData( TArray<FTextureSortElement>* InCandidateTextures, INT RequiredMemorySize, UBOOL* bSucceeded )
{
	RHIBeginScene();

	INT OldAllocatedMemorySize = INDEX_NONE;
	INT OldAvailableMemorySize = INDEX_NONE;
	UBOOL bRHISupportsMemoryStats = RHIGetTextureMemoryStats( OldAllocatedMemorySize, OldAvailableMemorySize );

	// Makes sure that texture memory can get freed up right away.
	RHIBlockUntilGPUIdle();

	FTextureSortElement* CandidateTextures = InCandidateTextures->GetTypedData();

#if XBOX
	// Fill in the texture base address
	for ( INT TextureIndex=0; TextureIndex < InCandidateTextures->Num(); ++TextureIndex )
	{
		FTextureSortElement& SortElement = CandidateTextures[TextureIndex];
		FXeTextureBase* XeTexture = SortElement.Texture->Resource->TextureRHI;
		SortElement.TextureDataAddress = DWORD(XeTexture->BaseAddress);
	}
#endif

	// Sort the candidates.
	::Sort<FTextureSortElement, FTextureStreamingCompare>( CandidateTextures, InCandidateTextures->Num() );

	// Attempt to shrink enough candidates to free up the required memory size. One mip-level at a time.
	INT SavedMemory = 0;
	UBOOL bKeepShrinking = TRUE;
	UBOOL bShrinkCharacterTextures = FALSE;	// Don't shrink any character textures the first loop.
	while ( SavedMemory < RequiredMemorySize && bKeepShrinking )
	{
		// If we can't shrink anything in the inner-loop, stop the outer-loop as well.
		bKeepShrinking = !bShrinkCharacterTextures;

		for ( INT TextureIndex=0; TextureIndex < InCandidateTextures->Num() && SavedMemory < RequiredMemorySize; ++TextureIndex )
		{
			FTextureSortElement& SortElement = CandidateTextures[TextureIndex];
			INT NewMipCount = SortElement.Texture->ResidentMips - 1;
			if ( (!SortElement.bIsCharacterTexture || bShrinkCharacterTextures) && NewMipCount >= SortElement.NumRequiredResidentMips )
			{
				FTexture2DResource* Resource = (FTexture2DResource*) SortElement.Texture->Resource;

				UBOOL bReallocationSucceeded = Resource->TryReallocate( SortElement.Texture->ResidentMips, NewMipCount );
				if ( bReallocationSucceeded )
				{
					// Start using the new one.
					INT OldSize = SortElement.Size;
					INT NewSize = SortElement.Texture->GetSize(NewMipCount);
					INT Savings = OldSize - NewSize;

					// Set up UTexture2D
					SortElement.Texture->ResidentMips = NewMipCount;
					SortElement.Texture->RequestedMips = NewMipCount;

					// Ok, we found at least one we could shrink. Lets try to find more! :)
					bKeepShrinking = TRUE;

					SavedMemory += Savings;
				}
			}
		}

		// Start shrinking character textures as well, if we have to.
		bShrinkCharacterTextures = TRUE;
	}

	INT NewAllocatedMemorySize = INDEX_NONE;
	INT NewAvailableMemorySize = INDEX_NONE;
	bRHISupportsMemoryStats = RHIGetTextureMemoryStats( NewAllocatedMemorySize, NewAvailableMemorySize );

	debugf(TEXT("Streaming out texture memory! Saved %.2f MB."), FLOAT(SavedMemory)/1024.0f/1024.0f);

	// Return the result.
	*bSucceeded = SavedMemory >= RequiredMemorySize;

	RHIEndScene();
}

/**
 *	Try to stream out texture mip-levels to free up more memory.
 *	@param RequiredMemorySize	- Additional texture memory required
 *	@return						- Whether it succeeded or not
 **/
UBOOL FStreamingManagerTexture::StreamOutTextureData( INT RequiredMemorySize )
{
	RequiredMemorySize = Max<INT>(RequiredMemorySize, MinEvictSize);

	// Array of candidates for reducing mip-levels.
	TArray<FTextureSortElement> CandidateTextures;
	CandidateTextures.Reserve( 1024 );

	// Don't stream out character textures (to begin with)
	FLOAT CurrentTime = FLOAT(appSeconds() - GStartTime);

	// Collect all textures will be considered for streaming out.
	TLinkedList<UTexture2D*>* CurrentStreamableLink = UTexture2D::GetStreamableList();
	while ( CurrentStreamableLink )
	{
		// Advance streamable link.
		UTexture2D* Texture		= **CurrentStreamableLink;
		CurrentStreamableLink	= CurrentStreamableLink->Next();

		// Skyboxes should not stream out.
		if ( Texture->LODGroup == TEXTUREGROUP_Skybox )
			continue;

		// Number of mip-levels that must be resident due to mip-tails and GMinTextureResidentMipCount.
		INT NumRequiredResidentMips = (Texture->MipTailBaseIdx >= 0) ? Max<INT>(Texture->Mips.Num() - Texture->MipTailBaseIdx, 0 ) : 0;
		NumRequiredResidentMips = Max<INT>(NumRequiredResidentMips, GMinTextureResidentMipCount);

		// Only consider streamable textures that have enough miplevels, and who are currently ready for streaming.
		if ( Texture->bIsStreamable && Texture->NeverStream == FALSE && Texture->ResidentMips > NumRequiredResidentMips && Texture->IsReadyForStreaming() )
		{
			// We can't stream out mip-tails.
			INT CurrentBaseMip = Texture->Mips.Num() - Texture->ResidentMips;
			if ( Texture->MipTailBaseIdx < 0 || CurrentBaseMip < Texture->MipTailBaseIdx )
			{
				// Figure out whether texture should be forced resident based on bools and forced resident time.
				UBOOL bForceMipLevelsToBeResident = (Texture->ShouldMipLevelsBeForcedResident() || Texture->ForceMipLevelsToBeResidentTimestamp >= CurrentTime);
				if ( bForceMipLevelsToBeResident == FALSE && Texture->Resource )
				{
					// Don't try to stream out if the texture isn't ready.
					UBOOL bSafeToStream = (Texture->UpdateStreamingStatus() == FALSE);
					if ( bSafeToStream )
					{
						UBOOL bIsCharacterTexture =	Texture->LODGroup == TEXTUREGROUP_Character ||
													Texture->LODGroup == TEXTUREGROUP_CharacterSpecular ||
													Texture->LODGroup == TEXTUREGROUP_CharacterNormalMap;
						DWORD TextureDataAddress = 0;
						CandidateTextures.AddItem( FTextureSortElement(Texture, Texture->GetSize(Texture->ResidentMips), bIsCharacterTexture ? 1 : 0, TextureDataAddress, NumRequiredResidentMips) );
					}
				}
			}
		}
	}

	UBOOL bSucceeded = FALSE;

	// Queue up the process on the render thread.
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
		StreamOutTextureDataCommand,
		TArray<FTextureSortElement>*,CandidateTextures,&CandidateTextures,
		INT,RequiredMemorySize,RequiredMemorySize,
		UBOOL*,bSucceeded,&bSucceeded,
	{
		Renderthread_StreamOutTextureData( CandidateTextures, RequiredMemorySize, bSucceeded );
	});

	// Block until the command has finished executing.
	FlushRenderingCommands();

	return bSucceeded;
}

/** Turn on ENABLE_TEXTURE_TRACKING and setup GTrackedTextures to track specific textures through the streaming system. */
#define ENABLE_TEXTURE_TRACKING 0
#if ENABLE_TEXTURE_TRACKING
struct FTrackedTextureEvent
{
	FTrackedTextureEvent( TCHAR* InTextureName=NULL )
	:	TextureName(InTextureName)
	,	NumResidentMips(0)
	,	NumRequestedMips(0)
	,	StreamingStatus(0)
	,	Timestamp(0.0f)
	{
	}
	FTrackedTextureEvent( const FString& InTextureNameString )
	:	NumResidentMips(0)
	,	NumRequestedMips(0)
	,	StreamingStatus(0)
	,	Timestamp(0.0f)
	{
		TextureName = new TCHAR[InTextureNameString.Len() + 1];
		appMemcpy( TextureName, *InTextureNameString, (InTextureNameString.Len() + 1)*sizeof(TCHAR) );
	}
	/** Partial name of the texture. */
	TCHAR*	TextureName;
	/** Number of mip-levels currently in memory. */
	INT		NumResidentMips;
	/** Number of mip-levels requested. */
	INT		NumRequestedMips;
	/** Streaming status. */
	INT		StreamingStatus;
	/** Timestamp, in seconds from startup. */
	FLOAT	Timestamp;
};
/** List of textures to track (using stristr for name comparison). */
FTrackedTextureEvent GTrackedTextureNames[] =
{
//	FTrackedTextureEvent(TEXT("Lambent_Brumak_05_B_Arm_D")),
//	FTrackedTextureEvent(TEXT("COG_HelmetAlt_Diffuse")),
	FTrackedTextureEvent(TEXT("COG_Dom.Game.COG_DOM_Game_D")),
	FTrackedTextureEvent(TEXT("Cog_Marcus_Game_U_D")),
//	FTrackedTextureEvent(TEXT("COG_Derrick")),
	FTrackedTextureEvent(TEXT("COG_Marcus_Cine_U_Body_D")),
	FTrackedTextureEvent(TEXT("COG_Minh_Kim_UV_MERGE_DiffuseMap")),
	FTrackedTextureEvent(TEXT("Locust_Grunt_UV_MERGE_DiffuseMap")),
};
#define NUM_TRACKEDTEXTUREEVENTS 512
FTrackedTextureEvent GTrackedTextureEvents[NUM_TRACKEDTEXTUREEVENTS];
INT GTrackedTextureEventIndex = -1;
TArray<FTrackedTextureEvent> GTrackedTextures;
void TrackTextureEvent( UTexture2D* Texture, UBOOL bIsDestroying, UBOOL bEnableLogging, UBOOL bForceMipLevelsToBeResident )
{
	INT NumTrackedTextures = ARRAY_COUNT(GTrackedTextureNames);
	if ( NumTrackedTextures )
	{
		// See if it matches any of the texture names that we're tracking.
		FTexture2DMipMap** MipMaps = Texture->Mips.GetTypedData();
		FString TextureNameString = Texture->GetFullName();
		const TCHAR* TextureName = *TextureNameString;
		for ( INT TrackedTextureIndex=0; TrackedTextureIndex < NumTrackedTextures; ++TrackedTextureIndex )
		{
			FTrackedTextureEvent& CurrEvent = GTrackedTextureNames[TrackedTextureIndex];
			if ( appStristr(TextureName, CurrEvent.TextureName) != NULL )
			{
				// Find the last event for this particular texture.
				FTrackedTextureEvent* LastEvent = NULL;
				for ( INT LastEventIndex=0; LastEventIndex < GTrackedTextures.Num(); ++LastEventIndex )
				{
					FTrackedTextureEvent* Event = &GTrackedTextures(LastEventIndex);
					if ( appStrcmp(TextureName, Event->TextureName) == 0 )
					{
						LastEvent = Event;
						break;
					}
				}
				// Didn't find any recorded event?
				if ( LastEvent == NULL )
				{
					INT NewIndex = GTrackedTextures.AddItem(FTrackedTextureEvent(TextureNameString));
					LastEvent = &GTrackedTextures(NewIndex);
				}
				INT StreamingStatus = Texture->PendingMipChangeRequestStatus.GetValue();
				if ( LastEvent->NumResidentMips != Texture->ResidentMips ||
					 LastEvent->NumRequestedMips != Texture->RequestedMips ||
					 LastEvent->StreamingStatus != StreamingStatus ||
					 bIsDestroying )
				{
					GTrackedTextureEventIndex = (GTrackedTextureEventIndex + 1) % NUM_TRACKEDTEXTUREEVENTS;
					FTrackedTextureEvent& NewEvent = GTrackedTextureEvents[GTrackedTextureEventIndex];
					NewEvent.TextureName = LastEvent->TextureName;
					NewEvent.NumResidentMips = LastEvent->NumResidentMips = Texture->ResidentMips;
					NewEvent.NumRequestedMips = LastEvent->NumRequestedMips = Texture->RequestedMips;
					NewEvent.StreamingStatus = LastEvent->StreamingStatus = StreamingStatus;
					NewEvent.Timestamp = LastEvent->Timestamp = FLOAT(appSeconds() - GStartTime);
					debugf( NAME_DevStreaming, TEXT("Texture: \"%s\", ResidentMips: %d, RequestedMips: %d, StreamingStatus: %d, Forced: %d (%s)"), TextureName, LastEvent->NumResidentMips, LastEvent->NumRequestedMips, LastEvent->StreamingStatus, bForceMipLevelsToBeResident, bIsDestroying ? TEXT("DESTROYED") : TEXT("updated") );
					break;
				}
			}
		}
	}
}
#endif

/**
 * Updates streaming, taking into all views infos.
 *
 * @param DeltaTime		Time since last call in seconds
 */
void FStreamingManagerTexture::UpdateResourceStreaming( FLOAT DeltaTime )
{
	SCOPE_CYCLE_COUNTER(STAT_GameThreadUpdateTime);

	// Reset stats first texture. Split up from updating as GC might cause a reset in the middle of iteration
	// and we don't want to report bogus stats in this case.
	if( UTexture2D::GetCurrentStreamableLink() == UTexture2D::GetStreamableList() )
	{
		NumStreamingTextures								= 0;
		NumRequestsInCancelationPhase						= 0;
		NumRequestsInUpdatePhase							= 0;
		NumRequestsInFinalizePhase							= 0;
		TotalIntermediateTexturesSize						= 0;
		NumIntermediateTextures								= 0;
		TotalStreamingTexturesSize							= 0;
		TotalStreamingTexturesMaxSize						= 0;
		TotalMipCountIncreaseRequestsInFlight				= 0;
		LastFullIterationTime								= CurrentFullIterationTime;
		CurrentFullIterationTime							= 0;
	}

	DWORD ThisFrameNumStreamingTextures						= 0;
	DWORD ThisFrameNumRequestsInCancelationPhase			= 0;
	DWORD ThisFrameNumRequestsInUpdatePhase					= 0;
	DWORD ThisFrameNumRequestsInFinalizePhase				= 0;
	DWORD ThisFrameTotalIntermediateTexturesSize			= 0;
	DWORD ThisFrameNumIntermediateTextures					= 0;
	DWORD ThisFrameTotalStreamingTexturesSize				= 0;
	DWORD ThisFrameTotalStreamingTexturesMaxSize			= 0;
	DWORD ThisFrameTotalRequestSize							= 0; 
	DWORD ThisFrameTotalMipCountIncreaseRequestsInFlight	= 0;

	// Keep track of time a full iteration takes.
	CurrentFullIterationTime += DeltaTime;

	// Decrease counter to disregard world textures if it's set.
	if( RemainingTicksToDisregardWorldTextures > 0 )
	{
		RemainingTicksToDisregardWorldTextures--;
	}

	// Fallback handler used if texture is not handled by any other handlers. Guaranteed to handle texture and not return INDEX_NONE.
	FStreamingHandlerTextureLastRender FallbackStreamingHandler;

	// Available texture memory, if supported by RHI. This stat is async as the renderer allocates the memory in its own thread so we
	// only query once and roughly adjust the values as needed.
	INT		AllocatedMemorySize		= INDEX_NONE;
	INT		AvailableMemorySize		= INDEX_NONE;
	UBOOL	bRHISupportsMemoryStats = RHIGetTextureMemoryStats( AllocatedMemorySize, AvailableMemorySize );

	// Update stats if supported.
	if( bRHISupportsMemoryStats )
	{
		// set total size for the pool (used to available)
		STAT(GStatManager.SetAvailableMemory(MCR_TexturePool1, AvailableMemorySize + AllocatedMemorySize));
		SET_DWORD_STAT(STAT_TexturePoolAllocatedSize,AllocatedMemorySize);
	}
	else
	{
		SET_DWORD_STAT(STAT_TexturePoolAllocatedSize,0);
	}

	// Update suspend count.
	if( RemainingTicksToSuspendActivity > 0 )
	{
		RemainingTicksToSuspendActivity--;
	}

	// Update fudge factor after clamping delta time to something reasonable.
	MinFudgeFactor	= Clamp( MinFudgeFactor, 0.1f, 10.f );
	FudgeFactor		= Clamp( FudgeFactor + FudgeFactorRateOfChange * Min( 0.1f, DeltaTime ), MinFudgeFactor, 10.f );

	FLOAT CurrentTime = FLOAT(appSeconds() - GStartTime);

	// Get current streamable link and iterate over subset of textures each frame.
	TLinkedList<UTexture2D*>*& CurrentStreamableLink = UTexture2D::GetCurrentStreamableLink();
	INT MaxTexturesToProcess = Max( 1, UTexture2D::GetNumStreamableTextures() / NUM_TICKS_FOR_FULL_ITERATION );
	while( CurrentStreamableLink && MaxTexturesToProcess-- )
	{
		// Advance streamable link.
		UTexture2D* Texture		= **CurrentStreamableLink;
		CurrentStreamableLink	= CurrentStreamableLink->Next();

		// This section can be used to track how specific textures are streamed.
#if ENABLE_TEXTURE_TRACKING
		TrackTextureEvent( Texture, FALSE, FALSE, FALSE );
#endif

		FTexture2DResource* Texture2DResource = (FTexture2DResource*) Texture->Resource;
		UBOOL bTextureIsOrphan = ((Texture2DResource != NULL) && Texture2DResource->IsOrphan());

		// Skip world textures to allow e.g. character textures to stream in first.
		if( RemainingTicksToDisregardWorldTextures 
		&&	((	Texture->LODGroup != TEXTUREGROUP_Character)
			&&	Texture->LODGroup != TEXTUREGROUP_CharacterSpecular
			&&	Texture->LODGroup != TEXTUREGROUP_CharacterNormalMap) )
		{
			continue;
		}

		// Figure out max number of miplevels allowed by LOD code.
		INT RequestedMips	= INDEX_NONE;
		INT	MaxResidentMips	= Max( 1, Min( Texture->Mips.Num() - Texture->GetCachedLODBias(), GMaxTextureMipCount ) );

		UBOOL bForceMipLevelsToBeResident = FALSE;

		// Only work on streamable textures that have enough miplevels.
		if( Texture->bIsStreamable
		&&	Texture->NeverStream == FALSE
		&&	((Texture->Mips.Num() > GMinTextureResidentMipCount) || bTextureIsOrphan))
		{
			STAT(ThisFrameNumStreamingTextures++);
			STAT(ThisFrameTotalStreamingTexturesSize += Texture->GetSize(Texture->ResidentMips));
			STAT(ThisFrameTotalStreamingTexturesMaxSize += Texture->GetSize(MaxResidentMips));

			// No need to figure out which miplevels we want if the texture is still in the process of being
			// created for the first time. We also cannot change the texture during that time outside of 
			// UpdateResource.
			if( Texture->IsReadyForStreaming() )
			{
				// Calculate LOD allowed mips passed to GetWantedMips.
				INT	TextureLODBias	= Texture->GetCachedLODBias();
				INT LODAllowedMips	= Max( 1, Texture->Mips.Num() - TextureLODBias );					

				// Figure out whether texture should be forced resident based on bools and forced resident time.
				if( Texture->ShouldMipLevelsBeForcedResident() )
				{
					bForceMipLevelsToBeResident = TRUE;
				}
				else if( Texture->ForceMipLevelsToBeResidentTimestamp >= CurrentTime )
				{
					bForceMipLevelsToBeResident = TRUE;
				}

				// We only stream in skybox textures as you are always within the bounding box and those textures
				// tend to be huge. A streaming fudge factor fluctuating around 1 will cause them to be streamed in
				// and out, making it likely for the allocator to fragment.
				if( Texture->LODGroup == TEXTUREGROUP_Skybox )
				{
					bForceMipLevelsToBeResident = TRUE;
				}

				// Don't stream in all referenced textures but rather only those that have been rendered in the last 5 minutes if
				// we only stream in textures. This means you still might see texture popping, but the option is designed to avoid
				// hitching due to CPU overhead, which is still taken care off by the 5 minute rule.
				if( GSystemSettings.bOnlyStreamInTextures )
				{
					FLOAT SecondsSinceLastRender = (GCurrentTime - Texture->Resource->LastRenderTime);
					if( SecondsSinceLastRender < 300 )
					{
						bForceMipLevelsToBeResident = TRUE;
					}
				}

				if(bTextureIsOrphan)
				{
					RequestedMips = Texture->ResidentMips;
				}
				// Figure out miplevels to request based on handlers and whether streaming is enabled.
				else if(!bForceMipLevelsToBeResident && GEngine->bUseTextureStreaming)
				{
					// Iterate over all handlers and figure out the maximum requested number of mips.
					for( INT HandlerIndex=0; HandlerIndex<TextureStreamingHandlers.Num(); HandlerIndex++ )
					{
						FStreamingHandlerTextureBase* TextureStreamingHandler = TextureStreamingHandlers(HandlerIndex);
						INT WantedMips	= TextureStreamingHandler->GetWantedMips( Texture, ViewInfos, FudgeFactor, LODAllowedMips );
						RequestedMips	= Max( RequestedMips, WantedMips );
					}

					// Not handled by any handler, use fallback handler.
					if( RequestedMips == INDEX_NONE )
					{
						RequestedMips	= FallbackStreamingHandler.GetWantedMips( Texture, ViewInfos, FudgeFactor, LODAllowedMips );
					}
				}
				// Stream in all allowed miplevels if requested.
				else
				{
					RequestedMips = LODAllowedMips;
				}
	
				// Take texture LOD and maximum texture size into account.
				INT MinAllowedMips  = Min( LODAllowedMips, GMinTextureResidentMipCount );
				INT MaxAllowedMips	= Min( LODAllowedMips, GMaxTextureMipCount );
				RequestedMips		= Clamp( RequestedMips, MinAllowedMips, MaxAllowedMips );

				// Number of levels in the packed miptail.
				INT NumMipTailLevels= Max(0,Texture->Mips.Num() - Texture->MipTailBaseIdx);
				// Make sure that we at least load the mips that reside in the packed miptail.
				RequestedMips		= Max( RequestedMips, NumMipTailLevels );
				
				check( RequestedMips != INDEX_NONE );
				check( RequestedMips >= 1 );
				check( RequestedMips <= Texture->Mips.Num() ); 

				// If TRUE we're so low on memory that we cannot request any changes.
				UBOOL	bIsStopLimitExceeded	= bRHISupportsMemoryStats && (AvailableMemorySize < MemoryStopStreamingLimit);
				// And due to the async nature of memory requests we must suspend activity for at least 2 frames.
				if( bIsStopLimitExceeded )
				{
					// Value gets decremented at top so setting it to 3 will suspend 2 ticks.
					RemainingTicksToSuspendActivity = 3;
				}
				UBOOL	bAllowRequests			= RemainingTicksToSuspendActivity == 0;

				// Update streaming status. A return value of FALSE means that we're done streaming this texture
				// so we can potentially request another change.
				UBOOL	bSafeToRequest			= (Texture->UpdateStreamingStatus() == FALSE);
				INT		RequestStatus			= Texture->PendingMipChangeRequestStatus.GetValue();
				UBOOL	bHasCancelationPending	= Texture->bHasCancelationPending;
			
				// Checked whether we should cancel the pending request if the new one is different.
				if( !bSafeToRequest && (Texture->RequestedMips != RequestedMips) && !bHasCancelationPending )
				{
					// Cancel load.
					if( (RequestedMips < Texture->RequestedMips) && (RequestedMips >= Texture->ResidentMips) )
					{
						bHasCancelationPending = Texture->CancelPendingMipChangeRequest();
					}
					// Cancel unload.
					else if( (RequestedMips > Texture->RequestedMips) && (RequestedMips <= Texture->ResidentMips) )
					{
						bHasCancelationPending = Texture->CancelPendingMipChangeRequest();
					}
				}

				if( bHasCancelationPending )
				{
					ThisFrameNumRequestsInCancelationPhase++;
				}
				else if( RequestStatus >= TEXTURE_READY_FOR_FINALIZATION )
				{
					ThisFrameNumRequestsInUpdatePhase++;
				}
				else if( RequestStatus == TEXTURE_FINALIZATION_IN_PROGRESS )
				{
					ThisFrameNumRequestsInFinalizePhase++;
				}	

				// Request is in flight so there is an intermediate texture with RequestedMips miplevels.
				if( RequestStatus > 0 )
				{
					ThisFrameTotalIntermediateTexturesSize += Texture->GetSize(Texture->RequestedMips);
					ThisFrameNumIntermediateTextures++;
					// Update texture increase request stats.
					if( Texture->RequestedMips > Texture->ResidentMips )
					{
						ThisFrameTotalMipCountIncreaseRequestsInFlight++;
					}
				}

				// Request a change if it's safe and requested mip count differs from resident.
				if( bSafeToRequest && bAllowRequests)
				{
					if( RequestedMips != Texture->ResidentMips )
					{
						UBOOL bCanRequestTextureIncrease = TRUE;

						// If memory stats are supported we respect the memory limit to not stream in miplevels.
						if( ( bRHISupportsMemoryStats && (AvailableMemorySize <= MemoryStopIncreasingLimit) )
						// Don't request increase if requested mip count is below threshold.
						||	(bUseMinRequestLimit && RequestedMips < MinRequestedMipsToConsider) )
						{
							bCanRequestTextureIncrease = FALSE;
						}

						// Only request change if it's a decrease or we can request an increase.
						if( (RequestedMips < Texture->ResidentMips) || bCanRequestTextureIncrease )						
						{
							// Manually update size as allocations are deferred/ happening in rendering thread.
							INT CurrentRequestSize		= Texture->GetSize(RequestedMips);
							AvailableMemorySize			-= CurrentRequestSize;
							// Keep track of current allocations this frame and stop iteration if we've exceeded
							// frame limit.
							ThisFrameTotalRequestSize	+= CurrentRequestSize;
							if( ThisFrameTotalRequestSize > MAX_PER_FRAME_REQUEST_LIMIT )
							{
								// We've exceeded the request limit for this frame.
								MaxTexturesToProcess = 0;
							}

							check(RequestStatus==TEXTURE_READY_FOR_REQUESTS);
							check(Texture->PendingMipChangeRequestStatus.GetValue()==TEXTURE_READY_FOR_REQUESTS);
							check(!Texture->bHasCancelationPending);
							// Set new requested mip count.
							Texture->RequestedMips = RequestedMips;
							// Enqueue command to update mip count.
							UBOOL bShouldPrioritizeAsyncIORequest = RemainingTicksToDisregardWorldTextures || bForceMipLevelsToBeResident;
							Texture2DResource->BeginUpdateMipCount( bShouldPrioritizeAsyncIORequest );
						}
					}
					else if(bTextureIsOrphan)
					{
						// This previously shared resource will be re-allocated 
						// so it may share memory again with a new donor
						INT CurrentRequestSize		= Texture->GetSize(Texture->RequestedMips);
						// Keep track of current allocations this frame and stop iteration if we've exceeded
						// frame limit.
						ThisFrameTotalRequestSize	+= CurrentRequestSize;
						// Enqueue command to re-allocate memory (mip count stays the same).
						UBOOL bShouldPrioritizeAsyncIORequest = RemainingTicksToDisregardWorldTextures || bForceMipLevelsToBeResident;
						Texture2DResource->BeginUpdateMipCount( bShouldPrioritizeAsyncIORequest );
					}
				}
			}
		}

		// This section can be used to track how specific textures are streamed.
#if ENABLE_TEXTURE_TRACKING
		TrackTextureEvent( Texture, FALSE, FALSE, bForceMipLevelsToBeResident );
#endif
	}

	// Update fudge factor rate of change if memory stats are supported.
	if( bRHISupportsMemoryStats )
	{
		if( AvailableMemorySize <= MemoryDropMipLevelsLimit )
		{
			FudgeFactorRateOfChange = FudgeFactorIncreaseRateOfChange;
		}
		else if( AvailableMemorySize > MemoryHysteresisLimit )
		{
			FudgeFactorRateOfChange	= FudgeFactorDecreaseRateOfChange;
			// Special handling if fudge factor is going to be less than 1.
			if( FudgeFactor < (1 - FudgeFactorDecreaseRateOfChange) )
			{
				// Multiply by min fudge factor to linearize range 0.1 .. 1
				FudgeFactorRateOfChange *= MinFudgeFactor;
			}
		}
		else
		{
			FudgeFactorRateOfChange = 0;
		}
	}

	// Update running counts with this frames stats.
	NumStreamingTextures					+= ThisFrameNumStreamingTextures;
	NumRequestsInCancelationPhase			+= ThisFrameNumRequestsInCancelationPhase;
	NumRequestsInUpdatePhase				+= ThisFrameNumRequestsInUpdatePhase;
	NumRequestsInFinalizePhase				+= ThisFrameNumRequestsInFinalizePhase;
	TotalIntermediateTexturesSize			+= ThisFrameTotalIntermediateTexturesSize;
	NumIntermediateTextures					+= ThisFrameNumIntermediateTextures;
	TotalStreamingTexturesSize				+= ThisFrameTotalStreamingTexturesSize;
	TotalStreamingTexturesMaxSize			+= ThisFrameTotalStreamingTexturesMaxSize;
	TotalMipCountIncreaseRequestsInFlight	+= ThisFrameTotalMipCountIncreaseRequestsInFlight;

	// Set the stats on wrap-around. Reset happens independently to correctly handle resetting in the middle of iteration.
	if( CurrentStreamableLink == NULL )
	{
		SET_DWORD_STAT( STAT_StreamingTextures,				NumStreamingTextures			);
		SET_DWORD_STAT( STAT_RequestsInCancelationPhase,	NumRequestsInCancelationPhase	);
		SET_DWORD_STAT( STAT_RequestsInUpdatePhase,			NumRequestsInUpdatePhase		);
		SET_DWORD_STAT( STAT_RequestsInFinalizePhase,		NumRequestsInFinalizePhase		);
		SET_DWORD_STAT( STAT_IntermediateTexturesSize,		TotalIntermediateTexturesSize	);
		SET_DWORD_STAT( STAT_IntermediateTextures,			NumIntermediateTextures			);
		SET_DWORD_STAT( STAT_StreamingTexturesSize,			TotalStreamingTexturesSize		);
		SET_DWORD_STAT( STAT_StreamingTexturesMaxSize,		TotalStreamingTexturesMaxSize	);

		// Determine whether should disable this feature.
		if( bUseMinRequestLimit )
		{
			FLOAT TimeSinceLastLevelChange = appSeconds() - LastLevelChangeTime;
			// No longer use min request limit if we're out of requests and the min guaranteed time has elapsed...
			if( (TimeSinceLastLevelChange > MinTimeToGuaranteeMinMipCount && ThisFrameTotalMipCountIncreaseRequestsInFlight == 0)
			// ... or if the max allowed time has elapsed.
			||	(TimeSinceLastLevelChange > MaxTimeToGuaranteeMinMipCount) )
			{
				bUseMinRequestLimit = FALSE;
			}
		}
		// debugf( TEXT("%5.2f MByte outstanding requests, %6.2f"), TotalIntermediateTexturesSize / 1024.f / 1024.f, appSeconds() - GStartTime );
	}

	SET_DWORD_STAT(		STAT_RequestSizeCurrentFrame,		ThisFrameTotalRequestSize		);
	INC_DWORD_STAT_BY(	STAT_RequestSizeTotal,				ThisFrameTotalRequestSize		);
	SET_FLOAT_STAT(		STAT_StreamingFudgeFactor,			FudgeFactor						);
}

/**
 * Blocks till all pending requests are fulfilled.
 */
void FStreamingManagerTexture::BlockTillAllRequestsFinished()
{
	// Flush rendering commands.
	FlushRenderingCommands();
	
	// Iterate over all textures and make sure that they are done streaming.
	for( TObjectIterator<UTexture2D> It; It; ++It )
	{
		UTexture2D* Texture	= *It;
		// Update the streaming portion till we're done streaming.
		while( Texture->UpdateStreamingStatus() )
		{
			// Give up the timeslice.
			appSleep(0);
		}
		INT RequestStatus = Texture->PendingMipChangeRequestStatus.GetValue();
		check( RequestStatus == TEXTURE_READY_FOR_REQUESTS || RequestStatus == TEXTURE_PENDING_INITIALIZATION );
	}
}

/**
 * Adds a textures streaming handler to the array of handlers used to determine which
 * miplevels need to be streamed in/ out.
 *
 * @param TextureStreamingHandler	Handler to add
 */
void FStreamingManagerTexture::AddTextureStreamingHandler( FStreamingHandlerTextureBase* TextureStreamingHandler )
{
	TextureStreamingHandlers.AddItem( TextureStreamingHandler );
}

/**
 * Removes a textures streaming handler from the array of handlers used to determine which
 * miplevels need to be streamed in/ out.
 *
 * @param TextureStreamingHandler	Handler to remove
 */
void FStreamingManagerTexture::RemoveTextureStreamingHandler( FStreamingHandlerTextureBase* TextureStreamingHandler )
{
	TextureStreamingHandlers.RemoveItem( TextureStreamingHandler );
}


/*-----------------------------------------------------------------------------
	Streaming handler implementations.
-----------------------------------------------------------------------------*/

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
INT FStreamingHandlerTextureStatic::GetWantedMips( UTexture2D* Texture, const TArray<FStreamingViewInfo>& ViewInfos, FLOAT FudgeFactor, INT LODAllowedMips )
{
	// INDEX_NONE signals handler  not handling the texture.
	INT		WantedMipCount		= INDEX_NONE;
	// Whether we should early out if we e.g. know that we need all allowed mips.
	UBOOL	bShouldAbortLoop	= FALSE;

	// Nothing do to if there are no views.
	if( ViewInfos.Num() )
	{
		// Cached so it's only calculated once per texture.
		const FLOAT SquaredFudgeFactor = Square(FudgeFactor);

		// Iterate over all associated/ visible levels.
		for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num() && !bShouldAbortLoop; LevelIndex++ )
		{
			// Find instances of the texture in this level.
			ULevel*								Level				= GWorld->Levels(LevelIndex);
			TArray<FStreamableTextureInstance>* TextureInstances	= Level->TextureToInstancesMap.Find( Texture );

			// Nothing to do if there are no instances.
			if( TextureInstances && TextureInstances->Num() )
			{
				// Iterate over all view infos.
				for( INT ViewIndex=0; ViewIndex<ViewInfos.Num() && !bShouldAbortLoop; ViewIndex++ )
				{
					const FStreamingViewInfo& ViewInfo	= ViewInfos(ViewIndex);

					// Iterate over all instances of the texture in the level.
					for( INT InstanceIndex=0; InstanceIndex<TextureInstances->Num() && !bShouldAbortLoop; InstanceIndex++ )
					{
						FStreamableTextureInstance& TextureInstance = (*TextureInstances)(InstanceIndex);

						// Calculate distance of viewer to bounding sphere.
						FLOAT	DistSq					= FDistSquared( ViewInfo.ViewOrigin, TextureInstance.BoundingSphere ) * SquaredFudgeFactor;
						FLOAT	DistSqMinusRadiusSq		= DistSq - Square(TextureInstance.BoundingSphere.W);

						// Outside the texture instance bounding sphere, calculate miplevel based on screen space size of bounding sphere.
						if( DistSqMinusRadiusSq > 1.f )
						{
							// Calculate the maximum screen space dimension in pixels.
							FLOAT	ScreenSizeInTexels	= TextureInstance.TexelFactor * appInvSqrtEst( DistSqMinusRadiusSq ) * ViewInfo.ScreenSize;
							// WantedMipCount is the number of mips so we need to adjust with "+ 1".
							WantedMipCount				= Max<INT>( WantedMipCount, 1 + appCeilLogTwo( appTrunc(ScreenSizeInTexels) ) );
						}
						// Request all miplevels to be loaded if we're inside the bounding sphere.
						else
						{
							WantedMipCount				= Texture->Mips.Num();
						}

						// Early out if we're already at the max.
						if( WantedMipCount >= LODAllowedMips )
						{
							bShouldAbortLoop = TRUE;
						}
					}
				}
			}
		}
	}
	return WantedMipCount;
}

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
INT FStreamingHandlerTextureLastRender::GetWantedMips( UTexture2D* Texture, const TArray<FStreamingViewInfo>& ViewInfos, FLOAT FudgeFactor, INT LODAllowedMips )
{
	FLOAT SecondsSinceLastRender = (GCurrentTime - Texture->Resource->LastRenderTime); // * FudgeFactor;

	if( SecondsSinceLastRender < 45 )
	{
		return LODAllowedMips;
	}
	else if( SecondsSinceLastRender < 90 )
	{
		return LODAllowedMips - 1;
	}
	else
	{
		return 0;
	}
}

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
INT FStreamingHandlerTextureLevelForced::GetWantedMips( UTexture2D* Texture, const TArray<FStreamingViewInfo>& ViewInfos, FLOAT FudgeFactor, INT LODAllowedMips )
{
	INT WantedMipCount = INDEX_NONE;
	for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		if( Level->ForceStreamTextures.Find( Texture ) )
		{
			WantedMipCount = LODAllowedMips;
			break;
		}
	}
	return WantedMipCount;
}


/*-----------------------------------------------------------------------------
	Texture streaming helper structs.
-----------------------------------------------------------------------------*/

/**
 * FStreamableTextureInstance serialize operator.
 *
 * @param	Ar					Archive to to serialize object to/ from
 * @param	TextureInstance		Object to serialize
 * @return	Returns the archive passed in
 */
FArchive& operator<<( FArchive& Ar, FStreamableTextureInstance& TextureInstance )
{
	Ar << TextureInstance.BoundingSphere;
	Ar << TextureInstance.TexelFactor;
	return Ar;
}
