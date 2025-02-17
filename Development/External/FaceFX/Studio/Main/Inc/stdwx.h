//------------------------------------------------------------------------------
// Standard header for wxWindows applications. 
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef stdwx_H__
#define stdwx_H__

#include "FxWxWrappers.h"
#include "FxResource.h"

#define wxUSE_UNICODE 1

#define IS_FACEFX_STUDIO

// Define this to enable the security code.
//#define FX_ENABLE_SECURITY

#ifndef __UNREAL__
	#if defined(_MSC_VER) && _MSC_VER >= 1400 && !defined(POINTER_64)
		#define POINTER_64 
	#endif
#endif // __UNREAL__

#ifndef __UNREAL__
	#ifndef __APPLE__
		#include "wx/wxprec.h"
	#else
		#include "wx/wxchar.h"
		#include "wx/wx.h"
	#endif
	#include "wx/help.h"
#endif

#ifdef __UNREAL__
	#include "XWindow.h"
	//#define FX_NO_ONLINE_HELP
	#ifdef FX_ENABLE_SECURITY
		#undef FX_ENABLE_SECURITY
	#endif
#endif

#include "FxVersionInfo.h"
#include "FxUtilityFunctions.h"
#include "FxUtil.h"

// The height of all tool bars in FaceFX Studio.
static const ::OC3Ent::Face::FxInt32 FxToolbarHeight = 27;

#ifndef __UNREAL__
	// Define USE_FX_RENDERER to use the built-in FaceFX renderer.
	#define USE_FX_RENDERER
#endif

// Define this for beta builds.
//#define FX_BETA

// Define this for internal builds.
//#define FX_INTERNAL

static const wxString PRODUCT_NAME = _("FaceFX Studio");

static const wxString APP_VERSION  = _("1.7.2");

#ifdef FX_BETA
	static const wxString APP_NAME = PRODUCT_NAME + _(" (") + APP_VERSION + _(" Beta)");
#else
	#ifdef FX_INTERNAL
		static const wxString APP_NAME = PRODUCT_NAME + _(" (") + APP_VERSION + _(" Internal Pre-Release)");
	#else
		static const wxString APP_NAME = PRODUCT_NAME;
	#endif
#endif

// Define this to force the analyze command to be available regardless of the
// state of EVALUATION_VERSION or MOD_DEVELOPER_VERSION.
//#define FX_FORCE_ENABLE_ANALYZE_COMMAND

#if defined(EVALUATION_VERSION)
	static const wxString APP_TITLE = APP_NAME + _(" (Evaluation)");
#elif defined(MOD_DEVELOPER_VERSION)
	static const wxString APP_TITLE = APP_NAME + _(" (Mod Developer)");
#elif defined(NO_SAVE_VERSION)
	static const wxString APP_TITLE = APP_NAME + _(" (No Save)");
#else
	static const wxString APP_TITLE = APP_NAME;
#endif

#if defined(EVALUATION_VERSION) || defined(MOD_DEVELOPER_VERSION) || defined(NO_SAVE_VERSION) || defined(FX_ENABLE_SECURITY)
	#if !defined(FX_FORCE_ENABLE_ANALYZE_COMMAND)
		#define FX_NO_ANALYZE_COMMAND
	#endif
#endif

#ifndef __UNREAL__
	#ifdef FX_DEBUG
		#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
		#include <crtdbg.h>
	#else
		#define DEBUG_NEW new
	#endif
#endif

#endif
