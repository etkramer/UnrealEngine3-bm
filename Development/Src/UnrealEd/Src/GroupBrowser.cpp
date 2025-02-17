/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "GroupBrowser.h"
#include "Controls.h"
#include "ScopedTransaction.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utility methods.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_COMPARE_CONSTREF( FString, GroupBrowser, { return appStricmp(*A,*B); } );

/**
 * @return		TRUE if the actor can be considered by the group browser, FALSE otherwise.
 */
static FORCEINLINE UBOOL IsValid(const AActor* Actor)
{
	return ( !Actor->IsABuilderBrush() && Actor->GetClass()->GetDefaultActor()->bHiddenEd == FALSE );
}

/**
 * @return		The number of selected actors.
 */
static INT NumActorsSelected()
{
	INT NumSelected = 0;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		++NumSelected;
	}

	return NumSelected;
}

/**
 * Iterates over the specified wxCheckListBox object and extracts entries to the output string list.
 */
static void GetAllGroupNames(const wxCheckListBox* GroupList, TArray<FString>& OutGroupNames)
{
	const INT NumEntries = GroupList->GetCount();

	// Assemble a list of group names.
	for( INT Index = 0 ; Index < NumEntries ; ++Index )
	{
		OutGroupNames.AddItem( GroupList->GetString(Index).c_str() );
	}
}

/**
 * Iterates over the specified wxListBox object and extracts selected entries to the output string list.
 */
static void GetSelectedNames(const wxListBox* InList, TArray<FString>& OutSelectedNames)
{
	wxArrayInt Selections;
	const INT NumSelections = InList->GetSelections( Selections );

	// Assemble a list of selected group names.
	for( INT SelectionIndex = 0 ; SelectionIndex < NumSelections ; ++SelectionIndex )
	{
		OutSelectedNames.AddItem( InList->GetString( Selections[SelectionIndex] ).c_str() );
	}
}

/** Iterates over the specified wxListView object and extracts selected entries to the output string list. */
static void GetSelectedActors(const wxListView* InList, TArray<AActor*>& OutSelectedActors)
{
	// Assemble a list of selected group names.
	INT SelectionIndex = InList->GetFirstSelected();
	while ( SelectionIndex != -1 )
	{
		OutSelectedActors.AddItem( (AActor*)InList->GetItemData(SelectionIndex) );
		SelectionIndex = InList->GetNextSelected( SelectionIndex );
	}
}

/**
 * Iterates over the specified wxCheckListBox object and extracts selected entry indices to the output index list.
 */
static void GetSelectedGroupIndices(const wxCheckListBox* GroupList, TArray<INT>& OutSelectedGroupIndices)
{
	wxArrayInt Selections;
	const INT NumSelections = GroupList->GetSelections( Selections );

	// Assemble a list of selected group indices.
	for( INT SelectionIndex = 0 ; SelectionIndex < NumSelections ; ++SelectionIndex )
	{
		OutSelectedGroupIndices.AddItem( static_cast<INT>( Selections[SelectionIndex] ) );
	}
}

/**
 * Removes an actor from the specified group, returning TRUE on success or FALSE on fail.
 */
static UBOOL DeleteActorFromGroup(AActor* Actor, const FString& GroupToRemove, UBOOL bShouldModify)
{
	if(	IsValid( Actor ) )
	{
		TArray<FString> GroupArray;
		FString GroupName = Actor->Group.ToString();
		GroupName.ParseIntoArray( &GroupArray, TEXT(","), FALSE );

		const UBOOL bRemovedFromGroup = GroupArray.RemoveItem( GroupToRemove ) > 0;
		if ( bRemovedFromGroup )
		{
			// Reconstruct a new group name list for the actor.
			GroupName = TEXT("");
			for( INT GroupIndex = 0 ; GroupIndex < GroupArray.Num() ; ++GroupIndex )
			{
				if( GroupName.Len() )
				{
					GroupName += TEXT(",");
				}
				GroupName += GroupArray(GroupIndex);
			}

			if ( bShouldModify )
			{
				Actor->Modify();
			}
			Actor->Group = FName( *GroupName );
			return TRUE;
		}
	}
	return FALSE;
}

static UBOOL DeleteSelectedActorsFromGroups(const TArray<FString>& GroupNames)
{
	UBOOL bReturnVal = FALSE;

	const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("DeleteSelectedActorsGroup")) );

	TArray<FString> GroupArray;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( IsValid( Actor ) )
		{
			// Break the actors group names down and only add the ones back in that don't occur in GroupNames.
			FString GroupName = Actor->Group.ToString();
			GroupName.ParseIntoArray( &GroupArray, TEXT(","), FALSE );

			UBOOL bRemovedFromGroup = FALSE;
			for( INT x = 0 ; x < GroupArray.Num() ; ++x )
			{
				if ( GroupNames.ContainsItem( GroupArray(x) ) )
				{
					GroupArray.Remove( x );
					--x;
					bRemovedFromGroup = TRUE;
				}
			}

			if ( bRemovedFromGroup )
			{
				// Reconstruct a new group name list for the actor.
				GroupName = TEXT("");
				for( INT GroupIndex = 0 ; GroupIndex < GroupArray.Num() ; ++GroupIndex )
				{
					if( GroupName.Len() )
					{
						GroupName += TEXT(",");
					}
					GroupName += GroupArray(GroupIndex);
				}
				Actor->Modify();
				Actor->Group = FName( *GroupName );
				bReturnVal = TRUE;
			}
		}
	}

	return bReturnVal;
}

/**
 * Selects actors belonging to the selected groups.
 * @return		TRUE if at least one actor was selected.
 */
static UBOOL SelectActorsInGroups(const TArray<FString>& SelectedGroupNames, UBOOL InSelect)
{
	UBOOL bSelectedActor = FALSE;

	if ( SelectedGroupNames.Num() > 0 )
	{
		for( FActorIterator It ; It ; ++It )
		{
			AActor* Actor = *It;
			if( IsValid( Actor ) )
			{
				for ( INT GroupIndex = 0 ; GroupIndex < SelectedGroupNames.Num() ; ++GroupIndex )
				{
					if ( Actor->IsInGroup( *SelectedGroupNames(GroupIndex) ) )
					{
						GUnrealEd->SelectActor( Actor, InSelect, NULL, FALSE, TRUE );
						bSelectedActor = TRUE;
						break;
					}
				}
			}
		}
	}

	return bSelectedActor;
}

/**
 * Adds the actor to the specified group, returning TRUE on sucess or FALSE on fail.
 */
static UBOOL AddActorToGroup(AActor* Actor, const FString& GroupName, UBOOL bShouldModify)
{
	if(	IsValid( Actor ) )
	{
		// Don't add the actor to a group it already belongs to.
		if( !Actor->IsInGroup( *GroupName ) )
		{
			if ( bShouldModify )
			{
				Actor->Modify();
			}

			const FString CurGroup = Actor->Group.ToString();
			Actor->Group = FName( *FString::Printf(TEXT("%s%s%s"), *CurGroup, (CurGroup.Len() ? TEXT(","):TEXT("")), *GroupName ) );

			// Remove the actor from "None.
			const FString NoneGroup( TEXT("None") );
			if ( Actor->IsInGroup( *NoneGroup ) )
			{
				DeleteActorFromGroup( Actor, NoneGroup, bShouldModify );
			}
			return TRUE;
		}
	}
	return FALSE;
}

static UBOOL AddSelectedActorsToGroups(const TArray<FString>& GroupNames)
{
	UBOOL bReturnVal = FALSE;

	const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("AddSelectedActorsGroup")) );

	const FString NoneString( TEXT("None") );
	const UBOOL bAddingToNoneGroup = GroupNames.ContainsItem( NoneString );

	TArray<FString> GroupArray;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( IsValid( Actor ) )
		{
			// Break the actors group names down and only add the ones back in that don't occur in GroupNames.
			FString GroupName = Actor->Group.ToString();
			GroupName.ParseIntoArray( &GroupArray, TEXT(","), FALSE );

			const INT OldGroupNum = GroupArray.Num();
			const UBOOL bWasInNoneGroup = GroupArray.ContainsItem( NoneString );

			// Add groups to the list.
			for( INT x = 0 ; x < GroupNames.Num() ; ++x )
			{
				GroupArray.AddUniqueItem( GroupNames(x) );
			}

			// Were any new groups added to this actor?
			if ( OldGroupNum != GroupArray.Num() )
			{
				// Reconstruct a new group name list for the actor.
				GroupName = TEXT("");

				// Skip over the "None" group if the actor used to belong to that group and we're not adding it.
				const UBOOL bSkipOverNoneGroup = bWasInNoneGroup && !bAddingToNoneGroup;

				for( INT GroupIndex = 0 ; GroupIndex < GroupArray.Num() ; ++GroupIndex )
				{
					// Skip over the "None" group if the actor used to belong to that group
					if ( bSkipOverNoneGroup && GroupArray(GroupIndex) == NoneString )
					{
						continue;
					}

					if( GroupName.Len() )
					{
						GroupName += TEXT(",");
					}
					GroupName += GroupArray(GroupIndex);
				}
				Actor->Modify();
				Actor->Group = FName( *GroupName );
				bReturnVal = TRUE;
			}
		}
	}

	return bReturnVal;
}

static UBOOL AddSelectedActorsToGroup(const FString& GroupName)
{
	UBOOL bReturnVal = FALSE;

	const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("AddSelectedActorsGroup")) );

	const FString NoneGroup( TEXT("None") );
	const UBOOL bAddingToNoneGroup = (GroupName == NoneGroup);

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( IsValid( Actor ) )
		{
			// Don't add the actor to a group it already belongs to.
			if( !Actor->IsInGroup( *GroupName ) )
			{
				// Add the group to the actors group list.
				Actor->Modify();

				const FString CurGroup = Actor->Group.ToString();
				Actor->Group = FName( *FString::Printf(TEXT("%s%s%s"), *CurGroup, (CurGroup.Len() ? TEXT(","):TEXT("")), *GroupName ) );

				// If we're not adding to the "None" group and the actor has single membership in the "None" group, remove the actor from "None.
				if ( !bAddingToNoneGroup && Actor->IsInGroup( *NoneGroup ) )
				{
					DeleteActorFromGroup( Actor, NoneGroup, TRUE );
				}
				bReturnVal = TRUE;
			}
		}
	}
	return bReturnVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxDlgGroup
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Class for the "New Group"/"Rename Group" dialog
 */
class WxDlgGroup : public wxDialog
{
public:
	WxDlgGroup()
	{
		const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_GROUP") );
		check( bSuccess );

		NameEdit = (wxTextCtrl*)FindWindow( XRCID( "IDEC_NAME" ) );
		check( NameEdit != NULL );

		FWindowUtil::LoadPosSize( TEXT("DlgGroup"), this );
		FLocalizeWindow( this );
	}

	~WxDlgGroup()
	{
		FWindowUtil::SavePosSize( TEXT("DlgGroup"), this );
	}

	const FString& GetObjectName() const
	{
		return Name;
	}

	/**
	 * Displays the dialog.
	 *
	 * @param		bInNew		If TRUE, behave has a "New Group" dialog; if FALSE, "Rename Group"
	 * @param		InName		The string with which to initialize the edit box.
	 */
	int ShowModal(UBOOL bInNew, const FString& InName)
	{
		if( bInNew )
		{
			SetTitle( *LocalizeUnrealEd("NewGroup") );
		}
		else
		{
			SetTitle( *LocalizeUnrealEd("RenameGroup") );
		}

		Name = InName;
		NameEdit->SetValue( *Name );

		return wxDialog::ShowModal();
	}

private:
	FString Name;
	wxTextCtrl *NameEdit;

	void OnOK(wxCommandEvent& In)
	{
		Name = NameEdit->GetValue();
		wxDialog::AcceptAndClose();
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxDlgGroup, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgGroup::OnOK )
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxMBGroupBrowser
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The group browser menu bar.
 */
class WxMBGroupBrowser : public wxMenuBar
{
public:
	WxMBGroupBrowser()
	{
		wxMenu* GroupMenu = new wxMenu();
		wxMenu* ViewMenu = new wxMenu();

		// Edit menu
		GroupMenu->Append( IDMN_GB_NEW_GROUP, *LocalizeUnrealEd("NewE"), *LocalizeUnrealEd("ToolTip_140") );
		GroupMenu->Append( IDMN_GB_RENAME_GROUP, *LocalizeUnrealEd("RenameE"), *LocalizeUnrealEd("ToolTip_141") );
		GroupMenu->Append( IDMN_GB_DELETE_GROUP, *LocalizeUnrealEd("Delete"), *LocalizeUnrealEd("ToolTip_142") );
		GroupMenu->AppendSeparator();
		GroupMenu->Append( IDMN_GB_ADD_TO_GROUP, *LocalizeUnrealEd("AddSelectedActorsGroup"), *LocalizeUnrealEd("ToolTip_143") );
		GroupMenu->Append( IDMN_GB_DELETE_FROM_GROUP, *LocalizeUnrealEd("DeleteSelectedActorsGroup"), *LocalizeUnrealEd("ToolTip_144") );
		GroupMenu->AppendSeparator();
		GroupMenu->Append( IDMN_GB_SELECT, *LocalizeUnrealEd("SelectActors"), *LocalizeUnrealEd("ToolTip_145") );
		GroupMenu->Append( IDMN_GB_DESELECT, *LocalizeUnrealEd("DeselectActors"), *LocalizeUnrealEd("ToolTip_146") );
		GroupMenu->AppendSeparator();
		GroupMenu->Append( IDMN_GB_ALLGROUPSVISIBLE, *LocalizeUnrealEd("MakeAllGroupsVisible"), *LocalizeUnrealEd("ToolTip_146") );

		// View menu
		ViewMenu->Append( IDM_RefreshBrowser, *LocalizeUnrealEd("RefreshWithHotkey"), *LocalizeUnrealEd("ToolTip_147") );

		Append( GroupMenu, *LocalizeUnrealEd("Group") );
		Append( ViewMenu, *LocalizeUnrealEd("View") );

		WxBrowser::AddDockingMenu( this );
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxGroupBrowser
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( WxGroupBrowser, WxBrowser )
	EVT_RIGHT_DOWN( WxGroupBrowser::OnRightButtonDown )
	EVT_MENU( IDM_RefreshBrowser, WxGroupBrowser::OnRefresh )
	EVT_MENU( IDMN_GB_NEW_GROUP, WxGroupBrowser::OnNewGroup )
	EVT_MENU( IDMN_GB_DELETE_GROUP, WxGroupBrowser::OnDeleteGroup )
	EVT_MENU( IDMN_GB_SELECT, WxGroupBrowser::OnSelect )
	EVT_MENU( IDMN_GB_DESELECT, WxGroupBrowser::OnDeselect )
	EVT_MENU( IDMN_GB_ALLGROUPSVISIBLE, WxGroupBrowser::OnAllGroupsVisible )
	EVT_MENU( IDMN_GB_RENAME_GROUP, WxGroupBrowser::OnRename )
	EVT_MENU( IDMN_GB_ADD_TO_GROUP, WxGroupBrowser::OnAddToGroup )
	EVT_MENU( IDMN_GB_DELETE_FROM_GROUP, WxGroupBrowser::OnDeleteSelectedActorsFromGroups )
	EVT_CHECKLISTBOX( ID_GB_Groups, WxGroupBrowser::OnToggled )
	EVT_LISTBOX( ID_GB_Groups, WxGroupBrowser::OnGroupSelectionChange )
	EVT_LISTBOX_DCLICK( ID_GB_Groups, WxGroupBrowser::OnDoubleClick )
	EVT_LIST_COL_CLICK( ID_GB_ActorList, WxGroupBrowser::OnActorListColumnClicked )
	EVT_LIST_ITEM_ACTIVATED( ID_GB_ActorList, WxGroupBrowser::OnActorListItemClicked )
	EVT_SIZE( WxGroupBrowser::OnSize )
END_EVENT_TABLE()

WxGroupBrowser::WxGroupBrowser()
	:	bAllowToggling( TRUE )
	,	bUpdateOnActivated( FALSE )
{
	// Register our callbacks
	GCallbackEvent->Register(CALLBACK_GroupChange,this);
	GCallbackEvent->Register(CALLBACK_RefreshEditor_GroupBrowser,this);
	// Tap into the same updates as the level browser for e.g. level visibility, actor adding, etc.
	GCallbackEvent->Register(CALLBACK_RefreshEditor_LevelBrowser,this);
}

/** Clears references the actor list and references to any actors. */
void WxGroupBrowser::ClearActorReferences()
{
	ActorList->DeleteAllItems();
	ReferencedActors.Empty();
}

/** Serialize actor references held by the browser. */
void WxGroupBrowser::Serialize(FArchive& Ar)
{
	Ar << ReferencedActors;
}

void WxGroupBrowser::Send(ECallbackEventType Event)
{
	// We've received a callback that we registered for.  We're expected to refresh now.

	// Make sure we're not still referencing actors, since we may have received this event because
	// the current map is being cleared out (to load or create a new one, etc.)
	ClearActorReferences();

	// Refresh the Group Browser GUI
	Update();
}

/**
 * Forwards the call to our base class to create the window relationship.
 * Creates any internally used windows after that
 *
 * @param DockID the unique id to associate with this dockable window
 * @param FriendlyName the friendly name to assign to this window
 * @param Parent the parent of this window (should be a Notebook)
 */
void WxGroupBrowser::Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent)
{
	WxBrowser::Create(DockID,FriendlyName,Parent);

	wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		SplitterWindow = new wxSplitterWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxSP_BORDER );
		{
			GroupList = new wxCheckListBox( SplitterWindow, ID_GB_Groups, wxDefaultPosition, wxDefaultSize, 0, wxLB_EXTENDED );
			ActorList = new WxListView( SplitterWindow, ID_GB_ActorList, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
			{
				ActorList->InsertColumn( 0, *LocalizeUnrealEd("Class"), wxLIST_FORMAT_LEFT, 160 );
				ActorList->InsertColumn( 1, *LocalizeUnrealEd("Actors"), wxLIST_FORMAT_LEFT, 260 );
			}
		}
		SplitterWindow->SplitVertically( GroupList, ActorList, 100 );
		SplitterWindow->SetSashGravity( 0.1 );
		SplitterWindow->SetMinimumPaneSize( 100 );
	}
	MainSizer->Add( SplitterWindow, 1, wxEXPAND );

	MenuBar = new WxMBGroupBrowser();

	SetAutoLayout( true );
	SetSizer( MainSizer );
	MainSizer->Fit(this);
}

void WxGroupBrowser::Update()
{
	if ( IsShown() && !GIsPlayInEditorWorld && GWorld )
	{
		BeginUpdate();

		// Loops through all the actors in the world and assembles a list of unique group names.
		// Actors can belong to multiple groups by separating the group names with commas.
		TArray<FString> UniqueGroups;
		TArray<FString> GroupArray;

		for( FActorIterator It ; It ; ++It )
		{
			AActor* Actor = *It;
			if(	IsValid( Actor ) )
			{
				Actor->GetGroups( GroupArray );
				for( INT GroupIndex = 0 ; GroupIndex < GroupArray.Num() ; ++GroupIndex )
				{
					UniqueGroups.AddUniqueItem( GroupArray(GroupIndex) );
				}
			}
		}

		// Assemble the set of visible groups.
		GWorld->GetWorldInfo()->VisibleGroups.ParseIntoArray( &GroupArray, TEXT(","), FALSE );

		// Assemble a list of selected group names.
		TArray<FString> SelectedGroupNames;
		GetSelectedNames( GroupList, SelectedGroupNames );

		// Add the list of unique group names to the group listbox.
		bAllowToggling = FALSE;
		GroupList->Clear();

		Sort<USE_COMPARE_CONSTREF(FString,GroupBrowser)>( UniqueGroups.GetTypedData(), UniqueGroups.Num() );
		for( INT GroupIndex = 0 ; GroupIndex < UniqueGroups.Num() ; ++GroupIndex )
		{
			const FString& GroupName = UniqueGroups(GroupIndex);
			const INT NewGroupIndex = GroupList->Append( *GroupName );

			// Restore the group's selection status.
			const UBOOL bWasSelected = SelectedGroupNames.ContainsItem( *GroupName );
			if ( bWasSelected )
			{
				GroupList->SetSelection( NewGroupIndex );
			}

			// Set the groups visibility status.
			const UBOOL bVisible = GroupArray.ContainsItem( *GroupName );
			GroupList->Check( GroupIndex, bVisible ? true : false );
		}
		bAllowToggling = TRUE;

		UpdateActorVisibility();
		PopulateActorList();
		bUpdateOnActivated = FALSE;

		EndUpdate();
	}
	else
	{
		bUpdateOnActivated = TRUE;
	}
}

void WxGroupBrowser::Activated()
{
	WxBrowser::Activated();
	if ( bUpdateOnActivated )
	{
		Update();
	}
}

/**
 * Loops through all actors in the world and updates their visibility based on which groups are checked.
 */
void WxGroupBrowser::UpdateActorVisibility()
{
	FString VisibleGroupsList;
	TArray<FString> VisibleGroups;

	for( UINT GroupIndex = 0 ; GroupIndex < GroupList->GetCount() ; ++GroupIndex )
	{
		const UBOOL bIsChecked = GroupList->IsChecked( GroupIndex );
		if( bIsChecked )
		{
			const wxString GroupName( GroupList->GetString( GroupIndex ) );

			FString Wk( TEXT(",") );
			Wk += GroupName;
			Wk += TEXT(",");
			new( VisibleGroups )FString( Wk );

			if( VisibleGroupsList.Len() )
			{
				VisibleGroupsList += TEXT(",");
			}
			VisibleGroupsList += GroupName;
		}
		const FString GroupName( GroupList->GetString( GroupIndex ) );
		//debugf(TEXT("%s: %s"), bIsChecked ? TEXT("Y"):TEXT("N"), *GroupName );
	}

	TArray<AActor*> ActorsNeedingComponentUpdate;
	UBOOL bActorWasDeselected = FALSE;
	for( FActorIterator It ; It ; ++It )
	{
		AActor* Actor = *It;

		if(	IsValid( Actor ) )
		{
			//Actor->Modify();

			FString ActorGroup( TEXT(",") );
			ActorGroup += Actor->Group.ToString();
			ActorGroup += TEXT(",");

			INT GroupIndex;
			for( GroupIndex = 0 ; GroupIndex < VisibleGroups.Num() ; ++GroupIndex )
			{
				if( ActorGroup.InStr( VisibleGroups(GroupIndex) ) != INDEX_NONE )
				{
					//debugf(TEXT("F: %s"), *Actor->GetPathName() );
					if ( Actor->bHiddenEdGroup )
					{
						ActorsNeedingComponentUpdate.AddItem( Actor );
					}
					Actor->bHiddenEdGroup = FALSE;
					break;
				}
			}

			// If the actor isn't part of a visible group, hide and deselect it.
			if( GroupIndex == VisibleGroups.Num() )
			{
				//debugf(TEXT("T: %s"), *Actor->GetPathName() );
				if ( !Actor->bHiddenEdGroup )
				{
					ActorsNeedingComponentUpdate.AddItem( Actor );
				}
				Actor->bHiddenEdGroup = TRUE;
				GEditor->SelectActor( Actor, FALSE, NULL, FALSE, TRUE );
				bActorWasDeselected = TRUE;
			}
		}
	}

	GWorld->GetWorldInfo()->VisibleGroups = VisibleGroupsList;

	// Update components for actors whose hidden status changed.
	for ( INT ActorIndex = 0 ; ActorIndex < ActorsNeedingComponentUpdate.Num() ; ++ActorIndex )
	{
		ActorsNeedingComponentUpdate(ActorIndex)->ForceUpdateComponents( FALSE, FALSE );
	}

	if ( bActorWasDeselected )
	{
		GEditor->NoteSelectionChange();
	}
	GEditor->RedrawLevelEditingViewports();
}

class FGBActorEntry
{
public:
	FString ActorName;
	FString ClassName;
	AActor* Actor;
	FGBActorEntry() {}
	FGBActorEntry(const FString& InActorName, const FString& InClassName, AActor* InActor)
		: ActorName( InActorName )
		, ClassName( InClassName )
		, Actor( InActor )
	{}
};

/** If TRUE, sort by ActorClassName.  If FALSE, sort by OuterName.ActorName. */
static UBOOL bSortByClass = FALSE;
/** Controls whether or not to sort ascending/descending (1/-1). */
static INT SortAscending = -1;

IMPLEMENT_COMPARE_CONSTREF( FGBActorEntry, GroupBrowser, \
{ \
	const INT NameResult = (bSortByClass ? 1 : SortAscending) * appStricmp(*A.ActorName,*B.ActorName); \
	const INT ClassResult = (bSortByClass ? SortAscending : 1) * appStricmp(*A.ClassName,*B.ClassName); \
	if ( bSortByClass ) \
		return ClassResult == 0 ? NameResult : ClassResult; \
	else \
		return NameResult == 0 ? ClassResult : NameResult; \
} );

/** Populates the actor list with actors in the selected groups. */
void WxGroupBrowser::PopulateActorList()
{
	// Assemble a list of selected group names.
	TArray<FString> SelectedGroupNames;
	GetSelectedNames( GroupList, SelectedGroupNames );

	ActorList->Freeze();
	ClearActorReferences();
	if ( SelectedGroupNames.Num() > 0 )
	{
		TArray<FGBActorEntry> ActorNames;
		for( FActorIterator It ; It ; ++It )
		{
			AActor* Actor = *It;
			if( IsValid( Actor ) )
			{
				for ( INT GroupIndex = 0 ; GroupIndex < SelectedGroupNames.Num() ; ++GroupIndex )
				{
					if ( Actor->IsInGroup( *SelectedGroupNames(GroupIndex) ) )
					{
						const UObject* StopOuter = Actor->GetOuter() ? Actor->GetOuter()->GetOuter() : NULL;
						ActorNames.AddItem( FGBActorEntry(Actor->GetPathName( StopOuter ), Actor->GetClass()->GetName(), Actor) );
						break;
					}
				}
			}
		}
		Sort<USE_COMPARE_CONSTREF(FGBActorEntry,GroupBrowser)>( ActorNames.GetTypedData(), ActorNames.Num() );
		for ( INT ActorIndex = 0 ; ActorIndex < ActorNames.Num() ; ++ActorIndex )
		{
			FGBActorEntry& Actor = ActorNames(ActorIndex);
			const INT ListIndex = ActorList->InsertItem( 0, *Actor.ClassName );
			ActorList->SetItem( ListIndex, 1, *Actor.ActorName );
			ActorList->SetItemData( ListIndex, (LONG)(void*)Actor.Actor );
			ReferencedActors.AddItem( Actor.Actor );
		}
	}
	ActorList->Thaw();
}

/** Called when the tile of a column in the actor list is clicked. */
void WxGroupBrowser::OnActorListColumnClicked(wxListEvent& In)
{
	const int Column = In.GetColumn();
	if( Column > -1 )
	{
		const UBOOL bOldSortByClass = bSortByClass;
		bSortByClass = Column == 0 ? TRUE : FALSE;
		if ( bSortByClass == bOldSortByClass )
		{
			// The user clicked on the column already used for sort,
			// so toggle ascending/descending sort.
			SortAscending = -SortAscending;
		}
		else
		{
			// The user clicked on a new column, so set default ordering.
			SortAscending = -1;
		}

		PopulateActorList();
	}
}

/** Called when an actor in the actor list is clicked. */
void WxGroupBrowser::OnActorListItemClicked(wxListEvent& In)
{
	// Assemble a list of selected group names.
	TArray<AActor*> SelectedActors;
	GetSelectedActors( ActorList, SelectedActors );

	const FScopedTransaction* Transaction = NULL;
	UBOOL bSelectedActor = FALSE;
	for ( INT ActorIndex = 0 ; ActorIndex < SelectedActors.Num() ; ++ActorIndex )
	{
		AActor* Actor = SelectedActors(ActorIndex);
		if ( Actor )
		{
			if ( !bSelectedActor )
			{
				Transaction = new FScopedTransaction( *LocalizeUnrealEd("ActorSearchGoto") );
				GEditor->SelectNone( FALSE, TRUE );
				bSelectedActor = TRUE;
			}
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );
		}
	}
	if ( bSelectedActor )
	{
		GEditor->NoteSelectionChange();
		GEditor->Exec( TEXT("CAMERA ALIGN") );
		delete Transaction;
	}
}

/**
 * Selects/deselects actors by group.
 * @return		TRUE if at least one actor was selected.
 */
UBOOL WxGroupBrowser::SelectActors(UBOOL InSelect)
{
	// Assemble a list of selected group names.
	TArray<FString> SelectedGroupNames;
	GetSelectedNames( GroupList, SelectedGroupNames );

	// Select actors belonging to the selected groups.
	const UBOOL bResult = SelectActorsInGroups( SelectedGroupNames, InSelect );
	if ( bResult )
	{
		GUnrealEd->NoteSelectionChange();
	}
	return bResult;
}

/** Called when a list item check box is toggled. */
void WxGroupBrowser::OnToggled( wxCommandEvent& In )
{
	if( bAllowToggling )
	{
		UpdateActorVisibility();
	}
}

/** Called when item selection in the group list changes. */
void WxGroupBrowser::OnGroupSelectionChange( wxCommandEvent& In )
{
	PopulateActorList();
}

/** Called when a list item in the group browser is double clicked. */
void WxGroupBrowser::OnDoubleClick( wxCommandEvent& In )
{
	// Remove all existing selections.
	wxArrayInt Selections;
	const INT NumSelections = GroupList->GetSelections( Selections );

	for( INT x = 0 ; x < NumSelections ; ++x )
	{
		GroupList->Deselect( x );
	}

	// Make as visible the group that was double clicked.
	const INT SelectionIndex = In.GetSelection();
	if ( !GroupList->IsChecked(SelectionIndex) )
	{
		GroupList->Check( SelectionIndex, true );
		UpdateActorVisibility();
	}

	// Select the group that was double clicked.
	GroupList->Select( SelectionIndex );
	PopulateActorList();

	// Select the actors in the newly selected group.
	GEditor->SelectNone( FALSE, TRUE );
	if ( !SelectActors( TRUE ) )
	{
		// Select actors returned FALSE, meaning that no actors were selected.
		// So, call NoteSelectionChange to cover the above SelectNone operation.
		GUnrealEd->NoteSelectionChange();
	}
}

/** Sets all groups to visible. */
void WxGroupBrowser::OnAllGroupsVisible( wxCommandEvent& In )
{
	if( bAllowToggling )
	{
		for( UINT GroupIndex = 0 ; GroupIndex < GroupList->GetCount() ; ++GroupIndex )
		{
			GroupList->Check( GroupIndex, true );
		}
		UpdateActorVisibility();
	}
}

/** Creates a new group. */
void WxGroupBrowser::OnNewGroup( wxCommandEvent& In )
{
	if( NumActorsSelected() == 0 )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MustSelectActorsToCreateGroup") );
		return;
	}

	// Generate a suggested group name to use as a default.
	FString DefaultName;

	INT GroupIndex = 1;
	do
	{
		DefaultName = FString::Printf(LocalizeSecure(LocalizeUnrealEd("Group_F"), GroupIndex++));
	} while ( GroupList->FindString( *DefaultName ) != -1 );

	WxDlgGroup dlg;
	if( dlg.ShowModal( TRUE, DefaultName ) == wxID_OK )
	{
		if( GWorld->GetWorldInfo()->VisibleGroups.Len() )
		{
			GWorld->GetWorldInfo()->VisibleGroups += TEXT(",");
		}
		GWorld->GetWorldInfo()->VisibleGroups += dlg.GetObjectName();

		AddSelectedActorsToGroup( dlg.GetObjectName() );

		GUnrealEd->UpdatePropertyWindows();
		Update();
	}
}

/** Deletes a group. */
void WxGroupBrowser::OnDeleteGroup( wxCommandEvent& In )
{
	// Assemble a list of selected group names.
	TArray<FString> SelectedGroupNames;
	GetSelectedNames( GroupList, SelectedGroupNames );

	if ( SelectedGroupNames.Num() > 0 )
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("DeleteGroup")) );

		for( FActorIterator It ; It ; ++It )
		{
			AActor* Actor = *It;
			if( IsValid( Actor ) )
			{
				UBOOL bActorAlreadyModified = TRUE;		// Flag to ensure that Actor->Modify is only called once.
				for ( INT GroupIndex = 0 ; GroupIndex < SelectedGroupNames.Num() ; ++GroupIndex )
				{
					bActorAlreadyModified = DeleteActorFromGroup( Actor, SelectedGroupNames(GroupIndex), bActorAlreadyModified );
				}
			}
		}

		GUnrealEd->UpdatePropertyWindows();
		Update();
	}
}

/** Handler for IDM_RefreshBrowser events; updates the browser contents. */
void WxGroupBrowser::OnRefresh( wxCommandEvent& In )
{
	Update();
}

/** Selects actors in the selected groups. */
void WxGroupBrowser::OnSelect( wxCommandEvent& In )
{
	SelectActors( TRUE );
}

/** Deselects actors in the selected groups. */
void WxGroupBrowser::OnDeselect( wxCommandEvent& In )
{
	SelectActors( FALSE );
}

/** Presents a rename dialog for each selected group. */
void WxGroupBrowser::OnRename( wxCommandEvent& In )
{
	// Assemble a list of selected group indices.
	TArray<INT> SelectedGroupIndices;
	GetSelectedGroupIndices( GroupList, SelectedGroupIndices );

	// Assemble a list of all group names.
	TArray<FString> AllGroupNames;
	GetAllGroupNames( GroupList, AllGroupNames );

	UBOOL bRenamedGroup = FALSE;

	// For each selected group, display a rename dialog.
	WxDlgGroup dlg;
	for( INT GroupIndex = 0 ; GroupIndex < SelectedGroupIndices.Num() ; ++GroupIndex )
	{
		const FString& CurGroupName = AllGroupNames( SelectedGroupIndices(GroupIndex) );
		if( dlg.ShowModal( FALSE, CurGroupName ) == wxID_OK )
		{
			const FString& NewGroupName = dlg.GetObjectName();
			if ( CurGroupName != NewGroupName )
			{
				// If the name already exists, notify the user and request a new name.
				if ( AllGroupNames.ContainsItem( NewGroupName ) )
				{
					appMsgf( AMT_OK, *LocalizeUnrealEd("Error_RenameFailedGroupAlreadyExists") );
					--GroupIndex;
				}
				else
				{
					const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("RenameGroup")) );

					// Iterate over all actors, swapping groups.
					for( FActorIterator It ; It ; ++It )
					{
						AActor* Actor = *It;
						if( IsValid( Actor ) )
						{
							if ( Actor->IsInGroup( *CurGroupName ) )
							{
								DeleteActorFromGroup( Actor, CurGroupName, TRUE );
								AddActorToGroup( Actor, NewGroupName, TRUE );
							}
						}
					}

					// Mark the new group as visible.
					if( GWorld->GetWorldInfo()->VisibleGroups.Len() )
					{
						GWorld->GetWorldInfo()->VisibleGroups += TEXT(",");
					}
					GWorld->GetWorldInfo()->VisibleGroups += dlg.GetObjectName();

					// Flag an update.
					bRenamedGroup = TRUE;
				}
			}
		}
	}

	if ( bRenamedGroup )
	{
		GUnrealEd->UpdatePropertyWindows();
		Update();
	}
}

/** Adds level-selected actors to the selected groups. */
void WxGroupBrowser::OnAddToGroup( wxCommandEvent& In )
{
	// Assemble a list of selected group names.
	TArray<FString> SelectedGroupNames;
	GetSelectedNames( GroupList, SelectedGroupNames );

	// Add selected actors to the selected groups.
	if ( AddSelectedActorsToGroups( SelectedGroupNames ) )
	{
		PopulateActorList();
		GUnrealEd->UpdatePropertyWindows();
	}
}

/** Deletes level-selected actors from the selected groups. */
void WxGroupBrowser::OnDeleteSelectedActorsFromGroups( wxCommandEvent& In )
{
	// Assemble a list of selected group names.
	TArray<FString> SelectedGroupNames;
	GetSelectedNames( GroupList, SelectedGroupNames );

	// Remove selected actors from the selected groups.
	if ( DeleteSelectedActorsFromGroups( SelectedGroupNames ) )
	{
		GUnrealEd->UpdatePropertyWindows();
		Update();
	}
}

/** Returns the key to use when looking up values. */
const TCHAR* WxGroupBrowser::GetLocalizationKey() const
{
	return TEXT("GroupBrowser");
}

/** Responds to size events by updating the splitter. */
void WxGroupBrowser::OnSize(wxSizeEvent& In)
{
	// During the creation process a sizing message can be sent so don't
	// handle it until we are initialized
	if ( bAreWindowsInitialized )
	{
		SplitterWindow->SetSize( GetClientRect() );
	}
}

/** Presents a context menu for group list items. */
void WxGroupBrowser::OnRightButtonDown( wxMouseEvent& In )
{
	// @todo DB: add 'Show in Scene Manager' functionality.
	/*
		class WxLevelPopupMenu : public wxMenu
	{
	public:
		WxLevelPopupMenu(ULevel* Level)
		{
			INT NumItems = 0;

			// Don't offer the option to load/unload if the level is locked.
			if ( !FLevelUtils::IsLevelLocked( Level ) )
			{
				Append( IDM_LB_MakeCurrentLevel, *LocalizeUnrealEd("MakeCurrent"), TEXT("") );
				++NumItems;
			}

			if ( NumItems )
			{
				AppendSeparator();
				NumItems = 0;
			}
			Append( IDM_LB_ShowSelectedLevelsInSceneManager, *LocalizeUnrealEd("ShowSelectedLevelsInSceneManager"), TEXT("") );
			Append( IDM_LB_SelectAllActors, *LocalizeUnrealEd("SelectAllActors"), TEXT("") );
			++NumItems;

			if ( NumItems )
			{
				AppendSeparator();
				NumItems = 0;
			}
			Append( IDM_LB_SaveSelectedLevels, *LocalizeUnrealEd("SaveSelectedLevels"), TEXT("") );
			++NumItems;

			AppendSeparator();

			// Don't offer the option to load/unload or edit properties if this is the persistent level.
			if ( Level != GWorld->PersistentLevel )
			{
				Append( IDM_LB_LevelProperties, *LocalizeUnrealEd("Properties"), TEXT("") );
			}

			// Self contained lighting check.
			AppendCheckItem( IDM_LB_SelfContainedLighting, *LocalizeUnrealEd("SelfContainedLighting") );
			const UBOOL bHasSelfContainedLighting = FLevelUtils::HasSelfContainedLighting( Level );
			Check( IDM_LB_SelfContainedLighting, bHasSelfContainedLighting == TRUE );

			// Operations based on selected levels.
			AppendSeparator();
			Append( IDM_LB_ShowSelectedLevels, *LocalizeUnrealEd("ShowSelectedLevels"), TEXT("") );
			Append( IDM_LB_HideSelectedLevels, *LocalizeUnrealEd("HideSelectedLevels"), TEXT("") );
			AppendSeparator();
			Append( IDM_LB_ShowOnlySelectedLevels, *LocalizeUnrealEd("ShowOnlySelectedLevels"), TEXT("") );
			Append( IDM_LB_ShowOnlyUnselectedLevels, *LocalizeUnrealEd("ShowOnlyUnselectedLevels"), TEXT("") );
			AppendSeparator();
			Append( IDM_LB_ShowAllLevels, *LocalizeUnrealEd("ShowAllLevels"), TEXT("") );
			Append( IDM_LB_HideAllLevels, *LocalizeUnrealEd("HideAllLevels"), TEXT("") );
			AppendSeparator();
			Append( IDM_LB_SelectAllLevels, *LocalizeUnrealEd("SelectAllLevels"), TEXT("") );
			Append( IDM_LB_DeselectAllLevels, *LocalizeUnrealEd("DeselectAllLevels"), TEXT("") );
			Append( IDM_LB_InvertLevelSelection, *LocalizeUnrealEd("InvertLevelSelection"), TEXT("") );

			// For options only presented when actors are selected.
			UBOOL bActorSelected = FALSE;
			UBOOL bStreamingLevelVolumeSelected = FALSE;
			for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
			{
				bActorSelected = TRUE;
				if ( Cast<ALevelStreamingVolume>( *It ) )
				{
					bStreamingLevelVolumeSelected = TRUE;
					break;
				}
			}

			AppendSeparator();
			// If any level streaming volumes are selected, present the option to associate the volumes with selected levels.
			if ( bStreamingLevelVolumeSelected )
			{
				Append( IDM_LB_AddSelectedLevelsToLevelStreamingVolumes, *LocalizeUnrealEd("AddStreamingVolumes"), TEXT("") );
				Append( IDM_LB_SetSelectedLevelsToLevelStreamingVolumes, *LocalizeUnrealEd("SetStreamingVolumes"), TEXT("") );
			}
			Append( IDM_LB_ClearLevelStreamingVolumeAssignments, *LocalizeUnrealEd("ClearStreamingVolumes"), TEXT("") );
			Append( IDM_LB_SelectAssociatedStreamingVolumes, *LocalizeUnrealEd("SelectAssociatedStreamingVolumes"), TEXT("") );

			// If any actors are selected, present the option to move actors between levels.
			if ( bActorSelected )
			{
				AppendSeparator();
				Append( IDM_LB_MoveSelectedActorsToCurrentLevel, *LocalizeUnrealEd("MoveSelectedActorsToCurrentLevel"), TEXT("") );
			}
		}
	};

	WxLevelPopupMenu Menu( Level );
	FTrackPopupMenu tpm( this, &Menu );
	tpm.Show();
	*/
}
