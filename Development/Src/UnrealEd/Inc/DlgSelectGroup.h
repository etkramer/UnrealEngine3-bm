/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DIALOGSELECTGROUP_H__
#define __DIALOGSELECTGROUP_H__

class WxDlgSelectGroup : public wxDialog
{
public:
	WxDlgSelectGroup( wxWindow* InParent );

	FString			Package, Group;
	UBOOL			bShowAllPackages;
	UPackage*		RootPackage;
	wxTreeCtrl*		TreeCtrl;

	void UpdateTree();
	void AddChildren( UPackage* InPackage, wxTreeItemId InId );

	int ShowModal( FString InPackageName, FString InGroupName );

private:
	void OnOK( wxCommandEvent& In );
	void OnShowAllPackages( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

#endif // __DIALOGSELECTGROUP_H__
