/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearEditorEngine extends EditorEngine
	native;


cpptext
{
	/** This allows the indiv game to return the set of decal materials it uses based off its internal objects (e.g. PhysicalMaterials)**/
	virtual class TArray<UMaterialInterface*> GetDecalMaterialsFromGame( UObject* InObject ) const;
};



DefaultProperties
{

}