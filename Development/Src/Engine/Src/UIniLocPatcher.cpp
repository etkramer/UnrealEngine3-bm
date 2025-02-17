/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"

#include "FConfigCacheIni.h"

IMPLEMENT_CLASS(UIniLocPatcher);

/**
 * Builds the path information needed to find the file in the config cache
 *
 * @param FileName the file to prepend with the path
 *
 * @return the new file name to use
 */
static FString CreateIniLocFilePath(const FString& FileName)
{
	FString NewFilename;
	FFilename DownloadedFile(FileName);
	FString Extension = DownloadedFile.GetExtension();
	// Use the extension to determine if it is loc or not
	if (Extension == TEXT("ini"))
	{
		NewFilename = appGameConfigDir() + FileName;
	}
	else
	{
		// Build in the proper language sub-directory (..\ExampleGame\Localization\fra\ExampleGame.fra)
		NewFilename = GSys->LocalizationPaths(0) * Extension * FileName;
	}
	return NewFilename;
}

/**
 * Takes the data, merges with the INI/Loc system, and then reloads the config for the
 * affected objects
 *
 * @param FileName the name of the file being merged
 * @param FileData the file data to merge with the config cache
 */
void UIniLocPatcher::ProcessIniLocFile(const FString& FileName,TArray<BYTE>& FileData)
{
	// Make sure that data was returned
	if (FileName.Len() > 0 && FileData.Num())
	{
		const FString FileNameWithPath = CreateIniLocFilePath(FileName);
		// Find the config file in the config cache and skip if not present
		FConfigFile* ConfigFile = GConfig->FindConfigFile(*FileNameWithPath);
		if (ConfigFile)
		{
			// Make sure the string is null terminated
			((TArray<BYTE>&)FileData).AddItem(0);
			// Now convert to a string for config updating
			const FString IniLocData = (const ANSICHAR*)FileData.GetTypedData();
			// Merge the string into the config file
			ConfigFile->CombineFromBuffer(*FileName,*IniLocData);
			TArray<UClass*> Classes;
			INT StartIndex = 0;
			INT EndIndex = 0;
			// Find the set of object classes that were affected
			while (StartIndex >= 0 && StartIndex < IniLocData.Len())
			{
				// Find the next section header
				StartIndex = IniLocData.InStr(TEXT("["),FALSE,FALSE,StartIndex);
				if (StartIndex > -1)
				{
					// Find the ending section identifier
					EndIndex = IniLocData.InStr(TEXT("]"),FALSE,FALSE,StartIndex);
					if (EndIndex > StartIndex)
					{
						// Snip the text out and try to find the class for that
						const FString ClassName = IniLocData.Mid(StartIndex + 1,EndIndex - StartIndex - 1);
						// Find the class for this so we know what to update
						UClass* Class = FindObject<UClass>(NULL,*ClassName,TRUE);
						if (Class)
						{
							// Add this to the list to check against
							Classes.AddItem(Class);
						}
						StartIndex = EndIndex;
					}
				}
			}
			DOUBLE StartTime = appSeconds();
			if (Classes.Num())
			{
				// Now that we have a list of classes to update, we can iterate objects and reload
				for (FObjectIterator It; It; ++It)
				{
					UClass* Class = It->GetClass();
					// Don't do anything for non-config classes
					if (Class->HasAnyClassFlags(CLASS_Config))
					{
						// Check to see if this class is in our list
						for (INT ClassIndex = 0; ClassIndex < Classes.Num(); ClassIndex++)
						{
							if (It->IsA(Classes(ClassIndex)))
							{
								// Force a reload of the config vars
								It->ReloadConfig();
							}
						}
					}
				}
			}
			debugf(TEXT("Updating config/loc from %s took %f seconds"),*FileName,(FLOAT)(appSeconds() - StartTime));
		}
	}
}
