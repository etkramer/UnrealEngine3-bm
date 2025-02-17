/*=============================================================================
	VelocityRendering.cpp: Velocity rendering implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

/** The minimum projected screen radius for a primitive to be drawn in the velocity pass, as a fraction of half the horizontal screen width. */
const FLOAT MinScreenRadiusForVelocityPass = 0.075f;
const FLOAT MinDistanceToDropVelocityPass = 768.0f;
FLOAT MinScreenRadiusForVelocityPassSquared = Square(MinScreenRadiusForVelocityPass);
FLOAT MinDistanceToDropVelocityPassSquared = Square(MinDistanceToDropVelocityPass);

//=============================================================================
/** Encapsulates the Velocity vertex shader. */
class FVelocityVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FVelocityVertexShader,MeshMaterial);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		//Only compile the velocity shaders for the default material or if it's masked,
		return (Material->IsSpecialEngineMaterial() || Material->IsMasked() 
			//or if the material is opaque and two-sided,
			|| (Material->IsTwoSided() && !IsTranslucentBlendMode(Material->GetBlendMode()))) 
			//and exclude decal materials.
			&& !Material->IsDecalMaterial();
	}

	FVelocityVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		PrevViewProjectionMatrixParameter.Bind(Initializer.ParameterMap,TEXT("PrevViewProjectionMatrix"));
		PreviousLocalToWorldParameter.Bind(Initializer.ParameterMap,TEXT("PreviousLocalToWorld"),TRUE);
		StretchTimeScaleParameter.Bind(Initializer.ParameterMap,TEXT("StretchTimeScale"),TRUE);
	}
	FVelocityVertexShader() {}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		bShaderHasOutdatedParameters |= Ar << VertexFactoryParameters;
		Ar << PrevViewProjectionMatrixParameter;
		Ar << PreviousLocalToWorldParameter;
		Ar << StretchTimeScaleParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FViewInfo& View)
	{
		FSceneViewState* ViewState = (FSceneViewState*) View.State;

		VertexFactoryParameters.Set( this, VertexFactory, View );

		// Set the view-projection matrix from the previous frame.
		SetVertexShaderValue(  GetVertexShader(), PrevViewProjectionMatrixParameter, View.PrevViewProjMatrix );

		// NOTE: This bias happens before any z-invert in the vertex shader, so a negative value with pull it closer to the camera on all platforms.
		static FLOAT DefaultDepthBias = -0.001f;
		const FLOAT DepthBias = DefaultDepthBias;

		// Set the time scale for stretching.
		extern INT GMotionBlurFullMotionBlur;
		UBOOL bFullMotionBlur = GMotionBlurFullMotionBlur < 0 ? View.MotionBlurParameters.bFullMotionBlur : (GMotionBlurFullMotionBlur > 0);
		if ( bFullMotionBlur )
		{
			SetVertexShaderValue(  GetVertexShader(), StretchTimeScaleParameter, FVector4(0,DepthBias,0,0) );
		}
		else
		{
			FLOAT StretchTimeScale = ViewState->MotionBlurTimeScale * View.MotionBlurParameters.VelocityScale * 1.6f;
			SetVertexShaderValue(  GetVertexShader(), StretchTimeScaleParameter, FVector4(StretchTimeScale,DepthBias,0,0) );
		}
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View,const FMatrix& PreviousLocalToWorld)
	{
		VertexFactoryParameters.SetMesh(this,Mesh,View);
		SetVertexShaderValue( GetVertexShader(), PreviousLocalToWorldParameter, PreviousLocalToWorld);
	}

	UBOOL SupportsVelocity( ) const
	{
		return PreviousLocalToWorldParameter.IsBound() && StretchTimeScaleParameter.IsBound();
	}

private:
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
//		new(OutEnvironment.CompilerFlags) ECompilerFlags(CFLAG_Debug);
	}

	FVertexFactoryParameterRef	VertexFactoryParameters;
	FShaderParameter			PrevViewProjectionMatrixParameter;
	FShaderParameter			PreviousLocalToWorldParameter;
	FShaderParameter			StretchTimeScaleParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FVelocityVertexShader,TEXT("VelocityShader"),TEXT("MainVertexShader"),SF_Vertex,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);


//=============================================================================
/** Encapsulates the Velocity pixel shader. */
class FVelocityPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FVelocityPixelShader,MeshMaterial);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		//Only compile the velocity shaders for the default material or if it's masked,
		return (Material->IsSpecialEngineMaterial() || Material->IsMasked() 
			//or if the material is opaque and two-sided,
			|| (Material->IsTwoSided() && !IsTranslucentBlendMode(Material->GetBlendMode())))
			//and exclude decal materials.
			&& !Material->IsDecalMaterial();
	}

	FVelocityPixelShader() {}

	FVelocityPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		VelocityScaleOffset.Bind( Initializer.ParameterMap, TEXT("VelocityScaleOffset"), TRUE );
		IndividualVelocityScale.Bind( Initializer.ParameterMap, TEXT("IndividualVelocityScale"), TRUE );
	}

	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FViewInfo& View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy, View.Family->CurrentWorldTime, View.Family->CurrentRealTime, &View);
		MaterialParameters.Set( this, MaterialRenderContext );

//		SetPixelShaderValue( GetPixelShader(), VelocityScaleOffset, FVector4( ScaleX*0.5f, ScaleY*0.5f, 0.5f, 0.5f ) );
	}

	void SetMesh(const FMeshElement& Mesh,const FViewInfo& View,UBOOL bBackFace,FLOAT VelocityScale)
	{
		MaterialParameters.SetMesh( this, Mesh, View, bBackFace );

		// Calculate the maximum velocity (MAX_PIXELVELOCITY is per 30 fps frame).
		FSceneViewState* ViewState = (FSceneViewState*) View.State;
		const FLOAT SizeX	= View.SizeX;
		const FLOAT SizeY	= View.SizeY;
		VelocityScale		*= ViewState->MotionBlurTimeScale * View.MotionBlurParameters.VelocityScale;
		FLOAT MaxVelocity	= MAX_PIXELVELOCITY * View.MotionBlurParameters.MaxVelocity;
		FLOAT ScaleX		= ( MaxVelocity > 0.0001f ) ? 0.5f*VelocityScale/MaxVelocity : 0.0f;
		FLOAT ScaleY		= ( MaxVelocity > 0.0001f ) ? 0.5f*VelocityScale/(MaxVelocity*SizeY/SizeX) : 0.0f;

		// Raw clamp values for the velocity buffer, since 0.0f means "use static background velocity".
		static FLOAT MinimumValue = 1.0f/255.0f;
		static FLOAT MaximumValue = 1.0f;

		// xy = scale values
		// zw = clamp values
		FVector4 VelocityScaleClamp( ScaleX, ScaleY, MinimumValue, MaximumValue );
		SetPixelShaderValue( GetPixelShader(), IndividualVelocityScale, VelocityScaleClamp );
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters << VelocityScaleOffset << IndividualVelocityScale;
		return bShaderHasOutdatedParameters;
	}

private:
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
//		new(OutEnvironment.CompilerFlags) ECompilerFlags(CFLAG_Debug);
	}

	FMaterialPixelShaderParameters	MaterialParameters;
	FShaderParameter				VelocityScaleOffset;
	FShaderParameter				IndividualVelocityScale;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FVelocityPixelShader,TEXT("VelocityShader"),TEXT("MainPixelShader"),SF_Pixel,VER_BACKGROUNDVELOCITYVALUE,0);


//=============================================================================
/** FVelocityDrawingPolicy - Policy to wrap FMeshDrawingPolicy with new shaders. */

FVelocityDrawingPolicy::FVelocityDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy
	)
	:	FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy)
{
	const FMaterialShaderMap* MaterialShaderIndex = InMaterialRenderProxy->GetMaterial()->GetShaderMap();
	const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
	UBOOL HasVertexShader = MeshShaderIndex->HasShader(&FVelocityVertexShader::StaticType);
	UBOOL HasPixelShader = MeshShaderIndex->HasShader(&FVelocityPixelShader::StaticType);
	VertexShader = HasVertexShader ? MeshShaderIndex->GetShader<FVelocityVertexShader>() : NULL;
	PixelShader = HasPixelShader ? MeshShaderIndex->GetShader<FVelocityPixelShader>() : NULL;
}

UBOOL FVelocityDrawingPolicy::SupportsVelocity() const
{
	return (VertexShader && PixelShader) ? VertexShader->SupportsVelocity() : FALSE;
}

void FVelocityDrawingPolicy::DrawShared( const FSceneView* SceneView, FBoundShaderStateRHIRef ShaderState ) const
{
	// NOTE: Assuming this cast is always safe!
	FViewInfo* View = (FViewInfo*) SceneView;

	// Set the depth-only shader parameters for the material.
	RHISetBoundShaderState( ShaderState );
	VertexShader->SetParameters( VertexFactory, MaterialRenderProxy, *View );
	PixelShader->SetParameters( VertexFactory, MaterialRenderProxy, *View );

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared( View );
}

void FVelocityDrawingPolicy::SetMeshRenderState(
	const FViewInfo& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	const FMotionBlurInfo* MotionBlurInfo;
	if ( FScene::GetPrimitiveMotionBlurInfo(PrimitiveSceneInfo, MotionBlurInfo) == TRUE )
	{
		VertexShader->SetMesh(Mesh,View,MotionBlurInfo->PreviousLocalToWorld);
	}
	else
	{
		VertexShader->SetMesh(Mesh,View,Mesh.LocalToWorld);
	}
	PixelShader->SetMesh(Mesh,View,bBackFace,PrimitiveSceneInfo->Component->MotionBlurScale);
	FMeshDrawingPolicy::SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,ElementData);
}

/** Determines whether this primitive has motionblur velocity to render */
UBOOL FVelocityDrawingPolicy::HasVelocity(const FViewInfo& View, const FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	// No velocity if motionblur is off, or if it's a non-moving object (treat as background in that case)
	if ( !View.bRequiresVelocities || PrimitiveSceneInfo->bStaticShadowing )
	{
		return FALSE;
	}

	// Foreground primitives can always be drawn (0 velocity)
	const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);
	if ( PrimitiveViewRelevance.GetDPG(SDPG_Foreground) )
	{
		return TRUE;
	}

	// If the motionblurscale is default (1.0), check if it has moved
	if ( Abs(PrimitiveSceneInfo->Component->MotionBlurScale - 1.0f) < 0.0001f )
	{
		const FMatrix* CheckMatrix = NULL;
		const FMotionBlurInfo* MBInfo;
		const UPrimitiveComponent* Component = PrimitiveSceneInfo->Component;
		if ( FScene::GetPrimitiveMotionBlurInfo(PrimitiveSceneInfo, MBInfo) == TRUE )
		{
			CheckMatrix = &(MBInfo->PreviousLocalToWorld);
		}

		// Hasn't moved (treat as background by not rendering any special velocities)?
		if ( CheckMatrix == NULL || Component->LocalToWorld.Equals(*CheckMatrix, 0.0001f) )
		{
			return FALSE;
		}
	}

	return TRUE;
}


/** 
 * Create bound shader state using the vertex decl from the mesh draw policy
 * as well as the shaders needed to draw the mesh
 * @param DynamicStride - optional stride for dynamic vertex data
 * @return new bound shader state object
 */
FBoundShaderStateRHIRef FVelocityDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
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

INT Compare(const FVelocityDrawingPolicy& A,const FVelocityDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialRenderProxy);
	return 0;
}


//=============================================================================
/** Policy to wrap FMeshDrawingPolicy with new shaders. */

void FVelocityDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType)
{
	// Velocity only needs to be directly rendered for movable meshes.
	if (!StaticMesh->PrimitiveSceneInfo->bStaticShadowing)
	{
	    const FMaterialRenderProxy* MaterialRenderProxy = StaticMesh->MaterialRenderProxy;
	    const FMaterial* Material = MaterialRenderProxy->GetMaterial();
	    EBlendMode BlendMode = Material->GetBlendMode();
	    if ( (BlendMode == BLEND_Opaque || BlendMode == BLEND_Masked) && !Material->IsDecalMaterial() )
	    {
		    if ( !Material->IsMasked() && !Material->IsTwoSided() )
		    {
			    // Default material doesn't handle masked, and doesn't have the correct bIsTwoSided setting.
			    MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
		    }

			FVelocityDrawingPolicy DrawingPolicy(StaticMesh->VertexFactory,MaterialRenderProxy);
			if (DrawingPolicy.SupportsVelocity())
			{
				// Add the static mesh to the depth-only draw list.
				Scene->DPGs[StaticMesh->DepthPriorityGroup].VelocityDrawList.AddMesh(
					StaticMesh,
					FVelocityDrawingPolicy::ElementDataType(),
					DrawingPolicy
					);
			}
	    }
	}
}

UBOOL FVelocityDrawingPolicyFactory::DrawDynamicMesh(
	const FViewInfo& View,
	ContextType DrawingContext,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	// Only draw opaque materials in the depth pass.
	const FMaterialRenderProxy* MaterialRenderProxy = Mesh.MaterialRenderProxy;
	const FMaterial* Material = MaterialRenderProxy->GetMaterial();
	EBlendMode BlendMode = Material->GetBlendMode();
	if ( BlendMode == BLEND_Opaque || BlendMode == BLEND_Masked && !Material->IsDecalMaterial() )
	{
		// This should be enforced at a higher level
		//@todo - figure out why this is failing and re-enable
		//check(FVelocityDrawingPolicy::HasVelocity(View, PrimitiveSceneInfo));
		if ( !Material->IsMasked() && !Material->IsTwoSided() )
		{
			// Default material doesn't handle masked, and doesn't have the correct bIsTwoSided setting.
			MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
		}
		FVelocityDrawingPolicy DrawingPolicy( Mesh.VertexFactory, MaterialRenderProxy );
		if ( DrawingPolicy.SupportsVelocity() )
		{
			DrawingPolicy.DrawShared(&View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			DrawingPolicy.SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Mesh);
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		return FALSE;
	}
}

/** Renders the velocities of movable objects for the motion blur effect. */
void FSceneRenderer::RenderVelocities(UINT DPGIndex)
{
	SCOPED_DRAW_EVENT(EventVelocities)(DEC_SCENE_ITEMS,TEXT("RenderVelocities"));
	SCOPE_CYCLE_COUNTER(STAT_VelocityDrawTime);

	UBOOL bWroteVelocities = FALSE;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);
		if ( View.bRequiresVelocities && View.bPrevTransformsReset == FALSE )
		{
			SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

			const FSceneViewState* ViewState = (FSceneViewState*)View.State;

			if ( !bWroteVelocities )
			{
				GSceneRenderTargets.BeginRenderingVelocities();
				bWroteVelocities = TRUE;
			}

			// Set the device viewport for the view.
			const UINT BufferSizeX = GSceneRenderTargets.GetBufferSizeX();
			const UINT BufferSizeY = GSceneRenderTargets.GetBufferSizeY();
			const UINT VelocityBufferSizeX = GSceneRenderTargets.GetVelocityBufferSizeX();
			const UINT VelocityBufferSizeY = GSceneRenderTargets.GetVelocityBufferSizeY();
			const UINT MinX = View.RenderTargetX * VelocityBufferSizeX / BufferSizeX;
			const UINT MinY = View.RenderTargetY * VelocityBufferSizeY / BufferSizeY;
			const UINT MaxX = (View.RenderTargetX + View.RenderTargetSizeX) * VelocityBufferSizeX / BufferSizeX;
			const UINT MaxY = (View.RenderTargetY + View.RenderTargetSizeY) * VelocityBufferSizeY / BufferSizeY;
			RHISetViewport(MinX,MinY,0.0f,MaxX,MaxY,1.0f);
			RHISetViewParameters(  &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

			// Clear the velocity buffer (0.0f means "use static background velocity").
			RHIClear(TRUE,FLinearColor(0.0f, 0.0f, 0.0f, 0.0f),FALSE,1.0f,FALSE,0);

			// Opaque blending, use depth tests, no z-writes, backface-culling. Depth-bias doesn't seem to work. :(
			RHISetBlendState(TStaticBlendState<BO_Max,BF_One,BF_One>::GetRHI());
			RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
			static FLOAT DepthBias = 0.0f;	//-0.01f;
			const FRasterizerStateInitializerRHI RasterState = { FM_Solid,CM_CW, DepthBias, 0.0f };
			RHISetRasterizerStateImmediate( RasterState);

			// Draw velocities for movable static meshes.
			bWroteVelocities |= Scene->DPGs[DPGIndex].VelocityDrawList.DrawVisible(View,View.StaticMeshVelocityMap);

			// Draw velocities for movable dynamic meshes.
			TDynamicPrimitiveDrawer<FVelocityDrawingPolicyFactory> Drawer(&View,DPGIndex,FVelocityDrawingPolicyFactory::ContextType(),TRUE);
			for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
			{
				const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
				const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);
				const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
				const FLOAT LODFactorDistanceSquared = (PrimitiveSceneInfo->Bounds.Origin - View.ViewOrigin).SizeSquared() * Square(View.LODDistanceFactor);

				// Only render if visible.
				if( bVisible 
				// Used to determine whether object is movable or not.
				&& !PrimitiveSceneInfo->bStaticShadowing 
				// Skip translucent objects as they don't support velocities and in the case of particles have a significant CPU overhead.
				&& PrimitiveViewRelevance.bOpaqueRelevance 
				// Only render if primitive is relevant to the current DPG.
				&& PrimitiveViewRelevance.GetDPG(DPGIndex) 
				// Skip primitives that only cover a small amount of screenspace, motion blur on them won't be noticeable.
				&& ( MinDistanceToDropVelocityPassSquared > LODFactorDistanceSquared  ||
					 Square(PrimitiveSceneInfo->Bounds.SphereRadius) > MinScreenRadiusForVelocityPassSquared * LODFactorDistanceSquared )
				// Only render primitives with velocity.
				&& FVelocityDrawingPolicy::HasVelocity(View, PrimitiveSceneInfo) )
				{
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&Drawer,
						&View,
						DPGIndex
						);
				}
			}
			bWroteVelocities |= Drawer.IsDirty();

			// For each view we draw the foreground DPG dynamic objects into the velocity buffer so that we can do the motion blur shader after all DPGs
			// are finished.
			//
			// Note that we simply write a velocity of 0 without any regard to the depth buffer.

			static bool bDrawForegroundVelocities = true;
			if (bDrawForegroundVelocities)
			{
				RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());

				FLOAT OldMaxVelocity = View.MotionBlurParameters.MaxVelocity;
				View.MotionBlurParameters.MaxVelocity = 0.f;

				// Draw velocities for movable dynamic meshes.
				TDynamicPrimitiveDrawer<FVelocityDrawingPolicyFactory> ForegroundDrawer(&View,SDPG_Foreground,FVelocityDrawingPolicyFactory::ContextType(),TRUE);

				for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
				{
					const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
					const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);
					
					// Used to determine whether object is movable or not.
					if( !PrimitiveSceneInfo->bStaticShadowing 
					// Skip translucent objects as they don't support velocities and in the case of particles have a significant CPU overhead.
					&& PrimitiveViewRelevance.bOpaqueRelevance 
					// Only render if primitive is relevant to the foreground DPG.
					&& PrimitiveViewRelevance.GetDPG(SDPG_Foreground)
					// Only render primitives with velocity.
					&& FVelocityDrawingPolicy::HasVelocity(View, PrimitiveSceneInfo) )
					{
						ForegroundDrawer.SetPrimitive(PrimitiveSceneInfo);
						PrimitiveSceneInfo->Proxy->DrawDynamicElements(&ForegroundDrawer,&View,SDPG_Foreground);
					}
				}
				bWroteVelocities |= ForegroundDrawer.IsDirty();
				View.MotionBlurParameters.MaxVelocity = OldMaxVelocity;

				RHISetDepthState(TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
			}
		}
	}

	if ( bWroteVelocities )
	{
		RHISetBlendState( TStaticBlendState<>::GetRHI());
		RHISetRasterizerState( TStaticRasterizerState<>::GetRHI());
		GSceneRenderTargets.FinishRenderingVelocities();
	}
}
