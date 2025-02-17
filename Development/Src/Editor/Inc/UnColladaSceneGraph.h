/*=============================================================================
	COLLADA scene graph helper.
	Based on Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
=============================================================================*/

#ifndef __UNCOLLADASCENEGRAPH_H__
#define __UNCOLLADASCENEGRAPH_H__

// Forward declarations
class FCDocument;
class FCDAnimated;
class FCDAnimationMultiCurve;
class FCDConversionFunctor;
class FCDGeometry;
class FCDGeometryInstance;
class FCDGeometryMesh;
class FCDSceneNode;
class FCDSkinController;
class FMMatrix44;

class UInterpGroupInst;

namespace UnCollada
{

/**
 * Scene Graph helper class. Used to traverse the scene graph and find relevant information.
 */
class CSceneGraph
{
public:
	CSceneGraph(FCDocument* ColladaDocument);

	/**
	 * Retrieve a list of the instantiated meshes/controllers within the visual scene graph.
	 */
	void RetrieveEntityList(TArray<const TCHAR*>& EntityNameList, UBOOL bSkeletalMeshes);

	/**
	 * Retrieve the mesh with the given name and its instantiation information.
	 * Set MeshName to NULL to grab the first available mesh.
	 */
	FCDGeometryInstance* RetrieveMesh(const TCHAR* MeshName, FCDGeometryMesh*& ColladaMesh, FCDSkinController*& ColladaSkin, FCDSceneNode*& ColladaNode, FMMatrix44* WorldTransform = NULL);

	/**
	 * Retrieve the list of all the bones/joints within the visual scene graph. Is used by the import of bone animations.
	 */
	void RetrieveBoneList(TArray<FCDSceneNode*> BoneList);

	/**
	 * Imports a COLLADA scene node into a Matinee actor group.
	 */
	FLOAT ImportMatineeActor(FCDSceneNode* ColladaNode, UInterpGroupInst* MatineeGroup);

	/**
	 * Imports a COLLADA animated element into a Matinee track.
	 */
	void ImportMatineeAnimated(FCDAnimationMultiCurve* ColladaCurve, INT ColladaDimension, FInterpCurveVector& Curve, INT CurveIndex, FCDConversionFunctor* Conversion = NULL);

private:
	FCDocument*		ColladaDocument;
	FCDSceneNode*	ColladaSceneRoot;
};

} // namespace UnCollada

#endif // __UNCOLLADASCENEGRAPH_H__
