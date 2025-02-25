/*=============================================================================
	PostProcessEditor.h: PostProcess editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __POSTPROCESSEDITOR_H__
#define __POSTPROCESSEDITOR_H__

class UPostProcessChain;
class UPostProcessEffect;
class WxPostProcessEditorToolBar;
class WxPropertyWindow;
class USceneRenderTarget;

class WxPostProcessEditor : public wxFrame, public FNotifyHook, public FLinkedObjEdNotifyInterface, public FSerializableObject, public FDockingParent
{
public:
	/** PostProcessChain currently being edited. */
	UPostProcessChain* PostProcess;

	/** All nodes in current tree (including the PostProcess itself). */
	TArray<UPostProcessEffect*> TreeNodes;

	/** Currently selected UPostProcessEffects. */
	TArray<UPostProcessEffect*> SelectedNodes;

	/** One and only scene render target node represents start of chain */
	USceneRenderTarget* SceneRenderTarget;

	/** Object containing selected connector. */
	UObject* ConnObj;

	/** Type of selected connector. */
	INT ConnType;

	/** Index of selected connector. */
	INT ConnIndex;

	/** 
	*	If we are shutting down (will be true after OnClose is called). 
	*	Needed because wxWindows does not call destructor right away, so things can get ticked after OnClose.
	*/
	UBOOL	bEditorClosing;

	WxPropertyWindow*			PropertyWindow;
	FLinkedObjViewportClient*	LinkedObjVC;
	UTexture2D*					BackgroundTexture;

	WxPostProcessEditor( wxWindow* InParent, wxWindowID InID, class UPostProcessChain* InPostProcess );
	~WxPostProcessEditor();

	void OnClose( wxCloseEvent& In );
	void RefreshViewport();

	// FLinkedObjViewportClient interface

	virtual void DrawObjects(FViewport* Viewport, FCanvas* Canvas);
	virtual void OpenNewObjectMenu();
	virtual void OpenObjectOptionsMenu();
	virtual void UpdatePropertyWindow();
	virtual UBOOL DrawCurves();

	virtual void EmptySelection();
	virtual void AddToSelection( UObject* Obj );
	virtual void RemoveFromSelection( UObject* Obj );
	virtual UBOOL IsInSelection( UObject* Obj ) const;
	virtual INT GetNumSelected() const;

	virtual FIntPoint GetConnectionLocation(UObject* ConnObj, INT ConnType, INT ConnIndex);
	virtual void SetSelectedConnector( FLinkedObjectConnector& Connector );
	virtual FIntPoint GetSelectedConnLocation(FCanvas* Canvas);
	virtual INT GetSelectedConnectorType();
	virtual FColor GetMakingLinkColor();

	// Make a connection between selected connector and an object or another connector.
	virtual void MakeConnectionToConnector( FLinkedObjectConnector& Connector );
	virtual void MakeConnectionToObject( UObject* EndObj );

	virtual void MoveSelectedObjects( INT DeltaX, INT DeltaY );
	virtual void EdHandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event);
	virtual void SpecialClick( INT NewX, INT NewY, INT SpecialIndex );

	// FSerializableObject interface
	void Serialize(FArchive& Ar);

	// FNotifyHook interface
	virtual void NotifyPreChange( void* Src, UProperty* PropertyAboutToChange );
	virtual void NotifyPostChange( void* Src, UProperty* PropertyThatChanged );
	virtual void NotifyExec( void* Src, const TCHAR* Cmd );

	// Menu handlers
	void OnNewPostProcessNode( wxCommandEvent& In );
	void OnBreakAllLinks( wxCommandEvent& In );
	void OnDeleteObjects( wxCommandEvent& In );
	void OnSize(wxSizeEvent& In);

	// Utils
	void DeleteSelectedObjects();
	void DuplicateSelectedObjects();

	// Drawing
	void DrawNode(UPostProcessEffect* Node, FCanvas* Canvas, UBOOL bSelected);

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

	DECLARE_EVENT_TABLE()
};

#endif // __POSTPROCESSEDITOR_H__
