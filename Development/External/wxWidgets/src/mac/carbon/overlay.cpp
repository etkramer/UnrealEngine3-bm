/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/overlay.cpp
// Purpose:     common wxOverlay code
// Author:      Stefan Csomor
// Modified by:
// Created:     2006-10-20
// RCS-ID:      $Id: overlay.cpp 43773 2006-12-03 18:56:25Z VZ $
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

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

#include "wx/overlay.h"

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
#endif

#include "wx/private/overlay.h"

#ifdef wxHAS_NATIVE_OVERLAY

// ============================================================================
// implementation
// ============================================================================

wxOverlayImpl::wxOverlayImpl()
{
    m_window = NULL ;
    m_overlayContext = NULL ;
    m_overlayWindow = NULL ;
}

wxOverlayImpl::~wxOverlayImpl()
{
    Reset();
}

bool wxOverlayImpl::IsOk()
{
    return m_overlayWindow != NULL ;
}

void wxOverlayImpl::MacGetBounds( Rect *bounds )
{
    int x, y;
    x=y=0;
    m_window->MacWindowToRootWindow( &x , &y ) ;
    WindowRef window = (WindowRef) m_window->MacGetTopLevelWindowRef() ;

    Point localwhere = { y, x };
    wxMacLocalToGlobal( window, &localwhere ) ;

    bounds->top = localwhere.v+m_y;
    bounds->left = localwhere.h+m_x;
    bounds->bottom = localwhere.v+m_y+m_height;
    bounds->right = localwhere.h+m_x+m_width;
}

OSStatus wxOverlayImpl::CreateOverlayWindow()
{
    OSStatus err;

    WindowAttributes overlayAttributes  = kWindowIgnoreClicksAttribute;

    if ( m_window )
    {
        m_overlayParentWindow =(WindowRef) m_window->MacGetTopLevelWindowRef();

        Rect bounds ;
        MacGetBounds(&bounds);
        err  = CreateNewWindow( kOverlayWindowClass, overlayAttributes, &bounds, &m_overlayWindow );
        if ( err == noErr )
        {
            SetWindowGroup( m_overlayWindow, GetWindowGroup(m_overlayParentWindow));    //  Put them in the same group so that their window layers are consistent
        }
    }
    else
    {
        m_overlayParentWindow = NULL ;
        CGRect cgbounds ;
        cgbounds = CGDisplayBounds(CGMainDisplayID());
        Rect bounds;
        bounds.top = (short)cgbounds.origin.y;
        bounds.left = (short)cgbounds.origin.x;
        bounds.bottom = (short)(bounds.top + cgbounds.size.height);
        bounds.right = (short)(bounds.left  + cgbounds.size.width);
        err  = CreateNewWindow( kOverlayWindowClass, overlayAttributes, &bounds, &m_overlayWindow );
    }
    ShowWindow(m_overlayWindow);
    return err;
}

void wxOverlayImpl::Init( wxWindowDC* dc, int x , int y , int width , int height )
{
    wxASSERT_MSG( !IsOk() , _("You cannot Init an overlay twice") );

    m_window = dc->GetWindow();
    m_x = x ;
    m_y = y ;
    if ( dc->IsKindOf( CLASSINFO( wxClientDC ) ))
    {
        wxPoint origin = m_window->GetClientAreaOrigin();
        m_x += origin.x;
        m_y += origin.y;
    }
    m_width = width ;
    m_height = height ;

    OSStatus err = CreateOverlayWindow();
    wxASSERT_MSG(  err == noErr , _("Couldn't create the overlay window") );
#ifndef __LP64__
    err = QDBeginCGContext(GetWindowPort(m_overlayWindow), &m_overlayContext);
#endif
    CGContextTranslateCTM( m_overlayContext, 0, m_height );
    CGContextScaleCTM( m_overlayContext, 1, -1 );
    CGContextTranslateCTM( m_overlayContext, -m_x , -m_y );
    wxASSERT_MSG(  err == noErr , _("Couldn't init the context on the overlay window") );
}

void wxOverlayImpl::BeginDrawing( wxWindowDC* dc)
{
    dc->SetGraphicsContext( wxGraphicsContext::CreateFromNative( m_overlayContext ) );
    wxSize size = dc->GetSize() ;
    dc->SetClippingRegion( 0 , 0 , size.x , size.y ) ;
}

void wxOverlayImpl::EndDrawing( wxWindowDC* dc)
{
    dc->SetGraphicsContext(NULL);
    CGContextSynchronize( m_overlayContext );
}

void wxOverlayImpl::Clear(wxWindowDC* dc)
{
    wxASSERT_MSG( IsOk() , _("You cannot Clear an overlay that is not inited") );
    CGRect box  = CGRectMake( m_x - 1, m_y - 1 , m_width + 2 , m_height + 2 );
    CGContextClearRect( m_overlayContext, box );
}

void wxOverlayImpl::Reset()
{
    if ( m_overlayContext )
    {
#ifndef __LP64__
        OSStatus err = QDEndCGContext(GetWindowPort(m_overlayWindow), &m_overlayContext);
        wxASSERT_MSG(  err == noErr , _("Couldn't end the context on the overlay window") );
#endif
        m_overlayContext = NULL ;
    }

    // todo : don't dispose, only hide and reposition on next run
    if (m_overlayWindow)
    {
        DisposeWindow(m_overlayWindow);
        m_overlayWindow = NULL ;
    }
}

#endif // wxHAS_NATIVE_OVERLAY
