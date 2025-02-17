//------------------------------------------------------------------------------
// Manages all of the found FxCG configurations.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCGConfigManager_H__
#define FxCGConfigManager_H__

#include "stdwx.h"

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxCoarticulationConfig.h"
#include "FxGestureConfig.h"

namespace OC3Ent
{

namespace Face
{

// A generic coarticulation configuration lookup.
class FxGenericCoarticulationConfigLookup
{
public:
	// Constructor.
	FxGenericCoarticulationConfigLookup( const FxString& path = "" );

	// The full path to the generic coarticulation configuration file.
	FxString ConfigFilePath;
	// The generic coarticulation configuration.
	FxCoarticulationConfig Config;

	// Loads the configuration specified by ConfigFilePath from disc.
	// Returns FxTrue if the load was successful or FxFalse otherwise.
	FxBool Load( void );
};

// A gesture configuration lookup.
class FxGestureConfigLookup
{
public:
	// Constructor.
	FxGestureConfigLookup( const FxString& path = "" );

	// The full path to the gesture configuration file.
	FxString ConfigFilePath;
	// The gesture configuration.
	FxGestureConfig Config;

	// Loads the configuration specified by ConfigFilePath from disc.
	// Returns FxTrue if the load was successful or FxFalse otherwise.
	FxBool Load( void );
};

struct FxCGConfigNameRemap
{
	FxName original;
	FxName current;
};

// Manages FxCG configurations.
class FxCGConfigManager
{
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxCGConfigManager);
public:

	// Starts up the configuration manager
	static void Startup( void );
	// Shuts down the configuration manager.
	static void Shutdown( void );

	// Scans the configuration directories and loads all found configurations.
	static void ScanAndLoadAllConfigs( void );

	// Removes all configurations from the manager.
	static void FlushAllConfigs( void );

	// Clears the name maps, to be called after saving.
	static void ClearNameMap( void );

	// Returns the gesture configuration associated with a specific name.
	static FxGestureConfig* GetGestureConfig( const FxName& name );
	// Returns the generic coarticulation configuration associated with a specific name.
	static FxCoarticulationConfig* GetGenericCoarticulationConfig( const FxName& name );

	// Returns the current name for a gesture configuration.
	static const FxName& GetGestureConfigName( const FxName& name );
	// Returns the current name for a generic coarticulation configuration.
	static const FxName& GetGenericCoarticulationConfigName( const FxName& name );

	// Returns the number of gesture configurations.
	static FxSize GetNumGestureConfigs( void );
	// Returns the gesture configuration name.
	static const FxName& GetGestureConfigName( FxSize index );
	// Returns the default gesture configuration name.
	static const FxName& GetDefaultGestureConfigName( void );

	// Returns the number of generic coarticulation configuration.
	static FxSize GetNumGenericCoarticulationConfigs( void );
	// Returns the generic coarticulation configuration name.
	static const FxName& GetGenericCoarticulationConfigName( FxSize index );
	// Returns the default gesture configuration name.
	static const FxName& GetDefaultGenericCoarticulationConfigName( void );

protected:
	// The generic coarticulation configurations.
	static FxArray<FxGenericCoarticulationConfigLookup> _genericCoarticulationConfigs;
	// The gesture configurations.
	static FxArray<FxGestureConfigLookup> _gestureConfigs;

	// The generic coarticulation name remap.
	static FxArray<FxCGConfigNameRemap> _genericCoarticulationNameMap;
	// The gesture name remap.
	static FxArray<FxCGConfigNameRemap> _gestureNameMap;

	// The default gesture configuration name.
	static FxName _defaultGestureConfigName;
	// The default generic coarticulation name.
	static FxName _defaultGenericCoarticulationName;
};

class FxCGConfigChooser : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxCGConfigChooser)
	DECLARE_EVENT_TABLE()

public:
	enum FxConfigChooserMode
	{
		CCM_Gesture,
		CCM_GenericCoarticulation
	};

	FxCGConfigChooser(FxConfigChooserMode mode = CCM_Gesture, wxString msg = _("Please select a new configuration"));

	wxString ChosenConfigName;

private:
	void OnConfigChange( wxCommandEvent& event );

	FxConfigChooserMode _mode;

	wxChoice*			_configName;
	wxStaticText*		_description;
};

} // namespace Face

} // namespace OC3Ent

#endif
