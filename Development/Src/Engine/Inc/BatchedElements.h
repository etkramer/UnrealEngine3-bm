/*=============================================================================
	BatchedElements.h: Batched element rendering.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_BATCHEDELEMENTS
#define _INC_BATCHEDELEMENTS

#include "StaticBoundShaderState.h"

/** The type used to store batched line vertices. */
struct FSimpleElementVertex
{
	FVector4 Position;
	FVector2D TextureCoordinate;
	FLinearColor Color;
	FColor HitProxyIdColor;

	FSimpleElementVertex() {}

	FSimpleElementVertex(const FVector4& InPosition,const FVector2D& InTextureCoordinate,const FLinearColor& InColor,FHitProxyId InHitProxyId):
		Position(InPosition),
		TextureCoordinate(InTextureCoordinate),
		Color(InColor),
		HitProxyIdColor(InHitProxyId.GetColor())
	{}
};

/**
* The simple element vertex declaration resource type.
*/
class FSimpleElementVertexDeclaration : public FRenderResource
{
public:

	FVertexDeclarationRHIRef VertexDeclarationRHI;

	// Destructor.
	virtual ~FSimpleElementVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,STRUCT_OFFSET(FSimpleElementVertex,Position),VET_Float4,VEU_Position,0));
		Elements.AddItem(FVertexElement(0,STRUCT_OFFSET(FSimpleElementVertex,TextureCoordinate),VET_Float2,VEU_TextureCoordinate,0));
		Elements.AddItem(FVertexElement(0,STRUCT_OFFSET(FSimpleElementVertex,Color),VET_Float4,VEU_Color,0));
		Elements.AddItem(FVertexElement(0,STRUCT_OFFSET(FSimpleElementVertex,HitProxyIdColor),VET_Color,VEU_Color,1));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/** The simple element vertex declaration. */
extern TGlobalResource<FSimpleElementVertexDeclaration> GSimpleElementVertexDeclaration;

/** Batched elements for later rendering. */
class FBatchedElements
{
public:

	/**
	* Constructor 
	*/
	FBatchedElements()
		:	MaxMeshIndicesAllowed(GDrawUPIndexCheckCount / sizeof(INT))
		,	MaxMeshVerticesAllowed(GDrawUPVertexCheckCount / sizeof(FSimpleElementVertex))
	{
	}

	/** Adds a line to the batch. */
	void AddLine(const FVector& Start,const FVector& End,const FLinearColor& Color,FHitProxyId HitProxyId);

	/** Adds a point to the batch. */
	void AddPoint(const FVector& Position,FLOAT Size,const FLinearColor& Color,FHitProxyId HitProxyId);

	/** Adds a mesh vertex to the batch. */
	INT AddVertex(const FVector4& InPosition,const FVector2D& InTextureCoordinate,const FLinearColor& InColor,FHitProxyId HitProxyId);

	/** Adds a quad mesh vertex to the batch. */
	void AddQuadVertex(const FVector4& InPosition,const FVector2D& InTextureCoordinate,const FLinearColor& InColor,FHitProxyId HitProxyId, const FTexture* Texture,EBlendMode BlendMode);

	/** Adds a triangle to the batch. */
	void AddTriangle(INT V0,INT V1,INT V2,const FTexture* Texture,EBlendMode BlendMode);

	/** 
	* Reserves space in index array for a mesh element
	* 
	* @param NumMeshTriangles - number of triangles to reserve space for
	* @param Texture - used to find the mesh element entry
	* @param BlendMode - used to find the mesh element entry
	*/
	void AddReserveTriangles(INT NumMeshTriangles,const FTexture* Texture,EBlendMode BlendMode);
	/** 
	* Reserves space in mesh vertex array
	* 
	* @param NumMeshVerts - number of verts to reserve space for
	* @param Texture - used to find the mesh element entry
	* @param BlendMode - used to find the mesh element entry
	*/
	void AddReserveVertices(INT NumMeshVerts);

	/** Adds a sprite to the batch. */
	void AddSprite(
		const FVector& Position,
		FLOAT SizeX,
		FLOAT SizeY,
		const FTexture* Texture,
		const FLinearColor& Color,
		FHitProxyId HitProxyId
		);

	/** Draws the batch. */
	UBOOL Draw(const FMatrix& Transform,UINT ViewportSizeX,UINT ViewportSizeY,UBOOL bHitTesting,FLOAT Gamma = 1.0f) const;

	FORCEINLINE UBOOL HasPrimsToDraw() const
	{
		return( LineVertices.Num() || Points.Num() || Sprites.Num() || MeshElements.Num() || QuadMeshElements.Num() );
	}

private:

	TArray<FSimpleElementVertex> LineVertices;

	struct FBatchedPoint
	{
		FVector Position;
		FLOAT Size;
		FColor Color;
		FHitProxyId HitProxyId;
	};
	TArray<FBatchedPoint> Points;

	struct FBatchedSprite
	{
		FVector Position;
		FLOAT SizeX;
		FLOAT SizeY;
		const FTexture* Texture;
		FColor Color;
		FHitProxyId HitProxyId;
	};
	TArray<FBatchedSprite> Sprites;

	struct FBatchedMeshElement
	{
		/** starting index in vertex buffer for this batch */
		UINT MinVertex;
		/** largest vertex index used by this batch */
		UINT MaxVertex;
		/** index buffer for triangles */
		TArray<INT,TInlineAllocator<6> > Indices;
		/** all triangles in this batch draw with the same texture */
		const FTexture* Texture;
		/** all triangles in this batch draw with the same blend mode */
		EBlendMode BlendMode;
	};

	struct FBatchedQuadMeshElement
	{
		TArray<FSimpleElementVertex> Vertices;
		const FTexture* Texture;
		EBlendMode BlendMode;
	};

	/** Max number of mesh index entries that will fit in a DrawPriUP call */
	INT MaxMeshIndicesAllowed;
	/** Max number of mesh vertices that will fit in a DrawPriUP call */
	INT MaxMeshVerticesAllowed;

	TArray<FBatchedMeshElement,TInlineAllocator<1> > MeshElements;
	TArray<FSimpleElementVertex,TInlineAllocator<4> > MeshVertices;

	TArray<FBatchedQuadMeshElement> QuadMeshElements;

	/** bound shader state for the fast path */
	static FGlobalBoundShaderState SimpleBoundShaderState;
	/** bound shader state for the regular mesh elements */
	static FGlobalBoundShaderState RegularBoundShaderState;
	/** bound shader state for masked mesh elements */
	static FGlobalBoundShaderState MaskedBoundShaderState;
	/** bound shader state for the hit testing mesh elements */
	static FGlobalBoundShaderState HitTestingBoundShaderState;

	/*
	 * Sets the appropriate vertex and pixel shader.
	 */
	void PrepareShaders(
		EBlendMode BlendMode,
		const FMatrix& Transform,
		const FTexture* BaseTexture,
		UBOOL bHitTesting,
		FLOAT Gamma
		) const;
};

#endif
