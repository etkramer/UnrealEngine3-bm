//------------------------------------------------------------------------------
// Phoneme list commands.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPhonemeListCommand_H__
#define FxPhonemeListCommand_H__

#include "FxCommand.h"
#include "FxPhonWordList.h" //@todo temp.

namespace OC3Ent
{

namespace Face
{

class FxPhonemeListCommand : public FxCommand
{
	FX_DECLARE_CLASS(FxPhonemeListCommand, FxCommand)

public:
	// Constructor
	FxPhonemeListCommand();
	// Destructor
	~FxPhonemeListCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	//@todo temporary!
	FxPhonWordList _redoState;
	FxPhonWordList _undoState;
};

} // namespace Face

} // namespace OC3Ent

#endif
