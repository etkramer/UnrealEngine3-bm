//------------------------------------------------------------------------------
// FaceFx Studio's main window.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxStudioMainWin.h"
#include "FxSplitterWindow.h"
#include "FxSDK.h"
#include "FxArchive.h"
#include "FxStudioApp.h"
#include "FxVM.h"
#include "FxAudioFile.h"
#include "FxSessionProxy.h"
#include "FxStudioOptions.h"
#include "FxFaceGraphNodeGroupManagerDialog.h"
#include "FxAnimManagerDialog.h"
#include "FxUndoRedoManager.h"
#include "FxStudioAnimPlayer.h"
#include "FxCurveManagerDialog.h"
#include "FxAudioPlayer.h"
#include "FxAudioPlayerOpenAL.h"
#include "FxPhonWordList.h"
#include "FxFaceGraphNodeGroup.h"
#include "FxCGConfigManager.h"
#include "FxCameraManager.h"
#include "FxActorTemplate.h"
#include "FxAnimSetManager.h"
#include "FxBoneWeightDialog.h"
#include "FxSecurityWrapper.h"

#if defined(WIN32) && !defined(__UNREAL__)
	#include "FxAudioPlayerDirectSound8.h"
#endif

#ifdef __UNREAL__
	#include "XWindow.h"
	// Include the needed Unreal headers.
	#include "FxRenderWidgetUE3.h"
	// Include the needed Unreal implementations.
	#include "FxRenderWidgetUE3.cpp"
#else
	#include "zlib.h"
	#include "wx/dcbuffer.h"
	#include "wx/textdlg.h"
	#include "wx/choicdlg.h"
	#include "wx/treectrl.h"
	#include "wx/notebook.h"
	#include "wx/fileconf.h"
	#include "wx/splash.h"
#endif

#ifdef USE_FX_RENDERER
	#include "FxRenderWidgetOC3.h"
#endif

#ifdef USE_FX_RENDERER
	#ifdef FX_DEBUG
		#pragma message("Linking Debug FxRenderer and DevIL...")
		#pragma comment(lib, "FxRenderer_DMTDLL.lib")
	#else
		#pragma message("Linking Release FxRenderer and DevIL...")
		#pragma comment(lib, "FxRenderer_RMTDLL.lib")
	#endif
	#pragma comment(lib, "DevIL.lib")
	#pragma comment(lib, "ILU.lib")
	#pragma comment(lib, "ILUT.lib")
#endif

namespace OC3Ent
{

namespace Face
{

class FxNotebookDoubleClickHandler : public wxEvtHandler
{
	typedef wxEvtHandler Super;
protected:
	void OnLeftDoubleClick( wxMouseEvent& FxUnused(event) )
	{
		if( FxStudioApp::GetMainWindow() )
		{
			wxCommandEvent fakeTearoffKeyPress(wxEVT_COMMAND_MENU_SELECTED, MenuID_MainWindowTearoff);
			FxStudioApp::GetMainWindow()->ProcessEvent(fakeTearoffKeyPress);
		}
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(FxNotebookDoubleClickHandler, wxEvtHandler)
	EVT_LEFT_DCLICK(FxNotebookDoubleClickHandler::OnLeftDoubleClick)
END_EVENT_TABLE()

class FxNotebookClickAndDragHandler : public wxEvtHandler
{
	typedef wxEvtHandler Super;
public:

	FxNotebookClickAndDragHandler( wxNotebook* pNotebook )
		: Super()
		, _pNotebook(pNotebook)
		, _leftIsDown(FxFalse)
	{
		_dragDelta = wxSystemSettings::GetMetric(wxSYS_DRAG_X);
	}

protected:
	wxNotebook* _pNotebook;
	FxBool _leftIsDown;
	wxPoint _startMousePos;
	FxInt32 _dragDelta;

	void OnLeftClick( wxMouseEvent& event )
	{
		_leftIsDown = FxTrue;
		if( _pNotebook )
		{
			_pNotebook->CaptureMouse();
		}
		_startMousePos = event.GetPosition();
		event.Skip();
	}

	void OnLeftUp( wxMouseEvent& event )
	{
		_leftIsDown = FxFalse;
		if( _pNotebook )
		{
			_pNotebook->ReleaseMouse();
		}
		event.Skip();
	}

	void OnMouseMove( wxMouseEvent& event )
	{
		if( _leftIsDown && _pNotebook )
		{
			wxPoint mousePos = event.GetPosition();
			if( FxAbs(mousePos.x - _startMousePos.x) >= _dragDelta )
			{
				FxInt32 tabUnderMouse = _pNotebook->HitTest(mousePos);
				FxInt32 selectedTab = _pNotebook->GetSelection();
				if( wxNOT_FOUND != tabUnderMouse && selectedTab != tabUnderMouse )
				{
					wxNotebookPage* pageBeingDragged = reinterpret_cast<wxNotebookPage*>(_pNotebook->GetPage(selectedTab));
					wxString textOfPageBeingDragged = _pNotebook->GetPageText(selectedTab);
					_pNotebook->RemovePage(selectedTab);
					_pNotebook->InsertPage(tabUnderMouse, pageBeingDragged, textOfPageBeingDragged, true);
					_startMousePos = mousePos;
				}
			}
		}
		event.Skip();
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(FxNotebookClickAndDragHandler, wxEvtHandler)
	EVT_LEFT_DOWN(FxNotebookClickAndDragHandler::OnLeftClick)
	EVT_LEFT_UP(FxNotebookClickAndDragHandler::OnLeftUp)
	EVT_MOTION(FxNotebookClickAndDragHandler::OnMouseMove)
END_EVENT_TABLE()

#define MENU_FILE 0
#define MENU_EDIT 1
#define MENU_VIEW 2
#define MENU_ACTOR 3
#define MENU_TOOLS 4
#define MENU_HELP 5

BEGIN_EVENT_TABLE(FxStudioMainWin, wxFrame)
	EVT_MENU_OPEN(FxStudioMainWin::OnMenuOpen)
	EVT_MENU(wxID_EXIT, FxStudioMainWin::OnExit)
	EVT_MENU(MenuID_MainWindowFileNewActor, FxStudioMainWin::OnNewActor)
	EVT_MENU(MenuID_MainWindowFileLoadActor, FxStudioMainWin::OnLoadActor)
	EVT_COMMAND(MenuID_MainWindowFileLoadActor, wxEVT_COMMAND_BUTTON_DROP_CLICKED, FxStudioMainWin::OnLoadActorDrop)
	EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, FxStudioMainWin::OnMRUFile )
	EVT_MENU(MenuID_MainWindowFileCloseActor, FxStudioMainWin::OnCloseActor)
	EVT_MENU(MenuID_MainWindowFileSaveActorProxy, FxStudioMainWin::OnSaveActorProxy)
	EVT_MENU(MenuID_MainWindowFileSaveActor, FxStudioMainWin::OnSaveActor)
	EVT_MENU(MenuID_MainWindowFileSaveActorAs, FxStudioMainWin::OnSaveActorAs)
	EVT_MENU(MenuID_MainWindowEditSearchAndReplaceName, FxStudioMainWin::OnSearchAndReplaceName)
	EVT_MENU(MenuID_MainWindowEditUndoProxy, FxStudioMainWin::OnUndoProxy)
	EVT_MENU(MenuID_MainWindowEditUndo, FxStudioMainWin::OnUndo)
	EVT_MENU(MenuID_MainWindowEditRedoProxy, FxStudioMainWin::OnRedoProxy)
	EVT_MENU(MenuID_MainWindowEditRedo, FxStudioMainWin::OnRedo)
	EVT_MENU(MenuID_MainWindowTearoff, FxStudioMainWin::OnTearoff)
	EVT_MENU(MenuID_MainWindowViewFullscreen, FxStudioMainWin::OnFullscreen)
	EVT_MENU(MenuID_MainWindowActorRenameActor, FxStudioMainWin::OnRenameActor)
	EVT_MENU(MenuID_MainWindowActorFaceGraphNodeGroupManager, FxStudioMainWin::OnFaceGraphNodeGroupManager)
	EVT_MENU(MenuID_MainWindowActorAnimationManager, FxStudioMainWin::OnAnimationManager)
	EVT_MENU(MenuID_MainWindowActorCurveManager, FxStudioMainWin::OnCurveManager)
	EVT_MENU(MenuID_MainWindowOptionsAppOptions, FxStudioMainWin::OnAppOptions)
	EVT_MENU(MenuID_MainWindowHelpAbout, FxStudioMainWin::OnAbout)
	EVT_MENU(MenuID_MainWindowHelpHelp, FxStudioMainWin::OnMenuHelp)
	EVT_MENU(MenuID_MainWindowActorExportTemplate, FxStudioMainWin::OnExportTemplate)
	EVT_MENU(MenuID_MainWindowActorSyncTemplate, FxStudioMainWin::OnSyncTemplate)
	EVT_MENU(MenuID_MainWindowActorMountAnimset, FxStudioMainWin::OnMountAnimSet)
	EVT_MENU(MenuID_MainWindowActorUnmountAnimset, FxStudioMainWin::OnUnmountAnimSet)
	EVT_MENU(MenuID_MainWindowActorImportAnimset, FxStudioMainWin::OnImportAnimSet)
	EVT_MENU(MenuID_MainWindowActorExportAnimset, FxStudioMainWin::OnExportAnimSet)
	EVT_MENU(MenuID_MainWindowActorEditDefaultBoneWeights, FxStudioMainWin::OnEditDefaultBoneWeights)
	EVT_MENU(MenuID_MainWindowActorEditAnimBoneWeights, FxStudioMainWin::OnEditAnimBoneWeights)
	EVT_MENU(MenuID_MainWindowActorPrevFrame, FxStudioMainWin::OnPrevFrame)
	EVT_MENU(MenuID_MainWindowActorNextFrame, FxStudioMainWin::OnNextFrame)
	EVT_SIZE(FxStudioMainWin::OnSize)
	EVT_NOTEBOOK_PAGE_CHANGED(ControlID_MainWindowNotebook, FxStudioMainWin::OnNotebookPageChanged)
	EVT_CLOSE(FxStudioMainWin::OnClose)
	EVT_PAINT(FxStudioMainWin::OnPaint)
	EVT_UPDATE_UI_RANGE(MenuID_MainWindowFileNewActor, MenuID_MainWindowHelpAbout, FxStudioMainWin::UpdateToolbarControlStates)

	EVT_MENU_RANGE(MenuID_MainWindowForwardStart, MenuID_MainWindowForwardEnd, FxStudioMainWin::RelayMenuCommand)
	EVT_HELP_RANGE(wxID_ANY, wxID_HIGHEST, FxStudioMainWin::OnHelp)
END_EVENT_TABLE()

FxStudioMainWin::FxStudioMainWin( FxBool expertMode, wxWindow* parent, 
								  wxWindowID id, const wxPoint& pos, 
								  const wxSize& size )
    : wxFrame(parent, id, APP_TITLE, pos, size)
{
	// Set a minimum size for the window.
	SetSizeHints(640, 480);

	// Register the classes.
	FxStudioSession::StaticClass();
	FxAudioPlayer::StaticClass();
#ifndef __APPLE__
	FxAudioPlayerOpenAL::StaticClass();
#endif
#if defined(WIN32) && !defined(__UNREAL__)
	FxAudioPlayerDirectSound8::StaticClass();
#endif
	FxAudio::StaticClass();
	// This is required for backwards compatibility reasons.
	FxDigitalAudio::StaticClass();
	FxAnimUserData::StaticClass();
	FxFaceGraphNodeGroup::StaticClass();
	FxPhonWordList::StaticClass();
	FxViewportCamera::StaticClass();
	FxWorkspace::StaticClass();
	FxActorTemplate::StaticClass();
	FxAnimSetMiniSession::StaticClass();


	FxVM::Startup();

	// Startup the virtual machine.
	FxSessionProxy::SetSession(&_session);
	FxBool canContinue = FxTrue;
	FxCheckLicense1(canContinue);
	if( canContinue )
	{
		// Startup the audio file system.
		FxAudioFile::Startup();
    
		// Startup the Studio time management system.
		FxTimeManager::Startup();

		// Startup the Studio animation player
		FxStudioAnimPlayer::Startup();

		// Startup the FxCG configuration manager.
		FxCGConfigManager::Startup();

		// Initialize the help controller.
		wxFileName helpPath(FxStudioApp::GetAppDirectory());
		helpPath.AppendDir(wxT("Documentation"));
		helpPath.SetName(wxT("FaceFX.chm"));
		_helpController = new FxHelpController();
		FxAssert(_helpController);
		_helpController->Initialize(helpPath.GetFullPath());
	}

	// Create the MRU manager.
	_mruManager = new FxMRUListManager(&_session);
    
	_isFullscreen = FxFalse;
	_hasAlreadyShutdown = FxFalse;
	_oldNotebookSelection = NULL;

	// Calm down CPU usage in wxWidgets 2.6.2.
	wxUpdateUIEvent::SetUpdateInterval(50);

	// Set up the application icon.  To make wxWindows show the executable's
	// icon automatically, the icon must be the first one in the resource file.
#if !defined(__UNREAL__) && !defined(__APPLE__)
	wxIcon studioIcon(wxICON(APP_ICON));
	SetIcon(studioIcon);
#endif
	// Set up the menus.
	CreateMenus();

	// Set up the status bar.
    wxStatusBar* statusBar = new wxStatusBar(this, wxID_ANY, 2);
	const int statusBarWidths[2] = {-4,-1};
	statusBar->SetFieldsCount(2, statusBarWidths);
    SetStatusBar(statusBar);
	SetStatusBarPane(0);

	// Set up the accelerator table.
	static const int numEntries = 12;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set(wxACCEL_NORMAL, WXK_F10, MenuID_MainWindowTearoff);
	entries[1].Set(wxACCEL_ALT, WXK_RETURN, MenuID_MainWindowViewFullscreen);
	entries[2].Set(wxACCEL_NORMAL, WXK_F11, MenuID_MainWindowViewFullscreen );
	entries[3].Set(wxACCEL_SHIFT|wxACCEL_ALT, WXK_RETURN, MenuID_MainWindowViewFullscreen );
	entries[4].Set(wxACCEL_CTRL, (FxInt32)'O', MenuID_MainWindowFileLoadActor);
	entries[5].Set(wxACCEL_CTRL, (FxInt32)'N', MenuID_MainWindowFileNewActor);
	entries[6].Set(wxACCEL_CTRL, (FxInt32)'S', MenuID_MainWindowFileSaveActorProxy);
	entries[7].Set(wxACCEL_CTRL, (FxInt32)'Z', MenuID_MainWindowEditUndoProxy);
	entries[8].Set(wxACCEL_CTRL, (FxInt32)'Y', MenuID_MainWindowEditRedoProxy);
	entries[9].Set(wxACCEL_CTRL|wxACCEL_SHIFT, (FxInt32)'Z', MenuID_MainWindowEditRedoProxy);
	entries[10].Set(wxACCEL_NORMAL, WXK_F5, MenuID_MainWindowActorPrevFrame);
	entries[11].Set(wxACCEL_NORMAL, WXK_F6, MenuID_MainWindowActorNextFrame);
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator(numEntries, entries);
	SetAcceleratorTable(accelerator);

	// Set up the main window.
	SetSizer(new wxBoxSizer(wxVERTICAL));

	// Set up the tool bar controls - MUST be right after creating the main window
	// sizer and before adding anything else to said sizer.
	CreateToolbarControls();

	SetAutoLayout(TRUE);
	_mainWindowSplitter = new FxSplitterWindow(this, wxID_ANY, FxFalse);
	// Prevent un-splitting.
	_mainWindowSplitter->SetMinimumPaneSize(150);
	GetSizer()->Add(_mainWindowSplitter, 1, wxGROW, 5);
	_actorWidget = new FxActorWidget(_mainWindowSplitter, &_session);

	_analysisWidget = new FxAnalysisWidget(&_session);
	_analysisWidget->SetAnimControllerPointer(_session.GetAnimController());

	_audioPlayerWidget = new FxAudioPlayerWidget(&_session);
			
	wxPanel* leftPane = new wxPanel(_mainWindowSplitter);
	leftPane->SetSizer(new wxBoxSizer(wxVERTICAL));
	leftPane->SetAutoLayout(TRUE);

	_notebook = new wxNotebook(leftPane, ControlID_MainWindowNotebook, wxDefaultPosition, wxDefaultSize, wxNB_FIXEDWIDTH);
	leftPane->GetSizer()->Add(_notebook, 1, wxGROW, 5);
	_notebook->PushEventHandler(new FxNotebookDoubleClickHandler());
	_notebook->PushEventHandler(new FxNotebookClickAndDragHandler(_notebook));

	_mainWindowSplitter->SplitVertically(_actorWidget, leftPane, 150);
	_mainWindowSplitter->Initialize(_("Actor Panel"), _("Studio Panels"));

#ifdef USE_FX_RENDERER
	_renderWidget = new FxRenderWidgetOC3(_notebook, &_session);
#else
	_renderWidget = NULL;
#endif

#ifdef __UNREAL__
	_renderWidget = new FxRenderWidgetUE3(_notebook, &_session);
#endif

	FxTearoffWindow* renderTearoffWindow = new FxTearoffWindow(_notebook, _renderWidget, _("Preview"));
	_tearoffWindows.PushBack(renderTearoffWindow);
	_notebook->AddPage(_renderWidget, renderTearoffWindow->GetTitle());

	_facegraphSplitter = new FxSplitterWindow(_notebook, wxID_ANY);
	_facegraphSplitter->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
	_facegraphSplitter->SetMinimumPaneSize(125);
	_faceGraphContainer = new FxFaceGraphWidgetContainer(_facegraphSplitter, &_session);
	_faceGraphWidget = _faceGraphContainer->GetFaceGraphWidget();
	_faceGraphWidget->SetAnimControllerPointer(_session.GetAnimController());
	wxPanel* lowerPane = new wxPanel(_facegraphSplitter);
	lowerPane->SetSizer(new wxBoxSizer(wxHORIZONTAL));
	lowerPane->SetAutoLayout(TRUE);
	_inputLinkPropertiesWidget = new FxNodeLinkPropertiesWidget(lowerPane, 
		FxNodeLinkPropertiesWidget::NS_InputLinks, &_session);
	lowerPane->GetSizer()->Add(_inputLinkPropertiesWidget, 1, wxALL|wxGROW, 5);
	_faceGraphNodePropertiesWidget = new FxNodePropertiesWidget(lowerPane, &_session);
	lowerPane->GetSizer()->Add(_faceGraphNodePropertiesWidget, 1, wxALL|wxGROW, 5);
	_faceGraphNodePropertiesWidget->BindToFaceGraphWidget(_faceGraphWidget);
	_outputLinkPropertiesWidget = new FxNodeLinkPropertiesWidget(lowerPane, 
		FxNodeLinkPropertiesWidget::NS_OutputLinks, &_session);
	lowerPane->GetSizer()->Add(_outputLinkPropertiesWidget, 1, wxALL|wxGROW, 5);
	_facegraphSplitter->SplitHorizontally(_faceGraphContainer, lowerPane, 290);
	_facegraphSplitter->Initialize(_("Face Graph Panel"), _("Face Graph Properties Panel"));
	FxTearoffWindow* faceGraphTearoffWindow = new FxTearoffWindow(_notebook, _facegraphSplitter, _("Face Graph"));
	_tearoffWindows.PushBack(faceGraphTearoffWindow);
	_notebook->AddPage(_facegraphSplitter, faceGraphTearoffWindow->GetTitle());
	
	_animSplitter = new FxSplitterWindow(_notebook, wxID_ANY);
	_curveWidget = new FxCurveWidget(_animSplitter, &_session);
	_curveWidget->SetAnimControllerPointer(_session.GetAnimController());

	_bottomPane = new wxPanel(_animSplitter);
	_bottomPane->SetSizer(new wxBoxSizer(wxVERTICAL));
	_bottomPane->SetAutoLayout(TRUE);

	_phonemeWidget = new FxPhonemeWidget(_bottomPane, &_session);
	_audioWidget = new FxAudioWidget(_bottomPane, &_session);
	_bottomPane->GetSizer()->Add(_audioWidget, 1, wxGROW, 5);
	_bottomPane->GetSizer()->Add(_phonemeWidget, 0, wxGROW, 5);

	_animSplitter->SplitHorizontally(_curveWidget, _bottomPane, 200);
	_animSplitter->SetMinimumPaneSize(150);
	_animSplitter->Initialize(_("Curve Editor Panel"), _("Audio Panel"));

	FxTearoffWindow* animTearoffWindow = new FxTearoffWindow(_notebook,_animSplitter,_("Animation"));
	_tearoffWindows.PushBack(animTearoffWindow);
	_notebook->AddPage(_animSplitter, animTearoffWindow->GetTitle());

	_mappingWidget = new FxMappingWidgetContainer(_notebook, &_session);
	FxTearoffWindow* mappingTearoffWindow = new FxTearoffWindow(_notebook,_mappingWidget,_("Mapping"));
	_tearoffWindows.PushBack(mappingTearoffWindow);
	_notebook->AddPage(_mappingWidget, mappingTearoffWindow->GetTitle());

	_timelineWidget = new FxTimelineWidget(this, &_session);
	GetSizer()->Add(_timelineWidget, 0, wxGROW, 0);

	_commandPanel = new FxCommandPanel(this);
	GetSizer()->Add(_commandPanel, 0, wxGROW, 0);

	if( expertMode )
	{
		_coarticulationWidget = new FxCoarticulationWidget(_notebook, &_session);
		FxTearoffWindow* coarticulationTearoffWindow = new FxTearoffWindow(_notebook, _coarticulationWidget, _("Coarticulation"));
		_tearoffWindows.PushBack(coarticulationTearoffWindow);
		_notebook->AddPage(_coarticulationWidget, coarticulationTearoffWindow->GetTitle());

		_gestureWidget = new FxGestureWidget(_notebook, &_session);
		FxTearoffWindow* gestureTearoffWindow = new FxTearoffWindow(_notebook, _gestureWidget, _("Gestures"));
		_tearoffWindows.PushBack(gestureTearoffWindow);
		_notebook->AddPage(_gestureWidget, gestureTearoffWindow->GetTitle());
	}
	else
	{
		_coarticulationWidget = NULL;
		_gestureWidget = NULL;
	}

	wxFileName logPath(FxStudioApp::GetAppDirectory());
	logPath.AppendDir(wxT("Logs"));
	FxString logsDirectory(logPath.GetFullPath().mb_str(wxConvLibc));
	FxConsole::Initialize(logsDirectory, "FaceFxStudioLog.html", &_consoleWidget);
	_consoleWidget.Initialize(_notebook);
	FxTearoffWindow* consoleTearoffWindow = new FxTearoffWindow(_notebook, _consoleWidget.GetHtmlWindow(), _("Console"));
	_tearoffWindows.PushBack(consoleTearoffWindow);
	// Delay adding the console widget to the notebook until after the workspaces have been initialized.
	
	FxMsg( "Initializing %s version %s...", APP_TITLE.mb_str(wxConvLibc), APP_VERSION.mb_str(wxConvLibc) );
	FxMsg( "Using FaceFX SDK version %s", FxSDKGetVersionString().GetData() );
	FxMsg( "Build Id: %s", FxSDKGetBuildId().GetData() );
	FxMsg( "FaceFX SDK licensee: %s", FxSDKGetLicenseeName().GetData() );
	FxMsg( "FaceFX SDK licensee project: %s", FxSDKGetLicenseeProjectName().GetData() );
	wxString wxVersionString = wxVERSION_STRING;
	FxMsg( "Using %s", wxVersionString.mb_str(wxConvLibc) );
	FxMsg( "Using zlib version %s", zlibVersion() );

	FxMsg( "OS Version: %s", ::wxGetOsDescription().mb_str(wxConvLibc) );
	wxMemorySize freeMemory = ::wxGetFreeMemory();
	if( freeMemory != -1 )
	{
		FxMsg( "Memory: %d bytes free", freeMemory );
	}
	else
	{
		FxMsg( "Memory: n/a" );
	}
	FxMsg( "System: %s", ::wxGetHostName().mb_str(wxConvLibc) );
	FxMsg( "User: %s", ::wxGetUserId().mb_str(wxConvLibc) );

	FxMsg( "Initializing widgets..." );
	if( _timelineWidget )
	{
		_timelineWidget->BindToTimeSubscriberGroup(0);
	}

	// Initialize the animation player.
	FxStudioAnimPlayer* animationPlayer = FxStudioAnimPlayer::Instance();
	if( animationPlayer )
	{
		animationPlayer->SetAudioPlayerWidgetPointer(_audioPlayerWidget);
		animationPlayer->SetSessionPointer(&_session);
		animationPlayer->SetTimelineWidgetPointer(_timelineWidget);
	}

#ifndef __UNREAL__
	// Print the libsndfile version.
	FxString libsndfileVersion("Using libsndfile version: ");
	libsndfileVersion += FxAudioFile::GetVersion();
	FxMsg(libsndfileVersion.GetCstr());
	// Print the supported audio file types.
	FxMsg("Supported audio file types:");
	FxArray<FxString> exts = FxAudioFile::GetSupportedExtensions();
	FxSize numExts = exts.Length();
	FxArray<FxString> descs = FxAudioFile::GetDescriptions();
	FxSize numDescs = descs.Length();
	FxAssert(numDescs == numExts);
	// Since numDescs is only checked in the above assert this prevents a 
	// compiler warning from being issued in Release builds.
	numDescs;
	for( FxSize i = 0; i < numExts; ++i )
	{
		FxString msg(descs[i]);
		msg += " (.";
		msg += exts[i];
		msg += ")";
		FxMsg(msg.GetData());
	}
#endif // __UNREAL__

	// Broadcast the application startup message.
	_session.OnAppStartup(NULL);

	// Startup the workspace manager.
	FxWorkspaceManager::Startup();
	
	// Start up the workspace system, which needs to be initialized after
	// the renderer has been started up.
	FxWidgetSwitcher* workspaceWidgetSwitcher = new FxWidgetSwitcher(_notebook);

	_workspaceWidget = new FxWorkspaceWidget(workspaceWidgetSwitcher, &_session);
	_workspaceWidget->SetAnimController(_session.GetAnimController());
	_workspaceEditWidget = new FxWorkspaceEditWidget(workspaceWidgetSwitcher, &_session);
	_workspaceEditWidget->SetAnimController(_session.GetAnimController());

	workspaceWidgetSwitcher->Initialize(_workspaceWidget, _workspaceEditWidget);
	
	FxTearoffWindow* workspaceWidgetTearoffWindow = new FxTearoffWindow(_notebook, workspaceWidgetSwitcher, _("Workspace"));
	_tearoffWindows.PushBack(workspaceWidgetTearoffWindow);
	_notebook->AddPage(workspaceWidgetSwitcher, workspaceWidgetTearoffWindow->GetTitle());

	// Delayed adding the console widget to the notebook.
	_notebook->AddPage(_consoleWidget.GetHtmlWindow(), consoleTearoffWindow->GetTitle());

	_workspaceWidget->OnAppStartup(NULL);
	_workspaceEditWidget->OnAppStartup(NULL);

	// Show the capabilities of the current render widget.
	FxRenderWidgetCaps renderWidgetCaps;
	_session.OnQueryRenderWidgetCaps(NULL, renderWidgetCaps);
	wxString trueString = _("TRUE");
	wxString falseString = _("FALSE");
	FxMsg( "Render Widget Capabilities:" );
	FxMsg( "Name: %s", renderWidgetCaps.renderWidgetName.GetData() );
	FxMsg( "Version: %f", static_cast<FxReal>(renderWidgetCaps.renderWidgetVersion/1000.0f) );
	FxMsg( "Supports Offscreen Render Targets: %s", (renderWidgetCaps.supportsOffscreenRenderTargets) ? trueString.mb_str(wxConvLibc) : falseString.mb_str(wxConvLibc) );
	FxMsg( "Supports Multiple Cameras: %s", (renderWidgetCaps.supportsMultipleCameras) ? trueString.mb_str(wxConvLibc) : falseString.mb_str(wxConvLibc) );
	FxMsg( "Supports Fixed Aspect Ratio Cameras: %s", (renderWidgetCaps.supportsFixedAspectRatioCameras) ? trueString.mb_str(wxConvLibc) : falseString.mb_str(wxConvLibc) );
	FxMsg( "Supports Bone Locked Cameras: %s", (renderWidgetCaps.supportsBoneLockedCameras) ? trueString.mb_str(wxConvLibc) : falseString.mb_str(wxConvLibc) );

#ifdef USE_FX_RENDERER
	FxInt32 compiledILVersion   = 0;
	FxInt32 compiledILUVersion  = 0;
	FxInt32 compiledILUTVersion = 0;
	FxInt32 inUseILVersion      = 0;
	FxInt32 inUseILUVersion     = 0;
	FxInt32 inUseILUTVersion    = 0;
	FxTexture::GetDevILVersionCompiledWith(compiledILVersion, compiledILUVersion, compiledILUTVersion);
	FxTexture::GetDevILVersionCurrentlyInUse(inUseILVersion, inUseILUVersion, inUseILUTVersion);

	FxMsg( "Compiled with: il version %d ilu version %d ilut version %d", 
		compiledILVersion, compiledILUVersion, compiledILUTVersion );
	FxMsg( "Using: il version %d ilu version %d ilut version %d",
		inUseILVersion, inUseILUVersion, inUseILUTVersion );
	if( inUseILVersion   < compiledILVersion  ||
		inUseILUVersion  < compiledILUVersion ||
		inUseILUTVersion < compiledILUTVersion ) 
	{
		FxWarn( "DevIL version different!" );	
	}
#endif // USE_FX_RENDERER

	FxMsg( "FaceFX Studio initialized properly." );
	if( expertMode )
	{
		FxMsg( "<b>Expert mode enabled!</b>" );
	}
}

FxStudioMainWin::~FxStudioMainWin()
{
	// Destroy the MRU manager
	delete _mruManager;
	_mruManager = NULL;

	for( FxSize i = 0; i < _tearoffWindows.Length(); ++i )
	{
		delete _tearoffWindows[i];
		_tearoffWindows[i] = NULL;
	}
	_tearoffWindows.Clear();
}

void FxStudioMainWin::UpdateTitleBar( void )
{
	FxString filename = _session.GetActorFilePath();
	wxString title = wxEmptyString;
	if( 0 == filename.Length() )
	{
		FxActor* pActor = _session.GetActor();
		if( pActor )
		{
			if( pActor->GetNameAsString().IsEmpty() )
		{
			title = wxString::Format(wxT("%s - New Actor"), APP_TITLE);
		}
		else
		{
				wxString actorNameString(pActor->GetNameAsCstr(), wxConvLibc);
				title = wxString::Format(wxT("%s - %s"), APP_TITLE, actorNameString);
			}
		}
		else
		{
			title = APP_TITLE;
		}
	}
	else
	{
		wxString file = wxString(filename.GetData(), wxConvLibc).AfterLast(FxPlatformPathSeparator); 
		title = wxString::Format(wxT("%s - %s"), APP_TITLE, file);
	}
	if( FxUndoRedoManager::CommandsToUndo() || _session.IsForcedDirty() )
	{
		title += wxT("*");
	}

	SetTitle(title);
}

void FxStudioMainWin::LoadOptions( void )
{
	// Load the options.
	FxStudioOptions::Load(&_session);
	SerializeWindowPositions(true);
	FxAssert(_bottomPane != NULL);
	_bottomPane->GetSizer()->SetItemMinSize(_phonemeWidget, 0, FxStudioOptions::GetPhonemeBarHeight());
}

FxStudioSession& FxStudioMainWin::GetSession( void )
{
	return _session;
}

FxRenderWidget* FxStudioMainWin::GetRenderWidget( void )
{
	return _renderWidget;
}

FxHelpController& FxStudioMainWin::GetHelpController( void )
{
	return *_helpController;
}

FxBool FxStudioMainWin::IsExpired( FxAnalysisLicenseeInfo& licenseeInfo )
{
	if( _analysisWidget )
	{
		licenseeInfo = _analysisWidget->GetLicenseeInfo();
		if( !_analysisWidget->IsExpired() )
		{
			return FxFalse;
		}
	}
	return FxTrue;
}

void FxStudioMainWin::OnMenuOpen( wxMenuEvent& event )
{
#ifndef __UNREAL__
	GetMenuBar()->GetMenu(MENU_FILE)->Enable(MenuID_MainWindowRecentFiles, _mruManager->GetNumMRUFiles() != 0);
#endif // __UNREAL__
	event.Skip();
}

void FxStudioMainWin::OnNewActor( wxCommandEvent& FxUnused(event) )
{
#ifndef __UNREAL__
	// Prompt to save.
	if( _session.PromptToSaveChangedActor() )
	{
		FxVM::ExecuteCommand("newActor");
		wxCommandEvent temp;
		OnRenameActor(temp);
	}
#endif
}

void FxStudioMainWin::OnLoadActor( wxCommandEvent& FxUnused(event) )
{
#ifndef __UNREAL__
	// Prompt to save.
	if( _session.PromptToSaveChangedActor() )
	{
		wxFileDialog fileDlg(this, _("Open FaceFX Actor"), wxEmptyString, wxEmptyString, _("FaceFX Actor (*.fxa)|*.fxa|All Files (*.*)|*.*"), wxFD_OPEN);
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		if( fileDlg.ShowModal() == wxID_OK )
		{
			FxString actorPath(fileDlg.GetPath().mb_str(wxConvLibc));
			FxString command("loadActor -file \"");
			command = FxString::Concat(command, actorPath);
			command = FxString::Concat(command, "\"");
			FxVM::ExecuteCommand(command);
		}
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
#endif
}

// Handles the load actor drop button.
void FxStudioMainWin::OnLoadActorDrop( wxCommandEvent& FxUnused(event) )
{
#ifndef __UNREAL__
	if( _mruManager->GetNumMRUFiles() )
	{
		PopupMenu(_recentFileMenu, wxPoint(_toolbarLoadActorButton->GetRect().GetRight()-10, _toolbarLoadActorButton->GetRect().GetBottom()));
	}
	else
	{
		// There are no MRU files -- display the standard open file dialog.
		wxCommandEvent temp;
		OnLoadActor(temp);
	}
#endif
}

void FxStudioMainWin::OnMRUFile( wxCommandEvent& event )
{
#ifndef __UNREAL__
	FxSize mruIndex = event.GetId() - wxID_FILE1;
	wxString actorPath(_mruManager->GetMRUFile(mruIndex));
	if( actorPath != wxEmptyString )
	{
		// Prompt to save.
		if( _session.PromptToSaveChangedActor() )
		{
			FxString command("loadActor -file \"");
			command = FxString::Concat(command, FxString(actorPath.mb_str(wxConvLibc)));
			command = FxString::Concat(command, "\"");
			FxVM::ExecuteCommand(command);
			if( !_session.GetActor() )
			{
				_mruManager->RemoveMRUFile(mruIndex);
			}
		}
	}
#endif
}

void FxStudioMainWin::OnCloseActor( wxCommandEvent& FxUnused(event) )
{
#ifndef __UNREAL__
	// Prompt to save.
	if( _session.PromptToSaveChangedActor() )
	{
		FxVM::ExecuteCommand("closeActor");
	}
#endif
}

void FxStudioMainWin::OnSaveActorProxy( wxCommandEvent& FxUnused(event) )
{
	wxCommandEvent temp;
	OnSaveActor(temp);
}

void FxStudioMainWin::OnSaveActor( wxCommandEvent& FxUnused(event) )
{
#ifndef __UNREAL__
	if( _session.GetActor() != NULL && "" != _session.GetActorFilePath() )
	{
		FxString command("saveActor -file \"");
		command = FxString::Concat(command, _session.GetActorFilePath());
		command = FxString::Concat(command, "\"");
		FxVM::ExecuteCommand(command);
	}
	else
	{
		wxCommandEvent temp;
		OnSaveActorAs(temp);
	}
#else
	_session.SaveActor(FxString());
#endif
}

void FxStudioMainWin::OnSaveActorAs( wxCommandEvent& FxUnused(event) )
{
#if !defined(__UNREAL__) && !defined(NO_SAVE_VERSION)
	if( _session.GetActor() )
	{
		wxFileDialog fileDlg(this, _("Save FaceFX Actor As"), wxEmptyString, wxEmptyString, _("FaceFX Actor (*.fxa)|*.fxa|All Files (*.*)|*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		FxString defaultActorFile = _session.GetActor()->GetNameAsString();
		defaultActorFile = FxString::Concat(defaultActorFile, ".fxa");
		fileDlg.SetFilename(wxString(defaultActorFile.GetData(), wxConvLibc));
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		if( fileDlg.ShowModal() == wxID_OK )
		{
			FxString command("saveActor -file \"");
			command = FxString::Concat(command, FxString(fileDlg.GetPath().mb_str(wxConvLibc)));
			command = FxString::Concat(command, "\"");
			FxVM::ExecuteCommand(command);
		}
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
#endif
}

void FxStudioMainWin::OnSearchAndReplaceName( wxCommandEvent& FxUnused(event) )
{
	if( wxYES == FxMessageBox(_("Globally searching and replacing a FaceFX name is not undoable. Continue?"), _("Search and Replace?"), wxYES_NO|wxICON_QUESTION) )
	{
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		wxTextEntryDialog oldNameDialog(this, _("Enter the name you wish to search for:"), _("Global Search And Replace Name Step 1"));
		int retval = oldNameDialog.ShowModal();
		wxString oldName = oldNameDialog.GetValue();
		if( wxID_OK == retval )
		{
			wxTextEntryDialog newNameDialog(this, _("Enter the name you wish to replace it with:"), _("Global Search And Replace Name Step 2"));
			retval = newNameDialog.ShowModal();
			wxString newName = newNameDialog.GetValue();
			if( wxID_OK == retval )
			{
				FxString command("rename -name \"");
				command = FxString::Concat(command, FxString(oldName.mb_str(wxConvLibc)));
				command = FxString::Concat(command, "\" -to \"");
				command = FxString::Concat(command, FxString(newName.mb_str(wxConvLibc)));
				command = FxString::Concat(command, "\"");
				FxVM::ExecuteCommand(command);
			}
		}
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
}

void FxStudioMainWin::OnUndoProxy( wxCommandEvent& FxUnused(event) )
{
	wxCommandEvent temp;
	OnUndo(temp);
}

void FxStudioMainWin::OnUndo( wxCommandEvent& FxUnused(event) )
{
	FxVM::ExecuteCommand("undo");
}

void FxStudioMainWin::OnRedoProxy( wxCommandEvent& FxUnused(event) )
{
	wxCommandEvent temp;
	OnRedo(temp);
}

void FxStudioMainWin::OnRedo( wxCommandEvent& FxUnused(event) )
{
	FxVM::ExecuteCommand("redo");
}

void FxStudioMainWin::OnTearoff( wxCommandEvent& FxUnused(event) )
{
	if( _notebook )
	{
		int selPage = _notebook->GetSelection();
		if( -1 != selPage )
		{
			// Tear the window off of the notebook.
			FxSize i = 0;
			wxNotebookPage* selPagePtr = _notebook->GetPage(selPage);
			for( ; i < _tearoffWindows.Length(); ++i )
			{
				if( _tearoffWindows[i] )
				{
					if( selPagePtr == _tearoffWindows[i]->GetWindow() )
					{
						if( _tearoffWindows[i]->IsTorn() == FxFalse )
						{
							_tearoffWindows[i]->Tear();
							break;
						}
					}
				}
			}
			// Select the old page.
			for( FxSize j = 0; j < _notebook->GetPageCount(); ++j )
			{
				if( _oldNotebookSelection == _notebook->GetPage(j) )
				{
					_notebook->SetSelection(j);
					break;
				}
			}
			// Make sure the newly torn off frame has focus.
			if( i < _tearoffWindows.Length() )
			{
				if( _tearoffWindows[i] && _tearoffWindows[i]->GetFreeFloatingFrame() )
				{
					_tearoffWindows[i]->GetFreeFloatingFrame()->SetFocus();
				}
			}
		}
	}
}

void FxStudioMainWin::OnFullscreen( wxCommandEvent& FxUnused(event) )
{
	_isFullscreen = !_isFullscreen;
	if( _isFullscreen )
	{
		ShowFullScreen(TRUE, 
			wxFULLSCREEN_NOSTATUSBAR|wxFULLSCREEN_NOBORDER|wxFULLSCREEN_NOCAPTION);
	}
	else
	{
		ShowFullScreen(FALSE);
	}
	GetMenuBar()->GetMenu(MENU_VIEW)->Check( MenuID_MainWindowViewFullscreen, _isFullscreen );
}

void FxStudioMainWin::OnNotebookPageChanged( wxNotebookEvent& event )
{
	if( _notebook )
	{
		// Remember the old page.
		int oldSelPage = event.GetOldSelection();
		if( -1 != oldSelPage )
		{
			_oldNotebookSelection = _notebook->GetPage(oldSelPage);
		}
	}
	event.Skip();
}

void FxStudioMainWin::OnRenameActor( wxCommandEvent& FxUnused(event) )
{
	FxActor* currActor = _session.GetActor();
	if( currActor )
	{
		wxString currActorName(currActor->GetNameAsCstr(), wxConvLibc);
		wxString actorName = ::wxGetTextFromUser(_("Enter new actor name:"), _("Rename Actor"), currActorName, this);
		if( actorName != wxEmptyString )
		{
			FxString command("renameActor -name \"");
			command = FxString::Concat(command, FxString(actorName.mb_str(wxConvLibc)));
			command = FxString::Concat(command, "\"");
			FxVM::ExecuteCommand(command);
		}
	}
}

void FxStudioMainWin::OnFaceGraphNodeGroupManager( wxCommandEvent& FxUnused(event) )
{
	FxFaceGraphNodeGroupManagerDialog faceGraphNodeGroupManager(this);
	faceGraphNodeGroupManager.Initialize(_session.GetActor(), &_session);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	faceGraphNodeGroupManager.ShowModal();
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxStudioMainWin::OnAnimationManager( wxCommandEvent& FxUnused(event) )
{
	FxAnimManagerDialog animManager;
	animManager.Initialize(_session.GetActor(), &_session);
	animManager.Create(this);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	animManager.ShowModal();
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxStudioMainWin::OnCurveManager( wxCommandEvent& FxUnused(event) )
{
	FxCurveManagerDialog curveManager;
	FxString selectedAnimGroup;
	FxString selectedAnim;
	if( FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup) &&
		FxSessionProxy::GetSelectedAnim(selectedAnim) )
	{
		curveManager.Initialize(_session.GetActor(), &_session, selectedAnimGroup.GetData(), selectedAnim.GetData());
	}
	else
	{
		curveManager.Initialize(_session.GetActor(), &_session);
	}
	curveManager.Create(this);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	curveManager.ShowModal();
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxStudioMainWin::OnAppOptions( wxCommandEvent& FxUnused(event) )
{
	FxStudioOptionsDialog optionsDlg(this, &_session);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( optionsDlg.ShowModal() == wxID_OK )
	{
		_session.OnRefresh(NULL);
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxStudioMainWin::OnAbout( wxCommandEvent& FxUnused(event) )
{
	wxString aboutBmpPath = FxStudioApp::GetBitmapPath(wxT("About.bmp"));
	wxBitmap aboutBoxBitmap(aboutBmpPath, wxBITMAP_TYPE_BMP);

	if( _analysisWidget )
	{
		FxAnalysisLicenseeInfo licenseeInfo = _analysisWidget->GetLicenseeInfo();
		wxMemoryDC dc;
		dc.SelectObject(aboutBoxBitmap);
		dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
		dc.SetTextForeground(*wxBLACK);
		dc.DrawText(wxString::Format(wxT("Licensed to: %s"), wxString::FromAscii(licenseeInfo.licenseeName)), 10, 210);
		dc.DrawText(wxString::Format(wxT("Project: %s"),  wxString::FromAscii(licenseeInfo.licenseeProject)), 10, 224);
		dc.DrawText(wxString::Format(wxT("Expires: %s"),  wxString::FromAscii(licenseeInfo.expirationDate)), 10, 238);
		dc.SelectObject(wxNullBitmap);
	}

	wxSplashScreen* about = NULL;
	about = new wxSplashScreen(aboutBoxBitmap,
		wxSPLASH_CENTRE_ON_PARENT|wxSPLASH_NO_TIMEOUT, -1, this, -1, 
		wxDefaultPosition, wxDefaultSize,
		wxSIMPLE_BORDER|wxSTAY_ON_TOP);
}

void FxStudioMainWin::OnMenuHelp( wxCommandEvent& FxUnused(event) )
{
	_helpController->LoadFile();
	_helpController->DisplayContents();
}

void FxStudioMainWin::OnExportTemplate( wxCommandEvent& FxUnused(event) )
{
	if( _session.GetActor() )
	{
		wxFileName defaultDir(FxStudioApp::GetAppDirectory());
		defaultDir.AppendDir(wxT("Templates"));
		wxFileDialog fileDlg(this, _("Export FaceFX Template"), wxEmptyString, wxEmptyString, _("FaceFX Template (*.fxt)|*.fxt|All Files (*.*)|*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		fileDlg.SetDirectory(defaultDir.GetPath());
		FxString defaultActorFile = _session.GetActor()->GetNameAsString();
		defaultActorFile = FxString::Concat(defaultActorFile, ".fxt");
		fileDlg.SetFilename(wxString(defaultActorFile.GetData(), wxConvLibc));
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		if( fileDlg.ShowModal() == wxID_OK )
		{
			FxString command("template -export -file \"");
			command += FxString(fileDlg.GetPath().mb_str(wxConvLibc));
			command += "\"";
			FxVM::ExecuteCommand(command);
		}
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
}

void FxStudioMainWin::OnSyncTemplate( wxCommandEvent& FxUnused(event) )
{
	// Prompt to save.
	if( wxYES == FxMessageBox(_("Syncing to a FaceFX Template is not undoable. Continue?"), _("Sync?"), wxYES_NO|wxICON_QUESTION) )
	{
		wxFileName defaultDir(FxStudioApp::GetAppDirectory());
		defaultDir.AppendDir(wxT("Templates"));
		wxFileDialog fileDlg(this, _("Open FaceFX Template"), wxEmptyString, wxEmptyString, _("FaceFX Template (*.fxt)|*.fxt|All Files (*.*)|*.*"), wxFD_OPEN);
		fileDlg.SetDirectory(defaultDir.GetPath());
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		if( fileDlg.ShowModal() == wxID_OK )
		{
			wxString choices[6] = {_("Face Graph"), _("Mapping"), _("Node Groups"), _("Cameras"), _("Workspaces"), _("Color Map")};
			wxArrayInt selections;
			selections.Add(0); selections.Add(1); selections.Add(2); selections.Add(3); selections.Add(4); selections.Add(5);
			FxSize numSelected = wxGetMultipleChoices(selections, _("Highlight the items to which to synch."),
				_("Choose items to synch"), 6, choices, this);
			if( numSelected )
			{
				FxString actorPath(fileDlg.GetPath().mb_str(wxConvLibc));
				FxString command("template -sync -file \"");
				command += actorPath;
				command += "\"";
				if( numSelected == 6 )
				{
					command += " -all";
				}
				else
				{
					for( FxSize i = 0; i < numSelected; ++i )
					{
						switch( selections[i] )
						{
						case 0:
							command += " -facegraph";
							break;
						case 1:
							command += " -mapping";
							break;
						case 2:
							command += " -nodegroups";
							break;
						case 3:
							command += " -cameras";
							break;
						case 4:
							command += " -workspaces";
							break;
						case 5:
							command += " -colormap";
							break;
						default:
							// Do nothing.
							break;
						};
					}
				}
				FxVM::ExecuteCommand(command);
			}
		}
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
}

void FxStudioMainWin::OnMountAnimSet( wxCommandEvent& FxUnused(event) )
{
	if( _session.GetActor() )
	{
		if( wxYES == FxMessageBox(_("Mounting an animation set is not undoable. Continue?"), _("Mount?"), wxYES_NO|wxICON_QUESTION) )
		{
			wxFileDialog fileDlg(this, _("Mount FaceFX Animation Set"), wxEmptyString, wxEmptyString, _("FaceFX Animation Set (*.fxe)|*.fxe|All Files (*.*)|*.*"), wxFD_OPEN);
			FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
			if( fileDlg.ShowModal() == wxID_OK )
			{
				FxString command("animSet -mount \"");
				command += FxString(fileDlg.GetPath().mb_str(wxConvLibc));
				command += "\"";
				FxVM::ExecuteCommand(command);
			}
			FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
		}
	}
}

void FxStudioMainWin::OnUnmountAnimSet( wxCommandEvent& FxUnused(event) )
{
	if( _session.GetActor() )
	{
		if( wxYES == FxMessageBox(_("Unmounting an animation set is not undoable. Continue?"), _("Unmount?"), wxYES_NO|wxICON_QUESTION) )
		{
			FxString animGroupToUnmount;
			FxSessionProxy::GetSelectedAnimGroup(animGroupToUnmount);
			FxString command("animSet -unmount \"");
			command += animGroupToUnmount;
			command += "\"";
			FxVM::ExecuteCommand(command);
		}
	}
}

void FxStudioMainWin::OnImportAnimSet( wxCommandEvent& FxUnused(event) )
{
	if( _session.GetActor() )
	{
		if( wxYES == FxMessageBox(_("Importing an animation set is not undoable. Continue?"), _("Import?"), wxYES_NO|wxICON_QUESTION) )
		{
			wxFileDialog fileDlg(this, _("Import FaceFX Animation Set"), wxEmptyString, wxEmptyString, _("FaceFX Animation Set (*.fxe)|*.fxe|All Files (*.*)|*.*"), wxFD_OPEN);
			FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
			if( fileDlg.ShowModal() == wxID_OK )
			{
				FxString command("animSet -import \"");
				command += FxString(fileDlg.GetPath().mb_str(wxConvLibc));
				command += "\"";
				FxVM::ExecuteCommand(command);
			}
			FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
		}
	}
}

void FxStudioMainWin::OnExportAnimSet( wxCommandEvent& FxUnused(event) )
{
	if( _session.GetActor() )
	{
		if( wxYES == FxMessageBox(_("Exporting an animation set is not undoable. Continue?"), _("Export?"), wxYES_NO|wxICON_QUESTION) )
		{
			FxString animGroupToExport;
			FxSessionProxy::GetSelectedAnimGroup(animGroupToExport);
			wxFileDialog fileDlg(this, _("Export FaceFX Animation Set"), wxEmptyString, wxEmptyString, _("FaceFX Animation Set (*.fxe)|*.fxe|All Files (*.*)|*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
			FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
			if( fileDlg.ShowModal() == wxID_OK )
			{
				FxString command("animSet -export \"");
				command += animGroupToExport;
				command += "\" -to \"";
				command += FxString(fileDlg.GetPath().mb_str(wxConvLibc));
				command += "\"";
				FxVM::ExecuteCommand(command);
			}
			FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
		}
	}
}

void FxStudioMainWin::OnEditDefaultBoneWeights( wxCommandEvent& FxUnused(event) )
{
	if( _session.GetActor() )
	{
		FxMasterBoneList& masterBoneList = _session.GetActor()->GetMasterBoneList();
		FxSize numBoneWeights = masterBoneList.GetNumRefBones();

		// Initialize the bone weights to all the bones in the MBL at 1.0.
		FxArray<FxAnimBoneWeight> boneWeights;
		boneWeights.Reserve(numBoneWeights);
		for( FxSize i = 0; i < numBoneWeights; ++i )
		{
			FxAnimBoneWeight boneWeight;
			boneWeight.boneName = masterBoneList.GetRefBone(i).GetName();
			boneWeight.boneWeight = masterBoneList.GetRefBoneWeight(boneWeight.boneName);
			boneWeights.PushBack(boneWeight);
		}

		FxBoneWeightDialog dialog(this);
		dialog.SetInitialBoneWeights(boneWeights);
		if( dialog.ShowModal() == wxID_OK )
		{
			boneWeights = dialog.GetFinalBoneWeights();
			for( FxSize i = 0; i < boneWeights.Length(); ++i )
			{
				masterBoneList.SetRefBoneWeight(boneWeights[i].boneName, boneWeights[i].boneWeight);
			}
			FxSessionProxy::SetIsForcedDirty(FxTrue);
			// Set the session time here rather than a standard refresh message
			// so that Studio correctly updates the animation and face graph 
			// state before rendering. 
			FxReal sessionTime;
			FxSessionProxy::GetSessionTime(sessionTime);
			FxSessionProxy::SetSessionTime(sessionTime);
		}
	}
}

void FxStudioMainWin::OnEditAnimBoneWeights( wxCommandEvent& FxUnused(event) )
{
	FxString animGroup, anim;
	if( _session.GetActor() &&
		FxSessionProxy::GetSelectedAnimGroup(animGroup) &&
		FxSessionProxy::GetSelectedAnim(anim) )
	{
		FxAnim* pAnim = NULL;
		FxSessionProxy::GetAnim(animGroup, anim, &pAnim);
		FxMasterBoneList& masterBoneList = _session.GetActor()->GetMasterBoneList();
		FxSize numBoneWeights = masterBoneList.GetNumRefBones();

		// Initialize the bone weights to all the bones in the MBL at 1.0.
		FxArray<FxAnimBoneWeight> boneWeights;
		boneWeights.Reserve(numBoneWeights);
		for( FxSize i = 0; i < numBoneWeights; ++i )
		{
			FxAnimBoneWeight boneWeight;
			boneWeight.boneName = masterBoneList.GetRefBone(i).GetName();
			boneWeight.boneWeight = masterBoneList.GetRefBoneWeight(boneWeight.boneName);
			boneWeights.PushBack(boneWeight);
		}

		// Check the animation for any overrides.
		for( FxSize i = 0; i < pAnim->GetNumBoneWeights(); ++i )
		{
			for( FxSize j = 0; j < boneWeights.Length(); ++j )
			{
				if( boneWeights[j].boneName == pAnim->GetBoneWeight(i).boneName )
				{
					boneWeights[j].boneWeight = pAnim->GetBoneWeight(i).boneWeight;
				}
			}
		}

		FxBoneWeightDialog dialog(this);
		dialog.SetInitialBoneWeights(boneWeights);
		if( dialog.ShowModal() == wxID_OK )
		{
			boneWeights = dialog.GetFinalBoneWeights();
			pAnim->RemoveAllBoneWeights();
			for( FxSize i = 0; i < boneWeights.Length(); ++i )
			{
				if( !FxRealEquality(boneWeights[i].boneWeight,
					masterBoneList.GetRefBoneWeight(boneWeights[i].boneName)) )
				{
					pAnim->AddBoneWeight(boneWeights[i]);
				}
			}
			FxSessionProxy::SetIsForcedDirty(FxTrue);
			// Set the session time here rather than a standard refresh message
			// so that Studio correctly updates the animation and face graph 
			// state before rendering. 
			FxReal sessionTime;
			FxSessionProxy::GetSessionTime(sessionTime);
			FxSessionProxy::SetSessionTime(sessionTime);
		}
	}
}

void FxStudioMainWin::OnPrevFrame( wxCommandEvent& FxUnused(event) )
{
	FxReal frameRate = 1.0f / FxStudioOptions::GetFrameRate();
	FxReal currentTime = 0.0f;
	FxSessionProxy::GetSessionTime(currentTime);
	FxReal frames = FxFloor(currentTime / frameRate);
	FxReal newTime = frameRate * frames;
	FxInt32 newFrame = FxNearestInteger(frames);
	if( FxRealEqualityRelaxed(newTime, currentTime) ) --newFrame;
	wxString command = wxString::Format(wxT("currentTime -frame %d"), newFrame);
	FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
}

void FxStudioMainWin::OnNextFrame( wxCommandEvent& FxUnused(event) )
{
	FxReal frameRate = 1.0f / FxStudioOptions::GetFrameRate();
	FxReal currentTime = 0.0f;
	FxSessionProxy::GetSessionTime(currentTime);
	FxReal frames = FxCeil(currentTime / frameRate);
	FxReal newTime = frameRate * frames;
	FxInt32 newFrame = FxNearestInteger(frames);
	if( FxRealEqualityRelaxed(newTime, currentTime) ) ++newFrame;
	wxString command = wxString::Format(wxT("currentTime -frame %d"), newFrame);
	FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
}

void FxStudioMainWin::OnSize( wxSizeEvent& event )
{
//	if( GetAutoLayout() )
//	{
//		Layout();
//	}
	event.Skip();
}

void FxStudioMainWin::OnExit( wxCommandEvent& event )
{
	if( _session.PromptToSaveChangedActor() )
	{
		FxVM::ExecuteCommand("quit");
	}
	event.Skip();
}

void FxStudioMainWin::OnClose( wxCloseEvent& event )
{
	if( !_session.PromptToSaveChangedActor() )
	{
		event.Veto();
		return;
	}

	// Don't execute this code more than once- e.g. the user was mashing the close
	// button repeatedly while loading an actor or something batches multiple
	// "quit" commands.
	if( !_hasAlreadyShutdown )
	{
		_hasAlreadyShutdown = FxTrue;

		// Quit the help system.
		_helpController->Quit();
		delete _helpController;

		// Save the options.
		SerializeWindowPositions(false);
		FxStudioOptions::Save(&_session);

		// Force all of the tear-off windows to dock themselves.
		FxTearoffWindow::DockAllCurrentlyTornTearoffWindows();

		// Broadcast the application shutdown message.
		_session.OnAppShutdown(NULL);

		// Clean up the audio player widget.
		if( _audioPlayerWidget )
		{
			delete _audioPlayerWidget;
			_audioPlayerWidget = NULL;
		}
		// Clean up the analysis widget.
		if( _analysisWidget )
		{
			delete _analysisWidget;
			_analysisWidget = NULL;
		}

		// Shutdown the workspace manager.
		FxWorkspaceManager::Shutdown();

		// Shutdown the FxCG configuration manager.
		FxCGConfigManager::Shutdown();

		// Shutdown the animation player
		FxStudioAnimPlayer::Shutdown();

		// Shutdown the Studio time management system.
		FxTimeManager::Shutdown();
		
		// Destroy the console.
		FxConsole::Destroy();

		// Shutdown the audio file system.
		FxAudioFile::Shutdown();

		// Shutdown the virtual machine.
		FxVM::Shutdown();

		// In Unreal, we want to tell the application that the main window has been
		// destroyed for bookkeeping purposes.
#ifdef __UNREAL__
		FxStudioApp::SetMainWindow(NULL);
#endif

		// Remove the event handlers from the notebook, since it doesn't seem
		// to do that itself.
		_notebook->PopEventHandler(FxTrue);
		_notebook->PopEventHandler(FxTrue);
	}

	event.Skip();
}

void FxStudioMainWin::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxRect clientRect = GetClientRect();
	wxRect toolbarRect(clientRect);
	toolbarRect.SetHeight(27);

	wxColour colourFace = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	wxColour colourShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	wxColour colourEngrave = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

	wxBufferedPaintDC dc(this);

	SetBackgroundColour(colourFace);
	dc.Clear();

	// Draw the toolbar background.
	dc.SetBrush(wxBrush(colourFace));
	dc.SetPen(wxPen(colourFace));
	dc.DrawRectangle(toolbarRect);

	// Shade the bottom edge.
	dc.SetPen(wxPen(colourShadow));
	dc.DrawLine(toolbarRect.GetLeft(), toolbarRect.GetBottom(),
		toolbarRect.GetRight(), toolbarRect.GetBottom());

	// Draw the separators.
	FxInt32 line0x = _toolbarSaveActorButton->GetRect().GetRight() + 3;
	FxInt32 line1x = _toolbarRedoButton->GetRect().GetRight() + 3;

	dc.SetPen(wxPen(colourEngrave));
	dc.DrawLine(line0x, toolbarRect.GetBottom() - 2,
				line0x, toolbarRect.GetTop() + 2);
	dc.DrawLine(line1x, toolbarRect.GetBottom() - 2,
				line1x, toolbarRect.GetTop() + 2);
}

void FxStudioMainWin::CreateMenus( void )
{
	wxMenuBar* menuBar = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu();
	_recentFileMenu = new wxMenu();
	_mruManager->AttachToMenu( _recentFileMenu );

#ifndef __UNREAL__
	fileMenu->Append(MenuID_MainWindowFileNewActor, _("&New Actor...\tCtrl+N"), _("Creates a new actor."));
	fileMenu->Append(MenuID_MainWindowFileLoadActor, _("&Open Actor...\tCtrl+O"), _("Loads an actor file."));
	fileMenu->Append(MenuID_MainWindowFileCloseActor, _("&Close Actor"), _("Closes the actor file."));
	fileMenu->AppendSeparator();
#endif
	fileMenu->Append(MenuID_MainWindowFileSaveActor, _("&Save Actor\tCtrl+S"), _("Saves the actor."));
#ifndef __UNREAL__
	fileMenu->Append(MenuID_MainWindowFileSaveActorAs, _("Save Actor &As..."), _("Saves the actor as a new file."));
	fileMenu->AppendSeparator();
	fileMenu->Append(MenuID_MainWindowRecentFiles, _("Recent &Files"), _recentFileMenu);
#endif
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, _("E&xit"), _("Exit FaceFX Studio."));
#ifndef __UNREAL__
	fileMenu->Enable(MenuID_MainWindowFileCloseActor, FALSE);
#endif
	fileMenu->Enable(MenuID_MainWindowFileSaveActor, FALSE);
#ifndef __UNREAL__
	fileMenu->Enable(MenuID_MainWindowFileSaveActorAs, FALSE);
#endif

	wxMenu* editMenu  = new wxMenu();
	editMenu->Append(MenuID_MainWindowEditSearchAndReplaceName, _("Global &Search and Replace Name..."), _("Globally searches for and replaces a FaceFX name."));
	editMenu->AppendSeparator();
	editMenu->Append(MenuID_MainWindowEditUndo, _("&Undo\tCtrl+Z"), _("Undo the last operation."));
	editMenu->Append(MenuID_MainWindowEditRedo, _("&Redo\tCtrl+Y"), _("Redo the last operation."));
	editMenu->Enable(MenuID_MainWindowEditSearchAndReplaceName, FALSE);
	editMenu->Enable(MenuID_MainWindowEditUndo, FALSE);
	editMenu->Enable(MenuID_MainWindowEditRedo, FALSE);

	wxMenu* viewMenu  = new wxMenu();
	viewMenu->Append(MenuID_MainWindowViewFullscreen, _("&Full Screen\tF11"), _("Displays FaceFX Studio in full-screen mode."), TRUE);
	viewMenu->Check(MenuID_MainWindowViewFullscreen, _isFullscreen);
	viewMenu->Append(MenuID_MainWindowForwardViewResetView, _("&Reset View\tCtrl+R"), _("Resets the time-synchronized widgets to the extents of the animation."));
	
	wxMenu* actorMenu = new wxMenu();
	actorMenu->Append(MenuID_MainWindowActorRenameActor, _("&Rename Actor..."), _("Change the actor name."));
	actorMenu->AppendSeparator();
	actorMenu->Append(MenuID_MainWindowActorFaceGraphNodeGroupManager, _("Node &Group Manager..."), _("Displays the face graph node group manager."));
	actorMenu->Append(MenuID_MainWindowActorAnimationManager, _("&Animation Manager..."), _("Displays the animation manager"));
	actorMenu->Append(MenuID_MainWindowActorCurveManager, _("&Curve Manager..."), _("Displays the curve manager"));
	actorMenu->Enable(MenuID_MainWindowActorFaceGraphNodeGroupManager, FALSE);
	actorMenu->Enable(MenuID_MainWindowActorAnimationManager, FALSE);
	actorMenu->Enable(MenuID_MainWindowActorCurveManager, FALSE);
	actorMenu->Enable(MenuID_MainWindowActorRenameActor, FALSE);
	actorMenu->AppendSeparator();
	actorMenu->Append(MenuID_MainWindowActorExportTemplate, _("&Export Template..."), _("Exports an actor template."));
	actorMenu->Append(MenuID_MainWindowActorSyncTemplate, _("S&ync to Template..."), _("Syncs to an actor template."));
	actorMenu->AppendSeparator();
#ifndef __UNREAL__
	actorMenu->Append(MenuID_MainWindowActorMountAnimset, _("&Mount Animation Set..."), _("Mounts an external animation set."));
	actorMenu->Append(MenuID_MainWindowActorUnmountAnimset, _("&Unmount Animation Set..."), _("Unmounts a mounted external animation set."));
	actorMenu->Append(MenuID_MainWindowActorImportAnimset, _("&Import Animation Set..."), _("Permanently imports an external animation set into the actor."));
	actorMenu->Append(MenuID_MainWindowActorExportAnimset, _("E&xport Animation Set..."), _("Exports an animation group to an external animation set."));
	actorMenu->AppendSeparator();
#endif
	actorMenu->Append(MenuID_MainWindowActorEditDefaultBoneWeights, _("Edit Default B&one Weights..."), _("Edit default bone weights"));
	actorMenu->Append(MenuID_MainWindowActorEditAnimBoneWeights, _("Edit Anim's &Bone Weights..."), _("Edit bone weights for the animation."));

	wxMenu* toolsMenu = new wxMenu();
	toolsMenu->Append(MenuID_MainWindowOptionsAppOptions, _("Application &Options..."), _("Displays the options for the entire application."));
	toolsMenu->AppendSeparator();
	toolsMenu->Append(MenuID_MainWindowForwardToolsOptionsPhonemebar, _("&Phoneme Bar Options..."), _("Displays the options for the phoneme bar."));
	toolsMenu->Append(MenuID_MainWindowForwardToolsOptionsAudioView, _("&Audio View Options..."), _("Displays the options for the audio view."));
	toolsMenu->Append(MenuID_MainWindowForwardToolsOptionsFaceGraph, _("&Face Graph Options..."), _("Displays the options for the face graph view."));
	
	wxMenu* helpMenu  = new wxMenu();
#ifndef FX_NO_ONLINE_HELP
	helpMenu->Append(MenuID_MainWindowHelpHelp, _("FaceFX Studio Help\tF1"), _("View the FaceFX Documentation."));
	helpMenu->AppendSeparator();
#endif // FX_NO_ONLINE_HELP
	helpMenu->Append(MenuID_MainWindowHelpAbout, _("About FaceFX Studio..."), _("View information about FaceFX Studio."));
				
	menuBar->Append(fileMenu, _("&File"));
	menuBar->Append(editMenu, _("&Edit"));
	menuBar->Append(viewMenu, _("&View"));
	menuBar->Append(actorMenu, _("&Actor"));
	menuBar->Append(toolsMenu, _("&Tools"));
	menuBar->Append(helpMenu, _("&Help"));
	SetMenuBar(menuBar);
}

void FxStudioMainWin::CreateToolbarControls( void )
{
	wxSizer* mainSizer = GetSizer();
	wxBoxSizer* toolbarSizer = new wxBoxSizer(wxHORIZONTAL);

#ifndef __UNREAL__
	_toolbarNewActorButton = new FxButton(this, MenuID_MainWindowFileNewActor, _("NA"), wxDefaultPosition, wxSize(20,20));
	_toolbarNewActorButton->MakeToolbar(FxTrue);
	_toolbarNewActorButton->SetToolTip(_("Create new actor"));
	toolbarSizer->Add(_toolbarNewActorButton, 0, wxLEFT|wxTOP|wxBOTTOM, 3);
	_toolbarLoadActorButton = new FxDropButton(this, MenuID_MainWindowFileLoadActor, _("LA"), wxDefaultPosition, wxSize(30,20));
	_toolbarLoadActorButton->MakeToolbar(FxTrue);
	_toolbarLoadActorButton->SetToolTip(_("Load actor"));
	toolbarSizer->Add(_toolbarLoadActorButton, 0, wxTOP|wxBOTTOM, 3);
#endif
	_toolbarSaveActorButton = new FxButton(this, MenuID_MainWindowFileSaveActorProxy, _("SA"), wxDefaultPosition, wxSize(20,20));
	_toolbarSaveActorButton->MakeToolbar(FxTrue);
	_toolbarSaveActorButton->SetToolTip(_("Save actor"));
	toolbarSizer->Add(_toolbarSaveActorButton, 0, wxTOP|wxBOTTOM, 3);

	toolbarSizer->AddSpacer(5);

	_toolbarUndoButton = new FxButton(this, MenuID_MainWindowEditUndo, _("U"), wxDefaultPosition, wxSize(20,20));
	_toolbarUndoButton->MakeToolbar(FxTrue);
	_toolbarUndoButton->SetToolTip(_("Undo"));
	toolbarSizer->Add(_toolbarUndoButton, 0, wxTOP|wxBOTTOM, 3);
	_toolbarRedoButton = new FxButton(this, MenuID_MainWindowEditRedo, _("R"), wxDefaultPosition, wxSize(20,20));
	_toolbarRedoButton->MakeToolbar(FxTrue);
	_toolbarRedoButton->SetToolTip(_("Redo"));
	toolbarSizer->Add(_toolbarRedoButton, 0, wxTOP|wxBOTTOM, 3);

	toolbarSizer->AddSpacer(5);

	_toolbarNodeGroupManagerButton = new FxButton(this, MenuID_MainWindowActorFaceGraphNodeGroupManager, _("NG"), wxDefaultPosition, wxSize(20,20));
	_toolbarNodeGroupManagerButton->MakeToolbar(FxTrue);
	_toolbarNodeGroupManagerButton->SetToolTip(_("Node Group Manager..."));
	toolbarSizer->Add(_toolbarNodeGroupManagerButton, 0, wxTOP|wxBOTTOM, 3);
	_toolbarAnimManagerButton = new FxButton(this, MenuID_MainWindowActorAnimationManager, _("AM"), wxDefaultPosition, wxSize(20,20));
	_toolbarAnimManagerButton->MakeToolbar(FxTrue);
	_toolbarAnimManagerButton->SetToolTip(_("Animation Manager..."));
	toolbarSizer->Add(_toolbarAnimManagerButton, 0, wxTOP|wxBOTTOM, 3);
	_toolbarCurveManagerButton = new FxButton(this, MenuID_MainWindowActorCurveManager, _("CM"), wxDefaultPosition, wxSize(20,20));
	_toolbarCurveManagerButton->MakeToolbar(FxTrue);
	_toolbarCurveManagerButton->SetToolTip(_("Curve Manager..."));
	toolbarSizer->Add(_toolbarCurveManagerButton, 0, wxTOP|wxBOTTOM, 3);

	mainSizer->Add(toolbarSizer);

	// Load the icons
#ifndef __UNREAL__
	wxString newPath = FxStudioApp::GetIconPath(wxT("new.ico"));
	wxString newDisabledPath = FxStudioApp::GetIconPath(wxT("new_disabled.ico"));
	if( FxFileExists(newPath) && FxFileExists(newDisabledPath) )
	{
		wxIcon newIcon = wxIcon(wxIconLocation(newPath));
		newIcon.SetHeight(16);
		newIcon.SetWidth(16);
		wxIcon newDisabledIcon = wxIcon(wxIconLocation(newDisabledPath));
		newDisabledIcon.SetHeight(16);
		newDisabledIcon.SetWidth(16);

		_toolbarNewActorButton->SetEnabledBitmap(newIcon);
		_toolbarNewActorButton->SetDisabledBitmap(newDisabledIcon);
	}

	wxString loadPath = FxStudioApp::GetIconPath(wxT("load.ico"));
	wxString loadDisabledPath = FxStudioApp::GetIconPath(wxT("load_disabled.ico"));
	if( FxFileExists(loadPath) && FxFileExists(loadDisabledPath) )
	{
		wxIcon loadIcon = wxIcon(wxIconLocation(loadPath));
		loadIcon.SetHeight(16);
		loadIcon.SetWidth(16);
		wxIcon loadDisabledIcon = wxIcon(wxIconLocation(loadDisabledPath));
		loadDisabledIcon.SetHeight(16);
		loadDisabledIcon.SetWidth(16);

		_toolbarLoadActorButton->SetEnabledBitmap(loadIcon);
		_toolbarLoadActorButton->SetDisabledBitmap(loadDisabledIcon);
	}
#endif

	wxString savePath = FxStudioApp::GetIconPath(wxT("save.ico"));
	wxString saveDisabledPath = FxStudioApp::GetIconPath(wxT("save_disabled.ico"));
	if( FxFileExists(savePath) && FxFileExists(saveDisabledPath) )
	{
		wxIcon saveIcon = wxIcon(wxIconLocation(savePath));
		saveIcon.SetHeight(16);
		saveIcon.SetWidth(16);
		wxIcon saveDisabledIcon = wxIcon(wxIconLocation(saveDisabledPath));
		saveDisabledIcon.SetHeight(16);
		saveDisabledIcon.SetWidth(16);

		_toolbarSaveActorButton->SetEnabledBitmap(saveIcon);
		_toolbarSaveActorButton->SetDisabledBitmap(saveDisabledIcon);
	}

	wxString undoPath = FxStudioApp::GetIconPath(wxT("undo.ico"));
	wxString undoDisabledPath = FxStudioApp::GetIconPath(wxT("undo_disabled.ico"));
	if( FxFileExists(undoPath) && FxFileExists(undoDisabledPath) )
	{
		wxIcon undoIcon = wxIcon(wxIconLocation(undoPath));
		undoIcon.SetHeight(16);
		undoIcon.SetWidth(16);
		wxIcon undoDisabledIcon = wxIcon(wxIconLocation(undoDisabledPath));
		undoDisabledIcon.SetHeight(16);
		undoDisabledIcon.SetWidth(16);

		_toolbarUndoButton->SetEnabledBitmap(undoIcon);
		_toolbarUndoButton->SetDisabledBitmap(undoDisabledIcon);
	}

	wxString redoPath = FxStudioApp::GetIconPath(wxT("redo.ico"));
	wxString redoDisabledPath = FxStudioApp::GetIconPath(wxT("redo_disabled.ico"));
	if( FxFileExists(redoPath) && FxFileExists(redoDisabledPath) )
	{
		wxIcon redoIcon = wxIcon(wxIconLocation(redoPath));
		redoIcon.SetHeight(16);
		redoIcon.SetWidth(16);
		wxIcon redoDisabledIcon = wxIcon(wxIconLocation(redoDisabledPath));
		redoDisabledIcon.SetHeight(16);
		redoDisabledIcon.SetWidth(16);

		_toolbarRedoButton->SetEnabledBitmap(redoIcon);
		_toolbarRedoButton->SetDisabledBitmap(redoDisabledIcon);
	}

	wxString nodegroupmanagerPath = FxStudioApp::GetIconPath(wxT("nodegroupmanager.ico"));
	wxString nodegroupmanagerDisabledPath = FxStudioApp::GetIconPath(wxT("nodegroupmanager_disabled.ico"));
	if( FxFileExists(nodegroupmanagerPath) && FxFileExists(nodegroupmanagerDisabledPath) )
	{
		wxIcon nodegroupmanagerIcon = wxIcon(wxIconLocation(nodegroupmanagerPath));
		nodegroupmanagerIcon.SetHeight(16);
		nodegroupmanagerIcon.SetWidth(16);
		wxIcon nodegroupmanagerDisabledIcon = wxIcon(wxIconLocation(nodegroupmanagerDisabledPath));
		nodegroupmanagerDisabledIcon.SetHeight(16);
		nodegroupmanagerDisabledIcon.SetWidth(16);

		_toolbarNodeGroupManagerButton->SetEnabledBitmap(nodegroupmanagerIcon);
		_toolbarNodeGroupManagerButton->SetDisabledBitmap(nodegroupmanagerDisabledIcon);
	}

	wxString animationmanagerPath = FxStudioApp::GetIconPath(wxT("animationmanager.ico"));
	wxString animationmanagerDisabledPath = FxStudioApp::GetIconPath(wxT("animationmanager_disabled.ico"));
	if( FxFileExists(animationmanagerPath) && FxFileExists(animationmanagerDisabledPath) )
	{
		wxIcon animationmanagerIcon = wxIcon(wxIconLocation(animationmanagerPath));
		animationmanagerIcon.SetHeight(16);
		animationmanagerIcon.SetWidth(16);
		wxIcon animationmanagerDisabledIcon = wxIcon(wxIconLocation(animationmanagerDisabledPath));
		animationmanagerDisabledIcon.SetHeight(16);
		animationmanagerDisabledIcon.SetWidth(16);

		_toolbarAnimManagerButton->SetEnabledBitmap(animationmanagerIcon);
		_toolbarAnimManagerButton->SetDisabledBitmap(animationmanagerDisabledIcon);
	}

	wxString curvemanagerPath = FxStudioApp::GetIconPath(wxT("curvemanager.ico"));
	wxString curvemanagerDisabledPath = FxStudioApp::GetIconPath(wxT("curvemanager_disabled.ico"));
	if( FxFileExists(curvemanagerPath) && FxFileExists(curvemanagerDisabledPath) )
	{
		wxIcon curvemanagerIcon = wxIcon(wxIconLocation(curvemanagerPath));
		curvemanagerIcon.SetHeight(16);
		curvemanagerIcon.SetWidth(16);
		wxIcon curvemanagerDisabledIcon = wxIcon(wxIconLocation(curvemanagerDisabledPath));
		curvemanagerDisabledIcon.SetHeight(16);
		curvemanagerDisabledIcon.SetWidth(16);

		_toolbarCurveManagerButton->SetEnabledBitmap(curvemanagerIcon);
		_toolbarCurveManagerButton->SetDisabledBitmap(curvemanagerDisabledIcon);
	}
}

// Updates the toolbar control states.
void FxStudioMainWin::UpdateToolbarControlStates(wxUpdateUIEvent& event)
{
	// Prevent this from executing if Studio is shutting down.
	if( !_hasAlreadyShutdown )
	{
		switch( event.GetId() )
		{
		case MenuID_MainWindowFileSaveActor:
#ifdef NO_SAVE_VERSION
			event.Enable(FxFalse);
#else
	#ifndef __UNREAL__
			event.Enable(_session.GetActor() != NULL && "" != _session.GetActorFilePath());
	#else
			event.Enable(_session.GetActor() != NULL);
	#endif
#endif
			break;
		case MenuID_MainWindowFileSaveActorProxy:
		case MenuID_MainWindowFileSaveActorAs:
		case MenuID_MainWindowActorExportTemplate:
#ifdef NO_SAVE_VERSION
			event.Enable(FxFalse);
#else
			event.Enable(_session.GetActor() != NULL);
#endif
			break;
		case MenuID_MainWindowFileCloseActor:
		case MenuID_MainWindowEditSearchAndReplaceName:
		case MenuID_MainWindowActorRenameActor:
		case MenuID_MainWindowActorFaceGraphNodeGroupManager:
		case MenuID_MainWindowActorAnimationManager:
		case MenuID_MainWindowActorCurveManager:
		case MenuID_MainWindowActorSyncTemplate:
#ifndef __UNREAL__
		case MenuID_MainWindowActorMountAnimset:
		case MenuID_MainWindowActorImportAnimset:
#endif
		case MenuID_MainWindowActorEditDefaultBoneWeights:
			event.Enable(_session.GetActor() != NULL);
			break;
		case MenuID_MainWindowActorEditAnimBoneWeights:
			if( _session.GetActor() != NULL )
			{
				// Only enable if a valid animation is selected.
				FxString selectedAnimGroup;
				FxString selectedAnimName;
				FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
                FxSessionProxy::GetSelectedAnim(selectedAnimName);
				FxSize animGroupIndex = _session.GetActor()->FindAnimGroup(selectedAnimGroup.GetData());
				if( FxInvalidIndex != animGroupIndex )
				{
					const FxAnimGroup& animGroup = _session.GetActor()->GetAnimGroup(animGroupIndex);
					if( FxInvalidIndex != animGroup.FindAnim(selectedAnimName.GetData()) )
					{
						event.Enable(FxTrue);
					}
					else
					{
						event.Enable(FxFalse);
					}
				}
				else
				{
					event.Enable(FxFalse);
				}
			}
			else
			{
				event.Enable(FxFalse);
			}
			break;
#ifndef __UNREAL__
		case MenuID_MainWindowActorUnmountAnimset:
			{
				if( _session.GetActor() != NULL )
				{
					// Only enable if a valid animation group is selected and it
					// is present in the animation set manager.
					FxString selectedAnimGroup;
					FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
					if( FxInvalidIndex != _session.GetActor()->FindAnimGroup(selectedAnimGroup.GetData()) && 
						FxInvalidIndex != FxAnimSetManager::FindMountedAnimSet(selectedAnimGroup.GetData()) )
					{
						event.Enable(FxTrue);
					}
					else
					{
						event.Enable(FxFalse);
					}
				}
				else
				{
					event.Enable(FxFalse);
				}
			}
			break;
		case MenuID_MainWindowActorExportAnimset:
			{
#ifdef NO_SAVE_VERSION
				event.Enable(FxFalse);
#else
				if( _session.GetActor() != NULL )
				{
					// Only enable if a valid animation group is selected and it
					// is NOT present in the animation set manager and it is
					// NOT the default animation group.
					FxString selectedAnimGroup;
					FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
					if( FxInvalidIndex != _session.GetActor()->FindAnimGroup(selectedAnimGroup.GetData())   && 
						FxInvalidIndex == FxAnimSetManager::FindMountedAnimSet(selectedAnimGroup.GetData()) &&
						selectedAnimGroup != FxAnimGroup::Default.GetAsString() )
					{
						event.Enable(FxTrue);
					}
					else
					{
						event.Enable(FxFalse);
					}
				}
				else
				{
					event.Enable(FxFalse);
				}
#endif
			}
			break;
#endif  // __UNREAL__
		case MenuID_MainWindowEditUndo:
			event.Enable(FxUndoRedoManager::CommandsToUndo());
			break;
		case MenuID_MainWindowEditRedo:
			event.Enable(FxUndoRedoManager::CommandsToRedo());
			break;
		default:
			// Nothing.
			break;
		}
	}
}

// Processes a menu element that should be redirected to a widget.
void FxStudioMainWin::RelayMenuCommand( wxCommandEvent& event )
{
	switch( event.GetId() )
	{
	case MenuID_MainWindowForwardViewResetView:
		{
			wxCommandEvent toRelay(wxEVT_COMMAND_MENU_SELECTED, MenuID_TimelineWidgetResetView);
			wxPostEvent(_timelineWidget, toRelay);
		}
		break;
	case MenuID_MainWindowForwardToolsOptionsPhonemebar:
		{
			wxCommandEvent toRelay(wxEVT_COMMAND_MENU_SELECTED, MenuID_PhonemeWidgetOptions);
			wxPostEvent(_phonemeWidget, toRelay);
		}
		break;
	case MenuID_MainWindowForwardToolsOptionsAudioView:
		{
			wxCommandEvent toRelay(wxEVT_COMMAND_MENU_SELECTED, MenuID_AudioWidgetEditOptions);
			wxPostEvent(_audioWidget, toRelay);
		}
		break;
	case MenuID_MainWindowForwardToolsOptionsFaceGraph:
		{
			wxCommandEvent toRelay(wxEVT_COMMAND_MENU_SELECTED, MenuID_FaceGraphWidgetOptions);
			wxPostEvent(_faceGraphWidget, toRelay);
		}
		break;
	default:
		FxAssert(!"No handler for that menu item!");
	};
}

void FxStudioMainWin::OnHelp(wxHelpEvent& FxUnused(event))
{
	_helpController->LoadFile();
#ifdef __UNREAL__
	_helpController->DisplaySection(wxT("Unreal Engine 3 Integration"));
#else
	_helpController->DisplayContents();
#endif // __UNREAL__
}

// Load/save window positions.
void FxStudioMainWin::SerializeWindowPositions(FxBool loading)
{
	if( loading )
	{
		if( !FxStudioApp::IsSafeMode() )
		{
			// Set the window's rect.
			wxRect mainWndRect = FxStudioOptions::GetMainWndRect();
			if( mainWndRect.GetWidth() > 0 && mainWndRect.GetHeight() > 0 &&
				mainWndRect.GetLeft()  > 0 && mainWndRect.GetTop() > 0 )
			{
				// Ensure the main window is within the screen's rect, in
				// case Studio was left on a detached dual monitor. Oops.
				int screenX = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
				int screenY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);

				if( mainWndRect.GetLeft() >= screenX ) mainWndRect.SetLeft(0);
				if( mainWndRect.GetTop() >= screenY ) mainWndRect.SetTop(0);

				SetSize(mainWndRect);
			}

			if( FxStudioOptions::GetMainWndMaximized() )
			{
				Maximize();
			}

			FxInt32 mainSplitter = 0;
			FxInt32 fgSplitter   = 0;
			FxInt32 animSplitter = 0;
			FxStudioOptions::GetSplitterPos(mainSplitter, fgSplitter, animSplitter);
			_mainWindowSplitter->SetSashPosition(mainSplitter);
			_facegraphSplitter->SetSashPosition(fgSplitter);
			_animSplitter->SetSashPosition(animSplitter);
			wxString selPageText = FxStudioOptions::GetSelectedTab();
			for( FxSize j = 0; j < _notebook->GetPageCount(); ++j )
			{
				if( selPageText == _notebook->GetPageText(j) )
				{
					_notebook->SetSelection(j);
					break;
				}
			}
		}
	}
	else // saving
	{
		// Save the window's rect.
		FxStudioOptions::SetMainWndRect(GetRect());
		FxStudioOptions::SetMainWndMaximized(IsMaximized());
		FxStudioOptions::SetSplitterPos(_mainWindowSplitter->GetSashPosition(),
										_facegraphSplitter->GetSashPosition(),
										_animSplitter->GetSashPosition());
		FxInt32 selPage = _notebook->GetSelection();
		if( -1 != selPage )
		{
			FxStudioOptions::SetSelectedTab(_notebook->GetPageText(selPage));
		}
		else
		{
			FxStudioOptions::SetSelectedTab(wxT(""));
		}
	}
}

} // namespace Face

} // namespace OC3Ent
