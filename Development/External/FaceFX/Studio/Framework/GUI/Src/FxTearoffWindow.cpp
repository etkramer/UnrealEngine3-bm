//------------------------------------------------------------------------------
// Defines a tear-off window that contains a top-level window and
// that can be re-docked with a notebook control.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxTearoffWindow.h"
#include "FxSplitterWindow.h"
#include "FxStudioApp.h"
#include "FxStudioMainWin.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/notebook.h"
#endif

namespace OC3Ent
{

namespace Face
{

FxBool FxTearoffWindow::_isMainWindowBeingDestroyed = FxFalse;
FxArray<FxTearoffWindow*> FxTearoffWindow::_managedTearoffWindows;

FxTearoffWindow::
FxTearoffWindow( wxNotebook* notebook, wxWindow* child, 
			     const wxString& title )
	             : _notebook(notebook)
	             , _splitterWindow(NULL)
	             , _childWindow(child)
	             , _title(title)
	             , _isTorn(FxFalse)
	             , _freeFloatingFrame(NULL)
	             , _pos(wxDefaultPosition)
	             , _size(wxDefaultSize)
	             , _isMaximized(FxFalse)
	             , _isMinimized(FxFalse)
#ifdef __UNREAL__
	             , _isAlwaysOnTop(FxFalse)
#else
				 , _isAlwaysOnTop(FxTrue)
#endif
{
	FxTearoffWindow::AddManagedTearoffWindow(this);
}

FxTearoffWindow::
FxTearoffWindow( FxSplitterWindow* splitterWindow, wxWindow* child,
  		         const wxString& title )
				 : _notebook(NULL)
				 , _splitterWindow(splitterWindow)
				 , _childWindow(child)
				 , _title(title)
				 , _isTorn(FxFalse)
				 , _freeFloatingFrame(NULL)
				 , _pos(wxDefaultPosition)
				 , _size(wxDefaultSize)
				 , _isMaximized(FxFalse)
				 , _isMinimized(FxFalse)
#ifdef __UNREAL__
				 , _isAlwaysOnTop(FxFalse)
#else
				 , _isAlwaysOnTop(FxTrue)
#endif

{
	FxTearoffWindow::AddManagedTearoffWindow(this);
}

FxTearoffWindow::~FxTearoffWindow()
{
	FxTearoffWindow::RemoveManagedTearoffWindow(this);
}

void FxTearoffWindow::AddManagedTearoffWindow( FxTearoffWindow* pTearoffWindow )
{
	if( FxInvalidIndex == _managedTearoffWindows.Find(pTearoffWindow) )
	{
		_managedTearoffWindows.PushBack(pTearoffWindow);
	}
	else
	{
		FxAssert(!"FxTearoffWindow::AddManagedTearoffWindow - pTearoffWindow already in managed list!");
	}
}

void FxTearoffWindow::RemoveManagedTearoffWindow( FxTearoffWindow* pTearoffWindow )
{
	FxSize index = _managedTearoffWindows.Find(pTearoffWindow);
	if( FxInvalidIndex != index )
	{
		_managedTearoffWindows.Remove(index);
	}
	else
	{
		FxAssert(!"FxTearoffWindow::RemoveTearoffWindow - pTearoffWindow not in managed list!");
	}
}

FxSize FxTearoffWindow::GetNumManagedTearoffWindows( void )
{
	return _managedTearoffWindows.Length();
}

FxTearoffWindow* FxTearoffWindow::GetManagedTearoffWindow( FxSize index )
{
	return _managedTearoffWindows[index];
}

void FxTearoffWindow::CacheAllCurrentlyTornTearoffWindowStates( void )
{
	for( FxSize i = 0; i < _managedTearoffWindows.Length(); ++i )
	{
		if( _managedTearoffWindows[i] && _managedTearoffWindows[i]->_isTorn &&
			_managedTearoffWindows[i]->_freeFloatingFrame )
		{
			_managedTearoffWindows[i]->_freeFloatingFrame->CacheState();
		}
	}
}

void FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop( void )
{
	for( FxSize i = 0; i < _managedTearoffWindows.Length(); ++i )
	{
		if( _managedTearoffWindows[i] && _managedTearoffWindows[i]->_isTorn && 
			_managedTearoffWindows[i]->_freeFloatingFrame )
		{
			// Unconditionally set always on top of the free floating frame to FxFalse.
			// The current FxTearoffWindow's _isAlwaysOnTop serves as the cache for the
			// free floating frame's current value of _isAlwaysOnTop.
			_managedTearoffWindows[i]->_isAlwaysOnTop = _managedTearoffWindows[i]->_freeFloatingFrame->GetAlwaysOnTop();
			_managedTearoffWindows[i]->_freeFloatingFrame->SetAlwaysOnTop(FxFalse);
		}
	}
}

void FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop( void )
{
	for( FxSize i = 0; i < _managedTearoffWindows.Length(); ++i )
	{
		if( _managedTearoffWindows[i] && _managedTearoffWindows[i]->_isTorn && 
			_managedTearoffWindows[i]->_freeFloatingFrame )
		{
			// Set always on top of the free floating frame to whatever it was cached
			// as in the FxTearoffWindow.
			_managedTearoffWindows[i]->_freeFloatingFrame->SetAlwaysOnTop(_managedTearoffWindows[i]->_isAlwaysOnTop);
		}
	}
}

void FxTearoffWindow::DockAllCurrentlyTornTearoffWindows( void )
{
	for( FxSize i = 0; i < _managedTearoffWindows.Length(); ++i )
	{
		if( _managedTearoffWindows[i] )
		{
			_managedTearoffWindows[i]->Dock();
		}
	}
}

void FxTearoffWindow::Tear( void )
{
	if( _childWindow && !_isTorn )
	{
		if( _notebook )
		{
			for( FxSize i = 0; i < _notebook->GetPageCount(); ++i )
			{
				if( _notebook->GetPage(i) == _childWindow )
				{
					_notebook->SetSelection(i);
					_notebook->RemovePage(i);
				}
			}
		}
		_freeFloatingFrame = new FxFreeFloatingFrame(this, FxStudioApp::GetMainWindow(), -1, _title, _pos, _size);
		_childWindow->Reparent(_freeFloatingFrame);
		_freeFloatingFrame->Show();
		if( _isMinimized )
		{
			_freeFloatingFrame->Iconize();
		}
		if( _isMaximized )
		{
			_freeFloatingFrame->Maximize();
		}
		if( _isAlwaysOnTop )
		{
			_freeFloatingFrame->SetAlwaysOnTop(FxTrue);
		}
		_childWindow->Refresh(FALSE);
		_isTorn = FxTrue;
	}
}

void FxTearoffWindow::Dock( void )
{
	if( _isTorn )
	{
		_dock();
		if( _freeFloatingFrame )
		{
			_freeFloatingFrame->Show(FALSE);
			_freeFloatingFrame->Destroy();
		}
	}
}

FxBool FxTearoffWindow::IsTorn( void ) const
{
	return _isTorn;
}

wxString FxTearoffWindow::GetTitle( void ) const
{
	return _title;
}

wxPoint FxTearoffWindow::GetPos( void ) const
{
	return _pos;
}

void FxTearoffWindow::SetPos( const wxPoint& pos )
{
	_pos = pos;
}

	wxSize FxTearoffWindow::GetSize( void ) const
{
	return _size;
}

void FxTearoffWindow::SetSize( const wxSize& size )
{
	_size = size;
}

FxBool FxTearoffWindow::GetMaximized( void ) const
{
	return _isMaximized;
}

void FxTearoffWindow::SetMaximized( FxBool maximized )
{
	_isMaximized = maximized;
	if( _isMinimized && _isMaximized )
	{
		_isMinimized = FxFalse;
	}
	if( _isMaximized && _freeFloatingFrame )
	{
		_freeFloatingFrame->Maximize();
	}
}

FxBool FxTearoffWindow::GetAlwaysOnTop( void ) const
{
	return _isAlwaysOnTop;
}

void FxTearoffWindow::SetAlwaysOnTop( FxBool alwaysOnTop )
{
	_isAlwaysOnTop = alwaysOnTop;
	if( _freeFloatingFrame )
	{
		_freeFloatingFrame->SetAlwaysOnTop(_isAlwaysOnTop);
	}
}

FxBool FxTearoffWindow::GetMinimized( void ) const
{
	return _isMinimized;
}

void FxTearoffWindow::SetMinimized( FxBool minimized )
{
	_isMinimized = minimized;
	if( _isMaximized && _isMinimized )
	{
		_isMaximized = FxFalse;
	}
	if( _isMinimized && _freeFloatingFrame )
	{
		_freeFloatingFrame->Iconize();
	}
}

wxWindow* FxTearoffWindow::GetWindow( void )
{
	return _childWindow;
}

FxFreeFloatingFrame* FxTearoffWindow::GetFreeFloatingFrame( void )
{
	return _freeFloatingFrame;
}

void FxTearoffWindow::_dock( void )
{
	if( _childWindow )
	{
		if( _notebook )
		{
			_childWindow->Reparent(_notebook);
			_notebook->AddPage(_childWindow, _title, TRUE);
		}
		if( _splitterWindow )
		{
			_childWindow->Reparent(_splitterWindow);
			if( _splitterWindow->GetOriginalWindow1() && _splitterWindow->GetOriginalWindow2() )
			{
				FxInt32 splitMode = _splitterWindow->GetSplitMode();
				if( wxSPLIT_VERTICAL == splitMode )
				{
					if( _splitterWindow->IsSplitRightOrBottom() )
					{
						if( _splitterWindow->IsFirstWindowSplitOff() )
						{
							_splitterWindow->SplitVertically(_childWindow, 
								_splitterWindow->GetOriginalWindow2(), _splitterWindow->GetOldSashPosition());
						}
						else
						{
							_splitterWindow->SplitVertically(_splitterWindow->GetOriginalWindow1(), 
								_childWindow, _splitterWindow->GetOldSashPosition());
						}
					}
					else
					{
						if( _splitterWindow->IsSecondWindowSplitOff() )
						{
							_splitterWindow->SplitVertically(_splitterWindow->GetOriginalWindow1(), 
								_childWindow, _splitterWindow->GetOldSashPosition());
						}
						else
						{
							_splitterWindow->SplitVertically(_childWindow, 
								_splitterWindow->GetOriginalWindow2(), _splitterWindow->GetOldSashPosition());
						}
					}
				}
				else if( wxSPLIT_HORIZONTAL == splitMode )
				{
					if( _splitterWindow->IsSplitRightOrBottom() )
					{
						if( _splitterWindow->IsFirstWindowSplitOff() )
						{
							_splitterWindow->SplitHorizontally(_childWindow, 
								_splitterWindow->GetOriginalWindow2(), _splitterWindow->GetOldSashPosition());
						}
						else
						{
							_splitterWindow->SplitHorizontally(_splitterWindow->GetOriginalWindow1(), 
								_childWindow, _splitterWindow->GetOldSashPosition());
						}
					}
					else
					{
						if( _splitterWindow->IsSecondWindowSplitOff() )
						{
							_splitterWindow->SplitHorizontally(_splitterWindow->GetOriginalWindow1(), 
								_childWindow, _splitterWindow->GetOldSashPosition());
						}
						else
						{
							_splitterWindow->SplitHorizontally(_childWindow, 
								_splitterWindow->GetOriginalWindow2(), _splitterWindow->GetOldSashPosition());
						}
					}
				}
			}
		}
		_isTorn = FxFalse;
	}
}

} // namespace Face

} // namespace OC3Ent
