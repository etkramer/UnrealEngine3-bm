/*=============================================================================
	DlgGenericBrowserSearch.h: Generic Browser search dialog.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGGENERICBROWSERSEARCH_H__
#define __DLGGENERICBROWSERSEARCH_H__

/**
 * Generic Browser search dialog.
 */
class WxDlgGenericBrowserSearch : public wxDialog
{
public:
	WxDlgGenericBrowserSearch();
	virtual ~WxDlgGenericBrowserSearch();

	/**
	 * Updates the contents of the results list.
	 */
	void UpdateResults();

private:
	wxTextCtrl*				SearchForEdit;
	wxRadioButton*			StartsWithRadio;
	wxListCtrl*				ResultsList;
	wxStaticText*			ResultsLabel;
	FListViewSortOptions	SearchOptions;

	void OnSearchTextChanged(wxCommandEvent& In);
	void OnColumnClicked(wxListEvent& In);
	void OnItemActivated(wxListEvent& In);
};

#endif // __DLGGENERICBROWSERSEARCH_H__
