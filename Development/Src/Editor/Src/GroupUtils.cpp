/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "GroupUtils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Helper functions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @return		TRUE if the actor can be considered by the group browser, FALSE otherwise.
 */
static inline UBOOL IsValid(const AActor* Actor)
{
	const UBOOL bIsBuilderBrush	= ( Actor == GWorld->GetBrush() );
	const UBOOL bIsHidden		= ( Actor->GetClass()->GetDefaultActor()->bHiddenEd == TRUE );
	const UBOOL bIsValid		= !bIsHidden && !bIsBuilderBrush;

	return bIsValid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Operations on an individual actor.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Adds the actor to the named group.
 *
 * @return		TRUE if the actor was added.  FALSE is returned if the
 *              actor already belongs to the group.
 */
UBOOL FGroupUtils::AddActorToGroup(AActor* Actor, const FString& GroupName, UBOOL bModifyActor)
{
	if(	IsValid( Actor ) )
	{
		// Don't add the actor to a group it already belongs to.
		if( !Actor->IsInGroup( *GroupName ) )
		{
			if ( bModifyActor )
			{
				Actor->Modify();
			}

			const FString CurGroup = Actor->Group.ToString();
			Actor->Group = FName( *FString::Printf(TEXT("%s%s%s"), *CurGroup, (CurGroup.Len() ? TEXT(","):TEXT("")), *GroupName ) );

			// Remove the actor from "None.
			const FString NoneGroup( TEXT("None") );
			if ( Actor->IsInGroup( *NoneGroup ) )
			{
				RemoveActorFromGroup( Actor, NoneGroup, FALSE );
			}
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Removes an actor from the specified group.
 *
 * @return		TRUE if the actor was removed from the group.  FALSE is returned if the
 *              actor already belonged to the group.
 */
UBOOL FGroupUtils::RemoveActorFromGroup(AActor* Actor, const FString& GroupToRemove, UBOOL bModifyActor)
{
	if(	IsValid( Actor ) )
	{
		TArray<FString> GroupArray;
		FString GroupName = Actor->Group.ToString();
		GroupName.ParseIntoArray( &GroupArray, TEXT(","), FALSE );

		const UBOOL bRemovedFromGroup = GroupArray.RemoveItem( GroupToRemove ) > 0;
		if ( bRemovedFromGroup )
		{
			// Reconstruct a new group name list for the actor.
			GroupName = TEXT("");
			for( INT GroupIndex = 0 ; GroupIndex < GroupArray.Num() ; ++GroupIndex )
			{
				if( GroupName.Len() )
				{
					GroupName += TEXT(",");
				}
				GroupName += GroupArray(GroupIndex);
			}

			if ( bModifyActor )
			{
				Actor->Modify();
			}
			Actor->Group = FName( *GroupName );
			return TRUE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Operations on selected actors.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Adds selected actors to the named group.
 *
 * @param	GroupName	A group name.
 * @return				TRUE if at least one actor was added.  FALSE is returned if all selected
 *                      actors already belong to the named group.
 */
UBOOL FGroupUtils::AddSelectedActorsToGroups(const TArray<FString>& GroupNames)
{
	UBOOL bReturnVal = FALSE;

	const FString NoneString( TEXT("None") );
	const UBOOL bAddingToNoneGroup = GroupNames.ContainsItem( NoneString );

	TArray<FString> GroupArray;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( IsValid( Actor ) )
		{
			// Break the actors group names down and only add the ones back in that don't occur in GroupNames.
			FString GroupName = Actor->Group.ToString();
			GroupName.ParseIntoArray( &GroupArray, TEXT(","), FALSE );

			const INT OldGroupNum = GroupArray.Num();
			const UBOOL bWasInNoneGroup = GroupArray.ContainsItem( NoneString );

			// Add groups to the list.
			for( INT x = 0 ; x < GroupNames.Num() ; ++x )
			{
				GroupArray.AddUniqueItem( GroupNames(x) );
			}

			// Were any new groups added to this actor?
			if ( OldGroupNum != GroupArray.Num() )
			{
				// Reconstruct a new group name list for the actor.
				GroupName = TEXT("");

				// Skip over the "None" group if the actor used to belong to that group and we're not adding it.
				const UBOOL bSkipOverNoneGroup = bWasInNoneGroup && !bAddingToNoneGroup;

				for( INT GroupIndex = 0 ; GroupIndex < GroupArray.Num() ; ++GroupIndex )
				{
					// Skip over the "None" group if the actor used to belong to that group
					if ( bSkipOverNoneGroup && GroupArray(GroupIndex) == NoneString )
					{
						continue;
					}

					if( GroupName.Len() )
					{
						GroupName += TEXT(",");
					}
					GroupName += GroupArray(GroupIndex);
				}
				Actor->Modify();
				Actor->Group = FName( *GroupName );
				bReturnVal = TRUE;
			}
		}
	}

	return bReturnVal;
}

/**
 * Adds selected actors to the named groups.
 *
 * @param	GroupNames	A valid list of group names.
 * @return				TRUE if at least one actor was added.  FALSE is returned if all selected
 *                      actors already belong to the named groups.
 */
UBOOL FGroupUtils::AddSelectedActorsToGroup(const FString& GroupName)
{
	UBOOL bReturnVal = FALSE;

	const FString NoneGroup( TEXT("None") );
	const UBOOL bAddingToNoneGroup = (GroupName == NoneGroup);

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( IsValid( Actor ) )
		{
			// Don't add the actor to a group it already belongs to.
			if( !Actor->IsInGroup( *GroupName ) )
			{
				// Add the group to the actors group list.
				Actor->Modify();

				const FString CurGroup = Actor->Group.ToString();
				Actor->Group = FName( *FString::Printf(TEXT("%s%s%s"), *CurGroup, (CurGroup.Len() ? TEXT(","):TEXT("")), *GroupName ) );

				// If we're not adding to the "None" group and the actor has single membership in the "None" group, remove the actor from "None.
				if ( !bAddingToNoneGroup && Actor->IsInGroup( *NoneGroup ) )
				{
					RemoveActorFromGroup( Actor, NoneGroup, FALSE );
				}
				bReturnVal = TRUE;
			}
		}
	}
	return bReturnVal;
}

/**
 * Removes selected actors from the named groups.
 *
 * @param	GroupNames	A valid list of group names.
 * @return				TRUE if at least one actor was removed.
 */
UBOOL FGroupUtils::RemoveSelectedActorsFromGroups(const TArray<FString>& GroupNames)
{
	UBOOL bReturnVal = FALSE;
	TArray<FString> GroupArray;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( IsValid( Actor ) )
		{
			// Break the actors group names down and only add the ones back in that don't occur in GroupNames.
			FString GroupName = Actor->Group.ToString();
			GroupName.ParseIntoArray( &GroupArray, TEXT(","), FALSE );

			UBOOL bRemovedFromGroup = FALSE;
			for( INT x = 0 ; x < GroupArray.Num() ; ++x )
			{
				if ( GroupNames.ContainsItem( GroupArray(x) ) )
				{
					GroupArray.Remove( x );
					--x;
					bRemovedFromGroup = TRUE;
				}
			}

			if ( bRemovedFromGroup )
			{
				// Reconstruct a new group name list for the actor.
				GroupName = TEXT("");
				for( INT GroupIndex = 0 ; GroupIndex < GroupArray.Num() ; ++GroupIndex )
				{
					if( GroupName.Len() )
					{
						GroupName += TEXT(",");
					}
					GroupName += GroupArray(GroupIndex);
				}
				Actor->Modify();
				Actor->Group = FName( *GroupName );
				bReturnVal = TRUE;
			}
		}
	}

	return bReturnVal;
}

/**
 * Selects/deselects actors belonging to the named groups.
 *
 * @param	GroupNames	A list of group names.
 * @param	bSelect		If TRUE actors are selected; if FALSE, actors are deselected.
 * @param	Editor		The editor in which to select/deselect the actors.
 * @return				TRUE if at least one actor was selected/deselected.
 */
UBOOL FGroupUtils::SelectActorsInGroups(const TArray<FString>& GroupNames, UBOOL bSelect, UEditorEngine* Editor)
{
	UBOOL bSelectedActor = FALSE;

	if ( GroupNames.Num() > 0 )
	{
		// Iterate over all actors, looking for actors in the specified groups.
		for( FActorIterator It ; It ; ++It )
		{
			AActor* Actor = *It;
			if( IsValid( Actor ) )
			{
				for ( INT GroupIndex = 0 ; GroupIndex < GroupNames.Num() ; ++GroupIndex )
				{
					const FString& CurGroup = GroupNames(GroupIndex);
					if ( Actor->IsInGroup( *CurGroup ) )
					{
						// The actor was found to be in a specified group.
						// Set selection state and move on to the next actor.
						Editor->SelectActor( Actor, bSelect, NULL, FALSE );
						bSelectedActor = TRUE;
						break;
					}
				}
			}
		}
	}

	return bSelectedActor;
}
