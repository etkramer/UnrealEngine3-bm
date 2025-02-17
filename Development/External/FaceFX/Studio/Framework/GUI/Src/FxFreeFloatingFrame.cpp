//------------------------------------------------------------------------------
// A free floating window frame.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxFreeFloatingFrame.h"
#include "FxStudioMainWin.h"

#include "FxTearoffWindow.h"
#include "FxSplitterWindow.h"

namespace OC3Ent
{

namespace Face
{

BEGIN_EVENT_TABLE(FxFreeFloatingFrame, wxFrame)
	EVT_CLOSE(FxFreeFloatingFrame::OnClose)
	EVT_SIZE(FxFreeFloatingFrame::OnSize)
	EVT_MOVE(FxFreeFloatingFrame::OnMove)
	EVT_MENU(MenuID_FreeFloatingFrameFullscreen, FxFreeFloatingFrame::OnFullscreen)
	EVT_MENU(MenuID_FreeFloatingFrameDock, FxFreeFloatingFrame::OnDock)
	EVT_MENU(MenuID_FreeFloatingFrameNewActor, FxFreeFloatingFrame::OnNewActor)
	EVT_MENU(MenuID_FreeFloatingFrameLoadActor, FxFreeFloatingFrame::OnLoadActor)
	EVT_MENU(MenuID_FreeFloatingFrameSaveActor, FxFreeFloatingFrame::OnSaveActor)
	EVT_MENU(MenuID_FreeFloatingFrameUndo, FxFreeFloatingFrame::OnUndo)
	EVT_MENU(MenuID_FreeFloatingFrameRedo, FxFreeFloatingFrame::OnRedo)
	EVT_MENU(MenuID_FreeFloatingFrameAlwaysOnTop, FxFreeFloatingFrame::OnAlwaysOnTop)
END_EVENT_TABLE()

FxFreeFloatingFrame::
FxFreeFloatingFrame( FxTearoffWindow* tearoffWindow, wxWindow* parent, 
					 wxWindowID id, const wxString& title, 
					 const wxPoint& pos, const wxSize& size, long style,
					 const wxString& name )
					 : wxFrame(parent, id, title, pos, size, style, name)
					 , _tearoffWindow(tearoffWindow)
				     , _isFullscreen(FxFalse)
#ifdef __UNREAL__
					 , _isAlwaysOnTop(FxFalse)
#else
					 , _isAlwaysOnTop(FxTrue)
#endif
{
#ifndef __APPLE__
	wxIcon impStudioIcon(wxICON(APP_ICON));
	SetIcon(impStudioIcon);
#endif

	// Set up the accelerator table.
	static const int numEntries = 11;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set(wxACCEL_NORMAL, WXK_F10, MenuID_FreeFloatingFrameDock);
	entries[1].Set(wxACCEL_ALT, WXK_RETURN, MenuID_FreeFloatingFrameFullscreen);
	entries[2].Set(wxACCEL_NORMAL, WXK_F11, MenuID_FreeFloatingFrameFullscreen);
	entries[3].Set(wxACCEL_SHIFT|wxACCEL_ALT, WXK_RETURN, MenuID_FreeFloatingFrameFullscreen);
	entries[4].Set(wxACCEL_CTRL, (FxInt32)'O', MenuID_FreeFloatingFrameLoadActor);
	entries[5].Set(wxACCEL_CTRL, (FxInt32)'N', MenuID_FreeFloatingFrameNewActor);
	entries[6].Set(wxACCEL_CTRL, (FxInt32)'S', MenuID_FreeFloatingFrameSaveActor);
	entries[7].Set(wxACCEL_CTRL, (FxInt32)'Z', MenuID_FreeFloatingFrameUndo);
	entries[8].Set(wxACCEL_CTRL, (FxInt32)'Y', MenuID_FreeFloatingFrameRedo);
	entries[9].Set(wxACCEL_CTRL|wxACCEL_SHIFT, (FxInt32)'Z', MenuID_FreeFloatingFrameRedo);
#if defined(WIN32) && defined(FX_DEBUG)
	// On Windows NT (and thus 2000 and XP), the F12 key is mapped to a hard
	// user debug break and there is no way around it.  The Microsoft suggested
	// "solution" to this problem is to temporarily remap the application's F12
	// key functionality to another key when running in debug mode.
	entries[10].Set(wxACCEL_NORMAL, WXK_F9, MenuID_FreeFloatingFrameAlwaysOnTop);
#else
	entries[10].Set(wxACCEL_NORMAL, WXK_F12, MenuID_FreeFloatingFrameAlwaysOnTop);
#endif
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator(numEntries, entries);
	SetAcceleratorTable(accelerator);
}

FxFreeFloatingFrame::~FxFreeFloatingFrame()
{
}

void FxFreeFloatingFrame::CacheState( void )
{
	if( _tearoffWindow )
	{
		if( !IsIconized() && !IsMaximized() )
		{
			_tearoffWindow->SetPos(GetPosition());
			_tearoffWindow->SetSize(GetSize());
		}
		_tearoffWindow->SetMinimized(IsIconized());
		_tearoffWindow->SetMaximized(IsMaximized());
		_tearoffWindow->SetAlwaysOnTop(_isAlwaysOnTop);
	}
}

FxBool FxFreeFloatingFrame::GetAlwaysOnTop( void ) const
{
	return _isAlwaysOnTop;
}

void FxFreeFloatingFrame::SetAlwaysOnTop( FxBool alwaysOnTop )
{
	_isAlwaysOnTop = alwaysOnTop;
	if( _isAlwaysOnTop )
	{
		SetWindowStyleFlag(GetWindowStyleFlag() | wxSTAY_ON_TOP);
	}
	else
	{
		SetWindowStyleFlag(GetWindowStyleFlag() & ~wxSTAY_ON_TOP);
	}
	Refresh();
}

void FxFreeFloatingFrame::OnClose( wxCloseEvent& event )
{
	Show(FALSE);
	CacheState();
	if( _tearoffWindow )
	{
		_tearoffWindow->_dock();
		_tearoffWindow->_freeFloatingFrame = NULL;
	}
	Destroy();
	event.Skip();
}

void FxFreeFloatingFrame::OnDock( wxCommandEvent& FxUnused(event) )
{
	wxCloseEvent temp;
	OnClose(temp);
}

void FxFreeFloatingFrame::OnNewActor( wxCommandEvent& FxUnused(event) )
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(GetParent());
	if( pMainWin )
	{
		wxCommandEvent temp;
		pMainWin->OnNewActor(temp);
	}
}

void FxFreeFloatingFrame::OnLoadActor( wxCommandEvent& FxUnused(event) )
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(GetParent());
	if( pMainWin )
	{
		wxCommandEvent temp;
		pMainWin->OnLoadActor(temp);
	}
}

void FxFreeFloatingFrame::OnSaveActor( wxCommandEvent& FxUnused(event) )
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(GetParent());
	if( pMainWin )
	{
		wxCommandEvent temp;
		pMainWin->OnSaveActorProxy(temp);
	}
}

void FxFreeFloatingFrame::OnUndo( wxCommandEvent& FxUnused(event) )
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(GetParent());
	if( pMainWin )
	{
		wxCommandEvent temp;
		pMainWin->OnUndoProxy(temp);
	}
}

void FxFreeFloatingFrame::OnRedo( wxCommandEvent& FxUnused(event) )
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(GetParent());
	if( pMainWin )
	{
		wxCommandEvent temp;
		pMainWin->OnRedoProxy(temp);
	}
}

void FxFreeFloatingFrame::OnSize( wxSizeEvent& event )
{
	CacheState();
	Layout();
	Refresh();
	Update();
	event.Skip();
}

void FxFreeFloatingFrame::OnMove( wxMoveEvent& event )
{
	CacheState();
	Layout();
	Refresh();
	Update();
	event.Skip();
}

void FxFreeFloatingFrame::OnFullscreen( wxCommandEvent& FxUnused(event) )
{
	_isFullscreen = !_isFullscreen;
	if( _isFullscreen )
	{
		ShowFullScreen(TRUE, wxFULLSCREEN_NOSTATUSBAR|wxFULLSCREEN_NOBORDER|wxFULLSCREEN_NOCAPTION);
	}
	else
	{
		ShowFullScreen(FALSE);
	}
}

void FxFreeFloatingFrame::OnAlwaysOnTop( wxCommandEvent& FxUnused(event) )
{
	_isAlwaysOnTop = !_isAlwaysOnTop;
	if( _isAlwaysOnTop )
	{
		SetWindowStyleFlag(GetWindowStyleFlag() | wxSTAY_ON_TOP);
	}
	else
	{
		SetWindowStyleFlag(GetWindowStyleFlag() & ~wxSTAY_ON_TOP);
	}
	Refresh();
}

} // namespace Face

} // namespace OC3Ent
