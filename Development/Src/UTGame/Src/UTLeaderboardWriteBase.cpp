//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTGame.h"

IMPLEMENT_CLASS(UUTLeaderboardWriteBase);

void UUTLeaderboardWriteBase::SetPureServerMode(const UBOOL bIsPureServer)
{
	if (bIsPureServer)
	{
		ViewIds = PureViewIds;
	}
	else
	{
		ViewIds = static_cast<UUTLeaderboardWriteBase*>(GetClass()->GetDefaultObject())->ViewIds;
	}
}

UBOOL UUTLeaderboardWriteBase::GetPropertyIdFromStatType(FName StatName,INT& StatId, BYTE& StatType)
{
	INT i, Len;	  

	Len = StatNameToStatIdMapping.Num();
	for (i=0; i<Len; i++)
	{
		if (StatNameToStatIdMapping(i).StatName == StatName)
		{
			const FStatMappingEntry& Entry = StatNameToStatIdMapping(i);
			StatId = Entry.Id;
			FSettingsData* Stat = FindStat(StatId);
			if (Stat != NULL)
			{
				StatType = Stat->Type;
				return TRUE;
			}
			else
			{
				warnf(TEXT("%s (%d) has a propertyID mapping, but no statistic"), *StatName.ToString(), StatId);
			}
		}
	}

	//warnf(*(StatName.ToString() + TEXT(" does not have a propertyID mapping")));
	StatId = 0;
	StatType = SDT_Empty;
	return FALSE;
}

UBOOL UUTLeaderboardWriteBase::SetIntStatFromMapping(FName StatName,INT StatValue)
{
	UBOOL ValueSet;
	INT StatId;
	BYTE DataType;
	ValueSet = FALSE;
	if (GetPropertyIdFromStatType(StatName, StatId, DataType))
	{
		if (DataType == SDT_Int32)
		{
			//debugf(TEXT("Setting %s to %d"), *StatName.ToString(), StatValue);
			SetIntStat(StatId, StatValue);
			ValueSet = TRUE;
		}
		else
		{
			FString StatString = StatName.ToString();
			warnf(*(StatString + TEXT(" mapping found but has invalid data type")));
		}
	}

	return ValueSet;
}

UBOOL UUTLeaderboardWriteBase::SetFloatStatFromMapping(FName StatName,FLOAT StatValue)
{
	UBOOL ValueSet;
	INT StatId;
	BYTE DataType;
	ValueSet = FALSE;
	if (GetPropertyIdFromStatType(StatName, StatId, DataType))
	{
		if (DataType == SDT_Float || DataType == SDT_Double)
		{
			//debugf(TEXT("Setting %s to %f"), *StatName.ToString(), StatValue);
			SetFloatStat(StatId, StatValue);
			ValueSet = TRUE;
		}
		else
		{
			FString StatString = StatName.ToString();
			warnf(*(StatString + TEXT(" mapping found but has invalid data type")));
		}
	}

	return ValueSet;
}


void UUTLeaderboardWriteBase::PrintDebugInformation(UOnlineSubsystem* OnlineSubsystem)
{
	for (INT i=0; i<ViewIds.Num(); i++)
	{
	  debugf(TEXT("ViewID: %d"), ViewIds(i));
	}
	
	for (INT i=0; i<ArbitratedViewIds.Num(); i++)
	{
	  debugf(TEXT("ArbitratedViewId: %d"), ArbitratedViewIds(i));
	}

	debugf(TEXT("NumProperties: %d NumMappings: %d"), Properties.Num(), StatNameToStatIdMapping.Num());

	//Print out all the properties in this mode
	
	FName NoName(TEXT("NONE"));
	for (INT i=0; i<Properties.Num(); i++)
	{
		const FSettingsProperty& SettingsProperty = Properties(i);

		FName PropertyName(TEXT("NONE"));
		for (INT j=0; j<StatNameToStatIdMapping.Num(); j++)
		{
			if (StatNameToStatIdMapping(j).Id == SettingsProperty.PropertyId)
			{
				PropertyName = StatNameToStatIdMapping(j).StatName;
				break;
			}
		}

		if (PropertyName == NoName)
		{
			debugf(TEXT("Property 0x%08x does not have a stats mapping"), SettingsProperty.PropertyId);
		}

		INT anInt;
		FLOAT aFloat;
		DOUBLE aDouble;
		switch(SettingsProperty.Data.Type)
		{
		case SDT_Double:
			SettingsProperty.Data.GetData(aDouble);
			debugf(TEXT("%s PropID: 0x%08x DoubleVal: %f"), *PropertyName.GetNameString(), SettingsProperty.PropertyId, aDouble);
			break;
		case SDT_Int32:
			SettingsProperty.Data.GetData(anInt);
			debugf(TEXT("%s PropID: 0x%08x IntVal: %d"), *PropertyName.GetNameString(), SettingsProperty.PropertyId, anInt);
			break;
		case SDT_Float:
			SettingsProperty.Data.GetData(aFloat);
			debugf(TEXT("%s PropID: 0x%08x FloatVal: %f"), *PropertyName.GetNameString(), SettingsProperty.PropertyId, aFloat);
			break;
		default:
			debugf(TEXT("%s PropID: 0x%08x OtherVal"), *PropertyName.GetNameString(), SettingsProperty.PropertyId);
			break;
		}
	}
		
	//Print out all the mappings
	for (INT i=0; i<StatNameToStatIdMapping.Num(); i++)
	{
	   const FStatMappingEntry& StatMapping = StatNameToStatIdMapping(i);
	   FSettingsData* Stat = FindStat(StatMapping.Id);
	   if (Stat != NULL)
	   {
		   debugf(TEXT("StatMapping %s ID: 0x%08x"), *StatMapping.StatName.GetNameString(), StatMapping.Id);
	   }
	   else
	   {
		   debugf(TEXT("StatMapping %s does not map to valid PropertyId"), *StatMapping.StatName.GetNameString());
	   }
	}
}
