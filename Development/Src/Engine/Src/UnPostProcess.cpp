/*=============================================================================
	UnPostProcess.cpp: 
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
 
IMPLEMENT_CLASS(UPostProcessEffect);
IMPLEMENT_CLASS(UPostProcessChain);
IMPLEMENT_CLASS(APostProcessVolume);

/*-----------------------------------------------------------------------------
UPostProcessEffect
-----------------------------------------------------------------------------*/

/**
* Check if the effect should be shown
* @param View - current view
* @return TRUE if the effect should be rendered
*/
UBOOL UPostProcessEffect::IsShown(const FSceneView* View) const
{
	check(View);
	check(View->Family);

	if( !(View->Family->ShowFlags & SHOW_PostProcess) )
	{
		return FALSE;
	}
	else if( View->Family->ShowFlags & SHOW_Editor )
	{
		return bShowInEditor;
	}
	else
	{
		return bShowInGame;
	}
}

/*-----------------------------------------------------------------------------
FPostProcessSceneProxy
-----------------------------------------------------------------------------*/

/** 
* Initialization constructor. 
* @param InEffect - post process effect to mirror in this proxy
*/
FPostProcessSceneProxy::FPostProcessSceneProxy(const UPostProcessEffect* InEffect)
:	DepthPriorityGroup(InEffect->SceneDPG)
,	FinalEffectInGroup(0)
,	bAffectsLightingOnly(InEffect->bAffectsLightingOnly)
{
}

/*-----------------------------------------------------------------------------
APostProcessVolume
-----------------------------------------------------------------------------*/

/**
 * Routes ClearComponents call to Super and removes volume from linked list in world info.
 */
void APostProcessVolume::ClearComponents()
{
	// Route clear to super first.
	Super::ClearComponents();

	// GWorld will be NULL during exit purge.
	if( GWorld )
	{
		APostProcessVolume* CurrentVolume  = GWorld->GetWorldInfo()->HighestPriorityPostProcessVolume;
		APostProcessVolume*	PreviousVolume = NULL;

		// Iterate over linked list, removing this volume if found.
		while( CurrentVolume )
		{
			// Found.
			if( CurrentVolume == this )
			{
				// Remove from linked list.
				if( PreviousVolume )
				{
					PreviousVolume->NextLowerPriorityVolume = NextLowerPriorityVolume;
				}
				// Special case removal from first entry.
				else
				{
					GWorld->GetWorldInfo()->HighestPriorityPostProcessVolume = NextLowerPriorityVolume;
				}

				// BREAK OUT OF LOOP
				break;
			}
			// Further traverse linked list.
			else
			{
				PreviousVolume	= CurrentVolume;
				CurrentVolume	= CurrentVolume->NextLowerPriorityVolume;
			}
		}

		// Reset next pointer to avoid dangling end bits and also for GC.
		NextLowerPriorityVolume = NULL;
	}
}

/**
 * Routes UpdateComponents call to Super and adds volume to linked list in world info.
 */
void APostProcessVolume::UpdateComponentsInternal(UBOOL bCollisionUpdate)
{
	// Route update to super first.
	Super::UpdateComponentsInternal( bCollisionUpdate );

	APostProcessVolume* CurrentVolume  = GWorld->GetWorldInfo()->HighestPriorityPostProcessVolume;
	APostProcessVolume*	PreviousVolume = NULL;

	// First volume in the world info.
	if( CurrentVolume == NULL )
	{
		GWorld->GetWorldInfo()->HighestPriorityPostProcessVolume = this;
		NextLowerPriorityVolume	= NULL;
	}
	// Find where to insert in sorted linked list.
	else
	{
		// Avoid double insertion!
		while( CurrentVolume && CurrentVolume != this )
		{
			// We use > instead of >= to be sure that we are not inserting twice in the case of multiple volumes having
			// the same priority and the current one already having being inserted after one with the same priority.
			if( Priority > CurrentVolume->Priority )
			{
				// Special case for insertion at the beginning.
				if( PreviousVolume == NULL )
				{
					GWorld->GetWorldInfo()->HighestPriorityPostProcessVolume = this;
				}
				// Insert before current node by fixing up previous to point to current.
				else
				{
					PreviousVolume->NextLowerPriorityVolume = this;
				}
				// Point to current volume, finalizing insertion.
				NextLowerPriorityVolume = CurrentVolume;

				// BREAK OUT OF LOOP.
				break;
			}
			// Further traverse linked list.
			else
			{
				PreviousVolume	= CurrentVolume;
				CurrentVolume	= CurrentVolume->NextLowerPriorityVolume;
			}
		}

		// We're the lowest priority volume, insert at the end.
		if( CurrentVolume == NULL )
		{
			check( PreviousVolume );
			PreviousVolume->NextLowerPriorityVolume = this;
			NextLowerPriorityVolume = NULL;
		}
	}
}

/**
* Called when properties change.
*/
void APostProcessVolume::PostEditChange(UProperty* PropertyThatChanged)
{
	// clamp desaturation to 0..1
	Settings.Scene_Desaturation = Clamp(Settings.Scene_Desaturation, 0.f, 1.f);

	Super::PostEditChange(PropertyThatChanged);

}

/**
* Called after this instance has been serialized.
*/
void APostProcessVolume::PostLoad()
{
	Super::PostLoad();

	// clamp desaturation to 0..1 (fixup for old data)
	Settings.Scene_Desaturation = Clamp(Settings.Scene_Desaturation, 0.f, 1.f);
}
