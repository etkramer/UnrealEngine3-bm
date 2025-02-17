/*=============================================================================
	BatchedElements.cpp: Batched element rendering.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "BatchedElements.h"

/**
 * Simple pixel shader that just reads from a texture
 * This is used for resolves and needs to be as efficient as possible
 */
class FSimpleElementPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FSimpleElementPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FSimpleElementPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		TextureParameter.Bind(Initializer.ParameterMap,TEXT("Texture"));
		TextureComponentReplicateParameter.Bind(Initializer.ParameterMap,TEXT("TextureComponentReplicate"),TRUE);
	}
	FSimpleElementPixelShader() {}

	void SetParameters(const FTexture* Texture)
	{
		SetTextureParameter(GetPixelShader(),TextureParameter,Texture);
		SetPixelShaderValue(GetPixelShader(),TextureComponentReplicateParameter,Texture->bGreyScaleFormat ? FLinearColor(1,0,0,0) : FLinearColor(0,0,0,0));
		RHISetRenderTargetBias(appPow(2.0f,GCurrentColorExpBias));
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << TextureParameter;
		Ar << TextureComponentReplicateParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter TextureParameter;
	FShaderParameter TextureComponentReplicateParameter;
};

/**
 * A pixel shader for rendering a texture on a simple element.
 */
class FSimpleElementGammaPixelShader : public FSimpleElementPixelShader
{
	DECLARE_SHADER_TYPE(FSimpleElementGammaPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FSimpleElementGammaPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FSimpleElementPixelShader(Initializer)
	{
		GammaParameter.Bind(Initializer.ParameterMap,TEXT("Gamma"));
	}
	FSimpleElementGammaPixelShader() {}

	void SetParameters(const FTexture* Texture,FLOAT Gamma,EBlendMode BlendMode)
	{
		FSimpleElementPixelShader::SetParameters(Texture);
		SetPixelShaderValue(GetPixelShader(),GammaParameter,Gamma);
		RHISetRenderTargetBias( (BlendMode == BLEND_Modulate) ? 1.0f : appPow(2.0f,GCurrentColorExpBias));
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FSimpleElementPixelShader::Serialize(Ar);
		Ar << GammaParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter GammaParameter;
};

/**
 * A pixel shader for rendering a masked texture on a simple element.
 */
class FSimpleElementMaskedGammaPixelShader : public FSimpleElementGammaPixelShader
{
	DECLARE_SHADER_TYPE(FSimpleElementMaskedGammaPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FSimpleElementMaskedGammaPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FSimpleElementGammaPixelShader(Initializer)
	{
		ClipRefParameter.Bind(Initializer.ParameterMap,TEXT("ClipRef"));
	}
	FSimpleElementMaskedGammaPixelShader() {}

	void SetParameters(const FTexture* Texture,FLOAT Gamma,FLOAT ClipRef,EBlendMode BlendMode)
	{
		FSimpleElementGammaPixelShader::SetParameters(Texture,Gamma,BlendMode);
		SetPixelShaderValue(GetPixelShader(),ClipRefParameter,ClipRef);
		RHISetRenderTargetBias((BlendMode == BLEND_Modulate) ? 1.0f : appPow(2.0f,GCurrentColorExpBias));
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FSimpleElementGammaPixelShader::Serialize(Ar);
		Ar << ClipRefParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter ClipRefParameter;
};

/**
 * A pixel shader for rendering a texture on a simple element.
 */
class FSimpleElementHitProxyPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FSimpleElementHitProxyPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return Platform == SP_PCD3D_SM4 || Platform == SP_PCD3D_SM3 || Platform == SP_PCD3D_SM2; }

	FSimpleElementHitProxyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		TextureParameter.Bind(Initializer.ParameterMap,TEXT("Texture"));
	}
	FSimpleElementHitProxyPixelShader() {}

	void SetParameters(const FTexture* Texture)
	{
		SetTextureParameter(GetPixelShader(),TextureParameter,Texture);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << TextureParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter TextureParameter;
};

/**
 * A vertex shader for rendering a texture on a simple element.
 */
class FSimpleElementVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FSimpleElementVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FSimpleElementVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		TransformParameter.Bind(Initializer.ParameterMap,TEXT("Transform"));
	}
	FSimpleElementVertexShader() {}

	void SetParameters(const FMatrix& Transform)
	{
		SetVertexShaderValue(GetVertexShader(),TransformParameter,Transform);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << TransformParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter TransformParameter;
};

// Shader implementations.
IMPLEMENT_SHADER_TYPE(,FSimpleElementPixelShader,TEXT("SimpleElementPixelShader"),TEXT("Main"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(,FSimpleElementGammaPixelShader,TEXT("SimpleElementPixelShader"),TEXT("GammaMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(,FSimpleElementMaskedGammaPixelShader,TEXT("SimpleElementPixelShader"),TEXT("GammaMaskedMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(,FSimpleElementHitProxyPixelShader,TEXT("SimpleElementHitProxyPixelShader"),TEXT("Main"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(,FSimpleElementVertexShader,TEXT("SimpleElementVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/** The simple element vertex declaration. */
TGlobalResource<FSimpleElementVertexDeclaration> GSimpleElementVertexDeclaration;

void FBatchedElements::AddLine(const FVector& Start,const FVector& End,const FLinearColor& Color,FHitProxyId HitProxyId)
{
	// Ensure the line isn't masked out.  Some legacy code relies on Color.A being ignored.
	FLinearColor OpaqueColor(Color);
	OpaqueColor.A = 1;

	new(LineVertices) FSimpleElementVertex(Start,FVector2D(0,0),OpaqueColor,HitProxyId);
	new(LineVertices) FSimpleElementVertex(End,FVector2D(0,0),OpaqueColor,HitProxyId);
}

void FBatchedElements::AddPoint(const FVector& Position,FLOAT Size,const FLinearColor& Color,FHitProxyId HitProxyId)
{
	// Ensure the point isn't masked out.  Some legacy code relies on Color.A being ignored.
	FLinearColor OpaqueColor(Color);
	OpaqueColor.A = 1;

	FBatchedPoint* Point = new(Points) FBatchedPoint;
	Point->Position = Position;
	Point->Size = Size;
	Point->Color = OpaqueColor;
	Point->HitProxyId = HitProxyId;
}

INT FBatchedElements::AddVertex(const FVector4& InPosition,const FVector2D& InTextureCoordinate,const FLinearColor& InColor,FHitProxyId HitProxyId)
{
	INT VertexIndex = MeshVertices.Num();
	new(MeshVertices) FSimpleElementVertex(InPosition,InTextureCoordinate,InColor,HitProxyId);
	return VertexIndex;
}

void FBatchedElements::AddQuadVertex(const FVector4& InPosition,const FVector2D& InTextureCoordinate,const FLinearColor& InColor,FHitProxyId HitProxyId, const FTexture* Texture,EBlendMode BlendMode)
{
	check(GSupportsQuads);

	// @GEMINI_TODO: Speed this up (BeginQuad call that returns handle to a MeshElement?)

	// Find an existing mesh element for the given texture.
	FBatchedQuadMeshElement* MeshElement = NULL;
	for (INT MeshIndex = 0;MeshIndex < QuadMeshElements.Num();MeshIndex++)
	{
		if(QuadMeshElements(MeshIndex).Texture == Texture && QuadMeshElements(MeshIndex).BlendMode == BlendMode)
		{
			MeshElement = &QuadMeshElements(MeshIndex);
			break;
		}
	}

	if (!MeshElement)
	{
		// Create a new mesh element for the texture if this is the first triangle encountered using it.
		MeshElement = new(QuadMeshElements) FBatchedQuadMeshElement;
		MeshElement->Texture = Texture;
		MeshElement->BlendMode = BlendMode;
	}

	new(MeshElement->Vertices) FSimpleElementVertex(InPosition,InTextureCoordinate,InColor,HitProxyId);
}

void FBatchedElements::AddTriangle(INT V0,INT V1,INT V2,const FTexture* Texture,EBlendMode BlendMode)
{
#if XBOX
	// Find an existing mesh element for the given texture and blend mode
	FBatchedMeshElement* MeshElement = NULL;
	for(INT MeshIndex = 0;MeshIndex < MeshElements.Num();MeshIndex++)
	{
		const FBatchedMeshElement& CurMeshElement = MeshElements(MeshIndex);
		if( CurMeshElement.Texture == Texture && 
			CurMeshElement.BlendMode == BlendMode &&
			// make sure we are not overflowing on indices
			(CurMeshElement.Indices.Num()+3) < MaxMeshIndicesAllowed )
		{
			// make sure we are not overflowing on vertices
			INT DeltaV0 = (V0 - (INT)CurMeshElement.MinVertex);
			INT DeltaV1 = (V1 - (INT)CurMeshElement.MinVertex);
			INT DeltaV2 = (V2 - (INT)CurMeshElement.MinVertex);
			if( DeltaV0 >= 0 && DeltaV0 < MaxMeshVerticesAllowed &&
				DeltaV1 >= 0 && DeltaV1 < MaxMeshVerticesAllowed &&
				DeltaV2 >= 0 && DeltaV2 < MaxMeshVerticesAllowed )
			{
				MeshElement = &MeshElements(MeshIndex);
				break;
			}			
		}
	}
	if(!MeshElement)
	{
		// make sure that vertex indices are close enough to fit within MaxVerticesAllowed
		if( Abs(V0 - V1) >= MaxMeshVerticesAllowed ||
			Abs(V0 - V2) >= MaxMeshVerticesAllowed )
		{
			warnf(TEXT("Omitting FBatchedElements::AddTriangle due to sparce vertices V0=%i,V1=%i,V2=%i"),V0,V1,V2);
		}
		else
		{
			// Create a new mesh element for the texture if this is the first triangle encountered using it.
			MeshElement = new(MeshElements) FBatchedMeshElement;
			MeshElement->Texture = Texture;
			MeshElement->BlendMode = BlendMode;
			MeshElement->MaxVertex = V0;
			// keep track of the min vertex index used
			MeshElement->MinVertex = Min(Min(V0,V1),V2);
		}
	}

	if( MeshElement )
	{
		// Add the triangle's indices to the mesh element's index array.
		MeshElement->Indices.AddItem(V0 - MeshElement->MinVertex);
		MeshElement->Indices.AddItem(V1 - MeshElement->MinVertex);
		MeshElement->Indices.AddItem(V2 - MeshElement->MinVertex);

		// keep track of max vertex used in this mesh batch
		MeshElement->MaxVertex = Max(Max(Max(V0,(INT)MeshElement->MaxVertex),V1),V2);
	}
#else
	// Find an existing mesh element for the given texture and blend mode
	FBatchedMeshElement* MeshElement = NULL;
	for(INT MeshIndex = 0;MeshIndex < MeshElements.Num();MeshIndex++)
	{
		if( MeshElements(MeshIndex).Texture == Texture && 
			MeshElements(MeshIndex).BlendMode == BlendMode )
		{
			MeshElement = &MeshElements(MeshIndex);
			break;
		}
	}
	if( !MeshElement )
	{
		// Create a new mesh element for the texture if this is the first triangle encountered using it.
		MeshElement = new(MeshElements) FBatchedMeshElement;
		MeshElement->Texture = Texture;
		MeshElement->BlendMode = BlendMode;
		MeshElement->MaxVertex = V0;
		// keep track of the min vertex index used
		MeshElement->MinVertex = Min(Min(V0,V1),V2);
	}

	// Add the triangle's indices to the mesh element's index array.
	MeshElement->Indices.AddItem(V0 - MeshElement->MinVertex);
	MeshElement->Indices.AddItem(V1 - MeshElement->MinVertex);
	MeshElement->Indices.AddItem(V2 - MeshElement->MinVertex);	

	// keep track of max vertex used in this mesh batch
	MeshElement->MaxVertex = Max(Max(Max(V0,(INT)MeshElement->MaxVertex),V1),V2);
#endif
}

/** 
* Reserves space in mesh vertex array
* 
* @param NumMeshVerts - number of verts to reserve space for
* @param Texture - used to find the mesh element entry
* @param BlendMode - used to find the mesh element entry
*/
void FBatchedElements::AddReserveVertices(INT NumMeshVerts)
{
	MeshVertices.Reserve( MeshVertices.Num() + NumMeshVerts );
}

/** 
* Reserves space in triangle arrays 
* 
* @param NumMeshTriangles - number of triangles to reserve space for
* @param Texture - used to find the mesh element entry
* @param BlendMode - used to find the mesh element entry
*/
void FBatchedElements::AddReserveTriangles(INT NumMeshTriangles,const FTexture* Texture,EBlendMode BlendMode)
{
	for(INT MeshIndex = 0;MeshIndex < MeshElements.Num();MeshIndex++)
	{
		FBatchedMeshElement& CurMeshElement = MeshElements(MeshIndex);
		if( CurMeshElement.Texture == Texture && 
			CurMeshElement.BlendMode == BlendMode &&
			(CurMeshElement.Indices.Num()+3) < MaxMeshIndicesAllowed )
		{
			CurMeshElement.Indices.Reserve( CurMeshElement.Indices.Num() + NumMeshTriangles );
			break;
		}
	}	
}

void FBatchedElements::AddSprite(
	const FVector& Position,
	FLOAT SizeX,
	FLOAT SizeY,
	const FTexture* Texture,
	const FLinearColor& Color,
	FHitProxyId HitProxyId
	)
{
	FBatchedSprite* Sprite = new(Sprites) FBatchedSprite;
	Sprite->Position = Position;
	Sprite->SizeX = SizeX;
	Sprite->SizeY = SizeY;
	Sprite->Texture = Texture;
	Sprite->Color = Color.Quantize();
	Sprite->HitProxyId = HitProxyId;
}

/** Translates a EBlendMode into a RHI state change for rendering a mesh with the blend mode normally. */
static void SetBlendState(EBlendMode BlendMode)
{
	switch(BlendMode)
	{
	case BLEND_Opaque:
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		break;
	case BLEND_Masked:
		RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_Zero,BO_Add,BF_One,BF_Zero,CF_GreaterEqual,128>::GetRHI());
		break;
	case BLEND_Translucent:
		RHISetBlendState(TStaticBlendState<BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_Zero,BF_One,CF_Greater,0>::GetRHI());
		break;
	case BLEND_Additive:
		RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_One,BO_Add,BF_Zero,BF_One>::GetRHI());
		break;
	case BLEND_Modulate:
		RHISetBlendState(TStaticBlendState<BO_Add,BF_DestColor,BF_Zero,BO_Add,BF_Zero,BF_One>::GetRHI());
		break;
	}
}

/** Translates a EBlendMode into a RHI state change for rendering a mesh with the blend mode for hit testing. */
static void SetHitTestingBlendState(EBlendMode BlendMode)
{
	switch(BlendMode)
	{
	case BLEND_Opaque:
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		break;
	case BLEND_Masked:
		RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_Zero,BO_Add,BF_One,BF_Zero,CF_GreaterEqual,128>::GetRHI());
		break;
	case BLEND_Translucent:
		RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_Zero,BO_Add,BF_One,BF_Zero,CF_GreaterEqual,1>::GetRHI());
		break;
	case BLEND_Additive:
		RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_Zero,BO_Add,BF_One,BF_Zero,CF_GreaterEqual,1>::GetRHI());
		break;
	case BLEND_Modulate:
		RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_Zero,BO_Add,BF_One,BF_Zero,CF_GreaterEqual,1>::GetRHI());
		break;
	}
}

FGlobalBoundShaderState FBatchedElements::SimpleBoundShaderState;
FGlobalBoundShaderState FBatchedElements::RegularBoundShaderState;
FGlobalBoundShaderState FBatchedElements::MaskedBoundShaderState;
FGlobalBoundShaderState FBatchedElements::HitTestingBoundShaderState;

/*
 * Sets the appropriate vertex and pixel shader.
 */
void FBatchedElements::PrepareShaders(
	EBlendMode BlendMode,
	const FMatrix& Transform,
	const FTexture* Texture,
	UBOOL bHitTesting,
	FLOAT Gamma
	) const
{
	TShaderMapRef<FSimpleElementVertexShader> VertexShader(GetGlobalShaderMap());

	// Set the simple element vertex shader parameters
	VertexShader->SetParameters(Transform);
		
	if (bHitTesting)
	{
		TShaderMapRef<FSimpleElementHitProxyPixelShader> HitTestingPixelShader(GetGlobalShaderMap());
		HitTestingPixelShader->SetParameters(Texture);
		SetHitTestingBlendState( BlendMode);
		SetGlobalBoundShaderState(HitTestingBoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI, 
			*VertexShader, *HitTestingPixelShader, sizeof(FSimpleElementVertex));
	}
	else
	{
		if (BlendMode == BLEND_Masked)
		{
			// use clip() in the shader instead of alpha testing as cards that don't support floating point blending
			// also don't support alpha testing to floating point render targets
			RHISetBlendState(TStaticBlendState<>::GetRHI());
			TShaderMapRef<FSimpleElementMaskedGammaPixelShader> MaskedPixelShader(GetGlobalShaderMap());
				MaskedPixelShader->SetParameters(Texture,Gamma,128.0f / 255.0f,BlendMode);
				SetGlobalBoundShaderState( MaskedBoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI, 
					*VertexShader, *MaskedPixelShader, sizeof(FSimpleElementVertex));
		}
		else
		{
			SetBlendState( BlendMode);

			if (fabs(Gamma - 1.0f) < KINDA_SMALL_NUMBER)
			{
				TShaderMapRef<FSimpleElementPixelShader> RegularPixelShader(GetGlobalShaderMap());
				RegularPixelShader->SetParameters(Texture);
				SetGlobalBoundShaderState( SimpleBoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI, 
					*VertexShader, *RegularPixelShader, sizeof(FSimpleElementVertex));
			}
			else
			{
				TShaderMapRef<FSimpleElementGammaPixelShader> RegularPixelShader(GetGlobalShaderMap());
				RegularPixelShader->SetParameters(Texture,Gamma,BlendMode);
				SetGlobalBoundShaderState( RegularBoundShaderState, GSimpleElementVertexDeclaration.VertexDeclarationRHI, 
					*VertexShader, *RegularPixelShader, sizeof(FSimpleElementVertex));
			}
		}
	}
}

UBOOL FBatchedElements::Draw(const FMatrix& Transform,UINT ViewportSizeX,UINT ViewportSizeY,UBOOL bHitTesting,FLOAT Gamma) const
{
	if( HasPrimsToDraw() )
	{
		SCOPED_DRAW_EVENT(EventBatched)(DEC_SCENE_ITEMS,TEXT("BatchedElements"));

		FMatrix InvTransform = Transform.InverseSafe();
		FVector CameraX = InvTransform.TransformNormal(FVector(1,0,0)).SafeNormal();
		FVector CameraY = InvTransform.TransformNormal(FVector(0,1,0)).SafeNormal();

		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		RHISetBlendState(TStaticBlendState<>::GetRHI());

		if( LineVertices.Num() > 0 || Points.Num() > 0 )
		{
			// Set the appropriate pixel shader parameters & shader state for the non-textured elements.
			PrepareShaders( BLEND_Opaque, Transform, GWhiteTexture, bHitTesting, Gamma);

			// Draw the line elements.
			if( LineVertices.Num() > 0 )
			{
				SCOPED_DRAW_EVENT(EventLines)(DEC_SCENE_ITEMS,TEXT("Lines"));

				INT MaxVerticesAllowed = ((GDrawUPVertexCheckCount / sizeof(FSimpleElementVertex)) / 2) * 2;
				/*
				hack to avoid a crash when trying to render large numbers of line segments.
				*/
				MaxVerticesAllowed = Min(MaxVerticesAllowed, 64 * 1024);

				INT MinVertex=0;
				INT TotalVerts = (LineVertices.Num() / 2) * 2;
				while( MinVertex < TotalVerts )
				{
					INT NumLinePrims = Min( MaxVerticesAllowed, TotalVerts - MinVertex ) / 2;
					RHIDrawPrimitiveUP(PT_LineList,NumLinePrims,&LineVertices(MinVertex),sizeof(FSimpleElementVertex));
					MinVertex += NumLinePrims * 2;
				}
			}

			// Draw the point elements.
			if( Points.Num() > 0 )
			{
				SCOPED_DRAW_EVENT(EventPoints)(DEC_SCENE_ITEMS,TEXT("Points"));
				
				for(INT PointIndex = 0;PointIndex < Points.Num();PointIndex++)
				{
					// @GEMINI_TODO: Support quad primitives here

					// preallocate some memory to directly fill out
					void* VerticesPtr;
					RHIBeginDrawPrimitiveUP(PT_TriangleStrip, 2, 4, sizeof(FSimpleElementVertex), VerticesPtr);
					FSimpleElementVertex* PointVertices = (FSimpleElementVertex*)VerticesPtr;

					const FBatchedPoint& Point = Points(PointIndex);
					FVector4 TransformedPosition = Transform.TransformFVector4(Point.Position);

					// Generate vertices for the point such that the post-transform point size is constant.
					const FVector	WorldPointX = InvTransform.TransformNormal(FVector(1,0,0)) * Point.Size / ViewportSizeX * TransformedPosition.W,
									WorldPointY = InvTransform.TransformNormal(FVector(0,1,0)) * -Point.Size / ViewportSizeY * TransformedPosition.W;
					PointVertices[2] = FSimpleElementVertex(FVector4(Point.Position - WorldPointX - WorldPointY,1),FVector2D(0,0),Point.Color,Point.HitProxyId);
					PointVertices[3] = FSimpleElementVertex(FVector4(Point.Position - WorldPointX + WorldPointY,1),FVector2D(0,1),Point.Color,Point.HitProxyId);
					PointVertices[1] = FSimpleElementVertex(FVector4(Point.Position + WorldPointX + WorldPointY,1),FVector2D(1,1),Point.Color,Point.HitProxyId);
					PointVertices[0] = FSimpleElementVertex(FVector4(Point.Position + WorldPointX - WorldPointY,1),FVector2D(1,0),Point.Color,Point.HitProxyId);

					// Draw the sprite.
					RHIEndDrawPrimitiveUP();
				}
			}
		}

		// Draw the sprites.
		if( Sprites.Num() > 0 )
		{
			SCOPED_DRAW_EVENT(EventSprites)(DEC_SCENE_ITEMS,TEXT("Sprites"));

			for(INT SpriteIndex = 0;SpriteIndex < Sprites.Num();SpriteIndex++)
			{
				const FBatchedSprite& Sprite = Sprites(SpriteIndex);

				// Set the appropriate pixel shader for the mesh.
				// Use alpha testing, but not blending.
				PrepareShaders( BLEND_Masked, Transform, Sprite.Texture, bHitTesting, Gamma);

				// @GEMINI_TODO: Support quad primitives here

				// preallocate some memory to directly fill out
				void* VerticesPtr;
				RHIBeginDrawPrimitiveUP(PT_TriangleStrip, 2, 4, sizeof(FSimpleElementVertex), VerticesPtr);
				FSimpleElementVertex* SpriteVertices = (FSimpleElementVertex*)VerticesPtr;
				// Compute the sprite vertices.
				FVector WorldSpriteX = CameraX * Sprite.SizeX,
					WorldSpriteY = CameraY * -Sprite.SizeY;
				FColor SpriteColor = Sprite.Color;
				FLinearColor LinearSpriteColor = FLinearColor(SpriteColor);
				SpriteVertices[2] = FSimpleElementVertex(FVector4(Sprite.Position - WorldSpriteX - WorldSpriteY,1),FVector2D(0,0),LinearSpriteColor,Sprite.HitProxyId);
				SpriteVertices[3] = FSimpleElementVertex(FVector4(Sprite.Position - WorldSpriteX + WorldSpriteY,1),FVector2D(0,1),LinearSpriteColor,Sprite.HitProxyId);
				SpriteVertices[1] = FSimpleElementVertex(FVector4(Sprite.Position + WorldSpriteX + WorldSpriteY,1),FVector2D(1,1),LinearSpriteColor,Sprite.HitProxyId);
				SpriteVertices[0] = FSimpleElementVertex(FVector4(Sprite.Position + WorldSpriteX - WorldSpriteY,1),FVector2D(1,0),LinearSpriteColor,Sprite.HitProxyId);
				// Draw the sprite.
				RHIEndDrawPrimitiveUP();
			}
		}

		if( MeshElements.Num() > 0 || QuadMeshElements.Num() > 0 )
		{
			SCOPED_DRAW_EVENT(EventMeshes)(DEC_SCENE_ITEMS,TEXT("Meshes"));		

			// Draw the mesh elements.
			for(INT MeshIndex = 0;MeshIndex < MeshElements.Num();MeshIndex++)
			{
				const FBatchedMeshElement& MeshElement = MeshElements(MeshIndex);

				// Set the appropriate pixel shader for the mesh.
				PrepareShaders( MeshElement.BlendMode, Transform, MeshElement.Texture, bHitTesting, Gamma);

				// Draw the mesh.
				RHIDrawIndexedPrimitiveUP(
					PT_TriangleList,
					0,
					MeshElement.MaxVertex - MeshElement.MinVertex + 1,
					MeshElement.Indices.Num() / 3,
					&MeshElement.Indices(0),
					sizeof(INT),
					&MeshVertices(MeshElement.MinVertex),
					sizeof(FSimpleElementVertex)
					);
			}

			// Draw the quad mesh elements.
			for(INT MeshIndex = 0;MeshIndex < QuadMeshElements.Num();MeshIndex++)
			{
				const FBatchedQuadMeshElement& MeshElement = QuadMeshElements(MeshIndex);

				// Set the appropriate pixel shader for the mesh.
				PrepareShaders(MeshElement.BlendMode,Transform,MeshElement.Texture,bHitTesting,Gamma);

				// Draw the mesh.
				RHIDrawPrimitiveUP(
					PT_QuadList,
					MeshElement.Vertices.Num() / 4,
					&MeshElement.Vertices(0),
					sizeof(FSimpleElementVertex)
					);
			}
		}

		// Restore the render target color bias.
		RHISetRenderTargetBias( appPow(2.0f,GCurrentColorExpBias));

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
