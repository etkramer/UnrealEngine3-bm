/*=============================================================================
	ScopedPropertyChange.cpp: Implementation of the FScopedPropertyChange class.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"
#include "Properties.h"
#include "ScopedPropertyChange.h"

/* ==========================================================================================================
	FScopedPropertyChange
========================================================================================================== */
/**
 * Constructor
 *
 * @param	InModifiedItem		the property window item corresponding to the property that was modified.
 * @param	InModifiedProperty	alternate property to use when calling Pre/PostEditChange; if not specified,
 *								InModifiedItem->Property is used
 * @param	bInPreventPropertyWindowRebuild
 *								if TRUE, will prevent CB_ObjectPropertyChanged from calling Rebuild() on the WxPropertyWindow
 *								which contains InModifiedItem.  Useful if InModifiedItem is handling the property value change
 *								notification itself, as Rebuild() would cause InModifiedItem to be deleted before we go out of scope.
 */
FScopedPropertyChange::FScopedPropertyChange( WxPropertyWindow_Base* InModifiedItem, UProperty* InModifiedProperty/*=NULL*/, UBOOL bInPreventPropertyWindowRebuild/*=FALSE*/ )
: PropagationArchive(NULL), PreviousPropagationArchive(NULL)
, ModifiedItem(InModifiedItem), ModifiedProperty(InModifiedProperty)
, bPreventPropertyWindowRebuild(bInPreventPropertyWindowRebuild)
{
	if ( ModifiedProperty == NULL && ModifiedItem != NULL )
	{
		ModifiedProperty = ModifiedItem->Property;
	}

	BeginEdit();
}

/** Destructor */
FScopedPropertyChange::~FScopedPropertyChange()
{
	FinishEdit();
}

/**
 * Creates the archetype propagation archive and send the PreEditChange notifications.
 */
void FScopedPropertyChange::BeginEdit()
{
	if ( ModifiedItem != NULL )
	{
		// store the existing value of GMemoryArchive, so that we don't clobber it if this class is used in a recursive method
		PreviousPropagationArchive = GMemoryArchive;

		// Create an FArchetypePropagationArc to propagate the updated property values from archetypes to instances of that archetype
		GMemoryArchive = PropagationArchive = new FArchetypePropagationArc();

		// notify the object that this property's value is about to be changed
		ModifiedItem->NotifyPreChange(ModifiedProperty);
	}
}

/**
 * Sends the PostEditChange notifications and deletes the archetype propagation archive.
 */
void FScopedPropertyChange::FinishEdit()
{
	if ( ModifiedItem != NULL )
	{
		check(PropagationArchive);
		check(ModifiedProperty);

		// now change the propagation archive to read mode
		PropagationArchive->ActivateReader();

		// Note the current property window so that CALLBACK_ObjectPropertyChanged
		// doesn't destroy the window out from under us.
		WxPropertyWindow* PreviousPropertyWindow = NULL;
		if ( bPreventPropertyWindowRebuild && GApp != NULL )
		{
			PreviousPropertyWindow = GApp->CurrentPropertyWindow;
			GApp->CurrentPropertyWindow = ModifiedItem->GetPropertyWindow();
		}

		// notify the object that this property's value has been changed
		ModifiedItem->NotifyPostChange(ModifiedProperty);

		// Unset, effectively making this property window updatable by CALLBACK_ObjectPropertyChanged.
		if ( bPreventPropertyWindowRebuild && GApp != NULL )
		{
			GApp->CurrentPropertyWindow = PreviousPropertyWindow;
		}

		// if GMemoryArchive is still pointing to the one we created, restore it to the previous value
		if ( GMemoryArchive == PropagationArchive )
		{
			GMemoryArchive = PreviousPropagationArchive;
		}

		// clean up the FArchetypePropagationArc we created
		delete PropagationArchive;
		PropagationArchive = NULL;
		PreviousPropagationArchive = NULL;
		ModifiedItem = NULL;
		ModifiedProperty = NULL;
	}
}



// EOF



