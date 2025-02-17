/*=============================================================================
	UTPatchHelper.h: Definitions of classes used for post-ship patching in UT
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * UT specialization for GamePatching
 */
class FUTPatchHelper : public FGamePatchHelper
{
public:
	/**
	 * Fix up the world in code however necessary
	 *
	 * @param World UWorld object for the world to fix
	 */
	virtual void FixupWorld(UWorld* World);
};
