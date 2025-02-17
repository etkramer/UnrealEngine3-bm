/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GROUPUTILS_H__
#define __GROUPUTILS_H__

// Forward declarations.
class AActor;
class FString;
class UEditorEngine;

/**
 * A set of static functions for common operations on Actor groups.  All functions return
 * a UBOOL indicating whether something changed (an actor was added to or removed form a group,
 * an actor was selected/deselected, etc).  Returning FALSE does not necessarily mean that
 * an operation failed, but rather nothing changed.
 */
class FGroupUtils
{
public:
	/**
	 * Adds the actor to the named group.
	 *
	 * @return		TRUE if the actor was added.  FALSE is returned if the
	 *              actor already belongs to the group.
	 */
	static UBOOL AddActorToGroup(AActor* Actor, const FString& GroupName, UBOOL bModifyActor);

	/**
	 * Removes an actor from the specified group.
	 *
	 * @return		TRUE if the actor was removed from the group.  FALSE is returned if the
	 *              actor already belonged to the group.
	 */
	static UBOOL RemoveActorFromGroup(AActor* Actor, const FString& GroupToRemove, UBOOL bModifyActor);

	/////////////////////////////////////////////////
	// Operations on selected actors.

	/**
	 * Adds selected actors to the named group.
	 *
	 * @param	GroupName	A group name.
	 * @return				TRUE if at least one actor was added.  FALSE is returned if all selected
	 *                      actors already belong to the named group.
	 */
	static UBOOL AddSelectedActorsToGroup(const FString& GroupName);

	/**
	 * Adds selected actors to the named groups.
	 *
	 * @param	GroupNames	A valid list of group names.
	 * @return				TRUE if at least one actor was added.  FALSE is returned if all selected
	 *                      actors already belong to the named groups.
	 */
	static UBOOL AddSelectedActorsToGroups(const TArray<FString>& GroupNames);

	/**
	 * Removes selected actors from the named groups.
	 *
	 * @param	GroupNames	A valid list of group names.
	 * @return				TRUE if at least one actor was removed.
	 */
	static UBOOL RemoveSelectedActorsFromGroups(const TArray<FString>& GroupNames);

	/**
	 * Selects/deselects actors belonging to the named groups.
	 *
	 * @param	GroupNames	A valid list of group names.
	 * @param	bSelect		If TRUE actors are selected; if FALSE, actors are deselected.
	 * @param	Editor		The editor in which to select/deselect the actors.
	 * @return				TRUE if at least one actor was selected/deselected.
	 */
	static UBOOL SelectActorsInGroups(const TArray<FString>& GroupNames, UBOOL bSelect, UEditorEngine* Editor);
};

#endif // __GROUPUTILS_H__
