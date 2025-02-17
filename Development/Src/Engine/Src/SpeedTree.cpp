/*=============================================================================
	SpeedTree.cpp: SpeedTree implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "SpeedTree.h"
#include "ScenePrivate.h"

#if WITH_SPEEDTREE

#pragma pack(push,8)
#pragma warning(push)
#pragma warning(disable : 4548) // expression before comma has no effect; expected expression with side-effect  (needed as xlocale does not compile cleanly)
#include "../../../External/SpeedTreeRT/include/SpeedTreeAllocator.h"
#pragma warning(pop)
#pragma pack(pop)

// Link against the SpeedTreeRT libraries.
#if WIN32
#ifdef _DEBUG
#pragma comment(lib,"../External/SpeedTreeRT/lib/PC/Debug/SpeedTreeRT_Static_d.lib")
#else
#pragma comment(lib,"../External/SpeedTreeRT/lib/PC/Release/SpeedTreeRT_Static.lib")
#endif
#elif XBOX
	#ifdef _DEBUG
		#pragma comment(lib,"../External/SpeedTreeRT/lib/Xenon/Debug/SpeedTreeRT_Static_d.lib")
	#else
		#pragma comment(lib,"../External/SpeedTreeRT/lib/Xenon/Release/SpeedTreeRT_Static.lib")
	#endif
#endif

CSpeedTreeRT::SGeometry GSpeedTreeGeometry;

#endif

IMPLEMENT_CLASS(ASpeedTreeActor);

/**
 * Function that gets called from within Map_Check to allow this actor to check itself
 * for any potential errors and register them with map check dialog.
 */
void ASpeedTreeActor::CheckForErrors()
{
	Super::CheckForErrors();

	// Check that materials applied to tree are only two-sided when needed.
	if( SpeedTreeComponent && SpeedTreeComponent->SpeedTree )
	{
		// Component materials override asset material reference.
		UMaterial* BranchMaterial = SpeedTreeComponent->BranchMaterial ? SpeedTreeComponent->BranchMaterial->GetMaterial() : NULL;
		UMaterial* LeafMaterial = SpeedTreeComponent->LeafMaterial ? SpeedTreeComponent->LeafMaterial->GetMaterial() : NULL;
		UMaterial* BillboardMaterial = SpeedTreeComponent->BillboardMaterial ? SpeedTreeComponent->BillboardMaterial->GetMaterial() : NULL;

		// Use material from asset if not overridden by component.
		if( !BranchMaterial )
		{
			BranchMaterial = SpeedTreeComponent->SpeedTree->BranchMaterial ? SpeedTreeComponent->SpeedTree->BranchMaterial->GetMaterial() : NULL;
		}
		if( !LeafMaterial )
		{
			LeafMaterial = SpeedTreeComponent->SpeedTree->LeafMaterial ? SpeedTreeComponent->SpeedTree->LeafMaterial->GetMaterial() : NULL;
		}
		if( !BillboardMaterial )
		{
			BillboardMaterial = SpeedTreeComponent->SpeedTree->BillboardMaterial ? SpeedTreeComponent->SpeedTree->BillboardMaterial->GetMaterial() : NULL;
		}

		// Only fronds should use two-sided materials as others are oriented towards the screen.
		if( BranchMaterial && BranchMaterial->TwoSided )
		{
			GWarn->MapCheck_Add(MCTYPE_WARNING, this, *FString::Printf(TEXT("SpeedTree uses two-sided branch material %s"),*BranchMaterial->GetPathName()), MCACTION_NONE );
		}
		if( LeafMaterial && LeafMaterial->TwoSided )
		{
			GWarn->MapCheck_Add(MCTYPE_WARNING, this, *FString::Printf(TEXT("SpeedTree uses two-sided leaf material %s"),*LeafMaterial->GetPathName()), MCACTION_NONE );
		}
		if( BillboardMaterial && BillboardMaterial->TwoSided )
		{
			GWarn->MapCheck_Add(MCTYPE_WARNING, this, *FString::Printf(TEXT("SpeedTree uses two-sided billboard material %s"),*BillboardMaterial->GetPathName()), MCACTION_NONE );
		}
	}
}



IMPLEMENT_CLASS(USpeedTree);

#if WITH_SPEEDTREE
	/** The UE3 implementation of the SpeedTree allocator interface, which routes all SpeedTree allocations through UE3's memory allocator. */
	class FUE3SpeedTreeAllocator : public CSpeedTreeAllocator
	{
	public:

		// CSpeedTreeAllocator interface.
		virtual void* Alloc(size_t BlockSize,size_t)
		{
			return appMalloc(BlockSize);
		}
		virtual void Free(void* Base)
		{
			appFree(Base);
		}
	};
#endif

void USpeedTree::StaticConstructor( )
{
#if WITH_SPEEDTREE

	CSpeedTreeRT::SetAllocator(new FUE3SpeedTreeAllocator);

	CSpeedTreeRT::Authorize( EPIC_SPEEDTREE_KEY );

	if (!CSpeedTreeRT::IsAuthorized( ))
	{
		GWarn->Logf(TEXT("SpeedTreeRT Error: %s"), TEXT("SpeedTreeRT is not authorized to run on this computer"));
	}

	// unreal has flipped textures, like DirectX
	CSpeedTreeRT::SetTextureFlip(TRUE);
	// set lowest LOD level to billboards
	CSpeedTreeRT::SetDropToBillboard(TRUE);
#endif
}


void USpeedTree::BeginDestroy( )
{
	Super::BeginDestroy();
#if WITH_SPEEDTREE
	if( SRH )
	{
		SRH->CleanUp();
	}
#endif
}

UBOOL USpeedTree::IsReadyForFinishDestroy()
{
#if WITH_SPEEDTREE
	return SRH ? SRH->ReleaseResourcesFence.GetNumPendingFences() == 0 : TRUE;
#else
	return TRUE;
#endif
}

void USpeedTree::FinishDestroy()
{
#if WITH_SPEEDTREE
	delete SRH;
	SRH = NULL;
#endif
	Super::FinishDestroy();
}

void USpeedTree::PostLoad()
{
	Super::PostLoad();
#if WITH_SPEEDTREE
		// Create helper object if not already present; will be NULL if duplicated as property is duplicatetransient
	if( !SRH )
	{
		SRH = new FSpeedTreeResourceHelper(this);
	}
#endif
}

#if WITH_SPEEDTREE
FSpeedTreeResourceHelper::FSpeedTreeResourceHelper( USpeedTree* InOwner )
:	Owner( InOwner )
,	bHasBranches(FALSE)
,	bHasFronds(FALSE)
,	bHasLeaves(FALSE)
,	bIsInitialized(FALSE)
,	bIsUpdated(FALSE)
,	SpeedTree(NULL)
,	BranchFrondVertexFactory(InOwner)
,	LeafCardVertexFactory(InOwner)
,	LeafMeshVertexFactory(InOwner)
,	BillboardVertexFactory(InOwner)
,	BranchElements(0)
,	FrondElements(0)
,	LeafCardElements(0)
,	LeafMeshElements(0)
,	LastWindTime(0.0f)
,	WindTimeOffset(0.0f)
,	bHasValidTexelFactors(FALSE)
{
	check(Owner);
}

void FSpeedTreeResourceHelper::CleanUp( )
{
	if (bIsInitialized)
	{
		// Begin releasing resources.
		BeginReleaseResource(&IndexBuffer);
		BeginReleaseResource(&BranchFrondPositionBuffer);
		BeginReleaseResource(&BranchFrondDataBuffer);
		BeginReleaseResource(&LeafCardPositionBuffer);
		BeginReleaseResource(&LeafCardDataBuffer);
		BeginReleaseResource(&LeafMeshPositionBuffer);
		BeginReleaseResource(&LeafMeshDataBuffer);
		BeginReleaseResource(&BillboardPositionBuffer);
		BeginReleaseResource(&BillboardDataBuffer);
		BeginReleaseResource(&BranchFrondVertexFactory);
		BeginReleaseResource(&BillboardVertexFactory);
		BeginReleaseResource(&LeafCardVertexFactory);
		BeginReleaseResource(&LeafMeshVertexFactory);

		// Defer the SpeedTree deletion until the rendering thread is done with it.
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			DeleteSpeedTree,
			CSpeedTreeRT*,SpeedTree,SpeedTree,
			{
				delete SpeedTree;
			});

		// insert a fence to signal when these commands completed
		ReleaseResourcesFence.BeginFence();

		// reset flags
		bHasBranches	= FALSE;
		bHasLeaves		= FALSE;
		bHasFronds		= FALSE;
		bIsInitialized	= FALSE;
	}
}

void FSpeedTreeResourceHelper::Load(const BYTE* Buffer, INT NumBytes)
{
	if (!CSpeedTreeRT::IsAuthorized( ))
	{
		GWarn->Logf(TEXT("SpeedTreeRT Error: %s"), TEXT("SpeedTreeRT is not authorized to run on this computer"));
		return;
	}

	// clean out any data we have
	CleanUp( );

	// make a new speedtree
	SpeedTree = new CSpeedTreeRT;

	// load the tree
	if( !SpeedTree->LoadTree( Buffer, NumBytes) )
	{
		// tree failed to load
		GWarn->Logf(TEXT("SpeedTreeRT Error: %s"), ANSI_TO_TCHAR(CSpeedTreeRT::GetCurrentError( )));
	}
	else
	{
		// set lighting methods
		SpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
		SpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
		SpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);

		// set wind methods according to tree settings
		SpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
		SpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);
		SpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);

		// save and restore the rand seed because this call to speedtree will set it to a deterministic value
		INT RandSeedSave = appRand();
		UBOOL bComputeResult = SpeedTree->Compute(NULL, Owner->RandomSeed, FALSE);
		appRandInit(RandSeedSave);

		// generate tree geometry
		if( !bComputeResult )
		{
			// tree failed to compute
			GWarn->Logf(TEXT("Error, cannot compute SpeedTree.\n\n%s"), ANSI_TO_TCHAR(CSpeedTreeRT::GetCurrentError()));
		}
		else
		{
			// get the actual random seed back out
			Owner->RandomSeed = SpeedTree->GetSeed();

			// make the leaves rock in the wind
			SpeedTree->SetLeafRockingState(TRUE);
			SpeedTree->SetNumWindMatrices(3);
			SpeedWind.SetQuantities(3, 3);

			// set flags on geometry
			bHasBranches	= SpeedTree->GetNumBranchTriangles() > 0;
			bHasFronds		= SpeedTree->GetNumFrondTriangles() > 0;
			bHasLeaves		= SpeedTree->GetNumLeafTriangles() > 0;

			// setup the vertex buffers
			if( bHasLeaves )
			{
				SetupLeaves();
			}

			SetupBillboards( );

			if( bHasBranches )
			{
				SetupBranchesOrFronds(TRUE);
			}

			if( bHasFronds )
			{
				SetupBranchesOrFronds(FALSE);
			}
			
			// prepare branch and frond position buffer
			BeginInitResource(&BranchFrondPositionBuffer);
			BeginInitResource(&BranchFrondDataBuffer);

			// prepare branch and frond vertex factory
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				InitSpeedTreeBranchFrondVertexFactory,
				FSpeedTreeResourceHelper*,ResourceHelper,this,
				{
					FSpeedTreeBranchVertexFactory::DataType Data;
					Data.PositionComponent			= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BranchFrondPositionBuffer, FSpeedTreeVertexPosition, Position, VET_Float3);
					Data.TangentBasisComponents[0]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BranchFrondDataBuffer, FSpeedTreeVertexDataBranchFrond, TangentX, VET_PackedNormal);
					Data.TangentBasisComponents[1]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BranchFrondDataBuffer, FSpeedTreeVertexDataBranchFrond, TangentY, VET_PackedNormal);
					Data.TangentBasisComponents[2]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BranchFrondDataBuffer, FSpeedTreeVertexDataBranchFrond, TangentZ, VET_PackedNormal);
					Data.WindInfo					= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BranchFrondDataBuffer, FSpeedTreeVertexDataBranchFrond, WindInfo, VET_Float4);
					while( Data.TextureCoordinates.Num( ) < 2 )
					{
						Data.TextureCoordinates.AddItem(FVertexStreamComponent( ));
					};
					Data.TextureCoordinates(0) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BranchFrondDataBuffer, FSpeedTreeVertexDataBranchFrond, TexCoord, VET_Float2);
					Data.TextureCoordinates(1) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BranchFrondDataBuffer, FSpeedTreeVertexDataBranchFrond, LODHint, VET_Float1);
					ResourceHelper->BranchFrondVertexFactory.SetData(Data);
				});
			BeginInitResource(&BranchFrondVertexFactory);

			// prepare leaf card buffers
			BeginInitResource(&LeafCardPositionBuffer);
			BeginInitResource(&LeafCardDataBuffer);

			// prepare leaf card vertex factory
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				InitSpeedTreeLeafCardVertexFactory,
				FSpeedTreeResourceHelper*,ResourceHelper,this,
				{
					FSpeedTreeLeafCardVertexFactory::DataType Data;
					Data.PositionComponent			= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafCardPositionBuffer, FSpeedTreeVertexPosition, Position, VET_Float3);
					Data.TangentBasisComponents[0]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafCardDataBuffer, FSpeedTreeVertexDataLeafCard, TangentX, VET_PackedNormal);
					Data.TangentBasisComponents[1]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafCardDataBuffer, FSpeedTreeVertexDataLeafCard, TangentY, VET_PackedNormal);
					Data.TangentBasisComponents[2]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafCardDataBuffer, FSpeedTreeVertexDataLeafCard, TangentZ, VET_PackedNormal);
					Data.WindInfo						= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafCardDataBuffer, FSpeedTreeVertexDataLeafCard, WindInfo, VET_Float4);
					while( Data.TextureCoordinates.Num( ) < 3 )
						Data.TextureCoordinates.AddItem(FVertexStreamComponent( ));
					Data.TextureCoordinates(0) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafCardDataBuffer, FSpeedTreeVertexDataLeafCard, LeafData1, VET_Float4);
					Data.TextureCoordinates(1) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafCardDataBuffer, FSpeedTreeVertexDataLeafCard, LeafData2, VET_Float4);
					Data.TextureCoordinates(2) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafCardDataBuffer, FSpeedTreeVertexDataLeafCard, TexCoord, VET_Float2);

					ResourceHelper->LeafCardVertexFactory.SetData(Data);
				});
			BeginInitResource(&LeafCardVertexFactory);

			// prepare leaf mesh buffers
			BeginInitResource(&LeafMeshPositionBuffer);
			BeginInitResource(&LeafMeshDataBuffer);

			// prepare leaf mesh vertex factory
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				InitSpeedTreeLeafMeshVertexFactory,
				FSpeedTreeResourceHelper*,ResourceHelper,this,
				{
					FSpeedTreeLeafMeshVertexFactory::DataType Data;
					Data.PositionComponent			= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshPositionBuffer, FSpeedTreeVertexPosition, Position, VET_Float3);
					Data.TangentBasisComponents[0]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshDataBuffer, FSpeedTreeVertexDataLeafMesh, TangentX, VET_PackedNormal);
					Data.TangentBasisComponents[1]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshDataBuffer, FSpeedTreeVertexDataLeafMesh, TangentY, VET_PackedNormal);
					Data.TangentBasisComponents[2]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshDataBuffer, FSpeedTreeVertexDataLeafMesh, TangentZ, VET_PackedNormal);
					Data.WindInfo						= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshDataBuffer, FSpeedTreeVertexDataLeafMesh, WindInfo, VET_Float4);
					while( Data.TextureCoordinates.Num( ) < 4 )
						Data.TextureCoordinates.AddItem(FVertexStreamComponent( ));
					Data.TextureCoordinates(0) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshDataBuffer, FSpeedTreeVertexDataLeafMesh, LeafData1, VET_Float4);
					Data.TextureCoordinates(1) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshDataBuffer, FSpeedTreeVertexDataLeafMesh, LeafData2, VET_Float3);
					Data.TextureCoordinates(2) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshDataBuffer, FSpeedTreeVertexDataLeafMesh, LeafData3, VET_Float3);
					Data.TextureCoordinates(3) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->LeafMeshDataBuffer, FSpeedTreeVertexDataLeafMesh, TexCoord, VET_Float2);

					ResourceHelper->LeafMeshVertexFactory.SetData(Data);
				});
			BeginInitResource(&LeafMeshVertexFactory);

			// prepare billboard position buffer
			BeginInitResource(&BillboardPositionBuffer);
			BeginInitResource(&BillboardDataBuffer);

			// prepare billboard vertex factory
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				InitSpeedTreeBillboardVertexFactory,
				FSpeedTreeResourceHelper*,ResourceHelper,this,
				{
					FSpeedTreeBillboardVertexFactory::DataType Data;
					Data.PositionComponent			= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BillboardPositionBuffer, FSpeedTreeVertexPosition, Position, VET_Float3);
					Data.TangentBasisComponents[0]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BillboardDataBuffer, FSpeedTreeVertexData, TangentX, VET_PackedNormal);
					Data.TangentBasisComponents[1]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BillboardDataBuffer, FSpeedTreeVertexData, TangentY, VET_PackedNormal);
					Data.TangentBasisComponents[2]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BillboardDataBuffer, FSpeedTreeVertexData, TangentZ, VET_PackedNormal);
					while( Data.TextureCoordinates.Num( ) < 3 )
						Data.TextureCoordinates.AddItem(FVertexStreamComponent( ));
					Data.TextureCoordinates(0) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BillboardDataBuffer, FSpeedTreeVertexDataBillboard, TexCoord, VET_Float2);
					Data.TextureCoordinates(1) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BillboardDataBuffer, FSpeedTreeVertexDataBillboard, bIsVerticalBillboard, VET_Float1);
					Data.TextureCoordinates(2) = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&ResourceHelper->BillboardDataBuffer, FSpeedTreeVertexDataBillboard, BillboardIndex, VET_Float1);

					ResourceHelper->BillboardVertexFactory.SetData(Data);
				});
			BeginInitResource(&BillboardVertexFactory);

			// prepare index buffer
			BeginInitResource(&IndexBuffer);

			// In the game, SpeedTree's authoritative copy of the  geometry data may be abandoned, since all we need has been copied into
			// our vertex/index buffers now.
			if(!GIsEditor)
			{
				SpeedTree->DeleteBranchGeometry();
				SpeedTree->DeleteFrondGeometry();
				SpeedTree->DeleteLeafGeometry();
				SpeedTree->DeleteTransientData();
			}

			// everything appeared to go well
			bIsInitialized = TRUE;
		}
	}

	if( !bIsInitialized )
	{
		CleanUp();
	}
}


void FSpeedTreeResourceHelper::SetupBranchesOrFronds(UBOOL bBranches)
{
	// pick whether to get branches or fronds
	DWORD SpeedTreeBitVector = (bBranches ? SpeedTree_BranchGeometry : SpeedTree_FrondGeometry);

	// get tree geometry
	SpeedTree->GetGeometry( GSpeedTreeGeometry, SpeedTreeBitVector );

	CSpeedTreeRT::SGeometry::SIndexed* IndexedGeometry = (bBranches ? &(GSpeedTreeGeometry.m_sBranches) : &(GSpeedTreeGeometry.m_sFronds));

	if (bBranches)
	{
		BranchFadingDistance = IndexedGeometry->m_fLodFadeDistance;
	}
	else
	{
		FrondFadingDistance = IndexedGeometry->m_fLodFadeDistance;
	}

	if( IndexedGeometry->m_nNumLods == 0 )
	{
		return;
	}

	// switch on either branches or fronds
	TArray<FMeshElement>& Elements = bBranches ? BranchElements : FrondElements;
	Elements.Empty(IndexedGeometry->m_nNumLods);
	Elements.AddZeroed(IndexedGeometry->m_nNumLods);

	// the first index and vertex
	INT StartIndex  = IndexBuffer.Indices.Num();
	INT StartVertex = BranchFrondDataBuffer.Vertices.Num();

	// fill the vertex array
	for( INT i=0; i<IndexedGeometry->m_nNumVertices; i++ )
	{
		// make a new vertex for the position
		FSpeedTreeVertexPosition* NewPosVertex = new(BranchFrondPositionBuffer.Vertices) FSpeedTreeVertexPosition;
		NewPosVertex->Position  = FVector(IndexedGeometry->m_pCoords[i * 3], IndexedGeometry->m_pCoords[i * 3 + 1], IndexedGeometry->m_pCoords[i * 3 + 2] - Owner->Sink);

		// make a new vertex for the data
		FSpeedTreeVertexDataBranchFrond* NewDataVertex = new(BranchFrondDataBuffer.Vertices) FSpeedTreeVertexDataBranchFrond;

		// set vertex normal, binormal, and tangent
		NewDataVertex->TangentZ = FPackedNormal(FVector(IndexedGeometry->m_pNormals[i * 3], IndexedGeometry->m_pNormals[i * 3 + 1], IndexedGeometry->m_pNormals[i * 3 + 2]));
		NewDataVertex->TangentY = FPackedNormal(FVector(IndexedGeometry->m_pBinormals[i * 3], IndexedGeometry->m_pBinormals[i * 3 + 1], IndexedGeometry->m_pBinormals[i * 3 + 2]));
		NewDataVertex->TangentX = FPackedNormal(FVector(IndexedGeometry->m_pTangents[i * 3], IndexedGeometry->m_pTangents[i * 3 + 1], IndexedGeometry->m_pTangents[i * 3 + 2]));

		// set diffuse texcoords
		NewDataVertex->TexCoord = FVector2D(IndexedGeometry->m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE][i * 2], IndexedGeometry->m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE][i * 2 + 1]);

		// wind info
		NewDataVertex->WindInfo = FVector4(IndexedGeometry->m_pWindMatrixIndices[0][i], IndexedGeometry->m_pWindWeights[0][i],
											IndexedGeometry->m_pWindMatrixIndices[1][i], IndexedGeometry->m_pWindWeights[1][i]);

		NewDataVertex->LODHint = IndexedGeometry->m_pLodFadeHints[i];
	}

	// fill index array
	for( INT LodIndex=0; LodIndex<IndexedGeometry->m_nNumLods; LodIndex++ )
	{
		Elements(LodIndex).Type				= PT_TriangleList;
		Elements(LodIndex).FirstIndex		= IndexBuffer.Indices.Num( );
		Elements(LodIndex).IndexBuffer		= &IndexBuffer;
		Elements(LodIndex).VertexFactory	= &BranchFrondVertexFactory;
		Elements(LodIndex).NumPrimitives	= 0;
		Elements(LodIndex).MinVertexIndex	= UINT(-1);
		Elements(LodIndex).MaxVertexIndex	= 0;

		// Decode the triangle strips.
		for(INT StripIndex = 0;StripIndex < IndexedGeometry->m_pNumStrips[LodIndex];StripIndex++)
		{
			const INT* const StripIndices = IndexedGeometry->m_pStrips[LodIndex][StripIndex];
			const INT NumStripIndices = IndexedGeometry->m_pStripLengths[LodIndex][StripIndex];
			for(TTriangleStripReader<INT> TriangleIt(StripIndices,NumStripIndices);TriangleIt.HasUnreadTriangles();TriangleIt.Advance())
			{
				// Read the next triangle from the triangle stip.
				INT TriangleIndices[3];
				TriangleIt.GetTriangleIndices(TriangleIndices[0],TriangleIndices[1],TriangleIndices[2]);

				// Add the triangle's vertex indices to the index buffer.
				for(INT TriangleVertexIndex = 0;TriangleVertexIndex < 3;TriangleVertexIndex++)
				{
					const INT VertexIndex = TriangleIndices[TriangleVertexIndex] + StartVertex;
					IndexBuffer.Indices.AddItem(VertexIndex);

					// Update the mesh element's min and max vertex index.
					Elements(LodIndex).MinVertexIndex = Min<UINT>(Elements(LodIndex).MinVertexIndex, VertexIndex);
					Elements(LodIndex).MaxVertexIndex = Max<UINT>(Elements(LodIndex).MaxVertexIndex, VertexIndex);
				}

				// Update the mesh element's triangle count.
				++Elements(LodIndex).NumPrimitives;
			}
		}
	}
}



void FSpeedTreeResourceHelper::SetupLeaves()
{
	bHasBillboardLeaves = FALSE;

	// get number of LODs and create arrays
	INT NumLodLevels = SpeedTree->GetNumLeafLodLevels();

	LeafCardElements.Empty(NumLodLevels);
	LeafCardElements.AddZeroed(NumLodLevels);

	LeafMeshElements.Empty(NumLodLevels);
	LeafMeshElements.AddZeroed(NumLodLevels);

	INT NumWindAngles = SpeedWind.GetNumLeafAngleMatrices( );

	// fill the vertex arrays
	for( INT LodIndex=0; LodIndex<NumLodLevels; LodIndex++ )
	{
		// override LOD and get geometry
		SpeedTree->GetGeometry( GSpeedTreeGeometry, SpeedTree_LeafGeometry );
		CSpeedTreeRT::SGeometry::SLeaf* Leaf = &GSpeedTreeGeometry.m_pLeaves[LodIndex];

		if (LodIndex == 0)
		{
			LeafAngleScalars.X = Leaf->m_fLeafRockScalar;
			LeafAngleScalars.Y = Leaf->m_fLeafRustleScalar;
		}

		// setup card section
		LeafCardElements(LodIndex).FirstIndex		= IndexBuffer.Indices.Num();
		LeafCardElements(LodIndex).MinVertexIndex	= LeafCardPositionBuffer.Vertices.Num();
		LeafCardElements(LodIndex).IndexBuffer		= &IndexBuffer;
		LeafCardElements(LodIndex).VertexFactory	= &LeafCardVertexFactory;
		LeafCardElements(LodIndex).Type				= PT_TriangleList;

		// cycle through leaves for cards
		for( INT LeafIndex=0; LeafIndex<Leaf->m_nNumLeaves; LeafIndex++ )
		{
			if( Leaf->m_pCards[Leaf->m_pLeafCardIndices[LeafIndex]].m_pMesh == NULL )
			{
				INT FirstLeafVertex = LeafCardPositionBuffer.Vertices.Num();

				CSpeedTreeRT::SGeometry::SLeaf::SCard* pCard = &Leaf->m_pCards[Leaf->m_pLeafCardIndices[LeafIndex]];

				// Flag the tree as having billboarded leaves.
				bHasBillboardLeaves = TRUE;

				// cycle through indices
				for( INT VertexIndex=0; VertexIndex<4; VertexIndex++ )
				{
					// set position (all 4 to center, but specify which corner)
					FSpeedTreeVertexPosition* NewPosVertex = new(LeafCardPositionBuffer.Vertices) FSpeedTreeVertexPosition;
					NewPosVertex->Position = FVector(Leaf->m_pCenterCoords[LeafIndex * 3], 
													Leaf->m_pCenterCoords[LeafIndex * 3 + 1], 
													Leaf->m_pCenterCoords[LeafIndex * 3 + 2] - Owner->Sink);

					// make vertex data
					FSpeedTreeVertexDataLeafCard* NewDataVertex = new(LeafCardDataBuffer.Vertices) FSpeedTreeVertexDataLeafCard;

					// set vertex normal, binormal, and tangent
					INT NormBase = LeafIndex * 12 + VertexIndex * 3;
					FVector VecNormal(Leaf->m_pNormals[NormBase], Leaf->m_pNormals[NormBase + 1], Leaf->m_pNormals[NormBase + 2]);
					FVector VecBinormal(Leaf->m_pBinormals[NormBase], Leaf->m_pBinormals[NormBase + 1], Leaf->m_pBinormals[NormBase + 2]);
					FVector VecTangent(Leaf->m_pTangents[NormBase], Leaf->m_pTangents[NormBase + 1], Leaf->m_pTangents[NormBase + 2]);

					NewDataVertex->TangentZ = FPackedNormal(VecNormal);
					NewDataVertex->TangentY = FPackedNormal(VecBinormal);
					NewDataVertex->TangentX = FPackedNormal(VecTangent);

					// set vertex texcoord0
					NewDataVertex->TexCoord = FVector2D(pCard->m_pTexCoords[VertexIndex * 2], pCard->m_pTexCoords[VertexIndex * 2 + 1]);

					// wind info
					NewDataVertex->WindInfo = FVector4(Leaf->m_pWindMatrixIndices[0][LeafIndex], Leaf->m_pWindWeights[0][LeafIndex],
														Leaf->m_pWindMatrixIndices[1][LeafIndex], Leaf->m_pWindWeights[1][LeafIndex]);

					// other card info
					static const FLOAT DegToRad = 0.017453292519943296;
					NewDataVertex->LeafData1 = FVector4(pCard->m_fWidth, pCard->m_fHeight, pCard->m_afPivotPoint[0] - 0.5f, pCard->m_afPivotPoint[1] - 0.5f);
					NewDataVertex->LeafData2 = FVector4(DegToRad * pCard->m_afAngleOffsets[0], DegToRad * pCard->m_afAngleOffsets[1],
														FLOAT(LeafIndex % NumWindAngles), VertexIndex);
				}

				// add to index buffer
				IndexBuffer.Indices.AddItem(FirstLeafVertex + 0);
				IndexBuffer.Indices.AddItem(FirstLeafVertex + 1);
				IndexBuffer.Indices.AddItem(FirstLeafVertex + 2);
				IndexBuffer.Indices.AddItem(FirstLeafVertex + 0);
				IndexBuffer.Indices.AddItem(FirstLeafVertex + 2);
				IndexBuffer.Indices.AddItem(FirstLeafVertex + 3);
			}
		}

		LeafCardElements(LodIndex).NumPrimitives	= (IndexBuffer.Indices.Num( ) - LeafCardElements(LodIndex).FirstIndex) / 3;
		LeafCardElements(LodIndex).MaxVertexIndex	= LeafCardPositionBuffer.Vertices.Num( ) - 1;


		// setup mesh section
		LeafMeshElements(LodIndex).FirstIndex		= IndexBuffer.Indices.Num();
		LeafMeshElements(LodIndex).MinVertexIndex	= LeafMeshPositionBuffer.Vertices.Num();
		LeafMeshElements(LodIndex).IndexBuffer		= &IndexBuffer;
		LeafMeshElements(LodIndex).VertexFactory	= &LeafMeshVertexFactory;
		LeafMeshElements(LodIndex).Type				= PT_TriangleList;

		// cycle through leaves for meshes
		for( INT LeafIndex=0; LeafIndex<Leaf->m_nNumLeaves; LeafIndex++ )
		{
			INT FirstLeafVertex = LeafMeshPositionBuffer.Vertices.Num();

			if( Leaf->m_pCards[Leaf->m_pLeafCardIndices[LeafIndex]].m_pMesh != NULL )
			{
				// use the leaf mesh
				const CSpeedTreeRT::SGeometry::SLeaf::SMesh* Mesh = Leaf->m_pCards[Leaf->m_pLeafCardIndices[LeafIndex]].m_pMesh;
				CSpeedTreeRT::SGeometry::SLeaf::SCard* pCard = &Leaf->m_pCards[Leaf->m_pLeafCardIndices[LeafIndex]];

				FMatrix MatRotate(	*(FVector*)(&Leaf->m_pTangents[LeafIndex * 12]),
									*(FVector*)(&Leaf->m_pBinormals[LeafIndex * 12]), 
									*(FVector*)(&Leaf->m_pNormals[LeafIndex * 12]),
									FVector(0.0f, 0.0f, 0.0f));

				for( INT VertexIndex=0; VertexIndex<Mesh->m_nNumVertices; VertexIndex++ )
				{					
					FSpeedTreeVertexPosition* NewPosVertex = new(LeafMeshPositionBuffer.Vertices) FSpeedTreeVertexPosition;
					NewPosVertex->Position = FVector(Leaf->m_pCenterCoords[LeafIndex * 3], 
											 		Leaf->m_pCenterCoords[LeafIndex * 3 + 1], 
											 		Leaf->m_pCenterCoords[LeafIndex * 3 + 2] - Owner->Sink);

					// make vertex data
					FSpeedTreeVertexDataLeafMesh* NewDataVertex = new (LeafMeshDataBuffer.Vertices) FSpeedTreeVertexDataLeafMesh;

					// set vertex normal, binormal, and tangent
					FVector VecNormal(Mesh->m_pNormals[VertexIndex * 3 + 0], Mesh->m_pNormals[VertexIndex * 3 + 1], Mesh->m_pNormals[VertexIndex * 3 + 2]);
					FVector VecBinormal(Mesh->m_pBinormals[VertexIndex * 3 + 0], Mesh->m_pBinormals[VertexIndex * 3 + 1], Mesh->m_pBinormals[VertexIndex * 3 + 2]);
					FVector VecTangent(Mesh->m_pTangents[VertexIndex * 3 + 0], Mesh->m_pTangents[VertexIndex * 3 + 1], Mesh->m_pTangents[VertexIndex * 3 + 2]);

					VecNormal	= MatRotate.TransformFVector(VecNormal);
					VecBinormal = MatRotate.TransformFVector(VecBinormal);
					VecTangent	= MatRotate.TransformFVector(VecTangent);

					NewDataVertex->TangentZ = FPackedNormal(VecNormal);
					NewDataVertex->TangentY = FPackedNormal(VecBinormal);
					NewDataVertex->TangentX = FPackedNormal(VecTangent);

					// set vertex texcoord0
					NewDataVertex->TexCoord = FVector2D(Mesh->m_pTexCoords[VertexIndex * 2], Mesh->m_pTexCoords[VertexIndex * 2 + 1]);

					// wind info
					NewDataVertex->WindInfo = FVector4(Leaf->m_pWindMatrixIndices[0][LeafIndex], Leaf->m_pWindWeights[0][LeafIndex],
														Leaf->m_pWindMatrixIndices[1][LeafIndex], Leaf->m_pWindWeights[1][LeafIndex]);

					// other leaf mesh info
					static const FLOAT DegToRad = 0.017453292519943296;
					NewDataVertex->LeafData1 = FVector4(Mesh->m_pCoords[VertexIndex * 3 + 0], Mesh->m_pCoords[VertexIndex * 3 + 1], Mesh->m_pCoords[VertexIndex * 3 + 2], 
														FLOAT(LeafIndex % NumWindAngles));
					NewDataVertex->LeafData2 = FVector4(Leaf->m_pTangents[LeafIndex * 12], Leaf->m_pBinormals[LeafIndex * 12 + 0], Leaf->m_pNormals[LeafIndex * 12 + 0], 0.0f);
					NewDataVertex->LeafData3 = FVector4(Leaf->m_pTangents[LeafIndex * 12 + 2], Leaf->m_pBinormals[LeafIndex * 12 + 2], Leaf->m_pNormals[LeafIndex * 12 + 2], 0.0f);
				}

				for( INT Index=0; Index<Mesh->m_nNumIndices; Index++ )
				{
					IndexBuffer.Indices.AddItem(FirstLeafVertex + Mesh->m_pIndices[Index]);
				}
			}
		}

		LeafMeshElements(LodIndex).NumPrimitives	= (IndexBuffer.Indices.Num( ) - LeafMeshElements(LodIndex).FirstIndex) / 3;
		LeafMeshElements(LodIndex).MaxVertexIndex	= LeafMeshPositionBuffer.Vertices.Num( ) - 1;
	}
}

void FSpeedTreeResourceHelper::SetupBillboards( )
{
	// the first index and vertex
	INT StartIndex	= IndexBuffer.Indices.Num();
	INT StartVertex = 0;

	// Compute the billboard geometry.
	const FLOAT CameraPosition[3] = { -100000.0f, 0.0f, 0.0f };
	const FLOAT CameraDirection[3] = { 1.0f, 0.0, 0.0f };
	SpeedTree->SetCamera(CameraPosition,CameraDirection);
	SpeedTree->SetLodLevel(0.0f);
	SpeedTree->GetGeometry(GSpeedTreeGeometry, SpeedTree_BillboardGeometry);
	SpeedTree->UpdateBillboardGeometry(GSpeedTreeGeometry);

	// add vertices to the buffers
	BillboardPositionBuffer.Vertices.Empty(12);
	BillboardPositionBuffer.Vertices.AddZeroed(12);
	BillboardDataBuffer.Vertices.Empty(12);
	BillboardDataBuffer.Vertices.AddZeroed(12);

	// check to make sure it has been exported for real-time before updating billboards
	if( GSpeedTreeGeometry.m_s360Billboard.m_pCoords != NULL )
	{
		const FLOAT Sink = Owner->Sink;

		const FVector2D BillboardUnitTexCoords[4] = 
		{
			FVector2D(1.0f, 1.0f), 
			FVector2D(0.0f, 1.0f), 
			FVector2D(0.0f, 0.0f), 
			FVector2D(1.0f, 0.0f) 
		};
		for(UINT VertexIndex = 0;VertexIndex < 4;VertexIndex++)
		{
			// compute normal/binormal/tangent for both
			FPackedNormal VecNormal(-*((FVector*)(&GSpeedTreeGeometry.m_s360Billboard.m_pNormals[VertexIndex * 3])));
			FPackedNormal VecBinormal(*((FVector*)(&GSpeedTreeGeometry.m_s360Billboard.m_pBinormals[VertexIndex * 3])));
			FPackedNormal VecTangent(*((FVector*)(&GSpeedTreeGeometry.m_s360Billboard.m_pTangents[VertexIndex * 3])));
			
			FSpeedTreeVertexDataBillboard* DataVertex = NULL;

			// first billboard
			if( GSpeedTreeGeometry.m_s360Billboard.m_pTexCoords[0] != NULL )
			{
				appMemcpy(&BillboardPositionBuffer.Vertices(VertexIndex).Position, &GSpeedTreeGeometry.m_s360Billboard.m_pCoords[VertexIndex * 3], 3 * sizeof(FLOAT));
				BillboardPositionBuffer.Vertices(VertexIndex).Position.Z -= Sink;

				DataVertex = &BillboardDataBuffer.Vertices(VertexIndex);
				DataVertex->TexCoord = BillboardUnitTexCoords[VertexIndex];
				DataVertex->TangentX = VecTangent;
				DataVertex->TangentY = VecBinormal;
				DataVertex->TangentZ = VecNormal;
				DataVertex->bIsVerticalBillboard = 1;
				DataVertex->BillboardIndex = 0;
			}

			// second billboard
			if( GSpeedTreeGeometry.m_s360Billboard.m_pTexCoords[1] != NULL )
			{
				appMemcpy(&BillboardPositionBuffer.Vertices(VertexIndex + 4).Position, &GSpeedTreeGeometry.m_s360Billboard.m_pCoords[VertexIndex * 3], 3 * sizeof(FLOAT));
				BillboardPositionBuffer.Vertices(VertexIndex + 4).Position.Z -= Sink;
	
				DataVertex = &BillboardDataBuffer.Vertices(VertexIndex + 4);
				DataVertex->TexCoord = BillboardUnitTexCoords[VertexIndex];
				DataVertex->TangentX = VecTangent;
				DataVertex->TangentY = VecBinormal;
				DataVertex->TangentZ = VecNormal;
				DataVertex->bIsVerticalBillboard = 1;
				DataVertex->BillboardIndex = 1;
			}

			// horiz billboard
			if( GSpeedTreeGeometry.m_sHorzBillboard.m_pTexCoords != NULL )
			{
				appMemcpy(&BillboardPositionBuffer.Vertices(VertexIndex + 8).Position, &GSpeedTreeGeometry.m_sHorzBillboard.m_pCoords[VertexIndex * 3], 3 * sizeof(FLOAT));
				BillboardPositionBuffer.Vertices(VertexIndex + 8).Position.Z -= Sink;

				DataVertex = &BillboardDataBuffer.Vertices(VertexIndex + 8);
				DataVertex->TexCoord = BillboardUnitTexCoords[VertexIndex];
				DataVertex->TangentX = FPackedNormal(*((FVector*)(&GSpeedTreeGeometry.m_sHorzBillboard.m_afTangents[VertexIndex * 3])));
				DataVertex->TangentY = FPackedNormal(*((FVector*)(&GSpeedTreeGeometry.m_sHorzBillboard.m_afBinormals[VertexIndex * 3])));
				DataVertex->TangentZ = FPackedNormal(*((FVector*)(&GSpeedTreeGeometry.m_sHorzBillboard.m_afNormals[VertexIndex * 3])));
				DataVertex->bIsVerticalBillboard = 0;
				DataVertex->BillboardIndex = 2;
			}
		}
	}

	// Count the maximum number of billboard triangles to be drawn.
	UINT NumBillboardTriangles = 0;
	if( GSpeedTreeGeometry.m_s360Billboard.m_pTexCoords[0] != NULL )
	{
		NumBillboardTriangles += 2;
	}
	if( GSpeedTreeGeometry.m_s360Billboard.m_pTexCoords[1] != NULL )
	{
		NumBillboardTriangles += 2;
	}
	if( GSpeedTreeGeometry.m_sHorzBillboard.m_pTexCoords != NULL )
	{
		NumBillboardTriangles += 4;
	}

	// Reset the horizontal billboard texture coordinate pointer, as UpdateBillboardGeometry doesn't set it if the tree doesn't have a horizontal billboard.
	GSpeedTreeGeometry.m_sHorzBillboard.m_pTexCoords = NULL;

	// setup index buffer
	IndexBuffer.Indices.AddItem(0 + StartVertex);
	IndexBuffer.Indices.AddItem(1 + StartVertex);
	IndexBuffer.Indices.AddItem(2 + StartVertex);
	IndexBuffer.Indices.AddItem(0 + StartVertex);
	IndexBuffer.Indices.AddItem(2 + StartVertex);
	IndexBuffer.Indices.AddItem(3 + StartVertex);

	IndexBuffer.Indices.AddItem(4 + StartVertex);
	IndexBuffer.Indices.AddItem(5 + StartVertex);
	IndexBuffer.Indices.AddItem(6 + StartVertex);
	IndexBuffer.Indices.AddItem(4 + StartVertex);
	IndexBuffer.Indices.AddItem(6 + StartVertex);
	IndexBuffer.Indices.AddItem(7 + StartVertex);

	// Negative Z facing horizontal billboard.
	IndexBuffer.Indices.AddItem(8 + StartVertex);
	IndexBuffer.Indices.AddItem(9 + StartVertex);
	IndexBuffer.Indices.AddItem(10 + StartVertex);
	IndexBuffer.Indices.AddItem(8 + StartVertex);
	IndexBuffer.Indices.AddItem(10 + StartVertex);
	IndexBuffer.Indices.AddItem(11 + StartVertex);

	// Positive Z facing horizontal billboard.
	IndexBuffer.Indices.AddItem(9 + StartVertex);
	IndexBuffer.Indices.AddItem(8 + StartVertex);
	IndexBuffer.Indices.AddItem(10 + StartVertex);
	IndexBuffer.Indices.AddItem(10 + StartVertex);
	IndexBuffer.Indices.AddItem(8 + StartVertex);
	IndexBuffer.Indices.AddItem(11 + StartVertex);


	// setup Elements
	BillboardElement.FirstIndex		= StartIndex;
	BillboardElement.NumPrimitives	= NumBillboardTriangles;
	BillboardElement.MinVertexIndex = StartVertex;
	BillboardElement.MaxVertexIndex = StartVertex + 12 - 1;
	BillboardElement.IndexBuffer	= &IndexBuffer;
	BillboardElement.VertexFactory	= &BillboardVertexFactory;
	BillboardElement.Type			= PT_TriangleList;
}


FBoxSphereBounds FSpeedTreeResourceHelper::GetBounds( )
{
	FLOAT BoundingBox[6];
	SpeedTree->GetBoundingBox(BoundingBox);
	return FBoxSphereBounds((FVector*)BoundingBox, 2);
}

void FSpeedTreeResourceHelper::UpdateWind(const FVector& WindDirection, FLOAT WindStrength, FLOAT CurrentTime)
{
	if( !bIsInitialized )
	{
		return;
	}

	if(LastWindTime != CurrentTime)
	{
		// Compute an offset to disallow negative time differences.
		const FLOAT PreviousWindTime = LastWindTime + WindTimeOffset;
		const FLOAT NewWindTime = Clamp(CurrentTime + WindTimeOffset,PreviousWindTime,PreviousWindTime + 1.0f);
		WindTimeOffset = NewWindTime - CurrentTime;

		LastWindTime = CurrentTime;

		// set speedwind values
		SpeedWind.SetMaxBendAngle(Owner->MaxBendAngle);
		SpeedWind.SetWindResponse(Owner->Response, Owner->ResponseLimiter);
		SpeedWind.SetGusting(Owner->Gusting_MinStrength, Owner->Gusting_MaxStrength, Owner->Gusting_Frequency, Owner->Gusting_MinDuration, Owner->Gusting_MaxDuration);
		SpeedWind.SetExponents(Owner->BranchExponent, Owner->LeafExponent);
		SpeedWind.SetBranchHorizontal(Owner->BranchHorizontal_LowWindAngle, Owner->BranchHorizontal_HighWindAngle, Owner->BranchHorizontal_LowWindSpeed, Owner->BranchVertical_HighWindSpeed);
		SpeedWind.SetBranchVertical(Owner->BranchVertical_LowWindAngle, Owner->BranchVertical_HighWindAngle, Owner->BranchVertical_LowWindSpeed, Owner->BranchVertical_HighWindSpeed);
		SpeedWind.SetLeafRocking(Owner->LeafRocking_LowWindAngle, Owner->LeafRocking_HighWindAngle, Owner->LeafRocking_LowWindSpeed, Owner->LeafRocking_HighWindSpeed);
		SpeedWind.SetLeafRustling(Owner->LeafRustling_LowWindAngle, Owner->LeafRustling_HighWindAngle, Owner->LeafRustling_LowWindSpeed, Owner->LeafRustling_HighWindSpeed);

		// advance speedwind
		SpeedWind.SetWindStrengthAndDirection(WindStrength, WindDirection.X, WindDirection.Y, WindDirection.Z);
		SpeedWind.Advance(CurrentTime + WindTimeOffset, TRUE, FALSE);

	    // flag for geometry update on first render
	    bIsUpdated = FALSE;
	}
}

INT FSpeedTreeResourceHelper::GetNumCollisionPrimitives( )
{
	if( SpeedTree && bIsInitialized )
	{
		return SpeedTree->GetNumCollisionObjects();
	}
	else
	{
		return 0;
	}
}

void FSpeedTreeResourceHelper::GetCollisionPrimitive( INT Index, CSpeedTreeRT::ECollisionObjectType& Type, FVector& Position, FVector& Dimensions, FVector& EulerAngles)
{
	if( SpeedTree && bIsInitialized && Index < SpeedTree->GetNumCollisionObjects() )
	{
		SpeedTree->GetCollisionObject( Index, Type, &Position.X, &Dimensions.X, &EulerAngles.X );
	}
	else
	{
		// no primitive, return nothing
		Position	= FVector(0,0,0);
		Dimensions	= FVector(0,0,0);
	}
}

#endif // WITH_SPEEDTREE


UBOOL USpeedTree::IsInitialized()
{ 
#if WITH_SPEEDTREE
	return SRH ? SRH->bIsInitialized : FALSE; 
#else
	return FALSE;
#endif
}


void USpeedTree::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
#if WITH_SPEEDTREE
	if( Ar.IsLoading() || Ar.IsSaving() )
	{
		// Create helper object used to allow USpeedTree to be script exposed.

		UINT	NumBytes		= 0;
		BYTE*	SpeedTreeData	= NULL;

		if(Ar.IsLoading() && !SRH)
		{
			SRH = new FSpeedTreeResourceHelper(this);
		}

		if(Ar.IsSaving() && SRH && SRH->SpeedTree)
		{
			SpeedTreeData = SRH->SpeedTree->SaveTree(NumBytes);
		}
				
		Ar << NumBytes;

		if( Ar.IsLoading() )
		{
			SpeedTreeData = (BYTE*) appMalloc( NumBytes );
		}

		if( NumBytes )
		{
			Ar.Serialize( SpeedTreeData, NumBytes );
		
			if( Ar.IsLoading() )
			{
				SRH->Load( SpeedTreeData, NumBytes );
			}
		}

		// Free data allocated to hold SPT file loaded from file.
		if( Ar.IsLoading() )
		{
		    appFree( SpeedTreeData );
	    }
		// Free data returned by SaveTree.
		else if( Ar.IsSaving() )
		{
			//@todo SpeedTree: route allocations through our allocator
			//@todo SpeedTree: free( SpeedTreeData );
		}
	}

	// Measure the memory allocated and used by the speedtree.
	if( Ar.IsCountingMemory() )
	{
		if ( SRH )
		{
			if ( SRH->SpeedTree )
			{
				Ar.CountBytes(SRH->SpeedTree->GetMemoryUsage(), SRH->SpeedTree->GetMemoryUsage() );
			}

			Ar.CountBytes( sizeof( CSpeedWind ),sizeof( CSpeedWind ) );

			SRH->BranchElements.CountBytes( Ar );
			SRH->FrondElements.CountBytes( Ar );
			SRH->LeafCardElements.CountBytes( Ar );
			SRH->LeafMeshElements.CountBytes( Ar );

			Ar.CountBytes( sizeof(FSpeedTreeResourceHelper ), sizeof( FSpeedTreeResourceHelper ) );
		}
	}

#else
	// Skip over the serialized data.
	INT NumBytes = 0;
	Ar << NumBytes;
	if(Ar.IsLoading())
	{
		Ar.Seek( Ar.Tell() + NumBytes );
	}
#endif
}

void USpeedTree::PreEditChange(UProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	// Ensure the rendering thread isn't accessing the object while it is being edited.
	FlushRenderingCommands();
}

void USpeedTree::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
#if WITH_SPEEDTREE
	if( PropertyThatChanged )
	{
		FString PropertyName = PropertyThatChanged->GetName();

		// only take action on these properties
		if( PropertyName == TEXT("RandomSeed") 
		||	PropertyName == TEXT("Sink")
		||	PropertyName == TEXT("BranchMaterial")
		||	PropertyName == TEXT("FrondMaterial")
		||	PropertyName == TEXT("LeafMaterial")
		||	PropertyName == TEXT("BillboardMaterial"))
		{
			if( SRH->SpeedTree != NULL )
			{
				// Detach all instances of this SpeedTree, to ensure there aren't any static mesh elements remaining that reference the soon to be freed resources.
				// They will be reattached when the array is destructed.
				TArray<FComponentReattachContext> PrimitiveReattachContexts;
				for(TObjectIterator<USpeedTreeComponent> PrimitiveIt;PrimitiveIt;++PrimitiveIt)
				{
					if(PrimitiveIt->SpeedTree == this)
					{
						new(PrimitiveReattachContexts) FComponentReattachContext(*PrimitiveIt);
					}
				}

				// Save the tree data.
				UINT	NumBytes		= 0;
				BYTE*	SpeedTreeData	= SRH->SpeedTree->SaveTree(NumBytes);

				// Cleanup and delete the resource helper object.
				SRH->CleanUp();
				SRH->ReleaseResourcesFence.Wait();
				delete SRH;

				// Reload the tree data.
				SRH = new FSpeedTreeResourceHelper(this);
				SRH->Load( SpeedTreeData, NumBytes );

				//@todo SpeedTree: route allocations through our allocator
				//@todo SpeedTree: appFree(SpeedTreeData);
			}
		}
	}
#endif
}

FString USpeedTree::GetDesc()
{
#if WITH_SPEEDTREE
	INT Triangles = 0;
	INT Verts = 0;

	if( SRH->bIsInitialized )
	{
		if( SRH->bHasBranches )
		{
			Triangles += SRH->BranchElements(0).NumPrimitives;
		}

		if( SRH->bHasFronds )
		{
			Triangles += SRH->FrondElements(0).NumPrimitives;	
			Verts += SRH->BranchFrondDataBuffer.Vertices.Num();
		}

		if( SRH->bHasLeaves )
		{
			Triangles += SRH->LeafCardElements(0).NumPrimitives + SRH->LeafMeshElements(0).NumPrimitives;
			Verts += SRH->BillboardDataBuffer.Vertices.Num( );
		}
	}

	return FString::Printf(TEXT("%d Triangles, %d Vertices"), Triangles, Verts);
#else
	return FString();
#endif
}

FString USpeedTree::GetDetailedDescription( INT InIndex )
{
#if WITH_SPEEDTREE
	INT Triangles = 0;
	INT Verts = 0;

	if( SRH->bIsInitialized )
	{
		if( SRH->bHasBranches )
		{
			Triangles += SRH->BranchElements(0).NumPrimitives;
		}

		if( SRH->bHasFronds )
		{
			Triangles += SRH->FrondElements(0).NumPrimitives;	
			Verts += SRH->BranchFrondDataBuffer.Vertices.Num();
		}

		if( SRH->bHasLeaves )
		{
			Triangles += SRH->LeafCardElements(0).NumPrimitives + SRH->LeafMeshElements(0).NumPrimitives;
			Verts += SRH->BillboardDataBuffer.Vertices.Num( );
		}
	}

	FString Description = TEXT("");

	switch(InIndex)
	{
	case 0:
		Description = FString::Printf(TEXT("%d Triangles"), Triangles);
		break;
	case 1: 
		Description = FString::Printf(TEXT("%d Vertices"), Verts);
		break;
	}

	return Description;
#else
	return FString();
#endif
}

INT USpeedTree::GetResourceSize()
{
	FArchiveCountMem CountBytesSize(this);
	const INT ResourceSize = CountBytesSize.GetNum();
	return ResourceSize;
}

#if WITH_SPEEDTREE
const FSpeedTreeVertexData* GetSpeedTreeVertexData(const FSpeedTreeResourceHelper* SRH,INT MeshType,INT VertexIndex)
{
	switch(MeshType)
	{
	case STMT_Branches:
	case STMT_Fronds:
		return &SRH->BranchFrondDataBuffer.Vertices(VertexIndex);
	case STMT_LeafMeshes:
		return &SRH->LeafMeshDataBuffer.Vertices(VertexIndex);
	case STMT_LeafCards:
		return &SRH->LeafCardDataBuffer.Vertices(VertexIndex);
	case STMT_Billboards:
		return &SRH->BillboardDataBuffer.Vertices(VertexIndex);
	default:
		appErrorf(TEXT("Unknown SpeedTree mesh type: %u"),MeshType);
		return NULL;
	};
}
#endif
