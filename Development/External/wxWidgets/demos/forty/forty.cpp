/////////////////////////////////////////////////////////////////////////////
// Name:        forty.cpp
// Purpose:     Forty Thieves patience game
// Author:      Chris Breeze
// Modified by:
// Created:     21/07/97
// RCS-ID:      $Id: forty.cpp 40798 2006-08-24 16:33:28Z ABX $
// Copyright:   (c) 1993-1998 Chris Breeze
// Licence:     wxWindows licence
//---------------------------------------------------------------------------
// Last modified: 22nd July 1998 - ported to wxWidgets 2.0
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "canvas.h"
#include "forty.h"
#include "card.h"
#include "scoredg.h"

#if wxUSE_HTML
#include "wx/textfile.h"
#include "wx/html/htmlwin.h"
#endif

#include "wx/stockitem.h"

BEGIN_EVENT_TABLE(FortyFrame, wxFrame)
    EVT_MENU(wxID_NEW, FortyFrame::NewGame)
    EVT_MENU(wxID_EXIT, FortyFrame::Exit)
    EVT_MENU(wxID_ABOUT, FortyFrame::About)
    EVT_MENU(wxID_HELP_CONTENTS, FortyFrame::Help)
    EVT_MENU(wxID_UNDO, FortyFrame::Undo)
    EVT_MENU(wxID_REDO, FortyFrame::Redo)
    EVT_MENU(SCORES, FortyFrame::Scores)
    EVT_MENU(RIGHT_BUTTON_UNDO, FortyFrame::ToggleRightButtonUndo)
    EVT_MENU(HELPING_HAND, FortyFrame::ToggleHelpingHand)
    EVT_MENU(LARGE_CARDS, FortyFrame::ToggleCardSize)
    EVT_CLOSE(FortyFrame::OnCloseWindow)
END_EVENT_TABLE()

// Create a new application object
IMPLEMENT_APP (FortyApp)

wxColour* FortyApp::m_backgroundColour = 0;
wxColour* FortyApp::m_textColour = 0;
wxBrush*  FortyApp::m_backgroundBrush = 0;

FortyApp::~FortyApp()
{
    delete m_backgroundColour;
    delete m_textColour;
    delete m_backgroundBrush;
    delete Card::m_symbolBmap;
    delete Card::m_pictureBmap;

}

bool FortyApp::OnInit()
{
    bool largecards = false;
#ifndef __WXWINCE__
    m_helpFile = wxGetCwd() + wxFILE_SEP_PATH + wxT("about.htm");
    if (!wxFileExists(m_helpFile))
#endif
    {
        m_helpFile = wxPathOnly(argv[0]) + wxFILE_SEP_PATH + wxT("about.htm");
    }

    wxSize size(668,510);

    if ((argc > 1) && (!wxStrcmp(argv[1],_T("-L"))))
    {
        largecards = true;
        size = wxSize(1000,750);
    }

    FortyFrame* frame = new FortyFrame(
            0,
            _T("Forty Thieves"),
            wxDefaultPosition,
            size,
            largecards
    );

     // Show the frame
    frame->Show(true);

    frame->GetCanvas()->ShowPlayerDialog();

    return true;
}

const wxColour& FortyApp::BackgroundColour()
{
    if (!m_backgroundColour)
    {
        m_backgroundColour = new wxColour(0, 128, 0);
    }

    return *m_backgroundColour;
}

const wxBrush& FortyApp::BackgroundBrush()
{
    if (!m_backgroundBrush)
    {
        m_backgroundBrush = new wxBrush(BackgroundColour(), wxSOLID);
    }

    return *m_backgroundBrush;
}

const wxColour& FortyApp::TextColour()
{
    if (!m_textColour)
    {
        m_textColour = new wxColour(*wxBLACK);
    }

    return *m_textColour;
}

// My frame constructor
FortyFrame::FortyFrame(wxFrame* frame, const wxString& title, const wxPoint& pos, const wxSize& size, bool largecards):
    wxFrame(frame, wxID_ANY, title, pos, size)
{
#ifdef __WXMAC__
    wxApp::s_macAboutMenuItemId = wxID_ABOUT ;
#endif
    // set the icon
#ifdef __WXMSW__
    SetIcon(wxIcon(_T("CardsIcon")));
#else
#ifdef GTK_TBD
    SetIcon(wxIcon(Cards_bits, Cards_width, Cards_height));
#endif
#endif

    // Make a menu bar
    wxMenu* gameMenu = new wxMenu;
    gameMenu->Append(wxID_NEW, wxGetStockLabel(wxID_NEW), _T("Start a new game"));
    gameMenu->Append(SCORES, _T("&Scores..."), _T("Displays scores"));
    gameMenu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT), _T("Exits Forty Thieves"));

    wxMenu* editMenu = new wxMenu;
    editMenu->Append(wxID_UNDO, wxGetStockLabel(wxID_UNDO), _T("Undo the last move"));
    editMenu->Append(wxID_REDO, wxGetStockLabel(wxID_REDO), _T("Redo a move that has been undone"));

    wxMenu* optionsMenu = new wxMenu;
    optionsMenu->Append(RIGHT_BUTTON_UNDO,
            _T("&Right button undo"),
            _T("Enables/disables right mouse button undo and redo"),
            true
            );
    optionsMenu->Append(HELPING_HAND,
            _T("&Helping hand"),
            _T("Enables/disables hand cursor when a card can be moved"),
            true
            );
    optionsMenu->Append(LARGE_CARDS,
            _T("&Large cards"),
            _T("Enables/disables large cards for high resolution displays"),
            true
            );
    optionsMenu->Check(HELPING_HAND, true);
    optionsMenu->Check(RIGHT_BUTTON_UNDO, true);
    optionsMenu->Check(LARGE_CARDS, largecards ? true : false);

    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_HELP_CONTENTS, _T("&Help Contents"), _T("Displays information about playing the game"));
    helpMenu->Append(wxID_ABOUT, _T("&About..."), _T("About Forty Thieves"));

    m_menuBar = new wxMenuBar;
    m_menuBar->Append(gameMenu,    _T("&Game"));
    m_menuBar->Append(editMenu,    _T("&Edit"));
    m_menuBar->Append(optionsMenu, _T("&Options"));
    m_menuBar->Append(helpMenu,    _T("&Help"));

    SetMenuBar(m_menuBar);

    if (largecards)
        Card::SetScale(1.3);

    m_canvas = new FortyCanvas(this, wxDefaultPosition, size);

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
    topsizer->Add( m_canvas, 1, wxEXPAND | wxALL, 0);
    SetSizer( topsizer );
    topsizer->SetSizeHints( this );

#if wxUSE_STATUSBAR
    CreateStatusBar();
#endif // wxUSE_STATUSBAR
}

void FortyFrame::OnCloseWindow(wxCloseEvent& event)
{
    if (m_canvas->OnCloseCanvas() )
    {
        this->Destroy();
    }
    else
        event.Veto();
}

void
FortyFrame::NewGame(wxCommandEvent&)
{
    m_canvas->NewGame();
}

void
FortyFrame::Exit(wxCommandEvent&)
{
    Close(true);
}

void
FortyFrame::Help(wxCommandEvent& event)
{
#if wxUSE_HTML
    if (wxFileExists(wxGetApp().GetHelpFile()))
    {
        FortyAboutDialog dialog(this, wxID_ANY, wxT("Forty Thieves Instructions"));
        if (dialog.ShowModal() == wxID_OK)
        {
        }
    }
    else
#endif
    {
        About(event);
    }
}

void
FortyFrame::About(wxCommandEvent&)
{
        wxMessageBox(
            _T("Forty Thieves\n\n")
            _T("A free card game written with the wxWidgets toolkit\n")
            _T("Author: Chris Breeze (c) 1992-2004\n")
            _T("email: chris@breezesys.com"),
            _T("About Forty Thieves"),
            wxOK|wxICON_INFORMATION, this
            );
}


void
FortyFrame::Undo(wxCommandEvent&)
{
    m_canvas->Undo();
}

void
FortyFrame::Redo(wxCommandEvent&)
{
    m_canvas->Redo();
}

void
FortyFrame::Scores(wxCommandEvent&)
{
    m_canvas->UpdateScores();
    ScoreDialog scores(this, m_canvas->GetScoreFile());
    scores.Display();
}

void
FortyFrame::ToggleRightButtonUndo(wxCommandEvent& event)
{
    bool checked = m_menuBar->IsChecked(event.GetId());
    m_canvas->EnableRightButtonUndo(checked);
}

void
FortyFrame::ToggleHelpingHand(wxCommandEvent& event)
{
    bool checked = m_menuBar->IsChecked(event.GetId());
    m_canvas->EnableHelpingHand(checked);
}

void
FortyFrame::ToggleCardSize(wxCommandEvent& event)
{
        bool checked = m_menuBar->IsChecked(event.GetId());
        Card::SetScale(checked ? 1.3 : 1);
        m_canvas->LayoutGame();
        m_canvas->Refresh();
}

//----------------------------------------------------------------------------
// stAboutDialog
//----------------------------------------------------------------------------

FortyAboutDialog::FortyAboutDialog( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    AddControls(this);

    Centre(wxBOTH);
}

bool FortyAboutDialog::AddControls(wxWindow* parent)
{
#if wxUSE_HTML
    wxString htmlText;
    wxString htmlFile = wxGetApp().GetHelpFile();

    {
        wxTextFile file(htmlFile);
        if (file.Exists())
        {
            file.Open();
            for ( htmlText = file.GetFirstLine();
                  !file.Eof();
                  htmlText << file.GetNextLine() << _T("\n") ) ;
        }
    }

    if (htmlText.empty())
    {
        htmlText.Printf(wxT("<html><head><title>Warning</title></head><body><P>Sorry, could not find resource for About dialog<P></body></html>"));
    }

    // Customize the HTML
    htmlText.Replace(wxT("$DATE$"), _T(__DATE__));

    wxSize htmlSize(400, 290);

    // Note: in later versions of wxWin this will be fixed so wxRAISED_BORDER
    // does the right thing. Meanwhile, this is a workaround.
#ifdef __WXMSW__
    long borderStyle = wxDOUBLE_BORDER;
#else
    long borderStyle = wxRAISED_BORDER;
#endif

    wxHtmlWindow* html = new wxHtmlWindow(this, ID_ABOUT_HTML_WINDOW, wxDefaultPosition, htmlSize, borderStyle);
    html -> SetBorders(10);
    html -> SetPage(htmlText);

    //// Start of sizer-based control creation

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxWindow *item1 = parent->FindWindow( ID_ABOUT_HTML_WINDOW );
    wxASSERT( item1 );
    item0->Add( item1, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item2 = new wxButton( parent, wxID_CLOSE );
    item2->SetDefault();
    item2->SetFocus();
    SetAffirmativeId(wxID_CLOSE);

    item0->Add( item2, 0, wxALIGN_RIGHT|wxALL, 5 );

    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );
#endif

    return true;
}
