/*=============================================================================
	UnFaceFXSupport.cpp: FaceFX support.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if WITH_FACEFX

#include "UnFaceFXSupport.h"
#include "UnFaceFXRegMap.h"
#include "UnFaceFXMaterialParameterProxy.h"
#include "UnFaceFXMorphTargetProxy.h"

#include "../FaceFX/UnFaceFXNode.h"
#include "../FaceFX/UnFaceFXMaterialNode.h"
#include "../FaceFX/UnFaceFXMorphNode.h"

// Define this to link in the debug libraries.
#define USE_FACEFX_DEBUG_LIBS

#if defined(XBOX)

#if defined(_DEBUG) && defined(USE_FACEFX_DEBUG_LIBS)
	#pragma message("Linking Xenon DEBUG FaceFX Libs")
	#pragma comment(lib, "Xenon/External/FaceFX/FxSDK/lib/xbox360/vs8/FxSDK_Unreal_Debug_Xbox360.lib")
#else
	#pragma message("Linking Xenon RELEASE FaceFX Libs")
	#pragma comment(lib, "Xenon/External/FaceFX/FxSDK/lib/xbox360/vs8/FxSDK_Unreal_Release_Xbox360.lib")
#endif

#elif defined(PS3)

#else // WIN32

#if defined(_DEBUG) && defined(USE_FACEFX_DEBUG_LIBS)
	#pragma message("Linking Win32 DEBUG FaceFX Libs")
	#pragma comment(lib, "../External/FaceFX/FxSDK/lib/win32/vs8/FxSDK_Unreal_Debug.lib")
#else
	#pragma message("Linking Win32 RELEASE FaceFX Libs")
	#pragma comment(lib, "../External/FaceFX/FxSDK/lib/win32/vs8/FxSDK_Unreal_Release.lib")
#endif

#endif

using namespace OC3Ent;
using namespace Face;

void* FxAllocateUE3( FxSize NumBytes )
{
	return appMalloc(NumBytes);
}

void* FxAllocateDebugUE3( FxSize NumBytes, const FxAChar* /*system*/ )
{
	return appMalloc(NumBytes);
}

void FxFreeUE3( void* Ptr, FxSize /*n*/ )
{
	appFree(Ptr);
}

void FxRemoveNodeUserDataUE3( FxCompiledFaceGraph& cg )
{
	FxSize numNodes = cg.nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		if( cg.nodes[i].pUserData )
		{
			switch( cg.nodes[i].nodeType )
			{
			case NT_MorphTargetUE3:
				{
					if( cg.nodes[i].pUserData )
					{
						FFaceFXMorphTargetProxy* pUserData = reinterpret_cast<FFaceFXMorphTargetProxy*>(cg.nodes[i].pUserData);
						delete pUserData;
						cg.nodes[i].pUserData = NULL;
					}
				}
				break;
			case NT_MaterialParameterUE3:
				{
					if( cg.nodes[i].pUserData )
					{
						FFaceFXMaterialParameterProxy* pUserData = reinterpret_cast<FFaceFXMaterialParameterProxy*>(cg.nodes[i].pUserData);
						delete pUserData;
						cg.nodes[i].pUserData = NULL;
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void FxPreDestroyCallbackFuncUE3( FxCompiledFaceGraph& cg )
{
	FxRemoveNodeUserDataUE3(cg);
}

void FxPreCompileCallbackFuncUE3( FxCompiledFaceGraph& cg, FxFaceGraph& FxUnused(faceGraph) )
{
	FxRemoveNodeUserDataUE3(cg);
}

void UnInitFaceFX( void )
{
	debugf(TEXT("Initializing FaceFX..."));
	FxMemoryAllocationPolicy allocPolicyUE3(MAT_Custom, FxFalse, FxAllocateUE3, FxAllocateDebugUE3, FxFreeUE3);
	FxSDKStartup(allocPolicyUE3);
	FxCompiledFaceGraph::SetPreDestroyCallback(FxPreDestroyCallbackFuncUE3);
	FxCompiledFaceGraph::SetCompilationCallbacks(FxPreCompileCallbackFuncUE3, NULL, NULL, NULL);
	FFaceFXRegMap::Startup();
	debugf(TEXT("FaceFX initialized:"));
	debugf(TEXT("    version : %s"), *FString(FxSDKGetVersionString().GetCstr()));
	debugf(TEXT("    licensee: %s"), *FString(FxSDKGetLicenseeName().GetCstr()));
	debugf(TEXT("    project : %s"), *FString(FxSDKGetLicenseeProjectName().GetCstr()));
}

void UnShutdownFaceFX( void )
{
	debugf(TEXT("Shutting down FaceFX..."));
	FFaceFXRegMap::Shutdown();
	FxSDKShutdown();
	debugf(TEXT("FaceFX shutdown."));
}

TArray<FFaceFXRegMapEntry> FFaceFXRegMap::RegMap;

void FFaceFXRegMap::Startup( void )
{
}

void FFaceFXRegMap::Shutdown( void )
{
	RegMap.Empty();
}

FFaceFXRegMapEntry* FFaceFXRegMap::GetRegisterMapping( const FName& RegName )
{
	INT NumRegMapEntries = RegMap.Num();
	for( INT i = 0; i < NumRegMapEntries; ++i )
	{
		if( RegMap(i).UnrealRegName == RegName )
		{
			return &RegMap(i);
		}
	}
	return NULL;
}

void FFaceFXRegMap::AddRegisterMapping( const FName& RegName )
{
	if( !GetRegisterMapping(RegName) )
	{
		RegMap.AddItem(FFaceFXRegMapEntry(RegName, FxName(TCHAR_TO_ANSI(*RegName.ToString()))));
	}
}

#endif // WITH_FACEFX

