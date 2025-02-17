/*=============================================================================
	VertexFactory.cpp: Vertex factory implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/**
 * @return The global shader factory list.
 */
TLinkedList<FVertexFactoryType*>*& FVertexFactoryType::GetTypeList()
{
	static TLinkedList<FVertexFactoryType*>* TypeList = NULL;
	return TypeList;
}

/**
 * Gets a list of FVertexFactoryType whose source file no longer matches what that type was compiled with
 */
void FVertexFactoryType::GetOutdatedTypes(TArray<FVertexFactoryType*>& OutdatedVFTypes)
{
	for(TLinkedList<FVertexFactoryType*>::TIterator It(GetTypeList()); It; It.Next())
	{
		const DWORD* VFTypeCRC = UShaderCache::GetVertexFactoryTypeCRC(*It, GRHIShaderPlatform);
		DWORD CurrentCRC = It->GetSourceCRC();

		if (VFTypeCRC != NULL && CurrentCRC != *VFTypeCRC)
		{
			warnf(TEXT("VertexFactoryType %s is outdated"), It->GetName());
			OutdatedVFTypes.Push(*It);
			//remove the type's crc from the cache, since we know it is outdated now
			UShaderCache::RemoveVertexFactoryTypeCRC(*It, GRHIShaderPlatform);
		}
	}
}

/**
 * Finds a FVertexFactoryType by name.
 */
FVertexFactoryType* FVertexFactoryType::GetVFByName(const FString& VFName)
{
	for(TLinkedList<FVertexFactoryType*>::TIterator It(GetTypeList()); It; It.Next())
	{
		FString CurrentVFName = FString(It->GetName());
		if (CurrentVFName == VFName)
		{
			return *It;
		}
	}
	return NULL;
}

FVertexFactoryType::FVertexFactoryType(
	const TCHAR* InName,
	const TCHAR* InShaderFilename,
	UBOOL bInUsedWithMaterials,
	UBOOL bInSupportsStaticLighting,
	UBOOL bInSupportsPrecisePrevWorldPos,
	ConstructParametersType InConstructParameters,
	ShouldCacheType InShouldCache,
	ModifyCompilationEnvironmentType InModifyCompilationEnvironment,
	INT InMinPackageVersion,
	INT InMinLicenseePackageVersion
	):
	Name(InName),
	ShaderFilename(InShaderFilename),
	TypeName(InName),
	bUsedWithMaterials(bInUsedWithMaterials),
	bSupportsStaticLighting(bInSupportsStaticLighting),
	bSupportsPrecisePrevWorldPos(bInSupportsPrecisePrevWorldPos),
	ConstructParameters(InConstructParameters),
	ShouldCacheRef(InShouldCache),
	ModifyCompilationEnvironmentRef(InModifyCompilationEnvironment),
	MinPackageVersion(InMinPackageVersion),
	MinLicenseePackageVersion(InMinLicenseePackageVersion)
{
	// Add this vertex factory type to the global list.
	(new TLinkedList<FVertexFactoryType*>(this))->Link(GetTypeList());

	// Assign the vertex factory type the next unassigned hash index.
	static DWORD NextHashIndex = 0;
	HashIndex = NextHashIndex++;
}

/**
 * Calculates a CRC based on this vertex factory type's source code and includes
 */
DWORD FVertexFactoryType::GetSourceCRC()
{
	return GetShaderFileCRC(GetShaderFilename());
}

FArchive& operator<<(FArchive& Ar,FVertexFactoryType*& TypeRef)
{
	if(Ar.IsSaving())
	{
		FName TypeName = TypeRef ? FName(TypeRef->GetName()) : NAME_None;
		Ar << TypeName;
	}
	else if(Ar.IsLoading())
	{
		FName TypeName = NAME_None;
		Ar << TypeName;
		TypeRef = FindVertexFactoryType(TypeName);
	}
	return Ar;
}

FVertexFactoryType* FindVertexFactoryType(FName TypeName)
{
	// Search the global vertex factory list for a type with a matching name.
	for(TLinkedList<FVertexFactoryType*>::TIterator VertexFactoryTypeIt(FVertexFactoryType::GetTypeList());VertexFactoryTypeIt;VertexFactoryTypeIt.Next())
	{
		if(VertexFactoryTypeIt->GetFName() == TypeName)
		{
			return *VertexFactoryTypeIt;
		}
	}
	return NULL;
}

FVertexFactory::DataType::DataType()
{
	LightMapStream.Offset = 0,
	LightMapStream.DirectionalStride = sizeof(FQuantizedDirectionalLightSample);
	LightMapStream.SimpleStride = sizeof(FQuantizedSimpleLightSample);
	LightMapStream.bUseInstanceIndex = FALSE;
	NumVerticesPerInstance = 0;
	NumInstances = 1;
}

void FVertexFactory::Set() const
{
	for(UINT StreamIndex = 0;StreamIndex < Streams.Num();StreamIndex++)
	{
		const FVertexStream& Stream = Streams(StreamIndex);
		RHISetStreamSource(StreamIndex,Stream.VertexBuffer->VertexBufferRHI,Stream.Stride,Stream.bUseInstanceIndex,Data.NumVerticesPerInstance,Data.NumInstances);
	}
}

void FVertexFactory::SetPositionStream() const
{
	// Set the predefined vertex streams.
	for(UINT StreamIndex = 0;StreamIndex < PositionStream.Num();StreamIndex++)
	{
		const FVertexStream& Stream = PositionStream(StreamIndex);
		RHISetStreamSource(StreamIndex,Stream.VertexBuffer->VertexBufferRHI,Stream.Stride,Stream.bUseInstanceIndex,Data.NumVerticesPerInstance,Data.NumInstances);
	}
}

void FVertexFactory::SetVertexShadowMap(const FVertexBuffer* ShadowMapVertexBuffer) const
{
	Set();
	// Set the shadow-map vertex stream.
	RHISetStreamSource(Streams.Num(),ShadowMapVertexBuffer->VertexBufferRHI,sizeof(FLOAT),FALSE,0,1);
}

void FVertexFactory::SetVertexLightMap(const FVertexBuffer* LightMapVertexBuffer,UBOOL bUseDirectionalLightMap) const
{
	Set();
	// Set the shadow-map vertex stream.
	BYTE LightMapStride = bUseDirectionalLightMap ? Data.LightMapStream.DirectionalStride : Data.LightMapStream.SimpleStride;
	RHISetStreamSource(Streams.Num(),LightMapVertexBuffer->VertexBufferRHI,LightMapStride,Data.LightMapStream.bUseInstanceIndex,Data.NumVerticesPerInstance,Data.NumInstances);
}

void FVertexFactory::ReleaseRHI()
{
	Declaration.SafeRelease();
	PositionDeclaration.SafeRelease();
	VertexShadowMapDeclaration.SafeRelease();
	DirectionalVertexLightMapDeclaration.SafeRelease();
	SimpleVertexLightMapDeclaration.SafeRelease();
	Streams.Empty();
	PositionStream.Empty();
}

/**
* Fill in array of strides from this factory's vertex streams without shadow/light maps
* @param OutStreamStrides - output array of # MaxVertexElementCount stream strides to fill
*/
INT FVertexFactory::GetStreamStrides(DWORD *OutStreamStrides, UBOOL bPadWithZeroes) const
{
	UINT StreamIndex;
	for(StreamIndex = 0;StreamIndex < Streams.Num();++StreamIndex)
	{
		OutStreamStrides[StreamIndex] = Streams(StreamIndex).Stride;
	}
	if (bPadWithZeroes)
	{
		// Pad stream strides with 0's to be safe (they can be used in hashes elsewhere)
		for (;StreamIndex < MaxVertexElementCount;++StreamIndex)
		{
			OutStreamStrides[StreamIndex] = 0;
		}
	}
	return StreamIndex;
}

/**
* Fill in array of strides from this factory's position only vertex streams
* @param OutStreamStrides - output array of # MaxVertexElementCount stream strides to fill
*/
void FVertexFactory::GetPositionStreamStride(DWORD *OutStreamStrides) const
{
	UINT StreamIndex;
	for(StreamIndex = 0;StreamIndex < PositionStream.Num();++StreamIndex)
	{
		OutStreamStrides[StreamIndex] = PositionStream(StreamIndex).Stride;
	}
	// Pad stream strides with 0's to be safe (they can be used in hashes elsewhere)
	for (;StreamIndex < MaxVertexElementCount;++StreamIndex)
	{
		OutStreamStrides[StreamIndex] = 0;
	}
}

/**
* Fill in array of strides from this factory's vertex streams when using shadow-maps
* @param OutStreamStrides - output array of # MaxVertexElementCount stream strides to fill
*/
void FVertexFactory::GetVertexShadowMapStreamStrides(DWORD *OutStreamStrides) const
{
	UINT StreamIndex = GetStreamStrides(OutStreamStrides, FALSE);
	OutStreamStrides[StreamIndex ++] = sizeof(FLOAT);
	// Pad stream strides with 0's to be safe (they can be used in hashes elsewhere)
	for (;StreamIndex < MaxVertexElementCount;++StreamIndex)
	{
		OutStreamStrides[StreamIndex] = 0;
	}
}

/**
* Fill in array of strides from this factory's vertex streams when using light-maps
* @param OutStreamStrides - output array of # MaxVertexElementCount stream strides to fill
*/
void FVertexFactory::GetVertexLightMapStreamStrides(DWORD *OutStreamStrides,UBOOL bUseDirectionalLightMap) const
{
	UINT StreamIndex = GetStreamStrides(OutStreamStrides, FALSE);
	BYTE LightMapStride = bUseDirectionalLightMap ? Data.LightMapStream.DirectionalStride : Data.LightMapStream.SimpleStride;
	OutStreamStrides[StreamIndex ++] = LightMapStride;
	// Pad stream strides with 0's to be safe (they can be used in hashes elsewhere)
	for (;StreamIndex < MaxVertexElementCount;++StreamIndex)
	{
		OutStreamStrides[StreamIndex] = 0;
	}
}

FVertexElement FVertexFactory::AccessStreamComponent(const FVertexStreamComponent& Component,BYTE Usage,BYTE UsageIndex)
{
	FVertexStream VertexStream;
	VertexStream.VertexBuffer = Component.VertexBuffer;
	VertexStream.Stride = Component.Stride;
	VertexStream.bUseInstanceIndex = Component.bUseInstanceIndex;

	return FVertexElement(Streams.AddUniqueItem(VertexStream),Component.Offset,Component.Type,Usage,UsageIndex,Component.bUseInstanceIndex,Data.NumVerticesPerInstance);
}

FVertexElement FVertexFactory::AccessPositionStreamComponent(const FVertexStreamComponent& Component,BYTE Usage,BYTE UsageIndex)
{
	FVertexStream VertexStream;
	VertexStream.VertexBuffer = Component.VertexBuffer;
	VertexStream.Stride = Component.Stride;
	VertexStream.bUseInstanceIndex = Component.bUseInstanceIndex;

	return FVertexElement(PositionStream.AddUniqueItem(VertexStream),Component.Offset,Component.Type,Usage,UsageIndex,Component.bUseInstanceIndex,Data.NumVerticesPerInstance);
}

//TTP:51684
/**
 *	GetVertexElementSize returns the size of data based on its type
 *	it's needed for FixFXCardDeclarator function
 */
static BYTE GetVertexElementSize( BYTE Type )
{
	switch( Type )
	{
		case VET_Float1:
			return 4;
		case VET_Float2:
			return 8;
		case VET_Float3:
			return 12;
		case VET_Float4:
			return 16;
		case VET_PackedNormal:
		case VET_UByte4:
		case VET_UByte4N:
		case VET_Color:
		case VET_Short2:
		case VET_Short2N:
		case VET_Half2:
			return sizeof 4;
		default:
			return 0;
	}
}

/**
 * Patches the declaration so vertex stream offsets are unique. This is required for e.g. GeForce FX cards, which don't support redundant 
 * offsets in the declaration. We're unable to make that many vertex elements point to the same offset so the function moves redundant 
 * declarations to higher offsets, pointing to garbage data.
 */
static void PatchVertexStreamOffsetsToBeUnique( FVertexDeclarationElementList& Elements )
{
	// check every vertex element
	for ( UINT e = 0; e < Elements.Num(); e++ )
	{
		// check if there's an element that reads from the same offset
		for ( UINT i = 0; i < Elements.Num(); i++ )
		{
			// but only in the same stream and if it's not the same element
			if ( ( Elements( i ).StreamIndex == Elements( e ).StreamIndex ) && ( Elements( i ).Offset == Elements( e ).Offset ) && ( e != i ) )
			{
				// the id of the highest offset element is stored here (it doesn't need to be the last element in the declarator because the last element may belong to another StreamIndex
				UINT MaxOffsetID = i;

				// find the highest offset element
				for ( UINT j = 0; j < Elements.Num(); j++ )
				{
					if ( ( Elements( j ).StreamIndex == Elements( e ).StreamIndex ) && ( Elements( MaxOffsetID ).Offset < Elements( j ).Offset ) )
					{
						MaxOffsetID = j;
					}
				}

				// get the size of the highest offset element, it's needed for the redundant element new offset
				BYTE PreviousElementSize = GetVertexElementSize( Elements( MaxOffsetID ).Type );

				// prepare a new vertex element
				FVertexElement VertElement;
				VertElement.Offset		= Elements( MaxOffsetID ).Offset + PreviousElementSize;
				VertElement.StreamIndex = Elements( i ).StreamIndex;
				VertElement.Type		= Elements( i ).Type;
				VertElement.Usage		= Elements( i ).Usage;
				VertElement.UsageIndex	= Elements( i ).UsageIndex;
				VertElement.bUseInstanceIndex = Elements(i).bUseInstanceIndex;
				VertElement.NumVerticesPerInstance = Elements(i).NumVerticesPerInstance;

				// remove the old redundant element
				Elements.Remove( i );

				// add a new element with "correct" offset
				Elements.AddItem( VertElement );

				// make sure that when the element has been removed its index is taken by the next element, so we must take care of it too
				i = i == 0 ? 0 : i - 1;
			}
		}
	}
}

void FVertexFactory::InitDeclaration(
	FVertexDeclarationElementList& Elements, 
	const DataType& InData, 
	UBOOL bCreateShadowMapDeclaration, 
	UBOOL bCreateDirectionalLightMapDeclaration, 
	UBOOL bCreateSimpleLightMapDeclaration)
{
	// Make a copy of the vertex factory data.
	Data = InData;

	// If GFFX detected, patch up the declarator
	if( !GVertexElementsCanShareStreamOffset )
	{
		PatchVertexStreamOffsetsToBeUnique( Elements );
	}

	// Create the vertex declaration for rendering the factory normally.
	Declaration = RHICreateVertexDeclaration(Elements);

	if (bCreateShadowMapDeclaration)
	{
		// Create the vertex declaration for rendering the factory with a vertex shadow-map.
		FVertexDeclarationElementList ShadowMapElements = Elements;
		ShadowMapElements.AddItem(FVertexElement(Streams.Num(),0,VET_Float1,VEU_BlendWeight,0));
		VertexShadowMapDeclaration = RHICreateVertexDeclaration(ShadowMapElements);
	}

	if (bCreateDirectionalLightMapDeclaration)
	{
		// Create the vertex declaration for rendering the factory with a directional vertex light-map.
		FVertexDeclarationElementList LightMapElements = Elements;
		for(INT CoefficientIndex = 0;CoefficientIndex < NUM_DIRECTIONAL_LIGHTMAP_COEF;CoefficientIndex++)
		{
			LightMapElements.AddItem(FVertexElement(
				Streams.Num(),
				Data.LightMapStream.Offset + STRUCT_OFFSET(FQuantizedDirectionalLightSample,Coefficients[CoefficientIndex]),
				VET_Color,
				VEU_TextureCoordinate,
				5 + CoefficientIndex,
				Data.LightMapStream.bUseInstanceIndex,
				Data.NumVerticesPerInstance
				));
		}
		DirectionalVertexLightMapDeclaration = RHICreateVertexDeclaration(LightMapElements);
	}

	if (bCreateSimpleLightMapDeclaration)
	{
		// Create the vertex declaration for rendering the factory with a simple vertex light-map.
		FVertexDeclarationElementList LightMapElements = Elements;
		LightMapElements.AddItem(FVertexElement(
			Streams.Num(),
			Data.LightMapStream.Offset + STRUCT_OFFSET(FQuantizedSimpleLightSample,Coefficients[0]),
			VET_Color,
			VEU_TextureCoordinate,
			5,
			Data.LightMapStream.bUseInstanceIndex,
			Data.NumVerticesPerInstance
			));
		SimpleVertexLightMapDeclaration = RHICreateVertexDeclaration(LightMapElements);
	}
}

void FVertexFactory::InitPositionDeclaration(const FVertexDeclarationElementList& Elements)
{
	// Create the vertex declaration for rendering the factory normally.
	PositionDeclaration = RHICreateVertexDeclaration(Elements);
}

UBOOL operator<<(FArchive& Ar,FVertexFactoryParameterRef& Ref)
{
	UBOOL bShaderHasOutdatedParameters = FALSE;

	Ar << Ref.VertexFactoryType;
	if(Ar.IsLoading())
	{
		delete Ref.Parameters;
		if(Ref.VertexFactoryType && Ar.Ver() >= Ref.VertexFactoryType->GetMinPackageVersion() && Ar.LicenseeVer() >= Ref.VertexFactoryType->GetMinLicenseePackageVersion())
		{
			Ref.Parameters = Ref.VertexFactoryType->CreateShaderParameters();
		}
		else
		{
			bShaderHasOutdatedParameters = TRUE;
			Ref.Parameters = NULL;
		}
	}

	// Need to be able to skip over parameters for no longer existing vertex factories.
	INT SkipOffset = Ar.Tell();
	// Write placeholder.
	Ar << SkipOffset;

	if(Ref.Parameters)
	{
		Ref.Parameters->Serialize(Ar);
	}
	else if(Ar.IsLoading())
	{
		Ar.Seek( SkipOffset );
	}
	else if (Ar.IsSaving())
	{
		//saving a NULL FVertexFactoryParameterRef will corrupt the shader cache
		//the shader containing this parameter ref should have been thrown away on load
		const TCHAR* VertexFactoryName = Ref.VertexFactoryType != NULL ? Ref.VertexFactoryType->GetName() : TEXT("NOT FOUND");
		appErrorf(TEXT("Attempting to save a NULL FVertexFactoryParameterRef for VF %s!"), VertexFactoryName);
	}

	if( Ar.IsSaving() )
	{
		INT EndOffset = Ar.Tell();
		Ar.Seek( SkipOffset );
		Ar << EndOffset;
		Ar.Seek( EndOffset );
	}

	return bShaderHasOutdatedParameters;
}
