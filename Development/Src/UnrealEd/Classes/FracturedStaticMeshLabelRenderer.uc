/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This is the fractured static mesh label renderer
 */
class FracturedStaticMeshLabelRenderer extends StaticMeshLabelRenderer
	native;

cpptext
{
protected:
	/**
	 * Adds the name of the object and information about the mesh
	 *
	 * @param Object the object to build the labels for
	 * @param OutLabels the array that is added to
	 */
	void BuildLabelList(UObject* Object, const ThumbnailOptions& InOptions, TArray<FString>& OutLabels);
}
