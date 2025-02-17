//------------------------------------------------------------------------------
// The commands governing the phoneme to track map.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMapCommand_H__
#define FxMapCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

class FxMapCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxMapCommand, FxCommand);

public:
	// Constructor.
	FxMapCommand();
	// Destructor
	virtual ~FxMapCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	enum CommandMode
	{
		CreateTrack,
		RemoveTrack,
		MapPhonemeToTrack,
		RemovePhoneme
	};
	CommandMode _commandMode;

	FxString _requestedTrackName;
	FxString _currentTrackName;

	FxString _requestedPhonemeString;

	FxReal _requestedValue;
	FxReal _currentValue;

	FxArray<FxString> _currentNames;
	FxArray<FxReal> _currentValues;

	FxBool _noRefresh;
};

} // namespace Face

} // namespace OC3Ent

#endif
