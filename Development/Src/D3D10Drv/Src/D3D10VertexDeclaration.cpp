/*=============================================================================
	D3D10VertexDeclaration.cpp: D3D vertex declaration RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

IMPLEMENT_COMPARE_CONSTREF(
						   D3D10_INPUT_ELEMENT_DESC,
						   D3DVertexDeclaration,
{ return ((INT)A.AlignedByteOffset + A.InputSlot * MAXWORD) - ((INT)B.AlignedByteOffset + B.InputSlot * MAXWORD); }
)

FD3D10VertexDeclaration::FD3D10VertexDeclaration(const FVertexDeclarationElementList& InElements)
{
	for(UINT ElementIndex = 0;ElementIndex < InElements.Num();ElementIndex++)
	{
		const FVertexElement& Element = InElements(ElementIndex);
		D3D10_INPUT_ELEMENT_DESC D3DElement;
		D3DElement.InputSlot = Element.StreamIndex;
		D3DElement.AlignedByteOffset = Element.Offset;
		switch(Element.Type)
		{
		case VET_Float1:		D3DElement.Format = DXGI_FORMAT_R32_FLOAT; break;
		case VET_Float2:		D3DElement.Format = DXGI_FORMAT_R32G32_FLOAT; break;
		case VET_Float3:		D3DElement.Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
		case VET_Float4:		D3DElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
		case VET_PackedNormal:	D3DElement.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break; //TODO: UINT doesn't work because D3D10 squishes it to 0 in the IA-VS conversion
		case VET_UByte4:		D3DElement.Format = DXGI_FORMAT_R8G8B8A8_UINT; break; //TODO: SINT, blendindices
		case VET_UByte4N:		D3DElement.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
		case VET_Color:			D3DElement.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
		case VET_Short2:		D3DElement.Format = DXGI_FORMAT_R16G16_SINT; break;
		case VET_Short2N:		D3DElement.Format = DXGI_FORMAT_R16G16_SNORM; break;
		case VET_Half2:			D3DElement.Format = DXGI_FORMAT_R16G16_FLOAT; break;
		default: appErrorf(TEXT("Unknown RHI vertex element type %u"),InElements(ElementIndex).Type);
		};
		switch(Element.Usage)
		{
		case VEU_Position:			D3DElement.SemanticName = "POSITION"; break;
		case VEU_TextureCoordinate:	D3DElement.SemanticName = "TEXCOORD"; break;
		case VEU_BlendWeight:		D3DElement.SemanticName = "BLENDWEIGHT"; break;
		case VEU_BlendIndices:		D3DElement.SemanticName = "BLENDINDICES"; break;
		case VEU_Normal:			D3DElement.SemanticName = "NORMAL"; break;
		case VEU_Tangent:			D3DElement.SemanticName = "TANGENT"; break;
		case VEU_Binormal:			D3DElement.SemanticName = "BINORMAL"; break;
		case VEU_Color:				D3DElement.SemanticName = "COLOR"; break;
		};
		D3DElement.SemanticIndex = Element.UsageIndex;
		D3DElement.InputSlotClass = Element.bUseInstanceIndex ? D3D10_INPUT_PER_INSTANCE_DATA : D3D10_INPUT_PER_VERTEX_DATA;

		// This is a divisor to apply to the instance index used to read from this stream.
		D3DElement.InstanceDataStepRate = Element.bUseInstanceIndex ? 1 : 0;

		VertexElements.AddItem(D3DElement);
	}

	// Sort the D3DVERTEXELEMENTs by stream then offset.
	Sort<USE_COMPARE_CONSTREF(D3D10_INPUT_ELEMENT_DESC,D3DVertexDeclaration)>(&VertexElements(0),InElements.Num());
}

FVertexDeclarationRHIRef FD3D10DynamicRHI::CreateVertexDeclaration(const FVertexDeclarationElementList& Elements)
{
	return new FD3D10VertexDeclaration(Elements);
}
