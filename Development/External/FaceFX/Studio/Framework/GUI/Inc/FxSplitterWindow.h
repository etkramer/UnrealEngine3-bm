//------------------------------------------------------------------------------
// A splitter window that is capable of being unsplit and resplit in the context
// of FaceFX Studio.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSplitterWindow_H__
#define FxSplitterWindow_H__

#include "FxPlatform.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/splitter.h"
#endif

namespace OC3Ent
{

namespace Face
{

// Forward declarations.
class FxTearoffWindow;

// A splitter window.
class FxSplitterWindow : public wxSplitterWindow
{
public:
	typedef wxSplitterWindow Super;
	// Constructor.
	FxSplitterWindow( wxWindow* parent, wxWindowID id, 
					  FxBool splitRightOrBottom = FxTrue,
					  const wxPoint& pos = wxDefaultPosition, 
					  const wxSize& size = wxDefaultSize, 
					  long style = wxSP_NO_XP_THEME|wxSP_3D|wxSP_PERMIT_UNSPLIT, 
					  const wxString& name = _("splitterWindow") );
	// Destructor.
	virtual ~FxSplitterWindow();

	// Add a managed splitter window to the system.
	static void AddManagedSplitterWindow( FxSplitterWindow* pSplitterWindow );
	// Remove a managed splitter window from the system.
	static void RemoveManagedSplitterWindow( FxSplitterWindow* pSplitterWindow );
	// Returns the number of managed splitter windows.
	static FxSize GetNumManagedSplitterWindows( void );
	// Returns the managed splitter window at index.
	static FxSplitterWindow* GetManagedSplitterWindow( FxSize index );
	
	// Initializes the splitter window.  Always call this after the splitter
	// window has been created and both windows have been added and the splitter
	// window has been split.
	void Initialize( const wxString& firstWindowName, 
		             const wxString& secondWindowName );

	// Returns FxTrue if the right or bottom window is split off when the
	// splitter window is unsplit or FxFalse if the left or top window is split
	// off.
	FxBool IsSplitRightOrBottom( void ) const;

	// Returns the position of the sash before the splitter window was unsplit.
	FxInt32 GetOldSashPosition( void ) const;

	// Returns the original first window (pre-split).
	wxWindow* GetOriginalWindow1( void );
	// Returns the original second window (pre-split).
	wxWindow* GetOriginalWindow2( void );

	// Returns the original tear-off window (pre-split).
	FxTearoffWindow* GetOriginalTearoffWindow1( void );
	// Returns the original tear-off window (pre-split).
	FxTearoffWindow* GetOriginalTearoffWindow2( void );

	// Returns FxTrue if the first window (left or top) is currently split off.
	FxBool IsFirstWindowSplitOff( void ) const;
	// Returns FxTrue if the second window (right or bottom) is currently split 
	// off.
	FxBool IsSecondWindowSplitOff( void ) const;

	// Called when the sash is double clicked.
	virtual void OnDoubleClickSash( FxInt32 x, FxInt32 y );
	// Called when the splitter window is unsplit.
	virtual void OnUnsplit( wxWindow* removed );

protected:
	// The tear-off window associated with the first window in the splitter 
	// window.
	FxTearoffWindow* _firstTearoffWindow;
	// The tear-off window associated with the second window in the splitter 
	// window.
	FxTearoffWindow* _secondTearoffWindow;
	// This is FxTrue if the right or bottom window is split off when the 
	// splitter window is unsplit or FxFalse if the left or top window is split 
	// off.
	FxBool _splitRightOrBottom;
	// The old sash position.
	FxInt32 _oldSashPosition;

	// An array of all of the splitter windows currently in the system.
	static FxArray<FxSplitterWindow*> _managedSplitterWindows;
    
    DECLARE_EVENT_TABLE()
};


} // namespace Face

} // namespace OC3Ent

#endif
