/*=============================================================================
	AmbientSound.h: Native AmbientSound calls
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

protected:
	// AActor interface.
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors( void );

	/**
	 * Starts audio playback if wanted.
	 */
	virtual void UpdateComponentsInternal( UBOOL bCollisionUpdate = FALSE );

