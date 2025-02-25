/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This is the particle system label renderer
 */
class ParticleSystemLabelRenderer extends ThumbnailLabelRenderer
	native;

cpptext
{
protected:
	/**
	 * Adds the name of the object and other information
	 *
	 * @param Object the object to build the labels for
	 * @param OutLabels the array that is added to
	 */
	void BuildLabelList(UObject* Object, const ThumbnailOptions& InOptions, TArray<FString>& OutLabels);

}
