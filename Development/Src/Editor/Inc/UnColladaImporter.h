/*=============================================================================
	COLLADA importer for Unreal Engine 3.
	Based on Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
=============================================================================*/

#ifndef __UNCOLLADAIMPORTER_H__
#define __UNCOLLADAIMPORTER_H__

#include "Factories.h"

// Forward declarations
class FCDAnimated;
class FCDAnimationCurve;
class FCDocument;
class FCDCamera;
class FCDGeometry;
class FCDGeometryMesh;
class FCDGeometryInstance;
class FCDGeometryPolygons;
class FCDMaterial;

class USeqAct_Interp;
class UInterpGroupInst;

namespace UnCollada
{

/**
 * Main COLLADA Importer class.
 * Except for CExporter, consider the other classes as private.
 */
class CImporter
{
public:
	/**
	 * Returns the importer singleton. It will be created on the first request.
	 */
	static CImporter* GetInstance();

	/**
	 * Attempt to import the given COLLADA text string.
	 * Returns TRUE on success. This is the same value returned by 'IsParsingSuccessful'.
	 */
	UBOOL ImportFromText(const FString& Filename, const TCHAR* FileStart, const TCHAR* FileEnd);

	/**
	 * Attempt to import a COLLADA document from a given filename
	 * Returns TRUE on success. This is the same value returned by 'IsParsingSuccessful'.
	 */
	UBOOL ImportFromFile(const TCHAR* Filename);

	/**
	 * Retrieves a list of all the meshes/controllers within the last successfully parsed COLLADA document.
	 * Do not hold onto the returned strings.
	 */
	void RetrieveEntityList(TArray<const TCHAR*>& EntityNameList, UBOOL bSkeletalMeshes=FALSE);

	/**
	 * Returns the state of the COLLADA document parser. This is the same value returned by 'ImportFromText'.
	 */
	UBOOL IsParsingSuccessful() const
	{
		return bParsingSuccessful;
	}

	/**
	 * For the internal skeletal/static importers: returns whether the up-axis should be inverted.
	 * This is necessary to import from Maya or XSI.
	 */
	UBOOL InvertUpAxis() const
	{
		return bInvertUpAxis;
	}

	/**
	 * Retrieve the COLLADA parser's error message explaining its failure to parse a given COLLADA document.
	 * Note that the message should be valid even if the parser is successful and may contain warnings.
	 */
	const TCHAR* GetErrorMessage() const
	{
		return *ErrorMessage;
	}

	/**
	 * Sets the camera creation flag. Call this function before the import in order to enforce
	 * the creation of COLLADA camera instances that do not exist in the current UE3 scene.
	 */
	inline void SetProcessUnknownCameras(UBOOL bCreateMissingCameras)
	{
		bCreateUnknownCameras = bCreateMissingCameras;
	}

	/**
	 * Retrieves whether there are any unknown camera instances within the COLLADA document.
	 */
	UBOOL HasUnknownCameras( USeqAct_Interp* MatineeSequence ) const;

	/**
	 * Creates a static mesh with the given name and flags, imported from within the COLLADA document.
	 * Returns the UStaticMesh object.
	 */
	UObject* ImportStaticMesh(UObject* InParent, const TCHAR* ColladaName, const FName& Name, EObjectFlags Flags);

	/**
	 * Creates a skeletal mesh with the given name and flags, imported from within the COLLADA document.
	 * Merges the given named COLLADA meshes into one skeletal mesh.
	 * Returns the USkeletalMesh object.
	 */
	UObject* ImportSkeletalMesh(UObject* InParent, const TArray<FString>& ColladaNames, const FName& Name, EObjectFlags Flags);

	/**
	 * Add to the animation set, the animations contained within the COLLADA document, for the given skeletal mesh
	 */
	void ImportAnimSet(UAnimSet* AnimSet, USkeletalMesh* Mesh, const TCHAR* AnimSequenceName);

	/**
	 * Add to the morpher set, the morph target contained within the COLLADA document, for the given skeletal mesh
	 */
	void ImportMorphTarget(USkeletalMesh* Mesh, UMorphTargetSet* MorphSet, TArray<FName> MorphTargetNames, TArray<const TCHAR*> ColladaNames, UBOOL bReplaceExisting, EMorphImportError* Error);

	/**
	 * Modifies the Matinee sequence with the animations found in the COLLADA document.
	 */
	void ImportMatineeSequence(USeqAct_Interp* MatineeSequence);

	/**
	 * Imports a COLLADA camera into properties tracks of a Matinee group for a camera actor.
	 */
	void ImportCamera(ACameraActor* Actor, UInterpGroupInst* MatineeGroup, FCDCamera* ColladaCamera);

	/**
	 * Imports a COLLADA animated value into a property track of a Matinee group.
	 */
	void ImportAnimatedProperty(FLOAT* Value, const TCHAR* ValueName, UInterpGroupInst* MatineeGroup, const FLOAT* ColladaValue, FCDAnimated* ColladaAnimated);
	void ImportAnimatedProperty(FLOAT* Value, const TCHAR* ValueName, UInterpGroupInst* MatineeGroup, FCDAnimationCurve* ColladaCurve);

	/**
	 * Retrieve the COLLADA base document object
	 */
	inline FCDocument* GetColladaDocument()
	{
		return ColladaDocument;
	}

	/**
	 * Closes the COLLADA base document, releasing its memory.
	 */
	void CloseDocument();

	/**
	 * Retrieve a COLLADA material, given a COLLADA geometry instance and a set of associated polygons
	 */
	const FCDMaterial* FindColladaMaterial(const FCDGeometryInstance* ColladaInstance, const FCDGeometryPolygons* ColladaPolygons);

	/**
	 * Retrieve an engine material instance, given a COLLADA geometry instance and a set of associated polygons
	 */
	UMaterialInterface* FindMaterialInstance(const FCDGeometryInstance* ColladaInstance, const FCDGeometryPolygons* ColladaPolygons);

private:
	FCDocument*	ColladaDocument;
	UBOOL		bParsingSuccessful;
	UBOOL		bInvertUpAxis;
	UBOOL		bCreateUnknownCameras;
	FString		ErrorMessage;

	CImporter();
	~CImporter();

	/**
	 * Post-import processing. Verifies that the up-axis is the expected Z-axis.
	 */
	void PostImport();

	/**
	 * Creates a Matinee group for a given actor within a given Matinee sequence.
	 */
	UInterpGroupInst* CreateMatineeGroup(USeqAct_Interp* Sequence, AActor* Actor);
};

} // namespace UnCollada

#endif // __UNCOLLADAIMPORTER_H__
