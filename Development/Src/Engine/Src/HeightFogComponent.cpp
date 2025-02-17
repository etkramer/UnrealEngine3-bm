/*=============================================================================
	HeightFogComponent.cpp: Height fog implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UHeightFogComponent);

void UHeightFogComponent::SetParentToWorld(const FMatrix& ParentToWorld)
{
	Super::SetParentToWorld(ParentToWorld);
	Height = ParentToWorld.GetOrigin().Z;
}

void UHeightFogComponent::Attach()
{
	Super::Attach();
	if(bEnabled)
	{
		Scene->AddHeightFog(this);
	}
}

void UHeightFogComponent::UpdateTransform()
{
	Super::UpdateTransform();
	Scene->RemoveHeightFog(this);
	if(bEnabled)
	{
		Scene->AddHeightFog(this);
	}
}

void UHeightFogComponent::Detach( UBOOL bWillReattach )
{
	Super::Detach( bWillReattach );
	Scene->RemoveHeightFog(this);
}

void UHeightFogComponent::SetEnabled(UBOOL bSetEnabled)
{
	if(bEnabled != bSetEnabled)
	{
		// Update bEnabled, and begin a deferred component reattach.
		bEnabled = bSetEnabled;
		BeginDeferredReattach();
	}
}

