/*=============================================================================
	ShadowRendering.cpp: Shadow rendering implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "UnTextureLayout.h"

// Globals
const UINT SHADOW_BORDER=5; 

/**
 * A vertex shader for rendering the depth of a mesh.
 */
class FShadowDepthVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FShadowDepthVertexShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile for default or masked materials
		return Material->IsSpecialEngineMaterial() || Material->IsMasked();
	}

	FShadowDepthVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		ProjectionMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ProjectionMatrix"));
		InvMaxSubjectDepthParameter.Bind(Initializer.ParameterMap,TEXT("InvMaxSubjectDepth"));
		DepthBiasParameter.Bind(Initializer.ParameterMap,TEXT("DepthBias"));
	}

	FShadowDepthVertexShader() {}
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		bShaderHasOutdatedParameters |= Ar << VertexFactoryParameters;
		Ar << ProjectionMatrixParameter;
		Ar << InvMaxSubjectDepthParameter;
		Ar << DepthBiasParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FSceneView& View,
		const FProjectedShadowInfo* ShadowInfo
		)
	{
		VertexFactoryParameters.Set(this,VertexFactory,View);

		SetVertexShaderValue(
			GetVertexShader(),
			ProjectionMatrixParameter,
			FTranslationMatrix(ShadowInfo->PreShadowTranslation - View.PreViewTranslation) * ShadowInfo->SubjectMatrix
			);

		SetVertexShaderValue(GetVertexShader(),InvMaxSubjectDepthParameter,1.0f / ShadowInfo->MaxSubjectDepth);
		FLOAT DepthBias = GSystemSettings.ShadowDepthBias * 512.0f / Max(ShadowInfo->ResolutionX,ShadowInfo->ResolutionY);
#if SUPPORTS_VSM
		const UBOOL bEnableVSMShadows = ShadowInfo->LightSceneInfo->ShadowProjectionTechnique == ShadowProjTech_VSM || 
			(GSystemSettings.bEnableVSMShadows && ShadowInfo->LightSceneInfo->ShadowProjectionTechnique == ShadowProjTech_Default);
		if (bEnableVSMShadows)
		{
			//no spatial bias for VSM shadows, they only need a statistical bias
			DepthBias = 0.0f;
		} 
		else 
#endif //#if SUPPORTS_VSM
		if (ShouldUseBranchingPCF(ShadowInfo->LightSceneInfo->ShadowProjectionTechnique))
		{
			//small tweakable to make the effect of the offset used with Branching PCF match up with the default PCF
			DepthBias += .001f;
		}
		SetVertexShaderValue(GetVertexShader(),DepthBiasParameter,DepthBias);
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View)
	{
		VertexFactoryParameters.SetMesh(this,Mesh,View);
	}

private:
	FVertexFactoryParameterRef VertexFactoryParameters;
	FShaderParameter ProjectionMatrixParameter;
	FShaderParameter InvMaxSubjectDepthParameter;
	FShaderParameter DepthBiasParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FShadowDepthVertexShader,TEXT("ShadowDepthVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/**
 * A pixel shader for rendering the depth of a mesh.
 */
class FShadowDepthPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FShadowDepthPixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile for default or masked materials
		return Material->IsSpecialEngineMaterial() || Material->IsMasked();
	}

	FShadowDepthPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
	}

	FShadowDepthPixelShader() {}

	void SetParameters(
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FSceneView& View,
		const FProjectedShadowInfo* ShadowInfo
		)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy, View.Family->CurrentWorldTime, View.Family->CurrentRealTime, &View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View,UBOOL bBackFace)
	{
		MaterialParameters.SetMesh(this,Mesh,View,bBackFace);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters;
		return bShaderHasOutdatedParameters;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FShadowDepthPixelShader,TEXT("ShadowDepthPixelShader"),TEXT("Main"),SF_Pixel,0,0);

/** The shadow frustum vertex declaration. */
TGlobalResource<FShadowFrustumVertexDeclaration> GShadowFrustumVertexDeclaration;

/*-----------------------------------------------------------------------------
	FShadowProjectionVertexShader
-----------------------------------------------------------------------------*/

UBOOL FShadowProjectionVertexShader::ShouldCache(EShaderPlatform Platform)
{
	return TRUE;
}

FShadowProjectionVertexShader::FShadowProjectionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FGlobalShader(Initializer)
{
}

void FShadowProjectionVertexShader::SetParameters(
										const FSceneView& View,
										const FProjectedShadowInfo* ShadowInfo
										)
{
}

UBOOL FShadowProjectionVertexShader::Serialize(FArchive& Ar)
{
	return FShader::Serialize(Ar);
}

IMPLEMENT_SHADER_TYPE(,FShadowProjectionVertexShader,TEXT("ShadowProjectionVertexShader"),TEXT("Main"),SF_Vertex,0,0);


/*-----------------------------------------------------------------------------
	FModShadowProjectionVertexShader
-----------------------------------------------------------------------------*/

/**
 * Constructor - binds all shader params
 * @param Initializer - init data from shader compiler
 */
FModShadowProjectionVertexShader::FModShadowProjectionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
:	FShadowProjectionVertexShader(Initializer)
{		
}

/**
 * Sets the current vertex shader
 * @param View - current view
 * @param ShadowInfo - projected shadow info for a single light
 */
void FModShadowProjectionVertexShader::SetParameters(	
													 const FSceneView& View,
													 const FProjectedShadowInfo* ShadowInfo
													 )
{
	FShadowProjectionVertexShader::SetParameters(View,ShadowInfo);
}

/**
 * Serialize the parameters for this shader
 * @param Ar - archive to serialize to
 */
UBOOL FModShadowProjectionVertexShader::Serialize(FArchive& Ar)
{
	return FShadowProjectionVertexShader::Serialize(Ar);
}

IMPLEMENT_SHADER_TYPE(,FModShadowProjectionVertexShader,TEXT("ModShadowProjectionVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/**
 * Implementations for TShadowProjectionPixelShader.  
 */

//Shader Model 2 compatible version that uses Hardware PCF
IMPLEMENT_SHADER_TYPE(template<>,TShadowProjectionPixelShader<F4SampleHwPCF>,TEXT("ShadowProjectionPixelShader"),TEXT("HardwarePCFMain"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);
//Shader Model 2 compatible version
IMPLEMENT_SHADER_TYPE(template<>,TShadowProjectionPixelShader<F4SampleManualPCF>,TEXT("ShadowProjectionPixelShader"),TEXT("Main"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);
//Full version that uses Hardware PCF
IMPLEMENT_SHADER_TYPE(template<>,TShadowProjectionPixelShader<F16SampleHwPCF>,TEXT("ShadowProjectionPixelShader"),TEXT("HardwarePCFMain"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);
//Full version that uses Fetch4
IMPLEMENT_SHADER_TYPE(template<>,TShadowProjectionPixelShader<F16SampleFetch4PCF>,TEXT("ShadowProjectionPixelShader"),TEXT("Fetch4Main"),SF_Pixel,0,0);
//Full version
IMPLEMENT_SHADER_TYPE(template<>,TShadowProjectionPixelShader<F16SampleManualPCF>,TEXT("ShadowProjectionPixelShader"),TEXT("Main"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);

/**
* Get the version of TShadowProjectionPixelShader that should be used based on the hardware's capablities
* @return a pointer to the chosen shader
*/
FShadowProjectionPixelShaderInterface* GetProjPixelShaderRef(BYTE LightShadowQuality)
{
	FShadowProjectionPixelShaderInterface * PixelShader = NULL;

	//apply the system settings bias to the light's shadow quality
	BYTE EffectiveShadowFilterQuality = Max(LightShadowQuality + GSystemSettings.ShadowFilterQualityBias, 0);

	//force shader model 2 cards to the 4 sample path
	if (EffectiveShadowFilterQuality == SFQ_Low || GRHIShaderPlatform == SP_PCD3D_SM2)
	{
		if (GSupportsHardwarePCF)
		{
			TShaderMapRef<TShadowProjectionPixelShader<F4SampleHwPCF> > FourSamplePixelShader(GetGlobalShaderMap());
			PixelShader = *FourSamplePixelShader;
		}
		else
		{
			TShaderMapRef<TShadowProjectionPixelShader<F4SampleManualPCF> > FourSamplePixelShader(GetGlobalShaderMap());
			PixelShader = *FourSamplePixelShader;
		}
	}
	//todo - implement medium quality path, 9 samples?
	else
	{
		if (GSupportsHardwarePCF)
		{
			TShaderMapRef<TShadowProjectionPixelShader<F16SampleHwPCF> > SixteenSamplePixelShader(GetGlobalShaderMap());
			PixelShader = *SixteenSamplePixelShader;
		}
		else if (GSupportsFetch4)
		{
			TShaderMapRef<TShadowProjectionPixelShader<F16SampleFetch4PCF> > SixteenSamplePixelShader(GetGlobalShaderMap());
			PixelShader = *SixteenSamplePixelShader;
		}
		else
		{
			TShaderMapRef<TShadowProjectionPixelShader<F16SampleManualPCF> > SixteenSamplePixelShader(GetGlobalShaderMap());
			PixelShader = *SixteenSamplePixelShader;
		}
	}
	return PixelShader;
}

/*-----------------------------------------------------------------------------
	FShadowDepthDrawingPolicy
-----------------------------------------------------------------------------*/

FShadowDepthDrawingPolicy::FShadowDepthDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy,
	const FProjectedShadowInfo* InShadowInfo
	):
	FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy),
	ShadowInfo(InShadowInfo)
{
	const FMaterial* MaterialResource = InMaterialRenderProxy->GetMaterial();
	if (MaterialResource->IsMasked())
	{
		// Get the shaders from the original material, so they can evaluate the mask
		VertexShader = MaterialResource->GetShader<FShadowDepthVertexShader>(InVertexFactory->GetType());
		PixelShader = MaterialResource->GetShader<FShadowDepthPixelShader>(InVertexFactory->GetType());
	}
	else
	{
		// If the material is not masked, get the shaders from the default material.
		// This still handles two-sided materials because we are only overriding which material the shaders come from,
		// not which material's two sided flag is checked.
		const FMaterial* DefaultMaterialResource = GEngine->DefaultMaterial->GetRenderProxy(FALSE)->GetMaterial();
		VertexShader = DefaultMaterialResource->GetShader<FShadowDepthVertexShader>(InVertexFactory->GetType());
		// Platforms that support depth textures use a null pixel shader for opaque materials to get double speed Z
		// Other platforms need to output depth as color and still need the pixel shader
		if (GSupportsDepthTextures)
		{
			PixelShader = NULL;
		}
		else
		{
			PixelShader = DefaultMaterialResource->GetShader<FShadowDepthPixelShader>(InVertexFactory->GetType());
		}
	}
}

void FShadowDepthDrawingPolicy::DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
{
	VertexShader->SetParameters(VertexFactory,MaterialRenderProxy,*View,ShadowInfo);

	if (PixelShader)
	{
		PixelShader->SetParameters(MaterialRenderProxy,*View,ShadowInfo);
	}

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared(View);

	// Set the actual shader & vertex declaration state
	RHISetBoundShaderState( BoundShaderState);
}

/** 
 * Create bound shader state using the vertex decl from the mesh draw policy
 * as well as the shaders needed to draw the mesh
 * @param DynamicStride - optional stride for dynamic vertex data
 * @return new bound shader state object
 */
FBoundShaderStateRHIRef FShadowDepthDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);
	if (DynamicStride)
	{
		StreamStrides[0] = DynamicStride;
	}

	return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader ? PixelShader->GetPixelShader() : FPixelShaderRHIRef());	
}

void FShadowDepthDrawingPolicy::SetMeshRenderState(
	const FSceneView& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	VertexShader->SetMesh(Mesh,View);
	if (PixelShader)
	{
		PixelShader->SetMesh(Mesh,View,bBackFace);
	}
	FMeshDrawingPolicy::SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,ElementData);
}

INT Compare(const FShadowDepthDrawingPolicy& A,const FShadowDepthDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialRenderProxy);
	return 0;
}

UBOOL FShadowDepthDrawingPolicyFactory::DrawDynamicMesh(
	const FSceneView& View,
	const FProjectedShadowInfo* ShadowInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	UBOOL bDirty = FALSE;

	// Use a per-FMeshElement check on top of the per-primitive check because dynamic primitives can submit multiple FMeshElements.
	if (Mesh.CastShadow)
	{
		const FMaterialRenderProxy* MaterialRenderProxy = Mesh.MaterialRenderProxy;
		const FMaterial* Material = MaterialRenderProxy->GetMaterial();
		const EBlendMode BlendMode = Material->GetBlendMode();

		if (!IsTranslucentBlendMode(BlendMode))
		{
			if (!Material->IsMasked() && !Material->IsTwoSided())
			{
				// Override with the default material for opaque materials that are not two sided
				MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
			}

			FShadowDepthDrawingPolicy DrawingPolicy(Mesh.VertexFactory,MaterialRenderProxy,ShadowInfo);
			DrawingPolicy.DrawShared(&View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			DrawingPolicy.SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Mesh);
			bDirty = TRUE;
		}
	}
	
	return bDirty;
}

/*-----------------------------------------------------------------------------
	FProjectedShadowInfo
-----------------------------------------------------------------------------*/

void FProjectedShadowInfo::RenderDepth(const FSceneRenderer* SceneRenderer,BYTE DepthPriorityGroup)
{
#if WANTS_DRAW_MESH_EVENTS
	const FName ParentName = ParentSceneInfo && ParentSceneInfo->Owner ? ParentSceneInfo->Owner->GetFName() : NAME_None;
	SCOPED_DRAW_EVENT(EventShadowDepthActor)(DEC_SCENE_ITEMS,*ParentName.ToString());
#endif

	// Set the viewport for the shadow.
	RHISetViewport(
		X,
		Y,
		0.0f,
		X + SHADOW_BORDER*2 + ResolutionX,
		Y + SHADOW_BORDER*2 + ResolutionY,
		1.0f
		);

	if( GSupportsDepthTextures || GSupportsHardwarePCF || GSupportsFetch4)
	{
		// Clear depth only.
		RHIClear(FALSE,FColor(255,255,255),TRUE,1.0f,FALSE,0);
	}
	else
	{
		// Clear color and depth.
		RHIClear(TRUE,FColor(255,255,255),TRUE,1.0f,FALSE,0);
	}	

	// Set the viewport for the shadow.
	RHISetViewport(
		X + SHADOW_BORDER,
		Y + SHADOW_BORDER,
		0.0f,
		X + SHADOW_BORDER + ResolutionX,
		Y + SHADOW_BORDER + ResolutionY,
		1.0f
		);

	// Opaque blending, depth tests and writes, solid rasterization w/ back-face culling.
	RHISetBlendState(TStaticBlendState<>::GetRHI());	
	RHISetDepthState(TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_CW>::GetRHI());

	// Choose an arbitrary view where this shadow is in the right DPG to use for rendering the depth.
	const FViewInfo* View = NULL;
	ESceneDepthPriorityGroup DPGToUseForDepths = (ESceneDepthPriorityGroup)DepthPriorityGroup;
	for(INT ViewIndex = 0;ViewIndex < SceneRenderer->Views.Num();ViewIndex++)
	{
		const FViewInfo* CheckView = &SceneRenderer->Views(ViewIndex);
		const FVisibleLightViewInfo& VisibleLightViewInfo = CheckView->VisibleLightInfos(LightSceneInfo->Id);
		FPrimitiveViewRelevance ViewRel = VisibleLightViewInfo.ProjectedShadowViewRelevanceMap(ShadowId);
		if (ViewRel.GetDPG(DepthPriorityGroup))
		{
			View = CheckView;
			break;
		}
		// Allow a view with the shadow relevant to the foreground DPG if we are in the WorldDPG and any view has the subject in the foreground DPG
		else if (bForegroundCastingOnWorld && ViewRel.GetDPG(SDPG_Foreground))
		{
			View = CheckView;
			DPGToUseForDepths = SDPG_Foreground;
			break;
		}
	}
	check(View);

	// Draw the subject's static elements.
	SubjectMeshElements.DrawAll(*View);

	// Draw the subject's dynamic elements.
	TDynamicPrimitiveDrawer<FShadowDepthDrawingPolicyFactory> Drawer(View,DPGToUseForDepths,this,TRUE);
	for(INT PrimitiveIndex = 0;PrimitiveIndex < SubjectPrimitives.Num();PrimitiveIndex++)
	{
		Drawer.SetPrimitive(SubjectPrimitives(PrimitiveIndex));
		SubjectPrimitives(PrimitiveIndex)->Proxy->DrawDynamicElements(&Drawer,View,DPGToUseForDepths);
	}
}

FGlobalBoundShaderState FProjectedShadowInfo::MaskBoundShaderState;
FGlobalBoundShaderState FProjectedShadowInfo::ShadowProjectionBoundShaderState;
#if SUPPORTS_VSM
FGlobalBoundShaderState FProjectedShadowInfo::ShadowProjectionVSMBoundShaderState;
#endif //#if SUPPORTS_VSM
FGlobalBoundShaderState FProjectedShadowInfo::BranchingPCFLowQualityBoundShaderState;
FGlobalBoundShaderState FProjectedShadowInfo::BranchingPCFMediumQualityBoundShaderState;
FGlobalBoundShaderState FProjectedShadowInfo::BranchingPCFHighQualityBoundShaderState;

UBOOL GUseHiStencil = TRUE;

void FProjectedShadowInfo::RenderProjection(const FViewInfo* View, BYTE DepthPriorityGroup) const
{
#if WANTS_DRAW_MESH_EVENTS
	const FName ParentName = ParentSceneInfo && ParentSceneInfo->Owner ? ParentSceneInfo->Owner->GetFName() : NAME_None;
	SCOPED_DRAW_EVENT(EventShadowProjectionActor)(DEC_SCENE_ITEMS,*ParentName.ToString());
#endif
	// Find the shadow's view relevance.
	const FVisibleLightViewInfo& VisibleLightViewInfo = View->VisibleLightInfos(LightSceneInfo->Id);
	FPrimitiveViewRelevance ViewRelevance = VisibleLightViewInfo.ProjectedShadowViewRelevanceMap(ShadowId);

	// Don't render shadows for subjects which aren't view relevant.
	if (ViewRelevance.bShadowRelevance == FALSE)
	{
		return;
	}

	if (!ViewRelevance.GetDPG(DepthPriorityGroup) && !bForegroundCastingOnWorld)
	{
		return;
	}

	// The shadow transforms and view transforms are relative to different origins, so the world coordinates need to be translated.
	const FVector4 PreShadowToPreViewTranslation(View->PreViewTranslation - PreShadowTranslation,0);

	FVector FrustumVertices[8];
	
	for(UINT Z = 0;Z < 2;Z++)
	{
		for(UINT Y = 0;Y < 2;Y++)
		{
			for(UINT X = 0;X < 2;X++)
			{
				const FVector4 UnprojectedVertex = InvReceiverMatrix.TransformFVector4(
					FVector4(
						(X ? -1.0f : 1.0f),
						(Y ? -1.0f : 1.0f),
						(Z ?  0.0f : 1.0f),
						1.0f
						)
					);
				const FVector ProjectedVertex = UnprojectedVertex / UnprojectedVertex.W + PreShadowToPreViewTranslation;
				FrustumVertices[GetCubeVertexIndex(X,Y,Z)] = ProjectedVertex;
			}
		}
	}

	// Find the projection shaders.
	TShaderMapRef<FShadowProjectionVertexShader> VertexShader(GetGlobalShaderMap());

	if( GSystemSettings.bEnableForegroundShadowsOnWorld &&
		DepthPriorityGroup == SDPG_Foreground &&
		(LightSceneInfo->LightShadowMode == LightShadow_Modulate || LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter) )
	{
		SCOPED_DRAW_EVENT(EventMaskSubjects)(DEC_SCENE_ITEMS,TEXT("Stencil Mask Subjects"));

		// For foreground subjects, only do self-shadowing in the foreground DPG. 
		// Shadowing from the foreground subjects onto the world will have been done in the world DPG,
		// to take advantage of the world DPG's depths being in the depth buffer,
		// and therefore the intersection of the shadow frustum and the scene can be stenciled.
		if (GUseHiStencil)
		{
			RHIBeginHiStencilRecord(TRUE, 0);
		}

		RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());

		// Set stencil to one.
		RHISetStencilState(TStaticStencilState<
			TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
			FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
			0xff,0xff,1
		>::GetRHI());

		// Draw the subject's dynamic elements
		TDynamicPrimitiveDrawer<FDepthDrawingPolicyFactory> Drawer(View,SDPG_Foreground,DDM_AllOccluders,TRUE);
		for(INT PrimitiveIndex = 0;PrimitiveIndex < SubjectPrimitives.Num();PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* SubjectPrimitiveSceneInfo = SubjectPrimitives(PrimitiveIndex);
			if(View->PrimitiveVisibilityMap(SubjectPrimitiveSceneInfo->Id))
			{
				Drawer.SetPrimitive(SubjectPrimitiveSceneInfo);
				SubjectPrimitiveSceneInfo->Proxy->DrawDynamicElements(&Drawer,View,SDPG_Foreground);
			}
		}

		if (GUseHiStencil)
		{
			RHIBeginHiStencilPlayback(TRUE);
		}
	}
	else
	{
		// Depth test wo/ writes, no color writing.
		RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
		RHISetColorWriteEnable(FALSE);

		if (GUseHiStencil)
		{
			RHIBeginHiStencilRecord(TRUE, 0);
		}

		// If this is a preshadow, mask the projection by the receiver primitives.
		if (bPreShadow)
		{
			// mask based on backfaces, emissive areas wrt light's modulated shadow
			if( GSystemSettings.bAllowBetterModulatedShadows &&
				LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter )
			{
				// clip using alpha ref test (> 0 are culled)
				RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_Zero,BO_Add,BF_Zero,BF_One,CF_LessEqual,0>::GetRHI());
				// Set stencil to one.
				RHISetStencilState(TStaticStencilState<
					TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
					FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
					0xff,0xff,1
				>::GetRHI());
				// render the prims iot update stencil
				RenderModShadowPrimitives(View,DepthPriorityGroup,ReceiverPrimitives);
			}
			else
			{
				// Set stencil to one.
				RHISetStencilState(TStaticStencilState<
					TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
					FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
					0xff,0xff,1
				>::GetRHI());

				// Draw the receiver's dynamic elements.
				TDynamicPrimitiveDrawer<FDepthDrawingPolicyFactory> Drawer(View,DepthPriorityGroup,DDM_AllOccluders,TRUE);
				for(INT PrimitiveIndex = 0;PrimitiveIndex < ReceiverPrimitives.Num();PrimitiveIndex++)
				{
					const FPrimitiveSceneInfo* ReceiverPrimitiveSceneInfo = ReceiverPrimitives(PrimitiveIndex);
					if(View->PrimitiveVisibilityMap(ReceiverPrimitiveSceneInfo->Id))
					{
						Drawer.SetPrimitive(ReceiverPrimitiveSceneInfo);
						ReceiverPrimitiveSceneInfo->Proxy->DrawDynamicElements(&Drawer,View,DepthPriorityGroup);
					}
				}
			}
		}
		// If this shadow should only self-shadow, mask the projection by the subject's pixels.
		else if (bSelfShadowOnly)
		{
			SCOPED_DRAW_EVENT(EventMaskSubjects)(DEC_SCENE_ITEMS,TEXT("Stencil Mask Subjects"));

			// Set stencil to one.
			RHISetStencilState(TStaticStencilState<
				TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
				FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
				0xff,0xff,1
			>::GetRHI());

			// Draw the subject's dynamic opaque elements
			//@todo -handle the masked subject primitives
			// This is currently disabled as the xbox 360 issues a priority 1 warning when rendering masked materials with HIStencil write on.
			TDynamicPrimitiveDrawer<FDepthDrawingPolicyFactory> Drawer(View,DepthPriorityGroup,DDM_NonMaskedOnly,TRUE);
			for(INT PrimitiveIndex = 0;PrimitiveIndex < SubjectPrimitives.Num();PrimitiveIndex++)
			{
				const FPrimitiveSceneInfo* SubjectPrimitiveSceneInfo = SubjectPrimitives(PrimitiveIndex);
				if(View->PrimitiveVisibilityMap(SubjectPrimitiveSceneInfo->Id))
				{
					Drawer.SetPrimitive(SubjectPrimitiveSceneInfo);
					SubjectPrimitiveSceneInfo->Proxy->DrawDynamicElements(&Drawer,View,DepthPriorityGroup);
				}
			}
		}
		// Not bSelfShadowOnly or a preshadow, mask the projection to any pixels inside the frustum.
		else
		{
			// Solid rasterization wo/ backface culling.
			RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

			// Increment stencil on front-facing zfail, decrement on back-facing zfail.
			RHISetStencilState(TStaticStencilState<
				TRUE,CF_Always,SO_Keep,SO_Increment,SO_Keep,
				TRUE,CF_Always,SO_Keep,SO_Decrement,SO_Keep,
				0xff,0xff
			>::GetRHI());

			// Set the projection vertex shader parameters
			VertexShader->SetParameters(*View,this);

			// Cache the bound shader state
			SetGlobalBoundShaderState(MaskBoundShaderState,GShadowFrustumVertexDeclaration.VertexDeclarationRHI,*VertexShader,NULL,sizeof(FVector));

			// Draw the frustum using the stencil buffer to mask just the pixels which are inside the shadow frustum.
			RHIDrawIndexedPrimitiveUP( PT_TriangleList, 0, 8, 12, GCubeIndices, sizeof(WORD), FrustumVertices, sizeof(FVector));

			if (bForegroundCastingOnWorld)
			{
				// We are rendering the shadow from a foreground DPG subject onto the world DPG,
				// so setup a stencil mask for the subject so that we don't waste shadow computations on those pixels,
				// which will just be overwritten later, in the foreground DPG.
				SCOPED_DRAW_EVENT(EventMaskSubjects)(DEC_SCENE_ITEMS,TEXT("Stencil Mask Subjects"));

				// No depth test or writes 
				RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());

				// Set stencil to zero.
				RHISetStencilState(TStaticStencilState<
					TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
					FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
					0xff,0xff,0
				>::GetRHI());

				// Draw the subject's dynamic opaque elements
				TDynamicPrimitiveDrawer<FDepthDrawingPolicyFactory> Drawer(View,SDPG_Foreground,DDM_NonMaskedOnly,TRUE);
				for(INT PrimitiveIndex = 0;PrimitiveIndex < SubjectPrimitives.Num();PrimitiveIndex++)
				{
					const FPrimitiveSceneInfo* SubjectPrimitiveSceneInfo = SubjectPrimitives(PrimitiveIndex);
					if(View->PrimitiveVisibilityMap(SubjectPrimitiveSceneInfo->Id))
					{
						Drawer.SetPrimitive(SubjectPrimitiveSceneInfo);
						SubjectPrimitiveSceneInfo->Proxy->DrawDynamicElements(&Drawer,View,SDPG_Foreground);
					}
				}
			}
		}

		if (GUseHiStencil)
		{
			RHIBeginHiStencilPlayback(TRUE);
		}

		// mask based on backfaces, emissive areas wrt light's modulated shadow
		if( GSystemSettings.bAllowBetterModulatedShadows &&
			!bPreShadow && 
			// Disable backface and emissive masking for foreground subjects casting on the world as the visual impact is minimal
			!bForegroundCastingOnWorld &&
			LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter )
		{
			// clip using alpha ref test (<= 0 are culled)
			RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_Zero,BO_Add,BF_Zero,BF_One,CF_Greater,0>::GetRHI());		
			// stencil test passes if != 0 and writes 0 to cull pixels for where we don't want shadow
			RHISetStencilState(TStaticStencilState<
				TRUE,CF_NotEqual,SO_Keep,SO_Keep,SO_Replace,
				FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
				0xff,0xff,0
			>::GetRHI());
			// render the prims to update stencil
			RenderModShadowPrimitives(View,DepthPriorityGroup,ModShadowReceiverPrimitives);
		}
	}	//DPGIndex != SDPG_Foreground

#if SUPPORTS_VSM
	const UBOOL bEnableVSMShadows = LightSceneInfo->ShadowProjectionTechnique == ShadowProjTech_VSM || 
		(GSystemSettings.bEnableVSMShadows && LightSceneInfo->ShadowProjectionTechnique == ShadowProjTech_Default);
#endif //#if SUPPORTS_VSM

	if( LightSceneInfo->LightShadowMode == LightShadow_Modulate ||
		LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter )
	{
		// modulated blending, preserve alpha
		RHISetBlendState(TStaticBlendState<BO_Add,BF_DestColor,BF_Zero,BO_Add,BF_Zero,BF_One>::GetRHI());

#if SUPPORTS_VSM
		if( bEnableVSMShadows )
		{
			// Set the VSM modulated shadow projection shaders.
			TShaderMapRef<FModShadowProjectionVertexShader> ModShadowProjVertexShader(GetGlobalShaderMap());
			FVSMModProjectionPixelShader* VSMModShadowProjPixelShader = LightSceneInfo->GetVSMModProjPixelShader();
			checkSlow(VSMModShadowProjPixelShader);
			ModShadowProjVertexShader->SetParameters(*View,this);
			VSMModShadowProjPixelShader->SetParameters(*View,this);
			RHISetBoundShaderState( LightSceneInfo->GetVSMModProjBoundShaderState());
		}
		else 
#endif //#if SUPPORTS_VSM
		if(ShouldUseBranchingPCF(LightSceneInfo->ShadowProjectionTechnique))
		{

			// Set the Branching PCF modulated shadow projection shaders.
			TShaderMapRef<FModShadowProjectionVertexShader> ModShadowProjVertexShader(GetGlobalShaderMap());
			FBranchingPCFProjectionPixelShaderInterface* BranchingPCFModShadowProjPixelShader = LightSceneInfo->GetBranchingPCFModProjPixelShader();
			checkSlow(BranchingPCFModShadowProjPixelShader);
			
			ModShadowProjVertexShader->SetParameters(*View,this);
			BranchingPCFModShadowProjPixelShader->SetParameters(*View,this);

			SetGlobalBoundShaderState( *LightSceneInfo->GetBranchingPCFModProjBoundShaderState(), GShadowFrustumVertexDeclaration.VertexDeclarationRHI, 
				*ModShadowProjVertexShader, BranchingPCFModShadowProjPixelShader, sizeof(FVector));
		}
		else
		{
			// Set the modulated shadow projection shaders.
			TShaderMapRef<FModShadowProjectionVertexShader> ModShadowProjVertexShader(GetGlobalShaderMap());
			
			FShadowProjectionPixelShaderInterface* ModShadowProjPixelShader = LightSceneInfo->GetModShadowProjPixelShader();
			checkSlow(ModShadowProjPixelShader);

			ModShadowProjVertexShader->SetParameters(*View,this);
			ModShadowProjPixelShader->SetParameters(*View,this);

			SetGlobalBoundShaderState( *LightSceneInfo->GetModShadowProjBoundShaderState(), GShadowFrustumVertexDeclaration.VertexDeclarationRHI, 
				*ModShadowProjVertexShader, ModShadowProjPixelShader, sizeof(FVector));
		}
	}
	else
	{
		//modulated blending, shadows may overlap
		RHISetBlendState(TStaticBlendState<BO_Add,BF_DestColor,BF_Zero>::GetRHI());
	
		// Set the shadow projection vertex shader.
		VertexShader->SetParameters(*View,this);

#if SUPPORTS_VSM
		if( bEnableVSMShadows )
		{
			// VSM shadow projection pixel shader
			TShaderMapRef<FVSMProjectionPixelShader> PixelShaderVSM(GetGlobalShaderMap());
			PixelShaderVSM->SetParameters(*View,this);

			SetGlobalBoundShaderState( ShadowProjectionVSMBoundShaderState, GShadowFrustumVertexDeclaration.VertexDeclarationRHI, 
				*VertexShader, *PixelShaderVSM, sizeof(FVector));
		}
		else 
#endif //#if SUPPORTS_VSM
		if (ShouldUseBranchingPCF(LightSceneInfo->ShadowProjectionTechnique))
		{
			// Branching PCF shadow projection pixel shader
			FShader* BranchingPCFPixelShader = SetBranchingPCFParameters(*View,this,LightSceneInfo->ShadowFilterQuality);

			FGlobalBoundShaderState* CurrentBPCFBoundShaderState = ChooseBPCFBoundShaderState(LightSceneInfo->ShadowFilterQuality, &BranchingPCFLowQualityBoundShaderState, 
				&BranchingPCFMediumQualityBoundShaderState, &BranchingPCFHighQualityBoundShaderState);
			
			SetGlobalBoundShaderState( *CurrentBPCFBoundShaderState, GShadowFrustumVertexDeclaration.VertexDeclarationRHI, 
				*VertexShader, BranchingPCFPixelShader, sizeof(FVector));
		} 
		else
		{
			FShadowProjectionPixelShaderInterface * PixelShader = GetProjPixelShaderRef(LightSceneInfo->ShadowFilterQuality);
			PixelShader->SetParameters(*View,this);

			SetGlobalBoundShaderState( ShadowProjectionBoundShaderState, GShadowFrustumVertexDeclaration.VertexDeclarationRHI, 
				*VertexShader, PixelShader, sizeof(FVector));
		}
	}

	// no depth test or writes, solid rasterization w/ back-face culling.
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetColorWriteEnable(TRUE);
	RHISetRasterizerState(
		View->bReverseCulling ? TStaticRasterizerState<FM_Solid,CM_CCW>::GetRHI() : TStaticRasterizerState<FM_Solid,CM_CW>::GetRHI());

	// Test stencil for != 0.
	// Note: Writing stencil while testing disables stencil cull on all Nvidia cards! (including RSX) It's better to just clear instead.
	RHISetStencilState(TStaticStencilState<
		TRUE,CF_NotEqual,SO_Keep,SO_Keep,SO_Keep,
		FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
		0xff,0xff,0
	>::GetRHI());

	// Draw the frustum using the projection shader..
	RHIDrawIndexedPrimitiveUP( PT_TriangleList, 0, 8, 12, GCubeIndices, sizeof(WORD), FrustumVertices, sizeof(FVector));

	if (GUseHiStencil)
	{
		RHIEndHiStencil();
	}

	// Reset the stencil state.
	RHISetStencilState(TStaticStencilState<>::GetRHI());

	// Clear the stencil buffer to 0.
	RHIClear(FALSE,FColor(0,0,0),FALSE,0,TRUE,0);
}

/**
* Render modulated shadow receiver primitives using the FMeshModShadowDrawingPolicyFactory
*
* @param View - current view to render with
* @param DepthPriorityGroup - current DPG to cull mesh elements based on relevancy
* @param ModShadowPrims - list of primitives to render
* @return TRUE if anything was rendered
*/
UBOOL FProjectedShadowInfo::RenderModShadowPrimitives(const FViewInfo* View, 
								BYTE DepthPriorityGroup, 
								const FProjectedShadowInfo::PrimitiveArrayType& ModShadowPrims ) const
{
	UBOOL bDirty=FALSE;

	if( ModShadowPrims.Num() )
	{
		SCOPED_DRAW_EVENT(EventModShadowPrims)(DEC_SCENE_ITEMS,TEXT("Mod Shadow Prims"));

		// For drawing scene prims with dynamic relevance.
		TDynamicPrimitiveDrawer<FMeshModShadowDrawingPolicyFactory> Drawer(View,DepthPriorityGroup,LightSceneInfo,TRUE);

		for(INT PrimitiveIndex = 0;PrimitiveIndex < ModShadowPrims.Num();PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = ModShadowPrims(PrimitiveIndex);
			const FPrimitiveViewRelevance& ViewRelevance = View->PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

			// we only need to do this work when modulating on backfaces or emissive is requested and allowed
			// when GModShadowsWithAlphaEmissiveBit is set, we do not let primitives with ONLY bCullModulatedShadowOnEmissive
			// render here. The emissive mask will be read from alpha later instead (it was written there in the base pass).
			const UBOOL bCullModulatedShadowOnEmissive= PrimitiveSceneInfo->bCullModulatedShadowOnEmissive && !GModShadowsWithAlphaEmissiveBit;
			const UBOOL bCullModulatedShadowOnBackfaces= PrimitiveSceneInfo->bCullModulatedShadowOnBackfaces;
			if( ViewRelevance.bOpaqueRelevance && 
				(bCullModulatedShadowOnEmissive || bCullModulatedShadowOnBackfaces) )
			{
				// Render dynamic scene prim
				if( ViewRelevance.bDynamicRelevance )
				{	
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(&Drawer,View,DepthPriorityGroup);
				}
				// Render static scene prim
				if( ViewRelevance.bStaticRelevance )
				{
					// Render static meshes from static scene prim
					for( INT StaticMeshIdx=0; StaticMeshIdx < PrimitiveSceneInfo->StaticMeshes.Num(); StaticMeshIdx++ )
					{
						const FStaticMesh& StaticMesh = PrimitiveSceneInfo->StaticMeshes(StaticMeshIdx);
						if (View->StaticMeshVisibilityMap(StaticMesh.Id))
						{
							bDirty |= FMeshModShadowDrawingPolicyFactory::DrawStaticMesh(
								View,
								LightSceneInfo,
								StaticMesh,
								FALSE,
								TRUE,
								PrimitiveSceneInfo,
								StaticMesh.HitProxyId
								);
						}
					}
				}
			}
		}
		
		// Mark dirty if dynamic drawer rendered
		bDirty |= Drawer.IsDirty();
	}

	return bDirty;
}

void FProjectedShadowInfo::RenderFrustumWireframe(FPrimitiveDrawInterface* PDI) const
{
	// Find the ID of an arbitrary subject primitive to use to color the shadow frustum.
	INT SubjectPrimitiveId = 0;
	if(SubjectPrimitives.Num())
	{
		SubjectPrimitiveId = SubjectPrimitives(0)->Id;
	}

	// Render the wireframe for the frustum derived from ReceiverMatrix.
	DrawFrustumWireframe(
		PDI,
		InvReceiverMatrix * FTranslationMatrix(-PreShadowTranslation),
		FColor(FLinearColor::FGetHSV(((SubjectPrimitiveId + LightSceneInfo->Id) * 31) & 255,0,255)),
		SDPG_World
		);
}

/*-----------------------------------------------------------------------------
FSceneRenderer
-----------------------------------------------------------------------------*/

/**
 * Used by RenderLights to figure out if projected shadows need to be rendered to the attenuation buffer.
 *
 * @param LightSceneInfo Represents the current light
 * @return TRUE if anything needs to be rendered
 */
UBOOL FSceneRenderer::CheckForProjectedShadows( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex )
{
	// Find the projected shadows cast by this light.
	FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightSceneInfo->Id);
	for( INT ShadowIndex=0; ShadowIndex<VisibleLightInfo.ProjectedShadows.Num(); ShadowIndex++ )
	{
		const FProjectedShadowInfo* ProjectedShadowInfo = VisibleLightInfo.ProjectedShadows(ShadowIndex);

		// Check that the shadow is visible in at least one view before rendering it.
		UBOOL bShadowIsVisible = FALSE;
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);
			const FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(LightSceneInfo->Id);
			const FPrimitiveViewRelevance ViewRelevance = VisibleLightViewInfo.ProjectedShadowViewRelevanceMap(ShadowIndex);
			bShadowIsVisible |= (ViewRelevance.GetDPG(DPGIndex) && VisibleLightViewInfo.ProjectedShadowVisibilityMap(ShadowIndex));
		}

		if(bShadowIsVisible)
		{
			return TRUE;
		}
	}
	return FALSE;
}
	 
/**
 * Used by RenderLights to render shadows to the attenuation buffer.
 *
 * @param LightSceneInfo Represents the current light
 * @return TRUE if anything got rendered
 */
UBOOL FSceneRenderer::RenderProjectedShadows( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex )
{
	SCOPE_CYCLE_COUNTER(STAT_ProjectedShadowDrawTime);

	UBOOL bAttenuationBufferDirty = FALSE;

	// Find the projected shadows cast by this light.
	FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightSceneInfo->Id);
	TArray<FProjectedShadowInfo*,SceneRenderingAllocator> Shadows;
	for(INT ShadowIndex = 0;ShadowIndex < VisibleLightInfo.ProjectedShadows.Num();ShadowIndex++)
	{
		FProjectedShadowInfo* ProjectedShadowInfo = VisibleLightInfo.ProjectedShadows(ShadowIndex);

		// Check that the shadow is visible in at least one view before rendering it.
		UBOOL bShadowIsVisible = FALSE;
		UBOOL bForegroundCastingOnWorld = FALSE;
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);
			const FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(LightSceneInfo->Id);
			const FPrimitiveViewRelevance ViewRelevance = VisibleLightViewInfo.ProjectedShadowViewRelevanceMap(ShadowIndex);

			// Mark shadows whose subjects are in the foreground DPG but are casting on the world DPG.
			bForegroundCastingOnWorld |= 
				DPGIndex == SDPG_World 
				&& ViewRelevance.GetDPG(SDPG_Foreground) 
				&& GSystemSettings.bEnableForegroundShadowsOnWorld;

			bShadowIsVisible |= (ViewRelevance.GetDPG(DPGIndex) && VisibleLightViewInfo.ProjectedShadowVisibilityMap(ShadowIndex));
		}

		if (DPGIndex == SDPG_Foreground && !GSystemSettings.bEnableForegroundSelfShadowing)
		{
			// Disable foreground self-shadowing based on system settings
			bShadowIsVisible = FALSE;
		}

		if(bShadowIsVisible || bForegroundCastingOnWorld)
		{
			// Add the shadow to the list of visible shadows cast by this light.
			INC_DWORD_STAT(STAT_ProjectedShadows);
			ProjectedShadowInfo->bForegroundCastingOnWorld = bForegroundCastingOnWorld;
			Shadows.AddItem(ProjectedShadowInfo);
		}
	}

	// Sort the projected shadows by resolution.
	Sort<USE_COMPARE_POINTER(FProjectedShadowInfo,ShadowRendering)>(&Shadows(0),Shadows.Num());

	INT PassNumber			= 0;
	INT NumShadowsRendered	= 0;
	while(NumShadowsRendered < Shadows.Num())
	{
		INT NumAllocatedShadows = 0;

		// Allocate shadow texture space to the shadows.
		const UINT ShadowBufferResolution = GSceneRenderTargets.GetShadowDepthTextureResolution();
		FTextureLayout ShadowLayout(1,1,ShadowBufferResolution,ShadowBufferResolution);
		for(INT ShadowIndex = 0;ShadowIndex < Shadows.Num();ShadowIndex++)
		{
			FProjectedShadowInfo* ProjectedShadowInfo = Shadows(ShadowIndex);
			if(!ProjectedShadowInfo->bRendered)
			{
				if(ShadowLayout.AddElement(
					&ProjectedShadowInfo->X,
					&ProjectedShadowInfo->Y,
					ProjectedShadowInfo->ResolutionX + SHADOW_BORDER*2,
					ProjectedShadowInfo->ResolutionY + SHADOW_BORDER*2
					))
				{
					ProjectedShadowInfo->bAllocated = TRUE;
					NumAllocatedShadows++;
#if STATS
					// Keep track of pass number.
					if( bShouldGatherDynamicShadowStats )
					{
						// Disregard passes used to render preshadow as those are taken into account by shadow subject rendering and we wouldn't
						// be able to find the interaction.
						if( !ProjectedShadowInfo->bPreShadow )
						{
							// Brute force find the matching interaction. This is SLOW but fast enough.
							FCombinedShadowStats* ShadowStats = NULL;
							for(TMap<FLightPrimitiveInteraction*,FCombinedShadowStats,SceneRenderingSetAllocator>::TIterator It(
									InteractionToDynamicShadowStatsMap
									);
								It;
								++It)
							{
								FLightPrimitiveInteraction* Interaction = It.Key();
								if( Interaction->GetLight() == LightSceneInfo 
								&&	Interaction->GetPrimitiveSceneInfo() == ProjectedShadowInfo->ParentSceneInfo )
								{
									// Found a match, keep track of address. This is only safe as we are not modifying the TMap below.
									ShadowStats = &It.Value();
								}
							}
							check( ShadowStats );
							// Keep track of pass number.
							ShadowStats->ShadowPassNumber = PassNumber;
						}
					}
#endif
				}
			}
		}

		// Abort if we encounter a shadow that doesn't fit in the render target.
		if(!NumAllocatedShadows)
		{
			break;
		}

#if SUPPORTS_VSM
		const UBOOL bEnableVSMShadows = LightSceneInfo->ShadowProjectionTechnique == ShadowProjTech_VSM || 
			(GSystemSettings.bEnableVSMShadows && LightSceneInfo->ShadowProjectionTechnique == ShadowProjTech_Default);
#else
		const UBOOL bEnableVSMShadows = FALSE;
#endif //#if SUPPORTS_VSM

		// Render the shadow depths.
		{
			SCOPED_DRAW_EVENT(EventShadowDepths)(DEC_SCENE_ITEMS,TEXT("Shadow Depths"));
			
			GSceneRenderTargets.BeginRenderingShadowDepth();			

			// keep track of the max RECT needed for resolving the variance surface	
			FResolveParams ResolveParams;
			ResolveParams.X1 = 0;
			ResolveParams.Y1 = 0;
			UBOOL bResolveParamsInit=FALSE;
			// render depth for each shadow
			for(INT ShadowIndex = 0;ShadowIndex < Shadows.Num();ShadowIndex++)
			{
				FProjectedShadowInfo* ProjectedShadowInfo = Shadows(ShadowIndex);
				if(ProjectedShadowInfo->bAllocated)
				{
					ProjectedShadowInfo->RenderDepth( this, DPGIndex);

					// init values
					if( !bResolveParamsInit )
					{
						ResolveParams.X2 = ProjectedShadowInfo->X + ProjectedShadowInfo->ResolutionX + SHADOW_BORDER*2;
						ResolveParams.Y2 = ProjectedShadowInfo->Y + ProjectedShadowInfo->ResolutionY + SHADOW_BORDER*2;
						bResolveParamsInit=TRUE;
					}
					// keep track of max extents
					else 
					{
						ResolveParams.X2 = Max<UINT>(ProjectedShadowInfo->X + ProjectedShadowInfo->ResolutionX + SHADOW_BORDER*2,ResolveParams.X2);
						ResolveParams.Y2 = Max<UINT>(ProjectedShadowInfo->Y + ProjectedShadowInfo->ResolutionY + SHADOW_BORDER*2,ResolveParams.Y2);
					}
				}
			}

			// only resolve the portion of the shadow buffer that we rendered to
			GSceneRenderTargets.FinishRenderingShadowDepth(ResolveParams);
		}

#if SUPPORTS_VSM
		// Render shadow variance 
		if( bEnableVSMShadows )
		{	
			SCOPED_DRAW_EVENT(EventShadowVariance)(DEC_SCENE_ITEMS,TEXT("Shadow Variance"));

			RenderShadowVariance(this,Shadows,DPGIndex);
		}
#endif //#if SUPPORTS_VSM

		// Render the shadow projections.
		{
			SCOPED_DRAW_EVENT(EventShadowProj)(DEC_SCENE_ITEMS,TEXT("Shadow Projection"));

			if( LightSceneInfo->LightShadowMode == LightShadow_Modulate ||
				LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter )
			{
				GSceneRenderTargets.BeginRenderingSceneColor(FALSE);
			}
			else
			{
				GSceneRenderTargets.BeginRenderingLightAttenuation();
			}

			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{
				SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

				const FViewInfo& View = Views(ViewIndex);

				// Set the device viewport for the view.
				RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
				RHISetViewParameters( &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

				// Set the light's scissor rectangle.
				LightSceneInfo->SetScissorRect(&View);

				// Project the shadow depth buffers onto the scene.
				for(INT ShadowIndex = 0;ShadowIndex < Shadows.Num();ShadowIndex++)
				{
					FProjectedShadowInfo* ProjectedShadowInfo = Shadows(ShadowIndex);
					if(ProjectedShadowInfo->bAllocated)
					{
						// Only project the shadow in its subject's DPG.
						ProjectedShadowInfo->RenderProjection( &View, DPGIndex);
					}
				}

				// Reset the scissor rectangle.
				RHISetScissorRect(FALSE,0,0,0,0);
			}

			// Mark and count the rendered shadows.
			for(INT ShadowIndex = 0;ShadowIndex < Shadows.Num();ShadowIndex++)
			{
				FProjectedShadowInfo* ProjectedShadowInfo = Shadows(ShadowIndex);
				if(ProjectedShadowInfo->bAllocated)
				{
					ProjectedShadowInfo->bAllocated = FALSE;
					// If we are in the world DPG and the subject is in the foreground DPG,
					// render the shadow again in the foreground DPG to handle self-shadowing.
					if (!ProjectedShadowInfo->bForegroundCastingOnWorld 
						|| GSystemSettings.bEnableForegroundSelfShadowing && DPGIndex == SDPG_Foreground)
					{
						ProjectedShadowInfo->bRendered = TRUE;
					}
					NumShadowsRendered++;
				}
			}

			// Mark the attenuation buffer as dirty.
			bAttenuationBufferDirty = TRUE;
		}

		// Increment pass number used by stats.
		PassNumber++;
	}
	return bAttenuationBufferDirty;
}

/*-----------------------------------------------------------------------------
	FMeshModShadowDrawingPolicy
-----------------------------------------------------------------------------*/

//** modulated shadow mesh attenuation shader implementations */
IMPLEMENT_MATERIAL_SHADER_TYPE(,FModShadowMeshVertexShader,TEXT("ModShadowMeshAttenuationVS"),TEXT("Main"),SF_Vertex,0,0);
IMPLEMENT_MATERIAL_SHADER_TYPE(,FModShadowMeshPixelShader,TEXT("ModShadowMeshAttenuationPS"),TEXT("Main"),SF_Pixel,VER_MODSHADOWMESHPIXELSHADER_ATTENALLOWED,0);

/**
* Constructor
* @param InIndexBuffer - index buffer for rendering
* @param InVertexFactory - vertex factory for rendering
* @param InMaterialRenderProxy - material instance for rendering
* @param InLight - info about light casting a shadow on this mesh
*/
FMeshModShadowDrawingPolicy::FMeshModShadowDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy,
	const FLightSceneInfo* InLight )
	:	FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy)
	,	Light(InLight)
{
	const FMaterial* MaterialResource = InMaterialRenderProxy->GetMaterial();
	VertexShader = MaterialResource->GetShader<FModShadowMeshVertexShader>(InVertexFactory->GetType());
	PixelShader = MaterialResource->GetShader<FModShadowMeshPixelShader>(InVertexFactory->GetType());
}

/**
* Match two draw policies
* @param Other - draw policy to compare
* @return TRUE if the draw policies are a match
*/
UBOOL FMeshModShadowDrawingPolicy::Matches(const FMeshModShadowDrawingPolicy& Other) const
{
	return	VertexShader == Other.VertexShader &&
			PixelShader == Other.PixelShader &&
			Light == Other.Light &&
			FMeshDrawingPolicy::Matches(Other);
}

/**
* Executes the draw commands which can be shared between any meshes using this drawer.
* @param CI - The command interface to execute the draw commands on.
* @param View - The view of the scene being drawn.
*/
void FMeshModShadowDrawingPolicy::DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
{
	// Set the material shader parameters for the material instance
	VertexShader->SetParameters(VertexFactory,MaterialRenderProxy,View);
	PixelShader->SetParameters(VertexFactory,MaterialRenderProxy,View);

	// Set shared mesh resources
	FMeshDrawingPolicy::DrawShared(View);

	// Set the actual shader & vertex declaration state
	RHISetBoundShaderState( BoundShaderState);
}

/** 
* Create bound shader state using the vertex decl from the mesh draw policy
* as well as the shaders needed to draw the mesh
* @param DynamicStride - optional stride for dynamic vertex data
* @return new bound shader state object
*/
FBoundShaderStateRHIRef FMeshModShadowDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);
	if (DynamicStride)
	{
		StreamStrides[0] = DynamicStride;
	}

	return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());

}

INT Compare(const FMeshModShadowDrawingPolicy& A,const FMeshModShadowDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialRenderProxy);
	return 0;
}

/**
* Sets the render states for drawing a mesh.
* @param PrimitiveSceneInfo - The primitive drawing the dynamic mesh.  If this is a view element, this will be NULL.
* @param Mesh - mesh element with data needed for rendering
* @param ElementData - context specific data for mesh rendering
*/
void FMeshModShadowDrawingPolicy::SetMeshRenderState(
	const FSceneView& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	EmitMeshDrawEvents(PrimitiveSceneInfo, Mesh);

	// set light position
	if( Light->LightType == LightType_Directional )
	{
		const FLOAT BigNumber = 10000.f;
		FVector LightPosition = Light->GetDirection().SafeNormal() * (-BigNumber) + Mesh.LocalToWorld.GetOrigin() + View.PreViewTranslation;
		VertexShader->SetLightPosition(VertexShader,FVector4(LightPosition,1.0f));
	}
	else
	{
		VertexShader->SetLightPosition(VertexShader,FVector4(Light->GetOrigin() + View.PreViewTranslation,1.0f));
	}

	// Set transforms
	VertexShader->SetMesh(Mesh,View);
	PixelShader->SetMesh(Mesh,View,bBackFace,*PrimitiveSceneInfo);

	// Set rasterizer state.
	const FRasterizerStateInitializerRHI Initializer = {
		FM_Solid,
		IsTwoSided() ? CM_None : (XOR( XOR(View.bReverseCulling,bBackFace), Mesh.ReverseCulling) ? CM_CCW : CM_CW),
		Mesh.DepthBias,
		Mesh.SlopeScaleDepthBias
	};
	RHISetRasterizerStateImmediate( Initializer);
}

/*-----------------------------------------------------------------------------
	FMeshModShadowDrawingPolicyFactory
-----------------------------------------------------------------------------*/

/**
* Render a dynamic mesh using the mod shadow mesh mesh draw policy 
*
* @return TRUE if the mesh rendered
*/
UBOOL FMeshModShadowDrawingPolicyFactory::DrawDynamicMesh(
	const FSceneView& View,
	const FLightSceneInfo* Light,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	UBOOL bDirty=FALSE;
	if( !bBackFace && 
		!Mesh.IsTranslucent() &&  
		Mesh.MaterialRenderProxy && 
		Mesh.MaterialRenderProxy->GetMaterial() && 
		Mesh.MaterialRenderProxy->GetMaterial()->GetLightingModel() != MLM_Unlit &&
		// Don't setup the mask for non-directional materials, since their lighting is independent of their normal.
		Mesh.MaterialRenderProxy->GetMaterial()->GetLightingModel() != MLM_NonDirectional)
	{
		FMeshModShadowDrawingPolicy DrawingPolicy(Mesh.VertexFactory, Mesh.MaterialRenderProxy,Light);
		DrawingPolicy.DrawShared(&View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
		DrawingPolicy.SetMeshRenderState(View, PrimitiveSceneInfo, Mesh, bBackFace, FMeshDrawingPolicy::ElementDataType());
		DrawingPolicy.DrawMesh(Mesh);
		bDirty = TRUE;
	}
	return bDirty;
}

/**
* Render a static mesh using the mod shadow mesh mesh draw policy 
*
* @return TRUE if the mesh rendered
*/
UBOOL FMeshModShadowDrawingPolicyFactory::DrawStaticMesh(
	const FSceneView* View,
	const FLightSceneInfo* Light,
	const FStaticMesh& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	UBOOL bDirty=FALSE;
	if( !bBackFace && 
		!Mesh.IsTranslucent() &&  
		Mesh.MaterialRenderProxy && 
		Mesh.MaterialRenderProxy->GetMaterial() && 
		Mesh.MaterialRenderProxy->GetMaterial()->GetLightingModel() != MLM_Unlit &&
		Mesh.MaterialRenderProxy->GetMaterial()->GetLightingModel() != MLM_NonDirectional)
	{
		FMeshModShadowDrawingPolicy DrawingPolicy(Mesh.VertexFactory, Mesh.MaterialRenderProxy,Light);
		DrawingPolicy.DrawShared(View, DrawingPolicy.CreateBoundShaderState());
		DrawingPolicy.SetMeshRenderState(*View, PrimitiveSceneInfo, Mesh, bBackFace, FMeshDrawingPolicy::ElementDataType());
		DrawingPolicy.DrawMesh(Mesh);
		bDirty = TRUE;
	}
	return bDirty;
}

/**
* Skip materials which are not lit or don't use opaque/masked blending 
* since these shouldn't be affected by the shadow
*
* @param MaterialRenderProxy - current material proxy being rendered for a mesh element
* @return TRUE if the material should be ignored
*/
UBOOL FMeshModShadowDrawingPolicyFactory::IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy)
{
	return	MaterialRenderProxy && 
			(IsTranslucentBlendMode(MaterialRenderProxy->GetMaterial()->GetBlendMode()) ||
			MaterialRenderProxy->GetMaterial()->GetLightingModel() == MLM_Unlit ||
			MaterialRenderProxy->GetMaterial()->GetLightingModel() == MLM_NonDirectional);
}


