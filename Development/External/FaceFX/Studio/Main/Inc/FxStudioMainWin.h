//------------------------------------------------------------------------------
// FaceFx Studio's main window.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxStudioMainWin_H__
#define FxStudioMainWin_H__

#include "stdwx.h"

#include "FxConsoleWidget.h"
#include "FxFaceGraphWidget.h"
#include "FxFaceGraphNodePropertiesWidget.h"
#include "FxLinkFnEditDlg.h"
#include "FxActorWidget.h"
#include "FxCurveWidget.h"
#include "FxAudioWidget.h"
#include "FxPhonemeWidget.h"
#include "FxTimelineWidget.h"
#include "FxCommandPanel.h"
#include "FxRenderWidget.h"
#include "FxAudioPlayerWidget.h"
#include "FxAnalysisWidget.h"
#include "FxMappingWidget.h"
#include "FxStudioSession.h"
#include "FxTearoffWindow.h"
#include "FxPlatform.h"
#include "FxArray.h"
#include "FxMRUFileListManager.h"
#include "FxCoarticulationWidget.h"
#include "FxGestureWidget.h"
#include "FxWorkspaceEditWidget.h"
#include "FxDropButton.h"

#ifdef FX_NO_ONLINE_HELP
	class FxNullHelpController
	{
	public:
		bool Initialize( const wxString& )     { return false; }
		bool LoadFile( void )                  { return false; }
		bool DisplayContents( void )           { return false; }
		bool Quit( void )                      { return false; }
		bool DisplaySection( const wxString& ) { return false; }
	};
	#define FxHelpController FxNullHelpController
#else
	#define FxHelpController wxHelpController
#endif // FX_NO_ONLINE_HELP

namespace OC3Ent
{

namespace Face
{

class FxStudioMainWin : public wxFrame
{
	// Typedef for the parent class. 
    typedef wxFrame Super;
public:
	// Constructor.
	FxStudioMainWin( FxBool expertMode = FxFalse, wxWindow* parent = NULL, 
		wxWindowID id = -1, 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(800, 600) );
	// Destructor.
	virtual ~FxStudioMainWin();

	// Updates the title bar.
	void UpdateTitleBar( void );

	// Loads the options.
	void LoadOptions( void );

	// Returns the session.
	FxStudioSession& GetSession( void );
	// Returns the render widget so other applications can do custom render
	// widget specific actions.
	FxRenderWidget* GetRenderWidget( void );
	// Returns the help controller.
	FxHelpController& GetHelpController( void );

	// Runs the expiration check.
	FxBool IsExpired( FxAnalysisLicenseeInfo& licenseeInfo );

	// Friends.
	friend class FxStudioSession;
	friend class FxFreeFloatingFrame;
	friend class FxWorkspaceInstanceWidget;

protected:
	// The current session.
	FxStudioSession _session;

	// The notebook.
	wxNotebook* _notebook;

	// The array of tear-off windows.
	FxArray<FxTearoffWindow*> _tearoffWindows;

	// The main widgets.
	FxActorWidget* _actorWidget;
	FxFaceGraphWidget* _faceGraphWidget;
	FxNodePropertiesWidget* _faceGraphNodePropertiesWidget;
	FxNodeLinkPropertiesWidget* _inputLinkPropertiesWidget;
	FxNodeLinkPropertiesWidget* _outputLinkPropertiesWidget;
	FxCurveWidget* _curveWidget;
	FxAudioWidget* _audioWidget;
	FxPhonemeWidget* _phonemeWidget;
	FxTimelineWidget* _timelineWidget;
	FxCommandPanel* _commandPanel;
	FxRenderWidget* _renderWidget;
	FxAudioPlayerWidget* _audioPlayerWidget;
	FxAnalysisWidget* _analysisWidget;
    FxMappingWidgetContainer* _mappingWidget;
	FxFaceGraphWidgetContainer* _faceGraphContainer;
	FxCoarticulationWidget* _coarticulationWidget;
	FxGestureWidget* _gestureWidget;
	FxWorkspaceEditWidget* _workspaceEditWidget;
	FxWorkspaceWidget* _workspaceWidget;

	// The splitters
	FxSplitterWindow* _mainWindowSplitter;
	FxSplitterWindow* _facegraphSplitter;
	FxSplitterWindow* _animSplitter;

	// The bottom pane of the animation splitter.
	wxPanel* _bottomPane;

	// The console widget.
	FxConsoleWidget _consoleWidget;

	// The old notebook selection.
	wxWindow* _oldNotebookSelection;

	// FxTrue if the main window is currently in full screen mode.
	FxBool _isFullscreen;
	// Prevents the shutdown code from executing more than once.
	FxBool _hasAlreadyShutdown;

	// The MRU manager.
	FxMRUListManager* _mruManager;

	// Toolbar buttons.
#ifndef __UNREAL__
	FxButton* _toolbarNewActorButton;
	FxDropButton* _toolbarLoadActorButton;
#endif
	FxButton* _toolbarSaveActorButton;

	FxButton* _toolbarUndoButton;
	FxButton* _toolbarRedoButton;

	FxButton* _toolbarNodeGroupManagerButton;
	FxButton* _toolbarAnimManagerButton;
	FxButton* _toolbarCurveManagerButton;

	wxMenu* _recentFileMenu;

	FxHelpController* _helpController;

	// Handles opening the main menu to give code a chance to enable or disable
	// menu options.
	void OnMenuOpen( wxMenuEvent& event );
	// Handles creating a new actor.
	void OnNewActor( wxCommandEvent& event );
	// Handles loading an actor.
	void OnLoadActor( wxCommandEvent& event );
	// Handles the load actor drop button.
	void OnLoadActorDrop( wxCommandEvent& event );
	// Handles the MRU file list.
	void OnMRUFile( wxCommandEvent& event );
	// Handles closing an actor.
	void OnCloseActor( wxCommandEvent& event );
	// Handles Ctrl+S events even if the Save Actor menu item is disabled.
	void OnSaveActorProxy( wxCommandEvent& event );
	// Handles saving an actor.
	void OnSaveActor( wxCommandEvent& event );
	// Handles saving an actor with a new file name.
	void OnSaveActorAs( wxCommandEvent& event );
	// Handles search and replace name.
	void OnSearchAndReplaceName( wxCommandEvent& event );
	// Handles Ctrl+Z events even if the Undo menu item is disabled.
	void OnUndoProxy( wxCommandEvent& event );
    // Handles undo.
	void OnUndo( wxCommandEvent& event );
	// Handles redo.
	void OnRedo( wxCommandEvent& event );
	// Handles Ctrl+Y events even if the Redo menu item is disabled.
	void OnRedoProxy( wxCommandEvent& event );
	// Tears off a window.
	void OnTearoff( wxCommandEvent& event );
	// Makes the main window fullscreen.
	void OnFullscreen( wxCommandEvent& event );
	// Handles notebook page changing events.  This is called before the page
	// actually changes.
	void OnNotebookPageChanged( wxNotebookEvent& event );
	// Renames the actor.
	void OnRenameActor( wxCommandEvent& event );
	// Displays the face graph node group manager dialog.
	void OnFaceGraphNodeGroupManager( wxCommandEvent& event );
	// Displays the animation manager dialog.
	void OnAnimationManager( wxCommandEvent& event );
	// Displays the curve manager dialog.
	void OnCurveManager( wxCommandEvent& event );
	// Displays an applications options dialog.
	void OnAppOptions( wxCommandEvent& event );
	// Displays the about screen.
	void OnAbout( wxCommandEvent& event );
	// Displays the help
	void OnMenuHelp( wxCommandEvent& event );
	// Exports a template
	void OnExportTemplate( wxCommandEvent& event );
	// Syncs to a template
	void OnSyncTemplate( wxCommandEvent& event );
	// Mounts an animation set.
	void OnMountAnimSet( wxCommandEvent& event );
	// Unmounts an animation set.
	void OnUnmountAnimSet( wxCommandEvent& event );
	// Imports an animation set.
	void OnImportAnimSet( wxCommandEvent& event );
	// Exports an animation set.
	void OnExportAnimSet( wxCommandEvent& event );
	// Shows the default bone weight edit dialog.
	void OnEditDefaultBoneWeights( wxCommandEvent& event );
	// Shows the anim bone weight edit dialog.
	void OnEditAnimBoneWeights( wxCommandEvent& event );
	// Goes to the previous frame.
	void OnPrevFrame( wxCommandEvent& event );
	// Goes to the next frame.
	void OnNextFrame( wxCommandEvent& event );

	// Handles size changes.
	void OnSize( wxSizeEvent& event );
	// Exit handler.
	void OnExit( wxCommandEvent& event );
	// Close handler.
	void OnClose( wxCloseEvent& event );

	// Handles painting the main window.
	void OnPaint( wxPaintEvent& event );

	// Creates the application menu.
	void CreateMenus( void );
	// Creates the toolbar controls.
	void CreateToolbarControls( void );
	// Updates the toolbar control states.
	void UpdateToolbarControlStates(wxUpdateUIEvent& event);
	// Processes a menu element that should be redirected to a widget.
	void RelayMenuCommand( wxCommandEvent& event );

	void OnHelp(wxHelpEvent& event);

	// Load/save window positions.
	void SerializeWindowPositions(FxBool loading);

    DECLARE_EVENT_TABLE()
};

} // namespace Face

} // namespace OC3Ent

#endif
