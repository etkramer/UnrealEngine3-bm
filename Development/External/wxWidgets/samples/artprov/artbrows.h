/////////////////////////////////////////////////////////////////////////////
// Name:        artbrows.h
// Purpose:     wxArtProvider demo - art browser dialog
// Author:      Vaclav Slavik
// Modified by:
// Created:     2002/04/05
// RCS-ID:      $Id: artbrows.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Vaclav Slavik
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef __ARTBROWS_H__
#define __ARTBROWS_H__

#include "wx/dialog.h"
#include "wx/artprov.h"

class WXDLLEXPORT wxListCtrl;
class WXDLLEXPORT wxListEvent;
class WXDLLEXPORT wxStaticBitmap;

class wxArtBrowserDialog : public wxDialog
{
public:
    wxArtBrowserDialog(wxWindow *parent);

    void SetArtClient(const wxArtClient& client);
    void SetArtBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size = wxDefaultSize);

private:
    void OnSelectItem(wxListEvent &event);
    void OnChooseClient(wxCommandEvent &event);
    
    wxListCtrl *m_list;
    wxStaticBitmap *m_canvas;
    wxStaticText *m_text;
    wxString m_client;

    DECLARE_EVENT_TABLE()
};

#endif // __ARTBROWS_H__

