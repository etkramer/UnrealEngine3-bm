//------------------------------------------------------------------------------
// A free floating window frame.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFreeFloatingFrame_H__
#define FxFreeFloatingFrame_H__

#include "FxPlatform.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/frame.h"
#endif

namespace OC3Ent
{

namespace Face
{

// Forward declare the tear-off window class.
class FxTearoffWindow;

// A free-floating frame.
class FxFreeFloatingFrame : public wxFrame
{
public:
	typedef wxFrame Super;
	// Constructor.
	FxFreeFloatingFrame( FxTearoffWindow* tearoffWindow, wxWindow* parent, 
						 wxWindowID id, const wxString& title, 
						 const wxPoint& pos = wxDefaultPosition, 
						 const wxSize& size = wxDefaultSize, 
						 long style = wxDEFAULT_FRAME_STYLE, 
						 const wxString& name = _("frame") );
	// Destructor.
	virtual ~FxFreeFloatingFrame();

	// Caches the state of the free-floater into the contained tear-off window.
	// Position, size, etc.
	void CacheState( void );

	// Returns FxTrue of the free-floater is currently always on top.
	FxBool GetAlwaysOnTop( void ) const;
	// Sets the free-floater always on top state.
	void SetAlwaysOnTop( FxBool alwaysOnTop );

protected:
	// The tear-off window this free-floater belongs to.
	FxTearoffWindow* _tearoffWindow;
	// FxTrue if the free-floater is currently in fullscreen mode.
	FxBool _isFullscreen;
	// FxTrue if the free-floating container is always on top
	FxBool _isAlwaysOnTop;

	// Close handlers.
	void OnClose( wxCloseEvent& event );
	void OnDock( wxCommandEvent& event );
	// New / Load / Save handlers.
	void OnNewActor( wxCommandEvent& event );
	void OnLoadActor( wxCommandEvent& event );
	void OnSaveActor( wxCommandEvent& event );
	// Undo / Redo handlers.
	void OnUndo( wxCommandEvent& event );
	void OnRedo( wxCommandEvent& event );
	// Size and move handlers.
	void OnSize( wxSizeEvent& event );
	void OnMove( wxMoveEvent& event );
	// Makes the main window fullscreen.
	void OnFullscreen( wxCommandEvent& event );
	// Makes the window always on top.
	void OnAlwaysOnTop( wxCommandEvent& event );

	DECLARE_EVENT_TABLE()
};

} // namespace Face

} // namespace OC3Ent

#endif
