/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "Log.h"

/**
 * Handles adding a cell (or string of cells) to the list. If the additions
 * go beyond the maximum specified, the oldest ones are removed
 *
 * @param Cell the new set of cells to append to our container
 */
void WxBoundedHtmlContainerCell::InsertCell(wxHtmlCell* Cell)
{
	check(Cell && "Can't add a NULL cell");
	// Init our vars that will track how much is being added
	wxHtmlCell* LastCellAdded = Cell;
	DWORD NumCellsAdded = 1;
	// Figure out how many cells are being added and keep a pointer to the last one
	for (wxHtmlCell* Start = Cell; Start->GetNext() != NULL;
		Start = Start->GetNext())
	{
		LastCellAdded = Start;
		NumCellsAdded++;
	}
	// If we are attempting to add more than we can hold, delete the first ones
	while (NumCellsAdded >= MaxCellCount)
	{
		// Grab the head of the list
		wxHtmlCell* DeleteMe = Cell;
		// Move to the next one in the list
		Cell = Cell->GetNext();
		// Snip it off
		DeleteMe->SetNext(NULL);
		// Now free it
		delete DeleteMe;
		// And keep track of our count
		NumCellsAdded--;
	}
	// If the addition will cross our max, delete cells until we are valid
	while ((NumCellsAdded + CurrentCellCount) >= MaxCellCount)
	{
		// Grab the head of the list
		wxHtmlCell* DeleteMe = m_Cells;
		// Snip it off
		m_Cells = m_Cells->GetNext();
		DeleteMe->SetNext(NULL);
		// Now free it
		delete DeleteMe;
		// And keep track of our count
		CurrentCellCount--;
	}
	// Update our count and tell the cell it is owned
	CurrentCellCount += NumCellsAdded;
    Cell->SetParent(this);
	// Now append the list of cells
	if (m_Cells != NULL)
	{
		m_LastCell->SetNext(Cell);
		m_LastCell = LastCellAdded;
	}
	else
	{
		m_Cells = Cell;
		m_LastCell = LastCellAdded;
	}
	// Invalidate our draw state
    m_LastLayout = -1;
}

/**
 * Constructor that creates the bounded cell container and puts that in
 * the html window's cell list
 *
 * @param InMaxCellsToDisplay the number of lines to display
 * @param InParent the parent of this window
 * @param InID the ID to assign to the new window
 */
WxBoundedHtmlWindow::WxBoundedHtmlWindow(DWORD InMaxCellsToDisplay,
	wxWindow* InParent,wxWindowID InID) : wxHtmlWindow(InParent,InID)
{
	// Remove any existing container
	if (m_Cell != NULL)
	{
		delete m_Cell;
		m_Cell = NULL;
	}
	// Create the container with the specified maximum 
	ContainerCell = new WxBoundedHtmlContainerCell(InMaxCellsToDisplay,NULL);
	check(ContainerCell != NULL);
	// Replace their container with this one
	m_Cell = ContainerCell;
}

/*-----------------------------------------------------------------------------
	WxHtmlWindow
-----------------------------------------------------------------------------*/
BEGIN_EVENT_TABLE( WxHtmlWindow, WxBoundedHtmlWindow )
	EVT_SIZE( WxHtmlWindow::OnSize )
END_EVENT_TABLE()

WxHtmlWindow::WxHtmlWindow(DWORD InMaxCellsToDisplay,wxWindow* InParent,wxWindowID InID)
: WxBoundedHtmlWindow( InMaxCellsToDisplay, InParent, InID )
{
}

void WxHtmlWindow::AppendLine( const FString& InLine )
{
	{
		wxClientDC dc(this);
		dc.SetMapMode(wxMM_TEXT);
		m_Parser->SetDC(&dc);

		wxHtmlContainerCell* c2 = (wxHtmlContainerCell*)m_Parser->Parse(*InLine);
		// Add to the bounded list so that old lines can be culled
		ContainerCell->InsertCell(c2);
	}
	CreateLayout();
	ScrollToBottom();
	Refresh();
}

inline void WxHtmlWindow::ScrollToBottom()
{
	INT x, y;
	GetVirtualSize( &x, &y );
	Scroll(0, y);
}

void WxHtmlWindow::OnSize( wxSizeEvent& In )
{
	// parent implementation of OnSize calls CreateLayout(), which will clear the current scrollbar positions.  so before we call the parent version
	// save the current scrollbar positions so we can restore them afterwards.
	INT CurrentScrollX=0, CurrentScrollY=0;
	GetViewStart(&CurrentScrollX, &CurrentScrollY);

	WxBoundedHtmlWindow::OnSize(In);

	// wxScrollingPanel sets Skip(true) on the event which triggers another [redundant] call to the base version of OnSize
	// so here we unset the skip flag
	In.Skip(false);

	// finally restore the original scroll positions.
	Scroll(CurrentScrollX, CurrentScrollY);
}

/*-----------------------------------------------------------------------------
	WxLogWindowFrame
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxLogWindowFrame,wxFrame);

BEGIN_EVENT_TABLE( WxLogWindowFrame, wxFrame )
	EVT_SIZE( WxLogWindowFrame::OnSize )
END_EVENT_TABLE()

WxLogWindowFrame::WxLogWindowFrame( wxWindow* Parent )
	: wxFrame( Parent, -1, *LocalizeUnrealEd("Log"), wxDefaultPosition, wxDefaultSize, (Parent ? wxFRAME_FLOAT_ON_PARENT : wxDIALOG_NO_PARENT ) | wxDEFAULT_FRAME_STYLE | wxFRAME_TOOL_WINDOW | wxFRAME_NO_TASKBAR )
{
	//LogWindow = new WxLogWindow( this, Log.Filename, Log.LogAr );

	FWindowUtil::LoadPosSize( TEXT("FloatingLogWindow"), this, 64,64,350,500 );
}

WxLogWindowFrame::~WxLogWindowFrame()
{
	FWindowUtil::SavePosSize( TEXT("FloatingLogWindow"), this );
}

void WxLogWindowFrame::OnSize( wxSizeEvent& In )
{
	if( LogWindow )
	{
		const wxPoint pt = GetClientAreaOrigin();
		wxRect rc = GetClientRect();
		rc.y -= pt.y;

		LogWindow->SetSize( rc );
	}
}

/*-----------------------------------------------------------------------------
	WxLogWindow
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxLogWindow,wxPanel);

BEGIN_EVENT_TABLE( WxLogWindow, wxPanel )
	EVT_SIZE( WxLogWindow::OnSize )
	EVT_TEXT_ENTER( XRCID("IDCB_COMMAND"), WxLogWindow::OnExecCommand )
END_EVENT_TABLE()

WxLogWindow::WxLogWindow( wxWindow* InParent )
	: wxPanel( InParent, -1 )
{
	Panel = (wxPanel*)wxXmlResource::Get()->LoadPanel( this, TEXT("ID_LOGWINDOW") );
	check( Panel != NULL );
	Panel->Fit();

	GLog->AddOutputDevice( this );

	INT MaxLogLines = 100;
	// Get the number of lines of log history from the editor ini
	GConfig->GetInt(TEXT("LogWindow"),TEXT("MaxNumberOfLogLines"),MaxLogLines,
		GEditorIni);

	HTMLWindow = new WxHtmlWindow( (DWORD)MaxLogLines, this, ID_LOG );
	wxXmlResource::Get()->AttachUnknownControl( wxT("ID_LOG"), HTMLWindow );
	HTMLWindow->SetWindowStyleFlag( wxSUNKEN_BORDER );

	CommandCombo = wxDynamicCast( FindWindow( XRCID( "IDCB_COMMAND" ) ), wxComboBox );
	check( CommandCombo != NULL );

	HTMLWindow->SetWindowStyle( HTMLWindow->GetWindowStyle() );
	CommandCombo->SetWindowStyle(CommandCombo->GetWindowStyle() | wxTE_PROCESS_ENTER);

//	ADDEVENTHANDLER( XRCID("IDCB_COMMAND"), wxEVT_COMMAND_TEXT_ENTER, &WxLogWindow::OnExecCommand );
}

WxLogWindow::~WxLogWindow()
{
	GLog->RemoveOutputDevice( this );
}

void WxLogWindow::Serialize( const TCHAR* V, EName Event )
{
	if( Event == NAME_Title )
	{
		SetLabel( V );
		return;
	}
	else if( IsShown() )
	{
		// Create a formatted string based on the event type

		FString Wk;
		FString FormattedText = V;
		FormattedText.ReplaceInline(TEXT("<"), TEXT("&lt;"));
		FormattedText.ReplaceInline(TEXT(">"), TEXT("&gt;"));
		FormattedText.ReplaceInline(TEXT("\n"), TEXT("<br>"));
		FormattedText.ReplaceInline(TEXT("\t"), TEXT("&nbsp;&nbsp;&nbsp;&nbsp;"));
		FormattedText.ReplaceInline(TEXT(" "), TEXT("&nbsp;"));

		switch( Event )
		{
			case NAME_Cmd:
				Wk += TEXT("<B>");
				break;

			case NAME_Error:
				Wk += TEXT("<B>");
				Wk += TEXT("<FONT COLOR=\"#FF0000\">");
				break;

			case NAME_Warning:
				Wk += TEXT("<FONT COLOR=\"#FF0000\">");
				break;
		}

		Wk += FName(Event).ToString();
		Wk += TEXT(": ");
		Wk += FormattedText;

		switch( Event )
		{
			case NAME_Cmd:
				Wk += TEXT("</B>");
				break;

			case NAME_Error:
				Wk += TEXT("</FONT>");
				Wk += TEXT("</B>");
				break;

			case NAME_Warning:
				Wk += TEXT("</FONT>");
				break;
		}

		//Wk += TEXT("<BR>");

		HTMLWindow->AppendLine( Wk );
	}
}

void WxLogWindow::OnSize( wxSizeEvent& In )
{
	if( Panel )
	{
		const wxPoint pt = GetClientAreaOrigin();
		wxRect rc = GetClientRect();
		rc.y -= pt.y;

		Panel->SetSize( rc );
	}
}

void WxLogWindow::OnExecCommand( wxCommandEvent& In )
{
	ExecCommand(); 
}

/**
 * Takes the text in the combobox and execs it.
 */

void WxLogWindow::ExecCommand()
{
	const wxString& String = CommandCombo->GetValue();
	const TCHAR* Cmd = String;

	if( Cmd && appStrlen(Cmd) )
	{
		// Add the command to the combobox if it isn't already there

		if( CommandCombo->FindString( Cmd ) == -1 )
		{
			CommandCombo->Append( Cmd );
		}

		// Send the exec to the engine

		GEditor->Exec( Cmd );
	}
}
