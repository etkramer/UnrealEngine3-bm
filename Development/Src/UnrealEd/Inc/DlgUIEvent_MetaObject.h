/*=============================================================================
	DlgUIEvent_MetaObject.h : Specialized dialog for the UIEvent_MetaObject sequence object.  Lets the user bind keys to the object, exposing them to kismet.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGUIEVENT_METAOBJECT_H__
#define __DLGUIEVENT_METAOBJECT_H__

/**
  * Specialized dialog for the UIEvent_MetaObject sequence object.  Lets the user bind keys to the object, exposing them to kismet.
  */
class WxDlgUIEvent_MetaObject : public wxDialog
{
public:

	/** This is the client data type used by the list views, it stores the data needed to sort the lists. */
	struct FSortData
	{
		FName KeyName;
		FName EventName;
	};

	/** Enums for the fields of the listview. */
	enum EFields
	{
		Field_Key,
		Field_Event
	};

	/** Options for the sorting process, tells whether to sort ascending or descending and which column to sort by. */
	struct FSortOptions
	{
		UBOOL bSortAscending;
		INT Column;
	};

	WxDlgUIEvent_MetaObject(const class WxKismet* InEditor, USequenceObject* InObject);


protected:

	/** Adds all of the keys available for binding to the available list. */
	void AddAvailableKeys();

	/** Moves selected items from the source list to the destlist. */
	void ListMove(WxListView* SourceList, WxListView* DestList);

	/** When the user clicks the OK button, we create/remove output links for the sequence object based on the state of the lists. */
	void OnOK(wxCommandEvent &Event);

	/** Moves selected items from the bound list to the available list. */
	void OnShiftLeft(wxCommandEvent &Event);

	/** Moves selected items from the available list to the bound list. */
	void OnShiftRight(wxCommandEvent &Event);

	/** Moves selected items from the bound list to the available list. */
	void OnBoundListActivated(wxListEvent &Event);

	/** Moves selected items from the available list to the bound list. */
	void OnAvailableListActivated(wxListEvent &Event);

	/** Sorts the list based on which column was clicked. */
	void OnClickListColumn(wxListEvent &Event);

	/** List of available keys. */
	WxListView* ListAvailable;

	/** List of bound keys. */
	WxListView* ListBound;

	/** Reference to the object we are modifying. */
	USequenceObject* SeqObj;

	/** Pointer to the parent editor of this dialog. */
	const class WxKismet* Editor;

	/** A list of keys and their supported input events. */
	TArray<FSortData> KeyActions;

	/** Options for sorting the lists, one for each list. */
	FSortOptions SortOptions[2];

	DECLARE_EVENT_TABLE()
};


#endif

