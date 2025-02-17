/*=============================================================================
	SampleClass.cpp
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	This is a minimal example of how to mix UnrealScript and C++ code
	within a class (defined by SampleClass.uc and SampleClass.cpp),
	for a package defined by ExampleGame.u.
=============================================================================*/

// Includes.
#include "ExampleGame.h"

/*-----------------------------------------------------------------------------
	ASampleClass class implementation.
-----------------------------------------------------------------------------*/

// C++ implementation of "SampleNativeFunction".

// The following function is the auto-declared in the ExampleClasses.h file when you compile the .uc file.
// The function signature is dictated by the parameters and return type of the corresponding function in
// the SampleClass.uc file.
INT ASampleClass::SampleNativeFunction(INT Integer, const FString& String, FVector Vector)
{
	// Write a message to the log.
	debugf(TEXT("Entered C++ SampleNativeFunction"));

	// Generate a message as an FString.
	// This accesses both UnrealScript function parameters (like "String"), and UnrealScript member variables (like "MyBool").
	FString Msg = FString::Printf(TEXT("In C++ SampleNativeFunction (Integer=%i,String=%s,Bool=%i)"),Integer,*String,MyBool);

	// Call some UnrealScript functions from C++.
    debugf(TEXT("Message from C++: %s"),*Msg);
	this->eventSampleEvent(Integer);

	return 0;
}

// Register ASampleClass.
// If you forget this, you get a VC++ linker error like:
// SampleClass.obj : error LNK2001: unresolved external symbol "private: static class UClass  ASampleClass::PrivateStaticClass" (?PrivateStaticClass@ASampleClass@@0VUClass@@A)
IMPLEMENT_CLASS(ASampleClass);

