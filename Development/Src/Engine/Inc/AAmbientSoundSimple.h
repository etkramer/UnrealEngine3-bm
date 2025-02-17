/*=============================================================================
	AmbientSoundSimple.h: Native AmbientSoundSimple calls
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

protected:
	/**
	 * Helper function used to sync up instantiated objects.
	 */
	void SyncUpInstantiatedObjects( void );

	/**
	 * Called from within SpawnActor, calling SyncUpInstantiatedObjects.
	 */
	virtual void Spawned( void );

	/**
	 * Called when after .t3d import of this actor (paste, duplicate or .t3d import),
	 * calling SyncUpInstantiatedObjects.
	 */
	virtual void PostEditImport( void );

	/**
	 * Used to temporarily clear references for duplication.
	 *
	 * @param PropertyThatWillChange	property that will change
	 */
	virtual void PreEditChange( UProperty* PropertyThatWillChange );

	/**
	 * Used to reset audio component when AmbientProperties change
	 *
	 * @param PropertyThatChanged	property that changed
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );

	virtual void EditorApplyScale( const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown );
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors( void );

