//------------------------------------------------------------------------------
// The coordsys command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCoordSysCommand_H__
#define FxCoordSysCommand_H__

#include "FxCommand.h"
#include "FxActor.h"

namespace OC3Ent
{

namespace Face
{

// The coordsys command.
class FxCoordSysCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxCoordSysCommand, FxCommand);
public:
	// Constructor.
	FxCoordSysCommand();
	// Destructor.
	virtual ~FxCoordSysCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

private:
	// The source package passed to the command.
	FxString _sourcePackage;
	// Converts from Maya coordinate system to FaceFX sample engine coordinate 
	// system.
	void _convertMayaCoordSys( FxActor* pActor );
	// Converts from Max coordinate system to FaceFX sample engine coordinate 
	// system.
	void _convertMaxCoordSys( FxActor* pActor );
	// Converts from XSI coordinate system to FaceFX sample engine coordinate
	// system.
	void _convertXsiCoordSys( FxActor* pActor );
};

} // namespace Face

} // namespace OC3Ent

#endif
