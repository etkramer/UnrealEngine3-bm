//------------------------------------------------------------------------------
// The export LTF command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxExportLTFCommand_H__
#define FxExportLTFCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The export LTF command.
class FxExportLTFCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxExportLTFCommand, FxCommand);
public:
	// Constructor.
	FxExportLTFCommand();
	// Destructor.
	virtual ~FxExportLTFCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Bake and write out a single animation as an LTF file.
	FxBool ExportLTF( const FxString& groupName, const FxString& animName, 
		const FxArray<FxString>& extraTracks, const FxString& filePath );
	// Bake and write out all animations as LTF files.
	FxBool ExportAll( const FxArray<FxString>& extraTracks, 
		const FxString& outputDirectory );
};

} // namespace Face

} // namespace OC3Ent

#endif

