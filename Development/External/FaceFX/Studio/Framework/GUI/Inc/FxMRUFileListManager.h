//------------------------------------------------------------------------------
// A most-recently used file list manager.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMRUListManager_H__
#define FxMRUListManager_H__

#include "FxPlatform.h"
#include "FxWidget.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	// For wxFileHistory.
	#include "wx/docview.h"
#endif

namespace OC3Ent
{

namespace Face
{

class FxMRUListManager : public FxWidget
{
public:
	FxMRUListManager( FxWidgetMediator* mediator );

	// Attaches the MRU list to a menu.
	void AttachToMenu( wxMenu* menu );
	// Returns the number of MRU files
	FxSize GetNumMRUFiles();
	// Returns the file at index.
	wxString GetMRUFile( FxSize index );

	// Remove the MRU entry at index
	void RemoveMRUFile( FxSize index );

	// Automatically add the path to the MRU list.
	void OnLoadActor( FxWidget* sender, const FxString& actorPath );
	void OnSaveActor( FxWidget* sender, const FxString& actorPath );
	// Automatically serialize the MRU list into the config file.
	void OnSerializeOptions( FxWidget* sender, wxFileConfig* config, FxBool isLoading );

protected:
	wxFileHistory _fileHistory;
};

} // namespace Face

} // namespace OC3Ent

#endif
