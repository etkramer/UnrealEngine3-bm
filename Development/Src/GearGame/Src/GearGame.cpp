//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "GearGame.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"
#include "EngineMaterialClasses.h"
#include "UnNetDrv.h"
#include "DelayedUnpauser.h"
#include "PerfMem.h"
#include "UnNet.h"
#include "EngineSequenceClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineMeshClasses.h"
#include "UnIpDrv.h"

#include "GearGameCameraClasses.h"
#include "GearGameSequenceClasses.h"
#include "GearGameSpecialMovesClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameSoundClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameUIClasses.h"
#include "GearGameAnimClasses.h"
#include "GearGameOnlineClasses.h"
#include "UnSurveys.h"
#include "Database.h"
#include "PerfMem.h"

#include "EnginePrefabClasses.h"

#include "BatchedElements.h"
#include "ProfilingHelpers.h"

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName GEARGAME_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "GearGameClasses.h"
#include "GearGameAIClasses.h"
#include "GearGamePCClasses.h"
#include "GearGamePawnClasses.h"
#include "GearGameCameraClasses.h"
#include "GearGameSequenceClasses.h"
#include "GearGameSpecialMovesClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameSoundClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"
#include "GearGameAnimClasses.h"
#include "GearGameOnlineClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "GearGameClasses.h"
#include "GearGameAIClasses.h"
#include "GearGamePCClasses.h"
#include "GearGamePawnClasses.h"
#include "GearGameCameraClasses.h"
#include "GearGameSequenceClasses.h"
#include "GearGameSpecialMovesClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameSoundClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"
#include "GearGameAnimClasses.h"
#include "GearGameOnlineClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsGearGame( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_PC
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_PAWN
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_CAMERA
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_SEQUENCE
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_SPECIALMOVES
    AUTO_INITIALIZE_REGISTRANTS_GEARGAME_VEHICLE
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_SOUND
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_AI
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_WEAPON
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_UI
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_UIPRIVATE
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_ANIM
	AUTO_INITIALIZE_REGISTRANTS_GEARGAME_ONLINE
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesGearGame()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
    #define AUTOGENERATE_NAME(name) GEARGAME_##name = FName(TEXT(#name));
	#include "GearGameClasses.h"
	#include "GearGameAIClasses.h"
	#include "GearGamePCClasses.h"
	#include "GearGamePawnClasses.h"
    #include "GearGameCameraClasses.h"
	#include "GearGameSequenceClasses.h"
	#include "GearGameSpecialMovesClasses.h"
    #include "GearGameVehicleClasses.h"
    #include "GearGameSoundClasses.h"
	#include "GearGameWeaponClasses.h"
	#include "GearGameUIClasses.h"
	#include "GearGameUIPrivateClasses.h"
	#include "GearGameAnimClasses.h"
	#include "GearGameOnlineClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}

IMPLEMENT_CLASS(UGearEngine)
IMPLEMENT_CLASS(UGearGameViewportClient)
IMPLEMENT_CLASS(UCheckpoint)

IMPLEMENT_CLASS(UGearCoverMeshComponent)

IMPLEMENT_CLASS(UGearDecal)
IMPLEMENT_CLASS(UGearTypes)
IMPLEMENT_CLASS(UGearDamageType)

IMPLEMENT_CLASS(UGearBloodInfo);
IMPLEMENT_CLASS(UGearDecalInfo);

IMPLEMENT_CLASS(AWalkVolume)

IMPLEMENT_CLASS(AGearPawn)
IMPLEMENT_CLASS(AGearPawn_Infantry)
IMPLEMENT_CLASS(AGearPawn_LocustBase)

IMPLEMENT_CLASS(AGearPawn_COGDom)
IMPLEMENT_CLASS(AGearPawn_COGGear)
IMPLEMENT_CLASS(AGearPawn_COGMarcus)
IMPLEMENT_CLASS(AGearPawn_LocustKantusBase)
IMPLEMENT_CLASS(AGearPawn_LocustSkorgeBase)

IMPLEMENT_CLASS(AGearPawn_LocustBrumakBase)
IMPLEMENT_CLASS(AGearPawn_LocustBrumakHelper_Base)

IMPLEMENT_CLASS(ATurret)
IMPLEMENT_CLASS(AGearTurret)
IMPLEMENT_CLASS(ATurret_TroikaBase)

IMPLEMENT_CLASS(AGearGame)
IMPLEMENT_CLASS(AGearGameSP_Base)
IMPLEMENT_CLASS(AGearGameMP_Base)
IMPLEMENT_CLASS(AGearGameTDM_Base)
IMPLEMENT_CLASS(AGearGameAnnex_Base)
IMPLEMENT_CLASS(AGearGameWingman_Base)
IMPLEMENT_CLASS(AGearGameCTM_Base)

IMPLEMENT_CLASS(AGearPC)
IMPLEMENT_CLASS(AGearPRI)
IMPLEMENT_CLASS(AGearGRI)
IMPLEMENT_CLASS(AGearTeamInfo)
IMPLEMENT_CLASS(UGearCheatManager)
IMPLEMENT_CLASS(UGearObjectiveManager)
IMPLEMENT_CLASS(UDifficultySettings)

IMPLEMENT_CLASS(AGearPickupFactory)
IMPLEMENT_CLASS(AGearWeaponPickupFactory)
IMPLEMENT_CLASS(AGearCrimsonOmenPickupFactoryBase)
IMPLEMENT_CLASS(AGearDiscoverablesPickupFactoryBase)
IMPLEMENT_CLASS(ATrigger_LadderInteraction)
IMPLEMENT_CLASS(ATrigger_Engage)
IMPLEMENT_CLASS(AGearDestructibleObject)
IMPLEMENT_CLASS(AGearEmitter)
IMPLEMENT_CLASS(AGearPointOfInterest)
IMPLEMENT_CLASS(AGearPointOfInterest_Meatflag)
IMPLEMENT_CLASS(AHOD_BeamManagerBase)

IMPLEMENT_CLASS(AInterpActor_Pushable)
IMPLEMENT_CLASS(UActorFactoryPushable)
IMPLEMENT_CLASS(AGearHUD_Base)
IMPLEMENT_CLASS(AGearSpectatorPoint)
IMPLEMENT_CLASS(AHeadTrackTarget)
IMPLEMENT_CLASS(ASpawnedGearEmitter)
IMPLEMENT_CLASS(AEmit_CameraLensEffectBase)
IMPLEMENT_CLASS(AGearDroppedPickup)

IMPLEMENT_CLASS(AGearPawn_RockWormBase);
IMPLEMENT_CLASS(ARockWorm_TailSegment);

IMPLEMENT_CLASS(AInterpActor_GearBasePlatform)
IMPLEMENT_CLASS(ACOG_DerrickWheelsBase)
IMPLEMENT_CLASS(AInterpActor_COGDerrickBase)
IMPLEMENT_CLASS(AInterpActor_LocustBargeBase)
IMPLEMENT_CLASS(AInterpActor_RaftBase)
IMPLEMENT_CLASS(AInterpActor_LocustTortureBargeBase)

IMPLEMENT_CLASS(UGearTutorial_Base);
IMPLEMENT_CLASS(UGearTutorialManager);

IMPLEMENT_CLASS(UGearPhysicalMaterialProperty);

IMPLEMENT_CLASS(AGearPawn_LocustWretchBase);
IMPLEMENT_CLASS(AGearPawn_LocustLeviathanBase);

IMPLEMENT_CLASS(AVolume_Nemacyst)

IMPLEMENT_CLASS(AGearPawn_SecurityBotFlyingBase)

IMPLEMENT_CLASS(AGearPawnBlockingVolume);
IMPLEMENT_CLASS(AGrenadeBlockingVolume);

IMPLEMENT_CLASS(UGearStatsObject);

IMPLEMENT_CLASS(AFlockTestLocust);

IMPLEMENT_CLASS(UGearEventsInterface);

IMPLEMENT_CLASS(USeqEvent_ChapterPoint);

IMPLEMENT_CLASS(AGearPawn_LocustTickerBase);

IMPLEMENT_CLASS(AGearSpawner_EHole)
IMPLEMENT_CLASS(AGearSpawner_EholeSpawnLocation)

IMPLEMENT_CLASS(AGearObjectPool);

IMPLEMENT_CLASS(AGearGameGeneric);

IMPLEMENT_CLASS(AGearPawn_LocustNemacystBase);

IMPLEMENT_CLASS(AGear_GrappleHookMarker);

IMPLEMENT_CLASS(AGearAimAssistActor);


IMPLEMENT_CLASS(UGearPerMapColorConfig);


// Pi / 180
#define DEG_TO_RAD 0.017453292519943296	



/**
 * Asynchronous screenshot compression.
 * PNG compression can take over 10 seconds.
 */
#define JPEGCOMPRESSION _XBOX
#define THREADEDCOMPRESSION (_XBOX && NDEBUG)
#define TRACK_COMPRESSION_TIME 1
#if !PS3
class FAsyncCompressScreenshot : public FAsyncWorkBase
{
protected:
#if !JPEGCOMPRESSION
	/**
	 * Used for PNG compression.
	 * Also holds the uncompressed data.
	 */
	FPNGHelper PNG;
#endif
	/* Width, in pixels, of the image */
	INT Width;
	/* Height, in pixels, of the image */
	INT Height;
	/* The compressed image */
	TArray<BYTE> CompressedData;
	/* The thumbnail */
	TArray<BYTE> ThumbnailData;
#if JPEGCOMPRESSION
	/* The texture to compress */
	FTexture2DRHIRef Texture;
	/* The thumbnail texture */
	FTexture2DRHIRef ThumbnailTexture;
#endif
#if TRACK_COMPRESSION_TIME
	DOUBLE StartTime;
#endif

public:
	/**
	 * Initializes the compression.
	 *
	 * @param Data Pointer to an array of color data.
	 * @param Width The width in pixels of the image.
	 * @param Height The height in pixels of the image.
	 */
	FAsyncCompressScreenshot(FTexture2DRHIParamRef InTexture, INT SizeX, INT SizeY)
		:	FAsyncWorkBase( NULL )
		,	Width(SizeX)
		,	Height(SizeY)
#if JPEGCOMPRESSION
		,	Texture(InTexture)
		,	ThumbnailTexture(NULL)
#endif
	{
	}

	/**
	 * Performs the async compression.
	 */
	virtual void DoWork(void)
	{
		#if USE_XeD3D_RHI
			#if JPEGCOMPRESSION
				CreateThumbnail();
			#endif

			#if TRACK_COMPRESSION_TIME
				StartTime = appSeconds();
			#endif
			#if JPEGCOMPRESSION
				// This code is only used in release builds. In debug builds, RenderThread_Compress() is used instead.
				// It will compress the texture into a separate buffer, while the renderthread may still be rendering
				// the texture to screen. Which is fine.
				ID3DXBuffer* CompressedResult = NULL;
				IDirect3DTexture9* D3DTexture = Texture;
				HRESULT Result = D3DXSaveTextureToFileInMemory( &CompressedResult, D3DXIFF_JPG, D3DTexture, NULL );
				check( SUCCEEDED(Result) );
				void* CompressedDataPtr = CompressedResult->GetBufferPointer();
				DWORD CompressedDataSize = CompressedResult->GetBufferSize();
				SetCompressedData(CompressedDataPtr, CompressedDataSize);
			//		FArchive* Ar = GFileManager->CreateFileWriter( *(appGameDir() + TEXT("screenshot.jpg")), 0, GNull, CompressedSize );
			// 		if ( Ar )
			// 		{
			// 			Ar->Serialize( CompressedDataPtr, CompressedDataSize );
			// 			delete Ar;
			// 		}
				CompressedResult->Release();
			#else
				CompressedData = PNG.GetCompressedData();
			#endif
			#if TRACK_COMPRESSION_TIME
				DOUBLE TotalTime = appSeconds() - StartTime;
				debugf(TEXT("Compressed screenshot from %0.3fMB to %0.3fMB in %0.3f seconds"),
					(Width*Height*sizeof(FColor))/(1024.*1024.),
					(CompressedData.Num())/(1024.*1024.),
					TotalTime);
			#endif
		#endif
	}

#if JPEGCOMPRESSION
	void CreateThumbnail()
	{
		// create a PNG thumbnail
		#if TRACK_COMPRESSION_TIME
			DOUBLE StartThumbnailTime = appSeconds();
		#endif

		IDirect3DTexture9* D3DThumbnailTexture = ThumbnailTexture;
		IDirect3DSurface9* D3DThumbnailSurface = NULL;
		HRESULT ThumbnailResult = D3DThumbnailTexture->GetSurfaceLevel(0, &D3DThumbnailSurface);
		check( SUCCEEDED(ThumbnailResult) );

		// the thumbnail is square, so setup a square source rect
		check(Height <= Width);  // we assume that the screen will be wider than high
		RECT SourceRect;
		SourceRect.top = 0;
		SourceRect.bottom = Height;
		SourceRect.left = (Width / 2) - (Height / 2);
		SourceRect.right = (Width / 2) + (Height / 2);

		// load the thumbnail surface from the full size texture
		IDirect3DTexture9* D3DTexture = Texture;
		IDirect3DSurface9* D3DSurface;
		ThumbnailResult = D3DTexture->GetSurfaceLevel(0, &D3DSurface);
		check( SUCCEEDED(ThumbnailResult) );
		#if TRACK_COMPRESSION_TIME
			DOUBLE StartResizeTime = appSeconds();
		#endif
		ThumbnailResult = D3DXLoadSurfaceFromSurface(
			D3DThumbnailSurface, NULL, NULL,
			D3DSurface, NULL, &SourceRect,
			D3DX_DEFAULT, 0);
		check( SUCCEEDED(ThumbnailResult) );
		#if TRACK_COMPRESSION_TIME
			DOUBLE TotalResizeTime = appSeconds() - StartResizeTime;
			debugf(TEXT("Resized thumbnail (D3DXLoadSurfaceFromSurface): %0.3f seconds"),
				TotalResizeTime);
		#endif

		// save the thumbnail to memory as a PNG
		ID3DXBuffer* PNGBuffer = NULL;
		#if TRACK_COMPRESSION_TIME
			DOUBLE StartCompressTime = appSeconds();
		#endif
		ThumbnailResult = D3DXSaveSurfaceToFileInMemory( &PNGBuffer, D3DXIFF_PNG, D3DThumbnailSurface, NULL, NULL );
		check( SUCCEEDED(ThumbnailResult) );
		D3DThumbnailSurface->Release();
		#if TRACK_COMPRESSION_TIME
			DOUBLE TotalCompressTime = appSeconds() - StartCompressTime;
			debugf(TEXT("Compressed thumbnail (D3DXSaveSurfaceToFileInMemory): %0.3f seconds"),
				TotalCompressTime);
		#endif

		DWORD PNGSize = 0;
		if ( PNGBuffer )
		{
			void* PNGPtr = PNGBuffer->GetBufferPointer();
			PNGSize = PNGBuffer->GetBufferSize();
			// give it to the task
			SetThumbnailData(PNGPtr, PNGSize);
			// release the PNG buffer
			PNGBuffer->Release();
			#if TRACK_COMPRESSION_TIME
				DOUBLE TotalThumbnailTime = appSeconds() - StartThumbnailTime;
				debugf(TEXT("Total thumbnail time: %0.3f seconds"),
					TotalThumbnailTime);
			#endif
		}
	}
#endif

	/**
	 * Gets the compressed data.
	 */
	const TArray<BYTE>& GetCompressedData()
	{
		check(IsDone());
		return CompressedData;
	}

	void SetCompressedData( void* Buffer, INT Size )
	{
		CompressedData.Empty(Size);
		CompressedData.Add(Size);
		appMemcpy(&CompressedData(0),Buffer, Size);
	}

	const TArray<BYTE>& GetThumbnailData()
	{
		return ThumbnailData;
	}

#if JPEGCOMPRESSION
	void SetThumbnailTexture( FTexture2DRHIParamRef InThumbnailTexture )
	{
		ThumbnailTexture = InThumbnailTexture;
	}
#endif

	void SetThumbnailData( void* Buffer, INT Size )
	{
		ThumbnailData.Empty(Size);
		ThumbnailData.Add(Size);
		appMemcpy(&ThumbnailData(0),Buffer, Size);
	}

#if !JPEGCOMPRESSION
	void SetUncompressedData( void* BaseAddress, INT SizeX, INT SizeY, INT Stride )
	{
		check( SizeX*sizeof(FColor) == Stride );
		Width = SizeX;
		Height = SizeY;
		INT NumPixels = (Width * Height);
		PNG.InitRaw(BaseAddress, NumPixels * sizeof(FColor), Width, Height);
	}
#endif
};
#endif

/**
 * Makes sure alpha is 1 for the render target resource and copies the contents into the texture.
 */
static void CopyRendertargetToTexture(FRenderTarget* RenderTarget, UTexture2DDynamic* Texture)
{
	FBatchedElements BatchedElements;
	FLOAT MinX = -1.0f - GPixelCenterOffset / ((FLOAT)RenderTarget->GetSizeX() * 0.5f);
	FLOAT MaxX = +1.0f - GPixelCenterOffset / ((FLOAT)RenderTarget->GetSizeX() * 0.5f);
	FLOAT MinY = +1.0f + GPixelCenterOffset / ((FLOAT)RenderTarget->GetSizeY() * 0.5f);
	FLOAT MaxY = -1.0f + GPixelCenterOffset / ((FLOAT)RenderTarget->GetSizeY() * 0.5f);
	INT V00 = BatchedElements.AddVertex(FVector4(MinX,MinY,0,1),FVector2D(0,0),FLinearColor::Black,FHitProxyId());
	INT V10 = BatchedElements.AddVertex(FVector4(MaxX,MinY,0,1),FVector2D(1,0),FLinearColor::Black,FHitProxyId());
	INT V01 = BatchedElements.AddVertex(FVector4(MinX,MaxY,0,1),FVector2D(0,1),FLinearColor::Black,FHitProxyId());
	INT V11 = BatchedElements.AddVertex(FVector4(MaxX,MaxY,0,1),FVector2D(1,1),FLinearColor::Black,FHitProxyId());
	RHISetRenderTarget(RenderTarget->GetRenderTargetSurface(),FSurfaceRHIRef());	
	RHISetColorWriteMask(CW_ALPHA);
	BatchedElements.AddTriangle(V00,V10,V11,GWhiteTexture,BLEND_Opaque);
	BatchedElements.AddTriangle(V00,V11,V01,GWhiteTexture,BLEND_Opaque);
	BatchedElements.Draw(FMatrix::Identity,RenderTarget->GetSizeX(),RenderTarget->GetSizeY(),FALSE);
	RHISetColorWriteMask(CW_RGBA);

	FTexture2DDynamicResource* TextureResource = (FTexture2DDynamicResource*) Texture->Resource;
	RHICopyToResolveTarget(RenderTarget->GetRenderTargetSurface(),TRUE,FResolveParams(-1,-1,-1,-1,CubeFace_PosX,TextureResource->GetTexture2DRHI()));
}

/**
* Callback to allow game viewport to override the splitscreen settings
* @param NewSettings - settings to modify
* @param SplitScreenType - current splitscreen type being used
*/
void UGearGameViewportClient::OverrideSplitscreenSettings(FSystemSettingsData& SplitscreenSettings,ESplitScreenType SplitScreenType) const
{
	if( SplitScreenType != eSST_NONE )
	{
		AWorldInfo* WorldInfo = GWorld ? GWorld->GetWorldInfo() : NULL;
		if( WorldInfo->GRI != NULL && 
			WorldInfo->GRI->GameClass != NULL && 
			WorldInfo->GRI->GameClass->IsChildOf(AGearGameSP_Base::StaticClass()) )
		{
			// maintain high detail mode during single player to prevent unnecessary reattaching of components
			SplitscreenSettings.DetailMode = DM_High;
		}
	}
}

void UGearGameViewportClient::Draw(FViewport* Viewport,FCanvas* Canvas)
{
	AGearPC *PC = (GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) != NULL) ? Cast<AGearPC>(GEngine->GamePlayers(0)->Actor) : NULL;
	if (PC != NULL && PC->bRequestingShot)
	{
#if !PS3
		debugf(TEXT("Screenshot requested"));
		UTexture2DDynamic *TexTarget2D = PC->LastCapturedShot;
		UCanvas* CanvasObject = NULL;
		if (TexTarget2D != NULL)
		{
			GGameScreenshotCounter = 2;

			// create the view family for rendering the world scene to the viewport's render target
			PC->LastShotInfo.WorldTime = GWorld->GetTimeSeconds();
			FSceneViewFamilyContext RTViewFamily(
				Viewport,
				GWorld->Scene,
				ShowFlags,
				PC->LastShotInfo.WorldTime,
				GWorld->GetDeltaSeconds(),
				GWorld->GetRealTimeSeconds(), 
				NULL, 
				FALSE,
				TRUE,
				FALSE,
				FALSE,
				TRUE
				);

			ULocalPlayer* Player = GEngine->GamePlayers(0);
			if (Player->Actor)
			{
				FVector		ViewLocation;
				FRotator	ViewRotation;
				Player->CalcSceneView( &RTViewFamily, ViewLocation, ViewRotation, Viewport );
				BeginRenderingViewFamily(Canvas,&RTViewFamily);
			}
			CanvasObject = FindObject<UCanvas>(UObject::GetTransientPackage(),TEXT("CanvasObject"));
			if( !CanvasObject )
			{
				CanvasObject = ConstructObject<UCanvas>(UCanvas::StaticClass(),UObject::GetTransientPackage(),TEXT("CanvasObject"));
				CanvasObject->AddToRoot();
			}
			FCanvas TempCanvas(Viewport,NULL);
			CanvasObject->Canvas = &TempCanvas;	
			CanvasObject->SceneView = (FSceneView*)RTViewFamily.Views(0);
			TempCanvas.Flush();

			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				CopyRTToTexture,
				FRenderTarget*,RTResource,Viewport,
				UTexture2DDynamic*,Texture,TexTarget2D,
			{
				// Makes sure alpha is 1 for the render target resource and copies the contents into the texture.
				CopyRendertargetToTexture(RTResource,Texture);
			});

			FlushRenderingCommands();
 		}
		else
		{
			debugf(TEXT("Screenshot request with no LastCapturedShot!"));
		}
		PC->eventTookScreenshot(CanvasObject);
#endif
		PC->bRequestingShot = FALSE;
	}
	if(GGameScreenshotCounter > 0)
	{
		GGameScreenshotCounter--;
	}
	Super::Draw(Viewport,Canvas);
}

/** sets levels to the appropriate loaded/visible state from the passed in list of records */
static void UpdateLevelStatusFromLevelRecords(AWorldInfo* Info, const TArray<FLevelRecord>& LevelRecords)
{
	for (INT i = 0; i < LevelRecords.Num(); i++)
	{
		for (INT j = 0; j < Info->StreamingLevels.Num(); j++)
		{
			if (Info->StreamingLevels(j) != NULL && Info->StreamingLevels(j)->PackageName == LevelRecords(i).LevelName)
			{
				Info->StreamingLevels(j)->bShouldBeLoaded = LevelRecords(i).bShouldBeLoaded;
				Info->StreamingLevels(j)->bShouldBeVisible = LevelRecords(i).bShouldBeVisible;
				break;
			}
		}
	}
}

/** Threadsafe counter used to figure out whether async saving of checkpoint data has already completed. */
FThreadSafeCounter SaveCheckPointDataAsyncCounter;

/**
 * Blocks on async checkpoint data being written do the storage device. If requested, suspends the 
 * rendering thread while waiting.
 *
 * @param	bShouldSuspendRenderingThread	Whether to suspend rendering thread while blocking on I/O
 */
static void BlockOnAsyncCheckPointSave( UBOOL bShouldSuspendRenderingThread )
{
	// Only need to do any work like suspending rendering thread if we need to wait.
	if( SaveCheckPointDataAsyncCounter.GetValue() > 0 )
	{
		// Suspend rendering thread if wanted. At this point we know we will have to wait.
		if( bShouldSuspendRenderingThread )
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND( SuspendRendering, { extern UBOOL GGameThreadWantsToSuspendRendering; GGameThreadWantsToSuspendRendering = TRUE; } );
			FlushRenderingCommands();
		}
		// Block till saving checkpoint data has completed.
		while( SaveCheckPointDataAsyncCounter.GetValue() > 0 )
		{
			// Sleep for 100 ms waiting for async IO writing checkpoint data.
			appSleep(0.1);
		}
		// Resume rendering if it was suspended previously.
		if( bShouldSuspendRenderingThread )
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND( ResumeRendering, { extern UBOOL GGameThreadWantsToSuspendRendering; GGameThreadWantsToSuspendRendering = FALSE; RHIResumeRendering(); } );
		}
	}
}

#if XBOX
/**
 * Async helper class for saving checkpoint data to storage unit.
 */
class FAsyncCheckPointSave : public FAsyncWorkBase
{
public:
	/**
	 * Initializes the data used by thread worker and copies temporary data.
	 *
	 * @param	InDisplayName	Display name to use for save data
	 * @param	InFileName		Filename to use for save data
	 * @param	InDeviceID		ID of device to save to
	 * @param	InUserID		ID of user to save for
	 * @param	InData			Data to save
	 * @param	InNumBytes		Number of bytes to save
	 * @param	Counter			Thread-safe counter to decrement upon completion of work
	 */
	FAsyncCheckPointSave(
		const TCHAR* InDisplayName, 
		const TCHAR* InFileName, 
		DWORD InDeviceID, 
		DWORD InUserID, 
		BYTE* InData, 
		INT InNumBytes,
		FThreadSafeCounter* Counter 
	)
	: FAsyncWorkBase( Counter )
	, DisplayName(InDisplayName)
	, FileName(InFileName)
	, DeviceID(InDeviceID)
	, UserID(InUserID)
	, Data(NULL)
	, NumBytes(InNumBytes)
	{
		// Allocate memory for to be saved data and make a copy.
		Data = (BYTE*) appMalloc( NumBytes );
		appMemcpy( Data, InData, NumBytes );
	}

	/**
	 * Called after work has finished. Marks object as being done.
	 */ 
	void Dispose()
	{
		// Signal work as being completed by decrementing counter.
		if ( SaveCheckPointDataAsyncCounter.Decrement() == 0 )
		{
			// The queued work code won't access "this" after calling dispose so it's safe to delete here.
			delete this;
		}
	}

	/**
	 * Performs the async work
	 */
	virtual void DoWork()
	{
		XCONTENT_DATA ContentData = {0};
		lstrcpyW(ContentData.szDisplayName, *DisplayName);
		strcpy(ContentData.szFileName, "Gears2Checkpoint");
		ContentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
		ContentData.DeviceID = DeviceID;

		DWORD ContentCreateResult = XContentCreate(	
			UserID, 
			"savedrive", 
			&ContentData, 
			XCONTENTFLAG_OPENALWAYS | XCONTENTFLAG_NOPROFILE_TRANSFER, 
			NULL, 
			NULL, 
			NULL );

		if( ContentCreateResult == ERROR_SUCCESS )
		{
			// Create the file, write out the data and close the handle.
			HANDLE FileHandle = CreateFile( TCHAR_TO_ANSI(*FileName), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (FileHandle != INVALID_HANDLE_VALUE)
			{
				DWORD NumWritten = 0;
				WriteFile(FileHandle, Data, NumBytes, &NumWritten, NULL);
				CloseHandle(FileHandle);
				debugf(TEXT("Async write of %i bytes of checkpoint data completed."),NumWritten);
			}
			else
			{
				debugf(TEXT("FAsyncCheckPointSave::DoWork(%s): CreateFile failed. Error = 0x%08X"),
					*FileName, GetLastError());
			}
		}
		else if ( ContentCreateResult == ERROR_FILE_CORRUPT )
		{
			debugf(NAME_Gear_CheckpointSystem, TEXT("Failed to save checkpoint - data file corrupted.  Clearing data file and trying again."));

			INT DeleteResultCode=0;
			UGearEngine* GearEngine = CastChecked<UGearEngine>(GEngine);
			if ( GearEngine->DeleteCheckpoints(&DeleteResultCode) )
			{
				// increment our counter and try again next frame
				SaveCheckPointDataAsyncCounter.Increment();
				return;
			}
			else
			{
				debugf(NAME_Error, TEXT("Unable to save checkpoint! Data file corrupted and cannot be deleted...err = 0x%08X"), DeleteResultCode);
			}
		}
		else
		{
			debugf(TEXT("Failed to write checkpoint because the user isn't signed in or doesn't have a valid device...err = 0x%08X"), ContentCreateResult);
		}

		// Free up resources from associated call to XContentCreate.
		XContentClose("savedrive", NULL);

		// Last but not least, free the temporary data created in the constructor.
		appFree(Data);
	}

protected:
	/** Display name to use for save data.			*/
	FString DisplayName;
	/** Filename to use for save data.				*/
	FString FileName;
	/** ID of device to save to.					*/
	DWORD	DeviceID;
	/** ID of use to save for.						*/
	DWORD	UserID;
	/** Copy of data to write.						*/
	BYTE*	Data;
	/** Number of bytes to write.					*/
	DWORD	NumBytes;
};
#endif


UBOOL UGearEngine::LoadMap(const FURL& URL, UPendingLevel* Pending, FString& Error)
{
	// Block on any async checkpoint saving. The loading movie is already going to be up at this point so we don't
	// need to worry about suspending the rendering thread.
	BlockOnAsyncCheckPointSave(FALSE);

	FString GameTypeClassName( URL.GetOption(TEXT("Game="), TEXT("")) );
	if (GameTypeClassName == TEXT(""))
	{
		// ask the default gametype what we should use
		UClass* DefaultGameClass = UObject::StaticLoadClass(AGameInfo::StaticClass(), NULL, TEXT("game-ini:Engine.GameInfo.DefaultGame"), NULL, LOAD_None, NULL);
		if (DefaultGameClass != NULL)
		{
			FString Options(TEXT(""));
			for (INT i = 0; i < URL.Op.Num(); i++)
			{
				Options += TEXT("?");
				Options += URL.Op(i);
			}
			GameTypeClassName = DefaultGameClass->GetDefaultObject<AGameInfo>()->eventGetDefaultGameClassPath(URL.Map, Options, *URL.Portal);
		}
	}

	// discard checkpoint if we are not entering SP
	if (GameTypeClassName != TEXT("GearGameContent.GearGameSP"))
	{
		CurrentCheckpoint = NULL;
	}

	PendingCheckpointAction = Checkpoint_Default;

	UBOOL bResult = Super::LoadMap(URL, Pending, Error);
	// throw away ?chapter= and ?loadcheckpoint options if we're returning to the menus
	if (bResult && GWorld != NULL && GWorld->GetWorldInfo()->IsMenuLevel())
	{
		LastURL.RemoveOption(TEXT("chapter"));
		LastURL.RemoveOption(TEXT("loadcheckpoint"));
	}
	else if (bResult && Error == TEXT("") && GWorld != NULL && GWorld->GetNetMode() != NM_Client)
	{
		// don't waste time with initial level streaming if we're going to load a checkpoint as it will get overridden anyway
		if (PendingCheckpointAction == Checkpoint_Load)
		{
			//@warning: this code assumes that Checkpoint_Load is guaranteed to eventually call LoadCheckpoint() which will reset this override
			GWorld->AllowLevelLoadOverride = -1;
		}

		FString ChapterOption = URL.GetOption(TEXT("chapter="), TEXT(""));
		if (ChapterOption != TEXT(""))
		{
			EChapterPoint ChapterNum = EChapterPoint(appAtoi(*ChapterOption));
			// transition to the required level if necessary
			UGearTypes* DefaultGearTypes = UGearTypes::StaticClass()->GetDefaultObject<UGearTypes>();
			if (ChapterNum < DefaultGearTypes->ChapterLevelNames.Num())
			{
				FString DesiredLevelName = DefaultGearTypes->ChapterLevelNames(ChapterNum).ToString();
				if (URL.Map != DesiredLevelName)
				{
					TArray<FName> LevelNames;
					LevelNames.AddItem(DefaultGearTypes->ChapterLevelNames(ChapterNum));
					PrepareMapChange(LevelNames);
					FlushAsyncLoading();
					CommitMapChange(FALSE, FALSE);
				}
			}
			USequence* GameSeq = GWorld->GetGameSequence();
			if (GameSeq != NULL)
			{
				// trigger any chapter events with a matching chapter number
				UBOOL bFoundChapterEvent = FALSE;
				TArray<USequenceObject*> ChapterEvents;
				GameSeq->FindSeqObjectsByClass(USeqEvent_ChapterPoint::StaticClass(), ChapterEvents, TRUE);
				for (INT i = 0; i < ChapterEvents.Num(); i++)
				{
					USeqEvent_ChapterPoint* Event = Cast<USeqEvent_ChapterPoint>(ChapterEvents(i));
					if (Event != NULL && Event->Chapter == ChapterNum && Event->CheckActivate(GWorld->GetWorldInfo(), NULL, FALSE))
					{
						// wipe all level loaded/visible and level beginning events
						if (!bFoundChapterEvent)
						{
							TArray<USequenceObject*> LevelEvents;
							GameSeq->FindSeqObjectsByClass(USeqEvent_LevelBeginning::StaticClass(), LevelEvents, TRUE);
							GameSeq->FindSeqObjectsByClass(USeqEvent_LevelLoaded::StaticClass(), LevelEvents, TRUE);
							for (INT j = 0; j < LevelEvents.Num(); j++)
							{
								USequenceOp* Op = CastChecked<USequenceOp>(LevelEvents(j));
								if (Op->ParentSequence->ActiveSequenceOps.RemoveItem(Op) > 0)
								{
									for (INT k = 0; k < Op->InputLinks.Num(); k++)
									{
										Op->InputLinks(k).bHasImpulse = FALSE;
									}
									for (INT k = 0; k < Op->OutputLinks.Num(); k++)
									{
										Op->OutputLinks(k).bHasImpulse = FALSE;
									}
								}
							}
							bFoundChapterEvent = TRUE;
						}
						// don't waste time with initial level streaming if we're going to load a checkpoint as it will get overridden anyway
						if (PendingCheckpointAction != Checkpoint_Load)
						{
							UpdateLevelStatusFromLevelRecords(GWorld->GetWorldInfo(), Event->SubLevelsToLoad);
							GWorld->FlushLevelStreaming();
						}
					}
				}
			}
		}
	}

	return bResult;
}

static void LoadCheckpointLevels(FVector CheckpointLocation)
{
	AWorldInfo* Info = GWorld->GetWorldInfo();

	// clean up various subsystems to remove object references
	UTexture2D::CancelPendingTextureStreaming();
	GWorld->RBPhysScene->FinishLineChecks();
#if WITH_NOVODEX
	GWorld->RBPhysScene->AsyncLineCheckManager.CheckForFinishedLineChecks();
#endif	//#if WITH_NOVODEX
	// immediate stop any "dialog" sound cues in the audio device so they don't continue to play after loading
	UAudioDevice* AudioDevice = (GEngine->Client != NULL) ? GEngine->Client->GetAudioDevice() : NULL;
	if (AudioDevice != NULL)
	{
		static FName NAME_Dialog = FName(TEXT("Dialog"));
		static FName NAME_DialogNoDSP = FName(TEXT("DialogNoDSP"));
		for (INT i = 0; i < AudioDevice->AudioComponents.Num(); i++)
		{
			UAudioComponent* Comp = AudioDevice->AudioComponents(i);
			if ( Comp != NULL && Comp->SoundCue != NULL &&
				(Comp->SoundCue->SoundGroup == NAME_Dialog || Comp->SoundCue->SoundGroup == NAME_DialogNoDSP) )
			{
				Comp->Stop();
				i--;
			}
		}
	}
	// destroy any GearExplosionActors as they sometimes hold on to inline objects of Kismet actions which aren't marked pending kill correctly
	for (FActorIterator It; It; ++It)
	{
		if (It->IsA(AGearExplosionActor::StaticClass()))
		{
			GWorld->DestroyActor(*It);
		}
	}
	// Cancel any pending async map changes after flushing async loading. We flush async loading before canceling the map change
	// to avoid completion after cancelation to not leave references to the "to be changed to" level around. Async loading is
	// implicitly flushed again later on during garbage collection.
	UObject::FlushAsyncLoading();
	CastChecked<UGameEngine>(GEngine)->CancelPendingMapChange();
	AGearGame* Game = Cast<AGearGame>(Info->Game);
	if (Game != NULL && Game->AIVisMan != NULL)
	{
		Game->AIVisMan->Flush();
	}
	// force clear any rumble
	for (FPlayerIterator It(GEngine); It; ++It)
	{
		if (It->Actor != NULL && It->Actor->ForceFeedbackManager != NULL)
		{
			for (INT i = 0; i < 4; i++)
			{
				It->Actor->ForceFeedbackManager->ForceClearWaveformData(i);
			}
		}
	}

	// unload all dynamic levels
	debugf(TEXT("Unloading dynamic levels..."));
	for (INT LevelIndex = 0; LevelIndex < Info->StreamingLevels.Num(); LevelIndex++)
	{
		ULevelStreamingKismet* LevelStreaming = Cast<ULevelStreamingKismet>(Info->StreamingLevels(LevelIndex));
		if (LevelStreaming != NULL && !LevelStreaming->bIsFullyStatic)
		{
			// mark it to unload
			LevelStreaming->bShouldBeLoaded = FALSE;
			LevelStreaming->bShouldBeVisible = FALSE;

			// mark all players as not having the level loaded, so we don't replicate stuff in them to the client until it tells us that it reloaded them
			for (AController* C = Info->ControllerList; C != NULL; C = C->NextController)
			{
				AGearPC* PC = Cast<AGearPC>(C);
				if (PC != NULL)
				{
					PC->ServerUpdateLevelVisibility(LevelStreaming->PackageName, FALSE);
				}
			}
		}
	}
	GWorld->FlushLevelStreaming();
	UObject::CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, TRUE);

	// match up the level set based on the records
	debugf(TEXT("Setting up level streaming properties..."));
	INT OldLevelLoadOverride = GWorld->AllowLevelLoadOverride;
	GWorld->AllowLevelLoadOverride = 1;
	GWorld->DelayStreamingVolumeUpdates(0);
    GWorld->ProcessLevelStreamingVolumes(&CheckpointLocation);

	// clean up any levels that should now be unloaded
	debugf(TEXT("Unloading any static levels..."));
	GWorld->AllowLevelLoadOverride = -1;
	GWorld->UpdateLevelStreaming();
	GWorld->AllowLevelLoadOverride = OldLevelLoadOverride;
	UObject::CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, TRUE);

	// load up any levels that should now be loaded
	debugf(TEXT("Loading levels..."));
    GWorld->FlushLevelStreaming();
	debugf(TEXT("...finished streaming"));
}

/** @return whether our free memory is too low so we must force failure and a map reload to reset things */
UBOOL UGearEngine::NeedMemoryHack()
{
	if (FailureMemoryHackThreshold > 0)
	{
#if XBOX
		MEMORYSTATUS MemStat;
		MemStat.dwLength = sizeof(MemStat);
		GlobalMemoryStatus(&MemStat);

		return (MemStat.dwAvailPhys < DWORD(FailureMemoryHackThreshold));
#endif
	}
	return FALSE;
}

void UGearEngine::Tick(FLOAT DeltaSeconds)
{
	// check if we need to invoke our hack to make the player fail and reload :(
	// only trigger once every 15 minutes so we don't get into loops of brokenness
	static DOUBLE LastHackActivationTime = -100000.0;
	if (GWorld != NULL && GWorld->IsServer() && NeedMemoryHack() && appSeconds() - LastHackActivationTime > 1800.0 && !IsPreparingMapChange())
	{
		// don't do it if we're in a cinematic or the game is already over
		UBOOL bSkipHack = FALSE;
		for (AController* C = GWorld->GetFirstController(); C != NULL; C = C->NextController)
		{
			AGearPC* PC = Cast<AGearPC>(C);
			if (PC != NULL && (PC->bCinematicMode || PC->bInMatinee || (PC->MyGearHud != NULL && PC->MyGearHud->GameoverUISceneInstance != NULL)))
			{
				bSkipHack = TRUE;
			}
		}

		if (!bSkipHack)
		{
			LastHackActivationTime = appSeconds();
			debugf(TEXT("MEMORY HACK: Forcing game over screen"));
			for (AController* C = GWorld->GetFirstController(); C != NULL; C = C->NextController)
			{
				AGearPC* PC = Cast<AGearPC>(C);
				if (PC != NULL)
				{
					PC->eventClientGameOver();
				}
			}
		}
	}

	// check for a pending checkpoint action
	if (PendingCheckpointAction == Checkpoint_Save)
	{
		SaveCheckpoint(PendingCheckpointLocation);
	}
	else if (PendingCheckpointAction == Checkpoint_Load)
	{
		// on the client, just load the levels required by the checkpoint load location
		if (GWorld->GetWorldInfo()->NetMode == NM_Client)
		{
			LoadCheckpointLevels(PendingCheckpointLocation);
		}
		else
		{
			LoadCheckpoint();
		}
	}
	else if ( PendingCheckpointAction == Checkpoint_DeleteAll )
	{
		DeleteCheckpoints();
	}
	PendingCheckpointAction = Checkpoint_Default;


	// If we currently have levels pending load and the loading movie happens to be up, then go
	// ahead and block until async loads have finished.
	if( GWorld != NULL && GWorld->GetWorldInfo() != NULL )
	{
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();

		// First check for a pending map change
		UBOOL bLevelsHaveLoadRequestPending	= WorldInfo->IsPreparingMapChange();

		// Iterate over level collection to find out whether we need to load any levels.
		for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
		{
			ULevelStreaming* StreamingLevel	= WorldInfo->StreamingLevels(LevelIndex);
			if( StreamingLevel != NULL )
			{
				if( StreamingLevel->bHasLoadRequestPending )
				{
					bLevelsHaveLoadRequestPending = TRUE;
				}
			}
		}

		if( bLevelsHaveLoadRequestPending )
		{
			// If we currently have a Bink movie playing, go ahead and block on loads
			const UBOOL bIsLoadingMoviePlaying =
				( GFullScreenMovie != NULL && GFullScreenMovie->GameThreadIsMoviePlaying( UCONST_LOADING_MOVIE ) );
			if( bIsLoadingMoviePlaying )
			{
				debugf(NAME_DevMovie, TEXT("UGearEngine::Tick: Flushing async load due to level load request (movie is already up!)") );

				// Block till all async requests are finished.
				UObject::FlushAsyncLoading();

				debugf(NAME_DevMovie, TEXT("UGearEngine::Tick: Finished flushing async load due to level load request") );
			}
		}
	}


	// normal tick
	Super::Tick(DeltaSeconds);
}

UBOOL UGearEngine::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
	if( ParseCommand( &Cmd, TEXT("GUDSTEST") ) )
	{
		// First token is usually the pawn name.
		// Test action:							PawnName Action <Action#>
		// Load all variety banks:				PawnName LoadAll
		// Unload all variety banks:			PawnName UnloadAll
		// Load a random # of variety banks:	PawnName LoadRandom <Count> <optional NOCLEAR> (to not unload Count banks as well)
		// FlushAll variety banks:				FlushAll
		// 

		FString CmdCopy = Cmd;
		FString InPawnStr(ParseToken(Cmd, 0));

		CmdCopy = CmdCopy.Right(CmdCopy.Len() - InPawnStr.Len() - 1);
		TArray<FString> Tokens;
		while (CmdCopy.Len() > 0)
		{
			const TCHAR* LocalCmd = *CmdCopy;
			FString Token = ParseToken(LocalCmd, 0);
			Tokens.AddItem(Token);
			CmdCopy = CmdCopy.Right(CmdCopy.Len() - Token.Len() - 1);
		}

		AGearGame* LocalGearGame = Cast<AGearGame>(GWorld->GetGameInfo());
		if (LocalGearGame)
		{
			if (LocalGearGame->UnscriptedDialogueManager)
			{
				LocalGearGame->UnscriptedDialogueManager->RunTest(InPawnStr, Tokens);
			}
		}

		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT("GAMESPECIFIC_BUGIT") ) )
	{
		for( AController* Controller = GWorld->GetFirstController(); Controller != NULL; Controller = Controller->NextController )
		{
			// if it's a player
			if( Controller->IsA(AGearPC::StaticClass()) == TRUE )
			{
				const BYTE DiffLevel = Cast<AGearPC>(Controller)->eventGetCurrentDifficultyLevel();
				Ar.Logf( *FString::Printf( TEXT( "Difficulty Level: %d" ), DiffLevel ) );
			}
		}
	}
	else if( ParseCommand( &Cmd, TEXT("GAMESPECIFIC_BUGITAI") ) )
	{
		Ar.Logf(TEXT( "Attached AI Log Files:" ));

		FString FinalDir = ParseToken(Cmd,0);
		AGearAI* GAI = NULL;
		for( AController* Controller = GWorld->GetFirstController(); Controller != NULL; Controller = Controller->NextController )
		{
			// if it's an AI copy off the log
			GAI = Cast<AGearAI>(Controller);
			if(GAI != NULL && GAI->AILogFile != NULL)
			{
				//const FString OutputDir = appGameDir() + TEXT("Logs") PATH_SEPARATOR;;
				//const FString FullFileName = OutputDir + TxtFileName;
				INT SlashOffset = GAI->AILogFile->Filename.Len() - GAI->AILogFile->Filename.InStr(PATH_SEPARATOR,TRUE,TRUE)-1;
				INT Count = (SlashOffset < 42) ? SlashOffset : 42;
				const FString FinalFileName = FinalDir + GAI->AILogFile->Filename.Right(Count);
				Ar.Logf( *FString::Printf( TEXT( "AI: %s %s(%s) TimeSinceRender:%.2f" ),*(FinalFileName), *GAI->GetName(),(GAI->Pawn) ? *GAI->Pawn->GetName() : TEXT("NULL"),  (GAI->Pawn) ? GWorld->GetTimeSeconds() -  GAI->Pawn->LastRenderTime : -1.f));
				GAI->AILogFile->CloseFile();
				GFileManager->Copy(*(FinalFileName),*GAI->AILogFile->Filename);
				SendDataToPCViaUnrealConsole( TEXT("UE_PROFILER!BUGIT:"), *(FinalFileName) );
				GWorld->DestroyActor( GAI->AILogFile );
				GAI->AILogFile = NULL;
			}
		}
	}
	else if( ParseCommand( &Cmd, TEXT("GAMESPECIFIC_MEMORY") ) )
	{
		Exec( TEXT("GUDS_DETAILED_LOADEDPACKAGES"), Ar );
	}
	else if( ParseCommand( &Cmd, TEXT("GUDS_DETAILED_LOADEDPACKAGES_FILE") ) )
	{
		// Create archive for log data.
		const FString CreateDirectoriesString = CreateProfileDirectoryAndFilename( TEXT("GUDs"), TEXT(".GUDsPkgs") );

		FOutputDeviceFile FileAr(*CreateDirectoriesString);

		debugf(TEXT("GUDS_DETAILED_LOADEDPACKAGES_FILE: saving to %s"), *CreateDirectoriesString);

		Exec( TEXT( "GUDS_DETAILED_LOADEDPACKAGES" ), FileAr );

		FileAr.TearDown();
	}
	else if( ParseCommand( &Cmd, TEXT("GUDS_DETAILED_LOADEDPACKAGES") ) )
	{
		// paranoia 
		if( (GWorld != NULL) && (GWorld->GetWorldInfo() != NULL) )
		{
			const AGearGame* GearGame = Cast<AGearGame>(GWorld->GetWorldInfo()->Game);
			if( ( GearGame != NULL ) && ( GearGame->UnscriptedDialogueManager != NULL ) ) 
			{
				GearGame->UnscriptedDialogueManager->LogoutStreamedPackageData( Ar );
			}
		}
	}
	else if (ParseCommand(&Cmd,TEXT("LISTGEAREMITTERS")))
	{
		// Iterate over all gear emitter and keep track of how many times each particle system is referenced.
		TMap<FString,INT> SystemToInstanceCount;
		for( TObjectIterator<AGearEmitter> It; It; ++It )
		{
			AGearEmitter* Emitter = *It;
			if( Emitter->ParticleSystemComponent )
			{
				INT InstanceCount = SystemToInstanceCount.FindRef( *Emitter->ParticleSystemComponent->Template->GetPathName() );
				SystemToInstanceCount.Set( *Emitter->ParticleSystemComponent->Template->GetPathName(), ++InstanceCount );
			}
		}
		// Log usage stats.
		debugf(TEXT("GearEmitter usage by particle system:"));
		for( TMap<FString,INT>::TConstIterator It(SystemToInstanceCount); It; ++It )
		{
			debugf(TEXT("%s,%i"),*(It.Key()),It.Value());
		}
		return TRUE;
	}

	return Super::Exec(Cmd, Ar);
}

inline void CheckpointSerializeName(FArchive& Ar, FName& N)
{
	// we cannot safely use the name index as that is not guaranteed to persist across game sessions
	FString NameText;
	if (Ar.IsSaving())
	{
		NameText = N.GetNameString();
	}
	INT Number = N.GetNumber();

	Ar << NameText;
	Ar << Number;

	if (Ar.IsLoading())
	{
		// copy over the name with a name made from the name string and number
		N = FName(*NameText, Number);
	}
}

class FArchiveSerializeKismetSequenceWriter : public FArchiveSaveCompressedProxy
{
public:
	FArchiveSerializeKismetSequenceWriter(USequence *Seq, TArray<BYTE> &InBytes)
	: FArchiveSaveCompressedProxy(InBytes,(ECompressionFlags)(COMPRESS_LZO|COMPRESS_BiasSpeed))
	{
		BaseSequence = Seq;
		if (Seq != NULL)
		{
			*this << Seq;
		}
	}

	virtual FArchive& operator<<( class FName& N )
	{
		CheckpointSerializeName(*this, N);
		return *this;
	}

	FArchive& operator<<( UObject*& Obj )
	{
		// there is no point in serializing dynamic objects since the loading code only finds existing objects and therefore
		// won't find them unless we just happen to be in the same Kismet state
		// EXCEPTION: re-create and serialize duplicate events created using the "Attach To Event" action
		BYTE bCanRecreate = (Cast<USequenceEvent>(Obj) && Obj->GetArchetype()->GetOuter() == Obj->GetOuter());
		if (Obj != NULL && !Obj->HasAnyFlags(RF_Transient) && (Obj->GetNetIndex() != INDEX_NONE || bCanRecreate))
		{
			BYTE bSerializedPathName = 1;
			*this << bSerializedPathName;
			// serialize the object name so that we can just do a FindObject on loading the checkpoint
			FString ObjPathName = Obj->GetPathName();
			*this << ObjPathName;
			// if it's recreateable (a duplicated event), record that and the archetype
			*this << bCanRecreate;
			if (bCanRecreate)
			{
				UObject* Archetype = Obj->GetArchetype();
				*this << Archetype;
			}
// 			debugf(TEXT("- inspecting object %s"),*ObjPathName);
			if ( (Obj->IsA(USequenceObject::StaticClass()) || Obj->IsIn(BaseSequence)) &&
				!Obj->IsA(UInterpData::StaticClass()) ) // special case matinee data to save memory since they shouldn't be changing at runtime
			{
				// if it hasn't already been serialized
				if (!SerializedObjects.HasKey(Obj))
				{
// 					debugf(TEXT("-> serializing %s (%s)"),*Obj->GetName(),*Obj->GetPathName());
					SerializedObjects.AddItem(Obj);
					Obj->Serialize(*this);
				}
// 				else
// 				{
// 					debugf(TEXT("-> already serialized"));
// 				}
			}
// 			else
// 			{
// 				debugf(TEXT("-> skipping serialization of non-kismet object %s"),*Obj->GetName());
// 			}
		}
		else
		{
			// attempt to store dynamic Actors by Guid that the checkpoint code can save and restore to persist those refs
			AActor* Actor = Cast<AActor>(Obj);
			FGuid* ActorGuid = NULL;
			if (Actor != NULL)
			{
				ActorGuid = Actor->GetGuid();
			}
			if (ActorGuid != NULL && ActorGuid->IsValid())
			{
//				debugf(TEXT("- saving reference to %s via GUID"), *Obj->GetName());
				BYTE bSerializedPathName = 0;
				*this << bSerializedPathName;
				*this << (*ActorGuid);
			}
			else
			{
//				debugf(TEXT("- null ref"));
				BYTE bSerializedPathName = 1;
				*this << bSerializedPathName;
				FString NullRef(TEXT("None"));
				*this << NullRef;
				BYTE bCanCreate = 0;
				*this << bCanCreate;
			}
		}
		return *this;
	}

private:
	USequence *BaseSequence;
	/** List of objects that have already been serialized */
	TLookupMap<UObject*> SerializedObjects;
};

//@see FArchiveSerializeKismetSequenceReader::operator<<( UObject*& Obj )
struct FSavedActorHiddenState
{
public:
	AActor* Actor;
	UBOOL bHidden;
	FSavedActorHiddenState(AActor* InActor)
		: Actor(InActor), bHidden(InActor->bHidden)
	{}
	~FSavedActorHiddenState()
	{
		Actor->SetHidden(bHidden);
	}
};

IMPLEMENT_COMPARE_POINTER( USeqAct_Interp, GearGame,
{
	if (A->bIsPlaying)
	{
		return 1;
	}
	else if (B->bIsPlaying)
	{
		return -1;
	}
	else if (A->TerminationTime == 0.f)
	{
		return 1;
	}
	else if (B->TerminationTime == 0.f)
	{
		return -1;
	}
	else
	{
		return (B->TerminationTime > A->TerminationTime) ? -1 : 1;
	}
} )

class FArchiveSerializeKismetSequenceReader : public FArchiveLoadCompressedProxy
{
public:
	FArchiveSerializeKismetSequenceReader(USequence *Seq, TArray<BYTE> &InBytes)
	: FArchiveLoadCompressedProxy(InBytes,COMPRESS_LZO)
	{
		BaseSequence = Seq;
		if (Seq != NULL)
		{
			*this << Seq;

			Sort<USE_COMPARE_POINTER(USeqAct_Interp, GearGame)>(InterpsToReinit.GetData(), InterpsToReinit.Num());
			for (INT i = 0; i < InterpsToReinit.Num(); i++)
			{
				USeqAct_Interp* InterpAction = InterpsToReinit(i);
				if (InterpAction->bActive)
				{
					InterpAction->GroupInst.Empty();
					InterpAction->InitInterp();
					for (INT i = 0; i < InterpAction->LatentActors.Num(); i++)
					{
						if (InterpAction->LatentActors(i) != NULL)
						{
							InterpAction->LatentActors(i)->LatentActions.AddUniqueItem(InterpAction);
							InterpAction->LatentActors(i)->eventInterpolationStarted(InterpAction);
							// force physics update so the Actor gets moved to the correct position immediately
							if (InterpAction->LatentActors(i)->Physics == PHYS_Interpolating)
							{
								InterpAction->LatentActors(i)->performPhysics(1.0f);
							}
						}
					}
				}
				else if (InterpAction->Position > 0.0f)
				{
					//@hack: undo the effects of visibility tracks for matinees that have terminated
					// this is needed because the Actor records (containing any post-matinee vis changes) have already been applied
					// so any Toggle Hidden actions after the matinee will be clobbered
					// could reverse Actor record and Kismet serialization order,
					// but my instinct says that won't fix the issues, just change them
					TArray<FSavedActorHiddenState> HiddenStateList;
					{
						TArray<UObject**> ObjectVars;
						InterpAction->GetObjectVars(ObjectVars);
						for (INT i = 0; i < ObjectVars.Num(); i++)
						{
							AActor* Actor = Cast<AActor>(*(ObjectVars(i)));
							if (Actor != NULL)
							{
								new(HiddenStateList) FSavedActorHiddenState(Actor);
							}
						}
					}

					// call SetPosition() with the serialized position so attached Actors are updated
					//debugf(TEXT("Updating position of %s"), *InterpAction->GetName());
					InterpAction->SetPosition(InterpAction->Position);
				}
				if (!InterpAction->bClientSideOnly && (InterpAction->bActive || InterpAction->Position > 0.f) && InterpAction->ReplicatedActorClass != NULL)
				{
					//@HACK: evil evil ship hack to get around client not simulating this particular matinee correctly
					if ( InterpAction->GetOutermost()->GetFName() == FName(TEXT("SP_Palace_6")) &&
						( InterpAction->GetPathName() == TEXT("SP_Palace_6.TheWorld:PersistentLevel.Main_Sequence.Prefabs.SeqAct_Interp_3") ||
							InterpAction->GetPathName() == TEXT("SP_Palace_6.TheWorld:PersistentLevel.Main_Sequence.Floor_Pit_4.DOOR_DEVICE.SeqAct_Interp_0") ) )
					{
						continue;
					}

					if (InterpAction->ReplicatedActor == NULL)
					{
						InterpAction->ReplicatedActor = (AMatineeActor*)GWorld->SpawnActor(InterpAction->ReplicatedActorClass);
						InterpAction->ReplicatedActor->InterpAction = InterpAction;
					}
					InterpAction->ReplicatedActor->eventUpdate();
					//@HACK: workaround for weird issue with ReplicatedActor->InterpAction not getting serialized correctly on initial replication... some kind of mismatch?
					if (InterpAction->GetOutermost()->GetFName() == FName(TEXT("SP_Riftworm_3_S")))
					{
						InterpAction->ReplicatedActor->bForceNetUpdate = FALSE;
					}
				}
			}
		}
	}

	virtual FArchive& operator<<( class FName& N )
	{
		CheckpointSerializeName(*this, N);
		return *this;
	}

	FArchive& operator<<( UObject*& Obj )
	{
 		// determine whether we're using the path name or a GUID search
		BYTE bSerializedPathName = 0;
		*this << bSerializedPathName;
		if (bSerializedPathName)
		{
			// grab the path name
			FString ObjPathName;
			*this << ObjPathName;
//			debugf(TEXT("- pathname %s"),*ObjPathName);
			// search for the object
			Obj = UObject::StaticFindObject(UObject::StaticClass(),NULL,*ObjPathName,FALSE);
			BYTE bCanRecreate = 0;
			*this << bCanRecreate;
			if (bCanRecreate)
			{
				UObject* Archetype = NULL;
				*this << Archetype;
				if (Obj == NULL && Archetype != NULL)
				{
					FName DupName = FName(*ObjPathName.Right(ObjPathName.Len() - ObjPathName.InStr(TEXT("."), TRUE) - 1));
					Obj = UObject::StaticConstructObject(Archetype->GetClass(), Archetype->GetOuter(), DupName, 0, Archetype);
					//debugf(TEXT("- recreated from archetype %s"), *Archetype->GetPathName());
				}
			}
			if ( Obj != NULL && (Obj->IsA(USequenceObject::StaticClass()) || Obj->IsIn(BaseSequence)) &&
				!Obj->IsA(UInterpData::StaticClass()) ) // special case matinee data to save memory since they shouldn't be changing at runtime
			{
				if (!SerializedObjects.HasKey(Obj))
				{
					// remove this action from any LatentActions lists, as those will be regenerated after loading
					USeqAct_Latent* Action = Cast<USeqAct_Latent>(Obj);
					if (Action != NULL)
					{
						for (INT i = 0; i < Action->LatentActors.Num(); i++)
						{
							if (Action->LatentActors(i) != NULL)
							{
								Action->LatentActors(i)->LatentActions.RemoveItem(Action);
							}
						}
					}

					UClass *ObjClass = Obj->GetClass();
					UObject *ObjArchetype = Obj->GetArchetype();
					// initialize to defaults first
//					debugf(TEXT("- initializing default for %s from %s"),*Obj->GetPathName(),*ObjArchetype->GetPathName());
					Obj->SafeInitProperties((BYTE*)Obj,ObjClass->GetPropertiesSize(),ObjClass,(BYTE*)ObjArchetype,ObjArchetype->GetClass()->GetPropertiesSize());
					// make sure we don't serialize this object multiple times
					SerializedObjects.AddItem(Obj);
					// and finally serialize the object props
//					debugf(TEXT("-> serializing %s (%s)"),*Obj->GetName(),*Obj->GetPathName());
					Obj->Serialize(*this);
					// special case re-init matinee groups if they were active
					//@todo: maybe should have a virtual in SequenceAction... not sure that's necessary
					USeqAct_Interp* InterpAction = Cast<USeqAct_Interp>(Obj);
					if (InterpAction != NULL)
					{
						// deferred so we can sort based on termination time
						InterpsToReinit.AddItem(InterpAction);
					}
				}
//				else
//				{
//					debugf(TEXT("- skipping already serialized %s"),*Obj->GetName());
//				}
			}
			else if (Obj == NULL && ObjPathName != TEXT("None"))
			{
				debugf(NAME_Warning, TEXT("Checkpoint loading failed to find %s"), *ObjPathName);
			}
		}
		else
		{
			FGuid FindGuid;
			*this << FindGuid;
			// we only ever do this for references to dynamically spawned actors
			for (FDynamicActorIterator It; It; ++It)
			{
				FGuid* TestGuid = It->GetGuid();
				if (TestGuid != NULL && *TestGuid == FindGuid)
				{
					Obj = *It;
					//debugf(TEXT("- Loading reference to %s by GUID"), *Obj->GetName());
					break;
				}
			}
			if (Obj == NULL)
			{
				debugf(NAME_Warning, TEXT("Checkpoint loading failed to find dynamic Actor with GUID %s"), *FindGuid.String());
			}
		}
		return *this;
	}

private:
	USequence *BaseSequence;
	/** List of objects that have already been serialized */
	TLookupMap<UObject*> SerializedObjects;
	/** SeqAct_Interps we have serialized so we can reinit them afterwards based on execution order */
	TArray<USeqAct_Interp*> InterpsToReinit;
};

/** class for reading/writing checkpoint Actor records that handles names as strings */
class FCheckpointActorRecordWriter : public FMemoryWriter
{
public:
	FCheckpointActorRecordWriter(TArray<BYTE>& InBytes)
		: FMemoryWriter(InBytes)
	{}

	virtual FArchive& operator<<( class FName& N )
	{
		CheckpointSerializeName(*this, N);
		return *this;
	}
};
class FCheckpointActorRecordReader : public FMemoryReader
{
public:
	FCheckpointActorRecordReader(TArray<BYTE>& InBytes)
		: FMemoryReader(InBytes)
	{}

	virtual FArchive& operator<<( class FName& N )
	{
		CheckpointSerializeName(*this, N);
		return *this;
	}
};

/**
 * Initializes each Kismet sequence recursively, to fix up any issues post checkpoint load.
 */
static void InitializeSequencePostCheckpointLoad(USequence *Seq)
{
	if (Seq != NULL)
	{
		Seq->InitializeSequence();
		for (INT Idx = 0; Idx < Seq->NestedSequences.Num(); Idx++)
		{
			USequence *NestedSeq = Seq->NestedSequences(Idx);
			if (NestedSeq != NULL)
			{
				InitializeSequencePostCheckpointLoad(NestedSeq);
			}
		}
	}
}

static void LogSequences(USequence *BaseSeq, INT RecurseCount = 0)
{
	FString Indent = TEXT("");
	for (INT IndentCnt = 0; IndentCnt < RecurseCount; IndentCnt++)
	{
		Indent += TEXT("  ");
	}
	debugf(TEXT("%sSequence: %s, parent: %s, num objs: %d"),*Indent,*BaseSeq->GetPathName(),*BaseSeq->ParentSequence->GetName(),BaseSeq->SequenceObjects.Num());
	for (INT NestedIdx = 0; NestedIdx < BaseSeq->NestedSequences.Num(); NestedIdx++)
	{
		USequence *NestedSeq = BaseSeq->NestedSequences(NestedIdx);
		if (NestedSeq != NULL)
		{
			debugf(TEXT("%s- nested %d: %s, parent: %s"),*Indent,NestedIdx,*NestedSeq->GetPathName(),*NestedSeq->ParentSequence->GetName());
			LogSequences(NestedSeq,RecurseCount + 1);
		}
	}
}

static FArchive& operator<<(FArchive& Ar, FLevelRecord& LevelRecord)
{
	FString NameString = LevelRecord.LevelName.ToString();
	Ar << NameString;
	LevelRecord.LevelName = FName(*NameString);

	BYTE bLoaded = LevelRecord.bShouldBeLoaded;
	Ar << bLoaded;
	LevelRecord.bShouldBeLoaded = bLoaded;

	BYTE bVisible = LevelRecord.bShouldBeVisible;
	Ar << bVisible;
	LevelRecord.bShouldBeVisible = bVisible;

	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FActorRecord& ActorRecord)
{
	Ar << ActorRecord.actorName;
	Ar << ActorRecord.ActorClassPath;
	Ar << ActorRecord.RecordData;

	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FCheckpointTime& SaveTime)
{
	Ar << SaveTime.Year << SaveTime.Month << SaveTime.Day << SaveTime.SecondsSinceMidnight;
	return Ar;
}

/** get the current base "P" level name */
static FString GetBaseLevelName()
{
	AWorldInfo* Info = GWorld->GetWorldInfo();
	for (INT i = 0; i < Info->StreamingLevels.Num(); i++)
	{
		if (Cast<ULevelStreamingPersistent>(Info->StreamingLevels(i)) != NULL)
		{
			return Info->StreamingLevels(i)->PackageName.ToString();
		}
	}
	return GWorld->GetOutermost()->GetName();
}

void UCheckpoint::SerializeFinalData(FArchive& Ar)
{
	// version check
	// if you modify this function, increment this number
	const INT CHECKPOINT_VERSION = 1;

	INT DataVersion = CHECKPOINT_VERSION;
	Ar << DataVersion;
	// fail with an empty checkpoint object if the versions don't match
	if (DataVersion == CHECKPOINT_VERSION)
	{
		Ar << BaseLevelName;
		Ar << Chapter;
		Ar << Difficulty;
		Ar << SaveTime;
		Ar << CheckpointLocation;
		
		// we can't serialize the record arrays directly as on load that will create elements with random data which will crash on assigning the loaded data
		INT NumLevelRecords = LevelRecords.Num();
		Ar << NumLevelRecords;
		if (Ar.IsLoading())
		{
			LevelRecords.Empty(NumLevelRecords);
			LevelRecords.AddZeroed(NumLevelRecords);
		}
		for (INT i = 0; i < NumLevelRecords; i++)
		{
			Ar << LevelRecords(i);
		}
		INT NumActorRecords = ActorRecords.Num();
		Ar << NumActorRecords;
		if (Ar.IsLoading())
		{
			ActorRecords.Empty(NumActorRecords);
			ActorRecords.AddZeroed(NumActorRecords);
		}
		for (INT i = 0; i < NumActorRecords; i++)
		{
			Ar << ActorRecords(i);
		}

		Ar << KismetData;
	}
	else
	{
		debugf(NAME_Warning, TEXT("Corrupt checkpoint data (Slot %i)"), SlotIndex);
	}
}

void UCheckpoint::SaveData()
{
	if (GWorld->PersistentLevel->GameSequences.Num() == 0 || GWorld->PersistentLevel->GameSequences(0) == NULL)
	{
		debugf(NAME_Warning, TEXT("Can't save checkpoint because the level doesn't have any Kismet"));
		return;
	}

	debugf(TEXT("--- SAVING CHECKPOINT \"%s\" ---"), *GetBaseLevelName());
	DOUBLE StartTime = appSeconds();

	eventPreSaveCheckpoint();

	// we need to flush level streaming before continuing so that the level set on load is guaranteed to be the same
	GWorld->FlushLevelStreaming(NULL, FALSE);

	FName CheckpointFunctionName = FName(TEXT("CreateCheckpointRecord"));
	FName ShouldSaveCheckpointFunctionName = FName(TEXT("ShouldSaveForCheckpoint"));

	BaseLevelName = GetBaseLevelName();
	// clear out any previous data first
	KismetData.Reset();
	ActorRecords.Reset();
	LevelRecords.Reset();
	AWorldInfo* Info = GWorld->GetWorldInfo();
	// save the chapterpoint number
	Chapter = (Cast<AGearGameSP_Base>(Info->Game) != NULL) ? ((AGearGameSP_Base*)Info->Game)->CurrentChapter : 0;
	// and difficulty
	Difficulty = DL_Normal;
	for (FPlayerIterator It(GEngine); It; ++It)
	{
		if (It->Actor != NULL)
		{
			AGearPRI* PRI = Cast<AGearPRI>(It->Actor->PlayerReplicationInfo);
			if (PRI != NULL && PRI->Difficulty != NULL)
			{
				Difficulty = PRI->Difficulty->GetDefaultObject<UDifficultySettings>()->DifficultyLevel;
				break;
			}
		}
	}
	// and save time
	INT DayOfWeek, Hour, Min, Sec, MSec;
	appSystemTime(SaveTime.Year, SaveTime.Month, DayOfWeek, SaveTime.Day, Hour, Min, Sec, MSec);
	SaveTime.SecondsSinceMidnight = Hour * 3600 + Min * 60 + Sec;
	// record the level status
	for (INT i = 0; i < Info->StreamingLevels.Num(); i++)
	{
		if (Info->StreamingLevels(i) != NULL)
		{
			FLevelRecord& Record = LevelRecords(LevelRecords.AddZeroed(1));
			Record.LevelName = Info->StreamingLevels(i)->PackageName;
			Record.bShouldBeLoaded = Info->StreamingLevels(i)->bShouldBeLoaded;
			Record.bShouldBeVisible = Info->StreamingLevels(i)->bShouldBeVisible;
		}
	}
	LevelRecords.Shrink();
	// serialize the current kismet data
	//debugf(TEXT("Serializing kismet data..."));
	{
		FArchiveSerializeKismetSequenceWriter Arc(GWorld->PersistentLevel->GameSequences(0), KismetData);
	}
	KismetData.Shrink();
	// debugf(TEXT("...serialized %d bytes"),KismetData.Num());
	// iterate through all actors looking for instances that we need to record
	for (FActorIterator It; It; ++It)
	{
		AActor *Actor = *It;
		
		UBOOL bShouldRecord = FALSE;
		for (INT Idx = 0; Idx < ActorClassesToRecord.Num(); Idx++)
		{
			UClass* ActorClass = ActorClassesToRecord(Idx);
			if (ActorClass != NULL && Actor->IsA(ActorClass))
			{
				bShouldRecord = TRUE;
				break;
			}
		}

		if (bShouldRecord)
		{
// 			debugf(TEXT("Recording actor data for %s..."),*Actor->GetName());
// 			if (Actor->LatentActions.Num() > 0)
// 			{
// 				for (INT Idx = 0; Idx < Actor->LatentActions.Num(); Idx++)
// 				{
// 					debugf(TEXT("- latent action %d - %s"),Idx,*Actor->LatentActions(Idx)->GetPathName());
// 				}
// 			}
			// find the checkpoint record struct in that class
			UStruct *CheckpointStruct = FindField<UStruct>(Actor->GetClass(),TEXT("CheckpointRecord"));
			if (CheckpointStruct != NULL)
			{
				// find the function to see if we should save this function
				UFunction *Function = Actor->FindFunction(ShouldSaveCheckpointFunctionName);
				if (Function != NULL && Function->ParmsSize == 4) // 4 bytes is the min return size for padding
				{
// 					debugf(TEXT("- checking to see if actor allowed to save... %d"),Function->ParmsSize);
					INT ReturnValue = 0;
					// call the function and check to see if we should skip this actor based on the return value
					Actor->ProcessEvent(Function,&ReturnValue,NULL);
					if (ReturnValue == 0)
					{
// 						debugf(TEXT("- skipping save"));
						continue;
					}
				}
				// look for the function to call that fills in the record data
				Function = Actor->FindFunction(CheckpointFunctionName);
				if (Function != NULL)
				{
					// create a new record
					INT Idx = ActorRecords.AddZeroed();
					FActorRecord &Record = ActorRecords(Idx);
					Record.actorName = Actor->GetPathName();
					Record.ActorClassPath = Actor->GetClass()->GetPathName();
					// allocate space for the checkpoint record struct
					TArray<BYTE> BaseData;
					BaseData.AddZeroed(CheckpointStruct->GetPropertiesSize());
					// call the function to fill in the data
					Actor->ProcessEvent(Function, BaseData.GetData(), NULL);
					// serialize the struct into the final data so that e.g. strings are in there as their characters and not data pointers, etc
					{
						FCheckpointActorRecordWriter RawDataWriter(Record.RecordData);
						RawDataWriter.SetPortFlags(PPF_ForceBinarySerialization);
						CheckpointStruct->SerializeBin(RawDataWriter, BaseData.GetTypedData(), CheckpointStruct->GetPropertiesSize());
					}
					Record.RecordData.Shrink();
					// clean up constructed elements of the checkpoint record
					for (UProperty* P = Function->ConstructorLink; P; P = P->ConstructorLinkNext)
					{
						if ((P->GetClass()->ClassCastFlags & CASTCLASS_UStructProperty) && ((UStructProperty*)P)->Struct == CheckpointStruct)
						{
							P->DestroyValue(BaseData.GetData());
							break;
						}
					}
// 					debugf(TEXT("- recorded %d bytes"),Record.RecordData.Num());
				}
// 				else
// 				{
// 					debugf(TEXT("- failed to find checkpoint function"));
// 				}
			}
// 			else
// 			{
// 				debugf(TEXT("- failed to find checkpoint record struct"));
// 			}
		}
	}
	ActorRecords.Shrink();

	// pass TRUE for bIgnoreDeviceStatus because we're going to show the device selector UI if the user doesn't have a valid device.
	UGearEngine* Engine = Cast<UGearEngine>(GEngine);
	if ( Engine == NULL || !Engine->eventAreStorageWritesAllowed(TRUE) )
	{
		return;
	}

#if !CONSOLE
	//debugf(TEXT("Writing data to disk..."));

	FArchive* Ar = GFileManager->CreateFileWriter(*FString::Printf(TEXT("%sSaveData\\Gears2Checkpoint%i.sav"), *appGameDir(), SlotIndex));
	if (Ar != NULL)
	{
		SerializeFinalData(*Ar);

		if (!Ar->IsError())
		{
			debugf(TEXT("Successfully wrote %i bytes"), Ar->TotalSize());
		}
		else
		{
			debugf(NAME_Error, TEXT("Failed to write save data!"));
		}

		delete Ar;
	}
#elif XBOX
	//debugf(TEXT("Writing data to XBOX..."));

	// write out the final data to a memory buffer
	TArray<BYTE> FinalData;
	{
		FMemoryWriter Writer(FinalData);
		SerializeFinalData(Writer);
	} 

	// Block on previous checkpoint save. This could happen if you save checkpoints in rapid succession or make it through
	// the game and save to a very very slow storage device like e.g. saving to the network if that ever makes it in.
	BlockOnAsyncCheckPointSave( TRUE );

	XCONTENTDEVICEID	DeviceID	= static_cast<XCONTENTDEVICEID>(Engine->GetCurrentDeviceID());
	XOVERLAPPED			XOV			= {0};
	ULARGE_INTEGER		FileSize	= {0};
	FileSize.QuadPart				= XContentCalculateSize(FinalData.Num(), 1);

	// if the user has not selected a device, show the device selector;  if the user has already selected a device, verify that the device
	// is still valid.
	const UBOOL bHasValidDevice = Engine->eventIsCurrentDeviceValid();
	const UBOOL bDeviceHasSpace = bHasValidDevice && Engine->eventIsCurrentDeviceValid((DWORD)FileSize.QuadPart);
	const UBOOL bForceShowUI = (bHasValidDevice && !bDeviceHasSpace) || (!bHasValidDevice && Engine->bHasSelectedValidStorageDevice);

	if ( !bHasValidDevice || !bDeviceHasSpace )
	{
		DWORD ContentFlags = XCONTENTFLAG_MANAGESTORAGE;
		if ( bForceShowUI )
		{
			ContentFlags |= XCONTENTFLAG_FORCE_SHOW_UI;
		}

		HRESULT Result = XShowDeviceSelectorUI((DWORD)Engine->CurrentUserID, XCONTENTTYPE_SAVEDGAME, ContentFlags,
			FileSize, &DeviceID, &XOV);

		if ( Result == ERROR_IO_PENDING && !XHasOverlappedIoCompleted(&XOV) )
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND( SuspendRendering, { extern UBOOL GGameThreadWantsToSuspendRendering; GGameThreadWantsToSuspendRendering = TRUE; } );
			FlushRenderingCommands();
			while( !XHasOverlappedIoCompleted(&XOV) )
			{
				appSleep(0);
			}
			ENQUEUE_UNIQUE_RENDER_COMMAND( ResumeRendering, { extern UBOOL GGameThreadWantsToSuspendRendering; GGameThreadWantsToSuspendRendering = FALSE; RHIResumeRendering(); } );
		}

		// Cache device ID now that it is valid.
		Engine->SetCurrentDeviceID(static_cast<INT>(DeviceID));
	}

	if ( Engine->eventIsCurrentDeviceValid((DWORD)FileSize.QuadPart) )
	{
		// Incremented counter decremented in SaveCheckPointDataAsync upon completion of serialization.
		SaveCheckPointDataAsyncCounter.Increment();

		// Use an async worked to do the dirty work.
		GThreadPool->AddQueuedWork( new FAsyncCheckPointSave(
			*DisplayName,
			*FString::Printf(TEXT("savedrive:\\Gears2Checkpoint%i.sav"), SlotIndex),
			DeviceID,
			Engine->CurrentUserID,
			FinalData.GetData(),
			FinalData.Num(),
			&SaveCheckPointDataAsyncCounter ) );

		// Block on I/O if requested.
		if( ParseParam(appCmdLine(),TEXT("NOASYNCCHECKPOINTS")) )
		{
			BlockOnAsyncCheckPointSave( TRUE );
		}
	}
	else
	{
		debugf(TEXT("Failed to write checkpoint because the user doesn't have or didn't select a valid device"));
	}
#endif

	eventPostSaveCheckpoint();

	debugf(TEXT("--- CHECKPOINT SAVED [%4.2f seconds] ---"), appSeconds() - StartTime);
}

/** adds attachments with bJustTeleported == FALSE to the list and recurses. See @hack in checkpoint loading code */
static void AddNotTeleportedAttachments(AActor* CurrentActor, TArray<AActor*>& NotTeleportedActors)
{
	//@warning: not checking recursion (assume that check in SetBase() is working)
	for (INT i = 0; i < CurrentActor->Attached.Num(); i++)
	{
		if (CurrentActor->Attached(i) != NULL)
		{
			if (!CurrentActor->Attached(i)->bJustTeleported)
			{
				NotTeleportedActors.AddItem(CurrentActor->Attached(i));
			}
			AddNotTeleportedAttachments(CurrentActor->Attached(i), NotTeleportedActors);
		}
	}
}

void UCheckpoint::LoadData()
{
	DOUBLE StartTime = appSeconds();
	debugf(TEXT("--- LOADING CHECKPOINT \"%s\" ---"), *BaseLevelName);

	AWorldInfo* Info = GWorld->GetWorldInfo();

	UGameEngine* Engine = Cast<UGameEngine>(GEngine);
	if (Engine != NULL && Engine->IsPreparingMapChange())
	{
		debugf(TEXT("Map change was in progress, cancelling..."));
		for (AController* C = Info->ControllerList; C != NULL; C = C->NextController)
		{
			APlayerController* PC = C->GetAPlayerController();
			if (PC != NULL)
			{
				UNetConnection* Conn = Cast<UNetConnection>(PC->Player);
				if (Conn != NULL && Conn->GetUChildConnection() == NULL)
				{
					PC->eventClientCancelPendingMapChange();
					PC->eventClientSetBlockOnAsyncLoading(); // so that GC isn't blocked by async loading from old map change
					PC->eventClientForceGarbageCollection();
					Conn->FlushNet();
				}
			}
		}
		Engine->CancelPendingMapChange();
		// invoke GC here so we don't potentially have two map changes' worth of levels in memory
		UObject::CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, TRUE);
	}

	if (BaseLevelName != GetBaseLevelName())
	{
		debugf(TEXT("Checkpoint is from a different base level (%s), performing map change..."), *BaseLevelName);

		if (Engine != NULL)
		{
			TArray<FName> LevelNames;
			LevelNames.AddItem(FName(*BaseLevelName));
			// tell remote clients to do the map change first
			for (AController* C = Info->ControllerList; C != NULL; C = C->NextController)
			{
				APlayerController* PC = C->GetAPlayerController();
				if (PC != NULL)
				{
					// make sure the loading movie is up
					if (Cast<AGearPC>(PC) != NULL)
					{
						((AGearPC*)PC)->eventClientShowLoadingMovie(TRUE);
					}
					UNetConnection* Conn = Cast<UNetConnection>(PC->Player);
					if (Conn != NULL && Conn->GetUChildConnection() == NULL)
					{
						PC->eventClientPrepareMapChange(LevelNames(0), TRUE, TRUE);
						PC->eventClientCommitMapChange();
						Conn->FlushNet();
					}
				}
			}

			// now have server do it
			Engine->PrepareMapChange(LevelNames);
			FlushAsyncLoading();
			Engine->CommitMapChange(FALSE, FALSE);
		}
		else
		{
			debugf(TEXT("- Unable to perform map change in PIE, aborting"));
			return;
		}
	}

	eventPreLoadCheckpoint();

	FName CheckpointFunctionName = FName(TEXT("ApplyCheckpointRecord"));

	AGearGameSP_Base* SPGame = Cast<AGearGameSP_Base>(Info->Game);
	if (SPGame != NULL)
	{
		SPGame->bCheckpointLoadInProgress = TRUE;
	}

	// inform clients
	TArray<AGearPC*> ClientPCs;
	for (AController* C = Info->ControllerList; C != NULL; C = C->NextController)
	{
		AGearPC* PC = Cast<AGearPC>(C);
		if (PC != NULL)
		{
			PC->eventClientShowLoadingMovie(TRUE);
			UNetConnection* Conn = Cast<UNetConnection>(PC->Player);
			if (Conn != NULL && Conn->GetUChildConnection() == NULL)
			{
				ClientPCs.AddItem(PC);
				PC->eventClientLoadCheckpoint(CheckpointLocation);
				Conn->FlushNet();
			}
		}
	}

	LoadCheckpointLevels(CheckpointLocation);

	// override with the checkpoint data in case the streaming volumes are not enough
	//@todo: might need to replicate this
	debugf(TEXT("Updating level status from checkpoint records..."));
	UpdateLevelStatusFromLevelRecords(Info, LevelRecords);
	// clean up any levels that should now be unloaded
	INT OldLevelLoadOverride = GWorld->AllowLevelLoadOverride;
	GWorld->AllowLevelLoadOverride = -1;
	GWorld->UpdateLevelStreaming();
	GWorld->AllowLevelLoadOverride = OldLevelLoadOverride;
	UObject::CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, TRUE);
	appDefragmentTexturePool();
	// load up any levels that should now be loaded
    GWorld->FlushLevelStreaming();

	// make sure clients have the correct final level setup
	for (INT i = 0; i < ClientPCs.Num(); i++)
	{
		for (INT j = 0; j < Info->StreamingLevels.Num(); j++)
		{
			ULevelStreaming* TheLevel = Info->StreamingLevels(j);
			if (TheLevel != NULL)
			{
				ClientPCs(i)->eventLevelStreamingStatusChanged(TheLevel, TheLevel->bShouldBeLoaded, TheLevel->bShouldBeVisible, TheLevel->bShouldBlockOnLoad);
			}
		}
		ClientPCs(i)->eventClientFlushLevelStreaming();
		((UNetConnection*)ClientPCs(i)->Player)->FlushNet();
	}

	// first pass looks for any actors with records, and destroys any that don't have records that are referenced in ActorClassesToDestroy
	//@note - this needs to be 2 passes since applying a record may create actors which would then be destroyed
	TMap<INT,AActor*> ActorsWithRecords;
	for (FActorIterator ActorIter; ActorIter; ++ActorIter)
	{
		AActor* Actor = *ActorIter;
		FString ActorName = Actor->GetPathName();
		UBOOL bFoundRecord = FALSE;
		// only attempt to match to level placed Actors to maintain consistency
		// between reloading a checkpoint while in the level and loading one at startup from initial state
		if (Actor->GetNetIndex() != INDEX_NONE)
		{
			for (INT Idx = 0; Idx < ActorRecords.Num(); Idx++)
			{
				FActorRecord &Record = ActorRecords(Idx);
				if (Record.actorName == ActorName)
				{
					bFoundRecord = TRUE;
					ActorsWithRecords.Set(Idx,Actor);
					break;
				}
			}
		}
		// if a record wasn't found check to see if this is an actor that should be destroyed
		if (!bFoundRecord)
		{
			// see if it's exempt from destruction
			UBOOL bExemptFromDestruction = FALSE;
			for (INT DontDestroyIdx = 0; DontDestroyIdx < ActorClassesNotToDestroy.Num(); DontDestroyIdx++)
			{
				if(Actor->IsA(ActorClassesNotToDestroy(DontDestroyIdx)))
				{
					bExemptFromDestruction=TRUE;
					break;
				}
			}

			if(!bExemptFromDestruction)
			{
				for (INT DestroyIdx = 0; DestroyIdx < ActorClassesToDestroy.Num(); DestroyIdx++)
				{
					if (Actor->IsA(ActorClassesToDestroy(DestroyIdx)) && !Actor->bNoDelete)
					{
						debugf(TEXT("Destroying actor %s..."),*ActorName);
						GWorld->DestroyActor(Actor);
						break;
					}
				}
			}
		}
	}
	// perform garbage collection to clear out everything
	//UObject::CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	// create actors for all records that couldn't be matched up to existing actors
	for (INT Idx = 0; Idx < ActorRecords.Num(); Idx++)
	{
		if (!ActorsWithRecords.HasKey(Idx))
		{
			UClass* ActorClass = FindObject<UClass>(NULL, *ActorRecords(Idx).ActorClassPath);
			if (ActorClass == NULL)
			{
				debugf(NAME_Error, TEXT("Failed to find class %s expected by checkpoint loading of Actor records"), *ActorRecords(Idx).ActorClassPath);
			}
			else if (ActorClass->IsChildOf(APlayerController::StaticClass()))
			{
				// match to existing PlayerController instead of spawning one
				//@note: we use the last found here because that will be the first spawned,
				//		which will correctly match up the server to the record for player 1 and so on
				AController* LastFound = NULL;
				for (AController* C = Info->ControllerList; C != NULL; C = C->NextController)
				{
					if (C->IsA(ActorClass) && ActorsWithRecords.FindKey(C) == NULL)
					{
						LastFound = C;
					}
				}
				if (LastFound != NULL)
				{
					ActorsWithRecords.Set(Idx, LastFound);
				}
				else
				{
					debugf(TEXT("Failed to match a player record to an existing PlayerController - possibly co-op player that is no longer in the game"));
					if (ActorClass->IsChildOf(AGearPC::StaticClass()))
					{
						AGearPC* Dummy = (AGearPC*)GWorld->SpawnActor(ActorClass, NAME_None, FVector(0,0,0), FRotator(0,0,0), NULL, TRUE);
						if (Dummy != NULL)
						{
							Dummy->bCheckpointDummy = TRUE;
							ActorsWithRecords.Set(Idx, Dummy);
							debugf(TEXT(" - Successfully created checkpoint dummy GearPC to parse the data"));
						}
					}
				}
			}
			else
			{
				debugf(TEXT("Recreating actor %s..."),*ActorRecords(Idx).actorName);
				AActor *Actor = GWorld->SpawnActor(ActorClass, NAME_None, FVector(0,0,0), FRotator(0,0,0), NULL, TRUE);
				if (Actor != NULL)
				{
					// add it to the list of actors to apply records too
					ActorsWithRecords.Set(Idx,Actor);
				}
				else
				{
					debugf(TEXT("- failed to spawn"));
				}
			}
		}
	}

	// clear bJustTeleported on all actors
	// this allows checkpoint code to determine if other Actors have already loaded a location from the checkpoint data
	for (FDynamicActorIterator It; It; ++It)
	{
		It->bJustTeleported = FALSE;
	}

	// apply records for all the actors we found in the previous loop
	for (TMap<INT,AActor*>::TIterator Iter(ActorsWithRecords); Iter; ++Iter)
	{
		FActorRecord &Record = ActorRecords(Iter.Key());
		AActor *Actor = Iter.Value();
		debugf(TEXT("Applying record for %s..."),*Actor->GetPathName());
		// look for the function to call that process the record data
		UFunction *Function = Actor->FindFunction(CheckpointFunctionName);
		if (Function != NULL)
		{
			UScriptStruct* CheckpointStruct = FindField<UScriptStruct>(Actor->GetClass(), TEXT("CheckpointRecord"));
			if (CheckpointStruct != NULL)
			{
				if (Function->ParmsSize == Align(CheckpointStruct->GetPropertiesSize(), CheckpointStruct->GetMinAlignment()))
				{
					//@hack: attachment chains have ordering issues with the bJustTeleported check as the attached move will set it on the entire chain
					//		which a middle attachment in the chain might check and think it shouldn't move the deeper attachment again when it should
					TArray<AActor*> RemoveJustTeleportedActors;
					if (Actor->IsA(AInterpActor::StaticClass()))
					{
						AddNotTeleportedAttachments(Actor, RemoveJustTeleportedActors);
					}

					// serialize the recorded data into the function's parameter
					TArray<BYTE> EventData;
					EventData.AddZeroed(Function->ParmsSize);
					{
						FCheckpointActorRecordReader Reader(Record.RecordData);
						Reader.SetPortFlags(PPF_ForceBinarySerialization);
						CheckpointStruct->SerializeBin(Reader, EventData.GetTypedData(), Function->ParmsSize);
					}
					Actor->ProcessEvent(Function, EventData.GetData(), NULL);
					// clean up constructed elements of the checkpoint record
					for (UProperty* P = Function->ConstructorLink; P; P = P->ConstructorLinkNext)
					{
						if ((P->GetClass()->ClassCastFlags & CASTCLASS_UStructProperty) && ((UStructProperty*)P)->Struct == CheckpointStruct)
						{
							P->DestroyValue(EventData.GetData());
							break;
						}
					}

					for (INT i = 0; i < RemoveJustTeleportedActors.Num(); i++)
					{
						RemoveJustTeleportedActors(i)->bJustTeleported = FALSE;
					}
				}
				else
				{
					debugf(TEXT("- size mismatch in record data vs function parms (%i vs %i)"), INT(Function->ParmsSize), Align(CheckpointStruct->GetPropertiesSize(), CheckpointStruct->GetMinAlignment()));
				}
			}
			else
			{
				debugf(TEXT("- failed to find checkpoint struct"));
			}
		}
		else
		{
			debugf(TEXT("- failed to find checkpoint function"));
		}
	}
	debugf(TEXT("Applying kismet data..."));
	void* LogFilePtr = GWorld->PersistentLevel->GameSequences(0)->LogFile;
	FArchiveSerializeKismetSequenceReader Arc(GWorld->PersistentLevel->GameSequences(0),KismetData);
	// fixup the parenting for all the sequences
	GWorld->PersistentLevel->GameSequences(0)->ParentSequence = NULL;
	// and make sure all events are properly registered, etc etc
	GWorld->PersistentLevel->GameSequences(0)->LogFile = LogFilePtr;
#if !CONSOLE && !NO_LOGGING && !FINAL_RELEASE
	GWorld->PersistentLevel->GameSequences(0)->ScriptLogf(TEXT("*** LOADED CHECKPOINT ***"));
#endif
	InitializeSequencePostCheckpointLoad(GWorld->PersistentLevel->GameSequences(0));

	// delay streaming volumes for one frame so Kismet can do teleports and such
	// (particularly relevant for chapter start point saves)
	GWorld->DelayStreamingVolumeUpdates(1);

	if (SPGame != NULL)
	{
		SPGame->bCheckpointLoadInProgress = FALSE;
		SPGame->eventCheckpointLoadComplete();
	}

	eventPostLoadCheckpoint();

	debugf(TEXT("--- CHECKPOINT LOADED [%4.2f seconds] ---"), appSeconds() - StartTime);
}

/**
 * Accessor for setting the value of CurrentDeviceID
 *
 * @param	NewDeviceID			the new value to set CurrentDeviceID to
 * @param	bProfileSignedOut	Controls whether the previous value of CurrentDeviceID is considered; specify TRUE when setting
 *								CurrentDeviceID to an invalid value as a result of a profile being signed out and
 *								bHasSelectedValidStorageDevice will be reset to FALSE.
 */
void UGearEngine::SetCurrentDeviceID( INT NewDeviceID, UBOOL bProfileSignedOut/*=FALSE*/ )
{
	CurrentDeviceID = NewDeviceID;
	if ( eventIsCurrentDeviceValid() )
	{
		bHasSelectedValidStorageDevice = TRUE;
	}
	else if ( bProfileSignedOut )
	{
		bHasSelectedValidStorageDevice = FALSE;
	}
	debugf(NAME_Gear_CheckpointSystem, TEXT("UGearEngine::SetCurrentDeviceID - NewDeviceId: %i  bProfileSignedOut:%i  bHasSelectedValidStorageDevice:%i  HasValidDevice:%s"),
		NewDeviceID, bProfileSignedOut, bHasSelectedValidStorageDevice, eventIsCurrentDeviceValid() ? GTrue : GFalse);
}

/**
 * Accessor for getting the value of CurrentDeviceID
 */
INT UGearEngine::GetCurrentDeviceID() const
{
	return CurrentDeviceID;
}

/**
 * Wrapper for checking whether a previously selected storage device has been removed.
 *
 * @return	TRUE if the currently selected device is invalid but bHasSelectedValidStorageDevice is TRUE.
 */
UBOOL UGearEngine::HasStorageDeviceBeenRemoved() const
{
	UBOOL bResult = FALSE;

	if ( bHasSelectedValidStorageDevice && !const_cast<UGearEngine*>(this)->eventIsCurrentDeviceValid() )
	{
		bResult = TRUE;
	}

	return bResult;
}

void UGearEngine::SaveCheckpoint(FVector CheckpointLocation)
{
	// create a new one if necessary
	if (CurrentCheckpoint == NULL)
	{
		CurrentCheckpoint = ConstructObject<UCheckpoint>(UCheckpoint::StaticClass(),this,NAME_None);
	}
	check(CurrentCheckpoint != NULL);
	// and save the data
	CurrentCheckpoint->CheckpointLocation = CheckpointLocation;
	CurrentCheckpoint->SaveData();

#if !FINAL_RELEASE
	if( ParseParam( appCmdLine(), TEXT("CHECKPOINTMEMLEAKCHECK") ) )
	{
		new(DeferredCommands) FString( TEXT( "MemLeakCheckPostGC" ) );
	}
#endif
}

/** loads the desired checkpoint slot from disk/memory card/etc and stores it in CurrentCheckpoint for later use */
UBOOL UGearEngine::FindCheckpointData( INT SlotIndex, FCheckpointEnumerationResult* EnumResult/*=NULL*/ )
{
	// force GC to clear out any previous checkpoint objects
	if (GWorld != NULL)
	{
		GWorld->GetWorldInfo()->ForceGarbageCollection();
	}

	UCheckpoint* Checkpoint = ConstructObject<UCheckpoint>(UCheckpoint::StaticClass(), this, NAME_None);
	const UBOOL bVerifyOnly = SlotIndex == INDEX_NONE && EnumResult != NULL;

	debugf(NAME_Gear_CheckpointSystem, TEXT("UGearEngine::FindCheckpointData - CurrentCheckpoint(%s)  PotentialCheckpoint(%s)  SlotIndex:%i  bVerifyOnly:%s  CurrentDeviceID:%i"),
		*CurrentCheckpoint->GetName(), *Checkpoint->GetName(), SlotIndex, bVerifyOnly ? GTrue : GFalse, CurrentDeviceID);

	if ( !bVerifyOnly )
	{
		// always create a new object, even for empty slots
		CurrentCheckpoint = Checkpoint;
	}

	if ( SlotIndex != INDEX_NONE )
	{
		Checkpoint->SlotIndex = SlotIndex;
	}

	UBOOL bResult = TRUE;

#if !CONSOLE
	debugf(TEXT("Reading checkpoint data from disk..."));

	if ( SlotIndex != INDEX_NONE )
	{
		FArchive* Ar = GFileManager->CreateFileReader(*FString::Printf(TEXT("%sSaveData\\Gears2Checkpoint%i.sav"), *appGameDir(), SlotIndex));
		if (Ar != NULL)
		{
			Checkpoint->SerializeFinalData(*Ar);

			if (!Ar->IsError())
			{
				debugf(TEXT("Successfully read %i bytes"), Ar->TotalSize());
			}
			else
			{
				debugf(NAME_Error, TEXT("Failed to read save data!"));
				if ( !bVerifyOnly )
				{
					CurrentCheckpoint = NULL;
				}
				bResult = FALSE;
			}

			delete Ar;
		}
		else
		{
			debugf(NAME_Error, TEXT("Failed to find checkpoint file"));
		}
	}
	else
	{
		bResult = FALSE;
	}

#elif XBOX

	// figure out the device to save on
	XCONTENTDEVICEID DeviceID = static_cast<XCONTENTDEVICEID>(GetCurrentDeviceID());
	ULARGE_INTEGER FileSize = {0};
	XOVERLAPPED XOV = {0};

	// if the user has not selected a device, show the device selector;  if the user has already selected a device, verify that the device
	// is still valid.
	if ( !eventIsCurrentDeviceValid() )
	{
		HRESULT Result = XShowDeviceSelectorUI( DWORD(CurrentUserID), XCONTENTTYPE_SAVEDGAME, XCONTENTFLAG_NONE,  
			FileSize, &DeviceID, &XOV );

		if ( Result == ERROR_IO_PENDING && !XHasOverlappedIoCompleted(&XOV) )
		{
			// block with suspended rendering while we wait for the user to choose a device
			ENQUEUE_UNIQUE_RENDER_COMMAND( SuspendRendering, { extern UBOOL GGameThreadWantsToSuspendRendering; GGameThreadWantsToSuspendRendering = TRUE; } );
			FlushRenderingCommands();
			while (!XHasOverlappedIoCompleted(&XOV))
			{
				appSleep(0);
			}
			ENQUEUE_UNIQUE_RENDER_COMMAND( ResumeRendering, { extern UBOOL GGameThreadWantsToSuspendRendering; GGameThreadWantsToSuspendRendering = FALSE; RHIResumeRendering(); } );
		}

		CurrentDeviceID = (INT)DeviceID;
	}

	DeviceID = static_cast<XCONTENTDEVICEID>(GetCurrentDeviceID());
	INT FirstIndex = SlotIndex, LastIndex = SlotIndex+1;
	if ( SlotIndex == INDEX_NONE && EnumResult != NULL )
	{
		FirstIndex = 0;
		LastIndex = eGEARCAMPMEMORYSLOT_MAX;
	}

	debugf(TEXT("Reading checkpoint data from XBOX (CurrentDeviceId:%d)..."), DWORD(CurrentDeviceID));

/*
	no enumeration necessary because we only have 1 savegame file
	if ( false )
	{
		DWORD SizeNeeded=0;
		HANDLE EnumHandle;
		// try enumeration
		HRESULT CreateEnumeratorResult = XContentCreateEnumerator(
			(DWORD)CurrentUserID,
			DeviceID,
			XCONTENTTYPE_SAVEDGAME,
			0,
			eGEARCAMPMEMORYSLOT_MAX,
			&SizeNeeded,
			&EnumHandle
			);

		debugf(TEXT("))))))))))))  XContentCreateEnumerator result: 0x%08X"), CreateEnumeratorResult);
		if ( CreateEnumeratorResult == ERROR_SUCCESS )
		{
			PXCONTENT_DATA pAllContentData = (PXCONTENT_DATA)appAlloca(SizeNeeded);
			appMemzero(pAllContentData, SizeNeeded);

			DWORD NumFound=0;
			HRESULT EnumeratorResult = XEnumerate(EnumHandle, pAllContentData, SizeNeeded, &NumFound, NULL);

			debugf(TEXT("===============  XEnumerateResult: 0x%08X   NumFound:%d"), EnumeratorResult, NumFound);
			if ( EnumeratorResult == ERROR_SUCCESS )
			{
				for ( DWORD i = 0; i < NumFound; i++ )
				{
					FString DisplayName(pAllContentData[i].szDisplayName);
					FString Filename(pAllContentData[i].szFileName);
					debugf(TEXT("%i)  DisplayName:%s    Filename:%s"), i, *DisplayName, *Filename);
				}
			}

			CloseHandle(EnumHandle);
		}
	}
*/

	// DeviceID <= zero means that the player hasn't selected a device or that there are no valid devices attached - calling XContentGetDeviceState()
	// using one of these invalid device IDs will trigger assertion in the live code.
	if (eventIsCurrentDeviceValid())
	{
		XCONTENT_DATA ContentData = { 0 };
		lstrcpyW(ContentData.szDisplayName, *Checkpoint->DisplayName);
		strcpy(ContentData.szFileName, "Gears2Checkpoint");
		ContentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
		ContentData.DeviceID = DeviceID;

		DWORD CreateResult = XContentCreate(DWORD(CurrentUserID), "savedrive", &ContentData, XCONTENTFLAG_OPENEXISTING | XCONTENTFLAG_NOPROFILE_TRANSFER, NULL, NULL, NULL);
		if ( CreateResult == ERROR_SUCCESS )
		{
			BOOL bUserIsCreator = FALSE;
			// See if this player created it
			XContentGetCreator(DWORD(CurrentUserID), &ContentData, &bUserIsCreator, NULL, NULL);
			if (!bUserIsCreator)
			{
				debugf(NAME_Error, TEXT("Savegame was created by a different user"));
				bResult = FALSE;
			}
			else
			{
				// read in the data to a memory buffer
				TArray<BYTE> FinalData;
				for ( INT CheckIndex = FirstIndex; CheckIndex < LastIndex; CheckIndex++ )
				{
					HANDLE Handle = CreateFile(TCHAR_TO_ANSI(*FString::Printf(TEXT("savedrive:\\Gears2Checkpoint%i.sav"), CheckIndex)), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (Handle != INVALID_HANDLE_VALUE)
					{
						INT FileSize = GetFileSize(Handle, NULL);
						FinalData.Empty(FileSize);
						FinalData.Add(FileSize);

						DWORD NumRead = 0;
						if ( ReadFile(Handle, FinalData.GetData(), FinalData.Num(), &NumRead, NULL) )
						{
							// serialize the data into a checkpoint object
							FMemoryReader Reader(FinalData);
							Checkpoint->SerializeFinalData(Reader);

							if ( Reader.IsError() )
							{
								debugf(NAME_Error, TEXT("Failed to read checkpoint file"));
								if ( !bVerifyOnly )
								{
									CurrentCheckpoint = NULL;
								}

								if (EnumResult)
								{
									EnumResult->bCheckpointFileCorrupted[CheckIndex] = TRUE;
								}
								bResult = FALSE;
							}
							else if ( bVerifyOnly )
							{
								EnumResult->bCheckpointFileExists[CheckIndex]		= TRUE;
								EnumResult->bCheckpointFileContainsData[CheckIndex]	= !Checkpoint->eventCheckpointIsEmpty();
								EnumResult->CheckpointTimestamp[CheckIndex]			= Checkpoint->SaveTime;
								EnumResult->CheckpointDifficulty[CheckIndex]		= Checkpoint->Difficulty;
								EnumResult->CheckpointChapter[CheckIndex]			= Checkpoint->Chapter;
							}
						}
						else
						{
							debugf(NAME_Error, TEXT("Failed to read checkpoint file"));
						}

						CloseHandle(Handle);
					}
					else
					{
						debugf(TEXT("File not found for checkpoint slot %i"), CheckIndex);
					}
				}
			}
		}
		else if ( CreateResult == ERROR_ACCESS_DENIED )
		{
			debugf(TEXT("Content was created by a different user"));
			bResult = FALSE;
		}
		else
		{
			debugf(NAME_Gear_CheckpointSystem, TEXT("Failed to find or open checkpoint file (err:0x%08X -  detailed info needed:%s)"), CreateResult, EnumResult != NULL ? GTrue : GFalse);
			if ( CreateResult == ERROR_FILE_CORRUPT )
			{
				if ( EnumResult != NULL )
				{
					for ( INT CheckIndex = FirstIndex; CheckIndex < LastIndex; CheckIndex++ )
					{
						EnumResult->bCheckpointFileExists[FirstIndex] = TRUE;
						EnumResult->bCheckpointFileContainsData[FirstIndex] = TRUE;
						EnumResult->bCheckpointFileCorrupted[FirstIndex] = TRUE;
					}
				}
			}
			// If they don't exist...
			else if ( CreateResult == ERROR_PATH_NOT_FOUND || CreateResult == ERROR_FILE_NOT_FOUND )
			{
				if ( EnumResult != NULL )
				{
					for ( INT CheckIndex = FirstIndex; CheckIndex < LastIndex; CheckIndex++ )
					{
						EnumResult->bCheckpointFileExists[FirstIndex] = FALSE;
						EnumResult->bCheckpointFileContainsData[FirstIndex] = FALSE;
						EnumResult->bCheckpointFileCorrupted[FirstIndex] = FALSE;
					}
				}
			}
			CurrentCheckpoint = NULL;

			bResult = FALSE;
		}

		XContentClose("savedrive", NULL);
	}
	else
	{
		debugf(TEXT("Failed to write checkpoint because the user doesn't have or didn't select a valid device"));
	}
#endif

	return bResult;
}

/**
 * Takes the current checkpoint and applies it.
 */
void UGearEngine::LoadCheckpoint()
{
	// reset level load override as it might have been set by LoadMap()
	GWorld->AllowLevelLoadOverride = 0;

	if (CurrentCheckpoint == NULL)
	{
		// load checkpoint from disk
		FindCheckpointData(0);
	}
	if (CurrentCheckpoint != NULL)
	{
		CurrentCheckpoint->LoadData();
	}
}

/**
 * Deletes the save game containing all checkpoints.  This function is triggered by setting PendingCheckpointAction to Checkpoint_DeleteAll.
 */
UBOOL UGearEngine::DeleteCheckpoints( INT* ResultCode/*=NULL*/ )
{
	UBOOL bResult = FALSE;

#if XBOX
	// DeviceID <= zero means that the player hasn't selected a device or that there are no valid devices attached - calling XContentGetDeviceState()
	// using one of these invalid device IDs will trigger assertion in the live code.
	if ( eventIsCurrentDeviceValid() )
	{
		XCONTENT_DATA ContentData = { 0 };
		lstrcpyW(ContentData.szDisplayName, *UCheckpoint::StaticClass()->GetDefaultObject<UCheckpoint>()->DisplayName);
		strcpy(ContentData.szFileName, "Gears2Checkpoint");
		ContentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
		ContentData.DeviceID = static_cast<XCONTENTDEVICEID>(CurrentDeviceID);

		DWORD DeleteResult = XContentDelete((DWORD)CurrentUserID, &ContentData, NULL);
		if ( DeleteResult == ERROR_SUCCESS )
		{
			CurrentCheckpoint = NULL;
			bResult = TRUE;
		}
		else
		{
			debugf(NAME_Gear_CheckpointSystem, TEXT("Failed to delete save-game data file! err: 0x%08X"), DeleteResult);
		}

		if ( ResultCode != NULL )
		{
			*ResultCode = (INT)DeleteResult;
		}
	}
	else
	{
		debugf(TEXT("Failed to delete save-game data file because the user doesn't have or didn't select a valid device"));
	}
#endif


	return bResult;
}


// Get the localized string
FString UGearObjectiveManager::RetrieveObjectiveString( const FString& TagName )
{
	return Localize( TEXT("OBJECTIVES"), *TagName, TEXT("geargame") );
}

void UGearCoverMeshComponent::UpdateMeshes()
{
	// if this in-game,
	if (GIsGame)
	{
		// replace all the normal references with our gameplay versions
		Meshes.Empty();
		Meshes.AddZeroed(GameplayMeshRefs.Num());
		for (INT Idx = 0; Idx < GameplayMeshRefs.Num(); Idx++)
		{
			Meshes(Idx) = GameplayMeshRefs(Idx);
		}
		AutoAdjustOn = GameplayAutoAdjustOn;
		AutoAdjustOff = GameplayAutoAdjustOff;
		Disabled = GameplayDisabled;
	}
}

void UGearStatsObject::BeginGameplaySession()
{
	if (!GIsEditor)
	{
		if (!bGameplaySessionInProgress)
		{
			FString MapName = CastChecked<UGameEngine>(GEngine)->LastURL.Map;
			debugf(TEXT("Beginning new gameplay session for %s"),*MapName);
			bGameplaySessionInProgress = TRUE;
			// record the current time, map, etc
			GameplaySessionStartTime = appSystemTimeString();
			// create the unique ID
			GameplaySessionID = appCreateGuid();
			// clear any previous events
			PlayerList.Empty(32);
			EventNames.Empty(32);
			EventDescList.Empty(128);
			GameplayEvents.Empty(1024);
			PlayerEvents.Empty(1024);
			EstimatedStatBytes = 0;

			// create a null player
			PlayerList.AddZeroed();
			PlayerList(0).ControllerName = TEXT("Game Events");
			PlayerList(0).PlayerName = TEXT("Game");
			// log the initial gameplay event
			//@STATS
			FString StartTime = FString::Printf(TEXT("%.4f"),GWorld->GetTimeSeconds());
			LogGameplayEvent(0x01,FName(TEXT("SessionBegun")),NULL,StartTime);
		}
		else
		{
			debugf(NAME_Warning,TEXT("Attempting to start a session that is already in progress"));
		}
	}
}

INT UGearStatsObject::ResolvePlayerIndex(AController *Player)
{
	INT PlayerIdx = INDEX_NONE;
	if (Player != NULL && Player->PlayerReplicationInfo != NULL)
	{
		QWORD SearchXuid = Player->PlayerReplicationInfo->UniqueId.Uid;
		// look for an existing entry
		for (INT Idx = 0; Idx < PlayerList.Num(); Idx++)
		{
			if (PlayerList(Idx).ControllerName == Player->GetName() ||
				(SearchXuid != 0 && PlayerList(Idx).UniqueId.Uid == SearchXuid) ||
				(!Player->PlayerReplicationInfo->bBot && PlayerList(PlayerIdx).PlayerName == Player->PlayerReplicationInfo->PlayerName))
			{
				PlayerIdx = Idx;
				PlayerList(PlayerIdx).PlayerName = Player->PlayerReplicationInfo->PlayerName;
				break;
			}
		}
		// if none found,
		if (PlayerIdx == INDEX_NONE)
		{
			// add an entry
			PlayerIdx = PlayerList.AddZeroed();
			PlayerList(PlayerIdx).ControllerName = Player->GetName();
			PlayerList(PlayerIdx).PlayerName = Player->PlayerReplicationInfo->PlayerName;
			PlayerList(PlayerIdx).bIsBot = Player->PlayerReplicationInfo->bBot;

			// THIS IS A HACK to detect the Meatflag and make sure we use the proper ID.  We look to see if the ControllerName
			// is set to the MeatFlag type and if it is, call GetMeatFlagUniqueId() to get it otherwise, just use the value in the PRI

			if (Player->PlayerReplicationInfo->bBot && PlayerList(PlayerIdx).ControllerName.Left(16) == "GearAI_CTMVictim" )
			{
				AGearGameMP_Base* MPGame = Cast<AGearGameMP_Base>(GWorld->GetWorldInfo()->Game);
				if (MPGame != NULL)
				{
					PlayerList(PlayerIdx).UniqueId = MPGame->GetMeatflagUniqueId();
				}
				else
				{
					Player->PlayerReplicationInfo->UniqueId;
				}
				
			}
			else
			{
				PlayerList(PlayerIdx).UniqueId = Player->PlayerReplicationInfo->UniqueId;
			}

			//@STATS

			PlayerList(PlayerIdx).LastPlayerEventIdx = -1;

			// debugf(TEXT("### Adding Player [%s] to PlayerList @ index %i"),*Player->PlayerReplicationInfo->PlayerName,PlayerIdx);
		}
	}
	else
	{
		// no player so use the null index
		PlayerIdx = 0;
	}
	return PlayerIdx;
}

void UGearStatsObject::LogGameplayEvent(INT StatsMask, FName EventName, AController *Player, const FString &Desc, AController *TargetPlayer, FVector EventLocation)
{
	if ( !(GWorld && GWorld->GetWorldInfo()->IsMenuLevel()) )
	{
		if (bGameplaySessionInProgress)
		{
			// If this is a horde event we do not want, exit.
			if (bUglyHordeHackFlag)
			{
				// Exit if we are ignoring this event
				for (INT EIdx=0;EIdx<HordeIgnoreEvents.Num();EIdx++)
				{
					if ( EventName == HordeIgnoreEvents(EIdx) )
					{
						return;
					}
				}
				// Exit if this is an AI Controller regardless of the event
				if (Cast<AAIController>(Player) != NULL)
				{
					return;
				}

			}

			if ( (StatsMask & AllowedStatsMask) != 0 )
			{
				// only log if we have not exceeded the stat size limit (150k)
				if (EstimatedStatBytes < 1024 * 150)
				{
					// we only support WORD indices, so stop logging when we approach those limits
					if ((EventNames.Num() < 0xff) // 256 max event names
						&& (EventDescList.Num() < 0x7fff)
						&& (PlayerEvents.Num() < 0xfffe)
						&& (PlayerList.Num() < 0x7fff))
					{
						const INT PlayerNum = PlayerList.Num();
						const INT PlayerIdx = ResolvePlayerIndex(Player);
						const INT TargetPlayerIdx = ResolvePlayerIndex(TargetPlayer);
						
						// did we add new player entries?
						if (PlayerList.Num() > PlayerNum)
						{
							EstimatedStatBytes += (PlayerList.Num() - PlayerNum) * sizeof(FPlayerInformation);
							for (INT NewIndex = PlayerNum; NewIndex < PlayerList.Num(); ++NewIndex)
							{
								const FPlayerInformation& NewPlayer = PlayerList(NewIndex);
								EstimatedStatBytes += NewPlayer.ControllerName.Len() * sizeof(TCHAR);
								EstimatedStatBytes += NewPlayer.PlayerName.Len() * sizeof(TCHAR);
							}
						}

						FRotator EventRotation(EC_EventParm);
						//debugf(TEXT("Player %s/%d, Target %s/%d [%s][%s]"),Player!=NULL?*Player->PlayerReplicationInfo->PlayerName:TEXT("None"),PlayerIdx,TargetPlayer!=NULL?*TargetPlayer->PlayerReplicationInfo->PlayerName:TEXT("None"),TargetPlayerIdx,*EventName.GetNameString(),*Desc);
						if (Player != NULL)
						{
							// auto-grab the current player location if not specified
							if (EventLocation.IsNearlyZero())
							{
								EventLocation = Player->Pawn != NULL ? Player->Pawn->Location : Player->Location;
							}
							// grab the rotation based off the pawn if available
							EventRotation = Player->Pawn != NULL ? Player->Pawn->Rotation : Player->Rotation;

							// we do not want to store over-wound rotations, since we will only store the low 16bits of each component
							EventRotation.MakeShortestRoute();
							check(EventRotation.Yaw >= -32768 && EventRotation.Yaw <= 32768);
							check(EventRotation.Pitch >= -32768 && EventRotation.Pitch <= 32768);
							check(EventRotation.Roll >= -32768 && EventRotation.Roll <= 32768);
						}

						// consider this a new player event if it's the default player (game) or if the event name matches
			
						UBOOL bNewPlayerEvent = PlayerIdx == 0 || EventName == FName(TEXT("PlayerStatsUpdate")) || EventName == FName(TEXT("PlayerLogin"));
			
						//@STATS
						// Look to see if the current player event for this player is beyond the threshold.
			
						if (!bNewPlayerEvent)
						{
							INT EventIdx = PlayerList(PlayerIdx).LastPlayerEventIdx;
							//debugf(TEXT("Testing for expired events %i %i %i %f-%f=%f"),EventIdx,PlayerEvents(EventIdx).PlayerIdx,PlayerIdx,GWorld->GetTimeSeconds(),PlayerEvents(EventIdx).EventTime,GWorld->GetTimeSeconds() - PlayerEvents(EventIdx).EventTime);
							if ( EventIdx < 0 || GWorld->GetTimeSeconds() - PlayerEvents(EventIdx).EventTime > 1.0 )
							{
								//debugf(TEXT(" -- EXPIRED!"));
								bNewPlayerEvent = true;
							}
						}
			
						// if it's a new player event create a new chunk
						if (bNewPlayerEvent)
						{
							// setup the new PlayerEvents entry
							INT Idx = PlayerEvents.AddZeroed();
							EstimatedStatBytes += sizeof(FPlayerEvent);
							FPlayerEvent &PlayerEvent = PlayerEvents(Idx);

							PlayerEvent.EventTime = GWorld->GetTimeSeconds();
							PlayerEvent.EventLocation = EventLocation;
							PlayerEvent.PlayerIndexAndYaw = ((PlayerIdx & 0xffff) << 16) + (EventRotation.Yaw & 0xffff);
							PlayerEvent.PlayerPitchAndRoll = ((EventRotation.Pitch & 0xffff) << 16) + (EventRotation.Roll & 0xffff);

							PlayerList(PlayerIdx).LastPlayerEventIdx = Idx;
							//debugf(TEXT("Created new player event %d for %s [%d]"),Idx,*PlayerList(PlayerIdx).PlayerName,PlayerIdx);
						}

						// resolve the events array for the given player for saving the event
						const INT PlayerEventIdx = PlayerList(PlayerIdx).LastPlayerEventIdx;
						
						// handle the name and description strings
						const INT NumEventNames = EventNames.Num();
						const INT NumEventDescriptions = EventDescList.Num();
						const INT EventNameIdx = EventNames.AddUniqueItem(EventName);
						const INT EventDescIndex = EventDescList.AddUniqueItem(Desc);

						// We enfoce one BYTE per EventNameIdx when later uploading to the LSP server.
						check(EventNameIdx < 256);

						// did we add new entries we need to account for in the size estimate?
						if (EventNames.Num() > NumEventNames)
						{
							EstimatedStatBytes += sizeof(FName);
						}
						if (EventDescList.Num() > NumEventDescriptions)
						{
							EstimatedStatBytes += Desc.Len() * sizeof(TCHAR);
						}

						// add a new event to the list
						INT Idx = GameplayEvents.AddZeroed();
						EstimatedStatBytes += sizeof(FGameplayEvent);
						GameplayEvents(Idx).EventNameAndDesc = ((EventNameIdx & 0xffff)<<16) + (EventDescIndex & 0xffff);
						GameplayEvents(Idx).PlayerEventAndTarget = ((PlayerEventIdx & 0xffff)<<16) + (TargetPlayerIdx & 0xffff);
						//debugf(TEXT("- Added gameplay event %s at %d to player %s [%d]"),EventName.GetName(),Idx,*PlayerList(PlayerIdx).PlayerName,PlayerIdx);
					}
				}
			}
		}
		else if (!GIsEditor)
		{
			debugf(NAME_Warning,TEXT("Attempting to log a gameplay event w/o a valid session: %s"),*EventName.GetNameString());
		}
	}
}

void UGearStatsObject::DumpGameplayStats()
{
#if !FINAL_RELEASE
	static INT FileNum = 0;
	const FString MapName = CastChecked<UGameEngine>(GEngine)->LastURL.Map.Left(8); // restricting the map name to 8 chars for brevity
	const FString FileNumber = appItoa( FileNum );
	const FString ShortDate = GameplaySessionStartTime.Right(GameplaySessionStartTime.Len() - 5); // dropping the year for brevity

	++FileNum;
	
	// default logging of stats data
	{
		// open a file writer
		FString FileName = appGameDir() + TEXT("Stats") PATH_SEPARATOR + MapName + TEXT("_") + ShortDate + TEXT("_raw_") + FileNumber + TEXT(".log");
		FArchive *Ar = GFileManager->CreateFileWriter(*FileName);

		// dump the raw version in a csv happy format
		if (Ar != NULL)
		{
			// write each event
			for (INT EvtIdx = 0; EvtIdx < GameplayEvents.Num(); EvtIdx++)
			{
				const FGameplayEvent &Evt = GameplayEvents(EvtIdx);
				const INT EventNameIdx = Evt.EventNameAndDesc >> 16;
				const INT EventDescIdx = Evt.EventNameAndDesc & 0xffff;
				
				const INT PlayerEvtIdx = Evt.PlayerEventAndTarget >> 16;
				const FPlayerEvent &PlayerEvt = PlayerEvents(PlayerEvtIdx);
				const FLOAT EventTime = PlayerEvt.EventTime;
				const FVector EventLocation = PlayerEvt.EventLocation;
				const INT PlayerIdx = PlayerEvt.PlayerIndexAndYaw >> 16;
				const FString PlayerName = PlayerList(PlayerIdx).PlayerName;

				Ar->Logf(TEXT("%s,%2.2f,%s,%s,%s,%2.2f,%2.2f,%2.2f"),*GameplaySessionID.String(),EventTime,*EventNames(EventNameIdx).GetNameString(),*EventDescList(EventDescIdx),*PlayerName,EventLocation.X,EventLocation.Y,EventLocation.Z);
			}

			Ar->Close();
			debugf(TEXT("--- RAW STATS SAVED ---"));
		}
	}

	// also log stats about the stats :)
	{
		// open a file writer
		FString FileName = appGameDir() + TEXT("Stats") PATH_SEPARATOR + MapName + TEXT("_") + ShortDate + TEXT("_mem_") + FileNumber + TEXT(".log");
		FArchive *Ar = GFileManager->CreateFileWriter(*FileName);

		// dump the mem version in a csv happy format
		if (Ar != NULL)
		{
			Ar->Logf(TEXT("Array, Num, Slack, Bytes Used, Bytes Unused"));

			Ar->Logf(TEXT("Player Events, %i, %i, %i, %i"), PlayerEvents.Num(), PlayerEvents.GetSlack(), PlayerEvents.Num() * PlayerEvents.GetTypeSize(), PlayerEvents.GetSlack() * PlayerEvents.GetTypeSize());
			Ar->Logf(TEXT("Gameplay Events, %i, %i, %i, %i"), GameplayEvents.Num(), GameplayEvents.GetSlack(), GameplayEvents.Num() * GameplayEvents.GetTypeSize(), GameplayEvents.GetSlack() * GameplayEvents.GetTypeSize());
			Ar->Logf(TEXT("Player List, %i, %i, %i, %i"), PlayerList.Num(), PlayerList.GetSlack(), PlayerList.Num() * PlayerList.GetTypeSize(), PlayerList.GetSlack() * PlayerList.GetTypeSize());
			Ar->Logf(TEXT("Event Names, %i, %i, %i, %i"), EventNames.Num(), EventNames.GetSlack(), EventNames.Num() * EventNames.GetTypeSize(), EventNames.GetSlack() * EventNames.GetTypeSize());
			Ar->Logf(TEXT("Event Desc, %i, %i, %i, %i"), EventDescList.Num(), EventDescList.GetSlack(), EventDescList.Num() * EventDescList.GetTypeSize(), EventDescList.GetSlack() * EventDescList.GetTypeSize());
			
			DWORD Num = 0;
			DWORD Slack = 0;
			DWORD BytesUsed = 0;
			DWORD BytesUnused = 0;

			// string overhead
			Num = 0;
			Slack = 0;
			BytesUsed = 0;
			BytesUnused = 0;

			for (INT i = 0; i < PlayerList.Num(); i++)
			{
				FPlayerInformation &PlayerInformation = PlayerList(i);

				Num += PlayerInformation.PlayerName.Len();
				//Slack += PlayerInformation.PlayerName.GetSlack();
				BytesUsed += PlayerInformation.PlayerName.Len() * sizeof(TCHAR);
				//BytesUnused += PlayerInformation.PlayerName.GetSlack() * PlayerInformation.PlayerName.GetTypeSize();
			}

			Ar->Logf(TEXT("Player Strings, %i, %i, %i, %i"), Num, Slack, BytesUsed, BytesUnused);

			Num = 0;
			Slack = 0;
			BytesUsed = 0;
			BytesUnused = 0;

			for (INT i = 0; i < PlayerList.Num(); i++)
			{
				FPlayerInformation &PlayerInformation = PlayerList(i);

				Num += PlayerInformation.ControllerName.Len();
				//Slack += PlayerInformation.ControllerName.GetSlack();
				BytesUsed += PlayerInformation.ControllerName.Len() * sizeof(TCHAR);
				//BytesUnused += PlayerInformation.ControllerName.GetSlack() * PlayerInformation.ControllerName.GetTypeSize();
			}

			Ar->Logf(TEXT("Controller Strings, %i, %i, %i, %i"), Num, Slack, BytesUsed, BytesUnused);

			Num = 0;
			Slack = 0;
			BytesUsed = 0;
			BytesUnused = 0;

			for (INT i = 0; i < EventDescList.Num(); i++)
			{
				FString &String = EventDescList(i);

				Num += String.Len();
				//Slack += String.GetSlack();
				BytesUsed += String.Len() * sizeof(TCHAR);
				//BytesUnused += String.GetSlack() * String.GetTypeSize();
			}

			Ar->Logf(TEXT("Descriptions Strings, %i, %i, %i, %i"), Num, Slack, BytesUsed, BytesUnused);

			Ar->Logf(TEXT("EstimatedStatBytes, %i"), EstimatedStatBytes);

			Ar->Close();
			debugf(TEXT("--- MEM STATS SAVED ---"));
		}
	}
#endif
}



void UGearStatsObject::EndGameplaySession()
{
	if (!GIsEditor)
	{
		if (bGameplaySessionInProgress)
		{
			FString MapName = CastChecked<UGameEngine>(GEngine)->LastURL.Map;
			FString EndTime = FString::Printf(TEXT("%.4f"),GWorld->GetTimeSeconds());
			debugf(TEXT("Ended gameplay session"));
			// log the closing gameplay event
			LogGameplayEvent(0x01, FName(TEXT("SessionEnded")),NULL,EndTime);
			bGameplaySessionInProgress = FALSE;

			DumpGameplayStats();
		}
		else
		{
			debugf(NAME_Warning,TEXT("Attempting to end a session that hasn't been started"));
		}
	}
}

/** This will send all of the GameEvents to the DB **/
void UGearStatsObject::SendGameEventsToDB()
{
	extern INT GScreenWidth;
	extern INT GScreenHeight;

	// start the "Run"
	const FString BeginRun = FString::Printf( TEXT("EXEC BeginRun @Date='%s', @PlatformName='%s', @MachineName='%s', @UserName='%s', @Changelist='%d', @GameName='%s', @ResolutionName='%s', @ConfigName='%s', @CmdLine='%s', @GameType='%s', @LevelName='%s', @TaskDescription='%s', @TaskParameter='%s', @Tag='%s'")
		, appTimestamp() // need to pass in the date for when the compile was done  format:  03/18/08 20:35:48
		, *appGetPlatformString()
		, appComputerName()
		, appUserName() 
		, GetChangeListNumberForPerfTesting()
		, appGetGameName()
		, *FString::Printf(TEXT("%dx%d"), GScreenWidth, GScreenHeight ) // resolution is:  width by height
		, *GetConfigName()
		, appCmdLine()
		, *this->GetName()
		, *CastChecked<UGameEngine>(GEngine)->LastURL.Map
		//	BeginSentinelRun( "TravelTheWorld", "TaskParameter", "TagDesc" );
		, TEXT("GamePlayStats")
		, TEXT("SessionBegun")
		, GWorld ? *GWorld->GetGameInfo()->SentinelTagDesc : TEXT("")
		);

	warnf( TEXT("%s"), *BeginRun );

	FDataBaseRecordSet* RecordSet = NULL;
	if( GTaskPerfMemDatabase->SendExecCommandRecordSet( *BeginRun, RecordSet ) && RecordSet )
	{
		// Retrieve RunID from recordset. It's the return value of the EXEC.
		GearsStatsRunID = RecordSet->GetInt(TEXT("Return Value"));
		//warnf( TEXT("RunID: %d"), GearsStatsRunID );
	}

	delete RecordSet;
	RecordSet = NULL;


	if( GearsStatsRunID == -1 )
	{
		warnf( TEXT( "Was unable to start a GearStatsData Run!  No data will be submitted to the DB." ) );
		return;
	}

	// write each event
	for (INT EvtIdx = 0; EvtIdx < GameplayEvents.Num(); EvtIdx++)
	{
		const FGameplayEvent &Evt = GameplayEvents(EvtIdx);
		const INT EventNameIdx = Evt.EventNameAndDesc >> 16;
		const INT EventDescIdx = Evt.EventNameAndDesc & 0xffff;
		const INT PlayerEvtIdx = Evt.PlayerEventAndTarget >> 16;
		const INT TargetPlayerIdx = Evt.PlayerEventAndTarget & 0xffff;
		
		const FPlayerEvent &PlayerEvt = PlayerEvents(PlayerEvtIdx);
		const FLOAT EventTime = PlayerEvt.EventTime;
		const FVector EventLocation = PlayerEvt.EventLocation;
		const INT PlayerIdx = PlayerEvt.PlayerIndexAndYaw >> 16;
		const INT PlayerYaw = PlayerEvt.PlayerIndexAndYaw & 0xffff;
		const INT PlayerPitch = PlayerEvt.PlayerPitchAndRoll >> 16;
		const INT PlayerRoll = PlayerEvt.PlayerPitchAndRoll & 0xffff;
		const FString PlayerName = PlayerList(PlayerIdx).PlayerName;

		const FString AddGearStatsData = FString::Printf(TEXT("EXEC AddGameEventData @RunID=%i, @SecondsFromStart=%5.2f, @PlayerName='%s', @TargetPlayerName='%s', @GameEventName='%s', @GameEventDesc='%s', @LocX=%d, @LocY=%d, @LocZ=%d, @RotYaw=%d, @RotPitch=%d, @RotRoll=%d" )
			, GearsStatsRunID
			, EventTime

			, *PlayerName
			, *(PlayerList(TargetPlayerIdx).PlayerName)

			, *EventNames(EventNameIdx).GetNameString()
			, *EventDescList(EventDescIdx)

			, appTrunc(EventLocation.X)
			, appTrunc(EventLocation.Y)				
			, appTrunc(EventLocation.Z)
			, PlayerYaw
			, PlayerPitch
			, PlayerRoll
			);

			//warnf( TEXT("%s"), *AddGearStatsData );

			GTaskPerfMemDatabase->SendExecCommand( *AddGearStatsData );
	}

	// end the "Run"
	const FString EndRun = FString::Printf(TEXT("EXEC EndRun @RunID=%i, @ResultDescription='%s'")
		, GearsStatsRunID
		, *PerfMemRunResultStrings[ARR_Passed] 
	);

	//warnf( TEXT("%s"), *EndRun );
	GTaskPerfMemDatabase->SendExecCommand( *EndRun );
}

UBOOL AGearPC::WantsLedgeCheck()
{
	if( Cast<AGearPawn_LocustBrumakBase>(Pawn) != NULL )
	{
		return FALSE;
	}
	else if ( Pawn && Pawn->Base && Pawn->Base->IsA(AInterpActor_COGDerrickBase::StaticClass()) )
	{
		// no ledge checks on derrick
		return FALSE;
	}

	return TRUE;
}

void AGearPC::physWalking(FLOAT deltaTime, INT Iterations)
{
	const FLOAT MaxFloatDist = 200.f;
	// save the current 'safe' location in case we end up in a bad spot
	FVector SafeLocation = Location;

	// some shared properties used for multiple checks
	UBOOL bHitFloor = FALSE;
	FVector FloorDir = FVector(0.f,0.f,-MaxFloatDist * 3.f);
	FCheckResult Hit(1.f);

	// figure where we want to move
	Velocity += Acceleration * deltaTime;
	AngularVelocity = FVector(0.f);
	FVector Delta = Velocity * deltaTime;
	// make sure there is a floor after the move
	FVector CheckLocation = Location + Delta;
	bHitFloor = !GWorld->SingleLineCheck(Hit,this,CheckLocation + FloorDir,CheckLocation,TRACE_AllBlocking,GetCylinderExtent());
	if (bHitFloor)
	{
		FLOAT FloorDist = (Hit.Time * FloorDir.Size());
		if (FloorDist > MaxFloatDist)
		{
			// already above the floor clamp so aim down
			Delta.Z = (-1.f * (FloorDist - MaxFloatDist) * deltaTime);
		}
		// hit a floor, allow the move
		//@note - use moveSmooth here to handle stepping up ledges, bouncing off walls, etc
		moveSmooth(Delta);
		// grab the new floor so we can clamp
		CheckLocation = Location;
		bHitFloor = !GWorld->SingleLineCheck(Hit,this,CheckLocation + FloorDir,CheckLocation,TRACE_AllBlocking,GetCylinderExtent());
		if (!bHitFloor)
		{
			debugf(TEXT("Failed to hit floor for second move, aborting"));
			// shouldn't happen, but unroll the move in this case
			GWorld->MoveActor(this,SafeLocation - Location,Rotation,0,Hit);
		}
		else
		{
			// otherwise make sure we're within reasonable distance of the floor
			if (FloorDist > MaxFloatDist)
			{
				// move downward towards the clamp
				GWorld->MoveActor(this,(FVector(0.f,0.f,-1.f) * (FloorDist - MaxFloatDist) * deltaTime),Rotation,0,Hit);
			}
		}
	}
	else
	{
		debugf(TEXT("Failed to hit floor for initial move"));
	}
}



/** Creates the online subsystem game session. - @todo: This is hacked in to get multiplayer working, this needs to be generalized - ASM 6/15/2006. */
void AGearGame::RegisterGameWithLive()
{

}

FString AGearGame::GetMapFilename()
{
	return GWorld->URL.Map;
}

UBOOL AGearGame::Tick(FLOAT DeltaSeconds, ELevelTick TickType )
{
	UBOOL bRetVal = Super::Tick(DeltaSeconds,TickType);
	{
		SCOPE_CYCLE_COUNTER(STAT_TempTime);

#if XBOX
		FString FrameOfPainTraceFileName;
		extern UBOOL GShouldTraceOutAFrameOfPain;
		if( GShouldTraceOutAFrameOfPain == TRUE )
		{
			//warnf( TEXT(" STARTING" ));
			//appStartCPUTrace( NAME_Game, FALSE, FALSE, 40, NULL );
			//XTraceSetBufferSize( 40 * 1024 * 1024 );
			//FrameOfPainTraceFileName = FString::Printf(TEXT("GAME:\\trace-frameOfPain-%s.pix2"), *appSystemTimeString());
			//XTraceStartRecording( TCHAR_TO_ANSI(*FrameOfPainTraceFileName) );
		}
#endif //  XBOX

		AIVisMan->Tick(DeltaSeconds);

#if XBOX
		if( GShouldTraceOutAFrameOfPain == TRUE )
		{
			//warnf( TEXT(" STOPPING" ));
			//appStopCPUTrace( NAME_Game );
			//GShouldTraceOutAFrameOfPain = FALSE;
			//XTraceStopRecording();
			//const FString NotifyString = FString::Printf(TEXT("UE_PROFILER!GAME:%s"), *FrameOfPainTraceFileName );
			//appSendNotificationString( TCHAR_TO_ANSI(*NotifyString) );
		}
#endif //  XBOX

	}

	return bRetVal;
}

void AGearGameSP_Base::ResetActorChannel(AActor *ChannelOwner)
{
	//@fixme - this is experimental, and not recommended to be used
	if (ChannelOwner != NULL && !ChannelOwner->bNetTemporary && GWorld->GetNetDriver() != NULL)
	{
		GWorld->GetNetDriver()->ForcedInitialReplicationMap.Remove(ChannelOwner);
		for (INT i = GWorld->GetNetDriver()->ClientConnections.Num() - 1; i >= 0; i--)
		{
			UNetConnection* Connection = GWorld->GetNetDriver()->ClientConnections(i);
			UActorChannel* Channel = Connection->ActorChannels.FindRef(ChannelOwner);
			if (Channel != NULL)
			{
				check(Channel->OpenedLocally);
				Channel->Close();
			}
		}
		// make sure the Actor is updated immediately
		ChannelOwner->bForceNetUpdate = TRUE;
		ChannelOwner->bNetDirty = TRUE;
	}
}


void AGearGameGeneric::AddSupportedGameTypes(AWorldInfo* Info, const TCHAR* WorldFilename) const
{
	// match the map prefix
	FString BaseFilename = FFilename(WorldFilename).GetBaseFilename();
	TArray<FGameTypePrefix> Prefixes = DefaultMapPrefixes;
	Prefixes += CustomMapPrefixes;

	// clear out existing entries
	Info->GameTypesSupportedOnThisMap.Empty();

	// dont save direct references to MP game types if they are cooked into separate standalone packages
	UBOOL bSeparateSharedMPGameContent = FALSE;
	GConfig->GetBool( TEXT("Engine.Engine"), TEXT("bCookSeparateSharedMPGameContent"), bSeparateSharedMPGameContent, GEngineIni );
	const UBOOL bIsMultiplayerMap = BaseFilename.StartsWith(TEXT("MP_"));
	if( bSeparateSharedMPGameContent && bIsMultiplayerMap )
	{
		return;
	}

	for (INT i = 0; i < Prefixes.Num(); i++)
	{
		if (Prefixes(i).Prefix.Len() > 0 && BaseFilename.StartsWith(Prefixes(i).Prefix))
		{
			UClass* GameClass = StaticLoadClass(AGameInfo::StaticClass(), NULL, *Prefixes(i).GameType, NULL, LOAD_None, NULL);
			if (GameClass != NULL)
			{
				Info->GameTypesSupportedOnThisMap.AddUniqueItem(GameClass);
			}
			for (INT j = 0; j < Prefixes(i).AdditionalGameTypes.Num(); j++)
			{
				GameClass = StaticLoadClass(AGameInfo::StaticClass(), NULL, *Prefixes(i).AdditionalGameTypes(j), NULL, LOAD_None, NULL);
				if (GameClass != NULL)
				{
					Info->GameTypesSupportedOnThisMap.AddUniqueItem(GameClass);
				}
			}
			break;
		}
	}
}

void AGearPickupFactory::PostLoad()
{
	Super::PostLoad();
#if !FINAL_RELEASE
	if (InventoryType != NULL && GIsEditor)
	{
		AInventory *DefInv = (AInventory*)(InventoryType->GetDefaultObject());
		if (DefInv != NULL &&
			DefInv->DroppedPickupMesh != NULL)
		{
			if (PickupMesh != NULL)
			{
				DetachComponent(PickupMesh);
				PickupMesh = NULL;
			}
			if (PickupMesh == NULL)
			{
				PickupMesh = (UMeshComponent*)StaticConstructObject(DefInv->DroppedPickupMesh->GetClass(),this, NAME_None, 0, DefInv->DroppedPickupMesh);
				//AttachComponent(PickupMesh);
				// just stick in the array and it'll get attached post-PostLoad automagically
				Components.AddItem(PickupMesh);
			}
		}
	}
#endif
}

void AGearPickupFactory::PostEditChange(UProperty* PropertyThatChanged)
{
	if( ( PropertyThatChanged != NULL ) &&
		( (PropertyThatChanged->GetFName() == FName(TEXT("InventoryType")) ||
		(PickupMesh == NULL && InventoryType != NULL)))
		)
	{
		if (InventoryType != NULL)
		{
			if (PickupMesh != NULL)
			{
				DetachComponent(PickupMesh);
				PickupMesh = NULL;
			}
			AInventory *DefInv = (AInventory*)(InventoryType->GetDefaultObject());
			if (DefInv != NULL &&
				DefInv->DroppedPickupMesh != NULL)
			{
				if (PickupMesh == NULL)
				{
					PickupMesh = (UMeshComponent*)StaticConstructObject(DefInv->DroppedPickupMesh->GetClass(),this, NAME_None, 0, DefInv->DroppedPickupMesh);
					AttachComponent(PickupMesh);
				}
			}
		}
		else
		if (PickupMesh != NULL)
		{
			DetachComponent(PickupMesh);
			PickupMesh = NULL;
		}
	}
	// if we have a mesh,
	if (PickupMesh != NULL)
	{
		// hide the sprite
		if (GoodSprite != NULL)
		{
			GoodSprite->HiddenEditor = TRUE;
		}
	}
	Super::PostEditChange(PropertyThatChanged);
}

void AGearWeaponPickupFactory::PostEditChange(UProperty* PropertyThatChanged)
{
	if( ( PropertyThatChanged != NULL )
		&& ( PropertyThatChanged->GetFName() == FName(TEXT("WeaponPickupClass")) )
		)
	{
		InventoryType = WeaponPickupClass;
		Super::PostEditChange(FindField<UProperty>(GetClass(),TEXT("InventoryType")));
	}
	else
	{
		Super::PostEditChange(PropertyThatChanged);
	}

	if (PickupMesh != NULL)
	{
		FComponentReattachContext ReattachContext(PickupMesh);
		PickupMesh->Translation = PickupMesh->GetArchetype<UPrimitiveComponent>()->Translation + MeshTranslation;
	}
}

UBOOL AGearPawn::IsValidAnchor( ANavigationPoint* AnchorCandidate )
{
	AGearAI* AI = Cast<AGearAI>(Controller);
	if( AI != NULL )
	{
		for( INT Idx = 0; Idx < AI->InvalidAnchorList.Num(); Idx++ )
		{
			FInvalidAnchorItem& Item = AI->InvalidAnchorList(Idx);
			if( Item.InvalidNav == NULL || 
				WorldInfo->TimeSeconds >= Item.InvalidTime + 3.f )
			{
				AI->InvalidAnchorList.Remove( Idx--, 1 );
				continue;
			}

			if( AnchorCandidate == Item.InvalidNav )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

UBOOL AGearPawn::CanFireWeapon(UBOOL bTestingForTargeting)
{
	// can't fire during a reload/switch, death, melee, commlink, ragdoll
	if( Health <= 0 || bTearOff || bNoWeaponFiring || bSwitchingWeapons || bDoingMeleeAttack || bUsingCommLink || bIsConversing || Physics == PHYS_RigidBody )
	{
		return FALSE;
	}

	if( bTestingForTargeting )
	{
		// If we're testing for reloading, then only weapons which prevent targeting while reload should fail here. For others it's accepted.
		if( MyGearWeapon && MyGearWeapon->eventShouldReloadingPreventTargeting() && IsReloadingWeapon() )
		{
			return FALSE;
		}
	}
	else
	{
		// disable firing shortly after evading
		if( (GWorld->GetTimeSeconds() - LastEvadeTime) < 0.3f )
		{
			return FALSE;
		}

		// Can't fire while reloading
		if( IsReloadingWeapon() )
		{
			return FALSE;
		}
	}

	// walk volumes disable weapon firing
	AWalkVolume *WalkVol = Cast<AWalkVolume>(PhysicsVolume);
	if (WalkVol != NULL && WalkVol->bActive)
	{
		return FALSE;
	}
	// check if the special move disables weapon firing
	if (IsDoingASpecialMove() && !SpecialMoves(SpecialMove)->bCanFireWeapon)
	{
		return FALSE;
	}
	if (CoverType != CT_None)
	{
		// Cannot fire while in standing cover w/o a lean
		if (CoverType == CT_Standing && !bDoing360Aiming && CoverAction != CA_LeanLeft && CoverAction != CA_LeanRight && CoverAction != CA_BlindLeft && CoverAction != CA_BlindRight)
		{
			INT OverlapIdx = PickClosestCoverSlot(TRUE);
			//debugf(TEXT("OverlapIdx: %d, left? %s right? %s"),OverlapIdx,OverlapIdx != -1?CurrentLink->Slots(OverlapIdx).bLeanLeft?TEXT("yep"):TEXT("nope"):TEXT("nope"),OverlapIdx != -1?CurrentLink->Slots(OverlapIdx).bLeanRight?TEXT("yep"):TEXT("nope"):TEXT("nope"));
			if (OverlapIdx == -1 || (!CurrentLink->Slots(OverlapIdx).bLeanLeft && !CurrentLink->Slots(OverlapIdx).bLeanRight))
			{
				return FALSE;
			}
		}
		// Cannot fire while peeking
		if (CoverAction == CA_PeekRight || CoverAction == CA_PeekLeft || CoverAction == CA_PeekUp)
		{
			return FALSE;
		}
		// Non blindfirable weapons cannot be blindfired.
		if (MyGearWeapon != NULL && !MyGearWeapon->bBlindFirable && !bDoing360Aiming && (CoverAction == CA_Default || CoverAction == CA_BlindLeft || CoverAction == CA_BlindRight || CoverAction == CA_BlindUp))
		{
			// when targetng a mortar in cover, it's not really blindfiring.
			if( !IsDoingSpecialMove(SM_TargetMortar) )
			{
				return FALSE;
			}
		}
	}
	// don't allow firing if at end of round/match
	AGearGRI *GRI = Cast<AGearGRI>(WorldInfo->GRI);
	if (GRI != NULL && (GRI->GameStatus == GS_RoundOver || GRI->GameStatus == GS_EndMatch))
	{
		return FALSE;
	}
	// no firing in forcewalk camera volumes
	for (INT Idx = 0; Idx < CameraVolumes.Num(); Idx++)
	{
		if (CameraVolumes(Idx) != NULL && CameraVolumes(Idx)->bForcePlayerToWalk)
		{
			return FALSE;
		}
	}
	return TRUE;
}

FGuid* AGearPawn::GetGuid()
{
	MyGuid = FGuid(EC_EventParm);
	if (Controller != NULL)
	{
		FGuid* ControllerGuid = Controller->GetGuid();
		if (ControllerGuid != NULL)
		{
			MyGuid = *ControllerGuid;
			MyGuid.SmallGuid += 1;
		}
	}

	return &MyGuid;
}

UBOOL AGearPawn::ShouldApplyNudge(ACoverLink *Link, INT SlotIdx, UBOOL bIgnoreCurrentCoverAction)
{
	if (Link != NULL && Link->Slots.Num() > 1 && (bIgnoreCurrentCoverAction || (CoverAction != CA_BlindLeft && CoverAction != CA_LeanLeft && CoverAction != CA_LeanRight && CoverAction != CA_BlindRight)))
	{
		const UBOOL bIsLeftEdge = Link->IsLeftEdgeSlot(SlotIdx,TRUE);
		const UBOOL bIsRightEdge = Link->IsRightEdgeSlot(SlotIdx,TRUE);
		// make sure we only have one edge
		if ((bIsLeftEdge && bIsRightEdge) || (!bIsLeftEdge && !bIsRightEdge))
		{
			return FALSE;
		}
		// enforce a min distance between the adjacent slot
		if ((bIsLeftEdge && (Link->GetSlotLocation(SlotIdx) - Link->GetSlotLocation(SlotIdx+1)).Size2D() < CylinderComponent->CollisionRadius) ||
			(bIsRightEdge && (Link->GetSlotLocation(SlotIdx) - Link->GetSlotLocation(SlotIdx-1)).Size2D() < CylinderComponent->CollisionRadius))
		{
			return FALSE;
		}
		// otherwise make sure the adjacent slot covertype is identical
		return ((bIsLeftEdge && Link->Slots(SlotIdx).CoverType == Link->Slots(SlotIdx+1).CoverType) || (bIsRightEdge && Link->Slots(SlotIdx).CoverType == Link->Slots(SlotIdx-1).CoverType));
	}
	return FALSE;
}

UBOOL AGearPawn::IsAtCoverEdge(UBOOL bMirrored, UBOOL bAllowLeanAsEdge, FLOAT Scale, INT *EdgeSlotIdx, INT StartSlotIdx, UBOOL bIgnoreCurrentCoverAction)
{
	// must be in cover
	if (CurrentLink != NULL)
	{
		// find the edge slot
		INT SlotIdx = StartSlotIdx != -1 ? StartSlotIdx : CurrentSlotIdx;
		INT AdjacentSlotIdx = (CurrentSlotIdx == LeftSlotIdx ? RightSlotIdx : LeftSlotIdx);
		if (bMirrored)
		{
			// mirrored, so look for the left-most slot
			while (SlotIdx > 0 && CurrentLink->Slots(SlotIdx-1).bEnabled)
			{
				// if we allow leans then check to see if it's a transition
				if (bAllowLeanAsEdge &&
					CurrentLink->Slots(SlotIdx).bLeanLeft)
				{
					// it's a lean transition, use this as the edge
					break;
				}
				SlotIdx--;
			}
		}
		else
		{
			while (SlotIdx < CurrentLink->Slots.Num() - 1 && CurrentLink->Slots(SlotIdx+1).bEnabled)
			{
				// if we allow leans then check to see if it's a transition
				if (bAllowLeanAsEdge &&
					CurrentLink->Slots(SlotIdx).bLeanRight)
				{
					// it's a lean transition, use this as the edge
					break;
				}
				SlotIdx++;
			}
		}
		if (CurrentLink->IsValidClaim(this,SlotIdx))
		{
			FLOAT SlotDist = (CurrentLink->GetSlotLocation(SlotIdx) - Location).Size2D();
			if (ShouldApplyNudge(CurrentLink,SlotIdx,bIgnoreCurrentCoverAction))
			{
				SlotDist = Max<FLOAT>(0.f,SlotDist - CylinderComponent->CollisionRadius * 0.5f);
				//debugf(TEXT("%2.2f: nudge! %2.3f (%s) slotidx: %d"),WorldInfo->TimeSeconds,SlotDist,bIgnoreCurrentCoverAction?TEXT("ignored"):TEXT("not ignored"),SlotIdx);
			}
			if (SlotDist <= (CylinderComponent->CollisionRadius * Scale))
			{
				if (EdgeSlotIdx != NULL)
				{
					*EdgeSlotIdx = SlotIdx;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

INT AGearPawn::PickClosestCoverSlot(UBOOL bRequireOverlap/* =TRUE */, FLOAT RadiusScale/* =0.5*/, UBOOL bIgnoreCurrentCoverAction)
{
	if (CurrentLink != NULL)
	{
		// check for the single slot case
		if (CurrentLink->Slots.Num() == 1)
		{
			return 0;
		}
		else
		{
			// reject slots that aren't aligned with current player rotation
			if ((CurrentLink->GetSlotRotation(LeftSlotIdx).Vector() | Rotation.Vector()) < 0.2f)
			{
				return RightSlotIdx;
			}
			if ((CurrentLink->GetSlotRotation(RightSlotIdx).Vector() | Rotation.Vector()) < 0.2f)
			{
				return LeftSlotIdx;
			}
			FVector LeftSlotLocation = CurrentLink->GetSlotLocation(LeftSlotIdx);
			FVector RightSlotLocation = CurrentLink->GetSlotLocation(RightSlotIdx);
			FLOAT LeftDist = (Location - LeftSlotLocation).Size2D();
			FLOAT RightDist = (Location - RightSlotLocation).Size2D();
			if (!bRequireOverlap)
			{
				return (LeftDist < RightDist ? LeftSlotIdx : RightSlotIdx);
			}
			else
			{
				// if doing overlaps account for nudging
				const FLOAT NudgeDistance = CylinderComponent->CollisionRadius * RadiusScale;
				if (ShouldApplyNudge(CurrentLink,LeftSlotIdx,bIgnoreCurrentCoverAction))
				{
					LeftDist = Max<FLOAT>(0.f,LeftDist - NudgeDistance);
				}
				if (ShouldApplyNudge(CurrentLink,RightSlotIdx,bIgnoreCurrentCoverAction))
				{
					RightDist = Max<FLOAT>(0.f,RightDist - NudgeDistance);
				}
				// if both are valid pick the one we're facing
				if (LeftDist <= CylinderComponent->CollisionRadius && RightDist <= CylinderComponent->CollisionRadius)
				{
					return (bIsMirrored ? LeftSlotIdx : RightSlotIdx);
				}
				// otherwise pick the one winner
				if (LeftDist <= CylinderComponent->CollisionRadius)
				{
					return LeftSlotIdx;
				}
				else if (RightDist <= CylinderComponent->CollisionRadius)
				{
					return RightSlotIdx;
				}
			}
		}
	}
	return -1;
}

/** Retrieves the combat zone this navigation point encompassed by */
ACombatZone* AGearPawn::GetCombatZoneForNav( ANavigationPoint* Nav )
{
	ACombatZone* CZ = NULL;
	if( Nav != NULL )
	{
		for( INT VolIdx = 0; VolIdx < Nav->Volumes.Num(); VolIdx++ )
		{
			CZ = Cast<ACombatZone>(Nav->Volumes(VolIdx));
			if( CZ != NULL )
				break;
		}			
	}
	return CZ;
}

void AGearPawn::SetAnchor( ANavigationPoint* NewAnchor )
{
	Super::SetAnchor( NewAnchor );

	if (MyGearAI != NULL)
	{
		ACombatZone* CZ = GetCombatZoneForNav(NewAnchor);
		MyGearAI->eventSetPendingCombatZone(CZ);
	}
}

/** Claim cover for this pawn + bookkeeping */
UBOOL AGearPawn::ClaimCover( ACoverLink* Link, INT SlotIdx )
{
	UBOOL bResult = FALSE;

	if( Link != NULL )
	{
		bResult = Link->eventClaim( this, SlotIdx );
		if( bResult )
		{
			if (MyGearAI != NULL)
			{
				ACombatZone* CZ = GetCombatZoneForNav(Link->GetSlotMarker(SlotIdx));
				if (CZ != NULL)
				{
					CZ->eventAddResident(this);
				}
			}
		}
	}

	return bResult;	
}

UBOOL AGearPawn::UnclaimCover( ACoverLink* Link, INT SlotIdx, UBOOL bUnclaimAll )
{
	UBOOL bResult = FALSE;

	if( Link != NULL )
	{
		bResult = Link->eventUnClaim( this, SlotIdx, bUnclaimAll );
		if( bResult )
		{
			if (MyGearAI != NULL)
			{
				ACombatZone* CZ = GetCombatZoneForNav(Link->GetSlotMarker(SlotIdx));
				if (CZ != NULL)
				{
					CZ->eventRemoveResident(this);
				}
			}
		}
	}

	return bResult;	
}

INT	AGearPawn::ModifyCostForReachSpec( UReachSpec* Spec, INT Cost )
{
	INT Result = 0;

	ANavigationPoint* End = Spec->End.Nav();
	if (MyGearAI != NULL && End != NULL)
	{
		// Check if combat zone invalidates this reach spec
		ACombatZone* CZ = GetCombatZoneForNav( End );
		UBOOL bInvalidZone = FALSE;
		if( CZ != NULL && !CZ->IsValidZoneFor( MyGearAI, FALSE ) )
		{
			bInvalidZone = TRUE;
		}
		// If end node is in an invalid zone, crank cost way up
		if( bInvalidZone )
		{
			INT AddedCost = Cost * 100;
			Result += AddedCost;

			//debug
			DEBUGREGISTERCOST( Spec->End.Nav(), TEXT("Invalid CZ"), AddedCost );
		}

		ACoverSlotMarker* Marker = Cast<ACoverSlotMarker>(End);
		if( Marker != NULL )
		{
			// Penalize paths through slots that are already claimed
			// ignore if the cover is acutually disabled, since obviously no one is taking cover there in that case
			if ( !Marker->IsValidClaim(this, TRUE) && Marker->OwningSlot.Link != NULL && Marker->OwningSlot.Link->IsEnabled() &&
				Marker->OwningSlot.Link->Slots(Marker->OwningSlot.SlotIdx).bEnabled )
			{
				INT AddedCost = Cost * 100;
				Result += AddedCost;

				//debug
				DEBUGREGISTERCOST( Spec->End.Nav(), TEXT("Claim CSM"), AddedCost );
			}
			// Penalize mantling when out of combat
			if (MyGearAI->bAvoidIdleMantling && MyGearAI->Squad != NULL && MyGearAI->Squad->EnemyList.Num() == 0 && Cast<UMantleReachSpec>(Spec) != NULL)
			{
				INT AddedCost = Cost * 100;
				Result += AddedCost;

				//debug
				DEBUGREGISTERCOST( Spec->End.Nav(), TEXT("Mantle OOC"), AddedCost );
			}
			// Penalize slot to slot reachspecs when unaware or oblivious
			if ( (MyGearAI->PerceptionMood == AIPM_Unaware || MyGearAI->PerceptionMood == AIPM_Oblivious) &&
				Cast<USlotToSlotReachSpec>(Spec) != NULL )
			{
				INT AddedCost = Cost * 100;
				Result += AddedCost;

				//debug
				DEBUGREGISTERCOST( Spec->End.Nav(), TEXT("Slot2Slot OOC"), AddedCost );
			}
		}
	}

	return Result;
}

void AGearPawn::InitForPathfinding( AActor* Goal, ANavigationPoint* EndAnchor )
{
}

UBOOL AGearPawn::IsDoingASpecialMove() const 
{
	return (SpecialMove != SM_None && !bEndingSpecialMove);
}

/** Returns TRUE if player is current performing AMove. */
UBOOL AGearPawn::IsDoingSpecialMove(BYTE AMove) const
{
	return (SpecialMove == AMove && !bEndingSpecialMove);
}

UBOOL AGearPawn::IsDoingDeathAnimSpecialMove() const
{
	return IsDoingSpecialMove(SM_DeathAnim) || IsDoingSpecialMove(SM_DeathAnimFire);
}

UBOOL AGearPawn::IsDoingMeleeHoldSpecialMove() const
{
	return (SpecialMove == SM_ChainSawHold);
}

UBOOL AGearPawn::IsDoingSpecialMeleeAttack() const
{
	return( IsDoingSpecialMove(SM_ChainSawAttack) || IsDoingSpecialMove(SM_ChainSawVictim) );
}

UBOOL AGearPawn::IsSpecialMeleeVictim() const
{
	return IsDoingSpecialMove(SM_ChainSawVictim);
}

/** Returns TRUE if pawn is DBNO */
UBOOL AGearPawn::IsDBNO() const
{
	//@note: need to consider executions as still DBNO so that code doesn't get confused and think we were revived
	return IsDoingSpecialMove(SM_DBNO) || (Health <= 0 && (IsDoingSpecialMove(SM_CQC_Victim) || IsDoingSpecialMove(SM_ChainSawVictim)));
}

/** Returns TRUE if pawn is a hostage */
UBOOL AGearPawn::IsAHostage() const
{
	return IsDoingSpecialMove(SM_Hostage);
}

/** Returns TRUE if pawn is a kidnapper */
UBOOL AGearPawn::IsAKidnapper() const
{
	return (IsDoingSpecialMove(SM_Kidnapper) || IsDoingSpecialMove(SM_Kidnapper_Execution));
}

UBOOL AGearPawn::IsAliveAndWell() const
{
	return ( Health > 0 && (!bHidden || (Base != NULL && Base->GetAPawn() && !Base->bHidden)) && !bDeleteMe && !bPlayedDeath && !IsDBNO() && !IsAHostage() );
}

UBOOL AGearPawn::IsFacingOther(AGearPawn *OtherPawn, FLOAT MinDot, FLOAT HeightTolerancePct) const
{
	check(CylinderComponent != NULL);

	if( OtherPawn && OtherPawn != this )
	{
		const FVector Delta = OtherPawn->Location - Location;
		return ((Delta.SafeNormal() | Rotation.Vector()) >= MinDot) && 
				(Abs(Delta.Z) < CylinderComponent->CollisionHeight * HeightTolerancePct);
	}

	return FALSE;
}

UBOOL AGearPawn::IsValidEnemyTargetFor(const APlayerReplicationInfo* OtherPRI, UBOOL bNoPRIIsEnemy) const
{
	// hostages are not valid targets
	if( IsAHostage() || bNeverAValidEnemy )
	{
		return FALSE;
	}

	// if we're on the magical neutral team, then we're not a valid enemy for anyone
	if(((AGearPawn*)this)->GetTeamNum() == 254)
	{
		return FALSE;
	}

	// ai doesn't target DBNO pawns in singleplayer
	if (IsDBNO())
	{
		static FName NAME_GearGameHorde(TEXT("GearGameHorde"));
		if ( WorldInfo->GRI != NULL && WorldInfo->GRI->GameClass != NULL && 
			(WorldInfo->GRI->GameClass->IsChildOf(AGearGameSP_Base::StaticClass()) || WorldInfo->GRI->GameClass->GetFName() == NAME_GearGameHorde) )
		{
			return FALSE;
		}
		else
		{
			if (PlayerReplicationInfo == NULL)
			{
				 return bNoPRIIsEnemy;
			}
			
			// and not on same team, or neither is on a team (which implies not a team game)
			return (OtherPRI == NULL || PlayerReplicationInfo->Team == NULL || PlayerReplicationInfo->Team != OtherPRI->Team);
		}
	}

	// AI doesn't target enemys in chainsaw duels in singleplayer
	if( bInDuelingMiniGame )
	{
		if( WorldInfo->GRI != NULL && WorldInfo->GRI->GameClass != NULL && WorldInfo->GRI->GameClass->IsChildOf(AGearGameSP_Base::StaticClass()) )
		{
			return FALSE;
		}
	}

	// only am valid target if not dead, and not driving a vehicle (unless the vehicle is actually a gear turret..)
	if ( bDeleteMe || (Health <=0))
	{
		return FALSE;
	}
	else if(DrivenVehicle)
	{
		AGearTurret* GT = Cast<AGearTurret>(DrivenVehicle);
		if( !GT || !GT->ShouldDriverBeValidTargetFor(this) )
		{
			return FALSE;
		}
	}

	AGearPC* PC = Cast<AGearPC>(Controller);
	if( PC != NULL && PC->bInvisible )
	{
		return FALSE;
	}


	if ( !PlayerReplicationInfo )
	{
		return bNoPRIIsEnemy;
	}

	// and not on same team, or neither is on a team (which implies not a team game)
	return !OtherPRI || !PlayerReplicationInfo->Team || (PlayerReplicationInfo->Team != OtherPRI->Team);
}

//
// BodyStance System
//

/**
 * Play a body stance animation.
 * This will play an animation on slots defined in the Pawn's AnimTree.
 * See definition of EBodyStance in GearPawn.uc
 *
 * @param	Stance			BodyStance animation to play.
 * @param	Rate			Rate animation should be played at.
 * @param	BlendInTime		Blend in time when animation is played.
 * @param	BlendOutTime	Time before animation ends (in seconds) to blend out.
 *							-1.f means no blend out.
 *							0.f = instant switch, no blend.
 *							otherwise it's starting to blend out at (Anim length - BlendOutTime) seconds.
 * @param	bLooping		Should the anim loop? (and play forever until told to stop by BS_Stop)
 * @param	bOverride		Play same animation over again from begining only if bOverride is set to TRUE.
 * @param	GroupName		Set if all the nodes playing an animation should be part of a group. 
 *							In that case they would be synchronized, and only the most relevant animation would trigger notifies.
 *
 * @return	PlayBack length of animation assuming play rate remains constant.
 */
FLOAT AGearPawn::BS_Play
(
	struct FBodyStance	Stance,
	FLOAT				Rate,
	float				BlendInTime,
	FLOAT				BlendOutTime,
	UBOOL				bLooping,
	UBOOL				bOverride,
	FName				GroupName
)
{
	FLOAT PlayBackLength = 0.f;
// 	debugf(TEXT("%3.2f - AGearPawn::BS_Play. Trying to play animation on"), GWorld->GetTimeSeconds(), *GetName());

	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{	
		if( Stance.AnimName(i) != NAME_None )
		{
			if( i < BodyStanceNodes.Num() && BodyStanceNodes(i) )
			{
		// 				debugf(TEXT("%3.2f - AGearPawn::BS_Play. Slot: %d playing: %s on %s"), GWorld->GetTimeSeconds(), i, *Stance.AnimName(i).ToString(), *GetName());
				PlayBackLength = BodyStanceNodes(i)->PlayCustomAnim(Stance.AnimName(i), Rate, BlendInTime, BlendOutTime, bLooping, bOverride);

				// Make sure root bone options are reset to default.
				BodyStanceNodes(i)->SetRootBoneAxisOption(RBA_Default, RBA_Default, RBA_Default);
				// by default animation is played non mirrored
				// It's possible to override that by using BS_SetMirrorOptions()
				BodyStanceNodes(i)->Children(BodyStanceNodes(i)->CustomChildIndex).bMirrorSkeleton = FALSE;
				// By default don't forward end of animation notification. This should be set through BS_SetAnimEndNotify()
				BodyStanceNodes(i)->SetActorAnimEndNotification(FALSE);
				// By default trigger an early anim end notify, as this improves blends.
				// Can be manually disabled through BS_SetEarlyAnimEndNotify().
				BodyStanceNodes(i)->bEarlyAnimEndNotify = TRUE;
				UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq();
				if( SeqNode )
				{
					// Cannot skip ticking this node as it makes a number of functions unreliable.
					SeqNode->bSkipTickWhenZeroWeight = FALSE;
					SeqNode->bZeroRootTranslation = FALSE;
					// Set or Reset AnimGroup. If group doesn't exist, we create it.
					if( AnimTreeRootNode )
					{
						AnimTreeRootNode->SetAnimGroupForNode(SeqNode, GroupName, TRUE);
					}
				}
			}
			else
			{
				debugf(TEXT("%3.2f - AGearPawn::BS_Play. Couldn't find slot index %d to play: %s on %s"), GWorld->GetTimeSeconds(), i, *Stance.AnimName(i).ToString(), *GetName());
			}
		}
	}

#if !FINAL_RELEASE
	if( PlayBackLength == 0.f )
	{
		debugf(TEXT("%3.2f - AGearPawn::BS_Play. %s failed to play animation. AnimTree: %d, Template: %s"), GWorld->GetTimeSeconds(), *GetName(), INT(Mesh->Animations != NULL), Mesh->AnimTreeTemplate ? *Mesh->AnimTreeTemplate->GetFName().ToString() : TEXT("NULL"));
		for(INT i=0; i<Stance.AnimName.Num(); i++)
		{
			if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() )
			{
				debugf(TEXT(" [%i] %s on Slot: %s"), i, *Stance.AnimName(i).ToString(), BodyStanceNodes(i) ? *BodyStanceNodes(i)->NodeName.ToString() : TEXT("NULL"));
			}
		}
	}
#endif 

	return PlayBackLength;
}

/**
 * Play a body stance animation.
 * This will play an animation on slots defined in the Pawn's AnimTree.
 * See definition of EBodyStance in GearPawn.uc
 *
 * @param	Stance			BodyStance animation to play.
 * @param	Duration		Duration in seconds the animation should play for.
 * @param	BlendInTime		Blend in time when animation is played.
 * @param	BlendOutTime	Time before animation ends (in seconds) to blend out.
 *							-1.f means no blend out.
 *							0.f = instant switch, no blend.
 *							otherwise it's starting to blend out at (Anim length - BlendOutTime) seconds.
 * @param	bLooping		Should the anim loop? (and play forever until told to stop by BS_Stop)
 * @param	bOverride		Play same animation over again from begining only if bOverride is set to TRUE.
 * @param	GroupName		Set if all the nodes playing an animation should be part of a group. 
 *							In that case they would be synchronized, and only the most relevant animation would trigger notifies.
 *
 * @return	PlayBack length of animation assuming play rate remains constant.
 */
void AGearPawn::BS_PlayByDuration
(
	struct FBodyStance	Stance,
	FLOAT				Duration,
	float				BlendInTime,
	FLOAT				BlendOutTime,
	UBOOL				bLooping,
	UBOOL				bOverride,
	FName				GroupName
)
{
// 	debugf(TEXT("%3.2f - AGearPawn::BS_PlayByDuration. Trying to play animation on"), GWorld->GetTimeSeconds(), *GetName());

	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None )
		{
			if( i < BodyStanceNodes.Num() && BodyStanceNodes(i) )
			{
// 				debugf(TEXT("%3.2f - AGearPawn::BS_PlayByDuration. Slot: %d playing: %s on %s"), GWorld->GetTimeSeconds(), i, *Stance.AnimName(i).ToString(), *GetName());

				BodyStanceNodes(i)->PlayCustomAnimByDuration(Stance.AnimName(i), Duration, BlendInTime, BlendOutTime, bLooping, bOverride);

				// Make sure root bone options are reset to default.
				BodyStanceNodes(i)->SetRootBoneAxisOption(RBA_Default, RBA_Default, RBA_Default);
				// by default animation is played non mirrored
				// It's possible to override that by using BS_SetMirrorOptions()
				BodyStanceNodes(i)->Children(BodyStanceNodes(i)->CustomChildIndex).bMirrorSkeleton = FALSE;
				// By default don't forward end of animation notification. This should be set through BS_SetAnimEndNotify()
				BodyStanceNodes(i)->SetActorAnimEndNotification(FALSE);
				// By default trigger an early anim end notify, as this improves blends.
				// Can be manually disabled through BS_SetEarlyAnimEndNotify().
				BodyStanceNodes(i)->bEarlyAnimEndNotify = TRUE;
				UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq();
				if( SeqNode )
				{
					// Cannot skip ticking this node as it makes a number of functions unreliable.
					SeqNode->bSkipTickWhenZeroWeight = FALSE;
					SeqNode->bZeroRootTranslation = FALSE;
					// Set or Reset AnimGroup. If group doesn't exist, we create it.
					if( AnimTreeRootNode )
					{
						AnimTreeRootNode->SetAnimGroupForNode(SeqNode, GroupName, TRUE);
					}
				}
			}
			else
			{
				debugf(TEXT("%3.2f - AGearPawn::BS_Play. Couldn't find slot index %d to play: %s on %s"), GWorld->GetTimeSeconds(), i, *Stance.AnimName(i).ToString(), *GetName());
			}
		}
	}
}

/** 
 * Returns TRUE if given BodyStance is being played currently.
 * Note that calling this from the AnimEnd notification is not reliable as the animation has technically stopped playing.
 */
UBOOL AGearPawn::BS_IsPlaying(struct FBodyStance Stance)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() && BodyStanceNodes(i) &&
			BodyStanceNodes(i)->bIsPlayingCustomAnim && BodyStanceNodes(i)->GetPlayedAnimation() == Stance.AnimName(i) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** Returns TRUE if animation has any weight in the tree */
UBOOL AGearPawn::BS_HasAnyWeight(struct FBodyStance Stance)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		// First see if we have a BodyStance node relevant for the animation we're trying to play
		if( Stance.AnimName(i) != NAME_None 
			&& i < BodyStanceNodes.Num() 
			&& BodyStanceNodes(i) && BodyStanceNodes(i)->NodeTotalWeight > ZERO_ANIMWEIGHT_THRESH )
		{
			// Now we have no other way then looking at all the children, and seeing if they're playing our animation
			// And if they have any weight, we're good.
			// We can't use GetCustomAnimNodeSeq() because that assumes that the node is playing our animation.
			// But if we blend out, then we 
			for(INT ChildIndex=0; ChildIndex<BodyStanceNodes(i)->Children.Num(); ChildIndex++)
			{
				UAnimNodeSequence* SeqNode = Cast<UAnimNodeSequence>(BodyStanceNodes(i)->Children(ChildIndex).Anim);
				if( SeqNode && SeqNode->AnimSeqName == Stance.AnimName(i) && SeqNode->NodeTotalWeight > ZERO_ANIMWEIGHT_THRESH )
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/** 
 * Returns TRUE if given AnimNodeSequence SeqNode belongs to the given BodyStance 
 * ie is current triggered in the AnimTree, but not necessarily playing anymore. The animation could be stopped or blending/blended out.
 * This is more reliable when testing if a BodyStance is done playing in an AnimEnd notification.
 */
UBOOL AGearPawn::BS_SeqNodeBelongsTo(UAnimNodeSequence* SeqNode, struct FBodyStance Stance)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() && BodyStanceNodes(i) &&
			BodyStanceNodes(i)->GetCustomAnimNodeSeq() == SeqNode && SeqNode->AnimSeqName == Stance.AnimName(i) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Stop playing a body stance.
 * This will only stop the nodes playing the actual stance.
 */
void AGearPawn::BS_Stop(struct FBodyStance Stance, FLOAT BlendOutTime)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i<BodyStanceNodes.Num() && BodyStanceNodes(i) )
		{
			UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq(); 
			// Only stop nodes actually playing the stance.
			// In case they've been taken over by another animation since.
			if( SeqNode && SeqNode->AnimSeqName == Stance.AnimName(i) )
			{
// 				debugf(TEXT("%3.2f - AGearPawn::BS_Stop. Slot: %d stopping: %s on %s"), GWorld->GetTimeSeconds(), i, *Stance.AnimName(i).ToString(), *GetName());

				BodyStanceNodes(i)->SetActorAnimEndNotification(FALSE);
				BodyStanceNodes(i)->StopCustomAnim(BlendOutTime);

				// Reset node's AnimGroup
				if( SeqNode->SynchGroupName != NAME_None && AnimTreeRootNode )
				{
					AnimTreeRootNode->SetAnimGroupForNode(SeqNode, NAME_None);
				}
			}
		}
	}
}

/**
 * Stop playing a body stance.
 * This will only stop the nodes playing the actual stance.
 */
void AGearPawn::BS_SetPlayingFlag(struct FBodyStance Stance, UBOOL bNewPlaying)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i<BodyStanceNodes.Num() && BodyStanceNodes(i) )
		{
			UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq(); 
			// Only stop nodes actually playing the stance.
			// In case they've been taken over by another animation since.
			if( SeqNode && SeqNode->AnimSeqName == Stance.AnimName(i) )
			{
				SeqNode->bPlaying = bNewPlaying;
			}
		}
	}
}

/**
 * Stop playing all body stances.
 */
void AGearPawn::BS_StopAll(FLOAT BlendOutTime)
{
	for(INT i=0; i<BodyStanceNodes.Num(); i++)
	{
		if( BodyStanceNodes(i) )
		{
			BodyStanceNodes(i)->StopCustomAnim(BlendOutTime);
			BodyStanceNodes(i)->SetActorAnimEndNotification(FALSE);

			UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq(); 
			// Reset node's AnimGroup
			if( SeqNode && SeqNode->SynchGroupName != NAME_None && AnimTreeRootNode )
			{
				AnimTreeRootNode->SetAnimGroupForNode(SeqNode, NAME_None);
			}
		}
	}
}

/**
 * Overrides a currently playing body stance with a new one.
 * It basically just switches animations.
 */
void AGearPawn::BS_Override(struct FBodyStance Stance)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() && BodyStanceNodes(i) && BodyStanceNodes(i)->bIsPlayingCustomAnim )
		{
			BodyStanceNodes(i)->SetCustomAnim(Stance.AnimName(i));
		}
	}
}

/**
 * Changes the animation position of a body stance.
 */
void AGearPawn::BS_SetPosition(struct FBodyStance Stance, FLOAT Position)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() && BodyStanceNodes(i) && BodyStanceNodes(i)->bIsPlayingCustomAnim )
		{
			UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq();
			if( SeqNode )
			{
				SeqNode->SetPosition(Position, FALSE);
			}
		}
	}
}

/**
 * Returns in seconds the time left until the animation is done playing.
 * This is assuming the play rate is not going to change.
 */
FLOAT AGearPawn::BS_GetTimeLeft(struct FBodyStance Stance)
{
	FLOAT ReturnValue = 0.f;
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() && BodyStanceNodes(i) && BodyStanceNodes(i)->bIsPlayingCustomAnim )
		{
			UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq();
			if( SeqNode )
			{
				ReturnValue = SeqNode->GetTimeLeft();
				break;
			}
		}
	}
	return ReturnValue;
}

/**
 * Returns the Play Rate of a currently playing body stance.
 * if None is found, 1.f will be returned.
 */
FLOAT AGearPawn::BS_GetPlayRate(struct FBodyStance Stance)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() && BodyStanceNodes(i) && BodyStanceNodes(i)->bIsPlayingCustomAnim )
		{
			UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq();
			if( SeqNode )
			{
				return SeqNode->GetGlobalPlayRate();
			}
		}
	}

	return 1.f;
}

/**
 * Set Play rate of the currently playing body stance.
 */
void AGearPawn::BS_SetPlayRate(struct FBodyStance Stance, float NewRate)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() && BodyStanceNodes(i) && BodyStanceNodes(i)->bIsPlayingCustomAnim )
		{
			UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq();
			if( SeqNode )
			{
				SeqNode->Rate = NewRate;
			}
		}
	}
}

/** Scale Play rate of a Body Stance */
void AGearPawn::BS_ScalePlayRate(struct FBodyStance Stance, FLOAT RateScale)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i < BodyStanceNodes.Num() && BodyStanceNodes(i) && BodyStanceNodes(i)->bIsPlayingCustomAnim )
		{
			UAnimNodeSequence* SeqNode = BodyStanceNodes(i)->GetCustomAnimNodeSeq();
			if( SeqNode )
			{
				SeqNode->Rate *= RateScale;
			}
		}
	}
}

/** Set body stances animation root bone options. */
void AGearPawn::BS_SetRootBoneAxisOptions(struct FBodyStance Stance, BYTE AxisX, BYTE AxisY, BYTE AxisZ)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i<BodyStanceNodes.Num() && BodyStanceNodes(i) )
		{
			BodyStanceNodes(i)->SetRootBoneAxisOption(AxisX, AxisY, AxisZ);
		}
	}
}

/**
 * Mirror a body stance animation.
 * Used by mirror transitions.
 */
void AGearPawn::BS_SetMirrorOptions(struct FBodyStance Stance, UBOOL bTransitionToMirrored, UBOOL bBeginTransition, UBOOL bMirrorAnimation)
{
	if( bBeginTransition )
	{
		// Set if we'd like to end up mirrored or not
		// This has to happen before the mirror node is told which nodes to track
		eventSetMirroredSide(bTransitionToMirrored);
	}

	if( !MirrorNode )
	{
		return;
	}

	// Have mirror master node track which nodes are playing the transition.
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i<BodyStanceNodes.Num() && BodyStanceNodes(i) )
		{
			MirrorNode->MirrorBodyStanceNode(BodyStanceNodes(i), bBeginTransition, bMirrorAnimation);
		}
	}
}

/** Request body stance animation to trigger OnAnimEnd event when done playing animation. */
void AGearPawn::BS_SetAnimEndNotify(struct FBodyStance Stance, UBOOL bNewStatus)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i<BodyStanceNodes.Num() && BodyStanceNodes(i) )
		{
			BodyStanceNodes(i)->SetActorAnimEndNotification(bNewStatus);
		}
	}
}

/** 
 * @see bEarlyAnimEndNotify for more details.
 * By default bEarlyAnimEndNotify is TRUE, meaning AnimEnd notifications will be triggered when the animation starts blending out.
 * Which is not when the animation has actually ended, but in most cases it improves transitions, blends and responsiveness.
 * BS_Play() and BS_PlayByDuration() automatically reset that option to TRUE, so you don't need to reset it when setting it to FALSE.
 */
void AGearPawn::BS_SetEarlyAnimEndNotify(struct FBodyStance Stance, UBOOL bNewEarlyAnimEndNotify)
{
	for(INT i=0; i<Stance.AnimName.Num(); i++)
	{
		if( Stance.AnimName(i) != NAME_None && i<BodyStanceNodes.Num() && BodyStanceNodes(i) )
		{
			BodyStanceNodes(i)->bEarlyAnimEndNotify = bNewEarlyAnimEndNotify;
		}
	}
}

UBOOL AGearPawn::IsLeaning()
{
	return (CoverAction == CA_LeanRight || CoverAction == CA_LeanLeft || CoverAction == CA_PopUp);
}

UBOOL AGearPawn::IsPoppingUp()
{
	// we do the extra check to cover some extra edge cases
	return ( CoverAction == CA_PopUp || ((CoverAction != CA_BlindUp) && (CoverDirection == CD_Up)) );
}

// Increase gravity on dead pawns for better look on rag dolls
FLOAT AGearPawn::GetGravityZ()
{
	FLOAT GravScale = Super::GetGravityZ();

	if( Health <= 0 )
	{
		GravScale *= 2.f;
	}

	// Let Special Moves scale gravity.
	if( SpecialMove != SM_None && SpecialMoves(SpecialMove) )
	{
		GravScale *= SpecialMoves(SpecialMove)->GravityScale * SpecialMoves(SpecialMove)->SpeedModifier;
	}

	if( bScalingToZero == TRUE )
	{
		GravScale /= 4.0f;
	}

	return GravScale;
}


/** notification when actor has bumped against the level */
void AGearPawn::NotifyBumpLevel(const FVector &HitLocation, const FVector& HitNormal)
{
	Super::NotifyBumpLevel(HitLocation, HitNormal);
	eventBumpLevel(HitNormal, HitNormal);

	// Monitor when Pawn last bumped into something
	LastBumpTime = WorldInfo->TimeSeconds;
}

#define MAX_MESHTRANSLATIONOFFSET	48.f
#define MAX_MESHROTATIONOFFSET		3000.f

void AGearPawn::SetMeshTranslationOffset(FVector NewOffset, UBOOL bForce)
{
	// Update MeshTranslationOffset variable in WarPawn
	if( (bForce || (MeshTranslationOffset != NewOffset)) && !bDisableMeshTranslationChanges && Cast<ATurret>(DrivenVehicle) == NULL )
	{
		MeshTranslationOffset = NewOffset;

		// Update the mesh's translation offset aswell.
		AGearPawn* DefaultPawn = Cast<AGearPawn>(GetClass()->GetDefaultObject());
		if( Mesh && DefaultPawn && DefaultPawn->Mesh )
		{
			const FVector TransformedOffset = LocalToWorld().InverseTransformNormal(MeshTranslationOffset);
			Mesh->Translation.X = DefaultPawn->Mesh->Translation.X + TransformedOffset.X;
			Mesh->Translation.Y = DefaultPawn->Mesh->Translation.Y + TransformedOffset.Y;
			Mesh->Translation.Z = DefaultPawn->Mesh->Translation.Z + TransformedOffset.Z - MeshFloorConformTranslation;
		} 
	}
}

/** Special version for knocked down - does the correction in two MoveActor calls - XY and then Z, allows Actor to track physics better, because cylinder collision is still on */
void AGearPawn::SyncActorToRBPhysics()
{
	USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(CollisionComponent);

	if(bTearOff || !SkelComp)
	{
		Super::SyncActorToRBPhysics();
		return;
	}

	// Get the RB_BodyInstance we are going to use as the basis for the Actor position.
	URB_BodyInstance* BodyInstance = NULL;
	FMatrix ComponentTM = FMatrix::Identity;

	if(SkelComp && !SkelComp->bUseSingleBodyPhysics)
	{
		// If there is no asset instance, we can't update from anything.
		UPhysicsAssetInstance* AssetInst = SkelComp->PhysicsAssetInstance;
		if(AssetInst && AssetInst->RootBodyIndex != INDEX_NONE)
		{
			BodyInstance = AssetInst->Bodies(AssetInst->RootBodyIndex);
			if(BodyInstance->IsValidBodyInstance())
			{
				// For the skeletal case, we just move the origin of the actor, but don't rotate it.
				// JTODO: Would rotating the Actor be bad?
				FMatrix RootBoneTM = BodyInstance->GetUnrealWorldTM();
				ComponentTM = FTranslationMatrix( RootBoneTM.GetOrigin() );
			}
			else
			{
				BodyInstance = NULL;
			}
		}
	}

	// If we could not find a BodyInstance, do not move Actor.
	if( !BodyInstance )
	{
		return;
	}

	// Update actor Velocity variable to that of rigid body. Might be used by other gameplay stuff.
	BodyInstance->PreviousVelocity = BodyInstance->Velocity;
	BodyInstance->Velocity = BodyInstance->GetUnrealWorldVelocity();
	Velocity = BodyInstance->Velocity;
	AngularVelocity = BodyInstance->GetUnrealWorldAngularVelocity();

	// Now we have to work out where to put the Actor to achieve the desired transform for the component.

	// First find the current Actor-to-Component transform
	FMatrix ActorTM = LocalToWorld();
	FMatrix RelTM = CollisionComponent->LocalToWorld * ActorTM.Inverse();

	// Then apply the inverse of this to the new Component TM to get the new Actor TM.
	FMatrix NewTM = RelTM.Inverse() * ComponentTM;

	FVector NewLocation = NewTM.GetOrigin();
	FVector MoveBy = NewLocation - Location;

	CheckStillInWorld();
	if (bDeleteMe || Physics != PHYS_RigidBody)
	{
		return;
	}

	FRotator NewRotation = NewTM.Rotator();

	// If the new location or rotation is actually different, call MoveActor.
	//@warning: do not reference BodyInstance again after calling MoveActor() - events from the move could have made it unusable (destroying the actor, SetPhysics(), etc)
	if(bAlwaysEncroachCheck || MoveBy.SizeSquared() > 0.01f * 0.01f || NewRotation != Rotation)
	{
		// Break down movement into XY and then Z parts
		FVector XYMove = MoveBy;
		XYMove.Z = 0.f;

		FVector ZMove = MoveBy;
		ZMove.X = ZMove.Y = 0.f;

		// First do XY move
		FCheckResult XYHit(1.0f);
		GWorld->MoveActor(this, XYMove, NewRotation, 0, XYHit);

		// Then, if not destroyed or physics mode changed, do second move
		if (!bDeleteMe && Physics == PHYS_RigidBody)
		{
			FCheckResult ZHit(1.0f);
			GWorld->MoveActor(this, ZMove, NewRotation, 0, ZHit);
		}

		bIsMoving = TRUE;
	}
	// If we have just stopped moving - update all components so their PreviousLocalToWorld is the same as their LocalToWorld,
	// and so motion blur realises they have stopped moving.
	else if(bIsMoving)
	{
		ForceUpdateComponents(FALSE);
		bIsMoving = FALSE;
	}
}

void AGearPawn::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	if (Mesh)
	{
		Mesh->bSyncActorLocationToRootRigidBody = TRUE;
	}

	if (!bTearOff && Physics == PHYS_RigidBody && Mesh != NULL && Mesh->PhysicsAsset != NULL && Mesh->PhysicsAssetInstance != NULL && Mesh->PhysicsAssetInstance->Bodies.IsValidIndex(Mesh->PhysicsAssetInstance->RootBodyIndex))
	{
		URB_BodyInstance* RootBI = Mesh->PhysicsAssetInstance->Bodies(Mesh->PhysicsAssetInstance->RootBodyIndex);
		FMatrix RootBodyTM = RootBI->GetUnrealWorldTM();
		// If server, pack state and flag it as new
		if (Role == ROLE_Authority)
		{
			ReplicationRootBodyPos.Position = RootBodyTM.GetOrigin();
			ReplicationRootBodyPos.bNewData = 1;
			bNetDirty = TRUE;
		}
		// If client, see if there is new state and apply it
		else if(ReplicationRootBodyPos.bNewData == 1)
		{
			// Set to FALSE so that PostNetReceiveLocation does not try and move the physics
			Mesh->bSyncActorLocationToRootRigidBody = FALSE;

#if 1
			const FVector OldPos = RootBodyTM.GetOrigin();
			const FVector NewPos = ReplicationRootBodyPos.Position * 0.2 + OldPos * 0.8;
			Mesh->SetRBPosition(NewPos);
			const FLOAT CorrectionTime = 1.f;
			const FVector CorrectionVel = (ReplicationRootBodyPos.Position - NewPos)/CorrectionTime;
			Mesh->SetRBLinearVelocity(CorrectionVel, TRUE);
#else
			DrawDebugCoordinateSystem(ReplicationRootBodyPos.Position, FRotator(0,0,0), 30.f, TRUE);
#endif

			// Make sure we apply new state just once
			ReplicationRootBodyPos.bNewData = 0;
		}
	}

	if(Velocity.SizeSquared() > (10.f * 10.f))
	{
		LastTimeMoving = GWorld->GetTimeSeconds();
	}

	// Update mesh rotation to match floor, if desired
	UpdateFloorConform(DeltaSeconds);

	FLOAT InterpAmt = Role == ROLE_SimulatedProxy ? 1.f : 32.f;

    // check to see if we should interpolate the Mesh->Translation.Z from the previous frame
	UBOOL bShouldDoMeshTranslation = (MeshTranslationOffset.SizeSquared() != 0.f);
	if (bShouldDoMeshTranslation) 
	{
		// if this is a remote pawn that is still moving
		if( !IsLocallyControlled() && Velocity.SizeSquared() > KINDA_SMALL_NUMBER )
		{
			// scale the interp amount down if moving in the same direction as the offset
			const FLOAT InterpScale = 1.f - (Velocity.SafeNormal() | (MeshTranslationOffset.SafeNormal() * -1));
			InterpAmt *= InterpScale;
		}

		FLOAT DummyZero = 0.f;
		FVector NewMeshTranslationOffset;
		NewMeshTranslationOffset.X = FInterpTo(MeshTranslationOffset.X, DummyZero, DeltaSeconds, Max<FLOAT>(InterpAmt * Abs(MeshTranslationOffset.X)/MAX_MESHTRANSLATIONOFFSET,8.f));
		NewMeshTranslationOffset.Y = FInterpTo(MeshTranslationOffset.Y, DummyZero, DeltaSeconds, Max<FLOAT>(InterpAmt * Abs(MeshTranslationOffset.Y)/MAX_MESHTRANSLATIONOFFSET,8.f));
		NewMeshTranslationOffset.Z = FInterpTo(MeshTranslationOffset.Z, DummyZero, DeltaSeconds, Max<FLOAT>(InterpAmt * Abs(MeshTranslationOffset.Z)/MAX_MESHTRANSLATIONOFFSET,8.f));

		// Update Mesh Translation
		SetMeshTranslationOffset(NewMeshTranslationOffset);
	}
	// We need to keep updating the mesh translation to ensure floor conform translation is applied
	else
	{
		SetMeshTranslationOffset(MeshTranslationOffset, TRUE);
	}

	/** Apply yaw to mesh to smooth out net updates. Don't do this if we don't want yaw messed with */
	if(Abs(MeshYawOffset) > KINDA_SMALL_NUMBER && !bDisableMeshTranslationChanges && bEnableFloorRotConform)
	{
		MeshYawOffset = FInterpTo(MeshYawOffset, 0.f, DeltaSeconds, Max<FLOAT>(InterpAmt * Abs(MeshTranslationOffset.Z)/MAX_MESHROTATIONOFFSET,8.f));
		Mesh->Rotation.Yaw = appRound(MeshYawOffset);
		Mesh->BeginDeferredUpdateTransform();
	}

	if(ScaleLimitTimeToGo > 0.f)
	{
		ScaleLimitTimeToGo -= DeltaSeconds;

		// This give number going from 1 to 0
		FLOAT PctScale = Clamp<FLOAT>(ScaleLimitTimeToGo / TimeToScaleLimits, 0.f, 1.f);
		// Turn into going from 0 to 1
		PctScale = 1.f - PctScale;

		if(Mesh && Mesh->PhysicsAsset && Mesh->PhysicsAssetInstance)
		{
			// Iterate over each constraint we want to reduce limits for.
			for(INT i=0; i<RagdollLimitScaleTable.Num(); i++)
			{
				const FDeadRagdollLimitScale& ScaleInfo = RagdollLimitScaleTable(i);
				const FName JointName = ScaleInfo.RB_ConstraintName;
				const INT ConstraintIndex = Mesh->PhysicsAsset->FindConstraintIndex(JointName);
				if(ConstraintIndex != INDEX_NONE)
				{
					URB_ConstraintInstance* ConInst = Mesh->PhysicsAssetInstance->Constraints(ConstraintIndex);

					const FLOAT NewSwing1Scale = Lerp(1.f, ScaleInfo.Swing1Scale, PctScale);
					const FLOAT NewSwing2Scale = Lerp(1.f, ScaleInfo.Swing2Scale, PctScale);
					const FLOAT NewTwistScale = Lerp(1.f, ScaleInfo.TwistScale, PctScale);

					ConInst->SetAngularDOFLimitScale( NewSwing1Scale, NewSwing2Scale, NewTwistScale, Mesh->PhysicsAsset->ConstraintSetup(ConInst->ConstraintIndex) );
				}
			}
		}
	}

	// Don't handle body impacts blending on rigid body physics (rag doll)
	if( Physics != PHYS_RigidBody && !bPlayedDeath )
	{
		// Physics Body Impact blending back to animation.
		if( Mesh && Mesh->PhysicsAssetInstance && (PhysicsImpactBlendTimeToGo > 0.f || Mesh->PhysicsWeight > 0.f) )
		{
			if( Mesh->PhysicsWeight > KINDA_SMALL_NUMBER && PhysicsImpactBlendTimeToGo > DeltaSeconds )
			{
				PhysicsImpactBlendTimeToGo	-= DeltaSeconds;
				const FLOAT	PctTimeLeft		= Clamp<FLOAT>(PhysicsImpactBlendTimeToGo / PhysicsImpactBlendOutTime, 0.f, 1.f);

				const FLOAT PctTimeElapsed	= 1.f - PctTimeLeft;
				Mesh->PhysicsWeight			= 1.f - PctTimeElapsed * PctTimeElapsed * PctTimeElapsed;
			}
			else if(PhysicsImpactBlendTimeToGo != 0.f)
			{
				Mesh->PhysicsWeight			= 0.f;
				PhysicsImpactBlendTimeToGo	= 0.f;

				// notification that we are done blending back from physics to animations
				eventBodyImpactBlendOutNotify();
			}
		}
	}

	// When gored / DBNO / Ragdolled - force lighting bounds to be around actor location 
	// as this stops the charater's flappy bits from fall outside of the world and then causing the DLE to make us unlit
	if( ( bIsGore == TRUE )
		|| ( IsDBNO() == TRUE )
		|| ( Physics == PHYS_RigidBody)
		)
	{
		LightEnvironment->bOverrideOwnerBounds = TRUE;
		LightEnvironment->OverriddenBounds = FBoxSphereBounds(Location, FVector(100.f,100.f,100.f), 100.f);
	}
	else
	{
		LightEnvironment->bOverrideOwnerBounds = FALSE;
	}

	// If in ragdoll 
	if( Physics == PHYS_RigidBody && 
		bPlayedDeath &&
		WorldInfo->TimeSeconds - LastRenderTime < 1.0f &&
		GWorld &&
		Mesh->PhysicsAsset &&
		Mesh->PhysicsAssetInstance )
	{
		const FLOAT GibThreshSqr = GibEffectsSpeedThreshold * GibEffectsSpeedThreshold;
		
		URB_BodyInstance* BodyInst;

		// and gibbed up - look to see if some parts are moving and we should fire some gib effects, otherwise let's do fall effects (see 'else')
		if(bIsGore)
		{
			// Set of bones to kill as they are too far away 
			TArray<INT> BonesToKill;
			const FLOAT GibKillDistSqr = GibKillDistance*GibKillDistance;

			// Iterate over bodies in gore to see if any are moving.
			for(INT i=0;i<Mesh->PhysicsAsset->BodySetup.Num();++i)
			{
				BodyInst = (Mesh->PhysicsAssetInstance->Bodies(i));
				if(BodyInst && BodyInst->IsValidBodyInstance())
				{
					// See how far this gib has moved from the actor location
					const FMatrix TM = BodyInst->GetUnrealWorldTM();
					if((TM.GetOrigin() - Location).SizeSquared() > GibKillDistSqr)
					{
						// If too far - mark it for killing
						const URB_BodySetup* BS = Mesh->PhysicsAsset->BodySetup(i);
						const INT BoneIndex = Mesh->MatchRefBone(BS->BoneName);
						if(BoneIndex != INDEX_NONE)
						{
							BonesToKill.AddUniqueItem(BoneIndex);
						}
						// No need to do more stuff
						continue;
					}

					// Look for changes in velocity
					const FVector Speed = BodyInst->GetUnrealWorldVelocity();
					if(BodyInst->Velocity != Speed)
					{
						BodyInst->PreviousVelocity = BodyInst->Velocity;
						BodyInst->Velocity = Speed;
					}

					const FLOAT SpeedSq = (BodyInst->Velocity - BodyInst->PreviousVelocity).SizeSquared();
					// If vel change is big enough..
					if( SpeedSq > GibThreshSqr )
					{
						const FMatrix Transform = BodyInst->GetUnrealWorldTM();
						// .. and it has been long enough since we last fired an effect..
						if( (BodyInst->LastEffectPlayedTime + TimeBetweenGibEffects) < GWorld->GetTimeSeconds() )
						{
							// ..call script event.
							eventPlayGibEffect(Transform.GetOrigin(),FVector(0.0f,0.0f,0.0f), SpeedSq);
							BodyInst->LastEffectPlayedTime = GWorld->GetTimeSeconds();
						}
					}
				}
			}

			// Kill bones that need it
			for(INT i=0; i<BonesToKill.Num(); i++)
			{
				Mesh->HideBone(BonesToKill(i), TRUE);
			}
		}
		else if(DeadBodyImpactSound != NULL) // we're not gibbed but we are dead and haven't fallen yet
		{
			INT i =0;
			BodyInst = NULL;
			BodyInst = Mesh->FindBodyInstanceNamed(TorsoBoneName);
			if(BodyInst && BodyInst->IsValidBodyInstance()) // if we have a torso:
			{
				BodyInst->PreviousVelocity = BodyInst->Velocity;
				BodyInst->Velocity = BodyInst->GetUnrealWorldVelocity();
				const FLOAT Speedsq = (BodyInst->Velocity - BodyInst->PreviousVelocity).SizeSquared(); // set up speeds and check for:
				// Flux in speed, Not moving anymore, or its just been too long:
				if(  Speedsq > DeadBodyImpactThreshold*DeadBodyImpactThreshold ||
					 ((BodyInst->Velocity).SizeSquared() == 0.0f) ||
					 WorldInfo->TimeSeconds -  TimeOfDeath > 1.5f)
				{
					// Play the sound and then cancel the sound out so we don't play it again.
					PlaySound( DeadBodyImpactSound );
					//debugf(TEXT("Dead Body Impact"));
					DeadBodyImpactSound = NULL;
				}
			}
			else
			{
				// we don't have a torso, so lets just cut this off at the pass in the future.
				DeadBodyImpactSound = NULL;
			}
		}
	}

	// Tick current Special Move
	if( SpecialMove != SM_None && SpecialMoves(SpecialMove) != NULL )
	{
		SpecialMoves(SpecialMove)->TickSpecialMove(DeltaSeconds);
	}

	// dirty the WeaponStartTraceLocation cache so next frame we get a valid result
	WeaponStartTraceLocationCache.bUpToDate = FALSE;

	// MeatShield MorphNodeWeight interpolation.
	if( MeatShieldMorphNodeWeight )
	{
		if( MeatShieldMorphTimeToGo > 0.f )
		{
			const FLOAT TargetAlpha = bActivateMeatShieldMorph ? 1.f : 0.f;
			if( MeatShieldMorphTimeToGo > DeltaSeconds )
			{
				MeatShieldMorphTimeToGo	-= DeltaSeconds;
				const FLOAT Delta		= TargetAlpha - MeatShieldMorphNodeWeight->NodeWeight;
				const FLOAT BlendDelta	= Delta * Clamp<FLOAT>((DeltaSeconds / MeatShieldMorphTimeToGo), 0.f, 1.f);
				MeatShieldMorphNodeWeight->SetNodeWeight(MeatShieldMorphNodeWeight->NodeWeight + BlendDelta);
			}
			else
			{
				MeatShieldMorphTimeToGo	= 0.f;
				MeatShieldMorphNodeWeight->SetNodeWeight(TargetAlpha);
			}
		}
	}
}


/** Returns TRUE if Pawn is currently reloading his weapon */
UBOOL AGearPawn::IsReloadingWeapon()
{
	return(	FiringMode == UCONST_RELOAD_FIREMODE ||
			FiringMode == UCONST_FIREMODE_FAILEDACTIVERELOAD ||
			FiringMode == UCONST_FIREMODE_ACTIVERELOADSUCCESS ||
			FiringMode == UCONST_FIREMODE_ACTIVERELOADSUPERSUCCESS );
}


#define DEBUGMESHBONECONTROLLERS 0
#define EASEINOUTEXPONTENT 2.f
#define RUNEASEINOUTEXPONTENT 1.5f

void AGearPawn::UpdateMeshBoneControllersNative(FLOAT DeltaTime)
{
	FVector2D NewAimOffsetPct(0,0);
	UBOOL bDoingHeadTracking = FALSE;

#if DEBUGMESHBONECONTROLLERS
	FString DebugString = FString::Printf(TEXT("UpdateMeshBoneControllersNative %f %s"), GWorld->GetTimeSeconds(), *this->GetFName().ToString() );
#endif
	// if being head track blend, please update headtrack time
	if ( HeadTrackBlendTargetWeight != HeadTrackBlendWeightToTarget )
		UpdateHeadTrackInterp( DeltaTime );
	
	// if the controlling player, or the server
	if( Controller != NULL )
	{
		FRotator HeadDeltaRot(0,0,0);
		FRotator AimDeltaRot(0,0,0);
		FRotator ResultDeltaRot(0,0,0);
		// if not doing a special move or the special move isn't overriding,
		if( !IsDoingASpecialMove() || (SpecialMoves(SpecialMove) && !SpecialMoves(SpecialMove)->eventGetAimOffsetOverride(ResultDeltaRot)) )
		{
			// if head lookat is active
			if( HeadControl && HeadLookAtActor && HeadLookAtActor != this && Mesh && HeadBoneNames.Num() > 0 )
			{
				if ( !eventIsInAimingPose() && !IsDoingSpecialMove(SM_RoadieRun) && IsInCover() == FALSE )
				{
					// These represent "deadzones" where the torso doesn't move and the head does.
					static const FVector2D HeadTrackYawDZ(20.f * (65536.f / 360.f), -5.f * (65536.f / 360.f));		// R, L
					static const FVector2D HeadTrackPitchDZ(20.f * (65536.f / 360.f), -10.f * (65536.f / 360.f));	// U, D

					FVector HeadLoc = Location;
					HeadLoc.Z = Mesh->GetBoneLocation(HeadBoneNames(0)).Z;

					FVector AimDir = HeadControl->DesiredTargetLocation - HeadLoc;
					FRotator OriginalDeltaRot(0,0,0);
					OriginalDeltaRot = (AimDir.Rotation() - Rotation).GetNormalized();

					HeadDeltaRot = OriginalDeltaRot;

					// adjust yaw for deadzone
					if( HeadDeltaRot.Yaw < HeadTrackYawDZ.Y )
					{
						HeadDeltaRot.Yaw -= appTrunc(HeadTrackYawDZ.Y);
					}
					else if (HeadDeltaRot.Yaw > HeadTrackYawDZ.X)
					{
						HeadDeltaRot.Yaw -= appTrunc(HeadTrackYawDZ.X);
					}
					else
					{
						HeadDeltaRot.Yaw = 0;
					}

					// adjust pitch for deadzone
					if (HeadDeltaRot.Pitch < HeadTrackPitchDZ.Y)
					{
						HeadDeltaRot.Pitch -= appTrunc(HeadTrackPitchDZ.Y);
					}
					else if (HeadDeltaRot.Pitch > HeadTrackPitchDZ.X)
					{
						HeadDeltaRot.Pitch -= appTrunc(HeadTrackPitchDZ.X);
					}
					else
					{
						HeadDeltaRot.Pitch = 0;
					}

					// tweakables
					static const FLOAT VelLim = 175.f;			// covers running forward and backward
					static const INT YawMax = 7000;
					static const INT YawMin = -7000;
					static const INT PitchMax = 8192;
					static const INT PitchMin = -8192;

					// limit range somewhat when running, to avoid looking funny
					FRotator RunDeltaRot(0,0,0);
					if( Velocity.Size2D() > VelLim )
					{
						// If not already adjusting for the run cone
						if( !bDoingHeadTrackRunOffsetInterpolation )
						{
							// Init the run timer using the last one... smooths the start/stop/start run interpolation
							HeadTrackRunInterpTime = (HeadTrackRunInterpDuration - HeadTrackRunInterpTime);
							bDoingHeadTrackRunOffsetInterpolation = TRUE;
						}

						// Run delta rot is difference between current delta and the clamped version
						RunDeltaRot.Pitch = Clamp<INT>(HeadDeltaRot.Pitch, PitchMin, PitchMax) - HeadDeltaRot.Pitch;
						RunDeltaRot.Yaw	  = Clamp<INT>(HeadDeltaRot.Yaw, YawMin, YawMax) - HeadDeltaRot.Yaw;
						RunDeltaRot		  = RunDeltaRot.GetNormalized();

						// Update interp time and percentage of time to completion
						HeadTrackRunInterpTime		= ::Max<FLOAT>( HeadTrackRunInterpTime - DeltaTime, 0.f );
						FLOAT HeadTrackRunInterpPct = 1.f - (HeadTrackRunInterpTime / HeadTrackRunInterpDuration);

						// Scale run delta rot by ease in/out and store the last delta
						RunDeltaRot = RunDeltaRot * FInterpEaseInOut(0.f, 1.f, HeadTrackRunInterpPct, RUNEASEINOUTEXPONTENT);
						LastHeadTrackRunDeltaRot = RunDeltaRot;
					}
					else
					{
						// If not already scaling out of run cone
						if( bDoingHeadTrackRunOffsetInterpolation )
						{
							// Init the run timer using the last time... smooths the stop/start/stop run interpolation
							HeadTrackRunInterpTime = (HeadTrackRunInterpDuration - HeadTrackRunInterpTime);
							bDoingHeadTrackRunOffsetInterpolation = FALSE;
						}

						// Initial rotation is from last run cone delta
						RunDeltaRot = LastHeadTrackRunDeltaRot;

						// Update interp time and percentage of time completion
						HeadTrackRunInterpTime = ::Max<FLOAT>( HeadTrackRunInterpTime - DeltaTime, 0.f );
						FLOAT HeadTrackRunInterpPct = 1.f - (HeadTrackRunInterpTime / HeadTrackRunInterpDuration);

						// Scale run delta rot by ease in/out
						RunDeltaRot = RunDeltaRot * FInterpEaseInOut(1.f, 0.f, HeadTrackRunInterpPct, RUNEASEINOUTEXPONTENT);
					}

					// Update interp time and percentage of time completion for full head/body interp
					HeadTrackTime = ::Max<FLOAT>( HeadTrackTime - DeltaTime, 0.f );
					FLOAT HeadTrackPct = 1.f - (HeadTrackTime / HeadTrackDuration);

					// Scale total delta (including run) by full ease in/out amount and store last delta for when we scale out of head tracking
					HeadDeltaRot = ((HeadDeltaRot + RunDeltaRot) * FInterpEaseInOut( 0.f, 1.f, HeadTrackPct, EASEINOUTEXPONTENT ));
					LastHeadTrackDeltaRot = HeadDeltaRot;

					bDoingHeadTracking = TRUE;
					bDoingHeadTrackAimOffsetInterpolation = TRUE;

#if DEBUGMESHBONECONTROLLERS
					DebugString += FString::Printf(TEXT(" HeadLookAtActor: %s, OriginalDeltaRot: %s"), *HeadLookAtActor->GetFullName(), *OriginalDeltaRot.ToString());
#endif
				}
				// if no headtracking is active, use last cached value with faster speed (since this happens between cover/out of cover or aim/noaim
				else 
				{
					HeadDeltaRot = LastHeadTrackDeltaRot;
					// this case the blendtime should be normal
					HeadTrackCurrentBlendSpeed = HeadTrackBlendSpeed*HeadTrackBlendSpeedModiferWhenSwitch;
				}
			}
			// if headtracking is inactive, use last cached value with slower speed
			else 
			{
				// I don't have anybody, use lastheadDeltaRot
				HeadDeltaRot = LastHeadTrackDeltaRot;
				// this case the blendtime should be normal
				HeadTrackCurrentBlendSpeed = HeadTrackBlendSpeed;
			}

			// if aim is still active, then calculate aimdeltarot
			if (HeadTrackBlendWeightToTarget < 1.0f)
			{
				if( MyGearAI != NULL )
				{
					if (SpecialMove != SM_RoadieRun && MyGearAI->HasValidTarget())
					{
						const FVector AimOffsetOrigin = eventGetAimOffsetOrigin();
						const FVector AimDir = MyGearAI->eventGetAimLocation(AimOffsetOrigin) - AimOffsetOrigin;

						AimDeltaRot = (AimDir.Rotation() - Rotation).GetNormalized();
						if( MyGearWeapon != NULL && MyGearWeapon->bHose && MyGearWeapon->eventIsFiring() )
						{
							FRotator Rot = MyGearWeapon->eventGetHoseOffset();
							AimDeltaRot += Rot;
						}
#if DEBUGMESHBONECONTROLLERS
						DebugString += FString(TEXT(" MyGearAI->HasValidTarget()"));
#endif
					}
				}
				else
				{
					// check the weapon aiming if available
					if( MyGearWeapon != NULL )
					{
						const FVector StartTrace = eventGetWeaponStartTraceLocation();
						const FVector EndTrace = StartTrace + (eventGetBaseAimRotation().Vector() * MyGearWeapon->eventGetTraceRange());

						const FVector AimOffsetOrigin = eventGetAimOffsetOrigin();
						const FVector AimDir = EndTrace - AimOffsetOrigin;

						AimDeltaRot = (AimDir.Rotation() - Rotation).GetNormalized();

						if (CoverAction == CA_BlindUp)
						{
							AimDeltaRot.Pitch = Max(MyGearWeapon->MinBlindFireUpAimPitch, AimDeltaRot.Pitch);
						}

						//`log("StartTrace:" @ StartTrace @ "EndTrace:" @ EndTrace @ "AimOffsetOrigin:" @ AimOffsetOrigin @ "DeltaRot:" @ DeltaRot);
#if DEBUGMESHBONECONTROLLERS
						DebugString += FString(TEXT(" MyGearWeapon"));
#endif
					}
					else
					{
						// find out delta angle between pawn's rotation and crosshair direction
						AimDeltaRot = (eventGetBaseAimRotation() - Rotation).GetNormalized();

#if DEBUGMESHBONECONTROLLERS
						DebugString += FString(TEXT(" eventGetBaseAimRotation"));
#endif
					}
				}
			}

			// if head is only active - resultdelta is headdelta
			if ( HeadTrackBlendWeightToTarget == 1.f )
			{
				ResultDeltaRot = HeadDeltaRot;
			}
			// if aim is only active, resultdelta is aim delta
			else if ( HeadTrackBlendWeightToTarget == 0.f)
			{
				ResultDeltaRot = AimDeltaRot;			 
			}

			// when current weight to target isn't met target weight, then continue interpolating
			if ( HeadTrackBlendWeightToTarget != HeadTrackBlendTargetWeight )
			{
				HeadTrackInterp( AimDeltaRot, HeadDeltaRot, ResultDeltaRot );
			}
		}

		// just started to head tracking, set new target weight
		if ( bDoingHeadTracking && !WasHeadTracking )
		{
			HeadTrackBlendTargetWeight = 1.f;
		} 
		// just getting out of head tracking, set new target weight
		else if ( !bDoingHeadTracking && WasHeadTracking )
		{
			HeadTrackBlendTargetWeight = 0.f;
		}
		// this could happen if this tick was interrupted and headtrackactor was removed before coming back to tick. 
		else if ( !bDoingHeadTracking && !WasHeadTracking && HeadTrackBlendTargetWeight==1.0f )
		{
			HeadTrackBlendTargetWeight = 0.f;
		}

		// Convert DeltaRot to AimOffsetPct
		NewAimOffsetPct.X = FLOAT(ResultDeltaRot.Yaw) / 16384.f;
		NewAimOffsetPct.Y = FLOAT(ResultDeltaRot.Pitch) / 16384.f;

#if DEBUGMESHBONECONTROLLERS
		DebugString += FString::Printf(TEXT(" DeltaRot: %s, NewAimOffsetPct: %s"), *ResultDeltaRot.ToString(), *NewAimOffsetPct.ToString() );
#endif
		// Debug Drawing
		//		DrawDebugLine(GetAimOffsetOrigin(), GetAimOffsetOrigin() + Vector(Rotation + DeltaRot) * MyGearWeapon.GetTraceRange(), 255, 0, 0);

		// set the replicated aim offset
		INT NewReplicatedAimOffsetPct = ((ResultDeltaRot.Yaw & 65535) << 16) + (ResultDeltaRot.Pitch & 65535);
		// only set ReplicatedAimOffsetPct if it's really different to avoid setting bNetDirty unnecessarily
		if( ReplicatedAimOffsetPct != NewReplicatedAimOffsetPct )
		{
			ReplicatedAimOffsetPct = NewReplicatedAimOffsetPct;
		}

		//DrawDebugLine(Location, Location + Vector(Rotation) * 300.f, 255, 0, 0);
		//DrawDebugLine(Location, Location + Vector(GetBaseAimRotation()) * 300.f, 0, 255, 0);
	}
	// If no controller, assume remote client, and use replicated aimoffset data
	else
	{
		NewAimOffsetPct.X = FRotator::NormalizeAxis(ReplicatedAimOffsetPct >> 16) / 16384.f;
		NewAimOffsetPct.Y = FRotator::NormalizeAxis(ReplicatedAimOffsetPct & 65535) / 16384.f;
	}

	// Update AimOffset for this Pawn (interpolation and 360 aiming).
	// Not for Troika, this is done by the turret.
	if( DrivenVehicle == NULL )// || DrivenVehicle->IsA('Troika_Cabal') )
	{
		UpdateAimOffset(NewAimOffsetPct, DeltaTime);

#if DEBUGMESHBONECONTROLLERS
		debugf( TEXT("%s, AimOffsetPct: %s"), *DebugString, *AimOffsetPct.ToString() );
#endif
	}

	// Update Mesh's bone controllers
	if( HeadControl != NULL )
	{
		if( HeadLookAtActor != NULL )
		{
			HeadControl->DesiredTargetLocation = eventGetHeadLookTargetLocation();
		}
		
		HeadControl->InterpolateTargetLocation(DeltaTime);
	}

	// Update WeaponAimIKPositionFix
	if( IKHackNode != NULL && MyGearWeapon != NULL )
	{
		eventFixWeaponAimIKPosition(DeltaTime);
	}

	// cache if we did headtracking in this frame
	WasHeadTracking = bDoingHeadTracking;
}

// Interpolation between head tracking and aim offset.
void AGearPawn::UpdateHeadTrackInterp( const FLOAT & DeltaTime )
{
	if ( HeadTrackBlendTargetWeight >= 1.0f )
	{
		HeadTrackBlendWeightToTarget += DeltaTime/HeadTrackCurrentBlendSpeed;
	}
	else if ( HeadTrackBlendTargetWeight <= 0.0f )
	{
		HeadTrackBlendWeightToTarget -= DeltaTime/HeadTrackCurrentBlendSpeed;
	}

	// clamp
	HeadTrackBlendWeightToTarget = Clamp( HeadTrackBlendWeightToTarget, 0.f, 1.0f );
}

void AGearPawn::HeadTrackInterp( const FRotator & AimDeltaRot, const FRotator & HeadTrackDeltaRot, FRotator & OutDeltaRot )
{
	FLOAT HeadTrackWeightInterp = Clamp<FLOAT>(CubicInterp<FLOAT>(0.f, 0.f, 1.f, 0.f, HeadTrackBlendWeightToTarget), 0.f, 1.f); ;

	OutDeltaRot = (HeadTrackWeightInterp*HeadTrackDeltaRot) + (1.f-HeadTrackWeightInterp)*AimDeltaRot;

#if DEBUGMESHBONECONTROLLERS
	FlushPersistentDebugLines();
	DrawDebugLine(Location, Location + (Rotation + HeadTrackDeltaRot).Vector()*100.f, 0, 255, 0, TRUE);
	DrawDebugLine(Location, Location + (Rotation + AimDeltaRot).Vector()*100.f, 0, 0, 255, TRUE);
	DrawDebugLine(Location, Location + (Rotation + OutDeltaRot).Vector()*100.f, 255, 0, 0, TRUE);
#endif
}

/** Is Value between -Max and +Max? */
static inline bool IsInAbsRange(const FLOAT Value, const FLOAT Max)
{
	return (Value <= +Max) && (Value >= -Max);
}

/** Smooth interp for AimOffsets */
FLOAT AGearPawn::AimInterpTo(FLOAT Current, FLOAT Target, FLOAT DeltaTime, FLOAT InterpSpeed)
{
	// Only do circular interpolation when doing 360 aiming.
	if( !bDoing360Aiming )
	{
		return FInterpTo(Current, Target, DeltaTime, InterpSpeed);
	}

	// If no interp speed, jump to target value
	if( InterpSpeed <= 0.f )
	{
		return Target;
	}

	// Distance to reach
	const FLOAT Dist = UnWindNormalizedAimAngleGT(Target - Current);

	// If distance is too small, just set the desired location
	if( Square(Dist) < SMALL_NUMBER )
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FLOAT DeltaMove = Dist * Clamp<FLOAT>(DeltaTime * InterpSpeed, 0.f, 1.f);

	return UnWindNormalizedAimAngleGT(Current + DeltaMove);
}

/** DeadZone used for various 360 aiming transition thresholds */
#define AIMING360_DEADZONE				0.10f

/** 
* Threshold for 360 aiming.
* This is using Normalized Aim angle of [-2;+2], with 0 == Facing Pawn's Rotation.
* between 0 and AIMING360_TRANSITION_THRESHOLD, Pawn is considered facing forward.
* between AIMING360_TRANSITION_THRESHOLD and 2.f, pawn is considered in 360 aiming.
*/
#define AIMING360_TRANSITION_THRESHOLD	(TestCoverType == CT_MidLevel ? 0.9f : 0.6f)

/** Threshold for triggering mirror transition while doing 360 aiming. */
#define AIMING360_MIRROR_THRESHOLD		(2.01f - AIMING360_DEADZONE)

#if 0 && !FINAL_RELEASE
	#define DEBUG_360AIMING(x)	{ ##x }
#else
	#define DEBUG_360AIMING(x)
#endif

/** Update AimOffset for this Pawn (interpolation and 360 aiming). */
void AGearPawn::UpdateAimOffset(FVector2D NewAimOffsetPct, FLOAT DeltaTime)
{
	const FLOAT AimOffsetInterpSpeed = IsHumanControlled() ? AimOffsetInterpSpeedHuman : AimOffsetInterpSpeedAI;

	const UBOOL bIsReloadingWeapon = IsReloadingWeapon();
	if( bIsReloadingWeapon != bReloadingAimInterp )
	{		
		bReloadingAimInterp = bIsReloadingWeapon;
		ReloadingAimInterpTimeToGo = 0.25f;
	}

	if( bIsReloadingWeapon )
	{
		const FLOAT TargetAim = Clamp<FLOAT>(NewAimOffsetPct.Y, -0.067f, +0.067f);
		if( ReloadingAimInterpTimeToGo > DeltaTime )
		{
			ReloadingAimInterpTimeToGo -= DeltaTime;
			NewAimOffsetPct.Y = AimOffsetPct.Y + (TargetAim - AimOffsetPct.Y) * Min(DeltaTime / ReloadingAimInterpTimeToGo, 1.f);
		}
		else
		{
			ReloadingAimInterpTimeToGo = 0.f;
			NewAimOffsetPct.Y = TargetAim;
		}
	}
	else if( ReloadingAimInterpTimeToGo > 0.f )
	{
		if( ReloadingAimInterpTimeToGo > DeltaTime )
		{
			ReloadingAimInterpTimeToGo -= DeltaTime;
			NewAimOffsetPct.Y = AimOffsetPct.Y + (NewAimOffsetPct.Y - AimOffsetPct.Y) * Min(DeltaTime / ReloadingAimInterpTimeToGo, 1.f);
		}
		else
		{
			ReloadingAimInterpTimeToGo = 0.f;
		}
	}

	// Interpolate AimOffset smoothly
	AimOffsetPct.X = AimInterpTo(AimOffsetPct.X, NewAimOffsetPct.X, DeltaTime, AimOffsetInterpSpeed);
	AimOffsetPct.Y = AimInterpTo(AimOffsetPct.Y, NewAimOffsetPct.Y, DeltaTime, AimOffsetInterpSpeed);

	// If Pawn can do 360 aiming, then we have to check for that...
	if( bCanDo360AimingInCover )
	{
		/** Are we already playing 360 animations? */
		const UBOOL	bWasDoing360Aiming = bDoing360Aiming;

		// Clear 360 aiming flag, and set back to TRUE if still relevant.
		bDoing360Aiming = FALSE;

		// Only update if player is alive in cover.
		// Can't be in 360 aiming when doing the cover run special move.
		if( Health > 0 && CoverType != CT_None && SpecialMove != SM_CoverRun && SpecialMove != SM_PushOutOfCover && MyGearWeapon && MyGearWeapon->bSupports360AimingInCover )
		{
			BYTE FacingCovDir;
			if( Simulate360Aiming(bWasDoing360Aiming, AimOffsetPct, CoverType, FacingCovDir) )
			{
				// See if we need to perform a mirror transition. 
				UBOOL	bNeedAMirrorTransition =	(FacingCovDir == CD_Left && !bWantsToBeMirrored) || 
													(FacingCovDir == CD_Right && bWantsToBeMirrored);

				if( bNeedAMirrorTransition && !bDoingMirrorTransition )
				{
					eventSetMirroredSide(FacingCovDir == CD_Left);
				}

				// See if we can officially be in 360 aiming.
				// If we just enterred 360 aiming and we need to do a mirror transition,
				// delay until transition has hapened. Then we can trigger animations.
				if( bWasDoing360Aiming || (!bNeedAMirrorTransition && !bDoingMirrorTransition) )
				{
					bDoing360Aiming = TRUE;
				}
			}

		}

		// If bDoing360Aiming flag changed, call notification
		if( bDoing360Aiming != bWasDoing360Aiming )
		{
			eventOn360AimingChangeNotify();
		}
	} 
}


UBOOL AGearPawn::Simulate360Aiming(UBOOL bCurrentlyDoing360Aiming, FVector2D TestAimOffsetPct, BYTE TestCoverType, BYTE& out_CoverDirection)
{
	// Normalized aim angle [-2;+2], with 0 == Facing Pawn's Rotation.
	const FLOAT HorizAimOffsetPct	= TestAimOffsetPct.X;

	DEBUG_360AIMING( debugf(TEXT("%3.2f HorizAimOffsetPct: %3.2f"), GWorld->GetTimeSeconds(), HorizAimOffsetPct); )

	/** Is Pawn is Forward Range? */
	const UBOOL bAimingFoward		= IsInAbsRange(HorizAimOffsetPct, (AIMING360_TRANSITION_THRESHOLD - AIMING360_DEADZONE));

	// Handle non 360 aiming here.
	if( bAimingFoward )
	{
		// Not in 360 aiming
		DEBUG_360AIMING( debugf(TEXT("%3.2f Not in 360 aiming. HorizAimOffsetPct: %3.2f"), GWorld->GetTimeSeconds(), HorizAimOffsetPct); )
		return FALSE;
	}
	else
	{
		// Doing 360 aiming...
		if( bIsMirrored )
		{
			out_CoverDirection				= CD_Left;
			const FLOAT Limit				= UnWindNormalizedAimAngleGT( -(AIMING360_MIRROR_THRESHOLD + AIMING360_DEADZONE) );
			check(Limit > 0.f);
			const UBOOL	bShouldTurnRight	= (HorizAimOffsetPct < Limit) && (HorizAimOffsetPct > 0.f);

			// See if went beyond threshold, and need to turn to the other side
			if( bShouldTurnRight )
			{
				out_CoverDirection = CD_Right;
				DEBUG_360AIMING( debugf(TEXT("%3.2f bShouldTurnRight, Limit: %3.2f"), GWorld->GetTimeSeconds(), Limit); )
			}
		}
		else
		{		
			out_CoverDirection				= CD_Right;
			const FLOAT Limit				= UnWindNormalizedAimAngleGT( +(AIMING360_MIRROR_THRESHOLD + AIMING360_DEADZONE) );
			check(Limit < 0.f);
			const UBOOL	bShouldTurnLeft		= ((HorizAimOffsetPct > Limit) && (HorizAimOffsetPct < 0.f));

			// See if went beyond threshold, and need to turn to the other side
			if( bShouldTurnLeft )
			{
				out_CoverDirection = CD_Left;
				DEBUG_360AIMING( debugf(TEXT("%3.2f bShouldTurnLeft, Limit: %3.2f"), GWorld->GetTimeSeconds(), Limit); )
			}
		} // if( bIsMirrored )

		DEBUG_360AIMING( debugf(TEXT("%3.2f In 360. HorizAimOffsetPct: %3.2f"), GWorld->GetTimeSeconds(), HorizAimOffsetPct); )
		return TRUE;

	} // Check if facing forward or in 360 aiming.

	return FALSE;
}

UBOOL AGearPawn::IsDoing360ToLeaningTransition()
{
	// handle 360 transitions, only if we changed 360 aiming status not too long ago.
	if( (GWorld->GetWorldInfo()->TimeSeconds - Last360AimingChangeTime) < 1.f )
	{
		// Transition from 360 aiming to leaning
		if( bWasDoing360Aiming && (CoverAction == CA_LeanRight || CoverAction == CA_LeanLeft || CoverAction == CA_PopUp) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL AGearPawn::IsDoingLeaningTo360Transition()
{
	// handle 360 transitions, only if we changed 360 aiming status not too long ago.
	if( (GWorld->GetWorldInfo()->TimeSeconds - Last360AimingChangeTime) < 1.f )
	{
		// Transition from 360 aiming to leaning
		if( bDoing360Aiming && (PreviousCoverAction == CA_LeanRight || PreviousCoverAction == CA_LeanLeft || PreviousCoverAction == CA_PopUp) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** Returns TRUE if player is doing an Evade special move. */
UBOOL AGearPawn::IsEvading()
{
	if( SpecialMove == SM_None )
	{
		return FALSE;
	}

	return (SpecialMove == SM_EvadeFwd	||
			SpecialMove == SM_EvadeBwd	||
			SpecialMove == SM_EvadeLt	||
			SpecialMove == SM_EvadeRt );
}


/** Return TRUE if Pawn is doing one of the move2idle transition special moves */
UBOOL AGearPawn::IsDoingMove2IdleTransition()
{
	return (SpecialMove == SM_Run2Idle || SpecialMove == SM_Walk2Idle);
}

/** Number of units added on top of the Pawn's collision box diagonal to push the player away from cover surface. */
#define COVER_PUSHOUT		4.f

/** 
 * Calculates PushOutOffset based on Pawn's collision cylinder.
 * Takes the hypotenuse as the safe distance to push pawn away from cover, so collision box doesn't try to slide into cover geometry.
 * Cover Slots are AlignDist distance away from cover surface, so take that out from the player's collision cylinder, and add COVER_PUSHOUT units for safety.
 */
static FLOAT GetCoverPushOutOffset(ACoverLink* Link, AGearPawn* GPawn)
{
	FLOAT CollisionRadius = GPawn->CylinderComponent->CollisionRadius;
	FLOAT PushOutDistance = CollisionRadius * 1.41421356f; // Approximation of Sqrt( a^2 + a^2 ) == Sqrt( 2 * a^2) == a * Sqrt(2).

	return (PushOutDistance - Link->AlignDist + COVER_PUSHOUT);
}

/** Utility function to get cover location, and optionally add cover push out offset. */
static FVector GetCoverLocation(const FVector &LeftSlotLocation, const FLOAT &CurrentSlotPct, const FVector &LtToRtSlots, ACoverLink* Link, AGearPawn* GPawn) 
{
	const FVector CoverPushOut = (LtToRtSlots.SafeNormal() ^ FVector(0,0,-1)).SafeNormal() * GetCoverPushOutOffset(Link, GPawn);
	return (LeftSlotLocation + CurrentSlotPct * LtToRtSlots + CoverPushOut);
}

/**
 * Overridden to handle cover slot interpolation.
 */
void AGearPawn::CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant)
{
	const UBOOL	bIsInCover = IsInCover();

	// servers replaying replicated moves just use passed in velocity, clamped appropriately
	if( Physics != PHYS_Walking || bForceRMVelocity || (IsDoingASpecialMove() && SpecialMoves(SpecialMove)->bBreakFromCoverOnEnd) ||
		(!bForceRegularVelocity && Mesh && Mesh->RootMotionMode == RMM_Accel && Mesh->PreviousRMM != RMM_Ignore) )
	{
		Super::CalcVelocity(AccelDir, DeltaTime, MaxSpeed, Friction, bFluid, bBrake, bBuoyant);
	}
	// In cover physics, not when using bPreciseDestination (used for run2cover transition)
	else if( bIsInCover && (!Controller || !Controller->bPreciseDestination) )
	{
		/** Getting velocity from RootMotion? @See Pawn::CalcVelocity() for details. */
		const UBOOL bDoRootMotionVelocity = (!bForceRegularVelocity && Mesh && Mesh->RootMotionMode == RMM_Velocity && Mesh->PreviousRMM != RMM_Ignore );

		/** Root Motion Magnitude. */
		const FLOAT RootMotionMag = bDoRootMotionVelocity ? (Mesh->RootMotionDelta.Translation.Size() / DeltaTime) : 0.f;

		// if we're using root motion, then clear accumulated root motion
		// See USkeletalMeshComponent::UpdateSkelPose() for details.
		if( bDoRootMotionVelocity )
		{
			Mesh->RootMotionDelta.Translation = FVector(0.f);
		}
		
		// Scale MaxSpeed and AccelRate.
		const FLOAT SpeedModifier = MaxSpeedModifier();

		/** Adjust MaxAccel by root motion. @See Pawn::CalcVelocity() for details. */
		const FLOAT MaxAccel = bDoRootMotionVelocity ? Max(AccelRate * SpeedModifier, (RootMotionMag / DeltaTime)) : AccelRate * SpeedModifier;

		/** Must not be in the middle of a mirror transition to move */
		const UBOOL	bNotInAMirrorTransition = (bWantsToBeMirrored == bIsMirrored && !bDoingMirrorTransition) || !MirrorNode || MirrorNode->bBlendingOut;

		// figure out in which direction we want to move. Cover only allows for left or right.
		FLOAT MoveDir = 0.f;
		if( !bIsInStationaryCover && bNotInAMirrorTransition && (!IsDoingASpecialMove() || IsDoingSpecialMove(SM_CoverRun) || !SpecialMoves(SpecialMove)->bDisableMovement) )
		{
			if( bDoing360Aiming )
			{
				if( CurrentSlotDirection == CD_Left )
				{
					MoveDir = -1.f;
				}
				else if( CurrentSlotDirection == CD_Right )
				{
					MoveDir = 1.f;
				}
			}
			else
			{
				if (bIsTargeting)
				{
					if (CurrentSlotDirection == CD_Left)
					{
						MoveDir = -1.f;
					}
					else
					if (CurrentSlotDirection == CD_Right)
					{
						MoveDir = 1.f;
					}
				}

				if( IsHumanControlled() )
				{
					// Player induced movement 
					if( bIsMirrored && (CurrentSlotDirection == CD_Left || SpecialMove == SM_CoverRun) )	
					{
						MoveDir = -1.f;
					}
					else 
					if( !bIsMirrored && (CurrentSlotDirection == CD_Right || SpecialMove == SM_CoverRun) )
					{
						MoveDir = 1.f;
					}

					// Eliminate any movement in standing cover when targeting (pushing the stick should only flip targeting direction if available)
					if ( CoverType == CT_Standing && bIsTargeting )
					{
						MoveDir = 0.f;
					}
				}
				else
				{
					if( CurrentSlotDirection == CD_Left || (bIsMirrored && SpecialMove == SM_CoverRun) )	
					{
						MoveDir = -1.f;
					}
					else
					if( CurrentSlotDirection == CD_Right || (!bIsMirrored && SpecialMove == SM_CoverRun) )	
					{
						MoveDir = 1.f;
					}
				}
			}
		}
		
		// If doing walk transitions, catch acceleration being zero triggering a break.
		if( bDoWalk2IdleTransitions && SpecialMove == SM_None &&
			MoveDir == 0.f && !Velocity.IsZero() )
		{
			eventConditionalMove2IdleTransition();
		}
		else if( IsDoingMove2IdleTransition() && MoveDir != 0.f )
		{
			// If was playing a transition, but now player moves again, then abort transition.
			eventMove2IdleTransitionFinished();
		}

		/*
		// See if transition forces movement
		if( !bIsInStationaryCover && IsDoingMove2IdleTransition() && (RootMotionMag > KINDA_SMALL_NUMBER || bForceMaxAccel) )
		{
			MoveDir = bIsMirrored ? -1.f : 1.f;
		}
		*/

		// move to the slot location defined by the current pct
		FVector LeftSlotLocation	= CurrentLink->GetSlotLocation(LeftSlotIdx);
		FVector RightSlotLocation	= CurrentLink->GetSlotLocation(RightSlotIdx);
		FVector	LtToRtSlots			= RightSlotLocation - LeftSlotLocation;

		// calculate the current slot pct 
		// by projecting our current location onto the slot axis and approximating the current location
		// Do every frame so if pawn is obstructed in movement, slot pct is accurate
		if( MoveDir != 0.f && LeftSlotIdx != RightSlotIdx )
		{
			const FLOAT LtToRtSlotsSize2D = LtToRtSlots.Size2D();
			const FVector LtToRtSlots2D(LtToRtSlots.X, LtToRtSlots.Y, 0.f);
			CurrentSlotPct = Clamp<FLOAT>(((Location - LeftSlotLocation) | (LtToRtSlots2D/LtToRtSlotsSize2D)) / LtToRtSlotsSize2D, 0.f, 1.f);
		}
		const FLOAT OldSlotPct = CurrentSlotPct;

		// Final max speed to use. This determines the maximum distance we can move on this frame.
		const FLOAT	FinalMaxSpeed = bDoRootMotionVelocity ? RootMotionMag : MaxSpeed * SpeedModifier;

		// Distance to move this frame.
		FVector	DeltaMove = FVector(0.f);

		// if cover is moving, then just try to keep up
		if( !CurrentLink->bStatic )
		{
			// ignore interpolating bases
			if( CurrentLink->Base == NULL || CurrentLink->Base->Physics != PHYS_Interpolating )
			{
				const FVector	LtToRightSlots2D(LtToRtSlots.X, LtToRtSlots.Y, 0.f);
				const FLOAT		LtToRtSlotsSize2D = LtToRtSlots.Size2D();
				const FLOAT		DesiredPct = (LeftSlotIdx != RightSlotIdx) ? ((Location - LeftSlotLocation) | (LtToRightSlots2D/LtToRtSlotsSize2D)) / LtToRtSlotsSize2D : 0.f;

				// Calculate the desired move
				DeltaMove	= GetCoverLocation(LeftSlotLocation, CurrentSlotPct, LtToRtSlots, CurrentLink, this) - Location;
				DeltaMove.Z = 0.f;

				if( Abs(DesiredPct-CurrentSlotPct) > 0.1f || DeltaMove.Size2D() > 32.f )
				{
					MoveDir = 0.f; // prevent any normal player moves
				}
			}
		}

		const UBOOL bIsBlindFiring = CoverAction == CA_BlindRight || CoverAction == CA_BlindLeft || CoverAction == CA_BlindUp;
		// check to see if we're at an edge of cover
		const UBOOL bIsAtLeftEdge = CurrentLink->IsLeftEdgeSlot(LeftSlotIdx,TRUE);
		const UBOOL bIsAtRightEdge = CurrentLink->IsRightEdgeSlot(RightSlotIdx,TRUE);

		// calculate the nudge distance and min/max percentages
		FLOAT NudgeDistance = CylinderComponent->CollisionRadius * 0.5f;
		FLOAT MinCurrentSlotPct = 0.f, MinNudgePct = 0.f, MaxCurrentSlotPct = 1.f, MaxNudgePct = 1.f;
		
		// if at either edge then clamp the max distance we can travel towards that edge
		//@note - there is no need to clamp if we're not at an edge, normal 0-1 defaults apply in that case
		if( LeftSlotIdx != RightSlotIdx )
		{
			// always calculate the nudge pct since we need it to detect when to auto-unnudge
			MinNudgePct = Min<FLOAT>(Clamp<FLOAT>(NudgeDistance/LtToRtSlots.Size2D(), 0.f, 1.f), 1.f);
			MaxNudgePct = Max<FLOAT>(1.f - Clamp<FLOAT>(NudgeDistance/LtToRtSlots.Size2D(), 0.f, 1.f), MinNudgePct);
			// if we're not at a transition spot (standing -> crouching) apply limits to both extents
			if (ShouldApplyNudge(CurrentLink,LeftSlotIdx))
			{
				MinCurrentSlotPct = MinNudgePct;
			}
			if (ShouldApplyNudge(CurrentLink,RightSlotIdx))
			{
				MaxCurrentSlotPct = MaxNudgePct;
			}
		}

		// if player wants to move, then calc physics updating the slot position
		if( MoveDir != 0.f )
		{
			// You cannot move at single nodes
			if( CurrentLink->Slots.Num() == 1 || (LeftSlotIdx == RightSlotIdx) )
			{
				DeltaMove = (LeftSlotLocation - Location) - CurrentLink->GetSlotRotation(LeftSlotIdx).Vector() * GetCoverPushOutOffset(CurrentLink, this);
			}
			else if( CurrentLink->Slots.Num() > 1 )
			{
				// this is the current ideal position of player on cover. Player may not be there actually, 
				// but we use that to determine the delta move the player should do on this frame.
				// Later we do position autocorrection, so player reaches that ideal position.
				const FVector OldIdealLocation = GetCoverLocation(LeftSlotLocation, CurrentSlotPct, LtToRtSlots, CurrentLink, this); 
				
				// Update current slot, defining ideal destination.
				CurrentSlotPct += ((MoveDir * FinalMaxSpeed * DeltaTime) / LtToRtSlots.Size2D());

				UBOOL	bUpdateSlots = FALSE;
				INT		NumSlots	 = CurrentLink->Slots.Num();

				// check to see if we've transitioned to an adjacent slot (ignored if we're already at the edge)
				if( MoveDir < 0 && (CurrentSlotPct <= MinCurrentSlotPct) && (!bIsAtLeftEdge || CurrentSlotIdx != 0 ) )
				{
					INT ReachIdx = LeftSlotIdx;

					// If reached end of the slot chain and should not loop
					if( !CurrentLink->bLooped && (LeftSlotIdx == 0 || bIsAtLeftEdge) )
					{
						// Set min pct
						CurrentSlotPct = 0.f;
					}
					else
					{
						// Otherwise, update to the next slot
						bUpdateSlots	 = TRUE;
						LeftSlotIdx		-= 1;
						RightSlotIdx	-= 1;

						// Wrap around if needed
						if( LeftSlotIdx  < 0 ) { LeftSlotIdx  += NumSlots; }
						if( RightSlotIdx < 0 ) { RightSlotIdx += NumSlots; }

						// If stationary cover OR
						// this is AI trying to get to exactly this slot
						if (TargetSlotMarker != NULL && TargetSlotMarker->OwningSlot.SlotIdx == ReachIdx && Cast<AGearAI>(Controller) != NULL)
						{
							CurrentSlotPct = 1.f;
						}
						else
						{
							// solve for unsimulated time, then resolve for new slot pct
							FVector	NewLtToRtSlots = CurrentLink->GetSlotLocation(RightSlotIdx) - CurrentLink->GetSlotLocation(LeftSlotIdx);
							FLOAT LeftoverSlotPct = CurrentSlotPct;
							FLOAT DeltaPctFromUnsimulatedTime = LeftoverSlotPct * LtToRtSlots.Size2D() / NewLtToRtSlots.Size2D();

							// theoretically, the overshoot could overshoot again, so ideally this would be in a 
							// while loop, but that's really not likely to happen, so we'll not deal with that.  worst case,
							// the motion will hitch slightly
							CurrentSlotPct = Clamp<FLOAT>(1.f + DeltaPctFromUnsimulatedTime, 0.f, 1.f);
						}
					}

					// Update anchor for tracking of combat zones/etc
					SetAnchor( CurrentLink->GetSlotMarker( ReachIdx ) );

					eventReachedCoverSlot(ReachIdx);
				}
				else if( MoveDir > 0 && (CurrentSlotPct >= MaxCurrentSlotPct) && (!bIsAtRightEdge || CurrentSlotIdx != NumSlots - 1) )
				{
					INT ReachIdx = RightSlotIdx;

					// If reached end of the slot chain and should not loop
					if( !CurrentLink->bLooped && (RightSlotIdx == NumSlots - 1 || bIsAtRightEdge) )
					{
						// Set max pct
						CurrentSlotPct = 1.f;
					}
					else
					{
						// Otherwise, update to the next slot
						bUpdateSlots	 = TRUE;
						LeftSlotIdx		+= 1;
						RightSlotIdx	+= 1;

						// Wrap around if needed
						if( LeftSlotIdx  >= NumSlots ) { LeftSlotIdx  -= NumSlots; }
						if( RightSlotIdx >= NumSlots ) { RightSlotIdx -= NumSlots; }

						// If stationary cover OR
						// this is AI trying to get to exactly this slot
						if (TargetSlotMarker != NULL && TargetSlotMarker->OwningSlot.SlotIdx == ReachIdx && Cast<AGearAI>(Controller) != NULL)
						{
							CurrentSlotPct = 0.f;
						}
						else
						{
							// solve for unsimulated time, then resolve for new slot pct
							FVector	NewLtToRtSlots = CurrentLink->GetSlotLocation(RightSlotIdx) - CurrentLink->GetSlotLocation(LeftSlotIdx);
							FLOAT LeftoverSlotPct = CurrentSlotPct - 1.f;
							FLOAT DeltaPctFromUnsimulatedTime = LeftoverSlotPct * LtToRtSlots.Size2D() / NewLtToRtSlots.Size2D();

							// theoretically, the overshoot could overshoot again, so ideally this would be in a 
							// while loop, but that's really not likely to happen, so we'll not deal with that.  worst case,
							// the motion will hitch slightly
							CurrentSlotPct = Clamp<FLOAT>(DeltaPctFromUnsimulatedTime, 0.f, 1.f);	
						}
					}

					// Update anchor for tracking of combat zones/etc
					SetAnchor( CurrentLink->GetSlotMarker( ReachIdx ) );

					eventReachedCoverSlot( ReachIdx );
				}
				// special case handling of coverrunning to the far edge of a slot
				else if (IsLocallyControlled() && SpecialMove == SM_CoverRun && ((bIsAtLeftEdge && CurrentSlotPct <= MinCurrentSlotPct) || (bIsAtRightEdge && CurrentSlotPct >= MaxCurrentSlotPct)))
				{
					AGearPC *PC = Cast<AGearPC>(Controller);
					if (PC != NULL)
					{
						PC->eventDoSpecialMove(SM_None,TRUE,NULL,0);
					}
					else
					{
						eventDoSpecialMove(SM_None,TRUE,NULL,0);
					}
				}
				else
				{
					CurrentSlotPct = Clamp<FLOAT>(CurrentSlotPct, 0.f, 1.f);
				}

				// update destination
				if( bUpdateSlots )
				{
					LeftSlotLocation	= CurrentLink->GetSlotLocation(LeftSlotIdx);
					RightSlotLocation	= CurrentLink->GetSlotLocation(RightSlotIdx);
					LtToRtSlots			= RightSlotLocation - LeftSlotLocation;
				}

				// if not targeting, blindfiring, or is 360 aiming then check for the auto-nudge back behind cover
				if( IsHumanControlled() && !bIsBlindFiring && SpecialMove != SM_CoverSlip )
				{
					if( !bIsTargeting || bDoing360Aiming )
					{
						CurrentSlotPct = Clamp<FLOAT>(CurrentSlotPct, MinCurrentSlotPct, MaxCurrentSlotPct);
					}
				}

				// This is the position in cover we'd like to reach.
				const FVector DesiredPositionInCover = GetCoverLocation(LeftSlotLocation, CurrentSlotPct, LtToRtSlots, CurrentLink, this); 

				// Move from ideal location to ideal destination in cover. 
				// We perform position correction later, to push player to his ideal location
				DeltaMove = DesiredPositionInCover - OldIdealLocation;
			}
		}
		else
		{
			if( IsHumanControlled() && !bIsBlindFiring && (SpecialMove == SM_CoverSlip || !bIsTargeting || bDoing360Aiming) )
			{
				// if not targeting or 360 aiming check to see if we need to nudge back behind cover even though we're not moving
				const FVector OldIdealLocation = GetCoverLocation(LeftSlotLocation, CurrentSlotPct, LtToRtSlots, CurrentLink, this); 

				//debugf(TEXT("%2.1f, %2.1f %2.1f"),CurrentSlotPct,MinCurrentSlotPct,MaxCurrentSlotPct);
				CurrentSlotPct = Clamp<FLOAT>(CurrentSlotPct, MinCurrentSlotPct, MaxCurrentSlotPct);

				// This is the position in cover we'd like to reach.
				const FVector DesiredPositionInCover = GetCoverLocation(LeftSlotLocation, CurrentSlotPct, LtToRtSlots, CurrentLink, this); 

				// Move from ideal location to ideal destination in cover. 
				// We perform position correction later, to push player to his ideal location
				DeltaMove = DesiredPositionInCover - OldIdealLocation;
			}
			else if (IsHumanControlled() && (bIsBlindFiring || CoverAction == CA_LeanLeft || CoverAction == CA_LeanRight))
			{
				//debugf(TEXT("min/max/current %2.4f(%2.3f)/%2.4f(%2.3f)/%2.3f"),MinCurrentSlotPct,MinNudgePct,MaxCurrentSlotPct,MaxNudgePct,CurrentSlotPct);
				// if blind firing check to see if we need to auto-unnudge
				if ((SpecialMove == SM_CoverSlip || CoverAction == CA_BlindLeft || CoverAction == CA_LeanLeft) && CurrentSlotIdx == LeftSlotIdx && MinCurrentSlotPct == 0.f && CurrentSlotPct > 0.f)
				{
					//debugf(TEXT("- unnudge left"));
					const FVector OldIdealLocation = GetCoverLocation(LeftSlotLocation, CurrentSlotPct, LtToRtSlots, CurrentLink, this); 

					// This is the position in cover we'd like to reach.
					const FVector DesiredPositionInCover = GetCoverLocation(LeftSlotLocation, 0.f, LtToRtSlots, CurrentLink, this); 

					// Move from ideal location to ideal destination in cover. 
					// We perform position correction later, to push player to his ideal location
					DeltaMove = DesiredPositionInCover - OldIdealLocation;
				}
				else if ((SpecialMove == SM_CoverSlip || CoverAction == CA_BlindRight || CoverAction == CA_LeanRight) && CurrentSlotIdx == RightSlotIdx && MaxCurrentSlotPct == 1.f && CurrentSlotPct < 1.f)
				{
					//debugf(TEXT("- unnudge right %2.3f"),CurrentSlotPct);
					const FVector OldIdealLocation = GetCoverLocation(LeftSlotLocation, CurrentSlotPct, LtToRtSlots, CurrentLink, this); 

					// This is the position in cover we'd like to reach.
					const FVector DesiredPositionInCover = GetCoverLocation(LeftSlotLocation, 1.f, LtToRtSlots, CurrentLink, this); 

					// Move from ideal location to ideal destination in cover. 
					// We perform position correction later, to push player to his ideal location
					DeltaMove = DesiredPositionInCover - OldIdealLocation;
				}
			}
		}

		// remove Z component for walking physics.
		DeltaMove.Z = 0.f;

		// Perform move if significant
		if( DeltaMove.SizeSquared2D() > KINDA_SMALL_NUMBER )
		{
			// Make sure we don't move past destination
			Velocity = DeltaMove / DeltaTime;
			if( Velocity.SizeSquared2D() > FinalMaxSpeed * FinalMaxSpeed )
			{
				Velocity = Velocity.UnsafeNormal() * FinalMaxSpeed;
			}
		}
		else
		{
			// otherwise zero out any movement
			if(!bDoRootMotionVelocity)
			{
				Acceleration = FVector(0.f);
			}	
			else
			{
				// nudge acceleration to get root motion going
				Acceleration = LtToRtSlots.SafeNormal() * MoveDir * AccelRate;
			}
			Velocity = FVector(0.f);
		}

		// if this is not the local player then find the closest slot idx
		if (Controller == NULL || !Controller->IsLocalPlayerController())
		{
			ClosestSlotIdx = PickClosestCoverSlot(TRUE);
			//debugf(TEXT("updated closest slot idx for %s to %d"),*GetName(),ClosestSlotIdx);
			if (ClosestSlotIdx == -1)
			{
				ClosestSlotIdx = CurrentSlotIdx;
			}
		}
	}
	else
	{
		// If doing walk transitions, catch acceleration being zero triggering a break.
		if( bDoWalk2IdleTransitions && !bIsInCover && SpecialMove == SM_None &&
			Acceleration.IsZero() && !Velocity.IsZero() )
		{
			eventConditionalMove2IdleTransition();
		}
		else if( IsDoingMove2IdleTransition() && !Acceleration.IsZero() )
		{
			// If was playing a transition, but now played moves again, then abort transition.
			eventMove2IdleTransitionFinished();
		}

		// otherwise perform a normal update
		Super::CalcVelocity(AccelDir, DeltaTime, MaxSpeed, Friction, bFluid, bBrake, bBuoyant);
	}

	// If doing Root Motion Accel, make sure we applied Stopping Power to it.
	const UBOOL bDoRootMotionAccel = (Mesh && Mesh->RootMotionMode == RMM_Accel && Mesh->PreviousRMM != RMM_Ignore && !bForceRegularVelocity);
	if( bDoRootMotionAccel )
	{
		Velocity *= (1.f - GetResultingStoppingPower());
	}


	// if we have a base that's encroaching on us, disallow movement in the direction of the penetration.
	if (bBaseIsEncroaching)
	{
		FVector AdjustedNormal = BaseEncroachHitLocation - Location;
		AdjustedNormal.Z = 0;
		AdjustedNormal = AdjustedNormal.SafeNormal();

//		FlushPersistentDebugLines();
//		DrawDebugLine(BaseEncroachHitLocation, Location, 255, 255, 0, TRUE);
//		DrawDebugLine(BaseEncroachHitLocation, BaseEncroachHitLocation + HackNormal*200.f, 255, 255, 255, TRUE);
//		DrawDebugBox(BaseEncroachHitLocation, FVector(6,6,6), 255, 64, 64, TRUE);

		// subtract component of velocity that pushes into the encroachment
		Velocity = Velocity - (Velocity | AdjustedNormal) * AdjustedNormal;

		// set to false, collision code will reset to true if encorachment continues
		bBaseIsEncroaching = FALSE;
	}

	RMVelocity = Velocity;

#if 0
	debugf(TEXT("%3.3f GearCalcVelocity Vel Mag: %3.2f, Vel Vect: %s"), GWorld->GetTimeSeconds(), Velocity.Size(), *Velocity.ToString());
#endif
}

void AGearPawn::TickSimulated(FLOAT DeltaSeconds)
{
	// Call PrePerformPhysics on simulated clients, if performPhysics is not called.
	if( !bHardAttach && Physics != PHYS_RigidBody && Physics != PHYS_Interpolating )
	{
		if( SpecialMove != SM_None && SpecialMoves(SpecialMove) )
		{
			SpecialMoves(SpecialMove)->PrePerformPhysics(DeltaSeconds);
		}
		if (CurrentLink != NULL && CurrentSlotIdx != -1)
		{
			ClosestSlotIdx = PickClosestCoverSlot(TRUE);
			if (ClosestSlotIdx == -1)
			{
				ClosestSlotIdx = CurrentSlotIdx;
			}
		}
	}

	// ignore simulated physics when carrying a crate
	if (CarriedCrate == NULL)
	{
		FVector OldLocation = Location;
		Super::TickSimulated(DeltaSeconds);
		PerformStepsSmoothing(OldLocation, DeltaSeconds);
	}
	else
	{
		eventTick(DeltaSeconds);
		ProcessState(DeltaSeconds);
		UpdateTimers(DeltaSeconds);
	}
}

void AGearPawn::stepUp(const FVector& GravDir, const FVector& DesiredDir, const FVector& Delta, FCheckResult &Hit)
{
	// this fixes some cases on our complicated movers (e.g. the Derrick) where Pawns where stepping up when they shouldn't
	// due to being slightly embedded in the mover
	// some form of this probably belongs in the APawn implementation, but it needs extensive testing to make sure it doesn't break other games
	if (!Hit.bStartPenetrating && (Cast<AInterpActor>(Hit.Actor) == NULL || !IsOverlapping(Hit.Actor, NULL, Hit.Component)))
	{
		Super::stepUp(GravDir, DesiredDir, Delta, Hit);
	}
}


UBOOL AGearPawn::ResolveAttachedMoveEncroachment(AActor* EncroachedBase, const FCheckResult& OverlapHit)
{
	static const FLOAT Buffer = 0.1f;

	// get characteristics of the penetration
	if (OverlapHit.Actor == EncroachedBase) // necessary?
	{
		// trying alternate "normal"
		FVector AdjustedNormal = Location - OverlapHit.Location;
		AdjustedNormal.Z = 0;
		AdjustedNormal = AdjustedNormal.SafeNormal();
		FVector const Dest = OverlapHit.Location + AdjustedNormal * Buffer;

		// make sure the new location doesn't leave us floating over the edge
		FCheckResult Hit(1.0f);
		if (!EncroachedBase->ActorLineCheck(Hit, Dest - FVector(0.f, 0.f, MaxStepHeight + MAXSTEPHEIGHTFUDGE), Dest, GetCylinderExtent(), TRACE_AllBlocking))
		{
			if (EncroachedBase->IsOverlapping(this, &Hit))
			{
				// still overlapping!  note this in the pawn, and adjust for this in CalcVelocity.
				bBaseIsEncroaching = TRUE;
//				BaseEncroachInvNormal = AdjustedNormal;		// not using at the moment
				BaseEncroachHitLocation = Hit.Location;
			}

			GWorld->FarMoveActor(this, Dest, FALSE, TRUE, TRUE);
			bJustTeleported = FALSE;
			return TRUE;
		}
	}
	
	return FALSE;
}

static FVector GearSavedRelativeLocation;
static FRotator GearSavedRelativeRotation;
static FVector GearSavedLocation;
static FRotator GearSavedRotation;
static AVehicle* GearSavedDrivenVehicle;
static UBOOL GearSavedHardAttach;


void AGearPawn::PreNetReceive()
{
	GearSavedLocation = Location;
	GearSavedRotation = Rotation;
	GearSavedDrivenVehicle = DrivenVehicle;
	GearSavedHardAttach = bHardAttach;
	Super::PreNetReceive();
}

void AGearPawn::PostNetReceive()
{
	GearSavedRelativeLocation = RelativeLocation;
	GearSavedRelativeRotation = RelativeRotation;

	ExchangeB( bHardAttach,	GearSavedHardAttach );

	Super::PostNetReceive();
	if (Role == ROLE_SimulatedProxy && Physics == PHYS_Walking && Cast<ATurret>(DrivenVehicle) == NULL)
	{
		MeshTranslationOffset += GearSavedLocation - Location;

		if( bEnableFloorRotConform )
		{
			FLOAT DeltaRot = FRotator::NormalizeAxis(GearSavedRotation.Yaw - Rotation.Yaw);
			MeshYawOffset += DeltaRot;
		}
	}
}

void AGearPawn::PostNetReceiveBase(AActor* NewBase)
{
	//debugf(TEXT("%s PostNetReceiveBase Base %s NewBase %s"), *GetName(), *NewBase->GetName(), *Base->GetName());

	if(Role == ROLE_SimulatedProxy && Cast<ATurret>(DrivenVehicle) != NULL && (DrivenVehicle == GearSavedDrivenVehicle))
	{
		return;
	}

	UBOOL bBaseChanged = ( Base!=NewBase );
	if( bBaseChanged )
	{
		bHardAttach = GearSavedHardAttach;

		// Base changed.
		SetBase( NewBase );
	}
	else
	{
		// If the base didn't change, but the 'hard attach' flag did, re-base this actor.
		if(GearSavedHardAttach != bHardAttach)
		{
			bHardAttach = GearSavedHardAttach;

			SetBase( NULL );
			SetBase( NewBase );
		}
	}

	if( Base && !Base->bWorldGeometry )
	{
		if( bBaseChanged || (RelativeLocation != GearSavedRelativeLocation) )
		{
			const FLOAT RelLocDiff = (GearSavedRelativeLocation - RelativeLocation).SizeSquared();
			AInterpActor_GearBasePlatform* Platform = Cast<AInterpActor_GearBasePlatform>(Base);

			if( ClampedBase == NULL || Base != ClampedBase || NewBase != ClampedBase || RelLocDiff > 100.0f)
			{
				// allow even more difference if we're supposed to be hard clamped to this location
				if( Platform == NULL || !Platform->bDisallowPawnMovement || RelLocDiff > 2500.0f )
				{
					GWorld->FarMoveActor( this, Base->Location + GearSavedRelativeLocation, 0, 1, 1 );
					RelativeLocation = GearSavedRelativeLocation;
				}
			}
		}
		// Base could be lost by above FarMoveActor() if it touches something and triggers script, for example
		if( Base != NULL && (bBaseChanged || RelativeRotation != GearSavedRelativeRotation) )
		{
			FCheckResult Hit;
			FRotator NewRotation = (FRotationMatrix(GearSavedRelativeRotation) * FRotationMatrix(Base->Rotation)).Rotator();
			GWorld->MoveActor( this, FVector(0,0,0), NewRotation, MOVE_NoFail, Hit);
			// MoveActor() won't automatically set RelativeRotation if we are based on a bone or a mover
			if (BaseBoneName != NAME_None || Physics == PHYS_Interpolating)
			{
				RelativeRotation = GearSavedRelativeRotation;
			}
		}
	}
	else
	{
		// Relative* are currently meaningless, but save them in case we simply haven't received Base yet
		RelativeLocation = GearSavedRelativeLocation;
		RelativeRotation = GearSavedRelativeRotation;
	}
	bJustTeleported = 0;
}

void AGearPawn::PostNetReceiveLocation()
{
	// If in a vehicle, and that fact has not just changed, do nothing
	if(Cast<ATurret>(DrivenVehicle) != NULL && (DrivenVehicle == GearSavedDrivenVehicle))
	{
		return;
	}

	Super::PostNetReceiveLocation();
}

void AGearPawn::PerformStepsSmoothing(const FVector& OldLocation, FLOAT DeltaSeconds)
{
	// Stairs smoothing interpolation code
	if( Mesh != NULL )
	{
		AGearPawn* DefaultPawn = Cast<AGearPawn>(GetClass()->GetDefaultObject());
		// figure out how much our location has changed
		FVector LocationDelta = Location - OldLocation;
		FLOAT DeltaZ = LocationDelta.Z;
		// if we're walking down a slope check to see if we should go ahead and push up - this will prevent character vibration when moving down slopes
		//@note - ideally we would want to move this into APawn::physWalking, and just compensate for MINFLOORDIST in the move down, but for now we'll keep it here to reduce risk of hurting UT/licensees
		if (DeltaZ < 0.f && Base != NULL)
		{
			FCheckResult Hit;
			FVector ColLocation = CollisionComponent ? Location + CollisionComponent->Translation : Location;
			GWorld->SingleLineCheck( Hit, this, ColLocation + FVector(0.f,0.f,-1.f * (MaxStepHeight + MAXSTEPHEIGHTFUDGE)), ColLocation, TRACE_AllBlocking, GetCylinderExtent() );
			if (!Hit.bStartPenetrating && Hit.Normal.Z < 1.0f && Hit.Normal.Z >= WalkableFloorZ)
			{
				FLOAT FloorDist = Hit.Time * (MaxStepHeight + MAXSTEPHEIGHTFUDGE);
				if (FloorDist < MINFLOORDIST)
				{
					// move up to correct position (average of MAXFLOORDIST and MINFLOORDIST above floor)
					FVector realNorm = Hit.Normal;
					GWorld->MoveActor(this, FVector(0.f,0.f,0.5f*(MINFLOORDIST+MAXFLOORDIST) - FloorDist), Rotation, 0, Hit);
				}
				// grab the new delta
				DeltaZ = Location.Z - OldLocation.Z;
			}
		}
		// Only do this if the Pawn allows it, and for Walking, Falling or Spider physics
		UBOOL bShouldPerformStepsSmoothing = bCanDoStepsSmoothing && (Physics == PHYS_Falling || Physics == PHYS_Walking || Physics == PHYS_Spider) && !Velocity.IsNearlyZero();
		// if falling make sure player is near enough to the ground to perform interpolation
		if( bShouldPerformStepsSmoothing && Physics == PHYS_Falling )
		{
			// Location of collision cylinder
			const FVector	ColLocation = CollisionComponent ? (Location + CollisionComponent->Translation) : Location;
			// Distance to check for ground below collision box
			const FVector	CheckVector(0.f, 0.f, -MaxStepHeight);
			// Collision box extent
			const FVector	Extent(CylinderComponent->CollisionRadius, CylinderComponent->CollisionRadius, CylinderComponent->CollisionHeight);

			// Trace to see if we hit something.
			FCheckResult Hit(1.f);
			GWorld->SingleLineCheck(Hit, this, ColLocation + CheckVector, ColLocation, TRACE_World|TRACE_StopAtAnyHit, Extent);

			// We haven't touched anything, so consider we're too far from ground
			// And so not perform interpolation
			if( !Hit.Actor )
			{
				bShouldPerformStepsSmoothing = FALSE;
			}
		}

        // If we can perform steps smoothing, apply the inverse delta as translation to the mesh
        // Limit by MaxStepHeight
		if( DefaultPawn != NULL && DefaultPawn->Mesh != NULL && bShouldPerformStepsSmoothing && DeltaZ != 0.f)
		{
			// apply the inverse delta as translation to the mesh
			MeshTranslationOffset.Z = Clamp<FLOAT>(MeshTranslationOffset.Z - DeltaZ, -MaxStepHeight * 1.5f, MaxStepHeight * 1.5f);
		}
		// apply the normal location delta for server-side position interpolation
		if (Role == ROLE_Authority && Controller != NULL && !IsLocallyControlled() && Physics == PHYS_Walking)
		{
			LocationDelta.Z = 0.f;
			const FLOAT DeltaSize = LocationDelta.Size();
			if (DeltaSize > 0 && DeltaSize < GroundSpeed)
			{
				MeshTranslationOffset -= LocationDelta;
			}
		}
	}
}

void AGearPawn::UpdateFloorConform(FLOAT DeltaSeconds)
{
	// Get the current special move we are doing
	UGearSpecialMove* CurrentGSM = NULL;
	if(SpecialMove != SM_None)
	{
		CurrentGSM = SpecialMoves(SpecialMove);
	}

	////////////// TRANSLATION

	// Figure out the current desired mesh translation to match floor
	FLOAT ConformTarget = 0.f;
	if(CurrentGSM && CurrentGSM->bConformMeshTranslationToFloor && CylinderComponent)
	{
		FLOAT SlopeAngle = Max(appAcos(Floor.Z), 0.f); // Ensure positive angle
		ConformTarget = (CylinderComponent->CollisionRadius * appTan(SlopeAngle));

		// Limit how much we can translate the mesh
		ConformTarget = Clamp(ConformTarget, 0.f, CurrentGSM->MaxConformToFloorMeshTranslation);
	}

	// Interpolate towards that target at MeshFloorConformTransSpeed
	FLOAT DeltaConform = ConformTarget - MeshFloorConformTranslation;
	FLOAT MaxDeltaConform = MeshFloorConformTransSpeed * DeltaSeconds;
	DeltaConform = Clamp(DeltaConform, -MaxDeltaConform, MaxDeltaConform);
	MeshFloorConformTranslation += DeltaConform;

	////////////// ROTATION

	// Now see if we want to do rotation
	if(bEnableFloorRotConform)
	{
		// Figure out the current desired mesh rotation to match floor
		FRotator TargetRot(0,0,0);
		if(CurrentGSM && CurrentGSM->bConformMeshRotationToFloor)
		{
			// Find world space quat that would get up vector to match floor
			FQuat DeltaQuat = FQuatFindBetween(FVector(0,0,1), Floor);

			// Decompose into an axis and angle for rotation
			FVector DeltaAxis(0,0,0);
			FLOAT DeltaAngle = 0.f;
			DeltaQuat.ToAxisAndAngle(DeltaAxis, DeltaAngle);

			// Limit the amount we can rotate.
			FLOAT MaxRotRadians = CurrentGSM->MaxConformToFloorMeshRotation * (PI/180.f);
			DeltaAngle = Clamp(DeltaAngle, -MaxRotRadians, MaxRotRadians);

			// Transform axis into actor space (which Mesh component is relative to)
			FMatrix PawnYawM = FRotationMatrix(FRotator(0, Rotation.Yaw, 0));
			FVector PawnSpaceAxis = PawnYawM.InverseTransformFVectorNoScale(DeltaAxis);

			// Turn into a Rotator
			FQuat PawnSpaceMeshQuat = FQuat(PawnSpaceAxis, DeltaAngle);
			TargetRot = FQuatRotationTranslationMatrix( PawnSpaceMeshQuat, FVector(0.f) ).Rotator();

			// Don't ever want to change yaw of component
			TargetRot.Yaw = 0;
		}

		// If rotation is not what we want, update it
		if(Mesh && Mesh->Rotation != TargetRot)
		{
			// Interpolate from current rotation to TargetRot
			FRotator DeltaRot = TargetRot - Mesh->Rotation;
			INT RotSpeedUnrealians = appRound(MeshFloorConformRotSpeed * (65536/360.f));
			INT MaxDeltaRot = appCeil(RotSpeedUnrealians * DeltaSeconds);

			Mesh->Rotation.Pitch += Clamp(DeltaRot.Pitch, -MaxDeltaRot, MaxDeltaRot);
			Mesh->Rotation.Roll += Clamp(DeltaRot.Roll, -MaxDeltaRot, MaxDeltaRot);

			// Mark component as having its translation changed
			Mesh->BeginDeferredUpdateTransform();
		}
	}
}

void AGearPawn::performPhysics(FLOAT DeltaSeconds)
{
    // store the current location
	FVector OldLocation = Location;
    // notify the current special move of the physics tick
	if( SpecialMove != SM_None && SpecialMoves(SpecialMove) )
	{
		SpecialMoves(SpecialMove)->PrePerformPhysics(DeltaSeconds);
	}

	if(Physics == PHYS_Walking && bAllowAccelSmoothing && Controller != NULL && SpecialMove != SM_RoadieRun && !IsInCover() && !IsHumanControlled() && !Acceleration.IsNearlyZero())
	{
		FVector AccelDir;
		if(OldAcceleration.IsNearlyZero())
		{
			AccelDir = Rotation.Vector();
		}
		else
		{
			AccelDir = OldAcceleration.SafeNormal();
		}

		const FLOAT DesiredAccelSize = Acceleration.Size();
		FVector DesiredDir = Acceleration/DesiredAccelSize;

		//DrawDebugLine(Location, Location + DesiredDir * 100.f,0,255,0);
		//DrawDebugLine(Location, Location + AccelDir * 110.f,255,0,255);

		//DrawDebugCylinder(Location,Location,EffectiveTurningRadius,16,255,0,0);
		const FLOAT ConvergeTime = (2.0f * PI * EffectiveTurningRadius) / Max<FLOAT>(Velocity.Size(),GroundSpeed);
		const FLOAT RadPerSecondMax = ((2.0f * PI)/ConvergeTime)*DeltaSeconds;
		FLOAT DeltaAngle = appAcos(AccelDir | DesiredDir);
		//FLOAT ORIG = DeltaAngle;
		DeltaAngle = Min<FLOAT>(DeltaAngle,RadPerSecondMax);
		FVector Left = AccelDir ^ FVector(0.f,0.f,1.f);

		//debugf(TEXT("DeltaAngle:%.4f OrigDeltaAngle: %.4f RadPerSecondMax:%.4f ConvergeTime:%.4f UU/sec %i GroundSpeed: %.2f"),DeltaAngle,ORIG,RadPerSecondMax,ConvergeTime,appTrunc((DeltaAngle*180.0f/PI) * 182.f),Max<FLOAT>(Velocity.Size(),GroundSpeed));
		FLOAT AccelDesDot = AccelDir | DesiredDir;
		DesiredDir = AccelDir.RotateAngleAxis(appTrunc((DeltaAngle*180.0f/PI) * 182.f),(-AccelDesDot > 0.995f && -AccelDesDot < 1.01f) ? FVector(0.f,0.f,1.f) : AccelDir^DesiredDir);
		checkSlow(!DesiredDir.ContainsNaN());

		Acceleration = DesiredDir * DesiredAccelSize;
		Controller->SetFocalPoint( Location + Acceleration, TRUE );
	}

	OldAcceleration = Acceleration;

	//debugf(TEXT("%s %2.3f: performphysics"),*GetName(),WorldInfo->TimeSeconds);
    // normal physics tick
	Super::performPhysics(DeltaSeconds);

	PerformStepsSmoothing(OldLocation, DeltaSeconds);
}


/** 
 * Post process physics. Called after player has moved.
 * Implements auto position correction in cover.
 */
void AGearPawn::PostProcessRBPhysics(FLOAT DeltaSeconds, const FVector& OldVelocity)
{
	if (Health > 0)
	{
		if (bCheckKnockdownFall)
		{
			FLOAT DeltaVel = (Velocity.Size() - OldVelocity.Size()) * DeltaSeconds;
			if (DeltaVel < -(180.f * DeltaSeconds) && Abs(KnockdownStartingPosition.Z - Location.Z) > 384.f)
			{
				eventKnockdownFailsafe();
			}
		}
		if (Velocity.Size() > 32.f)
		{
			FCheckResult Hit;
			FVector Dir = KnockdownImpulse.LinearVelocity.SafeNormal();
			const INT NumChecks = 3;
			const FLOAT CheckDist = 128.f;
			UBOOL bFoundGap = FALSE;
			//FlushPersistentDebugLines();
			for (INT Idx = 0; Idx < NumChecks; Idx++)
			{
				FVector Start = Location + (Dir * Idx * CheckDist) + FVector(0,0,32.f);
				FVector End = Start + FVector(0,0,-384.f);
				//DrawDebugLine(Start,End,0,255,0,TRUE);
				if (GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_AllBlocking,FVector(1.f)))
				{
					//debugf(TEXT("%s retarding direction on check %d"),*GetName(),Idx);
					bFoundGap = TRUE;
					Mesh->RetardRBLinearVelocity(Dir,1.f - (Idx+1)/FLOAT(NumChecks));
					break;
				}
			}
		}
	}
}
void AGearPawn::PostProcessPhysics(FLOAT DeltaSeconds, const FVector& OldVelocity)
{
	// Call PostProcess Physics on special move
	if( SpecialMove != SM_None && SpecialMoves(SpecialMove) )
	{
		SpecialMoves(SpecialMove)->PostProcessPhysics(DeltaSeconds);
	}

	if(Physics == PHYS_RigidBody)
	{
		PostProcessRBPhysics(DeltaSeconds,OldVelocity);
	}

	Super::PostProcessPhysics(DeltaSeconds, OldVelocity);

	// This does not apply if in full ragdoll or if dead.
	// Needs to be in walking physics.
	if( Physics != PHYS_Walking || bPlayedDeath || Health <= 0 )
	{
		return;
	}
	// skip cover correction if we're about to break from cover
	if ( IsDoingASpecialMove() && SpecialMoves(SpecialMove)->bBreakFromCoverOnEnd )
	{
	    return;
	}
	// Force auto correction when player enters cover.
	// Because we check against past Velocity, if the player was not moving, or using root motion, 
	// then auto correction update would not be triggered.
	// to solve that, we force one update when player first enters into cover.

	const UBOOL bIsInCover			= IsInCover();
	const UBOOL bForceCorrection	= !bWasInCover && bIsInCover;
				bWasInCover			= bIsInCover;

	// Only perform auto correction in cover if player is moving or moved last frame.
	// this is because sometimes player will never be able to reach his desired position in cover due to his size or 
	// cover placement. If auto position correction is unable to reach this position, and player is not actually being moved, 
	// then don't waste time trying to move it.

	FVector Vel = Velocity - (Base ? Base->Velocity : FVector(0));
	FVector VelSum = (OldVelocity + Vel);

	// if in cover, not doing a slide into cover, and moving fast enough or just transitioned to cover
	if( bIsInCover &&
		(VelSum.SizeSquared() > KINDA_SMALL_NUMBER || bForceCorrection) )
	{
		// Perform auto correction
		FVector IdealPosition = Location;

		// circular cover
		if( CurrentLink->bCircular )
		{
			FLOAT	CylRadius, CylHeight;
			GetBoundingCylinder(CylRadius, CylHeight);

			// Find ideal position in circular cover. That's where we want the player to be.
			IdealPosition = CurrentLink->CircularOrigin + (Location - CurrentLink->CircularOrigin).SafeNormal() * (CurrentLink->CircularRadius + GetCoverPushOutOffset(CurrentLink, this));
		}
		else if( CurrentLink->Slots.Num() == 1 || (LeftSlotIdx == RightSlotIdx) )
		{
			// Single slot, desired destination is the only slot.
			IdealPosition = CurrentLink->GetSlotLocation(CurrentSlotIdx) - CurrentLink->GetSlotRotation(CurrentSlotIdx).Vector() * GetCoverPushOutOffset(CurrentLink, this); 
		}
		else
		{
			const	FVector LeftSlotLocation	= CurrentLink->GetSlotLocation(LeftSlotIdx);
			const	FVector RightSlotLocation	= CurrentLink->GetSlotLocation(RightSlotIdx);
			const	FVector LtToRt				= RightSlotLocation - LeftSlotLocation;

			// Hack for now, but should be expanded to all cover types
			// don't re-evaluate position when using precise destination.
			// Later we'll use desired cover position, so we don't have to re-calculate it on the fly
			// and always considering the shortest distance to cover, which is not necessarily where we want to go!
			if( !Controller || !Controller->bPreciseDestination )
			{
				// if we have moved in between 2 slots, then update current position in cover. (so CurrentSlotPct is accurate).
				// Desired movement may not have been successful.
				// Do that only if we're not on a slot.
				if( Vel.SizeSquared() > KINDA_SMALL_NUMBER && CurrentSlotPct > 0.f && CurrentSlotPct < 1.f ) 	
				{
					// Update CurrentSlotPct to reflect where the player really is in cover
					// Project onto 2d line (remove Z component), so correction works seemlessly with slots at different heights
					const FVector	LtToRt2D(LtToRt.X, LtToRt.Y, 0.f);
					const FLOAT		LtToRtSize2D = LtToRt.Size2D();
					CurrentSlotPct = Clamp<FLOAT>(((Location - LeftSlotLocation) | (LtToRt2D/LtToRtSize2D)) / LtToRtSize2D, 0.f, 1.f);
				}
			}

			// Ideal Position in cover
			IdealPosition = GetCoverLocation(LeftSlotLocation, CurrentSlotPct, LtToRt, CurrentLink, this); 
		}

		// See how far we are from ideal position in cover.
		FVector	DeltaMove		= (IdealPosition - Location);
				DeltaMove.Z		= 0.f;

		// If distance is significant, then try to auto correct it
		if( DeltaMove.SizeSquared2D() > KINDA_SMALL_NUMBER )
		{
			const	FLOAT	MaxSpeed	= GroundSpeed * MaxSpeedModifier();
					FVector	MoveVel		= DeltaMove / DeltaSeconds;

			// do not exceed max speed
			if( MoveVel.SizeSquared2D() > MaxSpeed * MaxSpeed )
			{
				MoveVel = MoveVel.UnsafeNormal() * MaxSpeed;
			}

			FVector Velocity2D = FVector(Velocity.X, Velocity.Y, 0.f);
		
			// See how much of the current velocity is pushing us towards ideal position
			const FLOAT MoveVelSize2D	= MoveVel.Size2D();
			const FLOAT ExistingVelDot	= (Velocity2D | (MoveVel/MoveVelSize2D) ) / MoveVelSize2D;
			if( ExistingVelDot >= 1.f )
			{
				// If actual velocity is enough to correct position, no need to perform auto correction
				MoveVel = FVector(0.f);
			}
			else if( ExistingVelDot > 0.f )
			{
				// Otherwise add enough so sum of velocity and autocorrection doesn't exceed what we need
				MoveVel *= (1.f - ExistingVelDot);
			}

			DeltaMove = MoveVel * DeltaSeconds;

			// If move is significant, then perform it
			if( DeltaMove.SizeSquared2D() > KINDA_SMALL_NUMBER )
			{
				const FVector OldLocation = Location;

				FCheckResult Hit(1.f);
				GWorld->MoveActor(this, DeltaMove, Rotation, 0, Hit);

				// Update velocity to reflect move
				const FVector ActualMove = (Location - OldLocation);
				Velocity += (ActualMove / DeltaSeconds);
			}
		}
	}
}

/** Set new physics after landing. Overriden from APawn, since DBNO guys have below zero health, and can't be set to PHYS_None. */
void AGearPawn::SetPostLandedPhysics(AActor *HitActor, FVector HitNormal)
{
	if( !bPlayedDeath )
	{
		setPhysics(PHYS_Walking, HitActor, HitNormal);
	}
	else
	{
		setPhysics(PHYS_None, HitActor, HitNormal);
	}
}

void AGearPawn::physicsRotation(FLOAT DeltaTime, FVector OldVelocity)
{
	// If Pawn is doing a special move locking Pawn rotation, do not update the Pawn's rotation.
	if( !bLockRotation && !(SpecialMove != SM_None && SpecialMoves(SpecialMove)->bPawnRotationLocked) )
	{
		Super::physicsRotation(DeltaTime, OldVelocity);
	}
}

/** Script hook for read access to native APawn::MaxSpeedModifier() */
FLOAT  AGearPawn::GetMaxSpeedModifier()
{
	return MaxSpeedModifier();
}

/**
 * Scales the maximum velocity the Pawn can move.
 */
FLOAT AGearPawn::MaxSpeedModifier()
{
	// default running
	FLOAT Result = MovementPct;
	// check for special case walking (that overrides SM/cover)
	UBOOL bShouldWalk = bIsCrawling  || 
						(Cast<AWalkVolume>(PhysicsVolume) != NULL && Cast<AWalkVolume>(PhysicsVolume)->bActive);
						
	UBOOL bUseForceWalkSpeed = bIsConversing || bScriptedWalking ||
								( CameraVolumes.Num() > 0 && CameraVolumes(0) && CameraVolumes(0)->bForcePlayerToWalk );

	if (bIsCrawling)
	{
		//debugf(TEXT("%s crawling %2.3f/%2.3f"),*GetName(),CrawlSpeedModifier,DBNOCrawlNode->Rate);
		return 1.f + CrawlSpeedModifier;
	}
	else
	if (bShouldWalk)
	{
		Result = WalkingPct;
	}
	else if (bUseForceWalkSpeed)
	{
		Result = ForceWalkingPct;
	}
	else
	// get speed modifier for current special move
	if( SpecialMove != SM_None && SpecialMove != SM_ChainSawHold )
	{
		if( SpecialMove < SpecialMoves.Num() && SpecialMoves(SpecialMove) != NULL )
		{
			Result = SpecialMoves(SpecialMove)->GetSpeedModifier();
		}
	}
	// change maximum velocity based on cover type
	else if( CoverType != CT_None )
	{
		Result = CoverMovementPct(INT(CoverType));
		// slightly slower for circular cover
		if (CurrentLink != NULL && CurrentLink->bCircular)
		{
			Result *= 0.75f;
		}
		Result *= bIsWalking ? WalkingPct : MovementPct;

		// if carrying a heavy weapon (doesn't stack with walking)
		if( !bIsWalking ) 
		{
			if( MyGearWeapon && (MyGearWeapon->WeaponType == WT_Heavy) )
			{
				Result *= eventIsInAimingPose() ? HeavyWeaponFiringMovementSpeedPercentage : HeavyWeaponMovementSpeedPercentage;
			}
		}
	}
	else
	{
		// if crouched or walking use the modified values
		if( bIsCrouched )
		{
			Result = CrouchedPct;
		}
		else if( bIsWalking )
		{
			Result = WalkingPct;
		}
		else
		{
			// If running backwards
			if (Physics == PHYS_Walking && Velocity.Size2D() > GroundSpeed * 0.1f)
			{
				const FLOAT DotP = (Velocity.SafeNormal2D() | Rotation.Vector());
				if( DotP < -0.1f )
				{
					// Limit speed
					Result *= BackwardMovementSpeedPercentage;
				}			
			}

			// if carrying a heavy object (stacks with backwards multiplier)
			{
				if( MyGearWeapon && MyGearWeapon->WeaponType == WT_Heavy )
				{
					Result *= eventIsInAimingPose() ? HeavyWeaponFiringMovementSpeedPercentage : HeavyWeaponMovementSpeedPercentage;
				}
				else if( eventIsCarryingShield() )
				{
					Result *= eventIsInAimingPose() ? ShieldFiringMovementSpeedPercentage : ShieldMovementSpeedPercentage;
				}
			}


		}
	}

	// scale down movement if not in combat
	if (!bIsInCombat)
	{
		Result *= 0.85f;
	}

	const FLOAT StoppingPower = GetResultingStoppingPower();
	if( StoppingPower > 0.f )
	{
		Result *= (1.f - StoppingPower);
	}

	if (bAllowSpeedInterpolation)
	{
		FLOAT TmpResult = FInterpTo(LastMaxSpeedModifier,Result,WorldInfo->DeltaSeconds,4.f);
		//debugf(TEXT("%2.1f %s: speed, desired: %2.3f, last: %2.3f, current: %2.3f"),WorldInfo->TimeSeconds,*GetName(),Result,LastMaxSpeedModifier,TmpResult);
		LastMaxSpeedModifier = TmpResult;
		Result = TmpResult;
	}

	return Result;
}

FLOAT AGearPawn::GetResultingStoppingPower()
{
	// If Pawn is not moving, we can't compute Stopping Power.
	if( Velocity.SizeSquared2D() < 1.f )
	{
		return 0.f;
	}

	// See if Special Move overrides Stopping Power.
	if( SpecialMove != SM_None && SpecialMoves(SpecialMove)->bNoStoppingPower )
	{
		return 0.f;
	}

	FLOAT AccumulatedStoppingPower = 0.f;

	for(INT i=0; i<StoppingPowerList.Num(); i++)
	{
		FLOAT DotScale = StoppingPowerList(i).Direction | Velocity.SafeNormal2D();
		if( DotScale < StoppingPowerAngle )
		{
			// Make it power of 2 instead of linear scale
			DotScale = 1.f - Square(DotScale + 1.f);
			AccumulatedStoppingPower += StoppingPowerList(i).StoppingPower * DotScale * StoppingPowerList(i).Lifetime;
		}
	}

	// Reduce stopping power when carrying heavy weapons.
	if( MyGearWeapon && MyGearWeapon->WeaponType == WT_Heavy )
	{
		MaxStoppingPower *= StoppingPowerHeavyScale;
	}

	// Clamp to safe values.
	return Clamp<FLOAT>(AccumulatedStoppingPower, 0.f, Min<FLOAT>(MaxStoppingPower, 1.f));
}

void AGearPawn::UpdateCoverActionAnimTransition(INT InTickTag, UBOOL bInDoingTransition)
{
	// Reset flag once per tick
	if( CoverActionAnimUpdateTickTag != InTickTag )
	{
		CoverActionAnimUpdateTickTag	= InTickTag;
		bDoingCoverActionAnimTransition = bInDoingTransition;
	}
	// Update transition flag.
	else
	{
		bDoingCoverActionAnimTransition |= bInDoingTransition;
	}
}

void AGearPawn::UpdateTargetingNodeIsInIdleChannel(INT InTickTag, UBOOL bInAimingStance)
{
	// Reset flag once per tick
	if( TargetingNodeIsInIdleChannelTickTag != InTickTag )
	{
		TargetingNodeIsInIdleChannelTickTag	= InTickTag;
		bTargetingNodeIsInIdleChannel		= bInAimingStance;
// 		debugf(TEXT("UpdateTargetingNodeIsInIdleChannel SET %d"), bTargetingNodeIsInIdleChannel);
	}
	// Update transition flag.
	else
	{
		bTargetingNodeIsInIdleChannel |= bInAimingStance;
// 		debugf(TEXT("UpdateTargetingNodeIsInIdleChannel UPDATE %d"), bTargetingNodeIsInIdleChannel);
	}
}

FVector AGearPawn::CheckForLedges(FVector AccelDir, FVector Delta, FVector GravDir, int &bCheckedFall, int &bMustJump )
{
	if( Base != NULL )
	{
		APawn* PawnBase = Cast<APawn>(Base);
		// if we're on a pawn that can't be a base for pawns, let my people go!
		if(PawnBase != NULL && !PawnBase->bCanBeBaseForPawns)
		{
			return Delta;
		}

		FVector ColLocation = CollisionComponent ? Location + CollisionComponent->Translation : Location;

		FCheckResult chkResult;
		FLOAT TestDown = MaxStepHeight + (MAXSTEPHEIGHTFUDGE * 2.0f);
		FVector TestLocation = ColLocation;
		TestLocation.Z -= (CylinderComponent->CollisionHeight - 8.f);

		// first check distance to the surface we're standing on to grab the ignore offset				
		// initial ignore trace test further down
		const FLOAT InitialOffsetCheckDist = 2.0f * TestDown;
		if (!GWorld->SingleLineCheck(chkResult,this,TestLocation + GravDir * InitialOffsetCheckDist,TestLocation,TRACE_AllBlocking,FVector(1.f)))
		{
			TestDown += (chkResult.Location - TestLocation).Size();
		}

		// otherwise, check that the edge of our cylinder in the accel direction is sitting on something
		TestLocation = ColLocation + Delta + AccelDir * CylinderComponent->CollisionRadius;
		// adjust for collision height
		TestLocation.Z -= (CylinderComponent->CollisionHeight - 8.f);
		// check down enough to catch either step or slope
		
		FVector Extent = GetCylinderExtent() * 0.5f;
		Extent.Z = 1.f;
		if (GWorld->SingleLineCheck(chkResult,this,TestLocation + GravDir * TestDown,TestLocation,TRACE_World|TRACE_Blocking|TRACE_StopAtAnyHit,Extent))
		{
			// never allow if we must keep this base
			if (ClampedBase != Base)
			{
				// if player controlled then attempt a mantle down SM - only on local player
				AGearPC *PC = Cast<AGearPC>(Controller);
				if (PC != NULL)
				{
					// allow players to roll off ledges that could be stepped down
					if (PC->eventAllowEvadeOffLedge())
					{
						bCheckedFall = TRUE;
						bMustJump = TRUE;
						return Delta;
					}
					if ( PC->LocalPlayerController() )
					{
						PC->eventDoSpecialMove(SM_MantleDown);
					}
				}
			}
			// don't allow a move in this direction
			return FVector(0.f);
		}

		// Set flag to skip second ledge/fall check in physWalking
		bCheckedFall = (ClampedBase != Base);
		bCanJump = !bCheckedFall;
	}
	// allow the move
	return Delta;
}


static void DrawDebugSweptBox(AActor *Owner, FVector StartLoc, FVector EndLoc, FVector Extent, BYTE R, BYTE G, BYTE B)
{
	if( !Owner )
	{
		return;
	}

	const FVector BoxCenter		= (EndLoc + StartLoc) / 2.f;
	const FVector HalfLength	= (EndLoc - StartLoc) / 2.f;

	FVector BoxExtent			= Extent;
	Extent.X += Abs(HalfLength.X);
	Extent.Y += Abs(HalfLength.Y);
	Extent.Z += Abs(HalfLength.Z);

	Owner->DrawDebugBox(BoxCenter, BoxExtent, R, G, B, TRUE);
}

/** Test that Pawn is on a ledge, and can mantle down */
UBOOL AGearPawn::CanPerformMantleDown(FLOAT MinMantleHeight, FLOAT MaxMantleHeight, FRotator TestRotation)
{
	// if already falling, don't allow the move
	if( Base == NULL )
	{
		//debugf(TEXT("AGearPawn::CanPerformMantleDown - already falling, don't allow the move"));
		return FALSE;
	}

	if (TestRotation.Pitch == 0 && TestRotation.Yaw == 0)
	{
		TestRotation = Rotation;
	}

	FCheckResult Hit(1.f);
	const FVector	ColLocation	= CollisionComponent ? Location + CollisionComponent->Translation : Location;
	const FVector	DownDir		= FVector(0.f, 0.f, -1.f);

	// check down enough to catch collision
	const FVector	TestDir		= TestRotation.Vector();
	// CheckForLedges is doing extent checks, so we should already be partially floating by the time this gets called.. so only push forward 25% of collision radius
	const FVector	TestLoc		= ColLocation + TestDir*1.75f*CylinderComponent->CollisionRadius;
	const FVector	Extent		= GetCylinderExtent();
	const FVector	MinDest		= TestLoc + DownDir*MinMantleHeight;
	const FVector	MaxDest		= TestLoc + DownDir*MaxMantleHeight;

	// debugging information
	if( FALSE )
	{
		FlushPersistentDebugLines();
		
		DrawDebugSweptBox(this, ColLocation, TestLoc, Extent, 0, 255, 0);
		DrawDebugSweptBox(this, TestLoc, MinDest, Extent, 0, 255, 0);
		DrawDebugSweptBox(this, MinDest, MaxDest, Extent, 255, 0, 0);
	}

	// Front Test - if colliding against something before reaching TestLoc, we can't perform move
	if( !GWorld->SingleLineCheck(Hit, this, TestLoc, ColLocation, TRACE_AllBlocking|TRACE_StopAtAnyHit, Extent) )
	{
		//debugf(TEXT("AGearPawn::CanPerformMantleDown - colliding against something before reaching TestLoc for trace down, we can't perform move"));
		return FALSE;
	}

	// Ledge Test - if colliding against something before reaching MinMantleHeight, we can't perform move
	if( !GWorld->SingleLineCheck(Hit, this, MinDest, TestLoc, TRACE_AllBlocking|TRACE_StopAtAnyHit, Extent) )
	{
		//debugf(TEXT("AGearPawn::CanPerformMantleDown - colliding against something before reaching MinMantleHeight, we can't perform move"));
		return FALSE;
	}

	// Gound Test - if there is no ground between MinMantleHeight and MaxMantleHeight, then can't perform move.
	if( GWorld->SingleLineCheck(Hit, this, MaxDest, MinDest, TRACE_AllBlocking|TRACE_StopAtAnyHit, Extent) )
	{
		//debugf(TEXT("AGearPawn::CanPerformMantleDown - there is no ground between MinMantleHeight and MaxMantleHeight, we can't perform move"));
		return FALSE;
	}

	return TRUE;
}


/** 
 * Try to fit collision within world.
 * Called after turning collision back on, to make sure Pawn will not fall through world
 */
UBOOL AGearPawn::FitCollision()
{
	const	FVector	LocOffset		= CollisionComponent ? CollisionComponent->Translation : FVector(0.f);
			FVector	TestLocation	= Location + LocOffset;
			FVector	BackupTestLoc	= TestLocation;
			FVector Extent			= GetCylinderExtent();

	// debugging information
	if( FALSE )
	{
		DrawDebugBox(TestLocation, Extent, 255, 0, 0, TRUE);
		debugf(TEXT("AGearPawn::FitCollision"));
	}

	if( GWorld->FindSpot(Extent, TestLocation, bCollideComplex) )
	{
		if( TestLocation != BackupTestLoc )
		{
			//debugf(TEXT("AGearPawn::FitCollision, moved actor by: %s"), *((TestLocation-BackupTestLoc).ToString()));
			const FVector NewActorLoc = TestLocation - LocOffset;
			GWorld->FarMoveActor(this, NewActorLoc, 0, 0);
		}

		return TRUE;
	}

	// failsafe - find nearest navigation node and try to put the player there
	const FLOAT NAV_CHECK_RADIUS = 128.f;
	TArray<FNavigationOctreeObject*> NavObjects;
	GWorld->NavigationOctree->RadiusCheck(Location, NAV_CHECK_RADIUS, NavObjects);
	ANavigationPoint* Best = NULL;
	FLOAT BestDistSquared = 1000000.f;
	for (INT i = 0; i < NavObjects.Num(); i++)
	{
		ANavigationPoint* Nav = NavObjects(i)->GetOwner<ANavigationPoint>();
		// avoid nodes unusable as anchors
		if (Nav != NULL && Nav->IsUsableAnchorFor(this))
		{
			FVector Diff = Nav->Location - Location;
			// double influence of Z-axis so that we prefer nodes with similar Z
			FLOAT DistSquared = Square(Diff.X) + Square(Diff.Y) + Square(Diff.Z * 2.f);
			if (DistSquared < BestDistSquared)
			{
				Best = Nav;
				BestDistSquared = DistSquared;
			}
		}
	}
	if (Best != NULL && GWorld->FarMoveActor(this, Best->Location, FALSE, FALSE))
	{
		return TRUE;
	}

	debugf(NAME_Warning, TEXT("GearPawn::FitCollision() failed to find a good spot for %s"), *GetName());
	return FALSE;
}

void AGearPawn::OnRigidBodyCollision(const FRigidBodyCollisionInfo& MyInfo, const FRigidBodyCollisionInfo& OtherInfo, const FCollisionImpactData& RigidCollisionData)
{
	// If we've hit the world forward notification to special move.
	if( SpecialMove != SM_None && (!OtherInfo.Actor || OtherInfo.Actor->bWorldGeometry) )
	{
		SpecialMoves(SpecialMove)->eventRigidBodyWorldCollision(MyInfo.Component, OtherInfo.Component, RigidCollisionData);
	}
}


/** Start AnimControl. Add required AnimSets. */
void AGearPawn::MAT_BeginAnimControl(const TArray<class UAnimSet*>& InAnimSets)
{
	if( !Mesh )
	{
		debugf(TEXT("MAT_BeginAnimControl, no Mesh!!!") );
		return;
	}

	// Add passed in AnimSets to the AnimSet Array.
	for(INT i=0; i<InAnimSets.Num(); i++)
	{
		Mesh->AnimSets.AddItem( InAnimSets(i) );
	}

	// Keep track of the sets we've been adding, for later removal.
	MATAnimSets.Empty();
	MATAnimSets = InAnimSets;
}


/** Update AnimTree from track info */
void AGearPawn::MAT_SetAnimPosition(FName SlotName, INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition, UBOOL bFireNotifies, UBOOL bLooping)
{
	// Forward animation positions to slots. They will forward to relevant channels.
	for(INT i=0; i<BodyStanceNodes.Num(); i++)
	{
		UAnimNodeSlot* SlotNode = BodyStanceNodes(i);
		if( SlotNode && SlotNode->NodeName == SlotName )
		{
			SlotNode->MAT_SetAnimPosition(ChannelIndex, InAnimSeqName, InPosition, bFireNotifies, bLooping);
		}
	}
}

/** Update AnimTree from track weights */
void AGearPawn::MAT_SetAnimWeights(const TArray<FAnimSlotInfo>& SlotInfos)
{
#if 0
	debugf( TEXT("-- SET ANIM WEIGHTS ---") );
	for(INT i=0; i<SlotInfos.Num(); i++)
	{
		debugf( TEXT("SLOT: %s"), *(SlotInfos(i).SlotName) );
		for(INT j=0; j<SlotInfos(i).ChannelWeights.Num(); j++)
		{
			debugf( TEXT("   CHANNEL %d: %1.3f"), j, SlotInfos(i).ChannelWeights(j) );
		}
	}
#endif

	// Forward channel weights to relevant slot(s)
	for(INT SlotInfoIdx=0; SlotInfoIdx<SlotInfos.Num(); SlotInfoIdx++)
	{
		const FAnimSlotInfo& SlotInfo = SlotInfos(SlotInfoIdx);

		for(INT SlotIdx=0; SlotIdx<BodyStanceNodes.Num(); SlotIdx++)
		{
			UAnimNodeSlot* SlotNode = BodyStanceNodes(SlotIdx);
			if( SlotNode && SlotNode->NodeName == SlotInfo.SlotName )
			{	
				SlotNode->MAT_SetAnimWeights(SlotInfo);
			}
		}
	}
}


/** End AnimControl. Release required AnimSets */
void AGearPawn::MAT_FinishAnimControl()
{
	// Clear references to AnimSets which were added.
	
	// Start looking from the end, as the sets have been added at the end of the list.
	for(INT i=Mesh->AnimSets.Num() - 1; i>=0; i--)
	{
		for(INT j=MATAnimSets.Num(); j>=0; j--)
		{
			if( Mesh->AnimSets(i) == MATAnimSets(j) )
			{
				Mesh->AnimSets.Remove(i);
				break;
			}
		}
	}

	MATAnimSets.Empty();
}


/* This is where we add information on slots and channels to the array that was passed in. */
void AGearPawn::GetAnimControlSlotDesc(TArray<struct FAnimSlotDesc>& OutSlotDescs)
{
	if( !Mesh )
	{
		debugf(TEXT("AGearPawn::GetAnimControlSlotDesc, no Mesh!!!") );
		return;
	}

	if( !Mesh->Animations )
	{
		// fail
		appMsgf(AMT_OK, TEXT("SkeletalMeshActorMAT has no AnimTree Instance."));
		return;
	}

	// Find all AnimSlotNodes in the AnimTree
	for(INT i=0; i<BodyStanceNodes.Num(); i++)
	{
		// Number of channels available on this slot node.
		const INT NumChannels = BodyStanceNodes(i)->Children.Num() - 1;

		if( BodyStanceNodes(i)->NodeName != NAME_None && NumChannels > 0 )
		{
			// Add a new element
			const INT Index = OutSlotDescs.Add(1);
			OutSlotDescs(Index).SlotName	= BodyStanceNodes(i)->NodeName;
			OutSlotDescs(Index).NumChannels	= NumChannels;
		}
	}
}

/** Utility for scaling joint limits down of specific bones. */
void AGearPawn::ReduceConstraintLimits()
{
	if( TimeToScaleLimits <= 0.f )
	{
		warnf(TEXT("%3.2f %s - AGearPawn::ReduceConstraintLimits. Called with a Zero TimeToScaleLimits!! Nothing is being done."), GWorld->GetTimeSeconds(), *GetName());
	}

	ScaleLimitTimeToGo = TimeToScaleLimits;
}

void AGearPawn::PreviewBeginAnimControl(TArray<class UAnimSet*>& InAnimSets)
{
	if( !Mesh )
	{
		debugf(TEXT("AGearPawn::PreviewBeginAnimControl, no Mesh!!!") );
		return;
	}

	// We need an AnimTree in Matinee in the editor to preview the animations, so instance one now if we don't have one.
	// This function can get called multiple times, but this is safe - will only instance the first time.
	if( !Mesh->Animations && Mesh->AnimTreeTemplate )
	{
		Mesh->Animations = Mesh->AnimTreeTemplate->CopyAnimTree(Mesh);
	}

	// Add in AnimSets
	MAT_BeginAnimControl(InAnimSets);

	// Initialize tree
	Mesh->InitAnimTree();
}

void AGearPawn::PreviewSetAnimPosition(FName SlotName, INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition, UBOOL bLooping)
{
	if( !Mesh )
	{
		debugf(TEXT("AGearPawn::PreviewSetAnimPosition, no Mesh!!!") );
		return;
	}

	MAT_SetAnimPosition(SlotName, ChannelIndex, InAnimSeqName, InPosition, FALSE, bLooping);

	// Update space bases so new animation position has an effect.
	Mesh->UpdateSkelPose(0.f, FALSE);
	Mesh->ConditionalUpdateTransform();
}

void AGearPawn::PreviewSetAnimWeights(TArray<FAnimSlotInfo>& SlotInfos)
{
	MAT_SetAnimWeights(SlotInfos);
}

void AGearPawn::PreviewFinishAnimControl()
{
	if( !Mesh )
	{
		debugf(TEXT("AGearPawn::PreviewFinishAnimControl, no Mesh!!!") );
		return;
	}

	MAT_FinishAnimControl();

	// When done in Matinee in the editor, drop the AnimTree instance.
	Mesh->Animations = NULL;

	// Update space bases to reset it back to ref pose
	Mesh->UpdateSkelPose(0.f, FALSE);
	Mesh->ConditionalUpdateTransform();
}

void AGearPC::PreBeginPlay()
{
	MyGuid = appCreateGuid();

	Super::PreBeginPlay();
}

void AGearPC::CopyCheckpointMusicInfo(const FCheckpointMusicRecord& SourceInfo, FMusicTrackStruct& DestInfo)
{
	DestInfo = SourceInfo;
}

UBOOL AGearPawn::GetMoveDelta(FVector& out_Delta, const FVector &Dest)
{
	out_Delta = Dest - Location;
	if(!bAllowTurnSmoothing || Controller == NULL || Controller->MoveTarget != CurrentSplineActor)
	{
		return FALSE;
	}

	FVector PrevPt, CurrentPt, FinalPt;
	PrevPt = CurrentPt = FinalPt = FVector(0.f);
	if(PrevSplineActor != NULL && !PrevSplineActor->ActorIsPendingKill())
	{
		PrevPt = PrevSplineActor->GetDestination(Controller);
	}

	if(CurrentSplineActor != NULL && !CurrentSplineActor->ActorIsPendingKill())
	{
		CurrentPt = CurrentSplineActor->GetDestination(Controller);
	}
	
	if(FinalSplineActor != NULL && !FinalSplineActor->ActorIsPendingKill())
	{
		FinalPt = FinalSplineActor->GetDestination(Controller);
	}
	else
	{
		FinalPt = *FinalSplinePt;
	}

	//DrawDebugLine(Location,Location+(Acceleration.SafeNormal() * 100.f),255,128,0);
	//DrawDebugCoordinateSystem(PrevPt,FRotator(0,0,0),50.f);
	//DrawDebugCoordinateSystem(CurrentPt,FRotator(0,0,0),50.f);
	//DrawDebugCoordinateSystem(FinalPt,FRotator(0,0,0),50.f);
	//DrawDebugCoordinateSystem(Location,Rotation,10.f);

	AGearAI* GAI = Cast<AGearAI>(Controller);
	if(GAI != NULL && Controller->RouteCache.Num() > 0 && !PrevPt.IsNearlyZero() && !CurrentPt.IsNearlyZero() && !FinalPt.IsNearlyZero() )
	{		
		FVector PathDir = (CurrentPt - PrevPt);
		const FLOAT PathSize = PathDir.Size();

		if(PathSize <= SMALL_NUMBER)
		{
			return FALSE;
		}
		PathDir /= PathSize;
		const FLOAT DistForward = (PathDir | (Location - PrevPt));
		const FVector ClosestPathPt = PrevPt + (PathDir * DistForward);
		//DrawDebugCoordinateSystem(ClosestPathPt,PathDir.Rotation(),10.f,TRUE);
		checkSlow(!ClosestPathPt.ContainsNaN());

		//const FLOAT DistForward = (ClosestPathPt - PrevPt).Size();
		FLOAT t = 0.f;
		if(DistForward >= 1.f)
		{
			t = Clamp<FLOAT>(DistForward / PathSize,0.f,1.f);
		}

	
		// if we haven't started the turn yet check to see if it's safe to do so
		if(t >= 0.5f && SmoothTurnStartT<=0.f)
		{
			
			const FVector FarLookPt = CurrentPt + ((1.0f-t) * (FinalPt - CurrentPt));
			const FVector MidPt = (ClosestPathPt + (0.5f * (FarLookPt - ClosestPathPt)));
			
			FCheckResult Hit(1.0f);
			FVector Extent = GetCylinderExtent();
			Extent.Z *= 0.5f;
			if( GWorld->SingleLineCheck(Hit,this,MidPt,ClosestPathPt,TRACE_World|TRACE_StopAtAnyHit,Extent)  )	
			{
				if( GWorld->SingleLineCheck(Hit,this,FarLookPt,MidPt,TRACE_World|TRACE_StopAtAnyHit,Extent)  )
				{
					SmoothTurnStartT=t;
				}
				else
				{
					//DrawDebugLine(FarLookPt,MidPt,255,0,0);
				}
			}
			else
			{
				//DrawDebugLine(MidPt,ClosestPathPt,255,0,0);
			}
		}
		else if(t < 0.5f)
		{
			SmoothTurnStartT=-1.0f;
		}

		if(SmoothTurnStartT>0.f)
		{
			FLOAT CollisionRadius = GetCylinderExtent().X;
			const FVector FarLookPt = CurrentPt + ((t-SmoothTurnStartT) * (FinalPt - CurrentPt));
			checkSlow(!FarLookPt.ContainsNaN());
			FVector FarLookDelta = (FarLookPt - ClosestPathPt);
			FVector MovePointDelta = (ClosestPathPt + (t * FarLookDelta)) - Location;
			out_Delta = MovePointDelta.SafeNormal() * (FarLookPt - Location).Size2D();
			if((out_Delta | FarLookDelta) < 0.f || MovePointDelta.Size2D() < MovePointDelta.Z)
			{
				out_Delta = FarLookDelta;
			}
			checkSlow(!out_Delta.ContainsNaN());

			//// line we're following
			//DrawDebugLine(ClosestPathPt,FarLookPt,0,255,0,FALSE);


			////// corner we're taking
			//DrawDebugLine(PrevPt,PrevPt + (CurrentPt-PrevPt)*0.9f,0,0,255,FALSE);
			//DrawDebugLine(CurrentPt,FinalPt,0,128,255,FALSE);

			//DrawDebugCoordinateSystem(FarLookPt,FRotator(0,0,0),50.f,FALSE);
			//DrawDebugCoordinateSystem(ClosestPathPt,PathDir.Rotation(),10.f,FALSE);
			//DrawDebugLine(Location,Location+out_Delta*50.f,255,0,0,FALSE);
			//DrawDebugLine(PrevPt+FVector(0.f,0.f,10.f),ClosestPathPt+FVector(0.f,0.f,10.f),255,255,128,FALSE);


			//debugf(TEXT("%s Rad: %.2f Dist: %.2f t: %.2f"),*MyGearAI->GetName(),CollisionRadius,(Location - FarLookPt).SizeSquared2D(),t);
			if((Location - FarLookPt).SizeSquared2D() < CollisionRadius*CollisionRadius)
			{
				return TRUE;
				//SmoothTurnStartT=-1.f;
			}

		}
	}

	return FALSE;
}

UBOOL AGearPawn::moveToward(const FVector &Dest, AActor *GoalActor )
{
	//DrawDebugLine(Location,Dest,255,255,255);
	if(Physics != PHYS_Walking)
	{
		return Super::moveToward(Dest,GoalActor);
	}

	if ( !Controller )
	{
		return FALSE;
	}

	if ( Controller->bAdjusting )
	{
		GoalActor = NULL;
	}

	FVector Direction;
	if(GetMoveDelta(Direction,Dest)==TRUE)
	{
		//debugf(TEXT("moveToward for %s (%2.3f) exiting because GetMoveDelta told me to! GoalActor: %s"),*GetName(),GWorld->GetTimeSeconds(),(GoalActor)? *GoalActor->GetName() : TEXT("NULL"));
		// if Pawn just reached a navigation point, set a new anchor
		ANavigationPoint *Nav = Cast<ANavigationPoint>(GoalActor);
		if ( Nav )
		{
			SetAnchor(Nav);
		}

		// pull the current goal out of the route cache so we don't try to go to it again
		Controller->RouteCache_RemoveIndex(0);
		EffectiveTurningRadius=TurningRadius;
		return true;
	}
	checkSlow(!Direction.ContainsNaN());

	FLOAT ZDiff = Direction.Z;

	Direction.Z = 0.f;

	if ( Controller->MoveTarget && Controller->MoveTarget->IsA(APickupFactory::StaticClass()) 
		&& (Abs(Location.Z - Controller->MoveTarget->Location.Z) < CylinderComponent->CollisionHeight)
		&& (Square(Location.X - Controller->MoveTarget->Location.X) + Square(Location.Y - Controller->MoveTarget->Location.Y) < Square(CylinderComponent->CollisionRadius)) )
	{
		Controller->MoveTarget->eventTouch(this, this->CollisionComponent, Location, (Controller->MoveTarget->Location - Location) );
	}

	FLOAT Distance = Direction.Size();
	
	FCheckResult Hit(1.f);

	if ( ReachedDestination(Location, Dest, GoalActor) )
	{
		// if Pawn just reached a navigation point, set a new anchor
		ANavigationPoint *Nav = Cast<ANavigationPoint>(GoalActor);
		if ( Nav )
			SetAnchor(Nav);

		Controller->SetFocalPoint( Location + Rotation.Vector()*5.0f, (GoalActor && GoalActor->Base == Base) );
		EffectiveTurningRadius=TurningRadius;
		return true;
	}
	else 
	// if walking, and within radius, and goal is null or
	// the vertical distance is greater than collision + step height and trace hit something to our destination
	if (Physics == PHYS_Walking 
		&& Distance < CylinderComponent->CollisionRadius &&
		(GoalActor == NULL ||
		(ZDiff > CylinderComponent->CollisionHeight + 2.f * MaxStepHeight && 
		!GWorld->SingleLineCheck(Hit, this, Dest, Location, TRACE_World))))
	{
		Controller->eventMoveUnreachable(Dest,GoalActor);
		Controller->SetFocalPoint( FVector(0.f) );
		EffectiveTurningRadius=TurningRadius;
		return true;
	}
	else if ( Distance > 0.f )
	{
		Direction = Direction/Distance;
	}

	FLOAT speed = Velocity.Size();

	// set turning radius based on how close to our goal we are (consumed later by peformPhysics)
	FLOAT PctThere = Clamp<FLOAT>(Distance/AccelConvergeFalloffDistance,0.f,1.f);
	EffectiveTurningRadius = Min<FLOAT>(EffectiveTurningRadius,TurningRadius*PctThere*PctThere);

	//DrawDebugCylinder(Dest,Dest,EffectiveTurningRadius,16,255,0,0);

	Acceleration = Direction * AccelRate;

	if ( !Controller->bAdjusting && Controller->MoveTarget && Controller->MoveTarget->GetAPawn() )
	{
		UBOOL bDone = (Distance < CylinderComponent->CollisionRadius + Controller->MoveTarget->GetAPawn()->CylinderComponent->CollisionRadius + 0.8f * MeleeRange);
		if(bDone)
		{
			Controller->SetFocalPoint( FVector(0.f) );
		}
		EffectiveTurningRadius=TurningRadius;
		return bDone;
	}		

	if ( (speed > FASTWALKSPEED) )
	{
		//		FVector VelDir = Velocity/speed;
		//		Acceleration -= 0.2f * (1 - (Direction | VelDir)) * speed * (VelDir - Direction);
	}
	if ( Distance < 1.4f * AvgPhysicsTime * speed )
	{
		// slow pawn as it nears its destination to prevent overshooting
		if ( !bReducedSpeed ) 
		{
			//haven't reduced speed yet
			DesiredSpeed = 0.51f * DesiredSpeed;
			bReducedSpeed = 1;
		}
		if ( speed > 0.f )
			DesiredSpeed = Min(DesiredSpeed, (2.f*FASTWALKSPEED)/speed);
	}
	return false;
}


/**
* This function returns the physical fire start location, from weapon's barrel in world coordinates.
* Typically this is an offset from the weapon hand socket given by the current weapon.
* it is used to spawn effects/projectiles from the actual weapon's barrel.
* Network: ALL
*/
FVector AGearPawn::GetPhysicalFireStartLoc( FVector FireOffset )
{
	FVector	SocketLoc(0,0,0), X, Y, Z;
	FRotator SocketRot(0,0,0);

	if(bUseSimplePhysicalFireStartLoc)
	{
		return Location + FRotationMatrix(Rotation).TransformFVector(FireOffset);
	}


	if (Weapon != NULL)
	{
		return Weapon->eventGetPhysicalFireStartLoc();
	}

	GetWeaponHandPosition(SocketLoc, SocketRot);
	FRotationMatrix RMat(SocketRot);
	RMat.GetAxes(X,Y,Z);
	return (SocketLoc + FireOffset.X*X + FireOffset.Y*Y + FireOffset.Z*Z);
}


/**
* Returns the weapon hand bone position for this pawn's mesh
* @ param HandLoc is an out parameter returning the weapon hand bone location
* @ param HandRot is an out parameter returning the weapon hand bone rotation
*/
void AGearPawn::GetWeaponHandPosition(FVector& HandLoc, FRotator& HandRot)
{
	if(Mesh == NULL)
	{
		return;
	}

	Mesh->GetSocketWorldLocationAndRotation(GetRightHandSocketName(), HandLoc, &HandRot);
}

/**
* Returns socket used for Right Hand (main weapon hand).
* When using animation mirroring, left and right hand sockets are inverted.
*/
FName AGearPawn::GetRightHandSocketName()
{
	// we have inverted sockets because of animation mirroring
	// Use bWantsToBeMirrored instead of bIsMirrored to catch weapon attaching early before mirror transition is happening.
	if ((MirrorNode != NULL && MirrorNode->bPendingIsMirrored) || (CrateMirrorNode != NULL && CrateMirrorNode->bEnableMirroring))
	{
		return LeftHandSocketName;
	}
	return RightHandSocketName;
}

// That's not the most elegant. but we have to ship! :)
UBOOL AGearPawn_LocustKantusBase::IsThrowingOffHandGrenade()
{
	AGearWeap_PistolBase* Pistol = Cast<AGearWeap_PistolBase>(MyGearWeapon);
	return( Pistol && Pistol->GrenWeap && Pistol->GrenWeap->IsTimerActive(FName(TEXT("ThrowingOffHandGrenade"))) );
}

UBOOL AGearPawn_LocustKantusBase::CanFireWeapon(UBOOL bTestingForTargeting)
{
	if( IsThrowingOffHandGrenade() )
	{
		return FALSE;
	}

	return Super::CanFireWeapon(bTestingForTargeting);
}


/**
* We override this as we have pawns leaning out of cover and their view location is really where
* they are peeking out from.  We are using the NeckBone location as that is what we are using in other
* parts of the code.  We should probably make a more general socket based mechanic for this.
**/
FVector AGearPawn::GetPawnViewLocation()
{
	if (bIsGore)
	{
		return LocationWhenKilled;
	}
	else if (Mesh && IsDBNO())
	{
		return Mesh->GetBoneLocation( PelvisBoneName ) + FVector(0.f,0.f,32.f);
	}
	else if (Mesh && NeckBoneName != NAME_None)
	{
		return Mesh->GetBoneLocation( NeckBoneName ) + ViewLocationOffset * Mesh->GetBoneAxis( NeckBoneName, AXIS_Y );
	}
	else
	{
		// just used the location of the pawn. (this is good for vehicles and other pawns which are missing neckbonenames)
		return Location + FVector(0.0f,0.0f,1.0f) * BaseEyeHeight;
	}
}

FRotator AGearPawn::GetViewRotation()
{
	// if we're in cover, we're actually looking a different rotation than our pawn is facing
	if (MyGearAI != NULL && MyGearAI->FireTarget != NULL && IsInCover())
	{
		FVector TargetLoc = (MyGearAI->FireTarget == MyGearAI->Enemy) ? MyGearAI->GetEnemyLocation(MyGearAI->Enemy) : MyGearAI->FireTarget->Location;
		return (TargetLoc - (Location + FVector(0.f,0.f,1.f) * BaseEyeHeight)).Rotation();
	}
	else
	{
		return Super::GetViewRotation();
	}
}

UBOOL AGearPawn::IsProtectedByCover( FVector ShotDirection, UBOOL bSkipPlayerCheck )
{
	FVector2D	AngularDist;
	FVector	AxisX, AxisY, AxisZ;
	UBOOL		bIsInFront;

	// no protection while sliding to cover
	if (SpecialMove == SM_Run2MidCov || SpecialMove == SM_Run2StdCov)
	{
		return FALSE;
	}

	// Make sure player has valid cover
	// This CoverProtection FOV is not applied to AI characters.
	if (CoverType == CT_None ||	(!bSkipPlayerCheck && !IsPlayerOwned()))
	{
		return FALSE;
	}

	// If Pawn is leaning, then cover is not safe
	if (CoverAction == CA_LeanLeft ||
		CoverAction == CA_LeanRight ||
		CoverAction == CA_PopUp)
	{
		return FALSE;
	}

	FRotationMatrix RMat(Rotation);
	RMat.GetAxes(AxisX, AxisY, AxisZ);
	ShotDirection = -1.f * (ShotDirection).SafeNormal();
	bIsInFront = GetAngularDistance(AngularDist, ShotDirection, AxisX, AxisY, AxisZ );
	
	// convert to deg
	AngularDist.X *= 180.0f/PI;
	AngularDist.Y *= 180.0f/PI;

	// check shot's angle of attack against cover's orientation
	return (bIsInFront && Abs<FLOAT>(AngularDist.X) < CoverProtectionFOV.X && Abs<FLOAT>(AngularDist.Y) < CoverProtectionFOV.Y);
}

void AGearPC::Shot_360()
{
	AGearPlayerCamera *Cam = Cast<AGearPlayerCamera>(PlayerCamera);
	if (Cam != NULL)
	{
		const INT MaxShotCount = 36;
		Cam->CameraStyle = FName(TEXT("FreeCam"));
		Cam->eventUpdateCamera(0.05f);
		FRotator ViewRotation = Rotation;
		INT YawAmt = 65535/MaxShotCount;
		for (INT ShotCnt = 0; ShotCnt < MaxShotCount; ShotCnt++)
		{
			debugf(TEXT("- shot %d"),ShotCnt);
			GEngine->GameViewport->Exec(TEXT("SHOT"),*GLog);
			((UGameEngine*)GEngine)->RedrawViewports();
			SetRotation(Rotation + FRotator(0,YawAmt,0));
			Cam->eventUpdateCamera(0.05f);
		}
	}
}



void AGearPC::PreCacheAnimations()
{
// this is no longer needed as our animations are always in memory.
}

void AGearPC::ClientPlayMovie(const FString& MovieName)
{
	// make sure to kill loading movie first, if necessary
	ShowLoadingMovie(FALSE);
	if( GFullScreenMovie )
	{
		UINT MovieFlags = MM_PlayOnceFromStream | MF_OnlyBackButtonSkipsMovie;

		// Don't allow clients to pause movies.  Only the host can do that, and only if there
		// are no remote players.
		UBOOL bAllowPausingDuringMovies = ( GWorld == NULL || GWorld->GetNetMode() != NM_Client );

		// Check player controllers to make sure they're all local.  If we have any remote players
		// then we won't allow the host to pause movies
		if( GWorld != NULL && GWorld->GetWorldInfo() != NULL )
		{
			AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
			for( AController* CurController = WorldInfo->ControllerList;
				 CurController != NULL;
				 CurController = CurController->NextController )
			{
				// We only care about player controllers
				APlayerController* PlayerController = Cast< APlayerController >( CurController );
				if( PlayerController != NULL )
				{
					if( !PlayerController->IsLocalPlayerController() )
					{
						// Remote player, so disallow pausing
						bAllowPausingDuringMovies = FALSE;
					}
				}
			}
		}

		if( bAllowPausingDuringMovies )
		{
			MovieFlags |= MF_AllowUserToPause;
		}

		GFullScreenMovie->GameThreadPlayMovie( ( EMovieMode )MovieFlags, *MovieName);
	}
}

/**
 * Stops the currently playing movie
 *
 * @param	DelayInSeconds			number of seconds to delay before stopping the movie.
 * @param	bAllowMovieToFinish		indicates whether the movie should be stopped immediately or wait until it's finished.
 * @param	bForceStopNonSkippable	indicates whether a movie marked as non-skippable should be stopped anyway; only relevant if the specified
 *									movie is marked non-skippable (like startup movies).
 */
void AGearPC::ClientStopMovie( FLOAT DelayInSeconds/*=0*/, UBOOL bAllowMovieToFinish/*=TRUE*/, UBOOL bForceStopNonSkippable/*=FALSE*/, UBOOL bForceStopLoadingMovie /*= TRUE*/ )
{
	if (GFullScreenMovie != NULL)
	{
		if(!bForceStopLoadingMovie && GFullScreenMovie->GameThreadIsMoviePlaying(UCONST_LOADING_MOVIE))
		{
			return;
		}

		GFullScreenMovie->GameThreadStopMovie(DelayInSeconds, bAllowMovieToFinish, bForceStopNonSkippable);
	}
}

void AGearPC::GetCurrentMovie(FString& MovieName)
{
	//@todo: need to get current time in movie, too
	if( GFullScreenMovie && 
		GFullScreenMovie->GameThreadIsMoviePlaying(TEXT("")) )
	{
		MovieName = GFullScreenMovie->GameThreadGetLastMovieName();
	}
	else
	{
		MovieName = TEXT("");
	}
}

void AGearPC::SetAlternateControls(UBOOL bNewUseAlternateControls)
{
	if (bUseAlternateControls != bNewUseAlternateControls)
	{
		bUseAlternateControls = bNewUseAlternateControls;
		if (MainPlayerInput != NULL)
		{
			MainPlayerInput->eventInitializeButtonHandlingHandlers();
		}
		if (WorldInfo->NetMode == NM_Client)
		{
			eventServerSetAlternateControls(bNewUseAlternateControls);
		}
	}
}

UBOOL AGearPC::HearSound(USoundCue* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed)
{
	// ignore sounds played on others when we're a dedicated server spectator
	if (!bDedicatedServerSpectator || SoundPlayer == this)
	{
		return Super::HearSound(InSoundCue, SoundPlayer, SoundLocation, bStopWhenOwnerDestroyed);
	}
	return FALSE;
}

UBOOL AGearPC::Tick(FLOAT DeltaTime, ELevelTick TickType)
{
	// special hack to have the server tick physics for remote clients while mantling
	if (RemoteRole == ROLE_AutonomousProxy && !LocalPlayerController() && MyGearPawn != NULL && MyGearPawn->SpecialMove == SM_MidLvlJumpOver)
	{
		MyGearPawn->performPhysics(DeltaTime);
		ServerTimeStamp = WorldInfo->TimeSeconds;
	}

	UBOOL bResult = Super::Tick(DeltaTime, TickType);

	if (TutorialMgr && TutorialMgr->bTutorialSystemIsActive)
	{
		TutorialMgr->Update();
	}

	if (Role == ROLE_Authority)
	{
		eventCheckForInteractionEvents();
	}

	// handle asynch ClientVerifyState() and ServerVerifyState() requests
	if (bRequestClientVerifyState)
	{
		if (Role == ROLE_Authority && !LocalPlayerController())
		{
			eventClientVerifyState(GetStateFrame()->StateNode != NULL ? GetStateFrame()->StateNode->GetFName() : NAME_None);
		}
		bRequestClientVerifyState = FALSE;
	}
	if (bRequestServerVerifyState)
	{
		if (Role != ROLE_Authority)
		{
			eventServerVerifyState(GetStateFrame()->StateNode != NULL ? GetStateFrame()->StateNode->GetFName() : NAME_None);
		}
		bRequestServerVerifyState = FALSE;
	}

#if !PS3
	if(CompressScreenshotTask != NULL)
	{
		FAsyncCompressScreenshot* Task = (FAsyncCompressScreenshot*)CompressScreenshotTask;
		if(Task->IsDone())
		{
			eventScreenshotCompressed(Task->GetCompressedData(),Task->GetThumbnailData());
			delete Task;
			CompressScreenshotTask = NULL;
		}
	}
#endif

	return bResult;
}

void AGearPC::PostNetReceive()
{
	Super::PostNetReceive();

	MyGearPawn = Cast<AGearPawn>(Pawn);
}

UBOOL AGearPawn::SetCoverInfoFromSlotInfo(FCovPosInfo& OutCovInfo, FCoverInfo SlotInfo)
{
	if( SlotInfo.Link == NULL )
	{
		return FALSE;
	}

	const INT NumSlots		= SlotInfo.Link->Slots.Num();
	ACoverLink* Link		= SlotInfo.Link;

	OutCovInfo.Link			= Link;
	if (NumSlots == 1)
	{
		OutCovInfo.LtSlotIdx = 0;
		OutCovInfo.RtSlotIdx = 0;
	}
	else
	{
		OutCovInfo.LtSlotIdx	= (SlotInfo.SlotIdx == NumSlots-1) ? SlotInfo.SlotIdx - 1 : SlotInfo.SlotIdx;
		OutCovInfo.RtSlotIdx	= OutCovInfo.LtSlotIdx + 1;
	}
	OutCovInfo.LtToRtPct	= (SlotInfo.SlotIdx == OutCovInfo.LtSlotIdx ? 0.f : 1.f);
	OutCovInfo.Location		= (SlotInfo.SlotIdx == OutCovInfo.LtSlotIdx ? Link->GetSlotLocation(OutCovInfo.LtSlotIdx) : Link->GetSlotLocation(OutCovInfo.RtSlotIdx));

	// Set proper normal and tangent

	// Circular cover
	if( Link->bCircular )
	{
		OutCovInfo.Normal		= (OutCovInfo.Location - Link->CircularOrigin).SafeNormal2D();
		OutCovInfo.Tangent		= OutCovInfo.Normal ^ FVector(0,0,1.f);
	}
	// Single slot nodes
	else if( NumSlots == 1 )
	{
		OutCovInfo.Normal		= Link->GetSlotRotation(0).Vector().SafeNormal2D() * -1.f;
		OutCovInfo.Tangent		= OutCovInfo.Normal ^ FVector(0,0,1.f);
	}
	// otherwise look for a pair of bounding slots
	else if( NumSlots >= 2 )
	{
		const FVector LtSlotLoc	= Link->GetSlotLocation(OutCovInfo.LtSlotIdx);
		const FVector RtSlotLoc	= Link->GetSlotLocation(OutCovInfo.RtSlotIdx);
		const FVector LtToRtVec = RtSlotLoc - LtSlotLoc;
		const FVector LtToRtDir = LtToRtVec.SafeNormal();

		OutCovInfo.Normal		= FVector(0,0,1.f) ^ LtToRtDir;
		OutCovInfo.Tangent		= LtToRtDir;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

UBOOL AGearPawn::FindCoverFromLocAndDir(FVector FromLoc, FVector Direction, FLOAT FOV, FLOAT MaxDistance, FCovPosInfo& OutCovPosInfo)
{
	// Minim distance has to be slightly large than Pawn collision radius.
	const FLOAT MinDistance = CylinderComponent->CollisionRadius * 1.1f;
	MaxDistance = Max<FLOAT>(MaxDistance, MinDistance);

	// be safe, Direction needs to be normalized in 2d space
	Direction = Direction.SafeNormal2D();

	// push out from any walls we may be directly facing
	FCheckResult Hit;
	if (!GWorld->SingleLineCheck(Hit,this,FromLoc + (Direction * MinDistance * 1.25f),FromLoc,TRACE_World|TRACE_StopAtAnyHit))
	{
		FromLoc -= Direction * MinDistance * 0.5f;
	}

	UBOOL bFoundCover = FALSE;
	FCovPosInfo	PotentialCover;
	FLOAT MaxDistanceSquared = MaxDistance * MaxDistance;
	FLOAT MinDotFOV = 1.f - Clamp<FLOAT>(FOV, 0.f, 1.f);

	// Made a bit wider to account for large circular cover.
	FLOAT NavSearchRadius = MaxDistance + 512.f;

#if !FINAL_RELEASE	// COVER_DEBUG
	// if visually debugging cover then do the init stuff
	if( bCoverDebug_PlayerFOV || 
		bCoverDebug_CoverFOV ||
		bCoverDebug_CoverVolume ||
		bCoverDebug_ConsideredLinks || 
		bCoverDebug_FoundCover )
	{
		//debugf(TEXT("%3.2f FindCoverFromLocAndDir"), GWorld->GetTimeSeconds());
		FlushPersistentDebugLines();
	}

	// Debugging: Draw player search FOV.
	if( bCoverDebug_PlayerFOV )
	{
		const FLOAT	Angle2 = appAcos(MinDotFOV);
		DrawDebugCylinder(FromLoc, FromLoc, MaxDistance, 32,255,0,0,TRUE);
		DrawDebugCylinder(FromLoc, FromLoc, NavSearchRadius, 32,255,0,0,TRUE);
		DrawDebugCone(FromLoc, Direction, MaxDistance, Angle2, 0.f, 16, FColor(0,255,0), TRUE);
	}
#endif

	// clear visited flag on all NavigationPoints
	for (ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
	{
		Nav->bAlreadyVisited = FALSE;
	}

	// query the nav octree for nearby objects
	TArray<FNavigationOctreeObject*> NavObjects;
	GWorld->NavigationOctree->RadiusCheck(FromLoc, NavSearchRadius, NavObjects);
	for(INT Idx = 0; Idx < NavObjects.Num(); Idx++)
	{
		// look for valid slot markers
		FNavigationOctreeObject* NavObj = NavObjects(Idx);
		if( NavObj == NULL )
			continue;

		ACoverSlotMarker *Marker = NavObj->GetOwner<ACoverSlotMarker>();
		if( !Marker )
		{
			// if it isn't a marker directly, see if it's a spec that ends in a marker
			UReachSpec *Spec = NavObj->GetOwner<UReachSpec>();
			if( Spec )
			{
				Marker = Cast<ACoverSlotMarker>(Spec->End.Nav());
			}
		}
		// if it's a valid marker that hasn't been visited yet
		if( Marker && !Marker->bAlreadyVisited )
		{
			// mark it as visited so we don't check more than once
			Marker->bAlreadyVisited = TRUE;

			// grab info about the owning link
			ACoverLink *Link = Marker->OwningSlot.Link;
			
			INT SlotIdx = Marker->OwningSlot.SlotIdx;
			
			// validate the owning link information
			if( !Link || SlotIdx < 0 || SlotIdx > Link->Slots.Num() - 1 )
			{
				continue;
			}

			// early check to see if the link/slot is disabled
			if( Link->bDisabled || !Link->Slots(SlotIdx).bEnabled || !Link->IsValidClaim(this,SlotIdx,TRUE,TRUE) )
			{
#if !FINAL_RELEASE	// COVER_DEBUG
				if (bCoverDebug_FoundCover)
				{
					debugf(TEXT("- Failed enabled/claim check: %s %d, %d %d %d"),*Link->GetName(),SlotIdx,Link->bDisabled,Link->Slots(SlotIdx).bEnabled,Link->IsValidClaim(this,SlotIdx,TRUE,TRUE));
				}
#endif
				continue;
			}

			INT	  RightIdx		= Link->GetSlotIdxToRight(SlotIdx);
			UBOOL bRightEnabled = RightIdx >= 0 ? Link->Slots(RightIdx).bEnabled : FALSE;
			INT	  LeftIdx		= Link->GetSlotIdxToLeft(SlotIdx);
			UBOOL bLeftEnabled	= LeftIdx  >= 0 ? Link->Slots(LeftIdx).bEnabled : FALSE;

			// if this is a single slot cover link
			if( Link->Slots.Num() == 1 || (!bRightEnabled && !bLeftEnabled ) )
			{
				FillCoverPosInfo( Link, 0, 0, FromLoc, Direction, MaxDistance, PotentialCover );

				// See if potential cover is acceptable
				FLOAT TestDotFOV = MinDotFOV;
				FLOAT TestDistanceSquared = MaxDistanceSquared;
				if( ValidatePotentialCover(FromLoc, Direction, TestDotFOV, TestDistanceSquared, PotentialCover) )
				{
					OutCovPosInfo	= PotentialCover;
					bFoundCover		= TRUE;
					// and refine the search values
					MinDotFOV = TestDotFOV;
					MaxDistanceSquared = TestDistanceSquared;
				}
			}
			// multi-slot cover link
			else
			{
				// check the segment to the right if valid
				if( RightIdx < Link->Slots.Num() && bRightEnabled )
				{
					INT LtSlotIdx = SlotIdx;
					INT RtSlotIdx = RightIdx;
					FillCoverPosInfo(Link, LtSlotIdx, RtSlotIdx, FromLoc, Direction, MaxDistance, PotentialCover);

					// If adjacent slot is disabled
					if( !Link->Slots(PotentialCover.RtSlotIdx).bEnabled )
					{
						// Set cover pct to left side
						PotentialCover.LtToRtPct = 0.f;
					}
#if !FINAL_RELEASE	// COVER_DEBUG
					if (bCoverDebug_ConsideredLinks)
					{
						debugf(TEXT("-> Checking left to right: %s %d (%2.3f)"),*Link->GetName(),SlotIdx,PotentialCover.LtToRtPct);
					}
#endif
					// See if potential cover is acceptable
					FLOAT TestDotFOV = MinDotFOV;
					FLOAT TestDistanceSquared = MaxDistanceSquared;
					if( ValidatePotentialCover(FromLoc, Direction, TestDotFOV, TestDistanceSquared, PotentialCover) )
					{
						OutCovPosInfo	= PotentialCover;
						bFoundCover		= TRUE;
						// and refine the search values
						MinDotFOV = TestDotFOV;
						MaxDistanceSquared = TestDistanceSquared;
					}
				}
				// check the segment to the left if valid
				if( LeftIdx >= 0 && bLeftEnabled )
				{
					INT LtSlotIdx = LeftIdx;
					INT RtSlotIdx = SlotIdx;
					FillCoverPosInfo( Link, LtSlotIdx, RtSlotIdx, FromLoc, Direction, MaxDistance, PotentialCover );

					// If adjacent slot is disabled
					if( !Link->Slots(PotentialCover.LtSlotIdx).bEnabled )
					{
						// Set cover pct to right side
						PotentialCover.LtToRtPct = 1.f;
					}
#if !FINAL_RELEASE	// COVER_DEBUG
					if (bCoverDebug_ConsideredLinks)
					{
						debugf(TEXT("-> Checking right to left: %s %d (%2.3f)"),*Link->GetName(),SlotIdx,PotentialCover.LtToRtPct);
					}
#endif
					// See if potential cover is acceptable
					FLOAT TestDotFOV = MinDotFOV;
					FLOAT TestDistanceSquared = MaxDistanceSquared;
					if( ValidatePotentialCover(FromLoc, Direction, TestDotFOV, TestDistanceSquared, PotentialCover) )
					{
						OutCovPosInfo	= PotentialCover;
						bFoundCover		= TRUE;
						// and refine the search values
						MinDotFOV = TestDotFOV;
						MaxDistanceSquared = TestDistanceSquared;
					}
				}
			}
		}
	}

#if !FINAL_RELEASE	// COVER_DEBUG
	// debug cover found
	if( bFoundCover && bCoverDebug_FoundCover )
	{
		DrawDebugCoordinateSystem(OutCovPosInfo.Location, OutCovPosInfo.Normal.Rotation(), 50, TRUE);
	}
#endif

	return bFoundCover;
}

void AGearPawn::FillCoverPosInfo( ACoverLink* Link, INT LtSlotIdx, INT RtSlotIdx, FVector FromLoc, FVector Direction, FLOAT MaxDistance, FCovPosInfo& out_CovPosInfo )
{
	if( Link->Slots.Num() > 1 )
	{	
		// First find Left and Right slot indices
		out_CovPosInfo.LtSlotIdx = LtSlotIdx;
		out_CovPosInfo.RtSlotIdx = RtSlotIdx;

		// Find slots locations. Some checks are done in 2D space to fix issues with non flat grounds (ie stairs).
		FVector	LtSlotNormal	= Link->GetSlotRotation(out_CovPosInfo.LtSlotIdx).Vector().SafeNormal2D() * -1.f;
		FVector LtSlotLoc		= Link->GetSlotLocation(out_CovPosInfo.LtSlotIdx) - LtSlotNormal * Link->AlignDist;

		FVector	RtSlotNormal	= Link->GetSlotRotation(out_CovPosInfo.RtSlotIdx).Vector().SafeNormal2D() * -1.f;
		FVector RtSlotLoc		= Link->GetSlotLocation(out_CovPosInfo.RtSlotIdx) - RtSlotNormal * Link->AlignDist;
		
		FVector	RtSlotLoc2D		= FVector(RtSlotLoc.X, RtSlotLoc.Y, LtSlotLoc.Z);
		FVector LtToRtVec		= RtSlotLoc - LtSlotLoc;
		FVector LtToRtVec2D		= RtSlotLoc2D - LtSlotLoc;
		FVector LtToRtDir2D		= LtToRtVec2D.SafeNormal2D();

		// Find closest points between cover bounds and direction segments
		FVector P1, P2;
		SegmentDistToSegment(LtSlotLoc, RtSlotLoc2D, FromLoc, FromLoc + MaxDistance * Direction, P1, P2);

#if !FINAL_RELEASE && bCoverDebug_CoverFOV && 0	// COVER_DEBUG - disabled by default, because it's really slow
		DrawDebugLine(LtSlotLoc, RtSlotLoc2D, 0, 0, 255, TRUE);
		DrawDebugSphere(P1, 4.f, 8, 0, 255, 255, TRUE);
#endif

		// Compute potential cover
		out_CovPosInfo.LtToRtPct	= Clamp<FLOAT>( ((P1 - LtSlotLoc)  | LtToRtDir2D) / LtToRtVec2D.Size(), 0.f, 1.f);
		
		// Find Cover Normal by interpolating between both slots...
		out_CovPosInfo.Normal		= Lerp(LtSlotNormal, RtSlotNormal, out_CovPosInfo.LtToRtPct).SafeNormal2D();
		out_CovPosInfo.Location		= LtSlotLoc + out_CovPosInfo.LtToRtPct * LtToRtVec + out_CovPosInfo.Normal * Link->AlignDist;	// Since it was pushed back into wall, put it away again.
		out_CovPosInfo.Tangent		= out_CovPosInfo.Normal ^ FVector(0,0,1.f);
		out_CovPosInfo.Link			= Link;
	}
	else
	{
		const FVector DirToWall		= Link->GetSlotRotation(LtSlotIdx).Vector();

		// Return ideal cover location.
		out_CovPosInfo.Location		= Link->GetSlotLocation(LtSlotIdx);
		out_CovPosInfo.Normal		= DirToWall.SafeNormal2D() * -1.f;
		out_CovPosInfo.Tangent		= out_CovPosInfo.Normal ^ FVector(0,0,1.f);
		out_CovPosInfo.Link			= Link;
		out_CovPosInfo.LtSlotIdx	= LtSlotIdx;
		out_CovPosInfo.RtSlotIdx	= RtSlotIdx;
		out_CovPosInfo.LtToRtPct	= 0.f;
	}
}


/**
 * If we've found potential cover, make sure it's usable.
 * @param	FromLoc					Location of player willing to enter cover
 * @param	Direction				Direction player is looking for cover
 * @param	FOV						Cover has to be within this FOV from Direction. (Dot, 0.f == 0d, 1.f == 180d)
 * @param	OutMaxDistanceSquared	Max Distance squared cover has to be from player. If less, then it is updated with new value.
 * @param	OutCovPosInfo			Cover information to validate.
 * @param	PushedBackCovLoc		CoverLocation, pushed back against wall.
 */
UBOOL AGearPawn::ValidatePotentialCover(FVector FromLoc, FVector Direction, FLOAT& OutMinDotFOV, FLOAT& OutMaxDistanceSquared, FCovPosInfo& CovPosInfo)
{
	// Backup values, they can be changed when a slot is selected to narrow down results.
	FLOAT PendingMaxDistanceSquared = OutMaxDistanceSquared;
	FLOAT PendingMinDotFOV = OutMinDotFOV;

	const FLOAT CoverAcquireDot = 1.f - Clamp<FLOAT>(CoverAcquireFOV, 0.f, 1.f);

#if !FINAL_RELEASE	// COVER_DEBUG
	// Draw CoverAcquireFOV for this cover location
	if( bCoverDebug_CoverFOV )
	{
		const FLOAT	Angle2 = appAcos(CoverAcquireDot);
		DrawDebugCone(CovPosInfo.Location, CovPosInfo.Normal, 2.f * CovPosInfo.Link->AlignDist, Angle2, 0.f, 16, FColor(0,0,255), TRUE);
	}
#endif

	// CoverAcquisitionCheck: CoverNormal dot SearchDir must be within CoverAcquireFOV
	if( (CovPosInfo.Normal | Direction) >  -(CoverAcquireDot - KINDA_SMALL_NUMBER) )		
	{
		return FALSE;
	}

	// Adjust CoverLocation for FOV and distance tests.
	FVector AdjustedCovLoc = CovPosInfo.Location;

	// Push back cover location against the wall -- don't do that for circular cover
	if( !CovPosInfo.Link->bCircular )
	{
		const FVector OldLoc = AdjustedCovLoc;

		AdjustedCovLoc = AdjustedCovLoc - CovPosInfo.Normal * CovPosInfo.Link->AlignDist;

#if !FINAL_RELEASE	// COVER_DEBUG
		if( bCoverDebug_CoverVolume )
		{
			// Draw adjusted cover location (pushed back against geometry).
			DrawDebugLine(OldLoc, AdjustedCovLoc, 128, 0, 255, TRUE);
			// Draw closest point on extent to direction line
			DrawDebugSphere(AdjustedCovLoc, 2.f, 4, 128, 0, 255, TRUE);
		}
#endif
	}

	// In the case of edge slots, adjust cover location, taking into account cover extents. 
	if( !CovPosInfo.Link->bCircular && (CovPosInfo.LtToRtPct <= KINDA_SMALL_NUMBER || CovPosInfo.LtToRtPct >= (1.f - KINDA_SMALL_NUMBER)) )
	{
		// Cover has a slight extent, so let's check for the closest point along the cover bounding segment!
		const FVector	BoundExtent	= CovPosInfo.Tangent * CovPosInfo.Link->AlignDist;
		const FVector	RtExtent	= AdjustedCovLoc + BoundExtent;
		const FVector	LtExtent	= AdjustedCovLoc - BoundExtent;

		// Closest points between cover bounds and direction segments
		FVector P1, P2;
		SegmentDistToSegment(LtExtent, RtExtent, FromLoc, FromLoc + appSqrt(OutMaxDistanceSquared) * Direction, P1, P2);

		// This is the closest point along the cover bound segment to the direction search segment.
		AdjustedCovLoc = P1;

#if !FINAL_RELEASE	// COVER_DEBUG
		if( bCoverDebug_CoverVolume )
		{
			// Draw cover position extents
			const FVector HalfPoint = (LtExtent + RtExtent) * 0.5f;
			DrawDebugLine(LtExtent, HalfPoint, 255, 0, 128, TRUE);
			DrawDebugLine(HalfPoint, RtExtent, 128, 0, 255, TRUE);
			DrawDebugLine(FromLoc, FromLoc + appSqrt(OutMaxDistanceSquared) * Direction, 128, 0, 255, TRUE);
			
			// Draw closest point on extent to direction line
			DrawDebugSphere(P1, 4.f, 4, 255, 0, 128, TRUE);
			DrawDebugSphere(P2, 4.f, 4, 128, 0, 255, TRUE);
		}
#endif
	}

	const FVector	PawnCylExtent		= GetCylinderExtent();
	const FVector	VectToCover			= AdjustedCovLoc - FromLoc;
	const FLOAT		DistToCoverSq2D		= VectToCover.SizeSquared2D();

	if( DistToCoverSq2D > OutMaxDistanceSquared )		// check that this spot is the closest we can find
	{
		return FALSE;
	}

	// make sure it's within acceptable Z
	if( Abs(AdjustedCovLoc.Z - FromLoc.Z) > 2.5f * PawnCylExtent.Z )
	{
#if !FINAL_RELEASE	// COVER_DEBUG
		if( bCoverDebug_ValidatedCover )
		{
			debugf(TEXT("- Failed Z Check. Link: %s, Index: %d"), *CovPosInfo.Link->GetName(), CovPosInfo.LtSlotIdx);
		}
#endif
		return FALSE;
	}
	// adjust the z values so that we're only checking x/y
	AdjustedCovLoc.Z = FromLoc.Z;

	// Vector from ChkLoc to Adjusted cover position
	FVector			VectToAdjCover		= AdjustedCovLoc - FromLoc;
	const UBOOL		bIsTouchingCover	= (DistToCoverSq2D <= Square(PawnCylExtent.X));
	const FLOAT		BehindCovDot		= (VectToAdjCover.SafeNormal2D() | CovPosInfo.Normal);

	// If we've run by cover location, then we cannot perform the player acquisition FOV check
	if(	bIsTouchingCover && (BehindCovDot >= -KINDA_SMALL_NUMBER) )
	{
		// Set parameters to refine search with further candidates.
		// Since FOV is not relevant, distance is considered here
		PendingMaxDistanceSquared = DistToCoverSq2D;
	}
	else
	{
		// PlayerAcquisitionFOV Check: PawnToCoverDir dot SearchDir must be within MinDotFOV
		const FLOAT DotFOV = (VectToAdjCover.SafeNormal2D() | Direction);
		if( DotFOV < OutMinDotFOV - KINDA_SMALL_NUMBER )	
		{
#if !FINAL_RELEASE	// COVER_DEBUG
			if( bCoverDebug_ValidatedCover )
			{
				debugf(TEXT("- Failed PlayerAcquisitionFOV Check! DotFOV: %f, Link: %s, Index: %d"), DotFOV, *CovPosInfo.Link->GetName(), CovPosInfo.LtSlotIdx);
			}
#endif
			return FALSE;
		}

		// Set parameters to refine search with further candidates.
		// Distance is fighting with FOV... with a too wide FOV, close things may not be what we want..
		// for now, consider only cover the closest to direction line...
		PendingMinDotFOV = DotFOV;
	}

	// Cover slot snapping
	// Snap to Slot location if within Pawn collision cylinder distance
	if( !CovPosInfo.Link->bCircular && CovPosInfo.LtToRtPct != 0.f && CovPosInfo.LtToRtPct != 1.f )
	{
		// Left slot
		if( CovPosInfo.LtToRtPct < 0.5f )
		{
			const FVector SlotLocation = CovPosInfo.Link->GetSlotLocation(CovPosInfo.LtSlotIdx);
			if( (SlotLocation - CovPosInfo.Location).Size2D() < PawnCylExtent.X * CoverSnapScale )
			{
				// Update cover information
				CovPosInfo.Location		= SlotLocation;
				CovPosInfo.LtToRtPct	= 0.f;
			}
		}
		// Right Slot
		else
		{
			const FVector SlotLocation = CovPosInfo.Link->GetSlotLocation(CovPosInfo.RtSlotIdx);
			if( (SlotLocation - CovPosInfo.Location).Size2D() < PawnCylExtent.X * CoverSnapScale )
			{
				// Update cover information
				CovPosInfo.Location		= SlotLocation;
				CovPosInfo.LtToRtPct	= 1.f;
			}
		}
	}

	ACoverLink *Link = CovPosInfo.Link;

	// Account for nudges when entering the edges of cover
	if( !Link->bCircular && Link->Slots.Num() > 1 )
	{
		FLOAT	NudgeDistance	= PawnCylExtent.X * 0.5f;
		FVector LtSlotLoc		= Link->GetSlotLocation(CovPosInfo.LtSlotIdx);
		FVector RtSlotLoc		= Link->GetSlotLocation(CovPosInfo.RtSlotIdx);
		FVector LtToRtVec		= RtSlotLoc - LtSlotLoc;
		FLOAT	LtToRtSize2D	= LtToRtVec.Size2D();

		// Left Slot
		if( CovPosInfo.LtToRtPct < 0.5f )
		{
			if( Link->IsLeftEdgeSlot(CovPosInfo.LtSlotIdx, TRUE) )
			{
				// Update cover information based on nudge difference
				if( ShouldApplyNudge(Link, CovPosInfo.LtSlotIdx) )
				{
					CovPosInfo.LtToRtPct	= Max<FLOAT>(CovPosInfo.LtToRtPct, Clamp<FLOAT>(NudgeDistance/LtToRtSize2D, 0.f, 1.f));
					CovPosInfo.Location		= LtSlotLoc + CovPosInfo.LtToRtPct * LtToRtVec;
				}
			}
		}
		// Right Slot
		else
		{
			if( Link->IsRightEdgeSlot(CovPosInfo.RtSlotIdx, TRUE) )
			{
				// Update cover information based on nudge difference
				if( ShouldApplyNudge(Link,CovPosInfo.RtSlotIdx) )
				{
					CovPosInfo.LtToRtPct	= Min<FLOAT>(CovPosInfo.LtToRtPct, 1.f - Clamp<FLOAT>(NudgeDistance/LtToRtSize2D, 0.f, 1.f));
					CovPosInfo.Location		= LtSlotLoc + CovPosInfo.LtToRtPct * LtToRtVec;
				}
			}
		}
	}

	// Take into account PushOutOffset
	CovPosInfo.Location += (CovPosInfo.Tangent ^ FVector(0,0,-1)).SafeNormal2D() * GetCoverPushOutOffset(CovPosInfo.Link, this);

	// make sure no one is between us and the cover
	FCheckResult Hit;
	// If FromLoc is within Pawn collision cylinder, then use PointReachable.
	// we have to check for this because FromLoc can be nudged to be away from walls.
	if( (FromLoc-Location).SizeSquared2D() <= Square(PawnCylExtent.X) && !pointReachable(CovPosInfo.Location, FALSE) )
	{
#if !FINAL_RELEASE	// COVER_DEBUG
		if( bCoverDebug_ValidatedCover )
		{
			debugf(TEXT("- Failed point reachable! Link: %s, Index: %d"), *CovPosInfo.Link->GetName(), CovPosInfo.LtSlotIdx);
		}
#endif 
		return FALSE;
	}
	if (!GWorld->SinglePointCheck(Hit,CovPosInfo.Location,PawnCylExtent,TRACE_Pawns|TRACE_StopAtAnyHit) && Hit.Actor != this)
	{
#if !FINAL_RELEASE	// COVER_DEBUG
		if( bCoverDebug_ValidatedCover )
		{
			debugf(TEXT("- Failed point check! Link: %s, Index: %d (%s)"), *CovPosInfo.Link->GetName(), CovPosInfo.LtSlotIdx, *Hit.Actor->GetName());
		}
#endif 
		return FALSE;
	}
	/*
	else
	{
		// Make sure there is no geometry obstruction to found location
		FCheckResult Hit(1.f);
		FVector CheckExtent	= PawnCylExtent * 0.9f;
		CheckExtent.Z = 0.f;	// reduce Z, because ground is unlikely flat

		const FVector DirToCover = VectToCover.SafeNormal();

		if( !GWorld->SingleLineCheck(Hit, this, CovPosInfo.Location - DirToCover * PawnCylExtent.X, FromLoc, TRACE_World|TRACE_StopAtAnyHit, CheckExtent) )
		{
#if !FINAL_RELEASE	// COVER_DEBUG
			if( bCoverDebug_ValidatedCover )
			{
				debugf(TEXT("- Failed LineCheck to cover #1. HitActor: %s"), *Hit.Actor->GetName());
			}
#endif
			return FALSE;
		}
	}
	*/


#if !FINAL_RELEASE	// COVER_DEBUG
	// Draw validated/approved cover
	if( bCoverDebug_ValidatedCover )
	{
		DrawDebugCoordinateSystem(CovPosInfo.Location, CovPosInfo.Normal.Rotation(), 50, TRUE);
		debugf(TEXT("%3.2f Cover validated! Link: %s, Index: %d. Updating MaxDistanceSquared to: %f and MinDotFOV to: %f"), GWorld->GetTimeSeconds(), *CovPosInfo.Link->GetName(), CovPosInfo.LtSlotIdx, PendingMaxDistanceSquared, PendingMinDotFOV);
	}
#endif

	// Update values with any changes made
	OutMaxDistanceSquared = PendingMaxDistanceSquared;
	OutMinDotFOV = PendingMinDotFOV;

	return TRUE;
}

static void DrawLineArrow(FPrimitiveDrawInterface* PDI,const FVector &start,const FVector &end,const FColor &color,FLOAT mag)
{
	// draw a pretty arrow
	FVector dir = end - start;
	FLOAT dirMag = dir.Size();
	dir /= dirMag;
	FVector YAxis, ZAxis;
	dir.FindBestAxisVectors(YAxis,ZAxis);
	FMatrix arrowTM(dir,YAxis,ZAxis,start);
	DrawDirectionalArrow(PDI,arrowTM,color,dirMag,mag,SDPG_World);
}

UBOOL AGearPawn::IsInCover()
{
	return (CoverType != CT_None &&
			CurrentLink != NULL &&
			CurrentSlotIdx >= 0);
}

UBOOL AGearPawn::GuessAtCover( FCoverInfo& out_Cover )
{
	if (WorldInfo->TimeSeconds - LastCoverGuessTime < 0.3f)
	{
		out_Cover = CachedCoverGuess;
	}
	else
	{
		out_Cover.Link = NULL;
		out_Cover.SlotIdx = -1;

		// otherwise guess at the best cover
		FLOAT BestRating = -1.f, 
			  Rating = 0.f;

		// look at all slot markers around the pawn
		TArray<FNavigationOctreeObject*> NavObjects;
		GWorld->NavigationOctree->RadiusCheck(Location,UCONST_MaxCoverGuessDist,NavObjects);
		for( INT Idx = 0; Idx < NavObjects.Num(); Idx++ )
		{
			ACoverSlotMarker *Marker = NavObjects(Idx)->GetOwner<ACoverSlotMarker>();
			if (Marker != NULL &&
				Marker->OwningSlot.Link != NULL &&
				Marker->OwningSlot.SlotIdx >= 0 && Marker->OwningSlot.SlotIdx < Marker->OwningSlot.Link->Slots.Num())
			{
				FCoverSlot &Slot = Marker->OwningSlot.Link->Slots(Marker->OwningSlot.SlotIdx);
				// if the slot is enabled
				if (Slot.bEnabled)
				{
					Rating  = 1.f - ((Marker->Location - Location).Size() / UCONST_MaxCoverGuessDist);
					Rating += Rotation.Vector() | (Marker->Location - Location).SafeNormal();
					// if better than our current rating
					if( Rating > BestRating )
					{
						BestRating = Rating;
						out_Cover.Link = Marker->OwningSlot.Link;
						out_Cover.SlotIdx = Marker->OwningSlot.SlotIdx;
					}
				}
			}
		}

		// cache the results
		CachedCoverGuess = out_Cover;
		LastCoverGuessTime = WorldInfo->TimeSeconds;
	}
	return (out_Cover.Link != NULL);
}

/** 
 * Returns true if given test pawn is eligible for inclusion in the enemy list of the given list owner.
 */
static UBOOL PawnIsValidForEnemyList(APawn* ListOwnerPawn, AGearPawn* TestWP)
{
	return ( TestWP && ListOwnerPawn && (TestWP != ListOwnerPawn) && TestWP->IsAliveAndWell() && (ListOwnerPawn->GetTeamNum() != TestWP->GetTeamNum()) );
}

void AGearPC::MaintainEnemyList()
{
	// prune dead cache entries and score good ones
	// do this along with the LOS checks, avoid traversing list per frame?
	for (INT EnemyIdx=0; EnemyIdx<LocalEnemies.Num(); ++EnemyIdx)
	{
		AGearPawn* const WP = LocalEnemies(EnemyIdx).Enemy;
		if ( !PawnIsValidForEnemyList(Pawn, WP) )
		{
			//log("expired from cache:"@EnemyIdx@FrictionTargetLOSCache[EnemyIdx].Enemy);
			LocalEnemies.Remove(EnemyIdx, 1);

			if (NextLocalEnemyToCheckLOS > EnemyIdx)
			{
				--NextLocalEnemyToCheckLOS;
			}

			--EnemyIdx;
		}
	}

	// look for new potential friction targets and score them while we're there
	for (APawn *P = GWorld->GetWorldInfo()->PawnList; P != NULL; P = P->NextPawn)
	{
		AGearPawn* const WP = Cast<AGearPawn>(P);

		if (WP)
		{
			// ignore self, dead guys
			if (PawnIsValidForEnemyList(Pawn, WP))
			{
				// search to ensure uniqueness
				UBOOL bFound = FALSE;
				for (INT Idx=0; Idx<LocalEnemies.Num(); ++Idx)
				{
					if (LocalEnemies(Idx).Enemy == WP)
					{
						bFound = TRUE;
						break;
					}
				}

				if (!bFound)
				{
					// not in the cache, but it should be!  insert such that 
					// LOS check will happen at earliest opportunity

					// validate NextLocalEnemyToCheckLOS just to be safe
					if ( (NextLocalEnemyToCheckLOS < 0) || (NextLocalEnemyToCheckLOS >= LocalEnemies.Num()) )
					{
						NextLocalEnemyToCheckLOS = 0;
					}

					//log("added to cache:"@NextLocalEnemyToCheckLOS@P);
					LocalEnemies.Insert(NextLocalEnemyToCheckLOS, 1);
					LocalEnemies(NextLocalEnemyToCheckLOS).Enemy = WP;
					LocalEnemies(NextLocalEnemyToCheckLOS).bVisible = FALSE;
					LocalEnemies(NextLocalEnemyToCheckLOS).bSeen = FALSE;
				}
			}
		}
	}

	//
	// note, we're going through some apparent gymnastics to minimize
	// LineOfSight() calls.  We're calling that max once per frame here,
	// and caching the result until next time said pawn is checked
	//

	// do our one and only LOS call
	if (LocalEnemies.Num() > 0)
	{
		// do this first, since expiring the last element in the array can leave it hanging
		// off the end
		if (NextLocalEnemyToCheckLOS >= LocalEnemies.Num())
		{
			NextLocalEnemyToCheckLOS = 0;
		}

		// get camera pos/rot
		FVector CamLoc;
		FRotator CamRot;
		eventGetPlayerViewPoint( CamLoc, CamRot );

		FLocalEnemy& LE = LocalEnemies(NextLocalEnemyToCheckLOS);

		// Set whether the enemy is in LOS or not. (early out if it hasn't been rendered recently)
		LE.bHasLOS = (GWorld->GetTimeSeconds() - LE.Enemy->LastRenderTime) < 0.2f && (UBOOL) LineOfSightTo( LE.Enemy, 0, &CamLoc );

		// don't set anyone behind us to bVisible.  assumes a  180 deg "awareness".
		if ( LE.bHasLOS )
		{
			//log("checked"@NextLocalEnemyToCheckLOS@FrictionTargetLOSCache[NextLocalEnemyToCheckLOS].Enemy);
			// Note: only kicks out "noticed" messages for enemies in a 180 cone, but everyone with LOS 
			// is considered a "seen" enemy
			if ( !LE.bSeen && ((CamRot.Vector() | (LE.Enemy->Location - CamLoc)) > 0.f) )
			{
				// first time this guy has been seen, "notice" him
				if (Role == ROLE_Authority)
				{
					eventNoticeEnemy(Pawn, LE.Enemy);
				}
			}
			LE.bVisible = TRUE;
			LE.bSeen = TRUE;
		}
		else
		{
			LE.bVisible = FALSE;
		}

		++NextLocalEnemyToCheckLOS;
		if (NextLocalEnemyToCheckLOS >= LocalEnemies.Num())
		{
			NextLocalEnemyToCheckLOS = 0;
		}
	}
}

/** helper for GetFrictionTarget() */
static inline UBOOL IsPawnAValidFrictionAdhesionTarget(AGearPawn* WP, UBOOL bAdhesion)
{
	return ( WP && 
			( (!bAdhesion && WP->bCanBeFrictionedTo) || (bAdhesion && WP->bCanBeAdheredTo) ) &&
			( (WP->Health > 0) || WP->IsDBNO() ) && 
			!WP->bHidden && 
			!WP->bDeleteMe );
}

AActor* AGearPC::GetFrictionAdhesionTarget(FLOAT MaxDistance, UBOOL bAdhesion)
{
	AActor* BestFrictionTarget = NULL;
	FLOAT BestFrictionTargetScore = 0.f;

	// get camera pos/rot
	FVector CamLoc(Location);
	FRotator CamRot(Rotation);
	eventGetPlayerViewPoint( CamLoc, CamRot );

	INT const NumEnemies = LocalEnemies.Num();

	// Note
	// even though all actors have a bCanBeFrictionedTo flag
	// we don't want to traverse all actors every frame, so we'll pick and choose
	// here for efficiency.
	for (INT EnemyIdx=0; EnemyIdx<NumEnemies; ++EnemyIdx)
	{
		FLocalEnemy& LE = LocalEnemies(EnemyIdx);

		if ( (LE.Enemy != NULL) && LE.bVisible && IsPawnAValidFrictionAdhesionTarget(LE.Enemy, bAdhesion) )
		{
			// score all potential targets and find best one.
			FLOAT TmpScore = ScoreFrictionTarget(LE.Enemy, MaxDistance, CamRot);

			// track best visible target
			if ( TmpScore > BestFrictionTargetScore )
			{
				BestFrictionTargetScore = TmpScore;
				BestFrictionTarget = LE.Enemy;
			}
		}
	}

	// run through AimAssistActors as well, since that's what they are for
	AGearGRI* const GRI = Cast<AGearGRI>(WorldInfo->GRI);
	if (GRI)
	{
		for (INT AAAIdx=0; AAAIdx<GRI->AimAssistActorCache.Num(); ++AAAIdx)
		{
			AGearAimAssistActor* const GAAA = GRI->AimAssistActorCache(AAAIdx);
			if ( GAAA && 
				( (!bAdhesion && GAAA->bCanBeFrictionedTo) || (bAdhesion && GAAA->bCanBeAdheredTo) ) )
			{
				FLOAT Score = ScoreFrictionTarget(GAAA, MaxDistance, CamRot);
				if ( Score > BestFrictionTargetScore )
				{
					BestFrictionTargetScore = Score;
					BestFrictionTarget = GAAA;
				}
			}
		}
	}

	return BestFrictionTarget;
}

FLOAT AGearPC::ScoreFrictionTarget(AActor const* Actor, FLOAT MaxDistance, FRotator const& CamRot)
{
	FLOAT Score = 0.f;

	if (Actor && Pawn)
	{
		// Initial Score based on how much we're aiming at them
		// should probably use same location determination as friction code instead of actor->location
		// but differences should be subtle and minor
		FVector ToTargetNorm = Actor->Location - Pawn->Location;
		FLOAT DistToTarget = ToTargetNorm.Size();
		ToTargetNorm /= DistToTarget;

		Score = ToTargetNorm | CamRot.Vector();

		// If they're in front and within friction range
		if( (Score > 0.f) && (DistToTarget < MaxDistance) )
		{
			// Adjust Score based on distance, so that closer targets are slightly favored
			Score += (1.f - (DistToTarget/MaxDistance)) * Score * 0.65f;
		}
	}

	return Score;
}

/** @return The index of the PC in the game player's array. */
INT AGearPC::GetUIPlayerIndex()
{
	ULocalPlayer* LP = NULL;
	INT Result = INDEX_NONE;

	LP = Cast<ULocalPlayer>(Player);

	if(LP)
	{	
		Result = UUIInteraction::GetPlayerIndex(LP);
	}

	return Result;
}

void AGearPC::SetGamma(FLOAT GammaValue)
{
#if !PS3
	extern void SetDisplayGamma(FLOAT Gamma);
	SetDisplayGamma(GammaValue);
#endif
}


/**
 * Sets the post process settings for this player.
 * 
 * @param Preset	Preset to use to set the 4 PP values.
 */
void AGearPC::SetPostProcessValues(BYTE Preset)
{
	for(INT PresetIdx=0; PresetIdx<PostProcessPresets.Num(); PresetIdx++)
	{
		FPostProcessInfo &PostProcessPreset = PostProcessPresets(PresetIdx);

		if(PostProcessPreset.Preset == Preset)
		{
			ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

			if(LocalPlayer)
			{
				LocalPlayer->PP_MidTonesMultiplier = PostProcessPreset.MidTones;
				LocalPlayer->PP_ShadowsMultiplier = PostProcessPreset.Shadows;
				LocalPlayer->PP_HighlightsMultiplier = PostProcessPreset.HighLights;
				LocalPlayer->PP_DesaturationMultiplier = PostProcessPreset.Desaturation;
			}

			break;
		}
	}
}

#if 0 
/** 
* Native login changed handler.  Usually returns the player to the title screen.
*
* @param NewStatus		The new login state of the player.
*/
void AGearPC::OnLoginChanged(BYTE NewStatus)
{
	ELoginStatus LoginStatus = (ELoginStatus)NewStatus;
	FString CurrentMap = GetCurrMapName();

	// If we are on the main menu, we kick people back to title screen depending on what the new login status is.
	if(appStricmp(*CurrentMap,TEXT("gearstart"))==0)
	{
		UGameUISceneClient* SceneClient = UUIInteraction::GetSceneClient();

		// Notify the topmost menu that we've switched login status.
		ULocalPlayer* LP = Cast<ULocalPlayer>(Player);
		if(LP && SceneClient->ActiveScenes.Num())
		{
			INT PlayerIndex = UUIInteraction::GetPlayerIndex(LP);
			if ( PlayerIndex != INDEX_NONE )
			{
				// Loop through all scenes and activate the login changed event if a scene is on top or always visible.
				for(INT SceneIdx=0; SceneIdx<SceneClient->ActiveScenes.Num();SceneIdx++)
				{
					UUIScene* SceneIt = SceneClient->ActiveScenes(SceneIdx);

					if(SceneIdx==SceneClient->ActiveScenes.Num()-1 || SceneIt->bAlwaysRenderScene)
					{
//TODO:Hook to screens so they can handle login changing
//						SceneIt->ActivateEventByClass(PlayerIndex, UUIEvent_LoginChanged::StaticClass());
					}
				}

				// Check current scene tag and make sure login changes are valid on the scene.
				UUIScene* Scene = SceneClient->ActiveScenes.Last();
				if( Scene )
				{
					FString ErrorTitle;
					FString ErrorMessage;
					FString SceneToClose;

					// Since we are on the main menu, if they logged out at all, kick them back to the title screen.
					if(LoginStatus == LS_NotLoggedIn)
					{
// 						// If they are in the versus path or any of the online coop paths then we need to kick them back to the title screen.
// 						if(SceneClient->FindSceneByTag(TEXT("HostCoop")) || SceneClient->FindSceneByTag(TEXT("JoinCoop")))
// 						{
// 							SceneToClose=TEXT("CampaignType");
// 							ErrorMessage=TEXT("<Strings:WarfareGame.ErrorMessages.NeedProfileForMultiplayer_Message>");
// 							ErrorTitle=TEXT("<Strings:WarfareGame.ErrorMessages.NeedProfileForMultiplayer_Title>");
// 						}
// 						else if(SceneClient->FindSceneByTag(TEXT("VersusMenu")))
// 						{
// 							SceneToClose=TEXT("VersusMenu");
// 							ErrorMessage=TEXT("<Strings:WarfareGame.ErrorMessages.NeedProfileForMultiplayer_Message>");
// 							ErrorTitle=TEXT("<Strings:WarfareGame.ErrorMessages.NeedProfileForMultiplayer_Title>");
// 						}
					}	
					else if(LoginStatus == LS_UsingLocalProfile)
					{
// 						// See if the player is in the live path.
// 						if(SceneClient->FindSceneByTag(TEXT("VersusXboxLivePlayerMatch"), LP))
// 						{
// 							SceneToClose = TEXT("VersusMenu");
// 							ErrorMessage=TEXT("<Strings:WarfareGame.ErrorMessages.NeedGoldTier_Message>");
// 							ErrorTitle=TEXT("<Strings:WarfareGame.ErrorMessages.NeedGoldTier_Title>");
// 						}
// 						else if(SceneClient->FindSceneByTag(TEXT("VersusXboxLiveRankedMatch"), LP))
// 						{
// 							SceneToClose = TEXT("VersusMenu");
// 							ErrorMessage=TEXT("<Strings:WarfareGame.ErrorMessages.NeedGoldTier_Message>");
// 							ErrorTitle=TEXT("<Strings:WarfareGame.ErrorMessages.NeedGoldTier_Title>");
// 						}


						// Check connection type for coop since it goes through 1 path.
						UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();

						if(DataStoreClient)
						{
//							UUIDataStore_WarfareGameResource* ResourceDatastore = Cast<UUIDataStore_WarfareGameResource>(DataStoreClient->FindDataStore(TEXT("WarfareGameResources")));
// 
// 							INT ConnectionType = appAtoi(*ResourceDatastore->ConnectionType);
// 							if(ConnectionType==2)
// 							{
// 								if(SceneClient->FindSceneByTag(TEXT("CampaignMenu"), LP))
// 								{
// 									SceneToClose=TEXT("CampaignType");
// 									ErrorMessage=TEXT("<Strings:WarfareGame.ErrorMessages.NeedGoldTier_Message>");
// 									ErrorTitle=TEXT("<Strings:WarfareGame.ErrorMessages.NeedGoldTier_Title>");
// 								}
// 							}
						}
					}

					if(PlayerIndex==0 && SceneToClose.Len()==0)
					{
						//Always close leaderboard screen if the user changed profiles.
// 						if(SceneClient->FindSceneByTag(TEXT("Leaderboards"), LP))
// 						{
// 							SceneToClose = TEXT("Leaderboards");
// 						}
// 
// 						//Always close campaign menu screen if the user changed profiles.
// 						if(SceneClient->FindSceneByTag(TEXT("CampaignMenu"), LP))
// 						{
// 							SceneToClose = TEXT("CampaignMenu");
// 						}
					}

// 					if(SceneToClose.Len())
// 					{
// 						MenuCloseSceneAndDisplayError(SceneToClose, ErrorTitle, ErrorMessage);
// 					}
				}
			}
		}
	}
	else
	{
		// If we are in a single player game, then its alright to change your profile.
		ENetMode NetMode = (ENetMode)WorldInfo->NetMode;

		debugf(TEXT("Login Changed: Net Mode: %i"), (INT)NetMode);
		if(NetMode != NM_Standalone || NewStatus == LS_NotLoggedIn)
		{
			// Set the error if we are in a versus match.
			AGearGRI* GRI = Cast<AGearGRI>(GWorld->GetWorldInfo()->GRI);

			if(GRI)
			{
				// Always send the user back if they are in versus mode or logged out.
				if(GRI->bIsCoop == FALSE || NewStatus == LS_NotLoggedIn)
				{
					UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();

					if(DataStoreClient && !GRI->bIsCoop)
					{
//						UUIDataStore_WarfareGameResource* ResourceDatastore = Cast<UUIDataStore_WarfareGameResource>(DataStoreClient->FindDataStore(TEXT("WarfareGameResources")));

// 						if(ResourceDatastore)
// 						{
// 							// #57932 fix
// 							// DESCRIPTION:		DELTA: COOP: VERSUS: Incorrect message displayed when signing 
// 							//                  into LIVE from LIVE-enabled profile during System Link. The game 
// 							//                  disconnects you with an incorrect message if you enter Coop/Versus 
// 							//                  gameplay in System Link using LIVE-enabled player profiles that are 
// 							//                  not signed into LIVE that are subsequently signed into LIVE once 
// 							//                  the profile has entered the gameplay.  The message says "A signed-in 
// 							//                  gamer profile is required to play multiplayer games".  It should: 
// 							// SOLUTION:		If we gain LoggedIn status (and were in the game) it means we signed 
// 							//                  in to live from an already selected profile.
// 							if (NewStatus == LS_LoggedIn)
// 							{
// 								ResourceDatastore->GlobalMessage_MessageMarkup=TEXT("<Strings:WarfareGame.ErrorMessages.DisconnectedBecauseSignedIn_Message>");
// 								ResourceDatastore->GlobalMessage_TitleMarkup=TEXT("<Strings:WarfareGame.ErrorMessages.DisconnectedBecauseSignedIn_Title>");
// 							}
// 							else
// 							{
// 								ResourceDatastore->GlobalMessage_MessageMarkup=TEXT("<Strings:WarfareGame.ErrorMessages.NeedProfileForMultiplayer_Message>");
// 								ResourceDatastore->GlobalMessage_TitleMarkup=TEXT("<Strings:WarfareGame.ErrorMessages.NeedProfileForMultiplayer_Title>");
// 							}
// 							ResourceDatastore->DisplayGlobalMessage=TRUE;
//						}
					}

					// Send the user back to the start screen.
//					eventOnQuitToMainMenu(NULL);
				}
			}
		}

	}
}

/** 
* Native connection status changed handler.  Called when the user is in the main menu.
*
* @param ConnectionStatus		The current connection status of the online system.
*/
void AGearPC::OnMenuConnectionStatusChanged(BYTE ConnectionStatus)
{
	EOnlineServerConnectionStatus ConnectionStatusValue = (EOnlineServerConnectionStatus)(ConnectionStatus);
	UGameUISceneClient* SceneClient = UUIInteraction::GetSceneClient();

	// Check to see if we are in any scenes that require a xbox live connection and then kick the user back notifying why they lost their connection status.
	if(SceneClient->ActiveScenes.Num())
	{
		FString ErrorTitle;
		FString ErrorMessage;
		FString SceneToClose;
		UBOOL bKickUserBack = FALSE;

		switch(ConnectionStatusValue)
		{
		case OSCS_DuplicateLoginDetected:
			ErrorTitle=TEXT("<Strings:WarfareGame.ErrorMessages.DuplicateSignin_Title>");
			ErrorMessage=TEXT("<Strings:WarfareGame.ErrorMessages.DuplicateSignin_Message>");
			bKickUserBack = TRUE;
			break;

		case OSCS_ConnectionDropped:
		case OSCS_NoNetworkConnection:
		case OSCS_ServiceUnavailable:
		case OSCS_UpdateRequired:
		case OSCS_ServersTooBusy:
			ErrorTitle=TEXT("<Strings:WarfareGame.ErrorMessages.LostLiveConnection_Title>");
			ErrorMessage=TEXT("<Strings:WarfareGame.ErrorMessages.LostLiveConnection_Message>");
			bKickUserBack = TRUE;
			break;
		}

		// See if we have any scenes that allow us to kick the user back.
		if(bKickUserBack)
		{
// 			if(SceneClient->FindSceneByTag(TEXT("HostCoop")) || SceneClient->FindSceneByTag(TEXT("JoinCoop")))
// 			{
// 				SceneToClose = TEXT("CampaignType");
// 			}
// 			else if(SceneClient->FindSceneByTag(TEXT("VersusXboxLiveRankedMatch")) || SceneClient->FindSceneByTag(TEXT("VersusXboxLivePlayerMatch")) || 
// 				SceneClient->FindSceneByTag(TEXT("Leaderboards")))
// 			{
// 				SceneToClose = TEXT("VersusMenu");
// 			}
		}

		// See if we need to kick the user back to the main menu.
		if(SceneToClose.Len())
		{
//			MenuCloseSceneAndDisplayError(SceneToClose, ErrorTitle, ErrorMessage);
		}
	}
}

/** 
* Native link status changed handler.  Called when the user is in the main menu.
* 
* @param bHasLinkConnection		Whether or not the online system has a link connection active.
*/
void AGearPC::OnMenuLinkStatusChanged(UBOOL bHasLinkConnection)
{
	UGameUISceneClient* SceneClient = UUIInteraction::GetSceneClient();

	if(bHasLinkConnection==FALSE)
	{
		FString SceneToClose;

// 		if(SceneClient->FindSceneByTag(TEXT("HostCoop")) || SceneClient->FindSceneByTag(TEXT("JoinCoop")))
// 		{
// 			SceneToClose = TEXT("CampaignType");
// 		}
// 		else if(SceneClient->FindSceneByTag(TEXT("VersusXboxLiveRankedMatch")) || SceneClient->FindSceneByTag(TEXT("VersusXboxLivePlayerMatch")) || 
// 			SceneClient->FindSceneByTag(TEXT("Leaderboards")) || SceneClient->FindSceneByTag(TEXT("VersusSystemLink")))
// 		{
// 			SceneToClose = TEXT("VersusMenu");
// 		}

		// See if we need to kick the user back to the main menu.
		if(SceneToClose.Len())
		{
// 			const FString ErrorTitle = TEXT("<Strings:WarfareGame.ErrorMessages.LostLink_Title>");
// 			const FString ErrorMessage=TEXT("<Strings:WarfareGame.ErrorMessages.LostLink_Message>");
// 			MenuCloseSceneAndDisplayError(SceneToClose, ErrorTitle, ErrorMessage);
		}
	}
}
#endif

UBOOL AGearPC::CanDoSpecialMove(BYTE AMove, UBOOL bForceCheck)
{
	return (AMove == SM_None || (MyGearPawn && MyGearPawn->eventCanDoSpecialMove(AMove, bForceCheck)));
}

void AGearPC::UpdateCanDoSpecialMoveCacheList(TArrayNoInit<UBOOL>& CacheList, TArrayNoInit<BYTE>& PawnIndexList, FLOAT& LastUpdateTime, INT& LastUpdatedIndex)
{

	if(CacheList.Num() != PawnIndexList.Num())
	{
		check(CacheList.Num() == 0);
		CacheList.AddZeroed(PawnIndexList.Num());
	}

	// if it's been too long since we updated one of these, set them all to false
	FLOAT TimeSeconds = GWorld->GetTimeSeconds();
	if((TimeSeconds - LastUpdateTime) > 0.25f)
	{
		for(INT idx=0;idx<CacheList.Num();idx++)
		{
			CacheList(idx)=FALSE;
		}
	}

	LastUpdatedIndex++;
	// wrap
	if(LastUpdatedIndex >= CacheList.Num())
	{
		LastUpdatedIndex=0;
	}

	// check the next one in line
	CacheList(LastUpdatedIndex) = CanDoSpecialMove(PawnIndexList(LastUpdatedIndex),FALSE);
	LastUpdateTime = TimeSeconds;
}

void AGearPC::UpdateCanDoSpecialMoveCache(UBOOL bInCover)
{
	if(MyGearPawn == NULL)
	{
		return;
	}

	if(bInCover)
	{
		UpdateCanDoSpecialMoveCacheList(CachedCanDoCoverActionSpecialMoves,MyGearPawn->CoverActionSpecialMoves,LastCoverActionSpecialMoveUpdateTime,LastCoverActionSpecialMoveUpdateIdx);
	}
	else
	{
		UpdateCanDoSpecialMoveCacheList(CachedCanDoGlobalActionSpecialMoves,MyGearPawn->ActionSpecialMoves,LastGlobalActionSpecialMoveUpdateTime,LastGlobalActionSpecialMoveUpdateIdx);
	}
	
}

UBOOL AGearPC::CheckForSpecialMove()
{
	if(MyGearPawn == NULL)
	{
		return FALSE;
	}

	UClass*	MoveClass = NULL;
	// only check if normal cover
	if ( MyGearPawn->IsInCover() )
	{
		// update the cover cando specialmove cache
		UpdateCanDoSpecialMoveCache(TRUE);
		// iterate through each cover special move
		for (INT Idx = 0; Idx < MyGearPawn->CoverActionSpecialMoves.Num(); Idx++)
		{
			
			MoveClass = MyGearPawn->SpecialMoveClasses(MyGearPawn->CoverActionSpecialMoves(Idx));
			if (MoveClass)
			{
				// if we can do this special move
				if (CachedCanDoCoverActionSpecialMoves(Idx) ||
					((MyGearPawn->CoverActionSpecialMoves(Idx) == SM_PushObject) && MyGearPawn->IsDoingSpecialMove(SM_PushObject)) )
				{
					// then notify the hud and stop looking
					MyGearHud->eventSetActionInfo(AT_SpecialMove, MoveClass->GetDefaultObject<UGearSpecialMove>()->Action, MyGearPawn->bIsMirrored);
					return TRUE;
				}
			}
		}
	}

	MoveClass = NULL;
	// iterate through each global special move
	//update global cando cache
	UpdateCanDoSpecialMoveCache(FALSE);
	for (INT Idx = 0; Idx < MyGearPawn->ActionSpecialMoves.Num(); Idx++)
	{
		MoveClass = MyGearPawn->SpecialMoveClasses(MyGearPawn->ActionSpecialMoves(Idx));
		if (MoveClass)
		{
			// if we can do this special move
			if (CachedCanDoGlobalActionSpecialMoves(Idx) ||
				((MyGearPawn->ActionSpecialMoves(Idx) == SM_PushObject) && MyGearPawn->IsDoingSpecialMove(SM_PushObject)) ||
				((MyGearPawn->ActionSpecialMoves(Idx) == SM_ChainsawDuel_Leader) && MyGearPawn->IsDoingSpecialMove(SM_ChainsawDuel_Leader)) ||
				((MyGearPawn->ActionSpecialMoves(Idx) == SM_ChainsawDuel_Follower) && MyGearPawn->IsDoingSpecialMove(SM_ChainsawDuel_Follower)) )
			{
				// then notify the hud and stop looking
				MyGearHud->eventSetActionInfo(AT_SpecialMove, MoveClass->GetDefaultObject<UGearSpecialMove>()->Action, MyGearPawn->bIsMirrored);
				return TRUE;
			}
		}
	}

	return FALSE;
}

//void AGearDestructibleObject::ActivateRemoteEvent(FName EventName)
//{
	// @writeme

	//// look for all remote events in the entire sequence tree
	//USequence *RootSeq = GWorld->GetRootSequence();
	//GWorld->Ki
	//TArray<USequenceObject*> RemoteEvents;
	//RootSeq->FindSeqObjectsByClass(USeqEvent_RemoteEvent::StaticClass(),RemoteEvents);
	//for (INT Idx = 0; Idx < RemoteEvents.Num(); Idx++)
	//{
	//	USeqEvent_RemoteEvent *RemoteEvt = Cast<USeqEvent_RemoteEvent>(RemoteEvents(Idx));
	//	if (RemoteEvt != NULL && RemoteEvt->EventName == EventName && RemoteEvt->bEnabled)
	//	{
	//		// check activation for the event
	//		RemoteEvt->CheckActivate(this,this,FALSE,NULL);
	//	}
	//}
//}


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
void AGearDestructibleObject::BuildPhysStaticMeshCache(ULevel* Level,
													   INT& TriByteCount, INT& TriMeshCount, INT& HullByteCount, INT& HullCount)
{

	Super::BuildPhysStaticMeshCache(Level, TriByteCount, TriMeshCount, HullByteCount, HullCount);

	//In addition, do it for each sub-object
	for (INT i=0; i<SubObjects.Num(); i++)
	{
		const FDestructibleSubobject& SubObject = SubObjects(i);
		if (SubObject.Mesh != NULL)
		{
			for (INT j=0; j<SubObject.DamageMods.Num(); j++)
			{
				UStaticMesh* StaticMesh = SubObject.DamageMods(j).NewMesh;
				if (StaticMesh != NULL)
				{
					// Overall scale factor for this mesh.
					FVector TotalScale3D = SubObject.Mesh->Scale * DrawScale * SubObject.Mesh->Scale3D * DrawScale3D;
					StaticMesh->CookPhysConvexDataForScale(Level, TotalScale3D, NULL, TriByteCount, TriMeshCount, HullByteCount, HullCount);
				}
			}
		}
	}
}

void AGearDestructibleObject::CheckForErrors()
{
	Super::CheckForErrors();

	TArray<UStaticMesh*> CheckedMeshes;

	//In addition, do it for each sub-object
	for (INT i=0; i<SubObjects.Num(); i++)
	{
		const FDestructibleSubobject& SubObject = SubObjects(i);
		if (SubObject.Mesh != NULL)
		{
			if (!SubObject.Mesh->bUsePrecomputedShadows)
			{
				GWarn->MapCheck_Add(
					MCTYPE_WARNING, this,
					*FString::Printf( TEXT("GearDestructibleObject MeshComponent %i does not have bUsePrecomputedShadows=true, performance will suffer."), i), MCACTION_NONE );
			}

			for (INT j=0; j<SubObject.DamageMods.Num(); j++)
			{
				UStaticMesh* StaticMesh = SubObject.DamageMods(j).NewMesh;
				if (StaticMesh != NULL && CheckedMeshes.FindItemIndex(StaticMesh) == INDEX_NONE)
				{
					// Overall scale factor for this mesh.
					FVector TotalScale3D = SubObject.Mesh->Scale * DrawScale * SubObject.Mesh->Scale3D * DrawScale3D;
					if (!TotalScale3D.IsUniform() && 
						StaticMesh->BodySetup     &&
						(StaticMesh->BodySetup->AggGeom.BoxElems.Num() > 0 || StaticMesh->BodySetup->AggGeom.SphylElems.Num() > 0 || StaticMesh->BodySetup->AggGeom.SphereElems.Num() > 0 ))
					{
						GWarn->MapCheck_Add(
							MCTYPE_WARNING, this,
							*FString::Printf( TEXT( "Mesh '%s' has simple collision, but is being scaled non-uniformly, collision creation will fail." ),
							*StaticMesh->GetName() ), MCACTION_NONE );
					}

					//Prevents printing same mesh warning multiple times per actor
					CheckedMeshes.AddUniqueItem(StaticMesh);
				}
			}
		}
	}



	const FString& TheClassName = GetClass()->GetName() ;

	if( TheClassName != TEXT( "GearDestructibleObject")
		&& TheClassName != TEXT( "GDO_PropaneTank")
		)
	{
		GWarn->MapCheck_Add(
			MCTYPE_ERROR, this,
			*FString::Printf( TEXT( "GDO '%s' from Gears1 is being used.  This is incorrect.  This should be a prefab.  Find AlanW :-)" ),
			*GetName() ), MCACTION_NONE );
	}
}

void AGearDestructibleObject::SetupComponents()
{
	// attach all of the subobjects
	for (INT Idx=0; Idx<SubObjects.Num(); ++Idx)
	{
		if( SubObjects(Idx).Mesh && !SubObjects(Idx).Mesh->IsAttached() && !SubObjects(Idx).Mesh->IsTemplate() )
		{
			AttachComponent(SubObjects(Idx).Mesh);
		}
	}

	//Setup the light environment
	AttachComponent(LightEnvironment);

	//Initialize any physics related to the attaching of components above
	InitRBPhys();

	bComponentsSetUp = TRUE;
}

void AGearDestructibleObject::SetupHealthVars()
{
	SubObjectHealths.Empty();
	for (INT Idx=0; Idx<SubObjects.Num(); ++Idx)
	{
		SubObjectHealths.AddItem(SubObjects(Idx).DefaultHealth);
	}
}

void AGearDestructibleObject::PostLoad()
{
	// try: add to components array, editor will auto attach
	if (GIsEditor)
	{
		for (INT Idx=0; Idx<SubObjects.Num(); ++Idx)
		{
			Components.AddItem(SubObjects(Idx).Mesh);
		}
	}

	Super::PostLoad();
}

void AGearDestructibleObject::OneTimeInit()
{
	SetupComponents();

	GenerateUndo();

	// convert names to indices.  this looks crazy but each of the lists should be pretty short
	INT const NumSubObjs = SubObjects.Num();

	for (INT SubObjIdx=0; SubObjIdx<NumSubObjs; ++SubObjIdx)
	{
		FDestructibleSubobject* const SubObj = &SubObjects(SubObjIdx);
		INT const NumDamageMods = SubObj->DamageMods.Num();

		for (INT DamageModIdx=0; DamageModIdx<NumDamageMods; ++DamageModIdx)
		{
			FObjectDamageModifier* const DamageMod = &SubObj->DamageMods(DamageModIdx);
			INT const NumDependentSubObjs = DamageMod->DependentSubObjs.Num();

			for (INT DepSubObjIdx=0; DepSubObjIdx<NumDependentSubObjs; ++DepSubObjIdx)
			{
				FObjDamageModifierDependency* const Dep = &DamageMod->DependentSubObjs(DepSubObjIdx);

				// find subobject by name
				for (INT Idx=0; Idx<SubObjects.Num(); ++Idx)
				{
					if (Dep->DependentSubObjName == SubObjects(Idx).SubObjName)
					{
						Dep->DependentSubObjIdx = Idx;
						break;
					}
				}
			}
		}

		// tell primitivecomponents which LE to use
		SubObj->Mesh->SetLightEnvironment(LightEnvironment);
	}
}

void AGearDestructibleObject::InitRBPhys()
{
	// attach subobjects _before_ calling InitRBPhys
	for (INT Idx=0; Idx<SubObjects.Num(); ++Idx)
	{
		if( SubObjects(Idx).Mesh && !SubObjects(Idx).Mesh->IsAttached() && !SubObjects(Idx).Mesh->IsTemplate() )
		{
			AttachComponent(SubObjects(Idx).Mesh);
		}
	}

	Super::InitRBPhys();

	// Grab any cached rb phys data out of the level before it is cleared
	// so we have it when we swamp static meshes.
	// Saves creation of convex collision at runtime which is slow

	ULevel* Level = GetLevel();
	check(Level);

	INT const NumSubObjs = SubObjects.Num();
	for (INT SubObjIdx=0; SubObjIdx<NumSubObjs; ++SubObjIdx)
	{
		FDestructibleSubobject* const SubObj = &SubObjects(SubObjIdx);
		INT const NumDamageMods = SubObj->DamageMods.Num();

		for (INT DamageModIdx=0; DamageModIdx<NumDamageMods; ++DamageModIdx)
		{
			FObjectDamageModifier* const DamageMod = &SubObj->DamageMods(DamageModIdx);
			if (DamageMod->NewMesh != NULL)
			{
				UStaticMesh* StaticMesh = DamageMod->NewMesh;
				URB_BodySetup* BodySetup = StaticMesh->BodySetup;
				if(BodySetup)
				{
					// Overall scale factor for this mesh (same assumed calculation as in BuildPhysStaticMeshCache).
					FVector Scale3D = SubObj->Mesh->Scale * DrawScale * SubObj->Mesh->Scale3D * DrawScale3D;

					// Check the cache in the ULevel to see if we have cooked data for it already
					FKCachedConvexData* CachedData = Level->FindPhysStaticMeshCachedData(StaticMesh, Scale3D);

					// Make the debug name for this geometry...
					FString DebugName(TEXT(""));

#if (!CONSOLE || _DEBUG || LOOKING_FOR_PERF_ISSUES) && !FINAL_RELEASE && !NO_LOGGING
					DebugName += FString::Printf( TEXT("StaticMesh: %s"), *StaticMesh->GetPathName() );
#endif

					BodySetup->AddCollisionFromCachedData(Scale3D, CachedData, DebugName);
				}
			}
		}
	}
}

void AGearDestructibleObject::PostEditChange(UProperty* PropertyThatChanged)
{
	SetupComponents();
	SetupHealthVars();
	Super::PostEditChange(PropertyThatChanged);
}


void AGearDestructibleObject::SetSubObjectStaticMesh(FDestructibleSubobject const& SubObj, class UStaticMesh* SM)
{
	if ( (SM != NULL) && (SM != SubObj.Mesh->StaticMesh) )
	{
		// clear lighting data and force shadow casting off when swapping the component's static mesh
		// since the cached lighting will not be valid for the new static mesh
		if( SubObj.Mesh->HasStaticShadowing() )
		{
			SubObj.Mesh->CastShadow = FALSE;
		}

		// set the mesh and reattach it
		RemoveDecals(); // since we are changing the mesh we want to Remove All Decals;
		SubObj.Mesh->SetStaticMesh(SM);

		UDynamicLightEnvironmentComponent* DynLight = Cast<UDynamicLightEnvironmentComponent>(LightEnvironment);
		if (DynLight != NULL)
		{
			DynLight->SetEnabled(TRUE);
		}
	}
}


/** 
 * This function will remove all of the currently attached decals from the object.  
 * Basically, we want to have decals attach to these objects and then on state change (due to damage usually), we will
 * just detach them all with the big particle effect that occurs it should not be noticeable.
 **/
void AGearDestructibleObject::RemoveDecals()
{
	for( INT ComponentIndex = 0; ComponentIndex < Components.Num(); ++ComponentIndex )
	{
		UDecalComponent* DecalComp = Cast<UDecalComponent>(Components(ComponentIndex));
		if( DecalComp != NULL )
		{
			//warnf( TEXT( "DETACHING DECAL!!!!!! %s"), *DecalComp->GetFullName() );
			DecalComp->ResetToDefaults();
		}
	}
}


static FVector RotateVector(FVector const& LocalVect, FRotator const& SystemRot)
{
	return FRotationMatrix(SystemRot).TransformNormal( LocalVect );
}


void AGearDestructibleObject::GenerateUndo()
{
	for (INT SubObjIdx=0; SubObjIdx<SubObjects.Num(); ++SubObjIdx)
	{
		GenerateSubObjUndo(&SubObjects(SubObjIdx));
	}
}

void AGearDestructibleObject::GenerateSubObjUndo(FDestructibleSubobject* SubObj)
{
	FObjectDamageModifier* const UndoMod = &SubObj->UndoMod;

	// original static mesh
	UndoMod->NewMesh = SubObj->Mesh->StaticMesh;

	for (INT ModIdx=0; ModIdx<SubObj->DamageMods.Num(); ++ModIdx)
	{
		FObjectDamageModifier* const Mod = &SubObj->DamageMods(ModIdx);

		//
		// material replacements
		//
		for (INT Idx=0; Idx<Mod->MaterialReplacements.Num(); ++Idx)
		{
			// see if we've already recorded the original state for this index
			UBOOL bFound = FALSE;
			for (INT Jdx=0; Jdx<UndoMod->MaterialReplacements.Num(); ++Jdx)
			{
				if (UndoMod->MaterialReplacements(Jdx).MaterialIndex == Mod->MaterialReplacements(Idx).MaterialIndex)
				{
					bFound = TRUE;
				}
			}

			if (!bFound)
			{
				// add a modification back to the original
				FMaterialReplaceMod NewMaterialReplacement;
				NewMaterialReplacement.MaterialIndex = Mod->MaterialReplacements(Idx).MaterialIndex;
				NewMaterialReplacement.NewMaterial = SubObj->Mesh->GetMaterial( NewMaterialReplacement.MaterialIndex );
				UndoMod->MaterialReplacements.AddItem(NewMaterialReplacement);
			}
		}

		//
		// scalar params
		//
		for (INT Idx=0; Idx<Mod->MaterialScalarParams.Num(); ++Idx)
		{
			// see if we've already recorded the original state for this matinst/param pair
			UBOOL bFound = FALSE;
			for (INT Jdx=0; Jdx<UndoMod->MaterialScalarParams.Num(); ++Jdx)
			{
				if ( (UndoMod->MaterialScalarParams(Jdx).MatInst == Mod->MaterialScalarParams(Idx).MatInst) && 
					 (UndoMod->MaterialScalarParams(Jdx).ParamName == Mod->MaterialScalarParams(Idx).ParamName) )
				{
					bFound = TRUE;
				}
			}

			if (!bFound)
			{
				// add a modification back to the original
				FMaterialScalarParamMod NewMaterialScalarParamMod;
				NewMaterialScalarParamMod.MatInst = Mod->MaterialScalarParams(Idx).MatInst;
				NewMaterialScalarParamMod.ParamName = Mod->MaterialScalarParams(Idx).ParamName;
				NewMaterialScalarParamMod.MatInst->GetScalarParameterValue(NewMaterialScalarParamMod.ParamName, NewMaterialScalarParamMod.ScalarVal);
				UndoMod->MaterialScalarParams.AddItem(NewMaterialScalarParamMod);
			}
		}

		//
		// tex params
		//
		for (INT Idx=0; Idx<Mod->MaterialTexParams.Num(); ++Idx)
		{
			// see if we've already recorded the original state for this matinst/param pair
			UBOOL bFound = FALSE;
			for (INT Jdx=0; Jdx<UndoMod->MaterialTexParams.Num(); ++Jdx)
			{
				if ( (UndoMod->MaterialTexParams(Jdx).MatInst == Mod->MaterialTexParams(Idx).MatInst) && 
					(UndoMod->MaterialTexParams(Jdx).ParamName == Mod->MaterialTexParams(Idx).ParamName) )
				{
					bFound = TRUE;
				}
			}

			if (!bFound)
			{
				// add a modification back to the original
				FMaterialTexParamMod NewMaterialTexParamMod;
				NewMaterialTexParamMod.MatInst = Mod->MaterialTexParams(Idx).MatInst;
				NewMaterialTexParamMod.ParamName = Mod->MaterialTexParams(Idx).ParamName;
				NewMaterialTexParamMod.MatInst->GetTextureParameterValue(NewMaterialTexParamMod.ParamName, NewMaterialTexParamMod.NewTexture);
				UndoMod->MaterialTexParams.AddItem(NewMaterialTexParamMod);
			}
		}

		//
		// vector params
		//
		for (INT Idx=0; Idx<Mod->MaterialVectorParams.Num(); ++Idx)
		{
			// see if we've already recorded the original state for this matinst/param pair
			UBOOL bFound = FALSE;
			for (INT Jdx=0; Jdx<UndoMod->MaterialVectorParams.Num(); ++Jdx)
			{
				if ( (UndoMod->MaterialVectorParams(Jdx).MatInst == Mod->MaterialVectorParams(Idx).MatInst) && 
					(UndoMod->MaterialVectorParams(Jdx).ParamName == Mod->MaterialVectorParams(Idx).ParamName) )
				{
					bFound = TRUE;
				}
			}

			if (!bFound)
			{
				// add a modification back to the original
				FMaterialVectorParamMod NewMaterialVecParamMod;
				NewMaterialVecParamMod.MatInst = Mod->MaterialVectorParams(Idx).MatInst;
				NewMaterialVecParamMod.ParamName = Mod->MaterialVectorParams(Idx).ParamName;
				NewMaterialVecParamMod.MatInst->GetVectorParameterValue(NewMaterialVecParamMod.ParamName, NewMaterialVecParamMod.VectorVal);
				UndoMod->MaterialVectorParams.AddItem(NewMaterialVecParamMod);
			}
		}
	}

	// save original cover params to undo mod
	for (INT Idx=0; Idx<AttachedCover.Num(); ++Idx)
	{
		ACoverLink* const Cov = AttachedCover(Idx);
		if (Cov)
		{
			for (INT SlotIdx=0; SlotIdx<Cov->Slots.Num(); ++SlotIdx)
			{
				FCoverSlot* const Slot = &Cov->Slots(SlotIdx);

				FCoverModParams NewCovMod;

				NewCovMod.AttachedCoverIndex = Idx;
				NewCovMod.SlotIndex = SlotIdx;

				NewCovMod.NewCoverType = Slot->CoverType;
				NewCovMod.bUpdateCanMantle = TRUE;
				NewCovMod.bUpdateCanCoverSlip_Left = TRUE;
				NewCovMod.bUpdateCanCoverSlip_Right = TRUE;
				NewCovMod.bUpdateCanSwatTurn_Left = TRUE;
				NewCovMod.bUpdateCanSwatTurn_Right = TRUE;
				NewCovMod.bUpdateAllowPopup = TRUE;
				NewCovMod.bUpdateLeanLeft = TRUE;
				NewCovMod.bUpdateLeanRight = TRUE;

				NewCovMod.bNewCanMantle = Slot->bCanMantle;
				NewCovMod.bNewCanCoverSlip_Left = Slot->bCanCoverSlip_Left;
				NewCovMod.bNewCanCoverSlip_Right = Slot->bCanCoverSlip_Right;
				NewCovMod.bNewCanSwatTurn_Left = Slot->bCanSwatTurn_Left;
				NewCovMod.bNewCanSwatTurn_Right = Slot->bCanSwatTurn_Right;
				NewCovMod.bNewAllowPopup = Slot->bCanPopUp;
				NewCovMod.bNewLeanLeft = Slot->bLeanLeft;
				NewCovMod.bNewLeanRight = Slot->bLeanRight;

				UndoMod->CoverMods.AddItem(NewCovMod);
			}
		}
	}
}


void AGearDestructibleObject::UnDestroy()
{
	for (INT SubObjIdx=0; SubObjIdx<SubObjects.Num(); ++SubObjIdx)
	{
		FDestructibleSubobject* const SubObj = &SubObjects(SubObjIdx);
	
		// apply undo mod
		ApplyDamageModInternal(SubObj, &SubObj->UndoMod, FALSE, NULL);
		SubObj->UndoMod.bApplied = FALSE;

		// mark all mods as bApplied=FALSE
		for (INT ModIdx=0; ModIdx<SubObj->DamageMods.Num(); ++ModIdx)
		{
			SubObj->DamageMods(ModIdx).bApplied = FALSE;
		}

		// reset health
		SubObjectHealths(SubObjIdx) = SubObj->DefaultHealth;
	}

	// reset attached events' trigger counts
	for (INT Idx = 0; Idx < GeneratedEvents.Num(); Idx++)
	{
		if (GeneratedEvents(Idx) != NULL)
		{
			GeneratedEvents(Idx)->TriggerCount = 0;
		}
	}

	// undo the shutdown stuff
	eventUnShutDownObject();

	// restart ambientsoundcomponent
	if (AmbientSoundComponent)
	{
		AmbientSoundComponent->Play();
	}

	for (INT Idx=0; Idx<AttachedCover.Num(); ++Idx)
	{
		ACoverLink* const Cov = AttachedCover(Idx);
		if (Cov)
		{
			Cov->eventSetDisabled(FALSE);

			for (INT Jdx=0; Jdx<Cov->Slots.Num(); ++Jdx)
			{
				Cov->eventSetSlotEnabled(Jdx, TRUE);
			}
		}
	}

	// reset replication data
	appMemzero(ProcessedMods, sizeof(ProcessedMods));
	for (INT ModIdx=0; ModIdx<ARRAY_COUNT(ReplicatedDamageMods); ++ModIdx)
	{
		//debugf(TEXT("reseting replicated damage mods %d"),ModIdx);
		ReplicatedDamageMods[ModIdx].ObjIdx = 255;
		ReplicatedDamageMods[ModIdx].ModIdx = 255;
	}
}


void AGearDestructibleObject::ApplyDamageModInternal(FDestructibleSubobject* SubObj, FObjectDamageModifier* Mod, UBOOL bPartial, class AController* DamageInstigator)
{
	//float Timer[32]={0};
	//CLOCK_CYCLES(Timer[0]);

	INT Idx;

	// switch mesh?
	//CLOCK_CYCLES(Timer[1]);
	if (Mod->NewMesh != NULL)
	{
		SetSubObjectStaticMesh(*SubObj, Mod->NewMesh);
	}
	//UNCLOCK_CYCLES(Timer[1]);

	// material changes
	//CLOCK_CYCLES(Timer[2]);
	for (Idx=0; Idx<Mod->MaterialReplacements.Num(); ++Idx)
	{
		FMaterialReplaceMod const* const MatReplace = &Mod->MaterialReplacements(Idx);
		if (MatReplace->NewMaterial != NULL)
		{
			SubObj->Mesh->SetMaterial( MatReplace->MaterialIndex, MatReplace->NewMaterial );
		}
	}
	for (Idx=0; Idx<Mod->MaterialScalarParams.Num(); ++Idx)
	{
		FMaterialScalarParamMod const* const MatScalar = &Mod->MaterialScalarParams(Idx);
		if (Mod->MaterialScalarParams(Idx).MatInst != NULL)
		{
			MatScalar->MatInst->SetScalarParameterValue(MatScalar->ParamName, MatScalar->ScalarVal);
		}
	}
	for (Idx=0; Idx<Mod->MaterialVectorParams.Num(); ++Idx)
	{
		FMaterialVectorParamMod const* const MatVector = &Mod->MaterialVectorParams(Idx);
		if (MatVector->MatInst != NULL)
		{
			MatVector->MatInst->SetVectorParameterValue(MatVector->ParamName, MatVector->VectorVal);
		}
	}
	for (Idx=0; Idx<Mod->MaterialTexParams.Num(); ++Idx)
	{
		FMaterialTexParamMod const* const MatTex = &Mod->MaterialTexParams(Idx);
		if (MatTex->MatInst != NULL)
		{
			MatTex->MatInst->SetTextureParameterValue(MatTex->ParamName, MatTex->NewTexture);
		}
	}
	//UNCLOCK_CYCLES(Timer[2]);

	//CLOCK_CYCLES(Timer[3]);
	if (Mod->bSelfDestruct)
	{
		// set mesh to none
		eventShutDownSubObject(*SubObj);
	}
	//UNCLOCK_CYCLES(Timer[3]);

	//CLOCK_CYCLES(Timer[4]);
	if ( Mod->bStopAmbientSound && AmbientSoundComponent )
	{
		AmbientSoundComponent->Stop();
	}
	//UNCLOCK_CYCLES(Timer[4]);

	//CLOCK_CYCLES(Timer[5]);
	if (Mod->bForceDisableAttachedCover)
	{
		for (Idx=0; Idx<AttachedCover.Num(); ++Idx)
		{
			ACoverLink* const Cov = AttachedCover(Idx);
			if (Cov != NULL)
			{
				Cov->eventSetDisabled(TRUE);
				// replace mantles with regular specs that can be walked
				for (INT SlotIdx = 0; SlotIdx < Cov->Slots.Num(); SlotIdx++)
				{
					FCoverSlot &Slot = Cov->Slots(SlotIdx);
					if (Slot.bCanMantle && Slot.SlotMarker != NULL)
					{
						for (INT PathIdx = 0; PathIdx < Slot.SlotMarker->PathList.Num(); PathIdx++)
						{
							UMantleReachSpec *MantleSpec = Cast<UMantleReachSpec>(Slot.SlotMarker->PathList(PathIdx));
							if (MantleSpec != NULL)
							{
								UReachSpec *NewSpec = ConstructObject<UReachSpec>(UReachSpec::StaticClass(),MantleSpec->GetOuter(),NAME_None);
								NewSpec->Start = MantleSpec->Start;
								NewSpec->End = MantleSpec->End;
								NewSpec->CollisionHeight = MantleSpec->CollisionHeight;
								NewSpec->CollisionRadius = MantleSpec->CollisionRadius;
								NewSpec->reachFlags = MantleSpec->reachFlags;
								NewSpec->Distance = MantleSpec->Distance;
								// replace the entry in the array
								Slot.SlotMarker->PathList(PathIdx) = NewSpec;
								// leave the other spec in case AI are currently using it - GC will clean it up for us
							}
						}
					}
				}
			}
		}
	}

	if (Mod->SplashDamage.BaseDamage > 0.f)
	{
		// move from obj space to world space
		FVector Loc = Location + ::RotateVector(SubObj->Mesh->Translation, Rotation);
		eventApplySplashDamage(Loc, Mod->SplashDamage);
	}
	//UNCLOCK_CYCLES(Timer[5]);

	if (!bPartial)
	{
		// play sounds
		//CLOCK_CYCLES(Timer[6]);
		for (Idx=0; Idx<Mod->Sounds.Num(); ++Idx)
		{
			PlaySound(Mod->Sounds(Idx));
		}
		//UNCLOCK_CYCLES(Timer[6]);

		// spawn effect
		//CLOCK_CYCLES(Timer[7]);
		for (Idx=0; Idx<Mod->DestroyedEffects.Num(); ++Idx)
		{
			FDestroyedEffectParams const* const P = &Mod->DestroyedEffects(Idx);

			// move out from subobj space to obj space
			FVector ObjSpaceLoc = SubObj->Mesh->Translation + ::RotateVector(P->RelativeOffset, SubObj->Mesh->Rotation);
			FRotationMatrix SubObjRotMat(SubObj->Mesh->Rotation);
			FRotationMatrix EffectRelRotMat(P->RelativeRotation);
			FMatrix ObjSpaceRotMat = SubObjRotMat * EffectRelRotMat;

			// move from obj space to world space
			FVector Loc = Location + ::RotateVector(ObjSpaceLoc, Rotation);
			FRotationMatrix ObjRotMat(Rotation);
			FRotator Rot = (ObjSpaceRotMat * ObjRotMat).Rotator();

			AEmitter* const EmitActor = (AEmitter*)GWorld->SpawnActor(AGearEmitter::StaticClass(), NAME_None, Loc, Rot );
			if (EmitActor)
			{
				//debugf(TEXT("Created emitter %s, template %s"),*EmitActor->GetName(),*P->ParticleEffect->GetPathName());
				EmitActor->bKillDuringLevelTransition = TRUE;
				EmitActor->SetTemplate( P->ParticleEffect, TRUE);
			}
		}
		//UNCLOCK_CYCLES(Timer[7]);

		// reevaluate cover
		//CLOCK_CYCLES(Timer[8]);
		//if (!Mod->bForceDisableAttachedCover)
		//{
		//	for (Idx=0; Idx<AttachedCover.Num(); ++Idx)
		//	{
		//		ACoverLink* const Cov = AttachedCover(Idx);
		//		if (Cov)
		//		{
		//			for (INT Jdx=0; Jdx<Cov->Slots.Num(); ++Jdx)
		//			{
		//				Cov->AutoAdjustSlot(Jdx, FALSE);
		//			}
		//		}
		//	}
		//}
		//UNCLOCK_CYCLES(Timer[8]);
	}

	// cover modifications
	// @fixme
	//		- if disabled slot has a swat turn target, notify the target slot that swat turn to disabled slot is disabled
	//		- if disabled slot has mantle target, notify target slot that mantle to disabled slot is disabled
	//		- make both of those work with the undo stuff (the tricky bit)
	//CLOCK_CYCLES(Timer[8]);
	for (Idx=0; Idx<Mod->CoverMods.Num(); ++Idx)
	{
		FCoverModParams* const CovMod = &Mod->CoverMods(Idx);

		if ( (CovMod->AttachedCoverIndex >= 0) && (CovMod->AttachedCoverIndex < AttachedCover.Num()) )
		{
			ACoverLink* const CovLink = AttachedCover(CovMod->AttachedCoverIndex);

			if (CovLink == NULL)
			{
				continue;
			}

			if (CovMod->bDisableCoverLink)
			{
				// disable the whole link
				CovLink->eventSetDisabled(TRUE);
				// replace mantles with regular specs that can be walked
				for (INT SlotIdx = 0; SlotIdx < CovLink->Slots.Num(); SlotIdx++)
				{
					FCoverSlot &Slot = CovLink->Slots(SlotIdx);
					if (Slot.bCanMantle && Slot.SlotMarker != NULL)
					{
						for (INT PathIdx = 0; PathIdx < Slot.SlotMarker->PathList.Num(); PathIdx++)
						{
							UMantleReachSpec *MantleSpec = Cast<UMantleReachSpec>(Slot.SlotMarker->PathList(PathIdx));
							if (MantleSpec != NULL)
							{
								UReachSpec *NewSpec = ConstructObject<UReachSpec>(UReachSpec::StaticClass(),MantleSpec->GetOuter(),NAME_None);
								NewSpec->Start = MantleSpec->Start;
								NewSpec->End = MantleSpec->End;
								NewSpec->CollisionHeight = MantleSpec->CollisionHeight;
								NewSpec->CollisionRadius = MantleSpec->CollisionRadius;
								NewSpec->reachFlags = MantleSpec->reachFlags;
								NewSpec->Distance = MantleSpec->Distance;
								// replace the entry in the array
								Slot.SlotMarker->PathList(PathIdx) = NewSpec;
								// leave the other spec in case AI are currently using it - GC will clean it up for us
							}
						}
					}
				}
			} 
			else if ( (CovMod->SlotIndex >= 0) && CovLink && (CovMod->SlotIndex < CovLink->Slots.Num()) )
			{
				if (CovMod->bDisableSlot)
				{
					CovLink->eventSetSlotEnabled(CovMod->SlotIndex, FALSE);
					FCoverSlot &Slot = CovLink->Slots(CovMod->SlotIndex);
					if (Slot.SlotMarker != NULL)
					{
						for (INT PathIdx = 0; PathIdx < Slot.SlotMarker->PathList.Num(); PathIdx++)
						{
							UMantleReachSpec *MantleSpec = Cast<UMantleReachSpec>(Slot.SlotMarker->PathList(PathIdx));
							if (MantleSpec != NULL)
							{
								UReachSpec *NewSpec = ConstructObject<UReachSpec>(UReachSpec::StaticClass(),MantleSpec->GetOuter(),NAME_None);
								NewSpec->Start = MantleSpec->Start;
								NewSpec->End = MantleSpec->End;
								NewSpec->CollisionHeight = MantleSpec->CollisionHeight;
								NewSpec->CollisionRadius = MantleSpec->CollisionRadius;
								NewSpec->reachFlags = MantleSpec->reachFlags;
								NewSpec->Distance = MantleSpec->Distance;
								// replace the entry in the array
								Slot.SlotMarker->PathList(PathIdx) = NewSpec;
								// leave the other spec in case AI are currently using it - GC will clean it up for us
							}
						}
					}
				}
				else
				{
					// update necessary flags in the slot
					FCoverSlot* const Slot = &CovLink->Slots(CovMod->SlotIndex);

					if (CovMod->NewCoverType != CT_None)
					{
						Slot->CoverType = CovMod->NewCoverType;
					}

					if (CovMod->bUpdateCanMantle)
					{
						Slot->bCanMantle = CovMod->bNewCanMantle;
					}
					if (CovMod->bUpdateCanCoverSlip_Left)
					{
						Slot->bCanCoverSlip_Left = CovMod->bNewCanCoverSlip_Left;
					}
					if (CovMod->bUpdateCanCoverSlip_Right)
					{
						Slot->bCanCoverSlip_Right = CovMod->bNewCanCoverSlip_Right;
					}
					if (CovMod->bUpdateCanSwatTurn_Left)
					{
						Slot->bCanSwatTurn_Left = CovMod->bNewCanSwatTurn_Left;
					}
					if (CovMod->bUpdateCanSwatTurn_Right)
					{
						Slot->bCanSwatTurn_Right = CovMod->bNewCanSwatTurn_Right;
					}
					if (CovMod->bUpdateAllowPopup)
					{
						Slot->bCanPopUp = CovMod->bNewAllowPopup;
					}
					if (CovMod->bUpdateLeanLeft)
					{
						Slot->bLeanLeft = CovMod->bNewLeanLeft;
					}
					if (CovMod->bUpdateLeanRight)
					{
						Slot->bLeanRight = CovMod->bNewLeanRight;
					}

					if (Slot->SlotOwner != NULL && Slot->SlotOwner->Controller != NULL)
					{
						Slot->SlotOwner->Controller->eventNotifyCoverAdjusted();
					}
				}
			}
		}
	}
	//UNCLOCK_CYCLES(Timer[8]);

	// spawn actors
	//CLOCK_CYCLES(Timer[9]);
	for (Idx=0; Idx<Mod->ActorsToSpawn.Num(); ++Idx)
	{
		FActorSpawnParams const* const P = &Mod->ActorsToSpawn(Idx);

		// move out from subobj space to obj space
		FVector ObjSpaceLoc = SubObj->Mesh->Translation + ::RotateVector(P->RelativeOffset, SubObj->Mesh->Rotation);
		FRotationMatrix SubObjRotMat(SubObj->Mesh->Rotation);
		FRotationMatrix EffectRelRotMat(P->RelativeRotation);
		FMatrix ObjSpaceRotMat = SubObjRotMat * EffectRelRotMat;

		// move from obj space to world space
		FVector Loc = Location + ::RotateVector(ObjSpaceLoc, Rotation);
		FRotationMatrix ObjRotMat(Rotation);
		FRotator Rot = (ObjSpaceRotMat * ObjRotMat).Rotator();

		if (P->Factory)
		{
			FString Error;
			if(P->Factory->CanCreateActor(Error))
			{
				AActor* SpawnedActor = P->Factory->CreateActor(&Loc, &Rot, NULL);
				if (SpawnedActor)
				{
					SpawnedActor->bKillDuringLevelTransition = TRUE;
					SpawnedActor->LifeSpan = Max( 30.0f, P->LifeTimeSeconds );
					SpawnedActor->RemoteRole = ROLE_None;

					if (LightEnvironmentToUseForActorSpawnParams)
					{
						AKActor* TheKActor = Cast<AKActor>(SpawnedActor);
						if( TheKActor != NULL )
						{
							// Light environment setup
							if (!LightEnvironmentToUseForActorSpawnParams->IsEnabled())
							{
								// The GDO is not actually using this light env so we have to provide it with valid bounds
								LightEnvironmentToUseForActorSpawnParams->bOverrideOwnerBounds = TRUE;
								// Martin's fixed bounds
								LightEnvironmentToUseForActorSpawnParams->OverriddenBounds = FBoxSphereBounds(Location, FVector(100.f,100.f,100.f), 173.f);
								// Start visibility checks from the edge of the bounds instead of the center, since it may hit the GDO otherwise
								LightEnvironmentToUseForActorSpawnParams->bTraceFromClosestBoundsPoint = TRUE;
								// Override the light environment's lighting channels with user controllable ones, since the GDO is not actually using the light env
								LightEnvironmentToUseForActorSpawnParams->bOverrideOwnerLightingChannels = TRUE;
								LightEnvironmentToUseForActorSpawnParams->OverriddenLightingChannels = LightingChannelsToUseForActorSpawnParams;
								// Enable the light environment since it defaults to disabled
								LightEnvironmentToUseForActorSpawnParams->SetEnabled(TRUE);
							}
							// Set the KActor to use the GDO's light environment and lighting channels
							TheKActor->StaticMeshComponent->SetLightingChannels( LightingChannelsToUseForActorSpawnParams );
							TheKActor->StaticMeshComponent->SetLightEnvironment( LightEnvironmentToUseForActorSpawnParams );
						}
					}
				}
			}
			else
			{
				debugf(TEXT("GDO (%s) could not create actor: '%s'"), *GetName(), *Error);
			}
		}
	}
	//UNCLOCK_CYCLES(Timer[9]);

	//CLOCK_CYCLES(Timer[10]);
	// skip recursive damage for clients since the server will replicate the results directly
	if (Role == ROLE_Authority)
	{
		for (Idx=0; Idx<Mod->DependentSubObjs.Num(); ++Idx)
		{
			FObjDamageModifierDependency* const Dep = &Mod->DependentSubObjs(Idx);
	
			INT DepSubObjIdx = Dep->DependentSubObjIdx;
			if (DepSubObjIdx >= 0)
			{
				// found!
				INT DmgToDo = appTrunc(SubObjectHealths(DepSubObjIdx) - Dep->MaxHealthToAllow);
				if (DmgToDo > 0)
				{
					DamageSubObject(DepSubObjIdx, DmgToDo, DamageInstigator, NULL);
				}
			}
		}
	}

	// reactivate physics for any dropped pickups that landed on us
	for (INT i = 0; i < Attached.Num(); i++)
	{
		AGearDroppedPickup* DP = Cast<AGearDroppedPickup>(Attached(i));
		if (DP != NULL && DP->Physics == PHYS_RigidBody)
		{
			DP->eventRedrop();
		}
	}

	// notify kismet
	eventTriggerDamageModAppliedEvent(*SubObj, *Mod);

	// mark as applied and we're finished!
	Mod->bApplied = TRUE;

	//UNCLOCK_CYCLES(Timer[10]);

	//UNCLOCK_CYCLES(Timer[0]);

	//Time = Time * GSecondsPerCycle * 1000.f;

	//debugf(TEXT("Total %f"),Timer[0]);
	//debugf(TEXT("-Mesh switch %f"),Timer[1]);
	//debugf(TEXT("-Mat changes %f"),Timer[2]);
	//debugf(TEXT("-shutdownobject %f"),Timer[3]);
	//debugf(TEXT("-stopsound %f"),Timer[4]);
	//debugf(TEXT("-disable cov, splash dmg %f"),Timer[5]);
	//debugf(TEXT("-playsound %f"),Timer[6]);
	//debugf(TEXT("-spawn fx %f"),Timer[7]);
	//debugf(TEXT("-re-eval cover %f"),Timer[8]);
	//debugf(TEXT("-spawn actors %f"),Timer[9]);
	//debugf(TEXT("-The rest %f"),Timer[10]);
}

void AGearDestructibleObject::ApplyDamageMod(INT ObjIdx, INT ModIdx, UBOOL bPartial, class AController* DamageInstigator)
{
	//debugf(TEXT("applying mod %d %d"),ObjIdx,ModIdx);
	// overridemods don't get replicated
	if (Role == ROLE_Authority)
	{
		eventReplicateDamageMod(ObjIdx,ModIdx,bPartial);
	}

	checkSlow(ObjIdx >= 0 && ObjIdx < SubObjects.Num());
	FDestructibleSubobject* const SubObj = &SubObjects(ObjIdx);

	checkSlow(ModIdx >= 0 && ModIdx < SubObj->DamageMods.Num());
	FObjectDamageModifier* const Mod = &SubObj->DamageMods(ModIdx);

	ApplyDamageModInternal(SubObj, Mod, bPartial, DamageInstigator);
}



void AGearDestructibleObject::DamageSubObject(INT ObjIdx, INT Damage, class AController* EventInstigator, class UClass* DamType)
{
//	float Timer=0;
//	CLOCK_CYCLES(Timer);

	if (SubObjectHealths(ObjIdx) > 0.f)
	{
		UBOOL bActuallyApplyDamage = TRUE;
		if (DamType && bLimitDamageTypes)
		{
			UBOOL bFound = FALSE;
			for (INT Idx=0; Idx<VulnerableToDamageType.Num(); ++Idx)
			{
				if ( (DamType == VulnerableToDamageType(Idx)) || (DamType->IsChildOf(VulnerableToDamageType(Idx))) )
				{
					bFound = TRUE;
					break;
				}
			}

			bActuallyApplyDamage = bFound;
		}

		if (bActuallyApplyDamage)
		{
			SubObjectHealths(ObjIdx) = SubObjectHealths(ObjIdx) - (FLOAT)Damage;
			TempDamageCache(ObjIdx) += (FLOAT)Damage;

			INT ModToApplyIdx = -1;

			FDestructibleSubobject* const SubObj = &SubObjects(ObjIdx);

			// search and see if we've triggered any new damage mods
			for (INT Idx=0; Idx<SubObj->DamageMods.Num(); ++Idx)
			{
				FObjectDamageModifier* const Mod = &SubObj->DamageMods(Idx);

				if ( !Mod->bApplied && (SubObjectHealths(ObjIdx) <= Mod->HealthThreshold) ) 
				{
					if (ModToApplyIdx >= 0)
					{
						// find which one to partially apply
						if (Mod->HealthThreshold < SubObj->DamageMods(ModToApplyIdx).HealthThreshold)
						{
							ApplyDamageMod(ObjIdx, ModToApplyIdx, TRUE);
							ModToApplyIdx = Idx;
						}
						else
						{
							ApplyDamageMod(ObjIdx, Idx, TRUE);
						}
					}
					else
					{
						ModToApplyIdx = Idx;
					}
				}
			}
			if (ModToApplyIdx >= 0)
			{
				ApplyDamageMod(ObjIdx, ModToApplyIdx, FALSE);
			}

			// check and see if we need to throw kismet Destroyed event
			if (SubObjectHealths(ObjIdx) <= 0.f)
			{
				RemoveDecals();  // this portion was destroyed so we need to remove decals
				UBOOL bFoundNonZero = FALSE;
				for (INT Idx=0; Idx<SubObjects.Num(); ++Idx)
				{
					if (SubObjectHealths(Idx) > 0.f)
					{
						bFoundNonZero = TRUE;
						break;
					}
				}

				if (!bFoundNonZero)
				{
					eventTriggerDestroyedEvent(EventInstigator);
				}
			}
		}
	}

	//UNCLOCK_CYCLES(Timer);
	//debugf(TEXT("DamageSubObject took %f"), Timer);
}

UBOOL AGearDestructibleObject::ShouldTrace( UPrimitiveComponent* Primitive, AActor *SourceActor, DWORD TraceFlags )
{
	//@fixme - gearsetup
	return Super::ShouldTrace(Primitive,SourceActor,TraceFlags);
	//return !(TraceFlags & TRACE_IgnoreMovers) && ((TraceFlags & TRACE_Movers) || Super::ShouldTrace(Primitive,SourceActor,TraceFlags));
}

UBOOL AGearDestructibleObject::ReachedBy(APawn *P, const FVector &TestPosition, const FVector& Dest)
{
	if(P == NULL)
	{
		return FALSE;
	}	

	if ( TouchReachSucceeded(P, TestPosition) )
		return true;

	if ( !bBlockActors && GWorld->HasBegunPlay() )
		return Super::ReachedBy(P,TestPosition,Dest);
	
	// do a radius check against each one of our colliding components
	FRadiusOverlapCheck CheckInfo(TestPosition, P->GetDefaultCollisionSize().X);
	for (INT i = 0; i < Components.Num(); i++)
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Components(i));

		if (Primitive != NULL && Primitive->IsAttached() && Primitive->CollideActors && CheckInfo.SphereBoundsTest(Primitive->Bounds))
		{
			return TRUE;
		}
	}

	return FALSE;
}


void AGearPawn_LocustWretchBase::MarkEndPoints(ANavigationPoint* EndAnchor, AActor* Goal, const FVector& GoalLocation)
{
	Super::MarkEndPoints( EndAnchor, Goal, GoalLocation );

	bCanClimbCeilings = TRUE;
}

void AGearPC::ZoomToMap( const FString& MapName )
{
	GEngine->SetClientTravel( *MapName, TRAVEL_Absolute );
}

FString AGearPC::GetCurrMapName()
{
	return GWorld->CurrentLevel->GetOutermost()->GetName();
}

FString AGearPC::GetCurrGameType()
{
	AGearGRI *GRI = Cast<AGearGRI>(WorldInfo->GRI);
	if (GRI != NULL)
	{
		UClass *GameClass = WorldInfo->NetMode == NM_Client ? GRI->GameClass : WorldInfo->Game->GetClass();
		if (GameClass != NULL)
		{
			if (GRI->bAnnexIsKOTHRules)
			{
				return Localize(TEXT("GearUIScene_Base"),TEXT("KOTH"),TEXT("GearGame"));
			}
			if (GRI->bGameIsExecutionRules && GameClass->IsChildOf(AGearGameTDM_Base::StaticClass()))
			{
				return Localize(TEXT("GearUIScene_Base"),TEXT("Execution"),TEXT("GearGame"));
			}
			return GameClass->GetName();
		}
	}
	return Localize(TEXT("GearUIScene_Base"),TEXT("Warzone"),TEXT("GearGame"));
}



void AHeadTrackTarget::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * 500.0f;

	const FLOAT Multiplier = ( ModifiedScale.X > 0.0f || ModifiedScale.Y > 0.0f || ModifiedScale.Z > 0.0f ) ? 1.0f : -1.0f;
	Radius += Multiplier * ModifiedScale.Size();
	Radius = Max( 0.f, Radius );
	PostEditChange( NULL );
}


/** Update the render component to match the force radius. */
void AHeadTrackTarget::PostEditChange(UProperty* PropertyThatChanged)
{
	if (SphereRenderComponent)
	{
		FComponentReattachContext ReattachContext(SphereRenderComponent);
		SphereRenderComponent->SphereRadius = Radius;
	}
}

UBOOL AHeadTrackTarget::CanAffectPawn(AGearPawn* GP) const
{
	if (GP)
	{
		switch (PawnFilter)
		{
		case HT_PlayersOnly:
			return GP->IsLocallyControlled();

		case HT_AllPawns:
			return TRUE;

		case HT_PlayerTeamOnly:
			{
				APlayerController* const PC = ((GEngine->GamePlayers.Num() > 0) && (GEngine->GamePlayers(0) != NULL)) ? GEngine->GamePlayers(0)->Actor : NULL;
				if (PC)
				{
					return GP->GetTeamNum() == PC->GetTeamNum();
				}
			}
		}
	}

	return FALSE;
}


/*
[2006-07-13--18:45:14] BigSquid: but get length of Point
[2006-07-13--18:45:22] BigSquid: check that against ConeLength

[2006-07-13--18:45:57] BigSquid: reject if Point > ConeLength of course
[2006-07-13--18:46:15] BigSquid: then divide Point by PointLength to normalize it (and save a sqrt)
[2006-07-13--18:46:24] BigSquid: also reject if DotResult is negative
[2006-07-13--18:46:36] BigSquid: because you don't want the cone behind you
[2006-07-13--18:46:58] BigSquid: I would also not do the appCos every time
[2006-07-13--18:51:08] msew@epic: heh yeah my comments have have the dot > 0  ahh  and just move the appCos out to top nods nods!

*/

// can express this via a dot product constraint. essentially, you want the dot product of the 
// normalized 
// cone axis and the normalized vertex-to-test-point vector to be <= cos(cone_angle) (and >= 0)
void AGearPC::ConeHurt( AGearPawn* TargetPawn, FVector CamLoc, FVector HitNormal, FLOAT AimError )
{
	if( TargetPawn == NULL )
	{
		return;
	}

	FVector ConeStart = CamLoc;
	FVector ConeNormal = HitNormal;
	FLOAT ConeLength = 4500.0f;
	FLOAT ConeAngle = 30;

	FVector Point = TargetPawn->Location + FVector(0,0,30);
	FVector ConeToPoint;

	Point = ConeStart - Point;

	ConeNormal.Normalize();
	Point.Normalize();

	FLOAT DotResult = ConeNormal | Point;

	if( ( DotResult <= appCos( ConeAngle ) ) && ( DotResult >= 0 ) )
	{
		// inside the cone
		debugf( TEXT( "inside") );
	}
	else
	{
		// outside the cone
		debugf( TEXT( "outside") );
	}
}

// Get the localized string
FString AGearPointOfInterest::RetrievePOIString(const FString& TagName)
{
	return Localize( TEXT("POIS"), *TagName, TEXT("GearGame") );
}

// Get the localized string
FString AGearSpectatorPoint::RetrieveDisplayString(const FString& TagName )
{
	return Localize( TEXT("SpectatorCams"), *TagName, TEXT("GearGame") );
}

UBOOL AHOD_BeamManagerBase::IsAValidHODTarget(const FVector& Loc)
{
	if (bEnabled)
	{
		FVector const EndTrace = Loc + DirectionToDeathRayOriginNorm * 20000.f;

		FCheckResult SingleHit;
		DWORD const TraceFlags = TRACE_World /*| TRACE_Pawns/ */;

		GWorld->SingleLineCheck(SingleHit, NULL, EndTrace, Loc, TraceFlags);

		if (SingleHit.Actor == NULL)
		{

#if !FINAL_RELEASE
			if( bDebug )
			{
				debugf( TEXT("Validity clear!"));
			}
#endif

			// hit nothing, target is clear
			return TRUE;
		}

#if !FINAL_RELEASE
		if( bDebug )
		{
			debugf( TEXT("Validity trace hit... %s"), *SingleHit.Actor->GetName());
		}
#endif
	}

	return FALSE;
}

/**
* Returns camera "no render" cylinder.
* @param bViewTarget TRUE to return the cylinder for use when this pawn is the camera target, false to return the non-viewtarget dimensions
*/
void AGearPawn::GetCameraNoRenderCylinder(FLOAT& Radius, FLOAT& Height, UBOOL bViewTarget, UBOOL bHiddenLocally)
{
	if ( (CoverType == CT_MidLevel) && ((CoverAction != CA_PopUp) || bDoing360Aiming) )
	{
		if (bViewTarget)
		{
			Radius = CameraNoRenderCylinder_Low_ViewTarget.Radius;
			Height = CameraNoRenderCylinder_Low_ViewTarget.Height;
		}
		else
		{
			Radius = CameraNoRenderCylinder_Low.Radius;
			Height = CameraNoRenderCylinder_Low.Height;
		}
	}
	else
	{
		if (bViewTarget)
		{
			Radius = CameraNoRenderCylinder_High_ViewTarget.Radius;
			Height = CameraNoRenderCylinder_High_ViewTarget.Height;
		}
		else
		{
			Radius = CameraNoRenderCylinder_High.Radius;
			Height = CameraNoRenderCylinder_High.Height;
		}
	}

	// if already hidden, we expand the cylinder a little bit to prevent flickering caused
	// by the camera being right on the boundary.
	if (bHiddenLocally)
	{
		Radius += CameraNoRenderCylinder_FlickerBuffer.Radius;
		Height += CameraNoRenderCylinder_FlickerBuffer.Height;
	}
}


void AGearPawn::UpdateAutomaticHeadTracking()
{
	AGearGRI* const WGRI = Cast<AGearGRI>(WorldInfo->GRI);
	if (WGRI)
	{
		// find closest target
		FLOAT BestHTTDistSq = FLT_MAX;
		AHeadTrackTarget* BestHTT = NULL;
		INT const NumHTTs = WGRI->HeadTrackTargetCache.Num();
		for (INT Idx=0; Idx<NumHTTs; ++Idx)
		{
			AHeadTrackTarget* const HTT = WGRI->HeadTrackTargetCache(Idx);
			if ( HTT && HTT->bEnabled )
			{
				FLOAT DistSq = (HTT->Location - Location).SizeSquared();
				if ( (DistSq < BestHTTDistSq) && (DistSq < Square<FLOAT>(HTT->Radius)) && HTT->CanAffectPawn(this) )
				{
					BestHTT = HTT;
					BestHTTDistSq = DistSq;
				}
			}
		}

		if (BestHTT)
		{
			// override only if going from one htt to another
			UBOOL const bOverride = (BestHTT != CurrentHeadTrackTargetActor) && (CurrentHeadTrackTargetActor != NULL);
			AActor* const PrevHeadTrackTarget = CurrentHeadTrackTargetActor;

			// use alt?
			AActor* LookatActor = BestHTT;
			FName LookatBoneName = NAME_None;
			if (BestHTT->AlternateLookatActor != NULL)
			{
				LookatActor = BestHTT->AlternateLookatActor;
				LookatBoneName = BestHTT->AlternateLookAtActorBoneName;
			}

			// set it
			if (eventSetHeadTrackActor(BestHTT, LookatBoneName, bOverride))
			{
				CurrentHeadTrackTargetActor = BestHTT;

				if (PrevHeadTrackTarget != CurrentHeadTrackTargetActor)
				{
					// see if we're looking at a discoverable, and do special handling if so
					AGearDiscoverablesPickupFactoryBase* const Disc = Cast<AGearDiscoverablesPickupFactoryBase>(CurrentHeadTrackTargetActor->Owner);
					if (Disc && !Disc->bDidGUDSHint)
					{
						AGearPC* const GPC = Cast<AGearPC>(Controller);
						if (GPC && !GPC->eventHasFoundDiscoverable(Disc->DISC_DiscoverableType))
						{
							AGearGame* const GearGame = Cast<AGearGame>(WorldInfo->Game);
							if (GearGame && GearGame->UnscriptedDialogueManager)
							{
								GearGame->UnscriptedDialogueManager->PlayGUDSAction(GUDAction_NoticedACollectible, this);
								Disc->eventNotifyLookedAt();
							}
						}
					}
				}
			}
		}
		else if (CurrentHeadTrackTargetActor)
		{
			// stop tracking
			eventSetHeadTrackActor(NULL);
			CurrentHeadTrackTargetActor = NULL;
		}
	}
}

UBOOL ATurret::IgnoreBlockingBy( const AActor *Other) const
{
	// Avoid turrets on moving platforms being displaced by collision with the environment
	if(Other->Physics == PHYS_None || Other->Physics == PHYS_Interpolating)
	{
		return TRUE;
	}

	// Don't stop our user from moving
	if(Driver && Driver == Other)
	{
		return TRUE;
	}

	return Super::IgnoreBlockingBy(Other);
}

void ATurret::UpdateDriver(FLOAT DeltaTime)
{
	AGearPawn* P = Cast<AGearPawn>(Driver);
	if( !P || !P->Mesh || P->bDeleteMe || P->bPlayedDeath || !bDriving )
	{
		return;
	}

	// Force updating the Turret's mesh, we need bones in correct location
	if( Mesh )
	{
		Mesh->ForceSkelUpdate();
	}

	// Align Pawn Rotation with turret Aiming.
	FRotator DriverRot = eventGetDriverIdealRotation();
	if( P->Rotation.Yaw != DriverRot.Yaw )
	{
		FRotator NewPawnRotation = P->Rotation;
		NewPawnRotation.Yaw = DriverRot.Yaw;
		P->SetRotation( NewPawnRotation );
	}

	// Move Pawn with turret
	const	FVector	DesiredPawnLoc	= eventGetDriverIdealPosition();
	const	FVector	OldPawnLoc		= P->Location;
			FVector DeltaMove		= DesiredPawnLoc - OldPawnLoc;

	// No Z movement in walking physics.
	DeltaMove.Z = 0.f;

	// Use Root Motion as a hack to force the Pawn to move at a specific location using physics.
	P->Mesh->RootMotionMode					= RMM_Accel;
	P->Mesh->PreviousRMM					= RMM_Accel;
	P->Mesh->RootMotionDelta.Translation	= DeltaMove;
	// By default Physics abort when there is no controller, override that.
	P->bRunPhysicsWithNoController			= TRUE;

	P->performPhysics(DeltaTime);

	// Restore previous state
	P->Mesh->RootMotionMode					= RMM_Ignore;
	P->Mesh->PreviousRMM					= RMM_Ignore;
	P->bRunPhysicsWithNoController			= FALSE;

	// Clear velocity, only allow to fall down.
	P->Velocity = FVector(0.f, 0.f, P->Velocity.Z);

	// Update DeltaMove with actual movement
	DeltaMove = P->Location - OldPawnLoc;
	// FULL OF BEES! MT-> THIS probably isn't the best way to fix this.. but we want the magic animation to work, and we want our position to not be lame too.. Weee
	GWorld->FarMoveActor(P,DesiredPawnLoc+FVector(0.f,0.f,InitialEntryZOffset),FALSE,FALSE,TRUE);

	// Fake velocity so animations play movement.
	if( P->Mesh )
	{
		if( DeltaMove.SizeSquared() > 0.01f )
		{
			P->Mesh->RootMotionVelocity = DeltaMove / DeltaTime;
		}
		else
		{
			P->Mesh->RootMotionVelocity = FVector(0.f);
		}
	}

	// Set driver's aim offset
	FVector2D	NewAimOffsetPct;

	NewAimOffsetPct.X	= 0.f;
	NewAimOffsetPct.Y	= FLOAT(FRotator::NormalizeAxis(DriverRot.Pitch)) / 16384.f;

	P->UpdateAimOffset(NewAimOffsetPct, DeltaTime);

	// Set Hand IK
	if( Mesh && LeftHandBoneHandleName != NAME_None && RightHandBoneHandleName != NAME_None )
	{
		if( P->IKCtrl_RightHand )		P->IKCtrl_RightHand->EffectorLocation	= Mesh->GetBoneLocation(LeftHandBoneHandleName);
		if( P->IKCtrl_LeftHand )		P->IKCtrl_LeftHand->EffectorLocation	= Mesh->GetBoneLocation(RightHandBoneHandleName);
		if( P->IKRotCtrl_RightHand )	P->IKRotCtrl_RightHand->BoneRotation	= Mesh->GetBoneQuaternion(LeftHandBoneHandleName);
		if( P->IKRotCtrl_LeftHand )		P->IKRotCtrl_LeftHand->BoneRotation		= Mesh->GetBoneQuaternion(RightHandBoneHandleName);
	}

	// Set mesh as dirty, so it's updated with new Driver location, turret location, and IK Effector location, with no frame of lag.
	P->Mesh->BeginDeferredReattach();
}

/** returns absolute Aim direction of turret */
FRotator ATurret::GetTurretAimDir()
{
	return AimDir;
}

/** Physical fire start location. (from weapon's barrel in world coordinates) */
FVector ATurret::GetPhysicalFireStartLoc( FVector FireOffset )
{
	FVector	X, Y, Z;

	if(Mesh == NULL)
	{
		return FVector(0.f);
	}

	FRotationMatrix RMat(GetTurretAimDir());
	RMat.GetAxes(X, Y, Z);
	return (Mesh->GetBoneLocation(PitchBone) + CannonFireOffset.X*X + CannonFireOffset.Y*Y + CannonFireOffset.Z*Z);
}

FVector ATurret::GetPawnViewLocation()
{
	return eventGetPhysicalFireStartLoc( FVector(0.f) );
}

void AGearTurret::UpdateForMovingBase(class AActor* BaseActor)
{
	if (BaseActor && !BaseActor->bStatic)
	{
		FMatrix NewBaseTM = BaseActor->LocalToWorld();
		NewBaseTM.RemoveScaling();

		// don't want this for drivers, so player can have steady aim and not need to compensate for 
		// base object motion (unless turret aim limits are reached)
		if (Driver == NULL)
		{
			// here we update the aimdir to account for the motion of the base actor.
			FRotationMatrix const AimTM(AimDir);
			FMatrix const RelAimTM = AimTM * LastBaseTM.Inverse();
			FMatrix const UpdatedAimTM = RelAimTM * NewBaseTM;
			AimDir = UpdatedAimTM.Rotator();
		}

		if (bEnforceHardAttach && bHardAttach && bBlockActors)
		{
			// move turret to new relative location by hand
			FRotationTranslationMatrix HardRelMatrix(RelativeRotation, RelativeLocation);
			FMatrix const NewWorldTM = HardRelMatrix * NewBaseTM;
			FVector const WorldDelta = NewWorldTM.GetOrigin() - Location;
			FRotator const NewWorldRot = NewWorldTM.Rotator();

			FCheckResult Hit(1.0f);
			bMovingToEnforceHardAttach = TRUE;
			GWorld->MoveActor( this, WorldDelta, NewWorldRot, MOVE_IgnoreBases | MOVE_NoFail, Hit );
			bMovingToEnforceHardAttach = FALSE;
		}	

		LastBaseTM = NewBaseTM;
	}
}

void AGearTurret::PostBeginPlay()
{
	Super::PostBeginPlay();

	if (Base != NULL && !Base->bStatic)
	{
		LastBaseTM = Base->LocalToWorld();
		LastBaseTM.RemoveScaling();
	}
}

UBOOL AGearTurret::IgnoreBlockingBy( const AActor *Other) const
{
	/*
	// when enforcing hard attach, we want to ignore blocking against other objects when we move
	// note we only do this when the turret is moving to it's new position.  we want other things to 
	// block normally against the turret
	if (bMovingToEnforceHardAttach && bEnforceHardAttach && bHardAttach && bBlockActors)
	{
		return TRUE;
	}
	*/

	return Super::IgnoreBlockingBy(Other);
}


FVector ATurret_TroikaBase::GetPhysicalFireStartLoc( FVector FireOffset )
{
	FVector	MuzzleLoc;

	if(Mesh==NULL)
	{
		return FVector(0.f);
	}
	Mesh->GetSocketWorldLocationAndRotation(MuzzleSocketName, MuzzleLoc, NULL);
	return MuzzleLoc;
}

FRotator ATurret_TroikaBase::GetViewRotation()
{
	FVector	MuzzleLoc;
	FRotator MuzzleRot;

	if(Mesh == NULL)
	{
		return Super::GetViewRotation();
	}

	Mesh->GetSocketWorldLocationAndRotation(MuzzleSocketName, MuzzleLoc, &MuzzleRot);
	return MuzzleRot;
}

/**
 * Aborts any StopMovie calls that may be pending through the FDelayedUnpauser.
 */
void AGearPC::KeepPlayingLoadingMovie()
{
	// Abort any currently active unpauser
	if ( FDelayedPauserAndUnpauser::HasPendingUnpauser() )
	{
		debugf(NAME_DevMovie, TEXT("Aborting the current FDelayedUnpauser"));
		FDelayedPauserAndUnpauser::AbortPendingUnpauser();
	}
}

/**
 * Starts/stops the loading movie
 *
 * @param bShowMovie true to show the movie, false to stop it
 * @param bPauseAfterHide (optional) If TRUE, this will pause the game/delay movie stop to let textures stream in
 * @param PauseDuration (optional) allows overriding the default pause duration specified in .ini (only used if bPauseAfterHide is true)
 * @param KeepPlayingDuration (optional) keeps playing the movie for a specified more seconds after it's supposed to stop
 */
void AGearPC::ShowLoadingMovie(UBOOL bShowMovie, UBOOL bPauseAfterHide, FLOAT PauseDuration, FLOAT KeepPlayingDuration)
{
	if ( bShowMovie )
	{
		// Abort any currently active unpauser
		KeepPlayingLoadingMovie();
	}

	if (bShowMovie)
	{
		debugf(NAME_DevMovie, TEXT("ShowLoadingMovie(TRUE)"));
		if( GFullScreenMovie && 
			GFullScreenMovie->GameThreadIsMoviePlaying(UCONST_LOADING_MOVIE) == FALSE )
		{
//			debugf(TEXT("*****************************************************    SHOW LOADINGMOVIE (bPauseAfterHide:%s PauseDuration:%f)    *****************************************************"), bPauseAfterHide ? GTrue : GFalse, PauseDuration);
			GFullScreenMovie->GameThreadPlayMovie(MM_LoopFromMemory, UCONST_LOADING_MOVIE);
		}
	}
	else
	{
		if( GFullScreenMovie && 
			GFullScreenMovie->GameThreadIsMoviePlaying(UCONST_LOADING_MOVIE) == TRUE &&
			(!bPauseAfterHide || !FDelayedPauserAndUnpauser::HasPendingUnpauser()) )
		{
//			debugf(TEXT("#####################################################    STOP LOADINGMOVIE (bPauseAfterHide:%s PauseDuration:%f)    #####################################################"), bPauseAfterHide ? GTrue : GFalse, PauseDuration);
			// if we want to pause after hiding the movie, we use the elayed pauser/unpauser so we stay unpaused
			// for a couple of frames, and then pause while streaming textures, then unpause again
			if (bPauseAfterHide)
			{
				// leave movie playing longer with game paused for textures to stream in.
				if (PauseDuration <= 0.0f)
				{
					verify(GConfig->GetFloat(TEXT("StreamByURL"), TEXT("PostLoadPause"), PauseDuration, GEngineIni));
				}

				// pause while streaming textures.
				if(GEngine && GEngine->GamePlayers(0) && GEngine->GamePlayers(0)->Actor)
				{
					GEngine->GamePlayers(0)->Actor->eventConditionalPause(TRUE);
				}

				debugf(NAME_DevMovie, TEXT("Launching a FDelayedUnpauser (PauseDuration=%.1fs KeepPlayingDuration=%.1fs)"), PauseDuration, KeepPlayingDuration);
				new FDelayedPauserAndUnpauser(0.1f, PauseDuration, PauseDuration + 0.1 + KeepPlayingDuration, FString(UCONST_LOADING_MOVIE));
			}
			// Are we supposed to keep playing the loading movie for a bit (and not pause the game)?
			else if ( appIsNearlyZero(KeepPlayingDuration) == FALSE )
			{
				debugf(NAME_DevMovie, TEXT("Launching a FDelayedUnpauser (PauseDuration=0.0s KeepPlayingDuration=%.1fs)"), KeepPlayingDuration);
				new FDelayedPauserAndUnpauser(0.0f, 0.0f, 0.1 + KeepPlayingDuration, FString(UCONST_LOADING_MOVIE));
			}
			// otherwise, just stop it now
			else
			{
				GFullScreenMovie->GameThreadStopMovie();
			}
		}
	}
}


void AEmit_CameraLensEffectBase::UpdateLocation(const FVector& CamLoc, const FRotator& CamRot, FLOAT CamFOVDeg)
{
	FRotationMatrix M(CamRot);

	// the particle is FACING X being parallel to the Y axis.  So we just flip the entire thing to face toward the player who is looking down X
	const FVector& X = M.GetAxis(0);
	M.SetAxis(0, -X);
	M.SetAxis(1, -M.GetAxis(1));

	const FRotator& NewRot = M.Rotator();

	// base dist assumes fov of 80 deg
	const FLOAT DistAdjustedForFOV = DistFromCamera * appTan(float(80.f*0.5f*PI/180.f)) / appTan(float(CamFOVDeg*0.5f*PI/180.f));

	SetLocation( CamLoc + X * DistAdjustedForFOV );
	SetRotation( NewRot );

	// have to do this, since in UWorld::Tick the actors do the component update for all
	// actors before the camera ticks.  without this, the blood appears to be a frame behind
	// the camera.
	ConditionalUpdateComponents();
}

INT AGearPC::ChooseCameraBoneAnim(const TArray<struct FCameraBoneAnimation>& Anims)
{
	// start traces from camera
	FVector TraceStart;
	{
		FRotator UnusedRot(0,0,0);
		eventGetPlayerViewPoint(TraceStart, UnusedRot);
	}

	INT BestTolerableAnim = -1;
	TArray<INT> GreatCandidateAnims;

	//FlushPersistentDebugLines();
	//DrawDebugSphere(TraceStart, 16, 12, 0, 255, 0, TRUE);

	if (Pawn)
	{
		// build list of anims to choose from
		for (INT Idx=0; Idx<Anims.Num(); ++Idx)
		{
			// test vectors are offset from pawnloc
			FVector const TraceEnd = Pawn->Location + ::RotateVector(Anims(Idx).CollisionTestVector, Rotation);

			//DrawDebugSphere(TraceEnd, 16, 12, 255, 64*Idx, 64*Idx, TRUE);

			FCheckResult SingleHit;
			GWorld->SingleLineCheck(SingleHit, Pawn, TraceEnd, TraceStart, TRACE_World, FVector(12.f,12.f,12.f));

			if (SingleHit.Actor == NULL)
			{
				GreatCandidateAnims.AddItem(Idx);
			}
			// for now, any hit is intolerable.  we could flex this a bit by
			// setting a tolerance, such as 90% of the trace length or something.
			//else
			//{
			//	DistSq = VSizeSq(HitLoc - TraceStart);
			//	if ( (DistSq < BestDistSq) || (BestTolerableAnim < 0) )
			//	{
			//		BestTolerableAnim = Idx;
			//		BestDistSq = DistSq;
			//	}
			//}
		}
	}

	INT const NumCandidateAnims = GreatCandidateAnims.Num();
	if (NumCandidateAnims > 0)
	{
		UINT RandIdx = appRand() % NumCandidateAnims;
		return GreatCandidateAnims(RandIdx);
	}
	else
	{
		return BestTolerableAnim;
	}
}
void AGearPC::CauseHitch(FLOAT Sec)
{
	appSleep(Sec);
}

BYTE AGearPC::GetCoverDirection()
{
	return (MyGearPawn != NULL ? MyGearPawn->CoverDirection : CD_Default);
}


INT AGearPC::ChooseRandomCameraAnim(const TArray<class UCameraAnim*>& Anims, FLOAT Scale, UBOOL bDoNotRandomize)
{
	INT const NumAnims = Anims.Num();

	if ( (NumAnims > 0) && PlayerCamera )
	{
		// @fixme, better shuffling?
		INT RandStartIdx = bDoNotRandomize ? 0 : RandHelper(NumAnims);

		for (INT Idx=0, AnimIdx=RandStartIdx; Idx<NumAnims; ++Idx, ++AnimIdx)
		{
			if (AnimIdx >= NumAnims)
			{
				AnimIdx = 0;
			}

			UCameraAnim* const Anim = Anims(AnimIdx);
			if( CameraAnimHasEnoughSpace( Anim ) )
			{
				return AnimIdx;
			}
		}
	}

	// @todo, if we're returning -1 too often, maybe keep track of the best anim in the loop
	// and return it, subject to some acceptability threshold

	return INDEX_NONE;
}

UBOOL AGearPC::CameraAnimHasEnoughSpace( UCameraAnim* Anim, FLOAT Scale )
{
	if( Anim != NULL && PlayerCamera != NULL )
	{
		//	static UBOOL bDebugDrawBox = 1;

		// get world space AABB of the cameraanim's path
		FBox AnimAABB = Anim->GetAABB(PlayerCamera->Location, PlayerCamera->Rotation, Scale);
		FVector BoxCenter, BoxExtent;
		AnimAABB.GetCenterAndExtents(BoxCenter, BoxExtent);

		// tighten the extents somewhat, since we can tolerate a little world penetration.
		static FLOAT ExtentScale = 0.75f;
		BoxExtent *= ExtentScale;

		// test for penetration
		FCheckResult Hit(1.f);
		if (GWorld->EncroachingWorldGeometry(Hit, BoxCenter, BoxExtent, FALSE) == FALSE)
		{
			// no penetration, return it!
			//if (bDebugDrawBox)
			//{
			//	FlushPersistentDebugLines();
			//	DrawDebugBox(BoxCenter, BoxExtent, 255, 255, 255, TRUE);
			//}

			return TRUE;
		}
	}

	return FALSE;
}


/** Cap max speed of pushable actors. */
void AInterpActor_Pushable::physRigidBody(FLOAT DeltaSeconds)
{
	Super::physRigidBody(DeltaSeconds);

	if(CollisionComponent && Velocity.Size() > MaxSpeed)
	{
		Velocity = Velocity.SafeNormal() * MaxSpeed;
		CollisionComponent->SetRBLinearVelocity(Velocity, FALSE);
	}
}

/** Do network send/receieve before doing normal physics sync'ing */
void AGearDroppedPickup::physRigidBody(FLOAT DeltaTime)
{
	// If its a network game.
	if( GWorld->GetNetMode() != NM_Standalone )
	{
		// If we are the authority - pack current physics state into struct.
		if( Role == ROLE_Authority )
		{
			UBOOL bSuccess = GetCurrentRBState(RBState);
			if(bSuccess)
			{
				RBState.bNewData |= UCONST_RB_NeedsUpdate;
			}
		}
		// If we are a client, see if we have receieved new state. If so, apply it to the physics.
		else
		{
			if (RBState.bNewData & UCONST_RB_NeedsUpdate)
			{
				FVector OutDeltaPos;
				ApplyNewRBState(RBState, &AngErrorAccumulator, OutDeltaPos);
				RBState.bNewData = 0;
			}
		}
	}

	Super::physRigidBody(DeltaTime);
}

/**
* Game-specific code to handle DLC being added or removed
* 
* @param bContentWasInstalled TRUE if DLC was installed, FALSE if it was removed
*/
void appOnDownloadableContentChanged(UBOOL bContentWasInstalled)
{
	// refresh the data provider
	UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();
	if (DataStoreClient != NULL)
	{
		// @todo: What is this called in GearGame?
		UUIDataStore_GameResource* DataStore = (UUIDataStore_GameResource*)DataStoreClient->FindDataStore(FName(TEXT("WarfareGameResources")));
		if (DataStore)
		{
			// reparse the .ini files for the PerObjectConfig objects
			DataStore->InitializeListElementProviders();
			// make ay currently active widgets using the data get refreshed
			DataStore->eventRefreshSubscribers();
		}
	}
}

FSystemSettings& GetSystemSettings()
{
	static FSystemSettings SystemSettingsSingleton;
	return SystemSettingsSingleton;
}

static UBOOL DoCentaurRepulsion(AVehicle_Centaur_Base* Centaur, const FVector& RepelLoc, FLOAT RepelRadius, FLOAT RepelStrength, UBOOL bShowRepulsion)
{
	FVector ToCentaur = Centaur->Location - RepelLoc;
	FLOAT DistToCentaur = ToCentaur.Size();
	if(DistToCentaur < RepelRadius && DistToCentaur > KINDA_SMALL_NUMBER)
	{
		// Normalize direction vector for force.
		ToCentaur /= DistToCentaur;
		// Remove Z component - only want to push in XY plane
		ToCentaur.Z = 0.f;

		Centaur->AddForce(ToCentaur * RepelStrength);

#if !FINAL_RELEASE
		// If desired, draw DEBUG line
		if(bShowRepulsion)
		{
			GWorld->LineBatcher->DrawLine(Centaur->Location, Centaur->Location + (ToCentaur * RepelStrength * 10.f) , FColor(0,0,255), SDPG_World);
		}
#endif

		return TRUE;
	}

	return FALSE;
}

/** Brumak Tick. Check for legs/arms dealing damage */
void AGearPawn_LocustBrumakBase::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// Check for legs/arms dealing damage on server.
	if( GWorld->GetWorldInfo()->NetMode != NM_Client && Controller && Mesh )
	{
		FCheckResult Hit;
		FVector BoneLocation;

		for(INT Index=0; Index<DmgDealingLimbs.Num(); Index++)
		{
			if( !Mesh->GetSocketWorldLocationAndRotation(DmgDealingLimbs(Index).BoneName, BoneLocation, NULL) )
			{
				// Skip when not a socket, we should check for bones otherwise. But right now we're only using sockets.
				continue;
			}

			// Make sure last location has been initialized
			if( !DmgDealingLimbs(Index).LastLocation.IsZero() )
			{
				const FVector Delta = BoneLocation - DmgDealingLimbs(Index).LastLocation;

				// Skip trace if movement is too small.
				if( Delta.SizeSquared() > 1.f )
				{
					const FVector Extent(1,1,1);
					const UBOOL bHitSomething = !GWorld->SingleLineCheck(Hit, this, BoneLocation, DmgDealingLimbs(Index).LastLocation, TRACE_ProjTargets, Extent*DmgDealingLimbs(Index).BoxSize);
					if( bHitSomething && Hit.Actor && (Hit.Actor != GWorld->GetWorldInfo()) )
					{
						eventDmgDealingLimbsCollision(Hit.Actor, Hit.Location, Hit.Normal);
					}
				}
			}

			// Save last location for next round.
			DmgDealingLimbs(Index).LastLocation = BoneLocation;
		}
	}

	// If desired, do centaur repulsion
	if(bEnableCentaurRepulsion && GWorld->GetWorldInfo())
	{
		FVector RepelLocations[2];
		RepelLocations[0] = Mesh->GetBoneLocation(RightFootBoneName);
		RepelLocations[1] = Mesh->GetBoneLocation(LeftFootBoneName);

		APawn* CurrentP = GWorld->GetWorldInfo()->PawnList;
		while (CurrentP)
		{
			AVehicle_Centaur_Base* Centaur = Cast<AVehicle_Centaur_Base>(CurrentP);
			if(Centaur)
			{
				UBOOL bRepulsed = FALSE;
				bRepulsed = DoCentaurRepulsion(Centaur, RepelLocations[0], CentaurRepulsionRadius, CentaurRepulsionStrength, bShowCentaurRepulsion) || bRepulsed;
				bRepulsed = DoCentaurRepulsion(Centaur, RepelLocations[1], CentaurRepulsionRadius, CentaurRepulsionStrength, bShowCentaurRepulsion) || bRepulsed;

				if( bRepulsed )
				{
					eventRanInto( Centaur );
				}
			}

			CurrentP = CurrentP->NextPawn;
		}

	}
}

FVector AGearPawn_LocustBrumakBase::GetPhysicalFireStartLoc( FVector FireOffset )
{
	if( IsHumanControlled() )
	{
		return Super::GetPhysicalFireStartLoc( FireOffset );
	}

	return eventGetPawnViewLocation();
}

UBOOL AGearPawn_LocustBrumakBase::IgnoreBlockingBy( const AActor* Other ) const
{
	if( Other->IsA( AGearPawn::StaticClass() ) != NULL )
	{
		return TRUE;
	}
	else
	{
		return Super::IgnoreBlockingBy( Other );
	}
}


void AInterpActor_GearBasePlatform::FixBasedPawn( AGearPawn *GP )
{
	if( GP == NULL )
		return;

	ANavigationPoint* BestNav = NULL;
	FLOAT BestDist = 10000000000.f;

	// Look for the closest occupiable nav point on the derrick
	FVector OriginalLoc = GP->Location;
	for( INT NavIdx = 0; NavIdx < AttachedNavList.Num(); NavIdx++ )
	{
		ANavigationPoint* Nav = AttachedNavList(NavIdx);
		if( Nav == NULL )
		{
			AttachedNavList.Remove( NavIdx--, 1 );
			continue;
		}

		FLOAT DistSq = (GP->Location - Nav->Location).SizeSquared();
		if( BestNav == NULL || DistSq < BestDist )
		{
			UBOOL bFarMoveGood = GWorld->FarMoveActor( GP, Nav->Location, TRUE );
			if( bFarMoveGood )
			{
				BestNav = Nav;
				BestDist = DistSq;
			}
			GWorld->FarMoveActor(GP, OriginalLoc, TRUE);
		}
	}

	if( BestNav != NULL )
	{
		debugf(TEXT("Forcing %s back on clamped base [%s] by teleporting to %s"), *GP->GetName(), *GetName(), *BestNav->GetName());
		AGearAI* AI = Cast<AGearAI>(GP->Controller);
		if( AI != NULL )
		{
			AI->eventTeleportToLocation( BestNav->Location, GP->Rotation, TRUE );		
		}
		else
		{
			AGearPC* PC = Cast<AGearPC>(GP->Controller);
			if( PC != NULL )
			{
				FVector Extent = GP->GetCylinderExtent();
				GP->SetLocation(BestNav->Location);
				if (!PC->LocalPlayerController())
				{
					PC->eventClientSetLocationAndBase(GP->Location, this);
				}
			}
		}
				
		GP->SetBase( this );
	}
}

UBOOL AInterpActor_GearBasePlatform::InStasis()
{
	return bHidden;
}

UBOOL AInterpActor_GearBasePlatform::IsStillOnClampedBase(AGearPawn* GP, FVector PawnExtent)
{
	// ** Trace down to make sure we are standing on the platform
		
	if(!GP)
	{
		return FALSE;
	}

	// start at the top of the collision cylinder
	FVector Start = GP->Location;
	// end up half the collision height below the cylinder
	FVector End = GP->Location - FVector(0.f,0.f,PawnExtent.Z);
	
	FVector TraceExtent = PawnExtent * FVector(0.9f,0.9f,0.5f);


	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* MultiHit = GWorld->MultiLineCheck
		(
		GMainThreadMemStack,
		End,
		Start,
		TraceExtent,
		TRACE_Movers|TRACE_Blocking,
		GP,
		NULL
		);

	UBOOL bHitBasePlatForm = FALSE;
	//debugf(TEXT("%s CHECKING..."),*GP->GetName());
	// If we hit something solid or a physics volume that causes damage
	for( FCheckResult* Hit = MultiHit; Hit; Hit = Hit->GetNext() )
	{
		//debugf(TEXT("%s-> HIT %s Time: %.2f StartPenetrating?:%i "),*GP->GetName(),*Hit->Actor->GetName(), Hit->Time,Hit->bStartPenetrating);
		if ( Hit->Actor && 
			(!Hit->bStartPenetrating && Hit->Time > 0.1f) &&
			(Hit->Actor->GetAPawn() == NULL) &&
			( 
				(Hit->Actor == this) ||
				(Hit->Actor->Base == this) ||
				(Hit->Actor == this->Base) 
			)
		   )
		{
			bHitBasePlatForm=TRUE;
			break;
		}
	}

      
	/*
	if(!bHitBasePlatForm)
	{
		DrawDebugLine(Start,End,255,0,0,TRUE);
		DrawDebugBox(End ,TraceExtent,255,0,0,TRUE);
		DrawDebugBox(Start,TraceExtent,255,128,0,TRUE);
		DrawDebugLine(Start,Start+FVector(0,0,1000),255,255,0,TRUE);
	}
	*/
        
        

	Mark.Pop();
	return bHitBasePlatForm;
}



void AInterpActor_COGDerrickBase::PostNetReceive()
{
	Super::PostNetReceive();

	// propagate bHidden to wheel actor and attached turrets
	if (WheelActor != NULL)
	{
		WheelActor->SetHidden(bHidden);
	}
	for (INT i = 0; i < Attached.Num(); i++)
	{
		ATurret* Turret = Cast<ATurret>(Attached(i));
		if (Turret != NULL)
		{
			Turret->SetHidden(bHidden);
		}
	}
}

void AInterpActor_COGDerrickBase::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	if (!bHidden && bEngineRunning && !bNoAudio)
	{
		// see if there's a local player riding this derrick
		APawn* LocallyControlledRiderPawn = NULL;
		for( INT PlayerIdx=0; PlayerIdx<GEngine->GamePlayers.Num(); PlayerIdx++ )
		{
			ULocalPlayer* LP = GEngine->GamePlayers(PlayerIdx);
			if (LP)
			{
				AGearPC* GPC = Cast<AGearPC>(LP->Actor);
				if ( GPC && GPC->Pawn && (GPC->Pawn->Base == this) )
				{
					LocallyControlledRiderPawn = GPC->Pawn;
					break;
				}
			}
		}

		// set audio to handle new situation, if necessary	
		if ( (LocallyControlledRiderPawn == NULL) && (bPlayerAudioIsPlaying || !bNonPlayerAudioIsPlaying) )
		{
			eventSetupAudio(FALSE, TRUE, LocallyControlledRiderPawn);
		}
		else if ( (LocallyControlledRiderPawn != NULL) && (!bPlayerAudioIsPlaying || bNonPlayerAudioIsPlaying) )
		{
			eventSetupAudio(TRUE, FALSE, LocallyControlledRiderPawn);
		}

		// a little hacky, but this can turn off sometimes.  make sure it's on
		if (bPlayerAudioIsPlaying && PlayerEngineMainLoop)
		{
			if (!PlayerEngineMainLoop->IsPlaying())
			{
				PlayerEngineMainLoop->Play();
			}
		}

		// Calculations for engine audio pitch/volume shifting
		FVector ActualAccel = (Velocity - LastActualVelocity) / DeltaTime;
		if (!ActualAccel.IsZero())
		{
			// add in how much accel it took to overcome gravity, so the engine "works" more going uphill
			FVector const Up(0.f, 0.f, 1.f);
			FVector const Grav = Up * GetGravityZ();
			FVector const AccelAgainstGrav = Grav.ProjectOnTo(Velocity.SafeNormal()) / -DeltaTime;
			ActualAccel += AccelAgainstGrav;
		}

		// now find the portion of the acceleration along the derricks forward vector
		FVector const DerrickFVec = Rotation.Vector();
		FLOAT ForwardAccelMag = (ActualAccel | DerrickFVec);
		FVector ForwardAccel = (ForwardAccelMag > 0.f) ? (DerrickFVec * ForwardAccelMag) : FVector(0.f);


		FLOAT const RPMFromAccel = Max(GetRangePct(EngineRPMAccelRange.X,EngineRPMAccelRange.Y, ForwardAccel.Size()), 0.f);
		// also tie some portion of RPM to velocity, as a lame simulation of rolling friction
		FLOAT const RPMFromVel = Max(GetRangePct(EngineRPMVelRange.X,EngineRPMVelRange.Y, Velocity.Size()), 0.f);
		FLOAT TotalRPM = Clamp(RPMFromVel + RPMFromAccel, 0.f, 1.f);

		// smooth it
		EngineRPM = FInterpTo(LastEngineRPM, TotalRPM, DeltaTime, EngineRPMInterpSpeed);

		if (PlayerEngineMainLoop)
		{
			PlayerEngineMainLoop->SetFloatParameter(FName(TEXT("EngineSpeed")), EngineRPM);
		}
		if (WheelActor)
		{
			WheelActor->UpdateEngineAudio(EngineRPM);
		}	

		LastActualVelocity = Velocity;
		LastEngineRPM = EngineRPM;
	}
	else
	{
		if (bPlayerAudioIsPlaying || bNonPlayerAudioIsPlaying)
		{
			eventSetupAudio(FALSE, FALSE, NULL);
		}
	}
}

UBOOL ACOG_DerrickWheelsBase::InStasis()
{
	return bHidden;
}

void ACOG_DerrickWheelsBase::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	// for characters we are not trying to animate we don't need to BlendInPhysics!
	const UBOOL bRecentlyRendered = (LastRenderTime > WorldInfo->TimeSeconds - 0.5f);

	//debugf(TEXT("DERRICK: %s %d %f"), *GetName(), bRecentlyRendered, Mesh->MaxDistanceFactor);

	if ( ( bRecentlyRendered && 
		   !bHidden && 
		   (Mesh->MaxDistanceFactor >= MinDistFactorForUpdate) ) || 
		 (DerrickBody && DerrickBody->bAlwaysUpdateWheels) ||
		 bForceUpdateWheelsNextTick )
	{
		// update jiggle anim weight
		if (JiggleAnimControl && DerrickBody)
		{
			// Calculations for engine audio pitch/volume shifting
			FVector ActualAccel = (DerrickBody->Velocity - LastVelocity) / DeltaTime;

			// now find the portion of the acceleration along the derricks forward vector
			FRotationMatrix const DerrickRotMat(Rotation);
			FVector const DerrickUVec = DerrickRotMat.GetAxis(2);
			FLOAT const ZAxisAccelMag = Abs(ActualAccel | DerrickUVec);

			FLOAT NewJigglePct = Min(ZAxisAccelMag * AccelJiggleScalar, 1.f);
			FLOAT FadedNewJigglePct = Max(JiggleAnimControl->Child2Weight - DeltaTime / JiggleFadeTime, 0.f);
			JiggleAnimControl->SetBlendTarget(Max(FadedNewJigglePct, NewJigglePct), 0.f);

			LastVelocity = DerrickBody->Velocity;
		}

		UpdateWheels(DeltaTime);
		UpdateSteering(DeltaTime);

		// continue or resume normal interpolation
		bResetInterpolation = FALSE;
		bForceUpdateWheelsNextTick = FALSE;
	}
	else
	{
		// don't interpolate when we come back, just snap on that first frame
		bResetInterpolation = TRUE;
	}
}

//** Updates engine audio cues with new volume/pitch information from the "engine". */
void ACOG_DerrickWheelsBase::UpdateEngineAudio(FLOAT EngineSpeed)
{
	if (EngineAudio_Player_Left)
	{
		EngineAudio_Player_Left->SetFloatParameter(FName(TEXT("EngineSpeed")), EngineSpeed);
	}
	if (EngineAudio_Player_Right)
	{
		EngineAudio_Player_Right->SetFloatParameter(FName(TEXT("EngineSpeed")), EngineSpeed);
	}
	if (DerrickAudio_NonPlayer)
	{
		DerrickAudio_NonPlayer->SetFloatParameter(FName(TEXT("EngineSpeed")), EngineSpeed);
	}
}

/** Calculate ref-pose transform of each wheel. */
void ACOG_DerrickWheelsBase::CacheWheelRefPoseTMs()
{
	if (Mesh && Mesh->SkeletalMesh)
	{
		for (INT Idx=0; Idx<Wheels.Num(); ++Idx)
		{
			FDerrickWheel& Wheel = Wheels(Idx);
			INT BoneIndex = Mesh->MatchRefBone(Wheel.BoneName);
			if(BoneIndex == INDEX_NONE)
			{
				debugf( TEXT("ACOG_DerrickWheelsBase::CacheWheelRefPoseTMs : Bone (%s) not found in Derrick (%s) SkeletalMesh (%s)"), *Wheel.BoneName.ToString(), *GetName(), *Mesh->GetName() );
				check(BoneIndex != INDEX_NONE);
			}

			Wheel.WheelRefPoseTM = FQuatRotationTranslationMatrix(Mesh->SkeletalMesh->RefSkeleton(BoneIndex).BonePos.Orientation, Mesh->SkeletalMesh->RefSkeleton(BoneIndex).BonePos.Position);
		}
	}
}

void ACOG_DerrickWheelsBase::UpdateWheels(FLOAT DeltaTime)
{
	if (Mesh && Mesh->SkeletalMesh)
	{
		const FMatrix ActorToWorld = Owner->LocalToWorld();
		const FVector ForwardDir = ActorToWorld.GetAxis(0);

		FVector BaseVel(0,0,0);
		if(Base)
		{
			BaseVel = Base->Velocity;
		}

		for (INT Idx=0; Idx<Wheels.Num(); ++Idx)
		{
			FDerrickWheel& Wheel = Wheels(Idx);
			if (Wheel.WheelControl)
			{
				INT BoneIndex = Mesh->MatchRefBone(Wheel.BoneName);
				if(BoneIndex == INDEX_NONE)
				{
					debugf( TEXT("ACOG_DerrickWheelsBase::UpdateWheels : Bone (%s) not found in Derrick (%s) SkeletalMesh (%s)"), *Wheel.BoneName.ToString(), *GetName(), *Mesh->GetName() );
					check(BoneIndex != INDEX_NONE);
				}

				// find wheel world position
				FVector BaseWheelPosWorld;
				FVector TraceDir;
				{
					const INT ParentIndex		= Mesh->SkeletalMesh->RefSkeleton(BoneIndex).ParentIndex;
					const FMatrix BoneRefPose	= Wheel.WheelRefPoseTM * Mesh->SpaceBases(ParentIndex);

					FRotationTranslationMatrix const RotMat(Rotation, Location);
					BaseWheelPosWorld = RotMat.TransformFVector(BoneRefPose.GetOrigin() + Wheel.WheelRayBaseOffset);
					TraceDir = -RotMat.GetAxis(2);		// down in local space, transformed to world
//					DrawDebugSphere(BaseWheelPosWorld, Wheel.WheelRadius, 10, 255, 255, 0);
				}

				FVector StartTrace, EndTrace;
				{
					FVector const MaxDisplacementVectorWorld = ActorToWorld.TransformNormal( FVector(0,0,Wheel.WheelControl->WheelMaxRenderDisplacement + Wheel.WheelRadius) );

					// ground fudge is to keep the wheel spinning if it's within this far from the ground.
					static const FLOAT GroundFudge = 64.f;
					StartTrace = BaseWheelPosWorld - TraceDir * Wheel.WheelRadius;
					EndTrace = BaseWheelPosWorld + TraceDir * (Wheel.WheelRadius + GroundFudge);

				}

				// See if its time to do another trace.
				Wheel.FramesSinceLastTrace++;
				FLOAT DesiredWheelDisplacement = 0.f;
				if ( (Wheel.FramesSinceLastTrace >= WheelTraceInterval) || bResetInterpolation )
				{
					Wheel.FramesSinceLastTrace = 0;
					// cast ray straight down
					FCheckResult Hit;
					//DrawDebugBox(StartTrace, FVector(6,6,6), 255, 0, 0);
					//DrawDebugBox(EndTrace, FVector(6,6,6), 0, 255, 0);
					// note we use the derrick body as the source actor, since it has collision and can determine blocking, etc
					Wheel.bHasContact = !GWorld->SingleLineCheck(Hit, DerrickBody, EndTrace, StartTrace, TRACE_World | TRACE_Blocking | TRACE_Volumes, FVector(0.f,0.f,0.f));

#if 0
					if(Wheel.bHasContact)
					{
						GWorld->PersistentLineBatcher->DrawLine(StartTrace, Hit.Location, FColor(0,255,0), SDPG_World);
					}
					else
					{
						GWorld->PersistentLineBatcher->DrawLine(StartTrace, EndTrace, FColor(255,0,0), SDPG_World);
					}
#endif
					// Find desired displacement
					if(Wheel.bHasContact)
					{
						DesiredWheelDisplacement = Clamp<FLOAT>(( Wheel.WheelRadius - (Hit.Location - BaseWheelPosWorld).Size() ), 0.f, Wheel.WheelControl->WheelMaxRenderDisplacement);
					}

					// Then find amount to adjust each frame to get there.
					FLOAT CurrentDelta = DesiredWheelDisplacement - Wheel.WheelControl->WheelDisplacement;
					Wheel.WheelDispPerFrameAdjust = CurrentDelta / ((FLOAT)WheelTraceInterval);
				}

				// Interpolate towards correct position
				if (bResetInterpolation)
				{
					// bash to final position, no interp
					Wheel.WheelControl->WheelDisplacement = DesiredWheelDisplacement;
					Wheel.WheelDispPerFrameAdjust = 0.f;

					// re-randomize
					Wheel.FramesSinceLastTrace = RandHelper(WheelTraceInterval - 1);
				}
				else
				{
					Wheel.WheelControl->WheelDisplacement += Wheel.WheelDispPerFrameAdjust;
				}

				if (Wheel.bHasContact)
				{
					// if contacting last time and this time, just roll enough to handle the traversed distance
					if (Wheel.bHadContact)
					{
						const FLOAT TravelledDist = (BaseVel | ForwardDir) * DeltaTime;
						const FLOAT DegreesRot = (180.f * TravelledDist) / (PI * Wheel.WheelRadius);

						Wheel.WheelRoll += DegreesRot;
						Wheel.WheelControl->WheelRoll = Wheel.WheelRoll;

						Wheel.RollVel = DegreesRot / DeltaTime;
					}
					else
					{
						// just resumed contact after a lull.  LastContactPos isn't valid.
						// we won't roll this frame, like the wheel stuck
					}

					//Wheel.LastContactPos = Hit.Location;
					Wheel.bHadContact = TRUE;
				}
				else
				{
					// wheel spins at last rollvel, decelerating towards zero
					if (Wheel.RollVel < 0.f)
					{
						Wheel.RollVel += Wheel.FreeSpinningFrictionDecel * DeltaTime;		// friction
						Wheel.RollVel = Min<FLOAT>(Wheel.RollVel, 0.f);
					}
					else
					{
						Wheel.RollVel -= Wheel.FreeSpinningFrictionDecel * DeltaTime;		// friction
						Wheel.RollVel = Max<FLOAT>(Wheel.RollVel, 0.f);
					}

					Wheel.WheelRoll += Wheel.RollVel * DeltaTime;
					Wheel.WheelControl->WheelRoll = Wheel.WheelRoll;

					Wheel.bHadContact = FALSE;
				}
			}
		}
	}
}

/** Updates steering of the front wheels.  Done here instead of in UpdateWheels because SkelControlLookAt doesn't have a native interface. */
void ACOG_DerrickWheelsBase::UpdateSteering(FLOAT DeltaTime)
{
	if (SteeringLookatControl == NULL)
	{
		// first time through setups.
		LastSteeringPivotLoc = Mesh->GetBoneLocation(SteeringPivotBoneName);
		SteeringLookatControl = Cast<USkelControlLookAt>(Mesh->FindSkelControl(SteeringLookatControlName));
		if (SteeringLookatControl == NULL)
		{
			// couldn't find steering skel control, just bail
			return;
		}
	}

	FVector const SteeringPivotLoc = Mesh->GetBoneLocation(SteeringPivotBoneName);
	FVector const SteeringPivotDisplacement = SteeringPivotLoc - LastSteeringPivotLoc;
	FLOAT const SteeringPivotDisplacementLen = SteeringPivotDisplacement.Size();
	FRotationMatrix const RotMat(Rotation);


	if ( (SteeringPivotDisplacementLen > 0.1f)	 && 			// 0.1 is an arbitrary choice for "very small"
		 (Wheels(0).bHasContact || Wheels(3).bHasContact) )		// keep steering fixed if front wheels (0 and 3) are off the ground
	{
		FVector SteeringDirNorm = SteeringPivotDisplacement / SteeringPivotDisplacementLen;

		// handle moving backward
		if ( (SteeringDirNorm | Rotation.Vector()) < 0.f )
		{
			SteeringDirNorm = -SteeringDirNorm;
		}

		FVector const IdealTargetLocation = SteeringPivotLoc + SteeringDirNorm * 1024.f;

		// move to local space, we'll do the interp there to remove effects of the derrick turning
		FVector const IdealLocalTargetLocation = RotMat.InverseTransformFVectorNoScale(IdealTargetLocation - Location);
		FRotator const IdealLocalSteerRot = IdealLocalTargetLocation.Rotation();

		FRotator LocalSteerRot = bResetInterpolation ? IdealLocalSteerRot : RInterpTo(LastLocalSteerRot, IdealLocalSteerRot, DeltaTime, SteerInterpSpeed);

		// back to world after the interp
		SteeringDirNorm = RotMat.TransformFVector(LocalSteerRot.Vector());
		SteeringLookatControl->TargetLocation = Location + (SteeringDirNorm * 1024.f);
		LastLocalSteerRot = LocalSteerRot;

//		DrawDebugBox(SteeringLookatControl.TargetLocation, vect(30,30,30), 255, 255, 0);
//		DrawDebugSphere(SteeringPivotLoc, 30, 10, 255, 255, 255);
//		`log(self@"is moving!!! new steering"@VSize(SteeringDir)@"rot is"@IdealLocalSteerRot@"last loc"@LastSteeringPivotLoc@"cur loc"@SteeringPivotLoc);
	}
	else
	{
		// don't adjust steering, just leave it where it is
//		`log(self@"is stationary, steering unchanged"@VSize(SteeringDir)@"rot is"@LastLocalSteerRot);
		FVector SteeringDirNorm = RotMat.TransformFVector(LastLocalSteerRot.Vector());
		SteeringLookatControl->TargetLocation = Location + (SteeringDirNorm * 1024.f);
	}

	LastSteeringPivotLoc = SteeringPivotLoc;
}


/************************************************************************/
/* UGearTutorialManager C++ functions                                   */
/************************************************************************/
/**
* Checks to see if there is an active tutorial, and if not it will
* look for a new tutorial to start
*/
void UGearTutorialManager::Update()
{
	// Allow all tutorial manager to update if the system is turned on
	if ( bTutorialSystemIsActive )
	{
		// To set the next active tutorial if one exists and there is no active tutorial
		UBOOL bReadyTutorialExists = FALSE;
		// Pointer to the highest priority tutorial
		UGearTutorial_Base* HighestPriorityTutorial = NULL;

		// Allow all tutorials to update
		for ( INT Idx = 0; Idx < Tutorials.Num(); Idx++ )
		{
			UGearTutorial_Base* Tutorial = Tutorials(Idx);

			if ( Tutorial )
			{
				// Update
				Tutorial->eventUpdate();

				// Mark the flag so we know if a ready tutorial exists
				if ( Tutorial->bTutorialIsReadyForActivation )
				{
					bReadyTutorialExists = TRUE;
				}

				// If the tutorial completed on this update, allow the tutorial manager
				// to clean things up.  Part of cleanup is to remove the tutorial from
				// the system so decrement the index since the array just go shifted.
				if ( Tutorial->bTutorialIsComplete )
				{
					eventOnTutorialCompleted( Tutorials(Idx) );
					Idx--;
					Tutorial = NULL;
				}

				// See if this tutorial is the highest priority tutorial
				if ( Tutorial && Tutorial->bTutorialIsReadyForActivation && (!HighestPriorityTutorial || (Tutorial->TutorialPriority > HighestPriorityTutorial->TutorialPriority)) )
				{
					HighestPriorityTutorial = Tutorial;
				}
			}
		}

		// If there is a higher priority tutorial than the currently active one, deactivate the current one and allow the
		// eventSetNextActiveTutorial() function to reselect (it will pick the highest priority tutorial)
		if ( ActiveTutorial && HighestPriorityTutorial && (ActiveTutorial->TutorialPriority < HighestPriorityTutorial->TutorialPriority) )
		{
			eventDeactivateTutorial( TRUE );
		}

		// If there is no active tutorial and there is at least 1 ready tutorial
		if ( !ActiveTutorial && bReadyTutorialExists )
		{
			// Have the system set the next one
			eventSetNextActiveTutorial();
		}
	}
}

/************************************************************************/
/* AGearPawn_RockWormBase C++ Implementation                                */
/************************************************************************/
void AGearPawn_RockWormBase::TickAuthoritative( FLOAT DeltaSeconds)
{
	UpdateTail(DeltaSeconds);
	
	 //DEBUG
	//for(INT Idx=0; Idx< ReachSpecsImBlocking.Num(); Idx++)
	//{
	//	FVector Start = ReachSpecsImBlocking(Idx)->Start->Location;
	//	FVector End = ReachSpecsImBlocking(Idx)->End->Location;
	//	DrawDebugLine(Start,End,255,0,0);
	//	if(ReachSpecsImBlocking(Idx)->BlockedBy != NULL)
	//	{
	//		DrawDebugLine((Start+End)/2.f,ReachSpecsImBlocking(Idx)->BlockedBy->Location,255,255,128);
	//	}
	//}

	//for(INT Idx=0; Idx< ReachSpecsImBlocking.Num(); Idx++)
	//{
	//	FVector Start = ReachSpecsImBlocking(Idx)->Start->Location;
	//	FVector End = ReachSpecsImBlocking(Idx)->End->Location;
	//	
	//	if(ReachSpecsImBlocking(Idx)->BlockedBy == NULL)
	//	{
	//		DrawDebugLine(Start,End,255,0,0);
	//	}
	//}

	/*	
	for(INT Idx=0; Idx< ReachSpecsImBlocking.Num(); Idx++)
	{
		ANavigationPoint* Start;
		ANavigationPoint* End;
		Start = ReachSpecsImBlocking(Idx)->Start;
		End = ReachSpecsImBlocking(Idx)->End.Nav();
		FVector StartPt = ReachSpecsImBlocking(Idx)->Start->Location + FVector(0.f,0.f,12.f);
		FVector EndPt = ReachSpecsImBlocking(Idx)->End->Location + FVector(0.f,0.f,12.f);

		if(ReachSpecsImBlocking(Idx)->BlockedBy == NULL)
		{
			DrawDebugLine(StartPt,EndPt,255,255,0);
		}
		if(End->GetReachSpecTo(Start) != NULL && End->GetReachSpecTo(Start)->BlockedBy == NULL)
		{
			DrawDebugLine(StartPt,EndPt,255,0,0);
		}
	}
	*/

	// update cover
	UpdateCoverSlots();

	Super::TickAuthoritative(DeltaSeconds);
}

void AGearPawn_RockWormBase::TickSimulated( FLOAT DeltaSeconds )
{
	UpdateTail(DeltaSeconds);
	UpdateCoverSlots();
	Super::TickSimulated(DeltaSeconds);
}

void AGearPawn_RockWormBase::AddSlotToSlotSpecs()
{
	if(Scout == NULL )
	{
		Scout = FPathBuilder::GetScout();
	}
	for(INT Idx=0;Idx<LeftCover->Slots.Num();Idx++)
	{
		if(Idx > 0)
		{
			FCoverSlot& LeftSlot = LeftCover->Slots(Idx);
			FCoverSlot& PrevLeftSlot = LeftCover->Slots(Idx-1);
			FCoverSlot& RightSlot = RightCover->Slots(LeftCover->Slots.Num() - Idx - 1);
			FCoverSlot& PrevRightSlot = RightCover->Slots(LeftCover->Slots.Num() - Idx);

			LeftSlot.SlotMarker->ForcePathTo(PrevLeftSlot.SlotMarker,Scout,Scout->GetDefaultReachSpecClass());
			PrevLeftSlot.SlotMarker->ForcePathTo(LeftSlot.SlotMarker,Scout,Scout->GetDefaultReachSpecClass());
			RightSlot.SlotMarker->ForcePathTo(PrevRightSlot.SlotMarker,Scout,Scout->GetDefaultReachSpecClass());
			PrevRightSlot.SlotMarker->ForcePathTo(RightSlot.SlotMarker,Scout,Scout->GetDefaultReachSpecClass());
		}
	}
}

/**
 * Updates the position of all the tail segments
 */
void AGearPawn_RockWormBase::UpdateTail(FLOAT DeltaTime)
{
	ARockWorm_TailSegment* PrevSegment = NULL;
	ARockWorm_TailSegment* CurSegment = TailSegmentList;

	FLOAT WorldTime = GWorld->GetTimeSeconds();

	// movement
	while(CurSegment != NULL)
	{
		FVector PrevSegLoc = FVector(0.f);
		
		if(PrevSegment == NULL)
		{
			PrevSegLoc = Mesh->GetBoneLocation(RearPivotBoneName);
		}
		else
		{			
			PrevSegLoc = PrevSegment->Mesh->GetBoneLocation(PrevSegment->RearPivotBoneName);
		}
				
		
		FVector RearPivotLoc;
		if(CurSegment->bThisSegmentIsTheTail)
		{
			RearPivotLoc = CurSegment->Mesh->GetBoneLocation(CurSegment->FrontPivotBoneName) -  (CurSegment->Mesh->GetBoneAxis(CurSegment->FrontPivotBoneName,AXIS_Y)*50.f);//Y axis cuz the bone orientation is from maya
		}
		else
		{
			RearPivotLoc = CurSegment->Mesh->GetBoneLocation(CurSegment->RearPivotBoneName);
		}
		FVector NewSegDir = (PrevSegLoc - RearPivotLoc).SafeNormal();

		CurSegment->SetRotation(NewSegDir.Rotation());
		FVector FrontPivotLoc = CurSegment->Mesh->GetBoneLocation(CurSegment->FrontPivotBoneName);
		FVector OldLoc = CurSegment->Location;
		//CurSegment->SetLocation(PrevSegLoc + (OldLoc-FrontPivotLoc));		
		// hax so we don't do touch updates (we don't care about em anyway!)
		CurSegment->bCollideActors=FALSE;
		GWorld->FarMoveActor(CurSegment,PrevSegLoc + (OldLoc-FrontPivotLoc),FALSE,TRUE,FALSE);
		CurSegment->bCollideActors=TRUE;

		CurSegment->Velocity = Velocity;		

		PrevSegment = CurSegment;
		CurSegment = CurSegment->NextSegment;
	}
}
#if 0
#define SCOPE_QUICK_TIMER(TIMERNAME) \
	static DOUBLE AverageTime##TIMERNAME = -1.0; \
	static INT NumItts##TIMERNAME = 0; \
	class TimerClassName##TIMERNAME \
	{ \
		public:\
		TimerClassName##TIMERNAME() \
		{ \
			StartTimerTime = appSeconds(); \
		} \
	\
		~TimerClassName##TIMERNAME() \
		{\
			FLOAT Duration = appSeconds() - StartTimerTime;\
			if(AverageTime##TIMERNAME < 0.f)\
			{\
				AverageTime##TIMERNAME = Duration;\
			}\
			else\
			{\
				AverageTime##TIMERNAME += (Duration - AverageTime##TIMERNAME)/++NumItts##TIMERNAME;\
			}\
			\
			debugf(TEXT("Task %s took %.2f(ms) Avg %.2f(ms)"),TEXT(#TIMERNAME),Duration*1000.f,AverageTime##TIMERNAME*1000.f);\
		}\
		DOUBLE StartTimerTime;\
	};\
	TimerClassName##TIMERNAME TIMERNAME = TimerClassName##TIMERNAME();
#else
#define SCOPE_QUICK_TIMER(blah) {}
#endif

/**
 * updates the tail after the rockworm has moved a significant amount.  When an update is performed the back two cover slots are moved to the front,
 * and fully updated (e.g. re-linked into the path network). and then blocking tests are performed on all the tail segments to correctly mark
 * specs as blocked during the move.
 */
void AGearPawn_RockWormBase::UpdateCoverSlots()
{
	if(TailSegmentList == NULL)
	{
		return;
	}

	INT NumSlots = (UCONST_NUM_TAILSEGMENTS/UCONST_COVER_TO_SEGMENTS_RATIO) +1;


	if(TailSegmentList == NULL || LastTailSegment == NULL || LeftCover == NULL || LeftCover->Slots.Num() != NumSlots || RightCover == NULL || RightCover->Slots.Num() != NumSlots)
	{
		return;
	}

	// make sure all the slots have been filled
	for(INT Idx=0;Idx<NumSlots;Idx++)
	{
		if(LeftCover->Slots(Idx).SlotMarker == NULL || RightCover->Slots(Idx).SlotMarker == NULL)
		{
			return;
		}
	}
	
	// if we have moved sufficiently, pull the last two slots up to the front of the worm
	if((TailSegmentList->Location - LastCoverUpdatePos).SizeSquared() > (TailSegmentList->AttachOffset*UCONST_COVER_TO_SEGMENTS_RATIO  * TailSegmentList->AttachOffset * UCONST_COVER_TO_SEGMENTS_RATIO))
	{
		SCOPE_QUICK_TIMER(CoverSlotUpdate);

		
		LastCoverUpdatePos = TailSegmentList->Location;

		// left - move last one in list to the first slot, and update its position
		FCoverSlot TempSlot = LeftCover->Slots(LeftCover->Slots.Num() -1);
		LeftCover->Slots.Remove(LeftCover->Slots.Num() -1,1);
		LeftCover->Slots.InsertItem(TempSlot,0);

		// update it's location based on the first segment
		SetCoverSlotLocFromOffset(LeftCover,Location,0,FVector(CoverToSegmentOffset.X,-CoverToSegmentOffset.Y,CoverToSegmentOffset.Z),TailSegmentList);

		// right - move first one in list to the last slot, and update its position
		TempSlot = RightCover->Slots(0);
		RightCover->Slots.Remove(0,1);
		RightCover->Slots.AddItem(TempSlot);

		SetCoverSlotLocFromOffset(RightCover,Location,RightCover->Slots.Num()-1,CoverToSegmentOffset,TailSegmentList);

		ARockWorm_TailSegment* CurSegment = TailSegmentList;
		INT Idx = 0;
		for(INT i=0;i<UCONST_NUM_TAILSEGMENTS;i++)
		{
			if(CurSegment == NULL)
			{
				break;
			}

			if(i%UCONST_COVER_TO_SEGMENTS_RATIO==0)
			{
				// update mantle indices
				RightCover->Slots(Idx).MantleTarget.SlotIdx = NumSlots - Idx-1;
				LeftCover->Slots(Idx).MantleTarget.SlotIdx = NumSlots - Idx-1;

				// update indices since we just shifted everything around
				// right (all decremented, except slot0 which is moved to last)
				INT OldIndex = 0;
				
				if(Idx != 0)
				{
					OldIndex = (Idx+1 < RightCover->Slots.Num()) ? Idx+1 : 0;								
				}
				// if we're at the end, it means we used to be at slot0
				else
				{
					OldIndex = RightCover->Slots.Num()-1;
				}
				RightCover->eventSlotIndexUpdated(Idx,OldIndex);


				//left (all incremented, except last slot which is moved to slot0)
				if(Idx != LeftCover->Slots.Num()-1)
				{
					OldIndex = (Idx-1 >= 0) ? Idx-1 : LeftCover->Slots.Num()-1;
				}
				// if we're at the begining it means the old slot was at the end
				else
				{
					OldIndex = 0;
				}
				LeftCover->eventSlotIndexUpdated(Idx,OldIndex);




				// ensure all slots stay the proper distance from the tail
				AdjustCoverSlotPositionAlongTail(RightCover,Location,CurSegment,NumSlots - Idx-1);
				AdjustCoverSlotPositionAlongTail(LeftCover,Location,CurSegment,Idx);
				if(Scout == NULL)
				{
					Scout = FPathBuilder::GetScout();
				}
				RightCover->UpdateCoverSlot(NumSlots-Idx-1,TRUE,Scout);
				LeftCover->UpdateCoverSlot(Idx,TRUE,Scout);
				Idx++;

			}
			
			CurSegment = CurSegment->NextSegment;
		}
	
		LeftCover->SetLocation(Location);
		RightCover->SetLocation(Location);

		AddSlotToSlotSpecs();
		UpdateBlockedReachSpecs();		
	}


	
	//DEBUG
	//for(INT i=0;i<UCONST_NUM_TAILSEGMENTS;i++)
	//{
	//	if(i+1 < RightCover->Slots.Num() )
	//	{
	//		DrawDebugLine(RightCover->GetSlotLocation(i),RightCover->GetSlotLocation(i+1),255,255,0);
	//	}
	//	if(i+1 < LeftCover->Slots.Num() )
	//	{
	//		DrawDebugLine(LeftCover->GetSlotLocation(i),LeftCover->GetSlotLocation(i+1),128,255,0);
	//	}		
	//	DrawDebugCoordinateSystem(RightCover->GetSlotLocation(i),RightCover->GetSlotRotation(i),16.f);
	//	DrawDebugCoordinateSystem(LeftCover->GetSlotLocation(i),LeftCover->GetSlotRotation(i),16.f);
	//}
		

}
/**
 * this function nudges cover slots around as the tail slides to make sure they stay roughly the right distance from the tail segments
 **/
void AGearPawn_RockWormBase::AdjustCoverSlotPositionAlongTail(ACoverLink* Link, const FVector& LinkLocation, ARockWorm_TailSegment* TailSegment, INT SlotIdx)
 {
	check(Link != NULL);
	check(SlotIdx >= 0);
	check(SlotIdx < Link->Slots.Num());
	check(TailSegment != NULL);

	FVector SlotLocation = Link->GetSlotLocation(SlotIdx);
	

	FVector ClosestTailPt;	
	FVector TailNormal;
	FVector TailLineSegmentStart = FVector(0.f);
	FVector TailLineSegmentEnd = FVector(0.f);
	
	if(TailSegment != NULL)
	{	
		TailLineSegmentStart = TailSegment->Location;
		TailLineSegmentEnd = TailSegment->Location;

		if(TailSegment->PrevSegment != NULL)
		{
			TailLineSegmentStart = TailSegment->PrevSegment->Location;
		}
		if(TailSegment->NextSegment != NULL)
		{
			TailLineSegmentEnd = TailSegment->NextSegment->Location;
		}
	}
	TailLineSegmentStart.Z += CoverToSegmentOffset.Z;
	TailLineSegmentEnd.Z += CoverToSegmentOffset.Z;

	FLOAT DistToTail = PointDistToLine(SlotLocation,TailLineSegmentEnd-TailLineSegmentStart,TailLineSegmentStart,ClosestTailPt);
	//DrawDebugLine(TailLineSegmentStart,TailLineSegmentEnd,255,255,255);
	
	
	
	TailNormal = (TailLineSegmentEnd - TailLineSegmentStart).SafeNormal() ^ FVector(0.f,0.f,1.f);
	if( (-TailNormal | (SlotLocation - ClosestTailPt)) > (TailNormal | (SlotLocation - ClosestTailPt)) )
	{
		TailNormal = -TailNormal;
	}
	//DrawDebugLine(SlotLocation,TailSegment->Location,255,255,0);
	//DrawDebugCoordinateSystem(ClosestTailPt,TailNormal.Rotation(),60.f);
	
	FLOAT PctDesiredRange = ((DistToTail*DistToTail) / (CoverToSegmentOffset.Y * CoverToSegmentOffset.Y));

	if(Abs(1.0f - PctDesiredRange) > 0.25f)
	{
		FVector NewSlotLocation = ClosestTailPt + (TailNormal * CoverToSegmentOffset.Y);
		FRotator NewSlotRotation = Link->GetSlotRotation(SlotIdx);
		// only update rotation if we don't have an owner right now
		if(Link->Slots(SlotIdx).SlotOwner != NULL)
		{
			NewSlotRotation = (-TailNormal).Rotation();
		}

		Link->Slots(SlotIdx).SlotMarker->SetLocation(NewSlotLocation);
		Link->Slots(SlotIdx).SlotMarker->SetRotation(NewSlotRotation);

		Link->Slots(SlotIdx).LocationOffset = FRotationMatrix(Link->Rotation).InverseTransformFVector(NewSlotLocation - LinkLocation);
		Link->Slots(SlotIdx).RotationOffset = NewSlotRotation - Link->Rotation;
	}

}

FVector AGearPawn_RockWormBase::GetParentAttachPos(AActor* Parent, FName ParentBoneName/*=NAME_None*/)
{
	APawn* ParentPawn = NULL;
	if(Parent == NULL)
	{
		return FVector(0.f);
	}

	if(ParentBoneName == NAME_None)
	{
		return Parent->Location;
	}

	ParentPawn = Cast<APawn>(Parent);
	if(ParentPawn != NULL)
	{
		return ParentPawn->Mesh->GetBoneLocation(ParentBoneName);
	}
	else
	{
		return Parent->Location;
	}

}
void AGearPawn_RockWormBase::SetCoverSlotLocFromOffset(ACoverLink* Link, const FVector& LinkLocation, INT SlotIdx, const FVector& Offset, AActor* Parent, FName ParentBoneName/* = NAME_None*/)
{

	FVector ParentAttachPos = GetParentAttachPos(Parent,ParentBoneName);
	FVector NewLocation = ParentAttachPos + FRotationMatrix(Parent->Rotation).TransformFVector(Offset);
	ParentAttachPos.Z = NewLocation.Z;
	FRotator NewRotation = (ParentAttachPos - NewLocation).Rotation();

	ACoverSlotMarker* Marker = Link->Slots(SlotIdx).SlotMarker;

	if(Marker != NULL)
	{
		Marker->SetLocation(NewLocation);
		Marker->SetRotation(NewRotation);
	}

	Link->Slots(SlotIdx).LocationOffset = FRotationMatrix(Link->Rotation).InverseTransformFVector(NewLocation - LinkLocation);
	Link->Slots(SlotIdx).RotationOffset = NewRotation - Link->Rotation;

}

UBOOL SpecBlockedByActor(UReachSpec* Spec, AActor* Actor)
{
	FCheckResult Hit(1.f);
	FVector Start = Spec->Start->Location;
	FVector End = Spec->End->Location;
	// first do a quick check to see if the current segment is even close to the ray
	FLOAT DistFromSpec = PointDistToLine(Actor->Location,(End-Start),Start);
	if(DistFromSpec < Spec->CollisionRadius*1.25f)
	{
		if(!Actor->ActorLineCheck(Hit,End,Start,FVector(Spec->CollisionRadius,Spec->CollisionRadius,Spec->CollisionHeight*0.5f),TRACE_Pawns | TRACE_Others | TRACE_Blocking |TRACE_AllBlocking))
		{
			return TRUE;
		}		
	}

	return FALSE;
}
void AGearPawn_RockWormBase::UpdateBlockedReachSpecs()
{
	SCOPE_QUICK_TIMER(UpdateBlockedReachSpecs)

	// pre-block all edges we cross so guys will path around us, and through cover mantle specs instead of running into us!

	// first clear out reachspecs we're already marked as blocking
	for(INT Idx=0;Idx<ReachSpecsImBlocking.Num();Idx++)
	{
		UReachSpec* BlockingReachSpec = ReachSpecsImBlocking(Idx);
		if(BlockingReachSpec != NULL && BlockingReachSpec->BlockedBy != NULL && (BlockingReachSpec->BlockedBy == this || BlockingReachSpec->BlockedBy->GetClass() == TailSegmentClass))
		{
			BlockingReachSpec->BlockedBy = NULL;
		}
	}
	ReachSpecsImBlocking.Empty(ReachSpecsImBlocking.Num()/2);

	// now figure out what other reachspecs I block

	TArray<FNavigationOctreeObject*> NavObjects;  
	GWorld->NavigationOctree->RadiusCheck(Location, MAXPATHDIST, NavObjects);

	UReachSpec* Spec = NULL;
	for(INT Idx=0; Idx < NavObjects.Num(); Idx++)
	{
		Spec = NavObjects(Idx)->GetOwner<UReachSpec>();

		if(Spec != NULL && Spec->Start != NULL && Spec->End.Nav() != NULL && !Spec->IsA(UMantleReachSpec::StaticClass()) && Spec->BlockedBy == NULL)
		{
			// check against every tail segment
			ARockWorm_TailSegment* CurSeg = TailSegmentList;
			while(CurSeg != NULL)
			{
				if(SpecBlockedByActor(Spec,CurSeg))
				{
					ReachSpecsImBlocking.AddItem(Spec);
					Spec->BlockedBy = CurSeg;

					// add the one in the other direction
					UReachSpec* OtherDirSpec = Spec->End.Nav()->GetReachSpecTo(Spec->Start);
					if(OtherDirSpec != NULL)
					{
						ReachSpecsImBlocking.AddItem(Spec);
						Spec->BlockedBy = CurSeg;

						// add the one in the other direction
						UReachSpec* OtherDirSpec = Spec->End.Nav()->GetReachSpecTo(Spec->Start);
						if(OtherDirSpec != NULL)
						{
							ReachSpecsImBlocking.AddItem(OtherDirSpec);
							OtherDirSpec->BlockedBy = CurSeg;
						}
					}		

				}
				CurSeg = CurSeg->NextSegment;
			}

			//// check against me
			if(SpecBlockedByActor(Spec,this))
			{
				ReachSpecsImBlocking.AddItem(Spec);
				Spec->BlockedBy = this;

				// add the one in the other direction
				UReachSpec* OtherDirSpec = Spec->End.Nav()->GetReachSpecTo(Spec->Start);
				if(OtherDirSpec != NULL)
				{
					ReachSpecsImBlocking.AddItem(OtherDirSpec);
					OtherDirSpec->BlockedBy = this;
				}
			}
		}
	}

}

UBOOL AGearPawn_RockWormBase::IsActorBlockedByTail( AActor* Actor )
{
	if(TailSegmentList != NULL)
	{
		if( (Location - Actor->Location).SizeSquared() < MAXPATHDISTSQ )
		{
			FVector TailToMeVec = (Location - TailSegmentList->NextSegment->Location).SafeNormal2D();
			FLOAT Dot = (TailToMeVec | (Actor->Location - Location).SafeNormal2D());
			if(Dot < MinDotReachable)
			{
				return TRUE;
			}
		}

		// check against the tail to make sure we're not going to intersect it
		// start on second seg
		ARockWorm_TailSegment* CurSeg = TailSegmentList->NextSegment;
		FCheckResult Hit(1.0f);
		while(CurSeg != NULL)
		{

			if(!CurSeg->IsOverlapping(this) && !CurSeg->ActorLineCheck(Hit,Location,Actor->Location,FVector(0.f),TRACE_StopAtAnyHit|TRACE_AllBlocking|TRACE_Pawns))
			{
				return TRUE;
			}
			CurSeg = CurSeg->NextSegment;
		}
	}

	return FALSE;
}

int AGearPawn_RockWormBase::actorReachable(AActor *Other, UBOOL bKnowVisible, UBOOL bNoAnchorCheck)
{

	if(IsActorBlockedByTail(Other))
	{
		return FALSE;
	}

	return Super::actorReachable(Other,bKnowVisible,bNoAnchorCheck);
}

UBOOL AGearPawn_RockWormBase::IsValidAnchor( ANavigationPoint* AnchorCandidate )
{
	if(IsActorBlockedByTail(AnchorCandidate))
	{
		return FALSE;
	}

	return Super::IsValidAnchor(AnchorCandidate);
}


INT AngleDist(INT AngOne, INT AngTwo)
{
	INT Dist = Abs<INT>(AngOne - AngTwo);
	if( Dist > 32768 )
	{
		Dist = 65535 - Dist;
	}
	return Abs<INT>(Dist);
}
/** performs a fixed turn, without going through the clamp angle (e.g. rotate from A->B but you can't rotate past -180degrees, so go the long way) */
int fixedTurnNotThroughClamp(int current, int desired, int deltaRate, int ClampOrigin, int Clamp,const FVector& Location)
{
	if (deltaRate == 0)
		return (current & 65535);

	int result = current & 65535;
	current = result;
	desired = desired & 65535;

	// first make sure desired isn't inside the clamp

	INT DistFromClampOrigin = AngleDist(desired,ClampOrigin);
	
	if(DistFromClampOrigin < Clamp)
	{
		// find the closest clamp
		INT Clamp1 = (ClampOrigin+Clamp);
		if(Clamp1 > 65535)
		{
			Clamp1 -= 65535;
		}

		INT Clamp2 = (ClampOrigin-Clamp);
		if(Clamp2 < 0)
		{
			Clamp2 = 65535 - Clamp2;
		}

		if(AngleDist(Clamp1,desired) < AngleDist(Clamp2,desired))
		{
			desired = Clamp1;
		}
		else
		{
			desired = Clamp2;
		}
	}


	INT DistThroughClamp = AngleDist(current,ClampOrigin) + AngleDist(desired,ClampOrigin);

	if (current > desired)
	{
		if (current - desired < 32768 || DistThroughClamp < 32768)
			result -= Min((current - desired), Abs(deltaRate));
		else
			result += Min((desired + 65536 - current), Abs(deltaRate));
	}
	else
	{
		if (desired - current < 32768 || DistThroughClamp < 32768)
			result += Min((desired - current), Abs(deltaRate));
		else
			result -= Min((current + 65536 - desired), Abs(deltaRate));
	}
	return (result & 65535);
}


void AGearPawn_RockWormBase::physicsRotation(FLOAT deltaTime, FVector OldVelocity)
{
	if ( !Controller )
	{
		return;
	}

	FRotator ClampOrigin;
	if(TailSegmentList && TailSegmentList->NextSegment && TailSegmentList->NextSegment->NextSegment)
	{
		ClampOrigin = ((TailSegmentList->NextSegment->NextSegment->Location - Location) * FVector(1.f,1.f,0.f)).Rotation().GetNormalized();
	}
	else
	{
		ClampOrigin = (Rotation + FRotator(0,32767,0)).GetNormalized();
		
	}


	// always call SetRotationRate() as it may change our DesiredRotation
	FRotator deltaRot = Controller->SetRotationRate(deltaTime);

	// Accumulate a desired new rotation.
	FRotator NewRotation = Rotation;	

	//YAW
	if( DesiredRotation.Yaw != NewRotation.Yaw )
	{
		NewRotation.Yaw = fixedTurnNotThroughClamp(NewRotation.Yaw, DesiredRotation.Yaw, deltaRot.Yaw,ClampOrigin.Yaw,MinAngleToTail,Location);
	}


	// Set the new rotation.
	// fixedTurn() returns denormalized results so we must convert Rotation to prevent negative values in Rotation from causing unnecessary MoveActor() calls
	if( NewRotation != Rotation.GetDenormalized() )
	{
		FCheckResult Hit(1.f);
		GWorld->MoveActor( this, FVector(0,0,0), NewRotation, 0, Hit );
	}
}

UBOOL AGearPawn_RockWormBase::ReachedDesiredRotation()
{
	// we need to return TRUE when we hit our rotation clamp (so it doesn't hang forever trying to rotate into the clamp)
	FRotator ClampOrigin;
	if(TailSegmentList && TailSegmentList->NextSegment)
	{
		ClampOrigin = ((TailSegmentList->NextSegment->Location - Location) * FVector(1.f,1.f,0.f)).Rotation();
	}
	else
	{
		ClampOrigin = (Rotation + FRotator(0,32768,0)).GetNormalized();
	}
	
	INT desired = DesiredRotation.Yaw;
	INT DistFromClampOrigin = AngleDist(desired,ClampOrigin.Yaw);

	if(DistFromClampOrigin < MinAngleToTail)
	{
		// find the closest clamp
		INT Clamp1 = (ClampOrigin.Yaw+MinAngleToTail);
		if(Clamp1 > 65535)
		{
			Clamp1 -= 65535;
		}

		INT Clamp2 = (ClampOrigin.Yaw-MinAngleToTail);
		if(Clamp2 < 0)
		{
			Clamp2 = 65535 - Clamp2;
		}
		if(AngleDist(Clamp1,desired) < AngleDist(Clamp2,desired))
		{
			desired = Clamp1;
		}
		else
		{
			desired = Clamp2;
		}
	}

	// return TRUE if we're trying to rotate inside the clamp, and we've gone as far as we can
	return (Super::ReachedDesiredRotation() || AngleDist(Rotation.Yaw,desired) < AllowedYawError);		 

}

UBOOL AGearPawn_RockWormBase::IgnoreBlockingBy( const AActor *Other) const
{
	const ARockWorm_TailSegment* Seg = (Other != NULL) ? ConstCast<ARockWorm_TailSegment>(Other) : NULL;
	// if it's one of my tail segments
	if(Seg != NULL /*&& Seg->WormOwner == this*/)
	{
		return TRUE;
	}

	return Super::IgnoreBlockingBy(Other);
}

UBOOL ARockWorm_TailSegment::ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags)
{
	AGearPawn_RockWormBase* Worm = Cast<AGearPawn_RockWormBase>(SourceActor);
	if(SourceActor != NULL && SourceActor->IsA(AGearPawn_RockWormBase::StaticClass()) /*&& WormOwner == (SourceActor)*/)
	{
		return FALSE;
	}

	return Super::ShouldTrace(Primitive,SourceActor,TraceFlags);
}

void ARockWorm_TailSegment::NotifyBump(AActor *Other, UPrimitiveComponent* OtherComp, const FVector &HitNormal)
{
	AGearPawn* GP = NULL;

	if(Other != NULL)
	{
		GP = Cast<AGearPawn>(Other);

		// if they are going somewhere that's on the other side of us, force a repath

		if(GP->MyGearAI != NULL && GP->MyGearAI->MoveTarget != NULL)
		{
			FVector AIMove = (GP->MyGearAI->MoveTarget->Location - GP->Location).SafeNormal();
			FVector TaiLToAI = (GP->Location - Location).SafeNormal();
			if((AIMove | TaiLToAI) < 0.f)
			{
				GP->MyGearAI->eventForcePauseAndRepath(this);
			}
		}
	}
	Super::NotifyBump(Other,OtherComp,HitNormal);
}


void UGearDecalInfo::PostLoad()
{
	Super::PostLoad();
}



void UGearBloodInfo::PostLoad()
{
	Super::PostLoad();
}


void UGearPhysicalMaterialProperty::PostLoad()
{
	Super::PostLoad();
	RefreshTypeData();
}


void UGearPhysicalMaterialProperty::PostEditChange( class FEditPropertyChain& PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);
	RefreshTypeData();
}

/**
* Make sure all decal struct defaults have valid values. 
* Needed in order to work-around serialization issues with struct defaults.
*
* @param DecalDataEntries - decal data struct array of entries to fixup
**/
void UGearPhysicalMaterialProperty::FixupDecalDataStructDefaults( TArray<FDecalData>& DecalDataEntries )
{
	FDecalData DefaultDecalData(EC_NativeConstructor);
	UBOOL bOldVer = GetLinker() && GetLinker()->Ver() < VER_DECAL_PHYS_MATERIAL_ENTRY_FIXUP;
	FVector2D OldDefaults(89.f,95.f);

	for( INT DataIdx=0; DataIdx < DecalDataEntries.Num(); DataIdx++ )
	{
		FDecalData& DecalDataEntry = DecalDataEntries(DataIdx);
		// reset to new defaults if ~0
		// or if trying to override an old set of defaults
		if( DecalDataEntry.BlendRange.IsNearlyZero() ||
			(bOldVer && DecalDataEntry.BlendRange.Equals(OldDefaults)) )
		{
			DecalDataEntry.BlendRange = DefaultDecalData.BlendRange;
		}
	}
}

/** Enforces design that the "type" members match the enum/array position, so that content guys can see what they are editing. */
void UGearPhysicalMaterialProperty::RefreshTypeData()
{
	// we have a number of issues we need to solve here.
	// 0) someone has accidentally deleted an array entry and didn't notice it
	// 1) we have added new types which need to be added to the array
	// 2) we have a version based change where we have don't something non trivial (i.e. 1 is trivial)
	//
	// we must always look for missing entries
	// the others cases we can do based off version of the class 
	
	//// FXInfo


	// make certain that we have all of the entries we should have.  (this takes care of the case of adding a new enum)
	while( FXInfoBallistic.Num() < ITB_MAX )
	{
		FImpactFXBallistic NewEntry;
		appMemzero( &NewEntry, sizeof(NewEntry) );
		NewEntry.Type = FXInfoBallistic.Num();
		FXInfoBallistic.AddItem( NewEntry );
		MarkPackageDirty( TRUE );
		//warnf( TEXT("ADDED %s %d"), *GetFullName(), FXInfo.Num() );
	}

	// at this point we now have all of the "correctly added" entries so anything missing needs to be replaced
	for( INT Idx = 0; Idx < FXInfoBallistic.Num() && Idx < ITB_MAX ; ++Idx )
	{
		const FImpactFXBallistic& Entry = FXInfoBallistic( Idx );
		if( Entry.Type != Idx )
		{
			FImpactFXBallistic NewEntry;
			appMemzero( &NewEntry, sizeof(NewEntry) );
			NewEntry.Type = Idx;

			FXInfoBallistic.InsertItem( NewEntry, Idx );
			MarkPackageDirty( TRUE );
			//warnf( TEXT("INSERT %s %d"), *GetFullName(), Idx );
		}
	}
	// fixup struct defaults
	for( INT Idx = 0; Idx < FXInfoBallistic.Num(); ++Idx )
	{	
		FImpactFXBallistic& Entry = FXInfoBallistic(Idx);
		FixupDecalDataStructDefaults( Entry.DecalData );
		FixupDecalDataStructDefaults( Entry.DecalData_AR );
		FixupDecalDataStructDefaults( Entry.DecalData_NoGore );
		FixupDecalDataStructDefaults( Entry.DecalData_NoGore_AR );
	}
	FixupDecalDataStructDefaults( DefaultFXInfoBallistic.DecalData );
	FixupDecalDataStructDefaults( DefaultFXInfoBallistic.DecalData_AR );
	FixupDecalDataStructDefaults( DefaultFXInfoBallistic.DecalData_NoGore );
	FixupDecalDataStructDefaults( DefaultFXInfoBallistic.DecalData_NoGore_AR );


	// make certain that we have all of the entries we should have.  (this takes care of the case of adding a new enum)
	while( FXInfoExplosion.Num() < ITE_MAX )
	{
		FImpactFXExplosion NewEntry;
		appMemzero( &NewEntry, sizeof(NewEntry) );
		NewEntry.Type = FXInfoExplosion.Num();
		FXInfoExplosion.AddItem( NewEntry );
		MarkPackageDirty( TRUE );
		//warnf( TEXT("ADDED %s %d"), *GetFullName(), FXInfo.Num() );
	}

	// at this point we now have all of the "correctly added" entries so anything missing needs to be replaced
	for( INT Idx = 0; Idx < FXInfoExplosion.Num() && Idx < ITE_MAX ; ++Idx )
	{
		const FImpactFXExplosion& Entry = FXInfoExplosion( Idx );
		if( Entry.Type != Idx )
		{
			FImpactFXExplosion NewEntry;
			appMemzero( &NewEntry, sizeof(NewEntry) );
			NewEntry.Type = Idx;

			FXInfoExplosion.InsertItem( NewEntry, Idx );
			MarkPackageDirty( TRUE );
			//warnf( TEXT("INSERT %s %d"), *GetFullName(), Idx );
		}
	}
	// fixup struct defaults
	for( INT Idx = 0; Idx < FXInfoExplosion.Num(); ++Idx )
	{	
		FImpactFXExplosion& Entry = FXInfoExplosion(Idx);
		FixupDecalDataStructDefaults( Entry.DecalData );
		FixupDecalDataStructDefaults( Entry.DecalData_AR );
		FixupDecalDataStructDefaults( Entry.DecalData_NoGore );
		FixupDecalDataStructDefaults( Entry.DecalData_NoGore_AR );
	}
	FixupDecalDataStructDefaults( DefaultFXInfoExplosion.DecalData );
	FixupDecalDataStructDefaults( DefaultFXInfoExplosion.DecalData_AR );
	FixupDecalDataStructDefaults( DefaultFXInfoExplosion.DecalData_NoGore );
	FixupDecalDataStructDefaults( DefaultFXInfoExplosion.DecalData_NoGore_AR );

	// can do specific version remappings now



	//// FootStepInfo

	// make certain that we have all of the entries we should have.  (this takes care of the case of adding a new enum)
	while( FootStepInfo.Num() < CFST_MAX )
	{
		FFootStepsDatum NewEntry;
		appMemzero( &NewEntry, sizeof(NewEntry) );
		NewEntry.Type = FootStepInfo.Num();
		FootStepInfo.AddItem( NewEntry );
		MarkPackageDirty( TRUE );
		//warnf( TEXT("ADDED %s %d"), *GetFullName(), FootStepInfo.Num() );
	}

	// at this point we now have all of the "correctly added" entries so anything missing needs to be replaced
	for( INT Idx = 0; Idx < FootStepInfo.Num() && Idx < CFST_MAX ; ++Idx )
	{
		const FFootStepsDatum& Entry = FootStepInfo( Idx );
		if( Entry.Type != Idx )
		{
			FFootStepsDatum NewEntry;
			appMemzero( &NewEntry, sizeof(NewEntry) );
			NewEntry.Type = Idx;

			FootStepInfo.InsertItem( NewEntry, Idx );
			MarkPackageDirty( TRUE );
			//warnf( TEXT("INSERT %s %d"), *GetFullName(), Idx );
		}
	}

	// can do specific version remappings now


	//// DecalInfo
	if( DecalInfo != NULL )
	{
		// make certain that we have all of the entries we should have.  (this takes care of the case of adding a new enum)
		while( DecalInfo->DecalInfo.Num() < EGearDecalType_MAX )
		{
			FGearDecalDatum NewEntry;
			appMemzero( &NewEntry, sizeof(NewEntry) );
			NewEntry.Type = DecalInfo->DecalInfo.Num();
			DecalInfo->DecalInfo.AddItem( NewEntry );
			MarkPackageDirty( TRUE );
			//warnf( TEXT("ADDED %s %d"), *GetFullName(), FXInfo.Num() );
		}

		// at this point we now have all of the "correctly added" entries so anything missing needs to be replaced
		for( INT Idx = 0; Idx < DecalInfo->DecalInfo.Num() && Idx < EGearDecalType_MAX ; ++Idx )
		{
			const FGearDecalDatum& Entry = DecalInfo->DecalInfo( Idx );
			if( Entry.Type != Idx )
			{
				FGearDecalDatum NewEntry;
				appMemzero( &NewEntry, sizeof(NewEntry) );
				NewEntry.Type = Idx;

				DecalInfo->DecalInfo.InsertItem( NewEntry, Idx );
				MarkPackageDirty( TRUE );
				//warnf( TEXT("INSERT %s %d"), *GetFullName(), Idx );
			}			
		}

		// fixup struct defaults
		for( INT Idx = 0; Idx < DecalInfo->DecalInfo.Num(); ++Idx )
		{	
			FGearDecalDatum& Entry = DecalInfo->DecalInfo( Idx );
			FixupDecalDataStructDefaults( Entry.DecalData );			
		}

		// can do specific version remappings now
	}



}

//////////////////////////////////////////////////////////////////////////
// Volume_Nemacyst
UBOOL AVolume_Nemacyst::FormationContainsPoint(FVector& Point)
{
	if(ContainsPoint(Point))
	{
		//DrawDebugBox(Point,FVector(10.f),0,255,0,TRUE);		
		return TRUE;
	}

	for(INT Idx=0;Idx<Neighbors.Num();Idx++)
	{
		if(Neighbors(Idx)->ContainsPoint(Point))
		{
			//DrawDebugBox(Point,FVector(10.f),0,255,0,TRUE);		
			return TRUE;
		}
	}
	//DrawDebugBox(Point,FVector(10.f),255,0,0,TRUE);		
	return FALSE;
}


void AVolume_Nemacyst::FindNeighbors()
{
	
	TArray<UPrimitiveComponent*> TouchingPrimitives;
	GWorld->Hash->GetIntersectingPrimitives(Brush->Bounds.GetBox(), TouchingPrimitives);

/*
	FMemMark Mark(GMem);
	FCheckResult* FirstHit = GWorld->Hash ? GWorld->Hash->ActorEncroachmentCheck( GMem, this, Location, Rotation, TRACE_Volumes ) : NULL;	
*/
	AVolume_Nemacyst* Vol = NULL;
	for( INT Idx =0; Idx<TouchingPrimitives.Num();Idx++ )
	{
		if(TouchingPrimitives(Idx)->GetOwner() == NULL)
		{
			continue;
		}

		Vol = Cast<AVolume_Nemacyst>(TouchingPrimitives(Idx)->GetOwner());
		if(	Vol && Vol !=this )
		{
			//debugf(TEXT("%s adding neighbor %s"),*GetName(),*Vol->GetName());
			Neighbors.AddItem(Vol);			
		}						
		
	}
}
//////////////////////////////////////////////////////////////////////////
// GearPawn_FlyingSecurityBot
void AGearPawn_SecurityBotFlyingBase::CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant)
{
	Super::CalcVelocity(AccelDir,DeltaTime,MaxSpeed,Friction,bFluid,bBrake,bBuoyant);
	if(Physics != PHYS_Flying)
	{
		return;
	}

	if(MaxWobbleDist > 0.f && CurrentAccumulatedWobbleOffset.Size() > MaxWobbleDist)
	{
		CurrentWobble = -CurrentWobble;
		CurrentAccumulatedWobbleOffset *= 1.5f;
		MaxWobbleDist = -1.f;
	}

	CurrentAccumulatedWobbleOffset += CurrentWobble * DeltaTime;

	FLOAT CurDot = (CurrentAccumulatedWobbleOffset.SafeNormal() | CurrentWobble.SafeNormal());
	if(LastWobbleDot < 0.f && CurDot > 0.f || CurrentWobble.IsNearlyZero())
	{
		//debugf(TEXT("----------------NEW DIR!----------- curdot %.2f lastdot %.2f curwobble %.2f"), CurDot, LastWobbleDot,CurrentWobble.Size());
		CurrentWobble = VRand() * RandRange(WobbleMagMin,WobbleMagMax);		
		CurrentAccumulatedWobbleOffset = FVector(0.f);
		MaxWobbleDist = Cast<AGearPawn_SecurityBotFlyingBase>(GetClass()->GetDefaultObject())->MaxWobbleDist;
	}
	


	//DrawDebugLine(Location,Location + CurrentWobble,255,0,0);
	//DrawDebugLine(Location,Location-CurrentAccumulatedWobbleOffset,255,255,0);
	//debugf(TEXT("OFFSET SIZE:%.2f DOT:%.2f LASTDOT:%.2f"),CurrentAccumulatedWobbleOffset.Size(),CurDot,LastWobbleDot);
	Velocity.Z += sin(GWorld->GetTimeSeconds()*2.0f) * RandRange(WobbleMagMin,WobbleMagMax) * DeltaTime;
	Velocity += CurrentWobble * DeltaTime;
	LastWobbleDot = CurDot;


}

FVector AGearPawn_SecurityBotFlyingBase::GetPhysicalFireStartLoc( FVector FireOffset )
{
	FVector loc;

	if(Mesh == NULL)
	{
		return FVector(0.f);
	}

	Mesh->GetSocketWorldLocationAndRotation(PhysicalFireLocBoneName,loc,NULL);

	return loc;
}


UBOOL AGearPawnBlockingVolume::IgnoreBlockingBy(const AActor* Other) const
{
	// this is lame but trying to const-correct IsPlayerOwned() spiralled into a huge change
	APawn* P = const_cast<AActor*>(Other)->GetAPawn();
	if (P != NULL)
	{
		return (P->IsPlayerOwned() ? !bBlockPlayers : !bBlockMonsters);
	}
	else
	{
		return Super::IgnoreBlockingBy(Other);
	}
}

/** Only block grenades */
UBOOL AGrenadeBlockingVolume::IgnoreBlockingBy(const AActor* Other) const
{
	const AGearProjectile* GearProj = ConstCast<AGearProjectile>(Other);
	if(GearProj && GearProj->bStoppedByGrenadeBlockingVolume)
	{
		return FALSE;
	}

	return TRUE;
}


FLOAT AGearPawn_LocustTickerBase::GetGravityZ()
{
	if ( Physics == PHYS_RigidBody )
	{
		return Super::GetGravityZ() * GravityScaleZ;
	}
	else
	{
		return Super::GetGravityZ();
	}
}

UBOOL AGearPawn_LocustTickerBase::IsValidTargetFor(const AController* C) const
{
	// if I'm near one of dude's squad mates, I'm not a valid target cuz dude will blow up his squad mate!
	const AGearAI* GAI = ConstCast<AGearAI>(C);
	if(GAI != NULL && GAI->Squad != NULL && GAI->Squad->bPlayerSquad)
	{
		// see if there are any squad mates of C near me
		for(INT Idx=0;Idx<GAI->Squad->SquadMembers.Num();Idx++)
		{
			const AController* SquadMember = GAI->Squad->SquadMembers(Idx).Member;
			if(SquadMember!=C 
				&& SquadMember!=NULL 
				&& !SquadMember->IsPendingKill() 
				&& SquadMember->Pawn != NULL
				&& SquadMember->Pawn->IsHumanControlled())
			{
				FLOAT Threshold = ExplosionDamageRadius * 0.65f;
				Threshold *= Threshold;
				FLOAT DistSq = (Location - SquadMember->Pawn->Location).SizeSquared();
				if(DistSq < Threshold)
				{
					return FALSE;
				}
			}
		}
	}

	return Super::IsValidTargetFor(C);
}





void AGearSpawner_EHole::OpenEHole_Visuals( const FString& TagOfEvent )
{
	//warnf( TEXT("AGearSpawner_EHole::OpenEHole") );

	if( EHolePrefab != NULL && EHolePrefab->SequenceInstance != NULL )
	{
		for (INT Idx = 0; Idx < EHolePrefab->SequenceInstance->SequenceObjects.Num(); Idx++)
		{
			// if outer most sequence, check for SequenceActivated events as well

			USeqEvent_SequenceActivated* Evt = Cast<USeqEvent_SequenceActivated>(EHolePrefab->SequenceInstance->SequenceObjects(Idx));
			if( Evt != NULL )
			{
				if( Evt->InputLabel == TagOfEvent )
				{
					//warnf( TEXT("Found!!") );
					Evt->CheckActivate();
				}
				else
				{
					//warnf( TEXT("didn't find: %s"), *Evt->InputLabel );
				}
			}

			// find all of the collision cylinders in there and attach them to me!
			USeqEvent_TakeDamage* EvtD = Cast<USeqEvent_TakeDamage>(EHolePrefab->SequenceInstance->SequenceObjects(Idx));
			if( EvtD != NULL )
			{
				//warnf( TEXT("Found!! EvtD") );
				
				for( INT CompIndex = 0; CompIndex < EvtD->Originator->Components.Num(); CompIndex++ )
				{
					if( EvtD->Originator->Components(CompIndex) == NULL )
					{
						continue;
					}
					//warnf( TEXT("comp: %i %s"), CompIndex, *EvtD->Originator->Components(CompIndex)->GetName() );

					if( Cast<UCylinderComponent>(EvtD->Originator->Components(CompIndex)) != NULL )
					{
						//warnf( TEXT("UCylinderComponent: %i %s"), CompIndex, *EvtD->Originator->Components(CompIndex)->GetName() );

						// steal the cylinder!
						UCylinderComponent* Cyl = Cast<UCylinderComponent>(EvtD->Originator->Components(CompIndex));
						const FVector WorldCylLoc = Cyl->LocalToWorld.GetOrigin();
						const FVector ActorSpaceLoc = LocalToWorld().InverseTransformFVector(WorldCylLoc);

						EvtD->Originator->DetachComponent( Cyl );
						Cyl->Translation = ActorSpaceLoc;
						AttachComponent( Cyl );
						CollisionComponent = Cyl;

					}
				}
			}
		}
	}

	if( EHolePrefab != NULL )
	{
		TArray<AActor*> ActorsInPrefab;
		EHolePrefab->GetActorsInPrefabInstance( ActorsInPrefab );

		for( INT ActorIndex = 0; ActorIndex < ActorsInPrefab.Num(); ActorIndex++ )
		{
			if( ActorsInPrefab( ActorIndex ) != NULL )
			{
				//warnf( TEXT("AGearSpawner_EHole::OpenEHole found EholeSpawnLocation ActorIndex") );
				AGearSpawner_EholeSpawnLocation* SpawnLoc = Cast<AGearSpawner_EholeSpawnLocation>(ActorsInPrefab( ActorIndex ));
				if( SpawnLoc != NULL )
				{
					if( SpawnLoc->bEnabled == TRUE )
					{
						FSpawnerSlot NewSpawnSlot(EC_EventParm);

						const FVector WorldCylLoc = SpawnLoc->LocalToWorld().GetOrigin();
						//const FVector ActorSpaceLoc = LocalToWorld().InverseTransformFVector(WorldCylLoc);
						NewSpawnSlot.bEnabled = SpawnLoc->bEnabled;
						NewSpawnSlot.LocationOffset = WorldCylLoc + FVector(0.0f, 0.0f, -150.0f);
						NewSpawnSlot.RotationOffset.Pitch = 0;
						NewSpawnSlot.RotationOffset.Yaw = SpawnLoc->Rotation.Yaw;
						NewSpawnSlot.RotationOffset.Roll = 0;
						//warnf( TEXT("EmergeAnim: %d"), SpawnLoc->EmergeAnim );
						NewSpawnSlot.EmergeAnim = SpawnLoc->EmergeAnim;
						SpawnSlots.AddItem( NewSpawnSlot );
					}

				}
			}
		}
		
	}

	//warnf( TEXT("end! AGearSpawner_EHole::OpenEHole") );
}

void AGearSpawner_EHole::OpenEHoleNormal_Visuals()
{
	//warnf( TEXT("AGearSpawner_EHole::OpenEHoleImmediately") );
	OpenEHole_Visuals( TEXT("Open") );
}


void AGearSpawner_EHole::OpenEHoleImmediately_Visuals()
{
	//warnf( TEXT("AGearSpawner_EHole::OpenEHoleImmediately") );
	OpenEHole_Visuals( TEXT("OpenImmediately") );
}


void AGearSpawner_EHole::ClosEHole_Visuals()
{
	//warnf( TEXT("AGearSpawner_EHole::CloseEHole") );

	if( EHolePrefab != NULL && EHolePrefab->SequenceInstance != NULL )
	{
		for (INT Idx = 0; Idx < EHolePrefab->SequenceInstance->SequenceObjects.Num(); Idx++)
		{
			// if outer most sequence, check for SequenceActivated events as well
			USeqEvent_SequenceActivated* Evt = Cast<USeqEvent_SequenceActivated>(EHolePrefab->SequenceInstance->SequenceObjects(Idx));
			if( Evt != NULL )
			{
				if( Evt->InputLabel == TEXT("Close") )
				{
					//warnf( TEXT("Found!!") );
					Evt->CheckActivate();
				}
				else
				{
					//warnf( TEXT("didn't find: %s"), *Evt->InputLabel );
				}
			}
		}
	}

}


/**
 * This is our generic Decal Cache worker.  All of our indiv functions call this so we can modify it as we want and everyone will
 * get the benefit.
 *
 **/
UGearDecal* AGearObjectPool::GetDecal_Worker( INT& CurrIdx, const FDecalPool& TheDecalPool, TArray<struct FDecalDatum>& DecalList, const FVector& SpawnLocation )
{
	UGearDecal* DecalToUse = DecalList(0).GD;

	UBOOL bFoundADecalBasedOnRenderTime = FALSE;

	UGearDecal* DecalWithOldestSpawnTime = DecalList(0).GD;

	INT SpawnTimeIdx = 0;
	INT RendertimeIdx = 0;

	FLOAT OldestRendertime = DecalList(0).GD ? DecalList(0).GD->LastRenderTime : 0.0f;
	FLOAT OldestSpawnTime = DecalList(0).SpawnTime;

	// basically the number that we use for the + k is appFloor( CACHE_LINE_SIZE / sizeof(struct) )
	// each PREFETCH is going to grab the cache line containing the address OF what you pass in.
	// so you want to fetch as close to 128 bytes as possible
	// each thread can have up to 8 prefetches in flight at once
	// A big issue is if the Data is not aligned. In that case you wil need to probably prefetch the next cache line
	// because if you don't you will have a stall as the "inner" loop will be trying to work on something that is not in the cache yet
	// Additionally, you do not want to prefetch everything at once because the cache is a shared resource.  so when you prefetch something
	// you are going to bang a cache line.  which could:  affect other threads and could even affect your own computation as you just banged
	// out something you might need
	//
	//  For lists of pointers you are semi pwned as that access pattern is semi scattershot all over the place.  Depending on how much
	// iteration and computation you are going to be doing it might actually be better to have all of that data not be a pointer
	// but instead be either mirrored or to make them structs so you can prefetch them nicely
	//  
	//PREFETCH(&DecalList(Ida + 5)); // fetch the cache line containing DecalList(Ida + 5)
	//PREFETCH(&DecalList(Ida + 5) + CACHE_LINE_SIZE); // fetch the cache line after DecalList(Ida + 5) in case it straddles two
	//
	// Additionally you need to be careful about your stride especially when you have a function that takes a non constant sized list


	const FLOAT CurrTime = GWorld->GetTimeSeconds();

	// okie here we are going to reduce the number of active decals in the world based on FrameTime detail settings
	// bDropDetail
	// bAggressiveLOD
	INT DecalNum = DecalList.Num();

	const AWorldInfo* const WorldInfo = GWorld->GetWorldInfo();

	if( WorldInfo != NULL )
	{
		if( WorldInfo->bAggressiveLOD == TRUE )
		{
			DecalNum = TheDecalPool.MAX_DECALS_bAggressiveLOD;
		}
		else if( WorldInfo->bDropDetail == TRUE )
		{
			DecalNum = TheDecalPool.MAX_DECALS_bDropDetail;
		}
		else if( GEngine->IsSplitScreen() == TRUE )
		{
			DecalNum = TheDecalPool.MAX_DECALS_bAggressiveLOD;
		}
	}


	const INT PreFetchStride = 5;
	for( INT Ida = 0; Ida < DecalNum; Ida += PreFetchStride )
	{
#if XBOX
		// determine if we should PREFETCH or not
		if( Ida+PreFetchStride < DecalNum )
		{
			//warnf( TEXT("sizeof: %d"), sizeof(FDecalDatum) );
			// so 128 / sizeof(FDecalDatum) is 20   so 6  (this will prob get bigger so we are going to go for 5 as that divides into our list sizes
			PREFETCH(&DecalList(Ida + PreFetchStride)); // fetch the cache line containing DecalList(Ida + 5)
			PREFETCH(&DecalList(Ida + PreFetchStride) + CACHE_LINE_SIZE); // fetch the cache line after DecalList(Ida + 5) in case it straddles two
		}
#endif //_XBOX

		for( INT Idb = 0; Idb < PreFetchStride; ++Idb )
		{
			const INT Idx = Ida + Idb;
			if( Idx < DecalNum )
			{
				FDecalDatum& DecalData = DecalList(Idx);

				//warnf( TEXT(" %s %s %f %f"), *SpawnLocation.ToString(), *DecalData.SpawnLoc.ToString(), ( SpawnLocation - DecalData.SpawnLoc ).Size(), DecalData.CanSpawnDistance );
				if( ( SpawnLocation - DecalData.SpawnLoc ).Size() < DecalData.CanSpawnDistance )
				{
					// later instead of returning NULL we will add them to a list and then
					// combine them all together
					//warnf( TEXT( "  can't spawn too close!" ) );
					return NULL;
				}

				//warnf( TEXT( "  FOUND VALID" ) );

				//`log( "looking at:" @ DecalData.GD @ DecalData.GD.LastRenderTime @ DecalData.SpawnTime );
// 				debugf( TEXT("looking at: %s lastrender=%.3f lastspawn=%.3f"),
// 					DecalData.GD ? *DecalData.GD->GetPathName() : TEXT("None"),
// 					DecalData.GD->LastRenderTime,
// 					DecalData.SpawnTime );

				// Spawn a new component if the one in the cache has been destroyed. This can happen if the actor the component is attached to
				// gets destroyed and henceforth is marked as pending kill.
				// @todo also prob need to check if NOT RF_Unreachable  (e.g. the outer of this PSC was somehow destroyed and is not marked RF_PendingKill yet)
				if( DecalData.GD == NULL || DecalData.GD->IsPendingKill() )
				{
					//warnf( TEXT( "ZOOOP making new decal" ) );
					// Replace entry in cache.
					DecalData.GD = Cast<UGearDecal>( StaticConstructObject( UGearDecal::StaticClass(), this ) );
					DecalData.GD->MITV_Decal = Cast<UMaterialInstanceTimeVarying>( StaticConstructObject( UMaterialInstanceTimeVarying::StaticClass(), DecalData.GD ) );
					DecalData.SpawnTime = CurrTime;
					DecalData.GD->LastRenderTime = CurrTime; // we set this here as we just spawned it so it will be rendered and this fixes spawning multiple decals in the same frame
					DecalData.SpawnLoc = SpawnLocation;
					return DecalData.GD;
				}

				if( DecalData.GD->LastRenderTime < OldestRendertime )
				{
					//`log( "Previous:" @ DecalToUse @ "New:" @ DecalData.GD @ "at" @ OldestRendertime @ "new time:" @ DecalData.GD.LastRenderTime );
// 					debugf( TEXT("Previous: %s New: %s at %.3f new time %.3f"),
// 						DecalToUse ? *DecalToUse->GetPathName() : TEXT("None"),
// 						DecalData.GD ? *DecalData.GD->GetPathName() : TEXT("None"),
// 						OldestRendertime,
// 						DecalData.GD->LastRenderTime );

					DecalToUse = DecalData.GD;
					OldestRendertime = DecalData.GD->LastRenderTime;
					bFoundADecalBasedOnRenderTime = TRUE;
					RendertimeIdx = Idx;
				}

				if( DecalData.SpawnTime < OldestSpawnTime )
				{
					//`log( "Previous2:" @ DecalToUse @ "New:" @ DecalData.GD @ "at" @ OldestSpawnTime @ "new time:" @ DecalData.SpawnTime );
// 					debugf( TEXT("Previous2: %s New: %s at %.3f new time %.3f"),
// 						DecalToUse ? *DecalToUse->GetPathName() : TEXT("None"),
// 						DecalData.GD ? *DecalData.GD->GetPathName() : TEXT("None"),
// 						OldestSpawnTime,
// 						DecalData.SpawnTime );

					DecalWithOldestSpawnTime = DecalData.GD;
					OldestSpawnTime = DecalData.SpawnTime;
					SpawnTimeIdx = Idx;
				}
			}
		}
	}

	// we didn't find a decal based on render time so they all must be rendering (or on bsp (known issue))
	if( bFoundADecalBasedOnRenderTime == FALSE )
	{
		//`log( "bFoundADecalBasedOnRenderTime == FALSE" );
//		debugf( TEXT("bFoundADecalBasedOnRenderTime == FALSE") );
		DecalList(SpawnTimeIdx).SpawnTime = CurrTime;
		DecalList(SpawnTimeIdx).GD->LastRenderTime = CurrTime; // we set this here as we just spawned it so it will be rendered and this fixes spawning multiple decals in the same frame
		DecalList(SpawnTimeIdx).SpawnLoc = SpawnLocation;

		if( DecalWithOldestSpawnTime->MITV_Decal == NULL )
		{
			DecalWithOldestSpawnTime->MITV_Decal = Cast<UMaterialInstanceTimeVarying>( StaticConstructObject( UMaterialInstanceTimeVarying::StaticClass(), DecalWithOldestSpawnTime ) );
		}

		// needed for resetting parent to world transforms
		DecalWithOldestSpawnTime->DetachFromAny();
		DecalWithOldestSpawnTime->FreeStaticReceivers();
		DecalWithOldestSpawnTime->bHasBeenAttached=FALSE;
		//warnf( TEXT( "DecalWithOldestSpawnTime %s %s" ), *DecalWithOldestSpawnTime->GetName(), , *DecalWithOldestSpawnTime->MI_Decal->GetName( );
		return DecalWithOldestSpawnTime;
	}


		DecalList(RendertimeIdx).SpawnTime = CurrTime;
		DecalList(RendertimeIdx).GD->LastRenderTime = CurrTime; // we set this here as we just spawned it so it will be rendered and this fixes spawning multiple decals in the same frame
		DecalList(RendertimeIdx).SpawnLoc = SpawnLocation;

	if( ( DecalToUse != NULL ) && ( DecalToUse->MITV_Decal == NULL ) )
	{
		DecalToUse->MITV_Decal = Cast<UMaterialInstanceTimeVarying>( StaticConstructObject( UMaterialInstanceTimeVarying::StaticClass(), DecalToUse ) );
	}
	
	//warnf( TEXT( "DecalToUse %s %s" ), *DecalToUse->GetName(), *DecalToUse->MI_Decal->GetName() );

	// needed for resetting parent to world transforms
	DecalToUse->DetachFromAny();
	DecalToUse->FreeStaticReceivers();
	DecalToUse->bHasBeenAttached=FALSE;
	return DecalToUse;
}

UBOOL AGearObjectPool::IsSafeToRecreatePools()
{
	return ( !IsIncrementalPurgePending() && 
		(GWorld->TimeSinceLastPendingKillPurge <= GEngine->TimeBetweenPurgingPendingKillObjects || GEngine->TimeBetweenPurgingPendingKillObjects <= 0) );
}

/**
 * Gets the "real world" time.
 * The result is split between two INTs because passing a QWORD as a parameter from script is broken.
 */
void AGearPC::GetRealtime(INT& OutInt1,INT& OutInt2)
{
#if _XBOX
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);
	FILETIME FileTime;
	if(SystemTimeToFileTime(&SystemTime, &FileTime))
	{
		OutInt1 = FileTime.dwLowDateTime;
		OutInt2 = FileTime.dwHighDateTime;
		return;
	}
#endif
	OutInt1 = OutInt2 = 0;
	return;
}

#if !PS3
#define THUMBNAIL_DIMENSION 50
void RenderThread_Compress( FTexture2DDynamicResource* TextureResource, INT SizeX, INT SizeY, FAsyncCompressScreenshot* Task )
{
#if JPEGCOMPRESSION
	#if XBOX
		// create a thumbnail surface
		FTexture2DRHIRef ThumbnailTexture = RHICreateTexture2D(THUMBNAIL_DIMENSION,THUMBNAIL_DIMENSION,PF_A8R8G8B8,1,TexCreate_Dynamic|TexCreate_NoTiling,NULL);
		Task->SetThumbnailTexture(ThumbnailTexture);
	#endif

	#if THREADEDCOMPRESSION
		RHIKickCommandBuffer();
 		GThreadPool->AddQueuedWork(Task);
	#else
		Task->DoWork();
		appMemoryBarrier();
		Task->Dispose();
	#endif
#else
	// Get the screenshot data.
	UINT Stride = 0;
	void *BaseAddress = RHILockTexture2D(TextureResource->GetTexture2DRHI(), 0, FALSE, Stride, FALSE);
	Task->SetUncompressedData( BaseAddress, SizeX, SizeY, Stride );
	RHIUnlockTexture2D(TextureResource->GetTexture2DRHI(), 0, FALSE);

	// Launch the compression task.
	GThreadPool->AddQueuedWork(Task);

// 	FTexture2DMipMap& Screenshot = LastCapturedShot->Mips(0);
// 	void* ScreenshotData = Screenshot.Data.Lock(LOCK_READ_ONLY);
// 	INT ScreenshotDataSize = Screenshot.Data.GetBulkDataSize();
// 	check(ScreenshotDataSize == ((Screenshot.SizeX * Screenshot.SizeY) * sizeof(FColor)));
// 	debugf(TEXT("Compressing %d byte screenshot"), ScreenshotDataSize);
// 	check(!CompressScreenshotTask);
// 	CompressScreenshotTask = new FAsyncCompressScreenshot((FColor*)ScreenshotData, Screenshot.SizeX, Screenshot.SizeY);
// 	Screenshot.Data.Unlock();
// 	GThreadPool->AddQueuedWork((FAsyncCompressScreenshot*)CompressScreenshotTask);
#endif
}
#endif

UBOOL AGearPC::CompressLastScreenshot()
{
	UBOOL Result = FALSE;
#if !PS3
	if(LastCapturedShot != NULL)
	{
		INT SizeX = LastCapturedShot->GetSurfaceWidth();
		INT SizeY = LastCapturedShot->GetSurfaceHeight();
		FTexture2DDynamicResource* TextureResource = (FTexture2DDynamicResource*) LastCapturedShot->Resource;
		FAsyncCompressScreenshot* Task = new FAsyncCompressScreenshot(TextureResource->GetTexture2DRHI(), SizeX, SizeY);
 		check(!CompressScreenshotTask);
 		CompressScreenshotTask = Task;
		struct FCompressTextureParameters
		{
			FCompressTextureParameters(FTexture2DDynamicResource* InTextureResource, INT InSizeX, INT InSizeY, FAsyncCompressScreenshot* InTask)
			:	TextureResource(InTextureResource)
			,	SizeX(InSizeX)
			,	SizeY(InSizeY)
			,	Task(InTask)
			{
			}
			FTexture2DDynamicResource* TextureResource;
			INT SizeX;
			INT SizeY;
			FAsyncCompressScreenshot* Task;
		};
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			CompressTexture,
			FCompressTextureParameters,Parameters,FCompressTextureParameters(TextureResource,SizeX,SizeY,Task),
		{
			RenderThread_Compress(Parameters.TextureResource, Parameters.SizeX, Parameters.SizeY, Parameters.Task);
		});
		Result = TRUE;
	}
	else
	{
		debugf(TEXT("CompressLastScreenshot() called with no LastCapturedShot"));
	}
#endif
	return Result;
}

/** Returns true if this is a platform that supports screenshots */
UBOOL AGearPC::IsScreenshotPlatform()
{
#if _XBOX
	return TRUE;
#else
	return FALSE;
#endif
}

IMPLEMENT_CLASS(AGearpawn_SecurityBotStationaryBase)
void AGearpawn_SecurityBotStationaryBase::BeginDestroy()
{
	if (Controller != NULL && !Controller->bPendingDelete && !Controller->HasAnyFlags(RF_Unreachable|RF_PendingKill))
	{
		if(GWorld != NULL)
		{
			GWorld->DestroyActor(Controller);
		}
		Controller=NULL;
	}
	Super::BeginDestroy();
}
void AGearpawn_SecurityBotStationaryBase::GetBaseAimDir(FVector& out_AimDir)
{
	out_AimDir = -Mesh->GetBoneAxis(BarrelRotationBoneName,AXIS_X);
}

UBOOL AGearpawn_SecurityBotStationaryBase::ReachedDesiredRotation()
{
	if(GunRotControl == NULL || Mesh == NULL)
	{
		return Super::ReachedDesiredRotation();
	}

	FRotator Rot = GunRotControl->BoneRotation-BoneRotOffset;
	//DrawDebugLine(Location,Location + Rot.Vector()*150.f,255,0,0);
	//DrawDebugLine(Location,Location + DesiredTurretRot.Vector()*150.f,0,0,255);

	INT YawDiff = Abs((DesiredTurretRot.Yaw & 65535) - (Rot.Yaw & 65535));
	UBOOL bReached = ( (YawDiff < AllowedYawError) || (YawDiff > 65535 - AllowedYawError) );

	if(bReached)
	{
		eventNotifyDoneRotating();
	}

	return bReached;
}

UBOOL AGearpawn_SecurityBotStationaryBase::CanDeploy()
{
	for(INT idx=0;idx<Touching.Num();idx++)
	{
		if(Touching(idx) && Touching(idx)->IsA(APawn::StaticClass()))
		{
			return FALSE;
		}

	}

	return TRUE;
}

UBOOL AGearpawn_SecurityBotStationaryBase::NeedsTick()
{
	if(bTrackingAnEnemy)
	{
		return TRUE;
	}

	// if we're in a network game, use distance check
	AWorldInfo* WI = GWorld->GetWorldInfo();
	if(((ENetMode)WI->NetMode) != NM_Standalone && Role==ROLE_Authority)
	{
		AGearPC* ClosestPlayer = NULL;
		FLOAT	 ClosestDist = -1.f;
		// find shortest dist to any player
		for (AController* C = WI->ControllerList; C != NULL; C = C->NextController)
		{
			AGearPC* PC = Cast<AGearPC>(C);
			FVector ViewLoc=FVector(0.f); FRotator ViewRot=FRotator(0,0,0);
			if (PC != NULL)
			{
				PC->eventGetPlayerViewPoint(ViewLoc,ViewRot);
				FLOAT Dist = (Location - ViewLoc).Size();
				if(Dist < ClosestDist || ClosestDist < 0.f)
				{
					ClosestPlayer = PC;
					ClosestDist = Dist;
				}
			}
		}

		if(ClosestDist > 0.f && ClosestDist < Max<FLOAT>(DetectionRange*3.0f, 1500.f))
		{
			return TRUE;
		}
		return FALSE;
	}
	else if(((ENetMode)WI->NetMode) != NM_Standalone ) // clients always tick (but don't do work)
	{
		return TRUE;		
	}
	else // otherwise use lastrendertime
	{
		if(GWorld->GetTimeSeconds() - LastRenderTime < TimeNotRenderedUntilLODFreeze)
		{
			return TRUE;
		}

		return FALSE;
	}
}

UBOOL AGearpawn_SecurityBotStationaryBase::Tick( FLOAT DeltaSeconds, ELevelTick TickType )
{
	// check to see if we should turn on again
		if(bLODFrozen)
	{
		if(NeedsTick())
		{
			bLODFrozen = FALSE;
		}
	}

	if(bEnabled && bDeployed && !bLODFrozen)
	{
		if(!NeedsTick())
		{
			bLODFrozen = TRUE;
		}

		if(GunRotControl && Mesh)
		{
			FRotator Des = DesiredTurretRot + BoneRotOffset;
			GunRotControl->BoneRotation.Yaw = fixedTurn(GunRotControl->BoneRotation.Yaw,Des.Yaw,appTrunc(TurretRotationSpeed*182.f*DeltaSeconds));
 			
			// if we have an enemy in our sights rotate slower
			INT PitchRate = appTrunc(RotationRate.Pitch*DeltaSeconds);
			if(bTrackingAnEnemy)
			{
				PitchRate = appTrunc(TurretRotationSpeed*182.f*DeltaSeconds);
			}

			GunRotControl->BoneRotation.Pitch = fixedTurn(GunRotControl->BoneRotation.Pitch,Des.Pitch,PitchRate);
			GunRotControl->BoneRotation.Roll = Des.Roll;
		}

		FVector AimDir;
		GetBaseAimDir(AimDir);

		// do a check along the beam
		FVector FireStartLoc = eventGetPhysicalFireStartLoc(FVector(0.f));
		FCheckResult Hit(1.0f);
		DWORD Flags = TRACE_Pawns|TRACE_SingleResult;
		if(bConformLaserToGeometry)
		{
			Flags |= TRACE_World;
		}

		LaserHitPoint = FireStartLoc+AimDir*DetectionRange;
		if(bConformLaserToGeometry || Role==ROLE_Authority) // only need to trace if we need to conform to geo, or if we are the server and need to hit detect
		{
			if(!GWorld->SingleLineCheck(Hit,this,FireStartLoc+AimDir*DetectionRange,FireStartLoc,Flags))	
			{
				AGearPawn* HitPawn = Cast<AGearPawn>(Hit.Actor);
				if(HitPawn != NULL && HitPawn->Controller != NULL && !HitPawn->IsProtectedByCover(AimDir,TRUE) && HitPawn->GetTeamNum() < TEAM_EVERYONE)
				{
					if(Role == ROLE_Authority && HitPawn->IsPlayerPawn() && Controller != NULL)
					{
						Controller->eventSeePlayer(HitPawn);
					}
				}
				LaserHitPoint = Hit.Location;
			}
			else if(Role == ROLE_Authority) // clients don't care about second pass
			{
				FireStartLoc += SecondPassOffset;
				FCheckResult Hit(1.0f);
				if(!GWorld->SingleLineCheck(Hit,this,FireStartLoc+AimDir*DetectionRange,FireStartLoc,Flags))
				{
					AGearPawn* HitPawn = Cast<AGearPawn>(Hit.Actor);

					if(HitPawn != NULL && HitPawn->Controller != NULL && !HitPawn->IsProtectedByCover(AimDir,TRUE) && HitPawn->GetTeamNum() < TEAM_EVERYONE)
					{
						if(Role == ROLE_Authority && HitPawn->IsPlayerPawn() && Controller != NULL)
						{
							UClass* PawnClass = HitPawn->GetClass();
							for(INT Idx=0;Idx<SecondTracePassValidClasses.Num();Idx++)
							{
								if(PawnClass->IsChildOf( SecondTracePassValidClasses(Idx) ) )
								{
									Controller->eventSeePlayer(HitPawn);
									break;
								}
							}
						}
					}
				}

			}
		}
	}

	return Super::Tick(DeltaSeconds,TickType);
}

FVector AGearpawn_SecurityBotStationaryBase::GetPhysicalFireStartLoc( FVector FireOffset )
{
	FVector loc;

	if(Mesh == NULL)
	{
		return FVector(0.f);
	}

	Mesh->GetSocketWorldLocationAndRotation(PhysicalFireLocBoneName,loc,NULL);

	return loc;
}

FVector AGearpawn_SecurityBotStationaryBase::GetPawnViewLocation()
{
	if(Weapon != NULL)
	{
		return Weapon->eventGetMuzzleLoc();
	}
	return Super::GetPawnViewLocation();
}

IMPLEMENT_CLASS(USecurityBotStationaryRenderingComponent);
#include "DebugRenderSceneProxy.h"
class FSecurityBotStationaryRenderingSceneProxy : public FDebugRenderSceneProxy
{
public:

	FSecurityBotStationaryRenderingSceneProxy(const USecurityBotStationaryRenderingComponent* InComponent):
	  FDebugRenderSceneProxy(InComponent)
	  {
		  // draw patrol arc
		  check(InComponent);
		  AGearpawn_SecurityBotStationaryBase *PawnOwner= Cast<AGearpawn_SecurityBotStationaryBase>(InComponent->GetOwner());
		  if(PawnOwner && PawnOwner->IsSelected())
		  {
			  FRotator Rot = FRotator(0,0,0);
			  FVector MyLoc = PawnOwner->Location;
			  
			  // left
			  if(PawnOwner->PatrolArcDegreesLeft >=0)
			  {
				  Rot.Yaw -= PawnOwner->PatrolArcDegreesLeft*182;
			  }
			  else
			  {
				  Rot.Yaw -= PawnOwner->PatrolArcDegrees*91;
			  }

			  Rot.Pitch = appTrunc(PawnOwner->PatrolPitchoffset * 182.f);
			  FRotationMatrix OffsetMat(Rot);
			  FRotationMatrix CurrentMat(PawnOwner->Rotation);
			  Rot = (OffsetMat * CurrentMat).Rotator();
			  new(Lines) FDebugLine(MyLoc,MyLoc + Rot.Vector() * PawnOwner->DetectionRange,FColor(255,0,0));		 

			  // right
			  Rot = FRotator(0,0,0);
			  if(PawnOwner->PatrolArcDegreesRight >=0)
			  {
				  Rot.Yaw += PawnOwner->PatrolArcDegreesRight*182;
			  }
			  else
			  {
				  Rot.Yaw += PawnOwner->PatrolArcDegrees*91;
			  }

			  Rot.Pitch = appTrunc(PawnOwner->PatrolPitchoffset * 182.f);
			  OffsetMat = FRotationMatrix(Rot);
			  Rot = (OffsetMat * CurrentMat).Rotator();
			  new(Lines) FDebugLine(MyLoc,MyLoc + Rot.Vector() * PawnOwner->DetectionRange,FColor(0,0,255));

			  // center
			  new(DashedLines) FDashedLine(MyLoc,MyLoc + PawnOwner->Rotation.Vector()*PawnOwner->DetectionRange,FColor(255,255,255),50.f);	  
		  }
	  }

	  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	  {
		  FPrimitiveViewRelevance Result;
		  Result.bDynamicRelevance = IsShown(View);
		  Result.SetDPG(SDPG_World,TRUE);
		  if (IsShadowCast(View))
		  {
			  Result.bShadowRelevance = TRUE;
		  }
		  return Result;
	  }

	  virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	  virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	  DWORD GetAllocatedSize( void ) const { return( FDebugRenderSceneProxy::GetAllocatedSize() ); }
};



FPrimitiveSceneProxy* USecurityBotStationaryRenderingComponent::CreateSceneProxy()
{
	return new FSecurityBotStationaryRenderingSceneProxy(this);
}

/** Called each from while the Matinee action is running, to set the animation weights for the actor. */
void AGearPawn::SetAnimWeights( const TArray<struct FAnimSlotInfo>& SlotInfos )
{
	MAT_SetAnimWeights( SlotInfos );
}

/** Util to update motor strength of motors after being knocked down */
void AGearPawn::TickKnockDownMotorStrength(FLOAT DeltaTime)
{
	if( Mesh->PhysicsAssetInstance )
	{
		FLOAT TimeSinceKnockDown = GWorld->GetTimeSeconds() - KnockDownStartTime;			
		FLOAT MotorAmount = KnockDownMotorScale.Eval(TimeSinceKnockDown, 0.f);

		// Set motor strength
		Mesh->PhysicsAssetInstance->SetAllMotorsAngularDriveParams(KnockDownMotorStrength * MotorAmount, KnockDownMotorDamping * MotorAmount, 0.f, Mesh, TRUE);
	}
}

ANavigationPoint* AGearPawn::GetBestAnchor( AActor* TestActor, FVector TestLocation, UBOOL bStartPoint, UBOOL bOnlyCheckVisible, FLOAT& out_Dist )
{
	if( !ProhibitedFindAnchorPhysicsModes.ContainsItem(Physics))
	{
		return Super::GetBestAnchor(TestActor,TestLocation,bStartPoint,bOnlyCheckVisible,out_Dist);
	}
	else
	{
		debugf(NAME_DevPath,TEXT("%s tried to findanchor with prohibited physics mode! was: %i"),*GetName(),Physics);
	}

	return NULL;
}

void UGearCheatManager::ForceLog(const FString& s)
{
	debugf(TEXT("%s"),*s);
}

FVector AGearPawn_LocustNemacystBase::GetPawnViewLocation()
{
	if (Mesh && PawnViewSocketName != NAME_None)
	{
		FVector Loc;
		Mesh->GetSocketWorldLocationAndRotation(PawnViewSocketName,Loc,NULL);
		return Loc;
	}
	else
	{
		return Super::GetPawnViewLocation();
	}
}

void AGearPawn_LocustNemacystBase::physicsRotation(FLOAT deltaTime, FVector OldVelocity)
{
	if ( !Controller )
		return;
	// always call SetRotationRate() as it may change our DesiredRotation
	FRotator deltaRot = Controller->SetRotationRate(deltaTime);
	if( !bCrawler && (Rotation == DesiredRotation) && !IsHumanControlled() )
		return;
	
	if(Physics == PHYS_Flying && !bRollToDesired)
	{
		DesiredRotation.Pitch = 0;
	}

	Super::physicsRotation(deltaTime,OldVelocity);
}

FVector AGearPawn_LocustBrumakHelper_Base::GetPawnViewLocation()
{
	if(Weapon != NULL)
	{
		return Weapon->eventGetPhysicalFireStartLoc();
	}

	return Super::GetPawnViewLocation();
}

/** TRUE if NameToMatch is contained within NameArray */
UBOOL AGearPawn_Infantry::MatchNameArray(FName NameToMatch, const TArray<FName>& NameArray)
{
	for(INT i=0; i<NameArray.Num(); i++)
	{
		if( NameArray(i) == NameToMatch )
		{
			return TRUE;
		}
	}
	return FALSE;
}

/** Infantry stuck in world */
UBOOL AGearPawn_Infantry::InfantryStuckInWorld()
{
	if(CylinderComponent)
	{
		FMemMark Mark(GMainThreadMemStack);

		const FVector CheckVec = 0.9f * CylinderComponent->CollisionHeight * FVector(0,0,1);
		const FVector CheckStart = Location + CheckVec;
		const FVector CheckEnd = Location - CheckVec;

		FCheckResult* FirstHit = GWorld->MultiLineCheck(GMainThreadMemStack, CheckEnd, CheckStart, FVector(0,0,0), (TRACE_Volumes | TRACE_World), this);

		// See if we are penetrating an Actor
		AActor* HitActor = NULL;

		// Iterate over results..
		for( FCheckResult* Test=FirstHit; Test; Test=Test->GetNext() )
		{
			// .. ignore base ..
			if( this->IsBlockedBy(Test->Actor,Test->Component) )
			{
				HitActor = Test->Actor;
				break;
			}
		}

		Mark.Pop();

		// Create output device that will work in release builds
		UConsole* ViewportConsole = (GEngine->GameViewport != NULL) ? GEngine->GameViewport->ViewportConsole : NULL;
		FConsoleOutputDevice StrOut(ViewportConsole);

		if(HitActor)
		{
			StrOut.Logf( TEXT("InfantryStuckInWorld: ZE Stuck in %s!"), *HitActor->GetPathName() );
			return TRUE;
		}
	}

	//StrOut.Logf( TEXT("InfantryStuckInWorld: %f - NOT STUCK"), GWorld->GetTimeSeconds() );
	return FALSE;
}

void AGearAimAssistActor::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * 500.0f;

	const FLOAT Multiplier = ( ModifiedScale.X > 0.0f || ModifiedScale.Y > 0.0f || ModifiedScale.Z > 0.0f ) ? 1.0f : -1.0f;
	Radius += Multiplier * ModifiedScale.Size();
	Radius = Max( 0.f, Radius );
	PostEditChange( NULL );
}


/** Update the render component to match the force radius. */
void AGearAimAssistActor::PostEditChange(UProperty* PropertyThatChanged)
{
	if (SphereRenderComponent)
	{
		FComponentReattachContext ReattachContext(SphereRenderComponent);
		SphereRenderComponent->SphereRadius = Radius;
	}
}



void UGearPerMapColorConfig::PostLoad() 
{
	Super::PostLoad();
	RefreshTypeData();
}


void UGearPerMapColorConfig::PostEditChange( class FEditPropertyChain& PropertyThatChanged ) 
{
	Super::PostEditChange(PropertyThatChanged);
	RefreshTypeData();
}


void UGearPerMapColorConfig::RefreshTypeData() 
{
	while( WeaponColors.Num() < WC_MAX )
	{
		FWeaponColorDatum NewEntry;
		appMemzero( &NewEntry, sizeof(NewEntry) );
		NewEntry.Type = WeaponColors.Num();
		WeaponColors.AddItem( NewEntry );
		MarkPackageDirty( TRUE );
		//warnf( TEXT("ADDED %s %d"), *GetFullName(), FXInfo.Num() );
	}


	// at this point we now have all of the "correctly added" entries so anything missing needs to be replaced
	for( INT Idx = 0; Idx < WeaponColors.Num() && Idx < WC_MAX ; ++Idx )
	{
		const FWeaponColorDatum& Entry = WeaponColors( Idx );
		if( Entry.Type != Idx )
		{
			FWeaponColorDatum NewEntry;
			appMemzero( &NewEntry, sizeof(NewEntry) );
			NewEntry.Type = Idx;

			WeaponColors.InsertItem( NewEntry, Idx );
			MarkPackageDirty( TRUE );
			//warnf( TEXT("INSERT %s %d"), *GetFullName(), Idx );
		}
	}
}

/** @return the closest usable PlayerStart to the meatflag destination using the path network */
APlayerStart* AGearGameCTM_Base::GetClosestStartToDestination()
{
	if (DestinationPoint != NULL && GearPlayerStartClass != NULL)
	{
		// simple barebones pathfinding that uses only base Distance of ReachSpecs and assumes that all paths that could be usable are
		// the intent is to handle simple walls and elevation changes that a simple line distance check would break on,
		// not to perfectly consider reachability and actual traversal time

		for (ANavigationPoint* N = WorldInfo->NavigationPointList; N != NULL; N = N->nextNavigationPoint)
		{
			if (N->nextNavigationPoint)
			{
				CONSOLE_PREFETCH(&N->nextNavigationPoint->NavOctreeObject);
				CONSOLE_PREFETCH(&N->nextNavigationPoint->Cost);
				CONSOLE_PREFETCH(&N->nextNavigationPoint->AnchoredPawn);
			}
			N->ClearForPathFinding();
		}
		DestinationPoint->visitedWeight = 0;

		ANavigationPoint* CurrentNode = DestinationPoint;
		ANavigationPoint* ListEnd = CurrentNode;
		do
		{
			if (CurrentNode->GetClass() == GearPlayerStartClass)
			{
				checkSlow(CurrentNode->IsA(APlayerStart::StaticClass()));
				return Cast<APlayerStart>(CurrentNode);
			}
			CurrentNode->bAlreadyVisited = TRUE;
			for (INT i = 0; i < CurrentNode->PathList.Num(); i++)
			{
				UReachSpec* Spec = CurrentNode->PathList(i);
				if (Spec != NULL && *Spec->End != NULL)
				{
					ANavigationPoint* TestNode = Spec->End.Nav();
					if (!TestNode->bAlreadyVisited && CurrentNode->visitedWeight + Spec->Distance < TestNode->visitedWeight && !Spec->IsProscribed())
					{
						TestNode->visitedWeight = CurrentNode->visitedWeight + Spec->Distance;
						// start at end for insertion search unless this node has just reduced its weight (in which case start at the previous node to this one)
						ANavigationPoint* InsertPoint = ListEnd;
						if (TestNode->prevOrdered != NULL)
						{
							InsertPoint = TestNode->prevOrdered;
							TestNode->prevOrdered->nextOrdered = TestNode->nextOrdered;
							if (TestNode->nextOrdered != NULL)
							{
								TestNode->nextOrdered->prevOrdered = TestNode->prevOrdered;
							}
						}
						for (; InsertPoint != NULL; InsertPoint = InsertPoint->prevOrdered)
						{
							if (InsertPoint->visitedWeight <= TestNode->visitedWeight)
							{
								TestNode->nextOrdered = InsertPoint->nextOrdered;
								InsertPoint->nextOrdered = TestNode;
								TestNode->prevOrdered = InsertPoint;
								if (TestNode->nextOrdered != NULL)
								{
									TestNode->nextOrdered->prevOrdered = TestNode;
								}
								if (InsertPoint == ListEnd)
								{
									ListEnd = TestNode;
								}
								else if (ListEnd == TestNode)
								{
									while (ListEnd->nextOrdered != NULL)
									{
										ListEnd = ListEnd->nextOrdered;
									}
								}
								break;
							}
						}
					}
				}
			}
			CurrentNode = CurrentNode->nextOrdered;
		} while (CurrentNode != NULL);
	}

	return NULL;
}

/**
 * Reads the Meatflag xuid from config and caches that in the variable to prevent reparsing
 *
 * @return the xuid to use for the meatflag
 */
FUniqueNetId AGearGameMP_Base::GetMeatflagUniqueId(void)
{
	if (MeatflagId.Uid == 0)
	{
		FString MeatXuid;
		// Read it as a string since the config cache doesn't handle QWORDs
		if (GConfig->GetString(TEXT("Meatflag"),TEXT("MeatflagXuid"),MeatXuid,GGameIni))
		{
			if (MeatXuid.Len())
			{
				// Parse it into a QWORD
				MeatflagId.Uid = appAtoi64(*MeatXuid);
			}
		}
	}
	return MeatflagId;
}

/** @return determines whether we should ignore or handle network errors */
UBOOL AGearPC::ShouldIgnoreNetworkErrors(void)
{
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine != NULL)
	{
		if (GameEngine->GPendingLevel != NULL)
		{
			// ignore connection errors from the previous level, but accept those from the pending level
			return (GameEngine->GPendingLevel->ConnectionError.Len() == 0 && !GameEngine->GPendingLevel->bSuccessfullyConnected);
		}
		else if (GameEngine->TravelURL != TEXT(""))
		{
			return TRUE;
		}
	}
	return bIgnoreNetworkMessages;
}


/** Needed to pass a start IE_Released event to the UI **/
void AGearPC::Sentinel_PressStartKeyAtStartMenu()
{
	for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
	{
		ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
		if(Player->ViewportClient != NULL )
		{
			Player->ViewportClient->InputKey( Player->ViewportClient->Viewport, 0, KEY_XboxTypeS_Start, IE_Released, 0.0f, TRUE );
		}
	}
	
}


/** This wil make a bugit with the current language.  So we can then copy them to the PC without them over writing each other **/
void AGearPC::Sentinel_DoBugitWithLang()
{
	eventBugIt( FString(UObject::GetLanguage()) + FString(appComputerName()) );
}


