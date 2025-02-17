/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * UI scene that allows the user to setup a map cycle.
 */
class UTUIPanel_MapCycle extends UTTabPage;

/** List of available maps. */
var transient UIList AvailableList;
var	transient UIImage ListBackground_Available;

/** List of enabled maps. */
var transient UIList EnabledList;
var	transient UIImage ListBackground_Enabled;

/** The last focused UI List. */
var transient UIList LastFocused;

/** Label describing the currently selected map. */
var transient UILabel DescriptionLabel;

/** Label for the number of players the maps supports. */
var transient UILabel NumPlayersLabel;

/** Arrow images. */
var transient UIImage ShiftRightImage;
var transient UIImage ShiftLeftImage;

/** Reference to the menu datastore */
var transient UTUIDataStore_MenuItems MenuDataStore;

/** Reference to the shift up and down buttons. */
var transient UIButton ShiftUpButton;
var transient UIButton ShiftDownButton;

/** Shift up/down label callouts. */
var transient UILabel ShiftUpLabel;
var transient UILabel ShiftDownLabel;

const MANUAL_LIST_REFRESH_DATABINDING_INDEX=50;

/** Delegate for when the user selects a map on this page. */
delegate OnMapSelected();

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Store widget references
	AvailableList = UIList(FindChild('lstAvailable', true));
	LastFocused = AvailableList;
	AvailableList.OnValueChanged = OnAvailableList_ValueChanged;
	AvailableList.OnSubmitSelection = OnAvailableList_SubmitSelection;
	AvailableList.NotifyActiveStateChanged = OnList_NotifyActiveStateChanged;
	AvailableList.OnRawInputKey = OnMapList_RawInputKey;
	AvailableList.OnRefreshSubscriberValue = HandleRefreshSubscriberValue;

	EnabledList = UIList(FindChild('lstEnabled', true));
	EnabledList.OnValueChanged = OnEnabledList_ValueChanged;
	EnabledList.OnSubmitSelection = OnEnabledList_SubmitSelection;
	EnabledList.NotifyActiveStateChanged = OnList_NotifyActiveStateChanged;
	EnabledList.OnRawInputKey = OnMapList_RawInputKey;
	EnabledList.OnRefreshSubscriberValue = HandleRefreshSubscriberValue;

	DescriptionLabel = UILabel(FindChild('lblDescription', true));
	NumPlayersLabel = UILabel(FindChild('lblNumPlayers', true));
	ShiftUpLabel = UILabel(FindChild('lblShiftUp', true));
	ShiftDownLabel = UILabel(FindChild('lblShiftDown', true));
	ShiftRightImage = UIImage(FindChild('imgArrowLeft', true));
	ShiftLeftImage = UIImage(FindChild('imgArrowRight', true));
	ListBackground_Available = UIImage(FindChild('imgAvailable', true));
	ListBackground_Enabled = UIImage(FindChild('imgEnabled', true));

	// Setup shift button callbacks.
	ShiftUpButton = UIButton(FindChild('btnShiftUp', true));
	ShiftUpButton.OnClicked=OnButtonBar_ShiftUp;
	ShiftUpButton.SetVisibility(!IsConsole(CONSOLE_Any));
	ShiftDownButton = UIButton(FindChild('btnShiftDown', true));
	ShiftDownButton.OnClicked=OnButtonBar_ShiftDown;
	ShiftDownButton.SetVisibility(!IsConsole(CONSOLE_Any));

	// Get reference to the menu datastore
	MenuDataStore = UTUIDataStore_MenuItems(StaticResolveDataStore('UTMenuItems'));

	LoadMapCycle();
}


/**
 * Called when this widget receives a call to RefreshSubscriberValue.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being refreshed, for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0,
 *								while values greater than FIRST_DEFAULT_DATABINDING_INDEX correspond to tooltips and context
 *								menus.
 *
 * @return	TRUE to indicate that this widget is going to refresh its value manually.
 */
function bool HandleRefreshSubscriberValue( UIObject Sender, int BindingIndex )
{
	if ( BindingIndex < FIRST_DEFAULT_DATABINDING_INDEX && Sender.IsInitialized() )
	{
		if ( BindingIndex != MANUAL_LIST_REFRESH_DATABINDING_INDEX
		&&	Sender != EnabledList )
		{
			OnMapListChanged();
			return true;
		}
	}

	return false;
}

function name GetCurrentGameMode()
{
	local string GameMode;

	GetDataStoreStringValue("<Registry:SelectedGameMode>", GameMode);

	// strip out package so we just have class name
	return name(Right(GameMode, Len(GameMode) - InStr(GameMode, ".") - 1));
}

/** @return Returns the first currently selected map. */
function string GetSelectedMap()
{
	local string MapName;

	MapName="";

	if(EnabledList.Items.length>0)
	{
		class'UTUIMenuList'.static.GetCellFieldString(EnabledList, 'MapName', EnabledList.Items[0], MapName);
	}

	return MapName;
}


/** Loads the map cycle for the current game mode and sets up the datastore's lists. */
function LoadMapCycle()
{
	local int MapIdx;
	local int LocateIdx;
	local int CycleIdx;
	local name GameMode;

	GameMode = GetCurrentGameMode();
	MenuDataStore.MapCycle.length = 0;

	CycleIdx = class'UTGame'.default.GameSpecificMapCycles.Find('GameClassName', GameMode);
	if (CycleIdx != INDEX_NONE)
	{
		for(MapIdx=0; MapIdx<class'UTGame'.default.GameSpecificMapCycles[CycleIdx].Maps.length; MapIdx++)
		{
			LocateIdx = MenuDataStore.FindValueInProviderSet('Maps', 'MapName', class'UTGame'.default.GameSpecificMapCycles[CycleIdx].Maps[MapIdx]);

			if(LocateIdx != INDEX_NONE)
			{
				MenuDataStore.MapCycle.AddItem(LocateIdx);
			}
		}
	}

	OnMapListChanged();
}

/** Converts the current map cycle to a string map names and stores them in the config saved array. */
function GenerateMapCycleList(out GameMapCycle Cycle)
{
	local int MapIdx;
	local string MapName;

	Cycle.Maps.length = 0;

	for(MapIdx=0; MapIdx<MenuDataStore.MapCycle.length; MapIdx++)
	{
		if(MenuDataStore.GetValueFromProviderSet('Maps', 'MapName', MenuDataStore.MapCycle[MapIdx], MapName))
		{
			Cycle.Maps.AddItem(MapName);
		}
	}
}

/** Transfers the current map cycle in the menu datastore to our array of config saved map cycles for each gamemode. */
function SaveMapCycle()
{
	local int CycleIdx;
	local name GameMode;
	local GameMapCycle MapCycle;

	GameMode = GetCurrentGameMode();

	MapCycle.GameClassName = GameMode;
	GenerateMapCycleList(MapCycle);

	CycleIdx = class'UTGame'.default.GameSpecificMapCycles.Find('GameClassName', GameMode);
	if (CycleIdx == INDEX_NONE)
	{
		CycleIdx = class'UTGame'.default.GameSpecificMapCycles.length;
	}
	class'UTGame'.default.GameSpecificMapCycles[CycleIdx] = MapCycle;

	// Save the config for this class.
	class'UTGame'.static.StaticSaveConfig();
}

/** Sets up the button bar for the parent scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	// Hide labels by default
	ShiftUpLabel.SetVisibility(false);
	ShiftDownLabel.SetVisibility(false);

	if(EnabledList != None)
	{
		if(LastFocused==EnabledList)
		{
			if(EnabledList.Items.length > 0)
			{
				ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.RemoveMap>", OnButtonBar_MoveMap);

				// Shows/Hides the shifting labels depending on paltform.
				ShiftUpLabel.SetVisibility(IsConsole(CONSOLE_Any));
				ShiftDownLabel.SetVisibility(IsConsole(CONSOLE_Any));

				/*
				if(IsConsole(CONSOLE_Any))
				{
					ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ShiftMap>", OnButtonBar_ShiftUp);
				}
				else
				{
					ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ShiftUpTrigger>", OnButtonBar_ShiftUp);
					ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ShiftDownTrigger>", OnButtonBar_ShiftDown);
				}
				*/
			}
		}
		else
		{
			if(AvailableList.Items.length > 0)
			{
				ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.AddMap>", OnButtonBar_MoveMap);
			}
		}
	}
}

/** Selects the best list depending on which list has currently available maps. */
function SelectBestList()
{
	if(AvailableList.Items.length > 0)
	{
		AvailableList.SetFocus(none);
	}
	else
	{
		EnabledList.SetFocus(none);
	}
}

/** Called whenever one of the map lists changes. */
function OnMapListChanged()
{
	local int i, EnabledIndex, EnabledItem, AvailableIndex, AvailableItem;

	AvailableIndex = AvailableList.Index;
	AvailableItem = AvailableList.GetCurrentItem();

	EnabledIndex = EnabledList.Index;
	EnabledItem = EnabledList.GetCurrentItem();

	// Have both lists refresh their subscriber values
	AvailableList.IncrementAllMutexes();
	EnabledList.IncrementAllMutexes();

	// directly set the indexes to a bad value so that when we call SetIndex later, we're guaranteed to get an update notification
	AvailableList.Index = -2;
	EnabledList.Index = -2;

	// now repopulate the lists
	AvailableList.RefreshSubscriberValue(MANUAL_LIST_REFRESH_DATABINDING_INDEX);
	EnabledList.RefreshSubscriberValue(MANUAL_LIST_REFRESH_DATABINDING_INDEX);


	// first, we'll attempt to reselect the same mutator that was previously selected by searching for the previously
	// selected item.  it might not be there if that item was just moved to another list or something, in which case we'll
	// just select the item now in that position in the list
	i = AvailableList.Items.Find(AvailableItem);
	if ( i == INDEX_NONE )
	{
		i = AvailableIndex;
	}

	AvailableList.DecrementAllMutexes();
	AvailableList.SetIndex(i);


	// now do the same for the list of active mutators.
	i = EnabledList.Items.Find(EnabledItem);
	if ( i == INDEX_NONE )
	{
		i = EnabledIndex;
	}

	EnabledList.DecrementAllMutexes();
	EnabledList.SetIndex(i);

	// if the number of items in one of the lists was less than the number of items it can display but now is more, then
	// the list that didn't lose an item will have re-selected the same element, meaning that it won't trigger a scene update
	// so let's do that manually ourselves
	RequestFormattingUpdate();
	RequestSceneUpdate(false,true);
}

/** Clears the enabled map list. */
function OnClearMaps()
{
	MenuDataStore.MapCycle.length=0;
	OnMapListChanged();

	// Set focus to the available list.
	AvailableList.SetFocus(none);
	OnSelectedMapChanged();
}

/** Updates widgets when the currently selected map changes. */
function OnSelectedMapChanged()
{
	UpdateDescriptionLabel();
	UTUIFrontEnd(GetScene()).SetupButtonBar();

	// Update arrows
	if(LastFocused==EnabledList)
	{
		ShiftLeftImage.SetEnabled(false);
		ShiftRightImage.SetEnabled(true);
	}
	else
	{
		ShiftLeftImage.SetEnabled(true);
		ShiftRightImage.SetEnabled(false);
	}
}

/** Callback for when the user tries to move a map from one list to another. */
function OnMoveMap()
{
	local int MapId;

	if(LastFocused==AvailableList)
	{
		if(AvailableList.Items.length > 0)
		{
			MapId = AvailableList.GetCurrentItem();
			if(MenuDataStore.MapCycle.Find(MapId)==INDEX_NONE)
			{
				MenuDataStore.MapCycle.AddItem(MapId);
				OnMapListChanged();
			}

			if(AvailableList.Items.length==0)
			{
				EnabledList.SetFocus(none);

			}
		}
	}
	else
	{
		if(EnabledList.Items.length > 0)
		{
			MapId = EnabledList.GetCurrentItem();
			if(MenuDataStore.MapCycle.Find(MapId)!=INDEX_NONE)
			{
				MenuDataStore.MapCycle.RemoveItem(MapId);

				// If we removed all of the enabled maps, set focus back to the available list.
				if(MenuDataStore.MapCycle.length==0 && EnabledList.IsFocused())
				{
					AvailableList.SetFocus(none);
				}

				OnMapListChanged();
			}

			if(EnabledList.Items.length==0)
			{
				AvailableList.SetFocus(none);
			}
		}
	}

	OnSelectedMapChanged();
}

/** Shifts maps up and down in the map cycle. */
function OnShiftMap(bool bShiftUp)
{
	local int SelectedItem;
	local int SwapItem;
	local int NewIndex;

	SelectedItem = EnabledList.Index;

	if(bShiftUp)
	{
		NewIndex = SelectedItem-1;
	}
	else
	{
		NewIndex = SelectedItem+1;
	}

	if(NewIndex >= 0 && NewIndex < MenuDataStore.MapCycle.length)
	{
		SwapItem = MenuDataStore.MapCycle[NewIndex];
		MenuDataStore.MapCycle[NewIndex] = MenuDataStore.MapCycle[SelectedItem];
		MenuDataStore.MapCycle[SelectedItem] = SwapItem;

		OnMapListChanged();

		EnabledList.SetIndex(NewIndex);
	}
}

/** The user has finished setting up their cycle and wants to save changes. */
function OnAccept()
{
	SaveMapCycle();

	OnMapSelected();
}

/** Updates the description label. */
function UpdateDescriptionLabel()
{
	local string NewDescription;
	local int SelectedItem;

	SelectedItem = LastFocused.GetCurrentItem();

	if(class'UTUIMenuList'.static.GetCellFieldString(LastFocused, 'Description', SelectedItem, NewDescription))
	{
		DescriptionLabel.SetDataStoreBinding(NewDescription);
	}

	// Num Players
	if(class'UTUIMenuList'.static.GetCellFieldString(LastFocused, 'NumPlayers', SelectedItem, NewDescription))
	{
		NumPlayersLabel.SetDataStoreBinding(NewDescription);
	}
}

/**
 * Callback for when the user selects a new item in the available list.
 */
function OnAvailableList_ValueChanged( UIObject Sender, int PlayerIndex )
{
	OnSelectedMapChanged();
}

/**
 * Callback for when the user submits the selection on the available list.
 */
function OnAvailableList_SubmitSelection( UIList Sender, optional int PlayerIndex=GetBestPlayerIndex() )
{
	OnMoveMap();
}

/** Callback for when the object's active state changes. */
function OnList_NotifyActiveStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, optional UIState PreviouslyActiveState )
{
	local UIList ListSender;

	ListSender = UIList(Sender);
	if ( ListSender != None )
	{
		if ( UIState_Focused(NewlyActiveState) != None )
	{
			LastFocused = ListSender;
		OnSelectedMapChanged();


			// for visual effect - the disabled state of these images contains the data to make the background image appear focused
			if ( ListSender == EnabledList )
			{
				if ( ListBackground_Enabled != None )
				{
					ListBackground_Enabled.SetEnabled(false, PlayerIndex);
				}
			}
			else
			{
				if ( ListBackground_Available != None )
				{
					ListBackground_Available.SetEnabled(false, PlayerIndex);
				}
			}
		}
		else if ( UIState_Focused(PreviouslyActiveState) != None &&	!ListSender.IsFocused(PlayerIndex) )
		{
			// for visual effect - the enabled state of these images contains the data to make the background image appear not-focused
			if ( ListSender == EnabledList )
			{
				if ( ListBackground_Enabled != None )
				{
					ListBackground_Enabled.SetEnabled(true, PlayerIndex);
				}
			}
			else
			{
				if ( ListBackground_Available != None )
				{
					ListBackground_Available.SetEnabled(true, PlayerIndex);
				}
			}
		}
	}
}

/**
 * Callback for when the user selects a new item in the enabled list.
 */
function OnEnabledList_ValueChanged( UIObject Sender, int PlayerIndex )
{
	OnSelectedMapChanged();
}

/**
 * Callback for when the user submits the selection on the enabled list.
 */
function OnEnabledList_SubmitSelection( UIList Sender, optional int PlayerIndex=GetBestPlayerIndex() )
{
	OnMoveMap();
}


/** Callback for the map lists, captures the accept button before the lists get to it. */
function bool OnMapList_RawInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult = false;

	if(EventParms.EventType==IE_Released && EventParms.InputKeyName=='XboxTypeS_A')
	{
		OnAccept();
		bResult = true;
	}
	else if(EventParms.InputKeyName=='XboxTypeS_LeftTrigger')
	{

		if(LastFocused==EnabledList)
		{
			OnShiftMap(true);
		}

		bResult=true;
	}
	else if(EventParms.InputKeyName=='XboxTypeS_RightTrigger')
	{
		if(LastFocused==EnabledList)
		{
			OnShiftMap(false);
		}

		bResult=true;
	}

	return bResult;
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Accept(UIScreenObject InButton, int PlayerIndex)
{
	OnAccept();

	return true;
}

function bool OnButtonBar_ClearMaps(UIScreenObject InButton, int PlayerIndex)
{
	OnClearMaps();

	return true;
}

function bool OnButtonBar_MoveMap(UIScreenObject InButton, int PlayerIndex)
{
	OnMoveMap();

	return true;
}

function bool OnButtonBar_ShiftUp(UIScreenObject InButton, int PlayerIndex)
{
	OnShiftMap(true);

	return true;
}

function bool OnButtonBar_ShiftDown(UIScreenObject InButton, int PlayerIndex)
{
	OnShiftMap(false);

	return true;
}


/**
 * Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called BEFORE kismet is given a chance to process the input.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult=false;

	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_A' || EventParms.InputKeyName=='XboxTypeS_Enter')	// Accept Cycle
		{
			OnAccept();

			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_Y')		// Move map
		{
			OnMoveMap();

			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_LeftTrigger')
		{
			OnShiftMap(true);

			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_RightTrigger')
		{
			OnShiftMap(false);

			bResult=true;
		}
	}

	return bResult;
}

