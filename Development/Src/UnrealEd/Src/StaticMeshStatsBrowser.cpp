/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineAnimClasses.h"
#include "StaticMeshStatsBrowser.h"
#include "ScopedTransaction.h"
#include "SpeedTree.h"
#include "UnTerrain.h"
#include "EngineDecalClasses.h"
#include "UnDecalRenderData.h"
#include "EngineMeshClasses.h"

/** Whether the stats should be dumped to CSV during the next update. */
UBOOL GDumpPrimitiveStatsToCSVDuringNextUpdate;

BEGIN_EVENT_TABLE(WxPrimitiveStatsBrowser,WxBrowser)
	EVT_SIZE(WxPrimitiveStatsBrowser::OnSize)
    EVT_LIST_COL_CLICK(ID_PRIMITIVESTATSBROWSER_LISTCONTROL, WxPrimitiveStatsBrowser::OnColumnClick)
	EVT_LIST_COL_RIGHT_CLICK(ID_PRIMITIVESTATSBROWSER_LISTCONTROL, WxPrimitiveStatsBrowser::OnColumnRightClick)
    EVT_LIST_ITEM_ACTIVATED(ID_PRIMITIVESTATSBROWSER_LISTCONTROL, WxPrimitiveStatsBrowser::OnItemActivated)
END_EVENT_TABLE()

/** Current sort order (-1 or 1) */
INT WxPrimitiveStatsBrowser::CurrentSortOrder[PCSBC_MAX] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
/** Primary index/ column to sort by */
INT WxPrimitiveStatsBrowser::PrimarySortIndex = PCSBC_ObjLightCost;
/** Secondary index/ column to sort by */
INT WxPrimitiveStatsBrowser::SecondarySortIndex = PCSBC_ResourceSize;

/**
 * Inserts a column into the control.
 *
 * @param	ColumnId		Id of the column to insert
 * @param	ColumnHeader	Header/ description of the column.
 */
void WxPrimitiveStatsBrowser::InsertColumn( EPrimitiveStatsBrowserColumns ColumnId, const TCHAR* ColumnHeader, int Format )
{
	ListControl->InsertColumn( ColumnId, ColumnHeader, Format );
	new(ColumnHeaders) FString(ColumnHeader);
}

/**
 * Forwards the call to our base class to create the window relationship.
 * Creates any internally used windows after that
 *
 * @param DockID the unique id to associate with this dockable window
 * @param FriendlyName the friendly name to assign to this window
 * @param Parent the parent of this window (should be a Notebook)
 */
void WxPrimitiveStatsBrowser::Create(INT DockID,const TCHAR* FriendlyName, wxWindow* Parent)
{
	// Let our base class start up the windows
	WxBrowser::Create(DockID,FriendlyName,Parent);	

	// Add a menu bar
	MenuBar = new wxMenuBar();
	// Append the docking menu choices
	WxBrowser::AddDockingMenu(MenuBar);
	
	// Create list control
	ListControl = new WxListView( this, ID_PRIMITIVESTATSBROWSER_LISTCONTROL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES );

	// Insert columns.
	InsertColumn( PCSBC_Type,			*LocalizeUnrealEd("PrimitiveStatsBrowser_Type")									);
	InsertColumn( PCSBC_Name,			*LocalizeUnrealEd("PrimitiveStatsBrowser_Name")									);
	InsertColumn( PCSBC_Count,			*LocalizeUnrealEd("PrimitiveStatsBrowser_Count"), wxLIST_FORMAT_RIGHT			);
	InsertColumn( PCSBC_Triangles,		*LocalizeUnrealEd("PrimitiveStatsBrowser_Triangles"), wxLIST_FORMAT_RIGHT		);
	InsertColumn( PCSBC_InstTriangles,	*LocalizeUnrealEd("PrimitiveStatsBrowser_InstTriangles"), wxLIST_FORMAT_RIGHT	);
	InsertColumn( PCSBC_ResourceSize,	*LocalizeUnrealEd("PrimitiveStatsBrowser_ResourceSize"), wxLIST_FORMAT_RIGHT	);
	InsertColumn( PCSBC_LightsLM,		*LocalizeUnrealEd("PrimitiveStatsBrowser_LightsLM"), wxLIST_FORMAT_RIGHT		);
	InsertColumn( PCSBC_LightsOther,	*LocalizeUnrealEd("PrimitiveStatsBrowser_LightsOther"), wxLIST_FORMAT_RIGHT		);
	InsertColumn( PCSBC_LightsTotal,	*LocalizeUnrealEd("PrimitiveStatsBrowser_LightsTotal"), wxLIST_FORMAT_RIGHT		);
	InsertColumn( PCSBC_ObjLightCost,	*LocalizeUnrealEd("PrimitiveStatsBrowser_ObjLightCost"), wxLIST_FORMAT_RIGHT	);
	InsertColumn( PCSBC_LightMapData,	*LocalizeUnrealEd("PrimitiveStatsBrowser_LightMapData"), wxLIST_FORMAT_RIGHT	);
	InsertColumn( PCSBC_ShadowMapData,	*LocalizeUnrealEd("PrimitiveStatsBrowser_ShadowMapData"), wxLIST_FORMAT_RIGHT	);
	InsertColumn( PCSBC_LMSMResolution,	*LocalizeUnrealEd("PrimitiveStatsBrowser_LMSMResolution"), wxLIST_FORMAT_RIGHT	);
	InsertColumn( PCSBC_Sections,		*LocalizeUnrealEd("PrimitiveStatsBrowser_Sections"), wxLIST_FORMAT_RIGHT		);
	InsertColumn( PCSBC_RadiusMin,		*LocalizeUnrealEd("PrimitiveStatsBrowser_RadiusMin"), wxLIST_FORMAT_RIGHT		);
	InsertColumn( PCSBC_RadiusMax,		*LocalizeUnrealEd("PrimitiveStatsBrowser_RadiusMax"), wxLIST_FORMAT_RIGHT		);
	InsertColumn( PCSBC_RadiusAvg,		*LocalizeUnrealEd("PrimitiveStatsBrowser_RadiusAvg"), wxLIST_FORMAT_RIGHT		);
}

/**
 * Called when the browser is getting activated (becoming the visible
 * window in it's dockable frame).
 */
void WxPrimitiveStatsBrowser::Activated()
{
	// Let the super class do it's thing
	WxBrowser::Activated();
	Update();
}

/**
* Formats an integer value into a human readable string (i.e. 12345 becomes "12,345")
*
* @param	Val		The value to use
* @return	FString	The human readable string
*/

FString FFormatIntToHumanReadable( int Val )
{
	FString src = *FString::Printf( TEXT("%i"), Val );
	FString dst;

	if( Val > 999 )
	{
		dst = FString::Printf( TEXT(",%s"), *src.Mid( src.Len() - 3, 3 ) );
		src = src.Left( src.Len() - 3 );

	}

	if( Val > 999999 )
	{
		dst = FString::Printf( TEXT(",%s%s"), *src.Mid( src.Len() - 3, 3 ), *dst );
		src = src.Left( src.Len() - 3 );
	}

	dst = src + dst;

	return dst;
}

/**
 * Helper structure containing per static mesh stats.
 */
struct FPrimitiveStats
{
	/** Resource (e.g. UStaticMesh, USkeletalMesh, UModelComponent, UTerrainComponent, UDecalComponent, etc */
	UObject*		Resource;
	/** Number of occurances in map */
	UINT			Count;
	/** Triangle count of mesh */
	UINT			Triangles;
	/** Resource size in bytes */
	UINT			ResourceSize;
	/** Section count of mesh */
	UINT			Sections;
	/** Minimum radius of bounding sphere of instance in map */
	FLOAT			RadiusMin;
	/** Maximum radius of bounding sphere of instance in map */
	FLOAT			RadiusMax;
	/** Average radius of bounding sphere of instance in map */
	FLOAT			RadiusAvg;
	/** Average number of lightmap lights relevant to each instance */
	UINT			LightsLM;
	/** Average number of other lights relevant to each instance */
	UINT			LightsOther;
	/** Light map data in bytes */
	UINT			LightMapData;
	/** Shadow map data in bytes */
	UINT			ShadowMapData;
	/** Light/ shadow map resolution */
	UINT			LMSMResolution;

	/**
	* Returns a string representation of the selected column.  The returned string
	* is in human readable form (i.e. 12345 becomes "12,345")
	*
	* @param	Index	Column to retrieve float representation of - cannot be 0!
	* @return	FString	The human readable representation
	*/
	FString GetColumnDataString( UINT Index ) const
	{
		INT val = GetColumnData( Index );
		return FFormatIntToHumanReadable( val );
	}

	/**
	 * Returns a float representation of the selected column. Doesn't work for the first
	 * column as it is text. This code is slow but it's not performance critical.
	 *
	 * @param	Index	Column to retrieve float representation of - cannot be 0!
	 * @return	float	representation of column
	 */
	FLOAT GetColumnData( UINT Index ) const
	{
		check(Index>0);
		switch( Index )
		{
		case PCSBC_Type:
		case PCSBC_Name:
		default:
			appErrorf(TEXT("Unhandled case"));
			break;
		case PCSBC_Count:
			return Count;
			break;
		case PCSBC_Triangles:
			return Triangles;
			break;
		case PCSBC_InstTriangles:
			return Count * Triangles;
			break;
		case PCSBC_ResourceSize:
			return ResourceSize / 1024.f;
			break;
		case PCSBC_LightsLM:
			return (FLOAT) LightsLM / Count;
			break;
		case PCSBC_LightsOther:
			return (FLOAT) LightsOther / Count;
			break;
		case PCSBC_LightsTotal:
			return (FLOAT) (LightsOther + LightsLM) / Count;
			break;
		case PCSBC_ObjLightCost:
			return LightsOther * Sections;
			break;
		case PCSBC_LightMapData:
			return LightMapData / 1024.f;
			break;
		case PCSBC_ShadowMapData:
			return ShadowMapData / 1024.f;
			break;
		case PCSBC_LMSMResolution:
			return (FLOAT) LMSMResolution / Count;
			break;
		case PCSBC_Sections:
			return Sections;
			break;
		case PCSBC_RadiusMin:
			return RadiusMin;
			break;
		case PCSBC_RadiusMax:
			return RadiusMax;
			break;
		case PCSBC_RadiusAvg:
			return (FLOAT) RadiusAvg / Count;
			break;
		}
		return 0; // Can't get here.
	}

	/**
	 * Compare helper function used by the Compare used by Sort function.
	 *
	 * @param	Other		Other object to compare against
	 * @param	SortIndex	Index to compare
	 * @return	CurrentSortOrder if >, -CurrentSortOrder if < and 0 if ==
	 */
	INT Compare( const FPrimitiveStats& Other, INT SortIndex ) const
	{
		INT SortOrder = WxPrimitiveStatsBrowser::CurrentSortOrder[SortIndex];
		check( SortOrder != 0 );

		if( SortIndex == 0 )
		{
			// Same resource type
			if( Resource->GetClass() == Other.Resource->GetClass() )
			{
				return 0;
			}
			else if( Resource->GetClass()->GetName() > Other.Resource->GetClass()->GetName() )
			{
				return SortOrder;
			}
			else
			{
				return -SortOrder;
			}
		}
		else if( SortIndex == 1 )
		{			
			// This is going to be SLOW. At least we can assume that there are no duplicate static meshes.
			if( Resource->GetPathName() > Other.Resource->GetPathName() )
			{
				return SortOrder;
			}
			else
			{
				return -SortOrder;
			}
		}
		else
		{
			FLOAT SortKeyA = GetColumnData(SortIndex);
			FLOAT SortKeyB = Other.GetColumnData(SortIndex);
			if( SortKeyA > SortKeyB )
			{
				return SortOrder;
			}
			else if( SortKeyA < SortKeyB )
			{
				return -SortOrder;
			}
			else
			{
				return 0;
			}
		}
	}

	/**
	 * Compare function used by Sort function.
	 *
	 * @param	Other	Other object to compare against
	 * @return	CurrentSortOrder if >, -CurrentSortOrder if < and 0 if ==
	 */
	INT Compare( const FPrimitiveStats& Other ) const
	{	
		INT CompareResult = Compare( Other, WxPrimitiveStatsBrowser::PrimarySortIndex );
		if( CompareResult == 0 )
		{
			CompareResult = Compare( Other, WxPrimitiveStatsBrowser::SecondarySortIndex );
		}
		return CompareResult;
	}
};

// Sort helper class.
IMPLEMENT_COMPARE_CONSTREF( FPrimitiveStats, PrimitiveStatsBrowser, { return A.Compare(B); });

/**
 * Returns whether the passed in object is part of a visible level.
 *
 * @param Object	object to check
 * @return TRUE if object is inside (as defined by chain of outers) in a visible level, FALSE otherwise
 */
static UBOOL IsInVisibleLevel( UObject* Object )
{
	check( Object );
	UObject* ObjectPackage = Object->GetOutermost();
	for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		if( Level && Level->GetOutermost() == ObjectPackage )
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Tells the browser to update itself
 */
void WxPrimitiveStatsBrowser::Update()
{
	UpdateList(TRUE);
}

/**
 * Sets auto column width. Needs to be called after resizing as well.
 */
void WxPrimitiveStatsBrowser::SetAutoColumnWidth()
{
	// Set proper column width
	for( INT ColumnIndex=0; ColumnIndex<ARRAY_COUNT(WxPrimitiveStatsBrowser::CurrentSortOrder); ColumnIndex++ )
	{
		INT Width = 0;
		ListControl->SetColumnWidth( ColumnIndex, wxLIST_AUTOSIZE );
		Width = Max( ListControl->GetColumnWidth( ColumnIndex ), Width );
		ListControl->SetColumnWidth( ColumnIndex, wxLIST_AUTOSIZE_USEHEADER );
		Width = Max( ListControl->GetColumnWidth( ColumnIndex ), Width );
		ListControl->SetColumnWidth( ColumnIndex, Width );
	}
}

/**
 * Updates the primitives list with new data
 *
 * @param bResizeColumns	Whether or not to resize the columns after updating data.
 */
void WxPrimitiveStatsBrowser::UpdateList(UBOOL bResizeColumns)
{
	BeginUpdate();

	// Do nothing unless we are visible
	if( IsShown() == TRUE && !GIsPlayInEditorWorld && GWorld && GWorld->CurrentLevel )
	{
		// Mesh to stats map.
		TMap<UObject*,FPrimitiveStats> ResourceToStatsMap;

		// Iterate over all static mesh components.
		for( TObjectIterator<UPrimitiveComponent> It; It; ++It )
		{
			UPrimitiveComponent*	PrimitiveComponent		= *It;
			UStaticMeshComponent*	StaticMeshComponent		= Cast<UStaticMeshComponent>(*It);
			UModelComponent*		ModelComponent			= Cast<UModelComponent>(*It);
			USkeletalMeshComponent*	SkeletalMeshComponent	= Cast<USkeletalMeshComponent>(*It);
			UTerrainComponent*		TerrainComponent		= Cast<UTerrainComponent>(*It);
			USpeedTreeComponent*	SpeedTreeComponent		= Cast<USpeedTreeComponent>(*It);
			UDecalComponent*		DecalComponent			= Cast<UDecalComponent>(*It);
			UObject*				Resource				= NULL;
			AActor*					ActorOuter				= Cast<AActor>(PrimitiveComponent->GetOuter());

			// The static mesh is a static mesh component's resource.
			if( StaticMeshComponent )
			{
				Resource = StaticMeshComponent->StaticMesh;
			}
			// A model component is its own resource.
			else if( ModelComponent )			
			{
				// Make sure model component is referenced by level.
				ULevel* Level = CastChecked<ULevel>(ModelComponent->GetOuter());
				if( Level->ModelComponents.FindItemIndex( ModelComponent ) != INDEX_NONE )
				{
					Resource = ModelComponent;
				}
			}
			// The skeletal mesh of a skeletal mesh component is its resource.
			else if( SkeletalMeshComponent )
			{
				Resource = SkeletalMeshComponent->SkeletalMesh;
			}
			// A terrain component's resource is the terrain actor.
			else if( TerrainComponent )
			{
				Resource = TerrainComponent->GetTerrain();
			}
			// The speed tree actor of a speed tree component is its resource.
			else if( SpeedTreeComponent )
			{
				Resource = SpeedTreeComponent->SpeedTree;
			}
			// the decal component instance is 
			else if( DecalComponent )
			{
				Resource = DecalComponent;
			}

			// Dont' care about components without a resource.
			if(	Resource 
			// Only list primitives in visible levels
			&&	IsInVisibleLevel( PrimitiveComponent ) 
			// Require actor association for selection and to disregard mesh emitter components. The exception being model components.
			&&	(ActorOuter || ModelComponent)
			// Don't list pending kill components.
			&&	!PrimitiveComponent->IsPendingKill() )
			{
				// Retrieve relevant lights.
				TArray<const ULightComponent*> RelevantLights;
				GWorld->Scene->GetRelevantLights( PrimitiveComponent, &RelevantLights );

				// Calculate number of direct and other lights relevant to this component.
				INT		LightsLMCount			= 0;
				INT		LightsOtherCount		= 0;
				INT		LightsSSCount			= 0;
				UBOOL	bUsesOnlyUnlitMaterials	= PrimitiveComponent->UsesOnlyUnlitMaterials();

				// Only look for relevant lights if we aren't unlit.
				if( bUsesOnlyUnlitMaterials == FALSE )
				{
					for( INT LightIndex=0; LightIndex<RelevantLights.Num(); LightIndex++ )
					{
						const ULightComponent* LightComponent = RelevantLights(LightIndex);
						// Skylights are considered "free".
						if( !LightComponent->IsA(USkyLightComponent::StaticClass()) )
						{
							// Lightmapped lights.
							if( (LightComponent->UseDirectLightMap || PrimitiveComponent->bForceDirectLightMap) && LightComponent->HasStaticLighting() )
							{
								LightsLMCount++;
							}
							// "Other" lights.
							else
							{
								LightsOtherCount++;
								// Lights with static shadowing information.
								if( LightComponent->HasStaticShadowing() )
								{
									LightsSSCount++;
								}
							}
						}
					}
				}

				// Figure out memory used by light and shadow maps and light/ shadow map resolution.
				INT LightMapWidth	= 0;
				INT LightMapHeight	= 0;
				PrimitiveComponent->GetLightMapResolution( LightMapWidth, LightMapHeight );
				INT LMSMResolution	= appSqrt( LightMapHeight * LightMapWidth );
				INT LightMapData	= 0;
				INT ShadowMapData	= 0;
				PrimitiveComponent->GetLightAndShadowMapMemoryUsage( LightMapData, ShadowMapData );

				// We only generate a lightmap if there is at least a single lightmapped light affecting this primitive.
				if( LightsLMCount == 0 )
				{
					LightMapData = 0;
				}
				// Shadowmap data is per light affecting the primitive that supports static shadowing.
				ShadowMapData *= LightsSSCount;

				// Check whether we alread have an entry for the associated static mesh.
				FPrimitiveStats* StatsEntry = ResourceToStatsMap.Find( Resource );
				if( StatsEntry )
				{
					// We do. Update existing entry.
					StatsEntry->Count++;
					StatsEntry->RadiusMin		= Min( StatsEntry->RadiusMin, PrimitiveComponent->Bounds.SphereRadius );
					StatsEntry->RadiusMax		= Max( StatsEntry->RadiusMax, PrimitiveComponent->Bounds.SphereRadius );
					StatsEntry->RadiusAvg		+= PrimitiveComponent->Bounds.SphereRadius;
					StatsEntry->LightsLM		+= LightsLMCount;
					StatsEntry->LightsOther		+= LightsOtherCount;
					StatsEntry->LightMapData	+= LightMapData;
					StatsEntry->ShadowMapData	+= ShadowMapData;
					StatsEntry->LMSMResolution	+= LMSMResolution;

					// ... in the case of a terrain component.
					if( TerrainComponent )
					{
						// If Count represents the Terrain itself, we do NOT want to increment it now.
						StatsEntry->Count--;
						// Currently, the engine will make a draw call per component, regardless of the number of batch materials.
						StatsEntry->Sections++;
						// For terrain, this will be the MAX rendered triangles...
						StatsEntry->Triangles += TerrainComponent->GetMaxTriangleCount();
					}
				}
				else
				{
					// We don't. Create new base entry.
					FPrimitiveStats NewStatsEntry = { 0 };
					NewStatsEntry.Resource		= Resource;
					NewStatsEntry.Count			= 1;
					NewStatsEntry.Triangles		= 0;
					NewStatsEntry.ResourceSize	= Resource->GetResourceSize();
					NewStatsEntry.Sections		= 0;
					NewStatsEntry.RadiusMin		= PrimitiveComponent->Bounds.SphereRadius;
					NewStatsEntry.RadiusAvg		= PrimitiveComponent->Bounds.SphereRadius;
					NewStatsEntry.RadiusMax		= PrimitiveComponent->Bounds.SphereRadius;
					NewStatsEntry.LightsLM		= LightsLMCount;
					NewStatsEntry.LightsOther	= LightsOtherCount;
					NewStatsEntry.LightMapData	= LightMapData;
					NewStatsEntry.ShadowMapData = ShadowMapData;
					NewStatsEntry.LMSMResolution= LMSMResolution;

					// Fix up triangle and section count...

					// ... in the case of a static mesh component.
					if( StaticMeshComponent )
					{
						UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh;
						if( StaticMesh )
						{
							for( INT ElementIndex=0; ElementIndex<StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
							{
								const FStaticMeshElement& StaticMeshElement = StaticMesh->LODModels(0).Elements(ElementIndex);
								NewStatsEntry.Triangles	+= StaticMeshElement.NumTriangles;
								NewStatsEntry.Sections++;
							}
						}
					}
					// ... in the case of a model component (aka BSP).
					else if( ModelComponent )
					{
						TIndirectArray<FModelElement> Elements = ModelComponent->GetElements();
						for( INT ElementIndex=0; ElementIndex<Elements.Num(); ElementIndex++ )
						{
							const FModelElement& Element = Elements(ElementIndex);
							NewStatsEntry.Triangles += Element.NumTriangles;
							NewStatsEntry.Sections++;
						}

					}
					// ... in the case of skeletal mesh component.
					else if( SkeletalMeshComponent )
					{
						USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;
						if( SkeletalMesh && SkeletalMesh->LODModels.Num() )
						{
							const FStaticLODModel& BaseLOD = SkeletalMesh->LODModels(0);
							for( INT SectionIndex=0; SectionIndex<BaseLOD.Sections.Num(); SectionIndex++ )
							{
								const FSkelMeshSection& Section = BaseLOD.Sections(SectionIndex);
								NewStatsEntry.Triangles += Section.NumTriangles;
								NewStatsEntry.Sections++;
							}
						}
					}
					// ... in the case of a terrain component.
					else if( TerrainComponent )
					{
						// Currently, the engine will make a draw call per component, regardless of the number of batch materials.
						NewStatsEntry.Sections++;
						// For terrain, this will be the MAX rendered triangles...
						NewStatsEntry.Triangles += TerrainComponent->GetMaxTriangleCount();
					}
					// ... in the case of a speedtree component
					else if( SpeedTreeComponent && SpeedTreeComponent->SpeedTree && SpeedTreeComponent->SpeedTree->SRH )
					{
#if WITH_SPEEDTREE
						FSpeedTreeResourceHelper* SRH = SpeedTreeComponent->SpeedTree->SRH;

						if( SpeedTreeComponent->bUseBranches && SRH->bHasBranches )
						{
							NewStatsEntry.Sections++;
							NewStatsEntry.Triangles += SRH->BranchElements(0).NumPrimitives;
						}
						if( SpeedTreeComponent->bUseFronds && SRH->bHasFronds )
						{
							NewStatsEntry.Sections++;
							NewStatsEntry.Triangles += SRH->FrondElements(0).NumPrimitives;
						}
						if( SpeedTreeComponent->bUseLeaves && SRH->bHasLeaves )
						{
							if(SRH->LeafMeshElements.Num())
							{
								NewStatsEntry.Sections++;
								NewStatsEntry.Triangles += SRH->LeafMeshElements(0).NumPrimitives;
							}
							if(SRH->LeafCardElements.Num())
							{
								NewStatsEntry.Sections++;
								NewStatsEntry.Triangles += SRH->LeafCardElements(0).NumPrimitives;
							}
						}
#endif
					}
					// ... in the case of a decal component
					else if( DecalComponent )
					{
						for( INT ReceiverIdx=0; ReceiverIdx < DecalComponent->DecalReceivers.Num(); ReceiverIdx++ )
						{
							const FDecalReceiver& DecalReceiver = DecalComponent->DecalReceivers(ReceiverIdx);
							if( DecalReceiver.RenderData )
							{
								// every receiver counts as a section since it will be rendered separately
								NewStatsEntry.Sections++;
								// total number of triangles for this receiver
								NewStatsEntry.Triangles += DecalReceiver.RenderData->NumTriangles;
							}
						}
					}
					
					// Add to map.
					ResourceToStatsMap.Set( Resource, NewStatsEntry );
				}
			}
		}

		ListControl->Freeze();
		{
			// Clear existing items.
			ListControl->DeleteAllItems();

			// Gather total stats.
			FPrimitiveStats	CombinedStats					= { 0 };
			CombinedStats.RadiusMin							= FLT_MAX;
			FLOAT				CombinedObjLightCost		= 0;
			FLOAT				CombinedInstTriangles		= 0;
			FLOAT				CombinedStatsLightsLM		= 0;
			FLOAT				CombinedStatsLightsOther	= 0;
			FLOAT				CombinedStatsLightsTotal	= 0;
			FLOAT				CombinedLightMapData		= 0;
			FLOAT				CombinedShadowMapData		= 0;
			FLOAT				CombinedLMSMResolution		= 0;

			// Copy stats over to soon to be sorted array and gather combined stats.
			TArray<FPrimitiveStats> PrimitiveStats;
			for( TMap<UObject*,FPrimitiveStats>::TIterator It(ResourceToStatsMap); It; ++It )
			{
				const FPrimitiveStats& StatsEntry = It.Value();
				PrimitiveStats.AddItem(StatsEntry);

				CombinedStats.Count			+= StatsEntry.Count;
				CombinedStats.Triangles		+= StatsEntry.Triangles;
				CombinedStats.ResourceSize	+= StatsEntry.GetColumnData( PCSBC_ResourceSize		);
				CombinedStats.Sections		+= StatsEntry.Sections * StatsEntry.Count;
				CombinedStats.RadiusMin		= Min( CombinedStats.RadiusMin, StatsEntry.RadiusMin );
				CombinedStats.RadiusMax		= Max( CombinedStats.RadiusMax, StatsEntry.RadiusMax );
				CombinedStats.RadiusAvg		+= StatsEntry.GetColumnData( PCSBC_RadiusAvg		);
				CombinedStatsLightsLM		+= StatsEntry.GetColumnData( PCSBC_LightsLM			);
				CombinedStatsLightsOther	+= StatsEntry.GetColumnData( PCSBC_LightsOther		);
				CombinedObjLightCost		+= StatsEntry.GetColumnData( PCSBC_ObjLightCost		);
				CombinedInstTriangles		+= StatsEntry.GetColumnData( PCSBC_InstTriangles	);
				CombinedLightMapData		+= StatsEntry.GetColumnData( PCSBC_LightMapData		);
				CombinedShadowMapData		+= StatsEntry.GetColumnData( PCSBC_ShadowMapData	);
				CombinedLMSMResolution		+= StatsEntry.GetColumnData( PCSBC_LMSMResolution	);
			}

			// Average out certain combined stats.
			if( PrimitiveStats.Num() )
			{
				CombinedStats.RadiusAvg		/= PrimitiveStats.Num();
				CombinedStatsLightsLM		/= PrimitiveStats.Num();
				CombinedStatsLightsOther	/= PrimitiveStats.Num();
				CombinedStatsLightsTotal	 = CombinedStatsLightsOther + CombinedStatsLightsLM;
				CombinedLMSMResolution		/= PrimitiveStats.Num();
			}
			else
			{
				CombinedStats.RadiusMin		= 0;
			}

			// Sort static mesh stats based on sort criteria. We are NOT using wxListCtrl sort mojo here as implementation would be annoying
			// and there are a couple of features down the road that I don't think would be trivial to implement with its sorting.
			Sort<USE_COMPARE_CONSTREF(FPrimitiveStats,PrimitiveStatsBrowser)>( PrimitiveStats.GetTypedData(), PrimitiveStats.Num() );

			// Add sorted items.
			for( INT StatsIndex=0; StatsIndex<PrimitiveStats.Num(); StatsIndex++ )
			{
				const FPrimitiveStats& StatsEntry = PrimitiveStats(StatsIndex);

				long ItemIndex = ListControl->InsertItem( 0, *StatsEntry.Resource->GetClass()->GetName() );
				ListControl->SetItem( ItemIndex, PCSBC_Name,			*StatsEntry.Resource->GetPathName()													   ); // Name
				ListControl->SetItem( ItemIndex, PCSBC_Count,			*StatsEntry.GetColumnDataString( PCSBC_Count										 ) ); // Count
				ListControl->SetItem( ItemIndex, PCSBC_Triangles,		*StatsEntry.GetColumnDataString( PCSBC_Triangles									 ) ); // Triangles
				ListControl->SetItem( ItemIndex, PCSBC_InstTriangles,	*StatsEntry.GetColumnDataString( PCSBC_InstTriangles								 ) ); // Instanced Triangles
				ListControl->SetItem( ItemIndex, PCSBC_ResourceSize,	*StatsEntry.GetColumnDataString( PCSBC_ResourceSize									 ) ); // ResourceSize
				ListControl->SetItem( ItemIndex, PCSBC_LightsLM,		*FString::Printf(TEXT("%.3f"),		StatsEntry.GetColumnData( PCSBC_LightsLM		)) ); // LightsLM
				ListControl->SetItem( ItemIndex, PCSBC_LightsOther,		*FString::Printf(TEXT("%.3f"),		StatsEntry.GetColumnData( PCSBC_LightsOther		)) ); // LightsOther
				ListControl->SetItem( ItemIndex, PCSBC_LightsTotal,		*FString::Printf(TEXT("%.3f"),		StatsEntry.GetColumnData( PCSBC_LightsTotal		)) ); // LightsTotal
				ListControl->SetItem( ItemIndex, PCSBC_ObjLightCost,	*StatsEntry.GetColumnDataString( PCSBC_ObjLightCost									 ) ); // ObjLightCost
				ListControl->SetItem( ItemIndex, PCSBC_LightMapData,	*StatsEntry.GetColumnDataString( PCSBC_LightMapData									 ) ); // LightMapData
				ListControl->SetItem( ItemIndex, PCSBC_ShadowMapData,	*StatsEntry.GetColumnDataString( PCSBC_ShadowMapData								 ) ); // ShadowMapData
				ListControl->SetItem( ItemIndex, PCSBC_LMSMResolution,	*StatsEntry.GetColumnDataString( PCSBC_LMSMResolution								 ) ); // LMSMResolution
				ListControl->SetItem( ItemIndex, PCSBC_Sections,		*StatsEntry.GetColumnDataString( PCSBC_Sections										 ) ); // Sections
				ListControl->SetItem( ItemIndex, PCSBC_RadiusMin,		*StatsEntry.GetColumnDataString( PCSBC_RadiusMin									 ) ); // RadiusMin
				ListControl->SetItem( ItemIndex, PCSBC_RadiusMax,		*StatsEntry.GetColumnDataString( PCSBC_RadiusMax									 ) ); // RadiusMax
				ListControl->SetItem( ItemIndex, PCSBC_RadiusAvg,		*StatsEntry.GetColumnDataString( PCSBC_RadiusAvg									 ) ); // RadiusAvg
			}

			// Add combined stats.
			if( TRUE )
			{
				long ItemIndex = ListControl->InsertItem( 0, TEXT("") );
				ListControl->SetItem( ItemIndex, PCSBC_Name,			*LocalizeUnrealEd("PrimitiveStatsBrowser_CombinedStats")          ); // Name
				ListControl->SetItem( ItemIndex, PCSBC_Count,			*FFormatIntToHumanReadable( CombinedStats.Count			) ); // Count
				ListControl->SetItem( ItemIndex, PCSBC_Triangles,		*FFormatIntToHumanReadable( CombinedStats.Triangles		) ); // Triangles
				ListControl->SetItem( ItemIndex, PCSBC_InstTriangles,	*FFormatIntToHumanReadable( CombinedInstTriangles		) ); // Instanced Triangles
				ListControl->SetItem( ItemIndex, PCSBC_ResourceSize,	*FFormatIntToHumanReadable( CombinedStats.ResourceSize	) ); // ResourceSize
				ListControl->SetItem( ItemIndex, PCSBC_LightsLM,		*FString::Printf(TEXT("%.3f"),		CombinedStatsLightsLM		) ); // LightsLM
				ListControl->SetItem( ItemIndex, PCSBC_LightsOther,		*FString::Printf(TEXT("%.3f"),		CombinedStatsLightsOther	) ); // LightsOther
				ListControl->SetItem( ItemIndex, PCSBC_LightsTotal,		*FString::Printf(TEXT("%.3f"),		CombinedStatsLightsTotal	) ); // LightsTotal
				ListControl->SetItem( ItemIndex, PCSBC_ObjLightCost,	*FFormatIntToHumanReadable( CombinedObjLightCost		) ); // ObjLightCost
				ListControl->SetItem( ItemIndex, PCSBC_LightMapData,	*FFormatIntToHumanReadable( CombinedLightMapData		) ); // LightMapData
				ListControl->SetItem( ItemIndex, PCSBC_ShadowMapData,	*FFormatIntToHumanReadable( CombinedShadowMapData		) ); // ShadowMapData
				ListControl->SetItem( ItemIndex, PCSBC_LMSMResolution,	*FFormatIntToHumanReadable( CombinedLMSMResolution		) ); // LMSMResolution
				ListControl->SetItem( ItemIndex, PCSBC_Sections,		*FFormatIntToHumanReadable( CombinedStats.Sections		) ); // Sections
				ListControl->SetItem( ItemIndex, PCSBC_RadiusMin,		*FFormatIntToHumanReadable( CombinedStats.RadiusMin		) ); // RadiusMin
				ListControl->SetItem( ItemIndex, PCSBC_RadiusMax,		*FFormatIntToHumanReadable( CombinedStats.RadiusMax		) ); // RadiusMax
				ListControl->SetItem( ItemIndex, PCSBC_RadiusAvg,		*FFormatIntToHumanReadable( CombinedStats.RadiusAvg		) ); // RadiusAvg
			}

			// Dump to CSV file if wanted.
			if( GDumpPrimitiveStatsToCSVDuringNextUpdate )
			{
				// Number of rows == number of primitive stats plus combined stat.
				INT NumRows = PrimitiveStats.Num() + 1;
				DumpToCSV( NumRows );
			}

			// Set proper column width.
			if(bResizeColumns == TRUE)
			{
				SetAutoColumnWidth();
			}
		}
		ListControl->Thaw();
	}

	EndUpdate();
}

/**
 * Dumps current stats to CVS file.
 *
 * @param NumRows	Number of rows to dump
 */
void WxPrimitiveStatsBrowser::DumpToCSV( INT NumRows )
{
	check(PCSBC_MAX == ColumnHeaders.Num());

	// Create string with system time to create a unique filename.
	INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
	appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
	FString	CurrentTime = FString::Printf(TEXT("%i.%02i.%02i-%02i.%02i.%02i"), Year, Month, Day, Hour, Min, Sec );

	// CSV: Human-readable spreadsheet format.
	FString CSVFilename	= FString::Printf(TEXT("%sPrimitiveStats-%s-%s-%i-%s.csv"), 
								*appGameLogDir(), 
								*GWorld->GetOutermost()->GetName(), 
								GGameName, 
								GEngineVersion, 
								*CurrentTime);
	FArchive* CSVFile = GFileManager->CreateFileWriter( *CSVFilename );
	if( CSVFile )
	{
		// Write out header row.
		FString HeaderRow;
		for( INT ColumnIndex=0; ColumnIndex<PCSBC_MAX; ColumnIndex++ )
		{
			HeaderRow += ColumnHeaders(ColumnIndex);
			HeaderRow += TEXT(",");
		}
		HeaderRow += LINE_TERMINATOR;
		CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

		// Write individual rows. The + 1 is for the combined stats.
		for( INT RowIndex=0; RowIndex<NumRows; RowIndex++ )
		{
			FString Row;
			for( INT ColumnIndex=0; ColumnIndex<PCSBC_MAX; ColumnIndex++ )
			{
				Row += ListControl->GetColumnItemText( RowIndex, ColumnIndex );
				Row += TEXT(",");
			}
			Row += LINE_TERMINATOR;
			CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
		}

		// Close and delete archive.
		CSVFile->Close();
		delete CSVFile;
		CSVFile = NULL;
	}

	// Reset variable now that we dumped the stats to CSV.
	GDumpPrimitiveStatsToCSVDuringNextUpdate = FALSE;
}


/**
 * Sets the size of the list control based upon our new size
 *
 * @param In the command that was sent
 */
void WxPrimitiveStatsBrowser::OnSize( wxSizeEvent& In )
{
	// During the creation process a sizing message can be sent so don't
	// handle it until we are initialized
	if( bAreWindowsInitialized )
	{
		ListControl->SetSize( GetClientRect() );
		SetAutoColumnWidth();
	}
}

/**
 * Handler for column click events
 *
 * @param In the command that was sent
 */
void WxPrimitiveStatsBrowser::OnColumnClick( wxListEvent& In )
{
	INT ColumnIndex = In.GetColumn();

	if( ColumnIndex >= 0 )
	{
		if( WxPrimitiveStatsBrowser::PrimarySortIndex == ColumnIndex )
		{
			check( ColumnIndex < ARRAY_COUNT(WxPrimitiveStatsBrowser::CurrentSortOrder) );
			WxPrimitiveStatsBrowser::CurrentSortOrder[ColumnIndex] *= -1;
		}
		WxPrimitiveStatsBrowser::PrimarySortIndex = ColumnIndex;

		// Recreate the list from scratch.
		UpdateList(FALSE);
	}
}

/**
 * Handler for column right click events
 *
 * @param In the command that was sent
 */
void WxPrimitiveStatsBrowser::OnColumnRightClick( wxListEvent& In )
{
	INT ColumnIndex = In.GetColumn();

	if( ColumnIndex >= 0 )
	{
		if( WxPrimitiveStatsBrowser::SecondarySortIndex == ColumnIndex )
		{
			check( ColumnIndex < ARRAY_COUNT(WxPrimitiveStatsBrowser::CurrentSortOrder) );
			WxPrimitiveStatsBrowser::CurrentSortOrder[ColumnIndex] *= -1;
		}
		WxPrimitiveStatsBrowser::SecondarySortIndex = ColumnIndex;

		// Recreate the list from scratch.
		UpdateList(FALSE);
	}
}

/**
 * Handler for item activation (double click) event
 *
 * @param In the command that was sent
 */
void WxPrimitiveStatsBrowser::OnItemActivated( wxListEvent& In )
{
	// Try to find static mesh matching the name.
	FString				ResourceName	= FString( ListControl->GetColumnItemText( In.GetIndex(), PCSBC_Name ) );
	UStaticMesh*		StaticMesh		= FindObject<UStaticMesh>( NULL, *ResourceName );
	USkeletalMesh*		SkeletalMesh	= FindObject<USkeletalMesh>( NULL, *ResourceName );
	UModelComponent*	ModelComponent	= FindObject<UModelComponent>( NULL, *ResourceName );
	UDecalComponent*	DecalComponent	= FindObject<UDecalComponent>( NULL, *ResourceName );


	// Make sure this action is undoable.
	const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("PrimitiveStatsBroswer_ItemActivated")) );
	// Deselect everything.
	GUnrealEd->SelectNone( TRUE, TRUE );

	// Select all matching static mesh actors.
	if( StaticMesh )
	{
		// Iterate over all actors, finding for matching ones.
		for( FActorIterator It; It; ++It )
		{
			AStaticMeshActor*			StaticMeshActor = Cast<AStaticMeshActor>(*It);
			ADynamicSMActor*			DynamicSMActor	= Cast<ADynamicSMActor>(*It);
			AFracturedStaticMeshActor*	FSMActor		= Cast<AFracturedStaticMeshActor>(*It);
			AActor*						Match			= NULL;

			if(	StaticMeshActor 
			&&	!StaticMeshActor->IsHiddenEd() 
			&&	StaticMeshActor->StaticMeshComponent 
			&&	StaticMeshActor->StaticMeshComponent->StaticMesh == StaticMesh )
			{
				Match = StaticMeshActor;
			}

			if(	DynamicSMActor
			&&	!DynamicSMActor->IsHiddenEd() 
			&&	DynamicSMActor->StaticMeshComponent 
			&&	DynamicSMActor->StaticMeshComponent->StaticMesh == StaticMesh )
			{
				Match = DynamicSMActor;
			}

			if( FSMActor 
			&&	!FSMActor->IsHiddenEd()
			&&	FSMActor->FracturedStaticMeshComponent
			&&	FSMActor->FracturedStaticMeshComponent->StaticMesh == StaticMesh )
			{
				Match = FSMActor;
			}

			if( Match )
			{
				// Select actor with matching static mesh.
				GUnrealEd->SelectActor( Match, TRUE, NULL, FALSE );
			}
		}
	}
	// Select all matching skeletal mesh actors
	if( SkeletalMesh )
	{
		// Iterate over all actors, finding for matching ones.
		for( FActorIterator It; It; ++It )
		{
			ASkeletalMeshActor* SkeletalMeshActor	= Cast<ASkeletalMeshActor>(*It);
			AActor*				Match				= NULL;

			if(	SkeletalMeshActor 
			&&	!SkeletalMeshActor->IsHiddenEd() 
			&&	SkeletalMeshActor->SkeletalMeshComponent 
			&&	SkeletalMeshActor->SkeletalMeshComponent->SkeletalMesh == SkeletalMesh )
			{
				Match = SkeletalMeshActor;
			}

			if( Match )
			{
				// Select actor with matching static mesh.
				GUnrealEd->SelectActor( Match, TRUE, NULL, FALSE );
			}
		}
	}
	// Select all surfaces in this model component after making its level current.
	else if( ModelComponent )
	{
		// Set the current level to the one this model component resides in so surface selection works appropriately.
		ULevel* Level = CastChecked<ULevel>(ModelComponent->GetOuter());

		UBOOL bSendNewCurrentLevelEvent = ( GWorld->CurrentLevel != Level );
		GWorld->CurrentLevel = Level;
		if( bSendNewCurrentLevelEvent )
		{
			GCallbackEvent->Send( CALLBACK_NewCurrentLevel );
		}

		ModelComponent->SelectAllSurfaces();
	}
	// Select the decal component's actor
	else if( DecalComponent )
	{
		ADecalActorBase* DecalActor = Cast<ADecalActorBase>(DecalComponent->GetOwner());
		if( DecalActor &&
			!DecalActor->IsHiddenEd() )
		{
			GUnrealEd->SelectActor( DecalActor, TRUE, NULL, FALSE );
		}
	}

	GUnrealEd->NoteSelectionChange();
}

