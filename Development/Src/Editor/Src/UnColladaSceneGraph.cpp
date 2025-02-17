/*=============================================================================
	COLLADA scene graph helper.
	Based on top of the Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
==============================================================================*/

#include "EditorPrivate.h"
#include "EngineInterpolationClasses.h"
#include "UnColladaSceneGraph.h"
#include "UnCollada.h"

namespace UnCollada {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CSceneGraph - COLLADA Scene Graph Helper
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSceneGraph::CSceneGraph(FCDocument* _ColladaDocument)
	:	ColladaDocument( _ColladaDocument )
	,	ColladaSceneRoot( NULL )
{
	// Pre-cache the visual scene's root node.
	if ( ColladaDocument )
	{
		ColladaSceneRoot = ColladaDocument->GetVisualSceneRoot();
	}
}

// FCollada should not allow circular references within the scene graph.
#define SCENE_GRAPH_ITERATIVE_SEARCH(ExtraCondition) \
	TArray<FCDSceneNode*> NodeQueue; \
	NodeQueue.Reserve(12); \
	NodeQueue.Push(ColladaSceneRoot); \
	UINT32 BreakCounter = 0; \
	while (NodeQueue.Num() != 0 && ExtraCondition && ++BreakCounter < 1024) \
	{ \
		FCDSceneNode* Node = NodeQueue.Pop(); \
		for( UINT CurChildIndex = 0; CurChildIndex < Node->GetChildrenCount(); ++CurChildIndex ) \
		{ \
			NodeQueue.Push( Node->GetChild( CurChildIndex ) ); \
		}

#define SCENE_GRAPH_ITERATIVE_SEARCH_END \
	} \
	check(BreakCounter < 1024);

/**
 * Retrieve a list of the instantiated meshes/controllers within the visual scene graph.
 */
void CSceneGraph::RetrieveEntityList(TArray<const TCHAR*>& EntityNameList, UBOOL bSkeletalMeshes)
{
	// Iterate through all the nodes, searching for instantiated meshes/controllers
	SCENE_GRAPH_ITERATIVE_SEARCH(1)

	// Check for mesh/controller instances
	for( UINT CurEntityInstanceIndex = 0;
		 CurEntityInstanceIndex < Node->GetInstanceCount();
		 ++CurEntityInstanceIndex )
	{
		const FCDEntityInstance* Instance = Node->GetInstance( CurEntityInstanceIndex );
		if (Instance->GetType() == FCDEntityInstance::GEOMETRY)
		{
			const FCDEntity* Entity = Instance->GetEntity();
			if ((bSkeletalMeshes && Entity->GetType() == FCDEntity::CONTROLLER)
				|| (!bSkeletalMeshes && Entity->GetType() == FCDEntity::GEOMETRY))
			{
				// Ensure that only mesh-based controllers/geometries are added to the list
				const TCHAR* EntityName = Entity->GetName().c_str();
				while (Entity != NULL && Entity->GetType() == FCDEntity::CONTROLLER)
				{
					FCDController* Controller = (FCDController*) Entity;
					Entity = Controller->GetBaseTarget();
				}

				// Retrieve the mesh pointer
				if (Entity != NULL && Entity->GetType() == FCDEntity::GEOMETRY)
				{
					FCDGeometry* Geometry = (FCDGeometry*) Entity;
					if (Geometry->IsMesh())
					{
						EntityNameList.Push(EntityName);
					}
				}
			}
		}
	}

	SCENE_GRAPH_ITERATIVE_SEARCH_END
}

/**
 * Retrieve the mesh with the given name and its instantiation information.
 */
FCDGeometryInstance* CSceneGraph::RetrieveMesh(const TCHAR* MeshName, FCDGeometryMesh*& ColladaMesh, FCDSkinController*& ColladaSkin, FCDSceneNode*& ColladaNode, FMMatrix44* WorldTransform)
{
	FCDGeometryInstance* WantedInstance = NULL;
	ColladaSkin = NULL;
	ColladaMesh = NULL;
	ColladaNode = NULL;

	// Iterate through all the nodes, searching for instantiated controllers
	SCENE_GRAPH_ITERATIVE_SEARCH(WantedInstance == NULL)

		// Check for the wanted controller within the instances
	for( UINT CurEntityInstanceIndex = 0;
		 CurEntityInstanceIndex < Node->GetInstanceCount();
		 ++CurEntityInstanceIndex )
	{
		FCDEntityInstance* Instance = Node->GetInstance( CurEntityInstanceIndex );
		if (Instance->GetType() == FCDEntityInstance::GEOMETRY)
		{
			FCDEntity* Entity = Instance->GetEntity();
			if ((Entity->GetType() == FCDEntity::CONTROLLER || Entity->GetType() == FCDEntity::GEOMETRY)
				&& (MeshName == NULL || Entity->GetName() == MeshName))
			{
				WantedInstance = (FCDGeometryInstance*) Instance;

				// Calculate the world-space transformation for this instance
				if (WorldTransform != NULL)
				{
					*WorldTransform = FMMatrix44::Identity;
					FCDSceneNode* ParentNode = Node;
					while (ParentNode != NULL)
					{
						*WorldTransform = ParentNode->ToMatrix() * (*WorldTransform);
						ParentNode = ParentNode->GetParent();
					}
				}

				// For controllers: iterate through the sources, until we reach the base geometry
				while (Entity != NULL && Entity->GetType() == FCDEntity::CONTROLLER)
				{
					FCDController* Controller = (FCDController*) Entity;
					Entity = Controller->GetBaseTarget();

					if (ColladaSkin == NULL && Controller->GetSkinController() != NULL )
					{
						ColladaSkin = Controller->GetSkinController();
					}
				}

				// Retrieve the mesh pointer
				if (Entity != NULL && Entity->GetType() == FCDEntity::GEOMETRY)
				{
					FCDGeometry* Geometry = (FCDGeometry*) Entity;
					if (Geometry->IsMesh())
					{
						ColladaMesh = Geometry->GetMesh();
					}
				}

				ColladaNode = Node;
				break;
			}
		}
	}

	SCENE_GRAPH_ITERATIVE_SEARCH_END

	return WantedInstance;
}

/**
 * Retrieve the list of all the bones/joints within the visual scene graph. Is used by the import of bone animations.
 */
void CSceneGraph::RetrieveBoneList(TArray<FCDSceneNode*> BoneList)
{
	SCENE_GRAPH_ITERATIVE_SEARCH(1)

	if ( Node->IsJoint() )
	{
		BoneList.Push( Node );
	}

	SCENE_GRAPH_ITERATIVE_SEARCH_END
}

/**
 * Imports a COLLADA scene node into a Matinee actor group.
 */
FLOAT CSceneGraph::ImportMatineeActor(FCDSceneNode* ColladaNode, UInterpGroupInst* MatineeGroup)
{
	FName DefaultName(NAME_None);

	if (ColladaDocument == NULL || ColladaNode == NULL || MatineeGroup == NULL) return -1.0f;

	// Search for a Movement track in the Matinee group.
	UInterpTrackMove* MovementTrack = NULL;
	INT TrackCount = MatineeGroup->Group->InterpTracks.Num();
	for (INT TrackIndex = 0; TrackIndex < TrackCount && MovementTrack == NULL; ++TrackIndex)
	{
		MovementTrack = Cast<UInterpTrackMove>( MatineeGroup->Group->InterpTracks(TrackIndex) );
	}

	// Check whether the actor should be pivoted in the COLLADA document.
	UBOOL ColladaHasPivot = FALSE;
	if (ColladaNode->GetInstanceCount() == 0 && ColladaNode->GetChildrenCount() > 0)
	{
		// Retrieve the pivot node. The pivot transforms may have been modified,
		// so test against a simple point rather than check for actual transformations.
		FCDSceneNode* PivotNode = ColladaNode->GetChild(0);
		FMMatrix44 PivotTransform = PivotNode->ToMatrix();
		FMVector3 Forward = PivotTransform.TransformCoordinate( -FMVector3::ZAxis );
		FMVector3 Up = PivotTransform.TransformCoordinate( FMVector3::YAxis );
		ColladaHasPivot = IsEquivalent( Forward, FMVector3::XAxis ) && IsEquivalent( Up, FMVector3::ZAxis );
	}
	AActor* Actor = MatineeGroup->GetGroupActor();
	check( Actor != NULL ); // would this ever be triggered?
	UBOOL ColladaNeedsPivot = Actor->IsA( ACameraActor::StaticClass() ) || Actor->IsA( ALight::StaticClass() );

	if (ColladaNeedsPivot && !ColladaHasPivot)
	{
		// Add to the COLLADA scene node some transforms to force the correct forward/up-axises.
		// In all cases: this will force import sampling, because of the order of the rotations.
		FCDTRotation* Rotation1 = (FCDTRotation*) ColladaNode->AddTransform( FCDTransform::ROTATION );
		Rotation1->SetAxis(FMVector3::YAxis);
		Rotation1->SetAngle( 90.0f );
		FCDTRotation* Rotation2 = (FCDTRotation*) ColladaNode->AddTransform( FCDTransform::ROTATION );
		Rotation2->SetAxis(FMVector3::XAxis);
		Rotation2->SetAngle( -90.0f );
	}

	// Find out whether the COLLADA node is animated.
	// Bucket the transforms at the same time.
	// The Matinee Movement track can take in a Translation vector
	// and three animated Euler rotation angles.
	UBOOL IsNodeAnimated = FALSE;
	UBOOL ForceImportSampling = FALSE;
	FCDTTranslation* Translation = NULL;
	FCDTRotation* RotateX = NULL,* RotateY = NULL,* RotateZ = NULL;
	INT TransformBucket = 0;
	size_t TransformCount = ColladaNode->GetTransformCount();
	for (size_t TransformIndex = 0; TransformIndex < TransformCount; ++TransformIndex)
	{
		FCDTransform* Transform = ColladaNode->GetTransform(TransformIndex);
		UBOOL IsTransformAnimated = (UBOOL) Transform->IsAnimated();
		IsNodeAnimated |= IsTransformAnimated;

		if (Transform->GetType() == FCDTransform::TRANSLATION && TransformBucket == 0)
		{
			Translation = (FCDTTranslation*) Transform;
			TransformBucket = 1;
		}
		else if (Transform->GetType() == FCDTransform::ROTATION)
		{
			FCDTRotation* Rotation = (FCDTRotation*) Transform;
			FMVector3& RotationAxis = Rotation->GetAxis();
			if (IsEquivalent(RotationAxis, FMVector3::ZAxis) && TransformBucket <= 1)
			{
				RotateZ = Rotation;
				TransformBucket = 2;
			}
			else if (IsEquivalent(RotationAxis, FMVector3::YAxis) && TransformBucket <= 2)
			{
				RotateY = Rotation;
				TransformBucket = 3;
			}
			else if (IsEquivalent(RotationAxis, FMVector3::XAxis) && TransformBucket <= 3)
			{
				RotateX = Rotation;
				TransformBucket = 4;
			}
			else if (!IsEquivalent(Rotation->GetAngle(), 0.0f))
			{
				ForceImportSampling = TRUE;
				break;
			}
		}
		else if (Transform->GetType() == FCDTransform::SCALE)
		{
			FCDTScale* Scale = ( FCDTScale* )Transform;

			// Ignore scale changes
			break;
		}
		else
		{
			ForceImportSampling = TRUE;
		}
	}

	// Add a Movement track if the node is animated and the group does not already have one.
	if (MovementTrack == NULL && IsNodeAnimated)
	{
		MovementTrack = ConstructObject<UInterpTrackMove>( UInterpTrackMove::StaticClass(), MatineeGroup->Group, NAME_None, RF_Transactional );
		MatineeGroup->Group->InterpTracks.AddItem(MovementTrack);
		UInterpTrackInstMove* MovementTrackInst = ConstructObject<UInterpTrackInstMove>( UInterpTrackInstMove::StaticClass(), MatineeGroup, NAME_None, RF_Transactional );
		MatineeGroup->TrackInst.AddItem(MovementTrackInst);
		MovementTrackInst->InitTrackInst(MovementTrack);
	}

	// Remove all the keys in the Movement track
	if (MovementTrack != NULL)
	{
		MovementTrack->PosTrack.Reset();
		MovementTrack->EulerTrack.Reset();
		MovementTrack->LookupTrack.Points.Reset();
	}

	FLOAT TimeLength = -1.0f;

	// Fill in the Movement track with the COLLADA keys
	if (IsNodeAnimated)
	{
		if (!ForceImportSampling)
		{
			// Merge the position and rotation animation curves, in order to follow the UE3 editor assumptions:
			// The position and rotation tracks must have the same number of keys, the same key timings and
			// the same segment interpolation types.
			FCDAnimationCurveList ColladaCurves;
			FloatList DefaultValues;
			ColladaCurves.resize(6);
			DefaultValues.resize(6);

			if (Translation != NULL)
			{
				DefaultValues[0] = Translation->GetTranslation()->x;
				DefaultValues[1] = Translation->GetTranslation()->y;
				DefaultValues[2] = Translation->GetTranslation()->z;

				if (Translation->IsAnimated())
				{
					FCDAnimated* ColladaAnimated = Translation->GetAnimated();
					for (size_t CurveIdx = 0; CurveIdx < 3; ++CurveIdx)
					{
						ColladaCurves[CurveIdx] = ColladaAnimated->GetCurve(CurveIdx);
					}
				}
			}

	#define FILL_ROTATE_CURVE(RotateCurve, CurveIndex) { \
			if (RotateCurve != NULL) { \
				DefaultValues[CurveIndex] = RotateCurve->GetAngle(); \
				if (RotateCurve->IsAnimated()) { \
					ColladaCurves[CurveIndex] = RotateCurve->GetAnimated()->FindCurve(&RotateCurve->GetAngle()); } } }

			FILL_ROTATE_CURVE(RotateX, 3);
			FILL_ROTATE_CURVE(RotateY, 4);
			FILL_ROTATE_CURVE(RotateZ, 5);
	#undef FILL_ROTATE_CURVE

			// Reset the actor position.
			Actor->SetLocation( FVector( DefaultValues[0], -DefaultValues[1], DefaultValues[2] ) );
			Actor->SetRotation( FRotator( DefaultValues[3], -DefaultValues[4], -DefaultValues[5] ) );

			// Check for a relative or absolute curve
			UBOOL IsRelative = (MovementTrack->MoveFrame == IMF_RelativeToInitial);
			FVector InitialTranslation(0.0f, 0.0f, 0.0f);
			FVector InitialRotation(0.0f, 0.0f, 0.0f);
			if (IsRelative)
			{
				InitialTranslation = FVector(DefaultValues[0], -DefaultValues[1], DefaultValues[2]);
				InitialRotation = FVector(DefaultValues[3], -DefaultValues[4], -DefaultValues[5]);
			}

			// Merge all the curves
			FCDAnimationMultiCurve* ColladaMergedCurve = FCDAnimationCurveTools::MergeCurves(ColladaCurves, DefaultValues);
			if (ColladaMergedCurve != NULL)
			{
				if (ColladaMergedCurve->GetKeyCount() == 0)
				{
					// Create one key to enforce the current value.
					FInterpCurvePoint<FVector> Key;
					Key.InVal = 0.0f;
					Key.OutVal = InitialTranslation;
					Key.ArriveTangent = Key.LeaveTangent = FVector(0.0f, 0.0f, 0.0f);
					Key.InterpMode = CIM_Linear;
					MovementTrack->PosTrack.Points.AddItem(Key);

					Key.OutVal = InitialRotation;
					MovementTrack->EulerTrack.Points.AddItem(Key);

					MovementTrack->LookupTrack.AddPoint(0.0f, DefaultName);
				}
				else
				{
					// Import all the individual components
					FCDConversionScaleOffsetFunctor Conversion;
					ImportMatineeAnimated(ColladaMergedCurve, 0, MovementTrack->PosTrack, 0, &Conversion.Set(1.0f, -InitialTranslation.X));
					ImportMatineeAnimated(ColladaMergedCurve, 1, MovementTrack->PosTrack, 1, &Conversion.Set(-1.0f, InitialTranslation.Y));
					ImportMatineeAnimated(ColladaMergedCurve, 2, MovementTrack->PosTrack, 2, &Conversion.Set(1.0f, -InitialTranslation.Z));
					ImportMatineeAnimated(ColladaMergedCurve, 3, MovementTrack->EulerTrack, 0, &Conversion.Set(1.0f, -InitialRotation.X));
					ImportMatineeAnimated(ColladaMergedCurve, 4, MovementTrack->EulerTrack, 1, &Conversion.Set(-1.0f, InitialRotation.Y));
					ImportMatineeAnimated(ColladaMergedCurve, 5, MovementTrack->EulerTrack, 2, &Conversion.Set(-1.0f, InitialRotation.Z));

					// Generate empty look-up keys.
					for ( size_t Index = 0; Index < ColladaMergedCurve->GetKeyCount(); ++Index )
					{
						MovementTrack->LookupTrack.AddPoint( ColladaMergedCurve->GetKeys()[ Index ]->input, DefaultName );
					}

					// Scale the track timing to ensure that it is large enough
					TimeLength = ColladaMergedCurve->GetKeys()[ ColladaMergedCurve->GetKeyCount() - 1 ]->input;
				}

				ColladaMergedCurve->Release();
			}
		}
		else
		{
			FMVector3 DefaultScale, DefaultTranslation, DefaultEuler; float DefaultInverted;
			FMMatrix44 DefaultTransform = ColladaNode->ToMatrix();
			DefaultTransform.Decompose(DefaultScale, DefaultEuler, DefaultTranslation, DefaultInverted);

			// Reset the actor position.
			Actor->SetLocation( FVector( DefaultTranslation.x, -DefaultTranslation.y, DefaultTranslation.z ) );
			Actor->SetRotation( FRotator( -DefaultEuler.x, -DefaultEuler.y, DefaultEuler.z ) );

			// Check for a relative or absolute curve
			UBOOL IsRelative = (MovementTrack->MoveFrame == IMF_RelativeToInitial);
			FVector InitialTranslation(0.0f, 0.0f, 0.0f);
			FVector InitialRotation(0.0f, 0.0f, 0.0f);
			if (IsRelative)
			{
				InitialTranslation = FVector(DefaultTranslation.x, -DefaultTranslation.y, DefaultTranslation.z);
				InitialRotation = FVector(DefaultEuler.x, -DefaultEuler.y, -DefaultEuler.z);
			}

			// Do the import sampling.
			FloatList SamplingKeys;
			FMMatrix44List SamplingValues;
			{
				FCDSceneNodeTools::GenerateSampledAnimation( ColladaNode );
				SamplingKeys = FCDSceneNodeTools::GetSampledAnimationKeys();
				SamplingValues = FCDSceneNodeTools::GetSampledAnimationMatrices();
			}

			size_t SamplingKeyCount = SamplingKeys.size();
			check( SamplingValues.size() == SamplingKeyCount );

			for ( size_t KeyIndex = 0; KeyIndex < SamplingKeyCount; ++KeyIndex )
			{
				// Decompose the sample.
				FMMatrix44& SampleValue = SamplingValues[ KeyIndex ];
				float& SampleTime = SamplingKeys[ KeyIndex ];
				FMVector3 SampleTranslation, SampleScale, SampleEulers; float SampleInverted;
				SampleValue.Decompose( SampleScale, SampleEulers, SampleTranslation, SampleInverted );

				// Create the translation and rotation keys
				FInterpCurvePoint<FVector> PosKey, RotKey;
				PosKey.InVal = RotKey.InVal = SampleTime;
				PosKey.InterpMode = RotKey.InterpMode = CIM_CurveUser;
				PosKey.OutVal = FVector( SampleTranslation.x - InitialTranslation.X, - SampleTranslation.y + InitialTranslation.Y, SampleTranslation.z - InitialTranslation.Z );
				RotKey.OutVal = FVector( FMath::RadToDeg( SampleEulers.x - InitialRotation.X ), - FMath::RadToDeg( SampleEulers.y - InitialRotation.Y ), - FMath::RadToDeg( SampleEulers.z - InitialRotation.Z ) );

				// Add the new keys to the Matinee track.
				MovementTrack->PosTrack.Points.AddItem( PosKey );
				MovementTrack->EulerTrack.Points.AddItem( RotKey );

				// Generate an empty look-up key.
				MovementTrack->LookupTrack.AddPoint( SampleTime, DefaultName );
			}

			// As a second step: generate the Hermite tangents.
			for ( size_t KeyIndex = 0; KeyIndex < SamplingKeyCount; ++KeyIndex )
			{
				FInterpCurvePoint<FVector>& PosKey = MovementTrack->PosTrack.Points( KeyIndex );
				FInterpCurvePoint<FVector>& RotKey = MovementTrack->EulerTrack.Points( KeyIndex );

				// Retrieve the previous and next keys.
				size_t PreviousIndex = ( KeyIndex > 0 ) ? ( KeyIndex - 1 ) : 0;
				size_t NextIndex = ( KeyIndex < SamplingKeyCount - 1 ) ? ( KeyIndex + 1 ) : ( SamplingKeyCount - 1 );
				FInterpCurvePoint<FVector>& PreviousPosKey = MovementTrack->PosTrack.Points( PreviousIndex );
				FInterpCurvePoint<FVector>& PreviousRotKey = MovementTrack->EulerTrack.Points( PreviousIndex );
				FInterpCurvePoint<FVector>& NextPosKey = MovementTrack->PosTrack.Points( NextIndex );
				FInterpCurvePoint<FVector>& NextRotKey = MovementTrack->EulerTrack.Points( NextIndex );

				// Calculate the slope and generate the Hermite tangents.
				FLOAT SlopeSpan = !IsEquivalent( NextPosKey.InVal, PreviousPosKey.InVal ) ? NextPosKey.InVal - PreviousPosKey.InVal : 0.001f;
				PosKey.ArriveTangent = PosKey.LeaveTangent = ( NextPosKey.OutVal - PreviousPosKey.OutVal ) / SlopeSpan;
				RotKey.ArriveTangent = RotKey.LeaveTangent = ( NextRotKey.OutVal - PreviousRotKey.OutVal ) / SlopeSpan;
			}

			// Scale the track timing to ensure that it is large enough.
			TimeLength = SamplingKeys.back();
		}
	}

	// Inform the engine and UI that the tracks have been modified.
	if (MovementTrack != NULL)
	{
		MovementTrack->Modify();
	}
	MatineeGroup->Modify();
	return TimeLength;
}

void CSceneGraph::ImportMatineeAnimated(FCDAnimationMultiCurve* ColladaCurve, INT ColladaDimension, FInterpCurveVector& Curve, INT CurveIndex, FCDConversionFunctor* Conversion)
{
	if (ColladaCurve == NULL || ColladaDimension >= (INT) ColladaCurve->GetDimension() || CurveIndex >= 3) return;
	FCDConversionScaleFunctor IdentifyFunctor(1.0f);
	if (Conversion == NULL) Conversion = &IdentifyFunctor;

	INT KeyCount = (INT) ColladaCurve->GetKeyCount();

	for (INT KeyIndex = Curve.Points.Num(); KeyIndex < KeyCount; ++KeyIndex)
	{
		FCDAnimationMKey* CurKey = ColladaCurve->GetKey( KeyIndex );

		// Create the curve keys
		FInterpCurvePoint<FVector> Key;
		Key.InVal = CurKey->input;

		// Convert the interpolation type for this segment.
		switch( CurKey->interpolation )
		{
		case FUDaeInterpolation::BEZIER:
			{
				FCDAnimationMKeyBezier* CurBezierKey = static_cast< FCDAnimationMKeyBezier* >( CurKey );

				// Find out whether the tangents are broken or not from all the dimensions of the merged curve.
				UBOOL BrokenTangents = FALSE;
				for (uint32 Dimension = 0; Dimension < ColladaCurve->GetDimension() && !BrokenTangents; ++Dimension)
				{
					const FMVector2& InTangent = CurBezierKey->inTangent[ Dimension ];
					const FMVector2& OutTangent = CurBezierKey->outTangent[ Dimension ];
					const FLOAT& KeyValue = CurBezierKey->output[ Dimension ];

					FLOAT InSlope = (KeyValue - InTangent.y) / (Key.InVal - InTangent.x);
					FLOAT OutSlope = (OutTangent.y - KeyValue) / (OutTangent.x - Key.InVal);
					BrokenTangents = BrokenTangents || !IsEquivalent(InSlope, OutSlope);
				}
				Key.InterpMode = BrokenTangents ? CIM_CurveBreak : CIM_CurveUser;
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
		FCDAnimationMKey* CurKey = ColladaCurve->GetKey( KeyIndex );
		FInterpCurvePoint<FVector>& UnrealKey = Curve.Points( KeyIndex );
		
		// Prepare the COLLADA values to import into the track key.
		// Convert the Bezier control points, if available, into Hermite tangents
		FLOAT OutVal = (*Conversion)( CurKey->output[ColladaDimension] );

		FLOAT ArriveTangent = 0.0f;
		FLOAT LeaveTangent = 0.0f;

		if( CurKey->interpolation == FUDaeInterpolation::BEZIER )
		{
			FCDAnimationMKeyBezier* CurBezierKey = static_cast< FCDAnimationMKeyBezier* >( CurKey );

			FCDAnimationMKey* PrevSrcKey = ( KeyIndex > 0 ) ? ColladaCurve->GetKey( KeyIndex - 1 ) : NULL;
			FCDAnimationMKey* NextSrcKey = ( KeyIndex < KeyCount - 1 ) ? ColladaCurve->GetKey( KeyIndex + 1 ) : NULL;

			// Need to scale the tangent in case the X-coordinate of the
			// control point is not a third of the corresponding interval.
			FLOAT PreviousSpan = (KeyIndex > 0) ? (UnrealKey.InVal - PrevSrcKey->input) : 1.0f;
			FLOAT NextSpan = (KeyIndex < KeyCount - 1) ? (NextSrcKey->input - UnrealKey.InVal) : 1.0f;

			FVector2D InTangent( CurBezierKey->inTangent[ColladaDimension].u, CurBezierKey->inTangent[ColladaDimension].v );
			FVector2D OutTangent( CurBezierKey->outTangent[ColladaDimension].u, CurBezierKey->outTangent[ColladaDimension].v );

			if (IsEquivalent(InTangent.X, UnrealKey.InVal))
			{
				InTangent.X = UnrealKey.InVal - 0.001f; // Push the control point 1 millisecond away.
			}
			if (IsEquivalent(OutTangent.X, UnrealKey.InVal))
			{
				OutTangent.X = UnrealKey.InVal + 0.001f;
			}

			ArriveTangent = (OutVal - (*Conversion)(InTangent.Y)) * PreviousSpan / (UnrealKey.InVal - InTangent.X);
			LeaveTangent = ((*Conversion)(OutTangent.Y) - OutVal) * NextSpan / (OutTangent.X - UnrealKey.InVal);
		}

		// Fill in the track key with the prepared values
		switch (CurveIndex)
		{
		case 0:
			UnrealKey.OutVal.X = OutVal;
			UnrealKey.ArriveTangent.X = ArriveTangent;
			UnrealKey.LeaveTangent.X = LeaveTangent;
			break;
		case 1:
			UnrealKey.OutVal.Y = OutVal;
			UnrealKey.ArriveTangent.Y = ArriveTangent;
			UnrealKey.LeaveTangent.Y = LeaveTangent;
			break;
		case 2:
		default:
			UnrealKey.OutVal.Z = OutVal;
			UnrealKey.ArriveTangent.Z = ArriveTangent;
			UnrealKey.LeaveTangent.Z = LeaveTangent;
			break;
		}
	}
}

} // namespace UnCollada
