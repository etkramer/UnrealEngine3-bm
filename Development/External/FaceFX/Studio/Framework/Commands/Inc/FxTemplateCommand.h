//------------------------------------------------------------------------------
// The template command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxTemplateCommand_H__
#define FxTemplateCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The template command.
class FxTemplateCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxTemplateCommand, FxCommand);
public:
	// Constructor.
	FxTemplateCommand();
	// Destructor.
	virtual ~FxTemplateCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Export to template.
	FxBool ExportTemplate( const FxString& filename );
	// Sync from template.
	FxBool SyncTemplate( const FxString& filename, FxBool syncFaceGraph,
		FxBool syncMapping, FxBool syncNodeGroups, FxBool syncCameras,
		FxBool syncWorkspaces, FxBool syncColourMap );
};

} // namespace Face

} // namespace OC3Ent

#endif
