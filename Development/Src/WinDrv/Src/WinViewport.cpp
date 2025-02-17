/*=============================================================================
	WinViewport.cpp: FWindowsViewport code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "WinDrv.h"
#include "EngineUserInterfaceClasses.h"

#define WM_MOUSEWHEEL 0x020A

// From atlwin.h:
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)	((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	((int)(short)HIWORD(lParam))
#endif

/** If TRUE, FWindowsViewport::UpdateModifierState() will enqueue rather than process immediately. */
UBOOL GShouldEnqueueModifierStateUpdates = FALSE;

/** Settings for the game Window */
HWND GGameWindow = NULL;
UBOOL GGameWindowUsingStartupWindowProc = FALSE;	// Whether the game is using the startup window procedure
UBOOL GUseGameWindowUponResize = TRUE;				// Whether FWindowsViewport::Resize should use GGameWindow rather than create a new window
DWORD GGameWindowStyle = 0;
INT GGameWindowPosX = 0;
INT GGameWindowPosY = 0;
INT GGameWindowWidth = 100;
INT GGameWindowHeight = 100;

/** This variable reflects the latest status sent to us by WM_ACTIVATEAPP */
UBOOL GWindowActive = FALSE;

// It's for WTSUnRegisterSessionNotification() to get messages of changes in session state
#pragma comment( lib, "WtsApi32.lib" )

/**
 * This is client RECT for the window we most recently locked the mouse to (in screen coordinates).  Note that
 * it may not exactly match the rect returned by GetClipCursor, since that rect will be constrained to the
 * desktop area, while our client rect will not.
 *
 * The mouse lock rect is a shared system resource, so we should not try to unlock a mouse
 * if some other window wants it locked.
 */
RECT GMouseLockClientRect = { 0,0,0,0 };

#if WITH_IME
#pragma comment( lib, "Imm32.lib" )

UBOOL CheckIMESupport( HWND Window )
{
	HIMC Imc = ImmGetContext( Window );
	if( !Imc )
	{
		Imc = ImmCreateContext();
		if( Imc )	
		{
			ImmAssociateContext( Window, Imc ); 
		}
	}
	else
	{
		ImmReleaseContext( Window, Imc );
	}

	return( Imc != NULL );
}

void DestroyIME( HWND Window )
{
	HIMC Imc = ImmGetContext( Window );
	if( Imc )
	{
		ImmAssociateContext( Window, NULL );
		ImmDestroyContext( Imc );
	}
}
#endif

//
//	FWindowsViewport::FWindowsViewport
//
FWindowsViewport::FWindowsViewport(UWindowsClient* InClient,FViewportClient* InViewportClient,const TCHAR* InName,UINT InSizeX,UINT InSizeY,UBOOL InFullscreen,HWND InParentWindow,INT InPosX, INT InPosY)
	:	FViewport(InViewportClient)
	,	bUpdateModifierStateEnqueued( FALSE )
	,   bShouldResetMouseButtons( TRUE )
	,	PreventCapture(FALSE)
{
	Client					= InClient;

	Name					= InName;
	Window					= NULL;
	ParentWindow			= InParentWindow;

	// New MouseCapture/MouseLock API
	SystemMouseCursor		= MC_Arrow;
	bMouseLockRequested		= FALSE;
	bCapturingMouseInput	= FALSE;
	MouseLockClientRect.top = MouseLockClientRect.left = MouseLockClientRect.bottom = MouseLockClientRect.right = 0;

	// Old MouseCapture/MouseLock API
	bCapturingJoystickInput = 1;
	bLockingMouseToWindow	= 0;

#if WITH_IME
	bSupportsIME			= TRUE;
	CurrentIMESize			= 0;
#endif

	Minimized				= 0;
	Maximized				= 0;
	Resizing				= 0;
	bPerformingSize			= FALSE;
	
	LastMouseXEventTime		= 0;
	LastMouseYEventTime		= 0;

	Client->Viewports.AddItem(this);

	// if win positions are default, attempt to read from cmdline
	if (InPosX == -1)
	{
		Parse(appCmdLine(),TEXT("PosX="),InPosX);
	}
	if (InPosY == -1)
	{
		Parse(appCmdLine(),TEXT("PosY="),InPosY);
	}

	// Creates the viewport window.
	Resize(InSizeX,InSizeY,InFullscreen,InPosX,InPosY);

	// Set as active window.
	::SetActiveWindow(Window);

	// Set initial key state.
	for(UINT KeyIndex = 0;KeyIndex < 256;KeyIndex++)
	{
		if ( Client->KeyMapVirtualToName.HasKey(KeyIndex) )
		{
			KeyStates[KeyIndex] = ::GetKeyState(KeyIndex) & 0x8000;
		}
	}
}

FWindowsViewport::~FWindowsViewport()
{
}

//
//	FWindowsViewport::Destroy
//

void FWindowsViewport::Destroy()
{
	if (Window != NULL)
	{
		RECT ClipRect;
		UBOOL bClipRectValid = (::GetClipCursor( &ClipRect ) != 0);
		if( bClipRectValid )
		{
			// Release lock on mouse cursor rect if we had that
			if( GMouseLockClientRect.top == MouseLockClientRect.top &&
				GMouseLockClientRect.left == MouseLockClientRect.left &&
				GMouseLockClientRect.bottom == MouseLockClientRect.bottom &&
				GMouseLockClientRect.right == MouseLockClientRect.right )
			{
				MouseLockClientRect.top = MouseLockClientRect.left = MouseLockClientRect.bottom = MouseLockClientRect.right = 0;
				GMouseLockClientRect.top = GMouseLockClientRect.left = GMouseLockClientRect.bottom = GMouseLockClientRect.top = 0;
				::ClipCursor(NULL);
			}
		}
	}

	ViewportClient = NULL;

#if WITH_IME
	DestroyIME( Window );
#endif

	// Release the viewport RHI.
	UpdateViewportRHI(TRUE,0,0,FALSE);

	// Destroy the viewport's window.
	DestroyWindow(Window);
}

//
//	FWindowsViewport::Resize
//
void FWindowsViewport::Resize(UINT NewSizeX,UINT NewSizeY,UBOOL NewFullscreen,INT InPosX, INT InPosY)
{
	if( bPerformingSize )
	{
		debugf(NAME_Error,TEXT("bPerformingSize == 1, FWindowsViewport::Resize"));
		appDebugBreak();
		return;
	}

	// This makes sure that the Play From Here window in the editor can't go fullscreen
	if (NewFullscreen && appStricmp(*Name, PLAYWORLD_VIEWPORT_NAME) == 0)
	{
		return;
	}

	bPerformingSize			= TRUE;
	UBOOL bWasFullscreen	= IsFullscreen();
	UBOOL bUpdateWindow		= FALSE;

	// Make sure we're using a supported fullscreen resolution.
	if ( NewFullscreen && !ParentWindow )
	{
		RHIGetSupportedResolution(NewSizeX,NewSizeY);
	}

	DWORD	WindowStyle;
	UBOOL	bShowWindow		= TRUE;
	INT		WindowWidth		= NewSizeX;
	INT		WindowHeight	= NewSizeY;
	INT		WindowPosX		= InPosX;
	INT		WindowPosY		= InPosY;

	// Figure out window style
	if( ParentWindow )
	{
		WindowStyle			= WS_CHILD | WS_CLIPSIBLINGS;
		NewFullscreen		= 0;
	}
	else
	{
		if (NewFullscreen)
		{
			WindowStyle		= WS_POPUP | WS_SYSMENU;
		}
		else if (GIsGame)
		{
			WindowStyle		= WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER | WS_CAPTION;
		}
		else
		{
			WindowStyle		= WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION;
		}
	}

	// Adjust window size based on the styles
	{
		RECT  WindowRect;
		WindowRect.left		= 0;
		WindowRect.top		= 0;
		WindowRect.right	= NewSizeX;
		WindowRect.bottom	= NewSizeY;
		::AdjustWindowRect(&WindowRect,WindowStyle,0);
		WindowWidth			= WindowRect.right - WindowRect.left;
		WindowHeight		= WindowRect.bottom - WindowRect.top;
	}

	// Center the window, unless we're setting the position explicitly
	if( NewFullscreen )
	{
		WindowPosX		= 0;
		WindowPosY		= 0;
		WindowWidth		= NewSizeX;
		WindowHeight	= NewSizeY;
	}
	else if( Window == NULL )
	{
		// Obtain width and height of primary monitor.
		INT ScreenWidth		= ::GetSystemMetrics( SM_CXSCREEN );
		INT ScreenHeight	= ::GetSystemMetrics( SM_CYSCREEN );

		if (WindowPosX == -1)
		{
			if (!Parse(appCmdLine(), TEXT("WindowPosX="), WindowPosX))
			{
				WindowPosX = (ScreenWidth - WindowWidth) / 2;
			}
		}
		if (WindowPosY == -1)
		{
			if (!Parse(appCmdLine(), TEXT("WindowPosY="), WindowPosY))
			{
				WindowPosY = (ScreenHeight - WindowHeight) / 2;
			}
		}
	}
	else
	{
		RECT WindowRect;
		::GetWindowRect( Window, &WindowRect );
		WindowPosX = WindowRect.left;
		WindowPosY = WindowRect.top;
	}

	// If not a child window or fullscreen, clamp the position against the virtual screen.
	if( !ParentWindow && !NewFullscreen )
	{
		RECT ScreenRect;
		ScreenRect.left		= ::GetSystemMetrics( SM_XVIRTUALSCREEN );
		ScreenRect.top		= ::GetSystemMetrics( SM_YVIRTUALSCREEN );
		ScreenRect.right	= ::GetSystemMetrics( SM_CXVIRTUALSCREEN );
		ScreenRect.bottom	= ::GetSystemMetrics( SM_CYVIRTUALSCREEN );

		if ( (WindowPosX + WindowWidth) > ScreenRect.right )
		{
			WindowPosX = ScreenRect.right - WindowWidth;
		}
		if ( WindowPosX < ScreenRect.left )
		{
			WindowPosX = ScreenRect.left;
		}
		if ( (WindowPosY + WindowHeight) > ScreenRect.bottom )
		{
			WindowPosY = ScreenRect.bottom - WindowHeight;
		}
		if ( WindowPosY < ScreenRect.top )
		{
			WindowPosY = ScreenRect.top;
		}
	}

	if( Window == NULL )
	{
		// Reuse the pre-created game window?
		if ( GGameWindow && GUseGameWindowUponResize )
		{
			Window						= GGameWindow;
			GUseGameWindowUponResize	= FALSE;
			SetWindowText(Window, *Name);
			bShowWindow = FALSE;
		}
		else
		{
			// Create the window
			Window = CreateWindowEx( WS_EX_APPWINDOW, *Client->WindowClassName, *Name, WindowStyle, WindowPosX, WindowPosY, WindowWidth, WindowHeight, ParentWindow, NULL, hInstance, this );
		}
		verify( Window );
	}
	else
	{
		bUpdateWindow = TRUE;
	}

	// Is this the game window and it hasn't been displayed yet?
	if ( GGameWindow && Window == GGameWindow && GGameWindowUsingStartupWindowProc )
	{
		GGameWindowStyle	= WindowStyle;
		GGameWindowPosX		= WindowPosX;
		GGameWindowPosY		= WindowPosY;
		GGameWindowWidth	= WindowWidth;
		GGameWindowHeight	= WindowHeight;

		// D3D window proc hook doesn't work if we change it after the device has been created,
		// _IF_ we created D3D in fullscreen mode at startup.
		// This problem manifested itself by not being able to Alt-Tab properly (D3D didn't turn off WS_EX_TOPMOST).
		if ( NewFullscreen )
		{
			extern void appShowGameWindow();
			appShowGameWindow();
		}
	}

	// Fullscreen window?
	if ( NewFullscreen )
	{
		// Hide system cursor when we are in full screen mode.
		while ( ::ShowCursor(FALSE)>=0 );
	}

	// Switching to windowed?
	if ( bWasFullscreen && !NewFullscreen )
	{
		// Show system cursor when we switch to windowed mode.
		while ( ::ShowCursor(TRUE)<0 );

		// Update the render device FIRST (so that the window won't be clamped by the previous resolution).
		UpdateRenderDevice( NewSizeX, NewSizeY, NewFullscreen );

		// Update the window SECOND.
		if ( bUpdateWindow )
		{
			UpdateWindow( NewFullscreen, WindowStyle, WindowPosX, WindowPosY, WindowWidth, WindowHeight );
		}
	}
	else
	{
		// Update the window FIRST.
		if ( bUpdateWindow )
		{
			UpdateWindow( NewFullscreen, WindowStyle, WindowPosX, WindowPosY, WindowWidth, WindowHeight );
		}

		// Update the render device SECOND.
		UpdateRenderDevice( NewSizeX, NewSizeY, NewFullscreen );
	}

	// Show the viewport.
	if ( bShowWindow )
	{
		::ShowWindow( Window, SW_SHOW );
		::UpdateWindow( Window );
	}

	UpdateMouseLock();
	bPerformingSize = FALSE;
}

void FWindowsViewport::UpdateWindow( UBOOL NewFullscreen, DWORD WindowStyle, INT WindowPosX, INT WindowPosY, INT WindowWidth, INT WindowHeight )
{
	LONG CurrentWindowStyle = GetWindowLong(Window, GWL_STYLE);
	LONG CurrentWindowStyleEx = GetWindowLong(Window, GWL_EXSTYLE);

	// Don't change window style if we don't have to. Calling it will prevent proper refresh behind the window, which
	// looks bad if we're shrinking the window. Note also that we can't use exact equality, since CurrentWindowStyle may
	// contain extra bits, such as WS_CLIPSIBLINGS. At least this check will catch the four cases above.
	// Also, SWP_NOSENDCHANGING must not be set.
	UINT Flags = /*SWP_NOSENDCHANGING |*/ SWP_NOZORDER;
	if ( (CurrentWindowStyle & WindowStyle) != WindowStyle )
	{
		SetWindowLong(Window, GWL_STYLE, WindowStyle);
		Flags |= SWP_FRAMECHANGED;
	}

	if ( !ParentWindow && !NewFullscreen )
	{
	    HWND hWndInsertAfter = HWND_TOP;
	    if ( CurrentWindowStyleEx & WS_EX_TOPMOST )
	    {
		    // Turn off WS_EX_TOPMOST in window mode.
		    hWndInsertAfter = HWND_NOTOPMOST;
		    Flags &= ~SWP_NOZORDER;
	    }
	    ::SetWindowPos(Window, hWndInsertAfter, WindowPosX, WindowPosY, WindowWidth, WindowHeight, Flags);
	}
}

void FWindowsViewport::UpdateRenderDevice( UINT NewSizeX, UINT NewSizeY, UBOOL NewFullscreen )
{
	// Update the RHI with information about the largest viewport we currently have.  This helps
	// to reduce the chances of the rendering device being reset, especially during editor
	// startup and shutdown.
	{
		UINT LargestExpectedViewportWidth = NewSizeX;
		UINT LargestExpectedViewportHeight = NewSizeY;
		for( INT CurViewportIndex = 0; CurViewportIndex < Client->Viewports.Num(); ++CurViewportIndex )
		{
			FWindowsViewport* CurViewport = Client->Viewports( CurViewportIndex );

			// Skip the current viewport (since we already have the desired size)
			if( CurViewport != NULL && CurViewport != this )
			{
				const HWND CurViewportWindowHandle = ( HWND )( CurViewport->GetWindow() );
				if( CurViewportWindowHandle != NULL && CurViewport->ViewportClient != NULL )
				{
					RECT WindowClientRect;
					::GetClientRect( CurViewportWindowHandle, &WindowClientRect );

					UINT ViewportSizeX = WindowClientRect.right - WindowClientRect.left;
					UINT ViewportSizeY = WindowClientRect.bottom - WindowClientRect.top;

					LargestExpectedViewportWidth = Max( LargestExpectedViewportWidth, ViewportSizeX );
					LargestExpectedViewportHeight = Max( LargestExpectedViewportHeight, ViewportSizeY );
				}
			}
		}

		RHISetLargestExpectedViewportSize( LargestExpectedViewportWidth, LargestExpectedViewportHeight );
	}


	// Initialize the viewport's render device.
	if( NewSizeX && NewSizeY )
	{
		UpdateViewportRHI(FALSE,NewSizeX,NewSizeY,NewFullscreen);
		// Update system settings which actually saves the resolution
		GSystemSettings.SetResolution(NewSizeX, NewSizeY, NewFullscreen);
	}
	// #19088: Based on certain startup patterns, there can be a case when all viewports are destroyed, which in turn frees up the D3D device (which results in badness).
	// There are plans to fix the initialization code, but until then hack the known case when a viewport is deleted due to being resized to zero width or height.
	// (D3D does not handle the zero width or zero height case)
	else if( NewSizeX && !NewSizeY )
	{
		NewSizeY = 1;
		UpdateViewportRHI(FALSE,NewSizeX,NewSizeY,NewFullscreen);
	}
	else if( !NewSizeX && NewSizeY )
	{
		NewSizeX = 1;
		UpdateViewportRHI(FALSE,NewSizeX,NewSizeY,NewFullscreen);
	}
	// End hack
	else
	{
		UpdateViewportRHI(TRUE,NewSizeX,NewSizeY,NewFullscreen);
	}
}

//
//	FWindowsViewport::ShutdownAfterError - Minimalist shutdown.
//
void FWindowsViewport::ShutdownAfterError()
{
	if(Window)
	{
		::DestroyWindow(Window);
		Window = NULL;
	}
}

UBOOL FWindowsViewport::HasFocus() const
{
	HWND FocusWindow = ::GetFocus();
	return (FocusWindow == Window);
}

UBOOL FWindowsViewport::IsCursorVisible( ) const
{
	CURSORINFO CursorInfo;
	CursorInfo.cbSize = sizeof(CURSORINFO);
	UBOOL bIsVisible = (::GetCursorInfo( &CursorInfo ) != 0);
	bIsVisible = bIsVisible && UBOOL(CursorInfo.flags & CURSOR_SHOWING) && UBOOL(CursorInfo.hCursor != NULL);
	return bIsVisible;
}

void FWindowsViewport::ShowCursor( UBOOL bVisible )
{
	UBOOL bIsCursorVisible = IsCursorVisible();
	if ( bVisible && !bIsCursorVisible )
	{
		// Restore the old mouse position when we show the cursor.
		if ( PreCaptureMousePos.x >= 0 && PreCaptureMousePos.y >= 0 )
		{
			::SetCursorPos( PreCaptureMousePos.x, PreCaptureMousePos.y );
		}
		while ( ::ShowCursor(TRUE)<0 );
		PreCaptureMousePos.x = -1;
		PreCaptureMousePos.y = -1;
	}
	else if ( !bVisible && bIsCursorVisible )
	{
		while ( ::ShowCursor(FALSE)>=0 );

		// Remember the current mouse position when we hide the cursor.
		PreCaptureMousePos.x = -1;
		PreCaptureMousePos.y = -1;
		::GetCursorPos(&PreCaptureMousePos);
	}
}

void FWindowsViewport::CaptureMouse( UBOOL bCapture )
{
	HWND CaptureWindow = ::GetCapture();
	UBOOL bIsMouseCaptured = (CaptureWindow == Window);
	bCapturingMouseInput = bCapture;
	if ( bCapture && !bIsMouseCaptured )
	{
		::SetCapture( Window );
		UWindowsClient::FlushMouseInput();
	}
	else if ( !bCapture && bIsMouseCaptured )
	{
		::ReleaseCapture();
	}
}


void FWindowsViewport::UpdateMouseLock()
{
	// If we're the foreground window, let us decide whether the system cursor should be visible or not.
	UBOOL bIsForeground = (::GetForegroundWindow() == Window);
	UBOOL bIsSystemCursorVisible;
	if ( bIsForeground )
	{
		bIsSystemCursorVisible = (SystemMouseCursor != MC_None);
	}
	else
	{
		bIsSystemCursorVisible = IsCursorVisible();
	}

	RECT ClipRect;
	UBOOL bIsGameCursorVisible = (GEngine && GEngine->GameViewport && GEngine->GameViewport->bDisplayingUIMouseCursor);
	UBOOL bIsAnyCursorVisible = bIsSystemCursorVisible || bIsGameCursorVisible;
	UBOOL bClipRectValid = (::GetClipCursor( &ClipRect ) != 0);
	UBOOL bShouldMouseLock = HasFocus() && (IsFullscreen() || !bIsAnyCursorVisible || bMouseLockRequested);

	// @todo: The following doesn't handle a window with a mouse lock changing position or size.  Not a big deal
	// @todo: Doesn't handle a window being destroyed that had a mouse lock.  Uncommon.

	// Update mouse lock
	if ( bShouldMouseLock )
	{
		RECT ClientRect;
		::GetClientRect( Window, &ClientRect );
		::MapWindowPoints( Window, NULL, (POINT*)&ClientRect, 2 );

		if ( !bClipRectValid ||
			 ClientRect.top != GMouseLockClientRect.top ||
			 ClientRect.left != GMouseLockClientRect.left ||
			 ClientRect.bottom != GMouseLockClientRect.bottom ||
			 ClientRect.right != GMouseLockClientRect.right )
		{
			::ClipCursor( &ClientRect );

			// The rect we just set may have been clipped by the screen rect.
			UBOOL bClipRectValid = (::GetClipCursor( &ClipRect ) != 0);
			if ( bClipRectValid )
			{
				// NOTE: We store the ClientRect, not the ClipRect, since that's what we'll be testing against later!
				GMouseLockClientRect = ClientRect;
				MouseLockClientRect = ClientRect;
			}
			else
			{
				GMouseLockClientRect.top = GMouseLockClientRect.left = GMouseLockClientRect.bottom = GMouseLockClientRect.top = 0;
				MouseLockClientRect.top = MouseLockClientRect.left = MouseLockClientRect.bottom = MouseLockClientRect.right = 0;
				::ClipCursor(NULL);
			}
		}
	}
	else
	{
		// Is the current system lock rect the one _our_ viewport set? Only unlock if it is.
		if ( !bClipRectValid ||
			 (GMouseLockClientRect.top == MouseLockClientRect.top &&
			  GMouseLockClientRect.left == MouseLockClientRect.left &&
			  GMouseLockClientRect.bottom == MouseLockClientRect.bottom &&
			  GMouseLockClientRect.right == MouseLockClientRect.right) )
		{
			GMouseLockClientRect.top = GMouseLockClientRect.left = GMouseLockClientRect.bottom = GMouseLockClientRect.top = 0;
			MouseLockClientRect.top = MouseLockClientRect.left = MouseLockClientRect.bottom = MouseLockClientRect.right = 0;
			::ClipCursor( NULL );
		}
	}
}

UBOOL FWindowsViewport::UpdateMouseCursor( UBOOL bSetCursor )
{
	UBOOL bHandled = FALSE;
	HCURSOR NewCursor = NULL;
	if( IsFullscreen() )
	{
		bHandled = bSetCursor;
		SystemMouseCursor = MC_None;
	}
	else
	{
		INT		MouseX		= GetMouseX();
		INT		MouseY		= GetMouseY();
		SystemMouseCursor	= MC_Arrow;
		if(ViewportClient)
		{
			SystemMouseCursor = ViewportClient->GetCursor(this,MouseX,MouseY);
		}
		if( MouseX < 0 || MouseY < 0 || MouseX >= (INT)GetSizeX() || MouseY >= (INT)GetSizeY())
		{
			// Don't set the cursor if the mouse isn't in the client area.
			bHandled = FALSE;
		}
		else if ( SystemMouseCursor == MC_None && HasFocus() )
		{
			bHandled = bSetCursor;
		}
		else if(SystemMouseCursor == MC_NoChange)
		{
			// Intentionally do not set the cursor.
			return TRUE;
		}
		else
		{
			LPCTSTR	CursorResource = IDC_ARROW;
			switch(SystemMouseCursor)
			{
			    case MC_Arrow:				CursorResource = IDC_ARROW; break;
			    case MC_Cross:				CursorResource = IDC_CROSS; break;
			    case MC_SizeAll:			CursorResource = IDC_SIZEALL; break;
			    case MC_SizeUpRightDownLeft:CursorResource = IDC_SIZENESW; break;
			    case MC_SizeUpLeftDownRight:CursorResource = IDC_SIZENWSE; break;
			    case MC_SizeLeftRight:		CursorResource = IDC_SIZEWE; break;
			    case MC_SizeUpDown:			CursorResource = IDC_SIZENS; break;
			    case MC_Hand:				CursorResource = IDC_HAND; break;
			};
			NewCursor = ::LoadCursor(NULL, CursorResource);
			bHandled = bSetCursor;
		}
	}
	if ( bHandled )
	{
		::SetCursor( NewCursor );
	}
	return bHandled;
}

void FWindowsViewport::LockMouseToWindow(UBOOL bLock)
{
	bMouseLockRequested = bLock;
	UpdateMouseLock();
}

//
//	FWindowsViewport::CaptureJoystickInput
//
UBOOL FWindowsViewport::CaptureJoystickInput(UBOOL Capture)
{
	bCapturingJoystickInput	= Capture;

	return bCapturingJoystickInput;
}

//
//	FWindowsViewport::KeyState
//
UBOOL FWindowsViewport::KeyState(FName Key) const
{
	BYTE* VirtualKey = Client ? Client->KeyMapNameToVirtual.Find(Key) : NULL;
	if( VirtualKey )
	{
		return KeyStates[*VirtualKey];
	}
	else
	{
		return FALSE;
	}
}

//
//	FWindowsViewport::GetMouseX
//
INT FWindowsViewport::GetMouseX()
{
	POINT P;
	::GetCursorPos( &P );
	::ScreenToClient( Window, &P );
	return P.x;
}

//
//	FWindowsViewport::GetMouseY
//
INT FWindowsViewport::GetMouseY()
{
	POINT P;
	::GetCursorPos( &P );
	::ScreenToClient( Window, &P );
	return P.y;
}

void FWindowsViewport::GetMousePos( FIntPoint& MousePosition )
{
	POINT P;
	::GetCursorPos( &P );
	::ScreenToClient( Window, &P );
	MousePosition.X = P.x;
	MousePosition.Y = P.y;
}

void FWindowsViewport::SetMouse(INT x, INT y)
{
	PreCaptureMousePos.x = x;
	PreCaptureMousePos.y = y;
	::ClientToScreen(Window, &PreCaptureMousePos);
	::SetCursorPos(PreCaptureMousePos.x, PreCaptureMousePos.y);
}

//
//	FWindowsViewport::InvalidateDisplay
//

void FWindowsViewport::InvalidateDisplay()
{
	::InvalidateRect(Window,NULL,0);
}

FViewportFrame* FWindowsViewport::GetViewportFrame()
{
	return ParentWindow ? NULL : this;
}

//
//	FWindowsViewport::ProcessInput
//
void FWindowsViewport::ProcessInput( FLOAT DeltaTime )
{
	if( !ViewportClient )
	{
		return;
	}

#if WITH_PANORAMA
	extern UBOOL appIsPanoramaGuideOpen(void);
	/** If the Live guide is open, don't consume it's input */
	if (appIsPanoramaGuideOpen())
	{
		// Also pause any waveforms being played
		if ((bCapturingJoystickInput || ViewportClient->RequiresUncapturedAxisInput()) && HasFocus() )
		{
			XINPUT_VIBRATION Feedback = {0};
			for (INT JoystickIndex = 0; JoystickIndex < UWindowsClient::Joysticks.Num(); JoystickIndex++)
			{
				// This will turn off the vibration
				XInputSetState(JoystickIndex,&Feedback);
			}
		}
		return;
	}
#endif

	if( Client->bAllowJoystickInput && HasFocus() && (bCapturingJoystickInput || ViewportClient->RequiresUncapturedAxisInput()) )
	{
		for(INT JoystickIndex = 0;JoystickIndex < UWindowsClient::Joysticks.Num();JoystickIndex++)
		{
			DIJOYSTATE2 State;
			XINPUT_STATE XIstate;
			FJoystickInfo& JoystickInfo = UWindowsClient::Joysticks(JoystickIndex);
			UBOOL bIsConnected = FALSE;

			ZeroMemory( &XIstate, sizeof(XIstate) );
			ZeroMemory( &State, sizeof(State) );

			if ( JoystickInfo.DirectInput8Joystick )
			{
				// Focus issues with Viewports: Force gamepad input
				JoystickInfo.DirectInput8Joystick->Unacquire();
				JoystickInfo.DirectInput8Joystick->SetCooperativeLevel(GetForegroundWindow(),DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
				bIsConnected = ( SUCCEEDED(JoystickInfo.DirectInput8Joystick->Acquire()) && SUCCEEDED(JoystickInfo.DirectInput8Joystick->Poll()) );
				if( bIsConnected && SUCCEEDED(JoystickInfo.DirectInput8Joystick->GetDeviceState(sizeof(DIJOYSTATE2), &State)) )
				{
					// Store state
					JoystickInfo.PreviousState = State;
				}
				else
				{
					// if failed to get device state, use previous one.
					// this seems to happen quite frequently unfortunately :(
					State = JoystickInfo.PreviousState;
				}
			}
			else if ( JoystickIndex < JOYSTICK_NUM_XINPUT_CONTROLLERS )
			{
				// Simply get the state of the controller from XInput.
				if ( JoystickInfo.bIsConnected )
				{
					JoystickInfo.bIsConnected = ( XInputGetState( JoystickIndex, &XIstate ) == ERROR_SUCCESS ) ? TRUE : FALSE;
				}
				bIsConnected = JoystickInfo.bIsConnected;
			}

			if ( bIsConnected )
			{
				// see the UWindowsClient::UWindowsClient calls below for which slots in this array map to which names
				// 1 means pressed, 0 means not pressed
				UBOOL CurrentStates[MAX_JOYSTICK_BUTTONS] = {0};

				if( JoystickInfo.JoystickType == JOYSTICK_Xbox_Type_S )
				{
					// record our current button pressed state
					for (INT ButtonIndex = 0; ButtonIndex < 12; ButtonIndex++)
					{
						CurrentStates[ButtonIndex] = State.rgbButtons[ButtonIndex];
					}
					CurrentStates[12] = State.rgdwPOV[0] == 0;
					CurrentStates[13] = State.rgdwPOV[0] == 18000;
					CurrentStates[14] = State.rgdwPOV[0] == 27000;
					CurrentStates[15] = State.rgdwPOV[0] == 9000;

					// Axis, convert range 0..65536 set up in EnumAxesCallback to +/- 1.
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftX	, (State.lX  - 32768.f) / 32768.f, DeltaTime, TRUE );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftY	, -(State.lY  - 32768.f) / 32768.f, DeltaTime, TRUE );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightX	, (State.lRx - 32768.f) / 32768.f, DeltaTime, TRUE );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightY	, (State.lRy - 32768.f) / 32768.f, DeltaTime, TRUE );
				}
				// Support X360 controller
				else if( JoystickInfo.JoystickType == JOYSTICK_X360 )
				{
					CurrentStates[Client->X360ToXboxControllerMapping[0]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_A;
					CurrentStates[Client->X360ToXboxControllerMapping[1]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_B;
					CurrentStates[Client->X360ToXboxControllerMapping[2]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_X;
					CurrentStates[Client->X360ToXboxControllerMapping[3]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y;
					CurrentStates[Client->X360ToXboxControllerMapping[4]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
					CurrentStates[Client->X360ToXboxControllerMapping[5]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
					CurrentStates[Client->X360ToXboxControllerMapping[6]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
					CurrentStates[Client->X360ToXboxControllerMapping[7]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_START;
					CurrentStates[Client->X360ToXboxControllerMapping[8]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
					CurrentStates[Client->X360ToXboxControllerMapping[9]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;

					CurrentStates[Client->X360ToXboxControllerMapping[10]] = XIstate.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
					CurrentStates[Client->X360ToXboxControllerMapping[11]] = XIstate.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

					// record our current button pressed state
					CurrentStates[Client->X360ToXboxControllerMapping[12]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
					CurrentStates[Client->X360ToXboxControllerMapping[13]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
					CurrentStates[Client->X360ToXboxControllerMapping[14]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
					CurrentStates[Client->X360ToXboxControllerMapping[15]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

					// Axis, convert range -32768..+32767 set up in EnumAxesCallback to +/- 1.
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftX	, XIstate.Gamepad.sThumbLX / 32768.f, DeltaTime, TRUE );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftY	, XIstate.Gamepad.sThumbLY / 32768.f, DeltaTime, TRUE );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightX, XIstate.Gamepad.sThumbRX / 32768.f, DeltaTime, TRUE );
					// this needs to be inverted as the XboxTypeS axis are flipped from the "norm"
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightY, -XIstate.Gamepad.sThumbRY / 32768.f, DeltaTime, TRUE );

					if( GWorld->IsPaused() )
					{
						// Stop vibration when game is paused
						for(INT i=0; i<GEngine->GamePlayers.Num(); i++)
						{
							ULocalPlayer* LP = GEngine->GamePlayers(i);
							if(LP && LP->Actor && LP->Actor->ForceFeedbackManager)
							{
								LP->Actor->ForceFeedbackManager->bIsPaused = TRUE;
								if( LP->Actor->ForceFeedbackManager )
								{
									LP->Actor->ForceFeedbackManager->ApplyForceFeedback(JoystickIndex,DeltaTime);
								}
							}
						}
					}
					else
					{
					// Now update any waveform data
					UForceFeedbackManager* Manager = ViewportClient->GetForceFeedbackManager(JoystickIndex);
					if (Manager != NULL)
					{
						Manager->ApplyForceFeedback(JoystickIndex,DeltaTime);
					}
				}
				}
				else if( JoystickInfo.JoystickType == JOYSTICK_PS2_Old_Converter || JoystickInfo.JoystickType == JOYSTICK_PS2_New_Converter )
				{
					// PS2 controller has to be mapped funny, since we use Xbox button mapping below
					for (INT ButtonIndex = 0; ButtonIndex < 12; ButtonIndex++)
					{
						CurrentStates[Client->PS2ToXboxControllerMapping[ButtonIndex]] = State.rgbButtons[ButtonIndex];
					}

					CurrentStates[12] = State.rgdwPOV[0] == 0;
					CurrentStates[13] = State.rgdwPOV[0] == 18000;
					CurrentStates[14] = State.rgdwPOV[0] == 27000;
					CurrentStates[15] = State.rgdwPOV[0] == 9000;

					// Axis, convert range 0..65536 set up in EnumAxesCallback to +/- 1.
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftX			, (State.lX  - 32768.f) / 32768.f, DeltaTime, TRUE );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftY			, (State.lY  - 32768.f) / 32768.f, DeltaTime, TRUE );
					if( JoystickInfo.JoystickType == JOYSTICK_PS2_Old_Converter )
					{
						ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightX, (State.lRz - 32768.f) / 32768.f, DeltaTime, TRUE );
						ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightY, (State.lZ  - 32768.f) / 32768.f, DeltaTime, TRUE );
					}
					else
					{
						ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightX, (State.lZ  - 32768.f) / 32768.f, DeltaTime, TRUE );
						ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightY, (State.lRz - 32768.f) / 32768.f, DeltaTime, TRUE );
					}
				}

				const DOUBLE CurrentTime = appSeconds();

				// Buttons (treated as digital buttons).
				for (INT ButtonIndex = 0; ButtonIndex < MAX_JOYSTICK_BUTTONS; ButtonIndex++)
				{
					if (CurrentStates[ButtonIndex] != JoystickInfo.JoyStates[ButtonIndex])
					{
						ViewportClient->InputKey(this, JoystickInfo.ControllerId, Client->JoyNames[ButtonIndex], CurrentStates[ButtonIndex] ? IE_Pressed : IE_Released, 1.f, TRUE );
						if ( CurrentStates[ButtonIndex] != 0 )
						{
							// this button was pressed - set the button's NextRepeatTime to the InitialButtonRepeatDelay
							JoystickInfo.NextRepeatTime[ButtonIndex] = CurrentTime + Client->InitialButtonRepeatDelay;
						}
					}
					else if ( CurrentStates[ButtonIndex] != 0 && JoystickInfo.NextRepeatTime[ButtonIndex] <= CurrentTime )
					{
						// it is time to generate an IE_Repeat event for this joystick button
						ViewportClient->InputKey(this, JoystickInfo.ControllerId, Client->JoyNames[ButtonIndex], IE_Repeat, 1.f, TRUE );

						// set the button's NextRepeatTime to the ButtonRepeatDelay
						JoystickInfo.NextRepeatTime[ButtonIndex] = CurrentTime + Client->ButtonRepeatDelay;
					}
				}

				// update last frames state
				appMemcpy(JoystickInfo.JoyStates, CurrentStates, sizeof(JoystickInfo.JoyStates));
			}
		}
	}

	// Process keyboard input.

	for( UINT KeyIndex = 0;KeyIndex < 256;KeyIndex++ )
	{
		FName* KeyName = Client->KeyMapVirtualToName.Find(KeyIndex);

		if(  KeyName )
		{
			UBOOL NewKeyState = ::GetKeyState(KeyIndex) & 0x8000;

			// wxWindows does not let you tell the left and right shift apart, so the problem is pressing the right shift
			// results in a left shift event. Then this code looks to see if left shift is down, and it isn't, so it calls
			// a Released event straight away. This code means that both shift have to be up to generate a Release event.
			// This isn't perfect, but means you can use the right shift as a modifier in the editor etc.
			if(KeyIndex == VK_RSHIFT)
			{
				NewKeyState = NewKeyState || ::GetKeyState(VK_LSHIFT) & 0x8000;
			}
			else if(KeyIndex == VK_LSHIFT)
			{
				NewKeyState = NewKeyState || ::GetKeyState(VK_RSHIFT) & 0x8000;
			}


			if (*KeyName != KEY_LeftMouseButton 
			&&  *KeyName != KEY_RightMouseButton 
			&&  *KeyName != KEY_MiddleMouseButton 
			&&	*KeyName != KEY_ThumbMouseButton
			&&	*KeyName != KEY_ThumbMouseButton2)
			{
				if( !NewKeyState && KeyStates[KeyIndex] )
				{
					KeyStates[KeyIndex] = FALSE;
					ViewportClient->InputKey(this,0,*KeyName,IE_Released);
				}
			}
			else if ( GIsGame && KeyStates[KeyIndex] )
			{
				//@todo. The Repeat event should really be sent in both editor and game,
				// but the editor viewport input handlers need to be updated accordingly.
				// This would involve changing the lock mouse to windows functionality 
				// (which does not check the button for a press event, but simply 
				// captures if the mouse button state is true - which is the case for
				// repeats). It would also require verifying each function makes no 
				// assumptions about what event is passed in.
				ViewportClient->InputKey(this, 0, *KeyName, IE_Repeat);
			}
		}
	}

#if WITH_IME
	// Tick/init the input method editor
	bSupportsIME = CheckIMESupport( Window );
#endif
}

//
//	FWindowsViewport::HandlePossibleSizeChange
//
void FWindowsViewport::HandlePossibleSizeChange()
{
	// If ViewportClient has been cleared, the window is being destroyed, and size changes don't matter.
	if(ViewportClient)
	{
		RECT WindowClientRect;
		::GetClientRect( Window, &WindowClientRect );

		UINT NewSizeX = WindowClientRect.right - WindowClientRect.left;
		UINT NewSizeY = WindowClientRect.bottom - WindowClientRect.top;

		if(!IsFullscreen() && (NewSizeX != GetSizeX() || NewSizeY != GetSizeY()))
		{
			Resize( NewSizeX, NewSizeY, 0 );

			if(ViewportClient)
				ViewportClient->ReceivedFocus(this);
		}
	}
}

void UWindowsClient::DeferMessage(FWindowsViewport* Viewport,UINT Message,WPARAM wParam,LPARAM lParam)
{
	// Add the message to the queue.
	FDeferredMessage DeferredMessage;
	DeferredMessage.Viewport = Viewport;
	DeferredMessage.Message = Message;
	DeferredMessage.wParam = wParam;
	DeferredMessage.lParam = lParam;
	DeferredMessage.KeyStates.LeftControl = ::GetKeyState(VK_LCONTROL);
	DeferredMessage.KeyStates.RightControl = ::GetKeyState(VK_RCONTROL);
	DeferredMessage.KeyStates.LeftShift = ::GetKeyState(VK_LSHIFT);
	DeferredMessage.KeyStates.RightShift = ::GetKeyState(VK_RSHIFT);
	DeferredMessage.KeyStates.Menu = ::GetKeyState(VK_MENU);
	DeferredMessages.AddItem(DeferredMessage);
}

void UWindowsClient::ProcessDeferredMessages()
{
	for(INT MessageIndex = 0;MessageIndex < DeferredMessages.Num();MessageIndex++)
	{
		// Make a copy of the deferred message before we process it.  We need to do this because new messages
		// may be added to the DiferredMessages array while we're processing them!
		FDeferredMessage DeferredMessageCopy = DeferredMessages( MessageIndex );
		DeferredMessages(MessageIndex).Viewport->ProcessDeferredMessage( DeferredMessageCopy );
	}
	DeferredMessages.Empty();
}

//
//	FWindowsViewport::ViewportWndProc - Main viewport window function.
//
LONG FWindowsViewport::ViewportWndProc( UINT Message, WPARAM wParam, LPARAM lParam )
{
	// Process any enqueued UpdateModifierState requests.
	if ( bUpdateModifierStateEnqueued )
	{
		UpdateModifierState();
	}

	if( !Client->ProcessWindowsMessages || Client->Viewports.FindItemIndex(this) == INDEX_NONE )
	{
		return DefWindowProc(Window,Message,wParam,lParam);
	}

	// Message handler.
	switch(Message)
	{
	case WM_CLOSE:
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		Client->DeferMessage(this,Message,wParam,lParam);
		return 0;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Client->DeferMessage(this,Message,wParam,lParam);
		return 1;

	case WM_PAINT:
	case WM_SETCURSOR:
		// Painting and cursor updates only need to happen once per frame, so they replace previously deferred messages of the same type.
		Client->DeferMessage(this,Message,wParam,lParam);
		return DefWindowProc(Window,Message,wParam,lParam);

	case WM_MOUSEMOVE:
	case WM_CAPTURECHANGED:
	case WM_ACTIVATE:
	case WM_ENTERSIZEMOVE:
	case WM_EXITSIZEMOVE:
	case WM_SIZE:
	case WM_SIZING:
		Client->DeferMessage(this,Message,wParam,lParam);
		return DefWindowProc(Window,Message,wParam,lParam);

	case WM_ACTIVATEAPP:
		if( wParam == TRUE )
		{
			GWindowActive = TRUE;
		}
		else
		{
			GWindowActive = FALSE;
		}
		return DefWindowProc( Window, Message, wParam, lParam);

	case WM_DESTROY:
		Window = NULL;
		return 0;

	case WM_MOUSEACTIVATE:
		if(LOWORD(lParam) != HTCLIENT)
		{
			// The activation was the result of a mouse click outside of the client area of the window.
			// Prevent the mouse from being captured to allow the user to drag the window around.
			PreventCapture = 1;
		}
		Client->DeferMessage(this,Message,wParam,lParam);
		return MA_ACTIVATE;

	// Focus issues with Viewports: Viewports are owned by a wxPanel window (to wrap this plain Windows code into wxWidgets I guess),
	// which makes all these messages become processed by IsDialogMessage(). As part of that, we need to tell IsDialogMessage that
	// we want all WM_CHARS and don't let Windows beep for unprocessed chars in WM_KEYDOWN.
	// I never figured out why bound keys don't beep in WM_KEYDOWN (like spacebar) and un-bound keys beep... :(   -Smedis
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	case WM_DISPLAYCHANGE:
		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 160;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 120; 
		return DefWindowProc( Window, Message, wParam, lParam );

#if WITH_IME
	case WM_IME_COMPOSITION:
		{
			Client->DeferMessage(this,Message,wParam,lParam);
			if( lParam & GCS_RESULTSTR )
			{
				return 0;	
			}
			else if( lParam & GCS_COMPSTR ) 
			{
				return 0;
			}
			else
			{
				return DefWindowProc( Window, Message, wParam, lParam );
			}
		}
#endif

	case WM_ERASEBKGND:
		return 1;

	case WM_POWERBROADCAST:
		//Prevent power management
		switch( wParam )
		{
			case PBT_APMQUERYSUSPEND:
			case PBT_APMQUERYSTANDBY:
				return BROADCAST_QUERY_DENY;
		}
		return DefWindowProc( Window, Message, wParam, lParam );

	case WM_SYSCOMMAND:
		// Prevent moving/sizing and power loss in fullscreen mode.
		switch( wParam & 0xfff0 )
		{
			case SC_SCREENSAVE:
				return 1;

			case SC_MOVE:
			case SC_SIZE:
			case SC_MAXIMIZE:
			case SC_KEYMENU:
			case SC_MONITORPOWER:
				if( IsFullscreen() )
					return 1;
				break;
			case SC_RESTORE:
				if( IsIconic(Window) )
				{
					::ShowWindow(Window,SW_RESTORE);
					return 0;
				}
		}
		return DefWindowProc( Window, Message, wParam, lParam );

	case WM_NCHITTEST:
		// Prevent the user from selecting the menu in fullscreen mode.
		if( IsFullscreen() )
			return HTCLIENT;
		return DefWindowProc( Window, Message, wParam, lParam );

	case WM_POWER:
		if( PWR_SUSPENDREQUEST == wParam )
		{
			debugf( NAME_Log, TEXT("Received WM_POWER suspend") );
			return PWR_FAIL;
		}
		else
			debugf( NAME_Log, TEXT("Received WM_POWER") );
		return DefWindowProc( Window, Message, wParam, lParam );

	case WM_QUERYENDSESSION:
		debugf( NAME_Log, TEXT("Received WM_QUERYENDSESSION") );
		return DefWindowProc( Window, Message, wParam, lParam );

	case WM_ENDSESSION:
		if ( wParam )
		{
			debugf( NAME_Log, TEXT("Received WM_ENDSESSION") );
			appRequestExit( FALSE );
			return TRUE;
		}
		return DefWindowProc( Window, Message, wParam, lParam );

	default:
		return DefWindowProc( Window, Message, wParam, lParam );
	}
}

void FWindowsViewport::ProcessDeferredMessage(const FDeferredMessage& DeferredMessage)
{
	// Helper class to aid in sending callbacks, but still let each message return in their case statements
	class FCallbackHelper
	{
		FViewport* Viewport;
	public:
		FCallbackHelper(FViewport* InViewport, UINT InMessage) 
		:	Viewport( InViewport )
		{ 
			GCallbackEvent->Send( CALLBACK_PreWindowsMessage, Viewport, InMessage ); 
		}
		~FCallbackHelper() 
		{ 
			GCallbackEvent->Send( CALLBACK_PostWindowsMessage, Viewport, 0 ); 
		}
	};

	const WPARAM wParam = DeferredMessage.wParam;
	const LPARAM lParam = DeferredMessage.lParam;

	// Message handler.
	switch(DeferredMessage.Message)
	{
	case WM_CLOSE:
		if( ViewportClient )
			ViewportClient->CloseRequested(this);
		break;

	case WM_PAINT:
		if(ViewportClient)
			ViewportClient->RedrawRequested(this);
		break;

	case WM_ACTIVATE:
		switch (LOWORD(wParam))
		{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				if( GEngine != NULL )
				{
					GEngine->OnLostFocusPause(FALSE);
				}
				break;
			case WA_INACTIVE:
				if( GEngine != NULL )
				{
					GEngine->OnLostFocusPause(TRUE);
				}
				break;
		}
		break;

	case WM_MOUSEMOVE:
		if( !bCapturingMouseInput )
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);

			INT	X = GET_X_LPARAM(lParam);
			INT Y = GET_Y_LPARAM(lParam);

			if(ViewportClient)
			{
				ViewportClient->MouseMove(this,X,Y);
			}
		}
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);

			// Distinguish between left/right control/shift/alt.
			UBOOL ExtendedKey = (lParam & (1 << 24));
			UINT KeyCode = wParam;
			switch(wParam)
			{
				case VK_MENU:		
					KeyCode = ExtendedKey ? VK_RMENU : VK_LMENU;
					break;
				case VK_CONTROL:
				{
					//Extended information may not be supported for certain keyboards
					if( DeferredMessage.KeyStates.RightControl & 0x8000 )
					{
						KeyCode = VK_RCONTROL;
					}
					else if( (DeferredMessage.KeyStates.LeftControl & 0x8000) )
					{
						KeyCode = VK_LCONTROL;
					}
					break;
				}
				case VK_SHIFT:
				{
					//Extended information is not supported for the shift keys at all
					if( DeferredMessage.KeyStates.RightShift & 0x8000 )
					{
						KeyCode = VK_RSHIFT;
					}
					else if( DeferredMessage.KeyStates.LeftShift & 0x8000 )
					{
						KeyCode = VK_LSHIFT;
					}
					break;
				}
			};

			// Get key code.
			FName* Key = Client->KeyMapVirtualToName.Find(KeyCode);

			if(Key)
			{
				// Update the cached key state.
				UBOOL OldKeyState = KeyStates[KeyCode];
				KeyStates[KeyCode] = TRUE;

				// Send key to input system.
				if( *Key==KEY_Enter && (DeferredMessage.KeyStates.Menu & 0x8000) )
				{
					Resize(GetSizeX(),GetSizeY(),!IsFullscreen());
				}
				else if( ViewportClient )
				{
					ViewportClient->InputKey(this,0,*Key,OldKeyState ? IE_Repeat : IE_Pressed);
				}
			}
		}
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);

			// Distinguish between left/right control/shift/alt.
			UBOOL ExtendedKey = (lParam & (1 << 24));
			UINT KeyCode = wParam;
			switch(wParam)
			{
				case VK_MENU:
					KeyCode = ExtendedKey ? VK_RMENU : VK_LMENU;
					break;
				case VK_CONTROL:
				{
					if ( KeyStates[VK_RCONTROL] && !(DeferredMessage.KeyStates.RightControl & 0x8000) )
					{
						KeyCode = VK_RCONTROL;
					}
					else if ( KeyStates[VK_LCONTROL] && !(DeferredMessage.KeyStates.LeftControl & 0x8000) )
					{
						KeyCode = VK_LCONTROL;
					}
					break;
				}
				case VK_SHIFT:
				{
					if ( KeyStates[VK_RSHIFT] && !(DeferredMessage.KeyStates.RightShift & 0x8000) )
					{
						KeyCode = VK_RSHIFT;
					}
					else if ( KeyStates[VK_LSHIFT] && !(DeferredMessage.KeyStates.LeftShift & 0x8000) )
					{
						KeyCode = VK_LSHIFT;
					}
					break;
				}
			};

			// Get key code.
			FName* Key = Client->KeyMapVirtualToName.Find(KeyCode);

			if( !Key )
			{
				break;
			}

			// Update the cached key state.
			KeyStates[KeyCode] = FALSE;

			// Send key to input system.
			if( ViewportClient )
			{
				ViewportClient->InputKey(this,0,*Key,IE_Released);
			}
		}
		break;

	case WM_CHAR:
	case WM_SYSCHAR:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);

			TCHAR Character = wParam;
			if(ViewportClient)
			{
				ViewportClient->InputChar(this,0,Character);
			}
		}
		break;

	case WM_SETCURSOR:
		{
			UBOOL bHandled = UpdateMouseCursor( TRUE );
			if ( bHandled )
			{
				UpdateMouseLock();
			}
		}
		break;

	case WM_SETFOCUS:
		UWindowsClient::DirectInput8Mouse->Unacquire();
		UWindowsClient::DirectInput8Mouse->SetCooperativeLevel(Window,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		for(INT JoystickIndex = 0;JoystickIndex < UWindowsClient::Joysticks.Num();JoystickIndex++)
		{
			FJoystickInfo& JoystickInfo = UWindowsClient::Joysticks(JoystickIndex);
			if ( JoystickInfo.DirectInput8Joystick )
			{
				JoystickInfo.DirectInput8Joystick->Unacquire();
				JoystickInfo.DirectInput8Joystick->SetCooperativeLevel(Window,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			}
		}
		if(ViewportClient && !PreventCapture)
		{
			ViewportClient->ReceivedFocus(this);
		}
		if ( GFullScreenMovie )
		{
			GFullScreenMovie->Mute( FALSE );
		}
		UpdateMouseCursor( TRUE );
		UpdateModifierState();
		UpdateMouseLock();
		break;
	
	case WM_KILLFOCUS:
		if(ViewportClient)
		{
			ViewportClient->LostFocus(this);
		}
		if ( GFullScreenMovie )
		{
			GFullScreenMovie->Mute( TRUE );
		}
		LockMouseToWindow( FALSE );
		if ( bCapturingMouseInput )
		{
			CaptureMouse( FALSE );
			OnMouseButtonUp( WM_LBUTTONUP, 0 );
			OnMouseButtonUp( WM_MBUTTONUP, 0 );
			OnMouseButtonUp( WM_RBUTTONUP, 0 );
			OnMouseButtonUp( WM_XBUTTONUP, 0 );
			OnMouseButtonUp( WM_XBUTTONUP, 0 ); // Note: need two XBUTTONUP (for both xbuttons)
		}

		// Make sure our mouse button bits get reset to the 'up' state after losing focus
		bShouldResetMouseButtons = TRUE;

		break;

	case WM_CAPTURECHANGED:
		if ( bCapturingMouseInput )
		{
			LockMouseToWindow( FALSE );
			CaptureMouse( FALSE );
			OnMouseButtonUp( WM_LBUTTONUP, 0 );
			OnMouseButtonUp( WM_MBUTTONUP, 0 );
			OnMouseButtonUp( WM_RBUTTONUP, 0 );
			OnMouseButtonUp( WM_XBUTTONUP, 0 );
			OnMouseButtonUp( WM_XBUTTONUP, 0 ); // Note: need two XBUTTONUP (for both xbuttons)
		}
		break;

	case WM_MOUSEACTIVATE:
		{
			// Reset mouse buttons back to 'up' if we need to
			ConditionallyResetMouseButtons();
		}
		break;

#if WITH_IME
	case WM_IME_COMPOSITION:
		{
			HIMC Imc = ImmGetContext( Window );
			if( !Imc )
			{
				appErrorf( TEXT( "No IME context" ) );
			}

			if( lParam & GCS_RESULTSTR )
			{
				// Get the size of the result string.
				INT Size = ImmGetCompositionString( Imc, GCS_RESULTSTR, NULL, 0 );

				TCHAR* String = new TCHAR[Size + 1];
				appMemzero( String, sizeof( TCHAR ) * ( Size + 1 ) );

				// Get the result strings that is generated by IME.
				Size = ImmGetCompositionString( Imc, GCS_RESULTSTR, String, Size );
				Size /= sizeof( TCHAR );

				for( INT i = 0; i < CurrentIMESize; i++ )
				{
					ViewportClient->InputKey( this, 0, KEY_BackSpace, IE_Pressed );
					ViewportClient->InputKey( this, 0, KEY_BackSpace, IE_Released );
				}

				for( INT i = 0; i < Size; i++ )
				{
					INT Key = String[i];
					if( Key )
					{
						ViewportClient->InputChar( this, 0, Key );
					}
				}

				delete [] String;

				ImmReleaseContext( Window, Imc );

				CurrentIMESize = 0;
			}
			else if( lParam & GCS_COMPSTR ) 
			{
				// Get the size of the result string.
				INT Size = ImmGetCompositionString( Imc, GCS_COMPSTR, NULL, 0 );

				TCHAR* String = new TCHAR[Size + 1];
				appMemzero( String, sizeof( TCHAR ) * ( Size + 1 ) );

				// Get the result strings that is generated by IME.
				Size = ImmGetCompositionString( Imc, GCS_COMPSTR, String, Size );
				Size /= sizeof( TCHAR );

				for( INT i = 0; i < CurrentIMESize; i++ )
				{
					ViewportClient->InputKey( this, 0, KEY_BackSpace, IE_Pressed );
					ViewportClient->InputKey( this, 0, KEY_BackSpace, IE_Released );
				}

				for( INT i = 0; i < Size; i++ )
				{
					INT Key = String[i];
					if( Key )
					{
						ViewportClient->InputChar( this, 0, Key );
					}
				}

				delete [] String;

				ImmReleaseContext( Window, Imc );

				CurrentIMESize = Size;
			}

		}
		break;
#endif
		
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
		{		
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);
			OnMouseButtonDoubleClick( DeferredMessage.Message, wParam );
		}			
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);
			CaptureMouse( TRUE );
			OnMouseButtonDown( DeferredMessage.Message, wParam );
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);
			UBOOL bAnyButtonDown = FALSE;
			switch (DeferredMessage.Message)
			{
				case WM_LBUTTONUP: bAnyButtonDown = KeyState(KEY_RightMouseButton) || KeyState(KEY_MiddleMouseButton) || KeyState(KEY_ThumbMouseButton); break;
				case WM_RBUTTONUP: bAnyButtonDown = KeyState(KEY_LeftMouseButton) || KeyState(KEY_MiddleMouseButton) || KeyState(KEY_ThumbMouseButton); break;
				case WM_MBUTTONUP: bAnyButtonDown = KeyState(KEY_LeftMouseButton) || KeyState(KEY_RightMouseButton) || KeyState(KEY_ThumbMouseButton); break;
				case WM_XBUTTONUP:
					{
						UBOOL bXButton1 = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) ? TRUE : FALSE;	// Is XBUTTON1 currently pressed?
						UBOOL bXButton2 = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) ? TRUE : FALSE;	// Is XBUTTON2 currently pressed?
						if ( KeyState(KEY_ThumbMouseButton) && !bXButton1 )								// Did XBUTTON1 get released?
						{
							bAnyButtonDown = KeyState(KEY_LeftMouseButton) || KeyState(KEY_RightMouseButton) || KeyState(KEY_MiddleMouseButton) || KeyState(KEY_ThumbMouseButton2);
						}
						else
						{
							bAnyButtonDown = KeyState(KEY_LeftMouseButton) || KeyState(KEY_RightMouseButton) || KeyState(KEY_MiddleMouseButton) || KeyState(KEY_ThumbMouseButton);
						}
					}
					break;
			}
			if ( !bAnyButtonDown )
			{
				CaptureMouse( FALSE );
			}
			OnMouseButtonUp( DeferredMessage.Message, wParam );
		}
		break;
		
	case WM_ENTERSIZEMOVE:
		Resizing = TRUE;
		break;

	case WM_EXITSIZEMOVE:
		Resizing = FALSE;
		HandlePossibleSizeChange();
		break;

	case WM_SIZE:
		if ( !bPerformingSize )
		{
			// Show mouse cursor if we're being minimized.
			if( SIZE_MINIMIZED == wParam )
			{
				Minimized = TRUE;
				Maximized = FALSE;
			}
			else if( SIZE_MAXIMIZED == wParam )
			{
				Minimized = FALSE;
				Maximized = TRUE;
				HandlePossibleSizeChange();
			}
			else if( SIZE_RESTORED == wParam )
			{
				if( Maximized )
				{
					Maximized = FALSE;
					HandlePossibleSizeChange();
				}
				else if( Minimized)
				{
					Minimized = FALSE;
					HandlePossibleSizeChange();
				}
				else
				{
					// Game:
					// If we're neither maximized nor minimized, the window size 
					// is changing by the user dragging the window edges.  In this 
					// case, we don't reset the device yet -- we wait until the 
					// user stops dragging, and a WM_EXITSIZEMOVE message comes.
					if(!Resizing)
						HandlePossibleSizeChange();
				}
			}
		}
		break;

	case WM_SIZING:
		// Flush the rendering command queue to ensure that there aren't pending viewport draw commands for the old viewport size.
		FlushRenderingCommands();
		break;
	};
}

void FWindowsViewport::Tick(FLOAT DeltaTime)
{
	// Update the mouse lock every frame.
	UpdateMouseLock();
}


/**
 * Resets mouse buttons if we were asked to by a message handler
 */
void FWindowsViewport::ConditionallyResetMouseButtons()
{
	if( bShouldResetMouseButtons )
	{
		KeyStates[ VK_LBUTTON ] = FALSE;
		KeyStates[ VK_RBUTTON ] = FALSE;
		KeyStates[ VK_MBUTTON ] = FALSE;
	}

	bShouldResetMouseButtons = false;
}

/*
 * Resends the state of the modifier keys (Ctrl, Shift, Alt).
 * Used when receiving focus, otherwise these keypresses may
 * be lost to some other process in the system.
 */
void FWindowsViewport::UpdateModifierState()
{
	if ( GShouldEnqueueModifierStateUpdates )
	{
		bUpdateModifierStateEnqueued = TRUE;
	}
	else
	{
		// Clear any enqueued UpdateModifierState requests.
		bUpdateModifierStateEnqueued = FALSE;
		UpdateModifierState( VK_LCONTROL );
		UpdateModifierState( VK_RCONTROL );
		UpdateModifierState( VK_LSHIFT );
		UpdateModifierState( VK_RSHIFT );
		UpdateModifierState( VK_LMENU );
		UpdateModifierState( VK_RMENU );

		// Reset the mouse button states if we need to.  In the case of receiving focus
		// via a mouse click, this will have already happened through the MOUSEACTIVATE pathway.
		ConditionallyResetMouseButtons();
	}
}

/*
 * Resends the state of the specified key to the viewport client.
 * It would've been nice to call InputKey only if the current state differs from what 
 * FEditorLevelViewportClient::Input thinks, but I can't access that here... :/
 */
void FWindowsViewport::UpdateModifierState( INT VirtKey )
{
	if ( !ViewportClient || !Client )
	{
		return;
	}

	FName* Key = Client->KeyMapVirtualToName.Find( VirtKey );
	if (!Key)
	{
		return;
	}

	UBOOL bDown = (::GetKeyState(VirtKey) & 0x8000) ? TRUE : FALSE;
	UBOOL bChangedState = (KeyStates[VirtKey] != bDown);
	KeyStates[VirtKey] = bDown;
	if ( bChangedState )
	{
		ViewportClient->InputKey(this, 0, *Key, bDown ? IE_Pressed : IE_Released );
	}
}

void FWindowsViewport::OnMouseButtonDoubleClick( UINT Message, WPARAM wParam )
{
	// Note: When double-clicking, the following message sequence is sent:
	//	WM_*BUTTONDOWN
	//	WM_*BUTTONUP
	//	WM_*BUTTONDBLCLK	(Needs to set the KeyStates[*] to TRUE)
	//	WM_*BUTTONUP

	FName Key;
	if ( Message == WM_LBUTTONDBLCLK )
		Key = KEY_LeftMouseButton;
	else if ( Message == WM_MBUTTONDBLCLK )
		Key = KEY_MiddleMouseButton;
	else if ( Message == WM_RBUTTONDBLCLK )
		Key = KEY_RightMouseButton;
	else if ( Message == WM_XBUTTONDBLCLK )
		Key = ( GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ) ? KEY_ThumbMouseButton : KEY_ThumbMouseButton2;

	if ( Client )
	{
		BYTE* KeyIndex = Client->KeyMapNameToVirtual.Find(Key);
		if ( KeyIndex )
		{
			KeyStates[*KeyIndex] = TRUE;
		}
	}

	if(ViewportClient)
	{
		ViewportClient->InputKey(this,0,Key,IE_DoubleClick);
	}
}

void FWindowsViewport::OnMouseButtonDown( UINT Message, WPARAM wParam )
{
	FName Key;
	switch ( Message )
	{
		case WM_LBUTTONDOWN:
			Key = KEY_LeftMouseButton;
			break;
		case WM_MBUTTONDOWN:
			Key = KEY_MiddleMouseButton;
			break;
		case WM_RBUTTONDOWN:
			Key = KEY_RightMouseButton;
			break;
		case WM_XBUTTONDOWN:
			{
				UBOOL bXButton1 = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) ? TRUE : FALSE;	// Is XBUTTON1 currently pressed?
				UBOOL bXButton2 = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) ? TRUE : FALSE;	// Is XBUTTON2 currently pressed?
				if ( !KeyState(KEY_ThumbMouseButton) && bXButton1 )								// Did XBUTTON1 get pressed?
				{
					Key = KEY_ThumbMouseButton;
				}
				else
				{
					Key = KEY_ThumbMouseButton2;
				}
				break;
			}
		default:
			return;	// Unsupported Message
	}

	if ( Client )
	{
		BYTE* KeyIndex = Client->KeyMapNameToVirtual.Find(Key);
		if ( KeyIndex )
		{
			KeyStates[*KeyIndex] = TRUE;
		}
	}

	if(ViewportClient)
	{
		::SetFocus( Window );			// Focus issues with Viewports: Force focus on Window
		ViewportClient->InputKey(this,0,Key,IE_Pressed);
	}
}

void FWindowsViewport::OnMouseButtonUp( UINT Message, WPARAM wParam )
{
	FName Key;
	switch ( Message )
	{
		case WM_LBUTTONUP:
			Key = KEY_LeftMouseButton;
			break;
		case WM_MBUTTONUP:
			Key = KEY_MiddleMouseButton;
			break;
		case WM_RBUTTONUP:
			Key = KEY_RightMouseButton;
			break;
		case WM_XBUTTONUP:
		{
			UBOOL bXButton1 = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) ? TRUE : FALSE;	// Is XBUTTON1 currently pressed?
			UBOOL bXButton2 = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) ? TRUE : FALSE;	// Is XBUTTON2 currently pressed?
			if ( KeyState(KEY_ThumbMouseButton) && !bXButton1 )								// Did XBUTTON1 get released?
			{
				Key = KEY_ThumbMouseButton;
			}
			else
			{
				Key = KEY_ThumbMouseButton2;
			}
			break;
		}
		default:
			return;	// Unsupported Message
	}

	// allow mouse capture to resume after resizing
	PreventCapture = 0;

	UBOOL bChangedState = TRUE;
	if ( Client )
	{
		BYTE* KeyIndex = Client->KeyMapNameToVirtual.Find(Key);
		if ( KeyIndex )
		{
			bChangedState = KeyStates[*KeyIndex];
			KeyStates[*KeyIndex] = FALSE;
		}
	}

	if ( ViewportClient && bChangedState )
	{
		ViewportClient->InputKey(this,0,Key,IE_Released);
	}
}
