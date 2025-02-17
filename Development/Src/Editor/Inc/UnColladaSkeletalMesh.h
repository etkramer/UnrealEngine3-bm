/*=============================================================================
	COLLADA skeletal mesh helper.
	Based on Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
=============================================================================*/

#ifndef __UNCOLLADASKELETALMESH_H__
#define __UNCOLLADASKELETALMESH_H__

// Forward declarations
class FCDBoneList;
class FCDGeometry;
class FCDGeometryInstance;
class FCDGeometryMesh;
class FCDMaterial;
class FCDMorphController;
class FCDSceneNode;
class FCDSkinController;
class FCDSkinControllerJoint;

namespace UnCollada
{

// Forward declarations
class CImporter;

/**
 * Temporary structure to hold the COLLADA scene node equivalent to each engine bone.
 */
class CSkeletalMeshBone
{
public:
	FCDSceneNode* ColladaBone;
	FMatrix BindPose;
	UBOOL bNeedsRealBindPose;
};
typedef TArray<CSkeletalMeshBone> CSkeletalMeshBoneList;
typedef TArray<FCDSceneNode*> CSkeletalMeshBoneRootList;

/**
 * COLLADA skeletal mesh importer.
 */
class CSkeletalMesh
{
public:
	CSkeletalMesh(CImporter* BaseImporter, USkeletalMesh* SkeletalMesh);
	~CSkeletalMesh();

	/**
	 * Imports a full skeletal mesh found inside the COLLADA scene graph into the given object.
	 * To support skeletal mesh merging, import all the bones first to get the correct hierarchy.
	 */
	UBOOL ImportBones(const TCHAR* ControllerName);
	UBOOL ImportSkeletalMesh(const TCHAR* ControllerName);
	UBOOL FinishSkeletalMesh();

	/**
	 * Imports the animations of the first skeletal mesh found inside the COLLADA scene graph.
	 */
	UBOOL ImportAnimSet(UAnimSet* AnimSet, const TCHAR* AnimSequenceName);

	/**
	 * Imports the morph targets of the first skeletal mesh found inside the COLLADA scene graph.
	 */
	UBOOL ImportMorphTarget(UMorphTargetSet* MorphSet, UMorphTarget* MorphTarget, const TCHAR* MeshName);

private:
	CImporter*					BaseImporter;
	USkeletalMesh*				SkeletalMesh;

	/** Holds all the COLLADA information for the bones to match with the engine's. */
	CSkeletalMeshBoneList		BufferedBones;
	CSkeletalMeshBoneRootList	BufferedRootBones;

	/** COLLADA document objects that are used for the import. */
	FCDGeometryInstance*		ColladaInstance;
	FCDGeometryMesh*			ColladaMesh;
	FCDSkinController*			ColladaSkin;
	FCDSceneNode*				ColladaNode;

	/** Hold the base offset for the material of the current mesh import: used in mesh merging */
	INT							BaseMaterialOffset;

	/** Skeletal mesh data buffers, used for the import: buffered for mesh-merging. */
	TArray<FVector>				EnginePoints;
	TArray<FMeshWedge>			EngineWedges;
	TArray<FMeshFace>			EngineFaces;
	TArray<FVertInfluence>		EngineInfluences;

	typedef TArray<const FCDMaterial*> MaterialList;
	typedef TArray<FCDSceneNode*>	FCDBoneList;

	/** Skeletal mesh buffered material list. */
	MaterialList				ColladaMaterials;

	/**
	 * Imports a skeletal mesh's geometry.
	 */
	UBOOL ImportGeometry();

	/** Complete the import of the buffered bones */
	UBOOL FinishBones();

	/** Complete the import of the mesh's buffered geometry. */
	UBOOL FinishGeometry();

	/** Complete the import of the buffered materials. */
	UBOOL FinishMaterials();

	/**
	 * Search within the buffered bones for the given COLLADA bone.
	 * Returns the index to this bone within the BufferedBone array.
	 */
	INT FindBufferedBone(FCDSceneNode* ColladaBone);

	/** Buffer a COLLADA bone, with its bind pose, for import. */
	INT InsertBufferedBone(INT Index, FCDSceneNode* ColladaBone, FCDSkinControllerJoint* ColladaJoint);

	/** Searches for a joint that the current COLLADA skeleton is skinned to that matches the specified scene node */
	FCDSkinControllerJoint* FindSkinnedJointForSceneNode( const FCDSceneNode* Bone );

	/** Searches for a bone in the object hierarchy */
	FCDSceneNode* FindBoneInHierarchy( FCDSkinControllerJoint* ColladaJoint );


	/**
	 * After all the bones are imported, verify that the order of all the bones forms a valid hierarchy.
	 */
	void ArrangeBoneHierarchy();

	/**
	 * Imports the skeleton's animated elements into the sequence of an animation set.
	 */
	UBOOL ImportBoneAnimations(UAnimSet* AnimSet, UAnimSequence* AnimSequence);

	/**
	 * Retrieve the first controller within the COLLADA scene graph and its skin/geometry objects
	 * Returns TRUE when a valid controller was found.
	 */
	UBOOL RetrieveMesh(const TCHAR* ControllerName);
};

} // namespace UnCollada

#endif // __UNCOLLADASKELETALMESH_H__
