/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "FConfigCacheIni.h"

FMRUList::FMRUList(const FString& InINISection, wxMenu* InMenu)
	:	INISection( InINISection ),
		Menu( InMenu )
{}

FMRUList::~FMRUList()
{
	Items.Empty();
}

// Make sure we don't have more than MRU_MAX_ITEMS in the list.
void FMRUList::Cull()
{
	while( Items.Num() > MRU_MAX_ITEMS )
	{
		Items.Remove( Items.Num()-1 );
	}
}

void FMRUList::ReadINI()
{
	Items.Empty();
	for( INT mru = 0 ; mru < MRU_MAX_ITEMS ; ++mru )
	{
		FString Item;
		GConfig->GetString( *INISection, *FString::Printf(TEXT("MRUItem%d"),mru), Item, GEditorUserSettingsIni );

		// If we get a item, add it to the top of the MRU list.
		if( Item.Len() )
		{
			new(Items) FString( Item );
		}
	}
}

void FMRUList::WriteINI() const
{
	// first, clear the section so that we remove any stale entries
	FConfigCacheIni* ConfigCache = static_cast<FConfigCacheIni*>(GConfig);
	ConfigCache->EmptySection(*INISection, GEditorUserSettingsIni);

	for( INT mru = 0 ; mru < Items.Num() ; ++mru )
	{
		GConfig->SetString( *INISection, *FString::Printf(TEXT("MRUItem%d"),mru), *Items(mru), GEditorUserSettingsIni );
	}

	// EmptySection calls Flush(), so we should call Flush again to that we don't lose our MRU list if the editor crashes
	GConfig->Flush(FALSE, GEditorUserSettingsIni);
}

// Moves the specified item to the top of the list.
void FMRUList::MoveToTop(INT InItem)
{
	check( InItem > -1 && InItem < Items.Num() );

	TArray<FFilename> WkArray;
	WkArray = Items;

	const FFilename Save = WkArray(InItem);
	WkArray.Remove( InItem );

	Items.Empty();
	new(Items)FString( *Save );
	Items += WkArray;
}

// Attempts to add an item to the MRU list.
void FMRUList::AddItem(const FFilename& InItem)
{
	// See if the item already exists in the list.  If so,
	// move it to the top of the list and leave.
	for( INT mru = 0 ; mru < Items.Num() ; ++mru )
	{
		if( Items(mru) == InItem )
		{
			MoveToTop( mru );
			UpdateMenu();
			WriteINI();
			return;
		}
	}

	// Item is new, so add it to the bottom of the list.
	if( InItem.Len() )
	{
		new(Items)FFilename( *InItem );
		MoveToTop( Items.Num()-1 );
	}

	Cull();
	UpdateMenu();
	WriteINI();
}

// Returns the index of the specified item, or INDEX_NONE if not found.
INT FMRUList::FindItemIdx(const FFilename& InItem) const
{
	for( INT mru = 0 ; mru < Items.Num() ; ++mru )
	{
		if( Items(mru) == InItem )
		{
			return mru;
		}
	}

	return INDEX_NONE;
}

// Removes the specified item from the list.
void FMRUList::RemoveItem(const FFilename& InItem)
{
	RemoveItem( FindItemIdx( InItem ) );
}

// Removes the item at the specified index.
void FMRUList::RemoveItem(INT InItem)
{
	// Find the item and remove it.
	check( InItem > -1 && InItem < MRU_MAX_ITEMS );
	Items.Remove( InItem );
}

// Adds all the MRU items to the menu.
void FMRUList::UpdateMenu()
{
	// Remove all existing MRU menu items.
	for( INT x = IDM_MRU_START ; x < IDM_MRU_END ; ++x )
	{
		if( Menu->FindItem( x ) )
		{
			Menu->Delete( x );
		}
	}

	// Add the MRU items to the menu.
	for( INT x = 0 ; x < Items.Num() ; ++x )
	{
		const FFilename Item = Items(x);
		const FString Text( FString::Printf( TEXT("&%d %s"), x+1, *Item ) );
		const FString Help( FString::Printf( LocalizeSecure(LocalizeUnrealEd("OpenTheFile_F"), *Item.GetBaseFilename()) ) );
		const INT ID = IDM_MRU_START + x;
		Menu->Append( IDM_MRU_START + x, *Text, *Help );
	}
}

// Checks to make sure the file specified still exists.  If it does, it is moved to the top
// of the MRU list and we return 1.  If not, we remove it from the list and return 0.
UBOOL FMRUList::VerifyFile(INT InItem)
{
	check( InItem > -1 && InItem < MRU_MAX_ITEMS );
	const FFilename filename = Items(InItem);

	// If the file doesn't exist, tell the user about it, remove the file from the list and update the menu.
	if( GFileManager->FileSize( *filename ) == -1 )
	{
		appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_FileDoesNotExist"), *filename) ) );
		RemoveItem( InItem );
		UpdateMenu();
		WriteINI();

		return FALSE;
	}

	// Otherwise, move the file to the top of the list and update the menu.
	MoveToTop( InItem );
	UpdateMenu();
	WriteINI();

	return TRUE;
}
