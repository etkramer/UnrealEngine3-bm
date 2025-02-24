/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Core includes.
#include "CorePrivate.h"

/*----------------------------------------------------------------------------
	UFactory.
----------------------------------------------------------------------------*/
FString UFactory::CurrentFilename(TEXT(""));

/** For interactive object imports, this value indicates whether the user wants objects to be automatically
    overwritten (See EAppReturnType), or -1 if the user should be prompted. */
int UFactory::OverwriteYesOrNoToAllState = -1;


void UFactory::StaticConstructor()
{
	new(GetClass(),TEXT("Description"),RF_Public)UStrProperty(CPP_PROPERTY(Description),TEXT(""),0);
	UArrayProperty* A = new(GetClass(),TEXT("Formats"),RF_Public)UArrayProperty(CPP_PROPERTY(Formats),TEXT(""),0);
	A->Inner = new(A,TEXT("StrProperty0"),RF_Public)UStrProperty;

	UClass* TheClass = GetClass();
	TheClass->EmitObjectReference( STRUCT_OFFSET( UFactory, SupportedClass ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UFactory, ContextClass ) );
}
UFactory::UFactory()
: Formats( E_NoInit )
{
	bEditorImport = FALSE;
}
void UFactory::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	if( !Ar.IsLoading() && !Ar.IsSaving() )
		Ar << SupportedClass << ContextClass;
}

/**
 * @return	The object class supported by this factory.
 */
UClass* UFactory::GetSupportedClass()
{
	return SupportedClass;
}

/**
 * Resolves SupportedClass for factories which support multiple classes.
 * Such factories will have a NULL SupportedClass member.
 */
UClass* UFactory::ResolveSupportedClass()
{
	// This check forces factories which support multiple classes to overload this method.
	// In other words, you can't have a SupportedClass of NULL and not overload this method.
	check( SupportedClass );
	return SupportedClass;
}

IMPLEMENT_COMPARE_POINTER( UFactory, UFactory, { return A->AutoPriority - B->AutoPriority; } )

UObject* UFactory::StaticImportObject
(
	UClass*				Class,
	UObject*			InOuter,
	FName				Name,
	EObjectFlags		Flags,
	const TCHAR*		Filename,
	UObject*			Context,
	UFactory*			InFactory,
	const TCHAR*		Parms,
	FFeedbackContext*	Warn
)
{
	check(Class);

	// Disallow importing into cooked packages.
	if ( InOuter && (InOuter->GetOutermost()->PackageFlags & PKG_Cooked) )
	{
		Warn->Logf( *LocalizeError(TEXT("OperationDisallowedOnCookedContent"),TEXT("Core")) );
		return NULL;
	}

	CurrentFilename = Filename;

	// Make list of all applicable factories.
	TArray<UFactory*> Factories;
	if( InFactory )
	{
		// Use just the specified factory.
		check( !InFactory->SupportedClass || InFactory->SupportedClass->IsChildOf(Class));
		Factories.AddItem( InFactory );
	}
	else
	{
		// Try all automatic factories, sorted by priority.
		for( TObjectIterator<UClass> It; It; ++It )
		{
			if( It->IsChildOf(UFactory::StaticClass()) )
			{
				UFactory* Default = (UFactory*)It->GetDefaultObject();
				if( Class->IsChildOf(Default->SupportedClass) && Default->AutoPriority>=0 )
				{
					Factories.AddItem( ConstructObject<UFactory>(*It) );
				}
			}
		}
		Sort<USE_COMPARE_POINTER(UFactory,UFactory)>( &Factories(0), Factories.Num() );
	}

	UBOOL bLoadedFile=0;

	// Try each factory in turn.
	for( INT i=0; i<Factories.Num(); i++ )
	{
		UFactory* Factory = Factories(i);
		UObject* Result = NULL;
		if( Factory->bCreateNew )
		{
			if( appStricmp(Filename,TEXT(""))==0 )
			{
				debugf( NAME_Log, TEXT("FactoryCreateNew: %s with %s (%i %i %s)"), *Class->GetName(), *Factories(i)->GetClass()->GetName(), Factory->bCreateNew, Factory->bText, Filename );
				Factory->ParseParms( Parms );
				Result = Factory->FactoryCreateNew( Class, InOuter, Name, Flags, NULL, Warn );
			}
		}
		else if( appStricmp(Filename,TEXT(""))!=0 )
		{
			if( Factory->bText )
			{
#if BATMAN
				// Disable non-useful debug message.
#else
				debugf( NAME_Log, TEXT("FactoryCreateText: %s with %s (%i %i %s)"), *Class->GetName(), *Factories(i)->GetClass()->GetName(), Factory->bCreateNew, Factory->bText, Filename );
#endif
				FString Data;
				if( appLoadFileToString( Data, Filename ) )
				{
					bLoadedFile = true;
					const TCHAR* Ptr = *Data;
					Factory->ParseParms( Parms );
					Result = Factory->FactoryCreateText( Class, InOuter, Name, Flags, NULL, *FFilename(Filename).GetExtension(), Ptr, Ptr+Data.Len(), Warn );
				}
			}
			else
			{
				debugf( NAME_Log, TEXT("FactoryCreateBinary: %s with %s (%i %i %s)"), *Class->GetName(), *Factories(i)->GetClass()->GetName(), Factory->bCreateNew, Factory->bText, Filename );
				TArray<BYTE> Data;
				if( appLoadFileToArray( Data, Filename ) )
				{
					bLoadedFile = true;
					Data.AddItem( 0 );
					const BYTE* Ptr = &Data( 0 );
					Factory->ParseParms( Parms );
					Result = Factory->FactoryCreateBinary( Class, InOuter, Name, Flags, NULL, *FFilename(Filename).GetExtension(), Ptr, Ptr+Data.Num()-1, Warn );
				}
			}
		}
		if( Result )
		{
			check(Result->IsA(Class));
			Result->MarkPackageDirty();
			GCallbackEvent->Send( CALLBACK_LevelDirtied );
			GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );
			GCallbackEvent->Send( CALLBACK_ObjectPropertyChanged, Result );

			CurrentFilename = TEXT("");
			return Result;
		}
	}

	if ( !bLoadedFile )
	{
		Warn->Logf( LocalizeSecure(LocalizeError(TEXT("NoFindImport"),TEXT("Core")), Filename) );
	}

	CurrentFilename = TEXT("");

	return NULL;
}

/**
 * UFactory::ValidForCurrentGame
 *
 * Search through list of valid game names
 * If list is empty or current game name is in the list - return TRUE
 * Otherwise, name wasn't found in the list - return FALSE
 *
 * author: superville
 */
UBOOL UFactory::ValidForCurrentGame()
{
	if( ValidGameNames.Num() > 0 )
	{
		for( INT Idx = 0; Idx < ValidGameNames.Num(); Idx++ )
		{
			if( appStricmp( appGetGameName(), *ValidGameNames(Idx) ) == 0 )
			{
				return 1;
			}
		}

		return 0;
	}

	return 1;
}

UBOOL UFactory::ImportUntypedBulkDataFromText(const TCHAR*& Buffer, FUntypedBulkData& BulkData)
{
	FString StrLine;
	INT ElementCount = 0;
	INT ElementSize = 0;
	UBOOL bBulkDataIsLocked = FALSE;

	while(ParseLine(&Buffer,StrLine))
	{
		FString ParsedText;
		const TCHAR* Str = *StrLine;

		if (Parse(Str, TEXT("ELEMENTCOUNT="), ParsedText))
		{
			/** Number of elements in bulk data array */
			ElementCount = appAtoi(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("ELEMENTSIZE="), ParsedText))
		{
			/** Serialized flags for bulk data */
			ElementSize = appAtoi(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("BEGIN "), ParsedText) && (ParsedText.ToUpper() == TEXT("BINARYBLOB")))
		{
			BYTE* RawData = NULL;
			/** The bulk data... */
			while(ParseLine(&Buffer,StrLine))
			{
				Str = *StrLine;

				if (Parse(Str, TEXT("SIZE="), ParsedText))
				{
					INT Size = appAtoi(*ParsedText);

					check(Size == (ElementSize *ElementCount));

					BulkData.Lock(LOCKSTATUS_ReadWriteLock);
					void* RawBulkData = BulkData.Realloc(ElementCount);
					RawData = (BYTE*)RawBulkData;
					bBulkDataIsLocked = TRUE;
				}
				else
				if (Parse(Str, TEXT("BEGIN "), ParsedText) && (ParsedText.ToUpper() == TEXT("BINARY")))
				{
					BYTE* BulkDataPointer = RawData;
					while(ParseLine(&Buffer,StrLine))
					{
						Str = *StrLine;
						TCHAR* ParseStr = (TCHAR*)(Str);

						if (Parse(Str, TEXT("END "), ParsedText) && (ParsedText.ToUpper() == TEXT("BINARY")))
						{
							break;
						}

						// Clear whitespace
						while ((*ParseStr == L' ') || (*ParseStr == L'\t'))
						{
							ParseStr++;
						}

						// Parse the values into the bulk data...
						while ((*ParseStr != L'\n') && (*ParseStr != L'\r') && (*ParseStr != 0))
						{
							INT Value;
							if (!appStrnicmp(ParseStr, TEXT("0x"), 2))
							{
								ParseStr +=2;
							}
							Value = ParseHexDigit(ParseStr[0]) * 16 + ParseHexDigit(ParseStr[1]);
							*BulkDataPointer = (BYTE)Value;
							BulkDataPointer++;
							ParseStr += 2;
							ParseStr++;
						}
					}
				}
				else
				if (Parse(Str, TEXT("END "), ParsedText) && (ParsedText.ToUpper() == TEXT("BINARYBLOB")))
				{
					BulkData.Unlock();
					bBulkDataIsLocked = FALSE;
					break;
				}
			}
		}
		else
		if (Parse(Str, TEXT("END "), ParsedText) && (ParsedText.ToUpper() == TEXT("UNTYPEDBULKDATA")))
		{
			break;
		}
	}

	if (bBulkDataIsLocked == TRUE)
	{
		BulkData.Unlock();
	}

	return TRUE;
}

UBOOL UFactory::ParseObjectPropertyName(const FString& PropertyText, FString& OutClass, FString& OutName)
{
	// Will have the following format:
	//		CLASS'NAME'
	// where name will be either fully qualified (and external object) or just the name
	INT CheckIndex = PropertyText.InStr(TEXT("'"));
	if (CheckIndex != -1)
	{
		OutClass = PropertyText.Left(CheckIndex);
		OutName = PropertyText.Right(PropertyText.Len() - CheckIndex - 1);
		// Remove the last '''
		OutName = OutName.Left(OutName.Len() - 1);
	}
	else
	{
		OutClass = TEXT("");
		OutName = PropertyText;
	}
	return TRUE;
}

IMPLEMENT_CLASS(UFactory);
