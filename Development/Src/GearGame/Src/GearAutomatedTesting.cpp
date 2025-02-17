/**
 * MGS automation hooks
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
#include "GearGame.h"

#include "GearGameSequenceClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameWeaponClasses.h"



#if XBOX
	#include "Xtl.h"
#elif PS3
   // do nothing
#else
	#include "Iphlpapi.h"		// needed for adapter_info 
#endif	

// START justinmc@microsoft.com 2/12/2008
// Get Mac Address code taken from UnPhysLevel.cpp
#define NO_NIC_MAC	"AGEIA"
#define MAX_MAC_LENGTH 10
// Moving declaration out of MaskMacAddress() due to 
// stack corruption issues
TCHAR AnsiAddress[MAX_MAC_LENGTH];

// END justinmc@microsoft.com 2/12/2008

// MSSTART brantsch@microsoft.com - 05/09/2008: Debug Message functionality for MS tools
void UGearCheatManager::MS_SendDebugMessage(const FString& Msg)
{
	TCHAR sendlocation[MAX_SPRINTF];
	int i = 0;
	for(i = 0; i < appStrlen(*Msg); i++)
		sendlocation[i] = Msg[i];
	sendlocation[i] = '\0';
	appSendNotificationString(TCHAR_TO_ANSI(sendlocation));
}
// MSEND brantsch@microsoft.com - 05/09/2008: Debug Message functionality for MS tools

// MSSTART justinmc@microsoft.com- 4/3/2008: Exposing debug hooks
void AGearPC::MS_GetPlayerLoc(INT MessageGUID)
{	
	TCHAR sendlocation[MAX_SPRINTF] = TEXT("");
	if(Pawn != NULL)
	{
		appSprintf(sendlocation, TEXT("UNREAL!UDC!0|1|%d|GetPlayerLoc|%s|%.0f|%.0f|%.0f\r\n"),MessageGUID, *WorldInfo->GetMapName(),Pawn->Location.X,Pawn->Location.Y,Pawn->Location.Z);
	}
	else
	{
		appSprintf(sendlocation, TEXT("UNREAL!UDC!0|1|%d|GetPlayerLoc|Error:Unable to get player location due to NULL pawn\r\n"),MessageGUID);
	}
	appSendNotificationString(TCHAR_TO_ANSI(sendlocation));
}
// MSEND justinmc@microsoft.com- 4/3/2008

// MSSTART brantsch@microsoft.com 5/12/2008: Movement hooks
void AGearPC::MS_GetPlayerRotation(INT MessageGUID)
{
	TCHAR playerrotation[MAX_SPRINTF] = TEXT("");
	if(Pawn != NULL)
	{
		appSprintf(playerrotation, TEXT("UNREAL!UDC!0|1|%d|GetPlayerRotation|%d|%d|%d\r\n"),MessageGUID, Pawn->Rotation.Pitch, Pawn->Rotation.Yaw, Pawn->Rotation.Roll);
	}
	else
	{
		appSprintf(playerrotation, TEXT("UNREAL!UDC!0|1|%d|GetPlayerRotation|Error:Unable to get player rotation due to NULL pawn\r\n"), MessageGUID);
	}
	appSendNotificationString(TCHAR_TO_ANSI(playerrotation));	
}

void AGearPC::MS_GetCameraRotation(INT MessageGUID)
{
	TCHAR camerarotation[MAX_SPRINTF] = TEXT("");
	if(PlayerCamera != NULL)
	{
		appSprintf(camerarotation, TEXT("UNREAL!UDC!0|1|%d|GetCameraRotation|%d|%d|%d\r\n"),MessageGUID, PlayerCamera->Rotation.Pitch, PlayerCamera->Rotation.Yaw, PlayerCamera->Rotation.Roll);
	}
	else
	{
		appSprintf(camerarotation, TEXT("UNREAL!UDC!0|1|%d|GetCameraRotation|Error:Unable to get camera rotation due to NULL PlayerCamera\r\n"), MessageGUID);
	}
	appSendNotificationString(TCHAR_TO_ANSI(camerarotation));		
}
// MSEND brantsch@microsoft.com 5/12/2008: Movement hooks
void AGearPawn::GetCoverInfo()
{
	TCHAR coverstatus[MAX_SPRINTF]= TEXT("");

	if (IsInCover())
	{
		appSprintf(coverstatus, TEXT("UNREAL!CoverInfo:%s.%s|%d\n"), *CurrentLink->GetOutermost()->GetName(), *CurrentLink->GetName(), CurrentSlotIdx);
	}
	else
	{
		appSprintf(coverstatus, TEXT("UNREAL!CoverInfo:0\n"));
	}

	appOutputDebugString(coverstatus);
}

void AGearPawn::GetStreamingInfo(const FString& levelstatus)
{
	ANSICHAR streamstatus[100];
	ANSICHAR PackageName[MAX_SPRINTF]="";
	FString FPkgName;

	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
	{
		ULevelStreaming* LevelStreaming = WorldInfo->StreamingLevels(LevelIndex);
		if( LevelStreaming && LevelStreaming->PackageName != NAME_None && LevelStreaming->PackageName != GWorld->GetOutermost()->GetFName() )
		{

			FPkgName = LevelStreaming->PackageName.ToString();
			int i = 0;
			for(i = 0; i < appStrlen(*FPkgName); i++)
				PackageName[i] = (ANSICHAR) FPkgName[i];
			PackageName[i] = '\0';


			if( LevelStreaming->LoadedLevel && !LevelStreaming->bHasUnloadRequestPending )
			{
				if( GWorld->Levels.FindItemIndex( LevelStreaming->LoadedLevel ) != INDEX_NONE )
				{
					if(appStrcmp(*levelstatus,TEXT("Visible"))==0)
					{
						appSprintfANSI(streamstatus, "UNREAL!StreamInfo:%s",(const ANSICHAR*)PackageName);
					}
					else if(appStrcmp(*levelstatus,TEXT("All"))==0)
					{
						appSprintfANSI(streamstatus, "UNREAL!StreamAll:%s,Visible",(const ANSICHAR*)PackageName);

					}
				}
				else
				{
					if(appStrcmp(*levelstatus,TEXT("Loaded"))==0)
					{
						appSprintfANSI(streamstatus, "UNREAL!StreamInfo:%s",(const ANSICHAR*)PackageName);
					}
					else if(appStrcmp(*levelstatus,TEXT("All"))==0)
					{
						appSprintfANSI(streamstatus, "UNREAL!StreamAll:%s,Loaded",(const ANSICHAR*)PackageName);
					}
				}
			}
			else
			{
				if( FindObject<UPackage>( ANY_PACKAGE, *LevelStreaming->PackageName.ToString() ) )
				{
					if(appStrcmp(*levelstatus,TEXT("UnLoaded"))==0)
					{
						appSprintfANSI(streamstatus, "UNREAL!StreamInfo:%s",(const ANSICHAR*)PackageName);
					}
					else if(appStrcmp(*levelstatus,TEXT("All"))==0)
					{
						appSprintfANSI(streamstatus, "UNREAL!StreamAll:%s,UnLoaded",(const ANSICHAR*)PackageName);
					}

				}
				else
				{
					if(appStrcmp(*levelstatus,TEXT("NotLoaded"))==0)
					{
						appSprintfANSI(streamstatus, "UNREAL!StreamInfo:%s",(const ANSICHAR*)PackageName);
					}
					else if(appStrcmp(*levelstatus,TEXT("All"))==0)
					{
						appSprintfANSI(streamstatus, "UNREAL!StreamAll:%s,NotLoaded",(const ANSICHAR*)PackageName);
					}
				}
			}
			appSendNotificationString(streamstatus);
		}
	}
}

// MSSTART brantsch@microsoft.com 4/16/2008
/**
 * Builds the tabbing for an XML string
 * 
 * @param IndentCount the number of tabs to indent
 *
 * @return the string of tabs to use
 */
FString MS_BuildIndentString(DWORD IndentCount)
{
	FString Indent;
	for (DWORD Count = 0; Count < IndentCount; Count++)
	{
		Indent += TEXT('\t');
	}
	return Indent;
}

/**
 * Builds an XML string out of an individual property
 *
 * @param OutXmlString the out string
 * @param Property the list of properties to serialize
 * @param IndentCount the number of indents to use
 */
void MS_ToXml(FString& OutXmlString, UGearProfileSettings* profsettings, const FSettingsProperty& Property,const FString& InIndent)
{
	// Don't serialize empty properties
	if (Property.Data.Type != SDT_Empty)
	{
		FString Indent(InIndent);
		Indent += TEXT('\t');
		
		// Create an xml entry from this property
		OutXmlString += FString::Printf(TEXT("%s<%s id=\"%d\" idname=\"%s\" value=\"%s\"  valuename=\"%s\"/>\r\n"),
			*Indent,
			Property.Data.GetTypeString(),
			Property.PropertyId,
			*profsettings->GetProfileSettingName(Property.PropertyId).ToString(),
			*Property.Data.ToString(),
			*profsettings->GetProfileSettingValueName(Property.PropertyId).ToString());
	}
}

/**
 * Builds an XML string out of an array of properties
 *
 * @param OutXmlString the out string
 * @param Properties the list of properties to serialize
 * @param IndentCount the number of indents to use
 */
void MS_ToXml(FString& OutXmlString,UGearProfileSettings* profsettings, const TArray<FSettingsProperty>& Properties,DWORD IndentCount = 0)
{
	FString Indent = MS_BuildIndentString(IndentCount);
	OutXmlString += Indent;
	OutXmlString += TEXT("<SettingsProperties>\r\n");
	// Append each property
	for (INT PropIndex = 0; PropIndex < Properties.Num(); PropIndex++)
	{
		MS_ToXml(OutXmlString, profsettings, Properties(PropIndex),Indent);
	}
	OutXmlString += Indent;
	OutXmlString += TEXT("</SettingsProperties>\r\n");
}

/**
 * Builds an XML string out of an array of profile settings
 *
 * @param OutXmlString the out string
 * @param Settings the list of profile settings to serialize
 * @param IndentCount the number of indents to use
 */
void MS_ToXml(FString& OutXmlString, UGearProfileSettings* profsettings, const TArray<FOnlineProfileSetting>& Settings,DWORD IndentCount = 0)
{
	FString Indent = MS_BuildIndentString(IndentCount);
	OutXmlString += Indent;
	OutXmlString += TEXT("<ProfileSettings>\r\n");
	// Append each property
	for (INT SettingsIndex = 0; SettingsIndex < Settings.Num(); SettingsIndex++)
	{
		MS_ToXml(OutXmlString, profsettings, Settings(SettingsIndex).ProfileSetting,Indent);
	}
	OutXmlString += Indent;
	OutXmlString += TEXT("</ProfileSettings>\r\n");
}
void AGearPC::MS_DumpProfile()
{
	TCHAR profilexml[MAX_SPRINTF]= TEXT("");

	FString XmlPayload = TEXT("");
	MS_ToXml(XmlPayload, this->ProfileSettings, this->ProfileSettings->ProfileSettings,0);	
	appOutputDebugString(*XmlPayload);

}

void AGearPC::MS_GetProfileSettingName(INT ProfileSettingId)
{
	FString result = FString::Printf(TEXT("<MS_GetProfileSettingName Name=\"%s\">\r\n"),
		*this->ProfileSettings->GetProfileSettingName(ProfileSettingId).ToString());

	appOutputDebugString(*result);
}

void AGearPC::MS_GetProfileSettingValueList(INT ProfileSettingId)
{
	FString outputresult = TEXT("<MS_GetProfileSettingValueList Values=");
	TArray<FName> results;
	if (this->ProfileSettings->GetProfileSettingValues(ProfileSettingId, results))
	{
		for (INT index = 0; index < results.Num(); index++)
		{		
			outputresult += results(index).ToString();
			if (index + 1 < results.Num() )
				outputresult += TEXT(",");
		}
	}
	else
		outputresult += TEXT("<UNMAPPED SETTING ID>");
	outputresult += TEXT(">\r\n");
	appOutputDebugString(*outputresult);
}

void AGearPC::MS_GetProfileSettingValueName(INT ProfileSettingId)
{
	FString result = FString::Printf(TEXT("<MS_GetProfileSettingValueName ValueName=\"%s\">\r\n"),
		*this->ProfileSettings->GetProfileSettingValueName(ProfileSettingId).ToString());
	appOutputDebugString(*result);
}

void AGearPC::MS_SetProfileSettingValueId(INT ProfileSettingId,INT Value)
{
	FString outstring;
	if (this->ProfileSettings->SetProfileSettingValueInt(ProfileSettingId,Value))
	{
		outstring = FString::Printf(TEXT("<MS_SetProfileSettingValueId SUCCEEDED in setting ID\"%d\" to value=\"%d\">\r\n"),
			ProfileSettingId,Value);
	}
	else
	{
		outstring = FString::Printf(TEXT("<MS_SetProfileSettingValueId FAILED in setting ID\"%d\" to value=\"%d\">\r\n"),
			ProfileSettingId,Value);
	}

	appOutputDebugString(*outstring);
}
void AGearPC::MS_SetProfileSettingValueByName(INT ProfileSettingId,const FString& NewValue)
{
	FString outstring;
	if (this->ProfileSettings->SetProfileSettingValue(ProfileSettingId,NewValue))
	{
		outstring = FString::Printf(TEXT("<MS_SetProfileSettingValueByName SUCCEEDED in setting ID\"%d\" to value=\"%s\">\r\n"),
			ProfileSettingId,*NewValue);
	}
	else
	{
		outstring = FString::Printf(TEXT("<MS_SetProfileSettingValueId FAILED in setting ID\"%d\" to value=\"%s\">\r\n"),
			ProfileSettingId,*NewValue);
	}

	appOutputDebugString(*outstring);
}
// MSEND brantsch@microsoft.com 4/16/2008

// MSSTART justinmc@microsoft.com 06/27/2008: Function for tagging sound cue extraction
void UGearCheatManager::MS_GetSoundCueInfo()
{
	// signal the start of the sound cues
	appOutputDebugString(TEXT("<MSSTART_ExtractSoundCues>\n"));

	UObject::StaticExec(TEXT("OBJ LIST CLASS=SOUNDCUE"));

	// signal the end of the sound cues
	appOutputDebugString(TEXT("<MSEND_ExtractSoundCues>\n"));
}
// MSEND

