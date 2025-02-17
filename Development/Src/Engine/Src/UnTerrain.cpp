/*=============================================================================
	UnTerrain.cpp: New terrain
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnTerrain.h"
#include "UnTerrainRender.h"
#include "UnCollision.h"
#include "EngineDecalClasses.h"
#include "UnDecalRenderData.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(ATerrain);
IMPLEMENT_CLASS(UTerrainComponent);
IMPLEMENT_CLASS(UTerrainMaterial);
IMPLEMENT_CLASS(UTerrainLayerSetup);

static FPatchSampler	GCollisionPatchSampler(TERRAIN_MAXTESSELATION);

//
//	PerlinNoise2D
//

FLOAT Fade(FLOAT T)
{
	return T * T * T * (T * (T * 6 - 15) + 10);
}

static int Permutations[256] =
{
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

FLOAT Grad(INT Hash,FLOAT X,FLOAT Y)
{
	INT		H = Hash & 15;
	FLOAT	U = H < 8 || H == 12 || H == 13 ? X : Y,
			V = H < 4 || H == 12 || H == 13 ? Y : 0;
	return ((H & 1) == 0 ? U : -U) + ((H & 2) == 0 ? V : -V);
}

FLOAT PerlinNoise2D(FLOAT X,FLOAT Y)
{
	INT		TruncX = appTrunc(X),
			TruncY = appTrunc(Y),
			IntX = TruncX & 255,
			IntY = TruncY & 255;
	FLOAT	FracX = X - TruncX,
			FracY = Y - TruncY;

	FLOAT	U = Fade(FracX),
			V = Fade(FracY);

	INT	A = Permutations[IntX] + IntY,
		AA = Permutations[A & 255],
		AB = Permutations[(A + 1) & 255],
		B = Permutations[(IntX + 1) & 255] + IntY,
		BA = Permutations[B & 255],
		BB = Permutations[(B + 1) & 255];

	return	Lerp(	Lerp(	Grad(Permutations[AA],			FracX,	FracY	),
							Grad(Permutations[BA],			FracX-1,FracY	),	U),
					Lerp(	Grad(Permutations[AB],			FracX,	FracY-1	),
							Grad(Permutations[BB],			FracX-1,FracY-1	),	U),	V);
}

//
//	FNoiseParameter::Sample
//

FLOAT FNoiseParameter::Sample(INT X,INT Y) const
{
	FLOAT	Noise = 0.0f;

	if(NoiseScale > DELTA)
	{
		for(UINT Octave = 0;Octave < 4;Octave++)
		{
			FLOAT	OctaveShift = 1 << Octave;
			FLOAT	OctaveScale = OctaveShift / NoiseScale;
			Noise += PerlinNoise2D(X * OctaveScale,Y * OctaveScale) / OctaveShift;
		}
	}

	return Base + Noise * NoiseAmount;
}

//
//	FNoiseParameter::TestGreater
//

UBOOL FNoiseParameter::TestGreater(INT X,INT Y,FLOAT TestValue) const
{
	FLOAT	ParameterValue = Base;

	if(NoiseScale > DELTA)
	{
		for(UINT Octave = 0;Octave < 4;Octave++)
		{
			FLOAT	OctaveShift = 1 << Octave;
			FLOAT	OctaveAmplitude = NoiseAmount / OctaveShift;

			// Attempt to avoid calculating noise if the test value is outside of the noise amplitude.

			if(TestValue > ParameterValue + OctaveAmplitude)
				return 1;
			else if(TestValue < ParameterValue - OctaveAmplitude)
				return 0;
			else
			{
				FLOAT	OctaveScale = OctaveShift / NoiseScale;
				ParameterValue += PerlinNoise2D(X * OctaveScale,Y * OctaveScale) * OctaveAmplitude;
			}
		}
	}

	return TestValue >= ParameterValue;
}

void ATerrain::Allocate()
{
	// wait until resources are released
	FlushRenderingCommands();

	check(MaxComponentSize > 0);
	check(NumPatchesX > 0);
	check(NumPatchesY > 0);

	INT	OldNumVerticesX = NumVerticesX,
		OldNumVerticesY = NumVerticesY;

	// Clamp the terrain size properties to valid values.
	NumPatchesX = Clamp(NumPatchesX,1,2048);
	NumPatchesY = Clamp(NumPatchesY,1,2048);
	// We need to clamp the patches to MaxDetessellationLevel
	if ((NumPatchesX % MaxTesselationLevel) > 0)
	{
		NumPatchesX += MaxTesselationLevel - (NumPatchesX % MaxTesselationLevel);
	}
	if ((NumPatchesY % MaxTesselationLevel) > 0)
	{
		NumPatchesY += MaxTesselationLevel - (NumPatchesY % MaxTesselationLevel);
	}

	// Calculate the new number of vertices in the terrain.
	NumVerticesX = NumPatchesX + 1;
	NumVerticesY = NumPatchesY + 1;

	// Initialize the terrain size.
	NumSectionsX = ((NumPatchesX / MaxTesselationLevel) + MaxComponentSize - 1) / MaxComponentSize;
	NumSectionsY = ((NumPatchesY / MaxTesselationLevel) + MaxComponentSize - 1) / MaxComponentSize;

#if defined(_TERRAIN_LOG_COMPONENTS_)
	debugf(TEXT("Terrain %16s --> %2dx%2d Components!"), *GetName(), NumSectionsX, NumSectionsY);
#endif	//#if defined(_TERRAIN_LOG_COMPONENTS_)

	if ((NumVerticesX != OldNumVerticesX) || (NumVerticesY != OldNumVerticesY))
	{
		INT TotalVertices = NumVerticesX * NumVerticesY;
		// Allocate the height-map.
		TArray<FTerrainHeight>	NewHeights;
		NewHeights.Empty(TotalVertices);

		// Allocate the info data
		TArray<FTerrainInfoData> NewInfoData;
		NewInfoData.Empty(TotalVertices);

		// Copy and/or initialize the values
		for(INT Y = 0;Y < NumVerticesY;Y++)
		{
			for(INT X = 0;X < NumVerticesX;X++)
			{
				if ((X < OldNumVerticesX) && (Y < OldNumVerticesY))
				{
					new(NewHeights) FTerrainHeight(Heights(Y * OldNumVerticesX + X).Value);
					new(NewInfoData) FTerrainInfoData(InfoData(Y * OldNumVerticesX + X).Data);
				}
				else
				{
					new(NewHeights) FTerrainHeight(32768);
					new(NewInfoData) FTerrainInfoData(0);
				}
			}
		}
		Heights.Empty(NewHeights.Num());
		Heights.Add(NewHeights.Num());
		appMemcpy(&Heights(0),&NewHeights(0),NewHeights.Num() * sizeof(FTerrainHeight));

		InfoData.Empty(NewInfoData.Num());
		InfoData.Add(NewInfoData.Num());
		appMemcpy(&InfoData(0), &NewInfoData(0), NewInfoData.Num() * sizeof(FTerrainInfoData));

		// Allocate the alpha-maps.
		for (INT AlphaMapIndex = 0; AlphaMapIndex < AlphaMaps.Num(); AlphaMapIndex++)
		{
			TArray<BYTE>	NewAlphas;
			NewAlphas.Empty(TotalVertices);
			for (INT Y = 0; Y < NumVerticesY; Y++)
			{
				for (INT X = 0; X < NumVerticesX; X++)
				{
					if ((X < OldNumVerticesX) && (Y < OldNumVerticesY))
					{
						new(NewAlphas) BYTE(AlphaMaps(AlphaMapIndex).Data(Y * OldNumVerticesX + X));
					}
					else
					{
						new(NewAlphas) BYTE(0);
					}
				}
			}
			AlphaMaps(AlphaMapIndex).Data.Empty(NewAlphas.Num());
			AlphaMaps(AlphaMapIndex).Data.Add(NewAlphas.Num());
			appMemcpy(&AlphaMaps(AlphaMapIndex).Data(0),&NewAlphas(0),NewAlphas.Num());
		}
	}
	RecreateComponents();
}

void ATerrain::RecreateComponents()
{
	// wait until resources are released
	FlushRenderingCommands();

	// Delete existing components.
	for(INT ComponentIndex = 0;ComponentIndex < TerrainComponents.Num();ComponentIndex++)
	{
		UTerrainComponent* Comp = TerrainComponents(ComponentIndex);
		if (Comp)
		{
			Comp->TermComponentRBPhys(NULL);
			Comp->ConditionalDetach();
		}
	}
	TerrainComponents.Empty(NumSectionsX * NumSectionsY);

	// Create components.
	for (INT Y = 0; Y < NumSectionsY; Y++)
	{
		for (INT X = 0; X < NumSectionsX; X++)
		{
			// The number of quads
			INT NumQuadsX = NumPatchesX / MaxTesselationLevel;
			INT NumQuadsY = NumPatchesY / MaxTesselationLevel;
			INT ComponentSizeX = Min((NumPatchesX / MaxTesselationLevel), MaxComponentSize);
			INT ComponentSizeY = Min((NumPatchesY / MaxTesselationLevel), MaxComponentSize);
			INT TrueSizeX = ComponentSizeX * MaxTesselationLevel;
			INT TrueSizeY = ComponentSizeY * MaxTesselationLevel;

			INT BaseX = X * TrueSizeX;
			INT BaseY = Y * TrueSizeY;
			INT SizeX = Min(NumQuadsX - X * MaxComponentSize,MaxComponentSize);
			INT SizeY = Min(NumQuadsY - Y * MaxComponentSize,MaxComponentSize);

			UTerrainComponent* TerrainComponent = ConstructObject<UTerrainComponent>(UTerrainComponent::StaticClass(),this,NAME_None,RF_Transactional);
			TerrainComponents.AddItem(TerrainComponent);
			TerrainComponent->Init(
					BaseX,BaseY,
					SizeX,
					SizeY,
					SizeX * MaxTesselationLevel,
					SizeY * MaxTesselationLevel
				);

			// Propagate shadow/ lighting options from ATerrain to component.
			TerrainComponent->CastShadow			= bCastShadow;
			TerrainComponent->bCastDynamicShadow	= bCastDynamicShadow;
			TerrainComponent->bForceDirectLightMap	= bForceDirectLightMap;
			TerrainComponent->BlockRigidBody		= bBlockRigidBody;
			TerrainComponent->bAcceptsDynamicLights	= bAcceptsDynamicLights;
			TerrainComponent->LightingChannels		= LightingChannels;
			TerrainComponent->PhysMaterialOverride	= TerrainPhysMaterialOverride;

			// Set the collision display options
			TerrainComponent->bDisplayCollisionLevel = bShowingCollision;

#if defined(_TERRAIN_LOG_COMPONENTS_)
			debugf(TEXT("    Terrain %16s: Component %2d - Loc = %2dx%2d, Size = %2dx%2d, TrueSize = %2dx%2d"), 
				*GetName(), TerrainComponents.Num(), 
				TerrainComponent->SectionBaseX, TerrainComponent->SectionBaseY, 
				TerrainComponent->SectionSizeX, TerrainComponent->SectionSizeY,
				TerrainComponent->TrueSectionSizeX, TerrainComponent->TrueSectionSizeY);
#endif	//#if defined(_TERRAIN_LOG_COMPONENTS_)
		}
	}
}

ATerrain* ATerrain::SplitTerrain( UBOOL SplitOnXAxis, INT RemainingPatches )
{
	FVector Offset;
	if( SplitOnXAxis )
	{
		if( RemainingPatches >= NumPatchesX || RemainingPatches <= 0 )
		{
			return NULL;
		}
		Offset = FVector((FLOAT)RemainingPatches*DrawScale*DrawScale3D.X, 0, 0);
	}
	else
	{
		if( RemainingPatches >= NumPatchesY || RemainingPatches <= 0 )
		{
			return NULL;
		}
		Offset = FVector(0, (FLOAT)RemainingPatches*DrawScale*DrawScale3D.Y, 0);
	}

	ATerrain* NewTerrain = Cast<ATerrain>(GWorld->SpawnActor(ATerrain::StaticClass(), NAME_None, Location + Offset, FRotator(0,0,0)));
	NewTerrain->MinTessellationLevel		= MinTessellationLevel;
	NewTerrain->MaxTesselationLevel			= MaxTesselationLevel;
	NewTerrain->DrawScale					= DrawScale;
	NewTerrain->DrawScale3D					= DrawScale3D;
	NewTerrain->CollisionTesselationLevel	= MaxTesselationLevel;
	NewTerrain->MaxComponentSize			= MaxComponentSize;
	NewTerrain->StaticLightingResolution	= StaticLightingResolution;
	NewTerrain->bIsOverridingLightResolution= bIsOverridingLightResolution;
	NewTerrain->bCastShadow					= bCastShadow;
	NewTerrain->bForceDirectLightMap		= bForceDirectLightMap;
	NewTerrain->bCastDynamicShadow			= bCastDynamicShadow;
	NewTerrain->bBlockRigidBody				= bBlockRigidBody;
	NewTerrain->bAcceptsDynamicLights		= bAcceptsDynamicLights;
	NewTerrain->bLocked						= bLocked;
	NewTerrain->bLockLocation				= bLockLocation;
	NewTerrain->TerrainPhysMaterialOverride	= TerrainPhysMaterialOverride;

	// copy layers
	for( INT i=0;i<Layers.Num();i++ )
	{
		new(NewTerrain->Layers) FTerrainLayer(Layers(i));
		NewTerrain->Layers(i).AlphaMapIndex = INDEX_NONE;
	}
	for( INT i=0;i<DecoLayers.Num();i++ )
	{
		new(NewTerrain->DecoLayers) FTerrainDecoLayer(DecoLayers(i));
		NewTerrain->DecoLayers(i).AlphaMapIndex = INDEX_NONE;
	}

	INT MinX, MinY, MaxX, MaxY;
	INT OffsetX, OffsetY;

	if( SplitOnXAxis )
	{
		NewTerrain->NumPatchesX = NumPatchesX - RemainingPatches;
		NewTerrain->NumPatchesY = NumPatchesY;

		MinX = RemainingPatches;
		MaxX = NumVerticesX;
		MinY = 0;
		MaxY = NumVerticesY;
		OffsetX = RemainingPatches;
		OffsetY = 0;
	}
	else
	{
		NewTerrain->NumPatchesX = NumPatchesX;
		NewTerrain->NumPatchesY = NumPatchesY - RemainingPatches;

		MinX = 0;
		MaxX = NumVerticesX;
		MinY = RemainingPatches;
		MaxY = NumVerticesY;
		OffsetX = 0;
		OffsetY = RemainingPatches;
	}

	// reallocate the height/alphamap/terraindata arrays 
	NewTerrain->Allocate();

	// Update heights
	for( INT y=MinY;y<MaxY;y++ )
	{
		for( INT x=MinX;x<MaxX;x++ )
		{
            NewTerrain->Height(x-OffsetX,y-OffsetY) = Height(x,y);
		}
	}

	// Update layers
	for( INT l=0;l<Layers.Num();l++ )
	{
		for( INT y=MinY;y<MaxY;y++ )
		{
			for( INT x=MinX;x<MaxX;x++ )
			{
				NewTerrain->Alpha(NewTerrain->Layers(l).AlphaMapIndex, x-OffsetX,y-OffsetY) = Alpha(Layers(l).AlphaMapIndex, x,y);
			}
		}
	}

	// Update decolayers
	for( INT l=0;l<DecoLayers.Num();l++ )
	{
		for( INT y=MinY;y<MaxY;y++ )
		{
			for( INT x=MinX;x<MaxX;x++ )
			{
				NewTerrain->Alpha(NewTerrain->DecoLayers(l).AlphaMapIndex, x-OffsetX,y-OffsetY) = Alpha(DecoLayers(l).AlphaMapIndex, x,y);
			}
		}
	}

	// Update data
	for( INT y=MinY;y<MaxY;y++ )
	{
		for( INT x=MinX;x<MaxX;x++ )
		{
            NewTerrain->GetInfoData(x-OffsetX,y-OffsetY)->Data = GetInfoData(x,y)->Data;
		}
	}

	// create components to match this new data
	NewTerrain->RecreateComponents();
	NewTerrain->UpdateRenderData(0,0,NewTerrain->NumPatchesX,NewTerrain->NumPatchesY);
	// to be uncommented.
	// NewTerrain->UpdateComponents();

	// shrink existing terrain
	if( SplitOnXAxis )
	{
        NumPatchesX = RemainingPatches;
		UProperty* Property = FindField<UProperty>(GetClass(), TEXT("NumPatchesX"));
		PreEditChange(Property);
		PostEditChange(Property);
	}
	else
	{
		NumPatchesY = RemainingPatches;
		UProperty* Property = FindField<UProperty>(GetClass(), TEXT("NumPatchesY"));
		PreEditChange(Property);
		PostEditChange(Property);
	}

	NewTerrain->UpdateComponentsInternal(TRUE);

	return NewTerrain;
}

void ATerrain::SplitTerrainPreview( FPrimitiveDrawInterface* PDI, UBOOL SplitOnXAxis, INT RemainingPatches )
{
	if( SplitOnXAxis ) 
	{
		FVector LastVertex = GetWorldVertex(RemainingPatches, 0);
		for( INT y=1;y<NumVerticesY;y++ )
		{
			FVector Vertex = GetWorldVertex(RemainingPatches, y);
			PDI->DrawLine(LastVertex,Vertex,FColor(255,255,0),SDPG_Foreground);
			LastVertex = Vertex;
		}
	}
	else
	{
		FVector LastVertex = GetWorldVertex(0, RemainingPatches);
		for( INT x=1;x<NumVerticesX;x++ )
		{
			FVector Vertex = GetWorldVertex(x,RemainingPatches);
			PDI->DrawLine(LastVertex,Vertex,FColor(255,255,0),SDPG_Foreground);
			LastVertex = Vertex;
		}
	}
}

UBOOL ATerrain::MergeTerrain( ATerrain* Other )
{
	if (Other && (Other != this) && 
		(Abs(Other->Location.Z-Location.Z) < KINDA_SMALL_NUMBER))	// Z values match
	{
		FVector	MyScale = DrawScale * DrawScale3D;
		FVector OtherScale = Other->DrawScale * Other->DrawScale3D;
		if ((OtherScale - MyScale).SizeSquared() < KINDA_SMALL_NUMBER)	// same scale
		{
			UBOOL Success = FALSE;

			UProperty* Property=NULL;
			INT OffsetX=0, OffsetY=0;
			INT MinX=0, MinY=0, MaxX=0, MaxY=0;

			// Other terrain is immediately right
			FVector	AdjustedLocation;
			FLOAT	X_Check;
			FLOAT	X_CheckA;
			FLOAT	Y_Check;
			FLOAT	Y_CheckA;

			AdjustedLocation.X = Location.X + (NumPatchesX * MyScale.X);
			AdjustedLocation.Y = Location.Y + (NumPatchesY * MyScale.Y);

			X_Check = Other->Location.X - Location.X;
			Y_Check = Other->Location.Y - Location.Y;
			X_CheckA = AdjustedLocation.X - Other->Location.X;
			Y_CheckA = AdjustedLocation.Y - Other->Location.Y;

			X_Check = Abs(X_Check);
			Y_Check = Abs(Y_Check);
			X_CheckA = Abs(X_CheckA);
			Y_CheckA = Abs(Y_CheckA);

			// Other terrain is immediately right
			if ((X_CheckA < KINDA_SMALL_NUMBER) &&
				(Y_Check  < KINDA_SMALL_NUMBER) &&
				(Other->NumPatchesY == NumPatchesY)
				)
			{
				Success = TRUE;
				OffsetX = NumPatchesX;
				OffsetY = 0;
				NumPatchesX += Other->NumPatchesX;
				Property = FindField<UProperty>(GetClass(), TEXT("NumPatchesX"));

				MinX = 1;
				MaxX = Other->NumVerticesX;
				MinY = 0;
				MaxY = Other->NumVerticesY;
			}
			else
			// Other terrain is immediately below
			if ((Y_CheckA < KINDA_SMALL_NUMBER) &&
				(X_Check  < KINDA_SMALL_NUMBER) &&
				(Other->NumPatchesX == NumPatchesX)
				)
			{
				Success = TRUE;
				OffsetX = 0;
				OffsetY = NumPatchesY;
				NumPatchesY += Other->NumPatchesY;
				Property = FindField<UProperty>(GetClass(), TEXT("NumPatchesY"));

				MinX = 0;
				MaxX = Other->NumVerticesX;
				MinY = 1;
				MaxY = Other->NumVerticesY;
			}

			if( Success )
			{
				// find matching layers
				UBOOL NeedToReallocateAlphamaps = FALSE;
				TArray<INT> LayerRemap;
				for( INT l=0;l<Other->Layers.Num();l++ )
				{
					UBOOL Found = FALSE;
					for( INT i=0;i<Layers.Num();i++ )
					{
						if( Layers(i).Setup == Other->Layers(l).Setup )
						{
							LayerRemap.AddItem(i);
							Found = TRUE;
							break;
						}
					}

					if( !Found )
					{
						INT NewLayer = Layers.AddZeroed();
						Layers(NewLayer).Setup = Other->Layers(l).Setup;
						Layers(NewLayer).AlphaMapIndex = -1;
						LayerRemap.AddItem( NewLayer );
						NeedToReallocateAlphamaps = TRUE;
					}
				}

				// find matching decolayers
				NeedToReallocateAlphamaps = FALSE;
				TArray<INT> DecoLayerRemap;
				for( INT l=0;l<Other->DecoLayers.Num();l++ )
				{
					UBOOL Found = FALSE;
					for( INT i=0;i<DecoLayers.Num();i++ )
					{
						if( DecoLayers(i) == Other->DecoLayers(l) )
						{
							DecoLayerRemap.AddItem(i);
							Found = TRUE;
							break;
						}
					}

					if( !Found )
					{
						INT NewDecoLayer = DecoLayers.AddZeroed();
						DecoLayers(NewDecoLayer) = Other->DecoLayers(l);
						DecoLayers(NewDecoLayer).AlphaMapIndex = -1;
						DecoLayerRemap.AddItem( NewDecoLayer );
						NeedToReallocateAlphamaps = TRUE;
					}
				}

				// reallocate the area
				PreEditChange(Property);
				PostEditChange(Property);

				// Update heights
				for( INT y=MinY;y<MaxY;y++ )
				{
					for( INT x=MinX;x<MaxX;x++ )
					{
						Height(x+OffsetX,y+OffsetY) = Other->Height(x,y);
					}
				}

				// Update data
				for( INT y=MinY;y<MaxY;y++ )
				{
					for( INT x=MinX;x<MaxX;x++ )
					{
						GetInfoData(x+OffsetX,y+OffsetY)->Data = Other->GetInfoData(x,y)->Data;
					}
				}			

				// Update layers
				for( INT l=0;l<Other->Layers.Num();l++ )
				{
					INT NewLayerNum = LayerRemap(l);

					for( INT y=MinY;y<MaxY;y++ )
					{
						for( INT x=MinX;x<MaxX;x++ )
						{
							Alpha(Layers(NewLayerNum).AlphaMapIndex, x+OffsetX,y+OffsetY) = Other->Alpha(Other->Layers(l).AlphaMapIndex, x,y);
						}
					}
				}

				// Update decolayers
				for( INT l=0;l<Other->DecoLayers.Num();l++ )
				{
					INT NewDecoLayerNum = DecoLayerRemap(l);

					for( INT y=MinY;y<MaxY;y++ )
					{
						for( INT x=MinX;x<MaxX;x++ )
						{
							Alpha(DecoLayers(NewDecoLayerNum).AlphaMapIndex, x+OffsetX,y+OffsetY) = Other->Alpha(Other->DecoLayers(l).AlphaMapIndex, x,y);
						}
					}
				}

				GWorld->EditorDestroyActor( Other, TRUE );

				// recreate components to match this new data
				RecreateComponents();
				UpdateRenderData(0,0,NumPatchesX,NumPatchesY);
				// to be uncommented
				// UpdateComponents();
				UpdateComponentsInternal(TRUE);

				return TRUE;
			}
		}
	}

	return FALSE;
}

UBOOL ATerrain::MergeTerrainPreview( class FPrimitiveDrawInterface* PDI, ATerrain* Other )
{
	if (Other && (Other != this) && 
		(Abs(Other->Location.Z-Location.Z) < KINDA_SMALL_NUMBER))	// Z values match
	{
		FVector	MyScale = DrawScale * DrawScale3D;
		FVector OtherScale = Other->DrawScale * Other->DrawScale3D;
		if ((OtherScale - MyScale).SizeSquared() < KINDA_SMALL_NUMBER)	// same scale
		{
			// Other terrain is immediately right
			FVector	AdjustedLocation;
			FLOAT	X_Check;
			FLOAT	X_CheckA;
			FLOAT	Y_Check;
			FLOAT	Y_CheckA;

			AdjustedLocation.X = Location.X + (NumPatchesX * MyScale.X);
			AdjustedLocation.Y = Location.Y + (NumPatchesY * MyScale.Y);

			X_Check = Other->Location.X - Location.X;
			Y_Check = Other->Location.Y - Location.Y;
			X_CheckA = AdjustedLocation.X - Other->Location.X;
			Y_CheckA = AdjustedLocation.Y - Other->Location.Y;

			X_Check = Abs(X_Check);
			Y_Check = Abs(Y_Check);
			X_CheckA = Abs(X_CheckA);
			Y_CheckA = Abs(Y_CheckA);

			// NOTE: This line was broken out into the above temps due to release builds
			// optimizing it such that the if(...) never passed.
//			if ((Abs(Location.X + (NumPatchesX * MyScale.X) - Other->Location.X) < KINDA_SMALL_NUMBER) && 
//				(Abs(Other->Location.Y - Location.Y) < KINDA_SMALL_NUMBER) && 
			if ((X_CheckA < KINDA_SMALL_NUMBER) &&
				(Y_Check  < KINDA_SMALL_NUMBER) &&
				(Other->NumPatchesY == NumPatchesY)
				)
			{
				if( PDI )
				{
					FVector LastVertex = GetWorldVertex(NumVerticesX-1, 0);
					for( INT y=1;y<=NumVerticesY;y++ )
					{
						// draw line into the first terrain
						FVector Vertex = GetWorldVertex(NumVerticesX-2, y-1);
						PDI->DrawLine(LastVertex,Vertex,FColor(255,255,0),SDPG_Foreground);

						// draw line into the 2nd terrain
						Vertex = Other->GetWorldVertex(1, y-1);
						PDI->DrawLine(LastVertex,Vertex,FColor(255,255,0),SDPG_Foreground);
						
						if( y<NumVerticesY )
						{
							// draw line along the edge
							Vertex = GetWorldVertex(NumVerticesX-1, y);
							PDI->DrawLine(LastVertex,Vertex,FColor(255,255,0),SDPG_Foreground);
							LastVertex = Vertex;
						}
					}
				}

				return TRUE;
			}
			else
			// Other terrain is immediately below
			// NOTE: This line was broken out into the above temps due to release builds
			// optimizing it such that the if(...) never passed.
//			if ((Abs(Location.Y + (NumPatchesY * MyScale.Y) - Other->Location.Y) < KINDA_SMALL_NUMBER) && 
//				(Abs(Other->Location.X - Location.X) < KINDA_SMALL_NUMBER) && 
			if ((Y_CheckA < KINDA_SMALL_NUMBER) &&
				(X_Check  < KINDA_SMALL_NUMBER) &&
				(Other->NumPatchesX == NumPatchesX)
				)
			{
				if( PDI )
				{
					FVector LastVertex = GetWorldVertex(0, NumVerticesY-1);
					for( INT x=1;x<=NumVerticesX;x++ )
					{
						// draw line into the first terrain
						FVector Vertex = GetWorldVertex(x-1, NumVerticesY-2);
						PDI->DrawLine(LastVertex,Vertex,FColor(255,255,0),SDPG_Foreground);

						// draw line into the 2nd terrain
						Vertex = Other->GetWorldVertex(x-1, 1);
						PDI->DrawLine(LastVertex,Vertex,FColor(255,255,0),SDPG_Foreground);
						
						if( x<NumVerticesX )
						{
							// draw line along the edge
							Vertex = GetWorldVertex(x, NumVerticesY-1);
							PDI->DrawLine(LastVertex,Vertex,FColor(255,255,0),SDPG_Foreground);
							LastVertex = Vertex;
						}
					}
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

/**
 *	Add or remove sectors to the terrain
 *
 *	@param	CountX		The number of sectors in the X-direction. If negative,
 *						they will go to the left, otherwise to the right.
 *	@param	CountY		The number of sectors in the Y-direction. If negative,
 *						they will go to the bottom, otherwise to the top.
 *	@param	bRemove		If TRUE, remove the sectors, otherwise add them.
 *
 *	@return	UBOOL		TRUE if successful.
 */
UBOOL ATerrain::AddRemoveSectors(INT CountX, INT CountY, UBOOL bRemove)
{
	if ((CountX == 0) && (CountY == 0))
	{
		return TRUE;
	}

	// We have to flush the rendering thread...
	FlushRenderingCommands();

	// Clear out the components
	ClearComponents();

	FString	Connector = bRemove ? FString(TEXT("from")) : FString(TEXT("to  "));
	debugf(TEXT("Terrain 0x%08x - %s %2d sectors %s the %s, %2d sectors %s the %s"),
		this, bRemove ? TEXT("Removing") : TEXT("Adding  "),
		Abs(CountY), *Connector, CountY < 0 ? TEXT("Left ") : TEXT("Right"),
		Abs(CountX), *Connector, CountX < 0 ? TEXT("Bottom") : TEXT("Top   ")
		);

	UBOOL bResultX;
	UBOOL bResultY;

	if (bRemove)
	{
		bResultX = RemoveSectors_X(CountX);
		bResultY = RemoveSectors_Y(CountY);
	}
	else
	{
		bResultX = AddSectors_X(CountX);
		bResultY = AddSectors_Y(CountY);
	}

	ClearWeightMaps();
	RecreateComponents();
	UpdateRenderData(0, 0, NumVerticesX - 1, NumVerticesY - 1);
	ConditionalUpdateComponents();

	return (bResultX & bResultY);
}

void ATerrain::StoreOldData(TArray<FTerrainHeight>& OldHeights, TArray<FTerrainInfoData>& OldInfoData, TArray<FAlphaMap>& OldAlphaMaps)
{
	OldHeights.Empty(Heights.Num());
	OldHeights.Add(Heights.Num());
	appMemcpy(&OldHeights(0), &Heights(0), Heights.Num() * sizeof(FTerrainHeight));
	OldInfoData.Empty(InfoData.Num());
	OldInfoData.Add(InfoData.Num());
	appMemcpy(&OldInfoData(0), &InfoData(0), InfoData.Num() * sizeof(FTerrainInfoData));
	OldAlphaMaps.Empty(AlphaMaps.Num());
	OldAlphaMaps.AddZeroed(AlphaMaps.Num());
	for (INT AlphaMapIndex = 0; AlphaMapIndex < AlphaMaps.Num(); AlphaMapIndex++)
	{
		FAlphaMap* AlphaMap = &AlphaMaps(AlphaMapIndex);
		FAlphaMap* OldAlphaMap = &OldAlphaMaps(AlphaMapIndex);

		OldAlphaMap->Data.Empty(AlphaMap->Data.Num());
		OldAlphaMap->Data.Add(AlphaMap->Data.Num());
		appMemcpy(&(OldAlphaMap->Data(0)), &(AlphaMap->Data(0)), AlphaMap->Data.Num());
	}
}

void ATerrain::SetupSizeData()
{
	// Reallocate - setup the new size-related information
	// Clamp the terrain size properties to valid values.
	NumPatchesX = Clamp(NumPatchesX,1,2048);
	NumPatchesY = Clamp(NumPatchesY,1,2048);
	// We need to clamp the patches to MaxDetessellationLevel
	if ((NumPatchesX % MaxTesselationLevel) > 0)
	{
		NumPatchesX += MaxTesselationLevel - (NumPatchesX % MaxTesselationLevel);
	}
	if ((NumPatchesY % MaxTesselationLevel) > 0)
	{
		NumPatchesY += MaxTesselationLevel - (NumPatchesY % MaxTesselationLevel);
	}

	// Calculate the new number of vertices in the terrain.
	NumVerticesX = NumPatchesX + 1;
	NumVerticesY = NumPatchesY + 1;

	// Initialize the terrain size.
	NumSectionsX = ((NumPatchesX / MaxTesselationLevel) + MaxComponentSize - 1) / MaxComponentSize;
	NumSectionsY = ((NumPatchesY / MaxTesselationLevel) + MaxComponentSize - 1) / MaxComponentSize;
}

UBOOL ATerrain::AddSectors_X(INT Count)
{
	if (Count == 0)
	{
		return TRUE;
	}

	INT	TopCount = 0;
	INT BottomCount = 0;
	if (Count > 0)
	{
		// We are adding patches to the 'top' of the terrain
		TopCount = Abs(Count) * MaxTesselationLevel;
	}
	else
	{
		// We are adding patches to the 'bottom' of the terrain
		BottomCount = Abs(Count) * MaxTesselationLevel;
	}

	// Store off the old data
    TArray<FTerrainHeight> OldHeights;
    TArray<FTerrainInfoData> OldInfoData;
    TArray<FAlphaMap> OldAlphaMaps;
	StoreOldData(OldHeights, OldInfoData, OldAlphaMaps);

	// Adjust the number of patches
	INT	OldNumVerticesX = NumVerticesX,
		OldNumVerticesY = NumVerticesY;

	INT OldNumPatchesX = NumPatchesX;
	NumPatchesX += TopCount + BottomCount;
	SetupSizeData();

	INT TotalVertices = NumVerticesX * NumVerticesY;
	// Allocate the height-map & the info data
	Heights.Empty(TotalVertices);
	InfoData.Empty(TotalVertices);

	// Copy the old data back in
	for (INT Y = 0; Y < NumVerticesY; Y++)
	{
		// Grab the data at the first vertex in the row
		WORD OldHeightValue = OldHeights(Y * OldNumVerticesX).Value;
		BYTE OldInfoDataValue = OldInfoData(Y * OldNumVerticesX).Data;

		// Fill in new bottom values
		for (INT BottomAdd = 0; BottomAdd < BottomCount; BottomAdd++)
		{
			new(Heights) FTerrainHeight(OldHeightValue);
			new(InfoData) FTerrainInfoData(OldInfoDataValue);
		}
		// Insert the old values
		for (INT X = 0; X < OldNumVerticesX; X++)
		{
			OldHeightValue = OldHeights(Y * OldNumVerticesX + X).Value;
			OldInfoDataValue = OldInfoData(Y * OldNumVerticesX + X).Data;
			new(Heights) FTerrainHeight(OldHeightValue);
			new(InfoData) FTerrainInfoData(OldInfoDataValue);
		}
		// Fill in new top values
		for (INT TopAdd = 0; TopAdd < TopCount; TopAdd++)
		{
			new(Heights) FTerrainHeight(OldHeightValue);
			new(InfoData) FTerrainInfoData(OldInfoDataValue);
		}
	}

	// Allocate the alpha-maps.
	for (INT AlphaMapIndex = 0; AlphaMapIndex < AlphaMaps.Num(); AlphaMapIndex++)
	{
		TArray<BYTE> NewAlphas;
		NewAlphas.Empty(TotalVertices);

		for (INT Y = 0; Y < NumVerticesY; Y++)
		{
			// Grab the data at the first vertex in the row
			BYTE OldAlphaValue = OldAlphaMaps(AlphaMapIndex).Data(Y * OldNumVerticesX);

			// Fill in new bottom values
			for (INT BottomAdd = 0; BottomAdd < BottomCount; BottomAdd++)
			{
				new(NewAlphas) BYTE(OldAlphaValue);
			}
			// Insert the old values
			for (INT X = 0; X < OldNumVerticesX; X++)
			{
				OldAlphaValue = OldAlphaMaps(AlphaMapIndex).Data(Y * OldNumVerticesX + X);
				new(NewAlphas) BYTE(OldAlphaValue);
			}
			// Fill in new top values
			for (INT TopAdd = 0; TopAdd < TopCount; TopAdd++)
			{
				new(NewAlphas) BYTE(OldAlphaValue);
			}
		}
		AlphaMaps(AlphaMapIndex).Data.Add(NewAlphas.Num());
		appMemcpy(&AlphaMaps(AlphaMapIndex).Data(0),&NewAlphas(0),NewAlphas.Num());
	}

	if (BottomCount > 0)
	{
		// We need to move the position of the terrain...
		FVector PosOffset = FVector(-((FLOAT)BottomCount), 0.0f, 0.0f);
		PosOffset *= DrawScale * DrawScale3D;
		Location += PosOffset;
	}

	return TRUE;
}

UBOOL ATerrain::AddSectors_Y(INT Count)
{
	if (Count == 0)
	{
		return TRUE;
	}

	INT AbsCount = Abs(Count);
	INT	LeftCount = 0;
	INT RightCount = 0;
	if (Count > 0)
	{
		// We are adding patches to the 'right' of the terrain
		RightCount = AbsCount * MaxTesselationLevel;
	}
	else
	{
		// We are adding patches to the 'left' of the terrain
		LeftCount = AbsCount * MaxTesselationLevel;
	}

	// Store off the old data
    TArray<FTerrainHeight> OldHeights;
    TArray<FTerrainInfoData> OldInfoData;
    TArray<FAlphaMap> OldAlphaMaps;
	StoreOldData(OldHeights, OldInfoData, OldAlphaMaps);

	// Adjust the number of patches
	INT	OldNumVerticesX = NumVerticesX,
		OldNumVerticesY = NumVerticesY;

	INT OldNumPatchesY = NumPatchesY;
	NumPatchesY += LeftCount + RightCount;
	SetupSizeData();

	INT TotalVertices = NumVerticesX * NumVerticesY;
	// Allocate the height-map & the info data
	Heights.Empty(TotalVertices);
	InfoData.Empty(TotalVertices);

	// Copy the old data back in
	WORD OldHeightValue;
	BYTE OldInfoDataValue;
	// Fill in new left values
	for (INT LeftAdd = 0; LeftAdd < LeftCount; LeftAdd++)
	{
		// Insert the old values
		for (INT X = 0; X < OldNumVerticesX; X++)
		{
			// Grab the data at the first vertex in the row
			OldHeightValue = OldHeights(0 + X).Value;
			OldInfoDataValue = OldInfoData(0 + X).Data;
			new(Heights) FTerrainHeight(OldHeightValue);
			new(InfoData) FTerrainInfoData(OldInfoDataValue);
		}
	}

	// Copy the old values
	for (INT Y = 0; Y < OldNumVerticesY; Y++)
	{
		for (INT X = 0; X < OldNumVerticesX; X++)
		{
			OldHeightValue = OldHeights(Y * OldNumVerticesX + X).Value;
			OldInfoDataValue = OldInfoData(Y * OldNumVerticesX + X).Data;
			new(Heights) FTerrainHeight(OldHeightValue);
			new(InfoData) FTerrainInfoData(OldInfoDataValue);
		}
	}

	// Fill in new top values
	for (INT RightAdd = 0; RightAdd < RightCount; RightAdd++)
	{
		// Insert the old values
		for (INT X = 0; X < OldNumVerticesX; X++)
		{
			OldHeightValue = OldHeights((OldNumVerticesY - 1) * OldNumVerticesX + X).Value;
			OldInfoDataValue = OldInfoData((OldNumVerticesY - 1) * OldNumVerticesX + X).Data;
			new(Heights) FTerrainHeight(OldHeightValue);
			new(InfoData) FTerrainInfoData(OldInfoDataValue);
		}
	}

	// Allocate the alpha-maps.
	for (INT AlphaMapIndex = 0; AlphaMapIndex < AlphaMaps.Num(); AlphaMapIndex++)
	{
		TArray<BYTE> NewAlphas;
		NewAlphas.Empty(TotalVertices);

		BYTE OldAlphaValue;

		// Fill in new left values
		for (INT LeftAdd = 0; LeftAdd < LeftCount; LeftAdd++)
		{
			// Insert the old values
			for (INT X = 0; X < OldNumVerticesX; X++)
			{
				OldAlphaValue = OldAlphaMaps(AlphaMapIndex).Data(0 + X);
				new(NewAlphas) BYTE(OldAlphaValue);
			}
		}

		// Copy the old values
		for (INT Y = 0; Y < OldNumVerticesY; Y++)
		{
			for (INT X = 0; X < OldNumVerticesX; X++)
			{
				OldAlphaValue = OldAlphaMaps(AlphaMapIndex).Data(Y * OldNumVerticesX + X);
				new(NewAlphas) BYTE(OldAlphaValue);
			}
		}

		// Fill in new top values
		for (INT RightAdd = 0; RightAdd < RightCount; RightAdd++)
		{
			OldAlphaValue = OldAlphaMaps(AlphaMapIndex).Data((OldNumVerticesY - 1) * OldNumVerticesX + (OldNumVerticesX - 1));
			new(NewAlphas) BYTE(OldAlphaValue);
		}
		AlphaMaps(AlphaMapIndex).Data.Add(NewAlphas.Num());
		appMemcpy(&AlphaMaps(AlphaMapIndex).Data(0),&NewAlphas(0),NewAlphas.Num());
	}

	if (LeftCount > 0)
	{
		// We need to move the position of the terrain...
		FVector PosOffset = FVector(0.0f, -((FLOAT)LeftCount), 0.0f);
		PosOffset *= DrawScale * DrawScale3D;
		Location += PosOffset;
	}

	return TRUE;
}

UBOOL ATerrain::RemoveSectors_X(INT Count)
{
	if (Count == 0)
	{
		return TRUE;
	}

	INT	TopCount = 0;
	INT BottomCount = 0;
	if (Count > 0)
	{
		// We are removing patches from the 'top' of the terrain
		TopCount = Abs(Count) * MaxTesselationLevel;
	}
	else
	{
		// We are removing patches from the 'bottom' of the terrain
		BottomCount = Abs(Count) * MaxTesselationLevel;
	}

	// Store off the old data
    TArray<FTerrainHeight> OldHeights;
    TArray<FTerrainInfoData> OldInfoData;
    TArray<FAlphaMap> OldAlphaMaps;
	StoreOldData(OldHeights, OldInfoData, OldAlphaMaps);

	// Adjust the number of patches
	INT	OldNumVerticesX = NumVerticesX,
		OldNumVerticesY = NumVerticesY;

	INT OldNumPatchesX = NumPatchesX;
	NumPatchesX -= (TopCount + BottomCount);
	SetupSizeData();

	INT TotalVertices = NumVerticesX * NumVerticesY;
	// Allocate the height-map & the info data
	Heights.Empty(TotalVertices);
	InfoData.Empty(TotalVertices);

	// Copy the old data back in
	WORD OldHeightValue;
	BYTE OldInfoDataValue;
	for (INT Y = 0; Y < NumVerticesY; Y++)
	{
		// Insert the old values
		for (INT X = BottomCount; X < OldNumVerticesX - TopCount; X++)
		{
			OldHeightValue = OldHeights(Y * OldNumVerticesX + X).Value;
			OldInfoDataValue = OldInfoData(Y * OldNumVerticesX + X).Data;
			new(Heights) FTerrainHeight(OldHeightValue);
			new(InfoData) FTerrainInfoData(OldInfoDataValue);
		}
	}

	// Allocate the alpha-maps.
	for (INT AlphaMapIndex = 0; AlphaMapIndex < AlphaMaps.Num(); AlphaMapIndex++)
	{
		TArray<BYTE> NewAlphas;
		NewAlphas.Empty(TotalVertices);

		BYTE OldAlphaValue;

		for (INT Y = 0; Y < NumVerticesY; Y++)
		{
			// Insert the old values
			for (INT X = BottomCount; X < OldNumVerticesX - TopCount; X++)
			{
				OldAlphaValue = OldAlphaMaps(AlphaMapIndex).Data(Y * OldNumVerticesX + X);
				new(NewAlphas) BYTE(OldAlphaValue);
			}
		}
		AlphaMaps(AlphaMapIndex).Data.Add(NewAlphas.Num());
		appMemcpy(&AlphaMaps(AlphaMapIndex).Data(0),&NewAlphas(0),NewAlphas.Num());
	}

	if (BottomCount > 0)
	{
		// We need to move the position of the terrain...
		FVector PosOffset = FVector(((FLOAT)BottomCount), 0.0f, 0.0f);
		PosOffset *= DrawScale * DrawScale3D;
		Location += PosOffset;
	}

	return TRUE;
}

UBOOL ATerrain::RemoveSectors_Y(INT Count)
{
	if (Count == 0)
	{
		return TRUE;
	}

	INT AbsCount = Abs(Count);
	INT	LeftCount = 0;
	INT RightCount = 0;
	if (Count > 0)
	{
		// We are removing patches from the 'right' of the terrain
		RightCount = AbsCount * MaxTesselationLevel;
	}
	else
	{
		// We are removing patches from the 'left' of the terrain
		LeftCount = AbsCount * MaxTesselationLevel;
	}

	// Store off the old data
    TArray<FTerrainHeight> OldHeights;
    TArray<FTerrainInfoData> OldInfoData;
    TArray<FAlphaMap> OldAlphaMaps;
	StoreOldData(OldHeights, OldInfoData, OldAlphaMaps);

	// Adjust the number of patches
	INT	OldNumVerticesX = NumVerticesX,
		OldNumVerticesY = NumVerticesY;

	INT OldNumPatchesY = NumPatchesY;
	NumPatchesY -= (LeftCount + RightCount);
	SetupSizeData();

	INT TotalVertices = NumVerticesX * NumVerticesY;
	// Allocate the height-map & the info data
	Heights.Empty(TotalVertices);
	InfoData.Empty(TotalVertices);

	// Copy the old data back in
	WORD OldHeightValue;
	BYTE OldInfoDataValue;
	// Copy the old values
	for (INT Y = LeftCount; Y < (OldNumVerticesY - RightCount); Y++)
	{
		for (INT X = 0; X < OldNumVerticesX; X++)
		{
			OldHeightValue = OldHeights(Y * OldNumVerticesX + X).Value;
			OldInfoDataValue = OldInfoData(Y * OldNumVerticesX + X).Data;
			new(Heights) FTerrainHeight(OldHeightValue);
			new(InfoData) FTerrainInfoData(OldInfoDataValue);
		}
	}

	// Allocate the alpha-maps.
	for (INT AlphaMapIndex = 0; AlphaMapIndex < AlphaMaps.Num(); AlphaMapIndex++)
	{
		TArray<BYTE> NewAlphas;
		NewAlphas.Empty(TotalVertices);

		BYTE OldAlphaValue;

		// Copy the old values
		for (INT Y = LeftCount; Y < (OldNumVerticesY - RightCount); Y++)
		{
			for (INT X = 0; X < OldNumVerticesX; X++)
			{
				OldAlphaValue = OldAlphaMaps(AlphaMapIndex).Data(Y * OldNumVerticesX + X);
				new(NewAlphas) BYTE(OldAlphaValue);
			}
		}

		AlphaMaps(AlphaMapIndex).Data.Add(NewAlphas.Num());
		appMemcpy(&AlphaMaps(AlphaMapIndex).Data(0),&NewAlphas(0),NewAlphas.Num());
	}

	if (LeftCount > 0)
	{
		// We need to move the position of the terrain...
		FVector PosOffset = FVector(0.0f, ((FLOAT)LeftCount), 0.0f);
		PosOffset *= DrawScale * DrawScale3D;
		Location += PosOffset;
	}

	return TRUE;
}

void ATerrain::PreEditChange(UProperty* PropertyThatChanged)
{
	Super::PreEditChange(PropertyThatChanged);

	// wait until resources are released
	FlushRenderingCommands();

	//@todo. We don't want to do this every time...
	ClearComponents();
}

//
//	ATerrain::PostEditChange
//

void ATerrain::PostEditChange(UProperty* PropertyThatChanged)
{
	UBOOL bRecacheMaterials = FALSE;
	UBOOL bCycleComponents = FALSE;
	UBOOL bRebuildCollisionData = FALSE;

	// Ensure the min and max tessellation level is at a valid setting
	MaxTesselationLevel = Min<UINT>(appRoundUpToPowerOfTwo(Max(MaxTesselationLevel,1)),TERRAIN_MAXTESSELATION);
	MinTessellationLevel = Min<UINT>(appRoundUpToPowerOfTwo(Max(MinTessellationLevel,1)),TERRAIN_MAXTESSELATION);
	if (EditorTessellationLevel != 0)
	{
		EditorTessellationLevel = Min<UINT>(appRoundUpToPowerOfTwo(Max(EditorTessellationLevel,0)),TERRAIN_MAXTESSELATION);
	}

	// Clamp the terrain size properties to valid values.
	NumPatchesX = Clamp(NumPatchesX,1,2048);
	NumPatchesY = Clamp(NumPatchesY,1,2048);
	// We need to clamp the patches to MaxDetessellationLevel
	if ((NumPatchesX % MaxTesselationLevel) > 0)
	{
		NumPatchesX += MaxTesselationLevel - (NumPatchesX % MaxTesselationLevel);
	}
	if ((NumPatchesY % MaxTesselationLevel) > 0)
	{
		NumPatchesY += MaxTesselationLevel - (NumPatchesY % MaxTesselationLevel);
	}
	// Limit MaxComponentSize from being 0, negative or large enough to exceed the maximum vertex buffer size.
	MaxComponentSize = Clamp(MaxComponentSize,1,((255 / MaxTesselationLevel) - 1));

	if (PropertyThatChanged)
	{
		FString PropThatChangedStr = PropertyThatChanged->GetName();
		if (appStricmp(*PropThatChangedStr, TEXT("Setup")) == 0)
		{
			// It's a layer setup
			bRecacheMaterials = TRUE;
		}
		else
		if (appStricmp(*PropThatChangedStr, TEXT("Material")) == 0)
		{
			// It's a terrain material OR a material
			bRecacheMaterials = TRUE;
		}
		else
		if ((appStricmp(*PropThatChangedStr, TEXT("MinTessellationLevel")) == 0) ||
			(appStricmp(*PropThatChangedStr, TEXT("MaxTesselationLevel")) == 0))
		{
			if (appStricmp(*PropThatChangedStr, TEXT("MaxTesselationLevel")) == 0)
			{
				RecacheVisibilityFlags();
			}
			if (MinTessellationLevel > MaxTesselationLevel)
			{
				MinTessellationLevel = MaxTesselationLevel;
			}
			bCycleComponents = TRUE;
		}
		else
		if (appStricmp(*PropThatChangedStr, TEXT("NormalMapLayer")) == 0)
		{
			// They want a normal map from a different layer
			bRecacheMaterials = TRUE;
		}
		else
		if ((appStricmp(*PropThatChangedStr, TEXT("NumPatchesX")) == 0) ||
			(appStricmp(*PropThatChangedStr, TEXT("NumPatchesY")) == 0))
		{
			bCycleComponents = TRUE;
		}
		else
		if ((appStricmp(*PropThatChangedStr, TEXT("DrawScale")) == 0) ||
			(appStricmp(*PropThatChangedStr, TEXT("DrawScale3D")) == 0))
		{
			bRebuildCollisionData = TRUE;
		}
		else
		if (appStricmp(*PropThatChangedStr, TEXT("EditorTessellationLevel")) == 0)
		{
			EditorTessellationLevel = Clamp<INT>(EditorTessellationLevel, 0, MaxTesselationLevel);
			bCycleComponents = TRUE;
		}
		else
		if ((appStricmp(*PropThatChangedStr, TEXT("MappingType")) == 0) ||
			(appStricmp(*PropThatChangedStr, TEXT("MappingScale")) == 0) ||
			(appStricmp(*PropThatChangedStr, TEXT("MappingRotation")) == 0) ||
			(appStricmp(*PropThatChangedStr, TEXT("MappingPanU")) == 0) ||
			(appStricmp(*PropThatChangedStr, TEXT("MappingPanV")) == 0)
			)
		{
			CacheDisplacements(0,0,NumVerticesX - 1,NumVerticesY - 1);
			bCycleComponents = TRUE;
		}
		else
		if ((appStricmp(*PropThatChangedStr, TEXT("bMorphingEnabled")) == 0) ||
			(appStricmp(*PropThatChangedStr, TEXT("bMorphingGradientsEnabled")) == 0)
			)
		{
			bCycleComponents = TRUE;
			//recompile material shaders since a different vertex factory has to be compiled now
			bRecacheMaterials = TRUE;
		}
	}

	if (bRecacheMaterials == TRUE)
	{
		RecacheMaterials();
	}
	if (bCycleComponents == TRUE)
	{
		ClearComponents();
	}
	// No longer using lower tessellation collision.
	CollisionTesselationLevel = MaxTesselationLevel;
	if (bRebuildCollisionData == TRUE)
	{
		BuildCollisionData();
	}

	// Check the lighting resolution
	if (bIsOverridingLightResolution)
	{
		StaticLightingResolution = appRoundUpToPowerOfTwo(Max(StaticLightingResolution,1));

		if (GIsEditor)
		{
			// Warn the user?
			INT LightMapSize	= MaxComponentSize * StaticLightingResolution + 1;
			INT NumSectionsX = ((NumPatchesX / MaxTesselationLevel) + MaxComponentSize - 1) / MaxComponentSize;
			INT NumSectionsY = ((NumPatchesY / MaxTesselationLevel) + MaxComponentSize - 1) / MaxComponentSize;
			
			debugf(TEXT("Terrain %16s: Potential lightmap size per component = %4dx%4d"), *GetName(), LightMapSize, LightMapSize);
			debugf(TEXT("            : %2d x %2d Components"), NumSectionsX, NumSectionsY);
			
			INT	CheckMem		= (LightMapSize * LightMapSize) * (NumSectionsX * NumSectionsY);
			debugf(TEXT("            : Potential memory usage of %10d bytes (%d MBs)"),
				CheckMem, CheckMem / (1024*1024));
		}
	}
	else
	{
		StaticLightingResolution = Min<UINT>(appRoundUpToPowerOfTwo(Max(StaticLightingResolution,1)),MaxTesselationLevel);
	}

	// Cleanup and unreferenced alpha maps.
	CompactAlphaMaps();

	// Reallocate height-map and alpha-map data with the new dimensions.
	Allocate();

	// Update cached weightmaps and presampled displacement maps.
	ClearWeightMaps();
	CacheWeightMaps(0,0,NumVerticesX - 1,NumVerticesY - 1);
	TouchWeightMapResources();

	// Update the local to mapping transform for each material.
	for (UINT LayerIndex = 0;LayerIndex < (UINT)Layers.Num();LayerIndex++)
	{
		if (Layers(LayerIndex).Setup)
		{
			for(UINT MaterialIndex = 0;MaterialIndex < (UINT)Layers(LayerIndex).Setup->Materials.Num();MaterialIndex++)
			{
				if(Layers(LayerIndex).Setup->Materials(MaterialIndex).Material)
				{
					Layers(LayerIndex).Setup->Materials(MaterialIndex).Material->UpdateMappingTransform();
				}
			}
		}
	}

	if (bCycleComponents == TRUE)
	{
		ConditionalUpdateComponents(FALSE);
	}

	CacheDisplacements(0,0,NumVerticesX - 1,NumVerticesY - 1);
	CacheDecorations(0,0,NumVerticesX - 1,NumVerticesY - 1);

	// Re-init the rigid-body physics for the terrain components.
	InitRBPhys();

	if (GIsEditor)
	{
		GCallbackEvent->Send(CALLBACK_RefreshEditor_TerrainBrowser);

		// Weld to other terrains (eg if we moved it or changed its scale)
		WeldEdgesToOtherTerrains();
	}

	Super::PostEditChange(PropertyThatChanged);
}

void ATerrain::PostEditMove(UBOOL bFinished)
{
	Super::PostEditMove( bFinished );
}

//
//	ATerrain::PostLoad
//

void ATerrain::PostLoad()
{
	Super::PostLoad();

	// Remove terrain components from the main components array.
    for(INT ComponentIndex = 0;ComponentIndex < Components.Num();ComponentIndex++)
	{
		if(Components(ComponentIndex) && Components(ComponentIndex)->IsA(UTerrainComponent::StaticClass()))
		{
			Components.Remove(ComponentIndex--);
		}
	}

	// Propagate the terrain lighting properties to its components.
	// This is necessary when the terrain's default values for these properties change.
	for(INT ComponentIndex = 0;ComponentIndex < TerrainComponents.Num();ComponentIndex++)
	{
		UTerrainComponent* TerrainComponent = TerrainComponents(ComponentIndex);
		if(TerrainComponent)
		{
			TerrainComponent->CastShadow			= bCastShadow;
			TerrainComponent->bCastDynamicShadow	= bCastDynamicShadow;
			TerrainComponent->bForceDirectLightMap	= bForceDirectLightMap;
			TerrainComponent->BlockRigidBody		= bBlockRigidBody;
			TerrainComponent->bAcceptsDynamicLights	= bAcceptsDynamicLights;
			TerrainComponent->LightingChannels		= LightingChannels;
			TerrainComponent->PhysMaterialOverride	= TerrainPhysMaterialOverride;
		}
	}

	if (!NumPatchesX || !NumPatchesY || !MaxComponentSize)
	{
		// For terrain actors saved without a size, use the default.
		NumPatchesX = NumSectionsX * SectionSize;
		NumPatchesY = NumSectionsY * SectionSize;
		NumVerticesX = NumPatchesX + 1;
		NumVerticesY = NumPatchesY + 1;
		MaxComponentSize = 16;
	}

	// If the engine INI file has bForceStaticTerrain == TRUE, do so.
	if (((GIsGame == TRUE) || (GIsPlayInEditorWorld == TRUE)) && (GEngine->bForceStaticTerrain == TRUE))
	{
		// Force the MinTessellationLevel to the max - ie make terrain static.
		MinTessellationLevel = MaxTesselationLevel;
	}

	ClearWeightMaps();
	CacheWeightMaps(0,0,NumVerticesX - 1,NumVerticesY - 1);
#if !CONSOLE
	if (GIsGame == FALSE)
	{
		TouchWeightMapResources();
	}
#endif

	// Since the PostLoad of the cached materials will potentially compile the material,
	// need to ensure that the underlying materials have been fully loaded.
	for (INT LayerIndex = 0; LayerIndex < Layers.Num(); LayerIndex++)
	{
		FTerrainLayer* Layer = &(Layers(LayerIndex));
		if (Layer->Setup)
		{
			Layer->Setup->ConditionalPostLoad();
		}
	}

	// Make sure that all the necessary material resources are created on load, to work around a past bug where PreSave
	// deleted MSP_SM2 material resources.
	for( INT PlatformIdx=0; PlatformIdx < MSP_MAX; PlatformIdx++ )
	{
		const EMaterialShaderPlatform MatPlatform = (EMaterialShaderPlatform)PlatformIdx;
		for (INT ComponentIndex = 0; ComponentIndex < TerrainComponents.Num(); ComponentIndex++)
		{
			UTerrainComponent* Comp = TerrainComponents(ComponentIndex);
			if (Comp)
			{
				for (INT MaterialIndex = 0; MaterialIndex < Comp->BatchMaterials.Num(); MaterialIndex++)
				{
					// add new entry if missing
					GenerateCachedMaterial(Comp->BatchMaterials(MaterialIndex), MatPlatform);
				}
			}
		}
	}

	if (GCookingTarget & UE3::PLATFORM_Windows)
	{
		// cache shaders for all PC platforms if we are cooking for PC
		CacheResourceShaders(SP_PCD3D_SM2, FALSE, FALSE);
		CacheResourceShaders(SP_PCD3D_SM3, FALSE, FALSE);
		CacheResourceShaders(SP_PCD3D_SM4, FALSE, FALSE);
	}
	else
	{
		// make sure terrain material shader resources for the current platform have been initialized
		CacheResourceShaders(GRHIShaderPlatform, FALSE, FALSE);
	}

	// only postload the ones for the current RHI platform
	const EMaterialShaderPlatform MatPlatform = GetMaterialPlatform(GRHIShaderPlatform);
	TArrayNoInit<FTerrainMaterialResource*>& CachedMaterials = GetCachedTerrainMaterials(MatPlatform);
	for( INT MatIdx = 0; MatIdx < CachedMaterials.Num(); MatIdx++ )
	{
		FTerrainMaterialResource* CachedMaterial = CachedMaterials(MatIdx);
		if( CachedMaterial != NULL )
		{
			CachedMaterial->PostLoad(MatPlatform);
			if( GIsCooking )
			{
				// Material failed to compile? Don't bother streaming it if we are cooking!
				if( CachedMaterial->GetShaderMap() == NULL )
				{
					warnf(TEXT("Terrain::PostLoad> CachedMaterial failed to compile? Ditching %s"),*(CachedMaterial->GetFriendlyName()));
					delete CachedMaterials(MatIdx);
					CachedMaterials(MatIdx) = NULL;
					CachedMaterials.Remove(MatIdx);
					// Decrement MatIdx to account for the loop iteration
					--MatIdx;
				}
			}
			else
			{
				FMaterialShaderMap* LocalShaderMap = CachedMaterial->GetShaderMap();
				if( LocalShaderMap == NULL ||
					CachedMaterial->MaterialIds.Num() == 0 )
				{
					warnf(TEXT("Terrain::PostLoad> CachedMaterial failed to compile? Forcing re-compilation of materal %s"),*(CachedMaterial->GetFriendlyName()));
					// Force re-compilation of the material.
					delete CachedMaterials(MatIdx);
					CachedMaterials(MatIdx) = NULL;
					CachedMaterials.Remove(MatIdx);
					// Decrement MatIdx to account for the loop iteration
					--MatIdx;
				}
			}
		}
	}

	if (GIsEditor == TRUE)
	{
		CacheDisplacements(0,0,NumVerticesX - 1,NumVerticesY - 1);
	}

	// No longer using lower tessellation collision.
	CollisionTesselationLevel = MaxTesselationLevel;

	if (GIsGame)
	{
		for (INT TerrainCompIndex = 0; TerrainCompIndex < TerrainComponents.Num(); TerrainCompIndex++)
		{
			UTerrainComponent* S = TerrainComponents(TerrainCompIndex);
			if (S)
			{
				if (IsTerrainComponentVisible(S) == FALSE)
				{
					// Clear it out if it is not visible.
					TerrainComponents(TerrainCompIndex) = NULL;
				}
			}
		}
	}
}

/** 
 *	Called before the Actor is saved. 
 */
void ATerrain::PreSave()
{
	if(!IsTemplate())
	{
		// during save we want to generate the cached terrain material entries for all platforms
		for( INT PlatformIdx=0; PlatformIdx < MSP_MAX; PlatformIdx++ )
		{
			EMaterialShaderPlatform MatPlatform = (EMaterialShaderPlatform)PlatformIdx;

			// make sure all materials are generate before saving
			for (INT ComponentIndex = 0; ComponentIndex < TerrainComponents.Num(); ComponentIndex++)
			{
				UTerrainComponent* Comp = TerrainComponents(ComponentIndex);
				if (Comp)
				{
					for (INT MaterialIndex = 0; MaterialIndex < Comp->BatchMaterials.Num(); MaterialIndex++)
					{
						// add new entry if missing
						GenerateCachedMaterial(Comp->BatchMaterials(MaterialIndex), MatPlatform);
					}
				}
			}
		}

		for( INT PlatformIdx=0; PlatformIdx < MSP_MAX; PlatformIdx++ )
		{
			const EMaterialShaderPlatform MatPlatform = (EMaterialShaderPlatform)PlatformIdx;
			TArrayNoInit<FTerrainMaterialResource*>& CachedMaterials = GetCachedTerrainMaterials(MatPlatform);

			// clear out empty entries
			for( INT MatIdx=CachedMaterials.Num()-1;  MatIdx >= 0; MatIdx-- )
			{
				if( CachedMaterials(MatIdx) == NULL )
				{
					CachedMaterials.Remove(MatIdx);						
				}
			}
			// call presave for each one		
			for( INT MatIdx=0; MatIdx < CachedMaterials.Num(); MatIdx++ )
			{
				if( CachedMaterials(MatIdx) )
				{
						CachedMaterials(MatIdx)->PreSave(MatPlatform);
				}
			}
		}

		// make sure terrain material shader resources for all platforms have been compiled 
		// see comment UMaterial::PreSave
		// This is also done in cooking, as we may have only created the material resources that need to be compiled in this PreSave call.
		// UMaterial::PreSave doesn't do this during cooking, as PreSave won't invalidate the material's shaders in any case.
		if( !GIsUCCMake )
		{
			CacheResourceShaders(GRHIShaderPlatform, FALSE, TRUE);

			// Also ensure that displacements are cached, as they are now serialized as well.
			CacheDisplacements(0, 0, NumPatchesX, NumPatchesY);
		}
	}
}

void ATerrain::BeginDestroy()
{
	Super::BeginDestroy();

	FRenderCommandFence* Fence = RetrieveReleaseResourcesFence();
	if (Fence)
	{
		Fence->BeginFence();
	}
}

UBOOL ATerrain::IsReadyForFinishDestroy()
{
	// see if we have hit the resource flush fence
	UBOOL bIsReady = TRUE;
	FRenderCommandFence* Fence = GetReleaseResourcesFence();
	if (Fence)
	{
		bIsReady = (Fence->GetNumPendingFences() == 0);
	}
	UBOOL bSuperIsReady = Super::IsReadyForFinishDestroy();
	return (bSuperIsReady && bIsReady);
}

/**
* Delete the entries in the cached terrain materials
*/
void ATerrain::ClearCachedTerrainMaterials( UBOOL bOtherShaderPlatformsOnly )
{
	for( INT PlatformIdx=0; PlatformIdx < MSP_MAX; PlatformIdx++ )
	{
		const EMaterialShaderPlatform MatPlatform = (EMaterialShaderPlatform)PlatformIdx;

		if( !bOtherShaderPlatformsOnly || MatPlatform != GetMaterialPlatform(GRHIShaderPlatform) )
		{
			TArrayNoInit<FTerrainMaterialResource*>& CachedMaterials = GetCachedTerrainMaterials(MatPlatform);
			for( INT MatIdx=0; MatIdx < CachedMaterials.Num(); MatIdx++ )
			{
				delete CachedMaterials(MatIdx);
			}
			CachedMaterials.Empty();
		}
	}
}

void ATerrain::FinishDestroy()
{
	// Cleanup cached materials.
	ClearCachedTerrainMaterials();

	WeightedTextureMaps.Empty();
	WeightedMaterials.Empty();
	FreeReleaseResourcesFence();
	Super::FinishDestroy();
}

void ATerrain::ClearWeightMaps()
{
	// Set them free
	for (INT TextureIndex = 0; TextureIndex < WeightedTextureMaps.Num(); TextureIndex++)
	{
		UTerrainWeightMapTexture* Texture = WeightedTextureMaps(TextureIndex);
		if (Texture && Texture->Resource)
		{
			Texture->ReleaseResource();
		}
	}

	if ((GIsEditor == TRUE) && (GIsCooking == FALSE))
	{
		WeightedTextureMaps.Empty();
	}

	// ReleaseResource will release and flush, so we don't have to wait
	
	// Clear the weighted materials array
	WeightedMaterials.Empty();
}

void ATerrain::TouchWeightMapResources()
{
	if (GIsCooking == TRUE)
	{
		// Do not regenerate the weight map textures when cooking...
		return;
	}

	INT WeightMapIndex;
	for (WeightMapIndex = 0; WeightMapIndex < WeightedMaterials.Num(); WeightMapIndex += 4)
	{
		// Ensure the texture exists and all materials are properly set
		INT TextureIndex = WeightMapIndex / 4;
		UTerrainWeightMapTexture* Texture = NULL;
		if (TextureIndex >= WeightedTextureMaps.Num())
		{
			// Need to create one
			UTerrainWeightMapTexture* NewTexture = ConstructObject<UTerrainWeightMapTexture>(UTerrainWeightMapTexture::StaticClass(), this);
			check(NewTexture);
			for (INT InnerIndex = 0; InnerIndex < 4; InnerIndex++)
			{
				INT Index = WeightMapIndex + InnerIndex;
				if (Index < WeightedMaterials.Num())
				{
					FTerrainWeightedMaterial* WeightedMaterial = &(WeightedMaterials(Index));
					NewTexture->WeightedMaterials.AddItem(WeightedMaterial);
				}
			}
			NewTexture->Initialize(this);
			Texture = NewTexture;
			INT CheckIndex = WeightedTextureMaps.AddItem(Texture);
			check(CheckIndex == TextureIndex);
		}
		else
		{
			Texture = WeightedTextureMaps(TextureIndex);
			check(Texture);

			// Verify the texture is the correct size.
			// If not, we need to resize it.
			if ((Texture->SizeX != NumVerticesX) ||
				(Texture->SizeY != NumVerticesY))
			{
				if (Texture->Resource)
				{
					Texture->ReleaseResource();
					FlushRenderingCommands();
				}
				Texture->Initialize(this);
			}
			else
			{
				// Reconnect the ParentTerrain pointer
				Texture->ParentTerrain = this;
			}

			// Refill the weighted materials array, to ensure we catch any changes
			Texture->WeightedMaterials.Empty();
			INT DataIndex = WeightMapIndex % 4;
			for (INT InnerIndex = 0; InnerIndex < 4; InnerIndex++)
			{
				INT Index = WeightMapIndex + InnerIndex;
				if (Index < WeightedMaterials.Num())
				{
					FTerrainWeightedMaterial* WeightedMaterial = &(WeightedMaterials(Index));
					Texture->WeightedMaterials.AddItem(WeightedMaterial);
				}
			}
		}
	}

	// Now, actually initialize/update the texture
	for (INT TextureIndex = 0; TextureIndex < WeightedTextureMaps.Num(); TextureIndex++)
	{
		UTerrainWeightMapTexture* Texture = NULL;
		Texture = WeightedTextureMaps(TextureIndex);
		if (Texture)
		{
			Texture->UpdateData();
			Texture->UpdateResource();
		}
	}
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void ATerrain::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
	for( INT MaterialIndex=0; MaterialIndex<WeightedMaterials.Num(); MaterialIndex++ )
	{
		const FTerrainWeightedMaterial& WeightedMaterial = WeightedMaterials(MaterialIndex);
		AddReferencedObject( ObjectArray, WeightedMaterial.Terrain );
		AddReferencedObject( ObjectArray, WeightedMaterial.Material );
	}
}

//
//	ATerrain::Serialize
//
void ATerrain::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << Heights;
	Ar << InfoData;
	Ar << AlphaMaps;

	if(!Ar.IsSaving() && !Ar.IsLoading())
	{
		Ar << WeightedMaterials;
	}
	Ar << WeightedTextureMaps;
	
	if( Ar.Ver() < VER_ADDED_TERRAIN_MATERIAL_FALLBACK )
	{
		// Clean-up existing cached materials to avoid leaking memory
		ClearCachedTerrainMaterials();

		// only SM3 versions exist with legacy data
		INT NumCachedMaterials = 0;
		Ar << NumCachedMaterials;
		CachedTerrainMaterials[MSP_SM3].CachedMaterials.Add(NumCachedMaterials);
		for( INT MatIdx=0; MatIdx < CachedTerrainMaterials[MSP_SM3].CachedMaterials.Num(); MatIdx++ )
		{
			FTerrainMaterialResource* MatResource = new FTerrainMaterialResource();
			CachedTerrainMaterials[MSP_SM3].CachedMaterials(MatIdx) = MatResource;
			Ar << *MatResource;
		}
	}
	else
	{
		if( Ar.IsLoading() )
		{
			// Clean-up existing cached materials to avoid leaking memory
			ClearCachedTerrainMaterials();

			// Serialize in the materials for each platform
			for( INT PlatformIdx=0; PlatformIdx < MSP_MAX; PlatformIdx++ )
			{
				INT NumCachedMaterials = 0;
				Ar << NumCachedMaterials;
				CachedTerrainMaterials[PlatformIdx].CachedMaterials.Add(NumCachedMaterials);
				for( INT MatIdx=0; MatIdx < CachedTerrainMaterials[PlatformIdx].CachedMaterials.Num(); MatIdx++ )
				{
					FTerrainMaterialResource* MatResource = new FTerrainMaterialResource();
					CachedTerrainMaterials[PlatformIdx].CachedMaterials(MatIdx) = MatResource;
					Ar << *MatResource;
					if( Ar.IsTransacting() )
					{
						MatResource->InitShaderMap();
					}
				}
			}
		}
		else
		{
			// save the materials for each platform
			for( INT PlatformIdx=0; PlatformIdx < MSP_MAX; PlatformIdx++ )
			{
				INT NumCachedMaterials = CachedTerrainMaterials[PlatformIdx].CachedMaterials.Num();
				Ar << NumCachedMaterials;
				// serialize entries
				for( INT MatIdx=0; MatIdx < CachedTerrainMaterials[PlatformIdx].CachedMaterials.Num(); MatIdx++ )
				{
					FTerrainMaterialResource* MatResource = CachedTerrainMaterials[PlatformIdx].CachedMaterials(MatIdx);
					check(MatResource);
					Ar << *MatResource;
				}
			}
		}	
	}

	if (Ar.Ver() >= VER_TERRAIN_SERIALIZE_DISPLACEMENTS)
	{
		Ar << CachedDisplacements;
		Ar << MaxCollisionDisplacement;
	}
}

//
//	ATerrain::Spawned
//

void ATerrain::Spawned()
{
	Super::Spawned();

	// Allocate persistent terrain data.
	Allocate();

	// Update cached render data.
	PostEditChange(NULL);
}

//
//	ATerrain::ShouldTrace
//

UBOOL ATerrain::ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags)
{
	return (TraceFlags & TRACE_Terrain);
}

/**
 * Function that gets called from within Map_Check to allow this actor to check itself
 * for any potential errors and register them with map check dialog.
 */
void ATerrain::CheckForErrors()
{
	//@todo.SAS. Fill in more warning/error checks.

	// Check for 'empty' layers...
	for (INT LayerIndex = 0; LayerIndex < Layers.Num(); LayerIndex++)
	{
		FTerrainLayer& Layer = Layers(LayerIndex);
		if (Layer.Setup == NULL)
		{
			GWarn->MapCheck_Add(MCTYPE_WARNING, this, *FString::Printf(TEXT("%s - Layer missing setup at index %d!"), *GetName(), LayerIndex), MCACTION_DELETE);
		}
	}

	// Check the status of morphing
	if( bMorphingEnabled == FALSE )
	{
		GWarn->MapCheck_Add(MCTYPE_INFO, this, *FString::Printf(TEXT("Your terrain does not have bMorphingEnabled set to true.  You may see popping." ) ) );
	}
	else
	{
		if( bMorphingGradientsEnabled == FALSE )
		{
			GWarn->MapCheck_Add(MCTYPE_INFO, this, *FString::Printf(TEXT("Your terrain does not have bMorphingGradientsEnabled set to true.  You may see small lighting discontinuties." ) ) );
		}
	}

	// Scan for potential material errors
	CheckForMaterialErrors();
}

/**
 * Function that is called from CheckForErrors - specifically checks for material errors.
 */
void ATerrain::CheckForMaterialErrors()
{
	// Check for a NormalMapLayer index that is out or range...
	if ((NormalMapLayer > -1) && (NormalMapLayer >= Layers.Num()))
	{
		GWarn->MapCheck_Add(MCTYPE_INFO, this, 
			*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Terrain_Error_NormalMapLayer"), 
				NormalMapLayer, Layers.Num(), *(GetFullName()))));
	}

	// Examine the cache materials...

	// only check cached material entries for the currently running platform
	const EMaterialShaderPlatform MatPlatform = GetMaterialPlatform(GRHIShaderPlatform);
	TArrayNoInit<FTerrainMaterialResource*>& CachedMaterials = GetCachedTerrainMaterials(MatPlatform);
	for (INT CachedIndex = 0; CachedIndex < CachedMaterials.Num(); CachedIndex++)
	{
		FTerrainMaterialResource* CachedMaterial = CachedMaterials(CachedIndex);
		if (CachedMaterial)
		{
			const FTerrainMaterialMask& Mask = CachedMaterial->GetMask();
			// Count the number of terrain materials included in this material.
			INT	NumMaterials = 0;
			for(INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
			{
				if(Mask.Get(MaterialIndex))
				{
					NumMaterials++;
				}
			}

			// Are any materials missing the shader map?
			if (CachedMaterial->GetShaderMap() == NULL)
			{
				GWarn->MapCheck_Add(MCTYPE_INFO, this, 
					*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Terrain_Error_MissingShaderMap"), 
					*(CachedMaterial->GetFriendlyName()))));
			}

			if (NumMaterials == 1)
			{
				for (INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
				{
					if (Mask.Get(MaterialIndex))
					{
						FTerrainWeightedMaterial* WeightedMaterial = NULL;
						if (MaterialIndex < WeightedMaterials.Num())
						{
							WeightedMaterial = &(WeightedMaterials(MaterialIndex));
						}

						// Check for an invalid material source
						if (WeightedMaterial == NULL)
						{
							GWarn->MapCheck_Add(MCTYPE_INFO, this, 
								*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Terrain_Error_InvalidMaterialIndex"),
								MaterialIndex, *(GetPathName()))));
						}
					}
				}
			}
			else if(NumMaterials > 1)
			{
				INT MaterialIndex;
				FTerrainWeightedMaterial* WeightedMaterial;

				INT	Result = INDEX_NONE;
				INT TextureCount = 0;
				if (GEngine->TerrainMaterialMaxTextureCount > 0)
				{
					// Do a quick preliminary check to ensure we don't use too many textures.
					TArray<UTexture*> CheckTextures;
					INT WeightMapCount = 0;
					for(MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
					{
						if(Mask.Get(MaterialIndex))
						{
							if (MaterialIndex < WeightedMaterials.Num())
							{
								WeightMapCount = Max<INT>(WeightMapCount, (MaterialIndex / 4) + 1);
								WeightedMaterial = &(WeightedMaterials(MaterialIndex));
								if (WeightedMaterial->Material && WeightedMaterial->Material->Material)
								{
									WeightedMaterial->Material->Material->GetTextures(CheckTextures, TRUE);
								}
							}
						}
					}

					TextureCount = CheckTextures.Num() + WeightMapCount;
				}

				// Check for too many samplers
				if (TextureCount >= GEngine->TerrainMaterialMaxTextureCount)
				{
					// With a shadow map (or light maps) this will fail!
					GWarn->MapCheck_Add(MCTYPE_INFO, this, 
						*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Terrain_Error_MaterialTextureCount"), 
						TextureCount, *(CachedMaterial->GetFriendlyName()))));
				}
			}
		}
		else
		{
		}
	}

	// Check for name collisions on texture parameters...
	//@todo. Remove this check once the support issue is resolved!!!

	// Gather all the textures
	TArray<UTexture*> MaterialTextures;
	TMap<FName, INT> ParameterOccurences;
	ParameterOccurences.Empty();
	for (INT WeightedMaterialIndex = 0; WeightedMaterialIndex < WeightedMaterials.Num(); WeightedMaterialIndex++)
	{
		FTerrainWeightedMaterial& WeightedMaterial = WeightedMaterials(WeightedMaterialIndex);
		UTerrainMaterial* TerrainMat = WeightedMaterial.Material;
		if (TerrainMat)
		{
			UMaterialInterface* MatIntf = TerrainMat->Material;
			if (MatIntf)
			{
				UMaterial* Mat = MatIntf->GetMaterial();
				if (Mat)
				{
					for (INT ExpressionIndex = 0; ExpressionIndex < Mat->Expressions.Num(); ExpressionIndex++)
					{
						UMaterialExpression* MatExp = Mat->Expressions(ExpressionIndex);
						if (MatExp)
						{
							UMaterialExpressionTextureSampleParameter* TextureSample = 
								Cast<UMaterialExpressionTextureSampleParameter>(MatExp);
							if (TextureSample)
							{
								const INT* Value = ParameterOccurences.Find(TextureSample->ParameterName);
								if (Value)
								{
									ParameterOccurences.Set(TextureSample->ParameterName, *Value + 1);
								}
								else
								{
									ParameterOccurences.Set(TextureSample->ParameterName, 0);
								}
							}
						}
					}
				}
			}
		}
	}

	for (TMap<FName, INT>::TIterator It(ParameterOccurences); It; ++It)
	{
		const FName Key = It.Key();
		INT Value = It.Value();
		if (Value > 0)
		{
			GWarn->MapCheck_Add(MCTYPE_INFO, this, 
				*FString::Printf(TEXT("%s %s"), 
					*LocalizeUnrealEd("Terrain_Error_MaterialParameterName"), 
					*(Key.ToString())));
		}
	}

	// 
}

//
//	ATerrain::ClearComponents
//

void ATerrain::ClearComponents()
{
	// wait until resources are released
	FlushRenderingCommands();

	Super::ClearComponents();

	for(INT ComponentIndex = 0;ComponentIndex < TerrainComponents.Num();ComponentIndex++)
	{
		UTerrainComponent* Comp = TerrainComponents(ComponentIndex);
		if (Comp)
		{
			Comp->ConditionalDetach();
		}
	}

	for(UINT DecoLayerIndex = 0;DecoLayerIndex < (UINT)DecoLayers.Num();DecoLayerIndex++)
	{
		for(UINT DecorationIndex = 0;DecorationIndex < (UINT)DecoLayers(DecoLayerIndex).Decorations.Num();DecorationIndex++)
		{
			FTerrainDecoration&	Decoration = DecoLayers(DecoLayerIndex).Decorations(DecorationIndex);
			for(UINT InstanceIndex = 0;InstanceIndex < (UINT)Decoration.Instances.Num();InstanceIndex++)
			{
				FTerrainDecorationInstance&	DecorationInstance = Decoration.Instances(InstanceIndex);
				if (DecorationInstance.Component)
				{
					DecorationInstance.Component->ConditionalDetach();
				}
			}
		}
	}
}

/** updates decoration components to account for terrain/layer property changes */
void ATerrain::UpdateDecorationComponents()
{
	const FMatrix& ActorToWorld = LocalToWorld();

	for (INT DecoLayerIndex = 0; DecoLayerIndex < DecoLayers.Num(); DecoLayerIndex++)
	{
		for(INT DecorationIndex = 0; DecorationIndex < DecoLayers(DecoLayerIndex).Decorations.Num(); DecorationIndex++)
		{
			FTerrainDecoration&	Decoration = DecoLayers(DecoLayerIndex).Decorations(DecorationIndex);

			for (INT InstanceIndex = 0; InstanceIndex < Decoration.Instances.Num(); InstanceIndex++)
			{
				FTerrainDecorationInstance&	DecorationInstance = Decoration.Instances(InstanceIndex);
				check(DecorationInstance.Component != NULL);

				INT				IntX = appTrunc(DecorationInstance.X),
								IntY = appTrunc(DecorationInstance.Y);
				if (IsTerrainQuadVisible(IntX, IntY) == TRUE)
				{
					INT				SubX = 0;
					INT				SubY = 0;
					FTerrainPatch	Patch = GetPatch(IntX, IntY);
					FVector			Location = ActorToWorld.TransformFVector(GetCollisionVertex(Patch, IntX, IntY, SubX, SubY, MaxTesselationLevel));
					FRotator		Rotation(0, DecorationInstance.Yaw, 0);
					FVector			TangentX = FVector(1, 0, GCollisionPatchSampler.SampleDerivX(Patch, SubX, SubY) * TERRAIN_ZSCALE),
									TangentY = FVector(0 ,1, GCollisionPatchSampler.SampleDerivY(Patch, SubX, SubY) * TERRAIN_ZSCALE);

					const FMatrix& DecorationToWorld = FRotationMatrix(Rotation) *
						FRotationMatrix(
							Lerp(
								FVector(ActorToWorld.TransformNormal(FVector(1,0,0)).SafeNormal()),
								(TangentY ^ (TangentX ^ TangentY)).SafeNormal(),
								Clamp(Decoration.SlopeRotationBlend, 0.0f, 1.0f)
							).SafeNormal().Rotation()
							) *
						FScaleMatrix(DrawScale * DrawScale3D) *
						FTranslationMatrix(Location);

					DecorationInstance.Component->ConditionalDetach();
					DecorationInstance.Component->UpdateComponent(GWorld->Scene,this,DecorationToWorld);
				}
				else
				{
					DetachComponent(DecorationInstance.Component);
					DecorationInstance.Component = NULL;
					Decoration.Instances.Remove(InstanceIndex);
				}
			}
		}
	}
}

/** Clamps the vertex index to a valid vertex index (0 to NumVerticesX - 1, 0 to NumVerticesY - 1) that can be used to address the vertex collection.
 * An invalid vertex index is something like (-1,-1) which would cause an array out of bounds exception.
 *
 * @param	OutX	The clamped X coordinate.
 * @param	OutY	The clamped Y coordinate.
 */
void ATerrain::ClampVertexIndex(INT& OutX, INT& OutY) const
{
	OutX = Clamp(OutX, 0, NumVerticesX - 1);
	OutY = Clamp(OutY, 0, NumVerticesY - 1);
}

void ATerrain::UpdateComponentsInternal(UBOOL bCollisionUpdate)
{
	Super::UpdateComponentsInternal(bCollisionUpdate);

	const FMatrix&	ActorToWorld = LocalToWorld();

	for(INT ComponentIndex = 0;ComponentIndex < TerrainComponents.Num();ComponentIndex++)
	{
		UTerrainComponent* Comp = TerrainComponents(ComponentIndex);
		if (Comp)
		{
			Comp->UpdateComponent(GWorld->Scene,this,ActorToWorld);
		}
	}

	UpdateDecorationComponents();
}

//
//	ATerrain::UpdatePatchBounds
//
void ATerrain::UpdatePatchBounds(INT MinX,INT MinY,INT MaxX,INT MaxY)
{
	// Update the terrain components.
	for(UINT ComponentIndex = 0;ComponentIndex < (UINT)TerrainComponents.Num();ComponentIndex++)
	{
		UTerrainComponent*	Component = TerrainComponents(ComponentIndex);
		if (Component	&&
			(Component->SectionBaseX + (Component->SectionSizeX * MaxTesselationLevel) >= MinX) && 
			(Component->SectionBaseX <= MaxX) &&
			(Component->SectionBaseY + (Component->SectionSizeY * MaxTesselationLevel) >= MinY) && 
			(Component->SectionBaseY <= MaxY)
			)
		{
			Component->UpdatePatchBounds();
		}
	}
}

//
//  ATerrain::MatchTerrainEdges
//
void ATerrain::WeldEdgesToOtherTerrains()
{
	UBOOL Changed = FALSE;

	for (FActorIterator It; It; ++It)
	{
		ATerrain* Other = Cast<ATerrain>(*It);
		if( Other && Other != this && 
			Abs(Other->Location.Z-Location.Z) < SMALL_NUMBER &&											// Z values match
			(Other->DrawScale*Other->DrawScale3D-DrawScale*DrawScale3D).SizeSquared() < SMALL_NUMBER		// same scale
			)
		{
			// check Left
			if( Abs(Other->Location.X + (Other->NumPatchesX * Other->DrawScale * Other->DrawScale3D.X) - Location.X) < SMALL_NUMBER )
			{
				// get vertical offset
				FLOAT FYoff = (Other->Location.Y - Location.Y) / (DrawScale * DrawScale3D.Y);
				INT Yoff = appRound(FYoff);
				if( Abs(FYoff-Yoff) < SMALL_NUMBER )
				{
					INT MinY = Clamp<INT>( Yoff,						0, NumVerticesY-1 );
					INT MaxY = Clamp<INT>( Yoff + Other->NumVerticesY-1,0, NumVerticesY-1 );

					UBOOL Dirty = FALSE;

					// adjust heights
					for( INT y=MinY;y<=MaxY;y++ )
					{
						if( Height(0, y) != Other->Height(Other->NumVerticesX-1, y-Yoff) )
						{
							Height(0, y) = Other->Height(Other->NumVerticesX-1, y-Yoff);
							Dirty = TRUE;
						}
					}

					if( Dirty )
					{
						// update terrain
						UpdateRenderData(0,MinY,0,MaxY-1);
						UpdatePatchBounds(0,MinY,0,MaxY-1);
					}
				}
			}

			// check Right
			if( Abs(Location.X + (NumPatchesX * DrawScale * DrawScale3D.X) - Other->Location.X) < SMALL_NUMBER )
			{
				// get vertical offset
				FLOAT FYoff = (Other->Location.Y - Location.Y) / (DrawScale * DrawScale3D.Y);
				INT Yoff = appRound(FYoff);
				if( Abs(FYoff-Yoff) < SMALL_NUMBER )
				{
					INT MinY = Clamp<INT>( Yoff,						0, NumVerticesY-1 );
					INT MaxY = Clamp<INT>( Yoff + Other->NumVerticesY-1,0, NumVerticesY-1 );

					UBOOL Dirty = FALSE;

					// adjust heights
					for( INT y=MinY;y<=MaxY;y++ )
					{
						if( Height(NumVerticesX-1, y) != Other->Height(0, y-Yoff) )
						{
							Height(NumVerticesX-1, y) = Other->Height(0, y-Yoff);
							Dirty = TRUE;
						}
					}

					if( Dirty )
					{
						// update terrain
						UpdateRenderData(NumVerticesX-1,MinY,NumVerticesX-1,MaxY);
						UpdatePatchBounds(NumVerticesX-1,MinY,NumVerticesX-1,MaxY);
					}
				}
			}

			// check Above
			if( Abs(Other->Location.Y + (Other->NumPatchesY * Other->DrawScale * Other->DrawScale3D.Y) - Location.Y) < SMALL_NUMBER )
			{
				// get horizontal offset
				FLOAT FXoff = (Other->Location.X - Location.X) / (DrawScale * DrawScale3D.X);
				INT Xoff = appRound(FXoff);
				if( Abs(FXoff-Xoff) < SMALL_NUMBER )
				{
					INT MinX = Clamp<INT>( Xoff,						0, NumVerticesX-1 );
					INT MaxX = Clamp<INT>( Xoff + Other->NumVerticesX-1,0, NumVerticesX-1 );

					UBOOL Dirty = FALSE;

					// adjust heights
					for( INT x=MinX;x<=MaxX;x++ )
					{
						if( Height(x, 0) != Other->Height(x-Xoff, Other->NumVerticesY-1) )
						{
							Height(x, 0) = Other->Height(x-Xoff, Other->NumVerticesY-1);
							Dirty = TRUE;
						}
					}

					if( Dirty )
					{
						// update terrain
						UpdateRenderData(MinX,0,MaxX-1,0);
						UpdatePatchBounds(MinX,0,MaxX-1,0);
					}
				}
			}

			// check Below
			if( Abs(Location.Y + (NumPatchesY * DrawScale * DrawScale3D.Y) - Other->Location.Y) < SMALL_NUMBER )
			{
				// get horizontal offset
				FLOAT FXoff = (Other->Location.X - Location.X) / (DrawScale * DrawScale3D.X);
				INT Xoff = appRound(FXoff);
				if( Abs(FXoff-Xoff) < SMALL_NUMBER )
				{
					INT MinX = Clamp<INT>( Xoff,						0, NumVerticesX-1 );
					INT MaxX = Clamp<INT>( Xoff + Other->NumVerticesX-1,0, NumVerticesX-1 );

					UBOOL Dirty = FALSE;

					// adjust heights
					for( INT x=MinX;x<=MaxX;x++ )
					{
						if( Height(x, NumVerticesY-1) != Other->Height(x-Xoff, 0) )
						{
							Height(x, NumVerticesY-1) = Other->Height(x-Xoff, 0);
							Dirty = TRUE;
						}
					}

					if( Dirty )
					{
						// update terrain
						UpdateRenderData(MinX,NumVerticesY-1,MaxX,NumVerticesY-1);
						UpdatePatchBounds(MinX,NumVerticesY-1,MaxX,NumVerticesY-1);
					}
				}
			}
		}
	}
}

//
//	ATerrain::CompactAlphaMaps
//

void ATerrain::CompactAlphaMaps()
{
	// Build a list of referenced alpha maps.

	TArray<INT>		ReferencedAlphaMaps;
	for(UINT LayerIndex = 0;LayerIndex < (UINT)Layers.Num();LayerIndex++)
	{
		if(Layers(LayerIndex).AlphaMapIndex != INDEX_NONE)
		{
			ReferencedAlphaMaps.AddItem(Layers(LayerIndex).AlphaMapIndex);
		}
	}

	for(UINT DecoLayerIndex = 0;DecoLayerIndex < (UINT)DecoLayers.Num();DecoLayerIndex++)
	{
		if(DecoLayers(DecoLayerIndex).AlphaMapIndex != INDEX_NONE)
		{
			ReferencedAlphaMaps.AddItem(DecoLayers(DecoLayerIndex).AlphaMapIndex);
		}
	}

	// If there are any unused alpha maps, remove them and remap indices.

	if(ReferencedAlphaMaps.Num() != AlphaMaps.Num())
	{
		TArray<FAlphaMap>	OldAlphaMaps = AlphaMaps;
		TMap<INT,INT>		IndexMap;
		AlphaMaps.Empty(ReferencedAlphaMaps.Num());
		for(UINT AlphaMapIndex = 0;AlphaMapIndex < (UINT)ReferencedAlphaMaps.Num();AlphaMapIndex++)
		{
			new(AlphaMaps) FAlphaMap(OldAlphaMaps(ReferencedAlphaMaps(AlphaMapIndex)));
			IndexMap.Set(ReferencedAlphaMaps(AlphaMapIndex),AlphaMapIndex);
		}

		for(UINT LayerIndex = 0;LayerIndex < (UINT)Layers.Num();LayerIndex++)
		{
			if(Layers(LayerIndex).AlphaMapIndex != INDEX_NONE)
			{
				Layers(LayerIndex).AlphaMapIndex = IndexMap.FindRef(Layers(LayerIndex).AlphaMapIndex);
			}
		}

		for(UINT DecoLayerIndex = 0;DecoLayerIndex < (UINT)DecoLayers.Num();DecoLayerIndex++)
		{
			if(DecoLayers(DecoLayerIndex).AlphaMapIndex != INDEX_NONE)
			{
				DecoLayers(DecoLayerIndex).AlphaMapIndex = IndexMap.FindRef(DecoLayers(DecoLayerIndex).AlphaMapIndex);
			}
		}
	}

}

/** for each layer, calculate the rectangle encompassing all the vertices affected by it and store the result in
 * the layer's MinX, MinY, MaxX, and MaxY properties
 */
void ATerrain::CalcLayerBounds()
{
	// heightmap is always the whole thing
	if (Layers.Num() > 0)
	{
		Layers(0).MinX = 0;
		Layers(0).MinY = 0;
		Layers(0).MaxX = NumVerticesX - 1;
		Layers(0).MaxY = NumVerticesY - 1;
	}
	// calculate the box around the area for which the layer's alpha is greater than zero
	for (INT i = 1; i < Layers.Num(); i++)
	{
		if (Layers(i).AlphaMapIndex != INDEX_NONE)
		{
			Layers(i).MinX = NumVerticesX - 1;
			Layers(i).MinY = NumVerticesY - 1;
			Layers(i).MaxX = Layers(i).MaxY = 0;
			UBOOL bValid = false;
			for (INT x = 0; x < NumVerticesX; x++)
			{
				for (INT y = 0; y < NumVerticesY; y++)
				{
					if (Alpha(Layers(i).AlphaMapIndex, x, y) > 0)
					{
						Layers(i).MinX = Min<INT>(Layers(i).MinX, x);
						Layers(i).MinY = Min<INT>(Layers(i).MinY, y);
						Layers(i).MaxX = Max<INT>(Layers(i).MaxX, x);
						Layers(i).MaxY = Max<INT>(Layers(i).MaxY, y);
						bValid = true;
					}
				}
			}
			if (!bValid)
			{
				// this layer is not used anywhere - zero the bounds
				Layers(i).MinX = Layers(i).MinY = Layers(i).MaxX = Layers(i).MaxY = 0;
			}
		}
	}
}

//
//	FTerrainFilteredMaterial::BuildWeightMap
//

static FLOAT GetSlope(const FVector& A,const FVector& B)
{
	return Abs(B.Z - A.Z) * appInvSqrt(Square(B.X - A.X) + Square(B.Y - A.Y));
}

void FTerrainFilteredMaterial::BuildWeightMap(TArray<BYTE>& BaseWeightMap,
	UBOOL Highlighted, const FColor& InHighlightColor, UBOOL bInWireframeHighlighted, const FColor& InWireframeColor,
	class ATerrain* Terrain, class UTerrainLayerSetup* Layer, INT MinX, INT MinY, INT MaxX, INT MaxY) const
{
	if(!Material)
		return;

	UINT MaterialIndex;

	INT	SizeX = MaxX - MinX + 1,
		SizeY = MaxY - MinY + 1;

	// the stride into the raw data may need to be padded out to a power of two
	INT Stride = Terrain_GetPaddedSize(SizeX);

	check(BaseWeightMap.Num() == Stride * Terrain_GetPaddedSize(SizeY));

	// Filter the weightmap.

	TArray<BYTE>	MaterialWeightMap;
	MaterialWeightMap.Add(BaseWeightMap.Num());

	for(INT Y = MinY;Y <= MaxY;Y++)
	{
		BYTE*	BaseWeightPtr = &BaseWeightMap((Y - MinY) * Stride);
		BYTE*	MaterialWeightPtr = &MaterialWeightMap((Y - MinY) * Stride);
		for(INT X = MinX;X <= MaxX;X++,MaterialWeightPtr++,BaseWeightPtr++)
		{
			*MaterialWeightPtr = 0;
			if(*BaseWeightPtr)
			{
				FVector	Vertex = Terrain->GetWorldVertex(X, Y);
				if(MaxSlope.Enabled || MinSlope.Enabled)
				{
					FLOAT	Slope = Max(
										Max(
											GetSlope(Terrain->GetWorldVertex(X - 1,Y - 1),Vertex),
											Max(
												GetSlope(Terrain->GetWorldVertex(X,Y - 1),Vertex),
												GetSlope(Terrain->GetWorldVertex(X + 1,Y - 1),Vertex)
												)
											),
										Max(
											Max(
												GetSlope(Terrain->GetWorldVertex(X - 1,Y),Vertex),
												GetSlope(Terrain->GetWorldVertex(X + 1,Y),Vertex)
												),
											Max(
												GetSlope(Terrain->GetWorldVertex(X + 1,Y - 1),Vertex),
												Max(
													GetSlope(Terrain->GetWorldVertex(X + 1,Y),Vertex),
													GetSlope(Terrain->GetWorldVertex(X + 1,Y + 1),Vertex)
													)
												)
											)
										);
					if(MaxSlope.TestGreater(X,Y,Slope) || MinSlope.TestLess(X,Y,Slope))
						continue;
				}

				if(MaxHeight.Enabled || MinHeight.Enabled)
				{
					if(MaxHeight.TestGreater(X,Y,Vertex.Z) || MinHeight.TestLess(X,Y,Vertex.Z))
						continue;
				}

				if(UseNoise && FNoiseParameter(0.5f,NoiseScale,1.0f).TestLess(X,Y,NoisePercent))
					continue;

				*MaterialWeightPtr = Clamp<INT>(appTrunc(((FLOAT)*BaseWeightPtr * Layer->GetMaterialAlpha(this, Vertex))), 0, 255);
				*BaseWeightPtr -= *MaterialWeightPtr;
			}
		}
	}

	// Check for an existing weighted material to update.

	for(MaterialIndex = 0;MaterialIndex < (UINT)Terrain->WeightedMaterials.Num();MaterialIndex++)
	{
		FTerrainWeightedMaterial& WeightMap = Terrain->WeightedMaterials(MaterialIndex);
		if ((WeightMap.Material == Material) && (WeightMap.Highlighted == Highlighted))
		{
			for(INT Y = MinY;Y <= MaxY;Y++)
			{
				for(INT X = MinX;X <= MaxX;X++)
				{
					Terrain->WeightedMaterials(MaterialIndex).Data(
						Y * Terrain->WeightedMaterials(MaterialIndex).SizeX + X) += 
							MaterialWeightMap((Y - MinY) * Stride + X - MinX);
				}
			}
			return;
		}
	}

	// If generating the entire weightmap, create a new weighted material.

	check((MinX == 0) && (MaxX == Terrain->NumVerticesX - 1));
	check((MinY == 0) && (MaxY == Terrain->NumVerticesY - 1));

	FTerrainWeightedMaterial* NewWeightedMaterial = new(Terrain->WeightedMaterials) FTerrainWeightedMaterial(
		Terrain, MaterialWeightMap, Material, Highlighted, InHighlightColor,
		bInWireframeHighlighted, InWireframeColor);
	check(NewWeightedMaterial);
}

//
//	ATerrain::CacheWeightMaps
//

void ATerrain::CacheWeightMaps(INT MinX,INT MinY,INT MaxX,INT MaxY)
{
	INT	SizeX = (MaxX - MinX + 1),
		SizeY = (MaxY - MinY + 1);

	// the stride into the raw data may need to be padded out to a power of two
	INT Stride = Terrain_GetPaddedSize(SizeX),
		PaddedSizeX = Stride,
		PaddedSizeY = Terrain_GetPaddedSize(SizeY);

	// Clear the update rectangle in the weightmaps.

	//@todo.SAS. Needs to re-size the texture here!!!!
	for(UINT MaterialIndex = 0;MaterialIndex < (UINT)WeightedMaterials.Num();MaterialIndex++)
	{
		if(!WeightedMaterials(MaterialIndex).Data.Num())
		{
			check(MinX == 0 && MinY == 0 && MaxX == NumVerticesX - 1 && MaxY == NumVerticesY - 1);
			WeightedMaterials(MaterialIndex).Data.Add(PaddedSizeX * PaddedSizeY);
		}

		for(INT Y = MinY;Y <= MaxY;Y++)
		{
			for(INT X = MinX;X <= MaxX;X++)
			{
				WeightedMaterials(MaterialIndex).Data(Y * WeightedMaterials(MaterialIndex).SizeX + X) = 0;
			}
		}
	}

	// Build a base weightmap containing all texels set.

	TArray<BYTE>	BaseWeightMap(PaddedSizeX * PaddedSizeY);
	for(INT Y = MinY;Y <= MaxY;Y++)
	{
		for(INT X = MinX;X <= MaxX;X++)
		{
			BaseWeightMap((Y - MinY) * Stride + X - MinX) = 255;
		}
	}

	for(INT LayerIndex = Layers.Num() - 1;LayerIndex >= 0;LayerIndex--)
	{
		// Build a layer weightmap containing the texels set in the layer's alphamap and the base weightmap, removing the texels set in the layer weightmap from the base weightmap.

		TArray<BYTE>	LayerWeightMap(PaddedSizeX * PaddedSizeY);
		for(INT Y = MinY;Y <= MaxY;Y++)
		{
			for(INT X = MinX;X <= MaxX;X++)
			{
				FLOAT	LayerAlpha = LayerIndex ? ((FLOAT)Alpha(Layers(LayerIndex).AlphaMapIndex,X,Y) / 255.0f) : 1.0f;
				BYTE&	BaseWeight = BaseWeightMap((Y - MinY) * Stride + X - MinX);
				BYTE	Weight = (BYTE)Clamp(appTrunc((FLOAT)BaseWeight * LayerAlpha),0,255);

				LayerWeightMap((Y - MinY) * Stride + X - MinX) = Weight;
				BaseWeight -= Weight;
			}
		}

		// Generate weightmaps for each filtered material in the layer.  BuildWeightMap resets the texels in LayerWeightMap that it sets in the material weightmap.
		FTerrainLayer* Layer = &(Layers(LayerIndex));
		UTerrainLayerSetup* Setup = Layer->Setup;
		if (Setup && !Layer->Hidden)
		{
			for(UINT MaterialIndex = 0; MaterialIndex < (UINT)Setup->Materials.Num(); MaterialIndex++)
			{
				Layers(LayerIndex).Setup->Materials(MaterialIndex).BuildWeightMap(LayerWeightMap,
					Layer->Highlighted, Layer->HighlightColor, Layer->WireframeHighlighted, Layer->WireframeColor,
					this, Setup, MinX, MinY, MaxX, MaxY);
			}
		}

		// Add the texels set in the layer weightmap but not reset by the layer's filtered materials back into the base weightmap.

		for(INT Y = MinY;Y <= MaxY;Y++)
		{
			for(INT X = MinX;X <= MaxX;X++)
			{
				BaseWeightMap((Y - MinY) * Stride + X - MinX) += LayerWeightMap((Y - MinY) * Stride + X - MinX);
			}
		}
	}
}

//
//	ATerrain::UpdateRenderData
//

void ATerrain::UpdateRenderData(INT MinX,INT MinY,INT MaxX,INT MaxY)
{
	// Let the rendering thread 'catch up'
	FlushRenderingCommands();

	INT	SizeX = (MaxX - MinX + 1),
		SizeY = (MaxY - MinY + 1);

	// Generate the weightmaps.

	CacheWeightMaps(MinX,MinY,MaxX,MaxY);
	TouchWeightMapResources();

	// Cache displacements.
	CacheDisplacements(Max(MinX - 1,0),Max(MinY - 1,0),MaxX,MaxY);

	// Cache decorations.

	CacheDecorations(Max(MinX - 1,0),Max(MinY - 1,0),MaxX,MaxY);

	// Update the terrain components.

	for(UINT ComponentIndex = 0;ComponentIndex < (UINT)TerrainComponents.Num();ComponentIndex++)
	{
		UTerrainComponent*	Component = TerrainComponents(ComponentIndex);
		if (Component	&&
			((Component->SectionBaseX + Component->TrueSectionSizeX) >= MinX) && 
			(Component->SectionBaseX <= MaxX) &&
			((Component->SectionBaseY + Component->TrueSectionSizeY) >= MinY) && 
			(Component->SectionBaseY <= MaxY)
			)
		{
			// Discard any lightmap cached for the component.

			((UActorComponent*)Component)->InvalidateLightingCache();

			// Reinsert the component in the scene and update the vertex buffer.
			// THIS IS DONE IN THE InvalidateLightingCache FUNCTION!
			//FComponentReattachContext ReattachContext(Component);
		}
	}

}

// Hash function. Needed to avoid UObject v FResource ambiguity due to multiple inheritance
static inline DWORD GetTypeHash( const UTexture2D* Texture )
{
	return Texture ? Texture->GetIndex() : 0;
}

//
//	ATerrain::CacheDisplacements
//

void ATerrain::CacheDisplacements(INT MinX,INT MinY,INT MaxX,INT MaxY)
{
	if ((MinX == 0) && (MinY == 0) && (MaxX >= (NumVerticesX - 1)) && (MaxY >= (NumVerticesY - 1)))
	{
		CachedDisplacements.Empty(NumVerticesX * NumVerticesY);
		CachedDisplacements.Add(NumVerticesX * NumVerticesY);
	}

	// Build a list of materials with displacement maps and calculate the maximum possible displacement.

	MaxCollisionDisplacement = 0.0f;

	// We can't lock textures with cooked content so we only do it in the Editor to avoid bugs cooked vs. uncooked and rather live with Editor vs. game.
	if( GIsEditor )
	{
		TArray<UINT>	DisplacedMaterials;
		for(UINT MaterialIndex = 0;MaterialIndex < (UINT)WeightedMaterials.Num();MaterialIndex++)
		{
			FTerrainWeightedMaterial& WeightedMaterial = WeightedMaterials(MaterialIndex);
			if(WeightedMaterial.Material->DisplacementMap)
			{
				MaxCollisionDisplacement = Max(
					MaxCollisionDisplacement,
					Max(Abs(WeightedMaterial.Material->DisplacementMap->UnpackMin[3]),Abs(WeightedMaterial.Material->DisplacementMap->UnpackMax[3])) *
						WeightedMaterial.Material->DisplacementScale
					);
				DisplacedMaterials.AddItem(MaterialIndex);
			}
		}

		// Lock all displacement maps used by materials.
		TMap<UTexture2D*,BYTE*> WeightedMaterialDisplacementDataMap;
		for(UINT MaterialIndex = 0;MaterialIndex < (UINT)DisplacedMaterials.Num();MaterialIndex++)
		{
			const FTerrainWeightedMaterial&	WeightedMaterial = WeightedMaterials(DisplacedMaterials(MaterialIndex));
			if( WeightedMaterial.Material && WeightedMaterial.Material->DisplacementMap )
			{
				if( WeightedMaterialDisplacementDataMap.Find( WeightedMaterial.Material->DisplacementMap ) == NULL )
				{
					BYTE* LockedData = (BYTE*) WeightedMaterial.Material->DisplacementMap->Mips(0).Data.Lock(LOCK_READ_ONLY);
					WeightedMaterialDisplacementDataMap.Set( WeightedMaterial.Material->DisplacementMap, LockedData );
				}
			}
		}

		for (INT VertexY = MinY; VertexY <= MaxY; VertexY++)
		{
			for (INT VertexX = MinX; VertexX <= MaxX; VertexX++)
			{
				FLOAT	Displacement = 0.0f;

				for(UINT MaterialIndex = 0;MaterialIndex < (UINT)DisplacedMaterials.Num();MaterialIndex++)
				{
					// Compute the weight of this material on the current vertex.

					const FTerrainWeightedMaterial&	WeightedMaterial = WeightedMaterials(DisplacedMaterials(MaterialIndex));
					FLOAT Weight = WeightedMaterial.Weight(VertexX,VertexY);

					if ( (Weight > 0.0f) && (WeightedMaterial.Material))
					{
						// Sample the material's displacement map.
						FLOAT VX = (FLOAT)VertexX;
						FLOAT VY = (FLOAT)VertexY;
						FVector	UV = WeightedMaterial.Material->LocalToMapping.TransformFVector(FVector(VX,VY,0));
						BYTE* LockedData = WeightedMaterialDisplacementDataMap.FindRef( WeightedMaterial.Material->DisplacementMap );
						check(LockedData);
						Displacement += Weight / 255.0f * WeightedMaterial.Material->GetDisplacement(LockedData,UV.X,UV.Y);
					}
				}

				// Quantize and store the displacement for this vertex.
				CachedDisplacements(VertexY * NumVerticesX + VertexX) = Clamp<INT>(appTrunc(Displacement * 127.0f / MaxCollisionDisplacement),-127,128) + 127;
			}
		}

		// Unlock all previously locked textures.
		for (TMap<UTexture2D*,BYTE*>::TIterator It(WeightedMaterialDisplacementDataMap); It; ++It)
		{
			UTexture2D* Texture = It.Key();
			Texture->Mips(0).Data.Unlock();
		}
	}
}

//
//	ATerrain::CacheDecorations
//

void ATerrain::CacheDecorations(INT MinX,INT MinY,INT MaxX,INT MaxY)
{
	for(UINT DecoLayerIndex = 0;DecoLayerIndex < (UINT)DecoLayers.Num();DecoLayerIndex++)
	{
		FTerrainDecoLayer&	DecoLayer = DecoLayers(DecoLayerIndex);
		for(UINT DecorationIndex = 0;DecorationIndex < (UINT)DecoLayers(DecoLayerIndex).Decorations.Num();DecorationIndex++)
		{
			FTerrainDecoration&	Decoration = DecoLayers(DecoLayerIndex).Decorations(DecorationIndex);

			// Clear old decorations in the update rectangle.

			for(INT InstanceIndex = 0;InstanceIndex < Decoration.Instances.Num();InstanceIndex++)
			{
				FTerrainDecorationInstance&	DecorationInstance = Decoration.Instances(InstanceIndex);
				INT X = appTrunc(DecorationInstance.X);
				INT Y = appTrunc(DecorationInstance.Y);
				if ((X < MinX) || (X > MaxX) || (Y < MinY) || (Y > MaxY))
				{
					continue;
				}
				// Remove from Components array
				if (DecorationInstance.Component)
				{
					Components.RemoveItem(DecorationInstance.Component);
					DecorationInstance.Component->ConditionalDetach();
				}

				Decoration.Instances.Remove(InstanceIndex--);
			}

			// Create new decorations.

			if(Decoration.Factory && Decoration.Factory->FactoryIsValid())
			{
				INT NumQuadsX = NumPatchesX / MaxTesselationLevel;
				INT NumQuadsY = NumPatchesY / MaxTesselationLevel;
				UINT MaxInstances = Max<UINT>(appTrunc(Decoration.Density * NumQuadsX * NumQuadsY),0);
				const FMatrix&	ActorToWorld = LocalToWorld();

				appSRandInit(Decoration.RandSeed);

				for(UINT InstanceIndex = 0;InstanceIndex < MaxInstances;InstanceIndex++)
				{
					FLOAT	X = appSRand() * NumVerticesX,
							Y = appSRand() * NumVerticesY,
							Scale = appSRand();
					INT		IntX = appTrunc(X),
							IntY = appTrunc(Y),
							Yaw = appTrunc(appSRand() * 65536.0f);

					if (IsTerrainQuadVisible(IntX, IntY) == TRUE)
					{
						if (((appSRand() * 255.0f) <= Alpha(DecoLayer.AlphaMapIndex,IntX,IntY)) && 
							(IntX >= MinX) && (IntX <= MaxX) && (IntY >= MinY) && (IntY <= MaxY))
						{
							FTerrainDecorationInstance*	DecorationInstance = new(Decoration.Instances) FTerrainDecorationInstance;
							DecorationInstance->Component = Decoration.Factory->CreatePrimitiveComponent(this);
							DecorationInstance->X = X;
							DecorationInstance->Y = Y;
							DecorationInstance->Yaw = Yaw;
							DecorationInstance->Scale = Lerp(Decoration.MinScale,Decoration.MaxScale,Scale);
							FVector TotalScale = DrawScale3D * DrawScale;
							DecorationInstance->Component->Scale3D = DecorationInstance->Scale * FVector(1.f/TotalScale.X, 1.f/TotalScale.Y, 1.f/TotalScale.Z);

							// Add to components array
							INT				SubX = 0;
							INT				SubY = 0;
							FTerrainPatch	Patch = GetPatch(IntX, IntY);
							FVector			Location = ActorToWorld.TransformFVector(GetCollisionVertex(Patch, IntX, IntY, SubX, SubY, MaxTesselationLevel));
							FRotator		Rotation(0, Yaw, 0);
							FVector			TangentX = FVector(1, 0, GCollisionPatchSampler.SampleDerivX(Patch, SubX, SubY) * TERRAIN_ZSCALE),
								TangentY = FVector(0 ,1, GCollisionPatchSampler.SampleDerivY(Patch, SubX, SubY) * TERRAIN_ZSCALE);

							const FMatrix& DecorationToWorld = FRotationMatrix(Rotation) *
								FRotationMatrix(
								Lerp(
								FVector(ActorToWorld.TransformNormal(FVector(1,0,0)).SafeNormal()),
								(TangentY ^ (TangentX ^ TangentY)).SafeNormal(),
								Clamp(Decoration.SlopeRotationBlend, 0.0f, 1.0f)
								).SafeNormal().Rotation()
								) *
								FScaleMatrix(DrawScale * DrawScale3D) *
								FTranslationMatrix(Location);

							DecorationInstance->Component->UpdateComponent(GWorld->Scene,this,DecorationToWorld);
						}
					}
				}
			}
		}
	}
}

/**
 *	Returns TRUE is the component at the given X,Y has ANY patches contained in it visible.
 */	
UBOOL ATerrain::IsTerrainComponentVisible(INT InBaseX, INT InBaseY, INT InSizeX, INT InSizeY)
{
	for (INT Y = InBaseY; Y < InBaseY + InSizeY; Y++)
	{
		for (INT X = InBaseX; X < InBaseX + InSizeX; X++)
		{
            FTerrainInfoData* InfoData = GetInfoData(X, Y);
			if (InfoData)
			{
				// If even a single patch in the component is visible, return TRUE!
				if (InfoData->IsVisible() == TRUE)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

UBOOL ATerrain::IsTerrainComponentVisible(UTerrainComponent* InComponent)
{
	for (INT Y = InComponent->SectionBaseY; Y < InComponent->SectionBaseY + (InComponent->SectionSizeY * MaxTesselationLevel); Y++)
	{
		for (INT X = InComponent->SectionBaseX; X < InComponent->SectionBaseX + (InComponent->SectionSizeX * MaxTesselationLevel); X++)
		{
            FTerrainInfoData* InfoData = GetInfoData(X, Y);
			if (InfoData)
			{
				// If even a single patch in the component is visible, return TRUE!
				if (InfoData->IsVisible() == TRUE)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

//
//	ATerrain::GetLocalVertex
//

FVector ATerrain::GetLocalVertex(INT X,INT Y) const
{
	return FVector(X,Y,(-32768.0f + (FLOAT)Height(X,Y)) * TERRAIN_ZSCALE);
}

//
//	ATerrain::GetWorldVertex
//

FVector ATerrain::GetWorldVertex(INT X,INT Y) const
{
	return LocalToWorld().TransformFVector(GetLocalVertex(X,Y));
}

//
//	ATerrain::GetPatch
//

FTerrainPatch ATerrain::GetPatch(INT X,INT Y) const
{
	FTerrainPatch	Result;

	for(INT SubY = 0;SubY < 4;SubY++)
	{
		for(INT SubX = 0;SubX < 4;SubX++)
		{
			Result.Heights[SubX][SubY] = Height(X - 1 + SubX,Y - 1 + SubY);
		}
	}

	return Result;
}

//
//	ATerrain::GetCollisionVertex
//

FVector ATerrain::GetCollisionVertex(const FTerrainPatch& Patch,UINT PatchX,UINT PatchY,UINT SubX,UINT SubY,UINT TesselationLevel) const
{
	FLOAT	FracX = (FLOAT)SubX / (FLOAT)TesselationLevel,
			FracY = (FLOAT)SubY / (FLOAT)TesselationLevel;

	FLOAT SampleDeriveX = GCollisionPatchSampler.SampleDerivX(Patch,SubX,SubY);
	FLOAT SampleDeriveY = GCollisionPatchSampler.SampleDerivY(Patch,SubX,SubY);
	const FVector&	TangentZ = (FVector(1,0,SampleDeriveX * TERRAIN_ZSCALE) ^ FVector(0,1,SampleDeriveY * TERRAIN_ZSCALE)).UnsafeNormal();
	FLOAT			Displacement = GetCachedDisplacement(PatchX,PatchY,SubX,SubY);

	return FVector(
		PatchX + FracX,
		PatchY + FracY,
		(-32768.0f +
			GCollisionPatchSampler.Sample(
				Patch,
				SubX * TERRAIN_MAXTESSELATION / TesselationLevel,
				SubY * TERRAIN_MAXTESSELATION / TesselationLevel
				)
			) *
			TERRAIN_ZSCALE
		) +
			TangentZ * Displacement;
}

//
//	ATerrain::Alpha
//

const BYTE ATerrain::Alpha(INT AlphaMapIndex,INT X,INT Y) const
{
	if(AlphaMapIndex == INDEX_NONE)
		return 0;

	check(AlphaMapIndex >= 0 && AlphaMapIndex < AlphaMaps.Num());

	X = Clamp(X,0,NumVerticesX - 1);
	Y = Clamp(Y,0,NumVerticesY - 1);

	return AlphaMaps(AlphaMapIndex).Data(Y * NumVerticesX + X);
}

//
//	ATerrain::Alpha
//

BYTE& ATerrain::Alpha(INT& AlphaMapIndex,INT X,INT Y)
{
	if(AlphaMapIndex == INDEX_NONE)
	{
		AlphaMapIndex = AlphaMaps.Num();
		(new(AlphaMaps) FAlphaMap)->Data.AddZeroed(NumVerticesX * NumVerticesY);
	}

	check(AlphaMapIndex >= 0 && AlphaMapIndex < AlphaMaps.Num());

	X = Clamp(X,0,NumVerticesX - 1);
	Y = Clamp(Y,0,NumVerticesY - 1);

	return AlphaMaps(AlphaMapIndex).Data(Y * NumVerticesX + X);
}

//
//	ATerrain::GetCachedDisplacement
//

FLOAT ATerrain::GetCachedDisplacement(INT X,INT Y,INT SubX,INT SubY) const
{
	FLOAT fValue;
	INT	Index = (Y + SubY) * (NumPatchesX + 1) + (X + SubX);

	if ((Index < 0) || (Index >= CachedDisplacements.Num()))
	{
#if defined(_TERRAIN_WARN_INVALID_CACHED_DISPLACEMENTS_)
		if (GIsEditor)
		{
			warnf(NAME_Warning, TEXT("Invalid CachedDisplacements index - %d, max %d - TessLevel %d, X %d, Y %d, SubX %d, SubY %d"), 
				Index, CachedDisplacements.Num(), MaxTesselationLevel, X, Y, SubX, SubY);
		}
#endif	//#if defined(_TERRAIN_WARN_INVALID_CACHED_DISPLACEMENTS_)
		fValue = 0.0f;
	}
	else
	{
		INT		PackedDisplacement = CachedDisplacements(Index);
		fValue = (FLOAT)(PackedDisplacement - 127) / 127.0f * MaxCollisionDisplacement;
	}
	return fValue;
}

/**
 *	BuildCollisionData
 *
 *	Helper function to force the re-building of the collision date.
 */
void ATerrain::BuildCollisionData()
{
	check(IsInGameThread() == TRUE);

	// wait until resources are released
	FlushRenderingCommands();

	// Build the collision data for each comp
	for (INT CompIndex = 0; CompIndex < TerrainComponents.Num(); CompIndex++)
	{
		UTerrainComponent* Component = TerrainComponents(CompIndex);
		if (Component)
		{
			Component->BuildCollisionData();
		}
	}

	// Detach/re-attach all components...
	for (INT CompIndex = 0; CompIndex < TerrainComponents.Num(); CompIndex++)
	{
		UTerrainComponent* Component = TerrainComponents(CompIndex);
		if (Component)
		{
			Component->ConditionalDetach();
		}
	}

	ConditionalUpdateComponents(FALSE);
}

/**
 *	RecacheMaterials
 *
 *	Helper function that tosses the cached materials and regenerates them.
 */
void ATerrain::RecacheMaterials()
{
	check(IsInGameThread() == TRUE);

	// wait until resources are released
	FlushRenderingCommands();

	//@.HACK. This will forcibly recache all shaders used for this terrain
	ClearCachedTerrainMaterials();

	// Clear and reset weight map textures.
	ClearWeightMaps();
	CacheWeightMaps(0, 0, NumVerticesX - 1, NumVerticesY - 1);
	TouchWeightMapResources();

	// Detach/re-attach all components...
	for (INT CompIndex = 0; CompIndex < TerrainComponents.Num(); CompIndex++)
	{
		UTerrainComponent* Component = TerrainComponents(CompIndex);
		if (Component)
		{
			Component->ConditionalDetach();
		}
	}

	ConditionalUpdateComponents(FALSE);

	MarkPackageDirty();
}

/**
 *	UpdateLayerSetup
 *
 *	Editor function for updating altered materials/layers
 *
 *	@param	InSetup		The layer setup to update.
 */
void ATerrain::UpdateLayerSetup(UTerrainLayerSetup* InSetup)
{
	if (InSetup == NULL)
	{
		return;
	}

	// Find all the cached shaders that contain the materials in the setup
	for (INT MaterialIndex = 0; MaterialIndex < InSetup->Materials.Num(); MaterialIndex++)
	{
		FTerrainFilteredMaterial* TFMat = &(InSetup->Materials(MaterialIndex));
		if (TFMat)
		{
			UTerrainMaterial* TMat = TFMat->Material;
			if (TMat)
			{
				UpdateTerrainMaterial(TMat);
			}
		}
	}
}

/**
 *	RemoveLayerSetup
 *
 *	Editor function for removing altered materials/layers
 *
 *	@param	InSetup		The layer setup to Remove.
 */
void ATerrain::RemoveLayerSetup(UTerrainLayerSetup* InSetup)
{
	if (InSetup == NULL)
	{
		return;
	}

	// Find all the cached shaders that contain the materials in the setup
	for (INT MaterialIndex = 0; MaterialIndex < InSetup->Materials.Num(); MaterialIndex++)
	{
		FTerrainFilteredMaterial* TFMat = &(InSetup->Materials(MaterialIndex));
		if (TFMat)
		{
			UTerrainMaterial* TMat = TFMat->Material;
			if (TMat)
			{
				RemoveTerrainMaterial(TMat);
			}
		}
	}
}

/**
 *	UpdateTerrainMaterial
 *
 *	Editor function for updating altered materials/layers
 *
 *	@param	InTMat		The terrain material to update.
 */
void ATerrain::UpdateTerrainMaterial(UTerrainMaterial* InTMat)
{
	if (InTMat == NULL)
	{
		return;
	}

	// Just update the material it points to, as this will recompile shaders for any
	// terrain material changes.
	if (InTMat->Material)
	{
		UpdateCachedMaterial(InTMat->Material->GetMaterial());
	}
}

/**
 *	RemoveTerrainMaterial
 *
 *	Editor function for removing altered materials/layers
 *
 *	@param	InTMat		The terrain material to Remove.
 */
void ATerrain::RemoveTerrainMaterial(UTerrainMaterial* InTMat)
{
	if (InTMat == NULL)
	{
		return;
	}

	// Just update the material it points to, as this will recompile shaders for any
	// terrain material changes.
	if (InTMat->Material)
	{
		RemoveCachedMaterial(InTMat->Material->GetMaterial());
	}
}

/**
 *	UpdateMaterialInstance
 *
 *	Editor function for updating altered materials/layers
 *
 *	@param	InMatInst	The material instance to update.
 */
void ATerrain::UpdateMaterialInstance(UMaterialInterface* InMatInst)
{
}
	
/**
 *	UpdateCachedMaterial
 *
 *	Editor function for updating altered materials/layers
 *
 *	@param	InMat		The material instance to update.
 */
void ATerrain::UpdateCachedMaterial(UMaterial* InMat)
{	
	// We don't want to bother updating materials for platforms other than the current, so instead we'll
	// just delete those cached materials and they'll be regenerated when the level is saved (ATerrain:PreSave)
	if( GIsEditor && !GIsUCC )
	{
		const UBOOL bOtherShaderPlatformsOnly = TRUE;
		ClearCachedTerrainMaterials( bOtherShaderPlatformsOnly );
	}

	// only update cached material entries for the currently running platform
	const EMaterialShaderPlatform MatPlatform = GetMaterialPlatform(GRHIShaderPlatform);
	TArrayNoInit<FTerrainMaterialResource*>& CachedMaterials = GetCachedTerrainMaterials(MatPlatform);
	if( CachedMaterials.Num() == 0 || InMat == NULL )
	{
		// No cached materials? This shouldn't happen, but if so, quick out.
		return;
	}

	// Check each layer
	for (INT LayerIndex = 0; LayerIndex < Layers.Num(); LayerIndex++)
	{
		FTerrainLayer* Layer = &(Layers(LayerIndex));
		UTerrainLayerSetup* Setup = Layer->Setup;
		if (Setup != NULL)
		{
			for (INT MaterialIndex = 0; MaterialIndex < Setup->Materials.Num(); MaterialIndex++)
			{
				FTerrainFilteredMaterial* FilteredMaterial = &(Setup->Materials(MaterialIndex));
				UTerrainMaterial* TerrainMaterial = FilteredMaterial->Material;
				if( TerrainMaterial && 
					TerrainMaterial->Material &&
					TerrainMaterial->Material->GetMaterial() == InMat )
				{
					for (INT CachedIndex = 0; CachedIndex < CachedMaterials.Num(); CachedIndex++)
					{
						FTerrainMaterialResource* CachedMaterial = CachedMaterials(CachedIndex);
						if (CachedMaterial)
						{
							const FTerrainMaterialMask& CheckMask = CachedMaterial->GetMask();
							for (INT CachedMaterialIndex = 0; CachedMaterialIndex < CheckMask.Num(); CachedMaterialIndex++)
							{
								if( CheckMask.Get(CachedMaterialIndex) && CachedMaterialIndex < WeightedMaterials.Num() )
								{
									FTerrainWeightedMaterial* CheckWeightedMaterial = &WeightedMaterials(CachedMaterialIndex);
									if( CheckWeightedMaterial && 
										CheckWeightedMaterial->Material && 
										CheckWeightedMaterial->Material->Material &&
										CheckWeightedMaterial->Material->Material->GetMaterial() == InMat )
									{
										CachedMaterial->CacheShaders();
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
	
/**
 *	RemoveCachedMaterial
 *
 *	Editor function for removing altered materials/layers
 *
 *	@param	InMat		The material instance to remove.
 */
void ATerrain::RemoveCachedMaterial(UMaterial* InMat)
{
	// We don't want to bother updating materials for platforms other than the current, so instead we'll
	// just delete those cached materials and they'll be regenerated when the level is saved (ATerrain:PreSave)
	if( GIsEditor && !GIsUCC )
	{
		const UBOOL bOtherShaderPlatformsOnly = TRUE;
		ClearCachedTerrainMaterials( bOtherShaderPlatformsOnly );
	}

	// only remove cached material entries for the currently running platform
	const EMaterialShaderPlatform MatPlatform = GetMaterialPlatform(GRHIShaderPlatform);
	TArrayNoInit<FTerrainMaterialResource*>& CachedMaterials = GetCachedTerrainMaterials(MatPlatform);
	if( CachedMaterials.Num() == 0 || InMat == NULL )
	{
		// No cached materials? This shouldn't happen, but if so, quick out.
		return;
	}

	// Check each layer
	for (INT LayerIndex = 0; LayerIndex < Layers.Num(); LayerIndex++)
	{
		FTerrainLayer* Layer = &(Layers(LayerIndex));
		UTerrainLayerSetup* Setup = Layer->Setup;
		if (Setup != NULL)
		{
			for (INT MaterialIndex = 0; MaterialIndex < Setup->Materials.Num(); MaterialIndex++)
			{
				FTerrainFilteredMaterial* FilteredMaterial = &(Setup->Materials(MaterialIndex));
				UTerrainMaterial* TerrainMaterial = FilteredMaterial->Material;
				if( TerrainMaterial &&
					TerrainMaterial->Material &&
					TerrainMaterial->Material->GetMaterial() == InMat )
				{
					for (INT CachedIndex = 0; CachedIndex < CachedMaterials.Num(); CachedIndex++)
					{
						FTerrainMaterialResource* CachedMaterial = CachedMaterials(CachedIndex);
						if (CachedMaterial)
						{
							const FTerrainMaterialMask& CheckMask = CachedMaterial->GetMask();
							for (INT CachedMaterialIndex = 0; CachedMaterialIndex < CheckMask.Num(); CachedMaterialIndex++)
							{
								if( CheckMask.Get(CachedMaterialIndex) && 
									CachedMaterialIndex < WeightedMaterials.Num() )
								{
									FTerrainWeightedMaterial* CheckWeightedMaterial = &(WeightedMaterials(CachedMaterialIndex));
									if( CheckWeightedMaterial &&
										CheckWeightedMaterial->Material &&
										CheckWeightedMaterial->Material->Material &&
										CheckWeightedMaterial->Material->Material->GetMaterial() == InMat )
									{
										delete CachedMaterial;
										CachedMaterials(CachedIndex) = NULL;
										//Remove from the array to prevent crashes on serialization!
										CachedMaterials.Remove(CachedIndex);
										//account for the iteration of the loop by decrementing CachedIndex
										--CachedIndex; 
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

/**
 *	TessellateTerrainUp
 *
 *	Editor function for converting old terrain to the new hi-res model
 */
UBOOL ATerrain::TessellateTerrainUp(INT InTessellationlevel, UBOOL bRegenerateComponents)
{
	ClearComponents();

	debugf(TEXT("OLD TERRAIN - Must convert it!"));
	debugf(TEXT("\tPatches....................%4d x %4d"), NumPatchesX, NumPatchesY);
	debugf(TEXT("\tMin/MaxTessellation........%4d x %4d"), MinTessellationLevel, MaxTesselationLevel);
	debugf(TEXT("\tCollisionTessellation......%4d"), CollisionTesselationLevel);
	debugf(TEXT("\tScale......................%8.4f - %8.4f,%8.4f,%8.4f"), DrawScale, DrawScale3D.X, DrawScale3D.Y, DrawScale3D.Z);

	INT OldMaxTesselationLevel = MaxTesselationLevel;
	MaxTesselationLevel = InTessellationlevel;
	MinTessellationLevel = Min(MinTessellationLevel,MaxTesselationLevel);

	// Determine the new required patches and draw scale
	INT NewNumPatchesX = NumPatchesX * MaxTesselationLevel;
	INT NewNumPatchesY = NumPatchesY * MaxTesselationLevel;
	// We need to clamp the patches to MaxDetessellationLevel
	if ((NewNumPatchesX % MaxTesselationLevel) > 0)
	{
		NewNumPatchesX += MaxTesselationLevel - (NewNumPatchesX % MaxTesselationLevel);
	}
	if ((NewNumPatchesY % MaxTesselationLevel) > 0)
	{
		NewNumPatchesY += MaxTesselationLevel - (NewNumPatchesY % MaxTesselationLevel);
	}
	// Limit MaxComponentSize from being 0, negative or large enough to exceed the maximum vertex buffer size.
	MaxComponentSize = Clamp(MaxComponentSize,1,((255 / MaxTesselationLevel) - 1));

	if (DrawScale != 1.0f)
	{
		debugf(TEXT("\t\tDrawScale is not 1.0... putting into DrawScale3D"));
		// Feed the DrawScale into the DrawScale3D
		DrawScale3D *= DrawScale;
		// Set the DrawScale to 1.0
		DrawScale = 1.0f;
	}
	FVector NewDrawScale3D = FVector(DrawScale3D.X/MaxTesselationLevel, DrawScale3D.Y/MaxTesselationLevel, DrawScale3D.Z);
	INT NewCollisionTesselationLevel = MaxTesselationLevel / CollisionTesselationLevel;
	// Prevent it from going to 0
	NewCollisionTesselationLevel = Max(1, NewCollisionTesselationLevel);

	debugf(TEXT("\tNew Patch Count............%4d x %4d"), NewNumPatchesX, NewNumPatchesY);
	debugf(TEXT("\tNewCollisionTessellation...%4d"), NewCollisionTesselationLevel);
	debugf(TEXT("\tNewScale......................%8.4f - %8.4f,%8.4f,%8.4f"), DrawScale, NewDrawScale3D.X, NewDrawScale3D.Y, NewDrawScale3D.Z);

	// Calculate the new NumVertices
	INT NewNumVerticesX = NewNumPatchesX + 1;
	INT NewNumVerticesY = NewNumPatchesY + 1;

	if ((NewNumVerticesX != NumVerticesX) ||
		(NewNumVerticesY != NumVerticesY))
	{
		// Touch all the resources...
		for (INT LayerIndex = 0; LayerIndex < Layers.Num(); LayerIndex++)
		{
			FTerrainLayer& Layer = Layers(LayerIndex);
			UTerrainLayerSetup* Setup = Layer.Setup;
			if (Setup)
			{
				// Touch the mapping scale on terrain materials...
				for (INT MaterialIndex = 0; MaterialIndex < Setup->Materials.Num(); MaterialIndex++)
				{
					FTerrainFilteredMaterial& TFilteredMat = Setup->Materials(MaterialIndex);
					UTerrainMaterial* TMat = TFilteredMat.Material;
					if (TMat)
					{
						debugf(TEXT("Make sure to check the mapping scale on %s. Likely change: Increase by factor of 2 (tess change = %d)."), 
							*(TMat->GetPathName()), MaxTesselationLevel);
					}
				}
			}
		}

		// Setup the heights and infodata
		TArray<FTerrainHeight> NewHeights;
		TArray<FTerrainInfoData> NewInfoData;

		INT TotalVertices = NewNumVerticesX * NewNumVerticesY;
		NewHeights.Empty(TotalVertices);
		NewInfoData.Empty(TotalVertices);

		FPatchSampler PatchSampler(MaxTesselationLevel);

		// Convert the heights...
		for (INT Y = 0; Y < NumVerticesY; Y++)
		{
			for (INT SmoothY = 0; SmoothY < MaxTesselationLevel; SmoothY++)
			{
				INT NewYPos = Y * MaxTesselationLevel + SmoothY;
				if (NewYPos >= NewNumVerticesY)
				{
					// Make sure we don't tessellate the edges
					continue;
				}

				for (INT X = 0; X < NumVerticesX; X++)
				{
					const FTerrainPatch& Patch = GetPatch(X, Y);

					for (INT SmoothX = 0; SmoothX < MaxTesselationLevel; SmoothX++)
					{
						INT NewXPos = X * MaxTesselationLevel + SmoothX;
						if (NewXPos >= NewNumVerticesX)
						{
							// Make sure we don't tessellate the edges
							continue;
						}

						WORD Z = (WORD)appTrunc(PatchSampler.Sample(Patch,SmoothX,SmoothY));
						new(NewHeights) FTerrainHeight(Z);
					}
				}
			}
		}

		// Convert the infodata
		for (INT Y = 0; Y < NumVerticesY; Y++)
		{
			// Subdivide each Y patch by tesselation level
			for (INT SubY = 0; SubY < MaxTesselationLevel; SubY++)
			{
				INT NewYPos = Y * MaxTesselationLevel + SubY;
				if (NewYPos >= NewNumVerticesY)
				{
					// Make sure we don't tessellate the edges
					continue;
				}

				for (INT X = 0; X < NumVerticesX; X++)
				{
					// Get the 'current' info data entry
					FTerrainInfoData* InfoData = GetInfoData(X, Y);
					check(InfoData);

					// Subdivide each X patch by tesselation level
					for (INT SubX = 0; SubX < MaxTesselationLevel; SubX++)
					{
						INT NewXPos = X * MaxTesselationLevel + SubX;
						if (NewXPos >= NewNumVerticesX)
						{
							// Make sure we don't tessellate the edges
							continue;
						}

						new(NewInfoData) FTerrainInfoData(InfoData->Data);
					}
				}
			}
		}

		// Convert the alpha maps
		// This will cover deco layers as well...
		for (INT AlphaMapIndex = 0; AlphaMapIndex < AlphaMaps.Num(); AlphaMapIndex++)
		{
			FAlphaMap* AlphaMap = &AlphaMaps(AlphaMapIndex);
			TArray<BYTE> NewAlphas;
			NewAlphas.Empty(TotalVertices);

			FTerrainPatch SamplePatch;
			for (INT Y = 0; Y < NumVerticesY; Y++)
			{
				for (INT SubY = 0; SubY < ((Y < (NumVerticesY - 1)) ? MaxTesselationLevel : 1); SubY++)
				{
					for (INT X = 0; X < NumVerticesX; X++)
					{
						for (INT FillY = 0; FillY < 4; FillY++)
						{
							for (INT FillX = 0; FillX < 4; FillX++)
							{
								INT InnerX = Clamp<INT>(X - 1 + FillX, 0, NumVerticesX - 1);
								INT InnerY = Clamp<INT>(Y - 1 + FillY, 0, NumVerticesY - 1);
								SamplePatch.Heights[FillX][FillY] = AlphaMap->Data(InnerY * NumVerticesX + InnerX);
							}
						}
						for (INT SubX = 0; SubX < ((X < (NumVerticesX - 1)) ? MaxTesselationLevel : 1); SubX++)
						{
							FLOAT Value = PatchSampler.Sample(SamplePatch, SubX, SubY);
							BYTE NewValue = 0;
							Value = Clamp<FLOAT>(Value, 0.0f, 255.0f);
							NewValue = (BYTE)(appTrunc(Value));
							new(NewAlphas)BYTE(NewValue);
						}
					}
				}
			}

			AlphaMaps(AlphaMapIndex).Data.Empty(NewAlphas.Num());
			AlphaMaps(AlphaMapIndex).Data.Add(NewAlphas.Num());
			appMemcpy(&AlphaMaps(AlphaMapIndex).Data(0),&NewAlphas(0),NewAlphas.Num());
		}

		// Copy them...
		Heights.Empty(NewHeights.Num());
		Heights.Add(NewHeights.Num());
		appMemcpy(&Heights(0),&NewHeights(0),NewHeights.Num() * sizeof(FTerrainHeight));

		InfoData.Empty(NewInfoData.Num());
		InfoData.Add(NewInfoData.Num());
		appMemcpy(&InfoData(0), &NewInfoData(0), NewInfoData.Num() * sizeof(FTerrainInfoData));

		// No longer using lower tessellation collision.
		CollisionTesselationLevel = MaxTesselationLevel;
		// Reset the patches, etc.
		NumPatchesX = NewNumPatchesX;
		NumPatchesY = NewNumPatchesY;
		DrawScale3D = NewDrawScale3D;
		NumVerticesX = NewNumVerticesX;
		NumVerticesY = NewNumVerticesY;

		if (StaticLightingResolution > 1)
		{
			StaticLightingResolution /= MaxTesselationLevel;
			StaticLightingResolution = Max<INT>(1, StaticLightingResolution);
		}

		ClearWeightMaps();
		Allocate();
		CacheWeightMaps(0, 0, NumVerticesX - 1, NumVerticesY - 1);
		TouchWeightMapResources();

		CacheDisplacements(0, 0, NumVerticesX - 1, NumVerticesY - 1);
		CacheDecorations(0, 0, NumVerticesX - 1, NumVerticesY - 1);

		this->MarkPackageDirty();
	}
	else
	{
		// This is a direct copy, so we don't need to alter anything
		// MaxTesselationLevel BETTER be 1!
		check(MaxTesselationLevel == 1);

		ClearWeightMaps();
		Allocate();
		CacheWeightMaps(0, 0, NumVerticesX - 1, NumVerticesY - 1);
		TouchWeightMapResources();

		this->MarkPackageDirty();
	}

	// If the TessellateIncrease button was pressed, 
	// we need to regenerate the components
	ConditionalUpdateComponents();

	debugf(TEXT("Terrain Converted!!"));

	return TRUE;
}

UBOOL ATerrain::TessellateTerrainDown()
{
	if (((NumPatchesX / 2) == 0) || ((NumPatchesY / 2) == 0))
	{
		warnf(TEXT("Unable to remove detail from terrain: NumPatches too small."));
		return FALSE;
	}

	ClearComponents();

	debugf(TEXT("\tPatches....................%4d x %4d"), NumPatchesX, NumPatchesY);
	debugf(TEXT("\tScale......................%8.4f - %8.4f,%8.4f,%8.4f"), DrawScale, DrawScale3D.X, DrawScale3D.Y, DrawScale3D.Z);

	// Determine the new required patches and draw scale
	INT NewNumPatchesX = NumPatchesX / 2;
	INT NewNumPatchesY = NumPatchesY / 2;
	// We need to clamp the patches to MaxDetessellationLevel
	if (NewNumPatchesX < MaxTesselationLevel)
	{
		NewNumPatchesX = MaxTesselationLevel;
	}
	if (NewNumPatchesY < MaxTesselationLevel)
	{
		NewNumPatchesY = MaxTesselationLevel;
	}
	// Limit MaxComponentSize from being 0, negative or large enough to exceed the maximum vertex buffer size.
	MaxComponentSize = Clamp(MaxComponentSize,1,((255 / MaxTesselationLevel) - 1));

	if (DrawScale != 1.0f)
	{
		debugf(TEXT("\t\tDrawScale is not 1.0... putting into DrawScale3D"));
		// Feed the DrawScale into the DrawScale3D
		DrawScale3D *= DrawScale;
		// Set the DrawScale to 1.0
		DrawScale = 1.0f;
	}
	FVector NewDrawScale3D = FVector(DrawScale3D.X*2, DrawScale3D.Y*2, DrawScale3D.Z);

	debugf(TEXT("\tNew Patch Count............%4d x %4d"), NewNumPatchesX, NewNumPatchesY);
	debugf(TEXT("\tNewScale......................%8.4f - %8.4f,%8.4f,%8.4f"), DrawScale, NewDrawScale3D.X, NewDrawScale3D.Y, NewDrawScale3D.Z);

	// Calculate the new NumVertices
	INT NewNumVerticesX = NewNumPatchesX + 1;
	INT NewNumVerticesY = NewNumPatchesY + 1;

	// Touch all the resources...
	for (INT LayerIndex = 0; LayerIndex < Layers.Num(); LayerIndex++)
	{
		FTerrainLayer& Layer = Layers(LayerIndex);
		UTerrainLayerSetup* Setup = Layer.Setup;
		if (Setup)
		{
			// Touch the mapping scale on terrain materials...
			for (INT MaterialIndex = 0; MaterialIndex < Setup->Materials.Num(); MaterialIndex++)
			{
				FTerrainFilteredMaterial& TFilteredMat = Setup->Materials(MaterialIndex);
				UTerrainMaterial* TMat = TFilteredMat.Material;
				if (TMat)
				{
					debugf(TEXT("Make sure to check the mapping scale on %s. Decrease by factor of 2."), *(TMat->GetPathName()));
				}
			}
		}
	}

	INT TotalVertices = NewNumVerticesX * NewNumVerticesY;
	// Setup the heights and infodata
	TArray<FTerrainHeight> NewHeights;
	TArray<FTerrainInfoData> NewInfoData;

	NewHeights.Empty(TotalVertices);
	NewInfoData.Empty(TotalVertices);

	FPatchSampler PatchSampler(MaxTesselationLevel);

	// Convert the heights and infodata
	for (INT NewY = 0; NewY < NewNumVerticesY; NewY++)
	{
		for (INT NewX = 0; NewX < NewNumVerticesX; NewX++)
		{
			// Height
			FTerrainHeight& OldHeight = Heights((NewY*2) * NumVerticesX + (NewX*2));
			new(NewHeights)FTerrainHeight(OldHeight.Value);
			// InfoData
			FTerrainInfoData* InfoData = GetInfoData(NewX*2, NewY*2);
			check(InfoData);
			new(NewInfoData) FTerrainInfoData(InfoData->Data);
		}
	}

	// Convert the alpha maps
	// This will cover deco layers as well...
	for (INT AlphaMapIndex = 0; AlphaMapIndex < AlphaMaps.Num(); AlphaMapIndex++)
	{
		FAlphaMap* AlphaMap = &AlphaMaps(AlphaMapIndex);
		TArray<BYTE> NewAlphas;
		NewAlphas.Empty(TotalVertices);

		for (INT NewY = 0; NewY < NewNumVerticesY; NewY++)
		{
			for (INT NewX = 0; NewX < NewNumVerticesX; NewX++)
			{
				BYTE OldAlpha = AlphaMap->Data((NewY*2) * NumVerticesX + (NewX*2));
				new(NewAlphas)BYTE(OldAlpha);
			}
		}

		AlphaMaps(AlphaMapIndex).Data.Empty(NewAlphas.Num());
		AlphaMaps(AlphaMapIndex).Data.Add(NewAlphas.Num());
		appMemcpy(&AlphaMaps(AlphaMapIndex).Data(0),&NewAlphas(0),NewAlphas.Num());
	}

	// Copy them...
	Heights.Empty(NewHeights.Num());
	Heights.Add(NewHeights.Num());
	appMemcpy(&Heights(0),&NewHeights(0),NewHeights.Num() * sizeof(FTerrainHeight));

	InfoData.Empty(NewInfoData.Num());
	InfoData.Add(NewInfoData.Num());
	appMemcpy(&InfoData(0), &NewInfoData(0), NewInfoData.Num() * sizeof(FTerrainInfoData));

	// Reset the patches, etc.
	NumPatchesX = NewNumPatchesX;
	NumPatchesY = NewNumPatchesY;
	DrawScale3D = NewDrawScale3D;
	NumVerticesX = NewNumVerticesX;
	NumVerticesY = NewNumVerticesY;

	StaticLightingResolution *= 2;

	ClearWeightMaps();
	Allocate();
	CacheWeightMaps(0, 0, NumVerticesX - 1, NumVerticesY - 1);
	TouchWeightMapResources();

	CacheDisplacements(0, 0, NumVerticesX - 1, NumVerticesY - 1);
	CacheDecorations(0, 0, NumVerticesX - 1, NumVerticesY - 1);

	MarkPackageDirty();

	// Regenerate the components
	ConditionalUpdateComponents();

	return TRUE;
}

/**
 *	GetClosestVertex
 *
 *	Determine the vertex that is closest to the given location.
 *	Used for drawing tool items.
 *
 *	@param	InLocation		FVector representing the location caller is interested in
 *	@param	OutVertex		FVector the function will fill in
 *	@param	bConstrained	If TRUE, then select the closest according to editor tessellation level
 *
 *	@return	UBOOL			TRUE indicates the point was found and OutVertex is valid.
 *							FALSE indicates the point was not contained within the terrain.
 */
UBOOL ATerrain::GetClosestVertex(const FVector& InLocation, FVector& OutVertex, UBOOL bConstrained)
{
	FVector	LocalPosition = WorldToLocal().TransformFVector(InLocation); // In the terrain actor's local space.

	if ((LocalPosition.X < 0) || (LocalPosition.X > NumVerticesX) ||
		(LocalPosition.Y < 0) || (LocalPosition.Y > NumVerticesY))
	{
		return FALSE;
	}

	INT X = appRound(LocalPosition.X);
	INT Y = appRound(LocalPosition.Y);
	if ((bConstrained == TRUE) && (EditorTessellationLevel > 0))
	{
		// Adjust it to pick the closest vertex to the tessellation level
		INT CheckTessellationLevel = MaxTesselationLevel / EditorTessellationLevel;
		if ((X % CheckTessellationLevel) > 0)
		{
			X -= X % CheckTessellationLevel;
			X = Clamp<INT>(X, 0, this->NumVerticesX);
		}
		if ((Y % CheckTessellationLevel) > 0)
		{
			Y -= Y % CheckTessellationLevel;
			Y = Clamp<INT>(Y, 0, this->NumVerticesY);
		}
	}
	LocalPosition.X = (FLOAT)(X);
	LocalPosition.Y = (FLOAT)(Y);

	const FTerrainPatch& Patch = GetPatch(X, Y);
	const FVector&	TangentZ = (
		FVector(1,0,GCollisionPatchSampler.SampleDerivX(Patch,0,0) * TERRAIN_ZSCALE) ^ 
		FVector(0,1,GCollisionPatchSampler.SampleDerivY(Patch,0,0) * TERRAIN_ZSCALE)
		).UnsafeNormal();
	FLOAT Displacement = GetCachedDisplacement(X,Y,0,0);

	FLOAT VertexHeight = (FLOAT)(Height(X, Y));
	LocalPosition = FVector(
		LocalPosition.X,
		LocalPosition.Y,
		(-32768.0f + VertexHeight) * TERRAIN_ZSCALE
		) + TangentZ * Displacement;

	OutVertex = LocalToWorld().TransformFVector(LocalPosition);

	return TRUE;
}

/**
 *	GetClosestLocalSpaceVertex
 *
 *	Determine the vertex that is closest to the given location in local space.
 *	The returned position is also in local space.
 *	Used for drawing tool items.
 *
 *	@param	InLocation		FVector representing the location caller is interested in
 *	@param	OutVertex		FVector the function will fill in
 *	@param	bConstrained	If TRUE, then select the closest according to editor tessellation level
 *
 *	@return	UBOOL			TRUE indicates the point was found and OutVertex is valid.
 *							FALSE indicates the point was not contained within the terrain.
 */
UBOOL ATerrain::GetClosestLocalSpaceVertex(const FVector& InLocation, FVector& OutVertex, UBOOL bConstrained)
{
	FVector	LocalPosition = InLocation; // In the terrain actor's local space.

	if ((LocalPosition.X < 0) || (LocalPosition.X > NumVerticesX) ||
		(LocalPosition.Y < 0) || (LocalPosition.Y > NumVerticesY))
	{
		return FALSE;
	}

	INT X = appRound(LocalPosition.X);
	INT Y = appRound(LocalPosition.Y);
	if ((bConstrained == TRUE) && (EditorTessellationLevel > 0))
	{
		// Adjust it to pick the closest vertex to the tessellation level
		INT CheckTessellationLevel = MaxTesselationLevel / EditorTessellationLevel;
		if ((X % CheckTessellationLevel) > 0)
		{
			X -= X % CheckTessellationLevel;
			X = Clamp<INT>(X, 0, this->NumVerticesX);
		}
		if ((Y % CheckTessellationLevel) > 0)
		{
			Y -= Y % CheckTessellationLevel;
			Y = Clamp<INT>(Y, 0, this->NumVerticesY);
		}
	}
	LocalPosition.X = (FLOAT)(X);
	LocalPosition.Y = (FLOAT)(Y);

	const FTerrainPatch& Patch = GetPatch(X, Y);
	const FVector&	TangentZ = (
		FVector(1,0,GCollisionPatchSampler.SampleDerivX(Patch,0,0) * TERRAIN_ZSCALE) ^ 
		FVector(0,1,GCollisionPatchSampler.SampleDerivY(Patch,0,0) * TERRAIN_ZSCALE)
		).UnsafeNormal();
	FLOAT Displacement = GetCachedDisplacement(X,Y,0,0);

	FLOAT VertexHeight = (FLOAT)(Height(X, Y));
	LocalPosition = FVector(
		LocalPosition.X,
		LocalPosition.Y,
		(-32768.0f + VertexHeight) * TERRAIN_ZSCALE
		) + TangentZ * Displacement;

	OutVertex = LocalPosition;

	return TRUE;
}

/**
 *	ShowCollisionCallback
 *
 *	Called when SHOW terrain collision is toggled.
 *
 *	@param	bShow		Whether to show it or not.
 *
 */
void ATerrain::ShowCollisionCallback(UBOOL bShow)
{
	for (FActorIterator It; It; ++It)
	{
		ATerrain* Terrain = Cast<ATerrain>(*It);
		if (Terrain)
		{
			// Each terrain needs to be tagged to show/not show terrain collision.
			// This will require updating the terrain components.
			Terrain->ShowCollisionOverlay(bShow);
		}
	}
}

/**
 *	Show/Hide terrain collision overlay
 *
 *	@param	bShow				Show or hide
 */
void ATerrain::ShowCollisionOverlay(UBOOL bShow)
{
	if (bShowingCollision != bShow)
	{
		bShowingCollision = bShow;
		// Just detach/attach the terrain components
		const FMatrix&	ActorToWorld = LocalToWorld();
		for(INT ComponentIndex = 0;ComponentIndex < TerrainComponents.Num();ComponentIndex++)
		{
			UTerrainComponent* Comp = TerrainComponents(ComponentIndex);
			if (Comp)
			{
				Comp->bDisplayCollisionLevel = bShow;
				Comp->ConditionalDetach();
				Comp->ConditionalAttach(GWorld->Scene,this,ActorToWorld);
			}
		}
	}
}

/**
 *	Update the given selected vertex in the list.
 *	If the vertex is not present, then add it to the list (provided Weight > 0)
 *	
 *	@param	X			
 *	@param	Y			
 *	@param	Weight		
 *	
 */
void ATerrain::UpdateSelectedVertex(INT X, INT Y, FLOAT Weight)
{
	// 
	FSelectedTerrainVertex* FoundVert = NULL;
	INT Index = FindSelectedVertexInList(X, Y, FoundVert);
	if (Index >= 0)
	{
		check(FoundVert);
		FoundVert->Weight += Weight;
		if (FoundVert->Weight <= 0.0f)
		{
			// Remove it from the list 
			SelectedVertices.Remove(Index);
		}
		else
		{
			FoundVert->Weight = Clamp<FLOAT>(FoundVert->Weight, 0.0f, 1.0f);
		}
	}
	else
	if ((Weight > 0.0f) && (Weight <= 1.0f))
	{
		// Add it
		INT NewIndex = SelectedVertices.Add(1);
		FoundVert = &(SelectedVertices(NewIndex));
		FoundVert->X = X;
		FoundVert->Y = Y;
		FoundVert->Weight = Weight;
	}
}

/**
 *	Internal function for getting a selected vertex
 */
INT ATerrain::FindSelectedVertexInList(INT X, INT Y, FSelectedTerrainVertex*& SelectedVert)
{
	for (INT Index = 0; Index < SelectedVertices.Num(); Index++)
	{
		FSelectedTerrainVertex* TestVert = &(SelectedVertices(Index));
		if ((TestVert->X == X) &&
			(TestVert->Y == Y))
		{
			SelectedVert = TestVert;
			return Index;
		}
	}
	return -1;
}

/**
 *	Clear all selected vertices
 */
void ATerrain::ClearSelectedVertexList()
{
	SelectedVertices.Empty();
}

/**
 *	Retrieve the component(s) that contain the given vertex point
 *	The components will be added (using AddUniqueItem) to the supplied array.
 *
 *	@param	X				The X position of interest
 *	@param	Y				The Y position of interest
 *	@param	ComponentList	The array to add found components to
 *
 *	@return	UBOOL			TRUE if any components were found.
 *							FALSE if none were found
 */
UBOOL ATerrain::GetComponentsAtXY(INT X, INT Y, TArray<UTerrainComponent*>& ComponentList)
{
	UBOOL bFoundOne = FALSE;

	for (UINT ComponentIndex = 0;ComponentIndex < (UINT)TerrainComponents.Num();ComponentIndex++)
	{
		UTerrainComponent*	Component = TerrainComponents(ComponentIndex);
		if (Component &&
			(Component->SectionBaseX <= X) &&
			((Component->SectionBaseX + Component->TrueSectionSizeX) >= X) && 
			(Component->SectionBaseY <= Y) &&
			((Component->SectionBaseY + Component->TrueSectionSizeY) >= Y)
			)
		{
			ComponentList.AddUniqueItem(Component);
			bFoundOne = TRUE;
		}
	}

	return bFoundOne;
}

/**
 *	Recache the visibility flags - used when changing tessellation levels.
 */
void ATerrain::RecacheVisibilityFlags()
{
	for (INT ComponentIndex = 0; ComponentIndex < TerrainComponents.Num(); ComponentIndex++)
	{
		UTerrainComponent* TerrainComp = TerrainComponents(ComponentIndex);
		if (TerrainComp)
		{
			for (INT QuadY = 0; QuadY < TerrainComp->SectionSizeY; QuadY++)
			{
				for (INT QuadX = 0; QuadX < TerrainComp->SectionSizeX; QuadX++)
				{
					INT GlobalQuadX = TerrainComp->SectionBaseX + (QuadX * MaxTesselationLevel);
					INT GlobalQuadY = TerrainComp->SectionBaseY + (QuadY * MaxTesselationLevel);

					UBOOL bIsVisible = IsTerrainQuadVisible(GlobalQuadX, GlobalQuadY);

					for (INT InnerY = 0; InnerY < MaxTesselationLevel; InnerY++)
					{
						for (INT InnerX = 0; InnerX < MaxTesselationLevel; InnerX++)
						{
							FTerrainInfoData* InfoData = GetInfoData(GlobalQuadX + InnerX, GlobalQuadY + InnerY);
							if (InfoData)
							{
								if (InfoData->IsVisible() != bIsVisible)
								{
									debugf(TEXT("Terrain: Fixing up visibility on patch %4d,%4d"),
										GlobalQuadX + InnerX, GlobalQuadY + InnerY);
									InfoData->SetIsVisible(bIsVisible);
									MarkPackageDirty();
								}
							}
						}
					}
				}
			}
		}
	}
}

void UTerrainComponent::AddReferencedObjects(TArray<UObject*>& ObjectArray)
{
	Super::AddReferencedObjects(ObjectArray);
	if(LightMap != NULL)
	{
		LightMap->AddReferencedObjects(ObjectArray);
	}
}

void UTerrainComponent::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	Ar << CollisionVertices;
	Ar << BVTree;
	Ar << PatchBounds;
	
	// only allow full directional lightmaps for terrain when using SM3. SM2 uses simple lightmaps
	FLightMapSerializeHelper LightMapSerializeHelper(GetMaterialPlatform(GRHIShaderPlatform) == MSP_SM3,LightMap);
	Ar << LightMapSerializeHelper;
	
	PatchBatches.CountBytes( Ar );
	BatchMaterials.CountBytes( Ar );
}

/**
 * Gets the terrain collision data needed to pass to Novodex or to the
 * kDOP code. Note: this code generates vertices/indices based on the
 * Terrain->CollisionTessellationLevel
 *
 * @param OutVertices		[out] The array that gets each vert in the terrain
 * @param OutIndices		[out] The array that holds the generated indices
 */
void UTerrainComponent::GetCollisionData(TArray<FVector>& OutVertices,TArray<INT>& OutIndices) const
{
	const ATerrain* Terrain = GetTerrain();
	const FMatrix TerrainLocalToWorld( Terrain->LocalToWorld() );

	// Build the vertices for the specified collision level
	for (INT Y = 0; Y <= TrueSectionSizeY; Y++)
	{
		const INT GlobalY = SectionBaseY + Y;
		for (INT X = 0; X <= TrueSectionSizeX; X++)
		{
			const INT GlobalX = SectionBaseX + X;

			const FTerrainPatch& Patch = Terrain->GetPatch(GlobalX,GlobalY);
			// Get the location from the terrain.
			const FVector Vertex = Terrain->GetCollisionVertex(Patch,GlobalX,GlobalY,0,0,1);
			// Store it in world space.
			OutVertices.AddItem( TerrainLocalToWorld.TransformFVector(Vertex) );
		}
	}

	const INT NumVertsX = TrueSectionSizeX + 1;
	const INT NumVertsY = TrueSectionSizeY + 1;

	for(INT QuadY = 0; QuadY < NumVertsY - 1; QuadY++)
	{
		for(INT QuadX = 0; QuadX < NumVertsX - 1; QuadX++)
		{
			const INT GlobalY2 = SectionBaseY + QuadY;
			const INT GlobalX2 = SectionBaseX + QuadX;

			const INT VisibleLookUpX = SectionBaseX + (QuadX / Terrain->MaxTesselationLevel) * Terrain->MaxTesselationLevel;
			const INT VisibleLookUpY = SectionBaseY + (QuadY / Terrain->MaxTesselationLevel) * Terrain->MaxTesselationLevel;
			if (Terrain->IsTerrainQuadVisible(VisibleLookUpX, VisibleLookUpY) == FALSE)
			{
				continue;
			}

			OutIndices.AddItem( ((QuadY+0) * NumVertsX) + (QuadX+0) );
			OutIndices.AddItem( ((QuadY+0) * NumVertsX) + (QuadX+1) );
			OutIndices.AddItem( ((QuadY+1) * NumVertsX) + (QuadX+1) );

			OutIndices.AddItem( ((QuadY+0) * NumVertsX) + (QuadX+0) );
			OutIndices.AddItem( ((QuadY+1) * NumVertsX) + (QuadX+1) );
			OutIndices.AddItem( ((QuadY+1) * NumVertsX) + (QuadX+0) );
		}
	}
}

/**
 * Builds the collision data for this terrain.
 */
void UTerrainComponent::BuildCollisionData(void)
{
	// Generate collision data only for valid terrain components
	if ( TrueSectionSizeX>0 && TrueSectionSizeY>0 )
	{
		// Throw away any previous data
		CollisionVertices.Empty();

		TArray<INT> Indices;
		// Build the terrain verts & indices
		GetCollisionData(CollisionVertices,Indices);

		ATerrain* Terrain = GetTerrain();
		if (Terrain && (Terrain->CollisionType != COLLIDE_NoCollision))
		{
			// Build bounding volume tree for terrain queries.
			BVTree.Build(this);
		}
		else
		{
			// Clear out any existing nodes
			BVTree.Nodes.Empty();
		}
	}
}

/**
 * Rebuilds the collision data for saving
 */
void UTerrainComponent::PreSave(void)
{
	Super::PreSave();

	// Don't want to do this for default UBrushComponent object. Also, we need an Owner.
	if( !IsTemplate() && Owner )
	{
		// Build Unreal collision data.
		BuildCollisionData();
	}
	else
	{
		if (!IsTemplate() && !Owner)
		{
			debugf(TEXT("Serializing terrain component %s for terrain %s with no owner!"),
				*(GetName()), *(GetTerrain()->GetFullName()));
		}
	}
}

void UTerrainComponent::PostLoad()
{
	Super::PostLoad();

	// Fix for old terrain components which weren't created with transactional flag.
	SetFlags( RF_Transactional );

	if(!SectionSizeX || !SectionSizeY)
	{
		// Restore legacy terrain component sizes.
		SectionSizeX = 16;
		SectionSizeY = 16;
	}

	// Update patch bounds if we're converting old content.
	if( PatchBounds.Num() != SectionSizeX * SectionSizeY )
	{
		UpdatePatchBounds();
	}

	// Older content stores terrain collision data in local space, but we want
	// it in world space to address precision issues with huge terrain scales.
	if (GetLinker() && (GetLinker()->Ver() < VER_TERRAIN_COLLISION_WORLD_SPACE))
	{
		BuildCollisionData();
	}

	// Log a warning if collision data is missing
	if (CollisionVertices.Num() == 0)
	{
		warnf(NAME_Warning,TEXT("Terrain was not properly rebuilt, missing collision data"));
	}
}

/** Just kept for backwards serialization compatibility. */
class FTerrainCollisionDataProvider
{
	ATerrain* Terrain;
	UTerrainComponent* Component;
	FMatrix LocalToWorld;
	FMatrix WorldToLocal;
	FTerrainCollisionDataProvider(void)
	{
	}

public:
	FORCEINLINE FTerrainCollisionDataProvider(UTerrainComponent* InComponent) 
	  : Terrain(InComponent->GetTerrain())
	  , Component(InComponent)		
	{
		LocalToWorld = Terrain->LocalToWorld();
		WorldToLocal = Terrain->WorldToLocal();
	}

	FORCEINLINE const FVector& GetVertex(WORD Index) const
	{
		return Component->CollisionVertices(Index);
	}


	FORCEINLINE UMaterialInterface* GetMaterial(WORD MaterialIndex) const
	{
		return NULL;
	}

	FORCEINLINE const FMatrix& GetLocalToWorld(void) const
	{
		return LocalToWorld;
	}


	FORCEINLINE const FMatrix& GetWorldToLocal(void) const
	{
		return WorldToLocal;
	}

	FORCEINLINE FMatrix GetLocalToWorldTransposeAdjoint(void) const
	{
		return LocalToWorld.TransposeAdjoint();
	}

	FORCEINLINE FLOAT GetDeterminant(void) const
	{
		return LocalToWorld.Determinant();
	}
};

/**
 * Uses the kDOP code to determine whether a point with extent encroaches
 */
UBOOL UTerrainComponent::PointCheck(FCheckResult& Result,const FVector& InPoint,const FVector& InExtent,DWORD TraceFlags)
{
	UBOOL Hit = FALSE;
	if (BVTree.Nodes.Num())
	{ 
		FTerrainBVTreePointCollisionCheck BVCheck(InPoint, InExtent, this, &Result);
		Hit = BVTree.PointCheck(BVCheck);
		if (Hit == TRUE)
		{
			// Transform the hit normal to world space if there was a hit
			// This is deferred until now because multiple triangles might get
			// hit in the search and I want to delay the expensive transformation
			// as late as possible
			Result.Normal = BVCheck.GetHitNormal();
			Result.Location = BVCheck.GetHitLocation();
			Result.Actor = Owner;
			Result.Component = this;
		}
	}
	// Values are reversed here...need to fix this one day
	return !Hit;
}

//
//	LineCheckWithBox
//

static UBOOL LineCheckWithBox
(
	const FVector&	BoxCenter,
	const FVector&  BoxRadii,
	const FVector&	Start,
	const FVector&	Direction,
	const FVector&	OneOverDirection
)
{
	//const FVector* boxPlanes = &Box.Min;
	
	FLOAT tf, tb;
	FLOAT tnear = 0.f;
	FLOAT tfar = 1.f;
	
	FVector LocalStart = Start - BoxCenter;

	// X //
	// First - see if ray is parallel to slab.
	if(Direction.X != 0.f)
	{
		// If not, find the time it hits the front and back planes of slab.
		tf = - (LocalStart.X * OneOverDirection.X) - BoxRadii.X * Abs(OneOverDirection.X);
		tb = - (LocalStart.X * OneOverDirection.X) + BoxRadii.X * Abs(OneOverDirection.X);

		if(tf > tnear)
			tnear = tf;

		if(tb < tfar)
			tfar = tb;

		if(tfar < tnear)
			return 0;
	}
	else
	{
		// If it is parallel, early return if start is outiside slab.
		if(!(Abs(LocalStart.X) <= BoxRadii.X))
		{
			return 0;
		}
	}

	// Y //
	if(Direction.Y != 0.f)
	{
		// If not, find the time it hits the front and back planes of slab.
		tf = - (LocalStart.Y * OneOverDirection.Y) - BoxRadii.Y * Abs(OneOverDirection.Y);
		tb = - (LocalStart.Y * OneOverDirection.Y) + BoxRadii.Y * Abs(OneOverDirection.Y);

		if(tf > tnear)
			tnear = tf;

		if(tb < tfar)
			tfar = tb;

		if(tfar < tnear)
			return 0;
	}
	else
	{
		if(!(Abs(LocalStart.Y) <= BoxRadii.Y))
			return 0;
	}

	// Z //
	if(Direction.Z != 0.f)
	{
		// If not, find the time it hits the front and back planes of slab.
		tf = - (LocalStart.Z * OneOverDirection.Z) - BoxRadii.Z * Abs(OneOverDirection.Z);
		tb = - (LocalStart.Z * OneOverDirection.Z) + BoxRadii.Z * Abs(OneOverDirection.Z);

		if(tf > tnear)
			tnear = tf;

		if(tb < tfar)
			tfar = tb;

		if(tfar < tnear)
			return 0;
	}
	else
	{
		if(!(Abs(LocalStart.Z) <= BoxRadii.Z))
		{
			return 0;
		}
	}

	// we hit!
	return 1;
}

UBOOL ClipLineWithBox(const FBox& Box, const FVector& Start, const FVector& End, FVector& IntersectedStart, FVector& IntersectedEnd)
{
    IntersectedStart = Start;
    IntersectedEnd = End;

    FVector Dir;
    FLOAT TEdgeOfBox,TLineLength;
    UBOOL StartCulled,EndCulled;
   
    // Bound by neg X
    StartCulled = IntersectedStart.X < Box.Min.X;
    EndCulled = IntersectedEnd.X < Box.Min.X;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.X > IntersectedStart.X); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Min.X - IntersectedEnd.X;
        TLineLength = IntersectedStart.X - IntersectedEnd.X;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.X > IntersectedEnd.X); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Min.X - IntersectedStart.X;
        TLineLength = IntersectedEnd.X - IntersectedStart.X;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by pos X
    StartCulled = IntersectedStart.X > Box.Max.X;
    EndCulled = IntersectedEnd.X > Box.Max.X;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.X < IntersectedStart.X); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Max.X - IntersectedEnd.X;
        TLineLength = IntersectedStart.X - IntersectedEnd.X;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.X < IntersectedEnd.X); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Max.X - IntersectedStart.X;
        TLineLength = IntersectedEnd.X - IntersectedStart.X;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by neg Y
    StartCulled = IntersectedStart.Y < Box.Min.Y;
    EndCulled = IntersectedEnd.Y < Box.Min.Y;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.Y > IntersectedStart.Y); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Min.Y - IntersectedEnd.Y;
        TLineLength = IntersectedStart.Y - IntersectedEnd.Y;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.Y > IntersectedEnd.Y); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Min.Y - IntersectedStart.Y;
        TLineLength = IntersectedEnd.Y - IntersectedStart.Y;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by pos Y
    StartCulled = IntersectedStart.Y > Box.Max.Y;
    EndCulled = IntersectedEnd.Y > Box.Max.Y;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.Y < IntersectedStart.Y); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Max.Y - IntersectedEnd.Y;
        TLineLength = IntersectedStart.Y - IntersectedEnd.Y;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.Y < IntersectedEnd.Y); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Max.Y - IntersectedStart.Y;
        TLineLength = IntersectedEnd.Y - IntersectedStart.Y;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by neg Z
    StartCulled = IntersectedStart.Z < Box.Min.Z;
    EndCulled = IntersectedEnd.Z < Box.Min.Z;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.Z > IntersectedStart.Z); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Min.Z - IntersectedEnd.Z;
        TLineLength = IntersectedStart.Z - IntersectedEnd.Z;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.Z > IntersectedEnd.Z); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Min.Z - IntersectedStart.Z;
        TLineLength = IntersectedEnd.Z - IntersectedStart.Z;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by pos Z
    StartCulled = IntersectedStart.Z > Box.Max.Z;
    EndCulled = IntersectedEnd.Z > Box.Max.Z;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.Z < IntersectedStart.Z); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Max.Z - IntersectedEnd.Z;
        TLineLength = IntersectedStart.Z - IntersectedEnd.Z;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.Z < IntersectedEnd.Z); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Max.Z - IntersectedStart.Z;
        TLineLength = IntersectedEnd.Z - IntersectedStart.Z;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }
    return TRUE;
}


//
//	UTerrainComponent::LineCheck
//

UBOOL UTerrainComponent::LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags)
{
	if ((GIsEditor == TRUE) && ((TraceFlags & TRACE_Visible) != 0))
	{
		ATerrain* Terrain = Cast<ATerrain>(GetOwner());
		if (Terrain && Terrain->bHiddenEd)
		{
			return TRUE;
		}
	}

	UBOOL	ZeroExtent = (Extent == FVector(0,0,0));

#if STATS
	DWORD Counter = ZeroExtent ? STAT_TerrainZeroExtentTime : STAT_TerrainExtentTime;
	SCOPE_CYCLE_COUNTER(Counter);
#endif

	Result.Time = 1.0f;
	UBOOL Hit = FALSE;

	if (ZeroExtent)
	{
#if	!CONSOLE
		const UBOOL bUseInGameCollision = GWorld->HasBegunPlay();
#else
		// No need for the function call cost on console
		const UBOOL bUseInGameCollision = TRUE;
		// we want to know IMMEDIATELY if we have no nodes in our Tree on consoles as it will be using the slow path then
		// 07/12/19 SAS: Disabling this check as we no longer stream out BVTree data for terrains with
		//				 collision disabled on them. To avoid having any crashing it is being removed.
		//check(BVTree.Nodes.Num() > 0); 
#endif
		// Use the fast kDOP-tree for the all of the traces except in the editor where we use the high poly version
		// NOTE: Always use the slow method in the editor
		if (bUseInGameCollision && BVTree.Nodes.Num() > 0)
		{
			FTerrainBVTreeLineCollisionCheck BVCheck(Start, End, TraceFlags, this, &Result);
			Hit = BVTree.LineCheck(BVCheck);
			if (Hit == TRUE)
			{
				// Transform the hit normal to world space if there was a hit
				// This is deferred until now because multiple triangles might get
				// hit in the search and I want to delay the expensive transformation
				// as late as possible
				Result.Normal = BVCheck.GetHitNormal();
				Result.Component = this;
			}
		}
		else
		{
			if (GIsGame == TRUE)
			{
				warnf(TEXT("UTerrainComponent::LineCheck> GIsGame is TRUE, skipping old checks"));
				return TRUE;
			}

			// Generate the patch bounds if they aren't present
			if (PatchBounds.Num() != (SectionSizeX * SectionSizeY))
			{
				UpdatePatchBounds();
			}

			FVector BoundedStart, BoundedEnd;
			// Clip the trace against the bounding box for a tighter line bounding box
			if (ClipLineWithBox(Bounds.GetBox(), Start, End, BoundedStart, BoundedEnd) == FALSE)
			{
				return TRUE;
			}

			// Get the transform from the terrain
			ATerrain* Terrain = GetTerrain();
			const FMatrix& WorldToLocal = Terrain->WorldToLocal();

			// Transform the trace info to local space
			const FVector& LocalStart = WorldToLocal.TransformFVector(Start);
			const FVector& LocalEnd = WorldToLocal.TransformFVector(End);
			const FVector& LocalDirection = LocalEnd - LocalStart;
			FVector LocalOneOverDirection;
			LocalOneOverDirection.X = Square(LocalDirection.X) > Square(DELTA) ? 1.0f / LocalDirection.X : 0.0f;
			LocalOneOverDirection.Y = Square(LocalDirection.Y) > Square(DELTA) ? 1.0f / LocalDirection.Y : 0.0f;
			LocalOneOverDirection.Z = Square(LocalDirection.Z) > Square(DELTA) ? 1.0f / LocalDirection.Z : 0.0f;
			
			// Use the clipped line to generate the X/Y region to check
			const FVector& BoundedLocalStart = WorldToLocal.TransformFVector(BoundedStart);
			const FVector& BoundedLocalEnd = WorldToLocal.TransformFVector(BoundedEnd);
			// Clamp the line by the components section
			INT	MinX = Clamp(appTrunc(Min(BoundedLocalStart.X,BoundedLocalEnd.X)),SectionBaseX,SectionBaseX + TrueSectionSizeX - 1),
				MinY = Clamp(appTrunc(Min(BoundedLocalStart.Y,BoundedLocalEnd.Y)),SectionBaseY,SectionBaseY + TrueSectionSizeY - 1),
				MaxX = Clamp(appTrunc(Max(BoundedLocalStart.X,BoundedLocalEnd.X)),SectionBaseX,SectionBaseX + TrueSectionSizeX - 1),
				MaxY = Clamp(appTrunc(Max(BoundedLocalStart.Y,BoundedLocalEnd.Y)),SectionBaseY,SectionBaseY + TrueSectionSizeY - 1);

			if(!Result.Actor)
				Result.Time = 1.0f;

			INT	TesselationLevel = 1;

			// Check against the patches within the line's bounding box,
			// clipped to this component via the min/max values for the section
			for(INT PatchY = MinY;PatchY <= MaxY;PatchY++)
			{
				// Check each patch within the clipped row
				for (INT PatchX = MinX; PatchX <= MaxX; PatchX++)
				{
					// PatchX, PatchY will already be in the terrain grid space
					// (not the component space)
					if ((TraceFlags & TRACE_TerrainIgnoreHoles) == 0)
					{
						if (Terrain->IsTerrainQuadVisible(PatchX, PatchY) == FALSE)
						{
							continue;
						}
					}

					const FTerrainPatchBounds& PatchBound = PatchBounds(
						(PatchY - SectionBaseY) / Terrain->MaxTesselationLevel * SectionSizeX + 
						(PatchX - SectionBaseX) / Terrain->MaxTesselationLevel);

					FLOAT CenterHeight = (PatchBound.MaxHeight + PatchBound.MinHeight) * 0.5f;

					if (!LineCheckWithBox(
							FVector(PatchX + 0.5f,PatchY + 0.5f,CenterHeight),
							FVector(
								0.5f + PatchBound.MaxDisplacement,
								0.5f + PatchBound.MaxDisplacement,
								PatchBound.MaxHeight - CenterHeight
								),
							LocalStart,
							LocalDirection,
							LocalOneOverDirection
							)
						)
					{
						continue;
					}

					const FTerrainPatch&	Patch = Terrain->GetPatch(PatchX,PatchY);

					FVector	PatchVertexCache[2][TERRAIN_MAXTESSELATION + 1];
					INT		NextCacheRow = 0;

					for(INT SubX = 0;SubX <= TesselationLevel;SubX++)
					{
						PatchVertexCache[NextCacheRow][SubX] = Terrain->GetCollisionVertex(Patch,PatchX,PatchY,SubX,0,TesselationLevel);
					}

					NextCacheRow = 1 - NextCacheRow;

					for(INT SubY = 0;SubY < TesselationLevel;SubY++)
					{
						for(INT SubX = 0;SubX <= TesselationLevel;SubX++)
						{
							PatchVertexCache[NextCacheRow][SubX] = Terrain->GetCollisionVertex(Patch,PatchX,PatchY,SubX,SubY + 1,TesselationLevel);
						}

						for(INT SubX = 0;SubX < TesselationLevel;SubX++)
						{
							const FVector&	V00 = PatchVertexCache[1 - NextCacheRow][SubX],
											V10 = PatchVertexCache[1 - NextCacheRow][SubX + 1],
											V01 = PatchVertexCache[NextCacheRow][SubX],
											V11 = PatchVertexCache[NextCacheRow][SubX + 1];
							UBOOL			TesselationHit = 0;

								TesselationHit |= LineCheckWithTriangle(Result,V00,V01,V11,LocalStart,LocalEnd,LocalDirection);
								TesselationHit |= LineCheckWithTriangle(Result,V11,V10,V00,LocalStart,LocalEnd,LocalDirection);

							if(TesselationHit)
							{
								Result.Component = this;
								Result.Actor = Owner;
								Result.Material = NULL;
								Result.Normal.Normalize();
								Result.Normal = Terrain->LocalToWorld().TransposeAdjoint().TransformNormal(Result.Normal).SafeNormal(); 

								goto Finished;
							}
						}

						NextCacheRow = 1 - NextCacheRow;
					}
				}
			}
		}
	}
	// Swept box check
	else
	{
		if (BVTree.Nodes.Num() > 0)
		{
			FTerrainBVTreeBoxCollisionCheck BVCheck(Start, End, Extent, TraceFlags, this, &Result);
			Hit = BVTree.BoxCheck(BVCheck);
			if(Hit == TRUE)
			{
				// Transform the hit normal to world space if there was a hit
				// This is deferred until now because multiple triangles might get
				// hit in the search and I want to delay the expensive transformation
				// as late as possible
				Result.Normal = BVCheck.GetHitNormal();
				Result.Component = this;
			}
		}
	}

Finished:
	if(Result.Component == this)
	{
		Result.Actor = Owner;
		Result.Time = Clamp(Result.Time - Clamp(0.1f,0.1f / (End - Start).Size(),4.0f / (End - Start).Size()),0.0f,1.0f);
		Result.Location	= Start + (End - Start) * Result.Time;
	}
	return (Result.Component != this);
}

UBOOL ATerrain::ActorLineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags)
{
	UBOOL Hit = 0;
	for(INT ComponentIndex = 0;ComponentIndex < Components.Num();ComponentIndex++)
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Components(ComponentIndex));
		if(Primitive && !Primitive->LineCheck(Result,End,Start,Extent,TraceFlags))
		{
			Hit = 1;
		}
	}
	for(INT ComponentIndex = 0;ComponentIndex < TerrainComponents.Num();ComponentIndex++)
	{
		UPrimitiveComponent* Primitive = TerrainComponents(ComponentIndex);
		if(Primitive && !Primitive->LineCheck(Result,End,Start,Extent,TraceFlags))
		{
			Hit = 1;
		}
	}
	return !Hit;
}

//
//	UTerrainComponent::UpdateBounds
//

void UTerrainComponent::UpdateBounds()
{
	ATerrain* Terrain = GetTerrain();

	// Build patch bounds if necessary.
	if( PatchBounds.Num() != SectionSizeX * SectionSizeY )
	{
		UpdatePatchBounds();
	}

	FBox BoundingBox(0);

	INT Scalar = Terrain->MaxTesselationLevel;
	
	for(INT Y = 0;Y < SectionSizeY;Y++)
	{
		for(INT X = 0;X < SectionSizeX;X++)
		{
			const FTerrainPatchBounds&	Patch = PatchBounds(Y * SectionSizeX + X);
			BoundingBox += FBox(
				FVector((X * Scalar) - Patch.MaxDisplacement,
						(Y * Scalar) - Patch.MaxDisplacement,
						Patch.MinHeight),
				FVector(((X + 1) * Scalar) + Patch.MaxDisplacement,
						((Y + 1) * Scalar) + Patch.MaxDisplacement,
						Patch.MaxHeight)
				);
		}
	}
	Bounds = FBoxSphereBounds(BoundingBox.TransformBy(LocalToWorld).ExpandBy(1.0f));
}

/**
 * Returns the MAX number of triangle this component will render.
 *
 *	@return	UINT	Maximum number of triangle that could be rendered.
 */
UINT UTerrainComponent::GetMaxTriangleCount( ) const
{
	ATerrain* Terrain = GetTerrain();
	if (Terrain)
	{
		// Calculate the patch count for this component
		UINT PatchCount = (UINT)(TrueSectionSizeX * TrueSectionSizeY);
		// Two triangles per patch...
		return (PatchCount * 2);
	}

	return 0;
}

/**
 * Returns the lightmap resolution used for this primivite instance in the case of it supporting texture light/ shadow maps.
 * 0 if not supported or no static shadowing.
 *
 * @param Width		[out]	Width of light/shadow map
 * @param Height	[out]	Height of light/shadow map
 */
void UTerrainComponent::GetLightMapResolution( INT& Width, INT& Height ) const
{
	ATerrain* Terrain = GetTerrain();
	if (Terrain)
	{
		INT StaticLightingRes = Terrain->StaticLightingResolution;

		// NOTE1: The override light resolution only matters during setting of the property...
		// NOTE2: This code assume that there will be DXT1 compression of the light map!
		INT PixelPaddingX = GPixelFormats[PF_DXT1].BlockSizeX;
		INT PixelPaddingY = GPixelFormats[PF_DXT1].BlockSizeY;

		INT PatchExpandCountX = (TERRAIN_PATCH_EXPAND_SCALAR * PixelPaddingX) / StaticLightingRes;
		INT PatchExpandCountY = (TERRAIN_PATCH_EXPAND_SCALAR * PixelPaddingY) / StaticLightingRes;
		PatchExpandCountX = Max<INT>(1, PatchExpandCountX);
		PatchExpandCountY = Max<INT>(1, PatchExpandCountY);

		Width	= (2 * PatchExpandCountX + TrueSectionSizeX) * StaticLightingRes + 1;
		Height	= (2 * PatchExpandCountY + TrueSectionSizeY) * StaticLightingRes + 1;

		// Align it as this is what the lightmap texture mapping code will do
		INT Modulous = (PixelPaddingX - 1);
		Width	= (Width  + Modulous) & ~Modulous;
		Height	= (Height + Modulous) & ~Modulous;
	}
	else
	{
		Width	= 0;
		Height	= 0;
	}
}

/**
 * Returns the light and shadow map memory for this primite in its out variables.
 *
 * Shadow map memory usage is per light whereof lightmap data is independent of number of lights, assuming at least one.
 *
 * @param [out] LightMapMemoryUsage		Memory usage in bytes for light map (either texel or vertex) data
 * @param [out]	ShadowMapMemoryUsage	Memory usage in bytes for shadow map (either texel or vertex) data
 */
void UTerrainComponent::GetLightAndShadowMapMemoryUsage( INT& LightMapMemoryUsage, INT& ShadowMapMemoryUsage ) const
{
	// Zero initialize.
	ShadowMapMemoryUsage	= 0;
	LightMapMemoryUsage		= 0;

	ATerrain* Terrain = GetTerrain();
	if (Terrain)
	{
		// Cache light/ shadow map resolution.
		INT LightMapWidth		= 0;
		INT	LightMapHeight		= 0;
		GetLightMapResolution( LightMapWidth, LightMapHeight );

		// Determine whether static mesh/ static mesh component has static shadowing.
		if ( HasStaticShadowing() )
		{
			// Determine whether we are using a texture or vertex buffer to store precomputed data.
			if ( LightMapWidth > 0 && LightMapHeight > 0 )
			{
				// Stored in texture.
				const FLOAT MIP_FACTOR = 1.33f;
				ShadowMapMemoryUsage	= appTrunc( MIP_FACTOR * LightMapWidth * LightMapHeight ); // G8
				const UINT NumLightMapCoefficients = GSystemSettings.bAllowDirectionalLightMaps ? NUM_DIRECTIONAL_LIGHTMAP_COEF : NUM_SIMPLE_LIGHTMAP_COEF;
				LightMapMemoryUsage		= appTrunc( NumLightMapCoefficients * MIP_FACTOR * LightMapWidth * LightMapHeight / 2 ); // DXT1
			}
		}
	}
}

//
//	UTerrainComponent::Init
//

void UTerrainComponent::Init(INT InBaseX,INT InBaseY,INT InSizeX,INT InSizeY,INT InTrueSizeX,INT InTrueSizeY)
{
	SectionBaseX = InBaseX;
	SectionBaseY = InBaseY;
	SectionSizeX = InSizeX;
	SectionSizeY = InSizeY;
	TrueSectionSizeX = InTrueSizeX;
	TrueSectionSizeY = InTrueSizeY;
	UpdatePatchBounds();
}

//
//	UTerrainComponent::UpdatePatchBounds
//

void UTerrainComponent::UpdatePatchBounds()
{
	ATerrain* Terrain = GetTerrain();
	PatchBounds.Empty(SectionSizeX * SectionSizeY);
	FTerrainPatchBounds Bounds;

	for (INT Y = 0; Y < SectionSizeY; Y++)
	{
		for (INT X = 0; X < SectionSizeX; X++)
		{
			INT LocalX = X * Terrain->MaxTesselationLevel;
			INT LocalY = Y * Terrain->MaxTesselationLevel;

			Bounds.MinHeight = 32768.0f * TERRAIN_ZSCALE;
			Bounds.MaxHeight = -32768.0f * TERRAIN_ZSCALE;
			Bounds.MaxDisplacement = 0.0f;

			INT	GlobalX = SectionBaseX + LocalX,
				GlobalY = SectionBaseY + LocalY;

			// Don't need these... full tessellation is now stored...
			for (INT SubY = 0; SubY <= Terrain->MaxTesselationLevel; SubY++)
			{
				for (INT SubX = 0; SubX <= Terrain->MaxTesselationLevel; SubX++)
				{
					// Since we are sampling each vertex, we need to regrab the patch
					const FTerrainPatch& Patch = Terrain->GetPatch(GlobalX + SubX,GlobalY + SubY);
					const FVector& Vertex = Terrain->GetCollisionVertex(Patch,GlobalX+SubX,GlobalY+SubY,0,0,1);

					Bounds.MinHeight = Min(Bounds.MinHeight,Vertex.Z);
					Bounds.MaxHeight = Max(Bounds.MaxHeight,Vertex.Z);

					Bounds.MaxDisplacement = Max(
												Bounds.MaxDisplacement,
												Max(
													Max(Vertex.X - GlobalX - 1.0f,GlobalX - Vertex.X),
													Max(Vertex.Y - GlobalY - 1.0f,GlobalY - Vertex.Y)
													)
												);
				}
			}

			PatchBounds.AddItem(Bounds);
		}
	}
}

//
//	UTerrainComponent::SetParentToWorld
//

void UTerrainComponent::SetParentToWorld(const FMatrix& ParentToWorld)
{
	ATerrain* Terrain = GetTerrain();
	Super::SetParentToWorld(FTranslationMatrix(FVector(SectionBaseX,SectionBaseY,0)) * ParentToWorld);
}

FDecalRenderData* UTerrainComponent::GenerateDecalRenderData(FDecalState* Decal) const
{
	SCOPE_CYCLE_COUNTER(STAT_DecalTerrainAttachTime);

	// Do nothing if the specified decal doesn't project on terrain.
	if ( !Decal->bProjectOnTerrain )
	{
		return NULL;
	}
	FDecalRenderData* DecalRenderData = NULL;

	// scissor rect based on frustum assumes local space verts (see FMeshDrawingPolicy::SetMeshRenderState)
	FMatrix WorldToLocal = LocalToWorld.Inverse();
	Decal->TransformFrustumVerts( WorldToLocal );
	// no clipping occurs for terrain. Rely on screen space scissor rect culling instead
	Decal->bUseSoftwareClip = FALSE;

	if( TerrainObject )
	{
		DecalRenderData = TerrainObject->GenerateDecalRenderData(Decal);
	}

	return DecalRenderData;
}

/** Represents terrain to the static lighting system. */
class FTerrainComponentStaticLighting : public FStaticLightingMesh, public FStaticLightingTextureMapping
{
public:
	/** Initialization constructor. */
	FTerrainComponentStaticLighting(
		UTerrainComponent* InPrimitive,
		const TArray<FIntPoint>& InQuadIndexToCoordinatesMap,
		const TArray<ULightComponent*>& InRelevantLights,
		UBOOL bPerformFullQualityBuild,
		INT InPatchExpandX, INT InPatchExpandY
		):
		FStaticLightingMesh(
			InQuadIndexToCoordinatesMap.Num() * 2,
			InQuadIndexToCoordinatesMap.Num() * 4,
			InPrimitive->CastShadow | InPrimitive->bCastHiddenShadow,
			InPrimitive->bSelfShadowOnly,
			FALSE,
			InRelevantLights,
			InPrimitive->Bounds.GetBox()
			),
		FStaticLightingTextureMapping(
			this,
			InPrimitive,
			(2 * InPatchExpandX + InPrimitive->TrueSectionSizeX) * InPrimitive->GetTerrain()->StaticLightingResolution / (bPerformFullQualityBuild ? 1 : 2) + 1,
			(2 * InPatchExpandY + InPrimitive->TrueSectionSizeY) * InPrimitive->GetTerrain()->StaticLightingResolution / (bPerformFullQualityBuild ? 1 : 2) + 1,
			1,
			InPrimitive->bForceDirectLightMap,
			InPrimitive->GetTerrain()->bBilinearFilterLightmapGeneration
			),
		Terrain(InPrimitive->GetTerrain()),
		Primitive(InPrimitive),
		NumQuadsX(InPrimitive->TrueSectionSizeX),
		NumQuadsY(InPrimitive->TrueSectionSizeY),
		ExpandQuadsX(InPatchExpandX), 
		ExpandQuadsY(InPatchExpandY),
		QuadIndexToCoordinatesMap(InQuadIndexToCoordinatesMap)
	{
	}

	FStaticLightingVertex GetVertex(INT X,INT Y) const
	{
		FStaticLightingVertex Result;

		const INT PatchX = Primitive->SectionBaseX + X;
		const INT PatchY = Primitive->SectionBaseY + Y;

		FVector WorldTangentX;
		FVector WorldTangentY;
		FVector WorldTangentZ;
		FLOAT Displacement;

		FLOAT Z = (FLOAT)Terrain->Height(PatchX, PatchY);
		const FLOAT HeightNegX = (FLOAT)Terrain->Height(PatchX - 1,PatchY);
		const FLOAT HeightPosX = (FLOAT)Terrain->Height(PatchX + 1,PatchY);
		const FLOAT HeightNegY = (FLOAT)Terrain->Height(PatchX,PatchY - 1);
		const FLOAT HeightPosY = (FLOAT)Terrain->Height(PatchX,PatchY + 1);
		const FLOAT SampleDerivX = (HeightPosX - HeightNegX) / (FLOAT)1.0f / 2.0f;
		const FLOAT SampleDerivY = (HeightPosY - HeightNegY) / (FLOAT)1.0f / 2.0f;
		FVector TempWorldTangentX = FVector(1,0,(SampleDerivX * (Terrain->DrawScale3D.Z / Terrain->DrawScale3D.X) * TERRAIN_ZSCALE));
		FVector TempWorldTangentY = FVector(0,1,(SampleDerivY * (Terrain->DrawScale3D.Z / Terrain->DrawScale3D.Y) * TERRAIN_ZSCALE));

		WorldTangentX = Primitive->LocalToWorld.TransformNormal(TempWorldTangentX).SafeNormal();
		WorldTangentY = Primitive->LocalToWorld.TransformNormal(TempWorldTangentY).SafeNormal();
		WorldTangentZ = (WorldTangentX ^ WorldTangentY).SafeNormal();

		Displacement = Terrain->GetCachedDisplacement(PatchX,PatchY,0,0);
		Result.WorldPosition = Primitive->LocalToWorld.TransformFVector(FVector(X,Y,(-32768.0f + Z) * TERRAIN_ZSCALE)) + WorldTangentZ * Displacement;

		Result.WorldTangentX = WorldTangentX;
		Result.WorldTangentY = WorldTangentY;
		Result.WorldTangentZ = WorldTangentZ;

		check((X + ExpandQuadsX) >= 0);
		check((Y + ExpandQuadsY) >= 0);

		Result.TextureCoordinates[0] = FVector2D(PatchX,PatchY);
		Result.TextureCoordinates[1].X = ((X + ExpandQuadsX) * Terrain->StaticLightingResolution + 0.5f) / (FLOAT)SizeX;
		Result.TextureCoordinates[1].Y = ((Y + ExpandQuadsY) * Terrain->StaticLightingResolution + 0.5f) / (FLOAT)SizeY;

		return Result;
	}

	// FStaticLightingMesh interface.
	virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const
	{
		const INT QuadIndex = TriangleIndex / 2;
		const FIntPoint& QuadCoordinates = QuadIndexToCoordinatesMap(QuadIndex);

		if(TriangleIndex & 1)
		{
			OutV0 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 0);
			OutV1 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 1);
			OutV2 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 1);
		}
		else
		{
			OutV0 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 1);
			OutV1 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 0);
			OutV2 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 0);
		}
	}
	virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
	{
		const INT QuadIndex = TriangleIndex / 2;

		if(TriangleIndex & 1)
		{
			OutI0 = QuadIndex * 4 + 0;
			OutI1 = QuadIndex * 4 + 2;
			OutI2 = QuadIndex * 4 + 3;
		}
		else
		{
			OutI0 = QuadIndex * 4 + 3;
			OutI1 = QuadIndex * 4 + 1;
			OutI2 = QuadIndex * 4 + 0;
		}
	}
	virtual FLightRayIntersection IntersectLightRay(const FVector& Start,const FVector& End,UBOOL bFindNearestIntersection) const
	{
		// Intersect the light ray with the terrain component.
		FCheckResult Result(1.0f);
		const UBOOL bIntersects = !Primitive->LineCheck(Result,End,Start,FVector(0,0,0),TRACE_ShadowCast | (!bFindNearestIntersection ? TRACE_StopAtAnyHit : 0));

		// Setup a vertex to represent the intersection.
		FStaticLightingVertex IntersectionVertex;
		if(bIntersects)
		{
			IntersectionVertex.WorldPosition = Result.Location;
			IntersectionVertex.WorldTangentZ = Result.Normal;
		}
		else
		{
			IntersectionVertex.WorldPosition.Set(0,0,0);
			IntersectionVertex.WorldTangentZ.Set(0,0,1);
		}
		return FLightRayIntersection(bIntersects,IntersectionVertex);
	}

	// FStaticLightingTextureMapping interface.
	virtual void Apply(FLightMapData2D* LightMapData,const TMap<ULightComponent*,FShadowMapData2D*>& ShadowMapData)
	{
		// Create a light-map for the primitive.
		Primitive->LightMap = FLightMap2D::AllocateLightMap(
			Primitive,
			LightMapData,
			NULL,
			Primitive->Bounds
			);
		delete LightMapData;

		// Create the shadow-maps for the primitive.
		Primitive->ShadowMaps.Empty(ShadowMapData.Num());
		for(TMap<ULightComponent*,FShadowMapData2D*>::TConstIterator ShadowMapDataIt(ShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
		{
			Primitive->ShadowMaps.AddItem(
				new(Owner) UShadowMap2D(
					*ShadowMapDataIt.Value(),
					ShadowMapDataIt.Key()->LightGuid,
					NULL,
					Primitive->Bounds
					)
				);
			delete ShadowMapDataIt.Value();
		}

		// Build the list of statically irrelevant lights.
		// TODO: This should be stored per LOD.
		Primitive->IrrelevantLights.Empty();
		for(INT LightIndex = 0;LightIndex < RelevantLights.Num();LightIndex++)
		{
			const ULightComponent* Light = RelevantLights(LightIndex);

			// Check if the light is stored in the light-map.
			const UBOOL bIsInLightMap = Primitive->LightMap && Primitive->LightMap->LightGuids.ContainsItem(Light->LightmapGuid);

			// Check if the light is stored in the shadow-map.
			UBOOL bIsInShadowMap = FALSE;
			for(INT ShadowMapIndex = 0;ShadowMapIndex < Primitive->ShadowMaps.Num();ShadowMapIndex++)
			{
				if(Primitive->ShadowMaps(ShadowMapIndex)->GetLightGuid() == Light->LightGuid)
				{
					bIsInShadowMap = TRUE;
					break;
				}
			}

			// Add the light to the statically irrelevant light list if it is in the potentially relevant light list, but didn't contribute to the light-map or a shadow-map.
			if(!bIsInLightMap && !bIsInShadowMap)
			{	
				Primitive->IrrelevantLights.AddUniqueItem(Light->LightGuid);
			}
		}

		// Mark the primitive's package as dirty.
		Primitive->MarkPackageDirty();
	}

private:

	/** The terrain this object represents. */
	ATerrain* const Terrain;

	/** The primitive this object represents. */
	UTerrainComponent* const Primitive;

	/** The inverse transpose of the primitive's local to world transform. */
	const FMatrix LocalToWorldInverseTranspose;

	/** The number of quads in the component along the X axis. */
	const INT NumQuadsX;

	/** The number of quads in the component along the Y axis. */
	const INT NumQuadsY;

	/** The number of quads we are expanding to eliminate seams. */
	INT ExpandQuadsX;
	INT ExpandQuadsY;

	/** A map from quad index to the quad's coordinates. */
	TArray<FIntPoint> QuadIndexToCoordinatesMap;
};

void UTerrainComponent::GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options)
{
	if(HasStaticShadowing())
	{
		// Determine the number of quads we need to 'expand' by to ensure smooth lighting across seams...
		ATerrain* Terrain = GetTerrain();
		check(Terrain);

		// Assuming DXT_1 compression at the moment...
		INT PixelPaddingX = GPixelFormats[PF_DXT1].BlockSizeX;
		INT PixelPaddingY = GPixelFormats[PF_DXT1].BlockSizeY;

		INT PatchExpandCountX = (TERRAIN_PATCH_EXPAND_SCALAR * PixelPaddingX) / Terrain->StaticLightingResolution;
		INT PatchExpandCountY = (TERRAIN_PATCH_EXPAND_SCALAR * PixelPaddingY) / Terrain->StaticLightingResolution;

		PatchExpandCountX = Max<INT>(1, PatchExpandCountX);
		PatchExpandCountY = Max<INT>(1, PatchExpandCountY);

		// Index the quads in the component.
		TArray<FIntPoint> QuadIndexToCoordinateMap;
		for (INT QuadY = -PatchExpandCountY; QuadY < TrueSectionSizeY + PatchExpandCountY; QuadY++)
		{
			for (INT QuadX = -PatchExpandCountX; QuadX < TrueSectionSizeX + PatchExpandCountX; QuadX++)
			{
				if(Terrain->IsTerrainQuadVisible(SectionBaseX + QuadX,SectionBaseY + QuadY))
				{
					QuadIndexToCoordinateMap.AddItem(FIntPoint(QuadX,QuadY));
				}
			}
		}

		// Create the object which represents the component to the static lighting system.
		FTerrainComponentStaticLighting* PrimitiveStaticLighting = new FTerrainComponentStaticLighting(
			this,QuadIndexToCoordinateMap,InRelevantLights,Options.bPerformFullQualityBuild,
			PatchExpandCountX, PatchExpandCountY);
		OutPrimitiveInfo.Mappings.AddItem(PrimitiveStaticLighting);
		OutPrimitiveInfo.Meshes.AddItem(PrimitiveStaticLighting);
	}
}

void UTerrainComponent::InvalidateLightingCache()
{
	// Save the terrain component state for transactions.
	Modify();

	// Mark lighting as requiring a rebuilt.
	MarkLightingRequiringRebuild();

	// Detach the component from the scene for the duration of this function.
	FComponentReattachContext ReattachContext(this);

	// Discard all cached lighting.
	IrrelevantLights.Empty();
	ShadowMaps.Empty();
	LightMap = NULL;
}

/** Returns a vertex in the component's local space. */
FVector UTerrainComponent::GetLocalVertex(INT X,INT Y) const
{
	return FVector(X,Y,(-32768.0f + (FLOAT)GetTerrain()->Height(SectionBaseX + X,SectionBaseY + Y)) * TERRAIN_ZSCALE);
}

/** Returns a vertex in the component's local space. */
FVector UTerrainComponent::GetWorldVertex(INT X,INT Y) const
{
	if(IsAttached())
	{
		return LocalToWorld.TransformFVector(GetLocalVertex(X,Y));
	}
	else
	{
		return GetTerrain()->GetWorldVertex(SectionBaseX + X,SectionBaseY + Y);
	}
}

void UTerrainComponent::GetStaticTriangles(FPrimitiveTriangleDefinitionInterface* PTDI) const
{
	const ATerrain* const Terrain = GetTerrain();

	for(INT QuadY = 0;QuadY < TrueSectionSizeY;QuadY++)
	{
		for(INT QuadX = 0;QuadX < TrueSectionSizeX;QuadX++)
		{
			const INT GlobalQuadX = SectionBaseX + QuadX;
			const INT GlobalQuadY = SectionBaseY + QuadY;
			if(GetTerrain()->IsTerrainQuadVisible(GlobalQuadX,GlobalQuadY))
			{
				const FTerrainPatch& Patch = Terrain->GetPatch(GlobalQuadX,GlobalQuadY);

				// Setup the quad's vertices.
				FPrimitiveTriangleVertex Vertices[2][2];
				for(INT SubY = 0;SubY < 2;SubY++)
				{
					for(INT SubX = 0;SubX < 2;SubX++)
					{
						const FLOAT SampleDerivX = GCollisionPatchSampler.SampleDerivX(Patch,SubX,SubY);
						const FLOAT SampleDerivY = GCollisionPatchSampler.SampleDerivY(Patch,SubX,SubY);
						const FVector WorldTangentX = LocalToWorld.TransformNormal(FVector(1,0,SampleDerivX * TERRAIN_ZSCALE)).SafeNormal();
						const FVector WorldTangentY = LocalToWorld.TransformNormal(FVector(0,1,SampleDerivY * TERRAIN_ZSCALE)).SafeNormal();
						const FVector WorldTangentZ = (WorldTangentX ^ WorldTangentY).SafeNormal();
						const FLOAT Displacement = Terrain->GetCachedDisplacement(GlobalQuadX + SubX,GlobalQuadY + SubY,0,0);

						FPrimitiveTriangleVertex& DestVertex = Vertices[SubX][SubY];

						DestVertex.WorldPosition = LocalToWorld.TransformFVector(FVector(QuadX + SubX,QuadY + SubY,(-32768.0f + (FLOAT)(Terrain->Height(GlobalQuadX + SubX, GlobalQuadY + SubY))) * TERRAIN_ZSCALE)) + WorldTangentZ * Displacement;
						DestVertex.WorldTangentX = WorldTangentX;
						DestVertex.WorldTangentY = WorldTangentY;
						DestVertex.WorldTangentZ = WorldTangentZ;
					}
				}

				PTDI->DefineTriangle(Vertices[0][0],Vertices[0][1],Vertices[1][1]);
				PTDI->DefineTriangle(Vertices[1][1],Vertices[1][0],Vertices[0][0]);
			}
		}
	}
}

void UTerrainComponent::GetStreamingTextureInfo(TArray<FStreamingTexturePrimitiveInfo>& OutStreamingTextures) const
{
	const FSphere BoundingSphere = Bounds.GetSphere();
	const ATerrain* Terrain = GetTerrain();

	for( INT MaterialIndex=0; MaterialIndex<Terrain->WeightedMaterials.Num(); MaterialIndex++ )
	{
		const UTerrainMaterial* TerrainMaterial = Terrain->WeightedMaterials(MaterialIndex).Material;
		if(TerrainMaterial && TerrainMaterial->Material)
		{
			// Determine whether the terrain component uses this weighted material.
			UBOOL bComponentUsesWeightedMaterial = FALSE;
			for( INT BatchIndex=0; BatchIndex<BatchMaterials.Num(); BatchIndex++ )
			{
				if( BatchMaterials(BatchIndex).Get(MaterialIndex) )
				{
					bComponentUsesWeightedMaterial = TRUE;
					break;
				}
			}
			if(bComponentUsesWeightedMaterial)
			{
				// Calculate the texel factor for the material's mapping on the terrain.
				const FLOAT TexelFactor = TerrainMaterial->MappingScale * Terrain->DrawScale * Terrain->DrawScale3D.GetAbsMax();

				// Enumerate the material's textures.
				TArray<UTexture*> Textures;
				TerrainMaterial->Material->GetTextures(Textures);

				// Add each texture to the output with the appropriate parameters.
				for(INT TextureIndex = 0;TextureIndex < Textures.Num();TextureIndex++)
				{
					FStreamingTexturePrimitiveInfo& StreamingTexture = *new(OutStreamingTextures) FStreamingTexturePrimitiveInfo;
					StreamingTexture.Bounds = BoundingSphere;
					StreamingTexture.TexelFactor = TexelFactor;
					StreamingTexture.Texture = Textures(TextureIndex);
				}
			}
		}
	}
}

//
//	UTerrainMaterial::UpdateMappingTransform
//

void UTerrainMaterial::UpdateMappingTransform()
{
	FMatrix	BaseDirection;
	switch(MappingType)
	{
	case TMT_XZ:
		BaseDirection = FMatrix(FPlane(1,0,0,0),FPlane(0,0,1,0),FPlane(0,1,0,0),FPlane(0,0,0,1));
		break;
	case TMT_YZ:
		BaseDirection = FMatrix(FPlane(0,0,1,0),FPlane(1,0,0,0),FPlane(0,1,0,0),FPlane(0,0,0,1));
		break;
	case TMT_XY:
	default:
		BaseDirection = FMatrix::Identity;
		break;
	};

	LocalToMapping = BaseDirection *
		FScaleMatrix(FVector(1,1,1) * (MappingScale == 0.0f ? 1.0f : 1.0f / MappingScale)) *
		FMatrix(
			FPlane(+appCos(MappingRotation * (FLOAT)PI / 180.0f),	-appSin(MappingRotation * (FLOAT)PI / 180.0f),	0,	0),
			FPlane(+appSin(MappingRotation * (FLOAT)PI / 180.0f),	+appCos(MappingRotation * (FLOAT)PI / 180.0f),	0,	0),
			FPlane(0,												0,												1,	0),
			FPlane(MappingPanU,										MappingPanV,									0,	1)
			);
}

//
//	UTerrainMaterial::GetDisplacement
//

FLOAT UTerrainMaterial::GetDisplacement(BYTE* DisplacementData,FLOAT U,FLOAT V) const
{
	if(DisplacementMap && DisplacementMap->Format == PF_A8R8G8B8)
	{
		FTexture2DMipMap&	MipMap = DisplacementMap->Mips(0);
	
		INT		IntX = appTrunc(U * MipMap.SizeX),
				IntY = appTrunc(V * MipMap.SizeY);

		// Handle wrapping
		INT WrappedX = IntX % MipMap.SizeX;
		if (WrappedX < 0)
		{
			WrappedX += MipMap.SizeX;
		}
		INT WrappedY = IntY % MipMap.SizeY;
		if (WrappedY < 0)
		{
			WrappedY += MipMap.SizeY;
		}

		check(WrappedX >= 0);
		check(WrappedY >= 0);

		FLOAT	BaseDisplacement = ((FColor*)DisplacementData)[WrappedY * MipMap.SizeX + WrappedX].A / 255.0f;

		return (BaseDisplacement * (DisplacementMap->UnpackMax[3] - DisplacementMap->UnpackMin[3]) + DisplacementMap->UnpackMin[3]) * DisplacementScale;
	}
	else
	if(DisplacementMap && DisplacementMap->Format == PF_G8)
	{
		FTexture2DMipMap&	MipMap = DisplacementMap->Mips(0);
	
		INT		IntX = appTrunc(U * MipMap.SizeX),
				IntY = appTrunc(V * MipMap.SizeY);

		// Handle wrapping
		INT WrappedX = IntX % MipMap.SizeX;
		if (WrappedX < 0)
		{
			WrappedX += MipMap.SizeX;
		}
		INT WrappedY = IntY % MipMap.SizeY;
		if (WrappedY < 0)
		{
			WrappedY += MipMap.SizeY;
		}

		check(WrappedX >= 0);
		check(WrappedY >= 0);

		FLOAT BaseDisplacement = ((BYTE*)DisplacementData)[WrappedY * MipMap.SizeX + WrappedX] / 255.0f;

		return (BaseDisplacement * (DisplacementMap->UnpackMax[3] - DisplacementMap->UnpackMin[3]) + DisplacementMap->UnpackMin[3]) * DisplacementScale;
	}

	return 0.0f;
}

//
//	FTerrainWeightedMaterial::FTerrainWeightedMaterial
//

FTerrainWeightedMaterial::FTerrainWeightedMaterial()
{
}

FTerrainWeightedMaterial::FTerrainWeightedMaterial(ATerrain* InTerrain,const TArray<BYTE>& InData,UTerrainMaterial* InMaterial,
	UBOOL InHighlighted, const FColor& InHighlightColor, UBOOL bInWireframeHighlighted, const FColor& InWireframeColor)
	: Data(InData)
	, Terrain(InTerrain)
	, Highlighted(InHighlighted)
	, HighlightColor(InHighlightColor)
	, bWireframeHighlighted(bInWireframeHighlighted)
	, WireframeColor(InWireframeColor)
	, Material(InMaterial)
{
	SizeX = Terrain_GetPaddedSize(Terrain->NumVerticesX);
	SizeY = Terrain_GetPaddedSize(Terrain->NumVerticesY);

	HighlightColor.A	= 64;
	WireframeColor.A	= 64;
}

FTerrainWeightedMaterial::~FTerrainWeightedMaterial()
{
}

//
//	FTerrainWeightedMaterial::FilteredWeight
//
FLOAT FTerrainWeightedMaterial::FilteredWeight(INT IntX,FLOAT FracX,INT IntY,FLOAT FracY) const
{
	if ((IntX < (INT)SizeX - 1) && (IntY < (INT)SizeY - 1))
	{
		return BiLerp(
				(FLOAT)Weight(IntX,IntY),
				(FLOAT)Weight(IntX + 1,IntY),
				(FLOAT)Weight(IntX,IntY + 1),
				(FLOAT)Weight(IntX + 1,IntY + 1),
				FracX,
				FracY
				);
	}
	else
	if (IntX < (INT)SizeX - 1)
	{
		return Lerp(
				(FLOAT)Weight(IntX,IntY),
				(FLOAT)Weight(IntX + 1,IntY),
				FracX
				);
	}
	else
	if(IntY < (INT)SizeY - 1)
	{
		return Lerp(
				(FLOAT)Weight(IntX,IntY),
				(FLOAT)Weight(IntX,IntY + 1),
				FracY
				);
	}
	else
	{
		return (FLOAT)Weight(IntX,IntY);
	}
}


IMPLEMENT_CLASS(UTerrainWeightMapTexture);

void UTerrainWeightMapTexture::Serialize( FArchive& Ar )
{
	// Serialize the indices of the source weight maps
//	class ATerrain*				Terrain;
//	FTerrainWeightedMaterial*	WeightedMaterials[4];
	Super::Serialize(Ar);
}

void UTerrainWeightMapTexture::PostLoad()
{
	Super::PostLoad();
}

/** 
 * Returns a one line description of an object for viewing in the generic browser
 */
FString UTerrainWeightMapTexture::GetDesc()
{
	return FString::Printf(TEXT("WeightMap: %dx%d [%s]"), SizeX, SizeY, GPixelFormats[Format].Name);
}

/** 
 * Returns detailed info to populate listview columns
 */
FString UTerrainWeightMapTexture::GetDetailedDescription( INT InIndex )
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

void UTerrainWeightMapTexture::Initialize(ATerrain* InTerrain)
{
	check(InTerrain);
	ParentTerrain = InTerrain;
	
	INT UseSizeX = Terrain_GetPaddedSize(ParentTerrain->NumVerticesX);
	INT UseSizeY = Terrain_GetPaddedSize(ParentTerrain->NumVerticesY);
	NeverStream = TRUE;
	RequestedMips = 1;
	CompressionNone = 1;
	SRGB = 0;
	UTexture2D::Init(UseSizeX, UseSizeY, PF_A8R8G8B8);

	// Fill in the data...
}

void UTerrainWeightMapTexture::UpdateData()
{
	check(Mips.Num() > 0);

	FTexture2DMipMap* MipMap = &(Mips(0));

	void* Data = MipMap->Data.Lock(LOCK_READ_WRITE);
	INT DestStride = MipMap->SizeX * 4;

	BYTE* DestWeight = (BYTE*)Data;

	for (INT Y = 0; Y < ParentTerrain->NumVerticesY; Y++)
	{
		for (INT X = 0; X < ParentTerrain->NumVerticesX; X++)
		{
			INT DataIndex = 4 * X;
			INT SubIndex;
			for (SubIndex = 0; SubIndex < WeightedMaterials.Num(); SubIndex++)
			{
				FTerrainWeightedMaterial* WeightedMat = WeightedMaterials(SubIndex);
				if (WeightedMat)
				{
					DestWeight[DataIndex++] = WeightedMat->Data(Y * SizeX + X);
				}
				else
				{
					DestWeight[DataIndex++] = 0;
				}
			}
			// Make sure we fill in everything
			for (; SubIndex < 4; SubIndex++)
			{
				DestWeight[DataIndex++] = 0;
			}
		}
		DestWeight += DestStride;
	}

	MipMap->Data.Unlock();
}


FArchive& operator<<(FArchive& Ar,FTerrainWeightedMaterial& M)
{
	check(!Ar.IsSaving() && !Ar.IsLoading()); // Weight maps shouldn't be stored.
	return Ar << M.Terrain << M.Data << M.Material << M.Highlighted;
}

//
//	FPatchSampler::FPatchSampler
//

FPatchSampler::FPatchSampler(UINT InMaxTesselation):
	MaxTesselation(InMaxTesselation)
{
	for(UINT I = 0;I <= MaxTesselation;I++)
	{
		FLOAT	T = (FLOAT)I / (FLOAT)MaxTesselation;
		CubicBasis[I][0] = -0.5f * (T * T * T - 2.0f * T * T + T);
		CubicBasis[I][1] = (2.0f * T * T * T - 3.0f * T * T + 1.0f) - 0.5f * (T * T * T - T * T);
		CubicBasis[I][2] = (-2.0f * T * T * T + 3.0f * T * T) + 0.5f * (T * T * T - 2.0f * T * T + T);
		CubicBasis[I][3] = +0.5f * (T * T * T - T * T);

		CubicBasisDeriv[I][0] = 0.5f * (-1.0f + 4.0f * T - 3.0f * T * T);
		CubicBasisDeriv[I][1] = -6.0f * T + 6.0f * T * T + 0.5f * (2.0f * T - 3.0f * T * T);
		CubicBasisDeriv[I][2] = +6.0f * T - 6.0f * T * T + 0.5f * (1.0f - 4.0f * T + 3.0f * T * T);
		CubicBasisDeriv[I][3] = 0.5f * (-2.0f * T + 3.0f * T * T);
	}
}

//
//	FPatchSampler::Sample
//

FLOAT FPatchSampler::Sample(const FTerrainPatch& Patch,UINT X,UINT Y) const
{
	return Cubic(
			Cubic(Patch.Heights[0][0],Patch.Heights[1][0],Patch.Heights[2][0],Patch.Heights[3][0],X),
			Cubic(Patch.Heights[0][1],Patch.Heights[1][1],Patch.Heights[2][1],Patch.Heights[3][1],X),
			Cubic(Patch.Heights[0][2],Patch.Heights[1][2],Patch.Heights[2][2],Patch.Heights[3][2],X),
			Cubic(Patch.Heights[0][3],Patch.Heights[1][3],Patch.Heights[2][3],Patch.Heights[3][3],X),
			Y
			);
}

//
//	FPatchSampler::SampleDerivX
//

FLOAT FPatchSampler::SampleDerivX(const FTerrainPatch& Patch,UINT X,UINT Y) const
{
#if 0 // Return a linear gradient, so tesselation changes don't affect lighting.
	return Cubic(
			CubicDeriv(Patch.Heights[0][0],Patch.Heights[1][0],Patch.Heights[2][0],Patch.Heights[3][0],X),
			CubicDeriv(Patch.Heights[0][1],Patch.Heights[1][1],Patch.Heights[2][1],Patch.Heights[3][1],X),
			CubicDeriv(Patch.Heights[0][2],Patch.Heights[1][2],Patch.Heights[2][2],Patch.Heights[3][2],X),
			CubicDeriv(Patch.Heights[0][3],Patch.Heights[1][3],Patch.Heights[2][3],Patch.Heights[3][3],X),
			Y
			);
#else
	return Lerp(
			Lerp(Patch.Heights[2][1] - Patch.Heights[0][1],Patch.Heights[3][1] - Patch.Heights[1][1],(FLOAT)X / (FLOAT)MaxTesselation),
			Lerp(Patch.Heights[2][2] - Patch.Heights[0][2],Patch.Heights[3][2] - Patch.Heights[1][2],(FLOAT)X / (FLOAT)MaxTesselation),
			(FLOAT)Y / (FLOAT)MaxTesselation
			) / 2.0f;
#endif
}

//
//	FPatchSampler::SampleDerivY
//

FLOAT FPatchSampler::SampleDerivY(const FTerrainPatch& Patch,UINT X,UINT Y) const
{
#if 0 // Return a linear gradient, so tesselation changes don't affect lighting.
	return CubicDeriv(
			Cubic(Patch.Heights[0][0],Patch.Heights[1][0],Patch.Heights[2][0],Patch.Heights[3][0],X),
			Cubic(Patch.Heights[0][1],Patch.Heights[1][1],Patch.Heights[2][1],Patch.Heights[3][1],X),
			Cubic(Patch.Heights[0][2],Patch.Heights[1][2],Patch.Heights[2][2],Patch.Heights[3][2],X),
			Cubic(Patch.Heights[0][3],Patch.Heights[1][3],Patch.Heights[2][3],Patch.Heights[3][3],X),
			Y
			);
#else
	return Lerp(
			Lerp(Patch.Heights[1][2] - Patch.Heights[1][0],Patch.Heights[2][2] - Patch.Heights[2][0],(FLOAT)X / (FLOAT)MaxTesselation),
			Lerp(Patch.Heights[1][3] - Patch.Heights[1][1],Patch.Heights[2][3] - Patch.Heights[2][1],(FLOAT)X / (FLOAT)MaxTesselation),
			(FLOAT)Y / (FLOAT)MaxTesselation
			) / 2.0f;
#endif
}

//
//	FPatchSampler::Cubic
//

FLOAT FPatchSampler::Cubic(FLOAT P0,FLOAT P1,FLOAT P2,FLOAT P3,UINT I) const
{
	return	P0 * CubicBasis[I][0] +
			P1 * CubicBasis[I][1] +
			P2 * CubicBasis[I][2] +
			P3 * CubicBasis[I][3];
}

//
//	FPatchSampler::CubicDeriv
//

FLOAT FPatchSampler::CubicDeriv(FLOAT P0,FLOAT P1,FLOAT P2,FLOAT P3,UINT I) const
{
	return	P0 * CubicBasisDeriv[I][0] +
			P1 * CubicBasisDeriv[I][1] +
			P2 * CubicBasisDeriv[I][2] +
			P3 * CubicBasisDeriv[I][3];
}

void UTerrainMaterial::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange( PropertyThatChanged );

	// Determine if the property that changed was a property that only affects foliage.
	const UBOOL bIsFoliageProperty =
		PropertyThatChanged && (
			(PropertyThatChanged->GetOuter()->GetName() == TEXT("TerrainFoliageMesh")) ||
			PropertyThatChanged->GetName() == TEXT("FoliageMeshes")
			);

	// Find any terrain actors using this material.
	for (FActorIterator ActorIt; ActorIt; ++ActorIt)
	{
		ATerrain* Terrain = Cast<ATerrain>(*ActorIt);
		if (Terrain != NULL)
		{
			if(bIsFoliageProperty)
			{
				// Reattach terrain components.
				Terrain->ClearComponents();
				Terrain->ConditionalUpdateComponents();
			}
			else
			{
				Terrain->UpdateTerrainMaterial(this);
				Terrain->PostEditChange(PropertyThatChanged);
			}
		}
	}

	GCallbackEvent->Send(CALLBACK_RefreshEditor_TerrainBrowser);
}

void UTerrainMaterial::PostLoad()
{
	Super::PostLoad();

	// Fix terrain foliage that was created before the MinUniformScale, MaxUniformScale, and MinThinningRadius properties were added.
	if(GetLinker() && GetLinker()->Ver() < VER_FIX_DEFAULT_FOLIAGE_PARAMETERS)
	{
		for(INT MeshIndex = 0;MeshIndex < FoliageMeshes.Num();MeshIndex++)
		{
			FTerrainFoliageMesh& Mesh = FoliageMeshes(MeshIndex);
			if(Mesh.MinUniformScale == 0.0f && Mesh.MaxUniformScale == 0.0f && Mesh.MinThinningRadius == 0.0f)
			{
				Mesh.MinUniformScale = Mesh.MaxUniformScale = 1.0f;
				Mesh.MinThinningRadius = 1024.0f;
			}
		}
	}
}

/**
 *
 */
void UTerrainLayerSetup::PostEditChange(UProperty* PropertyThatChanged)
{
	UBOOL bRecacheWeightMaps = FALSE;
	UBOOL bRecacheMaterial = TRUE;
	if (PropertyThatChanged != NULL)
	{
		debugf(TEXT("TerrainLayerSetup: PostEditChange for %s"), *(PropertyThatChanged->GetName()));
		if ((appStricmp(*(PropertyThatChanged->GetName()), TEXT("UseNoise")) == 0) ||
			(appStricmp(*(PropertyThatChanged->GetName()), TEXT("NoiseScale")) == 0) ||
			(appStricmp(*(PropertyThatChanged->GetName()), TEXT("NoisePercent")) == 0) ||
			(appStricmp(*(PropertyThatChanged->GetName()), TEXT("Base")) == 0) ||
			(appStricmp(*(PropertyThatChanged->GetName()), TEXT("NoiseAmount")) == 0) ||
			(appStricmp(*(PropertyThatChanged->GetName()), TEXT("Alpha")) == 0))
		{
			bRecacheWeightMaps = TRUE;
			bRecacheMaterial = FALSE;
		}
		else
		if (appStricmp(*(PropertyThatChanged->GetName()), TEXT("Enabled")) == 0)
		{
			bRecacheWeightMaps = TRUE;
		}
	}

	// Limit to 64 materials in layer.
	if( Materials.Num() > 64 )
	{
		appMsgf( AMT_OK, TEXT("Cannot use more than 64 materials") );
		Materials.Remove( 64, Materials.Num() - 64 );
	}

	// Find any terrain actors using this layer setup.
	for (FActorIterator ActorIt; ActorIt; ++ActorIt)
	{
		ATerrain* Terrain = Cast<ATerrain>(*ActorIt);
		if (Terrain != NULL)
		{
			for (INT LayerIndex = 0; LayerIndex < Terrain->Layers.Num(); LayerIndex++)
			{
				if (Terrain->Layers(LayerIndex).Setup == this)
				{
					if (bRecacheWeightMaps == TRUE)
					{
						Terrain->ClearWeightMaps();
						Terrain->CacheWeightMaps(0, 0, Terrain->NumVerticesX - 1, Terrain->NumVerticesY - 1);
						Terrain->TouchWeightMapResources();
					}
					
					if (bRecacheMaterial == TRUE)
					{
						Terrain->UpdateLayerSetup(this);
					}
					break;
				}
			}
		}
	}
	Super::PostEditChange( PropertyThatChanged );

	GCallbackEvent->Send(CALLBACK_RefreshEditor_TerrainBrowser);
}

/**
 * Called after serialization. Ensures that there are only 64 materials.
 */
void UTerrainLayerSetup::PostLoad()
{
	Super::PostLoad();
	// Limit to 64 materials in layer.
	if( Materials.Num() > 64 )
	{
		debugf(TEXT("%s has %i materials but 64 is the new allowed maximum. Discarding extra ones."),*GetPathName(),Materials.Num());
		Materials.Remove( 64, Materials.Num() - 64 );
	}

	for (INT MaterialIndex = 0; MaterialIndex < Materials.Num(); MaterialIndex++)
	{
		FTerrainFilteredMaterial* TFiltMat = &(Materials(MaterialIndex));
		UTerrainMaterial* TMat = TFiltMat->Material;
		if (TMat)
		{
			TMat->ConditionalPostLoad();
			if (TMat->Material)
			{
				TMat->Material->ConditionalPostLoad();
			}
		}
	}
}


void UTerrainLayerSetup::SetMaterials(const TArray<FTerrainFilteredMaterial>& NewMaterials)
{
return;

	Materials = NewMaterials;

	// update the local to mapping transform for the new materials
	for (INT i = 0; i < Materials.Num(); i++)
	{
		if (Materials(i).Material != NULL)
		{
			Materials(i).Material->UpdateMappingTransform();
		}
	}

	// update terrain actors that reference us for the new materials
	for (FActorIterator It; It; ++It)
	{
		ATerrain* Terrain = Cast<ATerrain>(*It);
		if (Terrain != NULL)
		{
			for (INT i = 0; i < Terrain->Layers.Num(); i++)
			{
				if (Terrain->Layers(i).Setup == this)
				{
					// find out if all the new materials are already in the WeightedMaterials array
					// if so, we can do a partial weightmap update to save CPU
					UBOOL bFullWeightMap = false;
					for (INT j = 0; j < Materials.Num() && !bFullWeightMap; j++)
					{
						UBOOL bFound = false;
						for (INT k = 0; k < Terrain->WeightedMaterials.Num() && !bFound; k++)
						{
							FTerrainWeightedMaterial& WeightedMat = Terrain->WeightedMaterials(k);
							if ((Materials(j).Material == WeightedMat.Material) && (WeightedMat.Highlighted == Terrain->Layers(i).Highlighted))
							{
								bFound = true;
							}
						}
						bFullWeightMap = !bFound;
					}
					if (bFullWeightMap)
					{
						Terrain->CacheWeightMaps(0, 0, Terrain->NumVerticesX - 1, Terrain->NumVerticesY - 1);
					}
					else
					{
						Terrain->CacheWeightMaps(Terrain->Layers(i).MinX, Terrain->Layers(i).MinY, Terrain->Layers(i).MaxX, Terrain->Layers(i).MaxY);
					}
					Terrain->TouchWeightMapResources();

					// update displacement map
					Terrain->CacheDisplacements(Terrain->Layers(i).MinX, Terrain->Layers(i).MinY, Terrain->Layers(i).MaxX, Terrain->Layers(i).MaxY);

					// update all the terrain components that are relevant to the terrain area this layer affects
					for (INT j = 0; j < Terrain->TerrainComponents.Num(); j++)
					{
						UTerrainComponent* Component = Terrain->TerrainComponents(j);
						if (Component &&
							(Component->SectionBaseX + Component->SectionSizeX >= Terrain->Layers(i).MinX) && 
							(Component->SectionBaseX <= Terrain->Layers(i).MaxX) &&
							(Component->SectionBaseY + Component->SectionSizeY >= Terrain->Layers(i).MinY) && 
							(Component->SectionBaseY <= Terrain->Layers(i).MaxY))
						{
							Component->UpdatePatchBatches();
						}
					}

					break;
				}
			}
		}
	}
}
