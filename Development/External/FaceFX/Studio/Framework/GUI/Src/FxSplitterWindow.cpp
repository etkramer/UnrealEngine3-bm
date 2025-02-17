//------------------------------------------------------------------------------
// A splitter window that is capable of being unsplit and resplit in the context
// of FaceFX Studio.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxSplitterWindow.h"
#include "FxTearoffWindow.h"

namespace OC3Ent
{

namespace Face
{

FxArray<FxSplitterWindow*> FxSplitterWindow::_managedSplitterWindows;

BEGIN_EVENT_TABLE(FxSplitterWindow, wxSplitterWindow)
END_EVENT_TABLE()

FxSplitterWindow::
FxSplitterWindow( wxWindow* parent, wxWindowID id, 
				  FxBool splitRightOrBottom,
			      const wxPoint& pos, const wxSize& size, 
				  long style, const wxString& name )
				  : wxSplitterWindow(parent, id, pos, size, style, name)
				  , _firstTearoffWindow(NULL)
				  , _secondTearoffWindow(NULL)
				  , _splitRightOrBottom(splitRightOrBottom)
				  , _oldSashPosition(0)

{
	FxSplitterWindow::AddManagedSplitterWindow(this);
}

FxSplitterWindow::~FxSplitterWindow()
{
	if( _firstTearoffWindow )
	{
		delete _firstTearoffWindow;
		_firstTearoffWindow = NULL;
	}
	if( _secondTearoffWindow )
	{
		delete _secondTearoffWindow;
		_secondTearoffWindow = NULL;
	}
	FxSplitterWindow::RemoveManagedSplitterWindow(this);
}

void FxSplitterWindow::AddManagedSplitterWindow( FxSplitterWindow* pSplitterWindow )
{
	if( FxInvalidIndex == _managedSplitterWindows.Find(pSplitterWindow) )
	{
		_managedSplitterWindows.PushBack(pSplitterWindow);
	}
	else
	{
		FxAssert(!"FxSplitterWindow::AddManagedSplitterWindow - pSplitterWindow already in managed list!");
	}
}

void FxSplitterWindow::RemoveManagedSplitterWindow( FxSplitterWindow* pSplitterWindow )
{
	FxSize index = _managedSplitterWindows.Find(pSplitterWindow);
	if( FxInvalidIndex != index )
	{
		_managedSplitterWindows.Remove(index);
	}
	else
	{
		FxAssert(!"FxSplitterWindow::RemoveManagedSplitterWindow - pSplitterWindow not in managed list!");
	}
}

FxSize FxSplitterWindow::GetNumManagedSplitterWindows( void )
{
	return _managedSplitterWindows.Length();
}

FxSplitterWindow* FxSplitterWindow::GetManagedSplitterWindow( FxSize index )
{
	return _managedSplitterWindows[index];
}

void FxSplitterWindow::
Initialize( const wxString& firstWindowName, const wxString& secondWindowName )
{
	_firstTearoffWindow = new FxTearoffWindow(this, GetWindow1(), firstWindowName);
	_secondTearoffWindow = new FxTearoffWindow(this, GetWindow2(), secondWindowName);
}

FxBool FxSplitterWindow::IsSplitRightOrBottom( void ) const
{
	return _splitRightOrBottom;
}

FxInt32 FxSplitterWindow::GetOldSashPosition( void ) const
{
	return _oldSashPosition;
}

wxWindow* FxSplitterWindow::GetOriginalWindow1( void )
{
	if( _firstTearoffWindow )
	{
		return _firstTearoffWindow->GetWindow();
	}
	return NULL;
}

wxWindow* FxSplitterWindow::GetOriginalWindow2( void )
{
	if( _secondTearoffWindow )
	{
		return _secondTearoffWindow->GetWindow();
	}
	return NULL;
}

FxTearoffWindow* FxSplitterWindow::GetOriginalTearoffWindow1( void )
{
	return _firstTearoffWindow;
}

FxTearoffWindow* FxSplitterWindow::GetOriginalTearoffWindow2( void )
{
	return _secondTearoffWindow;
}

FxBool FxSplitterWindow::IsFirstWindowSplitOff( void ) const
{
	if( _firstTearoffWindow )
	{
		return _firstTearoffWindow->IsTorn();
	}
	return FxFalse;
}

FxBool FxSplitterWindow::IsSecondWindowSplitOff( void ) const
{
	if( _secondTearoffWindow )
	{
		return _secondTearoffWindow->IsTorn();
	}
	return FxFalse;
}

void FxSplitterWindow::
OnDoubleClickSash( FxInt32 FxUnused(x), FxInt32 FxUnused(y) )
{
	if( _splitRightOrBottom )
	{
		Unsplit(GetWindow2());
	}
	else
	{
		Unsplit(GetWindow1());
	}
}

void FxSplitterWindow::OnUnsplit( wxWindow* removed )
{
	_oldSashPosition = GetSashPosition();
	if( _firstTearoffWindow && _secondTearoffWindow )
	{
		if( _firstTearoffWindow->GetWindow() == removed )
		{
			if( !_firstTearoffWindow->IsTorn() )
			{
				_firstTearoffWindow->Tear();
			}
		}
		else if( _secondTearoffWindow->GetWindow() == removed  )
		{
			if( !_secondTearoffWindow->IsTorn() )
			{
				_secondTearoffWindow->Tear();
			}
		}
	}
}

} // namespace Face

} // namespace OC3Ent
