/*=============================================================================
	Cascade.h: 'Cascade' particle editor
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __CASCADE_H__
#define __CASCADE_H__

#include "UnrealEd.h"
#include "CurveEd.h"
#include "TrackableWindow.h"

// NOTE: Do not enable the module dump at this time...
// Use the Enable/Disable feature of modules to 'rem
//#define _CASCADE_ENABLE_MODULE_DUMP_

// Module offsets for drawing emitter editor window...
#define CASC_OFFSET_REQUIREDMODULE	1
#define CASC_OFFSET_SPAWNMODULE		2
#define CASC_OFFSET_MODULES			3

/** Hit proxies */
struct HCascadeEmitterProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascadeEmitterProxy,HHitProxy);

	class UParticleEmitter* Emitter;

	HCascadeEmitterProxy(class UParticleEmitter* InEmitter) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter)
	{}
};

struct HCascadeModuleProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascadeModuleProxy,HHitProxy);

	class UParticleEmitter* Emitter;
	class UParticleModule* Module;

	HCascadeModuleProxy(class UParticleEmitter* InEmitter, class UParticleModule* InModule) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter),
		Module(InModule)
	{}
};

struct HCascadeEmitterEnableProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascadeEmitterEnableProxy,HHitProxy);

	class UParticleEmitter* Emitter;

	HCascadeEmitterEnableProxy(class UParticleEmitter* InEmitter) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter)
	{}
};

struct HCascadeDrawModeButtonProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascadeDrawModeButtonProxy,HHitProxy);

	class UParticleEmitter* Emitter;
	INT DrawMode;

	HCascadeDrawModeButtonProxy(class UParticleEmitter* InEmitter, INT InDrawMode) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter),
		DrawMode(InDrawMode)
	{}
};

struct HCascadeGraphButton : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascadeGraphButton,HHitProxy);

	class UParticleEmitter* Emitter;
	class UParticleModule* Module;

	HCascadeGraphButton(class UParticleEmitter* InEmitter, class UParticleModule* InModule) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter),
		Module(InModule)
	{}
};

struct HCascadeColorButtonProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascadeColorButtonProxy,HHitProxy);

	class UParticleEmitter* Emitter;
	class UParticleModule* Module;

	HCascadeColorButtonProxy(class UParticleEmitter* InEmitter, class UParticleModule* InModule) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter),
		Module(InModule)
	{}
};

struct HCascade3DDrawModeButtonProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascade3DDrawModeButtonProxy,HHitProxy);

	class UParticleEmitter* Emitter;
	class UParticleModule* Module;

	HCascade3DDrawModeButtonProxy(class UParticleEmitter* InEmitter, class UParticleModule* InModule) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter),
		Module(InModule)
	{}
};

struct HCascadeEnableButtonProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascadeEnableButtonProxy,HHitProxy);

	class UParticleEmitter* Emitter;
	class UParticleModule* Module;

	HCascadeEnableButtonProxy(class UParticleEmitter* InEmitter, class UParticleModule* InModule) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter),
		Module(InModule)
	{}
};

struct HCascade3DDrawModeOptionsButtonProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HCascade3DDrawModeOptionsButtonProxy,HHitProxy);

	class UParticleEmitter* Emitter;
	class UParticleModule* Module;

	HCascade3DDrawModeOptionsButtonProxy(class UParticleEmitter* InEmitter, class UParticleModule* InModule) :
		HHitProxy(HPP_UI),
		Emitter(InEmitter),
		Module(InModule)
	{}
};

/*-----------------------------------------------------------------------------
	FCascadePreviewViewportClient
-----------------------------------------------------------------------------*/

class FCascadePreviewViewportClient : public FEditorLevelViewportClient
{
public:
	class WxCascade*			Cascade;

	FPreviewScene				PreviewScene;
	FEditorCommonDrawHelper		DrawHelper;
	ULineBatchComponent*		LineBatcher;
	UStaticMeshComponent*		FloorComponent;

	FLOAT						TimeScale;
	FLOAT						TotalTime;

	FRotator					PreviewAngle;
	FLOAT						PreviewDistance;

	UBOOL						bDrawOriginAxes;
	UBOOL						bDrawParticleCounts;
	UBOOL						bDrawParticleEvents;
	UBOOL						bDrawParticleTimes;
	UBOOL						bDrawSystemDistance;
	UBOOL						bWireframe;
	UBOOL						bBounds;
	/** indicates that we should draw a wire sphere of WireSphereRadius centered around the origin (for matching effects to damage radii, etc) */
	UBOOL						bDrawWireSphere;
	/** radius of the sphere that should be drawn */
	FLOAT						WireSphereRadius;

	FColor						BackgroundColor;
	/** Internal variable indicating a screen shot should be captured */
	UBOOL						bCaptureScreenShot;

	/** Show post-process flags	*/
	INT							ShowPPFlags;

	FCascadePreviewViewportClient( class WxCascade* InCascade );

	// FEditorLevelViewportClient interface
	virtual FSceneInterface* GetScene() { return PreviewScene.GetScene(); }
	virtual FLinearColor GetBackgroundColor();
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);
	virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI);

	/**
	 * Configures the specified FSceneView object with the view and projection matrices for this viewport.
	 * @param	View		The view to be configured.  Must be valid.
	 * @return	A pointer to the view within the view family which represents the viewport's primary view.
	 */
	virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily);

	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);
	virtual UBOOL InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad=FALSE);

	virtual void Serialize(FArchive& Ar) 
	{ 
		Ar << Input;
		Ar << PreviewScene; 
	}

	// FCascadePreviewViewportClient interface

	void SetPreviewCamera(const FRotator& PreviewAngle, FLOAT PreviewDistance);
	void UpdateLighting();

	void SetupPostProcessChain();
};

/*-----------------------------------------------------------------------------
	WxCascadePreview
-----------------------------------------------------------------------------*/

// wxWindows Holder for FCascadePreviewViewportClient
class WxCascadePreview : public wxWindow
{
public:
	FCascadePreviewViewportClient* CascadePreviewVC;


	WxCascadePreview( wxWindow* InParent, wxWindowID InID, class WxCascade* InCascade );
	~WxCascadePreview();

	void OnSize( wxSizeEvent& In );

	DECLARE_EVENT_TABLE()
};



/*-----------------------------------------------------------------------------
	FCascadeEmitterEdViewportClient
-----------------------------------------------------------------------------*/

enum ECascadeModMoveMode
{
	CMMM_None,
	CMMM_Move,
	CMMM_Instance,
    CMMM_Copy
};

class FCascadeEmitterEdViewportClient : public FEditorLevelViewportClient
{
public:
	class WxCascade*	Cascade;

	ECascadeModMoveMode	CurrentMoveMode;
	FIntPoint			MouseHoldOffset; // Top-left corner of dragged module relative to mouse cursor.
	FIntPoint			MousePressPosition; // Location of cursor when mouse was pressed.
	UBOOL				bMouseDragging;
	UBOOL				bMouseDown;
	UBOOL				bPanning;
    UBOOL               bDrawModuleDump;

	FIntPoint			Origin2D;
	INT					OldMouseX, OldMouseY;

	enum Icons
	{
		CASC_Icon_Render_Normal	= 0,
		CASC_Icon_Render_Cross,
		CASC_Icon_Render_Point,
		CASC_Icon_Render_None,
		CASC_Icon_CurveEdit,
		CASC_Icon_3DDraw_Enabled,
		CASC_Icon_3DDraw_Disabled,
		CASC_Icon_Module_Enabled,
		CASC_Icon_Module_Disabled,
		CASC_Icon_COUNT
	};

	// Currently dragged module.
	class UParticleModule*	DraggedModule;
	TArray<class UParticleModule*>	DraggedModules;

	// If we abort a drag - here is where put the module back to (in the selected Emitter)
	INT						ResetDragModIndex;

protected:
	UTexture2D*		IconTex[CASC_Icon_COUNT];
	UTexture2D*		TexModuleDisabledBackground;

public:
	FCascadeEmitterEdViewportClient( class WxCascade* InCascade );
	~FCascadeEmitterEdViewportClient();

	virtual void Serialize(FArchive& Ar) { Ar << Input; }

	// FEditorLevelViewportClient interface
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);

	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);
	virtual UBOOL InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad=FALSE);

	// FCascadeEmitterEdViewportClient interface

	void FindDesiredModulePosition(const FIntPoint& Pos, class UParticleEmitter* &OutEmitter, INT &OutIndex);
	FIntPoint FindModuleTopLeft(class UParticleEmitter* Emitter, class UParticleModule* Module, FViewport* Viewport);

    void DrawEmitter(INT Index, INT XPos, UParticleEmitter* Emitter, FViewport* Viewport, FCanvas* Canvas);
	void DrawHeaderBlock(INT Index, INT XPos, UParticleEmitter* Emitter, FViewport* Viewport, FCanvas* Canvas);
	void DrawTypeDataBlock(INT XPos, UParticleEmitter* Emitter, FViewport* Viewport, FCanvas* Canvas);
	void DrawRequiredBlock(INT XPos, UParticleEmitter* Emitter, FViewport* Viewport, FCanvas* Canvas);
	void DrawSpawnBlock(INT XPos, UParticleEmitter* Emitter, FViewport* Viewport, FCanvas* Canvas);
	void DrawEventGeneratorBlock(INT XPos, UParticleEmitter* Emitter, FViewport* Viewport, FCanvas* Canvas);
    void DrawModule(INT XPos, INT YPos, UParticleEmitter* Emitter, UParticleModule* Module, 
		FViewport* Viewport, FCanvas* Canvas, UBOOL bDrawEnableButton = TRUE);
	void DrawModule(FCanvas* Canvas, UParticleModule* Module, FColor ModuleBkgColor, UParticleEmitter* Emitter);
    void DrawDraggedModule(UParticleModule* Module, FViewport* Viewport, FCanvas* Canvas);
    void DrawCurveButton(UParticleEmitter* Emitter, UParticleModule* Module, UBOOL bHitTesting, FCanvas* Canvas);
	void DrawColorButton(INT XPos, UParticleEmitter* Emitter, UParticleModule* Module, UBOOL bHitTesting, FCanvas* Canvas);
    void Draw3DDrawButton(UParticleEmitter* Emitter, UParticleModule* Module, UBOOL bHitTesting, FCanvas* Canvas);
    void DrawEnableButton(UParticleEmitter* Emitter, UParticleModule* Module, UBOOL bHitTesting, FCanvas* Canvas);

    void DrawModuleDump(FViewport* Viewport, FCanvas* Canvas);

	virtual void SetCanvas(INT X, INT Y);
	virtual void PanCanvas(INT DeltaX, INT DeltaY);

	FMaterialRenderProxy*	GetIcon(Icons eIcon);
	FMaterialRenderProxy*	GetModuleDisabledBackground();
	FTexture*			GetIconTexture(Icons eIcon);
	FTexture*			GetTextureDisabledBackground();

protected:
	void CreateIconMaterials();
};

/*-----------------------------------------------------------------------------
	WxCascadeEmitterEd
-----------------------------------------------------------------------------*/

// wxWindows Holder for FCascadePreviewViewportClient
class WxCascadeEmitterEd : public wxWindow
{
public:
	FCascadeEmitterEdViewportClient*	EmitterEdVC;

	wxScrollBar*						ScrollBar_Horz;
	wxScrollBar*						ScrollBar_Vert;
	INT									ThumbPos_Horz;
	INT									ThumbPos_Vert;

	WxCascadeEmitterEd( wxWindow* InParent, wxWindowID InID, class WxCascade* InCascade );
	~WxCascadeEmitterEd();

	void OnSize( wxSizeEvent& In );
	void UpdateScrollBar(INT Horz, INT Vert);
	void OnScroll(wxScrollEvent& In);
	void OnMouseWheel(wxMouseEvent& In);

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxCascadeMenuBar
-----------------------------------------------------------------------------*/

class WxCascadeMenuBar : public wxMenuBar
{
public:
	wxMenu	*EditMenu, *ViewMenu;

	WxCascadeMenuBar(WxCascade* Cascade);
	~WxCascadeMenuBar();
};

/*-----------------------------------------------------------------------------
	WxCascadeToolBar
-----------------------------------------------------------------------------*/

class WxCascadeToolBar : public wxToolBar
{
public:
	WxCascadeToolBar( wxWindow* InParent, wxWindowID InID );
	~WxCascadeToolBar();

	WxCascade*		Cascade;

	WxMaskedBitmap	SaveCamB, ResetSystemB, OrbitModeB;
	WxMaskedBitmap	WireframeB;
	WxMaskedBitmap	BoundsB;
	WxMaskedBitmap	PostProcessB;
	WxMaskedBitmap	ToggleGridB;
	WxMaskedBitmap	PlayB, PauseB;
	WxMaskedBitmap	Speed1B, Speed10B, Speed25B, Speed50B, Speed100B;
	WxMaskedBitmap	LoopSystemB;
	WxMaskedBitmap	RealtimeB;
	UBOOL			bRealtime;
	WxMaskedBitmap	BackgroundColorB;
	WxMaskedBitmap	WireSphereB;
	WxMaskedBitmap	RestartInLevelB;
	WxMaskedBitmap	UndoB;
	WxMaskedBitmap	RedoB;
	WxMaskedBitmap	PerformanceMeterB;

	WxMaskedBitmap	LODLow;
	WxMaskedBitmap	LODLower;
	WxMaskedBitmap	LODAddBefore;
	wxTextCtrl*		LODCurrent;
	wxTextCtrl*		LODTotal;
	WxMaskedBitmap	LODAddAfter;
	WxMaskedBitmap	LODHigher;
	WxMaskedBitmap	LODHigh;
	WxMaskedBitmap	LODDelete;
	WxMaskedBitmap	LODRegenerate;
	WxMaskedBitmap	LODRegenerateDuplicate;
    
	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	FCascadeNotifyHook
-----------------------------------------------------------------------------*/
class FCascadeNotifyHook : public FNotifyHook
{
public:
	virtual void NotifyDestroy(void* Src);

	WxCascade*	Cascade;
	void*		WindowOfInterest;
};

/*-----------------------------------------------------------------------------
	WxCascade
-----------------------------------------------------------------------------*/

class WxCascade : public WxTrackableFrame, public FNotifyHook, public FSerializableObject, FDockingParent, FTickableObject, FCurveEdNotifyInterface
{
public:
	FCascadeNotifyHook	PropWindowNotifyHook;

	WxCascadeMenuBar* MenuBar;
	WxCascadeToolBar* ToolBar;

	WxPropertyWindow*					PropertyWindow;
	FCascadePreviewViewportClient*		PreviewVC;
	FCascadeEmitterEdViewportClient*	EmitterEdVC;
	WxCascadeEmitterEd*					EmitterEdWindow;
	class WxCurveEditor*				CurveEd;

	FLOAT								SimSpeed;

	class UParticleSystem*			PartSys;

	// Resources for previewing system
	class UCascadePreviewComponent*	CascPrevComp;
//	class UParticleSystemComponent*	PartSysComp;
	class UCascadeParticleSystemComponent* PartSysComp;

	class UParticleModule*		SelectedModule;
	INT							SelectedModuleIndex;
	class UParticleEmitter*		SelectedEmitter;

	class UParticleModule*		CopyModule;
	class UParticleEmitter*		CopyEmitter;

	TArray<class UParticleModule*>	ModuleDumpList;

	// Whether to reset the simulation once it has completed a run and all particles have died.
	UBOOL					bResetOnFinish;
	UBOOL					bPendingReset;
	DOUBLE					ResetTime;
	UBOOL					bOrbitMode;
	UBOOL					bWireframe;
	UBOOL					bBounds;

	/** Used to enfore that all LODLevels in an emitter are either SubUV or not... */
	EParticleSubUVInterpMethod	PreviousInterpolationMethod;

	UObject*				CurveToReplace;
	TArray<FParticleCurvePair> DynParamCurves;

	UCascadeOptions*		EditorOptions;

	/** The post-process effect to apply to the preview window */
    FString					DefaultPostProcessName;
    UPostProcessChain*		DefaultPostProcess;

	// Static list of all ParticleModule subclasses.
	static TArray<UClass*>	ParticleModuleBaseClasses;

	static TArray<UClass*>	ParticleModuleClasses;
	static UBOOL			bParticleModuleClassesInitialized;

	WxCascade( wxWindow* InParent, wxWindowID InID, class UParticleSystem* InPartSys  );
	~WxCascade();

	/**
	 * This function is called when the window has been selected from within the ctrl + tab dialog.
	 */
	virtual void OnSelected();

	// wxFrame interface
	virtual wxToolBar* OnCreateToolBar(long style, wxWindowID id, const wxString& name);

	// FSerializableObject interface
	void Serialize(FArchive& Ar);

	// FTickableObject interface
	/**
	 * Pure virtual that must be overloaded by the inheriting class. It will
	 * be called from within UnLevTick.cpp after ticking all actors.
	 *
	 * @param DeltaTime	Game time passed since the last call.
	 */
	virtual void Tick( FLOAT DeltaTime );

	/**
	 * Pure virtual that must be overloaded by the inheriting class. It is
	 * used to determine whether an object is ready to be ticked. This is 
	 * required for example for all UObject derived classes as they might be
	 * loaded async and therefore won't be ready immediately.
	 *
	 * @return	TRUE if class is ready to be ticked, FALSE otherwise.
	 */
	virtual UBOOL IsTickable() const
	{
		return TRUE;
	}

	/**
	 * Used to determine if an object should be ticked when the game is paused.
	 * Defaults to false, as that mimics old behavior.
	 *
	 * @return TRUE if it should be ticked when paused, FALSE otherwise
	 */
	virtual UBOOL IsTickableWhenPaused() const
	{
		return TRUE;
	}

	/**
	 * Used to determine whether the object should be ticked in the editor.  Defaults to FALSE since
	 * that is the previous behavior.
	 *
	 * @return	TRUE if this tickable object can be ticked in the editor
	 */
	virtual UBOOL IsTickableInEditor() const
	{
		return TRUE;
	}

	// FCurveEdNotifyInterface
	/**
	 *	PreEditCurve
	 *	Called by the curve editor when N curves are about to change
	 *
	 *	@param	CurvesAboutToChange		An array of the curves about to change
	 */
	virtual void PreEditCurve(TArray<UObject*> CurvesAboutToChange);
	/**
	 *	PostEditCurve
	 *	Called by the curve editor when the edit has completed.
	 */
	virtual void PostEditCurve();
	/**
	 *	MovedKey
	 *	Called by the curve editor when a key has been moved.
	 */
	virtual void MovedKey();
	/**
	 *	DesireUndo
	 *	Called by the curve editor when an Undo is requested.
	 */
	virtual void DesireUndo();
	/**
	 *	DesireRedo
	 *	Called by the curve editor when an Redo is requested.
	 */
	virtual void DesireRedo();

	// Menu callbacks
	void OnSize( wxSizeEvent& In );

	void OnRenameEmitter(wxCommandEvent& In);

	void OnNewEmitter( wxCommandEvent& In );
	void OnSelectParticleSystem( wxCommandEvent& In );
	void OnNewEmitterBefore( wxCommandEvent& In );
	void OnNewEmitterAfter( wxCommandEvent& In );
	void OnNewModule( wxCommandEvent& In );

	void OnDuplicateEmitter(wxCommandEvent& In);
	void OnDeleteEmitter(wxCommandEvent& In);
	void OnExportEmitter(wxCommandEvent& In);
	void OnConvertRainEmitter(wxCommandEvent& In);

	void OnAddSelectedModule(wxCommandEvent& In);
	void OnCopyModule(wxCommandEvent& In);
	void OnPasteModule(wxCommandEvent& In);
	void OnDeleteModule( wxCommandEvent& In );
	void OnEnableModule( wxCommandEvent& In );
	void OnResetModule( wxCommandEvent& In );
	void OnRefreshModule(wxCommandEvent& In);
	/** Sync the sprite material in the generic browser */
	void OnModuleSyncMaterial( wxCommandEvent& In );
	/** Assign the selected material to the sprite material */
	void OnModuleUseMaterial( wxCommandEvent& In );
	/**
	 *	Set the module to an exact duplicate (copy) of the same module in the highest LOD level.
	 */
	void OnModuleDupHighest( wxCommandEvent& In );
	/**
	 *	Set the module to an exact duplicate (copy) of the same module in the next higher LOD level.
	 */
	void OnModuleDupHigher( wxCommandEvent& In );
	/**
	 *	Set the module to the SAME module in the next higher LOD level.
	 */
	void OnModuleShareHigher( wxCommandEvent& In );

	void OnMenuSimSpeed( wxCommandEvent& In );
	void OnSaveCam( wxCommandEvent& In );
	void OnResetSystem( wxCommandEvent& In );
	void OnResetInLevel( wxCommandEvent& In );
	void OnResetPeakCounts(wxCommandEvent& In);
	void OnUberConvert(wxCommandEvent& In);
	void OnRegenerateLowestLOD(wxCommandEvent& In);
	void OnRegenerateLowestLODDuplicateHighest(wxCommandEvent& In);
	void OnSavePackage(wxCommandEvent& In);
	void OnOrbitMode(wxCommandEvent& In);
	void OnWireframe(wxCommandEvent& In);
	void OnBounds(wxCommandEvent& In);
	void OnBoundsRightClick(wxCommandEvent& In);
	void OnPostProcess(wxCommandEvent& In);
	void OnToggleGrid(wxCommandEvent& In);
	void OnViewAxes( wxCommandEvent& In );
	void OnViewCounts(wxCommandEvent& In);
	void OnViewEvents(wxCommandEvent& In);
	void OnViewTimes(wxCommandEvent& In);
	void OnViewDistance(wxCommandEvent& In);
	void OnViewGeometry(wxCommandEvent& In);
	void OnViewGeometryProperties(wxCommandEvent& In);
	void OnLoopSimulation( wxCommandEvent& In );
	void OnShowPPBloom( wxCommandEvent& In );
	void OnShowPPDOF( wxCommandEvent& In );
	void OnShowPPMotionBlur( wxCommandEvent& In );
	void OnShowPPVolumeMaterial( wxCommandEvent& In );

#if defined(_CASCADE_ENABLE_MODULE_DUMP_)
	void OnViewModuleDump(wxCommandEvent& In);
#endif	//#if defined(_CASCADE_ENABLE_MODULE_DUMP_)

	void OnPlay(wxCommandEvent& In);
	void OnPause(wxCommandEvent& In);
	void OnSpeed(wxCommandEvent& In);
	void OnLoopSystem(wxCommandEvent& In);
	void OnRealtime(wxCommandEvent& In);

	void OnBackgroundColor(wxCommandEvent& In);
	void OnToggleWireSphere(wxCommandEvent& In);

	void OnUndo(wxCommandEvent& In);
	void OnRedo(wxCommandEvent& In);

	void OnPerformanceCheck(wxCommandEvent& In);

	void OnLODLow(wxCommandEvent& In);
	void OnLODLower(wxCommandEvent& In);
	void OnLODAddBefore(wxCommandEvent& In);
	void OnLODAddAfter(wxCommandEvent& In);
	void OnLODHigher(wxCommandEvent& In);
	void OnLODHigh(wxCommandEvent& In);
	void OnLODDelete(wxCommandEvent& In);

	// Utils
	void CreateNewModule(INT ModClassIndex);
	void PasteCurrentModule();
	void CopyModuleToEmitter(UParticleModule* pkSourceModule, UParticleEmitter* pkTargetEmitter, UParticleSystem* pkTargetSystem);

	void SetSelectedEmitter( UParticleEmitter* NewSelectedEmitter);
	void SetSelectedModule( UParticleEmitter* NewSelectedEmitter, UParticleModule* NewSelectedModule);

	void DeleteSelectedEmitter();
	void MoveSelectedEmitter(INT MoveAmount);
	void ExportSelectedEmitter();

	void DeleteSelectedModule(UBOOL bConfirm = TRUE);
	void EnableSelectedModule();
	void ResetSelectedModule();
	void RefreshSelectedModule();
	void RefreshAllModules();
	void DupHighestSelectedModule();
	void DupHigherSelectedModule();
	void ShareHigherSelectedModule();
	UBOOL InsertModule(UParticleModule* Module, UParticleEmitter* TargetEmitter, INT TargetIndex, UBOOL bSetSelected = true);

	UBOOL ModuleIsShared(UParticleModule* Module);

	void AddSelectedToGraph();
	void SetSelectedInCurveEditor();

	void SetCopyEmitter(UParticleEmitter* NewEmitter);
	void SetCopyModule(UParticleEmitter* NewEmitter, UParticleModule* NewModule);

	void RemoveModuleFromDump(UParticleModule* Module);

	// FNotify interface
	void NotifyDestroy( void* Src );
	void NotifyPreChange( void* Src, UProperty* PropertyAboutToChange );
	void NotifyPostChange( void* Src, UProperty* PropertyThatChanged );
	void NotifyPreChange( void* Src, FEditPropertyChain* PropertyChain );
	void NotifyPostChange( void* Src, FEditPropertyChain* PropertyChain );
	void NotifyExec( void* Src, const TCHAR* Cmd );

	static void InitParticleModuleClasses();

	// Duplicate emitter
	bool DuplicateEmitter(UParticleEmitter* SourceEmitter, UParticleSystem* DestSystem, UBOOL bShare = FALSE);

	// Undo/Redo support
	bool BeginTransaction(const TCHAR* pcTransaction);
	bool EndTransaction(const TCHAR* pcTransaction);
	bool TransactionInProgress();

	void ModifySelectedObjects();
	void ModifyParticleSystem();
	void ModifyEmitter(UParticleEmitter* Emitter);

	void CascadeUndo();
	void CascadeRedo();
	void CascadeTouch();

	// LOD settings
	void UpdateLODLevelControls();

	// PostProces
	/**
	 *	Update the post process chain according to the show options
	 */
	void UpdatePostProcessChain();

	/**
	 *	Return the index of the currently selected LOD level
	 *
	 *	@return	INT		The currently selected LOD level...
	 */
	INT GetCurrentlySelectedLODLevelIndex();

	/**
	 *	Return the currently selected LOD level
	 *
	 *	@return	UParticleLODLevel*	The currently selected LOD level...
	 */
	UParticleLODLevel* GetCurrentlySelectedLODLevel();

	/**
	 *	Return the currently selected LOD level
	 *
	 *	@param	InEmitter			The emitter to retrieve it from.
	 *	@return	UParticleLODLevel*	The currently selected LOD level.
	 */
	UParticleLODLevel* GetCurrentlySelectedLODLevel(UParticleEmitter* InEmitter);

	/**
	 *	Is the module of the given name suitable for the right-click module menu?
	 */
	UBOOL IsModuleSuitableForModuleMenu(FString& InModuleName);

	/**
	 *	Is the base module of the given name suitable for the right-click module menu
	 *	given the currently selected emitter TypeData?
	 */
	UBOOL IsBaseModuleTypeDataPairSuitableForModuleMenu(FString& InModuleName);

	/**
	 *	Is the base module of the given name suitable for the right-click module menu
	 *	given the currently selected emitter TypeData?
	 */
	UBOOL IsModuleTypeDataPairSuitableForModuleMenu(FString& InModuleName);

protected:
	/**
	 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
	 *  @return A string representing a name to use for this docking parent.
	 */
	virtual const TCHAR* GetDockingParentName() const;

	/**
	 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
	 */
	virtual const INT GetDockingParentVersion() const;

	void SetLODValue(INT LODSetting);
	UBOOL GenerateLODModuleValues(UParticleModule* TargetModule, 
		UParticleModule* HighModule, UParticleModule* LowModule, 
		FLOAT Percentage);

	bool	bTransactionInProgress;
	FString	kTransactionName;

public:
	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxMBCascadeModule
-----------------------------------------------------------------------------*/

class WxMBCascadeModule : public wxMenu
{
public:
	WxMBCascadeModule(WxCascade* Cascade);
	~WxMBCascadeModule();
};

/*-----------------------------------------------------------------------------
	WxMBCascadeEmitterBkg
-----------------------------------------------------------------------------*/

class WxMBCascadeEmitterBkg : public wxMenu
{
public:
	enum Mode
	{
		EMITTER_ONLY		= 0x0001,
		SELECTEDMODULE_ONLY	= 0x0002,
		TYPEDATAS_ONLY		= 0x0004,
		SPAWNS_ONLY			= 0x0008,
		UPDATES_ONLY		= 0x0010,
		PSYS_ONLY			= 0x0020,
		NON_TYPEDATAS		= SPAWNS_ONLY | UPDATES_ONLY,
		EVERYTHING			= EMITTER_ONLY | SELECTEDMODULE_ONLY | TYPEDATAS_ONLY | SPAWNS_ONLY | UPDATES_ONLY | PSYS_ONLY
	};

	WxMBCascadeEmitterBkg(WxCascade* Cascade, Mode eMode);
	~WxMBCascadeEmitterBkg();

protected:
	void InitializeModuleEntries(WxCascade* Cascade);

	void AddEmitterEntries(WxCascade* Cascade, Mode eMode);
	void AddPSysEntries(WxCascade* Cascade, Mode eMode);
	void AddSelectedModuleEntries(WxCascade* Cascade, Mode eMode);
	void AddTypeDataEntries(WxCascade* Cascade, Mode eMode);
	void AddNonTypeDataEntries(WxCascade* Cascade, Mode eMode);

	static UBOOL			InitializedModuleEntries;
	static TArray<FString>	TypeDataModuleEntries;
	static TArray<INT>		TypeDataModuleIndices;
	static TArray<FString>	ModuleEntries;
	static TArray<INT>		ModuleIndices;

	wxMenu* EmitterMenu;
	wxMenu* PSysMenu;
	wxMenu* SelectedModuleMenu;
	wxMenu* TypeDataMenu;
	TArray<wxMenu*> NonTypeDataMenus;
};

/*-----------------------------------------------------------------------------
	WxMBCascadeBkg
-----------------------------------------------------------------------------*/

class WxMBCascadeBkg : public wxMenu
{
public:
	WxMBCascadeBkg(WxCascade* Cascade);
	~WxMBCascadeBkg();
};

/*-----------------------------------------------------------------------------
	WxCascadePostProcessMenu
-----------------------------------------------------------------------------*/
enum ECascShowPPFlags
{
	CASC_SHOW_BLOOM			= 0x01,
	CASC_SHOW_DOF			= 0x02,
	CASC_SHOW_MOTIONBLUR	= 0x04,
	CASC_SHOW_PPVOLUME		= 0x08
};

class FCascShowPPFlagData
{
public:
	INT					ID;
	FString				Name;
	ECascShowPPFlags	Mask;

	FCascShowPPFlagData(INT InID, const FString& InName, const ECascShowPPFlags& InMask) :
		  ID(InID)
		, Name(InName)
		, Mask(InMask)
	{}
};

class WxCascadePostProcessMenu : public wxMenu
{
public:
	WxCascadePostProcessMenu(WxCascade* Cascade);
	~WxCascadePostProcessMenu();

	TArray<FCascShowPPFlagData> ShowPPFlagData;
};

#endif // __CASCADE_H__
