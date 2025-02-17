/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"
#include "ScenePrivate.h"

IMPLEMENT_CLASS(ULightEnvironmentComponent);

void ULightEnvironmentComponent::SetEnabled(UBOOL bNewEnabled)
{
	if (bNewEnabled != bEnabled) 
	{
		// Make a copy of the component's owner, since this component may be detached during the call.
		AActor* LocalOwner = Owner;

		// Detach the owner's components.
		// This assumes that all the components using the light environment are attached to the same actor as the light environment.
		if(LocalOwner)
		{
			LocalOwner->ClearComponents();
		}

		bEnabled = bNewEnabled;

		// Reattach the owner's components with the new enabled state.
		if(LocalOwner)
		{
			LocalOwner->ConditionalUpdateComponents(FALSE);
		}
	}
}

UBOOL ULightEnvironmentComponent::IsEnabled() const
{
	return bEnabled;
}

void ULightEnvironmentComponent::AddAffectedComponent(UPrimitiveComponent* NewComponent)
{
	AffectedComponents.AddItem(NewComponent);
}

void ULightEnvironmentComponent::RemoveAffectedComponent(UPrimitiveComponent* OldComponent)
{
	// The order of primitive components using this light environment does not matter
	AffectedComponents.RemoveItemSwap(OldComponent);
}
