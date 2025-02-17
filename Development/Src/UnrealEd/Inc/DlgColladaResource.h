/*=============================================================================
	DlgColladaResource.h: Dialog used by the COLLADA importer.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGCOLLADARESOURCE_H__
#define __DLGCOLLADARESOURCE_H__

/**
 * Used by the COLLADA importer.  Populated with the set of resources in
 * a COLLADA file.  Allows the user to select the resources to import.
 */
class WxDlgCOLLADAResourceList : public wxDialog
{
public:
	WxDlgCOLLADAResourceList();
	virtual ~WxDlgCOLLADAResourceList();

	/**
	 * @param	ResourceNames		The set of resource names with which to populate the dialog.
	 */
	void FillResourceList(const TArray<const TCHAR*>& ResourceNames);

	/**
	 * @param	OutSelectedResources		The set of resource names currently selected in the dialog.
	 */
	void GetSelectedResources(TArray<FString>& OutSelectedResources);

private:
	wxCheckListBox* ResourceList;

	void OnSelectAll(wxCommandEvent& In);
	void OnSelectNone(wxCommandEvent& In);
};

#endif // __DLGCOLLADARESOURCE_H__
