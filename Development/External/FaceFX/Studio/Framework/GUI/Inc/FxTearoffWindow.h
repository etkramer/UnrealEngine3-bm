//------------------------------------------------------------------------------
// Defines a tear-off window that contains a top-level window and
// that can be re-docked with a notebook control or FxSplitterWindow.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxTearoffWindow_H__
#define FxTearoffWindow_H__

#include "FxPlatform.h"
#include "FxArray.h"

#include "FxFreeFloatingFrame.h"

class wxNotebook;

namespace OC3Ent
{

namespace Face
{

// Forward declarations.
class FxSplitterWindow;

// A tear-off window.
class FxTearoffWindow
{
public:
	// Constructor.
	FxTearoffWindow( wxNotebook* notebook, wxWindow* child, 
		             const wxString& title );
	// Constructor.
	FxTearoffWindow( FxSplitterWindow* splitterWindow, wxWindow* child,
		             const wxString& title );
	// Destructor.
	virtual ~FxTearoffWindow();

	// Add a managed tear-off window to the system.
	static void AddManagedTearoffWindow( FxTearoffWindow* pTearoffWindow );
	// Remove a managed tear-off window from the system.
	static void RemoveManagedTearoffWindow( FxTearoffWindow* pTearoffWindow );
	// Returns the number of managed tear-off windows.
	static FxSize GetNumManagedTearoffWindows( void );
	// Returns the managed tear-off window at index.
	static FxTearoffWindow* GetManagedTearoffWindow( FxSize index );
	// Caches the current state off all currently "free-floaing" (torn) tear-off windows.
	static void CacheAllCurrentlyTornTearoffWindowStates( void );
	// Cache the current value of the always on top member for all tear-off
	// windows and and unconditionally set it to FxFalse.
	static void CacheAndSetAllTearoffWindowsToNotAlwaysOnTop( void );
	// Restore the previous current value of the always on top member for all
	// tear-off windows.
	static void RestoreAllTearoffWindowsAlwaysOnTop( void );
	// Forces any currently torn-off windows to dock themselves immediately.
	static void DockAllCurrentlyTornTearoffWindows( void );

	// Tear the window off and make it free-floating.
	void Tear( void );
	// Dock the window and destroy the free-floating container.
	void Dock( void );
	
	// Returns FxTrue if the window is currently "torn" and FxFalse if not.
	FxBool IsTorn( void ) const;

	// Returns the title of the tear-off window.
	wxString GetTitle( void ) const;

	// Returns the current position of the free-floating container.
	wxPoint GetPos( void ) const;
	// Sets the current position of the free-floating container.
	void SetPos( const wxPoint& pos );

	// Returns the current size of the free-floating container.
	wxSize GetSize( void ) const;
	// Sets the current size of the free-floating container.
	void SetSize( const wxSize& size );

	// Returns the current maximized state of the free-floating container.
	FxBool GetMaximized( void ) const;
	// Sets the current maximized state of the free-floating container.
	void SetMaximized( FxBool maximized );

	// Returns the current minimized state of the free-floating container.
	FxBool GetMinimized( void ) const;
	// Sets the current minimized state of the free-floating container.
	void SetMinimized( FxBool minimized );

	// Returns the current always-on-top state of the free-floating container.
	FxBool GetAlwaysOnTop( void ) const;
	// Sets the current always-on-top state of the free-floating container.
	void SetAlwaysOnTop( FxBool alwaysOnTop );

	// Returns the window contained in the tear-off window.
	wxWindow* GetWindow( void );
	// Returns the free floating frame.
	FxFreeFloatingFrame* GetFreeFloatingFrame( void );

	// Give FxFreeFloatingFrame friend access so it can call _dock.
	friend class FxFreeFloatingFrame;

protected:
	// The notebook that contains the tear-off window.
	wxNotebook* _notebook;
	// The splitter window that contains the tear-off window.
	FxSplitterWindow* _splitterWindow;
	// The top-level window contained in the tear-off window.
	wxWindow* _childWindow;
	// The title of the tear-off window.
	wxString _title;
	// Whether or not the tear-off is actually "torn."
	FxBool _isTorn;
	// The free-floating container window.
	FxFreeFloatingFrame* _freeFloatingFrame;
	// The position of the free-floating container window.
	wxPoint _pos;
	// The size of the free-floating container window.
	wxSize _size;
	// FxTrue if the free-floating container window is maximized.
	FxBool _isMaximized;
	// FxTrue if the free-floating container window is minimized.
	FxBool _isMinimized;
	// FxTrue if the free-floating container windows is always-on-top.
	FxBool _isAlwaysOnTop;

	// FxTrue if the main window is currently being destroyed.
	static FxBool _isMainWindowBeingDestroyed;
	// An array of all of the tear-off windows currently in the system.
	static FxArray<FxTearoffWindow*> _managedTearoffWindows;

	// Dock the window.
	// This function will not destroy the free-floating window as it is assumed
	// the caller will destroy it.  This is called in Dock() as well as the
	// OnClose() handler of FxFreeFloatingFrame.
	void _dock( void );
};

} // namespace Face

} // namespace OC3Ent

#endif
