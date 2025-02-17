//------------------------------------------------------------------------------
// A most-recently used file list manager.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxMRUFileListManager.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/fileconf.h"
#endif

namespace OC3Ent
{

namespace Face
{

FxMRUListManager::FxMRUListManager( FxWidgetMediator* mediator )
	: FxWidget( mediator )
	, _fileHistory(9)
{
}

void FxMRUListManager::AttachToMenu( wxMenu* menu )
{
	_fileHistory.UseMenu( menu );
}

FxSize FxMRUListManager::GetNumMRUFiles()
{
	return _fileHistory.GetCount();
}

wxString FxMRUListManager::GetMRUFile( FxSize index )
{
	return _fileHistory.GetHistoryFile( index );
}

// Remove the MRU entry at index
void FxMRUListManager::RemoveMRUFile( FxSize index )
{
	_fileHistory.RemoveFileFromHistory( index );
}

void FxMRUListManager::OnLoadActor( FxWidget* FxUnused(sender), const FxString& actorPath )
{
	_fileHistory.AddFileToHistory(wxString::FromAscii(actorPath.GetData()));
}

void FxMRUListManager::OnSaveActor( FxWidget* FxUnused(sender), const FxString& actorPath )
{
	_fileHistory.AddFileToHistory(wxString::FromAscii(actorPath.GetData()));
}

void FxMRUListManager::OnSerializeOptions( FxWidget* FxUnused(sender), wxFileConfig* config, FxBool isLoading )
{
	if( isLoading )
	{
		for( FxInt32 i = 8; i >= 0; --i )
		{
			wxString path = config->Read( wxString::Format(wxT("/MRUList/File%d"),i), wxEmptyString );
			if( path != wxEmptyString )
			{
				_fileHistory.AddFileToHistory( path );
			}
		}
	}
	else
	{
		for( FxSize i = 0; i < _fileHistory.GetCount(); ++i )
		{
			config->Write( wxString::Format(wxT("/MRUList/File%d"),i), _fileHistory.GetHistoryFile(i) );
		}
	}
}

} // namespace Face

} // namespace OC3Ent
