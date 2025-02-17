/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#define WIN32_LEAN_AND_MEAN
#include "Core.h"

#if WITH_UE3_NETWORKING

#if WITH_PANORAMA && !CONSOLE

#include <WinLive.h>

/** Global used to determine if Panoram hooks are turned on */
UBOOL GIsUsingParorama = FALSE;
/** This will be FALSE for dedicated servers and when disabled */
UBOOL GPanoramaNeedsRendering = FALSE;
/** Holds the HWND that we render to and that Live is using as the main window */
HWND GPanoramaWindowHandle = NULL;
/** Whether the Panorama UI is rendering & intercepting input */
UBOOL GIsPanoramaUIOpen = FALSE;

/**
 * Stubbed out function when not enabled
 *
 * @param Device the device that is being used
 * @param Params the display parameters that are being used
 *
 * @return TRUE if it could initialize, FALSE otherwise
 */
void appPanoramaRenderHookInit(IUnknown* Device,void* PresentParams,HWND DeviceWindow)
{
	// Only init Live in the game
	if (!GIsEditor)
	{
		XLIVE_INITIALIZE_INFO xii = {0};
		xii.cbSize = sizeof(XLIVE_INITIALIZE_INFO);
		// These will be NULL for dedicated servers
		xii.pD3D = Device;
		xii.pD3DPP = PresentParams;
		// Don't hook up the rendering if this is a dedicated server
		if (!GIsServer && !GIsUCC)
		{
			// Need to be able to show the guide
			GPanoramaNeedsRendering = TRUE;
		}
		else
		{
			// Use command line parameters for logging on rather than default Live account
			xii.dwFlags = XLIVE_INITFLAG_NO_AUTO_LOGON;
		}
		// Let Panorama initialize and log the error code if it fails
		HRESULT hr = XLiveInitialize(&xii);
		if (SUCCEEDED(hr))
		{
			GIsUsingParorama = TRUE;
			// Set the window handle for code that needs to pass it to Live later
			GPanoramaWindowHandle = DeviceWindow;
			// Try to init the socket layer
			appSocketInit();
		}
		else
		{
			if (hr == E_DEBUGGER_PRESENT)
			{
				debugf(NAME_Error,
					TEXT("Shutting down because the debugger is present"));
				appMsgf(AMT_OK,
					*LocalizeError(TEXT("DebuggerPresentError"),TEXT("Engine")));
				appRequestExit(0);
			}
			else
			{
				debugf(NAME_Error,
					TEXT("XLiveInitialize() failed with 0x%08X (0x%08X)"),
					hr,
					XGetOverlappedExtendedError(NULL));
			}
		}
	}
}

/**
 * Initializes the Panorama hook without rendering support
 */
void appPanoramaHookInit(void)
{
	appPanoramaRenderHookInit(NULL,NULL,NULL);
}

/**
 * Deinitializes Panorama rendering shit
 */
void appPanoramaHookDeviceDestroyed(void)
{
	XLiveOnDestroyDevice();
}

/**
 * Allows the Panorama hook a chance to reset any resources
 *
 * @param PresentParameters the parameters used for the display
 */
void appPanoramaRenderHookReset(void* PresentParameters)
{
	if (GIsUsingParorama && GPanoramaNeedsRendering)
	{
		HRESULT hr = XLiveOnResetDevice(PresentParameters);
		if (FAILED(hr))
		{
			debugf(NAME_Error,
				TEXT("XLiveOnResetDevice() failed with 0x%08X (0x%08X)"),
				hr,
				XGetOverlappedExtendedError(NULL));
		}
	}
}

/**
 * Allows the Live Guide to render its UI
 */
void appPanoramaRenderHookRender(void)
{
	// This is off in the editor and UCC
	if (GIsUsingParorama && GPanoramaNeedsRendering)
	{
		XLiveRender();
	}
}

/**
 * Tears down the Panorama hook mechanism
 */
void appPanoramaHookUninitialize(void)
{
	if (GIsUsingParorama)
	{
		XLiveUnInitialize();
		GIsUsingParorama = GPanoramaNeedsRendering = FALSE;
	}
}

/**
 * Allows the Live Guide to intercept windows messages and process them
 *
 * @param hWnd the window handle of the window that is to process the message
 * @param Message the message to process
 * @param wParam the word parameter to the message
 * @param lParam the long parameter to the message
 * @param Return the return value if Live handled it
 *
 * @return TRUE if the app should process the message, FALSE if the guide processed it
 */
UBOOL appPanoramaInputHook(HWND hWnd,UINT Message,UINT wParam,LONG lParam,LONG& Return)
{
	UBOOL bNeedsProcessing = TRUE;
	// This is off in the editor and UCC
	if (GIsUsingParorama)
	{
		XLIVE_INPUT_INFO xii = {0};
	    xii.cbSize = sizeof(XLIVE_INPUT_INFO);
		// Init with the message parameters
		xii.hWnd = hWnd;
		xii.uMsg = Message;
		xii.wParam = wParam;
		xii.lParam = lParam;
		// Allow the Live guide to process the message
		HRESULT hr = XLiveInput(&xii);
		if (SUCCEEDED(hr))
		{
			// Only process if the Guide didn't
			bNeedsProcessing = !xii.fHandled;
			Return = xii.lRet;
		}
		else
		{
			debugf(NAME_Error,
				TEXT("XLiveInput() failed with 0x%08X (0x%08X)"),
				hr,
				XGetOverlappedExtendedError(NULL));
		}
	}
	return bNeedsProcessing;
}

/**
 * Returns the window handle that Live was configured to use
 *
 * @return the window handle that Live was told to use
 */
HWND appPanoramaHookGetHWND(void)
{
	return GPanoramaWindowHandle;
}

/**
 * Determines whether the Live UI is showing
 *
 * @return TRUE if the guide is open, FALSE otherwise
 */
UBOOL appIsPanoramaGuideOpen(void)
{
	return GIsPanoramaUIOpen;
}

/**
 * Allow Live to prefilter any messages for IME support
 *
 * @param Msg the windows message to filter
 *
 * @return TRUE if the app should handle it, FALSE if Live did
 */
UBOOL appPanoramaInputTranslateMessage(LPMSG Msg)
{
	return XLivePreTranslateMessage(Msg) == FALSE;
}

#endif

#endif	//#if WITH_UE3_NETWORKING
