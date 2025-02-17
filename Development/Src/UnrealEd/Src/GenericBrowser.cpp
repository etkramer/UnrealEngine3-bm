/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EnginePhysicsClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "LensFlare.h"
#include "LevelUtils.h"
#include "ScopedTransaction.h"
#include "GenericBrowser.h"
#include "ReferencedAssetsBrowser.h"
#include "..\..\Launch\Resources\resource.h"
#include "GenericBrowserToolBar.h"
#include "SourceControlIntegration.h"
#include "BusyCursor.h"
#include "Properties.h"
#include "GenericBrowserContextMenus.h"
#include "DlgGenericBrowserSearch.h"
#include "DlgRename.h"
#include "DlgOpenPackages.h"
#include "DlgMoveAssets.h"
#include "LocalizationExport.h"
#include "DlgGenericComboEntry.h"
#include "UnFracturedStaticMesh.h"
#include "StaticMeshEditor.h"

#if WITH_FACEFX
#include "../../../External/FaceFX/Studio/Main/Inc/FxStudioApp.h"
#include "../FaceFX/FxRenderWidgetUE3.h"
#endif // WITH_FACEFX

#define _GB_SCROLL_SPEED	50

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Helper functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @return		The USelection object shared between all generic browser instances.
 */
static inline USelection*& GetGBSelection()
{
	static USelection* SGBSelection = NULL;
	return SGBSelection;
}

/**
 * Determines which objects are selected. The selection is filtered by
 * the specific class. This also makes sure that objects can only be added
 * to the array once.
 *
 * @param GBTInfo						  The list of generic browser types to filter by.
 * @param OutSelectedObjects		[Out] Returns the set of selected objects.
 */
static void GetSelectedItems(const TArray<FGenericBrowserTypeInfo>& GBTInfo, TArray<UObject*>& OutSelectedObjects)
{
	// For each object selected in the GB . . .
	for ( USelection::TObjectIterator It( GetGBSelection()->ObjectItor() ) ; It ; ++It )
	{
		UObject* Object = *It;
		if( Object )
		{
			// Determine if there is a generic brower type that supports it.
			for ( INT GBTIndex = 0 ; GBTIndex < GBTInfo.Num() ; ++GBTIndex )
			{
				// Determine which objects are selected by class.
				const FGenericBrowserTypeInfo& Info = GBTInfo( GBTIndex );
				if ( Object->IsA( Info.Class ) )
				{
					OutSelectedObjects.AddItem( Object );
					break;
				}
			}
		}
	}
}

/**
 * Gets an icon index to use for the UnrealEd UI when drawing a package name.
 *
 * @param	InPackage		The package to get the icon index for.
 * @return					The index to be used.
 */
static INT SCCGetIconIndex(UObject* InPackage)
{
	INT IconIndex			= GBTCI_None;

	const UPackage* Package = CastChecked<UPackage>( InPackage );
	switch( Package->GetSCCState() )
	{
	case SCC_ReadOnly:
		IconIndex = GBTCI_SCC_ReadOnly;
		break;
	case SCC_NotCurrent:
		IconIndex = GBTCI_SCC_NotCurrent;
		break;
	case SCC_NotInDepot:
		IconIndex = GBTCI_SCC_NotInDepot;
		break;
	case SCC_CheckedOutOther:
		IconIndex = GBTCI_SCC_CheckedOutOther;
		break;
	case SCC_CheckedOut:
		IconIndex = GBTCI_SCC_CheckedOut;
		break;
	default:
		IconIndex = GBTCI_SCC_Doc;
		break;
	}

	return IconIndex;
}

/*-----------------------------------------------------------------------------
	WxDlgImportGeneric.
-----------------------------------------------------------------------------*/

class WxDlgImportGeneric : public wxDialog
{
public:
	WxDlgImportGeneric(UBOOL bInOKToAll=FALSE);
	virtual ~WxDlgImportGeneric()
	{
		FWindowUtil::SavePosSize( TEXT("DlgImportGeneric"), this );
	}

	const FString& GetPackage() const { return Package; }
	const FString& GetGroup() const { return Group; }

	/**
	* @param	bInImporting		Points to a flag that WxDlgImportGeneric sets to true just before DoImport() and false just afterwards.
	*/
	virtual int ShowModal( const FFilename& InFilename, const FString& InPackage, const FString& InGroup, UClass* InClass, UFactory* InFactory, UBOOL* bInImporting );

	void OnOK( wxCommandEvent& In )
	{
		FlaggedDoImport();
	}

	void OnOKAll( wxCommandEvent& In )
	{
		bOKToAll = 1;
		FlaggedDoImport();
	}

	void OnBuildFromPath( wxCommandEvent& In );

	/** Syncs the GB to the newly imported objects that have been imported so far. */
	void SyncGBToNewObjects()
	{
		WxGenericBrowser* GenericBrowserPtr = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
		check(GenericBrowserPtr);
		GenericBrowserPtr->SyncToObjects( NewObjects );
	}

protected:
	/** Collects newly imported objects as they are created via successive calls to ShowModal. */
	TArray<UObject*> NewObjects;

	FString Package, Group, Name;
	UFactory* Factory;
	UClass* Class;
	FFilename Filename;
	WxPropertyWindow* PropertyWindow;
	UBOOL bOKToAll;
	UBOOL* bImporting;

	wxBoxSizer* PGNSizer;
	wxPanel* PGNPanel;
	WxPkgGrpNameCtrl* PGNCtrl;
	wxComboBox *FactoryCombo;
	wxBoxSizer* PropsPanelSizer;

	virtual void DoImport();

	DECLARE_EVENT_TABLE()

private:
	/**
	 * Wraps DoImport() with bImporting flag toggling.
	 */
	void FlaggedDoImport()
	{
		// Set the client-specified bImporting flag to indicate that we're about to import.
		if ( bImporting )
		{
			*bImporting = true;
		}
		DoImport();
		if ( bImporting )
		{
			*bImporting = false;
		}
	}
};

BEGIN_EVENT_TABLE(WxDlgImportGeneric, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgImportGeneric::OnOK )
END_EVENT_TABLE()

WxDlgImportGeneric::WxDlgImportGeneric(UBOOL bInOKToAll)
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_GENERIC_IMPORT") );
	check( bSuccess );

	PGNPanel = (wxPanel*)FindWindow( XRCID( "ID_PKGGRPNAME" ) );
	check( PGNPanel != NULL );
	//wxSizer* szr = PGNPanel->GetSizer();

	PGNSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		PGNCtrl = new WxPkgGrpNameCtrl( PGNPanel, -1, NULL );
		PGNCtrl->SetSizer(PGNCtrl->FlexGridSizer);
		PGNSizer->Add(PGNCtrl, 1, wxEXPAND);
	}
	PGNPanel->SetSizer(PGNSizer);
	PGNPanel->SetAutoLayout(true);
	PGNPanel->Layout();
	
	// Add Property Window to the panel we created for it.
	wxPanel* PropertyWindowPanel = (wxPanel*)FindWindow( XRCID( "ID_PROPERTY_WINDOW_PANEL" ) );
	check( PropertyWindowPanel != NULL );

	PropsPanelSizer = new wxBoxSizer(wxVERTICAL);
	{
		PropertyWindow = new WxPropertyWindow;
		PropertyWindow->Create( PropertyWindowPanel, GUnrealEd );
		PropertyWindow->Show();
		PropsPanelSizer->Add( PropertyWindow, wxEXPAND, wxEXPAND );
	}
	PropertyWindowPanel->SetSizer( PropsPanelSizer );
	PropertyWindowPanel->SetAutoLayout(true);


	ADDEVENTHANDLER( XRCID("ID_OK_ALL"), wxEVT_COMMAND_BUTTON_CLICKED , &WxDlgImportGeneric::OnOKAll );
	ADDEVENTHANDLER( XRCID("IDPB_BUILD_FROM_PATH"), wxEVT_COMMAND_BUTTON_CLICKED , &WxDlgImportGeneric::OnBuildFromPath );

	Factory = NULL;
	bOKToAll = bInOKToAll;
	bImporting = NULL;

	FWindowUtil::LoadPosSize( TEXT("DlgImportGeneric"), this );
	wxRect ThisRect = GetRect();
	ThisRect.width = 500;
	ThisRect.height = 500;
	SetSize( ThisRect );
	FLocalizeWindow( this );
}

int WxDlgImportGeneric::ShowModal(const FFilename& InFilename, const FString& InPackage, const FString& InGroup, UClass* InClass, UFactory* InFactory, UBOOL* bInImporting )
{
	Filename = InFilename;
	Package = InPackage;
	Group = InGroup;
	Factory = InFactory;
	Class = InClass;

	// Special case handling for bulk package file.
	if (Filename.GetExtension() == TEXT("T3DPKG"))
	{
		// Special case handling!
		UObject* Result = UFactory::StaticImportObject( Class, NULL, FName( *Name ), RF_Public|RF_Standalone, *Filename, NULL, Factory );
		if ( !Result )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_ImportFailed") );
		}
		return wxID_OK;
	}

	PGNCtrl->PkgCombo->SetValue( *Package );
	PGNCtrl->GrpEdit->SetValue( *Group );
	PGNCtrl->NameEdit->SetValue( *InFilename.GetBaseFilename() );

	// Copy off the location of the flag which marks DoImport() being active.
	bImporting = bInImporting;

	if( bOKToAll )
	{
		FlaggedDoImport();
		return wxID_OK;
	}

	PropertyWindow->SetObject( Factory, 1,1,1 );
	PropertyWindow->Raise();
	PropsPanelSizer->Layout();
	Refresh();

	return wxDialog::ShowModal();
}

void WxDlgImportGeneric::OnBuildFromPath( wxCommandEvent& In )
{
	TArray<FString> Split;

	if( Filename.ParseIntoArray( &Split, TEXT("\\"), 0 ) > 0 )
	{
		FString Field, Group, Name;
		UBOOL bPackageFound = 0;

		FString PackageName = (const TCHAR *)PGNCtrl->PkgCombo->GetValue();

		for( INT x = 0 ; x < Split.Num() ; ++x )
		{
			Field = Split(x);

			if( x == Split.Num()-1 )
			{
				Name = Filename.GetBaseFilename();
			}
			else if( bPackageFound )
			{
				if( Group.Len() )
				{
					Group += TEXT(".");
				}
				Group += Field;
			}
			else if( PackageName == Field )
			{
				bPackageFound = 1;
			}
		}

		PGNCtrl->GrpEdit->SetValue( *Group );
		PGNCtrl->NameEdit->SetValue( *Name );
	}
}

void WxDlgImportGeneric::DoImport()
{
	// Make sure the property window has applied all outstanding changes
	if( PropertyWindow != NULL )
	{
		PropertyWindow->FlushLastFocused();
		PropertyWindow->ClearLastFocused();
	}


	const FScopedBusyCursor BusyCursor;
	Package = PGNCtrl->PkgCombo->GetValue();
	Group = PGNCtrl->GrpEdit->GetValue();
	Name = PGNCtrl->NameEdit->GetValue();

	FString	QualifiedName;
	if( Group.Len() )
	{
		QualifiedName = Package + TEXT(".") + Group + TEXT(".") + Name;
	}
	else
	{
		QualifiedName = Package + TEXT(".") + Name;
	}
	FString Reason;
	if (!FIsValidObjectName( *Name, Reason )
	||	!FIsValidGroupName( *Package, Reason )
	||	!FIsValidGroupName( *Group, Reason, TRUE))
	{
		appMsgf( AMT_OK, *FString::Printf(LocalizeSecure(LocalizeUnrealEd(TEXT("Error_ImportFailed_f")), *(QualifiedName + TEXT(": ") + Reason))) );
		return;
	}

	UPackage* Pkg = GEngine->CreatePackage(NULL,*Package);
	Pkg->SetDirtyFlag(TRUE);

	if( Group.Len() )
	{
		Pkg = GEngine->CreatePackage(Pkg,*Group);
	}


	// If a class wasn't specified, the factory can return multiple types.
	// Let factory use properties set in the import dialog to determine a class.
	if ( !Class )
	{
		Class = Factory->ResolveSupportedClass();
		check( Class );
	}

	UObject* Result = UFactory::StaticImportObject( Class, Pkg, FName( *Name ), RF_Public|RF_Standalone, *Filename, NULL, Factory );
	if ( !Result )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_ImportFailed") );
	}
	else
	{
		NewObjects.AddItem( Result );
	}

	if( IsModal() )
	{
		EndModal( wxID_OK );
	}
}

/*-----------------------------------------------------------------------------
	WxDlgExportGeneric.
-----------------------------------------------------------------------------*/

class WxDlgExportGeneric : public wxDialog
{
public:
	WxDlgExportGeneric()
	{
		const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_GENERIC_EXPORT") );
		check( bSuccess );

		NameText = (wxTextCtrl*)FindWindow( XRCID( "IDEC_NAME" ) );
		check( NameText != NULL );

		// Replace the placeholder window with a property window

		PropertyWindow = new WxPropertyWindow;
		PropertyWindow->Create( this, GUnrealEd );
		wxWindow* win = (wxWindow*)FindWindow( XRCID( "ID_PROPERTY_WINDOW" ) );
		check( win != NULL );
		const wxRect rc = win->GetRect();
		PropertyWindow->SetSize( rc );
		win->Show(0);

		Exporter = NULL;

		FWindowUtil::LoadPosSize( TEXT("DlgExportGeneric"), this );

		FLocalizeWindow( this );
	}

	~WxDlgExportGeneric()
	{
		FWindowUtil::SavePosSize( TEXT("DlgExportGeneric"), this );
	}

	/**
	 * Performs the export.  Modal.
	 *
	 * @param	InFilename	The filename to export the object as.
	 * @param	InObject	The object to export.
	 * @param	InExporter	The exporter to use.
	 * @param	bPrompt		If TRUE, display the export dialog and property window.
	 */
	int ShowModal(const FFilename& InFilename, UObject* InObject, UExporter* InExporter, UBOOL bPrompt)
	{
		Filename = InFilename;
		Exporter = InExporter;
		Object = InObject;

		if ( bPrompt )
		{
			NameText->SetLabel( *Object->GetFullName() );
			NameText->SetEditable(false);

			PropertyWindow->SetObject( Exporter, 1,1,0 );
			//PropertyWindow->SetSize(200,200);
			Refresh();

			return wxDialog::ShowModal();
		}
		else
		{
			DoExport();
			return wxID_OK;
		}
	}

private:
	UObject* Object;
	UExporter* Exporter;
	FFilename Filename;
	WxPropertyWindow* PropertyWindow;

	wxTextCtrl *NameText;

	void OnOK( wxCommandEvent& In )
	{
		DoExport();
		EndModal( wxID_OK );
	}

	void DoExport()
	{
		const FScopedBusyCursor BusyCursor;
		//UExporter::ExportToFile( Object, Exporter, *Filename, FALSE );

		UExporter::FExportToFileParams Params;
		Params.Object = Object;
		Params.Exporter = Exporter;
		Params.Filename = *Filename;
		Params.InSelectedOnly = FALSE;
		Params.NoReplaceIdentical = FALSE;
		Params.Prompt = FALSE;
		Params.bUseFileArchive = Object->IsA(UPackage::StaticClass());
		UExporter::ExportToFileEx(Params);
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxDlgExportGeneric, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgExportGeneric::OnOK )
END_EVENT_TABLE()

/*-----------------------------------------------------------------------------
	WxDropTargetFileImport.
-----------------------------------------------------------------------------*/

/**
 * WxDropTargetFileImport - DropTarget for handling dropping files onto the GB treectrl, results in the file list being imported.
 */
class WxDropTargetFileImport : public wxFileDropTarget
{
public:
	WxDropTargetFileImport(wxTreeCtrl* InParent);

	/**
	* Callback for when the drags files over the GB treectrl; autoexpands the package tree view if the element dragged over is collapsed
	*
	* @param InX			Drop position X
	* @param InY			Drop position X
	* @param InDef			Default return value
	*/
	//virtual wxDragResult OnDragOver(wxCoord InX, wxCoord InY, wxDragResult InDef);

	/**
	* Callback for when the user drops files onto the GB treectrl, imports all of the files to the highlighted package.
	*
	* @param InX			Drop position X
	* @param InY			Drop position X
	* @param InFileNames	Array of filenames
	*/
	virtual bool OnDropFiles(wxCoord InX, wxCoord InY, const wxArrayString& InFileNames);

private:
	wxTreeCtrl* ParentTree;
};


WxDropTargetFileImport::WxDropTargetFileImport(wxTreeCtrl* InParent)
	:	wxFileDropTarget(),
		ParentTree(InParent)
{
	check(ParentTree != NULL);
}

/**
* Callback for when the drags files over the GB treectrl; autoexpands the package tree view if the element dragged over is collapsed
*
* @param InX			Drop position X
* @param InY			Drop position X
* @param InDef			Default return value
*/
/*
wxDragResult WxDropTargetFileImport::OnDragOver(wxCoord InX, wxCoord InY, wxDragResult InDef)
{
INT HitFlags = wxTREE_HITTEST_ONITEMLABEL | wxTREE_HITTEST_ONITEMICON;
const wxTreeItemId Item = ParentTree->HitTest( wxPoint(InX,InY), HitFlags);

if( Item.IsOk() )
{
ParentTree->UnselectAll();
ParentTree->SelectItem( Item );

if(!ParentTree->IsExpanded(Item))
{
ParentTree->Expand( Item );
}
}

return InDef;
}
*/


/**
 * Imports all of the files in the string array passed in.
 *
 * @param InFiles					Files to import
 * @param InFactories				Array of UFactory classes to be used for importing.
 */
static void ImportFilesWithPackageNameGroupName(
	const TArray<FString>& InFiles,
	const TArray<UFactory*>& InFactories,
	const FString& PackageNameToUse,
	const FString& GroupNameToUse,
	WxDlgImportGeneric& ImportDialog,
	WxGenericBrowser* GenericBrowserPtr
	)
{
	// Detach all components while importing objects.
	// This is necessary for the cases where the import replaces existing objects which may be referenced by the attached components.
	FGlobalComponentReattachContext ReattachContext;

	FString Package = PackageNameToUse;
	FString Group = GroupNameToUse;

	UBOOL bSawSuccessfulImport = FALSE;		// Used to flag whether at least one import was successful

	// Reset the 'Do you want to overwrite the existing object?' Yes to All / No to All prompt, to make sure the
	// user gets a chance to select something
	UFactory::ResetOverwriteYesOrNoToAllState();

	// For each filename, open up the import dialog and get required user input.
	for( INT FileIndex = 0; FileIndex < InFiles.Num(); FileIndex++ )
	{
		GWarn->StatusUpdatef( FileIndex, InFiles.Num(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd( "Importingf" ), FileIndex,InFiles.Num()) ) );

		const FFilename Filename = *InFiles( FileIndex );
		GenericBrowserPtr->LastImportPath = Filename.GetPath();

		UFactory* Factory = NULL;
		UBOOL bFoundFactory = FALSE;

		for( INT FactoryIdx = 0 ; FactoryIdx < InFactories.Num() ; ++FactoryIdx )
		{
			for( UINT FormatIndex = 0; FormatIndex < ( UINT )InFactories(FactoryIdx)->Formats.Num(); FormatIndex++ )
			{
				if( InFactories( FactoryIdx )->Formats( FormatIndex ).Left( Filename.GetExtension().Len() ) == Filename.GetExtension() )
				{
					Factory = InFactories( FactoryIdx );

					bFoundFactory = TRUE;
					break;
				}
			}

			if( bFoundFactory )
			{
				break;
			}
		}

		if( Factory )
		{
			if( ImportDialog.ShowModal( Filename, Package, Group, Factory->GetSupportedClass(), Factory, &GenericBrowserPtr->RightContainer->bSuppressDraw ) == wxID_OK )
			{
				bSawSuccessfulImport = TRUE;
			}

			// Copy off the package and group for the next import.
			Package = ImportDialog.GetPackage();
			Group = ImportDialog.GetGroup();
		}
		else
		{
			// Couldn't find a factory for a class, so throw up an error message.
			appMsgf( AMT_OK, TEXT( "Unable to import file: %s\nCould not find an appropriate actor factory for this filetype." ), *InFiles( FileIndex ) );
		}
	}

	// Clean up the overwrite bit now that we're done importing files
	UFactory::ResetOverwriteYesOrNoToAllState();
}

/**
 * Callback for when the user drops files onto the GB treectrl, imports all of the files to the highlighted package.
 *
 * @param InX			Drop position X
 * @param InY			Drop position X
 * @param InFileNames	Array of filenames
 */
bool WxDropTargetFileImport::OnDropFiles(wxCoord InX, wxCoord InY, const wxArrayString& InFileNames)
{
	WxGenericBrowser* GenericBrowserPtr = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	check(GenericBrowserPtr);

	//Generate a list of factories that can be used in importing assets.
	GWarn->BeginSlowTask( *LocalizeUnrealEd("Importing"), TRUE );
	{

		TArray<UFactory*> Factories;			// All the factories we can possibly use

		for( TObjectIterator<UClass> It ; It ; ++It )
		{
			UClass* CurrentClass = (*It);

			if( CurrentClass->IsChildOf(UFactory::StaticClass()) && !(CurrentClass->ClassFlags & CLASS_Abstract) )
			{
				UFactory* Factory = ConstructObject<UFactory>( CurrentClass );
				if( Factory->bEditorImport && !Factory->bCreateNew )
				{
					Factories.AddItem( Factory );
				}
			}
		}

		// check to see if we are bulk importing sounds 
		// Bulk importing of sounds works the following way:
		//
		//  0) run the editor with -BULKIMPORTINGSOUNDS to enable the bulk sound importing code
		//  1) Open the editor and have the Generic Browser visible
		//  Have your raw .wav files in a directory structure of the following nature
		//      <PackageName_lang>(dir)
		//          <groupName>(dir)
		//               *.wav(file>
		//
		//  Example:
		//
		//        Human_Marcus_Dialog_deu
		//              HOS
		//                a.wav
		//                b.Wav
		//              Tyro
		//                3887a.wav
		//
		//  The bulk import code only supports groups one deep.  So Package->Group->Wav
		//
		//  2) Drag the Folder(s) (that is/are named the same as the loc package to be) onto the GB file list.
		//  3) Click on the editor (this will bring up the import dialog).
		//  4) Click on "ok to all".  And then repeat clicking on the "Ok to all" until all sounds are imported.
		//  5) The GB doesn't always update so you will need to select a different Filter type 
		//    (e.g. static meshes) and then reselect "sounds" as the filter to see your newly created package.
		//  6) save your newly created audio package!
		if( ParseParam(appCmdLine(),TEXT("BULKIMPORTINGSOUNDS")) == TRUE )
		{
			// so here allows us to do multiple dirs at once!
			WxDlgImportGeneric ImportDialog( TRUE );
			for( UINT FileIndex = 0; FileIndex < InFileNames.Count(); ++FileIndex )
			{
				// okie so we have the root dir.
				TArray<FString> SubDirs;
				const TCHAR* RootDirectory = (const TCHAR *)InFileNames[FileIndex];
				FString Root(RootDirectory);
				FString BaseDir = FString(RootDirectory) * TEXT("\\*.*");
				//const FFilename BaseDir = FString::Printf( TEXT( "%s\\"),  );
				GFileManager->FindFiles( SubDirs, *BaseDir, FALSE, TRUE );
				FString BaseFilename = FFilename(RootDirectory).GetBaseFilename();

				// recurse through the directories looking for more files/directories
				for( INT SubDirIndex = 0; SubDirIndex < SubDirs.Num(); ++SubDirIndex )
				{
					TArray<FString> Results;
					appFindFilesInDirectory( Results, *(Root * SubDirs(SubDirIndex)), FALSE, TRUE );
					//int breakme = -1;
					// call the package and group here
					ImportFilesWithPackageNameGroupName( Results, Factories, BaseFilename, SubDirs(SubDirIndex), ImportDialog, GenericBrowserPtr );

				}
			}
			// MSSTART- justinmc 7/19/2008: Moved garbage collection outside of the for loop
			// as resources were being reused after being garbage collected. This was causing
			// the bulkimportingsounds feature to crash the editor.

			// Lightwave importer seems to create some extra, empty static meshes for some reason.  Garbage collection makes them go away.
			UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );
			// MSEND
	
			ImportDialog.SyncGBToNewObjects();
			GenericBrowserPtr->Update();
		}
		else
		{
			//Import all files
			if(GenericBrowserPtr->ImportFiles(InFileNames, Factories))
			{
				// Lightwave importer seems to create some extra, empty static meshes for some reason.  Garbage collection makes them go away.
				UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );
			}
		}

	}
	GWarn->EndSlowTask();

	return TRUE;
}

/*-----------------------------------------------------------------------------
	WxGenericBrowser.
-----------------------------------------------------------------------------*/

FLOAT WxBrowserData::DefaultZoomFactor = 0.25f;
INT WxBrowserData::DefaultFixedSize = 0;
EThumbnailPrimType WxBrowserData::DefaultPrimitiveType = TPT_Sphere;
UBOOL WxBrowserData::bDefaultsAreInited = FALSE;

/**
 * Constructor for the browser data
 */
WxBrowserData::WxBrowserData(EGenericBrowser_TreeCtrlImages InTreeCtrlImg,
	const TCHAR* InKeyPrefix) :
	ScrollMax( 0 ),
	ScrollPos( 0 ),
	KeyPrefix(InKeyPrefix),
	TreeCtrlImg( InTreeCtrlImg )
{
	check(bDefaultsAreInited == TRUE);
	// Now use the defaults first
	ZoomFactor = DefaultZoomFactor;
	FixedSz = DefaultFixedSize;
	PrimitiveType = DefaultPrimitiveType;
	// And override with any saved data
	LoadConfig();
}

/**
 * Saves the settings for this instance
 */
WxBrowserData::~WxBrowserData(void)
{
	SaveConfig();
}

/**
 * This function loads the default values to use from the editor INI
 */
void WxBrowserData::LoadDefaults(void)
{
	bDefaultsAreInited = TRUE;
	FString Prefix(TEXT("Default_BrowserData"));
	// Read the last used zoom percentage
	GConfig->GetFloat(TEXT("BrowserData"),*(Prefix + TEXT("_ZoomFactor")),
		DefaultZoomFactor,GEditorUserSettingsIni);
	// Get the last used fixed size
	GConfig->GetInt(TEXT("BrowserData"),*(Prefix + TEXT("_FixedSize")),
		DefaultFixedSize,GEditorUserSettingsIni);
	INT Bogus = (INT)DefaultPrimitiveType;
	// And finally the last used primitive type
	GConfig->GetInt(TEXT("BrowserData"),*(Prefix + TEXT("_Primitive")),
		Bogus,GEditorUserSettingsIni);
	DefaultPrimitiveType = (EThumbnailPrimType)Bogus;
}

/**
 * This function saves the default values to the editor INI
 */
void WxBrowserData::SaveDefaults(void)
{
	FString Prefix(TEXT("Default_BrowserData"));
	// Save the last used zoom percentage
	GConfig->SetFloat(TEXT("BrowserData"),*(Prefix + TEXT("_ZoomFactor")),
		DefaultZoomFactor,GEditorUserSettingsIni);
	// Save the last used fixed size
	GConfig->SetInt(TEXT("BrowserData"),*(Prefix + TEXT("_FixedSize")),
		DefaultFixedSize,GEditorUserSettingsIni);
	// Save the last used primitive type
	GConfig->SetInt(TEXT("BrowserData"),*(Prefix + TEXT("_Primitive")),
		(INT)DefaultPrimitiveType,GEditorUserSettingsIni);
}

/**
 * Loads the per instance values from the editor INI. Uses defaults if they
 * are missing
 */
void WxBrowserData::LoadConfig(void)
{
	// Read the last used zoom percentage
	GConfig->GetFloat(TEXT("BrowserData"),*(KeyPrefix + TEXT("_ZoomFactor")),
		ZoomFactor,GEditorUserSettingsIni);
	// Get the last used fixed size
	GConfig->GetInt(TEXT("BrowserData"),*(KeyPrefix + TEXT("_FixedSize")),
		FixedSz,GEditorUserSettingsIni);
	INT Bogus = (INT)DefaultPrimitiveType;
	// And finally the last used primitive type
	GConfig->GetInt(TEXT("BrowserData"),*(KeyPrefix + TEXT("_Primitive")),
		Bogus,GEditorUserSettingsIni);
	PrimitiveType = (EThumbnailPrimType)Bogus;
}

/**
 * Saves the current values to the editor INI
 */
void WxBrowserData::SaveConfig(void)
{
	// Save the last used zoom percentage
	GConfig->SetFloat(TEXT("BrowserData"),*(KeyPrefix + TEXT("_ZoomFactor")),
		ZoomFactor,GEditorUserSettingsIni);
	// Save the last used fixed size
	GConfig->SetInt(TEXT("BrowserData"),*(KeyPrefix + TEXT("_FixedSize")),
		FixedSz,GEditorUserSettingsIni);
	// Save the last used primitive type
	GConfig->SetInt(TEXT("BrowserData"),*(KeyPrefix + TEXT("_Primitive")),
		(INT)PrimitiveType,GEditorUserSettingsIni);
}

/**
 * Creates a new menu exposing source control functionality.
 */
static wxMenu* CreateSourceControlMenu()
{
	wxMenu* SCCMenu = new wxMenu();
	SCCMenu->Append( ID_SCC_REFRESH, *LocalizeUnrealEd("Refresh"), TEXT("") );
	SCCMenu->AppendSeparator();
	SCCMenu->Append( ID_SCC_CHECK_OUT, *LocalizeUnrealEd("CheckOut"), TEXT("") );
	SCCMenu->Append( ID_SCC_CHECK_IN, *LocalizeUnrealEd("CheckInE"), TEXT("") );
	// @todo DB: restore once support is properly in place for package reloading in the editor.
	//SCCMenu->Append( ID_SCC_REVERT, *LocalizeUnrealEd("Revert"), TEXT("") );
	SCCMenu->Append( ID_SCC_ADD, *LocalizeUnrealEd("AddE"), TEXT("") );
	SCCMenu->AppendSeparator();
	SCCMenu->Append( ID_SCC_HISTORY, *LocalizeUnrealEd("RevisionHistoryE"), TEXT("") );
	//@todo put back in once we have support for moving a package (close and reopen it in a different location)
	//			SCCMenu->AppendSeparator();
	//			SCCMenu->Append( ID_SCC_MOVE_TO_TRASH, *LocalizeUnrealEd("MoveToTrashE"), TEXT("") );
	return SCCMenu;
}

class WxMBGenericBrowser : public wxMenuBar
{
public:
	wxMenu* MRUMenu;
	FMRUList* MRUList;

	WxMBGenericBrowser()
	{
		MRUMenu = new wxMenu();

		// File menu
		wxMenu* FileMenu = new wxMenu();
		FileMenu->Append( IDM_NEW, *LocalizeUnrealEd("NewE"), TEXT("") );
		FileMenu->AppendSeparator();
		FileMenu->Append( IDMN_FileOpen, *LocalizeUnrealEd("OpenE"), TEXT("") );
		FileMenu->Append( IDMN_FileOpenPackages, *LocalizeUnrealEd("OpenPackagesE"), TEXT("") );
		FileMenu->AppendSeparator();
		FileMenu->Append( IDMN_FileSave, *LocalizeUnrealEd("SaveE"), TEXT("") );
		FileMenu->Append( IDMN_FileSaveAs, *LocalizeUnrealEd("SaveAsE"), *LocalizeUnrealEd("ToolTip_83") );
		FileMenu->AppendSeparator();
		FileMenu->Append( IDM_IMPORT, *LocalizeUnrealEd("ImportE"), TEXT("") );
		FileMenu->Append( IDM_EXPORT, *LocalizeUnrealEd("ExportE"), TEXT("") );
		FileMenu->AppendSeparator();
		FileMenu->Append( IDM_MRU_LIST, *LocalizeUnrealEd("Recent"), MRUMenu );
		Append( FileMenu, *LocalizeUnrealEd("File") );

		// View menu
		wxMenu* ViewMenu = new wxMenu();
		ViewMenu->Append( IDM_RefreshBrowser, *LocalizeUnrealEd("RefreshWithHotkey"), TEXT("") );
		ViewMenu->AppendCheckItem( IDMN_GenericBrowser_ShowContentTags, *LocalizeUnrealEd("GenericBrowser_ShowContentTags"), TEXT("") );
		ViewMenu->AppendCheckItem( IDMN_SHOW_MEMORY_STATS, *LocalizeUnrealEd("ShowMemStats"), TEXT("") );
		ViewMenu->AppendSeparator();
		ViewMenu->Append( IDMN_GenericBrowser_SetMaterialPreviewBackgroundColor, *LocalizeUnrealEd("GenericBrowser_SetMaterialPreviewBackgroundColor"), TEXT("") );
		ViewMenu->Append( IDMN_GenericBrowser_SetTranslucentMaterialPreviewBackgroundColor, *LocalizeUnrealEd("GenericBrowser_SetTranslucentMaterialPreviewBackgroundColor"), TEXT("") );
		Append( ViewMenu, *LocalizeUnrealEd("View") );

#if HAVE_SCC
		// Source Code Control
		if( GApp->SCC && GApp->SCC->IsEnabled() )
		{
			wxMenu* SCCMenu = CreateSourceControlMenu();
			Append( SCCMenu, *LocalizeUnrealEd("SourceControl") );
		}
#endif // HAVE_SCC

		WxBrowser::AddDockingMenu( this );

		MRUList = new FMRUList( TEXT("GenericBrowserMRU"), MRUMenu );
		MRUList->ReadINI();
		MRUList->UpdateMenu();
	}

	WxMBGenericBrowser::~WxMBGenericBrowser()
	{
		MRUList->WriteINI();
		delete MRUList;
	}
};

BEGIN_EVENT_TABLE( WxGenericBrowser, WxBrowser )
	EVT_SIZE( WxGenericBrowser::OnSize )
	EVT_MENU( IDM_RefreshBrowser, WxGenericBrowser::OnRefresh )
	EVT_MENU_RANGE( IDM_MRU_START, IDM_MRU_END, WxGenericBrowser::MenuFileMRU )
	EVT_MENU( IDMN_FileOpen, WxGenericBrowser::OnFileOpen )
	EVT_MENU( IDMN_FileOpenPackages, WxGenericBrowser::OnFileOpenPackages )
	EVT_MENU( IDMN_FileSave, WxGenericBrowser::OnFileSave )
	EVT_MENU( IDMN_FileSaveAs, WxGenericBrowser::OnFileSaveAs )
	EVT_MENU( IDMN_FileFullyLoad, WxGenericBrowser::OnFileFullyLoad )
	EVT_MENU( IDM_NEW, WxGenericBrowser::OnFileNew )
	EVT_MENU( IDM_IMPORT, WxGenericBrowser::OnFileImport )
	EVT_MENU( IDM_EXPORT, WxGenericBrowser::OnObjectExport )
	EVT_MENU( IDM_GenericBrowser_BulkExport, WxGenericBrowser::OnBulkExport )
	EVT_MENU( IDM_GenericBrowser_FullLocExport, WxGenericBrowser::OnFullLocExport )
	EVT_MENU( IDM_GenericBrowser_ResliceFracturedMeshes, WxGenericBrowser::OnResliceFracturedMeshes )
	EVT_MENU_RANGE( ID_NEWOBJ_START, ID_NEWOBJ_END, WxGenericBrowser::OnContextObjectNew )
	EVT_MENU( ID_SCC_REFRESH, WxGenericBrowser::OnSCCRefresh )
	EVT_MENU( ID_SCC_HISTORY, WxGenericBrowser::OnSCCHistory )
	EVT_MENU( ID_SCC_MOVE_TO_TRASH, WxGenericBrowser::OnSCCMoveToTrash )
	EVT_MENU( ID_SCC_ADD, WxGenericBrowser::OnSCCAdd )
	EVT_MENU( ID_SCC_CHECK_OUT, WxGenericBrowser::OnSCCCheckOut )
	EVT_MENU( ID_SCC_CHECK_IN, WxGenericBrowser::OnSCCCheckIn )
	EVT_MENU( ID_SCC_REVERT, WxGenericBrowser::OnSCCRevert )
	EVT_UPDATE_UI( ID_SCC_CHECK_OUT, WxGenericBrowser::UI_SCCCheckOut )
	EVT_UPDATE_UI( ID_SCC_CHECK_IN, WxGenericBrowser::UI_SCCCheckIn )
	EVT_UPDATE_UI( ID_SCC_REVERT, WxGenericBrowser::UI_SCCRevert )
	EVT_UPDATE_UI( ID_SCC_ADD, WxGenericBrowser::UI_SCCAdd )

	EVT_MENU( IDM_GenericBrowser_UnloadPackage, WxGenericBrowser::OnUnloadPackage )

	EVT_MENU( IDMN_ObjectContext_CopyReference, WxGenericBrowser::OnCopyReference )
	EVT_MENU( IDMN_ObjectContext_Properties, WxGenericBrowser::OnObjectProperties )
	EVT_MENU( IDMN_ObjectContext_ShowRefs, WxGenericBrowser::OnObjectShowReferencers )
	EVT_MENU( IDMN_ObjectContext_Export, WxGenericBrowser::OnObjectExport )
	EVT_MENU( IDMN_ObjectContext_Editor, WxGenericBrowser::OnObjectEditor )
	EVT_MENU( IDMN_ObjectContent_AddTag, WxGenericBrowser::OnAddTag )
	EVT_MENU( IDMN_ObjectContext_EditNodes, WxGenericBrowser::OnObjectEditNodes )
	EVT_MENU( IDMN_ObjectContext_ResliceFracturedMeshes, WxGenericBrowser::OnObjectResliceFracturedMeshes )
	EVT_MENU_RANGE( IDMN_ObjectContext_Custom_START, IDMN_ObjectContext_Custom_END, WxGenericBrowser::OnObjectCustomCommand )

	EVT_MENU( IDMN_StaticMeshContext_SimplifyMesh, WxGenericBrowser::OnSimplifyStaticMesh )

	EVT_MENU( IDMN_TextureContext_ShowStreamingBounds, WxGenericBrowser::OnShowStreamingBounds )
	EVT_UPDATE_UI( IDMN_TextureContext_ShowStreamingBounds, WxGenericBrowser::UI_ShowStreamingBounds )

	EVT_MENU( IDMN_SHOW_MEMORY_STATS, WxGenericBrowser::OnShowMemStats )
	EVT_UPDATE_UI( IDMN_SHOW_MEMORY_STATS, WxGenericBrowser::UI_ShowMemStats )
	EVT_MENU( IDMN_GenericBrowser_ShowContentTags, WxGenericBrowser::OnShowContentTags )
	EVT_UPDATE_UI( IDMN_GenericBrowser_ShowContentTags, WxGenericBrowser::UI_ShowContentTags )
	EVT_MENU( IDMN_GenericBrowser_SetMaterialPreviewBackgroundColor, WxGenericBrowser::OnSetMaterialPreviewBackgroundColor )
	EVT_MENU( IDMN_GenericBrowser_SetTranslucentMaterialPreviewBackgroundColor, WxGenericBrowser::OnSetTranslucentMaterialPreviewBackgroundColor )

	EVT_MENU( IDM_USAGEFILTER, WxGenericBrowser::OnUsageFilter )

	EVT_MENU( IDMN_ObjectContext_DuplicateWithRefs, WxGenericBrowser::OnObjectDuplicateWithRefs )
	EVT_MENU( IDMN_ObjectContext_RenameWithRefs, WxGenericBrowser::OnObjectRenameWithRefs )
	EVT_MENU( IDMN_ObjectContext_RenameLocWithRefs, WxGenericBrowser::OnObjectRenameLocWithRefs )
	EVT_MENU( IDMN_ObjectContext_Delete, WxGenericBrowser::OnObjectDelete )
	EVT_MENU( IDMN_ObjectContext_DeleteWithRefs, WxGenericBrowser::OnObjectDeleteWithRefs )
	EVT_MENU( IDMN_ObjectContext_ShowRefObjs, WxGenericBrowser::OnObjectShowReferencedObjs )
END_EVENT_TABLE()

WxGenericBrowser::WxGenericBrowser(void) :
	SplitterWindow(NULL), LeftContainer(NULL), RightContainer(NULL),
	ImageListTree(NULL), ImageListView(NULL), bShowMemoryStats(FALSE), bShowContentTags( FALSE ),
	bUpdateOnActivated(FALSE), LabelRenderer(NULL), NumSelectedObjects(0),
	bShowReferencedOnly(FALSE), LastImportFilter(0)
{
	// Register our callbacks
	GCallbackEvent->Register(CALLBACK_RefreshEditor_GenericBrowser,this);
	GCallbackEvent->Register(CALLBACK_SelChange, this);
	GCallbackEvent->Register(CALLBACK_SelectObject, this);
}


IMPLEMENT_COMPARE_POINTER( UClass, GenericBrowser, { return appStricmp( *CastChecked<UFactory>( A->GetDefaultObject() )->Description, *CastChecked<UFactory>( B->GetDefaultObject() )->Description ); } )


/**
 * Forwards the call to our base class to create the window relationship.
 * Creates any internally used windows after that
 *
 * @param DockID the unique id to associate with this dockable window
 * @param FriendlyName the friendly name to assign to this window
 * @param Parent the parent of this window (should be a Notebook)
 */
void WxGenericBrowser::Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent)
{
	WxBrowser::Create(DockID,FriendlyName,Parent);

	MenuBar = new WxMBGenericBrowser();
	
	ImageListTree = new wxImageList( 16, 15 );
	ImageListView = new wxImageList( 16, 15 );
	
	ImageListTree->Add( WxBitmap( "FolderClosed" ), wxColor( 192,192,192 ) );
	ImageListTree->Add( WxBitmap( "FolderOpen" ), wxColor( 192,192,192 ) );
	ImageListTree->Add( WxBitmap( "Texture" ), wxColor( 192,192,192 ) );
	ImageListTree->Add( WxBitmap( "SCC_Doc" ), wxColor( 192,192,192 ) );
	ImageListTree->Add( WxBitmap( "SCC_ReadOnly" ), wxColor( 192,192,192 ) );
	ImageListTree->Add( WxBitmap( "SCC_CheckedOut" ), wxColor( 192,192,192 ) );
	ImageListTree->Add( WxBitmap( "SCC_CheckedOutOther" ), wxColor( 192,192,192 ) );
	ImageListTree->Add( WxBitmap( "SCC_NotCurrent" ), wxColor( 192,192,192 ) );
	ImageListTree->Add( WxBitmap( "SCC_NotInDepot" ), wxColor( 192,192,192 ) );

	*ImageListView = *ImageListTree;

	wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		SplitterWindow = new wxSplitterWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_BORDER );
		{
			LeftContainer = new WxGBLeftContainer( SplitterWindow );
			RightContainer = new WxGBRightContainer( SplitterWindow );

			const INT MinPaneSize = 20;
			SplitterWindow->SplitVertically( LeftContainer, RightContainer, STD_SPLITTER_SZ );
			SplitterWindow->SetMinimumPaneSize( MinPaneSize );

			LeftContainer->TreeCtrl->AssignImageList( ImageListTree );
			LeftContainer->GenericBrowser = this;

			RightContainer->ListView->AssignImageList( ImageListView, wxIMAGE_LIST_SMALL );
			RightContainer->GenericBrowser = this;
		}
		MainSizer->Add(SplitterWindow, 1, wxEXPAND);
	}
	SetSizer(MainSizer);
	SetAutoLayout( TRUE );
	

	LastExportPath = GApp->LastDir[LD_GENERIC_EXPORT];
	LastImportPath = GApp->LastDir[LD_GENERIC_IMPORT];
	LastOpenPath = GApp->LastDir[LD_GENERIC_OPEN];
	LastSavePath = GApp->LastDir[LD_GENERIC_SAVE];

	// Load the defaults for the WxBrowserData class
	WxBrowserData::LoadDefaults();

	for( INT r = 0 ; r < LeftContainer->ResourceTypes.Num() ; ++r )
	{
		for( INT c = 0 ; c < LeftContainer->ResourceTypes(r)->SupportInfo.Num() ; ++c )
		{
			BrowserData.Set( LeftContainer->ResourceTypes(r)->SupportInfo(c).Class,
				new WxBrowserData( GBTCI_Resource, *LeftContainer->ResourceTypes(r)->SupportInfo(c).Class->GetName() ) );
		}
	}

	MainSizer->Fit(this);

	InitialUpdate();

	// Initialize array of UFactory classes that can create new objects.
	for( TObjectIterator<UClass> It ; It ; ++It )
	{
		if( It->IsChildOf(UFactory::StaticClass()) && !(It->ClassFlags & CLASS_Abstract) )
		{
			UClass* FactoryClass = *It;

			// Add to list if factory creates new objects and the factory is valid for this game (ie Wargame)
			UFactory* Factory = ConstructObject<UFactory>( FactoryClass );
			if( Factory->bCreateNew && Factory->ValidForCurrentGame() )
			{
				FactoryNewClasses.AddItem( FactoryClass );
			}
		}
	}

	Sort<USE_COMPARE_POINTER(UClass,GenericBrowser)>( &FactoryNewClasses(0), FactoryNewClasses.Num() );
}


/**
 * Cleans up any allocated data that isn't freed via wxWindows
 */
WxGenericBrowser::~WxGenericBrowser(void)
{
	SaveSettings();
	// Iterate through each of the WxBrowserData entries and clean them up
	for (TMap<UClass*,WxBrowserData*>::TConstIterator It(BrowserData); It; ++It)
	{
		delete It.Value();
	}
	BrowserData.Empty();
}

void WxGenericBrowser::LoadSettings()
{
	GConfig->GetBool( TEXT("GenericBrowser"), TEXT("bShowReferencedOnly"), bShowReferencedOnly, GEditorUserSettingsIni );
}

void WxGenericBrowser::SaveSettings()
{
	GConfig->SetBool( TEXT("GenericBrowser"), TEXT("bShowReferencedOnly"), bShowReferencedOnly, GEditorUserSettingsIni );
}

/**
 * Saves the WxBrowserData default state in addition to base window state
 */
void WxGenericBrowser::SaveWindowState(void)
{
	WxBrowser::SaveWindowState();
	// Save the defaults in case they've changed. Each instance is
	// saved when they are deleted, not here.
	WxBrowserData::SaveDefaults();
}

WxBrowserData* WxGenericBrowser::FindBrowserData( UClass* InClass )
{
	WxBrowserData* bd = *BrowserData.Find( InClass );
	check(bd);		// Unknown class type!
	return bd;
}

/**
 *	Traverses the tree recursively in a attempt to locate a package.
 *
 *	@param	TreeId		The Tree item ID we are searching recursively.
 *  @param	Cookie		Cookie used to find a child in a tree control.
 *	@param	InPackage	Package to search for.
 *
 *	@return	TRUE if we found the package, FALSE otherwise.
 */
bool WxGenericBrowser::RecurseFindPackage( wxTreeItemId TreeId, wxTreeItemIdValue Cookie, UPackage *InPackage )
{
	if( !TreeId.IsOk() )
		return FALSE;

	WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)LeftContainer->TreeCtrl->GetItemData(TreeId);
	if( ItemData )
	{
		UPackage* Package = ItemData->GetObjectChecked<UPackage>();
		if( InPackage == Package )
		{
			LeftContainer->TreeCtrl->SelectItem( TreeId );
			LeftContainer->TreeCtrl->EnsureVisible( TreeId );

			return TRUE;

		}
	}
	else
	{
		wxTreeItemIdValue NewCookie;
		wxTreeItemId NewTreeId = LeftContainer->TreeCtrl->GetFirstChild( TreeId, NewCookie );
		if(RecurseFindPackage( NewTreeId, NewCookie, InPackage ))
			return TRUE;
	}

	wxTreeItemId NewTreeId = LeftContainer->TreeCtrl->GetNextChild( TreeId, Cookie );

	return RecurseFindPackage( NewTreeId, Cookie, InPackage );
}

/**
 *	Finds and selects InPackage in the tree control
 *
 *	@param	InPackage		Package to search for.
 */
void WxGenericBrowser::SelectPackage( UPackage* InPackage )
{
	wxTreeItemId TreeId = LeftContainer->TreeCtrl->GetRootItem();
	wxTreeItemIdValue Cookie;
	TreeId = LeftContainer->TreeCtrl->GetFirstChild( TreeId, Cookie );

	if(RecurseFindPackage(TreeId, Cookie, InPackage))
	{
		RightContainer->RemoveInvisibleObjects();
		LeftContainer->bTreeViewChanged = TRUE;
	}
}

void WxGenericBrowser::InitialUpdate()
{
	RightContainer->DisableUpdate();

	LoadSettings();
	RightContainer->LoadSettings();
    LeftContainer->LoadSettings();

	// Sychronize source control state.
	LeftContainer->RequestSCCUpdate();

	RightContainer->EnableUpdate();

	// Refresh.
	Update();
}

void WxGenericBrowser::Update()
{
	BeginUpdate();

	// GWorld will be NULL in the cleanse period while newing/loading a map, in which
	// case we should just tear down exsting controls.
	if ( !IsShown() || !GWorld )
	{
		bUpdateOnActivated = TRUE;
	}
	else
	{
		LeftContainer->Update();
		RightContainer->Update();
		bUpdateOnActivated = FALSE;
	}

	EndUpdate();
}

void WxGenericBrowser::Activated()
{
	WxBrowser::Activated();
	if ( bUpdateOnActivated )
	{
		Update();
	}
}

/**
 * This archive marks all objects referenced by the specified "root set" of objects.
 */
class FArchiveReferenceMarker : public FArchive
{
public:
	FArchiveReferenceMarker( TArray<UObject*>& SourceObjects )
	{
		ArIsObjectReferenceCollector = TRUE;
		ArIgnoreOuterRef = TRUE;

		for ( INT ObjectIndex = 0; ObjectIndex < SourceObjects.Num(); ObjectIndex++ )
		{
			UObject* Object = SourceObjects(ObjectIndex);
			Object->SetFlags(RF_TagImp);

			// RF_TagImp is used to allow serialization of objects which we would otherwise ignore.
			Object->Serialize(*this);
		}

		for ( INT ObjectIndex = 0; ObjectIndex < SourceObjects.Num(); ObjectIndex++ )
		{
			UObject* Object = SourceObjects(ObjectIndex);
			Object->ClearFlags(RF_TagImp);
		}
	}

	/** 
	 * UObject serialize operator implementation
	 *
	 * @param Object	reference to Object reference
	 * @return reference to instance of this class
	 */
	FArchive& operator<<( UObject*& Object )
	{
		if ( Object != NULL && !Object->HasAnyFlags(RF_TagExp|RF_PendingKill|RF_Unreachable) )
		{
			Object->SetFlags(RF_TagExp);

			const UBOOL bIgnoreObject = 

				// skip serialization of classes unless the class was part of the set of objects to find references from
				Object->GetClass() == UClass::StaticClass() ||

				// No need to call Serialize from here for any objects that were part of our root set.
				// By preventing re-entrant serialization using the RF_TagImp flag (instead of just marking each object in the root set with
				// RF_TagExp prior to calling Serialize) we can determine which objects from our root set are being referenced
				// by other objects in our root set.
				Object->HasAnyFlags(RF_TagImp);

			if ( bIgnoreObject == FALSE )
			{
				Object->Serialize( *this );
			}
		}

		return *this;
	}
};

/**
 * Returns the list of container objects that will be used to filter which resources are displayed when "Show Referenced Only" is enabled.
 * Normally these will correspond to ULevels (i.e. the currently loaded levels), but may contain other packages such as script or UI packages.
 *
 * @param	out_ReferencerContainers	receives the list of container objects that will be used for filtering which objects are displayed in the GB
 *										when "In Use Only" is enabled
 */
void WxGenericBrowser::GetReferencerContainers( TArray<UObject*>& out_ReferencerContainers )
{
	if ( LeftContainer != NULL )
	{
		LeftContainer->GetReferencerContainers(out_ReferencerContainers);
	}
}

/**
 * Builds a list of source objects which should be used for determining which assets to display in the GB when "In Use Only" is displayed.  Only assets referenced
 * by this set will be displayed in the GB.
 *
 * @param	out_RootSet		receives the list of objects that will be searched for referenced assets.
 * @param	RootContainers	if specified, the list of container objects to use for generating the dependency root set.  If not specified, uses the result of GetReferencerContainers.
 */
void WxGenericBrowser::BuildDependencyRootSet( TArray<UObject*>& out_RootSet, TArray<UObject*>* RootContainers/*=NULL*/ )
{
	const FScopedBusyCursor BusyCursor;
	GWarn->BeginSlowTask(*LocalizeUnrealEd(TEXT("ReferenceFilter")), TRUE);

	TArray<UObject*> RootSet;
	if ( RootContainers == NULL )
	{
		GetReferencerContainers(RootSet);
	}

	TArray<UObject*>& ReferenceRoots = RootContainers != NULL ? *RootContainers : RootSet;

	// we need a quick way to whittle down our list of potential objects to evaluate; to do this, we'll first discard any objects which aren't
	// in the same package as any of the ReferenceRoots;
	// (tmaps have faster lookup than TArrays, so use those instead)
	TMap<UObject*,UObject*> RootPackageMap;
	TMap<UObject*,UObject*> ReferenceRootMap;

	for ( INT RootIndex = 0; RootIndex < ReferenceRoots.Num(); RootIndex++ )
	{
		UObject* RootObject = ReferenceRoots(RootIndex);
		if ( RootObject != NULL )
		{
			RootPackageMap.Set(RootObject->GetOutermost(), RootObject->GetOutermost());
			ReferenceRootMap.Set(RootObject, RootObject);
		}
	}

	// Iterate through all objects in memory looking for objects which are contained in the same package as the reference roots
	for ( FObjectIterator It; It; ++It )
	{
		UObject* Obj = *It;
		if( Obj->HasAnyFlags(RF_PendingKill|RF_Unreachable) )
		{
			continue;
		}

		if ( RootPackageMap.Find(Obj->GetOutermost()) != NULL )
		{
			// this object is contained within the same package as one of our ReferenceRoots; now we do the more expensive check to see if this object is actually
			// contained in one of the ReferenceRoots
			for ( UObject* ObjectOuter = Obj->GetOuter(); ObjectOuter; ObjectOuter = ObjectOuter->GetOuter() )
			{
				if ( ReferenceRootMap.Find(ObjectOuter) != NULL )
				{
					// this object was contained within one of our ReferenceRoots
					out_RootSet.AddItem(Obj);
					break;
				}
			}
		}
	}

	GWarn->EndSlowTask();
}

/**
 * Marks all objects which are referenced by objects contained in the current filter set with RF_TagExp
 *
 * @param	out_RootSet		if specified, receives the array of objects which were used as the root set when performing the reference checking
 */
void WxGenericBrowser::MarkReferencedObjects( TArray<UObject*>* out_RootSet/*=NULL*/ )
{
	const UBOOL bUsageFilterEnabled = IsUsageFilterEnabled();
	if ( bUsageFilterEnabled )
	{
		// clear all mark flags
		for ( FObjectIterator It; It; ++It )
		{
			It->ClearFlags(RF_TagExp|RF_TagImp);
		}

		TArray<UObject*> Referencers;
		TArray<UObject*>& OutputReferencers = out_RootSet != NULL ? *out_RootSet : Referencers;
		BuildDependencyRootSet(OutputReferencers);

		FArchiveReferenceMarker Marker(OutputReferencers);
	}
}

void WxGenericBrowser::OnSize( wxSizeEvent& In )
{
	// During the creation process a sizing message can be sent so don't
	// handle it until we are initialized
	if (bAreWindowsInitialized)
	{
		SplitterWindow->SetSize( GetClientRect() );
	}
}

/** Handler for IDM_RefreshBrowser events; updates the browser contents. */
void WxGenericBrowser::OnRefresh( wxCommandEvent& In )
{
	Update();
}

/**
 * Event handler for when the user selects an item from the Most Recently Used menu.
 */
void WxGenericBrowser::MenuFileMRU( wxCommandEvent& In )
{
	const FFilename NewFilename = ((WxMBGenericBrowser*)MenuBar)->MRUList->GetItem( In.GetId() - IDM_MRU_START );
	
	if( ((WxMBGenericBrowser*)MenuBar)->MRUList->VerifyFile( In.GetId() - IDM_MRU_START ) )
	{
		UPackage* Package = LoadPackage( NewFilename );

		Update();

		// Make sure only the package we recently imported is selected.
		LeftContainer->TreeCtrl->UnselectAll();
		SelectPackage( Package );

		// Reorder the menu now that we've opened a package.
		((WxMBGenericBrowser*)MenuBar)->MRUList->UpdateMenu();
	}
}

/**
 * Requests that source control state be updated.
 */
void WxGenericBrowser::RequestSCCUpdate()
{
	LeftContainer->RequestSCCUpdate();
}

/**
 * Imports all of the files in the string array passed in.
 *
 * @param InFiles					Files to import
 * @param InFactories				Array of UFactory classes to be used for importing.
 */
UBOOL WxGenericBrowser::ImportFiles( const wxArrayString& InFiles, const TArray<UFactory*> &InFactories )
{
	// Detach all components while importing objects.
	// This is necessary for the cases where the import replaces existing objects which may be referenced by the attached components.
	FGlobalComponentReattachContext ReattachContext;

	UObject* PackageObj = GetTopPackage();
	UObject* GroupObj = GetGroup();
	FString Package = PackageObj ? PackageObj->GetName() : TEXT( "MyPackage" );
	FString Group = GroupObj ? GroupObj->GetFullGroupName( 0 ) : TEXT( "" );

	UBOOL bSawSuccessfulImport = FALSE;		// Used to flag whether at least one import was successful
	UFactory* Factory = NULL;

	// Reset the 'Do you want to overwrite the existing object?' Yes to All / No to All prompt, to make sure the
	// user gets a chance to select something
	UFactory::ResetOverwriteYesOrNoToAllState();

	// For each filename, open up the import dialog and get required user input.
	WxDlgImportGeneric ImportDialog;
	for( UINT FileIndex = 0 ; FileIndex < InFiles.Count() ; FileIndex++ )
	{
		GWarn->StatusUpdatef( FileIndex, InFiles.Count(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd( "Importingf" ), FileIndex,InFiles.Count()) ) );

		const FFilename Filename = ( const TCHAR * )InFiles[FileIndex];
		LastImportPath = Filename.GetPath();

		// Find a set of factories used to import files with this extension
		TArray<UFactory*> Factories;

		Factories.Empty( InFactories.Num() );
		for( INT FactoryIdx = 0; FactoryIdx < InFactories.Num(); ++FactoryIdx )
		{
			for( UINT FormatIndex = 0; FormatIndex < ( UINT )InFactories( FactoryIdx )->Formats.Num(); FormatIndex++ )
			{
				if( InFactories( FactoryIdx )->Formats( FormatIndex ).Left( Filename.GetExtension().Len() ) == Filename.GetExtension() )
				{
					Factories.AddItem( InFactories( FactoryIdx ) );
					break;
				}
			}
		}

		// Handle the potential of multiple factories being found
		if( Factories.Num() == 1 )
		{
			Factory = Factories( 0 );
		}
		else if( Factories.Num() > 1 )
		{
			Factory = Factories( 0 );
			for( INT i = 0; i < Factories.Num(); i++ )
			{
				if( Factories( i )->FactoryCanImport( Filename ) )
				{
					Factory = Factories( i );
					break;
				}
			}
		}

		// Found or chosen a factory
		if( Factory )
		{
			if( ImportDialog.ShowModal( Filename, Package, Group, Factory->GetSupportedClass(), Factory, &RightContainer->bSuppressDraw ) == wxID_OK )
			{
				bSawSuccessfulImport = TRUE;
			}

			// Copy off the package and group for the next import.
			Package = ImportDialog.GetPackage();
			Group = ImportDialog.GetGroup();
		}
		else
		{
			// Couldn't find a factory for a class, so throw up an error message.
			appMsgf( AMT_OK, TEXT( "Unable to import file: %s\nCould not find an appropriate actor factory for this filetype." ), ( const TCHAR* )InFiles.Item( FileIndex ) );
		}
	}

	// Clean up the overwrite bit now that we're done importing files
	UFactory::ResetOverwriteYesOrNoToAllState();

	// Only update the generic browser if we imported something successfully.
	if( bSawSuccessfulImport )
	{
		ImportDialog.SyncGBToNewObjects();
		Update();
	}

	return bSawSuccessfulImport;
}

UPackage* WxGenericBrowser::LoadPackage( FFilename InFilename )
{
	// Detach all components while loading a package.
	// This is necessary for the cases where the load replaces existing objects which may be referenced by the attached components.
	FGlobalComponentReattachContext ReattachContext;

	// if the package is already loaded, reset it so it will be fully reloaded
	UObject* ExistingPackage = UObject::StaticFindObject(UPackage::StaticClass(), ANY_PACKAGE, *InFilename.GetBaseFilename());
	if (ExistingPackage)
	{
		UObject::ResetLoaders(ExistingPackage);
	}

	// record the name of this file to make sure we load objects in this package on top of in-memory objects in this package
	GEditor->UserOpenedFile = InFilename;

	// clear any previous load errors
	EdClearLoadErrors();

	UPackage* Package = Cast<UPackage>(UObject::LoadPackage( NULL, *InFilename, 0 ));

	// display any load errors that happened while loading the package
	if (GEdLoadErrors.Num())
	{
		GCallbackEvent->Send( CALLBACK_DisplayLoadErrors );
	}

	// reset the opened package to nothing
	GEditor->UserOpenedFile = FString("");

	// Update the MRU file list
	((WxMBGenericBrowser*)MenuBar)->MRUList->AddItem( InFilename );

	// If a script package was loaded, update the
	// actor browser in case a script package was loaded
	if ( Package && (Package->PackageFlags & PKG_ContainsScript) )
	{
		GCallbackEvent->Send( CALLBACK_RefreshEditor_ActorBrowser );
	}

	return Package;
}

void WxGenericBrowser::OnFileOpen( wxCommandEvent& In )
{
	// Get the filename
	const wxChar* FileTypes = TEXT("All Files|*.*");

	WxFileDialog OpenFileDialog( this, 
		*LocalizeUnrealEd("GenericBrowser_OpenPackageDialogCaption"), 
		*LastOpenPath,
		TEXT(""),
		FileTypes,
		wxOPEN | wxFILE_MUST_EXIST | wxMULTIPLE,
		wxDefaultPosition);

	if( OpenFileDialog.ShowModal() == wxID_OK )
	{
		const FScopedBusyCursor BusyCursor;

		TArray<UPackage*> NewPackages;

		wxArrayString OpenFilePaths;
		OpenFileDialog.GetPaths(OpenFilePaths);

		GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("LoadingPackage")), TRUE );
		for( UINT FileIndex = 0 ; FileIndex < OpenFilePaths.Count() ; ++FileIndex )
		{
			const FFilename Filename = (const TCHAR *)OpenFilePaths[FileIndex];
			GWarn->StatusUpdatef( (INT)FileIndex, (INT)OpenFilePaths.Count(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd("LoadingPackagef"), *Filename.GetBaseFilename()) ) );
			LastOpenPath = Filename.GetPath();

			UPackage* Package = LoadPackage( Filename );
			if( Package )
			{
				NewPackages.AddItem( Package );
			}
		}
		GWarn->EndSlowTask();

		// Now is a good time to sychronize source control state.
		LeftContainer->RequestSCCUpdate();

		// Refresh.
		Update();

		// Select the newly imported packages.
		LeftContainer->TreeCtrl->UnselectAll();
		for( INT NewPackageIndex = 0 ; NewPackageIndex < NewPackages.Num() ; ++NewPackageIndex )
		{
			UPackage* Package = NewPackages(NewPackageIndex);
			SelectPackage( Package );
		}
	}

	((WxMBGenericBrowser*)MenuBar)->MRUList->UpdateMenu();
}

void WxGenericBrowser::OnFileOpenPackages( wxCommandEvent& In )
{
	WxDlgOpenPackages dlg;
	TArray<UPackage*> NewPackages;

	if( dlg.ShowModal() == wxID_OK)
	{
		const FScopedBusyCursor BusyCursor;

		GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("LoadingPackage")), TRUE );
		for( INT x = 0 ; x < dlg.SelectedItems.Num() ; ++x )
		{
			FFilename Filename = dlg.SelectedItems(x);

			if( Filename.GetExtension().Len() )
			{
				GWarn->StatusUpdatef( 0, 0, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("LoadingPackagef"), *Filename.GetBaseFilename()) ) );
				UPackage* pkg = LoadPackage( Filename );
				NewPackages.AddItem( pkg );
			}
			else
			{
				TArray<FString> PackageFilenames;
				appFindFilesInDirectory( PackageFilenames, *Filename, TRUE, FALSE );

				for( INT p = 0 ; p < PackageFilenames.Num() ; ++p )
				{
					FFilename Filename = PackageFilenames(p);
					GWarn->StatusUpdatef( 0, 0, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("LoadingPackagef"), *Filename.GetBaseFilename()) ) );
					UPackage* pkg = LoadPackage( PackageFilenames(p) );
					NewPackages.AddItem( pkg );
				}
			}
		}
		GWarn->EndSlowTask();

		// Now is a good time to synchronize source control state.
		LeftContainer->RequestSCCUpdate();

		// Refresh.
		Update();

		// Select the newly loaded packages

		LeftContainer->TreeCtrl->UnselectAll();
		for( INT x = 0 ; x < NewPackages.Num() ; ++x )
		{
			UPackage* pkg = NewPackages(x);
			SelectPackage( pkg );
		}

		((WxMBGenericBrowser*)MenuBar)->MRUList->UpdateMenu();
	}
}

#if WITH_FACEFX
/**
 * If FaceFX is open, gives the user the option to close it.
 *
 * @return						TRUE if FaceFX is closed.
 */
static UBOOL CloseFaceFX()
{
	UBOOL bCloseFaceFX = TRUE;

	wxWindow* StudioWin = OC3Ent::Face::FxStudioApp::GetMainWindow();
	if ( StudioWin )
	{
		bCloseFaceFX = appMsgf( AMT_YesNo, *LocalizeUnrealEd("Prompt_CloseFXStudioOpenOnSaveQ") );
		if ( bCloseFaceFX )
		{
			StudioWin->Close();
		}
	}

	return bCloseFaceFX;
}
#endif

/**
* Attempts to save all selected packages.
*
* @return		TRUE if all selected packages were successfully saved, FALSE otherwise.
*/
UBOOL WxGenericBrowser::SaveSelectedPackages()
{
	// Generate a list of unique packages.
	TArray<UPackage*> SelectedPackages;
	LeftContainer->GetSelectedPackages( &SelectedPackages );

	// Disallow export if any packages are cooked.
	if (DisallowOperationOnCookedPackages(SelectedPackages))
	{
		return FALSE;
	}

	return SavePackages( SelectedPackages, FALSE );
}


/**
 * Returns whether saving the specified package is allowed
 */
UBOOL WxGenericBrowser::AllowPackageSave( UPackage* PackageToSave )
{
	// DB: We explicitly don't check for cooked packages here, because
	// DB: this code needs to work for packages being saved for PIE.
	return LeftContainer->CurrentResourceType->IsSavePackageAllowed(PackageToSave);
}

/**
 * Attempts to save the specified packages; helper function for by e.g. SaveSelectedPackages().
 *
 * @param		InPackages					The content packages to save.
 * @param		bUnloadPackagesAfterSave	If TRUE, unload each package if it was saved successfully.
 * @return									TRUE if all packages were saved successfully, FALSE otherwise.
 */
UBOOL WxGenericBrowser::SavePackages(const TArray<UPackage*>& InPackages, UBOOL bUnloadPackagesAfterSave)
{
#if WITH_FACEFX
	// FaceFX must be closed before saving.
	if ( !CloseFaceFX() )
	{
		return FALSE;
	}
#endif

	// Get outermost packages, in case groups were selected.
	TArray<UPackage*> Packages;
	for( INT PackageIndex = 0 ; PackageIndex < InPackages.Num() ; ++PackageIndex )
	{
		UPackage* Package = InPackages(PackageIndex);
		Packages.AddUniqueItem( Package->GetOutermost() ? (UPackage*)Package->GetOutermost() : Package );
	}

	// Packages must be fully loaded before they can be saved.
	if( !HandleFullyLoadingPackages( Packages, TEXT("Save") ) )
	{
		return FALSE;
	}

	// Assume that all packages were saved.
	UBOOL bAllPackagesWereSaved = TRUE;

	// Present the user with a 'save file' dialog.
	FString FileTypes( TEXT("All Files|*.*") );
	for(INT i=0; i<GSys->Extensions.Num(); i++)
	{
		FileTypes += FString::Printf( TEXT("|(*.%s)|*.%s"), *GSys->Extensions(i), *GSys->Extensions(i) );
	}

	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("SavingPackage")), TRUE );
	for(INT i=0; i<Packages.Num(); i++)
	{
		UPackage* PackageToSave = Packages(i);
		GWarn->StatusUpdatef( i, Packages.Num(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd("SavingPackagef"), *PackageToSave->GetName() )) );

		// Prevent level packages from being saved via the Generic Browser.
		if( FindObject<UWorld>( PackageToSave, TEXT("TheWorld") ) )
		{
			appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_CantSaveMapViaGB"), *PackageToSave->GetName()) );
			bAllPackagesWereSaved = FALSE;
		}
		else
		{
			FString SaveFileName;

			FString PackageName = PackageToSave->GetName();
			FString ExistingFile;

			if( GPackageFileCache->FindPackageFile( *PackageName, NULL, ExistingFile ) )
			{
				// If the package already exists, save to the existing filename.
				SaveFileName = ExistingFile;
			}
			else
			{
				// Couldn't find the package name; present a file dialog so the user can specify a name.
				const FString File = FString::Printf( TEXT("%s.upk"), *PackageName );
				debugf(TEXT("NO_EXISTING: %s, %s, %s"), *ExistingFile, *File, *LastSavePath);

				WxFileDialog SaveFileDialog(this, 
					*LocalizeUnrealEd("SavePackage"), 
					*LastSavePath,
					*File,
					*FileTypes,
					wxSAVE,
					wxDefaultPosition);

				if( SaveFileDialog.ShowModal() == wxID_OK )
				{
					LastSavePath = SaveFileDialog.GetDirectory();
					SaveFileName = FString( SaveFileDialog.GetPath() );
				}
			}

			if ( SaveFileName.Len() )
			{
				const FScopedBusyCursor BusyCursor;
				debugf(TEXT("saving as %s"), *SaveFileName);
				if( GFileManager->IsReadOnly( *SaveFileName ) )
				{
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_CouldntWriteToFile_F"), *SaveFileName)) );
					bAllPackagesWereSaved = FALSE;
				}
				else if( !GUnrealEd->Exec( *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\""), *PackageName, *SaveFileName) ) )
				{
					// Couldn't save.
					appMsgf( AMT_OK, *LocalizeUnrealEd("Error_CouldntSavePackage") );
					bAllPackagesWereSaved = FALSE;
				}
				else
				{
					// The package saved successfully.  Unload it?
					if ( bUnloadPackagesAfterSave )
					{
						TArray<UPackage*> PackagesToUnload;
						PackagesToUnload.AddItem( PackageToSave );
						UnloadTopLevelPackages( PackagesToUnload );
					}
				}
			}
			else
			{
				// Couldn't save; no filename to save as.
				bAllPackagesWereSaved = FALSE;
			}
		}
	}

	GWarn->EndSlowTask();
	Update();

	return bAllPackagesWereSaved;
}

/**
* Attempts a 'save as' operation on all selected packages.
*
* @return		TRUE if all selected packages were successfully saved, FALSE otherwise.
*/
UBOOL WxGenericBrowser::SaveAsSelectedPackages()
{
#if WITH_FACEFX
	// FaceFX must be closed before saving.
	if ( !CloseFaceFX() )
	{
		return FALSE;
	}
#endif

	// Generate a list of unique packages.
	TArray<UPackage*> WkPackages;
	LeftContainer->GetSelectedPackages(&WkPackages);

	// Get outermost packages, in case groups were selected.
	TArray<UPackage*> Packages;
	for( INT PackageIndex = 0 ; PackageIndex < WkPackages.Num() ; ++PackageIndex )
	{
		UPackage* Package = WkPackages(PackageIndex);
		Packages.AddUniqueItem( Package->GetOutermost() ? (UPackage*)Package->GetOutermost() : Package );
	}

	// Disallow 'save as' if any packages are cooked.
	for( INT PackageIndex = 0 ; PackageIndex < Packages.Num() ; ++PackageIndex )
	{
		const UPackage* Package = Packages(PackageIndex);
		if( Package->PackageFlags & PKG_Cooked )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
			return FALSE;
		}
	}

	// Packages must be fully loaded before they can be saved.
	if( !HandleFullyLoadingPackages( Packages, TEXT("Save") ) )
	{
		return FALSE;
	}

	// Assume that all packages were saved.
	UBOOL bAllPackagesWereSaved = TRUE;

	// Present the user with a 'save file' dialog.
	FString FileTypes( TEXT("All Files|*.*") );
	for(INT i=0; i<GSys->Extensions.Num(); i++)
	{
		FileTypes += FString::Printf( TEXT("|(*.%s)|*.%s"), *GSys->Extensions(i), *GSys->Extensions(i) );
	}

	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("SavingPackage")), TRUE );
	for(INT i=0; i<Packages.Num(); i++)
	{
		GWarn->StatusUpdatef( i, Packages.Num(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd("SavingPackagef"), *Packages(i)->GetName() )) );

		// Prevent level packages from being saved via the Generic Browser.
		if( FindObject<UWorld>( Packages(i), TEXT("TheWorld") ) )
		{
			appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_CantSaveMapViaGB"), *Packages(i)->GetName()) );
			bAllPackagesWereSaved = FALSE;
		}
		else
		{
			// SaveAs always presents a dialog; propose a filename.
			FString ExistingFile, File, Directory(LastSavePath);
			FString PackageName = Packages(i)->GetName();

			if( GPackageFileCache->FindPackageFile( *PackageName, NULL, ExistingFile ) )
			{
				FString Filename, Extension;
				GPackageFileCache->SplitPath( *ExistingFile, Directory, Filename, Extension );
				File = FString::Printf( TEXT("%s.%s"), *Filename, *Extension );
			}
			else
			{
				Directory = TEXT("");
				File = FString::Printf( TEXT("%s.upk"), *PackageName );
			}

			WxFileDialog SaveFileDialog( this, 
				*LocalizeUnrealEd("SavePackage"), 
				*Directory,
				*File,
				*FileTypes,
				wxSAVE,
				wxDefaultPosition);

			if( SaveFileDialog.ShowModal() == wxID_OK )
			{
				const FScopedBusyCursor BusyCursor;
				LastSavePath = SaveFileDialog.GetDirectory();
				const FString SaveFileName = FString( SaveFileDialog.GetPath() );
				if( GFileManager->IsReadOnly( *SaveFileName ) )
				{
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_CouldntWriteToFile_F"), *SaveFileName)) );
					bAllPackagesWereSaved = FALSE;
				}
				else if( !GUnrealEd->Exec( *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\""), *PackageName, *SaveFileName) ) )
				{
					// Couldn't save.
					appMsgf( AMT_OK, *LocalizeUnrealEd("Error_CouldntSavePackage") );
					bAllPackagesWereSaved = FALSE;
				}
			}
			else
			{
				// User cancelled the save.
				bAllPackagesWereSaved = FALSE;
			}
		}
	}

	GWarn->EndSlowTask();
	Update();

	return bAllPackagesWereSaved;
}

void WxGenericBrowser::OnFileSave( wxCommandEvent& In )
{
	SaveSelectedPackages();
}

void WxGenericBrowser::OnFileSaveAs( wxCommandEvent& In )
{
	SaveAsSelectedPackages();
}

void WxGenericBrowser::OnFileFullyLoad( wxCommandEvent& In )
{
	// Generate a list of unique packages.
	TArray<UPackage*> WkPackages;
	LeftContainer->GetSelectedPackages(&WkPackages);

	// Get outermost packages, in case groups were selected.
	TArray<UPackage*> TopLevelPackages;
	for( INT PackageIndex=0; PackageIndex<WkPackages.Num(); PackageIndex++ )
	{
		UPackage* WkPackage = WkPackages(PackageIndex);
		TopLevelPackages.AddUniqueItem( WkPackage->GetOutermost() );
	}

	// Fully load selected packages.
	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("FullyLoadingPackages")), TRUE );
	const FScopedBusyCursor BusyCursor;
	for( INT PackageIndex=0; PackageIndex<WkPackages.Num(); PackageIndex++ )
	{
		UPackage* TopLevelPackage = TopLevelPackages(PackageIndex);
		TopLevelPackage->FullyLoad();
	}
	GWarn->EndSlowTask();

	// Refresh.
	Update();
}

UPackage* WxGenericBrowser::GetTopPackage()
{
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedPackages(&Packages);

	UPackage* pkg = Packages.Num() ? Packages(0) : NULL;

	// Walk the package chain to the top to get the proper package.

	if( pkg && pkg->GetOuter() )
	{
		pkg = (UPackage*)pkg->GetOutermost();
	}

	return pkg;
}

UPackage* WxGenericBrowser::GetGroup()
{
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedPackages(&Packages);

	UPackage* grp = Packages.Num() ? Packages(0) : NULL;

	// If the selected package has an outer, it must be a group.

	if( grp && grp->GetOuter() )
	{
		return grp;
	}

	// If we get here, the user has a top level package selected, so there is no valid group to pass back.

	return NULL;
}

/**
 * Selects the resources specified by ObjectArray and scrolls the viewport so that at least one selected resource is in view.
 *
 * @param	Array		The array of resources to sync to.  Must be an array of valid pointers.
 */
void WxGenericBrowser::SyncToObjects(TArray<UObject*> Objects)
{
	if ( Objects.Num() > 0 )
	{
		// If any of the selected object types don't fall under our filter,
		// then change the current type to "All" so that the object is guaranteed to show up.
		for ( INT ObjectIndex = 0 ; ObjectIndex < Objects.Num() ; ++ObjectIndex )
		{
			UObject* Object					= Objects(ObjectIndex);
			const UBOOL bObjectSupported	= LeftContainer->CurrentResourceType->Supports( Object );
			if( !bObjectSupported )
			{
				LeftContainer->SetShowAllTypes( TRUE );
				break;
			}
		}

		// Clear the filter field.
		RightContainer->DisableUpdate();
		RightContainer->NameFilter = TEXT("");
		RightContainer->ToolBar->SetNameFilterText( TEXT("") );
		RightContainer->EnableUpdate();
		RightContainer->Update();

		// Deselect all packages in the tree view.
		LeftContainer->TreeCtrl->UnselectAll();

		// Sync the windows.
		LeftContainer->SyncToObject( Objects(0) );
		RightContainer->SyncToObject( Objects(0) );

		for ( INT ObjectIndex = 1 ; ObjectIndex < Objects.Num() ; ++ObjectIndex )
		{
			UObject* Object = Objects(ObjectIndex);
			LeftContainer->SyncToObject( Object );
			RightContainer->SyncToObject( Object, FALSE );
		}

		// Update the list boxes and thumbnail views to reflect the selected items.
		RightContainer->Update();
	}
}

/**
 * Called when there is a Callback issued.
 * @param InType	The type of callback that was issued.
 */
void WxGenericBrowser::Send( ECallbackEventType InType )
{
	switch(InType)
	{
	case CALLBACK_SelChange:
		SelectionChangeNotify();
		break;

	default:
		// Make sure to route the call to the base class.
		WxBrowser::Send(InType);
		break;
	}

}

/**
 * Called when there is a Callback issued.
 * @param InType	The type of callback that was issued.
 * @param InObject	Object that was modified.
 */
void WxGenericBrowser::Send( ECallbackEventType InType, UObject* InObject )
{
	switch(InType)
	{
	case CALLBACK_SelectObject:
		SelectionChangeNotify();
		break;

	default:
		break;
	}
}

/**
 * Callback for when the Generic Browser selection set changes.
 */
void WxGenericBrowser::SelectionChangeNotify()
{
	// Count the number of objects selected in the generic browser.
	INT Count = 0;

	USelection* SelectedObjects = GetGBSelection();
	const INT SelectedObjectCount = SelectedObjects->Num();

	for ( INT SelectedObjectIndex = 0 ; SelectedObjectIndex < SelectedObjectCount ; SelectedObjectIndex++ )
	{
		UObject* SelectedObject = (*SelectedObjects)(SelectedObjectIndex);
		if( SelectedObject )
		{
			if ( !SelectedObject->IsA(AActor::StaticClass()) )
			{
				UTexture2D* Texture2D = Cast<UTexture2D>(SelectedObject);
				if (Texture2D)
				{
					if (Texture2D != GEditor->StreamingBoundsTexture)
					{
						GEditor->SetStreamingBoundsTexture(NULL);
					}
				}
				Count++;
			}
			else if ( !SelectedObject->HasAnyFlags(RF_ArchetypeObject) || SelectedObject->IsAPrefabArchetype() )
			{
				debugf( NAME_Log, TEXT("Actor %s(%s) appearing in the generic browser selection set"),
					*SelectedObject->GetName(), *SelectedObject->GetClass()->GetName() );
			}
		}
	}

	NumSelectedObjects = Count;
}


/**
 * Handles fully loading packages for a set of passed in objects.
 *
 * @param	Objects				Array of objects whose packages need to be fully loaded
 * @param	OperationString		Localization key for a string describing the operation; appears in the warning string presented to the user.
 * 
 * @return TRUE if all packages where fully loaded, FALSE otherwise
 */
UBOOL WxGenericBrowser::HandleFullyLoadingPackages( const TArray<UObject*>& Objects, const TCHAR* OperationString )
{
	// Get list of outermost packages.
	TArray<UPackage*> TopLevelPackages;
	for( INT ObjectIndex=0; ObjectIndex<Objects.Num(); ObjectIndex++ )
	{
		UObject* Object = Objects(ObjectIndex);
		if( Object )
		{
			TopLevelPackages.AddUniqueItem( Object->GetOutermost() );
		}
	}

	return HandleFullyLoadingPackages( TopLevelPackages, OperationString );
}

/**
 * Handles fully loading passed in packages.
 *
 * @param	TopLevelPackages	Packages to be fully loaded.
 * @param	OperationString		Localization key for a string describing the operation; appears in the warning string presented to the user.
 * 
 * @return TRUE if all packages where fully loaded, FALSE otherwise
 */
UBOOL WxGenericBrowser::HandleFullyLoadingPackages( const TArray<UPackage*>& TopLevelPackages, const TCHAR* OperationString )
{
	UBOOL bSuccessfullyCompleted = TRUE;

	// Make sure they are all fully loaded.
	UBOOL bNeedsUpdate = FALSE;
	for( INT PackageIndex=0; PackageIndex<TopLevelPackages.Num(); PackageIndex++ )
	{
		UPackage* TopLevelPackage = TopLevelPackages(PackageIndex);
		check( TopLevelPackage );
		check( TopLevelPackage->GetOuter() == NULL );

		if( !TopLevelPackage->IsFullyLoaded() )
		{
			// Ask user to fully load.
			if( appMsgf( AMT_YesNo, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("NeedsToFullyLoadPackageF")), *TopLevelPackage->GetName(), *LocalizeUnrealEd(OperationString)) ) ) )
			{
				// Fully load package.
				const FScopedBusyCursor BusyCursor;
				GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("FullyLoadingPackages")), TRUE );
				TopLevelPackage->FullyLoad();
				GWarn->EndSlowTask();
				bNeedsUpdate = TRUE;
			}
			// User declined abort operation.
			else
			{
				bSuccessfullyCompleted = FALSE;
				debugf(TEXT("Aborting operation as %s was not fully loaded."),*TopLevelPackage->GetName());
				break;
			}
		}
	}

	// Refresh generic browser as we loaded assets.
	if( bNeedsUpdate )
	{
		Update();
	}

	return bSuccessfullyCompleted;
}

void WxGenericBrowser::OnFileNew( wxCommandEvent& In )
{
	// Detach all components while creating new objects.
	// This is necessary for the cases where the creation replaces existing objects which may be referenced by the attached components.
	FGlobalComponentReattachContext ReattachContext;

	UObject* pkg = GetTopPackage();
	UObject* grp = GetGroup();

	WxDlgNewGeneric dlg;
	if ( dlg.ShowModal( pkg ? *pkg->GetName() : TEXT("MyPackage"), grp ? *grp->GetName() : TEXT("") ) == wxID_OK )
	{
		const FScopedBusyCursor BusyCursor;

		// Now is a good time to sychronize source control state.
		LeftContainer->RequestSCCUpdate();

		// Refresh
		Update();
	}
}

void WxGenericBrowser::OnContextObjectNew(wxCommandEvent& In)
{
	INT FactoryClassIndex = In.GetId() - ID_NEWOBJ_START;
	check( FactoryClassIndex >= 0 && FactoryClassIndex < FactoryNewClasses.Num() );

	UClass* FactoryClass = FactoryNewClasses(FactoryClassIndex);
	check( FactoryClass->IsChildOf(UFactory::StaticClass()) );

	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedPackages(&Packages);

	WxDlgNewGeneric Dialog;
	UPackage* Package = GetTopPackage();
	UPackage* Group = GetGroup();
	if( Dialog.ShowModal( Package ? *Package->GetName() : TEXT("MyPackage"), Group ? Group->GetFullGroupName( 0 ) : TEXT(""), FactoryClass ) == wxID_OK )
	{
		UFactory* Factory = ( UFactory* )FactoryClass->GetDefaultObject();
		if( Factory->bEditAfterNew )
		{
			const FScopedBusyCursor BusyCursor;
			UGenericBrowserType* BrowserType = ConstructObject<UGenericBrowserType>( UGenericBrowserType_All::StaticClass() );
			BrowserType->Init();
			BrowserType->ShowObjectEditor();
		}
		Update();
	}
}

void WxGenericBrowser::OnObjectEditor( wxCommandEvent& In )
{
	const INT WarningThreshold = 5;
	const INT NumSelected = GetGBSelection()->Num();
	UBOOL bProceed = FALSE;

	if(NumSelected >= 5)
	{
		bProceed = appMsgf( AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("OpenObjectEditorAreYouSure_F"), NumSelected) );
	}
	else
	{
		bProceed = TRUE;
	}

	if(bProceed == TRUE)
	{
		LeftContainer->CurrentResourceType->ShowObjectEditor();
	}
}

IMPLEMENT_COMPARE_CONSTREF(FName,GenericBrowser, { return appStricmp(*A.GetNameString(),*B.GetNameString()); })

/**
 * Dialog for modifying tags based on a group of selected objects.
 */
WxDlgAddTag::WxDlgAddTag(FTagInfo* InTagInfo, TArray<FName> &SelectedTags)
{
	TagInfo = InTagInfo;

	wxDialog::Create(NULL,wxID_ANY,TEXT("AddTag"),wxDefaultPosition,wxSize(300,200),wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

	SetLabel(*FString::Printf(TEXT("%s: %s"),*LocalizeUnrealEd(TEXT("AddTag")),*TagInfo->AssetTypeName));

	wxBoxSizer *MainSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		TagList = new wxCheckListBox(this,-1,wxDefaultPosition,wxDefaultSize,0,wxLB_EXTENDED);

		// build a sorted tag list
		SortedTags = TagInfo->Tags;
		Sort<USE_COMPARE_CONSTREF(FName,GenericBrowser)>(&SortedTags(0),SortedTags.Num());
		for (INT Idx = 0; Idx < SortedTags.Num(); Idx++)
		{
			TagList->Append(*SortedTags(Idx).GetNameString());
		}

		// pre-select any existing tags
		for (INT Idx = 0; Idx < SelectedTags.Num(); Idx++)
		{
			FName Tag = SelectedTags(Idx);
			INT TagIdx = SortedTags.FindItemIndex(Tag);
			if (TagIdx != INDEX_NONE)
			{
				TagList->Check((UINT)TagIdx);
			}
		}

		wxBoxSizer* ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			wxButton* ButtonOK = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonOK->SetDefault();
			ButtonSizer->Add(ButtonOK, 0, wxALIGN_TOP|wxALL, 5);

			wxButton* ButtonCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonSizer->Add(ButtonCancel, 0, wxALIGN_TOP|wxALL, 5);
		}

		MainSizer->Add(TagList,1,wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxALL|wxEXPAND,15);
		MainSizer->Add(ButtonSizer,0,wxALIGN_TOP|wxALL,15);
	}
	SetSizer(MainSizer);
	MainSizer->Fit(this);

	FWindowUtil::LoadPosSize( TEXT("AddTagDialog"), this, -1, -1, 300, 200 );
}

WxDlgAddTag::~WxDlgAddTag()
{
	FWindowUtil::SavePosSize( TEXT("AddTagDialog"), this );
}

/** Gets the results of the dialog. */
void WxDlgAddTag::GetSelectedTags(TArray<FName> &OutTags)
{
	if (TagList != NULL)
	{
		// build a list of the checked items
		TArray<INT> CheckedIndices;
		for (UINT Idx = 0; Idx < TagList->GetCount(); Idx++)
		{
			if (TagList->IsChecked(Idx))
			{
				CheckedIndices.AddItem((INT)Idx);
			}
		}
		if (CheckedIndices.Num() > 0)
		{
			// use those indices to get the actual tag FNames
			for (INT Idx = 0; Idx < CheckedIndices.Num(); Idx++)
			{
				OutTags.AddUniqueItem(SortedTags(CheckedIndices(Idx)));
			}
		}
	}
}


/**
 * Iterates through selected objects and displays tag selection dialog for each, which is then
 * applied to each object.
 */
void AddTagsToSelectedAssets(USelection *Selection)
{
	if (Selection != NULL)
	{
		UContentTagIndex *DefaultIndex = UContentTagIndex::StaticClass()->GetDefaultObject<UContentTagIndex>();
		// for each supported class show the tag dialog and apply the results
		static UClass* SupportedClasses[] = {UStaticMesh::StaticClass(),UParticleSystem::StaticClass(),UMaterial::StaticClass()};
		UBOOL bNeedsSave = FALSE;
		for (INT ClassIdx = 0; ClassIdx < ARRAY_COUNT(SupportedClasses); ClassIdx++)
		{
			UClass *Class = SupportedClasses[ClassIdx];
			TArray<UObject*> SelectedObjs;
			if (Selection->GetSelectedObjects(Class,SelectedObjs) > 0)
			{
				// find the default tag info
				FTagInfo *TagInfo = DefaultIndex->FindDefaultTagInfoForClass(Class);
				if (TagInfo == NULL)
				{
					warnf(TEXT("Failed to find tag info for asset %s"), *(Class->GetName()));
					continue;
				}

				TArray<FName> ExistingTags;
				UContentTagIndex::GetTagsFromObjects(SelectedObjs,Class,ExistingTags);
				WxDlgAddTag dlg(TagInfo,ExistingTags);
				INT Result = dlg.ShowModal();
				if (Result == wxID_OK)
				{
					TArray<FName> Tags;
					dlg.GetSelectedTags(Tags);
					UContentTagIndex::ApplyTagsToObjects(SelectedObjs,Class,Tags);
					bNeedsSave = TRUE;
				}
			}
		}
		if (bNeedsSave)
		{
			UContentTagIndex::SaveLocalContentTagIndex();
		}
	}
}

void WxGenericBrowser::OnAddTag(wxCommandEvent& In )
{
	AddTagsToSelectedAssets(GetGBSelection());
}

/**
 * Does a depth-first traversal of the sound node tree rooted at root, making a list of all nodes
 * whose type matches NodeType.
 */
static void GetAllNodesInCue(USoundNode* Root, UClass* NodeType, TArray<USoundNode*>& OutList)
{
	check( NodeType->IsChildOf(USoundNode::StaticClass()) );

	TArray<USoundNode*> NodesToProcess;
	NodesToProcess.AddItem( Root );

	while ( NodesToProcess.Num() > 0 )
	{
		USoundNode* CurNode = NodesToProcess.Pop();
		if ( CurNode )
		{
			// Push child nodes.
			for ( INT ChildIndex = 0 ; ChildIndex < CurNode->ChildNodes.Num() ; ++ChildIndex )
			{
				NodesToProcess.AddItem( CurNode->ChildNodes(ChildIndex) );
			}

			// Record this node if it is of the appropriate type.
			if ( CurNode->IsA( NodeType ) )
			{
				OutList.AddUniqueItem( CurNode );
			}
		}
	}
}

void WxGenericBrowser::OnObjectEditNodes( wxCommandEvent& In )
{
	TArray<UObject*>		SelectedObjects;
	GEditor->GetSelectedObjects()->GetSelectedObjects(SelectedObjects);

	TArray<USoundNode*>		NodeList;
	for ( INT SelectedObjectIndex = 0 ; SelectedObjectIndex < SelectedObjects.Num() ; ++SelectedObjectIndex )
	{
		USoundCue* SoundCue = Cast<USoundCue>( SelectedObjects(SelectedObjectIndex) );
		if ( SoundCue )
		{
			GetAllNodesInCue( SoundCue->FirstNode, USoundNodeAttenuation::StaticClass(), NodeList );
		}
	}

	if ( NodeList.Num() > 0 )
	{
		if(!GApp->ObjectPropertyWindow)
		{
			GApp->ObjectPropertyWindow = new WxPropertyWindowFrame;
			GApp->ObjectPropertyWindow->Create( GApp->EditorFrame, -1, TRUE, GUnrealEd );
			GApp->ObjectPropertyWindow->SetSize( 64,64, 350,600 );
		}
		GApp->ObjectPropertyWindow->SetObjectArray( NodeList, TRUE, TRUE, TRUE );
		GApp->ObjectPropertyWindow->Show();
	}
}

void WxGenericBrowser::OnObjectResliceFracturedMeshes( wxCommandEvent& In )
{
	TArray<UObject*> SelectedObjects;
	GEditor->GetSelectedObjects()->GetSelectedObjects(SelectedObjects);

	for(INT ObjectIndex = 0;ObjectIndex < SelectedObjects.Num();ObjectIndex++)
	{
		UFracturedStaticMesh* FracturedStaticMesh = Cast<UFracturedStaticMesh>(SelectedObjects(ObjectIndex));
		if(FracturedStaticMesh)
		{
			// Open a static mesh editor window for the mesh.
			WxStaticMeshEditor* StaticMeshEditor = new WxStaticMeshEditor( GApp->EditorFrame, -1, FracturedStaticMesh, FALSE );
			
			// Reslice the mesh.
			StaticMeshEditor->SliceMesh();

			// Close the static mesh editor window.
			StaticMeshEditor->Close();
		}
	}
}

void WxGenericBrowser::OnObjectCustomCommand( wxCommandEvent& In )
{
	LeftContainer->CurrentResourceType->InvokeCustomCommand( In.GetId() );
}

void WxGenericBrowser::OnFileImport( wxCommandEvent& In )
{
	FString FileTypes, AllExtensions;
	TArray<UFactory*> Factories;			// All the factories we can possibly use

	// Get the list of file filters from the factories

	for( TObjectIterator<UClass> It ; It ; ++It )
	{
		UClass* CurrentClass = (*It);

		if( CurrentClass->IsChildOf(UFactory::StaticClass()) && !(CurrentClass->ClassFlags & CLASS_Abstract) )
		{
			UFactory* Factory = ConstructObject<UFactory>( CurrentClass );
			if( Factory->bEditorImport && !Factory->bCreateNew )
			{
				Factories.AddItem( Factory );
				for( INT x = 0 ; x < Factory->Formats.Num() ; ++x )
				{
					FString Wk = Factory->Formats(x);

					TArray<FString> Components;
					Wk.ParseIntoArray( &Components, TEXT(";"), 0 );

					for( INT c = 0 ; c < Components.Num() ; c += 2 )
					{
						FString Ext = Components(c),
							Desc = Components(c+1),
							Line = FString::Printf( TEXT("%s (*.%s)|*.%s"), *Desc, *Ext, *Ext );

						if(AllExtensions.Len())
							AllExtensions += TEXT(";");
						AllExtensions += FString::Printf(TEXT("*.%s"), *Ext);

						if(FileTypes.Len())
							FileTypes += TEXT("|");
						FileTypes += Line;
					}
				}
			}
		}
	}

	FileTypes = FString::Printf(TEXT("All Files (%s)|%s|%s"),*AllExtensions,*AllExtensions,*FileTypes);

	// Prompt the user for the filenames

	WxFileDialog OpenFileDialog( this, 
		*LocalizeUnrealEd("Import"),
		*LastImportPath,
		TEXT(""),
		*FileTypes,
		wxOPEN | wxFILE_MUST_EXIST | wxMULTIPLE,
		wxDefaultPosition);

	OpenFileDialog.SetFilterIndex(LastImportFilter);
	if( OpenFileDialog.ShowModal() == wxID_OK )
	{
		GWarn->BeginSlowTask( *LocalizeUnrealEd("Importing"), TRUE );

		LastImportFilter = OpenFileDialog.GetFilterIndex();

		wxArrayString OpenFilePaths;
		OpenFileDialog.GetPaths( OpenFilePaths );

		if(ImportFiles( OpenFilePaths, Factories ))
		{
			// Lightwave importer seems to create some extra, empty static meshes for some reason.  Garbage collection makes them go away.
			UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );
		}

		GWarn->EndSlowTask();
	}
}

/**
 * Copies references for selected generic browser objects to the clipboard.
 */
void WxGenericBrowser::CopyReferences() const
{
	// Build the list of objects that are selected
	TArray<UObject*> SelectedObjects;
	GetSelectedItems( LeftContainer->CurrentResourceType->SupportInfo, SelectedObjects );

	FString Ref;
	for ( INT Index = 0 ; Index < SelectedObjects.Num() ; ++Index )
	{
		if( Ref.Len() )
		{
			Ref += LINE_TERMINATOR;
		}
		Ref += SelectedObjects(Index)->GetPathName();
	}

	appClipboardCopy( *Ref );
}

void WxGenericBrowser::OnCopyReference(wxCommandEvent& In)
{
	CopyReferences();
}

void WxGenericBrowser::OnObjectProperties(wxCommandEvent& In)
{
	// Call through to the appropriate GenericBrowserType, so that browser types
	// can provide their own notify hooks, etc.
	LeftContainer->CurrentResourceType->ShowObjectProperties();
}

/**
 * An archive for collecting object references that are top-level objects.
 */
class FArchiveTopLevelReferenceCollector : public FArchive
{
public:
	FArchiveTopLevelReferenceCollector(
		TArray<UObject*>* InObjectArray,
		const TArray<UObject*>& InIgnoreOuters,
		const TArray<UClass*>& InIgnoreClasses)
			:	ObjectArray( InObjectArray )
			,	IgnoreOuters( InIgnoreOuters )
			,	IgnoreClasses( InIgnoreClasses )
	{
		// Mark objects.
		for ( FObjectIterator It ; It ; ++It )
		{
			if ( ShouldSearchForAssets(*It) )
			{
				It->SetFlags(RF_TagExp);
			}
			else
			{
				It->ClearFlags(RF_TagExp);
			}
		}
	}

	/** @return		TRUE if the specified objects should be serialized to determine asset references. */
	FORCEINLINE UBOOL ShouldSearchForAssets(const UObject* Object) const
	{
		// Discard class default objects.
		if ( Object->IsTemplate(RF_ClassDefaultObject) )
		{
			return FALSE;
		}
		// Check to see if we should be based on object class.
		if ( IsAnIgnoreClass(Object) )
		{
			return FALSE;
		}
		// Discard sub-objects of outer objects to ignore.
		if ( IsInIgnoreOuter(Object) )
		{
			return FALSE;
		}
		return TRUE;
	}

	/** @return		TRUE if the specified object is an 'IgnoreClasses' type. */
	FORCEINLINE UBOOL IsAnIgnoreClass(const UObject* Object) const
	{
		for ( INT ClassIndex = 0 ; ClassIndex < IgnoreClasses.Num() ; ++ClassIndex )
		{
			if ( Object->IsA(IgnoreClasses(ClassIndex)) )
			{
				return TRUE;
			}
		}
		return FALSE;
	}

	/** @return		TRUE if the specified object is not a subobject of one of the IngoreOuters. */
	FORCEINLINE UBOOL IsInIgnoreOuter(const UObject* Object) const
	{
		for ( INT OuterIndex = 0 ; OuterIndex < IgnoreOuters.Num() ; ++OuterIndex )
		{
			if ( Object->IsIn(IgnoreOuters(OuterIndex)) )
			{
				return TRUE;
			}
		}
		return FALSE;
	}

private:
	/** 
	 * UObject serialize operator implementation
	 *
	 * @param Object	reference to Object reference
	 * @return reference to instance of this class
	 */
	FArchive& operator<<( UObject*& Obj )
	{
		if ( Obj != NULL && Obj->HasAnyFlags(RF_TagExp) )
		{
			// Clear the search flag so we don't revisit objects
			Obj->ClearFlags(RF_TagExp);
			if ( Obj->IsA(UField::StaticClass()) )
			{
				// skip all of the other stuff because the serialization of UFields will quickly overflow
				// our stack given the number of temporary variables we create in the below code
				Obj->Serialize(*this);
			}
			else
			{
				// Only report this object reference if it supports thumbnail display
				// this eliminates all of the random objects like functions, properties, etc.
				const UBOOL bIsClassDefaultObject = Obj->HasAnyFlags(RF_ClassDefaultObject);
				const UBOOL bIsContent = GUnrealEd->GetThumbnailManager()->GetRenderingInfo(Obj) != NULL;
				const UBOOL bShouldReportAsset = !bIsClassDefaultObject && bIsContent;
				if (Obj->IsValid())
				{
					if ( bShouldReportAsset )
					{
						ObjectArray->AddItem( Obj );
					}
					// Check this object for any potential object references.
					Obj->Serialize(*this);
				}
			}
		}
		return *this;
	}

	/** Stored pointer to array of objects we add object references to */
	TArray<UObject*>*		ObjectArray;

	/** Only objects not within these objects will be considered.*/
	const TArray<UObject*>&	IgnoreOuters;

	/** Only objects not of these types will be considered.*/
	const TArray<UClass*>&	IgnoreClasses;
};

/** Target package and object name for moving an asset. */
class FMoveInfo
{
public:
	FString FullPackageName;
	FString NewObjName;

	void Set(const TCHAR* InFullPackageName, const TCHAR* InNewObjName)
	{
		FullPackageName = InFullPackageName;
		NewObjName = InNewObjName;
		check( IsValid() );
	}
	/** @return		TRUE once valid (non-empty) move info exists. */
	UBOOL IsValid() const
	{
		return ( FullPackageName.Len() > 0 && NewObjName.Len() > 0 );
	}
};

static void GetReferencedTopLevelObjects(UObject* InObject, TArray<UObject*>& OutTopLevelRefs)
{
	OutTopLevelRefs.Empty();

	// Set up a list of classes to ignore.
	TArray<UClass*> IgnoreClasses;
	IgnoreClasses.AddItem(ULevel::StaticClass());
	IgnoreClasses.AddItem(UWorld::StaticClass());
	IgnoreClasses.AddItem(UPhysicalMaterial::StaticClass());

	// Set up a list of packages to ignore.
	TArray<UObject*> IgnoreOuters;
	IgnoreOuters.AddItem(FindObject<UPackage>(NULL,TEXT("EditorMaterials"),TRUE));
	IgnoreOuters.AddItem(FindObject<UPackage>(NULL,TEXT("EditorMeshes"),TRUE));
	IgnoreOuters.AddItem(FindObject<UPackage>(NULL,TEXT("EditorResources"),TRUE));
	IgnoreOuters.AddItem(FindObject<UPackage>(NULL,TEXT("EngineMaterials"),TRUE));
	IgnoreOuters.AddItem(FindObject<UPackage>(NULL,TEXT("EngineFonts"),TRUE));
	IgnoreOuters.AddItem(FindObject<UPackage>(NULL,TEXT("EngineScenes"),TRUE));
	IgnoreOuters.AddItem(FindObject<UPackage>(NULL,TEXT("EngineResources"),TRUE));
	IgnoreOuters.AddItem(FindObject<UPackage>(NULL,TEXT("DefaultUISkin"),TRUE));

	IgnoreOuters.AddItem(UObject::GetTransientPackage());

	// Add script packages to the IgnoreOuters list...
	// Get combined list of all script package names, including editor- only ones.
	TArray<FString> AllScriptPackageNames;
	appGetEngineScriptPackageNames(AllScriptPackageNames, TRUE);
	appGetGameNativeScriptPackageNames(AllScriptPackageNames, TRUE);
	appGetGameScriptPackageNames(AllScriptPackageNames, TRUE);
	for (INT ScriptNameIndex = 0; ScriptNameIndex < AllScriptPackageNames.Num(); ScriptNameIndex++)
	{
		IgnoreOuters.AddItem(FindObject<UPackage>(NULL,*(AllScriptPackageNames(ScriptNameIndex)),TRUE));
	}

	TArray<UObject*> PendingObjects;
	PendingObjects.AddItem( InObject );
	while ( PendingObjects.Num() > 0 )
	{
		// Pop the pending object and mark it for duplication.
		UObject* CurObject = PendingObjects.Pop();
		OutTopLevelRefs.AddItem( CurObject );

		// Collect asset references.
		TArray<UObject*> NewRefs;
		FArchiveTopLevelReferenceCollector Ar( &NewRefs, IgnoreOuters, IgnoreClasses );
		CurObject->Serialize( Ar );

		// Now that the object has been serialized, add it to the list of IgnoreOuters for subsequent runs.
		IgnoreOuters.AddItem( CurObject );

		// Enqueue any referenced objects that haven't already been processed for top-level determination.
		for ( INT i = 0 ; i < NewRefs.Num() ; ++i )
		{
			UObject* NewRef = NewRefs(i);
			if ( !OutTopLevelRefs.ContainsItem(NewRef) )
			{
				PendingObjects.AddUniqueItem(NewRef);
			}
		}				
	}
}

void WxGenericBrowser::OnObjectDuplicateWithRefs(wxCommandEvent& In)
{
	// Build the list of objects that are selected
	TArray<UObject*> SelectedObjects;
	GetSelectedItems( LeftContainer->CurrentResourceType->SupportInfo, SelectedObjects );

	if ( SelectedObjects.Num() < 1 )
	{
		return;
	}

	// Present the user with a move dialog
	const FString DialogTitle( LocalizeUnrealEd("DuplicateWithReferences") );
	WxDlgMoveAssets MoveDialog;
	const UBOOL bEnableTreatNameAsSuffix = SelectedObjects.Num() > 1;
	MoveDialog.ConfigureNameField( bEnableTreatNameAsSuffix );

	FString PreviousPackage;
	FString PreviousGroup;
	FString PreviousName;
	UBOOL bPreviousNameAsSuffix = bEnableTreatNameAsSuffix;
	UBOOL bSawOKToAll = FALSE;
	UBOOL bSawSuccessfulDuplicate = FALSE;

	TArray<UPackage*> PackagesUserRefusedToFullyLoad;
	TArray<UPackage*> OutermostPackagesToSave;
	for( INT ObjectIndex = 0 ; ObjectIndex < SelectedObjects.Num() ; ++ObjectIndex )
	{
		UObject* Object = SelectedObjects(ObjectIndex);
		if( !Object )
		{
			continue;
		}

		// Initialize the move dialog with the previously entered pkg/grp, or the pkg/grp of the object to move.
		const FString DlgPackage = PreviousPackage.Len() ? PreviousPackage : Object->GetOutermost()->GetName();
		FString DlgGroup = PreviousGroup.Len() ? PreviousGroup : (Object->GetOuter()->GetOuter() ? Object->GetFullGroupName( 1 ) : TEXT(""));

		if( !bSawOKToAll )
		{
			MoveDialog.SetTitle( *FString::Printf(TEXT("%s: %s"),*DialogTitle,*Object->GetPathName()) );
			const int MoveDialogResult = MoveDialog.ShowModal( DlgPackage, DlgGroup, ( bPreviousNameAsSuffix && PreviousName.Len() ? PreviousName : Object->GetName() ), TRUE );
			// Abort if the user cancelled.
			if ( MoveDialogResult != wxID_OK && MoveDialogResult != ID_OK_ALL)
			{
				return;
			}
			// Don't show the dialog again if "Ok to All" was selected.
			if ( MoveDialogResult == ID_OK_ALL )
			{
				bSawOKToAll = TRUE;
			}
			// Store the entered package/group/name for later retrieval.
			PreviousPackage = MoveDialog.GetNewPackage();
			PreviousGroup = MoveDialog.GetNewGroup();
			bPreviousNameAsSuffix = MoveDialog.GetNewName( PreviousName );
		}
		const FScopedBusyCursor BusyCursor;

		// Make a list of objects to duplicate.
		TArray<UObject*> ObjectsToDuplicate;

		// Include references?
		if ( MoveDialog.GetIncludeRefs() )
		{
			GetReferencedTopLevelObjects( Object, ObjectsToDuplicate );
		}
		else
		{
			// Add just the object itself.
			ObjectsToDuplicate.AddItem( Object );
		}

		// Check validity of each reference dup name.
		FString ErrorMessage;
		FString Reason;
		UBOOL	bUserDeclinedToFullyLoadPackage = FALSE;

		TArray<FMoveInfo> MoveInfos;
		for (INT RefObjectIndex = 0 ; RefObjectIndex < ObjectsToDuplicate.Num() ; ++RefObjectIndex )
		{
			const UObject *RefObject = ObjectsToDuplicate(RefObjectIndex);
			FMoveInfo* MoveInfo = new(MoveInfos) FMoveInfo;

			// Determine the target package.
			FString ClassPackage;
			FString ClassGroup;
			MoveDialog.DetermineClassPackageAndGroup( RefObject, ClassPackage, ClassGroup );

			FString TgtPackage;
			FString TgtGroup;
			// If a class-specific package was specified, use the class-specific package/group combo.
			if ( ClassPackage.Len() )
			{
				TgtPackage = ClassPackage;
				TgtGroup = ClassGroup;
			}
			else
			{
				// No class-specific package was specified, so use the 'universal' destination package.
				TgtPackage = MoveDialog.GetNewPackage();
				TgtGroup = ClassGroup.Len() ? ClassGroup : MoveDialog.GetNewGroup();
			}

			// Make sure that a traget package exists.
			if ( !TgtPackage.Len() )
			{
				ErrorMessage += TEXT("Invalid package name supplied\n");
			}
			else
			{
				// Make a new object name by concatenating the source object name with the optional name suffix.
				FString DlgName;
				const UBOOL bTreatDlgNameAsSuffix = MoveDialog.GetNewName( DlgName );
				const FString ObjName = bTreatDlgNameAsSuffix ? FString::Printf(TEXT("%s%s"), *RefObject->GetName(), *DlgName) : DlgName;

				// Make a full path from the target package and group.
				const FString FullPackageName = TgtGroup.Len()
					? FString::Printf(TEXT("%s.%s"), *TgtPackage, *TgtGroup)
					: TgtPackage;

				// Make sure the packages being duplicated into are fully loaded.
				TArray<UPackage*> TopLevelPackages;
				UPackage* ExistingPackage = UObject::FindPackage(NULL, *FullPackageName);
				if( ExistingPackage )
				{
					TopLevelPackages.AddItem( ExistingPackage->GetOutermost() );
				}

				if( (ExistingPackage && PackagesUserRefusedToFullyLoad.ContainsItem(ExistingPackage)) ||
					!HandleFullyLoadingPackages( TopLevelPackages, TEXT("Duplicate") ) )
				{
					// HandleFullyLoadingPackages should never return FALSE for empty input.
					check( ExistingPackage );
					PackagesUserRefusedToFullyLoad.AddItem( ExistingPackage );
					bUserDeclinedToFullyLoadPackage = TRUE;
				}
				else if( !ObjName.Len() )
				{
					ErrorMessage += TEXT("Invalid object name\n");
				}
				else if(!FIsValidObjectName( *ObjName,Reason ) 
					||	!FIsValidGroupName(*TgtPackage,Reason)
					||	!FIsValidGroupName(*TgtGroup,Reason,TRUE) )
				{
					// Make sure the object name is valid.
					ErrorMessage += FString::Printf(TEXT("    %s to %s.%s: %s\n"), *RefObject->GetPathName(), *FullPackageName, *ObjName, *Reason );
				}
				else if ( ExistingPackage && !FIsUniqueObjectName(*ObjName, ExistingPackage, Reason) )
				{
					// Make sure the object name is not already in use.
					ErrorMessage += FString::Printf(TEXT("    %s to %s.%s: %s\n"), *RefObject->GetPathName(), *FullPackageName, *ObjName, *Reason );
				}
				else
				{
					// No errors!  Set asset move info.
					MoveInfo->Set( *FullPackageName, *ObjName );
				}
			}
		}

		// User declined to fully load the target package; no need to display message box.
		if( bUserDeclinedToFullyLoadPackage )
		{
			continue;
		}

		// If any errors are present, display them and abort this object.
		if( ErrorMessage.Len() > 0 )
		{
			appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("CannotDuplicateList"), *Object->GetName(), *ErrorMessage) );
			continue;
		}

		// Duplicate each reference (including the main object itself)
		// And create ReplacementMap for replacing references.
		TArray<UObject*> Duplicates;
		TMap<UObject*, UObject*> ReplacementMap;

		for ( INT RefObjectIndex = 0 ; RefObjectIndex < ObjectsToDuplicate.Num() ; ++RefObjectIndex )
		{
			UObject *RefObject = ObjectsToDuplicate(RefObjectIndex);
			const FMoveInfo& MoveInfo = MoveInfos(RefObjectIndex);
			check( MoveInfo.IsValid() );

			const FString& PkgName = MoveInfo.FullPackageName;
			const FString& ObjName = MoveInfo.NewObjName;

			// @hack: Make sure the Outers of parts of PhysicsAssets are correct before trying to duplicate it.
			UPhysicsAsset* PhysicsAsset = Cast<UPhysicsAsset>(RefObject);
			if( PhysicsAsset )
			{
				PhysicsAsset->FixOuters();
			}

			// Make sure the referenced object is deselected before duplicating it.
			GetGBSelection()->Deselect( RefObject );

			UObject* DupObject = GEditor->StaticDuplicateObject( RefObject, RefObject, UObject::CreatePackage(NULL,*PkgName), *ObjName );
			if( DupObject )
			{
				Duplicates.AddItem( DupObject );
				ReplacementMap.Set( RefObject, DupObject );
				DupObject->MarkPackageDirty();
				OutermostPackagesToSave.AddUniqueItem( DupObject->GetOutermost() );
				bSawSuccessfulDuplicate = TRUE;

				// if the source object is in the MyLevel package and it's being duplicated into a content package, we need
				// to mark it RF_Standalone so that it will be saved (UWorld::CleanupWorld() clears this flag for all objects
				// inside the package)
				if (!RefObject->HasAnyFlags(RF_Standalone)
				&&	RefObject->GetOutermost()->ContainsMap()
				&&	!DupObject->GetOutermost()->ContainsMap() )
				{
					DupObject->SetFlags(RF_Standalone);
				}
			}
		}

		// Replace all references
		for (INT RefObjectIndex = 0 ; RefObjectIndex < ObjectsToDuplicate.Num() ; ++RefObjectIndex )
		{
			UObject* RefObject = ObjectsToDuplicate(RefObjectIndex);
			UObject* Duplicate = Duplicates(RefObjectIndex);

			FArchiveReplaceObjectRef<UObject> ReplaceAr( Duplicate, ReplacementMap, FALSE, TRUE, TRUE );
		}
	}

	// Update the browser if something was actually moved.
	if ( bSawSuccessfulDuplicate )
	{
		if ( MoveDialog.GetCheckoutPackages() )
		{
			CheckOutTopLevelPackages( OutermostPackagesToSave );
		}
		if ( MoveDialog.GetSavePackages() )
		{
			SavePackages( OutermostPackagesToSave, TRUE );
		}

		GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
		RightContainer->Update();
	}
}

void WxGenericBrowser::OnObjectShowReferencedObjs(wxCommandEvent& In)
{
	UObject* Object = GetGBSelection()->GetTop<UObject>();
	if( Object )
	{
		GetGBSelection()->Deselect( Object );

		// Find references.
		TArray<UObject*> ReferencedObjects;
		{
			const FScopedBusyCursor BusyCursor;
			TArray<UClass*> IgnoreClasses;
			TArray<UObject*> IgnorePackages;

			// Assemble an ignore list.
			IgnoreClasses.AddItem( ULevel::StaticClass() );
			IgnoreClasses.AddItem( UWorld::StaticClass() );
			IgnoreClasses.AddItem( UPhysicalMaterial::StaticClass() );
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EditorMaterials"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EditorMeshes"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EditorResources"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineMaterials"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineFonts"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineScenes"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineResources"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("DefaultUISkin"),TRUE));

			WxReferencedAssetsBrowser::BuildAssetList( Object, IgnoreClasses, IgnorePackages, ReferencedObjects );
		}

		if ( ReferencedObjects.Num() )
		{
			FString OutString( FString::Printf( TEXT("\nObjects referenced by %s:\r\n"), *Object->GetFullName() ) );
			for ( INT ObjectIndex = 0 ; ObjectIndex < ReferencedObjects.Num() ; ++ObjectIndex )
			{
				const UObject *ReferencedObject = ReferencedObjects( ObjectIndex );
				 // Don't list an object as referring to itself.
				if ( ReferencedObject != Object )
				{
					OutString += FString::Printf( TEXT("\t%s:\r\n"), *ReferencedObject->GetFullName() );
				}
			}

			appMsgf( AMT_OK, TEXT("%s"), *OutString );
			warnf( TEXT("%s"), *OutString );
		}
		else
		{
			appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("ObjectNotReferenced"), *Object->GetName()) );
		}

		GetGBSelection()->Select( Object );
	}
}

void WxGenericBrowser::RenameObjectsWithRefs( TArray<UObject*>& Objects, UBOOL bLocPackages )
{
	// Present the user with a rename dialog for each asset.
	const FString DialogTitle( LocalizeUnrealEd( "MoveWithReferences" ) );
	WxDlgMoveAssets MoveDialog;
	const UBOOL bEnableTreatNameAsSuffix = FALSE;
	MoveDialog.ConfigureNameField( bEnableTreatNameAsSuffix );

	FString ErrorMessage;
	FString Reason;
	FString PreviousPackage;
	FString PreviousGroup;
	FString PreviousName;
	UBOOL bPreviousNameAsSuffix = bEnableTreatNameAsSuffix;
	UBOOL bSawOKToAll = FALSE;
	UBOOL bSawSuccessfulRename = FALSE;

	TArray<UPackage*> PackagesUserRefusedToFullyLoad;
	TArray<UPackage*> OutermostPackagesToSave;
	for( INT Index = 0; Index < Objects.Num(); Index++ )
	{
		UObject* Object = Objects( Index );
		if( !Object )
		{
			continue;
		}

		FString DlgPackage = Object->GetOutermost()->GetName();
		if( !bLocPackages && PreviousPackage.Len() )
		{
			// Initialize the move dialog with the previously entered pkg/grp
			DlgPackage = PreviousPackage;
		}

		FString DlgGroup = PreviousGroup.Len() ? PreviousGroup : ( Object->GetOuter()->GetOuter() ? Object->GetFullGroupName( 1 ) : TEXT( "" ) );
		if( !bSawOKToAll )
		{
			MoveDialog.SetTitle( *FString::Printf( TEXT( "%s: %s" ), *DialogTitle, *Object->GetPathName() ) );
			const int MoveDialogResult = MoveDialog.ShowModal( DlgPackage, DlgGroup, ( bPreviousNameAsSuffix && PreviousName.Len() ? PreviousName : Object->GetName() ), !bLocPackages );
			// Abort if the user cancelled.
			if( MoveDialogResult != wxID_OK && MoveDialogResult != ID_OK_ALL )
			{
				return;
			}
			// Don't show the dialog again if "Ok to All" was selected.
			if( MoveDialogResult == ID_OK_ALL )
			{
				bSawOKToAll = TRUE;
			}

			// Store the entered package/group/name for later retrieval.
			PreviousPackage = MoveDialog.GetNewPackage();
			PreviousGroup = MoveDialog.GetNewGroup();
			bPreviousNameAsSuffix = MoveDialog.GetNewName( PreviousName );

			bSawOKToAll |= bLocPackages;
		}
		const FScopedBusyCursor BusyCursor;

		// Find references.
		TArray<UObject*> ReferencedTopLevelObjects;
		const UBOOL bIncludeRefs = MoveDialog.GetIncludeRefs();
		if ( bIncludeRefs )
		{
			GetReferencedTopLevelObjects( Object, ReferencedTopLevelObjects );
		}
		else
		{
			// Add just the object itself.
			ReferencedTopLevelObjects.AddItem( Object );
		}

		UBOOL bMoveFailed = FALSE;
		TArray<FMoveInfo> MoveInfos;
		for ( INT ObjectIndex = 0 ; ObjectIndex < ReferencedTopLevelObjects.Num() && !bMoveFailed ; ++ObjectIndex )
		{
			UObject *RefObject = ReferencedTopLevelObjects(ObjectIndex);
			FMoveInfo* MoveInfo = new(MoveInfos) FMoveInfo;

			// Determine the target package.
			FString ClassPackage;
			FString ClassGroup;
			MoveDialog.DetermineClassPackageAndGroup( RefObject, ClassPackage, ClassGroup );

			FString TgtPackage;
			FString TgtGroup;
			// If a class-specific package was specified, use the class-specific package/group combo.
			if ( ClassPackage.Len() )
			{
				TgtPackage = ClassPackage;
				TgtGroup = ClassGroup;
			}
			else
			{
				// No class-specific package was specified, so use the 'universal' destination package.
				if( bLocPackages )
				{
					// Can't rename to a different package when doing loc dependencies
					TgtPackage = DlgPackage;
				}
				else
				{
					TgtPackage = MoveDialog.GetNewPackage();
				}

				TgtGroup = ClassGroup.Len() ? ClassGroup : MoveDialog.GetNewGroup();
			}

			// Make sure that a traget package exists.
			if ( !TgtPackage.Len() )
			{
				ErrorMessage += TEXT("Invalid package name supplied\n");
				bMoveFailed = TRUE;
			}
			else
			{
				// Make a new object name by concatenating the source object name with the optional name suffix.
				FString DlgName;
				const UBOOL bTreatDlgNameAsSuffix = MoveDialog.GetNewName( DlgName );
				const FString ObjName = bTreatDlgNameAsSuffix ? FString::Printf(TEXT("%s%s"), *RefObject->GetName(), *DlgName) : DlgName;

				// Make a full path from the target package and group.
				const FString FullPackageName = TgtGroup.Len()
					? FString::Printf(TEXT("%s.%s"), *TgtPackage, *TgtGroup)
					: TgtPackage;

				// Make sure the target package is fully loaded.
				TArray<UPackage*> TopLevelPackages;
				UPackage* ExistingPackage = UObject::FindPackage(NULL, *FullPackageName);
				if( ExistingPackage )
				{
					TopLevelPackages.AddItem( ExistingPackage->GetOutermost() );
				}
				if( (ExistingPackage && PackagesUserRefusedToFullyLoad.ContainsItem(ExistingPackage)) ||
					!HandleFullyLoadingPackages( TopLevelPackages, TEXT("Rename") ) )
				{
					// HandleFullyLoadingPackages should never return FALSE for empty input.
					check( ExistingPackage );
					PackagesUserRefusedToFullyLoad.AddItem( ExistingPackage );
					bMoveFailed = TRUE;
				}
				else if( !ObjName.Len() )
				{
					ErrorMessage += TEXT("Invalid object name\n");
					bMoveFailed = TRUE;
				}
				else if(!FIsValidObjectName( *ObjName,Reason ) 
					||	!FIsValidGroupName(*TgtPackage,Reason)
					||	!FIsValidGroupName(*TgtGroup,Reason,TRUE) )
				{
					// Make sure the object name is valid.
					ErrorMessage += FString::Printf(TEXT("    %s to %s.%s: %s\n"), *RefObject->GetPathName(), *FullPackageName, *ObjName, *Reason );
					bMoveFailed = TRUE;
				}
				else
				{
					// We can rename on top of an object redirection (basically destroy the redirection and put us in its place).
					const FString FullObjectPath = FString::Printf(TEXT("%s.%s"), *FullPackageName, *ObjName);
					UObjectRedirector* Redirector = Cast<UObjectRedirector>( UObject::StaticFindObject(UObjectRedirector::StaticClass(), NULL, *FullObjectPath) );
					// If we found a redirector, check that the object it points to is of the same class.
					if ( Redirector
						&& Redirector->DestinationObject
						&& Redirector->DestinationObject->GetClass() == RefObject->GetClass() )
					{
						// Test renaming the redirector into a dummy package.
						if ( !Redirector->Rename(*Redirector->GetName(), UObject::CreatePackage(NULL, TEXT("DeletedRedirectors")), REN_Test) )
						{
							bMoveFailed = TRUE;
						}
					}

					if ( !bMoveFailed )
					{
						UPackage* NewPackage = UObject::CreatePackage( NULL, *FullPackageName );
						// Test to see if the rename will succeed.
						if ( RefObject->Rename(*ObjName, NewPackage, REN_Test) )
						{
							// No errors!  Set asset move info.
							MoveInfo->Set( *FullPackageName, *ObjName );
						}
						else
						{
							ErrorMessage += FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_ObjectNameAlreadyExists"), *Objects(Index)->GetFullName()) );
							bMoveFailed = TRUE;
						}
					}
				}
			} // Tgt package valid?
		} // Referenced top-level objects

		if ( !bMoveFailed )
		{
			// Actually perform the move!
			for ( INT RefObjectIndex = 0 ; RefObjectIndex < ReferencedTopLevelObjects.Num() ; ++RefObjectIndex )
			{
				UObject *RefObject = ReferencedTopLevelObjects(RefObjectIndex);
				const FMoveInfo& MoveInfo = MoveInfos(RefObjectIndex);
				check( MoveInfo.IsValid() );

				const FString& PkgName = MoveInfo.FullPackageName;
				const FString& ObjName = MoveInfo.NewObjName;
				const FString FullObjectPath = FString::Printf(TEXT("%s.%s"), *PkgName, *ObjName);

				// We can rename on top of an object redirection (basically destroy the redirection and put us in its place).
				UObjectRedirector* Redirector = Cast<UObjectRedirector>( UObject::StaticFindObject(UObjectRedirector::StaticClass(), NULL, *FullObjectPath) );
				// If we found a redirector, check that the object it points to is of the same class.
				if ( Redirector
					&& Redirector->DestinationObject
					&& Redirector->DestinationObject->GetClass() == RefObject->GetClass() )
				{
					// Remove public flag if set to ensure below rename doesn't create a redirect.
					Redirector->ClearFlags( RF_Public );
					// Instead of deleting we rename the redirector into a dummy package.
					Redirector->Rename(*Redirector->GetName(), UObject::CreatePackage( NULL, TEXT("DeletedRedirectors")) );
				}
				UPackage* NewPackage = UObject::CreatePackage( NULL, *PkgName );
				OutermostPackagesToSave.AddUniqueItem( RefObject->GetOutermost() );

				// if this object is being renamed out of the MyLevel package into a content package, we need to mark it RF_Standalone
				// so that it will be saved (UWorld::CleanupWorld() clears this flag for all objects inside the package)
				if (!RefObject->HasAnyFlags(RF_Standalone)
					&&	RefObject->GetOutermost()->ContainsMap()
					&&	!NewPackage->GetOutermost()->ContainsMap() )
				{
					RefObject->SetFlags(RF_Standalone);
				}
				GEditor->RenameObject( RefObject, NewPackage, *ObjName, REN_None );
				OutermostPackagesToSave.AddUniqueItem( NewPackage->GetOutermost() );
				bSawSuccessfulRename = TRUE;
			} // Referenced top-level objects
		}
		else
		{
			ErrorMessage += FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_CouldntRenameObjectF"), *Object->GetFullName()) );
		}
	} // Selected objects.

	// Display any error messages that accumulated.
	if ( ErrorMessage.Len() > 0 )
	{
		appMsgf( AMT_OK, *ErrorMessage );
	}

	// Update the browser if something was actually renamed.
	if ( bSawSuccessfulRename )
	{
		if ( MoveDialog.GetCheckoutPackages() )
		{
			CheckOutTopLevelPackages( OutermostPackagesToSave );
		}
		if ( MoveDialog.GetSavePackages() )
		{
			SavePackages( OutermostPackagesToSave, TRUE );
		}

		GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
		RightContainer->Update();
	}
}

/**
 * Renames an object and leaves redirectors so other content that references it does not break
 */
void WxGenericBrowser::OnObjectRenameWithRefs( wxCommandEvent& In ) 
{
	// Build the list of objects that are selected
	TArray<UObject*> SelectedObjects;
	GetSelectedItems( LeftContainer->CurrentResourceType->SupportInfo, SelectedObjects );

	RenameObjectsWithRefs( SelectedObjects, FALSE );
}

void WxGenericBrowser::AddLanguageVariants( TArray<UObject*>& Objects, USoundNodeWave* Wave )
{
	if( Wave->GetClass()->HasAnyClassFlags( CLASS_Localized | CLASS_PerObjectLocalized ) || Wave->HasAnyFlags( RF_PerObjectLocalized ) )
	{
		// Add in the American root
		Objects.AddUniqueItem( Wave );

		// Add all the localised variants to the list of objects to rename
		for( INT LangIndex = 1; GKnownLanguageExtensions[LangIndex]; LangIndex++ )
		{
			FString PackageName = Wave->GetOutermost()->GetName().ToLower();
			FString LocPackageName = FString::Printf( TEXT( "%s_%s" ), *PackageName, GKnownLanguageExtensions[LangIndex] );
			FString LocFullName = Wave->GetPathName().Replace( *PackageName, *LocPackageName, TRUE );

			USoundNodeWave* LocObject = LoadObject<USoundNodeWave>( NULL, *LocFullName, NULL, LOAD_None, NULL );
			if( LocObject )
			{
				Objects.AddUniqueItem( LocObject );
			}
		}
	}
}

/**
 * Renames an object and leaves redirectors so other content that references it does not break
 * Also renames all loc instances of the same asset
 */
void WxGenericBrowser::OnObjectRenameLocWithRefs( wxCommandEvent& In ) 
{
	TArray<UObject*> SelectedObjects;
	TArray<UObject*> LocObjects;

	GetSelectedItems( LeftContainer->CurrentResourceType->SupportInfo, SelectedObjects );
	
	for( INT Index = 0; Index < SelectedObjects.Num(); Index++ )
	{
		LocObjects.Empty();

		UObject* Object = SelectedObjects( Index );
		if( Object )
		{
			USoundNodeWave* Wave = ExactCast<USoundNodeWave>( Object );
			if( Wave )
			{
				// Find if this is localised and add in the other languages
				AddLanguageVariants( LocObjects, Wave );

				RenameObjectsWithRefs( LocObjects, TRUE );
			}
		}
	}
}

/**
 * Deletes the list of objects
 *
 * @param ObjectsToDelete The list of objects to delete
 *
 * @return The number of objects successfully deleted
 */
INT WxGenericBrowser::DeleteObjects( const TArray< UObject* >& ObjectsToDelete )
{
	const FScopedBusyCursor BusyCursor;

	// Make sure packages being saved are fully loaded.
	if( !HandleFullyLoadingPackages( ObjectsToDelete, TEXT("Delete") ) )
	{
		return 0;
	}

	RightContainer->DisableUpdate();
	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("Deleting")), TRUE );

	INT NumObjectsDeleted = 0;

	UBOOL bSawSuccessfulDelete = FALSE;

	for (INT Index = 0; Index < ObjectsToDelete.Num(); Index++)
	{
		GWarn->StatusUpdatef( Index, ObjectsToDelete.Num(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Deletingf"), Index, ObjectsToDelete.Num()) ) );

		UObject* Object = ObjectsToDelete(Index);

		// Give the generic browser type and opportunity to abort the delete.
		UBOOL bDeleteAborted = FALSE;
		UBOOL bDeleteSuccessful = FALSE;
		for ( INT ResourceIndex = 0; ResourceIndex < LeftContainer->ResourceTypes.Num(); ResourceIndex++ )
		{
			UGenericBrowserType* gbt = LeftContainer->ResourceTypes(ResourceIndex);
			if ( gbt->Supports(Object) && !gbt->NotifyPreDeleteObject(Object) )
			{
				bDeleteAborted = TRUE;
				break;
			}
		}

		if ( !bDeleteAborted )
		{
			GetGBSelection()->Deselect( Object );

			// Remove potential references to to-be deleted objects from thumbnail manager preview components.
			GUnrealEd->ThumbnailManager->ClearComponents();

			// Check and see whether we are referenced by any objects that won't be garbage collected. 
			if( UObject::IsReferenced( Object, GARBAGE_COLLECTION_KEEPFLAGS ) )
			{
				// We cannot safely delete this object. Print out a list of objects referencing this one
				// that prevent us from being able to delete it.
				FStringOutputDevice Ar;
				Object->OutputReferencers(Ar,FALSE);
				appMsgf(AMT_OK,LocalizeSecure(LocalizeUnrealEd("Error_InUse"), *Object->GetFullName(), *Ar));

				// Reselect the object as it failed to be deleted
				GetGBSelection()->Select( Object );
			}
			else
			{
				bDeleteSuccessful = bSawSuccessfulDelete = TRUE;

				++NumObjectsDeleted;

				// Mark its package as dirty as we're going to delete it.
				Object->MarkPackageDirty();

				// Remove standalone flag so garbage collection can delete the object.
				Object->ClearFlags( RF_Standalone );
			}
		}

		for ( INT ResourceIndex = 0; ResourceIndex < LeftContainer->ResourceTypes.Num(); ResourceIndex++ )
		{
			UGenericBrowserType* gbt = LeftContainer->ResourceTypes(ResourceIndex);
			if ( gbt->Supports(Object) )
			{
				gbt->NotifyPostDeleteObject(Object, bDeleteSuccessful);
			}
		}
	}

	GWarn->EndSlowTask();
	RightContainer->EnableUpdate();

	// Update the browser if something was actually deleted.
	if ( bSawSuccessfulDelete )
	{
		// Collect garbage.
		UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

		// Now is a good time to sychronize source control state.
		LeftContainer->RequestSCCUpdate();

		// Refresh.
		Update();
	}

	return NumObjectsDeleted;
}


void WxGenericBrowser::OnObjectDelete(wxCommandEvent& In)
{
	// Allows deleting of sounds after they have been previewed
	GEditor->ClearPreviewAudioComponents();

	// Build the list of objects that are selected.
	TArray<UObject*> SelectedObjects;
	GetSelectedItems(LeftContainer->CurrentResourceType->SupportInfo,SelectedObjects);

	// Delete the objects!
	INT NumObjectsDeleted = DeleteObjects( SelectedObjects );
}


void WxGenericBrowser::OnObjectDeleteWithRefs(wxCommandEvent& In)
{
	const FScopedBusyCursor BusyCursor;

	// Build the list of objects that are selected
	TArray<UObject*> SelectedObjects;
	GetSelectedItems(LeftContainer->CurrentResourceType->SupportInfo,SelectedObjects);

	// Allows deleting of sounds after they have been previewed
	GEditor->ClearPreviewAudioComponents();

	UBOOL bSawSuccessfulDelete = FALSE;
	FString ErrorMessage;

	for (INT Index = 0; Index < SelectedObjects.Num(); Index++)
	{
		// Find references
		TArray<UObject*> ReferencedObjects;
		{
			TArray<UClass*> IgnoreClasses;
			TArray<UObject*> IgnorePackages;

			// Set up our ignore lists
			IgnoreClasses.AddItem(ULevel::StaticClass());
			IgnoreClasses.AddItem(UWorld::StaticClass());
			IgnoreClasses.AddItem(UPhysicalMaterial::StaticClass());
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EditorMaterials"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EditorMeshes"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EditorResources"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineMaterials"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineFonts"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineScenes"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineResources"),TRUE));
			IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("DefaultUISkin"),TRUE));

			WxReferencedAssetsBrowser::BuildAssetList( SelectedObjects(Index), IgnoreClasses, IgnorePackages, ReferencedObjects );
		}

		for ( INT ObjectIndex = 0 ; ObjectIndex < ReferencedObjects.Num() ; ++ObjectIndex )
		{
			UObject* Object = ReferencedObjects( ObjectIndex );

			GetGBSelection()->Deselect( Object );

			// Remove potential references to to-be deleted objects from thumbnail manager preview components.
			GUnrealEd->ThumbnailManager->ClearComponents();

			// Delete redirects to the object.
			for( TObjectIterator<UObjectRedirector> It ; It ; ++It )
			{
				UObjectRedirector* Redirector = *It;
				UObject* RedirObj = static_cast<UObject*>( Redirector );
				if( Redirector->DestinationObject == Object && !UObject::IsReferenced( RedirObj, GARBAGE_COLLECTION_KEEPFLAGS ) )
				{
					Redirector->DestinationObject = NULL;
					Redirector->MarkPackageDirty();
					Redirector->ClearFlags( RF_Standalone );
				}
			}

			// Check and see whether we are referenced by any objects that won't be garbage collected. 
			if( UObject::IsReferenced( Object, GARBAGE_COLLECTION_KEEPFLAGS ) )
			{
				// We cannot safely delete this object. Print out a list of objects referencing this one
				// that prevent us from being able to delete it.
				ErrorMessage += FString::Printf( TEXT("%s\n"), *Object->GetFullName() );

				// Reselect the object as it failed to be deleted
				GetGBSelection()->Select( Object );
			}
			else
			{
				bSawSuccessfulDelete = TRUE;

				// Mark its package as dirty as we're going to delete it.
				Object->MarkPackageDirty();

				// Remove standalone flag so garbage collection can delete the object.
				Object->ClearFlags( RF_Standalone );
			}
		}
	}

	// Update the browser if something was actually deleted.
	if ( bSawSuccessfulDelete )
	{
		// Collect garbage.
		UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

		// Now is a good time to sychronize source control state.
		LeftContainer->RequestSCCUpdate();

		// Refresh.
		Update();
	}

	// Display any error messages that were accumulated.
	if ( ErrorMessage.Len() > 0 )
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("CannotDeleteList"), *ErrorMessage) );
	}
}

void WxGenericBrowser::OnObjectShowReferencers(wxCommandEvent& In)
{
	UObject* Object = GetGBSelection()->GetTop<UObject>();
	if(Object)
	{
		GetGBSelection()->Deselect( Object );

		if(UObject::IsReferenced(Object,RF_Native | RF_Public))
		{
			FStringOutputDevice Ar;
			Object->OutputReferencers(Ar,FALSE);
			warnf( TEXT("%s"), *Ar );  // also print the objects to the log so you can actually utilize the data
			appMsgf(AMT_OK, TEXT("%s"), *Ar);
		}
		else
		{
			appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("ObjectNotReferenced"), *Object->GetName()));
		}

		GetGBSelection()->Select( Object );
	}
}

namespace {
/**
 * Iterates over all classes and assembles a list of non-abstract UExport-derived type instances.
 */
static void AssembleListOfExporters(TArray<UExporter*>& OutExporters)
{
	// @todo DB: Assemble this set once.
	OutExporters.Empty();
	for( TObjectIterator<UClass> It ; It ; ++It )
	{
		if( It->IsChildOf(UExporter::StaticClass()) && !(It->ClassFlags & CLASS_Abstract) )
		{
			UExporter* Exporter = ConstructObject<UExporter>( *It );
			OutExporters.AddItem( Exporter );		
		}
	}
}

/**
 * Assembles a path from the outer chain of the specified object.
 */
static void GetDirectoryFromObjectPath(const UObject* Obj, FString& OutResult)
{
	if( Obj )
	{
		GetDirectoryFromObjectPath( Obj->GetOuter(), OutResult );
		OutResult *= Obj->GetName();
	}
}

} // namespace

/**
 * Exports the specified objects to file.
 *
 * @param	ObjectsToExport					The set of objects to export.
 * @param	bPromptIndividualFilenames		If TRUE, prompt individually for filenames.  If FALSE, bulk export to a single directory.
 */
void WxGenericBrowser::ExportObjects(const TArray<UObject*>& ObjectsToExport, UBOOL bPromptIndividualFilenames)
{
	if ( ObjectsToExport.Num() == 0 )
	{
		return;
	}

	FFilename SelectedExportPath;
	UBOOL bExportGroupsAsSubDirs = FALSE;

	if ( !bPromptIndividualFilenames )
	{
		// If not prompting individual files, prompt the user to select a target directory.
		wxDirDialog ChooseDirDialog(
			this,
			*LocalizeUnrealEd("ChooseADirectory"),
			*LastExportPath
			);

		if ( ChooseDirDialog.ShowModal() != wxID_OK )
		{
			return;
		}
		SelectedExportPath = FFilename( ChooseDirDialog.GetPath() );

		// Copy off the selected path for future export operations.
		LastExportPath = SelectedExportPath;

		// Prompt the user if group contents should be exported to their own subdirectories.
		bExportGroupsAsSubDirs = appMsgf( AMT_YesNo, *LocalizeUnrealEd(TEXT("Prompt_ExportGroupsAsSubDirs")) );
	}

	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("Exporting")), TRUE );

	// Create an array of all available exporters.
	TArray<UExporter*> Exporters;
	AssembleListOfExporters( Exporters );

	// Export the objects.
	for (INT Index = 0; Index < ObjectsToExport.Num(); Index++)
	{
		GWarn->StatusUpdatef( Index, ObjectsToExport.Num(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Exportingf"), Index, ObjectsToExport.Num()) ) );

		UObject* ObjectToExport = ObjectsToExport(Index);
		if ( !ObjectToExport )
		{
			continue;
		}

		// Can't export cooked content.
		const UPackage* ObjectPackage = ObjectToExport->GetOutermost();
		if( ObjectPackage->PackageFlags & PKG_Cooked )
		{
			return;
		}

		// Find all the exporters that can export this type of object and construct an export file dialog.
		FString FileTypes;
		FString AllExtensions;
		FString FirstExtension;

		// Iterate in reverse so the most relevant file formats are considered first.
		for( INT ExporterIndex = Exporters.Num()-1 ; ExporterIndex >=0 ; --ExporterIndex )
		{
			UExporter* Exporter = Exporters(ExporterIndex);
			if( Exporter->SupportedClass )
			{
				const UBOOL bObjectIsSupported = ObjectToExport->IsA( Exporter->SupportedClass );
				if ( bObjectIsSupported )
				{
					// Seed the default extension with the exporter's preferred format.
					if ( FirstExtension.Len() == 0 && Exporter->PreferredFormatIndex < Exporter->FormatExtension.Num() )
					{
						FirstExtension = Exporter->FormatExtension(Exporter->PreferredFormatIndex);
					}

					// Get a string representing of the exportable types.
					check( Exporter->FormatExtension.Num() == Exporter->FormatDescription.Num() );
					for( INT FormatIndex = Exporter->FormatExtension.Num()-1 ; FormatIndex >= 0 ; --FormatIndex )
					{
						const FString& FormatExtension = Exporter->FormatExtension(FormatIndex);
						const FString& FormatDescription = Exporter->FormatDescription(FormatIndex);

						if ( FirstExtension.Len() == 0 )
						{
							FirstExtension = FormatExtension;
						}
						if( FileTypes.Len() )
						{
							FileTypes += TEXT("|");
						}
						FileTypes += FString::Printf( TEXT("%s (*.%s)|*.%s"), *FormatDescription, *FormatExtension, *FormatExtension );

						if( AllExtensions.Len() )
						{
							AllExtensions += TEXT(";");
						}
						AllExtensions += FString::Printf( TEXT("*.%s"), *FormatExtension );
					}
				}
			}
		}

		// Skip this object if no exporter found for this resource type.
		if ( FirstExtension.Len() == 0 )
		{
			continue;
		}

		FileTypes = FString::Printf(TEXT("%s|All Files (%s)|%s"),*FileTypes, *AllExtensions, *AllExtensions);

		FFilename SaveFileName;
		if ( bPromptIndividualFilenames )
		{
			// Open dialog so user can chose save filename.
			WxFileDialog SaveFileDialog( this, 
				*FString::Printf( LocalizeSecure(LocalizeUnrealEd("Save_F"), *ObjectToExport->GetName()) ),
				*LastExportPath,
				*ObjectToExport->GetName(),
				*FileTypes,
				wxSAVE,
				wxDefaultPosition);

			if( SaveFileDialog.ShowModal() != wxID_OK )
			{
				continue;
			}
			SaveFileName = FFilename( SaveFileDialog.GetPath() );

			// Copy off the selected path for future export operations.
			LastExportPath = SaveFileName;
		}
		else
		{
			// Assemble a filename from the export directory and the object path.
			SaveFileName = SelectedExportPath;
			if ( bExportGroupsAsSubDirs )
			{
				// Assemble a path from the complete outer chain.
				GetDirectoryFromObjectPath( ObjectToExport, SaveFileName );
			}
			else
			{
				// Assemble a path only from the package name.
				SaveFileName *= ObjectToExport->GetOutermost()->GetName();
				SaveFileName *= ObjectToExport->GetName();
			}
			SaveFileName += FString::Printf( TEXT(".%s"), *FirstExtension );
			debugf(TEXT("Exporting \"%s\" to \"%s\""), *ObjectToExport->GetPathName(), *SaveFileName );
		}

		// Create the path, then make sure the target file is not read-only.
		const FString ObjectExportPath( SaveFileName.GetPath() );
		if ( !GFileManager->MakeDirectory( *ObjectExportPath, TRUE ) )
		{
			appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_FailedToMakeDirectory"), *ObjectExportPath)) );
		}
		else if( GFileManager->IsReadOnly( *SaveFileName ) )
		{
			appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_CouldntWriteToFile_F"), *SaveFileName)) );
		}
		else
		{
			// We have a writeable file.  Now go through that list of exporters again and find the right exporter and use it.
			TArray<UExporter*>	ValidExporters;

			for( INT ExporterIndex = 0 ; ExporterIndex < Exporters.Num(); ++ExporterIndex )
			{
				UExporter* Exporter = Exporters(ExporterIndex);
				if( Exporter->SupportedClass && ObjectToExport->IsA(Exporter->SupportedClass) )
				{
					check( Exporter->FormatExtension.Num() == Exporter->FormatDescription.Num() );
					for( INT FormatIndex = 0 ; FormatIndex < Exporter->FormatExtension.Num() ; ++FormatIndex )
					{
						const FString& FormatExtension = Exporter->FormatExtension(FormatIndex);
						if(	appStricmp( *FormatExtension, *SaveFileName.GetExtension() ) == 0 ||
							appStricmp( *FormatExtension, TEXT("*") ) == 0 )
						{
							ValidExporters.AddItem( Exporter );
							break;
						}
					}
				}
			}

			// Handle the potential of multiple exporters being found
			UExporter* ExporterToUse = NULL;
			if( ValidExporters.Num() == 1 )
			{
				ExporterToUse = ValidExporters( 0 );
			}
			else if( ValidExporters.Num() > 1 )
			{
				// Set up the first one as default
				ExporterToUse = ValidExporters( 0 );

				// ...but search for a better match if available
				for( INT i = 0; i < ValidExporters.Num(); i++ )
				{
					if( ValidExporters( i )->GetClass()->GetFName() == ObjectToExport->GetExporterName() )
					{
						ExporterToUse = ValidExporters( i );
						break;
					}
				}
			}

			// If an exporter was found, use it.
			if( ExporterToUse )
			{
				WxDlgExportGeneric dlg;
				dlg.ShowModal( SaveFileName, ObjectToExport, ExporterToUse, FALSE );
			}
		}
	}

	GWarn->EndSlowTask();
}

/**
 *	Exports the given packages to files.
 *
 *	@param	PackagesToExport	The set of packages to export.
 */
void WxGenericBrowser::ExportPackages(const TArray<UPackage*>& PackagesToExport)
{
	if ( PackagesToExport.Num() == 0 )
	{
		return;
	}

	FFilename SelectedExportPath;

	// Get the directory to export to...
	wxDirDialog ChooseDirDialog(
		this,
		*LocalizeUnrealEd("ChooseADirectory"),
		*LastExportPath
		);

	if ( ChooseDirDialog.ShowModal() != wxID_OK )
	{
		return;
	}
	SelectedExportPath = FFilename( ChooseDirDialog.GetPath() );

	// Copy off the selected path for future export operations.
	LastExportPath = SelectedExportPath;

	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("Exporting")), TRUE );

	// Create an array of all available exporters.
	TArray<UExporter*> Exporters;
	AssembleListOfExporters( Exporters );

	// Export the objects.
	for (INT Index = 0; Index < PackagesToExport.Num(); Index++)
	{
		GWarn->StatusUpdatef( Index, PackagesToExport.Num(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Exportingf"), Index, PackagesToExport.Num()) ) );

		UPackage* PackageToExport = PackagesToExport(Index);
		if (!PackageToExport)
		{
			continue;
		}

		// Can't export cooked content.
		if (PackageToExport->PackageFlags & PKG_Cooked)
		{
			continue;
		}

		UPackageExporterT3D* PackageExporter = new UPackageExporterT3D();
		check(PackageExporter);
		FFilename SaveFileName;

		// Assemble a filename from the export directory and the object path.
		SaveFileName = SelectedExportPath;
		// Assemble a path only from the package name.
		SaveFileName *= PackageToExport->GetName();
		SaveFileName += TEXT(".T3DPKG");
		debugf(TEXT("Exporting \"%s\" to \"%s\""), *PackageToExport->GetPathName(), *SaveFileName );

		// Create the path, then make sure the target file is not read-only.
		const FString ObjectExportPath( SaveFileName.GetPath() );
		if ( !GFileManager->MakeDirectory( *ObjectExportPath, TRUE ) )
		{
			appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_FailedToMakeDirectory"), *ObjectExportPath)) );
		}
		else if( GFileManager->IsReadOnly( *SaveFileName ) )
		{
			appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_CouldntWriteToFile_F"), *SaveFileName)) );
		}
		else
		{
			// We have a writeable file.  Now go through that list of exporters again and find the right exporter and use it.
			// If an exporter was found, use it.
			if( PackageExporter )
			{
				WxDlgExportGeneric dlg;
				dlg.ShowModal( SaveFileName, PackageToExport, PackageExporter, FALSE );
			}
		}
	}

	GWarn->EndSlowTask();
}

void WxGenericBrowser::OnObjectExport(wxCommandEvent& In)
{
	// Build the list of objects that are selected.
	TArray<UObject*> SelectedObjects;
	GetSelectedItems( LeftContainer->CurrentResourceType->SupportInfo, SelectedObjects );

	// Disallow export from cooked packages.
	for( INT Index = 0 ; Index < SelectedObjects.Num() ; ++Index )
	{
		if( SelectedObjects(Index)->GetOutermost()->PackageFlags & PKG_Cooked )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
			return;
		}
	}

	ExportObjects( SelectedObjects, TRUE );
}

void WxGenericBrowser::OnBulkExport(wxCommandEvent& In)
{
	TArray<UPackage*> TopLevelPackages;
	LeftContainer->GetSelectedTopLevelPackages( &TopLevelPackages );

	// Disallow export if any packages are cooked.
	if (DisallowOperationOnCookedPackages(TopLevelPackages))
	{
		return;
	}

	if ( HandleFullyLoadingPackages( TopLevelPackages, TEXT("BulkExportE") ) )
	{
		// Prompt the user to see if they want to export the packages in a single file.
		const UBOOL bDumpSingleFilePackage = appMsgf(AMT_YesNo, *LocalizeUnrealEd(TEXT("Prompt_ExportToSinglePackage")));
		if (bDumpSingleFilePackage)
		{
			ExportPackages(TopLevelPackages);
			return;
		}

		TArray<UObject*> ObjectsToExport;
		RightContainer->GetObjectsInPackages( TopLevelPackages, ObjectsToExport );

		// Prompt the user about how many objects will be exported before proceeding.
		const UBOOL bProceed = appMsgf( AMT_YesNo, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("Prompt_AboutToBulkExportNItems_F")), ObjectsToExport.Num()) ) );
		if ( bProceed )
		{
			ExportObjects( ObjectsToExport, FALSE );
		}
	}
}

void WxGenericBrowser::OnFullLocExport(wxCommandEvent& In)
{
	TArray<UPackage*> TopLevelPackages;
	LeftContainer->GetSelectedTopLevelPackages( &TopLevelPackages );

	// Disallow export if any packages are cooked.
	if (DisallowOperationOnCookedPackages(TopLevelPackages))
	{
		return;
	}

	if( HandleFullyLoadingPackages( TopLevelPackages, TEXT("FullLocExportE") ) )
	{
		const UBOOL bExportLocBinaries = appMsgf( AMT_YesNo, *LocalizeUnrealEd(TEXT("Prompt_ExportLocBinaries")) );
		const UBOOL bCompareAgainstDefaults = appMsgf( AMT_YesNo, *LocalizeUnrealEd(TEXT("Prompt_ExportLocCompareAgainstDefaults")) );
		if ( bExportLocBinaries )
		{
			// 1. Do bulk export first:
			OnBulkExport( In );
		}
		else
		{
			// prompt the user to select a target directory.
			wxDirDialog ChooseDirDialog(
				this,
				*LocalizeUnrealEd("ChooseADirectory"),
				*LastExportPath
				);

			if ( ChooseDirDialog.ShowModal() != wxID_OK )
			{
				return;
			}

			// Copy off the selected path for future export operations.
			LastExportPath = FFilename( ChooseDirDialog.GetPath() );
		}

		// 2. Do exportloc call:
		for ( INT PackageIndex = 0 ; PackageIndex < TopLevelPackages.Num() ; ++PackageIndex )
		{
			UPackage* Package		= TopLevelPackages(PackageIndex);
			FString IntName			= Package->GetName();

			INT i = IntName.InStr( TEXT("."), TRUE );
			if ( i >= 0 )
			{
				IntName = IntName.Left(i);
			}
			IntName += TEXT(".int");

			// Remove path separators from the name.
			i = IntName.InStr( TEXT("/"), TRUE );
			if ( i >= 0 )
			{
				IntName = IntName.Right( IntName.Len()-i-1 );
			}
			i = IntName.InStr( TEXT("\\"), TRUE );
			if ( i >= 0 )
			{
				IntName = IntName.Right( IntName.Len()-i-1 );
			}

			IntName = LastExportPath + PATH_SEPARATOR + IntName;
			FLocalizationExport::ExportPackage( Package, *IntName, bCompareAgainstDefaults, TRUE );
		}
	}
}

void WxGenericBrowser::OnResliceFracturedMeshes(wxCommandEvent& In)
{
	TArray<UPackage*> TopLevelPackages;
	LeftContainer->GetSelectedTopLevelPackages(&TopLevelPackages);
	if (HandleFullyLoadingPackages(TopLevelPackages, TEXT("ResliceFracturedMeshesE")))
	{
		TArray<UObject*> ObjectsToReslice;
		RightContainer->GetObjectsInPackages( TopLevelPackages, ObjectsToReslice );
		for (INT ObjectIndex = 0; ObjectIndex < ObjectsToReslice.Num(); ObjectIndex++)
		{
			UFracturedStaticMesh* FracturedStaticMesh = Cast<UFracturedStaticMesh>(ObjectsToReslice(ObjectIndex));
			if (FracturedStaticMesh)
			{
				// Open a static mesh editor window for the mesh.
				WxStaticMeshEditor StaticMeshEditor( GApp->EditorFrame, -1, FracturedStaticMesh, FALSE );

				// Reslice the mesh.
				StaticMeshEditor.SliceMesh();

				// Close the static mesh editor window.
				StaticMeshEditor.Close();
			}
		}
	}
}

void WxGenericBrowser::OnSCCRefresh( wxCommandEvent& In )
{
	// Sychronize source control state.
	LeftContainer->RequestSCCUpdate();

	// Refresh().
	LeftContainer->UpdateTree();
}

void WxGenericBrowser::OnSCCHistory( wxCommandEvent& In )
{
#if HAVE_SCC
	if ( GApp->SCC )
	{
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedTopLevelPackages( &Packages );
	GApp->SCC->ShowHistory( Packages );
}
#endif // HAVE_SCC
}

void WxGenericBrowser::OnSCCMoveToTrash( wxCommandEvent& In )
{
#if HAVE_SCC
	if ( GApp->SCC )
	{
	// move selected packages to the trashcan
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedTopLevelPackages( &Packages );

		// Disallow if any packages are cooked.
		for( INT PackageIndex = 0 ; PackageIndex < Packages.Num() ; ++PackageIndex )
		{
			const UPackage* Package = Packages(PackageIndex);
			if( Package->PackageFlags & PKG_Cooked )
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
				return;
			}
		}

	for( INT PackageIndex = 0 ; PackageIndex < Packages.Num() ; ++PackageIndex )
	{
		// @todo: check Package SCCState to make sure we can move it (if it's checked out by somone else we can't move it, etc)
		UPackage* Package = Packages(PackageIndex);
		GApp->SCC->MoveToTrash( Package );
	}
}
#endif // HAVE_SCC
}

void WxGenericBrowser::OnSCCAdd( wxCommandEvent& In )
{
#if HAVE_SCC
	if ( GApp->SCC )
	{
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedTopLevelPackages( &Packages );

	if ( Packages.Num() > 0 )
	{
		for( INT PackageIndex = 0 ; PackageIndex < Packages.Num() ; ++PackageIndex )
		{
			UPackage* Package = Packages(PackageIndex);
			GApp->SCC->Add( Package );
		}

		// Sychronize source control state.
		LeftContainer->RequestSCCUpdate();

		// Refresh.
		LeftContainer->UpdateTree();
	}
}
#endif // HAVE_SCC
}

void WxGenericBrowser::OnSCCCheckOut( wxCommandEvent& In )
{
	TArray<UPackage*> Packages;
	
	// get selected packages
	LeftContainer->GetSelectedTopLevelPackages( &Packages );

	// check them out
	CheckOutTopLevelPackages(Packages);
}

/** Helper function that attempts to check out the specified top-level packages. */
void WxGenericBrowser::CheckOutTopLevelPackages(const TArray<UPackage*> Packages) const
{
#if HAVE_SCC
	if ( GApp->SCC )
	{
	// Update to the latest source control state.
	GApp->SCC->Update(Packages);

	UBOOL bCheckedSomethingOut = FALSE;
	for( INT PackageIndex = 0 ; PackageIndex < Packages.Num() ; ++PackageIndex )
	{
		UPackage* Package = Packages(PackageIndex);
		if( Package->SCCCanCheckOut() )
		{
			// The package is still available, so do the check out.
			GApp->SCC->CheckOut( Package );
			bCheckedSomethingOut = TRUE;
		}
		else
		{
			// The status on the package has changed to something inaccessible, so we have to disallow the check out.
			// Don't warn if the file isn't in the depot.
			if ( Package->GetSCCState() != SCC_NotInDepot )
			{			
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_PackageStatusChanged") );
			}
			LeftContainer->UpdateTree();
		}
	}

	// Sychronize source control state if something was checked out.
	if ( bCheckedSomethingOut )
	{
		LeftContainer->RequestSCCUpdate();
	}

	// Refresh.
	LeftContainer->UpdateTree();
}
#endif // HAVE_SCC
}

void WxGenericBrowser::OnSCCCheckIn( wxCommandEvent& In )
{
#if HAVE_SCC
	if ( GApp->SCC )
	{
	TArray<UPackage*> Packages;
	GUnrealEd->GetPackageList( &Packages, NULL );

	if ( Packages.Num() > 0 )
	{
		// Synchronize to source control state before attempting a checkin.
		GApp->SCC->Update(Packages);

		UBOOL bCheckedSomethingIn = FALSE;
		for( INT PackageIndex = 0 ; PackageIndex < Packages.Num() ; ++PackageIndex )
		{
			UPackage* Package = Packages(PackageIndex);
			if( Package->GetSCCState() == SCC_CheckedOut )
			{
				// Only call "CheckIn" once since the user can submit as many files as they want to in a single check-in dialog.
				GApp->SCC->CheckIn( Package );
				bCheckedSomethingIn = TRUE;
				break;
			}
		}

		if ( bCheckedSomethingIn )
		{
			// Sychronize source control state.
			LeftContainer->RequestSCCUpdate();
		}

		// Refresh.
		LeftContainer->UpdateTree();
	}
}
#endif // HAVE_SCC
}

void WxGenericBrowser::OnSCCRevert( wxCommandEvent& In )
{
#if HAVE_SCC
	if ( GApp->SCC )
	{
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedTopLevelPackages( &Packages );

	if ( Packages.Num() > 0 )
	{
		for( INT PackageIndex = 0 ; PackageIndex < Packages.Num() ; ++PackageIndex )
		{
			UPackage* Package = Packages(PackageIndex);
			GApp->SCC->UncheckOut( Package );
		}
		// Sychronize source control state.
		LeftContainer->RequestSCCUpdate();

		// Refresh.
		LeftContainer->UpdateTree();
	}
}
#endif // HAVE_SCC
}

void WxGenericBrowser::UI_SCCAdd( wxUpdateUIEvent& In )
{
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedTopLevelPackages( &Packages );

	UBOOL bEnable = FALSE;

	for( INT x = 0 ; x < Packages.Num() ; ++x )
	{
		if( Packages(x)->GetSCCState() == SCC_NotInDepot )
		{
			bEnable = TRUE;
			break;
		}
	}

	In.Enable( bEnable == TRUE );
}

void WxGenericBrowser::UI_SCCCheckOut( wxUpdateUIEvent& In )
{
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedTopLevelPackages( &Packages );

	UBOOL bEnable = FALSE;

	for( INT x = 0 ; x < Packages.Num() ; ++x )
	{
		if( Packages(x)->GetSCCState() == SCC_ReadOnly )
		{
			bEnable = TRUE;
			break;
		}
	}

	In.Enable( bEnable == TRUE );
}

void WxGenericBrowser::UI_SCCCheckIn( wxUpdateUIEvent& In )
{
	TArray<UPackage*> Packages;
	GUnrealEd->GetPackageList( &Packages, NULL );

	UBOOL bEnable = FALSE;

	for( INT x = 0 ; x < Packages.Num() ; ++x )
	{
		if( Packages(x)->GetSCCState() == SCC_CheckedOut )
		{
			bEnable = TRUE;
			break;
		}
	}

	In.Enable( bEnable == TRUE );
}

void WxGenericBrowser::UI_SCCRevert( wxUpdateUIEvent& In )
{
	TArray<UPackage*> Packages;
	LeftContainer->GetSelectedTopLevelPackages( &Packages );

	UBOOL bEnable = FALSE;

	for( INT x = 0 ; x < Packages.Num() ; ++x )
	{
		if( Packages(x)->GetSCCState() == SCC_CheckedOut )
		{
			bEnable = TRUE;
			break;
		}
	}

	In.Enable( bEnable == TRUE );
}

/** Pointer to a function Called during GC, after reachability analysis is performed but before garbage is purged. */
typedef void (*EditorPostReachabilityAnalysisCallbackType)();
extern EditorPostReachabilityAnalysisCallbackType EditorPostReachabilityAnalysisCallback;

/** State passed to RestoreStandaloneOnReachableObjects. */
static UPackage* PackageBeingUnloaded = NULL;
static TMap<UObject*,UObject*> ObjectsThatHadFlagsCleared;

/**
 * Called during GC, after reachability analysis is performed but before garbage is purged.
 * Restores RF_Standalone to objects in the package-to-be-unloaded that are still reachable.
 */
void RestoreStandaloneOnReachableObjects()
{
	for( FObjectIterator It ; It ; ++It )
	{
		UObject* Object = *It;
		if ( !Object->HasAnyFlags(RF_Unreachable) && Object->GetOutermost() == PackageBeingUnloaded )
		{
			if ( ObjectsThatHadFlagsCleared.Find(Object) )
			{
				Object->SetFlags(RF_Standalone);
			}
		}
	}
}

void WxGenericBrowser::OnUnloadPackage(wxCommandEvent& In)
{
	TArray<UPackage*> TopLevelPackages;
	LeftContainer->GetSelectedTopLevelPackages( &TopLevelPackages );
	UnloadTopLevelPackages( TopLevelPackages );
}

/** Helper function that attempts to unlaod the specified top-level packages. */
void WxGenericBrowser::UnloadTopLevelPackages(const TArray<UPackage*> TopLevelPackages)
{
	// Split the set of selected top level packages into packages which are dirty (and thus cannot be unloaded)
	// and packages that are not dirty (and thus can be unloaded).
	TArray<UPackage*> DirtyPackages;
	TArray<UPackage*> PackagesToUnload;
	for ( INT PackageIndex = 0 ; PackageIndex < TopLevelPackages.Num() ; ++PackageIndex )
	{
		UPackage* Package = TopLevelPackages(PackageIndex);
		if ( Package->IsDirty() )
		{
			DirtyPackages.AddItem( Package );
		}
		else
		{
			PackagesToUnload.AddItem( Package );
		}
	}

	// Inform the user that dirty packages won't be unloaded.
	if ( DirtyPackages.Num() > 0 )
	{
		FString DirtyPackagesMessage( LocalizeUnrealEd(TEXT("UnloadDirtyPackagesList")) );
		for ( INT PackageIndex = 0 ; PackageIndex < DirtyPackages.Num() ; ++PackageIndex )
		{
			DirtyPackagesMessage += FString::Printf( TEXT("\n    %s"), *DirtyPackages(PackageIndex)->GetName() );
		}
		DirtyPackagesMessage += FString::Printf(TEXT("\n%s"), *LocalizeUnrealEd(TEXT("UnloadDirtyPackagesSave")) );
		appMsgf( AMT_OK, TEXT("%s"), *DirtyPackagesMessage );
	}

	if ( PackagesToUnload.Num() > 0 )
	{
		const FScopedBusyCursor BusyCursor;

		// Complete any load/streaming requests, then lock IO.
		UObject::FlushAsyncLoading();
		(*GFlushStreamingFunc)();
		//FIOManagerScopedLock ScopedLock(GIOManager);

		// Remove potential references to to-be deleted objects from the GB selection set and thumbnail manager preview components.
		GetGBSelection()->DeselectAll();
		GUnrealEd->ThumbnailManager->ClearComponents();

		// Prevent the right container from updating (due to paint events) and creating references to contenxt.
		RightContainer->DisableUpdate();

		// Set the callback for restoring RF_Standalone post reachability analysis.
		// GC will call this function before purging objects, allowing us to restore RF_Standalone
		// to any objects that have not been marked RF_Unreachable.
		EditorPostReachabilityAnalysisCallback = RestoreStandaloneOnReachableObjects;

		UBOOL bScriptPackageWasUnloaded = FALSE;

		GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("Unloading")), TRUE );
		for ( INT PackageIndex = 0 ; PackageIndex < PackagesToUnload.Num() ; ++PackageIndex )
		{
			PackageBeingUnloaded = PackagesToUnload(PackageIndex);
			check( !PackageBeingUnloaded->IsDirty() );
			GWarn->StatusUpdatef( PackageIndex, PackagesToUnload.Num(), *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Unloadingf"), *PackageBeingUnloaded->GetName()) ) );

			PackageBeingUnloaded->bHasBeenFullyLoaded = FALSE;
			if ( PackageBeingUnloaded->PackageFlags & PKG_ContainsScript )
			{
				bScriptPackageWasUnloaded = TRUE;
			}

			// Clear RF_Standalone flag from objects in the package to be unloaded so they get GC'd.
			for( FObjectIterator It ; It ; ++It )
			{
				UObject* Object = *It;
				if( Object->HasAnyFlags(RF_Standalone) && Object->GetOutermost() == PackageBeingUnloaded )
				{
					Object->ClearFlags(RF_Standalone);
					ObjectsThatHadFlagsCleared.Set( Object, Object );
				}
			}

			// Collect garbage.
			UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

			// Cleanup.
			ObjectsThatHadFlagsCleared.Empty();
			PackageBeingUnloaded = NULL;
		}
		GWarn->EndSlowTask();

		// Set the post reachability callback.
		EditorPostReachabilityAnalysisCallback = NULL;

		UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

		RightContainer->EnableUpdate();
		Update();

		// Update the actor browser if a script package was unloaded
		if ( bScriptPackageWasUnloaded )
		{
			GCallbackEvent->Send( CALLBACK_RefreshEditor_ActorBrowser );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxGenericBrowser static member functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns an USelection for the set of objects selected in the generic browser.
 */
USelection* WxGenericBrowser::GetSelection()
{
	return GetGBSelection();
};

/*
 * Called by UEditorEngine::Init() to initialize shared generic browser selection structures.
 */
void WxGenericBrowser::InitSelection()
{
	GetGBSelection() = GEditor->GetSelectedObjects();
	check( GetGBSelection() );
}

/*
 * Called by UEditorEngine::Destroy() to tear down shared generic browser selection structures.
 */
void WxGenericBrowser::DestroySelection()
{
	GetGBSelection() = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxGenericBrowser static member functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
* For use with the templated sort. Sorts by class name, ascending
*/
namespace WxGBLeftContainerCompareFunctions
{
	struct FBrowserTypeDescriptionCompare
	{
		static INT Compare(UGenericBrowserType* A, UGenericBrowserType* B)
		{
			return appStricmp(*(A->GetBrowserTypeDescription()),*(B->GetBrowserTypeDescription()));
		}
	};
}

/**
 * The menu for the left generic browser pane.
 */
class wxGBLeftContainerMenu : public wxMenu
{
public:
	wxGBLeftContainerMenu()
	{
		Append( IDMN_FileSave, *LocalizeUnrealEd("SaveE"), TEXT("") );
		Append( IDMN_FileFullyLoad, *LocalizeUnrealEd("FullyLoadE"), TEXT("") );
		Append( IDM_GenericBrowser_UnloadPackage, *LocalizeUnrealEd("UnloadE"), TEXT("") );
		Append( IDM_IMPORT, *LocalizeUnrealEd("ImportE"), TEXT("") );
		Append( IDM_GenericBrowser_BulkExport, *LocalizeUnrealEd("BulkExportE"), TEXT("") );
		Append( IDM_GenericBrowser_FullLocExport, *LocalizeUnrealEd("FullLocExportE"), TEXT("") );
		Append( IDM_GenericBrowser_ResliceFracturedMeshes, *LocalizeUnrealEd("ResliceFracturedMeshesE"), TEXT("") );

		AppendSeparator();

#if HAVE_SCC
		// Source control
		if( GApp->SCC && GApp->SCC->IsEnabled() )
		{
			wxMenu* SCCMenu = CreateSourceControlMenu();
			Append( -1, *LocalizeUnrealEd("SourceControl"), SCCMenu );
			AppendSeparator();
		}
#endif // HAVE_SCC

		Append( IDMN_CheckPackageForErrors, *LocalizeUnrealEd("CheckForErrorsE"), TEXT("") );

		AppendSeparator();

		Append( IDMN_PackageContext_SetFolder, *LocalizeUnrealEd("SetFolder"), TEXT("") );
		Append( IDMN_PackageContext_RemoveFromFolder, *LocalizeUnrealEd("GenericBrowser_PackageTreeContext_RemoveFromFolder"), TEXT("") );
		
		AppendSeparator();

		wxMenu*	BatchProcessMenu = new wxMenu();
		Append( IDMN_PackageContext_BatchProcess, *LocalizeUnrealEd("BatchProcess"), BatchProcessMenu );

		wxMenu*	SoundGroupMenu = new wxMenu();

		UEnum* SoundClassNames = FindObject<UEnum>( NULL, TEXT( "Engine.AudioDevice.ESoundClassName" ) );
		if( SoundClassNames )
		{
			// Retrieve sound groups from audio device.
			for( INT NameIndex = 0; NameIndex < SoundClassNames->NumEnums() - 1; NameIndex++ )
			{
				// Add sound group to SoundGroup submenu.
				FName SoundClassName = SoundClassNames->GetEnum( NameIndex );
				SoundGroupMenu->Append( IDMN_PackageContext_BatchProcess_SoundGroups_START + NameIndex, *SoundClassName.ToString() );
			}

			// Add SoundGroup submenu to BatchProcess submenu.
			BatchProcessMenu->Append(IDMN_PackageContext_BatchProcess_SoundGroups,*LocalizeUnrealEd("SoundGroups"), SoundGroupMenu);
			BatchProcessMenu->Append(IDMN_PackageContext_ClusterSounds, *LocalizeUnrealEd("ClusterSounds"), TEXT(""));
			BatchProcessMenu->Append(IDMN_PackageContext_ClusterSoundsWithAttenuation, *LocalizeUnrealEd("ClusterSoundsWithAttenuation"), TEXT(""));
		}
		// Texture groups.
		{
			wxMenu*	TextureGroupMenu = new wxMenu();

			// Retrieve texture groups.
			TArray<FString> TextureGroupNames = FTextureLODSettings::GetTextureGroupNames();
			for( INT NameIndex=0; NameIndex<TextureGroupNames.Num(); NameIndex++ )
			{
				// Add texture group to TextureGroup submenu.
				TextureGroupMenu->Append(IDMN_PackageContext_BatchProcess_TextureGroups_START+NameIndex,*TextureGroupNames(NameIndex));
			}

			// Add TextureGroup submenu to BatchProcess submenu.
			BatchProcessMenu->Append(IDMN_PackageContext_BatchProcess_TextureGroups,*LocalizeUnrealEd("TextureGroups"),TextureGroupMenu);
		}
	}
};

BEGIN_EVENT_TABLE( WxGBLeftContainer, wxPanel )
	EVT_TREE_SEL_CHANGED( ID_PKG_GRP_TREE, WxGBLeftContainer::OnTreeSelChanged )
	EVT_TREE_SEL_CHANGING( ID_PKG_GRP_TREE, WxGBLeftContainer::OnTreeSelChanging )
	EVT_SIZE( WxGBLeftContainer::OnSize )
	EVT_MENU_RANGE( IDMN_BatchProcess_START, IDMN_BatchProcess_END, WxGBLeftContainer::OnBatchProcess )
	EVT_MENU( IDMN_PackageContext_ClusterSounds, WxGBLeftContainer::OnClusterSounds )
	EVT_MENU( IDMN_PackageContext_ClusterSoundsWithAttenuation, WxGBLeftContainer::OnClusterSoundsWithAttenuation )
	EVT_MENU( IDMN_CheckPackageForErrors, WxGBLeftContainer::OnCheckPackageForErrors )
	EVT_MENU( IDMN_PackageContext_SetFolder, WxGBLeftContainer::OnSetFolder )
	EVT_MENU( IDMN_PackageContext_RemoveFromFolder, WxGBLeftContainer::OnRemoveFromFolder )
	EVT_CHECKLISTBOX(ID_RESOURCE_CHECKLIST, WxGBLeftContainer::OnResourceFilterListChecked)
	EVT_LISTBOX_DCLICK(ID_RESOURCE_CHECKLIST, WxGBLeftContainer::OnResourceFilterListDoubleClick)
	EVT_CHECKLISTBOX(ID_REFERENCER_CONTAINER_LIST, WxGBLeftContainer::OnUsageFilterContainerListChecked)
	EVT_LISTBOX_DCLICK(ID_REFERENCER_CONTAINER_LIST, WxGBLeftContainer::OnUsageFilterContainerListDoubleClick)
	EVT_CHECKBOX( ID_RESOURCE_SHOWALL, WxGBLeftContainer::OnShowAllTypesChanged )
	EVT_UPDATE_UI( ID_RESOURCE_SHOWALL, WxGBLeftContainer::UI_ShowAllTypes )
END_EVENT_TABLE()

WxGBLeftContainer::WxGBLeftContainer( wxWindow* InParent )
	:	wxPanel( InParent, -1 )
	,	ResourceAllType(NULL), ResourceFilterType(NULL), bTreeViewChanged(FALSE), bIsSCCStateDirty( TRUE )
{
	wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		// Create and bold the show all resource types checkbox.
		ShowAllCheckBox = new wxCheckBox(this, ID_RESOURCE_SHOWALL, *LocalizeUnrealEd("ShowAllResourceTypes"));

		MainSizer->Add(ShowAllCheckBox, 0, wxEXPAND | wxALL, 5);
		PackageSplitter = new wxSplitterWindow(this, wxID_ANY);
		{
			// Create a check list box with all of our filter types.
			ResourceFilterList = new wxCheckListBox(PackageSplitter, ID_RESOURCE_CHECKLIST, wxDefaultPosition, wxDefaultSize, 0, NULL);
			UsageFilterHSplitter = new wxSplitterWindow(PackageSplitter, wxID_ANY);
			{
				// Create package tree view, and add a drop target for files.
				TreeCtrl = new WxTreeCtrl( UsageFilterHSplitter, ID_PKG_GRP_TREE, new wxGBLeftContainerMenu(), wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_MULTIPLE | wxTR_LINES_AT_ROOT );
				TreeCtrl->SetDropTarget( new WxDropTargetFileImport( TreeCtrl ) );
				// Populate the tree/package map with outermost packages.
				TreeCtrl->AddRoot( *LocalizeUnrealEd("Packages") );

				// Create the list of usage filter containers
				UsageFilterContainerList = new wxCheckListBox( UsageFilterHSplitter, ID_REFERENCER_CONTAINER_LIST );
			}
		}
		PackageSplitter->SplitHorizontally(ResourceFilterList, UsageFilterHSplitter, STD_SPLITTER_SZ);
		PackageSplitter->SetSashGravity(0.1);

		const INT MinPaneSize = 20;
		PackageSplitter->SetMinimumPaneSize( MinPaneSize );

		UsageFilterHSplitter->SplitHorizontally( TreeCtrl, UsageFilterContainerList, STD_SPLITTER_SZ );
		UsageFilterHSplitter->SetSashGravity(1.0);
		UsageFilterHSplitter->SetMinimumPaneSize( MinPaneSize );

		MainSizer->Add(PackageSplitter, 1, wxEXPAND);
	}
	SetSizer(MainSizer);



	// Create the list of available resource types.
	for( TObjectIterator<UClass> ItC ; ItC ; ++ItC )
	{
		const UBOOL bIsAllType = (*ItC == UGenericBrowserType_All::StaticClass());
		const UBOOL bIsCustomType = (*ItC == UGenericBrowserType_Custom::StaticClass());

		if( !bIsAllType && !bIsCustomType && ItC->IsChildOf(UGenericBrowserType::StaticClass()) && !(ItC->ClassFlags&CLASS_Abstract) )
		{
			UGenericBrowserType* ResourceType = ConstructObject<UGenericBrowserType>( *ItC );
			if( ResourceType )
			{
				ResourceType->Init();
				ResourceTypes.AddItem( ResourceType );
			}
		}
	}


	// Sort the list of resource types
	Sort<UGenericBrowserType*, WxGBLeftContainerCompareFunctions::FBrowserTypeDescriptionCompare>( &(ResourceTypes(0)), ResourceTypes.Num() );

	ResourceFilterList->Freeze();
	{
		for( INT x = 0 ; x < ResourceTypes.Num() ; ++x )
		{
			// All generic browser types should have a description set for proper operation of the GB.
			checkMsg( (ResourceTypes(x)->GetBrowserTypeDescription().Len() > 0), "UGenericBrowserType has no description set." );

			ResourceFilterList->Append( *ResourceTypes(x)->GetBrowserTypeDescription() );
		}
	}
	ResourceFilterList->Thaw();

	// Create the "all" and "custom" types last as it needs all the others to be created before it.
	ResourceAllType = ConstructObject<UGenericBrowserType>( UGenericBrowserType_All::StaticClass() );
	ResourceAllType->Init();

	ResourceFilterType = ConstructObject<UGenericBrowserType>(UGenericBrowserType_Custom::StaticClass());

	// Default selection to the "All" GenericBrowserType.
	CurrentResourceType = ResourceAllType;
	bShowAllTypes = TRUE;


	// initialize the usage filter container list
	UsageFilterContainerList->Freeze();
	{
		// initialize the enabled status of each filter type to its defaults
		bUsageFilterContainers[USAGEFILTER_Content] = FALSE;
		bUsageFilterContainers[USAGEFILTER_AllLevels] = TRUE;
		bUsageFilterContainers[USAGEFILTER_CurrentLevel] = FALSE;
		bUsageFilterContainers[USAGEFILTER_VisibleLevels] = FALSE;

		// now add checkboxes for each type to the list
		for ( INT Index = 0; Index < USAGEFILTER_MAX; Index++ )
		{
			FString UsageFilterTypeString = *LocalizeUnrealEd(*FString::Printf(TEXT("UsageFilterTypes[%d]"), Index));
			UsageFilterContainerList->Append( *UsageFilterTypeString );
		}
	}
	UsageFilterContainerList->Thaw();
}

WxGBLeftContainer::~WxGBLeftContainer()
{
	SaveSettings();
}

/**
* Loads the window settings specific to this container.
*/
void WxGBLeftContainer::LoadSettings()
{
	// Load the sash position for the horizontal splitter.
	INT SashPos;
	GConfig->GetInt( TEXT("GenericBrowser"), TEXT("Package_Splitter_Sash_Position"), SashPos, GEditorUserSettingsIni );
	PackageSplitter->SetSashPosition(SashPos);

	// Load the value of the bShowAllTypes flag.
	GConfig->GetBool( TEXT("GenericBrowser"), TEXT("Package_Show_All_Types"), bShowAllTypes, GEditorUserSettingsIni );

	// Load list of selected filters, it is assumed that the list is sorted so we can quickly check items.
	FString FilterClass;
	GConfig->GetString ( TEXT("GenericBrowser"), TEXT("Package_Filter_List"), FilterClass, GEditorUserSettingsIni );

	INT CurrentFilterIdx = 0;
	TArray<FString> Filters;
	FilterClass.ParseIntoArray(&Filters, TEXT(","), TRUE);
	
	if( Filters.Num() )
	{	
		for( UINT ListIdx = 0; ListIdx < ResourceFilterList->GetCount() && CurrentFilterIdx < Filters.Num() ; ListIdx++)
		{
			const FString& ListString = ResourceTypes(ListIdx)->GetBrowserTypeDescription();
			const UBOOL bHasFilter = Filters.ContainsItem( ListString );
			
			if( bHasFilter )
			{
				ResourceFilterList->Check( ListIdx );
			}
		}
	}

	// "In-use" filter settings
	GConfig->GetInt( TEXT("GenericBrowser"), TEXT("UsageFilter_Splitter_Sash_Position"), UsageFilterSashPos, GEditorUserSettingsIni );
	UsageFilterHSplitter->SetSashPosition(UsageFilterSashPos);

	// Load list of containers to use for the usage filter
	UsageFilterContainerList->Freeze();
	for ( INT Index = 0; Index < USAGEFILTER_MAX; Index++ )
	{
		FString Key = FString::Printf(TEXT("UsageFilterEnabled[%d]"), Index);
		GConfig->GetBool( TEXT("GenericBrowser"), *Key, bUsageFilterContainers[Index], GEditorUserSettingsIni );
		UsageFilterContainerList->Check( Index, bUsageFilterContainers[Index] == TRUE );
	}
	UsageFilterContainerList->Thaw();
	SetUsageFilter(GenericBrowser->IsUsageFilterEnabled(), TRUE);

	// Update widgets and local flags.
	SetShowAllTypes( bShowAllTypes );
}

/**
* Saves the window settings specific to this container.
*/
void WxGBLeftContainer::SaveSettings()
{
	// Save the sash position for the horizontal splitter.
	INT SashPos = PackageSplitter->GetSashPosition();
	GConfig->SetInt( TEXT("GenericBrowser"), TEXT("Package_Splitter_Sash_Position"), UsageFilterSashPos, GEditorUserSettingsIni );

	// Save the value of the bShowAllTypes flag.
	GConfig->SetBool( TEXT("GenericBrowser"), TEXT("Package_Show_All_Types"), bShowAllTypes, GEditorUserSettingsIni );

	// Save Selected Resource Types.
	FString FilterList;
	UBOOL bFirstItem = TRUE;

	for(UINT ListIdx = 0; ListIdx < ResourceFilterList->GetCount(); ListIdx++)
	{
		const UBOOL bItemChecked = ResourceFilterList->IsChecked( ListIdx );

		if( bItemChecked )
		{
			if( bFirstItem )
			{
				FilterList = ResourceTypes(ListIdx)->GetBrowserTypeDescription();
				bFirstItem = FALSE;
			}
			else
			{
				FilterList += ",";
				FilterList += ResourceTypes(ListIdx)->GetBrowserTypeDescription();
			}
		}
	}

	GConfig->SetString( TEXT("GenericBrowser"), TEXT("Package_Filter_List"), *FilterList , GEditorUserSettingsIni );

	// "In-use" filter settings
	GConfig->SetInt( TEXT("GenericBrowser"), TEXT("UsageFilter_Splitter_Sash_Position"), UsageFilterSashPos, GEditorUserSettingsIni );

	// Save list of containers to use for the usage filter
	for ( INT Index = 0; Index < USAGEFILTER_MAX; Index++ )
	{
		FString Key = FString::Printf(TEXT("UsageFilterEnabled[%d]"), Index);
		GConfig->SetBool( TEXT("GenericBrowser"), *Key, bUsageFilterContainers[Index], GEditorUserSettingsIni );
	}
}


void WxGBLeftContainer::Update()
{
	UpdateTree();
}

/**
 * Requests that source control state be updated.
 */
void WxGBLeftContainer::RequestSCCUpdate()
{
	bIsSCCStateDirty = TRUE;
}

/**
 * Gets a list of packages that are editor only and should not be displayed.
 *
 * @param	PackageList		Receives the list of editor only packages.
 */
void WxGBLeftContainer::GetEditorOnlyPackages(TArray<const UPackage*> &PackageList)
{
	TArray<FString> EditorOnlyContentPackageNames;

	if ( GConfig->GetArray(TEXT("Editor.EditorEngine"), TEXT("EditorOnlyContentPackages"), EditorOnlyContentPackageNames, GEngineIni) )
	{
		for ( INT PkgIndex = 0; PkgIndex < EditorOnlyContentPackageNames.Num(); PkgIndex++ )
		{
			UPackage* Package = FindObject<UPackage>(NULL, *EditorOnlyContentPackageNames(PkgIndex), TRUE);
			if ( Package != NULL )
			{
				PackageList.AddUniqueItem(Package);
			}
			}
		}
	}

/**
 * Filters the global set of packages.
 *
 * @param	OutFilteredPackageMap		The map that will contain the filtered set of packages.
 * @param	OutFilteredLevelPackages	The map that will contain the filtered set of level packages.
 * @param	OutPackageList				The array that will contain the list of filtered packages.
 */
void WxGBLeftContainer::GetFilteredPackageList(TMap<const UPackage*, const UPackage*> &OutFilteredPackageMap, TMap<const UPackage*, const UPackage*> &OutFilteredLevelPackages, TArray<UPackage*> &OutPackageList)
{
	const UBOOL bUsageFilterEnabled = GenericBrowser->IsUsageFilterEnabled();
	if ( bUsageFilterEnabled )
	{
		GenericBrowser->MarkReferencedObjects();
	}

	// Assemble a list of level packages for levels referenced by the current world.
	// Handle GWorld potentially being NULL e.g. if refreshing between level transitions.
	TMap<const UPackage*, const UPackage*> LevelPackages;
	if ( GWorld )
	{
		const UPackage* CurrentLevelPackage = GWorld->CurrentLevel->GetOutermost();
		LevelPackages.Set( CurrentLevelPackage, CurrentLevelPackage );

		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		for ( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if ( StreamingLevel && StreamingLevel->LoadedLevel )
			{
				const UPackage* LevelPackage = StreamingLevel->LoadedLevel->GetOutermost();
				LevelPackages.Set( LevelPackage, LevelPackage );
			}
		}
	}

	// Assemble a list of packages.  Only show packages that match the current resource type filter.
	for( TObjectIterator<UObject> It ; It ; ++It )
	{
		UObject* Obj = *It;
		if ( bUsageFilterEnabled && !Obj->HasAnyFlags(RF_TagExp) )
		{
			continue;
		}

		const UBOOL bIsRedirector = Obj->IsA( UObjectRedirector::StaticClass() );

		if( !bIsRedirector && CurrentResourceType->Supports( Obj ) )
		{
			const UPackage* ObjectPackage				= Obj->GetOutermost();
			if ( ObjectPackage && !OutFilteredPackageMap.FindRef( ObjectPackage ) )
			{
				const UBOOL	bIsLevelPackage				= LevelPackages.FindRef( ObjectPackage ) != NULL;
				const UBOOL	bIsInPackageContainingMap	= ObjectPackage->ContainsMap();
				const UBOOL	bShowObject					=  (Obj->HasAnyFlags(RF_Public) || bIsLevelPackage)
														&& (!bIsInPackageContainingMap || bIsLevelPackage)
														&& (bUsageFilterEnabled || !(ObjectPackage->PackageFlags & PKG_Trash))
														// don't display texture if its outer is a UFont
														&& !(Cast<UTexture>(Obj) && Cast<UFont>(Obj->GetOuter()));
				if( bShowObject )
				{
					if( ObjectPackage != UObject::GetTransientPackage() 
					&&	!(ObjectPackage->PackageFlags & PKG_PlayInEditor) )
					{
						OutFilteredPackageMap.Set( ObjectPackage, ObjectPackage );
					}
				}
			}
		}
	}

	// Filter the LevelPackages list against the PackageList to create a list of level packages
	// that match the current resource type filter.
	for ( TMap<const UPackage*, const UPackage*>::TConstIterator It( LevelPackages ) ; It ; ++It )
	{
		const UPackage* LevelPackage = It.Key();
		if ( OutFilteredPackageMap.FindRef( LevelPackage ) )
		{
			OutFilteredLevelPackages.Set( LevelPackage, LevelPackage );
		}
	}

	// Make a TArray copy of PackageMap.
	for ( TMap<const UPackage*, const UPackage*>::TConstIterator It( OutFilteredPackageMap ) ; It ; ++It )
	{
		OutPackageList.AddItem( const_cast<UPackage*>( It.Key() ) );
	}

	// Let the source control system update the package flags.
	if ( bIsSCCStateDirty )
	{
		bIsSCCStateDirty = FALSE;
#if HAVE_SCC
		if ( GApp->SCC )
		{
			GApp->SCC->Update(OutPackageList);
		}
#endif // HAVE_SCC
	}
}

/**
 * Gets a filtered list of group packages.
 *
 * @param	InFilteredLevelPackages		The filtered list of level packages
 * @param	OutGroupPackages			The map that receives the filtered list of group packages.
 */
void WxGBLeftContainer::GetFilteredGroupList(TMap<const UPackage*, const UPackage*> &InFilteredLevelPackages, TMap<UPackage*,UPackage*> &OutGroupPackages)
{
	// The UObject list is iterated rather than the UPackage list because we need to be sure we are only adding
	// group packages that contain things the generic browser cares about.  The packages are derived by walking
	// the outer chain of each object.

	for( TObjectIterator<UObject> It ; It ; ++It )
	{
		UObject*	Obj							= *It;
		if ( !CurrentResourceType->Supports( Obj ) )
		{
			continue;
		}

		const UBOOL bUsageFilterEnabled = GenericBrowser->IsUsageFilterEnabled();
		if ( bUsageFilterEnabled && !Obj->HasAnyFlags(RF_TagExp) )
		{
			continue;
		}

		UPackage*	ObjectPackage				= Obj->GetOutermost();
		const UBOOL	bIsInLevelPackage			= ObjectPackage && InFilteredLevelPackages.FindRef( ObjectPackage );
		UBOOL		bIsInPackageContainingMap	= ObjectPackage ? ObjectPackage->ContainsMap() : FALSE;
		UBOOL		bShowObject					=	(Obj->HasAnyFlags(RF_Public) || bIsInLevelPackage)
			&& (!bIsInPackageContainingMap || bIsInLevelPackage)
			&& !Cast<UObjectRedirector>(Obj) 
			&& !Cast<UPackage>(Obj)
			&& (bUsageFilterEnabled || !(ObjectPackage->PackageFlags & PKG_Trash))
			// don't display texture if its outer is a UFont
			&& !(Cast<UTexture>(Obj) && Cast<UFont>(Obj->GetOuter()));

		if( bShowObject )
		{
			for( UPackage* pkg = (UPackage*)Obj->GetOuter() ; pkg->GetOuter() ; pkg = (UPackage*)pkg->GetOuter() )
			{
				if( pkg->IsA( UPackage::StaticClass() ) && !OutGroupPackages.FindRef( pkg ) )
				{
					OutGroupPackages.Set( pkg, pkg );
				}
			}
		}
	}
}

/*
 * Returns whether or not a package entry should be remnoved from the tree view.
 *
 * @param	Item					The item to be checked for removal.
 * @param	InFilteredPackages		The filtered list of packages.
 * @param	InGroupPackages			The list of groups.
 */
UBOOL WxGBLeftContainer::RemoveItemFromPackageTree(const wxTreeItemId &Item, TMap<const UPackage*, const UPackage*> &InFilteredPackages, TMap<UPackage*,UPackage*> &InGroupPackages)
{
	WxTreeObjectWrapper *Wrapper = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(Item);

	if(Wrapper)
	{
		UPackage *Package = Wrapper->GetObject<UPackage>();

		if(InFilteredPackages.FindRef(Package) || InGroupPackages.FindRef(Package))
		{
			return FALSE;
		}
	}
	else if(TreeCtrl->ItemHasChildren(Item)) // No data associated with it means its a folder so don't remove if it has children
	{
		return FALSE;
	}

	return TRUE;
}

/*
 * Recursively updates the package list within the tree view.
 *
 * @param	ParentId				The current parent node.
 * @param	InFilteredPackages		The filtered list of packages.
 * @param	InGroupPackages			The list of groups.
 * @param	TreeItemsToDeleteBuffer	A buffer containing package entries that are no longer valid and require deleting.
 */
void WxGBLeftContainer::RecursivelyUpdatePackageList(const wxTreeItemId &ParentId, TMap<const UPackage*, const UPackage*> &InFilteredPackages, TMap<UPackage*,UPackage*> InGroupPackages, TArray<wxTreeItemId> &TreeItemsToDeleteBuffer)
{
	wxTreeItemIdValue Cookie;

	wxTreeItemId Child = TreeCtrl->GetFirstChild(ParentId, Cookie);

	while(Child.IsOk())
	{
		if(RemoveItemFromPackageTree(Child, InFilteredPackages, InGroupPackages))
		{
			TreeItemsToDeleteBuffer.AddItem(Child);
		}
		else
		{
			UpdateTreeViewPackageItem(Child);
			RecursivelyUpdatePackageList(Child, InFilteredPackages, InGroupPackages, TreeItemsToDeleteBuffer);
		}

		Child = TreeCtrl->GetNextChild(ParentId, Cookie);
	}
}

/**
 * Updates the appearance of a package entry within the package tree view
 *
 * @param	Item	The package entry that needs updating.
 */
void WxGBLeftContainer::UpdateTreeViewPackageItem(const wxTreeItemId &Item)
{
	WxTreeObjectWrapper *Wrapper = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(Item);

	if(!Wrapper)
	{
		return;
	}

	UPackage *Package = Wrapper->GetObject<UPackage>();

	// Set the package icon.
	const INT IconIndex = SCCGetIconIndex(Package);
	TreeCtrl->SetItemImage(Item, IconIndex, wxTreeItemIcon_Normal);
	TreeCtrl->SetItemImage(Item, IconIndex, wxTreeItemIcon_Selected);

	FString PackageName = Package->GetName();

	// If top level, and dirty, add a star
	if(Package->GetOuter() == NULL && Package->bDirty)
	{
		PackageName = PackageName + FString(TEXT("*"));			
	}

	Package = Package->GetOuter() == NULL ? Package : Package->GetOutermost();

	TreeCtrl->SetItemText(Item, *PackageName);

	// Mark any packages the user has checked out in bold.
	if( Package->SCCCanEdit() )
	{
		TreeCtrl->SetItemBold( Item, true );
	}
	else
	{
		TreeCtrl->SetItemBold( Item, false );
	}

	if( !Package->IsFullyLoaded() )
	{
		wxColour Color = (Package->PackageFlags & PKG_Trash) == 0
			? wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT )
			: wxColour(255, 100, 100);

		TreeCtrl->SetItemTextColour(Item, Color );
	}
	else if ( (Package->PackageFlags & PKG_Trash) != 0 )
	{
		TreeCtrl->SetItemTextColour(Item, wxColour(255, 32, 32));
	}
	else
	{
		TreeCtrl->SetItemTextColour(Item, wxColour(0, 0, 0));
	}
}

/**
 * Recursively copies a branch in the tree view to the provided parent node.
 *
 * @param	DestParent	The destination node that will becoke the parent of the source branch.
 * @param	Src			The source branch to be copied.
 */
void WxGBLeftContainer::CopyPackageTreeBranch(const wxTreeItemId& DestParent, const wxTreeItemId& Src)
{
	wxTreeItemId NewParent = TreeCtrl->AppendItem(DestParent, TreeCtrl->GetItemText(Src), TreeCtrl->GetItemImage(Src), TreeCtrl->GetItemImage(Src, wxTreeItemIcon_Selected));

	WxTreeObjectWrapper *Wrapper = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(Src);

	if(Wrapper)
	{
		UPackage *Package = Wrapper->GetObject<UPackage>();
		TreeCtrl->SetItemData(NewParent, new WxTreeObjectWrapper(Package));

		PackageMap.Set(Package, NewParent);
	}

	wxTreeItemIdValue Cookie;
	wxTreeItemId Child = TreeCtrl->GetFirstChild(Src, Cookie);

	while(Child.IsOk())
	{
		CopyPackageTreeBranch(NewParent, Child);
		Child = TreeCtrl->GetNextChild(Src, Cookie);
	}
}

/**
 * Recursively removes packages from the package map.
 *
 * @param	Id		The parent id to remove.
 */
void WxGBLeftContainer::RecursivelyRemovePackages(const wxTreeItemId &Id)
{
	WxTreeObjectWrapper *Wrapper = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(Id);

	if(Wrapper)
	{
		UPackage *Package = Wrapper->GetObject<UPackage>();

		PackageMap.Remove(Package);
	}
	else //it's a folder
	{
		FolderMap.Remove(FString(TreeCtrl->GetItemText(Id)));
	}


	wxTreeItemIdValue Cookie;
	wxTreeItemId Child = TreeCtrl->GetFirstChild(Id, Cookie);

	while(Child.IsOk())
	{
		RecursivelyRemovePackages(Child);
		Child = TreeCtrl->GetNextChild(Id, Cookie);
	}
}

/**
 * Updates the tree control that contains the list of loaded packages.
 */
void WxGBLeftContainer::UpdateTree()
{
	// Remember the current selections.
	TArray<UPackage*> SelectedPackages;
	GetSelectedPackages( &SelectedPackages );

	TArray<const UPackage*> EditorOnlyContentPackages;
	GetEditorOnlyPackages(EditorOnlyContentPackages);

	// Init.
	TreeCtrl->Freeze();

	TMap<const UPackage*, const UPackage*> FilteredPackageMap;
	TMap<const UPackage*, const UPackage*> FilteredLevelPackages;
	TArray<UPackage*> PackageList;

	GetFilteredPackageList(FilteredPackageMap, FilteredLevelPackages, PackageList);

	TMap<UPackage*,UPackage*> GroupPackages;
	GetFilteredGroupList(FilteredLevelPackages, GroupPackages);

	TArray<wxTreeItemId> TreeItemsToDeleteBuffer;
	RecursivelyUpdatePackageList(TreeCtrl->GetRootItem(), FilteredPackageMap, GroupPackages, TreeItemsToDeleteBuffer);

	for(INT ItemIndex = 0; ItemIndex < TreeItemsToDeleteBuffer.Num(); ++ItemIndex)
	{
		wxTreeItemId Id = TreeItemsToDeleteBuffer(ItemIndex);

		RecursivelyRemovePackages(Id);

		TreeCtrl->Delete(Id);
	}

	for(TMap<const UPackage*, const UPackage*>::TConstIterator It( FilteredPackageMap ); It; ++It)
	{
		UPackage* Package = const_cast<UPackage*>(It.Key());

		if(!PackageMap.Find(Package) && !EditorOnlyContentPackages.ContainsItem(Package))
		{
			FString PackageName = Package->GetName();
			FString FolderName;
			wxTreeItemId *FolderItem = NULL;

			Package->GetFolderName().ToString(FolderName);

			if(FolderName.Len() > 0 && FolderName.ToLower() != TEXT("none"))
			{
				FolderItem = FolderMap.Find(*FolderName);

				if(!FolderItem)
				{
					wxTreeItemId FolderId = TreeCtrl->AppendItem(TreeCtrl->GetRootItem(), *FolderName, GBTCI_FolderClosed, -1);

					FolderMap.Set(*FolderName, FolderId);
					FolderItem = FolderMap.Find(*FolderName);
				}
			}

			// If top level, and dirty, add a star
			if(Package->GetOuter() == NULL && Package->bDirty)
			{
				PackageName = PackageName + FString(TEXT("*"));			
			}

			const wxTreeItemId Id = TreeCtrl->AppendItem(FolderItem ? *FolderItem : TreeCtrl->GetRootItem(), *PackageName, GBTCI_FolderClosed, -1, new WxTreeObjectWrapper(Package));
			PackageMap.Set( Package, Id );

			UpdateTreeViewPackageItem(Id);
		}
	}

	TArray<UPackage*> GroupList;
	for(TMap<UPackage*, UPackage*>::TIterator Iter(GroupPackages); Iter; ++Iter)
	{
		GroupList.AddItem(Iter.Value());
	}

	// Since objects (and hence, packages) can appear in any order in memory, we need to add the group packages to the tree
	// iteratively.  Each pass checks each group in the GroupPackages array against the package map to see if its immediate parent 
	// has been added to the tree already.  If it has, it adds the group package as a child of that parent item and removes it from the 
	// GroupPackages array.  If not, it ignores that group and leaves it to be handled in a future iteration.  Once all the 
	// groups are removed from the GroupPackages array, the tree is fully loaded.

	while(GroupList.Num())
	{
		for(INT x = 0; x < GroupList.Num(); ++x)
		{
			UPackage* CurGroup = GroupList(x);

			if(PackageMap.Find(CurGroup))
			{
				GroupList.Remove(x);
				x--;
				continue;
			}

			wxTreeItemId* parentid = PackageMap.Find( Cast<UPackage>(CurGroup->GetOuter()) );

			// If this packages outer is in the tree, add this package to the tree as a child of that item and remove it from the array.

			if(EditorOnlyContentPackages.ContainsItem(Cast<UPackage>(CurGroup->GetOuter())))
			{
				GroupList.Remove(x);
				x--;
			}
			else if( parentid )
			{
				const wxTreeItemId Id = TreeCtrl->AppendItem( *parentid, *CurGroup->GetName(), GBTCI_FolderClosed, -1, new WxTreeObjectWrapper( CurGroup ) );
				PackageMap.Set( CurGroup, Id );

				UPackage* pkg = CurGroup->GetOutermost();
				const INT idx = SCCGetIconIndex( pkg );

				TreeCtrl->SetItemImage( Id, idx, wxTreeItemIcon_Normal );
				TreeCtrl->SetItemImage( Id, idx, wxTreeItemIcon_Selected );

				UpdateTreeViewPackageItem(Id);

				GroupList.Remove(x);
				x--;
			}
		}
	}

	// Sort the tree - Packages
	for( TMap<UPackage*,wxTreeItemId>::TConstIterator It(PackageMap) ; It ; ++It )
	{
		TreeCtrl->SortChildren( It.Value() );
	}

	// Sort the tree - Folders
	for( TMap<FString, wxTreeItemId>::TConstIterator It(FolderMap); It ; ++It)
	{
		TreeCtrl->SortChildren( It.Value() );
	}


	// Sort the root of the tree
	TreeCtrl->SortChildren( TreeCtrl->GetRootItem() );

	// Find the packages that were previously selected and make sure they stay that way
	for( INT x = 0 ; x < SelectedPackages.Num() ; ++x )
	{
		UPackage* package = (UPackage*)SelectedPackages(x);
		const wxTreeItemId* tid = PackageMap.Find( package );
		if( tid )
		{
			TreeCtrl->SelectItem( *tid );
			TreeCtrl->EnsureVisible( *tid );
		}
	}

	// NOTE: We no longer need to expand the root item since we create the tree control with the wxTR_HIDE_ROOT option
	//	TreeCtrl->Expand( TreeCtrl->GetRootItem() );

	TreeCtrl->Thaw();

	// clear all mark flags
	for ( FObjectIterator It; It; ++It )
	{
		It->ClearFlags(RF_TagExp);
	}
}

/**
 * Opens the tree control to the package/group belonging to the specified object.
 *
 * @param	InObject	The object to sync to
 */
 
void WxGBLeftContainer::SyncToObject( UObject* InObject )
{
	UPackage* pkg = (UPackage*)InObject->GetOuter();
	wxTreeItemId* tid = PackageMap.Find( pkg );

	if( tid )
	{
		TreeCtrl->SelectItem( *tid );
		TreeCtrl->EnsureVisible( *tid );
	}
}

// Fills InPackages with a list of selected packages/groups

void WxGBLeftContainer::GetSelectedPackages( TArray<UPackage*>* InPackages ) const
{
	InPackages->Empty();
	
	// Cookie is necessary for iterating through the wxTreeControl
	wxTreeItemIdValue Cookie;

	wxArrayTreeItemIds tids;
	TreeCtrl->GetSelections( tids );

	for( UINT x = 0 ; x < tids.GetCount() ; ++x )
	{
		if( TreeCtrl->IsSelected( tids[x] ) )
		{
			// If we're looking at a package (as package TreeItems have associated data)
			if ( TreeCtrl->GetItemData(tids[x]) )
			{
				// Simply add the item to the list
				WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(tids[x]);
				if( ItemData )
				{
					InPackages->AddUniqueItem( ItemData->GetObjectChecked<UPackage>() );
				}
			}
			else // We are probably on a folder
			{
				// Iterate over all the children in the folder and add them to out list of selected packages
				wxTreeItemId PackageId = TreeCtrl->GetFirstChild(tids[x], Cookie);
				while (PackageId.IsOk())
				{
					WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(PackageId);
					if ( ItemData )
					{
						InPackages->AddUniqueItem(ItemData->GetObjectChecked<UPackage>());
					}
					
					PackageId = TreeCtrl->GetNextChild(tids[x], Cookie);
				}
			}
		}
	}

}

void WxGBLeftContainer::GetSelectedTopLevelPackages(TArray<UPackage*>* InPackages) const
{
	InPackages->Empty();

	wxArrayTreeItemIds tids;
	TreeCtrl->GetSelections( tids );

	for( UINT x = 0 ; x < tids.GetCount() ; ++x )
	{
		if( TreeCtrl->IsSelected( tids[x] ) && TreeCtrl->GetItemData(tids[x]) )
		{
			WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(tids[x]);
			if( ItemData )
			{
				UPackage* Package = CastChecked<UPackage>( ItemData->GetObjectChecked<UPackage>()->GetOutermost() );
				InPackages->AddUniqueItem( Package );
			}
		}
	}
}

/**
 * Toggles the visibility of the level list
 *
 * @param	bUsageFilterEnabled		specify TRUE to display the list of containers that can be enabled for the "in-use" filter
 * @param	bInitialUpdate			specify TRUE when calling SetUsageFilter during initialization of this panel
 */
void WxGBLeftContainer::SetUsageFilter( UBOOL bUsageFilterEnabled, UBOOL bInitialUpdate/*=FALSE*/ )
{
	if ( bUsageFilterEnabled == FALSE && bInitialUpdate == FALSE )
	{
		// store the current value of the splitter's sash before unsplitting the window
		UsageFilterSashPos = UsageFilterHSplitter->GetSashPosition();
	}

	TreeCtrl->Show();
	UsageFilterContainerList->Show();

	UsageFilterHSplitter->SplitHorizontally(TreeCtrl, UsageFilterContainerList, UsageFilterSashPos);
	UsageFilterHSplitter->SetSashPosition( UsageFilterSashPos );

	if ( bUsageFilterEnabled == FALSE )
	{
		UsageFilterHSplitter->Unsplit(UsageFilterContainerList);
	}
}


/**
 * Returns the list of container objects that will be used to filter which resources are displayed when "Show Referenced Only" is enabled.
 * Normally these will correspond to ULevels (i.e. the currently loaded levels), but may contain other packages such as script or UI packages.
 *
 * @param	out_ReferencerContainers	receives the list of container objects that will be used for filtering which objects are displayed in the GB
 *										when "In Use Only" is enabled
 */
void WxGBLeftContainer::GetReferencerContainers( TArray<UObject*>& out_ReferencerContainers )
{
	const FScopedBusyCursor BusyCursor;
	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("ReferenceFilter")),TRUE );

	out_ReferencerContainers.Empty();

	if ( bUsageFilterContainers[USAGEFILTER_Content] == TRUE )
	{
		// if "references from content" is enabled, add all currently loaded packages to the root set so that we can find references to this object from
		// any loaded package
		for ( TMap<UPackage*,wxTreeItemId>::TConstIterator It(PackageMap); It; ++It )
		{
			UPackage* VisiblePackage = It.Key();
			out_ReferencerContainers.AddItem(VisiblePackage);
		}
	}

	if ( GWorld != NULL && GWorld->PersistentLevel != NULL )
	{
		if ( bUsageFilterContainers[USAGEFILTER_CurrentLevel] == TRUE	||
			bUsageFilterContainers[USAGEFILTER_AllLevels] == TRUE		||
			bUsageFilterContainers[USAGEFILTER_VisibleLevels] == TRUE	)
		{
			out_ReferencerContainers.AddItem(GWorld->PersistentLevel);

			AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
			for ( INT LevelIndex = 0; LevelIndex < WorldInfo->StreamingLevels.Num(); LevelIndex++ )
			{
				ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
				if ( StreamingLevel != NULL && StreamingLevel->LoadedLevel != NULL )
				{
					UBOOL bShouldAddLevel = FALSE;
					if ( bUsageFilterContainers[USAGEFILTER_AllLevels] == TRUE )
					{
						bShouldAddLevel = TRUE;
					}
					else if ( bUsageFilterContainers[USAGEFILTER_VisibleLevels] == TRUE && FLevelUtils::IsLevelVisible( StreamingLevel ) )
					{
						bShouldAddLevel = TRUE;
					}
					else if ( bUsageFilterContainers[USAGEFILTER_CurrentLevel] == TRUE && StreamingLevel->LoadedLevel == GWorld->CurrentLevel )
					{
						bShouldAddLevel = TRUE;
					}

					if ( bShouldAddLevel == TRUE )
					{
						out_ReferencerContainers.AddItem(StreamingLevel->LoadedLevel);
					}
				}
			}
		}
	}

	GWarn->EndSlowTask();
}


/**
*  Sets the show all types flag, and enables and disables the check list as necessary.
*/
void WxGBLeftContainer::SetShowAllTypes( UBOOL bIn )
{
	bShowAllTypes = bIn;

	if( bShowAllTypes )
	{
		ResourceFilterList->SetForegroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT) );

		// Set the current resource type to the 'All' filter.
		CurrentResourceType = ResourceAllType;
	}
	else
	{
		ResourceFilterList->SetForegroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT) );
	
		RegenerateCustomResourceFilterType();

		// Set the current resource type to the filtered type.
		CurrentResourceType = ResourceFilterType;
	}

	// Rebuild supported object sets.
	ResourceTypeFilterChanged();
}

/**
 * Rebuilds the list of supported objects using the CurrentResourceType and then
 * calls the generic browser update function to update all of the UI elements.
 */
void WxGBLeftContainer::ResourceTypeFilterChanged()
{
	// Build a list of objects that are supported by the current filter.
	TArray<UObject*> SupportedObjects;

	USelection* Selection = GetGBSelection();
	for ( USelection::TObjectIterator It( Selection->ObjectItor() ) ; It ; ++It )
	{
		UObject* Object = *It;
		if ( Object )
		{
			const UBOOL bSupportedByCurrentResourceType = CurrentResourceType->Supports( Object );
			if ( bSupportedByCurrentResourceType )
			{
				SupportedObjects.AddItem( Object );
			}
		}
	}

	// Select only the objects supported by the current filter.
	Selection->DeselectAll();
	for ( INT ObjectIndex = 0 ; ObjectIndex < SupportedObjects.Num() ; ++ObjectIndex )
	{
		UObject* Object = SupportedObjects(ObjectIndex);
		Selection->Select( Object );
	}

	// Refresh the browser.
	GenericBrowser->Update();
}

/**
* Rebuilds the list of classes ResourceFilterType supports using the 
* currently checked items of the ResourceFilterList->
*/
void WxGBLeftContainer::RegenerateCustomResourceFilterType()
{
	// Generate the custom filter list.
	ResourceFilterType->SupportInfo.Empty();

	for(UINT ListIdx = 0; ListIdx < ResourceFilterList->GetCount(); ListIdx++)
	{
		const UBOOL bItemChecked = ResourceFilterList->IsChecked(ListIdx);

		
		if(bItemChecked)
		{
			UGenericBrowserType* BrowserType = ResourceTypes(ListIdx);

			for( INT SupportIdx = 0 ; SupportIdx < BrowserType->SupportInfo.Num() ; ++SupportIdx )
			{
				const FGenericBrowserTypeInfo &TypeInfo = BrowserType->SupportInfo(SupportIdx);
				const UClass* ResourceClass = TypeInfo.Class;
				INT InsertIdx = -1;
				UBOOL bAddItem = TRUE;

				// Loop through all the existing classes checking for duplicate classes and making sure the point of insertion for a class
				// is before any of its parents.
				for(INT ClassIdx = 0; ClassIdx < ResourceFilterType->SupportInfo.Num(); ClassIdx++)
				{
					UClass* PotentialParent = ResourceFilterType->SupportInfo(ClassIdx).Class;

					if(PotentialParent == ResourceClass)
					{
						bAddItem = FALSE;
						break;
					}

					if(ResourceClass->IsChildOf(PotentialParent) == TRUE)
					{
						InsertIdx = ClassIdx;
						break;
					}
				}

				if(bAddItem == TRUE)
				{
					if(InsertIdx == -1)
					{
						ResourceFilterType->SupportInfo.AddItem( TypeInfo );
					}
					else
					{
						ResourceFilterType->SupportInfo.InsertItem( TypeInfo, InsertIdx );
					}
				}
			}
		}
	}
}

void WxGBLeftContainer::OnTreeSelChanged( wxTreeEvent& In )
{
	bTreeViewChanged = TRUE;
}

/**
 * Hack to intercept the tree item selection inside of a SetFocus() call
 */
void WxGBLeftContainer::OnTreeSelChanging( wxTreeEvent& In )
{
	// We are in SetFocus() so don't process any selection changes
	if (TreeCtrl->IsInSetFocus() == TRUE)
	{
		In.Veto();
	}
}

/**
* Event handler for when the user double clicks on a item in the check list.
*/
void WxGBLeftContainer::OnResourceFilterListDoubleClick( wxCommandEvent& In )
{
	const INT TypeIdx = In.GetSelection();

	if( TypeIdx >= 0)
	{
		// Uncheck all items except for the one they double clicked on.
		for( UINT ListIdx = 0; ListIdx < ResourceFilterList->GetCount(); ListIdx++ )
		{
			ResourceFilterList->Check(ListIdx, false);
		}

		ResourceFilterList->DeselectAll();
		ResourceFilterList->Check(TypeIdx, true);
		ResourceFilterList->Select(TypeIdx);

		RegenerateCustomResourceFilterType();
	}

	// Switch to specific filter mode if it is not enabled, this will also
	// update the UI with the new filters.
	if( bShowAllTypes == TRUE)
	{
		SetShowAllTypes(FALSE);
	}
	else
	{
		// Rebuild supported object sets.
		ResourceTypeFilterChanged();
	}
}


/**
* Event handler for when the user changes a checkbox in the resource filter list.
*/
void WxGBLeftContainer::OnResourceFilterListChecked( wxCommandEvent& In )
{
	// If we are checking the item, add it to the support info of the ResourceFilterType.
	// Otherwise, regenerate the supported class list of ResourceFilterType->
	const INT TypeIdx = In.GetSelection();
	const UBOOL bItemChecked = ResourceFilterList->IsChecked(TypeIdx);

	// Set the current list selection to the same index as the check box we just modified to keep users
	// from getting confused about what the selection does.
	ResourceFilterList->Select( TypeIdx );

	RegenerateCustomResourceFilterType();

	// Switch to specific filter mode if it is not enabled, this will also
	// update the UI with the new filters.
	if( bShowAllTypes == TRUE) 
	{
		SetShowAllTypes(FALSE);
	}
	else
	{
		// Rebuild supported object sets.
		ResourceTypeFilterChanged();
	}
}

/**
 * Called when the user double-clicks an item in the usage filter containers list.
 */
void WxGBLeftContainer::OnUsageFilterContainerListDoubleClick( wxCommandEvent& Event )
{
	// skip, for now...
	//@todo ronp
	Event.Skip();
}

/**
 * Called when the user checks/unchecks an item in the usage filter container list.
 */
void WxGBLeftContainer::OnUsageFilterContainerListChecked( wxCommandEvent& Event )
{
	// If we are checking the item, add it to the support info of the ResourceFilterType.
	// Otherwise, regenerate the supported class list of ResourceFilterType->
	const INT TypeIdx = Event.GetSelection();
	const UBOOL bItemChecked = UsageFilterContainerList->IsChecked(TypeIdx);
	if ( bItemChecked == TRUE )
	{
		switch( TypeIdx )
		{
		case USAGEFILTER_Content:
			break;

		case USAGEFILTER_AllLevels:
			if ( bUsageFilterContainers[USAGEFILTER_CurrentLevel] == TRUE )
			{
				bUsageFilterContainers[USAGEFILTER_CurrentLevel] = FALSE;
				UsageFilterContainerList->Check(USAGEFILTER_CurrentLevel, FALSE);
			}

			if ( bUsageFilterContainers[USAGEFILTER_VisibleLevels] == TRUE )
			{
				bUsageFilterContainers[USAGEFILTER_VisibleLevels] = FALSE;
				UsageFilterContainerList->Check(USAGEFILTER_VisibleLevels, FALSE);
			}
			break;

		case USAGEFILTER_CurrentLevel:
		case USAGEFILTER_VisibleLevels:
			if ( bUsageFilterContainers[USAGEFILTER_AllLevels] == TRUE )
			{
				bUsageFilterContainers[USAGEFILTER_AllLevels] = FALSE;
				UsageFilterContainerList->Check(USAGEFILTER_AllLevels, FALSE);
			}
			break;
		}
	}

	bUsageFilterContainers[TypeIdx] = bItemChecked;

	// Set the current list selection to the same index as the check box we just modified to keep users
	// from getting confused about what the selection does.
	UsageFilterContainerList->Select( TypeIdx );
	GenericBrowser->Update();
}

/** 
 * Event handler for when the show all types checked box is changed. 
 */
void WxGBLeftContainer::OnShowAllTypesChanged( wxCommandEvent &In )
{
	SetShowAllTypes( In.IsChecked() );
}

/**
 * Event handler for when the UI system wants to update the ShowAllTypes check box. 
 * This function forces the checkbox to resemble the state of the bShowAllTypes flag.
 */
void WxGBLeftContainer::UI_ShowAllTypes( wxUpdateUIEvent &In )
{
	bool Check = (bShowAllTypes == TRUE);


	In.Check( Check );
	
	wxFont CheckFont = ShowAllCheckBox->GetFont();
	if( Check )
	{
		CheckFont.SetWeight( wxBOLD );
	}
	else
	{
		CheckFont.SetWeight( wxNORMAL );
	}

	ShowAllCheckBox->SetFont(CheckFont);
}


void WxGBLeftContainer::OnBatchProcess( wxCommandEvent& InEvent )
{
	INT EventId = InEvent.GetId();
	if( EventId >= IDMN_PackageContext_BatchProcess_SoundGroups_START && EventId < IDMN_PackageContext_BatchProcess_SoundGroups_END )
	{
        INT		GroupIndex	= EventId - IDMN_PackageContext_BatchProcess_SoundGroups_START;
		FName	GroupName	= NAME_None;

		// Retrieve group name from index.
		UEnum* SoundClassNames = FindObject<UEnum>( NULL, TEXT( "Engine.AudioDevice.ESoundClassName" ) );
		if( SoundClassNames )
		{
			GroupName = SoundClassNames->GetEnum( GroupIndex );
		}

		// Retrieve list of selected packages.
		TArray<UPackage*> SelectedPackages;
		GetSelectedPackages( &SelectedPackages );

		// Disallow export if any packages are cooked.
		if (WxGenericBrowser::DisallowOperationOnCookedPackages(SelectedPackages))
		{
			return;
		}

		// Iterate over all sound cues...
		for( TObjectIterator<USoundCue> It; It; ++It )
		{
			USoundCue*	SoundCue				= *It;
			UBOOL		bIsInSelectedPackage	= FALSE;
			
			// ... and check whether it belongs to any of the selected packages.
			for( INT PackageIndex=0; PackageIndex<SelectedPackages.Num(); PackageIndex++ )
			{
				UPackage* Package = SelectedPackages(PackageIndex);
				if( SoundCue->IsIn( Package ) )
				{
					bIsInSelectedPackage = TRUE;
					break;
				}
			}

			// Set the group to the specified one if we're part of a selected package.
			if( bIsInSelectedPackage )
			{
				SoundCue->SoundGroup = GroupName;
			}
		}
	}
	else if( EventId >= IDMN_PackageContext_BatchProcess_TextureGroups_START && EventId < IDMN_PackageContext_BatchProcess_TextureGroups_END )
	{
		INT GroupIndex = Clamp<INT>(EventId - IDMN_PackageContext_BatchProcess_TextureGroups_START,0,TEXTUREGROUP_MAX-1);

		// Retrieve list of selected packages.
		TArray<UPackage*> SelectedPackages;
		GetSelectedPackages( &SelectedPackages );

		// Disallow export if any packages are cooked.
		if (WxGenericBrowser::DisallowOperationOnCookedPackages(SelectedPackages))
		{
			return;
		}

		// Iterate over all textures....
		for( TObjectIterator<UTexture> It; It; ++It )
		{
			UTexture*	Texture					= *It;
			UBOOL		bIsInSelectedPackage	= FALSE;

			// ... and check whether it belongs to any of the selected packages.
			for( INT PackageIndex=0; PackageIndex<SelectedPackages.Num(); PackageIndex++ )
			{
				UPackage* Package = SelectedPackages(PackageIndex);
				if( Texture->IsIn( Package ) )
				{
					bIsInSelectedPackage = TRUE;
					break;
				}
			}

			// Set the group to the specified one if we're part of a selected package.
			if( bIsInSelectedPackage )
			{
				Texture->LODGroup = GroupIndex;
			}
		}
	}
}

static inline FString GetSoundNameSubString(const TCHAR* InStr)
{
	const FString String( InStr );
	INT LeftMostDigit = -1;
	for ( INT i = String.Len()-1 ; i >= 0 && appIsDigit( String[i] ) ; --i )
	{
		LeftMostDigit = i;
	}

	FString SubString;
	if ( LeftMostDigit > 0 )
	{
		// At least one non-digit char was found.
		SubString = String.Left( LeftMostDigit );
	}
	return SubString;
}

class FWaveList
{
public:
	TArray<USoundNodeWave*> WaveList;
};

/**
 * Clusters sounds into a cue through a random node, based on wave name.
 */
void WxGBLeftContainer::OnClusterSounds(wxCommandEvent& InEvent)
{
	ClusterSounds( FALSE );
}

/**
 * Clusters sounds into a cue through a random node, based on wave name, and an attenuation node.
 */
void WxGBLeftContainer::OnClusterSoundsWithAttenuation(wxCommandEvent& InEvent)
{
	ClusterSounds( TRUE );
}

/**
 * Clusters sounds into a cue through a random node, based on wave name, and optionally an attenuation node.
 */
void WxGBLeftContainer::ClusterSounds(UBOOL bIncludeAttenuationNode)
{
	// Retrieve list of selected packages.
	TArray<UPackage*> SelectedPackages;
	GetSelectedPackages( &SelectedPackages );

	// Disallow export if any packages are cooked.
	if (WxGenericBrowser::DisallowOperationOnCookedPackages(SelectedPackages))
	{
		return;
	}

	// WaveLists(i) is the list of waves in package SelectedPackages(i).
	//TArray< TArray<USoundNodeWave*> > WaveLists;
	TArray<FWaveList> WaveLists;
	WaveLists.AddZeroed( SelectedPackages.Num() );

	// Iterate over all waves and sort by package.
	INT NumWaves = 0;
	for( TObjectIterator<USoundNodeWave> It ; It ; ++It )
	{
		for( INT PackageIndex = 0 ; PackageIndex < SelectedPackages.Num() ; ++PackageIndex )
		{
			if( It->IsIn( SelectedPackages(PackageIndex) ) )
			{
				WaveLists(PackageIndex).WaveList.AddItem( *It );
				++NumWaves;
				break;
			}
		}
	}

	// Abort if no waves were found or the user aborts.
	if ( NumWaves == 0 || !appMsgf( AMT_YesNo, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("Prompt_AboutToCluserSoundsQ")), NumWaves) ) ) )
	{
		return;
	}

	// Prompt the user for a target group name.
	const FString GroupName;
	WxDlgGenericStringEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("NewGroupName"), TEXT("NewGroupName"), TEXT("GroupName") );
	if( Result != wxID_OK )
	{
		return;		
	}

	const FScopedBusyCursor BusyCursor;

	FString NewGroupName = dlg.GetEnteredString();
	NewGroupName = NewGroupName.Replace(TEXT(" "),TEXT("_"));

	INT NumCuesCreated = 0;
	TArray<FString> CuesNotCreated;
	for( INT PackageIndex = 0 ; PackageIndex < SelectedPackages.Num() ; ++PackageIndex )
	{
		const TArray<USoundNodeWave*>& WaveList	= WaveLists(PackageIndex).WaveList;
		if ( WaveList.Num() == 0 )
		{
			continue;
		}

		// Create a package name.
		UPackage* Package						= SelectedPackages(PackageIndex);
		const FString Pkg =
		NewGroupName.Len() > 0 ?
			FString::Printf( TEXT("%s.%s"), *Package->GetName(), *NewGroupName )
		:
			Package->GetName();

		// Create a map of common name prefixes to waves.
		TMap< FString, TArray<USoundNodeWave*> > StringToWaveMap;
		for ( INT WaveIndex = 0 ; WaveIndex < WaveList.Num() ; ++WaveIndex )
		{
			USoundNodeWave* Wave = WaveList(WaveIndex);
			const FString SubString = GetSoundNameSubString( *Wave->GetName() );
			debugf(TEXT("%s --> %s"), *Wave->GetName(), *SubString );

			if ( SubString.Len() > 0 )
			{
				TArray<USoundNodeWave*>* List = StringToWaveMap.Find( SubString );
				if ( List )
				{
					List->AddItem( Wave );
				}
				else
				{
					TArray<USoundNodeWave*> NewList;
					NewList.AddItem( Wave );
					StringToWaveMap.Set( *SubString, NewList );
				}
			}
		}

		// Iterate over name prefixes and create a cue for each, with all nodes sharing
		// that name connected to a SoundNodeRandom in that cue.
		for ( TMap< FString, TArray<USoundNodeWave*> >::TConstIterator It( StringToWaveMap ) ; It ; ++It )
		{
			const TArray<USoundNodeWave*>& List		= It.Value();
			if ( List.Num() < 2 )
			{
				continue;
			}
			const FString& SubString				= It.Key();
			const FString SoundCueName				= FString::Printf( TEXT("%sCue"), *SubString );

			// Check if a new cue with this name can be created;
			UObject* ExistingPackage				= UObject::FindPackage( NULL, *Pkg );
			FString Reason;
			if ( ExistingPackage && !FIsUniqueObjectName( *SoundCueName, ExistingPackage, Reason ) )
			{
				CuesNotCreated.AddItem( FString::Printf( TEXT("%s.%s"), *Pkg, *SoundCueName ) );
				continue;
			}

			// Create sound cue.
			USoundCue* SoundCue = ConstructObject<USoundCue>( USoundCue::StaticClass(), UObject::CreatePackage( NULL, *Pkg ), *SoundCueName, RF_Public|RF_Standalone );
			++NumCuesCreated;

			// Create a random node.
			USoundNodeRandom* RandomNode = ConstructObject<USoundNodeRandom>( USoundNodeRandom::StaticClass(), SoundCue, NAME_None );

			// If this node allows >0 children but by default has zero - create a connector for starters
			if( (RandomNode->GetMaxChildNodes() > 0 || RandomNode->GetMaxChildNodes() == -1) && RandomNode->ChildNodes.Num() == 0 )
			{
				RandomNode->CreateStartingConnectors();
			}

			// Create new editor data struct and add to map in SoundCue.
			FSoundNodeEditorData RandomEdData;
			appMemset(&RandomEdData, 0, sizeof(FSoundNodeEditorData));
			RandomEdData.NodePosX = 200;
			RandomEdData.NodePosY = -150;
			SoundCue->EditorData.Set(RandomNode, RandomEdData);

			// Connect the waves.
			for ( INT WaveIndex = 0 ; WaveIndex < List.Num() ; ++WaveIndex )
			{
				USoundNodeWave* Wave = List(WaveIndex);

				// Create new editor data struct and add to map in SoundCue.
				FSoundNodeEditorData WaveEdData;
				appMemset(&WaveEdData, 0, sizeof(FSoundNodeEditorData));
				WaveEdData.NodePosX = 350;
				WaveEdData.NodePosY = -200 + WaveIndex * 75;
				SoundCue->EditorData.Set( Wave, WaveEdData );

				// Link the random node to the wave.
				while ( WaveIndex >= RandomNode->ChildNodes.Num() )
				{
					RandomNode->InsertChildNode( RandomNode->ChildNodes.Num() );
				}
				RandomNode->ChildNodes(WaveIndex) = Wave;
			}

			if ( bIncludeAttenuationNode )
			{
				USoundNode* AttenuationNode = ConstructObject<USoundNode>( USoundNodeAttenuation::StaticClass(), SoundCue, NAME_None );

				// If this node allows >0 children but by default has zero - create a connector for starters
				if( (AttenuationNode->GetMaxChildNodes() > 0 || AttenuationNode->GetMaxChildNodes() == -1) && AttenuationNode->ChildNodes.Num() == 0 )
				{
					AttenuationNode->CreateStartingConnectors();
				}

				// Create new editor data struct and add to map in SoundCue.
				FSoundNodeEditorData AttenuationEdData;
				appMemset(&AttenuationEdData, 0, sizeof(FSoundNodeEditorData));
				AttenuationEdData.NodePosX = 50;
				AttenuationEdData.NodePosY = -150;
				SoundCue->EditorData.Set(AttenuationNode, AttenuationEdData);

				// Link the attenuation node to the wave.
				AttenuationNode->ChildNodes(0) = RandomNode;

				// Link the attenuation node to root.
				SoundCue->FirstNode = AttenuationNode;
			}
			else
			{
				// Link the wave to root.
				SoundCue->FirstNode = RandomNode;
			}
		}
	}

	appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("CreatedNCues")), NumCuesCreated) ) );
	if ( CuesNotCreated.Num() > 0 )
	{
		FString CuesNotCreatedMsg( *LocalizeUnrealEd(TEXT("CouldNotCreateTheFollowingCuesNamesInUse")) );
		for ( INT i = 0 ; i < CuesNotCreated.Num() ; ++i )
		{
			CuesNotCreatedMsg += FString::Printf(LINE_TERMINATOR TEXT("   %s"), *CuesNotCreated(i) );
		}
		appMsgf( AMT_OK, *CuesNotCreatedMsg );
	}
	GenericBrowser->Update();
}


/**
* Event handler for package error checking via the package tree view.
 */
void WxGBLeftContainer::OnCheckPackageForErrors( wxCommandEvent& InEvent )
{
	// Retrieve list of selected packages.
	TArray<UPackage*> SelectedPackages;
	GetSelectedPackages( &SelectedPackages );

	TArray<UObject*> Objects;
	TArray<UObject*> TrashcanObjects;
	GEditor->CheckForTrashcanReferences( SelectedPackages, Objects, TrashcanObjects );

	check( Objects.Num() == TrashcanObjects.Num() );

	// Output to dialog.
	FString OutputString;
	if ( Objects.Num() > 0 )
	{
		for ( INT ObjectIndex = 0 ; ObjectIndex < Objects.Num() ; ++ObjectIndex )
		{
			const UObject* Object = Objects(ObjectIndex);
			const UObject* TrashcanObject = TrashcanObjects(ObjectIndex);

			OutputString += FString::Printf( TEXT("%s -> %s  :  %s  ->  %s%s"),
				*Object->GetOutermost()->GetName(), *TrashcanObject->GetOutermost()->GetName(), *Object->GetFullName(), *TrashcanObject->GetFullName(), LINE_TERMINATOR );
		}
	}
	else
	{
		OutputString = TEXT("No package errors found.");
	}
	appMsgf( AMT_OK, *OutputString );
}

/**
 * Set the package's folder name from user dialog box
 */
void WxGBLeftContainer::OnSetFolder( wxCommandEvent& InEvent )
{
	// Generate a list of unique packages
	TArray<UPackage *> selPackages;
	GetSelectedPackages (&selPackages);

	// Disallow export if any packages are cooked.
	if (WxGenericBrowser::DisallowOperationOnCookedPackages(selPackages))
	{
		return;
	}

	TArray<UPackage*> packages;

	for (INT i = 0 ; i < selPackages.Num() ; ++i)
	{
		UPackage *pk = selPackages (i);
		packages.AddUniqueItem ( pk->GetOutermost() ? pk->GetOutermost() : pk );
	}

	if (packages.Num())
	{
		FString folder = packages(0)->GetFolderName().ToString();

		UBOOL more;
		do
		{
			more = false;

			WxDlgGenericStringEntry dlg;

			INT Result = dlg.ShowModal( TEXT("SetFolder"), TEXT(""), *folder);

			if (Result == wxID_OK)
			{
				folder = dlg.GetEnteredString();
				folder.Trim();

				FString reason;

				if ( FIsValidGroupName (*folder, reason) == TRUE)
				{
					wxTreeItemId *FolderPtr = FolderMap.Find(*folder);
					wxTreeItemId FolderId;

					if(!FolderPtr)
					{
						FolderId = TreeCtrl->AppendItem(TreeCtrl->GetRootItem(), *folder, GBTCI_FolderClosed, -1);

						FolderMap.Set(*folder, FolderId);
					}
					else
					{
						FolderId = *FolderPtr;
					}

					TreeCtrl->Freeze();
					for (INT i = 0; i < packages.Num(); i++)
					{
						UPackage *pk = packages (i);
						pk->SetFolderName (*folder);
						pk->SetDirtyFlag (TRUE);

						wxTreeItemId OldBranch = *PackageMap.Find(pk);

						CopyPackageTreeBranch(FolderId, OldBranch);
						TreeCtrl->Delete(OldBranch);
					}
					TreeCtrl->Thaw();

					GenericBrowser->Update();
				}
				else
				{
					appMsgf (AMT_OK, *reason);
					more = TRUE;
				}
			}
		}
		while (more);	// Try again?
	}
}

/**
 * Remove the package from the current folder and put it in the root
 */
void WxGBLeftContainer::OnRemoveFromFolder( wxCommandEvent& InEvent )
{
	// Generate a list of unique packages
	TArray<UPackage *> selPackages;
	GetSelectedPackages (&selPackages);

	if (WxGenericBrowser::DisallowOperationOnCookedPackages(selPackages))
	{
		return;
	}

	TArray<UPackage*> packages;

	for (INT i = 0 ; i < selPackages.Num() ; ++i)
	{
		UPackage *pk = selPackages (i);
		packages.AddUniqueItem ( pk->GetOutermost() ? pk->GetOutermost() : pk );
	}

	// There is no destination folder; use an empty string for the name
	const FString folder(TEXT(""));

	TreeCtrl->Freeze();
	for (INT i = 0; i < packages.Num(); i++)
	{
		UPackage *pk = packages (i);
		pk->SetFolderName (*folder);
		pk->SetDirtyFlag (TRUE);

		wxTreeItemId OldBranch = *PackageMap.Find(pk);

		CopyPackageTreeBranch(TreeCtrl->GetRootItem(), OldBranch);
		TreeCtrl->Delete(OldBranch);
	}
	TreeCtrl->Thaw();

	GenericBrowser->Update();
}

void WxGBLeftContainer::Serialize(FArchive& Ar)
{
	Ar << ResourceTypes << CurrentResourceType << ResourceFilterType << ResourceAllType;
}

/*-----------------------------------------------------------------------------
	WxGBRightContainer.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxGBRightContainer, wxPanel )
	EVT_SIZE( WxGBRightContainer::OnSize )
	EVT_LIST_ITEM_SELECTED( ID_LIST_VIEW, WxGBRightContainer::OnSelectionChanged_Select )
	EVT_LIST_ITEM_DESELECTED( ID_LIST_VIEW, WxGBRightContainer::OnSelectionChanged_Deselect )
	EVT_LIST_ITEM_RIGHT_CLICK( ID_LIST_VIEW, WxGBRightContainer::OnListViewRightClick )
	EVT_LIST_ITEM_ACTIVATED( ID_LIST_VIEW, WxGBRightContainer::OnListItemActivated )
	EVT_LIST_COL_CLICK( ID_LIST_VIEW, WxGBRightContainer::OnListColumnClicked )
	EVT_LIST_ITEM_ACTIVATED( ID_REFERENCER_LIST, WxGBRightContainer::OnReferencerListViewActivated )
	EVT_LIST_COL_CLICK( ID_REFERENCER_LIST, WxGBRightContainer::OnReferencerListColumnClicked )
	EVT_MENU( IDM_SEARCH, WxGBRightContainer::OnSearch )
	EVT_MENU( IDM_GROUPBYCLASS, WxGBRightContainer::OnGroupByClass )
	EVT_MENU( IDM_USAGEFILTER, WxGBRightContainer::OnUsageFilter )
	EVT_MENU( IDM_PSYSREALTIME, WxGBRightContainer::OnPSysRealTime )
	EVT_MENU_RANGE( IDM_VIEW_START, IDM_VIEW_END, WxGBRightContainer::OnViewMode )
	EVT_UPDATE_UI_RANGE( IDM_VIEW_START, IDM_VIEW_END, WxGBRightContainer::UI_ViewMode )
	EVT_MENU_RANGE( ID_PRIMTYPE_START, ID_PRIMTYPE_END, WxGBRightContainer::OnPrimType )
	EVT_UPDATE_UI_RANGE( ID_PRIMTYPE_START, ID_PRIMTYPE_END, WxGBRightContainer::UI_PrimType )
	EVT_UPDATE_UI( IDM_GROUPBYCLASS, WxGBRightContainer::UI_GroupByClass )
	EVT_UPDATE_UI( IDM_USAGEFILTER, WxGBRightContainer::UI_UsageFilter )
	EVT_UPDATE_UI( IDM_PSYSREALTIME, WxGBRightContainer::UI_PSysRealTime )
	EVT_UPDATE_UI( ID_SelectionInfo, WxGBRightContainer::UI_SelectionInfo )
	EVT_COMBOBOX( ID_ZOOM_FACTOR, WxGBRightContainer::OnZoomFactorSelChange )
	EVT_TEXT( ID_NAME_FILTER, WxGBRightContainer::OnNameFilterChanged )
	EVT_MENU( IDM_ATOMIC_RESOURCE_REIMPORT, WxGBRightContainer::OnAtomicResourceReimport )
	EVT_SCROLL( WxGBRightContainer::OnScroll )
END_EVENT_TABLE()

WxGBRightContainer::WxGBRightContainer( wxWindow* InParent )
	: wxPanel( InParent, -1 ),
	GenericBrowser( NULL ),
	ViewMode( RVM_Thumbnail ),
	bGroupByClass( 0 ),
	bIsScrolling( 0 ),
	bSuppressDraw( 0 ),
	ThumbnailPrimType( TPT_Sphere ),
	SplitterPosV( STD_SPLITTER_SZ ),
	SplitterPosH( STD_SPLITTER_SZ ),
	ReferencerNameColumnWidth( 200 ),
	ReferencerCountColumnWidth( 100 ),
	NameFilter( TEXT("") ),
	bDraggingObject(FALSE),
	SuppressUpdateMutex(0)
{
	ToolBar = new WxGBViewToolBar( (wxWindow*)this, -1 );

	SplitterWindowH = new wxSplitterWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxSP_BORDER );
	{
		SplitterWindowV = new wxSplitterWindow( SplitterWindowH, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_BORDER );
		{
			ViewportHolder = new WxViewportHolder( (wxWindow*)SplitterWindowV, -1, 1 );
			ListView = new wxListView( SplitterWindowV, ID_LIST_VIEW, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_ALIGN_LEFT | wxLC_SORT_ASCENDING );
		}

		ReferencersList = new wxListView( SplitterWindowH, ID_REFERENCER_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_ALIGN_LEFT | wxLC_SORT_ASCENDING );
	}
	SelectionLabel = new wxStaticText( this, ID_SelectionInfo, TEXT("") );

	SplitterWindowH->SplitHorizontally( SplitterWindowV, ReferencersList, STD_SPLITTER_SZ );
	ReferencersList->InsertColumn( COLHEADER_Name, *LocalizeUnrealEd(TEXT("PrimitiveStatsBrowser_Name")), wxLIST_FORMAT_LEFT, ReferencerNameColumnWidth);
	ReferencersList->InsertColumn( COLHEADER_Count, *LocalizeUnrealEd(TEXT("PrimitiveStatsBrowser_Count")), wxLIST_FORMAT_LEFT, ReferencerCountColumnWidth);
	ReferencersList->InsertColumn( COLHEADER_Class, *LocalizeUnrealEd(TEXT("Class")) ); 

	SplitterWindowV->SplitVertically(
		ListView,
		ViewportHolder,
		STD_SPLITTER_SZ );

	ListView->InsertColumn( GB_LV_COLUMN_Object, *LocalizeUnrealEd("Object"), wxLIST_FORMAT_LEFT, 150 );
	ListView->InsertColumn( GB_LV_COLUMN_Group, *LocalizeUnrealEd("Group"), wxLIST_FORMAT_LEFT, 150 );
	ListView->InsertColumn( GB_LV_COLUMN_ResourceSize, *LocalizeUnrealEd("ResourceSizeK"), wxLIST_FORMAT_LEFT, 200 );
	ListView->InsertColumn( GB_LV_COLUMN_Info0, *LocalizeUnrealEd("Info"), wxLIST_FORMAT_LEFT, 200 );
	for( INT i = GB_LV_COLUMN_Info1; i < GB_LV_COLUMN_Count; i++ )
	{
		ListView->InsertColumn( i, TEXT(""), wxLIST_FORMAT_LEFT, 150 );
	}

	Viewport = GEngine->Client->CreateWindowChildViewport( this, (HWND)ViewportHolder->GetHandle() );
	Viewport->CaptureJoystickInput(false);

	ViewportHolder->SetViewport( Viewport );

	SearchDialog = new WxDlgGenericBrowserSearch();
}

WxGBRightContainer::~WxGBRightContainer()
{
	SaveSettings();

	if(Viewport)
	{
		GEngine->Client->CloseViewport(Viewport);
		Viewport = NULL;
	}
}

/**
 * Called once the generic browser is fully set up as trying to set the view mode before 
 * then results in nothing happening.
 */

void WxGBRightContainer::LoadSettings()
{
	GConfig->GetInt( TEXT("GenericBrowser"), TEXT("ViewMode"), (INT&)ViewMode, GEditorUserSettingsIni );
	GConfig->GetInt( TEXT("GenericBrowser"), TEXT("ThumbnailPrimitiveType"), (INT&)ThumbnailPrimType, GEditorUserSettingsIni );
	GConfig->GetBool( TEXT("GenericBrowser"), TEXT("GroupByClass"), bGroupByClass, GEditorUserSettingsIni );
	GConfig->GetInt( TEXT("GenericBrowser"), TEXT("SplitterPosV"), SplitterPosV, GEditorUserSettingsIni );
	GConfig->GetInt( TEXT("GenericBrowser"), TEXT("SplitterPosH"), SplitterPosH, GEditorUserSettingsIni );
	GConfig->GetInt( TEXT("GenericBrowser"), TEXT("ReferencerNameColumnWidth"), ReferencerNameColumnWidth, GEditorUserSettingsIni );
	GConfig->GetInt( TEXT("GenericBrowser"), TEXT("ReferencerCountColumnWidth"), ReferencerCountColumnWidth, GEditorUserSettingsIni );

	for( INT ColIndex = 0; ColIndex < GB_LV_COLUMN_Count; ColIndex++ )
	{
		INT ColumnWidth = 100;

		FString Key = FString::Printf( TEXT( "ColumnWidth[%d]" ), ColIndex );
		GConfig->GetInt( TEXT( "GenericBrowser" ), *Key, ( INT& )ColumnWidth, GEditorUserSettingsIni );
		ListView->SetColumnWidth( ColIndex, ColumnWidth );
	}

	UBOOL bTempPSysRealTime = TRUE;
	GConfig->GetBool( TEXT("GenericBrowser"), TEXT("PSysRealTime"), bTempPSysRealTime, GEditorUserSettingsIni );
	GUnrealEd->GetThumbnailManager()->bPSysRealTime = bTempPSysRealTime;

	// Load View Size Combo.
	FString ViewSizeString;
	GConfig->GetString ( TEXT("GenericBrowser"), TEXT("ViewSize"), ViewSizeString, GEditorUserSettingsIni );

	wxCommandEvent Event;
	Event.SetString( *ViewSizeString );
	OnZoomFactorSelChange( Event );

	ReferencersList->SetColumnWidth( 0, ReferencerNameColumnWidth );
	ReferencersList->SetColumnWidth( 1, ReferencerCountColumnWidth );
	SetUsageFilter( GenericBrowser->IsUsageFilterEnabled() );

	SetViewMode( ViewMode );

	wxSizeEvent DummyEvent;
	OnSize( DummyEvent );

	Update();
}

/**
 * Writes out values we want to save to the INI file.
 */

void WxGBRightContainer::SaveSettings()
{
	SplitterPosV = SplitterWindowV->GetSashPosition();
	SplitterPosH = SplitterWindowH->GetSashPosition();

	ReferencerNameColumnWidth = ReferencersList->GetColumnWidth(0);
	ReferencerCountColumnWidth = ReferencersList->GetColumnWidth(1);

	GConfig->SetInt( TEXT("GenericBrowser"), TEXT("ViewMode"), ViewMode, GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("GenericBrowser"), TEXT("ThumbnailPrimitiveType"), ThumbnailPrimType, GEditorUserSettingsIni );
	GConfig->SetBool( TEXT("GenericBrowser"), TEXT("GroupByClass"), bGroupByClass, GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("GenericBrowser"), TEXT("SplitterPosV"), SplitterPosV, GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("GenericBrowser"), TEXT("SplitterPosH"), SplitterPosH, GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("GenericBrowser"), TEXT("ReferencerNameColumnWidth"), ReferencerNameColumnWidth, GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("GenericBrowser"), TEXT("ReferencerCountColumnWidth"), ReferencerCountColumnWidth, GEditorUserSettingsIni );

	for( INT ColIndex = 0; ColIndex < GB_LV_COLUMN_Count; ColIndex++ )
	{
		FString Key = FString::Printf( TEXT( "ColumnWidth[%d]" ), ColIndex );
		GConfig->SetInt( TEXT( "GenericBrowser" ), *Key, ListView->GetColumnWidth( ColIndex ), GEditorUserSettingsIni );
	}

	GConfig->SetBool( TEXT("GenericBrowser"), TEXT("PSysRealTime"), GUnrealEd->GetThumbnailManager()->bPSysRealTime, GEditorUserSettingsIni );

	GConfig->SetString( TEXT("GenericBrowser"), TEXT("ViewSize"), (const TCHAR*)ToolBar->ViewCB->GetValue(), GEditorUserSettingsIni );
}

int wxCALLBACK WxRightContainerListSort( long InItem1, long InItem2, long InSortData )
{
	UObject* A = (UObject*)InItem1;
	UObject* B = (UObject*)InItem2;
	FListViewSortOptions* so = (FListViewSortOptions*)InSortData;

	FString CompA;
	FString CompB;
	INT IntA;
	INT IntB;
	FLOAT FloatA;
	FLOAT FloatB;

	// Generate a string to run the compares against for each object based on
	// the current column.
	UBOOL bDoStringCompare = FALSE;
	int Ret = 0;
	switch( so->Column )
	{
		case WxGBRightContainer::GB_LV_COLUMN_Object:			// Object name
			CompA = A->GetName();
			CompB = B->GetName();
			Ret = appStricmp( *CompA, *CompB );
			break;

		case WxGBRightContainer::GB_LV_COLUMN_Group:			// Group Name
			CompA = A->GetFullGroupName( TRUE );
			CompB = B->GetFullGroupName( TRUE );
			Ret = appStricmp( *CompA, *CompB );
			break;

		case WxGBRightContainer::GB_LV_COLUMN_ResourceSize:		// Resource size.
			IntA = A->GetResourceSize();
			IntB = B->GetResourceSize();
			Ret = ( IntA < IntB ) ? -1 : ( IntA > IntB ? 1 : 0 );
			break;

		case WxGBRightContainer::GB_LV_COLUMN_Info0:			// Description
		case WxGBRightContainer::GB_LV_COLUMN_Info1:			// Description
		case WxGBRightContainer::GB_LV_COLUMN_Info2:			// Description
		case WxGBRightContainer::GB_LV_COLUMN_Info3:			// Description
		case WxGBRightContainer::GB_LV_COLUMN_Info4:			// Description
		case WxGBRightContainer::GB_LV_COLUMN_Info5:			// Description
		case WxGBRightContainer::GB_LV_COLUMN_Info6:			// Description
		case WxGBRightContainer::GB_LV_COLUMN_Info7:			// Description
			CompA = A->GetDetailedDescription( so->Column - WxGBRightContainer::GB_LV_COLUMN_Info0 );
			CompB = B->GetDetailedDescription( so->Column - WxGBRightContainer::GB_LV_COLUMN_Info0 );
			FloatA = appAtof( *CompA );
			FloatB = appAtof( *CompB );
			if( FloatA != 0.0f && FloatB != 0.0f )
			{
				Ret = ( FloatA < FloatB ) ? -1 : ( FloatA > FloatB ? 1 : 0 );
			}
			else
			{
				Ret = appStricmp( *CompA, *CompB );
			}
			break;

		default:
			check( 0 );	// Invalid column!
			break;
	}

	// If we are sorting backwards, invert the string comparison result.
	if( !so->bSortAscending )
	{
		Ret *= -1;
	}

	return Ret;
}

int wxCALLBACK WxReferencerListSort( long InItem1, long InItem2, long InSortData )
{
	UObject* A = (UObject*)InItem1;
	UObject* B = (UObject*)InItem2;
	FListViewSortOptions* so = (FListViewSortOptions*)InSortData;

	FString CompA;
	FString CompB;

	// Generate a string to run the compares against for each object based on
	// the current column.
	UBOOL bDoStringCompare = FALSE;
	int Ret = 0;
	switch( so->Column )
	{
	case WxGBRightContainer::COLHEADER_Name:			// Object path name
		CompA = A->GetPathName();
		CompB = B->GetPathName();
		bDoStringCompare = TRUE;
		break;

	case WxGBRightContainer::COLHEADER_Count:			// Number of references
		// can't sort by number of references
		break;

	case WxGBRightContainer::COLHEADER_Class:			// Class Name
		CompA = A->GetClass()->GetName();
		CompB = B->GetClass()->GetName();
		bDoStringCompare = TRUE;
		break;

	default:
		check(0);	// Invalid column!
		break;
	}


	if ( bDoStringCompare )
	{
		Ret = appStricmp( *CompA, *CompB );
	}

	// If we are sorting backwards, invert the string comparison result.
	if( !so->bSortAscending )
	{
		Ret *= -1;
	}

	return Ret;
}

// Fills the list view with items relating to the selected packages on the left side.
void WxGBRightContainer::UpdateList()
{
	ListView->Freeze();
	ListView->DeleteAllItems();

	// See what is selected on the left side.
	long SelectItem = -1;
	TArray<UPackage*> SelectedPackages;
	GenericBrowser->LeftContainer->GetSelectedPackages( &SelectedPackages );

	VisibleObjects.Empty();
	GetObjectsInPackages( SelectedPackages, VisibleObjects );

	// Loop through all visible objects and add them to the list.
	for( INT x = 0; x < VisibleObjects.Num(); ++x )
	{
		// WDM : should grab a bitmap to use for each different kind of resource.  For now, they'll all look the same.
		const INT item = ListView->InsertItem( 0, *VisibleObjects( x )->GetName(), GBTCI_Resource );

		// Create a string for the resource size.
		const INT ResourceSize = VisibleObjects( x )->GetResourceSize();
		FString ResourceSizeString;
		if( ResourceSize > 0 )
		{
			ResourceSizeString = FString::Printf( TEXT( "%.2f" ), ( ( FLOAT )ResourceSize ) / 1024.f );
		}

		// Add the columns.
		ListView->SetItem( item, 1, *VisibleObjects( x )->GetFullGroupName( TRUE ) );
		ListView->SetItem( item, 2, *ResourceSizeString );
		for( INT i = GB_LV_COLUMN_Info0; i < GB_LV_COLUMN_Count; i++ )
		{
			ListView->SetItem( item, i, *VisibleObjects( x )->GetDetailedDescription( i - GB_LV_COLUMN_Info0 ) );
		}
		ListView->SetItemData( item, ( long )VisibleObjects( x ) );

		// Select the list item if the item data's selection flag is set
		if( GetGBSelection()->IsSelected( VisibleObjects( x ) ) )
		{	
			ListView->Select( item );
			SelectItem = item;
		}
	}

	// If we have any items selected, jump to the first selected item.
	if(SelectItem > -1)
	{
		ListView->EnsureVisible(SelectItem);
	}

	ListView->SortItems( WxRightContainerListSort, (LONG)&SearchOptions );
	ListView->Thaw();
	Viewport->Invalidate();
}

/**
 * This updates the ReferencersList with the list of objects referencing the resources currently selected in the GB viewport or list.
 */
void WxGBRightContainer::UpdateReferencers( UObject* SelectedObject )
{
	TArray<UObject*> Referencers;
	ReferencersList->Freeze();
	ReferencersList->DeleteAllItems();

	if ( SelectedObject != NULL && GenericBrowser != NULL && GenericBrowser->IsUsageFilterEnabled() )
	{
		const FScopedBusyCursor BusyCursor;
		GWarn->BeginSlowTask(*LocalizeUnrealEd(TEXT("ReferenceGraphs")), TRUE);

		TArray<UObject*> RootSet;

		// first, build an initial base root set containing all selected levels, script, etc.
		GenericBrowser->GetReferencerContainers(RootSet);

		// now build an optimized root set of all objects contained in the loaded packages
		GenericBrowser->BuildDependencyRootSet(Referencers, &RootSet);

		// clear all mark flags
		for ( FObjectIterator It; It; ++It )
		{
			It->ClearFlags(RF_TagExp|RF_TagImp);
		}

		for ( INT ReferencerIndex = 0; ReferencerIndex < Referencers.Num(); ReferencerIndex++ )
		{
			UObject* PotentialReferencer = Referencers(ReferencerIndex);

			if ( PotentialReferencer == SelectedObject )
			{
				// skip if this is the object we're attempting to find references to
				continue;
			}

			FArchiveFindCulprit ArFind(SelectedObject,PotentialReferencer,FALSE);
			TArray<UProperty*> ReferencingProperties;

			INT Count = ArFind.GetCount(ReferencingProperties);
			if ( Count > 0 )
			{
				// List columns are "Name", "Reference Count", "Class"
				INT item = ReferencersList->InsertItem( COLHEADER_Name, *PotentialReferencer->GetPathName() );

				ReferencersList->SetItem( item, COLHEADER_Count, *appItoa(Count) );
				ReferencersList->SetItem( item, COLHEADER_Class, *PotentialReferencer->GetClass()->GetName() );
				ReferencersList->SetItemData( item, (long)PotentialReferencer );
			}
		}

		ReferencersList->SortItems( WxReferencerListSort, (LONG)&ReferencerOptions );
		GWarn->EndSlowTask();
	}
	ReferencersList->Thaw();
}

void WxGBRightContainer::Update()
{
	if ( IsShown() && SuppressUpdateMutex == 0 )
	{
		UpdateUI();

		// Always update the list view e.g. so that UI_SelectionInfo reports the right numbers.
		UpdateList();

		// If the 'In Use Only' flag is enabled, update referencers so that e.g. actor deletion,
		// which triggers a GB update, will also result in the clearing of any potentially stale
		// actor references held by the ReferencersList.
		if ( GenericBrowser->IsUsageFilterEnabled() )
		{
			UpdateReferencers(NULL);
		}

		// No need to update thumbnails if we're in a purely list view.
		if ( ViewMode != RVM_List )
		{
			Viewport->Invalidate();
		}
		bNeedsUpdate = TRUE;
	}
}

/**
 * Called by WxGenericBrowser::Update() when the browser is not shown to clear out unwanted state.
 */
void WxGBRightContainer::UpdateEmpty()
{
	if ( ReferencersList != NULL )
	{
		ReferencersList->DeleteAllItems();
	}
}

void WxGBRightContainer::UpdateUI()
{
	// Make sure we have atleast 1 supported class.
	const UBOOL bHasSupportInfo = (GenericBrowser->LeftContainer->CurrentResourceType->SupportInfo.Num() > 0);

	if( !bHasSupportInfo )
	{
		return;
	}

	WxBrowserData* bd = GenericBrowser->FindBrowserData( GenericBrowser->LeftContainer->CurrentResourceType->SupportInfo(0).Class );
	check( bd );

	FString Wk;
	if( !bd->GetFixedSz() )
		Wk = FString::Printf( TEXT("%d%%"), (INT)(bd->GetZoomFactor()*100) );
	else
		Wk = FString::Printf( TEXT("%dx%d"), bd->GetFixedSz(), bd->GetFixedSz() );

	ToolBar->ViewCB->Select( ToolBar->ViewCB->FindString( *Wk ) );
	// Update the primitive setting with the last used one
	switch (bd->GetPrimitive())
	{
		case TPT_Sphere:
			ToolBar->FindById(ID_PRIMTYPE_SPHERE)->SetToggle(TRUE);
			break;
		case TPT_Cube:
			ToolBar->FindById(ID_PRIMTYPE_CUBE)->SetToggle(TRUE);
			break;
		case TPT_Cylinder:
			ToolBar->FindById(ID_PRIMTYPE_CYLINDER)->SetToggle(TRUE);
			break;
		case TPT_Plane:
			ToolBar->FindById(ID_PRIMTYPE_PLANE)->SetToggle(TRUE);
			break;
	}
	ThumbnailPrimType = bd->GetPrimitive();
}

void WxGBRightContainer::OnSize( wxSizeEvent& In )
{
	const wxRect rc = GetClientRect();
	const wxRect rcT = ToolBar->GetClientRect();
	const wxRect rcL = SelectionLabel->GetClientRect();

	ToolBar->SetSize( 0, 0, rc.GetWidth(), rcT.GetHeight() );
	SplitterWindowH->SetSize( 0, rcT.GetHeight(), rc.GetWidth(), rc.GetHeight() - rcT.GetHeight() - rcL.GetHeight() );
	SelectionLabel->SetSize( 0, rc.GetHeight() - rcL.GetHeight(), rc.GetWidth(), rcL.GetHeight() );
}

void WxGBRightContainer::OnViewMode( wxCommandEvent& In )
{
	switch( In.GetId() )
	{
		case ID_VIEW_LIST:
			SetViewMode( RVM_List );
			break;

		case ID_VIEW_PREVIEW:
			SetViewMode( RVM_Preview );
			break;

		case ID_VIEW_THUMBNAIL:
			SplitterPosV = SplitterWindowV->GetSashPosition();
			SetViewMode( RVM_Thumbnail );
			break;
	}

	wxSizeEvent DummyEvent;
	OnSize( DummyEvent );
	Update();
}

/** Called when the user presses one of the "usage filter" buttons */
void WxGBRightContainer::OnUsageFilter( wxCommandEvent& Event )
{
	if ( Event.IsChecked() == FALSE )
	{
		// if the user unchecked the Reference filter button, store the current position of the sash
		SplitterPosH = SplitterWindowH->GetSashPosition();
	}

	Event.Skip();
}

void WxGBRightContainer::OnSearch( wxCommandEvent& In )
{
	SearchDialog->Show();
	SearchDialog->UpdateResults();
}

void WxGBRightContainer::OnGroupByClass( wxCommandEvent& In )
{
	bGroupByClass = !bGroupByClass;
	Update();
}

void WxGBRightContainer::OnPSysRealTime( wxCommandEvent& In )
{
	UThumbnailManager* ThumbMngr = GUnrealEd->GetThumbnailManager();
	if (ThumbMngr)
	{
		ThumbMngr->bPSysRealTime = !ThumbMngr->bPSysRealTime;
		Update();
	}
}

void WxGBRightContainer::OnPrimType( wxCommandEvent& In )
{
	switch( In.GetId() )
	{
		case ID_PRIMTYPE_SPHERE:
			ThumbnailPrimType = TPT_Sphere;
			break;
		case ID_PRIMTYPE_CUBE:
			ThumbnailPrimType = TPT_Cube;
			break;
		case ID_PRIMTYPE_CYLINDER:
			ThumbnailPrimType = TPT_Cylinder;
			break;
		case ID_PRIMTYPE_PLANE:
			ThumbnailPrimType = TPT_Plane;
			break;
	}

	// Make sure we have at least 1 supported class before updating the browser data struct.
	const UBOOL bHasSupportInfo = (GenericBrowser->LeftContainer->CurrentResourceType->SupportInfo.Num() > 0);

	if( bHasSupportInfo )
	{
		WxBrowserData* bd = GenericBrowser->FindBrowserData( GenericBrowser->LeftContainer->CurrentResourceType->SupportInfo(0).Class );
		bd->SetPrimitive(ThumbnailPrimType);
	}

	// If this is also the default, then update that setting too
	if (GenericBrowser->IsDefaultResourceSelected())
	{
		WxBrowserData::SetDefaultPrimitive(ThumbnailPrimType);
	}

	Update();
}

void WxGBRightContainer::OnSelectionChanged_Select( wxListEvent& In )
{
	UObject* obj = (UObject*)In.GetData();

	if(obj != NULL && obj->IsSelected() == FALSE)
	{
		GetGBSelection()->Select( obj );
		UpdateReferencers(obj);
		bNeedsUpdate = TRUE;
		Viewport->Invalidate();
	}
}

void WxGBRightContainer::OnSelectionChanged_Deselect( wxListEvent& In )
{
	UObject* obj = (UObject*)In.GetData();

	UTexture* Texture = Cast<UTexture>(obj);
	if (Texture)
	{
		if (Texture == GEditor->StreamingBoundsTexture)
		{
			GEditor->SetStreamingBoundsTexture(NULL);
		}
	}
	GetGBSelection()->Deselect( obj );
	UpdateReferencers(NULL);
	bNeedsUpdate = TRUE;
	Viewport->Invalidate();
}

void WxGBRightContainer::OnListViewRightClick( wxListEvent& In )
{
	const long FocusedItem = ListView->GetFocusedItem();
	UObject* obj = (UObject*)( ListView->GetItemData( FocusedItem ) );

	if( obj )
	{
		// Sanity checking to make sure the GB is synced up properly.
		check(obj->IsSelected());

		ListView->Select(FocusedItem);
		ShowContextMenu();
	}
}

void WxGBRightContainer::OnListItemActivated( wxListEvent& In )
{
	// Sanity checking to make sure the GB is synced up properly.
	GenericBrowser->LeftContainer->CurrentResourceType->DoubleClick();
}

void WxGBRightContainer::OnListColumnClicked( wxListEvent& In )
{
	int Column = In.GetColumn();

	if( Column > -1 )
	{
		if( Column == SearchOptions.Column )
		{
			// Clicking on the same column will flip the sort order

			SearchOptions.bSortAscending = !SearchOptions.bSortAscending;
		}
		else
		{
			// Clicking on a new column will set that column as current and reset the sort order.

			SearchOptions.Column = In.GetColumn();
			SearchOptions.bSortAscending = 1;
		}
	}

	UpdateList();
}

/** called when the user double-clicks an entry in the "referencer" list */
void WxGBRightContainer::OnReferencerListViewActivated( wxListEvent& Event )
{
	UBOOL bSuccess = FALSE;

	long ItemData = Event.GetData();
	UObject* Referencer = (UObject*)ItemData;
	if ( Referencer != NULL )
	{
		// first, determine whether referencer is contained in a level or a content package
		if ( Referencer->GetOutermost()->ContainsMap() )
		{
			FScopedTransaction Transaction( *LocalizeUnrealEd("ActorSearchGoto") );

			// find the Actor that contains this referencer
			for ( UObject* CheckOuter = Referencer; CheckOuter != NULL; CheckOuter = CheckOuter->GetOuter() )
			{
				AActor* ReferencingActor = Cast<AActor>(CheckOuter);
				if ( ReferencingActor != NULL )
				{
					// clear the current selection
					GEditor->SelectNone(TRUE, FALSE);
					
					// select the specified actor
					GEditor->SelectActor( ReferencingActor, TRUE, NULL, FALSE, TRUE );

					// center the camera on the actor
					GEditor->MoveViewportCamerasToActor( *ReferencingActor, FALSE );
					bSuccess = TRUE;
					break;
				}
			}

			if ( bSuccess == TRUE )
			{
				GEditor->NoteSelectionChange();
			}
			else
			{
				Transaction.Cancel();
			}
		}

		UBOOL bIncludePrivateObjects = FALSE;
		while ( bSuccess == FALSE )
		{
			// either the referencer is contained by a content package or is contained by a map package
			// that is not loaded but is appearing in the generic browser (for some reason), so now attempt
			// to sync the generic browser to display this object
			
			// most references will come from stuff like components, etc. - iterate up the Outer chain of the referencer to find the 
			// inner-most object that is marked RF_Public and is supported by the generic browser.
			for ( UObject* CheckOuter = Referencer; CheckOuter != NULL; CheckOuter = CheckOuter->GetOuter() )
			{
				UBOOL bSupported = FALSE;
				if ( bIncludePrivateObjects || CheckOuter->HasAnyFlags(RF_Public) )
				{
					// see if this object is supported by the generic browser
					for ( INT ResourceIndex = 0; ResourceIndex < GenericBrowser->LeftContainer->ResourceTypes.Num(); ResourceIndex++ )
					{
						UGenericBrowserType* gbt = GenericBrowser->LeftContainer->ResourceTypes(ResourceIndex);
						if ( gbt->Supports(CheckOuter) )
						{
							bSupported = TRUE;
							break;
						}
					}
				}

				if ( bSupported == TRUE )
				{
					TArray<UObject*> SyncObjects;
					SyncObjects.AddItem(CheckOuter);

					USelection* GBSelection = GetGBSelection();
					GBSelection->BeginBatchSelectOperation();
					GBSelection->DeselectAll();
					GenericBrowser->SyncToObjects(SyncObjects);
					GBSelection->EndBatchSelectOperation();

					if ( GetGBSelection()->Num() > 0 )
					{
						bSuccess = TRUE;

						// if we successfully selected and synched to this object, stop here
						break;
					}
				}
			}

			if ( bSuccess == FALSE )
			{
				if ( bIncludePrivateObjects == TRUE )
				{
					break;
				}
				else
				{
					bIncludePrivateObjects = TRUE;
				}
			}
		}
	}
}

/** called when the user clicks a column header in the "referencer" list */
void WxGBRightContainer::OnReferencerListColumnClicked( wxListEvent& Event )
{
	int Column = Event.GetColumn();
	if( Column > -1 )
	{
		if( Column == ReferencerOptions.Column )
		{
			// Clicking on the same column will flip the sort order
			ReferencerOptions.bSortAscending = !ReferencerOptions.bSortAscending;
		}
		else
		{
			// Clicking on a new column will set that column as current and reset the sort order.
			ReferencerOptions.Column = Event.GetColumn();
			ReferencerOptions.bSortAscending = TRUE;
		}
	}

	ReferencersList->SortItems( WxReferencerListSort, (LONG)&ReferencerOptions );
}

void WxGBRightContainer::SetViewMode( ERightViewMode InViewMode )
{
	ViewMode = InViewMode;

	ListView->Show();
	ViewportHolder->Show();
	SplitterWindowV->SplitVertically( ListView, ViewportHolder, SplitterPosV );
	SplitterWindowV->SetSashPosition( SplitterPosV );

	switch( ViewMode )
	{
		case RVM_List:
			SplitterWindowV->Unsplit( ViewportHolder );
			break;

		case RVM_Thumbnail:
			SplitterWindowV->Unsplit( ListView );
			break;
	}
}

/**
 * Toggles the visibility of the "referencers" list
 */
void WxGBRightContainer::SetUsageFilter( UBOOL bUsageFilterEnabled )
{
	SplitterWindowV->Show();
	ReferencersList->Show();

	SplitterWindowH->SplitHorizontally(SplitterWindowV, ReferencersList, SplitterPosH);
	SplitterWindowH->SetSashPosition( SplitterPosH );

	if ( bUsageFilterEnabled == FALSE )
	{
		SplitterWindowH->Unsplit(ReferencersList);
	}
}

void WxGBRightContainer::UI_ViewMode( wxUpdateUIEvent& In )
{
	switch( In.GetId() )
	{
		case ID_VIEW_LIST:		In.Check( ViewMode == RVM_List );			break;
		case ID_VIEW_PREVIEW:	In.Check( ViewMode == RVM_Preview );		break;
		case ID_VIEW_THUMBNAIL:	In.Check( ViewMode == RVM_Thumbnail );		break;
	}
}

void WxGBRightContainer::UI_UsageFilter( wxUpdateUIEvent& Event )
{
	Event.Check(GenericBrowser->IsUsageFilterEnabled() == TRUE);
}

void WxGBRightContainer::UI_PrimType( wxUpdateUIEvent& In )
{
	switch( In.GetId() )
	{
		case ID_PRIMTYPE_SPHERE:	In.Check( ThumbnailPrimType == TPT_Sphere );	break;
		case ID_PRIMTYPE_CUBE:		In.Check( ThumbnailPrimType == TPT_Cube );		break;
		case ID_PRIMTYPE_CYLINDER:	In.Check( ThumbnailPrimType == TPT_Cylinder );	break;
		case ID_PRIMTYPE_PLANE:		In.Check( ThumbnailPrimType == TPT_Plane );		break;
	}
}

void WxGBRightContainer::UI_GroupByClass( wxUpdateUIEvent& In )
{
	In.Check( bGroupByClass == TRUE );
}

void WxGBRightContainer::UI_PSysRealTime( wxUpdateUIEvent& In )
{
	UThumbnailManager* ThumbMngr = GUnrealEd->GetThumbnailManager();
	if (ThumbMngr)
	{
		In.Check(ThumbMngr->bPSysRealTime == TRUE);
	}
}

void WxGBRightContainer::UI_SelectionInfo( wxUpdateUIEvent& In )
{
	// See if we need to update the selection set.
	if(GenericBrowser->LeftContainer->bTreeViewChanged)
	{
		GenericBrowser->LeftContainer->bTreeViewChanged = FALSE;

		DisableUpdate();
		RemoveInvisibleObjects();
		EnableUpdate();

		Update();
	}

	const INT NumSelectedObjects	= GenericBrowser->GetNumSelectedObjects();
	const INT NumTotalObjects		= ListView->GetItemCount();
	const FString Label				= FString::Printf( LocalizeSecure(LocalizeUnrealEd("GBSelectedTotalF"), NumSelectedObjects, NumTotalObjects) );

	if( Label != SelectionLabel->GetLabel() )
	{	
		SelectionLabel->SetLabel( *Label );
	}	
}

// Handles left and right mouse button input.
UBOOL WxGBRightContainer::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT amt,UBOOL Gamepad)
{
	if ( Key == KEY_LeftMouseButton && Event == IE_Released )
	{
		bIsScrolling = 0;
		bDraggingObject = FALSE;
		Viewport->Invalidate();
	}
	else if ( bIsScrolling )
	{
		return TRUE;
	}

	if((Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton) && (Event == IE_Pressed || Event == IE_DoubleClick))
	{
		const INT	HitX = Viewport->GetMouseX();
		const INT	HitY = Viewport->GetMouseY();
		HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);
		UObject*	HitObject = NULL;

		if( HitResult )
		{
			if( HitResult->IsA(HObject::StaticGetType()) )
			{
				HitObject = ((HObject*)HitResult)->Object;
			}
		}
		else
		{
			// Clicking on the background with the left button initiates scrolling.
			if ( Key == KEY_LeftMouseButton && Event == IE_Pressed )
			{
				bIsScrolling = 1;
				Viewport->Invalidate();
				return TRUE;
			}
		}

		if( HitObject )
		{
			UBOOL bDeselectAll = TRUE;
			UBOOL bSelectionChanged = FALSE;

			// Right-clicking on an object that is already selected will not deselect anything,
			// and instead display a context menu for all selected items.
			// Right-clicking on an object that is currently unselected will select only that object,
			// then display the context menu for that item.
			if ( Key == KEY_RightMouseButton )
			{
				const UBOOL bObjectAlreadySelected = GetGBSelection()->IsSelected( HitObject );
				if ( bObjectAlreadySelected )
				{
					bDeselectAll = FALSE;
				}
			}

			// Deselect everything unless CTRL is held down.
			if ( bDeselectAll )
			{
				if( !Viewport->KeyState(KEY_LeftControl) && !Viewport->KeyState(KEY_RightControl) )
				{
					GetGBSelection()->DeselectAll();
					bSelectionChanged = TRUE;
				}
			}

			if( Key == KEY_RightMouseButton )
			{
				if(HitObject->IsSelected() == FALSE)
				{
					GetGBSelection()->Select( HitObject );
					bSelectionChanged = TRUE;
				}
			}
			else
			{
				GetGBSelection()->ToggleSelect( HitObject );
				bSelectionChanged = TRUE;
				
				// Set this flag when the user has held down the left mouse button on an object and selected it.
				const UBOOL bSelectedObject = GetGBSelection()->IsSelected( HitObject );
				if(Event == IE_Pressed && bSelectedObject)
				{
					bDraggingObject = TRUE;
				}

				// If selecting a material with the left mouse button, apply it to all selected BSP.
				if( Key == KEY_LeftMouseButton && GetGBSelection()->IsSelected( HitObject ) && Cast<UMaterialInterface>( HitObject) )
				{
					GUnrealEd->Exec( TEXT("POLY SETMATERIAL") );
				}
			}

			if( bSelectionChanged )
			{
				if ( ViewMode == RVM_Preview )
				{
					UpdateList();
				}

				if ( GenericBrowser->IsUsageFilterEnabled() )
				{
					UpdateReferencers(HitObject);
				}
			}
		}

		Viewport->InvalidateDisplay();

		if( HitObject )
		{
			if( Key == KEY_LeftMouseButton && Event == IE_DoubleClick )
			{
				GenericBrowser->LeftContainer->CurrentResourceType->DoubleClick();
			}
			else if( Key == KEY_RightMouseButton && Event == IE_Pressed )
			{
				ShowContextMenu();
			}
		}
		else
		{
			// Right clicking on background and not currently scrolling - show 'New' menu.
			if( Key == KEY_RightMouseButton && Event == IE_Pressed )
			{
				ShowNewObjectMenu();
			}
		}
	}

	if ( Key == KEY_MouseScrollUp )
	{
		ViewportHolder->ScrollViewport( -_GB_SCROLL_SPEED );
	}
	else if ( Key == KEY_MouseScrollDown )
	{
		ViewportHolder->ScrollViewport( _GB_SCROLL_SPEED );
	}
	// Handle refresh requests if this window has focus
	if (Key == KEY_F5)
	{
		Update();
	}

	// CTRL+C copies references for selected GB objects.
	if ( Key == KEY_C && IsCtrlDown(Viewport) )
	{
		GenericBrowser->CopyReferences();
	}

	return TRUE;
}

UBOOL WxGBRightContainer::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	if ( bIsScrolling )
	{
		const FLOAT GRAB_SCROLL_SPEED = 5.0f;
		ViewportHolder->ScrollViewport( -appTrunc( Delta * GRAB_SCROLL_SPEED ) );
	}

	// If we clicked on a object and now we are moving our mouse, initiate drag drop mode and reset the flag to false.
	if( bDraggingObject )
	{	
		//Disable dragging object flag and show the mouse cursor again.
		bDraggingObject = FALSE;

		// @todo ASM: Implement drag and drop start here.

		/*
		UObject* SelectedAsset = GetGBSelection()->GetTop<UObject>();

		if(SelectedAsset != NULL)
		{	
			wxCustomDataObject Obj( wxDataFormat( TEXT("unrealed/asset") ) );

			Obj.SetData(sizeof(UObject*), &SelectedAsset);

			wxDropSource dragSource( this );
			dragSource.SetData( Obj );
			wxDragResult result = dragSource.DoDragDrop( TRUE );
		}
		*/
	}

	return TRUE;
}

void WxGBRightContainer::Draw( FViewport* Viewport, FCanvas* Canvas )
{
	// Don't draw if GWorld is NULL, which could be the case if, for example,
	// an OS paint event was received during a map transition.
	if ( bSuppressDraw || !GWorld )
	{
		return;
	}

	// Get the list of objects that we should draw in the thumbnail viewport
	const TArray<UObject*>& ObjectDrawList = GetVisibleObjects();

	DrawTile
	(
		Canvas,
		0,
		0,
		Viewport->GetSizeX(),
		Viewport->GetSizeY(),
		0.0f,
		0.0f,
		(FLOAT)Viewport->GetSizeX() / (FLOAT)GEditor->Bkgnd->SizeX,
		(FLOAT)Viewport->GetSizeY() / (FLOAT)GEditor->Bkgnd->SizeY,
		FLinearColor::White,
		GEditor->Bkgnd->Resource
	);

	// We can skip trying to draw objects if there are no supported classes.
	const UBOOL bHasSupportClass = (GenericBrowser && GenericBrowser->LeftContainer->CurrentResourceType->SupportInfo.Num() > 0);

	if( !bHasSupportClass )
	{
		return;
	}

	INT ScrollBarPosition = ViewportHolder->GetScrollThumbPos();

	if(ViewMode == RVM_Preview || ViewMode == RVM_Thumbnail)
	{
		INT XPos, YPos, YHighest, SBTotalHeight;

		XPos = YPos = STD_TNAIL_PAD_HALF;
		YHighest = SBTotalHeight = 0;

		YPos -= ViewportHolder->GetScrollThumbPos();

		FMemMark	Mark(GMainThreadMemStack);

		for( INT x = 0 ; x < ObjectDrawList.Num() ; ++x )
		{
			UObject* obj = ObjectDrawList(x);

			// Get the rendering info for this object
			FThumbnailRenderingInfo* RenderInfo =
				GUnrealEd->GetThumbnailManager()->GetRenderingInfo(obj);
			if (RenderInfo != NULL && RenderInfo->Renderer != NULL)
			{
				// If we are trying to sync to a specific object and we find it, record the vertical position.

				if( SyncToNextDraw == obj )
				{
					ScrollBarPosition = YPos + ViewportHolder->GetScrollThumbPos();
				}
				
				const WxBrowserData* bd = GenericBrowser->FindBrowserData( GenericBrowser->LeftContainer->CurrentResourceType->SupportInfo(0).Class );
				
				// Set the fixed size
				DWORD Width = bd->GetFixedSz();
				DWORD Height = bd->GetFixedSz();

				// Don't ask the code to calculate it if it's fixed
				if (Width == 0)
				{
					// Figure out how big thumbnail rendering is going to be
					RenderInfo->Renderer->GetThumbnailSize(obj,ThumbnailPrimType,
						bd->GetZoomFactor(),Width,Height);
				}

				// This is the object that will render the labels
				UThumbnailLabelRenderer* LabelRenderer = RenderInfo->LabelRenderer;

				// Setup thumbnail options
				UThumbnailLabelRenderer::ThumbnailOptions ThumbnailOptions;
				{
					// Do we want to draw content tags?  Check the user preference.
					ThumbnailOptions.bShowContentTags = GenericBrowser->bShowContentTags;
				}

				UMemCountThumbnailLabelRenderer* MemCountLabelRenderer = GenericBrowser->GetMemCountLabelRenderer();
				// Handle the case where the user wants memory information too
				// Use the memory counting label renderer to append the information
				if (MemCountLabelRenderer != NULL)
				{
					// Assign the aggregated label renderer
					MemCountLabelRenderer->AggregatedLabelRenderer = LabelRenderer;
					LabelRenderer = MemCountLabelRenderer;
				}
				DWORD LabelsWidth = 0;
				DWORD LabelsHeight = 0;
				// See if there is a registered label renderer
				if (LabelRenderer)
				{
					// Get the size of the labels
					LabelRenderer->GetThumbnailLabelSize(obj,
						GEngine->SmallFont,Viewport,Canvas, ThumbnailOptions, LabelsWidth,
						LabelsHeight);
				}
				const DWORD MaxWidth = Max<DWORD>(Width,LabelsWidth);

				// If this thumbnail is too large for the current line, move to the next line.
				if( XPos + MaxWidth > Viewport->GetSizeX() )
				{
					YPos += YHighest + STD_TNAIL_PAD_HALF;
					SBTotalHeight += YHighest + STD_TNAIL_PAD_HALF;
					YHighest = 0;
					XPos = STD_TNAIL_PAD_HALF;
				}

				if(YPos + (INT)Height + STD_TNAIL_PAD >= 0 && YPos <= (INT)Viewport->GetSizeY())
				{
					if(Canvas->IsHitTesting())
					{
						// If hit testing, draw a tile instead of the actual thumbnail to avoid rendering thumbnail scenes(which screws up hit detection).

						Canvas->SetHitProxy(new HObject(obj));
						DrawTile(Canvas,XPos,YPos,MaxWidth,Height + LabelsHeight,0.0f,0.0f,1.f,1.f,FLinearColor::White,GEditor->BkgndHi->Resource);
						Canvas->SetHitProxy(NULL);
					}
					else
					{
						// Draw the border with the configured color
						DrawTile(Canvas,XPos - STD_TNAIL_HIGHLIGHT_EDGE,
							YPos - STD_TNAIL_HIGHLIGHT_EDGE,
							MaxWidth + (STD_TNAIL_HIGHLIGHT_EDGE * 2),
							Height + LabelsHeight + (STD_TNAIL_HIGHLIGHT_EDGE * 2),
							0.f,0.f,1.f,1.f,RenderInfo->BorderColor);
						// Figure whether to draw it with black or highlight color
						FColor Backdrop(0,0,0);
						if (GetGBSelection()->IsSelected( obj ))
						{
							Backdrop = FColor(255,255,255);
						}
						// Draw either a black backdrop or the highlight color one
						DrawTile(Canvas,XPos - STD_TNAIL_HIGHLIGHT_EDGE + 2,
							YPos - STD_TNAIL_HIGHLIGHT_EDGE + 2,
							MaxWidth + (STD_TNAIL_HIGHLIGHT_EDGE * 2) - 4,
							Height + LabelsHeight + (STD_TNAIL_HIGHLIGHT_EDGE * 2) - 4,
							0.f,0.f,1.f,1.f,Backdrop,GEditor->BkgndHi->Resource);
						// Draw the thumbnail
						RenderInfo->Renderer->Draw(obj,ThumbnailPrimType,
							XPos,YPos,Width,Height,Viewport,
							Canvas,TBT_None,
							GEditor->GetUserSettings().PreviewThumbnailBackgroundColor,
							GEditor->GetUserSettings().PreviewThumbnailTranslucentMaterialBackgroundColor);
						// See if there is a registered label renderer
						if (LabelRenderer != NULL)
						{
							// Now render the labels for this thumbnail
							LabelRenderer->DrawThumbnailLabels(obj,
								GEngine->SmallFont,XPos,YPos + Height,Viewport,Canvas,ThumbnailOptions);
						}
					}
				}

				// Keep track of the tallest thumbnail on this line
				if ((INT)(Height + LabelsHeight) > YHighest)
				{
					YHighest = Height + LabelsHeight;
				}

				XPos += MaxWidth + STD_TNAIL_PAD_HALF;

				// Zero any ref since it isn't needed anymore
				if (MemCountLabelRenderer != NULL)
				{
					MemCountLabelRenderer->AggregatedLabelRenderer = NULL;
				}
			}
		}
		Mark.Pop();

		// Update the scrollbar in the viewport holder

		SBTotalHeight += YHighest + STD_TNAIL_PAD_HALF + STD_TNAIL_HIGHLIGHT_EDGE;
		ViewportHolder->UpdateScrollBar( ScrollBarPosition, SBTotalHeight );

		// If we are trying to sync to a specific object, redraw the viewport so it
		// will be visible.

		if( SyncToNextDraw )
		{
			SyncToNextDraw = NULL;
			Draw( Viewport, Canvas );
		}
	}
	
	// Wait for the rendering thread to finish drawing the browser view before returning.
	// This prevents multiple pending frames from being queued up while scrolling.
	FlushRenderingCommands();
}

// Copies the list of currently selected objects into the input array.
const TArray<UObject*>& WxGBRightContainer::GetVisibleObjects(void)
{
	// Only update if something dirtied our state
	if (bNeedsUpdate)
	{
		bNeedsUpdate = FALSE;
		// Clear our arrays and fill them appropriately
		VisibleObjects.Empty();
		FilteredObjects.Empty();

		if( ViewMode == RVM_Thumbnail )
		{
			if( GenericBrowser )
			{
				TArray<UPackage*> Packages;
				GenericBrowser->LeftContainer->GetSelectedPackages( &Packages );
				GetObjectsInPackages( Packages, VisibleObjects );
			}
		}
		else
		{
			for( INT Sel = ListView->GetFirstSelected() ; Sel != -1 ; Sel = ListView->GetNextSelected( Sel ) )
			{
				UObject* obj = (UObject*)ListView->GetItemData( Sel );
				FilteredObjects.AddItem( obj );
			}
		}
	}
	// Return the set to draw based upon our view mode
	return ViewMode == RVM_Thumbnail ? VisibleObjects : FilteredObjects;
}

/**
 * For use with the templated sort. Sorts by class name then object name
 */
struct FClassNameObjNameCompare
{
	static INT Compare(UObject* A,UObject* B)
	{
		INT Class = appStricmp(*A->GetClass()->GetName(),*B->GetClass()->GetName());
		return Class == 0 ? appStricmp(*A->GetName(),*B->GetName()) : Class;
	}
};

/**
 * For use with the templated sort. Sorts by object name
 */
struct FObjNameCompare
{
	static INT Compare(UObject* A,UObject* B)
	{
		return appStricmp(*A->GetName(),*B->GetName());
	}
};

/**
 * Fills the OutObjects list with all valid objects that are supported by the current
 * browser settings and that reside withing the set of specified packages.
 *
 * @param	InPackages			Filters objects based on package.
 * @param	OutObjects			[out] Receives the list of objects
 * @param	bIgnoreUsageFilter	specify TRUE to prevent dependency checking serialization; useful for retrieving objects from packages in situations the returned object
 *								set isn't used for displaying stuff in the GB.
 */
void WxGBRightContainer::GetObjectsInPackages(const TArray<UPackage*>& InPackages, TArray<UObject*>& OutObjects, UBOOL bIgnoreUsageFilter/*=FALSE*/ )
{
	const UBOOL bUsageFilterEnabled = !bIgnoreUsageFilter && GenericBrowser->IsUsageFilterEnabled();
	if ( bUsageFilterEnabled )
	{
		GenericBrowser->MarkReferencedObjects();
	}

	// Assemble a list of level packages for levels referenced by the current world.
	// Handle GWorld potentially being NULL e.g. if refreshing between level transitions.
	TMap<const UPackage*, const UPackage*> LevelPackages;
	if ( GWorld )
	{
		const UPackage* CurrentLevelPackage = GWorld->CurrentLevel->GetOutermost();
		LevelPackages.Set( CurrentLevelPackage, CurrentLevelPackage );

		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		for ( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if ( StreamingLevel && StreamingLevel->LoadedLevel )
			{
				const UPackage* LevelPackage = StreamingLevel->LoadedLevel->GetOutermost();
				LevelPackages.Set( LevelPackage, LevelPackage );
			}
		}
	}

	// Iterate over all objects.
	for( TObjectIterator<UObject> It ; It ; ++It )
	{
		UObject* Obj = *It;

		// Filter out invalid objects early.
		const UBOOL bUnreferencedObject = bUsageFilterEnabled && !Obj->HasAnyFlags(RF_TagExp);
		const UBOOL bIndirectlyReferencedObject = bUsageFilterEnabled && Obj->HasAnyFlags(RF_TagImp);
		const UBOOL bRejectObject = Obj->GetOuter() == NULL ||
									Obj->HasAnyFlags(RF_Transient) ||
									Obj->IsPendingKill() ||
									bUnreferencedObject || bIndirectlyReferencedObject ||
									Obj->IsA( UPackage::StaticClass() );
		if ( bRejectObject )
		{
			continue;
		}

		// Make sure this object resides in one of the specified packages.
		UBOOL bIsInPackage = FALSE;
		for ( INT PackageIndex = 0 ; PackageIndex < InPackages.Num() ; ++PackageIndex )
		{
			const UPackage* Package = InPackages(PackageIndex);
			if ( Obj->IsIn(Package) )
			{
				bIsInPackage = TRUE;
				break;
			}
		}

		if ( bIsInPackage && GUnrealEd->GetThumbnailManager()->GetRenderingInfo(Obj) != NULL )
		{
			// Filter by the current browser resource type.
			if ( GenericBrowser->LeftContainer->CurrentResourceType->Supports(Obj) )
			{
				// Check to see if we need to filter based on name.
				if ( NameFilter.Len() > 0 )
				{
					const FString ObjNameUpper		= FString( *Obj->GetName() ).ToUpper();
					const FString NameFilterUpper	= NameFilter.ToUpper();
					if ( !appStrstr(*ObjNameUpper, *NameFilterUpper) )
					{
						// Names don't match, advance to the next one.
						continue;
					}
				}

				if( Obj->HasAnyFlags(RF_Public) || LevelPackages.FindRef( Obj->GetOutermost() ) )
				{
					// Add to the list.
					OutObjects.AddItem( Obj );

					// if usage filter is enabled, and this object's Outer isn't part of the original list of objects to display
					// (because it wasn't directly referenced by an object in the referencer container list, for example), add it now
					// and mark it with RF_TagImp to indicate that we've added this object
					if ( bUsageFilterEnabled )
					{
						for ( UObject* ResourceOuter = Obj->GetOuter(); ResourceOuter && !ResourceOuter->HasAnyFlags(RF_TagImp|RF_TagExp) &&
							ResourceOuter->GetClass() != UPackage::StaticClass(); ResourceOuter = ResourceOuter->GetOuter() )
						{
							OutObjects.AddItem(ResourceOuter);
							ResourceOuter->SetFlags(RF_TagImp);
						}
					}
				}
			}
		}
	}

	// Sort the resulting list of objects.
	if( bGroupByClass )
	{
		// Sort by class.
		Sort<UObject*,FClassNameObjNameCompare>( &OutObjects(0), OutObjects.Num() );
	}
	else
	{
		// Sort by name.
		Sort<UObject*,FObjNameCompare>( &OutObjects(0), OutObjects.Num() );
	}
}

// Brings up a context menu that allows creation of new objects using Factories.
// Serves as an alternative to choosing 'New' from the File menu.
void WxGBRightContainer::ShowNewObjectMenu()
{
	wxMenu* NewMenu = new wxMenu();

	for( INT i=0; i<GenericBrowser->FactoryNewClasses.Num(); i++ )
	{
		UFactory* DefFactory = CastChecked<UFactory>( GenericBrowser->FactoryNewClasses(i)->GetDefaultObject() );
		FString ItemText = FString::Printf( LocalizeSecure(LocalizeUnrealEd("New_F"), *DefFactory->Description) );

		NewMenu->Append( ID_NEWOBJ_START + i, *ItemText, TEXT("") );
	}

	POINT pt;
	::GetCursorPos(&pt);

	GenericBrowser->PopupMenu(NewMenu,GenericBrowser->ScreenToClient(wxPoint(pt.x,pt.y)));
	delete NewMenu;
}

/**
 * Selects the resource specified by InObject and scrolls the viewport
 * so that resource is in view.
 *
 * @param	InObject		The resource to sync to.
 * @param	bDeselectAll	If TRUE (the default), deselect all objects the specified resource's class.
 */

void WxGBRightContainer::SyncToObject(UObject* InObject, UBOOL bDeselectAll)
{
	check( InObject );
	if ( bDeselectAll )
	{
		GetGBSelection()->DeselectAll();
	}
	GetGBSelection()->Select( InObject );

    // Scroll the object into view
	SyncToNextDraw = InObject;

	Viewport->Invalidate();
}

/**
 * Display a popup menu for the selected objects.  If the objects vary in type it will show a generic pop up menu instead of a object specific one.
 */
void WxGBRightContainer::ShowContextMenu( )
{
	// Find the best context menu for the selected objects.
	USelection* Selection = GetGBSelection();
	WxMBGenericBrowserContextBase* ContextMenu = GenericBrowser->LeftContainer->CurrentResourceType->GetContextMenu( Selection );

	if(ContextMenu)
	{
		// Give the context menu a chance to set its state based on the selection set.
		ContextMenu->SetObjects( Selection );

		// Display the context menu.
		POINT pt;
		::GetCursorPos(&pt);
		GenericBrowser->PopupMenu(ContextMenu,GenericBrowser->ScreenToClient(wxPoint(pt.x,pt.y)));
	}
}

void WxGBRightContainer::OnZoomFactorSelChange( wxCommandEvent& In )
{
	UBOOL bIsDefaultSetting = GenericBrowser->IsDefaultResourceSelected();
	// If this is also the default, then update that setting too
	if (bIsDefaultSetting)
	{
		WxBrowserData::SetDefaultFixedSize(0);
	}


	WxBrowserData* bd = NULL;
	const UBOOL bHasSupportClass = (GenericBrowser->LeftContainer->CurrentResourceType->SupportInfo.Num() > 0);
	
	if( bHasSupportClass )
	{
		bd = GenericBrowser->FindBrowserData( GenericBrowser->LeftContainer->CurrentResourceType->SupportInfo(0).Class );
	}
	
	if( bd != NULL )
	{
		bd->SetFixedSz( 0 );
	}
	
	const FString WkString = (const TCHAR *)In.GetString();

	if( WkString.InStr( TEXT("%") ) > -1 )
	{
		FLOAT ZoomFactor = appAtof( *WkString ) / 100.0f;
		
		if( bd != NULL )
		{
			bd->SetZoomFactor( ZoomFactor );
		}

		// If this is also the default, then update that setting too
		if (bIsDefaultSetting)
		{
			WxBrowserData::SetDefaultZoomFactor(ZoomFactor);
		}
	}
	else
	{
		const INT Pos = WkString.InStr( TEXT("x") );
		INT FixedSize = appAtoi( *WkString.Left( Pos ) );

		if(bd != NULL)
		{
			bd->SetFixedSz( FixedSize );
		}
		
		// If this is also the default, then update that setting too
		if (bIsDefaultSetting)
		{
			WxBrowserData::SetDefaultFixedSize(FixedSize);
		}
	}

	Viewport->Invalidate();
}

void WxGBRightContainer::OnNameFilterChanged( wxCommandEvent& In )
{
	NameFilter = FString( In.GetString() );
	Update();
}

void WxGBRightContainer::OnScroll( wxScrollEvent& InEvent )
{
	ViewportHolder->Viewport->Invalidate();
	// Force it to draw so the view change is seen
	ViewportHolder->Viewport->Draw();
}

/** Resource input command event */
void WxGBRightContainer::OnAtomicResourceReimport( wxCommandEvent& In )
{
	TArray<UPackage*> Packages;
	GenericBrowser->LeftContainer->GetSelectedTopLevelPackages(&Packages);

	for (INT PackageIndex = 0; PackageIndex < Packages.Num(); PackageIndex++)
	{
		UPackage* Package = Packages(PackageIndex);

		// There appears to be no way to iterate through
		// objects that are children of a given object, so we have
		// to brute force our way through the entire object population.

		// Build a list of all objects that meet our criteria        
		TArray<UObject*>ObjList;
		INT i = 0;
		for( TObjectIterator<UObject> It ; It ; ++It )
		{
			if (It->IsIn(Package))
			{

				// Grab any/all textures.  They will be handled on a class by class basis below.
				if ( It->IsA( UTexture::StaticClass()))
				{
					ObjList.AddItem( *It );
				}       
				// Grab any other classes we have reimport factories for... (none right now)
			}
		}

		// Now do the work
		GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("ReimportingAssets")), TRUE );
		for (i=0;i<ObjList.Num();i++)
		{
			GWarn->StatusUpdatef(i+1,ObjList.Num(),LocalizeSecure(LocalizeUnrealEd(TEXT("ReimportingAssetF")), i+1,ObjList.Num()) );

			UObject* pObj = ObjList(i);

			FReimportManager::Instance()->Reimport( pObj );
		}
		GWarn->EndSlowTask();
	} 
	GenericBrowser->Refresh();
}

/**
 * Toggles the internal variable that manages whether memory counting is
 * on or not.
 *
 * @param In the event to process
 */
void WxGenericBrowser::OnShowMemStats(wxCommandEvent& In)
{
	bShowMemoryStats ^= TRUE;
	// Redraw the right pane
	RightContainer->Update();
}

/**
 * Updates the menu's check state based upon whether memory counting is
 * on or not
 *
 * @param In the event to set the checked or unchecked state on
 */
void WxGenericBrowser::UI_ShowMemStats(wxUpdateUIEvent& In)
{
	In.Check( bShowMemoryStats == TRUE );
}



/**
 * Toggles whether content tags are visible in thumbnails.  Called from a Wx menu event.  
 *
 * @param In the event to process
 */
void WxGenericBrowser::OnShowContentTags( wxCommandEvent& In )
{
	bShowContentTags = !bShowContentTags;

	// Redraw the right pane
	RightContainer->Update();
}

void WxGenericBrowser::OnSetMaterialPreviewBackgroundColor( wxCommandEvent& In )
{
	PickColorFor(GEditor->AccessUserSettings().PreviewThumbnailBackgroundColor);
	GEditor->SaveUserSettings();
}

void WxGenericBrowser::OnSetTranslucentMaterialPreviewBackgroundColor( wxCommandEvent& In )
{
	PickColorFor(GEditor->AccessUserSettings().PreviewThumbnailTranslucentMaterialBackgroundColor);
	GEditor->SaveUserSettings();
}

void WxGenericBrowser::PickColorFor(FColor &InOutColor)
{
	WxDlgColor colorPickDialog;

	wxColourData dialogOptions;
	dialogOptions.SetColour( wxColor(InOutColor.R, InOutColor.G, InOutColor.B) );
	dialogOptions.SetChooseFull(TRUE);

	colorPickDialog.Create(this, &dialogOptions);
	if( colorPickDialog.ShowModal() == wxID_OK )
	{
		wxColor outColor = colorPickDialog.GetColourData().GetColour();

		InOutColor = FColor( outColor.Red(), outColor.Green(), outColor.Blue() );

		RightContainer->Update();
	}
	return;
}

/**
 * Updates the menu's check state based upon whether content tags are visible
 *
 * @param In the event to set the checked or unchecked state on
 */
void WxGenericBrowser::UI_ShowContentTags( wxUpdateUIEvent& In )
{
	In.Check( bShowContentTags == TRUE );
}


	
void WxGenericBrowser::OnShowStreamingBounds(wxCommandEvent& In)
{
	UObject* FirstObject = NULL;
	USelection* Selection = GetGBSelection();
	for (USelection::TObjectIterator It(Selection->ObjectItor()) ; It ; ++It)
	{
		UObject* Object = *It;
		if (Object)
		{
			if (FirstObject == NULL)
			{
				FirstObject = Object;
				break;
			}
		}
	}

	UTexture2D* Texture = Cast<UTexture2D>(FirstObject);
	if (Texture == GEditor->StreamingBoundsTexture)
	{
		GEditor->SetStreamingBoundsTexture(NULL);
	}
	else
	{
		GEditor->SetStreamingBoundsTexture(Texture);
	}

	// Redraw the right pane
	RightContainer->Update();
}

void WxGenericBrowser::UI_ShowStreamingBounds(wxUpdateUIEvent& In)
{
	UBOOL bCheckIt = FALSE;

	UObject* FirstObject = NULL;
	USelection* Selection = GetGBSelection();
	for (USelection::TObjectIterator It(Selection->ObjectItor()) ; It ; ++It)
	{
		UObject* Object = *It;
		if (Object)
		{
			if (FirstObject == NULL)
			{
				FirstObject = Object;
				break;
			}
		}
	}

	UTexture* Texture = Cast<UTexture>(FirstObject);
	if (Texture == GEditor->StreamingBoundsTexture)
	{
		bCheckIt = TRUE;
	}

	In.Check(bCheckIt ? TRUE : FALSE);
}

/**
 * Called when the user toggles the "Display only referenced assets" button.  Updates the value of bShowReferencedOnly
 */
void WxGenericBrowser::OnUsageFilter( wxCommandEvent& Event )
{
	bShowReferencedOnly = !bShowReferencedOnly;

	LeftContainer->SetUsageFilter(bShowReferencedOnly);
	RightContainer->SetUsageFilter(bShowReferencedOnly);

	Update();
}

/**
 * Since this class holds onto an object reference, we need to serialize it
 * so that we don't have it GCed underneath us
 *
 * @param Ar The archive to serialize with
 */
void WxGenericBrowser::Serialize(FArchive& Ar)
{
	Ar << LabelRenderer;
	// Make sure to clean out any pointers that might be collected
	RightContainer->ClearObjectReferences();
}

/**
 * Empties the arrays that hold object pointers since they aren't
 * serialized. Marks the class as needing refreshed too
 */
void WxGBRightContainer::ClearObjectReferences(void)
{
	VisibleObjects.Empty();
	FilteredObjects.Empty();
	Update();
}

/**
 * Removes objects from the selection set that aren't currently visible to the user.
 */
void WxGBRightContainer::RemoveInvisibleObjects()
{
	// See what is selected on the left side.
	TArray<UPackage*> SelectedPackages;
	GenericBrowser->LeftContainer->GetSelectedPackages( &SelectedPackages );

	VisibleObjects.Empty();
	GetObjectsInPackages( SelectedPackages, VisibleObjects );

	// Make sure that our GB selection contains only visible objects.
	// @todo Very unoptimized, could use a better way
	TArray<UObject*> UnselectObjects;

	for ( USelection::TObjectIterator It( GEditor->GetSelectedObjects()->ObjectItor() ) ; It ; ++It )
	{
		UObject* Object = *It;

		if(Object != NULL)
		{
			const UBOOL bContainsItem = VisibleObjects.ContainsItem(Object);

			if(bContainsItem == FALSE)
			{
				UnselectObjects.AddItem(Object);
			}
		}
	}

	for(INT ObjectIdx = 0; ObjectIdx < UnselectObjects.Num(); ObjectIdx++)
	{
		UObject* Object = UnselectObjects(ObjectIdx);

		GetGBSelection()->Deselect(Object);
	}

}



/** Simplifies (or resimplifies) the selected static mesh for the current level */
void WxGenericBrowser::OnSimplifyStaticMesh( wxCommandEvent& In )
{
	for( USelection::TObjectIterator SelIter( GetGBSelection()->ObjectItor() );
		 SelIter != NULL;
		 ++SelIter )
	{
		if( *SelIter != NULL )
		{
			// Make sure we have a static mesh
		 	UStaticMesh* StaticMesh = Cast<UStaticMesh>( *SelIter );
			if( StaticMesh != NULL )
			{
				// OK, launch the static mesh editor (in 'simplify mode') with the selected mesh
				WxStaticMeshEditor* StaticMeshEditor =
					new WxStaticMeshEditor( GApp->EditorFrame, -1, StaticMesh, TRUE );
				StaticMeshEditor->Show( TRUE );

				// Only one mesh supported at a time (due to modeless dialog fun)
				break;
			}
		}
	}
}


UBOOL WxGenericBrowser::DisallowOperationOnCookedPackages(const TArray<UPackage*>& Packages)
{
	for( INT PackageIndex = 0 ; PackageIndex < Packages.Num() ; ++PackageIndex )
	{
		const UPackage* Package = Packages(PackageIndex);
		if( Package->PackageFlags & PKG_Cooked )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_OperationDisallowedOnCookedContent") );
			return FALSE;
		}
	}

	return FALSE;
}
