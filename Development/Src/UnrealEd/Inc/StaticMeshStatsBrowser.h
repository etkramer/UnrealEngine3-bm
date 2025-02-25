/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __PRIMITIVESTATSBROWSER_H__
#define __PRIMITIVESTATSBROWSER_H__

/** 
 * Enumeration of columns.
 */
enum EPrimitiveStatsBrowserColumns
{
	PCSBC_Type				= 0,
	PCSBC_Name,
	PCSBC_Count,
	PCSBC_Sections,
	PCSBC_Triangles,
	PCSBC_InstTriangles,
	PCSBC_ResourceSize,
	PCSBC_LightsLM,
	PCSBC_LightsOther,
	PCSBC_LightsTotal,
	PCSBC_ObjLightCost,
	PCSBC_LightMapData,
	PCSBC_ShadowMapData,
	PCSBC_LMSMResolution,
	PCSBC_RadiusMin,
	PCSBC_RadiusMax,
	PCSBC_RadiusAvg,

	PCSBC_MAX				// needs to be last entry
};

/**
 * Static mesh stats browser class.
 */
class WxPrimitiveStatsBrowser : public WxBrowser
{
	DECLARE_DYNAMIC_CLASS(WxPrimitiveStatsBrowser);

protected:
	/** List control used for displaying stats. */
	WxListView*		ListControl;
	/** Array of column headers, used during CSV export. */
	TArray<FString> ColumnHeaders;

public:
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
	 * Called when the browser is getting activated (becoming the visible
	 * window in it's dockable frame).
	 */
	void Activated(void);

	/**
	 * Tells the browser to update itself
	 */
	void Update(void);

	/**
	 * Returns the key to use when looking up values
	 */
	virtual const TCHAR* GetLocalizationKey(void) const
	{
		return TEXT("PrimitiveStatsBrowser");
	}

protected:

	/**
	 * Dumps current stats to CVS file.
	 *
	 * @param NumRows	Number of rows to dump
	 */
	void DumpToCSV( INT NumRows );

	/**
	 * Inserts a column into the control.
	 *
	 * @param	ColumnId		Id of the column to insert
	 * @param	ColumnHeader	Header/ description of the column.
	 * @param	Format			The alignment of the column text.
	 */
	void InsertColumn( EPrimitiveStatsBrowserColumns ColumnId, const TCHAR* ColumnHeader, int Format = wxLIST_FORMAT_LEFT );

	/**
	 * Updates the primitives list with new data
	 *
	 * @param bResizeColumns	Whether or not to resize the columns after updating data.
	 */
	void UpdateList(UBOOL bResizeColumns=TRUE);

	/**
	 * Handler for EVT_SIZE events.
	 *
	 * @param In the command that was sent
	 */
	void OnSize( wxSizeEvent& In );

	/**
	 * Handler for column click events
	 *
	 * @param In the command that was sent
	 */
	void OnColumnClick( wxListEvent& In );

	/**
	 * Handler for column right click events
	 *
	 * @param In the command that was sent
	 */
	void OnColumnRightClick( wxListEvent& In );

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

	/** Current sort order (-1 or 1) */
	static INT CurrentSortOrder[PCSBC_MAX];
	/** Primary index/ column to sort by */
	static INT PrimarySortIndex;
	/** Secondary index/ column to sort by */
	static INT SecondarySortIndex;

	friend struct FPrimitiveStats;

	DECLARE_EVENT_TABLE();
};

#endif // __PRIMITIVESTATSBROWSER_H__
