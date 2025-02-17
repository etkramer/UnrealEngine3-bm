/*=============================================================================
	UMakeCommandlet.cpp: UnrealEd script recompiler.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "Factories.h"

// @todo: Put UnrealEd into the project settings
#include "../../UnrealEd/Inc/SourceControlIntegration.h"

#include "UnCompileHelper.h"
#include "UnNativeClassExporter.h"
#include "UnScrPrecom.h"

IMPLEMENT_COMPARE_POINTER( UClass, UMakeCommandlet, { return appStricmp(*A->GetName(),*B->GetName()); } )
IMPLEMENT_COMPARE_CONSTREF( FString, UMakeCommandlet, { return appStricmp(*A,*B); } )

FCompilerMetadataManager*	GScriptHelper = NULL;

#define PROP_MARKER_BEGIN TEXT("//## BEGIN PROPS")
#define PROP_MARKER_END	TEXT("//## END PROPS")

/**
 * Helper class used to cache UClass* -> TCHAR* name lookup for finding the named used for C++ declaration.
 */
struct FNameLookupCPP
{
	/**
	 * Destructor, cleaning up allocated memory.
	 */
	~FNameLookupCPP()
	{
		for( TMap<UStruct*,TCHAR*>::TIterator It(StructNameMap); It; ++It )
		{
			TCHAR* Name = It.Value();
			delete [] Name;
		}
	}

	/**
	 * Returns the name used for declaring the passed in struct in C++
	 *
	 * @param	UStruct to obtain C++ name for
	 * @return	Name used for C++ declaration
	 */
	const TCHAR* GetNameCPP( UStruct* Struct )
	{
		TCHAR* NameCPP = StructNameMap.FindRef( Struct );
		if( NameCPP )
		{
			return NameCPP;
		}
		else
		{
			FString	TempName		= FString::Printf( TEXT("%s%s"), Struct->GetPrefixCPP(), *Struct->GetName() );
			INT		StringLength	= TempName.Len();
			NameCPP					= new TCHAR[StringLength + 1];
			appStrcpy( NameCPP, StringLength + 1, *TempName );
			NameCPP[StringLength]	= 0;
			StructNameMap.Set( Struct, NameCPP );
			return NameCPP;
		}
	}

private:
	/** Map of UStruct pointers to C++ names */
	TMap<UStruct*,TCHAR*> StructNameMap;
};

/** C++ name lookup helper */
static FNameLookupCPP* NameLookupCPP;

/**
	* Determines whether the glue version of the specified native function
	* should be exported
	*
	* @param	Function	the function to check
	* @return	TRUE if the glue version of the function should be exported.
	*/
UBOOL FNativeClassHeaderGenerator::ShouldExportFunction( UFunction* Function )
{
	UBOOL bExport = TRUE;

	// don't export any script stubs for native functions declared in interface classes
	if ( Function->GetOwnerClass()->HasAnyClassFlags(CLASS_Interface) )
	{
		bExport = FALSE;
	}
	else
	{
		// always export if the function is static
		if ( (Function->FunctionFlags&FUNC_Static) == 0 )
		{
			// don't export the function if this is not the original declaration and there is
			// at least one parent version of the function that is declared native
			for ( UFunction* ParentFunction = Function->GetSuperFunction(); ParentFunction;
				ParentFunction = ParentFunction->GetSuperFunction() )
			{
				if ( (ParentFunction->FunctionFlags&FUNC_Native) != 0 )
				{
					bExport = FALSE;
					break;
				}
			}
		}
	}

	return bExport;
}

/**
 * Determines whether this class's parent has been changed.
 * 
 * @return	TRUE if the class declaration for the current class doesn't match the disk-version of the class declaration
 */
UBOOL FNativeClassHeaderGenerator::HasParentClassChanged()
{
	UObject* ExistingClassDefaultObject = CurrentClass->GetDefaultObject();

	// if we don't have a ClassDefaultObject here, it means that this is a new native class (if this was a previously
	// existing native class, it would have been statically registered, which would have created the class default object
	// In this case, we can use the HasParentClassChanged method, which is 100% reliable in determining whether the parent
	// class has indeed been changed
	if ( ExistingClassDefaultObject != NULL && ExistingClassDefaultObject->HasParentClassChanged() )
	{
		return TRUE;
	}

	// but we also need to check to see if any multiple inheritance parents changed

	// if this is the first time we're generating the header, no need to do anything
	if ( OriginalHeader.Len() == 0 )
		return FALSE;

	FString Temp = FString::Printf(TEXT("class %s : public "), NameLookupCPP->GetNameCPP( CurrentClass ));

	INT OriginalStart = OriginalHeader.InStr(*Temp);
	if ( OriginalStart == INDEX_NONE )
	{
		return FALSE;
	}

	OriginalStart += Temp.Len();
	FString OriginalParentClass = OriginalHeader.Mid(OriginalStart);

	INT OriginalEnd = OriginalParentClass.InStr(TEXT("\r\n"));
	if ( OriginalEnd == INDEX_NONE )
	{
		OriginalEnd = OriginalParentClass.InStr(TEXT("\n"));
	}
	check(OriginalEnd != INDEX_NONE);

	OriginalParentClass = OriginalParentClass.Left(OriginalEnd);

	// cut if off at any multiple inheritance
	// @todo: parse apart the whole thing and compare it to the list of parents in FClassMetaData
	OriginalEnd = OriginalParentClass.InStr(TEXT(","));
	if (OriginalEnd != INDEX_NONE)
	{
		OriginalParentClass = OriginalParentClass.Left(OriginalEnd);
	}

	const TCHAR* SuperClassCPPName	= NameLookupCPP->GetNameCPP( CurrentClass->GetSuperClass() );
	if ( OriginalParentClass != SuperClassCPPName )
	{
		warnf(TEXT("%s != %s"), *OriginalParentClass, SuperClassCPPName);
		return TRUE;
	}

	return FALSE;
}

/**
 * Determines whether the property layout for this native class has changed
 * since the last time the header was generated
 *
 * @return	TRUE if the property block for the current class doesn't match the disk-version of the class declaration
 */
UBOOL FNativeClassHeaderGenerator::HavePropertiesChanged()
{
	// if this is the first time we're generating the header, no need to do anything
	if ( OriginalHeader.Len() == 0 )
		return FALSE;

	if ( HasParentClassChanged() )
		return TRUE;

	INT OriginalStart, OriginalEnd, CurrentStart, CurrentEnd;

	// first, determine the location of the beginning of the property block for the current class
	FString Temp = FString::Printf(TEXT("%s %s"), PROP_MARKER_BEGIN, *CurrentClass->GetName());
	OriginalStart = OriginalHeader.InStr(*Temp);
	if ( OriginalStart == INDEX_NONE )
	{
		// if the original header didn't contain this class, no need to do anything
		return FALSE;
	}

	CurrentStart = HeaderText.InStr(*Temp);
	check(CurrentStart != INDEX_NONE);

	// next, determine the location of the end of the property block for the current class
	Temp = FString::Printf(TEXT("%s %s"), PROP_MARKER_END, *CurrentClass->GetName());
	OriginalEnd = OriginalHeader.InStr(*Temp);
	check(OriginalEnd != INDEX_NONE);

	CurrentEnd = HeaderText.InStr(*Temp);
	check(CurrentEnd != INDEX_NONE);

	FString OriginalPropertyLayout = OriginalHeader.Mid(OriginalStart, OriginalEnd - OriginalStart);
	FString CurrentPropertyLayout = HeaderText.Mid(CurrentStart, CurrentEnd - CurrentStart);

	return OriginalPropertyLayout != CurrentPropertyLayout;
}

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
void FNativeClassHeaderGenerator::ClearMisalignmentFlag( const FClassTree* ClassNode )
{
	UClass* Class = ClassNode->GetClass();
	UClass* SuperClass = Class->GetSuperClass();

	// we're guaranteed that our parent class has already been exported, so if
	// RF_MisalignedObject is still set, then it means that it did actually change,
	// so we're still misaligned at this point
	if ( !SuperClass->IsMisaligned() )
	{
		Class->ClearFlags(RF_MisalignedObject);

		TArray<const FClassTree*> ChildClasses;
		ClassNode->GetChildClasses(ChildClasses);

		for ( INT i = 0; i < ChildClasses.Num(); i++ )
		{
			const FClassTree* ChildNode = ChildClasses(i);
			
			UClass* ChildClass = ChildNode->GetClass();
			if ( ChildClass->HasAnyFlags(RF_MisalignedObject) && (!ChildClass->HasAnyClassFlags(CLASS_Native) || ChildClass->HasAnyClassFlags(CLASS_NoExport)) )
			{
				// propagate this change to any noexport or non-native child classes
				ClearMisalignmentFlag(ChildNode);
			}
		}
	}
}

/**
 * Exports the struct's C++ properties to the HeaderText output device and adds special
 * compiler directives for GCC to pack as we expect.
 *
 * @param	Struct				UStruct to export properties
 * @param	TextIndent			Current text indentation
 * @param	ImportsDefaults		whether this struct will be serialized with a default value
 */
void FNativeClassHeaderGenerator::ExportProperties( UStruct* Struct, INT TextIndent, UBOOL ImportsDefaults )
{
	UProperty*	Previous			= NULL;
	UProperty*	LastInSuper			= NULL;
	UStruct*	InheritanceSuper	= Struct->GetInheritanceSuper();

	// Find last property in the lowest base class that has any properties
	UStruct* CurrentSuper = InheritanceSuper;
	while (LastInSuper == NULL && CurrentSuper)
	{
		for( TFieldIterator<UProperty,CASTCLASS_UProperty,0> It(CurrentSuper); It; ++It )
		{
			UProperty* Current = *It;

			// Disregard properties with 0 size like functions.
			if( It.GetStruct() == CurrentSuper && Current->ElementSize )
			{
				LastInSuper = Current;
			}
		}
		// go up a layer in the hierarchy
		CurrentSuper = CurrentSuper->GetSuperStruct();
	}

	EPropertyHeaderExportFlags CurrentExportType = PROPEXPORT_Public;

	// Iterate over all properties in this struct.
	for( TFieldIterator<UProperty,CASTCLASS_UProperty,0> It(Struct); It; ++It )
	{
		UProperty* Current = *It;

		FStringOutputDevice PropertyText;

		// Disregard properties with 0 size like functions.
		if( It.GetStruct()==Struct && Current->ElementSize )
		{
			// Skip noexport properties.
			if( !(Current->PropertyFlags&CPF_NoExport) )
			{
				// find the class info for this class
				FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);

				// find the compiler token for this property
				FTokenData* PropData = ClassData->FindTokenData(Current);
				if ( PropData != NULL )
				{
					// if this property has a different access specifier, then export that now
					if ( (PropData->Token.PropertyExportFlags & CurrentExportType) == 0 )
					{
						FString AccessSpecifier;
						if ( (PropData->Token.PropertyExportFlags & PROPEXPORT_Private) != 0 )
						{
							CurrentExportType = PROPEXPORT_Private;
							AccessSpecifier = TEXT("private");
						}
						else if ( (PropData->Token.PropertyExportFlags & PROPEXPORT_Protected) != 0 )
						{
							CurrentExportType = PROPEXPORT_Protected;
							AccessSpecifier = TEXT("protected");
						}
						else
						{
							CurrentExportType = PROPEXPORT_Public;
							AccessSpecifier = TEXT("public");
						}

						if ( AccessSpecifier.Len() )
						{
							PropertyText.Logf(TEXT("%s:%s"), *AccessSpecifier, LINE_TERMINATOR);
						}
					}
				}
				else if ( Current->IsA(UDelegateProperty::StaticClass()) )
				{
					// this is a delegate property generated by the compiler due to a delegate function declaration
					// these are always marked public
					if ( CurrentExportType != PROPEXPORT_Public )
					{
						CurrentExportType = PROPEXPORT_Public;
						PropertyText.Log(TEXT("public:") LINE_TERMINATOR);
					}
				}

				// Indent code and export CPP text.
				PropertyText.Logf( appSpc(TextIndent+4) );

// the following is a work in progress for supporting any type to have a {} type qualifier
#if 0
				// look up a variable type override
				FString VariableTypeOverride;

				// find the class info for this class
				FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);

				// FindTokenData can't be called on delegates?
				if (!Current->IsA(UDelegateProperty::StaticClass()))
				{
					FTokenData* PropData = ClassData->FindTokenData(Current);

					// if we have an override, set that as the type to export to the .h file
					if (PropData->Token.ExportInfo.Len() > 0)
					{
						VariableTypeOverride = PropData->Token.ExportInfo;

						// if it's the special "pointer" type, then append a * to the end of the declaration
						UStructProperty* StructProp = Cast<UStructProperty>(Current,CLASS_IsAUStructProperty);
						if (StructProp && StructProp->Struct->GetFName() == NAME_Pointer)
						{
							VariableTypeOverride += TEXT("*");
						}
					}
				}

				Current->ExportCppDeclaration( PropertyText, 1, 0, ImportsDefaults, VariableTypeOverride.Len() ? *VariableTypeOverride : NULL );
#else
				Current->ExportCppDeclaration( PropertyText, 1, 0, ImportsDefaults );
				ApplyAlternatePropertyExportText(*It, PropertyText);
#endif
				// Figure out whether we need to deal with alignment.
				UBOOL bRequiresAlignment	= FALSE;
				UBOOL bPreviousIsByte		= Previous && Previous->IsA(UByteProperty::StaticClass());
				UBOOL bPreviousIsBool		= Previous && Previous->IsA(UBoolProperty::StaticClass());
				UBOOL bLastInSuperIsByte	= LastInSuper && LastInSuper->IsA(UByteProperty::StaticClass());
				UBOOL bLastInSuperIsBool	= LastInSuper && LastInSuper->IsA(UBoolProperty::StaticClass());

				if( Current->IsA(UBoolProperty::StaticClass()) )
				{
					// Bool properties require alignment if they follow a byte or are the first property in a class
					// and the super ends in a bitfield or a byte.
					if( bPreviousIsByte || bLastInSuperIsBool || bLastInSuperIsByte )
					{
						bRequiresAlignment = TRUE;
					}
				}
				else if( Current->IsA(UByteProperty::StaticClass()) )
				{
					// Byte properties require alignment if they follow a bool property.
					if( bPreviousIsBool || bLastInSuperIsBool )
					{
						bRequiresAlignment = TRUE;
					}
				}

				// Emit alignment magic required for GCC support.
				if( bRequiresAlignment )
				{
					PropertyText.Logf(TEXT(" GCC_BITFIELD_MAGIC"));
				}

				// Finish up line.
				PropertyText.Logf(TEXT(";\r\n"));
			}

			LastInSuper	= NULL;
			Previous	= Current;
		}

		HeaderText.Log(PropertyText);
	}

	// if the last property that was exported wasn't public, emit a line to reset the access to "public" so that we don't interfere with cpptext
	if ( CurrentExportType != PROPEXPORT_Public )
	{
		HeaderText.Logf(TEXT("public:%s"), LINE_TERMINATOR);
	}
}

/**
 * Exports the C++ class declarations for a native interface class.
 */
void FNativeClassHeaderGenerator::ExportInterfaceClassDeclaration( UClass* Class )
{
	FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);

	const FClassTree* ClassNode = ClassTree.FindNode(Class);
	ClearMisalignmentFlag(ClassNode);

	TArray<UEnum*>			Enums;
	TArray<UScriptStruct*>	Structs;
	TArray<UConst*>			Consts;
	TArray<UFunction*>		CallbackFunctions;
	TArray<UFunction*>		NativeFunctions;

	// get the lists of fields that belong to this class that should be exported
	RetrieveRelevantFields(Class, Enums, Structs, Consts, CallbackFunctions, NativeFunctions);

	// export enum declarations
	ExportEnums(Enums);

	// export struct declarations
	ExportStructs(Structs);

	// export #defines for all consts
	ExportConsts(Consts);

	UClass* SuperClass = Class->GetSuperClass();


	// the name for the C++ version of the UClass
	const TCHAR* ClassCPPName		= NameLookupCPP->GetNameCPP( Class );
	const TCHAR* SuperClassCPPName	= NameLookupCPP->GetNameCPP( SuperClass );

	// Export the UClass declaration
	// Class definition.
	HeaderText.Logf( TEXT("class %s"), ClassCPPName );
	if( SuperClass )
	{
#if !EXPORT_NATIVEINTERFACE_UCLASS
		// interface classes don't derive from UObject in C++
		if ( SuperClass != UObject::StaticClass() )
#endif
		{
			HeaderText.Logf( TEXT(" : public %s"), SuperClassCPPName );
		}


		// look for multiple inheritance info
		const TArray<FMultipleInheritanceBaseClass>& InheritanceParents = ClassData->GetInheritanceParents();
		for (INT ParentIndex = 0; ParentIndex < InheritanceParents.Num(); ParentIndex++)
		{
			HeaderText.Logf(TEXT(", public %s"), *InheritanceParents(ParentIndex).ClassName);
		}
	}
	HeaderText.Logf( TEXT("\r\n{\r\npublic:\r\n") );

#if EXPORT_NATIVEINTERFACE_UCLASS

	// Build the DECLARE_CLASS line
	HeaderText.Logf( TEXT("%sDECLARE_ABSTRACT_CLASS(%s,"), appSpc(4), ClassCPPName );
	HeaderText.Logf( TEXT("%s,0"), SuperClassCPPName );

	// append class flags to the definition
	HeaderText.Logf(TEXT("%s,%s)%s"), *GetClassFlagExportText(Class), *Class->GetOuter()->GetName(), LINE_TERMINATOR);
	if(Class->ClassWithin != Class->GetSuperClass()->ClassWithin)
		HeaderText.Logf(TEXT("    DECLARE_WITHIN(%s)\r\n"), NameLookupCPP->GetNameCPP( Class->ClassWithin ) );

	// End of class.
	HeaderText.Logf( TEXT("    NO_DEFAULT_CONSTRUCTOR(%s)\r\n};\r\n\r\n"), ClassCPPName );



	// =============================================
	// Export the pure interface version of the class

	// the name of the pure interface class
	FString InterfaceCPPName		= FString::Printf(TEXT("I%s"), *Class->GetName());
	FString SuperInterfaceCPPName;
	if ( SuperClass != NULL )
	{
		SuperInterfaceCPPName = FString::Printf(TEXT("I%s"), *SuperClass->GetName());
	}

	// Class definition.
	HeaderText.Logf( TEXT("class %s"), *InterfaceCPPName );

	if( SuperClass )
	{
		// don't derive from IInterface, or we'll be unable to implement more than one interface
		// since the size of the interface's vtable [in the UObject class that implements the inteface]
		// will be different when more than one interface is implemented (caused by multiple inheritance of the same base class)
		if ( SuperClass != UObject::StaticClass() && SuperClass != UInterface::StaticClass()  )
		{
			HeaderText.Logf( TEXT(" : public %s"), *SuperInterfaceCPPName );
		}

		// look for multiple inheritance info
		const TArray<FMultipleInheritanceBaseClass>& InheritanceParents = ClassData->GetInheritanceParents();
		for (INT ParentIndex = 0; ParentIndex < InheritanceParents.Num(); ParentIndex++)
		{
			HeaderText.Logf(TEXT(", public %s"), *InheritanceParents(ParentIndex).ClassName);
		}
	}
	HeaderText.Logf( TEXT("\r\n{\r\n") );
	HeaderText.Logf( TEXT("protected:\r\n\tvirtual ~%s() {}\r\npublic:\r\n"), *InterfaceCPPName );
	HeaderText.Logf( TEXT("\ttypedef %s UClassType;\r\n"), ClassCPPName );

	// we'll need a way to get to the UObject portion of a native interface, so that we can safely pass native interfaces
	// to script VM functions
	if ( SuperClass == UInterface::StaticClass() )
	{
		HeaderText.Logf(TEXT("\tvirtual UObject* GetUObjectInterface%s()=0;\r\n"), *Class->GetName());
	}

#endif	// EXPORT_NATIVEINTERFACE_UCLASS


	// C++ -> UnrealScript stubs (native function execs)
	ExportNativeFunctions(NativeFunctions);

	// UnrealScript -> C++ proxies (events and delegates).
	ExportCallbackFunctions(CallbackFunctions);

	FString Filename = GEditor->EditPackagesInPath * *Class->GetOuter()->GetName() * TEXT("Inc") * ClassCPPName + TEXT(".h");
	if( Class->CppText && Class->CppText->Text.Len() )
	{
		HeaderText.Log( *Class->CppText->Text );
	}
	else if( GFileManager->FileSize(*Filename) > 0 )
	{
		HeaderText.Logf( TEXT("    #include \"%s.h\"\r\n"), ClassCPPName );
	}
	else
	{
		HeaderText.Logf( TEXT("    NO_DEFAULT_CONSTRUCTOR(%s)\r\n"), *InterfaceCPPName );
	}

	// End of class.
	HeaderText.Logf( TEXT("};\r\n") );

	// End.
	HeaderText.Logf( TEXT("\r\n") );
}

//	ExportClassHeader - Appends the header definition for an inheritance heirarchy of classes to the header.
void FNativeClassHeaderGenerator::ExportClassHeader(UClass* Class,UBOOL ExportedParent)
{
	if ( Class->ClassHeaderFilename == ClassHeaderFilename && Class->GetOuter() == Package )
	{
		// Export all classes we are dependent on.
		TArray<FName> DependentOnClassNames = Class->DependentOn;

		// Clear this class's dependency array prior to processing its dependencies to prevent infinite recursion
		Class->DependentOn.Empty();

		for( INT DependsOnIndex=0; DependsOnIndex<DependentOnClassNames.Num(); DependsOnIndex++ )
		{
			FName& DependencyClassName = DependentOnClassNames(DependsOnIndex);
			UClass* DependsOnClass = NULL;
			for( INT ClassIndex=0; ClassIndex<Classes.Num(); ClassIndex++ )
			{
				if( Classes(ClassIndex)->GetFName() == DependencyClassName )
				{
					DependsOnClass = FindObject<UClass>(ANY_PACKAGE,*DependencyClassName.ToString());
					if( !DependsOnClass )
					{
						warnf(NAME_Error,TEXT("Unknown class %s used in conjunction with dependson"), *DependencyClassName.ToString());
						return;
					}
					else if( !DependsOnClass->HasAnyClassFlags(CLASS_Exported) )
					{
						// Find first base class of DependsOnClass that is not a base class of Class. We can assume there is no manual
						// dependency on the base class as script compiler treats this as an error. Furthermore we can also assume that
						// there is no circular dependency chain as this is also caught by the compiler.
						while( !Class->IsChildOf( DependsOnClass->GetSuperClass() ) )
						{
							DependsOnClass = DependsOnClass->GetSuperClass();
						}

						ExportClassHeader(DependsOnClass,DependsOnClass->GetSuperClass()->HasAnyFlags(RF_TagExp));

						// if the ExportClassHeader() call eventually resulted in calling ExportClassHeader() for the
						// class we're currently working on, it will have the CLASS_Exported flag set, so just stop here
						if ( Class->HasAnyClassFlags(CLASS_Exported) && Class->GetOuter() == Package )
						{
							return;
						}
					}
				}
			}

			if( !DependsOnClass )
			{
				warnf(NAME_Error,TEXT("Unknown class %s used in conjunction with dependson"), *DependencyClassName.ToString());
				return;
			}
		}

		// restore the class's DependsOn list, so that we don't lose dependency information if the current class
		// isn't going to be exported to this header file (i.e. it uses a different custom header file)
		Class->DependentOn = DependentOnClassNames;
	}

	// Export class header.
	if( Class->HasAnyClassFlags(CLASS_Native) && Class->ScriptText && Class->GetOuter() == Package && !(Class->ClassFlags & (CLASS_NoExport|CLASS_Exported)) )
	{
		if(Class->ClassHeaderFilename == ClassHeaderFilename)
		{
			CurrentClass = Class;

			// Mark class as exported.
			Class->ClassFlags |= CLASS_Exported;

			if ( !Class->HasAnyClassFlags(CLASS_Interface) )
			{
				TArray<UEnum*>			Enums;
				TArray<UScriptStruct*>	Structs;
				TArray<UConst*>			Consts;
				TArray<UFunction*>		CallbackFunctions;
				TArray<UFunction*>		NativeFunctions;

				// get the lists of fields that belong to this class that should be exported
				RetrieveRelevantFields(Class, Enums, Structs, Consts, CallbackFunctions, NativeFunctions);

				// export enum declarations
				ExportEnums(Enums);

				// export struct declarations
				ExportStructs(Structs);

				// export #defines for all consts
				ExportConsts(Consts);

				// export parameters structs for all events and delegates
				ExportEventParms(CallbackFunctions);

				UClass* SuperClass = Class->GetSuperClass();

				const TCHAR* ClassCPPName		= NameLookupCPP->GetNameCPP( Class );
				const TCHAR* SuperClassCPPName	= NameLookupCPP->GetNameCPP( SuperClass );

				// Class definition.
				HeaderText.Logf( TEXT("class %s"), ClassCPPName );
				if( SuperClass )
				{
					HeaderText.Logf( TEXT(" : public %s"), SuperClassCPPName );

					// look for multiple inheritance info
					FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);
					const TArray<FMultipleInheritanceBaseClass>& InheritanceParents = ClassData->GetInheritanceParents();
			
					for (INT ParentIndex = 0; ParentIndex < InheritanceParents.Num(); ParentIndex++)
					{
						HeaderText.Logf(TEXT(", public %s"), *InheritanceParents(ParentIndex).ClassName);
					}
				}
				HeaderText.Log( TEXT("\r\n{\r\npublic:\r\n") );

				// export the class property marker
				HeaderText.Logf(TEXT("%s%s %s\r\n"), appSpc(4), PROP_MARKER_BEGIN, *CurrentClass->GetName());
				// Export the class' CPP properties.
				ExportProperties( Class, 0, 1 );
				HeaderText.Logf(TEXT("%s%s %s\r\n\r\n"), appSpc(4), PROP_MARKER_END, *CurrentClass->GetName());

				// if the properties for this native class haven't been changed since the last compile,
				// clear the misaligned flag
				if ( !HavePropertiesChanged() )
				{
					const FClassTree* ClassNode = ClassTree.FindNode(Class);
					ClearMisalignmentFlag(ClassNode);
				}

				// C++ -> UnrealScript stubs (native function execs)
				ExportNativeFunctions(NativeFunctions);

				// UnrealScript -> C++ proxies (events and delegates).
				ExportCallbackFunctions(CallbackFunctions);

				// Build the DECLARE_CLASS line
				{
					FString ClassDeclarationModifier;
					if ( Class->HasAnyClassFlags(CLASS_Abstract) )
					{
						ClassDeclarationModifier = TEXT("ABSTRACT_");
					}

					// this can only happen if a class was marked with a cast flag manually first, since there is no script keyword for this
					const UBOOL bCastedClass = Class->HasAnyCastFlag(CASTCLASS_AllFlags) && Class->GetSuperClass() && Class->ClassCastFlags != Class->GetSuperClass()->ClassCastFlags;
					if ( bCastedClass )
					{
						ClassDeclarationModifier += TEXT("CASTED_");
					}

					HeaderText.Logf( TEXT("%sDECLARE_%sCLASS(%s,"), appSpc(4), *ClassDeclarationModifier, ClassCPPName );
					HeaderText.Logf( TEXT("%s,0"), SuperClassCPPName );

					// append class flags to the definition
					HeaderText.Logf(TEXT("%s,%s"), *GetClassFlagExportText(Class), *Class->GetOuter()->GetName());
					if ( bCastedClass )
					{
						// append platform flags if any are set
						HeaderText.Logf(TEXT(",CASTCLASS_%s"), ClassCPPName);
					}
					HeaderText.Log(TEXT(")\r\n"));
					if(Class->ClassWithin != Class->GetSuperClass()->ClassWithin)
					{
						HeaderText.Logf(TEXT("    DECLARE_WITHIN(%s)\r\n"), NameLookupCPP->GetNameCPP( Class->ClassWithin ) );
					}
				}

				// export the class's config name
				if ( Class->ClassConfigName != NAME_None && Class->ClassConfigName != Class->GetSuperClass()->ClassConfigName )
				{
					HeaderText.Logf(TEXT("    static const TCHAR* StaticConfigName() {return TEXT(\"%s\");}\r\n\r\n"), *Class->ClassConfigName.ToString());
				}

				// arrays for preventing multiple export of accessor function in situations where the native class
				// implements multiple children of the same interface base class
				TArray<UClass*> UObjectExportedInterfaces;
				TArray<UClass*> ExportedInterfaces;

				for ( TMap<UClass*,UProperty*>::TIterator It(Class->Interfaces); It; ++It )
				{
					UClass* InterfaceClass = It.Key();
					if ( InterfaceClass->HasAnyClassFlags(CLASS_Native) )
					{
						for ( UClass* IClass = InterfaceClass; IClass && IClass->HasAnyClassFlags(CLASS_Interface); IClass = IClass->GetSuperClass() )
						{
							if ( IClass != UInterface::StaticClass() && !UObjectExportedInterfaces.ContainsItem(IClass) )
							{
								UObjectExportedInterfaces.AddItem(IClass);
								HeaderText.Logf(TEXT("%svirtual UObject* GetUObjectInterface%s(){return this;}\r\n"), appSpc(4), *IClass->GetName());
							}
						}

#if !EXPORT_NATIVEINTERFACE_UCLASS
						for ( UClass* IClass = InterfaceClass; IClass && IClass->HasAnyClassFlags(CLASS_Interface); IClass = IClass->GetSuperClass() )
						{
							if ( !ExportedInterfaces.ContainsItem(IClass) )
							{
								ExportedInterfaces.AddItem(IClass);

								// we'll need a way to get the interface portion of this UObject, so that we can get an interface pointer from
								// a pointer to a base UObject type.....this requires manually adding a virtual function to the base type that
								// matches this signature, but this is about the best we can do without RTTI
								HeaderText.Logf(TEXT("%svirtual %s* GetInterface%s(){return this;}\r\n"), appSpc(4), NameLookupCPP->GetNameCPP(IClass), *IClass->GetName());
							}
						}
#endif
					}
				}

				FString Filename = GEditor->EditPackagesInPath * *Class->GetOuter()->GetName() * TEXT("Inc") * ClassCPPName + TEXT(".h");
				if( Class->CppText && Class->CppText->Text.Len() )
				{
					HeaderText.Log( *Class->CppText->Text );
				}
				else if( GFileManager->FileSize(*Filename) > 0 )
				{
					HeaderText.Logf( TEXT("    #include \"%s.h\"\r\n"), ClassCPPName );
				}
				else
				{
					HeaderText.Logf( TEXT("    NO_DEFAULT_CONSTRUCTOR(%s)\r\n"), ClassCPPName );
				}

				// End of class.
				HeaderText.Log( TEXT("};\r\n") );

				// End.
				HeaderText.Log( TEXT("\r\n") );
			}
			else
			{
				// this is an interface class
				ExportInterfaceClassDeclaration(Class);
			}
		}
		else if(ExportedParent && !MasterHeader)
		{
			//@todo - need to write a class for detecting circular dependencies between header files
	//		appErrorf(TEXT("Encountered natively exported class with inheritance dependency on parent in another header file(%s:%s depending on %s:%s)."),*Class->GetName(),*Class->ClassHeaderFilename,*Class->GetSuperClass()->GetName(),*Class->GetSuperClass()->ClassHeaderFilename);
		}
	}

	// Export all child classes that are tagged for export.
	for ( INT i = 0; i < Classes.Num(); i++ )
	{
		UClass* ChildClass = Classes(i);
		if ( ChildClass->GetSuperClass() == Class )
		{
			ExportClassHeader(ChildClass,Class->HasAnyFlags(RF_TagExp));
		}
	}
}

/**
 * Returns a string in the format CLASS_Something|CLASS_Something which represents all class flags that are set for the specified
 * class which need to be exported as part of the DECLARE_CLASS macro
 */
FString FNativeClassHeaderGenerator::GetClassFlagExportText( UClass* Class )
{
	FString StaticClassFlagText;

	check(Class);
	if ( Class->HasAnyClassFlags(CLASS_Transient) )
	{
		StaticClassFlagText += TEXT("|CLASS_Transient");
	}				
	if( Class->HasAnyClassFlags(CLASS_Config) )
	{
		StaticClassFlagText += TEXT("|CLASS_Config");
	}
	if( Class->HasAnyClassFlags(CLASS_NativeReplication) )
	{
		StaticClassFlagText += TEXT("|CLASS_NativeReplication");
	}
	if ( Class->HasAnyClassFlags(CLASS_Interface) )
	{
		StaticClassFlagText += TEXT("|CLASS_Interface");
	}

	return StaticClassFlagText;
}

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
void FNativeClassHeaderGenerator::RetrieveRelevantFields(UClass* Class, TArray<UEnum*>& Enums, TArray<UScriptStruct*>& Structs, TArray<UConst*>& Consts, TArray<UFunction*>& CallbackFunctions, TArray<UFunction*>& NativeFunctions)
{
	for ( TFieldIterator<UField,CASTCLASS_None,0> It(Class); It; ++It )
	{
		UField* CurrentField = *It;
		UClass* FieldClass = CurrentField->GetClass();
		if ( FieldClass == UEnum::StaticClass() )
		{
			UEnum* Enum = (UEnum*)CurrentField;
			Enums.AddItem(Enum);
		}

		else if ( FieldClass == UScriptStruct::StaticClass() )
		{
			UScriptStruct* Struct = (UScriptStruct*)CurrentField;
			if ( Struct->HasAnyFlags(RF_Native) || ((Struct->StructFlags&STRUCT_Native) != 0) )
				Structs.AddItem(Struct);
		}

		else if ( FieldClass == UConst::StaticClass() )
		{
			UConst* Const = (UConst*)CurrentField;
			Consts.AddItem(Const);
		}

		else if ( FieldClass == UFunction::StaticClass() )
		{
			UFunction* Function = (UFunction*)CurrentField;
			if ( (Function->FunctionFlags&(FUNC_Event|FUNC_Delegate)) != 0 &&
				Function->GetSuperFunction() == NULL )
			{
				CallbackFunctions.AddItem(Function);
			}

			if ( (Function->FunctionFlags&FUNC_Native) != 0 )
			{
				NativeFunctions.AddItem(Function);
			}
		}
	}
}

/**
* Exports the header text for the list of enums specified
*
* @param	Enums	the enums to export
*/
void FNativeClassHeaderGenerator::ExportEnums( const TArray<UEnum*>& Enums )
{
	// Enum definitions.
	for( INT EnumIdx = 0; EnumIdx < Enums.Num(); EnumIdx++ )
	{
		UEnum* Enum = Enums(EnumIdx);

		// Export enum.
		EnumHeaderText.Logf( TEXT("enum %s\r\n{\r\n"), *Enum->GetName() );
		for( INT i=0; i<Enum->NumEnums(); i++ )
		{
			EnumHeaderText.Logf( TEXT("    %-24s=%i,\r\n"), *Enum->GetEnum(i).ToString(), i );
		}

		EnumHeaderText.Logf( TEXT("};\r\n") );
	}
}

/**
 * Exports the header text for the list of structs specified
 *
 * @param	Structs	the structs to export
 * @param	TextIndent	the current indentation of the header exporter
 */
void FNativeClassHeaderGenerator::ExportStructs( const TArray<UScriptStruct*>& NativeStructs, INT TextIndent/*=0*/ )
{
	// Struct definitions.

	// reverse the order.
	for( INT i=NativeStructs.Num()-1; i>=0; --i )
	{
		UScriptStruct* Struct = NativeStructs(i);

		// Export struct.
		HeaderText.Logf( TEXT("%sstruct %s"), appSpc(TextIndent), NameLookupCPP->GetNameCPP( Struct ) );
		if( Struct->SuperField )
			HeaderText.Logf(TEXT(" : public %s"), NameLookupCPP->GetNameCPP( Struct->GetSuperStruct() ) );
		HeaderText.Logf( TEXT("\r\n%s{\r\n"), appSpc(TextIndent) );

		// export internal structs
		TArray<UScriptStruct*> InternalStructs;
		for ( TFieldIterator<UField,CASTCLASS_None,0> It(Struct); It; ++It )
		{
			UField* CurrentField = *It;
			UClass* FieldClass = CurrentField->GetClass();
			if ( FieldClass == UScriptStruct::StaticClass() )
			{
				UScriptStruct* InnerStruct = (UScriptStruct*)CurrentField;
				if ( InnerStruct->HasAnyFlags(RF_Native) || ((InnerStruct->StructFlags&STRUCT_Native) != 0) )
				{
					InternalStructs.AddItem(InnerStruct);
				}
			}
		}
		ExportStructs(InternalStructs, 4);

		// Export the struct's CPP properties.
		ExportProperties( Struct, TextIndent, TRUE );

		// Export serializer
		if( Struct->StructFlags&STRUCT_Export )
		{
			HeaderText.Logf( TEXT("%sfriend FArchive& operator<<(FArchive& Ar,%s& My%s)\r\n"), appSpc(TextIndent + 4), NameLookupCPP->GetNameCPP( Struct ), *Struct->GetName() );
			HeaderText.Logf( TEXT("%s{\r\n"), appSpc(TextIndent + 4) );
			HeaderText.Logf( TEXT("%sreturn Ar"), appSpc(TextIndent + 8) );

			// if this struct extends another struct, serialize its properties first
			UStruct* SuperStruct = Struct->GetSuperStruct();
			if ( SuperStruct )
			{
				HeaderText.Logf(TEXT(" << (%s&)My%s"), NameLookupCPP->GetNameCPP(SuperStruct), *Struct->GetName());
			}

			for ( TFieldIterator<UProperty,CASTCLASS_UProperty,FALSE> StructProp(Struct); StructProp != NULL; ++StructProp )
			{
				FString PrefixText;
				if ( StructProp->IsA(UObjectProperty::StaticClass()) )
				{
					PrefixText = TEXT("(UObject*&)");
				}
				if( StructProp->ElementSize > 0 )
				{
					if( StructProp->ArrayDim > 1 )
					{
						for( INT i = 0; i < StructProp->ArrayDim; i++ )
						{
							HeaderText.Logf( TEXT(" << %sMy%s.%s[%d]"), *PrefixText, *Struct->GetName(), *StructProp->GetName(), i );
						}
					}
					else
					{
						HeaderText.Logf( TEXT(" << %sMy%s.%s"), *PrefixText, *Struct->GetName(), *StructProp->GetName() );
					}
				}
			}
			HeaderText.Logf( TEXT(";\r\n%s}\r\n"), appSpc(TextIndent + 4) );
		}

		// if the struct included cpptext, emit that now
		if ( Struct->CppText )
		{
			HeaderText.Logf(TEXT("%s%s\r\n"), appSpc(TextIndent), *Struct->CppText->Text);
		}
		else
		{
			FStringOutputDevice CtorAr, InitializationAr;

			if ( (Struct->StructFlags&STRUCT_Transient) != 0 )
			{
				INT PropIndex = 0;				

				// if the struct is transient, export initializers for the properties in the struct
				for ( TFieldIterator<UProperty,CASTCLASS_UProperty,FALSE> StructProp(Struct); StructProp; ++StructProp )
				{
					UProperty* Prop = *StructProp;
					if ( (Prop->PropertyFlags&CPF_NeedCtorLink) == 0 )
					{
						if ( Prop->GetClass() != UStructProperty::StaticClass() ) // special case: constructors are called for any members which are structs when the outer structs constructor is called
						{
							InitializationAr.Logf(TEXT("%s%s %s(%s)\r\n"), appSpc(TextIndent + 4), PropIndex++ == 0 ? TEXT(":") : TEXT(","), *Prop->GetName(), *GetNullParameterValue(Prop,FALSE,TRUE));
						}
					}
					else if ( Prop->GetClass() == UStructProperty::StaticClass() )
					{
						InitializationAr.Logf(TEXT("%s%s %s(EC_EventParm)\r\n"), appSpc(TextIndent + 4), PropIndex++ == 0 ? TEXT(":") : TEXT(","), *Prop->GetName());
					}
				}
			}

			if ( InitializationAr.Len() > 0 )
			{
				CtorAr.Logf(TEXT("%s%s()\r\n"), appSpc(TextIndent + 4), NameLookupCPP->GetNameCPP(Struct));
				CtorAr.Log(*InitializationAr);
				CtorAr.Logf(TEXT("%s{}\r\n"), appSpc(TextIndent + 4));
			}
			else
			{
				CtorAr.Logf(TEXT("%s%s() {}\r\n"), appSpc(TextIndent + 4), NameLookupCPP->GetNameCPP(Struct));
			}

			// generate the event parm constructor
			CtorAr.Logf(TEXT("%s%s(EEventParm)\r\n"), appSpc(TextIndent + 4), NameLookupCPP->GetNameCPP( Struct ));
			CtorAr.Logf(TEXT("%s{\r\n"), appSpc(TextIndent + 4));
			CtorAr.Logf(TEXT("%sappMemzero(this, sizeof(%s));\r\n"), appSpc(TextIndent + 8), NameLookupCPP->GetNameCPP( Struct ));
			CtorAr.Logf(TEXT("%s}\r\n"), appSpc(TextIndent + 4));

			if ( CtorAr.Len() > 0 )
			{
				HeaderText.Logf(TEXT("\r\n%s/** Constructors */\r\n"), appSpc(TextIndent + 4));
				HeaderText.Logf(*CtorAr);
			}
		}

		HeaderText.Logf( TEXT("%s};\r\n\r\n"), appSpc(TextIndent) );
	}
}


/**
 * Exports the header text for the list of consts specified
 *
 * @param	Consts	the consts to export
 */
void FNativeClassHeaderGenerator::ExportConsts( const TArray<UConst*>& Consts )
{
	// Constants.
	for( INT i = 0; i < Consts.Num(); i++ )
	{
		UConst* Const = Consts(i);
		FString V = Const->Value;

		// remove all leading whitespace from the value of the const
		while( V.Len() > 0 && appIsWhitespace(**V) )
			V=V.Mid(1);

		// remove all trailing whitespace from the value of the const
		while ( V.Len() > 0 && appIsWhitespace(V.Right(1)[0]) )
			V = V.LeftChop(1);

		// remove literal name delimiters, if they exist
		if( V.Len()>1 && V.Left(1)==TEXT("'") && V.Right(1)==TEXT("'") )
			V = V.Mid(1,V.Len()-2);

		// if this is a string, wrap it with the TEXT macro
		if ( V.Len()>1 && V.Left(1)==TEXT("\"") && V.Right(1)==TEXT("\"") )
		{
			V = FString::Printf(TEXT("TEXT(%s)"), *V);
		}
		HeaderText.Logf( TEXT("#define UCONST_%s %s\r\n"), *Const->GetName(), *V );
	}
	if( Consts.Num() > 0 )
	{
		HeaderText.Logf( TEXT("\r\n") );
	}
}

/**
* Exports the parameter struct declarations for the list of functions specified
*
* @param	CallbackFunctions	the functions that have parameters which need to be exported
*/
void FNativeClassHeaderGenerator::ExportEventParms( const TArray<UFunction*>& CallbackFunctions )
{
	// Parms struct definitions.
	for ( INT i = 0; i < CallbackFunctions.Num(); i++ )
	{
		UFunction* Function = CallbackFunctions(i);
		FString EventParmStructName = FString::Printf(TEXT("%s_event%s_Parms"), *Function->GetOwnerClass()->GetName(), *Function->GetName() );
		HeaderText.Logf( TEXT("struct %s\r\n"), *EventParmStructName);
		HeaderText.Log( TEXT("{\r\n") );

		// keep track of any structs which contain properties that require construction that
		// are used as a parameter for this event call
		TArray<UStructProperty*> StructEventParms;
		for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(Function); It && (It->PropertyFlags&CPF_Parm); ++It )
		{
			UProperty* Prop = *It;

			FStringOutputDevice PropertyText;
			PropertyText.Log( appSpc(4) );
			if (Prop->HasAnyPropertyFlags(CPF_Const)
			&&	Cast<UObjectProperty>(Prop,CLASS_IsAUObjectProperty) != NULL )
			{
				PropertyText.Log(TEXT("const "));
			}
			Prop->ExportCppDeclaration( PropertyText, 0, 0, 0 );
			ApplyAlternatePropertyExportText(Prop, PropertyText);
			
			PropertyText.Log( TEXT(";\r\n") );
			HeaderText += *PropertyText;

			UStructProperty* StructProp = Cast<UStructProperty>(Prop,CLASS_IsAUStructProperty);
			if ( StructProp != NULL )
			{
				// if this struct contains any properties which are exported as NoInit types, the event parm struct needs to call the
				// EEventParm ctor on the struct to initialize those properties to zero
				for ( UProperty* ConstructorProp = StructProp->Struct->ConstructorLink; ConstructorProp; ConstructorProp = ConstructorProp->ConstructorLinkNext )
				{
					if ( (ConstructorProp->PropertyFlags&CPF_AlwaysInit) == 0 )
					{
						StructEventParms.AddItem(StructProp);
						break;
					}
				}
			}	
		}

		// Export event parameter constructor, which will call the EEventParm constructor for any struct parameters
		// which contain properties that are NoInit
		HeaderText.Logf(TEXT("%s%s(EEventParm)\r\n"), appSpc(4), *EventParmStructName);
		for( INT i = 0; i < StructEventParms.Num(); i++ )
		{
			HeaderText.Logf(TEXT("%s%s %s(EC_EventParm)\r\n"), appSpc(4), i == 0 ? TEXT(":") : TEXT(","), *StructEventParms(i)->GetName());
		}
		HeaderText.Logf(TEXT("%s{\r\n"), appSpc(4));
		HeaderText.Logf(TEXT("%s}\r\n"), appSpc(4));
		HeaderText.Log( TEXT("};\r\n") );
	}
}

/**
 * Get the intrinsic null value for this property
 * 
 * @param	Prop				the property to get the null value for
 * @param	bMacroContext		TRUE when exporting the P_GET* macro, FALSE when exporting the friendly C++ function header
 * @param	bTranslatePointers	if true, FPointer structs will be set to NULL instead of FPointer()
 *
 * @return	the intrinsic null value for the property (0 for ints, TEXT("") for strings, etc.)
 */
FString FNativeClassHeaderGenerator::GetNullParameterValue( UProperty* Prop, UBOOL bMacroContext, UBOOL bTranslatePointers/*=FALSE*/ )
{
	if ( !bMacroContext && Prop->HasAllPropertyFlags(CPF_OptionalParm|CPF_OutParm) )
	{
		return TEXT("NULL");
	}

	UClass* PropClass = Prop->GetClass();
	if (PropClass == UByteProperty::StaticClass()
	||	PropClass == UIntProperty::StaticClass()
	||	PropClass == UBoolProperty::StaticClass()
	||	PropClass == UFloatProperty::StaticClass() )
	{
        // if we have a BoolProperty then set it to be FALSE instead of 0
 		if( PropClass == UBoolProperty::StaticClass() )
 		{
 			return TEXT("FALSE");
 		}

		return TEXT("0");
	}
	else if ( PropClass == UNameProperty::StaticClass() )
	{
		return TEXT("NAME_None");
	}
	else if ( PropClass == UStrProperty::StaticClass() )
	{
		return TEXT("TEXT(\"\")");
	}
	else if ( PropClass == UArrayProperty::StaticClass()
		||    PropClass == UMapProperty::StaticClass()
		||    PropClass == UStructProperty::StaticClass()
		||    PropClass == UDelegateProperty::StaticClass() )
	{
		if ( bTranslatePointers && Prop->GetFName() == NAME_Pointer && PropClass == UStructProperty::StaticClass() )
		{
			return TEXT("NULL");
		}
		else
		{
			FString Type, ExtendedType;
			Type = Prop->GetCPPType(&ExtendedType,CPPF_OptionalValue);
			return Type + ExtendedType + TEXT("(EC_EventParm)");
		}
	}
	else if ( PropClass->HasAnyCastFlag(CASTCLASS_UObjectProperty) )
	{
		return TEXT("NULL");
	}
	else if ( PropClass == UInterfaceProperty::StaticClass() )
	{
		return bMacroContext ? TEXT("") : TEXT("NULL");
	}

	appErrorf(TEXT("GetNullParameterValue - Unhandled property type '%s': %s"), *PropClass->GetName(), *Prop->GetPathName());
	return TEXT("");
}

/**
 * Retrieve the default value for an optional parameter
 *
 * @param	Prop			the property being parsed
 * @param	bMacroContext	TRUE when exporting the P_GET* macro, FALSE when exporting the friendly C++ function header
 * @param	DefaultValue	[out] filled in with the default value text for this parameter
 *
 * @return	TRUE if default value text was successfully retrieved for this property
 */
UBOOL FNativeClassHeaderGenerator::GetOptionalParameterValue( UProperty* Prop, UBOOL bMacroContext, FString& DefaultValue )
{
	FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);

	FTokenData* PropData = ClassData->FindTokenData(Prop);
	UBOOL bResult = PropData != NULL && PropData->DefaultValue != NULL;
	if ( bResult )
	{
		FTokenChain& TokenChain = PropData->DefaultValue->ParsedExpression;
		INT StartIndex = 0;

		// if the last link in the token chain is a literal value, we only need to export that
		if ( TokenChain.Top().TokenType == TOKEN_Const )
		{
			StartIndex = TokenChain.Num() - 1;
		}

		for ( INT TokenIndex = StartIndex; TokenIndex < TokenChain.Num(); TokenIndex++ )
		{
			FToken& Token = TokenChain(TokenIndex);

			// TokenType is TOKEN_None for functions
			if ( Token.TokenType != TOKEN_None )
			{
				if ( Token.TokenType == TOKEN_Const )
				{
					// constant value; either a literal or a reference to a script const
					if ( Token.Type == CPT_String )
					{
						DefaultValue += FString::Printf(TEXT("TEXT(\"%s\")"), *Token.GetValue());
					}
					else if ( Token.IsObject() )
					{
						// literal reference to object - these won't really work very well, but we'll try...

						// let's find out what other types there are  =)
						if  ( Token.TokenName != NAME_Class )
						{
							appErrorf(TEXT("The only type of explicit object reference allowed as the default value for an optional parameter of a native function is a class (%s.%s)"), *CurrentClass->GetName(), *Prop->GetOuter()->GetName());
						}

						if ( !Token.MetaClass->HasAnyClassFlags(CLASS_Native) )
						{
							appErrorf(TEXT("Not allowed to use an explicit reference to a non-native class as the default value for an optional parameter of a native function (%s.%s)"), *CurrentClass->GetName(), *Prop->GetOuter()->GetName());
						}

						if ( bMacroContext || TokenChain.Num() == 1 )
						{
							const TCHAR* MetaClassName = NameLookupCPP->GetNameCPP(Token.MetaClass);
							DefaultValue += FString::Printf(TEXT("%s::StaticClass()"), MetaClassName);

							// if this isn't the last token in the chain, emit the member access operator
							if ( TokenIndex < TokenChain.Num() - 1 )
							{
								DefaultValue += FString::Printf(TEXT("->GetDefaultObject<%s>()->"), MetaClassName);
							}
						}
						else
						{
							bResult = FALSE;
							break;
						}
					}
					else if (Token.Type == CPT_Name)
					{
						// name - need to add in FName constructor stuff
						DefaultValue += FString::Printf(TEXT("FName(TEXT(\"%s\"))"), *Token.GetValue());
					}
					else
					{
						DefaultValue += Token.GetValue();
					}
				}
				else if ( Token.TokenType == TOKEN_Identifier )
				{
					if ( Token.Type == CPT_Struct )
					{
						DefaultValue += Token.Identifier;
						// if this isn't the last token in the chain, emit the member access operator
						if ( TokenIndex < TokenChain.Num() - 1 )
						{
							DefaultValue += TCHAR('.');
						}
					}
					else if ( Token.IsObject() )
					{
						DefaultValue += Token.Identifier;

						// if this isn't the last token in the chain, emit the member access operator
						if ( TokenIndex < TokenChain.Num() - 1 )
						{
							DefaultValue += TEXT("->");
						}
					}
					else if (bMacroContext)
					{
						if ( Token.Type == CPT_None )
						{
							if ( Token.Matches(NAME_Default) || Token.Matches(NAME_Static) )
							{
								// reference to the owning class's default object
								DefaultValue += FString::Printf(TEXT("GetClass()->GetDefaultObject<%s>()"), NameLookupCPP->GetNameCPP(CurrentClass));
								// if this isn't the last token in the chain, emit the member access operator
								if ( TokenIndex < TokenChain.Num() - 1 )
								{
									DefaultValue += TEXT("->");
								}
							}
							else if ( Token.TokenName == NAME_Super )
							{
								DefaultValue += FString::Printf(TEXT("%s::"), Token.MetaClass
										? NameLookupCPP->GetNameCPP(Token.MetaClass)
										: TEXT("Super"));
							}
							else
							{
								bResult = FALSE;
								break;
							}
						}
						else
						{
							DefaultValue += Token.Identifier;
						}
					}
					else
					{
						bResult = FALSE;
						break;
					}
				}
				else
				{
					appErrorf(TEXT("Unhandled token type %i in GetOptionalParameterValue!"), (INT)Token.TokenType);
				}
			}
			else if ( Token.Type == CPT_ObjectReference && Token.TokenName == NAME_Self )
			{
				if ( bMacroContext )
				{
					DefaultValue += TEXT("this");
				}
				else
				{
					FStringOutputDevice InvalidObjectString = TEXT("((");
					Prop->ExportCppDeclaration(InvalidObjectString, FALSE, TRUE, FALSE);
					ApplyAlternatePropertyExportText(Prop, InvalidObjectString);

					// string now looks like "((class UObject* PropName", so cut off the string at the last space character
					INT SpacePos = InvalidObjectString.InStr(TEXT(" "), TRUE);
					check(SpacePos>0);

					DefaultValue += InvalidObjectString.Left(SpacePos) + TEXT(")-1)");
				}
			}
			else if ( bMacroContext )
			{
				//@todo Super.FunctionCall still broken

				// it's a function call - these can only be used in the P_GET macros
				if ( Token.TokenFunction != NULL )
				{
					const FFuncInfo& FunctionData = Token.TokenFunction->GetFunctionData();
					UFunction* Function = FunctionData.FunctionReference;

					const UBOOL bIsEvent = Function->HasAnyFunctionFlags(FUNC_Event);
					const UBOOL bIsDelegate = Function->HasAnyFunctionFlags(FUNC_Delegate);
					const UBOOL bIsNativeFunc = Function->HasAnyFunctionFlags(FUNC_Native) && (FunctionData.FunctionExportFlags&FUNCEXPORT_NoExport) == 0;

					if ( !bIsEvent && !bIsDelegate && !bIsNativeFunc )
					{
						appErrorf(TEXT("Invalid optional parameter value in (%s.%s).  The only function type allowed as the default value for an optional parameter of a native function is event, delegate, or exported native function"), *CurrentClass->GetName(), *Prop->GetOuter()->GetName());
					}

					//@todo: process the parameter values to translate context references (i.e. replace . with -> for objects, etc.)
					if ( bIsEvent )
					{
						DefaultValue += TEXT("event");
					}
					else if ( bIsDelegate )
					{
						DefaultValue += TEXT("delegate");
					}
					DefaultValue += PropData->DefaultValue->RawExpression;
				}
			}
			else
			{
				bResult = FALSE;
				break;
			}
		}
	}

	if ( bResult && DefaultValue.Len() == 1 )
	{
		bResult = FALSE;
	}
	return bResult;
}

/**
 * Exports a native function prototype
 *
 * @param	FunctionData	data representing the function to export
 * @param	bEventTag		TRUE to export this function prototype as an event stub, FALSE to export as a native function stub.
 *							Has no effect if the function is a delegate.
 * @param	Return			[out] will be assigned to the return value for this function, or NULL if the return type is void
 * @param	Parameters		[out] will be filled in with the parameters for this function
 */
void FNativeClassHeaderGenerator::ExportNativeFunctionHeader( const FFuncInfo& FunctionData, UBOOL bEventTag, UProperty*& Return, TArray<UProperty*>&Parameters )
{
	UFunction* Function = FunctionData.FunctionReference;

	UBOOL bIsInterface = Function->GetOwnerClass()->HasAnyClassFlags(CLASS_Interface);
	UBOOL bOutputEnabled = bEventTag || (FunctionData.FunctionExportFlags&FUNCEXPORT_NoExportHeader) == 0;
	

	// Return type.
	Return = Function->GetReturnProperty();
	if (bOutputEnabled)
	{
		HeaderText.Log( appSpc(4) );

		// if the owning class is an interface class
		if ( bIsInterface ||
		//	or this is not an event, AND
		(	!bEventTag
		//	the function is not a static function, AND
		&&	!Function->HasAnyFunctionFlags(FUNC_Static)
		//	the function is either virtual or not marked final
		&& ((FunctionData.FunctionExportFlags&FUNCEXPORT_Virtual) != 0 || (FunctionData.FunctionExportFlags&FUNCEXPORT_Final) == 0)) )
		{
			HeaderText.Log(TEXT("virtual "));
		}

		if( !Return )
		{
			HeaderText.Log( TEXT("void") );
		}
		else
		{
			FString ReturnType, ExtendedReturnType;
			ReturnType = Return->GetCPPType(&ExtendedReturnType);
			HeaderText.Logf(TEXT("%s%s"), *ReturnType, *ExtendedReturnType);
		}

		// Function name and parms.
		FString FunctionType;
		if( Function->HasAnyFunctionFlags(FUNC_Delegate) )
		{
			FunctionType = TEXT("delegate");
		}
		else if ( bEventTag )
		{
			FunctionType = TEXT("event");
		}
		else
		{
			// nothing
		}

		HeaderText.Logf( TEXT(" %s%s("), *FunctionType, *Function->GetName() );
	}

	INT ParmCount=0;
	Parameters.Empty();
	UBOOL bHasOptionalParms = FALSE;
	for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(Function); It && (It->PropertyFlags&(CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
	{
		Parameters.AddItem(*It);
		if (bOutputEnabled)
		{
			if( ParmCount++ )
			{
				HeaderText.Log(TEXT(","));
			}

			FStringOutputDevice PropertyText;
			It->ExportCppDeclaration( PropertyText, FALSE, TRUE, FALSE );
			ApplyAlternatePropertyExportText(*It, PropertyText);

			HeaderText += PropertyText;
			if ( It->HasAnyPropertyFlags(CPF_OptionalParm) )
			{
				bHasOptionalParms = TRUE;

				FString DefaultValue=TEXT("=");
				if ( !GetOptionalParameterValue(*It, FALSE, DefaultValue) )
				{
					DefaultValue += GetNullParameterValue(*It,FALSE);
				}

				HeaderText.Log(*DefaultValue);
			}
		}
	}
	if (bOutputEnabled)
	{
		HeaderText.Log( TEXT(")") );
		if ( (FunctionData.FunctionExportFlags & FUNCEXPORT_Const) != 0 )
		{
			HeaderText.Log( TEXT(" const") );
		}

		if ( bIsInterface )
		{
			// all methods in interface classes are pure virtuals
			HeaderText.Log(TEXT("=0"));
		}
	}
}

/**
 * Exports the native stubs for the list of functions specified
 *
 * @param	NativeFunctions	the functions to export
 */
void FNativeClassHeaderGenerator::ExportNativeFunctions( const TArray<UFunction*>& NativeFunctions )
{
	// This is used to allow the C++ declarations and stubs to be separated without iterating through the list of functions twice
	// (put the stubs in this archive, then append this archive to the main archive once we've exported all declarations)
	FStringOutputDevice UnrealScriptWrappers;

	// find the class info for this class
	FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);

	// export the C++ stubs
	for ( INT i = NativeFunctions.Num() - 1; i >= 0; i-- )
	{
		UFunction* Function = NativeFunctions(i);
		FFunctionData* CompilerInfo = ClassData->FindFunctionData(Function);
		check(CompilerInfo);

		const FFuncInfo& FunctionData = CompilerInfo->GetFunctionData();
		
		UProperty* Return = NULL;
		TArray<UProperty*> Parameters;

		UBOOL bExportExtendedWrapper = FALSE;
		if ( (FunctionData.FunctionExportFlags&FUNCEXPORT_NoExport) == 0 &&
			(Function->FunctionFlags&FUNC_Iterator) == 0 &&
			(Function->FunctionFlags&FUNC_Operator) == 0 )
			bExportExtendedWrapper = TRUE;

		if ( bExportExtendedWrapper)
		{
			ExportNativeFunctionHeader(FunctionData,FALSE,Return,Parameters);
			if ((FunctionData.FunctionExportFlags&FUNCEXPORT_NoExportHeader) == 0)
			{
				HeaderText.Logf(TEXT(";%s"), LINE_TERMINATOR);
			}
		}

		// if this function was originally declared in a base class, and it isn't a static function,
		// only the C++ function header will be exported
		if ( !ShouldExportFunction(Function) )
			continue;

		// export the script wrappers
		UnrealScriptWrappers.Logf( TEXT("%sDECLARE_FUNCTION(exec%s)"), appSpc(4), *Function->GetName() );

		if ( bExportExtendedWrapper )
		{
			UnrealScriptWrappers.Logf(TEXT("%s%s{%s"), LINE_TERMINATOR, appSpc(4), LINE_TERMINATOR);

			// export the GET macro for this parameter
			FString ParameterList;
			for ( INT ParameterIndex = 0; ParameterIndex < Parameters.Num(); ParameterIndex++ )
			{
				FString EvalBaseText = TEXT("P_GET_");	// e.g. P_GET_STR
				FString EvalModifierText;				// e.g. _OPTX, _OPTX_REF
				FString EvalParameterText;				// e.g. (UObject*,NULL)


				UProperty* Param = Parameters(ParameterIndex);
				FString TypeText;
				if ( Param->ArrayDim > 1 )
				{
					EvalBaseText += TEXT("ARRAY");
					// questionable
					TypeText = Param->GetCPPMacroType(TypeText);
				}
				else
				{
					EvalBaseText += Param->GetCPPMacroType(TypeText);
				}

				FString DefaultValueText;
				if ( Param->GetClass() == UStructProperty::StaticClass() )
				{
					// if this is a struct property which contains NoInit types
					UStructProperty* StructParam = ExactCast<UStructProperty>(Param);
					if ( (StructParam->Struct->StructFlags&STRUCT_Transient) == 0 &&
						StructParam->Struct->ConstructorLink != NULL )
					{
						EvalModifierText += TEXT("_INIT");
					}
				}

				// if this property is an optional parameter, add the OPTX tag
				if ( (Param->PropertyFlags&CPF_OptionalParm) != 0 )
				{
					DefaultValueText = TEXT(",");
					EvalModifierText += TEXT("_OPTX");
					if ( !GetOptionalParameterValue(Param, TRUE, DefaultValueText) )
					{
						DefaultValueText += GetNullParameterValue(Param, TRUE);
					}
				}

				// if this property is an out parm, add the REF tag
				if ( (Param->PropertyFlags&CPF_OutParm) != 0 )
				{
					EvalModifierText += TEXT("_REF");
				}

				// if this property requires a specialization, add a comma to the type name so we can print it out easily
				if ( TypeText != TEXT("") )
				{
					TypeText += TCHAR(',');
				}

				EvalParameterText = FString::Printf(TEXT("(%s%s%s)"), *TypeText, *Param->GetName(), *DefaultValueText);

				UnrealScriptWrappers.Logf(TEXT("%s%s%s%s;%s"), appSpc(8), *EvalBaseText, *EvalModifierText, *EvalParameterText, LINE_TERMINATOR);

				// add this property to the parameter list string
				if ( ParameterList.Len() )
				{
					ParameterList += TCHAR(',');
				}

				if ( Param->HasAllPropertyFlags(CPF_OptionalParm|CPF_OutParm) && Param->ArrayDim == 1 )
				{
					ParameterList += FString::Printf(TEXT("p%s ? &%s : NULL"), *Param->GetName(), *Param->GetName());
				}
				else
				{
					ParameterList += Param->GetName();
				}
			}

			UnrealScriptWrappers.Logf(TEXT("%sP_FINISH;%s"), appSpc(8), LINE_TERMINATOR);

			// write out the return value
			UnrealScriptWrappers.Log(appSpc(8));
			if ( Return != NULL )
			{
				FString ReturnType, ReturnExtendedType;
				ReturnType = Return->GetCPPType(&ReturnExtendedType);
				UnrealScriptWrappers.Logf(TEXT("*(%s%s*)Result="), *ReturnType, *ReturnExtendedType);
			}

			// export the call to the C++ version
			UnrealScriptWrappers.Logf(TEXT("%s(%s);%s"), *Function->GetName(), *ParameterList, LINE_TERMINATOR);
			UnrealScriptWrappers.Logf(TEXT("%s}%s"), appSpc(4), LINE_TERMINATOR);
		}
		else
		{
			UnrealScriptWrappers.Logf(TEXT(";%s"), LINE_TERMINATOR);
		}
	}

	HeaderText += UnrealScriptWrappers;
}

TMap<class UFunction*,INT> FuncEmitCountMap;

/**
 * Exports the methods which trigger UnrealScript events and delegates.
 *
 * @param	CallbackFunctions	the functions to export
 */
void FNativeClassHeaderGenerator::ExportCallbackFunctions( const TArray<UFunction*>& CallbackFunctions )
{
	// find the class info for this class
	FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);
	UBOOL bIsInterface = CurrentClass->HasAnyClassFlags(CLASS_Interface);

	for ( INT CallbackIndex = 0; CallbackIndex < CallbackFunctions.Num(); CallbackIndex++ )
	{
		UFunction* Function = CallbackFunctions(CallbackIndex);
		UClass* Class = CurrentClass;

		FFunctionData* CompilerInfo = ClassData->FindFunctionData(Function);
		check(CompilerInfo);

		const FFuncInfo& FunctionData = CompilerInfo->GetFunctionData();

		UProperty* Return = NULL;
		TArray<UProperty*> Parameters;

		// export the line that looks like: INT eventMain(const FString& Parms)
		ExportNativeFunctionHeader(FunctionData,TRUE,Return,Parameters);
		if ( bIsInterface )
		{
			HeaderText.Log(TEXT(";\r\n"));
			continue;
		}

		const UBOOL ProbeOptimization = (Function->GetFName().GetIndex()>=NAME_PROBEMIN && Function->GetFName().GetIndex()<NAME_PROBEMAX);

		// cache the TCHAR* for a few strings we'll use a lot here
		const FString FunctionName = Function->GetName();
		const FString ReturnName = Return ? Return->GetName() : TEXT("");

		// now the body - first we need to declare a struct which will hold the parameters for the event/delegate call
		HeaderText.Logf( TEXT("\r\n%s{\r\n"), appSpc(4) );

		// declare and zero-initialize the parameters and return value, if applicable
		if( Return != NULL || Parameters.Num() > 0 )
		{
			// export a line which will invoke the struct using the EC_EventParm ctor
			HeaderText.Logf( TEXT("%s%s_event%s_Parms Parms(EC_EventParm);\r\n"), appSpc(8), *Class->GetName(), *FunctionName );
			if ( Return != NULL )
			{
				// if we have a return value, initialize to 0 for types that require initialization.
				if ( Cast<UStructProperty>(Return,CLASS_IsAUStructProperty) != NULL )
				{
					HeaderText.Logf( TEXT("%sappMemzero(&Parms.%s,sizeof(Parms.%s));\r\n"), appSpc(8), *ReturnName, *ReturnName );
				}
				else if ( Cast<UStrProperty>(Return) == NULL )
				{
					const FString NullValue = GetNullParameterValue(Return, TRUE, FALSE);
					if ( NullValue.Len() > 0 )
					{
						HeaderText.Logf(TEXT("%sParms.%s=%s;\r\n"), appSpc(8), *ReturnName, *NullValue);
					}
				}
			}
		}
		if( ProbeOptimization )
		{
			HeaderText.Logf(TEXT("%sif(IsProbing(NAME_%s)) {\r\n"), appSpc(8), *FunctionName);
		}

		if( Return != NULL || Parameters.Num() > 0 )
		{
			// Declare a parameter struct for this event/delegate and assign the struct members using the values passed into the event/delegate call.
			for ( INT ParmIndex = 0; ParmIndex < Parameters.Num(); ParmIndex++ )
			{
				UProperty* Prop = Parameters(ParmIndex);
				const FString PropertyName = Prop->GetName();

				if ( Prop->ArrayDim > 1 )
				{
					HeaderText.Logf( TEXT("%sappMemcpy(Parms.%s,%s,sizeof(Parms.%s));\r\n"), appSpc(8), *PropertyName, *PropertyName, *PropertyName );
				}
				else
				{
					FString ValueAssignmentText = PropertyName;
					if ( Cast<UBoolProperty>(Prop) != NULL )
					{
						ValueAssignmentText += TEXT(" ? FIRST_BITFIELD : FALSE");
					}
					else if ( Prop->HasAnyPropertyFlags(CPF_OptionalParm) && Cast<UObjectProperty>(Prop) != NULL )
					{
						// if it's an optional object parameter and the default value specified in script was "Self", the exporter
						// will convert that to INVALID_OBJECT....convert it back now.
						FString DefaultValue;
						if (GetOptionalParameterValue(Prop, FALSE, DefaultValue)
						&&	DefaultValue.Right(3) == TEXT("-1)") )
						{
							ValueAssignmentText += TEXT("==INVALID_OBJECT ? this : ");
							ValueAssignmentText += PropertyName;
						}
					}

					if ( Prop->HasAllPropertyFlags(CPF_OptionalParm|CPF_OutParm) )
					{
						HeaderText.Logf(TEXT("%sif(%s){Parms.%s=*%s;}\r\n"), appSpc(8), *PropertyName, *PropertyName, *ValueAssignmentText);
					}
					else
					{
						HeaderText.Logf( TEXT("%sParms.%s=%s;\r\n"), appSpc(8), *PropertyName, *ValueAssignmentText );
					}
				}
			}
			if( Function->HasAnyFunctionFlags(FUNC_Delegate) )
			{
				HeaderText.Logf( TEXT("%sProcessDelegate(%s_%s,&__%s__Delegate,&Parms);\r\n"), appSpc(8), *API, *FunctionName, *FunctionName );
			}
			else
			{
				HeaderText.Logf( TEXT("%sProcessEvent(FindFunctionChecked(%s_%s),&Parms);\r\n"), appSpc(8), *API, *FunctionName );
			}
		}
		else
		{
			if( Function->HasAnyFunctionFlags(FUNC_Delegate) )
			{
				HeaderText.Logf( TEXT("%sProcessDelegate(%s_%s,&__%s__Delegate,NULL);\r\n"), appSpc(8), *API, *FunctionName, *FunctionName );
			}
			else
			{
				HeaderText.Logf( TEXT("%sProcessEvent(FindFunctionChecked(%s_%s),NULL);\r\n"), appSpc(8), *API, *FunctionName );
			}
		}
		if( ProbeOptimization )
		{
			HeaderText.Logf(TEXT("%s}\r\n"), appSpc(8));
		}

		// Out parm copying.
		for ( INT ParmIndex = 0; ParmIndex < Parameters.Num(); ParmIndex++ )
		{
			UProperty* Prop = Parameters(ParmIndex);

			const FString PropertyName = Prop->GetName();
			if ( Prop->HasAnyPropertyFlags(CPF_OutParm) 
			&&	(!Prop->HasAnyPropertyFlags(CPF_Const)
				|| Cast<UObjectProperty>(Prop) != NULL) )
			{
				if ( Prop->ArrayDim > 1 )
				{
					HeaderText.Logf( TEXT("%sappMemcpy(&%s,&Parms.%s,sizeof(%s));\r\n"), appSpc(8), *PropertyName, *PropertyName, *PropertyName );
				}
				else if ( Prop->HasAnyPropertyFlags(CPF_OptionalParm) )
				{
					HeaderText.Logf(TEXT("%sif ( %s ) { *%s=Parms.%s; }\r\n"), appSpc(8), *PropertyName, *PropertyName, *PropertyName);
				}
				else
				{
					HeaderText.Logf(TEXT("%s%s=Parms.%s;\r\n"), appSpc(8), *PropertyName, *PropertyName);
				}
			}
		}

		// Return value.
		if( Return )
		{
			HeaderText.Logf( TEXT("%sreturn Parms.%s;\r\n"), appSpc(8), *ReturnName );
		}
		HeaderText.Logf( TEXT("%s}\r\n"), appSpc(4) );
	}
}


/**
 * Determines if the property has alternate export text associated with it and if so replaces the text in PropertyText with the
 * alternate version. (for example, structs or properties that specify a native type using export-text).  Should be called immediately
 * after ExportCppDeclaration()
 *
 * @param	Prop			the property that is being exported
 * @param	PropertyText	the string containing the text exported from ExportCppDeclaration
 */
void FNativeClassHeaderGenerator::ApplyAlternatePropertyExportText( UProperty* Prop, FStringOutputDevice& PropertyText )
{
	// if this is a pointer property and a pointer type was specified, export that type rather than FPointer
	UStructProperty* StructProp = Cast<UStructProperty>(Prop,CLASS_IsAUStructProperty);
	if ( StructProp == NULL )
	{
		UArrayProperty* ArrayProp = Cast<UArrayProperty>(Prop);
		if ( ArrayProp != NULL )
		{
			StructProp = Cast<UStructProperty>(ArrayProp->Inner,CLASS_IsAUStructProperty);
		}
	}
	if ( StructProp )
	{
		// find the class info for this class
		FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);

		// find the compiler token for this property
		FTokenData* PropData = ClassData->FindTokenData(Prop);
		if ( PropData != NULL )
		{
			FString ExportText;
			if ( PropData->Token.ExportInfo.Len() > 0 )
			{
				ExportText = PropData->Token.ExportInfo;
			}
			else
			{
				// we didn't have any export-text associated with the variable, so check if we have
				// anything for the struct itself
				FStructData* StructData = ClassData->FindStructData(StructProp->Struct);
				if ( StructData != NULL && StructData->StructData.ExportInfo.Len() )
				{
					ExportText = StructData->StructData.ExportInfo;
				}
			}

			if ( ExportText.Len() > 0 )
			{
				// special case the pointer struct - add the asterisk for them
				if ( StructProp->Struct->GetFName() == NAME_Pointer )
				{
					ExportText += TEXT("*");
				}

				(FString&)PropertyText = PropertyText.Replace(NameLookupCPP->GetNameCPP(StructProp->Struct), *ExportText);
			}
		}
	}
	else
	{
		UMapProperty* MapProp = Cast<UMapProperty>(Prop);
		if ( MapProp != NULL )
		{
			// find the class info for this class
			FClassMetaData* ClassData = GScriptHelper->FindClassData(CurrentClass);

			// find the compiler token for this property
			FTokenData* PropData = ClassData->FindTokenData(Prop);
			if ( PropData != NULL && PropData->Token.ExportInfo.Len() > 0 )
			{
				(FString&)PropertyText = PropertyText.Replace(TEXT("fixme"), *PropData->Token.ExportInfo);
			}
		}
	}
}

// Constructor.

FNativeClassHeaderGenerator::FNativeClassHeaderGenerator(const TCHAR* InClassHeaderFilename,UObject* InPackage,UBOOL InMasterHeader,FClassTree& inClassTree)
: ClassHeaderFilename(InClassHeaderFilename)
, Package(InPackage)
, API(FString(*InPackage->GetName()).ToUpper())
, MasterHeader(InMasterHeader)
, CurrentClass(NULL)
, ClassTree(inClassTree)
{
	// If the Inc directory is missing, then this is a mod/runtime build, and we don't want
	// to export the headers.
	TArray<FString> IncDir;
	GFileManager->FindFiles(IncDir, *(GEditor->EditPackagesInPath * *Package->GetName() * TEXT("Inc")), FALSE, TRUE);
	// if we couldn't find the root directory, then aobrt
	if (IncDir.Num() == 0)
	{
		// clear the misaligned flag on the classes because the exporting isn't called which would normally do it
		for( TObjectIterator<UClass> It; It; ++It )
		{
			UClass* Class = *It;
			if( Class->GetOuter()==Package)
			{
				Class->ClearFlags(RF_MisalignedObject);
			}
		}
		debugf(TEXT("Failed to find %s directory, skipping header generation."), *(GEditor->EditPackagesInPath * *Package->GetName() * TEXT("Inc")));
		
		return;
	}

	// Tag native classes in this package for export.

	// Clear the RF_TagExp flags for all names.  If this is the master header for the package,
	// also clear the RF_TagImp flag, which is used to track duplicate names across multiple
	// header files within the same package
	QWORD ClearMask = RF_TagExp;
	if ( MasterHeader )
	{
		ClearMask |= RF_TagImp;
	}
	for( INT i=0; i<FName::GetMaxNames(); i++ )
	{
		if( FName::GetEntry(i) )
		{
			FName::GetEntry(i)->ClearFlags( ClearMask );
		}
	}

	INT ClassCount = 0;
	for( TObjectIterator<UClass> It; It; ++It )
	{
		UClass* Class = *It;
		if( Class->GetOuter()==Package && Class->HasAnyClassFlags(CLASS_Native) && Class->ClassHeaderFilename == ClassHeaderFilename )
		{
			if ( !Class->HasAnyClassFlags(CLASS_Parsed|CLASS_Intrinsic) ) 
			{
				if (ParseParam(appCmdLine(),TEXT("auto")) ||
					GIsUnattended ||
					GWarn->YesNof(LocalizeSecure(LocalizeQuery(TEXT("RemoveNativeClass"),TEXT("Core")),*Class->GetName())) )
				{
					warnf(TEXT("Class '%s' has no script to compile and is not marked intrinsic - removing from static class registration..."), *Class->GetName());
					continue;
				}
				else
				{
					warnf(NAME_Warning,TEXT("Class '%s' has no script to compile.  It should be marked intrinsic unless it is being removed."), *Class->GetName());
				}
			}
			else if ( Class->HasAllClassFlags(CLASS_Parsed|CLASS_Intrinsic) )
			{
				warnf(NAME_Error, TEXT("Class '%s' contains script but is marked intrinsic."), *Class->GetName());
			}

			if ( Class->ScriptText && !(Class->ClassFlags&CLASS_NoExport) )
			{
				ClassCount++;
				Class->ClearFlags(RF_TagImp);
				Class->SetFlags(RF_TagExp);
				for(TFieldIterator<UFunction,CASTCLASS_UFunction,0> Function(Class); Function; ++Function)
				{
					if( (Function->FunctionFlags & (FUNC_Event|FUNC_Delegate)) && !Function->GetSuperFunction() )
					{
						// mark this name as needing to be exported
						Function->GetFName().SetFlags(RF_TagExp);
					}
				}
			}
		}
		else
		{
			It->ClearFlags( RF_TagImp | RF_TagExp );
		}
	
		Classes.AddItem(Class);
	}

	if( ClassCount == 0 )
		return;

	// Load the original header file into memory
	FString HeaderFileLocation = GEditor->EditPackagesInPath * Package->GetName() * TEXT("Inc");
	FString	HeaderPath = HeaderFileLocation * Package->GetName() + ClassHeaderFilename + TEXT("Classes.h");;
	appLoadFileToString(OriginalHeader,*HeaderPath);

	// Iterate over all classes and sort them by name.
	Sort<USE_COMPARE_POINTER(UClass,UMakeCommandlet)>( &Classes(0), Classes.Num() );

	debugf( TEXT("Autogenerating C++ header: %s"), *ClassHeaderFilename );

	PreHeaderText.Logf(
		TEXT("/*===========================================================================\r\n")
		TEXT("    C++ class definitions exported from UnrealScript.\r\n")
		TEXT("    This is automatically generated by the tools.\r\n")
		TEXT("    DO NOT modify this manually! Edit the corresponding .uc files instead!\r\n")
		TEXT("    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.\r\n")
		TEXT("===========================================================================*/\r\n")
		TEXT("#if SUPPORTS_PRAGMA_PACK\r\n")
		TEXT("#pragma pack (push,%i)\r\n")
		TEXT("#endif\r\n")
		TEXT("\r\n")
		TEXT("\r\n"),
		(BYTE)PROPERTY_ALIGNMENT
		);

	// if a global auto-include file exists, generate a line to have that file included
	FString GlobalAutoIncludeFilename = Package->GetName() + ClassHeaderFilename + TEXT("GlobalIncludes.h");
	if ( GFileManager->FileSize(*(HeaderFileLocation * GlobalAutoIncludeFilename)) > 0 )
	{
		PreHeaderText.Logf(TEXT("#include \"%s\"\r\n\r\n"), *GlobalAutoIncludeFilename);
	}

	HeaderText.Logf( TEXT("#if !ENUMS_ONLY\r\n") );
	HeaderText.Logf( TEXT("\r\n") );

	HeaderText.Logf(
		TEXT("#ifndef NAMES_ONLY\r\n")
		TEXT("#define AUTOGENERATE_NAME(name) extern FName %s_##name;\r\n")
		TEXT("#define AUTOGENERATE_FUNCTION(cls,idx,name)\r\n")
		TEXT("#endif\r\n")
		TEXT("\r\n"),
		*API
		);

	// Autogenerate names (alphabetically sorted).
	TArray<FString> Names;
	for( INT i=0; i<FName::GetMaxNames(); i++ )
	{
		FNameEntry* Entry = FName::GetEntry(i);
		if ( Entry && Entry->HasAnyFlags(RF_TagExp) && !Entry->HasAnyFlags(RF_TagImp) )
		{
			// the RF_TagImp flag is persistent across all header files which are generated
			// within a given package - it prevents duplicate names from being generated
			// in multiple headers.
			Entry->SetFlags( RF_TagImp );
			new(Names) FString(FName((EName)(i)).ToString());
		}
	}

	Sort<USE_COMPARE_CONSTREF(FString,UMakeCommandlet)>( &Names(0), Names.Num() );
	for( INT i=0; i<Names.Num(); i++ )
	{
		HeaderText.Logf( TEXT("AUTOGENERATE_NAME(%s)\r\n"), *Names(i) );
	}

	for( INT i=0; i<FName::GetMaxNames(); i++ )
	{
		if( FName::GetEntry(i) )
		{
			FName::GetEntry(i)->ClearFlags( RF_TagExp );
		}
	}

	FString HeaderAPI = API;
	if( ClassHeaderFilename != TEXT("") )
	{
		HeaderAPI = HeaderAPI + TEXT("_") + FString(ClassHeaderFilename).ToUpper();
	}

	HeaderText.Logf( TEXT("\r\n#ifndef NAMES_ONLY\r\n\r\n") );

	HeaderText.Logf( TEXT("#ifndef INCLUDED_%s_CLASSES\r\n"), *HeaderAPI);
	HeaderText.Logf( TEXT("#define INCLUDED_%s_CLASSES 1\r\n"), *HeaderAPI);
	HeaderText.Logf( TEXT("\r\n") );

	ExportClassHeader(UObject::StaticClass());

	HeaderText.Logf( TEXT("#endif // !INCLUDED_%s_CLASSES\r\n"), *HeaderAPI );
	HeaderText.Logf( TEXT("#endif // !NAMES_ONLY\r\n") );
	HeaderText.Logf( TEXT("\r\n") );

	for( INT i=0; i<Classes.Num(); i++ )
	{
		UClass* Class = Classes(i);
		if( !Class->HasAnyClassFlags(CLASS_Interface) && Class->HasAnyFlags(RF_TagExp) )
		{
			for( TFieldIterator<UFunction,CASTCLASS_UFunction,0> Function(Class); Function; ++Function )
			{
				if( (Function->FunctionFlags & FUNC_Native) != 0 /*&& ShouldExportFunction(*Function)*/ )
				{
					HeaderText.Logf( TEXT("AUTOGENERATE_FUNCTION(%s,%i,exec%s);\r\n"), NameLookupCPP->GetNameCPP( Class ), Function->iNative ? Function->iNative : -1, *Function->GetName() );
				}
			}
		}
	}

	HeaderText.Logf( TEXT("\r\n") );
	HeaderText.Logf( TEXT("#ifndef NAMES_ONLY\r\n") );
	HeaderText.Logf( TEXT("#undef AUTOGENERATE_NAME\r\n") );
	HeaderText.Logf( TEXT("#undef AUTOGENERATE_FUNCTION\r\n") );
	HeaderText.Logf( TEXT("#endif\r\n") );

	HeaderText.Logf( TEXT("\r\n") );

	HeaderText.Logf( TEXT("#ifdef STATIC_LINKING_MOJO\r\n"));

	HeaderText.Logf( TEXT("#ifndef %s_NATIVE_DEFS\r\n"), *HeaderAPI);
	HeaderText.Logf( TEXT("#define %s_NATIVE_DEFS\r\n"), *HeaderAPI);
	HeaderText.Logf( TEXT("\r\n") );

	for( INT i=0; i<Classes.Num(); i++ )
	{
		UClass* Class = Classes(i);
		if( Class->HasNativesToExport(Package) && (Class->ClassHeaderFilename == ClassHeaderFilename) )
		{
			HeaderText.Logf( TEXT("DECLARE_NATIVE_TYPE(%s,%s);\r\n"), *Package->GetName(), NameLookupCPP->GetNameCPP(Class) );
		}
	}
	HeaderText.Logf( TEXT("\r\n") );

	HeaderText.Logf( TEXT("#define AUTO_INITIALIZE_REGISTRANTS_%s \\\r\n"), *HeaderAPI );
	for( INT i=0; i<Classes.Num(); i++ )
	{
		UClass* Class = Classes(i);
		if( (Class->GetOuter() == Package) && Class->HasAnyClassFlags(CLASS_Native) && (Class->ClassHeaderFilename == ClassHeaderFilename) )
		{
			HeaderText.Logf( TEXT("\t%s::StaticClass(); \\\r\n"), NameLookupCPP->GetNameCPP( Class ) );
			if( !Class->HasAnyClassFlags(CLASS_Interface) && Class->HasNativesToExport( Package ) )
			{
				for( TFieldIterator<UFunction,CASTCLASS_UFunction,0> Function(Class); Function && Function.GetStruct()==Class; ++Function )
				{
					if( (Function->FunctionFlags&FUNC_Native) != 0 /*&& ShouldExportFunction(*Function)*/ )
					{
						HeaderText.Logf( TEXT("\tGNativeLookupFuncs[Lookup++] = &Find%s%sNative; \\\r\n"), *Package->GetName(), NameLookupCPP->GetNameCPP(Class) );
						break;
					}
				}
			}
		}
	}
	HeaderText.Logf( TEXT("\r\n") );

	HeaderText.Logf( TEXT("#endif // %s_NATIVE_DEFS\r\n"), *HeaderAPI ); // #endif // s_NATIVE_DEFS
	HeaderText.Logf( TEXT("\r\n") );

	HeaderText.Logf( TEXT("#ifdef NATIVES_ONLY\r\n") );
	for( INT i=0; i<Classes.Num(); i++ )
	{
		UClass* Class = Classes(i);
		if( !Class->HasAnyClassFlags(CLASS_Interface) && Class->HasNativesToExport( Package ) && (Class->ClassHeaderFilename == ClassHeaderFilename) )
		{
			TArray<UFunction*> Functions;

			for( TFieldIterator<UFunction,CASTCLASS_UFunction,0> Function(Class); Function; ++Function )
			{
				if( (Function->FunctionFlags&FUNC_Native) != 0 /*&& ShouldExportFunction(*Function)*/ )
				{
					Functions.AddUniqueItem(*Function);
				}
			}

			if( Functions.Num() )
			{
				HeaderText.Logf( TEXT("NATIVE_INFO(%s) G%s%sNatives[] = \r\n"), NameLookupCPP->GetNameCPP(Class), *Package->GetName(), NameLookupCPP->GetNameCPP(Class) );
				HeaderText.Logf( TEXT("{ \r\n"));
				for ( INT i = 0; i < Functions.Num(); i++ )
				{
					UFunction* Function = Functions(i);
					HeaderText.Logf( TEXT("\tMAP_NATIVE(%s,exec%s)\r\n"), NameLookupCPP->GetNameCPP(Class), *Function->GetName() );
				}
				HeaderText.Logf( TEXT("\t{NULL,NULL}\r\n") );
				HeaderText.Logf( TEXT("};\r\n") );
				HeaderText.Logf( TEXT("IMPLEMENT_NATIVE_HANDLER(%s,%s);\r\n"), *Package->GetName(), NameLookupCPP->GetNameCPP(Class) );
				HeaderText.Logf( TEXT("\r\n") );
			}
		}
	}
	HeaderText.Logf( TEXT("#endif // NATIVES_ONLY\r\n"), *HeaderAPI ); // #endif // NAMES_ONLY
	HeaderText.Logf( TEXT("#endif // STATIC_LINKING_MOJO\r\n"), *HeaderAPI ); // #endif // STATIC_LINKING_MOJO

	// Generate code to automatically verify class offsets and size.
	HeaderText.Logf( TEXT("\r\n#ifdef VERIFY_CLASS_SIZES\r\n") ); // #ifdef VERIFY_CLASS_SIZES
	for( INT i=0; i<Classes.Num(); i++ )
	{
		UClass* Class = Classes(i);
		if( Class->HasAnyClassFlags(CLASS_Native) && !Class->HasAnyClassFlags(CLASS_Intrinsic) && (Class->GetOuter() == Package) && (Class->ClassHeaderFilename ==  ClassHeaderFilename) )
		{
			// Only verify all property offsets for noexport classes to avoid running into compiler limitations.
			if( Class->ClassFlags & CLASS_NoExport )
			{
				// Iterate over all properties that are new in this class.
				for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(Class); It && It.GetStruct()==Class; ++It )
				{
					// We can't verify bools due to the packing and we skip noexport variables so you can e.g. declare a placeholder for the virtual function table in script.
					if( It->ElementSize && !It->IsA(UBoolProperty::StaticClass()) && !(It->PropertyFlags & CPF_NoExport) )
					{
						// Emit verification macro.
						HeaderText.Logf( TEXT("VERIFY_CLASS_OFFSET_NODIE(%s,%s,%s)\r\n"),Class->GetPrefixCPP(),*Class->GetName(),*It->GetName());
					}
				}
			}
			// Verify first and last property for regular classes.
			else
			{
				UProperty* First	= NULL;
				UProperty* Last		= NULL;

				// Iterate over all properties that are new in this class.
				for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(Class); It && It.GetStruct()==Class; ++It )
				{
					// We can't verify bools due to the packing and we skip noexport variables so you can e.g. declare a placeholder for the virtual function table in script.
					if( It->ElementSize && !It->IsA(UBoolProperty::StaticClass()) && !(It->PropertyFlags & CPF_NoExport) )
					{
						// Keep track of first and last usable property.
						if( !First )
						{
							First = *It;
						}

						Last = *It;
					}
				}

				// Emit verification macro.
				if( First )
				{
					HeaderText.Logf( TEXT("VERIFY_CLASS_OFFSET_NODIE(%s,%s,%s)\r\n"),Class->GetPrefixCPP(),*Class->GetName(),*First->GetName());
				}
				if( Last && Last != First )
				{
					HeaderText.Logf( TEXT("VERIFY_CLASS_OFFSET_NODIE(%s,%s,%s)\r\n"),Class->GetPrefixCPP(),*Class->GetName(),*Last->GetName());
				}
			}

			HeaderText.Logf( TEXT("VERIFY_CLASS_SIZE_NODIE(%s)\r\n"), NameLookupCPP->GetNameCPP(Class) );
		}
	}
	HeaderText.Logf( TEXT("#endif // VERIFY_CLASS_SIZES\r\n") ); // #endif // VERIFY_CLASS_SIZES

	HeaderText.Logf( TEXT("#endif // !ENUMS_ONLY\r\n") ); // #endif // !ENUMS_ONLY

	HeaderText.Logf( TEXT("\r\n") );
	HeaderText.Logf( TEXT("#if SUPPORTS_PRAGMA_PACK\r\n") );
	HeaderText.Logf( TEXT("#pragma pack (pop)\r\n") );
	HeaderText.Logf( TEXT("#endif\r\n") );

	// Save the header file.;

	// build the full header file out of its pieces
	FString FullHeader = FString::Printf(
		TEXT("%s")
		TEXT("// Split enums from the rest of the header so they can be included earlier\r\n")
		TEXT("// than the rest of the header file by including this file twice with different\r\n")
		TEXT("// #define wrappers. See Engine.h and look at EngineClasses.h for an example.\r\n")
		TEXT("#if !NO_ENUMS && !defined(NAMES_ONLY)\r\n\r\n")
		TEXT("#ifndef INCLUDED_%s_ENUMS\r\n")
		TEXT("#define INCLUDED_%s_ENUMS 1\r\n\r\n")
		TEXT("%s\r\n")
		TEXT("#endif // !INCLUDED_%s_ENUMS\r\n")
		TEXT("#endif // !NO_ENUMS\r\n\r\n")
		TEXT("%s"),
		*PreHeaderText, 
		*HeaderAPI,
		*HeaderAPI,
		*EnumHeaderText, 
		*HeaderAPI,
		*HeaderText);

	if(OriginalHeader.Len() == 0 || appStrcmp(*OriginalHeader, *FullHeader))
	{
		if( ( OriginalHeader.Len() )
			&& ( GIsSilent || GIsUnattended )
			&& !ParseParam(appCmdLine(),TEXT("auto"))
			)
		{
			warnf(NAME_Error,TEXT("Cannot export %s while in silent/unattended mode."),*HeaderPath);
		}
		else 
		{
			UBOOL bExportUpdatedHeader = FALSE;
			const UBOOL bInitialExport = OriginalHeader.Len() == 0;
			const UBOOL bAutoExport = ParseParam(appCmdLine(), TEXT("auto"));
			bExportUpdatedHeader = bInitialExport || bAutoExport;

			if ( bExportUpdatedHeader == FALSE )
			{
				// display a prompt to the user that this header needs to be updated.

				// save the updated version to a tmp file so that the user can see what will be changing
				FString TmpHeaderFilename = HeaderPath + TEXT(".tmp");

				// delete any existing temp file
				GFileManager->Delete( *TmpHeaderFilename, FALSE, TRUE );
				if ( !appSaveStringToFile(*FullHeader, *TmpHeaderFilename) )
				{
					warnf(NAME_Warning, TEXT("Failed to save header export preview: '%s'"), *TmpHeaderFilename);
				}

				bExportUpdatedHeader = GWarn->YesNof(LocalizeSecure(LocalizeQuery(TEXT("Overwrite"),TEXT("Core")),*HeaderPath));

				// delete the tmp file we created
				GFileManager->Delete( *TmpHeaderFilename, FALSE, TRUE );
			}

			if( bExportUpdatedHeader == TRUE )
			{
				if( GFileManager->IsReadOnly( *HeaderPath ) )
				{
#if HAVE_SCC
					if( ParseParam(appCmdLine(), TEXT("auto")) || GWarn->YesNof(LocalizeSecure(LocalizeQuery(TEXT("CheckoutPerforce"),TEXT("Core")), *HeaderPath)) )
					{
						// source control object
						FSourceControlIntegration* SCC = new FSourceControlIntegration;

						// make a fully qualified filename
						FString Dir		= GFileManager->GetCurrentDirectory();
						FString File	= HeaderPath;
					
						// Attempt to check out the header file
						SCC->CheckOut( *(Dir + File) );

						//destroy the source control object
						delete SCC;
					}
#endif
				}
				debugf(TEXT("Exported updated C++ header: %s"), *HeaderPath);

				if(!appSaveStringToFile(*FullHeader,*HeaderPath))
				{
					warnf( NAME_Error, LocalizeSecure(LocalizeError(TEXT("ExportWrite"),TEXT("Core")),*InPackage->GetPathName(),*HeaderPath));
				}
			}
		}
	}
}

/**
 * Returns the number of seconds since the specified directory has been modified,
 * including changes to any files contained within that directory.
 * 
 * @param	PackageScriptDirectoryPath	the path to the Classes subdirectory for the package being checked.
										(i.e. ../Development/Src/SomePackage/Classes/)
 *
 * @return	the number of seconds since the directory or any of its files has been modified.
 *			Values less than 0 indicate that the directory doesn't exist.
 */
static DOUBLE GetScriptDirectoryAgeInSeconds( const FString& PackageScriptDirectoryPath )
{
	DOUBLE PackageDirectoryAgeInSeconds = GFileManager->GetFileAgeSeconds(*PackageScriptDirectoryPath);

	// Modifying a file in a directory doesn't always seem to update the
	// directory's 'last modified' time, so now check each of the script files from
	// this directory to be sure we're not missing one
	const FString ClassesWildcard	= PackageScriptDirectoryPath * TEXT("*.uc");

	// grab the list of files matching the wildcard path
	TArray<FString> ClassesFiles;
	GFileManager->FindFiles( ClassesFiles, *ClassesWildcard, TRUE, FALSE );

	// check the timestamp for each .uc file
	for(INT ClassIndex = 0; ClassIndex < ClassesFiles.Num(); ClassIndex++ )
	{
		// expanded name of this .uc file
		const FString ClassName = PackageScriptDirectoryPath * ClassesFiles(ClassIndex);
		const DOUBLE ScriptFileAgeInSeconds = GFileManager->GetFileAgeSeconds(*ClassName);

		if( ScriptFileAgeInSeconds < PackageDirectoryAgeInSeconds )
		{
			// this script file's 'last modified' stamp is newer than the one for the
			// directory - some text editors save the file in such a way that the directory
			// modification timestamp isn't updated.
			PackageDirectoryAgeInSeconds = ScriptFileAgeInSeconds;
		}
	}

	return PackageDirectoryAgeInSeconds;
}

static UBOOL AreScriptPackagesOutOfDate_Worker(const TArray<FString>& ScriptPackageNames,
											   const FString& ScriptSourcePath,
											   const FString& ScriptPackagePath,
											   UBOOL bIsShippingPackage)
{
	UBOOL bFoundOutOfDatePackage = FALSE;

	// For each script package, see if any of its scripts are newer
	for( INT ScriptPackageIndex = 0 ; ScriptPackageIndex < ScriptPackageNames.Num() && !bFoundOutOfDatePackage ; ++ScriptPackageIndex )
	{
		// The compiled script package file.
		const FString PackageFile			= ScriptPackagePath * ScriptPackageNames(ScriptPackageIndex) + TEXT(".u");
		const DOUBLE PackageAgeInSeconds	= GFileManager->GetFileAgeSeconds(*PackageFile);

		// If the script package is read-only, we can't recompile it so just skip it.
		if ( PackageAgeInSeconds > 0 && GFileManager->IsReadOnly(*PackageFile) )
		{
			continue;
		}

		// The script source file.
		const FString SourceFile = ScriptSourcePath * ScriptPackageNames(ScriptPackageIndex) * TEXT("Classes");
		const DOUBLE SourceAgeInSeconds	= GetScriptDirectoryAgeInSeconds(SourceFile);

		// Does the script source file exist?
		if ( SourceAgeInSeconds > 0 )
		{
			if( PackageAgeInSeconds <= 0 )
			{
				// The compiled script package doesn't exist.
				bFoundOutOfDatePackage = TRUE;
			}
			else if ( SourceAgeInSeconds <= PackageAgeInSeconds )
			{
				// The source script file is newer than the compiled script package.
				bFoundOutOfDatePackage = TRUE;
			}
		}
	}

	return bFoundOutOfDatePackage;
}

/** @return		TRUE if if scripts need to be recompiled. */
UBOOL AreScriptPackagesOutOfDate()
{
	UBOOL bResult = FALSE;

#if !SHIPPING_PC_GAME
	// If running in 'user mode', don't look to compile shipping script.
	const UBOOL bRunningAsUser =
		ParseParam(appCmdLine(), TEXT("user"))
		|| ParseParam(appCmdLine(), TEXT("SEEKFREELOADING"))
		|| ParseParam(appCmdLine(), TEXT("installed"));
	if ( !bRunningAsUser )
	{
		// Look for uncompiled shipping script.
		FString ShippingScriptSourcePath;
		verify(GConfig->GetString( TEXT("Editor.EditorEngine"), TEXT("EditPackagesInPath"), ShippingScriptSourcePath, GEngineIni ));

		// Get the list of script filenames from .ini
		TArray<FString> ShippingScriptPackageNames;
		TMultiMap<FString,FString>* Sec = GConfig->GetSectionPrivate( TEXT("Editor.EditorEngine"), 0, 1, GEngineIni );
		Sec->MultiFind( FString(TEXT("EditPackages")), ShippingScriptPackageNames );

		// Don't need to check SeekFreePCPaths here because this code won't be run in user-mode.
		const FString ScriptPackagePath = appScriptOutputDir();
		bResult |= AreScriptPackagesOutOfDate_Worker( ShippingScriptPackageNames, ShippingScriptSourcePath, ScriptPackagePath, TRUE );
	}
#endif

	// Look for uncompiled mod script.
	// These are loaded from the editor ini because it's not signed in cooked pc builds.
	const TCHAR* ModIni = GEditorIni;

	// Get the list of script filenames from .ini
	TArray<FString> ModScriptPackageNames;
	TMultiMap<FString,FString>* ModSec = GConfig->GetSectionPrivate( TEXT("ModPackages"), 0, 1, ModIni );
	if( ModSec != NULL )
	{
		ModSec->MultiFind( FString(TEXT("ModPackages")), ModScriptPackageNames );

		FString ModScriptSourcePath;
		if( GConfig->GetString( TEXT("ModPackages"), TEXT("ModPackagesInPath"), ModScriptSourcePath, ModIni ) )
		{
			FString ModOutputDir;
			if( GConfig->GetString( TEXT("ModPackages"), TEXT("ModOutputDir"), ModOutputDir, GEditorIni ) )
			{
				bResult |= AreScriptPackagesOutOfDate_Worker( ModScriptPackageNames, ModScriptSourcePath, ModOutputDir, FALSE );
			}
		}
	}

	return bResult;
}

/*-----------------------------------------------------------------------------
	UMakeCommandlet.
-----------------------------------------------------------------------------*/

/**
 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. If
 * the commandlet implements this function, it should fully initialize the UEngine object as well.  Commandlets
 * should indicate that they have implemented this function by assigning the custom UEngine to GEngine.
 */
void UMakeCommandlet::CreateCustomEngine()
{
	UClass* EditorEngineClass	= UObject::StaticLoadClass( UEditorEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.EditorEngine"), NULL, LOAD_DisallowFiles, NULL );

	// must do this here so that the engine object that we create on the next line receives the correct property values
	UObject* DefaultEngine = EditorEngineClass->GetDefaultObject(TRUE);

	// ConditionalLink() won't call LoadConfig() if GIsUCCMake is true, so we must do it here so that the editor
	// engine has EditPackages
	EditorEngineClass->ConditionalLink();
	DefaultEngine->LoadConfig();

	GEngine = GEditor			= ConstructObject<UEditorEngine>( EditorEngineClass );
	GEditor->InitEditor();
}

/** Stores a script package name and a flag indicating whether the package is a shipping package. */
class FPackageShipInfo
{
public:
	FPackageShipInfo(const TCHAR* InPackage, UBOOL bInIsShippingPackage)
		:	Package( InPackage )
		,	bIsShippingPackage( bInIsShippingPackage )
	{}

	/** Script package name. */
	FString Package;
	/** TRUE if this is a shipped game package, FALSE if it's a mod package. */
	UBOOL	bIsShippingPackage;
};

INT UMakeCommandlet::Main( const FString& Params )
{
#if !SUPPORT_NAME_FLAGS
#error Please set SUPPORT_NAME_FLAGS to 1 in UnBuild.h if you want to support compiling script
#endif

	// TRUE if 'make' is being run in user mode (ie compile mod script, not shipping game script).
#if SHIPPING_PC_GAME
	const UBOOL bIsRunningAsUser = TRUE;
#else
	const UBOOL bIsRunningAsUser =
		ParseParam(appCmdLine(), TEXT("user"))
		|| ParseParam(appCmdLine(), TEXT("SEEKFREELOADING"))
		|| ParseParam(appCmdLine(), TEXT("installed"));
#endif

	// Can't rely on properties being serialized so we need to manually set them via the .ini config system.
	verify(GConfig->GetString( TEXT("Editor.EditorEngine"), TEXT("FRScriptOutputPath"), GEditor->FRScriptOutputPath, GEngineIni ));
	verify(GConfig->GetString( TEXT("Editor.EditorEngine"), TEXT("EditPackagesOutPath"), GEditor->EditPackagesOutPath, GEngineIni ));
	verify(GConfig->GetString( TEXT("Editor.EditorEngine"), TEXT("EditPackagesInPath"), GEditor->EditPackagesInPath, GEngineIni ));
	verify(GConfig->GetArray( TEXT("Editor.EditorEngine"), TEXT("EditPackages"), GEditor->EditPackages, GEngineIni ));

	// get a list of packages to skip warning about it the directory doesn't exist
	TArray<FString> OptionalPackages;
	GConfig->GetArray( TEXT("Editor.EditorEngine"), TEXT("OptionalScriptPackages"), OptionalPackages, GEngineIni );

	FString ShippingOutputDir;
	if ( bIsRunningAsUser )
	{
		USystem* DefaultSystemObject = USystem::StaticClass()->GetDefaultObject<USystem>();
		ShippingOutputDir = DefaultSystemObject->SeekFreePCPaths(0);
	}
	else
	{
		ShippingOutputDir = appScriptOutputDir();
	}

	// Look to the editor ini for mod script because the editor ini is not signed in cooked pc builds.
	const TCHAR* ModIni = GEditorIni;
	FString ModPackagesInPath;
	FString ModOutputDir;
	TArray<FString> ModPackages;
	verify(GConfig->GetString( TEXT("ModPackages"), TEXT("ModPackagesInPath"), ModPackagesInPath, ModIni ));
	verify(GConfig->GetString( TEXT("ModPackages"), TEXT("ModOutputDir"), ModOutputDir, GEditorIni ));
	GConfig->GetArray( TEXT("ModPackages"), TEXT("ModPackages"), ModPackages, ModIni );

	// PackagesAndPaths stores a list of all shipping packages, followed by a list of all mod packages.
	TArray<FPackageShipInfo> PackagesAndPaths;
	for ( INT PackageIndex = 0 ; PackageIndex < GEditor->EditPackages.Num() ; ++PackageIndex )
	{
		PackagesAndPaths.AddItem(FPackageShipInfo(*GEditor->EditPackages(PackageIndex),TRUE));
	}

	// Stores the index into PackagesAndPaths of the first mod package.
	const INT FirstModPacakgeIndex = PackagesAndPaths.Num();

	for ( INT PackageIndex = 0 ; PackageIndex < ModPackages.Num() ; ++PackageIndex )
	{
		PackagesAndPaths.AddItem(FPackageShipInfo(*ModPackages(PackageIndex),FALSE));
	}

	// Store the script package directory and package list that are deletable.
	const FString DeletePackagesDir = bIsRunningAsUser ? ModOutputDir : ShippingOutputDir;

	// When compiling in 'user' mode, only mod packages can be deleted.
	const TArray<FString>& DeletePackages = bIsRunningAsUser ? ModPackages : GEditor->EditPackages;

	// indicates that we should delete the current package -
	// used to ensure that dependent packages are always recompiled
	UBOOL bDeletePackage = FALSE;

	// If -full was specified on the command-line, we want to wipe all .u files. Listing unreferenced functions also
	// relies on a full recompile.
	if( ParseParam(appCmdLine(), TEXT("FULL")) || ParseParam(appCmdLine(),TEXT("LISTUNREFERENCED")) )
	{
		bDeletePackage = TRUE;
	}

	if(!bDeletePackage && !AreScriptPackagesOutOfDate())
	{
		warnf(TEXT("No scripts need recompiling."));
		GIsRequestingExit = TRUE;
		return 0;
	}

	NameLookupCPP				= new FNameLookupCPP();

	FCompilerMetadataManager*	ScriptHelper = GScriptHelper = new FCompilerMetadataManager();

	// indicates that we should only never delete successive .u files in the dependency chain
	const UBOOL bNoDelete = ParseParam(appCmdLine(), TEXT("nodelete"));

	if(bDeletePackage)
	{
		DeleteEditPackages( DeletePackagesDir, DeletePackages, 0 );
	}

	// Load classes for editing.
	UClassFactoryUC* ClassFactory = new UClassFactoryUC;
	UBOOL Success = TRUE;

	for( INT PackageIndex=0; PackageIndex<PackagesAndPaths.Num(); PackageIndex++ )
	{
		// Try to load class.
		const UBOOL bIsShippingPackage = PackagesAndPaths(PackageIndex).bIsShippingPackage;
		const TCHAR* Pkg = *PackagesAndPaths( PackageIndex ).Package;
		const FString& PackageInPath = bIsShippingPackage ? GEditor->EditPackagesInPath : ModPackagesInPath;
		const FString Filename = FString::Printf(TEXT("%s") PATH_SEPARATOR TEXT("%s.u"), bIsShippingPackage ? *ShippingOutputDir : *ModOutputDir, Pkg );
		GWarn->Log( NAME_Heading, FString::Printf(TEXT("%s - %s"),Pkg,ParseParam(appCmdLine(), TEXT("DEBUG"))? TEXT("Debug") : TEXT("Release"))); //DEBUGGER

		// Check whether this package needs to be recompiled because a class is newer than the package.
		//@warning: This won't detect changes to resources being compiled into packages.
		UBOOL bFileExists = GFileManager->FileSize(*Filename) >= 0;
		if( bFileExists )
		{
			// Can't recompile shipping packages when running as user.
			const UBOOL bCanRecompilePackage = !bIsShippingPackage || !bIsRunningAsUser;
			if ( bCanRecompilePackage )
			{
				const DOUBLE		PackageBinaryAgeInSeconds	= GFileManager->GetFileAgeSeconds(*Filename);
				const FString		PackageDirectory = PackageInPath * Pkg * TEXT("Classes");
				const DOUBLE		PackageDirectoryAgeInSeconds = GetScriptDirectoryAgeInSeconds(PackageDirectory);

				if ( PackageDirectoryAgeInSeconds > 0 && PackageDirectoryAgeInSeconds <= PackageBinaryAgeInSeconds )
				{
					warnf(TEXT("Package %s changed, recompiling"), Pkg);

					bFileExists = FALSE;
					INT DeleteCount = INDEX_NONE;
					if ( bNoDelete )
					{
						// in this case, we only want to delete one package
						DeleteCount = 1;
					}
					else
					{
						bDeletePackage = TRUE;
					}

					// Delete package and all the ones following in EditPackages. This is required in certain cases so we rather play safe.
					INT PackageIndexToDelete = PackageIndex;
					if ( bIsRunningAsUser )
					{
						PackageIndexToDelete -= FirstModPacakgeIndex;
					}
					DeleteEditPackages(DeletePackagesDir,DeletePackages,PackageIndexToDelete,DeleteCount);
				}
			}
		}
		else
		{
			// Can't find a shipping package!
			if ( bIsShippingPackage && bIsRunningAsUser )
			{
				appErrorf( TEXT("Missing shipping script package %s"), Pkg );
			}

			if ( !bNoDelete )
			{
				// if we've encountered a missing package, and we haven't already deleted all
				// dependent packages, do that now
				if ( !bDeletePackage )
				{
					INT PackageIndexToDelete = PackageIndex;
					if ( bIsRunningAsUser )
					{
						PackageIndexToDelete -= FirstModPacakgeIndex;
					}
					DeleteEditPackages(DeletePackagesDir,DeletePackages,PackageIndexToDelete);
				}

				bDeletePackage = TRUE;
			}
		}

		if( GWarn->Errors.Num() > 0 )
		{
			break;
		}

		// disable loading of objects outside of this package (or more exactly, objects which aren't UFields, CDO, or templates)
		GUglyHackFlags |= HACK_VerifyObjectReferencesOnly;
		UPackage* Package = (bDeletePackage && (!bIsRunningAsUser || !bIsShippingPackage))
			? NULL 
			: Cast<UPackage>(LoadPackage( NULL, *Filename, LOAD_NoWarn|LOAD_NoVerify ));
		GUglyHackFlags &= ~HACK_VerifyObjectReferencesOnly;

		const UBOOL bNeedsCompile = Package == NULL;

		// if we couldn't load the package, but the .u file exists, then we have some other problem that is preventing
		// the .u file from being loaded - in this case we don't want to attempt to recompile the package
		if ( bNeedsCompile && bFileExists )
		{
			warnf(NAME_Error, TEXT("Could not load existing package file '%s'.  Check the log file for more information."), *Filename);
			Success = FALSE;
		}

		if ( Success )
		{
			// Create package.
			const FString IniName = PackageInPath * Pkg * TEXT("Classes") * Pkg + TEXT(".upkg");

			if ( bNeedsCompile )
			{
				// Rebuild the class from its directory.
				const FString Spec = PackageInPath * Pkg * TEXT("Classes") * TEXT("*.uc");

				TArray<FString> Files;
				GFileManager->FindFiles( Files, *Spec, 1, 0 );
				if ( Files.Num() > 0 )
				{
					GWarn->Log( TEXT("Analyzing...") );
					Package = CreatePackage( NULL, Pkg );

					// set some package flags for indicating that this package contains script
					Package->PackageFlags |= PKG_ContainsScript;

					if ( ParseParam(*Params, TEXT("DEBUG")) )
					{
						Package->PackageFlags |= PKG_ContainsDebugInfo;
					}

					// Try reading from package's .ini file.
					Package->PackageFlags &= ~(PKG_AllowDownload|PKG_ClientOptional|PKG_ServerSideOnly);

					UBOOL B=0;
					// the default for AllowDownload is TRUE
					if (!GConfig->GetBool(TEXT("Flags"), TEXT("AllowDownload"), B, *IniName) || B)
					{
						Package->PackageFlags |= PKG_AllowDownload;
					}
					// the default for ClientOptional is FALSE
					if (GConfig->GetBool(TEXT("Flags"), TEXT("ClientOptional"), B, *IniName) && B)
					{
						Package->PackageFlags |= PKG_ClientOptional;
					}
					// the default for ServerSideOnly is FALSE
					if (GConfig->GetBool(TEXT("Flags"), TEXT("ServerSideOnly"), B, *IniName) && B)
					{
						Package->PackageFlags |= PKG_ServerSideOnly;
					}

					Package->PackageFlags |= PKG_Compiling;

					// Make script compilation deterministic by sorting .uc files by name.
					Sort<USE_COMPARE_CONSTREF(FString,UMakeCommandlet)>( &Files(0), Files.Num() );

					for( INT i=0; i<Files.Num() && Success; i++ )
					{
						// Import class.
						const FString Filename = PackageInPath * Pkg * TEXT("Classes") * Files(i);
						const FString ClassName = Files(i).LeftChop(3);
						Success = ImportObject<UClass>( Package, *ClassName, RF_Public|RF_Standalone, *Filename, NULL, ClassFactory ) != NULL;
					}
				}
				else
				{
					// look to see if this package is optional; if so, skip the warning
					INT Index;
					if (!OptionalPackages.FindItem(Pkg, Index))
					{
						FString DisplayFilename( GFileManager->ConvertToAbsolutePath(*Spec) );
						if ( !bIsShippingPackage )
						{
							DisplayFilename = GFileManager->ConvertAbsolutePathToUserPath(*DisplayFilename);
						}
						warnf(NAME_Warning,TEXT("Can't find files matching %s"), *DisplayFilename );
					}
					continue;
				}
			}
			else
			{
				// if the package isn't recompiled we still need to load the macros
				FMacroProcessingFilter DummyMacroFilter(Pkg,TEXT("NULL"));
			}

			if ( Success )
			{
				// Verify that all script declared superclasses exist.
				for( TObjectIterator<UClass> ItC; ItC; ++ItC )
				{
					const UClass* ScriptClass = *ItC;
					if( ScriptClass->ScriptText && ScriptClass->GetSuperClass() )
					{
						if( !ScriptClass->GetSuperClass()->ScriptText )
						{
							warnf(NAME_Error, TEXT("Superclass %s of class %s not found"), *ScriptClass->GetSuperClass()->GetName(), *ScriptClass->GetName());
							Success = FALSE;
						}
					}
				}

				if (Success)
				{
					// Bootstrap-recompile changed scripts.
					GEditor->Bootstrapping = 1;
					GEditor->ParentContext = Package;

					GUglyHackFlags |= HACK_VerifyObjectReferencesOnly;
					Success = GEditor->MakeScripts( NULL, GWarn, 0, TRUE, TRUE, Package, !bNeedsCompile );
					GUglyHackFlags &= ~HACK_VerifyObjectReferencesOnly;
					GEditor->ParentContext = NULL;
					GEditor->Bootstrapping = 0;
				}
			}
		}

        if( !Success )
        {
            warnf ( TEXT("Compile aborted due to errors.") );
            break;
        }

		if ( bNeedsCompile )
		{
			// Save package.
			ULinkerLoad* Conform = NULL;
#if !SHIPPING_PC_GAME
			if( !ParseParam(appCmdLine(),TEXT("NOCONFORM")) )
			{
				// check the default location for script packages to conform against, if a like-named package exists in the
				// auto-conform directory, use that as the conform package
				BeginLoad();
				const FFilename OldFilename = appGameDir() * TEXT("ScriptOriginal") * Pkg + TEXT(".u");
				UPackage* OldPackage = CreatePackage( NULL, *(OldFilename.GetBaseFilename() + TEXT("_OLD")) );
				Conform = UObject::GetPackageLinker( OldPackage, *OldFilename, LOAD_Verify|LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
				EndLoad();
				if( Conform )
				{
					debugf( TEXT("Conforming: %s"), Pkg );
				}

				// Calling GetPackageLinker with an alternate location for a known package will cause that package's entry in the file cache
				// to point to the alternate location from here on out.  We don't want this (otherwise subsequence attempts to load that package
				// will result in loading the alternate version of the package) so we restore it now
				GPackageFileCache->CachePackage(*Filename, TRUE, FALSE);
			}
#endif

			Package->PackageFlags &= ~PKG_Compiling;

			// write a message indicating that we have finished the "compilation" phase and are beginning the "saving" part, so that crashes in SavePackage aren't
			// mistakenly assumed to be related to script compilation.

			FString DisplayFilename( GFileManager->ConvertToAbsolutePath(*Filename) );
			if ( !bIsShippingPackage )
			{
				// For mod packages, make sure to display the true location of the saved script package.
				DisplayFilename = GFileManager->ConvertAbsolutePathToUserPath(*DisplayFilename);
			}

			warnf(TEXT("Scripts successfully compiled - saving package '%s'"), *DisplayFilename);
			SavePackage( Package, NULL, RF_Standalone, *Filename, GError, Conform );
		}

		// Avoid TArray slack for meta data.
		GScriptHelper->Shrink();
	}

// DB: No more 'publish by copy'!
#if 0
	if ( bIsRunningAsUser )
	{
		if( !ParseParam(appCmdLine(),TEXT("noautopublish")) )
		{
			// Shell out a copy commandline.
			const FString UserGameDir = GFileManager->ConvertAbsolutePathToUserPath(*GFileManager->ConvertToAbsolutePath(*appGameDir()));
			// The /d switch means 'only copy those files whose source time is newer than the destiation time'.
			// So, we'll only be copying newly-created script packages.
			const FString Command = FString::Printf(TEXT("xcopy /d /s /i \"%sUnpublished\" \"%sPublished\""), *UserGameDir, *UserGameDir);
			system(TCHAR_TO_ANSI(*Command));
		}
	}
#endif

	// if we successfully compiled scripts, perform any post-compilation steps
	if (Success)
	{
		GEditor->PostScriptCompile();
	}

	if (ParseParam(appCmdLine(),TEXT("LISTUNREFERENCED")))
	{
		warnf( TEXT("Checking for unreferenced functions...") );
		INT UnrefCount = 0;
		for (TObjectIterator<UFunction> It; It; ++It)
		{
			UFunction *Func = *It;
			// ignore natives/events
			if ((Func->FunctionFlags & FUNC_Native) != 0 ||
				(Func->FunctionFlags & FUNC_Event) != 0 ||
				(Func->FunctionFlags & FUNC_Exec) != 0)
			{
				continue;
			}
			INT *EmitCnt = FuncEmitCountMap.Find(Func);
			if (EmitCnt == NULL)
			{
				//debugf(TEXT("- function %s not directly referenced, checking parents"),*Func->GetPathName());
				// check to see if this function's parents are referenced
				UBOOL bFoundRef = FALSE;
				UFunction *SearchFunc = Func;
				UFunction *SuperFunc = NULL;
				do
				{
					// try the direct parent
					SuperFunc = SearchFunc->GetSuperFunction();
					if (SuperFunc == NULL)
					{
						// otherwise look up the state/class tree
						SuperFunc = UAnalyzeScriptCommandlet::FindSuperFunction(SearchFunc);
					}
					if (SuperFunc != NULL)
					{
						if ((SuperFunc->FunctionFlags & FUNC_Native) != 0 ||
							(SuperFunc->FunctionFlags & FUNC_Event) != 0 ||
							(SuperFunc->FunctionFlags & FUNC_Exec) != 0)
						{
						}
						//debugf(TEXT("-+ checking parent %s of %s"),*SuperFunc->GetPathName(),*SearchFunc->GetPathName());
						EmitCnt = FuncEmitCountMap.Find(SuperFunc);
						if (EmitCnt != NULL)
						{
							bFoundRef = TRUE;
							//debugf(TEXT("-+ parent is ref'd!!!"));
							break;
						}
					}
					SearchFunc = SuperFunc;
				} while (SuperFunc != NULL);
				if (!bFoundRef)
				{
					UnrefCount++;
					warnf( TEXT("- function %s was never referenced"), *Func->GetPathName());
				}
			}
		}
		warnf( TEXT("%d unreferenced functions found"), UnrefCount);
	}

	delete NameLookupCPP;
	NameLookupCPP = NULL;

	delete ScriptHelper;
	ScriptHelper = NULL;

	delete FMacroProcessingFilter::GlobalSymbols;
	FMacroProcessingFilter::GlobalSymbols = NULL;

	GEditor = NULL;
	GIsRequestingExit = TRUE;

	return 0;
}

/**
 * Deletes all dependent .u files.  Given an index into the EditPackages array, deletes the .u files corresponding to
 * that index, as well as the .u files corresponding to subsequent members of the EditPackages array.
 *
 * @param	ScriptOutputDir		output directory for script packages
 * @param	PackageList			list of package names to delete
 * @param	StartIndex			index to start deleting packages
 * @param	Count				number of packages to delete - defaults to all
 */
void UMakeCommandlet::DeleteEditPackages(const FString& ScriptOutputDir, const TArray<FString>& PackageList, INT StartIndex, INT Count /* = INDEX_NONE */ ) const
{
	check(StartIndex>=0);

	if ( Count == INDEX_NONE )
	{
		Count = PackageList.Num() - StartIndex;
	}

	// Delete package and all the ones following in EditPackages. This is required in certain cases so we rather play safe.
	for( INT DeleteIndex = StartIndex; DeleteIndex < PackageList.Num() && Count > 0; DeleteIndex++, Count-- )
	{
		// create the output directory, if it doesn't exist
		GFileManager->MakeDirectory( *ScriptOutputDir, TRUE );

		const FString Filename = FString::Printf(TEXT("%s") PATH_SEPARATOR TEXT("%s.u"), *ScriptOutputDir, *PackageList(DeleteIndex) );
		if( !GFileManager->Delete(*Filename, 0, 0) )
		{
#if SHIPPING_PC_GAME
			warnf(NAME_Warning, TEXT("Failed to delete %s"), *Filename);
#else
			warnf( NAME_Error,TEXT("Failed to delete %s"), *Filename );
#endif
		}
#if SHIPPING_PC_GAME
		// also attempt to delete Published dir's version so the engine can't possibly load it and break things
		GFileManager->Delete(*GFileManager->ConvertAbsolutePathToUserPath(*GFileManager->ConvertToAbsolutePath(*Filename)).Replace(TEXT("Unpublished"), TEXT("Published"), TRUE));
#endif
	}
}

IMPLEMENT_CLASS(UMakeCommandlet)
