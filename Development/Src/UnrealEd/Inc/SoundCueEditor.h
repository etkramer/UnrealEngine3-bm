/*=============================================================================
	SoundCueEditor.h: SoundCue editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SOUNDCUEEDITOR_H__
#define __SOUNDCUEEDITOR_H__

#include "UnrealEd.h"

/*-----------------------------------------------------------------------------
	WxSoundCueEdToolBar
-----------------------------------------------------------------------------*/

class WxSoundCueEdToolBar : public wxToolBar
{
public:
	WxSoundCueEdToolBar( wxWindow* InParent, wxWindowID InID );
	~WxSoundCueEdToolBar();

	WxMaskedBitmap PlayCueB, PlayNodeB, StopB;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxSoundCueEditor
-----------------------------------------------------------------------------*/

class WxSoundCueEditor : public WxLinkedObjEd
{
public:
	WxSoundCueEditor( wxWindow* InParent, wxWindowID InID, class USoundCue* InSoundCue );
	virtual ~WxSoundCueEditor();

	// LinkedObjEditor interface
	virtual void InitEditor();

	/**
	 * @return Returns the name of the inherited class, so we can generate .ini entries for all LinkedObjEd instances.
	 */
	virtual const TCHAR* GetConfigName() const
	{
		return TEXT( "SoundCueEditor" );
	}

	/**
     * Creates the controls for this window
     */
	virtual void CreateControls( UBOOL bTreeControl );

	virtual void OpenNewObjectMenu();
	virtual void OpenObjectOptionsMenu();
	virtual void OpenConnectorOptionsMenu();
	virtual void DrawObjects( FViewport* Viewport, FCanvas* Canvas );
	virtual void UpdatePropertyWindow();

	virtual void EmptySelection();
	virtual void AddToSelection( UObject* Obj );
	virtual UBOOL IsInSelection( UObject* Obj ) const;
	virtual INT GetNumSelected() const;

	virtual void SetSelectedConnector( FLinkedObjectConnector& Connector );
	virtual FIntPoint GetSelectedConnLocation( FCanvas* Canvas );

	// Make a connection between selected connector and an object or another connector.
	virtual void MakeConnectionToConnector( FLinkedObjectConnector& Connector );
	virtual void MakeConnectionToObject( UObject* EndObj );

	/**
	 * Called when the user releases the mouse over a link connector and is holding the ALT key.
	 * Commonly used as a shortcut to breaking connections.
	 *
	 * @param	Connector	The connector that was ALT+clicked upon.
	 */
	virtual void AltClickConnector( FLinkedObjectConnector& Connector );

	virtual void MoveSelectedObjects( INT DeltaX, INT DeltaY );
	virtual void EdHandleKeyInput( FViewport* Viewport, FName Key, EInputEvent Event );

	// Static, initialiasation stuff
	static void InitSoundNodeClasses();

	// Utils
	void ConnectNodes( USoundNode* ParentNode, INT ChildIndex, USoundNode* ChildNode );
	void DeleteSelectedNodes();

	// Context Menu handlers
	void OnContextNewSoundNode( wxCommandEvent& In );
	void OnContextNewWave( wxCommandEvent& In );
	void OnContextAddInput( wxCommandEvent& In );
	void OnContextDeleteInput( wxCommandEvent& In );
	void OnContextDeleteNode( wxCommandEvent& In );
	void OnContextPlaySoundNode( wxCommandEvent& In );
	void OnContextPlaySoundCue( wxCommandEvent& In );
	void OnContextStopPlaying( wxCommandEvent& In );
	void OnContextBreakLink( wxCommandEvent& In );
	void OnSize( wxSizeEvent& In );

	/**
	 * Used to serialize any UObjects contained that need to be to kept around.
	 *
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize( FArchive& Ar );

	WxSoundCueEdToolBar* ToolBar;

	// SoundCue currently being edited
	class USoundCue* SoundCue;

	// Currently selected USoundNodes
	TArray<class USoundNode*> SelectedNodes;

	// Selected Connector
	UObject* ConnObj; // This is usually a SoundNode, but might be the SoundCue to indicate the 'root' connector.
	INT ConnType;
	INT ConnIndex;

	// Static list of all SequenceObject classes.
	static TArray<UClass*>	SoundNodeClasses;
	static UBOOL			bSoundNodeClassesInitialized;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxMBSoundCueEdNewNode
-----------------------------------------------------------------------------*/

class WxMBSoundCueEdNewNode : public wxMenu
{
public:
	WxMBSoundCueEdNewNode( WxSoundCueEditor* CueEditor );
	~WxMBSoundCueEdNewNode();
};

/*-----------------------------------------------------------------------------
	WxMBSoundCueEdNodeOptions
-----------------------------------------------------------------------------*/

class WxMBSoundCueEdNodeOptions : public wxMenu
{
public:
	WxMBSoundCueEdNodeOptions( WxSoundCueEditor* CueEditor );
	~WxMBSoundCueEdNodeOptions();
};

/*-----------------------------------------------------------------------------
	WxMBSoundCueEdConnectorOptions
-----------------------------------------------------------------------------*/

class WxMBSoundCueEdConnectorOptions : public wxMenu
{
public:
	WxMBSoundCueEdConnectorOptions( WxSoundCueEditor* CueEditor );
	~WxMBSoundCueEdConnectorOptions();
};

#endif // __SOUNDCUEEDITOR_H__
