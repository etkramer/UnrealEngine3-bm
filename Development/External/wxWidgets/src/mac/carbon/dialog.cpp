/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/dialog.cpp
// Purpose:     wxDialog class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: dialog.cpp 47699 2007-07-24 10:22:17Z SC $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/dialog.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/frame.h"
    #include "wx/settings.h"
#endif // WX_PRECOMP

#include "wx/mac/uma.h"


// Lists to keep track of windows, so we can disable/enable them
// for modal dialogs
wxList wxModalDialogs;

IMPLEMENT_DYNAMIC_CLASS(wxDialog, wxTopLevelWindow)

void wxDialog::Init()
{
    m_isModalStyle = false;
}

bool wxDialog::Create( wxWindow *parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name )
{
    SetExtraStyle( GetExtraStyle() | wxTOPLEVEL_EX_DIALOG );

    // All dialogs should really have this style...
    style |= wxTAB_TRAVERSAL;

    // ...but not these styles
    style &= ~(wxYES | wxOK | wxNO); // | wxCANCEL

    if ( !wxTopLevelWindow::Create( parent, id, title, pos, size, style, name ) )
        return false;

    return true;
}

void wxDialog::SetModal( bool flag )
{
    if ( flag )
    {
        m_isModalStyle = true;

        wxModelessWindows.DeleteObject( this );

#if TARGET_CARBON
        SetWindowModality( (WindowRef)MacGetWindowRef(), kWindowModalityAppModal, NULL ) ;
#endif
    }
    else
    {
        m_isModalStyle = false;

        wxModelessWindows.Append( this );
    }
}

wxDialog::~wxDialog()
{
    m_isBeingDeleted = true;
    Show(false);
}

// On mac command-stop does the same thing as Esc, let the base class know
// about it
bool wxDialog::IsEscapeKey(const wxKeyEvent& event)
{
    if ( event.GetKeyCode() == '.' && event.GetModifiers() == wxMOD_CMD )
        return true;

    return wxDialogBase::IsEscapeKey(event);
}

bool wxDialog::IsModal() const
{
    return wxModalDialogs.Find((wxDialog *)this) != NULL; // const_cast
    //    return m_isModalStyle;
}


bool wxDialog::IsModalShowing() const
{
    return wxModalDialogs.Find((wxDialog *)this) != NULL; // const_cast
}

bool wxDialog::Show(bool show)
{
    if ( !wxDialogBase::Show(show) )
        // nothing to do
        return false;

    if ( show )
        // usually will result in TransferDataToWindow() being called
        InitDialog();

    HiliteMenu(0);

    if ( m_isModalStyle )
    {
        if ( show )
        {
            DoShowModal();
        }
        else // end of modal dialog
        {
            // this will cause IsModalShowing() return false and our local
            // message loop will terminate
            wxModalDialogs.DeleteObject(this);
        }
    }

    return true;
}

#if !TARGET_CARBON
extern bool s_macIsInModalLoop ;
#endif

void wxDialog::DoShowModal()
{
    wxCHECK_RET( !IsModalShowing(), wxT("DoShowModal() called twice") );

    wxModalDialogs.Append(this);

    SetFocus() ;

#if TARGET_CARBON
    WindowRef windowRef = (WindowRef) MacGetWindowRef();
    WindowGroupRef formerGroup = GetWindowGroup(windowRef) ;
    SetWindowGroup( windowRef, GetWindowGroupOfClass( kMovableModalWindowClass ));
    BeginAppModalStateForWindow(windowRef) ;
#else
    // TODO : test whether parent gets disabled
    bool formerModal = s_macIsInModalLoop ;
    s_macIsInModalLoop = true ;
#endif

    while ( IsModalShowing() )
    {
        wxTheApp->MacDoOneEvent() ;
        // calls process idle itself
    }

#if TARGET_CARBON
    EndAppModalStateForWindow(windowRef) ;
    SetWindowGroup( windowRef, formerGroup );
#else
    // TODO probably reenable the parent window if any
    s_macIsInModalLoop = formerModal ;
#endif
}


// Replacement for Show(true) for modal dialogs - returns return code
int wxDialog::ShowModal()
{
    if ( !m_isModalStyle )
        SetModal(true);

    Show(true);

    return GetReturnCode();
}

// NB: this function (surprizingly) may be called for both modal and modeless
//     dialogs and should work for both of them
void wxDialog::EndModal(int retCode)
{
    SetReturnCode(retCode);
    Show(false);
    SetModal(false);
}

