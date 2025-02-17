/*============================================================================
	AnimSetViewer.h: AnimSet viewer
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __ANIMSETVIEWER_H__
#define __ANIMSETVIEWER_H__

#include "SocketManager.h"
#include "TrackableWindow.h"

// Forward declarations.
class UAnimNodeMirror;
class UAnimNodeSequence;
class UAnimSequence;
class UAnimSet;
class UAnimTree;
class UMorphNodePose;
class UMorphTarget;
class UMorphTargetSet;
class WxAnimSetViewer;
class WxASVPreview;
class WxDlgAnimationCompression;
class WxPropertyWindow;
class WxSocketManager;

/**
 * Viewport client for the AnimSet viewer.
 */
struct FASVViewportClient: public FEditorLevelViewportClient
{
	WxAnimSetViewer*			AnimSetViewer;

	FPreviewScene				PreviewScene;
	FEditorCommonDrawHelper		DrawHelper;

	UBOOL						bShowBoneNames;
	UBOOL						bShowFloor;
	UBOOL						bShowSockets;
	UBOOL						bShowWindDir;

	UBOOL						bManipulating;
	EAxis						ManipulateAxis;
	FLOAT						DragDirX;
	FLOAT						DragDirY;
	FVector						WorldManDir;
	FVector						LocalManDir;

	FASVViewportClient(WxAnimSetViewer* InASV);
	virtual ~FASVViewportClient();

	// FEditorLevelViewportClient interface

	virtual FSceneInterface* GetScene() { return PreviewScene.GetScene(); }
	virtual FLinearColor GetBackgroundColor();
	virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI);
	virtual void Draw(FViewport* Viewport, FCanvas* Canvas);
	virtual void Tick(FLOAT DeltaSeconds);

	virtual UBOOL InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);
	virtual UBOOL InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad=FALSE);
	virtual void MouseMove(FViewport* Viewport,INT x, INT y);

	virtual void Serialize(FArchive& Ar);

	// FASVViewportClient interface
	void DrawWindDir(FViewport* Viewport, FCanvas* Canvas);
};

/*-----------------------------------------------------------------------------
	WxASVMenuBar
-----------------------------------------------------------------------------*/

class WxASVMenuBar : public wxMenuBar
{
public:
	WxASVMenuBar();

	wxMenu*	FileMenu;
	wxMenu*	ViewMenu;
	wxMenu* MeshMenu;
	wxMenu* AnimSetMenu;
	wxMenu* AnimSeqMenu;
	wxMenu* NotifyMenu;
	wxMenu* MorphSetMenu;
	wxMenu* MorphTargetMenu;
	wxMenu* AnimationCompressionMenu;
};

/*-----------------------------------------------------------------------------
	WxASVToolBar
-----------------------------------------------------------------------------*/

class WxASVToolBar : public wxToolBar
{
public:
	WxASVToolBar( wxWindow* InParent, wxWindowID InID );

private:
	WxMaskedBitmap SocketMgrB;
	WxMaskedBitmap ShowBonesB;
	WxMaskedBitmap ShowBoneNamesB;
	WxMaskedBitmap ShowWireframeB;
	WxMaskedBitmap ShowRefPoseB;
	WxMaskedBitmap ShowMirrorB;
	WxMaskedBitmap NewNotifyB;
	WxMaskedBitmap ClothB;
	WxMaskedBitmap SoftBodyGenerateB;
	WxMaskedBitmap SoftBodyToggleSimB;

	WxMaskedBitmap ShowRawAnimationB;

	WxMaskedBitmap LODAutoB;
	WxMaskedBitmap LODBaseB;
	WxMaskedBitmap LOD1B;
	WxMaskedBitmap LOD2B;
	WxMaskedBitmap LOD3B;

	WxMaskedBitmap Speed1B;
	WxMaskedBitmap Speed10B;
	WxMaskedBitmap Speed25B;
	WxMaskedBitmap Speed50B;
	WxMaskedBitmap Speed100B;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxASVStatusBar
-----------------------------------------------------------------------------*/

class WxASVStatusBar : public wxStatusBar
{
public:
	WxASVStatusBar( wxWindow* InParent, wxWindowID InID);

	void UpdateStatusBar( WxAnimSetViewer* AnimSetViewer );
};

/*-----------------------------------------------------------------------------
	WxSkeletonTreePopUpMenu
-----------------------------------------------------------------------------*/
class WxSkeletonTreePopUpMenu : public wxMenu
{
public:
	WxSkeletonTreePopUpMenu(WxAnimSetViewer* AnimSetViewer);
	~WxSkeletonTreePopUpMenu();
};

/*-----------------------------------------------------------------------------
	WxAnimSetViewer
-----------------------------------------------------------------------------*/

#define ASV_SCRUBRANGE (10000)

struct FASVSocketPreview
{
	USkeletalMeshSocket*	Socket;
	UPrimitiveComponent*	PreviewComp;

	FASVSocketPreview( USkeletalMeshSocket* InSocket, UPrimitiveComponent* InPreviewComp ) :
		Socket(InSocket),
		PreviewComp(InPreviewComp)
	{}

	FASVSocketPreview() {}

	friend FArchive& operator<<( FArchive& Ar, FASVSocketPreview& S )
	{
		return Ar << S.Socket << S.PreviewComp;
	}
};

template <> struct TIsPODType<FASVSocketPreview> { enum { Value = true }; };

class WxAnimSetViewer : public WxTrackableFrame, public FNotifyHook, public FSerializableObject, public FDockingParent
{
public:
	WxASVMenuBar*			MenuBar;
	WxASVToolBar*			ToolBar;
	WxASVStatusBar*			StatusBar;
	WxASVPreview*			PreviewWindow;
	WxTreeCtrl*				SkeletonTreeCtrl;
	TMap<wxTreeItemId,INT>	SkeletonTreeItemBoneIndexMap;

	USkeletalMesh*			SelectedSkelMesh;
	UAnimSet*				SelectedAnimSet;
	UAnimSequence*			SelectedAnimSeq;
	UMorphTargetSet*		SelectedMorphSet;
	UMorphTarget*			SelectedMorphTarget;
	USkeletalMeshSocket*	SelectedSocket;

	/** The main preview SkeletalMeshComponent.  Always plays compressed animation, if present. */
	UASVSkelComponent*		PreviewSkelComp;
	/** The main preview SkeletalMeshComponent.  Always plays uncompressed animation, if present. Only for drawing uncompressed bones. */
	UASVSkelComponent*		PreviewSkelCompRaw;
	USkeletalMeshComponent*	PreviewSkelCompAux1;
	USkeletalMeshComponent*	PreviewSkelCompAux2;
	USkeletalMeshComponent*	PreviewSkelCompAux3;

	UAnimTree*				PreviewAnimTree;
	/** Uncompressed animation tree. */
	UAnimTree*				PreviewAnimTreeRaw;
	UAnimNodeSequence*		PreviewAnimNode;
	/** Uncompressed animation sequence. */
	UAnimNodeSequence*		PreviewAnimNodeRaw;
	UAnimNodeMirror*		PreviewAnimMirror;
	/** Uncompressed animation mirroring. */
	UAnimNodeMirror*		PreviewAnimMirrorRaw;
	UMorphNodePose*			PreviewMorphPose;

	FRBPhysScene*			RBPhysScene;
	FRotator				WindRot;
	FLOAT					WindStrength;

	/** Animation playback speed, in [0.01f,1.0f]. */
	FLOAT					PlaybackSpeed;

	// Various wxWindows controls
	WxBitmap				StopB, PlayB, LoopB, NoLoopB, UseButtonB;

	wxComboBox*				SkelMeshCombo;
	wxComboBox*				SkelMeshAux1Combo;
	wxComboBox*				SkelMeshAux2Combo;
	wxComboBox*				SkelMeshAux3Combo;

	wxComboBox*				AnimSetCombo;
	wxListBox*				AnimSeqList;
	wxComboBox*				MorphSetCombo;
	wxListBox*				MorphTargetList;
	wxNotebook*				PropNotebook;
	wxNotebook*				ResourceNotebook;
	wxSlider*				TimeSlider;
	wxBitmapButton*			PlayButton;
	wxBitmapButton*			LoopButton;

	WxPropertyWindow*		MeshProps;
	WxPropertyWindow*		AnimSetProps;
	WxPropertyWindow*		AnimSeqProps;

	WxSocketManager*				SocketMgr;
	TArray<FASVSocketPreview>		SocketPreviews;

	WxDlgAnimationCompression*		DlgAnimationCompression;

	/** Toggle usage of instance weights on the preview mesh */
	UBOOL					bPreviewInstanceWeights;

	WxAnimSetViewer(wxWindow* InParent, wxWindowID InID, USkeletalMesh* InSkelMesh, UAnimSet* InSelectAnimSet, UMorphTargetSet* InMorphSet);
	virtual ~WxAnimSetViewer();

	/**
	 * This function is called when the window has been selected from within the ctrl + tab dialog.
	 */
	virtual void OnSelected();

	/**
	* Called once to load AnimSetViewer settings, including window position.
	*/
	void LoadSettings();

	/**
	* Writes out values we want to save to the INI file.
	*/
	void SaveSettings();

	/**
	 * Returns a handle to the anim set viewer's preview viewport client.
	 */
	FASVViewportClient* GetPreviewVC();

	// FSerializableObject interface
	void Serialize(FArchive& Ar);

	// Menu handlers
	/**
	 *	Called when a SIZE event occurs on the window
	 *
	 *	@param	In		The size event information
	 */
	void OnSize( wxSizeEvent& In );
	void OnSkelMeshComboChanged( wxCommandEvent& In );
	void OnAuxSkelMeshComboChanged( wxCommandEvent& In );
	void OnAnimSetComboChanged( wxCommandEvent& In );
	void OnAnimSeqListChanged( wxCommandEvent& In );
	void OnSkelMeshUse( wxCommandEvent& In );
	void OnAnimSetUse( wxCommandEvent& In );
	void OnAuxSkelMeshUse( wxCommandEvent& In );
	void OnImportPSA( wxCommandEvent& In );
	void OnImportCOLLADAAnim( wxCommandEvent& In );
	void OnImportMeshLOD( wxCommandEvent& In );
	void OnImportMeshWeights( wxCommandEvent& In );
	void OnToggleMeshWeights( wxCommandEvent& In );
	void OnNewAnimSet( wxCommandEvent& In );
	void OnTimeScrub( wxScrollEvent& In );
	void OnViewBones( wxCommandEvent& In );
	void OnShowRawAnimation( wxCommandEvent& In );
	void OnViewBoneNames( wxCommandEvent& In );
	void OnViewFloor( wxCommandEvent& In );
	void OnViewWireframe( wxCommandEvent& In );
	void OnViewGrid( wxCommandEvent& In );
	void OnViewSockets( wxCommandEvent& In );
	void OnViewRefPose( wxCommandEvent& In );
	void OnViewMirror( wxCommandEvent& In );
	void OnViewBounds( wxCommandEvent& In );
	void OnViewCollision( wxCommandEvent& In );
	void OnViewSoftBodyTetra(wxCommandEvent& In);
	void OnForceLODLevel( wxCommandEvent& In );
	void OnRemoveLOD( wxCommandEvent& In );
	void OnLoopAnim( wxCommandEvent& In );
	void OnPlayAnim( wxCommandEvent& In );
	void OnEmptySet( wxCommandEvent& In );
	/** Copies anim notifies from the selected sequence to a user-named destination sequence. */
	void OnNotifyCopy( wxCommandEvent& In );

	void OnSpeed(wxCommandEvent& In);

	void OnDeleteTrack( wxCommandEvent& In );
	void OnCopyTranslationBoneNames(wxCommandEvent& In);
	void OnRenameSequence( wxCommandEvent& In );
	void OnDeleteSequence( wxCommandEvent& In );
	void OnCopySequence( wxCommandEvent& In );
	void OnMoveSequence( wxCommandEvent& In );
	void OnMakeSequencesAdditive(wxCommandEvent& In);
	void OnSequenceApplyRotation( wxCommandEvent& In );
	void OnSequenceReZero( wxCommandEvent& In );
	void OnSequenceCrop( wxCommandEvent& In );
	void OnNotifySort( wxCommandEvent& In );
	void OnNewNotify( wxCommandEvent& In );
	void OnCopySequenceName( wxCommandEvent& In );
	void OnCopySequenceNameList( wxCommandEvent& In );
	void OnCopyMeshBoneNames( wxCommandEvent& In );
	void OnFixupMeshBoneNames( wxCommandEvent& In );
	void OnSwapLODSections( wxCommandEvent& In );
	void OnMergeMaterials( wxCommandEvent& In );
	void OnAutoMirrorTable( wxCommandEvent& In );
	void OnCheckMirrorTable( wxCommandEvent& In );
	void OnCopyMirrorTable( wxCommandEvent& In );
	void OnCopyMirroTableFromMesh( wxCommandEvent& In );
	void OnUpdateBounds( wxCommandEvent& In );
	void OnClearPreviewMeshes( wxCommandEvent& In );
	void OnCopySockets( wxCommandEvent& In );
	void OnPasteSockets( wxCommandEvent& In );

	void OnToggleClothSim( wxCommandEvent& In );
	void OnSoftBodyGenerate( wxCommandEvent& In );
	void OnSoftBodyToggleSim( wxCommandEvent& In );

	void OnNewMorphTargetSet( wxCommandEvent& In );
	void OnImportMorphTarget( wxCommandEvent& In );
	void OnImportMorphTargetLOD( wxCommandEvent& In );
	void OnMorphSetComboChanged( wxCommandEvent& In );
	void OnMorphSetUse( wxCommandEvent& In );
	void OnMorphTargetListChanged( wxCommandEvent& In );
	void OnRenameMorphTarget( wxCommandEvent& In );
	void OnDeleteMorphTarget( wxCommandEvent& In );

	// Socket manager window
	void OnOpenSocketMgr( wxCommandEvent& In );
	void OnNewSocket( wxCommandEvent& In );
	void OnDeleteSocket( wxCommandEvent& In );
	void OnClickSocket( wxCommandEvent& In );
	void OnSocketMoveMode( wxCommandEvent& In );

	void SetSelectedSocket(USkeletalMeshSocket* InSocket);
	void UpdateSocketList();
	void SetSocketMoveMode( EASVSocketMoveMode NewMode );

	void ClearSocketPreviews();
	void RecreateSocketPreviews();
	void UpdateSocketPreviews();

	// Skeleton Tree Manager
	void FillSkeletonTree();
	void OnFillSkeletonTree(wxCommandEvent &In);
	void OnSkeletonTreeItemRightClick(wxTreeEvent& In);
	void OnSkeletonTreeMenuHandleCommand(wxCommandEvent &In);

	/** Toggles the modal animation Animation compression dialog. */
	void OnOpenAnimationCompressionDlg(wxCommandEvent& In);

	/** Updates the contents of the status bar, based on e.g. the selected set/sequence. */
	void UpdateStatusBar();

	// Tools
	void SetSelectedSkelMesh(USkeletalMesh* InSkelMesh, UBOOL bClearMorphTarget);
	void SetSelectedAnimSet(UAnimSet* InAnimSet, UBOOL bAutoSelectMesh);
	void SetSelectedAnimSequence(UAnimSequence* InAnimSeq);
	void SetSelectedMorphSet(UMorphTargetSet* InMorphSet, UBOOL bAutoSelectMesh);
	void SetSelectedMorphTarget(UMorphTarget* InMorphTarget);

	void ImportPSA();
	void ImportCOLLADAAnim();
	void ImportMeshLOD();
	void ImportMeshLOD(USkeletalMesh* InSkeletalMesh,INT DesiredLOD);
	void ImportMeshWeights();
	
	/**
	* Displays and handles dialogs necessary for importing 
	* a new morph target mesh from file. The new morph
	* target is placed in the currently selected MorphTargetSet
	*
	* @param bImportToLOD - if TRUE then new files will be treated as morph target LODs 
	* instead of new morph target resources
	*/
	void ImportMorphTarget(UBOOL bImportToLOD);
	
	void CreateNewAnimSet();
	void UpdateAnimSeqList();
	void UpdateAnimSetCombo();
	void UpdateMorphTargetList();
	void UpdateMorphSetCombo();
	void RefreshPlaybackUI();
	FLOAT GetCurrentSequenceLength();
	void TickViewer(FLOAT DeltaSeconds);
	void EmptySelectedSet();
	void DeleteTrackFromSelectedSet();
	void CopyTranslationBoneNamesToAnimSet();
	void RenameSelectedSeq();
	void DeleteSelectedSequence();
	void CopySelectedSequence();
	void MoveSelectedSequence();
	void MakeSelectedSequencesAdditive();
	UBOOL ConvertAnimSeqToAdditive(UAnimSequence* SourceAnimSeq, UAnimSequence* DestAnimSeq, UAnimSequence* RefAnimSeq, USkeletalMesh* SkelMesh);
	UBOOL CopyAnimSequence(UAnimSequence* SourceAnimSeq, UAnimSequence *DestAnimSeq, UAnimSet* SourceAnimSet, UAnimSet* DestAnimSet, USkeletalMesh* FillInMesh);
	UBOOL CopyAnimSequenceProperties(UAnimSequence* SourceAnimSeq, UAnimSequence* DestAnimSeq);
	UBOOL CopyNotifies(UAnimSequence* SourceAnimSeq, UAnimSequence* DestAnimSeq);
	void SequenceApplyRotation();
	void SequenceReZeroToCurrent();
	void SequenceCrop(UBOOL bFromStart=true);
	void UpdateMeshBounds();
	void UpdateSkelComponents();
	void UpdateForceLODButtons();
	void RenameSelectedMorphTarget();
	void DeleteSelectedMorphTarget();
	void UpdateClothWind();

	/**
	 *	Update the preview window.
	 */
	void UpdatePreviewWindow();

	// Notification hook.
	virtual void NotifyDestroy( void* Src );
	virtual void NotifyPreChange( void* Src, UProperty* PropertyAboutToChange );
	virtual void NotifyPostChange( void* Src, UProperty* PropertyThatChanged );
	virtual void NotifyExec( void* Src, const TCHAR* Cmd );

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

private:
	DECLARE_EVENT_TABLE()
};

#endif // __ANIMSETVIEWER_H__
