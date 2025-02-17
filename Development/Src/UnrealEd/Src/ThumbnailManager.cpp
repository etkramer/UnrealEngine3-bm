/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"

#include "EngineMaterialClasses.h"
#include "EngineAnimClasses.h"
#include "EnginePrefabClasses.h"
#include "EnginePhysicsClasses.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "EngineSoundClasses.h"
#include "LensFlare.h"

#define _WANTS_ALL_HELPERS 1
// FPreviewScene derived helpers for rendering
#include "ThumbnailHelpers.h"

IMPLEMENT_CLASS(UThumbnailManager);
IMPLEMENT_CLASS(UThumbnailRenderer);
IMPLEMENT_CLASS(UThumbnailLabelRenderer);
IMPLEMENT_CLASS(UTextureThumbnailRenderer);
IMPLEMENT_CLASS(UTextureCubeThumbnailRenderer);
IMPLEMENT_CLASS(UDefaultSizedThumbnailRenderer);
IMPLEMENT_CLASS(UMaterialInstanceThumbnailRenderer);
IMPLEMENT_CLASS(UParticleSystemThumbnailRenderer);
IMPLEMENT_CLASS(USkeletalMeshThumbnailRenderer);
IMPLEMENT_CLASS(UStaticMeshThumbnailRenderer);
IMPLEMENT_CLASS(UIconThumbnailRenderer);
IMPLEMENT_CLASS(UMemCountThumbnailLabelRenderer);
IMPLEMENT_CLASS(UAnimSetLabelRenderer);
IMPLEMENT_CLASS(UAnimTreeLabelRenderer);
IMPLEMENT_CLASS(UGenericThumbnailLabelRenderer);
IMPLEMENT_CLASS(UPhysicsAssetLabelRenderer);
IMPLEMENT_CLASS(UPhysXDestructibleLabelRenderer);
IMPLEMENT_CLASS(USkeletalMeshLabelRenderer);
IMPLEMENT_CLASS(UStaticMeshLabelRenderer);
IMPLEMENT_CLASS(UFracturedStaticMeshLabelRenderer);
IMPLEMENT_CLASS(UPostProcessLabelRenderer);
IMPLEMENT_CLASS(USoundLabelRenderer);
IMPLEMENT_CLASS(UArchetypeThumbnailRenderer);
IMPLEMENT_CLASS(UPrefabThumbnailRenderer);
IMPLEMENT_CLASS(UFontThumbnailRenderer);
IMPLEMENT_CLASS(UUISceneThumbnailRenderer);
IMPLEMENT_CLASS(UMaterialInstanceLabelRenderer);
IMPLEMENT_CLASS(ULensFlareThumbnailRenderer);
IMPLEMENT_CLASS(UParticleSystemLabelRenderer);

#define DEBUG_THUMBNAIL_MANAGER 0

#if DEBUG_THUMBNAIL_MANAGER
/**
 * Logs information about the thumbnail render info structure for debugging
 * purposes
 */
void DumpRenderInfo(const FThumbnailRenderingInfo& RenderInfo)
{
	debugf(TEXT("Thumbnail rendering entry:"));
	debugf(TEXT("\tClassNeedingThumbnailName %s"),*RenderInfo.ClassNeedingThumbnailName);
	debugf(TEXT("\tClassNeedingThumbnail %s"),
		RenderInfo.ClassNeedingThumbnail ? *RenderInfo.ClassNeedingThumbnail->GetName() : TEXT("Null"));
	debugf(TEXT("\tRendererClassName %s"),*RenderInfo.RendererClassName);
	debugf(TEXT("\tRenderer %s"),
		RenderInfo.Renderer ? *RenderInfo.Renderer->GetName() : TEXT("Null"));
	debugf(TEXT("\tLabelRendererClassName %s"),*RenderInfo.LabelRendererClassName);
	debugf(TEXT("\tLabelRenderer %s"),
		RenderInfo.LabelRenderer ? *RenderInfo.LabelRenderer->GetName() : TEXT("Null"));
	debugf(TEXT("\tIconName %s"),*RenderInfo.IconName);
	debugf(TEXT("\tBorderColor R=%d,G=%d,B=%d,A=%d"),RenderInfo.BorderColor.R,
		RenderInfo.BorderColor.G,RenderInfo.BorderColor.B,RenderInfo.BorderColor.A);
}

// Conditional macro so it compiles out easily
#define DUMP_INFO(x) DumpRenderInfo(x)

#else

// Compiled out version
#define DUMP_INFO(x) void(0)

#endif

/**
 * Fixes up any classes that need to be loaded in the thumbnail types
 */
void UThumbnailManager::Initialize(void)
{
	if (bIsInitialized == FALSE)
	{
		InitializeRenderTypeArray(RenderableThumbnailTypes, GetRenderInfoMap());
		InitializeRenderTypeArray(ArchetypeRenderableThumbnailTypes, GetArchetypeRenderInfoMap());

		bIsInitialized = TRUE;
	}
}

/**
 * Fixes up any classes that need to be loaded in the thumbnail types per-map type
 */
void UThumbnailManager::InitializeRenderTypeArray( TArray<FThumbnailRenderingInfo>& ThumbnailRendererTypes, FClassToRenderInfoMap& ThumbnailMap )
{
	// Loop through setting up each thumbnail entry
	for (INT Index = 0; Index < ThumbnailRendererTypes.Num(); Index++)
	{
		FThumbnailRenderingInfo& RenderInfo = ThumbnailRendererTypes(Index);
		// Load the class that this is for
		if (RenderInfo.ClassNeedingThumbnailName.Len() > 0)
		{
			// Try to load the specified class
			RenderInfo.ClassNeedingThumbnail = LoadObject<UClass>(NULL,
				*RenderInfo.ClassNeedingThumbnailName,NULL,LOAD_None,NULL);
		}
		if (RenderInfo.RendererClassName.Len() > 0)
		{
			// Try to create the renderer object by loading its class and
			// constructing one
			UClass* RenderClass = LoadObject<UClass>(NULL,*RenderInfo.RendererClassName,
				NULL,LOAD_None,NULL);
			if (RenderClass != NULL)
			{
				RenderInfo.Renderer = ConstructObject<UThumbnailRenderer>(RenderClass);
				// Set the icon information if this is an icon renderer
				if (RenderClass->IsChildOf(UIconThumbnailRenderer::StaticClass()))
				{
					((UIconThumbnailRenderer*)RenderInfo.Renderer)->IconName = RenderInfo.IconName;
				}
			}
		}
		if (RenderInfo.LabelRendererClassName.Len() > 0)
		{
			// Try to create the label renderer object by loading its class and
			// constructing one
			UClass* RenderClass = LoadObject<UClass>(NULL,*RenderInfo.LabelRendererClassName,
				NULL,LOAD_None,NULL);
			if (RenderClass != NULL)
			{
				RenderInfo.LabelRenderer = ConstructObject<UThumbnailLabelRenderer>(RenderClass);
			}
		}
		// Add this to the map if it created the renderer component
		if (RenderInfo.Renderer != NULL)
		{
			ThumbnailMap.Set(RenderInfo.ClassNeedingThumbnail,&RenderInfo);
		}
		DUMP_INFO(RenderInfo);
	}
}

/**
 * Returns the entry for the specified object
 *
 * @param Object the object to find thumbnail rendering info for
 *
 * @return A pointer to the rendering info if valid, otherwise NULL
 */
FThumbnailRenderingInfo* UThumbnailManager::GetRenderingInfo(UObject* Object)
{
	// If something may have been GCed, empty the map so we don't crash
	if (bMapNeedsUpdate == TRUE)
	{
		GetRenderInfoMap().Empty();
		GetArchetypeRenderInfoMap().Empty();
		bMapNeedsUpdate = FALSE;
	}

	check(Object);
	const UBOOL bIsArchtypeObject = Object->IsTemplate(RF_ArchetypeObject);

	FClassToRenderInfoMap& RenderInfoMap = bIsArchtypeObject
		? GetArchetypeRenderInfoMap()
		: GetRenderInfoMap();

	TArray<FThumbnailRenderingInfo>& ThumbnailTypes = bIsArchtypeObject
		? ArchetypeRenderableThumbnailTypes
		: RenderableThumbnailTypes;

	// Search for the cached entry and do the slower if not found
	FThumbnailRenderingInfo* RenderInfo = RenderInfoMap.FindRef(Object->GetClass());;
	if (RenderInfo == NULL)
	{
		// Loop through searching for the right thumbnail entry
		for (INT Index = 0; Index < ThumbnailTypes.Num() &&
			RenderInfo == NULL; Index++)
		{
			RenderInfo = &ThumbnailTypes(Index);
			// See if this thumbnail renderer will work for the specified class or
			// if there is some data reason not to render the thumbnail
			if (Object->GetClass()->IsChildOf(RenderInfo->ClassNeedingThumbnail) == FALSE ||
				RenderInfo->Renderer == NULL ||
				RenderInfo->Renderer->SupportsThumbnailRendering(Object,FALSE) == FALSE)
			{
				RenderInfo = NULL;
			}
		}
	}
	// If this is null, there isn't a renderer for it. So don't search anymore unless it's an archetype
	if ( RenderInfo == NULL )
	{
		RenderInfoMap.Set(Object->GetClass(),&NotSupported);
	}
	// Check to see if this object is the "not supported" type or not
	else if (RenderInfo == &NotSupported)
	{
		RenderInfo = NULL;
	}
	// Make sure to add it to the cache if it is missing
	else if (RenderInfoMap.HasKey(Object->GetClass()) == FALSE)
	{
		RenderInfoMap.Set(Object->GetClass(),RenderInfo);
	}
	// Perform the per object check, in case there is a flag that invalidates this
	if (RenderInfo && RenderInfo->Renderer &&
		RenderInfo->Renderer->SupportsThumbnailRendering(Object,TRUE) == FALSE)
	{
		RenderInfo = NULL;
	}
	return RenderInfo;
}

/**
 * Serializes any object renferences and sets the map needs update flag
 *
 * @param Ar the archive to serialize to/from
 */
void UThumbnailManager::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	// Just mark us as dirty so that the cache is rebuilt
	bMapNeedsUpdate = TRUE;
}

/**
 * Deletes the map of class to render info pointers
 */
void UThumbnailManager::FinishDestroy(void)
{
	FClassToRenderInfoMap* Map = (FClassToRenderInfoMap*)RenderInfoMap;
	if (Map != NULL)
	{
		Map->Empty();
		delete Map;
		Map = NULL;
	}

	if ( ArchetypeRenderInfoMap != NULL )
	{
		delete ArchetypeRenderInfoMap;
		ArchetypeRenderInfoMap = NULL;
	}
	Super::FinishDestroy();
}

/**
 * Clears cached components.
 */
void UThumbnailManager::ClearComponents(void)
{
	BackgroundComponent = NULL;
	SMPreviewComponent	= NULL;
	SKPreviewComponent	= NULL;
}

/**
 * Draws a thumbnail for the object that was specified. Uses the icon
 * that was specified as the tile
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType ignored
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void UIconThumbnailRenderer::Draw(UObject* Object,EThumbnailPrimType,
		INT X,INT Y,DWORD Width,DWORD Height,FViewport*,
		FCanvas* Canvas,EThumbnailBackgroundType /*BackgroundType*/,
		FColor PreviewBackgroundColor,
		FColor PreviewBackgroundColorTranslucent)
{
	// Now draw the icon
	DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
		GetIcon()->Resource,FALSE);
}

/**
 * Calculates the size the thumbnail labels will be for the specified font.
 * Note: that this is a common method for handling lists of strings. The
 * child class is responsible for building this list of strings.
 *
 * @param Labels the list of strings to write out as the labels
 * @param Font the font object to render with
 * @param Viewport the viewport that will be rendered into
 * @param RI the render interface to use for getting the size
 * @param OutWidth the var that gets the width of the labels
 * @param OutHeight the var that gets the height
 */
void UThumbnailLabelRenderer::GetSizeFromLabels(const TArray<FString>& Labels,
	UFont* Font,FViewport* Viewport,FCanvas* Canvas,DWORD& OutWidth,
	DWORD& OutHeight)
{
	check(Canvas && Font);
	// Set our base "out" values
	OutWidth = 0;
	OutHeight = STD_TNAIL_HIGHLIGHT_EDGE / 2;
	if (Labels.Num() > 0)
	{
		// Draw each label for this thumbnail
		for (INT Index = 0; Index < Labels.Num(); Index++)
		{
			// Ignore empty strings
			if (Labels(Index).Len() > 0)
			{
				INT LabelWidth;
				INT LabelHeight;
				// Get the size that will be drawn
				StringSize(Font,LabelWidth,LabelHeight,*Labels(Index));
				// Update our max width if this string is wider
				if (static_cast<UINT>(LabelWidth) > OutWidth)
				{
					OutWidth = LabelWidth;
				}
				// Update our total height
				OutHeight += LabelHeight;
			}
		}
	}
}

/**
 * Renders the thumbnail labels for the specified object with the specified
 * font and text color
 * Note: that this is a common method for handling lists of strings. The
 * child class is resposible for building this list of strings.
 *
 * @param Labels the list of strings to write out as the labels
 * @param Font the font to draw with
 * @param X the X location to start drawing at
 * @param Y the Y location to start drawing at
 * @param Viewport the viewport to draw into
 * @param RI the render interface to draw with
 * @param TextColor the color to draw the text with
 */
void UThumbnailLabelRenderer::DrawLabels(const TArray<FString>& Labels,
	UFont* Font,INT X,INT Y,FViewport* Viewport,FCanvas* Canvas,
	const FColor& TextColor)
{
	check(Canvas && Font);
	if (Labels.Num() > 0)
	{
		// Shift the text a touch down
		Y += STD_TNAIL_HIGHLIGHT_EDGE / 2;
		INT Ignored, LabelHeight;
		// Get the height that will be drawn
		StringSize(Font,Ignored,LabelHeight,TEXT("X"));
		// Draw each label for this thumbnail
		for (INT Index = 0; Index < Labels.Num(); Index++)
		{
			// Ignore empty strings
			if (Labels(Index).Len() > 0)
			{
				// Now draw the label
				DrawString(Canvas,X,Y,*Labels(Index),Font,FLinearColor::White);
				// And finally move to the next line
				Y += LabelHeight;
			}
		}
	}
}

/**
 * Calculates the size the thumbnail labels will be for the specified font
 *
 * @param Object the object the thumbnail is of
 * @param Font the font object to render with
 * @param Viewport the viewport that will be rendered into
 * @param RI the render interface to use for getting the size
 * @param OutWidth the var that gets the width of the labels
 * @param OutHeight the var that gets the height
 */
void UThumbnailLabelRenderer::GetThumbnailLabelSize(UObject* Object,
	UFont* Font,FViewport* Viewport,FCanvas* Canvas, const ThumbnailOptions& InOptions,
	DWORD& OutWidth,DWORD& OutHeight)
{
	TArray<FString> Labels;
	// Build the list
	BuildLabelList(Object, InOptions, Labels);
	// Use the common function for calculating the size
	GetSizeFromLabels(Labels,Font,Viewport,Canvas,OutWidth,OutHeight);
}

/**
 * Renders the thumbnail labels for the specified object with the specified
 * font and text color
 *
 * @param Object the object to render labels for
 * @param Font the font to draw with
 * @param X the X location to start drawing at
 * @param Y the Y location to start drawing at
 * @param Viewport the viewport to draw into
 * @param RI the render interface to draw with
 * @param TextColor the color to draw the text with
 */
void UThumbnailLabelRenderer::DrawThumbnailLabels(UObject* Object,UFont* Font,
	INT X,INT Y,FViewport* Viewport,FCanvas* Canvas, const ThumbnailOptions& InOptions,
	const FColor& TextColor)
{
	TArray<FString> Labels;
	// Build the list
	BuildLabelList(Object, InOptions, Labels);
	// Use the common function for drawing them
	DrawLabels(Labels,Font,X,Y,Viewport,Canvas,TextColor);
}

/**
 * Adds the name of the object and the amount of memory used to the array
 *
 * @param Object the object to build the labels for
 * @param OutLabels the array that is added to
 */
void UMemCountThumbnailLabelRenderer::BuildLabelList(UObject* Object,
	 const ThumbnailOptions& InOptions, TArray<FString>& OutLabels)
{
	// Only append the memory information if we aren't using our aggregated
	// label renderer
	if (AggregatedLabelRenderer == NULL)
	{
		// Add the name first
		new(OutLabels)FString(*Object->GetName());
	}
	else
	{
		// Use the aggregated one to build the list first. Then
		// append the memory usage afterwards
		AggregatedLabelRenderer->BuildLabelList(Object, InOptions, OutLabels);
	}

	// Retrieve resource size from object.
	FLOAT ResourceSize = Object->GetResourceSize();
	// Only list memory usage for resources supporting it correctly.
	if( ResourceSize > 0 )
	{
		// Default to using KByte.
		TCHAR* SizeDescription = TEXT("KByte");
		ResourceSize /= 1024;
		// Use MByte if we're more than one.
		if( ResourceSize > 1024 )
		{
			SizeDescription = TEXT("MByte");
			ResourceSize /= 1024;
		}
		// Add the size to the label list
		new(OutLabels)FString(FString::Printf(TEXT("%.2f %s"),ResourceSize,SizeDescription));
	}
}

/**
 * Calculates the size the thumbnail labels will be for the specified font
 * Doesn't serialize the object so that it's faster
 *
 * @param Object the object the thumbnail is of
 * @param Font the font object to render with
 * @param Viewport the viewport that will be rendered into
 * @param RI the render interface to use for getting the size
 * @param OutWidth the var that gets the width of the labels
 * @param OutHeight the var that gets the height
 */
void UMemCountThumbnailLabelRenderer::GetThumbnailLabelSize(UObject* Object,
	UFont* Font,FViewport* Viewport,FCanvas* Canvas, const ThumbnailOptions& InOptions,
	DWORD& OutWidth,DWORD& OutHeight)
{
	TArray<FString> Labels;
	// Only append the memory information if we aren't using our aggregated
	// label renderer
	if (AggregatedLabelRenderer == NULL)
	{
		// Add the name first
		new(Labels)FString(*Object->GetName());
	}
	else
	{
		// Use the aggregated one to build the list first. Then
		// append the memory usage afterwards
		AggregatedLabelRenderer->BuildLabelList(Object, InOptions, Labels);
	}
	new(Labels)FString(FString::Printf(TEXT("%.3f %c"),1024.1024f,'M'));
	// Use the common function for calculating the size
	GetSizeFromLabels(Labels,Font,Viewport,Canvas,OutWidth,OutHeight);
}

/**
 * Calculates the size the thumbnail would be at the specified zoom level
 *
 * @param Object the object the thumbnail is of
 * @param PrimType ignored
 * @param Zoom the current multiplier of size
 * @param OutWidth the var that gets the width of the thumbnail
 * @param OutHeight the var that gets the height
 */
void UTextureThumbnailRenderer::GetThumbnailSize(UObject* Object,EThumbnailPrimType,
	FLOAT Zoom,DWORD& OutWidth,DWORD& OutHeight)
{
	UTexture* Texture = Cast<UTexture>(Object);
	if (Texture != NULL)
	{
		OutWidth = appTrunc(Zoom * (FLOAT)Texture->GetSurfaceWidth());
		OutHeight = appTrunc(Zoom * (FLOAT)Texture->GetSurfaceHeight());
	}
	else
	{
		OutWidth = OutHeight = 0;
	}
}

/**
 * Draws a thumbnail for the object that was specified.
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType ignored
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void UTextureThumbnailRenderer::Draw(UObject* Object,EThumbnailPrimType,
		INT X,INT Y,DWORD Width,DWORD Height,FViewport*,
		FCanvas* Canvas,EThumbnailBackgroundType /*BackgroundType*/,
		FColor PreviewBackgroundColor,
		FColor PreviewBackgroundColorTranslucent)
{
	UTexture* Texture = Cast<UTexture>(Object);
	if (Texture != NULL)
	{
		// Use the texture interface to draw
		DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
			Texture->Resource,FALSE);
	}
}

/**
 * Calculates the size the thumbnail would be at the specified zoom level
 *
 * @param Object the object the thumbnail is of
 * @param PrimType ignored
 * @param Zoom the current multiplier of size
 * @param OutWidth the var that gets the width of the thumbnail
 * @param OutHeight the var that gets the height
 */
void UTextureCubeThumbnailRenderer::GetThumbnailSize(UObject* Object,
	EThumbnailPrimType,FLOAT Zoom,DWORD& OutWidth,DWORD& OutHeight)
{
	OutWidth = 0;
	OutHeight = 0;

	UTextureCube* CubeMap = Cast<UTextureCube>(Object);
	if (CubeMap != NULL)
	{
		for ( INT FaceIndex = 0 ; FaceIndex < 6 ; ++FaceIndex )
		{
			UTexture2D* FaceTex = CubeMap->GetFace(FaceIndex);
			if (FaceTex)
			{
				// Let the base class work on each first face.
				Super::GetThumbnailSize(FaceTex,
										TPT_Plane,
										Zoom,
										OutWidth,OutHeight);

				OutWidth *= 4;
				OutHeight *= 3;
				return;
			}
		}
	}
}

/**
 * Draws a thumbnail for the object that was specified. Uses the icon
 * that was specified as the tile
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType ignored
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void UTextureCubeThumbnailRenderer::Draw(UObject* Object,EThumbnailPrimType,
		INT X,INT Y,DWORD Width,DWORD Height,FViewport*,
		FCanvas* Canvas,EThumbnailBackgroundType /*BackgroundType*/,
		FColor PreviewBackgroundColor,
		FColor PreviewBackgroundColorTranslucent)
{
	UTextureCube* CubeMap = Cast<UTextureCube>(Object);
	if (CubeMap != NULL)
	{
		const INT WidthSize = Width / 4;
		const INT HeightSize = Height / 3;

		const INT XOffsetArray[6] = {2,0,1,1,1,3};
		const INT YOffsetArray[6] = {1,1,0,2,1,1};

		for ( INT FaceIndex = 0 ; FaceIndex < 6 ; ++FaceIndex )
		{
			UTexture2D* FaceTex = CubeMap->GetFace(FaceIndex);
			if (FaceTex)
			{
				// Let the base class work on each first face.
				Super::Draw(FaceTex,
							TPT_Plane,
							X+XOffsetArray[FaceIndex]*WidthSize,
							Y+YOffsetArray[FaceIndex]*HeightSize,
							WidthSize,HeightSize,
							NULL,
							Canvas,
							TBT_None,
							FColor(0, 0, 0),
							FColor(0, 0, 0));
			}
		}
	}
}

/**
 * Calculates the size the thumbnail would be at the specified zoom level
 * based off of the configured default sizes
 *
 * @param Object ignored
 * @param PrimType ignored
 * @param Zoom the current multiplier of size
 * @param OutWidth the var that gets the width of the thumbnail
 * @param OutHeight the var that gets the height
 */
void UDefaultSizedThumbnailRenderer::GetThumbnailSize(UObject*,
	EThumbnailPrimType,FLOAT Zoom,DWORD& OutWidth,DWORD& OutHeight)
{
	OutWidth = appTrunc(DefaultSizeX * Zoom);
	OutHeight = appTrunc(DefaultSizeY * Zoom);
}

/**
 * Draws a thumbnail for the material instance
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType The primitive type to render on
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void UMaterialInstanceThumbnailRenderer::Draw(UObject* Object,
	EThumbnailPrimType PrimType,INT X,INT Y,DWORD Width,DWORD Height,
	FViewport* Viewport,FCanvas* Canvas,EThumbnailBackgroundType BackgroundType,
	FColor PreviewBackgroundColor,
	FColor PreviewBackgroundColorTranslucent)
{
	UMaterialInterface* MatInst = Cast<UMaterialInterface>(Object);
	if (MatInst != NULL)
	{
		static FLOAT Temp = 1.55f;
		FLOAT Scale = 1.f;
		// Per primitive scaling factors, per artist request
		switch (PrimType)
		{
			case TPT_Sphere:
				Scale = 1.75f;
				break;
			case TPT_Cube:
				Scale = 1.45f;
				break;
			case TPT_Plane:
				Scale = 2.f;
				break;
			case TPT_Cylinder:
				Scale = 1.8f;
				break;
		}
		
		FMaterialThumbnailScene	ThumbnailScene(MatInst,PrimType,0,0,Scale,BackgroundType, PreviewBackgroundColor, PreviewBackgroundColorTranslucent);
		FSceneViewFamilyContext ViewFamily(Viewport,ThumbnailScene.GetScene(),(SHOW_DefaultEditor&~SHOW_ViewMode_Mask)|SHOW_ViewMode_Lit,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
		ThumbnailScene.GetView(&ViewFamily,X,Y,Width,Height);
		BeginRenderingViewFamily(Canvas,&ViewFamily);
	}
}


/**
 * Adds the name of the object and other information
 *
 * @param Object the object to build the labels for
 * @param OutLabels the array that is added to
 */
void UParticleSystemLabelRenderer::BuildLabelList(UObject* Object,
												  const ThumbnailOptions& InOptions,
												  TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());

	UParticleSystem* ParticleSystem = Cast<UParticleSystem>(Object);
	if (ParticleSystem != NULL)
	{
		if( InOptions.bShowContentTags )
		{
			// Show any content tags
			if (ParticleSystem->ContentTags.Num() > 0)
			{
				FString TagsText("Tags:");
				for (INT Idx = 0; Idx < ParticleSystem->ContentTags.Num(); Idx++)
				{
					TagsText += FString::Printf(TEXT(" %s"),*ParticleSystem->ContentTags(Idx).GetNameString());
				}
				OutLabels.AddItem(TagsText);
			}
		}
	}
}



/**
 * Calculates the size the thumbnail would be at the specified zoom level
 *
 * @param Object the object the thumbnail is of
 * @param PrimType ignored
 * @param Zoom the current multiplier of size
 * @param OutWidth the var that gets the width of the thumbnail
 * @param OutHeight the var that gets the height
 */
void UParticleSystemThumbnailRenderer::GetThumbnailSize(UObject* Object,EThumbnailPrimType,
	FLOAT Zoom,DWORD& OutWidth,DWORD& OutHeight)
{
	// Particle system thumbnails will be 1024x1024 at 100%.
	UParticleSystem* PSys = Cast<UParticleSystem>(Object);
	if (PSys != NULL)
	{
		if ((PSys->bUseRealtimeThumbnail) ||
			(PSys->ThumbnailImage) || 
			(NoImage))
		{
			OutWidth = appTrunc(1024 * Zoom);
			OutHeight = appTrunc(1024 * Zoom);
		}
		else
		{
			// Nothing valid to display
			OutWidth = OutHeight = 0;
		}
	}
	else
	{
		OutWidth = OutHeight = 0;
	}
}

/**
 * Draws a thumbnail for the particle system
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType The primitive type to render on
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void UParticleSystemThumbnailRenderer::Draw(UObject* Object,
	EThumbnailPrimType PrimType,INT X,INT Y,DWORD Width,DWORD Height,
	FViewport* Viewport,FCanvas* Canvas,EThumbnailBackgroundType /*BackgroundType*/,
	FColor PreviewBackgroundColor,
	FColor PreviewBackgroundColorTranslucent)
{
	if (GUnrealEd->GetThumbnailManager())
	{
		UParticleSystem* ParticleSystem = Cast<UParticleSystem>(Object);
		if (ParticleSystem != NULL)
		{
			if (ParticleSystem->bUseRealtimeThumbnail && (GUnrealEd->GetThumbnailManager()->bPSysRealTime == TRUE))
			{
				FParticleSystemThumbnailScene ThumbnailScene(ParticleSystem);
				FSceneViewFamilyContext ViewFamily(Viewport,ThumbnailScene.GetScene(),(SHOW_DefaultEditor&~SHOW_ViewMode_Mask)|SHOW_ViewMode_Lit,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
				ThumbnailScene.GetView(&ViewFamily,X,Y,Width,Height);
				BeginRenderingViewFamily(Canvas,&ViewFamily);
			}
			else
			if (ParticleSystem->ThumbnailImage)
			{
				DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
					ParticleSystem->ThumbnailImage->Resource,FALSE);
				if (ParticleSystem->ThumbnailImageOutOfDate == TRUE)
				{
					DrawTile(Canvas,X,Y,Width/2,Height/2,0.f,0.f,1.f,1.f,FLinearColor::White,
						OutOfDate->Resource,TRUE);
				}
			}
			else
			if (NoImage)
			{
				// Use the texture interface to draw
				DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
					NoImage->Resource,FALSE);
			}
		}
	}
}

/**
 * Draws a thumbnail for the skeletal mesh
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType The primitive type to render on
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void USkeletalMeshThumbnailRenderer::Draw(UObject* Object,
	EThumbnailPrimType PrimType,INT X,INT Y,DWORD Width,DWORD Height,
	FViewport* Viewport,FCanvas* Canvas,EThumbnailBackgroundType /*BackgroundType*/,
	FColor PreviewBackgroundColor,
	FColor PreviewBackgroundColorTranslucent)
{
	USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Object);
	if (SkeletalMesh != NULL)
	{
		FSkeletalMeshThumbnailScene	ThumbnailScene(SkeletalMesh);
		FSceneViewFamilyContext ViewFamily(Viewport,ThumbnailScene.GetScene(),(SHOW_DefaultEditor&~SHOW_ViewMode_Mask)|SHOW_ViewMode_Lit,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
		ThumbnailScene.GetView(&ViewFamily,X,Y,Width,Height);
		BeginRenderingViewFamily(Canvas,&ViewFamily);
	}
}

/**
 * Draws a thumbnail for the static mesh
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType The primitive type to render on
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void UStaticMeshThumbnailRenderer::Draw(UObject* Object,
	EThumbnailPrimType PrimType,INT X,INT Y,DWORD Width,DWORD Height,
	FViewport* Viewport,FCanvas* Canvas,EThumbnailBackgroundType /*BackgroundType*/,
	FColor PreviewBackgroundColor,
	FColor PreviewBackgroundColorTranslucent)
{
	UStaticMesh* StaticMesh = Cast<UStaticMesh>(Object);
	if (StaticMesh != NULL)
	{
		FStaticMeshThumbnailScene ThumbnailScene(StaticMesh);
		FSceneViewFamilyContext ViewFamily(Viewport,ThumbnailScene.GetScene(),(SHOW_DefaultEditor&~SHOW_ViewMode_Mask)|SHOW_ViewMode_Lit,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
		ThumbnailScene.GetView(&ViewFamily,X,Y,Width,Height);
		BeginRenderingViewFamily(Canvas,&ViewFamily);
	}
}

/**
 * Adds the name of the object and anim set to the labels
 *
 * @param Object the object to build the labels for
 * @param OutLabels the array that is added to
 */
void UAnimSetLabelRenderer::BuildLabelList(UObject* Object,
	 const ThumbnailOptions& InOptions, TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());
	// And any specific items if it's of the right type
	UAnimSet* AnimSet = Cast<UAnimSet>(Object);
	if (AnimSet != NULL)
	{
		new(OutLabels)FString(FString::Printf(TEXT("%d Sequences"),
			AnimSet->Sequences.Num()));
	}
}

/**
 * Adds the name of the object and anim tree to the labels
 *
 * @param Object the object to build the labels for
 * @param OutLabels the array that is added to
 */
void UAnimTreeLabelRenderer::BuildLabelList(UObject* Object,
	 const ThumbnailOptions& InOptions, TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());
	// And any specific items if it's of the right type
	UAnimTree* AnimTree = Cast<UAnimTree>(Object);
	if (AnimTree != NULL)
	{
		TArray<USkelControlBase*> Controls;
		TArray<UAnimNode*> Nodes;
		AnimTree->GetNodes(Nodes);
		AnimTree->GetSkelControls(Controls);
		new(OutLabels)FString(FString::Printf(TEXT("%d Nodes, %d Controls"),
			Nodes.Num(),Controls.Num()));
	}
}

/**
 * Adds the name of the object and information about the material.
 *
 * @param Object		The object to build the labels for.
 * @param OutLabels		The array that is added to.
 */
void UMaterialInstanceLabelRenderer::BuildLabelList(UObject* Object,
													const ThumbnailOptions& InOptions,
													TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());
	// And any specific items if it's of the right type
	UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(Object);
	if (MaterialInterface != NULL)
	{
		UMaterial* Material = MaterialInterface->GetMaterial();
		const EMaterialShaderPlatform QueryPlatform = Material->bIsFallbackMaterial ? MSP_SM2 : MSP_SM3;
		const FMaterialResource* MaterialResource = MaterialInterface->GetMaterialResource(QueryPlatform);

		// See if we are a MIC
		UMaterialInstance* MIC = Cast<UMaterialInstance>(MaterialInterface);
		if(MIC)
		{
			FName ParentName(NAME_None);

			if(MIC->Parent)
			{
				ParentName = MIC->Parent->GetFName();
			}

			// Display the parent of the MIC.
			FString FinalName = FString::Printf(LocalizeSecure(LocalizeUnrealEd("Parent_F"),*ParentName.ToString()));
			new(OutLabels)FString(FinalName);
		}


		if ( Material && MaterialResource )
		{
			TArray<FString> Descriptions;
			TArray<INT> InstructionCounts;
			MaterialResource->GetRepresentativeInstructionCounts(Descriptions, InstructionCounts);

			if (InstructionCounts.Num() > 0)
			{
				new(OutLabels) FString(FString::Printf(
					TEXT("%u instructions"),
					InstructionCounts(0)
					));
			}

			// Determine the number of textures used by the material instance.
			new(OutLabels) FString(FString::Printf(TEXT("%u textures"),MaterialResource->GetSamplerUsage()));

			// Determine the maximum texture dependency length.
			INT MaxTextureDependencyLength = MaterialResource->GetMaxTextureDependencyLength();

			if(MaxTextureDependencyLength > 1)
			{
				// Draw the the material's texture dependency length.
				new(OutLabels) FString(FString::Printf(
					TEXT("M.T.D.L.: %u"),
					MaxTextureDependencyLength - 1
					));
			}


			if( InOptions.bShowContentTags )
			{
				// Show any content tags
				if (Material->ContentTags.Num() > 0)
				{
					FString TagsText("Tags:");
					for (INT Idx = 0; Idx < Material->ContentTags.Num(); Idx++)
					{
						TagsText += FString::Printf(TEXT(" %s"),*Material->ContentTags(Idx).GetNameString());
					}
					OutLabels.AddItem(TagsText);
				}
			}
		}
	}
}

/**
* Adds the name of the object and anim tree to the labels
*
* @param Object the object to build the labels for
* @param OutLabels the array that is added to
*/
void UPostProcessLabelRenderer::BuildLabelList(UObject* Object,
											   const ThumbnailOptions& InOptions, 
											TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());
	// And any specific items if it's of the right type
	UPostProcessChain* Chain = Cast<UPostProcessChain>(Object);
	if (Chain != NULL)
	{
		new(OutLabels)FString(FString::Printf(TEXT("%d Nodes"),Chain->Effects.Num()));
	}
}

/**
 * Adds the name of the object and specific asset data to the labels
 *
 * @param Object the object to build the labels for
 * @param OutLabels the array that is added to
 */
void UPhysicsAssetLabelRenderer::BuildLabelList(UObject* Object,
	const ThumbnailOptions& InOptions, 
	TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());
	// And any specific items if it's of the right type
	UPhysicsAsset* PhysAsset = Cast<UPhysicsAsset>(Object);
	if (PhysAsset != NULL)
	{
		new(OutLabels)FString(FString::Printf(TEXT("%d Bodies, %d Constraints"),
			PhysAsset->BodySetup.Num(),PhysAsset->ConstraintSetup.Num()));
	}
}

/**
* Adds the name of the object and specific asset data to the labels
*
* @param Object the object to build the labels for
* @param OutLabels the array that is added to
*/
void UPhysXDestructibleLabelRenderer::BuildLabelList(UObject* Object,
													 const ThumbnailOptions& InOptions,
													 TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());
	// And any specific items if it's of the right type
	UPhysXDestructible* PhysXDestructible = Cast<UPhysXDestructible>(Object);
	if (PhysXDestructible != NULL)
	{
		INT MeshCount = 0;
		INT BodyCount = 0;
		for( INT i = 0; i < PhysXDestructible->DestructibleAssets.Num(); ++i )
		{
			MeshCount += PhysXDestructible->DestructibleAssets(i)->Meshes.Num();
			for( INT j = 0; j < PhysXDestructible->DestructibleAssets(i)->Assets.Num(); ++j )
			{
				BodyCount += PhysXDestructible->DestructibleAssets(i)->Assets(j)->BodySetup.Num();
			}
		}
		new(OutLabels)FString(FString::Printf(TEXT("%d DestructibleAssets"), PhysXDestructible->DestructibleAssets.Num()));
		new(OutLabels)FString(FString::Printf(TEXT("%d SkeletalMeshes"), MeshCount));
		new(OutLabels)FString(FString::Printf(TEXT("%d Bodies"), BodyCount));
	}
}

/**
 * Adds the name of the object and specific asset data to the labels
 *
 * @param Object the object to build the labels for
 * @param OutLabels the array that is added to
 */
void USkeletalMeshLabelRenderer::BuildLabelList(UObject* Object,
												const ThumbnailOptions& InOptions,
												TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());
	// And any specific items if it's of the right type
	USkeletalMesh* Mesh = Cast<USkeletalMesh>(Object);
	if (Mesh != NULL && Mesh->LODModels.Num() > 0)
	{
		new(OutLabels)FString(FString::Printf(TEXT("%d Triangles, %d Bones"),
			Mesh->LODModels(0).GetTotalFaces(),Mesh->RefSkeleton.Num()));
		new(OutLabels)FString(FString::Printf(TEXT("%d Chunk%s, %d Section%s"),
			Mesh->LODModels(0).Chunks.Num(), Mesh->LODModels(0).Chunks.Num()>1 ? TEXT("s") : TEXT(""),
			Mesh->LODModels(0).Sections.Num(), Mesh->LODModels(0).Sections.Num()>1 ? TEXT("s") : TEXT("") ));
	}
}

/**
 * Adds the name of the object and specific asset data to the labels
 *
 * @param Object the object to build the labels for
 * @param OutLabels the array that is added to
 */
void UStaticMeshLabelRenderer::BuildLabelList(UObject* Object,
											  const ThumbnailOptions& InOptions,
											  TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());
	// And any specific items if it's of the right type
	UStaticMesh* Mesh = Cast<UStaticMesh>(Object);
	if (Mesh != NULL)
	{
		if (Mesh->LODModels.Num() == 1) 
		{
			new(OutLabels)FString(FString::Printf(TEXT("%d Tris, %d Verts"),
				//@todo joeg Make sure to handle something other than Trilists
				Mesh->LODModels(0).IndexBuffer.Indices.Num() / 3,
				Mesh->LODModels(0).NumVertices));
		} 
		else
		{
			new(OutLabels)FString(FString::Printf(TEXT("%d LODs"), Mesh->LODModels.Num()));
			for (INT i = 0; i < Mesh->LODModels.Num(); i++)
			{
				new(OutLabels)FString(FString::Printf(TEXT("LOD %d: %d Tris, %d Verts"),
					i,
					Mesh->LODModels(i).IndexBuffer.Indices.Num() / 3,
					Mesh->LODModels(i).NumVertices));
			}

		}

		// Add the collision model warnings
   		if (!Mesh->BodySetup)
   		{
   			new(OutLabels)FString(TEXT("NO COLLISION MODEL!"));
   		}
		else if(Mesh->BodySetup->AggGeom.ConvexElems.Num() > 20)
		{
			new(OutLabels)FString(FString::Printf(TEXT("%d COLLISION PRIMS!"), Mesh->BodySetup->AggGeom.ConvexElems.Num()));
		}

		if( InOptions.bShowContentTags )
		{
			// Show any content tags
			if (Mesh->ContentTags.Num() > 0)
			{
				FString TagsText("Tags:");
				for (INT Idx = 0; Idx < Mesh->ContentTags.Num(); Idx++)
				{
					TagsText += FString::Printf(TEXT(" %s"),*Mesh->ContentTags(Idx).GetNameString());
				}
				OutLabels.AddItem(TagsText);
			}
		}
	}
}

/** Add number of chunks for this FracturedStaticMesh. */
void UFracturedStaticMeshLabelRenderer::BuildLabelList(UObject* Object, const ThumbnailOptions& InOptions, TArray<FString>& OutLabels)
{
	Super::BuildLabelList(Object, InOptions, OutLabels);

	UFracturedStaticMesh* FracMesh = Cast<UFracturedStaticMesh>(Object);
	if(FracMesh)
	{
		// Show if we have a core
		if(FracMesh->GetCoreFragmentIndex() != INDEX_NONE)
		{
			new(OutLabels)FString(FString::Printf(TEXT("%d Fragments - Has Core"), FracMesh->GetNumFragments()));
		}
		else
		{
			new(OutLabels)FString(FString::Printf(TEXT("%d Fragments"), FracMesh->GetNumFragments()));
		}

		if (FracMesh->NonCriticalBuildVersion < FSMNonCriticalBuildVersion
			|| FracMesh->LicenseeNonCriticalBuildVersion < LicenseeFSMNonCriticalBuildVersion)
		{
			new(OutLabels) FString(TEXT("NEEDS RESLICING!"));
		}

		// Assuming a non-zero LightMapCoordinateIndex means the mesh has lightmap UV's
		if (FracMesh->LightMapCoordinateIndex > 0)
		{
			new(OutLabels) FString(FString::Printf(TEXT("Has Lightmap UV's, Res=%u"), FracMesh->LightMapResolution));
		}
		else
		{
			new(OutLabels) FString(TEXT("MISSING LIGHTMAP UV'S"));
		}
	}
}

/**
 * Adds the name of the object and specific asset data to the labels
 *
 * @param Object the object to build the labels for
 * @param OutLabels the array that is added to
 */
void USoundLabelRenderer::BuildLabelList(UObject* Object,
	const ThumbnailOptions& InOptions,
	TArray<FString>& OutLabels)
{
	// Add the name
	new(OutLabels)FString(*Object->GetName());

	// Add the duration, sound group name and other type information.
	USoundNodeWave*		SoundNodeWave		= Cast<USoundNodeWave>(Object);
	USoundCue*			SoundCue			= Cast<USoundCue>(Object);
	if( SoundNodeWave )
	{
		new(OutLabels)FString(SoundNodeWave->GetDesc());
	}
	else if( SoundCue )
	{
		new(OutLabels)FString(SoundCue->GetDesc());
	}
}

/**
 * Render thumbnail icon for Archetype objects in a package.
 * Will not show thumbnail in browser for Archetypes which are within a Prefab.
 */

/**
 * Determines whether this thumbnail renderer supports rendering thumbnails for the specified object.
 *
 * @param Object 			the object to inspect
 * @param bCheckObjectState	TRUE indicates that the object's state should be inspected to determine whether it can be supported;
 *							FALSE indicates that only the object's type should be considered (for caching purposes)
 *
 * @return	TRUE if the object is supported by this renderer (basically must be an archetype)
 */
UBOOL UArchetypeThumbnailRenderer::SupportsThumbnailRendering(UObject* Object,UBOOL bCheckObjectState/*=TRUE*/)
{
	if ( Object == NULL )
	{
		return FALSE;
	}

	// if we are only checking whether this renderer is the most appropriate for the specified object's class, don't worry about whether
	// the object is an archetype or not - this renderer supports every object type
	if ( !bCheckObjectState )
	{
		return TRUE;
	}

	if ( Object->HasAnyFlags(RF_ArchetypeObject) )
	{
		// See if this Archetype is 'within' a Prefab or another Archetype. If so - don't show it.
		UBOOL bWithinPrefabOrArchetype = Object->IsAPrefabArchetype();
		UObject* OuterObj;
		for ( OuterObj = Object->GetOuter(); !bWithinPrefabOrArchetype && OuterObj; OuterObj = OuterObj->GetOuter() )
		{
			bWithinPrefabOrArchetype = OuterObj->HasAnyFlags(RF_ArchetypeObject);
		}

		// If this is an Archetype object, but isn't part of a Prefab or another Archetype, then show it.
		return !bWithinPrefabOrArchetype;
	}

	return FALSE;
}

/**
* Draws a thumbnail for the prefab preview
*
* @param Object the object to draw the thumbnail for
* @param PrimType ignored
* @param X the X coordinate to start drawing at
* @param Y the Y coordinate to start drawing at
* @param Width the width of the thumbnail to draw
* @param Height the height of the thumbnail to draw
* @param Viewport ignored
* @param Canvas the render interface to draw with
* @param BackgroundType type of background for the thumbnail
* @param PreviewBackgroundColor background color for material previews
* @param PreviewBackgroundColorTranslucent background color for translucent material previews
*/
void UPrefabThumbnailRenderer::Draw( UObject* Object,EThumbnailPrimType,
										INT X,INT Y,DWORD Width,DWORD Height,FViewport*,
										FCanvas* Canvas,EThumbnailBackgroundType /*BackgroundType*/,
										FColor PreviewBackgroundColor,
										FColor PreviewBackgroundColorTranslucent)
{
	UPrefab* Prefab = Cast<UPrefab>(Object);
	if (Prefab != NULL)
	{
		// If we have a preview texture, render that.
		if(Prefab->PrefabPreview)
		{
			// Use the texture interface to draw
			DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
				Prefab->PrefabPreview->Resource,FALSE);
		}
		// If not, just render 'Prefab' icon.
		else
		{
			DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
				GetIcon()->Resource,FALSE);
		}
	}
}

/**
 * Calculates the size the thumbnail would be at the specified zoom level for
 * the font texture data
 *
 * @param Object the object the thumbnail is of
 * @param PrimType ignored
 * @param Zoom the current multiplier of size
 * @param OutWidth the var that gets the width of the thumbnail
 * @param OutHeight the var that gets the height
 */
void UFontThumbnailRenderer::GetThumbnailSize(UObject* Object,EThumbnailPrimType,
	FLOAT Zoom,DWORD& OutWidth,DWORD& OutHeight)
{
	UFont* Font = Cast<UFont>(Object);
	if (Font != NULL && Font->Textures(0) != NULL)
	{
		// Get the texture interface for the font text
		UTexture2D* Tex = Font->Textures(0);
		OutWidth = appTrunc(Zoom * (FLOAT)Tex->GetSurfaceWidth());
		OutHeight = appTrunc(Zoom * (FLOAT)Tex->GetSurfaceHeight());
	}
	else
	{
		OutWidth = OutHeight = 0;
	}
}

/**
 * Draws a thumbnail for the font that was specified. Uses the first font page
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType ignored
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void UFontThumbnailRenderer::Draw(UObject* Object,EThumbnailPrimType,
	INT X,INT Y,DWORD Width,DWORD Height,FViewport*,
	FCanvas* Canvas,EThumbnailBackgroundType /*BackgroundType*/,
	FColor PreviewBackgroundColor,
	FColor PreviewBackgroundColorTranslucent)
{
	UFont* Font = Cast<UFont>(Object);
	if (Font != NULL && Font->Textures(0) != NULL)
	{
		// Use the texture interface to draw the first page of font data
		DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
			Font->Textures(0)->Resource,TRUE);
	}
}

/**
 * Draws a thumbnail for the font that was specified. Uses the first font page
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType ignored
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void UUISceneThumbnailRenderer::Draw(UObject* Object,EThumbnailPrimType PrimType,
	INT X,INT Y,DWORD Width,DWORD Height,FViewport* Viewport,
	FCanvas* Canvas,EThumbnailBackgroundType BackgroundType,
	FColor PreviewBackgroundColor,
	FColor PreviewBackgroundColorTranslucent)
{
	UUIScene* Scene = Cast<UUIScene>(Object);

	if(Scene != NULL)
	{
		// If we have a preview texture, render that.
		if(Scene->ScenePreview)
		{
			// Use the texture interface to draw
			DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
				Scene->ScenePreview->Resource,FALSE);
		}
		// If not, just render a icon.
		else
		{
			Super::Draw(Object, PrimType, X, Y, Width, Height, Viewport, Canvas, BackgroundType, FColor(0, 0, 0), FColor(0, 0, 0));
		}
	}
}

/**
 *	ULensFlareThumbnailRenderer
 */
/**
 * Calculates the size the thumbnail would be at the specified zoom level
 *
 * @param Object the object the thumbnail is of
 * @param PrimType ignored
 * @param Zoom the current multiplier of size
 * @param OutWidth the var that gets the width of the thumbnail
 * @param OutHeight the var that gets the height
 */
void ULensFlareThumbnailRenderer::GetThumbnailSize(UObject* Object,EThumbnailPrimType, FLOAT Zoom,DWORD& OutWidth,DWORD& OutHeight)
{
	// Particle system thumbnails will be 1024x1024 at 100%.
	ULensFlare* LensFlare = Cast<ULensFlare>(Object);
	if (LensFlare != NULL)
	{
		if (LensFlare->ThumbnailImage || NoImage)
		{
			OutWidth = appTrunc(512 * Zoom);
			OutHeight = appTrunc(512 * Zoom);
		}
		else
		{
			// Nothing valid to display
			OutWidth = OutHeight = 0;
		}
	}
	else
	{
		OutWidth = OutHeight = 0;
	}
}

/**
 * Draws a thumbnail for the object that was specified
 *
 * @param Object the object to draw the thumbnail for
 * @param PrimType ignored
 * @param X the X coordinate to start drawing at
 * @param Y the Y coordinate to start drawing at
 * @param Width the width of the thumbnail to draw
 * @param Height the height of the thumbnail to draw
 * @param Viewport ignored
 * @param Canvas the render interface to draw with
 * @param BackgroundType type of background for the thumbnail
 * @param PreviewBackgroundColor background color for material previews
 * @param PreviewBackgroundColorTranslucent background color for translucent material previews
 */
void ULensFlareThumbnailRenderer::Draw(UObject* Object,EThumbnailPrimType,INT X,INT Y,
									   DWORD Width,DWORD Height,FViewport*,FCanvas* Canvas, EThumbnailBackgroundType BackgroundType,
									   FColor PreviewBackgroundColor,
									   FColor PreviewBackgroundColorTranslucent)
{
	if (GUnrealEd->GetThumbnailManager())
	{
		ULensFlare* LensFlare = Cast<ULensFlare>(Object);
		if (LensFlare != NULL)
		{
			if (LensFlare->ThumbnailImage)
			{
				DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
					LensFlare->ThumbnailImage->Resource,FALSE);
				if (LensFlare->ThumbnailImageOutOfDate == TRUE)
				{
					DrawTile(Canvas,X,Y,Width/2,Height/2,0.f,0.f,1.f,1.f,FLinearColor::White,
						OutOfDate->Resource,TRUE);
				}
			}
			else
			if (NoImage)
			{
				// Use the texture interface to draw
				DrawTile(Canvas,X,Y,Width,Height,0.f,0.f,1.f,1.f,FLinearColor::White,
					NoImage->Resource,FALSE);
			}
		}
	}
}
