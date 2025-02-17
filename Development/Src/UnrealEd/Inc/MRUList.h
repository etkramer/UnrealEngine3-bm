/*=============================================================================
	MRUList : Helper class for handling MRU lists

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * An MRU list of files which containts FMRUList::MRU_MAX_ITEMS items.
 */
class FMRUList
{
public:
	FMRUList(const FString& InINISection, wxMenu* InMenu );
	~FMRUList();

	/**
	 * Make sure we don't have more than MRU_MAX_ITEMS in the list.
	 */
	void Cull();
	
	void ReadINI();
	void WriteINI() const;
	
	/**
	 * Moves the specified item to the top of the MRU list.
	 *
	 * @param	ItemIndex		Index of the item to bring to top.
	 */
	void MoveToTop(INT ItemIndex);

	/**
	 * Adds an item to the MRU list, moving it to the top.
	 *
	 * @param	Item		The item to add.
	 */
	void AddItem(const FFilename& Item);
	
	/**
	 * Returns the index of the specified item, or INDEX_NONE if not found.
	 *
	 * @param	Item		The item to query.
	 * @reutrn				The index of the specified item.
	 */
	INT FindItemIdx(const FFilename& Item) const;

	/**
	 * Removes the specified item from the list if it exists.
	 *
	 * @param	Item		The item to remove.
	 */
	void RemoveItem(const FFilename& Item);

	/**
	 * Removes the item at the specified index.
	 *
	 * @param	ItemIndex	Index of the item to remove.  Must be in [0, MRU_MAX_ITEMS-1].
	 */
	void RemoveItem(INT ItemIndex);

	/**
	 * Adds all the MRU items to the menu.
	 */
	void UpdateMenu();

	/**
	 * Checks to make sure the file specified still exists.  If it does, it is
	 * moved to the top of the MRU list and we return TRUE.  If not, we remove it
	 * from the list and return FALSE.
	 *
	 * @param	ItemIndex		Index of the item to query
	 * @return					TRUE if the item exists, FALSE if it doesn't
	 */
	UBOOL VerifyFile(INT ItemIndex);

	/**
	 * Accessor.
	 *
	 * @param	ItemIndex		Index of the item to access.
	 * @return					The filename at that index.
	 */
	const FFilename& GetItem(INT ItemIndex) const { return Items(ItemIndex); }

private:
	enum
	{
		/** The maximum number of items a FMRUList contains. */
		MRU_MAX_ITEMS = 8
	};

	/** The filenames. */
	TArray<FFilename> Items;

	/** The INI section we read/write the filenames to. */
	FString INISection;

	/** The menu that we write these MRU items to. */
	wxMenu* Menu;
};
