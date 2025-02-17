/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"
#include "DownloadableContent.h"


IMPLEMENT_CLASS(UOnlineSubsystem);
IMPLEMENT_CLASS(UOnlineGameSettings);
IMPLEMENT_CLASS(UOnlineGameSearch);
IMPLEMENT_CLASS(UOnlineGameplayEvents);

/**
 * Loads and creates any registered named interfaces
 */
UBOOL UOnlineSubsystem::Init(void)
{
	// Iterate through each configured named interface load it and create an instance
	for (INT InterfaceIndex = 0; InterfaceIndex < NamedInterfaceDefs.Num(); InterfaceIndex++)
	{
		const FNamedInterfaceDef& Def = NamedInterfaceDefs(InterfaceIndex);
		// Load the specified interface class name
		UClass* Class = LoadClass<UObject>(NULL,*Def.InterfaceClassName,NULL,LOAD_None,NULL);
		if (Class)
		{
			INT AddIndex = NamedInterfaces.AddZeroed();
			FNamedInterface& Interface = NamedInterfaces(AddIndex);
			// Set the object and interface names
			Interface.InterfaceName = Def.InterfaceName;
			Interface.InterfaceObject = ConstructObject<UObject>(Class);
			debugf(NAME_DevOnline,
				TEXT("Created named interface (%s) of type (%s)"),
				*Def.InterfaceName.ToString(),
				*Def.InterfaceClassName);
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Failed to load class (%s) for named interface (%s)"),
				*Def.InterfaceClassName,
				*Def.InterfaceName.ToString());
		}
	}
	return TRUE;
}

/**
 * Generates a string representation of a UniqueNetId struct.
 *
 * @param	IdToConvert		the unique net id that should be converted to a string.
 *
 * @return	the specified UniqueNetId represented as a string.
 */
FString UOnlineSubsystem::UniqueNetIdToString( const FUniqueNetId& IdToConvert )
{
	FString Result = FString::Printf(TEXT("0x%016I64X"), (QWORD&)IdToConvert.Uid);
	return Result;
}

FORCEINLINE INT HexDigit(TCHAR c)
{
	INT Result = 0;

	if (c >= '0' && c <= '9')
	{
		Result = c - '0';
	}
	else if (c >= 'a' && c <= 'f')
	{
		Result = c + 10 - 'a';
	}
	else if (c >= 'A' && c <= 'F')
	{
		Result = c + 10 - 'A';
	}
	else
	{
		Result = 0;
	}

	return Result;
}

/**
 * Converts a string representing a UniqueNetId into a UniqueNetId struct.
 *
 * @param	UniqueNetIdString	the string containing the text representation of the unique id.
 * @param	out_UniqueId		will receive the UniqueNetId generated from the string.
 *
 * @return	TRUE if the string was successfully converted into a UniqueNetId; FALSE if the string was not a valid UniqueNetId.
 */
UBOOL UOnlineSubsystem::StringToUniqueNetId( const FString& UniqueNetIdString, FUniqueNetId& out_UniqueId )
{
	UBOOL bResult=FALSE;

	// strip off the leading 0x, if it was included.
	INT Start=0;
	if ( UniqueNetIdString.Left(2) == TEXT("0x") )
	{
		Start=2;
	}

	QWORD& ConvertedValue = (QWORD&)out_UniqueId.Uid;
	ConvertedValue = 0;
	for ( INT Idx = Start; Idx < UniqueNetIdString.Len(); Idx++ )
	{
		INT NextDigit = HexDigit(UniqueNetIdString[Idx]);
		if ( NextDigit == 0 && UniqueNetIdString[Idx] != TEXT('0') )
		{
			break;
		}

		if ( Idx != Start )
		{
			ConvertedValue <<= 4;
		}
		ConvertedValue |= NextDigit;
		bResult = TRUE;
	}

	return bResult;
}

/**
 * @return	TRUE if the netids are the same
 */
UBOOL UOnlineSubsystem::AreUniqueNetIdsEqual( const FUniqueNetId& NetIdA, const FUniqueNetId& NetIdB )
{
	return (const QWORD&)NetIdA.Uid == (const QWORD&)NetIdB.Uid;
}

/**
 * Handle downloaded content in a platform-independent way
 *
 * @param Content The content descriptor that describes the downloaded content files
 *
 * @param return TRUE is successful
 */
UBOOL UOnlineSubsystem::ProcessDownloadedContent(const FOnlineContent& Content)
{
	// use engine DLC function
	GDownloadableContent->InstallDownloadableContent(*Content.FriendlyName, Content.ContentPackages, Content.ContentFiles, Content.UserIndex);

	return TRUE;
}

/**
 * Flush downloaded content for all users, making the engine stop using the content
 *
 * @param MaxNumUsers Platform specific max number of users to flush (this will iterate over all users from 0 to MaxNumUsers, as well as NO_USER 
 */
void UOnlineSubsystem::FlushAllDownloadedContent(INT MaxNumUsers)
{
	GDownloadableContent->RemoveAllDownloadableContent(MaxNumUsers);
}

/**
 * Generates a unique number based off of the current script compilation
 *
 * @return the unique number from the current script compilation
 */
INT UOnlineSubsystem::GetBuildUniqueId(void)
{
	INT Crc = 0;
	if (bUseBuildIdOverride == FALSE)
	{
		UPackage* EnginePackage = UEngine::StaticClass()->GetOutermost();
		if (EnginePackage)
		{
			// Use the GUID of the engine package to determine a unique CRC
			Crc = appMemCrc(&EnginePackage->Guid,sizeof(FGuid));
		}
	}
	else
	{
		Crc = BuildIdOverride;
	}
	return Crc;
}

