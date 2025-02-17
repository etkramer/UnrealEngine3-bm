/*=============================================================================
	UnNativeClassExporter.h: Native class *Classes.h exporter header
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNNATIVECLASSEXPORTER_H__
#define __UNNATIVECLASSEXPORTER_H__

//
//	FNativeClassHeaderGenerator
//

struct FNativeClassHeaderGenerator
{
private:

	UClass*				CurrentClass;
	TArray<UClass*>		Classes;
	FString				ClassHeaderFilename, API;
	UObject*			Package;
	UBOOL				MasterHeader;
	FStringOutputDevice	PreHeaderText;
	FStringOutputDevice	EnumHeaderText;
	FStringOutputDevice	HeaderText;

	/** the class tree for this package */
	const FClassTree&	ClassTree;

	/** the existing disk version of this header */
	FString				OriginalHeader;

	/**
	 * Determines whether the glue version of the specified native function
	 * should be exported
	 * 
	 * @param	Function	the function to check
	 * @return	TRUE if the glue version of the function should be exported.
	 */
	UBOOL ShouldExportFunction( UFunction* Function );
	
	/**
	 * Determines whether this class's parent has been changed.
	 * 
	 * @return	TRUE if the class declaration for the current class doesn't match the disk-version of the class declaration
	 */
	UBOOL HasParentClassChanged();
	/**
	 * Determines whether the property layout for this native class has changed
	 * since the last time the header was generated
	 * 
	 * @return	TRUE if the property block for the current class doesn't match the disk-version of the class declaration
	 */
	UBOOL HavePropertiesChanged();

	/**
	 * Exports the struct's C++ properties to the HeaderText output device and adds special
	 * compiler directives for GCC to pack as we expect.
	 *
	 * @param	Struct				UStruct to export properties
	 * @param	TextIndent			Current text indentation
	 * @param	ImportsDefaults		whether this struct will be serialized with a default value
	 */
	void ExportProperties( UStruct* Struct, INT TextIndent, UBOOL ImportsDefaults );

	/**
	 * Exports the C++ class declarations for a native interface class.
	 */
	void ExportInterfaceClassDeclaration( UClass* Class );

	//	ExportClassHeader - Appends the header definition for an inheritance heirarchy of classes to the header.
	void ExportClassHeader(UClass* Class,UBOOL ExportedParent = 0);

	/**
	 * Returns a string in the format CLASS_Something|CLASS_Something which represents all class flags that are set for the specified
	 * class which need to be exported as part of the DECLARE_CLASS macro
	 */
	FString GetClassFlagExportText( UClass* Class );

	/**
	 * Iterates through all fields of the specified class, and separates fields that should be exported with this class into the appropriate array.
	 * 
	 * @param	Class				the class to pull fields from
	 * @param	Enums				[out] all enums declared in the specified class
	 * @param	Structs				[out] list of structs declared in the specified class
	 * @param	Consts				[out] list of pure consts declared in the specified class
	 * @param	CallbackFunctions	[out] list of delegates and events declared in the specified class
	 * @param	NativeFunctions		[out] list of native functions declared in the specified class
	 */
	void RetrieveRelevantFields(UClass* Class, TArray<UEnum*>& Enums, TArray<UScriptStruct*>& Structs, TArray<UConst*>& Consts, TArray<UFunction*>& CallbackFunctions, TArray<UFunction*>& NativeFunctions);

	/**
	 * Exports the header text for the list of enums specified
	 * 
	 * @param	Enums	the enums to export
	 */
	void ExportEnums( const TArray<UEnum*>& Enums );

	/**
	 * Exports the header text for the list of structs specified
	 * 
	 * @param	Structs		the structs to export
	 * @param	TextIndent	the current indentation of the header exporter
	 */
	void ExportStructs( const TArray<UScriptStruct*>& NativeStructs, INT TextIndent=0 );

	/**
	 * Exports the header text for the list of consts specified
	 * 
	 * @param	Consts	the consts to export
	 */
	void ExportConsts( const TArray<UConst*>& Consts );

	/**
	 * Exports the parameter struct declarations for the list of functions specified
	 * 
	 * @param	CallbackFunctions	the functions that have parameters which need to be exported
	 */
	void ExportEventParms( const TArray<UFunction*>& CallbackFunctions );

	/**
	 * Get the intrinsic null value for this property
	 * 
	 * @param	Prop				the property to get the null value for
	 * @param	bMacroContext		TRUE when exporting the P_GET* macro, FALSE when exporting the friendly C++ function header
	 * @param	bTranslatePointers	if true, FPointer structs will be set to NULL instead of FPointer()
	 *
	 * @return	the intrinsic null value for the property (0 for ints, TEXT("") for strings, etc.)
	 */
	FString GetNullParameterValue( UProperty* Prop, UBOOL bMacroContext, UBOOL bTranslatePointers=FALSE );

	/**
	 * Retrieve the default value for an optional parameter
	 * 
	 * @param	Prop	the property being parsed
	 * @param	bMacroContext	TRUE when exporting the P_GET* macro, FALSE when exporting the friendly C++ function header
	 * @param	DefaultValue	[out] filled in with the default value text for this parameter
	 *
	 * @return	TRUE if default value text was successfully retrieved for this property
	 */
	UBOOL GetOptionalParameterValue( UProperty* Prop, UBOOL bMacroContext, FString& DefaultValue );

	/**
	 * Exports a native function prototype
	 * 
	 * @param	FunctionData	data representing the function to export
	 * @param	bEventTag		TRUE to export this function prototype as an event stub, FALSE to export as a native function stub.
	 *							Has no effect if the function is a delegate.
	 * @param	Return			[out] will be assigned to the return value for this function, or NULL if the return type is void
	 * @param	Parameters		[out] will be filled in with the parameters for this function
	 */
	void ExportNativeFunctionHeader( const FFuncInfo& FunctionData, UBOOL bEventTag, UProperty*& Return, TArray<UProperty*>&Parameters );

	/**
	 * Exports the native stubs for the list of functions specified
	 * 
	 * @param	NativeFunctions	the functions to export
	 */
	void ExportNativeFunctions( const TArray<UFunction*>& NativeFunctions );

	/**
	 * Exports the proxy definitions for the list of enums specified
	 * 
	 * @param	CallbackFunctions	the functions to export
	 */
	void ExportCallbackFunctions( const TArray<UFunction*>& CallbackFunctions );

	/**
	 * Determines if the property has alternate export text associated with it and if so replaces the text in PropertyText with the
	 * alternate version. (for example, structs or properties that specify a native type using export-text).  Should be called immediately
	 * after ExportCppDeclaration()
	 *
	 * @param	Prop			the property that is being exported
	 * @param	PropertyText	the string containing the text exported from ExportCppDeclaration
	 */
	void ApplyAlternatePropertyExportText( UProperty* Prop, FStringOutputDevice& PropertyText );


	/**
	 * Determines whether the specified class should still be considered misaligned,
	 * and clears the RF_MisalignedObject flag if the existing member layout [on disk]
	 * matches the current member layout.  Also propagates the result to any child classes
	 * which are noexport classes (otherwise, they'd never be cleared since noexport classes
	 * don't export header files).
	 *
	 * @param	ClassNode	the node for the class to check.  It is assumed that this class has already exported
	 *						it new class declaration.
	 */
	void ClearMisalignmentFlag( const FClassTree* ClassNode );

public:

	// Constructor
	FNativeClassHeaderGenerator(const TCHAR* InClassHeaderFilename,UObject* InPackage,UBOOL InMasterHeader, FClassTree& inClassTree);
};

#endif
