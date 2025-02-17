/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Map selection screen for UT3
 */

class UTUITabPage_MapSelection extends UTTabPage
	placeable;

/** Map cycle panel. */
var transient UTUIPanel_MapCycle	MapCyclePanel;

/** Single map panel. */
var transient UTUIPanel_SingleMap	SingleMapPanel;

/** Delegate for when the user selected a map or accepted their map cycle. */
delegate OnMapSelected();

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	MapCyclePanel = UTUIPanel_MapCycle(FindChild('pnlMapCycle', true));
	MapCyclePanel.OnMapSelected=OnMapCycleSelected;

	SingleMapPanel = UTUIPanel_SingleMap(FindChild('pnlSingleMap', true));
	SingleMapPanel.OnMapSelected=OnSingleMapSelected;

	// Setup tab caption
	SetDataStoreBinding("<Strings:UTGameUI.FrontEnd.TabCaption_Map>");

	SingleMapPanel.SetVisibility(true);
	MapCyclePanel.SetVisibility(false);
}


/** Sets up the button bar for the parent scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	if(MapCyclePanel.IsVisible())
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.SingleMap>", OnButtonBar_MapCycle);
		MapCyclePanel.SetupButtonBar(ButtonBar);
	}
	else
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.MapCycle>", OnButtonBar_MapCycle);
	}
}

/** Callback for when the map cycle has been selected. */
function OnMapCycleSelected()
{
	OnMapSelected();
}

/** Callback for when a single map has been selected. */
function OnSingleMapSelected()
{
	OnMapSelected();
}

/** Shows the map cycle configuration scene. */
function OnShowMapCycle()
{
	UTUIFrontEnd(GetScene()).GetUTInteraction().BlockUIInput(true);

	UTUIFrontEnd(GetScene()).ButtonBar.PlayUIAnimation('ButtonBarHide');

	if(MapCyclePanel.IsVisible())
	{
		MapCyclePanel.Add_UIAnimTrackCompletedHandler(OnMapPanel_Hide_UIAnimEnd);
		MapCyclePanel.PlayUIAnimation('TabPageExitLeft');
	}
	else
	{
		SingleMapPanel.Add_UIAnimTrackCompletedHandler(OnMapPanel_Hide_UIAnimEnd);
		SingleMapPanel.PlayUIAnimation('TabPageExitRight');
	}


}


/** Called when a tab page has finished hiding. */
function OnMapPanel_Hide_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
// moved into the check for TrackTypeMask==0
//	AnimTarget.Remove_UIAnimTrackCompletedHandler(OnMapPanel_Hide_UIAnimEnd);
	if ( TrackTypeMask == 0 )
	{
		AnimTarget.Remove_UIAnimTrackCompletedHandler(OnMapPanel_Hide_UIAnimEnd);
		if(MapCyclePanel.IsVisible())
		{
			SingleMapPanel.Add_UIAnimTrackCompletedHandler(OnMapPanel_Show_UIAnimEnd);
			SingleMapPanel.PlayUIAnimation('TabPageEnterRight');

			MapCyclePanel.SetVisibility(false);
			SingleMapPanel.SetVisibility(true);
			SingleMapPanel.SetFocus(none);
		}
		else
		{
			MapCyclePanel.Add_UIAnimTrackCompletedHandler(OnMapPanel_Show_UIAnimEnd);
			MapCyclePanel.PlayUIAnimation('TabPageEnterLeft');

			SingleMapPanel.SetVisibility(false);
			MapCyclePanel.SetVisibility(true);
			MapCyclePanel.SetFocus(none);
		}


		// Update button bar
		UTUIFrontEnd(GetScene()).SetupButtonBar();

		// Start show animations for the new tab page
		UTUIFrontEnd(GetScene()).ButtonBar.PlayUIAnimation('ButtonBarShow');
	}
}

/** Called when a tab page has finished showing. */
function OnMapPanel_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	if ( TrackTypeMask == 0 )
	{
		AnimTarget.Remove_UIAnimTrackCompletedHandler(OnMapPanel_Show_UIAnimEnd);
		UTUIFrontEnd(GetScene()).GetUTInteraction().BlockUIInput(false);
	}
}

/** @return Returns the first map, either the map selected in the single map selection or the first map of the map cycle. */
function string GetFirstMap()
{
	local string Result;

	if(MapCyclePanel.IsVisible())
	{
		Result = MapCyclePanel.GetSelectedMap();
	}
	else
	{
		Result = SingleMapPanel.GetSelectedMap();
	}

	return Result;
}

/** Callback for when the game mode changes, updates both panels. */
function OnGameModeChanged()
{
	// Update map cycle lists
	MapCyclePanel.LoadMapCycle();

	// Update currently selected map
	SingleMapPanel.MapList.RefreshSubscriberValue();
	SingleMapPanel.OnMapList_ValueChanged(SingleMapPanel.MapList);
}

/** @return Whether or not we can begin the match. */
function bool CanBeginMatch()
{
	local bool bResult;

	bResult = true;

	if(MapCyclePanel.IsVisible())
	{
		bResult = (MapCyclePanel.GetSelectedMap()!="");

		if(bResult==true)
		{
			MapCyclePanel.SaveMapCycle();
		}
		else
		{
			UTUIScene(GetScene()).DisplayMessageBox("<Strings:UTGameUI.Errors.EmptyMapCycle_Message>","<Strings:UTGameUI.Errors.EmptyMapCycle_Title>");
		}
	}

	return bResult;
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_MapCycle(UIScreenObject InButton, int PlayerIndex)
{
	OnShowMapCycle();

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
		if(EventParms.InputKeyName=='XboxTypeS_X')  // Swap betwene map cycle and single map
		{
			OnShowMapCycle();

			bResult=true;
		}
	}


	if(bResult==false)
	{
		if(MapCyclePanel.IsVisible())
		{
			bResult = MapCyclePanel.HandleInputKey(EventParms);
		}
		else
		{
			bResult = SingleMapPanel.HandleInputKey(EventParms);
		}
	}

	return bResult;
}
