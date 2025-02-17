///////////////////////////////////////////////////////////////////////////////
// Name:        msw/evtloop.cpp
// Purpose:     implements wxEventLoop for MSW
// Author:      Vadim Zeitlin
// Modified by:
// Created:     01.06.01
// RCS-ID:      $Id: evtloop.cpp 39912 2006-06-30 22:59:01Z VZ $
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// License:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/app.h"
#endif //WX_PRECOMP

#include "wx/evtloop.h"

#include "wx/tooltip.h"
#include "wx/except.h"
#include "wx/ptr_scpd.h"

#include "wx/msw/private.h"

#if wxUSE_THREADS
    #include "wx/thread.h"

    // define the list of MSG strutures
    WX_DECLARE_LIST(MSG, wxMsgList);

    #include "wx/listimpl.cpp"

    WX_DEFINE_LIST(wxMsgList)
#endif // wxUSE_THREADS

// ============================================================================
// wxEventLoop implementation
// ============================================================================

wxWindowMSW *wxEventLoop::ms_winCritical = NULL;

// ----------------------------------------------------------------------------
// ctor/dtor
// ----------------------------------------------------------------------------

wxEventLoop::wxEventLoop()
{
    m_shouldExit = false;
    m_exitcode = 0;
}

// ----------------------------------------------------------------------------
// wxEventLoop message processing
// ----------------------------------------------------------------------------

void wxEventLoop::ProcessMessage(WXMSG *msg)
{
    // give us the chance to preprocess the message first
    if ( !PreProcessMessage(msg) )
    {
        // if it wasn't done, dispatch it to the corresponding window
        ::TranslateMessage(msg);
        ::DispatchMessage(msg);
    }
}



// @UE3 2007-16-11: Class for unreal callbacks
void wxEventLoopBase::SetUnrealCallbacks( wxUnrealCallbacks* Callbacks )
{
	s_UnrealCallbacks = Callbacks;
}

wxEventLoopBase::wxUnrealCallbacks* wxEventLoopBase::s_UnrealCallbacks=NULL;


bool wxEventLoop::IsChildOfCriticalWindow(wxWindowMSW *win)
{
    while ( win )
    {
        if ( win == ms_winCritical )
            return true;

        win = win->GetParent();
    }

    return false;
}

bool wxEventLoop::PreProcessMessage(WXMSG *msg)
{
    HWND hwnd = msg->hwnd;
    wxWindow *wndThis = wxGetWindowFromHWND((WXHWND)hwnd);
    wxWindow *wnd;

    // this might happen if we're in a modeless dialog, or if a wx control has
    // children which themselves were not created by wx (i.e. wxActiveX control children)
    if ( !wndThis )
    {
		// @UE3 2007-16-11: Unreal Engine integration.
		// We don't want pre- processing if this is an Unreal window handle without a wxWindow association.
		// Make certain we have a callback class 
		wxCHECK(NULL != s_UnrealCallbacks, false);
		if( s_UnrealCallbacks->IsUnrealWindowHandle( hwnd ) == true )
		{
			return false;
		}

		while ( hwnd && (::GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD ))
        {
            hwnd = ::GetParent(hwnd);

            // If the control has a wx parent, break and give the parent a chance
            // to process the window message
            wndThis = wxGetWindowFromHWND((WXHWND)hwnd);
            if (wndThis != NULL)
                break;
        }

        if ( !wndThis )
        {
            // this may happen if the event occurred in a standard modeless dialog (the
            // only example of which I know of is the find/replace dialog) - then call
            // IsDialogMessage() to make TAB navigation in it work

            // NOTE: IsDialogMessage() just eats all the messages (i.e. returns true for
            // them) if we call it for the control itself
            return hwnd && ::IsDialogMessage(hwnd, msg) != 0;
        }
    }

    if ( !AllowProcessing(wndThis) )
    {
        // not a child of critical window, so we eat the event but take care to
        // stop an endless stream of WM_PAINTs which would have resulted if we
        // didn't validate the invalidated part of the window
        if ( msg->message == WM_PAINT )
            ::ValidateRect(hwnd, NULL);

        return true;
    }

#if wxUSE_TOOLTIPS
    // we must relay WM_MOUSEMOVE events to the tooltip ctrl if we want it to
    // popup the tooltip bubbles
    if ( msg->message == WM_MOUSEMOVE )
    {
        // we should do it if one of window children has an associated tooltip
        // (and not just if the window has a tooltip itself)
        if ( wndThis->HasToolTips() )
            wxToolTip::RelayEvent((WXMSG *)msg);
    }
#endif // wxUSE_TOOLTIPS

    // allow the window to prevent certain messages from being
    // translated/processed (this is currently used by wxTextCtrl to always
    // grab Ctrl-C/V/X, even if they are also accelerators in some parent)
    if ( !wndThis->MSWShouldPreProcessMessage((WXMSG *)msg) )
    {
        return false;
    }

    // try translations first: the accelerators override everything
    for ( wnd = wndThis; wnd; wnd = wnd->GetParent() )
    {
        if ( wnd->MSWTranslateMessage((WXMSG *)msg))
            return true;

        // stop at first top level window, i.e. don't try to process the key
        // strokes originating in a dialog using the accelerators of the parent
        // frame - this doesn't make much sense
        if ( wnd->IsTopLevel() )
            break;
    }

    // now try the other hooks (kbd navigation is handled here)
    for ( wnd = wndThis; wnd; wnd = wnd->GetParent() )
    {
        if ( wnd->MSWProcessMessage((WXMSG *)msg) )
            return true;

        // also stop at first top level window here, just as above because
        // if we don't do this, pressing ESC on a modal dialog shown as child
        // of a modal dialog with wxID_CANCEL will cause the parent dialog to
        // be closed, for example
        if ( wnd->IsTopLevel() )
            break;
    }

    // no special preprocessing for this message, dispatch it normally
    return false;
}

// ----------------------------------------------------------------------------
// wxEventLoop running and exiting
// ----------------------------------------------------------------------------


// @UE3 2007-16-11: Unreal Engine integration.

int wxEventLoop::MainRun()
{
	// event loops are not recursive, you need to create another loop!
	wxCHECK_MSG( !IsRunning(), -1, _T("can't reenter a message loop") );

	// ProcessIdle() and Dispatch() below may throw so the code here should
	// be exception-safe, hence we must use local objects for all actions we
	// should undo
//	wxEventLoopActivator activate(ms_activeLoop);
	wxEventLoopActivator activate(wx_static_cast(wxEventLoop *, this));

	//wxRunningEventLoopCounter evtLoopCounter;


	// we must ensure that OnExit() is called even if an exception is thrown
	// from inside Dispatch() but we must call it from Exit() in normal
	// situations because it is supposed to be called synchronously,
	// wxModalEventLoop depends on this (so we can't just use ON_BLOCK_EXIT or
	// something similar here)
#if wxUSE_EXCEPTIONS
	for ( ;; )
	{
		try
		{
#error DONT_USE_EXCEPTIONS_WITH_UNREAL_ENGINE
#endif // wxUSE_EXCEPTIONS

			// this is the event loop itself
			for ( ;; )
			{
#if wxUSE_THREADS
				wxMutexGuiLeaveOrEnter();
#endif // wxUSE_THREADS

				// give them the possibility to do whatever they want
				OnNextIteration();


				// make certain we have a callback class
				wxCHECK(NULL != s_UnrealCallbacks, -1);
				do
				{
					if( wxTheApp )
					{
						wxTheApp->TickUnreal();
						wxTheApp->ProcessIdle();
					}
				} while( !Pending() && !s_UnrealCallbacks->IsRequestingExit() && !m_shouldExit );

				if ( m_shouldExit )
				{
					while( Pending() )
					{
						Dispatch();
					}
					s_UnrealCallbacks->SetRequestingExit(true);
					break;
				}

				while( Pending() )
				{
					if( !Dispatch() )
					{
						s_UnrealCallbacks->SetRequestingExit(true);
						break;
					}
				}                // give them the possibility to do whatever they want
				OnNextIteration();


				if( s_UnrealCallbacks->IsRequestingExit() == true )
				{
					break;
				}
			}

#if wxUSE_EXCEPTIONS
			// exit the outer loop as well
			break;
		}
		catch ( ... )
		{
			try
			{
				if ( !wxTheApp || !wxTheApp->OnExceptionInMainLoop() )
				{
					OnExit();
					break;
				}
				//else: continue running the event loop
			}
			catch ( ... )
			{
				// OnException() throwed, possibly rethrowing the same
				// exception again: very good, but we still need OnExit() to
				// be called
				OnExit();
				throw;
			}
		}
	}
#endif // wxUSE_EXCEPTIONS

	return m_exitcode;
}


// ----------------------------------------------------------------------------
// wxEventLoopManual customization
// ----------------------------------------------------------------------------

void wxEventLoop::OnNextIteration()
{
#if wxUSE_THREADS
    wxMutexGuiLeaveOrEnter();
#endif // wxUSE_THREADS
}

void wxEventLoop::WakeUp()
{
    ::PostMessage(NULL, WM_NULL, 0, 0);
}

// ----------------------------------------------------------------------------
// wxEventLoop message processing dispatching
// ----------------------------------------------------------------------------

bool wxEventLoop::Pending() const
{
    MSG msg;
    return ::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE) != 0;
}

bool wxEventLoop::Dispatch()
{
    wxCHECK_MSG( IsRunning(), false, _T("can't call Dispatch() if not running") );

    MSG msg;
    BOOL rc = ::GetMessage(&msg, (HWND) NULL, 0, 0);

    if ( rc == 0 )
    {
        // got WM_QUIT
        return false;
    }

    if ( rc == -1 )
    {
        // should never happen, but let's test for it nevertheless
        wxLogLastError(wxT("GetMessage"));

        // still break from the loop
        return false;
    }

#if wxUSE_THREADS
    wxASSERT_MSG( wxThread::IsMain(),
                  wxT("only the main thread can process Windows messages") );

    static bool s_hadGuiLock = true;
    static wxMsgList s_aSavedMessages;

    // if a secondary thread owning the mutex is doing GUI calls, save all
    // messages for later processing - we can't process them right now because
    // it will lead to recursive library calls (and we're not reentrant)
    if ( !wxGuiOwnedByMainThread() )
    {
        s_hadGuiLock = false;

        // leave out WM_COMMAND messages: too dangerous, sometimes
        // the message will be processed twice
        if ( !wxIsWaitingForThread() || msg.message != WM_COMMAND )
        {
            MSG* pMsg = new MSG(msg);
            s_aSavedMessages.Append(pMsg);
        }

        return true;
    }
    else
    {
        // have we just regained the GUI lock? if so, post all of the saved
        // messages
        //
        // FIXME of course, it's not _exactly_ the same as processing the
        //       messages normally - expect some things to break...
        if ( !s_hadGuiLock )
        {
            s_hadGuiLock = true;

            wxMsgList::compatibility_iterator node = s_aSavedMessages.GetFirst();
            while (node)
            {
                MSG* pMsg = node->GetData();
                s_aSavedMessages.Erase(node);

                ProcessMessage(pMsg);
                delete pMsg;

                node = s_aSavedMessages.GetFirst();
            }
        }
    }
#endif // wxUSE_THREADS

    ProcessMessage(&msg);

    return true;
}

