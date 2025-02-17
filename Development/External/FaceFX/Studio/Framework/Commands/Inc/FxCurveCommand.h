//------------------------------------------------------------------------------
// The curve command.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCurveCommand_H__
#define FxCurveCommand_H__

#include "FxCommand.h"
#include "FxAnimCurve.h"

namespace OC3Ent
{

namespace Face
{

class FxCurveCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxCurveCommand, FxCommand);

public:
	// Constructor.
	FxCurveCommand();
	// Destructor.
	virtual ~FxCurveCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	FxString _group;
	FxString _anim;
	FxString _curve;

	FxAnimCurve* _cachedAnimCurve;

	FxBool	 _requestedOwner;
	FxBool   _cachedOwner;
	FxBool   _isValidOperation;
	FxInterpolationType _requestedInterp;
	FxInterpolationType _cachedInterp;

	FxArray<FxString> _cachedSelectedCurves;
	FxArray<FxSize> _cachedCurveIndices;
	FxArray<FxSize> _cachedKeyIndices;

	FxSize _oldIndex;

	// Curve copying
	FxString _sourceGroup;
	FxString _sourceAnim;
	FxString _sourceName;

	enum FxCurveCommandMode
	{
		NONE = 0,
		ADD = 1,
		REMOVE = 2,
		EDITOWNER = 4,
		EDITINTERP = 8,
		COPYCURVE = 16
	};
	FxCurveCommandMode _mode;
};

} // namespace Face

} // namespace OC3Ent

#endif
