/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"

IMPLEMENT_CLASS(ADecalManager);

/** @return whether dynamic decals are enabled */
UBOOL ADecalManager::AreDynamicDecalsEnabled()
{
	return GSystemSettings.bAllowDynamicDecals;
}

void ADecalManager::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	for (INT i = 0; i < ActiveDecals.Num(); i++)
	{
		if (ActiveDecals(i).Decal == NULL || ActiveDecals(i).Decal->IsPendingKill())
		{
			ActiveDecals.Remove(i--);
		}
		else if (ActiveDecals(i).Decal->DecalReceivers.Num() == 0)
		{
			// not projecting on anything, so no point in keeping it around
			eventDecalFinished(ActiveDecals(i).Decal);
			ActiveDecals.Remove(i--);
		}
		else
		{
			// update lifetime and remove if it ran out
			ActiveDecals(i).LifetimeRemaining -= DeltaTime;
			if (ActiveDecals(i).LifetimeRemaining <= 0.f)
			{
				eventDecalFinished(ActiveDecals(i).Decal);
				ActiveDecals.Remove(i--);
			}
		}
	}
}
