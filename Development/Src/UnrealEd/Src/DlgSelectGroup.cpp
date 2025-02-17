/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgSelectGroup.h"

BEGIN_EVENT_TABLE(WxDlgSelectGroup, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgSelectGroup::OnOK )
END_EVENT_TABLE()

WxDlgSelectGroup::WxDlgSelectGroup( wxWindow* InParent )
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, InParent, TEXT("ID_DLG_SELECTGROUP") );
	check( bSuccess );

	TreeCtrl = wxDynamicCast( FindWindow( XRCID( "IDTC_GROUPS" ) ), wxTreeCtrl );
	check( TreeCtrl != NULL );

	bShowAllPackages = TRUE;
	ADDEVENTHANDLER( XRCID("IDCK_SHOW_ALL"), wxEVT_COMMAND_CHECKBOX_CLICKED , &WxDlgSelectGroup::OnShowAllPackages );

	FLocalizeWindow( this );
}

int WxDlgSelectGroup::ShowModal( FString InPackageName, FString InGroupName )
{
	// Find the package objects

	RootPackage = Cast<UPackage>(GEngine->StaticFindObject( UPackage::StaticClass(), ANY_PACKAGE, *InPackageName ) );

	if( RootPackage == NULL )
	{
		bShowAllPackages = TRUE;
	}

	UpdateTree();

	return wxDialog::ShowModal();
}

/**
* Loads the tree control with a list of groups and/or packages.
*/

void WxDlgSelectGroup::UpdateTree()
{
	UPackage* Root = RootPackage;
	if( bShowAllPackages )
	{
		Root = NULL;
	}

	// Load tree control with the groups associated with this package.

	TreeCtrl->DeleteAllItems();

	const wxTreeItemId id = TreeCtrl->AddRoot( Root ? *Root->GetName() : TEXT("Packages"), -1, -1, new WxTreeObjectWrapper( Root ) );

	AddChildren( Root, id );

	TreeCtrl->SortChildren( id );
	TreeCtrl->Expand( id );
}

/**
* Adds the child packages of InPackage to the tree.
*
* @param	InPackage	The package to add children of
* @param	InId		The tree item id to add the children underneath
*/

void WxDlgSelectGroup::AddChildren( UPackage* InPackage, wxTreeItemId InId )
{
	for( TObjectIterator<UPackage> It ; It ; ++It )
	{
		if( It->GetOuter() == InPackage )
		{
			UPackage* pkg = *It;
			wxTreeItemId id = TreeCtrl->AppendItem( InId, *It->GetName(), -1, -1, new WxTreeObjectWrapper( pkg ) );

			AddChildren( pkg, id );
			TreeCtrl->SortChildren( id );
		}
	}
}

void WxDlgSelectGroup::OnOK( wxCommandEvent& In )
{
	WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*) TreeCtrl->GetItemData(TreeCtrl->GetSelection());

	if( ItemData )
	{
		UPackage* pkg = ItemData->GetObjectChecked<UPackage>();
		FString Prefix;

		Group = TEXT("");

		while( pkg->GetOuter() )
		{
			Prefix = pkg->GetName();
			if( Group.Len() )
			{
				Prefix += TEXT(".");
			}

			Group = Prefix + Group;

			pkg = (UPackage*)(pkg->GetOuter());
		}

		Package = pkg->GetName();
	}
	wxDialog::AcceptAndClose();
}

void WxDlgSelectGroup::OnShowAllPackages( wxCommandEvent& In )
{
	bShowAllPackages = In.IsChecked();

	UpdateTree();
}
