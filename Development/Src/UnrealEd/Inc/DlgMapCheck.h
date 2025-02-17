/*=============================================================================
	DlgMapCheck.h: UnrealEd dialog for displaying map errors.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGMAPCHECK_H__
#define __DLGMAPCHECK_H__

#include "TrackableWindow.h"

/**
 * Dialog that displays map errors and allows the user to focus on and delete
 * any actors associated with those errors.
 */
class WxDlgMapCheck : public WxTrackableDialog, public FSerializableObject
{
public:
	WxDlgMapCheck(wxWindow* InParent);
	~WxDlgMapCheck();

	/**
	 * This function is called when the window has been selected from within the ctrl + tab dialog.
	 */
	virtual void OnSelected();

	/**
	 * Shows the dialog only if there are messages to display.
	 */
	void ShowConditionally();

	/**
	 * Clears out the list of messages appearing in the window.
	 */
	void ClearMessageList();

	/**
	 * Freezes the message list.
	 */
	void FreezeMessageList();

	/**
	 * Thaws the message list.
	 */
	void ThawMessageList();

	/**
	 * Adds a message to the map check dialog, to be displayed when the dialog is shown.
	 *
	 * @param	InType					The type of message.
	 * @param	InActor					Actor associated with the message; can be NULL.
	 * @param	InMessage				The message to display.
	 * @param	InRecommendedAction		The recommended course of action to take.
	 * @param	InUDNPage				UDN Page to visit if the user needs more info on the warning.  This will send the user to https://udn.epicgames.com/Three/MapErrors#InUDNPage. 
	 */
	static void AddItem(MapCheckType InType, AActor* InActor, const TCHAR* InMessage, MapCheckAction InRecommendedAction, const TCHAR* InUDNPage=TEXT(""));

	virtual void Serialize(FArchive& Ar);

	/**
	* Loads the list control with the contents of the GErrorWarningInfoList array.
	*/
	void LoadListCtrl();

	virtual bool Show( bool show = true );

	/** Struct that holds information about a entry in the ErrorWarningList. */
	struct FErrorWarningInfo
	{
		/** The actor that is related to this error/warning.  Can be NULL if not relevant. */
		AActor* Actor;

		/** What type of error/warning this is */
		MapCheckType Type;

		/** The message we want to display to the LD. */
		FString Message;

		/** The UDN page where help can be found for this error/warning. */
		FString UDNPage;

		/** Recommended way to deal with this error/warning. */
		MapCheckAction RecommendedAction;

		/** UDN Page that describes the warning in detail. */
		FString	UDNHelpString;
	};

protected:
	wxListCtrl*					ErrorWarningList;
	wxImageList*				ImageList;
	TArray<AActor*>				ReferencedActors;

private:

	/**
	 * Removes all items from the map check dialog that pertain to the specified actor.
	 *
	 * @param	Actor	The actor to match when removing items.
	 */
	void RemoveActorItems(AActor* Actor);

	/** Event handler for when the close button is clicked on. */
	void OnClose(wxCommandEvent& In);

	/** Event handler for when the refresh button is clicked on. */
	void OnRefresh(wxCommandEvent& In);

	/** Event handler for when the goto button is clicked on. */
	void OnGoTo(wxCommandEvent& In);

	/** Event handler for when the delete button is clicked on. */
	void OnDelete(wxCommandEvent& In);

	/** Event handler for when the "delete all" button is clicked on. */
	void OnDeleteAll(wxCommandEvent& In);

	/** Event handler for when the "Show Help" button is clicked on. */
	void OnShowHelpPage(wxCommandEvent& In);

	/** Event handler for when a message is clicked on. */
	void OnItemActivated(wxListEvent& In);

	/** Event handler for when wx wants to update UI elements. */
	void OnUpdateUI(wxUpdateUIEvent& In);

	DECLARE_EVENT_TABLE()
};

static TArray<WxDlgMapCheck::FErrorWarningInfo> GErrorWarningInfoList;

#endif // __DLGMAPCHECK_H__
