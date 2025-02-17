/*=============================================================================
	UnLightMap.cpp: Light-map implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#define COMPRESS_LIGHTMAPS 1

/** Fudge factor to roughly match up lightmaps and regular lighting. */
#define LIGHTMAP_COLOR_FUDGE_FACTOR 1.5f
/** Maximum light intensity stored in vertex/ texture lightmaps. */
#define MAX_LIGHT_INTENSITY	16.f

#include "UnTextureLayout.h"

IMPLEMENT_CLASS(ULightMapTexture2D);

/** 
 * The three orthonormal bases used with directional static lighting, and then the surface normal in tangent space.
 */
static FVector LightMapBasis[NUM_GATHERED_LIGHTMAP_COEF] =
{
	FVector(	0.0f,						appSqrt(6.0f) / 3.0f,			1.0f / appSqrt(3.0f)),
	FVector(	-1.0f / appSqrt(2.0f),		-1.0f / appSqrt(6.0f),			1.0f / appSqrt(3.0f)),
	FVector(	+1.0f / appSqrt(2.0f),		-1.0f / appSqrt(6.0f),			1.0f / appSqrt(3.0f)),
	FVector(	0.0f,						0.0f,							1.0f)
};

FLightSample FLightSample::PointLight(const FLinearColor& Color,const FVector& Direction)
{
	FLightSample Result;
	FLinearColor FudgedColor = Color * LIGHTMAP_COLOR_FUDGE_FACTOR;
	for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		const FLOAT CoefficientScale = Max(0.0f,Direction | LightMapBasis[CoefficientIndex]);
		Result.Coefficients[CoefficientIndex][0] = FudgedColor.R * CoefficientScale;
		Result.Coefficients[CoefficientIndex][1] = FudgedColor.G * CoefficientScale;
		Result.Coefficients[CoefficientIndex][2] = FudgedColor.B * CoefficientScale;
	}
	return Result;
}

FLightSample FLightSample::SkyLight(const FLinearColor& UpperColor,const FLinearColor& LowerColor,const FVector& WorldZ)
{
	FLightSample Result;
	for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		const FLOAT Dot = WorldZ | LightMapBasis[CoefficientIndex];
		const FLOAT UpperCoefficientScale = Square(0.5f + 0.5f * +Dot);
		const FLOAT LowerCoefficientScale = Square(0.5f + 0.5f * -Dot);
		Result.Coefficients[CoefficientIndex][0] = UpperColor.R * UpperCoefficientScale + LowerColor.R * LowerCoefficientScale;
		Result.Coefficients[CoefficientIndex][1] = UpperColor.G * UpperCoefficientScale + LowerColor.G * LowerCoefficientScale;
		Result.Coefficients[CoefficientIndex][2] = UpperColor.B * UpperCoefficientScale + LowerColor.B * LowerCoefficientScale;
	}
	return Result;
}

void FLightSample::AddWeighted(const FLightSample& OtherSample,FLOAT Weight)
{
	for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			Coefficients[CoefficientIndex][ColorIndex] = Coefficients[CoefficientIndex][ColorIndex] + OtherSample.Coefficients[CoefficientIndex][ColorIndex] * Weight;
		}
	}
}

void FLightMap::Serialize(FArchive& Ar)
{
	Ar << LightGuids;
}

void FLightMap::FinishCleanup()
{
	delete this;
}

void ULightMapTexture2D::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

/** 
 * Returns a one line description of an object for viewing in the generic browser
 */
FString ULightMapTexture2D::GetDesc()
{
	return FString::Printf( TEXT("Lightmap: %dx%d [%s]"), SizeX, SizeY, GPixelFormats[Format].Name );
}

/** 
 * Returns detailed info to populate listview columns
 */
FString ULightMapTexture2D::GetDetailedDescription( INT InIndex )
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

/**
 * The quantized coefficients for a single light-map texel.
 */
struct FLightMapCoefficients
{
	BYTE Coverage;
	BYTE Coefficients[NUM_GATHERED_LIGHTMAP_COEF][3];
};

/**
 * An allocation of a region of light-map texture to a specific light-map.
 */

#if _MSC_VER && !CONSOLE
struct FLightMapAllocation
{
	FLightMap2D* LightMap;
	UObject* Outer;
	UINT BaseX;
	UINT BaseY;
	UINT SizeX;
	UINT SizeY;
	TArray<FLightMapCoefficients> RawData;
	FLOAT Scale[NUM_GATHERED_LIGHTMAP_COEF][3];
	UMaterialInterface* Material;
};

/** 
 * Compresses SourceData using DXT and adds the compressed data to Texture2D->Mips.
 * DXTCompress will first try to use Nvidia Texture Tools 2 with CUDA acceleration.  If that fails, it will disable CUDA and try again.  
 * If compression still fails, DXTCompress will fall back to the old nvDXT compressor.  
 *
 * @param Texture2D - The texture whose Mips array should be augmented with the compressed data
 * @param SourceData - Input data to compress, in BGRA 8bit per channel unsigned format.
 * @param PixelFormat - Output format, must be one of: PF_DXT1, PF_DXT3 or PF_A8R8G8B8
 * @param SizeX - Dimensions of SourceData
 * @param SizeY - Dimensions of SourceData
 * @param SRGB - Whether the input data is in gamma space and needs to be converted to linear before operations
 * @param bIsNormalMap - Whether the input data contains normals instead of color.  This will force the use of the old nvDXT compressor to maintain quality.
 */
extern void DXTCompress(UTexture2D* Texture2D, BYTE* SourceData, EPixelFormat PixelFormat, INT SizeX, INT SizeY, UBOOL SRGB, UBOOL bIsNormalMap);

/**
 * A light-map texture which has been partially allocated, but not yet encoded.
 */
struct FLightMapPendingTexture: FTextureLayout
{
	TArray<FLightMapAllocation*> Allocations;
	UMaterialInterface* Material;
	UObject* Outer;

	FLightMapPendingTexture(UINT InSizeX,UINT InSizeY):
#if COMPRESS_LIGHTMAPS
		FTextureLayout(GPixelFormats[PF_DXT1].BlockSizeX,GPixelFormats[PF_DXT1].BlockSizeY,InSizeX,InSizeY,true)
#else
		FTextureLayout(GPixelFormats[PF_A8R8G8B8].BlockSizeX,GPixelFormats[PF_A8R8G8B8].BlockSizeY,InSizeX,InSizeY,true)
#endif
	{}

	void Encode()
	{
		// Encode and compress the coefficient textures.
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
		{
			// Create the light-map texture for this coefficient.
			ULightMapTexture2D* Texture = new(Outer) ULightMapTexture2D;
			Texture->SizeX		= GetSizeX();
			Texture->SizeY		= GetSizeY();
#if COMPRESS_LIGHTMAPS
			Texture->Format		= PF_DXT1;
#else
			Texture->Format		= PF_A8R8G8B8;
#endif
			Texture->LODGroup	= TEXTUREGROUP_LightAndShadowMap;

			// Create the uncompressed top mip-level.
			TArray< TArray<FColor> > MipData;
			TArray<FColor>* TopMipData = new(MipData) TArray<FColor>();
			TopMipData->Empty(GetSizeX() * GetSizeY());
			TopMipData->AddZeroed(GetSizeX() * GetSizeY());
			for(INT AllocationIndex = 0;AllocationIndex < Allocations.Num();AllocationIndex++)
			{
				FLightMapAllocation* Allocation = Allocations(AllocationIndex);

				// Link the light-map to the texture.
				Allocation->LightMap->Textures[CoefficientIndex] = Texture;
				Allocation->LightMap->ScaleVectors[CoefficientIndex] = FVector4(
					Allocation->Scale[CoefficientIndex][0],
					Allocation->Scale[CoefficientIndex][1],
					Allocation->Scale[CoefficientIndex][2],
					1.0f
					);

				// Copy the raw data for this light-map into the raw texture data array.
				for(UINT Y = 0;Y < Allocation->SizeY;Y++)
				{
					for(UINT X = 0;X < Allocation->SizeX;X++)
					{
						FColor& DestColor = (*TopMipData)((Y + Allocation->BaseY) * Texture->SizeX + X + Allocation->BaseX);
						const FLightMapCoefficients& SourceCoefficients = Allocation->RawData(Y * Allocation->SizeX + X);
#if VISUALIZE_PACKING						
						if( X == 0 || Y == 0 || X==Allocation->SizeX-1 || Y==Allocation->SizeY-1 
						||	X == 1 || Y == 1 || X==Allocation->SizeX-2 || Y==Allocation->SizeY-2 )
						{
							DestColor = FColor(255,0,0);
						}
						else
						{
							DestColor = FColor(0,255,0);
						}
#else
						DestColor.R = SourceCoefficients.Coefficients[CoefficientIndex][0];
						DestColor.G = SourceCoefficients.Coefficients[CoefficientIndex][1];
						DestColor.B = SourceCoefficients.Coefficients[CoefficientIndex][2];
						DestColor.A = SourceCoefficients.Coverage;
#endif
					}
				}
			}

#if COMPRESS_LIGHTMAPS
			UINT NumMips = Max(appCeilLogTwo(Texture->SizeX),appCeilLogTwo(Texture->SizeY)) + 1;
#else	//#if COMPRESS_LIGHTMAPS
			//@todo. Fix this case. When not compressing light maps, there is no lower mip data!
			// This is due to the compression code actually generating the mips.
			UINT NumMips = 1;
#endif	//#if COMPRESS_LIGHTMAPS
			for(UINT MipIndex = 1;MipIndex < NumMips;MipIndex++)
			{
				UINT SourceMipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> (MipIndex - 1));
				UINT SourceMipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> (MipIndex - 1));
				UINT DestMipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
				UINT DestMipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);

				// Downsample the previous mip-level, taking into account which texels are mapped.
				TArray<FColor>* NextMipData = new(MipData) TArray<FColor>();
				NextMipData->Empty(DestMipSizeX * DestMipSizeY);
				NextMipData->AddZeroed(DestMipSizeX * DestMipSizeY);
				UINT MipFactorX = SourceMipSizeX / DestMipSizeX;
				UINT MipFactorY = SourceMipSizeY / DestMipSizeY;
				for(UINT Y = 0;Y < DestMipSizeY;Y++)
				{
					for(UINT X = 0;X < DestMipSizeX;X++)
					{
						FLinearColor AccumulatedColor = FLinearColor::Black;
						UINT Coverage = 0;
						for(UINT SourceY = Y * MipFactorY;SourceY < (Y + 1) * MipFactorY;SourceY++)
						{
							for(UINT SourceX = X * MipFactorX;SourceX < (X + 1) * MipFactorX;SourceX++)
							{
								FColor& SourceColor = MipData(MipIndex - 1)(SourceY * SourceMipSizeX + SourceX);
								if(SourceColor.A)
								{
									AccumulatedColor += FLinearColor(SourceColor) * SourceColor.A;
									Coverage += SourceColor.A;
								}
							}
						}
						if(Coverage)
						{
							FColor AverageColor(AccumulatedColor / Coverage);
							(*NextMipData)(Y * DestMipSizeX + X) = FColor(AverageColor.R,AverageColor.G,AverageColor.B,Coverage / (MipFactorX * MipFactorY));
						}
						else
						{
							(*NextMipData)(Y * DestMipSizeX + X) = FColor(0,0,0,0);
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
						FLinearColor AccumulatedColor = FLinearColor::Black;
						UINT Coverage = 0;
						for(INT SourceY = (INT)DestY - 1;SourceY <= (INT)DestY + 1;SourceY++)
						{
							if(SourceY >= 0 && SourceY < (INT)MipSizeY)
							{
								for(INT SourceX = (INT)DestX - 1;SourceX <= (INT)DestX + 1;SourceX++)
								{
									if(SourceX >= 0 && SourceX < (INT)MipSizeX)
									{
										FColor& SourceColor = MipData(MipIndex)(SourceY * MipSizeX + SourceX);
										if(SourceColor.A)
										{
											static const UINT Weights[3][3] =
											{
												{ 1, 255, 1 },
												{ 255, 0, 255 },
												{ 1, 255, 1 },
											};
											AccumulatedColor += FLinearColor(SourceColor) * SourceColor.A * Weights[SourceX - DestX + 1][SourceY - DestY + 1];
											Coverage += SourceColor.A * Weights[SourceX - DestX + 1][SourceY - DestY + 1];
										}
									}
								}
							}
						}

						FColor& DestColor = MipData(MipIndex)(DestY * MipSizeX + DestX);
						if(DestColor.A == 0 && Coverage)
						{
							DestColor = FColor(AccumulatedColor / Coverage);
							DestColor.A = 0;
						}
					}
				}
			}

			for(INT MipIndex = 0;MipIndex < MipData.Num();MipIndex++)
			{
				UINT MipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
				UINT MipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);

				EPixelFormat PixelFormat = PF_DXT1;

#if !COMPRESS_LIGHTMAPS
				PixelFormat = PF_A8R8G8B8;
#endif

				DXTCompress(Texture, (BYTE*)&MipData(MipIndex)(0), PixelFormat, MipSizeX, MipSizeY, TRUE, FALSE);
			}

			// Update the texture resource.
			Texture->UpdateResource();
		}

		for(INT AllocationIndex = 0;AllocationIndex < Allocations.Num();AllocationIndex++)
		{
			FLightMapAllocation* Allocation = Allocations(AllocationIndex);

			// Calculate the coordinate scale/biases this light-map.
			Allocation->LightMap->CoordinateScale = FVector2D(
				(FLOAT)Allocation->SizeX / (FLOAT)GetSizeX(),
				(FLOAT)Allocation->SizeY / (FLOAT)GetSizeY()
				);
			Allocation->LightMap->CoordinateBias = FVector2D(
				(FLOAT)(Allocation->BaseX + 0.5f) / (FLOAT)GetSizeX(),
				(FLOAT)(Allocation->BaseY + 0.5f) / (FLOAT)GetSizeY()
				);

			// Free the light-map's raw data.
			Allocation->RawData.Empty();
		}
	}
};

/** The light-maps which have not yet been encoded into textures. */
static TIndirectArray<FLightMapAllocation> PendingLightMaps;
static UINT PendingLightMapSize = 0;
#endif

FLightMap2D* FLightMap2D::AllocateLightMap(UObject* LightMapOuter,FLightMapData2D* RawData,UMaterialInterface* Material, const FBoxSphereBounds& Bounds)
{
	// If the light-map has no lights in it, return NULL.
	if(!RawData || !RawData->Lights.Num())
	{
		return NULL;
	}

#if _MSC_VER && !CONSOLE
	FLightMapAllocation* Allocation = new(PendingLightMaps) FLightMapAllocation;
	Allocation->Material = Material;
	Allocation->Outer = LightMapOuter->GetOutermost();

	// Calculate the range of each coefficient in this light-map.
	FLOAT MaxCoefficient[NUM_GATHERED_LIGHTMAP_COEF][3];
	for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			MaxCoefficient[CoefficientIndex][ColorIndex] = 0;
		}
	}
	for(UINT Y = 0;Y < RawData->GetSizeY();Y++)
	{
		for(UINT X = 0;X < RawData->GetSizeX();X++)
		{
			const FLightSample& SourceSample = (*RawData)(X,Y);
			if(SourceSample.bIsMapped)
			{
				for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
				{
					for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
					{
						MaxCoefficient[CoefficientIndex][ColorIndex] = Clamp(
							SourceSample.Coefficients[CoefficientIndex][ColorIndex],
							MaxCoefficient[CoefficientIndex][ColorIndex],
							MAX_LIGHT_INTENSITY
							);
					}
				}
			}
		}
	}

	// Gather the GUIDs of lights stored in the light-map.
	TArray<FGuid> LightGuids;
	for(INT LightIndex = 0;LightIndex < RawData->Lights.Num();LightIndex++)
	{
		LightGuids.AddItem(RawData->Lights(LightIndex)->LightmapGuid);
	}

	// Create a new light-map.
	FLightMap2D* LightMap = new FLightMap2D(LightGuids);
	Allocation->LightMap = LightMap;

	// Calculate the scale/bias for the light-map coefficients.
	FLOAT InvCoefficientScale[NUM_GATHERED_LIGHTMAP_COEF][3];
	for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			Allocation->Scale[CoefficientIndex][ColorIndex] = MaxCoefficient[CoefficientIndex][ColorIndex];
			InvCoefficientScale[CoefficientIndex][ColorIndex] = 1.0f / Max<FLOAT>(MaxCoefficient[CoefficientIndex][ColorIndex],DELTA);
		}
	}

	// Quantize the coefficients for this texture.
	Allocation->SizeX = RawData->GetSizeX();
	Allocation->SizeY = RawData->GetSizeY();
	Allocation->RawData.Empty(RawData->GetSizeX() * RawData->GetSizeY());
	Allocation->RawData.Add(RawData->GetSizeX() * RawData->GetSizeY());
	for(UINT Y = 0;Y < RawData->GetSizeY();Y++)
	{
		for(UINT X = 0;X < RawData->GetSizeX();X++)
		{
			const FLightSample& SourceSample = (*RawData)(X,Y);
			FLightMapCoefficients& DestCoefficients = Allocation->RawData(Y * RawData->GetSizeX() + X);
			DestCoefficients.Coverage = SourceSample.bIsMapped ? 255 : 0;
			for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
			{
				for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
				{
					DestCoefficients.Coefficients[CoefficientIndex][ColorIndex] = (BYTE)Clamp<INT>(
						appTrunc(
							appPow(
								SourceSample.Coefficients[CoefficientIndex][ColorIndex] * InvCoefficientScale[CoefficientIndex][ColorIndex],
								1.0f / 2.2f
								) * 255.0f
							),
						0,
						255);
				}
			}
		}
	}

	// Track the size of pending light-maps.
	PendingLightMapSize += ((Allocation->SizeX + 3) & ~3) * ((Allocation->SizeY + 3) & ~3);

	// Once there are enough pending light-maps, flush encoding.
	const UINT PackedLightAndShadowMapTextureSize = GWorld->GetWorldInfo()->PackedLightAndShadowMapTextureSize;
	const UINT MaxPendingLightMapSize = Square(PackedLightAndShadowMapTextureSize) * 4;
	if(PendingLightMapSize >= MaxPendingLightMapSize)
	{
		FinishEncoding();
	}

	return LightMap;
#else
	return NULL;
#endif
}

#if _MSC_VER && !CONSOLE
IMPLEMENT_COMPARE_POINTER(FLightMapAllocation,UnLightMap,{ return Max(B->SizeX,B->SizeY) - Max(A->SizeX,A->SizeY); });
#endif

void FLightMap2D::FinishEncoding()
{
#if _MSC_VER && !CONSOLE
	GWarn->BeginSlowTask(TEXT("Encoding light-maps"),FALSE);

	UINT PackedLightAndShadowMapTextureSize = GWorld->GetWorldInfo()->PackedLightAndShadowMapTextureSize;

	// Reset the pending light-map size.
	PendingLightMapSize = 0;

	// Sort the light-maps in descending order by size.
	Sort<USE_COMPARE_POINTER(FLightMapAllocation,UnLightMap)>((FLightMapAllocation**)PendingLightMaps.GetData(),PendingLightMaps.Num());

	// Allocate texture space for each light-map.
	TIndirectArray<FLightMapPendingTexture> PendingTextures;

	for(INT LightMapIndex = 0;LightMapIndex < PendingLightMaps.Num();LightMapIndex++)
	{
		FLightMapAllocation& Allocation = PendingLightMaps(LightMapIndex);
		FLightMapPendingTexture* Texture = NULL;

		// Lightmaps will always be 4-pixel aligned...
		UINT PaddingBaseX = 0;
		UINT PaddingBaseY = 0;

		// Find an existing texture which the light-map can be stored in.
		for(INT TextureIndex = 0;TextureIndex < PendingTextures.Num();TextureIndex++)
		{
			FLightMapPendingTexture* ExistingTexture = &PendingTextures(TextureIndex);
			if( ExistingTexture->Outer == Allocation.Outer )
			{
				if(ExistingTexture->AddElement(&PaddingBaseX,&PaddingBaseY,Allocation.SizeX,Allocation.SizeY))
				{
					Texture = ExistingTexture;
					break;
				}
			}
		}

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
			Texture = new(PendingTextures) FLightMapPendingTexture(NewTextureSizeX,NewTextureSizeY);
			Texture->Material = Allocation.Material;
			Texture->Outer = Allocation.Outer;
			verify(Texture->AddElement(&PaddingBaseX,&PaddingBaseY,Allocation.SizeX,Allocation.SizeY));
		}

		// Position the light-maps in the middle of their padded space.
		Allocation.BaseX = PaddingBaseX;
		Allocation.BaseY = PaddingBaseY;

		Texture->Allocations.AddItem(&Allocation);
	}

	// Encode all the pending textures.
	for(INT TextureIndex = 0;TextureIndex < PendingTextures.Num();TextureIndex++)
	{
		GWarn->StatusUpdatef(TextureIndex,PendingTextures.Num(),LocalizeSecure(LocalizeUnrealEd(TEXT("EncodingLightMapsF")),TextureIndex,PendingTextures.Num()));
		PendingTextures(TextureIndex).Encode();
	}

	PendingTextures.Empty();
	PendingLightMaps.Empty();

	GWarn->EndSlowTask();
#endif
}

FLightMap2D::FLightMap2D(UBOOL InbAllowDirectionalLightMaps)
:	FLightMap(InbAllowDirectionalLightMaps)
{
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		Textures[CoefficientIndex] = NULL;
	}
}

FLightMap2D::FLightMap2D(const TArray<FGuid>& InLightGuids)
{
	LightGuids = InLightGuids;
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		Textures[CoefficientIndex] = NULL;
	}
}

const UTexture2D* FLightMap2D::GetTexture(UINT BasisIndex) const
{
	if (bAllowDirectionalLightMaps)
	{
		check(BasisIndex < SIMPLE_LIGHTMAP_COEF_INDEX);
	}
	else
	{
		check(BasisIndex >= SIMPLE_LIGHTMAP_COEF_INDEX && BasisIndex < NUM_GATHERED_LIGHTMAP_COEF);
	}
	
	check(Textures[BasisIndex]);
	return Textures[BasisIndex];
}

struct FLegacyLightMapTextureInfo
{
	ULightMapTexture2D* Texture;
	FLinearColor Scale;
	FLinearColor Bias;

	friend FArchive& operator<<(FArchive& Ar,FLegacyLightMapTextureInfo& I)
	{
		return Ar << I.Texture << I.Scale << I.Bias;
	}
};

void FLightMap2D::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		UObject::AddReferencedObject(ObjectArray,Textures[CoefficientIndex]);
	}
}

void FLightMap2D::Serialize(FArchive& Ar)
{
	FLightMap::Serialize(Ar);

	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		Ar << Textures[CoefficientIndex];

		//@warning This used to use the FVector serialization (wrong). Now it would have used
		//         the FVector4 serialization (correct), but this would cause backwards compability
		//         problems.
		//         We are now assuming that FVector and the first 12 bytes of FVector4 are binary compatible.
		Ar << (FVector&)ScaleVectors[CoefficientIndex];
	}

	Ar << CoordinateScale << CoordinateBias;

	//Release unneeded texture references on load so they will be garbage collected.  This will keep from cooking simple lightmaps.
	//In the editor we need to keep these references since they will need to be saved.
	if (Ar.IsLoading() && (!GIsEditor || (GCookingTarget & UE3::PLATFORM_Console)))
	{
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
		{
			if ((bAllowDirectionalLightMaps && (CoefficientIndex >= SIMPLE_LIGHTMAP_COEF_INDEX))
				|| (!bAllowDirectionalLightMaps && (CoefficientIndex < NUM_DIRECTIONAL_LIGHTMAP_COEF)))
			{
				Textures[CoefficientIndex] = NULL;
			}
		}

	}
}

FLightMapInteraction FLightMap2D::GetInteraction() const
{
	if (bAllowDirectionalLightMaps)
	{
		// When the FLightMap2D is first created, the textures aren't set, so that case needs to be handled.
		if(Textures[0] && Textures[1] && Textures[2])
		{
			return FLightMapInteraction::Texture(Textures,ScaleVectors,CoordinateScale,CoordinateBias,TRUE);
		}
	}
	else
	{
		if(Textures[SIMPLE_LIGHTMAP_COEF_INDEX])
		{
			return FLightMapInteraction::Texture(Textures,ScaleVectors,CoordinateScale,CoordinateBias,FALSE);
		}
	}
	return FLightMapInteraction::None();
}

FLightMap1D::FLightMap1D(UObject* InOwner,FLightMapData1D& Data):
	Owner(InOwner),
	CachedSampleDataSize(0),
	CachedSampleData(NULL)
{
	// Gather the GUIDs of lights stored in the light-map.
	for(INT LightIndex = 0;LightIndex < Data.Lights.Num();LightIndex++)
	{
		LightGuids.AddItem(Data.Lights(LightIndex)->LightmapGuid);
	}

	// Calculate the maximum light coefficient for each color component.
	FLOAT MaxCoefficient[NUM_GATHERED_LIGHTMAP_COEF][3];
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			MaxCoefficient[CoefficientIndex][ColorIndex] = 0;
		}
	}
	for(INT SampleIndex = 0;SampleIndex < Data.GetSize();SampleIndex++)
	{
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
		{
			for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
			{
				MaxCoefficient[CoefficientIndex][ColorIndex] = Clamp(
					Data(SampleIndex).Coefficients[CoefficientIndex][ColorIndex],
					MaxCoefficient[CoefficientIndex][ColorIndex],
					MAX_LIGHT_INTENSITY
					);
			}
		}
	}

	// Calculate the scale and inverse scale for the quantized coefficients.
	FLOAT InvScale[NUM_GATHERED_LIGHTMAP_COEF][3];
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			ScaleVectors[CoefficientIndex].Component(ColorIndex) = MaxCoefficient[CoefficientIndex][ColorIndex];
			InvScale[CoefficientIndex][ColorIndex] = 1.0f / Max(MaxCoefficient[CoefficientIndex][ColorIndex],DELTA);
		}
	}

	QuantizeBulkSamples(DirectionalSamples, Data, InvScale, NUM_DIRECTIONAL_LIGHTMAP_COEF, 0);
	QuantizeBulkSamples(SimpleSamples, Data, InvScale, NUM_SIMPLE_LIGHTMAP_COEF, SIMPLE_LIGHTMAP_COEF_INDEX);

	check( CachedSampleData == NULL && CachedSampleDataSize == 0 );

	InitResources();
}

/**
 * Scales, gamma corrects and quantizes the passed in samples and then stores them in BulkData.
 *
 * @param BulkData - The bulk data where the quantized samples are stored.
 * @param Data - The samples to process.
 * @param InvScale - 1 / max sample value.
 * @param NumCoefficients - Number of coefficients in the BulkData samples.
 * @param RelativeCoefficientOffset - Coefficient offset in the Data samples.
 */
template<class BulkDataType>
void FLightMap1D::QuantizeBulkSamples(
	BulkDataType& BulkData, 
	FLightMapData1D& Data, 
	const FLOAT InvScale[][3],
	UINT NumCoefficients, 
	UINT RelativeCoefficientOffset)
{
	// Rescale and quantize the light samples.
	BulkData.Lock( LOCK_READ_WRITE );
	typename BulkDataType::SampleType* SampleData = (typename BulkDataType::SampleType*) BulkData.Realloc( Data.GetSize() );
	for(INT SampleIndex = 0;SampleIndex < Data.GetSize();SampleIndex++)
	{
		typename BulkDataType::SampleType& DestSample = SampleData[SampleIndex];
		const FLightSample& SourceSample = Data(SampleIndex);
		for(UINT CoefficientIndex = 0;CoefficientIndex < NumCoefficients;CoefficientIndex++)
		{
			const UINT AbsoluteCoefficientIndex = CoefficientIndex + RelativeCoefficientOffset;
			DestSample.Coefficients[CoefficientIndex] = FColor(
				(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[AbsoluteCoefficientIndex][0] * InvScale[AbsoluteCoefficientIndex][0],1.0f / 2.2f) * 255.0f),0,255),
				(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[AbsoluteCoefficientIndex][1] * InvScale[AbsoluteCoefficientIndex][1],1.0f / 2.2f) * 255.0f),0,255),
				(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[AbsoluteCoefficientIndex][2] * InvScale[AbsoluteCoefficientIndex][2],1.0f / 2.2f) * 255.0f),0,255),
				0);
		}
	}
	BulkData.Unlock();
}

FLightMap1D::~FLightMap1D()
{
	const UINT BulkSampleSize = bAllowDirectionalLightMaps ? DirectionalSamples.GetBulkDataSize() : SimpleSamples.GetBulkDataSize();
	DEC_DWORD_STAT_BY( STAT_VertexLightingMemory, BulkSampleSize );
	CachedSampleDataSize = 0;
	if(CachedSampleData)
	{
		appFree(CachedSampleData);
		CachedSampleData = NULL;
	}
}

void FLightMap1D::Serialize(FArchive& Ar)
{
	FLightMap::Serialize(Ar);
	Ar << Owner;

	DirectionalSamples.Serialize( Ar, Owner );
	for (INT ElementIndex = 0; ElementIndex < NUM_GATHERED_LIGHTMAP_COEF; ElementIndex++)
	{
		Ar << ScaleVectors[ElementIndex].X;
		Ar << ScaleVectors[ElementIndex].Y;
		Ar << ScaleVectors[ElementIndex].Z;
	}

	SimpleSamples.Serialize( Ar, Owner );

	//remove the simple lighting bulk data when cooking for consoles.
	if (Ar.IsLoading() && (GCookingTarget & UE3::PLATFORM_Console))
	{
		SimpleSamples.RemoveBulkData();
	}
}

void FLightMap1D::InitResources()
{
	if(!CachedSampleData)
	{
		if (bAllowDirectionalLightMaps)
		{
			CachedSampleDataSize = DirectionalSamples.GetBulkDataSize();
			DirectionalSamples.GetCopy( &CachedSampleData, TRUE );
			// Remove unused simple samples outside the Editor.
			if( !GIsEditor )
			{
				SimpleSamples.RemoveBulkData();
			}
		}
		else
		{
			CachedSampleDataSize = SimpleSamples.GetBulkDataSize();
			SimpleSamples.GetCopy( &CachedSampleData, TRUE );
			// Remove unused directional samples outside the Editor.
			if( !GIsEditor )
			{
				DirectionalSamples.RemoveBulkData();
			}
		}
		INC_DWORD_STAT_BY( STAT_VertexLightingMemory, CachedSampleDataSize );
		
		// Only initialize resource if it has data to avoid 0 size assert.
		if( CachedSampleDataSize )
		{
			BeginInitResource(this);
		}
	}
}

void FLightMap1D::Cleanup()
{
	if ( IsInRenderingThread() )
	{
		ReleaseResource();
	}
	else
	{
		BeginReleaseResource(this);
	}
	FLightMap::Cleanup();
}

void FLightMap1D::InitRHI()
{
	// Create the light-map vertex buffer.
	VertexBufferRHI = RHICreateVertexBuffer(CachedSampleDataSize,NULL,RUF_Static);

	// Write the light-map data to the vertex buffer.
	void* Buffer = RHILockVertexBuffer(VertexBufferRHI,0,CachedSampleDataSize,FALSE);
	appMemcpy(Buffer,CachedSampleData,CachedSampleDataSize);
	appFree(CachedSampleData);
	CachedSampleData = NULL;
	RHIUnlockVertexBuffer(VertexBufferRHI);
}

/**
 * Creates a new lightmap that is a copy of this light map, but where the sample data
 * has been remapped according to the specified sample index remapping.
 *
 * @param		SampleRemapping		Sample remapping: Dst[i] = Src[SampleRemapping[i]].
 * @return							The new lightmap.
 */
FLightMap1D* FLightMap1D::DuplicateWithRemappedVerts(const TArray<INT>& SampleRemapping)
{
	check( SampleRemapping.Num() > 0 );

	// Create a new light map.
	FLightMap1D* NewLightMap = new FLightMap1D;

	// Copy over the owner and GUIDS.
	NewLightMap->LightGuids = LightGuids;
	NewLightMap->Owner = Owner;

	// Copy over coefficient scale vectors.
	for ( INT ElementIndex = 0 ; ElementIndex < NUM_GATHERED_LIGHTMAP_COEF ; ++ElementIndex )
	{
		NewLightMap->ScaleVectors[ElementIndex] = ScaleVectors[ElementIndex];
	}

	if ( GIsRHIInitialized )
	{
		check( NewLightMap->CachedSampleData == NULL && NewLightMap->CachedSampleDataSize == 0 );
		if (NewLightMap->bAllowDirectionalLightMaps)
		{
			CopyBulkSamples(DirectionalSamples, NewLightMap->DirectionalSamples, SampleRemapping, NUM_DIRECTIONAL_LIGHTMAP_COEF);
			NewLightMap->CachedSampleDataSize = NewLightMap->DirectionalSamples.GetBulkDataSize();
			NewLightMap->DirectionalSamples.GetCopy( &NewLightMap->CachedSampleData, TRUE );
		}
		else
		{
			CopyBulkSamples(SimpleSamples, NewLightMap->SimpleSamples, SampleRemapping, NUM_SIMPLE_LIGHTMAP_COEF);
			NewLightMap->CachedSampleDataSize = NewLightMap->SimpleSamples.GetBulkDataSize();
			NewLightMap->SimpleSamples.GetCopy( &NewLightMap->CachedSampleData, TRUE );
		}

		if ( IsInRenderingThread() )
		{
			NewLightMap->InitResource();
		}
		else
		{
			BeginInitResource( NewLightMap );
		}
	}
	else
	{
		// Access the raw sample data.
		FQuantizedSimpleLightSample* SourceSimpleSampleData = NULL;
		FQuantizedDirectionalLightSample* SourceDirectionalSampleData = NULL;
		if(bAllowDirectionalLightMaps)
		{
			if(CachedSampleData)
			{
				SourceDirectionalSampleData = (FQuantizedDirectionalLightSample*)CachedSampleData;
			}
			else
			{
				DirectionalSamples.GetCopy((void**)&SourceDirectionalSampleData,TRUE);
				CachedSampleData = SourceDirectionalSampleData;
			}
			if(SimpleSamples.IsBulkDataLoaded())
			{
				SimpleSamples.GetCopy((void**)&SourceSimpleSampleData,FALSE);
			}
		}
		else
		{
			if(CachedSampleData)
			{
				SourceSimpleSampleData = (FQuantizedSimpleLightSample*)CachedSampleData;
			}
			else
			{
				SimpleSamples.GetCopy((void**)&SourceSimpleSampleData,TRUE);
				CachedSampleData = SourceSimpleSampleData;
			}
			if(DirectionalSamples.IsBulkDataLoaded())
			{
				DirectionalSamples.GetCopy((void**)&SourceDirectionalSampleData,FALSE);
			}
		}

		// If the light-map has simple sample data, remap it.
		if(SourceSimpleSampleData)
		{
			NewLightMap->SimpleSamples.Lock( LOCK_READ_WRITE );
			FQuantizedSimpleLightSample* DestSimpleSampleData = (FQuantizedSimpleLightSample*) NewLightMap->SimpleSamples.Realloc( SampleRemapping.Num() );
			for(INT SampleIndex = 0;SampleIndex < SampleRemapping.Num();SampleIndex++)
			{
				DestSimpleSampleData[SampleIndex] = SourceSimpleSampleData[SampleRemapping(SampleIndex)];
			}
			NewLightMap->SimpleSamples.Unlock();
		}

		// If the light-map has directional sample data, remap it.
		if(SourceDirectionalSampleData)
		{
			NewLightMap->DirectionalSamples.Lock( LOCK_READ_WRITE );
			FQuantizedDirectionalLightSample* DestDirectionalSampleData = (FQuantizedDirectionalLightSample*) NewLightMap->DirectionalSamples.Realloc( SampleRemapping.Num() );
			for(INT SampleIndex = 0;SampleIndex < SampleRemapping.Num();SampleIndex++)
			{
				DestDirectionalSampleData[SampleIndex] = SourceDirectionalSampleData[SampleRemapping(SampleIndex)];
			}
			NewLightMap->DirectionalSamples.Unlock();
		}

		// Initialize the new light-maps resources.
		NewLightMap->InitResources();

		// Free the temporary copy of the source simple sample data.
		if(CachedSampleData != SourceSimpleSampleData)
		{
			appFree(SourceSimpleSampleData);
		}
		
		// Free the temporary copy of the source directional sample data.
		if(CachedSampleData != SourceDirectionalSampleData)
		{
			appFree(SourceDirectionalSampleData);
		}
	}

	return NewLightMap;
}

/**
 * Copies samples using the given remapping.
 *
 * @param SourceBulkData - The source samples
 * @param DestBulkData - The destination samples
 * @param SampleRemapping - Dst[i] = Src[SampleRemapping[i]].
 * @param NumCoefficients - Number of coefficients in the sample type.
 */
template<class BulkDataType>
void FLightMap1D::CopyBulkSamples(
	BulkDataType& SourceBulkData, 
	BulkDataType& DestBulkData, 
	const TArray<INT>& SampleRemapping,
	UINT NumCoefficients)
{
	// Copy over samples given the index remapping.
	typename BulkDataType::SampleType* SrcSimpleSampleData = 
		(typename BulkDataType::SampleType*)RHILockVertexBuffer(VertexBufferRHI,0,SourceBulkData.GetBulkDataSize(),TRUE);

	DestBulkData.Lock( LOCK_READ_WRITE );
	typename BulkDataType::SampleType* DstSimpleSampleData = (typename BulkDataType::SampleType*)DestBulkData.Realloc( SampleRemapping.Num() );

	for( INT SampleIndex = 0 ; SampleIndex < SampleRemapping.Num() ; ++SampleIndex )
	{
		const INT RemappedIndex = SampleRemapping(SampleIndex);
		checkSlow( RemappedIndex >= 0 && RemappedIndex < NumSamples() );
		const typename BulkDataType::SampleType& SrcSample = SrcSimpleSampleData[RemappedIndex];
		typename BulkDataType::SampleType& DstSample = DstSimpleSampleData[SampleIndex];
		for( UINT CoefficientIndex = 0 ; CoefficientIndex < NumCoefficients ; ++CoefficientIndex )
		{
			DstSample.Coefficients[CoefficientIndex] = SrcSample.Coefficients[CoefficientIndex];
		}
	}
	DestBulkData.Unlock();

	////// restore me
	RHIUnlockVertexBuffer(VertexBufferRHI);
}

/*-----------------------------------------------------------------------------
	FQuantizedLightSample version of bulk data.
-----------------------------------------------------------------------------*/

/**
 * Returns whether single element serialization is required given an archive. This e.g.
 * can be the case if the serialization for an element changes and the single element
 * serialization code handles backward compatibility.
 */
template<class QuantizedLightSampleType>
UBOOL TQuantizedLightSampleBulkData<QuantizedLightSampleType>::RequiresSingleElementSerialization( FArchive& Ar )
{
	return FALSE;
}

/**
 * Returns size in bytes of single element.
 *
 * @return Size in bytes of single element
 */
template<class QuantizedLightSampleType>
INT TQuantizedLightSampleBulkData<QuantizedLightSampleType>::GetElementSize() const
{
	return sizeof(QuantizedLightSampleType);
}

/**
 * Serializes an element at a time allowing and dealing with endian conversion and backward compatiblity.
 * 
 * @param Ar			Archive to serialize with
 * @param Data			Base pointer to data
 * @param ElementIndex	Element index to serialize
 */
template<class QuantizedLightSampleType>
void TQuantizedLightSampleBulkData<QuantizedLightSampleType>::SerializeElement( FArchive& Ar, void* Data, INT ElementIndex )
{
	QuantizedLightSampleType* QuantizedLightSample = (QuantizedLightSampleType*)Data + ElementIndex;
	// serialize as colors
	const UINT NumCoefficients = sizeof(QuantizedLightSampleType) / sizeof(FColor);
	for(INT CoefficientIndex = 0; CoefficientIndex < NumCoefficients; CoefficientIndex++)
	{
		DWORD ColorDWORD = QuantizedLightSample->Coefficients[CoefficientIndex].DWColor();
		Ar << ColorDWORD;
		QuantizedLightSample->Coefficients[CoefficientIndex] = FColor(ColorDWORD);
	} 
};

FArchive& operator<<(FArchive& Ar,FLightMap*& R)
{
	DWORD LightMapType = FLightMap::LMT_None;
	if(Ar.IsSaving())
	{
		if(R != NULL)
		{
			if(R->GetLightMap1D())
			{
				LightMapType = FLightMap::LMT_1D;
			}
			else if(R->GetLightMap2D())
			{
				LightMapType = FLightMap::LMT_2D;
			}
		}
	}
	Ar << LightMapType;

	if(Ar.IsLoading())
	{
		if(LightMapType == FLightMap::LMT_1D)
		{
			R = new FLightMap1D();
		}
		else if(LightMapType == FLightMap::LMT_2D)
		{
			R = new FLightMap2D();
		}
	}

	if(R != NULL)
	{
		R->Serialize(Ar);
	}

	return Ar;
}

FArchive& operator<<(FArchive& Ar,FLightMapSerializeHelper& R)
{
	if( Ar.IsLoading() )
	{
		DWORD LightMapType = FLightMap::LMT_None;
		Ar << LightMapType;
		if(LightMapType == FLightMap::LMT_1D)
		{
			R.LightMapRef = new FLightMap1D(R.bAllowDirectionalLightMaps && GSystemSettings.bAllowDirectionalLightMaps);
		}
		else if(LightMapType == FLightMap::LMT_2D)
		{
			R.LightMapRef = new FLightMap2D(R.bAllowDirectionalLightMaps && GSystemSettings.bAllowDirectionalLightMaps);
		}
		if( R.LightMapRef != NULL )
		{
			R.LightMapRef->Serialize(Ar);
		}
	}
	else
	{
		Ar << R.LightMapRef;
	}
	return Ar;
}

