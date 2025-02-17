/*=============================================================================
	UnShadowMap.cpp: Shadow-map implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#include "UnTextureLayout.h"

IMPLEMENT_CLASS(UShadowMapTexture2D);
IMPLEMENT_CLASS(UShadowMap2D);
IMPLEMENT_CLASS(UShadowMap1D);

void UShadowMapTexture2D::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

// Editor thumbnail viewer interface. Unfortunately this is duplicated across all UTexture implementations.

/** 
 * Returns a one line description of an object for viewing in the generic browser
 */
FString UShadowMapTexture2D::GetDesc()
{
	return FString::Printf( TEXT("Shadowmap: %dx%d [%s]"), SizeX, SizeY, GPixelFormats[Format].Name );
}

/** 
 * Returns detailed info to populate listview columns
 */
FString UShadowMapTexture2D::GetDetailedDescription( INT InIndex )
{
	FString Description = TEXT( "" );
	switch( InIndex )
	{
	case 0:
		Description = FString::Printf( TEXT( "%dx%d" ), SizeX, SizeY );
		break;
	case 1:
		Description = GPixelFormats[Format].Name;
		break;
	}
	return( Description );
}

struct FQuantizedShadowSample
{
	BYTE Visibility;
	BYTE Coverage;

	FQuantizedShadowSample() {}
	FQuantizedShadowSample(const FShadowSample& InSample)
	{
		Visibility = (BYTE)Clamp<INT>(appTrunc(appPow(InSample.Visibility,1.0f / 2.2f) * 255.0f),0,255);
		Coverage = InSample.IsMapped ? 255 : 0;
	}
};

struct FShadowMapAllocation
{
	UShadowMap2D* ShadowMap;
	UMaterialInterface* Material;
	UObject* TextureOuter;
	FGuid LightGuid;
	UINT BaseX;
	UINT BaseY;
	UINT SizeX;
	UINT SizeY;
	TArray<FQuantizedShadowSample> RawData;
};

struct FShadowMapPendingTexture : FTextureLayout
{
	TArray<FShadowMapAllocation*> Allocations;

	UObject* Outer;
	UMaterialInterface* Material;
	FGuid LightGuid;

	/**
	 * Minimal initialization constructor.
	 */
	FShadowMapPendingTexture(UINT InSizeX,UINT InSizeY):
		FTextureLayout(1,1,InSizeX,InSizeY,true)
	{}
};

static TIndirectArray<FShadowMapAllocation> PendingShadowMaps;
static UINT PendingShadowMapSize;

/**
 * A utility function which calculates the address of a single texel from a shadow-map texture.
 * @param	ShadowData - Base pointer to shadow-map data
 * @param	X - The X coordinate of the texel to access.
 * @param	Y - The Y coordinate of the texel to access.
 * @param	SizeX - The width of the shadow-map texture
 * @return	a FLOAT value representing the texel
 */
static FORCEINLINE FLOAT AccessShadowTexel( BYTE* ShadowData, INT X, INT Y, INT SizeX )
{
	return ShadowData[Y * SizeX + X];
}

FLOAT UShadowMap2D::SampleTexture(const FVector2D& Coordinate) const
{
	check(IsValid());

	BYTE* ShadowData = (BYTE*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);

	FLOAT FloatX = (Coordinate.X * CoordinateScale.X + CoordinateBias.X) * Texture->SizeX - 0.5f;
	FLOAT FloatY = (Coordinate.Y * CoordinateScale.Y + CoordinateBias.Y) * Texture->SizeY - 0.5f;
	INT IntX = appTrunc(FloatX);
	INT IntY = appTrunc(FloatY);
	FLOAT FracX = FloatX - IntX;
	FLOAT FracY = FloatY - IntY;
	FLOAT SampledTextureValue = 0;

	if(IntX < (INT)Texture->SizeX - 1 && IntY < (INT)Texture->SizeY - 1)
	{
		SampledTextureValue = BiLerp(
				AccessShadowTexel(ShadowData,IntX,IntY,Texture->SizeX),
				AccessShadowTexel(ShadowData,IntX + 1,IntY,Texture->SizeX),
				AccessShadowTexel(ShadowData,IntX,IntY + 1,Texture->SizeX),
				AccessShadowTexel(ShadowData,IntX + 1,IntY + 1,Texture->SizeX),
				FracX,
				FracY
				) / 255.0f;
	}
	else if(IntX < (INT)Texture->SizeX - 1)
	{
		SampledTextureValue = Lerp(
				AccessShadowTexel(ShadowData,IntX,IntY,Texture->SizeX),
				AccessShadowTexel(ShadowData,IntX + 1,IntY,Texture->SizeX),
				FracX
				) / 255.0f;
	}
	else if(IntY < (INT)Texture->SizeY - 1)
	{
		SampledTextureValue = Lerp(
				AccessShadowTexel(ShadowData,IntX,IntY,Texture->SizeX),
				AccessShadowTexel(ShadowData,IntX,IntY + 1,Texture->SizeX),
				FracY
				) / 255.0f;
	}
	else
	{
		SampledTextureValue = AccessShadowTexel(ShadowData,IntX,IntY,Texture->SizeX) / 255.0f;
	}

	Texture->Mips(0).Data.Unlock();

	return SampledTextureValue;
}

UShadowMap2D::UShadowMap2D(const FShadowMapData2D& RawData,const FGuid& InLightGuid,UMaterialInterface* Material,const FBoxSphereBounds& Bounds):
	LightGuid(InLightGuid)
{
	// Add a pending allocation for this shadow-map.
	FShadowMapAllocation* Allocation = new(PendingShadowMaps) FShadowMapAllocation;
	Allocation->ShadowMap = this;
	Allocation->Material = Material;
	Allocation->LightGuid = InLightGuid;
	Allocation->TextureOuter = GetOutermost();
	Allocation->SizeX = RawData.GetSizeX();
	Allocation->SizeY = RawData.GetSizeY();
	Allocation->RawData.Empty(Allocation->SizeX * Allocation->SizeY);
	for(UINT Y = 0;Y < Allocation->SizeY;Y++)
	{
		for(UINT X = 0;X < Allocation->SizeX;X++)
		{
			Allocation->RawData.AddItem(RawData(X,Y));
		}
	}
	
	// Track the size of pending light-maps.
	PendingShadowMapSize += Allocation->SizeX * Allocation->SizeY;

	// Once there are enough pending shadow-maps, flush encoding.
	const UINT PackedLightAndShadowMapTextureSize = GWorld->GetWorldInfo()->PackedLightAndShadowMapTextureSize;
	const UINT MaxPendingShadowMapSize = Square(PackedLightAndShadowMapTextureSize) * 4;
	if(PendingShadowMapSize >= MaxPendingShadowMapSize)
	{
		FinishEncoding();
	}
}

IMPLEMENT_COMPARE_POINTER(FShadowMapAllocation,UnShadowMap,{ return B->SizeX * B->SizeY - A->SizeX * A->SizeY; });

void UShadowMap2D::FinishEncoding()
{
	const UINT PackedLightAndShadowMapTextureSize = GWorld->GetWorldInfo()->PackedLightAndShadowMapTextureSize;

	// Reset the pending shadow-map size.
	PendingShadowMapSize = 0;

	// Allocate texture space for each light-map.
	TIndirectArray<FShadowMapPendingTexture> PendingTextures;
	for(INT ShadowMapIndex = 0;ShadowMapIndex < PendingShadowMaps.Num();ShadowMapIndex++)
	{
		FShadowMapAllocation& Allocation = PendingShadowMaps(ShadowMapIndex);

		// 4-byte alignment, no more padding
		UINT PaddingBaseX = 0;
		UINT PaddingBaseY = 0;

		// Find an existing texture which the light-map can be stored in.
		FShadowMapPendingTexture* Texture = NULL;
		for(INT TextureIndex = 0;TextureIndex < PendingTextures.Num();TextureIndex++)
		{
			FShadowMapPendingTexture& ExistingTexture = PendingTextures(TextureIndex);
			if(	ExistingTexture.Outer == Allocation.TextureOuter )
			{
				if(ExistingTexture.AddElement(&PaddingBaseX,&PaddingBaseY,Allocation.SizeX,Allocation.SizeY))
				{
					Texture = &ExistingTexture;
					break;
				}
			}
		}

		// If there is no appropriate texture, create a new one.
		if(!Texture)
		{
			UINT NewTextureSizeX = PackedLightAndShadowMapTextureSize;
			UINT NewTextureSizeY = PackedLightAndShadowMapTextureSize;
			if(Allocation.SizeX > NewTextureSizeX || Allocation.SizeY > NewTextureSizeY)
			{
				NewTextureSizeX = appRoundUpToPowerOfTwo(Allocation.SizeX);
				NewTextureSizeY = appRoundUpToPowerOfTwo(Allocation.SizeY);
			}

			// If there is no existing appropriate texture, create a new one.
			Texture = ::new(PendingTextures) FShadowMapPendingTexture(NewTextureSizeX,NewTextureSizeY);
			Texture->Material = Allocation.Material;
			Texture->LightGuid = Allocation.LightGuid;
			Texture->Outer = Allocation.TextureOuter;
			verify(Texture->AddElement(&PaddingBaseX,&PaddingBaseY,Allocation.SizeX,Allocation.SizeY));
		}

		// Position the shadow-maps in the middle of their padded space.
		Allocation.BaseX = PaddingBaseX;
		Allocation.BaseY = PaddingBaseY;

		// Add the shadow-map to the list of shadow-maps allocated space in this texture.
		Texture->Allocations.AddItem(&Allocation);
	}

	GWarn->BeginSlowTask(TEXT("Encoding shadow-maps"),FALSE);

	for(INT TextureIndex = 0;TextureIndex < PendingTextures.Num();TextureIndex++)
	{
		GWarn->StatusUpdatef(TextureIndex,PendingTextures.Num(),LocalizeSecure(LocalizeUnrealEd(TEXT("EncodingShadowMapsF")),TextureIndex,PendingTextures.Num()));

		FShadowMapPendingTexture& PendingTexture = PendingTextures(TextureIndex);

		// Create the shadow-map texture.
		UShadowMapTexture2D* Texture = new(PendingTexture.Outer) UShadowMapTexture2D;
		Texture->SizeX		= PendingTexture.GetSizeX();
		Texture->SizeY		= PendingTexture.GetSizeY();
		Texture->Format		= PF_G8;
		Texture->LODGroup	= TEXTUREGROUP_LightAndShadowMap;

		// Create the uncompressed top mip-level.
		TArray< TArray<FQuantizedShadowSample> > MipData;
		TArray<FQuantizedShadowSample>* TopMipData = new(MipData) TArray<FQuantizedShadowSample>();
		TopMipData->Empty(PendingTexture.GetSizeX() * PendingTexture.GetSizeY());
		TopMipData->AddZeroed(PendingTexture.GetSizeX() * PendingTexture.GetSizeY());
		for(INT AllocationIndex = 0;AllocationIndex < PendingTexture.Allocations.Num();AllocationIndex++)
		{
			FShadowMapAllocation& Allocation = *PendingTexture.Allocations(AllocationIndex);

			// Copy the raw data for this light-map into the raw texture data array.
			for(UINT Y = 0;Y < Allocation.SizeY;Y++)
			{
				for(UINT X = 0;X < Allocation.SizeX;X++)
				{
					FQuantizedShadowSample& DestSample = (*TopMipData)((Y + Allocation.BaseY) * Texture->SizeX + X + Allocation.BaseX);
					const FQuantizedShadowSample& SourceSample = Allocation.RawData(Y * Allocation.SizeX + X);
					DestSample = SourceSample;
				}
			}

			// Link the shadow-map to the texture.
			Allocation.ShadowMap->Texture = Texture;

			// Free the shadow-map's raw data.
			Allocation.RawData.Empty();
			
			// Calculate the coordinate scale/biases for each shadow-map stored in the texture.
			Allocation.ShadowMap->CoordinateScale = FVector2D(
				(FLOAT)Allocation.SizeX / (FLOAT)PendingTexture.GetSizeX(),
				(FLOAT)Allocation.SizeY / (FLOAT)PendingTexture.GetSizeY()
				);
			Allocation.ShadowMap->CoordinateBias = FVector2D(
				(FLOAT)(Allocation.BaseX + 0.5f) / (FLOAT)PendingTexture.GetSizeX(),
				(FLOAT)(Allocation.BaseY + 0.5f) / (FLOAT)PendingTexture.GetSizeY()
				);
		}

		UINT NumMips = Max(appCeilLogTwo(Texture->SizeX),appCeilLogTwo(Texture->SizeY)) + 1;
		for(UINT MipIndex = 1;MipIndex < NumMips;MipIndex++)
		{
			UINT SourceMipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> (MipIndex - 1));
			UINT SourceMipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> (MipIndex - 1));
			UINT DestMipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
			UINT DestMipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);

			// Downsample the previous mip-level, taking into account which texels are mapped.
			TArray<FQuantizedShadowSample>* NextMipData = new(MipData) TArray<FQuantizedShadowSample>();
			NextMipData->Empty(DestMipSizeX * DestMipSizeY);
			NextMipData->AddZeroed(DestMipSizeX * DestMipSizeY);
			UINT MipFactorX = SourceMipSizeX / DestMipSizeX;
			UINT MipFactorY = SourceMipSizeY / DestMipSizeY;
			for(UINT Y = 0;Y < DestMipSizeY;Y++)
			{
				for(UINT X = 0;X < DestMipSizeX;X++)
				{
					UINT AccumulatedVisibility = 0;
					UINT Coverage = 0;
					for(UINT SourceY = Y * MipFactorY;SourceY < (Y + 1) * MipFactorY;SourceY++)
					{
						for(UINT SourceX = X * MipFactorX;SourceX < (X + 1) * MipFactorX;SourceX++)
						{
							const FQuantizedShadowSample& SourceSample = MipData(MipIndex - 1)(SourceY * SourceMipSizeX + SourceX);
							if(SourceSample.Coverage)
							{
								AccumulatedVisibility += SourceSample.Visibility * SourceSample.Coverage;
								Coverage += SourceSample.Coverage;
							}
						}
					}
					FQuantizedShadowSample& DestSample = (*NextMipData)(Y * DestMipSizeX + X);
					if(Coverage)
					{
						DestSample.Visibility = (BYTE)appTrunc((FLOAT)AccumulatedVisibility / (FLOAT)Coverage);
						DestSample.Coverage = (BYTE)(Coverage / (MipFactorX * MipFactorY));
					}
					else
					{
						DestSample.Visibility = DestSample.Coverage = 0;
					}
				}
			}
		}

		// Expand texels which are mapped into adjacent texels which are not mapped to avoid artifacts when using texture filtering.
		for(INT MipIndex = 0;MipIndex < MipData.Num();MipIndex++)
		{
			UINT MipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
			UINT MipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);
			for(UINT DestY = 0;DestY < MipSizeY;DestY++)
			{
				for(UINT DestX = 0;DestX < MipSizeX;DestX++)
				{
					UINT AccumulatedVisibility = 0;
					UINT Coverage = 0;
					for(INT SourceY = (INT)DestY - 1;SourceY <= (INT)DestY + 1;SourceY++)
					{
						if(SourceY >= 0 && SourceY < (INT)MipSizeY)
						{
							for(INT SourceX = (INT)DestX - 1;SourceX <= (INT)DestX + 1;SourceX++)
							{
								if(SourceX >= 0 && SourceX < (INT)MipSizeX)
								{
									const FQuantizedShadowSample& SourceSample = MipData(MipIndex)(SourceY * MipSizeX + SourceX);
									if(SourceSample.Coverage)
									{
										static const UINT Weights[3][3] =
										{
											{ 1, 255, 1 },
											{ 255, 0, 255 },
											{ 1, 255, 1 },
										};
										AccumulatedVisibility += SourceSample.Visibility * SourceSample.Coverage * Weights[SourceX - DestX + 1][SourceY - DestY + 1];
										Coverage += SourceSample.Coverage * Weights[SourceX - DestX + 1][SourceY - DestY + 1];
									}
								}
							}
						}
					}

					FQuantizedShadowSample& DestSample = MipData(MipIndex)(DestY * MipSizeX + DestX);
					if(DestSample.Coverage == 0 && Coverage)
					{
						DestSample.Visibility = (BYTE)appTrunc((FLOAT)AccumulatedVisibility / (FLOAT)Coverage);
					}
				}
			}
		}

		// Copy the mip-map data into the UShadowMapTexture2D's mip-map array.
		for(INT MipIndex = 0;MipIndex < MipData.Num();MipIndex++)
		{
			UINT MipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
			UINT MipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);

			// Copy this mip-level into the texture's mips array.
			FTexture2DMipMap* MipMap = new(Texture->Mips) FTexture2DMipMap;
			MipMap->SizeX = MipSizeX;
			MipMap->SizeY = MipSizeY;
			MipMap->Data.Lock( LOCK_READ_WRITE );
			BYTE* DestMipData = (BYTE*) MipMap->Data.Realloc( MipSizeX * MipSizeY );
			for(UINT Y = 0;Y < MipSizeY;Y++)
			{
				for(UINT X = 0;X < MipSizeX;X++)
				{
					const FQuantizedShadowSample& SourceSample = MipData(MipIndex)(Y * MipSizeX + X);
					DestMipData[ Y * MipSizeX + X ] = SourceSample.Visibility;
				}
			}
			MipMap->Data.Unlock();
		}

		// Update the texture resource.
		Texture->UpdateResource();
	}

	PendingTextures.Empty();
	PendingShadowMaps.Empty();

	GWarn->EndSlowTask();
}

UShadowMap1D::UShadowMap1D(const FGuid& InLightGuid,const FShadowMapData1D& Data):
	LightGuid(InLightGuid)
{
	// Copy the shadow occlusion samples.
	Samples.Empty(Data.GetSize());
	for(INT SampleIndex = 0;SampleIndex < Data.GetSize();SampleIndex++)
	{
		Samples.AddItem(Data(SampleIndex));
	}

	// Initialize the vertex buffer.
	BeginInitResource(this);
}

void UShadowMap1D::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	Ar << Samples;
	Ar << LightGuid;
}

void UShadowMap1D::PostLoad()
{
	Super::PostLoad();
	if(!HasAnyFlags(RF_ClassDefaultObject))
	{
		// Initialize the vertex buffer.
		BeginInitResource(this);
	}
}

void UShadowMap1D::BeginDestroy()
{
	Super::BeginDestroy();
	BeginReleaseResource(this);
	ReleaseFence.BeginFence();
}

UBOOL UShadowMap1D::IsReadyForFinishDestroy()
{
	return Super::IsReadyForFinishDestroy() && ReleaseFence.GetNumPendingFences() == 0;
}

void UShadowMap1D::InitRHI()
{
	// Compute the vertex buffer size.
	if( Samples.GetResourceDataSize() > 0 )
	{
		// Create the light-map vertex buffer.
		VertexBufferRHI = RHICreateVertexBuffer(Samples.GetResourceDataSize(),&Samples,RUF_Static);
	}
}
