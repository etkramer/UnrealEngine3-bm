/*=============================================================================
	ParticleDataManager.cpp: Particle dynamic data manager implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"

/*-----------------------------------------------------------------------------
	ParticleDataManager
-----------------------------------------------------------------------------*/
//
typedef TMap<UParticleSystemComponent*, UBOOL>::TIterator TParticleDataIterator;

/**
 *	Update the dynamic data for all particle system componets
 */
void FParticleDataManager::UpdateDynamicData()
{
	for (TParticleDataIterator It(PSysComponents); It; ++It)
	{
		UParticleSystemComponent* PSysComp = (UParticleSystemComponent*)(It.Key());
		if (PSysComp)
		{
			FParticleSystemSceneProxy* SceneProxy = (FParticleSystemSceneProxy*)Scene_GetProxyFromInfo(PSysComp->SceneInfo);
			if (SceneProxy != NULL)
			{
				if (PSysComp->bRecacheViewRelevance == TRUE)
				{
					PSysComp->UpdateViewRelevance(SceneProxy);
				}

				// check to see if this PSC is active.  When you attach a PSC it gets added to the DataManager
				// even if it might be bIsActive = FALSE  (e.g. attach and later in the frame activate it)
				// or also for PSCs that are attached to a SkelComp which is being attached and reattached but the PSC itself is not active!
				if( PSysComp->bIsActive )
				{
					PSysComp->UpdateDynamicData(SceneProxy);
				}
				else
				{
					// so if we just were deactivated we want to update the renderer with NULL so the renderer will clear out the data there and not have outdated info which may/will cause a crash
					if ( (PSysComp->bWasDeactivated || PSysComp->bWasCompleted) && (PSysComp->SceneInfo) )
					{
						SceneProxy->UpdateData(NULL);
					}
				}
			}
		}
	}
	Clear();
}
	
//
/**
 *	Add a particle system component to the list.
 *
 *	@param		InPSysComp		The particle system component to add.
 *
 */
void FParticleDataManager::AddParticleSystemComponent(UParticleSystemComponent* InPSysComp)
{
	if ((GIsUCC == FALSE) && (GIsCooking == FALSE))
	{
		if (InPSysComp)
		{
			PSysComponents.Set(InPSysComp, TRUE);
		}
	}
}

/**
 *	Remove a particle system component to the list.
 *
 *	@param		InPSysComp		The particle system component to remove.
 *
 */
void FParticleDataManager::RemoveParticleSystemComponent(UParticleSystemComponent* InPSysComp)
{
	if ((GIsUCC == FALSE) && (GIsCooking == FALSE))
	{
		PSysComponents.Remove(InPSysComp);
	}
}

/**
 *	Clear all pending components from the queue.
 *
 */
void FParticleDataManager::Clear()
{
	PSysComponents.Reset();
}
