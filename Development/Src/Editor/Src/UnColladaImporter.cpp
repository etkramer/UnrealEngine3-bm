/*=============================================================================
	COLLADA importer for Unreal Engine 3.
	Based on top of the Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
==============================================================================*/

#include "EditorPrivate.h"
#include "EngineAnimClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineSequenceClasses.h"
#include "UnLinkedObjDrawUtils.h"
#include "UnColladaImporter.h"
#include "UnColladaSceneGraph.h"
#include "UnColladaSkeletalMesh.h"
#include "UnColladaStaticMesh.h"
#include "UnCollada.h"

namespace UnCollada {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CImporter - Main COLLADA CImporter Class
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CImporter::CImporter()
	:	ColladaDocument( NULL )
	,	bParsingSuccessful( FALSE )
	,	bInvertUpAxis( FALSE )
	,	bCreateUnknownCameras( FALSE )
{
	// Initialize COLLADA.  It's fine for this to be called more than once, as long as Release is also called.
	FCollada::Initialize();
}

CImporter::~CImporter()
{
	delete ColladaDocument;

	// Clean up COLLADA (ref counted)
	FCollada::Release();
}

/**
 * Returns the importer singleton. It will be created on the first request.
 */
CImporter* CImporter::GetInstance()
{
	static CImporter* ImporterInstance = new CImporter();
	return ImporterInstance;
}

/**
 * Attempt to import the given COLLADA text string.
 * Returns TRUE on success. This is the same value returned by 'IsParsingSuccessful'.
 */
UBOOL CImporter::ImportFromText(const FString& Filename, const TCHAR* FileStart, const TCHAR* FileEnd)
{
	delete ColladaDocument;
	ColladaDocument = FCollada::NewTopDocument();

	// Strip the filename of its name, in order to get the absolute file path to base the COLLADA document on.
	FUFileManager ColladaFileManager;
	fstring FilePath = ColladaFileManager.StripFileFromPath(*Filename);
	fstring FileText = fstring(FileStart, FileEnd - FileStart);

	FUErrorSimpleHandler ErrorHandler;

	bParsingSuccessful =
		FCollada::LoadDocumentFromMemory(
			*Filename, ColladaDocument,
			( void* )FileText.c_str(),
			FileText.length() * sizeof( TCHAR ) );

	// Retrieve the error message and buffer it for the engine
	ErrorMessage = ErrorHandler.GetErrorString();
	if( bParsingSuccessful )
	{
		PostImport();
	}
	return bParsingSuccessful;
}

/**
 * Attempt to import a COLLADA document from a given filename
 * Returns TRUE on success. This is the same value returned by 'IsParsingSuccessful'.
 */
UBOOL CImporter::ImportFromFile(const TCHAR* Filename)
{
	// Read in the COLLADA document
	delete ColladaDocument;
	ColladaDocument = FCollada::NewTopDocument();

	FUErrorSimpleHandler ErrorHandler;

	bParsingSuccessful = FCollada::LoadDocument( ColladaDocument, Filename );

	// Retrieve the error message and buffer it for the engine
	ErrorMessage = ErrorHandler.GetErrorString();
	if( bParsingSuccessful )
	{
		PostImport();
	}
	return bParsingSuccessful;
}

/**
 * Post-import processing. Verifies that the up-axis is the expected Z-axis.
 */
void CImporter::PostImport()
{
	if ( bParsingSuccessful && ColladaDocument != NULL )
	{
		bInvertUpAxis = !IsEquivalent(ColladaDocument->GetAsset()->GetUpAxis(), FMVector3::ZAxis);
	}
}

/**
 * Retrieves whether there are any unknown camera instances within the COLLADA document.
 */

// Repeated part from function below.
inline UBOOL _HasUnknownCameras( USeqAct_Interp* MatineeSequence, FCDSceneNode* ColladaSceneNode, const TCHAR* Name )
{
	size_t ColladaInstanceCount = ColladaSceneNode->GetInstanceCount();
	for ( size_t InstanceIndex = 0; InstanceIndex < ColladaInstanceCount; ++InstanceIndex )
	{
		FCDEntityInstance* ColladaInstance = ColladaSceneNode->GetInstance( InstanceIndex );
		if ( ColladaInstance->GetEntityType() == FCDEntity::CAMERA )
		{

			// If we have a Matinee, try to name-match the node with a Matinee group name
			if( MatineeSequence != NULL && MatineeSequence->InterpData != NULL )
			{
				const UInterpGroupInst* GroupInst = MatineeSequence->FindFirstGroupInstByName( FString( Name ) );
				if( GroupInst != NULL )
				{
					// Make sure we have an actor
					if( GroupInst->GroupActor != NULL &&
						GroupInst->GroupActor->IsA( ACameraActor::StaticClass() ) )
					{
						// OK, we found an existing camera!
						return TRUE;
					}
				}
			}


			// Attempt to name-match the scene node for this camera with one of the actors.
			AActor* Actor = FindObject<AActor>( ANY_PACKAGE, Name );
			if ( Actor == NULL || Actor->bDeleteMe )
			{
				return TRUE;
			}
			else
			{
				// If you trigger this assertion, then you've got a name
				// clash between the COLLADA file and the UE3 level.
				check( Actor->IsA( ACameraActor::StaticClass() ) );
			}
			break;
		}
	}
	return FALSE;
}

UBOOL CImporter::HasUnknownCameras( USeqAct_Interp* MatineeSequence ) const
{
	if ( !bParsingSuccessful || ColladaDocument == NULL )
	{
		return FALSE;
	}

	FCDSceneNode* ColladaSceneRoot = ColladaDocument->GetVisualSceneRoot();
	size_t ColladaSceneNodeCount = ColladaSceneRoot->GetChildrenCount();
	for ( size_t NodeIndex = 0; NodeIndex < ColladaSceneNodeCount; ++NodeIndex )
	{
		FCDSceneNode* ColladaSceneNode = ColladaSceneRoot->GetChild(NodeIndex);
		if ( _HasUnknownCameras( MatineeSequence, ColladaSceneNode, ColladaSceneNode->GetName().c_str() ) )
		{
			return TRUE;
		}

		// Also check for instances on pivot nodes.
		size_t ColladaChildCount = ColladaSceneNode->GetChildrenCount();
		for ( size_t ChildIndex = 0; ChildIndex < ColladaChildCount; ++ChildIndex )
		{
			FCDSceneNode* ColladaChildNode = ColladaSceneNode->GetChild(ChildIndex);
			if ( _HasUnknownCameras( MatineeSequence, ColladaChildNode, ColladaSceneNode->GetName().c_str() ) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/**
 * Retrieves a list of all the meshes/controllers within the last successfully parsed COLLADA document.
 * Do not hold onto the returned strings.
 */
void CImporter::RetrieveEntityList(TArray<const TCHAR*>& EntityNameList, UBOOL bSkeletalMeshes)
{
	if ( bParsingSuccessful && ColladaDocument != NULL && ColladaDocument->GetVisualSceneRoot() != NULL )
	{
		CSceneGraph SceneGraph(ColladaDocument);
		SceneGraph.RetrieveEntityList(EntityNameList, bSkeletalMeshes);
	}
}

/**
 * Creates a static mesh with the given name and flags, imported from within the COLLADA document.
 * Returns the UStaticMesh object.
 */
UObject* CImporter::ImportStaticMesh(UObject* InParent, const TCHAR* ColladaName, const FName& Name, EObjectFlags Flags)
{
	if ( !bParsingSuccessful || ColladaDocument == NULL )
	{
		return NULL;
	}

	// Retrieve the wanted geometry instance from the scene graph.
	CSceneGraph SceneGraph(ColladaDocument);
	FCDGeometryMesh* ColladaMesh = NULL;
	FCDSkinController* ColladaSkin = NULL; // Intentionally unused
	FCDSceneNode* ColladaNode = NULL; // Intentionally unused
	FCDGeometryInstance* Instance = SceneGraph.RetrieveMesh( ColladaName, ColladaMesh, ColladaSkin, ColladaNode );
	if ( Instance == NULL || ColladaMesh == NULL )
	{
		return NULL;
	}

	// Create a UStaticMesh object to contain this COLLADA mesh
	UStaticMesh* Mesh = new(InParent, Name, Flags | RF_Public) UStaticMesh();
	FStaticMeshRenderData* RenderMesh = NULL;
	if ( Mesh->LODModels.Num() == 0 )
	{
		// Add one LOD for the base mesh
		RenderMesh = new(Mesh->LODModels) FStaticMeshRenderData();
		Mesh->LODInfo.AddZeroed();
	}
	else
	{
		RenderMesh = &Mesh->LODModels(0);
	}

	CStaticMesh StaticMesh(this);
	StaticMesh.ImportStaticMeshPolygons(RenderMesh, Instance, ColladaMesh);
	Mesh->Build();
	return Mesh;
}

/**
 * Retrieve a COLLADA material, given a COLLADA geometry instance and a set of associated polygons
 */
const FCDMaterial* CImporter::FindColladaMaterial(const FCDGeometryInstance* ColladaInstance, const FCDGeometryPolygons* ColladaPolygons)
{
	const FCDMaterial* ColladaMaterial = NULL;

	if( ColladaInstance != NULL && ColladaPolygons != NULL )
	{
		const fstring& MaterialSemantic	= ColladaPolygons->GetMaterialSemantic();
		const FCDMaterialInstance* ColladaMaterialInstance = ColladaInstance->FindMaterialInstance( MaterialSemantic );
		if( ColladaMaterialInstance != NULL )
		{
			ColladaMaterial = ColladaMaterialInstance->GetMaterial();
		}
	}
	return ColladaMaterial;
}

/**
 * Retrieve an engine material instance, given a COLLADA geometry instance and a set of associated polygons
 */
UMaterialInterface* CImporter::FindMaterialInstance(const FCDGeometryInstance* ColladaInstance, const FCDGeometryPolygons* ColladaPolygons)
{
	UMaterialInterface* MaterialInterface = NULL;
	const FCDMaterial* ColladaMaterial = FindColladaMaterial( ColladaInstance, ColladaPolygons );
	if ( ColladaMaterial != NULL )
	{
		MaterialInterface = FindObject<UMaterialInterface>( ANY_PACKAGE, ColladaMaterial->GetName().c_str() );
	}

	return MaterialInterface;
}

/**
 * Creates a skeletal mesh with the given name and flags, imported from within the COLLADA document.
 * Returns the USkeletalMesh object.
 */
UObject* CImporter::ImportSkeletalMesh(UObject* InParent, const TArray<FString>& ColladaNames, const FName& Name, EObjectFlags Flags)
{
	if ( !bParsingSuccessful || ColladaDocument == NULL || ColladaNames.Num() < 1 )
	{
		return NULL;
	}
	
	// Create a USkeletalMesh object to contain this COLLADA controlled mesh
	USkeletalMesh* Mesh = new(InParent, Name, Flags | RF_Public) USkeletalMesh();

	// Verify that if there is some skeletal mesh merging requested, then they all
	// have the identity transform. Otherwise, this import cannot succeed.
	CSceneGraph SceneGraph( ColladaDocument );
	INT ColladaMeshCount = ColladaNames.Num();
	if( ColladaMeshCount > 1 )
	{
		FMMatrix44 WorldTransform;
		for( INT MeshIndex = 0; MeshIndex < ColladaMeshCount; ++MeshIndex )
		{
			FCDGeometryMesh* DummyMesh;
			FCDSkinController* DummySkin;
			FCDSceneNode* DummyNode;
			FMMatrix44 DummyTransform;
			SceneGraph.RetrieveMesh( *ColladaNames(MeshIndex), DummyMesh, DummySkin, DummyNode, &DummyTransform );
			if( DummySkin != NULL )
			{
				// If this assert triggers, you are attempting to merge into one skeletal mesh
				// some static/skeletal meshes which are not compatible.
				check( IsEquivalent( DummyTransform, FMMatrix44::Identity ) );
			}
		}
	}

	CSkeletalMesh MeshImporter( this, Mesh );
	for( INT MeshIndex = 0; MeshIndex < ColladaMeshCount; ++MeshIndex )
	{
		// Attempt to import a controller into this object
		if ( !MeshImporter.ImportBones( *ColladaNames( MeshIndex ) ) )
		{
			return NULL;
		}
	}

	for( INT MeshIndex = 0; MeshIndex < ColladaMeshCount; ++MeshIndex )
	{
		// Attempt to import a controller into this object
		if ( !MeshImporter.ImportSkeletalMesh( *ColladaNames( MeshIndex ) ) )
		{
			return NULL;
		}
	}

	if ( !MeshImporter.FinishSkeletalMesh() )
	{
		return NULL;
	}
	else
	{
		return Mesh;
	}
}

/**
 * Add to the animation set, the animations contained within the COLLADA document, for the given skeletal mesh
 */
void CImporter::ImportAnimSet(UAnimSet* AnimSet, USkeletalMesh* Mesh, const TCHAR* AnimSequenceName)
{
	if (!bParsingSuccessful || ColladaDocument == NULL)
	{
		return;
	}

	AnimSet->bAnimRotationOnly = FALSE;

	// Use the skeletal mesh importer class to import skeletal animations
	CSkeletalMesh MeshImporter(this, Mesh);
	MeshImporter.ImportAnimSet(AnimSet, AnimSequenceName);
}

#define SET_COLLADA_ERROR(ErrorType) { if (Error != NULL) *Error = ErrorType; }

/**
 * Add to the morpher set, the morph target contained within the COLLADA document, for the given skeletal mesh
 */
void CImporter::ImportMorphTarget(USkeletalMesh* Mesh, UMorphTargetSet* MorphSet, TArray<FName> MorphTargetNames, TArray<const TCHAR*> ColladaNames, UBOOL bReplaceExisting, EMorphImportError* Error)
{
	// Verify the basic status of the importer
	if (Mesh == NULL || MorphSet == NULL || MorphTargetNames.Num() < 1 || MorphTargetNames.Num() != ColladaNames.Num())
	{
		SET_COLLADA_ERROR(MorphImport_CantLoadFile);
		return;
	}
	if (!bParsingSuccessful || ColladaDocument == NULL)
	{
		SET_COLLADA_ERROR(MorphImport_CantLoadFile);
		return;
	}
	GWarn->BeginSlowTask(TEXT("Generating Morph Model"), TRUE);

	// Create/Replace the morph targets
	INT MorphTargetCount = MorphTargetNames.Num();
	for (INT TargetIndex = 0; TargetIndex < MorphTargetCount; ++TargetIndex)
	{
		const FName& MorphTargetName = MorphTargetNames( TargetIndex );
		const TCHAR* ColladaName = ColladaNames( TargetIndex );

		UMorphTarget* MorphTarget = MorphSet->FindMorphTarget( MorphTargetName );
		if (MorphTarget != NULL && !bReplaceExisting)
		{
			SET_COLLADA_ERROR( MorphImport_AlreadyExists );
		}
		else
		{
			if (MorphTarget == NULL)
			{
				// Create the new morph target mesh
				MorphTarget = ConstructObject<UMorphTarget>( UMorphTarget::StaticClass(), MorphSet, MorphTargetName );
			}

			// Use the skeletal mesh importer class to import morph targets
			CSkeletalMesh MeshImporter( this, Mesh );
			if ( !MeshImporter.ImportMorphTarget( MorphSet, MorphTarget, ColladaName ) )
			{
				SET_COLLADA_ERROR( MorphImport_InvalidMeshFormat );
				MorphTarget = NULL;
			}
			else
			{
				SET_COLLADA_ERROR( MorphImport_OK );
				MorphSet->Targets.AddItem( MorphTarget );
			}
		}
	}

	GWarn->EndSlowTask();
}

inline FCDCamera* FindInstancedCamera(FCDSceneNode* ColladaSceneNode)
{
	// Look for a camera in the instances of the scene node.
	FCDCamera* ColladaCamera = NULL;
	size_t ColladaInstanceCount = ColladaSceneNode->GetInstanceCount();
	for (size_t ColladaInstanceIndex = 0; ColladaInstanceIndex < ColladaInstanceCount && ColladaCamera == NULL; ++ColladaInstanceIndex)
	{
		FCDEntityInstance* ColladaInstance = ColladaSceneNode->GetInstance(ColladaInstanceIndex);
		if (ColladaInstance->GetEntity() != NULL && ColladaInstance->GetEntity()->GetObjectType().Includes(FCDCamera::GetClassType()))
		{
			return (FCDCamera*) ColladaInstance->GetEntity();
		}
	}

	// Look for a camera in the possible pivot child nodes.
	size_t ColladaChildrenCount = ColladaSceneNode->GetChildrenCount();
	for (size_t ColladaChildIndex = 0; ColladaChildIndex < ColladaChildrenCount && ColladaCamera == NULL; ++ColladaChildIndex)
	{
		FCDSceneNode* ColladaChildNode = ColladaSceneNode->GetChild(ColladaChildIndex);
		size_t ColladaInstanceCount = ColladaChildNode->GetInstanceCount();
		for (size_t ColladaInstanceIndex = 0; ColladaInstanceIndex < ColladaInstanceCount && ColladaCamera == NULL; ++ColladaInstanceIndex)
		{
			FCDEntityInstance* ColladaInstance = ColladaChildNode->GetInstance(ColladaInstanceIndex);
			if (ColladaInstance->GetEntity() != NULL && ColladaInstance->GetEntity()->GetObjectType().Includes(FCDCamera::GetClassType()))
			{
				return (FCDCamera*) ColladaInstance->GetEntity();
			}
		}
	}

	return NULL;
}

void CImporter::ImportMatineeSequence(USeqAct_Interp* MatineeSequence)
{
	if (ColladaDocument == NULL || MatineeSequence == NULL) return;
	CSceneGraph ColladaSceneGraph(ColladaDocument);

	// If the Matinee editor is not open, we need to initialize the sequence.
	UBOOL InitializeMatinee = MatineeSequence->InterpData == NULL;
	if (InitializeMatinee)
	{
		// Force the initialization of the sequence
		// This sets the sequence in edition mode as well?
		MatineeSequence->InitInterp();
	}

	UInterpData* MatineeData = MatineeSequence->InterpData;
	FLOAT InterpLength = -1.0f;

	FCDSceneNode* ColladaSceneRoot = ColladaDocument->GetVisualSceneRoot();
	size_t ColladaSceneNodeCount = ColladaSceneRoot->GetChildrenCount();
	for (size_t NodeIndex = 0; NodeIndex < ColladaSceneNodeCount; ++NodeIndex)
	{
		FCDSceneNode* ColladaSceneNode = ColladaSceneRoot->GetChild(NodeIndex);

		AActor* Actor = NULL;

		// First check to see if the scene node name matches a Matinee group name
		UInterpGroupInst* FoundGroupInst =
			MatineeSequence->FindFirstGroupInstByName( FString( ColladaSceneNode->GetName().c_str() ) );
		if( FoundGroupInst != NULL )
		{
			// OK, we found an actor bound to a Matinee group that matches this scene node name
			Actor = FoundGroupInst->GroupActor;
		}


		// Attempt to name-match the scene node with one of the actors.
		if( Actor == NULL )
		{
			Actor = FindObject<AActor>( ANY_PACKAGE, ColladaSceneNode->GetName().c_str() );
		}


		if ( Actor == NULL || Actor->bDeleteMe )
		{
			if ( bCreateUnknownCameras && FindInstancedCamera(ColladaSceneNode) != NULL )
			{
				Actor = GWorld->SpawnActor( ACameraActor::StaticClass(), ColladaSceneNode->GetName().c_str() );
			}
			else
			{
				continue;
			}
		}

		UInterpGroupInst* MatineeGroup = MatineeSequence->FindGroupInst(Actor);

		// Before attempting to create/import a movement track: verify that the node is animated.
		UBOOL IsAnimated = FALSE;
		size_t ColladaTransformCount = ColladaSceneNode->GetTransformCount();
		for (size_t ColladaTransformIndex = 0; ColladaTransformIndex < ColladaTransformCount; ++ColladaTransformIndex)
		{
			IsAnimated |= (UBOOL) ColladaSceneNode->GetTransform(ColladaTransformIndex)->IsAnimated();
		}

		if (IsAnimated)
		{
			if (MatineeGroup == NULL)
			{
				MatineeGroup = CreateMatineeGroup(MatineeSequence, Actor);
			}

			FLOAT TimeLength = ColladaSceneGraph.ImportMatineeActor(ColladaSceneNode, MatineeGroup);
			InterpLength = max(InterpLength, TimeLength);
		}

		// Right now, cameras are the only supported import entity type.
		if (Actor->IsA(ACameraActor::StaticClass()))
		{
			FCDCamera* ColladaCamera = FindInstancedCamera(ColladaSceneNode);

			if (ColladaCamera != NULL)
			{
				if (MatineeGroup == NULL)
				{
					MatineeGroup = CreateMatineeGroup(MatineeSequence, Actor);
				}

				ImportCamera((ACameraActor*) Actor, MatineeGroup, ColladaCamera);
			}
		}

		if (MatineeGroup != NULL)
		{
			MatineeGroup->Modify();
		}
	}

	MatineeData->InterpLength = (InterpLength < 0.0f) ? 5.0f : InterpLength;
	MatineeSequence->Modify();

	if (InitializeMatinee)
	{
		MatineeSequence->TermInterp();
	}
}

void CImporter::ImportCamera(ACameraActor* Actor, UInterpGroupInst* MatineeGroup, FCDCamera* ColladaCamera)
{
	// FIXME COLLADA: Does this even work?

	// Import the default, animatable camera properties
	ImportAnimatedProperty(&Actor->FOVAngle, TEXT("FOVAngle"), MatineeGroup, &( *ColladaCamera->GetFovX() ),
		ColladaCamera->GetFovX().IsAnimated() ? ColladaCamera->GetFovX().GetAnimated() : NULL );
	ImportAnimatedProperty(&Actor->AspectRatio, TEXT("AspectRatio"), MatineeGroup, &( *ColladaCamera->GetAspectRatio() ),
		ColladaCamera->GetAspectRatio().IsAnimated() ? ColladaCamera->GetAspectRatio().GetAnimated() : NULL );
	
	// Look for a depth-of-field or motion blur description in the COLLADA extra information structure.
	FCDENode* DepthOfFieldNode = ColladaCamera->GetExtra()->GetDefaultType()->FindRootNode(DAEFC_CAMERA_DEPTH_OF_FIELD_ELEMENT);
	FCDENode* MotionBlurNode = ColladaCamera->GetExtra()->GetDefaultType()->FindRootNode(DAEMAX_CAMERA_MOTIONBLUR_ELEMENT);
	if (DepthOfFieldNode != NULL)
	{
		// Import the animatable depth-of-field parameters
		FCDENode* ParameterNode = DepthOfFieldNode->FindParameter(DAEFC_CAMERA_DOF_FOCALDEPTH_PARAMETER);
		if (ParameterNode != NULL && ParameterNode->GetAnimated() != NULL)
		{
			ImportAnimatedProperty(&Actor->CamOverridePostProcess.DOF_FocusDistance, TEXT("CamOverridePostProcess.DOF_FocusDistance"), MatineeGroup,
				&ParameterNode->GetAnimated()->GetDummy(), ParameterNode->GetAnimated() );
		}

		ParameterNode = DepthOfFieldNode->FindParameter(DAEFC_CAMERA_DOF_SAMPLERADIUS_PARAMETER);
		// TODO: This one needs a conversion with the 'focus distance' to get the 'focus radius'.
	}
	else if (MotionBlurNode != NULL)
	{
		FCDENode* ParameterNode = MotionBlurNode->FindParameter(DAEMAX_CAMERA_MB_DURATION_PARAMETER);
		if (ParameterNode != NULL && ParameterNode->GetAnimated() != NULL)
		{
			ImportAnimatedProperty(&Actor->CamOverridePostProcess.MotionBlur_Amount, TEXT("CamOverridePostProcess.MotionBlur_Amount"), MatineeGroup,
				&ParameterNode->GetAnimated()->GetDummy(), ParameterNode->GetAnimated() );
		}
	}
}

void CImporter::ImportAnimatedProperty(FLOAT* Value, const TCHAR* ValueName, UInterpGroupInst* MatineeGroup, const FLOAT* ColladaValue, FCDAnimated* ColladaAnimated)
{
	if (ColladaDocument == NULL || ColladaAnimated== NULL || Value == NULL || MatineeGroup == NULL) return;

	// Retrieve the COLLADA animated element for this value and verify that it contains an animation curve.
	size_t ColladaAnimatedIndex = ColladaAnimated->FindValue(ColladaValue);
	if (ColladaAnimatedIndex >= ColladaAnimated->GetValueCount())
	{
		*Value = *ColladaValue;
		return;
	}

	FCDAnimationCurve* ColladaCurve = ColladaAnimated->GetCurve(ColladaAnimatedIndex);
	if (ColladaCurve == NULL || ColladaCurve->GetKeyCount() == 0)
	{
		*Value = *ColladaValue;
		return;
	}

	// Look for a track for this property in the Matinee group.
	UInterpTrackFloatProp* PropertyTrack = NULL;
	INT TrackCount = MatineeGroup->Group->InterpTracks.Num();
	for (INT TrackIndex = 0; TrackIndex < TrackCount; ++TrackIndex)
	{
		UInterpTrackFloatProp* Track = Cast<UInterpTrackFloatProp>( MatineeGroup->Group->InterpTracks(TrackIndex) );
		if (Track != NULL && Track->PropertyName == ValueName)
		{
			PropertyTrack = Track;
			PropertyTrack->FloatTrack.Reset(); // Remove all the existing keys from this track.
			break;
		}
	}

	// If a track for this property was not found, create one.
	if (PropertyTrack == NULL)
	{
		PropertyTrack = ConstructObject<UInterpTrackFloatProp>( UInterpTrackFloatProp::StaticClass(), MatineeGroup->Group, NAME_None, RF_Transactional );
		MatineeGroup->Group->InterpTracks.AddItem(PropertyTrack);
		UInterpTrackInstFloatProp* PropertyTrackInst = ConstructObject<UInterpTrackInstFloatProp>( UInterpTrackInstFloatProp::StaticClass(), MatineeGroup, NAME_None, RF_Transactional );
		MatineeGroup->TrackInst.AddItem(PropertyTrackInst);
		PropertyTrack->PropertyName = ValueName;
		PropertyTrack->TrackTitle = ValueName;
		PropertyTrackInst->InitTrackInst(PropertyTrack);
	}
	FInterpCurveFloat& Curve = PropertyTrack->FloatTrack;


	INT KeyCount = (INT) ColladaCurve->GetKeyCount();

	for (INT KeyIndex = Curve.Points.Num(); KeyIndex < KeyCount; ++KeyIndex)
	{
		FCDAnimationKey* CurKey = ColladaCurve->GetKey( KeyIndex );

		// Create the curve keys
		FInterpCurvePoint<FLOAT> Key;
		Key.InVal = CurKey->input;

		// Convert the interpolation type for this segment.
		switch( CurKey->interpolation )
		{
		case FUDaeInterpolation::BEZIER:
			{
				FCDAnimationKeyBezier* CurBezierKey = static_cast< FCDAnimationKeyBezier* >( CurKey );

				// Find out whether the tangents are broken or not from all the dimensions of the merged curve.
				const FMVector2& InTangent = CurBezierKey->inTangent;
				const FMVector2& OutTangent = CurBezierKey->outTangent;
				const FLOAT& KeyValue = CurBezierKey->output;

				FLOAT InSlope = (KeyValue - InTangent.y) / (Key.InVal - InTangent.x);
				FLOAT OutSlope = (OutTangent.y - KeyValue) / (OutTangent.x - Key.InVal);
				const UBOOL bBrokenTangents = !IsEquivalent(InSlope, OutSlope);

				Key.InterpMode = bBrokenTangents ? CIM_CurveBreak : CIM_CurveUser;
				break;
			}

		case FUDaeInterpolation::STEP:
			Key.InterpMode = CIM_Constant;
			break;

		case FUDaeInterpolation::LINEAR:
			Key.InterpMode = CIM_Linear;
			break;
		}

		// Add this new key to the curve
		Curve.Points.AddItem(Key);
	}

	// Fill in the curve keys with the correct data for this dimension.
	for (INT KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
	{
		FCDAnimationKey* CurKey = ColladaCurve->GetKey( KeyIndex );
		FInterpCurvePoint<FLOAT>& UnrealKey = Curve.Points( KeyIndex );
		
		// Prepare the COLLADA values to import into the track key.
		// Convert the Bezier control points, if available, into Hermite tangents
		FLOAT OutVal = CurKey->output;	// FIXME COLLADA: No conversion here?

		FLOAT ArriveTangent = 0.0f;
		FLOAT LeaveTangent = 0.0f;

		if( CurKey->interpolation == FUDaeInterpolation::BEZIER )
		{
			FCDAnimationKeyBezier* CurBezierKey = static_cast< FCDAnimationKeyBezier* >( CurKey );

			FCDAnimationKey* PrevSrcKey = ( KeyIndex > 0 ) ? ColladaCurve->GetKey( KeyIndex - 1 ) : NULL;
			FCDAnimationKey* NextSrcKey = ( KeyIndex < KeyCount - 1 ) ? ColladaCurve->GetKey( KeyIndex + 1 ) : NULL;

			// Need to scale the tangent in case the X-coordinate of the
			// control point is not a third of the corresponding interval.
			FLOAT PreviousSpan = (KeyIndex > 0) ? (CurKey->input - PrevSrcKey->input) : 1.0f;
			FLOAT NextSpan = (KeyIndex < KeyCount - 1) ? (NextSrcKey->input - CurKey->input) : 1.0f;

			FVector2D InTangent( CurBezierKey->inTangent.u, CurBezierKey->inTangent.v );
			FVector2D OutTangent( CurBezierKey->outTangent.u, CurBezierKey->outTangent.v );

			if (IsEquivalent(InTangent.X, CurKey->input))
			{
				InTangent.X = CurKey->input - 0.001f; // Push the control point 1 millisecond away.
			}
			if (IsEquivalent(OutTangent.X, CurKey->input))
			{
				OutTangent.X = CurKey->input + 0.001f;
			}

			// FIXME COLLADA: No conversion here?
			ArriveTangent = (CurKey->output - (InTangent.Y)) * PreviousSpan / (CurKey->input - InTangent.X);
			LeaveTangent = ((OutTangent.Y) - CurKey->output) * NextSpan / (OutTangent.X - CurKey->input);
		}

		UnrealKey.OutVal = OutVal;
		UnrealKey.ArriveTangent = ArriveTangent;
		UnrealKey.LeaveTangent = LeaveTangent;
	}
}



void CImporter::ImportAnimatedProperty(FLOAT* Value, const TCHAR* ValueName, UInterpGroupInst* MatineeGroup, FCDAnimationCurve* ColladaCurve )
{
	if (ColladaDocument == NULL || ColladaCurve	== NULL) return;

	// Look for a track for this property in the Matinee group.
	UInterpTrackFloatProp* PropertyTrack = NULL;
	INT TrackCount = MatineeGroup->Group->InterpTracks.Num();
	for (INT TrackIndex = 0; TrackIndex < TrackCount; ++TrackIndex)
	{
		UInterpTrackFloatProp* Track = Cast<UInterpTrackFloatProp>( MatineeGroup->Group->InterpTracks(TrackIndex) );
		if (Track != NULL && Track->PropertyName == ValueName)
		{
			PropertyTrack = Track;
			PropertyTrack->FloatTrack.Reset(); // Remove all the existing keys from this track.
			break;
		}
	}

	// If a track for this property was not found, create one.
	if (PropertyTrack == NULL)
	{
		PropertyTrack = ConstructObject<UInterpTrackFloatProp>( UInterpTrackFloatProp::StaticClass(), MatineeGroup->Group, NAME_None, RF_Transactional );
		MatineeGroup->Group->InterpTracks.AddItem(PropertyTrack);
		UInterpTrackInstFloatProp* PropertyTrackInst = ConstructObject<UInterpTrackInstFloatProp>( UInterpTrackInstFloatProp::StaticClass(), MatineeGroup, NAME_None, RF_Transactional );
		MatineeGroup->TrackInst.AddItem(PropertyTrackInst);
		PropertyTrack->PropertyName = ValueName;
		PropertyTrack->TrackTitle = ValueName;
		PropertyTrackInst->InitTrackInst(PropertyTrack);
	}
	FInterpCurveFloat& Curve = PropertyTrack->FloatTrack;


	INT KeyCount = (INT) ColladaCurve->GetKeyCount();

	for (INT KeyIndex = Curve.Points.Num(); KeyIndex < KeyCount; ++KeyIndex)
	{
		FCDAnimationKey* CurKey = ColladaCurve->GetKey( KeyIndex );

		// Create the curve keys
		FInterpCurvePoint<FLOAT> Key;
		Key.InVal = CurKey->input;

		// Convert the interpolation type for this segment.
		switch( CurKey->interpolation )
		{
		case FUDaeInterpolation::BEZIER:
			{
				FCDAnimationKeyBezier* CurBezierKey = static_cast< FCDAnimationKeyBezier* >( CurKey );

				// Find out whether the tangents are broken or not from all the dimensions of the merged curve.
				const FMVector2& InTangent = CurBezierKey->inTangent;
				const FMVector2& OutTangent = CurBezierKey->outTangent;
				const FLOAT& KeyValue = CurBezierKey->output;

				FLOAT InSlope = (KeyValue - InTangent.y) / (Key.InVal - InTangent.x);
				FLOAT OutSlope = (OutTangent.y - KeyValue) / (OutTangent.x - Key.InVal);
				const UBOOL bBrokenTangents = !IsEquivalent(InSlope, OutSlope);

				Key.InterpMode = bBrokenTangents ? CIM_CurveBreak : CIM_CurveUser;
				break;
			}

		case FUDaeInterpolation::STEP:
			Key.InterpMode = CIM_Constant;
			break;

		case FUDaeInterpolation::LINEAR:
			Key.InterpMode = CIM_Linear;
			break;
		}

		// Add this new key to the curve
		Curve.Points.AddItem(Key);
	}

	// Fill in the curve keys with the correct data for this dimension.
	for (INT KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
	{
		FCDAnimationKey* CurKey = ColladaCurve->GetKey( KeyIndex );
		FInterpCurvePoint<FLOAT>& UnrealKey = Curve.Points( KeyIndex );
		
		// Prepare the COLLADA values to import into the track key.
		// Convert the Bezier control points, if available, into Hermite tangents
		FLOAT OutVal = CurKey->output;	// FIXME COLLADA: No conversion here?

		FLOAT ArriveTangent = 0.0f;
		FLOAT LeaveTangent = 0.0f;

		if( CurKey->interpolation == FUDaeInterpolation::BEZIER )
		{
			FCDAnimationKeyBezier* CurBezierKey = static_cast< FCDAnimationKeyBezier* >( CurKey );

			FCDAnimationKey* PrevSrcKey = ( KeyIndex > 0 ) ? ColladaCurve->GetKey( KeyIndex - 1 ) : NULL;
			FCDAnimationKey* NextSrcKey = ( KeyIndex < KeyCount - 1 ) ? ColladaCurve->GetKey( KeyIndex + 1 ) : NULL;

			// Need to scale the tangent in case the X-coordinate of the
			// control point is not a third of the corresponding interval.
			FLOAT PreviousSpan = (KeyIndex > 0) ? (UnrealKey.InVal - PrevSrcKey->input) : 1.0f;
			FLOAT NextSpan = (KeyIndex < KeyCount - 1) ? (NextSrcKey->input - UnrealKey.InVal) : 1.0f;

			FVector2D InTangent( CurBezierKey->inTangent.u, CurBezierKey->inTangent.v );
			FVector2D OutTangent( CurBezierKey->outTangent.u, CurBezierKey->outTangent.v );

			if (IsEquivalent(InTangent.X, UnrealKey.InVal))
			{
				InTangent.X = UnrealKey.InVal - 0.001f; // Push the control point 1 millisecond away.
			}
			if (IsEquivalent(OutTangent.X, UnrealKey.InVal))
			{
				OutTangent.X = UnrealKey.InVal + 0.001f;
			}

			// FIXME COLLADA: No conversion here?
			ArriveTangent = (OutVal - (InTangent.Y)) * PreviousSpan / (UnrealKey.InVal - InTangent.X);
			LeaveTangent = ((OutTangent.Y) - OutVal) * NextSpan / (OutTangent.X - UnrealKey.InVal);
		}

		UnrealKey.OutVal = OutVal;
		UnrealKey.ArriveTangent = ArriveTangent;
		UnrealKey.LeaveTangent = LeaveTangent;
	}
}


UInterpGroupInst* CImporter::CreateMatineeGroup(USeqAct_Interp* MatineeSequence, AActor* Actor)
{
	// There are no groups for this actor: create the Matinee group data structure.
	UInterpGroup* MatineeGroupData = ConstructObject<UInterpGroup>( UInterpGroup::StaticClass(), MatineeSequence->InterpData, NAME_None, RF_Transactional );
	MatineeGroupData->GroupName = FName( *( FString(Actor->GetName()) + TEXT("Group") ) );
	MatineeSequence->InterpData->InterpGroups.AddItem(MatineeGroupData);

	// Instantiate the Matinee group data structure.
	UInterpGroupInst* MatineeGroup = ConstructObject<UInterpGroupInst>( UInterpGroupInst::StaticClass(), MatineeSequence, NAME_None, RF_Transactional );
	MatineeSequence->GroupInst.AddItem(MatineeGroup);
	MatineeGroup->InitGroupInst(MatineeGroupData, Actor);
	MatineeGroup->SaveGroupActorState();
	MatineeSequence->UpdateConnectorsFromData();

	// Retrieve the Kismet connector for the new group.
	INT ConnectorIndex = MatineeSequence->FindConnectorIndex(MatineeGroupData->GroupName.ToString(), LOC_VARIABLE);
	FIntPoint ConnectorPos = MatineeSequence->GetConnectionLocation(LOC_VARIABLE, ConnectorIndex);			

	// Look for this actor in the Kismet object variables
	USequence* Sequence = MatineeSequence->ParentSequence;
	INT ObjectCount = Sequence->SequenceObjects.Num();
	USeqVar_Object* KismetVariable = NULL;
	for (INT ObjectIndex = 0; ObjectIndex < ObjectCount; ++ObjectIndex)
	{
		USeqVar_Object* Variable = Cast<USeqVar_Object>(Sequence->SequenceObjects(ObjectIndex));
		if (Variable != NULL && Variable->ObjValue == Actor)
		{
			KismetVariable = Variable;
			break;
		}
	}

	if (KismetVariable == NULL)
	{
		// Create the object variable in Kismet.
		KismetVariable = ConstructObject<USeqVar_Object>( USeqVar_Object::StaticClass(), Sequence, NAME_None, RF_Transactional );
		KismetVariable->ObjValue = Actor;
		KismetVariable->ObjPosX = MatineeSequence->ObjPosX + ConnectorIndex * LO_MIN_SHAPE_SIZE * 3 / 2; // ConnectorPos.X is not yet valid. It becomes valid only at the first render.
		KismetVariable->ObjPosY = ConnectorPos.Y + LO_MIN_SHAPE_SIZE * 2;
		Sequence->AddSequenceObject(KismetVariable);
		KismetVariable->OnCreated();
	}

	// Connect this new object variable with the Matinee group data connector.
	FSeqVarLink& VariableConnector = MatineeSequence->VariableLinks(ConnectorIndex);
	VariableConnector.LinkedVariables.AddItem(KismetVariable);
	KismetVariable->OnConnect(MatineeSequence, ConnectorIndex);

	// Dirty the Kismet sequence
	Sequence->Modify();

	return MatineeGroup;
}

void CImporter::CloseDocument()
{
	SAFE_DELETE(ColladaDocument);
	ErrorMessage.Empty();
}

} // namespace UnCollada
