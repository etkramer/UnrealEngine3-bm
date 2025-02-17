//------------------------------------------------------------------------------
// The analysis command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnalysisCommand_H__
#define FxAnalysisCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The analysis command.
class FxAnalysisCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAnalysisCommand, FxCommand);
public:
	// Constructor.
	FxAnalysisCommand();
	// Destructor.
	virtual ~FxAnalysisCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
