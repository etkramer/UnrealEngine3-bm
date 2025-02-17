//------------------------------------------------------------------------------
// The FaceFx command panel for entering commands and displaying the the last
// command status.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCommandPanel_H__
#define FxCommandPanel_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxString.h"

namespace OC3Ent
{

namespace Face
{

// A wrapper class for wxTextCtrl so that the up and down arrows can be 
// processed correctly.
class FxCommandLine : public wxTextCtrl
{
	typedef wxTextCtrl Super;
	WX_DECLARE_DYNAMIC_CLASS(FxCommandLine);
	DECLARE_EVENT_TABLE();
public:
	FxCommandLine();
	FxCommandLine( wxWindow* parent,
				   wxWindowID id = -1,
				   const wxString& rsValue = wxEmptyString,
				   const wxPoint& pos = wxDefaultPosition,
				   const wxSize& size = wxDefaultSize,
				   long style = 0 );

protected:
	void OnChar( wxKeyEvent& event );
};

// A command panel.
class FxCommandPanel : public wxWindow
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxCommandPanel);
	DECLARE_EVENT_TABLE();
public:
	FxCommandPanel();
	FxCommandPanel( wxWindow* parent, 
					wxWindowID id = -1, 
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& size = wxDefaultSize, 
					long style = 0 );

	void Create( wxWindow *parent, 
				 wxWindowID id = -1, 
		         const wxPoint& pos  = wxDefaultPosition, 
				 const wxSize& size = wxDefaultSize, 
				 long style = 0 );

	wxSize DoGetBestSize( void ) const;

	void ScrollCommandHistory( FxBool scrollUp );

protected:
	void OnSize( wxSizeEvent& event );
	void OnLeftClick( wxMouseEvent& event );
	void OnPaint( wxPaintEvent& event );
	
	void ProcessCommand( wxCommandEvent& event );

	wxSizer* _mainSizer;
	FxBool   _collapsed;

	FxCommandLine* _commandLine;
	wxTextCtrl* _commandResult;

	FxArray<FxString> _commandHistory;
	FxSize _commandHistoryPos;
};

} // namespace Face

} // namespace OC3Ent

#endif
