/*=============================================================================
	UnEdExp.cpp: Editor exporters.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "UnTerrain.h"
#include "EngineSequenceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"

/*------------------------------------------------------------------------------
	UTextBufferExporterTXT implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextBufferExporterTXT::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UTextBuffer::StaticClass();
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("TXT")) );
	new(FormatDescription)FString(TEXT("Text file"));
	bText = 1;
}
UBOOL UTextBufferExporterTXT::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	UTextBuffer* TextBuffer = CastChecked<UTextBuffer>( Object );
	FString Str( TextBuffer->Text );

	TCHAR* Start = const_cast<TCHAR*>(*Str);
	TCHAR* End   = Start + Str.Len();
	while( Start<End && (Start[0]=='\r' || Start[0]=='\n' || Start[0]==' ') )
		Start++;
	while( End>Start && (End [-1]=='\r' || End [-1]=='\n' || End [-1]==' ') )
		End--;
	*End = 0;

	Ar.Log( Start );

	return 1;
}
IMPLEMENT_CLASS(UTextBufferExporterTXT);

/*------------------------------------------------------------------------------
	USoundExporterWAV implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
#if 1
void USoundExporterWAV::InitializeIntrinsicPropertyValues()
{
	SupportedClass = USoundNodeWave::StaticClass();
	bText = 0;
	new( FormatExtension ) FString( TEXT( "WAV" ) );
	new( FormatDescription ) FString( TEXT( "Sound" ) );
}
UBOOL USoundExporterWAV::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	USoundNodeWave* Sound = CastChecked<USoundNodeWave>( Object );
	void* RawWaveData = Sound->RawData.Lock( LOCK_READ_ONLY );
	Ar.Serialize( RawWaveData, Sound->RawData.GetBulkDataSize() );
	Sound->RawData.Unlock();
	return TRUE;
}
IMPLEMENT_CLASS(USoundExporterWAV);

#else
/*------------------------------------------------------------------------------
	USoundExporterOGG implementation.
------------------------------------------------------------------------------*/

/**
* Initializes property values for intrinsic classes.  It is called immediately after the class default object
* is initialized against its archetype, but before any objects of this class are created.
*/
void USoundExporterOGG::InitializeIntrinsicPropertyValues()
{
	SupportedClass = USoundNodeWave::StaticClass();
	bText = 0;
	new( FormatExtension ) FString( TEXT( "OGG" ) );
	new( FormatDescription ) FString( TEXT( "Sound" ) );
}
UBOOL USoundExporterOGG::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	USoundNodeWave* Sound = CastChecked<USoundNodeWave>( Object );
	void* RawOggData = Sound->CompressedPCData.Lock( LOCK_READ_ONLY );
	Ar.Serialize( RawOggData, Sound->CompressedPCData.GetBulkDataSize() );
	Sound->CompressedPCData.Unlock();
	return TRUE;
}
IMPLEMENT_CLASS(USoundExporterOGG);
#endif

/*------------------------------------------------------------------------------
	USoundSurroundExporterWAV implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USoundSurroundExporterWAV::InitializeIntrinsicPropertyValues()
{
	SupportedClass = USoundNodeWave::StaticClass();
	bText = 0;
	new( FormatExtension ) FString( TEXT( "WAV" ) );
	new( FormatDescription ) FString( TEXT( "Multichannel Sound" ) );
}

INT USoundSurroundExporterWAV::GetFileCount( void ) const
{
	return( SPEAKER_Count );
}

FString USoundSurroundExporterWAV::GetUniqueFilename( const TCHAR* Filename, INT FileIndex )
{
	static FString SpeakerLocations[SPEAKER_Count] =
	{
		TEXT( "_fl" ),			// SPEAKER_FrontLeft
		TEXT( "_fr" ),			// SPEAKER_FrontRight
		TEXT( "_fc" ),			// SPEAKER_FrontCenter
		TEXT( "_lf" ),			// SPEAKER_LowFrequency
		TEXT( "_sl" ),			// SPEAKER_SideLeft
		TEXT( "_sr" ),			// SPEAKER_SideRight
		TEXT( "_bl" ),			// SPEAKER_BackLeft
		TEXT( "_br" )			// SPEAKER_BackRight
	};

	FFilename WorkName = Filename;
	FString ReturnName = WorkName.GetBaseFilename( FALSE ) + SpeakerLocations[FileIndex] + FString( ".WAV" );

	return( ReturnName );
}

UBOOL USoundSurroundExporterWAV::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	UBOOL bResult = FALSE;

	USoundNodeWave* Sound = CastChecked<USoundNodeWave>( Object );
	if ( Sound->ChannelSizes.Num() > 0 )
	{
		BYTE* RawWaveData = ( BYTE * )Sound->RawData.Lock( LOCK_READ_ONLY );

		if( Sound->ChannelSizes( FileIndex ) )
		{
			Ar.Serialize( RawWaveData + Sound->ChannelOffsets( FileIndex ), Sound->ChannelSizes( FileIndex ) );
		}

		Sound->RawData.Unlock();

		bResult = Sound->ChannelSizes( FileIndex ) != 0;
	}

	return bResult;
}
IMPLEMENT_CLASS(USoundSurroundExporterWAV);

/*------------------------------------------------------------------------------
	UClassExporterUC implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UClassExporterUC::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UClass::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("UC")) );
	new(FormatDescription)FString(TEXT("UnrealScript"));
}
UBOOL UClassExporterUC::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	UClass* Class = CastChecked<UClass>( Object );

	// if the class is instrinsic, it won't have script text
	if ( (Class->ClassFlags&CLASS_Intrinsic) != 0 )
	{
		return FALSE;
	}

	// Nothing to do if the script text was e.g. stripped by the cooker.
	if ( Class->ScriptText == NULL )
	{
		return FALSE;
	}

	// Export script text.
	check(Class->GetDefaultsCount());
	UExporter::ExportToOutputDevice( Context, Class->ScriptText, NULL, Ar, TEXT("txt"), TextIndent, PortFlags );

	// Export cpptext.
	if( Class->CppText )
	{
		Ar.Log( TEXT("\r\n\r\ncpptext\r\n{\r\n") );
		Ar.Log( *Class->CppText->Text );
		Ar.Log( TEXT("\r\n}\r\n") );
	}

	// Export default properties that differ from parent's.
	Ar.Log( TEXT("\r\n\r\ndefaultproperties\r\n{\r\n") );
	ExportProperties
	(
		Context,
		Ar,
		Class,
		Class->GetDefaults(),
		TextIndent+3,
		Class->GetSuperClass(),
		Class->GetSuperClass() ? Class->GetSuperClass()->GetDefaults() : NULL,
		Class
	);
	Ar.Log( TEXT("}\r\n") );

	return TRUE;
}
IMPLEMENT_CLASS(UClassExporterUC);

/*------------------------------------------------------------------------------
	UObjectExporterT3D implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UObjectExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UObject::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3D")) );
	new(FormatDescription)FString(TEXT("Unreal object text"));
	new(FormatExtension)FString(TEXT("COPY"));
	new(FormatDescription)FString(TEXT("Unreal object text"));
}
UBOOL UObjectExporterT3D::ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags )
{
	Ar.Logf( TEXT("%sBegin Object Class=%s Name=%s ObjName=%s Archetype=%s'%s'") LINE_TERMINATOR,
		appSpc(TextIndent), *Object->GetClass()->GetName(), *Object->GetName(), *Object->GetName(),
		*Object->GetArchetype()->GetClass()->GetName(), *Object->GetArchetype()->GetPathName() );
		ExportObjectInner( Context, Object, Ar, PortFlags);
	Ar.Logf( TEXT("%sEnd Object") LINE_TERMINATOR, appSpc(TextIndent) );

	return 1;
}
IMPLEMENT_CLASS(UObjectExporterT3D);

/*------------------------------------------------------------------------------
	UPolysExporterT3D implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UPolysExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UPolys::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3D")) );
	new(FormatDescription)FString(TEXT("Unreal poly text"));
}
UBOOL UPolysExporterT3D::ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags )
{
	UPolys* Polys = CastChecked<UPolys>( Object );

	FLightingChannelContainer		DefaultLightingChannels;
	DefaultLightingChannels.Bitfield		= 0;
	DefaultLightingChannels.BSP				= TRUE;
	DefaultLightingChannels.bInitialized	= TRUE;

	Ar.Logf( TEXT("%sBegin PolyList\r\n"), appSpc(TextIndent) );
	for( INT i=0; i<Polys->Element.Num(); i++ )
	{
		FPoly* Poly = &Polys->Element(i);
		TCHAR TempStr[MAX_SPRINTF]=TEXT("");

		// Start of polygon plus group/item name if applicable.
		// The default values need to jive FPoly::Init().
		Ar.Logf( TEXT("%s   Begin Polygon"), appSpc(TextIndent) );
		if( Poly->ItemName != NAME_None )
			Ar.Logf( TEXT(" Item=%s"), *Poly->ItemName.ToString() );
		if( Poly->Material )
			Ar.Logf( TEXT(" Texture=%s"), *Poly->Material->GetPathName() );
		if( Poly->PolyFlags != 0 )
			Ar.Logf( TEXT(" Flags=%i"), Poly->PolyFlags );
		if( Poly->iLink != INDEX_NONE )
			Ar.Logf( TEXT(" Link=%i"), Poly->iLink );
		if ( Poly->ShadowMapScale != 32.0f )
			Ar.Logf( TEXT(" ShadowMapScale=%f"), Poly->ShadowMapScale );
		if ( Poly->LightingChannels != DefaultLightingChannels.Bitfield )
			Ar.Logf( TEXT(" LightingChannels=%i"), Poly->LightingChannels );
		Ar.Logf( TEXT("\r\n") );

		// All coordinates.
		Ar.Logf( TEXT("%s      Origin   %s\r\n"), appSpc(TextIndent), SetFVECTOR(TempStr,&Poly->Base) );
		Ar.Logf( TEXT("%s      Normal   %s\r\n"), appSpc(TextIndent), SetFVECTOR(TempStr,&Poly->Normal) );
		Ar.Logf( TEXT("%s      TextureU %s\r\n"), appSpc(TextIndent), SetFVECTOR(TempStr,&Poly->TextureU) );
		Ar.Logf( TEXT("%s      TextureV %s\r\n"), appSpc(TextIndent), SetFVECTOR(TempStr,&Poly->TextureV) );
		for( INT j=0; j<Poly->Vertices.Num(); j++ )
			Ar.Logf( TEXT("%s      Vertex   %s\r\n"), appSpc(TextIndent), SetFVECTOR(TempStr,&Poly->Vertices(j)) );
		Ar.Logf( TEXT("%s   End Polygon\r\n"), appSpc(TextIndent) );
	}
	Ar.Logf( TEXT("%sEnd PolyList\r\n"), appSpc(TextIndent) );

	return 1;
}
IMPLEMENT_CLASS(UPolysExporterT3D);

/*------------------------------------------------------------------------------
	UModelExporterT3D implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UModelExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UModel::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3D")) );
	new(FormatDescription)FString(TEXT("Unreal model text"));
}
UBOOL UModelExporterT3D::ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags )
{
	UModel* Model = CastChecked<UModel>( Object );

	Ar.Logf( TEXT("%sBegin Brush Name=%s\r\n"), appSpc(TextIndent), *Model->GetName() );
		UExporter::ExportToOutputDevice( Context, Model->Polys, NULL, Ar, Type, TextIndent+3, PortFlags );
//		ExportObjectInner( Context, Model, Ar, PortFlags | PPF_ExportsNotFullyQualified );
	Ar.Logf( TEXT("%sEnd Brush\r\n"), appSpc(TextIndent) );

	return 1;
}
IMPLEMENT_CLASS(UModelExporterT3D);

/*------------------------------------------------------------------------------
	ULevelExporterT3D implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void ULevelExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UWorld::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3D")) );
	new(FormatDescription)FString(TEXT("Unreal world text"));
	new(FormatExtension)FString(TEXT("COPY"));
	new(FormatDescription)FString(TEXT("Unreal world text"));
}

void ExporterHelper_DumpPackageInners(const FExportObjectInnerContext* Context, UPackage* InPackage, INT TabCount)
{
	const TArray<UObject*>* Inners = Context->GetObjectInners(InPackage);
	if (Inners)
	{
		for (INT InnerIndex = 0; InnerIndex < Inners->Num(); InnerIndex++)
		{
			UObject* InnerObj = (*Inners)(InnerIndex);

			FString OutputString;
			for (INT TabOutIndex = 0; TabOutIndex < TabCount; TabOutIndex++)
			{
				OutputString += TEXT("\t");
				OutputString += TEXT("\t");
			}

			OutputString += FString::Printf(TEXT("%s : %s (%s)"), 
				InnerObj ? *(InnerObj->GetClass()->GetName()) : TEXT("*NULL*"),
				InnerObj ? *(InnerObj->GetName()) : TEXT("*NULL*"),
				InnerObj ? *(InnerObj->GetPathName()) : TEXT("*NULL*"));
			debugf(*OutputString);

			UPackage* InnerPackage = Cast<UPackage>(InnerObj);
			if (InnerPackage)
			{
				TabCount++;
				ExporterHelper_DumpPackageInners(Context, InnerPackage, TabCount);
				TabCount--;
			}
		}
	}
}

UBOOL ULevelExporterT3D::ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags )
{
	UWorld* World = CastChecked<UWorld>( Object );
	APhysicsVolume* DefaultPhysicsVolume = World->GetDefaultPhysicsVolume();

	for( FObjectIterator It; It; ++It )
		It->ClearFlags( RF_TagImp | RF_TagExp );

	UPackage* MapPackage = NULL;
	if ((PortFlags & PPF_Copy) == 0)
	{
		// If we are not copying to clipboard, then export objects contained in the map package itself...
		MapPackage = Cast<UPackage>(Object->GetOutermost());
	}

	// this is the top level in the .t3d file
	if (MapPackage)
	{
		Ar.Logf(TEXT("%sBegin Map Name=%s\r\n"), appSpc(TextIndent),  *(MapPackage->GetName()));
	}
	else
	{
		Ar.Logf(TEXT("%sBegin Map\r\n"), appSpc(TextIndent));
	}

	// are we exporting all actors or just selected actors?
	UBOOL bAllActors = appStricmp(Type,TEXT("COPY"))!=0 && !bSelectedOnly;

	TextIndent += 3;

	if ((PortFlags & PPF_Copy) == 0)
	{
		// If we are not copying to clipboard, then export objects contained in the map package itself...
		UPackage* MapPackage = Cast<UPackage>(Object->GetOutermost());
		if (MapPackage && ((MapPackage->PackageFlags & PKG_ContainsMap) != 0))
		{
			MapPackage->FullyLoad();

			debugf(TEXT("Exporting objects found in map package: %s"), *(MapPackage->GetName()));
			ExporterHelper_DumpPackageInners(Context, MapPackage, 1);
/***
			// We really only care about the following:
			//	Packages (really groups under the main package)
			//	Materials
			//	MaterialInstanceConstants
			//	MaterialInstanceTimeVarying
			//	Textures
			FExportPackageParams ExpPackageParams;
			ExpPackageParams.RootMapPackageName = MapPackage->GetName();
			ExpPackageParams.Context = Context;
			ExpPackageParams.InPackage = MapPackage;
			ExpPackageParams.Type = Type;
			ExpPackageParams.Ar = &Ar;
			ExpPackageParams.Warn = Warn;
			ExpPackageParams.PortFlags = PortFlags;
			ExpPackageParams.InObject = NULL;
			ExportPackageInners(ExpPackageParams);
***/
			Ar.Logf(TEXT("%sBegin MapPackage\r\n"), appSpc(TextIndent));
				TextIndent += 3;
				UPackageExporterT3D* PackageExp = new UPackageExporterT3D();
				check(PackageExp);
				PackageExp->TextIndent = TextIndent;
				PackageExp->ExportText(Context, MapPackage, TEXT("T3DPKG"), Ar, Warn, PortFlags);
				TextIndent -=3;
			Ar.Logf(TEXT("%sEnd MapPackage\r\n"), appSpc(TextIndent));
		}
	}

	TextIndent -= 3;
	TextIndent += 3;

	ULevel* Level;

	// start a new level section
	if (appStricmp(Type, TEXT("COPY")) == 0)
	{
		// for copy and paste, we want to select actors in the current level
		Level = World->CurrentLevel;

		// if we are copy/pasting, then we don't name the level - we paste into the current level
		Ar.Logf(TEXT("%sBegin Level\r\n"), appSpc(TextIndent));

		// mark that we are doing a clipboard copy
		PortFlags |= PPF_Copy;
	}
	else
	{
		// for export, we only want the persistent level
		Level = World->PersistentLevel;

		//@todo seamless if we are exporting only selected, should we export from all levels? or maybe from the current level?

		// if we aren't copy/pasting, then we name the level so that when we import, we get the same level structure
		Ar.Logf(TEXT("%sBegin Level NAME=%s\r\n"), appSpc(TextIndent), *Level->GetName());
	}

	TextIndent += 3;

	// loop through all of the actors just in this level
	for( INT iActor=0; iActor<Level->Actors.Num(); iActor++ )
	{
		AActor* Actor = Level->Actors(iActor);
		// Don't export the default physics volume, as it doesn't have a UModel associated with it
		// and thus will not import properly.
		if ( Actor == DefaultPhysicsVolume )
		{
			continue;
		}
		ATerrain* pkTerrain = Cast<ATerrain>(Actor);
		if (pkTerrain && (bAllActors || pkTerrain->IsSelected()))
		{
			// Terrain exporter...
			// Find the UTerrainExporterT3D exporter?
			UTerrainExporterT3D* pkTerrainExp = ConstructObject<UTerrainExporterT3D>(UTerrainExporterT3D::StaticClass());
			if (pkTerrainExp)
			{
				pkTerrainExp->TextIndent = TextIndent;
				UBOOL bResult = pkTerrainExp->ExportText( Context, pkTerrain, Type, Ar, Warn );
			}
		}
		else
		if( Actor && ( bAllActors || Actor->IsSelected() ) )
		{
			Ar.Logf( TEXT("%sBegin Actor Class=%s Name=%s Archetype=%s'%s'") LINE_TERMINATOR, 
				appSpc(TextIndent), *Actor->GetClass()->GetName(), *Actor->GetName(),
				*Actor->GetArchetype()->GetClass()->GetName(), *Actor->GetArchetype()->GetPathName() );

			ExportObjectInner( Context, Actor, Ar, PortFlags | PPF_ExportsNotFullyQualified );

			Ar.Logf( TEXT("%sEnd Actor\r\n"), appSpc(TextIndent) );
		}
	}

	TextIndent -= 3;

	Ar.Logf(TEXT("%sEnd Level\r\n"), appSpc(TextIndent));

	TextIndent -= 3;

	// Export information about the first selected surface in the map.  Used for copying/pasting
	// information from poly to poly.
	Ar.Logf( TEXT("%sBegin Surface\r\n"), appSpc(TextIndent) );
	TCHAR TempStr[256];
	for( INT i=0; i<GWorld->GetModel()->Surfs.Num(); i++ )
	{
		FBspSurf *Poly = &GWorld->GetModel()->Surfs(i);
		if( Poly->PolyFlags&PF_Selected )
		{
			Ar.Logf( TEXT("%sTEXTURE=%s\r\n"), appSpc(TextIndent+3), *Poly->Material->GetPathName() );
			Ar.Logf( TEXT("%sBASE      %s\r\n"), appSpc(TextIndent+3), SetFVECTOR(TempStr,&(GWorld->GetModel()->Points(Poly->pBase))) );
			Ar.Logf( TEXT("%sTEXTUREU  %s\r\n"), appSpc(TextIndent+3), SetFVECTOR(TempStr,&(GWorld->GetModel()->Vectors(Poly->vTextureU))) );
			Ar.Logf( TEXT("%sTEXTUREV  %s\r\n"), appSpc(TextIndent+3), SetFVECTOR(TempStr,&(GWorld->GetModel()->Vectors(Poly->vTextureV))) );
			Ar.Logf( TEXT("%sNORMAL    %s\r\n"), appSpc(TextIndent+3), SetFVECTOR(TempStr,&(GWorld->GetModel()->Vectors(Poly->vNormal))) );
			Ar.Logf( TEXT("%sPOLYFLAGS=%d\r\n"), appSpc(TextIndent+3), Poly->PolyFlags );
			break;
		}
	}
	Ar.Logf( TEXT("%sEnd Surface\r\n"), appSpc(TextIndent) );

	Ar.Logf( TEXT("%sEnd Map\r\n"), appSpc(TextIndent) );


	return 1;
}

void ULevelExporterT3D::ExportPackageObject(FExportPackageParams& ExpPackageParams){}
void ULevelExporterT3D::ExportPackageInners(FExportPackageParams& ExpPackageParams){}

IMPLEMENT_CLASS(ULevelExporterT3D);

/*------------------------------------------------------------------------------
	ULevelExporterSTL implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void ULevelExporterSTL::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UWorld::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("STL")) );
	new(FormatDescription)FString(TEXT("Stereolithography"));
}
UBOOL ULevelExporterSTL::ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags )
{
	//@todo seamless - this needs to be world, like the t3d version above
	UWorld* World = CastChecked<UWorld>(Object);
	ULevel* Level = World->PersistentLevel;

	for( FObjectIterator It; It; ++It )
		It->ClearFlags( RF_TagImp | RF_TagExp );

	//
	// GATHER TRIANGLES
	//

	TArray<FVector> Triangles;

	// Specific actors can be exported
	for( INT iActor=0; iActor<Level->Actors.Num(); iActor++ )
	{
		ATerrain* T = Cast<ATerrain>(Level->Actors(iActor));
		if( T && ( !bSelectedOnly || T->IsSelected() ) )
		{
			for( int y = 0 ; y < T->NumVerticesY-1 ; y++ )
			{
				for( int x = 0 ; x < T->NumVerticesX-1 ; x++ )
				{
					FVector P1	= T->GetWorldVertex(x,y);
					FVector P2	= T->GetWorldVertex(x,y+1);
					FVector P3	= T->GetWorldVertex(x+1,y+1);
					FVector P4	= T->GetWorldVertex(x+1,y);

					Triangles.AddItem( P4 );
					Triangles.AddItem( P1 );
					Triangles.AddItem( P2 );

					Triangles.AddItem( P3 );
					Triangles.AddItem( P2 );
					Triangles.AddItem( P4 );
				}
			}
		}

		AStaticMeshActor* Actor = Cast<AStaticMeshActor>(Level->Actors(iActor));
		if( Actor && ( !bSelectedOnly || Actor->IsSelected() ) && Actor->StaticMeshComponent->StaticMesh )
		{
			const FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) Actor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.Lock(LOCK_READ_ONLY);
			for( INT tri = 0 ; tri < Actor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.GetElementCount() ; tri++ )
			{
				for( INT v = 2 ; v > -1 ; v-- )
				{
					FVector vtx = Actor->LocalToWorld().TransformFVector( RawTriangleData[tri].Vertices[v] );
					Triangles.AddItem( vtx );
				}
			}
			Actor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.Unlock();
		}
	}

	// Selected BSP surfaces
	for( INT i=0;i<GWorld->GetModel()->Nodes.Num();i++ )
	{
		FBspNode* Node = &GWorld->GetModel()->Nodes(i);
		if( !bSelectedOnly || GWorld->GetModel()->Surfs(Node->iSurf).PolyFlags&PF_Selected )
		{
			if( Node->NumVertices > 2 )
			{
				FVector vtx1(GWorld->GetModel()->Points(GWorld->GetModel()->Verts(Node->iVertPool+0).pVertex)),
					vtx2(GWorld->GetModel()->Points(GWorld->GetModel()->Verts(Node->iVertPool+1).pVertex)),
					vtx3;

				for( INT v = 2 ; v < Node->NumVertices ; v++ )
				{
					vtx3 = GWorld->GetModel()->Points(GWorld->GetModel()->Verts(Node->iVertPool+v).pVertex);

					Triangles.AddItem( vtx1 );
					Triangles.AddItem( vtx2 );
					Triangles.AddItem( vtx3 );

					vtx2 = vtx3;
				}
			}
		}
	}

	//
	// WRITE THE FILE
	//

	Ar.Logf( TEXT("%ssolid LevelBSP\r\n"), appSpc(TextIndent) );

	for( INT tri = 0 ; tri < Triangles.Num() ; tri += 3 )
	{
		FVector vtx[3];
		vtx[0] = Triangles(tri) * FVector(1,-1,1);
		vtx[1] = Triangles(tri+1) * FVector(1,-1,1);
		vtx[2] = Triangles(tri+2) * FVector(1,-1,1);

		FPlane Normal( vtx[0], vtx[1], vtx[2] );

		Ar.Logf( TEXT("%sfacet normal %1.6f %1.6f %1.6f\r\n"), appSpc(TextIndent+2), Normal.X, Normal.Y, Normal.Z );
		Ar.Logf( TEXT("%souter loop\r\n"), appSpc(TextIndent+4) );

		Ar.Logf( TEXT("%svertex %1.6f %1.6f %1.6f\r\n"), appSpc(TextIndent+6), vtx[0].X, vtx[0].Y, vtx[0].Z );
		Ar.Logf( TEXT("%svertex %1.6f %1.6f %1.6f\r\n"), appSpc(TextIndent+6), vtx[1].X, vtx[1].Y, vtx[1].Z );
		Ar.Logf( TEXT("%svertex %1.6f %1.6f %1.6f\r\n"), appSpc(TextIndent+6), vtx[2].X, vtx[2].Y, vtx[2].Z );

		Ar.Logf( TEXT("%sendloop\r\n"), appSpc(TextIndent+4) );
		Ar.Logf( TEXT("%sendfacet\r\n"), appSpc(TextIndent+2) );
	}

	Ar.Logf( TEXT("%sendsolid LevelBSP\r\n"), appSpc(TextIndent) );

	Triangles.Empty();

	return 1;
}
IMPLEMENT_CLASS(ULevelExporterSTL);

/*------------------------------------------------------------------------------
	ULevelExporterOBJ implementation.
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void ULevelExporterOBJ::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UWorld::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("OBJ")) );
	new(FormatDescription)FString(TEXT("Object File"));
}
static void ExportPolys( UPolys* Polys, INT &PolyNum, INT TotalPolys, FOutputDevice& Ar, FFeedbackContext* Warn, UBOOL bSelectedOnly, UModel* Model )
{
    UMaterialInterface *DefaultMaterial = GEngine->DefaultMaterial;

    UMaterialInterface *CurrentMaterial;

    CurrentMaterial = DefaultMaterial;

	// Create a list of the selected polys by finding the selected BSP Surfaces
	TArray<INT> SelectedPolys;
	if (bSelectedOnly)
	{
		// loop through the bsp surfaces
		for (INT SurfIndex = 0; SurfIndex < Model->Surfs.Num(); SurfIndex++)
		{
			FBspSurf& Surf = Model->Surfs(SurfIndex);

			// if the surface is selected, then add the poly it points to to our list of selected polys
			if ((Surf.PolyFlags & PF_Selected) && Surf.Actor)
			{
				SelectedPolys.AddItem(Surf.iBrushPoly);
			}
		}
	}

	for (INT i = 0; i < Polys->Element.Num(); i++)
	{
		Warn->StatusUpdatef( PolyNum++, TotalPolys, *LocalizeUnrealEd(TEXT("ExportingLevelToOBJ")) );

		FPoly *Poly = &Polys->Element(i);

		// if we are exporting only selected, then check to see if we found this poly to be selected
		if (bSelectedOnly && SelectedPolys.FindItemIndex(i) == INDEX_NONE)
		{
			continue;
		}

		int j;

        if (
            (!Poly->Material && (CurrentMaterial != DefaultMaterial)) ||
            (Poly->Material && (Poly->Material != CurrentMaterial))
           )
        {
            FString Material;

            CurrentMaterial = Poly->Material;

            if( CurrentMaterial )
        	    Material = FString::Printf (TEXT("usemtl %s"), *CurrentMaterial->GetName());
            else
        	    Material = FString::Printf (TEXT("usemtl DefaultMaterial"));

            Ar.Logf (TEXT ("%s\n"), *Material );
        }

		for (j = 0; j < Poly->Vertices.Num(); j++)
        {
            // Transform to Lightwave's coordinate system
			Ar.Logf (TEXT("v %f %f %f\n"), Poly->Vertices(j).X, Poly->Vertices(j).Z, Poly->Vertices(j).Y);
        }

		FVector	TextureBase = Poly->Base;

        FVector	TextureX, TextureY;

		TextureX = Poly->TextureU / (FLOAT)CurrentMaterial->GetWidth();
		TextureY = Poly->TextureV / (FLOAT)CurrentMaterial->GetHeight();

		for (j = 0; j < Poly->Vertices.Num(); j++)
        {
            // Invert the y-coordinate (Lightwave has their bitmaps upside-down from us).
    		Ar.Logf (TEXT("vt %f %f\n"),
			    (Poly->Vertices(j) - TextureBase) | TextureX, -((Poly->Vertices(j) - TextureBase) | TextureY));
        }

		Ar.Logf (TEXT("f "));

        // Reverse the winding order so Lightwave generates proper normals:
		for (j = Poly->Vertices.Num() - 1; j >= 0; j--)
			Ar.Logf (TEXT("%i/%i "), (j - Poly->Vertices.Num()), (j - Poly->Vertices.Num()));

		Ar.Logf (TEXT("\n"));
	}
}

UBOOL ULevelExporterOBJ::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	UWorld* World = CastChecked<UWorld>(Object);

	GEditor->bspBuildFPolys(World->GetModel(), 0, 0 );
	UPolys* Polys = World->GetModel()->Polys;

    UMaterialInterface *DefaultMaterial = GEngine->DefaultMaterial;

    UMaterialInterface *CurrentMaterial;

    INT i, j, TotalPolys;
    INT PolyNum;

    // Calculate the total number of polygons to export:

    PolyNum = 0;
    TotalPolys = Polys->Element.Num();

	for( FSelectedActorIterator It; It; ++It )
	{
		AActor* Actor = *It;

		// only export selected actors if the flag is set
        if( !Actor || (bSelectedOnly && !Actor->IsSelected()))
		{
            continue;
		}
        
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>( Actor );
        if( !StaticMeshActor || !StaticMeshActor->StaticMeshComponent || !StaticMeshActor->StaticMeshComponent->StaticMesh )
		{
            continue;
		}
         
        UStaticMesh* StaticMesh = StaticMeshActor->StaticMeshComponent->StaticMesh;

        TotalPolys += StaticMesh->LODModels(0).RawTriangles.GetElementCount();
    }

    // Export the BSP

	Ar.Logf (TEXT("# OBJ File Generated by UnrealEd\n"));

	Ar.Logf (TEXT("o PersistentLevel\n"));
    Ar.Logf (TEXT("g BSP\n") );

    ExportPolys( Polys, PolyNum, TotalPolys, Ar, Warn, bSelectedOnly, World->GetModel() );

    // Export the static meshes

    CurrentMaterial = DefaultMaterial;

	for( FSelectedActorIterator It; It; ++It )
	{
		AActor* Actor = *It;

		// only export selected actors if the flag is set
        if( !Actor || (bSelectedOnly && !Actor->IsSelected()))
		{
            continue;
		}
        
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>( Actor );
        if( !StaticMeshActor || !StaticMeshActor->StaticMeshComponent || !StaticMeshActor->StaticMeshComponent->StaticMesh )
		{
            continue;
		}

        FMatrix LocalToWorld = Actor->LocalToWorld();
         
    	Ar.Logf (TEXT("g %s\n"), *Actor->GetName() );

        const FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) StaticMeshActor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.Lock(LOCK_READ_ONLY);

	    for (i = 0; i < StaticMeshActor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.GetElementCount(); i++)
	    {
			Warn->StatusUpdatef( PolyNum++, TotalPolys, *LocalizeUnrealEd(TEXT("ExportingLevelToOBJ")) );

            const FStaticMeshTriangle &Triangle = RawTriangleData[i];

            if ( CurrentMaterial != DefaultMaterial )
            {
                FString Material;

                CurrentMaterial = StaticMeshActor->StaticMeshComponent->GetMaterial(Triangle.MaterialIndex); // sjs

                if( CurrentMaterial )
				{
        	        Material = FString::Printf (TEXT("usemtl %s"), *CurrentMaterial->GetName());
				}
                else
				{
        	        Material = FString::Printf (TEXT("usemtl DefaultMaterial"));
				}

                Ar.Logf (TEXT ("%s\n"), *Material);
            }

		    for( j = 0; j < ARRAY_COUNT(Triangle.Vertices); j++ )
            {
                FVector V = Triangle.Vertices[j];

                V = LocalToWorld.TransformFVector( V );

                // Transform to Lightwave's coordinate system
			    Ar.Logf( TEXT("v %f %f %f\n"), V.X, V.Z, V.Y );
            }

		    for( j = 0; j < ARRAY_COUNT(Triangle.Vertices); j++ )
            {
                // Invert the y-coordinate (Lightwave has their bitmaps upside-down from us).
    		    Ar.Logf( TEXT("vt %f %f\n"), Triangle.UVs[j][0].X, -Triangle.UVs[j][0].Y );
            }

		    Ar.Logf (TEXT("f "));

		    for( j = 0; j < ARRAY_COUNT(Triangle.Vertices); j++ )
			{
			    Ar.Logf (TEXT("%i/%i "), (j - ARRAY_COUNT(Triangle.Vertices)), (j - ARRAY_COUNT(Triangle.Vertices)));
			}

		    Ar.Logf (TEXT("\n"));
	    }

		StaticMeshActor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.Unlock();
	}

	GWorld->GetModel()->Polys->Element.Empty();

	Ar.Logf (TEXT("# dElaernU yb detareneG eliF JBO\n"));
	return 1;
}

IMPLEMENT_CLASS(ULevelExporterOBJ);

/*------------------------------------------------------------------------------
	UPackageExporterT3D implementation.
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UPackageExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UPackage::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3DPKG")) );
	new(FormatDescription)FString(TEXT("Unreal package text"));
}

UBOOL UPackageExporterT3D::ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags )
{
	UPackage* Package = Cast<UPackage>(Object);
	// this is the top level in the .t3d file
	if (Package == NULL)
	{
		return FALSE;
	}

	if (Package != Package->GetOutermost())
	{
		warnf(TEXT("PackageExporterT3D: Only supports exporting TopLevelPackages!"));
		return FALSE;
	}

	Ar.Logf( TEXT("%sBegin TopLevelPackage Class=%s Name=%s Archetype=%s'%s'") LINE_TERMINATOR, 
		appSpc(TextIndent), 
		*(Object->GetClass()->GetName()), *(Object->GetName()),
		*(Object->GetArchetype()->GetClass()->GetName()), *(Object->GetArchetype()->GetPathName()));
		TextIndent += 3;

		ExportProperties(Context, Ar, Object->GetClass(), (BYTE*)Object, TextIndent, Object->GetArchetype()->GetClass(), (BYTE*)Object->GetArchetype(), Object, PortFlags);


		TextIndent -= 3;
	Ar.Logf( TEXT("%sEnd TopLevelPackage\r\n"), appSpc(TextIndent) );

	// We really only care about the following:
	//	Packages (really groups under the main package)
	//	Materials
	//	MaterialInstanceConstants
	//	MaterialInstanceTimeVarying
	//	Textures
	FExportPackageParams ExpPackageParams;
	ExpPackageParams.RootMapPackageName = Package->GetName();
	ExpPackageParams.Context = Context;
	ExpPackageParams.InPackage = Package;
	ExpPackageParams.Type = Type;
	ExpPackageParams.Ar = &Ar;
	ExpPackageParams.Warn = Warn;
	ExpPackageParams.PortFlags = PortFlags;
	ExpPackageParams.InObject = NULL;
	ExportPackageInners(ExpPackageParams);

	return 1;
}

void UPackageExporterT3D::ExportPackageObject(FExportPackageParams& ExpPackageParams)
{
	UObject* ExpObject = ExpPackageParams.InObject;
	FOutputDevice& Ar = *(ExpPackageParams.Ar);

	FString PackageName = ExpPackageParams.InPackage ? *(ExpPackageParams.InPackage->GetName()) : TEXT("***NULL***");
	if (ExpObject->IsA(UPackage::StaticClass()))
	{
		Ar.Logf( TEXT("%sBegin Package ParentPackage=%s Class=%s Name=%s Archetype=%s'%s'") LINE_TERMINATOR, 
			appSpc(TextIndent), *PackageName,
			*(ExpObject->GetClass()->GetName()), *(ExpObject->GetName()),
			*(ExpObject->GetArchetype()->GetClass()->GetName()), *(ExpObject->GetArchetype()->GetPathName()));

//			ExportObjectInner( ExpPackageParams.Context, ExpObject, Ar, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified, TRUE );
//			UExporter::ExportToOutputDevice(ExpPackageParams.Context, ExpObject, NULL, Ar, ExpPackageParams.Type, TextIndent+3, ExpPackageParams.PortFlags);
			// export the object's properties - don't want to use ExportObjectInner or ExportToOutputDevice as it will 
			// write out all of the contained items!
			TextIndent += 3;
			ExportProperties(ExpPackageParams.Context, Ar, ExpObject->GetClass(), (BYTE*)ExpObject, TextIndent, ExpObject->GetArchetype()->GetClass(), (BYTE*)ExpObject->GetArchetype(), ExpObject, ExpPackageParams.PortFlags);
			TextIndent -= 3;

		Ar.Logf( TEXT("%sEnd Package\r\n"), appSpc(TextIndent) );
	}
	else
	if (
		(ExpObject->IsA(UMaterialInstanceConstant::StaticClass())) || 
		(ExpObject->IsA(UMaterialInstanceTimeVarying::StaticClass()) ||
		(ExpObject->IsA(UPhysicalMaterial::StaticClass())))
		)
	{
		Ar.Logf( TEXT("%sBegin PackageObject ParentPackage=%s Class=%s Name=%s Archetype=%s'%s'") LINE_TERMINATOR, 
			appSpc(TextIndent), *PackageName,
			*(ExpObject->GetClass()->GetName()), *(ExpObject->GetName()),
			*(ExpObject->GetArchetype()->GetClass()->GetName()), *(ExpObject->GetArchetype()->GetPathName()));

			ExportObjectInner( ExpPackageParams.Context, ExpObject, Ar, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified, TRUE );

		Ar.Logf( TEXT("%sEnd PackageObject\r\n"), appSpc(TextIndent) );
	}
	else
	if (ExpObject->IsA(UMaterial::StaticClass()))
	{
		UMaterialExporterT3D* MatExporter = NULL;
		for( TObjectIterator<UClass> It ; It ; ++It )
		{
			if (*It == UMaterialExporterT3D::StaticClass())
			{
				MatExporter = ConstructObject<UMaterialExporterT3D>(*It);
				break;
			}
		}

		if (MatExporter)
		{
			Ar.Logf( TEXT("%sBegin PackageMaterial ParentPackage=%s") LINE_TERMINATOR, appSpc(TextIndent), *PackageName);
			TextIndent += 3;

				MatExporter->TextIndent = TextIndent;
				MatExporter->ExportText(ExpPackageParams.Context, ExpObject, TEXT("T3D"), Ar, 
					ExpPackageParams.Warn, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified);

			TextIndent -= 3;
			Ar.Logf( TEXT("%sEnd PackageMaterial\r\n"), appSpc(TextIndent) );
		}
		else
		{
			Ar.Logf( TEXT("%sBegin PackageObject ParentPackage=%s Class=%s Name=%s Archetype=%s'%s'") LINE_TERMINATOR, 
				appSpc(TextIndent), *PackageName,
				*(ExpObject->GetClass()->GetName()), *(ExpObject->GetName()),
				*(ExpObject->GetArchetype()->GetClass()->GetName()), *(ExpObject->GetArchetype()->GetPathName()));

				ExportObjectInner( ExpPackageParams.Context, ExpObject, Ar, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified, TRUE );

			Ar.Logf( TEXT("%sEnd PackageObject\r\n"), appSpc(TextIndent) );
		}
	}
	else
	if (ExpObject->IsA(UStaticMesh::StaticClass()))
	{		
		UStaticMeshExporterT3D* SMExporter = NULL;
		for( TObjectIterator<UClass> It ; It ; ++It )
		{
			if (*It == UStaticMeshExporterT3D::StaticClass())
			{
				SMExporter = ConstructObject<UStaticMeshExporterT3D>(*It);
				break;
			}
		}

		if (SMExporter)
		{
			Ar.Logf( TEXT("%sBegin PackageStaticMesh ParentPackage=%s") LINE_TERMINATOR, appSpc(TextIndent), *PackageName);
			TextIndent += 3;

				SMExporter->TextIndent = TextIndent;
				SMExporter->ExportText(ExpPackageParams.Context, ExpObject, TEXT("T3D"), Ar, 
					ExpPackageParams.Warn, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified);

			TextIndent -= 3;
			Ar.Logf( TEXT("%sEnd PackageStaticMesh\r\n"), appSpc(TextIndent) );
		}
		else
		{
			Ar.Logf( TEXT("%sBegin PackageObject ParentPackage=%s Class=%s Name=%s Archetype=%s'%s'") LINE_TERMINATOR, 
				appSpc(TextIndent), *PackageName,
				*(ExpObject->GetClass()->GetName()), *(ExpObject->GetName()),
				*(ExpObject->GetArchetype()->GetClass()->GetName()), *(ExpObject->GetArchetype()->GetPathName()));

				ExportObjectInner( ExpPackageParams.Context, ExpObject, Ar, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified, TRUE );

			Ar.Logf( TEXT("%sEnd PackageObject\r\n"), appSpc(TextIndent) );
		}
	}
	else
	if (
		((ExpObject->IsA(UTexture2D::StaticClass())) ||
		(ExpObject->IsA(UTextureCube::StaticClass()))) &&
		(!ExpObject->IsA(ULightMapTexture2D::StaticClass()))
		)
	{
		UTextureExporterT3D* TextureExporter = NULL;
		for( TObjectIterator<UClass> It ; It ; ++It )
		{
			if (*It == UTextureExporterT3D::StaticClass())
			{
				TextureExporter = ConstructObject<UTextureExporterT3D>(*It);
				break;
			}
		}

		if (TextureExporter)
		{
			Ar.Logf( TEXT("%sBegin PackageTexture ParentPackage=%s") LINE_TERMINATOR, appSpc(TextIndent), *PackageName);
			TextIndent += 3;

				TextureExporter->TextIndent = TextIndent;
				TextureExporter->ExportText(ExpPackageParams.Context, ExpObject, TEXT("T3D"), Ar, 
					ExpPackageParams.Warn, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified);

			TextIndent -= 3;
			Ar.Logf( TEXT("%sEnd PackageTexture\r\n"), appSpc(TextIndent) );
		}
		else
		{
			Ar.Logf( TEXT("%sBegin PackageObject ParentPackage=%s Class=%s Name=%s Archetype=%s'%s'") LINE_TERMINATOR, 
				appSpc(TextIndent), *PackageName,
				*(ExpObject->GetClass()->GetName()), *(ExpObject->GetName()),
				*(ExpObject->GetArchetype()->GetClass()->GetName()), *(ExpObject->GetArchetype()->GetPathName()));

				ExportObjectInner( ExpPackageParams.Context, ExpObject, Ar, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified, TRUE );

			Ar.Logf( TEXT("%sEnd PackageObject\r\n"), appSpc(TextIndent) );
		}
	}
	else
	{
#if 0
		// We don't want to just export any object at this particular time...
		Ar.Logf( TEXT("%sBegin PackageObject ParentPackage=%s Class=%s Name=%s Archetype=%s'%s'") LINE_TERMINATOR, 
			appSpc(TextIndent), *PackageName,
			*(ExpObject->GetClass()->GetName()), *(ExpObject->GetName()),
			*(ExpObject->GetArchetype()->GetClass()->GetName()), *(ExpObject->GetArchetype()->GetPathName()));

			ExportObjectInner( ExpPackageParams.Context, ExpObject, Ar, ExpPackageParams.PortFlags | PPF_ExportsNotFullyQualified, TRUE );

		Ar.Logf( TEXT("%sEnd PackageObject\r\n"), appSpc(TextIndent) );
#else
		// To see what objects are getting skipped, uncomment this line
//		warnf(TEXT("Skipping unsupported object: %s - %s"), 
//			*(ExpObject->GetClass()->GetName()), *(ExpObject->GetPathName()));
#endif
	}
}

void UPackageExporterT3D::ExportPackageInners(FExportPackageParams& ExpPackageParams)
{
	const TArray<UObject*>* Inners = ExpPackageParams.Context->GetObjectInners(ExpPackageParams.InPackage);
	if (Inners)
	{
		for (INT InnerIndex = 0; InnerIndex < Inners->Num(); InnerIndex++)
		{
			UObject* InnerObj = (*Inners)(InnerIndex);
			if (InnerObj)
			{
				FExportPackageParams NewParams(ExpPackageParams);
				NewParams.InObject = InnerObj;
				ExportPackageObject(NewParams);
				UPackage* InnerPackage = Cast<UPackage>(InnerObj);
				if (InnerPackage)
				{
//					TextIndent += 3;
					FExportPackageParams NewParams(ExpPackageParams);
					NewParams.InPackage = InnerPackage;
					ExportPackageInners(NewParams);
//					TextIndent -= 3;
				}
			}
		}
	}
}

IMPLEMENT_CLASS(UPackageExporterT3D);

/*------------------------------------------------------------------------------
	UPolysExporterOBJ implementation.
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UPolysExporterOBJ::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UPolys::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("OBJ")) );
	new(FormatDescription)FString(TEXT("Object File"));
}
UBOOL UPolysExporterOBJ::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
    UPolys* Polys = CastChecked<UPolys> (Object);

    INT PolyNum = 0;
    INT TotalPolys = Polys->Element.Num();

	Ar.Logf (TEXT("# OBJ File Generated by UnrealEd\n"));

    ExportPolys( Polys, PolyNum, TotalPolys, Ar, Warn, false, NULL );

	Ar.Logf (TEXT("# dElaernU yb detareneG eliF JBO\n"));

	return 1;
}

IMPLEMENT_CLASS(UPolysExporterOBJ);

/*------------------------------------------------------------------------------
	USequenceExporterT3D implementation.
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USequenceExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = USequence::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3D")) );
	new(FormatDescription)FString(TEXT("Unreal sequence text"));
	new(FormatExtension)FString(TEXT("COPY"));
	new(FormatDescription)FString(TEXT("Unreal sequence text"));
}
UBOOL USequenceExporterT3D::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	USequence* Sequence = CastChecked<USequence>( Object );

	UBOOL bAsSingle = (appStricmp(Type,TEXT("T3D"))==0);

	// If exporting everything - just pass in the sequence.
	if(bAsSingle)
	{
		Ar.Logf( TEXT("%sBegin Object Class=%s Name=%s Archetype=%s'%s'\r\n"), appSpc(TextIndent), *Sequence->GetClass()->GetName(), *Sequence->GetName(), *Sequence->GetArchetype()->GetClass()->GetName(), *Sequence->GetArchetype()->GetPathName() );
			ExportObjectInner( Context, Sequence, Ar, PortFlags );
		Ar.Logf( TEXT("%sEnd Object\r\n"), appSpc(TextIndent) );
	}
	// If we want only a selection, iterate over to find the SequenceObjects we want.
	else
	{
		for(INT i=0; i<Sequence->SequenceObjects.Num(); i++)
		{
			USequenceObject* SeqObj = Sequence->SequenceObjects(i);
			if( SeqObj && SeqObj->IsSelected() )
			{
				Ar.Logf( TEXT("%sBegin Object Class=%s Name=%s Archetype=%s'%s'\r\n"), appSpc(TextIndent), *SeqObj->GetClass()->GetName(), *SeqObj->GetName(), *SeqObj->GetArchetype()->GetClass()->GetName(), *SeqObj->GetArchetype()->GetPathName() );
					// when we export sequences in this sequnce, we don't want to count on selection, we want all objects to be exported
					// and PPF_Copy will only exported selected objects
					ExportObjectInner( Context, SeqObj, Ar, PortFlags & ~PPF_Copy );
				Ar.Logf( TEXT("%sEnd Object\r\n"), appSpc(TextIndent) );
			}
		}
	}

	return true;
}

IMPLEMENT_CLASS(USequenceExporterT3D);

/*------------------------------------------------------------------------------
	UStaticMeshExporterOBJ implementation.
------------------------------------------------------------------------------*/
/**
* Initializes property values for intrinsic classes.  It is called immediately after the class default object
* is initialized against its archetype, but before any objects of this class are created.
*/
void UStaticMeshExporterOBJ::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UStaticMesh::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("OBJ")) );
	new(FormatDescription)FString(TEXT("Object File"));
}
UBOOL UStaticMeshExporterOBJ::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	UStaticMesh* StaticMesh = CastChecked<UStaticMesh>( Object );

	// Create a new filename for the lightmap coordinate OBJ file (by adding "_UV1" to the end of the filename)
	FString Filename;
	Filename = UExporter::CurrentFilename.Left( UExporter::CurrentFilename.Len() - 4 );
	Filename += "_UV1.";
	Filename += UExporter::CurrentFilename.Right( 3 );

	// Open a second archive here so we can export lightmap coordinates at the same time we export the regular mesh
	FArchive* UV1File = GFileManager->CreateFileWriter( *Filename, FILEWRITE_AllowRead );

	TArray<FVector> Verts;				// The verts in the mesh
	TArray<FVector2D> UVs;				// UV coords from channel 0
	TArray<FVector2D> UVLMs;			// Lightmap UVs from channel 1
	TArray<DWORD> SmoothingMasks;		// Complete collection of the smoothing groups from all triangles
	TArray<DWORD> UniqueSmoothingMasks;	// Collection of the unique smoothing groups (used when writing out the face info into the OBJ file so we can group by smoothing group)

	UV1File->Logf( TEXT("# UnrealEd OBJ exporter\r\n") );
	Ar.Log( TEXT("# UnrealEd OBJ exporter\r\n") );

	// Collect all the data about the mesh

	const FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) StaticMesh->LODModels(0).RawTriangles.Lock(LOCK_READ_ONLY);
	for( INT tri = 0 ; tri < StaticMesh->LODModels(0).RawTriangles.GetElementCount() ; tri++ )
	{
		// Vertices
		Verts.AddItem( RawTriangleData[tri].Vertices[0] );
		Verts.AddItem( RawTriangleData[tri].Vertices[1] );
		Verts.AddItem( RawTriangleData[tri].Vertices[2] );

		// UVs from channel 0
		UVs.AddItem( RawTriangleData[tri].UVs[0][0] );
		UVs.AddItem( RawTriangleData[tri].UVs[1][0] );
		UVs.AddItem( RawTriangleData[tri].UVs[2][0] );

		// UVs from channel 1 (lightmap coords)
		UVLMs.AddItem( RawTriangleData[tri].UVs[0][1] );
		UVLMs.AddItem( RawTriangleData[tri].UVs[1][1] );
		UVLMs.AddItem( RawTriangleData[tri].UVs[2][1] );

		// Smoothing groups
		SmoothingMasks.AddItem( RawTriangleData[tri].SmoothingMask );

		// Unique smoothing groups
		UniqueSmoothingMasks.AddUniqueItem( RawTriangleData[tri].SmoothingMask );
	}
	StaticMesh->LODModels(0).RawTriangles.Unlock();

	// Write out the vertex data

	UV1File->Logf( TEXT("\r\n") );
	Ar.Log( TEXT("\r\n") );
	for( INT v = 0 ; v < Verts.Num() ; ++v )
	{
		// Transform to Lightwave's coordinate system
		UV1File->Logf( TEXT("v %f %f %f\r\n"), Verts(v).X, Verts(v).Z, Verts(v).Y );
		Ar.Logf( TEXT("v %f %f %f\r\n"), Verts(v).X, Verts(v).Z, Verts(v).Y );
	}

	// Write out the UV data (the lightmap file differs here in that it writes from the UVLMs array instead of UVs)

	UV1File->Logf( TEXT("\r\n") );
	Ar.Log( TEXT("\r\n") );
	for( INT uv = 0 ; uv < UVs.Num() ; ++uv )
	{
		// Invert the y-coordinate (Lightwave has their bitmaps upside-down from us).
		UV1File->Logf( TEXT("vt %f %f\r\n"), UVLMs(uv).X, UVLMs(uv).Y * -1 );
		Ar.Logf( TEXT("vt %f %f\r\n"), UVs(uv).X, UVs(uv).Y * -1 );
	}

	// Write object header

	UV1File->Logf( TEXT("\r\n") );
	Ar.Log( TEXT("\r\n") );
	UV1File->Logf( TEXT("g UnrealEdObject\r\n") );
	Ar.Log( TEXT("g UnrealEdObject\r\n") );
	UV1File->Logf( TEXT("\r\n") );
	Ar.Log( TEXT("\r\n") );

	// Write out the face windings, sectioned by unique smoothing groups

	INT SmoothingGroup = 0;

	for( INT sm = 0 ; sm < UniqueSmoothingMasks.Num() ; ++sm )
	{
		UV1File->Logf( TEXT("s %i\r\n"), SmoothingGroup );
		Ar.Logf( TEXT("s %i\r\n"), SmoothingGroup );
		SmoothingGroup++;

		for( INT tri = 0 ; tri < StaticMesh->LODModels(0).RawTriangles.GetElementCount() ; tri++ )
		{
			if( SmoothingMasks(tri) == UniqueSmoothingMasks(sm)  )
			{
				int idx = 1 + (tri * 3);

				UV1File->Logf( TEXT("f %d/%d %d/%d %d/%d\r\n"), idx, idx, idx+1, idx+1, idx+2, idx+2 );
				Ar.Logf( TEXT("f %d/%d %d/%d %d/%d\r\n"), idx, idx, idx+1, idx+1, idx+2, idx+2 );
			}
		}
	}

	// Write out footer

	UV1File->Logf( TEXT("\r\n") );
	Ar.Log( TEXT("\r\n") );
	UV1File->Logf( TEXT("g\r\n") );
	Ar.Log( TEXT("g\r\n") );

	// Clean up and finish

	delete UV1File;

	return TRUE;
}

IMPLEMENT_CLASS(UStaticMeshExporterOBJ);

/*-----------------------------------------------------------------------------
	'Extended' T3D exporters...
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UExporterT3DX)

void UExporterT3DX::ExportObjectStructProperty(const FString& PropertyName, const FString& ScriptStructName, BYTE* DataValue, UObject* ParentObject, FOutputDevice& Ar, DWORD PortFlags)
{
	UScriptStruct* TheStruct = FindField<UScriptStruct>(UObject::StaticClass(), *ScriptStructName);
	check(TheStruct);
	FString DataString;
	UStructProperty_ExportTextItem(TheStruct, DataString, DataValue, NULL, ParentObject, PortFlags);
	Ar.Logf(TEXT("%s%s=%s") LINE_TERMINATOR, appSpc(TextIndent), *PropertyName, *DataString);
}

void UExporterT3DX::ExportBooleanProperty(const FString& PropertyName, UBOOL BooleanValue, FOutputDevice& Ar, DWORD PortFlags)
{
	TCHAR* Temp =	(TCHAR*) ((PortFlags & PPF_Localized)
		? (BooleanValue ? GTrue  : GFalse ) : (BooleanValue ? TEXT("True") : TEXT("False")));
	Ar.Logf(TEXT("%s%s=%s") LINE_TERMINATOR, appSpc(TextIndent), *PropertyName, Temp);
}

void UExporterT3DX::ExportFloatProperty(const FString& PropertyName, FLOAT FloatValue, FOutputDevice& Ar, DWORD PortFlags)
{
	Ar.Logf(TEXT("%s%s=%f") LINE_TERMINATOR, appSpc(TextIndent), *PropertyName, FloatValue);
}

void UExporterT3DX::ExportIntProperty(const FString& PropertyName, INT IntValue, FOutputDevice& Ar, DWORD PortFlags)
{
	Ar.Logf(TEXT("%s%s=%d") LINE_TERMINATOR, appSpc(TextIndent), *PropertyName, IntValue);
}

void UExporterT3DX::ExportStringProperty(const FString& PropertyName, const FString& StringValue, FOutputDevice& Ar, DWORD PortFlags)
{
	Ar.Logf(TEXT("%s%s=%s") LINE_TERMINATOR, appSpc(TextIndent), *PropertyName, *StringValue);
}

void UExporterT3DX::ExportObjectProperty(const FString& PropertyName, const UObject* InExportingObject, const UObject* InObject, FOutputDevice& Ar, DWORD PortFlags)
{
	if (InObject)
	{
		UBOOL bExportFullObjectName = TRUE;
		UObject* InObjectOuter = InObject->GetOuter();
		while (InObjectOuter)
		{
			if (InObjectOuter == InExportingObject)
			{
				bExportFullObjectName = FALSE;
				InObjectOuter = NULL;
			}
			else
			{
				InObjectOuter = InObjectOuter->GetOuter();
			}
		}

		Ar.Logf(TEXT("%s%s=%s'%s'") LINE_TERMINATOR, appSpc(TextIndent), *PropertyName,
			*(InObject->GetClass()->GetName()), 
			bExportFullObjectName ? *(InObject->GetPathName()) : *(InObject->GetName()));
	}
	else
	{
		Ar.Logf(TEXT("%s%s=") LINE_TERMINATOR, appSpc(TextIndent), *PropertyName);
	}
}

void UExporterT3DX::ExportUntypedBulkData(FUntypedBulkData& BulkData, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	Ar.Logf(TEXT("%sBegin UntypedBulkData") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

	/** Number of elements in bulk data array																			*/
	ExportIntProperty(TEXT("ElementCount"), BulkData.GetElementCount(), Ar, PortFlags);
	/** Serialized flags for bulk data																					*/
	ExportIntProperty(TEXT("ElementSize"), BulkData.GetElementSize(), Ar, PortFlags);
	/** The bulk data... */
	INT Size = BulkData.GetBulkDataSize();
	BYTE* BulkDataPointer = (BYTE*)(BulkData.Lock(LOCK_READ_ONLY));
	ExportBinaryBlob(Size, BulkDataPointer, Ar, PortFlags);
	BulkData.Unlock();

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd UntypedBulkData") LINE_TERMINATOR, appSpc(TextIndent));
}

void UExporterT3DX::ExportBinaryBlob(INT BlobSize, BYTE* BlobData, FOutputDevice& Ar, DWORD PortFlags)
{
	Ar.Logf(TEXT("%sBegin BinaryBlob") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

	ExportIntProperty(TEXT("Size"), BlobSize, Ar, PortFlags);
	
	Ar.Logf(TEXT("%sBegin Binary") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

	FString OutputString;
	TArray<TCHAR>& ResultArray = OutputString.GetCharArray();
	INT Slack = 32 * 4 + TextIndent + 5;
	ResultArray.Empty(Slack);
	TCHAR TempDigits[12];
	UBOOL bOutputFinalLine = FALSE;
	for (INT ByteIndex = 0; ByteIndex < BlobSize; ByteIndex++)
	{
		bOutputFinalLine = TRUE;
		appSprintf(TempDigits, TEXT("%02x,"), BlobData[ByteIndex]);
		OutputString += TempDigits;

		if (((ByteIndex + 1) % 32) == 0)
		{
			Ar.Logf(TEXT("%s%s") LINE_TERMINATOR, appSpc(TextIndent), *OutputString);
			ResultArray.Empty(Slack);
			bOutputFinalLine = FALSE;
		}
	}
	if (bOutputFinalLine)
	{
		Ar.Logf(TEXT("%s%s") LINE_TERMINATOR, appSpc(TextIndent), *OutputString);
	}

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd Binary") LINE_TERMINATOR, appSpc(TextIndent));

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd BinaryBlob") LINE_TERMINATOR, appSpc(TextIndent));
}

/**
* Initializes property values for intrinsic classes.  It is called immediately after the class default object
* is initialized against its archetype, but before any objects of this class are created.
*/
void UMaterialExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UMaterial::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3DMAT")) );
	new(FormatDescription)FString(TEXT("Unreal material text"));
}

//*** NOTE: This code assumes that any expressions within the Material will have
//			the material as their outer!
//***
UBOOL UMaterialExporterT3D::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	UMaterial* MaterialObj = CastChecked<UMaterial>(Object);

	// Remove all the Expression for its inner list!
	const TArray<UObject*>* MaterialInners = Context->GetObjectInners(MaterialObj);

	Ar.Logf( TEXT("%sBegin Material Class=%s Name=%s ObjName=%s Archetype=%s'%s'") LINE_TERMINATOR,
		appSpc(TextIndent), *Object->GetClass()->GetName(), *Object->GetName(), *Object->GetName(),
		*Object->GetArchetype()->GetClass()->GetName(), *Object->GetArchetype()->GetPathName() );

//		ExportObjectInner( Context, Object, Ar, PortFlags);

		TextIndent += 3;
		Ar.Logf(TEXT("%sBegin MaterialData") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent += 3;

		/** Versioning system... */
		Ar.Logf(TEXT("%sVersion=%d.%d") LINE_TERMINATOR, appSpc(TextIndent), VersionMax, VersionMin);

		// Export out the expression lists first as they will have to be created before setting the material properties!
		Ar.Logf(TEXT("%sBegin ExpressionObjectList") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent += 3;

			if (MaterialInners)
			{
				for (INT InnerIndex = 0; InnerIndex < MaterialInners->Num(); InnerIndex++)
				{
					UObject* InnerObj = (*MaterialInners)(InnerIndex);
					UMaterialExpression* MatExp = Cast<UMaterialExpression>(InnerObj);
					if (MatExp)
					{
						// export the object
						UExporter::ExportToOutputDevice(Context, MatExp, NULL, Ar, (PortFlags & PPF_Copy) ? TEXT("Copy") : TEXT("T3D"), TextIndent, PortFlags);
						// don't reexport below in ExportProperties
						MatExp->SetFlags(RF_TagImp);
					}
					else
					{
						warnf(TEXT("Non-MaterialExpression object in Material inner object list: %s (Mat %s)"), 
							*(InnerObj->GetName()), *(MaterialObj->GetName()));
					}
				}
			}
		TextIndent -= 3;
		Ar.Logf(TEXT("%sEnd ExpressionObjectList") LINE_TERMINATOR, appSpc(TextIndent));

		// Export the properties of the static mesh...
		UScriptStruct* ColorMaterialInputStruct = FindField<UScriptStruct>(UMaterial::StaticClass(), TEXT("ColorMaterialInput"));
		check(ColorMaterialInputStruct);
		UScriptStruct* ScalarMaterialInputStruct = FindField<UScriptStruct>(UMaterial::StaticClass(), TEXT("ScalarMaterialInput"));
		check(ScalarMaterialInputStruct);
		UScriptStruct* VectorMaterialInputStruct = FindField<UScriptStruct>(UMaterial::StaticClass(), TEXT("VectorMaterialInput"));
		check(VectorMaterialInputStruct);
		UScriptStruct* Vector2MaterialInputStruct = FindField<UScriptStruct>(UMaterial::StaticClass(), TEXT("Vector2MaterialInput"));
		check(Vector2MaterialInputStruct);
		FString ValueString;

		ExportObjectProperty(TEXT("PhysMaterial"), MaterialObj, MaterialObj->PhysMaterial, Ar, PortFlags);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ColorMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->DiffuseColor), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sDiffuseColor=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ScalarMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->DiffusePower), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sDiffusePower=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ColorMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->SpecularColor), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sSpecularColor=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ScalarMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->SpecularPower), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sSpecularPower=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(VectorMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->Normal), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sNormal=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ColorMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->EmissiveColor), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sEmissiveColor=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ScalarMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->Opacity), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sOpacity=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ScalarMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->OpacityMask), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sOpacityMask=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ExportFloatProperty(TEXT("OpacityMaskClipValue"), MaterialObj->OpacityMaskClipValue, Ar, PortFlags);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(Vector2MaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->Distortion), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sDistortion=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ExportStringProperty(TEXT("BlendMode"), UMaterial::GetBlendModeString((EBlendMode)(MaterialObj->BlendMode)), Ar, PortFlags);
		ExportStringProperty(TEXT("LightingModel"), UMaterial::GetMaterialLightingModelString((EMaterialLightingModel)(MaterialObj->LightingModel)), Ar, PortFlags);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ColorMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->CustomLighting), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sCustomLighting=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ScalarMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->TwoSidedLightingMask), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sTwoSidedLightingMask=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ValueString = TEXT("");
		UStructProperty_ExportTextItem(ColorMaterialInputStruct, ValueString, (BYTE*)&(MaterialObj->TwoSidedLightingColor), NULL, MaterialObj, PortFlags);
		Ar.Logf(TEXT("%sTwoSidedLightingColor=%s") LINE_TERMINATOR, appSpc(TextIndent), *ValueString);
		ExportBooleanProperty(TEXT("TwoSided"), MaterialObj->TwoSided, Ar, PortFlags);
		ExportBooleanProperty(TEXT("DisableDepthTest"), MaterialObj->bDisableDepthTest, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedAsLightFunction"), MaterialObj->bUsedAsLightFunction, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithFogVolumes"), MaterialObj->bUsedWithFogVolumes, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedAsSpecialEngineMaterial"), MaterialObj->bUsedAsSpecialEngineMaterial, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithSkeletalMesh"), MaterialObj->bUsedWithSkeletalMesh, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithFracturedMeshes"), MaterialObj->bUsedWithFracturedMeshes, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithParticleSprites"), MaterialObj->bUsedWithParticleSprites, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithBeamTrails"), MaterialObj->bUsedWithBeamTrails, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithParticleSubUV"), MaterialObj->bUsedWithParticleSubUV, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithFoliage"), MaterialObj->bUsedWithFoliage, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithSpeedTree"), MaterialObj->bUsedWithSpeedTree, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithStaticLighting"), MaterialObj->bUsedWithStaticLighting, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithLensFlare"), MaterialObj->bUsedWithLensFlare, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithGammaCorrection"), MaterialObj->bUsedWithGammaCorrection, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithInstancedMeshParticles"), MaterialObj->bUsedWithInstancedMeshParticles, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithFluidSurfaces"), MaterialObj->bUsedWithFluidSurfaces, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithDecals"), MaterialObj->bUsedWithDecals, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedWithMaterialEffect"), MaterialObj->bUsedWithMaterialEffect, Ar, PortFlags);
		ExportBooleanProperty(TEXT("Wireframe"), MaterialObj->Wireframe, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bIsFallbackMaterial"), MaterialObj->bIsFallbackMaterial, Ar, PortFlags);
		ExportObjectProperty(TEXT("FallbackMaterial"), MaterialObj, MaterialObj->FallbackMaterial, Ar, PortFlags);
		ExportIntProperty(TEXT("EditorX"), MaterialObj->EditorX, Ar, PortFlags);
		ExportIntProperty(TEXT("EditorY"), MaterialObj->EditorY, Ar, PortFlags);
		ExportIntProperty(TEXT("EditorPitch"), MaterialObj->EditorPitch, Ar, PortFlags);
		ExportIntProperty(TEXT("EditorYaw"), MaterialObj->EditorYaw, Ar, PortFlags);

		Ar.Logf(TEXT("%sBegin ExpressionList") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent += 3;

			for (INT ExpIndex = 0; ExpIndex < MaterialObj->Expressions.Num(); ExpIndex++)
			{
				UMaterialExpression* MatExp = MaterialObj->Expressions(ExpIndex);
				ExportObjectProperty(TEXT("Expression"), MaterialObj, MatExp, Ar, PortFlags);
			}

		TextIndent -= 3;
		Ar.Logf(TEXT("%sEnd ExpressionList") LINE_TERMINATOR, appSpc(TextIndent));

		// /** Array of comments associated with this material; viewed in the material editor. */
		// var editoronly array<MaterialExpressionComment>	EditorComments;
		Ar.Logf(TEXT("%sBegin ExpressionCommentList") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent += 3;

			for (INT CommentIndex = 0; CommentIndex < MaterialObj->EditorComments.Num(); CommentIndex++)
			{
				UMaterialExpression* MatExp = MaterialObj->EditorComments(CommentIndex);
				ExportObjectProperty(TEXT("Comment"), MaterialObj, MatExp, Ar, PortFlags);
			}

		TextIndent -= 3;
		Ar.Logf(TEXT("%sEnd ExpressionCommentList") LINE_TERMINATOR, appSpc(TextIndent));

		// Ignoring Compounds as they aren't supported (yet)...
		// var editoronly array<MaterialExpressionCompound> EditorCompounds;
		ExportBooleanProperty(TEXT("bUsesDistortion"), MaterialObj->bUsesDistortion, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bIsMasked"), MaterialObj->bIsMasked, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bSupportsSinglePassSHLight"), MaterialObj->bSupportsSinglePassSHLight, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bIsPreviewMaterial"), MaterialObj->bIsPreviewMaterial, Ar, PortFlags);

		Ar.Logf(TEXT("%sBegin ReferencedTextureList") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent += 3;

			for (INT RefTxtrIndex = 0; RefTxtrIndex < MaterialObj->ReferencedTextures.Num(); RefTxtrIndex++)
			{
				UTexture* RefTxtr = MaterialObj->ReferencedTextures(RefTxtrIndex);
				ExportObjectProperty(TEXT("ReferencedTexture"), MaterialObj, RefTxtr, Ar, PortFlags);
			}

		TextIndent -= 3;
		Ar.Logf(TEXT("%sEnd ReferencedTextureList") LINE_TERMINATOR, appSpc(TextIndent));

		UArrayProperty* TheTagsProp = FindField<UArrayProperty>(UMaterial::StaticClass(), TEXT("ContentTags"));
		if (TheTagsProp)
		{
			FString DataString;
			TheTagsProp->ExportTextItem(DataString, (BYTE*)&(MaterialObj->ContentTags), NULL, MaterialObj, PortFlags);
			Ar.Logf(TEXT("%sContentTags=%s") LINE_TERMINATOR, appSpc(TextIndent), *DataString);
		}

		TextIndent -= 3;

		Ar.Logf(TEXT("%sEnd MaterialData") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent -= 3;

	Ar.Logf( TEXT("%sEnd Material") LINE_TERMINATOR, appSpc(TextIndent));

	return TRUE;
}

IMPLEMENT_CLASS(UMaterialExporterT3D);

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UStaticMeshExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UStaticMesh::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3DSTM")) );
	new(FormatDescription)FString(TEXT("Unreal mesh text"));
}

UBOOL UStaticMeshExporterT3D::ExportText( const FExportObjectInnerContext* Context, 
	UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags )
{
	UStaticMesh* StaticMeshObj = CastChecked<UStaticMesh>(Object);

	Ar.Logf( TEXT("%sBegin StaticMesh Class=%s Name=%s ObjName=%s Archetype=%s'%s'") LINE_TERMINATOR,
		appSpc(TextIndent), *Object->GetClass()->GetName(), *Object->GetName(), *Object->GetName(),
		*Object->GetArchetype()->GetClass()->GetName(), *Object->GetArchetype()->GetPathName() );

		// Export the properties of the static mesh...
		ExportObjectInner( Context, Object, Ar, PortFlags);

		TextIndent += 3;
		// Export the geometry data, etc.
		Ar.Logf(TEXT("%sBegin SMData") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent += 3;

		/** Versioning system... */
		Ar.Logf(TEXT("%sVersion=%d.%d") LINE_TERMINATOR, appSpc(TextIndent), VersionMax, VersionMin);
		/** LOD distance ratio for this mesh */
		ExportFloatProperty(TEXT("LODDistanceRatio"), StaticMeshObj->LODDistanceRatio, Ar, PortFlags);
		/** Range at which only the lowest detail LOD can be displayed */
		ExportFloatProperty(TEXT("LODMaxRange"), StaticMeshObj->LODMaxRange, Ar, PortFlags);
		/** Thumbnail */
		ExportObjectStructProperty(TEXT("ThumbnailAngle"), TEXT("Rotator"), (BYTE*)&(StaticMeshObj->ThumbnailAngle), StaticMeshObj, Ar, PortFlags);
		//
		ExportFloatProperty(TEXT("ThumbnailDistance"), StaticMeshObj->ThumbnailDistance, Ar, PortFlags);
		ExportIntProperty(TEXT("LightMapResolution"), StaticMeshObj->LightMapResolution, Ar, PortFlags);
		ExportIntProperty(TEXT("LightMapCoordinateIndex"), StaticMeshObj->LightMapCoordinateIndex, Ar, PortFlags);
		//
		ExportObjectStructProperty(TEXT("Bounds"), TEXT("BoxSphereBounds"), (BYTE*)&(StaticMeshObj->Bounds), StaticMeshObj, Ar, PortFlags);
		//
		ExportBooleanProperty(TEXT("UseSimpleLineCollision"), StaticMeshObj->UseSimpleLineCollision, Ar, PortFlags);
		ExportBooleanProperty(TEXT("UseSimpleBoxCollision"), StaticMeshObj->UseSimpleBoxCollision, Ar, PortFlags);
		ExportBooleanProperty(TEXT("UseSimpleRigidBodyCollision"), StaticMeshObj->UseSimpleRigidBodyCollision, Ar, PortFlags);
		ExportBooleanProperty(TEXT("DoubleSidedShadowVolumes"), StaticMeshObj->DoubleSidedShadowVolumes, Ar, PortFlags);
		ExportBooleanProperty(TEXT("UseFullPrecisionUVs"), StaticMeshObj->UseFullPrecisionUVs, Ar, PortFlags);
		ExportBooleanProperty(TEXT("bUsedForInstancing"), StaticMeshObj->bUsedForInstancing, Ar, PortFlags);

		UArrayProperty* TheTagsProp = FindField<UArrayProperty>(UStaticMesh::StaticClass(), TEXT("ContentTags"));
		if (TheTagsProp)
		{
			FString DataString;
			TheTagsProp->ExportTextItem(DataString, (BYTE*)&(StaticMeshObj->ContentTags), NULL, StaticMeshObj, PortFlags);
			Ar.Logf(TEXT("%sContentTags=%s") LINE_TERMINATOR, appSpc(TextIndent), *DataString);
		}

		ExportIntProperty(TEXT("InternalVersion"), StaticMeshObj->InternalVersion, Ar, PortFlags);
		ExportStringProperty(TEXT("HighResSourceMeshName"), StaticMeshObj->HighResSourceMeshName, Ar, PortFlags);
		ExportIntProperty(TEXT("HighResSourceMeshCRC"), StaticMeshObj->HighResSourceMeshCRC, Ar, PortFlags);

		// We don't care about these???
		/** Array of physics-engine shapes that can be used by multiple StaticMeshComponents. */
		//TArray<void*>							PhysMesh;
		/** Scale of each PhysMesh entry. Arrays should be same size. */
		//TArray<FVector>							PhysMeshScale3D;

		/** Array of LODs, holding their associated rendering and collision data */
		//TIndirectArray<FStaticMeshRenderData>	LODModels;
		for (INT ModelIndex = 0; ModelIndex < StaticMeshObj->LODModels.Num(); ModelIndex++)
		{
			FStaticMeshRenderData& Model = StaticMeshObj->LODModels(ModelIndex);
			ExportRenderData(Model, Ar, Warn, PortFlags);
		}

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd SMData") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent -= 3;

	Ar.Logf( TEXT("%sEnd StaticMesh") LINE_TERMINATOR, appSpc(TextIndent) );

	return TRUE;
}

void UStaticMeshExporterT3D::ExportStaticMeshElement(const FString& Name, const FStaticMeshElement& SMElement, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	Ar.Logf(TEXT("%sBegin StaticMeshElement") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

	ExportStringProperty(TEXT("Material"), SMElement.Material ? SMElement.Material->GetPathName() : TEXT(""), Ar, PortFlags);
	ExportBooleanProperty(TEXT("EnableCollision"), SMElement.EnableCollision, Ar, PortFlags);
	ExportBooleanProperty(TEXT("OldEnableCollision"), SMElement.OldEnableCollision, Ar, PortFlags);
	ExportBooleanProperty(TEXT("bEnableShadowCasting"), SMElement.bEnableShadowCasting, Ar, PortFlags);
	ExportIntProperty(TEXT("FirstIndex"), SMElement.FirstIndex, Ar, PortFlags);
	ExportIntProperty(TEXT("NumTriangles"), SMElement.NumTriangles, Ar, PortFlags);
	ExportIntProperty(TEXT("MinVertexIndex"), SMElement.MinVertexIndex, Ar, PortFlags);
	ExportIntProperty(TEXT("MaxVertexIndex"), SMElement.MaxVertexIndex, Ar, PortFlags);
	ExportIntProperty(TEXT("MaterialIndex"), SMElement.MaterialIndex, Ar, PortFlags);

	Ar.Logf(TEXT("%sBegin Fragments") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

		for (INT FragmentIndex = 0; FragmentIndex < SMElement.Fragments.Num(); FragmentIndex++)
		{
			Ar.Logf(TEXT("%s%d,%d") LINE_TERMINATOR, appSpc(TextIndent), 
				SMElement.Fragments(FragmentIndex).BaseIndex,
				SMElement.Fragments(FragmentIndex).NumPrimitives);
		}

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd Fragments") LINE_TERMINATOR, appSpc(TextIndent));

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd StaticMeshElement") LINE_TERMINATOR, appSpc(TextIndent));
}

void UStaticMeshExporterT3D::ExportStaticMeshTriangleBulkData(FStaticMeshTriangleBulkData& SMTBulkData, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	Ar.Logf(TEXT("%sBegin StaticMeshTriangleBulkData") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

	ExportUntypedBulkData(SMTBulkData, Ar, Warn, PortFlags);

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd StaticMeshTriangleBulkData") LINE_TERMINATOR, appSpc(TextIndent));
}

UBOOL UStaticMeshExporterT3D::ExportRenderData(FStaticMeshRenderData& Model, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags )
{
	Ar.Logf(TEXT("%sBegin SMRenderData") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

	/** The number of vertices in the LOD. */
	ExportIntProperty(TEXT("NumVertices"), Model.NumVertices, Ar, PortFlags);

	//TArray<FStaticMeshElement>				Elements;
	Ar.Logf(TEXT("%sBegin Elements") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;
	ExportIntProperty(TEXT("Count"), Model.Elements.Num(), Ar, PortFlags);
	for (INT ElementIndex = 0; ElementIndex < Model.Elements.Num(); ElementIndex++)
	{
		const FStaticMeshElement& Element = Model.Elements(ElementIndex);
		FString Name;
		ExportStaticMeshElement(Name, Element, Ar, Warn, PortFlags);
	}
	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd Elements") LINE_TERMINATOR, appSpc(TextIndent));

	ExportStaticMeshTriangleBulkData(Model.RawTriangles, Ar, Warn, PortFlags);

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd SMRenderData") LINE_TERMINATOR, appSpc(TextIndent));

	return TRUE;
}

IMPLEMENT_CLASS(UStaticMeshExporterT3D)

/*------------------------------------------------------------------------------
	UTextureExporterT3D implementation.
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureExporterT3D::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UTexture::StaticClass();
	bText = 1;
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("T3DT2D")) );
	new(FormatDescription)FString(TEXT("Unreal texture text"));
}

UBOOL UTextureExporterT3D::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	FString PropText;
	UTexture* TextureObj = CastChecked<UTexture>(Object);
	UTexture2D* Texture2DObj = Cast<UTexture2D>(Object);
	UTextureCube* TextureCubeObj = Cast<UTextureCube>(Object);

	Ar.Logf( TEXT("%sBegin Texture Class=%s Name=%s ObjName=%s Archetype=%s'%s'") LINE_TERMINATOR,
		appSpc(TextIndent), *Object->GetClass()->GetName(), *Object->GetName(), *Object->GetName(),
		*Object->GetArchetype()->GetClass()->GetName(), *Object->GetArchetype()->GetPathName() );

		// Export the properties of the static mesh...
		ExportObjectInner( Context, Object, Ar, PortFlags);

		TextIndent += 3;
		// Export the geometry data, etc.
		Ar.Logf(TEXT("%sBegin TextureData") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent += 3;

			/** Versioning system... */
			Ar.Logf(TEXT("%sVersion=%d.%d") LINE_TERMINATOR, appSpc(TextIndent), VersionMax, VersionMin);

			ExportBooleanProperty(TEXT("SRGB"), TextureObj->SRGB, Ar, PortFlags);
			ExportBooleanProperty(TEXT("RGBE"), TextureObj->RGBE, Ar, PortFlags);
			Ar.Logf(TEXT("%sUnpackMin=(%f,%f,%f,%f)") LINE_TERMINATOR, appSpc(TextIndent),
				TextureObj->UnpackMin[0], TextureObj->UnpackMin[1], TextureObj->UnpackMin[2], TextureObj->UnpackMin[3]);
			Ar.Logf(TEXT("%sUnpackMax=(%f,%f,%f,%f)") LINE_TERMINATOR, appSpc(TextIndent),
				TextureObj->UnpackMax[0], TextureObj->UnpackMax[1], TextureObj->UnpackMax[2], TextureObj->UnpackMax[3]);

			ExportBooleanProperty(TEXT("CompressionNoAlpha"), TextureObj->CompressionNoAlpha, Ar, PortFlags);
			ExportBooleanProperty(TEXT("CompressionNone"), TextureObj->CompressionNone, Ar, PortFlags);
			ExportBooleanProperty(TEXT("CompressionNoMipmaps"), TextureObj->CompressionNoMipmaps, Ar, PortFlags);
			ExportBooleanProperty(TEXT("CompressionFullDynamicRange"), TextureObj->CompressionFullDynamicRange, Ar, PortFlags);
			ExportBooleanProperty(TEXT("DeferCompression"), TextureObj->DeferCompression, Ar, PortFlags);

			/** Allows artists to specify that a texture should never have its miplevels dropped which is useful for e.g. HUD and menu textures */
			ExportBooleanProperty(TEXT("NeverStream"), TextureObj->NeverStream, Ar, PortFlags);
			/** When TRUE, mip-maps are dithered for smooth transitions. */
			ExportBooleanProperty(TEXT("bDitherMipMapAlpha"), TextureObj->bDitherMipMapAlpha, Ar, PortFlags);
			/** If TRUE, the color border pixels are preseved by mipmap generation.  One flag per color channel. */
			ExportBooleanProperty(TEXT("bPreserveBorderR"), TextureObj->bPreserveBorderR, Ar, PortFlags);
			ExportBooleanProperty(TEXT("bPreserveBorderG"), TextureObj->bPreserveBorderG, Ar, PortFlags);
			ExportBooleanProperty(TEXT("bPreserveBorderB"), TextureObj->bPreserveBorderB, Ar, PortFlags);
			ExportBooleanProperty(TEXT("bPreserveBorderA"), TextureObj->bPreserveBorderA, Ar, PortFlags);
			/** If TRUE, the RHI texture will be created using TexCreate_NoTiling */
			ExportBooleanProperty(TEXT("bNoTiling"), TextureObj->bNoTiling, Ar, PortFlags);

			ExportStringProperty(TEXT("CompressionSettings"), 
				UTexture::GetCompressionSettingsString((TextureCompressionSettings)(TextureObj->CompressionSettings)), 
				Ar, PortFlags);
			ExportStringProperty(TEXT("Filter"), 
				UTexture::GetTextureFilterString((TextureFilter)(TextureObj->Filter)), 
				Ar, PortFlags);
			ExportStringProperty(TEXT("LODGroup"), 
				UTexture::GetTextureGroupString((TextureGroup)(TextureObj->LODGroup)), 
				Ar, PortFlags);

			/** A bias to the index of the top mip level to use. */
			ExportIntProperty(TEXT("LODBias"), TextureObj->LODBias, Ar, PortFlags);
			ExportStringProperty(TEXT("SourceFilePath"), TextureObj->SourceFilePath, Ar, PortFlags);
			ExportStringProperty(TEXT("SourceFileTimestamp"), TextureObj->SourceFileTimestamp, Ar, PortFlags);

			Ar.Logf(TEXT("%sBegin SourceArt") LINE_TERMINATOR, appSpc(TextIndent));
			TextIndent += 3;

				ExportUntypedBulkData(TextureObj->SourceArt, Ar, Warn, PortFlags);

			TextIndent -= 3;
			Ar.Logf(TEXT("%sEnd SourceArt") LINE_TERMINATOR, appSpc(TextIndent));

			if (Texture2DObj)
			{
				ExportText_Texture2D(Context, Texture2DObj, Type, Ar, Warn, PortFlags);
			}
			if (TextureCubeObj)
			{
				ExportText_TextureCube(Context, TextureCubeObj, Type, Ar, Warn, PortFlags);
			}

		TextIndent -= 3;
		Ar.Logf(TEXT("%sEnd TextureData") LINE_TERMINATOR, appSpc(TextIndent));
		TextIndent -= 3;

	Ar.Logf( TEXT("%sEnd Texture") LINE_TERMINATOR, appSpc(TextIndent) );

	return TRUE;
}

UBOOL UTextureExporterT3D::ExportText_Texture2D(const FExportObjectInnerContext* Context, UTexture2D* InTexture2D, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	//@NOTE: This must match up with the TextureFactory importing code!
	Ar.Logf(TEXT("%sBegin Texture2DData") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

		FString PropText;
		/** The width of the texture.												*/
		ExportIntProperty(TEXT("SizeX"), InTexture2D->SizeX, Ar, PortFlags);
		/** The height of the texture.												*/
		ExportIntProperty(TEXT("SizeY"), InTexture2D->SizeY, Ar, PortFlags);
		/** The format of the texture data.											*/
		ExportStringProperty(TEXT("Format"), GPixelFormats[InTexture2D->Format].Name, Ar, PortFlags);
		/** The addressing mode to use for the X axis.								*/
		ExportStringProperty(TEXT("AddressX"), 
			UTexture::GetTextureAddressString((TextureAddress)(InTexture2D->AddressX)), Ar, PortFlags);
		/** The addressing mode to use for the Y axis.								*/
		ExportStringProperty(TEXT("AddressY"), 
			UTexture::GetTextureAddressString((TextureAddress)(InTexture2D->AddressY)), Ar, PortFlags);
		/** Global/ serialized version of ForceMiplevelsToBeResident.				*/
		ExportBooleanProperty(TEXT("bGlobalForceMipLevelsToBeResident"), InTexture2D->bGlobalForceMipLevelsToBeResident, Ar, PortFlags);
		/** 
		* Keep track of the first mip level stored in the packed miptail.
		* it's set to highest mip level if no there's no packed miptail 
		*/
		ExportIntProperty(TEXT("MipTailBaseIdx"), InTexture2D->MipTailBaseIdx, Ar, PortFlags);

		// Write out the top-level mip only...
		if (InTexture2D->SourceArt.IsBulkDataLoaded() == FALSE)
		{
			if (InTexture2D->Mips.Num() > 0)
			{
				FTexture2DMipMap& TopLevelMip = InTexture2D->Mips(0);

				Ar.Logf(TEXT("%sBegin Mip0") LINE_TERMINATOR, appSpc(TextIndent));
				TextIndent += 3;
				ExportIntProperty(TEXT("SizeX"), TopLevelMip.SizeX, Ar, PortFlags);
				ExportIntProperty(TEXT("SizeY"), TopLevelMip.SizeY, Ar, PortFlags);

				//var native TextureMipBulkData_Mirror Data{FTextureMipBulkData};	
				Ar.Logf(TEXT("%sBegin TextureMipBulkData") LINE_TERMINATOR, appSpc(TextIndent));
				TextIndent += 3;

					ExportUntypedBulkData(TopLevelMip.Data, Ar, Warn, PortFlags);

				TextIndent -= 3;
				Ar.Logf(TEXT("%sEnd TextureMipBulkData") LINE_TERMINATOR, appSpc(TextIndent));

				TextIndent -= 3;
				Ar.Logf(TEXT("%sEnd Mip0") LINE_TERMINATOR, appSpc(TextIndent));
			}
		}

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd Texture2DData") LINE_TERMINATOR, appSpc(TextIndent));

	return TRUE;
}

UBOOL UTextureExporterT3D::ExportText_TextureCube(const FExportObjectInnerContext* Context, UTextureCube* InTextureCube, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	//@NOTE: This must match up with the TextureFactory importing code!
	Ar.Logf(TEXT("%sBegin TextureCubeData") LINE_TERMINATOR, appSpc(TextIndent));
	TextIndent += 3;

		FString PropText;
		/** The width of the texture.												*/
		ExportIntProperty(TEXT("SizeX"), InTextureCube->SizeX, Ar, PortFlags);
		/** The height of the texture.												*/
		ExportIntProperty(TEXT("SizeY"), InTextureCube->SizeY, Ar, PortFlags);
		/** The format of the texture data.											*/
		ExportStringProperty(TEXT("Format"), GPixelFormats[InTextureCube->Format].Name, Ar, PortFlags);
		/** The addressing mode to use for the X axis.								*/

		ExportObjectProperty(TEXT("FacePosX"), InTextureCube, InTextureCube->FacePosX, Ar, PortFlags);
		ExportObjectProperty(TEXT("FaceNegX"), InTextureCube, InTextureCube->FaceNegX, Ar, PortFlags);
		ExportObjectProperty(TEXT("FacePosY"), InTextureCube, InTextureCube->FacePosY, Ar, PortFlags);
		ExportObjectProperty(TEXT("FaceNegY"), InTextureCube, InTextureCube->FaceNegY, Ar, PortFlags);
		ExportObjectProperty(TEXT("FacePosZ"), InTextureCube, InTextureCube->FacePosZ, Ar, PortFlags);
		ExportObjectProperty(TEXT("FaceNegZ"), InTextureCube, InTextureCube->FaceNegZ, Ar, PortFlags);

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd TextureCubeData") LINE_TERMINATOR, appSpc(TextIndent));

	return TRUE;
}

IMPLEMENT_CLASS(UTextureExporterT3D);
