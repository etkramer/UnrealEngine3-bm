//------------------------------------------------------------------------------
// The main module for the FaceFx Max plug-in.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMaxMain.h"
#include "FxMaxData.h"

// External that returns the class description for the FaceFX Max plug-in.
extern ClassDesc2* GetFxMaxClassDesc( void );

// External that returns the class description for the FaceFX Max function publishing.
extern ClassDesc2* GetFxMaxScriptInterfaceClassDesc( void );

// The instance handle for this DLL.
HINSTANCE hInstance;
// TRUE if the controls have been initialized for this DLL.
BOOL controlsInit = FALSE;

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
BOOL WINAPI DllMain( HINSTANCE hInstDLL, ULONG fdwReason, LPVOID lpvReserved )
{
	// Hang on to this DLL's instance handle.
	hInstance = hInstDLL;
	if( DLL_PROCESS_ATTACH == fdwReason && !controlsInit )
	{
		// Start up the FaceFX Max plug-in interface.
		OC3Ent::Face::FxMaxData::maxInterface.Startup();

		controlsInit = TRUE;
		// Initialize MAX's custom controls.
		InitCustomControls(hInstance);
		// Initialize Win95 controls.
		InitCommonControls();
		if( TheManager )
		{
			// Only initialize the logging system if it hasn't been initialized yet.
			if( 0 == OC3Ent::Face::FxToolLogFile::GetPath().Length() )
			{
// Max 9 removed support for APP_MAXROOT_DIR in anticipation of plugine not being able to
// write to the install path in Windows Vista.  The end result is that different version of Max
// Will write to different locations.
#ifdef APP_MAXDATA_DIR
				const TCHAR* maxRootDirectory = TheManager->GetDir(APP_MAXDATA_DIR);
#else
				const TCHAR* maxRootDirectory = TheManager->GetDir(APP_MAXROOT_DIR);
#endif
				OC3Ent::Face::FxString logPath = maxRootDirectory;
				logPath = OC3Ent::Face::FxString::Concat(logPath, "FaceFX_Log.txt");
				OC3Ent::Face::FxToolLogFile::SetPath(logPath);
			}
		}
		else
		{
			// Only initialize the logging system if it hasn't been initialized yet.
			if( 0 == OC3Ent::Face::FxToolLogFile::GetPath().Length() )
			{
				OC3Ent::Face::FxString logPath = "C:/FaceFX_Log.txt";
				OC3Ent::Face::FxToolLogFile::SetPath(logPath);
			}
		}
		
		FxToolLog("Loaded FaceFX for 3dsmax %s", kCurrentFxMaxVersion);
		FxToolLog("Running under 3dsmax r%f", static_cast<OC3Ent::Face::FxReal>(GET_MAX_RELEASE(Get3DSMAXVersion())/1000.0f));
	}
	if( DLL_PROCESS_DETACH == fdwReason && controlsInit )
	{
		// Shut down the FaceFX Max plug-in interface.
		OC3Ent::Face::FxMaxData::maxInterface.Shutdown();

		FxToolLog("Unloaded FaceFX for 3dsmax %s", kCurrentFxMaxVersion);
	}
	return TRUE;
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
FX_PLUGIN_API const TCHAR* LibDescription( void )
{
	return GetString(IDS_LIBDESCRIPTION);
}

// Returns the number of plug-in classes in this DLL.
FX_PLUGIN_API int LibNumberClasses( void )
{
	return 2;
}

// Returns the number of plug-in classes in this DLL.
FX_PLUGIN_API ClassDesc* LibClassDesc( int i )
{
	switch( i ) 
	{
		case 0 : return GetFxMaxClassDesc();
		case 1 : return GetFxMaxScriptInterfaceClassDesc();
		default: return NULL;
	}
}

// Returns a pre-defined constant indicating the version of the system under 
// which it was compiled.  It is used to allow the system to catch obsolete 
// DLLs.
FX_PLUGIN_API ULONG LibVersion( void )
{
	return VERSION_3DSMAX;
}

TCHAR* GetString( int id )
{
	static TCHAR buffer[256];
	if( hInstance )
	{
		return LoadString(hInstance, id, buffer, sizeof(buffer)) ? buffer : NULL;
	}
	return NULL;
}
