//=============================================================================
// Simplified version of ambient sound used to enhance workflow.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class AmbientSoundSimple extends AmbientSound
	hidecategories(Audio)
	native(Sound);

/** Mirrored property for easier editability, set in Spawned.		*/
var()	editinline editconst	SoundNodeAmbient	AmbientProperties;
/** Dummy sound cue property to force instantiation of subobject.	*/
var		editinline export const SoundCue			SoundCueInstance;
/** Dummy sound node property to force instantiation of subobject.	*/
var		editinline export const SoundNodeAmbient	SoundNodeInstance;

cpptext
{
	/**
	 * Helper function used to sync up instantiated objects.
	 */
	void SyncUpInstantiatedObjects();

	/**
	 * Called from within SpawnActor, calling SyncUpInstantiatedObjects.
	 */
	virtual void Spawned();

	/**
	 * Called when after .t3d import of this actor (paste, duplicate or .t3d import),
	 * calling SyncUpInstantiatedObjects.
	 */
	virtual void PostEditImport();

	/**
	 * Used to temporarily clear references for duplication.
	 *
	 * @param PropertyThatWillChange	property that will change
	 */
	virtual void PreEditChange(UProperty* PropertyThatWillChange);

	/**
	 * Used to reset audio component when AmbientProperties change
	 *
	 * @param PropertyThatChanged	property that changed
	 */
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();
}

defaultproperties
{
	Begin Object Name=DrawSoundRadius0
		SphereColor=(R=173,G=239,B=231,A=255)
	End Object

	Begin Object Class=SoundNodeAmbient Name=SoundNodeAmbient0
	End Object
	SoundNodeInstance=SoundNodeAmbient0

	Begin Object Class=SoundCue Name=SoundCue0
	End Object
	SoundCueInstance=SoundCue0
}
