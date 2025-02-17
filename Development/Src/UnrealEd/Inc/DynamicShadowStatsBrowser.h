/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DYNAMICSHADOWSTATSBROWSER_H__
#define __DYNAMICSHADOWSTATSBROWSER_H__

/** 
 * Enumeration of columns.
 */
enum EDynamicShadowStatsBrowserColumns
{
	DSSBC_Light			= 0,
	DSSBC_Subject,
	DSSBC_PreShadow,
	DSSBC_PassNumber,
	DSSBC_ClassName,
	DSSBC_FullComponentName,
	DSSBC_MAX			// needs to be last entry
};

/**
 * Dynamic shadow stats browser class.
 */
class WxDynamicShadowStatsBrowser : public WxBrowser
{
	DECLARE_DYNAMIC_CLASS(WxDynamicShadowStatsBrowser);

protected:
	/** List control used for displaying stats */
	wxListCtrl*		ListControl;
	/** Array of object names. One for each row. Used as I can't seem to be able to retrieve this information via wxWidgets. */
	TArray<FString> ObjectNames;

public:
	WxDynamicShadowStatsBrowser();

	/**
	 * Forwards the call to our base class to create the window relationship.
	 * Creates any internally used windows after that
	 *
	 * @param DockID the unique id to associate with this dockable window
	 * @param FriendlyName the friendly name to assign to this window
	 * @param Parent the parent of this window (should be a Notebook)
	 */
	virtual void Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent);

	/**
	 * Tells the browser to update itself
	 */
	void Update(void);

	/**
	 * Returns the key to use when looking up values
	 */
	virtual const TCHAR* GetLocalizationKey(void) const
	{
		return TEXT("DynamicShadowStatsBrowser");
	}

protected:
	/**
	 * Handler for EVT_SIZE events.
	 *
	 * @param In the command that was sent
	 */
	void OnSize( wxSizeEvent& In );

	/**
	 * Handler for item activation (double click) event
	 *
	 * @param In the command that was sent
	 */
	void OnItemActivated( wxListEvent& In );

	/**
	 * Sets auto column width. Needs to be called after resizing as well.
	 */
	void SetAutoColumnWidth();

	DECLARE_EVENT_TABLE();
};

#endif // __DYNAMICSHADOWSTATSBROWSER_H__
