/*=============================================================================
SpeedTreeVertexFactory.cpp: SpeedTree vertex factory implementation.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "SpeedTree.h"

#if WITH_SPEEDTREE

extern CSpeedTreeRT::SGeometry GSpeedTreeGeometry;

/**
 * Calculates the crossfade opacity modulator for a SpeedTree LOD.
 * @param MeshUserData - The LOD info for the mesh being drawn.
 * @param View - The view being drawn.
 * @return A percentage to modulate the LOD's opacity with.
 */
static FLOAT CalcLodFadeOpacity(const FSpeedTreeVertexFactory::MeshUserDataType& MeshUserData,const FSceneView& View)
{
	// Compute the distance between the camera and the tree.
	FLOAT LODDistance = 0.0f;
	if(View.ViewOrigin.W > 0.0f)
	{
		LODDistance = ((FVector)View.ViewOrigin - MeshUserData.BoundsOrigin).Size() * View.LODDistanceFactor;
	}

	// Compute a linear fade for the LOD.
	FLOAT LinearFadeOpacity = 1.0f;
	if(LODDistance < MeshUserData.LodMinDistance)
	{
		LinearFadeOpacity = Max(0.0f,1.0f - (MeshUserData.LodMinDistance - LODDistance) / MeshUserData.LodFadeRadius);
	}
	else if(LODDistance > MeshUserData.LodMaxDistance)
	{
		LinearFadeOpacity = Max(0.0f,1.0f - (LODDistance - MeshUserData.LodMaxDistance) / MeshUserData.LodFadeRadius);
	}

	// Apply a non-linear ramp to the fade.
	const FLOAT FadeExponent = 0.7f;
	return appPow(LinearFadeOpacity,FadeExponent);
}

UBOOL FSpeedTreeVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return Material->IsUsedWithSpeedTree() || Material->IsSpecialEngineMaterial();
}

void FSpeedTreeVertexFactory::InitRHI()
{
	FVertexDeclarationElementList Elements;

	Elements.AddItem(AccessStreamComponent(Data.PositionComponent, VEU_Position));

	if( Data.WindInfo.VertexBuffer )
		Elements.AddItem(AccessStreamComponent(Data.WindInfo, VEU_BlendIndices));

	EVertexElementUsage TangentBasisUsages[3] = { VEU_Tangent, VEU_Binormal, VEU_Normal };
	for( INT AxisIndex=0; AxisIndex<3; AxisIndex++ )
	{
		if( Data.TangentBasisComponents[AxisIndex].VertexBuffer != NULL )
		{
			Elements.AddItem(AccessStreamComponent(Data.TangentBasisComponents[AxisIndex], TangentBasisUsages[AxisIndex]));
		}
	}

	if( Data.TextureCoordinates.Num() )
	{
		for( UINT CoordinateIndex=0; CoordinateIndex<Data.TextureCoordinates.Num(); CoordinateIndex++ )
		{
			Elements.AddItem(AccessStreamComponent(Data.TextureCoordinates(CoordinateIndex),VEU_TextureCoordinate,CoordinateIndex));
		}

		for( UINT CoordinateIndex=Data.TextureCoordinates.Num(); CoordinateIndex<MAX_TEXCOORDS; CoordinateIndex++ )
		{
			Elements.AddItem(AccessStreamComponent(Data.TextureCoordinates(Data.TextureCoordinates.Num()-1),VEU_TextureCoordinate,CoordinateIndex));
		}
	}

	if( Data.ShadowMapCoordinateComponent.VertexBuffer )
	{
		Elements.AddItem(AccessStreamComponent(Data.ShadowMapCoordinateComponent, VEU_Color));
	}
	else if( Data.TextureCoordinates.Num() )
	{
		Elements.AddItem(AccessStreamComponent(Data.TextureCoordinates(0), VEU_Color));
	}

	InitDeclaration(Elements, FVertexFactory::DataType(), TRUE, TRUE);
}

class FSpeedTreeVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		LocalToWorldParameter.Bind( ParameterMap, TEXT("LocalToWorld") );
		WorldToLocalParameter.Bind( ParameterMap, TEXT("WorldToLocal"), TRUE );
		AlphaAdjustmentParameter.Bind( ParameterMap, TEXT("LODAlphaAdjustment"), TRUE );
		WindMatrixOffsetParameter.Bind( ParameterMap, TEXT("WindMatrixOffset"), TRUE );
		WindMatricesParameter.Bind( ParameterMap, TEXT("WindMatrices"), TRUE );
		RotationOnlyMatrixParameter.Bind( ParameterMap, TEXT("RotationOnlyMatrix"), TRUE );
		LeafRockAnglesParameter.Bind( ParameterMap, TEXT("LeafRockAngles"), TRUE );
		LeafRustleAnglesParameter.Bind( ParameterMap, TEXT("LeafRustleAngles"), TRUE );
		CameraAlignMatrixParameter.Bind( ParameterMap, TEXT("CameraAlignMatrix"), TRUE );
		LeafAngleScalarsParameter.Bind( ParameterMap, TEXT("LeafAngleScalars"), TRUE );
	}

	virtual void Serialize(FArchive& Ar)
	{
		Ar << LocalToWorldParameter;
		Ar << WorldToLocalParameter;
		Ar << AlphaAdjustmentParameter;
		Ar << WindMatrixOffsetParameter;
		Ar << WindMatricesParameter;
		Ar << RotationOnlyMatrixParameter;
		Ar << LeafRockAnglesParameter;
		Ar << LeafRustleAnglesParameter;
		Ar << CameraAlignMatrixParameter;
		Ar << LeafAngleScalarsParameter;
	}

	virtual void Set(FShader* VertexShader, const FVertexFactory* VertexFactory, const FSceneView& View) const
	{
		FSpeedTreeVertexFactory* SpeedTreeVertexFactory = (FSpeedTreeVertexFactory*)VertexFactory;
		const USpeedTree* SpeedTree = SpeedTreeVertexFactory->GetSpeedTree();
		check(SpeedTree);
		check(SpeedTree->SRH);

		if(LeafRockAnglesParameter.IsBound() || LeafRustleAnglesParameter.IsBound() || WindMatricesParameter.IsBound())
		{
			// Update wind if time has passed.
			SpeedTree->SRH->UpdateWind(FVector(1.0f, 0.0f, 0.0f), 0.5f, View.Family->CurrentWorldTime);

			// Read the leaf rock/rustle angles.
			FLOAT LeafRockAngles[3];
			FLOAT LeafRustleAngles[3];
			SpeedTree->SRH->SpeedWind.GetRockAngles(LeafRockAngles);
			SpeedTree->SRH->SpeedWind.GetRustleAngles(LeafRustleAngles);

			// Convert degrees to radians
			for(UINT AngleIndex = 0;AngleIndex < 3;++AngleIndex)
			{
				LeafRockAngles[AngleIndex] = LeafRockAngles[AngleIndex] * PI / 180.0f;
				LeafRustleAngles[AngleIndex] = LeafRustleAngles[AngleIndex] * PI / 180.0f;
			}

			// Set the leaf rock/rustle parameters
			SetVertexShaderValue(  VertexShader->GetVertexShader(), LeafRockAnglesParameter, *(FVector*)&LeafRockAngles );
			SetVertexShaderValue(  VertexShader->GetVertexShader(), LeafRustleAnglesParameter, *(FVector*)&LeafRustleAngles );
			SetVertexShaderValue(  VertexShader->GetVertexShader(), LeafAngleScalarsParameter, SpeedTree->SRH->LeafAngleScalars );

			if( WindMatricesParameter.IsBound() )
			{
				FMatrix* WindMatrices = (FMatrix*)((FSpeedTreeVertexFactory*)VertexFactory)->GetSpeedTree()->SRH->SpeedWind.GetWindMatrix(0);
				SetVertexShaderValues(  VertexShader->GetVertexShader(), WindMatricesParameter, WindMatrices, 3);
			}
		}

		if( CameraAlignMatrixParameter.IsBound() )
		{
			FMatrix CameraToWorld = View.ViewMatrix;
			CameraToWorld.SetOrigin(FVector(0,0,0));
			CameraToWorld = CameraToWorld.Transpose();
			SetVertexShaderValues<FVector4>( VertexShader->GetVertexShader(), CameraAlignMatrixParameter, (FVector4*)&CameraToWorld,3);
		}
	}

	virtual void SetMesh(FShader* VertexShader, const FMeshElement& Mesh,const FSceneView& View) const
	{
		FSpeedTreeVertexFactory* SpeedTreeVertexFactory = (FSpeedTreeVertexFactory*)Mesh.VertexFactory;
		const USpeedTree* SpeedTree = SpeedTreeVertexFactory->GetSpeedTree();
		check(SpeedTree);
		check(SpeedTree->SRH);

		check(Mesh.UserData != NULL);
		const FSpeedTreeVertexFactory::MeshUserDataType& MeshUserData = *(FSpeedTreeVertexFactory::MeshUserDataType*)Mesh.UserData;

		// Call the parameter setting function with the derived data.
		SetMeshInner(VertexShader,Mesh,View,MeshUserData,SpeedTree->SRH);
	}

	virtual void SetMeshInner(
		FShader* VertexShader,
		const FMeshElement& Mesh,
		const FSceneView& View,
		const FSpeedTreeVertexFactory::MeshUserDataType& MeshUserData,
		const FSpeedTreeResourceHelper* SRH
		) const
	{
		SetVertexShaderValue( 
			VertexShader->GetVertexShader(),
			LocalToWorldParameter,
			Mesh.LocalToWorld.ConcatTranslation(View.PreViewTranslation)
			);
		SetVertexShaderValue(  VertexShader->GetVertexShader(), WorldToLocalParameter, Mesh.WorldToLocal);
		SetVertexShaderValue(  VertexShader->GetVertexShader(), WindMatrixOffsetParameter, MeshUserData.WindMatrixOffset );
		SetVertexShaderValue( VertexShader->GetVertexShader(), RotationOnlyMatrixParameter, MeshUserData.RotationOnlyMatrix );

		if( AlphaAdjustmentParameter.IsBound() )
		{
			// Compute the fade alpha for this LOD.
			const FLOAT AlphaAdjustment = Lerp(-1.0f,0.0f,CalcLodFadeOpacity(MeshUserData,View));
			SetVertexShaderValue(  VertexShader->GetVertexShader(), AlphaAdjustmentParameter, AlphaAdjustment );
		}
	}

private:
	FShaderParameter LocalToWorldParameter;
	FShaderParameter WorldToLocalParameter;
	FShaderParameter AlphaAdjustmentParameter;
	FShaderParameter WindMatrixOffsetParameter;
	FShaderParameter WindMatricesParameter;
	FShaderParameter RotationOnlyMatrixParameter;
	FShaderParameter LeafRockAnglesParameter;
	FShaderParameter LeafRustleAnglesParameter;
	FShaderParameter CameraAlignMatrixParameter;
	FShaderParameter LeafAngleScalarsParameter;
	FShaderParameter IntermediateBranchFrondFadeValuesParameter;
};

class FSpeedTreeBillboardVertexFactoryShaderParameters : public FSpeedTreeVertexFactoryShaderParameters
{
public:

	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		FSpeedTreeVertexFactoryShaderParameters::Bind(ParameterMap);
		TextureCoordinateScaleBiasParameter.Bind(ParameterMap,TEXT("TextureCoordinateScaleBias"),TRUE);
		ViewToLocalParameter.Bind(ParameterMap,TEXT("ViewToLocal"),TRUE);
		BillboardMaskClipValuesParameter.Bind(ParameterMap,TEXT("BillboardMaskClipValues"),TRUE);
	}

	virtual void Serialize(FArchive& Ar)
	{
		FSpeedTreeVertexFactoryShaderParameters::Serialize(Ar);
		Ar << TextureCoordinateScaleBiasParameter;
		Ar << ViewToLocalParameter;
		Ar << BillboardMaskClipValuesParameter;
	}

	virtual void SetMeshInner(
		FShader* VertexShader,
		const FMeshElement& Mesh,
		const FSceneView& View,
		const FSpeedTreeVertexFactory::MeshUserDataType& MeshUserData,
		const FSpeedTreeResourceHelper* SRH
		) const
	{
		FSpeedTreeVertexFactoryShaderParameters::SetMeshInner(VertexShader,Mesh,View,MeshUserData,SRH);

		// Set the current camera position and direction.
		const FMatrix InvViewMatrix = View.ViewMatrix.Inverse();
		const FVector CurrentCameraOrigin = InvViewMatrix.GetOrigin();
		const FVector CurrentCameraZ = MeshUserData.RotationOnlyMatrix.TransformNormal(InvViewMatrix.GetAxis(2));
		CSpeedTreeRT::SetCamera(&CurrentCameraOrigin.X, &CurrentCameraZ.X);

		// set the position & transform of the component
		const FVector Origin(Mesh.LocalToWorld.GetOrigin());
		SRH->SpeedTree->SetTreePosition(Origin.X, Origin.Y, Origin.Z);

		// Generate the billboard geometry for this camera.
		//SRH->SpeedTree->GetGeometry(GSpeedTreeGeometry, SpeedTree_BillboardGeometry);
		SRH->SpeedTree->UpdateBillboardGeometry(GSpeedTreeGeometry);

		// Derive the UV range of the visible billboards.
		FVector4 TextureCoordinateScaleBias[3];
		const FLOAT* const BillboardTexCoords[3] =
		{
			GSpeedTreeGeometry.m_s360Billboard.m_pTexCoords[0],
			GSpeedTreeGeometry.m_s360Billboard.m_pTexCoords[1],
			GSpeedTreeGeometry.m_sHorzBillboard.m_pTexCoords
		};
		for(UINT BillboardIndex = 0;BillboardIndex < 3;BillboardIndex++)
		{
			if(BillboardTexCoords[BillboardIndex])
			{
				// The 3rd vertex maps the upper left corner of the billboard.
				const FLOAT MinU = BillboardTexCoords[BillboardIndex][2 * 2 + 0];
				const FLOAT MinV = BillboardTexCoords[BillboardIndex][2 * 2 + 1];
				// The 1st vertex maps the lower right corner of the billboard.
				const FLOAT MaxU = BillboardTexCoords[BillboardIndex][0 * 2 + 0];
				const FLOAT MaxV = BillboardTexCoords[BillboardIndex][0 * 2 + 1];
				TextureCoordinateScaleBias[BillboardIndex] = FVector4(
					MaxU - MinU,
					MaxV - MinV,
					MinV,
					MinU
					);
			}
			else
			{
				TextureCoordinateScaleBias[BillboardIndex] = FVector4(0,0,0,0);
			}
		}
		SetVertexShaderValue(VertexShader->GetVertexShader(),TextureCoordinateScaleBiasParameter,TextureCoordinateScaleBias);

		// Compute and set the view-to-local transform.
		const FMatrix ViewToLocal = InvViewMatrix * Mesh.WorldToLocal * MeshUserData.RotationOnlyMatrix;
		SetVertexShaderValues(VertexShader->GetVertexShader(),ViewToLocalParameter,(FVector4*)&ViewToLocal,3);

		// Compute and set the billboard mask clip values.
		const FLOAT BillboardMaskClipValues[3] =
		{
			Lerp(255.0f,GSpeedTreeGeometry.m_s360Billboard.m_afAlphaTestValues[0],(GSpeedTreeGeometry.m_sHorzBillboard.m_fAlphaTestValue - 84.0f) / (255.0f - 84.0f)),
			Lerp(255.0f,GSpeedTreeGeometry.m_s360Billboard.m_afAlphaTestValues[1],(GSpeedTreeGeometry.m_sHorzBillboard.m_fAlphaTestValue - 84.0f) / (255.0f - 84.0f)),
			GSpeedTreeGeometry.m_sHorzBillboard.m_fAlphaTestValue
		};
		for(UINT BillboardIndex = 0;BillboardIndex < 3;BillboardIndex++)
		{
			SetVertexShaderValue(
				VertexShader->GetVertexShader(),
				BillboardMaskClipValuesParameter,
				(Lerp(255.0f,BillboardMaskClipValues[BillboardIndex],CalcLodFadeOpacity(MeshUserData,View)) - 84.0f) / 255.0f,
				BillboardIndex
				);
		}

		// Reset the horizontal billboard texture coordinate pointer, as UpdateBillboardGeometry doesn't set it if the tree doesn't have a horizontal billboard.
		GSpeedTreeGeometry.m_sHorzBillboard.m_pTexCoords = NULL;
	}

private:
	FShaderParameter TextureCoordinateScaleBiasParameter;
	FShaderParameter ViewToLocalParameter;
	FShaderParameter BillboardMaskClipValuesParameter;
};

IMPLEMENT_VERTEX_FACTORY_TYPE(FSpeedTreeBillboardVertexFactory, FSpeedTreeBillboardVertexFactoryShaderParameters, "SpeedTreeBillboardVertexFactory", TRUE, TRUE, FALSE, VER_SPEEDTREE_SHADER_CHANGE,0);
IMPLEMENT_VERTEX_FACTORY_TYPE(FSpeedTreeBranchVertexFactory, FSpeedTreeVertexFactoryShaderParameters, "SpeedTreeBranchVertexFactory", TRUE, TRUE, FALSE, VER_SPEEDTREE_SHADER_CHANGE,0);
IMPLEMENT_VERTEX_FACTORY_TYPE(FSpeedTreeLeafCardVertexFactory, FSpeedTreeVertexFactoryShaderParameters, "SpeedTreeLeafCardVertexFactory", TRUE, TRUE, FALSE, VER_SPEEDTREE_SHADER_CHANGE,0);
IMPLEMENT_VERTEX_FACTORY_TYPE(FSpeedTreeLeafMeshVertexFactory, FSpeedTreeVertexFactoryShaderParameters, "SpeedTreeLeafMeshVertexFactory", TRUE, TRUE, FALSE, VER_SPEEDTREE_SHADER_CHANGE,0);


#endif // WITH_SPEEDTREE

