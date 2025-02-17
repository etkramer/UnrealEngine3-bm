//------------------------------------------------------------------------------
// A widget to view the HTML console output in real-time.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxConsoleWidget.h"

namespace OC3Ent
{

namespace Face
{

FxConsoleWidget::FxConsoleWidget()
	: _parent(NULL)
	, _htmlWin(NULL)
{
}

FxConsoleWidget::~FxConsoleWidget()
{
}

void FxConsoleWidget::Initialize( wxWindow* parent )
{
	_parent = parent;

	if( _parent )
	{
		_htmlWin = new FxHtmlLogWindow(this, _parent);
		if( _htmlWin )
		{
			_htmlWin->SetBorders(0);
		}
		ClearHtmlWindowText();
	}
}

void FxConsoleWidget::ClearHtmlWindowText( void )
{
	if( _htmlWin )
	{
		_htmlWin->SetPage(wxT("<html><body>"));
	}
}

void FxConsoleWidget::Append( const FxString& source )
{
	if( _htmlWin )
	{
		_htmlWin->AppendLine(wxString(source.GetData(), wxConvLibc));
	}
}

void FxConsoleWidget::AppendW( const FxWString& source )
{
	if( _htmlWin )
	{
		_htmlWin->AppendLine(wxString(source.GetData(), wxConvLibc));
	}
}

wxHtmlWindow* FxConsoleWidget::GetHtmlWindow( void )
{
	return _htmlWin;
}

} // namespace Face

} // namespace OC3Ent
