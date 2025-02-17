//------------------------------------------------------------------------------
// The key command.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxKeyCommand_H__
#define FxKeyCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

class FxKeyCommand : FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxKeyCommand, FxCommand);

public:
	// Constructor.
	FxKeyCommand();
	// Destructor.
	virtual ~FxKeyCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	enum KeyCommandMode
	{
		NONE,
		SELECTION,
		CLEARSELECTION,
		INSERTION,
		DEFAULTINSERTION,
		DELETION,
		TRANSFORMATION,
		SETPIVOT,
		EDIT
	};
	KeyCommandMode _commandMode;

	// Selection-relevant member variables.
	FxReal _requestedMinTime;
	FxReal _requestedMaxTime;
	FxReal _requestedMinValue;
	FxReal _requestedMaxValue;
	FxBool _requestedToggle;
	FxBool _singleKeySelection;
	FxArray<FxSize> _cachedSelCurveIndices;
	FxArray<FxSize> _cachedSelKeyIndices;

	// Transformation-relevant member variables
	FxReal _requestedTimeDelta;
	FxReal _requestedValueDelta;
	FxBool _fromGUI;

	// Pivot key-relevant member variables
	FxSize _requestedCurveIndex;
	FxSize _requestedKeyIndex;
	FxSize _cachedCurveIndex;
	FxSize _cachedKeyIndex;

	// Selection deleting-relevant member variables.
	FxArray<FxReal> _cachedKeyTimes;
	FxArray<FxReal> _cachedKeyValues;
	FxArray<FxReal> _cachedKeyIncDerivs;
	FxArray<FxReal> _cachedKeyOutDerivs;
	FxArray<FxInt32> _cachedKeyFlags;

	// Insertion/Edit-relevant member variables.
	FxReal _requestedKeyTime;
	FxReal _requestedKeyValue;
	FxReal _requestedKeySlopeIn;
	FxReal _requestedKeySlopeOut;
	FxReal _cachedKeyTime;
	FxReal _cachedKeyValue;
	FxReal _cachedKeySlopeIn;
	FxReal _cachedKeySlopeOut;
};

} // namespace Face

} // namespace OC3Ent

#endif
