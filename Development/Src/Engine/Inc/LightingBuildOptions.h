/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __LIGHTINGBUILDOPTIONS_H__
#define __LIGHTINGBUILDOPTIONS_H__

/**
 * A set of parameters specifying how static lighting is rebuilt.
 */
class FLightingBuildOptions
{
public:
	FLightingBuildOptions()
	:	bOnlyBuildSelectedActors(FALSE)
	,	bOnlyBuildCurrentLevel(FALSE)
	,	bOnlyBuildChanged(FALSE)
	,	bBuildBSP(TRUE)
	,	bBuildActors(TRUE)
	,	bOnlyBuildSelectedLevels(FALSE)
	,	bPerformFullQualityBuild(TRUE)
	{}

	/**
	 * @return TRUE if the lighting should be built for the level, given the current set of lighting build options.
	 */
	UBOOL ShouldBuildLightingForLevel(ULevel* Level) const;

	/** Whether to only build lighting for selected actors							*/
	UBOOL					bOnlyBuildSelectedActors;
	/** Whether to only build lighting for current level							*/
	UBOOL					bOnlyBuildCurrentLevel;
	/** 
	 * Whether to only build changed lighting. This is only an approximation to speed up iterative level design and levels should
	 * be fully rebuild before checking them back into source control
	 */
	UBOOL					bOnlyBuildChanged;
	/** Whether to build lighting for BSP											*/
	UBOOL					bBuildBSP;
	/** Whether to build lighting for Actors (e.g. static meshes)					*/
	UBOOL					bBuildActors;
	/** Whether to only build lighting for levels selected in the Level Browser.	*/
	UBOOL					bOnlyBuildSelectedLevels;
	/** Whether to use fast or pretty lighting build.								*/
	UBOOL					bPerformFullQualityBuild;
	/** The set of levels selected in the Level Browser.							*/
	TArray<ULevel*>			SelectedLevels;
};

#endif // __LIGHTINGBUILDOPTIONS_H__
