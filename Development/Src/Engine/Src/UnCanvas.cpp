/*=============================================================================
	UnCanvas.cpp: Unreal canvas rendering.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineUserInterfaceClasses.h"
#include "ScenePrivate.h"
#include "TileRendering.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(UCanvas);

DECLARE_STATS_GROUP(TEXT("Canvas"),STATGROUP_Canvas);
DECLARE_CYCLE_STAT(TEXT("Flush Time"),STAT_Canvas_FlushTime,STATGROUP_Canvas);
DECLARE_CYCLE_STAT(TEXT("Draw Texture Tile Time"),STAT_Canvas_DrawTextureTileTime,STATGROUP_Canvas);
DECLARE_CYCLE_STAT(TEXT("Draw Material Tile Time"),STAT_Canvas_DrawMaterialTileTime,STATGROUP_Canvas);
DECLARE_CYCLE_STAT(TEXT("Draw String Time"),STAT_Canvas_DrawStringTime,STATGROUP_Canvas);
DECLARE_CYCLE_STAT(TEXT("Get Batched Element Time"),STAT_Canvas_GetBatchElementsTime,STATGROUP_Canvas);
DECLARE_CYCLE_STAT(TEXT("Add Material Tile Time"),STAT_Canvas_AddTileRenderTime,STATGROUP_Canvas);
DECLARE_DWORD_COUNTER_STAT(TEXT("Num Batches Created"),STAT_Canvas_NumBatchesCreated,STATGROUP_Canvas);

FCanvas::FCanvas(FRenderTarget* InRenderTarget,FHitProxyConsumer* InHitProxyConsumer)
:	RenderTarget(InRenderTarget)
,	bEnableDepthTest(FALSE)
,	bRenderTargetDirty(FALSE)
,	HitProxyConsumer(InHitProxyConsumer)
,	AllowedModes(-1)
{
	check(RenderTarget);
	// Push the viewport transform onto the stack.  Default to using a 2D projection. 
	new(TransformStack) FTransformEntry( 
		FMatrix( CalcBaseTransform2D(RenderTarget->GetSizeX(),RenderTarget->GetSizeY()) ) 
		);
	// init alpha to 1
	AlphaModulate=1.0;

	// init sort key to 0
	PushDepthSortKey(0);
}

/**
* Replace the base (ie. TransformStack(0)) transform for the canvas with the given matrix
*
* @param Transform - The transform to use for the base
*/
void FCanvas::SetBaseTransform(const FMatrix& Transform)
{
	// set the base transform
	if( TransformStack.Num() > 0 )
	{
		TransformStack(0).SetMatrix(Transform);
	}
	else
	{
		new(TransformStack) FTransformEntry(Transform);
	}
}

/**
* Generate a 2D projection for the canvas. Use this if you only want to transform in 2D on the XY plane
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcBaseTransform2D(UINT ViewSizeX, UINT ViewSizeY)
{
	return 
		FTranslationMatrix(FVector(-GPixelCenterOffset,-GPixelCenterOffset,0)) *
		FMatrix(
			FPlane(	1.0f / (ViewSizeX / 2.0f),	0.0,										0.0f,	0.0f	),
			FPlane(	0.0f,						-1.0f / (ViewSizeY / 2.0f),					0.0f,	0.0f	),
			FPlane(	0.0f,						0.0f,										1.0f,	0.0f	),
			FPlane(	-1.0f,						1.0f,										0.0f,	1.0f	)
			);
}

/**
* Generate a 3D projection for the canvas. Use this if you want to transform in 3D 
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @param NearPlane - Distance to the near clip plane
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcBaseTransform3D(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane)
{
	FMatrix ViewMat(CalcViewMatrix(ViewSizeX,ViewSizeY,fFOV));
	FMatrix ProjMat(CalcProjectionMatrix(ViewSizeX,ViewSizeY,fFOV,NearPlane));
	return ViewMat * ProjMat;
}

/**
* Generate a view matrix for the canvas. Used for CalcBaseTransform3D
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @return Matrix for canvas view orientation
*/
FMatrix FCanvas::CalcViewMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV)
{
	// convert FOV to randians
	FLOAT FOVRad = fFOV * (FLOAT)PI / 360.0f;
	// move camera back enough so that the canvas items being rendered are at the same screen extents as regular canvas 2d rendering	
	FTranslationMatrix CamOffsetMat(-FVector(0,0,-appTan(FOVRad)*ViewSizeX/2));
	// adjust so that canvas items render as if they start at [0,0] upper left corner of screen 
	// and extend to the lower right corner [ViewSizeX,ViewSizeY]. 
	FMatrix OrientCanvasMat(
		FPlane(	1.0f,				0.0f,				0.0f,	0.0f	),
		FPlane(	0.0f,				-1.0f,				0.0f,	0.0f	),
		FPlane(	0.0f,				0.0f,				1.0f,	0.0f	),
		FPlane(	ViewSizeX * -0.5f,	ViewSizeY * 0.5f,	0.0f, 1.0f		)
		);
	return 
		// also apply screen offset to align to pixel centers
		FTranslationMatrix(FVector(-GPixelCenterOffset,-GPixelCenterOffset,0)) * 
		OrientCanvasMat * 
		CamOffsetMat;
}

/**
* Generate a projection matrix for the canvas. Used for CalcBaseTransform3D
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @param NearPlane - Distance to the near clip plane
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcProjectionMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane)
{
	// convert FOV to randians
	FLOAT FOVRad = fFOV * (FLOAT)PI / 360.0f;
	// project based on the FOV and near plane given
	return FPerspectiveMatrix(
		FOVRad,
		ViewSizeX,
		ViewSizeY,
		NearPlane
		);
}

/**
* Base interface for canvas items which can be batched for rendering
*/
class FCanvasBaseRenderItem
{
public:
	virtual ~FCanvasBaseRenderItem()
	{}

	/**
	* Renders the canvas item
	*
	* @param Canvas - canvas currently being rendered
	* @return TRUE if anything rendered
	*/
	virtual UBOOL Render( const FCanvas* Canvas ) =0;
	/**
	* FCanvasBatchedElementRenderItem instance accessor
	*
	* @return FCanvasBatchedElementRenderItem instance
	*/
	virtual class FCanvasBatchedElementRenderItem* GetCanvasBatchedElementRenderItem() { return NULL; }
	/**
	* FCanvasTileRendererItem instance accessor
	*
	* @return FCanvasTileRendererItem instance
	*/
	virtual class FCanvasTileRendererItem* GetCanvasTileRendererItem() { return NULL; }
};

/**
* Info needed to render a batched element set
*/
class FCanvasBatchedElementRenderItem : public FCanvasBaseRenderItem
{
public:
	/** 
	* Init constructor 
	*/
	FCanvasBatchedElementRenderItem(
		const FTexture* InTexture=NULL,
		EBlendMode InBlendMode=BLEND_MAX,
		FCanvas::EElementType InElementType=FCanvas::ET_MAX,
		const FCanvas::FTransformEntry& InTransform=FCanvas::FTransformEntry(FMatrix::Identity) )
		// this data is deleted after rendering has completed
		: Data( new FRenderData(InTexture,InBlendMode,InElementType,InTransform) )
	{}

	/**
	* Destructor to delete data in case nothing rendered
	*/
	virtual ~FCanvasBatchedElementRenderItem()
	{
		delete Data;
	}

	/**
	* FCanvasBatchedElementRenderItem instance accessor
	*
	* @return this instance
	*/
	virtual class FCanvasBatchedElementRenderItem* GetCanvasBatchedElementRenderItem() 
	{ 
		return this; 
	}
	
	/**
	* Renders the canvas item. 
	* Iterates over all batched elements and draws them with their own transforms
	*
	* @param Canvas - canvas currently being rendered
	* @return TRUE if anything rendered
	*/
	virtual UBOOL Render( const FCanvas* Canvas );

	/**
	* Determine if this is a matching set by comparing texture,blendmode,elementype,transform. All must match
	*
	* @param InTexture - texture resource for the item being rendered
	* @param InBlendMode - current alpha blend mode 
	* @param InElementType - type of item being rendered: triangle,line,etc
	* @param InTransform - the transform for the item being rendered
	* @return TRUE if the parameters match this render item
	*/
	UBOOL IsMatch( const FTexture* InTexture, EBlendMode InBlendMode, FCanvas::EElementType InElementType, const FCanvas::FTransformEntry& InTransform )
	{
		return(	Data->Texture == InTexture &&
				Data->BlendMode == InBlendMode &&
				Data->ElementType == InElementType &&
				Data->Transform.GetMatrixCRC() == InTransform.GetMatrixCRC() );
	}

	/**
	* Accessor for the batched elements. This can be used for adding triangles and primitives to the batched elements
	*
	* @return pointer to batched elements struct
	*/
	FORCEINLINE FBatchedElements* GetBatchedElements()
	{
		return &Data->BatchedElements;
	}

private:
	class FRenderData
	{
	public:
		/**
		* Init constructor
		*/
		FRenderData(
			const FTexture* InTexture=NULL,
			EBlendMode InBlendMode=BLEND_MAX,
			FCanvas::EElementType InElementType=FCanvas::ET_MAX,
			const FCanvas::FTransformEntry& InTransform=FCanvas::FTransformEntry(FMatrix::Identity) )
			:	Texture(InTexture)
			,	BlendMode(InBlendMode)
			,	ElementType(InElementType)
			,	Transform(InTransform)
		{}
		/** Current batched elements, destroyed once rendering completes. */
		FBatchedElements BatchedElements;
		/** Current texture being used for batching, set to NULL if it hasn't been used yet. */
		const FTexture* Texture;
		/** Current blend mode being used for batching, set to BLEND_MAX if it hasn't been used yet. */
		EBlendMode BlendMode;
		/** Current element type being used for batching, set to ET_MAX if it hasn't been used yet. */
		FCanvas::EElementType ElementType;
		/** Transform used to render including projection */
		FCanvas::FTransformEntry Transform;
	};
	/**
	* Render data which is allocated when a new FCanvasBatchedElementRenderItem is added for rendering.
	* This data is only freed on the rendering thread once the item has finished rendering
	*/
	FRenderData* Data;		
};

/**
* Renders the canvas item. 
* Iterates over all batched elements and draws them with their own transforms
*
* @param Canvas - canvas currently being rendered
* @return TRUE if anything rendered
*/
UBOOL FCanvasBatchedElementRenderItem::Render( const FCanvas* Canvas )
{	
	checkSlow(Data);
	UBOOL bDirty=FALSE;		
	if( Data->BatchedElements.HasPrimsToDraw() )
	{
		bDirty = TRUE;

		// current render target set for the canvas
		const FRenderTarget* CanvasRenderTarget = Canvas->GetRenderTarget();
		FLOAT Gamma = 1.0f / CanvasRenderTarget->GetDisplayGamma();
		if ( Data->Texture && Data->Texture->bIgnoreGammaConversions )
		{
			Gamma = 1.0f;
		}

		// this allows us to use FCanvas operations from the rendering thread (ie, render subtitles
		// on top of a movie that is rendered completely in rendering thread)
		if (IsInRenderingThread())
		{
			SCOPED_DRAW_EVENT(EventUIBatchFromRT)(DEC_SCENE_ITEMS,TEXT("UI Texture Draw [RT]"));
			// draw batched items
			Data->BatchedElements.Draw(
				Data->Transform.GetMatrix(),
				CanvasRenderTarget->GetSizeX(),
				CanvasRenderTarget->GetSizeY(),
				Canvas->IsHitTesting(),
				Gamma
				);

			if( Canvas->GetAllowedModes() & FCanvas::Allow_DeleteOnRender )
			{
				// delete data since we're done rendering it
				delete Data;
			}
		}
		else
		{
			// Render the batched elements.
			struct FBatchedDrawParameters
			{
				FRenderData* RenderData;
				BITFIELD bHitTesting : 1;
				UINT ViewportSizeX;
				UINT ViewportSizeY;
				FLOAT DisplayGamma;
				DWORD AllowedCanvasModes;
			};
			// all the parameters needed for rendering
			FBatchedDrawParameters DrawParameters =
			{
				Data,
				Canvas->IsHitTesting(),
				CanvasRenderTarget->GetSizeX(),
				CanvasRenderTarget->GetSizeY(),
				Gamma,
				Canvas->GetAllowedModes()
			};
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				BatchedDrawCommand,
				FBatchedDrawParameters,Parameters,DrawParameters,
			{
				SCOPED_DRAW_EVENT(EventUIBatchFromGT)(DEC_SCENE_ITEMS,TEXT("UI Texture Draw [GT]"));

				// draw batched items
				Parameters.RenderData->BatchedElements.Draw(
					Parameters.RenderData->Transform.GetMatrix(),
					Parameters.ViewportSizeX,
					Parameters.ViewportSizeY,
					Parameters.bHitTesting,
					Parameters.DisplayGamma
					);
				if( Parameters.AllowedCanvasModes & FCanvas::Allow_DeleteOnRender )
				{
					delete Parameters.RenderData;
				}
			});
		}
	}
	if( Canvas->GetAllowedModes() & FCanvas::Allow_DeleteOnRender )
	{
		Data = NULL;
	}
	return bDirty;
}

/**
* Info needed to render a single FTileRenderer
*/
class FCanvasTileRendererItem : public FCanvasBaseRenderItem
{
public:
	/** 
	* Init constructor 
	*/
	FCanvasTileRendererItem( 
		const FMaterialRenderProxy* InMaterialRenderProxy=NULL,
		const FCanvas::FTransformEntry& InTransform=FCanvas::FTransformEntry(FMatrix::Identity) )
		// this data is deleted after rendering has completed
		:	Data(new FRenderData(InMaterialRenderProxy,InTransform))	
	{}

	/**
	* Destructor to delete data in case nothing rendered
	*/
	virtual ~FCanvasTileRendererItem()
	{
		delete Data;
	}

	/**
	* FCanvasTileRendererItem instance accessor
	*
	* @return this instance
	*/
	virtual class FCanvasTileRendererItem* GetCanvasTileRendererItem() 
	{ 
		return this; 
	}

	/**
	* Renders the canvas item. 
	* Iterates over each tile to be rendered and draws it with its own transforms
	*
	* @param Canvas - canvas currently being rendered
	* @return TRUE if anything rendered
	*/
	virtual UBOOL Render( const FCanvas* Canvas );

	/**
	* Determine if this is a matching set by comparing material,transform. All must match
	*
	* @param IInMaterialRenderProxy - material proxy resource for the item being rendered
	* @param InTransform - the transform for the item being rendered
	* @return TRUE if the parameters match this render item
	*/
	UBOOL IsMatch( const FMaterialRenderProxy* InMaterialRenderProxy, const FCanvas::FTransformEntry& InTransform )
	{
		return( Data->MaterialRenderProxy == InMaterialRenderProxy && 
				Data->Transform.GetMatrixCRC() == InTransform.GetMatrixCRC() );
	};

	/**
	* Add a new tile to the render data. These tiles all use the same transform and material proxy
	*
	* @param X - tile X offset
	* @param Y - tile Y offset
	* @param SizeX - tile X size
	* @param SizeY - tile Y size
	* @param U - tile U offset
	* @param V - tile V offset
	* @param SizeU - tile U size
	* @param SizeV - tile V size
	* @param return number of tiles added
	*/
	FORCEINLINE INT AddTile(FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,FHitProxyId HitProxyId)
	{
		return Data->AddTile(X,Y,SizeX,SizeY,U,V,SizeU,SizeV,HitProxyId);
	};

private:
	class FRenderData
	{
	public:
		FRenderData(
			const FMaterialRenderProxy* InMaterialRenderProxy=NULL,
			const FCanvas::FTransformEntry& InTransform=FCanvas::FTransformEntry(FMatrix::Identity) )
			:	MaterialRenderProxy(InMaterialRenderProxy)
			,	Transform(InTransform)
		{}
		const FMaterialRenderProxy* MaterialRenderProxy;
		FCanvas::FTransformEntry Transform;

		struct FTileInst
		{
			FLOAT X,Y;
			FLOAT SizeX,SizeY;
			FLOAT U,V;
			FLOAT SizeU,SizeV;
			FHitProxyId HitProxyId;
		};
		TArray<FTileInst> Tiles;

		FORCEINLINE INT AddTile(FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,FHitProxyId HitProxyId)
		{
			FTileInst NewTile = {X,Y,SizeX,SizeY,U,V,SizeU,SizeV,HitProxyId};
			return Tiles.AddItem(NewTile);
		};
	};
	/**
	* Render data which is allocated when a new FCanvasTileRendererItem is added for rendering.
	* This data is only freed on the rendering thread once the item has finished rendering
	*/
	FRenderData* Data;		
};

/**
* Renders the canvas item. 
* Iterates over each tile to be rendered and draws it with its own transforms
*
* @param Canvas - canvas currently being rendered
* @return TRUE if anything rendered
*/
UBOOL FCanvasTileRendererItem::Render( const FCanvas* Canvas )
{
	checkSlow(Data);
	// current render target set for the canvas
	const FRenderTarget* CanvasRenderTarget = Canvas->GetRenderTarget();
	FSceneViewFamily* ViewFamily = new FSceneViewFamily(
		CanvasRenderTarget,
		NULL,
		SHOW_DefaultGame,
		GWorld->GetTimeSeconds(),
		GWorld->GetDeltaSeconds(),
		GWorld->GetRealTimeSeconds(),
		NULL,FALSE,FALSE,FALSE,TRUE,TRUE,
		CanvasRenderTarget->GetDisplayGamma()
		);

	// make a temporary view
	FViewInfo* View = new FViewInfo(ViewFamily, 
		NULL, 
		-1,
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL,
		NULL, 
		NULL, 
		0, 
		0, 
		CanvasRenderTarget->GetSizeX(), 
		CanvasRenderTarget->GetSizeY(), 
		FMatrix::Identity, 
		Data->Transform.GetMatrix(), 
		FLinearColor::Black, 
		FLinearColor::White, 
		FLinearColor::White, 
		TSet<UPrimitiveComponent*>()
		);
	// Render the batched elements.
	if( IsInRenderingThread() )
	{
		SCOPED_DRAW_EVENT(EventUIDrawMatFromRT)(DEC_SCENE_ITEMS,TEXT("UI Material Draw [RT]"));

		FTileRenderer TileRenderer;

		for( INT TileIdx=0; TileIdx < Data->Tiles.Num(); TileIdx++ )
		{
			const FRenderData::FTileInst& Tile = Data->Tiles(TileIdx);
			TileRenderer.DrawTile(
				*View, 
				Data->MaterialRenderProxy, 
				Tile.X, Tile.Y, Tile.SizeX, Tile.SizeY, 
				Tile.U, Tile.V, Tile.SizeU, Tile.SizeV,
				Canvas->IsHitTesting(), Tile.HitProxyId
				);
		}

		delete View->Family;
		delete View;
		if( Canvas->GetAllowedModes() & FCanvas::Allow_DeleteOnRender )
		{
			delete Data;
		}
	}
	else
	{
		struct FDrawTileParameters
		{
			FViewInfo* View;
			FRenderData* RenderData;
			BITFIELD bIsHitTesting : 1;
			DWORD AllowedCanvasModes;
		};
		FDrawTileParameters DrawTileParameters =
		{
			View,
			Data,
			Canvas->IsHitTesting(),
			Canvas->GetAllowedModes()
		};
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			DrawTileCommand,
			FDrawTileParameters,Parameters,DrawTileParameters,
		{
			SCOPED_DRAW_EVENT(EventUIDrawMatFromGT)(DEC_SCENE_ITEMS,TEXT("UI Material Draw [GT]"));

			FTileRenderer TileRenderer;

			for( INT TileIdx=0; TileIdx < Parameters.RenderData->Tiles.Num(); TileIdx++ )
			{
				const FRenderData::FTileInst& Tile = Parameters.RenderData->Tiles(TileIdx);
				TileRenderer.DrawTile(
					*Parameters.View, 
					Parameters.RenderData->MaterialRenderProxy, 
					Tile.X, Tile.Y, Tile.SizeX, Tile.SizeY, 
					Tile.U, Tile.V, Tile.SizeU, Tile.SizeV,
					Parameters.bIsHitTesting, Tile.HitProxyId
					);
			}

			delete Parameters.View->Family;
			delete Parameters.View;
			if( Parameters.AllowedCanvasModes & FCanvas::Allow_DeleteOnRender )
			{
				delete Parameters.RenderData;
			}
		});
	}
	if( Canvas->GetAllowedModes() & FCanvas::Allow_DeleteOnRender )
	{
		Data = NULL;
	}
	return TRUE;
}

/**
* Get the sort element for the given sort key. Allocates a new entry if one does not exist
*
* @param DepthSortKey - the key used to find the sort element entry
* @return sort element entry
*/
FCanvas::FCanvasSortElement& FCanvas::GetSortElement(INT DepthSortKey)
{
	// find the FCanvasSortElement array entry based on the sortkey
	INT ElementIdx = INDEX_NONE;
	INT* ElementIdxFromMap = SortedElementLookupMap.Find(DepthSortKey);
	if( ElementIdxFromMap )
	{
		ElementIdx = *ElementIdxFromMap;
		checkSlow( SortedElements.IsValidIndex(ElementIdx) );
	}	
	// if it doesn't exist then add a new entry (no duplicates allowed)
	else
	{
		new(SortedElements) FCanvasSortElement(DepthSortKey);
		ElementIdx = SortedElements.Num()-1;
		// keep track of newly added array index for later lookup
		SortedElementLookupMap.Set( DepthSortKey, ElementIdx );
	}
	return SortedElements(ElementIdx);
}

/**
* Returns a FBatchedElements pointer to be used for adding vertices and primitives for rendering.
* Adds a new render item to the sort element entry based on the current sort key.
*
* @param InElementType - Type of element we are going to draw.
* @param InTexture - New texture that will be set.
* @param InBlendMode - New blendmode that will be set.	* 
* @return Returns a pointer to a FBatchedElements object.
*/
FBatchedElements* FCanvas::GetBatchedElements(EElementType InElementType, const FTexture* InTexture, EBlendMode InBlendMode)
{
	SCOPE_CYCLE_COUNTER(STAT_Canvas_GetBatchElementsTime);

	// get sort element based on the current sort key from top of sort key stack
	FCanvasSortElement& SortElement = FCanvas::GetSortElement(TopDepthSortKey());
	// find a batch to use 
	FCanvasBatchedElementRenderItem* RenderBatch = NULL;
	// get the current transform entry from top of transform stack
	const FTransformEntry& TopTransformEntry = TransformStack.Top();

	// try to use the current top entry in the render batch array
	if( SortElement.RenderBatchArray.Num() > 0 )
	{
		checkSlow( SortElement.RenderBatchArray.Last() );
		RenderBatch = SortElement.RenderBatchArray.Last()->GetCanvasBatchedElementRenderItem();
	}	
	// if a matching entry for this batch doesn't exist then allocate a new entry
	if( RenderBatch == NULL ||		
		!RenderBatch->IsMatch(InTexture,InBlendMode,InElementType,TopTransformEntry) )
	{
		INC_DWORD_STAT(STAT_Canvas_NumBatchesCreated);

		RenderBatch = new FCanvasBatchedElementRenderItem( InTexture,InBlendMode,InElementType,TopTransformEntry );
		SortElement.RenderBatchArray.AddItem(RenderBatch);
	}
	return RenderBatch->GetBatchedElements();
}

/**
* Generates a new FCanvasTileRendererItem for the current sortkey and adds it to the sortelement list of itmes to render
*/
void FCanvas::AddTileRenderItem(FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,const FMaterialRenderProxy* MaterialRenderProxy,FHitProxyId HitProxyId)
{
	SCOPE_CYCLE_COUNTER(STAT_Canvas_AddTileRenderTime);

	// get sort element based on the current sort key from top of sort key stack
	FCanvasSortElement& SortElement = FCanvas::GetSortElement(TopDepthSortKey());
	// find a batch to use 
	FCanvasTileRendererItem* RenderBatch = NULL;
	// get the current transform entry from top of transform stack
	const FTransformEntry& TopTransformEntry = TransformStack.Top();	

	// try to use the current top entry in the render batch array
	if( SortElement.RenderBatchArray.Num() > 0 )
	{
		checkSlow( SortElement.RenderBatchArray.Last() );
		RenderBatch = SortElement.RenderBatchArray.Last()->GetCanvasTileRendererItem();
	}	
	// if a matching entry for this batch doesn't exist then allocate a new entry
	if( RenderBatch == NULL ||		
		!RenderBatch->IsMatch(MaterialRenderProxy,TopTransformEntry) )
	{
		INC_DWORD_STAT(STAT_Canvas_NumBatchesCreated);

		RenderBatch = new FCanvasTileRendererItem( MaterialRenderProxy,TopTransformEntry );
		SortElement.RenderBatchArray.AddItem(RenderBatch);
	}
	// add the quad to the tile render batch
	RenderBatch->AddTile( X,Y,SizeX,SizeY,U,V,SizeU,SizeV,HitProxyId);
}

/**
 * Setup the current masked region during flush
 */
void FCanvas::FlushSetMaskRegion()
{
	if( !(AllowedModes&Allow_MaskedRegions) ) 
	{ 
		return; 
	}

	FMaskRegion MaskRegion = GetCurrentMaskRegion();
	checkSlow(MaskRegion.IsValid());

	// create a batch for rendering the masked region 
	FBatchedElements* BatchedElements = new FBatchedElements();
	const FVector2D ZeroUV(0,0);
	const FLinearColor& Color = FLinearColor::White;
	INT V00 = BatchedElements->AddVertex(FVector4(MaskRegion.X,MaskRegion.Y,0,1), ZeroUV, Color,FHitProxyId());
	INT V10 = BatchedElements->AddVertex(FVector4(MaskRegion.X + MaskRegion.SizeX,MaskRegion.Y,0,1),ZeroUV,	Color,FHitProxyId());
	INT V01 = BatchedElements->AddVertex(FVector4(MaskRegion.X,MaskRegion.Y + MaskRegion.SizeY,0,1),ZeroUV,	Color,FHitProxyId());
	INT V11 = BatchedElements->AddVertex(FVector4(MaskRegion.X + MaskRegion.SizeX,MaskRegion.Y + MaskRegion.SizeY,0,1), ZeroUV, Color,FHitProxyId());
	BatchedElements->AddTriangle(V00,V10,V11,GWhiteTexture,BLEND_Opaque);
	BatchedElements->AddTriangle(V00,V11,V01,GWhiteTexture,BLEND_Opaque);

	if( IsInRenderingThread() )
	{
		// Set the RHI render target to the scene color surface, which is necessary so the render target's dimensions and anti-aliasing
		// parameters match the the scene depth surface
		RHISetRenderTarget(GSceneRenderTargets.GetSceneColorSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
		// set viewport to RT size
		RHISetViewport(0,0,0.0f,RenderTarget->GetSizeX(),RenderTarget->GetSizeY(),1.0f);	
		// disable color writes
		RHISetColorWriteEnable(FALSE);
		// set stencil write enable to one
		RHISetStencilState(TStaticStencilState<TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,1>::GetRHI());
		// render the masked region
		BatchedElements->Draw(
			MaskRegion.Transform,
			RenderTarget->GetSizeX(),
			RenderTarget->GetSizeY(),
			IsHitTesting(),
			1.0f
			);
		// Restore the canvas's render target
		RHISetRenderTarget(RenderTarget->GetRenderTargetSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
		// Restore the viewport; RHISetRenderTarget resets to the full render target.
		RHISetViewport(0,0,0.0f,RenderTarget->GetSizeX(),RenderTarget->GetSizeY(),1.0f);	
		// reenable color writes
		RHISetColorWriteEnable(TRUE);
		// set stencil state to only render to the masked region
		RHISetStencilState(TStaticStencilState<TRUE,CF_NotEqual,SO_Keep,SO_Keep,SO_Keep,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,0>::GetRHI());
		// done rendering batch so delete it
		delete BatchedElements;
	}
	else
	{
		struct FCanvasFlushParameters
		{
			UINT ViewSizeX;
			UINT ViewSizeY;
			const FRenderTarget* RenderTarget;
			BITFIELD bIsHitTesting : 1;
			FMatrix Transform;
			FBatchedElements* BatchedElements;
		};
		FCanvasFlushParameters FlushParameters =
		{			
			RenderTarget->GetSizeX(),
			RenderTarget->GetSizeY(),
			RenderTarget,
			IsHitTesting(),
			MaskRegion.Transform,
			BatchedElements
		};
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			CanvasFlushSetMaskRegion,
			FCanvasFlushParameters,Parameters,FlushParameters,
		{
			// Set the RHI render target to the scene color surface, which is necessary so the render target's dimensions and anti-aliasing
			// parameters match the the scene depth surface
			RHISetRenderTarget(GSceneRenderTargets.GetSceneColorSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
			// set viewport to RT size
			RHISetViewport(0,0,0.0f,Parameters.ViewSizeX,Parameters.ViewSizeY,1.0f);	
			// disable color writes
			RHISetColorWriteEnable(FALSE);
			// set stencil write enable to one
			RHISetStencilState(TStaticStencilState<TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,1>::GetRHI());
			// render the masked region
			Parameters.BatchedElements->Draw(
				Parameters.Transform,
				Parameters.ViewSizeX,
				Parameters.ViewSizeY,
				Parameters.bIsHitTesting,
				1.0f
				);
			// Restore the canvas's render target
			RHISetRenderTarget(Parameters.RenderTarget->GetRenderTargetSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
			// Restore the viewport; RHISetRenderTarget resets to the full render target.
			RHISetViewport(0,0,0.0f,Parameters.ViewSizeX,Parameters.ViewSizeY,1.0f);	
			// reenable color writes
			RHISetColorWriteEnable(TRUE);
			// set stencil state to only render to the masked region
			RHISetStencilState(TStaticStencilState<TRUE,CF_NotEqual,SO_Keep,SO_Keep,SO_Keep,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,0>::GetRHI());
			// done rendering batch so delete it
			delete Parameters.BatchedElements;
		});
	}
}

/**
* Clear masked region during flush
*/
void FCanvas::FlushResetMaskRegion()
{
	if( !(AllowedModes&Allow_MaskedRegions) ) 
	{ 
		return; 
	}

	checkSlow(GetCurrentMaskRegion().IsValid());

	if( IsInRenderingThread() )
	{
		// clear stencil to 0
		RHIClear(FALSE,FLinearColor::Black,FALSE,0,TRUE,0);
		// reset stencil state
		RHISetStencilState(TStaticStencilState<>::GetRHI());
	}
	else
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND(
			CanvasFlushResetMaskRegionCommand,
		{
			// clear stencil to 0
			RHIClear(FALSE,FLinearColor::Black,FALSE,0,TRUE,0);
			// reset stencil state
			RHISetStencilState(TStaticStencilState<>::GetRHI());
		});
	}
}

/**
* Destructor for canvas
*/
FCanvas::~FCanvas()
{
	// delete batches from elements entries
	for( INT Idx=0; Idx < SortedElements.Num(); Idx++ )
	{
		FCanvasSortElement& SortElement = SortedElements(Idx);
		for( INT BatchIdx=0; BatchIdx < SortElement.RenderBatchArray.Num(); BatchIdx++ )
		{
			FCanvasBaseRenderItem* RenderItem = SortElement.RenderBatchArray(BatchIdx);
			delete RenderItem;
		}
	}
}

/** 
* Sends a message to the rendering thread to draw the batched elements. 
*/
void FCanvas::Flush(UBOOL bForce)
{
	SCOPE_CYCLE_COUNTER(STAT_Canvas_FlushTime);

	if( !(AllowedModes&Allow_Flush) && !bForce ) 
	{ 
		return; 
	}

	// current render target set for the canvas
	check(RenderTarget);	 	

	// sort the array of FCanvasSortElement entries so that higher sort keys render first (back-to-front)
	Sort<USE_COMPARE_CONSTREF(FCanvasSortElement,UnCanvas)>( &SortedElements(0), SortedElements.Num() );
	
	if( IsInRenderingThread() )
	{
		if( bEnableDepthTest &&
			(AllowedModes&Allow_DepthTest) )
		{
			// Set the RHI render target. and the scene depth surface
			RHISetRenderTarget(RenderTarget->GetRenderTargetSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
			// enable depth test & disable writes
			RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
		}
		else
		{
			// Set the RHI render target.
			RHISetRenderTarget(RenderTarget->GetRenderTargetSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
			// disable depth test & writes
			RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		}
		// set viewport to RT size
		RHISetViewport(0,0,0.0f,RenderTarget->GetSizeX(),RenderTarget->GetSizeY(),1.0f);	
	}
	else 
	{
		struct FCanvasFlushParameters
		{
			BITFIELD bDepthTestEnabled : 1;
			UINT ViewSizeX;
			UINT ViewSizeY;
			const FRenderTarget* CanvasRenderTarget;
		};
		FCanvasFlushParameters FlushParameters =
		{
			bEnableDepthTest && (AllowedModes&Allow_DepthTest),
			RenderTarget->GetSizeX(),
			RenderTarget->GetSizeY(),
			RenderTarget
		};
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			CanvasFlushSetupCommand,
			FCanvasFlushParameters,Parameters,FlushParameters,
		{
			if( Parameters.bDepthTestEnabled )
			{
				// Set the RHI render target. and the scene depth surface
				RHISetRenderTarget(Parameters.CanvasRenderTarget->GetRenderTargetSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
				// enable depth test & disable writes
				RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
			}
			else
			{
				// Set the RHI render target.
				RHISetRenderTarget(Parameters.CanvasRenderTarget->GetRenderTargetSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
				// disable depth test & writes
				RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
			}
			// set viewport to RT size
			RHISetViewport(0,0,0.0f,Parameters.ViewSizeX,Parameters.ViewSizeY,1.0f);	
		});
	}

	if( GetCurrentMaskRegion().IsValid() )
	{
		// setup the masked region if it was valid
		FlushSetMaskRegion();
	}

	// iterate over the FCanvasSortElements in sorted order and render all the batched items for each entry
	for( INT Idx=0; Idx < SortedElements.Num(); Idx++ )
	{
		FCanvasSortElement& SortElement = SortedElements(Idx);
		for( INT BatchIdx=0; BatchIdx < SortElement.RenderBatchArray.Num(); BatchIdx++ )
		{
			FCanvasBaseRenderItem* RenderItem = SortElement.RenderBatchArray(BatchIdx);
			if( RenderItem )
			{
				// mark current render target as dirty since we are drawing to it
				bRenderTargetDirty |= RenderItem->Render(this);
				if( AllowedModes & Allow_DeleteOnRender )
				{
					delete RenderItem;
				}
			}			
		}
		if( AllowedModes & Allow_DeleteOnRender )
		{
			SortElement.RenderBatchArray.Empty();
		}
	}
	if( AllowedModes & Allow_DeleteOnRender )
	{
		// empty the array of FCanvasSortElement entries after finished with rendering	
		SortedElements.Empty();
		SortedElementLookupMap.Empty();
	}

	if( GetCurrentMaskRegion().IsValid() )
	{
		// reset the masked region if it was valid
		FlushResetMaskRegion();
	}
}

void FCanvas::PushRelativeTransform(const FMatrix& Transform)
{
	INT PreviousTopIndex = TransformStack.Num() - 1;
#if 0
	static UBOOL DEBUG_NoRotation=1;
	if( DEBUG_NoRotation )
	{
		FMatrix TransformNoRotation(FMatrix::Identity);
		TransformNoRotation.SetOrigin(Transform.GetOrigin());
		TransformStack.AddItem( FTransformEntry(TransformNoRotation * TransformStack(PreviousTopIndex).GetMatrix()) );
	}
	else
#endif
	{
		TransformStack.AddItem( FTransformEntry(Transform * TransformStack(PreviousTopIndex).GetMatrix()) );
	}
}

void FCanvas::PushAbsoluteTransform(const FMatrix& Transform) 
{
	TransformStack.AddItem( FTransformEntry(Transform * TransformStack(0).GetMatrix()) );
}

void FCanvas::PopTransform()
{
	TransformStack.Pop();
}

void FCanvas::SetHitProxy(HHitProxy* HitProxy)
{
	// Change the current hit proxy.
	CurrentHitProxy = HitProxy;

	if(HitProxyConsumer && HitProxy)
	{
		// Notify the hit proxy consumer of the new hit proxy.
		HitProxyConsumer->AddHitProxy(HitProxy);
	}
}

/**
* Determine if the canvas has dirty batches that need to be rendered
*
* @return TRUE if the canvas has any element to render
**/
UBOOL FCanvas::HasBatchesToRender() const
{
	for( INT Idx=0; Idx < SortedElements.Num(); Idx++ )
	{
		const FCanvasSortElement& SortElement = SortedElements(Idx);
		for( INT BatchIdx=0; BatchIdx < SortElement.RenderBatchArray.Num(); BatchIdx++ )
		{
			if( SortElement.RenderBatchArray(BatchIdx) )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
* Copy the conents of the TransformStack from an existing canvas
*
* @param Copy	canvas to copy from
**/
void FCanvas::CopyTransformStack(const FCanvas& Copy)
{ 
	TransformStack = Copy.TransformStack;
}

/**
* Toggles current depth testing state for the canvas. All batches
* will render with depth testing against the depth buffer if enabled.
*
* @param bEnabled - if TRUE then depth testing is enabled
*/
void FCanvas::SetDepthTestingEnabled(UBOOL bEnabled)
{
	if( bEnableDepthTest != bEnabled )
	{
		Flush();
		bEnableDepthTest = bEnabled;
	}
}

/** 
 * Set the current masked region on the canvas
 * All rendering from this point on will be masked to this region.
 * The region being masked uses the current canvas transform
 *
 * @param X - x offset in canvas coords
 * @param Y - y offset in canvas coords
 * @param SizeX - x size in canvas coords
 * @param SizeY - y size in canvas coords
 */
void FCanvas::PushMaskRegion( FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY )
{
	FMaskRegion NewMask(X, Y, SizeX, SizeY, TransformStack.Top().GetMatrix());
	if ( !NewMask.IsEqual(GetCurrentMaskRegion()) )
	{
		Flush();
	}

	MaskRegionStack.Push(NewMask);
}

/**
 * Replace the top element of the masking region stack with a new region
 */
void FCanvas::ReplaceMaskRegion( FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY )
{
	if ( MaskRegionStack.Num() > 0 )
	{
		const INT CurrentMaskIdx = MaskRegionStack.Num() - 1;

		FMaskRegion NewMask(X, Y, SizeX, SizeY, TransformStack.Top().GetMatrix());
		if ( !NewMask.IsEqual(MaskRegionStack(CurrentMaskIdx)) )
		{
			Flush();
			MaskRegionStack(CurrentMaskIdx) = NewMask;
		}
	}
	else
	{
		PushMaskRegion(X, Y, SizeX, SizeY);
	}
}

/**
 * Remove the current masking region; if other masking regions were previously pushed onto the stack,
 * the next one down will be activated.
 */
void FCanvas::PopMaskRegion()
{
	FMaskRegion NextMaskRegion = MaskRegionStack.Num() > 1 
		? MaskRegionStack(MaskRegionStack.Num() - 2)
		: FMaskRegion();

	if ( !NextMaskRegion.IsEqual(GetCurrentMaskRegion()) )
	{
		Flush();
	}

	if ( MaskRegionStack.Num() > 0 )
	{
		MaskRegionStack.Pop();
	}
}

/**
 * Get the top-most canvas masking region from the stack.
 */
FCanvas::FMaskRegion FCanvas::GetCurrentMaskRegion() const
{
	if ( MaskRegionStack.Num() > 0 )
	{
		return MaskRegionStack(MaskRegionStack.Num() - 1);
	}

	return FMaskRegion();
}

void FCanvas::SetRenderTarget(FRenderTarget* NewRenderTarget)
{
	if( RenderTarget != NewRenderTarget )
	{
		// flush whenever we swap render targets
		if( RenderTarget )
		{
			Flush();			

			// resolve the current render target if it is dirty
			if( bRenderTargetDirty )
			{
				if( IsInRenderingThread() )
				{
					RHICopyToResolveTarget(RenderTarget->GetRenderTargetSurface(),TRUE,FResolveParams());					
				}
				else
				{
					ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
						ResolveCanvasRTCommand,
						FRenderTarget*,CanvasRenderTarget,RenderTarget,
					{
						RHICopyToResolveTarget(CanvasRenderTarget->GetRenderTargetSurface(),TRUE,FResolveParams());
					});
				}
				SetRenderTargetDirty(FALSE);
			}
		}
		// Change the current render target.
		RenderTarget = NewRenderTarget;
	}
}

void Clear(FCanvas* Canvas,const FLinearColor& Color)
{
	// desired display gamma space
	const FLOAT DisplayGamma = (GEngine && GEngine->Client) ? GEngine->Client->DisplayGamma : 2.2f;
	// render target gamma space expected
	FLOAT RenderTargetGamma = DisplayGamma;
	if( Canvas->GetRenderTarget() )
	{
		RenderTargetGamma = Canvas->GetRenderTarget()->GetDisplayGamma();
	}
	// assume that the clear color specified is in 2.2 gamma space
	// so convert to the render target's color space 
	FLinearColor GammaCorrectedColor(Color);
	GammaCorrectedColor.R = appPow(Clamp<FLOAT>(GammaCorrectedColor.R,0.0f,1.0f), DisplayGamma / RenderTargetGamma);
	GammaCorrectedColor.G = appPow(Clamp<FLOAT>(GammaCorrectedColor.G,0.0f,1.0f), DisplayGamma / RenderTargetGamma);
	GammaCorrectedColor.B = appPow(Clamp<FLOAT>(GammaCorrectedColor.B,0.0f,1.0f), DisplayGamma / RenderTargetGamma);

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		ClearCommand,
		FColor,Color,GammaCorrectedColor,
		FRenderTarget*,CanvasRenderTarget,Canvas->GetRenderTarget(),
		{
			if( CanvasRenderTarget )
			{
				RHISetRenderTarget(CanvasRenderTarget->GetRenderTargetSurface(),FSurfaceRHIRef());
				RHISetViewport(0,0,0.0f,CanvasRenderTarget->GetSizeX(),CanvasRenderTarget->GetSizeY(),1.0f);
			}
			RHIClear(TRUE,Color,FALSE,0.0f,FALSE,0);
		});
}

/**
 *	Draws a line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
void DrawLine(FCanvas* Canvas,const FVector& StartPos,const FVector& EndPos,const FLinearColor& Color)
{
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Line);
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	BatchedElements->AddLine(StartPos,EndPos,Color,HitProxyId);
}

/**
 *	Draws a 2D line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
void DrawLine2D(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos,const FLinearColor& Color)
{
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Line);
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	BatchedElements->AddLine(FVector(StartPos.X,StartPos.Y,0),FVector(EndPos.X,EndPos.Y,0),Color,HitProxyId);
}

void DrawBox2D(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos,const FLinearColor& Color)
{
	DrawLine2D(Canvas,FVector2D(StartPos.X,StartPos.Y),FVector2D(StartPos.X,EndPos.Y),Color);
	DrawLine2D(Canvas,FVector2D(StartPos.X,EndPos.Y),FVector2D(EndPos.X,EndPos.Y),Color);
	DrawLine2D(Canvas,FVector2D(EndPos.X,EndPos.Y),FVector2D(EndPos.X,StartPos.Y),Color);
	DrawLine2D(Canvas,FVector2D(EndPos.X,StartPos.Y),FVector2D(StartPos.X,StartPos.Y),Color);
}

void DrawTile(
	FCanvas* Canvas,
	FLOAT X,
	FLOAT Y,
	FLOAT SizeX,
	FLOAT SizeY,
	FLOAT U,
	FLOAT V,
	FLOAT SizeU,
	FLOAT SizeV,
	const FLinearColor& Color,
	const FTexture* Texture,
	UBOOL AlphaBlend
	)
{
	SCOPE_CYCLE_COUNTER(STAT_Canvas_DrawTextureTileTime);

	FLinearColor ActualColor = Color;
	ActualColor.A *= Canvas->AlphaModulate;

	const FTexture* FinalTexture = Texture ? Texture : GWhiteTexture;
	const EBlendMode BlendMode = AlphaBlend ? BLEND_Translucent : BLEND_Opaque;
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Triangle, FinalTexture, BlendMode);	
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	INT V00 = BatchedElements->AddVertex(FVector4(X,		Y,			0,1),FVector2D(U,			V),			ActualColor,HitProxyId);
	INT V10 = BatchedElements->AddVertex(FVector4(X + SizeX,Y,			0,1),FVector2D(U + SizeU,	V),			ActualColor,HitProxyId);
	INT V01 = BatchedElements->AddVertex(FVector4(X,		Y + SizeY,	0,1),FVector2D(U,			V + SizeV),	ActualColor,HitProxyId);
	INT V11 = BatchedElements->AddVertex(FVector4(X + SizeX,Y + SizeY,	0,1),FVector2D(U + SizeU,	V + SizeV),	ActualColor,HitProxyId);

	BatchedElements->AddTriangle(V00,V10,V11,FinalTexture,BlendMode);
	BatchedElements->AddTriangle(V00,V11,V01,FinalTexture,BlendMode);
}

void DrawTile(
	FCanvas* Canvas,
	FLOAT X,
	FLOAT Y,
	FLOAT SizeX,
	FLOAT SizeY,
	FLOAT U,
	FLOAT V,
	FLOAT SizeU,
	FLOAT SizeV,
	const FMaterialRenderProxy* MaterialRenderProxy
	)
{
	SCOPE_CYCLE_COUNTER(STAT_Canvas_DrawMaterialTileTime);

	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	// add a new FTileRenderItem entry. These get rendered and freed when the canvas is flushed
	Canvas->AddTileRenderItem(X,Y,SizeX,SizeY,U,V,SizeU,SizeV,MaterialRenderProxy,HitProxyId);
}

void DrawTriangle2D(
	FCanvas* Canvas,
	const FVector2D& Position0,
	const FVector2D& TexCoord0,
	const FVector2D& Position1,
	const FVector2D& TexCoord1,
	const FVector2D& Position2,
	const FVector2D& TexCoord2,
	const FLinearColor& Color,
	const FTexture* Texture,
	UBOOL AlphaBlend
	)
{
	const EBlendMode BlendMode = AlphaBlend ? BLEND_Translucent : BLEND_Opaque;
	const FTexture* FinalTexture = Texture ? Texture : GWhiteTexture;
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Triangle, FinalTexture, BlendMode);
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	INT V0 = BatchedElements->AddVertex(FVector4(Position0.X,Position0.Y,0,1),TexCoord0,Color,HitProxyId);
	INT V1 = BatchedElements->AddVertex(FVector4(Position1.X,Position1.Y,0,1),TexCoord1,Color,HitProxyId);
	INT V2 = BatchedElements->AddVertex(FVector4(Position2.X,Position2.Y,0,1),TexCoord2,Color,HitProxyId);

	BatchedElements->AddTriangle(V0,V1,V2,FinalTexture, BlendMode);
}

void DrawTriangle2D(
	FCanvas* Canvas,
	const FVector2D& Position0,
	const FVector2D& TexCoord0,
	const FVector2D& Position1,
	const FVector2D& TexCoord1,
	const FVector2D& Position2,
	const FVector2D& TexCoord2,
	const FMaterialRenderProxy* MaterialRenderProxy
	)
{}

INT DrawStringCentered(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color)
{
	INT XL, YL;
	StringSize( Font, XL, YL, Text );

	return DrawString(Canvas, StartX-(XL/2), StartY, Text, Font, Color );
}

INT DrawString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color,FLOAT XScale, FLOAT YScale, FLOAT HorizSpacingAdjust, const FLOAT* ForcedViewportHeight/*=NULL*/ )
{
	SCOPE_CYCLE_COUNTER(STAT_Canvas_DrawStringTime);

	if(Font == NULL || Text == NULL)
	{
		return FALSE;
	}

	// Get the scaling and resolution information from the font.
	const FLOAT FontResolutionTest = (ForcedViewportHeight && *ForcedViewportHeight != 0) ? *ForcedViewportHeight : Canvas->GetRenderTarget()->GetSizeY();
	const INT PageIndex = Font->GetResolutionPageIndex(FontResolutionTest);
	const FLOAT FontScale = Font->GetScalingFactor(FontResolutionTest);

	// apply the font's internal scale to the desired scaling
	XScale *= FontScale;
	YScale *= FontScale;

	FLinearColor ActualColor = Color;
	ActualColor.A *= Canvas->AlphaModulate;

	FBatchedElements* BatchedElements = NULL;
	const EBlendMode BlendMode = BLEND_Translucent;	
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();
	FTexture* LastTexture = NULL;
	UTexture2D* Tex = NULL;

	const FLOAT CharIncrement = ( (FLOAT)Font->Kerning + HorizSpacingAdjust ) * XScale;

	// Draw all characters in string.
	FLOAT LineX = 0;
	INT TextLen = appStrlen(Text);
	for( INT i=0; i < TextLen; i++ )
	{
		INT Ch = (INT)Font->RemapChar(Text[i]);

		// Process character if it's valid.
		if( Font->Characters.IsValidIndex(Ch + PageIndex) )
		{
			FFontCharacter& Char = Font->Characters(Ch + PageIndex);
			if( Font->Textures.IsValidIndex(Char.TextureIndex) && 
				(Tex=Font->Textures(Char.TextureIndex))!=NULL && 
				Tex->Resource != NULL )
			{
				if( LastTexture != Tex->Resource || BatchedElements == NULL )
				{
					BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Triangle, Tex->Resource, BlendMode);

					// trade-off between memory and performance by pre-allocating more reserved space 
					// for the triangles/vertices of the batched elements used to render the text tiles
					//BatchedElements->AddReserveTriangles(TextLen*2,Tex->Resource,BlendMode);
					//BatchedElements->AddReserveVertices(TextLen*4);
				}
				LastTexture = Tex->Resource;

				const FLOAT X      = LineX + StartX;
				const FLOAT Y      = StartY + Char.VerticalOffset * YScale;
				FLOAT SizeX = Char.USize * XScale;
				const FLOAT SizeY = Char.VSize * YScale;
				const FLOAT U     = Char.StartU / (FLOAT)Tex->SizeX;
				const FLOAT V     = Char.StartV / (FLOAT)Tex->SizeY;
				const FLOAT SizeU = Char.USize / (FLOAT)Tex->SizeX;
				const FLOAT SizeV = Char.VSize / (FLOAT)Tex->SizeY;				

				INT V00 = BatchedElements->AddVertex(FVector4(X,		Y,			0,1),FVector2D(U,			V),			ActualColor,HitProxyId);
				INT V10 = BatchedElements->AddVertex(FVector4(X + SizeX,Y,			0,1),FVector2D(U + SizeU,	V),			ActualColor,HitProxyId);
				INT V01 = BatchedElements->AddVertex(FVector4(X,		Y + SizeY,	0,1),FVector2D(U,			V + SizeV),	ActualColor,HitProxyId);
				INT V11 = BatchedElements->AddVertex(FVector4(X + SizeX,Y + SizeY,	0,1),FVector2D(U + SizeU,	V + SizeV),	ActualColor,HitProxyId);

				BatchedElements->AddTriangle(V00,V10,V11,Tex->Resource,BlendMode);
				BatchedElements->AddTriangle(V00,V11,V01,Tex->Resource,BlendMode);

				// if we have another non-whitespace character to render, add the font's kerning.
				if ( Text[i+1] && !appIsWhitespace(Text[i+1]) )
				{
					SizeX += CharIncrement;
				}

				// Update the current rendering position
				LineX += SizeX;
			}
		}
	}

	return appTrunc(LineX);
}



void DrawStringWrapped( FCanvas* Canvas, UBOOL Draw, FLOAT CurX, FLOAT CurY, FLOAT& XL, FLOAT& YL, UFont* Font, const TCHAR* Text, FLinearColor DrawColor )
{
	if (!Font)
	{
		return;
	}

	FLOAT FontResolutionTest = Canvas->GetRenderTarget()->GetSizeY();
	INT ResolutionPageIndex = Font->GetResolutionPageIndex( FontResolutionTest );
	FLOAT FontScale = Font->GetScalingFactor( FontResolutionTest );

	// @todo Expose scaling and spacing options to function parameters
	FLOAT EffectiveXScale = 1.0f * FontScale;
	FLOAT EffectiveYScale = 1.0f * FontScale;
	FLOAT HorizSpacingAdjust = 0.0f;


	FLOAT Width = XL;
	FLOAT ClipX = CurX + XL;
	do
	{
		INT iCleanWordEnd=0, iTestWord;
		FLOAT TestXL = 0, CleanXL=0;
		FLOAT TestYL = 0, CleanYL=0;
		UBOOL GotWord = FALSE;
		for( iTestWord = 0 ; Text[iTestWord] != 0 && Text[iTestWord] != '\n'; )
		{
			FLOAT ChW, ChH;
			Font->GetCharSize(Text[iTestWord], ChW, ChH, ResolutionPageIndex);
			TestXL += ChW * EffectiveXScale; 

			// if we have another non-whitespace character to render, add the font's kerning.
			if ( Text[ iTestWord ] != 0 && !appIsWhitespace( Text[ iTestWord + 1 ] ) )
			{
				TestXL += EffectiveXScale * ( ( FLOAT )Font->Kerning + HorizSpacingAdjust );
			}

			TestYL = Max(TestYL, ChH * EffectiveYScale );

			// If we are past the width, break here

			if( TestXL > Width )
			{
				break;
			}
			
			iTestWord++;

			UBOOL WordBreak = Text[iTestWord]==' ' || Text[iTestWord]=='\n' || Text[iTestWord]==0;

			if ( WordBreak || !GotWord )
			{
				iCleanWordEnd = iTestWord;
				CleanXL       = TestXL;
				CleanYL       = TestYL;
				GotWord       = GotWord || WordBreak;
			}

			if ( Text[iTestWord] =='\n' && Text[iTestWord+1] == '\n' )
			{
				CleanYL *= 2;
			}
		}

		if( iCleanWordEnd == 0 )
		{
			break;
		}

		// Sucessfully split this line, now draw it.
		if ( Draw && iCleanWordEnd>0 )
		{
			FString TextLine(Text);
			FLOAT LineX = CurX;
			LineX += DrawString(Canvas,LineX, CurY,*(TextLine.Left(iCleanWordEnd)),Font,DrawColor);
		}

		// Update position.
		CurY += CleanYL;
		YL   += CleanYL;
		XL   = Max( XL,CleanXL);
		Text += iCleanWordEnd;

		// Skip whitespace after word wrap.
		while( *Text==' ' || *Text=='\n' )
		{
			Text++;
		}
	}
	while( *Text );
}




INT DrawShadowedString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color)
{
	// Draw a shadow of the text offset by 1 pixel in X and Y.
	DrawString(Canvas,StartX + 1,StartY + 1,Text,Font,FLinearColor::Black);

	// Draw the text.
	return DrawString(Canvas,StartX,StartY,Text,Font,Color);
}

/**
* Render string using both a font and a material. The material should have a font exposed as a 
* parameter so that the correct font page can be set based on the character being drawn.
*
* @param Canvas - valid canvas for rendering tiles
* @param StartX - starting X screen position
* @param StartY - starting Y screen position
* @param XScale - scale of text rendering in X direction
* @param YScale - scale of text rendering in Y direction
* @param Text - string of text to be rendered
* @param Font - font containing texture pages of character glyphs
* @param MatInst - material with a font parameter
* @param FontParam - name of the font parameter in the material
* @return total size in pixels of text drawn
*/
INT DrawStringMat(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,FLOAT XScale,FLOAT YScale,FLOAT HorizSpacingAdjust,const TCHAR* Text,class UFont* Font,UMaterialInterface* MatInst,const TCHAR* FontParam)
{
	checkSlow(Canvas);

	INT Result = 0;	
	if( Font && Text )
	{
		if( MatInst )
		{
			// check for valid font parameter name
			UFont* TempFont;
			INT TempFontPage;
			if( !FontParam || 
				!MatInst->GetFontParameterValue(FName(FontParam),TempFont,TempFontPage) )
			{
				//debugf(NAME_Warning,TEXT("Invalid font parameter name [%s]"),FontParam ? FontParam : TEXT("NULL"));
				Result = DrawString(Canvas,StartX,StartY,Text,Font,FLinearColor(0,1,0,1),XScale,YScale);
			}
			else
			{
				// Get the scaling and resolution information from the font.
				const FLOAT FontResolutionTest = Canvas->GetRenderTarget()->GetSizeY();
				const INT PageIndex = Font->GetResolutionPageIndex(FontResolutionTest);
				const FLOAT FontScale = Font->GetScalingFactor(FontResolutionTest);

				// apply the font's internal scale to the desired scaling
				XScale *= FontScale;
				YScale *= FontScale;

				// create a FFontMaterialRenderProxy for each font page
				TArray<FFontMaterialRenderProxy> FontMats;
				for( INT FontPage=0; FontPage < Font->Textures.Num(); FontPage++ )
				{
					new(FontMats) FFontMaterialRenderProxy(MatInst->GetRenderProxy(FALSE),Font,FontPage,FName(FontParam));
				}
				// Draw all characters in string.
				FLOAT LineX = 0;
				for( INT i=0; Text[i]; i++ )
				{
					// unicode mapping of text
					INT Ch = (INT)Font->RemapChar(Text[i]);
					// Process character if it's valid.
					if( Ch < Font->Characters.Num() )
					{
						UTexture2D* Tex = NULL;
						FFontCharacter& Char = Font->Characters(Ch);
						// only render fonts with a valid texture page
						if( Font->Textures.IsValidIndex(Char.TextureIndex) && 
							(Tex=Font->Textures(Char.TextureIndex)) != NULL )
						{
							const FLOAT X			= LineX + StartX;
							const FLOAT Y			= StartY + Char.VerticalOffset * YScale;
							const FLOAT CU			= Char.StartU;
							const FLOAT CV			= Char.StartV;
							const FLOAT CUSize		= Char.USize;
							const FLOAT CVSize		= Char.VSize;
							const FLOAT ScaledSizeU	= CUSize * XScale;
							const FLOAT ScaledSizeV	= CVSize * YScale;
							
							// Draw using the font material instance
							DrawTile(
								Canvas,
								X,
								Y,
								ScaledSizeU,
								ScaledSizeU,
								CU		/ (FLOAT)Tex->SizeX,
								CV		/ (FLOAT)Tex->SizeY,
								CUSize	/ (FLOAT)Tex->SizeX,
								CVSize	/ (FLOAT)Tex->SizeY,
								&FontMats(Char.TextureIndex)
								);

							// Update the current rendering position
							LineX += ScaledSizeU;

							// if we have another non-whitespace character to render, add the font's kerning.
							if ( Text[i+1] && !appIsWhitespace(Text[i+1]) )
							{
								LineX += XScale * ( ( FLOAT )Font->Kerning + HorizSpacingAdjust );
							}
						}
					}
				}
				// return the resulting line position
				Result = appTrunc(LineX);
			}
		}
		else
		{
			// fallback to just using the font texture without a material
			Result = DrawString(Canvas,StartX,StartY,Text,Font,FLinearColor(0,1,0,1),XScale,YScale);
		}
	}

	return Result;
}

void StringSize(UFont* Font,INT& XL,INT& YL,const TCHAR* Format,...)
{
	TCHAR Text[4096];
	GET_VARARGS( Text, ARRAY_COUNT(Text), ARRAY_COUNT(Text)-1, Format, Format );

	// this functionality has been moved to a static function in UIString
	FRenderParameters Parameters(Font,1.f,1.f);
	UUIString::StringSize(Parameters, Text);

	XL = appTrunc(Parameters.DrawXL);
	YL = appTrunc(Parameters.DrawYL);
}

/*-----------------------------------------------------------------------------
	UCanvas object functions.
-----------------------------------------------------------------------------*/

void UCanvas::Init()
{
}

void UCanvas::Update()
{
	// Call UnrealScript to reset.
	eventReset();

	// Copy size parameters from viewport.
	ClipX = SizeX;
	ClipY = SizeY;

}

/*-----------------------------------------------------------------------------
	UCanvas scaled sprites.
-----------------------------------------------------------------------------*/

void UCanvas::SetPos(FLOAT X, FLOAT Y)
{
	CurX = X;
	CurY = Y;
}

void UCanvas::SetDrawColor(BYTE R, BYTE G, BYTE B, BYTE A)
{
	DrawColor.R = R;
	DrawColor.G = G;
	DrawColor.B = B;
	DrawColor.A = A;
}

//
// Draw arbitrary aligned rectangle.
//
void UCanvas::DrawTile
(
	UTexture*			Tex,
	FLOAT				X,
	FLOAT				Y,
	FLOAT				XL,
	FLOAT				YL,
	FLOAT				U,
	FLOAT				V,
	FLOAT				UL,
	FLOAT				VL,
	const FLinearColor&	Color
)
{
    if ( !Canvas || !Tex ) 
        return;

	FLOAT MyClipX = OrgX + ClipX;
	FLOAT MyClipY = OrgY + ClipY;
	FLOAT w = X + XL > MyClipX ? MyClipX - X : XL;
	FLOAT h = Y + YL > MyClipY ? MyClipY - Y : YL;
	if (XL > 0.f &&
		YL > 0.f)
	{
		FLOAT SizeX = Tex->GetSurfaceWidth();
		FLOAT SizeY = Tex->GetSurfaceHeight();
		::DrawTile(
			Canvas,
			(X),
			(Y),
			(w),
			(h),
			U/SizeX,
			V/SizeY,
			UL/SizeX * w/XL,
			VL/SizeY * h/YL,
			Color,
			Tex->Resource
			);		
	}
}

void UCanvas::DrawMaterialTile(
	UMaterialInterface*	Material,
	FLOAT				X,
	FLOAT				Y,
	FLOAT				XL,
	FLOAT				YL,
	FLOAT				U,
	FLOAT				V,
	FLOAT				UL,
	FLOAT				VL
	)
{
    if ( !Canvas || !Material ) 
        return;

	::DrawTile(
		Canvas,
		(X),
		(Y),
		(XL),
		(YL),
		U,
		V,
		UL,
		VL,
		Material->GetRenderProxy(0)
		);
}

void UCanvas::ClippedStrLen( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Text )
{
	XL = 0;
	YL = 0;
	if (Font != NULL)
	{
		FRenderParameters Parameters(Font,ScaleX,ScaleY);
		UUIString::StringSize(Parameters, Text);

		XL = appTrunc(Parameters.DrawXL);
		YL = appTrunc(Parameters.DrawYL);
	}
}

//
// Calculate the size of a string built from a font, word wrapped
// to a specified region.
//
void VARARGS UCanvas::WrappedStrLenf( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Fmt, ... ) 
{
	TCHAR Text[4096];
	GET_VARARGS( Text, ARRAY_COUNT(Text), ARRAY_COUNT(Text)-1, Fmt, Fmt );

	WrappedPrint( 0, XL, YL, Font, ScaleX, ScaleY, 0, Text ); 
}

//
// Compute size and optionally print text with word wrap.
//!!For the next generation, redesign to ignore CurX,CurY.
//
void UCanvas::WrappedPrint( UBOOL Draw, INT& out_XL, INT& out_YL, UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Text ) 
{
	// FIXME: Wrapped Print is screwed which kills the hud.. fix later

	if( ClipX<0 || ClipY<0 )
		return;
	check(Font);
	

	FLOAT FontResolutionTest = Canvas->GetRenderTarget()->GetSizeY();
	INT ResolutionPageIndex = Font->GetResolutionPageIndex( FontResolutionTest );
	FLOAT FontScale = Font->GetScalingFactor( FontResolutionTest );

	// @todo Expose spacing option to function parameters
	FLOAT HorizSpacingAdjust = 0.0f;
	FLOAT EffectiveXScale = ScaleX * FontScale;
	FLOAT EffectiveYScale = ScaleY * FontScale;


	// Process each word until the current line overflows.
	FLOAT XL=0.f, YL=0.f;
	do
	{
		INT iCleanWordEnd=0, iTestWord;
		FLOAT TestXL= CurX, CleanXL=0;
		FLOAT TestYL=0,    CleanYL=0;
		UBOOL GotWord=0;
		for( iTestWord=0; Text[iTestWord]!=0 && Text[iTestWord]!='\n'; )
		{
			FLOAT ChW, ChH;
			Font->GetCharSize(Text[iTestWord], ChW, ChH, ResolutionPageIndex);
			TestXL += ChW * EffectiveXScale; 
			TestYL = Max(TestYL, ChH * EffectiveYScale);

			// if we have another non-whitespace character to render, add the font's kerning.
			if ( Text[ iTestWord ] != 0 && !appIsWhitespace( Text[ iTestWord + 1 ] ) )
			{
				TestXL += EffectiveXScale * ( ( FLOAT )Font->Kerning + HorizSpacingAdjust );
			}

			if( TestXL>ClipX )
			{
				break;
			}

			iTestWord++;
			UBOOL WordBreak = Text[iTestWord]==' ' || Text[iTestWord]=='\n' || Text[iTestWord]==0;
			if( WordBreak || !GotWord )
			{
				iCleanWordEnd = iTestWord;
				CleanXL       = TestXL;
				CleanYL       = TestYL;
				GotWord       = GotWord || WordBreak;
			}
		}
		if( iCleanWordEnd==0 )
			break;

		// Sucessfully split this line, now draw it.
		if (Draw && CurY < ClipY && CurY + CleanYL > 0)
		{
			FString TextLine(Text);
			FLOAT LineX = Center ? CurX+(ClipX-CleanXL)/2 : CurX;
			LineX += DrawString(Canvas,OrgX + LineX, OrgY + CurY,*(TextLine.Left(iCleanWordEnd)),Font,DrawColor, ScaleX, ScaleY);
			CurX = LineX;
		}

		// Update position.
		CurX  = 0;
		CurY += CleanYL;
		YL   += CleanYL;
		XL    = Max(XL,CleanXL);
		Text += iCleanWordEnd;

		// Skip whitespace after word wrap.
		while( *Text==' '|| *Text=='\n' )
			Text++;
	}
	while( *Text );

	out_XL = appTrunc(XL);
	out_YL = appTrunc(YL);
}


/*-----------------------------------------------------------------------------
	UCanvas natives.
-----------------------------------------------------------------------------*/

	
void UCanvas::execSetPos(FFrame& Stack, RESULT_DECL)
{
	P_GET_FLOAT(X);
	P_GET_FLOAT(Y);
	P_FINISH;
	SetPos(X,Y);
}

void UCanvas::execSetDrawColor(FFrame& Stack, RESULT_DECL)
{
	P_GET_BYTE(R);
	P_GET_BYTE(G);
	P_GET_BYTE(B);
	P_GET_BYTE_OPTX(A,255);
	P_FINISH;
	SetDrawColor(R,G,B,A);
}
IMPLEMENT_FUNCTION(UCanvas,INDEX_NONE,execSetDrawColor);

void UCanvas::execDrawTile( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture,Tex);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_FINISH;
	if( !Tex )
		return;

	DrawTile
	(
		Tex,
		OrgX+CurX,
		OrgY+CurY,
		XL,
		YL,
		U,
		V,
		UL,
		VL,
		DrawColor
	);
	CurX += XL;
	CurYL = Max(CurYL,YL);
}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execDrawTile );

void UCanvas::execDrawColorizedTile( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture,Tex);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_GET_STRUCT(FLinearColor,DrawColor);
	P_FINISH;
	if( !Tex )
		return;

	DrawTile
	(
		Tex,
		OrgX+CurX,
		OrgY+CurY,
		XL,
		YL,
		U,
		V,
		UL,
		VL,
		DrawColor
	);
	CurX += XL;
	CurYL = Max(CurYL,YL);
}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execDrawColorizedTile );


void UCanvas::execDrawMaterialTile( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UMaterialInterface,Material);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT_OPTX(U,0.f);
	P_GET_FLOAT_OPTX(V,0.f);
	P_GET_FLOAT_OPTX(UL,1.f);
	P_GET_FLOAT_OPTX(VL,1.f);
	P_FINISH;
	if(!Material)
		return;
	DrawMaterialTile
	(
		Material,
		OrgX+CurX,
		OrgY+CurY,
		XL,
		YL,
		U,
		V,
		UL,
		VL
	);
	CurX += XL;
	CurYL = Max(CurYL,YL);
}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execDrawMaterialTile );

void UCanvas::execDrawMaterialTileClipped( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UMaterialInterface,Material);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT_OPTX(U,0.f);
	P_GET_FLOAT_OPTX(V,0.f);
	P_GET_FLOAT_OPTX(UL,1.f);
	P_GET_FLOAT_OPTX(VL,1.f);
	P_FINISH;

	if ( !Material )
	{
		return;
	}

	if( CurX<0 )
		{FLOAT C=CurX*UL/XL; U-=C; UL+=C; XL+=CurX; CurX=0;}
	if( CurY<0 )
		{FLOAT C=CurY*VL/YL; V-=C; VL+=C; YL+=CurY; CurY=0;}
	if( XL>ClipX-CurX )
		{UL+=(ClipX-CurX-XL)*UL/XL; XL=ClipX-CurX;}
	if( YL>ClipY-CurY )
		{VL+=(ClipY-CurY-YL)*VL/YL; YL=ClipY-CurY;}

	DrawMaterialTile
	(
		Material,
		OrgX+CurX,
		OrgY+CurY,
		XL,
		YL,
		U,
		V,
		UL,
		VL
	);
	CurX += XL;
	CurYL = Max(CurYL,YL);
}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execDrawMaterialTileClipped );

void UCanvas::DrawText(FString &Text)
{
	INT XL = 0;
	INT YL = 0;
	WrappedPrint( 1, XL, YL, Font, 1.f, 1.f, bCenter, *Text ); 
}

void UCanvas::execDrawText( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(InText);
	P_GET_UBOOL_OPTX(CR,1);
	P_GET_FLOAT_OPTX(XScale,1.0f);
	P_GET_FLOAT_OPTX(YScale,1.0f);
	P_FINISH;

	if( !Font )
	{
		Stack.Logf( NAME_Warning, TEXT("DrawText: No font") ); 
		return;
	}
	INT		XL		= 0;
	INT		YL		= 0; 
	FLOAT	OldCurX	= CurX;
	FLOAT	OldCurY	= CurY;
	WrappedPrint( 1, XL, YL, Font, XScale, YScale, bCenter, *InText ); 
    
	CurX += XL;
	CurYL = Max(CurYL,(FLOAT)YL);
	if( CR )
	{
		CurX	= OldCurX;
		CurY	= OldCurY + CurYL;
		CurYL	= 0;
	}

}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execDrawText );

void UCanvas::execDrawTextClipped( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(InText);
	P_GET_UBOOL_OPTX(CheckHotKey, 0);
	P_GET_FLOAT_OPTX(XScale,1.0f);
	P_GET_FLOAT_OPTX(YScale,1.0f);
	P_FINISH;

	if( !Font )
	{
		Stack.Logf( TEXT("DrawTextClipped: No font") ); 
		return;
	}

	check(Font);

	DrawString(Canvas,appTrunc(OrgX + CurX), appTrunc(OrgY + CurY), *InText, Font, DrawColor, XScale, YScale);

}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execDrawTextClipped );

void UCanvas::execStrLen( FFrame& Stack, RESULT_DECL ) // wrapped 
{
	P_GET_STR(InText);
	P_GET_FLOAT_REF(XL);
	P_GET_FLOAT_REF(YL);
	P_FINISH;

	INT XLi = 0, YLi = 0;
	INT OldCurX, OldCurY;

	OldCurX = appTrunc(CurX);
	OldCurY = appTrunc(CurY);
	CurX = 0;
	CurY = 0;

	WrappedStrLenf( Font, 1.f, 1.f, XLi, YLi, TEXT("%s"), *InText );

	CurY = OldCurY;
	CurX = OldCurX;
	XL = XLi;
	YL = YLi;

}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execStrLen );

void UCanvas::execTextSize( FFrame& Stack, RESULT_DECL ) // clipped
{
	P_GET_STR(InText);
	P_GET_FLOAT_REF(XL);
	P_GET_FLOAT_REF(YL);
	P_FINISH;

	INT XLi, YLi;

	if( !Font )
	{
		Stack.Logf( TEXT("TextSize: No font") ); 
		return;
	}

	ClippedStrLen( Font, 1.f, 1.f, XLi, YLi, *InText );

	XL = XLi;
	YL = YLi;

}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execTextSize );

void UCanvas::execProject( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(Location);
	P_FINISH;

	FPlane V(0,0,0,0);

	if (SceneView!=NULL)
		V = SceneView->Project(Location);

	FVector resultVec(V);
	resultVec.X = (ClipX/2.f) + (resultVec.X*(ClipX/2.f));
	resultVec.Y *= -1.f;
	resultVec.Y = (ClipY/2.f) + (resultVec.Y*(ClipY/2.f));
	*(FVector*)Result =	resultVec;

}
IMPLEMENT_FUNCTION( UCanvas, -1, execProject);


/** 
 * Pushes a translation matrix onto the canvas. 
 *
 * @param TranslationVector		Translation vector to use to create the translation matrix.
 */
void UCanvas::execPushTranslationMatrix(FFrame& Stack, RESULT_DECL)
{
	P_GET_VECTOR(TranslationVector);
	P_FINISH;

	if(Canvas != NULL)
	{
		Canvas->PushRelativeTransform(FTranslationMatrix(TranslationVector));
	}
}

IMPLEMENT_FUNCTION( UCanvas, -1, execPushTranslationMatrix );

/** Pops the topmost matrix from the canvas transform stack. */
void UCanvas::execPopTransform(FFrame& Stack, RESULT_DECL)
{
	P_FINISH;

	if(Canvas != NULL)
	{
		Canvas->PopTransform();
	}
}

IMPLEMENT_FUNCTION( UCanvas, -1, execPopTransform );

void UCanvas::execDrawTileClipped( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture,Tex);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_FINISH;

	if( !Tex )
	{
		Stack.Logf( TEXT("DrawTileClipped: Missing Material") );
		return;
	}


	// Clip to ClipX and ClipY
	if( XL > 0 && YL > 0 )
	{		
		if( CurX<0 )
			{FLOAT C=CurX*UL/XL; U-=C; UL+=C; XL+=CurX; CurX=0;}
		if( CurY<0 )
			{FLOAT C=CurY*VL/YL; V-=C; VL+=C; YL+=CurY; CurY=0;}
		if( XL>ClipX-CurX )
			{UL+=(ClipX-CurX-XL)*UL/XL; XL=ClipX-CurX;}
		if( YL>ClipY-CurY )
			{VL+=(ClipY-CurY-YL)*VL/YL; YL=ClipY-CurY;}
	
		DrawTile
		(
			Tex,
			OrgX+CurX,
			OrgY+CurY,
			XL,
			YL,
			U,
			V,
			UL,
			VL,
			DrawColor
		);

		CurX += XL;
		CurYL = Max(CurYL,YL);
	}

}
IMPLEMENT_FUNCTION( UCanvas, 468, execDrawTileClipped );


void UCanvas::execDrawTileStretched( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture,Tex);
	P_GET_FLOAT(AWidth);
	P_GET_FLOAT(AHeight);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_GET_STRUCT(FLinearColor,DrawColor);
	P_GET_UBOOL_OPTX(bStretchHorizontally,TRUE);
	P_GET_UBOOL_OPTX(bStretchVertically,TRUE);
	P_GET_FLOAT_OPTX(ScalingFactor,1.0);
	P_FINISH;

	DrawTileStretched(Tex,CurX, CurY, AWidth, AHeight, U, V, UL, VL, DrawColor,bStretchHorizontally,bStretchVertically,ScalingFactor);

}
IMPLEMENT_FUNCTION( UCanvas, -1, execDrawTileStretched );

void UCanvas::DrawTileStretched(UTexture* Tex, FLOAT Left, FLOAT Top, FLOAT AWidth, FLOAT AHeight, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLinearColor DrawColor, UBOOL bStretchHorizontally,UBOOL bStretchVertically, FLOAT ScalingFactor)
{
	// Offset for the origin.
	Left += OrgX;
	Top += OrgY;

	// Compute the fraction of the tile which the texture edges cover without being stretched.
	const FLOAT EdgeFractionX = (Abs(AWidth) < DELTA || !bStretchHorizontally) ? 1.0f : Min(1.0f,Abs(UL * ScalingFactor / AWidth));
	const FLOAT EdgeFractionY = (Abs(AHeight) < DELTA || !bStretchVertically) ? 1.0f :  Min(1.0f,Abs(VL * ScalingFactor / AHeight));

	// Compute the dimensions of each row and column.
	const FLOAT EdgeSizeX = AWidth * EdgeFractionX * 0.5f;
	const FLOAT EdgeSizeY = AHeight * EdgeFractionY * 0.5f;
	const FLOAT EdgeSizeU = UL * 0.5f;
	const FLOAT EdgeSizeV = VL * 0.5f;

	const FLOAT ColumnSizeX[3] = {	EdgeSizeX,	AWidth - EdgeSizeX * 2,		EdgeSizeX	};
	const FLOAT ColumnSizeU[3] = {	EdgeSizeU,	0.0f,						EdgeSizeU	};

	const FLOAT RowSizeY[3] = {		EdgeSizeY,	AHeight - EdgeSizeY * 2,	EdgeSizeY	};
	const FLOAT RowSizeV[3] = {		EdgeSizeV,	0.0f,						EdgeSizeV	};

	// Draw each row, starting from the top.
	FLOAT RowY = Top;
	FLOAT RowV = V;
	for(INT RowIndex = 0;RowIndex < 3;RowIndex++)
	{
		// Draw each column in the row, starting from the left.
		FLOAT ColumnX = Left;
		FLOAT ColumnU = U;
		for(INT ColumnIndex = 0;ColumnIndex < 3;ColumnIndex++)
		{
			// The tile may not be stretched on all axes, so some rows or columns will have zero size, and don't need to be rendered.
			if(ColumnSizeX[ColumnIndex] > 0.0f && RowSizeY[RowIndex] > 0.0f)
			{
				DrawTile(Tex,ColumnX,RowY,ColumnSizeX[ColumnIndex],RowSizeY[RowIndex],ColumnU,RowV,ColumnSizeU[ColumnIndex],RowSizeV[RowIndex],DrawColor);

				ColumnX += ColumnSizeX[ColumnIndex];
				ColumnU += ColumnSizeU[ColumnIndex];
			}
		}

		RowY += RowSizeY[RowIndex];
		RowV += RowSizeV[RowIndex];
	}
}

void UCanvas::execDrawRotatedTile(FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture,Tex);
	P_GET_ROTATOR(Rotation);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_GET_FLOAT_OPTX(AnchorX,0.5);
	P_GET_FLOAT_OPTX(AnchorY,0.5);

	P_FINISH;
	DrawRotatedTile(Tex, Rotation, XL, YL, U, V, UL, VL, AnchorX, AnchorY);
}

void UCanvas::DrawRotatedTile(UTexture* Tex, FRotator Rotation, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLOAT AnchorX, FLOAT AnchorY)
{
	if(!Tex)
	{
		return;
	}
	// Figure out where we are drawing
	FVector Position( OrgX + CurX, OrgY + CurY, 0 );

	// Anchor is the center of the tile
	FVector AnchorPos( XL * AnchorX, YL * AnchorY, 0 );

	FRotationMatrix RotMatrix( Rotation );
	FMatrix TransformMatrix = FTranslationMatrix(-AnchorPos) * RotMatrix * FTranslationMatrix(AnchorPos);

	// translate the matrix back to origin, apply the rotation matrix, then transform back to the current position
 	FMatrix FinalTransform = FTranslationMatrix(-Position) * TransformMatrix * FTranslationMatrix(Position);

	Canvas->PushRelativeTransform(FinalTransform);

	DrawTile(Tex,OrgX+CurX,OrgY+CurY,XL,YL,U,V,UL,VL,DrawColor);

	Canvas->PopTransform();
}
IMPLEMENT_FUNCTION( UCanvas, -1, execDrawRotatedTile);

void UCanvas::execDrawRotatedMaterialTile(FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UMaterialInterface,Material);
	P_GET_ROTATOR(Rotation);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT_OPTX(U,0.f);
	P_GET_FLOAT_OPTX(V,0.f);
	P_GET_FLOAT_OPTX(UL,0.f);
	P_GET_FLOAT_OPTX(VL,0.f);
	P_GET_FLOAT_OPTX(AnchorX,0.5);
	P_GET_FLOAT_OPTX(AnchorY,0.5);
	P_FINISH;

	DrawRotatedMaterialTile(Material, Rotation, XL, YL, U, V, UL, VL, AnchorX, AnchorY);
}

void UCanvas::DrawRotatedMaterialTile(UMaterialInterface* Tex, FRotator Rotation, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLOAT AnchorX, FLOAT AnchorY)
{
	if(!Tex)
	{
		return;
	}

	UMaterial* Mat = Tex->GetMaterial();

	if (UL <= 0.0f)
	{
		UL = 1.0f;
	}
	if (VL <= 0.0f) 
	{
		VL = 1.0f;
	}

	// Figure out where we are drawing
	FVector Position( OrgX + CurX, OrgY + CurY, 0 );

	// Anchor is the center of the tile
	FVector AnchorPos( XL * AnchorX, YL * AnchorY, 0 );

	FRotationMatrix RotMatrix( Rotation );
	FMatrix TransformMatrix;
	TransformMatrix = FTranslationMatrix(-AnchorPos) * RotMatrix * FTranslationMatrix(AnchorPos);

	// translate the matrix back to origin, apply the rotation matrix, then transform back to the current position
 	FMatrix FinalTransform = FTranslationMatrix(-Position) * TransformMatrix * FTranslationMatrix(Position);

	Canvas->PushRelativeTransform(FinalTransform);
	DrawMaterialTile(Tex,OrgX+CurX,OrgY+CurY,XL,YL,U,V,UL,VL);
	Canvas->PopTransform();

}
IMPLEMENT_FUNCTION( UCanvas, -1, execDrawRotatedMaterialTile);

void UCanvas::execDraw2DLine(FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(X1);
	P_GET_FLOAT(Y1);
	P_GET_FLOAT(X2);
	P_GET_FLOAT(Y2);
	P_GET_STRUCT(FColor,LineColor);
	P_FINISH;

	X1+= OrgX;
	X2+= OrgX;
	Y1+= OrgY;
	Y2+= OrgY;

	DrawLine2D(Canvas, FVector2D(X1, Y1), FVector2D(X2, Y2), LineColor);
}

IMPLEMENT_FUNCTION(UCanvas, -1, execDraw2DLine);


void UCanvas::execDrawTextureLine(FFrame& Stack, RESULT_DECL)
{
	P_GET_VECTOR(StartPoint);
	P_GET_VECTOR(EndPoint);
	P_GET_FLOAT(Perc);
	P_GET_FLOAT(Width);
	P_GET_STRUCT(FColor,LineColor);
	P_GET_OBJECT(UTexture,LineTexture);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_FINISH;

	DrawTextureLine(StartPoint, EndPoint, Perc, Width, LineColor, LineTexture,U,V,UL,VL);
}
IMPLEMENT_FUNCTION(UCanvas, -1, execDrawTextureLine);

void UCanvas::DrawTextureLine(FVector StartPoint, FVector EndPoint, FLOAT Perc, FLOAT Width, FColor LineColor, UTexture* Tex, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL)
{
	if (!Tex)
	{
		Tex = DefaultTexture;
	}

	FRotator R(0,0,0);
	FLOAT Dist;
	FVector Dir;

	Dir = (EndPoint - StartPoint).SafeNormal();

	DrawColor = LineColor;

	R.Yaw = (StartPoint - EndPoint).SafeNormal().Rotation().Yaw;
	Dist = (StartPoint - EndPoint).Size2D();
	Dir *= Dist * 0.5;

	Dist -= Perc;

	CurX = StartPoint.X + Dir.X - (Dist *0.5);
	CurY = StartPoint.Y + Dir.Y - 1;

	DrawRotatedTile(Tex, R, Dist, Width, U, V, UL, VL, 0.5, 0.5);
}

void UCanvas::execDrawTextureDoubleLine(FFrame& Stack, RESULT_DECL)
{
	P_GET_VECTOR(StartPoint);
	P_GET_VECTOR(EndPoint);
	P_GET_FLOAT(Perc);
	P_GET_FLOAT(Spacing);
	P_GET_FLOAT(Width);
	P_GET_STRUCT(FColor, LineColor);
	P_GET_STRUCT(FColor, AltLineColor);
	P_GET_OBJECT(UTexture,LineTexture);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_FINISH;

	DrawTextureDoubleLine(StartPoint, EndPoint, Perc, Spacing, Width, LineColor, AltLineColor, LineTexture,U,V,UL,VL);
}
IMPLEMENT_FUNCTION(UCanvas, -1, execDrawTextureDoubleLine);

void UCanvas::DrawTextureDoubleLine(FVector StartPoint, FVector EndPoint, FLOAT Perc, FLOAT Spacing, FLOAT Width, FColor LineColor, FColor AltLineColor, UTexture *Tex, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL)
{
	if (!Tex)
	{
		Tex = DefaultTexture;
	}

	FRotator R(0,0,0);
	FLOAT Dist;
	FVector Dir,Center, Ofst;

	Dir = (EndPoint - StartPoint).SafeNormal();
	R.Yaw = (StartPoint - EndPoint).SafeNormal().Rotation().Yaw;
	Dist = (StartPoint - EndPoint).Size2D();

	Center.X = StartPoint.X + (Dir.X * Dist * 0.5);
	Center.Y = StartPoint.Y + (Dir.Y * Dist * 0.5);

	Dist -= Perc;
	Ofst = Dir * (Spacing + Width);

	CurX = Center.X+(Ofst.Y) - (Dist * 0.5);
	CurY = Center.Y+(Ofst.X * -1) - Width;

	DrawColor = LineColor;
	DrawRotatedTile(Tex, R, Dist, Width, U, V, UL, VL, 0.5, 0.5);

	Ofst = Dir * Spacing;

	CurX = Center.X-(Ofst.Y) - (Dist * 0.5);
	CurY = Center.Y-(Ofst.X * -1) - Width;

	DrawColor = AltLineColor;
	DrawRotatedTile(Tex, R, Dist, Width, U, V, UL, VL, 0.5, 0.5);
}

