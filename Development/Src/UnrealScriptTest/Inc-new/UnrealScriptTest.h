/*=============================================================================
	UnrealScriptTest.h
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "Engine.h"
#include "UnrealScriptTestClasses.h"

#ifndef __UNREALSCRIPTTEST_H__
#define __UNREALSCRIPTTEST_H__

/**
 * Base class for all UObject types that contain fields.
 */
class UByteCodeSerializer : public UStruct
{
public:
	DECLARE_CLASS_INTRINSIC(UByteCodeSerializer,UStruct,0|CLASS_Transient,UnrealScriptTest);
	NO_DEFAULT_CONSTRUCTOR(UByteCodeSerializer);

	/**
	 * Entry point for processing a single struct's bytecode array.  Calls SerializeExpr to process the struct's bytecode.
	 *
	 * @param	InParentStruct	the struct containing the bytecode to be processed.
	 */
	void ProcessStructBytecode( UStruct* InParentStruct );

	/**
	 * Determines the correct class context based on the specified property.
	 *
	 * @param	ContextProperty		a property which potentially changes the current lookup context, such as object or interface property
	 *
	 * @return	if ContextProperty corresponds to an object, interface, or delegate property, result is a pointer to the class associated
	 *			with the property (e.g. for UObjectProperty, would be PropertyClass); NULL if no context switch will happen as a result of
	 *			processing ContextProperty
	 */
	UClass* DetermineCurrentContext( UProperty* ContextProperty ) const;

	/**
	 * Sets the value of CurrentContext based on the results of calling DetermineCurrentContext()
	 *
	 * @param	ContextProperty		a property which potentially changes the current lookup context, such as object or interface property
	 */
	void SetCurrentContext( UProperty* ContextProperty=GProperty );

	/**
	 * Marks the specified function as referenced by finding the UFunction corresponding to its original declaration.
	 *
	 * @param	ReferencedFunction	the function that should be marked referenced
	 */
	static void MarkFunctionAsReferenced( UFunction* ReferencedFunction );

	/**
	 * Wrapper for checking whether the specified function is referenced.  Uses the UFunction corresponding to
	 * the function's original declaration.
	 */
	static UBOOL IsFunctionReferenced( UFunction* FunctionToCheck );

	/**
	 * Searches for the UFunction corresponding to the original declaration of the specified function.
	 *
	 * @param	FunctionToCheck		the function to lookup
	 *
	 * @return	a pointer to the original declaration of the specified function.
	 */
	static UFunction* FindOriginalDeclaration( UFunction* FunctionToCheck );

	/**
	 * Find the function associated with the native index specified.
	 *
	 * @param	iNative		the native function index to search for
	 *
	 * @return	a pointer to the UFunction using the specified native function index.
	 */
	UFunction* GetIndexedNative( INT iNative );

	/** === UStruct interface === */
	/**
	 * Processes compiled bytecode for UStructs, marking any UFunctions encountered as referenced.
	 *
	 * @param	iCode	the current position in the struct's bytecode array.
	 * @param	Ar		the archive used for serializing the bytecode; not really necessary for this purpose.
	 *
	 * @return	the token that was parsed from the bytecode stream.
	 */
	virtual EExprToken SerializeExpr( INT& iCode, FArchive& Ar );

protected:
	/**
	 * The struct (function, state, class, etc.) containing the bytecode currently being processed.
	 */
	class UStruct* ParentStruct;

	/**
	 * The class associated with the symbol that was most recently parsed.  Used for function lookups to ensure that the correct class is being searched.
	 */
	class UClass* CurrentContext;

	/**
	 * Caches the results of GetIndexedNative, for faster lookup.
	 */
	TMap<INT,UFunction*>	NativeFunctionIndexMap;
};

#endif

