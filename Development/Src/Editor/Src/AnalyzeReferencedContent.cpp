/** 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EditorPrivate.h"
#include "EnginePhysicsClasses.h"
#include "EngineParticleClasses.h"
#include "EngineAnimClasses.h"

#include "PerfMem.h"
#include "AnimationEncodingFormat.h"
#include "AnimationUtils.h"

IMPLEMENT_CLASS(UAnalyzeReferencedContentCommandlet);

/*-----------------------------------------------------------------------------
UAnalyzeReferencedContentCommandlet
-----------------------------------------------------------------------------*/
void FAnalyzeReferencedContentStat::WriteOutAllAvailableStatData( const FString& CSVDirectory )
{
	if ((IgnoreObjects & IGNORE_StaticMesh) == 0)
	{
		WriteOutCSVs<FStaticMeshStats>( ResourceNameToStaticMeshStats, CSVDirectory, TEXT( "StaticMeshStats" ) );
	}

	if ((IgnoreObjects & IGNORE_SkeletalMesh) == 0)
	{
		WriteOutCSVs<FSkeletalMeshStats>( ResourceNameToSkeletalMeshStats, CSVDirectory, TEXT( "SkeletalMeshStats" ) );
	}

	if ((IgnoreObjects & IGNORE_Texture) == 0)
	{
		WriteOutCSVs<FTextureStats>( ResourceNameToTextureStats, CSVDirectory, TEXT( "TextureStats" ) );
	}

	if ((IgnoreObjects & IGNORE_Material) == 0)
	{
		WriteOutCSVs<FMaterialStats>( ResourceNameToMaterialStats, CSVDirectory, TEXT( "MaterialStats" ) );
	}

	if ((IgnoreObjects & IGNORE_Particle) == 0)
	{
		WriteOutCSVs<FParticleStats>( ResourceNameToParticleStats, CSVDirectory, TEXT( "ParticleStats" ) );
	}

	if ((IgnoreObjects & IGNORE_Anim) == 0)
	{
		WriteOutCSVs<FAnimSequenceStats>( ResourceNameToAnimStats, CSVDirectory, TEXT( "AnimStats" ) );
	}

	if ((IgnoreObjects & IGNORE_FaceFXAnimSet) == 0)
	{
		WriteOutCSVs<FFaceFXAnimSetStats>( ResourceNameToFaceFXAnimSetStats, CSVDirectory, TEXT( "FaceFXAnimSetStats" ) );
	}

	if ((IgnoreObjects & IGNORE_StaticMeshActor) == 0)
	{
		WriteOutCSVs<FLightingOptimizationStats>( ResourceNameToLightingStats, CSVDirectory, TEXT( "LightMapStats" ) );
	}

	if ((IgnoreObjects & IGNORE_Particle) == 0)
	{
		WriteOutCSVs<FTextureToParticleSystemStats>( ResourceNameToTextureToParticleSystemStats, CSVDirectory, TEXT( "TextureToParticleStats" ) );
	}

	if ((IgnoreObjects & IGNORE_SoundCue) == 0)
	{
		WriteOutCSVs<FSoundCueStats>( ResourceNameToSoundCueStats, CSVDirectory, TEXT( "SoundCueStats" ) );
	}

	if ((IgnoreObjects & IGNORE_ShadowMap) == 0 )
	{
		WriteOutCSVs<FShadowMap1DStats>( ResourceNameToShadowMap1DStats, CSVDirectory, TEXT( "ShadowMap1DStats" ) );
		WriteOutCSVs<FShadowMap2DStats>( ResourceNameToShadowMap2DStats, CSVDirectory, TEXT( "ShadowMap2DStats" ) );
	}		

	// Write PerLevel Data
	if ((IgnoreObjects & IGNORE_StaticMesh) == 0)
	{
		WriteOutCSVsPerLevel<FStaticMeshStats>(ResourceNameToStaticMeshStats, CSVDirectory, TEXT("StaticMeshStats"));
	}

	if ((IgnoreObjects & IGNORE_SoundCue) == 0)
	{
		WriteOutCSVsPerLevel<FSoundCueStats>(ResourceNameToSoundCueStats, CSVDirectory, TEXT("SoundCueStats"));
	}

	if ((IgnoreObjects & IGNORE_Anim) == 0)
	{
		WriteOutCSVsPerLevel<FAnimSequenceStats>(ResourceNameToAnimStats, CSVDirectory, TEXT("AnimStats"));
	}

	if ((IgnoreObjects & IGNORE_SkeletalMesh) == 0)
	{
		WriteOutCSVsPerLevel<FSkeletalMeshStats>(ResourceNameToSkeletalMeshStats, CSVDirectory, TEXT("SkeletalMeshStats"));
	}

	if ((IgnoreObjects & IGNORE_FaceFXAnimSet) == 0)
	{
		WriteOutCSVsPerLevel<FFaceFXAnimSetStats>( ResourceNameToFaceFXAnimSetStats, CSVDirectory, TEXT("FaceFXAnimSetStats"));
	}

#if 0
	debugf(TEXT("%s"),*FStaticMeshStats::GetCSVHeaderRow());
	for( TMap<FString,FStaticMeshStats>::TIterator It(ResourceNameToStaticMeshStats); It; ++ It )
	{
		const FStaticMeshStats& StatsEntry = It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}

	debugf(TEXT("%s"),*FTextureStats::GetCSVHeaderRow());
	for( TMap<FString,FTextureStats>::TIterator It(ResourceNameToTextureStats); It; ++ It )
	{
		const FTextureStats& StatsEntry	= It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}

	debugf(TEXT("%s"),*FMaterialStats::GetCSVHeaderRow());
	for( TMap<FString,FMaterialStats>::TIterator It(ResourceNameToMaterialStats); It; ++ It )
	{
		const FMaterialStats& StatsEntry = It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}
#endif
}

/**
* This function fills up MapsUsedIn and LevelNameToInstanceCount if bAddPerLevelDataMap is TRUE. 
*
* @param	LevelPackage	Level Package this object belongs to
* @param	bAddPerLevelDataMap	Set this to be TRUE if you'd like to collect this stat per level (in the Level folder)
* 
*/
void FAnalyzeReferencedContentStat::FAssetStatsBase::AddLevelInfo( UPackage* LevelPackage, UBOOL bAddPerLevelDataMap )
{
	if ( LevelPackage != NULL )
	{
		// set MapUsedIn
		MapsUsedIn.AddUniqueItem( LevelPackage->GetFullName() );

		// if to collect data per level, save lever map data
		if ( bAddPerLevelDataMap )
		{
			UINT* NumInst = LevelNameToInstanceCount.Find( LevelPackage->GetOutermost()->GetName() );
			if( NumInst != NULL )
			{
				LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), ++(*NumInst) );
			}
			else
			{
				LevelNameToInstanceCount.Set( LevelPackage->GetOutermost()->GetName(), 1 );
			}
		}
	}
}

/** Constructor, initializing all members. */
FAnalyzeReferencedContentStat::FStaticMeshStats::FStaticMeshStats( UStaticMesh* StaticMesh )
:	ResourceType(StaticMesh->GetClass()->GetName())
,	ResourceName(StaticMesh->GetPathName())
,	NumInstances(0)
,	NumTriangles(0)
,	NumSections(0)
,   NumConvexPrimitives(0)
,   bUsesSimpleRigidBodyCollision(StaticMesh->UseSimpleRigidBodyCollision)
,   NumElementsWithCollision(0)
,	bIsReferencedByScript(FALSE)
,   bIsReferencedByParticles(FALSE)
,	ResourceSize(StaticMesh->GetResourceSize())
,   bIsMeshNonUniformlyScaled(FALSE)
,   bShouldConvertBoxColl(FALSE)
{
	// Update triangle and section counts.
	for( INT ElementIndex=0; ElementIndex<StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
	{
		const FStaticMeshElement& StaticMeshElement = StaticMesh->LODModels(0).Elements(ElementIndex);
		NumElementsWithCollision += StaticMeshElement.EnableCollision ? 1 : 0;
		NumTriangles += StaticMeshElement.NumTriangles;
		NumSections++;
	}

	if(StaticMesh->BodySetup)
	{
		NumConvexPrimitives = StaticMesh->BodySetup->AggGeom.ConvexElems.Num();
	}
}



/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FStaticMeshStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumInstances,NumTriangles,NumSections,NumSectionsWithCollision,UsesSimpleRigidBodyCollision,ConvexCollisionPrims,NumMapsUsedIn,ResourceSize,ScalesUsed,NonUniformScale,ShouldConvertBoxColl,ParticleMesh,Instanced Triangles") LINE_TERMINATOR;

	// we would like to have this in the "Instanced Triangles" column but the commas make it auto parsed into the next column for the CSV
	//=Table1[[#This Row],[NumInstances]]*Table1[[#This Row],[NumTriangles]]
}


/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FStaticMeshStats::ToCSV() const
{

	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%d,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,
		NumInstances,
		NumTriangles,
		NumSections,
		NumElementsWithCollision,
		bUsesSimpleRigidBodyCollision,
		NumConvexPrimitives,
		MapsUsedIn.Num(),
		ResourceSize,
		UsedAtScales.Num(),
		bIsMeshNonUniformlyScaled,
		bShouldConvertBoxColl,
		bIsReferencedByParticles,
		LINE_TERMINATOR);
}

/** This takes a LevelName and then looks for the number of Instances of this StatMesh used within that level **/
FString FAnalyzeReferencedContentStat::FStaticMeshStats::ToCSV( const FString& LevelName ) const
{
	UINT* NumInst = const_cast<UINT*>(LevelNameToInstanceCount.Find( LevelName ));
	if( NumInst == NULL )
	{
		*NumInst = 0;
	}

	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%d,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,
		*NumInst,
		NumTriangles,
		NumSections,
		NumElementsWithCollision,
		bUsesSimpleRigidBodyCollision,
		NumConvexPrimitives,
		MapsUsedIn.Num(),
		ResourceSize,
		UsedAtScales.Num(),
		bIsMeshNonUniformlyScaled,
		bShouldConvertBoxColl,
		bIsReferencedByParticles,
		LINE_TERMINATOR);
}



/** Constructor, initializing all members. */
FAnalyzeReferencedContentStat::FSkeletalMeshStats::FSkeletalMeshStats( USkeletalMesh* SkeletalMesh )
:	ResourceType(SkeletalMesh->GetClass()->GetName())
,	ResourceName(SkeletalMesh->GetPathName())
,   NumInstances(0)
,   NumTriangles(0)
,   NumVertices(0)
,   NumRigidVertices(0)
,   NumSoftVertices(0)
,   NumSections(0)
,   NumChunks(0)
,	MaxBoneInfluences(0)
,   NumActiveBoneIndices(0)
,   NumRequiredBones(0)
,   NumMaterials(0)
,   bUsesPerPolyBoneCollision(FALSE)
,	bIsReferencedByScript(FALSE)
,   bIsReferencedByParticles(FALSE)
,	ResourceSize(SkeletalMesh->GetResourceSize())
{
	NumTriangles = SkeletalMesh->LODModels(0).GetTotalFaces();
	NumSections = SkeletalMesh->LODModels(0).Sections.Num();
	NumChunks = SkeletalMesh->LODModels(0).Chunks.Num();
	for (INT ChunkIndex = 0; ChunkIndex < NumChunks; ChunkIndex++)
	{
		const FSkelMeshChunk& SkelMeshChunk = SkeletalMesh->LODModels(0).Chunks(ChunkIndex);
		NumRigidVertices += SkelMeshChunk.GetNumRigidVertices();
		NumSoftVertices += SkelMeshChunk.GetNumSoftVertices();
		NumVertices += SkelMeshChunk.GetNumVertices();

		// If this keeps coming up zero, need to call CalcMaxBoneInfluences first
		MaxBoneInfluences = (MaxBoneInfluences > SkelMeshChunk.MaxBoneInfluences ?
MaxBoneInfluences : SkelMeshChunk.MaxBoneInfluences);
	}

	NumActiveBoneIndices = SkeletalMesh->LODModels(0).ActiveBoneIndices.Num();
	NumRequiredBones = SkeletalMesh->LODModels(0).RequiredBones.Num();
	NumMaterials = SkeletalMesh->Materials.Num();

	if ((SkeletalMesh->bUseSimpleLineCollision == 0) &&
		(SkeletalMesh->bUseSimpleBoxCollision == 0) &&
		(SkeletalMesh->PerPolyCollisionBones.Num() > 0))
	{
		bUsesPerPolyBoneCollision = TRUE;
	}
}



/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FSkeletalMeshStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumInstances,NumTriangles,NumVertices,NumRigidVertices,NumSoftVertices,NumSections,NumChunks,MaxBoneInfluences,NumActiveBoneIndices,NumRequiredBones,NumMaterials,bUsesPerPolyBoneCollision,bIsReferencedByScript,bIsReferencedByParticles,ResourceSize") LINE_TERMINATOR;
}


/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FSkeletalMeshStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,
		NumInstances,
		NumTriangles,
		NumVertices,
		NumRigidVertices,
		NumSoftVertices,
		NumSections,
		NumChunks,
		MaxBoneInfluences,
		NumActiveBoneIndices,
		NumRequiredBones,
		NumMaterials,
		bUsesPerPolyBoneCollision,
		bIsReferencedByScript,
		bIsReferencedByParticles,
		ResourceSize,
		LINE_TERMINATOR);
}

/** This takes a LevelName and then looks for the number of Instances of this StatMesh used within that level **/
FString FAnalyzeReferencedContentStat::FSkeletalMeshStats::ToCSV( const FString& LevelName ) const
{
	UINT* NumInst = const_cast<UINT*>(LevelNameToInstanceCount.Find( LevelName ));
	if( NumInst == NULL )
	{
		*NumInst = 0;
	}

	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,
		*NumInst,
		NumInstances,
		NumTriangles,
		NumVertices,
		NumRigidVertices,
		NumSoftVertices,
		NumSections,
		NumChunks,
		MaxBoneInfluences,
		NumActiveBoneIndices,
		NumRequiredBones,
		NumMaterials,
		bUsesPerPolyBoneCollision,
		bIsReferencedByScript,
		bIsReferencedByParticles,
		ResourceSize,
		LINE_TERMINATOR);
}




//Initialization of static member in FLightingOptimizationInfo
const INT FAnalyzeReferencedContentStat::FLightingOptimizationStats::LightMapSizes[NumLightmapTextureSizes] = { 256, 128, 64, 32 };
static const INT BytesUsedThreshold = 5000;

/**
*   Calculate the memory required to light a mesh with given NumVertices using vertex lighting
*/
INT FAnalyzeReferencedContentStat::FLightingOptimizationStats::CalculateVertexLightingBytesUsed(INT NumVertices)
{
	//3 color channels are  (3 colors * 4 bytes/color = 12 bytes)
	//1 color channel is    (1 color * 4 bytes/color = 4 bytes)
	const INT VERTEX_LIGHTING_DATA_SIZE = GSystemSettings.bAllowDirectionalLightMaps ? sizeof(FQuantizedDirectionalLightSample) : sizeof(FQuantizedSimpleLightSample);

	//BytesUsed = (Number of Vertices) * sizeof(VertexLightingData)
	INT BytesUsed = NumVertices * VERTEX_LIGHTING_DATA_SIZE;

	return BytesUsed;
}

/** Assuming DXT1 lightmaps...
*   4 bits/pixel * width * height = Highest MIP Level * 1.333 MIP Factor for approx usage for a full mip chain
*   Either 1 or 3 textures if we're doing simple or directional (3-axis) lightmap
*   Most lightmaps require a second UV channel which is probably an extra 4 bytes (2 floats compressed to SHORT) 
*/
INT FAnalyzeReferencedContentStat::FLightingOptimizationStats::CalculateLightmapLightingBytesUsed(INT Width, INT Height, INT NumVertices, INT UVChannelIndex)
{
	if (Width <= 0 || Height <= 0 || NumVertices <= 0)
	{
		return 0;
	}

	const FLOAT MIP_FACTOR = 4.0f / 3.0f;

	FLOAT BYTES_PER_PIXEL = 0.0f;
	if (GSystemSettings.bAllowDirectionalLightMaps)
	{
		//DXT1 4bits/pixel * 4/3 mipfactor * 3 channels = 16 bits = 2 bytes / pixel
		BYTES_PER_PIXEL = 0.5f * MIP_FACTOR * NUM_DIRECTIONAL_LIGHTMAP_COEF;
	}
	else
	{      
		//DXT1 4bits/pixel * 4/3 mipfactor * 1 channel = 16/3 bits = 0.6666 bytes / pixel
		BYTES_PER_PIXEL = 0.5f * MIP_FACTOR * NUM_SIMPLE_LIGHTMAP_COEF;
	}

	//BytesUsed = (SizeOfTexture) + (SizeOfUVData)
	//SizeOfTexture = (Width * Height * BytesPerTexel) * MIP_FACTOR
	//SizeOfUVData = (Number of Vertices * SizeOfUVCoordData)
	INT SizeOfTexture = Width * Height * BYTES_PER_PIXEL;

	INT BytesUsed = 0;
	if ( TRUE )
	{
		BytesUsed = SizeOfTexture;
	}
	else
	{
		//I'm told by Dan that most static meshes will probably already have a 2nd uv channel and it wouldn't necessarily go away
		//with the addition/subtraction of a light map, otherwise we can reenable this
		const FLOAT UV_COORD_DATA_SIZE = UVChannelIndex * 2 * sizeof(SHORT); //Index * (Tuple / UVChannel) * (2 bytes / Tuple)
		INT SizeOfUVData = NumVertices * UV_COORD_DATA_SIZE;
		BytesUsed = SizeOfTexture + SizeOfUVData;
	}

	return BytesUsed;
}

/** 
*	For a given list of parameters, compute a full spread of potential savings values using vertex light, or 256, 128, 64, 32 pixel square light maps
*  @param LMType - Current type of lighting being used
*  @param NumVertices - Number of vertices in the given mesh
*  @param Width - Width of current lightmap
*  @param Height - Height of current lightmap
*  @param TexCoordIndex - channel index of the uvs currently used for lightmaps
*  @param LOI - A struct to be filled in by the function with the potential savings
*/
void FAnalyzeReferencedContentStat::FLightingOptimizationStats::CalculateLightingOptimizationInfo(ELightMapInteractionType LMType, INT NumVertices, INT Width, INT Height, INT TexCoordIndex, FLightingOptimizationStats& LOStats)
{
	// Current Values
	LOStats.IsType = LMType;
	LOStats.TextureSize = Width;

	if (LMType == LMIT_Vertex)
	{
		LOStats.CurrentBytesUsed = CalculateVertexLightingBytesUsed(NumVertices);
	}
	else if (LMType == LMIT_Texture)
	{
		LOStats.CurrentBytesUsed = CalculateLightmapLightingBytesUsed(Width, Height, NumVertices, TexCoordIndex);
	}

	//Potential savings values
	INT VertexLitBytesUsed = CalculateVertexLightingBytesUsed(NumVertices);

	INT TextureMapBytesUsed[FLightingOptimizationStats::NumLightmapTextureSizes];
	for (INT i=0; i<FLightingOptimizationStats::NumLightmapTextureSizes; i++)
	{
		const INT TexCoordIndexAssumed = 1; //assume it will require 2 texcoord channels to do the lightmap
		TextureMapBytesUsed[i] = CalculateLightmapLightingBytesUsed(LightMapSizes[i], LightMapSizes[i], NumVertices, TexCoordIndexAssumed);
	}

	//Store this all away in a nice little struct
	for (INT i=0; i<NumLightmapTextureSizes; i++)
	{
		LOStats.BytesSaved[i] = LOStats.CurrentBytesUsed - TextureMapBytesUsed[i];
	}

	LOStats.BytesSaved[NumLightmapTextureSizes] = LOStats.CurrentBytesUsed - VertexLitBytesUsed;
}

/** 
* Calculate the potential savings for a given static mesh component by using an alternate static lighting method
*/
FAnalyzeReferencedContentStat::FLightingOptimizationStats::FLightingOptimizationStats(AStaticMeshActor* StaticMeshActor)   :
LevelName(StaticMeshActor->GetOutermost()->GetName())
,	ActorName(StaticMeshActor->GetName())
,	SMName(StaticMeshActor->StaticMeshComponent->StaticMesh->GetName())
,   IsType(LMIT_None)        
,   TextureSize(0)  
,	CurrentBytesUsed(0)
{
	UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->StaticMeshComponent;

	appMemzero(BytesSaved, ARRAYSIZE(BytesSaved));
	if (StaticMeshComponent && StaticMeshComponent->StaticMesh && StaticMeshComponent->HasStaticShadowing())
	{
		UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh;

		INT NumLODModels = StaticMesh->LODModels.Num();
		for (INT LODModelIndex = 0; LODModelIndex < NumLODModels; LODModelIndex++)
		{
			const FStaticMeshRenderData& StaticMeshRenderData = StaticMesh->LODModels(LODModelIndex);

			//Mesh has to have LOD data in order to even consider this calculation?
			if (LODModelIndex < StaticMeshComponent->LODData.Num())
			{
				const FStaticMeshComponentLODInfo& StaticMeshComponentLODInfo = StaticMeshComponent->LODData(LODModelIndex);

				//Again without a lightmap, don't bother?
				if (StaticMeshComponentLODInfo.LightMap != NULL)
				{
					//What is the mesh currently using
					FLightMapInteraction LMInteraction = StaticMeshComponentLODInfo.LightMap->GetInteraction();
					IsType = LMInteraction.GetType();

					INT Width  = 0;
					INT Height = 0;
					//Returns the correct (including overrides) width/height for the lightmap
					StaticMeshComponent->GetLightMapResolution(Width, Height);

					//Get the number of vertices used by this static mesh
					INT NumVertices = StaticMeshRenderData.NumVertices;

					//Get the number of uv coordinates stored in the vertex buffer
					INT TexCoordIndex = StaticMesh->LightMapCoordinateIndex; 

					CalculateLightingOptimizationInfo(IsType, NumVertices, Width, Height, TexCoordIndex, *this);
				}
			}
		} //for each lodmodel
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FLightingOptimizationStats::ToCSV() const
{
	FString CSVString;
	if (CurrentBytesUsed > BytesUsedThreshold)
	{
		FString CurrentType(TEXT("Unknown"));
		if (IsType == LMIT_Vertex)
		{
			CurrentType = TEXT("Vertex");
		}
		else
		{
			CurrentType = FString::Printf(TEXT("%d-Texture"), TextureSize);
		}

		FString BytesSavedString;
		UBOOL FoundSavings = FALSE;
		for (INT i=0; i<NumLightmapTextureSizes + 1; i++)
		{
			if (BytesSaved[i] > 0)
			{
				BytesSavedString += FString::Printf(TEXT(",%1.3f"), (FLOAT)BytesSaved[i] / 1024.0f);
				FoundSavings = TRUE;
			}
			else
			{
				BytesSavedString += TEXT(",");
			}
		}

		if (FoundSavings)
		{
			CSVString = FString::Printf(TEXT("%s,%s,%s,%s,%1.3f"), *LevelName, *ActorName, *SMName, *CurrentType, (FLOAT)CurrentBytesUsed/1024.0f);
			CSVString += BytesSavedString;
			CSVString += LINE_TERMINATOR;
		}
	}

	return CSVString;
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FLightingOptimizationStats::GetCSVHeaderRow()
{
	FString CSVHeaderString = FString::Printf(TEXT("LevelName,StaticMeshActor,StaticMesh,CurrentLightingType,CurrentUsage(kb)"));
	for (INT i=0; i<NumLightmapTextureSizes; i++)
	{
		CSVHeaderString += FString::Printf(TEXT(",%d-Texture"), LightMapSizes[i]);
	}

	CSVHeaderString += FString::Printf(TEXT(",Vertex"));
	CSVHeaderString += LINE_TERMINATOR;
	return CSVHeaderString;
}

/** Constructor, initializing all members */
FAnalyzeReferencedContentStat::FShadowMap1DStats::FShadowMap1DStats(UShadowMap1D* ShadowMap1D)
:	ResourceType(ShadowMap1D->GetClass()->GetName())
,	ResourceName(ShadowMap1D->GetPathName())
,	ResourceSize(0)
,	NumSamples(0)
,	UsedByLight(TEXT("None"))
{
	NumSamples = ShadowMap1D->NumSamples();
	FArchiveCountMem CountAr(ShadowMap1D);
	ResourceSize = CountAr.GetMax();

	// find the light for this shadow map	
	for( TObjectIterator<ULightComponent> It; It; ++It )
	{
		const ULightComponent* Light = *It;
		if( Light->LightGuid == ShadowMap1D->GetLightGuid() )
		{
			if( Light->GetOwner() )
			{
				UsedByLight = Light->GetOwner()->GetName();
			}
			break;
		}
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FShadowMap1DStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%s%s"),
		*ResourceType,
		*ResourceName,						
		ResourceSize,
		NumSamples,		
		*UsedByLight,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FShadowMap1DStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ResourceSize,NumSamples,UsedByLight") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
FAnalyzeReferencedContentStat::FShadowMap2DStats::FShadowMap2DStats(UShadowMap2D* ShadowMap2D)
:	ResourceType(ShadowMap2D->GetClass()->GetName())
,	ResourceName(ShadowMap2D->GetPathName())
,	ShadowMapTexture2D(TEXT("None"))
,	ShadowMapTexture2DSizeX(0)
,	ShadowMapTexture2DSizeY(0)
,	ShadowMapTexture2DFormat(TEXT("None"))
,	UsedByLight(TEXT("None"))
{	
	if( ShadowMap2D->IsValid() )
	{
		UShadowMapTexture2D* ShadowTex2D = ShadowMap2D->GetTexture();
		ShadowMapTexture2DSizeX = ShadowTex2D->SizeX;
		ShadowMapTexture2DSizeY = ShadowTex2D->SizeY;
		ShadowMapTexture2DFormat = FString(GPixelFormats[ShadowTex2D->Format].Name ? GPixelFormats[ShadowTex2D->Format].Name : TEXT("None"));
		ShadowMapTexture2D = ShadowTex2D->GetPathName();
	}

	// find the light for this shadow map	
	for( TObjectIterator<ULightComponent> It; It; ++It )
	{
		const ULightComponent* Light = *It;
		if( Light->LightGuid == ShadowMap2D->GetLightGuid() )
		{
			if( Light->GetOwner() )
			{
				UsedByLight = Light->GetOwner()->GetName();
			}
			break;
		}
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FShadowMap2DStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%s,%i,%i,%s,%s%s"),
		*ResourceType,
		*ResourceName,						
		*ShadowMapTexture2D,		
		ShadowMapTexture2DSizeX,
		ShadowMapTexture2DSizeY,
		*ShadowMapTexture2DFormat,
		*UsedByLight,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FShadowMap2DStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ShadowMapTexture2D,SizeX,SizeY,Format,UsedByLight") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
FAnalyzeReferencedContentStat::FTextureStats::FTextureStats( UTexture* Texture )
:	ResourceType(Texture->GetClass()->GetName())
,	ResourceName(Texture->GetPathName())
,	bIsReferencedByScript(FALSE)
,	ResourceSize(Texture->GetResourceSize())
,	LODBias(Texture->LODBias)
,	LODGroup(Texture->LODGroup)
,	Format(TEXT("UNKOWN"))
{
	// Update format.
	UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
	if( Texture2D )
	{
		Format = GPixelFormats[Texture2D->Format].Name;
	}
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FTextureStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%s%s"),
		*ResourceType,
		*ResourceName,						
		MaterialsUsedBy.Num(),
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		ResourceSize,
		LODBias,
		LODGroup,
		*Format,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FTextureStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumMaterialsUsedBy,ScriptReferenced,NumMapsUsedIn,ResourceSize,LODBias,LODGroup,Format") LINE_TERMINATOR;
}

/**
* Static helper to return instructions used by shader type.
*
* @param	MeshShaderMap	Shader map to use to find shader of passed in type
* @param	ShaderType		Type of shader to query instruction count for
* @return	Instruction count if found, 0 otherwise
*/
static INT GetNumInstructionsForShaderType( const FMeshMaterialShaderMap* MeshShaderMap, FShaderType* ShaderType )
{
	INT NumInstructions = 0;
	const FShader* Shader = MeshShaderMap->GetShader(ShaderType);
	if( Shader )
	{
		NumInstructions = Shader->GetNumInstructions();
	}
	return NumInstructions;
}

/** Shader type for base pass pixel shader (no lightmap).  */
FShaderType*	FAnalyzeReferencedContentStat::FMaterialStats::ShaderTypeBasePassNoLightmap = NULL;
/** Shader type for base pass pixel shader (including lightmap). */
FShaderType*	FAnalyzeReferencedContentStat::FMaterialStats::ShaderTypeBasePassAndLightmap = NULL;
/** Shader type for point light with shadow map pixel shader. */
FShaderType*	FAnalyzeReferencedContentStat::FMaterialStats::ShaderTypePointLightWithShadowMap = NULL;

/** Constructor, initializing all members */
FAnalyzeReferencedContentStat::FMaterialStats::FMaterialStats( UMaterial* Material )
:	ResourceType(Material->GetClass()->GetName())
,	ResourceName(Material->GetPathName())
,	NumBrushesAppliedTo(0)
,	NumStaticMeshInstancesAppliedTo(0)
,	NumSkeletalMeshInstancesAppliedTo(0)
,	bIsReferencedByScript(FALSE)
,	ResourceSizeOfReferencedTextures(0)
{
	// Keep track of unique textures and texture sample count.
	TArray<UTexture*> UniqueTextures;
	TArray<UTexture*> SampledTextures;
	Material->GetTextures( UniqueTextures );
	Material->GetTextures( SampledTextures, FALSE );

	// Update texture samplers count.
	NumTextureSamples = SampledTextures.Num();

	// Update dependency chain stats.
	check( Material->MaterialResources[MSP_SM3]);
	MaxTextureDependencyLength = Material->MaterialResources[MSP_SM3]->GetMaxTextureDependencyLength();

	// Update instruction counts.
	const FMaterialShaderMap* MaterialShaderMap = Material->MaterialResources[MSP_SM3]->GetShaderMap();
	if(MaterialShaderMap)
	{
		// Use the local vertex factory shaders.
		const FMeshMaterialShaderMap* MeshShaderMap = MaterialShaderMap->GetMeshShaderMap(&FLocalVertexFactory::StaticType);
		check(MeshShaderMap);

		// Get intruction counts.
		NumInstructionsBasePassNoLightmap		= GetNumInstructionsForShaderType( MeshShaderMap, ShaderTypeBasePassNoLightmap	);
		NumInstructionsBasePassAndLightmap		= GetNumInstructionsForShaderType( MeshShaderMap, ShaderTypeBasePassAndLightmap );
		NumInstructionsPointLightWithShadowMap	= GetNumInstructionsForShaderType( MeshShaderMap, ShaderTypePointLightWithShadowMap );
	}

	// Iterate over unique texture refs and update resource size.
	for( INT TextureIndex=0; TextureIndex<UniqueTextures.Num(); TextureIndex++ )
	{
		UTexture* Texture = UniqueTextures(TextureIndex);
		if (Texture != NULL)
		{
			ResourceSizeOfReferencedTextures += Texture->GetResourceSize();
			TexturesUsed.AddItem( Texture->GetFullName() );
		}
		else
		{
			warnf(TEXT("Material %s has a NULL texture reference..."), *(Material->GetFullName()));
		}
	}
}

void FAnalyzeReferencedContentStat::FMaterialStats::SetupShaders()
{
	ShaderTypeBasePassNoLightmap		= FindShaderTypeByName(TEXT("TBasePassPixelShaderFNoLightMapPolicyNoSkyLight"));
	ShaderTypeBasePassAndLightmap		= FindShaderTypeByName(TEXT("TBasePassPixelShaderFDirectionalVertexLightMapPolicyNoSkyLight"));
	ShaderTypePointLightWithShadowMap	= FindShaderTypeByName(TEXT("TLightPixelShaderFPointLightPolicyFShadowTexturePolicy"));

	check( ShaderTypeBasePassNoLightmap	);
	check( ShaderTypeBasePassAndLightmap );
	check( ShaderTypePointLightWithShadowMap );
}
/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FMaterialStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,
		NumBrushesAppliedTo,
		NumStaticMeshInstancesAppliedTo,
		StaticMeshesAppliedTo.Num(),
		NumSkeletalMeshInstancesAppliedTo,
		SkeletalMeshesAppliedTo.Num(),
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		TexturesUsed.Num(),
		NumTextureSamples,
		MaxTextureDependencyLength,
		NumInstructionsBasePassNoLightmap,
		NumInstructionsBasePassAndLightmap,
		NumInstructionsPointLightWithShadowMap,
		ResourceSizeOfReferencedTextures,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FMaterialStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumBrushesAppliedTo,NumStaticMeshInstancesAppliedTo,NumStaticMeshesAppliedTo,NumSkeletalMeshInstancesAppliedTo,NumSkeletalMeshesAppliedTo,ScriptReferenced,NumMapsUsedIn,NumTextures,NumTextureSamples,MaxTextureDependencyLength,BasePass,BasePassLightmap,PointLightShadowMap,ResourceSizeOfReferencedTextures") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
FAnalyzeReferencedContentStat::FParticleStats::FParticleStats( UParticleSystem* ParticleSystem )
:	ResourceType(ParticleSystem->GetClass()->GetName())
,	ResourceName(ParticleSystem->GetPathName())
,	bIsReferencedByScript(FALSE)
,	NumEmitters(0)
,	NumModules(0)
,	NumPeakActiveParticles(0)
,   NumEmittersUsingCollision(0)
,   NumEmittersUsingPhysics(0)
,   MaxNumDrawnPerFrame(0)
,   PeakActiveToMaxDrawnRatio(0.0f)
,   NumBytesUsed(0)
,	bUsesDistortionMaterial(FALSE)
,	bUsesSceneTextureMaterial(FALSE)
,   bMeshEmitterHasDoCollisions(FALSE)
,   bMeshEmitterHasCastShadows(FALSE)
,   WarmUpTime( 0.0f )
,	bHasPhysXEmitters(FALSE)
{

	if( ParticleSystem->WarmupTime > 0.0f )
	{
		WarmUpTime = ParticleSystem->WarmupTime;
	}

	// Iterate over all sub- emitters and update stats.
	for( INT EmitterIndex=0; EmitterIndex<ParticleSystem->Emitters.Num(); EmitterIndex++ )
	{
		UParticleEmitter* ParticleEmitter = ParticleSystem->Emitters(EmitterIndex);
		if( ParticleEmitter )
		{
			if (ParticleEmitter->LODLevels.Num() > 0)
			{
				NumEmitters++;
				UParticleLODLevel* HighLODLevel = ParticleEmitter->LODLevels(0);
				check(HighLODLevel);
				NumModules += HighLODLevel->Modules.Num();

				for (INT LODIndex = 0; LODIndex < ParticleEmitter->LODLevels.Num(); LODIndex++)
				{
					UParticleLODLevel* LODLevel = ParticleEmitter->LODLevels(LODIndex);
					if (LODLevel)
					{
						if (LODLevel->bEnabled)
						{
							// Get peak active particles from LOD 0.
							if (LODIndex == 0)
							{
								INT PeakParticles = 0;
								if (ParticleEmitter->InitialAllocationCount > 0)
								{
									//If this value is non-zero it was overridden by user in the editor
									PeakParticles = ParticleEmitter->InitialAllocationCount;
								}
								else
								{
									//Peak number of particles simulated
									PeakParticles = LODLevel->PeakActiveParticles;
								}

								NumPeakActiveParticles += PeakParticles;                     

								if (LODLevel->RequiredModule && LODLevel->RequiredModule->bUseMaxDrawCount)
								{
									//Maximum number of particles allowed to draw per frame by this emitter
									MaxNumDrawnPerFrame += LODLevel->RequiredModule->MaxDrawCount;
								}
								else
								{
									//Make the "max drawn" equal to the number of particles simulated
									MaxNumDrawnPerFrame += PeakParticles;
								}
							}

							// flag distortion and scene color usage of materials
							if( LODLevel->RequiredModule && 
								LODLevel->RequiredModule->Material &&
								LODLevel->RequiredModule->Material->GetMaterial() )
							{
								if( LODLevel->RequiredModule->Material->GetMaterial()->HasDistortion() )
								{
									bUsesDistortionMaterial = TRUE;
									DistortMaterialNames.AddUniqueItem(LODLevel->RequiredModule->Material->GetPathName());
								}
								if( LODLevel->RequiredModule->Material->GetMaterial()->UsesSceneColor() )
								{
									bUsesSceneTextureMaterial = TRUE;
									SceneColorMaterialNames.AddUniqueItem(LODLevel->RequiredModule->Material->GetPathName());
								}
							}

							if( LODLevel->TypeDataModule)
							{
								UParticleModuleTypeDataMesh* MeshType = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
								UParticleModuleTypeDataPhysX* PhysXType = Cast<UParticleModuleTypeDataPhysX>(LODLevel->TypeDataModule);
								UParticleModuleTypeDataMeshPhysX* PhysXMeshType = Cast<UParticleModuleTypeDataMeshPhysX>(LODLevel->TypeDataModule);
								if (PhysXType || PhysXMeshType)
								{
									bHasPhysXEmitters = TRUE;
								}

								if (MeshType)
								{
									if( MeshType->DoCollisions == TRUE )
									{
										bMeshEmitterHasDoCollisions = TRUE;
									}

									if( MeshType->CastShadows == TRUE )
									{
										bMeshEmitterHasCastShadows = TRUE;
									}

									if( MeshType->Mesh )
									{
										for( INT MeshLODIdx=0; MeshLODIdx < MeshType->Mesh->LODInfo.Num(); MeshLODIdx++ )
										{
											const FStaticMeshLODInfo& MeshLOD = MeshType->Mesh->LODInfo(MeshLODIdx);
											for( INT ElementIdx=0; ElementIdx < MeshLOD.Elements.Num(); ElementIdx++ )
											{
												// flag distortion and scene color usage of materials
												const FStaticMeshLODElement& MeshElement = MeshLOD.Elements(ElementIdx);
												if( MeshElement.Material &&
													MeshElement.Material->GetMaterial() )
												{
													if( MeshElement.Material->GetMaterial()->HasDistortion() )
													{
														bUsesDistortionMaterial = TRUE;
														DistortMaterialNames.AddUniqueItem(MeshElement.Material->GetPathName());
													}
													if( MeshElement.Material->GetMaterial()->UsesSceneColor() )
													{
														bUsesSceneTextureMaterial = TRUE;
														SceneColorMaterialNames.AddUniqueItem(MeshElement.Material->GetPathName());
													}
												}
											}
										}
									}
								}
							}

							for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
							{
								UParticleModuleCollision* CollisionModule = Cast<UParticleModuleCollision>(LODLevel->Modules(ModuleIndex));                                      
								if (CollisionModule && CollisionModule->bEnabled)
								{
									NumEmittersUsingCollision++;
									if (CollisionModule->bApplyPhysics == TRUE)
									{
										NumEmittersUsingPhysics++;
									}
								}

								UParticleModuleMeshMaterial* MeshModule = Cast<UParticleModuleMeshMaterial>(LODLevel->Modules(ModuleIndex));
								if( MeshModule )
								{
									for( INT MatIdx=0; MatIdx < MeshModule->MeshMaterials.Num(); MatIdx++ )
									{
										// flag distortion and scene color usage of materials
										UMaterialInterface* Mat = MeshModule->MeshMaterials(MatIdx);									
										if( Mat && Mat->GetMaterial() )
										{
											if( Mat->GetMaterial()->HasDistortion() )
											{
												bUsesDistortionMaterial = TRUE;
												DistortMaterialNames.AddUniqueItem(Mat->GetPathName());
											}
											if( Mat->GetMaterial()->UsesSceneColor() )
											{
												bUsesSceneTextureMaterial = TRUE;
												SceneColorMaterialNames.AddUniqueItem(Mat->GetPathName());
											}
										}
									}
								}
								UParticleModuleMaterialByParameter* MaterialByModule = Cast<UParticleModuleMaterialByParameter>(LODLevel->Modules(ModuleIndex));
								if( MaterialByModule )
								{
									for( INT MatIdx=0; MatIdx < MaterialByModule->DefaultMaterials.Num(); MatIdx++ )
									{
										// flag distortion and scene color usage of materials
										UMaterialInterface* Mat = MaterialByModule->DefaultMaterials(MatIdx);
										if( Mat && Mat->GetMaterial() )
										{
											if( Mat->GetMaterial()->HasDistortion() )
											{
												bUsesDistortionMaterial = TRUE;
												DistortMaterialNames.AddUniqueItem(Mat->GetPathName());
											}
											if( Mat->GetMaterial()->UsesSceneColor() )
											{
												bUsesSceneTextureMaterial = TRUE; 
												SceneColorMaterialNames.AddUniqueItem(Mat->GetPathName());
											}
										}
									}
								}
							}
						}
						else
						{
							if (LODLevel->TypeDataModule)
							{
								UParticleModuleTypeDataPhysX* PhysXType = Cast<UParticleModuleTypeDataPhysX>(LODLevel->TypeDataModule);
								UParticleModuleTypeDataMeshPhysX* PhysXMeshType = Cast<UParticleModuleTypeDataMeshPhysX>(LODLevel->TypeDataModule);
								if (PhysXType || PhysXMeshType)
								{
									bHasPhysXEmitters = TRUE;
								}
							}
						}
					}
				} //for each lod
			}
		}
	} //for each emitter

	// @todo add this into the .xls
	//ParticleSystem->CalculateMaxActiveParticleCounts();

	//A number greater than 1 here indicates more particles simulated than drawn each frame
	if (MaxNumDrawnPerFrame > 0)
	{
		PeakActiveToMaxDrawnRatio = (FLOAT)NumPeakActiveParticles / (FLOAT)MaxNumDrawnPerFrame;
	}


	// determine the number of bytes this ParticleSystem uses
	FArchiveCountMem Count( ParticleSystem );
	NumBytesUsed = Count.GetNum();

	// Determine the number of bytes a PSysComp would use w/ this PSys as the template...
	if (ParticleSystem->IsTemplate() == FALSE)
	{
		UParticleSystemComponent* PSysComp = ConstructObject<UParticleSystemComponent>(UParticleSystemComponent::StaticClass());
		if (PSysComp)
		{
			PSysComp->SetTemplate(ParticleSystem);
			PSysComp->ActivateSystem(TRUE);

			FArchiveCountMem CountPSysComp(PSysComp);
			NumBytesUsed += CountPSysComp.GetNum();

			PSysComp->DeactivateSystem();
		}
	}
}

/** @return TRUE if this asset type should be logged */
UBOOL FAnalyzeReferencedContentStat::FParticleStats::ShouldLogStat() const
{
	return TRUE;
	//return bUsesDistortionMaterial || bUsesSceneTextureMaterial;
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FParticleStats::ToCSV() const
{
	FString MatNames;
	for( INT MatIdx=0; MatIdx < SceneColorMaterialNames.Num(); MatIdx++ )
	{
		MatNames += FString(TEXT("(scenecolor)")) + SceneColorMaterialNames(MatIdx); 
		MatNames += FString(TEXT(","));
	}
	for( INT MatIdx=0; MatIdx < DistortMaterialNames.Num(); MatIdx++ )
	{
		MatNames += FString(TEXT("(distort)")) + DistortMaterialNames(MatIdx);
		MatNames += FString(TEXT(","));
	}

	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%1.2f,%i,%i,%i,%s,%s,%s,%s,%1.2f,%s,%s,%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		NumEmitters,
		NumModules,
		NumPeakActiveParticles,
		MaxNumDrawnPerFrame,
		PeakActiveToMaxDrawnRatio,
		NumEmittersUsingCollision,
		NumEmittersUsingPhysics,
		NumBytesUsed,
		bUsesDistortionMaterial ? TEXT("TRUE") : TEXT("FALSE"),
		bUsesSceneTextureMaterial ? TEXT("TRUE") : TEXT("FALSE"),
		bMeshEmitterHasDoCollisions ? TEXT("TRUE") : TEXT("FALSE"),
		bMeshEmitterHasCastShadows ? TEXT("TRUE") : TEXT("FALSE"),
		WarmUpTime,
		bHasPhysXEmitters ? TEXT("TRUE") : TEXT("FALSE"),
		*MatNames,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FParticleStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ScriptReferenced,NumMapsUsedIn,NumEmitters,NumModules,NumPeakActiveParticles,MaxParticlesDrawnPerFrame,PeakToMaxDrawnRatio,NumEmittersUsingCollision,NumEmittersUsingPhysics,NumBytesUsed,bUsesDistortion,bUsesSceneColor,bMeshEmitterHasDoCollisions,bMeshEmitterHasCastShadows,WarmUpTime,bHasPhysXEmitters") LINE_TERMINATOR;
}

//
//	FTextureToParticleSystemStats
//
FAnalyzeReferencedContentStat::FTextureToParticleSystemStats::FTextureToParticleSystemStats(UTexture* InTexture) :
TextureName(InTexture->GetPathName())
{
	UTexture2D* Texture2D = Cast<UTexture2D>(InTexture);
	if (Texture2D)
	{
		TextureSize = FString::Printf(TEXT("%d x %d"), (INT)(Texture2D->GetSurfaceWidth()), (INT)(Texture2D->GetSurfaceHeight()));
		Format = GPixelFormats[Texture2D->Format].Name;
	}
	else
	{
		TextureSize = FString::Printf(TEXT("???"));
		Format = TextureSize;
	}
	appMemzero(&ParticleSystemsContainedIn, sizeof(TArray<UParticleSystem*>));
}

void FAnalyzeReferencedContentStat::FTextureToParticleSystemStats::AddParticleSystem(UParticleSystem* InParticleSystem)
{
	ParticleSystemsContainedIn.AddUniqueItem(InParticleSystem->GetPathName());
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FTextureToParticleSystemStats::ToCSV() const
{
	FString Row = FString::Printf(TEXT("%s,%s,%s,%i,%s"),
		*TextureName,
		*TextureSize, 
		*Format,
		ParticleSystemsContainedIn.Num(),
		LINE_TERMINATOR);

	// this will print out the specific particles systems that use this texture
	for (INT PSysIndex = 0; PSysIndex < GetParticleSystemsContainedInCount(); PSysIndex++)
	{
		const FString OutputText = FString::Printf(TEXT(",,,,%s,%s"), *(GetParticleSystemContainedIn(PSysIndex)), LINE_TERMINATOR);
		Row += OutputText;
	}

	return Row;
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FTextureToParticleSystemStats::GetCSVHeaderRow()
{
	return TEXT("ResourceName,Size,Format,NumParticleSystemsUsedIn") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
FAnalyzeReferencedContentStat::FAnimSequenceStats::FAnimSequenceStats( UAnimSequence* Sequence )
:	ResourceType(Sequence->GetClass()->GetName())
,	ResourceName(Sequence->GetPathName())
,	bIsReferencedByScript(FALSE)
,	CompressionScheme(FString(TEXT("")))
,	TranslationFormat(ACF_None)
,	RotationFormat(ACF_None)
,	AnimationSize(0)
,	TotalTracks(0)
,	NumTransTracksWithOneKey(0)
,	NumRotTracksWithOneKey(0)
,	TrackTableSize(0)
,	TotalNumTransKeys(0)
,	TotalNumRotKeys(0)
,	TranslationKeySize(0)
,	RotationKeySize(0)
,	TotalFrames(0)
{
	// The sequence object name is not very useful - strip and add the friendly name.
	FString Left, Right;
	ResourceName.Split(TEXT("."), &Left, &Right, TRUE);
	ResourceName = Left + TEXT(".") + Sequence->SequenceName.ToString();

	UAnimSet * AnimSet = Sequence->GetAnimSet();
	if ( AnimSet )
	{
		AnimSetName = AnimSet->GetName();
	}
	else
	{
		AnimSetName = TEXT("NONE");
	}

	if(Sequence->CompressionScheme)
	{
		CompressionScheme = Sequence->CompressionScheme->GetClass()->GetName();
	}

	TranslationFormat = static_cast<AnimationCompressionFormat>(Sequence->TranslationCompressionFormat);
	RotationFormat = static_cast<AnimationCompressionFormat>(Sequence->RotationCompressionFormat);
	AnimationSize = Sequence->GetResourceSize();

	INT NumRotTracks = 0;

	AnimationFormat_GetStats(	
		Sequence, 
		TotalTracks,
		NumRotTracks,
		TotalNumTransKeys,
		TotalNumRotKeys,
		TranslationKeySize,
		RotationKeySize,
		NumTransTracksWithOneKey,
		NumRotTracksWithOneKey);

	TrackTableSize = sizeof(INT)*Sequence->CompressedTrackOffsets.Num();
	TotalFrames = Sequence->NumFrames;
}


static FString GetCompressionFormatString(AnimationCompressionFormat InFormat)
{
	switch(InFormat)
	{
	case ACF_None:
		return FString(TEXT("ACF_None"));
	case ACF_Float96NoW:
		return FString(TEXT("ACF_Float96NoW"));
	case ACF_Fixed48NoW:
		return FString(TEXT("ACF_Fixed48NoW"));
	case ACF_IntervalFixed32NoW:
		return FString(TEXT("ACF_IntervalFixed32NoW"));
	case ACF_Fixed32NoW:
		return FString(TEXT("ACF_Fixed32NoW"));
	case ACF_Float32NoW:
		return FString(TEXT("ACF_Float32NoW"));
	default:
		warnf( TEXT("AnimationCompressionFormat was not found:  %i"), static_cast<INT>(InFormat) );
	}

	return FString(TEXT("Unknown"));
}

static FString GetAnimationKeyFormatString(AnimationKeyFormat InFormat)
{
	switch(InFormat)
	{
	case AKF_ConstantKeyLerp:
		return FString(TEXT("AKF_ConstantKeyLerp"));
	case AKF_VariableKeyLerp:
		return FString(TEXT("AKF_VariableKeyLerp"));
	default:
		warnf( TEXT("AnimationKeyFormat was not found:  %i"), static_cast<INT>(InFormat) );
	}

	return FString(TEXT("Unknown"));
}

static FString GetReferenceTypeString(FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType ReferenceType)
{
	switch(ReferenceType)
	{
	case FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType::ART_SkeletalMeshComponent:
		return FString(TEXT("SkeletalMeshComponent"));
	case FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType::ART_Matinee:
		return FString(TEXT("Matinee"));
	case FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType::ART_Crowd:
		return FString(TEXT("Crowd"));
	}

	return FString(TEXT("Unknown"));
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FAnimSequenceStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%s,%i,%i,%s,%s,%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,	
		*AnimSetName, 
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		*GetReferenceTypeString(ReferenceType),
		*GetCompressionFormatString(TranslationFormat),
		*GetCompressionFormatString(RotationFormat),
		*CompressionScheme,
		AnimationSize,
		TrackTableSize,
		TotalTracks,
		NumTransTracksWithOneKey,
		NumRotTracksWithOneKey,
		TotalNumTransKeys,
		TotalNumRotKeys,
		TranslationKeySize,
		RotationKeySize,
		TotalFrames,
		LINE_TERMINATOR);
}
/** This takes a LevelName and then looks for the number of Instances of this AnimStat used within that level **/
FString FAnalyzeReferencedContentStat::FAnimSequenceStats::ToCSV( const FString& LevelName ) const
{
	UINT* NumInst = const_cast<UINT*>(LevelNameToInstanceCount.Find( LevelName ));
	if( NumInst == NULL )
	{
		*NumInst = 0;
	}

	return FString::Printf(TEXT("%s,%s,%s,%i,%i,%s,%s,%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i%s"),
		*ResourceType,
		*ResourceName,	
		*AnimSetName, 
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		*GetReferenceTypeString(ReferenceType),
		*GetCompressionFormatString(TranslationFormat),
		*GetCompressionFormatString(RotationFormat),
		*CompressionScheme,
		AnimationSize,
		TrackTableSize,
		TotalTracks,
		NumTransTracksWithOneKey,
		NumRotTracksWithOneKey,
		TotalNumTransKeys,
		TotalNumRotKeys,
		TranslationKeySize,
		RotationKeySize,
		TotalFrames,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FAnimSequenceStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,AnimSetName,ScriptReferenced,NumMapsUsedIn,ReferenceType,TransFormat,RotFormat,CompressionScheme,AnimSize,TrackTableSize,Tracks,TracksWithoutTrans,TracksWithoutRot,TransKeys,RotKeys,TransKeySize,RotKeySize,TotalFrames") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
FAnalyzeReferencedContentStat::FFaceFXAnimSetStats::FFaceFXAnimSetStats( UFaceFXAnimSet* FaceFXAnimSet)
:	ResourceType(FaceFXAnimSet->GetClass()->GetName())
,	ResourceName(FaceFXAnimSet->GetPathName())
,	bIsReferencedByScript(FALSE)
{
#if WITH_FACEFX
	ResourceSize = FaceFXAnimSet->GetResourceSize();
	OC3Ent::Face::FxAnimSet* AnimSet = FaceFXAnimSet->GetFxAnimSet();
	if ( AnimSet )
	{
		const OC3Ent::Face::FxAnimGroup & Group = AnimSet->GetAnimGroup();
		GroupName = ANSI_TO_TCHAR(Group.GetName().GetAsCstr());
		NumberOfAnimations = Group.GetNumAnims();
	}
#endif //#if WITH_FACEFX
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FFaceFXAnimSetStats::ToCSV() const
{
#if WITH_FACEFX
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%s,%i%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		ResourceSize,
		*GroupName, 
		NumberOfAnimations, 	
		LINE_TERMINATOR);
#endif //#if WITH_FACEFX
}
/** This takes a LevelName and then looks for the number of Instances of this AnimStat used within that level **/
FString FAnalyzeReferencedContentStat::FFaceFXAnimSetStats::ToCSV( const FString& LevelName ) const
{
#if WITH_FACEFX
	UINT* NumInst = const_cast<UINT*>(LevelNameToInstanceCount.Find( LevelName ));
	if( NumInst == NULL )
	{
		*NumInst = 0;
	}

	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%s,%i%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		ResourceSize,
		*GroupName, 
		NumberOfAnimations, 	
		LINE_TERMINATOR);
#endif // #if WITH_FACEFX
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FFaceFXAnimSetStats::GetCSVHeaderRow()
{
#if WITH_FACEFX
	return TEXT("ResourceType,ResourceName,ScriptReferenced,NumMapsUsedIn,ResourceSize,GroupName,NumberOfAnimations") LINE_TERMINATOR;
#endif // #if WITH_FACEFX
}

/** Constructor, initializing all members. */
FAnalyzeReferencedContentStat::FSoundCueStats::FSoundCueStats( USoundCue* SoundCue )
:	ResourceType(SoundCue->GetClass()->GetName())
,	ResourceName(SoundCue->GetPathName())
,	bIsReferencedByScript(FALSE)
,	FaceFXAnimName(SoundCue->FaceFXAnimName)
,	FaceFXGroupName(SoundCue->FaceFXGroupName)
,	ResourceSize(SoundCue->GetResourceSize())
{
}

/**
* Stringifies gathered stats in CSV format.
*
* @return comma separated list of stats
*/
FString FAnalyzeReferencedContentStat::FSoundCueStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%s,%s%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		ResourceSize,
		*FaceFXAnimName,
		*FaceFXGroupName,
		LINE_TERMINATOR);
}

/** This takes a LevelName and then looks for the number of Instances of this StatMesh used within that level **/
FString FAnalyzeReferencedContentStat::FSoundCueStats::ToCSV( const FString& LevelName ) const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%s,%s%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		MapsUsedIn.Num(),
		ResourceSize,
		*FaceFXAnimName,
		*FaceFXGroupName,
		LINE_TERMINATOR);
}

/**
* Returns a header row for CSV
*
* @return comma separated header row
*/
FString FAnalyzeReferencedContentStat::FSoundCueStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ScriptReferenced,NumMapsUsedIn,NumBytesUsed,FaceFXAnimName,FaceFXGroupName") LINE_TERMINATOR;
}

/**
* Retrieves/ creates material stats associated with passed in material.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	Material	Material to retrieve/ create material stats for
* @return	pointer to material stats associated with material
*/
FAnalyzeReferencedContentStat::FMaterialStats* FAnalyzeReferencedContentStat::GetMaterialStats( UMaterial* Material )
{
	FAnalyzeReferencedContentStat::FMaterialStats* MaterialStats = ResourceNameToMaterialStats.Find( Material->GetFullName() );
	if( MaterialStats == NULL )
	{
		MaterialStats =	&ResourceNameToMaterialStats.Set( *Material->GetFullName(), FAnalyzeReferencedContentStat::FMaterialStats( Material ) );
	}
	return MaterialStats;
}

/**
* Retrieves/ creates texture stats associated with passed in texture.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	Texture		Texture to retrieve/ create texture stats for
* @return	pointer to texture stats associated with texture
*/
FAnalyzeReferencedContentStat::FTextureStats* FAnalyzeReferencedContentStat::GetTextureStats( UTexture* Texture )
{
	FAnalyzeReferencedContentStat::FTextureStats* TextureStats = ResourceNameToTextureStats.Find( Texture->GetFullName() );
	if( TextureStats == NULL )
	{
		TextureStats = &ResourceNameToTextureStats.Set( *Texture->GetFullName(), FAnalyzeReferencedContentStat::FTextureStats( Texture ) );
	}
	return TextureStats;
}

/**
* Retrieves/ creates static mesh stats associated with passed in static mesh.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	StaticMesh	Static mesh to retrieve/ create static mesh stats for
* @return	pointer to static mesh stats associated with static mesh
*/
FAnalyzeReferencedContentStat::FStaticMeshStats* FAnalyzeReferencedContentStat::GetStaticMeshStats( UStaticMesh* StaticMesh, UPackage* LevelPackage )
{
	FAnalyzeReferencedContentStat::FStaticMeshStats* StaticMeshStats = ResourceNameToStaticMeshStats.Find( StaticMesh->GetFullName() );

	if( StaticMeshStats == NULL )
	{
		StaticMeshStats = &ResourceNameToStaticMeshStats.Set( *StaticMesh->GetFullName(), FAnalyzeReferencedContentStat::FStaticMeshStats( StaticMesh ) );
	}

	return StaticMeshStats;
}

/**
* Retrieves/ creates skeletal mesh stats associated with passed in skeletal mesh.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	SkeletalMesh	Skeletal mesh to retrieve/ create skeletal mesh stats for
* @return	pointer to skeletal mesh stats associated with skeletal mesh
*/
FAnalyzeReferencedContentStat::FSkeletalMeshStats* FAnalyzeReferencedContentStat::GetSkeletalMeshStats( USkeletalMesh* SkeletalMesh, UPackage* LevelPackage )
{
	FAnalyzeReferencedContentStat::FSkeletalMeshStats* SkeletalMeshStats = ResourceNameToSkeletalMeshStats.Find( SkeletalMesh->GetFullName() );

	if( SkeletalMeshStats == NULL )
	{
		SkeletalMeshStats = &ResourceNameToSkeletalMeshStats.Set( *SkeletalMesh->GetFullName(), FAnalyzeReferencedContentStat::FSkeletalMeshStats( SkeletalMesh ) );
	}

	return SkeletalMeshStats;
}

/**
* Retrieves/ creates particle stats associated with passed in particle system.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	ParticleSystem	Particle system to retrieve/ create static mesh stats for
* @return	pointer to particle system stats associated with static mesh
*/
FAnalyzeReferencedContentStat::FParticleStats* FAnalyzeReferencedContentStat::GetParticleStats( UParticleSystem* ParticleSystem )
{
	FAnalyzeReferencedContentStat::FParticleStats* ParticleStats = ResourceNameToParticleStats.Find( ParticleSystem->GetFullName() );
	if( ParticleStats == NULL )
	{
		ParticleStats = &ResourceNameToParticleStats.Set( *ParticleSystem->GetFullName(), FAnalyzeReferencedContentStat::FParticleStats( ParticleSystem ) );
	}
	return ParticleStats;
}

/**
* Retrieves/creates texture in particle system stats associated with the passed in texture.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	InTexture	The texture to retrieve/create stats for
* @return	pointer to textureinparticlesystem stats
*/
FAnalyzeReferencedContentStat::FTextureToParticleSystemStats* FAnalyzeReferencedContentStat::GetTextureToParticleSystemStats(UTexture* InTexture)
{
	FAnalyzeReferencedContentStat::FTextureToParticleSystemStats* TxtrToPSysStats = ResourceNameToTextureToParticleSystemStats.Find(InTexture->GetPathName());
	if (TxtrToPSysStats == NULL)
	{
		TxtrToPSysStats = &ResourceNameToTextureToParticleSystemStats.Set(*InTexture->GetPathName(), 
			FAnalyzeReferencedContentStat::FTextureToParticleSystemStats(InTexture));
	}

	return TxtrToPSysStats;
}


/**
* Retrieves/ creates animation sequence stats associated with passed in animation sequence.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	AnimSequence	Anim sequence to retrieve/ create anim sequence stats for
* @return	pointer to particle system stats associated with anim sequence
*/
FAnalyzeReferencedContentStat::FAnimSequenceStats* FAnalyzeReferencedContentStat::GetAnimSequenceStats( UAnimSequence* AnimSequence )
{
	FAnalyzeReferencedContentStat::FAnimSequenceStats* AnimStats = ResourceNameToAnimStats.Find( AnimSequence->GetFullName() );
	if( AnimStats == NULL )
	{
		AnimStats = &ResourceNameToAnimStats.Set( *AnimSequence->GetFullName(), FAnalyzeReferencedContentStat::FAnimSequenceStats( AnimSequence ) );
	}
	return AnimStats;
}

/**
* Retrieves/ creates animation sequence stats associated with passed in animation sequence.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	FaceFXAnimSet	FaceFXAnimSet to retrieve/ create anim sequence stats for
* @return	pointer to particle system stats associated with anim sequence
*/
FAnalyzeReferencedContentStat::FFaceFXAnimSetStats* FAnalyzeReferencedContentStat::GetFaceFXAnimSetStats( UFaceFXAnimSet* FaceFXAnimSet )
{
	FAnalyzeReferencedContentStat::FFaceFXAnimSetStats* FaceFXAnimSetStats = ResourceNameToFaceFXAnimSetStats.Find( FaceFXAnimSet->GetFullName() );
	if( FaceFXAnimSetStats == NULL )
	{
		FaceFXAnimSetStats = &ResourceNameToFaceFXAnimSetStats.Set( *FaceFXAnimSet->GetFullName(), FAnalyzeReferencedContentStat::FFaceFXAnimSetStats( FaceFXAnimSet ) );
	}
	return FaceFXAnimSetStats;
}

/**
* Retrieves/ creates lighting optimization stats associated with passed in static mesh actor.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	ActorComponent	Actor component to calculate potential light map savings stats for
* @return	pointer to lighting optimization stats associated with this actor component
*/
FAnalyzeReferencedContentStat::FLightingOptimizationStats* FAnalyzeReferencedContentStat::GetLightingOptimizationStats( AStaticMeshActor* StaticMeshActor )
{
	FAnalyzeReferencedContentStat::FLightingOptimizationStats* LightingStats = ResourceNameToLightingStats.Find( StaticMeshActor->GetFullName() );
	if( LightingStats == NULL )
	{
		LightingStats = &ResourceNameToLightingStats.Set( *StaticMeshActor->GetFullName(), FAnalyzeReferencedContentStat::FLightingOptimizationStats( StaticMeshActor ) );
	}
	return LightingStats;
}

/**
* Retrieves/ creates sound cue stats associated with passed in sound cue.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
* @return				pointer to sound cue  stats associated with sound cue  
*/
FAnalyzeReferencedContentStat::FSoundCueStats* FAnalyzeReferencedContentStat::GetSoundCueStats( USoundCue* SoundCue, UPackage* LevelPackage )
{
	FAnalyzeReferencedContentStat::FSoundCueStats* SoundCueStats = ResourceNameToSoundCueStats.Find( SoundCue->GetFullName() );

	if( SoundCueStats == NULL )
	{
		SoundCueStats = &ResourceNameToSoundCueStats.Set( *SoundCue->GetFullName(), FAnalyzeReferencedContentStat::FSoundCueStats( SoundCue ) );
	}

	return SoundCueStats;
}

/**
* Retrieves/ creates shadowmap 1D stats associated with passed in shadowmap 1D object.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
* @return				pointer to sound cue  stats associated with sound cue  
*/
FAnalyzeReferencedContentStat::FShadowMap1DStats* FAnalyzeReferencedContentStat::GetShadowMap1DStats( UShadowMap1D* ShadowMap1D, UPackage* LevelPackage )
{
	FAnalyzeReferencedContentStat::FShadowMap1DStats* ShadowMap1DStats = ResourceNameToShadowMap1DStats.Find( ShadowMap1D->GetFullName() );

	if( ShadowMap1DStats == NULL )
	{
		ShadowMap1DStats = &ResourceNameToShadowMap1DStats.Set( *ShadowMap1D->GetFullName(), FAnalyzeReferencedContentStat::FShadowMap1DStats( ShadowMap1D ) );
	}

	return ShadowMap1DStats;
}

/**
* Retrieves/ creates shadowmap 2D stats associated with passed in shadowmap 2D object.
*
* @warning: returns pointer into TMap, only valid till next time Set is called
*
* @param	SoundCue	Sound cue  to retrieve/ create sound cue  stats for
* @return				pointer to sound cue  stats associated with sound cue  
*/
FAnalyzeReferencedContentStat::FShadowMap2DStats* FAnalyzeReferencedContentStat::GetShadowMap2DStats( UShadowMap2D* ShadowMap2D, UPackage* LevelPackage )
{
	FAnalyzeReferencedContentStat::FShadowMap2DStats* ShadowMap2DStats = ResourceNameToShadowMap2DStats.Find( ShadowMap2D->GetFullName() );

	if( ShadowMap2DStats == NULL )
	{
		ShadowMap2DStats = &ResourceNameToShadowMap2DStats.Set( *ShadowMap2D->GetFullName(), FAnalyzeReferencedContentStat::FShadowMap2DStats( ShadowMap2D ) );
	}

	return ShadowMap2DStats;
}

void UAnalyzeReferencedContentCommandlet::StaticInitialize()
{
	ShowErrorCount = FALSE;
}

/**
* Handles encountered object, routing to various sub handlers.
*
* @param	Object			Object to handle
* @param	LevelPackage	Currently loaded level package, can be NULL if not a level
* @param	bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleObject( UObject* Object, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	// Disregard marked objects as they won't go away with GC.
	if( !Object->HasAnyFlags( RF_Marked ) )
	{
		// Whether the object is the passed in level package if it is != NULL.
		const UBOOL bIsInALevelPackage = LevelPackage && Object->IsIn( LevelPackage );

		if( Object->IsA(UParticleSystemComponent::StaticClass()) && bIsInALevelPackage && ((ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Particle)) == 0))
		{
			HandleStaticMeshOnAParticleSystemComponent( (UParticleSystemComponent*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle static mesh.
		else if( Object->IsA(UStaticMesh::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_StaticMesh) == 0))
		{
			HandleStaticMesh( (UStaticMesh*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles static mesh component if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(UStaticMeshComponent::StaticClass()) && bIsInALevelPackage && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_StaticMeshComponent) == 0))
		{
			HandleStaticMeshComponent( (UStaticMeshComponent*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles static mesh component if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(AStaticMeshActor::StaticClass()) && bIsInALevelPackage && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_StaticMeshActor) == 0))
		{
			HandleStaticMeshActor( (AStaticMeshActor*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle static mesh.
		else if( Object->IsA(USkeletalMesh::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_SkeletalMesh) == 0))
		{
			HandleSkeletalMesh( (USkeletalMesh*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles static mesh component if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(USkeletalMeshComponent::StaticClass()) )
		{
			if ( bIsInALevelPackage && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_SkeletalMeshComponent) == 0) )
			{
				HandleSkeletalMeshComponentForSMC( (USkeletalMeshComponent*) Object, LevelPackage, bIsScriptReferenced );
			}

			if ( ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Anim) == 0 )
			{
				HandleSkeletalMeshComponentForAnim( (USkeletalMeshComponent*) Object, LevelPackage, bIsScriptReferenced );
			}
		}
		// Handle material.
		else if( Object->IsA(UMaterial::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Material) == 0))
		{
			HandleMaterial( (UMaterial*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle texture.
		else if( Object->IsA(UTexture::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Texture) == 0))
		{
			HandleTexture( (UTexture*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles brush actor if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(ABrush::StaticClass()) && bIsInALevelPackage && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Brush) == 0))
		{
			HandleBrush( (ABrush*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle particle system.
		else if( Object->IsA(UParticleSystem::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Particle) == 0))
		{
			HandleParticleSystem( (UParticleSystem*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle anim sequence.
		else if( Object->IsA(UInterpTrackAnimControl::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Anim) == 0))
		{
			HandleInterpTrackAnimControl((UInterpTrackAnimControl*) Object, LevelPackage, bIsScriptReferenced );
		}
		else if( Object->IsA(UInterpGroup::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Anim) == 0))
		{
			HandleInterpGroup((UInterpGroup*) Object, LevelPackage, bIsScriptReferenced );
		}
		else if( Object->IsA(USeqAct_CrowdSpawner::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Anim) == 0))
		{
			HandleSeqAct_CrowdSpawner((USeqAct_CrowdSpawner*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle level
		else if( Object->IsA(ULevel::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_Level) == 0))
		{
			HandleLevel( (ULevel*)Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle sound cue
		else if (Object->IsA(USoundCue::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_SoundCue) == 0))
		{
			HandleSoundCue((USoundCue*)Object, LevelPackage, bIsScriptReferenced);
		}
		// Handle 1D shadow maps
		else if (Object->IsA(UShadowMap1D::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_ShadowMap) == 0))
		{
			HandleShadowMap1D((UShadowMap1D*)Object,LevelPackage,bIsScriptReferenced);			
		}
		// Handle 2D shadow maps
		else if (Object->IsA(UShadowMap2D::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_ShadowMap) == 0))
		{
			HandleShadowMap2D((UShadowMap2D*)Object,LevelPackage,bIsScriptReferenced);
		}
		else if (Object->IsA(UFaceFXAnimSet::StaticClass()) && (ReferencedContentStat.InIgnoreObjectFlag(FAnalyzeReferencedContentStat::IGNORE_FaceFXAnimSet) == 0))
		{
			// if platform is windows or script referenced, handle them
			if ( !UsingPersistentFaceFXAnimSetGenerator() || bIsScriptReferenced )
			{
				HandleFaceFXAnimSet((UFaceFXAnimSet*)Object,LevelPackage,bIsScriptReferenced);
			}
		}
	}
}

/**
* Handles gathering stats for passed in static mesh.
*
* @param StaticMesh	StaticMesh to gather stats for.
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleStaticMesh( UStaticMesh* StaticMesh, UPackage* LevelPackage, UBOOL bIsScriptReferenced  )
{
	FAnalyzeReferencedContentStat::FStaticMeshStats* StaticMeshStats = ReferencedContentStat.GetStaticMeshStats( StaticMesh, LevelPackage );

	if (appStristr(*StaticMesh->GetName(), TEXT("destruct")) != NULL)
	{
		debugf(TEXT("HandleStaticMesh MeshName:%s"), *StaticMesh->GetFullName());
	}

	StaticMeshStats->AddLevelInfo( LevelPackage );

	if( bIsScriptReferenced )
	{
		StaticMeshStats->bIsReferencedByScript = TRUE;
	}

	// Populate materials array, avoiding duplicate entries.
	TArray<UMaterial*> Materials;
	// @todo need to do foreach over all LODModels
	INT MaterialCount = StaticMesh->LODModels(0).Elements.Num();
	for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
	{
		UMaterialInterface* MaterialInterface = StaticMesh->LODModels(0).Elements(MaterialIndex).Material;
		if( MaterialInterface && MaterialInterface->GetMaterial(MSP_SM3) )
		{
			Materials.AddUniqueItem( MaterialInterface->GetMaterial(MSP_SM3) );
		}
	}

	// Iterate over materials and create/ update associated stats.
	for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
	{
		UMaterial* Material	= Materials(MaterialIndex);	
		FAnalyzeReferencedContentStat::FMaterialStats* MaterialStats = ReferencedContentStat.GetMaterialStats( Material );
		MaterialStats->StaticMeshesAppliedTo.Set( *StaticMesh->GetFullName(), TRUE );
	}
}

/**
* Handles gathering stats for passed in static actor component.
*
* @param StaticMeshActor	StaticMeshActor to gather stats for
* @param LevelPackage		Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleStaticMeshActor( AStaticMeshActor* StaticMeshActor, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->StaticMeshComponent;
	if( StaticMeshComponent && StaticMeshComponent->StaticMesh && StaticMeshComponent->StaticMesh->LODModels.Num() && StaticMeshComponent->HasStaticShadowing() )
	{
		// Track lighting optimization values for a given static mesh actor.
		FAnalyzeReferencedContentStat::FLightingOptimizationStats* LightingStats = ReferencedContentStat.GetLightingOptimizationStats( StaticMeshActor );

		FAnalyzeReferencedContentStat::FStaticMeshStats* StaticMeshStats = ReferencedContentStat.GetStaticMeshStats( StaticMeshComponent->StaticMesh, LevelPackage );
		StaticMeshStats->AddLevelInfo( LevelPackage, TRUE );
	}
}

/**
* Handles special case for stats for passed in static mesh component who is part of a ParticleSystemComponent
*
* @param ParticleSystemComponent	ParticleSystemComponent to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleStaticMeshOnAParticleSystemComponent( UParticleSystemComponent* ParticleSystemComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UParticleSystemComponent* PSC = ParticleSystemComponent;
	//warnf( TEXT("%d"), PSC->SMComponents.Num() );

	UStaticMeshComponent* StaticMeshComponent = NULL;
	TArray<UStaticMesh*> ReferencedStaticMeshes;
	for( INT i = 0; i < PSC->SMComponents.Num(); ++i )
	{
		StaticMeshComponent = PSC->SMComponents(i);
		if (StaticMeshComponent && StaticMeshComponent->StaticMesh)
		{
			ReferencedStaticMeshes.AddUniqueItem( StaticMeshComponent->StaticMesh );
		}
	}

	UStaticMesh* StaticMesh = NULL;
	for( INT i = 0; i < ReferencedStaticMeshes.Num(); ++i )
	{
		//warnf( TEXT("%s"), *ReferencedStaticMeshes(i)->GetFullName() );
		StaticMesh = ReferencedStaticMeshes(i);
		HandleStaticMesh( StaticMesh, LevelPackage, bIsScriptReferenced );

		FAnalyzeReferencedContentStat::FStaticMeshStats* StaticMeshStats = ReferencedContentStat.GetStaticMeshStats( StaticMesh, LevelPackage );
		StaticMeshStats->NumInstances++;

		// Mark object as being referenced by particles.
		StaticMeshStats->bIsReferencedByParticles = TRUE;
	}
}


/**
* Handles gathering stats for passed in material.
*
* @param Material	Material to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleMaterial( UMaterial* Material, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	FAnalyzeReferencedContentStat::FMaterialStats* MaterialStats = ReferencedContentStat.GetMaterialStats( Material );	
	MaterialStats->AddLevelInfo( LevelPackage );

	if( bIsScriptReferenced )
	{
		MaterialStats->bIsReferencedByScript = TRUE;
	}

	// Array of textures used by this material. No duplicates.
	TArray<UTexture*> TexturesUsed;
	Material->GetTextures(TexturesUsed);

	// Update textures used by this material.
	for( INT TextureIndex=0; TextureIndex<TexturesUsed.Num(); TextureIndex++ )
	{
		UTexture* Texture = TexturesUsed(TextureIndex);
		if (Texture != NULL)
		{
			FAnalyzeReferencedContentStat::FTextureStats* TextureStats = ReferencedContentStat.GetTextureStats(Texture);
			TextureStats->MaterialsUsedBy.Set( *Material->GetFullName(), TRUE );
		}
	}
}

/**
* Handles gathering stats for passed in texture.
*
* @paramTexture	Texture to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleTexture( UTexture* Texture, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	FAnalyzeReferencedContentStat::FTextureStats* TextureStats = ReferencedContentStat.GetTextureStats( Texture );

	// Only handle further if we have a level package.
	TextureStats->AddLevelInfo( LevelPackage );

	// Mark as being referenced by script.
	if( bIsScriptReferenced )
	{
		TextureStats->bIsReferencedByScript = TRUE;
	}
}

/**
* Handles gathering stats for passed in brush.
*
* @param BrushActor Brush actor to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleBrush( ABrush* BrushActor, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	if( BrushActor->Brush && BrushActor->Brush->Polys )
	{
		UPolys* Polys = BrushActor->Brush->Polys;

		// Populate materials array, avoiding duplicate entries.
		TArray<UMaterial*> Materials;
		for( INT ElementIndex=0; ElementIndex<Polys->Element.Num(); ElementIndex++ )
		{
			const FPoly& Poly = Polys->Element(ElementIndex);
			if( Poly.Material && Poly.Material->GetMaterial(MSP_SM3) )
			{
				Materials.AddUniqueItem( Poly.Material->GetMaterial(MSP_SM3) );
			}
		}

		// Iterate over materials and create/ update associated stats.
		for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
		{
			UMaterial* Material = Materials(MaterialIndex);
			FAnalyzeReferencedContentStat::FMaterialStats* MaterialStats = ReferencedContentStat.GetMaterialStats( Material );
			MaterialStats->NumBrushesAppliedTo++;
		}
	}
}

/**
* Handles gathering stats for passed in particle system.
*
* @param ParticleSystem	Particle system to gather stats for
* @param LevelPackage		Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleParticleSystem( UParticleSystem* ParticleSystem, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	FAnalyzeReferencedContentStat::FParticleStats* ParticleStats = ReferencedContentStat.GetParticleStats( ParticleSystem );

	// Only handle further if we have a level package.
	ParticleStats->AddLevelInfo( LevelPackage );

	// Mark object as being referenced by script.
	if( bIsScriptReferenced )
	{
		ParticleStats->bIsReferencedByScript = TRUE;
	}

	// Loop over the textures used in the particle system...
	for (INT EmitterIndex = 0; EmitterIndex < ParticleSystem->Emitters.Num(); EmitterIndex++)
	{
		UParticleSpriteEmitter* Emitter = Cast<UParticleSpriteEmitter>(ParticleSystem->Emitters(EmitterIndex));
		if (Emitter)
		{
			for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
			{
				UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
				check(LODLevel);
				check(LODLevel->RequiredModule);

				TArray<UTexture*> OutTextures;

				// First, check the sprite material
				UMaterialInterface* MatIntf = LODLevel->RequiredModule->Material;
				if (MatIntf)
				{
					OutTextures.Empty();
					MatIntf->GetTextures(OutTextures);
					for (INT TextureIndex = 0; TextureIndex < OutTextures.Num(); TextureIndex++)
					{
						UTexture* Texture = OutTextures(TextureIndex);
						Texture->ConditionalPostLoad();
						FAnalyzeReferencedContentStat::FTextureToParticleSystemStats* TxtrToPSysStats = ReferencedContentStat.GetTextureToParticleSystemStats(Texture);
						if (TxtrToPSysStats)
						{
							TxtrToPSysStats->AddParticleSystem(ParticleSystem);
						}
					}
				}

				// Check if it is a mesh emitter...
				if (LODIndex == 0)
				{
					if (LODLevel->TypeDataModule)
					{
						UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
						if (MeshTD)
						{
							if (MeshTD->bOverrideMaterial == FALSE)
							{
								// Grab the materials on the mesh...
								if (MeshTD->Mesh)
								{
									for (INT LODInfoIndex = 0; LODInfoIndex < MeshTD->Mesh->LODInfo.Num(); LODInfoIndex++)
									{
										FStaticMeshLODInfo& LODInfo = MeshTD->Mesh->LODInfo(LODInfoIndex);
										for (INT ElementIndex = 0; ElementIndex < LODInfo.Elements.Num(); ElementIndex++)
										{
											FStaticMeshLODElement& Element = LODInfo.Elements(ElementIndex);
											MatIntf = Element.Material;
											if (MatIntf)
											{
												OutTextures.Empty();
												MatIntf->GetTextures(OutTextures);
												for (INT TextureIndex = 0; TextureIndex < OutTextures.Num(); TextureIndex++)
												{
													UTexture* Texture = OutTextures(TextureIndex);

													FAnalyzeReferencedContentStat::FTextureToParticleSystemStats* TxtrToPSysStats = 
														ReferencedContentStat.GetTextureToParticleSystemStats(Texture);
													if (TxtrToPSysStats)
													{
														TxtrToPSysStats->AddParticleSystem(ParticleSystem);
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

				// Check for a MeshMaterial override module...
			}
		}
	}
}


/**
* Handles gathering stats for passed in animation sequence.
*
* @param AnimSequence		AnimSequence to gather stats for
* @param LevelPackage		Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleFaceFXAnimSet( UFaceFXAnimSet* FaceFXAnimSet, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
#if WITH_FACEFX
	FAnalyzeReferencedContentStat::FFaceFXAnimSetStats* FaceFXAnimSetStat= ReferencedContentStat.GetFaceFXAnimSetStats( FaceFXAnimSet );

	// Only handle further if we have a level package.
	FaceFXAnimSetStat->AddLevelInfo( LevelPackage, TRUE );

	// Mark object as being referenced by script.
	if( bIsScriptReferenced )
	{
		FaceFXAnimSetStat->bIsReferencedByScript = TRUE;
	}
#endif
}

/**
* Handles gathering stats for passed in animation sequence.
*
* @param AnimSequence		AnimSequence to gather stats for
* @param LevelPackage		Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleAnimSetInternal( UAnimSet* AnimSet, UPackage* LevelPackage, UBOOL bIsScriptReferenced, FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType ReferenceType )
{
	if (AnimSet)	
	{
		for ( INT I=0; I<AnimSet->Sequences.Num(); ++I )
		{
			HandleAnimSequenceInternal( AnimSet->Sequences(I), LevelPackage, bIsScriptReferenced, ReferenceType );
		}
	}
}

/**
* Handles gathering stats for passed in animation sequence.
*
* @param AnimSequence		AnimSequence to gather stats for
* @param LevelPackage		Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleAnimSequenceInternal( UAnimSequence* AnimSequence, UPackage* LevelPackage, UBOOL bIsScriptReferenced, FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType ReferenceType )
{
	FAnalyzeReferencedContentStat::FAnimSequenceStats* AnimStats = ReferencedContentStat.GetAnimSequenceStats( AnimSequence );

	AnimStats->ReferenceType = ReferenceType;

	// Only handle further if we have a level package.
	AnimStats->AddLevelInfo( LevelPackage, TRUE );

	// Mark object as being referenced by script.
	if( bIsScriptReferenced )
	{
		AnimStats->bIsReferencedByScript = TRUE;
	}
}

/**
* Handles gathering stats for passed in animation sequence.
*
* @param AnimSet	InterpTrackAnimControl to gather stats for
* @param LevelPackage				Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced		Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleInterpGroup( UInterpGroup* InterpGroup, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	for (INT I=0; I<InterpGroup->GroupAnimSets.Num(); ++I)
	{
		HandleAnimSetInternal(InterpGroup->GroupAnimSets(I), LevelPackage, bIsScriptReferenced, FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType::ART_Matinee);
	}
}

/**
* Handles gathering stats for passed in animation sequence.
*
* @param InterpTrackAnimControl		InterpTrackAnimControl to gather stats for
* @param LevelPackage				Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced		Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleInterpTrackAnimControl( UInterpTrackAnimControl* InterpTrackAnimControl, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	for (INT I=0; I<InterpTrackAnimControl->AnimSets.Num(); ++I)
	{
		HandleAnimSetInternal(InterpTrackAnimControl->AnimSets(I), LevelPackage, bIsScriptReferenced, FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType::ART_Matinee);
	}
}

/**
* Handles gathering stats for passed in animation sequence.
*
* @param SkeletalMeshComponent		SkeletalMeshComponent to gather stats for
* @param LevelPackage				Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced		Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleSkeletalMeshComponentForAnim( USkeletalMeshComponent* SkeletalMeshComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	for (INT I=0; I<SkeletalMeshComponent->AnimSets.Num(); ++I)
	{
		HandleAnimSetInternal(SkeletalMeshComponent->AnimSets(I), LevelPackage, bIsScriptReferenced, FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType::ART_SkeletalMeshComponent);
	}
}

/**
* Handles gathering stats for passed in animation sequence.
*
* @param AnimSet	InterpTrackAnimControl to gather stats for
* @param LevelPackage				Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced		Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleSeqAct_CrowdSpawner( USeqAct_CrowdSpawner* SeqAct_CrowdSpawner, UPackage * LevelPackage, UBOOL bIsScriptReferenced )
{
	for (INT I=0; I<SeqAct_CrowdSpawner->FlockAnimSets.Num(); ++I)
	{
		HandleAnimSetInternal(SeqAct_CrowdSpawner->FlockAnimSets(I), LevelPackage, bIsScriptReferenced, FAnalyzeReferencedContentStat::FAnimSequenceStats::EAnimReferenceType::ART_Crowd);
	}
}

void UAnalyzeReferencedContentCommandlet::HandleLevel( ULevel* Level, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	for ( TMultiMap<UStaticMesh*, FCachedPhysSMData>::TIterator MeshIt(Level->CachedPhysSMDataMap); MeshIt; ++MeshIt )
	{
		UStaticMesh* Mesh = MeshIt.Key();
		FVector Scale3D = MeshIt.Value().Scale3D;

		if(Mesh)
		{
			FAnalyzeReferencedContentStat::FStaticMeshStats* StaticMeshStats = ReferencedContentStat.GetStaticMeshStats( Mesh, LevelPackage );

			if (appStristr(*Mesh->GetName(), TEXT("destruct")) != NULL)
			{
				debugf(TEXT("HandleLevel MeshName:%s Scale: [%f %f %f]"), *Mesh->GetFullName(), Scale3D.X, Scale3D.Y, Scale3D.Z);
			}
			UBOOL bHaveScale = FALSE;
			for (INT i=0; i < StaticMeshStats->UsedAtScales.Num(); i++)
			{
				// Found a shape with the right scale
				if ((StaticMeshStats->UsedAtScales(i) - Scale3D).IsNearlyZero())
				{
					bHaveScale = TRUE;
					break;
				}
			}

			if(!bHaveScale)
			{
				if (!Scale3D.IsUniform())
				{
					StaticMeshStats->bIsMeshNonUniformlyScaled = TRUE;
					//Any non uniform scaling of this mesh with box collision will result in no collision
					if (Mesh->BodySetup	&& Mesh->BodySetup->AggGeom.BoxElems.Num() > 0)
					{
						StaticMeshStats->bShouldConvertBoxColl = TRUE;
					}
				}

				StaticMeshStats->UsedAtScales.AddItem(Scale3D);
			}
		}
	}
}

/**
* Handles gathering stats for passed in static mesh component.
*
* @param StaticMeshComponent	StaticMeshComponent to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleStaticMeshComponent( UStaticMeshComponent* StaticMeshComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	if( StaticMeshComponent->StaticMesh && StaticMeshComponent->StaticMesh->LODModels.Num() )
	{
		// Populate materials array, avoiding duplicate entries.
		TArray<UMaterial*> Materials;
		INT MaterialCount = StaticMeshComponent->StaticMesh->LODModels(0).Elements.Num();
		for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
		{
			UMaterialInterface* MaterialInterface = StaticMeshComponent->GetMaterial( MaterialIndex );
			if( MaterialInterface && MaterialInterface->GetMaterial(MSP_SM3) )
			{
				Materials.AddUniqueItem( MaterialInterface->GetMaterial(MSP_SM3) );
			}
		}

		// Iterate over materials and create/ update associated stats.
		for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
		{
			UMaterial* Material	= Materials(MaterialIndex);	
			FAnalyzeReferencedContentStat::FMaterialStats* MaterialStats = ReferencedContentStat.GetMaterialStats( Material );
			MaterialStats->NumStaticMeshInstancesAppliedTo++;
		}

		// Track static meshes used by static mesh components.
		const UBOOL bBelongsToAParticleSystemComponent = StaticMeshComponent->GetOuter()->IsA(UParticleSystemComponent::StaticClass());

		if( bBelongsToAParticleSystemComponent == FALSE )
		{
			FAnalyzeReferencedContentStat::FStaticMeshStats* StaticMeshStats = ReferencedContentStat.GetStaticMeshStats( StaticMeshComponent->StaticMesh, LevelPackage );
			StaticMeshStats->NumInstances++;

			//warnf( TEXT("HandleStaticMeshComponent SMC: %s   Outer: %s  %d"), *StaticMeshComponent->StaticMesh->GetFullName(), *StaticMeshComponent->GetOuter()->GetFullName(), StaticMeshStats->NumInstances );
		}	
	}
}

/**
* Handles gathering stats for passed in skeletal mesh.
*
* @param SkeletalMesh	SkeletalMesh to gather stats for.
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleSkeletalMesh( USkeletalMesh* SkeletalMesh, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	FAnalyzeReferencedContentStat::FSkeletalMeshStats* SkeletalMeshStats = ReferencedContentStat.GetSkeletalMeshStats( SkeletalMesh, LevelPackage );

	if (appStristr(*SkeletalMesh->GetName(), TEXT("destruct")) != NULL)
	{
		debugf(TEXT("HandleSkeletalMesh MeshName:%s"), *SkeletalMesh->GetFullName());
	}

	SkeletalMeshStats->AddLevelInfo( LevelPackage, TRUE );

	if( bIsScriptReferenced )
	{
		SkeletalMeshStats->bIsReferencedByScript = TRUE;
	}

	// Populate materials array, avoiding duplicate entries.
	TArray<UMaterial*> Materials;
	// @todo need to do foreach over all LODModels
	INT MaterialCount = SkeletalMesh->Materials.Num();
	for (INT MaterialIndex = 0; MaterialIndex < MaterialCount; MaterialIndex++)
	{
		UMaterialInterface* MaterialInterface = SkeletalMesh->Materials(MaterialIndex);
		if( MaterialInterface && MaterialInterface->GetMaterial(MSP_SM3) )
		{
			Materials.AddUniqueItem( MaterialInterface->GetMaterial(MSP_SM3) );
		}
	}

	// Iterate over materials and create/ update associated stats.
	for (INT MaterialIndex = 0; MaterialIndex < Materials.Num(); MaterialIndex++)
	{
		UMaterial* Material	= Materials(MaterialIndex);	
		FAnalyzeReferencedContentStat::FMaterialStats* MaterialStats = ReferencedContentStat.GetMaterialStats( Material );
		MaterialStats->SkeletalMeshesAppliedTo.Set( *SkeletalMesh->GetFullName(), TRUE );
	}
}

/**
* Handles gathering stats for passed in skeletal mesh component.
*
* @param SkeletalMeshComponent	SkeletalMeshComponent to gather stats for
* @param LevelPackage	Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleSkeletalMeshComponentForSMC( USkeletalMeshComponent* SkeletalMeshComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	if( SkeletalMeshComponent->SkeletalMesh && SkeletalMeshComponent->SkeletalMesh->LODModels.Num() )
	{
		// Populate materials array, avoiding duplicate entries.
		TArray<UMaterial*> Materials;
		INT MaterialCount = SkeletalMeshComponent->SkeletalMesh->Materials.Num();
		for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
		{
			UMaterialInterface* MaterialInterface = SkeletalMeshComponent->GetMaterial( MaterialIndex );
			if( MaterialInterface && MaterialInterface->GetMaterial(MSP_SM3) )
			{
				Materials.AddUniqueItem( MaterialInterface->GetMaterial(MSP_SM3) );
			}
		}

		// Iterate over materials and create/ update associated stats.
		for (INT MaterialIndex = 0; MaterialIndex < Materials.Num(); MaterialIndex++)
		{
			UMaterial* Material	= Materials(MaterialIndex);	
			FAnalyzeReferencedContentStat::FMaterialStats* MaterialStats = ReferencedContentStat.GetMaterialStats( Material );
			MaterialStats->NumSkeletalMeshInstancesAppliedTo++;
		}

		// Track skeletal meshes used by skeletal mesh components.
		const UBOOL bBelongsToAParticleSystemComponent = SkeletalMeshComponent->GetOuter()->IsA(UParticleSystemComponent::StaticClass());

		if( bBelongsToAParticleSystemComponent == FALSE )
		{
			FAnalyzeReferencedContentStat::FSkeletalMeshStats* SkeletalMeshStats = ReferencedContentStat.GetSkeletalMeshStats( SkeletalMeshComponent->SkeletalMesh, LevelPackage );
			SkeletalMeshStats->NumInstances++;

			//warnf( TEXT("HandleSkeletalMeshComponent SMC: %s   Outer: %s  %d"), *SkeletalMeshComponent->SkeletalMesh->GetFullName(), *SkeletalMeshComponent->GetOuter()->GetFullName(), SkeletalMeshStats->NumInstances );
		}	
	}
}

/**
* Handles gathering stats for passed in sound cue.
*
* @param SoundCue				SoundCue to gather stats for.
* @param LevelPackage			Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced	Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleSoundCue( USoundCue* SoundCue, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	FAnalyzeReferencedContentStat::FSoundCueStats* SoundCueStats = ReferencedContentStat.GetSoundCueStats( SoundCue, LevelPackage );
	// Only handle further if we have a level package.
	SoundCueStats->AddLevelInfo( LevelPackage, TRUE );

	// Mark as being referenced by script.
	if( bIsScriptReferenced )
	{
		SoundCueStats->bIsReferencedByScript = TRUE;
	}
}

/**
* Handles gathering stats for passed in shadow map 1D.
*
* @param SoundCue				SoundCue to gather stats for.
* @param LevelPackage			Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced	Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleShadowMap1D( UShadowMap1D* ShadowMap1D, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	FAnalyzeReferencedContentStat::FShadowMap1DStats* ShadowMap1DStats = ReferencedContentStat.GetShadowMap1DStats( ShadowMap1D, LevelPackage );
	// Only handle further if we have a level package.
	ShadowMap1DStats->AddLevelInfo( LevelPackage, TRUE );
}

/**
* Handles gathering stats for passed in shadow map 2D.
*
* @param SoundCue				SoundCue to gather stats for.
* @param LevelPackage			Currently loaded level package, can be NULL if not a level
* @param bIsScriptReferenced	Whether object is handled because there is a script reference
*/
void UAnalyzeReferencedContentCommandlet::HandleShadowMap2D( UShadowMap2D* ShadowMap2D, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	FAnalyzeReferencedContentStat::FShadowMap2DStats* ShadowMap2DStats = ReferencedContentStat.GetShadowMap2DStats( ShadowMap2D, LevelPackage );
	// Only handle further if we have a level package.
	ShadowMap2DStats->AddLevelInfo( LevelPackage, TRUE );
}

/** This will write out the specified Stats to the AssetStatsCSVs dir **/
template< typename STAT_TYPE >
void FAnalyzeReferencedContentStat::WriteOutCSVs( const TMap<FString,STAT_TYPE>& StatsData, const FString& CSVDirectory, const FString& StatsName )
{
	if (StatsData.Num() > 0)
	{
		// Re-used helper variables for writing to CSV file.
		FString		CSVFilename		= TEXT("");
		FArchive*	CSVFile			= NULL;


		// Create CSV folder in case it doesn't exist yet.
		GFileManager->MakeDirectory( *CSVDirectory );

		// CSV: Human-readable spreadsheet format.
		CSVFilename	= FString::Printf(TEXT("%s%s-%s-%i.csv"), *CSVDirectory, *StatsName, GGameName, GetChangeListNumberForPerfTesting() );
		CSVFile	= GFileManager->CreateFileWriter( *CSVFilename );
		if( CSVFile != NULL )
		{	
			// Write out header row.
			const FString& HeaderRow = STAT_TYPE::GetCSVHeaderRow();
			CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

			// Write out each individual stats row.
			for( TMap<FString,STAT_TYPE>::TConstIterator It(StatsData); It; ++ It )
			{
				const STAT_TYPE& StatsEntry = It.Value();
				const FString& Row = StatsEntry.ToCSV();
				if( Row.Len() > 0 &&
					StatsEntry.ShouldLogStat() )
				{
					CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
				}
			}

			// Close and delete archive.
			CSVFile->Close();
			delete CSVFile;
		}
		else
		{
			debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
		}
	}
}

/** This will write out the specified Stats to the CSVDirectory/Level Dir**/
template< typename STAT_TYPE >
void FAnalyzeReferencedContentStat::WriteOutCSVsPerLevel( const TMap<FString,STAT_TYPE>& StatsData, const FString& CSVDirectory, const FString& StatsName )
{
	if (StatsData.Num() > 0)
	{
		// this will now re organize the data into Level based statistics 
		// (we can template this to do it for any stat)
		typedef TMap<FString,TArray<STAT_TYPE>> LevelToDataMapType;
		LevelToDataMapType LevelToDataMap;

		for( TMap<FString,STAT_TYPE>::TConstIterator It(StatsData); It; ++ It )
		{
			const STAT_TYPE& StatsEntry = It.Value();

			for( FAnalyzeReferencedContentStat::PerLevelDataMap::TConstIterator Itr(StatsEntry.LevelNameToInstanceCount); Itr; ++Itr )
			{
				// find the map name in our LevelToDataMap
				TArray<STAT_TYPE>* LevelData = LevelToDataMap.Find( Itr.Key() );
				// if the data exists for this level then we need to add it
				if( LevelData == NULL )
				{
					TArray<STAT_TYPE> NewArray;
					NewArray.AddItem( StatsEntry );
					LevelToDataMap.Set( Itr.Key(), NewArray  );
				}
				else
				{
					LevelData->AddItem( StatsEntry );
				}
			}
		}

		// Re-used helper variables for writing to CSV file.
		FString		CSVFilename		= TEXT("");
		FArchive*	CSVFile			= NULL;

		// so now we just need to print them all out per level as we have a list of STAT_TYPE and we can use our modified ToCSV which takes a levelname
		for( LevelToDataMapType::TConstIterator Itr(LevelToDataMap); Itr; ++Itr )
		{
			const FString LevelSubDir = CSVDirectory + FString::Printf( TEXT("%s"), PATH_SEPARATOR TEXT("Levels") PATH_SEPARATOR );
			// CSV: Human-readable spreadsheet format.
			CSVFilename	= FString::Printf(TEXT("%s%s-%s-%i.csv")
				, *LevelSubDir
				, *FString::Printf( TEXT( "%s-%s" ), *Itr.Key(), *StatsName )
				, GGameName
				, GetChangeListNumberForPerfTesting()
				);

			CSVFile = GFileManager->CreateFileWriter( *CSVFilename );
			if( CSVFile != NULL )
			{	
				// Write out header row.
				const FString& HeaderRow = STAT_TYPE::GetCSVHeaderRow();
				CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

				// Write out each individual stats row.
				for( TArray<STAT_TYPE>::TConstIterator It(Itr.Value()); It; ++It )
				{
					const STAT_TYPE& StatsEntry = *It;
					const FString& Row = StatsEntry.ToCSV( Itr.Key() );
					if( Row.Len() > 0 )
					{
						CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
					}
				}

				// Close and delete archive.
				CSVFile->Close();
				delete CSVFile;
			}
			else
			{
				debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
			}
		}
	}
}
/**
* Setup the commandlet's platform setting based on commandlet params
* @param Params The commandline parameters to the commandlet - should include "platform=xxx"
*/
/*UBOOL UAnalyzeReferencedContentCommandlet::SetPlatform(const FString& Params)
{
// default to success
UBOOL Ret = TRUE;

FString PlatformStr;
if (Parse(*Params, TEXT("PLATFORM="), PlatformStr))
{
if (PlatformStr == TEXT("PS3"))
{
Platform = UE3::PLATFORM_PS3;
}
else if (PlatformStr == TEXT("xenon") || PlatformStr == TEXT("xbox360"))
{	
Platform = UE3::PLATFORM_Xenon;
}
else if (PlatformStr == TEXT("pc") || PlatformStr == TEXT("win32"))
{
Platform = UE3::PLATFORM_Windows;
}
else
{
// this is a failure
Ret = FALSE;
}
}
else
{
Ret = FALSE;
}

return Ret;
}*/

/**
* Fill up persistent map list among MapList
* @param MapList list of name of maps - only map names
*/
INT UAnalyzeReferencedContentCommandlet::FillPersistentMapList( const TArray<FString>& MapList )
{
	TMultiMap<FString, FString>		SubLevelMap;

	PersistentMapList.Empty();

	TArray<FString> LocalMapList = MapList;

	// Collect garbage, going back to a clean slate.
	UObject::CollectGarbage( RF_Native );

	// go through load sublevels and fill up SubLevelMap
	for ( INT I=0; I<LocalMapList.Num(); ++I )
	{
		FFilename Filename = LocalMapList(I);
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package )
		{
			// Find the world and load all referenced levels.
			UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
			if( World )
			{
				// Iterate over streaming level objects loading the levels.		
				AWorldInfo* WorldInfo	= World->GetWorldInfo();
				for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
				{
					ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
					if( StreamingLevel )
					{
						// Load package if found.
						FString SubFilename;
						if( GPackageFileCache->FindPackageFile( *StreamingLevel->PackageName.ToString(), NULL, SubFilename ) )
						{
							SubLevelMap.Add(Filename, SubFilename);
						}
					}
				}
			}
			else // world is not found, then please remove me from LocalMapList
			{
				LocalMapList.Remove(I);
				--I;
			}
		}
		else // package is unloadable, then please remove me from LocalMapList
		{
			LocalMapList.Remove(I);
			--I;
		}
	}

	// Collect garbage, going back to a clean slate.
	UObject::CollectGarbage( RF_Native );

	TArray<FString> SubLevelList;
	// get all sublevel list first
	SubLevelMap.GenerateValueArray( SubLevelList );

	if ( SubLevelList.Num() )
	{
		// Now see if this map can be found in sublevel list
		for ( INT I=0; I<LocalMapList.Num(); ++I )
		{
			// if I can't find current map in the sublevel list, 
			// this doesn't work if it goes down to 2-3 layer 
			// say the command for this commandlet was only Level1, Level2, Level3
			// this will go through find Level-1, Level1-2, Level1-3
			if ( SubLevelList.ContainsItem( LocalMapList(I) ) == FALSE )
			{
				PersistentMapList.AddUniqueItem( LocalMapList(I) );
			}
		}
	}
	else
	{
		PersistentMapList = LocalMapList;
	}

	return PersistentMapList.Num();
}
/**
* Handle Persistent FaceFXAnimset
* @param MapList list of name of maps - only map names
*/
UBOOL UAnalyzeReferencedContentCommandlet::HandlePersistentFaceFXAnimSet( const TArray<FString>& MapList, UBOOL bEnableLog  )
{
#if WITH_FACEFX
	if ( FillPersistentMapList(MapList) > 0 )
	{
		// this fills up PersistentMapList
		PersistentFaceFXAnimSetGenerator.SetLogPersistentFaceFXGeneration( bEnableLog );
		PersistentFaceFXAnimSetGenerator.GeneratePersistentMapList(MapList);
		PersistentFaceFXAnimSetGenerator.SetupScriptReferencedFaceFXAnimSets();

		UFaceFXAnimSet * FaceFXAnimSet;
		for (INT I=0; I<PersistentMapList.Num(); ++I)
		{
			FFilename Filename = PersistentMapList(I);
			// generate persistent map facefx animset
			FaceFXAnimSet = PersistentFaceFXAnimSetGenerator.GeneratePersistentMapFaceFXAnimSet( Filename );
			if (FaceFXAnimSet)
			{
				// I need to load package later because GeneratePersistentMap function GCed if I do it earlier
				UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
				if( Package  )
				{
					HandleFaceFXAnimSet( FaceFXAnimSet, Package, FALSE );
				}
			}
		}

		// Collect garbage, going back to a clean slate.
		UObject::CollectGarbage( RF_Native );

		return TRUE;
	}
#endif

	return FALSE;
}

INT UAnalyzeReferencedContentCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Whether to only deal with map files.
	const UBOOL bShouldOnlyLoadMaps	= Switches.FindItemIndex(TEXT("MAPSONLY")) != INDEX_NONE;
	// Whether to exclude script references.
	const UBOOL bExcludeScript		= Switches.FindItemIndex(TEXT("EXCLUDESCRIPT")) != INDEX_NONE;
	// Whether to load non native script packages (e.g. useful for seeing what will always be loaded)
	const UBOOL bExcludeNonNativeScript = Switches.FindItemIndex(TEXT("EXCLUDENONNATIVESCRIPT")) != INDEX_NONE;
	// Whether to automatically load all the sublevels from the world
	const UBOOL bAutoLoadSublevels = Switches.FindItemIndex(TEXT("LOADSUBLEVELS")) != INDEX_NONE;

	INT IgnoreObjects = 0;
	if (Switches.FindItemIndex(TEXT("IGNORESTATICMESH")) != INDEX_NONE)			IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_StaticMesh;
	if (Switches.FindItemIndex(TEXT("IGNORESMC")) != INDEX_NONE)				IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_StaticMeshComponent;
	if (Switches.FindItemIndex(TEXT("IGNORESTATICMESHACTOR")) != INDEX_NONE)	IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_StaticMeshActor;
	if (Switches.FindItemIndex(TEXT("IGNORETEXTURE")) != INDEX_NONE)			IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_Texture;
	if (Switches.FindItemIndex(TEXT("IGNOREMATERIAL")) != INDEX_NONE)			IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_Material;
	if (Switches.FindItemIndex(TEXT("IGNOREPARTICLE")) != INDEX_NONE)			IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_Particle;
	if (Switches.FindItemIndex(TEXT("IGNOREANIMS")) != INDEX_NONE)				IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_Anim;
	if (Switches.FindItemIndex(TEXT("IGNORESOUNDCUE")) != INDEX_NONE)			IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_SoundCue;
	if (Switches.FindItemIndex(TEXT("IGNOREBRUSH")) != INDEX_NONE)				IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_Brush;
	if (Switches.FindItemIndex(TEXT("IGNORELEVEL")) != INDEX_NONE)				IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_Level;
	if (Switches.FindItemIndex(TEXT("IGNORESHADOWMAP")) != INDEX_NONE)			IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_ShadowMap;
	if (Switches.FindItemIndex(TEXT("IGNOREFACEFX")) != INDEX_NONE)				IgnoreObjects |= FAnalyzeReferencedContentStat::IGNORE_FaceFXAnimSet;

	ReferencedContentStat.SetIgnoreObjectFlag(IgnoreObjects);

	if( bExcludeNonNativeScript == FALSE )
	{
		// Load up all script files in EditPackages.
		const UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEngine);
		for( INT i=0; i<EditorEngine->EditPackages.Num(); i++ )
		{
			LoadPackage( NULL, *EditorEngine->EditPackages(i), LOAD_NoWarn );
		}
	}

	// Mark loaded objects as they are part of the always loaded set and are not taken into account for stats.
	for( TObjectIterator<UObject> It; It; ++It )
	{
		UObject* Object = *It;
		// Script referenced asset.
		if( !bExcludeScript )
		{
			HandleObject( Object, NULL, TRUE );
		}
		// Mark object as always loaded so it doesn't get counted multiple times.
		Object->SetFlags( RF_Marked );
	}

	TArray<FString> FileList;

	// Build package file list from passed in command line if tokens are specified.
	if( Tokens.Num() )
	{
		for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			// Lookup token in file cache and add filename if found.
			FString OutFilename;
			if( GPackageFileCache->FindPackageFile( *Tokens(TokenIndex), NULL, OutFilename ) )
			{
				new(FileList)FString(OutFilename);
			}
		}
	}
	// Or use all files otherwise.
	else
	{
		FileList = GPackageFileCache->GetPackageFileList();
	}

	if( FileList.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT("No packages found") );
		return 1;
	}

	// Find shader types.
	FAnalyzeReferencedContentStat::FMaterialStats::SetupShaders();

	// include only map list that has been loaded
	TArray<FString> MapFileList;

	// Iterate over all files, loading up ones that have the map extension..
	for( INT FileIndex=0; FileIndex<FileList.Num(); FileIndex++ )
	{
		const FFilename& Filename = FileList(FileIndex);		

		// Disregard filenames that don't have the map extension if we're in MAPSONLY mode.
		if( bShouldOnlyLoadMaps && (Filename.GetExtension() != FURL::DefaultMapExt) )
		{
			continue;
		}

		// Skip filenames with the script extension. @todo: don't hardcode .u as the script file extension
		if( (Filename.GetExtension() == TEXT("u")) )
		{
			continue;
		}

		warnf( NAME_Log, TEXT("Loading %s"), *Filename );
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}
		else
		{
			// Find the world and load all referenced levels.
			UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
			if( bAutoLoadSublevels && World )
			{
				AWorldInfo* WorldInfo	= World->GetWorldInfo();
				// Iterate over streaming level objects loading the levels.
				for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
				{
					ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
					if( StreamingLevel )
					{
						// Load package if found.
						FString SubFilename;
						if( GPackageFileCache->FindPackageFile( *StreamingLevel->PackageName.ToString(), NULL, SubFilename ) )
						{
							warnf(NAME_Log, TEXT("Loading sub-level %s"), *SubFilename);
							UObject::LoadPackage( NULL, *SubFilename, LOAD_None );

							// this is map - so add to MapFileList - 
							// FIXME: Just for test
							MapFileList.AddUniqueItem(SubFilename);
						}
					}
				}
			}

			// Figure out whether package is a map or content package.
			UBOOL bIsAMapPackage = World ? TRUE : FALSE;

			if ( bIsAMapPackage )
			{
				// this is map - so add to MapFileList
				MapFileList.AddUniqueItem(Filename);
			}

			// Handle currently loaded objects.
			for( TObjectIterator<UObject> It; It; ++It )
			{
				UObject* Object = *It;
				HandleObject( Object, bIsAMapPackage ? Package : NULL, FALSE );
			}

			for( TObjectIterator<UStaticMesh> It; It; ++It )
			{
				UStaticMesh* Mesh = *It;
				if (appStristr(*Mesh->GetName(), TEXT("destruct")) != NULL)
				{
					debugf(TEXT("ITERATOR MeshName:%s"), *Mesh->GetFullName());
				}
			}
		}

		// Collect garbage, going back to a clean slate.
		UObject::CollectGarbage( RF_Native );

		// Verify that everything we cared about got cleaned up correctly.
		UBOOL bEncounteredUnmarkedObject = FALSE;
		for( TObjectIterator<UObject> It; It; ++It )
		{
			UObject* Object = *It;
			if( !Object->HasAllFlags( RF_Marked ) && !Object->IsIn(UObject::GetTransientPackage()) )
			{
				bEncounteredUnmarkedObject = TRUE;
				debugf(TEXT("----------------------------------------------------------------------------------------------------"));
				debugf(TEXT("%s didn't get cleaned up!"),*Object->GetFullName());
				UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=%s NAME=%s"),*Object->GetClass()->GetName(),*Object->GetPathName()));
				TMap<UObject*,UProperty*>	Route		= FArchiveTraceRoute::FindShortestRootPath( Object, TRUE, RF_Native  );
				FString						ErrorString	= FArchiveTraceRoute::PrintRootPath( Route, Object );
				debugf(TEXT("%s"),*ErrorString);
			}
		}
		check(!bEncounteredUnmarkedObject);
	}

	// now MapFileList should be filled, go for persistentfacefx animset generator
	if ( UsingPersistentFaceFXAnimSetGenerator() )
	{
		// FIXME
		HandlePersistentFaceFXAnimSet( MapFileList, TRUE );
	}

	// Get time as a string
	const FString CurrentTime = appSystemTimeString();

	// Re-used helper variables for writing to CSV file.
	const FString CSVDirectory = appGameLogDir() + TEXT("AssetStatsCSVs") + PATH_SEPARATOR + FString::Printf( TEXT("%s-%d-%s"), GGameName, GetChangeListNumberForPerfTesting(), *CurrentTime ) + PATH_SEPARATOR;

	// Create CSV folder in case it doesn't exist yet.
	GFileManager->MakeDirectory( *CSVDirectory );

	ReferencedContentStat.WriteOutAllAvailableStatData( CSVDirectory );

	return 0;
}

