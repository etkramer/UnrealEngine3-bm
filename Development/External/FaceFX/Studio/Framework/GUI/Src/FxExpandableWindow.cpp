//------------------------------------------------------------------------------
// This is a window containing controls that can be expanded and collapsed.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxExpandableWindow.h"
#include "FxString.h"

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Expandable window.
//------------------------------------------------------------------------------
WX_IMPLEMENT_CLASS(FxExpandableWindow, wxWindow)

BEGIN_EVENT_TABLE(FxExpandableWindow, wxWindow)
	EVT_LEFT_DOWN(FxExpandableWindow::OnLeftClick)
	EVT_RIGHT_DOWN(FxExpandableWindow::OnRightClick)
	EVT_SIZE(FxExpandableWindow::OnSize)
END_EVENT_TABLE()

FxExpandableWindow::FxExpandableWindow( wxWindow* parent )
	: wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
	, _isExpanded(FxFalse)
	, _resizeChildren(FxTrue)
{
	_container = static_cast<FxExpandableWindowContainer*>(parent);
}

FxExpandableWindow::~FxExpandableWindow()
{
}

FxBool FxExpandableWindow::operator<( const FxExpandableWindow& other ) const
{
	return (FxString(GetName().mb_str(wxConvLibc)) < 
		    FxString(other.GetName().mb_str(wxConvLibc)));
}

void FxExpandableWindow::Expand( FxBool refresh )
{
	if( !_isExpanded )
	{
		Freeze();
		_isExpanded = FxTrue;
		CreateChildren();
		SetClientSize(_container->GetClientRect().width, GetHeight());
		if( FxTrue == refresh )
		{
			_container->PositionExapandableWindows();
		}
		Thaw();
	}
}

void FxExpandableWindow::Collapse( FxBool refresh )
{
	if( _isExpanded )
	{
		Freeze();
		_isExpanded = FxFalse;
		DestroyChildren();
		SetClientSize(_container->GetClientRect().width, GetHeight());
		if( FxTrue == refresh )
		{
			_container->PositionExapandableWindows();
		}
		Thaw();
	}
}

void FxExpandableWindow::LeftClickHandler( void )
{
	if( _isExpanded )
	{
		Collapse();
	}
	else
	{
		Expand();
	}
}

void FxExpandableWindow::ControlLeftClickHandler( void )
{
}

void FxExpandableWindow::ControlShiftLeftClickHandler( void )
{
	if( FxFalse == _container->AllAreExpanded() )
	{
		_container->ExpandAll();
	}
	else
	{
		_container->CollapseAll();
	}
}

void FxExpandableWindow::RightClickHandler( void )
{
}

FxBool FxExpandableWindow::IsExpanded( void ) const
{
	return _isExpanded;
}

void FxExpandableWindow::Hide( void )
{
	Show(false);
	_container->PositionExapandableWindows();
}

void FxExpandableWindow::UnHide( void )
{
	Show(true);
	_container->PositionExapandableWindows();
}

FxBool FxExpandableWindow::IsHidden( void	) const
{
	return IsShown() ? FxFalse : FxTrue;
}

void FxExpandableWindow::CreateChildren( void )
{
}

void FxExpandableWindow::DestroyChildren( void )
{
	Freeze();
	SetSizer(NULL, TRUE);
	for( FxSize i = 0; i < _children.Length(); ++i )
	{
		_children[i]->Destroy();
	}
	_children.Clear();
	Thaw();
}

void FxExpandableWindow::Draw( void )
{
	wxPaintDC dc(this);
	wxRect rect = GetClientRect();
	dc.SetFont(*wxNORMAL_FONT);
	dc.SetPen(wxPen(_container->_panelBackgroundColor, 1, wxSOLID));
	dc.SetBrush(wxBrush(_container->_panelBackgroundColor,wxSOLID));
	dc.DrawRectangle(GetClientRect());
	dc.SetBrush(wxBrush(_container->_expandableWindowColor, wxSOLID));
	
    dc.SetPen(wxPen(_container->_shadowColor, 1, wxSOLID));
	dc.DrawRoundedRectangle(rect.x+1,rect.y+1, rect.width-2,rect.height-2, 5);

	wxFont font = dc.GetFont();
	font.SetWeight(wxBOLD);
	dc.SetFont(font);
	dc.SetTextForeground(_container->_highlightColor);
	dc.DrawText(GetName(), rect.x+4,rect.y+3);
	dc.SetTextForeground(_container->_textColor);
	dc.DrawText(GetName(), rect.x+5,rect.y+4);
}

FxInt32 FxExpandableWindow::GetHeight( void ) const
{
	if( _isExpanded )
	{
		if( GetSizer() )
		{
			return GetSizer()->GetMinSize().GetHeight() + kCollapsedHeight/2;
		}
		else
		{
			FxInt32 height = 0;
			for( FxSize i = 0; i < _children.Length(); ++i )
			{
				if( _children[i] )
				{
					height += _children[i]->GetRect().height;
				}
			}
			return height + 2*kCollapsedHeight;
		}
	}
	return kCollapsedHeight;
}

void FxExpandableWindow::OnLeftClick( wxMouseEvent& event )
{
	// Only allow clicks in the title region to produce expand and
	// collapse events.
	wxPoint clickPos = event.GetPosition();
	wxRect titleRegion = GetClientRect();
	// The title region is defined as the client rect with a
	// max height equal to the height of the collapsed window.
	titleRegion.height = kCollapsedHeight;
	if( titleRegion.Contains(clickPos) )
	{
		if( event.ControlDown() && event.ShiftDown() )
		{
			ControlShiftLeftClickHandler();
		}
		else if( event.ControlDown() )
		{
			ControlLeftClickHandler();
		}
		else
		{
			LeftClickHandler();
		}
	}
}

void FxExpandableWindow::OnRightClick( wxMouseEvent& event )
{
	// Only allow clicks in the title region to produce right click
	// events.
	wxPoint clickPos = event.GetPosition();
	wxRect titleRegion = GetClientRect();
	// The title region is defined as the client rect with a
	// max height equal to the height of the collapsed window.
	titleRegion.height = kCollapsedHeight;
	if( titleRegion.Contains(clickPos) )
	{
		RightClickHandler();
	}
}

void FxExpandableWindow::OnSize( wxSizeEvent& FxUnused(event) )
{
	// Update the children.
	Freeze();
	if( _resizeChildren )
	{
		for( FxSize i = 0; i < _children.Length(); ++i )
		{
			_children[i]->SetSize(wxSize(GetRect().width - 5 - kChildIndent,
				_children[i]->GetRect().height));
		}
	}
	if( GetAutoLayout() )
	{
		Layout();
	}
	if( GetSizer() )
	{
		FxInt32 w = 0;
		FxInt32 h = 0;
		GetVirtualSize(&w, &h);
		GetSizer()->SetDimension(0, 0, w, h);
	}
	Thaw();
}

//------------------------------------------------------------------------------
// Expandable window container.
//------------------------------------------------------------------------------

wxColour FxExpandableWindowContainer::_expandableWindowColor = wxColour();
wxColour FxExpandableWindowContainer::_panelBackgroundColor  = wxColour();
wxColour FxExpandableWindowContainer::_shadowColor           = wxColour();
wxColour FxExpandableWindowContainer::_highlightColor        = wxColour();
wxColour FxExpandableWindowContainer::_textColor             = wxColour();

WX_IMPLEMENT_DYNAMIC_CLASS(FxExpandableWindowContainer, wxScrolledWindow)

BEGIN_EVENT_TABLE(FxExpandableWindowContainer, wxScrolledWindow)
	EVT_SIZE(FxExpandableWindowContainer::OnSize)
	EVT_SYS_COLOUR_CHANGED(FxExpandableWindowContainer::OnSystemColorsChanged)
END_EVENT_TABLE()

FxExpandableWindowContainer::FxExpandableWindowContainer( wxWindow* parent )
	: wxScrolledWindow( parent, -1, wxDefaultPosition, wxDefaultSize )
	, _height(0)
	, _allAreExpanded(FxFalse)
{
	SetWindowStyle(GetWindowStyle()|wxFULL_REPAINT_ON_RESIZE);
	_expandableWindowColor = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	_panelBackgroundColor  = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
	_shadowColor           = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	_highlightColor        = wxSystemSettings::GetColour(wxSYS_COLOUR_3DHILIGHT);
	_textColor             = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
	EnableScrolling(FALSE, TRUE);
}

FxExpandableWindowContainer::~FxExpandableWindowContainer()
{
}

void FxExpandableWindowContainer::
AddExpandableWindow( FxExpandableWindow* expandableWindow )
{
	// Don't allow duplicates.
	if( FxInvalidIndex == _expandableWindows.Find(expandableWindow) )
	{
		_expandableWindows.PushBack(expandableWindow);
	}
	PositionExapandableWindows();
}

void FxExpandableWindowContainer::
RemoveExpandableWindow( FxExpandableWindow* expandableWindow )
{
	FxSize index = _expandableWindows.Find(expandableWindow);
	if( FxInvalidIndex != index )
	{
		if( _expandableWindows[index] )
		{
			_expandableWindows[index]->Destroy();
			_expandableWindows.Remove(index);
		}
	}
	PositionExapandableWindows();
}

void FxExpandableWindowContainer::RemoveAllExpandableWindows( void )
{
	Freeze();
	for( FxSize i = 0; i < _expandableWindows.Length(); ++i )
	{
		if( _expandableWindows[i] )
		{
			_expandableWindows[i]->Destroy();
		}
	}
	_expandableWindows.Clear();
	Thaw();
}

FxSize FxExpandableWindowContainer::GetNumExpandableWindows( void ) const
{
	return _expandableWindows.Length();
}

FxExpandableWindow* 
FxExpandableWindowContainer::GetExpandableWindow( FxSize index )
{
	if( index < _expandableWindows.Length() )
	{
		return _expandableWindows[index];
	}
	return NULL;
}

void FxExpandableWindowContainer::
PositionExapandableWindows( void )
{
	Freeze();
	 // Properly size and position all the contained windows.
	_height = 0;
   	for( FxSize i = 0; i < _expandableWindows.Length(); ++i )
	{
		if( _expandableWindows[i] )
		{
			if( FxFalse == _expandableWindows[i]->IsHidden() )
			{
				FxInt32 windowHeight = _expandableWindows[i]->GetHeight();
				FxInt32 scrolledPosX = 0;
				FxInt32 scrolledPosY = 0;
				CalcScrolledPosition(0,_height,&scrolledPosX,&scrolledPosY);
				_expandableWindows[i]->SetSize(scrolledPosX,scrolledPosY,
					GetClientRect().GetWidth(),windowHeight);
				_height += windowHeight;
			}
		}
	}
	UpdateScrollBar();
	Thaw();
	// Refresh();
}

void FxExpandableWindowContainer::Sort( FxBool ascending )
{
	// Simple bubble sort.
	FxBool exchanges = FxTrue;
	while( exchanges )
	{
		exchanges = FxFalse;
		for( FxSize i = 0; i < _expandableWindows.Length() - 1; ++i ) 
		{
			if( _expandableWindows[i] && _expandableWindows[i+1] )
			{
				if( *_expandableWindows[i] < *_expandableWindows[i+1] ) 
				{
					FxExpandableWindow* temp = _expandableWindows[i]; 
					_expandableWindows[i]    = _expandableWindows[i+1]; 
					_expandableWindows[i+1]  = temp;
					exchanges = FxTrue;
				}
			}
		}
	}
	if( ascending )
	{
		FxArray<FxExpandableWindow*> temp;
		while( _expandableWindows.Length() )
		{
			temp.PushBack(_expandableWindows.Back());
			_expandableWindows.PopBack();
		}
		for( FxSize i = 0; i < temp.Length(); ++i )
		{
			_expandableWindows.PushBack(temp[i]);
		}
		temp.Clear();
	}
	PositionExapandableWindows();
}

void FxExpandableWindowContainer::ExpandAll( void )
{
	for( FxSize i = 0; i < _expandableWindows.Length(); ++i )
	{
		if( _expandableWindows[i] )
		{
			if( FxFalse == _expandableWindows[i]->IsHidden() )
			{
				_expandableWindows[i]->Expand(FxFalse);
			}
		}
	}
	_allAreExpanded = FxTrue;
	PositionExapandableWindows();
	SetFocus();
}

void FxExpandableWindowContainer::CollapseAll( void )
{
	for( FxSize i = 0; i < _expandableWindows.Length(); ++i )
	{
		if( _expandableWindows[i] )
		{
			if( FxFalse == _expandableWindows[i]->IsHidden() )
			{
				_expandableWindows[i]->Collapse(FxFalse);
			}
		}
	}
	_allAreExpanded = FxFalse;
	PositionExapandableWindows();
}

FxBool FxExpandableWindowContainer::AllAreExpanded( void ) const
{
	return _allAreExpanded;
}

void FxExpandableWindowContainer::OnDraw( wxDC& dc )
{
	dc.SetPen(wxPen(_panelBackgroundColor, 1, wxSOLID));
	dc.SetBrush(wxBrush(_panelBackgroundColor,wxSOLID));
	dc.DrawRectangle(GetClientRect());
	// Draw all the contained windows.
	for( FxSize i = 0; i < _expandableWindows.Length(); ++i )
	{
		if( _expandableWindows[i] )
		{
			if( FxFalse == _expandableWindows[i]->IsHidden() )
			{
				_expandableWindows[i]->Draw();
			}
		}
	}
}

void FxExpandableWindowContainer::OnSize( wxSizeEvent& event )
{
	PositionExapandableWindows();
	UpdateScrollBar();
	Refresh();
	event.Skip();
}

void FxExpandableWindowContainer::OnSystemColorsChanged( wxSysColourChangedEvent& event )
{
	_expandableWindowColor = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	_panelBackgroundColor  = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
	_shadowColor           = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	_highlightColor        = wxSystemSettings::GetColour(wxSYS_COLOUR_3DHILIGHT);
	_textColor             = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
	Refresh();
	event.Skip();
}

void FxExpandableWindowContainer::UpdateScrollBar( void )
{
	SetScrollbars(0,kCollapsedHeight,0,_height/kCollapsedHeight,0,
		          GetScrollPos(wxVERTICAL),true);
}

} // namespace Face

} // namespace OC3Ent
