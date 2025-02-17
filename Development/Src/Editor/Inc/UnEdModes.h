/*=============================================================================
	UnEdModes : Classes for handling the various editor modes
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

enum EEditorMode
{
	/** Gameplay, editor disabled. */
	EM_None = 0,

	/** Camera movement, actor placement. */
	EM_Default,

	/** Terrain editing. */
	EM_TerrainEdit,

	/** Geometry editing mode. */
	EM_Geometry,

	/** Interpolation editing. */
	EM_InterpEdit,

	/** Texture alignment via the widget. */
	EM_Texture,
	EM_CoverEdit,			// Cover positioning/editing
};

class FScopedTransaction;
class WxModeBar;

/**
 * Base class for all editor modes.
 */
class FEdMode : public FSerializableObject
{
public:
	FEdMode();
	virtual ~FEdMode();

	virtual UBOOL MouseMove(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,INT x, INT y);
	virtual UBOOL InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event);
	virtual UBOOL InputAxis(FEditorLevelViewportClient* InViewportClient,FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime);
	virtual UBOOL InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale);
	virtual UBOOL StartTracking();
	virtual UBOOL EndTracking();

	virtual void Tick(FEditorLevelViewportClient* ViewportClient,FLOAT DeltaTime);

	virtual void ActorMoveNotify() {}
	virtual void CamMoveNotify(FEditorLevelViewportClient* ViewportClient) {}
	virtual void ActorSelectionChangeNotify() {}
	virtual void ActorPropChangeNotify() {}
	virtual void MapChangeNotify() {}
	virtual UBOOL ShowModeWidgets() const { return 1; }

	/** If the EdMode is handling InputDelta (ie returning true from it), this allows a mode to indicated whether or not the Widget should also move. */
	virtual UBOOL AllowWidgetMove() { return true; }

	virtual UBOOL ShouldDrawBrushWireframe( AActor* InActor ) const { return 1; }
	virtual UBOOL GetCustomDrawingCoordinateSystem( FMatrix& InMatrix, void* InData ) { return 0; }
	virtual UBOOL GetCustomInputCoordinateSystem( FMatrix& InMatrix, void* InData ) { return 0; }

	/**
	 * Allows each mode to customize the axis pieces of the widget they want drawn.
	 *
	 * @param	InwidgetMode	The current widget mode
	 *
	 * @return					A bitfield comprised of AXIS_* values
	 */
	virtual INT GetWidgetAxisToDraw( FWidget::EWidgetMode InWidgetMode ) const;

	/**
	 * Allows each mode/tool to determine a good location for the widget to be drawn at.
	 */
	virtual FVector GetWidgetLocation() const;

	/**
	 * Lets the mode determine if it wants to draw the widget or not.
	 */
	virtual UBOOL ShouldDrawWidget() const;
	virtual void UpdateInternalData() {}

	virtual FVector GetWidgetNormalFromCurrentAxis( void* InData );

	void ClearComponent();
	void UpdateComponent();
	UEditorComponent* GetComponent() const { return Component; }

	virtual void Enter();
	virtual void Exit();
	virtual UTexture2D* GetVertexTexture() { return GWorld->GetWorldInfo()->BSPVertex; }
	
	/**
	 * Lets each tool determine if it wants to use the editor widget or not.  If the tool doesn't want to use it,
	 * it will be fed raw mouse delta information (not snapped or altered in any way).
	 */
	virtual UBOOL UsesWidget() const;

	/**
	 * Lets each mode selectively exclude certain widget types.
	 */
	virtual UBOOL UsesWidget(FWidget::EWidgetMode CheckMode) const { return TRUE; }

	virtual void SoftSelect() {}
	virtual void SoftSelectClear() {}

	virtual void PostUndo() {}

	virtual UBOOL ExecDelete();

	/**
	 * Lets each mode/tool handle box selection in its own way.
	 *
	 * @param	InBox	The selection box to use, in worldspace coordinates.
	 * @return		TRUE if something was selected/deselected, FALSE otherwise.
	 */
	UBOOL BoxSelect( FBox& InBox, UBOOL InSelect = TRUE );
	void SelectNone();

	/**
	 * Used to serialize any UObjects references.
	 *
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Component;
	}

	virtual UBOOL HandleClick(HHitProxy *HitProxy, const FViewportClick &Click) { return 0; }
	virtual UBOOL Select( AActor* InActor, UBOOL bInSelected ) { return 0; }

	/** Returns the editor mode. */
	EEditorMode GetID() const { return ID; }

	friend class FEditorModeTools;

	// Tools

	void SetCurrentTool( EModeTools InID );
	void SetCurrentTool( FModeTool* InModeTool );
	FModeTool* FindTool( EModeTools InID );

	const TArray<FModeTool*>& GetTools() const		{ return Tools; }

	virtual void CurrentToolChanged() {}

	/** Returns the current tool. */
	//@{
	FModeTool* GetCurrentTool()				{ return CurrentTool; }
	const FModeTool* GetCurrentTool() const	{ return CurrentTool; }
	//@}

	/** @name Settings */
	//@{
	FToolSettings* Settings;
	virtual const FToolSettings* GetSettings() const;
	//@}

	/** @name Current widget axis. */
	//@{
	void SetCurrentWidgetAxis(EAxis InAxis)		{ CurrentWidgetAxis = InAxis; }
	EAxis GetCurrentWidgetAxis() const			{ return CurrentWidgetAxis; }
	//@}

	/**
	 * @name Mode bar.
	 * The bar to display at the top of the editor when this mode is current.
	 */
	//@{
	void SetModeBar(WxModeBar* InModeBar)		{ check( InModeBar); ModeBar = InModeBar; }
	void SetModeBar(FEdMode& EditorMode)		{ ModeBar = EditorMode.ModeBar; }

	WxModeBar* GetModeBar()						{ return ModeBar; }
	const WxModeBar* GetModeBar() const			{ return ModeBar; }
	//@}

	/** @name Rendering */
	//@{
	/** Draws translucent polygons on brushes and volumes. */
	virtual void Render(const FSceneView* View,FViewport* Viewport,FPrimitiveDrawInterface* PDI);
	//void DrawGridSection(INT ViewportLocX,INT ViewportGridY,FVector* A,FVector* B,FLOAT* AX,FLOAT* BX,INT Axis,INT AlphaCase,FSceneView* View,FPrimitiveDrawInterface* PDI);

	/** Overlays the editor hud (brushes, drag tools, static mesh vertices, etc*. */
	virtual void DrawHUD(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,const FSceneView* View,FCanvas* Canvas);
	//@}

	/**
	 * Called when attempting to duplicate the selected actors using a normal key bind,
	 * return TRUE to prevent normal duplication.
	 */
	virtual UBOOL HandleDuplicate() { return FALSE; }

	/**
	 * Called when attempting to duplicate the selected actors by alt+dragging,
	 * return TRUE to prevent normal duplication.
	 */
	virtual UBOOL HandleDragDuplicate() { return FALSE; }

protected:
	/** Description for the editor to display. */
	FString Desc;

	/** EMaskedBitmap pointers. */
	void *BitmapOn, *BitmapOff;

	/** The scene component specific to this mode. */
	UEdModeComponent* Component;

	/** The bar to display at the top of the editor when this mode is current. */
	WxModeBar* ModeBar;

	/** The current axis that is being dragged on the widget. */
	EAxis CurrentWidgetAxis;

	/** Optional array of tools for this mode. */
	TArray<FModeTool*> Tools;

	/** The tool that is currently active within this mode. */
	FModeTool* CurrentTool;

	/** The enumerated ID. */
	EEditorMode ID;
};

/*------------------------------------------------------------------------------
    Default.
------------------------------------------------------------------------------*/

/**
 * The default editing mode.  User can work with BSP and the builder brush.
 */
class FEdModeDefault : public FEdMode
{
public:
	FEdModeDefault();
};

/*------------------------------------------------------------------------------
    Vertex Editing.
------------------------------------------------------------------------------*/

/**
 * Allows the editing of vertices on BSP brushes.
 */
class FSelectedVertex
{
public:
	FSelectedVertex( ABrush* InBrush, FVector* InVertex ):
	  Brush( InBrush ),
	  Vertex( InVertex )
	{}

	/** The brush containing the vertex. */
	ABrush* Brush;

	/** The vertex itself. */
	FVector* Vertex;

private:
	FSelectedVertex();
};

/*------------------------------------------------------------------------------
	Geometry Editing.	
------------------------------------------------------------------------------*/

/**
 * Allows MAX "edit mesh"-like editing of BSP geometry.
 */
class FEdModeGeometry : public FEdMode
{
public:
	FEdModeGeometry();
	virtual ~FEdModeGeometry();

	virtual void CurrentToolChanged();

	virtual void Render(const FSceneView* View,FViewport* Viewport,FPrimitiveDrawInterface* PDI);
	virtual UBOOL ShowModeWidgets() const;

	/** If the actor isn't selected, we don't want to interfere with it's rendering. */
	virtual UBOOL ShouldDrawBrushWireframe( AActor* InActor ) const;
	virtual UBOOL GetCustomDrawingCoordinateSystem( FMatrix& InMatrix, void* InData );
	virtual UBOOL GetCustomInputCoordinateSystem( FMatrix& InMatrix, void* InData );
	virtual void Enter();
	virtual void Exit();
	virtual void ActorSelectionChangeNotify();
	virtual void MapChangeNotify();
	void SelectNone();
	void UpdateModifierWindow();
	virtual void Serialize( FArchive &Ar );

	virtual FVector GetWidgetLocation() const;

	/**
	 * Compiles geometry mode information from the selected brushes.
	 */
	void GetFromSource();
	
	/**
	 * Changes the source brushes to match the current geometry data.
	 */
	void SendToSource();
	UBOOL FinalizeSourceData();
	virtual void PostUndo();
	virtual UBOOL ExecDelete();
	virtual void UpdateInternalData();

	/** Applies delta information to a single vertex. */
	void ApplyToVertex( FGeomVertex* InVtx, FVector& InDrag, FRotator& InRot, FVector& InScale );


	void RenderSingleObject( const FGeomObject* InObject, const FSceneView *InView, FPrimitiveDrawInterface* InPDI );
	void RenderSinglePoly( const FGeomPoly* InPoly, const FSceneView *InView, FPrimitiveDrawInterface* InPDI, FDynamicMeshBuilder* MeshBuilder );
	void RenderSingleEdge( const FGeomEdge* InEdge, FColor InColor, const FSceneView *InView, FPrimitiveDrawInterface* InPDI, UBOOL InShowEdgeMarkers );
	void RenderSinglePolyWireframe( const FGeomPoly* InPoly, FColor InColor, const FSceneView *InView, FPrimitiveDrawInterface* InPDI );

	void RenderObject( const FSceneView* View, FPrimitiveDrawInterface* PDI );
	void RenderPoly( const FSceneView* View, FPrimitiveDrawInterface* PDI );
	void RenderEdge( const FSceneView* View, FPrimitiveDrawInterface* PDI );
	void RenderVertex( const FSceneView* View, FPrimitiveDrawInterface* PDI );

	/**
	 * Applies soft selection to the currently selected vertices.
	 */
	virtual void SoftSelect();

	/**
	 * Called when soft selection is turned off.  This clears out any extra data
	 * that the geometry data might contain related to soft selection.
	 */
	virtual void SoftSelectClear();

	void ShowModifierWindow(UBOOL bShouldShow);

	/** @name GeomObject iterators */
	//@{
	typedef TArray<FGeomObject*>::TIterator TGeomObjectIterator;
	typedef TArray<FGeomObject*>::TConstIterator TGeomObjectConstIterator;

	TGeomObjectIterator			GeomObjectItor()			{ return TGeomObjectIterator( GeomObjects ); }
	TGeomObjectConstIterator	GeomObjectConstItor() const	{ return TGeomObjectConstIterator( GeomObjects ); }

	// @todo DB: Get rid of these; requires changes to FGeomBase::ParentObjectIndex
	FGeomObject* GetGeomObject(INT Index)					{ return GeomObjects( Index ); }
	const FGeomObject* GetGeomObject(INT Index) const		{ return GeomObjects( Index ); }
	//@}

	WxGeomModifiers& GetModifierWindow()	{ return *ModifierWindow; }

protected:
	/** 
	 * Custom data compiled when this mode is entered, based on currently
	 * selected brushes.  This data is what is drawn and what the LD
	 * interacts with while in this mode.  Changes done here are
	 * reflected back to the real data in the level at specific times.
	 */
	TArray<FGeomObject*> GeomObjects;

	/** The modifier window that pops up when the mode is active. */
	WxGeomModifiers* ModifierWindow;
};

/*------------------------------------------------------------------------------
	Terrain Editing.
------------------------------------------------------------------------------*/

/**
 * Allows editing of terrain heightmaps and their layers.
 */
class FEdModeTerrainEditing : public FEdMode
{
public:
	FEdModeTerrainEditing();

	virtual const FToolSettings* GetSettings() const;
	
	virtual UBOOL AllowWidgetMove()	{ return FALSE;	}

	void DrawTool( const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI, 
		class ATerrain* Terrain, FVector& Location, FLOAT InnerRadius, FLOAT OuterRadius, 
		TArray<ATerrain*>& Terrains );
	void DrawToolCircle( const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI, 
		class ATerrain* Terrain, FVector& Location, FLOAT Radius, TArray<ATerrain*>& Terrains, 
		UBOOL bMirror = FALSE );
	void DrawToolCircleBallAndSticks( const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI, 
		class ATerrain* Terrain, FVector& Location, FLOAT InnerRadius, FLOAT OuterRadius, 
		TArray<ATerrain*>& Terrains, UBOOL bMirror = FALSE );
	void DrawBallAndStick( const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI, 
		class ATerrain* Terrain, FVector& Location, FLOAT Strength = 1.0f );
	virtual void Render(const FSceneView* View,FViewport* Viewport,FPrimitiveDrawInterface* PDI);

	void DetermineMirrorLocation( FPrimitiveDrawInterface* PDI, class ATerrain* Terrain, FVector& Location );
	INT GetMirroredValue_Y(ATerrain* Terrain, INT InY, UBOOL bPatchOperation = FALSE);
	INT GetMirroredValue_X(ATerrain* Terrain, INT InX, UBOOL bPatchOperation = FALSE);

	virtual UBOOL InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event);
	virtual UBOOL HandleClick(HHitProxy *HitProxy, const FViewportClick &Click);

	UBOOL			bPerToolSettings;
	FColor			ModeColor;
	UBOOL			bConstrained;
	UBOOL			bShowBallAndSticks;

	FLinearColor	ToolColor;
	FLinearColor	MirrorColor;

	UTexture2D*		BallTexture;

	// To avoid re-calculating multiple times a frame?
	FVector			MirrorLocation;

	ATerrain*		CurrentTerrain;

	// location of the terrain under mouse cursor
	FVector			ToolHitLocation;
	// terrains currently contained in the tool circle
	TArray<class ATerrain*> ToolTerrains;
};

/*------------------------------------------------------------------------------
	Texture.
------------------------------------------------------------------------------*/

/**
 * Allows texture alignment on BSP surfaces via the widget.
 */
class FEdModeTexture : public FEdMode
{
public:
	FEdModeTexture();
	virtual ~FEdModeTexture();

	/** Stores the coordinate system that was active when the mode was entered so it can restore it later. */
	ECoordSystem SaveCoordSystem;

	virtual void Enter();
	virtual void Exit();
	virtual FVector GetWidgetLocation() const;
	virtual UBOOL ShouldDrawWidget() const;
	virtual UBOOL GetCustomDrawingCoordinateSystem( FMatrix& InMatrix, void* InData );
	virtual UBOOL GetCustomInputCoordinateSystem( FMatrix& InMatrix, void* InData );
	virtual INT GetWidgetAxisToDraw( FWidget::EWidgetMode InWidgetMode ) const;
	virtual UBOOL StartTracking();
	virtual UBOOL EndTracking();
	virtual UBOOL AllowWidgetMove() { return FALSE; }

protected:
	/** The current transaction. */
	FScopedTransaction*		ScopedTransaction;
};

class FEdModeCoverEdit : public FEdMode
{
	UBOOL bCanAltDrag;

public:
	FEdModeCoverEdit();
	virtual ~FEdModeCoverEdit();

	void Enter();
	void Exit();

	virtual UBOOL InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event);
	virtual UBOOL InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale);
	virtual UBOOL Select( AActor *InActor, UBOOL bInSelected );
	virtual UBOOL UsesWidget() const { return TRUE; }
	virtual UBOOL UsesWidget(FWidget::EWidgetMode CheckMode) const;
	virtual FVector GetWidgetLocation() const;
	virtual UBOOL HandleDragDuplicate();
	virtual UBOOL HandleDuplicate();
	virtual UBOOL HandleClick(HHitProxy *HitProxy, const FViewportClick &Click);
	virtual void DrawHUD(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,const FSceneView* View,FCanvas* Canvas);
};

/*------------------------------------------------------------------------------
	FEditorModeTools
------------------------------------------------------------------------------*/

/**
 * A helper class to store the state of the various editor modes.
 */
class FEditorModeTools :
	// Interface for receiving notifications of events
	public FCallbackEventDevice
{
public:
	FEditorModeTools();
	virtual ~FEditorModeTools();

	void Init();
	void SetCurrentMode( EEditorMode InID );
	FEdMode* FindMode( EEditorMode InID );

	/**
	 * Returns TRUE if the current mode is not the specified ModeID.  Also optionally warns the user.
	 *
	 * @param	ModeID			The editor mode to query.
	 * @param	bNotifyUser		If TRUE, inform the user that the requested operation cannot be performed in the specified mode.
	 * @return					TRUE if the current mode is not the specified mode.
	 */
	UBOOL EnsureNotInMode(EEditorMode ModeID, UBOOL bNotifyUser) const;

	FMatrix GetCustomDrawingCoordinateSystem();
	FMatrix GetCustomInputCoordinateSystem();
	
	FEdMode*& GetCurrentMode()					{ return CurrentMode; }
	const FEdMode* GetCurrentMode() const		{ return CurrentMode; }

	const FModeTool* GetCurrentTool() const		{ return GetCurrentMode()->GetCurrentTool(); }
	EEditorMode GetCurrentModeID() const		{ return GetCurrentMode()->GetID(); }

	void SetShowWidget( UBOOL InShowWidget )	{ bShowWidget = InShowWidget; }
	UBOOL GetShowWidget() const					{ return bShowWidget; }

	void SetMouseLock( UBOOL InMouseLock )		{ bMouseLock = InMouseLock; }
	UBOOL GetMouseLock() const					{ return bMouseLock; }

	/**
	 * Returns a good location to draw the widget at.
	 */
	FVector GetWidgetLocation() const;

	/**
	 * Changes the current widget mode.
	 */
	void SetWidgetMode( FWidget::EWidgetMode InWidgetMode );

	/**
	 * Allows you to temporarily override the widget mode.  Call this function again
	 * with WM_None to turn off the override.
	 */
	void SetWidgetModeOverride( FWidget::EWidgetMode InWidgetMode );

	/**
	 * Retrieves the current widget mode, taking overrides into account.
	 */
	FWidget::EWidgetMode GetWidgetMode() const;

	/**
	 * Sets a bookmark in the levelinfo file, allocating it if necessary.
	 */
	void SetBookMark( INT InIndex, FEditorLevelViewportClient* InViewportClient );

	/**
	 * Retrieves a bookmark from the list.
	 */
	void JumpToBookMark( INT InIndex );

	/**
	 * Serializes the components for all modes.
	 */
	void Serialize( FArchive &Ar );

	/**
	 * Loads the state that was saved in the INI file
	 */
	void LoadConfig(void);

	/**
	 * Saves the current state to the INI file
	 */
	void SaveConfig(void);

// FCallbackEventDevice interface

	/**
	 * Handles notification of an object selection change. Updates the
	 * Pivot and Snapped location values based upon the selected actor
	 *
	 * @param InType the event that was fired
	 * @param InObject the object associated with this event
	 */
	void Send(ECallbackEventType InType,UObject* InObject);


	/** A list of all available editor modes. */
	TArray<FEdMode*> Modes;

	UBOOL PivotShown, Snapping;
	FVector PivotLocation, SnappedLocation, GridBase;

	/** The coordinate system the widget is operating within. */
	ECoordSystem CoordSystem;		// The coordinate system the widget is operating within

	/** Draws in the top level corner of all FEditorLevelViewportClient windows (can be used to relay info to the user). */
	FString InfoString;

protected:
	/** The editor mode currently in use. */
	FEdMode* CurrentMode;

	/** The mode that the editor viewport widget is in. */
	FWidget::EWidgetMode WidgetMode;

	/** If the widget mode is being overridden, this will be != WM_None. */
	FWidget::EWidgetMode OverrideWidgetMode;

	/** If 1, draw the widget and let the user interact with it. */
	UBOOL bShowWidget;

	/** If 1, the mouse is locked from moving actors but button presses are still accepted. */
	UBOOL bMouseLock;
};
