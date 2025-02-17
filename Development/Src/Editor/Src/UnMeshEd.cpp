/*=============================================================================
	UnMeshEd.cpp: Skeletal mesh import code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "Factories.h"
#include "SkelImport.h"
#include "EngineAnimClasses.h"

/*-----------------------------------------------------------------------------
	Special importers for skeletal data.
-----------------------------------------------------------------------------*/

//
//	USkeletalMeshFactory::StaticConstructor
//
void USkeletalMeshFactory::StaticConstructor()
{
	new(GetClass(), TEXT("bAssumeMayaCoordinates"), RF_Public) UBoolProperty(CPP_PROPERTY(bAssumeMayaCoordinates), TEXT(""), CPF_Edit);
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USkeletalMeshFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = USkeletalMesh::StaticClass();
	new(Formats)FString(TEXT("psk;Skeletal Mesh"));
	bCreateNew = 0;
}
//
//	USkeletalMeshFactory::USkeletalMeshFactory
//
USkeletalMeshFactory::USkeletalMeshFactory(FSkelMeshOptionalImportData* InOptionalImportData)
{
	OptionalImportData		= InOptionalImportData;
	bEditorImport			= 1;
	bAssumeMayaCoordinates	= false;
}

IMPLEMENT_COMPARE_CONSTREF( VRawBoneInfluence, UnMeshEd, 
{
	if		( A.VertexIndex > B.VertexIndex	) return  1;
	else if ( A.VertexIndex < B.VertexIndex	) return -1;
	else if ( A.Weight      < B.Weight		) return  1;
	else if ( A.Weight      > B.Weight		) return -1;
	else if ( A.BoneIndex   > B.BoneIndex	) return  1;
	else if ( A.BoneIndex   < B.BoneIndex	) return -1;
	else									  return  0;	
}
)

static INT FindBoneIndex( const TArray<FMeshBone>& RefSkeleton, FName BoneName )
{
	for(INT i=0; i<RefSkeleton.Num(); i++)
	{
		if(RefSkeleton(i).Name == BoneName)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

/** Check that root bone is the same, and that any bones that are common have the correct parent. */
static UBOOL SkeletonsAreCompatible( const TArray<FMeshBone>& NewSkel, const TArray<FMeshBone>& ExistSkel )
{
	if(NewSkel(0).Name != ExistSkel(0).Name)
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("MeshHasDifferentRoot"), *NewSkel(0).Name.ToString(), *ExistSkel(0).Name.ToString()) );
		return false;
	}

	for(INT i=1; i<NewSkel.Num(); i++)
	{
		// See if bone is in both skeletons.
		INT NewBoneIndex = i;
		FName NewBoneName = NewSkel(NewBoneIndex).Name;
		INT BBoneIndex = FindBoneIndex(ExistSkel, NewBoneName);

		// If it is, check parents are the same.
		if(BBoneIndex != INDEX_NONE)
		{
			FName NewParentName = NewSkel( NewSkel(NewBoneIndex).ParentIndex ).Name;
			FName ExistParentName = ExistSkel( ExistSkel(BBoneIndex).ParentIndex ).Name;

			if(NewParentName != ExistParentName)
			{
				appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("MeshHasDifferentRoot"), *NewBoneName.ToString(), *NewParentName.ToString(), *ExistParentName.ToString()) );
				return false;
			}
		}
	}

	return true;
}

/**
* Takes an imported bone name, removes any leading or trailing spaces, and converts the remaining spaces to dashes.
*/
FString FSkeletalMeshBinaryImport::FixupBoneName( ANSICHAR *AnisBoneName )
{
	FString BoneName = AnisBoneName;

	BoneName.Trim();
	BoneName.TrimTrailing();
	BoneName = BoneName.Replace( TEXT( " " ), TEXT( "-" ) );
	
	return( BoneName );
}

/**
* Helper function for filling in the various FSkeletalMeshBinaryImport values from buffer of data
* 
* @param DestArray - destination array of data to allocate and copy to
* @param BufferReadPtr - source data buffer to read from
* @param BufferEnd - end of data buffer
*/
template<typename DataType>
static void CopyMeshDataAndAdvance( TArray<DataType>& DestArray, BYTE*& BufferReadPtr, const BYTE* BufferEnd )
{
	// assume that BufferReadPtr is positioned at next chunk header 
	const VChunkHeader* ChunkHeader = (const VChunkHeader*) BufferReadPtr;
	// advance buffer ptr to data block
	BufferReadPtr += sizeof(VChunkHeader);	
	// make sure we don't overrun the buffer when reading data
	check((sizeof(DataType) * ChunkHeader->DataCount + BufferReadPtr) <= BufferEnd);
	// allocate space in import data
	DestArray.Add( ChunkHeader->DataCount );	
	// copy from buffer
	appMemcpy( &DestArray(0), BufferReadPtr, sizeof(DataType) * ChunkHeader->DataCount );
	// advance buffer
	BufferReadPtr  += sizeof(DataType) * ChunkHeader->DataCount;		
};

/**
* Parse skeletal mesh (psk file) from buffer into raw import data
* 
* @param BufferReadPtr	- start of data to be read
* @param BufferEnd - end of data to be read
* @param bShowSummary - if TRUE then print a summary of what was read to file
*/
void FSkeletalMeshBinaryImport::ImportFromFile( BYTE* BufferReadPtr, const BYTE* BufferEnd, UBOOL bShowSummary )
{
	check(BufferReadPtr);

	// Skip passed main dummy header. 
	BufferReadPtr  += sizeof(VChunkHeader);
  
	// Read the temp skin structures..
	// 3d points "vpoints" datasize*datacount....
	CopyMeshDataAndAdvance<FVector>( Points, BufferReadPtr, BufferEnd );

	//  Wedges (VVertex)	
	CopyMeshDataAndAdvance<VVertex>( Wedges, BufferReadPtr, BufferEnd );

	// Faces (VTriangle)
	CopyMeshDataAndAdvance<VTriangle>( Faces, BufferReadPtr, BufferEnd );

	// Materials (VMaterial)
	CopyMeshDataAndAdvance<VMaterial>( Materials, BufferReadPtr, BufferEnd );

	// Reference skeleton (VBones)
	CopyMeshDataAndAdvance<VBone>( RefBonesBinary, BufferReadPtr, BufferEnd );

	// Raw bone influences (VRawBoneInfluence)
	CopyMeshDataAndAdvance<VRawBoneInfluence>( Influences, BufferReadPtr, BufferEnd );

	// Y-flip quaternions and translations from Max/Maya/etc space into Unreal space.
	for( INT b=0; b<RefBonesBinary.Num(); b++)
	{
		FQuat Bone = RefBonesBinary(b).BonePos.Orientation;
		Bone.Y = - Bone.Y;
		// W inversion only needed on the parent - since PACKAGE_FILE_VERSION 133.
		// @todo - clean flip out of/into exporters
		if( b==0 ) 
		{
			Bone.W = - Bone.W; 
		}
		RefBonesBinary(b).BonePos.Orientation = Bone;

		FVector Pos = RefBonesBinary(b).BonePos.Position;
		Pos.Y = - Pos.Y;
		RefBonesBinary(b).BonePos.Position = Pos;
	}

	// Y-flip skin, and adjust handedness
	for( INT p=0; p<Points.Num(); p++ )
	{
		Points(p).Y *= -1;
	}
	for( INT f=0; f<Faces.Num(); f++)
	{
		Exchange( Faces(f).WedgeIndex[1], Faces(f).WedgeIndex[2] );
	}	

	// Necessary: Fixup face material index from wedge 0 as faces don't always have the proper material index (exporter's task).
	for( INT i=0; i<Faces.Num(); i++)
	{
		Faces(i).MatIndex		= Wedges( Faces(i).WedgeIndex[0] ).MatIndex;
		Faces(i).AuxMatIndex	= 0;
	}

	if( bShowSummary )
	{
		// display summary info
		debugf(NAME_Log,TEXT(" * Skeletal skin VPoints            : %i"),Points.Num()			);
		debugf(NAME_Log,TEXT(" * Skeletal skin VVertices          : %i"),Wedges.Num()			);
		debugf(NAME_Log,TEXT(" * Skeletal skin VTriangles         : %i"),Faces.Num()			);
		debugf(NAME_Log,TEXT(" * Skeletal skin VMaterials         : %i"),Materials.Num()		);
		debugf(NAME_Log,TEXT(" * Skeletal skin VBones             : %i"),RefBonesBinary.Num()	);
		debugf(NAME_Log,TEXT(" * Skeletal skin VRawBoneInfluences : %i"),Influences.Num()		);
	}
}

/**
* Copy mesh data for importing a single LOD
*
* @param LODPoints - vertex data.
* @param LODWedges - wedge information to static LOD level.
* @param LODFaces - triangle/ face data to static LOD level.
* @param LODInfluences - weights/ influences to static LOD level.
*/ 
void FSkeletalMeshBinaryImport::CopyLODImportData( 
					   TArray<FVector>& LODPoints, 
					   TArray<FMeshWedge>& LODWedges,
					   TArray<FMeshFace>& LODFaces,	
					   TArray<FVertInfluence>& LODInfluences )
{
	// Copy vertex data.
	LODPoints.Empty( Points.Num() );
	LODPoints.Add( Points.Num() );
	for( INT p=0; p < Points.Num(); p++ )
	{
		LODPoints(p) = Points(p);
	}

	// Copy wedge information to static LOD level.
	LODWedges.Empty( Wedges.Num() );
	LODWedges.Add( Wedges.Num() );
	for( INT w=0; w < Wedges.Num(); w++ )
	{
		LODWedges(w).iVertex	= Wedges(w).VertexIndex;
		LODWedges(w).U		= Wedges(w).U;
		LODWedges(w).V		= Wedges(w).V;
	}

	// Copy triangle/ face data to static LOD level.
	LODFaces.Empty( Faces.Num() );
	LODFaces.Add( Faces.Num() );
	for( INT f=0; f < Faces.Num(); f++)
	{
		FMeshFace Face;
		Face.iWedge[0]			= Faces(f).WedgeIndex[0];
		Face.iWedge[1]			= Faces(f).WedgeIndex[1];
		Face.iWedge[2]			= Faces(f).WedgeIndex[2];
		Face.MeshMaterialIndex	= Faces(f).MatIndex;

		LODFaces(f) = Face;
	}			

	// Copy weights/ influences to static LOD level.
	LODInfluences.Empty( Influences.Num() );
	LODInfluences.Add( Influences.Num() );
	for( INT i=0; i < Influences.Num(); i++ )
	{
		LODInfluences(i).Weight		= Influences(i).Weight;
		LODInfluences(i).VertIndex	= Influences(i).VertexIndex;
		LODInfluences(i).BoneIndex	= Influences(i).BoneIndex;
	}
}

/**
* Process and fill in the mesh Materials using the raw binary import data
* 
* @param Materials - [out] array of materials to update
* @param SkelMeshImporter - raw binary import data to process
*/
static void ProcessImportMeshMaterials(TArray<UMaterialInterface*>& Materials, FSkeletalMeshBinaryImport& SkelMeshImporter)
{
	TArray <VMaterial>&	MaterialsBinary = SkelMeshImporter.Materials;

	// If direct linkup of materials is requested, try to find them here - to get a texture name from a 
	// material name, cut off anything in front of the dot (beyond are special flags).
	Materials.Empty();
	for( INT m=0; m < MaterialsBinary.Num(); m++)
	{			
		TCHAR MaterialName[128];
		appStrcpy( MaterialName, ANSI_TO_TCHAR( MaterialsBinary(m).MaterialName ) );

		// Terminate string at the dot, or at any double underscore (Maya doesn't allow 
		// anything but underscores in a material name..) Beyond that, the materialname 
		// had tags that are now already interpreted by the exporter to go into flags
		// or order the materials for the .PSK refrence skeleton/skin output.
		TCHAR* TagsCutoff = appStrstr( MaterialName , TEXT(".") );
		if(  !TagsCutoff )
		{
			TagsCutoff = appStrstr( MaterialName, TEXT("__"));
		}
		if( TagsCutoff ) 
		{
			*TagsCutoff = 0; 
		}

		UMaterialInterface* Material = FindObject<UMaterialInterface>( ANY_PACKAGE, MaterialName );
		Materials.AddItem(Material);
		MaterialsBinary(m).TextureIndex = m; // Force 'skin' index to point to the exact named material.

		if( Material )
		{
			debugf(TEXT(" Found texture for material %i: [%s] skin index: %i "), m, *Material->GetName(), MaterialsBinary(m).TextureIndex );
		}
		else
		{
			debugf(TEXT(" Mesh material not found among currently loaded ones: %s"), MaterialName );
		}
	}

	// Pad the material pointers.
	while( MaterialsBinary.Num() > Materials.Num() )
	{
		Materials.AddItem( NULL );
	}
}

/**
* Process and fill in the mesh ref skeleton bone hierarchy using the raw binary import data
* 
* @param RefSkeleton - [out] reference skeleton hierarchy to update
* @param SkeletalDepth - [out] depth of the reference skeleton hierarchy
* @param SkelMeshImporter - raw binary import data to process
*/
static void ProcessImportMeshSkeleton(TArray<FMeshBone>& RefSkeleton, INT& SkeletalDepth, FSkeletalMeshBinaryImport& SkelMeshImporter)
{
	TArray <VBone>&	RefBonesBinary = SkelMeshImporter.RefBonesBinary;

	// Setup skeletal hierarchy + names structure.
	RefSkeleton.Empty( RefBonesBinary.Num() );
	RefSkeleton.AddZeroed( RefBonesBinary.Num() );

	// Digest bones to the serializable format.
	for( INT b=0; b<RefBonesBinary.Num(); b++ )
	{
		FMeshBone& Bone = RefSkeleton( b );
		VBone& BinaryBone = RefBonesBinary( b );

		Bone.Flags					= 0;
		Bone.BonePos.Position		= BinaryBone.BonePos.Position;     // FVector - Origin of bone relative to parent, or root-origin.
		Bone.BonePos.Orientation	= BinaryBone.BonePos.Orientation;  // FQuat - orientation of bone in parent's Trans.
		Bone.NumChildren			= BinaryBone.NumChildren;
		Bone.ParentIndex			= BinaryBone.ParentIndex;		
		Bone.BoneColor				= FColor(255,255,255);

		FString BoneName = FSkeletalMeshBinaryImport::FixupBoneName( BinaryBone.Name );
		Bone.Name = FName( *BoneName, FNAME_Add, TRUE );
	}

	// Add hierarchy index to each bone and detect max depth.
	SkeletalDepth = 0;
	for( INT b=0; b < RefSkeleton.Num(); b++ )
	{
		INT Parent	= RefSkeleton(b).ParentIndex;
		INT Depth	= 1.0f;

		RefSkeleton(b).Depth	= 1.0f;
		if( Parent != b )
		{
			Depth += RefSkeleton(Parent).Depth;
		}
		if( SkeletalDepth < Depth )
		{
			SkeletalDepth = Depth;
		}
		RefSkeleton(b).Depth = Depth;
	}
}

/**
* Process and update the vertex Influences using the raw binary import data
* 
* @param SkelMeshImporter - raw binary import data to process
*/
static void ProcessImportMeshInfluences(FSkeletalMeshBinaryImport& SkelMeshImporter)
{
	TArray <FVector>& Points = SkelMeshImporter.Points;
	TArray <VVertex>& Wedges = SkelMeshImporter.Wedges;
	TArray <VRawBoneInfluence>& Influences = SkelMeshImporter.Influences;

	// Sort influences by vertex index.
	Sort<USE_COMPARE_CONSTREF(VRawBoneInfluence,UnMeshEd)>( &Influences(0), Influences.Num() );

	// Remove more than allowed number of weights by removing least important influences (setting them to 0). 
	// Relies on influences sorted by vertex index and weight and the code actually removing the influences below.
	INT LastVertexIndex		= INDEX_NONE;
	INT InfluenceCount		= 0;
	for(  INT i=0; i<Influences.Num(); i++ )
	{		
		if( ( LastVertexIndex != Influences(i).VertexIndex ) )
		{
			InfluenceCount	= 0;
			LastVertexIndex	= Influences(i).VertexIndex;
		}

		InfluenceCount++;

		if( InfluenceCount > 4 || LastVertexIndex >= Points.Num() )
		{
			Influences(i).Weight = 0.f;
		}
	}

	// Remove influences below a certain threshold.
	INT RemovedInfluences	= 0;
	const FLOAT MINWEIGHT	= 0.01f; // 1%
	for( INT i=Influences.Num()-1; i>=0; i-- )
	{
		if( Influences(i).Weight < MINWEIGHT )
		{
			Influences.Remove(i);
			RemovedInfluences++;
		}
	}

	// Renormalize influence weights.
	INT	LastInfluenceCount	= 0;
	InfluenceCount			= 0;
	LastVertexIndex			= INDEX_NONE;
	FLOAT TotalWeight		= 0.f;
	for( INT i=0; i<Influences.Num(); i++ )
	{
		if( LastVertexIndex != Influences(i).VertexIndex )
		{
			LastInfluenceCount	= InfluenceCount;
			InfluenceCount		= 0;

			// Normalize the last set of influences.
			if( LastInfluenceCount && (TotalWeight != 1.0f) )
			{				
				FLOAT OneOverTotalWeight = 1.f / TotalWeight;
				for( int r=0; r<LastInfluenceCount; r++)
				{
					Influences(i-r-1).Weight *= OneOverTotalWeight;
				}
			}
			TotalWeight		= 0.f;				
			LastVertexIndex = Influences(i).VertexIndex;							
		}
		InfluenceCount++;
		TotalWeight	+= Influences(i).Weight;			
	}

	// Ensure that each vertex has at least one influence as e.g. CreateSkinningStream relies on it.
	// The below code relies on influences being sorted by vertex index.
	LastVertexIndex = -1;
	InfluenceCount	= 0;
	if( Influences.Num() == 0 )
	{
		// warn about no influences
		appMsgf( AMT_OK, *LocalizeUnrealEd("WarningNoSkelInfluences") );
		// add one for each wedge entry
		Influences.Add(Wedges.Num());
		for( INT WedgeIdx=0; WedgeIdx<Wedges.Num(); WedgeIdx++ )
		{	
			Influences(WedgeIdx).VertexIndex = WedgeIdx;
			Influences(WedgeIdx).BoneIndex = 0;
			Influences(WedgeIdx).Weight = 1.0f;
		}		
	}
	for( INT i=0; i<Influences.Num(); i++ )
	{
		INT CurrentVertexIndex = Influences(i).VertexIndex;

		if( LastVertexIndex != CurrentVertexIndex )
		{
			for( INT j=LastVertexIndex+1; j<CurrentVertexIndex; j++ )
			{
				// Add a 0-bone weight if none other present (known to happen with certain MAX skeletal setups).
				Influences.Insert(i,1);
				Influences(i).VertexIndex	= j;
				Influences(i).BoneIndex		= 0;
				Influences(i).Weight		= 1.f;
			}
			LastVertexIndex = CurrentVertexIndex;
		}
	}
}

UObject* USkeletalMeshFactory::FactoryCreateBinary
(
 UClass*			Class,
 UObject*			InParent,
 FName				Name,
 EObjectFlags		Flags,
 UObject*			Context,
 const TCHAR*		Type,
 const BYTE*&		Buffer,
 const BYTE*		BufferEnd,
 FFeedbackContext*	Warn
 )
{
	// Before importing the new skeletal mesh - see if it already exists, and if so copy any data out we don't want to lose.
	USkeletalMesh* ExistingSkelMesh = FindObject<USkeletalMesh>( InParent, *Name.ToString() );

	UBOOL								bRestoreExistingData = FALSE;
	FVector								ExistingOrigin;
	FRotator							ExistingRotOrigin;
	TArray<USkeletalMeshSocket*>		ExistingSockets;
	TIndirectArray<FStaticLODModel>		ExistingLODModels;
	TArray<FSkeletalMeshLODInfo>		ExistingLODInfo;
	TArray<FMeshBone>					ExistingRefSkeleton;
	TArray<UMaterialInterface*>			ExistingMaterials;
	UFaceFXAsset*						ExistingFaceFXAsset = NULL;
	UPhysicsAsset*						ExistingBoundsPreviewAsset = NULL;
	UBOOL								bExistingForceCPUSkinning = FALSE;
	TArray<FName>						ExistingPerPolyCollisionBones;
	TArray<FName>						ExistingAddToParentPerPolyCollisionBone;
	FGuid								ExistingGUID;

	TArray<FName>						ExistingClothBones;
	UBOOL								bExistingEnableClothBendConstraints = FALSE;
	UBOOL								bExistingEnableClothDamping = FALSE;
	UBOOL								bExistingUseClothCOMDamping = FALSE;
	FLOAT								ExistingClothStretchStiffness = 0.f;
	FLOAT								ExistingClothBendStiffness = 0.f;
	FLOAT								ExistingClothDensity = 0.f;
	FLOAT								ExistingClothThickness = 0.f;
	FLOAT								ExistingClothDamping = 0.f;
	INT									ExistingClothIterations = 0;
	FLOAT								ExistingClothFriction = 0.f;

	TArray<FName>						ExistingSoftBodyBones;
	TArray<FSoftBodySpecialBoneInfo>	ExistingSoftBodySpecialBones;
	FLOAT								ExistingSoftBodyVolumeStiffness = 0.f;
	FLOAT								ExistingSoftBodyStretchingStiffness = 0.f;
	FLOAT								ExistingSoftBodyDensity = 0.f;
	FLOAT								ExistingSoftBodyParticleRadius = 0.f;
	FLOAT								ExistingSoftBodyDamping = 0.f;
	INT									ExistingSoftBodySolverIterations = 0;
	FLOAT								ExistingSoftBodyFriction = 0.f;
	FLOAT								ExistingSoftBodyRelativeGridSpacing = 0.f;
	FLOAT								ExistingSoftBodySleepLinearVelocity = 0.f;
	UBOOL								bExistingEnableSoftBodySelfCollision = FALSE;
	FLOAT								ExistingSoftBodyAttachmentResponse = 0.f;
	FLOAT								ExistingSoftBodyCollisionResponse = 0.f;
	FLOAT								ExistingSoftBodyDetailLevel = 0.f;
	INT									ExistingSoftBodySubdivisionLevel = 0;
	UBOOL								bExistingSoftBodyIsoSurface = FALSE;
	UBOOL								bExistingEnableSoftBodyDamping= FALSE;
	UBOOL								bExistingUseSoftBodyCOMDamping= FALSE;
	FLOAT								ExistingSoftBodyAttachmentThreshold = 0.f;
	UBOOL								bExistingEnableSoftBodyTwoWayCollision = FALSE;
	FLOAT								ExistingSoftBodyAttachmentTearFactor = 0.f;
	UBOOL								bExistingEnableSoftBodyLineChecks = FALSE;

	TArray<FBoneMirrorExport>			ExistingMirrorTable;

	if(ExistingSkelMesh)
	{
		// Free any RHI resources for existing mesh before we re-create in place.
		ExistingSkelMesh->PreEditChange(NULL);

		ExistingSockets = ExistingSkelMesh->Sockets;
		ExistingMaterials = ExistingSkelMesh->Materials;

		ExistingOrigin = ExistingSkelMesh->Origin;
		ExistingRotOrigin = ExistingSkelMesh->RotOrigin;

		if( ExistingSkelMesh->LODModels.Num() > 0 &&
			ExistingSkelMesh->LODInfo.Num() == ExistingSkelMesh->LODModels.Num() )
		{
			// Remove the zero'th LOD (ie: the LOD being reimported).
			ExistingSkelMesh->LODModels.Remove(0);
			ExistingSkelMesh->LODInfo.Remove(0);

			// Copy off the reamining LODs.
			for ( INT LODModelIndex = 0 ; LODModelIndex < ExistingSkelMesh->LODModels.Num() ; ++LODModelIndex )
			{
				FStaticLODModel& LODModel = ExistingSkelMesh->LODModels(LODModelIndex);
				LODModel.RawPointIndices.Lock( LOCK_READ_ONLY );
			}
			ExistingLODModels = ExistingSkelMesh->LODModels;
			for ( INT LODModelIndex = 0 ; LODModelIndex < ExistingSkelMesh->LODModels.Num() ; ++LODModelIndex )
			{
				FStaticLODModel& LODModel = ExistingSkelMesh->LODModels(LODModelIndex);
				LODModel.RawPointIndices.Unlock();
			}

			ExistingLODInfo = ExistingSkelMesh->LODInfo;
			ExistingRefSkeleton = ExistingSkelMesh->RefSkeleton;
		}

		ExistingFaceFXAsset = ExistingSkelMesh->FaceFXAsset;
		ExistingSkelMesh->FaceFXAsset = NULL;
		if( ExistingFaceFXAsset )
		{
			ExistingFaceFXAsset->DefaultSkelMesh = NULL;
#if WITH_FACEFX
			OC3Ent::Face::FxActor* FaceFXActor = ExistingFaceFXAsset->GetFxActor();
			if( FaceFXActor )
			{
				FaceFXActor->SetShouldClientRelink(FxTrue);
			}
#endif // WITH_FACEFX
		}
		
		ExistingBoundsPreviewAsset = ExistingSkelMesh->BoundsPreviewAsset;
		bExistingForceCPUSkinning = ExistingSkelMesh->bForceCPUSkinning;
		ExistingPerPolyCollisionBones = ExistingSkelMesh->PerPolyCollisionBones;
		ExistingAddToParentPerPolyCollisionBone = ExistingSkelMesh->AddToParentPerPolyCollisionBone;

		ExistingGUID = ExistingSkelMesh->SkelMeshGUID;

		ExistingClothBones = ExistingSkelMesh->ClothBones;
		bExistingEnableClothBendConstraints = ExistingSkelMesh->bEnableClothBendConstraints;
		bExistingEnableClothDamping = ExistingSkelMesh->bEnableClothDamping;
		bExistingUseClothCOMDamping = ExistingSkelMesh->bUseClothCOMDamping;
		ExistingClothStretchStiffness = ExistingSkelMesh->ClothStretchStiffness;
		ExistingClothBendStiffness = ExistingSkelMesh->ClothBendStiffness;
		ExistingClothDensity = ExistingSkelMesh->ClothDensity;
		ExistingClothThickness = ExistingSkelMesh->ClothThickness;
		ExistingClothDamping = ExistingSkelMesh->ClothDamping;
		ExistingClothIterations = ExistingSkelMesh->ClothIterations;
		ExistingClothFriction = ExistingSkelMesh->ClothFriction;

		ExistingSoftBodyBones = ExistingSkelMesh->SoftBodyBones;
		ExistingSoftBodySpecialBones = ExistingSkelMesh->SoftBodySpecialBones;
		ExistingSoftBodyVolumeStiffness = ExistingSkelMesh->SoftBodyVolumeStiffness;
		ExistingSoftBodyStretchingStiffness = ExistingSkelMesh->SoftBodyStretchingStiffness;
		ExistingSoftBodyDensity = ExistingSkelMesh->SoftBodyDensity;
		ExistingSoftBodyParticleRadius = ExistingSkelMesh->SoftBodyParticleRadius;
		ExistingSoftBodyDamping = ExistingSkelMesh->SoftBodyDamping;
		ExistingSoftBodySolverIterations = ExistingSkelMesh->SoftBodySolverIterations;
		ExistingSoftBodyFriction = ExistingSkelMesh->SoftBodyFriction;
		ExistingSoftBodyRelativeGridSpacing = ExistingSkelMesh->SoftBodyRelativeGridSpacing;
		ExistingSoftBodySleepLinearVelocity = ExistingSkelMesh->SoftBodySleepLinearVelocity;
		bExistingEnableSoftBodySelfCollision = ExistingSkelMesh->bEnableSoftBodySelfCollision;
		ExistingSoftBodyAttachmentResponse = ExistingSkelMesh->SoftBodyAttachmentResponse;
		ExistingSoftBodyCollisionResponse = ExistingSkelMesh->SoftBodyCollisionResponse;
		ExistingSoftBodyDetailLevel = ExistingSkelMesh->SoftBodyDetailLevel;
		ExistingSoftBodySubdivisionLevel = ExistingSkelMesh->SoftBodySubdivisionLevel;
		bExistingSoftBodyIsoSurface = ExistingSkelMesh->bSoftBodyIsoSurface;
		bExistingEnableSoftBodyDamping = ExistingSkelMesh->bEnableSoftBodyDamping;
		bExistingUseSoftBodyCOMDamping = ExistingSkelMesh->bUseSoftBodyCOMDamping;
		ExistingSoftBodyAttachmentThreshold = ExistingSkelMesh->SoftBodyAttachmentThreshold;
		bExistingEnableSoftBodyTwoWayCollision = ExistingSkelMesh->bEnableSoftBodyTwoWayCollision;
		ExistingSoftBodyAttachmentTearFactor = ExistingSkelMesh->SoftBodyAttachmentTearFactor;
		bExistingEnableSoftBodyLineChecks = ExistingSkelMesh->bEnableSoftBodyLineChecks;

		ExistingSkelMesh->ExportMirrorTable(ExistingMirrorTable);
		bRestoreExistingData = TRUE;
	}

	// Create 'empty' mesh.
	USkeletalMesh* SkeletalMesh = CastChecked<USkeletalMesh>( StaticConstructObject( Class, InParent, Name, Flags ) );
	
	SkeletalMesh->PreEditChange(NULL);

	if( bAssumeMayaCoordinates )
	{
		SkeletalMesh->RotOrigin = FRotator(0, -16384, 16384);		
	}

	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("ImportingSkeletalMeshFile")), TRUE );

	// Fill with data from buffer - contains the full .PSK file. 	
	FSkeletalMeshBinaryImport SkelMeshImporter;
	SkelMeshImporter.ImportFromFile( (BYTE*)Buffer, BufferEnd );

	// process materials from import data
	ProcessImportMeshMaterials(SkeletalMesh->Materials,SkelMeshImporter);

	// process reference skeleton from import data
	ProcessImportMeshSkeleton(SkeletalMesh->RefSkeleton,SkeletalMesh->SkeletalDepth,SkelMeshImporter);
	debugf( TEXT("Bones digested - %i  Depth of hierarchy - %i"), SkeletalMesh->RefSkeleton.Num(), SkeletalMesh->SkeletalDepth );

	// Build map between bone name and bone index now.
	SkeletalMesh->InitNameIndexMap();

	// process bone influences from import data
	ProcessImportMeshInfluences(SkelMeshImporter);

	check(SkeletalMesh->LODModels.Num() == 0);
	SkeletalMesh->LODModels.Empty();
	new(SkeletalMesh->LODModels)FStaticLODModel();

	SkeletalMesh->LODInfo.Empty();
	SkeletalMesh->LODInfo.AddZeroed();
	SkeletalMesh->LODInfo(0).LODHysteresis = 0.02f;

	// Create initial bounding box based on expanded version of reference pose for meshes without physics assets. Can be overridden by artist.
	FBox BoundingBox( &SkelMeshImporter.Points(0), SkelMeshImporter.Points.Num() );
	FBox Temp = BoundingBox;
	FVector MidMesh		= 0.5f*(Temp.Min + Temp.Max);
	BoundingBox.Min		= Temp.Min + 1.0f*(Temp.Min - MidMesh);
    BoundingBox.Max		= Temp.Max + 1.0f*(Temp.Max - MidMesh);
	// Tuck up the bottom as this rarely extends lower than a reference pose's (e.g. having its feet on the floor).
	// Maya has Y in the vertical, other packages have Z.
	const INT CoordToTuck = bAssumeMayaCoordinates ? 1 : 2;
	BoundingBox.Min[CoordToTuck]	= Temp.Min[CoordToTuck] + 0.1f*(Temp.Min[CoordToTuck] - MidMesh[CoordToTuck]);
	SkeletalMesh->Bounds= FBoxSphereBounds(BoundingBox);

	// copy vertex data needed to generate skinning streams for LOD
	TArray<FVector> LODPoints;
	TArray<FMeshWedge> LODWedges;
	TArray<FMeshFace> LODFaces;
	TArray<FVertInfluence> LODInfluences;
	SkelMeshImporter.CopyLODImportData(LODPoints,LODWedges,LODFaces,LODInfluences);

	// process optional import data if available and use it when generating the skinning streams
	FSkelMeshExtraInfluenceImportData* ExtraInfluenceDataPtr = NULL;
	FSkelMeshExtraInfluenceImportData ExtraInfluenceData;
	if( OptionalImportData != NULL )
	{
		if( OptionalImportData->RawMeshInfluencesData.Wedges.Num() > 0 )
		{
			// process reference skeleton from import data
			INT TempSkelDepth=0;
			ProcessImportMeshSkeleton(ExtraInfluenceData.RefSkeleton,TempSkelDepth,OptionalImportData->RawMeshInfluencesData);
			// process bone influences from import data
			ProcessImportMeshInfluences(OptionalImportData->RawMeshInfluencesData);
			// copy vertex data needed for processing the extra vertex influences from import data
			OptionalImportData->RawMeshInfluencesData.CopyLODImportData(
				ExtraInfluenceData.Points,
				ExtraInfluenceData.Wedges,
				ExtraInfluenceData.Faces,
				ExtraInfluenceData.Influences
				);
			ExtraInfluenceDataPtr = &ExtraInfluenceData;
		}
	}

	// Create actual rendering data.
	SkeletalMesh->CreateSkinningStreams(LODInfluences,LODWedges,LODFaces,LODPoints,ExtraInfluenceDataPtr);

	// RequiredBones for base model includes all bones.
	FStaticLODModel& LODModel = SkeletalMesh->LODModels(0);
	LODModel.RequiredBones.Add(SkeletalMesh->RefSkeleton.Num());
	for(INT i=0; i<LODModel.RequiredBones.Num(); i++)
	{
		LODModel.RequiredBones(i) = i;
	}

	// Presize the per-section shadow casting array with the number of sections in the imported LOD.
	const INT NumSections = LODModel.Sections.Num();
	SkeletalMesh->LODInfo(0).bEnableShadowCasting.Empty( NumSections );
	for ( INT SectionIndex = 0 ; SectionIndex < NumSections ; ++SectionIndex )
	{
		SkeletalMesh->LODInfo(0).bEnableShadowCasting.AddItem( TRUE );
	}

	// Make GUID.
	SkeletalMesh->SkelMeshGUID = appCreateGuid();

	if(bRestoreExistingData)
	{
		// Fix Materials array to be the correct size.
		if(ExistingMaterials.Num() > SkeletalMesh->Materials.Num())
		{
			ExistingMaterials.Remove( SkeletalMesh->Materials.Num(), ExistingMaterials.Num() - SkeletalMesh->Materials.Num() );
		}
		else if(SkeletalMesh->Materials.Num() > ExistingMaterials.Num())
		{
			ExistingMaterials.AddZeroed( SkeletalMesh->Materials.Num() - ExistingMaterials.Num() );
		}

		SkeletalMesh->Materials = ExistingMaterials;

		SkeletalMesh->Origin = ExistingOrigin;
		SkeletalMesh->RotOrigin = ExistingRotOrigin;

		// Assign sockets from old version of this SkeletalMesh.
		// Only copy ones for bones that exist in the new mesh.
		for(INT i=0; i<ExistingSockets.Num(); i++)
		{
			const INT BoneIndex = SkeletalMesh->MatchRefBone( ExistingSockets(i)->BoneName );
			if(BoneIndex != INDEX_NONE)
			{
				SkeletalMesh->Sockets.AddItem( ExistingSockets(i) );
			}
		}

		// We copy back and fix-up the LODs that still work with this skeleton.
		if( ExistingLODModels.Num() > 0 && SkeletonsAreCompatible(SkeletalMesh->RefSkeleton, ExistingRefSkeleton) )
		{
			// First create mapping table from old skeleton to new skeleton.
			TArray<INT> OldToNewMap;
			OldToNewMap.Add(ExistingRefSkeleton.Num());
			for(INT i=0; i<ExistingRefSkeleton.Num(); i++)
			{
				OldToNewMap(i) = FindBoneIndex(SkeletalMesh->RefSkeleton, ExistingRefSkeleton(i).Name);
			}

			for(INT i=0; i<ExistingLODModels.Num(); i++)
			{
				FStaticLODModel& LODModel = ExistingLODModels(i);
				FSkeletalMeshLODInfo& LODInfo = ExistingLODInfo(i);

				// Fix ActiveBoneIndices array.
				UBOOL bMissingBone = false;
				FName MissingBoneName = NAME_None;
				for(INT j=0; j<LODModel.ActiveBoneIndices.Num() && !bMissingBone; j++)
				{
					INT NewBoneIndex = OldToNewMap( LODModel.ActiveBoneIndices(j) );
					if(NewBoneIndex == INDEX_NONE)
					{
						bMissingBone = true;
						MissingBoneName = ExistingRefSkeleton( LODModel.ActiveBoneIndices(j) ).Name;
					}
					else
					{
						LODModel.ActiveBoneIndices(j) = NewBoneIndex;
					}
				}

				// Fix RequiredBones array.
				for(INT j=0; j<LODModel.RequiredBones.Num() && !bMissingBone; j++)
				{
					INT NewBoneIndex = OldToNewMap( LODModel.RequiredBones(j) );
					if(NewBoneIndex == INDEX_NONE)
					{
						bMissingBone = true;
						MissingBoneName = ExistingRefSkeleton( LODModel.RequiredBones(j) ).Name;
					}
					else
					{
						LODModel.RequiredBones(j) = NewBoneIndex;
					}
				}

				// Fix the chunks' BoneMaps.
				for(INT ChunkIndex = 0;ChunkIndex < LODModel.Chunks.Num();ChunkIndex++)
				{
					FSkelMeshChunk& Chunk = LODModel.Chunks(ChunkIndex);
					for(INT BoneIndex = 0;BoneIndex < Chunk.BoneMap.Num();BoneIndex++)
					{
						INT NewBoneIndex = OldToNewMap( Chunk.BoneMap(BoneIndex) );
						if(NewBoneIndex == INDEX_NONE)
						{
							bMissingBone = true;
							MissingBoneName = ExistingRefSkeleton(Chunk.BoneMap(BoneIndex)).Name;
							break;
						}
						else
						{
							Chunk.BoneMap(BoneIndex) = NewBoneIndex;
						}
					}
					if(bMissingBone)
					{
						break;
					}
				}

				if(bMissingBone)
				{
					appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("NewMeshMissingBoneFromLOD"), *MissingBoneName.ToString()) );
				}
				else
				{
					// Assert that the per-section shadow casting array matches the number of sections.
					check( LODInfo.bEnableShadowCasting.Num() == LODModel.Sections.Num() );

					new(SkeletalMesh->LODModels) FStaticLODModel( LODModel );
					SkeletalMesh->LODInfo.AddItem( LODInfo );
				}
			}
		}

		SkeletalMesh->FaceFXAsset = ExistingFaceFXAsset;
		if( ExistingFaceFXAsset )
		{
			ExistingFaceFXAsset->DefaultSkelMesh = SkeletalMesh;
			ExistingFaceFXAsset->MarkPackageDirty();
#if WITH_FACEFX
			OC3Ent::Face::FxActor* FaceFXActor = ExistingFaceFXAsset->GetFxActor();
			if( FaceFXActor )
			{
				FaceFXActor->SetShouldClientRelink(FxTrue);
			}
#endif // WITH_FACEFX
		}

		SkeletalMesh->BoundsPreviewAsset = ExistingBoundsPreviewAsset;
		SkeletalMesh->bForceCPUSkinning = bExistingForceCPUSkinning;
		SkeletalMesh->PerPolyCollisionBones = ExistingPerPolyCollisionBones;
		SkeletalMesh->AddToParentPerPolyCollisionBone = ExistingAddToParentPerPolyCollisionBone;
		SkeletalMesh->SkelMeshGUID = ExistingGUID;
			
		SkeletalMesh->ClothBones = ExistingClothBones;
		SkeletalMesh->bEnableClothBendConstraints = bExistingEnableClothBendConstraints;
		SkeletalMesh->bEnableClothDamping = bExistingEnableClothDamping;
		SkeletalMesh->bUseClothCOMDamping = bExistingUseClothCOMDamping;
		SkeletalMesh->ClothStretchStiffness = ExistingClothStretchStiffness;
		SkeletalMesh->ClothBendStiffness = ExistingClothBendStiffness;
		SkeletalMesh->ClothDensity = ExistingClothDensity;
		SkeletalMesh->ClothThickness = ExistingClothThickness;
		SkeletalMesh->ClothDamping = ExistingClothDamping;
		SkeletalMesh->ClothIterations = ExistingClothIterations;
		SkeletalMesh->ClothFriction = ExistingClothFriction;

		SkeletalMesh->SoftBodyBones = ExistingSoftBodyBones;
		SkeletalMesh->SoftBodySpecialBones = ExistingSoftBodySpecialBones;
		SkeletalMesh->SoftBodyVolumeStiffness = ExistingSoftBodyVolumeStiffness;
		SkeletalMesh->SoftBodyStretchingStiffness = ExistingSoftBodyStretchingStiffness;
		SkeletalMesh->SoftBodyDensity = ExistingSoftBodyDensity;
		SkeletalMesh->SoftBodyParticleRadius = ExistingSoftBodyParticleRadius;
		SkeletalMesh->SoftBodyDamping = ExistingSoftBodyDamping ;
		SkeletalMesh->SoftBodySolverIterations = ExistingSoftBodySolverIterations;
		SkeletalMesh->SoftBodyFriction = ExistingSoftBodyFriction;
		SkeletalMesh->SoftBodyRelativeGridSpacing = ExistingSoftBodyRelativeGridSpacing;
		SkeletalMesh->SoftBodySleepLinearVelocity = ExistingSoftBodySleepLinearVelocity;
		SkeletalMesh->bEnableSoftBodySelfCollision = bExistingEnableSoftBodySelfCollision;
		SkeletalMesh->SoftBodyAttachmentResponse = ExistingSoftBodyAttachmentResponse;
		SkeletalMesh->SoftBodyCollisionResponse = ExistingSoftBodyCollisionResponse;
		SkeletalMesh->SoftBodyDetailLevel = ExistingSoftBodyDetailLevel;
		SkeletalMesh->SoftBodySubdivisionLevel = ExistingSoftBodySubdivisionLevel ;
		SkeletalMesh->bSoftBodyIsoSurface = bExistingSoftBodyIsoSurface;
		SkeletalMesh->bEnableSoftBodyDamping = bExistingEnableSoftBodyDamping;
		SkeletalMesh->bUseSoftBodyCOMDamping = bExistingUseSoftBodyCOMDamping;
		SkeletalMesh->SoftBodyAttachmentThreshold = ExistingSoftBodyAttachmentThreshold;
		SkeletalMesh->bEnableSoftBodyTwoWayCollision = bExistingEnableSoftBodyTwoWayCollision;
		SkeletalMesh->SoftBodyAttachmentTearFactor = ExistingSoftBodyAttachmentTearFactor;
		SkeletalMesh->bEnableSoftBodyLineChecks = bExistingEnableSoftBodyLineChecks;

		// Copy mirror table.
		SkeletalMesh->ImportMirrorTable(ExistingMirrorTable);
	}

	// End the importing, mark package as dirty so it prompts to save on exit.
	SkeletalMesh->CalculateInvRefMatrices();
	SkeletalMesh->PostEditChange(NULL);

	SkeletalMesh->MarkPackageDirty();

	// We have to go and fix any AnimSetMeshLinkup objects that refer to this skeletal mesh, as the reference skeleton has changed.
	for(TObjectIterator<UAnimSet> It;It;++It)
	{
		UAnimSet* AnimSet = *It;
		for(INT i=0; i<AnimSet->LinkupCache.Num(); i++)
		{
			FAnimSetMeshLinkup& Linkup = AnimSet->LinkupCache(i);
			if( Linkup.SkelMeshLinkupGUID == SkeletalMesh->SkelMeshGUID )
			{
				Linkup.BuildLinkup(SkeletalMesh, AnimSet);
			}
		}
	}

	// Now iterate over all skeletal mesh components re-initialising them.
	for(TObjectIterator<USkeletalMeshComponent> It; It; ++It)
	{
		USkeletalMeshComponent* SkelComp = *It;
		if(SkelComp->SkeletalMesh == SkeletalMesh && SkelComp->GetScene())
		{
			FComponentReattachContext ReattachContext(SkelComp);
		}
	}

	GWarn->EndSlowTask();
	return SkeletalMesh;
}

IMPLEMENT_CLASS(USkeletalMeshFactory);


