/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGACTORSEARCH_H__
#define __DLGACTORSEARCH_H__

/**
* wxWidgets timer ID
*/
#define ACTORSEARCH_TIMER_ID 9998

/** delay after a new character is entered into the search dialog to wait before updating the list (to give them time to enter a whole string instead of useless updating every time a char is put in) **/
#define SEARCH_TEXT_UPDATE_DELAY 750

class WxDlgActorSearch : public wxDialog, public FSerializableObject
{
public:
	WxDlgActorSearch( wxWindow* InParent );
	virtual ~WxDlgActorSearch();

	/**
	 * Empties the search list, releases actor references, etc.
	 */
	void Clear();

	/** Updates the results list using the current filter and search options set. */
	void UpdateResults();

	/** Serializes object references to the specified archive. */
	virtual void Serialize(FArchive& Ar);

	class FActorSearchOptions
	{
	public:
		FActorSearchOptions()
			: Column( 0 )
			, bSortAscending( TRUE )
		{}
		/** The column currently being used for sorting. */
		int Column;
		/** Denotes ascending/descending sort order. */
		UBOOL bSortAscending;
	};

protected:
	wxTextCtrl *SearchForEdit;
	wxRadioButton *StartsWithRadio;
	wxComboBox *InsideOfCombo;
	wxListCtrl *ResultsList;
	wxStaticText *ResultsLabel;

	TArray<AActor*> ReferencedActors;

	FActorSearchOptions SearchOptions;

	/** timer to handle updating of search items **/
	wxTimer _updateResultsTimer;

	/** Wx Event Handlers */
	void OnTimer( wxTimerEvent& event );
	void OnSearchTextChanged( wxCommandEvent& In );
	void OnColumnClicked( wxListEvent& In );
	void OnItemActivated( wxListEvent& In );
	void OnGoTo( wxCommandEvent& In );
	void OnDelete( wxCommandEvent& In );
	void OnProperties( wxCommandEvent& In );

	/** Refreshes the results list when the window gets focus. */
	void OnActivate( wxActivateEvent& In);

	DECLARE_EVENT_TABLE()
};

#endif // __DLGACTORSEARCH_H__
