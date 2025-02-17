/*=============================================================================
	UnCanvas.h: Unreal canvas definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

struct FCanvasIcon
{
	class UTexture *Texture;
	FLOAT U, V, UL, VL;
};

class FBatchedElements;

/**
 * Encapsulates the canvas state.
 */
class FCanvas
{
public:	

	/** 
	 * Enum that describes what type of element we are currently batching.
	 */
	enum EElementType
	{
		ET_Line,
		ET_Triangle,
		ET_MAX
	};

	/**
	 * Enum for canvas features that are allowed
	 **/
	enum ECanvasAllowModes
	{
		// stencil masked regions
		Allow_MaskedRegions	= 1<<0,
		// depth testing
		Allow_DepthTest		= 1<<1,
		// flushing and rendering
		Allow_Flush			= 1<<2,
		// delete the render batches when rendering
		Allow_DeleteOnRender= 1<<3
	};

	/** 
	* Constructor.
	*/
	FCanvas(FRenderTarget* InRenderTarget,FHitProxyConsumer* InHitProxyConsumer);

	/** 
	* Destructor.
	*/
	~FCanvas();

	/**
	* Returns a FBatchedElements pointer to be used for adding vertices and primitives for rendering.
	* Adds a new render item to the sort element entry based on the current sort key.
	*
	* @param InElementType - Type of element we are going to draw.
	* @param InTexture - New texture that will be set.
	* @param InBlendMode - New blendmode that will be set.	* 
	* @return Returns a pointer to a FBatchedElements object.
	*/
	FBatchedElements* GetBatchedElements(EElementType InElementType, const FTexture* Texture=NULL, EBlendMode BlendMode=BLEND_MAX);
	
	/**
	* Generates a new FCanvasTileRendererItem for the current sortkey and adds it to the sortelement list of itmes to render
	*/
	void AddTileRenderItem(FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,const FMaterialRenderProxy* MaterialRenderProxy,FHitProxyId HitProxyId);
	
	/** 
	* Sends a message to the rendering thread to draw the batched elements. 
	* @param bForce - force the flush even if Allow_Flush is not enabled
	*/
	void Flush(UBOOL bForce=FALSE);

	/**
	 * Pushes a transform onto the canvas's transform stack, multiplying it with the current top of the stack.
	 * @param Transform - The transform to push onto the stack.
	 */
	void PushRelativeTransform(const FMatrix& Transform);

	/**
	 * Pushes a transform onto the canvas's transform stack.
	 * @param Transform - The transform to push onto the stack.
	 */
	void PushAbsoluteTransform(const FMatrix& Transform);

	/**
	 * Removes the top transform from the canvas's transform stack.
	 */
	void PopTransform();

	/**
	* Replace the base (ie. TransformStack(0)) transform for the canvas with the given matrix
	*
	* @param Transform - The transform to use for the base
	*/
	void SetBaseTransform(const FMatrix& Transform);

	/**
	* Generate a 2D projection for the canvas. Use this if you only want to transform in 2D on the XY plane
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcBaseTransform2D(UINT ViewSizeX, UINT ViewSizeY);

	/**
	* Generate a 3D projection for the canvas. Use this if you want to transform in 3D 
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @param NearPlane - Distance to the near clip plane
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcBaseTransform3D(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane);
	
	/**
	* Generate a view matrix for the canvas. Used for CalcBaseTransform3D
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @return Matrix for canvas view orientation
	*/
	static FMatrix CalcViewMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV);
	
	/**
	* Generate a projection matrix for the canvas. Used for CalcBaseTransform3D
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @param NearPlane - Distance to the near clip plane
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcProjectionMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane);

	/**
	* Get the current top-most transform entry without the canvas projection
	* @return matrix from transform stack. 
	*/
	FMatrix GetTransform() const 
	{ 
		return TransformStack.Top().GetMatrix() * TransformStack(0).GetMatrix().Inverse(); 
	}

	/** 
	* Get the bottom-most element of the transform stack. 
	* @return matrix from transform stack. 
	*/
	const FMatrix& GetBottomTransform() const 
	{ 
		return TransformStack(0).GetMatrix(); 
	}

	/**
	* Get the current top-most transform entry 
	* @return matrix from transform stack. 
	*/
	const FMatrix& GetFullTransform() const 
	{ 
		return TransformStack.Top().GetMatrix(); 
	}

	/**
	 * Copy the conents of the TransformStack from an existing canvas
	 *
	 * @param Copy	canvas to copy from
	 **/
	void CopyTransformStack(const FCanvas& Copy);

	/**
	* Toggles current depth testing state for the canvas. All batches
	* will render with depth testing against the depth buffer if enabled.
	*
	* @param bEnabled - if TRUE then depth testing is enabled
	*/
	void SetDepthTestingEnabled(UBOOL bEnabled);

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
	void PushMaskRegion( FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY );

	/**
	 * Replace the top element of the masking region stack with a new region
	 */
	void ReplaceMaskRegion( FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY );

	/**
	 * Remove the current masking region; if other masking regions were previously pushed onto the stack,
	 * the next one down will be activated.
	 */
	void PopMaskRegion();

	/**
	 * Sets the render target which will be used for subsequent canvas primitives.
	 */
	void SetRenderTarget(FRenderTarget* NewRenderTarget);	

	/**
	* Get the current render target for the canvas
	*/	
	FORCEINLINE FRenderTarget* GetRenderTarget() const 
	{ 
		return RenderTarget; 
	}

	/**
	* Marks render target as dirty so that it will be resolved to texture
	*/
	void SetRenderTargetDirty(UBOOL bDirty) 
	{ 
		bRenderTargetDirty = bDirty; 
	}

	/**
	* Sets the hit proxy which will be used for subsequent canvas primitives.
	*/ 
	void SetHitProxy(HHitProxy* HitProxy);

	// HitProxy Accessors.	

	FHitProxyId GetHitProxyId() const { return CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId(); }
	FHitProxyConsumer* GetHitProxyConsumer() const { return HitProxyConsumer; }
	UBOOL IsHitTesting() const { return HitProxyConsumer != NULL; }
	
	/**
	* Push sort key onto the stack. Rendering is done with the current sort key stack entry.
	*
	* @param InSortKey - key value to push onto the stack
	*/
	void PushDepthSortKey(INT InSortKey)
	{
		DepthSortKeyStack.Push(InSortKey);
	};
	/**
	* Pop sort key off of the stack.
	*
	* @return top entry of the sort key stack
	*/
	INT PopDepthSortKey()
	{
		INT Result = 0;
		if( DepthSortKeyStack.Num() > 0 )
		{
			Result = DepthSortKeyStack.Pop();
		}
		else
		{
			// should always have one entry
			PushDepthSortKey(0);
		}
		return Result;		
	};
	/**
	* Return top sort key of the stack.
	*
	* @return top entry of the sort key stack
	*/
	INT TopDepthSortKey()
	{
		checkSlow(DepthSortKeyStack.Num() > 0);
		return DepthSortKeyStack.Top();
	}

	/**
	 * Toggle allowed canvas modes
	 *
	 * @param InAllowedModes	New modes to set
	 **/
	void SetAllowedModes(DWORD InAllowedModes)
	{
		AllowedModes = InAllowedModes;
	}
	/**
	 * Accessor for allowed canvas modes
	 *
	 * @return current allowed modes
	 **/
	DWORD GetAllowedModes() const
	{
		return AllowedModes;
	}

	/**
	 * Determine if the canvas has dirty batches that need to be rendered
	 *
	 * @return TRUE if the canvas has any element to render
	 **/
	UBOOL HasBatchesToRender() const;

public:
	FLOAT AlphaModulate;

	/** Entry for the transform stack which stores a matrix and its CRC for faster comparisons */
	class FTransformEntry
	{
	public:
		FTransformEntry(const FMatrix& InMatrix)
			:	Matrix(InMatrix)
		{			
			MatrixCRC = appMemCrc(&Matrix,sizeof(FMatrix));
		}
		FORCEINLINE void SetMatrix(const FMatrix& InMatrix)
		{
			Matrix = InMatrix;
			MatrixCRC = appMemCrc(&Matrix,sizeof(FMatrix));
		}
		FORCEINLINE const FMatrix& GetMatrix() const
		{
			return Matrix;
		}
		FORCEINLINE DWORD GetMatrixCRC() const
		{
			return MatrixCRC;
		}
	private:
		FMatrix Matrix;
		DWORD MatrixCRC;
	};

private:
	/** Current render target used by the canvas */
	FRenderTarget* RenderTarget;
	/** TRUE to enable depth testing. Useful when using a canvas scene for primitives */
	UBOOL bEnableDepthTest;
	/** TRUE if the render target has been rendered to since last calling SetRenderTarget() */
	UBOOL bRenderTargetDirty;	
	/** Current hit proxy consumer */
	FHitProxyConsumer* HitProxyConsumer;
	/** Current hit proxy object */
	TRefCountPtr<HHitProxy> CurrentHitProxy;
	/** Stack of SortKeys. All rendering is done using the top most sort key */
	TArray<INT> DepthSortKeyStack;	
	/** Stack of matrices. Bottom most entry is the canvas projection */
	TArray<FTransformEntry> TransformStack;	
	/** Toggles for various canvas rendering functionality **/
	DWORD AllowedModes;
	
	/** 
	* Region on the canvas that should be masked
	*/
	struct FMaskRegion
	{
		FMaskRegion(FLOAT InX=-1,FLOAT InY=-1,FLOAT InSizeX=-1,FLOAT InSizeY=-1, const FMatrix& InTransform=FMatrix::Identity) 
			: X(InX), Y(InY), SizeX(InSizeX), SizeY(InSizeY), Transform(InTransform) 
		{}
		FORCEINLINE UBOOL IsEqual(const FMaskRegion& R) const		
		{ 
			return(	Abs(X-R.X) < KINDA_SMALL_NUMBER && 
					Abs(Y-R.Y) < KINDA_SMALL_NUMBER && 
					Abs(SizeX-R.SizeX) < KINDA_SMALL_NUMBER && 
					Abs(SizeY-R.SizeY) < KINDA_SMALL_NUMBER );
		}
		UBOOL IsValid() const 
		{ 
			return X >= -DELTA && Y >= -DELTA && SizeX >= -DELTA && SizeY >= -DELTA; 
		}
		FORCEINLINE UBOOL IsZero(FLOAT Tolerance=DELTA) const
		{
			//@todo - do we need to check tranform?
			return Abs(X) < Abs(Tolerance)
				&& Abs(Y) < Abs(Tolerance)
				&& Abs(SizeX) < Abs(Tolerance)
				&& Abs(SizeY) < Abs(Tolerance);
		}
		FLOAT X,Y,SizeX,SizeY;
		FMatrix Transform;
	};

	/**
	 * Stack of mask regions - top of stack (last element) is current canvas mask
	 */
	TArray<FMaskRegion> MaskRegionStack;

public:
	/**
	 * Get the top-most canvas masking region from the stack.
	 */
	FMaskRegion GetCurrentMaskRegion() const;
private:


	/** 
	* Contains all of the batched elements that need to be rendered at a certain depth sort key
	*/
	class FCanvasSortElement
	{
	public:
		/** 
		* Init constructor 
		*/
		FCanvasSortElement(INT InDepthSortKey=0)
			:	DepthSortKey(InDepthSortKey)
		{}

		/** 
		* Equality is based on sort key 
		*
		* @param Other - instance to compare against
		* @return TRUE if equal
		*/
		UBOOL operator==(const FCanvasSortElement& Other) const
		{
			return DepthSortKey == Other.DepthSortKey;
		}

		/** sort key for this set of render batch elements */
		INT DepthSortKey;
		/** list of batches that should be rendered at this sort key level */
		TArray<class FCanvasBaseRenderItem*> RenderBatchArray;
	};
	
	/** FCanvasSortElement compare class */
	IMPLEMENT_COMPARE_CONSTREF(FCanvasSortElement,UnCanvas,{ return A.DepthSortKey <= B.DepthSortKey ? 1 : -1;	})	
	/** Batched canvas elements to be sorted for rendering. Sort order is back-to-front */
	TArray<FCanvasSortElement> SortedElements;
	/** Map from sortkey to array index of SortedElements for faster lookup of existing entries */
	TMap<INT,INT> SortedElementLookupMap;
	
	/**
	* Get the sort element for the given sort key. Allocates a new entry if one does not exist
	*
	* @param DepthSortKey - the key used to find the sort element entry
	* @return sort element entry
	*/
	FCanvasSortElement& GetSortElement(INT DepthSortKey);

	/**
	* Setup the current masked region during flush
	*/
	void FlushSetMaskRegion();
	/**
	* Clear masked region during flush
	*/
	void FlushResetMaskRegion();
};

extern void Clear(FCanvas* Canvas,const FLinearColor& Color);

/**
 *	Draws a line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
extern void DrawLine(FCanvas* Canvas,const FVector& StartPos,const FVector& EndPos,const FLinearColor& Color);

/**
 * Draws a 2D line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
extern void DrawLine2D(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos,const FLinearColor& Color);

extern void DrawBox2D(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos,const FLinearColor& Color);

extern void DrawTile(
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
	const FTexture* Texture = NULL,
	UBOOL AlphaBlend = TRUE
	);

extern void DrawTile(
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
	);

extern void DrawTriangle2D(
	FCanvas* Canvas,
	const FVector2D& Position0,
	const FVector2D& TexCoord0,
	const FVector2D& Position1,
	const FVector2D& TexCoord1,
	const FVector2D& Position2,
	const FVector2D& TexCoord2,
	const FLinearColor& Color,
	const FTexture* Texture = NULL,
	UBOOL AlphaBlend = 1
	);

extern void DrawTriangle2D(
	FCanvas* Canvas,
	const FVector2D& Position0,
	const FVector2D& TexCoord0,
	const FVector2D& Position1,
	const FVector2D& TexCoord1,
	const FVector2D& Position2,
	const FVector2D& TexCoord2,
	const FMaterialRenderProxy* MaterialRenderProxy
	);

extern INT DrawStringCentered(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color);

extern INT DrawString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color, FLOAT XScale=1.0, FLOAT YScale=1.0, FLOAT HorizSpacingAdjust=0.0f, const FLOAT* ForcedViewportHeight=NULL );

extern void DrawStringWrapped( FCanvas* Canvas, UBOOL Draw, FLOAT CurX, FLOAT CurY, FLOAT& XL, FLOAT& YL, UFont* Font, const TCHAR* Text, FLinearColor DrawColor );

extern INT DrawShadowedString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color);

extern INT DrawStringMat(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,FLOAT XScale,FLOAT YScale,FLOAT HorizSpacingAdjust,const TCHAR* Text,class UFont* Font,UMaterialInterface* MatInst,const TCHAR* FontParam);

extern void StringSize(UFont* Font,INT& XL,INT& YL,const TCHAR* Format,...);

/*
	UCanvas
	A high-level rendering interface used to render objects on the HUD.
*/

struct Canvas_eventReset_Parms
{
	UBOOL			bKeepOrigin;
	Canvas_eventReset_Parms(EEventParm)
	{
	}
};

class UCanvas : public UObject
{
	DECLARE_CLASS(UCanvas,UObject,CLASS_Transient|CLASS_NoExport,Engine);
	NO_DEFAULT_CONSTRUCTOR(UCanvas);
public:

	// Variables.
	UFont*			Font;
	FLOAT			OrgX, OrgY;
	FLOAT			ClipX, ClipY;
	FLOAT			CurX, CurY;
	FLOAT			CurYL;
	FColor			DrawColor;
	BITFIELD		bCenter:1;
	BITFIELD		bNoSmooth:1;
	INT				SizeX, SizeY;
	FCanvas*		Canvas;
	FSceneView*		SceneView;
	FPlane			ColorModulate;
	UTexture2D*		DefaultTexture;

	// UCanvas interface.
	void Init();
	void Update();

	void DrawTile( UTexture* Tex, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& Color );
	void DrawMaterialTile( UMaterialInterface* Tex, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL );
	static void ClippedStrLen( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Text );
	void VARARGS WrappedStrLenf( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Fmt, ... );	
	void WrappedPrint( UBOOL Draw, INT& XL, INT& YL, UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Text );	
	void DrawTileStretched(UTexture* Tex, FLOAT Left, FLOAT Top, FLOAT AWidth, FLOAT AHeight, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLinearColor DrawColor,UBOOL bStretchHorizontally=1,UBOOL bStretchVertically=1,FLOAT ScalingFactor=1.0);

	void DrawTextureLine(FVector StartPoint, FVector EndPoint, FLOAT Perc, FLOAT Width, FColor LineColor, UTexture* Tex, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL);
	void DrawTextureDoubleLine(FVector StartPoint, FVector EndPoint, FLOAT Perc, FLOAT Spacing, FLOAT Width, FColor LineColor, FColor AltLineColor, UTexture *Tex, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL);

	void DrawRotatedTile(UTexture* Tex, FRotator Rotation, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLOAT AnchorX, FLOAT AnchorY);
	void DrawRotatedMaterialTile(UMaterialInterface* Tex, FRotator Rotation, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLOAT AnchorX, FLOAT AnchorY);

	void SetDrawColor(BYTE R, BYTE G, BYTE B, BYTE A);
	void SetPos( FLOAT PosX, FLOAT PosY );
	void DrawText(FString &Text);

	// Natives.
	DECLARE_FUNCTION(execDrawTile);
	DECLARE_FUNCTION(execDrawMaterialTile);
	DECLARE_FUNCTION(execDrawMaterialTileClipped);
	DECLARE_FUNCTION(execDrawColorizedTile);
	DECLARE_FUNCTION(execDrawTileStretched);
	DECLARE_FUNCTION(execDrawTileClipped);
	DECLARE_FUNCTION(execDrawText);
	DECLARE_FUNCTION(execDrawTextClipped);
	DECLARE_FUNCTION(execStrLen);
	DECLARE_FUNCTION(execTextSize);
	DECLARE_FUNCTION(execProject);
	DECLARE_FUNCTION(execPushTranslationMatrix);
	DECLARE_FUNCTION(execPopTransform);
	DECLARE_FUNCTION(execDrawRotatedTile);
	DECLARE_FUNCTION(execDrawRotatedMaterialTile);
	DECLARE_FUNCTION(execDraw2DLine);
	DECLARE_FUNCTION(execDrawTextureLine);
	DECLARE_FUNCTION(execDrawTextureDoubleLine);

	DECLARE_FUNCTION(execSetDrawColor);
	DECLARE_FUNCTION(execSetPos);


    void eventReset(UBOOL bKeepOrigin=false)
    {
		Canvas_eventReset_Parms Parms(EC_EventParm);
		Parms.bKeepOrigin = bKeepOrigin;
        ProcessEvent(FindFunctionChecked(TEXT("Reset")),&Parms);
    }
};

