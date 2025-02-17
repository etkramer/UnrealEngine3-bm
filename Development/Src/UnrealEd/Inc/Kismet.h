/*=============================================================================
	Kismet.h: Gameplay sequence editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __KISMET_H__
#define __KISMET_H__

#include "UnrealEd.h"
#include "UnLinkedObjEditor.h"

/*-----------------------------------------------------------------------------
	WxKismetToolBar
-----------------------------------------------------------------------------*/

class WxKismetToolBar : public wxToolBar
{
public:
	WxKismetToolBar( wxWindow* InParent, wxWindowID InID );
	~WxKismetToolBar();

	WxMaskedBitmap ParentSequenceB, RenameSequenceB, HideB, ShowB, CurvesB, SearchB, ZoomToFitB, UpdateB, OpenB;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxKismetStatusBar
-----------------------------------------------------------------------------*/

class WxKismetStatusBar : public wxStatusBar
{
public:
	WxKismetStatusBar( wxWindow* InParent, wxWindowID InID);
	~WxKismetStatusBar();

	void SetMouseOverObject( class USequenceObject* SeqObj );
};

/* ==========================================================================================================
	WxSequenceTreeCtrl
========================================================================================================== */
/**
 * This tree control displays the list of sequences opened for edit
 */ 
class WxSequenceTreeCtrl : public WxTreeCtrl, public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS(WxSequenceTreeCtrl)

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxSequenceTreeCtrl();
	virtual ~WxSequenceTreeCtrl();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InParent	the window that opened this dialog
	 * @param	InID		the ID to use for this dialog
	 * @param	InEditor	pointer to the editor window that contains this control
	 * @param   InStyle		Style of this tree control.
	 */
	void Create( wxWindow* InParent, wxWindowID InID, class WxKismet* InEditor, LONG InStyle = wxTR_HAS_BUTTONS );

	/**
	 * Repopulates the tree control with the sequences of all levels in the world.
	 */
	virtual void RefreshTree();

	/**
	 * Deletes all tree items from the list and clears the Sequence-ItemId map.
	 */
	void ClearTree();

	/**
	 * Recursively adds all subsequences of the specified sequence to the tree control.
	 */
	virtual void AddChildren( USequence* ParentSeq, wxTreeItemId ParentId );

	/**
	 * De/selects the tree item corresponding to the specified object
	 *
	 * @param	SeqObj		the sequence object to select in the tree control
	 * @param	bSelect		whether to select or deselect the sequence
	 *
	 * @param	True if the specified sequence object was found
	 */
	virtual UBOOL SelectObject( USequenceObject* SeqObj, UBOOL bSelect=TRUE );

	/** FSerializableObject interface */
	virtual void Serialize( FArchive& Ar );

protected:
	/** the kismet editor window that contains this tree control */
	class WxKismet* KismetEditor;

	/** maps the sequence objects to their corresponding tree id */
	TMap<USequenceObject*, wxTreeItemId> TreeMap;

	/**
	 * Adds the root sequence of the specified level to the sequence tree, if a sequence exists for the specified level.
	 *
	 * @param	Level		The level whose root sequence should be added.  Can be NULL.
	 * @param	RootId		The ID of the tree's root node.
	 */
	void AddLevelToTree(ULevel* Level, wxTreeItemId RootId);

private:
	/** Hide this constructor */
	WxSequenceTreeCtrl( wxWindow* parent, wxWindowID id, wxMenu* InMenu ) {}
};


/*-----------------------------------------------------------------------------
	WxKismetSearch
-----------------------------------------------------------------------------*/

enum EKismetSearchType
{
	KST_NameComment = 0,
	KST_ObjName = 1,
	KST_VarName = 2,
	KST_VarNameUses = 3,
	KST_RemoteEvents = 4,
	KST_ReferencesRemoteEvent = 5,

	MAX_KST_TYPES
};

enum EKismetSearchScope
{
	KSS_CurrentLevel = 0,
	KSS_CurrentSequence = 1,
	KSS_AllLevels = 2,

	MAX_KSS_TYPES
};

class WxKismetSearch : public wxFrame
{
public:
	WxKismetSearch( WxKismet* InKismet, wxWindowID InID );

	/**
	 * Loads prior search settings from the editor user settings ini.
	 */
	void LoadSearchSettings();

	/**
	 * Saves prior search settings to the editor user settings ini.
	 */
	void SaveSearchSettings();

	/**
	 * Clears the results list, clearing references to the previous results.
	 */
	void ClearResultsList();

	/**
	 * Appends a results to the results list.
	 *
	 * @param	ResultString	The string to display in the results list.
	 * @param	SequenceObj		The sequence object associated with the search result.
	 */
	void AppendResult(const TCHAR* ResultString, USequenceObject* SequenceObject);

	UBOOL SetResultListSelection(INT Index);

	/**
	 * @return		The selected search result sequence, or NULL if none selected.
	 */
	USequenceObject* GetSelectedResult() const;

	/**
	 * @return		The number of results in the results list.
	 */
	INT GetNumResults() const;

	/**
	 * @return		The current search string.
	 */
	FString GetSearchString() const;

	/**
	 * Sets the search string field.
	 *
	 * @param	SearchString	The new search string.
	 */
	void SetSearchString(const TCHAR* SearchString);

	/**
	 * @return		The selected search type setting.
	 */
	INT GetSearchType() const;

	/**
	 * Sets the search type.
	 */
	void SetSearchType(EKismetSearchType SearchType);

	/**
	 * @return		The selected search scope setting.
	 */
	INT GetSearchScope() const;

	/**
	 * Sets the search scope.
	 */
	void SetSearchScope(EKismetSearchScope SearchScope);

private:
	wxListBox*		ResultList;
	wxTextCtrl*		NameEntry;
	wxComboBox*		SearchTypeCombo;
	wxComboBox*		SearchScopeCombo;
	wxButton*		SearchButton;

	WxKismet*		Kismet;

	void OnClose(wxCloseEvent& In);

	DECLARE_EVENT_TABLE()
};

class WxKismetUpdate : public wxFrame
{
public:
	WxKismetUpdate( WxKismet* InKismet, wxWindowID InID );
	~WxKismetUpdate();

	void OnClose( wxCloseEvent& In );

	wxListBox*		UpdateList;
	wxButton		*UpdateButton, *UpdateAllButton;

	WxKismet*		Kismet;

	virtual void BuildUpdateList();
	void OnUpdateListChanged( wxCommandEvent &In );

	void OnContextUpdate( wxCommandEvent &In );
	void OnContextUpdateAll( wxCommandEvent &In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxKismet
-----------------------------------------------------------------------------*/

struct FCopyConnInfo
{
	class USequenceObject*	SeqObject;
	INT						ConnIndex;

	FCopyConnInfo()
	: SeqObject(NULL)
	{}

	FCopyConnInfo(USequenceObject* InSeqObject, INT InConnIndex) 
	: SeqObject(InSeqObject)
	, ConnIndex(InConnIndex)
	{}

	friend FArchive& operator<<(FArchive &Ar, FCopyConnInfo &Info);
};

struct FExposeVarLinkInfo
{
	// for EExposeType::Property
	class UProperty *Property;
	class UClass *VariableClass;
	// for EExposeType::HiddenLink
	INT LinkIdx;

	enum EExposeType
	{
		TYPE_Unknown = 0,
		TYPE_Property = 1,			// expose a property in a new link
		TYPE_HiddenLink = 2,		// expose a hidden link
	};
	// type of expose this is
	EExposeType Type;

	FExposeVarLinkInfo()
	: Property(NULL)
	, VariableClass(NULL)
	, LinkIdx(-1)
	, Type(TYPE_Unknown)
	{}

	FExposeVarLinkInfo(class UProperty *InProperty, class UClass *InVariableClass)
	: Property(InProperty)
	, VariableClass(InVariableClass)
	, LinkIdx(-1)
	, Type(TYPE_Property)
	{}

	FExposeVarLinkInfo(INT InLinkIdx)
	: Property(NULL)
	, VariableClass(NULL)
	, LinkIdx(InLinkIdx)
	, Type(TYPE_HiddenLink)
	{}

	friend FArchive& operator<<(FArchive &Ar, FExposeVarLinkInfo &Info);
};

//FIXME: change this to a configurable value maybe?
#define KISMET_GRIDSIZE			8

class WxKismet : public WxLinkedObjEd, public FCallbackEventDevice
{
public:
	WxKismet( wxWindow* InParent, wxWindowID InID, class USequence* InSequence, const TCHAR* Title = TEXT("Kismet") );
	virtual ~WxKismet();

	// Initalization / Accessors
	virtual void		InitEditor();
	virtual void		CreateControls( UBOOL bTreeControl );
	virtual USequence*	GetRootSequence();
//	virtual void		SetRootSequence( USequence* Seq );
	virtual void		InitSeqObjClasses();

	/**
	 * @return Returns the name of the inherited class, so we can generate .ini entries for all LinkedObjEd instances.
	 */
	virtual const TCHAR* GetConfigName() const
	{
		return TEXT("Kismet");
	}

	// LinkedObjEditor interface

	/**
	 * Creates the tree control for this linked object editor.  Only called if TRUE is specified for bTreeControl
	 * in the constructor.
	 *
	 * @param	TreeParent	the window that should be the parent for the tree control
	 */
	virtual void CreateTreeControl( wxWindow* TreeParent );

	virtual void OpenNewObjectMenu();
	virtual void OpenObjectOptionsMenu();
	virtual void OpenConnectorOptionsMenu();
	virtual void ClickedLine(FLinkedObjectConnector &Src, FLinkedObjectConnector &Dest);
	virtual void DoubleClickedLine(FLinkedObjectConnector &Src, FLinkedObjectConnector &Dest);
	virtual void DoubleClickedObject(UObject* Obj);
	virtual void DoubleClickedConnector(FLinkedObjectConnector& Connector);
	virtual UBOOL ClickOnBackground();
	virtual UBOOL ClickOnConnector(UObject* InObj, INT InConnType, INT InConnIndex);
	virtual void DrawObjects(FViewport* Viewport, FCanvas* Canvas);
	virtual void UpdatePropertyWindow();

	virtual void EmptySelection();
	virtual void AddToSelection( UObject* Obj );
	virtual void RemoveFromSelection( UObject* Obj );
	virtual UBOOL IsInSelection( UObject* Obj ) const;
	virtual INT GetNumSelected() const;

	virtual void SetSelectedConnector( FLinkedObjectConnector& Connector );
	virtual FIntPoint GetSelectedConnLocation(FCanvas* Canvas);
	virtual INT GetSelectedConnectorType();
	virtual UBOOL ShouldHighlightConnector(FLinkedObjectConnector& Connector);
	virtual FColor GetMakingLinkColor();

	// Make a connection between selected connector and an object or another connector.
	virtual void MakeConnectionToConnector( FLinkedObjectConnector& Connector );
	virtual void MakeConnectionToObject( UObject* EndObj );

	/**
	 * Called when the user releases the mouse over a link connector and is holding the ALT key.
	 * Commonly used as a shortcut to breaking connections.
	 *
	 * @param	Connector	The connector that was ALT+clicked upon.
	 */
	virtual void AltClickConnector(FLinkedObjectConnector& Connector);

	virtual void MoveSelectedObjects( INT DeltaX, INT DeltaY );
	virtual void PositionSelectedObjects();
	virtual void EdHandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event);
	virtual void OnMouseOver(UObject* Obj);
	virtual void ViewPosChanged();
	virtual void SpecialDrag( INT DeltaX, INT DeltaY, INT NewX, INT NewY, INT SpecialIndex );

	virtual void BeginTransactionOnSelected();
	virtual void EndTransactionOnSelected();

	virtual void NotifyObjectsChanged();

	// Static, initialiasation stuff

	/** 
	 * Opens a kismet window to edit the specified sequence.  If InSequence is NULL, the kismet
	 * of the current level will be opened, or created if it does not yet exist.
	 *
	 * @param	InSequence			The sequence to open.  If NULL, open the current level's root sequence.
	 * @param	bCreateNewWindow	If TRUE, open the sequence in a new kismet window.
	 * @param	WindowParent		When a window is being created, use this as its parent.
	 */
	static void OpenKismet(USequence* InSequence, UBOOL bCreateNewWindow, class wxWindow* ParentWindow);

	static void CloseAllKismetWindows();
	static void EnsureNoKismetEditingSequence(USequence* InSeq);

	/** 
	 *	Searches the current levels Kismet sequences to find any references to supplied Actor.
	 *	Opens a Kismet window if none are currently open, or re-uses first one. Opens SearchWindow as well if not currently open.
	 *
	 *	@param FindActor	Actor to find references to.
	 */
	static void FindActorReferences(AActor* FindActor);

	static void OpenSequenceInKismet(USequence* InSeq, wxWindow* ParentWindow);

	virtual void ChangeActiveSequence(USequence *newSeq, UBOOL bNotifyTree=true);
	void CenterViewOnSeqObj(USequenceObject* ViewObj);
	void ZoomSelection();
	void ZoomAll();
	void ZoomToBox(const FIntRect& Box);

	// Sequence Tools

	static void OpenMatinee(class USeqAct_Interp* Interp);
	void DeleteSelected(UBOOL bSilent=FALSE);
	void MakeLogicConnection(class USequenceOp* OutOp, INT OutConnIndex, class USequenceOp* InOp, INT InConnIndex);
	void MakeVariableConnection(class USequenceOp* Op, INT OpConnIndex, class USequenceVariable* Var);
	USequenceObject* NewSequenceObject(UClass* NewSeqObjClass, INT NewPosX, INT NewPosY, UBOOL bTransient=FALSE);
	virtual USequenceObject* NewShorcutObject();
	void SingleTickSequence();


	void KismetUndo();
	void KismetRedo();
	void KismetCopy();
	void KismetPaste(UBOOL bAtMousePos=false);
	void ClearCopiedConnections();

	void RebuildTreeControl();
	void BuildSelectedActorLists();
	FIntRect CalcBoundingBoxOfSelected();
	FIntRect CalcBoundingBoxOfAll();

	/** 
	*	Run a silent search now. 
	*	This will do a search without opening any search windows
	*
	* @param Results - the array to fill with search results
	* @param SearchString - the substring to search for
	* @param Type - indicates which fields to search
	* @param Scope - the scope of the search, which defaults to the current sequence
	*/
	void DoSilentSearch(TArray<USequenceObject*> &Results, const FString SearchString, EKismetSearchType Type, EKismetSearchScope Scope = KSS_CurrentSequence);
	void DoSearch(const TCHAR* InText, EKismetSearchType Type, UBOOL bJumpToFirstResult, EKismetSearchScope Scope = KSS_CurrentSequence);
	void UpdateSearchWindowResults(TArray<USequenceObject*> &Results);

	static UBOOL FindOutputLinkTo(const class USequence *sequence, const class USequenceOp *targetOp, const INT inputIdx, TArray<class USequenceOp*> &outputOps, TArray<INT> &outputIndices);
	static void ShowConnectors(TArray<USequenceObject*> &objList);
	static void HideConnectors(USequence *Sequence,TArray<USequenceObject*> &objList);

	// Context menu functions.

	void OnContextUpdateAction( wxCommandEvent& In );
	void OnContextNewScriptingObject( wxCommandEvent& In );
	virtual void OnContextNewScriptingObjVarContext( wxCommandEvent& In );
	void OnContextNewScriptingEventContext( wxCommandEvent& In );
	void OnContextCreateLinkedVariable( wxCommandEvent& In );
	void OnContextClearVariable( wxCommandEvent& In );
	void OnContextDelSeqObject( wxCommandEvent& In );
	void OnContextBreakLink( wxCommandEvent& In );
	void OnContextToggleLink( wxCommandEvent& In );
	void OnContextToggleLinkPIE( wxCommandEvent& In );
	void OnContextSelectEventActors( wxCommandEvent& In );
	void OnContextFireEvent( wxCommandEvent& In );
	void OnContextOpenInterpEdit( wxCommandEvent& In );
	void OnContextSwitchAdd( wxCommandEvent& In );
	void OnConextSwitchRemove( wxCommandEvent& In );
	void OnContextBreakAllOpLinks( wxCommandEvent& In );
	void OnContextHideConnectors( wxCommandEvent& In );
	void OnContextShowConnectors( wxCommandEvent& In );
	void OnContextFindNamedVarUses( wxCommandEvent& In );
	void OnContextFindNamedVarDefs( wxCommandEvent& In );
	void OnContextCreateSequence( wxCommandEvent& In );
	void OnContextExportSequence( wxCommandEvent& In );
	void OnContextImportSequence( wxCommandEvent& In );
	void OnContextPasteHere( wxCommandEvent& In );
	void OnContextLinkEvent( wxCommandEvent& In );
	void OnContextLinkObject( wxCommandEvent& In );
	void OnContextInsertIntoObjectList( wxCommandEvent& In );
	void OnContextRenameSequence( wxCommandEvent &In );	
	void OnContextCopyConnections( wxCommandEvent &In );	
	void OnContextPasteConnections( wxCommandEvent &In );	
	void OnContextExposeVariable( wxCommandEvent &In );
	void OnContextRemoveVariable( wxCommandEvent &In );
	void OnContextExposeOutput( wxCommandEvent &In );
	void OnContextRemoveOutput( wxCommandEvent &In );
	void OnContextSetOutputDelay( wxCommandEvent &In );
	void OnContextSetInputDelay( wxCommandEvent &In );
	void OnContextSelectAssigned( wxCommandEvent &In );
	void OnContextSearchRemoteEvents( wxCommandEvent &In );
	void OnContextApplyCommentStyle( wxCommandEvent &In );
	void OnContextCommentToFront( wxCommandEvent &In );
	void OnContextCommentToBack( wxCommandEvent &In );

	void OnButtonParentSequence( wxCommandEvent &In );	
	void OnButtonRenameSequence( wxCommandEvent &In );	
	void OnButtonHideConnectors( wxCommandEvent &In );
	void OnButtonShowConnectors( wxCommandEvent &In );
	void OnButtonShowCurves( wxCommandEvent &In );
	void OnButtonZoomToFit( wxCommandEvent &In );

	void OnTreeItemSelected( wxTreeEvent &In );
	void OnTreeExpanding( wxTreeEvent &In );
	void OnTreeCollapsed( wxTreeEvent &In );

	/**
	 * Toggles the "Search for SequencesObjects" tool.
	 */
	void OnOpenSearch( wxCommandEvent &In );

	void OnOpenUpdate( wxCommandEvent &In );

	/** 
	 * Handler for pressing Search button (or pressing Enter on entry box) on the Kismet Search tool.
	 * Searches all sequences and puts results into the ResultList list box.
	 */
	void OnDoSearch( wxCommandEvent &In );

	/**
	 * Called when user clicks on a search result. Changes active Sequence
	 * and moves the view to center on the selected SequenceObject.
	 */
	void OnSearchResultChanged( wxCommandEvent &In );

	/**
	 * Open the current level's root sequence in a new window.
	 */
	void OnOpenNewWindow( wxCommandEvent &In );

	void OnClose( wxCloseEvent& In );
	void OnSize(wxSizeEvent& In);

	//FCallbackEventDevice interface
	virtual void Send( ECallbackEventType InType );

	WxKismetToolBar*		ToolBar;
	WxKismetStatusBar*	StatusBar;
	WxKismetSearch*		SearchWindow;

	// Sequence currently being edited
	class USequence* Sequence;

	// Previously edited sequence. Used for Ctrl-Tab.
	class USequence* OldSequence;

	// Currently selected SequenceObjects
	TArray<class USequenceObject*> SelectedSeqObjs;

	// Selected Connector
	class USequenceOp* ConnSeqOp;
	INT ConnType;
	INT ConnIndex;

	INT PasteCount;
	INT DuplicationCount;

	// Used for copying/pasting connections.
	INT CopyConnType;
	TArray<FCopyConnInfo>	CopyConnInfo;

	// When creating a new event, here are the options. Generated by WxMBKismetNewObject each time its brought up.
	TArray<AActor*>		NewObjActors;
	TArray<UClass*>		NewEventClasses;
	UBOOL				bAttachVarsToConnector;

	// List of all SequenceObject classes.
	TArray<UClass*>		SeqObjClasses;

	/** List of properties that can be exposed for the currently selected action */
	TMap<INT,FExposeVarLinkInfo> OpExposeVarLinkMap;

	virtual void Serialize(FArchive& Ar);


protected:

	/**
	 * Returns whether the specified class can be displayed in the list of ops which are available to be placed.
	 *
	 * @param	SequenceClass	a child class of USequenceObject
	 *
	 * @return	TRUE if the specified class should be available in the context menus for adding sequence ops
	 */
	virtual UBOOL IsValidSequenceClass( UClass* SequenceClass ) const;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxMBKismetNewObject
-----------------------------------------------------------------------------*/

class WxMBKismetNewObject : public wxMenu
{
public:
	WxMBKismetNewObject(WxKismet* SeqEditor);
	~WxMBKismetNewObject();

	wxMenu *ActionMenu, *VariableMenu, *ConditionMenu, *EventMenu, *ContextEventMenu;
};

/*-----------------------------------------------------------------------------
	WxMBKismetConnectorOptions
-----------------------------------------------------------------------------*/

class WxMBKismetConnectorOptions : public wxMenu
{
public:
	wxMenu *BreakLinkMenu;
	WxMBKismetConnectorOptions(WxKismet* SeqEditor);
	~WxMBKismetConnectorOptions();
};

/*-----------------------------------------------------------------------------
	WxMBKismetObjectOptions
-----------------------------------------------------------------------------*/

class WxMBKismetObjectOptions : public wxMenu
{
public:
	WxMBKismetObjectOptions(WxKismet* SeqEditor);
	~WxMBKismetObjectOptions();

	wxMenu *NewVariableMenu;
};

#endif	// __KISMET_H__
