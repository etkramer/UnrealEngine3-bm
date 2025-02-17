/*=============================================================================
	DlgColladaResource.cpp: Dialog used by the COLLADA importer.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgColladaResource.h"

WxDlgCOLLADAResourceList::WxDlgCOLLADAResourceList()
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_COLLADA_RESOURCE_LIST") );
	check( bSuccess );

	ResourceList = (wxCheckListBox*)FindWindow( XRCID( "IDCLB_RESOURCE_LIST_BOX" ) );
	check( ResourceList );

	FWindowUtil::LoadPosSize( TEXT("DlgCOLLADAResourceList"), this );
	wxRect ThisRect = GetRect();
	ThisRect.width = 300;
	ThisRect.height = 500;
	SetSize( ThisRect );
	Layout();
	FLocalizeWindow( this );

	ADDEVENTHANDLER( XRCID("ID_SELECT_ALL"), wxEVT_COMMAND_BUTTON_CLICKED, &WxDlgCOLLADAResourceList::OnSelectAll );
	ADDEVENTHANDLER( XRCID("ID_SELECT_NONE"), wxEVT_COMMAND_BUTTON_CLICKED, &WxDlgCOLLADAResourceList::OnSelectNone );
}

WxDlgCOLLADAResourceList::~WxDlgCOLLADAResourceList()
{
	FWindowUtil::SavePosSize( TEXT("DlgCOLLADAResourceList"), this );
}

/**
 * @param	ResourceNames		The set of resource names with which to populate the dialog.
 */
void WxDlgCOLLADAResourceList::FillResourceList(const TArray<const TCHAR*>& ResourceNames)
{
	ResourceList->Freeze();
	ResourceList->Clear();
	{
		for( INT i = ResourceNames.Num()-1 ; i >= 0 ; --i )
		{
			const TCHAR* ResourceName = ResourceNames(i);
			ResourceList->Append( ResourceName );
		}
	}
	ResourceList->Thaw();
}

/**
 * @param	OutSelectedResources		The set of resource names currently selected in the dialog.
 */
void WxDlgCOLLADAResourceList::GetSelectedResources(TArray<FString>& OutSelectedResources)
{
	OutSelectedResources.Empty();
	for( UINT Index = 0 ; Index < ResourceList->GetCount() ; ++Index )
	{
		const UBOOL bItemChecked = ResourceList->IsChecked( Index );
		if( bItemChecked )
		{
			OutSelectedResources.AddItem( FString( ResourceList->GetString( Index ) ) );
		}
	}
}

void WxDlgCOLLADAResourceList::OnSelectAll(wxCommandEvent& In)
{
	ResourceList->Freeze();
	for( UINT Index = 0 ; Index < ResourceList->GetCount() ; ++Index )
	{
		ResourceList->Check( Index, true );
	}
	ResourceList->Thaw();
}

void WxDlgCOLLADAResourceList::OnSelectNone(wxCommandEvent& In)
{
	ResourceList->Freeze();
	for( UINT Index = 0 ; Index < ResourceList->GetCount() ; ++Index )
	{
		ResourceList->Check( Index, false );
	}
	ResourceList->Thaw();
}
