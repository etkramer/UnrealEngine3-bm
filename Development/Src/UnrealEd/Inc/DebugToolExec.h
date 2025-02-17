/*=============================================================================
	DebugToolExec.h: Game debug tool implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#ifndef __DEBUGTOOLEXEC_H__
#define __DEBUGTOOLEXEC_H__

/**
 * This class servers as an interface to debugging tools like "EDITOBJECT" which
 * can be invoked by console commands which are parsed by the exec function.
 */
class FDebugToolExec : public FExec
{
protected:
	/**
	 * Brings up a property window to edit the passed in object.
	 *
	 * @param Object	property to edit
	 */
	void EditObject( UObject* Object );

	/** A list of all property window frames that have been created in EditObject during PIE */
	TArray<class WxPropertyWindowFrame*> PIEFrames;

public:
	/**
	 * Exec handler, parsing the passed in command
	 *
	 * @param Cmd	Command to parse
	 * @param Ar	output device used for logging
	 */
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );

	/**
	 * Special UnrealEd cleanup function for cleaning up after a Play In Editor session
	 */
	void CleanUpAfterPIE();

};

#endif
