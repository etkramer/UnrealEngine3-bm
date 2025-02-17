//------------------------------------------------------------------------------
// The main module for the FaceFx Max plug-in.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMaxMain_H__
#define FxMaxMain_H__

// 3DSMAX includes.
#include "max.h"
#include "decomp.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "utilapi.h"

// For Biped funtionality.  File taken from the reactor sample.
#include "reactapi.h"
#include "maxscrpt.h"
#include "bmmlib.h"
#include "ISTDPLUG.H"
#include "ANIMTBL.H"
#include "modstack.h"
#include "guplib.h"

// These come from the resource file included with wm3.h.
#define IDS_MORPHMTL                    39
#define IDS_MTL_MAPNAME                 45
#define IDS_MTL_BASENAME                46

// This file is not included in the SDK, but is required to get access to the 
// Max Morpher.  
#include "wm3.h"

#include "FxToolLog.h"

#define kCurrentFxMaxVersion "1.7.2"

#define FACEFX_CLASS_ID	Class_ID(0x48be5a9a, 0x4ae949c7)

// The plug-in API on windows.
#ifdef WIN32
	#define FX_PLUGIN_API __declspec( dllexport )
#endif

extern TCHAR* GetString( int id );
extern HINSTANCE hInstance;

#endif
