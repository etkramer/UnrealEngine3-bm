/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MorphTargetSet extends Object
	native(Anim);
	
cpptext
{
	/** 
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc();

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	INT GetResourceSize();
}

/** Array of pointers to MorphTarget objects, containing vertex deformation information. */ 
var	array<MorphTarget>		Targets;

/** SkeletalMesh that this MorphTargetSet works on. */
var	SkeletalMesh			BaseSkelMesh;

/** Find a morph target by name in this MorphTargetSet. */ 
native final function MorphTarget FindMorphTarget( Name MorphTargetName );
