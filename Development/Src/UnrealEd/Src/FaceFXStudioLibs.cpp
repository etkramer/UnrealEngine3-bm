/*=============================================================================
	FaceFXStudioLibs.cpp: Code for linking in the FaceFX Studio libraries.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

#if WITH_FACEFX

#include "../../../External/FaceFX/Studio/Main/Inc/FxStudioMainWin.h"

// This is only here to force the compiler to link in the libraries!
OC3Ent::Face::FxStudioMainWin* GFaceFXStudio = NULL;

// Define this to link in the debug libraries.
//#define USE_FACEFX_DEBUG_LIBS

#if defined(_DEBUG) && defined(USE_FACEFX_DEBUG_LIBS)
	#pragma message("Linking Win32 DEBUG FaceFX Studio Libs")
	#pragma comment(lib, "../External/FaceFX/FxCG/lib/win32/vs8/FxCG_Unreal_Debug.lib")
	#pragma comment(lib, "../External/FaceFX/FxAnalysis/lib/win32/vs8/FxAnalysis_Unreal_Debug.lib")
#else
	#pragma message("Linking Win32 RELEASE FaceFX Studio Libs")
	#pragma comment(lib, "../External/FaceFX/FxCG/lib/win32/vs8/FxCG_Unreal_Release.lib")
	#pragma comment(lib, "../External/FaceFX/FxAnalysis/lib/win32/vs8/FxAnalysis_Unreal_Release.lib")
#endif

#pragma comment(lib, "../External/FaceFX/Studio/External/libresample-0.1.3/libresample.lib")

#endif // WITH_FACEFX

