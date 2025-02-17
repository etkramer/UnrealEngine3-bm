//------------------------------------------------------------------------------
// The collapse face graph command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCollapseFaceGraphCommand_H__
#define FxCollapseFaceGraphCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The collapse face graph command.
class FxCollapseFaceGraphCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxCollapseFaceGraphCommand, FxCommand);
public:
	// Constructor.
	FxCollapseFaceGraphCommand();
	// Destructor.
	virtual ~FxCollapseFaceGraphCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
