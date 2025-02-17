/*=============================================================================
	COLLADA exporter for Unreal Engine 3.
	Based on Feeling Software's FCollada.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineInterpolationClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineParticleClasses.h"
#include "UnCollada.h"
#include "UnColladaExporter.h"
#include "UnColladaStaticMesh.h"
#include "UnTerrain.h"

namespace UnCollada {

// Custom conversion functors.
class FCDConversionSampleRadiusFunctor : public FCDConversionFunctor
{
public:
	FLOAT FocusDistanceTimes10;
	FCDConversionSampleRadiusFunctor(FLOAT _FocusDistance)
		:	FocusDistanceTimes10(_FocusDistance * 10.0f) {}
	virtual ~FCDConversionSampleRadiusFunctor() {}
	virtual FLOAT operator() (FLOAT Value) { return Value > 1.0f ? FocusDistanceTimes10 / Value : FocusDistanceTimes10; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CExporterActor - A structure to hold the FCollada structures for a given UE3 actor
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CExporterActor::CExporterActor()
	:	Actor(NULL), ColladaNode(NULL), ColladaEntity(NULL), ColladaInstance(NULL)
{
}

CExporterActor::~CExporterActor()
{
	// Release all the properties
	INT PropertyCount = Properties.Num();
	for (INT Idx = 0; Idx < PropertyCount; ++Idx)
	{
		CExporterAnimatableProperty* Property = Properties(Idx);
		SAFE_DELETE(Property->Conversion);
		SAFE_DELETE(Property);
	}
	Properties.Empty();

	// Reset the actor link pointers
	Actor = NULL;
	ColladaNode = NULL;
	ColladaEntity = NULL;
	ColladaInstance = NULL;
}

void CExporterActor::Instantiate(FCDEntity* _ColladaEntity)
{
	ColladaEntity = _ColladaEntity;

	if (ColladaNode != NULL)
	{
		if (ColladaEntity->GetObjectType().Includes(FCDLight::GetClassType())
			|| ColladaEntity->GetObjectType().Includes(FCDCamera::GetClassType()))
		{
			// For cameras and lights: always add a Y-pivot rotation to get the correct coordinate system.
			FCDSceneNode* ColladaPivotNode = ColladaNode->AddChildNode();
			FCDTRotation* ColladaPivotRotationZ = (FCDTRotation*) ColladaPivotNode->AddTransform(FCDTransform::ROTATION);
			FCDTRotation* ColladaPivotRotationX = (FCDTRotation*) ColladaPivotNode->AddTransform(FCDTransform::ROTATION);
			ColladaPivotRotationZ->SetRotation(FMVector3::ZAxis, -90.0f);
			ColladaPivotRotationX->SetRotation(FMVector3::XAxis, 90.0f);
			ColladaPivotNode->SetDaeId(ColladaNode->GetDaeId() + "-pivot");
			ColladaInstance = ColladaPivotNode->AddInstance(ColladaEntity);
		}
		else
		{
			ColladaInstance = ColladaNode->AddInstance(ColladaEntity);
		}
	}
}

CExporterAnimatableProperty* CExporterActor::FindProperty(const TCHAR* Name)
{
	// The UE3 generates property names programmatically (?) and
	// prefixes the property names with the name of the structure(s) needed to
	// access them.
	// We only want to consider the property suffix.
	const TCHAR* Suffix = appStrrchr(Name, '.');
	if (Suffix != NULL)
	{
		Name = Suffix + 1;
	}

	// Look for the property name suffix in the COLLADA linking structures.
	INT PropertyCount = Properties.Num();
	for (INT Idx = 0; Idx < PropertyCount; ++Idx)
	{
		CExporterAnimatableProperty* Property = Properties(Idx);
		if (appStrcmp(*Property->PropertyName, Name) == 0)
		{
			return Property;
		}
	}
	return NULL;
}

CExporterAnimatableProperty* CExporterActor::CreateProperty(const TCHAR* Name, FCDAnimated* ColladaAnimated, FLOAT* ColladaValues, INT ColladaValueCount, FCDConversionFunctor* Conversion)
{
	// All properties should have an animated representation.  We don't actually need to do anything
	// with this here other than verify that it exists and then store it for later!
	check( ColladaAnimated != NULL );

	// Create the UE3-COLLADA property linking structure
	CExporterAnimatableProperty* Property = new CExporterAnimatableProperty();
	Property->PropertyName = Name;
	Property->ColladaAnimated = ColladaAnimated;
	Property->ColladaValues.Add(ColladaValueCount);
	for (INT Idx = 0; Idx < ColladaValueCount; ++Idx)
	{
		Property->ColladaValues(Idx) = ColladaValues + Idx;
	}
	Property->Conversion = Conversion;
	Properties.AddItem(Property);

	return Property;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CExporter - Main COLLADA exporter singleton
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CExporter::CExporter()
	:	ColladaDocument(NULL)
{
	// Initialize COLLADA.  It's fine for this to be called more than once, as long as Release is also called.
	FCollada::Initialize();
}

CExporter::~CExporter()
{
	CloseDocument();

	// Clean up COLLADA (ref counted)
	FCollada::Release();
}

/**
 * Returns the exporter singleton. It will be created on the first request.
 */
CExporter* CExporter::GetInstance()
{
	static CExporter* ExporterInstance = new CExporter();
	return ExporterInstance;
}

/**
 * Creates and readies an empty document for export.
 */
void CExporter::CreateDocument()
{
	// Clears any old document that is still present.
	SAFE_DELETE(ColladaDocument);

	// Create an empty COLLADA document and fill up its asset information.
	ColladaDocument = FCollada::NewTopDocument();
	ExportAsset(ColladaDocument->GetAsset());
}

/**
 * Writes the COLLADA document to disk and releases it.
 */
void CExporter::WriteToFile(const TCHAR* Filename)
{
	// Write out the COLLADA document to disk.
	FCollada::SaveDocument( ColladaDocument, Filename );

	// Release the document
	CloseDocument();
}

/**
 * Closes the COLLADA document, releasing its memory.
 */
void CExporter::CloseDocument()
{
	// You need to release all the buffered actor structures before releasing the FCollada document.
	INT ActorCount = ColladaActors.Num();
	for (INT Idx = 0; Idx < ActorCount; ++Idx)
	{
		CExporterActor* ColladaActor = ColladaActors(Idx);
		SAFE_DELETE(ColladaActor);
	}
	ColladaActors.Empty();
	SAFE_DELETE(ColladaDocument);
}

/**
 * Exports the basic scene information to the COLLADA document.
 */
void CExporter::ExportLevelMesh(ULevel* Level, USeqAct_Interp* MatineeSequence )
{
	if (ColladaDocument == NULL || Level == NULL) return;

	// Exports the level's scene geometry
	if (Level->Model != NULL && Level->Model->VertexBuffer.Vertices.Num() > 2 && Level->Model->MaterialIndexBuffers.Num() > 0)
	{
		// Create the COLLADA structures to hold the scene geometry
		FCDSceneNode* ColladaSceneRoot = ColladaDocument->GetVisualSceneRoot();
		if (ColladaSceneRoot == NULL)
		{
			ColladaSceneRoot = ColladaDocument->AddVisualScene();
		}
		FCDSceneNode* ColladaWorldGeometryNode = ColladaSceneRoot->AddChildNode();
		ColladaWorldGeometryNode->SetName(TEXT("LevelMesh"));
		ColladaWorldGeometryNode->SetDaeId("LevelMesh-node");
		FCDGeometry* ColladaWorldGeometry = ColladaDocument->GetGeometryLibrary()->AddEntity();
		ColladaWorldGeometry->SetName(TEXT("LevelMesh"));
		ColladaWorldGeometry->SetDaeId("LevelMesh-obj");
		FCDGeometryInstance* ColladaWorldInstance = (FCDGeometryInstance*) ColladaWorldGeometryNode->AddInstance(ColladaWorldGeometry);

		// Export the mesh for the world
		ExportModel(Level->Model, ColladaWorldGeometry, ColladaWorldInstance);
	}

	// Export all the recognized global actors.
	// Right now, this only includes lights.
	INT ActorCount = GWorld->CurrentLevel->Actors.Num();
	for (INT ActorIndex = 0; ActorIndex < ActorCount; ++ActorIndex)
	{
		AActor* Actor = GWorld->CurrentLevel->Actors(ActorIndex);
		if (Actor != NULL)
		{
			if (Actor->IsA(ALight::StaticClass()))
			{
				ExportLight((ALight*) Actor, MatineeSequence );
			}
			else if (Actor->IsA(AStaticMeshActor::StaticClass()))
			{
				ExportStaticMesh( Actor, ((AStaticMeshActor*) Actor)->StaticMeshComponent, MatineeSequence );
			}
			else if (Actor->IsA(ADynamicSMActor::StaticClass()))
			{
				ExportStaticMesh( Actor, ((ADynamicSMActor*) Actor)->StaticMeshComponent, MatineeSequence );
			}
			else if (Actor->IsA(ABrush::StaticClass()))
			{
				// All brushes should be included within the world geometry exported above.
				// ExportBrush((ABrush*) Actor);
			}
			else if (Actor->IsA(ATerrain::StaticClass()))
			{
				// ExportTerrain?((ATerrain*) Actor);
			}
			else if (Actor->IsA(AEmitter::StaticClass()))
			{
				ExportActor( Actor, MatineeSequence ); // Just export the placement of the particle emitter.
			}
		}
	}
}

/**
 * Exports the light-specific information for a UE3 light actor.
 */
void CExporter::ExportLight( ALight* Actor, USeqAct_Interp* MatineeSequence )
{
	if (ColladaDocument == NULL || Actor == NULL || Actor->LightComponent == NULL) return;

	// Export the basic actor information.
	CExporterActor* ColladaActor = ExportActor( Actor, MatineeSequence );
	ULightComponent* BaseLight = Actor->LightComponent;

	// Choose a name for this actor in the COLLADA file.  If the actor is bound to a Matinee sequence, we'll
	// use the Matinee group name, otherwise we'll just use the actor name.
	FString MyColladaNodeName = Actor->GetName();
	if( MatineeSequence != NULL )
	{
		const UInterpGroupInst* FoundGroupInst = MatineeSequence->FindGroupInst( Actor );
		if( FoundGroupInst != NULL )
		{
			MyColladaNodeName = FoundGroupInst->Group->GroupName.ToString();
		}
	}

	// Create a properly-named COLLADA light structure and instantiate it in the COLLADA scene graph
	FCDLight* ColladaLight = ColladaDocument->GetLightLibrary()->AddEntity();
	ColladaLight->SetName( *MyColladaNodeName );
	ColladaLight->SetDaeId(TO_STRING(*Actor->GetName()) + "-object");
	ColladaActor->Instantiate(ColladaLight);

	// Export the 'enabled' flag as the visibility for the scene node.
	ColladaActor->ColladaNode->SetVisibility(Actor->bEnabled);

	// Export the basic light information
	ColladaLight->SetIntensity(BaseLight->Brightness);
	ColladaActor->CreateProperty(TEXT("Brightness"), ColladaLight->GetIntensity().GetAnimated(), &( *ColladaLight->GetIntensity() ));
	ColladaLight->SetColor(ToFMVector3(BaseLight->LightColor)); // not animatable

	// Look for the higher-level light types and determine the lighting method
	if (BaseLight->IsA(UPointLightComponent::StaticClass()))
	{
		UPointLightComponent* PointLight = (UPointLightComponent*) BaseLight;
		if (BaseLight->IsA(USpotLightComponent::StaticClass()))
		{
			USpotLightComponent* SpotLight = (USpotLightComponent*) BaseLight;
			ColladaLight->SetLightType(FCDLight::SPOT);

			// Export the spot light parameters.
			ColladaLight->SetFallOffAngle(SpotLight->InnerConeAngle); // not animatable
			ColladaLight->SetOuterAngle(SpotLight->OuterConeAngle); // not animatable
		}
		else
		{
			ColladaLight->SetLightType(FCDLight::POINT);
		}

		// Export the point light parameters.
		ColladaLight->SetDropoff(PointLight->FalloffExponent);
		ColladaActor->CreateProperty(TEXT("FalloffExponent"), ColladaLight->GetDropoff().GetAnimated(), &( *ColladaLight->GetDropoff() ));
		ColladaLight->SetLinearAttenuationFactor(1.0f / PointLight->Radius);								   
		ColladaActor->CreateProperty(TEXT("Radius"), ColladaLight->GetLinearAttenuationFactor().GetAnimated(), &( *ColladaLight->GetLinearAttenuationFactor() ));
	}
	else if (BaseLight->IsA(UDirectionalLightComponent::StaticClass()))
	{
		// The directional light has no interesting properties.
		ColladaLight->SetLightType(FCDLight::DIRECTIONAL);
	}
}

void CExporter::ExportCamera( ACameraActor* Actor, USeqAct_Interp* MatineeSequence )
{
	if (ColladaDocument == NULL || Actor == NULL) return;

	// Export the basic actor information.
	CExporterActor* ColladaActor = ExportActor( Actor, MatineeSequence );

	// Choose a name for this actor in the COLLADA file.  If the actor is bound to a Matinee sequence, we'll
	// use the Matinee group name, otherwise we'll just use the actor name.
	FString MyColladaNodeName = Actor->GetName();
	if( MatineeSequence != NULL )
	{
		const UInterpGroupInst* FoundGroupInst = MatineeSequence->FindGroupInst( Actor );
		if( FoundGroupInst != NULL )
		{
			MyColladaNodeName = FoundGroupInst->Group->GroupName.ToString();
		}
	}

	// Create a properly-named COLLADA light structure and instantiate it in the COLLADA scene graph
	FCDCamera* ColladaCamera = ColladaDocument->GetCameraLibrary()->AddEntity();
	ColladaCamera->SetName( *MyColladaNodeName );
	ColladaCamera->SetDaeId(TO_STRING(*Actor->GetName()) + "-object");
	ColladaActor->Instantiate(ColladaCamera);

	// Export the view area information
	ColladaCamera->SetPerspective();
	ColladaCamera->SetFovX(Actor->FOVAngle);
	ColladaActor->CreateProperty(TEXT("FOVAngle"), ColladaCamera->GetFovX().GetAnimated(), &( *ColladaCamera->GetFovX() ));
	ColladaCamera->SetAspectRatio(Actor->AspectRatio);
	ColladaActor->CreateProperty(TEXT("AspectRatio"), ColladaCamera->GetAspectRatio().GetAnimated(), &( *ColladaCamera->GetAspectRatio() ));

	// Push the near/far clip planes away, as the UE3 engine uses larger values than the default.
	ColladaCamera->SetNearZ(10.0f);
	ColladaCamera->SetFarZ(10000.0f);

	// Export the post-processing information: only one of depth-of-field or motion blur can be supported in 3dsMax.
	// And Maya only supports some simple depth-of-field effect.
	FPostProcessSettings* PostProcess = &Actor->CamOverridePostProcess;
	if (PostProcess->bEnableDOF)
	{
		// Export the depth-of-field information
		FCDETechnique* CustomTechnique = ColladaCamera->GetExtra()->GetDefaultType()->AddTechnique(DAE_FCOLLADA_PROFILE);
		FCDENode* MaxDepthOfFieldNode = CustomTechnique->AddChildNode(DAEFC_CAMERA_DEPTH_OF_FIELD_ELEMENT);

		// 'focal depth' <- 'focus distance'.
		if (PostProcess->DOF_FocusType == FOCUS_Distance)
		{
			FCDENode* FocalDepthNode = MaxDepthOfFieldNode->AddParameter(DAEFC_CAMERA_DOF_FOCALDEPTH_PARAMETER, PostProcess->DOF_FocusDistance);
			ColladaActor->CreateProperty(TEXT("DOF_FocusDistance"), FocalDepthNode->GetAnimated(), &FocalDepthNode->GetAnimated()->GetDummy(), 1, NULL);
		}
		else if (PostProcess->DOF_FocusType == FOCUS_Position)
		{
			MaxDepthOfFieldNode->AddParameter(DAEFC_CAMERA_DOF_FOCALDEPTH_PARAMETER, (Actor->Location - PostProcess->DOF_FocusPosition).Size());
		}

		// 'sample radius' <- 10 * 'focus distance' / 'focus radius'.
		// Since 'focus distance' is animatable, this complicates thing: we'll simply scale using the local value.
		FCDConversionSampleRadiusFunctor* SampleRadiusConversion = new FCDConversionSampleRadiusFunctor(PostProcess->DOF_FocusDistance);
		FCDENode* SampleRadiusNode = MaxDepthOfFieldNode->AddParameter(DAEFC_CAMERA_DOF_SAMPLERADIUS_PARAMETER, (*SampleRadiusConversion)(PostProcess->DOF_FocusInnerRadius));
		ColladaActor->CreateProperty(TEXT("DOF_FocusInnerRadius"), SampleRadiusNode->GetAnimated(), &SampleRadiusNode->GetAnimated()->GetDummy(), 1, SampleRadiusConversion);

		// 'tile size' <- 'blur kernel size'.
		FCDENode* TileSizeNode = MaxDepthOfFieldNode->AddParameter(DAEFC_CAMERA_DOF_TILESIZE_PARAMETER, PostProcess->DOF_BlurKernelSize);
		ColladaActor->CreateProperty(TEXT("DOF_BlurKernelSize"), TileSizeNode->GetAnimated(), &TileSizeNode->GetAnimated()->GetDummy(), 1, NULL);

		// 'use target distance'
		// Always set to false, since we'll be overwriting the target distance with the 'focus distance'.
		MaxDepthOfFieldNode->AddParameter(DAEFC_CAMERA_DOF_USETARGETDIST_PARAMETER, false);
	}
	else if (PostProcess->bEnableMotionBlur)
	{
		// Export the motion-blur information
		FCDETechnique* CustomTechnique = ColladaCamera->GetExtra()->GetDefaultType()->AddTechnique(DAEMAX_MAX_PROFILE);
		FCDENode* MotionBlurNode = CustomTechnique->AddChildNode(DAEMAX_CAMERA_MOTIONBLUR_ELEMENT);

		// 'duration' <- 'blur amount'
		FCDENode* BlurAmountNode = MotionBlurNode->AddParameter(DAEMAX_CAMERA_MB_DURATION_PARAMETER, PostProcess->MotionBlur_Amount);
		ColladaActor->CreateProperty(TEXT("MotionBlur_Amount"), BlurAmountNode->GetAnimated(), &BlurAmountNode->GetAnimated()->GetDummy(), 1, NULL);
	}
}

/**
 * Exports the mesh and the actor information for a UE3 brush actor.
 */
void CExporter::ExportBrush(ABrush* Actor)
{
	if (ColladaDocument == NULL || Actor == NULL || Actor->BrushComponent == NULL) return;

	// Retrieve the information structures, verifying the integrity of the data.
	UModel* Model = Actor->BrushComponent->Brush;
	if (Model == NULL || Model->VertexBuffer.Vertices.Num() < 3 || Model->MaterialIndexBuffers.Num() == 0) return;

	// Create the COLLADA actor, the COLLADA geometry and instantiate it.
	CExporterActor* ColladaActor = ExportActor( Actor, NULL );
	FCDGeometry* ColladaGeometry = ColladaDocument->GetGeometryLibrary()->AddEntity();
	ColladaGeometry->SetName(*Actor->GetName());
	ColladaGeometry->SetDaeId(TO_STRING(ColladaGeometry->GetDaeId()) + "-obj");
	ColladaActor->Instantiate(ColladaGeometry);
	FCDGeometryInstance* ColladaInstance = (FCDGeometryInstance*) ColladaActor->ColladaInstance;

	// Export the mesh information
	ExportModel(Model, ColladaGeometry, ColladaInstance);
}

void CExporter::ExportModel(UModel* Model, FCDGeometry* ColladaGeometry, FCDGeometryInstance* ColladaInstance)
{
	INT VertexCount = Model->VertexBuffer.Vertices.Num();
	INT MaterialCount = Model->MaterialIndexBuffers.Num();

	const FLOAT BiasedHalfWorldExtent = HALF_WORLD_MAX * 0.95f;

	// Create the mesh and three data sources for the vertex positions, normals and texture coordinates.
	FCDGeometryMesh* ColladaMesh = ColladaGeometry->CreateMesh();
	FCDGeometrySource* PositionSource = ColladaMesh->AddVertexSource(FUDaeGeometryInput::POSITION);
	FCDGeometrySource* NormalSource = ColladaMesh->AddSource(FUDaeGeometryInput::NORMAL);
	FCDGeometrySource* TexcoordSource = ColladaMesh->AddSource(FUDaeGeometryInput::TEXCOORD);
	PositionSource->SetDaeId(ColladaGeometry->GetDaeId() + "-positions");
	NormalSource->SetDaeId(ColladaGeometry->GetDaeId() + "-normals");
	TexcoordSource->SetDaeId(ColladaGeometry->GetDaeId() + "-texcoords");
	PositionSource->SetStride(3);
	NormalSource->SetStride(3);
	TexcoordSource->SetStride(2);
	PositionSource->SetValueCount( VertexCount );
	NormalSource->SetValueCount( VertexCount );
	TexcoordSource->SetValueCount( VertexCount );
	for (INT VertexIdx = 0; VertexIdx < VertexCount; ++VertexIdx)
	{
		FModelVertex& Vertex = Model->VertexBuffer.Vertices(VertexIdx);
		FVector Normal = (FVector) Vertex.TangentZ;

		// If the vertex is outside of the world extent, snap it to the origin.  The faces associated with
		// these vertices will be removed before exporting.  We leave the snapped vertex in the buffer so
		// we won't have to deal with reindexing everything.
		FVector FinalVertexPos = Vertex.Position;
		if( Abs( Vertex.Position.X ) > BiasedHalfWorldExtent ||
			Abs( Vertex.Position.Y ) > BiasedHalfWorldExtent ||
			Abs( Vertex.Position.Z ) > BiasedHalfWorldExtent )
		{
			FinalVertexPos = FVector( 0.0f, 0.0f, 0.0f );
		}
	
		PositionSource->GetSourceData().set( VertexIdx * 3 + 0, FinalVertexPos.X );
		PositionSource->GetSourceData().set( VertexIdx * 3 + 1, -FinalVertexPos.Y );
		PositionSource->GetSourceData().set( VertexIdx * 3 + 2, FinalVertexPos.Z );
		NormalSource->GetSourceData().set( VertexIdx * 3 + 0, Normal.X );
		NormalSource->GetSourceData().set( VertexIdx * 3 + 1, -Normal.Y );
		NormalSource->GetSourceData().set( VertexIdx * 3 + 2, Normal.Z );
		TexcoordSource->GetSourceData().set( VertexIdx * 2 + 0, Vertex.TexCoord.X );
		TexcoordSource->GetSourceData().set( VertexIdx * 2 + 1, -Vertex.TexCoord.Y );
	}

	// Create the materials and the per-material tesselation structures.
	TMap<UMaterialInterface*,TScopedPointer<FRawIndexBuffer32> >::TIterator MaterialIterator(Model->MaterialIndexBuffers);
	for (; MaterialIterator; ++MaterialIterator)
	{
		UMaterialInterface* MaterialInterface = MaterialIterator.Key();
		FRawIndexBuffer32& IndexBuffer = *MaterialIterator.Value();
		INT IndexCount = IndexBuffer.Indices.Num();
		if (IndexCount < 3) continue;

		// Create the COLLADA polygons set.
		FCDGeometryPolygons* ColladaPolygons = ColladaMesh->AddPolygons();

		// Add normal and texcoord source inputs to the per-vertex input.
		// Since the position source is a per-vertex source, it is automatically added
		// to the per-vertex input that is automatically created for the polygon set.
		ColladaPolygons->AddInput(NormalSource, 0);
		ColladaPolygons->AddInput(TexcoordSource, 0);

		// Retrieve and fill in the index buffer.
		UInt32List Indices;
		const INT TriangleCount = IndexCount / 3;
		for( INT TriangleIdx = 0; TriangleIdx < TriangleCount; ++TriangleIdx )
		{
			UBOOL bSkipTriangle = FALSE;

			for( INT IndexIdx = 0; IndexIdx < 3; ++IndexIdx )
			{
				// Skip triangles that belong to BSP geometry close to the world extent, since its probably
				// the automatically-added-brush for new levels.  The vertices will be left in the buffer (unreferenced)
				FVector VertexPos = Model->VertexBuffer.Vertices( IndexBuffer.Indices( TriangleIdx * 3 + IndexIdx ) ).Position;
				if( Abs( VertexPos.X ) > BiasedHalfWorldExtent ||
					Abs( VertexPos.Y ) > BiasedHalfWorldExtent ||
					Abs( VertexPos.Z ) > BiasedHalfWorldExtent )
				{
					bSkipTriangle = TRUE;
					break;
				}
			}

			if( !bSkipTriangle )
			{
				for( INT IndexIdx = 0; IndexIdx < 3; ++IndexIdx )
				{
					Indices.push_back( IndexBuffer.Indices( TriangleIdx * 3 + IndexIdx ) );
				}
				ColladaPolygons->AddFaceVertexCount( 3 );
			}
		}
		ColladaPolygons->FindInput( PositionSource )->AddIndices( Indices );

		// Are NULL materials okay?
		if (MaterialInterface != NULL && MaterialInterface->GetMaterial(MSP_BASE) != NULL)
		{
			FCDMaterial* ColladaMaterial = ExportMaterial(MaterialInterface->GetMaterial(MSP_BASE));
			ColladaPolygons->SetMaterialSemantic(ColladaMaterial->GetName());
			ColladaInstance->AddMaterialInstance(ColladaMaterial, ColladaPolygons);
		}
		else
		{
			// Set no material: Maya/3dsMax will assign some default material.
		}
	}
}

void CExporter::ExportStaticMesh( AActor* Actor, UStaticMeshComponent* StaticMeshComponent, USeqAct_Interp* MatineeSequence )
{
	if (ColladaDocument == NULL || Actor == NULL || StaticMeshComponent == NULL) return;

	// Retrieve the static mesh rendering information at the correct LOD level.
	UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh;
	if (StaticMesh == NULL || StaticMesh->LODModels.Num() == 0) return;
	INT LODIndex = StaticMeshComponent->ForcedLodModel;
	if (LODIndex >= StaticMesh->LODModels.Num())
	{
		LODIndex = StaticMesh->LODModels.Num() - 1;
	}
	FStaticMeshRenderData& RenderMesh = StaticMesh->LODModels(LODIndex);

	// Choose a name for this actor in the COLLADA file.  If the actor is bound to a Matinee sequence, we'll
	// use the Matinee group name, otherwise we'll just use the actor name.
	FString MyColladaNodeName = Actor->GetName();
	if( MatineeSequence != NULL )
	{
		const UInterpGroupInst* FoundGroupInst = MatineeSequence->FindGroupInst( Actor );
		if( FoundGroupInst != NULL )
		{
			MyColladaNodeName = FoundGroupInst->Group->GroupName.ToString();
		}
	}

	// Export the render mesh as a COLLADA geometry.
	// This handles the duplication of static meshes.
	CStaticMesh StaticMeshExporter(this);
	FCDGeometry* ColladaGeometry = StaticMeshExporter.ExportStaticMesh(RenderMesh, *MyColladaNodeName);
	if (ColladaGeometry == NULL) return;

	// Create the COLLADA actor for this UE actor and instantiate it.
	CExporterActor* ColladaActor = ExportActor( Actor, MatineeSequence );
	ColladaActor->Instantiate(ColladaGeometry);
	FCDGeometryInstance* ColladaGeometryInstance = (FCDGeometryInstance*) ColladaActor->ColladaInstance;
	FCDGeometryMesh* ColladaMesh = ColladaGeometry->GetMesh();

	// Generate the materials and links for this COLLADA geometry.
	size_t ColladaPolygonsCount = ColladaMesh->GetPolygonsCount();
	for (size_t PolygonsIndex = 0; PolygonsIndex < ColladaPolygonsCount; ++PolygonsIndex)
	{
		FCDGeometryPolygons* ColladaPolygons = ColladaMesh->GetPolygons(PolygonsIndex);

		// Retrieve the static mesh instance's material instance for this polygons set.
		UMaterialInterface* MaterialInterface = StaticMeshComponent->GetMaterial((INT) PolygonsIndex);
		if (MaterialInterface == NULL) continue;
		FCDMaterial* ColladaMaterial = ExportMaterial(MaterialInterface->GetMaterial(MSP_BASE));

		// Attach the material and the polygons set.
		UNUSED(FCDMaterialInstance* ColladaMaterialInstance = )
			ColladaGeometryInstance->AddMaterialInstance(ColladaMaterial, ColladaPolygons);
	}
}

/**
* Exports the profile_COMMON information for a UE3 material.
*/
FCDMaterial* CExporter::ExportMaterial(UMaterial* Material)
{
	if (ColladaDocument == NULL || Material == NULL) return NULL;

	// Verify that this material has not already been exported:
	// the FCDMaterial objects hold a pointer to the equivalent UMaterial object.
	size_t MaterialCount = ColladaDocument->GetMaterialLibrary()->GetEntityCount();
	for (size_t Idx = 0; Idx < MaterialCount; ++Idx)
	{
		FCDMaterial* ColladaMaterial = ColladaDocument->GetMaterialLibrary()->GetEntity(Idx);
		if (ColladaMaterial->GetUserHandle() == Material)
		{
			return ColladaMaterial;
		}
	}

	// Create the COLLADA material and effect structures
	FCDMaterial* ColladaMaterial = ColladaDocument->GetMaterialLibrary()->AddEntity();
	ColladaMaterial->SetName(*Material->GetName());
	ColladaMaterial->SetDaeId(TO_STRING(ColladaMaterial->GetName()));
	FCDEffect* ColladaEffect = ColladaDocument->GetEffectLibrary()->AddEntity();
	ColladaEffect->SetName(*Material->GetName());
	ColladaEffect->SetDaeId(TO_STRING(ColladaEffect->GetDaeId() + "-fx"));
	ColladaMaterial->SetEffect(ColladaEffect);
	FCDEffectStandard* ColladaStdEffect = (FCDEffectStandard*) ColladaEffect->AddProfile(FUDaeProfileType::COMMON);

	// Set the lighting model
	if (Material->LightingModel == MLM_Phong || Material->LightingModel == MLM_Custom || Material->LightingModel == MLM_SHPRT)
	{
		ColladaStdEffect->SetLightingType(FCDEffectStandard::PHONG);
	}
	else if (Material->LightingModel == MLM_NonDirectional)
	{
		ColladaStdEffect->SetLightingType(FCDEffectStandard::LAMBERT);
	}
	else if (Material->LightingModel == MLM_Unlit)
	{
		ColladaStdEffect->SetLightingType(FCDEffectStandard::CONSTANT);
	}

	// Fill in the profile_COMMON effect with the UE3 material information.
	// TODO: Look for textures/constants in the Material expressions...
	ColladaStdEffect->SetDiffuseColor(ToFMVector4(Material->DiffuseColor.Constant));
	ColladaStdEffect->SetEmissionColor(ToFMVector4(Material->EmissiveColor.Constant));
	ColladaStdEffect->SetSpecularColor(ToFMVector4(Material->SpecularColor.Constant));
	ColladaStdEffect->SetShininess(Material->SpecularPower.Constant);
	ColladaStdEffect->SetTranslucencyFactor(Material->Opacity.Constant);
	return ColladaMaterial;
}

/**
 * Exports the given Matinee sequence information into a COLLADA document.
 */
void CExporter::ExportMatinee(USeqAct_Interp* MatineeSequence)
{
	if (MatineeSequence == NULL || ColladaDocument == NULL) return;

	// If the Matinee editor is not open, we need to initialize the sequence.
	UBOOL InitializeMatinee = MatineeSequence->InterpData == NULL;
	if (InitializeMatinee)
	{
		MatineeSequence->InitInterp();
	}

	// Iterate over the Matinee data groups and export the known tracks
	INT GroupCount = MatineeSequence->GroupInst.Num();
	for (INT GroupIndex = 0; GroupIndex < GroupCount; ++GroupIndex)
	{
		UInterpGroupInst* Group = MatineeSequence->GroupInst(GroupIndex);
		AActor* Actor = Group->GetGroupActor();
		if (Group->Group == NULL || Actor == NULL) continue;
		CExporterActor* ColladaActor = ExportActor( Actor, MatineeSequence );

		// Look for the class-type of the actor.
		if (Actor->IsA(ACameraActor::StaticClass()))
		{
			ExportCamera( (ACameraActor*) Actor, MatineeSequence );
		}

		// Look for the tracks that we currently support
		INT TrackCount = min(Group->TrackInst.Num(), Group->Group->InterpTracks.Num());
		for (INT TrackIndex = 0; TrackIndex < TrackCount; ++TrackIndex)
		{
			// TODO: Figure out the right way to identify the track and what they affect.
			UInterpTrackInst* TrackInst = Group->TrackInst(TrackIndex);
			UInterpTrack* Track = Group->Group->InterpTracks(TrackIndex);
			if (TrackInst->IsA(UInterpTrackInstMove::StaticClass()) && Track->IsA(UInterpTrackMove::StaticClass()))
			{
				UInterpTrackInstMove* MoveTrackInst = (UInterpTrackInstMove*) TrackInst;
				UInterpTrackMove* MoveTrack = (UInterpTrackMove*) Track;
				ExportMatineeTrackMove(ColladaActor, MoveTrackInst, MoveTrack);
			}
			else if (TrackInst->IsA(UInterpTrackInstFloatProp::StaticClass()) && Track->IsA(UInterpTrackFloatProp::StaticClass()))
			{
				UInterpTrackInstFloatProp* PropertyTrackInst = (UInterpTrackInstFloatProp*) TrackInst;
				UInterpTrackFloatProp* PropertyTrack = (UInterpTrackFloatProp*) Track;
				ExportMatineeTrackFloatProp(ColladaActor, PropertyTrack);
			}
		}
	}

	if (InitializeMatinee)
	{
		MatineeSequence->TermInterp();
	}
}

/**
 * Writes out the asset information about the UE3 level and user in the COLLADA document.
 */
void CExporter::ExportAsset(FCDAsset* ColladaAsset)
{
	if (ColladaDocument == NULL || ColladaAsset == NULL) return;

	// Add a new contributor to the asset and fill it up with the tool/user information
	FCDAssetContributor* Contributor = ColladaAsset->AddContributor();

	// Set the author's information
	TCHAR Username[128];
	appGetEnvironmentVariable(TEXT("USER"), Username, 128);
	if (Username[0] == 0) appGetEnvironmentVariable(TEXT("USERNAME"), Username, 128);
	Contributor->SetAuthor(Username);
	Contributor->SetAuthoringTool(TEXT("UE3 Editor")); // TODO: Add some versioning information here.

	// Set the up axis information which is crucial to get the DCC tools to accept the data.
	ColladaAsset->SetUpAxis(FMVector3::ZAxis);


#if 0
	// Set the unit scale.  Because unreal units are very small (1 UU == 0.0127 m) and we often want
	// to export entire levels (Matinee curve editing, for example), we'll apply a unit scale so
	// that the scene is more manageable inside of the external art tool.
	const FLOAT UnrealUnitInMeters = 0.0127f;
//	ColladaAsset->SetUnitName( TEXT( "meter" ) );
	ColladaAsset->SetUnitConversionFactor( UnrealUnitInMeters );
#endif
}


/**
 * Exports a scene node with the placement indicated by a given UE3 actor.
 * This scene node will always have four transformations: one translation vector and three Euler rotations.
 * The transformations will appear in the order: {translation, rot Z, rot Y, rot X} as per the COLLADA specifications.
 */
CExporterActor* CExporter::ExportActor(AActor* Actor, USeqAct_Interp* MatineeSequence )
{
	// Verify that this actor isn't already exported, create a structure for it
	// and buffer it.
	CExporterActor* ColladaActor = FindActor(Actor);
	if (ColladaActor == NULL)
	{
		ColladaActor = new CExporterActor();
		ColladaActor->Actor = Actor;
		ColladaActors.AddItem(ColladaActor);
	}

	// Create a visual scene node in the COLLADA document for this actor.
	if (ColladaActor->ColladaNode == NULL)
	{
		// Retrieve or create the visual scene root.
		FCDSceneNode* ColladaVisualScene = ColladaDocument->GetVisualSceneRoot();
		if (ColladaVisualScene == NULL)
		{
			ColladaVisualScene = ColladaDocument->AddVisualScene();
		}

		// Choose a name for this actor in the COLLADA file.  If the actor is bound to a Matinee sequence, we'll
		// use the Matinee group name, otherwise we'll just use the actor name.
		FString MyColladaNodeName = Actor->GetName();
		if( MatineeSequence != NULL )
		{
			const UInterpGroupInst* FoundGroupInst = MatineeSequence->FindGroupInst( Actor );
			if( FoundGroupInst != NULL )
			{
				MyColladaNodeName = FoundGroupInst->Group->GroupName.ToString();
			}
		}

		// Create a properly named visual scene node.
		FCDSceneNode* ColladaNode = ColladaVisualScene->AddChildNode();
		ColladaNode->SetName( *MyColladaNodeName );
		ColladaNode->SetDaeId(TO_STRING(*Actor->GetName()));
		ColladaActor->ColladaNode = ColladaNode;

		// Set the default position of the actor on the transforms
		// The UE3 transformation is different from COLLADA's Z-up: invert the Y-axis for translations and the Y/Z angle values in rotations.
		FCDTTranslation* ColladaPosition = (FCDTTranslation*) ColladaNode->AddTransform(FCDTransform::TRANSLATION);
		FVector InitialPos = Actor->Location;
		FVector InitialRot = Actor->Rotation.Euler();
		ColladaPosition->SetTranslation(ToFMVector3(InitialPos));

		// Rotation
		FCDTRotation* ColladaRotationZ = (FCDTRotation*) ColladaNode->AddTransform(FCDTransform::ROTATION);
		FCDTRotation* ColladaRotationY = (FCDTRotation*) ColladaNode->AddTransform(FCDTransform::ROTATION);
		FCDTRotation* ColladaRotationX = (FCDTRotation*) ColladaNode->AddTransform(FCDTransform::ROTATION);
		ColladaRotationX->SetRotation(FMVector3::XAxis, InitialRot.X);
		ColladaRotationY->SetRotation(FMVector3::YAxis, -InitialRot.Y);
		ColladaRotationZ->SetRotation(FMVector3::ZAxis, -InitialRot.Z);

		// Actor scaling
		// NOTE: We don't use ToFMVector3 to convert the DrawScale; inverting scale components isn't needed
		const FVector ActorScale = Actor->DrawScale * Actor->DrawScale3D;
		FCDTScale* ColladaScale = ( FCDTScale* )ColladaNode->AddTransform( FCDTransform::SCALE );
		ColladaScale->SetScale( FMVector3( ActorScale.X, ActorScale.Y, ActorScale.Z ) );
	}

	return ColladaActor;
}

/**
 * Exports the Matinee movement track into the COLLADA animation library.
 */
void CExporter::ExportMatineeTrackMove(CExporterActor* ColladaActor, UInterpTrackInstMove* MoveTrackInst, UInterpTrackMove* MoveTrack)
{
	if (ColladaActor == NULL || MoveTrack == NULL) return;
	if (ColladaActor->Actor == NULL || ColladaActor->ColladaNode == NULL) return;
	FCDSceneNode* ColladaNode = ColladaActor->ColladaNode;

	// Retrieve the necessary transforms in the COLLADA scene node to express the UE placement.
    FCDTTranslation* ColladaPosition = (FCDTTranslation*) ColladaNode->GetTransform(0);
	FCDTRotation* ColladaRotationZ = (FCDTRotation*) ColladaNode->GetTransform(1);
	FCDTRotation* ColladaRotationY = (FCDTRotation*) ColladaNode->GetTransform(2);
	FCDTRotation* ColladaRotationX = (FCDTRotation*) ColladaNode->GetTransform(3);


	// For the Y and Z angular rotations, we need to invert the relative animation frames,
	// While keeping the standard angles constant.

	if (MoveTrack != NULL)
	{
		FCDConversionScaleOffsetFunctor Conversion;

		{
			// Name the animated tracks
			ColladaPosition->SetSubId("translate");

			// Export the movement track in the animation curves of these COLLADA transforms.
			// The UE3 transformation is different from COLLADA's Z-up.
			fstring PosTrackName = ColladaNode->GetName() + FC(".translate");

			const UBOOL bPosCurve = TRUE;

			FCDAnimated* ColladaAnimatedPosition = ColladaPosition->GetTranslation().GetAnimated();
			ExportAnimatedVector(PosTrackName.c_str(), ColladaAnimatedPosition, 0, MoveTrack, MoveTrackInst, bPosCurve, 0, &Conversion.Set(1.0f, 0.0f));
			ExportAnimatedVector(PosTrackName.c_str(), ColladaAnimatedPosition, 1, MoveTrack, MoveTrackInst, bPosCurve, 1, &Conversion.Set(-1.0f, 0.0f));
			ExportAnimatedVector(PosTrackName.c_str(), ColladaAnimatedPosition, 2, MoveTrack, MoveTrackInst, bPosCurve, 2, &Conversion.Set(1.0f, 0.0f));
		}

		{
			// Name the animated tracks
			ColladaRotationX->SetSubId("rotateX");
			ColladaRotationY->SetSubId("rotateY");
			ColladaRotationZ->SetSubId("rotateZ");

			const UBOOL bPosCurve = FALSE;

			fstring RotXTrackName = ColladaNode->GetName() + FC(".rotateX");
			FCDAnimated* ColladaAnimatedRotationX = ColladaRotationX->GetAnimated();
			ExportAnimatedVector(RotXTrackName.c_str(), ColladaAnimatedRotationX, 3, MoveTrack, MoveTrackInst, bPosCurve, 0, &Conversion.Set(1.0f, 0.0f));

			fstring RotYTrackName = ColladaNode->GetName() + FC(".rotateY");
			FCDAnimated* ColladaAnimatedRotationY = ColladaRotationY->GetAnimated();
			ExportAnimatedVector(RotYTrackName.c_str(), ColladaAnimatedRotationY, 3, MoveTrack, MoveTrackInst, bPosCurve, 1, &Conversion.Set(-1.0f, 0.0f));

			fstring RotZTrackName = ColladaNode->GetName() + FC(".rotateZ");
			FCDAnimated* ColladaAnimatedRotationZ = ColladaRotationZ->GetAnimated();
			ExportAnimatedVector(RotZTrackName.c_str(), ColladaAnimatedRotationZ, 3, MoveTrack, MoveTrackInst, bPosCurve, 2, &Conversion.Set(-1.0f, 0.0f));
		}
	}
}

/**
 * Exports the Matinee float property track into the COLLADA animation library.
 */
void CExporter::ExportMatineeTrackFloatProp(CExporterActor* ColladaActor, UInterpTrackFloatProp* PropTrack)
{
	if (ColladaActor == NULL || PropTrack == NULL) return;
	if (ColladaActor->Actor == NULL || ColladaActor->ColladaEntity == NULL) return;

	// Retrieve the property linking structure
	CExporterAnimatableProperty* Property = ColladaActor->FindProperty(*PropTrack->PropertyName.GetNameString());
	if (Property != NULL)
	{
		fstring TrackName = ColladaActor->ColladaNode->GetName() + TEXT("-") + *Property->PropertyName;
		ExportAnimatedFloat(TrackName.c_str(), Property->ColladaAnimated, &PropTrack->FloatTrack, Property->Conversion);
	}
}

/**
 * Exports the Matinee vector property track into the COLLADA animation library.
 */
void CExporter::ExportMatineeTrackVectorProp(CExporterActor* ColladaActor, UInterpTrackVectorProp* PropTrack)
{
	if (ColladaActor == NULL || PropTrack == NULL) return;
	if (ColladaActor->Actor == NULL || ColladaActor->ColladaEntity == NULL) return;

	// TODO: Find a vector property that is useful to export.
}

/**
 * Exports a given interpolation curve into the COLLADA animation curve.
 */
void CExporter::ExportAnimatedVector(const TCHAR* Name, FCDAnimated* ColladaAnimated, INT ColladaAnimatedIndex, UInterpTrackMove* MoveTrack, UInterpTrackInstMove* MoveTrackInst, UBOOL bPosCurve, INT CurveIndex, FCDConversionFunctor* Conversion)
{
	if (ColladaDocument == NULL) return;
	if (ColladaAnimated == NULL || ColladaAnimatedIndex >= (INT) ColladaAnimated->GetValueCount()) return;

	FInterpCurveVector* Curve = bPosCurve ? &MoveTrack->PosTrack : &MoveTrack->EulerTrack;

	if (Curve == NULL || CurveIndex >= 3) return;

	// Create the animation entity for this curve and the animation curve.
	fstring BaseName = fstring(Name) + TO_FSTRING(ColladaAnimated->GetQualifier(ColladaAnimatedIndex));
	FCDAnimation* ColladaAnimation = ColladaDocument->GetAnimationLibrary()->AddEntity();
	ColladaAnimation->SetDaeId(TO_STRING(BaseName));
	FCDAnimationChannel* ColladaChannel = ColladaAnimation->AddChannel();
	FCDAnimationCurve* ColladaCurve = ColladaChannel->AddCurve();
	ColladaAnimated->AddCurve(ColladaAnimatedIndex, ColladaCurve);

	// Write out the key times from the UE3 curve to the COLLADA curve.
	INT KeyCount = Curve->Points.Num();
	FloatList KeyTimes;
	KeyTimes.reserve(KeyCount);
	for (INT KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
	{
		FInterpCurvePoint<FVector>& Key = Curve->Points(KeyIndex);

		// The Unreal engine allows you to place more than one key at one time value:
		// displace the extra keys. This assumes that Unreal's keys are always ordered.
		FLOAT KeyTime = Key.InVal;
		if (!KeyTimes.empty() && KeyTime < KeyTimes.back() + FLT_TOLERANCE)
		{
			KeyTime = KeyTimes.back() + 0.01f; // Add 1 millisecond to the timing of this key.
		}
		KeyTimes.push_back(KeyTime);
	}

	// Write out the key values from the UE3 curve to the COLLADA curve.
	for (INT KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
	{
		// First, convert the output value to the correct coordinate system, if we need that.  For movement
		// track keys that are in a local coordinate system (IMF_RelativeToInitial), we need to transform
		// the keys to world space first
		FVector FinalOutVec;
		{
			FInterpCurvePoint<FVector>& PosKey = MoveTrack->PosTrack.Points(KeyIndex);
			FInterpCurvePoint<FVector>& RotKey = MoveTrack->EulerTrack.Points(KeyIndex);

			FVector WorldSpacePos;
			FRotator WorldSpaceRotator;
			MoveTrack->ComputeWorldSpaceKeyTransform(
				MoveTrackInst,
				PosKey.OutVal,
				FRotator( FQuat::MakeFromEuler( RotKey.OutVal ) ),
				WorldSpacePos,			// Out
				WorldSpaceRotator );	// Out

			if( bPosCurve )
			{
				FinalOutVec = WorldSpacePos;
			}
			else
			{
				FinalOutVec = WorldSpaceRotator.Euler();
			}
		}

		FLOAT KeyTime = KeyTimes[KeyIndex];
		FLOAT OutValue = (CurveIndex == 0) ? FinalOutVec.X : (CurveIndex == 1) ? FinalOutVec.Y : FinalOutVec.Z;

		FInterpCurvePoint<FVector>& Key = Curve->Points(KeyIndex);

		// Add a new key to the COLLADA curve
		FCDAnimationKeyBezier* NewColladaKey =
			static_cast< FCDAnimationKeyBezier* >( ColladaCurve->AddKey( FUDaeInterpolation::BEZIER ) );

		// Set key time and value
		NewColladaKey->input = KeyTime;
		NewColladaKey->output = OutValue;

		// Calculate the in/out control points and write them out to the COLLADA curve.
		FLOAT InTangentValue = (CurveIndex == 0) ? Key.ArriveTangent.X : (CurveIndex == 1) ? Key.ArriveTangent.Y : Key.ArriveTangent.Z;
		FLOAT OutTangentValue = (CurveIndex == 0) ? Key.LeaveTangent.X : (CurveIndex == 1) ? Key.LeaveTangent.Y : Key.LeaveTangent.Z;

		// Calculate the time values for the control points.
		FLOAT InTangentX = (KeyIndex > 0) ? (2.0f * KeyTime + KeyTimes[KeyIndex - 1]) / 3.0f : (KeyTime - 0.333f);
		FLOAT OutTangentX = (KeyIndex < KeyCount - 1) ? (KeyTimes[KeyIndex + 1] + 2.0f * KeyTime) / 3.0f : (KeyTime + 0.333f);
		if (IsEquivalent(OutTangentX, KeyTime))
		{
			OutTangentX = KeyTime + 0.00333f; // 1/3rd of a millisecond.
		}

		// Convert the tangent values from Hermite to Bezier control points.
		FLOAT InTangentY = NewColladaKey->output - InTangentValue / 3.0f;
		FLOAT OutTangentY = NewColladaKey->output + OutTangentValue / 3.0f;

		NewColladaKey->inTangent = FMVector2(InTangentX, InTangentY);
		NewColladaKey->outTangent = FMVector2(OutTangentX, OutTangentY);
	}

	// Convert the values. This is useful for inversions and negative scales.
	ColladaCurve->ConvertValues(Conversion, Conversion);
}

void CExporter::ExportAnimatedFloat(const TCHAR* Name, FCDAnimated* ColladaAnimated, FInterpCurveFloat* Curve, FCDConversionFunctor* Conversion)
{
	if (ColladaAnimated == NULL || Curve == NULL || ColladaDocument == NULL) return;

	// Create the animation entity for this curve and the animation curve.
	FCDAnimation* ColladaAnimation = ColladaDocument->GetAnimationLibrary()->AddEntity();
	ColladaAnimation->SetDaeId(TO_STRING((const fchar*) Name));
	FCDAnimationChannel* ColladaChannel = ColladaAnimation->AddChannel();
	FCDAnimationCurve* ColladaCurve = ColladaChannel->AddCurve();
	ColladaAnimated->AddCurve(0, ColladaCurve);

	// Write out the key times from the UE3 curve to the COLLADA curve.
	INT KeyCount = Curve->Points.Num();
	FloatList KeyTimes;
	KeyTimes.reserve(KeyCount);
	for (INT KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
	{
		FInterpCurvePoint<FLOAT>& Key = Curve->Points(KeyIndex);

		// The Unreal engine allows you to place more than one key at one time value:
		// displace the extra keys. This assumes that Unreal's keys are always ordered.
		FLOAT KeyTime = Key.InVal;
		if (!KeyTimes.empty() && KeyTime < KeyTimes.back() + FLT_TOLERANCE)
		{
			KeyTime = KeyTimes.back() + 0.01f; // Add 1 millisecond to the timing of this key.
		}
		KeyTimes.push_back(KeyTime);
	}

	// Write out the key values from the UE3 curve to the COLLADA curve.
	for (INT KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
	{
		FInterpCurvePoint<FLOAT>& Key = Curve->Points(KeyIndex);
		FLOAT KeyTime = KeyTimes[KeyIndex];

		// Write out the correct interpolation type.
		FUDaeInterpolation::Interpolation KeyType;
		switch (Key.InterpMode)
		{
		case CIM_Constant: 
			KeyType = FUDaeInterpolation::STEP;
			break;
		case CIM_Linear: 
			KeyType = FUDaeInterpolation::LINEAR;
			break;
		default: 
			KeyType = FUDaeInterpolation::BEZIER;
		}

		// Add a new key to the COLLADA curve
		FCDAnimationKey* NewColladaKey = ColladaCurve->AddKey( KeyType );

		// Set key time and value
		NewColladaKey->input = KeyTime;
		NewColladaKey->output = Key.OutVal;

		if( KeyType == FUDaeInterpolation::BEZIER )
		{
			FCDAnimationKeyBezier* BezierKey = static_cast< FCDAnimationKeyBezier* >( NewColladaKey );

			// Calculate the time values for the control points.
			FLOAT InTangentX = (KeyIndex > 0) ? (2.0f * KeyTime + KeyTimes[KeyIndex - 1]) / 3.0f : (KeyTime - 0.333f);
			FLOAT OutTangentX = (KeyIndex < KeyCount - 1) ? (KeyTimes[KeyIndex + 1] + 2.0f * KeyTime) / 3.0f : (KeyTime + 0.333f);

			// Convert the tangent values from Hermite to Bezier control points.
			FLOAT InTangentY = Key.OutVal - Key.ArriveTangent / 3.0f;
			FLOAT OutTangentY = Key.OutVal + Key.LeaveTangent / 3.0f;

			BezierKey->inTangent = FMVector2(InTangentX, InTangentY);
			BezierKey->outTangent = FMVector2(OutTangentX, OutTangentY);
		}
	}

	// Convert the values. This is useful for inversions and negative scales.
	ColladaCurve->ConvertValues(Conversion, Conversion);
}

/**
 * Finds the given UE3 actor in the already-exported list of structures
 */
CExporterActor* CExporter::FindActor(AActor* Actor)
{
	INT Count = ColladaActors.Num();
	for (INT Idx = 0; Idx < Count; ++Idx)
	{
		CExporterActor* ColladaActor = ColladaActors(Idx);
		if (ColladaActor->Actor == Actor)
		{
			return ColladaActor;
		}
	}
	return NULL;
}

};

