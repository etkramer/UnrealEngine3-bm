/*=============================================================================
	CorePrivate.h: Unreal core private header file.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*----------------------------------------------------------------------------
	Core public includes.
----------------------------------------------------------------------------*/

#ifndef _INC_COREPRIVATE
#define _INC_COREPRIVATE

#include "Core.h"

/*-----------------------------------------------------------------------------
	Locals functions.
-----------------------------------------------------------------------------*/

extern void appPlatformPreInit();
extern void appPlatformInit();
extern void appPlatformPostInit();

/*-----------------------------------------------------------------------------
	Includes.
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
	UTextBufferFactory.
-----------------------------------------------------------------------------*/

//
// Imports UTextBuffer objects.
//
class UTextBufferFactory : public UFactory
{
	DECLARE_CLASS(UTextBufferFactory,UFactory,CLASS_Intrinsic,Core)

	// Constructors.
	UTextBufferFactory();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	// UFactory interface.
	UObject* FactoryCreateText( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn );
};

#endif

