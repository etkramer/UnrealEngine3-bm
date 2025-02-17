//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "GearGame.h"

UBOOL UGearEventsInterface::Init(INT MaxNumEvents)
{
	return FALSE;
}

void UGearEventsInterface::BeginLog()
{
}

void UGearEventsInterface::EndLog()
{
}

void UGearEventsInterface::BeginEvent(const FString& EventName)
{
}

void UGearEventsInterface::AddParamInt(const FString& ParamName, INT ParamValue)
{
}

void UGearEventsInterface::AddParamFloat(const FString& ParamName, FLOAT ParamValue)
{
}

void UGearEventsInterface::AddParamString(const FString& ParamName, const FString& ParamValue)
{
}

void UGearEventsInterface::EndEvent()
{
}
