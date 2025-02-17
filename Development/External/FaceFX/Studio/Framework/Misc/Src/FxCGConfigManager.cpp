//------------------------------------------------------------------------------
// Manages all of the found FxCG configurations.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCGConfigManager.h"
#include "FxStudioApp.h"
#include "FxFileSystemUtilitiesWin32.h"
#include "FxConsole.h"
#include "FxArchiveStoreFile.h"
#include "FxArchive.h"
#include "FxTearoffWindow.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dir.h"
#endif

//@todo temp?
#define DEFAULT_CONFIG_NAME FxName("Default")

namespace OC3Ent
{

namespace Face
{

FxGenericCoarticulationConfigLookup::FxGenericCoarticulationConfigLookup( const FxString& path )
	: ConfigFilePath(path)
{
}

FxBool FxGenericCoarticulationConfigLookup::Load( void )
{
	return FxLoadCoarticulationConfigFromFile(Config, ConfigFilePath.GetData(), FxTrue);
}

FxGestureConfigLookup::FxGestureConfigLookup( const FxString& path )
	: ConfigFilePath(path)
{
}

FxBool FxGestureConfigLookup::Load( void )
{
	return FxLoadGestureConfigFromFile(Config, ConfigFilePath.GetData(), FxTrue);
}

FxArray<FxGenericCoarticulationConfigLookup> FxCGConfigManager::_genericCoarticulationConfigs;
FxArray<FxGestureConfigLookup> FxCGConfigManager::_gestureConfigs;
FxArray<FxCGConfigNameRemap> FxCGConfigManager::_genericCoarticulationNameMap;
FxArray<FxCGConfigNameRemap> FxCGConfigManager::_gestureNameMap;
FxName FxCGConfigManager::_defaultGenericCoarticulationName;
FxName FxCGConfigManager::_defaultGestureConfigName;

void FxCGConfigManager::Startup( void )
{
	_defaultGenericCoarticulationName = DEFAULT_CONFIG_NAME;
	_defaultGestureConfigName = DEFAULT_CONFIG_NAME;
	ScanAndLoadAllConfigs();
}

void FxCGConfigManager::Shutdown( void )
{
	// Nothing for now.
}

void FxCGConfigManager::ScanAndLoadAllConfigs( void )
{
	// Clear the configurations.
	FlushAllConfigs();
	
	// Find gesture configuration.
	wxFileName configPath(FxStudioApp::GetAppDirectory());
	configPath.AppendDir(wxT("Configs"));
	configPath.AppendDir(wxT("Gestures"));
	wxArrayString fileList;
	wxDir::GetAllFiles(configPath.GetPath(), &fileList, wxT("*.fxg"));
	_gestureConfigs.Reserve(fileList.GetCount());
	for( FxSize i = 0; i < fileList.GetCount(); ++i )
	{
		_gestureConfigs.PushBack(FxGestureConfigLookup(FxString(fileList[i].mb_str(wxConvLibc))));
	}

	// Find coarticulation configurations.
	configPath = FxStudioApp::GetAppDirectory();
	configPath.AppendDir(wxT("Configs"));
	configPath.AppendDir(wxT("Generic Coarticulation"));
	fileList.Clear();
	wxDir::GetAllFiles(configPath.GetPath(), &fileList, wxT("*.fxc"));
	_genericCoarticulationConfigs.Reserve(fileList.GetCount());
	for( FxSize i = 0; i < fileList.GetCount(); ++i )
	{
		_genericCoarticulationConfigs.PushBack(FxGenericCoarticulationConfigLookup(FxString(fileList[i].mb_str(wxConvLibc))));
	}

	// Load the gesture configurations.
	FxBool foundDefault = FxFalse;
	for( FxSize i = 0; i < _gestureConfigs.Length(); ++i )
	{
		if( _gestureConfigs[i].Load() )
		{
			// See if this name has been seen before.
			for( FxSize j = 0; j < i; ++j )
			{
				if( _gestureConfigs[i].Config.GetName() == _gestureConfigs[j].Config.GetName() )
				{
					FxWarn("Gesture config %s (%s) has the same name as the config in %s", 
						_gestureConfigs[i].Config.GetNameAsCstr(),
						_gestureConfigs[i].ConfigFilePath.GetData(),
						_gestureConfigs[j].ConfigFilePath.GetData());
				}
			}

			if( _gestureConfigs[i].Config.GetName() == _defaultGestureConfigName )
			{
				foundDefault = FxTrue;
			}
		}
		else
		{
			_gestureConfigs.Remove(i--);
		}
	}

	if( !foundDefault )
	{
		// Ensure the default config always exists.
		FxGestureConfigLookup defaultConfigLookup;
		defaultConfigLookup.ConfigFilePath = "";
		defaultConfigLookup.Config = FxGestureConfig();
		defaultConfigLookup.Config.SetName(_defaultGestureConfigName);
		_gestureConfigs.PushBack(defaultConfigLookup);
	}

	// Load the coarticulation configurations.
	foundDefault = FxFalse;
	for( FxSize i = 0; i < _genericCoarticulationConfigs.Length(); ++i )
	{
		if( _genericCoarticulationConfigs[i].Load() )
		{
			// See if this name has been seen before.
			for( FxSize j = 0; j < i; ++j )
			{
				if( _genericCoarticulationConfigs[i].Config.GetName() == _genericCoarticulationConfigs[j].Config.GetName() )
				{
					FxWarn("Generic coarticulation config %s (%s) has the same name as the config in %s", 
						_genericCoarticulationConfigs[i].Config.GetNameAsCstr(),
						_genericCoarticulationConfigs[i].ConfigFilePath.GetData(),
						_genericCoarticulationConfigs[j].ConfigFilePath.GetData());
				}
			}

			if( _gestureConfigs[i].Config.GetName() == _defaultGenericCoarticulationName )
			{
				foundDefault = FxTrue;
			}
		}
		else
		{
			_genericCoarticulationConfigs.Remove(i--);
		}
	}

	if( !foundDefault )
	{
		// Ensure the default config always exists.
		FxGenericCoarticulationConfigLookup defaultConfigLookup;
		defaultConfigLookup.ConfigFilePath = "";
		defaultConfigLookup.Config = FxCoarticulationConfig();
		defaultConfigLookup.Config.SetName(_defaultGenericCoarticulationName);
		_genericCoarticulationConfigs.PushBack(defaultConfigLookup);
	}
}	

void FxCGConfigManager::FlushAllConfigs( void )
{
	_genericCoarticulationConfigs.Clear();
	_gestureConfigs.Clear();
}

void FxCGConfigManager::ClearNameMap( void )
{
	_genericCoarticulationNameMap.Clear();
	_gestureNameMap.Clear();
}

FxGestureConfig* FxCGConfigManager::GetGestureConfig( const FxName& name )
{
	// Take into account any remapping.
	const FxName& currentName = GetGestureConfigName(name);

	// Attempt to find the configuration.
	for( FxSize i = 0; i < _gestureConfigs.Length(); ++i )
	{
		if( currentName == _gestureConfigs[i].Config.GetName() )
		{
			return &_gestureConfigs[i].Config;
		}
	}
	return NULL;
}

FxCoarticulationConfig* FxCGConfigManager::GetGenericCoarticulationConfig( const FxName& name )
{
	// Take into account any remapping.
	const FxName& currentName = GetGenericCoarticulationConfigName(name);

	// Attempt to find the configuration.
	for( FxSize i = 0; i < _genericCoarticulationConfigs.Length(); ++i )
	{
		if( currentName == _genericCoarticulationConfigs[i].Config.GetName() )
		{
			return &_genericCoarticulationConfigs[i].Config;
		}
	}
	return NULL;
}

const FxName& FxCGConfigManager::GetGestureConfigName( const FxName& name )
{
	// See if the configuration name exists already.
	for( FxSize i = 0; i < _gestureConfigs.Length(); ++i )
	{
		if( name == _gestureConfigs[i].Config.GetName() )
		{
			return _gestureConfigs[i].Config.GetName();
		}
	}

	// If we didn't find the configuration name, it may have been renamed.
	for( FxSize i = 0; i < _gestureNameMap.Length(); ++i )
	{
		if( name == _gestureNameMap[i].original )
		{
			return _gestureNameMap[i].current;
		}
	}

	// If it's not in the remapping table, we need to ask the user what to do.
	FxCGConfigChooser configChooser(FxCGConfigChooser::CCM_Gesture, 
		wxString::Format(_("The gesture configuration \"%s\" was not found. Please select a replacement."), wxString::FromAscii(name.GetAsCstr())));
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	configChooser.ShowModal(); // No need to check the return - it can only be OK.
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	FxCGConfigNameRemap remapEntry;
	remapEntry.original = name;
	remapEntry.current  = FxName(FxString(configChooser.ChosenConfigName.mb_str(wxConvLibc)).GetData());
	_gestureNameMap.PushBack(remapEntry);

	return _gestureNameMap.Back().current;
}

const FxName& FxCGConfigManager::GetGenericCoarticulationConfigName( const FxName& name )
{
	// See if the configuration name exists already.
	for( FxSize i = 0; i < _genericCoarticulationConfigs.Length(); ++i )
	{
		if( name == _genericCoarticulationConfigs[i].Config.GetName() )
		{
			return _genericCoarticulationConfigs[i].Config.GetName();
		}
	}

	// If we didn't find the configuration name, it may have been renamed.
	for( FxSize i = 0; i < _genericCoarticulationNameMap.Length(); ++i )
	{
		if( name == _genericCoarticulationNameMap[i].original )
		{
			return _genericCoarticulationNameMap[i].current;
		}
	}

	// If it's not in the remapping table, we need to ask the user what to do.
	FxCGConfigChooser configChooser(FxCGConfigChooser::CCM_GenericCoarticulation, 
		wxString::Format(_("The coarticulation configuration \"%s\" was not found. Please select a replacement."), wxString::FromAscii(name.GetAsCstr())));
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	configChooser.ShowModal(); // No need to check the return - it can only be OK.
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	FxCGConfigNameRemap remapEntry;
	remapEntry.original = name;
	remapEntry.current  = FxName(FxString(configChooser.ChosenConfigName.mb_str(wxConvLibc)).GetData());
	_genericCoarticulationNameMap.PushBack(remapEntry);

	return _genericCoarticulationNameMap.Back().current;
}

FxSize FxCGConfigManager::GetNumGestureConfigs( void )
{
	return _gestureConfigs.Length();
}

const FxName& FxCGConfigManager::GetGestureConfigName( FxSize index )
{
	return _gestureConfigs[index].Config.GetName();
}

const FxName& FxCGConfigManager::GetDefaultGestureConfigName( void )
{
	return _defaultGestureConfigName;
}

FxSize FxCGConfigManager::GetNumGenericCoarticulationConfigs( void )
{
	return _genericCoarticulationConfigs.Length();
}

const FxName& FxCGConfigManager::GetGenericCoarticulationConfigName( FxSize index )
{
	return _genericCoarticulationConfigs[index].Config.GetName();
}

const FxName& FxCGConfigManager::GetDefaultGenericCoarticulationConfigName( void )
{
	return _defaultGenericCoarticulationName;
}

WX_IMPLEMENT_DYNAMIC_CLASS(FxCGConfigChooser, wxDialog)

BEGIN_EVENT_TABLE(FxCGConfigChooser,wxDialog)
	EVT_CHOICE(ControlID_CGConfigChooserDlgConfigNameChoice, FxCGConfigChooser::OnConfigChange)
END_EVENT_TABLE()

FxCGConfigChooser::FxCGConfigChooser(FxConfigChooserMode mode, wxString msg)
	: wxDialog( FxStudioApp::GetMainWindow(), wxID_ANY, _("Select A Configuration"), wxDefaultPosition, wxDefaultSize, wxCAPTION )
	, ChosenConfigName(wxEmptyString)
	, _mode(mode)
{
	FxCGConfigManager::ScanAndLoadAllConfigs();

	// Create the config name array.
	FxSize numConfigs = 0;
	if( _mode == CCM_Gesture )
	{
		numConfigs = FxCGConfigManager::GetNumGestureConfigs();
	}
	else
	{
		numConfigs = FxCGConfigManager::GetNumGenericCoarticulationConfigs();
	}

	wxString* configNames = new wxString[numConfigs];
	for( FxSize i = 0; i < numConfigs; ++i )
	{
		if( _mode == CCM_Gesture )
		{
			configNames[i] = wxString::FromAscii(FxCGConfigManager::GetGestureConfigName(i).GetAsCstr());
		}
		else
		{
			configNames[i] = wxString::FromAscii(FxCGConfigManager::GetGenericCoarticulationConfigName(i).GetAsCstr());
		}
	}

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(new wxStaticText(this, wxID_ANY, msg, wxDefaultPosition, wxSize(300, 35)), 0, wxALL, 5);
	mainSizer->Add((_configName = new wxChoice(this, ControlID_CGConfigChooserDlgConfigNameChoice, wxDefaultPosition, wxDefaultSize, numConfigs, configNames)), 0, wxEXPAND|wxALL, 5);
	wxStaticBoxSizer* descSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Description")), wxVERTICAL);
	descSizer->Add((_description = new wxStaticText(this, ControlID_CGConfigChooserDlgDescriptionText, _(""), wxDefaultPosition, wxSize(300,100), wxST_NO_AUTORESIZE )), 0, wxALL, 5);
	mainSizer->Add(descSizer, 0, wxALL, 5);
	mainSizer->Add(new wxButton(this, wxID_OK, _("OK")), wxALL, 5);

	SetSizer(mainSizer);
	SetAutoLayout(TRUE);
	Fit();

	delete [] configNames;

	// Set a default selection.
	_configName->SetSelection(_configName->FindString(wxString::FromAscii(DEFAULT_CONFIG_NAME.GetAsCstr())));
	wxCommandEvent evt;
	OnConfigChange(evt);
}

void FxCGConfigChooser::OnConfigChange( wxCommandEvent& FxUnused(event) )
{
	ChosenConfigName = _configName->GetString(_configName->GetSelection());
	FxString configName(ChosenConfigName.mb_str(wxConvLibc));
	if( _mode == CCM_Gesture )
	{
		FxGestureConfig* config = FxCGConfigManager::GetGestureConfig(configName.GetData());
		if( config )
		{
			_description->SetLabel(wxString::FromAscii(config->description.GetData()));
		}
	}
	else
	{
		FxCoarticulationConfig* config = FxCGConfigManager::GetGenericCoarticulationConfig(configName.GetData());
		if( config )
		{
			_description->SetLabel(wxString::FromAscii(config->description.GetData()));
		}
	}
}

} // namespace Face

} // namespace OC3Ent
