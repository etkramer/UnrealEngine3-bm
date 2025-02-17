/*=============================================================================
	StaticMeshEditor.h: StaticMesh editor definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __STATICMESHEDITOR_H__
#define __STATICMESHEDITOR_H__

#include "ConvexDecompTool.h"
#include "TrackableWindow.h"

struct FStaticMeshEditorViewportClient;
struct FVoronoiRegion;

class WxViewportHolder;
class WxPropertyWindow;
class WxStaticMeshEditorBar;
class WxStaticMeshEditMenu;
class WxMeshSimplificationWindow;

enum EChunkViewMode
{
	ECVM_ViewOnly,
	ECVM_ViewAllBut,
	ECVM_ViewUpTo
};

enum ESliceColorationMode
{
	ESCM_Random,
	ESCM_FixedChunks,
	ESCM_DestroyableChunks,
	ESCM_NoPhysChunks
};

struct FFractureEdInfo
{
	UBOOL bCanBeDestroyed;
	UBOOL bRootFragment;
	UBOOL bNeverSpawnPhysics;
};

class WxStaticMeshEditor : public WxTrackableFrame, public FSerializableObject, public FDockingParent, public FConvexDecompOptionHook
{
	/**The highest LOD level allowed (by design)*/
	static const INT MaxAllowedLOD = 3;

public:
	DECLARE_DYNAMIC_CLASS(WxStaticMeshEditor)

	WxStaticMeshEditor();
	WxStaticMeshEditor(wxWindow* parent, wxWindowID id, UStaticMesh* InStaticMesh, UBOOL bInSimplifyMode);
	virtual ~WxStaticMeshEditor();

	/**
	 * This function is called when the window has been selected from within the ctrl + tab dialog.
	 */
	virtual void OnSelected();

	UStaticMesh* StaticMesh;
	UBOOL DrawOpenEdges;
	UBOOL DrawUVOverlay;
	UINT NumTriangles, NumVertices, NumUVChannels;
	WxViewportHolder* ViewportHolder;
	WxPropertyWindow* PropertyWindow;
	FStaticMeshEditorViewportClient* ViewportClient;
	WxStaticMeshEditorBar* ToolBar;
	WxStaticMeshEditMenu* MenuBar;
	WxConvexDecompOptions* DecompOptions;
	class WxFractureToolOptions* FractureOptions;
	WxMeshSimplificationWindow* MeshSimplificationWindow;

	TArray<FVoronoiRegion> FractureChunks;
	TArray<FVector> FractureCenters;
	TArray<FFractureEdInfo> FractureInfos;

	INT CurrentViewChunk;
	EChunkViewMode CurrentChunkViewMode;
	UBOOL bViewChunkChanged;

	TArray<INT>	SelectedChunks;

	UBOOL bAdjustingCore;
	EWidgetMovementMode CoreEditMode;

	FVector CurrentCoreOffset;
	FRotator CurrentCoreRotation;
	FLOAT CurrentCoreScale;
	FVector CurrentCoreScale3D;
	UStaticMesh* PendingCoreMesh;

	// Map from the final fragment index to the chunk in the tool it came from.
	TArray<INT>	FinalToToolMap;

	/** True if the editor layout is in 'Simplify Mode' */
	UBOOL bSimplifyMode;


	/** Changes the StaticMeshEditor to look at this new mesh instead. */
	void SetEditorMesh(UStaticMesh* NewMesh);

	void LockCamera(UBOOL bInLock);

	/** Invalidates the viewport */
	void InvalidateViewport();

	/** Updates the toolbars*/
	void UpdateToolbars();
	void GenerateKDop(const FVector* Directions, UINT NumDirections);
	void GenerateSphere();

	/** Converts the collision data for the static mesh */
	void ConvertBoxToConvexCollision(void);

	/** Clears the collision data for the static mesh */
	void RemoveCollision(void);

	/** Handler for the FConvexDecompOptionHook hook */
	virtual void DoDecomp(INT Depth, INT MaxVerts, FLOAT CollapseThresh);
	virtual void DecompOptionsClosed();

	// Called by WxFractureToolOptions

	/** Generate points randomly for regions. */
	void GenerateRandomPoints(INT NumChunks);
	/** Cluster existing points based on the vertices of the graphics mesh. */
	void ClusterPoints();
	/** Generate regions that will be used to slice mesh. */
	void ComputeRegions(const FVector& PlaneBias, UBOOL bResetInfos);
	/** Add some random noise to cluster centers */
	void RandomizePoints(const FVector& RandScale);
	/** Move points towards nearest bounds face */
	void MovePointsTowardsNearestFace(const FVector& MoveAmount);
	/** Apply regions to cut actual mesh up */
	void SliceMesh();
	/** Called when options dialog is closed. */
	virtual void FractureOptionsClosed();
	void ChangeChunkView( INT ChunkIndex, EChunkViewMode ViewMode );

	void RemoveCore();
	void BeginAddCore(UBOOL bUseExistingCoreTransform, UStaticMesh* CoreMesh);
	void BeginAdjustCore();
	void AcceptAddCore();
	void CancelAddCore();
	void DoMergeStaticMesh(UStaticMesh* OtherMesh, const FVector& InOffset, const FRotator& InRotation, FLOAT InScale, const FVector& InScale3D);

	/** Change the selected chunk. */
	void AddChunkToSelection(INT NewChunkIndex);
	void RemoveChunkFromSelection(INT NewChunkIndex);
	void ClearChunkSelection();
	void GrowChunkSelection();
	void ShrinkChunkSelection();
	/** Select all chunks at the top of the mesh. */
	void SelectTopChunks();
	/** Select all chunks at the bottom of the mesh. */
	void SelectBottomChunks();
	/** Invert current selection set */
	void InvertSelection();

	/** Util for finding color of a slice chunks, based on color mode and selection state. */
	FColor GetChunkColor(INT ChunkIndex);

	virtual void Serialize(FArchive& Ar);

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

	/**
	 * Updates the property view.
	 */
	void UpdatePropertyView();

private:
	/**
	 * Fill the array with strings representing the available options
	 * when altering LODs for a static mesh.
	 *
	 * Note: We should not allow the user to skip an LOD (e.g. add LOD2 before LOD1 exists).
	 *
	 * @param	Strings	The TArray of strings to be populated; this array will be emptied.
	 */
	void FillOutLODAlterationStrings( TArray<FString>& Strings ) const;

	void UpdateChunkUI();

	void OnSize( wxSizeEvent& In );
	void OnPaint( wxPaintEvent& In );
	void UI_ShowEdges( wxUpdateUIEvent& In );
	void UI_ShowUVOverlay( wxUpdateUIEvent& In );
	void UI_ShowWireframe( wxUpdateUIEvent& In );
	void UI_ShowBounds( wxUpdateUIEvent& In );
	void UI_ShowCollision( wxUpdateUIEvent& In );
	void UI_LockCamera( wxUpdateUIEvent& In );
	void OnShowEdges( wxCommandEvent& In );
	void OnShowUVOverlay( wxCommandEvent& In );
	void OnShowWireframe( wxCommandEvent& In );
	void OnShowBounds( wxCommandEvent& In );
	void OnShowCollision( wxCommandEvent& In );
	void OnLockCamera( wxCommandEvent& In );
	void OnSaveThumbnailAngle( wxCommandEvent& In );
	void OnCollision6DOP( wxCommandEvent& In );
	void OnCollision10DOPX( wxCommandEvent& In );
	void OnCollision10DOPY( wxCommandEvent& In );
	void OnCollision10DOPZ( wxCommandEvent& In );
	void OnCollision18DOP( wxCommandEvent& In );
	void OnCollision26DOP( wxCommandEvent& In );
	void OnCollisionSphere( wxCommandEvent& In );

	/** Handles the remove collision menu option */
	void OnCollisionRemove( wxCommandEvent& In );

	/** Handles the convert collision menu option */
	void OnConvertBoxToConvexCollision( wxCommandEvent& In );

	/** When chosen from the menu, pop up the 'convex decomposition' dialog. */
	void OnCollisionConvexDecomp( wxCommandEvent& In );

	/** Prototype pre-fracturing tool. */
	void OnFractureTool( wxCommandEvent& In );


	/** Tool for merging static meshes. */
	void OnMergeStaticMesh( wxCommandEvent& In );

	/** Event for forcing an LOD */
	void OnForceLODLevel( wxCommandEvent& In );
	/** Event for removing an LOD */
	void OnRemoveLOD( wxCommandEvent& In );
	/** Event for generating an LOD */
	void OnGenerateLOD( wxCommandEvent& In );
	/** Event for generating UV's */
	void OnGenerateUVs( wxCommandEvent& In );
	/** Event for importing an LOD */
	void OnImportMeshLOD( wxCommandEvent& In );
	/** Updates NumTriangles, NumVertices, NumOpenEdges, NumDoubleSidedShadowTriangles and NumUVChannels for the given LOD */
	void UpdateLODStats(INT CurrentLOD);

	void OnChangeMesh( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};


//////////////////////////////////////////////////////////////////////////
// FRACTURE TOOL
//////////////////////////////////////////////////////////////////////////

class WxFractureToolOptions : public wxDialog
{
public:
	WxFractureToolOptions( WxStaticMeshEditor* InSME );

	/** If true, draw wireframe indication of cut regions. */
	UBOOL bShowCuts;
	/** If true, draw all cuts as sold. */
	UBOOL bShowCutsSolid;
	/** If true, only show cuts for chunks that are visible. */
	UBOOL bShowOnlyVisibleCuts;
	/** Whether to show the core or not. */
	UBOOL bShowCore;

	/** Use coloration to show fixed pieces. */
	ESliceColorationMode ColorMode;

	/** Util to change range of chunk view slider. */
	void UpdateViewChunkSlider(INT NumChunks);
	/** Recompute regions based on set of FracturePoints and PlaneBias entries. */
	void RegenRegions(UBOOL bResetInfos);
	/** Util for updating state of core control buttons. */
	void UpdateCoreButtonStates();
	/** Read info from Plane Bias dialog */
	FVector GetPlaneBiasFromDialog();

	wxCheckBox *ShowCutsBox, *DestroyableBox, *RootChunkBox, *NoPhysBox, *ShowCutsSolidBox, *ShowVisibleCutsBox, *ShowCoreBox;
private:
	wxButton *AddCoreButton, *RemoveCoreButton, *CancelCoreButton, *AcceptCoreButton;
	wxSlider *NumChunkSlider, *ViewChunkSlider;
	wxButton *ComputeButton, *ApplyButton, *CloseButton, *RandomizerButton, *MoveToFaceButton;
	wxButton *GrowSelButton, *ShrinkSelButton, *SelectTopButton, *SelectBottomButton, *InvertSelectionButton;
	wxComboBox *ChunkViewModeCombo, *ColorModeCombo;
	wxTextCtrl *XBiasEntry, *YBiasEntry, *ZBiasEntry;
	wxTextCtrl *XRandScaleEntry, *YRandScaleEntry, *ZRandScaleEntry;

	WxStaticMeshEditor* Editor;

	void OnPlaneBiasChangeX(wxCommandEvent& In);
	void OnPlaneBiasChangeY(wxCommandEvent& In);
	void OnPlaneBiasChangeZ(wxCommandEvent& In);
	void OnPlaneBiasLoseFocus(wxCommandEvent& In);
	void OnGeneratePoints( wxCommandEvent& In );
	void OnRandomize( wxCommandEvent& In );
	void OnMoveToFaces( wxCommandEvent& In );
	void OnCluster( wxCommandEvent& In );
	void OnSlice( wxCommandEvent& In );
	void OnShowCutsChange( wxCommandEvent& In );
	void OnShowCutsSolidChange( wxCommandEvent& In );
	void OnShowOnlyVisibleCuts( wxCommandEvent& In );
	void OnColorModeChange( wxCommandEvent& In );
	void OnShowCoreChange( wxCommandEvent& In );
	void OnPressClose( wxCommandEvent& In );
	void OnClose( wxCloseEvent& In );
	void OnViewChunkChange( wxScrollEvent& In );
	void OnViewChunkModeChange( wxCommandEvent& In );
	void OnChunkOptChange( wxCommandEvent& In );
	void OnGrowSelection( wxCommandEvent& In );
	void OnShrinkSelection( wxCommandEvent& In );
	void OnSelectTop( wxCommandEvent& In );
	void OnSelectBottom( wxCommandEvent& In );
	void OnInvertSelection( wxCommandEvent& In );
	void OnAddCore( wxCommandEvent& In );
	void OnRemoveCore( wxCommandEvent& In );
	void OnAcceptCore( wxCommandEvent& In );
	void OnCancelCore( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};




#endif // __STATICMESHEDITOR_H__
