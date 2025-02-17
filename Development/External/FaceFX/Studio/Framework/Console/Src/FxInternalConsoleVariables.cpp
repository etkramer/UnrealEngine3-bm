//------------------------------------------------------------------------------
// All built-in FaceFX console variables should be included here.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxInternalConsoleVariables.h"
#include "FxVM.h"
#include "FxStudioApp.h"
#include "FxSDK.h"
#include "FxAudioWidget.h"
#include "FxAudioPlayerWidget.h"
#include "FxAnalysisWidget.h"

#ifdef USE_FX_RENDERER
	#include "FxRenderWidgetOC3.h"
#endif // USE_FX_RENDERER

namespace OC3Ent
{

namespace Face
{

void FxInternalConsoleVariables::RegisterAll( void )
{
	// Register the console variable class.
	FxConsoleVariable::StaticClass();

	FxChar str[32] = {0};

	// Register the console variables.
	FxVM::RegisterConsoleVariable("g_appdirectory", 
		                          FxString(FxStudioApp::GetAppDirectory().GetPath().mb_str(wxConvLibc)), 
								  FX_CONSOLE_VARIABLE_READONLY, 
								  NULL);
	FxItoa(FxSDKGetVersion(), str, 32);
	FxVM::RegisterConsoleVariable("g_sdkversion", 
								  str, 
								  FX_CONSOLE_VARIABLE_READONLY, 
							      NULL);
	FxVM::RegisterConsoleVariable("g_licenseename", 
								  FxSDKGetLicenseeName(), 
								  FX_CONSOLE_VARIABLE_READONLY, 
								  NULL);
	FxVM::RegisterConsoleVariable("g_licenseeprojectname", 
								  FxSDKGetLicenseeProjectName(), 
								  FX_CONSOLE_VARIABLE_READONLY, 
								  NULL);
	FxItoa(FxSDKGetLicenseeVersion(), str, 32);
	FxVM::RegisterConsoleVariable("g_licenseeversion", 
								  str, 
								  FX_CONSOLE_VARIABLE_READONLY, 
								  NULL);
	FxVM::RegisterConsoleVariable("g_playbackspeed",
								  "1.0",
								  0,
								  NULL);
	FxVM::RegisterConsoleVariable("g_audiosystem",
								  "openal",
								  0,
								  FxAudioPlayerWidget::OnAudioSystemChanged);
	FxVM::RegisterConsoleVariable("g_loopallfromhere",
								  "0",
								  0,
								  NULL);
	FxVM::RegisterConsoleVariable("g_unattended",
								  "0",
								  0,
								  NULL);
	FxVM::RegisterConsoleVariable("aw_showstress",
								  "0",
								  0,
								  FxAudioWidget::OnDrawOptionUpdated);
	FxVM::RegisterConsoleVariable("cw_historylength",
								  "250",
								  0,
								  NULL);
	FxVM::RegisterConsoleVariable("pl_realtimeupdate",
								  "0",
								  0,
								  NULL);
	FxVM::RegisterConsoleVariable("a_audiomin",
								  "0.5",
								  0,
								  FxAnalysisWidget::OnAudioMinimumUpdated);
	FxVM::RegisterConsoleVariable("a_audiomax",
								  "90.0",
								  0,
							      FxAnalysisWidget::OnAudioMaximumUpdated);
	FxVM::RegisterConsoleVariable("a_detectspeech",
								  "auto",
								  0,
								  NULL);

#ifdef USE_FX_RENDERER
	FxVM::RegisterConsoleVariable("rw_showbindpose",
								  "0",
								  0,
								  FxRenderWidgetOC3::OnShowBindPoseUpdated);
	FxVM::RegisterConsoleVariable("rw_scalefactor",
								  "1.0",
								  0,
								  FxRenderWidgetOC3::OnScaleFactorUpdated);
#endif // USE_FX_RENDERER
}

void FxInternalConsoleVariables::UnregisterAll( void )
{
	// Unregister the console variables.
	FxVM::UnregisterConsoleVariable("g_appdirectory");
	FxVM::UnregisterConsoleVariable("g_sdkversion");
	FxVM::UnregisterConsoleVariable("g_licenseename");
	FxVM::UnregisterConsoleVariable("g_licenseeprojectname");
	FxVM::UnregisterConsoleVariable("g_licenseeversion");
	FxVM::UnregisterConsoleVariable("g_playbackspeed");
	FxVM::UnregisterConsoleVariable("g_audiosystem");
	FxVM::UnregisterConsoleVariable("g_loopallfromhere");
	FxVM::UnregisterConsoleVariable("g_unattended");
	FxVM::UnregisterConsoleVariable("aw_showstress");
	FxVM::UnregisterConsoleVariable("cw_historylength");
	FxVM::UnregisterConsoleVariable("pl_realtimeupdate");
	FxVM::UnregisterConsoleVariable("a_audiomin");
	FxVM::UnregisterConsoleVariable("a_audiomax");
	FxVM::UnregisterConsoleVariable("a_detectspeech");

#ifdef USE_FX_RENDERER
	FxVM::UnregisterConsoleVariable("rw_showbindpose");
	FxVM::UnregisterConsoleVariable("rw_scalefactor");
#endif // USE_FX_RENDERER
}

} // namespace Face

} // namespace OC3Ent
