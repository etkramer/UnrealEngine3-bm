/*=============================================================================
	COLLADA skeletal mesh helper.
	Based on top of the Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
==============================================================================*/

#include "EditorPrivate.h"
#include "EngineAnimClasses.h"
#include "UnColladaSkeletalMesh.h"
#include "UnColladaImporter.h"
#include "UnColladaSceneGraph.h"
#include "UnCollada.h"

namespace UnCollada {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static helper functions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Converts a COLLADA 3-vector to an Unreal 3-vector.
 */
static inline FVector ToFVector(const FMVector3& ColladaPoint, UBOOL bInvertUpAxis)
{
	if( !bInvertUpAxis )
	{
		return FVector( ColladaPoint.x, -ColladaPoint.y, ColladaPoint.z );
	}
	else
	{
		return FVector( ColladaPoint.x, ColladaPoint.z, ColladaPoint.y );
	}
}

/**
 * Converts an Unreal 3-vector into a usable form.
 */
static inline FVector ToFVector(const FVector& EnginePoint, UBOOL bInvertUpAxis)
{
	if( !bInvertUpAxis )
	{
		return FVector( EnginePoint.X, -EnginePoint.Y, EnginePoint.Z );
	}
	else
	{
		return FVector( EnginePoint.X, EnginePoint.Z, EnginePoint.Y );
	}
}

/**
 * Converts a COLLADA 4x4 matrix to a Unreal 4x4 matrix.
 */
static inline FMatrix ToFMatrix(const FMMatrix44& ColladaMatrix)
{
	FMatrix EngineMatrix;
	EngineMatrix.SetIdentity();
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			EngineMatrix.M[j][i] = ColladaMatrix.m[j][i];
		}
	}
	return EngineMatrix;
}

/**
 * Converts a Unreal 4x4 matrix transformation to an Unreal quaternion.
 */
static inline FQuat ToFQuat(const FMatrix& EngineMatrix, UBOOL bInvertUpAxis)
{
	FQuat Quaternion( EngineMatrix );
	if( !bInvertUpAxis )
	{
		Quaternion.Y = -Quaternion.Y;
	}
	else
	{
		FLOAT Z = Quaternion.Z;
		Quaternion.Z = Quaternion.Y;
		Quaternion.Y = Z;
	}
	return Quaternion;
}

/**
 * Converts a COLLADA 4x4 matrix transformation to an Unreal quaternion.
 */
static inline FQuat ToFQuat(const FMMatrix44& ColladaTransform, UBOOL bInvertUpAxis)
{
	FMatrix EngineMatrix = ToFMatrix( ColladaTransform );
	return ToFQuat( EngineMatrix, bInvertUpAxis );
}

/**
 * Calculate the world-space transform for a given COLLADA node
 */
static inline FMMatrix44 CalculateWorldSpaceTransform(FCDSceneNode* ColladaNode)
{
	FMMatrix44 Transform = FMMatrix44::Identity;

	// Iteratively process the parent nodes, multiplying by their local transfom.
	while (ColladaNode != NULL)
	{
		Transform = ColladaNode->ToMatrix() * Transform;
		ColladaNode = ColladaNode->GetParent();
	}
	return Transform;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CVertexNormal - Helper structure for smoothing group handling
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CVertexNormal
{
	FMVector3 VertexNormal;
	INT PointIndex;
};
typedef fm::vector<CVertexNormal> CVertexNormalList;
typedef fm::vector<CVertexNormalList> CPerVertexNormalMap;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CSkeletalMesh - COLLADA Skeletal Mesh Importer
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSkeletalMesh::CSkeletalMesh(CImporter* BaseImporter, USkeletalMesh* SkeletalMesh)
{
	this->BaseImporter = BaseImporter;
	this->SkeletalMesh = SkeletalMesh;

	ColladaInstance = NULL;
	ColladaMesh = NULL;
	ColladaSkin = NULL;

	BaseMaterialOffset = 0;
}

CSkeletalMesh::~CSkeletalMesh()
{
	BufferedBones.Empty();
	BufferedRootBones.Empty();
	EnginePoints.Empty();
	EngineWedges.Empty();
	EngineFaces.Empty();
	EngineInfluences.Empty();
	ColladaMaterials.Empty();

	BaseImporter = NULL;
	SkeletalMesh = NULL;
	ColladaInstance = NULL;
	ColladaMesh = NULL;
	ColladaSkin = NULL;
}

INT CSkeletalMesh::FindBufferedBone(FCDSceneNode* ColladaNode)
{
	INT BoneCount = BufferedBones.Num();
	for( INT Index = 0; Index < BoneCount; ++Index )
	{
		if( BufferedBones( Index ).ColladaBone == ColladaNode )
		{
			return Index;
		}
	}
	return INDEX_NONE;
}



/** Searches for a bone in the object hierarchy */
FCDSceneNode* CSkeletalMesh::FindBoneInHierarchy( FCDSkinControllerJoint* ColladaJoint )
{
	// Find the scene node for this joint
	// FIXME COLLADA: Id is probably not correct
	FCDSceneNode* ColladaBone =
		static_cast< FCDSceneNode* >( ColladaNode->FindSubId( ColladaJoint->GetId() ) );
	return ColladaBone;
}



/** Searches for a joint that the current COLLADA skeleton is skinned to that matches the specified scene node */
FCDSkinControllerJoint* CSkeletalMesh::FindSkinnedJointForSceneNode( const FCDSceneNode* Bone )
{
	// FIXME COLLADA: Test more thoroughly
	for( UINT CurJointIndex = 0; CurJointIndex < ColladaSkin->GetJointCount(); ++CurJointIndex )
	{
		FCDSkinControllerJoint* ColladaJoint = ColladaSkin->GetJoint( CurJointIndex );
		
		// Find the scene node for this joint
		const FCDSceneNode* ColladaBone = FindBoneInHierarchy( ColladaJoint );

		if( Bone == ColladaBone )
		{
			return ColladaJoint;
		}
	}

	return NULL;
}



// Buffer a COLLADA bone, with its bind pose, for import.
INT CSkeletalMesh::InsertBufferedBone(INT Index, FCDSceneNode* ColladaBone, FCDSkinControllerJoint* ColladaJoint )
{
	CSkeletalMeshBone Bone;
	Bone.ColladaBone = ColladaBone;
	if( ColladaSkin != NULL )
	{
		if( ColladaJoint != NULL )
		{
			Bone.BindPose = ToFMatrix(ColladaJoint->GetBindPoseInverse().Inverted());
			Bone.bNeedsRealBindPose = FALSE;
		}
		else
		{
			Bone.BindPose = ToFMatrix(CalculateWorldSpaceTransform(ColladaBone));
			Bone.bNeedsRealBindPose = TRUE;
		}
	}
	else
	{
		Bone.BindPose = ToFMatrix(CalculateWorldSpaceTransform(ColladaBone));
		Bone.bNeedsRealBindPose = TRUE;
	}

	return BufferedBones.InsertItem(Bone, Index);
}

/**
 * Imports the first skeletal mesh found inside the COLLADA scene graph into the given object.
 */
UBOOL CSkeletalMesh::ImportSkeletalMesh(const TCHAR* ControllerName)
{
	if( BaseImporter == NULL || SkeletalMesh == NULL )
	{
		return FALSE;
	}

	// Retrieve the scene graph's first controller and its skin/morph objects
	if( !RetrieveMesh(ControllerName) || ColladaNode == NULL )
	{
		return FALSE;
	}

	if( ColladaSkin != NULL )
	{
		// Sony's code hints that the engine supports only 4 influences for each vertex.
		// Remove weights that are below the MINWEIGHT threshold.
		const int MaxWeightsPerVertex = 4;
		const FLOAT MINWEIGHT = 0.01f; // 1%
		ColladaSkin->ReduceInfluences( MaxWeightsPerVertex, MINWEIGHT );
	}

	const UBOOL bResult = ImportGeometry();

	// Note that not all the processing has been done at this point:
	// the skeletal mesh isn't fully imported. You'll need to 
	// call FinishSkeletalMesh() to finish the import.

	return bResult;
}

/**
 * Imports the animations of the first skeletal mesh found inside the COLLADA scene graph.
 */
UBOOL CSkeletalMesh::ImportAnimSet(UAnimSet* AnimSet, const TCHAR* _AnimSequenceName)
{
	if ( BaseImporter == NULL || SkeletalMesh == NULL )
	{
		return FALSE;
	}

	// Until we get clips exported correctly out of 3dsMax, import only one animation per COLLADA file.
	// See if the sequence already exists.
	const FName AnimSequenceName(_AnimSequenceName);
	UAnimSequence* AnimSequence = AnimSet->FindAnimSequence( AnimSequenceName );

	// If not, create new one now.
	if( AnimSequence == NULL )
	{
		AnimSequence = ConstructObject<UAnimSequence>( UAnimSequence::StaticClass(), AnimSet );
		AnimSet->Sequences.AddItem( AnimSequence );
	}
	else
	{
		const UBOOL bDoReplace = appMsgf( AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Prompt_25"), *AnimSequence->GetName()) );
		if( !bDoReplace )
		{
			return FALSE;
		}

		// Recycle anim sequence
		AnimSequence->RecycleAnimSequence();
	}

	AnimSequence->SequenceName = AnimSequenceName;
	if ( !ImportBoneAnimations(AnimSet, AnimSequence) )
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Imports the first morph target of the first skeletal mesh found inside the COLLADA scene graph.
 */
UBOOL CSkeletalMesh::ImportMorphTarget(UMorphTargetSet* MorphSet, UMorphTarget* MorphTarget, const TCHAR* MeshName)
{
	if( MorphSet == NULL || MorphTarget == NULL || SkeletalMesh == NULL || MeshName == NULL || *MeshName == 0 )
	{
		return FALSE;
	}

	// Look for the given mesh name. It might be a controller, but don't count on it.
	if( !RetrieveMesh( MeshName ) || ColladaInstance == NULL || ColladaMesh == NULL || ColladaNode == NULL)
	{
		return FALSE;
	}

	// Create the base mesh stream from the current skeletal mesh.
	USkeletalMesh* BaseSkeletalMesh = SkeletalMesh;
	FMorphMeshRawSource BaseMeshData(BaseSkeletalMesh, 0);

	// Load in the geometry for the morph target, creating a second skeletal mesh
	SkeletalMesh = new(UObject::GetTransientPackage(), *MorphTarget->GetName(), NAME_None) USkeletalMesh();

	// Read in the skeletal mesh, using the wanted COLLADA geometry.
	if ( !ImportGeometry() )
	{
		return FALSE;
	}
	if ( !FinishSkeletalMesh() )
	{
		return FALSE;
	}

	// Create the morph target stream and build the morph target.
	FMorphMeshRawSource TargetMeshData(SkeletalMesh, 0);
	MorphTarget->CreateMorphMeshStreams(BaseMeshData, TargetMeshData, 0);
	return TRUE;
}

/**
 * Retrieves a named mesh instance from within the COLLADA scene graph and its skin/geometry objects
 * Returns TRUE when a valid controller was found.
 */
UBOOL CSkeletalMesh::RetrieveMesh(const TCHAR* ControllerName)
{
	CSceneGraph SceneGraph( BaseImporter->GetColladaDocument() );

	// Retrieve the named geometry instance from the scene graph. That's the controller we'll be importing.
	ColladaInstance = SceneGraph.RetrieveMesh( ControllerName, ColladaMesh, ColladaSkin, ColladaNode );
	return ColladaInstance != NULL && ColladaMesh != NULL;
}

/**
 * Imports a skeletal mesh's bones.  Should be called in the order they are declared.
 */
UBOOL CSkeletalMesh::ImportBones(const TCHAR* ControllerName)
{
	// Problematic areas:
	// 
	// When merging multiple skeletal meshes, all the bones shared must have the same bind pose.
	// 
	// If a bone is considered skipped because it is not skinned to the mesh, but both a parent
	// and a child are skinned to the mesh: it will be forcefully added to the hierarchy during the
	// import of the first skeletal mesh. If a second skeletal mesh uses this bone, the bind pose
	// will most likely be invalid.

	if( BaseImporter == NULL || SkeletalMesh == NULL )
	{
		return FALSE;
	}

	// Retrieve the scene graph's first controller and its skin/morph objects
	if( !RetrieveMesh( ControllerName ) || ColladaNode == NULL )
	{
		return FALSE;
	}

	// No skin implies work to do
	if( ColladaSkin == NULL )
	{
		return TRUE;
	}


	// Generate an ordered tree with the COLLADA document's bones, for this skeletal mesh

	// COLLADA is not very strict, when it comes to joint hierarchies.
	// This engine is very strict, though. So, we'll first need to verify
	// that we have one and only one parent bone and that no intermediary
	// scene node is skipped. There is an added condition that all parent bones
	// occur before their child bones in the list.
	FCDSceneNode* FirstBone = NULL;

	// Order all the COLLADA joints in a tree-like list, in order to generate the indices correctly.
	for( UINT CurJointIndex = 0; CurJointIndex < ColladaSkin->GetJointCount(); ++CurJointIndex )
	{
		FCDSkinControllerJoint* ColladaJoint = ColladaSkin->GetJoint( CurJointIndex );
		
		// Find the scene node for this joint
		FCDSceneNode* ColladaBone = FindBoneInHierarchy( ColladaJoint );

		INT BoneTreeIt = FindBufferedBone( ColladaBone );
		if (BoneTreeIt == INDEX_NONE)
		{
			BoneTreeIt = InsertBufferedBone( BufferedBones.Num(), ColladaBone, ColladaJoint );
		}
		else if ( BufferedBones( BoneTreeIt ).bNeedsRealBindPose )
		{
			CSkeletalMeshBone& Bone = BufferedBones( BoneTreeIt );
			Bone.BindPose = ToFMatrix( ColladaJoint->GetBindPoseInverse().Inverted() );
			Bone.bNeedsRealBindPose = FALSE;
		}
		else
		{
			FMatrix BindPose = ToFMatrix( ColladaJoint->GetBindPoseInverse().Inverted() );
			check( BufferedBones( BoneTreeIt ).BindPose.Equals( BindPose ) );
		}

		// Iterate over the bone's parent nodes, to verify that
		// no scene nodes are skipped and whether this is a parent bone.
		UBOOL IsTop = TRUE;
		FCDBoneList ParentPath;
		for( FCDSceneNode* ParentBone = ColladaBone->GetParent();
			IsTop && ParentBone != NULL;
			ParentBone = ParentBone->GetParent() )
		{
			FCDSkinControllerJoint* ParentJoint = FindSkinnedJointForSceneNode( ParentBone );
			if( FindBufferedBone( ParentBone ) != INDEX_NONE || ParentJoint != NULL )
			{
				IsTop = FALSE;
				UINT ParentPathLength = ParentPath.Num();
				if( ParentPathLength != 0 )
				{
					// Unfortunately, there were skipped bones. They'll need to be
					// added to the hierarchy, with their animations and no influence.
					// Check that we don't add skipped bones more than once.
					for( UINT ParentPathIndex = 0; ParentPathIndex < ParentPathLength; ++ParentPathIndex )
					{
						FCDSceneNode* SkippedBone = ParentPath( ParentPathIndex );
						INT SkippedBoneIt = FindBufferedBone( SkippedBone );
						FCDSkinControllerJoint* SkippedJoint = FindSkinnedJointForSceneNode( SkippedBone );
						if( SkippedBoneIt == INDEX_NONE )
						{
							InsertBufferedBone( BoneTreeIt, SkippedBone, SkippedJoint );
						}
						else if( SkippedBoneIt > BoneTreeIt )
						{
							BufferedBones.Remove( SkippedBoneIt );
							BoneTreeIt = InsertBufferedBone( BoneTreeIt, SkippedBone, SkippedJoint );
						}
					}
				}

				// If this bone's parent is new to the list, insert it and its parents before the current bone
				UINT ParentBoneTreeIt = FindBufferedBone( ParentBone );
				while( ParentBoneTreeIt == INDEX_NONE )
				{
					BoneTreeIt = InsertBufferedBone( BoneTreeIt, ParentBone, ParentJoint );

					// Check the parent's parent
					ParentBone = ParentBone->GetParent();
					if( ParentBone == NULL || FindSkinnedJointForSceneNode( ParentBone ) == NULL ) break;
					ParentBoneTreeIt = FindBufferedBone( ParentBone );
				}
			}
			else
			{
				ParentPath.AddItem( ParentBone );
			}
		}

		if( IsTop )
		{
			// Add to the list of bones believed to be parent bones.
			BufferedRootBones.AddItem( ColladaBone );
		}
	}

	return TRUE;
}

/**
 * Imports the skeleton's animated elements into the sequence of an animation set.
 */
UBOOL CSkeletalMesh::ImportBoneAnimations(UAnimSet* AnimSet, UAnimSequence* AnimSequence)
{
	AnimSequence->SequenceLength = 0.0f;
	FCDocument* ColladaDocument = BaseImporter->GetColladaDocument();
	FCDBoneList BoneMap;
	TArray<INT> TrackMap;

	// Fill in the COLLADA->engine index map, in order to load the animations correctly
	// Use name-matching between the engine bones and the COLLADA scene graph elements
	UINT EngineBoneCount = SkeletalMesh->RefSkeleton.Num();
	BoneMap.Reserve(EngineBoneCount);
	for( UINT EngineBoneIt = 0; EngineBoneIt < EngineBoneCount; ++EngineBoneIt )
	{
		fm::string BoneId = FUStringConversion::ToString( *( SkeletalMesh->RefSkeleton(EngineBoneIt).Name.ToString() ) );
		FCDSceneNode* ColladaBone = ColladaDocument->FindSceneNode( BoneId.c_str() );
		BoneMap.AddItem( ColladaBone ); // May be NULL
	}

	// Ensure that we have the right number of tracks, then match them with the correct names to make the map of tracks
	TrackMap.Reserve(EngineBoneCount);
	if( AnimSet->TrackBoneNames.Num() == 0 )
	{
		AnimSet->TrackBoneNames.Reserve( EngineBoneCount );
		for( UINT EngineBoneIt = 0; EngineBoneIt < EngineBoneCount; ++EngineBoneIt )
		{
			AnimSet->TrackBoneNames.AddItem( SkeletalMesh->RefSkeleton( EngineBoneIt ).Name );
			TrackMap.AddItem( EngineBoneIt );
		}
	}
	else if (EngineBoneCount == AnimSet->TrackBoneNames.Num())
	{
		// You cannot add/remove tracks, so the bone names must be directly tied to the tracks.
		for (UINT TrackIt = 0; TrackIt < EngineBoneCount; ++TrackIt)
		{
			const FName& TrackName = AnimSet->TrackBoneNames(TrackIt);
			for (UINT EngineBoneIt = 0; EngineBoneIt < EngineBoneCount; ++EngineBoneIt)
			{
				if (SkeletalMesh->RefSkeleton(EngineBoneIt).Name == TrackName)
				{
					TrackMap.AddItem(EngineBoneIt);
					break;
				}
			}
		}
		if (TrackMap.Num() != EngineBoneCount) return FALSE;
	}
	else
	{
		return FALSE;
	}

	UINT TrackCount = EngineBoneCount;
	AnimSequence->RawAnimData.AddZeroed(TrackCount);

	// We'll need to merge in the animated object key times, and evaluate at those times on the scene graph elements
	// in order to succesfully retrieve the animation key values (quat + point).
	for (UINT TrackIndex = 0; TrackIndex < TrackCount; ++TrackIndex)
	{
		UINT EngineBoneIndex = TrackMap(TrackIndex);
		FCDSceneNode* Bone = BoneMap(EngineBoneIndex);

		// Sample the COLLADA bone animation
		FloatList SampleKeys; 
		FMMatrix44List SampleValues;
		if (Bone != NULL)
		{
			FCDSceneNodeTools::GenerateSampledAnimation( Bone );
			SampleKeys = FCDSceneNodeTools::GetSampledAnimationKeys();
			SampleValues = FCDSceneNodeTools::GetSampledAnimationMatrices();
			if (SampleKeys.empty() || SampleValues.empty())
			{
				// There is no animation: Grab the pose
				SampleKeys.clear();
				SampleValues.clear();
				SampleKeys.push_back(0.0f);
				SampleValues.push_back(Bone->ToMatrix());
			}
			else
			{
				// Max out the sequence length
				if (AnimSequence->SequenceLength < SampleKeys.back()) AnimSequence->SequenceLength = SampleKeys.back();
			}
		}

		// Fill in the track information for the sequence
		FRawAnimSequenceTrack& RawTrack = AnimSequence->RawAnimData(TrackIndex);
		if (!SampleKeys.empty())
		{
			UINT KeyCount = (UINT) SampleKeys.size();
			if (KeyCount > (UINT) SampleValues.size()) KeyCount = (UINT) SampleValues.size();
			RawTrack.KeyTimes.Reserve(KeyCount);
			RawTrack.PosKeys.Reserve(KeyCount);
			RawTrack.RotKeys.Reserve(KeyCount);
			for (UINT KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
			{
				// It seems that "KeyTimes" holds the time lengths, not the time at which to trigger the key
				if (KeyCount >= 2)
				{
					if (KeyIndex < KeyCount - 1) RawTrack.KeyTimes.AddItem(SampleKeys[KeyIndex + 1] - SampleKeys[KeyIndex]);
					else RawTrack.KeyTimes.AddItem(SampleKeys[KeyIndex] - SampleKeys[KeyIndex - 1]);
				}
				else
				{
					RawTrack.KeyTimes.AddItem(0.0f);
				}

				FMMatrix44& SampleValue = SampleValues[KeyIndex];
				RawTrack.PosKeys.AddItem( ToFVector( SampleValue.GetTranslation(), BaseImporter->InvertUpAxis() ) );
				RawTrack.RotKeys.AddItem( ToFQuat( SampleValue, BaseImporter->InvertUpAxis() ) );

				// [GL_TODO] There seems to be some confusion as to which bone orientation(s) need to be W-inverted.
				if (TrackIndex == 0) RawTrack.RotKeys(KeyIndex).W = -RawTrack.RotKeys(KeyIndex).W;
			}
		}
		else
		{
			// Grab the pose from the engine bone
			RawTrack.KeyTimes.AddItem( 0.0f );
			RawTrack.PosKeys.AddItem( SkeletalMesh->RefSkeleton( EngineBoneIndex ).BonePos.Position );
			RawTrack.RotKeys.AddItem( SkeletalMesh->RefSkeleton( EngineBoneIndex ).BonePos.Orientation );
			if (TrackIndex != 0)
			{
				RawTrack.RotKeys(0).W = -RawTrack.RotKeys(0).W;
			}
		}
	}

	// [From UEditorEngine::ImportPSAIntoAnimSet]

	// Remove unnecessary frames.
	UAnimationCompressionAlgorithm_RemoveTrivialKeys* KeyReducer = ConstructObject<UAnimationCompressionAlgorithm_RemoveTrivialKeys>(UAnimationCompressionAlgorithm_RemoveTrivialKeys::StaticClass());
	KeyReducer->Reduce( AnimSequence, NULL, FALSE );

	// Flush the LinkupCache.
	AnimSet->LinkupCache.Empty();

	// We need to re-init any skeletal mesh components now, because they might still have references to linkups in this set.
	for( TObjectIterator<USkeletalMeshComponent> It; It != NULL; ++It )
	{
		USkeletalMeshComponent* SkeletonComponent = *It;
		if( !SkeletonComponent->IsPendingKill() )
		{
			SkeletonComponent->InitAnimTree();
		}
	}

	return TRUE;
}

/**
 * After all the bones are imported, verify that the order of all the bones forms a valid hierarchy.
 */
void CSkeletalMesh::ArrangeBoneHierarchy()
{
	check( BufferedBones.Num() > 0 );

	// We have buffered parent bones, then we need to process them.
	FCDSceneNode* ActualRootBone = NULL;
	for( INT BoneIt = 0; BoneIt < BufferedRootBones.Num(); ++BoneIt )
	{
		FCDSceneNode* PossibleRootBone = BufferedRootBones( BoneIt );
		INT BoneIndex = FindBufferedBone( PossibleRootBone );
		if( BoneIndex != INDEX_NONE )
		{
			// Verify that they are still, after all the merging: root bones.
			FCDSceneNode* ParentRootBone = PossibleRootBone->GetParent();
			INT ParentIndex = FindBufferedBone( ParentRootBone );
			if( ParentIndex == INDEX_NONE )
			{
				if( ActualRootBone == NULL )
				{
					// A first valid root bone was found.
					ActualRootBone = PossibleRootBone;
				}
				else
				{
					// If there is more than one root bones, then add a NULL bone at the top.
					CSkeletalMeshBone EmptyRootBone;
					EmptyRootBone.ColladaBone = NULL;
					EmptyRootBone.BindPose.SetIdentity();
					BufferedBones.InsertItem(EmptyRootBone, 0);
					ActualRootBone = NULL;
					break;
				}
			}
		}
	}

	// Now that we have the root(s) settled: make sure that all the child bones occur after their parent.
	for( INT BoneIt = 0; BoneIt < BufferedBones.Num(); )
	{
		CSkeletalMeshBone& Bone = BufferedBones( BoneIt );
		if (Bone.ColladaBone != NULL)
		{
			INT ParentBoneIndex = FindBufferedBone( Bone.ColladaBone->GetParent() );
			if( ParentBoneIndex < BoneIt )
			{
				// This is a valid root bone or the parent bone occurs before the bone: that's valid
				++BoneIt;
			}
			else
			{
				// The parent bone occurs after the child bone.
				// So, move the child bone after the parent bone
				CSkeletalMeshBone BoneCopy = Bone;
				BufferedBones.Remove( BoneIt );
				BufferedBones.InsertItem( BoneCopy, ParentBoneIndex );
			}
		}
		else
		{
			// We have a valid NULL root bone
			++BoneIt;
		}
	}

	// Verify that the root is first
	check( ActualRootBone == BufferedBones(0).ColladaBone );

	BufferedRootBones.Empty();
}

/**
 * Imports a skeletal mesh's geometry.
 */
UBOOL CSkeletalMesh::ImportGeometry()
{
    if( BufferedRootBones.Num() > 0 )
	{
		// This is done once, after all the bones are loaded from the different skeletal meshes to merge.
		ArrangeBoneHierarchy();
	}

	// The COLLADA bind-shape matrix is the transform at which the geometry was bound.
	// As such, it needs to be taken out of the vertex positions.
	FMMatrix44 BindShapeMatrix;
	if( ColladaSkin != NULL )
	{
		BindShapeMatrix = ColladaSkin->GetBindShapeTransform();
	}
	else
	{
		// Use the world-space transform [rotation and scale only]
		// from the instancing node as the bind-shape transform
		BindShapeMatrix = CalculateWorldSpaceTransform( ColladaNode );
	}

	// The engine's point list should be a direct import of the geometry's vertex position source.
	const FCDGeometrySource* ColladaPositionData = ColladaMesh->GetPositionSource();
	if( ColladaPositionData->GetStride() < 3 ) return FALSE;
	UINT PointCount = ColladaPositionData->GetSourceData().size() / ColladaPositionData->GetStride();
	INT EnginePointOffset = EnginePoints.Num();
	EnginePoints.Reserve(EnginePointOffset + PointCount * 6 / 5); // Reserve 20% more space for duplication related to hard edges.
	EnginePoints.Add(PointCount);
	for( UINT PointIndex = 0; PointIndex < PointCount; ++PointIndex )
	{
		FMVector3 BindPosition( &( ColladaPositionData->GetSourceData().front() ), PointIndex * ColladaPositionData->GetStride() );
		BindPosition = BindShapeMatrix.TransformCoordinate( BindPosition );
		EnginePoints( EnginePointOffset + PointIndex ) = ToFVector( BindPosition, BaseImporter->InvertUpAxis() );
	}

	// Prepare a map for the per-vertex indexation of the normals
	// This will allow us to add extra vertices when we have hard edges.
	CPerVertexNormalMap VertexNormalMap;
	VertexNormalMap.resize( PointCount );

	// Import the COLLADA mesh tessellation information into the format wanted by the engine.
	// The engine supports only one, un-indexed, texture coordinate on each vertex.
	UINT ColladaPolygonsCount = ColladaMesh->GetPolygonsCount();
	for( UINT PolygonsIndex = 0; PolygonsIndex < ColladaPolygonsCount; ++PolygonsIndex )
	{
		const FCDGeometryPolygons* ColladaPolygons = ColladaMesh->GetPolygons(PolygonsIndex);

		// Retrieve the material index for this list of polygons.
		// Add the material to the buffered list, if it is missing.
		const FCDMaterial* ColladaMaterial = BaseImporter->FindColladaMaterial( ColladaInstance, ColladaPolygons );
		INT MaterialIndex = ColladaMaterials.FindItemIndex( ColladaMaterial );
		if( MaterialIndex == INDEX_NONE )
		{
			// Insert this material in the list
			MaterialIndex = ColladaMaterials.Num();
			ColladaMaterials.AddItem( ColladaMaterial );
		}

		// Retrieve the necessary vertex position indices
		const FCDGeometryPolygonsInput* PositionInput = ColladaPolygons->FindInput(FUDaeGeometryInput::POSITION);
		if( PositionInput == NULL || PositionInput->GetSource() == NULL ) return FALSE;
		const uint32* PositionIndices = PositionInput->GetIndices();
		if( PositionIndices == NULL ) return FALSE;

		// Retrieve the optional vertex normal indices
		// As opposed to COLLADA itself, if there are no vertex normals, the UE3 importer assumes that the mesh is smooth.
		const uint32* NormalIndices = NULL;
		const FCDGeometrySource* NormalSource = NULL;
		const FCDGeometryPolygonsInput* NormalInput = ColladaPolygons->FindInput(FUDaeGeometryInput::NORMAL);
		if( NormalInput != NULL )
		{
			NormalSource = NormalInput->GetSource();
			if (NormalSource != NULL && NormalSource->GetStride() >= 3)
			{
				NormalIndices = NormalInput->GetIndices();
			}
		}

		// Retrieve the optional texture coordinate indices
		const uint32* TexcoordIndices = NULL;
		const FCDGeometrySource* TexcoordSource = NULL;
		const FCDGeometryPolygonsInput* TexcoordInput = ColladaPolygons->FindInput(FUDaeGeometryInput::TEXCOORD);
		if( TexcoordInput != NULL )
		{
			TexcoordSource = TexcoordInput->GetSource();
			if (TexcoordSource != NULL && TexcoordSource->GetStride() >= 2)
			{
				TexcoordIndices = TexcoordInput->GetIndices();
			}
		}
		UBOOL HasTexcoord = TexcoordIndices != NULL;

		// Count the number of triangles that we will be creating, in order to pre-cache the correct size
		UINT TotalTriangleCount = 0;
		for( UINT CurVertCountIndex = 0; CurVertCountIndex < ColladaPolygons->GetFaceVertexCountCount(); ++CurVertCountIndex )
		{
			TotalTriangleCount += ColladaPolygons->GetFaceVertexCount( CurVertCountIndex );
		}

		// Process the COLLADA tessellation information to create the wedges and faces
		UINT WedgeOffset = EngineWedges.Num();
		UINT TotalWedgeCount = ColladaPolygons->GetFaceVertexCount();
		EngineWedges.Add( TotalWedgeCount );
		UINT TriangleOffset = EngineFaces.Num();
		EngineFaces.Add( TotalTriangleCount );
		UINT FaceVertexOffset = 0;
		for( UINT CurVertCountIndex = 0; CurVertCountIndex < ColladaPolygons->GetFaceVertexCountCount(); ++CurVertCountIndex )
		{
			// Create and fill in the wedges first: one per original face-vertex.
			// It should be possible to compress this later.
			UINT WedgeCount = ColladaPolygons->GetFaceVertexCount( CurVertCountIndex );
			for( UINT WedgeIndex = 0; WedgeIndex < WedgeCount; ++WedgeIndex )
			{
				FMeshWedge& EngineWedge = EngineWedges( WedgeOffset + FaceVertexOffset + WedgeIndex );
				UINT PointIndex = PositionIndices[ FaceVertexOffset + WedgeIndex ];

				// Retrieve the normal for this wedge and verify smoothness
				if( NormalIndices != NULL )
				{
					UINT NormalIndex = NormalIndices[ FaceVertexOffset + WedgeIndex ];
					FMVector3 VertexNormal( &NormalSource->GetSourceData()[ NormalIndex * NormalSource->GetStride() ] );
					
					// Look for this normal within the buffer
					CVertexNormalList& VertexNormalList = VertexNormalMap[ PointIndex ];
					UINT VertexNormalCount = (UINT) VertexNormalList.size(), VertexNormalIndex;
					for( VertexNormalIndex = 0; VertexNormalIndex < VertexNormalCount; ++VertexNormalIndex )
					{
						CVertexNormal& N = VertexNormalList[ VertexNormalIndex ];
						if( IsEquivalent( N.VertexNormal, VertexNormal ) )
						{
							PointIndex = N.PointIndex;
							break;
						}
					}

					if( VertexNormalIndex == VertexNormalCount )
					{
						// New normal for this vertex.
						CVertexNormal N;
						N.VertexNormal = VertexNormal;
						if( VertexNormalIndex > 0 )
						{
							// Duplicate the vertex
							FVector EnginePoint = EnginePoints( EnginePointOffset + PointIndex );
							EnginePoints.AddItem( EnginePoint );
							PointIndex = EnginePoints.Num() - EnginePointOffset - 1;
						}
						N.PointIndex = PointIndex;

						VertexNormalList.push_back( N );
					}
				}

				EngineWedge.iVertex = (WORD) ( EnginePointOffset + PointIndex );
			}

			// Create the faces, triangulating along the way using a simple faning method
			UINT TriangleCount = WedgeCount - 2;
			for( UINT TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex )
			{
				FMeshFace& EngineFace = EngineFaces( TriangleOffset + TriangleIndex );
				EngineFace.iWedge[0] = WedgeOffset + FaceVertexOffset;
				EngineFace.iWedge[1] = WedgeOffset + FaceVertexOffset + TriangleIndex + 1;
				EngineFace.iWedge[2] = WedgeOffset + FaceVertexOffset + TriangleIndex + 2;
				EngineFace.MeshMaterialIndex = MaterialIndex;
			}

			TriangleOffset += TriangleCount;
			FaceVertexOffset += WedgeCount;
		}

		if( HasTexcoord )
		{
			// Give the wedges their texture coordinates
			for( UINT WedgeIndex = 0; WedgeIndex < TotalWedgeCount; ++WedgeIndex )
			{
				FMeshWedge& EngineWedge = EngineWedges( WedgeOffset + WedgeIndex );
				UINT TexcoordIndex = TexcoordIndices[ WedgeIndex ];
				EngineWedge.U = TexcoordSource->GetSourceData()[ TexcoordIndex * TexcoordSource->GetStride() ];
				EngineWedge.V = 1.0f - TexcoordSource->GetSourceData()[ TexcoordIndex * TexcoordSource->GetStride() + 1 ];
			}
		}
		else
		{
			// Reset the wedges texture coordinates
			for( UINT WedgeIndex = WedgeOffset; WedgeIndex < WedgeOffset + TotalWedgeCount; ++WedgeIndex )
			{
				FMeshWedge& EngineWedge = EngineWedges( WedgeIndex );
				EngineWedge.U = EngineWedge.V = 0.0f;
			}
		}
	}

	INT EngineInfluencesOffset = EngineInfluences.Num();
	if( ColladaSkin != NULL )
	{
		// As a search would otherwise be needed for each influence point:
		// pre-buffer the joint index-bone index map
		TArray<WORD> SkinJointIndexMap;
		SkinJointIndexMap.Add( ColladaSkin->GetJointCount() );
		for( UINT JointIt = 0; JointIt < ColladaSkin->GetJointCount(); ++JointIt )
		{
			// Find the scene node for this joint
			FCDSceneNode* ColladaBone = FindBoneInHierarchy( ColladaSkin->GetJoint( JointIt ) );

			INT BoneIndex = FindBufferedBone( ColladaBone );
			SkinJointIndexMap(JointIt) = (BoneIndex >= 0) ? (WORD) BoneIndex : 0;
		}

		// Count the total number of influences that are present on the skin controller
		UINT MeshInfluenceCount = 0;
		for( UINT CurVertexInfIndex = 0; CurVertexInfIndex < ColladaSkin->GetInfluenceCount(); ++CurVertexInfIndex )
		{
			MeshInfluenceCount += ColladaSkin->GetVertexInfluence( CurVertexInfIndex )->GetPairCount();
		}

		// Finally: import the skin and bone influences from the COLLADA skin controller.
		EngineInfluences.Reserve( EngineInfluencesOffset + MeshInfluenceCount * 6 / 5 ); // add 20% to account for some vertex duplication.
		UINT VertexCount = (UINT) ColladaSkin->GetInfluenceCount();
		for( UINT VertexIndex = 0; VertexIndex < VertexCount; ++VertexIndex )
		{
			// As vertices may have been duplicated, check the vertex normal map
			CVertexNormalList& VertexNormals = VertexNormalMap[ VertexIndex ];
			UINT VertexNormalCount = VertexNormals.size();

			const FCDSkinControllerVertex* CurInfVertex = ColladaSkin->GetVertexInfluence( VertexIndex );
			for( UINT CurPairIndex = 0; CurPairIndex < CurInfVertex->GetPairCount(); ++CurPairIndex )
			{
				const FCDJointWeightPair* CurPair = CurInfVertex->GetPair( CurPairIndex );
				if( VertexNormalCount != 0 )
				{
					// Since the vertex was duplicated, for smoothing: duplicate the influences as well.
					for( UINT VertexNormalIndex = 0; VertexNormalIndex < VertexNormalCount; ++VertexNormalIndex )
					{
						CVertexNormal& N = VertexNormals[ VertexNormalIndex ];
						EngineInfluences.Add( 1 );
						FVertInfluence& Influence = EngineInfluences.Last();
						Influence.BoneIndex = SkinJointIndexMap( CurPair->jointIndex );
						Influence.VertIndex = EnginePointOffset + N.PointIndex;
						Influence.Weight = CurPair->weight;
					}
				}
				else
				{
					// No normals were provided or the vertex is unused: make sure that the UE3 always has an influence.
					EngineInfluences.Add( 1 );
					FVertInfluence& Influence = EngineInfluences.Last();
					Influence.BoneIndex = SkinJointIndexMap( CurPair->jointIndex );
					Influence.VertIndex = EnginePointOffset + VertexIndex;
					Influence.Weight = CurPair->weight;
				}
			}
		}
	}
	else
	{
		// Use the top bone on all the vertices
		UINT VertexCount = EnginePoints.Num() - EnginePointOffset;
		EngineInfluences.Add( VertexCount );
		for( UINT VertexIndex = 0; VertexIndex < VertexCount; ++VertexIndex)
		{
			FVertInfluence& Influence = EngineInfluences( EngineInfluencesOffset + VertexIndex );
			Influence.BoneIndex = 0;
			Influence.VertIndex = EnginePointOffset + VertexIndex;
			Influence.Weight = 1.0f;
		}
	}

	return TRUE;
}

UBOOL CSkeletalMesh::FinishSkeletalMesh()
{
	UBOOL bCompleted = FinishMaterials();
	bCompleted |= FinishBones();
	bCompleted |= FinishGeometry();
	return bCompleted;
}

// Complete the import of the buffered materials.
UBOOL CSkeletalMesh::FinishMaterials()
{
	INT MaterialCount = ColladaMaterials.Num();
	SkeletalMesh->Materials.Add( MaterialCount );
	for( INT MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex )
	{
		const FCDMaterial* Material = ColladaMaterials( MaterialIndex );
		if ( Material != NULL )
		{
			SkeletalMesh->Materials( MaterialIndex ) = FindObject<UMaterialInterface>( ANY_PACKAGE, Material->GetName().c_str() );
		}
		else
		{
			SkeletalMesh->Materials( MaterialIndex ) = NULL;
		}
	}

	return TRUE;
}

// Complete the import of the bones
UBOOL CSkeletalMesh::FinishBones()
{
	// Buffer in the new bones
	UINT BoneCount = BufferedBones.Num();

	if( BoneCount == 0 )
	{
		// There should always be one bone in the skeletal mesh.
		// Having no bones at this point implies that the user is importing/merging only static meshes.
		SkeletalMesh->RefSkeleton.Add( 1 );
		FMeshBone& Bone = SkeletalMesh->RefSkeleton( 0 );
		Bone.Name = TEXT( "Main" );
		Bone.ParentIndex = 0;
		Bone.Depth = 0;
		Bone.NumChildren = 0;
		Bone.Flags = 0;
		Bone.BonePos.Position = FVector( 0, 0, 0 );
		Bone.BonePos.Orientation = FQuat( 0, 0, 0, 1 );
		Bone.BoneColor = FColor(255,255,255);
	}
	else
	{
		// Set-up the bone hierarchy and position within the engine, following the generated bone tree
		SkeletalMesh->RefSkeleton.Add( BoneCount );
		for( UINT BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex )
		{
			FMeshBone& EngineBone = SkeletalMesh->RefSkeleton( BoneIndex );
			CSkeletalMeshBone& BufferedBone = BufferedBones( BoneIndex );

			// Set some default values
			EngineBone.ParentIndex = 0;
			EngineBone.Depth = 0;
			EngineBone.NumChildren = 0;
			EngineBone.Flags = 0;
			EngineBone.BoneColor = FColor(255,255,255);

			FCDSceneNode* ColladaBone = BufferedBone.ColladaBone;
			if( ColladaBone != NULL )
			{
				fstring BoneName = FUStringConversion::ToFString( ColladaBone->GetDaeId() );
				EngineBone.Name = BoneName.c_str();

				// Find the parent's index
				FCDSceneNode* ColladaParent = ColladaBone->GetParent();
				CSkeletalMeshBone* ParentBone = NULL;
				if( BoneIndex != 0 && ColladaParent != NULL )
				{
					INT ParentBoneIt = FindBufferedBone(ColladaParent);
					if( ParentBoneIt != INDEX_NONE )
					{
						EngineBone.ParentIndex = ParentBoneIt;
						ParentBone = &BufferedBones( ParentBoneIt );
					}
				}

				// Calculate the relative, initial bone transform
				FMatrix InitialBoneTransform = BufferedBone.BindPose;
				if( ParentBone != NULL )
				{
					InitialBoneTransform *= ParentBone->BindPose.Inverse();
				}

				EngineBone.BonePos.Position = ToFVector( InitialBoneTransform.GetOrigin(), BaseImporter->InvertUpAxis() );
				EngineBone.BonePos.Orientation = ToFQuat( InitialBoneTransform, BaseImporter->InvertUpAxis() );

				// [GL_TODO] There seems to be some confusion as to which bone orientation(s) need to be W-inverted.
				EngineBone.BonePos.Orientation.W = -EngineBone.BonePos.Orientation.W;
			}
			else
			{
				EngineBone.Name = TEXT("Reparented");
				EngineBone.BonePos.Orientation = FQuat(0, 0, 0, 1);
				EngineBone.BonePos.Position = FVector(0, 0, 0);
			}
		}
	}

	// Traverse the engine bone hierarchy directly, to calculate the number of children and the depth.
	// Note that all child bones are always positioned after their parents: the parent is always fully initialized.
	SkeletalMesh->SkeletalDepth = 0;
	for( UINT BoneIndex = 1; BoneIndex < BoneCount; ++BoneIndex )
	{
		FMeshBone& EngineBone = SkeletalMesh->RefSkeleton( BoneIndex );
		FMeshBone& ParentBone = SkeletalMesh->RefSkeleton( EngineBone.ParentIndex );
		++(ParentBone.NumChildren);
		EngineBone.Depth = ParentBone.Depth + 1;
		if( SkeletalMesh->SkeletalDepth < EngineBone.Depth )
		{
			SkeletalMesh->SkeletalDepth = EngineBone.Depth;
		}
	}

	// Now RefSkeleton is complete, initialise bone name -> bone index map
	SkeletalMesh->InitNameIndexMap();

	return TRUE;
}

// Complete the import of the mesh's geometry
UBOOL CSkeletalMesh::FinishGeometry()
{
	// In the engine, create/retrieve the first LOD mesh using the vertex streams
	// The rest of this function is adapted from USkeletalMeshFactory::FactoryCreateBinary
	SkeletalMesh->LODModels.Empty();
	new( SkeletalMesh->LODModels ) FStaticLODModel();

	SkeletalMesh->LODInfo.Empty();
	new( SkeletalMesh->LODInfo ) FSkeletalMeshLODInfo();
	SkeletalMesh->LODInfo(0).LODHysteresis = 0.02f;

	// Create the initial bounding box based on expanded version of reference pose
	// for meshes without physics assets. Can be overridden by artist.
	// Tuck up the bottom as this rarely extends lower than a reference pose's
	// (e.g. having its feet on the floor).
	FBox BoundingBox( &EnginePoints(0), EnginePoints.Num() );
	FBox Temp = BoundingBox;
	FVector MidMesh = 0.5f * ( Temp.Min + Temp.Max );
	BoundingBox.Min = Temp.Min + 1.0f * ( Temp.Min - MidMesh );
	BoundingBox.Max = Temp.Max + 1.0f * ( Temp.Max - MidMesh );
	BoundingBox.Min.Z = Temp.Min.Z + 0.1f * ( Temp.Min.Z - MidMesh.Z );
	SkeletalMesh->Bounds = FBoxSphereBounds( BoundingBox );

	// Create actual rendering data.
	SkeletalMesh->CreateSkinningStreams( EngineInfluences, EngineWedges, EngineFaces, EnginePoints );

	// Presize the per-section shadow casting array with the number of sections in the imported LOD.
	const INT NumSections = SkeletalMesh->LODModels(0).Sections.Num();
	SkeletalMesh->LODInfo(0).bEnableShadowCasting.Empty( NumSections );
	for ( INT SectionIndex = 0 ; SectionIndex < NumSections ; ++SectionIndex )
	{
		SkeletalMesh->LODInfo(0).bEnableShadowCasting.AddItem( TRUE );
	}

	// RequiredBones for base model includes all bones.
	FStaticLODModel& LODModel = SkeletalMesh->LODModels( 0 );
	INT RequiredBoneCount = SkeletalMesh->RefSkeleton.Num();
	LODModel.RequiredBones.Add( RequiredBoneCount );
	for( INT i=0; i < RequiredBoneCount; ++i )
	{
		LODModel.RequiredBones(i) = i;
	}

	// Make GUID.
	SkeletalMesh->SkelMeshGUID = appCreateGuid();

	// End the importing, mark package as dirty so it prompts to save on exit.
	SkeletalMesh->PreEditChange(NULL);
	SkeletalMesh->PostEditChange(NULL);
	SkeletalMesh->CalculateInvRefMatrices();

	SkeletalMesh->MarkPackageDirty();
	return TRUE;
}

} // namespace UnCollada
