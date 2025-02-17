//------------------------------------------------------------------------------
// The FaceFx command panel for entering commands and displaying the the last
// command status.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxCommandPanel.h"
#include "FxVM.h"

// Error code color.  Imported from FxVM.cpp.
extern wxColour kNormalColor;

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxCommandLine, wxTextCtrl);

BEGIN_EVENT_TABLE(FxCommandLine, wxWindow)
	EVT_CHAR(FxCommandLine::OnChar)
END_EVENT_TABLE()

FxCommandLine::FxCommandLine()
{
}

FxCommandLine::FxCommandLine( wxWindow* parent,	wxWindowID id, const wxString& rsValue, const wxPoint& pos, const wxSize& size, long style )
{
	Super::Create(parent, id, rsValue, pos, size, style);
}

void FxCommandLine::OnChar( wxKeyEvent& event )
{
	if( WXK_UP == event.GetKeyCode() )
	{
		if( GetParent() )
		{
			static_cast<FxCommandPanel*>(GetParent())->ScrollCommandHistory(FxTrue);
		}
	}
	else if( WXK_DOWN == event.GetKeyCode() )
	{
		if( GetParent() )
		{
			static_cast<FxCommandPanel*>(GetParent())->ScrollCommandHistory(FxFalse);
		}
	}
	else
	{
		Super::OnChar(event);
		event.Skip();
	}
}

WX_IMPLEMENT_DYNAMIC_CLASS(FxCommandPanel, wxWindow)

BEGIN_EVENT_TABLE(FxCommandPanel, wxWindow)
	EVT_SIZE(FxCommandPanel::OnSize)
	EVT_LEFT_UP(FxCommandPanel::OnLeftClick)
	EVT_PAINT(FxCommandPanel::OnPaint)
	
	EVT_TEXT_ENTER(ControlID_CommandPanelCommandLineText, FxCommandPanel::ProcessCommand)
END_EVENT_TABLE()

FxCommandPanel::FxCommandPanel()
	: _collapsed(FxFalse)
	, _mainSizer(0)
	, _commandLine(NULL)
	, _commandResult(NULL)
{
}

FxCommandPanel::FxCommandPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
	: _collapsed(FxFalse)
	, _mainSizer(0)
	, _commandLine(NULL)
	, _commandResult(NULL)
{
	Create(parent, id, pos, size, style);
}

void FxCommandPanel::Create(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
	wxWindow::Create(parent, id, pos, size, style);

	_mainSizer  = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(_mainSizer);
	SetAutoLayout(FALSE);

	_mainSizer->AddSpacer(12);
	// Create the left side text control.
	_commandLine = new FxCommandLine(this, ControlID_CommandPanelCommandLineText, wxEmptyString, wxDefaultPosition, wxSize(-1, 20), wxTE_PROCESS_ENTER);
	_commandLine->SetToolTip(_("Command Line"));
	_mainSizer->Add(_commandLine, 1, wxGROW, 5);
	
	// Create the right side text control.
	_commandResult = new wxTextCtrl(this, ControlID_CommandPanelCommandResultText, wxEmptyString, wxDefaultPosition, wxSize(-1, 20), wxTE_READONLY);
	_commandResult->SetBackgroundColour(kNormalColor);
	_commandResult->SetToolTip(_("Command Result"));
	_mainSizer->Add(_commandResult, 1, wxGROW, 5);

	FxVM::SetOutputCtrl(_commandResult);

	Layout();
}

wxSize FxCommandPanel::DoGetBestSize( void ) const
{
	if( !_collapsed )
	{
		return wxSize(-1, 22);
	}
	else
	{
		return wxSize(-1, 5);
	}
}

void FxCommandPanel::ScrollCommandHistory( FxBool scrollUp )
{
	if( scrollUp )
	{
		if( _commandHistoryPos != 0 )
		{
			_commandHistoryPos--;
		}
	}
	else
	{
		if( _commandHistoryPos != _commandHistory.Length() - 1 )
		{
			_commandHistoryPos++;
		}
	}
	if( _commandHistoryPos >= 0 && _commandHistoryPos < _commandHistory.Length() )
	{
		if( _commandLine )
		{
			_commandLine->SetValue(wxString(_commandHistory[_commandHistoryPos].GetData(), wxConvLibc));
		}
	}
}

void FxCommandPanel::OnSize( wxSizeEvent& FxUnused(event) )
{
	Layout();
}

void FxCommandPanel::OnLeftClick( wxMouseEvent& event )
{
	if( (_collapsed && event.GetX() <= 23) ||
		(!_collapsed && event.GetX() <= 10 ) )
	{
		_collapsed = !_collapsed;

		if( _collapsed )
		{
			_mainSizer->Show(1, FALSE);
			_mainSizer->Show(2, FALSE);
		}
		else
		{
			_mainSizer->Show(1, TRUE);
			_mainSizer->Show(2, TRUE);
		}

		if( GetParent() )
		{
			GetParent()->Layout();
		}
		Layout();
	}
}

void FxCommandPanel::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);
	wxColour buttonColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	wxColour buttonHighlight = wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT);
	wxColour buttonShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	if( _collapsed )
	{
		dc.SetBrush(wxBrush(buttonColour,wxSOLID));
		dc.SetPen(wxPen(buttonShadow,1,wxSOLID));
		dc.DrawRectangle(1, 0, 23, 5);
		dc.SetPen(wxPen(buttonHighlight,1,wxSOLID));
		dc.DrawLine(1,5,1,0);
		dc.DrawLine(1,0,23,0);

		wxPoint lines[] = {wxPoint(0,5), wxPoint(2,0), wxPoint(4,5)};
		dc.SetPen(wxPen(wxColour(75,75,75),1,wxSOLID));
		dc.DrawLines(WXSIZEOF(lines), lines, 3);
		dc.DrawLines(WXSIZEOF(lines), lines, 9);
		dc.DrawLines(WXSIZEOF(lines), lines, 15);
	}
	else
	{
		dc.SetBrush(wxBrush(buttonColour,wxSOLID));
		dc.SetPen(wxPen(buttonShadow,1,wxSOLID));
		dc.DrawRectangle(1, 0, 11, 20);
		dc.SetPen(wxPen(buttonHighlight,1,wxSOLID));
		dc.DrawLine(1,20,1,0);
		dc.DrawLine(1,0,11,0);

		wxPoint lines[] = {wxPoint(0,0), wxPoint(2,5), wxPoint(4,0)};
		dc.SetPen(wxPen(wxColour(75,75,75),1,wxSOLID));
		dc.DrawLines(WXSIZEOF(lines), lines, 4, 1);
		dc.DrawLines(WXSIZEOF(lines), lines, 4, 7);
		dc.DrawLines(WXSIZEOF(lines), lines, 4, 13);

		event.Skip();
	}
}

void FxCommandPanel::ProcessCommand( wxCommandEvent& FxUnused(event) )
{
	if( _commandLine && _commandResult )
	{
		wxString commandString = _commandLine->GetValue();
		_commandLine->SetValue(wxEmptyString);
		if( wxEmptyString != commandString )
		{
			FxString command(commandString.mb_str(wxConvLibc));
			FxVM::ExecuteCommand(command);
			_commandHistory.PushBack(command);
			_commandHistoryPos = _commandHistory.Length();
		}
	}
}

} // namespace Face

} // namespace OC3Ent
