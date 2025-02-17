/*=============================================================================
	FluidSurfaceRender.cpp: Fluid surface rendering.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "LocalVertexFactoryShaderParms.h"
#include "EngineFluidClasses.h"
#include "FluidSurface.h"

#define ENABLE_TRACKING 0


//@DEBUG
DOUBLE		GTrackStartTime = 0.0;	// seconds
FTrackEvent	GTrackEvents[32];
INT			GCurrentTrackEvent = 0;
FCriticalSection	GTrackEventLock;

void AddTrackEvent( ETrackEventType Type, INT ID )
{
#if ENABLE_TRACKING
	FScopeLock ScopeLock( &GTrackEventLock );
	DOUBLE Time = appSeconds();
	if ( GTrackStartTime < 0.1 )
	{
		GTrackStartTime = Time;
	}
	GTrackEvents[ GCurrentTrackEvent ].Type = Type;
	GTrackEvents[ GCurrentTrackEvent ].ID = ID;
	GTrackEvents[ GCurrentTrackEvent ].Time = (Time - GTrackStartTime) * 1000.0;
	GCurrentTrackEvent = (GCurrentTrackEvent + 1) % 32;
#endif
}


/*=============================================================================
	FFluidVertexFactoryShaderParameters
=============================================================================*/

/**
 *	Vertexshader parameters for fluids.
 */
class FFluidVertexFactoryShaderParameters : public FLocalVertexFactoryShaderParameters
{
public:
	virtual void	Bind(const FShaderParameterMap& ParameterMap);
	virtual void	Serialize(FArchive& Ar);
	virtual void	Set(FShader* VertexShader, const FVertexFactory* VertexFactory, const FSceneView& View) const;

private:
 	FShaderParameter	GridSizeParameter;
	FShaderParameter	TessellationParameters;
	FShaderParameter	TessellationFactors1;
	FShaderParameter	TessellationFactors2;
	FShaderParameter	TexcoordScaleBias;
	FShaderParameter	SplineParameters;
	FShaderResourceParameter HeightmapParameter;
};

void FFluidVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	FLocalVertexFactoryShaderParameters::Bind( ParameterMap );
	GridSizeParameter.Bind( ParameterMap, TEXT("GridSize") );

	// Xbox-specific parameters.
	TessellationParameters.Bind( ParameterMap, TEXT("TessellationParameters"), TRUE );
	TessellationFactors1.Bind( ParameterMap, TEXT("TessellationFactors1"), TRUE );
	TessellationFactors2.Bind( ParameterMap, TEXT("TessellationFactors2"), TRUE );
	HeightmapParameter.Bind( ParameterMap, TEXT("Heightmap"), TRUE );
	TexcoordScaleBias.Bind( ParameterMap, TEXT("TexcoordScaleBias"), TRUE );
	SplineParameters.Bind( ParameterMap, TEXT("SplineParameters"), TRUE );
}

void FFluidVertexFactoryShaderParameters::Serialize(FArchive& Ar)
{
	FLocalVertexFactoryShaderParameters::Serialize( Ar );
	Ar << GridSizeParameter;
	Ar << TessellationParameters;
	Ar << HeightmapParameter;
	Ar << TessellationFactors1;
	Ar << TessellationFactors2;
	Ar << TexcoordScaleBias;
	Ar << SplineParameters;
}

void FFluidVertexFactoryShaderParameters::Set(FShader* VertexShader, const FVertexFactory* VertexFactory, const FSceneView& View) const
{
	FLocalVertexFactoryShaderParameters::Set( VertexShader, VertexFactory, View );

	FFluidVertexFactory* FluidVertexFactory = (FFluidVertexFactory*) VertexFactory;
	const FFluidSimulation* FluidSimulation = FluidVertexFactory->GetSimulation();
	SetVertexShaderValue( VertexShader->GetVertexShader(), GridSizeParameter, FluidSimulation->GetGridSize() );

#if XBOX && USE_XeD3D_RHI
	if ( FluidVertexFactory->UseGPUTessellation() )
	{
		static FLOAT SplineMargin = 0.1f;
		FVector4 SplineParameterValue( 0.5f - SplineMargin, 1.0f/SplineMargin, 0.0f, 0.0f );
		SetVertexShaderValue( VertexShader->GetVertexShader(), TessellationParameters, FluidSimulation->GetTessellationParameters() );
		SetVertexShaderValue( VertexShader->GetVertexShader(), TessellationFactors1, FluidSimulation->GetTessellationFactors1() );
		SetVertexShaderValue( VertexShader->GetVertexShader(), TessellationFactors2, FluidSimulation->GetTessellationFactors2() );
		SetVertexShaderValue( VertexShader->GetVertexShader(), TexcoordScaleBias, FluidSimulation->GetTexcoordScaleBias() );
		SetVertexShaderValue( VertexShader->GetVertexShader(), SplineParameters, SplineParameterValue );

		static UBOOL bUseLinearFilter = TRUE;
		extern IDirect3DDevice9* GDirect3DDevice;
		if ( bUseLinearFilter )
		{
			SetTextureParameter( NULL, HeightmapParameter, TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(), FluidVertexFactory->GetHeightmapTexture() );
			GDirect3DDevice->SetSamplerState( HeightmapParameter.GetBaseIndex(), D3DSAMP_BORDERCOLOR, 0x00000000 );
			GDirect3DDevice->SetSamplerState( HeightmapParameter.GetBaseIndex(), D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER );
		}
		else
		{
			SetTextureParameter( NULL, HeightmapParameter, TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(), FluidVertexFactory->GetHeightmapTexture() );
		}
	}
#endif
}


IMPLEMENT_VERTEX_FACTORY_TYPE(FFluidVertexFactory,FFluidVertexFactoryShaderParameters,"FluidVertexFactory",TRUE,TRUE,FALSE,VER_FLUID_VERTEX_FACTORY,0);
IMPLEMENT_VERTEX_FACTORY_TYPE(FFluidTessellationVertexFactory,FFluidVertexFactoryShaderParameters,"FluidVertexFactory",TRUE,TRUE,FALSE,VER_FLUID_VERTEX_FACTORY,0);


/*=============================================================================
	FFluidVertexDeclaration
=============================================================================*/

/**
 * Fluid vertex declaration resource type.
 */
class FFluidVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		INT	Offset = 0;

		/** Vertex height */
		Elements.AddItem(FVertexElement(0,Offset,VET_Float1,VEU_Position,0));
		Offset += sizeof(FLOAT);
		/** Texture coordinate*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float2,VEU_TextureCoordinate,0));
		Offset += sizeof(FVector2D);
		/** HeightDelta */
		Elements.AddItem(FVertexElement(0,Offset,VET_Float2,VEU_Tangent,0));
		Offset += sizeof(FVector2D);

		// Create the vertex declaration for rendering the factory normally.
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/** The vertex declaration used by all fluidsurfaces. */
static TGlobalResource<FFluidVertexDeclaration> GFluidVertexDeclaration;


/*=============================================================================
	FFluidMaterialRenderProxy
=============================================================================*/

/**
 * A material render proxy that sets parameters needed by the fluid normal material node.
 */
class FFluidMaterialRenderProxy : public FMaterialRenderProxy
{
private:
	const FMaterialRenderProxy* const Parent;
	FTexture FluidNormalTexture;
	FLinearColor DetailCoordOffset;
	FLinearColor DetailCoordScale;

public:

	/** Initialization constructor. */
	FFluidMaterialRenderProxy(const FMaterialRenderProxy* InParent, const FFluidSimulation* FluidSimulation) :
		Parent(InParent)
	{
		FVector2D DetailMin;
		FVector2D DetailMax;
		FluidSimulation->DetailGPUResource.GetDetailRect(DetailMin, DetailMax, FluidSimulation->bEnableGPUSimulation);

		// Setup a scale to convert fluid texture coordinates in the range [0,1] over the entire low res grid to be [0,1] on the high res grid
		// These will be used to map the detail normal and attenuation textures onto the fluid surface
		DetailCoordScale = FLinearColor(
			FluidSimulation->TotalWidth / (DetailMax.X - DetailMin.X), 
			FluidSimulation->TotalHeight / (DetailMax.Y - DetailMin.Y), 0);

		// Setup a texture coordinate offset
		DetailCoordOffset = FLinearColor(
			(DetailMin.X + .5f * FluidSimulation->TotalWidth) / FluidSimulation->TotalWidth, 
			(DetailMin.Y + .5f * FluidSimulation->TotalHeight) / FluidSimulation->TotalHeight, 0);

		// Get the detail normal texture from the GPU simulation
		FluidNormalTexture.TextureRHI = FluidSimulation->DetailGPUResource.GetNormalTexture();
		FluidNormalTexture.SamplerStateRHI = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
	}

	// FMaterialRenderProxy interface.
	virtual const FMaterial* GetMaterial() const
	{
		return Parent->GetMaterial();
	}

	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const
	{
		static FName DetailCoordOffsetParam = FName(TEXT("DetailCoordOffset"));
		static FName DetailCoordScaleParam = FName(TEXT("DetailCoordScale"));
		if (ParameterName == DetailCoordOffsetParam)
		{
			*OutValue = DetailCoordOffset;
			return TRUE;
		}
		else if (ParameterName == DetailCoordScaleParam)
		{
			*OutValue = DetailCoordScale;
			return TRUE;
		}
		else
		{
			return Parent->GetVectorValue(ParameterName, OutValue, Context);
		}
	}

	virtual UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const
	{
		return Parent->GetScalarValue(ParameterName, OutValue, Context);
	}

	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const
	{
		static FName FluidDetailNormalParam = FName(TEXT("FluidDetailNormal"));
		if (ParameterName == FluidDetailNormalParam)
		{
			*OutValue = &FluidNormalTexture;
			return TRUE;
		}
		else
		{
			return Parent->GetTextureValue(ParameterName,OutValue,Context);
		}
	}
};

/** Implementation of the LCI for fluid surfaces */
class FFluidSurfaceLCI : public FLightCacheInterface
{
public:

	/** Initialization constructor. */
	FFluidSurfaceLCI(const UFluidSurfaceComponent* InComponent) :
	  Component(InComponent)
	  {}

	  // FLightCacheInterface
	  virtual FLightInteraction GetInteraction(const class FLightSceneInfo* LightSceneInfo) const
	  {
		  if(Component->LightMap && Component->LightMap->LightGuids.ContainsItem(LightSceneInfo->LightmapGuid))
		  {
			  return FLightInteraction::LightMap();
		  }

		  return FLightInteraction::Uncached();
	  }

	  virtual FLightMapInteraction GetLightMapInteraction() const
	  {
		  if (Component->LightMap)
		  {
			  return Component->LightMap->GetInteraction();
		  }
		  else
		  {
			  return FLightMapInteraction();
		  }
	  }

private:
	const UFluidSurfaceComponent* const Component;
};


/*=============================================================================
	FFluidVertexBuffer implementation
=============================================================================*/

FFluidVertexBuffer::FFluidVertexBuffer( )
:	Owner(NULL)
,	MaxNumVertices(0)
,	bIsLocked(FALSE)
,	BufferType(BT_Simulation)
{
}

void FFluidVertexBuffer::Setup( FFluidSimulation* InOwner, DWORD InMaxNumVertices, EBufferType InBufferType, INT InNumQuadsX/*=0*/, INT InNumQuadsY/*=0*/ )
{
	Owner = InOwner;
	MaxNumVertices = InMaxNumVertices;
	BufferType = InBufferType;
	NumQuadsX = InNumQuadsX;
	NumQuadsY = InNumQuadsY;
}

FFluidVertex* FFluidVertexBuffer::Lock()
{
	DWORD TotalSize = MaxNumVertices * sizeof(FFluidVertex);
	FFluidVertex* Vertices = (FFluidVertex*)RHILockVertexBuffer( VertexBufferRHI, 0, TotalSize, FALSE );
	bIsLocked = TRUE;
	return Vertices;
}

void FFluidVertexBuffer::Unlock()
{
	RHIUnlockVertexBuffer(VertexBufferRHI);
	bIsLocked = FALSE;
}

UBOOL FFluidVertexBuffer::IsLocked()
{
	return bIsLocked;
}

UBOOL FFluidVertexBuffer::IsBusy()
{
	return RHIIsBusyVertexBuffer( VertexBufferRHI );
}

DWORD FFluidVertexBuffer::GetMaxNumVertices() const
{
	return MaxNumVertices;
}

INT FFluidVertexBuffer::GetNumQuadsX() const
{
	return NumQuadsX;
}

INT FFluidVertexBuffer::GetNumQuadsY() const
{
	return NumQuadsY;
}

void FFluidVertexBuffer::InitDynamicRHI()
{
	if ( BufferType == BT_Border )
	{
		// Create the vertices for the flat border geometry.
		DWORD TotalVBSize = MaxNumVertices * sizeof(FFluidVertex);
		VertexBufferRHI = RHICreateVertexBuffer( TotalVBSize, NULL, RUF_Dynamic );
		FFluidVertex* Vertices = Lock();
		Owner->UpdateBorderGeometry( Vertices );
		Unlock();
	}
	else if ( BufferType == BT_Simulation )
	{
#if XBOX
		VertexBufferRHI = RHICreateVertexBuffer( 1, NULL, RUF_Static );
#else
		// create a dynamic vertex buffer
		DWORD TotalVBSize = MaxNumVertices * sizeof(FFluidVertex);
		VertexBufferRHI = RHICreateVertexBuffer( TotalVBSize, NULL, RUF_Dynamic );

		INT NumCellsX = Owner->GetNumCellsX();
		INT NumCellsY = Owner->GetNumCellsY();
		const FIntPoint& TotalSize = Owner->GetTotalSize();
		const FIntPoint& SimulationPos = Owner->GetSimulationPosition();

		FFluidVertex* Vertices = Lock();
		FFluidVertex Vertex;
		FLOAT CellSize = Owner->GetWidth() / FLOAT(NumCellsX);
		FVector2D UVOrigin( FLOAT(SimulationPos.X)/FLOAT(TotalSize.X), FLOAT(SimulationPos.Y)/FLOAT(TotalSize.Y) );
		FVector2D StepUV( 1.0f/TotalSize.X, 1.0f/TotalSize.Y );
		Vertex.Height		= 0.0f;
		Vertex.HeightDelta	= FVector2D( 0.0f, 0.0f );

		for ( INT Y=0, VertexIndex=0; Y <= NumCellsY; ++Y )
		{
			Vertex.UV		= UVOrigin;
			for ( INT X=0; X <= NumCellsX; ++X, ++VertexIndex )
			{
				Vertices[VertexIndex]	= Vertex;
				Vertex.UV.X				+= StepUV.X;
			}
			UVOrigin.Y	+= StepUV.Y;
		}
		Unlock();
#endif
	}
	else if ( BufferType == BT_Quad )
	{
		check( MaxNumVertices == ((NumQuadsX+1)*(NumQuadsY+1)) );
		DWORD TotalVBSize	= MaxNumVertices * sizeof(FFluidVertex);
		VertexBufferRHI		= RHICreateVertexBuffer( TotalVBSize, NULL, RUF_Static );
		FFluidVertex* Vertices = Lock();
		FFluidVertex Vertex;
		FVector2D UVOrigin( 0.0f, 0.0f );
		FVector2D StepUV( 1.0f/NumQuadsX, 1.0f/NumQuadsY );
		Vertex.Height		= 0.0f;
		Vertex.HeightDelta	= FVector2D( 0.0f, 0.0f );

		for ( INT Y=0, VertexIndex=0; Y <= NumQuadsY; ++Y )
		{
			Vertex.UV = UVOrigin;
			for ( INT X=0; X <= NumQuadsX; ++X, ++VertexIndex )
			{
				Vertices[VertexIndex] = Vertex;
				Vertex.UV.X += StepUV.X;
			}
			UVOrigin.Y += StepUV.Y;
		}
		Unlock();
	}
}

void FFluidVertexBuffer::ReleaseDynamicRHI()
{
	if ( IsValidRef(VertexBufferRHI) )
	{
		Owner->BlockOnSimulation();
		if ( IsLocked() )
		{
			Unlock();
		}
	}
	VertexBufferRHI.SafeRelease();
}


/*=============================================================================
	FFluidVertexFactory implementation
=============================================================================*/

FFluidVertexFactory::FFluidVertexFactory()
{
	bUseGPUTessellation = FALSE;
}

FFluidVertexFactory::~FFluidVertexFactory()
{
}

inline const FFluidSimulation* FFluidVertexFactory::GetSimulation()
{
	return FluidSimulation;
}

#if XBOX
inline FTexture2DRHIRef& FFluidVertexFactory::GetHeightmapTexture()
{
	AddTrackEvent( TRACK_TextureRendered, 1 - FluidSimulation->SimulationIndex );
	return FluidSimulation->HeightMapTextures[1 - FluidSimulation->SimulationIndex];
}
#endif

void FFluidVertexFactory::InitResources( const FFluidVertexBuffer& VertexBuffer, FFluidSimulation* InFluidSimulation )
{
	FluidSimulation = InFluidSimulation;

	// Height stream
	Height = FVertexStreamComponent(&VertexBuffer,STRUCT_OFFSET(FFluidVertex,Height),sizeof(FFluidVertex),VET_Float1);
	// UV stream
	TexCoord = FVertexStreamComponent(&VertexBuffer,STRUCT_OFFSET(FFluidVertex,UV),sizeof(FFluidVertex),VET_Float2);
	// Tangents stream
	HeightDelta = FVertexStreamComponent(&VertexBuffer,STRUCT_OFFSET(FFluidVertex,HeightDelta),sizeof(FFluidVertex),VET_Float2);

	UpdateRHI();
}

void FFluidVertexFactory::InitRHI()
{
	// Dummy call, just to add a stream source.
	AccessStreamComponent( Height, VEU_Position );

	SetDeclaration(GFluidVertexDeclaration.VertexDeclarationRHI);
}

FString FFluidVertexFactory::GetFriendlyName()
{
	return FString( TEXT("Fluids Vertex Factory") );
}

/**
 * Should we cache the material's shadertype on this platform with this vertex factory? 
 */
UBOOL FFluidVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	if ( Material->IsUsedWithFluidSurfaces() || Material->IsUsedWithDecals() || Material->IsSpecialEngineMaterial() )
	{
		if ( !appStrstr(ShaderType->GetShaderFilename(),TEXT("VelocityShader")) )
		{
			return TRUE;
		}
	}
	return FALSE;
}


/**
 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
 */
void FFluidVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	FVertexFactory::ModifyCompilationEnvironment(Platform, OutEnvironment);
	OutEnvironment.Definitions.Set(TEXT("XBOXTESSELLATION"),TEXT("0"));
}

/** Returns whether the vertex shader should generate vertices (TRUE) or if it should use a vertex buffer (FALSE). */
UBOOL FFluidVertexFactory::UseGPUTessellation()
{
	return bUseGPUTessellation;
}

/*=============================================================================
	FFluidTessellationVertexFactory
=============================================================================*/

FFluidTessellationVertexFactory::FFluidTessellationVertexFactory()
{
	bUseGPUTessellation = TRUE;
}

void FFluidTessellationVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	FVertexFactory::ModifyCompilationEnvironment(Platform, OutEnvironment);
	if ( Platform == SP_XBOXD3D )
	{
		OutEnvironment.Definitions.Set(TEXT("XBOXTESSELLATION"),TEXT("1"));
	}
	else
	{
		OutEnvironment.Definitions.Set(TEXT("XBOXTESSELLATION"),TEXT("0"));
	}
}

/*=============================================================================
	FFluidSurfaceSceneProxy
=============================================================================*/

/**
 * A fluid surface component scene proxy.
 */
class FFluidSurfaceSceneProxy : public FPrimitiveSceneProxy
{
public:
	/** Initialization constructor. */
	FFluidSurfaceSceneProxy( const UFluidSurfaceComponent* Component )
	:	FPrimitiveSceneProxy(Component)
	,	FluidSurfaceComponent(Component)
	,	MaterialViewRelevance(Component->GetMaterialViewRelevance())
	,	LCI(Component)
	,	bSelected(Component->IsOwnerSelected())
	{
		UMaterialInterface* MaterialInterface = Component->GetMaterial();
		const UBOOL bCorrectStaticLightingUsage = Component->LightMap.GetReference() == NULL || MaterialInterface->CheckMaterialUsage( MATUSAGE_StaticLighting );
		if ( MaterialInterface->CheckMaterialUsage( MATUSAGE_FluidSurface ) && bCorrectStaticLightingUsage )
		{
			MaterialProxy = MaterialInterface->GetRenderProxy(bSelected);
		}
		else
		{
			MaterialProxy = GEngine->DefaultMaterial->GetRenderProxy(bSelected);
		}
	}

	virtual ~FFluidSurfaceSceneProxy()
	{
	}

	/** 
	 * Draw the scene proxy as a dynamic element
	 *
	 * @param	PDI - draw interface to render to
	 * @param	View - current view
	 * @param	DPGIndex - current depth priority 
	 * @param	Flags - optional set of flags from EDrawDynamicElementFlags
	 */
	virtual void DrawDynamicElements( FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, DWORD Flags )
	{
		FFluidSimulation* FluidSimulation = FluidSurfaceComponent ? FluidSurfaceComponent->FluidSimulation : NULL;
		if ( FluidSimulation && GetDepthPriorityGroup(View) == DPGIndex )
		{
			if ( GIsEditor 
				&& !View->Family->bRealtimeUpdate 
				&& FluidSimulation->bEnableGPUSimulation
				&& FluidSurfaceComponent->bShowFluidDetail
				&& !(View->Family->ShowFlags & SHOW_HitProxies) 
				&& !(View->Family->ShowFlags & SHOW_Wireframe) )
			{
				// Initialize render targets when starting up the editor in a non-realtime viewport
				// Note: this will potentially cause the rest of the first frame to render incorrectly due to overridden render state
				FluidSimulation->DetailGPUResource.InitializeRenderTargetContents();
			}

			const FMatrix& WorldToLocal	= FluidSimulation->GetWorldToLocal();
			const FMatrix& LocalToWorld	= FluidSurfaceComponent->LocalToWorld;
			FVector WorldViewDirection	= FVector(View->ViewMatrix.M[0][2], View->ViewMatrix.M[1][2], View->ViewMatrix.M[2][2]);
			FVector LocalViewDirection	= WorldToLocal.TransformNormal( WorldViewDirection );
			FVector LocalViewPosition	= WorldToLocal.TransformFVector( View->ViewOrigin );
			INT OctantID				= FFluidSimulation::ClassifyOctant( LocalViewDirection );


			FMeshElement Mesh;
			Mesh.LCI					= &LCI;
			Mesh.LocalToWorld			= LocalToWorld;
			Mesh.WorldToLocal			= WorldToLocal;
			Mesh.CastShadow				= FALSE;
			Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;
			Mesh.FirstIndex				= 0;
			Mesh.MinVertexIndex			= 0;
			Mesh.MaxVertexIndex			= FluidSimulation->NumVertices - 1;
			Mesh.bWireframe				= FALSE;

			FluidSimulation->UpdateShaderParameters( OctantID );

			// Setup the flat border geometry.
			UBOOL bCameraWithinSimulationGrid	= FluidSimulation->IsWithinSimulationGrid( LocalViewPosition, 0.0f );
			FMeshElement BorderMesh;
			BorderMesh.LCI					= &LCI;
			BorderMesh.LocalToWorld			= LocalToWorld;
			BorderMesh.WorldToLocal			= WorldToLocal;
			BorderMesh.CastShadow			= FALSE;
			BorderMesh.DepthPriorityGroup	= (ESceneDepthPriorityGroup)DPGIndex;
			BorderMesh.FirstIndex			= 0;
			BorderMesh.MinVertexIndex		= 0;
			BorderMesh.MaxVertexIndex		= FluidSimulation->FlatVertexBuffers[1 - FluidSimulation->SimulationIndex].GetMaxNumVertices() - 1;
			BorderMesh.ReverseCulling		= (LocalToWorldDeterminant < 0.0f ? TRUE : FALSE);
			BorderMesh.Type					= PT_TriangleList;
			BorderMesh.NumPrimitives		= FluidSimulation->FlatIndexBuffer.GetNumIndices() / 3;
			BorderMesh.bWireframe			= FALSE;

			// Override all geometry with a flat quad if we can.
			if ( FluidSimulation->bShowSimulation == FALSE )
			{
				INT NumLowResQuadsX		= FluidSimulation->FlatQuadVertexBuffer.GetNumQuadsX();
				INT NumLowResQuadsY		= FluidSimulation->FlatQuadVertexBuffer.GetNumQuadsY();
				Mesh.Type				= PT_TriangleList;
				Mesh.VertexFactory		= &FluidSimulation->FlatQuadVertexFactory;
				Mesh.IndexBuffer		= &FluidSimulation->FlatQuadIndexBuffer;
				Mesh.MaxVertexIndex		= (NumLowResQuadsX + 1) * (NumLowResQuadsY + 1) - 1;
				Mesh.NumPrimitives		= NumLowResQuadsX * NumLowResQuadsY * 2;
				Mesh.ReverseCulling		= (LocalToWorldDeterminant < 0.0f ? TRUE : FALSE);
			}
			else
			{
#if XBOX
				RHISetTessellationMode( TESS_Continuous, FluidSimulation->TessellationLevel - 1.0f, FluidSimulation->TessellationLevel - 1.0f );
				Mesh.Type				= PT_TessellatedQuadPatch;
				Mesh.ReverseCulling		= XOR((LocalToWorldDeterminant < 0.0f ? TRUE : FALSE), FluidSimulation->bReverseCullingXbox);
				Mesh.IndexBuffer		= NULL;
				Mesh.NumPrimitives		= FluidSimulation->NumTessQuadsX * FluidSimulation->NumTessQuadsY;
#else
				if (FluidSimulation->bUseYFirstIndexBuffer[1 - FluidSimulation->SimulationIndex])
				{
					Mesh.IndexBuffer	= &FluidSimulation->YFirstIndexBuffer;
				}
				else
				{
					Mesh.IndexBuffer	= &FluidSimulation->XFirstIndexBuffer;
				}
				Mesh.ReverseCulling		= XOR((LocalToWorldDeterminant < 0.0f ? TRUE : FALSE), FluidSimulation->bReverseCulling[1 - FluidSimulation->SimulationIndex]);
				Mesh.Type				= PT_TriangleList;
				Mesh.NumPrimitives		= FluidSimulation->NumIndices / 3;
#endif
				Mesh.VertexFactory			= &FluidSimulation->VertexFactories[1 - FluidSimulation->SimulationIndex];
				BorderMesh.VertexFactory	= &FluidSimulation->FlatVertexFactories[1 - FluidSimulation->SimulationIndex];
				BorderMesh.IndexBuffer		= &FluidSimulation->FlatIndexBuffer;
			}

			FLinearColor WireframeColor(0.7f, 0.005f, 0.00f);
			if ( FluidSimulation->bEnableGPUSimulation && FluidSurfaceComponent->bShowFluidDetail )
			{
				FFluidMaterialRenderProxy Proxy(MaterialProxy, FluidSimulation);
				Mesh.MaterialRenderProxy			= &Proxy;
				BorderMesh.MaterialRenderProxy	= &Proxy;
				if ( FluidSimulation->bShowSimulation == FALSE )	// Override all geometry with a flat quad if we can.
				{
					DrawRichMesh(PDI, Mesh, WireframeColor, FLinearColor::White, FLinearColor::White, PrimitiveSceneInfo, bSelected);
				}
				else
				{
					DrawRichMesh(PDI, BorderMesh, WireframeColor, FLinearColor::White, FLinearColor::White, PrimitiveSceneInfo, bSelected);
					DrawRichMesh(PDI, Mesh, WireframeColor, FLinearColor::White, FLinearColor::White, PrimitiveSceneInfo, bSelected);
				}
			}
			else
			{
				Mesh.MaterialRenderProxy = MaterialProxy;
				BorderMesh.MaterialRenderProxy = MaterialProxy;
				if ( FluidSimulation->bShowSimulation == FALSE )	// Override all geometry with a flat quad if we can.
				{
					DrawRichMesh(PDI, Mesh, WireframeColor, FLinearColor::White, FLinearColor::White, PrimitiveSceneInfo, bSelected);
				}
				else
				{
					DrawRichMesh(PDI, BorderMesh, WireframeColor, FLinearColor::White, FLinearColor::White, PrimitiveSceneInfo, bSelected);
					DrawRichMesh(PDI, Mesh, WireframeColor, FLinearColor::White, FLinearColor::White, PrimitiveSceneInfo, bSelected);
				}
			}

#if XBOX && USE_XeD3D_RHI
			{
//				FluidSimulation->VertexFactories[1 - FluidSimulation->SimulationIndex]->
//				SetTextureParameter( NULL, HeightmapParameter, TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(), NULL );
				extern IDirect3DDevice9* GDirect3DDevice;
				GDirect3DDevice->SetTexture(D3DVERTEXTEXTURESAMPLER0,NULL);
				GDirect3DDevice->SetStreamSource(0,NULL,0,0);
				RHIKickCommandBuffer();
			}
#endif

			if ( FluidSurfaceComponent->bShowSimulationNormals && FluidSimulation->DebugPositions.Num() > 0 )
			{
				for ( INT Y = 1; Y < FluidSimulation->NumCellsY; Y++ )
				{
					for ( INT X = 1; X < FluidSimulation->NumCellsX; X++ )
					{
						INT CurrentIndex = Y * (FluidSimulation->NumCellsX + 1) + X;
						PDI->DrawLine(FluidSimulation->DebugPositions(CurrentIndex), FluidSimulation->DebugPositions(CurrentIndex) + FluidSurfaceComponent->NormalLength * FluidSimulation->DebugNormals(CurrentIndex), FLinearColor::White, SDPG_World);
					}
				}
			}

			if ( FluidSurfaceComponent->bShowDetailPosition && FluidSurfaceComponent->EnableDetail )
			{
				FVector2D DetailMin;
				FVector2D DetailMax;
				FluidSimulation->GetDetailRect(DetailMin, DetailMax);
				FVector Corner0(DetailMin.X, DetailMin.Y, 1.0f);
				FVector Corner1(DetailMin.X, DetailMax.Y, 1.0f);
				FVector Corner2(DetailMax.X, DetailMax.Y, 1.0f);
				FVector Corner3(DetailMax.X, DetailMin.Y, 1.0f);
				Corner0 = FluidSurfaceComponent->LocalToWorld.TransformFVector(Corner0);
				Corner1 = FluidSurfaceComponent->LocalToWorld.TransformFVector(Corner1);
				Corner2 = FluidSurfaceComponent->LocalToWorld.TransformFVector(Corner2);
				Corner3 = FluidSurfaceComponent->LocalToWorld.TransformFVector(Corner3);
				PDI->DrawLine(Corner0, Corner1, FLinearColor::White, SDPG_World);
				PDI->DrawLine(Corner1, Corner2, FLinearColor::White, SDPG_World);
				PDI->DrawLine(Corner2, Corner3, FLinearColor::White, SDPG_World);
				PDI->DrawLine(Corner3, Corner0, FLinearColor::White, SDPG_World);
			}

			if ( FluidSurfaceComponent->bShowSimulationPosition && FluidSurfaceComponent->EnableSimulation )
			{
				FVector2D SimMin;
				FVector2D SimMax;
				FluidSimulation->GetSimulationRect(SimMin, SimMax);
				FVector Corner0(SimMin.X, SimMin.Y, 1.0f);
				FVector Corner1(SimMin.X, SimMax.Y, 1.0f);
				FVector Corner2(SimMax.X, SimMax.Y, 1.0f);
				FVector Corner3(SimMax.X, SimMin.Y, 1.0f);
				Corner0 = FluidSurfaceComponent->LocalToWorld.TransformFVector(Corner0);
				Corner1 = FluidSurfaceComponent->LocalToWorld.TransformFVector(Corner1);
				Corner2 = FluidSurfaceComponent->LocalToWorld.TransformFVector(Corner2);
				Corner3 = FluidSurfaceComponent->LocalToWorld.TransformFVector(Corner3);
				FLinearColor Color(1,1,0);
				PDI->DrawLine(Corner0, Corner1, Color, SDPG_World);
				PDI->DrawLine(Corner1, Corner2, Color, SDPG_World);
				PDI->DrawLine(Corner2, Corner3, Color, SDPG_World);
				PDI->DrawLine(Corner3, Corner0, Color, SDPG_World);
			}

#if !CONSOLE
			if ( FluidSurfaceComponent->bShowDetailNormals 
				&& FluidSimulation->bEnableGPUSimulation
				&& !(View->Family->ShowFlags & SHOW_HitProxies) 
				&& !(View->Family->ShowFlags & SHOW_Wireframe) )
			{
				// Render a visualization of the detail simulation
				FluidSimulation->DetailGPUResource.Visualize(View);
			}
#endif
		}

		if (DPGIndex == SDPG_Foreground && (View->Family->ShowFlags & SHOW_Bounds) && (GIsGame || bSelected) )
		{
			// Draw the static mesh's bounding box and sphere.
			DrawWireBox(PDI,PrimitiveSceneInfo->Bounds.GetBox(), FColor(72,72,255),SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,1,0),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(0,1,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
		}

		if (FluidSimulation && !(View->Family->ShowFlags & SHOW_HitProxies) && !(View->Family->ShowFlags & SHOW_Wireframe))
		{
			// Store the view direction that was last used for rendering
			//@todo - calculate this in PreRenderViews so that it only happens once per frame, and multiple views don't conflict
			FVector WorldViewDirection = FVector(View->ViewMatrix.M[0][2], View->ViewMatrix.M[1][2], View->ViewMatrix.M[2][2]);
			FMatrix RotationOnlyWorldToLocal = FluidSimulation->GetWorldToLocal();
			RotationOnlyWorldToLocal.RemoveScaling();
			FluidSimulation->LastViewDirection[1 - FluidSimulation->SimulationIndex] = RotationOnlyWorldToLocal.TransformNormal(WorldViewDirection);
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance( const FSceneView* View )
	{
		FPrimitiveViewRelevance Result;
		Result.SetDPG(GetDepthPriorityGroup(View),TRUE);
		Result.bDynamicRelevance = TRUE;
		MaterialViewRelevance.SetPrimitiveViewRelevance(Result);

#if !FINAL_RELEASE
		// Only add to foreground DPG for debug rendering
		if (View->Family->ShowFlags & SHOW_Bounds)
		{
			Result.SetDPG(SDPG_Foreground,TRUE);
		}
#endif

		return Result;
	}

	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped)
	{
		const ELightInteractionType InteractionType = LCI.GetInteraction(LightSceneInfo).GetType();

		bDynamic = (InteractionType == LIT_Uncached);
		bRelevant = (InteractionType != LIT_CachedIrrelevant);
		bLightMapped = (InteractionType == LIT_CachedLightMap || InteractionType == LIT_CachedIrrelevant);
	}

	virtual EMemoryStats	GetMemoryStatType( ) const		{ return STAT_StaticMeshVertexMemory; }
	virtual DWORD			GetMemoryFootprint( ) const		{ return 0; }

protected:
	const UFluidSurfaceComponent*	FluidSurfaceComponent;
	FMaterialViewRelevance			MaterialViewRelevance;
	FMaterialRenderProxy*	MaterialProxy;
	FFluidSurfaceLCI		LCI;
	UBOOL					bSelected;
	BITFIELD				bDrawWireFrame:1;
};


/*=============================================================================
	UFluidSurfaceComponent scene proxy creation
=============================================================================*/

FPrimitiveSceneProxy* UFluidSurfaceComponent::CreateSceneProxy()
{
	return new FFluidSurfaceSceneProxy( this );
}
