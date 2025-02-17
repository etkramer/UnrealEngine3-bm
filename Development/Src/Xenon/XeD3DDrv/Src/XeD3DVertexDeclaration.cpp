/*=============================================================================
	D3DVertexDeclaration.cpp: D3D vertex declaration RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI

IMPLEMENT_COMPARE_CONSTREF(
						   D3DVERTEXELEMENT9,
						   D3DVertexDeclaration,
{ return ((INT)A.Offset + A.Stream * MAXWORD) - ((INT)B.Offset + B.Stream * MAXWORD); }
)

/**
* Key used to map a set of vertex element definitions to 
* an IDirect3DVertexDeclaration9 resource
*/
class FVertexDeclarationKey
{
public:
	/**
	* Constructor
	* @param InNumVertexEelements - number of elements to allocate
	*/
	FVertexDeclarationKey(const FVertexDeclarationElementList& InElements)
	{
		for(UINT ElementIndex = 0;ElementIndex < InElements.Num();ElementIndex++)
		{
			D3DVERTEXELEMENT9 VertexElement;
			VertexElement.Stream = InElements(ElementIndex).StreamIndex;
			VertexElement.Offset = InElements(ElementIndex).Offset;
			switch(InElements(ElementIndex).Type)
			{
			case VET_Float1:		VertexElement.Type = D3DDECLTYPE_FLOAT1; break;
			case VET_Float2:		VertexElement.Type = D3DDECLTYPE_FLOAT2; break;
			case VET_Float3:		VertexElement.Type = D3DDECLTYPE_FLOAT3; break;
			case VET_Float4:		VertexElement.Type = D3DDECLTYPE_FLOAT4; break;
			case VET_PackedNormal:	VertexElement.Type = D3DDECLTYPE_UBYTE4; break;
			case VET_UByte4:		VertexElement.Type = D3DDECLTYPE_UBYTE4; break;
			case VET_UByte4N:		VertexElement.Type = D3DDECLTYPE_UBYTE4N; break;
			case VET_Color:			VertexElement.Type = D3DDECLTYPE_D3DCOLOR; break;
			case VET_Short2:		VertexElement.Type = D3DDECLTYPE_SHORT2; break;
			case VET_Short2N:		VertexElement.Type = D3DDECLTYPE_SHORT2N; break;
			case VET_Half2:			VertexElement.Type = D3DDECLTYPE_FLOAT16_2; break;
			default: appErrorf(TEXT("Unknown RHI vertex element type %u"),InElements(ElementIndex).Type);
			};
			VertexElement.Method = D3DDECLMETHOD_DEFAULT;
			switch(InElements(ElementIndex).Usage)
			{
			case VEU_Position:			VertexElement.Usage = D3DDECLUSAGE_POSITION; break;
			case VEU_TextureCoordinate:	VertexElement.Usage = D3DDECLUSAGE_TEXCOORD; break;
			case VEU_BlendWeight:		VertexElement.Usage = D3DDECLUSAGE_BLENDWEIGHT; break;
			case VEU_BlendIndices:		VertexElement.Usage = D3DDECLUSAGE_BLENDINDICES; break;
			case VEU_Normal:			VertexElement.Usage = D3DDECLUSAGE_NORMAL; break;
			case VEU_Tangent:			VertexElement.Usage = D3DDECLUSAGE_TANGENT; break;
			case VEU_Binormal:			VertexElement.Usage = D3DDECLUSAGE_BINORMAL; break;
			case VEU_Color:				VertexElement.Usage = D3DDECLUSAGE_COLOR; break;
			};
			VertexElement.UsageIndex = InElements(ElementIndex).UsageIndex;

			VertexElements.AddItem(VertexElement);
		}

		// Sort the D3DVERTEXELEMENTs by stream then offset.
		Sort<USE_COMPARE_CONSTREF(D3DVERTEXELEMENT9,D3DVertexDeclaration)>(GetVertexElements(),InElements.Num());

		// Terminate the vertex element list.
		D3DVERTEXELEMENT9 EndElement = D3DDECL_END();
		VertexElements.AddItem(EndElement);

		Hash = appMemCrc(GetVertexElements(), sizeof(D3DVERTEXELEMENT9) * VertexElements.Num()); 
	}

	/**
	* @return TRUE if the decls are the same
	* @param Other - instance to compare against
	*/
	UBOOL operator == (const FVertexDeclarationKey &Other) const
	{
		// same number and matching element
		return	VertexElements.Num() == Other.VertexElements.Num() && 
				!appMemcmp(Other.GetVertexElements(), GetVertexElements(), sizeof(D3DVERTEXELEMENT9) * VertexElements.Num());
	}

	// Accessors.
	D3DVERTEXELEMENT9* GetVertexElements() 
	{ 
		return &VertexElements(0);
	}
	const D3DVERTEXELEMENT9* GetVertexElements() const
	{ 
		return &VertexElements(0);
	}

	/** 
	* @return hash value for this type 
	*/
	friend DWORD GetTypeHash(const FVertexDeclarationKey &Key)
	{
		return Key.Hash;
	}

private:
	/** array of D3D vertex elements */
	TStaticArray<D3DVERTEXELEMENT9,MaxVertexElementCount + 1> VertexElements;
	/** hash value based on vertex elements */
	DWORD Hash;
};

/**
* Maps 
*/
class FVertexDeclarationCache
{
public:
	/**
	* Get a vertex declaration
	* Tries to find the decl within the set. Creates a new one if it doesn't exist
	* @param Declaration - key containing the vertex elements
	* @return D3D vertex decl object
	*/
	IDirect3DVertexDeclaration9 * GetVertexDeclaration(const FVertexDeclarationKey& Declaration)
	{
		TRefCountPtr<IDirect3DVertexDeclaration9>* Value = VertexDeclarationMap.Find(Declaration);
		if (!Value)
		{
			TRefCountPtr<IDirect3DVertexDeclaration9> NewDeclaration;
			VERIFYD3DRESULT(GDirect3DDevice->CreateVertexDeclaration(Declaration.GetVertexElements(),NewDeclaration.GetInitReference()));
			Value = &VertexDeclarationMap.Set(Declaration, NewDeclaration);
		}
		return *Value;
	}

private:
	/** maps key consisting of vertex elements to the D3D decl */
	TMap<FVertexDeclarationKey, TRefCountPtr<IDirect3DVertexDeclaration9> > VertexDeclarationMap;
};

/** global set of all vertex decls */
FVertexDeclarationCache GVertexDeclarationCache;

FVertexDeclarationRHIRef RHICreateVertexDeclaration(const FVertexDeclarationElementList& Elements)
{
	// Create the vertex declaration.
	FVertexDeclarationRHIRef VertexDeclaration;
	VertexDeclaration = GVertexDeclarationCache.GetVertexDeclaration(FVertexDeclarationKey(Elements));

	return VertexDeclaration;
}

#endif
