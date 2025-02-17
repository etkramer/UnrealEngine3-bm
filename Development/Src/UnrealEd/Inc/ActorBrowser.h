/*=============================================================================
	ActorBrowser.h: UnrealEd's actor browser.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __ACTORBROWSER_H__
#define __ACTORBROWSER_H__

class WxActorBrowser : public WxBrowser
{
	DECLARE_DYNAMIC_CLASS(WxActorBrowser);

public:
	WxActorBrowser();
	~WxActorBrowser();

	/**
	 * Forwards the call to our base class to create the window relationship.
	 * Creates any internally used windows after that
	 *
	 * @param DockID the unique id to associate with this dockable window
	 * @param FriendlyName the friendly name to assign to this window
	 * @param Parent the parent of this window (should be a Notebook)
	 */
	virtual void Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent);

	virtual void Update();
	virtual void Activated();

	void UpdateActorList();
	void AddChildren( wxTreeItemId InID );
	void UpdatePackageList();

	/**
	 * Returns the key to use when looking up values
	 */
	virtual const TCHAR* GetLocalizationKey(void) const
	{
		return TEXT("ActorBrowser");
	}

protected:
	wxCheckBox *ActorAsParentCheck, *PlaceableCheck;
	wxTreeCtrl *TreeCtrl;
	wxStaticText *FullPathStatic;
	wxListBox *PackagesList;
	UClass* RightClickedClass;		/** The class that was last right clicked. */

	class FClassTree* ClassTree;

	/**
	 * TRUE if an update was requested while the browser tab wasn't active.
	 * The browser will Update() the next time this browser tab is Activated().
	 */
	UBOOL					bUpdateOnActivated;

private:
	void OnFileOpen( wxCommandEvent& In );
	void OnFileExportAll( wxCommandEvent& In );
	void OnItemExpanding( wxCommandEvent& In );
	void OnSelChanged( wxCommandEvent& In );
	void OnViewSource( wxCommandEvent& In );
	/** Creates an archteype from the actor class selected in the Actor Browser. */
	void OnCreateArchetype( wxCommandEvent& In );
	void OnItemRightClicked( wxCommandEvent& In );
	void OnUseActorAsParent( wxCommandEvent& In );
	void OnPlaceableClassesOnly( wxCommandEvent& In );

	DECLARE_EVENT_TABLE();
};

#endif // __ACTORBROWSER_H__
