/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Player stats scene for UT3.
 */
class UTUIFrontEnd_PlayerStats extends UTUIFrontEnd;

/** The index to use for getting detailed stats information for a specific result row on the leaderboard in the previous screen. */
var transient int StatsIndex;

/** Reference to the various tab pages. */
var transient UTUITabPage_StatsGeneral GeneralTab;
var transient UTUITabPage_StatsWeapons WeaponsTab;
var transient UTUITabPage_StatsVehicles VehiclesTab;
var transient UTUITabPage_StatsVehicleWeapons VehicleWeaponsTab;
var transient UTUITabPage_StatsRewards RewardsTab;

/** Reference to the stats datastore. */
var transient UTDataStore_OnlineStats StatsDataStore;

/** Player name label. */
var transient UILabel	PlayerNameLabel;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize()
{
	Super.PostInitialize();

	// Get widget references
	PlayerNameLabel = UILabel(FindChild('lblPlayerName', true));

	// Add tab pages to tab control
	GeneralTab = UTUITabPage_StatsGeneral(FindChild('pnlGeneral', true));
	if(GeneralTab != none)
	{
		TabControl.InsertPage(GeneralTab, 0, 0, false);
	}

	WeaponsTab = UTUITabPage_StatsWeapons(FindChild('pnlWeapons', true));
	if(WeaponsTab != none)
	{
		TabControl.InsertPage(WeaponsTab, 0, 1, false);
	}

	VehiclesTab = UTUITabPage_StatsVehicles(FindChild('pnlVehicles', true));
	if(VehiclesTab != none)
	{
		TabControl.InsertPage(VehiclesTab, 0, 2, false);
	}

	VehicleWeaponsTab = UTUITabPage_StatsVehicleWeapons(FindChild('pnlVehicleWeapons', true));
	if(VehicleWeaponsTab != none)
	{
		TabControl.InsertPage(VehicleWeaponsTab, 0, 3, false);
	}

	RewardsTab = UTUITabPage_StatsRewards(FindChild('pnlRewards', true));
	if(RewardsTab != none)
	{
		TabControl.InsertPage(RewardsTab, 0, 4, false);
	}

	if(IsGame())
	{
		//@todo: Remove tabs depending on the current game mode.
	}

	StatsDataStore = UTDataStore_OnlineStats(FindDataStore('UTLeaderboards', GetPlayerOwner()));

	PlayerNameLabel.SetDataStoreBinding(StatsDataStore.DetailsPlayerNick);
}

/** Setup the scene's button bar. */
function SetupButtonBar()
{
	if(ButtonBar!=None)
	{	
		ButtonBar.Clear();
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.GamerCard>", OnButtonBar_PlayerCard);
	}
}

/** Callback to show the player card for this player. */
function OnPlayerCard()
{
	ShowPlayerCard(StatsDataStore.DetailsPlayerNetId,StatsDataStore.DetailsPlayerNick);
}

/** Callback for when the user wants to exit the scene. */
function OnBack()
{
	CloseScene(self);
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
		if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
		if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnPlayerCard();
			bResult=true;
		}
	}

	return bResult;
}


/** Buttonbar Callbacks. */
function bool OnButtonBar_Back(UIScreenObject InButton, int PlayerIndex)
{
	OnBack();

	return true;
}

function bool OnButtonBar_PlayerCard(UIScreenObject InButton, int PlayerIndex)
{
	OnPlayerCard();

	return true;
}


/**
 * Called when a new page is activated.
 *
 * @param	Sender			the tab control that activated the page
 * @param	NewlyActivePage	the page that was just activated
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this event.
 */
function OnPageActivated( UITabControl Sender, UITabPage NewlyActivePage, int PlayerIndex )
{
	// Start a stats read for the page we just activated
	UTUITabPage_StatsPage(NewlyActivePage).ReadStats();

	Super.OnPageActivated(Sender, NewlyActivePage, PlayerIndex);
}

defaultproperties
{


}