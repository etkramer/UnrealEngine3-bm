/*=============================================================================
	LocalDecalVertexFactory.cpp: Local vertex factory bound to a shader that computes decal texture coordinates.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "LocalVertexFactoryShaderParms.h"
#include "LocalDecalVertexFactory.h"

/**
 * Shader parameters for use with FGPUSkinDecalVertexFactory.
 */
class FLocalDecalVertexFactoryShaderParameters : public FLocalVertexFactoryShaderParameters
{
public:
	typedef FLocalVertexFactoryShaderParameters Super;

	/**
	 * Bind shader constants by name
	 * @param	ParameterMap - mapping of named shader constants to indices
	 */
	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		Super::Bind( ParameterMap );
		DecalMatrixParameter.Bind( ParameterMap, TEXT("DecalMatrix"), TRUE );
		DecalLocationParameter.Bind( ParameterMap, TEXT("DecalLocation"), TRUE );
		DecalOffsetParameter.Bind( ParameterMap, TEXT("DecalOffset"), TRUE );
		DecalLocalBinormalParameter.Bind( ParameterMap, TEXT("DecalLocalBinormal"), TRUE );
		DecalLocalTangentParameter.Bind( ParameterMap, TEXT("DecalLocalTangent"), TRUE );
		DecalLocalNormalParameter.Bind( ParameterMap, TEXT("DecalLocalNormal"), TRUE );
		DecalBlendIntervalParameter.Bind( ParameterMap, TEXT("DecalBlendInterval"), TRUE );
	}

	/**
	 * Serialize shader params to an archive
	 * @param	Ar - archive to serialize to
	 */
	virtual void Serialize(FArchive& Ar)
	{
		Super::Serialize( Ar );
		Ar << DecalMatrixParameter;
		Ar << DecalLocationParameter;
		Ar << DecalOffsetParameter;
		Ar << DecalLocalBinormalParameter;
		Ar << DecalLocalTangentParameter;
		Ar << DecalLocalNormalParameter;
		Ar << DecalBlendIntervalParameter;
	}

	/**
	 * Set any shader data specific to this vertex factory
	 */
	virtual void Set(FShader* VertexShader, const FVertexFactory* VertexFactory, const FSceneView& View) const
	{
		Super::Set( VertexShader, VertexFactory, View );

		FLocalDecalVertexFactory* DecalVF = (FLocalDecalVertexFactory*)VertexFactory;
		SetVertexShaderValue(  VertexShader->GetVertexShader(), DecalMatrixParameter, DecalVF->GetDecalMatrix() );
		SetVertexShaderValue(  VertexShader->GetVertexShader(), DecalLocationParameter, DecalVF->GetDecalLocation() );
		SetVertexShaderValue(  VertexShader->GetVertexShader(), DecalOffsetParameter, DecalVF->GetDecalOffset() );
		SetVertexShaderValue(  VertexShader->GetVertexShader(), DecalLocalBinormalParameter, DecalVF->GetDecalLocalBinormal() );
		SetVertexShaderValue(  VertexShader->GetVertexShader(), DecalLocalTangentParameter, DecalVF->GetDecalLocalTangent() );
		SetVertexShaderValue(  VertexShader->GetVertexShader(), DecalLocalNormalParameter, DecalVF->GetDecalLocalNormal() );	

		SetVertexShaderValue(  VertexShader->GetVertexShader(), DecalBlendIntervalParameter, DecalVF->GetDecalMinMaxBlend() );
	}

private:
	FShaderParameter DecalMatrixParameter;
	FShaderParameter DecalLocationParameter;
	FShaderParameter DecalOffsetParameter;
	FShaderParameter DecalLocalBinormalParameter;
	FShaderParameter DecalLocalTangentParameter;
	FShaderParameter DecalLocalNormalParameter;
	FShaderParameter DecalBlendIntervalParameter;
};

IMPLEMENT_VERTEX_FACTORY_TYPE(FLocalDecalVertexFactory,FLocalDecalVertexFactoryShaderParameters,"LocalVertexFactory",TRUE,TRUE,FALSE,VER_DECAL_VERTEX_FACTORY_VER3,0);
